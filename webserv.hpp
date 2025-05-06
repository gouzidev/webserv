#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <set>
#include <vector>

using namespace std;

class ServerNode
{
    public :
        ServerNode();
        unsigned short port;
        string host;
        set <string> serverNames;
        vector <LocationNode> locationNodes;
        long long clientMaxBodySize;
};

class ErrorPageNode
{
    public :
        ErrorPageNode();
        set <unsigned short> codes;
        string page;
};


class LocationNode
{
    public :
        LocationNode();
        vector <string> headers;

        string path;
        set <string> methods;
        string root;
        string index;
        bool   autoIndex;
};

class WebServ
{
    private:
        bool    criticalErr;
    public:
        WebServ(char *confName);
        void parsing(char *filename);
        ServerNode parseServer();
        // void readFile(ifstream file);
};

string trimSpaces(string &text);

#endif