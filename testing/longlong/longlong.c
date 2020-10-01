#include <stdio.h>

long long g = 8589934591LL;	/* 2^33 -1 */

void f(long long *A)
{
  g = *A + 1;
}


int main()
{
    long long h;

    h = (g+1)*2 - 1;

/*     printf("before f: g=%qx, h=%qx\n", g, h); */
    printf("before f: g=%llx, h=%llx\n", g, h);
    f(&h);
/*     printf("after f:  g=%qx, h=%qx\n", g, h); */
    printf("after f:  g=%llx, h=%llx\n", g, h);
    return(0);
}
