#pragma once
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

#define DATABASE_FILE "./server/database/database.db"
#define MAX_QUESTIONS 50

sqlite3 *db;

typedef struct Question
{
    int id;
    char content[1024];
    char ans1[128];
    char ans2[128];
    char ans3[128];
    int correctAns;
    int level;
} Question;

typedef struct Ranking
{
    char username[128];
    int money;
} Ranking;

void create_users_table();
void create_questions_table();
void create_rooms_table();
void create_ranking_round_table();

void initialize_database()
{
    create_users_table();         // Tạo bảng users
    create_questions_table();     // Tạo bảng questions
    create_rooms_table();         // Tạo bảng rooms
    create_ranking_round_table(); // Tạo bảng ranking_round
}
void open_database()
{
    if (sqlite3_open(DATABASE_FILE, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Error: Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
}

void close_database()
{
    sqlite3_close(db);
}

void create_users_table();
int search_users_table(char *username);
int insert_users_table(char *username, char *password);
int check_login(char *username, char *password);

void create_questions_table();
int insert_questions_table(char *content, char *ans1, char *ans2, char *ans3, int correctAns, int level);
void insert_sample_questions();
Question *search_questions_by_level(int level);

void create_rooms_table();
int insert_rooms_table(int numbers);
int search_rooms_by_id(int id);
int get_id_new_room();
int update_room_table(int id, int status, int numbers);
char *get_all_rooms();

void create_ranking_round_table();
int insert_ranking_round_table(int roomId, int userId, int round, int money);
Ranking *search_rankings_by_roomId_round(int roomId, int round);

void create_users_table()
{
    open_database();
    const char *create_table_query =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "password TEXT NOT NULL,"
        "status INTEGER  DEFAULT 1" // Mac dinh la 1
        ");";

    if (sqlite3_exec(db, create_table_query, 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Error: Can't create table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
}

int search_users_table(char *username)
{
    open_database();
    int check = 0;
    const char *select_query = "SELECT * FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            check = 1;
            int id = sqlite3_column_int(stmt, 0);                                       // Removed unused variable
            const char *retrievedUsername = (const char *)sqlite3_column_text(stmt, 1); // Removed unused variable
            const char *retrievedPassword = (const char *)sqlite3_column_text(stmt, 2); // Removed unused variable

            printf("User found:\n");
            printf("ID: %d\n", id);
            printf("Username: %s\n", retrievedUsername);
            printf("Password: %s\n", retrievedPassword);
        }

        sqlite3_finalize(stmt);
        if (check == 1)
            return 0;
        return 2;
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    close_database();
    // return state
    // 0 = found
    // 1 = fail
    // 2 = not found
}

int insert_users_table(char *username, char *password)
{

    open_database();
    int check_username_exist = search_users_table(username);

    if (check_username_exist == 0)
    {
        fprintf(stderr, "Username has existed Cannot Add\n");
        return 1;
    }
    const char *insert_query = "INSERT INTO users (username, password) VALUES (?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, insert_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "Error: Can't insert data: %s\n", sqlite3_errmsg(db));
        }
        else
        {
            printf("users inserted successfully.\n");
        }

        sqlite3_finalize(stmt);
        return 0;
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        return 2;
    }
    close_database();
    // return state
    // 0 = success
    // 1 = fail, users exists
    // 2 = fail
}

void display_users_table()
{
    open_database(); // Đảm bảo cơ sở dữ liệu đã mở

    const char *query = "SELECT id, username, password, status FROM users;";
    sqlite3_stmt *stmt;

    // Chuẩn bị truy vấn
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK)
    {
        printf("ID\tUsername\tPassword\tStatus\n");
        printf("------------------------------------------\n");

        // Duyệt qua từng dòng kết quả
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            const char *username = (const char *)sqlite3_column_text(stmt, 1);
            const char *password = (const char *)sqlite3_column_text(stmt, 2);
            int status = sqlite3_column_int(stmt, 3);

            // Hiển thị thông tin từng dòng
            printf("%d\t%s\t\t%s\t\t%d\n", id, username, password, status);
        }

        // Hoàn thành truy vấn
        sqlite3_finalize(stmt);
    }
    else
    {
        fprintf(stderr, "Error: Failed to execute query: %s\n", sqlite3_errmsg(db));
    }

    close_database(); // Đảm bảo cơ sở dữ liệu được đóng
}

