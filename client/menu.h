#pragma once

#include <stdio.h>
#include <string.h>

typedef struct Account
{
    char username[20];
    char password[20];
} Account;

int display_welcome_menu()
{
    printf("---------------WELCOME TO AUCTION SYSTEM---------------\n");
    printf("1. Login\n");
    printf("2. Register\n");
    printf("Please choose 1-2: ...\n");
    int n;
    scanf("%d", &n);
    while (n != 1 && n != 2)
    {
        printf("Please choose 1 or 2: ");
        scanf("%d", &n);
    }
    return n;
}

Account display_login_menu()
{
    Account user_acc;
    printf("Please provide some information to login: \n");
    printf("Username: ");
    scanf("%s", user_acc.username);
    printf("Password: ");
    scanf("%s", user_acc.password);

    return user_acc;
}

Account display_register_menu()
{
    Account user_acc;
    printf("Please provide some information to register: \n");
    printf("Username: ");
    scanf("%s", user_acc.username);
    printf("Password: ");
    scanf("%s", user_acc.password);
    return user_acc;
}
