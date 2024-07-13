#include <stdio.h>
#include <stdlib.h>

int a = 8;
int b = 6;

int main()
{
    int d;
    int e;

    fprintf(stderr, "a=(%d)\n", a);
    fprintf(stderr, "b=(%d)\n", b);

    d = (a % 6);
    e = (a % 4);
    fprintf(stderr, "a%%6=%d  a%%4=%d\n", d, e);

    exit(0);
}
