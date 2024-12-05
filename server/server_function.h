#ifndef SERVER_FUNCTION_H
#define SERVER_FUNCTION_H

#include <stdio.h>
#include <string.h>
#include "./database/database_function.h"
#include <ctype.h>

// Định nghĩa hằng số
#define MAXLINE 4096

// Các hàm xử lý
void process_message(char *msg, int n, int connfd);
void send_message(char *header, char *data, int connfd);
void handle_login_request(char *data, int connfd);
void handle_register_request(char *data, int connfd);
void handle_create_room_request(int connfd);

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
    if (response == 0)
    {
        send_message("LOGIN_RES", "0", connfd); // Đăng nhập thành công
    }
    else
    {
        send_message("LOGIN_RES", "1", connfd); // Đăng nhập thất bại
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

void handle_create_room_request(int connfd)
{
    char response[MAXLINE];
    int numbers = 3; // Số người chơi mặc định khi tạo phòng mới

    // Chèn phòng vào cơ sở dữ liệu
    if (insert_rooms_table(numbers) == 0)
    {
        // Lấy ID phòng mới
        int room_id = get_id_new_room();
        sprintf(response, "Phòng đã được tạo thành công với ID: %d\n", room_id);
    }
    else
    {
        strcpy(response, "Lỗi khi tạo phòng. Vui lòng thử lại.\n");
    }

    // Gửi thông báo về client
    send_message("CREATE_ROOM_RES", response, connfd);
}

// Trích xuất username và password từ data

// server.c
void handle_view_all_room_request(int connfd, char *buffer)
{
    if (strncmp(buffer, "HEADER: VIEW_ALL_ROOMS_REQ", 26) == 0)
    {
        printf("Server received VIEW_ALL_ROOMS_REQ\n");

        // Lấy thông tin các phòng từ cơ sở dữ liệu
        char *rooms_info = get_all_rooms();
        if (rooms_info != NULL)
        {
            // Gửi danh sách phòng về client
            send_message("VIEW_ALL_ROOMS_RES", rooms_info, connfd);
            free(rooms_info); // Giải phóng bộ nhớ đã cấp phát
        }
        else
        {
            // Lỗi khi truy vấn cơ sở dữ liệu
            send_message("ERROR", "Lỗi khi truy vấn danh sách phòng.", connfd);
        }
    }
    else
    {
        // Phản hồi lỗi nếu yêu cầu không hợp lệ
        send_message("ERROR", "Unknown request type", connfd);
    }
}

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
    printf("Server received: %s\n", msg); // Debug thông điệp nhận

    char header[MAXLINE] = {0};
    char data[MAXLINE] = {0};

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
        printf("Received VIEW_ALL_ROOMS_REQ\n");

        // Gọi hàm xử lý yêu cầu
        handle_view_all_room_request(connfd, data);
    }
    else
    {
        printf("Unknown request type: %s\n", header);
        send_message("ERROR", "Unknown request type", connfd); // Phản hồi lỗi
    }

    // Gửi phản hồi về client
    char response[] = "Response from server"; // Thay đổi nội dung phản hồi nếu cần
    send_message("RESPONSE", response, connfd);
    printf("Response sent to client: %s\n", response);
}

#endif // SERVER_FUNCTION_H
