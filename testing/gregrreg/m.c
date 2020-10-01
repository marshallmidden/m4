#include <stdio.h>

unsigned long  greg[15] __attribute__((section(".data"), aligned(4)));
unsigned long *rreg     __attribute__((section(".data"), aligned(4)));

unsigned long bigbuff[1024];

union abc {
  unsigned long ngreg[16];
  struct def {
    unsigned long ignore_these[15];
    unsigned long *nrreg;
  } def;
} abc;

union {
  unsigned long ngreg[16];
  struct {
    unsigned long ignore_these[15];
    unsigned long *nrreg;
  };
} zzz;

int main()
{
	int i;

	for (i = 0; i < 16; i++)
	{
	    greg[i] = i;
	    abc.ngreg[i] = i;
	    zzz.ngreg[i] = i;
	}
	rreg = bigbuff;
	abc.def.nrreg = bigbuff;
	zzz.nrreg = bigbuff;

	for (i = 0; i < 16; i++)
	{
	    printf("%2d greg=%08x   ngreg=%08x  zzz.ngreg=%08x\n", i, greg[i], abc.ngreg[i], zzz.ngreg[i]);
	}

	exit(0);
}
