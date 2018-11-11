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

char* read_line()
{

    char* line = NULL;
    size_t buffer = 0;
    getline(&line, &buffer, stdin)
    return line;
}

char** parse_line(char* line)
{

}
