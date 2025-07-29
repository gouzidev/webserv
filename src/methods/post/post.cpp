#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"

void WebServ::handleLogin(Client &client)
{
    Request &req = client.request;
    ServerNode &serv = req.serv;
    string errorRes;

    map < string , string > queryParams;

    string body = req.body;

    if (!exists(req.headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        throw HttpException(400, client);
    }
    if (!strHas(body, "email=") || !strHas(body, "password="))
    {
        cerr << "please provide an email and password" << endl;
        throw HttpException(400, client);
    }

    urlFormParser(body, queryParams);

    if (!exists(queryParams, string("email")) || !exists(queryParams, string("password")))
    {
        cerr << "email or password not found in query params" << endl;
        throw HttpException(400, client);
    }
    string email = queryParams["email"];
    string password = queryParams["password"];

    auth->login(client, email, password);
}

void WebServ::handleFormData(Client &client)
{
    Request &req = client.request;
    ServerNode &serv = req.serv;
    string errorRes;
    string dataDivStr = "<div>";
    map < string , string > queryParams;

    string body = req.body;

    if (!exists(req.headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        throw HttpException(400, client);
    }

    urlFormParser(body, queryParams);


    map < string , string >::iterator it;

    it = queryParams.begin();

    while (it != queryParams.end())
    {
        dataDivStr += "<li>" + it->first + " -> " + it->second + "</li>";
        it++;
    }
    dataDivStr += "</div>";

    queryParams["data"] = dataDivStr;
    string page = dynamicRender("/home/sgouzi/webserv/www/auth/form.html", queryParams);

    string response;
    response += "HTTP/1.1 " + ushortToStr(201) + " " + getStatusMessage(201) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Content-Length: " + ushortToStr(page.size()) + "\r\n\r\n";
    response += page;

    client.responseBuff = response;
    
}

string WebServ::getDataStrInDiv(string &name, string &value)
{
    string errorRes;
    string dataDivStr = "<div>";
    dataDivStr += "<li>" + name + " ---> " + value + "</li>";
    dataDivStr += "</div>";

    
    return dataDivStr;
}

void WebServ::handleSignup(Client &client)
{
    Request &req = client.request;
    ServerNode &serv = req.serv;
    string errorRes;

    string body = req.body;

    if (!exists(req.headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        throw HttpException(400, client);
    }

    if (!strHas(body, "firstName=") || !strHas(body, "lastName=")
        || !strHas(body, "email=") || !strHas(body, "password="))
    {
        cerr << "please provide firstName, lastName, email and password" << endl;
        throw HttpException(400, client);
    }
    map < string , string > queryParams;
    urlFormParser(body, queryParams);
    if (!exists(queryParams, string("firstName")) || !exists(queryParams, string("lastName"))
        || !exists(queryParams, string("email")) || !exists(queryParams, string("password"))
        )
    {
        cerr << "firstName, lastName, email or password not found in query params" << endl;
        throw HttpException(400, client);
    }
    string userName;
    if (exists(queryParams, "userName"))
        userName = queryParams["userName"];
    string fName = queryParams["firstName"];
    string lName = queryParams["lastName"];
    string email = queryParams["email"];
    string password = queryParams["password"];

    auth->signup(client, fName, userName, lName, email, password);
}

void WebServ::handleLogout(Client &client)
{
    Request &req = client.request;
    ServerNode &serv = req.serv;
    cout << "handling logout" << endl;
    string sessionKey = req.getSessionKey();
    auth->logout(client, sessionKey);
}

// long long extractContentLen(Client &client)
// {
//     Request &req = client.request;
//     ServerNode &serv = req.serv;
//     if (!exists(req.headers, "content-length"))
//     {
//         cerr << "could not find 'content-length' header" << endl;
//         throw HttpException(400, client);
//     }
//     string contentLenStr = req.headers.find("content-length")->second;
//     contentLenStr = trimWSpaces(contentLenStr);
//     if (!strAllDigit(contentLenStr))
//     {
//         cerr << "content-length is not a number: " << contentLenStr << endl;
//         throw  HttpException(400, client);
//     }

//     istringstream contentLenStream (contentLenStr);
//     if (contentLenStream.fail())
//     {
//         cerr << "content-length stream failed" << endl;
//         throw HttpException(400, client);
//     }
//     long long contentLen;
//     contentLenStream >> contentLen;
//     return contentLen;
// }

void WebServ::postMethode(Client &client)
{
    Request &req = client.request;
    string &buff = client.requestBuff;
    map <string, string> &headers = req.headers;
    ServerNode &serv = req.serv;
    string locationTarget = getLocation(req, serv); // will get "/" if the location is not in the server--
    cout << "POST target {{ " << locationTarget  << "}}" << endl;

    string errorRes;
    LocationNode locationNode = serv.locationDict.find(locationTarget)->second;
    if (!exists(locationNode.methods, string("POST"))) // methods are stored in upper case
        throw HttpException(405, client);
    vector <string> startLine = req.getStartLine();
    string &location = req.getResource();

    if (!exists(req.headers, "content-length"))
    {
        // sendErrPageToClient(req.cfd, 400, serv);
        cout << "could not find 'content-length'" << endl;
        throw HttpException(400, client);
    }
    long long contentLen = req.contentLen; // stored in MB
    if (contentLen <= 0 && locationNode.needContentLen)
    {
        // sendErrPageToClient(req.cfd, 400, serv);
        cout << "content length is not valid" << endl;
        throw HttpException(400, client);
    }

    if (!exists(req.headers, "content-type"))
    {
        // sendErrPageToClient(req.cfd, 400, serv);
        cout << "could not find 'content-type'" << endl;
        throw HttpException(400, client);
    }
    string contentType = headers.find("content-type")->second;
    string rootFolder = serv.root;

    if (locationTarget == "") // doesnt exist
    {
        cout << "location not found in server node for target -> " << req.getResource() << endl;
        throw HttpException(404, client);
    }

    cout << "location target is " << locationTarget << endl;

    if (locationNode.isProtected)
    {
        string sessionKey = req.extractSessionId();
        if (!auth->isLoggedIn(sessionKey))
        {
            cout << "un auth" << endl;
            throw HttpException(401, client);
        }
    }


    cout << "content type is " << contentType << endl;
    if (contentType == "application/x-www-form-urlencoded") // handle form post request
    {
        if (locationTarget == "/login")
            handleLogin(client);
        else if (locationTarget == "/signup")
            handleSignup(client);
        else if (locationTarget == "/logout")
            handleLogout(client);
        else
            handleFormData(client);
    }   
    else if (startsWith(contentType, "multipart/form-data; boundary=")) // handle file upload
    {
        cout << "handling file upload" << endl;
        handleUpload(client, locationNode);
    }
    else
    {
        cout << "content type is not supported for post request" << endl;
        const char *successResponse =
        "HTTP/1.1 404 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Successfull!";
        send(req.cfd, successResponse, strlen(successResponse), 0);

    }
    client.requestBuff.clear();
    client.clientState = SENDING_CHUNKS;
}