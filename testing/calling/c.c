#include <stdio.h>
#include "local.h"
void c(int one, int two, int three)
{
  int w,x,y,z;
  w = 0;
  x = 1;
  y = 2;
  z = 3;
  fprintf(stderr, "in c(0x%x, 0x%x, 0x%x) ebp=%p  esp=%p\n", one, two, three, get_ebp(), get_esp());
  fprintf(stderr, "c locals=%d, %d, %d, %d\n", w,x,y,z);
}
