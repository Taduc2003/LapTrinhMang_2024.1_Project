#ifndef CLIENT_FUNCTION_GAME_H
#define CLIENT_FUNCTION_GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <unistd.h>  // Add this for STDIN_FILENO

#define MAXLINE 4096
#define BUFFER_SIZE 1000000

void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void join_game(int sockfd)
{
    char buffer[BUFFER_SIZE];
    int n;
    fd_set readfds;
    struct timeval timeout;

    // Gửi yêu cầu tham gia game
    strcpy(buffer, "HEADER: JOIN_GAME; DATA: ");
    if (send(sockfd, buffer, strlen(buffer), 0) == -1)
    {
        perror("send failed");
        exit(1);
    }
    printf("Yêu cầu tham gia game đã được gửi.\n");

    // Vòng lặp đợi phản hồi từ server
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        // Đặt timeout cho select (ví dụ: 30 giây)
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;

        int activity = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0)
        {
            perror("select error");
            pclose(sockfd);
            exit(1);
        }
        else if (activity == 0)
        {
            // Timeout xảy ra
            printf("Đang chờ phản hồi từ server...\n");
            continue; // Tiếp tục vòng lặp để đợi phản hồi
        }

        if (FD_ISSET(sockfd, &readfds))
        {
            // Nhận phản hồi từ server
            n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
            if (n <= 0)
            {
                if (n == 0)
                {
                    printf("Server đã ngắt kết nối.\n");
                }
                else
                {
                    perror("recv failed");
                }
                pclose(sockfd);
                exit(1);
            }

            buffer[n] = '\0';
            printf("Phản hồi từ server: %s\n", buffer);

            if (strcmp(buffer, "WAITING_FOR_PLAYERS") == 0)
            {
                printf("Đang chờ người chơi khác tham gia...\n");
                // Tiếp tục đợi phản hồi trong vòng lặp
            }
            else if (strcmp(buffer, "GAME_START") == 0)
            {
                printf("Game đã bắt đầu!\n");
                // Bắt đầu vòng chơi
                break; // Thoát vòng lặp đợi và tiếp tục bắt đầu game
            }
            else
            {
                printf("Phản hồi không xác định từ server.\n");
                // Xử lý các phản hồi khác nếu cần
            }
        }
    }

    // Bắt đầu vòng chơi sau khi nhận được "GAME_START"
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            if (n == 0) printf("Server đã kết thúc game hoặc ngắt kết nối.\n");
            else perror("recv failed");
            break;
        }
        buffer[n] = '\0';

        // Kiểm tra thông báo chuẩn bị
        if (strstr(buffer, "READY") != NULL) {
            printf("Chuẩn bị nhận câu hỏi...\n");
            continue;
        }

        // Kiểm tra nếu là câu hỏi
        if (strstr(buffer, "QUESTION") != NULL)
        {
            printf("\n----------------------------------------\n");
            printf("|||            CÂU HỎI MỚI           |||\n");
            printf("----------------------------------------\n");
            printf("%s\n", buffer);
            printf("----------------------------------------\n");
            
            char answer[10];
            memset(answer, 0, sizeof(answer));
            
            // Đọc input với timeout mà không cần clear stdin
            fd_set rfds;
            struct timeval tv;
            FD_ZERO(&rfds);
            FD_SET(STDIN_FILENO, &rfds);
            tv.tv_sec = 58;
            tv.tv_usec = 0;

            printf(">>> Nhập câu trả lời của bạn (1, 2, hoặc 3): ");
            fflush(stdout);

            int retval = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
            if (retval > 0) 
            {
                if (scanf("%1s", answer) == 1)
                {
                    // Xử lý ký tự newline còn lại trong buffer
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                    
                    if (answer[0] >= '1' && answer[0] <= '3')
                    {
                        send(sockfd, answer, 1, 0);
                        printf("Đã gửi câu trả lời: %s\n", answer);
                    }
                    else
                    {
                        printf("Câu trả lời không hợp lệ, gửi 0\n");
                        send(sockfd, "0", 1, 0);
                    }
                }
            }
            else if (retval == 0)
            {
                printf("\nHết thời gian trả lời!\n");
                send(sockfd, "0", 1, 0);
            }

            // Đợi kết quả từ server
            memset(buffer, 0, BUFFER_SIZE);
            n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
            if (n > 0) {
                buffer[n] = '\0';
                printf("Kết quả: %s\n", buffer);
            }
        }
        else if (strstr(buffer, "Game Over") != NULL)
        {
            printf("Game kết thúc!\n");
            break;
        }
        else
        {
            printf("Server: %s", buffer);
        }
    }
}

#endif // CLIENT_FUNCTION_GAME_H