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

typedef struct PlayerInGame
{
    int sockfd;
    int user_id;
    int current_money;
    int room_id;
} PlayerInGame;

// void handle_send_answer(PlayerInGame *playerInGame);

// void clear_stdin()
// {
//     int c;
//     while ((c = getchar()) != '\n' && c != EOF)
//         ;
// }

// void join_game(int sockfd, char *user_id, char *room_id)
// {
//     char buffer[MAXLINE];
//     int n;
//     fd_set readfds;
//     struct timeval timeout;
//     PlayerInGame *playerInGame = malloc(sizeof(PlayerInGame));
//     playerInGame->sockfd = sockfd;
//     playerInGame->user_id = atoi(user_id);
//     playerInGame->current_money = INITIAL_MONEY;
//     playerInGame->room_id = atoi(room_id);

//     // Gửi yêu cầu tham gia game
//     snprintf(buffer, MAXLINE, "HEADER: JOIN_GAME; DATA: %d %d", playerInGame->user_id, playerInGame->room_id);
//     if (send(sockfd, buffer, strlen(buffer), 0) == -1)
//     {
//         perror("send failed");
//         exit(1);
//     }
//     printf("Yêu cầu tham gia game đã được gửi.\n");

//     // Vòng lặp đợi phản hồi từ server
//     while (1)
//     {
//         FD_ZERO(&readfds);
//         FD_SET(sockfd, &readfds);

//         // Đặt timeout cho select (ví dụ: 30 giây)
//         timeout.tv_sec = 30;
//         timeout.tv_usec = 0;

//         int activity = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

//         if (activity < 0)
//         {
//             perror("select error");
//             close(sockfd);
//             exit(1);
//         }
//         else if (activity == 0)
//         {
//             // Timeout xảy ra
//             printf("Đang chờ phản hồi từ server...\n");
//             continue; // Tiếp tục vòng lặp để đợi phản hồi
//         }

//         if (FD_ISSET(sockfd, &readfds))
//         {

//             // Nhận phản hồi từ server
//             n = recv(sockfd, buffer, MAXLINE - 1, 0);
//             if (n <= 0)
//             {
//                 if (n == 0)
//                 {
//                     printf("Server đã ngắt kết nối.\n");
//                 }
//                 else
//                 {
//                     perror("recv failed");
//                 }
//                 close(sockfd);
//                 exit(1);
//             }

//             buffer[n] = '\0';
//             printf("Phản hồi từ server: %s\n", buffer);

//             if (strcmp(buffer, "WAITING_FOR_PLAYERS") == 0)
//             {
//                 printf("Đang chờ người chơi khác tham gia...\n");
//                 // Tiếp tục đợi phản hồi trong vòng lặp
//             }
//             else if (strcmp(buffer, "GAME_START") == 0)
//             {
//                 printf("Game đã bắt đầu!\n");
//                 // Bắt đầu vòng chơi
//                 break; // Thoát vòng lặp đợi và tiếp tục bắt đầu game
//             }
//             else
//             {
//                 printf("Phản hồi không xác định từ server.\n");
//                 // Xử lý các phản hồi khác nếu cần
//             }
//         }
//     }

//     // Bắt đầu vòng chơi sau khi nhận được "GAME_START"
//     while (1)
//     {
//         memset(buffer, 0, MAXLINE);
//         n = recv(sockfd, buffer, MAXLINE - 1, 0);
//         if (n <= 0)
//         {
//             printf("Mất kết nối với server.\n");
//             break;
//         }
//         buffer[n] = '\0';

//         // Kiểm tra nếu là câu hỏi
//         if (strstr(buffer, "QUESTION") != NULL)
//         {
//             printf("\n----------------------------------------\n");
//             printf("|||             CÂU HỎI MỚI           |||\n");
//             printf("----------------------------------------\n");
//             printf("%s\n", buffer);
//             printf("----------------------------------------\n");
//             printf("Số tiền hiện tại: %d\n", playerInGame->current_money);
//             printf("----------------------------------------\n");

