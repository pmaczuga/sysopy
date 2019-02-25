#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>

#define MAX_BUF_SIZE 4096
#define MAX_SIZE 3840

const int outputLineSize = 12;

int picture[MAX_SIZE][MAX_SIZE];
double filter[MAX_SIZE][MAX_SIZE];
int result[MAX_SIZE][MAX_SIZE];

int height;
int width;
int filterSize;

char errorBuffer[MAX_BUF_SIZE];

struct timespec start;
struct timespec end;
struct timespec res;

void timeStart()
{
    clock_gettime(CLOCK_MONOTONIC, &start);
}

void timeEnd()
{
    clock_gettime(CLOCK_MONOTONIC, &end);
    if(end.tv_nsec - start.tv_nsec < 0)
    {
        res.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
        res.tv_sec = end.tv_sec - start.tv_sec - 1;
    }
    else
    {
        res.tv_nsec = end.tv_nsec - start.tv_nsec;
        res.tv_sec = end.tv_sec - start.tv_sec;
    }
}

int isWhiteChar(char c)
{
    if(c == ' ')
        return 1;
    if(c == '\t')
        return 1;
    if(c == '\n')
        return 1;
    if(c == '\v')
        return 1;
    if(c == '\f')
        return 1;
    if(c == '\r')
        return 1;
    return 0;
}

int readWord(char *from, char *to, int startAt)
{
    if(from[startAt] == '\0')
    {
        return -1;
    }

    while(from[startAt] != '\0' && isWhiteChar(from[startAt]) == 1)
        startAt++;

    int i = 0;
    while(from[startAt] != '\0' && isWhiteChar(from[startAt]) == 0)
    {
        to[i] = from[startAt];
        i++;
        startAt++;
    }
    to[i] = '\0';

    while(from[startAt] != '\0' && isWhiteChar(from[startAt]) == 1)
        startAt++;

    return startAt;
}

int readPicture(FILE *inPicture)
{
    char buf[MAX_BUF_SIZE];
    char word[64];

    int startParams = 0;
    while(startParams < 3)
    {
        fgets(buf, MAX_BUF_SIZE, inPicture);
        if(buf[0] == '#')
            continue;
        if(startParams == 1)
        {
            int i = 0;
            i = readWord(buf, word, i);
            width = atoi(word);
            i = readWord(buf, word, i);
            height = atoi(word);
        }
        if(startParams == 2)
        {;
            readWord(buf, word, 0);
            if(atoi(word) > 255)
            {
                sprintf(errorBuffer, "Pixel colors can only be from 0 to 255");
                return -1;
            }
        }
        startParams++;
    }

    if(height > MAX_SIZE || width > MAX_SIZE)
    {
        sprintf(errorBuffer, "Too big picture. Can be up to %d", MAX_SIZE);
        return -1;
    }


    int column = 0;
    int line = 0;
    while(fgets(buf, MAX_BUF_SIZE, inPicture) != NULL)
    {
        int i = 0;

        while(1)
        {
            i = readWord(buf, word, i);
            if(i == -1)
                break;
            if(line >= height)
            {
                sprintf(errorBuffer, "Too many numbers were given");
                return -1;
            }

            picture[line][column] = atoi(word);
            line = line + ((column + 1) / width);
            column = (column + 1) % width;
        }
    }
    if(line != height || column != 0)
    {
        sprintf(errorBuffer, "Wrong amount of numbers were given");
        return -1;
    }
    return 0;
}

int readFilter(FILE *inFilter)
{
    char buf[MAX_BUF_SIZE];
    char word[64];

    fgets(buf, MAX_BUF_SIZE, inFilter);
    readWord(buf, word, 0);
    filterSize = atoi(word);

    int column = 0;
    int line = 0;
    while(fgets(buf, MAX_BUF_SIZE, inFilter) != NULL)
    {
        int i = 0;

        while(1)
        {
            i = readWord(buf, word, i);
            if(i == -1)
                break;
            if(line >= filterSize)
            {
                sprintf(errorBuffer, "Too many numbers were given");
                return -1;
            }

            filter[line][column] = atof(word);
            line = line + ((column + 1) / filterSize);
            column = (column + 1) % filterSize;
        }
    }
    if(line != filterSize || column != 0)
    {
        sprintf(errorBuffer, "Wrong amount of numbers were given");
        return -1;
    }
    return 0;
}

