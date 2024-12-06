#include "database_function.h"
#include <assert.h>
#include <stdio.h>

// Khai báo hàm delete_test_user
void delete_test_user(const char* username) {
    // Định nghĩa hàm xóa người dùng
    printf("User %s has been deleted.\n", username);
    // Thực hiện xóa người dùng từ cơ sở dữ liệu ở đây
}

void test_check_login_success()
{
    // Set up
    insert_users_table("testuser", "password123");

    // Test
    assert(check_login("testuser", "password123") == 0);

    // Clean up
    delete_test_user("testuser"); // Tạo hàm xóa sau
}

void test_check_login_wrong_password()
{
    insert_users_table("testuser", "password123");
    assert(check_login("testuser", "wrongpassword") == 1);
    delete_test_user("testuser");
}

void test_check_login_user_not_found()
{
    assert(check_login("nonexistentuser", "password123") == 2);
}
void test_get_all_rooms()
{
    char *rooms = get_all_rooms();
    if (rooms)
    {
        printf("%s", rooms);
        free(rooms); // Giải phóng bộ nhớ sau khi sử dụng
    }
    else
    {
        printf("Không thể lấy danh sách các phòng.\n");
    }
}

int main()
{
    display_users_table();
    insert_rooms_table(4);
    insert_rooms_table(2);
    insert_rooms_table(6);

    // Kiểm tra hàm get_all_rooms
    test_get_all_rooms();
    return 0;
}
