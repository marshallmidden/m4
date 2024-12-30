/* $Id: CacheManager.h 145133 2010-08-05 12:43:28Z m4 $*/
/*===========================================================================
** FILE NAME:       CacheManager.h
** MODULE TITLE:    CCB Cache Manager
**
** DESCRIPTION:     Manager for all cached data on the CCB.
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/

#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include "CacheSize.h"
#include "MR_Defs.h"
#include "PacketInterface.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines - Constants
*****************************************************************************/
#define VLINK_NAME_LEN                  8

/*
** Firmware header types used in the GetFirmwareHeader
** cache lookup function.
**
** NOTE: This list must match the list that is used when
** refreshing the firmware headers in RefreshFirmwareHeaders.
*/
#define FW_HDR_TYPE_CCB_RUNTIME         0
#define FW_HDR_TYPE_BE_RUNTIME          1
#define FW_HDR_TYPE_FE_RUNTIME          2

/* Cache refresh intervals for each system component */
#define CACHE_REFRESH_INTERVAL          5000    /* Time is milliseconds */

#define CACHE_STATE_OK                  0x0000
#define CACHE_STATE_HOLD                0x2000
#define CACHE_STATE_INVALID             0x4000
#define CACHE_STATE_UPDATING            0x8000

enum PI_CACHE_DIRTY_FLAG_LIST
{
    PI_CACHE_INVALIDATE_DISKBAY,            // 0
#ifndef NO_PDISK_CACHE
    PI_CACHE_INVALIDATE_PDISK,              // 1
#else   /* NO_PDISK_CACHE */
    PI_CACHE_INVALIDATE_PDISK_NOTUSED,      // 1    (same as above)
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
    PI_CACHE_INVALIDATE_VDISK,              // 2
#else   /* NO_VDISK_CACHE */
    PI_CACHE_INVALIDATE_VDISK_NOTUSED,      // 2    (same as above)
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
    PI_CACHE_INVALIDATE_RAID,               // 3
#else   /* NO_RAID_CACHE */
    PI_CACHE_INVALIDATE_RAID_NOTUSED,       // 3    (same as above)
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
    PI_CACHE_INVALIDATE_SERVER,             // 4
#else   /* NO_SERVER_CACHE */
    PI_CACHE_INVALIDATE_SERVER_NOTUSED,     // 4    (same as above)
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
    PI_CACHE_INVALIDATE_TARGET,             // 5
#else   /* NO_TARGET_CACHE */
    PI_CACHE_INVALIDATE_TARGET_NOTUSED,     // 5    (same as above)
#endif  /* NO_TARGET_CACHE */
    PI_CACHE_INVALIDATE_ENV,                // 6
#ifndef NO_HAB_CACHE
    PI_CACHE_INVALIDATE_BE_STATS,           // 7
    PI_CACHE_INVALIDATE_FE_STATS,           // 8
    PI_CACHE_INVALIDATE_BE_PORT,            // 9
#else   /* NO_HAB_CACHE */
    PI_CACHE_INVALIDATE_BE_STATS_NOTUSED,   // 7    (same as above)
    PI_CACHE_INVALIDATE_FE_STATS_NOTUSED,   // 8    (same as above)
    PI_CACHE_INVALIDATE_BE_PORT_NOTUSED,    // 9    (same as above)
#endif  /* NO_HAB_CACHE */
#ifndef NO_VCG_CACHE
    PI_CACHE_INVALIDATE_VCG_INFO,           // 10
#else   /* NO_VCG_CACHE */
    PI_CACHE_INVALIDATE_VCG_INFO_NOTUSED,   // 10   (same as above)
#endif  /* NO_VCG_CACHE */
    PI_CACHE_INVALIDATE_WORKSET,            // 11
    PI_CACHE_INVALIDATE_UNUSED12,           // 12
    PI_CACHE_INVALIDATE_FREE_SPACE,         // 13
#ifndef NO_VCD_CACHE
    PI_CACHE_INVALIDATE_VDISK_CACHE_INFO,   // 14
#else   /* NO_VCD_CACHE */
    PI_CACHE_INVALIDATE_VDISK_CACHE_INFO_NOTUSED, // 14 (same as above)
#endif  /* NO_VCD_CACHE */
    PI_CACHE_INVALIDATE_MAXELEMENT          // 15
};

