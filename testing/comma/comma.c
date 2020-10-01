#include <stdio.h>

int a = 0;
int b = 1;
int c;
int d;
int j;
int k;

int main()
{
    c = (a == 0) ? (j = 10, 9) : (j = 11, 8); 
    d = (a == 1) ? (k = 10, 9) : (k = 11, 8); 
    fprintf(stderr, "(%d==0) c=%d, j=%d\n", a, c, j);
    fprintf(stderr, "(%d==1) d=%d, k=%d\n", a, d, k);
    exit(0);
}
