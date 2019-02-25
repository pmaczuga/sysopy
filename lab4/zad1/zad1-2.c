#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

char *scriptName = "./script.sh";
pid_t childPID = 0;

void makeChild()
{
	sigset_t blockMask;
	sigemptyset(&blockMask);
	sigaddset(&blockMask, SIGINT);
	sigaddset(&blockMask, SIGTSTP);

	childPID = 0;
	childPID = fork();
	if (childPID == 0) 
	{
		sigprocmask(SIG_BLOCK, &blockMask, NULL);
		int status = execl(scriptName, scriptName, NULL);
	}
}

void intSignal()
{
	printf("\nOdebrano sygnał SIGINT\n");
	if (kill(childPID, 0) == 0) 
	{
	    kill(childPID, SIGTERM);
	}
	raise(SIGTERM);
}

void stopSignal()
{
	if (kill(childPID, 0) == 0) 
	{
		printf("\nOczekuje na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
	    kill(childPID, SIGTERM);
	}
	else
	{
		makeChild();
	}
}

int main()
{
	struct sigaction act1;
	act1.sa_handler = stopSignal;
	sigemptyset(&act1.sa_mask);
	act1.sa_flags = 0;
	sigaction(SIGTSTP, &act1, NULL); 

	struct sigaction act2;
	act2.sa_handler = intSignal;
	sigemptyset(&act2.sa_mask);
	act2.sa_flags = 0;
	sigaction(SIGINT, &act2, NULL); 

	makeChild();

	while(1)
	{
		int status;
		wait(&status);
	}
	
	return 0;
}