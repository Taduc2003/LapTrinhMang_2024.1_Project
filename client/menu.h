#pragma once

#include <stdio.h>
#include <string.h>
#include <ctype.h> // Để sử dụng isdigit và isalpha
typedef struct users
{
    char username[20];
    char password[20];
} users;
#define MAXLINE 4096

// Kiểm tra username có chứa ký tự '@'
int is_valid_email(const char *username)
{
    return strchr(username, '@') != NULL; // Trả về 1 nếu có '@', 0 nếu không có
}

// Kiểm tra password có ít nhất 1 chữ hoa, 1 số, và 1 ký tự đặc biệt
int is_valid_password(const char *password)
{
    int has_upper = 0, has_digit = 0, has_special = 0;
    const char *special_chars = "!@#$%^&*()-_=+[]{};:'\",.<>?/\\|`~";

    for (int i = 0; password[i] != '\0'; i++)
    {
        if (isupper(password[i]))
            has_upper = 1; // Kiểm tra chữ hoa
        if (isdigit(password[i]))
            has_digit = 1; // Kiểm tra chữ số
        if (strchr(special_chars, password[i]))
            has_special = 1; // Kiểm tra ký tự đặc biệt
    }

    return has_upper && has_digit && has_special; // Trả về 1 nếu đủ điều kiện, 0 nếu không
}

int display_welcome_menu()
{
    printf("---------------WELCOME TO DUNG DE TIEN ROI---------------\n");
    printf("1. Login\n");
    printf("2. Register\n");
    printf("3. Exit\n");
    printf("Please choose 1-3: ...\n");
    int n;
    scanf("%d", &n);

    // Xóa bộ đệm sau khi đọc
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

    while (n < 1 || n > 3)
    {
        printf("Please choose 1 - 3: ");
        scanf("%d", &n);

        // Xóa bộ đệm sau khi đọc
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
    return n;
}

users display_login_menu()
{
    users user_acc;
    char username[MAXLINE], password[MAXLINE];

    printf("Please provide some information to login:\n");

    // Vòng lặp kiểm tra Username
    while (1)
    {
        printf("Username (must contain '@'): ");
        fflush(stdout); // Đảm bảo in ra màn hình ngay lập tức
        if (fgets(username, MAXLINE, stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        trim_whitespace(username); // Loại bỏ khoảng trắng dư thừa

        if (strlen(username) == 0 || !is_valid_email(username))
        {
            printf("Invalid username. It must contain '@'. Please try again.\n");
            continue; // Quay lại nếu username không hợp lệ
        }
        break; // Thoát vòng lặp nếu username hợp lệ
    }

    // Vòng lặp kiểm tra Password
    while (1)
    {
        printf("Password: ");
        fflush(stdout); // Đảm bảo in ra màn hình ngay lập tức
        if (fgets(password, MAXLINE, stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        trim_whitespace(password); // Loại bỏ khoảng trắng dư thừa

        // Không cần kiểm tra điều kiện phức tạp cho login (chỉ cần kiểm tra không rỗng)
        if (strlen(password) == 0)
        {
            printf("Password cannot be empty. Please try again.\n");
            continue;
        }
        break; // Thoát vòng lặp nếu password hợp lệ
    }

    // Lưu username và password vào cấu trúc user_acc
    strcpy(user_acc.username, username);
    strcpy(user_acc.password, password);

    return user_acc;
}

users display_register_menu()
{
    users user_acc;
    char username[MAXLINE], password[MAXLINE];

    printf("Please provide some information to register:\n");

    // Vòng lặp kiểm tra Username
    while (1)
    {
        printf("Username (must contain '@'): ");
        fflush(stdout); // Đảm bảo in ra màn hình ngay lập tức
        if (fgets(username, MAXLINE, stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        trim_whitespace(username); // Loại bỏ khoảng trắng dư thừa

        if (strlen(username) == 0 || !is_valid_email(username))
        {
            printf("Invalid username. It must contain '@'. Please try again.\n");
            continue; // Quay lại nếu username không hợp lệ
        }
        break; // Thoát vòng lặp nếu username hợp lệ
    }

    // Vòng lặp kiểm tra Password
    while (1)
    {
        printf("Password (must have at least 1 uppercase, 1 number, and 1 special character): ");
        fflush(stdout); // Đảm bảo in ra màn hình ngay lập tức
        if (fgets(password, MAXLINE, stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        trim_whitespace(password); // Loại bỏ khoảng trắng dư thừa

        if (strlen(password) == 0 || !is_valid_password(password))
        {
            printf("Invalid password. It must have at least 1 uppercase letter, 1 number, and 1 special character. Please try again.\n");
            continue; // Quay lại nếu password không hợp lệ
        }
        break; // Thoát vòng lặp nếu password hợp lệ
    }

    // Lưu username và password vào cấu trúc user_acc
    strcpy(user_acc.username, username);
    strcpy(user_acc.password, password);

    return user_acc;
}

int display_main_menu()
{
    while (1)
    {
        printf("---------------ROOM MENU---------------\n");
        printf("1. Tao phong\n");
        printf("2. Vao phong theo ID\n");
        printf("3. Xem tat ca cac phong\n");
        printf("4. Dang xuat\n");
        int choice;
        scanf("%d", &choice);
        while (choice < 1 || choice > 4)
        {
            printf("Please choose 1 - 4: ");
            scanf("%d", &choice);
        }
        return choice;
    }
}

int display_in_room_menu(char *room_id)
{
    while (1)
    {
        printf("---------------ROOM %s---------------\n", room_id);
        printf("1. Xem thong tin phong\n");
        printf("2. Sẵn sàng\n");
        printf("3. Thoat Phong\n");
        int choice;
        scanf("%d", &choice);
        while (choice < 1 || choice > 3)
        {
            printf("Please choose 1 - 3: ");
            scanf("%d", &choice);
        }
        return choice;
    }
}
