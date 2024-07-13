#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int i;
	for (i = -2; i < 3; i++)
	{
	    if (i)
		{
			fprintf(stderr, "%d is TRUE  --", i);
		}
		else
		{
			fprintf(stderr, "%d is FALSE --", i);
		}
	    if (!i)
		{
			fprintf(stderr, "!%d is TRUE\n", i);
		}
		else
		{
			fprintf(stderr, "!%d is FALSE\n", i);
		}
	}
	exit(0);
}
