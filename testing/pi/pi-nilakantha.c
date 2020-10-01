#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    double seq = 2.0;
    double pi = 3.0;
    int count = 1;

    if (argc > 1)
    {
	fprintf(stderr, "argc=%d, argv[0]=%s, argv[1]=%s, ...\n", argc, argv[0], argv[1]);
	exit(0);
    }

    printf("Nilakantha, 15th century\n");
    for (;;)
    {
	pi = pi + (4.0/(seq * (seq+1.0) * (seq+2.0)));
	seq += 2.0;
	pi = pi - (4.0/(seq * (seq+1.0) * (seq+2.0)));
	seq += 2.0;

	printf("%5d pi=%2.20f\n", count, pi);
	count++;
    }

}
