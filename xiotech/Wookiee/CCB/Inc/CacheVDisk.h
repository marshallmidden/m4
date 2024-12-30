/* $Id: CacheVDisk.h 146064 2010-08-20 21:15:33Z m4 $*/
/*===========================================================================
** FILE NAME:       CacheVDisk.h
** MODULE TITLE:    CCB Cache - VDisk
**
** DESCRIPTION:     Cached data for Virtual Disks
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef __CACHEVDISK_H__
#define __CACHEVDISK_H__

#include "CacheSize.h"
#include "PacketInterface.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** Status byte - bit definitions
** Copied from proc code vcd.h
*/

#define VC_CACHED_BIT               0x01    /**< bit 0 = 0 VID not cached
                                                       = 1 VID Cached       */

#define VC_DISABLE_IP_BIT           0x02    /**< bit 1 = 0 Disable not in progress
                                                       = 1 Disable cache in progress    */

#define VC_ERROR_BIT                0x04    /**< bit 2 = 0 Not in error state
                                                       = 1 Error occurred during flush  */

#define VC_MIRROR_WRITE_INFO_BIT    0x08    /**< bit 3 = 0 Do not mirror Write Info
                                                       = 1 Mirror Write Info to partner */

#define VC_COPY_DEST_IP_BIT         0x10    /**< bit 4 = 0 No copy to this VDisk
                                                       = 1 Copy to this VDisk in progress   */

#define VC_REBUILD_REQUIRED_BIT     0x20    /**< bit 5 = 0 No Rebuild required before Mirr
                                                       = 1 Rebuild required before Mirror   */

#define VC_NO_MORE_DATA_BIT         0x40    /**< Bits = 2 and 1 should not allow
                                                 more data to come into cache for
                                                 the VID                          */

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
#ifndef NO_VDISK_CACHE
extern UINT8 cacheVDiskCopyMap[CACHE_SIZE_VDISK_MAP];
extern UINT8 cacheVDiskMirrorMap[CACHE_SIZE_VDISK_MAP];
extern UINT8 cacheVirtualDiskMap[CACHE_SIZE_VDISK_MAP];
extern UINT8 *cacheVirtualDiskAddr[MAX_VIRTUAL_DISKS];

extern UINT8 *cacheVirtualDisks;
extern UINT8 *cacheTempVirtualDisks;
#endif  /* NO_VDISK_CACHE */

/*****************************************************************************
** Public defines - Data structures
*****************************************************************************/
typedef struct _CACHE_VDISK_CACHE_INFO
{
    UINT16      vid;            /* Virtual ID                           */
    UINT8       rsvd2;
    UINT8       stat;           /* Status                               */
    UINT8       rsvd4[4];
    void       *pCache;         /* Data in Cache Tree Ptr               */
    UINT8       rsvd12[4];
} CACHE_VDISK_CACHE_INFO;

/*****************************************************************************
** Function prototypes
*****************************************************************************/
extern PI_VDISKS_RSP *CachedVDisks(void);
extern void GetTenVDisk(UINT16 *, UINT32 *, UINT16 *, UINT32 *, UINT16 *, UINT32 *);
extern INT32 RefreshVDiskCacheInfo(void);

#ifndef NO_VDISK_CACHE
extern void VirtualDisksGet(void *buf);
extern INT32 RefreshVirtualDisks(void);
extern INT32 PI_MakeVDisksCacheDirty(void);
#endif  /* NO_VDISK_CACHE */

#ifndef NO_RAID_CACHE
extern INT32 PI_MakeRaidsCacheDirty(void);
#endif  /* NO_RAID_CACHE */

extern UINT16 VirtualDisksCount(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __CACHEVDISK_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
