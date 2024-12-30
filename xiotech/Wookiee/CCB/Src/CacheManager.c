/* $Id: CacheManager.c 158811 2011-12-20 20:42:56Z m4 $ */
/*===========================================================================
** FILE NAME:       CacheManager.c
** MODULE TITLE:    CCB Cache Manager
**
** DESCRIPTION:     Manager for all cached data on the CCB.
**
** Copyright (c) 2002-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#include "CacheManager.h"

#include "CacheBay.h"
#include "CacheLoop.h"
#include "CacheMisc.h"
#include "CachePDisk.h"
#include "CacheRaid.h"
#include "CacheServer.h"
#include "CacheTarget.h"
#include "CacheVDisk.h"
#include "cps_init.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "LargeArrays.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "pcb.h"
#include "PI_Utils.h"
#include "quorum_utils.h"
#include "X1_AsyncEventHandler.h"
#include "X1_Structs.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

/* #define DEBUG_CACHE_INVALIDATE */

/* #define DEBUG_CACHE_INVALIDATE_RESOLVE */

/* #define CACHE_DEBUG_SERVER_MASK */

#define CacheRefreshFlagGet(a)       (cacheRefreshFlags & (a))
#define CacheRefreshFlagSet(a)       (cacheRefreshFlags |= (a))
#define CacheRefreshFlagDoneSet(a)   (cacheRefreshFlags &= ~(a))

/*****************************************************************************
** Public variables
*****************************************************************************/
UINT16      cacheDiskBaysState = CACHE_STATE_OK;
#ifndef NO_PDISK_CACHE
UINT16      cachePhysicalDisksState = CACHE_STATE_OK;
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
UINT16      cacheVirtualDisksState = CACHE_STATE_OK;
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
UINT16      cacheRaidsState = CACHE_STATE_OK;
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
UINT16      cacheServersState = CACHE_STATE_OK;
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
UINT16      cacheTargetsState = CACHE_STATE_OK;
#endif  /* NO_TARGET_CACHE */
UINT16      cacheEnvState = CACHE_STATE_OK;
UINT16      cacheBEPortState = CACHE_STATE_OK;
#ifndef NO_VCG_CACHE
UINT16      cacheVcgInfoState = CACHE_STATE_OK;
#endif  /* NO_VCG_CACHE */
UINT16      cacheStatsFeState = CACHE_STATE_OK;
UINT16      cacheStatsBeState = CACHE_STATE_OK;
UINT16      cacheWorksetState = CACHE_STATE_OK;
UINT16      cacheFreeSpaceState = CACHE_STATE_OK;
#ifndef NO_VCD_CACHE
UINT16      cacheVDiskCacheInfoState = CACHE_STATE_OK;
#endif  /* NO_VCD_CACHE */

bool        cacheFeReady = false;

/* The following due to original being a #define, and conditionalizing for
 * NO_RAID_CACHE, etc. was ugly. */
UINT32      CACHE_INVALIDATE_ALL;
UINT32      CACHE_INVALIDATE_BE;
UINT32      CACHE_INVALIDATE_FE;
UINT32      CACHE_INVALIDATE_STATS;
UINT32      CACHE_INVALIDATE_BE_LOOP_INFO;

/*****************************************************************************
** Private variables
*****************************************************************************/
static CACHE_INVALIDATE_QUEUE_RETURN *pGlobalInvalidReturn = NULL;
static CACHE_INVALIDATE_QUEUE cacheInvalidQueue;

static bool cacheInvalidQueueEmpty = true;
static PCB *pInvalidCachePcb = NULL;
static UINT32 cacheRefreshFlags = CACHE_INVALIDATE_NONE;

static PCB *cacheDiskBaysPcb = 0;
#ifndef NO_PDISK_CACHE
static PCB *cachePhysicalDisksPcb = 0;
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
static PCB *cacheVirtualDisksPcb = 0;
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
static PCB *cacheRaidsPcb = 0;
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
static PCB *cacheServersPcb = 0;
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
static PCB *cacheTargetsPcb = 0;
#endif  /* NO_TARGET_CACHE */
static PCB *cacheEnvPcb = 0;
static PCB *cacheStatsFePcb = 0;
static PCB *cacheStatsBePcb = 0;
static PCB *cacheBEPortPcb = 0;
#ifndef NO_VCG_CACHE
static PCB *cacheVcgInfoPcb = 0;
#endif  /* NO_VCG_CACHE */
static PCB *cacheWorksetPcb = 0;
static PCB *cacheFreeSpacePcb = 0;
#ifndef NO_VCD_CACHE
static PCB *cacheVDiskCacheInfoPcb = 0;
#endif  /* NO_VCD_CACHE */
static UINT32 piCacheMask = 0;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void CacheRefreshTask(TASK_PARMS *parms);
static void CacheMgr_InitCacheTask(TASK_PARMS *parms);
static void ProcessInvalidCacheQueueTask(TASK_PARMS *parms);
static bool DequeueInvalidCache(CACHE_INVALIDATE_QUEUE *invalidQueue);
static void InvalidateCacheTask(TASK_PARMS *parms);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    CacheMgr_Init
**
** Description: Initializes the the cache manager variables.
**
** Inputs:      NONE
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void CacheMgr_Init(void)
{
    dprintf(DPRINTF_CACHEMGR, "CacheMgr_Init - ENTER\n");

    /* Initialize the account information */
    memset(&cacheAccountInfo, 0x00, sizeof(cacheAccountInfo));

    /* Initialize the free space information */
    memset(&cacheFreeSpace, 0x00, sizeof(cacheFreeSpace));

    /* Initialize our pointers */
    cacheDiskBays = cacheDiskBays1;
    cacheTempDiskBays = cacheDiskBays2;
#ifndef NO_PDISK_CACHE
    cachePhysicalDisks = cachePhysicalDisks1;
    cacheTempPhysicalDisks = cachePhysicalDisks2;
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
    cacheVirtualDisks = cacheVirtualDisks1;
    cacheTempVirtualDisks = cacheVirtualDisks2;
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
    cacheRaids = cacheRaids1;
    cacheTempRaids = cacheRaids2;
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
    cacheServers = cacheServers1;
    cacheTempServers = cacheServers2;
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
    cacheTargets = cacheTargets1;
    cacheTempTargets = cacheTargets2;
#endif  /* NO_TARGET_CACHE */
#ifndef NO_HAB_CACHE
    cacheFELoopStats = cacheFELoopStats1;
    cacheTempFELoopStats = cacheFELoopStats2;

    cacheBELoopStats = cacheBELoopStats1;
    cacheTempBELoopStats = cacheBELoopStats2;

    cacheTempBEPortInfo = cacheBEPortInfo2;
    cacheBEPortInfo = cacheBEPortInfo1;
#endif  /* NO_HAB_CACHE */
#ifndef NO_VCG_CACHE
    cacheVcgInfo = cacheVcgInfo1;
    cacheTempVcgInfo = cacheVcgInfo2;
#endif  /* NO_VCG_CACHE */

    cacheWorksetInfo = cacheWorksetInfo1;
    cacheTempWorksetInfo = cacheWorksetInfo2;

    dprintf(DPRINTF_CACHEMGR, "CacheMgr_Init - EXIT\n");
}

