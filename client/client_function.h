#ifndef CLIENT_FUNCTION_H
#define CLIENT_FUNCTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include "client_function.h"
#include "menu.h"
#include "client_function_game_2.h"
#define MAXLINE 4096

char user_id[MAXLINE];
void send_message(char *header, char *data, int sockfd);
void process_message(char *msg, int n);
void display_data(const char *data);
void trim_whitespace(char *str);
void process_message_in_game(char *msg, int n, int sockfd);

void process_message(char *msg, int n)
{
    msg[n] = '\0';                                       // Đảm bảo thông điệp là chuỗi null-terminated
    printf("Processing message from server: %s\n", msg); // Có thể giữ lại dòng này cho debug

    char header[MAXLINE];
    char data[MAXLINE];

    // Phân tách HEADER và DATA từ thông điệp
    char *header_start = strstr(msg, "HEADER: ");
    char *data_start = strstr(msg, "DATA: ");

    if (header_start && data_start)
    {
        header_start += 8; // Bỏ qua "HEADER: "
        char *header_end = strstr(header_start, ";");
        if (header_end)
        {
            strncpy(header, header_start, header_end - header_start);
            header[header_end - header_start] = '\0'; // Kết thúc chuỗi
        }
        else
        {
            fprintf(stderr, "Invalid message format: HEADER end not found\n");
            return;
        }

        // Lấy DATA
        data_start += 6; // Bỏ qua "DATA: "
        strncpy(data, data_start, MAXLINE - 1);
        data[MAXLINE - 1] = '\0'; // Kết thúc chuỗi
    }
    else
    {
        fprintf(stderr, "Invalid message format\n");
        return;
    }

    // In thông báo từ server
    printf("---------------------\n");
    printf("HEADER: %s\n", header);
    printf("DATA: %s\n", data);
    printf("---------------------\n");

    // Xử lý phản hồi đăng nhập
    if (strcmp(header, "LOGIN_RES") == 0)
    {
        if (strcmp(data, "0") == 0)
        {
            printf("Đăng nhập thành công!\n");
        }
        else if (strcmp(data, "-1") == 0)
        {
            printf("Đăng nhập thất bại\n");
        }
        else if (atoi(data) > 0) // Kiểm tra nếu data là ID người dùng
        {
            strncpy(user_id, data, sizeof(user_id) - 1); // Lưu ID người dùng
            user_id[sizeof(user_id) - 1] = '\0';         // Đảm bảo chuỗi kết thúc
            printf("Đăng nhập thành công với ID: %s\n", data);
        }
        else
        {
            printf("Thông báo từ server: %s\n", data);
        }
        return;
    }

    // Xử lý phản hồi đăng ký
    if (strcmp(header, "REGISTER_RES") == 0)
    {
        if (strcmp(data, "0") == 0)
        {
            printf("Đăng ký thành công! Quay lại menu chính...\n");
        }
        else if (strcmp(data, "1") == 0)
        {
            printf("Đăng ký thất bại. Tên đăng nhập đã tồn tại.\n");
        }
        else
        {
            printf("Thông báo từ server: %s\n", data);
        }
        return;
    }

    // Xử lý phản hồi xem danh sách phòng
    if (strcmp(header, "VIEW_ALL_ROOMS_RES") == 0)
    {
        if (strcmp(data, "0") == 0)
        {
            printf("Xem danh sách phòng thất bại.\n");
        }
        else
        {
            printf("Danh sách phòng:\n");
            printf("%s\n", data);
        }
        return;
    }

    // Xử lý phản hồi join phòng
    if (strcmp(header, "JOIN_ROOM_RES") == 0)
    {
        if (strcmp(data, "0") == 0)
        {
            printf("Tham gia phòng thành công.\n");
        }
        else
        {
            printf("Tham gia phòng thất bại!\n");
        }
        return;
    }

    if (strcmp(header, "LOGOUT_RES") == 0)
    {
        printf("%s\n", data);
        return;
    }

    if (strcmp(header, "WAITING_FOR_PLAYERS") == 0)
    {
        printf("Đang chờ người chơi khác tham gia...\n");
        return;
    }

    if (strcmp(header, "GAME_START") == 0)
    {
        printf("Game đã bắt đầu!\n");
        return;
    }

    printf("Phản hồi không xác định từ server.\n");
    // Xử lý các phản hồi khác nếu cần
}

