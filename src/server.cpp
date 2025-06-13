#include "../includes/webserv.hpp"
#include "../includes/Debugger.hpp"

string readFromFile(string path) // for html files
{
    cout << "hello" << endl;
    try
    {
        std::ifstream file(path.c_str());
        if (file)
        {
            string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            while(content.find('\n') != string::npos)
            {
                int i = content.find('\n');
                content.replace(i, 1, "\r\n");
            }
            return content;
        }
        else
            throw 404;
    }
    catch (...)
    {
        cout << "err" << endl;
    }
    return "";
}


string getStatusMessage(unsigned short code) 
{
    switch (code) {
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden"; 
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "Unknown Error";
    }
}

void WebServ::GET_METHODE(Request req)
{
    const char *testResponse =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "Hello, World!";
    string requestedFile = readFromFile(req.resource);

    
    Response resp;
    
    resp.fullResponse = testResponse + requestedFile;
    
    send(req.cfd, resp.fullResponse.c_str(), resp.fullResponse.size(), 0);

    // cerr << testResponse;

}


// the resource will be starting with a slash
// this function will return the location of the resource in the server node, ex : 
    // GET /auth/login HTTP/1.1
    // location -> /auth
string getLocation(string resource, ServerNode &servNode)
{
    string location = resource;
    size_t i = 1;
    for (; i < resource.size(); i++)
    {
        if (resource[i] == '/')
            break;
    }
    location = resource.substr(0, i);
    if (!exists(servNode.locationDict, location))
    {
        string defaultLocation = "/";
        if (exists(servNode.locationDict, defaultLocation))
            return defaultLocation;
    }
    cout << "location for the request is -> " << location << endl;
    return location;
}


void urlFormParser(string body, map<string, string> &queryParms)
{
    cout << "body -<> " << body << endl;
    return ;
    string temp = "";
    string key = "";
    size_t i = 0;
    while (i < body.size())
    {
        if (body[i] == '=')
        {
            key = temp;
            temp = "";
        }
        else if (body[i] == '&')
        {
            queryParms[key] = temp;
            cout << "key: " << key << " value: " << temp << endl;
            key = "";
            temp = "";
        }
        else
        {
            temp += body[i];
        }
        i++;
    }
    cout << "done " << endl;
    Debugger::printMap("queryParms", queryParms);
}

string getErrorResponse(unsigned short errorCode, string body = "")
{
    string statusMsg = getStatusMessage(errorCode);
    string errorRes;
    errorRes += "HTTP/1.1 " + ushortToStr(errorCode) + " " + statusMsg + " \r\n";
    errorRes +=  "Content-Type: text/html\r\n";
    if (body != "")
    {
        errorRes +=  "Content-Length: " + ushortToStr(statusMsg.size()) + "\r\n\r\n";
        errorRes += statusMsg;
    }
    else
    {
        errorRes +=  "Content-Length: " + ushortToStr(body.size()) + "\r\n\r\n";
        errorRes += body;
    }
}


void handleLogin(Request &req, ServerNode &serv)
{
    
    string body = req.body;

    
    // if (body.find("email=") == body.npos || body.find("password=") == body.npos)
    // {
    //     getErrorResponse(400, "please provide an email and password");
    // }
    map < string , string > queryParams;
    urlFormParser(body, queryParams);
    string email;
    string pw;
}

