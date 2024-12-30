#include <assert.h>
#include <stdarg.h>
#include "sgl.h"

// first variable parameter is the dest sgl; nSgl has number of params

extern void          RL_XorSGL (int nSgl, ...);
extern unsigned long RL_CompareSGL (SGL *pSGL1, SGL *pSGL2);
extern unsigned long RL_FastMemCmp (unsigned long *p1, unsigned long *p2, unsigned long size);

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
