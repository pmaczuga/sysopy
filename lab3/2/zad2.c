#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>

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

int checkArguments(int argc, char *argv[])
{
	if(argc < 4)
	{
		printf("Not enough arguments. Three arguments required\n");
		return 1;
	}
	if(isNumber(argv[2]) != 0 || isNumber(argv[3]) != 0)
	{
		printf("Second and thrid arguments must be numbers\n");
		return 1;
	}
	return 0;
}

char **addToList(int argc, char *argv[], char *newArg)
{
	char **result = (char **)malloc(sizeof(char *) * (argc + 1));
	int i = 0;
	while(i < argc)
	{
		result[i] = argv[i];
		i++;
	}

	char *tmp = NULL;
	if(newArg != NULL)
	{
		tmp = (char *)malloc(strlen(newArg)+1);
		strcpy(tmp, newArg);
	}

	free(argv);
	result[i] = tmp;
	return result;
}

char **parseLine(char *line)
{
	int argc = 0;
	char **result = NULL;

	char tmp[32];
	int lastWhite = 0;	//0 - last char was white    1 - last char wasn't white
	int i = 0;			//index of line
	int argI = 0;		//index of one argument
	while(line[i] != '\0' && line[i] != '\n')
	{
		if(lastWhite == 0)
		{
			if(line[i] != ' ' && line[i] != '\t')
			{
				argI = 0;
				tmp[argI] = line[i];
				argI++;
				argc++;
				lastWhite = 1;
			}
		}
		else
		{
			if(line[i] == ' ' || line[i] == '\t')
			{
				tmp[argI] = '\0';
				result = addToList(argc-1, result, tmp);
				lastWhite = 0;
			}
			else
			{
				tmp[argI] = line[i];
				argI++;
			}
		}
		i++;
	}
	if(lastWhite != 0)
	{
		tmp[argI] = '\0';
		result = addToList(argc-1, result, tmp);
	}
	result = addToList(argc, result, NULL);
	return result;
}

void printArguments(char **args)
{
	int i = 0;
	while(args[i] != NULL)
	{
		printf("%s ", args[i]);
		i++;
	}
	printf("\n");
}

void printProcessUsage(struct rusage *usage)
{
	printf("Usage of process: \n");
	printf("User time: %ld.%ld sec\n", usage->ru_utime.tv_sec, usage->ru_utime.tv_usec);
	printf("System time: %ld.%ld sec\n", usage->ru_stime.tv_sec, usage->ru_stime.tv_usec);
}

int startProgram(FILE *file, int maxTime, int maxMemory)
{
	maxMemory = maxMemory * 1024 * 1024;
	struct rlimit timeLimit;
	timeLimit.rlim_cur = maxTime;
	timeLimit.rlim_max = maxTime;
	struct rlimit memoryLimit;
	memoryLimit.rlim_cur = maxMemory;
	memoryLimit.rlim_max = maxMemory;

	char buff[512];

	while(fgets(buff, sizeof(buff), file) != NULL)
	{
		char **arguments = parseLine(buff);
		if(arguments[0] !=  NULL)
		{
			pid_t child  = 0;
			child = fork();
			if(child == 0)
			{
				setrlimit(RLIMIT_AS, &memoryLimit);
				setrlimit(RLIMIT_CPU, &timeLimit);
				printf("\n\nExecuting line: ");
				printArguments(arguments);
				printf("Process PID: %d\n", getpid());
				printf("\n");
				int status = execvp(arguments[0], arguments);
				exit(status);
			}
			int status;
			wait(&status);
			printf("\n");
			if(status != 0)
			{
				printf("Problem with executing line: ");
				printArguments(arguments);
				return 1;
			}
			struct rusage usage;
			int rusageStatus =  getrusage(RUSAGE_CHILDREN, &usage);
			if(rusageStatus != 0)
			{
				printf("Problem with getrusage function\n");
			}
			printProcessUsage(&usage);
		}
	}
}


int main(int argc, char *argv[])
{
	if(checkArguments(argc, argv) != 0)
		return 1;
	
	FILE *file = fopen(argv[1], "r");
	if(file == NULL)
	{
		printf("Problem opening file\n");
		return 1;
	}
	

	startProgram(file, atoi(argv[2]), atoi(argv[3]));
	
	fclose(file);

	return 0;
}
