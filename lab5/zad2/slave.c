#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <time.h>

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
    if(argc < 3)
    {
        perror("Not enough arguments");
        return 1;
    }
    if(isNumber(argv[2]) != 0)
    {
        perror("Second argument has to be number");
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    if(checkArguments(argc, argv) != 0)
    {
        return EXIT_FAILURE;
    }

    int N = atoi(argv[2]);

    printf("Process PID: %d\n\n", getpid());


    int buffSize = 1024;
    char inBuff[buffSize];
    char outBuff[buffSize];
    int i = 0;
    while(i < N)
    {
        FILE *dateCall = popen("date", "r");
        fread(inBuff, 1, buffSize, dateCall);
        sprintf(outBuff, "PID: %d  DATE: %s", getpid(), inBuff);

        FILE *fifo = fopen(argv[1], "w");
        fwrite(outBuff, 1, buffSize, fifo);
        printf("Sent message\n");
        fclose(fifo);

        sleep(rand()%4 + 2);
        i++;
        fclose(dateCall);
    }

    FILE *fifo = fopen(argv[1], "w");
    char *endMessage = "END\0";
    fwrite(endMessage, 1, 4 ,fifo);
    fclose(fifo);

    return 0;
}