#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* ------------------------------------------------------------------------ */

/* primes 2 * 3 * 5 (30) handled in main loop. */
#define	LOOP_START 	 30		/* Main loop increment and offset. */
#define TABLE_ENTER       7		/* Starting entering primes at this number into table. */

#define NUMBERS_IN_COLUMN 8
static int      widthcount;		/* Width of line for printing. */

#define	PRIME_TYPE	unsigned int
static PRIME_TYPE	TABLE_MAX;
static struct table
{
    PRIME_TYPE      double_number;
    PRIME_TYPE      next_non_prime;
}              *prime_table;
static PRIME_TYPE inc_table = 0;	/* Nothing in table yet. */

/* ------------------------------------------------------------------------ */

/* Print the prime number, and if beyond loop knowledge, enter into table. */
static inline void p(PRIME_TYPE i)
{
    if (widthcount < NUMBERS_IN_COLUMN - 1)
    {
	printf("%10d ", i);
	widthcount++;
    }
    else
    {
	printf("%10d\n", i);
	widthcount = 0;
    }

    if (i < TABLE_ENTER)		/* Ignore the few loop handles. */
    {
	return;
    }
    if (inc_table + 1 >= TABLE_MAX)	/* If table is full. */
    {
	return;
    }
    prime_table[inc_table].double_number = i * 2;
    prime_table[inc_table].next_non_prime = i * i;
    inc_table++;
}

/* ------------------------------------------------------------------------ */
static void is_prime(PRIME_TYPE x)
{
    int             i;
    int             flag = 1;

    for (i = 0; i < inc_table; i++)
    {
	while (prime_table[i].next_non_prime <= x)	// Not prime
	{
	    if (prime_table[i].next_non_prime == x)
	    {
		flag = 0;
	    }
	    prime_table[i].next_non_prime += prime_table[i].double_number;
	}
    }
    if (flag)
    {
	p(x);
    }
}

/* ------------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    PRIME_TYPE      i;
    PRIME_TYPE      m;
    double	    dm;

    if (argc != 2)
    {
	fprintf(stderr, "error, argc(%d) != 1\n", argc - 1);
	exit(1);
    }
    m = atoi(argv[1]);
    dm = m;

    TABLE_MAX = sqrt(dm) + 1;
    prime_table = malloc(sizeof(struct table) * TABLE_MAX);
//    fprintf(stderr, "m=%d\n", m);
//    exit (0);

    printf("primes, 1 to %d:\n", m);
    widthcount = 0;
    /* Prime the tables, ignore less than TABLE_ENTER. */
    p(1);
    p(2);
    p(3);
    p(5);

    p(7);
    p(11);
    p(13);
    p(17);

    p(19);
    p(23);
    p(29);
//    printf("%10d\n", 31);

    i = LOOP_START;
    while (i < m)
    {
	is_prime(i + 1);
	// 2
	// 3
	// 4
	// 5
	// 6
	is_prime(i + 7);
	// 8
	// 9
	// 10
	is_prime(i + 11);
	// 12
	is_prime(i + 13);
	// 14
	// 15
	// 16
	is_prime(i + 17);
	// 18
	is_prime(i + 19);
	// 20
	// 21
	// 22
	is_prime(i + 23);
	// 24
	// 25
	// 26
	// 27
	// 28
	is_prime(i + 29);
	// 30
	i += LOOP_START;
    }
    if (widthcount != 0)
    {
	printf("\n");
    }
    return 0;
}

/* ------------------------------------------------------------------------ */
