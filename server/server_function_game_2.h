#ifndef SERVER_FUNCTION_GAME_2_H
#define SERVER_FUNCTION_GAME_2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>
#include "./database/database_function.h"
#include "server_function.h"
#include "server.h"
#include "send_question_function.h"

#define MAXLINE 4096
#define INITIAL_POINTS 1000000 // Số tiền ban đầuđầu
#define PENALTY_POINTS 500000  // Số tiền phạt khi mắc lỗi
#define TIMEOUT_SECONDS 60
#define MAX_ROUND_GAME 5 // Số vòng tối đã trong 1 game

int count[MAXLINE] = {0};

void handle_answer_of_client(char *data, int connfd);
char *handle_winner(int room_id);
char *display_ranking_in_round(int room_id, int round);
void handle_send_question(int room_id);
Question *get_random_questions(int level);
int check_client_in_room(char *username, int room_id, int round);

Question *get_random_questions(int level)
{
    srand(time(NULL));
    // Get questions for current level
    Question *questions = search_questions_by_level(level);
    if (questions == NULL)
    {
        printf("Error: Could not load questions for level %d\n", level);
        return NULL; // Return NULL if questions could not be loaded
    }

    // Count number of questions
    int num_questions = 0;
    while (num_questions < MAX_QUESTIONS && questions[num_questions].id != 0)
    {
        num_questions++;
    }

    if (num_questions == 0)
    {
        printf("No questions found for level %d\n", level);
        free(questions); // Free allocated memory
        return NULL;     // Return NULL if no questions found
    }

    // Select random question
    int random_index = rand() % num_questions;
    Question *selected_question = malloc(sizeof(Question));
    if (selected_question == NULL)
    {
        printf("Error: Could not allocate memory for selected question\n");
        free(questions); // Free allocated memory
        return NULL;     // Return NULL if memory allocation fails
    }
    *selected_question = questions[random_index];
    free(questions); // Free allocated memory
    return selected_question;
}

int check_client_in_room(char *username, int room_id, int round)
{
    if (username == NULL || strlen(username) == 0)
        return 0;
    int id = get_id_by_username(username);
    if (id == 0)
        return 0;

    if (strcmp(username, get_username_in_ranking_round(room_id, id, round)) == 0)
    {
        return 1;
    }
    return 0;
}

void handle_join_game_request(char *data, int sockfd)
{
    char user_id[10];
    char room_id[10];
    sscanf(data, "%s %s", user_id, room_id);

    int room_id_int = atoi(room_id);
    insert_ranking_round_table(room_id_int, atoi(user_id), 0, INITIAL_POINTS);

    count[room_id_int] = get_ready_user_number(room_id_int, 0);
    if (count[room_id_int] == 3)
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (check_client_in_room(clients_status[i].username, room_id_int, 0) == 1)
            {
                send_message("GAME_START", "1", clients_status[i].socket);
            }
        }
        handle_send_question(room_id_int);
    }
    else
    {
        send_message("WAITING_FOR_PLAYERS", "0", sockfd);
    }
}

// typedef struct
// {
//     int user_id;
//     int connfd;
//     int room_id;
// } Player;

// typedef struct
// {
//     int answer;
//     int bet_amount;
// } BetAnswer;