//             handle_send_answer(playerInGame);
//         }
//         else if (strstr(buffer, "GAME_OVER") != NULL)
//         {
//             printf("%s\n", buffer);
//             break;
//         }
//         else if (strstr(buffer, "END_GAME") != NULL)
//         {
//             printf("%s\n", buffer);
//             break;
//         }
//         else if (strstr(buffer, "OVER_MONEY") != NULL)
//         {
//             printf("%s\n", buffer);
//         }
//         else if (strstr(buffer, "WIN") != NULL)
//         {
//             printf("%s\n", buffer);
//             break;
//         }
//         else if (strstr(buffer, "KQ") != NULL)
//         {
//             char *token = strtok(buffer, "|"); // Split by delimiter
//             if (token != NULL)
//             {
//                 // Skip "KQ" prefix and print result
//                 char *result = token + 3;
//                 printf("Kết quả: %s", result);

//                 // Get money status part
//                 token = strtok(NULL, "|");
//                 if (token != NULL && strstr(token, "CURRENT_MONEY") != NULL)
//                 {
//                     // Skip "CURRENT_MONEY" prefix and convert to integer
//                     char *money_str = token + 13;
//                     playerInGame->current_money = atoi(money_str);
//                     printf("Số tiền hiện tại: %d\n", playerInGame->current_money);
//                 }
//             }
//         }
//         else
//         {
//             printf("Server: %s", buffer);
//         }
//     }

//     free(playerInGame);
//     return; // Return to main menu
// }

// typedef struct
// {
//     int answer;
//     int bet_amount;
// } BetAnswer;

// void handle_send_answer(PlayerInGame *playerInGame)
// {
//     BetAnswer *bets = malloc(2 * sizeof(BetAnswer));

//     // Khởi tạo bet ở mỗi vòng là 0
//     for (int i = 0; i < 2; i++)
//     {
//         bets[i].answer = 0;
//         bets[i].bet_amount = 0;
//     }

//     printf("Nhập câu trả lời 1 (1, 2, hoặc 3): ");
//     scanf("%d", &bets[0].answer);
//     clear_stdin();

//     while (1)
//     {
//         printf(">>> Đặt cược cho câu trả lời 1:");
//         scanf("%d", &bets[0].bet_amount);
//         clear_stdin();
//         if (bets[0].bet_amount < 0 || bets[0].bet_amount > playerInGame->current_money)
//         {
//             printf("Số tiền cược không hợp lệ. Vui lòng nhập lại.\n");
//         }
//         else
//         {
//             break;
//         }
//     }

//     while (1)
//     {
//         printf("\nNhập câu trả lời 2 (1, 2, hoặc 3): ");
//         scanf("%d", &bets[1].answer);
//         clear_stdin();

//         if (bets[1].answer == bets[0].answer)
//         {
//             printf("Câu trả lời 2 không được trùng với câu trả lời 1. Vui lòng nhập lại.\n");
//         }
//         else
//         {
//             break;
//         }
//     }

//     bets[1].bet_amount = playerInGame->current_money - bets[0].bet_amount;
//     printf(">>> Đặt cược cho câu trả lời 2: %d", bets[1].bet_amount);

//     for (int i = 0; i < 2; i++)
//     {
//         if (bets[i].answer < 1 || bets[i].answer > 3)
//         {
//             bets[i].answer = 0;
//             bets[i].bet_amount = 0;
//         }
//     }

//     // Gửi cả 2 câu trả lời và tiền cược theo format "answer1:bet1;answer2:bet2"
//     char send_buffer[50];
//     snprintf(send_buffer, sizeof(send_buffer), "%d:%d;%d:%d",
//              bets[0].answer, bets[0].bet_amount,
//              bets[1].answer, bets[1].bet_amount);
//     send(playerInGame->sockfd, send_buffer, sizeof(send_buffer), 0);
//     printf("\nĐã gửi câu trả lời: %s\n", send_buffer);

//     free(bets);
// }

#endif // CLIENT_FUNCTION_GAME_H