#define CACHE_INVALIDATE_NONE               0x00000000
#define CACHE_INVALIDATE_DISKBAY            (1 << PI_CACHE_INVALIDATE_DISKBAY)          // 0x0001
#ifndef NO_PDISK_CACHE
#define CACHE_INVALIDATE_PDISK              (1 << PI_CACHE_INVALIDATE_PDISK)            // 0x0002
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
#define CACHE_INVALIDATE_VDISK              (1 << PI_CACHE_INVALIDATE_VDISK)            // 0x0004
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
#define CACHE_INVALIDATE_RAID               (1 << PI_CACHE_INVALIDATE_RAID)             // 0x0008
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
#define CACHE_INVALIDATE_SERVER             (1 << PI_CACHE_INVALIDATE_SERVER)           // 0x0010
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
#define CACHE_INVALIDATE_TARGET             (1 << PI_CACHE_INVALIDATE_TARGET)           // 0x0020
#endif  /* NO_TARGET_CACHE */
#define CACHE_INVALIDATE_ENV                (1 << PI_CACHE_INVALIDATE_ENV)              // 0x0040
#ifndef NO_HAB_CACHE
#define CACHE_INVALIDATE_BE_STATS           (1 << PI_CACHE_INVALIDATE_BE_STATS)         // 0x0080
#define CACHE_INVALIDATE_FE_STATS           (1 << PI_CACHE_INVALIDATE_FE_STATS)         // 0x0100
#define CACHE_INVALIDATE_BE_PORT            (1 << PI_CACHE_INVALIDATE_BE_PORT)          // 0x0200
#endif  /* NO_HAB_CACHE */
#ifndef NO_VCD_CACHE
#define CACHE_INVALIDATE_VCG_INFO           (1 << PI_CACHE_INVALIDATE_VCG_INFO)         // 0x0400
#endif  /* NO_VCD_CACHE */
#define CACHE_INVALIDATE_WORKSET            (1 << PI_CACHE_INVALIDATE_WORKSET)          // 0x0800

/* The cache invalidate bit (0x00001000) can be reused. */

#define CACHE_INVALIDATE_FREE_SPACE         (1 << PI_CACHE_INVALIDATE_FREE_SPACE)       // 0x2000
#ifndef NO_VCD_CACHE
#define CACHE_INVALIDATE_VDISK_CACHE_INFO   (1 << PI_CACHE_INVALIDATE_VDISK_CACHE_INFO) // 0x4000
#endif  /* NO_VCD_CACHE */

#define CACHE_STATE_FINISHED            0x80000000

#define CACHE_MAX_REFRESH_MRP_TIMEOUT        100000     /* 100 seconds */
#define REFRESH_VDISKS_INJECT_INTERVAL       15000      /* 15 seconds */

/*****************************************************************************
** Public defines - Macros
*****************************************************************************/

#define X1ValidPid(a)           ((a) < MAX_PHYSICAL_DISKS)
#define X1ValidRid(a)           ((a) < MAX_RAIDS)
#define X1ValidVid(a)           ((a) < MAX_VIRTUAL_DISKS)
#define X1ValidSid(a)           ((a) < MAX_SERVERS)
#define X1ValidTid(a)           ((a) < MAX_TARGETS)
#define X1ValidBid(a)           ((a) < MAX_DISK_BAYS)
#define X1ValidWid(a)           ((a) < MAX_WORKSETS)
#define X1ValidVBlkId(a)        ((a) < MAX_VBLOCKS)
#define X1ValidServerIndex(a)   ((a) < MAX_SERVERS_UNIQUE)
#define X1ValidHABId(a)         ((a) < MAX_HABS)

