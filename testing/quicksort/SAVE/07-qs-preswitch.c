#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------ */
// #define STATIC_ARRAY
/* ------------------------------------------------------------------------ */
#define MAX_ARRAY	9
static int      array[MAX_ARRAY] = { 7, 12, 1, -2, 0, 15, 4, 11, 9 };
static int      orig[MAX_ARRAY];
static int      b[MAX_ARRAY];

/* ------------------------------------------------------------------------ */
#define swap(a,b)	{ int _t_ = a; a = b; b = _t_; }

/* ------------------------------------------------------------------------ */
static void printarray(const char *str, int a[], int l, int h)
{
    int             i;

    printf("%s:\n", str);
    for (i = l; i <= h; ++i)
    {
	printf(" %2d", i);
    }
    printf("\n");
    for (i = l; i <= h; ++i)
    {
	printf(" %2d", a[i]);
    }
    printf("\n");
}					/* End of printarray */

/* ------------------------------------------------------------------------ */
static int partition(int a[], int l, int h)
{
    int             pivot = a[l];
    int             left = l + 1;
    int             right = h;

// printf("entering partition l=%d  h=%d\n", l, h);
    do
    {
	while (a[left] <= pivot && left < h)
	{ left++; }

	while (a[right] > pivot)
	{ right--; }

	if (left >= right) { break; }
	swap(a[left], a[right]);
	left++;
	right--;
    } while (left <= right);
    swap(a[l], a[right]);
    return (right);
}					/* End of partition */

/* ------------------------------------------------------------------------ */
static void quickSort(int a[], int l, int r)
{
    int             p;

// printf("entering quickSort l=%d  r=%d\n", l, r);
#if 1
    switch (r - l)
    {
        case 0:
	    printf("%s:%u:%s Should not be in quick_sort_recursive with nothing to do.\n", __FILE__, __LINE__, __func__);
	    exit(1);

        case 1:
	    if (a[l] > a[r])
	    {
		swap(a[l], a[r]);
	    }
	    return;

#define TWO
#ifdef TWO
        case 2:         /* 3 to order. */
#undef s
#undef m
#undef h
#define s a[l]
#define m a[l+1]
#define h a[r]
            if (s <= m)
            {
                if (m <= h)     // order:  s <= m <= h
                { ; }		// do nothing
                else if (h < s) // order:  h < s <= m
                { p=s; s=h; h=m; m=p; }
                else // h > s   // order:  s <= h < m
                { swap(m, h); }
            }
            else        // s > m
            {
                if (h < m)      // order:  h m s
                { swap(h,s); }
                else if (s <= h)        // order:  m s h
                { swap(m,s); }
                else // s>m, h>m, s> h  // order:  m h s
                { p=s; s=m; m=h; h=p; }
            }
            return;
#undef s
#undef m
#undef h
#endif /* TWO */

	default: ;
    }
#endif /* 0 */

    p = partition(a, l, r);

    switch ((p - 1)-l)
    {
      case -1: break;
      case 0:  break;
      case 1: if (a[l] > a[p-1]) { swap(a[l], a[p-1]); }; break;
      default:
	quickSort(a, l, p - 1);
    }

#if 1
    switch (r-(p+1))
    {
      case -1: break;
      case 0:  break;
      case 1: if (a[p+1] > a[r]) { swap(a[p+1], a[r]); }; break;
      default:
	quickSort(a, p + 1, r);
    }
#endif /* 1 */
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

#ifdef STATIC_ARRAY
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    printarray("Unsorted array is:  ", array, 0, MAX_ARRAY - 1);

    for (z = 0; z < MAX_ARRAY; z++)
    {
	orig[z] = b[z] = array[z];
    }

    quickSort(array, 0, MAX_ARRAY - 1);

    printarray("  Sorted array is:  ", array, 0, MAX_ARRAY - 1);

    qsort(b, MAX_ARRAY, sizeof(int), compare);
    for (z = 0; z < MAX_ARRAY; z++)
    {
	if (array[z] != b[z])
	{
	    printf("qsort != quick_sort\n");
	    printarray("qsort array b", b, 0, MAX_ARRAY - 1);
	    exit(1);
	}
    }
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#else  // STATIC_ARRAY
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    int             i, j, k, l, m, n, o, p, q;

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
				    for (q = 0; q < MAX_ARRAY; q++) {
					orig[8] = q;
					memcpy(array, orig, sizeof(array));
					memcpy(b, orig, sizeof(array));

					quickSort(array, 0, MAX_ARRAY - 1);

					qsort(b, MAX_ARRAY, sizeof(int), compare);

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
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }
printf("\n"); fflush(stdout);
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#endif /* STATIC_ARRAY */
    exit(0);
}					/* End of main */

/* ------------------------------------------------------------------------ */