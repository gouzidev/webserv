#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Auth.hpp"
#include "Parsing.hpp"

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

#define MAXEVENTS 1024

enum ParserState
{
    pSearchingForBoundary,
    pParsingHeaders,
    pStreamingBody,
    pDone
};

enum MultipartState
{
    pMultipartBoundary,
    pMultipartHeaders,
    pMultipartBody,
    pMultipartError,
    pMultipartDone
};

// States for the de-chunker
enum ChunkedState
{
    pChunkSize,
    pChunkData,
    pChunkEnd
};

class ServerNode;

class LocationNode;

class User;

class Session;

class Request;

#define BUFFSIZE 1000000

#define ERROR 1

typedef string REQUEST;



enum ClientState
{
    // in handleClientRead
    READING_HEADERS, // reading the headers from client 
    
    READING_BODY, // reading body (and processing it)

    // in handleClientWrite
    WRITING_RESPONSE, // default state when writing in the response buffer, if the response is small it will be sent if it's big the state will change to WRITING_RESPONSE

    WRITING_DONE, // clean the client here (after sending the response)

    WRITING_ERROR, // send a http error (if a problem happened while reading the data or sending it, will set WRITING_DONE too to clean the client) 

    SENDING_DONE,
};

class Client
{
    public:
        ClientState clientState; // the state of our dear client (what interaction we are having with him (or her))

        // file descriptors
        int cfd; // client fd
        int sfd; // server associated with client
        int ifd; // input  file desciptor -> will be used with get  request (open an input  file to send chunks from it)
        int ofd; // output file desciptor -> will be used with post request (open an output file to recv chunks to   it)
        bool is_eof;
        
        Request &request;
        long lastReadPosition; // where i stopped reading from my file in case of sending response with chunks
        // track progress
        size_t totalRead;
        size_t bodyBytesRead;

        // buffers
        string requestBuff;
        string responseBuff;
    
        // calonical form
        Client(Request &request);
        Client(Request &request, int cfd);
        Client(Request &request, int cfd, int sfd);
        Client(Request &request, int cfd, int sfd, ClientState state);
        Client &operator=(const Client &);
};

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

struct UploadData
{
    // -- State for the Multipart Parser --
    // this state machine handles the actual file content (boundaries, headers, body).
    MultipartState multipartState;

    // this state machine handles the chunked encoding itself (e.g., "4\r\nWiki\r\n").
    ChunkedState chunkedState;

    // current content type header without boundary
    std::string contentType;

    int filefd;

    // `multipart_chunk` holds the clean, continuous data after de-chunking.
    // for normal requests, data moves directly from socket_chunk to multipart_chunk.
    string multipart_chunk;
    // `socket_chunk` holds the raw data exactly as it comes from the network.
    string socket_chunk;
    std::string filename;
    std::string name;
    bool isChunked;
    size_t remaining_in_chunk;

    // boundary vars
    std::string rawBoundary;
    std::string boundary_marker;
    std::string end_boundary_marker;
    std::string first_boundary_marker;

    UploadData(Client &client);
};

enum ContentType
{
    textPlain_t,
    wwwURLEncoded_t,
    multipartFormData_t,
    OTHER_t
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
        size_t bodyLen;
        string extractSessionId();
        void setCookies();
        unsigned short error;
        long contentLen;
        struct UploadData *uploadData;
        map <string, string> cookies;

        bool isChunked;
        ContentType contentType;    // text/plain
                                    // application/x-www-form-urlencoded
                                    //   multipart/form-data                 
        

    public:
        int cfd; // client fd
        void setStartLine(string);
        void setHeader(string line);
        void setHeaders(string &buff, size_t startLineEnd, size_t headersEnd);
        void setBody(string line);
        string getSessionKey();
        int isStartLineValid();
        string getReqType();
        string &getResource();
        string getHttpVer();
        void fillQuery(string queryStr);
        vector<string> &getStartLine();
        map<string, string> &getHeaders();
        bool fillHeaders(int fd);
        vector<string> &getBody();
        Request(ServerNode &serv);
        void verifyHeaders(Client &client);
};



class WebServ
{
    private:
        int epollfd;
        set<short> validRedirects;
        bool criticalErr;
        vector<ServerNode> servNodes;
        map<string, ServerNode> hostServMap; // this map host:port to some server node
        map<string, ServerNode> servNameServMap; // this map servName:port to some server node
        bool logged;
        User loggedUser;
        Auth *auth; // auth instance (will manage the login and users)
        map <int, Client> clients; // maps the client fd -> (to) -> Client

        set<int> servSockets;
        map<int, ServerNode> servSocketMap;
        struct epoll_event ev;
        
        // max possible number of files uploaded to the server -> 999
        short MAXSERVERUPLOADS;

        // max digits of the user id (if 2 -> 99), (if 3 -> 999)
        short MAX_USERID_DIGITS;

        // current number of files uploaded to the server (increments every time user uploads new file) 
        short currentUploadCount;
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
        void getMethode(Client &client);
        void validateParsing();
        bool validateLocationStr(string &location, ServerNode &serverNode, size_t &lineNum);
        bool validateLocation(ServerNode &servNode, LocationNode &locationNode);
        void postMethode(Client &client);
        void deleteMethod(Request &req, ServerNode &servNode);
        void server();
        void requestChecks(Client &client, string &location, LocationNode &node);
        string checkResource(string fullResource, string location);
        void handleGetUpload(Request req, LocationNode node, User loggedUser, string location);
        string uploadFile(string path, string root, Request req);
        bool checkIndex(LocationNode node, Client &client, string location);
        string listUploadFiles(string root, Request req);
        bool parseHeaders(Client &client);
        bool parseBody(Client &client);
        bool processReqBodyChunk(Client &client);
        void cleanClient(Client &client);
        void getMimeType(Request &req);
        void handleClientRead(Client &client);
        void handleClientWrite(Client &client);

        void setClientReadyToRecvData(Client &client, bool error);  // this will set the right flags for epoll, and make client ready to handle the data sent to him, will make call to handleClientWrite possible

        void processCompleteRequest(Client &client);

        // will return the name with the user id in the front
        string getFileNameWithUserId(Request &req, unsigned int userId, string originalName);
        // will get the original name and set the id found in the saved name 
        string getOriginalFileName(Request &req, string fileNameWithUserId, unsigned int &userIdAssociated); 
        // void handleGetFile(Request req);
        void dirList(string root, string location, Client &client);

        void handleGetFile(Client &client, map<string, string> &data);
        int serverLoop();
        void urlFormParser(string body, map<string, string> &queryParms);
        void handleLogin(Client &client);
        void handleSignup(Client &client);
        void handleLogout(Client &client);
        void handleUpload(Client &client, LocationNode &locationNode);
        void handleFormData(Client &client);
        string getDataStrInDiv(string &name, string &value);

};

void makeResponse(Client &client, string fileContent);

string getSmallErrPage(unsigned short errCode);

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

// for html files
std::string readChunkFromFile(Client &client);

string getStatusMessage(unsigned short code);

long long extractContentLen(Request &req, ServerNode &serv);

vector<string> splitNoSpace(string &str, char delim);

bool exists(map<string, string> &m, string key);
bool existsAndIs(map <string, string> &m, string key, string val);
bool strHas(string str, string sub);
string checkResource(string fullResource);
bool isDirectory(string& path);
bool isRegularFile(string& path);

bool strHas(string str, string sub, size_t pos);

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

bool isValidPos(ssize_t pos);
template <typename T>
bool exists(set<T> s, T v)
{
    return s.find(v) != s.end();
}

#endif 