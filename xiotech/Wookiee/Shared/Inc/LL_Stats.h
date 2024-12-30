/* $Id: LL_Stats.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       LL_Stats.h
**
**  Copyright (c) 2003-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _LL_STATS_H_
#define _LL_STATS_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** Link layer statistics gathering structure.
**/
typedef struct
{
    UINT16  LL_OutCount;    /**< Count of active outgoing VRPs */
    UINT16  LL_InCount;     /**< Count of active incoming VRPs */
    UINT32  LL_TotOutCount; /**< Total outgoing VRP count */
    UINT32  LL_TotInCount;  /**< Total incoming VRP count */
    UINT16  LL_LastIOp;     /**< Last inbound operation processed */
    UINT16  LL_LastOOp;     /**< Last outbound operation processed */
} LL_STATS;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern LL_STATS *L_stattbl; /**< Link layer statistics structure */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _LL_STATS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
