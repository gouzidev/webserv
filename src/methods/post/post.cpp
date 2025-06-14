#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"

void WebServ::handleLogin(Request &req, ServerNode &serv)
{
    const char *generalErrorResponse =
    "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "Server Error";

    string body = req.body;
    if (body.find("email=") == body.npos || body.find("password=") == body.npos)
    {
        getErrorResponse(400, "please provide an email and password");
    }
    cout << "[" << body << "]" << endl;
    map < string , string > queryParams;
    urlFormParser(body, queryParams);
    if (!exists(queryParams, string("email")) || !exists(queryParams, string("password")))
    {
        sendErrToClient(req.cfd, 400, serv);
        return ;
    }
    string email = queryParams["email"];
    string password = queryParams["password"];

    cout << "received email: " << email << endl;
    cout << "received password: " << password << endl;
    if (!exists(users, email))
    {
        cout << "email not found in users" << endl;
        sendErrToClient(req.cfd, 404, serv);
        return ;
    }
    string dbPassword = users[email].getPassword();
    if (dbPassword != password)
    {
        cout << "password does not match" << endl;
        sendErrToClient(req.cfd, 403, serv);
        return ;
    }
    cout << "user logged in successfully" << endl;
    logged = true;
    loggedUser = users[email];

    ifstream dashboardFile;
    dashboardFile.open("/home/sgouzi/prj/webserv/www/auth/dashboard.html");
    if (dashboardFile.fail())
    {
        cerr << "Error happened opening the file of dashboard" << endl;
        send(req.cfd, generalErrorResponse, strlen(generalErrorResponse), 0);
        return ;
    }
    string line, dashboardContent = "";
    while (getline(dashboardFile, line))
    {
        dashboardContent += line + "\r\n";
    }
    string response;
    response += "HTTP/1.1 " + ushortToStr(200) + " " + getStatusMessage(200) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Content-Length: " + ushortToStr(dashboardContent.size()) + "\r\n\r\n";
    response += dashboardContent;
    send(req.cfd, response.c_str(), response.length(), 0);
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
        const char *testResponse =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
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

    if (contentType == "application/x-www-form-urlencoded")
    {
        handleLogin(req, serv);
    }
    const char *successResponse =
    "HTTP/1.1 404 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 12\r\n"
    "\r\n"
    "Successfull!";
    send(req.cfd, successResponse, strlen(successResponse), 0);

    cout << "BODY -> " << req.body << endl;
    const char *testResponse =
    "HTTP/1.1 404 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 12\r\n"
    "\r\n"
    "bad request!";
    send(req.cfd, testResponse, strlen(testResponse), 0);


    // cerr << testResponse;

}

