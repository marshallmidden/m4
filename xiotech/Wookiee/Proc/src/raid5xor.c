/* $Id: raid5xor.c 156018 2011-05-27 16:01:36Z m4 $ */

/******************************************************************/
/* This file has functions required for RAID5 XOR only.           */
/*                                                                */
/* The following functions are exported by this file:             */
/* 1. RL_XorSGL()                                                 */
/* 2. RL_CompareSGL()                                             */
/* 3. RL_FastMemCmp()                                             */
/*                                                                */
/* Terminology: xorN means N-1 inputs are XORed to get            */
/* 1 output (which could be one of the                            */
/* inputs.)                                                       */
/*                                                                */
/* The fundamental idea is to derive the following:               */
/* result = p1 ^ p2 ^ ... ^ pN;                                   */
/*                                                                */
/* For this, there are two cases:                                 */
/* 1. dest is same as one of the source buffers.                  */
/* 2. dest is different from the source buffers.                  */
/*                                                                */
/* In case 1, we should NOT use 'movntps' or any other            */
/* write combining approach because the data is already           */
/* in the cache and we should take advantage of this fact.        */
/*                                                                */
/* In case 2, we have non-temporal data (used once only),         */
/* it is best to write the result directly into memory            */
/* without polluting the cache.                                   */
/*                                                                */
/* For this reason, the functions start with a check to           */
/* determine which type of write should be used for               */
/* storing the result.                                            */
/*                                                                */
/******************************************************************/

#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "sgl.h"
#include "raid5xor.h"

#define SGL_XOR_DEBUG       1
#define SGL_XOR_PRINT       0
#define SGL_XOR_PRINT_FREQ  100

// Stop the assert macro from expanding
// if we are not in DEBUG mode.

#if !SGL_XOR_DEBUG
#define NDEBUG
#include <assert.h>
#endif

#define ASSERT      assert
#define MAX_SGL    (9)

#undef REG
#undef LD
#undef STNT
#undef STT
#undef XOR
#undef MFENCE
#undef SFENCE
#undef PF

// two levels are needed for recursive macro substitution.
#define REG(x) #x
#define LD(offset,x,y)        "movaps "#offset""REG (x)", %%"#y"      ;\n"
// STNT == store non temporat, STT == store temporal
#define STNT(y,offset,x)      "movntps %%"#y", "#offset""REG (x)"     ;\n"
#define STT(y,offset,x)       "movaps %%"#y", "#offset""REG (x)"      ;\n"
#define XOR(offset,x,y)       "xorps  "#offset""REG (x)", %%"#y"      ;\n"
#define MFENCE()              "mfence                                ;\n"
#define SFENCE()              "sfence                                ;\n"
#define PF(offset,x)          "prefetchnta "#offset""REG (x)"         ;\n"

// based on the remaining len in the buffer, get the location where the next xor should start.
#define SGL_BUF_OFFSET(desc,remLen)  (((unsigned char *) ((desc)->addr)) + (desc)->len - (remLen))


void xor3 (unsigned long   size
                  , SGL_DESC    **ppDesc
                  , int          *remLen);

void xor4 (unsigned long   size
                  , SGL_DESC    **ppDesc
                  , int          *remLen);

void xor5 (unsigned long   size
                  , SGL_DESC    **ppDesc
                  , int          *remLen);

void xor9 (unsigned long   size
                  , SGL_DESC    **ppDesc
                  , int          *remLen);

/* inline */ int getMin (int size, int *a);
int getSglBufSize (SGL *p);



// Non-Temporal data: result != one of the inputs.
// Temporal data:     result == one of the inputs.

typedef void (*XOR_FN_PTR)(unsigned long size, SGL_DESC **ppDesc, int *remLen);

XOR_FN_PTR xorTbl[] =
{
     (XOR_FN_PTR) 0,
     (XOR_FN_PTR) 1,
     (XOR_FN_PTR) 2,
     xor3,
     xor4,
     xor5,
     (XOR_FN_PTR) 6,
     (XOR_FN_PTR) 7,
     (XOR_FN_PTR) 8,
     xor9
};


/*************************************************************/
/* Functions begin here: Each has some #defines that         */
/* it undefines later                                        */
/*************************************************************/

void validateSGLs (int nSgl, SGL **ppSGL);

// first parameter is the dest sgl; nSgl has number of params
void RL_XorSGL (int nSgl, ...)
{
     int i;
     int minLen;
     int len;
     int lenDone;
     va_list ap;

     SGL        *ppSGL  [MAX_SGL];
     SGL_DESC   *ppDesc [MAX_SGL];
     int         remLen[MAX_SGL];     // remaining len in each SGL Desc.


     ASSERT ((nSgl == 3) || (nSgl == 4) || (nSgl == 5) || (nSgl == 9));

     va_start (ap, nSgl);

     for (i = 0; i < nSgl; i++)
     {
          ppSGL[i] = va_arg (ap, SGL *);

          ASSERT (ppSGL[i]);

          ppDesc[i] = (SGL_DESC *) (ppSGL[i] + 1);
          remLen[i] = ppDesc[i]->len;
     }

     // Disabling the asserts would still leave the for loops intact.
     // Get rid of them by defining this debug value to zero.

#if SGL_XOR_DEBUG

     validateSGLs (nSgl, ppSGL);

#endif // SGL_XOR_DEBUG


     len = getSglBufSize (ppSGL[0]);
     lenDone = 0;

     // while more work needs to be done...
     do
     {
          // if any of the descriptors are finished (at least one is guaranteed to be
          // exhausted each time except during the first time), go on to the next one...
          // Don't forget to update the remaining len for that desc!

          for (i = 0; i < nSgl; i++)
          {
               if (remLen[i] <= 0)
               {
                    ASSERT (remLen[i] == 0);
                    ppDesc[i]++;
                    remLen[i] = ppDesc[i]->len;
                    ASSERT (remLen[i] % 512 == 0);
               }
          }

          // get the min common len value and perform xor on those regions.
          minLen = getMin (nSgl, remLen);
          ASSERT (minLen > 0);

          // call the XOR function from the table

          xorTbl[nSgl]((unsigned)minLen, ppDesc, remLen);


          // update rem values so that they reflect the latest remaining len
          // for each descriptor.
          for (i = 0; i < nSgl; i++)
          {
               remLen[i] -= minLen;
          }

          lenDone += minLen;

     } while (lenDone < len);

     ASSERT (lenDone == len);
}


