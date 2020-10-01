#include <stdio.h>
#include <stdlib.h>

#define TRUE 1

int gt2tb = 0;
int b = 1;
int c = -1;

#define cs(dlink,dlinkgt2tb)	(*(gt2tb != TRUE ? &dlink : &dlinkgt2tb))

int main()
{
    gt2tb = 0;
    b = 1;
    c = 2;
//    *(a != 1 ? &b : &c) = 5;
    cs(b,c) = 5;
    fprintf(stderr, "%d != %d ? %d : %d\n", gt2tb, TRUE, b, c);

    gt2tb = 1;
    b = 1;
    c = 2;
//    *(a != 1 ? &b : &c) = 5;
    cs(b,c) = 5;
    fprintf(stderr, "%d != %d ? %d : %d\n", gt2tb, TRUE, b, c);

    exit(0);
}
