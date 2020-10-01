#include <stdio.h>
#include <sys/types.h>
#include <stdarg.h>


/* ------------------------------------------------------------------------ */
#define long64 long long
#define ulong64 unsigned long long
#define uchar  unsigned char
#define byte   unsigned char
#define schar  signed char

/* ------------------------------------------------------------------------ */
    typedef unsigned long long          UINT64;
    typedef unsigned int                UINT32;
    typedef unsigned short                  UINT16;
    typedef unsigned char                   UINT8;
    typedef signed long long            INT64;
    typedef signed int                  INT32;
    typedef signed short                    INT16;
    typedef signed char                     INT8;

/* ------------------------------------------------------------------------ */
extern ulong  greg[16];
extern ulong* rreg;             /* Cannot be accessed in -O2 programs! */
/* ------------------------------------------------------------------------ */
#define g0      greg[0]
#define g1      greg[1]
#define g2      greg[2]
#define g3      greg[3]
#define g4      greg[4]
#define g5      greg[5]
#define g6      greg[6]
#define g7      greg[7]
#define g8      greg[8]
#define g9      greg[9]
#define g10     greg[10]
#define g11     greg[11]
#define g12     greg[12]
#define g13     greg[13]
#define g14     greg[14]
#define g15     greg[15]
#define fp      g15

#define        r0      ((ulong*)greg[15])[0]
#define        r1      ((ulong*)greg[15])[1]
#define        r2      ((ulong*)greg[15])[2]
#define        r3      ((ulong*)greg[15])[3]
#define        r4      ((ulong*)greg[15])[4]
#define        r5      ((ulong*)greg[15])[5]
#define        r6      ((ulong*)greg[15])[6]
#define        r7      ((ulong*)greg[15])[7]
#define        r8      ((ulong*)greg[15])[8]
#define        r9      ((ulong*)greg[15])[9]
#define        r10     ((ulong*)greg[15])[10]
#define        r11     ((ulong*)greg[15])[11]
#define        r12     ((ulong*)greg[15])[12]
#define        r13     ((ulong*)greg[15])[13]
#define        r14     ((ulong*)greg[15])[14]
#define        r15     ((ulong*)greg[15])[15]
/* ------------------------------------------------------------------------ */

#include "vdd.h"

/* ------------------------------------------------------------------------ */

int main()
{

	*(UINT64 *)&r8 = 0;
	r11 = 0;
	do {;
        r10 = (UINT32)(*(VDD *)(&V_vddindx + 0 + (r11)*4));
        if (r10 != 0) {;
            if (((VDD *)r10)->pRDD->type == 6) {;
                *(UINT64 *)&r8 += ((VDD *)r10)->devCap >> 11;
            }; 
        }; 
        r11++;
	} while (r11 != 4000);
	*(UINT64 *)&r8 += ((VDD *)r13)->devCap >> 11;

}
/* ------------------------------------------------------------------------ */
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/

