#include "../includes/webserv.hpp"

set <string> LocationNode::possibleCgiExts;
set <string> LocationNode::possibleMethods;



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
    uploadPath = "";
}

WebServ::WebServ(char *filename)
{
    criticalErr = false;
    db["salahgouzi11@gmail.com"] = "1234";
    parsing(filename);
    if (!criticalErr)
        server();
}

WebServ::WebServ(string filename)
{
    criticalErr = false;
    db["salahgouzi11@gmail.com"] = "1234";
    char *filename2 = const_cast<char *> (filename.c_str());
    parsing(filename2);
    if (!criticalErr)
        server();
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