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
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#include "common.h"

struct SharedMemory *mem;
int shMemory;
int semNum = 5;
int semid;

pid_t allClients[MAX_SIZE];
int clientsNumber;

void prepareClients(int number, int visits)
{
    key_t memKey = ftok(PATH, MEMORY_ID);
    shMemory = shmget(memKey, sizeof(struct SharedMemory), 0);
    if(shMemory == -1)
    {
        perror("ERROR: Couldn't open shared memory");
        exit(EXIT_FAILURE);
    }

    mem = shmat(shMemory, NULL, 0);
    if(mem == (void *) -1)
    {
        perror("ERROR: Problem with mapping");
        exit(EXIT_FAILURE);
    }

    key_t semKey = ftok(PATH, SEM_ID);
    semid = semget(semKey, semNum, 0);

    if(semid == -1)
    {
        perror("ERROR: Problem getting semaphores");
        exit(EXIT_FAILURE);
    }
}

void handleINT(int sig)
{
    exit(EXIT_SUCCESS);
}

void prepareSignals()
{
    struct sigaction act;
    act.sa_handler = handleINT;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);
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
    shmdt(mem);
}

void clientBehaviour(int visits)
{
    int i = 0;
    while(i < visits)
    {
        // check if barber sleeps
        semWait(semid, semMem);
        if(mem->barberSleeps == 1)
        {
            // barber sleeps - wake him up
            mem->barberSleeps = 0;
            mem->currentClient = getpid();
            semPost(semid, semMem);
            myLog("Wakes barber");
            semPost(semid, semBarberReady);
        }
        else if(queueFull(&mem->queue) == 0)
        {
            // barber is awake - sit in queue
            queuePut(&mem->queue, getpid());
            semPost(semid, semMem);
            myLog("Sits in queue");

            while(1)
            {
                semWait(semid, semClientInvitation);
                semWait(semid, semMem);
                if(mem->currentClient == getpid())
                {
                    semPost(semid, semMem);
                    break;
                }
                semPost(semid, semMem);
                semPost(semid, semClientInvitation);
            }
        }
        else
        {
            // no places in queue - leave
            semPost(semid, semMem);
            myLog("Leaves. No free sits in queue");
            i++;
            continue;
        }
        myLog("Sits on a cutting chair");
        semPost(semid, semClientSits);
        semWait(semid, semClientCutting);
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
