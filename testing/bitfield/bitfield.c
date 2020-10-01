#include <stdio.h>
#include <stdlib.h>

int main()
{
    struct bit_field
    {
        int bit0:1;
        int bit1:1;
        int bit2:1;
        int bit3:1;
        int topfour:4;
    } b;

    b.bit3 = 1;
    b.bit2 = 1;
    b.bit1 = 0;
    b.bit0 = 1;
    b.topfour = 0;

    printf("byte=0x%02x (expect 0x0d)\n", *((unsigned char*)&b));
    exit(0);
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
