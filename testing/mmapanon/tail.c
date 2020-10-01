#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

int main(void)
{
    char *start;

#if 0
-rw-------  1 root root 268587008 Nov 24 16:40 tail.core
    start = mmap((void *)0x90000000, 256*1024*1024, PROT_READ|PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
#endif
#if 0
-rw-------  1 root root 268587008 Nov 24 16:42 tail.core
    start = mmap((void *)0x90000000, 256*1024*1024, PROT_READ|PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
#endif
#if 0
-rw-------  1 root root 268587008 Nov 24 16:43 tail.core
    start = mmap((void *)0x90000000, 256*1024*1024, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
#endif
#if 0
-rw-------  1 root root 268587008 Nov 24 16:45 tail.core
    start = mmap((void *)0x90000000, 256*1024*1024, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, 0, 0);
#endif
#if 0
-rw-------  1 root root 268587008 Nov 24 16:47 tail.core
    start = mmap((void *)0x90000000, 256*1024*1024, PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, 0, 0);
#endif

    start = mmap((void *)0x90000000, 256*1024*1024, PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS |
    MAP_PRIVATE | MAP_NORESERVE, -1, 0);
    fprintf(stderr, "start=%p\n", start);

    sprintf(start, "hi there\n");

    fprintf(stderr, "start=%p, %s\n", start, start);
    sleep(300);
    exit(0);
}
