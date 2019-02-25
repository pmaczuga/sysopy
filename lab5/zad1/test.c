#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	char buffer[2000];
	FILE *in; 
	FILE *out;
	in = popen(argv[1], "r");
	out = popen(argv[2], "w");
	
	while(fgets(buffer, 100, in))
	{
		fputs(buffer, out);
	}

    pclose(in);
    pclose(out);

	return 0;
}
