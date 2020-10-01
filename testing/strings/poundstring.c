#include <stdio.h>
#include <string.h>

#define abc(z)	fprintf(stderr, "integer z=%d\n", z);
#define def(z)	fprintf(stderr, "string z=%s integer z=%d\n", #z, z);

#define ghi(...)	fprintf(stderr, "string %s\n",  #__VA_ARGS__); __VA_ARGS__ ; fprintf(stderr, "buf=%s\n", buf)

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    const char *a;
    char buf[100];

    abc(123);
    def(456);
//    ghi(a,b,c);

//    ghi(strncpy(a,b,c));
//    ghi(strncpy(a,b,c););
    a = "Hello";
    ghi(strncpy(buf,a,sizeof(buf)););
}
