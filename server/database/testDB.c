#include <stdio.h>
#include <stdlib.h>
#include "database_function.h"

void test_is_user_logged_in()
{
    printf("Testing is_user_logged_in function:\n");

    // Kiểm tra tài khoản đã đăng nhập
    const char *username_logged_in = "user1"; // Giả sử user1 đã đăng nhập
    int result = is_user_logged_in(username_logged_in);
    printf("User: %s, Logged in: %d\n", username_logged_in, result); // Kỳ vọng 1

    // Kiểm tra tài khoản chưa đăng nhập
    const char *username_not_logged_in = "user3"; // Giả sử user3 chưa đăng nhập
    result = is_user_logged_in(username_not_logged_in);
    printf("User: %s, Logged in: %d\n", username_not_logged_in, result); // Kỳ vọng 0
}

void test_mark_user_as_logged_in()
{
    printf("Testing mark_user_as_logged_in function:\n");

    const char *username = "bao"; // Tài khoản cần đánh dấu
    mark_user_as_logged_in(username);

    // Kiểm tra lại trạng thái đăng nhập
    int result = is_user_logged_in(username);
    printf("User: %s, Logged in: %d\n", username, result); // Kỳ vọng 1
}

int main()
{
    // Gọi các hàm test
    display_all_accounts();
    test_is_user_logged_in();
    test_mark_user_as_logged_in();

    return 0;
}
