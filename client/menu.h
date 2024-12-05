#pragma once

#include <stdio.h>
#include <string.h>

typedef struct users
{
    char username[20];
    char password[20];
} users;

int display_welcome_menu()
{
    printf("---------------WELCOME TO AUCTION SYSTEM---------------\n");
    printf("1. Login\n");
    printf("2. Register\n");
    printf("3. Exit\n");
    printf("Please choose 1-3: ...\n");
    int n;
    scanf("%d", &n);
    while (n <1 || n > 3)
    {
        printf("Please choose 1 - 3: ");
        scanf("%d", &n);
    }
    return n;
}

users display_login_menu()
{
    users user_acc;
    printf("Please provide some information to login: \n");
    printf("Username: ");
    scanf("%s", user_acc.username);
    printf("Password: ");
    scanf("%s", user_acc.password);

    return user_acc;
}

users display_register_menu()
{
    users user_acc;
    printf("Please provide some information to register: \n");
    printf("Username: ");
    scanf("%s", user_acc.username);
    printf("Password: ");
    scanf("%s", user_acc.password);
    return user_acc;
}

int display_main_menu() {
    while (1)
    {
        printf("---------------WELCOME TO DUNG DE TIEN ROI---------------\n");
        printf("1. Tao phong\n");
        printf("2. Tim phong\n");
        printf("3. Xem tat ca cac phong\n");
        printf("4. Exit\n");
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
