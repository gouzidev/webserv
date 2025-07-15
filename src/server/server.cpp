#include "../../includes/webserv.hpp"
#include "../../includes/Exceptions.hpp"
#include "../../includes/Debugger.hpp"


void setupHints(struct addrinfo &hints)
{
    memset(&hints, 0, sizeof(hints)); // lets make our own memset cause forbidden
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
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
            throw NetworkException("setsockopt failed", 500);
        if (bind(sock, temp->ai_addr, temp->ai_addrlen) == 0)
            break;
        close(sock);
        temp = temp->ai_next;
    }
    if (temp == NULL)
        throw NetworkException("bind failed", 500);
    return sock;
}


string getHostPort(string host, unsigned short port)
{
    size_t colPos = host.find(":");
    string hostStr = host.substr(0, colPos);
    if (hostStr == "localhost")
        host = "127.0.0.1:" + ushortToStr(port);
    if (colPos == string::npos)
    {
        host += ":" + ushortToStr(port);
    }
    return host;
}


int WebServ::server()
{
    // servNameServMap
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
        throw NetworkException("epoll_create1 failed", 500);
    while (servIt != hostServMap.end()) // for every server in the config file
    {
        ServerNode serv = servIt->second;
        setupHints(hints);
        string host = serv.hostIp;
        string portStr = ushortToStr(serv.port);
        if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) == -1)
        {
            freeaddrinfo(res);
            throw NetworkException("getaddrinfo failed", 500);
        }
        sock = bindAndGetSock(res);
        if (listen(sock, 10) == -1)
            throw NetworkException("listen failed", 500);
        cout << "Server listening on " << host << ":" << portStr << endl;
        freeaddrinfo(res);
        ev.events = EPOLLIN;
        ev.data.fd = sock;
        servSockets.insert(sock);
        servSocketMap[sock] = servIt->second;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1)
            throw NetworkException("epoll_ctl failed", 500);
        servIt++;
    }
    serverLoop(epollfd, ev, servSockets, servSocketMap);
    return 0;
}

// long extractContentLength(string &headerData, char *peek, long buffSize, bool &done)
// {

// }

string readLine(int fd, bool &error)
{
    string line = "";
    char c;
    size_t bytesRead = 1;

    bytesRead = recv(fd, &c, 1, 0);
    if (bytesRead <= 0)
    {
        throw NetworkException("recv failed", 500);
    }
    while (bytesRead > 0)
    {
        if (c == '\r')
        {
            bytesRead = recv(fd, &c, 1, 0);
            if (bytesRead < 0)
            {
                throw NetworkException("recv failed", 500);
            }
            if (c == '\n')
                break;
        }
        line += c;
        bytesRead = recv(fd, &c, 1, 0);
        if (bytesRead <= 0)
        {
            throw NetworkException("recv failed", 500);
        }
    }
    return line;
}

bool Request::fillHeaders(int fd)
{
    char buff[BUFFSIZE + 1];
    
    size_t totalRead = 0;
    bool completedHeaders = false;
    string fullRequest = "";
    size_t headersEnd = -1;
    long expectedContentLength = 0;

    while (!completedHeaders && totalRead < BUFFSIZE * 2)
    {
        ssize_t bytesRead = recv(fd, buff, BUFFSIZE, 0);
        if (bytesRead <= 0) {
            throw NetworkException("recv failed", 500);
        }
        fullRequest.append(buff, bytesRead);
        
        totalRead += bytesRead;
        headersEnd = fullRequest.find("\r\n\r\n");
        if (headersEnd != string::npos)
        {
            completedHeaders = true;
        }
    }
    if (completedHeaders && headersEnd != -1)
        body = fullRequest.substr(headersEnd + 4);
    else
        throw NetworkException("Incomplete headers received", 400);

    size_t startLineEnd = fullRequest.find("\r\n");
    if (startLineEnd == string::npos) {
        throw NetworkException("Invalid start line", 400);
    }
    string startLine = fullRequest.substr(0, startLineEnd);
    setStartLine(startLine);
    isStartLineValid();
    
    // Parse headers
    size_t i = startLineEnd + 2;
    while (i < headersEnd) {
        size_t nextLineEnd = fullRequest.find("\r\n", i);
        if (nextLineEnd == string::npos || nextLineEnd > headersEnd) {
            break;
        }
        
        string headerLine = fullRequest.substr(i, nextLineEnd - i);
        setHeaders(headerLine);
        
        size_t colonPos = headerLine.find(':');
        if (colonPos != string::npos) {
            string key = headerLine.substr(0, colonPos);
            string val = headerLine.substr(colonPos + 1);
            key = trimWSpaces(key);
            val = trimWSpaces(val);
            transform(key.begin(), key.end(), key.begin(), ::tolower);
            
            if (key == "content-length")
                expectedContentLength = atol(val.c_str());
        }
        i = nextLineEnd + 2;
    }
    
    size_t bodyStart = headersEnd + 4;
    size_t bodyAlreadyRead = 0;

    if (bodyStart < fullRequest.size())
    {
        bodyAlreadyRead = fullRequest.size() - bodyStart;
        this->body = fullRequest.substr(bodyStart);
    }
    else
        this->body = "";
    
    if (bodyAlreadyRead < expectedContentLength) // we have some content length that isnt read
    {
        size_t remainingBodyBytes = expectedContentLength - bodyAlreadyRead;
        while (remainingBodyBytes > 0)
        {
            size_t toRead = min((size_t) BUFFSIZE, remainingBodyBytes);
            ssize_t bytesRead = recv(fd, buff, toRead, 0);
            if (bytesRead <= 0)
                throw NetworkException("recv failed", 500);
            this->body.append(buff, bytesRead);
            remainingBodyBytes -= bytesRead;
        }
    }
    cout << "body ->  " << endl << endl << "{{"  << endl << body << endl  << "}}" << endl << endl;
    return true;
}
    

