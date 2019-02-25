#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "header.h"

mqd_t myQueue;
mqd_t serverQueue;
char name[3];

int fromFileFlag = 0;
FILE *stream;

//---------------------------PARSER--------------------------------
int countWords(char *line)
{
    int count = 0;
    int lastWhite = 1;
    int i = 0;
    while(line[i] != '\0')
    {
        if(lastWhite == 1 && line[i] != ' ')
        {
            count++;
            lastWhite = 0;
        }
        else if(line[i] == ' ')
            lastWhite = 1;
        i++;
    }
    return count;
}

int finishWord(char *line, int i)
{
    while(line[i] != '\0' && line[i] != '\n' && line[i] != ' ')
    {
        i++;
    }
    return i;
}

void copy(char *to, char *from, int i, int j)
{
    int toIndex = 0;
    while(i < j)
    {
        to[toIndex] = from[i];
        i++;
        toIndex++;
    }
    to[toIndex] = '\0';
}

int parseLine(char *line, char ***parsedPointer)
{
    char **parsed = *parsedPointer;
    int words = countWords(line);
    parsed = malloc(sizeof(char *) * words);
    int wordIndex = 0;
    int i = 0;
    while(line[i] != '\0')
    {
        if(line[i] == ' ' || line[i] == '\n')
        {
            i++;
        }
        else
        {
            int j = finishWord(line, i);
            parsed[wordIndex] = malloc(j - i + 1);
            copy(parsed[wordIndex], line, i, j);
            i = j;
            wordIndex++;
        }
    }
    *parsedPointer = parsed;
    return words;
}
//-------------------END-OF-PARSER-----------------------------------------

void cleanUp()
{
    printf("Cleaning up\n");
    char message[MAX_SIZE];
    sprintf(message, "%c%012d", STOP, getpid());
    mq_send(serverQueue, message, MAX_SIZE, END_PRIO);
    mq_close(serverQueue);
    mq_receive(myQueue, message, MAX_SIZE, NULL);
    mq_close(myQueue);
    mq_unlink(name);
}

void handleINT()
{
	exit(EXIT_SUCCESS);
}

char *getText(char *message)
{
    char *text = malloc(MAX_SIZE - 1);
    int i = 0;
    while(message[i+1] != '\0')
    {
        text[i] = message[i+1];
        i++;
    }
    text[i] = '\0';
    return text;
}

int isNumber(char *arg)
{
    int i = 0;
    while(arg[i] != '\0')
    {
        if(arg[i] < 48 || arg[i] > 57)
            return 1;

        i++;
    }
    return 0;
}

int fillMessage(char *message, int size, char **line)
{
    if(size < 1)
        return -1;

    if(strcmp(line[0], "TIME") == 0)
    {
        sprintf(message, "%c%012d", TIME, getpid());
    }
    else if(strcmp(line[0], "MIRROR") == 0)
    {
        if(size < 2)
            return -1;
        sprintf(message, "%c%012d%s", MIRROR, getpid(), line[1]);
    }
    else if(strcmp(line[0], "END") == 0)
    {
        sprintf(message, "%c%012d", END, getpid());
    }
    else
    {
        char type;
        if(strcmp(line[0], "ADD") == 0)
            type = ADD;
        else if(strcmp(line[0], "SUB") == 0)
            type = SUB;
        else if(strcmp(line[0], "MUL") == 0)
            type = MUL;
        else if(strcmp(line[0], "DIV") == 0)
            type = MUL;
        else
            return -1;
        if(size < 3)
            return -1;
        if(isNumber(line[1]) != 0 || isNumber(line[2]) != 0)
            return -1;
        sprintf(message, "%c%012d%s %s", type, getpid(), line[1], line[2]);
    }

    return 0;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	atexit(cleanUp);
	signal(SIGINT, handleINT);
    stream = stdin;

    name[0] = '/';
    name[2] = '\0';

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;
    do
    {
        name[1] = rand()%26 + 97;
        myQueue = mq_open(name, O_RDONLY | O_CREAT | O_EXCL, 0660, &attr);
    }while(myQueue < 0);

    printf("Client set\n");
    printf("Client PID: %d\n", getpid());
    printf("Client name: %s\n", name);
    printf("Client queue: %d\n", (unsigned int)myQueue);

    serverQueue = mq_open(SERVER_QUEUE, O_WRONLY);
    if(serverQueue == -1)
    {
        perror("Error in mq_open() - server");
        return EXIT_FAILURE;
    }

    printf("Server queue: %d\n\n", (unsigned int)serverQueue);

    char message[MAX_SIZE];
    sprintf(message, "%c%012d%s", START, getpid(), name);
    if(mq_send(serverQueue, message, MAX_SIZE, PRIO) == -1)
    {
        perror("Error in mq_send() - start message");
        return EXIT_FAILURE;
    }

    printf("Sent START message\n");

    if(mq_receive(myQueue, message, MAX_SIZE, NULL) == -1)
    {
        perror("Error in mq_receive() - start message");
        return EXIT_FAILURE;
    }

    printf("Got response\n\n");


    if(argc > 1)
    {
        fromFileFlag = 1;
        stream = fopen(argv[1], "r");
        if(stream == NULL)
        {
            perror("Problem opening file");
            return EXIT_FAILURE;
        }
    }
    else
    {
        stream = stdin;
    }


    while(1)
    {
        char line[MAX_SIZE - 13];
        char *endOfFile = fgets(line, MAX_SIZE - 13, stream);
        if(endOfFile == NULL)
            break;

        char **parsedLine;
        int size = parseLine(line, &parsedLine);
        int status = fillMessage(message, size, parsedLine);
        if(status == -1)
        {
            printf("Wrong input line\n");
            continue;
        }

        if(mq_send(serverQueue, message, MAX_SIZE, PRIO) == -1)
        {
            perror("Error sending message");
            return EXIT_FAILURE;
        }

        if(mq_receive(myQueue, message, MAX_SIZE, NULL) == -1)
        {
            perror("Error receiving message");
            return EXIT_FAILURE;
        }

        char *text = getText(message);
        printf("Answer: %s\n\n", text);
    }

	return EXIT_SUCCESS;
}
