#ifndef WEBSERV_HPP
#define WEBSERV_HPP


// containers
#include <map>
#include <set>
#include <vector>
// containers


// io operations
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <sstream>
#include <fcntl.h>
// io operations


// networking
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
// networking

// others
#include <unistd.h>
#include <dirent.h>
#include <algorithm>
// others

using namespace std;

#define GETROOT "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nDate: Wed, 30 Apr 2025 14:18:33 GMT\r\nLast-Modified: Thu, 17 Oct 2019 07:18:26 GMT\r\nContent-Length: 133\r\n\r\n<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n    <title>Document</title>\n</head>\n<body>\n    asmaaaaaaaaaaaaa<3\n</body>\n</html>"
#define GET 0
#define POST 1
#define DELETE 2
#define other 3

#define ERROR  1

typedef int REQUEST;

class Request
{
    private :
        REQUEST req_type;
        string resource; // tje resource is the path after the method in the request
        vector <string> start_line; //possible update
        map <string, string> headers;
        map <string, string> queryParams;
        vector <string> body; //possible update for large files in post
    public :
        void setStartLine(string);
        void setHeaders(string line);
        void setBody(string line);
        int isStartLineValid();
        int getReqType();
        string getResource();
        string getHttpVer();
        void fillQuery(string queryStr);
        vector <string> getStartLine();
        map  <string ,string> getHeaders();
        vector <string> getBody();
};

class LocationNode
{
    public :
        LocationNode();
        static set <string> possibleMethods;
        static set <string> possibleCgiExts;
        vector <string> headers;
        set <string> methods;
        pair <short, string> redirect;
        string path;
        string root;
        vector <string> index;
        bool   autoIndex;
        string upload_path;
        map <string, string> cgi_exts;
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
        map <string, LocationNode> locationDict;
        map <unsigned short, string> errorNodes;
        long long clientMaxBodySize;

};

class Debugger
{
    public:
        template <typename Container>
        static void printVec(string varName, vector <Container> &v)
        {
            cout << varName << ": " << endl << "----";
            for (size_t i = 0; i < v.size(); i++)
                cout << v[i] << "  ";
            cout << endl;
        }

        template <typename type1, typename type2>
        static void printMap(string varName, map <type1, type2> &m)
        {
            cout << varName << ": " << endl << "----";
            for (size_t i = 0; i < m.size(); i++)
                cout << m[i].first << " -> " << m[i].second << "  ";
            cout << endl;
        }

        static void printLocationNode(const LocationNode &locNode, int indent = 2)
        {
            string spaces(indent, ' ');
            
            cout << spaces << "🔹 Location Path: " << locNode.path << endl;
            cout << spaces << "  Root: " << locNode.root << endl;
            
            cout << spaces << "  Allowed Methods: ";
            for (set<string>::const_iterator it = locNode.methods.begin(); it != locNode.methods.end(); ++it)
                cout << *it << " ";
            cout << endl;
            
            cout << spaces << "  AutoIndex: " << (locNode.autoIndex ? "ON" : "OFF") << endl;
            
            cout << spaces << "  Index Files: ";
            for (size_t i = 0; i < locNode.index.size(); i++)
                cout << locNode.index[i] << " ";
            cout << endl;
            
            if (locNode.redirect.first != 0)
                cout << spaces << "  Redirect: " << locNode.redirect.first << " -> " << locNode.redirect.second << endl;
            
            if (!locNode.upload_path.empty())
                cout << spaces << "  Upload Path: " << locNode.upload_path << endl;
            
            if (!locNode.cgi_exts.empty())
            {
                cout << spaces << "  CGI Extensions:" << endl;
                for (map<string, string>::const_iterator it = locNode.cgi_exts.begin(); it != locNode.cgi_exts.end(); ++it)
                    cout << spaces << "    " << it->first << " -> " << it->second << endl;
            }
            
            if (!locNode.headers.empty())
            {
                cout << spaces << "  Headers:" << endl;
                for (size_t i = 0; i < locNode.headers.size(); i++)
                    cout << spaces << "    " << locNode.headers[i] << endl;
            }
        }
        