int check_login(char *username, char *password)

{

    open_database();

    int userId = -1; // Khởi tạo userId là -1

    const char *select_query = "SELECT id, password FROM users WHERE username = ?;"; // Chọn id và password

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, 0) == SQLITE_OK)
    {

        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) // Kiểm tra nếu có dòng kết quả
        {

            userId = sqlite3_column_int(stmt, 0); // Lấy id của user

            const char *retrievedPassword = (const char *)sqlite3_column_text(stmt, 1);

            if (strcmp(password, retrievedPassword) == 0) // So sánh mật khẩu

            {
                printf("Login successful\n");

                sqlite3_finalize(stmt);

                close_database();

                return userId; // Trả về id của user
            }

            else

            {

                printf("Login failed, incorrect password\n");
            }
        }

        else

        {

            printf("Login failed, user not found\n");
        }

        sqlite3_finalize(stmt);
    }

    else

    {

        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }

    close_database();

    return -1; // Trả về -1 nếu không tìm thấy user
}

void create_questions_table()
{
    open_database();
    const char *create_table_query =
        "CREATE TABLE IF NOT EXISTS questions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "content TEXT NOT NULL,"
        "ans1 TEXT NOT NULL,"
        "ans2 TEXT NOT NULL,"
        "ans3 TEXT NOT NULL,"
        "correctAns INTEGER NOT NULL," // Tu 1-3 tuong ung voi 3 dap an
        "level INTEGER NOT NULL"
        ");";

    if (sqlite3_exec(db, create_table_query, 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Error: Can't create table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
}

int insert_questions_table(char *content, char *ans1, char *ans2, char *ans3, int correctAns, int level)
{

    open_database();
    // int check_username_exist = search_users_table(username);

    // if (check_username_exist == 0)
    // {
    //     fprintf(stderr, "Username has existed Cannot Add\n");
    //     return 1;
    // }
    const char *insert_query = "INSERT INTO questions (content, ans1, ans2, ans3, correctAns, level) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, insert_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, content, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, ans1, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, ans2, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, ans3, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, correctAns);
        sqlite3_bind_int(stmt, 6, level);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "Error: Can't insert data: %s\n", sqlite3_errmsg(db));
        }
        else
        {
            printf("question inserted successfully.\n");
        }

        sqlite3_finalize(stmt);
        return 0;
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    close_database();
    // return state
    // 0 = success
    // 1 = fail, users exists
    // 2 = fail
}
// Lấy các câu hỏi theo từng mức độ
Question *search_questions_by_level(int level)
{
    open_database(); // Mở kết nối cơ sở dữ liệu

    const char *select_query = "SELECT id, content, ans1, ans2, ans3, correctAns, level FROM questions WHERE level = ?;";
    sqlite3_stmt *stmt;
    Question *questions = malloc(sizeof(Question) * MAX_QUESTIONS); // Cấp phát mảng
    if (!questions)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    int count = 0; // Khởi tạo số lượng câu hỏi tìm thấy

    // Chuẩn bị câu lệnh truy vấn
    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, 0) == SQLITE_OK)
    {
        // Gán giá trị tham số cho câu truy vấn
        sqlite3_bind_int(stmt, 1, level);

        // Duyệt qua từng kết quả
        while (sqlite3_step(stmt) == SQLITE_ROW && count < MAX_QUESTIONS)
        {
            // Lấy dữ liệu từ hàng hiện tại
            questions[count].id = sqlite3_column_int(stmt, 0);
            strncpy(questions[count].content, (const char *)sqlite3_column_text(stmt, 1), sizeof(questions[count].content) - 1);
            strncpy(questions[count].ans1, (const char *)sqlite3_column_text(stmt, 2), sizeof(questions[count].ans1) - 1);
            strncpy(questions[count].ans2, (const char *)sqlite3_column_text(stmt, 3), sizeof(questions[count].ans2) - 1);
            strncpy(questions[count].ans3, (const char *)sqlite3_column_text(stmt, 4), sizeof(questions[count].ans3) - 1);
            questions[count].correctAns = sqlite3_column_int(stmt, 5);
            questions[count].level = sqlite3_column_int(stmt, 6);

            count++; // Tăng số lượng câu hỏi tìm thấy
        }

        sqlite3_finalize(stmt); // Giải phóng câu lệnh
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        free(questions); // Giải phóng bộ nhớ nếu xảy ra lỗi
        questions = NULL;
    }

    close_database(); // Đóng cơ sở dữ liệu

    return questions; // Trả về mảng các câu hỏi
}
// Thêm các câu hỏi theo level từ 1 đến 10
void insert_sample_questions()
{
    // Level 1
    insert_questions_table("Ai là người sáng lập ra Microsoft?", "Steve Jobs", "Bill Gates", "Mark Zuckerberg", 2, 1);
    insert_questions_table("Ai là người sáng lập ra Google?", "Larry Page", "Elon Musk", "Jeff Bezos", 1, 1);
    insert_questions_table("Ai là người sáng lập ra Facebook?", "Elon Musk", "Mark Zuckerberg", "Larry Page", 2, 1);

    // Level 2
    insert_questions_table("Thủ đô của Việt Nam là gì?", "Hà Nội", "Hồ Chí Minh", "Đà Nẵng", 1, 2);
    insert_questions_table("Thành phố nào là thủ đô của Anh?", "Manchester", "Liverpool", "London", 3, 2);
    insert_questions_table("Thành phố nào là thủ đô của Đức?", "Frankfurt", "Berlin", "Munich", 2, 2);

    // Level 3
    insert_questions_table("Ai là tác giả của tác phẩm 'Truyện Kiều'?", "Nguyễn Du", "Nguyễn Trãi", "Nguyễn Đình Chiểu", 1, 3);
    insert_questions_table("Ai là người đầu tiên đặt chân lên mặt trăng?", "Yuri Gagarin", "Neil Armstrong", "Buzz Aldrin", 2, 3);
    insert_questions_table("Ai là hiệu trưởng đầu tiên của Đại học Bách Khoa Hà Nội?", "Nguyễn Văn Đạo", "Hoàng Xuân Sính", "Trần Đại Nghĩa", 3, 3);

    // Level 4
    insert_questions_table("Nước nào có diện tích lớn nhất thế giới?", "Trung Quốc", "Nga", "Mỹ", 2, 4);
    insert_questions_table("Con vật nào là biểu tượng của nước Úc?", "Gấu Koala", "Gấu trúc", "Kangaroo", 3, 4);
    insert_questions_table("Bảy chú lùn trong truyện 'Nàng bạch tuyết và bảy chú lùn làm nghề gì'?", "Thợ may", "Thở rèn", "Thợ săn", 3, 4);

    // Level 5
    insert_questions_table("Nguyên tố hóa học Vàng có ký hiệu là gì?", "Al", "Ag", "Au", 3, 5);
    insert_questions_table("Trái cây nào có nhiều vitamin C nhất?", "Chuối", "Táo", "Cam", 3, 5);
    insert_questions_table("Công thức hóa học của đá vôi?", "CaSo3", "CaSo4", "CaCo3", 3, 5);

    // Level 6
    insert_questions_table("Haiku là thể thơ truyền thống của nước nào?", "Nhật Bản", "Hàn Quốc", "Trung Quốc", 1, 6);
    insert_questions_table("Nguyên tố hóa học nào có ký hiệu là H?", "Helium", "Hydrogen", "Hafnium", 2, 6);
    insert_questions_table("Thành phố nào được mệnh danh là thành phố ngàn hoa?", "Đà Lạt", "Hà Nội", "Hồ Chí Minh", 1, 6);

    // Level 7
    insert_questions_table("Đại học Bách Khoa Hà Nội nằm ở quận nào của Hà Nội?", "Hoàn Kiếm", "Hai Bà Trưng", "Đống Đa", 2, 7);
    insert_questions_table("Khoa nào là khoa đầu tiên được thành lập tại Đại học Bách Khoa Hà Nội?", "Khoa Cơ khí", "Khoa Điện", "Khoa Hóa học", 1, 7);
    insert_questions_table("Đại học Bách Khoa Hà Nội có bao nhiêu khoa?", "20", "25", "30", 2, 7);

    // Level 8
    insert_questions_table("Ai là người chỉ huy chiến thắng trận Điện Biên Phủ năm 1954?", "Võ Nguyên Giáp", "Hồ Chí Minh", "Phạm Văn Đồng", 1, 8);
    insert_questions_table("Sông nào dài nhất ở Việt Nam?", "Sông Đà", "Sông Hồng", "Sông Mekong", 2, 8);
    insert_questions_table("Đội tuyển bóng đá nào vô địch World Cup 2018?", "Brazil", "Pháp", "Đức", 2, 8);

    // Level 9
    insert_questions_table("Tổ chức nào thường được gọi là 'Ngân hàng Thế giới'", "FBI", "WHO", "WB", 3, 9);
    insert_questions_table("Ai là tác giả của bài hát 'Tiến quân ca,' quốc ca Việt Nam?", "Văn Cao", "Trịnh Công Sơn", "Phạm Tuyên", 1, 9);
    insert_questions_table("Hành tinh nào trong hệ Mặt Trời có nhiều vệ tinh nhất?", "Sao Thổ", "Sao Mộc", "Sao Hỏa", 1, 9);

    // Level 10
    insert_questions_table("Thiên văn học: Hiện tượng 'Sao băng' là gì?",
                           "Vụ nổ của một ngôi sao lớn",
                           "Mảnh vụn của thiên thạch đi vào khí quyển Trái Đất",
                           "Một hành tinh cháy trong vũ trụ",
                           2, 10);
    insert_questions_table("Văn hóa dân gian: Tượng Phật bằng đồng lớn nhất Việt Nam nằm ở đâu?", "Yên Tử", "Cha Bái Đính", "Chùa Hương", 1, 10);
    insert_questions_table("Địa lý: Hồ nước nào lớn nhất thế giới tính theo diện tích mặt nước?", "Hồ Baikal", "Hồ Superior", "Hồ Victoria", 2, 10);
}

