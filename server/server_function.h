#ifndef SERVER_FUNCTION_H
#define SERVER_FUNCTION_H

#include <stdio.h>
#include <string.h>
#include "./database/database_function.h"
#include <ctype.h>
#include "send_question_function.h"
#include "server_function_login_logout.h"
#include "server_function_room.h"
#include "server_function_game_2.h"
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
    msg[n] = '\0';                       // Đảm bảo chuỗi null-terminated
    printf("Server đã nhận: %s\n", msg); // Debug thông điệp nhận

    char header[MAXLINE] = {0};
    char data[MAXLINE] = {0};

    // static int game_requests = 0; // số lượng yêu cầu vào game
    // static Player *player[3];     // Mảng chứa các kết nối của người chơi

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
    else if (strcmp(header, "LOGOUT_REQ") == 0)
    {
        handle_logout_request(data, connfd);
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
    else if (strcmp(header, "LEAVE_ROOM_REQ") == 0)
    {
        handle_leave_room_request(data, connfd);
    }
    else if (strcmp(header, "VIEW_ROOM_INFO_REQ") == 0)
    {
        handle_view_room_info_request(data, connfd);
    }

    else if (strcmp(header, "JOIN_GAME") == 0)
    {
        // Tách user_id và room_id từ data
        // char user_id[10];
        // char room_id[10];
        // sscanf(data, "%s %s", user_id, room_id);
        // player[game_requests] = (Player *)malloc(sizeof(Player));
        // player[game_requests]->connfd = connfd;
        // player[game_requests]->user_id = atoi(user_id);
        // player[game_requests]->room_id = atoi(room_id);
        // printf("Player %d joined with connection %d\n", game_requests + 1, connfd);
        // game_requests++;

        // if (game_requests == 3)
        // {
        //     printf("Starting game with connections: %d, %d, %d\n",
        //            player[0]->connfd, player[1]->connfd, player[2]->connfd);

        //     // Notify all players that game is starting
        //     for (int i = 0; i < 3; i++)
        //     {
        //         send(player[i]->connfd, "GAME_START", strlen("GAME_START"), 0);
        //     }

        //     // Start the game
        //     handle_game(player);

        //     // Reset for next game
        //     game_requests = 0;
        //     printf("Game ended. Resetting game requests = %d\n", game_requests);
        //     for (int i = 0; i < 3; i++)
        //     {
        //         free(player[i]);
        //         player[i] = NULL;
        //     }
        // }
        // else
        // {
        //     send(connfd, "WAITING_FOR_PLAYERS", strlen("WAITING_FOR_PLAYERS"), 0);
        // }
        handle_join_game_request(data, connfd);
    }
    else if (strcmp(header, "ANSWER") == 0)
    {
        handle_answer_of_client(data, connfd);
    }

    else
    {
        printf("Header không hợp lệ: %s\n", header);
        send_message("ERROR", "HEADER không hợp lệ", connfd); // Phản hồi lỗi
    }
}

#endif // SERVER_FUNCTION_H