bool cleanFd(int fdToClose, map<int, int> &clientServMap, int epollfd)
{
    close(fdToClose);
    clientServMap.erase(fdToClose);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fdToClose, NULL);

    return true;
}

bool checkSessions(time_t &lastCleanup, Auth *auth)
{
    time_t now = time(0);
    if (now - lastCleanup > 300)
    {
        auth->cleanUpSessions();
        lastCleanup = now;
    }
    return true;
}


bool parseChuncked()
{
    return true;
}

int WebServ::serverLoop(int epollfd, struct epoll_event ev, set <int> servSockets, map <int, ServerNode> &servSocketMap)
{
    bool error = false;
    bool done;
    int maxEvents = 1024;
    struct epoll_event events[maxEvents];
    socklen_t clientAddrSize = sizeof(struct sockaddr);
    struct sockaddr clientAddr;
    long contentLength;
    map <int, int> clientServMap;
    time_t lastCleanup = time(0);
    while (1)
    {
        try
        {
            int nfds = epoll_wait(epollfd, events, maxEvents, -1);
            if (nfds == -1)
                throw NetworkException("epoll_wait failed", 500);
            for (int i = 0 ; i < nfds; i++)
            {
                int readyFd = events[i].data.fd;
                if (exists(servSockets, readyFd))
                {
                    // its  a  server, which means we have a new client, add it to the clients being monitored
                    int client = accept(readyFd, &clientAddr, &clientAddrSize);  // check if fails
                    if (client == -1)
                        throw NetworkException("accept failed", 500);
                    clientServMap[client] = readyFd;
                    ev.events = EPOLLIN;
                    ev.data.fd = client;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client, &ev) == -1)
                        throw NetworkException("epoll_ctl failed", 500);
                }
                else
                {
                    try
                    {
                        string rest;
                        int servFd = clientServMap[readyFd];
                        ServerNode serv = servSocketMap[servFd];
                        Request req(serv);
                        req.cfd = readyFd;

                        req.fillHeaders(readyFd);
                        req.setCookies();
                        if (!exists(req.headers, "host"))
                        {
                            sendErrPageToClient(req.cfd, 400, serv);
                            throw RequestException("could not find 'host' header", 400, req);
                        }
                        string hostPort = getHostPort(req.headers["host"], serv.port);
                        if (!exists(hostServMap, hostPort))
                            throw RequestException("host:port not recognizable", 500, req);
                        if (req.getReqType() == POST)
                        {
                            postMethode(req, serv);
                            cleanFd(readyFd, clientServMap, epollfd);
                            continue;
                        }
                        else if (req.getReqType() == GET)
                        {
                            getMethode(req, serv);
                            cleanFd(readyFd, clientServMap, epollfd);
                            continue;
                        }
                    }

                    catch (RequestException &requestException)
                    {
                        Request req = requestException.getReq();
                        short errorCode = requestException.getErrorCode();
                        cout << "RequestException caught: " << requestException.what() << endl;
                        sendErrPageToClient(readyFd, errorCode, req.serv);
                        cleanFd(readyFd, clientServMap, epollfd);
                        continue; // continue to the next iteration
                    }
                }
            }
    
            checkSessions(lastCleanup, auth);
        }
        // handle epoll_wait error
        catch(const std::exception& e)
        {
            cout << "Exception caught in server loop: " << e.what() << endl;
            std::cerr << e.what() << '\n';
            return ERROR;
        }
        

    }
    return 0;
}

