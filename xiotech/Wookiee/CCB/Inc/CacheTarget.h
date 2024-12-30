/* $Id: CacheTarget.h 143845 2010-07-07 20:51:58Z mdr $*/
/*===========================================================================
** FILE NAME:       CacheTarget.h
** MODULE TITLE:    CCB Cache - Targets
**
** DESCRIPTION:     Cached data for Targets
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef __CACHETARGET_H__
#define __CACHETARGET_H__

#include "CacheSize.h"
#include "PacketInterface.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
extern UINT16 cacheTargetsCount;
extern UINT8 cacheTargetMap[CACHE_SIZE_TARGET_MAP];
extern UINT8 cacheTargets1[CACHE_SIZE_TARGETS];
extern UINT8 cacheTargets2[CACHE_SIZE_TARGETS];

extern UINT8 *cacheTargets;
extern UINT8 *cacheTempTargets;

/*****************************************************************************
** Function prototypes
*****************************************************************************/
extern INT32 RefreshTargets(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __CACHETARGET_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