#define CacheStateSetInUse(a)           ((a)++)
#define CacheStateSetNotInUse(a)        ((a)--)

#define CacheStateOk(a)                 ((a) == CACHE_STATE_OK)
#define CacheStateWaitOk(a)             while (!CacheStateOk(a)) {TaskSleepMS(200);}

#define CacheStateUpdating(a)           (((a) & CACHE_STATE_UPDATING) > 0)
#define CacheStateSetUpdating(a)        ((a) |= CACHE_STATE_UPDATING)
#define CacheStateSetUpdateDone(a)      ((a) &= ~CACHE_STATE_UPDATING)
#define CacheStateWaitUpdating(a)       while (CacheStateUpdating(a)) {TaskSleepMS(200);}

#define CacheStateWaitOkToUpdate(a)     CacheStateWaitOk(((a) & 0x00FF))

#define CacheStateInvalid(a)            ((a) & CACHE_STATE_INVALID) > 0
#define CacheStateSetInvalid(a)         ((a) |= CACHE_STATE_INVALID)
#define CacheStateSetValid(a)           ((a) &= ~CACHE_STATE_INVALID)

#define CacheStateHold(a)               ((a) & CACHE_STATE_HOLD) > 0

#define Invalidate(a,b)              (((a) & (b)) > 0)

#define CacheStateFinishedAdd(a)        (++(a))
#define CacheStateFinishedRemove(a)     (--(a))
#define CacheStateFinished(a)           ((a) & CACHE_STATE_FINISHED) > 0
#define CacheStateSetFinished(a)        (a) |= CACHE_STATE_FINISHED
#define CacheStateWaitFinished(a)       while (!CacheStateFinished(a)) {TaskSleepMS(200);}
#define CacheStateWaitFinishedDone(a)   while (((a) & ~CACHE_STATE_FINISHED) > 0) {TaskSleepMS(200);}

#define SwapPointers(a,b,c,d)        if ((a) == (c)) {(a) = (d); (b) = (c);} \
                                     else        {(a) = (c); (b) = (d);}

#define SwapPointersDiskBays()       SwapPointers(cacheDiskBays,    \
                                                  cacheTempDiskBays,\
                                                  cacheDiskBays1,   \
                                                  cacheDiskBays2)

#ifndef NO_PDISK_CACHE
#define SwapPointersPdisks()         SwapPointers(cachePhysicalDisks,    \
                                                  cacheTempPhysicalDisks,\
                                                  cachePhysicalDisks1,   \
                                                  cachePhysicalDisks2)
#endif  /* NO_PDISK_CACHE */

#ifndef NO_VDISK_CACHE
#define SwapPointersVdisks()         SwapPointers(cacheVirtualDisks,    \
                                                  cacheTempVirtualDisks,\
                                                  cacheVirtualDisks1,   \
                                                  cacheVirtualDisks2)
#endif  /* NO_VDISK_CACHE */

#ifndef NO_RAID_CACHE
#define SwapPointersRaids()          SwapPointers(cacheRaids,    \
                                                  cacheTempRaids,\
                                                  cacheRaids1,   \
                                                  cacheRaids2)
#endif  /* NO_RAID_CACHE */

#ifndef NO_SERVER_CACHE
#define SwapPointersServers()        SwapPointers(cacheServers,    \
                                                  cacheTempServers,\
                                                  cacheServers1,   \
                                                  cacheServers2)
#endif  /* NO_SERVER_CACHE */

#ifndef NO_TARGET_CACHE
#define SwapPointersTargets()        SwapPointers(cacheTargets,    \
                                                  cacheTempTargets,\
                                                  cacheTargets1,   \
                                                  cacheTargets2)
#endif  /* NO_TARGET_CACHE */

#ifndef NO_HAB_CACHE
#define SwapPointersFELoopStats()    SwapPointers(cacheFELoopStats,    \
                                                  cacheTempFELoopStats,\
                                                  cacheFELoopStats1,   \
                                                  cacheFELoopStats2)

