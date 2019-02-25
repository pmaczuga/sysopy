#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX_SIZE 3840

double filter[MAX_SIZE][MAX_SIZE];
int size;
const int outputLineSize = 12;

void generateFilter()
{
    srand48(time(NULL));
    double sum = 0.0;
    int i = 0;
    while(i < size)
    {
        int j = 0;
        while(j < size)
        {
            double num = drand48();
            sum += num;
            filter[i][j] = num;
            j++;
        }
        i++;
    }

    i = 0;
    while(i < size)
    {
        int j = 0;
        while(j < size)
        {
            filter[i][j] /= sum;

            j++;
        }
        i++;
    }
}

int writeFilter(FILE *output)
{
    char buf[32];

    sprintf(buf, "%d \n", size);
    fputs(buf, output);

    int num = 0;

    int i = 0;
    while(i < size)
    {
        int j = 0;
        while(j < size)
        {
            sprintf(buf, "%f ", filter[i][j]);
            fputs(buf, output);
            num++;
            if(num == outputLineSize)
            {
                num = 0;
                fputc('\n', output);
            }
            j++;
        }
        i++;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        fprintf(stderr, "ERROR: Not enough arguments\n");
        return EXIT_FAILURE;
    }
    size = atoi(argv[1]);

    FILE *output = fopen(argv[2], "w");
    if(output == NULL)
    {
        perror("ERROR: Problem opening file");
        exit(EXIT_FAILURE);
    }

    generateFilter();
    writeFilter(output);

    //printf("Random filter created with size: %d\n", size);

    return EXIT_SUCCESS;
}
