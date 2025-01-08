#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include "./database/database_function.h"
#include "./server_function.h"
#include "./server.h"

#define MAXLINE 4096   /* max text line length */
#define SERV_PORT 3000 /* port */
#define LISTENQ 8      /* maximum number of client connections */
#define MAX_CLIENTS 30 /* maximum number of connected clients */

char user_id[MAXLINE];
ClientStatus clients_status[MAX_CLIENTS]; // Array to manage client login status
int clients[MAX_CLIENTS] = {0};           // Array to manage connected sockets
int listenfd;                             // Listening socket

void manage_sockets(fd_set *readfds);
void handle_new_connection();
void handle_client_disconnect(int sd);
void handle_client_message(int sd, char *buf, int valread);
void logout_all_clients();
void handle_server_shutdown(int signum);

int main(int argc, char **argv)
{
    fd_set readfds;
    struct sockaddr_in servaddr;

    // Register signal handlers
    signal(SIGINT, handle_server_shutdown);  // Ctrl+C
    signal(SIGTERM, handle_server_shutdown); // Terminate signal

    // Initialize client status
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

    // Create server socket
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
        manage_sockets(&readfds);
    }

    close(listenfd);
    return 0;
}

void manage_sockets(fd_set *readfds)
{
    int max_sd = listenfd;
    char buf[MAXLINE];

    // Prepare the socket set
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

    // Monitor sockets for activity
    int activity = select(max_sd + 1, readfds, NULL, NULL, NULL);
    if ((activity < 0) && (errno != EINTR))
    {
        perror("select error");
        return;
    }

    // Handle new connections
    if (FD_ISSET(listenfd, readfds))
    {
        handle_new_connection();
    }

    // Handle IO operations for each client
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        int sd = clients[i];
        if (FD_ISSET(sd, readfds))
        {
            int valread = recv(sd, buf, MAXLINE, 0);
            if (valread == 0)
            {
                handle_client_disconnect(sd);
            }
            else if (valread < 0)
            {
                perror("recv error");
                handle_client_disconnect(sd);
            }
            else
            {
                buf[valread] = '\0';
                handle_client_message(sd, buf, valread);
            }
        }
    }
}

void handle_new_connection()
{
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
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

void handle_client_disconnect(int sd)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients_status[i].socket == sd)
        {
            printf("User '%s' disconnected\n", clients_status[i].username);

            // Update status in the database
            update_user_status(clients_status[i].username, 0);

            // Clear client status
            clients_status[i].socket = 0;
            clients_status[i].is_logged_in = 0;
            clients_status[i].username[0] = '\0';
            break;
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == sd)
        {
            clients[i] = 0;
            break;
        }
    }

    close(sd);
    printf("Closed socket %d.\n", sd);
}

void handle_client_message(int sd, char *buf, int valread)
{
    printf("Received message from socket %d: %s\n", sd, buf);
    process_message(buf, valread, sd);
}

void logout_all_clients()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients_status[i].is_logged_in)
        {
            printf("Logging out user '%s'\n", clients_status[i].username);

            // Update status in the database
            update_user_status(clients_status[i].username, 0);

            // Notify client about server shutdown
            if (clients_status[i].socket > 0)
            {
                char logout_message[MAXLINE] = "Server is shutting down. You have been logged out.";
                send(clients_status[i].socket, logout_message, strlen(logout_message), 0);
                close(clients_status[i].socket);
            }

            // Clear client status
            // clients_status[i].socket = 0;
            clients_status[i].is_logged_in = 0;
            // clients_status[i].username[0] = '\0';
        }
    }
}

void handle_server_shutdown(int signum)
{
    printf("\nServer shutting down... Signal: %d\n", signum);

    // Logout all clients
    logout_all_clients();

    // Close database
    close_database();

    printf("Server stopped.\n");
    exit(0);
}