int writeOutput(FILE *output)
{
    char buf[32];

    fputs("P2\n", output);
    sprintf(buf, "%d %d \n", width, height);
    fputs(buf, output);
    fputs("255 \n", output);

    int num = 0;

    int i = 0;
    while(i < height)
    {
        int j = 0;
        while(j < width)
        {
            sprintf(buf, "%d ", result[i][j]);
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

int max(int a, int b)
{
    if(a > b)
        return a;
    else
        return b;
}

int div2Ceil(int arg)
{
    return arg / 2 + arg % 2;
}

int computePixel(int line, int column)
{
    int x = line + 1;
    int y = column + 1;
    double res = 0;

    int c = filterSize;
    int i = 1;
    while(i < filterSize)
    {
        int j = 1;
        while(j < filterSize)
        {
            int left = max(1, x - div2Ceil(c) + i);
            int right = max(1, y - div2Ceil(c) + j);
            if(left <= height && right <= width)
            {
                res += (double)picture[left-1][right-1] * filter[i-1][j-1];
            }
            j++;
        }
        i++;
    }
    res = round(res);
    return (int) res;
}

void *threadBehaviour(void *arg)
{
    int *intArg = (int *) arg;
    int lines = intArg[0];
    int i = intArg[1];
    int maxLine = lines + i;
    int *retValue = malloc(sizeof(int));

    //printf("Thread working with args: %d, %d\n", lines, i);

    while(i < maxLine)
    {
        int j = 0;
        while(j < width)
        {
            result[i][j] = computePixel(i, j);
            j++;
        }
        i++;
    }

    *retValue = 0;
    return (void *) retValue;
}

int main(int argc, char *argv[])
{
    if(argc < 5)
    {
        fprintf(stderr ,"ERROR: Not enough arguments\n");
        exit(EXIT_FAILURE);
    }

    FILE *inPicture = fopen(argv[2], "r");
    FILE *inFilter = fopen(argv[3], "r");
    FILE *output = fopen(argv[4], "w");

    if(inPicture == NULL || inFilter == NULL || output == NULL)
    {
        perror("ERROR: Problem opening files");
        exit(EXIT_FAILURE);
    }

    if(readPicture(inPicture) == -1)
    {
        fprintf(stderr, "ERROR: Picture - %s\n", errorBuffer);
        exit(EXIT_FAILURE);
    }
    if(readFilter(inFilter) == -1)
    {
        fprintf(stderr, "ERROR: Filter - %s\n", errorBuffer);
        exit(EXIT_FAILURE);
    }

    timeStart();

    int threadNumber = atoi(argv[1]);

    int threadArgs[threadNumber][2];
    int coveredLines = 0;
    int i = 0;
    while(i < threadNumber)
    {
        threadArgs[i][0] = height / threadNumber;
        if(i == 0)
            threadArgs[i][0] += height % threadNumber;
        threadArgs[i][1] = coveredLines;
        coveredLines += threadArgs[i][0];
        i++;
    }

    pthread_t threads[threadNumber];

    i = 0;
    while(i < threadNumber)
    {
        int res = pthread_create(&threads[i], NULL, threadBehaviour, (void *) threadArgs[i]);
        if(res != 0)
            perror("Problem creating thread");
        i++;
    }

    int *threadRes;
    i = 0;
    while(i < threadNumber)
    {
        if(pthread_join(threads[i], (void **) &threadRes)  != 0)
            fprintf(stderr, "ERROR: Problem with joining thread\n");
        else if(*threadRes == -1)
            fprintf(stderr, "ERROR: Thread returned with -1 code\n");
        else
        {
            //printf("Thread returned with status: %d\n", *threadRes);
        }

        i++;
    }

    timeEnd();

    printf("Picture: %dx%d, filter: %d, threads: %d, time: %ld [s] %ld [ns]\n",
        width, height,
        filterSize,
        threadNumber,
        res.tv_sec, res.tv_nsec
        );

    writeOutput(output);

    fclose(inPicture);
    fclose(inFilter);
    fclose(output);

    return EXIT_SUCCESS;
}
