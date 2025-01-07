#ifndef SERVER_FUNCTION_GAME_H
#define SERVER_FUNCTION_GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>
#include "./database/database_function.h"
#include "server_function.h"

#define MAXLINE 4096
#define INITIAL_POINTS 1000000 // Số tiền ban đầuđầu
#define PENALTY_POINTS 500000  // Số tiền phạt khi mắc lỗi
#define TIMEOUT_SECONDS 60
#define MAX_ROUND_GAME 4 // Số vòng tối đã trong 1 game

int count[MAXLINE] = {0};

int handle_answer_of_client(int connfd, int current_money, int correct_answer, int level);
char *handle_winner(Player *player[3]);
char *display_ranking_in_round(int room_id, int round);

void handle_join_game_request(char *data, int connfd)
{
    char user_id[10];
    char room_id[10];
    sscanf(data, "%s %s", user_id, room_id);

    insert_ranking_round_table(atoi(room_id), atoi(user_id), 0, INITIAL_POINTS);

    count[room_id] = get_ready_user_number(atoi(room_id), 0);
    while (count[room_id] < 3)
    {
        count[room_id] = get_ready_user_number(atoi(room_id), 0);
        if (count[room_id] == 3)
        {
            send_message("GAME_START", "1", connfd);
        }
        else
        {
            send_message("WAITING_FOR_PLAYERS", "0", connfd);
        }
    }
}

typedef struct
{
    int user_id;
    int connfd;
    int room_id;
} Player;

typedef struct
{
    int answer;
    int bet_amount;
} BetAnswer;

char *handle_winner(Player *player[3])
{
    int winner_count = 0;
    int max_money = 0;
    char *result = malloc(MAXLINE * sizeof(char));
    snprintf(result, MAXLINE, "WIN\n");

    int current_money[3];
    for (int i = 0; i < 3; i++)
    {
        current_money[i] = get_current_money(player[i]->room_id, player[i]->user_id, MAX_ROUND_GAME);
    }

    // Find max money
    for (int i = 0; i < 3; i++)
    {
        if (current_money[i] >= max_money)
        {
            max_money = current_money[i];
        }
    }

    // Count winners and build result string
    for (int i = 0; i < 3; i++)
    {
        if (current_money[i] == max_money)
        {
            winner_count++;
            char temp[100];
            snprintf(temp, 100, "Người chơi %s ", get_username_by_id(player[i]->user_id));
            strcat(result, temp);
        }
    }

    if (winner_count > 1)
    {
        strcat(result, "hòa nhau");
    }
    else
    {
        strcat(result, "là người chiến thắng");
    }

    return result;
}

char *display_ranking_in_round(int room_id, int round)
{
    char *result = malloc(MAXLINE * sizeof(char));
    snprintf(result, MAXLINE, " \n----------------------------\nRanking in round\nRoom ID: %d, Round: %d\n%-15s %-10s\n", room_id, round, "Username", "Money");

    Ranking *rankings = search_rankings_by_roomId_round(room_id, round);
    if (rankings != NULL)
    {
        for (int i = 0; i < 3; i++)
        {
            if (strlen(rankings[i].username) > 0)
            {
                char line[MAXLINE];
                snprintf(line, MAXLINE, "%-15s %-10d\n", rankings[i].username, rankings[i].money);
                strncat(result, line, MAXLINE - strlen(result) - 1);
            }
        }
        free(rankings);
    }
    strncat(result, "--------\t------------\n", MAXLINE - strlen(result) - 1);

    return result;
}

int handle_answer_of_client(int connfd, int current_money)
{
    char buffer[MAXLINE];
    BetAnswer bets[2];
    int question_id;
    // Receive answer from client
    int valread = read(connfd, buffer, MAXLINE);
    if (valread < 0)
    {
        perror("recv error");
        return -1;
    }
    buffer[valread] = '\0';

    // Parse answers and bets "answer1:bet1;answer2:bet2"
    sscanf(buffer, "%d;%d:%d;%d:%d", &question_id,
           &bets[0].answer, &bets[0].bet_amount,
           &bets[1].answer, &bets[1].bet_amount);

    int correct_answer = search_question_by_id(question_id).correctAns;

    // Check each answer
    if (bets[0].answer == correct_answer && bets[1].answer == 0) // Neu cau thu nhat tra loi dung va cau 2 khong tra loiloi
    {
        current_money -= PENALTY_POINTS;
        snprintf(buffer, MAXLINE, "%d là câu trả lời ĐÚNG nhưng chưa trả lời xong.\nPHẠT %d\n", bets[0].answer, PENALTY_POINTS);
    }
    else if (bets[0].answer == correct_answer) // Neu cau thu nhat ddng
    {
        // Correct answer - keep the bet
        current_money = bets[0].bet_amount;
        snprintf(buffer, MAXLINE, "%d là câu trả lời đúng \n", bets[0].answer);
    }
    else if (bets[0].answer != correct_answer && bets[1].answer == 0) // Neu cau thu nhat sai va cau thu hai khong tra loi
    {
        current_money = current_money - PENALTY_POINTS;
        snprintf(buffer, MAXLINE, "%d là câu trả lời SAI và chưa trả lời xong.\nPHẠT %d\n", bets[0].answer, PENALTY_POINTS);
    }
    else if (bets[0].answer == 0) // Neu cau thu nhat khong tra loi
    {
        current_money -= PENALTY_POINTS;
        snprintf(buffer, MAXLINE, "Bạn chưa trả lời hoặc trả lời chưa đúng định dạng.\nPHẠT %d\n", PENALTY_POINTS);
    }
    else if (bets[1].answer == correct_answer) // Neu cau thu hai dung
    {
        current_money = bets[1].bet_amount;
        snprintf(buffer, MAXLINE, "%d là câu trả lời Đúng \n", bets[1].answer);
    }
    else
    {
        current_money = 0;
        snprintf(buffer, MAXLINE, "Trả lời sai toàn bộ\n");
    }

    if (current_money <= 0)
    {
        current_money = 0;
    }

    send_message("ANSWER_RESULT", buffer, connfd);
    return current_money;
}

void handle_answer_request(char *data, int sockfd)
{
}

#endif // SERVER_FUNCTION_GAME_H
