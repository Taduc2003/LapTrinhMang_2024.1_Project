#pragma once
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

#define DATABASE_FILE "./server/database/database.db"

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

int search_account_table(char *username)
{
    open_database();
    int check = 0;
    const char *select_query = "SELECT * FROM accounts WHERE username = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            check = 1;
            // int id = sqlite3_column_int(stmt, 0); // Removed unused variable
            // const char *retrievedUsername = (const char *)sqlite3_column_text(stmt, 1); // Removed unused variable
            // const char *retrievedPassword = (const char *)sqlite3_column_text(stmt, 2); // Removed unused variable

            // printf("Account found:\n");
            // printf("ID: %d\n", id);
            // printf("Username: %s\n", retrievedUsername);
            // printf("Password: %s\n", retrievedPassword);
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

int insert_account_table(char *username, char *password)
{

    open_database();
    int check_username_exist = search_account_table(username);

    if (check_username_exist == 0)
    {
        fprintf(stderr, "Username has existed Cannot Add\n");
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

int check_login(char *username, char *password)
{
    open_database();

    int check = -1;
    const char *select_query = "SELECT * FROM accounts WHERE username = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            check = 1;
            // int id = sqlite3_column_int(stmt, 0); // Removed unused variable
            // const char *retrievedUsername = (const char *)sqlite3_column_text(stmt, 1); // Removed unused variable
            const char *retrievedPassword = (const char *)sqlite3_column_text(stmt, 2);

            // printf("Account found:\n");
            // printf("ID: %d\n", id);
            // printf("Username: %s\n", retrievedUsername);
            // printf("Password: %s\n", retrievedPassword);
            check = strcmp(password, retrievedPassword);
        }

        sqlite3_finalize(stmt);

        if (check == 0)
            return 0;
        if (check == 1)
            return 1;
        return 2;
    }
    else
    {
        fprintf(stderr, "Error: Can't prepare statement: %s\n", sqlite3_errmsg(db));
        return 3;
    }

    close_database();

    // return state
    // 0 = found & correct pwd
    // 1 = found & wrong pwd
    // 2 = not found
    // 3 = err
}
