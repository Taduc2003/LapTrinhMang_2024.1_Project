#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "database_function.h"


int main(int argc, char **argv)
{
    create_users_table();
    insert_users_table("acc0", "12345678");
    insert_users_table("acc1", "12345678");
    insert_users_table("acc2", "12345678");
    insert_users_table("acc2", "12345678");
    insert_users_table("acc3", "12345678");
    search_users_table("acc1");
    search_users_table("acc5");

    create_rooms_table();
    insert_rooms_table(1);
    insert_rooms_table(1);

    create_questions_table();
    insert_sample_questions();
    // insert_questions_table("content1", "ans1", "ans2", "ans3", 1, 1);
    // insert_questions_table("content2", "ans1", "ans2", "ans3", 1, 1);
    // insert_questions_table("content3", "ans1", "ans2", "ans3", 1, 1);

    create_ranking_round_table();
    insert_ranking_round_table(1,1,1,100);
    insert_ranking_round_table(1,2,1,100);
    insert_ranking_round_table(1,3,1,100);
    
    printf("%d", check_login("acc0", "12345678"));
    return (0);
}