/*----------------------------------------------------------------------------
** Function:    CacheMgr_Start
**
** Description: Starts the cache manager.
**
** Inputs:      NONE
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void CacheMgr_Start(void)
{
    TASK_PARMS  parms;

    dprintf(DPRINTF_CACHEMGR, "CacheMgr_Start - ENTER\n");

    /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    /* Set CACHE_INVALIDATE_FE to all valid caches. */
    CACHE_INVALIDATE_FE =
#ifndef NO_SERVER_CACHE
                          CACHE_INVALIDATE_SERVER |
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
                          CACHE_INVALIDATE_TARGET |
#endif  /* NO_TARGET_CACHE */
                          0;
    /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    /* Set CACHE_INVALIDATE_ALL to all valid caches. */
    CACHE_INVALIDATE_ALL =
                           CACHE_INVALIDATE_DISKBAY |
                           CACHE_INVALIDATE_WORKSET |
                           CACHE_INVALIDATE_FREE_SPACE |
                           CACHE_INVALIDATE_ENV |
#ifndef NO_PDISK_CACHE
                           CACHE_INVALIDATE_PDISK |
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
                           CACHE_INVALIDATE_VDISK |
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
                           CACHE_INVALIDATE_RAID |
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
                           CACHE_INVALIDATE_SERVER |
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
                           CACHE_INVALIDATE_TARGET |
#endif  /* NO_TARGET_CACHE */
#ifndef NO_HAB_CACHE
                           CACHE_INVALIDATE_BE_PORT |
                           CACHE_INVALIDATE_BE_STATS |
                           CACHE_INVALIDATE_FE_STATS |
#endif  /* NO_HAB_CACHE */
#ifndef NO_VCG_CACHE
                           CACHE_INVALIDATE_VCG_INFO |
#endif  /* NO_VCG_CACHE */
#ifndef NO_VCD_CACHE
                           CACHE_INVALIDATE_VDISK_CACHE_INFO |
#endif  /* NO_VCD_CACHE */
                           0;
    /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    /* Set CACHE_INVALIDATE_BE to all valid caches. */
    CACHE_INVALIDATE_BE =
                          CACHE_INVALIDATE_DISKBAY |
                          CACHE_INVALIDATE_FREE_SPACE |
#ifndef NO_PDISK_CACHE
                          CACHE_INVALIDATE_PDISK |
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
                          CACHE_INVALIDATE_VDISK |
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
                          CACHE_INVALIDATE_RAID |
#endif  /* NO_RAID_CACHE */
#ifndef NO_HAB_CACHE
                          CACHE_INVALIDATE_BE_PORT |
                          CACHE_INVALIDATE_BE_STATS |
#endif  /* NO_HAB_CACHE */
#ifndef NO_VCD_CACHE
                          CACHE_INVALIDATE_VDISK_CACHE_INFO |
#endif  /* NO_VCD_CACHE */
                          0;
    /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    /* Set CACHE_INVALIDATE_STATS to all valid caches. */
    CACHE_INVALIDATE_STATS =
#ifndef NO_HAB_CACHE
                             CACHE_INVALIDATE_BE_STATS |
                             CACHE_INVALIDATE_FE_STATS |
#endif  /* NO_HAB_CACHE */
#ifndef NO_VCD_CACHE
                             CACHE_INVALIDATE_VDISK_CACHE_INFO |
#endif  /* NO_VCD_CACHE */
                             0;
    /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    /* Set CACHE_INVALIDATE_BE_LOOP_INFO to all valid caches. */
    CACHE_INVALIDATE_BE_LOOP_INFO =
                                    CACHE_INVALIDATE_DISKBAY |
#ifndef NO_HAB_CACHE
                                    CACHE_INVALIDATE_BE_STATS |
                                    CACHE_INVALIDATE_BE_PORT |
#endif  /* NO_HAB_CACHE */
                                    0;
    /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

    /*
     * Refresh the firmware header cache information immediately
     * since it is available and required early.
     */
    RefreshFirmwareHeaders();

    /* Fork the tasks. */
    (void)TaskCreate(CacheRefreshTask, NULL);

    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheDiskBaysState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_DISKBAY;
    parms.p4 = (UINT32)&cacheDiskBaysPcb;
    parms.p5 = (UINT32)RefreshDiskBays;
    cacheDiskBaysPcb = TaskCreate(InvalidateCacheTask, &parms);

