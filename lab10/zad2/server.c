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
#include <sys/epoll.h>

#include "common.h"

struct Client
{
    char name[64];          // client name
    int fd;                 // file descriptor - local or network
    struct sockaddr addr;   // client socket address
    socklen_t addr_len;     // length of addr
};

struct Client *clients[MAX_CLIENTS];    // list of clients
int instruction_count = 0;              // number of current instruction
struct Message message;                 // message - filled in input thread

// semaphores
sem_t sem_memory;   // synchronies access to global variables
sem_t sem_input;    // to check if there is new instruction form terminal
sem_t sem_ping;     // blocks sending and receiving signals when ping thread works

// Network
int fd;                             // file descriptor got from epoll_create -
int fd_local;
struct sockaddr_un addr_local;
char path_local[64];
int fd_network;
struct sockaddr_in addr_network;

// threads
pthread_t thread_input;
pthread_t thread_network;
pthread_t thread_ping;

// clean up after exit - cancel threads, destroy semaphres, shutdown file descriptores, unlink local path
void clean_up()
{
    printf("\nCleaning up\n");

    pthread_cancel(thread_input);
    pthread_cancel(thread_network);
    pthread_cancel(thread_ping);

    if(sem_destroy(&sem_memory) == -1)
        perror("ERROR: Problem destroying semaphore: sem_memory");
    if(sem_destroy(&sem_input) == -1)
        perror("ERROR: Problem destroying semaphore: sem_input");
    if(sem_destroy(&sem_ping) == -1)
        perror("ERROR: Problem destroying semaphore: sem_ping");

    close(fd);
    close(fd_local);
    close(fd_network);
    unlink(path_local);
}

// INT signal handler
void handle_INT()
{
    exit(EXIT_SUCCESS);
}

//initialize atexit, semaphores, clients list
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

    // NULL clients list
    int i = 0;
    while(i < MAX_CLIENTS)
    {
        clients[i] = NULL;
        i++;
    }

    // Init semaphores
    if(sem_init(&sem_memory, 0, 1) == -1)
        perror("ERROR: Problem initizlizng semaphore: sem_memory");
    if(sem_init(&sem_input, 0, 0) == -1)
        perror("ERROR: Problem initizlizng semaphore: sem_input");
    if(sem_init(&sem_ping, 0, 1) == -1)
        perror("ERROR: Problem initizlizng semaphore: sem_ping");
}

// add new client to list
int add_client(char *name, int fd, struct sockaddr addr, socklen_t addr_len)
{
    int destination = -1;
    int i = 0;
    while(i < MAX_CLIENTS)
    {
        if(clients[i] == NULL)
            destination = i;
        else if(strcmp(name, clients[i]->name) == 0)
            return NAME_TAKEN;
        i++;
    }
    if(destination == -1)
        return LIST_FULL;

    clients[destination] = malloc(sizeof(struct Client));
    strcpy(clients[destination]->name, name);
    clients[destination]->fd = fd;
    clients[destination]->addr = addr;
    clients[destination]->addr_len = addr_len;
    return OK;
}

// remove client from list
int remove_client(char *name)
{
    int i = 0;
    while(i < MAX_CLIENTS)
    {
        if(clients[i] != NULL && strcmp(name, clients[i]->name) == 0)
        {
            free(clients[i]);
            clients[i] = NULL;
            return 0;
        }
        i++;
    }
    return 0;
}

// find clietn with given name
struct Client *find_client(char *name)
{
    int i = 0;
    while(i < MAX_CLIENTS)
    {
        if(clients[i] != NULL && strcmp(name, clients[i]->name) == 0)
            return clients[i];
        i++;
    }
    return NULL;
}

// finds first client in list
struct Client *find_first_client()
{
    int i = 0;
    while(i < MAX_CLIENTS)
    {
        if(clients[i] != NULL)
            return clients[i];
        i++;
    }
    return NULL;
}

// find random client
// works same as find_first_client !!!
struct Client *find_random_client()
{
    return find_first_client();
}

// thread handling terminal input
void *task_input(void *args)
{
    printf("Input thread working\n");

    while(1)
    {
        int arg1, arg2, action;
        char buf[MAX_BUF_SIZE];
        scanf("%s %d %d", buf, &arg1, &arg2);
        if(strcmp(buf, "ADD") == 0)
            action = ADD;
        else if(strcmp(buf, "SUB") == 0)
            action = SUB;
        else if(strcmp(buf, "MUL") == 0)
            action = MUL;
        else if(strcmp(buf, "DIV") == 0)
            action = DIV;
        else
        {
            printf("Instruction has to be one of: ADD, SUB, MUL, DIV\n");
            continue;
        }
        sem_wait(&sem_memory);
        instruction_count++;
        message.action = action;
        message.arg1 = arg1;
        message.arg2 = arg2;
        message.number = instruction_count;
        sem_post(&sem_memory);

        sem_post(&sem_input);
    }
}

