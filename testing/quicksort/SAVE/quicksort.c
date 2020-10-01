#include <stdio.h>
#include <stdlib.h>

/* ------------------------------------------------------------------------ */
// #define DEBUG
// #define STATIC_ARRAY

/* ------------------------------------------------------------------------ */
#define MAX_ARRAY	9

/* ------------------------------------------------------------------------ */
#define swap2(a,b)	{ int _t_ = a; a = b; b = _t_; }

/* ------------------------------------------------------------------------ */
static int      level = 0;

#define LEVEL   level,level,"           "
#define INDENT  "%*.*s"

/* ------------------------------------------------------------------------ */
static void printarray(const char *str, int a[MAX_ARRAY], int start, int end)
{
    int             i;

    printf(INDENT "%s:\n" INDENT, LEVEL, str, LEVEL);

    for (i = start; i <= end; i++)
    {
	printf(" %2d", i);
    }
    printf("\n" INDENT, LEVEL);
    for (i = start; i <= end; i++)
    {
	printf(" %2d", a[i]);
    }
    printf("\n");
}					/* End of printarray */

/* ------------------------------------------------------------------------ */
static int partition(int a[MAX_ARRAY], int lo, int hi)
{
    int             pivot = a[lo];
    int             i = lo + 1;
    int             j = hi;

#ifdef DEBUG
char            str[BUFSIZ];
snprintf(str, BUFSIZ, "Entering partition lo=%d, hi=%d) pivot=%d i=%d j=%d", lo, hi, pivot, i, j);
printarray(str, a, lo, hi);
#endif /* DEBUG */

    for (;;)
    {
#ifdef DEBUG
printf(INDENT "start loop: i=%d (%d)  j=%d (%d)\n", LEVEL, i, a[i], j, a[j]);
#endif /* DEBUG */
	while (a[i] <= pivot && i <= hi)
	{
	    i++;
#ifdef DEBUG
printf("   i++ => %d\n", i);
#endif /* DEBUG */
	}

	while (a[j] > pivot)
	{
	    j--;
#ifdef DEBUG
printf("   j-- => %d\n", j);
#endif /* DEBUG */
	}

	if (i >= j)
	{
#ifdef DEBUG
printf("   i(%d) >= j(%d)\n", i, j);
#endif /* DEBUG */
	    break;
	}
#ifdef DEBUG
snprintf(str, BUFSIZ, "swap#1 i=%d (%d) and j=%d (%d)", i, a[i], j, a[j]);
#endif /* DEBUG */
	swap2(a[i], a[j]);
	i++;
	j--;
#ifdef DEBUG
printarray(str, a, lo, hi);
#endif /* DEBUG */
    }

#ifdef DEBUG
snprintf(str, BUFSIZ, "swap#2 i=%d (%d) and hi=%d (%d) -- j=%d (%d)", i, a[i], hi, a[hi], j, a[j]);
#endif /* DEBUG */
    swap2(a[lo], a[j]);
#ifdef DEBUG
printarray(str, a, lo, hi);
#endif /* DEBUG */

    return (j);
}					/* End of partition */

