#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>


#define MAX_MESSAGE 1024


int socket_fd;


void shutdown_client() {
    printf("\nClient is exitting...\n");

    if (socket_fd > 0) {
        close(socket_fd);
        shutdown(socket_fd, SHUT_RDWR);
    }

    exit(0);
}


void wheel(int socket_fd) {
    char sendment[MAX_MESSAGE], receivement[MAX_MESSAGE];
    int turn;

    printf("Enter \'exit\' or Press ^C (or send the SIGTERM to %d) in order to exit\n", getpid());

    memset(receivement, '\0', sizeof(receivement));
    if (read(socket_fd, receivement, sizeof(receivement)) <= 0) {
        printf("Connection to the chat server lost.\n");
        return;
    }

    if (strcmp(receivement, "WAIT") == 0) {
        printf("Waiting for the second client to connect.\n");
        if (read(socket_fd, receivement, sizeof(receivement)) <= 0) {
            printf("Connection to the chat server lost.\n");
            return;
        }
        if (strcmp(receivement, "OK") == 0) {
            turn = 1;
            printf("Second client connected.\n");
        } else {
            printf("Bad request");
            return;
        }
    } else if (strcmp(receivement, "OK") == 0) {
        turn = 2;
    } else if (strcmp(receivement, "REJECTED") == 0) {
        printf("Connection rejected by the chat server.\n");
        return;
    } else {
        printf("Bad request");
        return;
    }

    printf("CHAT STARTS\n\n");
    while (true) {
        if (turn == 1) {
            size_t len_message = 0, read;
            char *message = NULL;

            printf("You: ");
            while ((read = getline(&message, &len_message, stdin)) <= 1)
                ;
            message[read - 1] = '\0';

            memset(sendment, '\0', sizeof(sendment));
            strcpy(sendment, message);
            if (strcmp(sendment, "exit") == 0) {
                printf("Thank you for chatting\n");
                strcpy(sendment, "Good-bye!!!");
                write(socket_fd, sendment, sizeof(sendment));
                break;
            }

            if (write(socket_fd, sendment, sizeof(sendment)) <= 0) {
                printf("Connection to the chat server lost.\n");
                break;
            }
            turn = 2;
        } else {
            memset(receivement, '\0', sizeof(receivement));
            if (read(socket_fd, receivement, sizeof(receivement)) <= 0) {
                printf("Connection to the chat server lost.\n");
                break;
            }
            printf("Other person: %s\n", receivement);
            turn = 1;
        }

    }
}


void handler(int signum) {
    shutdown_client();
}


void attach_handlers() {
    signal(SIGPIPE, SIG_IGN);

    struct sigaction action_int;
    memset(&action_int, 0, sizeof(action_int));
    action_int.sa_handler = handler;
    action_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action_int, NULL);

    struct sigaction action_term;
    memset(&action_term, 0, sizeof(action_term));
    action_term.sa_handler = handler;
    action_term.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &action_term, NULL);
}


int main(int argc, char** argv) {
    attach_handlers();

    int server_port;
    printf("Input the port of the server: ");
    scanf("%d", &server_port);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Error in socket()");
        shutdown_client();
    } else {
        printf("Socket created successfully\n");
    }

    struct sockaddr_in chat_address = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = inet_addr("127.0.0.1"),
            .sin_port = htons(server_port)
    };

    printf("Connecting to the server...\n");
    if (connect(socket_fd,
                (const struct sockaddr*) (&chat_address), sizeof(chat_address)) == -1) {
        perror("Error in connect()");
        shutdown_client();
    }
    else {
        printf("Successfully connected.\n\n");
    }

    wheel(socket_fd);

    shutdown_client();
    return 0;
}
