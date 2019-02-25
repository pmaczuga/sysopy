#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "header.h"

struct universalMessage
{
	long type;
	pid_t sender;
	key_t key;
	int number1;
	int number2;
	char text[MAX_SIZE - 2 * sizeof(int) - sizeof(pid_t) - sizeof(key_t)];
};

struct client
{
    char id;
	int pid;
	mqd_t qid;
};

struct client *clients[MAX_CLIENTS];
mqd_t qid;


void initClients()
{
    int i = 0;
    while(i < MAX_CLIENTS)
    {
        clients[i] = NULL;
        i++;
    }
}

int getClient(pid_t pid)
{
	int i = 0;
	while(i < MAX_CLIENTS)
	{
		if(clients[i] !=NULL && pid == clients[i]->pid)
			return i;
		i++;
	}
	return -1;
}

int addClient(pid_t pid, char *name)
{
    int i = 0;
    while(i < MAX_CLIENTS)
    {
        if(clients[i] == NULL)
        {
            clients[i] = malloc(sizeof(struct client));
            clients[i]->id = i+1;
            clients[i]->pid = pid;
            clients[i]->qid = mq_open(name, O_WRONLY);
            if(clients[i]->qid == -1)
            {
                perror("Error opening clients queue");
                return -1;
            }
            printf("Clients queue opened: %d\n", (unsigned int)clients[i]->qid);
            printf("With name: %s\n\n", name);
            return i;
        }
        i++;
    }
    return -1;
}

void removeClient(pid_t pid)
{
    int client = getClient(pid);
    clients[client] = NULL;
}

void cleanUp()
{
    printf("\nCleaning up\n");
    char message[MAX_SIZE];

    int i = 0;
    while(i < MAX_CLIENTS)
    {
        if(clients[i] != NULL)
        {
            kill(clients[i]->pid, SIGINT);
            sleep(0.1);
            mq_receive(qid, message ,MAX_SIZE, NULL);
            mq_send(clients[i]->qid, message, MAX_SIZE, END_PRIO);
            mq_close(clients[i]->qid);
        }
        i++;
    }

    mq_close(qid);
	mq_unlink(SERVER_QUEUE);
}

void handleINT()
{
    exit(EXIT_SUCCESS);
}


char *mirror(char *text)
{
    char *returnText = malloc(sizeof(text) + 1);
    int lastIndex = strlen(text) - 1;
    int i = 0;
    while(lastIndex >= 0)
    {
        returnText[i] = text[lastIndex];
        lastIndex--;
        i++;
    }
    returnText[i] = '\0';

    return returnText;
}

char *getTime()
{
    char *returnText = malloc(30);
    FILE *dateCall = popen("date", "r");
    fread(returnText, 1, MAX_SIZE, dateCall);

    return returnText;
}

char *getAdd(char *text)
{
    char *res = malloc(12);
    int a, b;
    sscanf(text, "%d %d", &a, &b);
    sprintf(res, "%d", a + b);
    return res;
}

char *getSub(char *text)
{
    char *res = malloc(12);
    int a, b;
    sscanf(text, "%d %d", &a, &b);
    sprintf(res, "%d", a - b);
    return res;
}

char *getMul(char *text)
{
    char *res = malloc(12);
    int a, b;
    sscanf(text, "%d %d", &a, &b);
    sprintf(res, "%d", a * b);
    return res;
}

char *getDiv(char *text)
{
    char *res = malloc(12);
    int a, b;
    sscanf(text, "%d %d", &a, &b);
    sprintf(res, "%d", a / b);
    return res;
}

pid_t getSender(char *message)
{
    char *sender = malloc(13);
    int i = 0;
    while(i < 12)
    {
        sender[i] = message[i+1];
        i++;
    }
    sender[i] = '\0';

    return (pid_t)atoi(sender);
}

char *getText(char * message)
{
    char *text = malloc(MAX_SIZE - 13);
    int i = 0;
    while(message[i+13] != '\0')
    {
        text[i] = message[i+13];
        i++;
    }
    text[i] = '\0';

    return text;
}

int main(int argc, char *argv[])
{
    atexit(cleanUp);
	signal(SIGINT, handleINT);
	initClients();

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;
    qid = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT | O_NONBLOCK, 0660, &attr);
	if(qid == -1)
    {
        perror("Error in mq_open()");
        return EXIT_FAILURE;
    }

    printf("Server created\n");
    printf("Name: %s\n", SERVER_QUEUE);
    printf("Server queue ID: %d\n\n", (unsigned int)qid);

    int endFlag = 0;

	char message[MAX_SIZE];
	int client;
	pid_t sender;
	char type;
	char *text;
    char *response;
	while(1)
    {
        if(mq_receive(qid, message, MAX_SIZE, NULL) != -1)
        {
            sender = getSender(message);
            type = message[0];
            text = getText(message);

            printf("Got message: %d\n", type);
            printf("From: %d\n\n", sender);

            switch (type)
            {
                case START:
                    client = addClient(sender, text);
                    if(client == -1)
                    {
                        printf("Too many clients already\n");
                    }
                    else
                    {
                        sprintf(message, "%c", clients[client]->id);
                        mq_send(clients[client]->qid, message, MAX_SIZE, PRIO);
                    }
                    break;
                case STOP:
                    client = getClient(sender);
                    sprintf(message, "%c", clients[client]->id);
                    mq_send(clients[client]->qid, message, MAX_SIZE, END_PRIO);
                    removeClient(sender);
                    break;
                case END:
                    endFlag = 1;
                    break;
                case MIRROR:
                    client = getClient(sender);
                    response = mirror(text);
                    sprintf(message, "%c%s", clients[client]->id, response);
                    mq_send(clients[client]->qid, message, MAX_SIZE, PRIO);
                    break;
                case TIME:
                    client = getClient(sender);
                    response = getTime();
                    sprintf(message, "%c%s", clients[client]->id, response);
                    mq_send(clients[client]->qid, message, MAX_SIZE, PRIO);
                    break;
                case ADD:
                    client = getClient(sender);
                    response = getAdd(text);
                    sprintf(message, "%c%s", clients[client]->id, response);
                    mq_send(clients[client]->qid, message, MAX_SIZE, PRIO);
                    break;
                case SUB:
                    client = getClient(sender);
                    response = getSub(text);
                    sprintf(message, "%c%s", clients[client]->id, response);
                    mq_send(clients[client]->qid, message, MAX_SIZE, PRIO);
                    break;
                case MUL:
                    client = getClient(sender);
                    response = getMul(text);
                    sprintf(message, "%c%s", clients[client]->id, response);
                    mq_send(clients[client]->qid, message, MAX_SIZE, PRIO);
                    break;
                case DIV:
                    client = getClient(sender);
                    response = getDiv(text);
                    sprintf(message, "%c%s", clients[client]->id, response);
                    mq_send(clients[client]->qid, message, MAX_SIZE, PRIO);
                    break;
            }
        }
        else
        {
            if(endFlag == 1)
                break;
        }
    }


	return EXIT_SUCCESS;
}
