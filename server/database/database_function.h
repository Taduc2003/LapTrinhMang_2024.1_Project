#pragma once
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#define DATABASE_FILE "database.db"

sqlite3 *db;

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

static int callback(void *data, int argc, char **argv, char **azColName)
{
    int i;
    fprintf(stderr, "%s: ", (const char *)data);

    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}

void create_account_table()
{
    open_database();
    const char *create_table_query =
        "CREATE TABLE IF NOT EXISTS accounts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "password TEXT NOT NULL"
        ");";

    if (sqlite3_exec(db, create_table_query, 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Error: Can't create table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
}

int search_account_table(const char *username)
{
    open_database();

    const char *select_query = "SELECT * FROM accounts WHERE username = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            if (SQLITE_ROW == 0)
                return 2;
            int id = sqlite3_column_int(stmt, 0);
            const char *retrievedUsername = (const char *)sqlite3_column_text(stmt, 1);
            const char *retrievedPassword = (const char *)sqlite3_column_text(stmt, 2);

            printf("Account found:\n");
            printf("ID: %d\n", id);
            printf("Username: %s\n", retrievedUsername);
            printf("Password: %s\n", retrievedPassword);
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
    // 2 = not found
}

int insert_account_table(const char *username, const char *password)
{

    open_database();
    int check_username_exist = search_account_table(username);

    if (check_username_exist == 2)
    {
        fprintf(stderr, "Username has existed");
        return 1;
    }
    const char *insert_query = "INSERT INTO accounts (username, password) VALUES (?, ?);";
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
            printf("Account inserted successfully.\n");
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
    // 1 = fail, account exists
    // 2 = fail
}
