#ifndef SERVER_FUNCTION_H
#define SERVER_FUNCTION_H

#include <stdio.h>
#include <string.h>
#include "./database/database_function.h"
#include <ctype.h>
#include "send_question_function.h"
// Định nghĩa hằng số
#define MAXLINE 4096

// Các hàm xử lý
void process_message(char *msg, int n, int connfd);
void send_message(char *header, char *data, int connfd);
void handle_login_request(char *data, int connfd);
void handle_register_request(char *data, int connfd);
void handle_create_room_request(int connfd);
void handle_view_all_room_request(int connfd);
void handle_join_room_request(char *data, int connfd, char *user_id);
int join_room(char *room_id, int connfd, char *user_id);

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
        send_message("LOGIN_RES", "response", connfd); // Đăng nhập thành công
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

void handle_join_room_request(char *data, int connfd, char *user_id)
{
    char room_id[10] = {0};

    // Trích xuất ID phòng từ data
    if (sscanf(data, "room_id: %9s", room_id) != 1)
    {
        send_message("JOIN_ROOM_RES", "Invalid data format", connfd);
        return;
    }

    int room_id_int = atoi(room_id); // Chuyển đổi room_id thành số nguyên
    char room_id_str[10];
    sprintf(room_id_str, "%d", room_id_int); // Chuyển đổi số nguyên thành chuỗi
    int response = join_room(room_id_str, connfd, user_id); // Gọi hàm join_room với kiểu dữ liệu đúng
    if (response == 0)
    {
        send_message("JOIN_ROOM_RES", "0", connfd); // Tham gia phòng thành công
    }
    else
    {
        send_message("JOIN_ROOM_RES", "1", connfd); // Lỗi khi tham gia phòng
    }
}

int join_room(char *room_id, int connfd, char *user_id)
{
    int status = check_room_status(atoi(room_id));
    if (status == -1)
    {
        printf("Phòng không tồn tại.\n");
        return 0;
    }
    else if (status == 1)
    {
        printf("Phòng đang chơi.\n");
        return 0;
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
        handle_join_room_request(data, connfd, user_id);
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
