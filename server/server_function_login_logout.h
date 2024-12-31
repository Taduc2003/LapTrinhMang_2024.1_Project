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

void handle_login_request(char *data, int connfd);

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

    // Kiểm tra đăng nhập
    int response = check_login(username, password);
    if (response >= 0)
    {
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
