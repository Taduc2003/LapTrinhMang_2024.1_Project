#ifndef SERVER_FUNCTION_ROOM_H
#define SERVER_FUNCTION_ROOM_H

#include <stdio.h>
#include <string.h>
#include "./database/database_function.h"
#include <ctype.h>
#include "send_question_function.h"
#include "server_function_login_logout.h"
#include "server_function.h"

// Thêm khai báo hàm send_message
void send_message(char *header, char *data, int connfd);

// Định nghĩa hằng số
#define MAXLINE 4096

void handle_create_room_request(int connfd);
void handle_view_all_room_request(int connfd);
void handle_join_room_request(char *data, int connfd);
int process_join_room(char *room_id, int connfd, char *user_id);
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
    insert_users_rooms_table(atoi(room_id), atoi(user_id));

    if (get_current_numbers(atoi(room_id)) == 3)
    {
        update_room_status(atoi(room_id), 1, 3);
    }
    // // Cập nhật thông tin người chơi vào phòng
    // update_player_in_room(atoi(room_id), atoi(user_id), 1, 0); // Giả sử round = 1 và money = 0

    return status;
}
void handle_view_room_info_request(char *data, int connfd)
{
    printf("Server nhận yêu cầu VIEW_ROOM_INFO_REQ\n");
    char room_id[10] = {0};

    // Trích xuất room_id từ data
    if (sscanf(data, "%9s", room_id) != 1)
    {
        send_message("VIEW_ROOM_INFO_RES", "Invalid data format", connfd);
        return;
    }

    printf("Xử lý hiển thị thông tin phòng với ID phòng: %s\n", room_id);

    // Lấy thông tin người chơi trong phòng từ cơ sở dữ liệu
    char *player_info = get_players_in_room(atoi(room_id));
    if (player_info != NULL)
    {
        // Gửi danh sách người chơi về client
        send_message("VIEW_ROOM_INFO_RES", player_info, connfd);
        free(player_info); // Giải phóng bộ nhớ đã cấp phát
    }
    else
    {
        send_message("VIEW_ROOM_INFO_RES", "Không có thông tin phòng hoặc phòng không tồn tại.", connfd);
    }
}

void handle_leave_room_request(char *data, int connfd)
{
    printf("Server nhận yêu cầu LEAVE_ROOM_REQ\n");
    char room_id[10] = {0};
    char user_id[10] = {0};

    // Trích xuất room_id và user_id từ data
    if (sscanf(data, "%9s %9s", room_id, user_id) != 2)
    {
        send_message("LEAVE_ROOM_RES", "Invalid data format", connfd);
        return;
    }

    printf("Xử lý thoát phòng với ID phòng: %s, ID người dùng: %s\n", room_id, user_id);

    // Giảm số lượng người chơi trong phòng
    int currentNumbers = get_current_numbers(atoi(room_id));
    if (currentNumbers > 0)
    {
        update_room_status(atoi(room_id), 0, currentNumbers - 1);
        delete_users_rooms_table(atoi(room_id), atoi(user_id));
        send_message("LEAVE_ROOM_RES", "0", connfd); // Phản hồi thành công
        printf("Người chơi đã thoát phòng. Số lượng người chơi hiện tại: %d\n", currentNumbers - 1);

        // Nếu phòng không còn người chơi, cập nhật trạng thái phòng
        if (currentNumbers - 1 == 0)
        {
            printf("Phòng %s không còn người chơi. Cập nhật trạng thái phòng.\n", room_id);
            update_room_status(atoi(room_id), 2, 0); // Đặt trạng thái phòng là "Đã bị hủy"
        }
    }
    else
    {
        send_message("LEAVE_ROOM_RES", "1", connfd); // Phản hồi lỗi: không thể thoát phòng
        printf("Không thể thoát phòng. Phòng trống hoặc không hợp lệ.\n");
    }
}