void WebServ::POST_METHODE(Request req, ServerNode servNode)
{
    short responseCode;
    string responseText;
    string responseBody;
    vector <string> start_line = req.getStartLine();
    ServerNode serv;
    string &location = req.getResource();
    map <string, string> &headers = req.getHeaders();
    string key = "host";
    // Debugger::printMap("headers", headers);
    if (!exists(headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        const char *testResponse =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
       send(req.cfd, testResponse, strlen(testResponse), 0);
       return ;
    }

    string contentType = headers.find("content-type")->second;
    string rootFolder = servNode.root;

    string errorRes;
    
    string locationTarget = getLocation(req.resource, servNode); // will get "/" if the location is not in the server

    if (!exists(servNode.locationDict, locationTarget)) // doesnt exist
    {
        errorRes  = getErrorResponse(404); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }


    LocationNode locationNode = servNode.locationDict.find(locationTarget)->second;
    if (!exists(locationNode.methods, string("POST"))) // methods are stored in upper case
    {
        errorRes  = getErrorResponse(405); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }

    if (contentType == "application/x-www-form-urlencoded")
    {
        handleLogin(req, serv);
    }
    const char *successResponse =
    "HTTP/1.1 404 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 12\r\n"
    "\r\n"
    "Successfull!";
    send(req.cfd, successResponse, strlen(successResponse), 0);

    cout << "BODY -> " << req.body << endl;
    const char *testResponse =
    "HTTP/1.1 404 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 12\r\n"
    "\r\n"
    "bad request!";
    send(req.cfd, testResponse, strlen(testResponse), 0);


    // cerr << testResponse;

}

void WebServ::answer_req(Request req, set <int> servSockets, ServerNode &servNode)
{
    if (req.getReqType() == GET)
        GET_METHODE(req);
    else if (req.getReqType() == POST)
        POST_METHODE(req, servNode);
    // else if (req.getReqType() == DELETE)
    //     DELETE_METHODE(req);

}

void WebServ::sendErrToClient(int clientfd, unsigned short errCode, ServerNode &servNode)
{
    string errorRes;
    const char *generalErrorResponse =
        "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Server Error";
    if (exists(servNode.errorNodes, errCode)) 
    {
        string errorFileStr = servNode.errorNodes.find(errCode)->second;
        cout << errorFileStr << endl;
        if (!validPath(errorFileStr) || !checkFile(errorFileStr, O_RDONLY))
        {
            send(clientfd, generalErrorResponse, strlen(generalErrorResponse), 0);
        }
        else
        {
            ifstream errorFile;
            errorFile.open(errorFileStr.c_str());
            if (errorFile.fail())
            {
                cerr << "Error happened opening the file" << endl;
                send(clientfd, generalErrorResponse, strlen(generalErrorResponse), 0);
                return ;
            }
            string htmlErrFileStr, line;
            while (getline(errorFile, line))
            {
                htmlErrFileStr += line + "\r\n";
            }

            errorRes += "HTTP/1.1 " + ushortToStr(errCode) + " " + getStatusMessage(errCode) + " \r\n";
            errorRes +=  "Content-Type: text/html\r\n";
            errorRes +=  "Content-Length: " + ushortToStr(htmlErrFileStr.size()) + "\r\n\r\n";
            errorRes += htmlErrFileStr;
            send(clientfd, errorRes.c_str(), errorRes.length(), 0);
        }
    }
    else
    {
        send(clientfd, generalErrorResponse, strlen(generalErrorResponse), 0);
    }
}

string removeTrailingCR(string str)
{
    if (!str.empty() && str[str.size() - 1] == '\r')
        str = str.substr(0, str.size() - 1);
    return str;
}

string getHostPort(string host, unsigned short port)
{
    if (host.find (":") == string::npos)
    {
        host += ":" + ushortToStr(port);
    }
    return host;
}

int WebServ::parse_request(int cfd, set <int> servSockets, ServerNode &servNode)
{
    string line;
    Request req;

    req.cfd = cfd;
    ifstream read("Request");
    cout << "*********************************************" << endl;
    
    if (read.fail())
    {
        cerr << "[ " << line << " ]" << "wtf" << endl;
        return ERROR;
    }
    line = "";
    // rfc: if the server find CRLF first
        // at the beginning it should ignore the it."
    while (line.empty())
    {
        if (read.eof() || read.fail())
        {
            criticalErr = true;
            cerr << "Error reading request" << endl;
            return ERROR;
        }

        getline(read, line);
    }
    line = removeTrailingCR(line);
    req.setStartLine(line);
    if (req.isStartLineValid() == ERROR)
    {
        criticalErr = true;
        cout << "invalid Request" << endl;
        return ERROR;
    }
    while (getline(read, line))
    {
        if (read.eof() || read.fail())
        {
            criticalErr = true;
            cerr << "Error reading request" << endl;
            return ERROR;
        }
        line = removeTrailingCR(line);
        if (line.empty())
        {
            break;
        }
        req.setHeaders(line);
    }

    // done parsing ********************



    if (!exists(req.headers, "host"))
    {
        cout << "no host ===> "  << endl;
        Debugger::printMap("req.headers", req.headers);
        sendErrToClient(cfd, 400, servNode);
        return ERROR;
    }
    cout << "line [" << line << "]" << endl;
    
    while (getline(read, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
        {
            break;
        }
        cout << "body [" << line << "]" << endl;
        req.setBody(line);
    }
    cout << "*********************************************" << endl;

    string hostPort = getHostPort(req.headers["host"], servNode.port);
    // Debugger::printServerNode(servNode);
    //    501 (Not Implemented) if the method is
    //    unrecognized or not implemented by the origin server

    cout << "hostPort: " << hostPort << endl;
    if (!exists(hostServMap, hostPort))
    {
        sendErrToClient(cfd, 500, servNode);
        return 0;
    }
    answer_req(req, servSockets, servNode);
    read.close();
    return 0;
}

int bindAndGetSock(struct addrinfo *&res)
{
    struct addrinfo *temp = res;
    int sock;

    while (temp)
    {
        sock = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);
        if (sock == -1)
            continue;
        int flag = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
        {
            cerr << "setsockopt error " << endl;
            return -1;
        }
        if (bind(sock, temp->ai_addr, temp->ai_addrlen) == 0)
            break;
        close(sock);
        temp = temp->ai_next;
    }
    if (temp == NULL)
        return -1;
    return sock;
}

// Convert port to string
string ushortToStr(unsigned short port)
{
    stringstream ss;
    ss << port;
    string portStr = ss.str();
    return portStr;
}

void setupHints(struct addrinfo &hints)
{
    memset(&hints, 0, sizeof(hints)); // lets make our own memset cause forbidden
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
}

// here we will fill the start line, headers and body
bool fillRequest(ofstream &outputFile, int new_sock)
{
    int res;
    char buff[BUFFERSIZE + 1];

    res = recv(new_sock, buff, BUFFERSIZE, 0);
    while (res > 0 && res == BUFFERSIZE)
    {
        buff[res] = '\0';
        outputFile.write(buff, res);
        res = recv(new_sock, buff, BUFFERSIZE, 0);
    }
    if (res > 0)
        outputFile.write(buff, res);
    outputFile.close();

    ifstream read("Request");
    string line;
    cout << "*********************************************" << endl;
    getline(read, line);
    if (read.fail())
    {
        cerr << "[ " << line << " ]" << endl;
        return ERROR;
    }
    cout << line << endl;
    while (getline(read, line))
    {
        if(line.empty())
        {
            break;
        }
        cout << line << endl;
    }
    while(getline(read, line))
        cout << (line);
    cout << "*********************************************" << endl;

    return (1);

}

int WebServ::server()
{
    // Debugger::printCompleteDebugInfo(servNodes, hostServMap, servNameServMap);
    std::map<std::string, ServerNode>::iterator servIt = hostServMap.begin();
    struct epoll_event ev;
    struct addrinfo *res;
    set <int> servSockets;
    map <int, ServerNode> servSocketMap;
    int sock;
    int epollfd;
    struct addrinfo hints;
    // setup the server
    epollfd = epoll_create1(0);
    if (epollfd == -1)
        {cout << "epoll_create1 failed" << endl; criticalErr = true; return ERROR;}
    while (servIt != hostServMap.end()) // for every server in the config file
    {
        ServerNode serv = servIt->second;
        setupHints(hints);
        string host = serv.hostStr;
        string portStr = ushortToStr(serv.port);
        if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) == -1)
            {cout << "could not getaddrinfo.. aborting." << endl; criticalErr = true; freeaddrinfo(res); ; return ERROR;}
        sock = bindAndGetSock(res);
        if (sock == -1)
            {cout << "could not bind.. aborting." << endl; criticalErr = true; return ERROR;}
        if (listen(sock, 10) == -1)
            {cout << "could not listen.. aborting." << endl; criticalErr = true; return ERROR;}
        cout << "Server listening on " << host << ":" << portStr << endl;
        freeaddrinfo(res);
        ev.events = EPOLLIN;
        ev.data.fd = sock;
        servSockets.insert(sock);
        servSocketMap[sock] = servIt->second;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1)
            {cout << "epoll_ctl failed" << endl; criticalErr = true; return ERROR;}
        servIt++;
    }
    serverLoop(epollfd, ev, servSockets, servSocketMap);
    return 0;
}

