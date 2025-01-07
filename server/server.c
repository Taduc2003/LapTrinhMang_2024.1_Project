#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> // Include inet_ntoa
#include "./database/database_function.h"
#include "./server_function.h"
#include "./server.h"

#define MAXLINE 4096   /* max text line length */
#define SERV_PORT 3000 /* port */
#define LISTENQ 8      /* maximum number of client connections */
// ... existing code ...
#define MAX_CLIENTS 30

char user_id[MAXLINE];                    // Define user_id variable
ClientStatus clients_status[MAX_CLIENTS]; // Định nghĩa biến clients_status

void manage_sockets(int listenfd, int *clients, fd_set *readfds);

int main(int argc, char **argv)
{
    int listenfd;
    int clients[MAX_CLIENTS] = {0}; // Array to hold client sockets
    fd_set readfds;                 // Set of socket descriptors
    struct sockaddr_in servaddr;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients_status[i].socket = 0;
        clients_status[i].is_logged_in = 0;
        clients_status[i].username[0] = '\0';
    }

    // Open and initialize the database
    open_database();
    initialize_database();
    update_all_users_status();
    // Create a socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    // Bind the socket
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(listenfd, LISTENQ) < 0)
    {
        perror("Listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    printf("Server running on port %d...waiting for connections.\n", SERV_PORT);

    // Main loop to manage sockets
    while (1)
    {
        manage_sockets(listenfd, clients, &readfds);
    }

    close(listenfd);
    return 0;
}

// Function to manage sockets
void manage_sockets(int listenfd, int *clients, fd_set *readfds)
{
    int max_sd = listenfd;      // Track the highest socket descriptor
    struct sockaddr_in cliaddr; // Client address structure
    socklen_t clilen = sizeof(cliaddr);
    char buf[MAXLINE]; // Buffer for incoming messages

    // Clear and prepare the socket set
    FD_ZERO(readfds);
    FD_SET(listenfd, readfds);

    // Add active client sockets to the set
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        int sd = clients[i];
        if (sd > 0)
        {
            FD_SET(sd, readfds);
        }
        if (sd > max_sd)
        {
            max_sd = sd;
        }
    }

    // Wait for an activity on one of the sockets
    int activity = select(max_sd + 1, readfds, NULL, NULL, NULL);
    if ((activity < 0) && (errno != EINTR))
    {
        perror("select error");
        return;
    }

    // Handle new connections
    if (FD_ISSET(listenfd, readfds))
    {
        int new_socket = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (new_socket < 0)
        {
            perror("Accept failed");
            return;
        }

        printf("New connection, socket fd: %d, IP: %s, Port: %d\n",
               new_socket, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

        // Add new socket to clients array
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] == 0)
            {
                clients[i] = new_socket;
                printf("Added to client list at position %d\n", i);
                break;
            }
        }
    }
    printf("------- CÁC SOCKET ĐANG KẾT NỐI -----\n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] > 0)
        {
            struct sockaddr_in peer_addr;
            socklen_t peer_len = sizeof(peer_addr);

            // Lấy thông tin IP và port của socket
            if (getpeername(clients[i], (struct sockaddr *)&peer_addr, &peer_len) == 0)
            {
                printf("Socket %d, IP: %s, Port: %d\n",
                       clients[i],
                       inet_ntoa(peer_addr.sin_addr),
                       ntohs(peer_addr.sin_port));
            }
            else
            {
                perror("getpeername error"); // In lỗi nếu không lấy được thông tin
            }
        }
    }
    printf("-------------------------------------\n");

    // Handle IO operations for each client
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        int sd = clients[i];

        if (FD_ISSET(sd, readfds))
        {
            int valread = recv(sd, buf, MAXLINE, 0);
            if (valread == 0)
            {
                // Lấy thông tin tài khoản đăng nhập trên socket này
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clients_status[i].socket == sd)
                    {
                        printf("User '%s' disconnected\n", clients_status[i].username);

                        // Cập nhật trạng thái trong cơ sở dữ liệu
                        update_user_status(clients_status[i].username, 0);

                        // Xóa thông tin trạng thái
                        clients_status[i].socket = 0;
                        clients_status[i].is_logged_in = 0;
                        clients_status[i].username[0] = '\0';
                        break;
                    }
                }

                // Đóng socket
                close(sd);
                clients[i] = 0;
            }
            else if (valread < 0)
            {
                perror("recv error");
                close(sd);
                clients[i] = 0; // Remove from clients list
            }
            else
            {
                // Process the received message
                buf[valread] = '\0';
                printf("Received message from socket %d: %s\n", sd, buf);
                process_message(buf, valread, sd);
            }
        }
    }
}
