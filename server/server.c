#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h> // Include errno.h
#include <time.h>

#include "./database/database_function.h"
#include "./server_function.h"
#define MAXLINE 4096   /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8      /*maximum number of client connections */
#define MAX_CLIENTS 30 // Maximum number of clients

char user_id[MAXLINE]; // Định nghĩa biến user_id

// Định nghĩa mảng clients_info
ClientInfo clients_info[MAX_CLIENTS]; // Mảng chứa thông tin người dùng

int main(int argc, char **argv)
{
    signal(SIGINT, handle_signal);  // Xử lý Ctrl+C
    signal(SIGTERM, handle_signal); // Xử lý tín hiệu dừng
    int listenfd, n;
    int clients[MAX_CLIENTS]; // Array to hold client sockets
    fd_set readfds;           // Set of socket descriptors
    int max_sd, sd, activity, new_socket;
    struct sockaddr_in cliaddr, servaddr;
    socklen_t clilen = sizeof(cliaddr); // Initialize clilen
    char buf[MAXLINE];

    // Initialize all client sockets to 0
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i] = 0;
    }

    open_database();
    initialize_database();

    // Tạo socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Định cấu hình địa chỉ server
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    // Bind socket
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        close(listenfd);
        exit(1);
    }

    // Listen
    if (listen(listenfd, LISTENQ) < 0)
    {
        perror("Listen failed");
        close(listenfd);
        exit(1);
    }

    printf("Server running on port %d...waiting for connections.\n", SERV_PORT);

    // Main loop
    for (;;)
    {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add listen socket to set
        FD_SET(listenfd, &readfds);
        max_sd = listenfd;

        // Add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = clients[i];

            // If valid socket descriptor then add to read list
            if (sd > 0)
            {
                FD_SET(sd, &readfds);
            }

            // Highest file descriptor number, needed for the select function
            if (sd > max_sd)
            {
                max_sd = sd;
            }
        }

        // Wait for an activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            perror("select error");
            continue;
        }
        handle_reconnection(new_socket, user_id);
        // If something happened on the listen socket, then it's an incoming connection
        if (FD_ISSET(listenfd, &readfds))
        {
            clilen = sizeof(cliaddr); // Initialize clilen before accept
            if ((new_socket = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
            {
                perror("Accept failed");
                continue;
            }

            printf("New connection, socket fd is %d, ip is : %s, port : %d\n", new_socket, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

            // Add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] == 0)
                {
                    clients[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        // Else it's some IO operation on some other socket
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = clients[i];

            if (FD_ISSET(sd, &readfds))
            {
                // Check if it was for closing, and also read the incoming message
                if ((n = recv(sd, buf, MAXLINE, 0)) == 0)
                {
                    // Somebody disconnected
                    getpeername(sd, (struct sockaddr *)&cliaddr, &clilen);
                    printf("Host disconnected, ip %s, port %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

                    // Cập nhật trạng thái người dùng về 0
                    const char *username = get_username_from_connfd(sd); // Lấy tên người dùng từ connfd
                    if (username != NULL)
                    {
                        update_user_status(username, 0); // Đặt trạng thái là 0
                        printf("User %s status updated to 0 (disconnected).\n", username);
                    }
                    else
                    {
                        printf("Could not retrieve username for disconnected client.\n");
                    }

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    clients[i] = 0;
                }
                else if (n < 0)
                {
                    perror("recv error");
                    close(sd);
                    clients[i] = 0;
                }
                else
                {
                    // Process the message
                    buf[n] = '\0'; // Null-terminate the received data
                    printf("Received message: %s\n", buf);
                    process_message(buf, n, sd);
                }
            }
        }

        // Gửi heartbeat đến các client mỗi 5 giây
        static time_t last_heartbeat = 0;
        if (time(NULL) - last_heartbeat >= 5)
        {
            send_heartbeat_to_clients(clients, MAX_CLIENTS);
            last_heartbeat = time(NULL);
        }
    }

    close(listenfd);
    return 0;
}
