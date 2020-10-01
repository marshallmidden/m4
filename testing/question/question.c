#include <stdio.h>
int main(void)
{
    int a = 2;
    int b = 3;
    int c = 4;
    int d = 5;

    (a == 2) ? (d=6) : 0;
    fprintf(stderr, "a=%d, b= %d, c=%d, d= %d\n", a,b,c,d);
    exit(0);
}
