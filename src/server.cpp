#include "main.hpp"

using namespace std;

int server()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int status;
    cout << "sock -> " << sock << endl;

    sockaddr_in sock_addr_in;

    sockaddr_in res;
    sock_addr_in.sin_family = AF_INET;
    sock_addr_in.sin_port = htons(9999);
    sock_addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");

    
    fd_set *fd;
    if (sock == -1)
    {
        cerr << "error on socket" << endl;
    }
    else
    {
        status = bind(sock, (struct sockaddr *)&sock_addr_in, sizeof(sock_addr_in));
        if (status == -1)
        {
            cerr << "error on bind" << endl;
        }
        else
        {
            socklen_t res_size;
            status = listen(sock, 100);
            if (status == -1)
            {
                cerr << "error on listen" << endl;
            }
            else
            {
                int clientSocket;
                clientSocket = accept(sock, (struct sockaddr *)&res, &res_size);
                if (clientSocket == -1)
                {
                    cerr << "error on accept" << endl;
                }
                else
                {
                    cout << "listen successfully" << endl;

                    char buffer[100];
                    ssize_t size;
                    while (size = recv(clientSocket, buffer, 100, 0))
                    {
                        cout << buffer << endl;
                    }
                    // read()
                    if (size > 0)
                    {
                        cout << "server recved data successfully" << endl;
                    }
                    else
                        cout << "issue with recv" << endl;
                }
            }
        }
    }
    return 0;
}

int main()
{
    server();
}