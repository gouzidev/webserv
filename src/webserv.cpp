#include "../includes/webserv.hpp"

set <string> LocationNode::possibleCgiExts;
set <string> LocationNode::possibleMethods;

ServerNode::ServerNode()
{
    clientMaxBodySize = 5000000ll;
    host = "127.0.0.1";
}

LocationNode::LocationNode()
{
    possibleMethods.insert("DELETE");
    possibleMethods.insert("GET");
    possibleMethods.insert("POST");


    possibleCgiExts.insert(".py");
    possibleCgiExts.insert(".php");
    autoIndex = true;
    root = ".";
    path = "/";
}

WebServ::WebServ(char *filename)
{
    criticalErr = false;
    parsing(filename);
}

WebServ::WebServ(string filename)
{
    criticalErr = false;
    char *filename2 = const_cast<char *> (filename.c_str());
    parsing(filename2);
}

// void WebServ::readFile(ifstream file)
// {
//     string line;
//     while (getline(file, line))
//     {

//     }
// }

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