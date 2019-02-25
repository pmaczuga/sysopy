#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <errno.h>

int checkArguments(int argc, char *argv[])
{
    if(argc < 2)
    {
        perror("Not enough arguments");
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if(checkArguments(argc, argv) != 0)
    {
        return EXIT_FAILURE;
    }


    if(mkfifo(argv[1], 0666) != 0)
    {
        perror("Problem with creating fifo");
        return EXIT_FAILURE;
    }

    FILE *fifo = fopen(argv[1], "r");
    if(fifo == NULL)
    {
        perror("Problem with opening fifo");
        return EXIT_FAILURE;
    }

    printf("Opened fifo\n\n");

    int buffSize = 1024;
    char buff[buffSize];
    while(1)
    {
        int charsRead;
        if(charsRead = fread(buff, 1, buffSize, fifo) != 0)
        {
            if(strncmp(buff, "END", charsRead) == 0)
                break;
            printf("%s\n", buff);
        }
    }

    fclose(fifo);
    remove(argv[1]);

    return EXIT_SUCCESS;
}