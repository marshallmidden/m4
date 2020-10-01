#include <stdio.h>

static int v1 = 0x4117;
static int v2 = 0x104c;

static void vprint(int v)
{
	int thousandths	= (v & 0x000f);
	int hundredths 	= (v & 0x00f0) >> 4;
	int tenths 	= (v & 0x0f00) >> 8;
	int units 	= (v & 0xf000) >> 12;

	if (v == 0xFFFF)
	{
	    printf("END MARKER 0xFFFF\n");
	}
	else if (v == 0xFFFD)
	{
	    printf("0xFFFD The battery has been disabled using the jumper or the battery fuse is open\n");
	}
	else if (thousandths == 0xC)
	{
	    printf("0xxxxC The battery has been disabled using software\n");
	}
	else if (thousandths >= 0xA || hundredths >= 0xA || tenths >= 0xA || units >= 0xA)
	{
	    printf("RESERVED DIGIT - units=%x tenths=%x hundredths=%x thousandths=%x\n",
		    units, tenths, hundredths, thousandths);
	}
	else
	{
	    printf("units=%d tenths=%d hundredths=%d thousandths=%d   => %d.%d%d%d\n",
		    units, tenths, hundredths, thousandths, units, tenths, hundredths, thousandths);
	}
}


int main()
{
	vprint(v1);
	vprint(v2);
}