#ifndef NO_PDISK_CACHE
    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cachePhysicalDisksState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_PDISK;
    parms.p4 = (UINT32)&cachePhysicalDisksPcb;
    parms.p5 = (UINT32)RefreshPhysicalDisks;
    cachePhysicalDisksPcb = TaskCreate(InvalidateCacheTask, &parms);
#endif  /* NO_PDISK_CACHE */

#ifndef NO_VDISK_CACHE
    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheVirtualDisksState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_VDISK;
    parms.p4 = (UINT32)&cacheVirtualDisksPcb;
    parms.p5 = (UINT32)RefreshVirtualDisks;
    cacheVirtualDisksPcb = TaskCreate(InvalidateCacheTask, &parms);
#endif  /* NO_VDISK_CACHE */

#ifndef NO_RAID_CACHE
    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheRaidsState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_RAID;
    parms.p4 = (UINT32)&cacheRaidsPcb;
    parms.p5 = (UINT32)RefreshRaids;
    cacheRaidsPcb = TaskCreate(InvalidateCacheTask, &parms);
#endif  /* NO_RAID_CACHE */

#ifndef NO_SERVER_CACHE
    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheServersState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_SERVER;
    parms.p4 = (UINT32)&cacheServersPcb;
    parms.p5 = (UINT32)RefreshServers;
    cacheServersPcb = TaskCreate(InvalidateCacheTask, &parms);
#endif  /* NO_SERVER_CACHE */

#ifndef NO_TARGET_CACHE
    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheTargetsState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_TARGET;
    parms.p4 = (UINT32)&cacheTargetsPcb;
    parms.p5 = (UINT32)RefreshTargets;
    cacheTargetsPcb = TaskCreate(InvalidateCacheTask, &parms);
#endif  /* NO_TARGET_CACHE */

    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheEnvState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_ENV;
    parms.p4 = (UINT32)&cacheEnvPcb;
    parms.p5 = (UINT32)RefreshEnvironmentals;
    cacheEnvPcb = TaskCreate(InvalidateCacheTask, &parms);

#ifndef NO_HAB_CACHE
    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheStatsFeState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_FE_STATS;
    parms.p4 = (UINT32)&cacheStatsFePcb;
    parms.p5 = (UINT32)RefreshFELoopStats;
    cacheStatsFePcb = TaskCreate(InvalidateCacheTask, &parms);

    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheStatsBeState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_BE_STATS;
    parms.p4 = (UINT32)&cacheStatsBePcb;
    parms.p5 = (UINT32)RefreshBELoopStats;
    cacheStatsBePcb = TaskCreate(InvalidateCacheTask, &parms);

    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheBEPortState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_BE_PORT;
    parms.p4 = (UINT32)&cacheBEPortPcb;
    parms.p5 = (UINT32)RefreshBEPortInfo;
    cacheBEPortPcb = TaskCreate(InvalidateCacheTask, &parms);
#endif  /* NO_HAB_CACHE */

#ifndef NO_VCG_CACHE
    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheVcgInfoState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_VCG_INFO;
    parms.p4 = (UINT32)&cacheVcgInfoPcb;
    parms.p5 = (UINT32)RefreshVcgInfo;
    cacheVcgInfoPcb = TaskCreate(InvalidateCacheTask, &parms);
#endif  /* NO_VCG_CACHE */

    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheWorksetState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_WORKSET;
    parms.p4 = (UINT32)&cacheWorksetPcb;
    parms.p5 = (UINT32)RefreshWorksets;
    cacheWorksetPcb = TaskCreate(InvalidateCacheTask, &parms);

    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheFreeSpaceState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_FREE_SPACE;
    parms.p4 = (UINT32)&cacheFreeSpacePcb;
    parms.p5 = (UINT32)CacheFreeSpaceRefresh;
    cacheFreeSpacePcb = TaskCreate(InvalidateCacheTask, &parms);

#ifndef NO_VCD_CACHE
    parms.p1 = (UINT32)InvalidateCacheTask;
    parms.p2 = (UINT32)&cacheVDiskCacheInfoState;
    parms.p3 = (UINT32)CACHE_INVALIDATE_VDISK_CACHE_INFO;
    parms.p4 = (UINT32)&cacheVDiskCacheInfoPcb;
    parms.p5 = (UINT32)RefreshVDiskCacheInfo;
    cacheVDiskCacheInfoPcb = TaskCreate(InvalidateCacheTask, &parms);
#endif  /* NO_VCD_CACHE */

    dprintf(DPRINTF_CACHE_INVALIDATE, "CacheMgr_Start: Forking Invalidate process\n");

    pInvalidCachePcb = TaskCreate(ProcessInvalidCacheQueueTask, NULL);

    /* Initialize the cache. */
    (void)TaskCreate(CacheMgr_InitCacheTask, NULL);

    dprintf(DPRINTF_CACHEMGR, "CacheMgr_Start - EXIT\n");
}