void send_message(char *header, char *data, int sockfd)
{
    char sendline[MAXLINE];
    // Nu data trống, không cần phải thêm phần đó vào thông điệp
    if (data == NULL || strlen(data) == 0)
    {
        snprintf(sendline, sizeof(sendline), "HEADER: %s", header);
    }
    else
    {
        snprintf(sendline, sizeof(sendline), "HEADER: %s; DATA: %s", header, data);
    }
    send(sockfd, sendline, strlen(sendline), 0);

    printf("Client yêu cầu: %s\n", sendline);
}
void display_data(const char *data)
{
    if (data == NULL)
    {
        printf("Dữ liệu rỗng.\n");
        return;
    }
    printf("Dữ liệu: %s\n", data);
}
void trim_whitespace(char *str)
{
    char *end;

    // Bỏ khoảng trắng đầu chuỗi
    while (isspace((unsigned char)*str))
    {
        str++;
    }

    if (*str == 0) // Nếu chuỗi rỗng sau khi loại bỏ khoảng trắng đầu
        return;

    // Bỏ khoảng trắng cuối chuỗi
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
    {
        end--;
    }

    if (*end == ';')
    { // Nếu có dấu chấm phẩy ở cuối, loại bỏ luôn
        end--;
    }

    *(end + 1) = '\0'; // Kết thúc chuỗi tại vị trí cuối cùng của nó
}

void process_message_in_game(char *msg, int n, int sockfd)
{
    msg[n] = '\0';                                       // Đảm bảo thông điệp là chuỗi null-terminated
    printf("Processing message from server: %s\n", msg); // Có thể giữ lại dòng này cho debug

    char header[MAXLINE];
    char data[MAXLINE];

    // Phân tách HEADER và DATA từ thông điệp
    char *header_start = strstr(msg, "HEADER: ");
    char *data_start = strstr(msg, "DATA: ");

    if (header_start && data_start)
    {
        header_start += 8; // Bỏ qua "HEADER: "
        char *header_end = strstr(header_start, ";");
        if (header_end)
        {
            strncpy(header, header_start, header_end - header_start);
            header[header_end - header_start] = '\0'; // Kết thúc chuỗi
        }
        else
        {
            fprintf(stderr, "Invalid message format: HEADER end not found\n");
            return;
        }

        // Lấy DATA
        data_start += 6; // Bỏ qua "DATA: "
        strncpy(data, data_start, MAXLINE - 1);
        data[MAXLINE - 1] = '\0'; // Kết thúc chuỗi
    }
    else
    {
        fprintf(stderr, "Invalid message format\n");
        return;
    }

    // In thông báo từ server
    printf("---------------------\n");
    printf("HEADER: %s\n", header);
    printf("DATA: %s\n", data);
    printf("---------------------\n");

    if (strcmp(header, "QUESTION") == 0)
    {
        handle_question(data, sockfd);
        return;
    }

    if (strcmp(header, "ANSWER_RESULT") == 0)
    {
        printf("%s\n", data);
        return;
    }

    if (strcmp(header, "OVER_MONEY") == 0)
    {
        printf("%s\n", data);
        return;
    }

    if (strcmp(header, "GAME_OVER") == 0)
    {
        printf("%s\n", data);
        return;
    }

    if (strcmp(header, "WIN") == 0)
    {
        printf("%s\n", data);
        return;
    }

    if (strcmp(header, "RANKING") == 0)
    {
        printf("%s\n", data);
        return;
    }

    printf("Phản hồi không xác định từ server. khi xu li game%s\n", msg);
    // Xử lý các phản hồi khác nếu cần
}

#endif // CLIENT_FUNCTION_H
