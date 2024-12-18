#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "./database/database_function.h"
#include "./server_function.h"
#define MAXLINE 4096   /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8      /*maximum number of client connections */

char user_id[MAXLINE]; // Định nghĩa biến user_id

int main(int argc, char **argv)
{
    int listenfd, connfd, n;
    pid_t childpid;
    socklen_t clilen;
    char buf[MAXLINE];
    struct sockaddr_in cliaddr, servaddr;

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
        exit(1);
    }

    // Listen
    if (listen(listenfd, LISTENQ) < 0)
    {
        perror("Listen failed");
        exit(1);
    }

    printf("Server running...waiting for connections.\n");

    // Vòng lặp chính
    for (;;)
    {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("Received request...\n");

        if ((childpid = fork()) == 0)
        { // Tạo tiến trình con
            printf("Forked a new child process to handle the connection\n");
            close(listenfd); // Tiến trình con không cần listen socket

            while ((n = recv(connfd, buf, MAXLINE, 0)) > 0)
            {
                process_message(buf, n, connfd); // Giả sử process_message sẽ xử lý và gửi phản hồi
                memset(buf, 0, sizeof(buf));     // Xóa bộ đệm
            }

            if (n < 0)
            {
                perror("Read error");
            }
            close(connfd);
            exit(0); // Thoát tiến trình con
        }
        close(connfd); // Tiến trình cha đóng kết nối client
    }

    close(listenfd);
    return 0;
}
