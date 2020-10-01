#include <stdio.h>
#include <math.h>
#include <malloc.h>

// Note: sizeof(long) = sizeof(pointer) for both 32 and 64 bit machines.
unsigned long *abc[2][5];
unsigned long *((*def)[2][5]);

int main()
{
	long a = 0;

	abc[0][0] = (unsigned long*)++a;
	abc[0][1] = (unsigned long*)++a;
	abc[0][2] = (unsigned long*)++a;
	abc[0][3] = (unsigned long*)++a;
	abc[0][4] = (unsigned long*)++a;
	abc[1][0] = (unsigned long*)++a;
	abc[1][1] = (unsigned long*)++a;
	abc[1][2] = (unsigned long*)++a;
	abc[1][3] = (unsigned long*)++a;
	abc[1][4] = (unsigned long*)++a;

	def = malloc(sizeof(*def));
	printf("sizeof(*def)=%d\n", sizeof(*def));
	a = 100;
	(*def)[0][0] = (unsigned long*)++a;
	(*def)[0][1] = (unsigned long*)++a;
	(*def)[0][2] = (unsigned long*)++a;
	(*def)[0][3] = (unsigned long*)++a;
	(*def)[0][4] = (unsigned long*)++a;
	(*def)[1][0] = (unsigned long*)++a;
	(*def)[1][1] = (unsigned long*)++a;
	(*def)[1][2] = (unsigned long*)++a;
	(*def)[1][3] = (unsigned long*)++a;
	(*def)[1][4] = (unsigned long*)++a;
	
	unsigned long *b = (unsigned long *)abc;
	unsigned long *c = (unsigned long *)def;
	int i;
	for (i = 0; i < 10; i++)
	{
	    printf("b[%d] = %d    c[%d] = %d\n", i, b[i], i, c[i]);
	}
	int j;
	for (i = 0; i < 2; i++)
	{
	    for (j = 0; j < 5; j++)
	    {
		printf("abc[%d][%d] = %d    (*def)[%d][%d] = %d\n", i,j, abc[i][j], i,j, (*def)[i][j]);
	    }
	}
	exit(0);
}