/*----------------------------------------------------------------------------
** Function:    CacheMgr_InitCacheTask
**
** Description: Initializes the cache.
**
** Inputs:      None (Forked).
**
**--------------------------------------------------------------------------*/
NORETURN static void CacheMgr_InitCacheTask(UNUSED TASK_PARMS *parms)
{
    UINT32      initCache = CACHE_INVALIDATE_ALL;
    UINT32      initCacheResp = CACHE_INVALIDATE_NONE;

    dprintf(DPRINTF_DEFAULT, "CacheMgr_InitCacheTask: Initializing Cache.\n");

#ifndef NO_PDISK_CACHE
    /* First priority is to take care of the pdisks. */
    while (CacheInvalidateRC(CACHE_INVALIDATE_PDISK) == ERROR)
    {
        TaskSleepMS(20);
    }
#endif  /* NO_PDISK_CACHE */

    /* Send a PCHANGE to the XIOservice. */
    EnqueueX1AsyncNotification(CACHE_INVALIDATE_NONE, X1_ASYNC_PCHANGED);

#ifndef NO_HAB_CACHE
    do
    {
        (void)InvalidateCache(initCache, true);
        TaskSleepMS(1000);
    } while (RefreshHABs() != GOOD);
#endif  /* NO_HAB_CACHE */

    /* Keep the cache initialized until power up. */
    do
    {
        (void)InvalidateCache(initCache, true);
        TaskSleepMS(1000);
    } while (!PowerUpBEReady() || !CacheCheckFEReady());

    /* Loop until all the cache is initialized. */
    do
    {
        initCacheResp = InvalidateCache(initCache, true);
        initCache ^= initCacheResp;
        TaskSleepMS(1000);
    } while (initCache != GOOD);

    LogMessage(LOG_TYPE_DEBUG, "CACHE-Initialization COMPLETE");

    /* Send the change events. */
    SendX1ChangeEvent(X1_ASYNC_CONFIG_ALL);

    /* If we are the Master, see if any raids are initializing. */
    if (TestforMaster(GetMyControllerSN()))
    {
        X1_MonitorRaidInitialization();
    }

    /* Forever, print ten minute interval statistics. */
    for (;;)
    {
        UINT8   air_temp;
        UINT8   cpu_temp;
        UINT32  mbPerSecond;
        UINT32  ioPerSecond;
        UINT16  max_vdisk_qd;
        UINT32  max_v_qd;       // qd
        UINT16  max_vdisk_mbps;
        UINT32  max_v_mbps;     // spsCnt - Sample period sector count
        UINT16  max_vdisk_iops;
        UINT32  max_v_iops;     // sprCnt- Sample period request count
        UINT16  max_pdisk_qd;
        UINT32  max_p_qd;
        UINT16  max_pdisk_mbps;
        UINT32  max_p_mbps;
        UINT16  max_pdisk_iops;
        UINT32  max_p_iops;

        CacheSetRefreshFlag(CACHE_INVALIDATE_ENV);      /* Make sure Environmentals updated every so often. */
        GetTenEnv(&air_temp, &cpu_temp, &mbPerSecond, &ioPerSecond);
        GetTenVDisk(&max_vdisk_qd, &max_v_qd, &max_vdisk_mbps, &max_v_mbps, &max_vdisk_iops, &max_v_iops);
        GetTenPDisk(&max_pdisk_qd, &max_p_qd, &max_pdisk_mbps, &max_p_mbps, &max_pdisk_iops, &max_p_iops);
        dprintf(DPRINTF_DEFAULT, ",temp,%u,%u,mbps,%0.2f,iops,%u,vqd,%u,%u,vmb,%u,%0.2f,vio,%u,%u,pqd,%u,%u,pmb,%u,%0.2f,pio,%u,%u\n",
            (UINT32)(air_temp*(9.0/5.0) + 32.0), (UINT32)(cpu_temp*(9.0/5.0) + 32.0),
            mbPerSecond / 100.0, ioPerSecond,
            max_vdisk_qd, max_v_qd, max_vdisk_mbps, max_v_mbps / 100.0, max_vdisk_iops, max_v_iops,
            max_pdisk_qd, max_p_qd, max_pdisk_mbps, max_p_mbps / 100.0, max_pdisk_iops, max_p_iops);
        TaskSleepMS(10 * 60 * 1000);    // 10 minutes * 60 seconds * 1000 ms
    }
}

/*----------------------------------------------------------------------------
** Function:    ProcessX1AsyncTable
**
** Description: Forked task to process X1 async Notifications.
**
** Inputs:      Dummy   -   Makes it forkable
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static NORETURN void ProcessInvalidCacheQueueTask(UNUSED TASK_PARMS *parms)
{
    CACHE_INVALIDATE_QUEUE invalid;
    UINT32      cacheMask;
    UINT32      sequence;
    PCB        *pPCB = NULL;

    /*
     * Loop forever and ever and ever and ever and ever...
     */
    while (1)
    {
        dprintf(DPRINTF_CACHE_INVALIDATE, "ProcessInvalidCacheQueueTask: LOOP ENTER\n");

        /*
         * While we have items on the queue, process those items
         */
        while (DequeueInvalidCache(&invalid))
        {
            cacheMask = invalid.cacheMask;
            sequence = invalid.pSatisfied->satisfied;
            pGlobalInvalidReturn = invalid.pSatisfied;

            dprintf(DPRINTF_CACHE_INVALIDATE, "ProcessInvalidCacheQueueTask: 0x%08X cacheMask 0x%08X\n",
                    sequence, cacheMask);

            /*
             * With this first round of if statements we are going to
             * wake up all the necessary tasks that we need to do the
             * invalidation.
             */

            if (Invalidate(cacheMask, CACHE_INVALIDATE_DISKBAY))
            {
                pPCB = (PCB *)cacheDiskBaysPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_DISKBAY;
                }
            }

#ifndef NO_PDISK_CACHE
            if (Invalidate(cacheMask, CACHE_INVALIDATE_PDISK))
            {
                pPCB = (PCB *)cachePhysicalDisksPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_PDISK;
                }
            }
#endif  /* NO_PDISK_CACHE */