/************************************************************************
 * Performs a block-optimized EXCLUSIVE-OR operation on one to
 * eight data blocks and stores the result into a different data block.
 ************************************************************************/

// For some reason, unrolling it further (block size = 32)
// seems to decrease the performance!

void xor9 (unsigned long     size
                  , SGL_DESC      **ppDesc
                  , int            *remLen)
{
     unsigned long i;

     int blockSize = 16;   // chars

     unsigned char * result = SGL_BUF_OFFSET (ppDesc[0], remLen[0]);
     unsigned char * s1     = SGL_BUF_OFFSET (ppDesc[1], remLen[1]);
     unsigned char * s2     = SGL_BUF_OFFSET (ppDesc[2], remLen[2]);
     unsigned char * s3     = SGL_BUF_OFFSET (ppDesc[3], remLen[3]);
     unsigned char * s4     = SGL_BUF_OFFSET (ppDesc[4], remLen[4]);
     unsigned char * s5     = SGL_BUF_OFFSET (ppDesc[5], remLen[5]);
     unsigned char * s6     = SGL_BUF_OFFSET (ppDesc[6], remLen[6]);
     unsigned char * s7     = SGL_BUF_OFFSET (ppDesc[7], remLen[7]);
     unsigned char * s8     = SGL_BUF_OFFSET (ppDesc[8], remLen[8]);

     ASSERT (((unsigned long)s1 & 0xF) == 0);
     ASSERT (((unsigned long)s2 & 0xF) == 0);
     ASSERT (((unsigned long)s3 & 0xF) == 0);
     ASSERT (((unsigned long)s4 & 0xF) == 0);
     ASSERT (((unsigned long)s5 & 0xF) == 0);
     ASSERT (((unsigned long)s6 & 0xF) == 0);
     ASSERT (((unsigned long)s7 & 0xF) == 0);
     ASSERT (((unsigned long)s8 & 0xF) == 0);


     ASSERT (size % blockSize == 0);

     // Loop to handle blocks of N bytes
     for (i = 0; i < size; i += blockSize)
     {
          asm volatile ("movaps   %0,%%xmm0" :: "m" (s1[i]) : "xmm0");
          asm volatile ("movaps   %0,%%xmm1" :: "m" (s2[i]) : "xmm1");
          asm volatile ("movaps   %0,%%xmm2" :: "m" (s3[i]) : "xmm2");
          asm volatile ("movaps   %0,%%xmm3" :: "m" (s4[i]) : "xmm3");
          asm volatile ("movaps   %0,%%xmm4" :: "m" (s5[i]) : "xmm4");
          asm volatile ("movaps   %0,%%xmm5" :: "m" (s6[i]) : "xmm5");
          asm volatile ("movaps   %0,%%xmm6" :: "m" (s7[i]) : "xmm6");
          asm volatile ("movaps   %0,%%xmm7" :: "m" (s8[i]) : "xmm7");

          asm volatile ("xorps     %xmm1,%xmm0");
          asm volatile ("xorps     %xmm3,%xmm2");
          asm volatile ("xorps     %xmm5,%xmm4");
          asm volatile ("xorps     %xmm7,%xmm6");

          // Prefetch
          asm volatile ("prefetchnta %0" :: "m" (s1[i+128]));
          asm volatile ("prefetchnta %0" :: "m" (s2[i+128]));
          asm volatile ("prefetchnta %0" :: "m" (s3[i+128]));
          asm volatile ("prefetchnta %0" :: "m" (s4[i+128]));

          asm volatile ("xorps     %xmm2,%xmm0");
          asm volatile ("xorps     %xmm6,%xmm4");

          asm volatile ("xorps     %xmm4,%xmm0");

          asm volatile ("movntps %%xmm0,%0" : "=m" (result[i]));

          // Prefetch
          asm volatile ("prefetchnta %0" :: "m" (s5[i+128]));
          asm volatile ("prefetchnta %0" :: "m" (s6[i+128]));
          asm volatile ("prefetchnta %0" :: "m" (s7[i+128]));
          asm volatile ("prefetchnta %0" :: "m" (s8[i+128]));

     }
}


#undef RESULT
#undef P1
#undef P2
#undef P3
#undef P4

#define RESULT (%4)
#define P1     (%0)
#define P2     (%1)
#define P3     (%2)
#define P4     (%3)