#define SwapPointersBELoopStats()    SwapPointers(cacheBELoopStats,    \
                                                  cacheTempBELoopStats,\
                                                  cacheBELoopStats1,   \
                                                  cacheBELoopStats2)

#define SwapPointersBEPortInfo()     SwapPointers(cacheBEPortInfo,    \
                                                  cacheTempBEPortInfo,\
                                                  cacheBEPortInfo1,   \
                                                  cacheBEPortInfo2)
#endif  /* NO_HAB_CACHE */

#ifndef NO_VCG_CACHE
#define SwapPointersVcgInfo()        SwapPointers(cacheVcgInfo,    \
                                                  cacheTempVcgInfo,\
                                                  cacheVcgInfo1,   \
                                                  cacheVcgInfo2)
#endif  /* NO_VCG_CACHE */

#define SwapPointersWorksets()       SwapPointers(cacheWorksetInfo,    \
                                                  cacheTempWorksetInfo,\
                                                  cacheWorksetInfo1,   \
                                                  cacheWorksetInfo2)

#define CacheFEReady()  (cacheFeReady == true)

/*
** CacheInvalidateRC(a)
** inputs
**      a = Mask to invalidate.
** returns
**      GOOD on success
**      ERROR on failure.
*/
#define CacheInvalidateRC(a)   (((InvalidateCache((a), true) & (a)) ^ (a)) > 0)

#define InvalidateCacheDiskBays(a)          InvalidateCache(CACHE_INVALIDATE_DISKBAY, (a))
#ifndef NO_HAB_CACHE
#define InvalidateCacheBE(a)                InvalidateCache(CACHE_INVALIDATE_BE, (a))
#define InvalidateCacheFE(a)                InvalidateCache(CACHE_INVALIDATE_FE, (a))
#endif  /* NO_HAB_CACHE */
#ifndef NO_PDISK_CACHE
#define InvalidateCachePhysicalDisks(a)     InvalidateCache(CACHE_INVALIDATE_PDISK, (a))
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
#define InvalidateCacheVirtualDisks(a)      InvalidateCache(CACHE_INVALIDATE_VDISK, (a))
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
#define InvalidateCacheRaids(a)             InvalidateCache(CACHE_INVALIDATE_RAID, (a))
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
#define InvalidateCacheServers(a)           InvalidateCache(CACHE_INVALIDATE_SERVER, (a))
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
#define InvalidateCacheTargets(a)           InvalidateCache(CACHE_INVALIDATE_TARGET, (a))
#endif  /* NO_TARGET_CACHE */
#define InvalidateCacheEnvironmentals(a)    InvalidateCache(CACHE_INVALIDATE_ENV, (a))
#define InvalidateCacheStats(a)             InvalidateCache(CACHE_INVALIDATE_STATS, (a))

#define InvalidateCacheFreeSpace(a)         (void)InvalidateCache(CACHE_INVALIDATE_FREE_SPACE, (a))

/*****************************************************************************
** Public defines - Data structures
*****************************************************************************/

typedef struct _CACHE_INVALIDATE_QUEUE_RETURN
{
    UINT32      satisfied;
    UINT32      returnMask;
    UINT32      finishedMask;
} CACHE_INVALIDATE_QUEUE_RETURN;

typedef struct _CACHE_INVALIDATE_QUEUE
{
    UINT32      cacheMask;
    CACHE_INVALIDATE_QUEUE_RETURN *pSatisfied;
} CACHE_INVALIDATE_QUEUE;

typedef INT32 (*RefreshCache_func)(void);

