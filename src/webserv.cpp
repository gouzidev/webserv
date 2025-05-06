#include "../includes/webserv.hpp"

ServerNode::ServerNode()
{
    clientMaxBodySize = 5000000ll;
    host = "127.0.0.1";
}

LocationNode::LocationNode()
{
    autoIndex = true;
    index = "index.html";
    root = ".";
    path = "/";
}

WebServ::WebServ(char *filename)
{
    parsing(filename);
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
    if (ac != 2)
    {
        cout << "usage: ./wevserv [*.conf]" << endl;
    }
    else
    {
        WebServ webserv = WebServ(av[1]);
        (void) webserv;
    }
}