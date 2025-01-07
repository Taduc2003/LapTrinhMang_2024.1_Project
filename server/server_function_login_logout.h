#ifndef SERVER_FUNCTION_LOGIN_LOGOUT_H
#define SERVER_FUNCTION_LOGIN_LOGOUT_H

#include <stdio.h>
#include <string.h>
#include "./database/database_function.h"
#include <ctype.h>
#include "send_question_function.h"
#include "server_function_room.h"
#include "server_function.h"
#include "server.h"
// Định nghĩa hằng số
#define MAXLINE 4096
extern char user_id[MAXLINE]; // Khai báo biến user_id
// ... existing code ...
int get_user_status(const char *username); // Thêm khai báo hàm này
// ... existing code ...
void handle_login_request(char *data, int connfd);

void handle_logout_request(char *data, int connfd);

void handle_logout_request(char *data, int connfd)
{
    char userid[50] = {0};

    // Trích xuất userid từ data
    if (sscanf(data, "userid: %49s", userid) != 1)
    {
        send_message("LOGOUT_RES", "Invalid data format", connfd);
        return;
    }

    // Tìm username theo userid từ database
    char *username = get_username_from_database(userid);
    if (username == NULL)
    {
        send_message("LOGOUT_RES", "User not found", connfd);
        return;
    }

    // Cập nhật trạng thái của tài khoản về 0 (không đăng nhập)
    update_user_status(username, 0);

    // Gửi thông báo về client rằng đã đăng xuất thành công
    send_message("LOGOUT_RES", "Logged out successfully", connfd);
}


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

    // Kiểm tra trạng thái đăng nhập của tài khoản
    int status = get_user_status(username); // Lấy trạng thái từ DB (1 = đang đăng nhập, 0 = chưa)
    if (status == 1)
    {
        send_message("LOGIN_RES", "User already logged in", connfd);
        return;
    }

    // Kiểm tra xem socket này đã được ánh xạ với tài khoản khác chưa
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients_status[i].socket == connfd && clients_status[i].is_logged_in)
        {
            send_message("LOGIN_RES", "Socket already in use", connfd);
            return;
        }
    }

    // Kiểm tra đăng nhập (username và password)
    int response = check_login(username, password);
    if (response >= 0)
    {
        // Đánh dấu tài khoản này đang đăng nhập
        update_user_status(username, 1); // Đặt trạng thái `status` = 1 trong DB

        // Cập nhật trạng thái cho socket
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients_status[i].socket == 0)
            {
                clients_status[i].socket = connfd;
                strncpy(clients_status[i].username, username, sizeof(clients_status[i].username));
                clients_status[i].is_logged_in = 1;
                break;
            }
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

#endif // SERVER_FUNCTION_LOGIN_LOGOUT_H
