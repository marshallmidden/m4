/* $Id: ccb_statistics.c 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       ccb_statistics.c
** MODULE TITLE:    Statistics gathering for CCB runtime code
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "ccb_statistics.h"

#include "FW_Header.h"
#include "led.h"
#include "debug_files.h"
#include "heap.h"
#include "LL_Stats.h"
#include "timer.h"
#include "rtc.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "memory.h"

/*****************************************************************************
** Public variables externed in header file
*****************************************************************************/
CCB_STATS_STRUCTURE CCBStats;

extern struct fmm K_ncdram;         /* Free shared memory management structure  */
/*****************************************************************************
** Code Start
*****************************************************************************/

/******************************************************************************
** NAME:        CCBStatsTask
**
** PURPOSE:     To collect statistics for the CCB.
**
** DESCRIPTION: This task collects statistics and updates the local statistics
**              table, <P_stattbl>.  This task places itself into timer 0 wait
**              for 1000 milliseconds and collects the statistics at this rate.
**
** INPUT:       Statistics from <P_stattbl> and other sources.
**
** OUTPUT:      Primary Statistic Table copied to Magnitude memory
**              (<stloc> in BIT is nonzero)
#*****************************************************************************/
NORETURN void CCBStatsTask(UNUSED TASK_PARMS *parms)
{
    FW_HEADER  *firmwareHeaderPtr = (FW_HEADER *)CCB_RUNTIME_FW_HEADER_ADDR;

    /* Save the FW version and compile information. */
    CCBStats.fwvers = firmwareHeaderPtr->revision;
    CCBStats.fwcomp = firmwareHeaderPtr->revCount;

    /* Main statistics loop */
    while (1)
    {
        /* Update the heap statistics */
        CCBStats.memavl = K_ncdram.fmm_fms->fms_Available_memory;
        CCBStats.minavl = K_ncdram.fmm_fms->fms_Minimum_available;
        CCBStats.maxavl = K_ncdram.fmm_fms->fms_Maximum_available;
        CCBStats.memwait = 0;           // no longer valid
        CCBStats.memcnt = 0;            // no longer valid

        /* Update kernel statistics */
        CCBStats.pcbcnt = 0;    /* Update the number of active tasks - does not apply on Wookiee */
        CCBStats.hrcount = K_timel;     /* Update the "total seconds uptime" field  */
        CCBStats.iltcnt = 0;    /* Update the current ILT useage -- does not apply on Wookiee */
        CCBStats.iltmax = 0;    /* Update the maximum ILT useage -- does not apply on Wookiee */

        /* Update Link Layer statistics */
        CCBStats.vrpOCount = L_stattbl->LL_OutCount;
        CCBStats.vrpICount = L_stattbl->LL_InCount;
        CCBStats.vrpOTotal = L_stattbl->LL_TotOutCount;
        CCBStats.vrpITotal = L_stattbl->LL_TotInCount;

        /*
         * Ethernet packet send/receive statistics are updated in real-time by
         * the software.  Call EthernetGetStatistics to pull the i82559
         * internal counters directly from the device.  Do not reset the
         * i82559 internal counters.
         */
        EthernetGetStatistics(&CCBStats.ethernetCounters, FALSE);

        /* Send an informational message when the link layer gets jammed (wedged?) */
        if (CCBStats.vrpOCount >= 15)
        {
            dprintf(DPRINTF_DEFAULT, "CCB Link Layer: %hu outbound VRPs queued\n",
                    CCBStats.vrpOCount);
        }
        if (CCBStats.vrpICount >= 15)
        {
            dprintf(DPRINTF_DEFAULT, "CCB Link Layer: %hu inbound VRPs queued (0x%x)\n",
                    CCBStats.vrpICount, L_stattbl->LL_LastIOp);
        }

        /* Sleep for five seconds before updating the statistics again */
        TaskSleepMS(5000);
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