#ifndef NO_VDISK_CACHE
            if (Invalidate(cacheMask, CACHE_INVALIDATE_VDISK))
            {
                pPCB = (PCB *)cacheVirtualDisksPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_VDISK;
                }
            }
#endif  /* NO_VDISK_CACHE */

#ifndef NO_RAID_CACHE
            if (Invalidate(cacheMask, CACHE_INVALIDATE_RAID))
            {
                pPCB = (PCB *)cacheRaidsPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_RAID;
                }
            }
#endif  /* NO_RAID_CACHE */

#ifndef NO_SERVER_CACHE
            if (Invalidate(cacheMask, CACHE_INVALIDATE_SERVER))
            {
                pPCB = (PCB *)cacheServersPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_SERVER;
                }
            }
#endif  /* NO_SERVER_CACHE */

#ifndef NO_TARGET_CACHE
            if (Invalidate(cacheMask, CACHE_INVALIDATE_TARGET))
            {
                pPCB = (PCB *)cacheTargetsPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_TARGET;
                }
            }
#endif  /* NO_TARGET_CACHE */

            if (Invalidate(cacheMask, CACHE_INVALIDATE_ENV))
            {
                pPCB = (PCB *)cacheEnvPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_ENV;
                }
            }


#ifndef NO_HAB_CACHE
            if (Invalidate(cacheMask, CACHE_INVALIDATE_BE_STATS))
            {
                pPCB = (PCB *)cacheStatsBePcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_BE_STATS;
                }
            }

            if (Invalidate(cacheMask, CACHE_INVALIDATE_FE_STATS))
            {
                pPCB = (PCB *)cacheStatsFePcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_FE_STATS;
                }
            }
#endif  /* NO_HAB_CACHE */

#ifndef NO_VCG_CACHE
            if (Invalidate(cacheMask, CACHE_INVALIDATE_VCG_INFO))
            {
                pPCB = (PCB *)cacheVcgInfoPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_VCG_INFO;
                }
            }
#endif  /* NO_VCG_CACHE */

            if (Invalidate(cacheMask, CACHE_INVALIDATE_WORKSET))
            {
                pPCB = (PCB *)cacheWorksetPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_WORKSET;
                }
            }

            if (Invalidate(cacheMask, CACHE_INVALIDATE_FREE_SPACE))
            {
                pPCB = (PCB *)cacheFreeSpacePcb;

                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_FREE_SPACE;
                }
            }

#ifndef NO_VCD_CACHE
            if (Invalidate(cacheMask, CACHE_INVALIDATE_VDISK_CACHE_INFO))
            {
                pPCB = (PCB *)cacheVDiskCacheInfoPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_VDISK_CACHE_INFO;
                }
            }
#endif  /* NO_VCD_CACHE */

#ifndef NO_HAB_CACHE
            /*
             * This is a special case because if we are also
             * running a CACHE_INVALIDATE_BE_STATS, it must finish
             * first before we start.
             *
             * This is a special case because CACHE_INVALIDATE_BE_PORT
             * requires CACHE_INVALIDATE_DISKBAY and CACHE_INVALIDATE_BE_STATS
             * (in that order).
             */
            if (Invalidate(cacheMask, CACHE_INVALIDATE_BE_PORT))
            {
                /* Check if CACHE_INVALIDATE_DISKBAY is running. */
                if (Invalidate(cacheMask, CACHE_INVALIDATE_DISKBAY))
                {
                    /* Kick off the other tasks. */
                    TaskSwitch();

                    /* Wait until CACHE_INVALIDATE_DISKBAY finishes. */
                    while (!(invalid.pSatisfied->finishedMask & CACHE_INVALIDATE_DISKBAY))
                    {
                        TaskSleepMS(20);
                    }
                }

                /* Next check if CACHE_INVALIDATE_BE_STATS is running. */
                if (Invalidate(cacheMask, CACHE_INVALIDATE_BE_STATS))
                {
                    /*
                     * Wait until CACHE_INVALIDATE_BE_STATS finishes.
                     * No TaskSwitch() required - task would have been
                     * started by TaskSwitch() above.
                     */
                    while (!(invalid.pSatisfied->finishedMask &
                             CACHE_INVALIDATE_BE_STATS))
                    {
                        TaskSleepMS(20);
                    }
                }

                /* Now we can update the BE_PORT cache */
                pPCB = (PCB *)cacheBEPortPcb;
                if ((pPCB != NULL) && (TaskGetState(pPCB) == PCB_NOT_READY))
                {
                    TaskSetState(pPCB, PCB_READY);
                }
                else
                {
                    invalid.pSatisfied->finishedMask |= CACHE_INVALIDATE_BE_PORT;
                }
            }