void xor5 (unsigned long     size
                  , SGL_DESC      **ppDesc
                  , int            *remLen)
{
     int blockSize = 128;                   /* chars */
     int blocksPerLoop = 1;
     unsigned char * endp1;

     unsigned char * result = SGL_BUF_OFFSET (ppDesc[0], remLen[0]);
     unsigned char * p1 = SGL_BUF_OFFSET (ppDesc[1], remLen[1]);
     unsigned char * p2 = SGL_BUF_OFFSET (ppDesc[2], remLen[2]);
     unsigned char * p3 = SGL_BUF_OFFSET (ppDesc[3], remLen[3]);
     unsigned char * p4 = SGL_BUF_OFFSET (ppDesc[4], remLen[4]);

     //     printv ("p1 = %#x, endp1 = %#x, a = %#x, &a[%d-1] = %#x\n", p1, endp1, a, size, &a[size-1]);

     ASSERT (((unsigned long)p1 & 0xF) == 0);
     ASSERT (((unsigned long)p2 & 0xF) == 0);
     ASSERT (((unsigned long)p3 & 0xF) == 0);
     ASSERT (((unsigned long)p4 & 0xF) == 0);
     ASSERT (((unsigned long)size % 4) == 0); // multiple of 4 32-bits.

     endp1 = (unsigned char *) ((unsigned long)p1 + size);

     // is dest and one of the sources point to the same location
     if ((p1 == result) || (p2 == result) || (p3 == result) || (p4 == result))
     {
          // write to the cache, not to RAM directly.
          while (p1 < endp1)
          {
               __asm__ __volatile__ (

                    // get p2 into xmm
                    LD  (0  ,  P2, xmm0)
                    LD  (16 ,  P2, xmm1)
                    LD  (32 ,  P2, xmm2)
                    LD  (48 ,  P2, xmm3)

                    PF  (128+0,  P1)
                    PF  (128+32, P1)
                    PF  (128+64, P1)
                    PF  (128+96, P1)

                    // xor p2 and p1, by fetching p1 directly from memory.
                    XOR (0  ,  P1, xmm0)
                    XOR (16 ,  P1, xmm1)
                    XOR (32 ,  P1, xmm2)
                    XOR (48 ,  P1, xmm3)

                    // xor p3 and (p1^p2), by fetching p3 directly from memory.
                    XOR (0  ,  P3, xmm0)
                    XOR (16 ,  P3, xmm1)
                    XOR (32 ,  P3, xmm2)
                    XOR (48 ,  P3, xmm3)

                    PF  (128+0,  P3)
                    PF  (128+32, P3)
                    PF  (128+64, P3)
                    PF  (128+96, P3)

                    // xor p4 and (p1^p2^p3), by fetching p4 directly from memory.
                    XOR (0  ,  P4, xmm0)
                    XOR (16 ,  P4, xmm1)
                    XOR (32 ,  P4, xmm2)
                    XOR (48 ,  P4, xmm3)

                    // write the result p1^p2^p3 int RESULT (which == p1)
                    STT  (xmm0, 0  ,  RESULT)
                    STT  (xmm1, 16 ,  RESULT)
                    STT  (xmm2, 32 ,  RESULT)
                    STT  (xmm3, 48 ,  RESULT)

                    PF  (128+0,  P2)
                    PF  (128+32, P2)
                    PF  (128+64, P2)
                    PF  (128+96, P2)

                    // BATCH 2


                    // get p2 into xmm
                    LD  (64 ,  P2, xmm4)
                    LD  (80 ,  P2, xmm5)
                    LD  (96 ,  P2, xmm6)
                    LD  (112,  P2, xmm7)

                    // xor p2 and p1, by fetching p1 directly from memory.
                    XOR (64 ,  P1, xmm4)
                    XOR (80 ,  P1, xmm5)
                    XOR (96 ,  P1, xmm6)
                    XOR (112,  P1, xmm7)

                    // because we are prefetching a bit late, we might as well fetch something
                    // a little farther away to benefit the next loop and the one after that.
                    PF  (128+64+0,  P4)
                    PF  (128+64+32, P4)
                    PF  (128+64+64, P4)
                    PF  (128+64+96, P4)

                    // xor p3 and (p1^p2), by fetching p3 directly from memory.
                    XOR (64 ,  P3, xmm4)
                    XOR (80 ,  P3, xmm5)
                    XOR (96 ,  P3, xmm6)
                    XOR (112,  P3, xmm7)

                    // xor p4 and (p1^p2^p3), by fetching p4 directly from memory.
                    XOR (64 ,  P4, xmm4)
                    XOR (80 ,  P4, xmm5)
                    XOR (96 ,  P4, xmm6)
                    XOR (112,  P4, xmm7)

                    // write the result p1^p2^p3 int RESULT (which == p1)
                    STT  (xmm4, 64 ,  RESULT)
                    STT  (xmm5, 80 ,  RESULT)
                    STT  (xmm6, 96 ,  RESULT)
                    STT  (xmm7, 112,  RESULT)


                    // Not using mfence/sfence because only one processor
                    // will access this data and also, this data will not
                    // be accessed more than once.

                    :"=r" (p1), "=r" (p2), "=r" (p3), "=r" (p4), "=r" (result)

                    :"0" (p1),"1" (p2), "2" (p3), "3" (p4), "4" (result)

                    :"memory"
                    , "xmm0"
                    , "xmm1"
                    , "xmm2"
                    , "xmm3"
                    , "xmm4"
                    , "xmm5"
                    , "xmm6"
                    , "xmm7"
                    );

               p1 += blockSize * blocksPerLoop;
               p2 += blockSize * blocksPerLoop;
               p3 += blockSize * blocksPerLoop;
               p4 += blockSize * blocksPerLoop;
               result += blockSize * blocksPerLoop;
          }
     }
     else
     {
          // write to RAM, not to cache directly.
          while (p1 < endp1)
          {
               __asm__ __volatile__ (

                    // get p2 into xmm
                    LD  (0  ,  P2, xmm0)
                    LD  (16 ,  P2, xmm1)
                    LD  (32 ,  P2, xmm2)
                    LD  (48 ,  P2, xmm3)

                    PF  (128+0,  P1)
                    PF  (128+32, P1)
                    PF  (128+64, P1)
                    PF  (128+96, P1)

                    // xor p2 and p1, by fetching p1 directly from memory.
                    XOR (0  ,  P1, xmm0)
                    XOR (16 ,  P1, xmm1)
                    XOR (32 ,  P1, xmm2)
                    XOR (48 ,  P1, xmm3)

                    // xor p3 and (p1^p2), by fetching p3 directly from memory.
                    XOR (0  ,  P3, xmm0)
                    XOR (16 ,  P3, xmm1)
                    XOR (32 ,  P3, xmm2)
                    XOR (48 ,  P3, xmm3)

                    PF  (128+0,  P3)
                    PF  (128+32, P3)
                    PF  (128+64, P3)
                    PF  (128+96, P3)

                    // xor p4 and (p1^p2^p3), by fetching p4 directly from memory.
                    XOR (0  ,  P4, xmm0)
                    XOR (16 ,  P4, xmm1)
                    XOR (32 ,  P4, xmm2)
                    XOR (48 ,  P4, xmm3)

                    // write the result p1^p2^p3 int RESULT (which == p1)
                    STNT  (xmm0, 0  ,  RESULT)
                    STNT  (xmm1, 16 ,  RESULT)
                    STNT  (xmm2, 32 ,  RESULT)
                    STNT  (xmm3, 48 ,  RESULT)

                    PF  (128+0,  P2)
                    PF  (128+32, P2)
                    PF  (128+64, P2)
                    PF  (128+96, P2)

                    // BATCH 2


                    // get p2 into xmm
                    LD  (64 ,  P2, xmm4)
                    LD  (80 ,  P2, xmm5)
                    LD  (96 ,  P2, xmm6)
                    LD  (112,  P2, xmm7)

                    // xor p2 and p1, by fetching p1 directly from memory.
                    XOR (64 ,  P1, xmm4)
                    XOR (80 ,  P1, xmm5)
                    XOR (96 ,  P1, xmm6)
                    XOR (112,  P1, xmm7)

                    // because we are prefetching a bit late, we might as well fetch something
                    // a little farther away to benefit the next loop and the one after that.
                    PF  (128+64+0,  P4)
                    PF  (128+64+32, P4)
                    PF  (128+64+64, P4)
                    PF  (128+64+96, P4)

                    // xor p3 and (p1^p2), by fetching p3 directly from memory.
                    XOR (64 ,  P3, xmm4)
                    XOR (80 ,  P3, xmm5)
                    XOR (96 ,  P3, xmm6)
                    XOR (112,  P3, xmm7)

                    // xor p4 and (p1^p2^p3), by fetching p4 directly from memory.
                    XOR (64 ,  P4, xmm4)
                    XOR (80 ,  P4, xmm5)
                    XOR (96 ,  P4, xmm6)
                    XOR (112,  P4, xmm7)

                    // write the result p1^p2^p3 int RESULT (which == p1)
                    STNT  (xmm4, 64 ,  RESULT)
                    STNT  (xmm5, 80 ,  RESULT)
                    STNT  (xmm6, 96 ,  RESULT)
                    STNT  (xmm7, 112,  RESULT)


                    // Not using mfence/sfence because only one processor
                    // will access this data and also, this data will not
                    // be accessed more than once.

                    :"=r" (p1), "=r" (p2), "=r" (p3), "=r" (p4), "=r" (result)

                    :"0" (p1),"1" (p2), "2" (p3), "3" (p4), "4" (result)

                    :"memory"
                    , "xmm0"
                    , "xmm1"
                    , "xmm2"
                    , "xmm3"
                    , "xmm4"
                    , "xmm5"
                    , "xmm6"
                    , "xmm7"
                    );

               p1 += blockSize * blocksPerLoop;
               p2 += blockSize * blocksPerLoop;
               p3 += blockSize * blocksPerLoop;
               p4 += blockSize * blocksPerLoop;
               result += blockSize * blocksPerLoop;
          }
     }
}


