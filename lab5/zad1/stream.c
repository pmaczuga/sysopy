#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>

int main(int argc, char *argv[])
{

	int old_fd[2];
	int new_fd[2];
	pipe(old_fd);
	pipe(new_fd);
	if (fork() == 0) { // dziecko 1 - pisarz
	    close(old_fd[0]);
	    dup2(old_fd[1], STDOUT_FILENO);
	    execlp("ps", "ps", "au", NULL);
	} 
	else if (fork() == 0) { // dziecko 2 - czytelnik
	    close(old_fd[1]);
	    dup2(old_fd[0],STDIN_FILENO);
	    execlp("grep", "grep","pawel", NULL);
	}
	close(old_fd[1]);
	close(old_fd[0]);
	close(new_fd[1]);
	close(new_fd[0]);

	wait(NULL);
	printf("Finish 1\n");
	wait(NULL);
	printf("Finish 2\n");

	return 0;
}