void create_rooms_table()
{
    open_database();
    const char *create_table_query =
        "CREATE TABLE IF NOT EXISTS rooms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "numbers INTEGER NOT NULL," // So nguoi choi trong phong
        "status INTEGER DEFAULT 0"  // Mac dinh la 0 : dang cho /  1 la dang choi / 2 la ket thuc
        ");";

    if (sqlite3_exec(db, create_table_query, 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Error: Can't create table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
}

int insert_rooms_table(int numbers)
{
    // numbers: số người chơi trong phòng
    open_database();
    const char *insert_query = "INSERT INTO rooms (numbers, status) VALUES (?, 0);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, insert_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, numbers);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "Error: Can't insert data: %s\n", sqlite3_errmsg(db));
        }
        else
        {
            printf("Phòng đã được thêm thành công.\n");
        }

        sqlite3_finalize(stmt);
        return 0;
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    close_database();
    // return state
    // 0 = success
    // 1 = fail
}

int search_rooms_by_id(int id)
{
    open_database();
    int check = 0;
    const char *select_query = "SELECT * FROM rooms WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            check = 1;
            int id = sqlite3_column_int(stmt, 0);     // Removed unused variable
            int status = sqlite3_column_int(stmt, 1); // Removed unused variable

            printf("Room found:\n");
            printf("ID: %d\n", id);
            printf("Status: %d\n", status);
        }

        sqlite3_finalize(stmt);
        if (check == 1)
            return 0;
        return 2;
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        close_database();
        return 1;
    }

    close_database();
    // return state
    // 0 = found
    // 1 = fail
    // 2 = not found
}

