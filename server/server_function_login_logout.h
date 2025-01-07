#ifndef SERVER_FUNCTION_LOGIN_LOGOUT_H
#define SERVER_FUNCTION_LOGIN_LOGOUT_H

#include <stdio.h>
#include <string.h>
#include "./database/database_function.h"
#include <ctype.h>
#include "send_question_function.h"
#include "server_function_room.h"
#include "server_function.h"

// Định nghĩa hằng số
#define MAXLINE 4096

extern char user_id[MAXLINE]; // Khai báo biến user_id

// Cấu trúc lưu thông tin kết nối
typedef struct
{
    int connfd;
    char username[50];
} ConnectionInfo;

// Mảng lưu trữ thông tin kết nối
#define MAX_CONNECTIONS 100
static ConnectionInfo connections[MAX_CONNECTIONS];
static int connection_count = 0;

// Hàm xử lý yêu cầu đăng nhập
void handle_login_request(char *data, int connfd);

// Hàm xử lý yêu cầu đăng xuất
void handle_logout_request(int connfd);

// Hàm lấy username từ connfd
const char *get_username_from_connfd(int connfd);

// Hàm xử lý yêu cầu đăng ký
void handle_register_request(char *data, int connfd);

void mark_user_as_logged_out(const char *username); // Thêm khai báo hàm này

void handle_login_request(char *data, int connfd)
{
    char username[50] = {0};
    char password[50] = {0};

    // Trích xuất username và password từ data
    if (sscanf(data, "username: %49[^;]; password: %49s", username, password) != 2)
    {
        send_message("LOGIN_RES", "Invalid data format", connfd);
        return;
    }

    // Kiểm tra trạng thái đăng nhập
    if (is_user_logged_in(username)) // Hàm kiểm tra trạng thái đăng nhập
    {
        send_message("LOGIN_RES", "Account already logged in", connfd);
        return;
    }

    // Kiểm tra đăng nhập
    int response = check_login(username, password);
    if (response >= 0)
    {
        // Đánh dấu tài khoản là đã đăng nhập
        mark_user_as_logged_in(username);

        // Lưu thông tin kết nối
        if (connection_count < MAX_CONNECTIONS)
        {
            connections[connection_count].connfd = connfd;
            strncpy(connections[connection_count].username, username, sizeof(connections[connection_count].username) - 1);
            connection_count++;
        }

        char resp_str[12];
        snprintf(resp_str, sizeof(resp_str), "%d", response);
        send_message("LOGIN_RES", resp_str, connfd);
    }
    else
    {
        send_message("LOGIN_RES", "-1", connfd); // Đăng nhập thất bại
    }
}

void handle_register_request(char *data, int connfd)
{
    char username[50] = {0};
    char password[50] = {0};

    // Trích xuất username và password từ data
    if (sscanf(data, "username: %49[^;]; password: %49s", username, password) != 2)
    {
        send_message("REGISTER_RES", "Invalid data format", connfd);
        return;
    }

    // Kiểm tra đăng ký
    int response = insert_users_table(username, password);
    if (response == 0)
    {
        send_message("REGISTER_RES", "0", connfd); // Đăng ký thành công
    }
    else if (response == 1)
    {
        send_message("REGISTER_RES", "1", connfd); // Tài khoản đã tồn tại
    }
    else
    {
        send_message("REGISTER_RES", "2", connfd); // Lỗi đăng ký
    }
}

void handle_logout_request(int connfd)
{
    // Lấy username từ connfd
    const char *username = get_username_from_connfd(connfd);

    if (username)
    {
        // Đánh dấu tài khoản là chưa đăng nhập
        mark_user_as_logged_out(username);

        // Xóa thông tin kết nối
        for (int i = 0; i < connection_count; i++)
        {
            if (connections[i].connfd == connfd)
            {
                // Xóa phần tử bằng cách di chuyển các phần tử sau lên
                for (int j = i; j < connection_count - 1; j++)
                {
                    connections[j] = connections[j + 1];
                }
                connection_count--;
                break;
            }
        }
    }
}

const char *get_username_from_connfd(int connfd)
{
    for (int i = 0; i < connection_count; i++)
    {
        if (connections[i].connfd == connfd)
        {
            return connections[i].username;
        }
    }
    return NULL; // Trả về NULL nếu không tìm thấy
}

#endif // SERVER_FUNCTION_LOGIN_LOGOUT_H
