/* $Id: CachePDisk.h 159129 2012-05-12 06:25:16Z marshall_midden $*/
/**
******************************************************************************
**
**  @file   CachePDisk.h
**
**  @brief  CCB Cache - Cached data for Physical Disks
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef __CACHEPDISK_H__
#define __CACHEPDISK_H__

#include "CacheSize.h"
#include "PacketInterface.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
extern UINT8 *cachePhysicalDiskAddr[MAX_PHYSICAL_DISKS];
extern UINT8 cachePDiskMap[CACHE_SIZE_PDISK_MAP];       /* This is a bit map */
extern UINT8 cachePDiskFailMap[CACHE_SIZE_PDISK_MAP];   /* This is a bit map */
extern UINT8 cachePDiskRebuildMap[CACHE_SIZE_PDISK_MAP];        /* This is a bit map */
extern UINT8 cachePDiskPaths[CACHE_SIZE_PDISK_PATHS];

extern UINT8 *cachePhysicalDisks;
extern UINT8 *cacheTempPhysicalDisks;

/*****************************************************************************
** Public defines - Data structures
*****************************************************************************/

/*
** Cache Good Device Structure
*/
typedef struct cacheGoodDevice_t
{
    UINT16      pid;            /* PID for this device                  */
    UINT16      ses;            /* Current enclosure PID                */
    UINT8       slot;           /* Current slot number n enclosure      */
    UINT8       rsvd1[3];       /* Reserved                             */
    UINT8       serial[12];     /* serial #                             */
    UINT64      wwn;            /* WWN for this device                  */
} CACHE_GOOD_DEVICE;

/*****************************************************************************
** Function prototypes
*****************************************************************************/
extern INT32 CalcSizePDisksCached(void);
extern UINT16 PhysicalDisksCount(void);
extern void PhysicalDisksGet(void *buf);
extern void PhysicalDiskGet(UINT16 pid, MRGETPINFO_RSP * pPhyDevOut);
extern INT32 GetPDiskInfoFromPid(UINT16 pid, MRGETPINFO_RSP * pPInfoOut);
extern INT32 GetPDiskGoodDeviceFromWwn(UINT64 wwn, CACHE_GOOD_DEVICE *pGoodDeviceOut);
extern INT32 GetPDiskGoodDeviceFromWwnNOW(UINT64 wwn, CACHE_GOOD_DEVICE *);
extern void GetTenPDisk(UINT16 *, UINT32 *, UINT16 *, UINT32 *, UINT16 *, UINT32 *);

#if defined(MODEL_7000) || defined(MODEL_4700)
extern INT32 GetPDiskGoodDeviceFromPid(UINT16 pid, CACHE_GOOD_DEVICE *pGoodDeviceOut);
extern INT32 GetPDiskGoodDeviceFromPidNOW(UINT16 pid, CACHE_GOOD_DEVICE *pGoodDeviceOut);
#endif /* MODEL_7000 || MODEL_4700 */
extern void RemovePdiskFromGoodPdisks(UINT64 wwn);
extern INT32 RefreshPhysicalDisks(void);
extern INT32 PI_MakePDisksCacheDirty(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __CACHEPDISK_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
