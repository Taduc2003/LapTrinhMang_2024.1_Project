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

#define MAX_BUFFER 1000000
#define MAX_QUESTIONS 3
#define INITIAL_POINTS 1000000
#define PENALTY_POINTS 200000
#define TIMEOUT_SECONDS 60

typedef struct {
    int connfd;
    int points;
   // int room_id;
} Player;

void display_ranking_in_round(int room_id, int round) {
    printf("Ranking in round\n");
    printf("Room ID: %d, Round: %d\n", room_id, round);
    printf("Username\tMoney\n");
    Ranking* rankings = search_rankings_by_roomId_round(room_id, round);
    if (rankings != NULL) {
        for (int i = 0; i < 3; i++) {
            if (strlen(rankings[i].username) > 0) {
                printf("%s\t%d\n", rankings[i].username, rankings[i].money);
            }
        }
        free(rankings);
    }
    printf("--------\t-----\n");
}

void handle_answer_of_client(Player *player, Question selected_question) {
    char buffer[MAX_BUFFER];
    fd_set readfds;
    struct timeval timeout;

    // Set up the timeout
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;

    // Set up the file descriptor set
    FD_ZERO(&readfds);
    FD_SET(player->connfd, &readfds);

    // Wait for the client to respond within the timeout period
    int activity = select(player->connfd + 1, &readfds, NULL, NULL, &timeout);

    if (activity == 0) {
        // Timeout occurred, deduct points
        player->points -= PENALTY_POINTS;
        if (player->points <= 0) {
            strcpy(buffer, "You have been kicked out due to insufficient points.\n");
            send(player->connfd, buffer, strlen(buffer), 0);
            close(player->connfd);
            player->connfd = -1; // Mark the player as kicked out
            return;
        } else {
            snprintf(buffer, MAX_BUFFER, "Timeout! You lost %d points. Your current points: %d\n", PENALTY_POINTS, player->points);
            send(player->connfd, buffer, strlen(buffer), 0);
            return;
        }
    } else if (activity < 0) {
        perror("select error");
        return;
    }

    // Receive answer from client
    int valread = read(player->connfd, buffer, MAX_BUFFER);
    if (valread < 0) {
        perror("recv error");
        return;
    }
    buffer[valread] = '\0';
    printf("Answer from client %d: %s\n", player->connfd,buffer);

    // Check if the answer is correct
    int answer = atoi(buffer);

    if (answer == selected_question.correctAns) {
        strcpy(buffer, "Correct\n");
    } else {
        strcpy(buffer, "Incorrect\n");
    }
    send(player->connfd, buffer, strlen(buffer), 0);
}

void handle_game(int connfd1, int connfd2, int connfd3) {
    char buffer[MAX_BUFFER];
    srand(time(NULL));

    Player players[3] = {
        {connfd1, INITIAL_POINTS},
        {connfd2, INITIAL_POINTS},
        {connfd3, INITIAL_POINTS}
    };

    // Thông báo bắt đầu game
    strcpy(buffer, "Game is starting!\n");
    for (int i = 0; i < 3; i++) {
        if (send(players[i].connfd, buffer, strlen(buffer), 0) == -1) {
            perror("Error sending start message");
            return;
        }
    }

    for (int level = 1; level <= 10; level++) {
        printf("Starting level %d\n", level);
        
        // Get questions for current level
        Question *questions = malloc(MAX_QUESTIONS * sizeof(Question));
        questions = search_questions_by_level(level);
        if (questions == NULL) {
            printf("Error: Could not load questions for level %d\n", level);
            continue;
        }

        // Count number of questions
        int num_questions = 0;
        while (num_questions < MAX_QUESTIONS && questions[num_questions].id != 0) {
            num_questions++;
        }

        if (num_questions == 0) {
            printf("No questions found for level %d\n", level);
            continue;
        }

        // Select random question
        int random_index = rand() % num_questions;
        Question selected_question = questions[random_index];

        // Thêm thông báo chuẩn bị trước khi gửi câu hỏi
        strcpy(buffer, "READY\n");
        for (int i = 0; i < 3; i++) {
            if (players[i].connfd != -1) {
                send(players[i].connfd, buffer, strlen(buffer), 0);
                // Đợi một chút để client chuẩn bị
                usleep(500000); // 500ms delay
            }
        }

        // Gửi câu hỏi
        snprintf(buffer, MAX_BUFFER, "QUESTION\nLevel %d: %s\n1. %s\n2. %s\n3. %s\nENDQ\n",
                 level, selected_question.content, selected_question.ans1, 
                 selected_question.ans2, selected_question.ans3);

        for (int i = 0; i < 3; i++) {
            if (players[i].connfd != -1) {
                send(players[i].connfd, buffer, strlen(buffer), 0);
            }
        }

        // Handle answers from active players
        for (int i = 0; i < 3; i++) {
            if (players[i].connfd != -1) {
                handle_answer_of_client(&players[i], selected_question);
            }
        }

        // Free questions after use
        if (questions != NULL) {
            free(questions);
            questions = NULL;
        }
    }

    // Send game over message
    strcpy(buffer, "Game Over!\n");
    for (int i = 0; i < 3; i++) {
        if (players[i].connfd != -1) {
            send(players[i].connfd, buffer, strlen(buffer), 0);
        }
    }
}

#endif
