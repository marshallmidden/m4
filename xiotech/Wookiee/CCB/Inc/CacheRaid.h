/* $Id: CacheRaid.h 145021 2010-08-03 14:16:38Z m4 $*/
/*===========================================================================
** FILE NAME:       CacheRaid.h
** MODULE TITLE:    CCB Cache - RAIDs
**
** DESCRIPTION:     Cached data for RAIDs
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef __CACHERAID_H__
#define __CACHERAID_H__

#include "CacheSize.h"
#include "PacketInterface.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
#ifndef NO_RAID_CACHE
extern UINT8 cacheRaidMap[CACHE_SIZE_RAID_MAP]; /* This is a bit map */
extern UINT8 *cacheRaidAddr[MAX_RAIDS];

extern UINT8 *cacheRaids;
extern UINT8 *cacheTempRaids;
#endif  /* NO_RAID_CACHE */

/*****************************************************************************
** Function prototypes
*****************************************************************************/

extern INT32 CalcSizeRaidsCached(void);
extern UINT16 RaidsCount(void);
#ifndef NO_RAID_CACHE
extern void RaidsGet(void *buf);
extern INT32 RefreshRaids(void);
#endif  /* NO_RAID_CACHE */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __CACHERAID_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
