#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"

void WebServ::handleLogin(Request &req, ServerNode &serv)
{
    string errorRes;

    string body = req.body;
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
        sendErrToClient(req.cfd, 400, serv);
        return ;
    }
    string email = queryParams["email"];
    string password = queryParams["password"];

    auth->login(req.cfd, email, password, serv);
}

void WebServ::handleSignup(Request &req, ServerNode &serv)
{
    string errorRes;

    string body = req.body;
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
        sendErrToClient(req.cfd, 400, serv);
        return ;
    }
    string userName;
    if (exists(queryParams, "userName"))
        userName = queryParams["userName"];
    string fName = queryParams["firstName"];
    string lName = queryParams["lastName"];
    string email = queryParams["email"];
    string password = queryParams["password"];

    auth->signup(req.cfd, fName, userName, lName, email, password, serv);
}


void WebServ::handleLogout(Request &req, ServerNode &serv)
{
    string errorRes;

    string sessionKey = req.getSessionKey();
    
    auth->logout(req.cfd, sessionKey, serv);
}

void WebServ::handleUplaod(Request &req, ServerNode &servNode, LocationNode &locationNode)
{
    string errorRes;
    if (locationNode.uploadDir == "")
    {
        errorRes  = getErrorResponse(405, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
        // error (no upload path provided)
    }
    string boundary;
    string &contentType = req.headers["content-type"];
    size_t pos = contentType.find("=");
    boundary = contentType.substr(pos + 1);
}

void WebServ::postMethode(Request req, ServerNode servNode)
{
    short responseCode;
    string responseText;
    string responseBody;
    vector <string> startLine = req.getStartLine();
    ServerNode serv;
    string &location = req.getResource();
    map <string, string> &headers = req.getHeaders();
    string key = "host";
    // Debugger::printMap("headers", headers);
    if (!exists(headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        const char *testResponse = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nBad Client Request";
        send(req.cfd, testResponse, strlen(testResponse), 0);
        return ;
    }

    string contentType = headers.find("content-type")->second;
    string rootFolder = servNode.root;

    string errorRes;
    
    string locationTarget = getLocation(req.resource, servNode); // will get "/" if the location is not in the server

    if (!exists(servNode.locationDict, locationTarget)) // doesnt exist
    {
        errorRes  = getErrorResponse(404, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }


    LocationNode locationNode = servNode.locationDict.find(locationTarget)->second;
    if (!exists(locationNode.methods, string("POST"))) // methods are stored in upper case
    {
        errorRes  = getErrorResponse(405, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }

    if (contentType == "application/x-www-form-urlencoded") // handle form post request
    {
        cout << "locationTarget -> " << locationTarget << endl;
        if (locationTarget == "/login")
            handleLogin(req, serv);
        else if (locationTarget == "/signup")
            handleSignup(req, serv);
        else if (locationTarget == "/logout")
            handleLogout(req, serv);
    }   
    else if (startsWith(contentType, "multipart/form-data; boundary=")) // handle file upload
    {
        handleUplaod(req, servNode, locationNode);
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

