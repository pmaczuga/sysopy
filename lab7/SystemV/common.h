#ifndef header
#define header

#include <sys/types.h>

#define PATH "."
#define MEMORY_ID 7
#define SEM_ID 11

#define semMem 0
#define semBarberReady 1
#define semClientCutting 2
#define semClientInvitation 3
#define semClientSits 4

#define MAX_SIZE 10000

struct Queue
{
    pid_t array[MAX_SIZE];
    int maxSize;
    int queueStart;
    int queueEnd;
    int size;
};

struct SharedMemory
{
    struct Queue queue;
    int barberSleeps;
    pid_t currentClient;
};

int semPost(int semid, int semnum);
int semWait(int semid, int semnum);
int semInit(int semid, int semnum, int value);

void myLog(const char *text);

void queueInit(struct Queue *queue ,int maxSize);
int queueEmpty(struct Queue *queue);
int queueFull(struct Queue *queue);
int queuePut(struct Queue *queue, pid_t el);
int queueGet(struct Queue *queue);

#endif // header
