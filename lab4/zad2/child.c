#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

// 0 - no permission
// 1 - permission received
int permission = 0;

void gotPermissionHandler()
{
	permission = 1;
}

void childBehaviour()
{
	srand(time(NULL) + getpid());

	// Block all signals but SIGUSR1 and SIGQUIT
	sigset_t blockMask;
	sigfillset(&blockMask);
	sigdelset(&blockMask, SIGUSR1);
	sigdelset(&blockMask, SIGQUIT);
	sigprocmask(SIG_BLOCK, &blockMask, NULL);

	// SIGUSR1 - permission
	struct sigaction act;
	act.sa_handler = gotPermissionHandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGUSR1, &act, NULL); 


	union sigval value;
	int sleepTime = rand()%10+1;
	sleep(sleepTime);
	
	while(permission == 0)
	{
		sigqueue(getppid(), SIGUSR1, value);
		sleep(10);
	}
	
	int n = rand()%32;
	sigqueue(getppid(), SIGRTMIN+n, value);
	exit(sleepTime);
}

int main()
{
	childBehaviour();
}
