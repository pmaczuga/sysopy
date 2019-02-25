#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "common.h"

int fd;                 // socket file descriptor
struct sockaddr *addr;  // socket address

// cleaning up after exit
void clean_up()
{
    printf("\nCleaning up\n");

    free(addr);

    shutdown(fd, SHUT_RDWR);
    close(fd);
}

// INT signal handler
void handle_INT()
{
    exit(EXIT_SUCCESS);
}

void init()
{
    // Clean up at exit
    atexit(clean_up);

    // Handle INT signal
    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = handle_INT;
    sigaction(SIGINT, &act, NULL);
}

int main(int argc, char *argv[])
{
    // check if there are enough arguments
    if(argc < 4)
    {
        printf("Usage\nclient [name] [local/network] [address]\n");
        exit(EXIT_SUCCESS);
    }

    // init atexit, signal handler
    init();

    // create socket (local or network)
    if(strcmp(argv[2], "local") == 0)
    {
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un *addr_un = malloc(sizeof(struct sockaddr_un));
        strcpy(addr_un->sun_path, argv[3]);
        addr_un->sun_family = AF_UNIX;
        addr = (struct sockaddr *) addr_un;
    }
    else if(strcmp(argv[2], "network") == 0)
    {
        fd = socket(AF_INET, SOCK_STREAM, 0);

        struct in_addr address;
        if(inet_aton(argv[3], &address) == 0)
        {
            perror("ERROR: Wrong address");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in *addr_in = malloc(sizeof(struct sockaddr_in));
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(PORT);
        addr_in->sin_addr = address;
        addr = (struct sockaddr *) addr_in;
    }
    else
    {
        fprintf(stderr, "ERROR: Second argument has to be either network or local\n");
        exit(EXIT_FAILURE);
    }

    if(fd == -1)
    {
        perror("ERROR: Problem creating socket");
        exit(EXIT_FAILURE);
    }

    // connect to server
    if(connect(fd, addr, sizeof(* addr)) == -1)
    {
        perror("ERROR: Problem connectiong");
        exit(EXIT_FAILURE);
    }

    // send name to server
    char name[64];
    strcpy(name, argv[1]);
    if(send(fd, name, 64, 0) == -1)
    {
        perror("ERROR: Problem sending name");
        exit(EXIT_FAILURE);
    }

    // Receive registartion resukt - either NAME_TAKEN, LIST_FULL or OK
    int int_buf;
    if(recv(fd, &int_buf, sizeof(int_buf), 0) == -1)
    {
        perror("ERROR: Problem receiving reply");
        exit(EXIT_FAILURE);
    }
    if(int_buf == NAME_TAKEN)
    {
        printf("Name is already taken\n");
        exit(EXIT_SUCCESS);
    }
    else if(int_buf == LIST_FULL)
    {
        printf("Too many clients registerd\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Registration successfull\n");
    }

    while(1)
    {
        struct Message message;
        int recv_result = recv(fd, &message, sizeof(message), 0);
        if(recv_result == -1)
        {
            perror("ERROR: problem receiving message");
            exit(EXIT_FAILURE);
        }
        else if(recv_result == 0)   // file descriptor shutdown
        {
            printf("Server shutdown file descriptor");
            exit(EXIT_SUCCESS);
        }
        else
        {
            struct Reply reply;
            reply.number = message.number;
            if(message.action == ADD)
            {
                reply.result = message.arg1 + message.arg2;
                printf("Received message nr %d: ADD %d %d\n", message.number, message.arg1, message.arg2);
            }
            else if(message.action == SUB)
            {
                reply.result = message.arg1 - message.arg2;
                printf("Received message nr %d: SUB %d %d\n", message.number, message.arg1, message.arg2);
            }
            else if(message.action == MUL)
            {
                reply.result = message.arg1 * message.arg2;
                printf("Received message nr %d: MUL %d %d\n", message.number, message.arg1, message.arg2);
            }
            else if(message.action == DIV)
            {
                reply.result = message.arg1 / message.arg2;
                printf("Received message nr %d: DIV %d %d\n", message.number, message.arg1, message.arg2);
            }
            else if(message.action == TRY)
            {
                reply.result = 0;
                reply.number = 0;
            }
            strcpy(reply.name, name);
            if(send(fd, &reply, sizeof(reply), 0) == -1)
                perror("ERROR: Problem sending reply\n");
        }
    }

    return EXIT_SUCCESS;
}
