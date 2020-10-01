#include <stdio.h>

int a,b,c,d;

int main()
{
	a = 0;
	b = 0;
	d = 0;

	for (c = a; c < b+d; c++)
	{
		printf("in for loop?, a=%d, b=%d, c=%d\n");
	}
	printf("after for loop, a=%x, b=%x, c=%x\n", a,b,c);
	return(0);
}
