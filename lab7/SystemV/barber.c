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

#include "common.h"

struct SharedMemory *mem;
int shMemory;
int semNum = 5;
int semid;

void cleanUp()
{
    printf("Cleaning up\n");

    if(shmdt(mem) == -1)
        perror("ERROR: Cleannig up - Problem with deleting the mapping");
    if(shmctl(shMemory, IPC_RMID, NULL) == -1)
        perror("ERROR: Cleanig up - Problem with removing memory");

    if(semctl(semid, 0, IPC_RMID) == -1)
        perror("ERROR: Cleaning up - Problem removing semaphores");
}

void prepareBarber(int places)
{
    myLog("Barber is preparing workshop");

    key_t memKey = ftok(PATH, MEMORY_ID);
    shMemory = shmget(memKey, sizeof(struct SharedMemory), IPC_CREAT | IPC_EXCL | 0660);
    if(shMemory == -1)
    {
        perror("ERROR: Couldn't get shared memory");
        exit(EXIT_FAILURE);
    }

    mem = shmat(shMemory, NULL, 0);
    if(mem == (void *) -1)
    {
        perror("ERROR: Problem with mapping");
        exit(EXIT_FAILURE);
    }

    key_t semkey = ftok(PATH, SEM_ID);
    semid = semget(semkey, semNum, IPC_CREAT | IPC_EXCL | 0660);

    if(semid == -1)
    {
        perror("ERROR: Problem getting semaphores");
        exit(EXIT_FAILURE);
    }

    if( semInit(semid, semMem, 1) == -1 ||
        semInit(semid, semBarberReady, 0) == -1 ||
        semInit(semid, semClientCutting, 0) == -1 ||
        semInit(semid, semClientInvitation, 0) == -1 ||
        semInit(semid, semClientSits, 0) == -1
        )
    {
        perror("ERROR: Problem initizlizng semaphores");
        exit(EXIT_FAILURE);
    }

    queueInit(&mem->queue, places);
    mem->barberSleeps = 0;
    myLog("Barber finished preparing workshop");
}

void handleSIGTERM(int sig)
{
    printf("\nGot order from above. Closing\n");
    exit(EXIT_SUCCESS);
}

void prepareSignals()
{
    struct sigaction act;
    act.sa_handler = handleSIGTERM;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Not enough arguments\n");
        exit(EXIT_FAILURE);
    }

    atexit(cleanUp);
    prepareBarber(atoi(argv[1]));
    prepareSignals();

    char buf[1024];
    while(1)
    {
        // Check if anyone is waiting
        semWait(semid ,semMem);
        if(queueEmpty(&mem->queue) == 1)
        {
            // Nobody waits - barber goes to slepp
            mem->barberSleeps = 1;
            myLog("Barber is going to sleep");
            semPost(semid, semMem);
            semWait(semid, semBarberReady);
            myLog("Barber wakes up");
        }
        else
        {
            // Sombody waits - barber invites him
            pid_t client = queueGet(&mem->queue);
            mem->currentClient = client;
            sprintf(buf, "Barber invites: %d", client);
            myLog(buf);
            semPost(semid, semMem);
            semPost(semid ,semClientInvitation);
        }
        // client sits and is being cut
        semWait(semid, semMem);
        pid_t client = mem->currentClient;
        semPost(semid, semMem);
        semWait(semid, semClientSits);
        sprintf(buf, "Barber starts cutting: %d", client);
        myLog(buf);
        sprintf(buf, "Barber finishes cutting: %d", client);
        myLog(buf);
        semPost(semid, semClientCutting);
    }

    return EXIT_SUCCESS;
}
