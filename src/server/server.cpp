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


bool isLarge(char *buff, size_t buffSize, bool &error)
{
    buff[buffSize] = '\0';
    string buffStr = string(buff, buffSize);
    cout  << "buff substr: " << buffStr.substr(0, 100) << endl; // print only the first 100 chars for debugging
    size_t startPos = buffStr.find("Content-Length:");
    if (startPos == string::npos)
        error = true;
    else
    {
        size_t endPos = buffStr.find("\r\n", startPos);
        string contentLenStr = buffStr.substr(startPos + 15, endPos - startPos - 15);
        contentLenStr = trimWSpaces(contentLenStr);
        if (!strAllDigit(contentLenStr))
        {
            cout << "Error, Content-Length '" << contentLenStr << "' not a number" << endl;
            error = true;
            return false;
        }

        istringstream contentLenStream (contentLenStr);
        if (contentLenStream.fail())
        {
            cout << "Error, Content-Length is not a number (fail)" << endl;
            error = true;
            return false;
        }

        long contentLen;
        contentLenStream >> contentLen;
        cout << "Content-Length is " << contentLen << endl;
        if (contentLen > 5000000) // 5m
            return true;
        return false;
    }
    return false;
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
        cout << "hello 149" << endl;
        throw NetworkException("recv failed", 500);
    }
    while (bytesRead > 0)
    {
        if (c == '\r')
        {
            bytesRead = recv(fd, &c, 1, 0);
            if (bytesRead < 0)
            {
                cout << "hello 158" << endl;
                throw NetworkException("recv failed", 500);
            }
            if (c == '\n')
                break;
        }
        line += c;
        bytesRead = recv(fd, &c, 1, 0);
        if (bytesRead < 0)
        {
            cout << "hello 169" << endl;
            
            throw NetworkException("recv failed", 500);
        }
    }
    return line;
}

bool Request::fillHeaders(int fd, string &rest)
{
    char buff[BUFFSIZE + 1];
    size_t bytesRead = recv(fd, buff, BUFFSIZE, 0);
    buff[bytesRead] = '\0';
    
    if (bytesRead == -1)
        throw NetworkException("recv failed or headers are too large", 500);
    
    string headersStr = string(buff, bytesRead);
    // cout << "Headers string: {{{" << headersStr << "}}}" <<  endl; // print only the first 100 chars for debugging
    size_t endPos = headersStr.find("\r\n\r\n");
    if (endPos == string::npos)
        throw RequestException("headers not found in request", 400, *this);
    
    size_t i = 0;
    while (i < endPos)
    {
        string key;
        string val;

        // Find the end of current line FIRST
        size_t nextNlPos = headersStr.find("\r\n", i);
        if (nextNlPos == string::npos || nextNlPos > endPos) {
            break; // No more complete lines
        }
        
        // Now find colon ONLY within this line
        size_t sepPos = headersStr.find(':', i);
        if (sepPos == string::npos || sepPos >= nextNlPos) {
            // No colon in this line, skip it
            i = nextNlPos + 2;
            continue;
        }

        key = headersStr.substr(i, sepPos - i);
        val = headersStr.substr(sepPos + 1, nextNlPos - sepPos - 1);

        key = trimWSpaces(key);
        val = trimWSpaces(val);
        
        // Convert key to lowercase
        transform(key.begin(), key.end(), key.begin(), ::tolower);
        headers[key] = val;
        
        // Move to next line
        i = nextNlPos + 2;  // +2 to skip \r\n
    }
    
    rest = headersStr.substr(endPos + 4); // +4 to skip \r\n\r\n
    return true;
}



// steps :
    // 1. Read start line
    // 2. Read headers 
    // 3. Read body (if POST)
    // 4. Return complete request
bool readCompleteRequest(int fd, Request &req, string &error)
{

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
                        string startLine = readLine(readyFd, error);
                        ServerNode serv = servSocketMap[servFd];
                        Request req(serv);
                        req.cfd = readyFd;
                        req.setStartLine(startLine);
                        req.isStartLineValid();
                        req.fillHeaders(readyFd, rest);
                        req.body = rest;
                        if (!exists(req.headers, "host"))
                        {
                            sendErrToClient(req.cfd, 400, serv);
                            throw RequestException("could not find 'host' header", 400, req);
                        }
                        cout << req.headers["host"] << endl;
                        string hostPort = getHostPort(req.headers["host"], serv.port);
                        if (!exists(hostServMap, hostPort))
                            throw RequestException("host:port not recognizable", 500, req);
                        
                        if (req.getReqType() == POST)
                        {
                            postMethode(req, serv);
                            close(readyFd);
                            clientServMap.erase(readyFd);
                            epoll_ctl(epollfd, EPOLL_CTL_DEL, readyFd, NULL);
                            continue;
                        }
                        else if (req.getReqType() == GET)
                        {
                            getMethode(req, serv);
                            close(readyFd);
                            clientServMap.erase(readyFd);
                            epoll_ctl(epollfd, EPOLL_CTL_DEL, readyFd, NULL);
                            continue;
                        }
                    }

                    catch (RequestException &requestException)
                    {
                        Request req = requestException.getReq();
                        short errorCode = requestException.getErrorCode();
                        cout << "RequestException caught: " << requestException.what() << endl;
                        sendErrToClient(req.cfd, errorCode, req.serv);
                        close(req.cfd);
                        clientServMap.erase(req.cfd);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, req.cfd, NULL);
                        continue; // continue to the next iteration
                    }
                }
            }
    
    
            time_t now = time(0);
            if (now - lastCleanup > 300)
            {
                auth->cleanUpSessions();
                lastCleanup = now;
            }
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

