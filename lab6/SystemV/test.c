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

int countWords(char *line)
{
	int count = 0;
	int lastWhite = 1;
	int i = 0;
	while(line[i] != '\0')
	{
		if(lastWhite == 1 && line[i] != ' ')
		{
			count++;
			lastWhite = 0;
		}
		else if(line[i] == ' ')
			lastWhite = 1;
		i++;
	}
	return count;
}

int finishWord(char *line, int i)
{
	while(line[i] != '\0' && line[i] != '\n' && line[i] != ' ')
	{
		i++;
	}
	return i;
}

void copy(char *to, char *from, int i, int j)
{
	int toIndex = 0;
	while(i < j)
	{
		to[toIndex] = from[i];
		i++;
		toIndex++;
	}
	to[toIndex] = '\0';
}

int parseLine(char *line, char ***parsedPointer)
{
	char **parsed = *parsedPointer;
	int words = countWords(line);
	parsed = malloc(sizeof(char *) * words);
	int wordIndex = 0;
	int i = 0;
	while(line[i] != '\0')
	{
		if(line[i] == ' ' || line[i] == '\n')
		{
			i++;
		}
		else
		{
			int j = finishWord(line, i);
			parsed[wordIndex] = malloc(j - i + 1);
			copy(parsed[wordIndex], line, i, j);
			i = j;
			wordIndex++;
		}
	}
	*parsedPointer = parsed;
	return words;
}

int main(int argc, char *argv[])
{
	char str[100];
	fgets(str, 100, stdin);
	char **parsed;
	int size;
	size = parseLine(str, &parsed);
	int i = 0;

	while(i < size)
	{
		printf("%s\n", parsed[i]);
		i++;
	}
	printf("END\n\n");
	
	return EXIT_SUCCESS;
}
