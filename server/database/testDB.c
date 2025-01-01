#include "database_function.h"
#include <assert.h>
#include <stdio.h>

// Khai báo hàm delete_test_user
void delete_test_user(const char *username)
{
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
    int testRoomId = 14;                         // Thay đổi roomId này để kiểm tra
    display_ranking_round_by_roomId(testRoomId); // Gọi hàm hiển thị
}
void test_get_players_in_room()
{
    // Set up: Thêm dữ liệu thử nghiệm
    insert_users_table("testuser1", "password1");
    insert_users_table("testuser2", "password2");
    insert_users_table("testuser3", "password3");

    // Thêm phòng thử nghiệm
    insert_rooms_table(0);          // Tạo một phòng mới, số người chơi ban đầu = 0
    int roomId = get_id_new_room(); // Lấy ID của phòng vừa tạo

    // Thêm người chơi vào phòng (ranking_round)
    insert_ranking_round_table(roomId, 1, 1, 1000); // userId = 1
    insert_ranking_round_table(roomId, 2, 1, 2000); // userId = 2
    insert_ranking_round_table(roomId, 3, 1, 3000); // userId = 3

    // Test: Gọi hàm get_players_in_room
    char *players = get_players_in_room(roomId);
    if (players)
    {
        printf("Danh sách người chơi trong phòng %d:\n%s", roomId, players);
        free(players); // Giải phóng bộ nhớ
    }
    else
    {
        printf("Không tìm thấy người chơi trong phòng %d.\n", roomId);
    }

    // Clean up: Xóa dữ liệu thử nghiệm
    // Lưu ý: Bạn cần triển khai hàm xóa dữ liệu thử nghiệm nếu cần.
}

int main()
{
    // test_display_ranking_round_by_roomId();

    // Nếu chưa có data hãy chạy này
    //---------------------------------------
    // create_users_table();
    // create_questions_table();
    // create_rooms_table();
    // create_ranking_round_table();

    // insert_users_table("user1@", "U1#");
    // insert_users_table("user2@", "U2#");
    // insert_users_table("user3@", "U3#");
    // insert_sample_questions(); // Thêm câu hỏi
    //---------------------------------------
    // test_get_players_in_room();
    // get_all_user_table();
    return 0;
}
