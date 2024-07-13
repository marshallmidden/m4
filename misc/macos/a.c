#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    char *a = NULL;
    char b[3] = "ab";

//    for (int i = 0; i < 99999999; i++)
    for (int i = 0; i < 9999; i++)
    {
	*(a+i) = b[i];
    }
    fprintf(stderr, "exiting\n");
    exit(0);
}