int get_id_new_room()
{
    open_database();
    const char *select_query = "SELECT id FROM rooms ORDER BY id DESC LIMIT 1;";
    sqlite3_stmt *stmt;
    int id = 0;
    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, 0) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            id = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return id;
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        close_database();
        return 0;
    }

    close_database();
}

int update_room_table(int id, int status, int numbers)
{
    open_database();
    const char *update_query = "UPDATE rooms SET status = ?, numbers = ? WHERE id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, update_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, status);
        sqlite3_bind_int(stmt, 2, numbers);
        sqlite3_bind_int(stmt, 3, id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "Error: Can't update data: %s\n", sqlite3_errmsg(db));
        }
        else
        {
            printf("Room updated successfully.\n");
        }

        sqlite3_finalize(stmt);
        return 0;
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    close_database();
    // return state
    // 0 = success
    // 1 = fail
}
char *get_all_rooms()
{
    open_database();

    const char *query = "SELECT id, numbers, status FROM rooms;"; // Truy vấn tất cả các phòng
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK)
    {
        printf("Lỗi truy vấn: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL; // Trả về NULL nếu có lỗi
    }

    // Dữ liệu tiêu đề ban đầu
    char *buffer = malloc(1024); // Cấp phát bộ nhớ động cho chuỗi kết quả
    if (!buffer)
    {
        printf("Lỗi cấp phát bộ nhớ\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }

    snprintf(buffer, 1024, "Danh sách các phòng:\n"); // Tiêu đề ban đầu

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        int numbers = sqlite3_column_int(stmt, 1);
        int status = sqlite3_column_int(stmt, 2);

        // Format thông tin phòng và nối vào buffer
        char room_info[256];
        snprintf(room_info, sizeof(room_info), "Phòng ID: %d, Số lượng người: %d, Trạng thái: %s\n",
                 id, numbers, status == -1 ? "Không tồn tại" : status == 0 ? "Đang chờ" : status == 1 ? "Đang chơi" : "Đã huỷ");

        // Kiểm tra nếu buffer đã đầy, cấp phát thêm bộ nhớ
        if (strlen(buffer) + strlen(room_info) >= 1024)
        {
            buffer = realloc(buffer, strlen(buffer) + strlen(room_info) + 1); // Tăng bộ nhớ cho buffer
        }

        strcat(buffer, room_info);
    }

    close_database();

    return buffer; // Trả về chuỗi kết quả
}

void create_ranking_round_table()
{
    open_database();
    const char *create_table_query =
        "CREATE TABLE IF NOT EXISTS ranking_round ("
        "roomId INTEGER NOT NULL,"
        "userId INTEGER NOT NULL,"
        "round INTEGER NOT NULL,"
        "money INTEGER NOT NULL,"
        "PRIMARY KEY (roomId, userId, round),"
        "FOREIGN KEY (roomId) REFERENCES rooms(id),"
        "FOREIGN KEY (userId) REFERENCES users(id)"
        ");";
    if (sqlite3_exec(db, create_table_query, 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Error: Can't create table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
}

int insert_ranking_round_table(int roomId, int userId, int round, int money)
{
    open_database();
    const char *insert_query = "INSERT INTO ranking_round (roomId, userId, round, money) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, insert_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, roomId);
        sqlite3_bind_int(stmt, 2, userId);
        sqlite3_bind_int(stmt, 3, round);
        sqlite3_bind_int(stmt, 4, money);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "Error: Can't insert data: %s\n", sqlite3_errmsg(db));
        }
        else
        {
            printf("Ranking round inserted successfully.\n");
        }

        sqlite3_finalize(stmt);
        return 0;
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    close_database();
    // return state
    // 0 = success
    // 1 = fail
}
// Hàm lấy bảng xếp hạng theo phòng và vòng chơi
Ranking *search_rankings_by_roomId_round(int roomId, int round)
{
    open_database();

    const char *query = "SELECT u.username, r.money FROM ranking_round r "
                        "JOIN users u ON r.userId = u.id "
                        "WHERE r.roomId = ? AND r.round = ? "
                        "ORDER BY r.money DESC;";
    sqlite3_stmt *stmt;
    Ranking *rankings = NULL;
    int result_count = 0;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, roomId);
        sqlite3_bind_int(stmt, 2, round);
        rankings = malloc(3 * sizeof(Ranking));

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            // lưu thông tin vào mảng kết quả
            strncpy(rankings[result_count].username, (const char *)sqlite3_column_text(stmt, 0), sizeof(rankings[result_count].username) - 1); // Copy the username to the results array
            rankings[result_count].money = sqlite3_column_int(stmt, 1);
            result_count++;
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db)); // Print an error message if the statement preparation fails
    }

    close_database();
    return rankings;
}

