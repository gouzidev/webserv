#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"


void WebServ::getMethode(Request req, ServerNode servNode)
{
    string target = req.getResource();
    string location = getLocation(target, servNode);
    LocationNode node = servNode.locationDict.find(location)->second;
    if (!exists(node.methods, string("GET")))
    {
        string errorRes  = getErrorResponse(404, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }
    cout << "location is [ " << location << " ]" << endl;
    const char *testResponse =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "\r\n";
    // "Hello, World!";
    string requestedFile = readFromFile(req.resource);
    Response resp;
    resp.fullResponse = testResponse + requestedFile;
    cout << "response is [ " << resp.fullResponse << " ]" << endl;
    send(req.cfd, resp.fullResponse.c_str(), resp.fullResponse.size(), 0);
    // cerr << testResponse;
}

