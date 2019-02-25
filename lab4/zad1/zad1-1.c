#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

int sigtstpAction = 0;

void intSignal()
{
	printf("\nOdebrano sygnał SIGINT\n");
	raise(SIGQUIT);
}

void stopSignal()
{
	sigtstpAction = (sigtstpAction + 1)%2;

	if(sigtstpAction == 1)
	{
		printf("\nOczekuje na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
		pause();
	}
}

int main()
{
	printf("Begin\n");

	struct sigaction act;
	act.sa_handler = stopSignal;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;
	sigaction(SIGTSTP, &act, NULL);
	signal(SIGINT, intSignal);

	time_t rawtime;
	struct tm * timeinfo;

	while(1)
	{
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		printf ("%s", asctime (timeinfo) );
		sleep(1);
	}
	
	printf("End\n");

	return 0;
}