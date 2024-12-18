#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "menu.h"  // Đảm bảo bao gồm menu.h để sử dụng cấu trúc users
#include <ctype.h> // Thêm thư viện này để sử dụng isspace

#define MAXLINE 4096   /*max text line length*/
#define SERV_PORT 3000 /*port*/
char user_id[MAXLINE];

void send_message(char *header, char *data, int sockfd);
void process_message(char *msg, int n);
void display_data(const char *data);
void trim_whitespace(char *str);
void create_room(int sockfd);
void view_all_rooms(int sockfd);
void join_room(int sockfd, char *room_id, char *user_id);
void login_user(int sockfd);
void register_user(int sockfd);
void menu_user(int sockfd);

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;

    // Kiểm tra tham số đầu vào
    if (argc != 2)
    {
        perror("Usage: TCPClient <IP address of the server");
        exit(1);
    }

    // Tạo socket cho client
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }

    // Cấu hình địa chỉ server
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(SERV_PORT); // Convert to big-endian order

    // Kết nối đến server
    printf("Connecting to server at %s:%d...\n", argv[1], SERV_PORT);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(3);
    }
    printf("Connection established.\n");

    //-----------------------------------------------------------
    // Vòng lặp chương trình client
    while (1)
    {
        int login_or_register = display_welcome_menu();
        printf("Option selected: %d\n", login_or_register); // Debug line

        if (login_or_register == 1) // Đăng nhập
        {
            login_user(sockfd);
        }
        else if (login_or_register == 2) // Xử lý đăng ký
        {
            register_user(sockfd);
        }
        else if (login_or_register == 3) // Thoát chương trình
        {
            printf("Goodbye!\n");
            exit(0);
        }
    }

    exit(0);
}
//-------------------------------------------------------------
void process_message(char *msg, int n)
{
    msg[n] = '\0';                           // Đảm bảo thông điệp là chuỗi null-terminated
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

void register_user(int sockfd)
{
    users sender_acc;
    char header[MAXLINE], data[MAXLINE], recvline[MAXLINE];

    while (1) // Vòng lặp đăng ký
    {
        sender_acc = display_register_menu();
        sprintf(header, "REGISTER_REQ");
        sprintf(data, "username: %s; password: %s", sender_acc.username, sender_acc.password);
        send_message(header, data, sockfd);

        if (recv(sockfd, recvline, MAXLINE, 0) == 0)
        {
            perror("The server terminated prematurely");
            exit(4);
        }

        // Trước tiên, loại bỏ khoảng trắng hoặc ký tự không cần thiết
        trim_whitespace(recvline);

        // Sau đó, kiểm tra phần "DATA" trong thông điệp của server
        char *data_start = strstr(recvline, "DATA: ");
        if (data_start != NULL)
        {
            data_start += 6; // Bỏ qua "DATA: " để chỉ lấy phần giá trị thực tế
            if (strncmp(data_start, "0", 1) == 0)
            { // Kiểm tra dữ liệu "0" (Đăng ký thành công)
                printf("Đăng ký thành công! Quay lại menu chính...\n");
                break; // Thoát vòng lặp đăng ký
            }
            else if (strncmp(data_start, "1", 1) == 0)
            { // Đăng ký thất bại
                printf("Tên đăng nhập đã tồn tại. Vui lòng thử lại.\n");
                printf("1. Thử lại\n");
                printf("2. Quay lại menu chính\n");
                int retry_choice;
                scanf("%d", &retry_choice);

                if (retry_choice == 2)
                { // Quay lại menu chính
                    break;
                }
            }
        }
    }
}

void login_user(int sockfd)
{
    while (1) // Vòng lặp đăng nhập
    {
        users sender_acc;
        char header[MAXLINE], data[MAXLINE], recvline[MAXLINE];

        sender_acc = display_login_menu(); // Hiển thị màn hình nhập thông tin đăng nhập
        sprintf(header, "LOGIN_REQ");
        sprintf(data, "username: %s; password: %s", sender_acc.username, sender_acc.password);
        send_message(header, data, sockfd); // Gửi thông điệp đăng nhập đến server

        if (recv(sockfd, recvline, MAXLINE, 0) == 0) // Kiểm tra phản hồi từ server
        {
            printf("Máy chủ đã kết thúc sớm, không có phản hồi.\n");
            exit(4);
        }

        process_message(recvline, strlen(recvline));
        // Sau khi nhận thông điệp từ server, loại bỏ khoảng trắng và ký tự xuống dòng thừa
        // Trước tiên, loại bỏ khoảng trắng hoặc ký tự không cần thit
        trim_whitespace(recvline);

        // Sau đó, kiểm tra phần "DATA" trong thông điệp của server
        char *data_start = strstr(recvline, "DATA: ");
        if (data_start != NULL)
        {
            data_start += 6; // Bỏ qua "DATA: " để chỉ lấy phần giá trị thực tế
            if (strncmp(data_start, "0", 1) == 0)
            { // Kiểm tra dữ liệu "0" (Đăng nhập thành công)
                printf("Đăng nhập thành công!\n");
                menu_user(sockfd);
                break;
            }
            else if (strncmp(data_start, "-1", 2) == 0)
            { // Đăng nhập thất bại
                printf("Sai tên đăng nhập hoặc mật khẩu. Vui lòng thử lại.\n");
                printf("1. Thử lại\n");
                printf("2. Quay lại menu chính\n");
                int retry_choice;
                scanf("%d", &retry_choice);

                if (retry_choice == 2)
                { // Quay lại menu chính
                    break;
                }
            }
            else if (atoi(data_start) > 0) // Kiểm tra nếu data là ID người dùng
            {
                printf("Đăng nhập thành công với ID: %s\n", data_start);
                menu_user(sockfd);
                break;
            }
            else
            { // Dữ liệu không hợp lệ từ server
                printf("Dữ liệu không hợp lệ từ server: %s\n", data_start);
            }
        }
    }
}

void menu_user(int sockfd)
{
    printf("Kết quả: Đăng nhập thành công\n");
    char room_id[MAXLINE];
    while (1)
    {
        int choice = display_main_menu();
        switch (choice) // Vòng lặp chính
        {
        case 1:
            create_room(sockfd);
            break;
        case 2:

            printf("Nhập ID phòng để tham gia: ");
            scanf("%s", room_id);
            join_room(sockfd, room_id, user_id);
            break;
        case 3:
            view_all_rooms(sockfd);
            break;
        case 4:
            printf("Logging out...\n");
            return;
        default:
            printf("Lựa chọn không hợp lệ. Vui lòng thử lại.\n");
        }
    }
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

void create_room(int sockfd)
{
    char request[200];
    sprintf(request, "HEADER: CREATE_ROOM_REQ; DATA: ");
    send(sockfd, request, strlen(request), 0); // Gửi yêu cầu tạo phòng đến server

    char buffer[MAXLINE];
    int n = recv(sockfd, buffer, MAXLINE, 0); // Nhận phản hồi từ server
    buffer[n] = '\0';                         // Đảm bảo kết thúc chuỗi hợp lệ

    printf("%s\n", buffer); // In ra thông báo từ server (ID phòng hoặc lỗi)
}
void view_all_rooms(int sockfd)
{
    char request[200] = {0};
    char recvline[MAXLINE];
    // Tạo thông điệp gửi đi, chỉ chứa HEADER, không có DATA
    snprintf(request, sizeof(request), "HEADER: VIEW_ALL_ROOMS_REQ;");

    // Gửi yêu cầu đến server
    printf("Chuẩn bị gửi yêu cầu về Xem danh sách phòng ...\n");
    send(sockfd, request, strlen(request), 0); // Gửi thông điệp chỉ chứa HEADER
    printf("Yêu cầu đã gửi: %s\n", request);

    // Nhận phản hồi từ server
    if (recv(sockfd, recvline, MAXLINE, 0) == 0) // Kiểm tra phản hồi từ server
    {
        printf("Máy chủ đã kết thúc sớm, không có phản hồi.\n");
        exit(4);
    }

    process_message(recvline, strlen(recvline)); // Kết thúc chuỗi

    // Hiện thị thông báo kết thúc
    printf("\nKết thúc phản hồi về Xem danh sách phòng .\n");
}

void join_room(int sockfd, char *room_id, char *user_id)
{
    char recvline[MAXLINE];

    while (1)
    {
        printf("Đang gửi yêu cầu về tham gia phòng...\n");

        char request[200];
        sprintf(request, "HEADER: JOIN_ROOM_REQ; DATA: %s %s", room_id, user_id);
        send(sockfd, request, strlen(request), 0); // Gửi yêu cầu về tham gia phòng

        // Nhận phản hồi từ server
        if (recv(sockfd, recvline, MAXLINE, 0) == 0) // Kiểm tra phản hồi từ server
        {
            printf("Máy chủ đã kết thúc sớm, không có phản hồi.\n");
            exit(4);
        }

        process_message(recvline, strlen(recvline)); // Kết thúc chuỗi

        // Sau khi vào phòng thành công thì add vào main game
        char *data_start = strstr(recvline, "DATA: ");
        if (data_start != NULL)
        {
            data_start += 6; // Bỏ qua "DATA: " để chỉ lấy phần giá trị thực tế
            if (strncmp(data_start, "0", 1) == 0)
            { // Kiểm tra dữ liệu "0" (Đăng nhập thành công)
                printf("Vào phòng thành công!\n");
                while (1)
                {
                    int choice = display_in_room_menu(room_id);
                    if (choice == 1)
                    {
                        printf("Hiển thị thông tin phòng\n");
                    }
                    else if (choice == 2)
                    {
                        printf("Thoat phong\n");
                        return;
                    }
                }
            }
            else if (strncmp(data_start, "1", 1) == 0)
            {
                printf("Bạn không thể vào phòng vì một vài lí do. Vui lòng thử lại.\n");
                printf("1. Thử lại\n");
                printf("2. Quay lại menu chính\n");
                int retry_choice;
                scanf("%d", &retry_choice);

                if (retry_choice == 2)
                { // Quay lại menu chính
                    break;
                }
            }
            else
            { // Dữ liệu không hợp lệ từ server
                printf("Dữ liệu không hợp lệ từ server: %s\n", data_start);
            }
        }
        // Hiện thị thông báo kết thúc
        printf("\nKết thúc phản hồi về Join phòng .\n");
    }
    
}
