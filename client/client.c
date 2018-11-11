/*
 * client.c -- client program to send commands to the remote server.
 */

#include "include.h"

int shell_loop(void *param);
char* read_line();
char** parse_line(char* line);
void send_to_server(char** cmds);

int main(int argc, char** argv)
{

    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    int sockfd = 0;

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (isdigit(argv[1])) {
        fprintf(stderr, "Expected port, got: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Error in socket creation\n");
        return EXIT_FAILURE;
    }

    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)){
        fprintf(stderr, "Error: Connection Failed\n");
        return EXIT_FAILURE;
    }

    shell_loop(sockfd);
    return EXIT_SUCCESS;
}

int shell_loop(void *param)
{

    char *line;
    char **args;

    do{
        printf("> ");
        line = read_line();
        args = parse_line(line);
        send_to_server(args);
    }while(1);

    free(line);
    free(args);

    return -1;
}

char *read_line()
{

    char *line = NULL;
    size_t buffer = 0;
    getline(&line, &buffer, stdin);
    return line;
}

char **parse_line(char *line)
{

    int buffsize = SHELL_TOK_BUFFSIZE;
    int idx = 0;
    char **tokens = malloc(buffsize * sizeof(char*));
    char *token;

    if(tokens == NULL){
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SHELL_TOK_DELIM);
    while(token != NULL){
        tokens[idx] = token;
        idx++;

        if(idx >= buffsize){
            buffsize += SHELL_TOK_BUFFSIZE;
            tokens = realloc(tokens, buffsize * sizeof(char*));

            if(tokens == NULL){
                fprintf(stderr, "shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SHELL_TOK_DELIM);
    }

    return tokens;
}

void send_to_server(char **cmds)
{

}
