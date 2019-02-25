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
	long id;
	int pid;
	int qid;
};

struct client *clients[MAX_CLIENTS];
int qid;


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

int addClient(pid_t pid, key_t key)
{
    int successfullyAdded = 0;

    int i = 0;
    while(i < MAX_CLIENTS)
    {
        if(clients[i] == NULL)
        {
            clients[i] = malloc(sizeof(struct client));
            clients[i]->pid = pid;
            clients[i]->id = i + 1;
            clients[i]->qid = msgget(key, 0);
            if(clients[i]->qid == -1)
            {
                perror("Error opening clients queue");
                return -1;
            }
            printf("Clients queue opened: %d\n", clients[i]->qid);
            printf("With key: %lu\n\n", (unsigned long)key);
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

    int i = 0;
    while(i < MAX_CLIENTS)
    {
        if(clients[i] != NULL)
        {
            kill(clients[i]->pid, SIGINT);
        }
        i++;
    }

	msgctl(qid, IPC_RMID, NULL);
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


int main(int argc, char *argv[])
{
    atexit(cleanUp);
	signal(SIGINT, handleINT);
	initClients();

    key_t serverKey;
	serverKey = ftok(KEY_PATH, PROJECT_ID);
	if(serverKey == -1)
	{
		perror("Error in ftok()");
		return EXIT_FAILURE;
	}

	qid = msgget(serverKey, IPC_CREAT | 0660);
	if(qid == -1)
	{
		perror("Error in server msgget()");
		return EXIT_FAILURE;
	}

	printf("Server created\n");
	printf("Key: %lu\n", (unsigned long)serverKey);
	printf("Server queue ID: %d\n\n", qid);

    struct universalMessage message;
    int endFlag = 0;

	while(1)
	{
		if(msgrcv(qid, &message, MAX_SIZE, 0, IPC_NOWAIT) != -1)
		{
			printf("Got message: %ld\nFrom: %d\n\n", message.type, message.sender);

			struct universalMessage reply;
			int client;

			switch (message.type)
            {
                case START:
                    client = addClient(message.sender, message.key);
                    if(client == -1)
                    {
                        printf("Too many clients already\n");
                    }
                    else
                    {
                        reply.type = START;
                        reply.number1 = clients[client]->id;
                        msgsnd(clients[client]->qid, &reply, MAX_SIZE, 0);
                    }
                    break;
                case STOP:
                    removeClient(message.sender);
                    break;
                case END:
                    endFlag = 1;
                    break;
                case MIRROR:
                    client = getClient(message.sender);
                    reply.type = clients[client]->id;
                    strcpy(reply.text, mirror(message.text));
                    msgsnd(clients[client]->qid, &reply, MAX_SIZE, 0);
                    break;
                case TIME:
                    client = getClient(message.sender);
                    reply.type = clients[client]->id;
                    strcpy(reply.text, getTime());
                    msgsnd(clients[client]->qid, &reply, MAX_SIZE, 0);
                    break;
                case ADD:
                    client = getClient(message.sender);
                    reply.type = clients[client]->id;
                    reply.number1 = message.number1 + message.number2;
                    msgsnd(clients[client]->qid, &reply, MAX_SIZE, 0);
                    break;
                case SUB:
                    client = getClient(message.sender);
                    reply.type = clients[client]->id;
                    reply.number1 = message.number1 - message.number2;
                    msgsnd(clients[client]->qid, &reply, MAX_SIZE, 0);
                    break;
                case MUL:
                    client = getClient(message.sender);
                    reply.type = clients[client]->id;
                    reply.number1 = message.number1 * message.number2;
                    msgsnd(clients[client]->qid, &reply, MAX_SIZE, 0);
                    break;
                case DIV:
                    client = getClient(message.sender);
                    reply.type = clients[client]->id;
                    reply.number1 = message.number1 / message.number2;
                    msgsnd(clients[client]->qid, &reply, MAX_SIZE, 0);
                    break;
            }
		}
		else if(endFlag == 1)
        {
            break;
        }
	}

	return EXIT_SUCCESS;
}