#undef RESULT
#undef P1
#undef P2
#undef P3


#define RESULT (%3)
#define P1     (%0)
#define P2     (%1)
#define P3     (%2)

void xor4 (unsigned long   size
                  , SGL_DESC    **ppDesc
                  , int          *remLen)
{
     int blockSize = 128;                   /* chars */
     int blocksPerLoop = 1;
     unsigned char *  endp1;

     unsigned char * result = SGL_BUF_OFFSET (ppDesc[0], remLen[0]);
     unsigned char * p1 = SGL_BUF_OFFSET (ppDesc[1], remLen[1]);
     unsigned char * p2 = SGL_BUF_OFFSET (ppDesc[2], remLen[2]);
     unsigned char * p3 = SGL_BUF_OFFSET (ppDesc[3], remLen[3]);

     ASSERT (((unsigned long)p1 & 0xF) == 0);
     ASSERT (((unsigned long)p2 & 0xF) == 0);
     ASSERT (((unsigned long)p3 & 0xF) == 0);
     ASSERT (((unsigned long)size % 4) == 0); // multiple of 4 32-bits.

     endp1 = (unsigned char*) ((unsigned long)p1 + size);

     // is dest and one of the sources point to the same location
     if ((p1 == result) || (p2 == result) || (p3 == result))
     {
          // write to the cache, not to RAM directly.

          while (p1 < endp1)
          {
               __asm__ __volatile__ (

                    // get p2 into xmm
                    LD  (0  ,  P2, xmm0)
                    LD  (16 ,  P2, xmm1)
                    LD  (32 ,  P2, xmm2)
                    LD  (48 ,  P2, xmm3)

                    PF  (128+0,  P1)
                    PF  (128+32, P1)
                    PF  (128+64, P1)
                    PF  (128+96, P1)

                    // xor p2 and p1, by fetching p1 directly from memory.
                    XOR (0  ,  P1, xmm0)
                    XOR (16 ,  P1, xmm1)
                    XOR (32 ,  P1, xmm2)
                    XOR (48 ,  P1, xmm3)

                    // xor p3 and (p1^p2), by fetching p3 directly from memory.
                    XOR (0  ,  P3, xmm0)
                    XOR (16 ,  P3, xmm1)
                    XOR (32 ,  P3, xmm2)
                    XOR (48 ,  P3, xmm3)

                    PF  (128+0,  P3)
                    PF  (128+32, P3)
                    PF  (128+64, P3)
                    PF  (128+96, P3)

                    // write the result p1^p2^p3 int RESULT (which == p1)
                    STT  (xmm0, 0  ,  RESULT)
                    STT  (xmm1, 16 ,  RESULT)
                    STT  (xmm2, 32 ,  RESULT)
                    STT  (xmm3, 48 ,  RESULT)


                    // BATCH 2


                    // get p2 into xmm
                    LD  (64 ,  P2, xmm4)
                    LD  (80 ,  P2, xmm5)
                    LD  (96 ,  P2, xmm6)
                    LD  (112,  P2, xmm7)

                    // because we are prefetching a bit late, we might as well fetch something
                    // a little farther away to benefit the next loop and the one after that.
                    PF  (128+64+0,  P2)
                    PF  (128+64+32, P2)
                    PF  (128+64+64, P2)
                    PF  (128+64+96, P2)

                    // xor p2 and p1, by fetching p1 directly from memory.
                    XOR (64 ,  P1, xmm4)
                    XOR (80 ,  P1, xmm5)
                    XOR (96 ,  P1, xmm6)
                    XOR (112,  P1, xmm7)

                    // xor p3 and (p1^p2), by fetching p3 directly from memory.
                    XOR (64 ,  P3, xmm4)
                    XOR (80 ,  P3, xmm5)
                    XOR (96 ,  P3, xmm6)
                    XOR (112,  P3, xmm7)

                    // write the result p1^p2^p3 int RESULT (which == p1)
                    STT  (xmm4, 64 ,  RESULT)
                    STT  (xmm5, 80 ,  RESULT)
                    STT  (xmm6, 96 ,  RESULT)
                    STT  (xmm7, 112,  RESULT)


                    // Not using mfence/sfence because only one processor
                    // will access this data and also, this data will not
                    // be accessed more than once.

                    :"=q" (p1), "=q" (p2), "=q" (p3), "=q" (result)

                    :"0" (p1),"1" (p2), "2" (p3), "3" (result)

                    :"memory"
                    , "xmm0"
                    , "xmm1"
                    , "xmm2"
                    , "xmm3"
                    , "xmm4"
                    , "xmm5"
                    , "xmm6"
                    , "xmm7"
                    );

               p1 += blockSize * blocksPerLoop;
               p2 += blockSize * blocksPerLoop;
               p3 += blockSize * blocksPerLoop;
               result += blockSize * blocksPerLoop;
          }
     }
     else
     {
          // write to RAM, not to cache directly.

          while (p1 < endp1)
          {
               __asm__ __volatile__ (

                    // get p2 into xmm
                    LD  (0  ,  P2, xmm0)
                    LD  (16 ,  P2, xmm1)
                    LD  (32 ,  P2, xmm2)
                    LD  (48 ,  P2, xmm3)

                    PF  (128+0,  P1)
                    PF  (128+32, P1)
                    PF  (128+64, P1)
                    PF  (128+96, P1)

                    // xor p2 and p1, by fetching p1 directly from memory.
                    XOR (0  ,  P1, xmm0)
                    XOR (16 ,  P1, xmm1)
                    XOR (32 ,  P1, xmm2)
                    XOR (48 ,  P1, xmm3)

                    // xor p3 and (p1^p2), by fetching p3 directly from memory.
                    XOR (0  ,  P3, xmm0)
                    XOR (16 ,  P3, xmm1)
                    XOR (32 ,  P3, xmm2)
                    XOR (48 ,  P3, xmm3)

                    PF  (128+0,  P3)
                    PF  (128+32, P3)
                    PF  (128+64, P3)
                    PF  (128+96, P3)

                    // write the result p1^p2^p3 int RESULT (which == p1)
                    STNT  (xmm0, 0  ,  RESULT)
                    STNT  (xmm1, 16 ,  RESULT)
                    STNT  (xmm2, 32 ,  RESULT)
                    STNT  (xmm3, 48 ,  RESULT)


                    // BATCH 2


                    // get p2 into xmm
                    LD  (64 ,  P2, xmm4)
                    LD  (80 ,  P2, xmm5)
                    LD  (96 ,  P2, xmm6)
                    LD  (112,  P2, xmm7)

                    // because we are prefetching a bit late, we might as well fetch something
                    // a little farther away to benefit the next loop and the one after that.
                    PF  (128+64+0,  P2)
                    PF  (128+64+32, P2)
                    PF  (128+64+64, P2)
                    PF  (128+64+96, P2)

                    // xor p2 and p1, by fetching p1 directly from memory.
                    XOR (64 ,  P1, xmm4)
                    XOR (80 ,  P1, xmm5)
                    XOR (96 ,  P1, xmm6)
                    XOR (112,  P1, xmm7)

                    // xor p3 and (p1^p2), by fetching p3 directly from memory.
                    XOR (64 ,  P3, xmm4)
                    XOR (80 ,  P3, xmm5)
                    XOR (96 ,  P3, xmm6)
                    XOR (112,  P3, xmm7)

                    // write the result p1^p2^p3 int RESULT (which == p1)
                    STNT  (xmm4, 64 ,  RESULT)
                    STNT  (xmm5, 80 ,  RESULT)
                    STNT  (xmm6, 96 ,  RESULT)
                    STNT  (xmm7, 112,  RESULT)

                    // Not using mfence/sfence because only one processor
                    // will access this data and also, this data will not
                    // be accessed more than once.

                    :"=q" (p1), "=q" (p2), "=q" (p3), "=q" (result)

                    :"0" (p1),"1" (p2), "2" (p3), "3" (result)

                    :"memory"
                    , "xmm0"
                    , "xmm1"
                    , "xmm2"
                    , "xmm3"
                    , "xmm4"
                    , "xmm5"
                    , "xmm6"
                    , "xmm7"
                    );

               p1 += blockSize * blocksPerLoop;
               p2 += blockSize * blocksPerLoop;
               p3 += blockSize * blocksPerLoop;
               result += blockSize * blocksPerLoop;
          }
     }
}


