
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <byteswap.h>


static unsigned long long A64;
static unsigned long long B64;

static unsigned long A32;
static unsigned long B32;

static unsigned char *p;

static void p_p(char *str)
{
	int i;

	fprintf(stderr, "%s ", str);

	for (i = 0; i < 8; i++)
	{
	    fprintf(stderr, " %02x", *p++);
	}
	fprintf(stderr, "\n");
}

int main(void)
{

	A32 = 0x01020304;
	A64 = 0x0102030405060708ULL;

	B32 = bswap_32(A32);
	B64 = bswap_64(A64);

	fprintf(stderr, "32: orig = 0x%8.8lx   bswap_32=0x%8.8lx\n", A32, B32);
	fprintf(stderr, "64: orig = 0x%16.16llx  bswap_64=0x%16.16llx\n", A64, B64);
	fprintf(stderr, "    orig = 0x%8.8lx %8.8lx  bswap_64=0x%8.8lx %8.8lx\n",
		A64 >> 32, A64 & 0xffffffff,
		B64 >> 32, B64 & 0xffffffff);

	p = (unsigned char *)&A64;
	p_p("A64");
	p = (unsigned char *)&B64;
	p_p("B64");
	exit(0);
}
