#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "menu.h"  // Đảm bảo bao gồm menu.h để sử dụng cấu trúc users
#include <ctype.h> // Thêm thư viện này để sử dụng isspace
#include "client_function_login_logout.h"
#include "client_function_room.h"
#include "client_function.h"

#define BUFFER_SIZE 1000000
#define MAXLINE 4096   /*max text line length*/
#define SERV_PORT 3000 /*port*/
char user_id[MAXLINE];

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;

    // Kiểm tra tham số đầu vào
    if (argc != 2)
    {
        perror("Usage: TCPClient <IP address of the server");
        exit(1);
    }

    // Tạo socket cho client
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }

    // Cấu hình địa chỉ server
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(SERV_PORT); // Convert to big-endian order

    // Kết nối đến server
    printf("Connecting to server at %s:%d...\n", argv[1], SERV_PORT);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(3);
    }
    printf("Connection established.\n");

    //-----------------------------------------------------------
    // Vòng lặp chương trình client
    while (1)
    {
        int login_or_register = display_welcome_menu();
        printf("Option selected: %d\n", login_or_register); // Debug line

        if (login_or_register == 1) // Đăng nhập
        {
            login_user(sockfd);
        }
        else if (login_or_register == 2) // Xử lý đăng ký
        {
            register_user(sockfd);
        }
        else if (login_or_register == 3) // Thoát chương trình
        {
            printf("Goodbye!\n");
            exit(0);
        }
    }

    exit(0);
}
//-------------------------------------------------------------