/**********************************************************************/

#undef RESULT
#undef P1
#undef P2


#define RESULT (%2)
#define P1     (%0)
#define P2     (%1)

void xor3 (unsigned long   size
                  , SGL_DESC    **ppDesc
                  , int          *remLen)
{
     int blockSize = 128;                   /* chars */
     int blocksPerLoop = 1;
     unsigned char *  endp1;

     unsigned char * result = SGL_BUF_OFFSET (ppDesc[0], remLen[0]);
     unsigned char * p1 = SGL_BUF_OFFSET (ppDesc[1], remLen[1]);
     unsigned char * p2 = SGL_BUF_OFFSET (ppDesc[2], remLen[2]);

     ASSERT (((unsigned long)p1 & 0xF) == 0);
     ASSERT (((unsigned long)p2 & 0xF) == 0);
     ASSERT ((size % 16) == 0); // multiple of 4 32-bits.

     endp1 = (unsigned char *) ((unsigned long)p1 + size);

     // is dest and one of the sources point to the same location
     if ((p1 == result) || (p2 == result))
     {
          // write to the cache, not to RAM directly.

          while (p1 < endp1)
          {
               __asm__ __volatile__ (

                    LD  (0  ,  P2, xmm0)
                    LD  (16 ,  P2, xmm1)
                    LD  (32 ,  P2, xmm2)
                    LD  (48 ,  P2, xmm3)

                    PF  (128+0,  P1)
                    PF  (128+32, P1)
                    PF  (128+64, P1)
                    PF  (128+96, P1)

                    XOR (0  ,  P1, xmm0)
                    XOR (16 ,  P1, xmm1)
                    XOR (32 ,  P1, xmm2)
                    XOR (48 ,  P1, xmm3)

                    STT  (xmm0, 0  ,  RESULT)
                    STT  (xmm1, 16 ,  RESULT)
                    STT  (xmm2, 32 ,  RESULT)
                    STT  (xmm3, 48 ,  RESULT)

                    LD  (64 ,  P2, xmm4)
                    LD  (80 ,  P2, xmm5)
                    LD  (96 ,  P2, xmm6)
                    LD  (112,  P2, xmm7)

                    // because we are prefetching a bit late, we might as well fetch something
                    // a little farther away to benefit the next loop and the one after that.
                    PF  (128+64+0,  P2)
                    PF  (128+64+32, P2)
                    PF  (128+64+64, P2)
                    PF  (128+64+96, P2)

                    XOR (64 ,  P1, xmm4)
                    XOR (80 ,  P1, xmm5)
                    XOR (96 ,  P1, xmm6)
                    XOR (112,  P1, xmm7)

                    STT  (xmm4, 64 ,  RESULT)
                    STT  (xmm5, 80 ,  RESULT)
                    STT  (xmm6, 96 ,  RESULT)
                    STT  (xmm7, 112,  RESULT)


                    // Not using mfence/sfence because only one processor
                    // will access this data and also, this data will not
                    // be accessed more than once.

                    :"=q" (p1), "=q" (p2), "=q" (result)

                    :"0" (p1),"1" (p2), "2" (result)

                    :"memory"
                    , "xmm0"
                    , "xmm1"
                    , "xmm2"
                    , "xmm3"
                    , "xmm4"
                    , "xmm5"
                    , "xmm6"
                    , "xmm7"
                    );

               p1     += blockSize * blocksPerLoop;
               p2     += blockSize * blocksPerLoop;
               result += blockSize * blocksPerLoop;
          }
     }
     else
     {
          // write to RAM, not to cache directly.

          while (p1 < endp1)
          {
               __asm__ __volatile__ (

                    LD  (0  ,  P2, xmm0)
                    LD  (16 ,  P2, xmm1)
                    LD  (32 ,  P2, xmm2)
                    LD  (48 ,  P2, xmm3)

                    PF  (128+0,  P1)
                    PF  (128+32, P1)
                    PF  (128+64, P1)
                    PF  (128+96, P1)

                    XOR (0  ,  P1, xmm0)
                    XOR (16 ,  P1, xmm1)
                    XOR (32 ,  P1, xmm2)
                    XOR (48 ,  P1, xmm3)

                    STNT  (xmm0, 0  ,  RESULT)
                    STNT  (xmm1, 16 ,  RESULT)
                    STNT  (xmm2, 32 ,  RESULT)
                    STNT  (xmm3, 48 ,  RESULT)

                    LD  (64 ,  P2, xmm4)
                    LD  (80 ,  P2, xmm5)
                    LD  (96 ,  P2, xmm6)
                    LD  (112,  P2, xmm7)

                    // because we are prefetching a bit late, we might as well fetch something
                    // a little farther away to benefit the next loop and the one after that.
                    PF  (128+64+0,  P2)
                    PF  (128+64+32, P2)
                    PF  (128+64+64, P2)
                    PF  (128+64+96, P2)

                    XOR (64 ,  P1, xmm4)
                    XOR (80 ,  P1, xmm5)
                    XOR (96 ,  P1, xmm6)
                    XOR (112,  P1, xmm7)

                    STNT  (xmm4, 64 ,  RESULT)
                    STNT  (xmm5, 80 ,  RESULT)
                    STNT  (xmm6, 96 ,  RESULT)
                    STNT  (xmm7, 112,  RESULT)


                    // Not using mfence/sfence because only one processor
                    // will access this data.

                    :"=q" (p1), "=q" (p2), "=q" (result)

                    :"0" (p1),"1" (p2), "2" (result)

                    :"memory"
                    , "xmm0"
                    , "xmm1"
                    , "xmm2"
                    , "xmm3"
                    , "xmm4"
                    , "xmm5"
                    , "xmm6"
                    , "xmm7"
                    );

               p1     += blockSize * blocksPerLoop;
               p2     += blockSize * blocksPerLoop;
               result += blockSize * blocksPerLoop;
          }
     }
}

