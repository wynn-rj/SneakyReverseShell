#ifndef SNEAKY_INCLUDE

#define SNEAKY_INCLUDE

#define BUFFER_SIZE 1024
#define BACKLOG_SIZE 5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#ifdef DEBUG
#define PRINT_ERRNO printf("Errno msg: %s", strerror(errno))
#define DEBUG(...) (printf(__VA_ARGS__))
#else
#define PRINT_ERRNO
#define DEBUG(...)
#endif

#endif
