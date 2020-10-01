#include <stdio.h>

static char small[] = "a";
static char big[] = "abcdefghijklmnopqrstuvwxyz";
static char eight[] = "eight-8.";

int main()
{
    fprintf(stderr, "(%8s)\n", small);
    fprintf(stderr, "(%8s)\n", big);
    fprintf(stderr, "(%8s)\n", eight);

    fprintf(stderr, "(%-8s)\n", small);
    fprintf(stderr, "(%-8s)\n", big);
    fprintf(stderr, "(%-8s)\n", eight);

    fprintf(stderr, "(%-8.8s)\n", small);
    fprintf(stderr, "(%-8.8s)\n", big);
    fprintf(stderr, "(%-8.8s)\n", eight);

    fprintf(stderr, "(%8.8s)\n", small);
    fprintf(stderr, "(%8.8s)\n", big);
    fprintf(stderr, "(%8.8s)\n", eight);

}