#undef RESULT
#undef P1
#undef P2


/**********************************************************************/
/* void xor9 (unsigned long     size                                  */
/*          , SGL_DESC      **ppDesc                                  */
/*          , int            *remLen)                                 */
/* {                                                                  */
/*      int i;                                                        */
/*                                                                    */
/*      unsigned char * d  = SGL_BUF_OFFSET (ppDesc[0], remLen[0]);   */
/*      unsigned char * s1 = SGL_BUF_OFFSET (ppDesc[1], remLen[1]);   */
/*      unsigned char * s2 = SGL_BUF_OFFSET (ppDesc[2], remLen[2]);   */
/*      unsigned char * s3 = SGL_BUF_OFFSET (ppDesc[3], remLen[3]);   */
/*      unsigned char * s4 = SGL_BUF_OFFSET (ppDesc[4], remLen[4]);   */
/*      unsigned char * s5 = SGL_BUF_OFFSET (ppDesc[5], remLen[5]);   */
/*      unsigned char * s6 = SGL_BUF_OFFSET (ppDesc[6], remLen[6]);   */
/*      unsigned char * s7 = SGL_BUF_OFFSET (ppDesc[7], remLen[7]);   */
/*      unsigned char * s8 = SGL_BUF_OFFSET (ppDesc[8], remLen[8]);   */
/*                                                                    */
/*      for (i = 0; i < (int)size; i++)                               */
/*           d[i] = s1[i] ^ s2[i] ^ s3[i] ^ s4[i]                     */
/*             ^ s5[i] ^ s6[i] ^ s7[i] ^ s8[i];                       */
/* }                                                                  */
/*                                                                    */
/*                                                                    */
/* void vxor2 (int size                                               */
/*             , unsigned char *d                                     */
/*             , unsigned char *s1                                    */
/*             , unsigned char *s2                                    */
/*      )                                                             */
/* {                                                                  */
/*      int i;                                                        */
/*                                                                    */
/*      for (i = 0; i < size; i++)                                    */
/*           d[i] = s1[i] ^ s2[i];                                    */
/* }                                                                  */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/* void vxor5 (int size                                               */
/*             , unsigned char *d                                     */
/*             , unsigned char *s1                                    */
/*             , unsigned char *s2                                    */
/*             , unsigned char *s3                                    */
/*             , unsigned char *s4                                    */
/*      )                                                             */
/* {                                                                  */
/*      int i;                                                        */
/*                                                                    */
/*      for (i = 0; i < size; i++)                                    */
/*           d[i] = s1[i] ^ s2[i] ^ s3[i] ^ s4[i];                    */
/* }                                                                  */
/*                                                                    */
/* void vxor9 (int size                                               */
/*             , unsigned char *d                                     */
/*             , unsigned char *s1                                    */
/*             , unsigned char *s2                                    */
/*             , unsigned char *s3                                    */
/*             , unsigned char *s4                                    */
/*             , unsigned char *s5                                    */
/*             , unsigned char *s6                                    */
/*             , unsigned char *s7                                    */
/*             , unsigned char *s8                                    */
/*      )                                                             */
/* {                                                                  */
/*      int i;                                                        */
/*                                                                    */
/*      for (i = 0; i < size; i++)                                    */
/*           d[i] = s1[i] ^ s2[i] ^ s3[i] ^ s4[i]                     */
/*             ^ s5[i] ^ s6[i] ^ s7[i] ^ s8[i];                       */
/* }                                                                  */
/**********************************************************************/

