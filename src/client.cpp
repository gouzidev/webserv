#include "main.hpp"

using namespace std;

int client()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int status;
    cout << "sock -> " << sock << endl;
    sockaddr_in sock_addr_in;
    sock_addr_in.sin_family = AF_INET;
    sock_addr_in.sin_port = htons(9999);
    sock_addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (sock == -1)
    {
        cout << "issue with socket" << endl;
    }
    else
    {
        status = connect(sock, (struct sockaddr *)&sock_addr_in, sizeof(sock_addr_in));
        cout << "client connected to the socket successfully" << endl;
        ssize_t size ;
        for (int i = 0; i < 10; i++)
        {
            size = send(sock, "Salam " + i, 10, 0);
        }
        if (size > 0)
            cout << "client sent data successfully" << endl;
        else
            cout << "issue with send" << endl;
        }
    (void) status;
    return 0;
}


int main()
{
   client()   ;
}