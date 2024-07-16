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
#include <sys/mman.h>


#define MAX_MESSAGE 1024
#define MAX_NAME 32


int socket_fd, client_fd1, client_fd2;


void shutdown_server () {
    printf("Server is shutting down...\n");

    if (client_fd1 > 0) {
        shutdown(client_fd1, SHUT_RDWR);
        close(client_fd1);
    }

    if (client_fd2 > 0) {
        shutdown(client_fd2, SHUT_RDWR);
        close(client_fd2);
    }

    if (socket_fd > 0) {
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);
    }

    exit(0);
}


void wheel(int client_in, int client_out) {
    char message[1024];

    printf("Press ^C (or send a SIGTERM to %d) in order to exit\n", getpid());

    memset(message, '\0', sizeof(message));
    strcpy(message, "OK");
    if (write(client_in, message, sizeof(message)) <= 0) {
        perror("Error in client connection");
        shutdown_server();
    }

    printf("Client %d chatting.\n", client_in);
    while (true) {
        memset(message, '\0', sizeof(message));
        if (read(client_in, message, sizeof(message)) <= 0) {
            printf("Connection to the client lost.\n");
            break;
        }
        printf("Server received a message from client %d: %s\n", client_in, message);

        if (write(client_out, message, sizeof(message)) <= 0) {
            printf("Connection to the client lost.\n");
            return;
        }
        printf("Server sent the message to client %d: %s\n", client_out, message);
    }
}


void handler (int signum) {
    shutdown_server();
}


void attach_handlers () {
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


int main (int argc, char** argv) {
    attach_handlers();

    int server_port;
    if (argc != 2) {
        printf("Not enough arguments\n");
        exit(0);
    }
    server_port = atoi(argv[1]);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Error in socket()");
        shutdown_server();
    }else {
        printf("Socket for chat server created successfully\n");
    }

    int setsockopt_val = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &setsockopt_val, sizeof(setsockopt_val));

    struct sockaddr_in server_address = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = htons(server_port)
    };

    if (bind(socket_fd,
             (const struct sockaddr*) (&server_address), sizeof(server_address)) == -1) {
        perror("Error in bind");
        shutdown_server();
    } else {
        printf("Socket for chat server bound successfully in port %d\n", server_port);
    }

    while (true) {
        listen(socket_fd, 1);
        struct sockaddr_in client_address1;
        socklen_t client_address_length1;
        client_fd1 = accept(socket_fd, (struct sockaddr*) (&client_address1), &client_address_length1);
        if (client_fd1 == -1) {
            perror("Error in client accept");
            shutdown_server();
        } else {
            printf("Client %d accepted successfully to the chat.\n", client_fd1);
        }

        char message[10];
        strcpy(message, "WAIT");
        if (write(client_fd1, message, sizeof(message)) <= 0) {
            perror("Error in client connection");
            shutdown_server();
        }

        listen(socket_fd, 1);
        struct sockaddr_in client_address2;
        socklen_t client_address_length2;
        client_fd2 = accept(socket_fd, (struct sockaddr*) (&client_address2), &client_address_length2);
        if (client_fd2 == -1) {
            perror("Error in client accept");
            shutdown_server();
        } else {
            printf("Client %d accepted successfully to the chat.\n", client_fd2);
        }

        if (fork() == 0) {
            printf("Client %d connected to the chat\n", client_fd1);
            wheel(client_fd1, client_fd2);
            printf("Client %d disconnected\n", client_fd1);
        } else {
            if (fork() == 0) {
                printf("Client %d connected to the chat\n", client_fd2);
                wheel(client_fd2, client_fd1);
                printf("Client %d disconnected\n", client_fd2);
                return 0;
            } else {
                while (true) {
                    listen(socket_fd, 1);
                    struct sockaddr_in client_address;
                    socklen_t client_address_length;
                    int client_fd = accept(socket_fd, (struct sockaddr*) (&client_address), &client_address_length);
                    if (client_fd == -1) {
                        perror("Error in client accept");
                    } else {
                        char message[MAX_MESSAGE];
                        strcpy(message, "REJECTED");
                        if (write(client_fd, message, sizeof(message)) <= 0) {
                            printf("Connection to the client lost.\n");
                        } else {
                            printf("Client %d rejected.\n", client_fd);
                        }
                    }
                }
            }

        }
    }

    shutdown_server();
    return 0;
}
