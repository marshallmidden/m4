#include <stdio.h>
#include <stdlib.h>

#define SCSI_SES_DIAGNOSTIC_PAGE_80 "\x1c\x01\x80\x01\x00\x00"
char           tempStr[20];
struct this { int a; long b; float c; } z;

// #define sizeof(x) ((!__builtin_types_compatible_p(x,x)) | sizeof(x))

extern unsigned int __invalid_size_argument;
#define sizeof(t)    ( sizeof(t) == sizeof(t[1]) ?  sizeof(t) : __invalid_size_argument )



int main(void)
{
  int c = sizeof(struct this);
  int d = sizeof(z);
  int a = sizeof(tmpStr);
  int b = sizeof("abc");

  printf("SCSI_SES_DIAGNOSTIC_PAGE_80's size=%d\n", sizeof(SCSI_SES_DIAGNOSTIC_PAGE_80));
  printf("sizeof(tempStr)=%d, sizeof(*tempStr)=%d\n", sizeof(tempStr), sizeof(*tempStr));
  printf("sizeof(5)=%d\n", sizeof(5));
  exit(0);
}
