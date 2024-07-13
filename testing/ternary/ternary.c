#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* ------------------------------------------------------------------------ */
/*What does the syntax  "x ?: b;" do. */
/* ------------------------------------------------------------------------ */
#define NORETURN __attribute__((noreturn))
#define UNUSED   __attribute__((unused)) /*@unused@*/

/* ------------------------------------------------------------------------ */
int main(int argc UNUSED, char **argv UNUSED)
{
    char a = 0;
    char b = 1;
    char c = 2;

    switch (a ?: b)
    {
        case 0: printf("a ?: b;   gives 0\n"); break;
        case 1: printf("a ?: b;   gives 1\n"); break;
        case 2: printf("a ?: b;   gives 2\n"); break;
        default: printf("a ?: b;   default\n"); break;
    }

    switch (b ?: c)
    {
        case 0: printf("b ?: c;   gives 0\n"); break;
        case 1: printf("b ?: c;   gives 1\n"); break;
        case 2: printf("b ?: c;   gives 2\n"); break;
        default: printf("b ?: c;   default\n"); break;
    }

    switch (c ?: a)
    {
        case 0: printf("c ?: a;   gives 0\n"); break;
        case 1: printf("c ?: a;   gives 1\n"); break;
        case 2: printf("c ?: a;   gives 2\n"); break;
        default: printf("c ?: a;   default\n"); break;
    }

    exit(0);
}	/* End of main */

/* ------------------------------------------------------------------------ */
/*
 * vi: sw=4 ts=8 expandtab
 */
