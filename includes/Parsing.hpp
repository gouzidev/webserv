#ifndef PARSING_HPP
#define PARSING_HPP

#include "Auth.hpp"

// containers
#include <map>
#include <set>
#include <vector>

#include <string>


using namespace std;

class LocationNode
{
    public:
        LocationNode();
        static set<string> possibleCgiExts;
        vector<string> headers;
        set<string> methods;

        // httpCode -> /location
        pair <short, string> redirect;
        string path;
        string root;
        vector<string> index;
        bool autoIndex;
        string uploadDir;
        // long long clientMaxBodySize;
        bool isProtected;
        bool needContentLen;
        map<string, string> cgiExts;
};

class ServerNode
{
    public:
        ServerNode();
        unsigned short port;
        string hostIp;
        string root;
        string errorFolder;
        string authFolder;
        vector<LocationNode> locationNodes;
        map<string, LocationNode> locationDict;
        map<unsigned short, string> errorNodes;
        string defaultErrorPage;
        long long clientMaxBodySize; // in MB


        // login
        string loginLocation; // usally /login
        string signupLocation; // usally /signup
        string logoutLocation; // usally /logout


};

#endif