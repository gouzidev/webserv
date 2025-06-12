#ifndef WEBSERV_HPP
#define WEBSERV_HPP


#include <errno.h>  // strictly forbidden, must remove later 


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
#include <sys/types.h>
#include <netdb.h>
// networking

// others
#include <unistd.h>
#include <dirent.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
// others



using namespace std;

#define GETROOT "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nDate: Wed, 30 Apr 2025 14:18:33 GMT\r\nLast-Modified: Thu, 17 Oct 2019 07:18:26 GMT\r\nContent-Length: 133\r\n\r\n<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n    <title>Document</title>\n</head>\n<body>\n    asmaaaaaaaaaaaaa<3\n</body>\n</html>"
#define GET 0
#define POST 1
#define DELETE 2
#define other 3

#define BUFFERSIZE 1024

#define ERROR  1

typedef int REQUEST;

class Response
{
    public :
        string fullResponse;
        string status_line;
        map <string, string> headers;
        vector <char> body;
        void setStatusLine();
        void setHeaders();
        void setBody();
};


class Request
{
    public :

        REQUEST req_type;
        string resource; // the resource is the path after the method in the request
        vector <string> start_line; //possible update
        map <string, string> headers;
        map <string, string> queryParams;
        vector <string> body; // possible update for large files in post
    public :
        int cfd; // client fd
        void setStartLine(string);
        void setHeaders(string line);
        void setBody(string line);
        int isStartLineValid();
        int getReqType();
        string & getResource();
        string getHttpVer();
        void fillQuery(string queryStr);
        vector <string> & getStartLine();
        map  <string ,string> & getHeaders();
        vector <string> & getBody();
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
        string hostStr;
        string hostIp;
        string root;
        set <string> serverNames;
        vector <LocationNode> locationNodes;
        map <string, LocationNode> locationDict;
        map <unsigned short, string> errorNodes;
        long long clientMaxBodySize;

};
class WebServ
{
    private:
        bool    criticalErr;
        vector <ServerNode> servNodes;
        map <string, ServerNode> hostServMap; // this map host:port to some server node
        map <string, ServerNode> servNameServMap; // this map servName:port to some server node


        map < string, string> db; // email password db to manage users
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
        void sendErrToClient(int clientfd, unsigned short errCode, ServerNode &servNode);
        void POST_METHODE(Request req, ServerNode servNode);
        void answer_req(Request req, set <int> activeSockets, ServerNode &servNode);
        int parse_request(int fd, set <int> activeSockets, ServerNode &servNode);
        int server();
        int serverLoop(int epollfd, struct epoll_event ev, set <int> activeSockets, map <int, ServerNode> &servSocketMap);
        int serverAsma();
        // void readFile(ifstream file);
};

std::vector<std::string> split (const std::string &s, char delim);
string trimWSpaces(string &text);
string trimSpaces(string &text);
bool startsWith(string str, string sub);
bool    isStrEq(string a, string b);
vector<string> split(string &str, char delim);
bool    strAllDigit(string s);
bool    checkFile(string filename, int perm);
bool    checkDir(string dirname, int dirStat);
bool validPath(string path);
string ushortToStr(unsigned short port);

bool exists(map <string, string> &m, string key);

template <typename T>
bool exists(map <T, T> &m, T key)
{
    return m.find(key) != m.end();
};

template <typename T1, typename T2>
bool exists(map <T1, T2> &m, T1 key)
{
    return m.find(key) != m.end();
};

template <typename T>
bool exists(set <T> s, T v)
{
    return s.find(v) != s.end();
}
#endif