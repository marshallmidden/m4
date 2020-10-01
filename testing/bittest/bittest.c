#include <stdio.h>
#include <stdlib.h>

int main()
{
	int abc=1;
	int bcd=2;

	if ((abc & 1)) {
	  fprintf(stderr, "abc has bit 0 set to 1.\n");
	} else {
	  fprintf(stderr, "abc has bit 0 set to 0.\n");
	}

	if ((bcd & 1)) {
	  fprintf(stderr, "bcd has bit 0 set to 1.\n");
	} else {
	  fprintf(stderr, "bcd has bit 0 set to 0.\n");
	}
	exit (0);
}
