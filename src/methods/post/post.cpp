#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"


void handleLogin(Request &req, ServerNode &serv)
{
    string body = req.body;

    
    // if (body.find("email=") == body.npos || body.find("password=") == body.npos)
    // {
    //     getErrorResponse(400, "please provide an email and password");
    // }
    map < string , string > queryParams;
    urlFormParser(body, queryParams);
    string email;
    string pw;
}

void WebServ::POST_METHODE(Request req, ServerNode servNode)
{
    short responseCode;
    string responseText;
    string responseBody;
    vector <string> start_line = req.getStartLine();
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

