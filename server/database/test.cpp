#include <iostream>
#include <sqlite3.h>
#include <string.h>
#include "database_function.h"

int main(int argc, char **argv)
{
    create_account_table();
    insert_account_table("admin", "admin");
    search_account_table("admin");
    return (0);
}
