
#include <stdio.h>

#define	ERROR	((void)0)

//#define SIZEOF(x) (__builtin_choose_expr(__builtin_constant_p(x),ERROR,sizeof(x)))
//#define	SIZEOF(x) (__builtin_constant_p(x) ? ERROR : sizeof(x))
//#define	SIZEOF(x) (__builtin_types_compatible_p(x,x), sizeof(x))

//-- #define	SIZEOF(x,s,o) (__builtin_types_compatible_p(x,x), s##o(x))
//-- #define	sizeof(x) SIZEOF(x,size,of)
#define	sizeof(x) ((!__builtin_types_compatible_p(x,x)) | sizeof(x))

#define	CONST	74

typedef struct abc
{
	long	word;
	short	s;
} ABC;

int main(void)
{
	printf("struct=%d\n", sizeof(ABC));
//	printf("CONST=%d\n",
//		//(__builtin_constant_p(ABC) ?
//			//ERROR :
//			__builtin_choose_expr(
//			__builtin_constant_p(ABC), ERROR, sizeof(ABC))/*)*/);
	printf("struct=%d\n", sizeof(ABC));
//	printf("CONST=%d\n", sizeof(CONST));
	return 0;
}
