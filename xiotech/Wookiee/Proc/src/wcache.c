/* $Id: wcache.c 159870 2012-09-20 12:59:51Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       wcache.c
**
**  @brief      To provide Cache support.
**
**  Copyright (c) 2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "CT_defines.h"
#include "system.h"
#include "ilt.h"
#include "vrp.h"
#include "vcd.h"

/* ------------------------------------------------------------------------ */
extern UINT32 ct_c$qio;         // Just need the address of the completion routine.

extern UINT32 C_ctv;
extern VCD *vcdIndex[MAX_VIRTUALS];

/* ------------------------------------------------------------------------ */
/* Forward routine definitions. */
void wc_submit(ILT *ilt, void *cr);
/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   wc_submit
**
**  @brief  Submit an I/O to the Cache code.
**
**  This routine accepts an I/O ILT for Cache processing.
**
**  @param  ilt - Pointer to I/O ILT.
**  @param  cr  - Completion routine to call when done.
**
**  @return none
**
***********************************************************************
**/
void wc_submit(ILT *ilt, void *cr)
{
    VRP *vrp = ((VRPCMD*)(&ilt->ilt_normal.w0))->pvrVRP;

    /* Add this Ops OTV to the VTV and CTV for throttling monitoring. */
#ifdef M4_DEBUG_C_ctv
CT_history_printf("%s%s:%u: C_ctv starts=%u ends=%u vc_vtv[%d]=%d\n", FEBEMESSAGE,__FILE__, __LINE__, C_ctv, C_ctv + vrp->vr_otv, vrp->vid, vcdIndex[vrp->vid]->vtv + vrp->vr_otv);
#endif  // M4_DEBUG_C_ctv
    C_ctv += vrp->vr_otv;
    vcdIndex[vrp->vid]->vtv += vrp->vr_otv;

    /* Queue the operation to the cache I/O queue. */
    EnqueueILT((void *)&ct_c$qio, ilt, cr);
}                               /* End of wc_wubmit */

/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
