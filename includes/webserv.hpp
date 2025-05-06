#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <set>
#include <vector>
#include <cstdlib>

using namespace std;

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



class WebServ
{
    private:
        bool    criticalErr;
    public:
        WebServ(char *confName);
        void parsing(char *filename);
        ServerNode parseServer(ifstream &configFile);
        // LocationNode parseLocation(ServerNode &serverNode, ifstream &configFile, string &line, size_t &lineI);
        // void readFile(ifstream file);
};

string trimSpaces(string &text);
bool    isStrEq(string a, string b);
#endif