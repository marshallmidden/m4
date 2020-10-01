#include <stdio.h>
#include <math.h>
#include <stdlib.h>
// #include <malloc.h>

// Note: sizeof(long) = sizeof(pointer) for both 32 and 64 bit machines.
long *abc[5];
long **def;

int main()
{
	long a1 = 1;
	long a2 = 2;
	long a3 = 3;
	long a4 = 4;
	long a5 = 5;

	abc[0] = &a1;
	abc[1] = &a2;
	abc[2] = &a3;
	abc[3] = &a4;
	abc[4] = &a5;

	def = malloc(sizeof(long *) * 5);
	long b1 = 101;
	long b2 = 102;
	long b3 = 103;
	long b4 = 104;
	long b5 = 105;
	def[0] = &b1;
	def[1] = &b2;
	def[2] = &b3;
	def[3] = &b4;
	def[4] = &b5;
	
	long **b = abc;
	long **c = def;
	int i;
	for (i = 0; i < 5; i++)
	{
	    printf("b[%d] = %d    c[%d] = %d\n", i, *b[i], i, *c[i]);
	}
	int j;
	for (j = 0; j < 5; j++)
	{
	    printf("abc[%d] = %d    def[%d] = %d\n", j, *abc[j], j, *def[j]);
	}
	exit(0);
}
