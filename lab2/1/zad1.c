#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <sys/times.h>
#include <string.h>

//------------------------------CLOCKS-----------------------------------
clock_t rTimeStart;
clock_t rTimeEnd;
struct tms oTimeStart;
struct tms oTimeEnd;

void timeStart()
{
	rTimeStart = clock();
	times(&oTimeStart);
}

void timeEnd()
{
	rTimeEnd = clock();
	times(&oTimeEnd);
	
	double realTime = ((double)(rTimeEnd - rTimeStart));
	double systemTime = ((double)(oTimeEnd.tms_stime - oTimeStart.tms_stime));
	double userTime = ((double)(oTimeEnd.tms_utime - oTimeStart.tms_utime));
	
	realTime = realTime / 1000000.0;
	systemTime = systemTime / 100.0;
	userTime = userTime / 100.0;
	
	
	printf("\tReal time: %f sec\n\tSystem time: %f sec\n\tUser time: %f sec\n", realTime, systemTime, userTime);
}

//------------------------------------------FUNCTIONS-------------------------------------
unsigned char *generateLine(int size)
{
	unsigned char *line = malloc(size);
	for(int i=0;i<size;i++)
	{
		//224
		//96
		line[i] = rand()%256;
	}
	return line;
}

int generate(char *fileName, int lines, int size)
{
	int file = open(fileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if(file < 0)
	{
		printf("Problem with openig file");
		return 1;
	}
		
	
	for(int i=0;i<lines;i++)
	{
		unsigned char *buffer = generateLine(size);
		write(file, buffer, size);
	}
	close(file);
	return 0;
}

int sysSort(char *fileName, int lines, int size)
{
	int file = open(fileName, O_RDWR);
	if(file < 0)
	{
		printf("Problem with opening file");
		return 1;
	}
		
	
	unsigned char *line1 = malloc(size);
	unsigned char *line2 = malloc(size);
	int i = 1;
	int j;
	while(i < lines)
	{
		j = i;
		
		lseek(file, j * size, SEEK_SET);
		read(file, line1, size);
		lseek(file, (j-1) * size, SEEK_SET);
		read(file, line2, size);
		
		while(j > 0 && line2[0] > line1[0])
		{	
			lseek(file, (j-1) * size, SEEK_SET);
			write(file, line1, size);
			lseek(file, j * size, SEEK_SET);
			write(file, line2, size);
			
			j--;
			
			lseek(file, j * size, SEEK_SET);
			read(file, line1, size);
			lseek(file, (j-1) * size, SEEK_SET);
			read(file, line2, size);
		}
		i++;
	}
	close(file);
	return 0;
}

int sysCopy(char *inName, char *outName, int lines, int size)
{
	int in = open(inName, O_RDONLY);
	int out = open(outName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if(in < 0 || out < 0)
	{
		printf("Problem with opening files\n");
		return 1;
	}
		
	
	for(int i=0;i<lines;i++)
	{
		unsigned char *buff = malloc(size);
		int bytesRead = read(in, buff, size);
		write(out, buff, bytesRead);
	}
	close(in);
	close(out);
	return 0;
}

int libSort(char *fileName, int lines, int size)
{
	FILE *file = fopen(fileName, "r+b");
	if(!file)
	{
		printf("Problem with opening file\n");
		return 1;
	}	
		
	unsigned char *line1 = malloc(size);
	unsigned char *line2 = malloc(size);
	int i = 1;
	int j;
	while(i < lines)
	{
		j = i;
		
		fseek(file, j * size, 0);
		fread(line1, 1, size, file);
		fseek(file, (j-1) * size, 0);
		fread(line2, 1, size, file);
		
		while(j > 0 && line2[0] > line1[0])
		{	
			fseek(file, (j-1) * size, 0);
			fwrite(line1, 1, size, file);
			fseek(file, j * size, 0);
			fwrite(line2, 1, size, file);
			
			j--;
			
			fseek(file, j * size, 0);
			fread(line1, 1, size, file);
			fseek(file, (j-1) * size, 0);
			fread(line2, 1, size, file);
		}
		i++;
	}
	fclose(file);
	return 0;
}

int libCopy(char *inName, char *outName, int lines, int size)
{
	FILE *in = fopen(inName, "rb");
	FILE *out = fopen(outName, "wb");
	if(!in || !out)
	{
		printf("Problem with opening files\n");
		return 1;
	}
	
	for(int i=0;i<lines;i++)
	{
		unsigned char *buff = malloc(size);
		int bytesRead = fread(buff, 1, size, in);
		fwrite(buff, 1, size, out);
	}
	
	fclose(in);
	fclose(out);
	return 0;
}

//--------------------------------ARGUMENTS-CHECK-------------------------------------
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
		printf("Not enough arguments\n");
		return 1;
	}
	char *op = argv[1];
	if(strcmp(op, "generate") == 0)
	{
		if(argc < 5)
		{
			printf("Not enough arguments for generate function.\n");
			printf("Example: ./program generate dane 100 512\n");
			return 1;
		}
		if(isNumber(argv[3]) != 0 || isNumber(argv[4]) != 0)
		{
			printf("3 and 4 arguments must be numbers\n");
			return 1;
		}
	}
	else if(strcmp(op, "copy") == 0)
	{
		if(argc < 7)
		{
			printf("Not enough arguments for copy function.\n");
			printf("Example: ./program copy plik1 plik2 100 512 sys\n");
			return 1;
		}
		if(isNumber(argv[4]) != 0 || isNumber(argv[5]) != 0)
		{
			printf("4 and 5 arguments must be numbers\n");
			return 1;
		}
		if(strcmp(argv[6], "sys") != 0 && strcmp(argv[6], "lib") != 0)
		{
			printf("6 argument must be lib or sys\n");
			return 1;
		}
	}
	else if(strcmp(op, "sort") == 0)
	{
		if(argc < 6)
		{
			printf("Not enough arguments for sort function.\n");
			printf("Example: ./program sort dane 100 512 lib\n");
			return 1;
		}
		if(isNumber(argv[3]) != 0 || isNumber(argv[4]) != 0)
		{
			printf("3 and 4 arguments must be numbers\n");
			return 1;
		}
		if(strcmp(argv[5], "sys") != 0 && strcmp(argv[5], "lib") != 0)
		{
			printf("5 argument must be lib or sys\n");
			return 1;
		}
	}
	else
	{
		printf("First argument must be one of the following:\n");
		printf("generate  copy  sort\n");
		return 1;
	}
		
	return 0;
}