#endif  /* NO_HAB_CACHE */

            /*
             * Get our tasks started.
             */
            TaskSwitch();

            dprintf(DPRINTF_CACHE_INVALIDATE, "ProcessInvalidCacheQueueTask: 0x%08X cacheMask 0x%08X finishedMask 0x%08X\n",
                    sequence, cacheMask, invalid.pSatisfied->finishedMask);

            /*
             * Wait for all the tasks to finish.
             * TaskSleepMS(20) could be replace with a
             * TaskSwitch() if it is waiting too long.
             */
            while ((invalid.pSatisfied->finishedMask & cacheMask) != cacheMask)
            {
                TaskSleepMS(20);
            }

            dprintf(DPRINTF_CACHE_INVALIDATE, "ProcessInvalidCacheQueueTask: 0x%08X cacheMask 0x%08X finishedMask 0x%08X\n",
                    sequence, cacheMask, invalid.pSatisfied->finishedMask);

            dprintf(DPRINTF_CACHE_INVALIDATE, "ProcessInvalidCacheQueueTask: sequence-0x%08X CacheStateSetFinished\n",
                    invalid.pSatisfied->satisfied);
            /*
             * Set the Finished bit of pSatisfied.
             */
            CacheStateSetFinished(invalid.pSatisfied->satisfied);

            dprintf(DPRINTF_CACHE_INVALIDATE, "ProcessInvalidCacheQueueTask: sequence-0x%08X CacheStateWaitFinishedDone\n",
                    invalid.pSatisfied->satisfied);
            /*
             * Wait for all processes to exit.
             */
            CacheStateWaitFinishedDone(invalid.pSatisfied->satisfied);

            dprintf(DPRINTF_CACHE_INVALIDATE, "ProcessInvalidCacheQueueTask: sequence-0x%08X free\n",
                    invalid.pSatisfied->satisfied);

            /*
             * Set our pPCB to NULL.
             */
            pPCB = NULL;

            /*
             * Set the global return pointer to NULL.
             */
            pGlobalInvalidReturn = NULL;

            /*
             * Free the memory we were using.
             */
            Free(invalid.pSatisfied);
        }

        dprintf(DPRINTF_CACHE_INVALIDATE, "ProcessInvalidCacheQueueTask: LOOP EXIT waiting until woken up\n");

        /*
         * We are done processing the queue for now,
         * Sleep until we are woken up.
         */
        TaskSetState(pInvalidCachePcb, PCB_NOT_READY);
        TaskSwitch();

        dprintf(DPRINTF_CACHE_INVALIDATE, "ProcessInvalidCacheQueueTask: LOOP EXIT woken up\n");
    }
}

/*----------------------------------------------------------------------------
** Function:    EnqueueInvalidCache
**
** Description: Enqueues Invalidate cache requests.
**
** Inputs:      cacheMask   -  Actions to be taken
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static CACHE_INVALIDATE_QUEUE_RETURN *EnqueueInvalidCache(UINT32 cacheMask)
{
    dprintf(DPRINTF_CACHE_INVALIDATE, "EnqueueInvalidCache: cacheMask-0x%08X\n", cacheMask);
    /*
     * If the queue is empty, clear it out.
     */
    if (cacheInvalidQueueEmpty)
    {
        cacheInvalidQueue.pSatisfied = MallocWC(sizeof(*cacheInvalidQueue.pSatisfied));
        cacheInvalidQueueEmpty = false;
    }

    /*
     * Place the data on the queue
     */
    cacheInvalidQueue.cacheMask |= (CACHE_INVALIDATE_ALL & cacheMask);

    /*
     * if the Inalid Queue task exists and is not ready, wake it up.
     */
    if ((pInvalidCachePcb != NULL) && (TaskGetState(pInvalidCachePcb) == PCB_NOT_READY))
    {
        TaskSetState(pInvalidCachePcb, PCB_READY);
    }

    dprintf(DPRINTF_CACHE_INVALIDATE, "EnqueueInvalidCache: satisfied-0x%08X\n",
            cacheInvalidQueue.pSatisfied->satisfied);

    return cacheInvalidQueue.pSatisfied;
}

/*----------------------------------------------------------------------------
** Function:    DequeueInvalidCache
**
** Description: Dequeues Invalid cache.
**
** Inputs:      invalidQueue   -   output CACHE_INVALIDATE_QUEUE copied here.
**
** Returns:     true if successful, false if queue is empty.
**
**--------------------------------------------------------------------------*/
static bool DequeueInvalidCache(CACHE_INVALIDATE_QUEUE *invalidQueue)
{
    bool        dequeued = false;

    /*
     * If there's data on the queue then process it.
     */
    if (!cacheInvalidQueueEmpty)
    {
        dprintf(DPRINTF_CACHE_INVALIDATE, "DequeueInvalidCache: cacheMask-0x%08X,  satisfied-0x%08X\n",
                cacheInvalidQueue.cacheMask, cacheInvalidQueue.pSatisfied->satisfied);

        /*
         * Copy the data from the queue to the CACHE_INVALIDATE_QUEUE passed in.
         */
        invalidQueue->cacheMask = cacheInvalidQueue.cacheMask;
        invalidQueue->pSatisfied = cacheInvalidQueue.pSatisfied;

        cacheInvalidQueue.cacheMask = CACHE_INVALIDATE_NONE;
        cacheInvalidQueue.pSatisfied = NULL;

        dequeued = true;
        cacheInvalidQueueEmpty = true;
    }

    return dequeued;
}

