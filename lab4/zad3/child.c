#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

int signalsGot = 0;
int signalsSent = 0;

void pass(){}

void sig1Handler(int sig)
{
	signalsGot++;
	if(kill(getppid(), sig) != -1)
	{
		signalsSent++;
	}
	
}

void sig2Handler()
{
	printf("Signals got from parent: %d\n", signalsGot);
	exit(0);
}

void childBehaviour()
{
	// Block all signals but SIGUSR1, SIGUSR2, SIGRTMIN, SIGRTMIN+1
	sigset_t blockMask;
	sigfillset(&blockMask);
	sigdelset(&blockMask, SIGUSR1);
	sigdelset(&blockMask, SIGUSR2);
	sigdelset(&blockMask, SIGQUIT);
	sigdelset(&blockMask, SIGRTMIN);
	sigdelset(&blockMask, SIGRTMIN+1);
	sigprocmask(SIG_BLOCK, &blockMask, NULL);

	struct sigaction sig1Act;
	sig1Act.sa_handler = sig1Handler;
	sigemptyset(&sig1Act.sa_mask);
	sig1Act.sa_flags = 0;
	sigaction(SIGUSR1, &sig1Act, NULL);
	sigaction(SIGRTMIN, &sig1Act, NULL);

	struct sigaction sig2Act;
	sig2Act.sa_handler = sig2Handler;
	sigemptyset(&sig2Act.sa_mask);
	sig2Act.sa_flags = 0;
	sigaction(SIGUSR2, &sig2Act, NULL);
	sigaction(SIGRTMIN+1, &sig2Act, NULL);

	while(1)
	{
		pause();
	}
}

int main(int argc, char *argv[])
{
	childBehaviour();

	return 0;
}