#include <stdio.h>

#define get_tsc()       ({ unsigned long long __scr; \
        __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})

int main()
{
	unsigned long long a = get_tsc();
	unsigned long long b = get_tsc();
	unsigned long long c = get_tsc();
	unsigned long long d = get_tsc();
	unsigned long long e = get_tsc();
	unsigned long long f = get_tsc();
	unsigned long long g = get_tsc();
	unsigned long long h = get_tsc();
	unsigned long long i = get_tsc();

	sleep(1);
	unsigned long long s1 = get_tsc();
	sleep(10);
	unsigned long long s10 = get_tsc();


	fprintf(stderr, " a=%llu\n", a);
	fprintf(stderr, " b=%llu\n", b);
	fprintf(stderr, " c=%llu\n", c);
	fprintf(stderr, " d=%llu\n", d);
	fprintf(stderr, " e=%llu\n", e);
	fprintf(stderr, " f=%llu\n", f);
	fprintf(stderr, " g=%llu\n", g);
	fprintf(stderr, " h=%llu\n", h);
	fprintf(stderr, " i=%llu\n", i);
	fprintf(stderr, " s1=%llu\n", s1);
	fprintf(stderr, " s10=%llu\n", s10);
	exit(0);
}
