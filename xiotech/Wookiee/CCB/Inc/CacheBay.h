/* $Id: CacheBay.h 146530 2010-08-27 15:04:46Z m4 $*/
/**
******************************************************************************
**
**  @file   CacheBay.h
**
**  @brief  CCB Cache - Cached data for Disk Bays
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef __CACHEBAY_H__
#define __CACHEBAY_H__

#include "CacheSize.h"
#include "PacketInterface.h"
#include "XIO_Std.h"
#include "X1_Structs.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
extern UINT8  cacheDiskBays1[CACHE_SIZE_DISK_BAYS];
extern UINT8  cacheDiskBays2[CACHE_SIZE_DISK_BAYS];
extern UINT8  cacheDiskBayMap[CACHE_SIZE_DISK_BAY_MAP];
extern UINT8  cacheFibreBayMap[CACHE_SIZE_DISK_BAY_MAP];
extern UINT8  cacheSATABayMap[CACHE_SIZE_DISK_BAY_MAP];
extern UINT8  cacheDiskBayPaths[CACHE_SIZE_DISK_BAY_PATHS];
extern UINT8 *cacheDiskBays;
extern UINT8 *cacheTempDiskBays;

/*****************************************************************************
** Function prototypes
*****************************************************************************/

extern void BayGetInfo(UINT16 bid, X1_BAY_INFO_RSP *buf);

extern void  GetCachedBayMap(void *pBuf);
extern INT32 GetDiskBayInfoFromWwn(UINT64 wwn, MRGETEINFO_RSP * pPInfoOut);
extern INT32 GetDiskBayInfoFromWwnNOW(UINT64 wwn, MRGETEINFO_RSP *);
extern INT32 GetDiskBayInfoFromBid(UINT16 bid, MRGETEINFO_RSP * pPInfoOut);
extern INT32 GetDiskBayInfoFromBidNOW(UINT16 bid, MRGETEINFO_RSP *);
extern INT32 RefreshDiskBays(void);
extern void  GetPortOnRemoteCN(UINT8 port, UINT16 *pRemoteID);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __CACHEBAY_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