        static void printServerNode(const ServerNode &servNode)
        {
            cout << "\n🌐 SERVER NODE" << endl;
            cout << "===========================================" << endl;
            cout << "Host: " << servNode.host << endl;
            cout << "Port: " << servNode.port << endl;
            cout << "Root: " << servNode.root << endl;
            
            cout << "Server Names: ";
            for (set<string>::const_iterator it = servNode.serverNames.begin(); it != servNode.serverNames.end(); ++it)
                cout << *it << " ";
            cout << endl;
            
            cout << "Client Max Body Size: " << servNode.clientMaxBodySize << " megabytes" << endl;
            
            // Print error pages
            if (!servNode.errorNodes.empty())
            {
                typedef map<unsigned short, string> errorCodePageMapType;
                 map<unsigned short, string> errorCodePageMap = servNode.errorNodes;
                cout << "\n📄 Error Pages:" << endl;
                for (errorCodePageMapType::iterator it = errorCodePageMap.begin(); it != errorCodePageMap.end(); it++)
                {
                    cout << "  Error Code: " << it->first << endl;
                    cout << "  Error Page: " << it->second << endl;
                }
                cout << endl;
            }
            
            // Print locations
            if (!servNode.locationNodes.empty())
            {
                cout << "\n📂 Locations (" << servNode.locationNodes.size() << "):" << endl;
                for (size_t i = 0; i < servNode.locationNodes.size(); i++)
                {
                    cout << "\n  [" << i + 1 << "] Location" << endl;
                    printLocationNode(servNode.locationNodes[i]);
                }
            }
            cout << "===========================================" << endl;
        }
    
        // Main method to print all server configurations
        static void printServerConfig(const vector<ServerNode> &servNodes)
        {
            cout << "\n\n🖥️  WEBSERV CONFIGURATION" << endl;
            cout << "==========================================" << endl;
            cout << "Total Server Blocks: " << servNodes.size() << endl;
            
            for (size_t i = 0; i < servNodes.size(); i++)
            {
                cout << "\n📌 Server Block [" << i + 1 << "]" << endl;
                printServerNode(servNodes[i]);
            }
            
            // Print a summary of all servers by host:port
            cout << "\n📊 SERVER SUMMARY BY HOST:PORT" << endl;
            cout << "==========================================" << endl;
            map<string, vector<size_t> > hostPortMap;
            
            for (size_t i = 0; i < servNodes.size(); i++)
            {
                string hostPort = servNodes[i].host + ":" + to_string(servNodes[i].port);
                hostPortMap[hostPort].push_back(i);
            }
            
            for (map<string, vector<size_t> >::const_iterator it = hostPortMap.begin(); it != hostPortMap.end(); ++it)
            {
                cout << "Host:Port " << it->first << " has " << it->second.size() << " server(s)" << endl;
                cout << "  Default server: Server Block [" << it->second[0] + 1 << "]" << endl;
                
                if (it->second.size() > 1)
                {
                    cout << "  Additional servers: ";
                    for (size_t i = 1; i < it->second.size(); i++)
                        cout << "Server Block [" << it->second[i] + 1 << "] ";
                    cout << endl;
                }
            }
            cout << "==========================================" << endl;
        }

        // Helper function for to_string since it might not be available in all C++ versions
        template <typename T>
        static string to_string(T value)
        {
            ostringstream os;
            os << value;
            return os.str();
        }

};

class WebServ
{
    private:
        bool    criticalErr;
        vector <ServerNode> servNodes;
    public:
        WebServ(char *confName);
        WebServ(string filename);
        vector <ServerNode> parsing(char *filename);
        ServerNode parseServer(ifstream &configFile, size_t &lineNum);
        void handleServerLine(ServerNode &servNode, ifstream &configFile, vector <string> &tokens, string &line, size_t &lineNum);
        void handleServerBlock(ServerNode &servNode, vector <string> &tokens, size_t &lineNum);
        void handleLocationLine(LocationNode &locationNode, vector <string> &tokens, size_t &lineNum);
        void parseLocation(ServerNode &serverNode, ifstream &configFile, string &line, size_t &lineNum);
        void GET_METHODE(Request req);
        void validateParsing();
        bool validateLocationStr(string &location, ServerNode &serverNode, size_t &lineNum);
        bool validateLocation(ServerNode &servNode, LocationNode &locationNode);
        
        void POST_METHODE(Request req);
        void answer_req(Request req);
        int parse_request(int fd);
        int server();
        // void readFile(ifstream file);
};

std::vector<std::string> split (const std::string &s, char delim);
string trimSpaces(string &text);
bool startsWith(string str, string sub);
bool    isStrEq(string a, string b);
vector<string> split(string &str, char delim);
bool    strAllDigit(string s);
bool    checkFile(string filename, int perm);
bool    checkDir(string dirname, int dirStat);
#endif