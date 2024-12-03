#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#define MAXLINE 4096   /*max text line length*/
#define SERV_PORT 3000 /*port*/

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE], recvline[MAXLINE];

    // basic check of the arguments
    // additional checks can be inserted
    if (argc != 2)
    {
        perror("Usage: TCPClient <IP address of the server");
        exit(1);
    }

    // Create a socket for the client
    // If sockfd<0 there was an error in the creation of the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }

    // Đảm bảo rằng cấu trúc servaddr được khởi tạo sạch sẽ
    memset(&servaddr, 0, sizeof(servaddr));
    // Đảm bảo rằng chúng ta đang sử dụng giao thức IPv4
    servaddr.sin_family = AF_INET;
    // Đảm bảo rằng địa chỉ IP của máy chủ được chuyển đổi thành dạng số và được lưu trữ trong servaddr.sin_addr.s_addr
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    // Đảm bảo rằng cổng của máy chủ được chuyển đổi thành thứ tự byte lớn (big-endian) và được lưu trữ trong servaddr.sin_port
    servaddr.sin_port = htons(SERV_PORT);

    // Connection of the client to the socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(3);
    }

    while (fgets(sendline, MAXLINE, stdin) != NULL)
    {

        send(sockfd, sendline, strlen(sendline), 0);

        if (recv(sockfd, recvline, MAXLINE, 0) == 0)
        {
            // error: server terminated prematurely
            perror("The server terminated prematurely");
            exit(4);
        }
        printf("%s", "String received from the server: ");
        fputs(recvline, stdout);
    }

    exit(0);
}
