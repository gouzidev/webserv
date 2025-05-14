#include "../includes/webserv.hpp"

void WebServ::GET_METHODE(Request req)
{
    (void) req;
}

void WebServ::POST_METHODE(Request req)
{
    vector <string> start_line = req.getStartLine();
    
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

int WebServ::parse_request(int fd)
{
    (void) fd;
    string line;
    Request req;
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




    const char *testResponse =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "Hello, World!";

    send(fd, testResponse, strlen(testResponse), 0);

    cerr << testResponse;

    // answer_req(req);
    read.close();
    return 0;
}

int WebServ::server()
{
    sockaddr_in ss;
    ss.sin_family = AF_INET;
    ss.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ss.sin_port = htons(5551);
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
        printf("%d\n", errno);
        return 1;
    }
    res = listen(sock, 1);
    cout << "server is listening at PORT => " << 5551 << " |  http://localhost:5551" << endl;
    if (res < 0)
    {
        cout << "listen() failed" << endl;
        return 1;
    }
    socklen_t len;
    while (1)
    {
        int new_sock = accept(sock, (struct sockaddr *)&ss, &len);
        if (new_sock < 0)
        {
            cout << "accept() didn't accept" << endl;
            return 1;
        }
        else
            cout << "accept() accepted " << new_sock << endl;
        char buff[1000];
        ofstream write1("Request");
        cout << "new socket -> " << new_sock << endl;
        res = recv(new_sock, buff, 1000, 0);
        if (res == -1)
        {
            cout << "recv() didn't recv errno -> " << errno << " new socket " << new_sock << endl;
            return 1;
        }
        if (res > 0)
        {
            buff[res] = '\0';
            write1.write(buff, res);
            write1.close();
        }
        write1.close();
        if (parse_request(new_sock))
            return 100;
        close(new_sock);
   }
    close(sock);
    return 0;
}