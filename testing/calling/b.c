#include <stdio.h>
#include "local.h"
void b(int one, int two)
{
  int w,x,y,z;
  w = 0;
  x = 1;
  y = 2;
  z = 3;
  fprintf(stderr, "in b(0x%x, 0x%x) ebp=%p  esp=%p\n", one, two, get_ebp(), get_esp());
  fprintf(stderr, "  locals=%d, %d, %d, %d\n", w,x,y,z);
  c(0x100+z, 0x200+z, 0x300+z);
  fprintf(stderr, "b locals=%d, %d, %d, %d\n", w,x,y,z);
}
