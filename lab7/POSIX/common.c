#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

struct timespec tp;

void myLog(const char *text)
{
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("[%ld.%ld] %d: %s\n", tp.tv_sec, tp.tv_nsec, getpid(), text);
}

void queueInit(struct Queue *queue ,int maxSize)
{
    queue->maxSize = maxSize;
    queue->queueEnd = 0;
    queue->queueStart = 0;
    queue->size = 0;
}

int queueEmpty(struct Queue *queue)
{
    if(queue->size == 0)
        return 1;
    else
        return 0;
}
int queueFull(struct Queue *queue)
{
    if(queue->size == queue->maxSize)
        return 1;
    else
        return 0;
}
int queuePut(struct Queue *queue, pid_t el)
{
    if(queueFull(queue) == 1)
        return -1;
    queue->array[queue->queueEnd] = el;
    queue->queueEnd = (queue->queueEnd + 1) % queue->maxSize;
    queue->size++;
    return 0;
}
int queueGet(struct Queue *queue)
{
    if(queueEmpty(queue) == 1)
        return -1;
    pid_t el = queue->array[queue->queueStart];
    queue->queueStart = (queue->queueStart + 1) % queue->maxSize;
    queue->size--;
    return el;
}

