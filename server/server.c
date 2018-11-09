/*
 * server.c -- a stealthy server that executes commands sent to it
 */
#include "include.h"

int main(int argc, char** argv)
{
    int port, sockfd, new_sockfd;
    struct sockaddr_in host_addr, client_addr;
    socklen_t sockin_size;
    int yes = 1;
    char buffer[BUFFER_SIZE];

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


    return EXIT_SUCCESS;
}
