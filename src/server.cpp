#include "../includes/webserv.hpp"
#include "../includes/Debugger.hpp"

void WebServ::GET_METHODE(Request req)
{
    const char *testResponse =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "Hello, World!";

    send(req.cfd, testResponse, strlen(testResponse), 0);

    // cerr << testResponse;

}

void WebServ::POST_METHODE(Request req)
{
    vector <string> start_line = req.getStartLine();
    ServerNode serv;
    string &location = req.getResource();
    cout << "location -> " << location << endl;
    map <string, string> &headers = req.getHeaders();
    string key = "host";
    Debugger::printMap("headers", headers);
    if (!exists(headers, "host") || !exists(headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        const char *testResponse =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
       send(req.cfd, testResponse, strlen(testResponse), 0);
    }
    else
    {
        const char *testResponse =
        "HTTP/1.1 404 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "bad request!";
        send(req.cfd, testResponse, strlen(testResponse), 0);
}


    // cerr << testResponse;

}

void WebServ::answer_req(Request req)
{

    if (req.getReqType() == GET)
        GET_METHODE(req);
    else if (req.getReqType() == POST)
        POST_METHODE(req);
    // else if (req.getReqType() == DELETE)
    //     DELETE_METHODE(req);

}

int WebServ::parse_request(int cfd)
{

    string line;
    Request req;

    req.cfd = cfd;
    ifstream read("Request");
    cout << "*********************************************" << endl;
    getline(read, line);
    if (read.fail())
    {
        cerr << "[ " << line << " ]" << "wtf" << endl;
        return ERROR;
    }
    cout << line << endl;
    req.setStartLine(line);
    if (req.isStartLineValid() == 1)
    {
        cout << "invalid Request" << endl;
        return ERROR;
    }
    while(getline(read, line))
    {
        if(line.empty())
        {
            break;
        }
        req.setHeaders(line);
        cout << line << endl;
    }
    while(getline(read, line))
        req.setBody(line);
    cout << "*********************************************" << endl;



    answer_req(req);
    read.close();
    return 0;
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
        outputFile.write(buff, res);
        res = recv(new_sock, buff, BUFFERSIZE, 0);
        cout << "recv -> " << res << endl;
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
    std::map<std::string, ServerNode>::iterator servIt = hostServMap.begin();
    int maxEvents = 1024;
    struct epoll_event ev;
    struct epoll_event events[maxEvents];
    int sock;

    // setup the server
    while (servIt != hostServMap.end())
    {
        struct addrinfo hints;
        struct addrinfo *res;
        memset(&hints, 0, sizeof(hints)); // lets make our own memset cause forbidden
        hints.ai_family = AF_INET;
        hints.ai_protocol = SOCK_STREAM;
        ServerNode serv = servIt->second;
        unsigned short port = serv.port;
        string host  = serv.host;
        
        sockaddr_in ss;
        ss.sin_family = AF_INET;
        ss.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        ss.sin_port = htons(port);
        if (getaddrinfo(host.c_str(), "http" ,&hints, &res) == -1)
            {cout << "getaddrinfo failed" << endl; criticalErr = true; return ERROR;}
        
        struct addrinfo *temp = res;
        while (temp)
        {
            temp = temp->ai_next;
            sock = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);
            if (sock == -1)
                continue;
            if (bind(sock, temp->ai_addr, temp->ai_addrlen) == 0)
                break;
            close(sock);
        }
        freeaddrinfo(res);
        if (temp == NULL)
            {cout << "could not bind.. aborting." << endl; criticalErr = true; return ERROR;}
        int epollfd = epoll_create1(0);
        ev.events = EPOLLIN;
        ev.data.fd = sock;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev);
    }
    return 0;
}


int WebServ::serverAsma()
{
    sockaddr_in ss;
    ss.sin_family = AF_INET;
    ss.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ss.sin_port = htons(9999);
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        cout << "socket() failed" << endl;
    int flag = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
        cout << "error :/" << endl;
    int res = bind(sock, (struct sockaddr *)&ss, sizeof ss);
    if (res < 0)
    {
        cout << "bind() failed socket : " << sock << endl;
        return ERROR;
    }
    res = listen(sock, 1);
    cout << "server is listening at PORT => " << 9999 << " |  http://localhost:9999" << endl;
    if (res < 0)
    {
        cout << "listen() failed" << endl;
        return ERROR;
    }
    socklen_t len;
    while (1)
    {
        int new_sock = accept(sock, (struct sockaddr *)&ss, &len);
        if (new_sock < 0)
        {
            cout << "accept() didn't accept" << endl;
            return ERROR;
        }
        ofstream write1("Request");
        fillRequest(write1, new_sock);
        write1.close();
        if (parse_request(new_sock))
            return 100;
        close(new_sock);
   }
    close(sock);
    return 0;
}