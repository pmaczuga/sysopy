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
pid_t childPID = 0;

int signalsSent = 0;
int signalsGot = 0;

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

// check arguments
int checkArguments(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("Two argument required\n");
		return -1;
	}

	if(isNumber(argv[1]) != 0)
	{
		printf("First argument must be number\n");
		return -1;
	}

	if(strlen(argv[2]) != 2)
	{
		printf("Second argument has to be \"-1\", \"-2\" or \"-3\"\n");
		return -1;
	}

	if(argv[2][0] != '-')
		return -1;

	if(argv[2][1] == '1')
		return 1;
	else if(argv[2][1] == '2')
		return 2;
	else if(argv[2][1] == '3')
		return 3;
	else
	{
		printf("Second argument has to be \"-1\", \"-2\" or \"-3\"\n");
		return -1;
	}
}

void makeChild()
{
	childPID = 0;
	childPID = fork();
	if(childPID == 0)
	{
		execl(childProgram, childProgram, NULL);
	}
}

void intHandler()
{
	printf("Signas sent to child: %d\n", signalsSent);
	kill(childPID, SIGUSR2);
	printf("Signals got from child: %d\n", signalsGot);
	raise(SIGQUIT);
}

void sig1Handler()
{
	signalsGot++;
}

// SIG1 - signal send to child
// SIG2 - signal to end child
// waitForChild = 0 - wait for child to response
// waitForChild = 1 - don't
void sender(int L, int SIG1, int SIG2, int waitForChild)
{
	// SIGINT
	struct sigaction intAct;
	intAct.sa_handler = intHandler;
	sigemptyset(&intAct.sa_mask);
	intAct.sa_flags = 0;
	sigaction(SIGINT, &intAct, NULL);

	// SIGUSR1
	struct sigaction sig1Act;
	sig1Act.sa_handler = sig1Handler;
	sigemptyset(&sig1Act.sa_mask);
	sig1Act.sa_flags = 0;
	sigaction(SIG1, &sig1Act, NULL);

	// !!!
	sleep(1);

	// sending L signals to child
	int i = 0;
	while(i < L)
	{
		if(kill(childPID, SIG1) != -1)
		{
			signalsSent++;
		}
		i++;

		// works for ~500 signals
		//if(waitForChild == 0) pause();

		// works for ~200000 signals
		if(waitForChild == 0 && signalsSent != signalsGot) pause();
	}


	printf("Signas sent to child: %d\n", signalsSent);
	if(waitForChild == 1) sleep(1);
	kill(childPID, SIG2);
	sleep(1);
	printf("Signals got from child: %d\n", signalsGot);
}

void start(int L, int type)
{
	makeChild();

	if(type == 1)
	{
		printf("Type 1\n");
		printf("Sending SIGUSR1, SIGUSR2 using kill()\n\n");
		sender(L, SIGUSR1, SIGUSR2, 1);
	}
	if(type == 2)
	{
		printf("Type 2\n");
		printf("Sending SIGUSR1, SIGUSR2 using kill()\n");
		printf("Parent waits for child to response\n\n");
		sender(L, SIGUSR1, SIGUSR2, 0);
	}
	if(type == 3)
	{
		printf("Type 3\n");
		printf("Sending SIGRTMIN, SIGRTMIN+1 using kill()\n\n");
		sender(L, SIGRTMIN, SIGRTMIN+1, 1);
	}
}

int main(int argc, char *argv[])
{
	int type = checkArguments(argc, argv);
	if(type < 0)
		return 1;

	start(atoi(argv[1]), type);

	return 0;
}