#ifndef CLIENT_FUNCTION_ROOM_H
#define CLIENT_FUNCTION_ROOM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include "client_function.h"
#include "menu.h"
#define MAXLINE 4096

char user_id[MAXLINE];

void create_room(int sockfd);
void view_all_rooms(int sockfd);
void join_room(int sockfd, char *room_id, char *user_id);




void create_room(int sockfd)
{
    char request[200];
    sprintf(request, "HEADER: CREATE_ROOM_REQ; DATA: ");
    send(sockfd, request, strlen(request), 0); // Gửi yêu cầu tạo phòng đến server

    char buffer[MAXLINE];
    int n = recv(sockfd, buffer, MAXLINE, 0); // Nhận phản hồi từ server
    buffer[n] = '\0';                         // Đảm bảo kết thúc chuỗi hợp lệ

    printf("%s\n", buffer); // In ra thông báo từ server (ID phòng hoặc lỗi)
}
void view_all_rooms(int sockfd)
{
    char request[200] = {0};
    char recvline[MAXLINE];
    // Tạo thông điệp gửi đi, chỉ chứa HEADER, không có DATA
    snprintf(request, sizeof(request), "HEADER: VIEW_ALL_ROOMS_REQ;");

    // Gửi yêu cầu đến server
    printf("Chuẩn bị gửi yêu cầu về Xem danh sách phòng ...\n");
    send(sockfd, request, strlen(request), 0); // Gửi thông điệp chỉ chứa HEADER
    printf("Yêu cầu đã gửi: %s\n", request);

    // Nhận phản hồi từ server
    if (recv(sockfd, recvline, MAXLINE, 0) == 0) // Kiểm tra phản hồi từ server
    {
        printf("Máy chủ đã kết thúc sớm, không có phản hồi.\n");
        exit(4);
    }

    process_message(recvline, strlen(recvline)); // Kết thúc chuỗi

    // Hiện thị thông báo kết thúc
    printf("\nKết thúc phản hồi về Xem danh sách phòng .\n");
}

void join_room(int sockfd, char *room_id, char *user_id)
{
    char recvline[MAXLINE];

    while (1)
    {
        printf("Đang gửi yêu cầu về tham gia phòng...\n");

        char request[200];
        sprintf(request, "HEADER: JOIN_ROOM_REQ; DATA: %s %s", room_id, user_id);
        send(sockfd, request, strlen(request), 0); // Gửi yêu cầu về tham gia phòng

        // Nhận phản hồi từ server
        if (recv(sockfd, recvline, MAXLINE, 0) == 0) // Kiểm tra phản hồi từ server
        {
            printf("Máy chủ đã kết thúc sớm, không có phản hồi.\n");
            exit(4);
        }

        process_message(recvline, strlen(recvline)); // Kết thúc chuỗi

        // Sau khi vào phòng thành công thì add vào main game
        char *data_start = strstr(recvline, "DATA: ");
        if (data_start != NULL)
        {
            data_start += 6; // Bỏ qua "DATA: " để chỉ lấy phần giá trị thực tế
            if (strncmp(data_start, "0", 1) == 0)
            { // Kiểm tra dữ liệu "0" (Đăng nhập thành công)
                printf("Vào phòng thành công!\n");
                while (1)
                {
                    int choice = display_in_room_menu(room_id);
                    if (choice == 1)
                    {
                        printf("Hiển thị thông tin phòng\n");
                    }
                    else if (choice == 2)
                    {
                        printf("Thoat phong\n");
                        return;
                    }
                }
            }
            else if (strncmp(data_start, "1", 1) == 0)
            {
                printf("Bạn không thể vào phòng vì một vài lí do. Vui lòng thử lại.\n");
                printf("1. Thử lại\n");
                printf("2. Quay lại menu chính\n");
                int retry_choice;
                scanf("%d", &retry_choice);

                if (retry_choice == 2)
                { // Quay lại menu chính
                    break;
                }
            }
            else
            { // Dữ liệu không hợp lệ từ server
                printf("Dữ liệu không hợp lệ từ server: %s\n", data_start);
            }
        }
        // Hiện thị thông báo kết thúc
        printf("\nKết thúc phản hồi về Join phòng .\n");
    }
}

#endif // CLIENT_FUNCTION_ROOM_H
