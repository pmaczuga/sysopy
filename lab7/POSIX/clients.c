#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include "common.h"

struct SharedMemory *mem;
sem_t *semMem;
sem_t *semBarberReady;
sem_t *semClientCutting;
sem_t *semClientInvitation;
sem_t *semClientSits;

pid_t allClients[MAX_SIZE];
int clientsNumber;

void prepareClients(int number, int visits)
{
    int shMemory = shm_open(MEMORY_NAME, O_RDWR, 0660);
    if(shMemory == -1)
    {
        perror("ERROR: Couldn't open shared memory - queue");
        exit(EXIT_FAILURE);
    }
    if(ftruncate(shMemory, sizeof(struct SharedMemory)) == -1)
    {
        perror("ERROR: Couldn't allocate memory - queue");
        exit(EXIT_FAILURE);
    }

    mem = mmap(NULL, sizeof(struct SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shMemory, 0);
    if(mem == MAP_FAILED)
    {
        perror("ERROR: Problem with mapping");
        exit(EXIT_FAILURE);
    }

    semMem = sem_open(SEM_MEM_NAME, O_RDWR);
    semBarberReady = sem_open(SEM_BARBER_NAME, O_RDWR);
    semClientCutting = sem_open(SEM_CLIENT_NAME, O_RDWR);
    semClientInvitation = sem_open(SEM_CLIENT_INV_NAME, O_RDWR);
    semClientSits = sem_open(SEM_CLIENT_SITS_NAME, O_RDWR);

    if(semMem == SEM_FAILED ||
        semBarberReady == SEM_FAILED ||
        semClientCutting == SEM_FAILED ||
        semClientInvitation == SEM_FAILED ||
        semClientSits == SEM_FAILED
        )
    {
        perror("ERROR: Problem opening semaphores");
        exit(EXIT_FAILURE);
    }
}

void handleINT(int sig)
{
    exit(EXIT_SUCCESS);
}

void handleInvitation()
{
    sem_post(semClientInvitation);
}

void prepareSignals()
{
    struct sigaction act;
    act.sa_handler = handleINT;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);

    act.sa_handler = handleInvitation;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGUSR1, &act, NULL);

    sigset_t blocked;
    sigfillset(&blocked);
    sigdelset(&blocked, SIGINT);
    sigdelset(&blocked, SIGUSR1);
    sigprocmask(SIG_SETMASK, &blocked, NULL);
}

void killClients()
{
    int i = 0;
    while(i < clientsNumber)
    {
        kill(allClients[i], SIGINT);
        wait(NULL);
        i++;
    }
}

void cleanUp()
{
    munmap(mem, sizeof(struct SharedMemory));
    sem_close(semMem);
    sem_close(semBarberReady);
    sem_close(semClientCutting);
    sem_close(semClientInvitation);
    sem_close(semClientSits);
}

void clientMakerCleanUp()
{
    cleanUp();
    killClients();
}

void clientBehaviour(int visits)
{

    int i = 0;
    while(i < visits)
    {
        // check if barber sleeps
        sem_wait(semMem);
        if(mem->barberSleeps == 1)
        {
            // barber sleeps - wake him up
            mem->barberSleeps = 0;
            mem->currentClient = getpid();
            sem_post(semMem);
            myLog("Wakes barber");
            sem_post(semBarberReady);
        }
        else if(queueFull(&mem->queue) == 0)
        {
            // barber is awake - sit in queue
            queuePut(&mem->queue, getpid());
            sem_post(semMem);
            myLog("Sits in queue");

            while(1)
            {
                sem_wait(semClientInvitation);
                sem_wait(semMem);
                if(mem->currentClient == getpid())
                {
                    sem_post(semMem);
                    break;
                }
                sem_post(semMem);
                sem_post(semClientInvitation);
            }
        }
        else
        {
            // no places in queue - leave
            sem_post(semMem);
            myLog("Leaves. No free sits in queue");
            i++;
            continue;
        }
        myLog("Sits on a cutting chair");
        sem_post(semClientSits);
        sem_wait(semClientCutting);
        myLog("Leaves with hair cut");

        i++;
    }

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("Not enough arguments");
        exit(EXIT_FAILURE);
    }

    clientsNumber = atoi(argv[1]);
    int visits = atoi(argv[2]);

    atexit(cleanUp);
    prepareSignals();
    prepareClients(clientsNumber, visits);

    int i = 0;
    while(i < clientsNumber)
    {
        pid_t child = 0;
        child = fork();
        if(child == 0)
        {
            clientBehaviour(visits);
        }
        allClients[i] = child;
        i++;
    }
    atexit(killClients);

    i = 0;
    while(i < clientsNumber)
    {
        wait(NULL);
        i++;
    }

    return EXIT_SUCCESS;
}
