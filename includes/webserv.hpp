#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <set>
#include <vector>
#include <sstream>

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
        string root;
        set <string> serverNames;
        vector <LocationNode> locationNodes;
        long long clientMaxBodySize;

};

class ErrorPageNode
{
    public :
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
        ServerNode parseServer(ifstream &configFile, size_t &lineNum);
        LocationNode parseLocation(ServerNode &serverNode, ifstream &configFile, size_t &lineNum);
        // void readFile(ifstream file);
};
std::vector<std::string> split (const std::string &s, char delim);
string trimSpaces(string &text);
bool    isStrEq(string a, string b);
vector<string> split(string &str, char delim);
bool    strAllDigit(string s);
#endif