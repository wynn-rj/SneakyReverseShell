/*
 * server.c -- a stealthy server that executes commands sent to it
 */
#include "include.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (isdigit(argv[1])) {
        fprintf(stderr, "Expected port, got: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
