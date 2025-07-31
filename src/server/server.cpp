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

void WebServ::server()
{
    std::map<std::string, ServerNode>::iterator servIt = hostServMap.begin();
    struct addrinfo *res;
    int sock;
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
        if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
            throw NetworkException("epoll_ctl failed", 500);
        servIt++;
    }
    serverLoop();
}
   

bool cleanFd(int fdToClose, map<int, int> &clients, int epollfd)
{
    close(fdToClose);
    clients.erase(fdToClose);
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


void Request::setHeaders(string &buff, size_t startLineEnd, size_t headersEnd)
{
    size_t i = startLineEnd + 2;
    while (i < headersEnd)
    {
        size_t nextLineEnd = buff.find("\r\n", i);
        if (nextLineEnd > headersEnd)
            break;
        
        string headerLine = buff.substr(i, nextLineEnd - i);
        setHeader(headerLine);

        size_t colonPos = headerLine.find(':');
        if (colonPos != string::npos)
        {
            string key = headerLine.substr(0, colonPos);
            string val = headerLine.substr(colonPos + 1);
            key = trimWSpaces(key);
            val = trimWSpaces(val);
            transform(key.begin(), key.end(), key.begin(), ::tolower);
            
            if (key == "content-length")
                contentLen = atol(val.c_str());
            else if (key == "transfer-encoding" && val == "chunked") 
                isChunked = true;

        }
        i = nextLineEnd + 2;
    }
}

void Request::verifyHeaders(Client &client)
{
    Request &req = client.request;
    string &buff = client.requestBuff;
    size_t headersEnd;
    map <string, string> &headers = req.headers;

    if (req.reqType == "POST")
    {
        if (!exists(headers, "content-length") && !exists(headers, "transfer-encoding"))
        {
            cerr << "bad request, no content type and no transfer encoding" << endl;
            throw HttpException(413, client); // payload too large
        }
        if (exists(headers, "content-type"))
        {
            string &contentType = headers.at("content-type");
            if (contentType == "text/plain")
                req.contentType = textPlain_t;
            else if (contentType == "application/x-www-form-urlencoded")
                req.contentType = wwwURLEncoded_t;
            else if (startsWith(contentType, "multipart/form-data; boundary="))
                req.contentType = multipartFormData_t;
            else
                req.contentType = OTHER_t;
        }
        else if (exists(headers, "content-type") && req.contentLen > 0) // bad request
        {
            throw HttpException(400, client); // no content-type but body is present
        }
    }
}

bool WebServ::parseHeaders(Client &client)
{
    Request &req = client.request;
    string &buff = client.requestBuff;
    size_t headersEnd;
    map <string, string> &headers = req.headers;


    size_t startLineEnd = buff.find("\r\n");
    if (startLineEnd == string::npos)
        return false; // still not done -> read more
    if (req.startLine.empty())
    {
        string startLine = buff.substr(0, startLineEnd);
        req.setStartLine(startLine);
        req.isStartLineValid();
    }
    headersEnd = buff.find("\r\n\r\n");
    if (headersEnd == string::npos)
        return false; // still not done -> read more

    req.contentLen = -1;
    req.isChunked = false;

    req.setHeaders(buff, startLineEnd, headersEnd);
    req.verifyHeaders(client); // currently verifying post only

    size_t bodyStart = headersEnd + 4;
    if (bodyStart < buff.size())
    {
        client.requestBuff = buff.substr(bodyStart);
        client.bodyBytesRead = client.requestBuff.size();
    }
    else
    {
        client.requestBuff = "";
        setClientReadyToRecvData(client, false);
        return false;
    }

    client.clientState = READING_BODY;
    // returning true means we parsed all the headers (we also changed the state)
    return true;
}

bool WebServ::parseBody(Client &client) // returning true means task is done
{

    Request &req = client.request;
    string &buff = client.requestBuff;
    map <string, string> &headers = req.headers;
    ServerNode &serv = req.serv;
    string locationTarget = getLocation(req, serv); // will get "/" if the location is not in the server--
    LocationNode locationNode = serv.locationDict.find(locationTarget)->second;
    if (locationNode.isProtected)
    {
        string sessionKey = req.extractSessionId();
        if (!auth->isLoggedIn(sessionKey))
        {
            cout << "un auth" << endl;
            throw HttpException(401, client);
        }
    }
    if (req.reqType == "POST")
    {
        // first lets handle super short requests like for /login ...
        if (req.contentType == wwwURLEncoded_t)
        {
            if (locationTarget == "/login")
                handleLogin(client);
            else if (locationTarget == "/signup")
                handleSignup(client);
            else if (locationTarget == "/logout")
                handleLogout(client);
            else
                handleFormData(client);
            setClientReadyToRecvData(client, false);
        }
        else if (req.contentType == textPlain_t)
        {
            handleFormData(client);
        }
        else if (req.contentType == multipartFormData_t)
        {
            handleUpload(client, locationNode);
        }
        else
        {
            handleFormData(client);
        }
        return true;
    }
    return false;
}

void WebServ::setClientReadyToRecvData(Client &client, bool error)
{
    ev.data.fd = client.cfd;
    ev.events = EPOLLOUT | EPOLLET; // we are done reading,
    epoll_ctl(epollfd, EPOLL_CTL_MOD, client.cfd, &ev);
    if (error)
        client.clientState = WRITING_ERROR;
    else
        client.clientState = WRITING_RESPONSE;
}

void WebServ::handleClientRead(Client &client)
{
    ssize_t bytesRead;
    char buff[BUFFSIZE];

    bytesRead = recv(client.cfd, buff, BUFFSIZE, 0);

    if (bytesRead <= 0)
    {
        if (bytesRead == 0)
            client.clientState = WRITING_RESPONSE;
        return cleanClient(client);
    }

    client.requestBuff.append(buff, bytesRead);

    bool made_progress = true;
    while (made_progress)
    {
        made_progress = false;
        switch (client.clientState)
        {
            case READING_HEADERS:
                if (parseHeaders(client))
                {
                    if(parseBody(client))
                        made_progress = true;
                }
                break;
            case READING_BODY:
                if (parseBody(client))
                    made_progress = true;
                break;

            default:
                break;
        }
    }
}

void WebServ::handleClientWrite(Client &client)
{
    if (client.clientState == WRITING_RESPONSE)
    {
        if (client.request.getReqType() == "GET")
            getMethode(client);
    }
    else if (client.clientState == WRITING_DONE) // there is something in the buffer that needs to be sent (after post only .. maybe)
    {

    }
    else if (client.clientState == WRITING_ERROR) // the responseBuff will already be filled just send it
    {
        // handle errors aka send or write
        ssize_t err = send(client.cfd, client.responseBuff.c_str(), client.responseBuff.size(), 0);
        if (err == -1)
            cleanClient(client);
        client.clientState = SENDING_DONE;
    }
    // else if (client.clientState == SENDING_DONE)
    // {
        
    // }
}

void WebServ::cleanClient(Client &client)
{
    
    if (client.cfd != -1)
        close(client.cfd);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, client.cfd, NULL);
    if (exists(clients, client.cfd))
        clients.erase(client.cfd);
}

