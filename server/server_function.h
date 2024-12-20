#ifndef SERVER_FUNCTION_H
#define SERVER_FUNCTION_H

#include <stdio.h>
#include <string.h>
#include "./database/database_function.h"
#include <ctype.h>
#include "send_question_function.h"
#include "server_function_login_logout.h"
#include "server_function_room.h"
// Định nghĩa hằng số
#define MAXLINE 4096
extern char user_id[MAXLINE]; // Khai báo biến user_id

// Các hàm xử lý
void process_message(char *msg, int n, int connfd);
void send_message(char *header, char *data, int connfd);
// Xử lí thông điệp gửi đến

void send_message(char *header, char *data, int connfd)
{
    char sendline[MAXLINE];
    sprintf(sendline, "HEADER: %s; DATA: %s", header, data);
    send(connfd, sendline, strlen(sendline), 0);

    printf("Server sent: %s\n", sendline);
}

void process_message(char *msg, int n, int connfd)
{
    msg[n] = '\0';                        // Đảm bảo chuỗi null-terminated
    printf("Server đã nhận: %s\n", msg); // Debug thông điệp nhận

    char header[MAXLINE] = {0};
    char data[MAXLINE] = {0};

    static int game_requests = 0; // số lượng yêu cầu vào game
    static int connfds[3]; // Mảng chứa các kết nối của người chơi

    // Tách HEADER và DATA
    char *header_start = strstr(msg, "HEADER: ");
    char *data_start = strstr(msg, "DATA: ");

    if (!header_start)
    {
        fprintf(stderr, "Invalid message format: Missing HEADER\n");
        send_message("ERROR", "Missing HEADER in the message.", connfd);
        return;
    }

    // Tách HEADER
    header_start += strlen("HEADER: ");
    char *header_end = strstr(header_start, ";"); // Chờ dấu chấm phẩy kết thúc HEADER
    if (!header_end)
    {
        fprintf(stderr, "Invalid message format: HEADER end not found\n");
        send_message("ERROR", "Invalid header format", connfd);
        return;
    }

    strncpy(header, header_start, header_end - header_start); // Lấy phần HEADER
    header[header_end - header_start] = '\0';                 // Kết thúc chuỗi

    // Kiểm tra DATA
    if (data_start)
    {
        // Nếu có DATA, tách phần DATA
        data_start += strlen("DATA: ");
        strncpy(data, data_start, MAXLINE - 1);
        data[MAXLINE - 1] = '\0'; // Kết thúc chuỗi
    }
    else
    {
        // Nếu không có DATA, đặt DATA là rỗng
        strcpy(data, "");
    }

    // Thêm thông báo debug để kiểm tra HEADER và DATA
    printf("Received HEADER: '%s', DATA: '%s'\n", header, data);

    // Gọi các hàm xử lý dựa trên HEADER
    if (strcmp(header, "LOGIN_REQ") == 0)
    {
        handle_login_request(data, connfd);
    }
    else if (strcmp(header, "REGISTER_REQ") == 0)
    {
        handle_register_request(data, connfd);
    }
    else if (strcmp(header, "CREATE_ROOM_REQ") == 0)
    {
        handle_create_room_request(connfd);
    }
    else if (strcmp(header, "VIEW_ALL_ROOMS_REQ") == 0)
    {
        handle_view_all_room_request(connfd);
    }
    else if (strcmp(header, "JOIN_ROOM_REQ") == 0)
    {
        handle_join_room_request(data, connfd);
    }
    else if (strcmp(header, "JOIN_GAME") == 0)
    {     
        connfds[game_requests++] = connfd;
        if (game_requests == 3)
        {
            // Đủ 3 người chơi, bắt đầu game
            handle_game(connfds[0], connfds[1], connfds[2]);
            game_requests = 0; // Reset số lượng yêu cầu vào game
        }
        else
        {
            send(connfd, "WAITING_FOR_PLAYERS", strlen("WAITING_FOR_PLAYERS"), 0);
        }
    }
    else
    {
        printf("Header không hợp lệ: %s\n", header);
        send_message("ERROR", "HEADER không hợp lệ", connfd); // Phản hồi lỗi
    }
}

#endif // SERVER_FUNCTION_H
