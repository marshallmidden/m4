#include <stdio.h>
#include <stdlib.h>
#include "get_single_ch.h"

int main(void)
{
    char            key = 0;

    do
    {
        key = get_single_ch();
        fprintf(stderr, "key=%02x\n", key);
    } while (key != 'q');
    exit(0);
}   /* End of main */
