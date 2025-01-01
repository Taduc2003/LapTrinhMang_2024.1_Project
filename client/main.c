#include <gtk/gtk.h>
#include "gtk_ui.h"

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <IP address of the server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(3000);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(3);
    }

    display_welcome_menu(sockfd); // Hiển thị giao diện GTK

    gtk_main(); // Chạy vòng lặp GTK

    close(sockfd);
    return 0;
}
