#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/* ------------------------------------------------------------------------ */
// #define STATIC_ARRAY
#if !defined(STATIC_ARRAY)
   #define STATIC_ALL
#endif
#if !defined(STATIC_ARRAY) && !defined(STATIC_ALL)
  #define STATIC_RANDOM
  #define MAX_ARRAY	100000000
#else
//  #define MAX_ARRAY	9
  #define MAX_ARRAY	8
#endif
/* ------------------------------------------------------------------------ */
static int      array[MAX_ARRAY];
static int      orig[MAX_ARRAY];
static int      b[MAX_ARRAY];

/* ------------------------------------------------------------------------ */
static struct timeval _start_time_;
static struct timeval _end_time_;
static struct timeval _limited_;

#define START_TIMING	gettimeofday(&_start_time_, NULL);
#define STOP_TIMING	{ gettimeofday(&_end_time_, NULL); \
                          timersub(&_end_time_, &_start_time_, &_limited_); }

/* ------------------------------------------------------------------------ */
#undef timeradd
#define timeradd(tvp, uvp, vvp)                                         \
        do {                                                            \
                (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;          \
                (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;       \
                if ((vvp)->tv_usec >= 1000000) {                        \
                        (vvp)->tv_sec++;                                \
                        (vvp)->tv_usec -= 1000000;                      \
                }                                                       \
        } while (0)
#undef timersub
#define timersub(tvp, uvp, vvp)                                         \
        do {                                                            \
                (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;          \
                (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;       \
                if ((vvp)->tv_usec < 0) {                               \
                        (vvp)->tv_sec--;                                \
                        (vvp)->tv_usec += 1000000;                      \
                }                                                       \
        } while (0)

/* ------------------------------------------------------------------------ */
#undef swap
#define swap(a,b)	{ int _t_ = a; a = b; b = _t_; }

#undef swap3
#define swap3(s,m,h)	{					\
	    if (s <= m)						\
	    {							\
		if (m <= h)     /* order:  s <= m <= h */	\
		{ ; }		/* do nothing */		\
		else if (h < s) /* order:  h < s <= m */	\
		{ int _tmp_=s; s=h; h=m; m=_tmp_; }		\
		else /* h > s      order:  s <= h < m */	\
		{ swap(m, h); }					\
	    }							\
	    else        /* s > m */				\
	    {							\
		if (h < m)      /* order:  h m s */		\
		{ swap(h,s); }					\
		else if (s <= h)        /* order:  m s h */	\
		{ swap(m,s); }					\
		else /* s>m, h>m, s> h     order:  m h s */	\
		{ int _tmp_=s; s=m; m=h; h=_tmp_; }		\
	    }							\
	}

/* ------------------------------------------------------------------------ */
#if defined(STATIC_RANDOM)
static uint32_t random_state[64];

static void init_random(void)
{
    unsigned int    i;
    unsigned int    seed;
    struct timespec time_now;

    for (i = 0; i < sizeof(random_state) / sizeof(random_state[0]); i += 2)
    {
	clock_gettime(CLOCK_REALTIME, &time_now);
	random_state[i + 1] = time_now.tv_sec;
	random_state[i] = time_now.tv_nsec;
    }

    seed = 1;
    i = 256;
    initstate(seed, (char *)random_state, i);
    setstate((char *)random_state);
    srandom((unsigned int)time_now.tv_nsec);
}					/* End of init_random */

/* ------------------------------------------------------------------------ */
static long get_random(int random_max)
{
    long            r = random();
    long            s = ((double)r / (double)0x7fffffff) * (double)random_max;
    return (s);
}					/* End of get_random */
#endif /* STATIC_RANDOM */

/* ------------------------------------------------------------------------ */
static void printarray(const char *str, int a[], int lo, int hi)
{
    int             i;

    printf("%s:\n", str);
    for (i = lo; i <= hi; ++i)
    {
	printf(" %2d", i);
    }
    printf("\n");
    for (i = lo; i <= hi; ++i)
    {
	printf(" %2d", a[i]);
    }
    printf("\n");
}					/* End of printarray */

/* ------------------------------------------------------------------------ */
static int partition(int a[], int lo, int hi)
{
    int             pivot = a[lo];
    int             left = lo + 1;
    int             right = hi;

    do
    {
	while (a[left] <= pivot)
	{ left++;
	  if (left >= right) break;
	}

	while (a[right] > pivot)
	{ right--; }

	if (left < right)
	{
	    swap(a[left], a[right]);
	    left++;
	    right--;
	}
    } while (left <= right);
    swap(a[lo], a[right]);
    return (right);
}					/* End of partition */

/* ------------------------------------------------------------------------ */
static void quickSort(int a[], int lo, int hi)
{
    int             p;

    p = partition(a, lo, hi);

    switch ((p - 1) - lo)
    {
	case -1: break;
	case 0:  break;
	case 1:  if (a[lo] > a[p - 1]) { swap(a[lo], a[p - 1]); }; break;
	case 2:  swap3(a[lo], a[lo + 1], a[p - 1]); break;
	default: quickSort(a, lo, p - 1);
    }

    switch (hi - (p + 1))
    {
	case -1: break;
	case 0:  break;
	case 1:  if (a[p + 1] > a[hi]) { swap(a[p + 1], a[hi]); }; break;
	case 2:  swap3(a[p + 1], a[p + 2], a[hi]); break;
	default: quickSort(a, p + 1, hi);
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

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#ifdef STATIC_ARRAY
    array[0] =   7;
    array[1] =  12;
    array[2] =   1;
    array[3] =  -2;
    array[4] =   0;
    array[5] =  15;
    array[6] =   4;
    array[7] =  11;
#if MAX_ARRAY>8
    array[8] =   9;
#endif	/* 9 */
    printarray("Unsorted array is:  ", array, 0, MAX_ARRAY - 1);

    for (z = 0; z < MAX_ARRAY; z++)
    {
        orig[z] = b[z] = array[z];
    }

    START_TIMING;
    quickSort(array, 0, MAX_ARRAY - 1);
    STOP_TIMING;
    printf("quickSort - Time used %d.%06d\n", (int)_limited_.tv_sec, (int)_limited_.tv_usec);

    printarray("  Sorted array is:  ", array, 0, MAX_ARRAY - 1);

    START_TIMING;
    qsort(b, MAX_ARRAY, sizeof(int), compare);
    STOP_TIMING;
    printf("    qsort - Time used %d.%06d\n", (int)_limited_.tv_sec, (int)_limited_.tv_usec);
    for (z = 0; z < MAX_ARRAY; z++)
    {
        if (array[z] != b[z])
        {
            printf("qsort != quick_sort\n");
            printarray("qsort array b", b, 0, MAX_ARRAY - 1);
            exit(1);
        }
    }
#endif // STATIC_ARRAY
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#ifdef STATIC_ALL
    int             i, j, k, l, m, n, o, p;
#if MAX_ARRAY>8
    int             q;
#endif	/* 9 */
    struct timeval qs_time = {0,0};
    struct timeval qsort_time = {0,0};

    for (i = 0; i < MAX_ARRAY; i++) {
        orig[0] = i;
        for (j = 0; j < MAX_ARRAY; j++) {
printf("."); fflush(stdout);
            orig[1] = j;
            for (k = 0; k < MAX_ARRAY; k++) {
                orig[2] = k;
                for (l = 0; l < MAX_ARRAY; l++) {
                    orig[3] = l;
                    for (m = 0; m < MAX_ARRAY; m++) {
                        orig[4] = m;
                        for (n = 0; n < MAX_ARRAY; n++) {
                            orig[5] = n;
                            for (o = 0; o < MAX_ARRAY; o++) {
                                orig[6] = o;
                                for (p = 0; p < MAX_ARRAY; p++) {
                                    orig[7] = p;
#if MAX_ARRAY>8
                                    for (q = 0; q < MAX_ARRAY; q++) {
                                        orig[8] = q;
#endif	/* 9 */
                                        memcpy(array, orig, sizeof(array));
                                        memcpy(b, orig, sizeof(array));

// printarray("\nUnsorted array", orig, 0, MAX_ARRAY - 1);
    START_TIMING;
                                        quickSort(array, 0, MAX_ARRAY - 1);
    STOP_TIMING;
    timeradd(&qs_time, &_limited_, &qs_time);

    START_TIMING;
                                        qsort(b, MAX_ARRAY, sizeof(int), compare);
    STOP_TIMING;
    timeradd(&qs_time, &_limited_, &qsort_time);

                                        for (z = 0; z < MAX_ARRAY; z++)
                                        {
                                            if (array[z] != b[z])
                                            {
                                                printarray("Unsorted array", orig, 0, MAX_ARRAY - 1);
                                                printarray("  Sorted array", array, 0, MAX_ARRAY - 1);
                                                printf("qsort != quick_sort\n");
                                                printarray("qsort array b", b, 0, MAX_ARRAY - 1);
                                                exit(1);
                                            }
                                        }
#if MAX_ARRAY>8
                                    }
#endif	/* 9 */
                                }
                            }
                        }
                    }
                }
            }
        }
    }
printf("\n"); fflush(stdout);
    printf("quickSort - Time used %d.%06d\n", (int)qs_time.tv_sec, (int)qs_time.tv_usec);
    printf("    qsort - Time used %d.%06d\n", (int)qsort_time.tv_sec, (int)qsort_time.tv_usec);
#endif /* STATIC_ALL */
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#ifdef STATIC_RANDOM
    init_random();

    for (z = 0; z < MAX_ARRAY; z++)
    {
	orig[z] = b[z] = array[z] = get_random(MAX_ARRAY * 2);
    }

    START_TIMING;
    quickSort(array, 0, MAX_ARRAY - 1);
    STOP_TIMING;
    printf("quickSort - Time used %d.%06d\n", (int)_limited_.tv_sec, (int)_limited_.tv_usec);

    START_TIMING;
    qsort(b, MAX_ARRAY, sizeof(int), compare);
    STOP_TIMING;
    printf("    qsort - Time used %d.%06d\n", (int)_limited_.tv_sec, (int)_limited_.tv_usec);
    for (z = 0; z < MAX_ARRAY; z++)
    {
	if (array[z] != b[z])
	{
	    printarray("Unsorted array is:  ", array, 0, MAX_ARRAY - 1);
	    printarray("  Sorted array is:  ", array, 0, MAX_ARRAY - 1);
	    printf("qsort != quick_sort\n");
	    printarray("qsort array b", b, 0, MAX_ARRAY - 1);
	    exit(1);
	}
    }
#endif /* STATIC_RANDOM */

    exit(0);
}					/* End of main */

/* ------------------------------------------------------------------------ */
