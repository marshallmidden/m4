#include <stdio.h>

asm("banana: .ascii \"DTMT\"");

/* unsigned int banner = 0x44544d54; */
unsigned int banner = 0x544d5444;
extern unsigned int banana;


int main()
{
    unsigned int aha = banana;

    printf("banana=%c%c%c%c\n",
    		(banana >> 24)&0xff,
    		(banana >> 16)&0xff,
    		(banana >> 8)&0xff,
    		(banana >> 0)&0xff);
    printf("banner=%c%c%c%c\n",
    		(banner >> 24)&0xff,
    		(banner >> 16)&0xff,
    		(banner >> 8)&0xff,
    		(banner >> 0)&0xff);
    printf("aha   =%c%c%c%c\n",
    		(aha    >> 24)&0xff,
    		(aha    >> 16)&0xff,
    		(aha    >> 8)&0xff,
    		(aha    >> 0)&0xff);

    exit(0);
}