/*
** Environmental Information Cache
*/
typedef struct _CACHE_ENV_STATS
{
    INT8        ctrlTempHost;   /* Host side controller temp in degC            */
    INT8        ctrlTempStore;  /* Storage side controller temp in degC         */

    UINT8       ctrlAC1;        /* Controller power supply 1 - b0: 1=good, 0=bad */
    UINT8       ctrlAC2;        /* Controller power supply 2 - b0: 1=good, 0=bad */

    UINT8       ctrlDC1;        /* Ctrl pwr supply 1- b7: 1=exists, 0= NOT exist */
    /*                    b0: 1=good, 0=bad         */
    UINT8       ctrlDC2;        /* Ctrl pwr supply 2- b7: 1=exists, 0= NOT exist */
    /*                    b0: 1=good, 0=bad         */

    UINT8       ctrlFan1;       /* Controller fan 1 - b0: 1=good, 0=bad         */
    UINT8       ctrlFan2;       /* Controller fan 2 - b0: 1=good, 0=bad         */

    UINT8       ctrlBufferHost; /* Host side buffer - b0: 1=good, 0=bad     */
    UINT8       ctrlBufferStore;        /* Storage side buffer - b0: 1=good, 0=bad  */

    UINT8       fibreBayExistBitmap[CACHE_SIZE_DISK_BAY_MAP];   /* Fibre bay exists map */
    UINT8       fibreBayTempIn1[MAX_DISK_BAYS]; /* Fibre bay temp input 1   */
    UINT8       fibreBayTempIn2[MAX_DISK_BAYS]; /* Fibre bay temp input 2   */
    UINT8       fibreBayTempOut1[MAX_DISK_BAYS];        /* Fibre bay temp output 1  */
    UINT8       fibreBayTempOut2[MAX_DISK_BAYS];        /* Fibre bay temp output 2  */

    /*
     ** Fibre bay power supply and fan status
     **  bit7: 1=power supply 2 AC good, 0=fail
     **  bit6: 1=power supply 1 AC good, 0=fail
     **  bit5: 1=power supply 2 DC good, 0=fail
     **  bit4: 1=power supply 1 DC good, 0=fail
     **  bit3: 1=fan 4 good, 0=fail
     **  bit2: 1=fan 3 good, 0=fail
     **  bit1: 1=fan 2 good, 0=fail
     **  bit0: 1=fan 1 good, 0=fail
     */
    UINT8       fibreBayPSFan[MAX_DISK_BAYS];

    UINT32      mbPerSecond;    /* Server MB per second * 100               */
    UINT32      ioPerSecond;    /* Server I/O per second                    */
    UINT16      rsvd;
    UINT32      beHeartbeat;    /* Backend processor heartbeat              */
    UINT32      feHeartbeat;    /* Frontend processor heartbeat             */

    /*
     ** Items below added at X1_COMPATIBILITY = 0x14 (SATA support)
     */

    UINT8       sataBayExistBitmap[CACHE_SIZE_DISK_BAY_MAP]; /**< SATA bay exists map */
    UINT8       sataBayTempOut1[MAX_DISK_BAYS];              /**< SATA bay temp input */
    UINT8       sataBayTempOut2[MAX_DISK_BAYS];              /**< SATA bay temp output */

    /**
    ** SATA bay power supply status             /n
    **  bit3: 1=power supply 2 AC good, 0=fail  /n
    **  bit2: 1=power supply 1 AC good, 0=fail  /n
    **  bit1: 1=power supply 2 DC good, 0=fail  /n
    **  bit0: 1=power supply 1 DC good, 0=fail  /n
    **/
    UINT8       sataBayPS[MAX_DISK_BAYS];

    /**
    ** SATA bay fan status                      /n
    **  bit5: 1=fan 6 good, 0=fail              /n
    **  bit4: 1=fan 5 good, 0=fail              /n
    **  bit3: 1=fan 4 good, 0=fail              /n
    **  bit2: 1=fan 3 good, 0=fail              /n
    **  bit1: 1=fan 2 good, 0=fail              /n
    **  bit0: 1=fan 1 good, 0=fail
    **/
    UINT8       sataBayFan[MAX_DISK_BAYS];

} CACHE_ENV_STATS;

