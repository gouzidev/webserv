#include "../includes/webserv.hpp"

set <string> LocationNode::possibleCgiExts;
set <string> LocationNode::possibleMethods;

unsigned int User::userCount;

ServerNode::ServerNode()
{
    port = 0;
    clientMaxBodySize = 0;
    // host = "";
    hostStr = "";
    root = "";
}

LocationNode::LocationNode()
{
    possibleMethods.insert("DELETE");
    possibleMethods.insert("GET");
    possibleMethods.insert("POST");


    possibleCgiExts.insert(".py");
    possibleCgiExts.insert(".php");
    autoIndex = true;
    root = "";
    path = "";
    uploadDir = "";
    clientMaxBodySize = 0;
}


WebServ::WebServ(char *filename)
{
    generalErrorResponse = (char *)"HTTP/1.1 500 INTERNAL SERVER ERROR\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nServer Error";
   
    criticalErr = false;
    auth = new Auth();
    
    
    parsing(filename);
    if (!criticalErr)
        server();
}

WebServ::WebServ(string filename)
{
    generalErrorResponse = (char *)"HTTP/1.1 500 INTERNAL SERVER ERROR\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nServer Error";
    criticalErr = false;
    User admin("salah", "gouzi", "salahgouzi11@gmail.com", "1234");
    auth = new Auth();
    char *filename2 = const_cast<char *> (filename.c_str());
    parsing(filename2);
    if (!criticalErr)
        server();
}

WebServ::~WebServ()
{
    delete auth;
}

string Request::getSessionKey()
{
    if (!exists(headers, "cookie")) // its stored in lowercase
        return "";
    string cookie = headers["cookie"];

    if (!strHas(cookie, "sessionId="))
        return "";
    
    size_t pos = cookie.find('=');
    string sessionKey = cookie.substr(pos + 1);
    return sessionKey;
}

int main(int ac, char **av)
{
    if (ac == 1)
    {
        string filename = "config.conf";
        WebServ webserv = WebServ(filename);
        (void) webserv;
    }
    else if (ac != 2)
    {
        cout << "usage: ./wevserv [*.conf]" << endl;
    }
    else
    {
        WebServ webserv = WebServ(av[1]);
        (void) webserv;
    }
}