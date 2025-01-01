#ifndef CLIENT_FUNCTION_LOGIN_LOGOUT_H
#define CLIENT_FUNCTION_LOGIN_LOGOUT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include "client_function.h"
#include "menu.h"
#include "client_function_room.h"
#include "client_function_game.h"

#define MAXLINE 4096

char user_id[MAXLINE];

void login_user(int sockfd);
void register_user(int sockfd);
void menu_user(int sockfd);
void register_user(int sockfd)
{
    users sender_acc;
    char header[MAXLINE], data[MAXLINE], recvline[MAXLINE];

    while (1) // Vòng lặp đăng ký
    {
        sender_acc = display_register_menu();
        sprintf(header, "REGISTER_REQ");
        sprintf(data, "username: %s; password: %s", sender_acc.username, sender_acc.password);
        send_message(header, data, sockfd);

        if (recv(sockfd, recvline, MAXLINE, 0) == 0)
        {
            perror("The server terminated prematurely");
            exit(4);
        }

        // Trước tiên, loại bỏ khoảng trắng hoặc ký tự không cần thiết
        trim_whitespace(recvline);

        // Sau đó, kiểm tra phần "DATA" trong thông điệp của server
        char *data_start = strstr(recvline, "DATA: ");
        if (data_start != NULL)
        {
            data_start += 6; // Bỏ qua "DATA: " để chỉ lấy phần giá trị thực tế
            if (strncmp(data_start, "0", 1) == 0)
            { // Kiểm tra dữ liệu "0" (Đăng ký thành công)
                printf("Đăng ký thành công! Quay lại menu chính...\n");
                break; // Thoát vòng lặp đăng ký
            }
            else if (strncmp(data_start, "1", 1) == 0)
            { // Đăng ký thất bại
                printf("Tên đăng nhập đã tồn tại. Vui lòng thử lại.\n");
                printf("1. Thử lại\n");
                printf("2. Quay lại menu chính\n");
                int retry_choice;
                scanf("%d", &retry_choice);
                int c;
                while ((c = getchar()) != '\n' && c != EOF)
                    ;
                if (retry_choice == 2)
                { // Quay lại menu chính
                    break;
                }
            }
        }
    }
}

void login_user(int sockfd)
{
    while (1) // Vòng lặp đăng nhập
    {
        users sender_acc;
        char header[MAXLINE], data[MAXLINE], recvline[MAXLINE];

        sender_acc = display_login_menu(); // Hiển thị màn hình nhập thông tin đăng nhập
        sprintf(header, "LOGIN_REQ");
        sprintf(data, "username: %s; password: %s", sender_acc.username, sender_acc.password);
        send_message(header, data, sockfd); // Gửi thông điệp đăng nhập đến server

        if (recv(sockfd, recvline, MAXLINE, 0) == 0) // Kiểm tra phản hồi từ server
        {
            printf("Máy chủ đã kết thúc sớm, không có phản hồi.\n");
            exit(4);
        }

        process_message(recvline, strlen(recvline));
        // Sau khi nhận thông điệp từ server, loại bỏ khoảng trắng và ký tự xuống dòng thừa
        // Trước tiên, loại bỏ khoảng trắng hoặc ký tự không cần thit
        trim_whitespace(recvline);

        // Sau đó, kiểm tra phần "DATA" trong thông điệp của server
        char *data_start = strstr(recvline, "DATA: ");
        if (data_start != NULL)
        {
            data_start += 6; // Bỏ qua "DATA: " để chỉ lấy phần giá trị thực tế
            if (strncmp(data_start, "0", 1) == 0)
            { // Kiểm tra dữ liệu "0" (Đăng nhập thành công)
                printf("Đăng nhập thành công!\n");
                menu_user(sockfd);
                break;
            }
            else if (strncmp(data_start, "-1", 2) == 0)
            { // Đăng nhập thất bại
                printf("Sai tên đăng nhập hoặc mật khẩu. Vui lòng thử lại.\n");
                printf("1. Thử lại\n");
                printf("2. Quay lại menu chính\n");
                int retry_choice;
                scanf("%d", &retry_choice);
                int c;
                while ((c = getchar()) != '\n' && c != EOF)
                    ;
                if (retry_choice == 2)
                { // Quay lại menu chính
                    break;
                }
            }
            else if (atoi(data_start) > 0) // Kiểm tra nếu data là ID người dùng
            {
                printf("Đăng nhập thành công với ID: %s\n", data_start);
                menu_user(sockfd);
                break;
            }
            else
            { // Dữ liệu không hợp lệ từ server
                printf("Dữ liệu không hợp lệ từ server: %s\n", data_start);
            }
        }
    }
}

void menu_user(int sockfd)
{
    printf("Kết quả: Đăng nhập thành công\n");
    char room_id[MAXLINE];
    while (1)
    {
        int choice = display_main_menu();
        switch (choice) // Vòng lặp chính
        {
        case 1:
            create_room(sockfd);
            break;
        case 2:

            printf("Nhập ID phòng để tham gia: ");
            scanf("%s", room_id);
            join_room(sockfd, room_id, user_id);
            break;
        case 3:
            view_all_rooms(sockfd);
            break;
        case 4:
            printf("Logging out...\n");
            return;
        default:
            printf("Lựa chọn không hợp lệ. Vui lòng thử lại.\n");
        }
    }
}
#endif // CLIENT_FUNCTION_LOGIN_LOGOUT_H