/* ------------------------------------------------------------------------ */
static void quickSort(int a[MAX_ARRAY], int lo, int hi)
{
    int             p;

    switch (hi - lo)
    {
	case 0:
	    printf("%s:%u:%s Should not be in quick_sort_recursive with nothing to do.\n",
		   __FILE__, __LINE__, __func__);
	    exit(1);

	case 1:
	    if (a[lo] > a[hi])
	    {
#ifdef DEBUG
printf("swap lo and hi\n");
#endif /* DEBUG */
		swap2(a[lo], a[hi]);
	    }
#ifdef DEBUG
else { printf("do not swap lo and hi\n"); }
#endif /* DEBUG */
	    return;

//#define TWO
#ifdef TWO
	case 2:		/* 3 to order. */
#undef l
#undef m
#undef h
#define l a[lo]
#define m a[lo+1]
#define h a[hi]
	    if (l <= m)
	    {
	        if (m <= h)	// order:  l <= m <= h
		{
		    ;		// do nothing
		}
		else if (h < l)	// order:  h < l <= m
		{
		    p=l; l=h; h=m; m=p;
		}
		else // h > l	// order:  l <= h < m
		{
		    swap2(m, h);
		}
	    }
	    else 	// l > m
	    {
	        if (h < m)	// order:  h m l
		{
		    swap2(h,l);
		}
		else if (l <= h)	// order:  m l h
		{
		    swap2(m,l);
		}
		else // l>m, h>m, l> h 	// order:  m h l
		{
		    p=l; l=m; m=h; h=p;
		}
	    }
	    return;
#undef l
#undef m
#undef h
#endif /* TWO */

	default:
	    break;
    }

    p = partition(a, lo, hi);
#ifdef DEBUG
printf(INDENT "before lo recursive p=%d lo=%d hi=%d\n", LEVEL, p, lo, hi);
#endif /* DEBUG */
    if (lo < (p - 1))
    {
	level += 2;
	quickSort(a, lo, p - 1);
	level -= 2;
    }

#ifdef DEBUG
printf(INDENT "before hi recursive p=%d lo=%d hi=%d\n", LEVEL, p, lo, hi);
#endif /* DEBUG */
    if ((p + 1) < hi)
    {
	level += 2;
	quickSort(a, p + 1, hi);
	level -= 2;
    }
}					/* End of quickSort */

/* ------------------------------------------------------------------------ */
static int compare(const void *A, const void *B)
{
    return (*(const int *)A - *(const int *)B);
}					/* End of compare */

/* ------------------------------------------------------------------------ */
int main(void)
{
    int             z;

    int             orig[MAX_ARRAY];
    int             a[MAX_ARRAY]
#ifdef STATIC_ARRAY
    = { 7, 12, 1, -2, 0, 15, 4, 11, 9 }
#endif /* STATIC_ARRAY */
    ;
    int             b[MAX_ARRAY];

#ifndef STATIC_ARRAY
    int             i, j, k, l, m, n, o, p, q;

    for (i = 0; i < MAX_ARRAY; i++)	// 0
    {
	for (j = 0; j < MAX_ARRAY; j++)	// 1
	{
printf("."); fflush(stdout);
	    for (k = 0; k < MAX_ARRAY; k++)	// 2
	    {
		for (l = 0; l < MAX_ARRAY; l++)	// 3
		{
		    for (m = 0; m < MAX_ARRAY; m++)	// 4
		    {
			for (n = 0; n < MAX_ARRAY; n++)	// 5
			{
			    for (o = 0; o < MAX_ARRAY; o++)	// 6
			    {
				for (p = 0; p < MAX_ARRAY; p++)	// 7
				{
				    for (q = 0; q < MAX_ARRAY; q++)	// 8
				    {
					a[0] = i; a[1] = j; a[2] = k; a[3] = l; a[4] = m;
					a[5] = n; a[6] = o; a[7] = p; a[8] = q;
#endif /* !STATIC_ARRAY */
					for (z = 0; z < MAX_ARRAY; z++)
					{
					    orig[z] = b[z] = a[z];
					}

#ifdef DEBUG
                                        printarray("Unsorted array", a, 0, MAX_ARRAY-1);
#endif /* DEBUG */

					quickSort(a, 0, MAX_ARRAY - 1);

#ifdef DEBUG
                                        printarray("  Sorted array", a, 0, MAX_ARRAY-1);
#endif /* DEBUG */


					qsort(b, MAX_ARRAY, sizeof(int), compare);

					for (z = 0; z < MAX_ARRAY; z++)
					{
					    if (a[z] != b[z])
					    {
#ifndef DEBUG
						printarray("Unsorted array", orig, 0, MAX_ARRAY - 1);
						printarray("  Sorted array", a, 0, MAX_ARRAY-1);
#endif /* DEBUG */
						printf("qsort != quick_sort\n");
						printarray("qsort array b", b, 0,
							   MAX_ARRAY - 1);
						exit(1);
					    }
					}
#ifndef STATIC_ARRAY
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }
#endif /* !STATIC_ARRAY0 */
}					/* End of main */

/* ------------------------------------------------------------------------ */
