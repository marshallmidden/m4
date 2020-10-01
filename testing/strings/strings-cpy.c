#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>

/* ------------------------------------------------------------------------ */
/* Test out strcpy, strncpy, strlcpy, stpcpy, stpncpy functions. */

#define M4_TEST_PRINTAB								\
    fprintf(stderr, "  a=0x%02x%02x%02x%02x %02x%02x%02x%02x %02x\n",		\
		    a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]);	\
    fprintf(stderr, "  b=0x%02x%02x%02x%02x %02x%02x%02x%02x %02x\n",		\
		    b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8]);

#define M4_TEST_INIT(...)							\
    memset(a, 0, sizeof(a));							\
    a[0] = 'd';									\
    a[1] = 'e';									\
    a[2] = 'a';									\
    a[3] = 'f';									\
    memset(b, 'b', sizeof(b));							\
    fprintf(stderr, "start test '%s'\n", #__VA_ARGS__);				\
    M4_TEST_PRINTAB

#define M4_TEST_DONE(...)							\
    fprintf(stderr, "done with test %s\n", #__VA_ARGS__);			\
    M4_TEST_PRINTAB;								\
    fprintf(stderr, "\n");

#define M4_TEST_RUN(...)							\
    M4_TEST_INIT(__VA_ARGS__);							\
    fprintf(stderr, "running test %s\n", #__VA_ARGS__);				\
    __VA_ARGS__;								\
    M4_TEST_DONE(__VA_ARGS__)

/* ------------------------------------------------------------------------ */
int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    char a[9];
    char b[9];

    M4_TEST_RUN( strcpy(b,a) );
    M4_TEST_RUN( strncpy(b,a,sizeof(b)) );
    M4_TEST_RUN( stpcpy(b,a) );
    M4_TEST_RUN( stpncpy(b,a,sizeof(b)) );
    M4_TEST_RUN( strlcpy(b,a,sizeof(b)) );

    exit(0);
}	/* End of main */

