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

int main(int argc, char *argv[])
{
	char buff[1024];
	char *str = "2 3";
	int a;
	int b;
	sscanf(str, "%d %d", &a, &b);
	printf("%d, %d\n", a,b);

	return EXIT_SUCCESS;
}
