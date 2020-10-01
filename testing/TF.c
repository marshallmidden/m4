#include <stdio.h>
#include <stdlib.h>

int a = 0;
int b = 1;
int c = -1;
int d;
int e;
int f = 27;

int main()
{
    if (a) {
	fprintf(stderr, "a (%d) is true\n", a);
    } else {
	fprintf(stderr, "a (%d) is false\n", a);
    }
    if (b) {
	fprintf(stderr, "b (%d) is true\n", b);
    } else {
	fprintf(stderr, "b (%d) is false\n", b);
    }
    if (c) {
	fprintf(stderr, "c (%d) is true\n", c);
    } else {
	fprintf(stderr, "c (%d) is false\n", c);
    }

    d = (a == 0);
    e = (a != 0);
    fprintf(stderr, "(a==0) %d, (a!=0) %d\n", d, e);

    if (f) {
	fprintf(stderr, "f (%d) is true\n", f);
    } else {
	fprintf(stderr, "f (%d) is false\n", f);
    }
    exit(0);
}
