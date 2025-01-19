#ifndef SERVER_H
#define SERVER_H

#define MAX_CLIENTS 30

typedef struct
{
    int socket;        // Socket descriptor
    char username[50]; // Tên tài khoản đăng nhập
    int is_logged_in;  // 1 nếu đã đăng nhập, 0 nếu chưa
    int room_id;     // ID của phòng đang tham gia (nếu có) mặc định nếu chưa tham gia là 00
} ClientStatus;

extern ClientStatus clients_status[MAX_CLIENTS];

#endif // SERVER_H
