/* $Id: trace.c 144193 2010-07-15 20:46:51Z m4 $ */
/*============================================================================
** FILE NAME:       trace.c
** MODULE TITLE:    Trace Event
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/

#include "kernel.h"
#include "LargeArrays.h"
#include "trace.h"
#include "XIO_Std.h"
#include "nvram_structure.h"
#include "debug_files.h"
#include "ddr.h"

/*****************************************************************************
** Private variables
*****************************************************************************/
static PROFILE_QUEUE prQueue;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/

/* NOTE: These are in the bss section, and thus are zero upon startup. */
TRACE_QUEUE evQueue;

/*****************************************************************************
** Code Start
*****************************************************************************/

/**********************************************************************
*  Name:        TraceInit( )                                          *
*                                                                     *
*  Description: Initialize the trace buffer. Initially, the runFlag   *
*               is 0, disabling tracing altogether. After it is       *
*               enabled, this initialization routine will be called   *
*               to allocate memory. Once memory is allocated, this    *
*               routine will not be called again.                     *
*                                                                     *
*  Input:       void                                                  *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
void TraceInit(void)
{
    evQueue.evBaseP = evQueue.evNextP = (TRACE_EVENT *)traceBuffer;
    evQueue.evEndP = &evQueue.evBaseP[NUM_TRACE_EVENTS];

    TraceStart();
}


/**********************************************************************
*  Name:        CopyTraceDataToNVRAM()                                *
*                                                                     *
*  Description: Copy trace records to NVRAM on errortrap etc.         *
*                                                                     *
*  Input:       nvramP - pointer to region in NVRAM to copy to        *
*               length - the length of the NVRAM region               *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
void CopyTraceDataToNVRAM(UINT8 *nvramP, UINT32 length)
{
    INT32       evCount;
    INT32       evStart;
    TRACE_EVENT *cpyThis;

    DDR_FID_HEADER *pFidHdr = (DDR_FID_HEADER *)nvramP;
    UINT8      *pCopyTo = (UINT8 *)&pFidHdr[1];

    /*
     * Initialize header
     */
    memset(pFidHdr, 0, sizeof(*pFidHdr));
    pFidHdr->magicNumber = DDR_FID_HEADER_MAGIC_NUM;
    pFidHdr->fid = CCB_FID + CCB_TRACEBUF;
    pFidHdr->version = FetchFIDVersion(pFidHdr->fid);
    strncpy(pFidHdr->id, ccbDdrTable.entry[CCB_TRACEBUF].id, 8);

    /* Calculate num events to copy */
    evCount = (length - sizeof(*pFidHdr)) / sizeof(*cpyThis);

    /* Figure which event to copy first */
    evStart = (((INT32)evQueue.evNextP - (INT32)evQueue.evBaseP) / sizeof(*cpyThis)) - evCount;

    /* If underflow, wrap to end */
    if (evStart < 0)
    {
        evStart += ((INT32)evQueue.evEndP - (INT32)evQueue.evBaseP) / sizeof(*cpyThis);
    }

    /* Initialize pointer to first event to copy */
    cpyThis = &evQueue.evBaseP[evStart];

    /* Byte copy events to nvram */
    while (evCount--)
    {
        if (cpyThis->id)
        {
            pCopyTo += MemCpyBytes(pCopyTo, cpyThis, sizeof(*cpyThis));
        }

        if (++cpyThis == evQueue.evEndP)
        {
            cpyThis = evQueue.evBaseP;
        }
    }
}


/**********************************************************************
*  Name:        ProfileInit( )                                        *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
void ProfileInit(char *bufP)
{
    prQueue.prBaseP = prQueue.prNextP = (PROFILE_EVENT *)(bufP + sizeof(PROFILE_TIME));

    prQueue.prEndP = &prQueue.prBaseP[NUM_PROFILE_EVENTS];
    prQueue.prRunFlag = 1;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
