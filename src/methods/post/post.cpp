#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"

void WebServ::handleLogin(Request &req, ServerNode &serv)
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

    if (!strHas(body, "email=") || !strHas(body, "password="))
    {
        errorRes = getErrorResponse(400, "please provide an email and password");
        send(req.cfd, errorRes.c_str(), strlen(errorRes.c_str()), 0);
        return ;
    }
    map < string , string > queryParams;
    urlFormParser(body, queryParams);
    if (!exists(queryParams, string("email")) || !exists(queryParams, string("password")))
    {
        cout << "wtf0" << endl;
        sendErrPageToClient(req.cfd, 400, serv);
        return ;
    }
    string email = queryParams["email"];
    string password = queryParams["password"];

    cout << "email: " << email << endl;
    cout << "password: " << password << endl;
    auth->login(req.cfd, email, password, req);
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

void WebServ::postMethode(Request &req, ServerNode &serv)
{
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
    if (contentLen < 0)
    {
        // sendErrPageToClient(req.cfd, 400, serv);
        throw RequestException("content length is not valid", 400, req);
    }


    string contentType = headers.find("content-type")->second;
    string rootFolder = serv.root;

    string errorRes;
    string locationTarget = getLocation(req, serv); // will get "/" if the location is not in the server--
    if (locationTarget == "") // doesnt exist
    {
        errorRes  = getErrorResponse(404, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }

    LocationNode locationNode = serv.locationDict.find(locationTarget)->second;
    if (locationNode.isProtected)
    {
        string sessionKey = req.extractSessionId();
        if (!auth->isLoggedIn(sessionKey))
        {
            sendErrPageToClient(req.cfd, 401, serv);
            return ;
        }
    }
    if (!exists(locationNode.methods, string("POST"))) // methods are stored in upper case
    {
        errorRes  = getErrorResponse(405, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }

    if (contentType == "application/x-www-form-urlencoded") // handle form post request
    {
        if (locationTarget == "/login")
            handleLogin(req, serv);
        else if (locationTarget == "/signup")
            handleSignup(req, serv);
        else if (locationTarget == "/logout")
            handleLogout(req, serv);
    }   
    else if (startsWith(contentType, "multipart/form-data; boundary=")) // handle file upload
    {
        handleUplaod(req, contentLen, serv, locationNode);
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
    // const char *successResponse =
    // "HTTP/1.1 404 OK\r\n"
    // "Content-Type: text/plain\r\n"
    // "Content-Length: 12\r\n"
    // "\r\n"
    // "Successfull!";
    // send(req.cfd, successResponse, strlen(successResponse), 0);

    // cout << "BODY -> " << req.body << endl;
    // const char *testResponse =
    // "HTTP/1.1 404 OK\r\n"
    // "Content-Type: text/plain\r\n"
    // "Content-Length: 12\r\n"
    // "\r\n"
    // "bad request!";
    // send(req.cfd, testResponse, strlen(testResponse), 0);


    // cerr << testResponse;

}