int check_room_status(int roomId)
{
    open_database();
    const char *select_query = "SELECT status FROM rooms WHERE id = ?;";
    sqlite3_stmt *stmt;
    int status = -1; // -1 có nghĩa là phòng không tồn tại

    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, roomId);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            status = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }

    close_database();
    return status; // Trả về trạng thái phòng
}

int update_room_status(int roomId, int newStatus, int newNumbers)
{
    open_database();
    const char *update_query = "UPDATE rooms SET status = ?, numbers = ? WHERE id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, update_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, newStatus);
        sqlite3_bind_int(stmt, 2, newNumbers);
        sqlite3_bind_int(stmt, 3, roomId);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "Error: Can't update data: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            close_database();
            return 1; // Thất bại
        }

        sqlite3_finalize(stmt);
        close_database();
        return 0; // Thành công
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        close_database();
        return 1; // Thất bại
    }
}

int update_player_in_room(int roomId, int userId, int round, int money)
{
    open_database();
    const char *insert_query = "INSERT INTO ranking_round (roomId, userId, round, money) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, insert_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, roomId);
        sqlite3_bind_int(stmt, 2, userId);
        sqlite3_bind_int(stmt, 3, round);
        sqlite3_bind_int(stmt, 4, money);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "Error: Can't insert data: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            close_database();
            return 1; // Thất bại
        }

        sqlite3_finalize(stmt);
        close_database();
        return 0; // Thành công
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        close_database();
        return 1; // Thất bại
    }
}

