#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <ftw.h>
#include <sys/types.h>
#include <unistd.h>

//arguments
char *mode;      // > < or = 
char *argTime;

//----------------------------------CHECK-ARGUMENTS------------------------------------------

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

int isValueInArray(int val, int *array, int size)
{
	for(int i = 0;i<size;i++)
	{
		if(val == array[i])
			return 0;
	}
	
	return 1;
}

int checkArguments(int argc, char *argv[])
{
	if(argc < 5)
	{
		printf("Not enough arguments\n");
		return 1;
	}
	char *mode = argv[2];
	if(strcmp(mode, ">") != 0 && strcmp(mode, "<") != 0 && strcmp(mode, "=") != 0)
	{
		printf("Second argument must be one of the following: '<', '>' or '='\n");
		return 1;
	}
	
	char *day = malloc(3);
	char *month = malloc(3);
	char *year = malloc(5);
	
	int size = strlen(argv[3]);
	int i = 0;
	int j = 0;
	while(argv[3][i] != '-' && i < size && j<2)
	{
		day[j] = argv[3][i];
		i++;
		j++;
	}
	day[j] = '\0';
	if(i == size || argv[3][i] != '-')
	{
		printf("Third argument must be DD-MM-YYYY\n");
		return 1;
	}
	j = 0;
	i++;
	while(argv[3][i] != '-' && i < size && j<2)
	{
		month[j] = argv[3][i];
		i++;
		j++;
	}
	month[j] = '\0';
	if(i == size || argv[3][i] != '-')
	{
		printf("Third argument must be DD-MM-YYYY\n");
		return 1;
	}
	j = 0;
	i++;
	while(i < size && j < 4)
	{
		year[j] = argv[3][i];
		i++;
		j++;
	}
	year[j] = '\0';
	if(i != size)
	{
		printf("Third argument must be DD-MM-YYYY\n");
		return 1;
	}
	
	if(isNumber(day) != 0 || isNumber(month) != 0 || isNumber(year) != 0)
	{
		printf("Day, month and year must be numbers\n");
		return 1;
	}
	
	int d = atoi(day);
	int m = atoi(month);
	int y = atoi(year);
	
	int days30[4] = {4, 6, 9, 11};
	int days31[7] = {1, 3, 5, 7, 8, 10, 12};
	int days29[1] = {2};
	
	if(y < 1900) return 1;
	if(isValueInArray(m, days30, 4) == 0 && d > 30)
	{
		printf("Day in this month must cant be greater than 30\n");
		return 1;
	}
	else if(isValueInArray(m, days31, 7) == 0 && d > 31)
	{
		printf("Day in this month must cant be greater than 31\n");
		return 1;
	}
	else if(isValueInArray(m, days29, 1) == 0 && d > 29)
	{
		printf("Day in this month must cant be greater than 29\n");
		return 1;
	}
	if(m <= 0 || m > 12)
	{
		printf("Month must be number between 1 and 12\n");
		return 1;
	}
	if(d <= 0)
	{
		printf("Day have to be greater than 0\n");
		return 1;
	}
	if(strcmp(argv[4], "nftw") !=0 && strcmp(argv[4], "regular") != 0)
	{
		printf("Last argument has to be either: 'nftw' or 'regular'\n");
		return 1;
	}
	
	return 0;
}


//------------------------------------------FUNCTIONS-----------------------------------

