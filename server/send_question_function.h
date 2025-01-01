#ifndef SEND_QUESTIONS_FUNCTION_H
#define SEND_QUESTIONS_FUNCTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>
#include "./database/database_function.h"

#define MAXLINE 4096
#define INITIAL_POINTS 1000000 // Số tiền ban đầuđầu
#define PENALTY_POINTS 500000  // Số tiền phạt khi mắc lỗi
#define TIMEOUT_SECONDS 60
#define MAX_ROUND_GAME 4 // Số vòng tối đã trong 1 game

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

int handle_answer_of_client(int connfd, int current_money, int correct_answer, int level)
{
    char buffer[MAXLINE];
    BetAnswer bets[2];

    // Receive answer from client
    int valread = read(connfd, buffer, MAXLINE);
    if (valread < 0)
    {
        perror("recv error");
        return -1;
    }
    buffer[valread] = '\0';

    // Parse answers and bets "answer1:bet1;answer2:bet2"
    sscanf(buffer, "%d:%d;%d:%d",
           &bets[0].answer, &bets[0].bet_amount,
           &bets[1].answer, &bets[1].bet_amount);

    char result_str[MAXLINE];
    snprintf(result_str, MAXLINE, "KQ\n");

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
    strcat(result_str, buffer);

    // Send current money status
    char money_status[MAXLINE];
    if (current_money <= 0)
    {
        current_money = 0;
    }
    snprintf(money_status, MAXLINE, "CURRENT_MONEY\n%d\n", current_money);

    // Use a delimiter
    char full_message[MAXLINE * 2];
    snprintf(full_message, MAXLINE * 2, "%s|%s", result_str, money_status);
    send(connfd, full_message, strlen(full_message), 0);
    return current_money;
}

void handle_game(Player *players[3])
{
    char buffer[MAXLINE];
    srand(time(NULL));
    int current_money[3];

    // Thông báo bắt đầu game
    strcpy(buffer, "START_GAME");
    for (int i = 0; i < 3; i++)
    {
        insert_ranking_round_table(players[i]->room_id, players[i]->user_id, 0, INITIAL_POINTS);
        if (send(players[i]->connfd, buffer, strlen(buffer), 0) == -1)
        {
            perror("Error sending start message");
            return;
        }
    }

    int active_players = 3;

    for (int level = 1; level <= MAX_ROUND_GAME + 1 && active_players > 0; level++)
    {
        if (level == MAX_ROUND_GAME + 1)
        {
            snprintf(buffer, MAXLINE, "WIN\n%s\n", handle_winner(players));
            for (int i = 0; i < 3; i++)
            {
                send(players[i]->connfd, buffer, strlen(buffer), 0);
            }
            break;
        }

        // lấy số tiền hiện tại của từng người chơi = só tiền còn lại ở vòng trướctrước
        for (int i = 0; i < 3; i++)
        {
            current_money[i] = get_current_money(players[i]->room_id, players[i]->user_id, level - 1);
        }

        printf("Bắt đầu vòng %d\n", level);
        // Get questions for current level
        Question *questions = malloc(MAX_QUESTIONS * sizeof(Question));
        questions = search_questions_by_level(level);
        if (questions == NULL)
        {
            printf("Error: Không thể tải câu hỏi cho vòng %d\n", level);
            continue;
        }

        // Count number of questions
        int num_questions = 0;
        while (num_questions < MAX_QUESTIONS && questions[num_questions].id != 0)
        {
            num_questions++;
        }

        if (num_questions == 0)
        {
            printf("Không tìm thấy câu hỏi nào cho vòng%d\n", level);
            continue;
        }

        // Select random question
        int random_index = rand() % num_questions;
        Question selected_question = questions[random_index];

        // Prepare question message
        snprintf(buffer, MAXLINE, "QUESTION\nLevel %d: %s\n1. %s\n2. %s\n3. %s\n",
                 level, selected_question.content, selected_question.ans1,
                 selected_question.ans2, selected_question.ans3);

        // Count active players before each round
        active_players = 0;
        for (int i = 0; i < 3; i++)
        {
            if (current_money[i] > 0)
            {
                active_players++;
            }
        }

        if (active_players < 1)
        {
            printf("Không đủ người chơi để tiếp tục. Trò chơi đã kết thúc.\n");
            // Send game over to remaining player
            strcpy(buffer, "GAME_OVER\nKhông đủ người chơi để tiếp tục. Trò chơi đã kết thúc.\n");
            for (int i = 0; i < 3; i++)
            {
                send(players[i]->connfd, buffer, strlen(buffer), 0);
            }
            break;
        }

        // nếu còn 1 người chơi thì in ra thông báo người chơi đó là người chiến thắng
        if (active_players == 1)
        {
            Player *winner;
            for (int i = 0; i < 3; i++)
            {
                if (current_money[i] > 0)
                {
                    winner = players[i];
                }
            }
            for (int i = 0; i < 3; i++)
            {
                snprintf(buffer, MAXLINE, "WIN\nNgười chơi %s là người chiến thắng\n", get_username_by_id(winner->user_id));
                send(players[i]->connfd, buffer, strlen(buffer), 0);
            }

            break;
        }

        // Send question to all active players
        for (int i = 0; i < 3; i++)
        {
            if (current_money[i] > 0)
            {
                if (send(players[i]->connfd, buffer, strlen(buffer), 0) == -1)
                {
                    printf("Player %d disconnected\n", players[i]->user_id);
                    active_players--;
                    continue;
                }
            }
        }

        int player_answered_num = 0;
        fd_set readfds;
        // struct timeval timeout;

        // Handle answers from active players
        while (player_answered_num < active_players)
        {
            FD_ZERO(&readfds);
            for (int i = 0; i < 3; i++)
            {
                if (current_money[i] > 0)
                {
                    FD_SET(players[i]->connfd, &readfds);
                }
            }

            // timeout.tv_sec = TIMEOUT_SECONDS;
            // timeout.tv_usec = 0;

            int activity = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
            if (activity < 0)
            {
                perror("select error");
                break;
            }

            for (int i = 0; i < 3; i++)
            {
                if (current_money[i] > 0 && FD_ISSET(players[i]->connfd, &readfds))
                {
                    int update_money = handle_answer_of_client(players[i]->connfd, current_money[i], selected_question.correctAns, level);
                    insert_ranking_round_table(players[i]->room_id, players[i]->user_id, level, update_money);
                    player_answered_num++;
                    if (update_money <= 0)
                    {
                        strcpy(buffer, "OVER_MONEY\nBạn đã hết tiền để tiếp tục game.\nHãy ngồi theo dõi và chờ kết quả cuối cùng.\n");
                        send(players[i]->connfd, buffer, strlen(buffer), 0);
                    }
                }
            }
        }

        // Send ranking in round
        for (int i = 0; i < 3; i++)
        {
            strcpy(buffer, display_ranking_in_round(players[i]->room_id, level));
            if (send(players[i]->connfd, buffer, strlen(buffer), 0) == -1)
            {
                perror("Error sending ranking");
                continue;
            }
        }

        // Free questions after use
        if (questions != NULL)
        {
            free(questions);
            questions = NULL;
        }
    }
}

#endif
