
#include <stdio.h>
static char str1[] = "Thisisthestring.";
static char str2[] = "Short.";

int main(void)
{
    printf(".10s=(%.10s)\n", str1);
    printf("10s=(%10s)\n", str1);
    printf("10.10s=(%10.10s)\n", str1);
    printf("\n");
    printf(".10s=(%.10s)\n", str2);
    printf("10s=(%10s)\n", str2);
    printf("10.10s=(%10.10s)\n", str2);
    exit(0);
}
