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

struct universalMessage
{
	long type;
	pid_t sender;
	key_t key;
	int number1;
	int number2;
	char text[MAX_SIZE - 2 * sizeof(int) - sizeof(pid_t) - sizeof(key_t)];
};

int myQid;
int serverQid;
key_t myKey;
key_t serverKey;
unsigned long id;

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
	struct universalMessage message;
	message.type = STOP;
	message.sender = getpid();
	msgsnd(serverQid, &message, MAX_SIZE, 0);
	msgctl(myQid, IPC_RMID, NULL);
	if(fromFileFlag == 1)
    {
        fclose(stream);
    }
}

void handleINT()
{
	exit(EXIT_SUCCESS);
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

int fillMessage(struct universalMessage *message, int size, char **line)
{
    if(size < 1)
        return -1;

    message->sender = getpid();

    if(strcmp(line[0], "TIME") == 0)
    {
        message->type = TIME;
    }
    else if(strcmp(line[0], "MIRROR") == 0)
    {
        message->type = MIRROR;
        if(size < 2)
            return -1;
        strcpy(message->text, line[1]);
    }
    else if(strcmp(line[0], "END") == 0)
    {
        message->type = END;
    }
    else
    {
        if(strcmp(line[0], "ADD") == 0)
            message->type = ADD;
        else if(strcmp(line[0], "SUB") == 0)
            message->type = SUB;
        else if(strcmp(line[0], "MUL") == 0)
            message->type = MUL;
        else if(strcmp(line[0], "DIV") == 0)
            message->type = DIV;
        else
            return -1;
        if(size < 3)
            return -1;
        if(isNumber(line[1]) != 0 || isNumber(line[2]) != 0)
            return -1;
        message->number1 = atoi(line[1]);
        message->number2 = atoi(line[2]);
    }

    return 0;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	atexit(cleanUp);
	signal(SIGINT, handleINT);

	do
	{
		myKey = ftok(KEY_PATH, CLIENT_PROJECT_ID);
		myQid = msgget(myKey, IPC_CREAT | IPC_EXCL | 0660);
	}while(myQid < 0);

	serverKey = ftok(KEY_PATH, PROJECT_ID);
	if(serverKey == -1)
	{
		perror("Error in ftok() - server key");
		return EXIT_FAILURE;
	}

	serverQid = msgget(serverKey, 0);
	if(serverQid == -1)
	{
		perror("Error in msgget() - server queue");
		return EXIT_FAILURE;
	}

	printf("Client set\n");
	printf("Server key: %lu\n", (unsigned long)serverKey);
	printf("Server queue ID: %d\n", serverQid);
	printf("Client key: %lu\n", (unsigned long)myKey);
	printf("Client queue ID: %d\n", myQid);

	struct universalMessage message;

	message.type = START;
	message.sender = getpid();
	message.key = myKey;
	if(msgsnd(serverQid, &message, MAX_SIZE, 0) == -1)
	{
		perror("Error sending message");
		return EXIT_FAILURE;
	}
	if(msgrcv(myQid, &message, MAX_SIZE, START, 0) == -1)
	{
		perror("Didn't receive ID");
		return EXIT_FAILURE;
	}
	id = (unsigned long)message.number1;

	printf("Client name: %d\n", getpid());
	printf("Client ID: %lu\n\n", id);

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
		char line[sizeof(message.text)];
		char *endOfFile = fgets(line, sizeof(message.text), stream);
		if(endOfFile == NULL)
            break;

		char **parsedLine;
		int size = parseLine(line, &parsedLine);
		int status = fillMessage(&message, size, parsedLine);
		long type = message.type;
		if(status == -1)
        {
            printf("Wrong input line\n");
            continue;
        }

        if(msgsnd(serverQid, &message, MAX_SIZE, 0) == -1)
        {
            perror("Error sending message");
            return EXIT_FAILURE;
        }

        if(type == END)
        {
            break;
        }
        if(msgrcv(myQid, &message, MAX_SIZE, id, 0) == -1)
        {
            perror("Error receiving message");
            return EXIT_FAILURE;
        }
		if(type == MIRROR || type == TIME)
        {
            printf("Answer: %s\n\n", message.text);
        }
        else
        {
            printf("Answer: %d\n\n", message.number1);
        }
	}

	return EXIT_SUCCESS;
}