// can't trust the size field anymore!
int getSglBufSize (SGL *p)
{
     int i;
     int size = 0;
     SGL_DESC *d;

     ASSERT (p);

     d = (SGL_DESC *) (p + 1);

     for (i = 0; i < p->scnt; i++)
          size += d[i].len;

     return size;

}


// get min in an array
/* inline */ int getMin (int size, int *a)
{
     int min;
     int i;

     ASSERT ((size > 0) && a);

     min = a[0];

     for (i = 1; i < size; i++)
     {
          if (a[i] < min)
          {
               min = a[i];
          }
     }

     return min;
}


// If others have been using these declarations,
// let them not have any conflicts.

#undef REG
#undef LD
#undef STNT
#undef STT
#undef XOR
#undef MFENCE
#undef SFENCE
#undef PF

void validateSGLs (int nSgl, SGL **ppSGL)
{
     int i;
#if SGL_XOR_PRINT
     static long nCalled = 0;
     static long n3 = 0;
     static long n4 = 0;
     static long n5 = 0;
     static long n9 = 0;
     static double bytesXORed = 0.0;
#endif
     FILE *fp;

     ASSERT (nSgl > 0);

#if 0
     fp = fopen ("xor.dump", "w");
#else
     fp = stderr;
#endif

#if SGL_XOR_PRINT
     // Some data to give the developer some satisfaction!

     bytesXORed += (nSgl - 1) * getSglBufSize (ppSGL[0]);
     nCalled++;

     switch (nSgl)
     {
     case 3: n3++;
          break;
     case 4: n4++;
          break;
     case 5: n5++;
          break;
     case 9: n9++;
          break;
     }



     if (nCalled % SGL_XOR_PRINT_FREQ == 0)
     {
#if 1
          fprintf(stderr, "RL_XorSGL: called %ld times(%--3:4:5:9 = %d:%d:%d:%d); XORed %.2f %s of data\n"
                  , nCalled
                  , (n3 * 100) / nCalled
                  , (n4 * 100) / nCalled
                  , (n5 * 100) / nCalled
                  , (n9 * 100) / nCalled
                  , bytesXORed < (1 << 30) ? bytesXORed / (1 << 20) :  bytesXORed / (1 << 30)
                  , bytesXORed < (1 << 30) ? "MB" : "GB"
               );
#else
          fprintf(stderr, "RL_XorSGL: called %ld times; XORed %.2f %s of data\n"
                  , nCalled
                  , bytesXORed < (1 << 30) ? bytesXORed / (1 << 20) :  bytesXORed / (1 << 30)
                  , bytesXORed < (1 << 30) ? "MB" : "GB"
               );
#endif
     }
#endif // SGL_XOR_PRINT

     // len should be greater than zero, and a multiple of 512 bytes.
     // all SGLs should have same length!

     for (i = 0; i < nSgl; i++)
     {
          // fprintf(stderr, "sgl[%d] size = %d\n", i, getSglBufSize (ppSGL[i]));
          ASSERT (getSglBufSize (ppSGL[i]) > 0);
          ASSERT ((getSglBufSize (ppSGL[i])) % 512 == 0);
     }

     for (i = 0; i < nSgl - 1; i++)
     {
          ASSERT (getSglBufSize (ppSGL[i]) == getSglBufSize (ppSGL[i + 1]));
     }
}


