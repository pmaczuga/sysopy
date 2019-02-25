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

#include "common.h"

struct SharedMemory *mem;
sem_t *semMem;
sem_t *semBarberReady;
sem_t *semClientCutting;
sem_t *semClientInvitation;
sem_t *semClientSits;

void cleanUp()
{
    printf("Cleaning up\n");
    if(munmap(mem, sizeof(struct SharedMemory)) == -1)
        perror("ERROR: Cleannig up - Problem with deleting the mapping");
    if(shm_unlink(MEMORY_NAME) == -1)
        perror("ERROR: Cleanig up - Problem with unlinking memory");
    if(sem_close(semMem) == -1)
        perror("ERROR: Cleaning up - Problem closing semMem semaphore");
    if(sem_close(semBarberReady) == -1)
        perror("ERROR: Cleaning up - Problem closing semBarberReady semaphore");
    if(sem_close(semClientCutting) == -1)
        perror("ERROR: Cleaning up - Problem closing semClientCutting semaphore");
    if(sem_close(semClientInvitation) == -1)
        perror("ERROR: Cleaning up - Problem closing semClientInvitation semaphore");
    if(sem_close(semClientSits) == -1)
        perror("ERROR: Cleaning up - Problem closing semClientSits semaphore");
    if(sem_unlink(SEM_MEM_NAME) == -1)
        perror("ERROR: Cleaning up - Problem unlinking semMem semaphore");
    if(sem_unlink(SEM_BARBER_NAME) == -1)
        perror("ERROR: Cleaning up - Problem unlinking semBarberReady semaphore");
    if(sem_unlink(SEM_CLIENT_NAME) == -1)
        perror("ERROR: Cleaning up - Problem unlinking semClientCutting semaphore");
    if(sem_unlink(SEM_CLIENT_INV_NAME) == -1)
        perror("ERROR: Cleaning up - Problem unlinking semClientInvitation semaphore");
    if(sem_unlink(SEM_CLIENT_SITS_NAME) == -1)
        perror("ERROR: Cleaning up - Problem unlinking semClientSits semaphore");

}

void prepareBarber(int places)
{
    myLog("Barber is preparing workshop");
    int shMemory = shm_open(MEMORY_NAME, O_RDWR | O_CREAT, 0660);
    if(shMemory == -1)
    {
        perror("ERROR: Couldn't create shared memory - queue");
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

    semMem = sem_open(SEM_MEM_NAME, O_CREAT | O_EXCL | O_RDWR, 0660, 1);
    semBarberReady = sem_open(SEM_BARBER_NAME, O_CREAT | O_EXCL | O_RDWR, 0660, 0);
    semClientCutting = sem_open(SEM_CLIENT_NAME, O_CREAT | O_EXCL | O_RDWR, 0660, 0);
    semClientInvitation = sem_open(SEM_CLIENT_INV_NAME, O_CREAT | O_EXCL | O_RDWR, 0660, 0);
    semClientSits = sem_open(SEM_CLIENT_SITS_NAME, O_CREAT | O_EXCL | O_RDWR, 0660, 0);

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
        sem_wait(semMem);
        if(queueEmpty(&mem->queue) == 1)
        {
            // Nobody waits - barber goes to slepp
            mem->barberSleeps = 1;
            myLog("Barber is going to sleep");
            sem_post(semMem);
            sem_wait(semBarberReady);
            myLog("Barber wakes up");
        }
        else
        {
            // Sombody waits - barber invites him
            pid_t client = queueGet(&mem->queue);
            mem->currentClient = client;
            sprintf(buf, "Barber invites: %d", client);
            myLog(buf);
            sem_post(semMem);
            sem_post(semClientInvitation);
        }
        // client sits and is being cut
        sem_wait(semMem);
        pid_t client = mem->currentClient;
        sem_post(semMem);
        sem_wait(semClientSits);
        sprintf(buf, "Barber starts cutting: %d", client);
        myLog(buf);
        sprintf(buf, "Barber finishes cutting: %d", client);
        myLog(buf);
        sem_post(semClientCutting);
    }

    return EXIT_SUCCESS;
}
