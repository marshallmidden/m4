#include <stdio.h>

struct testing {
	unsigned char a;
	unsigned char b;
	unsigned char c;
	unsigned char d;
} abc = {1,2,3,4};

#if 0
typedef struct data {
	int a;
	int b;
} data;

typedef struct ft {
	int c;
	int d;
} ft;

typedef struct big {
	int e;
	union {
	  struct data f;
	  struct ft g;
	} u;
} big;
#endif /* 0 */

typedef struct data data;
typedef struct ft ft;

typedef struct big {
	int e;
	union {
	struct data {
		int a;
		int b;
	} f;
	struct ft {
		int c;
		int d;
	} g;
	} u;
} big;

big z;
data y;
ft x;

struct ILT;
typedef struct abc ABC;

int main()
{
    printf("struct = %d, %d, %d, %d\n", abc.a, abc.b, abc.c, abc.d);
    printf("struct = %-8.8x\n", *((unsigned int*)&(abc.a)));
    printf("sizeof(ILT*)=%d, sizeof(ABC*)=%d\n", sizeof(struct ILT*), sizeof(ABC*));

    z.e = 9;
    z.u.f.a = 10;
    z.u.f.b = 11;
    printf("z.e=%d (9?)  z.u.g.c=%d (10?)  z.u.g.d=%d (11?)\n", z.e, z.u.g.c, z.u.g.d);
    return(0);
}
