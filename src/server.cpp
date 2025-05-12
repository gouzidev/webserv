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
    std::string line;
    Request req;
    std::ifstream read("Request");
    cout << "*********************************************" << endl;
    std::getline(read, line);
    if (read.fail())
    {
        std::cerr << "[ " << line << " ]" << "wtf" << std::endl;
        return ERROR;
    }
    std::cout << line << std::endl;
    req.setStartLine(line);
    if (req.isStartLineValid() == 1)
    {
        cout << "invalid Request" << endl;
        return ERROR;
    }
    while(std::getline(read, line))
    {
        if(line.empty())
        {
            break;
        }
        req.setHeaders(line);
        std::cout << line << std::endl;
    }
    while(getline(read, line))
        req.setBody(line);
    cout << "*********************************************" << endl;

    answer_req(req);
    // std::cerr << "line is " << line << std::endl;
    // if (req.getStartLine().find("GET / HTTP/1.1") != std::string::npos)
    // {
    //     std::cerr << "wiwiwiwiw" << std::endl;
    //     int bytes_sent = send(fd, GETROOT, strlen(GETROOT), 0);
    //     if (bytes_sent < 0) {
    //         std::cerr << "Send failed: " << strerror(errno) << std::endl;
    //         return 1;
    //     } else {
    //         std::cerr << "Sent " << bytes_sent << " bytes" << std::endl;
    //     }
    // }
    // std::cerr << "bye bye" << std::endl;
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
        std::cout << "socket() failed" << std::endl;
    int flag = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
        std::cout << "error :/" << std::endl;
    int res = bind(sock, (struct sockaddr *)&ss, sizeof ss);
    if (res < 0)
    {
        std::cout << "bind() failed socket : " << sock << std::endl;
        printf("%d\n", errno);
        return 1;
    }
    res = listen(sock, 1);
    cout << "server is listening at PORT => " << 5551 << " |  http://localhost:5551" << endl;
    if (res < 0)
    {
        std::cout << "listen() failed" << std::endl;
        return 1;
    }
    socklen_t len;
    while(1)
    {
        int new_sock = accept(sock, (struct sockaddr *)&ss, &len);
        if (new_sock < 0)
        {
            std::cout << "accept() didn't accept" << std::endl;
            return 1;
        }
        else
            std::cout << "accept() accepted " << new_sock << std::endl;
        char buff[1000];
        std::ofstream write1("Request");
        // std::ifstream read("Request");
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
            // std::cout << buff << std::endl;    
            write1.write(buff, res);
            write1.close();
            // write1.clear();
        }
        write1.close();
        if (parse_request(new_sock))
            return 100;
        close(new_sock);
   }
    close(sock);
    return 0;
}