/*----------------------------------------------------------------------------
** Function:    InvalidateCacheTask
**
** Description: Invalidate cache.
**
** Inputs:      dummy           - Dummy parameter required for forked tasks..
**              pState          - Pointer to the cache state.
**              pPCB            - Pointer to our PCB.
**              mask            - value to mask to pReturnMask on success.
**              RefreshFunction - Pointer to Refresh Function to call.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
static void InvalidateCacheTask(TASK_PARMS *parms)
{
    UINT16     *pState = (UINT16 *)parms->p2;
    UINT32      mask = parms->p3;
    PCB       **pPCB = (PCB **) parms->p4;
    RefreshCache_func RefreshFunction = (RefreshCache_func)parms->p5;
    INT32       rc = PI_GOOD;
    char        debugStr[40];

    /* Double check to make sure our parameters are good. */
    if (pState == NULL || pPCB == NULL || RefreshFunction == NULL)
    {
        dprintf(DPRINTF_DEFAULT, "InvalidateCacheTask: Invalid parameters\n");
        return;
    }

    dprintf(DPRINTF_CACHE_INVALIDATE, "InvalidateCacheTask: state-0x%04hX  pPCB-0x%08X  mask-0x%08X\n",
            *pState, (UINT32)*pPCB, mask);

    while (1)
    {
        dprintf(DPRINTF_CACHE_INVALIDATE, "InvalidateCacheTask: PCB-0x%08X  Going to sleep\n", (UINT32)*pPCB);

        /* Sleep until we are woken up. */
        TaskSetState(*pPCB, PCB_NOT_READY);
        TaskSwitch();

        dprintf(DPRINTF_CACHE_INVALIDATE, "InvalidateCacheTask: PCB-0x%08X  Woke up\n", (UINT32)*pPCB);

        /* Set the cache state invalid. */
        CacheStateSetInvalid(*pState);

        /* While we still need to validate. */
        if ((rc = RefreshFunction()) == PI_GOOD)
        {
            if (pGlobalInvalidReturn != NULL)
            {
                pGlobalInvalidReturn->returnMask |= mask;
            }
        }

        if (MD_DPRINTF_CACHE_REFRESH & modeData.ccb.bitsDPrintf)
        {
            if (*pPCB == cacheDiskBaysPcb)
            {
                strcpy(debugStr, "Disk Bays");
            }
#ifndef NO_PDISK_CACHE
            else if (*pPCB == cachePhysicalDisksPcb)
            {
                strcpy(debugStr, "Pdisks");
            }
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
            else if (*pPCB == cacheVirtualDisksPcb)
            {
                strcpy(debugStr, "Vdisks");
            }
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
            else if (*pPCB == cacheRaidsPcb)
            {
                strcpy(debugStr, "Raids");
            }
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
            else if (*pPCB == cacheServersPcb)
            {
                strcpy(debugStr, "Servers");
            }
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
            else if (*pPCB == cacheTargetsPcb)
            {
                strcpy(debugStr, "Targets");
            }
#endif  /* NO_TARGET_CACHE */
            else if (*pPCB == cacheEnvPcb)
            {
                strcpy(debugStr, "Environmentals");
            }
            else if (*pPCB == cacheStatsFePcb)
            {
                strcpy(debugStr, "FE Loop Statistics");
            }
            else if (*pPCB == cacheStatsBePcb)
            {
                strcpy(debugStr, "BE Loop Statistics");
            }
            else if (*pPCB == cacheBEPortPcb)
            {
                strcpy(debugStr, "BE Port");
            }
#ifndef NO_VCG_CACHE
            else if (*pPCB == cacheVcgInfoPcb)
            {
                strcpy(debugStr, "VCG Info");
            }
#endif  /* NO_VCG_CACHE */
            else if (*pPCB == cacheWorksetPcb)
            {
                strcpy(debugStr, "Workset Info");
            }
#ifndef NO_VCD_CACHE
            else if (*pPCB == cacheVDiskCacheInfoPcb)
            {
                strcpy(debugStr, "VDisk Cache Info");
            }
#endif  /* NO_VCD_CACHE */
            else
            {
                strcpy(debugStr, "UKNOWN");
            }

            dprintf(DPRINTF_CACHE_REFRESH, "CACHE-%s refresh %s\n", debugStr,
                    ((rc == PI_GOOD) ? "OK" : "FAILED"));
        }

        /* Set the cache state valid. */
        CacheStateSetValid(*pState);

        if (pGlobalInvalidReturn != NULL)
        {
            pGlobalInvalidReturn->finishedMask |= mask;
        }
    }
}

/*----------------------------------------------------------------------------
** Function:    InvalidateCache
**
** Description: Invalidates the disk bays and refreshes the data.
**
** Inputs:      cacheMask           -   What to invalidate.
**              waitForCompletion   -   wait for the task to finish.
**
** Returns:     Mask of what was completed.
**--------------------------------------------------------------------------*/
UINT32 InvalidateCache(UINT32 cacheMask, bool waitForCompletion)
{
    CACHE_INVALIDATE_QUEUE_RETURN *pSatisfied = NULL;
    UINT32      retCacheMask = cacheMask;

    /* If we have nothing to invalidate, return. */
    if (cacheMask != CACHE_INVALIDATE_NONE)
    {
        pSatisfied = EnqueueInvalidCache(cacheMask);

        if (pSatisfied != NULL)
        {
            dprintf(DPRINTF_CACHE_INVALIDATE, "InvalidateCache: 0x%08X, cacheMask-0x%08X, waitForCompletion-%s\n",
                    pSatisfied->satisfied, cacheMask, (waitForCompletion ? "TRUE" : "FALSE"));

            if (waitForCompletion)
            {
                /* Increment reference count. */
                CacheStateFinishedAdd(pSatisfied->satisfied);

                dprintf(DPRINTF_CACHE_INVALIDATE, "InvalidateCache: %lu Waiting for Finish\n",
                        (long)pSatisfied->satisfied);
                CacheStateWaitFinished(pSatisfied->satisfied);

                /* Get the mask of what completed successfully. */
                retCacheMask = pSatisfied->returnMask;

                /* Decrement reference count. */
                CacheStateFinishedRemove(pSatisfied->satisfied);
            }
            dprintf(DPRINTF_CACHE_INVALIDATE, "InvalidateCache: 0x%08X Exit\n", pSatisfied->satisfied);
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "InvalidateCache: ERROR, INVALID POINTER\n");
        }
    }

    /* We return the retCacheMask masked with what we were tasked to do. */
    return (cacheMask & retCacheMask);
}

/*----------------------------------------------------------------------------
** Function:    CacheCheckFEReady
**
** Description: Tests for FE Ready.
**
** Inputs:      NONE.
**
** Returns:     true if FE is ready, false if the FE is not ready.
**--------------------------------------------------------------------------*/
bool CacheCheckFEReady(void)
{
    /*
     * If the FE is ready, no reason to repeat the MRP.
     */
    if (!cacheFeReady)
    {
        /*
         * If the SERVER bit is set in the status
         * then we can successfully retrieve server info.
         */
        if (GetProcAddress_FEII() != 0 && GetProcAddress_FEII()->status & II_STATUS_SERVER)
        {
            cacheFeReady = true;
            dprintf(DPRINTF_DEFAULT, "CacheFEReady: Front end is ready.\n");
        }
    }

    return cacheFeReady;
}

