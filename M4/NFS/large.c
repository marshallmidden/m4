#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>

#define PAGESIZE 4096

int main(int argc, char **argv)
{
    size_t          size;
    size_t          count;
    long	    loop;
    long            input = 0;

    unsigned char  *base;
    unsigned char  *limit;
    unsigned char  *p;

    if (argc != 2 || (size = atoi(argv[1])) <= 1)
    {
	fprintf(stderr, "Usage: %s size(MB)\n", argv[0]);
	exit(-1);
    }

    input = size;
    fprintf(stderr, "Size = %.3f GB\n", size / 1024.0);
    size *= 1024 * 1024;

    base = malloc(size);
    if (base == 0)
    {
	fprintf(stderr, "%s: malloc failed\n", argv[0]);
	exit(-1);
    }

    nice(20);

    limit = base + size;
    count = 0;
    loop = 0;

    fprintf(stderr, "Starting write loop...\n");
    
    for (;;)
    {
	loop++;
	count++;

	for (p = base; p != limit; p += PAGESIZE)
	{
	    *(size_t *) p = count++;
	}

	if (base[50] == 3)	/* Force a read, compilers are too smart! */
	{
	    return 2;
	}

	if (loop == 1)
	{
	    fprintf(stderr, "Done with first loop setting every %d bytes, for %ld MB\n", PAGESIZE, input);
	}
    }

    return 0;
}
