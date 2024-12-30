/* $Id: raid5xorTest.c 156018 2011-05-27 16:01:36Z m4 $ */


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "sgl.h"



#define MM         (1 << 20)

#define S1_NUM_DESC  (4)
#define S2_NUM_DESC  (2)
#define S3_NUM_DESC  (3)
#define S4_NUM_DESC  (6)
#define S5_NUM_DESC  (4)
#define S6_NUM_DESC  (12)
#define S7_NUM_DESC  (3)
#define S8_NUM_DESC  (6)
#define D_NUM_DESC   (3)

// This needs to be managed carefully to ensure that
// the descriptor's buffers are multiples of 512 bytes.

#define SIZE       (12 * 51200)

#define MAX_SGL    (9)



SGL *createSgl (int size, int nDesc)
{
     SGL *p;
     SGL_DESC *d;
     int bufSize;
     int i;

     assert (nDesc > 0);
     assert (size > 0);
     assert (size % nDesc == 0);

     bufSize = size/nDesc;

     p = (SGL *)memalign (16, sizeof (SGL) + nDesc * (sizeof (SGL_DESC)));
     assert (p);

     p->size = sizeof (SGL) + nDesc * (sizeof (SGL_DESC));
     p->scnt = nDesc;

     d = (SGL_DESC *)(p + 1);

     for (i = 0; i < nDesc; i++, d++)
     {
          d->addr = (unsigned char *) memalign (16, bufSize);
          d->len  = bufSize;
          assert (d->addr);
     }

     return p;
}

void freeSgl (SGL *p)
{
     SGL_DESC *d;
     int bufSize;
     int i;
     int nDesc;

     nDesc = p->scnt;

     assert (p);
     assert (nDesc > 0);

     d = (SGL_DESC *)(p + 1);

     for (i = 0; i < nDesc; i++, d++)
     {
          assert (d->addr);
          free (d->addr);
     }

     free (p);
}




void fillSgl (SGL *p, unsigned char filler)
{
     SGL_DESC *d;
     int i;
     int j;
     int bufSize = p->size / p->scnt;

     d = (SGL_DESC *)(p + 1);

     for (i = 0; i < p->scnt; i++, d++)
     {
          for (j = 0; j < d->len; j++)
          {
               d->addr[j] = filler;
          }
     }
}


int checkSgl (SGL *p, unsigned char filler)
{
     SGL_DESC *d;
     int i;
     int j;
     int bufSize = p->size / p->scnt;
     int screwed = 0;

     d = (SGL_DESC *)(p + 1);

     for (i = 0; i < p->scnt; i++, d++)
     {
          for (j = 0; j < d->len; j++)
          {
               if (d->addr[j] != filler)
               {
                    screwed = 1;
                    fprintf(stderr, "ERROR! SGL value isn't %X, but %X: pSgl = %p, maxDesc = %d, nDesc = %d, offset = %d\n"
                            , filler
                            , d->addr[j]
                            , p
                            , p->scnt
                            , i
                            , j
                         );
                    break;

               }
          }
     }

     if (!screwed)
     {
          //printf ("Check Success! SGL contains only: %X\n", filler);
          return 1;
     }
     else
     {
          return 0;
     }

}





int test3sgl()
{
     SGL *s1;
     SGL *s2;
     SGL *d;
     int  retVal;

     s1 = createSgl (SIZE, S1_NUM_DESC);
     s2 = createSgl (SIZE, S2_NUM_DESC);
     d  = createSgl (SIZE, D_NUM_DESC);

     fillSgl (s1, 0xAA);
     fillSgl (s2, 0x55);
     fillSgl (d,  0x11);

     RL_XorSGL (3, d, s1, s2);


     retVal = checkSgl (d, 0xFF);

     freeSgl (s1);
     freeSgl (s2);
     freeSgl (d );

     return retVal;

}


int test4sgl()
{
     SGL *s1;
     SGL *s2;
     SGL *s3;
     SGL *d;
     int  retVal;


     s1 = createSgl (SIZE, S1_NUM_DESC);
     s2 = createSgl (SIZE, S2_NUM_DESC);
     s3 = createSgl (SIZE, S3_NUM_DESC);
     d  = createSgl (SIZE, D_NUM_DESC);

     fillSgl (s1, 0x11);
     fillSgl (s2, 0xAA);
     fillSgl (s3, 0x44);
     fillSgl (d,  0x11);

     RL_XorSGL (4, d, s1, s2, s3);

     retVal = checkSgl (d, 0xFF);

     freeSgl (s1);
     freeSgl (s2);
     freeSgl (s3);
     freeSgl (d );

     return retVal;
}


int test5sgl()
{
     SGL *s1;
     SGL *s2;
     SGL *s3;
     SGL *s4;
     SGL *d;
     int  retVal;


     s1 = createSgl (SIZE, S1_NUM_DESC);
     s2 = createSgl (SIZE, S2_NUM_DESC);
     s3 = createSgl (SIZE, S3_NUM_DESC);
     s4 = createSgl (SIZE, S4_NUM_DESC);
     d  = createSgl (SIZE, D_NUM_DESC);

     fillSgl (s1, 0x11);
     fillSgl (s2, 0x22);
     fillSgl (s3, 0x44);
     fillSgl (s4, 0x88);
     fillSgl (d,  0x11);

     RL_XorSGL (5, d, s1, s2, s3, s4);

     retVal = checkSgl (d, 0xFF);

     freeSgl (s1);
     freeSgl (s2);
     freeSgl (s3);
     freeSgl (s4);
     freeSgl (d );

     return retVal;

}

int test9sgl()
{
     SGL *s1;
     SGL *s2;
     SGL *s3;
     SGL *s4;
     SGL *s5;
     SGL *s6;
     SGL *s7;
     SGL *s8;
     SGL *d;
     int retVal;

     s1 = createSgl (SIZE, S1_NUM_DESC);
     s2 = createSgl (SIZE, S2_NUM_DESC);
     s3 = createSgl (SIZE, S3_NUM_DESC);
     s4 = createSgl (SIZE, S4_NUM_DESC);
     s5 = createSgl (SIZE, S5_NUM_DESC);
     s6 = createSgl (SIZE, S6_NUM_DESC);
     s7 = createSgl (SIZE, S7_NUM_DESC);
     s8 = createSgl (SIZE, S8_NUM_DESC);
     d  = createSgl (SIZE, D_NUM_DESC);

     fillSgl (s1, 0x1);
     fillSgl (s2, 0x2);
     fillSgl (s3, 0x4);
     fillSgl (s4, 0x8);
     fillSgl (s5, 0x10);
     fillSgl (s6, 0x20);
     fillSgl (s7, 0x40);
     fillSgl (s8, 0x80);
     fillSgl (d,  0x1);

     RL_XorSGL (9, d, s1, s2, s3, s4, s5, s6, s7, s8);

     retVal = checkSgl (d, 0xFF);

     freeSgl (s1);
     freeSgl (s2);
     freeSgl (s3);
     freeSgl (s4);
     freeSgl (s5);
     freeSgl (s6);
     freeSgl (s7);
     freeSgl (s8);
     freeSgl (d );

     return retVal;
}


int main ()
{
     int i;
     int okay = 1;

     for (i = 0; i < 1000; i++)
     {
          if (0
              || !test3sgl()
              || !test4sgl()
              || !test5sgl()
              || !test9sgl()
               )
          {
               fprintf(stderr, "SCREWED!\n");
               okay = 0;
               break;
          }
     }

     if (okay)
     {
          fprintf(stderr, "SUCCESS!\n");
     }

     return 0;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
