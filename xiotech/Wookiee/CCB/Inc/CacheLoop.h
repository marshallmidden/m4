/* $Id: CacheLoop.h 146064 2010-08-20 21:15:33Z m4 $*/
/*===========================================================================
** FILE NAME:       CacheLoop.h
** MODULE TITLE:    CCB Cache - BE and FE Loop
**
** DESCRIPTION:     Cached data for BE and FE Loop Info
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef __CACHELOOP_H__
#define __CACHELOOP_H__

#include "CacheSize.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

struct _X1_BE_LOOP_INFO_RSP;
struct _PI_STATS_LOOP;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
extern UINT8 *cacheFELoopStatsAddr[MAX_FE_PORTS];
extern UINT8 cacheFELoopStats1[CACHE_SIZE_FE_LOOP_STATS];
extern UINT8 cacheFELoopStats2[CACHE_SIZE_FE_LOOP_STATS];

extern UINT8 *cacheBELoopStatsAddr[MAX_BE_PORTS];
extern UINT8 cacheBELoopStats1[CACHE_SIZE_BE_LOOP_STATS];
extern UINT8 cacheBELoopStats2[CACHE_SIZE_BE_LOOP_STATS];

extern UINT8 *cacheFELoopStats;
extern UINT8 *cacheTempFELoopStats;
extern UINT8 *cacheBELoopStats;
extern UINT8 *cacheTempBELoopStats;

/*****************************************************************************
** Function prototypes
*****************************************************************************/

extern void GetBELoopInfo(struct _X1_BE_LOOP_INFO_RSP *buf);
extern INT32 RefreshBELoopStats(void);
extern INT32 RefreshFELoopStats(void);
extern void SetBEFabricMode(UINT32 port, UINT32 mode);
extern UINT32 GetBEFabricMode(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __CACHELOOP_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
