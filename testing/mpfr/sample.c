#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>
#include <mpfr.h>

int main(void)
{
    unsigned int    i;

    mpfr_t          s;
    mpfr_t          t;
    mpfr_t          u;

    mpfr_init2(t, 200);
    mpfr_init2(s, 200);
    mpfr_init2(u, 200);

    mpfr_set_d(t, 1.0, MPFR_RNDD);
    mpfr_set_d(s, 1.0, MPFR_RNDD);

    for (i = 1; i <= 100; i++)
    {
	mpfr_mul_ui(t, t, i, MPFR_RNDU);
	mpfr_set_d(u, 1.0, MPFR_RNDD);
	mpfr_div(u, u, t, MPFR_RNDD);
	mpfr_add(s, s, u, MPFR_RNDD);
    }
    printf("Sum is ");
    mpfr_out_str(stdout, 10, 0, s, MPFR_RNDD);
    putchar('\n');

    mpfr_clear(s);
    mpfr_clear(t);
    mpfr_clear(u);

    mpfr_free_cache();
    exit(0);
}

// The result of this program is:
// 
// $ ./sample
// Sum is 2.7182818284590452353602874713526624977572470936999595749669131e0