// thread handling accepting clients and sending messages
void *task_network(void *args)
{
    printf("Network thread working\n");

    while(1)
    {
        // to save in clients list
        char client_name[64];
        struct sockaddr client_addr;
        int addr_len = sizeof(struct sockaddr);

        // saves info about event
        struct epoll_event events[1];

        sem_wait(&sem_ping);
        int received_events = epoll_wait(fd, events, 1, 0);
        if(received_events > 0)
        {
            // receive name from client
            int bytes_recv = recvfrom(events[0].data.fd, &client_name, 64, 0, &client_addr, &addr_len);
            if(bytes_recv == -1)
            {
                perror("ERROR: Problem with receiving name");
                continue;
            }

            // message sent to client - either NAME_TAKEN, LIST_FULL or OK
            int int_message;
            sem_wait(&sem_memory);
            int_message = add_client(client_name, events[0].data.fd, client_addr, addr_len);
            sem_post(&sem_memory);
            if(int_message == NAME_TAKEN)
                printf("Refused registration of: %s - name taken\n", client_name);
            else if(int_message == LIST_FULL)
                printf("Refused registration of: %s - list is full\n", client_name);
            else
                printf("New client registerd: %s\n", client_name);
            if(sendto(events[0].data.fd, &int_message, sizeof(int), 0, &client_addr, addr_len) == -1)
                perror("ERROR: Problem sending registration result");
        }
        else if(received_events < 0)
            perror("ERROR: Problem with epoll_wait");
        sem_post(&sem_ping);

        // check if message was typed in terminal
        if(sem_trywait(&sem_input) == 0)
        {
            struct Message my_message;
            sem_wait(&sem_memory);
            my_message = message;
            struct Client *client = find_random_client();
            sem_post(&sem_memory);

            sem_wait(&sem_ping);
            if(client == NULL || sendto(client->fd, &my_message, sizeof(my_message), MSG_NOSIGNAL, &client->addr, client->addr_len) == -1)
                printf("Couldn't send message. Client not responding or no clients are registered\n");
            else
            {
                printf("Sent message nr %d\n", my_message.number);

                struct Reply reply;
                if(recvfrom(client->fd, &reply, sizeof(reply), 0, NULL, NULL) == -1)
                    perror("ERROR: Problem getting reply");
                else
                    printf("Got reply nr %d from %s: %d\n", reply.number, reply.name, reply.result);

            }
            sem_post(&sem_ping);
        }
    }
    return NULL;
}

// thread checking if clients are still responding
void *task_ping(void *args)
{
    printf("Ping thread working\n");

    while(1)
    {
        struct Message my_message;
        struct Reply reply;

        sem_wait(&sem_ping);
        sem_wait(&sem_memory);
        int i = 0;
        while(i < MAX_CLIENTS)
        {
            int to_remove = 0;
            if(clients[i] != NULL)
            {
                my_message.action = TRY;
                if(sendto(clients[i]->fd, &my_message, sizeof(my_message), MSG_NOSIGNAL, &clients[i]->addr, clients[i]->addr_len) == -1)
                    to_remove = 1;
                usleep(10);
                if(recvfrom(clients[i]->fd, &reply, sizeof(reply), MSG_DONTWAIT, &clients[i]->addr, &clients[i]->addr_len) == -1)
                    to_remove = 1;
                if(to_remove == 1)
                {
                    printf("Removing client: %s\n", clients[i]->name);
                    remove_client(clients[i]->name);
                }
            }
            i++;
        }
        sem_post(&sem_memory);
        sem_post(&sem_ping);

        sleep(1);
    }

}


// MAIN
int main(int argc, char *argv[])
{
    // check if there are enough arguments
    if(argc < 3)
    {
        printf("Usage:\nserver [port_nr] [socket_path]\n");
        exit(EXIT_SUCCESS);
    }

    // initialize signals, semaphres, idp.
    init();

    strcpy(path_local, argv[2]);

    // create local socket
    fd_local = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd_local == -1)
    {
        perror("ERROR: Problem creating local socket");
        return EXIT_FAILURE;
    }

    // create network socket
    fd_network = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd_network == -1)
    {
        perror("ERROR: Problem creating network socket");
        return EXIT_FAILURE;
    }

    // bind local socket
    addr_local.sun_family = AF_UNIX;
    strcpy(addr_local.sun_path, path_local);
    if(bind(fd_local, (struct sockaddr *) &addr_local, sizeof(addr_local)) == -1)
    {
        perror("ERROR: Problem binding local socket");
        return EXIT_FAILURE;
    }

    // bind network socket
    struct in_addr address;
    if(inet_aton(argv[1], &address) == 0)
    {
        perror("ERROR: Wrong address");
        exit(EXIT_FAILURE);
    }
    addr_network.sin_family = AF_INET;
    addr_network.sin_port = htons(PORT);
    addr_network.sin_addr = address;
    if(bind(fd_network, (struct sockaddr *) &addr_network, sizeof(addr_network)) == -1)
    {
        perror("ERROR: Problem binding network socket");
        return EXIT_FAILURE;
    }

    // epoll - fd_local and fd_network
    fd = epoll_create1(0);
    struct epoll_event event;

    event.events = EPOLLIN;

    event.data.fd = fd_local;
    if(epoll_ctl(fd, EPOLL_CTL_ADD, fd_local, &event) == -1)
    {
        perror("ERROR: Problem with epoll");
        return EXIT_FAILURE;
    }

    event.data.fd = fd_network;
    if(epoll_ctl(fd, EPOLL_CTL_ADD, fd_network, &event) == -1)
    {
        perror("ERROR: Problem with epoll");
        return EXIT_FAILURE;
    }

    // Start threads
    if(pthread_create(&thread_input, NULL, task_input, NULL) != 0 ||
        pthread_create(&thread_network, NULL, task_network, NULL) != 0 ||
        pthread_create(&thread_ping, NULL, task_ping, NULL) != 0
        )
    {
        perror("ERROR: Problem creating threads");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
    }

    return EXIT_SUCCESS;
}
