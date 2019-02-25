#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <dlfcn.h>

static int numberOfRandChars = 10000 - 1;

//clocks
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
	
	printf("\tReal time: %f\n\tSystem time: %f\n\tUser time: %f\n", realTime, systemTime, userTime);
}

char **randChars(int number, int length)
{
	char **randChars = malloc(number * sizeof(char **));
	int i = 0;
	while(i < number)
	{
		int j = 0;
		randChars[i] = malloc(length);
		while(j < length)
		{
			randChars[i][j] = rand()%74 + 48;
			j++;
		}
		i++;
	}
	return randChars;
}

int isDynamic(int argc, char* argv[])
{
	if(strcmp(argv[1], "-d") == 0)
		return 0;
	return 1;
}


void testStatic(int argc, char* argv[])
{	
	void *handle = dlopen("./libArray.so", RTLD_LAZY);
	if(!handle)
	{
		printf("Error. Can't find handle\n'");
		return;
	}

	void (*sInsertBlock)(char* chars, int sizeOfChars);
	sInsertBlock = dlsym(handle, "sInsetBlock");
	
	void (*sDeleteBlock)(int pos);
	sDeleteBlock = dlsym(handle, "sDeleteBlock");

	int (*sFindSimilarSum)(int pos);
	sFindSimilarSum = dlsym(handle, "sFindSimilarSum");
	
	int i = 2;
	while(i < argc)
	{
		if(strcmp(argv[i], "search_element") == 0)
		{
			i++;
			int arg = atoi(argv[i]);
			printf("search_element %d\n", arg);
			
			timeStart();
			int el = sFindSimilarSum(arg);
			timeEnd();
			
			printf("\tFound: %d\n", el);
		}
		else if(strcmp(argv[i], "remove") == 0)
		{
			i++;
			int arg = atoi(argv[i]);
			printf("remove %d\n", arg);
			
			timeStart();
			int j = 0;
			while(j<arg)
			{
				sDeleteBlock(j);
				j++;
			}
			timeEnd();
		}
		else if(strcmp(argv[i], "add") == 0)
		{
			i++;
			int arg = atoi(argv[i]);
			printf("add %d\n", arg);
			
			char **randC = randChars(arg, numberOfRandChars);
			
			timeStart();
			int j = 0;
			while(j<arg)
			{
				sInsertBlock(randC[j], numberOfRandChars);
				j++;
			}
			timeEnd();
		}
		else if(strcmp(argv[i], "remove_and_add") == 0)
		{
			i++;
			int arg = atoi(argv[i]);
			printf("remove_and_add %d\n", arg);
			
			char **randC = randChars(arg, numberOfRandChars);
			
			timeStart();
			int j = 0;
			while(j<arg)
			{
				sDeleteBlock(j);
				sInsertBlock(randC[j], numberOfRandChars);
				j++;
			}
			timeEnd();
		}
		i++;
	}
	dlclose(handle);
}


void testDynamic(int argc, char* argv[])
{
	void *handle = dlopen("./libArray.so", RTLD_LAZY);
	if(!handle)
	{
		printf("Error. Can't find handle\n'");
		return;
	}
	
	char **(*dMakeArray)(int size);
	dMakeArray = dlsym(handle, "dMakeArray");
	
	char **(*dDeleteArray)(char **array, int size);
	dDeleteArray = dlsym(handle, "dDeleteArray");
	
	void (*dInsertBlock)(char **array, int size, char* chars, int sizeOfChars);
	dInsertBlock = dlsym(handle, "dInsertBlock");
	
	void (*dDeleteBlock)(char **array, int size, int pos);
	dDeleteBlock = dlsym(handle, "dDeleteBlock");
	
	int (*dFindSimilarSum)(char **array, int size, int pos);
	dFindSimilarSum = dlsym(handle, "dFindSimilarSum");
	
	
	char **array;
	int size;
		
	int i = 2;
	while(i < argc)
	{
		if(strcmp(argv[i], "create_array") == 0)
		{
			i++;
			int arg = atoi(argv[i]);
			size = arg;
			printf("create_array %d\n", arg);
			
			timeStart();
			array = dMakeArray(arg);
			timeEnd();
		}
		else if(strcmp(argv[i], "search_element") == 0)
		{
			i++;
			int arg = atoi(argv[i]);
			printf("search_element %d\n", arg);
			
			timeStart();
			int el = dFindSimilarSum(array, size, arg);
			timeEnd();
			
			printf("\tFound: %d\n", el);
		}
		else if(strcmp(argv[i], "remove") == 0)
		{
			i++;
			int arg = atoi(argv[i]);
			printf("remove %d\n", arg);
			
			timeStart();
			int j = 0;
			while(j<arg)
			{
				dDeleteBlock(array, size, j);
				j++;
			}
			timeEnd();
		}
		else if(strcmp(argv[i], "add") == 0)
		{
			i++;
			int arg = atoi(argv[i]);
			printf("add %d\n", arg);
			
			char **randC = randChars(arg, numberOfRandChars);
			
			timeStart();
			int j = 0;
			while(j<arg)
			{
				dInsertBlock(array, size, randC[j], numberOfRandChars);
				j++;
			}
			timeEnd();
		}
		else if(strcmp(argv[i], "remove_and_add") == 0)
		{
			i++;
			int arg = atoi(argv[i]);
			printf("remove_and_add %d\n", arg);
			
			char **randC = randChars(arg, numberOfRandChars);
			
			timeStart();
			int j = 0;
			while(j<arg)
			{
				dDeleteBlock(array, size, j);
				dInsertBlock(array, size, randC[j], numberOfRandChars);
				j++;
			}
			timeEnd();
		}
		i++;
	}
	dlclose(handle);
}

void printArguments(int argc, char* argv[])
{
	int i = 0;
	while(i < argc)
	{
		printf("%d: %s\n",i , argv[i]);
		i++;
	}
}

//------------------------------MAIN-------------------------------------
int main(int argc, char* argv[])
{	
	srand(time(NULL));
	
	int dynamic;	// 0 - dymaniczna alokacja, 1 - statyczna alokacja
	dynamic = isDynamic(argc, argv);
	
	
	if(dynamic != 0)
	{
		printf("Static array test\n");
		printf("Of arguments: \n");
		printArguments(argc, argv);
		printf("\n");
		
		testStatic(argc, argv);
	}
	else
	{
		printf("Dynamic array test\n");
		printf("Of arguments: \n");
		printArguments(argc, argv);
		printf("\n");
		
		testDynamic(argc, argv);
	}
	
	return 0;
}
