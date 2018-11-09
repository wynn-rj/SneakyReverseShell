/*
 * server.c -- a stealthy server that executes commands sent to it
 */
#include "include.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("USAGE: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