/*
** Data structure for an individual firmware version entry.
*/
typedef struct _X1_VERSION_ENTRY
{
    UINT32      verMajMin;      /* Major (high UINT16) and minor (low UINT16) version */
    UINT32      verSub;         /* Sub-version (if available */
    UINT8       fwCompatIndex;  /* These index parameters match those */
    UINT8       fwBackLevelIndex;       /* defined in the FW_HEADER_LOAD_ID   */
    UINT8       fwSequencingIndex;      /* structure.                         */
    UINT8       reserved[1];
    char        tag[8];         /* ASCII tag field (i.e. M880)              */
} X1_VERSION_ENTRY;

/*
** Configuration Account flag field definitions
*/
#define X1_CFG_EXACT_MIN_PD         0x01        /* 1=exactly minPD, 0= at least minPD */
#define X1_CFG_FORCE_REDUNDANCY     0x02        /* Enforce bay & bus redundancy */

/*
** The flag bit 0x04 is now free-can be reused.
*/
#define X1_CFG_RSVD                 0x08
#define X1_CFG_RSVD_MMC_4           0x10
#define X1_CFG_RSVD_MMC_5           0x20
#define X1_CFG_RSVD_MMC_6           0x40
#define X1_CFG_RSVD_MMC_7           0x80

/*
** Configuration Account Info
*/
typedef struct _X1_CONFIG_ACCOUNT
{
    UINT32      pdiskLocBitmap[MAX_LOCATION_BAYS];
    UINT8       clusters;
    UINT32      pdiskQuota;
    char        name[8];
    char        description[24];
    char        password[8];
    UINT8       r5StripeSize;
    UINT8       r10StripeSize;
    UINT8       r5Parity;
    UINT8       defRaidType;
    UINT16      threshold;
    UINT16      raidsPerCreate;
    UINT8       flags;
    UINT8       depth;
    UINT8       minPD;          /* If non-zero, min. number of PDisks per RAID create   */
} X1_CONFIG_ACCOUNT;

/*****************************************************************************
** Public variables
*****************************************************************************/
extern UINT16 cacheDiskBaysState;
#ifndef NO_PDISK_CACHE
extern UINT16 cachePhysicalDisksState;
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
extern UINT16 cacheVirtualDisksState;
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
extern UINT16 cacheRaidsState;
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
extern UINT16 cacheServersState;
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
extern UINT16 cacheTargetsState;
#endif  /* NO_TARGET_CACHE */
extern UINT16 cacheEnvState;
#ifndef NO_HAB_CACHE
extern UINT16 cacheStatsFeState;
extern UINT16 cacheStatsBeState;
extern UINT16 cacheBEPortState;
#endif  /* NO_HAB_CACHE */
#ifndef NO_VCG_CACHE
extern UINT16 cacheVcgInfoState;
#endif  /* NO_VCG_CACHE */
extern UINT16 cacheWorksetState;
extern UINT16 cacheFreeSpaceState;
#ifndef NO_VCD_CACHE
extern UINT16 cacheVDiskCacheInfoState;
#endif  /* NO_VCD_CACHE */
extern bool cacheFeReady;

extern UINT32 CACHE_INVALIDATE_ALL;
extern UINT32 CACHE_INVALIDATE_BE;
extern UINT32 CACHE_INVALIDATE_FE;
extern UINT32 CACHE_INVALIDATE_STATS;
extern UINT32 CACHE_INVALIDATE_BE_LOOP_INFO;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void CacheMgr_Init(void);
extern void CacheMgr_Start(void);
extern INT32 CalcSizeVDisksCached(void);
extern UINT32 InvalidateCache(UINT32 cacheMask, bool waitForCompletion);
extern void CacheSetRefreshFlag(UINT32 refreshFlags);
extern bool CacheCheckFEReady(void);
extern UINT32 PI_IsCacheDirty(UINT32 mask);
extern void PI_MakeCacheDirty(UINT32 mask);
extern void PI_MakeCacheValid(UINT32 mask);
extern MR_LIST_RSP *CacheGetList(UINT16 listType);
extern void VdisksCacheRefreshInjectTask(TASK_PARMS *parms);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __CACHEMANAGER_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
