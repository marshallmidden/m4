#include <stdio.h>

int main()
{
	char data[5];
	int i;

	for (i = 0; i < 5; i++)
	{
	  data[i] = i-2;
	}

	for (i = 0; i < 5; i++)
	{
	    fprintf(stderr, "%d =%2.2hhx\n", i, data[i]);
	}
	exit(0);
}