int WebServ::serverLoop()
{
    struct epoll_event events[MAXEVENTS];
    while (1)
    {
        int nfds = epoll_wait(epollfd, events, MAXEVENTS, -1);
        if (nfds == -1)
            throw NetworkException("epoll_wait failed", 500);
        for (int i = 0 ; i < nfds; i++)
        {
            int readyFd = events[i].data.fd;
            // if the readyFd is in server Sockets, readyFd is a server fd, which means we have a new client, add it to the clients being monitored
            if (exists(servSockets, readyFd)) // here readyFd is a server socket
            {
                int serverSock = readyFd;
                struct sockaddr clientAddr;
                socklen_t clientAddrSize = sizeof(struct sockaddr);
                while (true)
                {
                    int clientSock = accept(serverSock, &clientAddr, &clientAddrSize);  // check if fails
                    if (clientSock == -1)
                    {
                        if (errno == EWOULDBLOCK || errno == EAGAIN)
                            break; // we done accepting
                        throw NetworkException("accept failed", 500);
                    }
                    if (fcntl(clientSock, F_SETFL, O_NONBLOCK) == -1)
                        throw NetworkException("fcntl failed", 500);
                    ServerNode &serv = servSocketMap[serverSock];
                    Request clientReq(serv);
                    Client newClient = Client(clientReq, clientSock, serverSock);
                    clients.insert(make_pair(clientSock, newClient));
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = clientSock;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientSock, &ev) == -1)
                        throw NetworkException("epoll_ctl failed", 500);
                }
            }
            else // client already exists
            {
                int clientSock = readyFd;
                Client &client = clients.at(clientSock);
                try
                {
                    if (events[i].events & EPOLLIN)
                        handleClientRead(client);
                    else if (events[i].events & EPOLLOUT)
                        handleClientWrite(client);
                }
                catch (HttpException &e)
                {
                    cout << "catched an exception of error -> " << e.getErrorCode() << endl;
                    // here we will catch http exceptions. we will have the http code in obj -> e
                        // we will use that to generate an error response that will be sent in next event with handleClientWrite in WRITING_ERROR state.
                        // we have a client object that we can use to send the response inside the error obj

                        Client &client  = e.getClient();
                        client.responseBuff = getSmallErrPage(e.getErrorCode());
                        setClientReadyToRecvData(client, true);
                }
            }
            if (exists(clients, readyFd) && clients.at(readyFd).clientState == SENDING_DONE)
            {
                cleanClient(clients.at(readyFd));
            }
        }
    }
    return 0;
}

