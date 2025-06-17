#include "../../includes/webserv.hpp"
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

int WebServ::serverLoop(int epollfd, struct epoll_event ev, set <int> servSockets, map <int, ServerNode> &servSocketMap)
{
    bool error;
    int maxEvents = 1024;
    struct epoll_event events[maxEvents];
    socklen_t clientAddrSize = sizeof(struct sockaddr);
    struct sockaddr clientAddr;
    map <int, int> clientServMap;
    time_t lastCleanup = time(0);
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
                char peek[BUFFSIZE + 1]; // 64k byte
                ssize_t bytesRead = recv(readyFd, peek, BUFFSIZE, 0);
                if (bytesRead == -1)
                {
                    error = true;
                    cout << "recv failed, closing connection" << endl;
                }
                else if (bytesRead < BUFFSIZE || !isLarge(peek, BUFFSIZE, error))
                {
                    cout << "Request received, processing..." << endl;
                    cout << "Bytes read: " << bytesRead << endl;
                    ofstream write1("Request");
                    int servFd = clientServMap[readyFd];
                    ServerNode serv = servSocketMap[servFd];
                    write1.write(peek, bytesRead);
                    fillRequest(write1, readyFd);
                    parseRequest(readyFd, servSockets, serv);
                    clientServMap.erase(readyFd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, readyFd, NULL);  // check if fails
                    write1.close();
                    close(readyFd);
                }
                else
                {
                    cout << "Request too large, closing connection" << endl;
                    ofstream write1("Request");
                    write1 << "HTTP/1.1 413 Payload Too Large\r\n\r\n";
                    write1.close();
                }
                close(readyFd);
            }
        }


        time_t now = time(0);
        if (now - lastCleanup > 300)
        {
            auth->cleanUpSessions();
            lastCleanup = now;
        }
    }
}

