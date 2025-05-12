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

class ErrorPageNode
{
    public :
        set <unsigned short> codes;
        string page;
};

class LocationNode
{
    public :
        LocationNode();
        static set <string> possibleMethods;
        static set <string> possibleCgiExts;
        vector <string> headers;
        pair <short, string> redirect;
        string path;
        set <string> methods;
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
        vector <ErrorPageNode> errorNodes;
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