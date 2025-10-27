#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include "protocol.h"

#define PORT 1255
#define MAXLINE 1024

void send_and_receive(int sockfd, struct sockaddr_in *servaddr, const char *message, char *response) {
    socklen_t len = sizeof(*servaddr);
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)servaddr, len);
    int n = recvfrom(sockfd, response, MAXLINE, 0, NULL, NULL);
    response[n] = '\0';
}

void handle_authentication(int sockfd, struct sockaddr_in *servaddr) {
    char buffer[MAXLINE], response[MAXLINE];
    char username[50], password[50];
    int choice;

    while (1) {
        printf("\nWelcome!\n1. Login\n2. Register\n3. Exit\n> ");
        scanf("%d", &choice);

        if (choice == 1) {
            printf("Username: ");
            scanf("%s", username);
            printf("Password: ");
            scanf("%s", password);
            sprintf(buffer, "LOGIN username=%s password=%s", username, password);
            send_and_receive(sockfd, servaddr, buffer, response);
            if (strstr(response, "SUCCESS")) {
                printf("Login successful!\n");
                break;
            } else {
                printf("Login failed.\n");
            }
        } else if (choice == 2) {
            printf("Choose a username: ");
            scanf("%s", username);
            printf("Choose a password: ");
            scanf("%s", password);
            sprintf(buffer, "CREATE_ACCOUNT username=%s password=%s role=user", username, password);
            send_and_receive(sockfd, servaddr, buffer, response);
            if (strstr(response, "SUCCESS")) {
                printf("Account created!\n");
            } else {
                printf("Failed to create account.\n");
            }
        } else if (choice == 3) {
            printf("Goodbye!\n");
            exit(0);
        } else {
            printf("Invalid choice. Try again.\n");
        }
    }
}

void show_user_menu(int sockfd, struct sockaddr_in *servaddr) {
    int choice;
    char buffer[MAXLINE], response[MAXLINE];

    while (1) {
        printf("\n--- Welcome to our cinema ---\n");
        printf("1. Display all movies\n");
        printf("2. Search movie by name\n");
        printf("3. Filter movies\n");
        printf("4. Purchase tickets\n");
        printf("5. Exit\n> ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                strcpy(buffer, "LIST_MOVIES");
                send_and_receive(sockfd, servaddr, buffer, response);
                printf("%s\n", response);
                break;

            case 2: {
                char title[100];
                printf("Enter movie name: ");
                scanf("%s", title);
                sprintf(buffer, "SEARCH title=%s", title);
                send_and_receive(sockfd, servaddr, buffer, response);
                printf("%s\n", response);
                break;
            }

            case 3:
                printf("Filter options:\n1. By genre\n2. By time\n> ");
                int filter_choice;
                scanf("%d", &filter_choice);
                while (getchar() != '\n'); // flush newline

                if (filter_choice == 1) {
                    strcpy(buffer, "LIST_GENRES");
                    send_and_receive(sockfd, servaddr, buffer, response);
                    printf("Available genres:\n%s\n", response);

                    char genre[50];
                    printf("Enter genre: ");
                    fgets(genre, sizeof(genre), stdin);
                    genre[strcspn(genre, "\n")] = '\0';
                    printf("Filtering genre: %s\n", genre);
                    sprintf(buffer, "FILTER_GENRE genre=\"%s\"", genre);
                    send_and_receive(sockfd, servaddr, buffer, response);
                    printf("%s\n", response);
                }
                else if (filter_choice == 2) {
                    char begin[10], end[10], day[10];
                    printf("Enter day (e.g., Thứ 2): ");
                    fgets(day, sizeof(day), stdin);
                    day[strcspn(day, "\n")] = '\0';
                    printf("Enter begin time (e.g., 12h): ");
                    scanf("%s", begin);
                    printf("Enter end time (e.g., 22h): ");
                    scanf("%s", end);
                    sprintf(buffer, "FILTER_TIME day=\"%s\" begin=%s end=%s", day, begin, end);
                    send_and_receive(sockfd, servaddr, buffer, response);
                    printf("%s\n", response);
                } else {
                    printf("Invalid filter option.\n");
                }
                break;

            case 4: {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);  // flush leftover newline

            char title[100], time[10], day[10];
            printf("Enter movie title: ");
            fgets(title, sizeof(title), stdin);
            title[strcspn(title, "\n")] = '\0';

            printf("Enter day (e.g., Thứ 2): ");
            fgets(day, sizeof(day), stdin);
            day[strcspn(day, "\n")] = '\0';

            printf("Enter start time (e.g., 12h): ");
            scanf("%s", time);
            while ((ch = getchar()) != '\n' && ch != EOF);  // flush after scanf

            // Request seatmap once
            sprintf(buffer, "GET_SEATMAP title=\"%s\" day=\"%s\" start=%s", title, day, time);
            send_and_receive(sockfd, servaddr, buffer, response);
            printf("Seatmap:\n%s\n", response);
            printf("|x| = booked, | | = available\n");

            int choice;
            do {
                int row, col;
                printf("Choose seat:\nRow (1-3): ");
                scanf("%d", &row);
                printf("Column (1-5): ");
                scanf("%d", &col);
                while ((ch = getchar()) != '\n' && ch != EOF);

                sprintf(buffer, "BOOK_SEAT title=\"%s\" day=\"%s\" time=%s row=%d col=%d", title, day, time, row, col);
                send_and_receive(sockfd, servaddr, buffer, response);
                printf("%s\n", response);

                // Show updated seatmap
                sprintf(buffer, "GET_SEATMAP title=\"%s\" day=\"%s\" start=%s", title, day, time);
                send_and_receive(sockfd, servaddr, buffer, response);
                printf("Updated Seatmap:\n%s\n", response);

                printf("Do you want to book another seat?\n");
                printf("1. Yes\n");
                printf("2. No\n");
                printf("> ");
                scanf("%d", &choice);
                while ((ch = getchar()) != '\n' && ch != EOF);

            } while (choice == 1);

            break;
        }


            case 5:
                printf("Goodbye!\n");
                return;

            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    handle_authentication(sockfd, &servaddr);
    show_user_menu(sockfd, &servaddr);

    close(sockfd);
    return 0;
}
