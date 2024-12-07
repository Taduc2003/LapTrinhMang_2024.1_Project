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
void handle_view_all_room_request(int connfd);
void handle_join_room_request(char *data, int connfd);
int process_join_room(char *room_id, int connfd, char *user_id);

extern char user_id[MAXLINE]; // Khai báo biến user_id

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
        char resp_str[10];
        sprintf(resp_str, "%d", response);
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

void handle_create_room_request(int connfd)
{
    char response[MAXLINE];
    int numbers = 0; 

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

void handle_view_all_room_request(int connfd)
{
        printf("Server đã nhận yêu cầu VIEW_ALL_ROOMS_REQ\n");
        printf("Đang xử lí Xem danh sách phòng...\n");
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

void handle_join_room_request(char *data, int connfd)
{
    printf("Servereah nhận yêu cầu JOIN_ROOM_REQ\n");
    char room_id[10] = {0};
    char user_id[10] = {0};

    // Trích xuất ID phòng và ID người dùng từ data
    if (sscanf(data, "%9s %9s", room_id, user_id) != 2)
    {
        send_message("JOIN_ROOM_RES", "Invalid data format", connfd);
        return;
    }
    printf("Đang xử lí trong data với ID phòng: %s, ID người dùng: %s\n", room_id, user_id);
    int response = process_join_room(room_id, connfd, user_id); // Gọi hàm join_room với kiểu dữ liệu đúng
    if (response == 0)
    {
        send_message("JOIN_ROOM_RES", "0", connfd); // Tham gia phòng thành công
    }
    else
    {
        send_message("JOIN_ROOM_RES", "1", connfd); // Lỗi khi tham gia phòng
    }
}

int process_join_room(char *room_id, int connfd, char *user_id)
{
    int status = check_room_status(atoi(room_id));
        if (status == -1)
        {
            printf("Phòng không tồn tại.\n");
            return 1;
        }
        else if (status == 1)
        {
            printf("Phòng đang chơi.\n");
            return 1;
        }
        else if (status == 0)
        {
            printf("Phòng đang chờ.\n");
            // Tiến hành xử lý tham gia phòng
        }
        else if (status == 2)
        {
            printf("Phòng đã bị huỷ.\n");
            return 1;
        }

    // Cập nhật số lượng người trong phòng
    int currentNumbers = get_current_numbers(atoi(room_id)); // Hàm này cần được định nghĩa để lấy số lượng hiện tại
    update_room_status(atoi(room_id), status, currentNumbers + 1);

    // Cập nhật thông tin người chơi vào phòng
    update_player_in_room(atoi(room_id), atoi(user_id), 1, 0); // Giả sử round = 1 và money = 0

    return status;
}

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
    
    else
    {
        printf("Header không hợp lệ: %s\n", header);
        send_message("ERROR", "HEADER không hợp lệ", connfd); // Phản hồi lỗi
    }
}

#endif // SERVER_FUNCTION_H