int WebServ::serverLoop(int epollfd, struct epoll_event ev, set <int> servSockets, map <int, ServerNode> &servSocketMap)
{
    int maxEvents = 1024;
    struct epoll_event events[maxEvents];
    socklen_t clientAddrSize = sizeof(struct sockaddr);
    struct sockaddr clientAddr;
    map <int, int> clientServMap;
    while (1)
    {
        int nfds = epoll_wait(epollfd, events, maxEvents, -1);
        if (nfds == -1)
        {
            cerr << "epoll_wait failed" << endl;
            criticalErr = true;
            return ERROR;
        }
        for (int i = 0 ; i < nfds; i++)
        {
            int readyFd = events[i].data.fd;
            if (exists(servSockets, readyFd))
            {
                // its  a  server, which means we have a new client, add it to the clients being monitored
                int client = accept(readyFd, &clientAddr, &clientAddrSize);  // check if fails
                if (client == -1)
                {
                    switch(errno) {
                        case EAGAIN:
                            cout << "No pending connections (non-blocking)" << endl;
                            break;
                        case EINTR:
                            cout << "Interrupted system call" << endl;
                            break;
                        case EMFILE:
                            cout << "Process file descriptor limit reached" << endl;
                            break;
                        case ENFILE:
                            cout << "System file descriptor limit reached" << endl;
                            break;
                        case ENOTSOCK:
                            cout << "Not a socket" << endl;
                            break;
                        case EOPNOTSUPP:
                            cout << "Socket not listening" << endl;
                            break;
                    }
                    cerr << "accept failed" << endl;
                    continue; // continue to the next iteration
                }
                clientServMap[client] = readyFd;
                ev.events = EPOLLIN;
                ev.data.fd = client;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, client, &ev); // check if fails
            }
            else
            {
                // its a client that we were monitoring (added him in the block above)  (new fd so we must remeber him)
                ofstream write1("Request");
                int servFd = clientServMap[readyFd];
                ServerNode serv = servSocketMap[servFd];
                fillRequest(write1, readyFd);
                write1.close();
                parse_request(readyFd, servSockets, serv);
                clientServMap.erase(readyFd);
                epoll_ctl(epollfd, EPOLL_CTL_DEL, readyFd, NULL);  // check if fails
                close(readyFd);
            }
        }
    }
}






