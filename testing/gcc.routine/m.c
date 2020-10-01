#include <stdio.h>
#include "local.h"
int main()
{
  int w,x,y,z;
  w = 0;
  x = 1;
  y = 2;
  z = 3;
  fprintf(stderr, "in m  ebp=%p  esp=%p\n", get_ebp(), get_esp());
  fprintf(stderr, "  locals=%d, %d, %d, %d\n", w,x,y,z);
  a(0x100+x);
  fprintf(stderr, "m locals=%d, %d, %d, %d\n", w,x,y,z);
/*   exit(0); */
  return(0);
}
