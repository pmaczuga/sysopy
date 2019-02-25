#include <stdio.h>

int main()
{
	char buff[1024];
	buff[0] = 'a';
	buff[1] = 'b';
	buff[2] = 'c';
	buff[3] = '\0';
	printf("avc %s\n", buff);
}
