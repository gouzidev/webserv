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


string getHostPort(string host, unsigned short port)
{
    if (host.find (":") == string::npos)
    {
        host += ":" + ushortToStr(port);
    }
    return host;
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
    size_t endPos = headersStr.find("\r\n\r\n");
    if (endPos == string::npos)
        throw RequestException("headers not found in request", 400, serv);
    
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
long extractContentLen(Request &req, ServerNode &serv, bool &error)
{
    cout << "here looking for content-length" << endl;
    if (!exists(req.headers, "content-length"))
        throw RequestException("could not find 'content-length' header", 400, serv);
    string contentLenStr = req.headers.find("content-length")->second;
    contentLenStr = trimWSpaces(contentLenStr);
    if (!strAllDigit(contentLenStr))
        throw RequestException("content-length: '" + contentLenStr + "' is not a number", 400, serv);

    istringstream contentLenStream (contentLenStr);
    if (contentLenStream.fail())
        throw RequestException("failed to parse content-length: '" + contentLenStr + "' it is not a number", 400, serv);
    long contentLen;
    contentLenStream >> contentLen;
    return contentLen;
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
                        cout << "start line is [ " << startLine << " ]" << endl;
                        req.isStartLineValid();
                        req.fillHeaders(readyFd, rest);
                        Debugger::printMap("headers\n", req.headers);
                        if (!exists(req.headers, "host"))
                        {
                            cout << "wtf3" << endl;
                            sendErrToClient(req.cfd, 400, serv);
                            throw RequestException("could not find 'host' header", 400, serv);
                        }
                        string hostPort = getHostPort(req.headers["host"], serv.port);
                        if (!exists(hostServMap, hostPort))
                        {
                            sendErrToClient(req.cfd, 500, serv);
                            throw ServerException("host:port not found in server config", 500);
                        }
                        
                        if (req.getReqType() == POST)
                        {
                            if (exists(req.headers, "content-length"))
                            {
                                cout << "wtf4" << endl;
                                sendErrToClient(req.cfd, 400, serv);
                                throw RequestException("could not find 'content-length'", 400, serv);
                            }
                            long contentLen = extractContentLen(req, serv, error);
                            cout << "Content-Length in bytes is " << contentLen  << endl;
                            if (error || contentLen < 0)
                            {
                                cout << "wtf5" << endl;
                                sendErrToClient(req.cfd, 400, serv);
                                throw RequestException("content length is not valid", 400, serv);
                            }
                            if (contentLen > 10000000)
                            {
                                sendErrToClient(req.cfd, 413, serv);
                                throw RequestException("content length is too large for the server", 413, serv);
                            }
                            req.body = rest;
                            postMethode(req, serv);
                        }
                        if (req.getReqType() == GET)
                        {
                            getMethode(req, serv);
                        }
                    }

                    catch (RequestException &requestException)
                    {
                        std::cerr << requestException.what() << '\n';
                        close(readyFd);
                        clientServMap.erase(readyFd);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, readyFd, NULL); //
                        continue; // continue to the next iteration
                    }
                    catch (WebServException &webservException)
                    {
                        std::cerr << webservException.what() << '\n';
                        close(readyFd);
                        clientServMap.erase(readyFd);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, readyFd, NULL); //
                        continue; // continue to the next iteration
                    }
                    
                    
                    
                    // its a client that we were monitoring (added him in the block above) (new fd so we must remeber him)

                    // cout << "rest is -> '" << rest << "'" << endl;
    
                    // else if (bytesRead < BUFFSIZE) // done (read (in past) all data)
                    // {
                    //     cout << "Request received, processing..." << endl;
                    //     cout << "Bytes read: " << bytesRead << endl;
                    //     ofstream write1("Request");
                    //     int servFd = clientServMap[readyFd];
                    //     ServerNode serv = servSocketMap[servFd];
                    //     write1.write(peek, bytesRead);
                    //     fillRequest(write1, readyFd);
                    //     parseRequest(readyFd, servSockets, serv);
                    //     clientServMap.erase(readyFd);
                    //     epoll_ctl(epollfd, EPOLL_CTL_DEL, readyFd, NULL);  // check if fails
                    //     write1.close();
                    //     close(readyFd);
                    // }
                    
                    // else // request is larger then BUFFSIZE so we must while loop recv
                    // {
                    //     cout << "Request too large, closing connection" << endl;
                    //     ofstream write1("Request");
                    //     write1 << "HTTP/1.1 413 Payload Too Large\r\n\r\n";
                    //     write1.close();
                    // }
                    // close(readyFd);
    
    
                }
            }
    
    
            time_t now = time(0);
            if (now - lastCleanup > 300)
            {
                auth->cleanUpSessions();
                lastCleanup = now;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        

    }
    return 0;
}