/***************************************************************/
/* Performs fast memcmp() for buffers whose size is            */
/* a multiple of 32 bytes. However, if the size isn't          */
/* larger than 4kB, this function will not help, and           */
/* for very small arrays (< 1kB), it might even reduce         */
/* performance because of the prefetches.                      */
/*                                                             */
/* Loop unrolling of 8 seems to give max speed.                */
/*                                                             */
/* NOTE: Unlike regular memcmp, we return only two             */
/*       values, TRUE if equal, FALSE otherwise.               */
/***************************************************************/

unsigned long RL_FastMemCmp (unsigned long *p1, unsigned long *p2, unsigned long size)
{
     int i;
     int numWords;
     int notEqual = 0;

     ASSERT ((size % 32) == 0); // multiple of N bytes

     numWords = size / (sizeof (long));

     // compare longs instead of chars.
     for (i = 0; !notEqual && (i < numWords); i=i+8)
     {
          // only 32 ahead because we are dealing with longs.
          // This is 128 bytes ahead, in fact.
          asm volatile("prefetchnta %0" :: "m" (p1[i+32]));
          asm volatile("prefetchnta %0" :: "m" (p2[i+32]));

          notEqual = (   (p1[i + 0] != p2[i + 0])
                         || (p1[i + 1] != p2[i + 1])
                         || (p1[i + 2] != p2[i + 2])
                         || (p1[i + 3] != p2[i + 3])
                         || (p1[i + 4] != p2[i + 4])
                         || (p1[i + 5] != p2[i + 5])
                         || (p1[i + 6] != p2[i + 6])
                         || (p1[i + 7] != p2[i + 7])
               );
     }

     return notEqual ? 0 : 1;
}


/****************************************************/
/* This function goes through the SGLs and          */
/* logically chops it into corresponding            */
/* parallel blocks. It compares these blocks        */
/* repeatedly until the entire SGLs are             */
/* compared. This logic is similar to that          */
/* used in RL_XorSGL() function.                    */
/****************************************************/

#define MAX_CMP_SGL (2)

unsigned long RL_CompareSGL (SGL *p1, SGL *p2)
{
     int i;
     int minLen;
     int len;
     int lenDone;
     int nSgl;

     SGL        *ppSGL [MAX_CMP_SGL];
     SGL_DESC   *ppDesc[MAX_CMP_SGL];
     int         remLen[MAX_CMP_SGL];     // remaining len in each SGL Desc.
     int         areEqual = 1;

     // we always compare two SGLs only.
     nSgl = 2;

     ppSGL[0] = p1;
     ppSGL[1] = p2;

     for (i = 0; i < nSgl; i++)
     {
          ASSERT (ppSGL[i]);

          ppDesc[i] = (SGL_DESC *) (ppSGL[i] + 1);
          remLen[i] = ppDesc[i]->len;
     }

#if SGL_XOR_DEBUG

     validateSGLs (nSgl, ppSGL);

#endif // SGL_XOR_DEBUG

     // if the SGLs are of different lengths, return FALSE.
     if (getSglBufSize (ppSGL[0]) != getSglBufSize (ppSGL[1]))
     {
          // fprintf(stderr, "RL_CompareSGL: SGL compare failed; different lengths.\n");
          areEqual = 0;
     }
     else
     {
          len = getSglBufSize (ppSGL[0]);
          lenDone = 0;
          areEqual = 1;

          // while more work needs to be done...
          do
          {
               // if any of the descriptors are finished (at least one
               // is guaranteed to be exhausted each time except
               // during the first time), go on to the next one...
               // Don't forget to update the remaining len for that desc!
               for (i = 0; i < nSgl; i++)
               {
                    if (remLen[i] <= 0)
                    {
                         ASSERT (remLen[i] == 0);
                         ppDesc[i]++;
                         remLen[i] = ppDesc[i]->len;
                         ASSERT (remLen[i] % 512 == 0);
                    }
               }

               // get the min common len value and perform xor on those regions.
               minLen = getMin (nSgl, remLen);
               ASSERT (minLen > 0);

               // compare these blocks.
               areEqual = RL_FastMemCmp ((unsigned long *)SGL_BUF_OFFSET (ppDesc[0], remLen[0])
                                         , (unsigned long *)SGL_BUF_OFFSET (ppDesc[1], remLen[1])
                                         , (unsigned)minLen);

               // update rem values so that they reflect the latest remaining len
               // for each descriptor.
               for (i = 0; i < nSgl; i++)
               {
                    remLen[i] -= minLen;
               }

               lenDone += minLen;

          } while ((lenDone < len) && areEqual);
     }

     //printf ("***V: RL_CompareSGL: %s\n", areEqual ? "TRUE" : "FALSE");

     return areEqual ? 1 : 0;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
