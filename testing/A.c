#include <stdio.h>

main()
{
    fprintf(stderr, "Hello World\n");
    unsigned long long a = (1ULL)<<31;
    unsigned long long b = (1ULL)<<22;
    unsigned long long c = (1ULL)<<11;
    fprintf(stderr, "a=0x%016llx  b=0x%016llx  c=0x%016llx\n", a, b, c);
    fprintf(stderr, "a=%lld  b=%lld  c=%lld\n", a, b, c);
    fprintf(stderr, "a=0x%016llx  b=0x%016llx  c=0x%016llx\n", a>>11, b>>11, c>>11);
    fprintf(stderr, "a=%lld  b=%lld  c=%lld\n", a>>11, b>>11, c>>11);
    fprintf(stderr, "a=%lld\n", (1ULL)<<11);

    fprintf(stderr, "drb_id(2) << 18 | 0x1000000 = %lx\n", 2<<18|0x1000000);
}
