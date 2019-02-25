#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

// max buffer size
#define MAX_SIZE 4096
// max line length
#define MAX_LENGTH 1024

// print mode - possible values
#define SIMPLE_MODE 0
#define DESC_MODE 1

struct Config_params
{
    int P;
    int C;
    int L;
    int N;
    int nk;
    int print_mode;
    char input_name[64];
};

struct Buffer
{
    char *buf[MAX_SIZE];
    int p_pos;
    int c_pos;
    int size;
};

int produce_counter;
int consume_counter;
struct Buffer buffer;
int print_mode;
FILE *input;

// semaphores
sem_t sem_memory;
sem_t sem_fill_count;
sem_t sem_empty_count;
sem_t sem_eof;

// Parsing config file
int parse_config(FILE *file, struct Config_params *params)
{
    char buf[64];
    char key[64];
    char data[64];

    while(fgets(buf, 64, file) != NULL)
    {
        int i = 0;
        while(buf[i] != '=' && buf[i] != '\0' && buf[i] != '\n')
        {
            key[i] = buf[i];
            i++;
        }
        key[i] = '\0';

        if(buf[i] != '=')
            return -1;

        i++;
        int j = 0;
        while(buf[i] != '\0' && buf[i] != '\n')
        {
            data[j] = buf[i];
            j++;
            i++;
        }
        data[j] = '\0';

        if(strcmp(key, "consumers") == 0)
            params->C = atoi(data);
        else if(strcmp(key, "producers") == 0)
            params->P = atoi(data);
        else if(strcmp(key, "buffer_size") == 0)
            params->N = atoi(data);
        else if(strcmp(key, "string_length") == 0)
            params->L = atoi(data);
        else if(strcmp(key, "thread_time") == 0)
            params->nk = atoi(data);
        else if(strcmp(key, "input_name") == 0)
            strcpy(params->input_name, data);
        else if(strcmp(key, "print_mode") == 0)
        {
            if(strcmp(data, "simple") == 0)
                params->print_mode = SIMPLE_MODE;
            else if(strcmp(data, "desc") == 0)
                params->print_mode = DESC_MODE;
            else
                return -1;
        }
        else
            return -1;
    }
    return 0;
}

int init_buffer(struct Buffer *buffer, int size)
{
    if(size <= 0)
        return -1;
    buffer->c_pos = 0;
    buffer->p_pos = 0;
    buffer->size = size;
    return 0;
}

void buffer_add(struct Buffer *buffer, char *string)
{
    char *el = malloc(strlen(string) + 1);
    strcpy(el, string);
    buffer->buf[buffer->p_pos] = el;
    buffer->p_pos = (buffer->p_pos + 1) % buffer->size;
}

char *buffer_remove(struct Buffer *buffer)
{
    char *el = buffer->buf[buffer->c_pos];
    buffer->c_pos = (buffer->c_pos + 1) % buffer->size;
    return el;
}

void clean_up()
{
    if(sem_destroy(&sem_memory) == -1)
        perror("ERROR: Problem destroying semaphore: sem_memory");
    if(sem_destroy(&sem_fill_count) == -1)
        perror("ERROR: Problem destroying semaphore: sem_fill_count");
    if(sem_destroy(&sem_empty_count) == -1)
        perror("ERROR: Problem destroying semaphore: sem_empty_count");
    if(sem_destroy(&sem_eof) == -1)
        perror("ERROR: Problem destroying semaphore: sem_eof");
}

void *producer(void *arg)
{
    if(print_mode == DESC_MODE)
        printf("Producer thread started\n");

    char line[MAX_LENGTH];

    while(1)
    {
        sem_wait(&sem_empty_count);
        sem_wait(&sem_memory);

        if(fgets(line, MAX_LENGTH, input) != NULL)
        {
            if(print_mode == DESC_MODE)
                printf("Producer adds line: %s\n", line);
            buffer_add(&buffer, line);
            produce_counter++;
            sem_post(&sem_memory);
            sem_post(&sem_fill_count);
        }
        else
        {
            sem_post(&sem_eof);
            sem_post(&sem_memory);
        }
    }

    return NULL;
}

