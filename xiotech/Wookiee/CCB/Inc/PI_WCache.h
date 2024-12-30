/* $Id: PI_WCache.h 122127 2010-01-06 14:04:36Z m4 $*/
/**
******************************************************************************
**
**  @file       PI_WCache.h
**
**  @brief      Packet Interface and miscellaneous functions for
**              Write Cache Commands
**
**  These functions support the write cache requests.
**
**  Copyright (c) 2001 - 2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef __PI_WCACHE_H__
#define __PI_WCACHE_H__

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern INT32 WCacheInvalidate(UINT32 controllerSN, UINT32 option);
extern INT32 WCacheFlushBEWC(UINT32 controllerSN);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PI_WCACHE_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
