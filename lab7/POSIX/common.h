#ifndef header
#define header

#include <sys/types.h>

#define MEMORY_NAME "/memoryName"
#define SEM_MEM_NAME "/semMemoryName"
#define SEM_BARBER_NAME "/semBarberName"
#define SEM_CLIENT_NAME "/semClientName"
#define SEM_CLIENT_INV_NAME "/semClientInvName"
#define SEM_CLIENT_SITS_NAME "/semClientSitsName"

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

void myLog(const char *text);

void queueInit(struct Queue *queue ,int maxSize);
int queueEmpty(struct Queue *queue);
int queueFull(struct Queue *queue);
int queuePut(struct Queue *queue, pid_t el);
int queueGet(struct Queue *queue);

#endif // header
