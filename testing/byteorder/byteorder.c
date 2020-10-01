
#include <stdio.h>

union TOGETHER {
	char cmd[16];
	unsigned int words[4];
} a;

int main(void)
{
	a.cmd[0] = 0x1;
	a.cmd[1] = 0x2;
	a.cmd[2] = 0x3;
	a.cmd[3] = 0x4;
	a.cmd[4] = 0x5;
	a.cmd[5] = 0x6;
	a.cmd[6] = 0x7;
	a.cmd[7] = 0x8;
	a.cmd[8] = 0x9;
	a.cmd[9] = 0xa;
	a.cmd[10] = 0xb;
	a.cmd[11] = 0xc;
	a.cmd[12] = 0xd;
	a.cmd[13] = 0xe;
	a.cmd[14] = 0xf;
	a.cmd[15] = 0xff;

	fprintf(stderr, "words[0]=%#x, words[1]=%#x, words[2]=%#x, words[3]=%#x\n", a.words[0], a.words[1], a.words[2], a.words[3]);

	exit(0);
}
