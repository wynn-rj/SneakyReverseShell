/*
 * server.c -- a stealthy server that executes commands sent to it
 */
#include "include.h"

void *respond_client(void *param);

int main(int argc, char** argv)
{
    int port, sockfd, new_sockfd;
    struct sockaddr_in host_addr, client_addr;
    socklen_t sin_size;
    int yes = 1;

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (isdigit(argv[1])) {
        fprintf(stderr, "Expected port, got: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Fatal error in socket\n");
        PRINT_ERRNO;
        return EXIT_FAILURE;
    }

    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
        fprintf(stderr, "Failed to set socket option SO_REUSEADDR\n");
        PRINT_ERRNO;
        return EXIT_FAILURE;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(port);
    // Automatically use my IP
    host_addr.sin_addr.s_addr = 0;
    memset(&(host_addr.sin_zero), '\0', 8);

    if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "Failed to bind to socket\n");
        PRINT_ERRNO;
        return EXIT_FAILURE;
    }

    if (listen(sockfd, BACKLOG_SIZE) == -1) {
        fprintf(stderr, "Failed to listen on socket\n");
        PRINT_ERRNO;
        return EXIT_FAILURE;
    }

    while(1) {
        pthread_t tid;
        pthread_attr_t attr;

        sin_size = sizeof(struct sockaddr_in);
        new_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &sin_size);
        if (new_sockfd == -1) {
            fprintf(stderr, "Faied to accept connection\n");
            PRINT_ERRNO;
            return EXIT_FAILURE;
        }
        DEBUG("Accepted connection from %s", inet_ntoa(client_addr.sin_addr));
        pthread_attr_init(&attr);
        pthread_create(&tid, &attr, respond_client, &new_sockfd);
    }

    return EXIT_SUCCESS;
}

void *respond_client(void *param)
{
    int recv_length, sockfd = *(int *)param;
    char buffer[BUFFER_SIZE];
    char message[23] = "Connection established\n";
    send(sockfd, message, 23, 0);
    recv_length = recv(sockfd, &buffer, BUFFER_SIZE, 0);
    printf("Recieved message of length %i. Message content: %s",
            recv_length, buffer);
    close(sockfd);
}
