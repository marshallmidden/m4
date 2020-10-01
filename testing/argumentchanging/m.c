#include <stdio.h>
#include <math.h>
#include <malloc.h>

// Note: sizeof(long) = sizeof(pointer) for both 32 and 64 bit machines.
unsigned long *abc[2][5];
unsigned long *((*def)[2][5]);

void subroutine(long aa, long bb, long cc)
{
	printf("subroutine - aa=%d, bb=%d, cc=%d\n", aa, bb, cc);
	aa = 10;
	bb = 11;
	cc = 12;
	printf("subroutine - aa=%d, bb=%d, cc=%d\n", aa, bb, cc);
}

int main()
{
	long a = 0;
	long b = 1;
	long c = 2;

	printf("main - a=%d, b=%d, c=%d\n", a, b, c);
	subroutine(a,b,c);
	printf("main - a=%d, b=%d, c=%d\n", a, b, c);

	exit(0);
}
