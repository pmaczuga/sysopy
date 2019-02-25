#include <stdlib.h>

int main(int argc, char *argv[])
{
	if(argc < 2) return 1;

	int size = atoi(argv[1]);

	char *array = malloc(size);

	int j = 0;
	for(int i = 0;i<size;i++)
	{
		array[i] = j;
		j = (j+1)%255;
	}

	return 0;
}