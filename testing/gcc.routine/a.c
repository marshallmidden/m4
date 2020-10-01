#include <stdio.h>
#include "local.h"
void a(int one)
{
  int w,x,y,z;
  w = 0;
  x = 1;
  y = 2;
  z = 3;
  fprintf(stderr, "in a(0x%x) ebp=%p  esp=%p\n", one, get_ebp(), get_esp());
  fprintf(stderr, "  locals=%d, %d, %d, %d\n", w,x,y,z);
  b(0x100 + y, 0x200 + y);
  fprintf(stderr, "a locals=%d, %d, %d, %d\n", w,x,y,z);
}
