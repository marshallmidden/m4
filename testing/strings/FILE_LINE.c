
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *thefile = __FILE__;

#define InitMutex(m)	XK_InitMutex(m, __FILE__, __LINE__)


static void XK_InitMutex(int m, const char *file, const unsigned int line)
{
  thefile = file;
  fprintf(stderr, "XK_InitMutex, m=%d, file=%s, line=%u\n", m, file, line);
}

int main(void)
{
	fprintf(stderr, "strlen(abc)=%d\n", (int)strlen("abc"));

	InitMutex(1);

	fprintf(stderr, "thefile=%p\n", thefile);
	fprintf(stderr, "thefile=   (%s)\n", thefile);
	fprintf(stderr, "here=%p\n", (const char *)__FILE__);
	fprintf(stderr, "here=   (%s)\n", (const char *)__FILE__);


	exit(0);
}
