#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS 64
#define SERVER_PORT 8080
#define MAX_CLIENTS 1024
#define WRITE_BUFFER_SIZE 65536 // 64KB write buffer

// Represents the state of a single client connection
typedef struct {
    char write_buf[WRITE_BUFFER_SIZE];
    size_t bytes_to_write;
    size_t bytes_written;
} Connection;

// Array to hold the state for all possible fds. Indexed by fd.
Connection *connections;

// A large string to simulate a file download
const char* large_content = "This is a large piece of content... [repeat many times] ...end of content.";
char http_response[1024 * 1024]; // 1MB response buffer
int response_len;


// Function to set a file descriptor to non-blocking mode
void set_nonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

// Prepare the large HTTP response once at the start
void prepare_large_response() {
    char content_buffer[1024 * 900]; // 900KB content
    strcpy(content_buffer, "");
    for(int i=0; i < 5000; i++) {
        strcat(content_buffer, large_content);
    }

    response_len = sprintf(http_response,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "\r\n"
        "%s",
        strlen(content_buffer), content_buffer);
}

// Attempts to write the remaining data for a connection
void handle_write(int epoll_fd, int client_fd) {
    Connection* conn = &connections[client_fd];
    ssize_t bytes_sent;

    while (conn->bytes_written < conn->bytes_to_write) {
        bytes_sent = write(client_fd,
                           conn->write_buf + conn->bytes_written,
                           conn->bytes_to_write - conn->bytes_written);

        if (bytes_sent >= 0) {
            conn->bytes_written += bytes_sent;
        } else {
            // EWOULDBLOCK means the kernel's send buffer is full.
            // We must wait for EPOLLOUT to become writable again.
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                 // The socket is not ready for writing, wait for EPOLLOUT
                return;
            }
            // A real error occurred
            close(client_fd);
            return;
        }
    }

    // All data has been written successfully
    if (conn->bytes_written == conn->bytes_to_write) {
        printf("✅ Finished sending response to fd %d\n", client_fd);
        // We can close the connection or wait for the next request.
        // For this example, we close it.
        close(client_fd);
    }
}


int main() {
    int listen_sock, conn_sock, epoll_fd, num_events;
    struct sockaddr_in server_addr;
    struct epoll_event event, events[MAX_EVENTS];

    // Allocate memory for connection states
    connections = (Connection*) calloc(MAX_CLIENTS, sizeof(Connection));
    prepare_large_response();

    // --- Standard server setup ---
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(listen_sock, SOMAXCONN);
    set_nonblocking(listen_sock);

    // --- Epoll setup ---
    epoll_fd = epoll_create1(0);
    event.events = EPOLLIN; // Monitor for incoming connections
    event.data.fd = listen_sock;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &event);

    printf("🚀 Server started on port %d, serving a large response.\n", SERVER_PORT);

    // --- Main Event Loop ---
    while (1) {
        num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for (int i = 0; i < num_events; i++) {
            int current_fd = events[i].data.fd;

            if (current_fd == listen_sock) {
                // New connection
                while ((conn_sock = accept(listen_sock, NULL, NULL)) > 0) {
                    set_nonblocking(conn_sock);
                    event.events = EPOLLIN; // Monitor new socket for read events
                    event.data.fd = conn_sock;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, &event);
                    printf("Accepted new connection on fd %d\n", conn_sock);
                }
            } else if (events[i].events & EPOLLIN) {
                // Client sent data (e.g., an HTTP request)
                char read_buffer[1024];
                read(current_fd, read_buffer, sizeof(read_buffer)); // Drain the read

                printf("Client on fd %d sent a request. Preparing large response.\n", current_fd);

                // Prepare the connection state for writing the response
                Connection* conn = &connections[current_fd];
                conn->bytes_to_write = response_len;
                conn->bytes_written = 0;
                // For simplicity, we assume the response fits in the buffer.
                // A real server would stream from a file into this buffer.
                memcpy(conn->write_buf, http_response, response_len);

                // IMPORTANT: Change monitoring from read to write
                event.events = EPOLLOUT; // Now we want to know when we can write
                event.data.fd = current_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, current_fd, &event);

            } else if (events[i].events & EPOLLOUT) {
                // Socket is now writable, continue sending data
                printf("Socket %d is writable (EPOLLOUT), sending more data.\n", current_fd);
                handle_write(epoll_fd, current_fd);
            }
        }
    }

    // --- Cleanup ---
    close(listen_sock);
    close(epoll_fd);
    free(connections);
    return 0;
}
