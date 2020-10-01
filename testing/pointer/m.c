#include <stdio.h>
#include <malloc.h>

struct abc
{
	int a;
	int b;
	int c;
	int d;
};

int main()
{
	struct abc *p = (struct abc *)malloc(2 * sizeof(struct abc *));
	struct abc *q;

	q = p + 1;
	fprintf(stderr, "p = %p, q= %p\n", p, q);
	return(0);
}

