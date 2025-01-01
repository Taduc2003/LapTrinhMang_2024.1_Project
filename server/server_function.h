#ifndef SERVER_FUNCTION_H
#define SERVER_FUNCTION_H

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "./database/database_function.h"
#include <ctype.h>
#include "send_question_function.h"
#include "server_function_login_logout.h"
#include "server_function_room.h"
// Định nghĩa hằng số
#define MAXLINE 4096
#define MAX_CLIENTS 30 // Số lượng client tối đa

// Cấu trúc để lưu thông tin người dùng
typedef struct
{
    int connfd;             // Socket descriptor
    char username[MAXLINE]; // Tên người dùng
} ClientInfo;

// Mảng chứa thông tin người dùng
extern ClientInfo clients_info[MAX_CLIENTS]; // Khai báo mảng clients_info

// Các hàm xử lý
void process_message(char *msg, int n, int connfd);
void send_message(char *header, char *data, int connfd);
void send_heartbeat_to_clients(int *clients, int max_clients);
void handle_reconnection(int client_socket, const char *username);
void handle_signal(int signal);
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

    static int game_requests = 0; // số lượng yêu cầu vào game
    static Player *player[3];     // Mảng chứa các kết nối của người chơi

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
        char user_id[10];
        char room_id[10];
        sscanf(data, "%s %s", user_id, room_id);
        player[game_requests] = (Player *)malloc(sizeof(Player));
        player[game_requests]->connfd = connfd;
        player[game_requests]->user_id = atoi(user_id);
        player[game_requests]->room_id = atoi(room_id);
        printf("Player %d joined with connection %d\n", game_requests + 1, connfd);
        game_requests++;

        if (game_requests == 3)
        {
            printf("Starting game with connections: %d, %d, %d\n",
                   player[0]->connfd, player[1]->connfd, player[2]->connfd);

            // Notify all players that game is starting
            for (int i = 0; i < 3; i++)
            {
                send(player[i]->connfd, "GAME_START", strlen("GAME_START"), 0);
            }

            // Start the game
            handle_game(player);

            // Reset for next game
            game_requests = 0;
            for (int i = 0; i < 3; i++)
            {
                free(player[i]);
                player[i] = NULL;
            }
        }
        else
        {
            send(connfd, "WAITING_FOR_PLAYERS", strlen("WAITING_FOR_PLAYERS"), 0);
        }
    }
    else if (strcmp(header, "LOGOUT_REQ") == 0)
    {
        const char *username = get_username_from_connfd(connfd); // Lấy tên người dùng từ connfd
        if (username != NULL)
        {
            update_user_status(username, 0); // Đặt trạng thái là 0
            printf("User %s has logged out and status updated to 0.\n", username);
            strcpy(clients_info[connfd].username, ""); // Xóa tên người dùng khỏi mảng
            clients_info[connfd].connfd = 0;           // Xóa connfd khỏi mảng
        }
        else
        {
            printf("Could not retrieve username for logout request.\n");
        }

        // Gửi phản hồi cho client
        send_message("LOGOUT_SUCCESS", "You have successfully logged out.", connfd);
    }
    else
    {
        printf("Header không hợp lệ: %s\n", header);
        send_message("ERROR", "HEADER không hợp lệ", connfd); // Phản hồi lỗi
    }
}

void send_heartbeat_to_clients(int *clients, int max_clients)
{
    for (int i = 0; i < max_clients; i++)
    {
        if (clients[i] != 0)
        {
            // Gửi gói tin "PING" đến client
            if (send(clients[i], "PING", 4, 0) < 0)
            {
                if (errno == ECONNRESET)
                {
                    printf("Client %d disconnected abruptly\n", i);
                }
                close(clients[i]);
                clients[i] = 0; // Đánh dấu socket là không hợp lệ
            }
        }
    }
}

void handle_reconnection(int client_socket, const char *username)
{
    int status = check_user_status(username);

    if (status == 0)
    {
        // Tài khoản đã được đánh dấu là chưa đăng nhập, cho phép người dùng đăng nhập lại
        printf("User %s can log in again.\n", username);
        // Gọi hàm xử lý đăng nhập
    }
    else if (status == 1)
    {
        // Tài khoản vẫn đang đăng nhập, có thể thông báo cho người dùng
        printf("User %s is already logged in.\n", username);
    }
    else
    {
        // Xử lý trường hợp không tìm thấy tài khoản
        printf("User %s not found.\n", username);
    }
}

void handle_signal(int signal)
{
    printf("Received signal %d, shutting down server...\n", signal);
    update_all_user_status(0); // Cập nhật trạng thái tất cả người dùng về 0
    exit(0);                   // Thoát khỏi chương trình
}
#endif // SERVER_FUNCTION_H
