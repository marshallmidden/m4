/* $Id: ccb_statistics.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       ccb_statistics.h
** MODULE TITLE:    Header file for ccb_statistics.c
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _CCB_STATISTICS_H_
#define _CCB_STATISTICS_H_

#include "i82559.h"
#include "XIO_Types.h"

#include "xk_kernel.h"

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
typedef struct _CCB_STATS_STRUCTURE
{
    UINT32      hrcount;        /* Kernel ticks                 */

    /* Heap Statistics */
    UINT32      memavl;         /* Total DRAM available         */
    UINT32      minavl;         /* Minimum DRAM available       */
    UINT32      maxavl;         /* Maximum DRAM available       */
    UINT32      memwait;        /* Total memory waits           */
    UINT32      memcnt;         /* Outstanding malloc count     */

    /* ILT Statistics */
    UINT32      iltcnt;         /* Total ILTs available         */
    UINT32      iltmax;         /* Maximum ILTs available       */

    /* PCB Statistics */
    UINT32      pcbcnt;         /* Number of active tasks       */

    /* FW Version - why(?) */
    UINT32      fwvers;         /* Firmware version number      */
    UINT32      fwcomp;         /* Firmware revision count      */

    /* Ethernet Statistics */
    UINT32      psent;          /* Ethernet packets sent        */
    UINT32      precv;          /* Ethernet packets received    */
    ETHERNET_STATISTIC_COUNTERS ethernetCounters;

    /* Link Layer Statistics */
    UINT16      vrpOCount;      /* outstanding outbound VRP count */
    UINT16      vrpICount;      /* outstanding inbound VRP count  */
    UINT32      vrpOTotal;      /* total outbound VRP count       */
    UINT32      vrpITotal;      /* total inbound VRP count        */
} CCB_STATS_STRUCTURE;

/*****************************************************************************
** Public variables
*****************************************************************************/
extern CCB_STATS_STRUCTURE CCBStats;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void CCBStatsTask(TASK_PARMS *parms);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _CCB_STATISTICS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
