#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

char childProgram[20] = "./child";

// print flags
// 0 - don't print
// 1 - print
int createFlag = 0;			// print child created
int requestsFlag = 0;		// print requests received
int sendFlag = 0;			// print permissions sent
int realTimeFlag = 0;		// print REAL TIME signals received
int processEndFlag = 0;		// print finiched child processes


struct betterArray
{
	pid_t *array;
	int size;
	int lastIndex;
};

// array that stores all reqests received
struct betterArray *reqestsGot;

// how many children still exists
int childLeft;

// init betterArray structure
struct betterArray *initBetterArray(int size)
{
	struct betterArray *array = malloc(sizeof(struct betterArray));
	array->size = size;
	array->lastIndex = 0;
	array->array = malloc(sizeof(int) * size);

	return array;
}

// add element to betterArray
int addToBetterArray(struct betterArray *array, pid_t el)
{
	if(array->lastIndex >= array->size)
	{
		return 1;
	}

	array->array[array->lastIndex] = el;
	array->lastIndex++;

	return 0;
}

// is argument (string) a number
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

// check arguments and chenge print flags
int checkArguments(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("Not enough arguments (at lest 2)\n");
		return 1;
	}
	if(isNumber(argv[1]) != 0 || isNumber(argv[2]) != 0)
	{
		printf("First and second arguments must be numbers\n");
		return 1;
	}
	if(argc == 3)
		return 0;

	if(argv[3][0] != '-')
		return 0;

	int i = 1;
	while(i < strlen(argv[3]))
	{
		if(argv[3][i] == '1')
			createFlag = 1;
		if(argv[3][i] == '2')
			requestsFlag = 1;
		if(argv[3][i] == '3')
			sendFlag = 1;
		if(argv[3][i] == '4')
			realTimeFlag = 1;
		if(argv[3][i] == '5')
			processEndFlag = 1;
		i++;
	}
	return 0;
}

// send back SIGUSR1 to prcess that sent signal
void acceptRequest(int sig, siginfo_t* info, void* ucontext)
{
	if(kill(info->si_pid, SIGUSR1) != -1)
	{
		if(sendFlag == 1)
		{
			printf("Sent permisson to: %d\n", info->si_pid);
		}
	}
	else
	{
		printf("Error sending permisson to :%d\n", info->si_pid);
	}
}

// add pid of process that send signal to global struct "reqestsGot"
void addRequestToArray(int sig, siginfo_t* info, void* ucontext)
{
	if(requestsFlag == 1)
	{
		printf("Got request: %d from: %d\n", sig, info->si_pid);
	}
	if(reqestsGot->lastIndex < reqestsGot->size)
	{
		addToBetterArray(reqestsGot, info->si_pid);
	}
	else
	{
		acceptRequest(sig, info, ucontext);
	}
}

// REAL TIME signals handler
// just prints
void realTimeHander(int sig, siginfo_t* info, void* ucontext)
{
	if(realTimeFlag == 1)
	{
		printf("Got RT signal: %d from: %d\n", sig - SIGRTMIN, info->si_pid);
	}
}

void intHandler()
{
	kill(-getpid(), SIGQUIT);
}

// SIGCHLD handelr - when child ends
void childEndHandler(int sig, siginfo_t* info, void* ucontext)
{
	int status;
	int pid;
	while(1)
	{
		pid = waitpid(-1, &status, WNOHANG);
		if(pid > 0)
		{
			if(processEndFlag == 1)
			{
				printf("Child finished: %d with sleep time: %d\n", pid, WEXITSTATUS(status));
				childLeft--;
			}
		}
		else if(pid == 0)
		{
			break;
		}
		else
		{
			break;
		}
	}
}

// parent behavoiur
void start(int N, int K)
{
	// init array that stores requests received
	reqestsGot = initBetterArray(K);

	//SIGINT
	struct sigaction intAct;
	intAct.sa_handler = intHandler;
	sigemptyset(&intAct.sa_mask);
	intAct.sa_flags = 0;
	sigaction(SIGINT, &intAct, NULL);

	// SIGUSR1 - wait for K requests, then send permissions right away
	struct sigaction waitAct;
	waitAct.sa_sigaction = addRequestToArray;
	sigemptyset(&waitAct.sa_mask);
	waitAct.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &waitAct, NULL);

	// SIGCHLD
	struct sigaction endAct;
	endAct.sa_sigaction = childEndHandler;
	sigemptyset(&endAct.sa_mask);
	endAct.sa_flags = SA_SIGINFO;
	sigaction(SIGCHLD, &endAct, NULL);

	// REALT TIME signals
	struct sigaction realAct;
	realAct.sa_sigaction = realTimeHander;
	sigemptyset(&realAct.sa_mask);
	realAct.sa_flags = SA_SIGINFO;
	int i = 0;
	while(i < 32)
	{
		sigaction(SIGRTMIN+i, &realAct, NULL);
		i++;
	}

	// create N child processes
	i = 0;
	while(i < N)
	{
		pid_t childPID = 0;
		childPID = fork();

		if(childPID == 0)
		{
			execl(childProgram, childProgram, NULL);
		}
		else if(createFlag == 1)
		{
			printf("Created child of PID: %d\n", childPID);
		}
		i++;
	}
	childLeft = N;

	// wait for K requests
	// requestsGot->size == K
	while(reqestsGot->lastIndex < reqestsGot->size)
	{
		pause();
	}

	// send permissons to all requests in array requestsGot
	i = 0;
	while(i < reqestsGot->lastIndex)
	{
		if(kill(reqestsGot->array[i], SIGUSR1) != -1)
		{
			if(sendFlag == 1)
			{
				printf("Sent permisson to: %d\n", reqestsGot->array[i]);
			}
		}
		else
		{
			printf("Error sending permisson to :%d\n", reqestsGot->array[i]);
		}
		i++;
	}


	// wait for all childlren to finish
	while(childLeft > 0)
	{
		pause();
	}
}

void printMessages()
{
	printf("print child created: %d\n", createFlag);
	printf("print requests received: %d\n", createFlag);
	printf("print permissions sent %d\n", createFlag);
	printf("print REAL TIME signals received: %d\n", createFlag);
	printf("print finiched child processes: %d\n", createFlag);
	printf("\n\n");
}

int main(int argc, char *argv[])
{
	//srand(time(NULL));

	if(checkArguments(argc, argv) != 0)
		return 1;

	printMessages();

	start(atoi(argv[1]), atoi(argv[2]));


	printf("END\n"); 

	return 0;
}