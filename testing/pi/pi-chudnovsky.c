/*
    Calculate Pi using the "mpfr" (Multi-Precision Floating with Rounding)
    GNU library and the Chudnovsky algorithm published in 1988 and recently
    used to calculate Pi to 31.4 trillion digits from September 2018 thru
    January 2019.

    Interestingly, that means 4 terabytes of memory for one variable, and
    many such are needed. Yet -- with analysis of the patterns within the
    various fractions, compromise and precision can be maintained by using
    less bits per variable. That is an exercise left to the reader. :)
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpfr.h>
/* ------------------------------------------------------------------------ */
#define bits10(x)	(int)((x * 3.3333333333) +1)	// for x digits, in bits.
/* ------------------------------------------------------------------------ */
static void PI(mpfr_t result, unsigned long maxK, unsigned long prec)
{
    unsigned long long i;		// For index loop.
    unsigned long long tmp;
    mpfr_t          K;			// *sigh* Unlikely to overflow an unsigned long.
    mpfr_t          M;			// Definition for variable M.
    mpfr_t          L;
    mpfr_t          X;
    mpfr_t          S;
    mpfr_t          final;
    mpfr_t          bigX;		// -262537412640768000
    mpfr_t          temp1;		// internal result
    mpfr_t          temp2;		// internal result

    mpfr_init2(K, bits10(prec));	// Initialize for prec bits of information.
    mpfr_init2(M, bits10(prec));
    mpfr_init2(L, bits10(prec));
    mpfr_init2(X, bits10(prec));
    mpfr_init2(S, bits10(prec));

    mpfr_init2(final, bits10(prec));
    mpfr_init2(bigX, bits10(prec));

    mpfr_init2(temp1, bits10(prec));
    mpfr_init2(temp2, bits10(prec));

    mpfr_set_ui(K, 6, MPFR_RNDD);
    mpfr_set_ui(M, 1, MPFR_RNDD);
    mpfr_set_ui(L, 13591409, MPFR_RNDD);
    mpfr_set_ui(X, 1, MPFR_RNDD);
    mpfr_set_ui(S, 13591409, MPFR_RNDD);

    mpfr_sqrt_ui(final, 10005, MPFR_RNDD);	// sqrt(10005.0)
    mpfr_mul_ui(final, final, 426880, MPFR_RNDD);	// (426880 * sqrt(10005.0))
    mpfr_set_ld(bigX, -262537412640768000.0, bits10(prec));

    for (i = 1; i <= maxK; i++)
    {
// M = (K**3 - 16*K) * M // i**3 ;
	mpfr_mul(temp1, K, K, MPFR_RNDD);	// K**2
	mpfr_mul(temp1, temp1, K, MPFR_RNDD);	//   K**3
	mpfr_mul_ui(temp2, K, 16, MPFR_RNDD);	// 16*K
	mpfr_sub(temp1, temp1, temp2, MPFR_RNDD);	// (K**3 - 16*K)
	mpfr_mul(temp1, temp1, M, MPFR_RNDD);	// (K**3 - 16*K) * M
	tmp = i * i * i;		// i**3
	mpfr_div_ui(temp1, temp1, tmp, MPFR_RNDD);	// (K**3 - 16*K) * M / i**3
	mpfr_floor(M, temp1);		// floor is //.
// L += 545140134;
	mpfr_add_ui(L, L, 545140134, MPFR_RNDD);	// L += 545140134
// X *= -262537412640768000;
	mpfr_mul(X, X, bigX, MPFR_RNDD);	// X *= -262537412640768000
// S += Dec(M * L) / X;
	mpfr_mul(temp1, M, L, MPFR_RNDD);	// Dec(M * L)
	mpfr_div(temp1, temp1, X, MPFR_RNDD);	// Dec(M * L) / X
	mpfr_add(S, S, temp1, MPFR_RNDD);	// S += Dec(M * L) / X
// K += 12;
	mpfr_add_ui(K, K, 12, MPFR_RNDD);	// K += 12
    }
// result = 426880 * Dec(10005).sqrt() / S;
    mpfr_div(result, final, S, MPFR_RNDD);	// (426880 * sqrt(10005.0)) / S

    mpfr_printf("PI(maxK=%ld iterations, precision=%ld, digits=\n%*.*RNf\n",
		maxK, prec, prec, prec, result);

    mpfr_clear(K);
    mpfr_clear(M);
    mpfr_clear(L);
    mpfr_clear(X);
    mpfr_clear(S);

    mpfr_clear(final);
    mpfr_clear(bigX);

    mpfr_clear(temp1);
    mpfr_clear(temp2);
}   /* End of PI */

/* ------------------------------------------------------------------------ */
int main(int argc, char **argv)
{
    int             arg_one = argc;
    mpfr_t          Pi_1;

    if (argc <= 1)
    {
	// Parameters chosen to gain 1000+ digits within a few seconds
	arg_one = 1000;
    }
    else
    {
	arg_one = atoi(argv[1]);
    }
    mpfr_init2(Pi_1, arg_one);		// Precision (bits) is second argument.
    PI(Pi_1, arg_one, arg_one);
    exit(0);
}   /* End of main */

/* ------------------------------------------------------------------------ */
/* End of file pi-chundnovsky.c */
