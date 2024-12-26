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
void test_display_ranking_round_by_roomId()
{
    int testRoomId = 14;                          // Thay đổi roomId này để kiểm tra
    display_ranking_round_by_roomId(testRoomId); // Gọi hàm hiển thị
}
int main()
{
    //test_display_ranking_round_by_roomId();


    // Nếu chưa có data hãy chạy này
    //---------------------------------------
    // create_users_table();
    // create_questions_table();
    // create_rooms_table();
    // create_ranking_round_table();

    // insert_users_table("user1", "u1");
    // insert_users_table("user2", "u2");
    // insert_users_table("user3", "u3");
    // insert_sample_questions(); // Thêm câu hỏi 
    //---------------------------------------
    return 0;
}
