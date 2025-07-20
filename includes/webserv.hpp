#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Auth.hpp"

#include <errno.h> // strictly forbidden, must remove later

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
#include <dirent.h>
#include <sys/stat.h>
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
#include <wait.h>
#include <dirent.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
// others

using namespace std;

#define GETROOT "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nDate: Wed, 30 Apr 2025 14:18:33 GMT\r\nLast-Modified: Thu, 17 Oct 2019 07:18:26 GMT\r\nContent-Length: 133\r\n\r\n<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n    <title>Document</title>\n</head>\n<body>\n    asmaaaaaaaaaaaaa<3\n</body>\n</html>"
// #define GET 0
// #define POST 1
// #define DELETE 2
// #define other 3

#define BUFFSIZE 64000

#define ERROR 1

typedef string REQUEST;

class Response
{
public:
    string fullResponse;
    string statusLine;
    string headers;
    vector<char> body;
    void setStatusLine(string sttsLine);
    void setHeaders(string header);
    void setBody();
};

class Request
{
    public:
        Response resp;
        REQUEST reqType;
        string resource;          // the resource is the path after the method in the request
        string fullResource; // /nodeRoot/resource
        vector<string> startLine; // possible update
        map<string, string> headers;
        map<string, string> queryParams;
        string mimeType;
        ServerNode &serv; // a server that the request belongs to
        string body;      // possible update for large files in post
        string extractSessionId();
        void setCookies();
        unsigned short error;
        long contentLen;
        map <string, string> cookies;

    public:
        int cfd; // client fd
        void setStartLine(string);
        void setHeaders(string line);
        void setBody(string line);
        string getSessionKey();
        int isStartLineValid();
        string getReqType();
        void getMimeType();
        string &getResource();
        string getHttpVer();
        void fillQuery(string queryStr);
        vector<string> &getStartLine();
        map<string, string> &getHeaders();
        bool fillHeaders(int fd);
        vector<string> &getBody();
        Request(ServerNode &serv);
};

class LocationNode
{
    public:
        LocationNode();
        static set<string> possibleMethods;
        static set<string> possibleCgiExts;
        vector<string> headers;
        set<string> methods;
        pair<short, string> redirect;
        string path;
        string root;
        vector<string> index;
        bool autoIndex;
        string uploadDir;
        // long long clientMaxBodySize;
        bool isProtected;
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
        set<string> serverNames;
        vector<LocationNode> locationNodes;
        map<string, LocationNode> locationDict;
        map<unsigned short, string> errorNodes;
        string defaultErrorPage;
        long long clientMaxBodySize; // in MB
};

class User
{
    private:
        static unsigned int userCount;
        unsigned int id;
        string email;
        string userName;
        string firstName;
        string lastName;
        string password;

    public:
        User();
        User(string email, string password);
        User(string fName, string lName, string email, string password);
        User(string fName, string lName, string userName, string email, string password);
        map<string, string> getKeyValData();
        const string &getEmail() const;
        const string &getFirstName() const;
        const string &getLastName() const;
        const string &getPassword() const;

        void setEmail(string str);
        void setFirstName(string str);
        void setLastName(string str);
        void setPassword(string str);
};

class Session
{
    private:
        User &user;
        string key;
        long int expiredAt; // in seconds
        long int createdAt; // in seconds
    public:
        Session(User &user);
        User &getUser();
        string &getKey();
        long int const &getExpiredAt() const;
        long int const &getCreatedAt() const;
        string generateSessionKey(const string &email);
        Session &operator=(const Session &session);
};

class WebServ
{
    private:
        bool criticalErr;
        vector<ServerNode> servNodes;
        map<string, ServerNode> hostServMap; // this map host:port to some server node
        map<string, ServerNode> servNameServMap; // this map servName:port to some server node
        bool logged;
        User loggedUser;
        Auth *auth; // auth instance (will manage the login and users)
    public:
        WebServ(char *confName);
        ~WebServ();
        WebServ(string filename);
        char *generalErrorResponse;
        vector<ServerNode> parsing(char *filename);
        ServerNode parseServer(ifstream &configFile, size_t &lineNum);
        void handleServerLine(ServerNode &servNode, ifstream &configFile, vector<string> &tokens, string &line, size_t &lineNum);
        void handleServerBlock(ServerNode &servNode, vector<string> &tokens, size_t &lineNum);
        void handleLocationLine(LocationNode &locationNode, vector<string> &tokens, size_t &lineNum);
        void parseLocation(ServerNode &serverNode, ifstream &configFile, string &line, size_t &lineNum);
        void getMethode(Request &req, ServerNode &serverNode);
        void validateParsing();
        bool validateLocationStr(string &location, ServerNode &serverNode, size_t &lineNum);
        bool validateLocation(ServerNode &servNode, LocationNode &locationNode);
        void postMethode(Request &req, ServerNode &servNode);
        void deleteMethod(Request &req, ServerNode &servNode);
        int server();
        void requestChecks(Request &req, ServerNode &serv, string &location, LocationNode &node);
        // void handleGetFile(Request req);
        void handleGetFile(Request req, map<string, string> &data);
        int serverLoop(int epollfd, struct epoll_event ev, set<int> activeSockets, map<int, ServerNode> &servSocketMap);
        void urlFormParser(string body, map<string, string> &queryParms);
        void handleLogin(Request &req, ServerNode &serv);
        void handleSignup(Request &req, ServerNode &serv);
        void handleLogout(Request &req, ServerNode &serv);
        void handleUplaod(Request &req, long contentLen, ServerNode &servNode, LocationNode &locationNode);
};

void sendErrPageToClient(int clientfd, unsigned short errCode, ServerNode &servNode);
string getQuickResponse(short errCode, string fileStr);
map<string, string> getErrorData(unsigned short errCode);
std::vector<std::string> split(const std::string &s, char delim);
string trimWSpaces(string &text);
string trimSpaces(string &text);
bool startsWith(string &str, string sub);
bool isStrEq(string a, string b);
vector<string> split(string &str, char delim);
bool strAllDigit(string s);
bool checkFile(string filename, int perm);
bool checkDir(string dirname, int dirStat);
bool validPath(string path);

string ushortToStr(unsigned short port);
char decodeHex(string &str, size_t &idx);
long stringToHexLong(string str, Request &req);

string removeTrailingCR(string str);
string getHostPort(string host, unsigned short port);

string dynamicRender(string path, map<string, string> &data); // for html files

string getLocation(Request &req, ServerNode &servNode); // resource no location is the rest of location part

string getErrorResponse(unsigned short errorCode, string body);

string readFromFile(string path); // for html files

string getStatusMessage(unsigned short code);

long long extractContentLen(Request &req, ServerNode &serv);

vector<string> splitNoSpace(string &str, char delim);

bool exists(map<string, string> &m, string key);

bool strHas(string str, string sub);
string checkResource(string fullResource);
bool isDirectory(string& path);
bool isRegularFile(string& path);


// converts T types to strings, T could be int, float, double, etc.
template <typename T>
string toString(T value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template <typename T>
bool exists(map<T, T> &m, T key)
{
    return m.find(key) != m.end();
};

template <typename T1, typename T2>
bool exists(map<T1, T2> &m, T1 key)
{
    return m.find(key) != m.end();
};

template <typename T>
bool exists(set<T> s, T v)
{
    return s.find(v) != s.end();
}

#endif 