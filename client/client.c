/*
 * client.c -- client program to send commands to the remote server.
 */

#include "include.h"

int shell_loop();
char* read_line();
char** parse_line(char* line);


int main(int argc, char** argv)
{
    shell_loop();
    return EXIT_SUCCESS;
}

int shell_loop()
{

    char* line;
    char** args;

    do{
        printf("> ");
        line = read_line();
        args = parse_line(line);
    }while(1);

    free(line);
    free(args);

    return -1;
}

char *read_line()
{

    char* line = NULL;
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

        //TODO - add way to handle if number of tokens is larger than buffer

        token = strtok(NULL, SHELL_TOK_DELIM);
    }

    return tokens;
}
