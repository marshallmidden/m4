#include <stdio.h>
#include <unistd.h>

#define	get_tsc()	({ unsigned long long __scr; \
	__asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})
#define get_tsc_l()	(get_tsc & 0xffffffff)

int	main(int argc, char *argv[])
{
	unsigned long long	start, end;

	printf("Starting...");
	start = get_tsc();
	sleep(1);
	end = get_tsc();
	printf("time=%lld\n", start);
	printf("Ending.....");
	printf("time=%lld\n", end);
	printf("Difference = %lld\n", (end - start));
	return 0;
}
