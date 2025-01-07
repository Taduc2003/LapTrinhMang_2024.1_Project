#ifndef CLIENT_FUNCTION_GAME_H
#define CLIENT_FUNCTION_GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <unistd.h> // Add this for STDIN_FILENO

#define MAXLINE 4096
#define INITIAL_MONEY 1000000

void handle_join_game(int sockfd, char *user_id, char *room_id);
void handle_in_game(int sockfd, char *user_id, char *room_id);
void handle_send_answer(int sockfd, int current_money, int question_id);
void handle_question(int sockfd, char *data);

void handle_join_game(int sockfd, char *user_id, char *room_id)
{
    char data[MAXLINE];
    snprintf(data, MAXLINE, "%s %s", user_id, room_id);
    send_message("JOIN_GAME", data, sockfd);
    int n;
    char buffer[MAXLINE];

    // Vòng lặp đợi phản hồi từ server
    while (1)
    {
        // Nhận phản hồi từ server
        if (recv(sockfd, buffer, MAXLINE, 0) == 0) // Kiểm tra phản hồi từ server
        {
            printf("Máy chủ đã kết thúc sớm, không có phản hồi.\n");
            exit(4);
        }

        process_message(buffer, strlen(buffer), sockfd); // Kết thúc chuỗi
        char *data_start = strstr(buffer, "DATA: ");
        if (strncmp(data_start, "1", 1) == 0)
        {
            break;
        }else{
            printf("Đang chờ người chơi khác tham gia...\n");
        }
    }

    // Bắt đầu vòng chơi sau khi nhận được "GAME_START"
    handle_in_game(sockfd, user_id, room_id);
    return;
}

void handle_in_game(int sockfd, char *user_id, char *room_id)
{
    int n;
    char buffer[MAXLINE];
    while (1)
    {
        memset(buffer, 0, MAXLINE);
        n = recv(sockfd, buffer, MAXLINE - 1, 0);
        if (n <= 0)
        {
            printf("Mất kết nối với server.\n");
            break;
        }
        //buffer[n] = '\0';
        process_message(buffer, strlen(buffer), sockfd);
        if (strncmp(data_start, "1", 1) == 0)
        {
            break;
        }
    }
}

void handle_question(int sockfd, char *data)
{
    char content[1024];
    int current_money;
    int question_id;
    sscanf(data, "%d %d %s", &question_id, &current_money, content);
    printf("\n----------------------------------------\n");
    printf("|||             CÂU HỎI MỚI           |||\n");
    printf("----------------------------------------\n");
    printf("%s\n", content);
    printf("----------------------------------------\n");
    printf("Số tiền hiện tại: %d\n", current_money);
    printf("----------------------------------------\n");

    handle_send_answer(sockfd,current_money,question_id);
}

typedef struct
{
    int answer;
    int bet_amount;
} BetAnswer;

void handle_send_answer(int sockfd, int current_money, int question_id)
{
    BetAnswer *bets = malloc(2 * sizeof(BetAnswer));

    // Khởi tạo bet ở mỗi vòng là 0
    for (int i = 0; i < 2; i++)
    {
        bets[i].answer = 0;
        bets[i].bet_amount = 0;
    }

    printf("Nhập câu trả lời 1 (1, 2, hoặc 3): ");
    scanf("%d", &bets[0].answer);
    clear_stdin();

    while (1)
    {
        printf(">>> Đặt cược cho câu trả lời 1:");
        scanf("%d", &bets[0].bet_amount);
        clear_stdin();
        if (bets[0].bet_amount < 0 || bets[0].bet_amount > current_money)
        {
            printf("Số tiền cược không hợp lệ. Vui lòng nhập lại.\n");
        }
        else
        {
            break;
        }
    }

    while (1)
    {
        printf("\nNhập câu trả lời 2 (1, 2, hoặc 3): ");
        scanf("%d", &bets[1].answer);
        clear_stdin();

        if (bets[1].answer == bets[0].answer)
        {
            printf("Câu trả lời 2 không được trùng với câu trả lời 1. Vui lòng nhập lại.\n");
        }
        else
        {
            break;
        }
    }

    bets[1].bet_amount = current_money - bets[0].bet_amount;
    printf(">>> Đặt cược cho câu trả lời 2: %d", bets[1].bet_amount);

    for (int i = 0; i < 2; i++)
    {
        if (bets[i].answer < 1 || bets[i].answer > 3)
        {
            bets[i].answer = 0;
            bets[i].bet_amount = 0;
        }
    }

    // Gửi cả 2 câu trả lời và tiền cược theo format "answer1:bet1;answer2:bet2"
    char send_buffer[50];
    snprintf(send_buffer, sizeof(send_buffer), "%d;%d:%d;%d:%d", question_id,
             bets[0].answer, bets[0].bet_amount,
             bets[1].answer, bets[1].bet_amount);

    send_message("ANSWER", send_buffer, sockfd);
    printf("\nĐã gửi câu trả lời: %s\n", send_buffer);

    free(bets);
}

#endif // CLIENT_FUNCTION_GAME_H