int get_current_numbers(int roomId){
    open_database();
    const char *select_query = "SELECT numbers FROM rooms WHERE id = ?;";
    sqlite3_stmt *stmt;
    int numbers = 0;

    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, 0) == SQLITE_OK){
        sqlite3_bind_int(stmt, 1, roomId);
        if (sqlite3_step(stmt) == SQLITE_ROW){
            numbers = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    else{
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }

    close_database();
    return numbers;
}

void display_ranking_round_by_roomId(int roomId)
{
    open_database(); // Mở cơ sở dữ liệu

    const char *query = "SELECT userId, round, money FROM ranking_round WHERE roomId = ?;"; // Truy vấn dữ liệu
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, roomId); // Gán giá trị roomId vào truy vấn

        printf("Dữ liệu bảng ranking_round cho roomId %d:\n", roomId);
        printf("UserId\tRound\tMoney\n");
        printf("-------------------------\n");

        while (sqlite3_step(stmt) == SQLITE_ROW) // Duyệt qua từng dòng kết quả
        {
            int userId = sqlite3_column_int(stmt, 0);
            int round = sqlite3_column_int(stmt, 1);
            int money = sqlite3_column_int(stmt, 2);

            printf("%d\t%d\t%d\n", userId, round, money); // Hiển thị thông tin
        }

        sqlite3_finalize(stmt); // Giải phóng câu lệnh
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }

    close_database(); // Đóng cơ sở dữ liệu
}
