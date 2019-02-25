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
	if(argc < 2)
	{
		printf("Not enough arguments. Three arguments required\n");
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

int getEndOfCommand(char **line, int start)
{
	int i = start;
	while(line[i] != NULL && strcmp(line[i], "|") != 0)
	{
		i++;
	}
	return i;
}

char **getCommandLine(char **line, int start, int end)
{
	char **result = malloc(sizeof(char *) * (end - start));
	int i = start;
	int j = 0;
	while(i < end)
	{
		result[j] = malloc(strlen(line[i]) + 1);
		result[j] = strcpy(result[j], line[i]);
		i++;
		j++;
	}
	result[j] = NULL;
	return result;
}

int countCommands(char **line)
{
	int result = 0;
	int i = 0;
	while(line[i] != NULL)
	{
		if(strcmp(line[i], "|") == 0)
			result++;
		i++;
	}
	return result + 1;
}

int startProgram(FILE *file)
{
	char buff[512];

	while(fgets(buff, sizeof(buff), file) != NULL)
	{
		char **arguments = parseLine(buff);

		int commandsNumber = countCommands(arguments);
		int **fd;
		fd = malloc(sizeof(int *) * commandsNumber);

		int i = 0;
		while(i < commandsNumber)
		{
			fd[i] = malloc(sizeof(int) * 2);
			pipe(fd[i]);
			i++;
		}

		int argumentI = 0;
		i = 0;
		while(i < commandsNumber)
		{
			int argumentEnd = getEndOfCommand(arguments, argumentI);
			char **command = getCommandLine(arguments, argumentI, argumentEnd);
			printf("Command: \n");
			printArguments(command);
			printf("\n");


			argumentI = argumentEnd + 1;

			pid_t child  = 0;
			child = fork();
			if(child == 0)
			{
				int j = 0;
				while(j < commandsNumber)
				{
					if(j != i && j != (i+1))
					{
						close(fd[j][0]);
						close(fd[j][1]);
					}
					j++;
				}


				close(fd[i][1]);
				if(i == 0)
					close(fd[i][0]);
				else
					dup2(fd[i][0], STDIN_FILENO);

				if(i != commandsNumber - 1)
				{
					close(fd[i+1][0]);
					dup2(fd[i+1][1], STDOUT_FILENO);
				}

				int status = execvp(command[0], command);
				exit(status);
			}

			i++;
		}

		i = 0;
		while(i < commandsNumber)
		{
			close(fd[i][1]);
			close(fd[i][0]);
			i++;
		}

		while(1)
		{
			int status;
			int child_pid;
			if(child_pid = wait(&status) == -1)
				break;
			//else 
				//printf("Child: %d ended with status: %d\n", child_pid, status);
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
	

	startProgram(file);
	
	fclose(file);

	return 0;
}
