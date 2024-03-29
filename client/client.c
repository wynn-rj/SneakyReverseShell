/*
 * client.c -- client program to send commands to the remote server.
 */

#include "include.h"

int shell_loop(void *param);
void read_line(char* buffer, size_t buf_size);
void send_to_server(void *param, char *line);

int main(int argc, char** argv)
{
    int port;
    struct sockaddr_in server_addr;
    struct in_addr addr;
    int sockfd = 0;

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "USAGE: %s <port> [ip-address]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc != 3 || !inet_aton(argv[2], &addr)) {
        addr.s_addr = 0;
        if (argc == 3) {
            fprintf(stderr, "Bad address, using default\n");
        } else {
#ifdef DEBUG
            printf("No address specified, defaulting to local host\n");
#endif
        }
    }

    int length = strlen(argv[1]);
    for (int i = 0; i < length; i++) {
        if (!isdigit(argv[1][i])) {
            fprintf(stderr, "Expected port, got: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    port = atoi(argv[1]);

    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Error in socket creation\n");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = 0;
    memset(&(server_addr.sin_zero), '\0', sizeof(server_addr.sin_zero));


    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        fprintf(stderr, "Error: Connection Failed\n");
        return EXIT_FAILURE;
    }

    shell_loop((void *)&sockfd);

    close(sockfd);
    return EXIT_SUCCESS;
}

int shell_loop(void *param)
{

    char line[1024];

    do{
        printf("> ");
        read_line(line, 1024);
        //args = parse_line(line);
        send_to_server(param,line);
    }while(strncmp(line, "shutdown", 8) != 0 && strncmp(line, "disconnect", 10) != 0);

    return -1;
}

void read_line(char *buffer, size_t buf_size)
{
    fgets(buffer, buf_size, stdin);
    buffer[strlen(buffer) - 1] = 0;
}

void send_to_server(void *param, char *line)
{
    int sockfd = *(int *)param, recv_len;
    char buffer[1024];

    send(sockfd, line, strlen(line), 0);

    do{
        recv_len = recv(sockfd, &buffer, 1024, 0);
        if(recv_len > 0){
            printf("%s", buffer);
        }
    } while(recv_len == 1024);

}