/*----------------------------------------------------------------------------
** Function:    CacheGetList
**
** Description: Wrapper for PI_ExecMRP with retries.
**
**                - MR_LIST_RSP
**                   - When caller is done with MR_LIST_RSP
**                     free using "free"
**
** Inputs:      listType - type of list to retrieve
**              retries (500ms between) (0 == try forever)
**
** Returns:     MR_LIST_RSP* on SUCCESS.
**              NULL on ERROR.
**
**--------------------------------------------------------------------------*/
MR_LIST_RSP *CacheGetList(UINT16 listType)
{
    MR_DEVID_REQ *ptrListIn = NULL;
    MR_LIST_RSP *ptrListOut = NULL;
    UINT32      listOutSize;
    UINT16      numDevs;
    UINT16      rc = PI_GOOD;
    UINT16      startID = 0;

    listType |= GET_LIST;

    numDevs = numDevs_From_listType(listType);

    /*
     * Allocate memory for the MRP input and output packets.
     * Fill in the input parm.
     */
    ptrListIn = MallocWC(sizeof(*ptrListIn));
    ptrListIn->id = startID;

    do
    {
        listOutSize = (sizeof(*ptrListOut) + ((numDevs - 1) * sizeof(UINT16)));

        /*
         * If an output list was previously allocated, free it before
         * allocating a new one.
         */
        if (ptrListOut != NULL)
        {
            Free(ptrListOut);
        }

        ptrListOut = MallocSharedWC(listOutSize);

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
        rc = PI_ExecMRP(ptrListIn, sizeof(*ptrListIn), (listType & MRP_MASK),
                        ptrListOut, listOutSize, CACHE_MAX_REFRESH_MRP_TIMEOUT);

        /*
         * Save the number of devices in case we need to make the
         * request again.
         */
        numDevs = ptrListOut->ndevs;

    } while ((listType & GET_LIST) && (rc == PI_ERROR) &&
             (ptrListOut->header.status == DETOOMUCHDATA));

    /* Free the input list. */
    Free(ptrListIn);

    /*
     * If there was an error and the list is not NULL,
     * free the list and set the pointer to NULL.
     */
    if ((rc == PI_TIMEOUT) && (ptrListOut != NULL))
    {
        ptrListOut = NULL;
    }
    else if ((rc != PI_GOOD) && (ptrListOut != NULL))
    {
        Free(ptrListOut);
        ptrListOut = NULL;
    }

    return ptrListOut;
}

/*----------------------------------------------------------------------------
** Function:    CacheSetRefreshFlag
**
** Description: Set a flag telling the cache to refresh.
**
** Inputs:      refreshFlags    -   flags to refresh.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void CacheSetRefreshFlag(UINT32 refreshFlags)
{
    CacheRefreshFlagSet(refreshFlags);
}

/*----------------------------------------------------------------------------
** Function:    CacheRefreshTask - FORKED
**
** Description: Process to update the cached disk bay information on
**              a periodic basis.
**
** Inputs:      UINT32 dummy - Dummy parameter required for forked tasks.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
static NORETURN void CacheRefreshTask(UNUSED TASK_PARMS *parms)
{
    /*
     * Loop here and check the refresh flags.
     */
    while (1)
    {
        if (CacheRefreshFlagGet(CACHE_INVALIDATE_ALL))
        {
            (void)InvalidateCache(CacheRefreshFlagGet(CACHE_INVALIDATE_ALL), true);

            CacheRefreshFlagDoneSet(CACHE_INVALIDATE_ALL);
        }

        TaskSleepMS(CACHE_REFRESH_INTERVAL);
    }
}


/*----------------------------------------------------------------------------
** Function:    PI_IsCacheDirty
**
** Description: Checks the perticular component's cache is dirty
**              or not.
**
** Inputs:      UINT32 mask to check the dirty or not.
**
**
** Returns:     0 if not dirty, 1 if dirty.
**--------------------------------------------------------------------------*/
UINT32 PI_IsCacheDirty(UINT32 mask)
{
    return (piCacheMask & mask);
}

/*----------------------------------------------------------------------------
** Function:    PI_MakeCacheDirty
**
** Description: make the appropriate bits set to say cache dirty
**
** Inputs:      UINT32 mask is bits to set
**
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void PI_MakeCacheDirty(UINT32 mask)
{
    piCacheMask |= mask;
}

/*----------------------------------------------------------------------------
** Function:    PI_MakeCacheValid
**
** Description: Make the appropriate bits cleared
**
** Inputs:      UINT32 mask to clear the bits.
**
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void PI_MakeCacheValid(UINT32 mask)
{
    piCacheMask &= ~mask;
}


NORETURN void VdisksCacheRefreshInjectTask(UNUSED TASK_PARMS *parms)
{
    UINT32      cacheMask;

    cacheMask =
#ifndef NO_VDISK_CACHE
                CACHE_INVALIDATE_VDISK |
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
                CACHE_INVALIDATE_RAID |
#endif  /* NO_RAID_CACHE */
#ifndef NO_PDISK_CACHE
                CACHE_INVALIDATE_PDISK |
#endif  /* NO_PDISK_CACHE */
                0;

    while (1)
    {
        TaskSleepMS(REFRESH_VDISKS_INJECT_INTERVAL);

        (void)EnqueueInvalidCache(cacheMask);
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