int programStart(int argc, char *argv[])
{
	if(checkArguments(argc, argv) != 0)
		return 1;
		
	char *op = argv[1];
	if(strcmp(op, "generate") == 0)
	{
		int lines = atoi(argv[3]);
		int size = atoi(argv[4]);
		
		printf("Generate\tRecords: %d\tSize: %d\n", lines, size);
		timeStart();
		generate(argv[2], lines, size);
		timeEnd();
	}
	else if(strcmp(op, "copy") == 0)
	{
		int lines = atoi(argv[4]);
		int size = atoi(argv[5]);
		
		if(strcmp(argv[6], "sys") == 0)
		{
			printf("System copy\tRecords: %d\tSize: %d\n", lines, size);
			timeStart();
			sysCopy(argv[2], argv[3], lines, size);
			timeEnd();
		}
		else
		{
			printf("Library copy\tRecords: %d\tSize: %d\n", lines, size);
			timeStart();
			libCopy(argv[2], argv[3], lines, size);
			timeEnd();
		}
	}
	else if(strcmp(op, "sort") == 0)
	{
		int lines = atoi(argv[3]);
		int size = atoi(argv[4]);
		
		if(strcmp(argv[5], "sys") == 0)
		{
			printf("System sort\tRecords: %d\tSize: %d\n", lines, size);
			timeStart();
			sysSort(argv[2], lines, size);
			timeEnd();
		}
		else
		{
			printf("Library sort\tRecords: %d\tSize: %d\n", lines, size);
			timeStart();
			libSort(argv[2], lines, size);
			timeEnd();		
		}
	}
	return 0;
}

//-----------------------------------------MAIN--------------------------------
int main(int argc, char *argv[])
{
	srand(time(NULL));
	
	programStart(argc, argv);
	
	return 0;
}
