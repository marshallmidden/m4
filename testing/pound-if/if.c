
#include <stdio.h>


int main(void)
{
#if ABCDEFG
    printf("ABCDEFG\n");
#endif
#if !ABCDEFG
    printf("!ABCDEFG\n");
#endif
    exit(0);
}