void *consumer(void *arg)
{
    int *argument = (int *) arg;
    int length = argument[0];
    char *line;

    if(print_mode == DESC_MODE)
        printf("Consumer thread started with arg: %d\n", length);

    while(1)
    {
        sem_wait(&sem_fill_count);
        sem_wait(&sem_memory);

        line = buffer_remove(&buffer);
        if(print_mode == DESC_MODE)
            printf("Consumer got line: %s\n", line);

        consume_counter++;
        if(strlen(line) == length)
        {
            printf("Consumer found right line: %s\n", line);
        }
        free(line);

        sem_post(&sem_memory);
        sem_post(&sem_empty_count);
    }

    return NULL;
}

void handle_INT(int sig)
{
    printf("Produced %d lines\n", produce_counter);
    printf("Consumed %d lines\n", consume_counter);

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    // Clean up at exit
    atexit(clean_up);

    // INT signal handler
    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = handle_INT;
    sigaction(SIGINT, &act, NULL);

    // Check if there are enough arguments
    if(argc < 2)
    {
        printf("Synopis\nmain CONFIG\n");
        exit(EXIT_SUCCESS);
    }

    // Open config file
    FILE *config = fopen(argv[1], "r");
    if(config == NULL)
    {
        fprintf(stderr, "ERROR: Problem with opening config file\n");
        exit(EXIT_FAILURE);
    }

    // Parse config file - fill Config_params struct
    struct Config_params params;
    if(parse_config(config, &params) == -1)
    {
        fprintf(stderr, "ERROR: Problem with config file\n");
        exit(EXIT_FAILURE);
    }
    fclose(config);

    // Set print_mode variable
    print_mode = params.print_mode;

    // Open input file
    input = fopen(params.input_name, "r");
    if(input == NULL)
    {
        fprintf(stderr, "ERROR: Problem with opening input file\n");
        exit(EXIT_FAILURE);
    }

    // Initialize buffer
    if(init_buffer(&buffer, params.N) == -1)
    {
        fprintf(stderr, "ERROR: Problem initizlizng buffer\n");
        exit(EXIT_FAILURE);
    }

    // Initialize semaphores
    if(sem_init(&sem_memory, 0, 1) == -1)
        perror("ERROR: Problem destroying semaphore: sem_memory");
    if(sem_init(&sem_fill_count, 0, 0) == -1)
        perror("ERROR: Problem destroying semaphore: sem_fill_count");
    if(sem_init(&sem_empty_count, 0, params.N) == -1)
        perror("ERROR: Problem destroying semaphore: sem_empty_count");
    if(sem_init(&sem_eof, 0, 0) == -1)
        perror("ERROR: Problem destroying semaphore: sem_eof");

    // Arrays with threads id
    pthread_t producers[params.P];
    pthread_t consumers[params.C];

    // Thread argument - length of proper line
    int thread_arg[1];
    thread_arg[0] = params.L;

    // Init global variables
    produce_counter = 0;
    consume_counter = 0;

    // Create threads
    int i = 0;
    while(i < params.P)
    {
        if(pthread_create(&producers[i], NULL, producer, NULL) != 0)
        {
            perror("ERROR: Problem creating producer thread");
            exit(EXIT_FAILURE);
        }
        i++;
    }
    i = 0;
    while(i < params.C)
    {
        if(pthread_create(&consumers[i], NULL, consumer, thread_arg) != 0)
        {
            perror("ERROR: Problem creating consumer thread");
            exit(EXIT_FAILURE);
        }
        i++;
    }

    // Wait nk seconds
    if(params.nk > 0)
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += params.nk;

        int res = sem_timedwait(&sem_eof, &ts);

        if(res == -1 && errno == ETIMEDOUT)
            printf("Time passed\n");
        else
            printf("End of file reached\n");
    }
    // Or untill end of file is reached
    else
    {
        sem_wait(&sem_eof);
        printf("End of file reached\n");
    }

    // Cancell threads
    i = 0;
    while(i < params.P)
    {
        if(pthread_cancel(producers[i]) != 0)
            perror("ERROR: Problem cancelling producer thread");
        i++;
    }
    i = 0;
    while(i < params.C)
    {
        if(pthread_cancel(consumers[i]) != 0)
            perror("ERROR: Problem cancelling producer thread");
        i++;
    }

    fclose(input);

    return EXIT_SUCCESS;
}