char *concat2(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char *concat3(char *s1, char *s2, char *s3)
{
	char *tmp = concat2(s1, s2);
	char *result = concat2(tmp, s3);
	free(tmp);
	return result;
}

int isFolder(char *path)
{
    struct stat fileStat;
    lstat(path, &fileStat);
    return S_ISDIR(fileStat.st_mode);
}

int isRegular(char *path)
{
    struct stat fileStat;
    stat(path, &fileStat);
    return S_ISREG(fileStat.st_mode);
}

int compareTm(struct tm *t1, struct tm *t2)
{
	if(t1->tm_year > t2->tm_year)
		return 1;
	else if(t1->tm_year < t2->tm_year)
		return -1;
	else
	{
		if(t1->tm_mon > t2->tm_mon)
			return 1;
		else if(t1->tm_mon < t2->tm_mon)
			return -1;
		else
		{
			if(t1->tm_mday > t2->tm_mday)
				return 1;
			else if(t1->tm_mday < t2->tm_mday)
				return -1;
			else
				return 0;
		}
	}
}

struct tm *charToTm(char *oldTime)
{
	char *day = malloc(3);
	char *month = malloc(3);
	char *year = malloc(5);
	
	int size = strlen(oldTime);
	int i = 0;
	int j = 0;
	while(oldTime[i] != '-' && i < size)
	{
		day[j] = oldTime[i];
		i++;
		j++;
	}
	day[j] = '\0';
	j = 0;
	i++;
	while(oldTime[i] != '-' && i < size)
	{
		month[j] = oldTime[i];
		i++;
		j++;
	}
	month[j] = '\0';
	j = 0;
	i++;
	while(i < size)
	{
		year[j] = oldTime[i];
		i++;
		j++;
	}
	year[j] = '\0';
	
	struct tm *newTime = malloc(sizeof(struct tm));
	
	newTime->tm_mday = atoi(day);
	newTime->tm_mon = atoi(month) - 1;
	newTime->tm_year = atoi(year) - 1900;
	
	free(day);
	free(month);
	free(year);
	return newTime;
}

void printFileInfo(char *path)
{	
	struct stat fileStat;
	stat(path, &fileStat);

	//permissions
    printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
	
    //size
    printf("\t");
    printf("%d",fileStat.st_size);
    
    //modification date
    printf("\t");
    char date[20];
    strftime(date, 20, "%d-%m %H:%M", localtime(&fileStat.st_mtime));
    printf("%s", date);
	
	//full path
	printf("\t");
 	char *fullPath = realpath(path, NULL);
 	printf("%s", fullPath);

    printf("\n");
}

int checkFileDate(char *path)
{
	struct stat fileStat;
	stat(path, &fileStat);
	
	struct tm *fileTime = localtime(&fileStat.st_mtime);
	struct tm *tmTime = charToTm(argTime);
	
	int res = compareTm(fileTime, tmTime);
	
	if(strcmp(mode, "<") == 0 && res < 0)
		return 0;
	if(strcmp(mode, ">") == 0 && res > 0)
		return 0;
	if(strcmp(mode, "=") == 0 && res == 0)
		return 0;
	
	return 1;
}

int listFiles(char *path)
{
	struct dirent *file;
	DIR *dir = NULL;
	
	dir = opendir(path);
	if(dir == NULL)
	{
		printf("Problem with opening directory\n");
		return 1;
	}
	while(file = readdir(dir))
	{
		if(strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0)
		{
			char *pathToFile = concat3(path, "/", file->d_name);
			
			if(isRegular(pathToFile))
			{
				//laboratory 3 change
				if(checkFileDate(pathToFile) == 0)
				{
					printf("%s PID: %d\t\t", file->d_name, (int)getpid());
					printFileInfo(pathToFile);
				}
				//end of changes
			}
			if(isFolder(pathToFile))
			{
				//laboratory 3 change
				pid_t child;
				child = fork();
				if(child == 0)
				{
					listFiles(pathToFile);
					exit(0);
				}
				//end of changes
			}
			free(pathToFile);
		}
	}
	closedir(dir);
	free(file);
}

int printFileInfoNftw(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
	char *path = malloc(strlen(fpath)+1);
	strcpy(path, fpath);
	if(isRegular(path) && checkFileDate(path) == 0)
	{
		printFileInfo(path);
	}
	return 0;
}


//------------------------------------------------MAIN---------------------------------
int main(int argc, char *argv[])
{	
	if(checkArguments(argc, argv) != 0)
		return 1;
		
	mode = argv[2];
	argTime = argv[3];
	
	if(strcmp(argv[4], "regular") == 0)
	{
		printf("\nListing files using opendir and readdir functions\n\n");
		int res = listFiles(argv[1]);
		if(res != 0)
		{
			printf("Some error in listFiles function\n");
			return 1;
		}
	}
	else
	{
		printf("\nListing files using nftw function\n\n");
		int res = nftw(argv[1], &printFileInfoNftw, 20, FTW_PHYS);
		if(res != 0)
		{
			printf("Some error in nftw function\n");
			return 1;
		}	
	}
	return 0;
}

