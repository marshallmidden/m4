#include <stdio.h>

int main()
{
   char *a;

   fprintf(stderr, "hi there.\n");
   a = (char *)malloc(1*1024*1024);
   fprintf(stderr, "a = %p\n", a);
   exit(0);
}
