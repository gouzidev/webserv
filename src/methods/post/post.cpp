#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"

void WebServ::handleLogin(Request &req, ServerNode &serv)
{
    string errorRes;

    map < string , string > queryParams;

    string body = req.body;

    if (!exists(req.headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        const char *testResponse = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nBad Client Request";
        send(req.cfd, testResponse, strlen(testResponse), 0);
        return ;
    }
    if (!strHas(body, "email=") || !strHas(body, "password="))
    {
        errorRes = getErrorResponse(400, "please provide an email and password");
        send(req.cfd, errorRes.c_str(), strlen(errorRes.c_str()), 0);
        return ;
    }

    urlFormParser(body, queryParams);

    if (!exists(queryParams, string("email")) || !exists(queryParams, string("password")))
    {
        sendErrPageToClient(req.cfd, 400, serv);
        return ;
    }
    string email = queryParams["email"];
    string password = queryParams["password"];

    auth->login(req.cfd, email, password, req);
}

void WebServ::handleFormData(Request &req, ServerNode &serv)
{
    string errorRes;
    string dataDivStr = "<div>";
    map < string , string > queryParams;

    string body = req.body;

    if (!exists(req.headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        const char *testResponse = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nBad Client Request";
        send(req.cfd, testResponse, strlen(testResponse), 0);
        return ;
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
    cout << "{{{{" << page << "}}}}" << endl;

    string response;
    response += "HTTP/1.1 " + ushortToStr(201) + " " + getStatusMessage(201) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Content-Length: " + ushortToStr(page.size()) + "\r\n\r\n";
    response += page;

    send(req.cfd, response.c_str(), response.size(), 0);
    
}

string WebServ::getDataStrInDiv(string &name, string &value)
{
    string errorRes;
    string dataDivStr = "<div>";
    dataDivStr += "<li>" + name + " ---> " + value + "</li>";
    dataDivStr += "</div>";

    
    return dataDivStr;
}

void WebServ::handleSignup(Request &req, ServerNode &serv)
{
    string errorRes;

    string body = req.body;

    if (!exists(req.headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        const char *testResponse = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nBad Client Request";
        send(req.cfd, testResponse, strlen(testResponse), 0);
        return ;
    }

    if (!strHas(body, "firstName=") || !strHas(body, "lastName=")
        || !strHas(body, "email=") || !strHas(body, "password="))
    {
        errorRes = getErrorResponse(400, "please provide an email and password");
        send(req.cfd, errorRes.c_str(), strlen(errorRes.c_str()), 0);
        return ;
    }
    map < string , string > queryParams;
    urlFormParser(body, queryParams);
    if (!exists(queryParams, string("firstName")) || !exists(queryParams, string("lastName"))
        || !exists(queryParams, string("email")) || !exists(queryParams, string("password"))
        )
    {
        sendErrPageToClient(req.cfd, 400, serv);
        return ;
    }
    string userName;
    if (exists(queryParams, "userName"))
        userName = queryParams["userName"];
    string fName = queryParams["firstName"];
    string lName = queryParams["lastName"];
    string email = queryParams["email"];
    string password = queryParams["password"];

    auth->signup(req.cfd, fName, userName, lName, email, password, req);
}

void WebServ::handleLogout(Request &req, ServerNode &serv)
{
    cout << "handling logout" << endl;
    string sessionKey = req.getSessionKey();
    auth->logout(req.cfd, sessionKey, serv);
}

long long extractContentLen(Request &req, ServerNode &serv)
{
    if (!exists(req.headers, "content-length"))
        throw RequestException("could not find 'content-length' header", 400, req);
    string contentLenStr = req.headers.find("content-length")->second;
    contentLenStr = trimWSpaces(contentLenStr);
    if (!strAllDigit(contentLenStr))
        throw RequestException("content-length: '" + contentLenStr + "' is not a number", 400, req);

    istringstream contentLenStream (contentLenStr);
    if (contentLenStream.fail())
        throw RequestException("failed to parse content-length: '" + contentLenStr + "' it is not a number", 400, req);
    long long contentLen;
    contentLenStream >> contentLen;
    return contentLen;
}

bool handleChunkedBodyStart(string &body, Request &req)
{
    // since i know this is chunked i know it must be  :
    //      HEX\r\n
    long hex = -1;
    size_t i = 0;
    string chunk;
    string newBody;
    string hexStr;
    size_t startReadingIdx = 0;
    size_t hexEndPos = body.find("\r\n", 0);
    while (hexEndPos != string::npos)
    {
        hexStr = body.substr(i, hexEndPos - i);
        hex = stringToHexLong(hexStr, req);
        if (hex == 0)
            break;

        startReadingIdx = hexEndPos += 2; // skip the \r\n
        chunk = body.substr(startReadingIdx, hex);
        i = startReadingIdx + chunk.size();
        cout << "will read chunk of size " << hex << " bytes, -> {{" << chunk.substr(20) << "..." << chunk.substr(chunk.size() - 20) << "}}" << ",  i -> " << i  << endl;
        if (i > body.size() - 2)
        {
            cout << "Error: Reached the end of the body before finding the end of the chunked data." << endl;
            throw RequestException("Invalid chunked body format", 400, req);
        }
        if (body[i] != '\r' || body[i + 1] != '\n')
        {
            cout << "Error: Expected '\\r\\n' after chunk data, but found: '" << body.substr(i, 2) << "'" << endl;
            throw RequestException("Invalid chunked body format", 400, req);
        }
        i += 2;
        newBody += chunk;
        hexEndPos = body.find("\r\n", i);
    }
    body = newBody; // Update the original body with the parsed chunks
    req.headers["content-length"] = toString(newBody.size());
    return true;
}

bool parseChuncked(Request &req, ServerNode &serv)
{
    handleChunkedBodyStart(req.body, req);
    return true;

}

void WebServ::postMethode(Request &req, ServerNode &serv)
{
    string locationTarget = getLocation(req, serv); // will get "/" if the location is not in the server--
    string errorRes;
    LocationNode locationNode = serv.locationDict.find(locationTarget)->second;
    if (!exists(locationNode.methods, string("POST"))) // methods are stored in upper case
    {
        errorRes  = getErrorResponse(405, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }
    cout << "handling post request" << endl;
    vector <string> startLine = req.getStartLine();
    string &location = req.getResource();
    map <string, string> &headers = req.getHeaders();
    string key = "host";

    if (!exists(req.headers, "content-length"))
    {
        // sendErrPageToClient(req.cfd, 400, serv);
        throw RequestException("could not find 'content-length'", 400, req);
    }
    long long contentLen = extractContentLen(req, serv); // stored in MB
    if (contentLen <= 0 && locationNode.needContentLen)
    {
        // sendErrPageToClient(req.cfd, 400, serv);
        throw RequestException("content length is not valid", 400, req);
    }

    if (!exists(req.headers, "content-type"))
    {
        // sendErrPageToClient(req.cfd, 400, serv);
        throw RequestException("could not find 'content-type'", 400, req);
    }
    string contentType = headers.find("content-type")->second;
    string rootFolder = serv.root;

    if (locationTarget == "") // doesnt exist
    {
        cout << "location not found in server node for target -> " << req.getResource() << endl;
        errorRes  = getErrorResponse(404, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }

    if (locationNode.isProtected)
    {
        string sessionKey = req.extractSessionId();
        if (!auth->isLoggedIn(sessionKey))
        {
            sendErrPageToClient(req.cfd, 401, serv);
            return ;
        }
    }
 

    cout << "content type is " << contentType << endl;
    if (contentType == "application/x-www-form-urlencoded") // handle form post request
    {

        if (locationTarget == "/login")
            handleLogin(req, serv);
        else if (locationTarget == "/signup")
            handleSignup(req, serv);
        else if (locationTarget == "/logout")
            handleLogout(req, serv);
        else if (locationTarget == "/form-test")
            handleFormData(req, serv);
    }   
    else if (startsWith(contentType, "multipart/form-data; boundary=")) // handle file upload
    {
        cout << "handling file upload" << endl;
        handleUpload(req, serv, locationNode);
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
}