char *handle_winner(int room_id)
{
    int winner_count = 0;
    int max_money = 0;
    char *result = malloc(MAXLINE * sizeof(char));
    if (result == NULL)
    {
        printf("Error: Could not allocate memory for result\n");
        return NULL; // Return NULL if memory allocation fails
    }
    result[0] = '\0'; // Initialize result string

    int current_money[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (check_client_in_room(clients_status[i].username, room_id, MAX_ROUND_GAME) == 1)
        {
            current_money[i] = get_current_money(room_id, get_id_by_username(clients_status[i].username), MAX_ROUND_GAME);
        }
    }

    // Find max money
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (check_client_in_room(clients_status[i].username, room_id, MAX_ROUND_GAME) == 1)
        {
            if (current_money[i] >= max_money)
            {
                max_money = current_money[i];
            }
        }
    }

    // Count winners and build result string
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (check_client_in_room(clients_status[i].username, room_id, MAX_ROUND_GAME) == 1)
        {
            if (current_money[i] == max_money)
            {
                winner_count++;
                char temp[100];
                snprintf(temp, 100, "Người chơi %s ", clients_status[i].username);
                strcat(result, temp);
            }
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
    if (result == NULL)
    {
        printf("Error: Could not allocate memory for result\n");
        return NULL; // Return NULL if memory allocation fails
    }
    snprintf(result, MAXLINE, " \n----------------------------\nRanking in round\nRoom ID: %d, Round: %d\n%-15s %-10s\n", room_id, round, "Username", "Money");

    Ranking *rankings = search_rankings_by_roomId_round(room_id, round);
    if (rankings != NULL)
    {
        for (int i = 0; i < 3; i++)
        {
            if (strlen(rankings[i].username) > 0 || rankings[i].money > 0)
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

void handle_answer_of_client(char *data, int connfd)
{
    char buffer[MAXLINE];
    BetAnswer bets[2];
    int question_id;

    // Parse answers and bets "answer1:bet1;answer2:bet2"
    sscanf(data, "%d;%d:%d;%d:%d", &question_id,
           &bets[0].answer, &bets[0].bet_amount,
           &bets[1].answer, &bets[1].bet_amount);

    int correct_answer = search_question_by_id(question_id)->correctAns;
    int level = search_question_by_id(question_id)->level;
    int current_money = 0;
    int room_id = 0;
    int user_id = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients_status[i].socket == connfd)
        {
            room_id = clients_status[i].room_id;
            user_id = get_id_by_username(clients_status[i].username);
            current_money = get_current_money(room_id, user_id, level - 1);
        }
    }

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

    insert_ranking_round_table(room_id, user_id, level, current_money);
    send_message("ANSWER_RESULT", buffer, connfd);
}

void handle_send_question(int room_id)
{
    char buffer[MAXLINE];
    int current_money[MAX_CLIENTS];

    int active_players = 3;

    for (int level = 1; level <= MAX_ROUND_GAME + 1 && active_players > 0; level++)
    {
        if (level == MAX_ROUND_GAME + 1)
        {
            char *result = handle_winner(room_id);
            if (result != NULL)
            {
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (check_client_in_room(clients_status[i].username, room_id, 0) == 1)
                    {
                        send_message("WIN", result, clients_status[i].socket);
                    }
                }
                free(result);
            }
            break;
        }
        printf("Starting level %d\n", level);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (check_client_in_room(clients_status[i].username, room_id, 0) == 1)
            {
                current_money[i] = get_current_money(room_id, get_id_by_username(clients_status[i].username), level - 1);
            }
        }

        // Count active players before each round
        active_players = 0;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (check_client_in_room(clients_status[i].username, room_id, 0) == 1)
            {
                if (current_money[i] > 0)
                {
                    active_players++;
                }
            }
        }
        if (active_players < 1)
        {
            printf("Not enough players to continue. Game ended.\n");
            // Send game over to remaining player
            strcpy(buffer, "Not enough players to continue. Game ended.\n");
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (current_money[i] > 0)
                {
                    send_message("GAME_OVER", buffer, clients_status[i].socket);
                }
            }
            break;
        }

        // nếu còn 1 người chơi thì in ra thông báo người chơi đó là người chiến thắng
        if (active_players == 1)
        {
            int max_money = 0;
            ClientStatus winner;
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (check_client_in_room(clients_status[i].username, room_id, 0) == 1)
                {
                    if (current_money[i] > max_money)
                    {
                        max_money = current_money[i];
                        winner = clients_status[i];
                    }
                }
            }
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (check_client_in_room(clients_status[i].username, room_id, 0) == 1)
                {
                    snprintf(buffer, MAXLINE, "Người chơi %s là người chiến thắng\n", winner.username);
                    send_message("WIN", buffer, clients_status[i].socket);
                }
            }
            break;
        }

        Question *selected_question = get_random_questions(level);
        if (selected_question == NULL)
        {
            printf("Error: Could not get random question for level %d\n", level);
            break; // Exit if no question could be retrieved
        }
        // Prepare question message
        snprintf(buffer, MAXLINE, "%d|Level %d: %s\n1. %s\n2. %s\n3. %s\n", selected_question->id,
                 level, selected_question->content, selected_question->ans1,
                 selected_question->ans2, selected_question->ans3);
        free(selected_question); // Free the allocated memory
        // Send question to all active players
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (check_client_in_room(clients_status[i].username, room_id, 0) == 1)
            {
                if (current_money[i] > 0)
                {
                    char send_mes[MAXLINE];
                    snprintf(send_mes, MAXLINE, "%d|%s", current_money[i], buffer);
                    send_message("QUESTION", send_mes, clients_status[i].socket);
                }
            }
        }

        int player_answered_num = 0;
        fd_set readfds;
        struct timeval timeout;

        // Handle answers from active players
        while (player_answered_num < active_players)
        {
            FD_ZERO(&readfds);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (check_client_in_room(clients_status[i].username, room_id, 0) == 1)
                {
                    if (current_money[i] > 0)
                    {
                        FD_SET(clients_status[i].socket, &readfds);
                    }
                }
            }

            timeout.tv_sec = TIMEOUT_SECONDS;
            timeout.tv_usec = 0;

            int activity = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
            if (activity < 0)
            {
                perror("select error");
                break;
            }

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (check_client_in_room(clients_status[i].username, room_id, 0) == 1)
                {
                    if (current_money[i] > 0 && FD_ISSET(clients_status[i].socket, &readfds))
                    {
                        // Receive answer from client
                        int valread = read(clients_status[i].socket, buffer, MAXLINE);
                        if (valread < 0)
                        {
                            perror("recv error");
                            handle_client_disconnect(clients_status[i].socket);
                            active_players--;
                            return;
                        }
                        if (valread > 0)
                        {
                            buffer[valread] = '\0';
                            process_message(buffer, strlen(buffer), clients_status[i].socket);
                            player_answered_num++;
                        }
                    }
                }
            }
        }

        // Send ranking in round
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (check_client_in_room(clients_status[i].username, room_id, 0) == 1) // gui cho cac client trong phong
            {
                char *ranking_result = display_ranking_in_round(room_id, level);
                if (ranking_result != NULL)
                {
                    send_message("RANKING", ranking_result, clients_status[i].socket);
                    free(ranking_result);
                }
            }
        }
    }
}

#endif // SERVER_FUNCTION_GAME_2_H
