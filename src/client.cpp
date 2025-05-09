#include "../includes/webserv.hpp"

int client()
{
    sockaddr_in ss;
    ss.sin_family = AF_INET;
    ss.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ss.sin_port = htons(8080);
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        std::cout << "client socket() failed" << std::endl;
    // socklen_t len;
    int res = connect(sock, (struct sockaddr *)&ss, sizeof(ss));
    if (res < 0)
        std::cout << "client connect() failed" << std::endl;
    std::cout << "alloooo" <<std::endl;
    res = send(sock, "ASMA", 5, 0);
    std::cout << "Sent " << res << " bytes" << std::endl;
    return 0;
}

int main()
{
    client();
}