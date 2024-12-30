/* $Id: ddr.c 161368 2013-07-29 14:53:10Z marshall_midden $*/
/**
******************************************************************************
**
**  @file   ddr.c
**
**  @brief  FE/BE/CCB DDR Table definitions
**
**  Definition of FE/BE/CCB DDR Tables. Debug Data Retrieval
**
** Copyright (c) 2003-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "ddr.h"

#include <stdio.h>
#include <netinet/in.h>
#include <errno.h>

#include "CacheBay.h"
#include "CacheLoop.h"
#include "CacheManager.h"
#include "CacheMisc.h"
#include "CachePDisk.h"
#include "CacheRaid.h"
#include "CacheServer.h"
#include "CacheTarget.h"
#include "CacheVDisk.h"
#include "ccb_flash.h"
#include "ccb_statistics.h"
#include "debug_files.h"
#include "LargeArrays.h"
#include "nvram_structure.h"
#include "PktCmdHdl.h"
#include "PI_Misc.h"
#include "PI_PDisk.h"
#include "PI_Utils.h"
#include "rtc.h"
#include "SerBuff.h"
#include "sm.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "EL.h"
#include "LargeArrays.h"
#include "../../Proc/inc/apool.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

/*
** NOTE: All FIDs that are to be returned in a FID Snapshot dump MUST
**       be in the array below!
**
** The array below contains the list of FIDs that will be returned when
** FID 302 - CCB_FID_LIST is read.  The first item returned will be the
** number of FIDs in the list.  No data structure has been defined because
** none is needed, however it would look like the one below -
**
**  typedef struct FID_LIST
**  {
**      UINT16  count;          # of FIDs in fidList[]
**      UINT16  fidList[0];     # Array of FID numbers.
**  } FID_LIST;
**
*/
static UINT16 fidList[] = {
    /* CCB FIDs */
    2, 6, 7, 23, 256, 257, 258, 260, 261, 262,
    263, 264, 265, 266, 267, 268, 269, 270, 271, 272,
    273, 274, 275, 276, 277, 278, 279, 280, 281, 282,
    284, 285, 286, 287, 288, 289, 290, 291, 292, 293,
    294, 295, 296, 298, 299, 300, 301, 302, 303, 304,
    306, 307, 308, 309, 311, 313, 353, 354, 355,

    /* FE FIDs */
    512, 514, 520, 521, 522, 523, 524, 525, 526, 527,
    528, 533, 537, 538, 539, 540, 541, 542, 543, 544,
    545, 550, 556, 562, 563,

    /* BE FIDs */
    768, 770, 776, 785, 786, 787, 788, 789, 790, 791,
    792, 793, 794, 795, 796, 797, 798, 799, 800, 801,
    802, 803, 804, 805, 807, 808, 809, 810, 811, 812,
    813, 814, 815, 816, 817, 818, 819
};

/*****************************************************************************
** Private variables
*****************************************************************************/
#define MAX_LINUX_FNAME_SIZE    128
#define LINUX_ZIPPED_FILENAME   "/var/log/dump/xioZippedFile.archive"
#define LINUX_PAM_LOGS          "XIO_PAM_LOGS"
#define LINUX_SYSTEM_LOGS       "XIO_LINUX_LOGS"
#define LINUX_RAID_LOGS         "XIO_RAID_LOGS"
#define LINUX_CORE_SUMMARY      "XIO_CORE_SUMMARY"
#define LINUX_CORES             "XIO_CORES"
#define LINUX_QLOGIC_CORES      "XIO_QL_CORES"
#define LINUX_APP_LOGS          "XIO_APP_LOGS"

static char linuxReadFileName[MAX_LINUX_FNAME_SIZE] = { 0 };
static FILE *linFile = NULL;

struct DMC *DMC_CCB = (struct DMC *)CCB_DMC_BASE_ADDR;

/* Direct Memory Copy entry data buffers. */
static struct DMC_nvramp2percentused g_nvramp2percentused LOCATE_IN_SHMEM;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
CCB_DDR_TABLE ccbDdrTable;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void FillDdrEntry(UINT32 idx, const char *id, void *addr, UINT32 len,
                         DDR_FUNC_PTR pFunc);
static UINT32 DDRGetTraceDump(void *addr, UINT32 len);
static UINT32 DDRGetProfileDump(void *addr, UINT32 len);
static UINT32 DDRGetPCBDump(void *addr, UINT32 len);
static UINT32 DDREmpty(void *addr, UINT32 len);
static UINT32 DDRStatsProc(void *addr, UINT32 len);
static UINT32 DDRStatsPCI(void *addr, UINT32 len);
static UINT32 DDRStatsEnviron(void *addr, UINT32 len);
static UINT32 DDRStatsServer(void *addr, UINT32 len);
static UINT32 DDRStatsVDisk(void *addr, UINT32 len);
static UINT32 DDRStatsCacheDevices(void *addr, UINT32 len);
static UINT32 DDRStatsLoop(void *addr, UINT32 len);
static UINT32 DDRStatsFCALCounters(void *addr, UINT32 len);
static UINT32 DDRCopyFromActiveCache(void *addr, UINT32 len);
#if defined(NO_VDISK_CACHE) || defined(NO_RAID_CACHE)
static UINT32 DDRCopyFromMemory(void *addr, UINT32 len);
#endif  /* defined(NO_VDISK_CACHE) || defined(NO_RAID_CACHE) */
static UINT32 DDRProcDDRTables(void *addr, UINT32 len);
static UINT32 DDRFWVersions(void *addr, UINT32 len);
static UINT32 DDRTimeStamp(void *addr, UINT32 len);
static UINT32 DDRFCMCounters(void *addr, UINT32 len);
static UINT32 DDRVCGInfo(void *pAddr, UINT32 len);
static UINT32 DDRTargetResourceList(void *pAddr, UINT32 len);
static UINT32 DDRFIDList(void *pAddr, UINT32 len);
static UINT32 DDRMirrorPartnerList(void *pAddr, UINT32 len);
static UINT32 DDRBaySESData(void *pAddr, UINT32 len);

#if ISCSI_CODE
static UINT32 DDRiSCSIStats(void *pAddr, UINT32 len);
#endif  /* ISCSI_CODE */
static UINT32 DDRAsyncRep(void *pAddr, UINT32 len);

static UINT32 DDRLinuxFileOpen(void *pAddr, UINT32 len);
static UINT32 DDRLinuxFileReadPamLog(void *pAddr, UINT32 len);
static UINT32 DDRLinuxFileReadLinuxLog(void *addr, UINT32 len);
static UINT32 DDRLinuxFileReadRaidLog(void *addr, UINT32 len);
static UINT32 DDRLinuxFileCoreSummary(void *addr, UINT32 len);
static UINT32 DDRLinuxFileQLCores(void *addr, UINT32 len);
static UINT32 DDRLinuxFileCores(void *addr, UINT32 len);
static UINT32 DDRLinuxFileReadAppLog(void *addr, UINT32 len);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*****************************************************************************
** Function Name:   FillDdrEntry()
**
** Returns:         void
******************************************************************************/
static void FillDdrEntry(UINT32 idx, const char *id, void *addr, UINT32 len,
                         DDR_FUNC_PTR pFunc)
{
    strncpy((char *)ccbDdrTable.entry[idx].id, id, sizeof(ccbDdrTable.entry[0].id));
    ccbDdrTable.entry[idx].addr = addr;
    ccbDdrTable.entry[idx].len = len;
    ccbDdrTable.entry[idx].pFunc = pFunc;
}

/*****************************************************************************
** Function Name:   InitDdrTable()
**
** Returns:         void
******************************************************************************/
void InitDdrTable(void)
{
    UINT32      i;

    ccbDdrTable.version = CCB_DDR_VERSION;
    ccbDdrTable.numEntries = CCB_DDR_NUM_ENTRIES;
    ccbDdrTable.crc = 0;

    /*************************************************************************/

    FillDdrEntry(CCB_SERIALBUF, "SerialBf", NULL, 0, DDREmpty);
    FillDdrEntry(CCB_HEAPSTATS, "HeapStat", NULL, 0, DDREmpty);
    FillDdrEntry(CCB_TRACEBUF, "TraceBuf", gBigBuffer, 0, DDRGetTraceDump);
    FillDdrEntry(CCB_PROFDUMP, "ProfDump", gBigBuffer, 0, DDRGetProfileDump);
    FillDdrEntry(CCB_PCBDUMP, "PCBDump ", gBigBuffer, 0, DDRGetPCBDump);

    /*************************************************************************/

    FillDdrEntry(CCB_CUSTLOGS,            "CustLogs",
            (void *)FLASH_LOG_START_ADDRESS,
            (FLASH_LOG_NUM_SECTORS * CCB_FLASH_LARGE_SECTOR_SIZE), NULL);
    FillDdrEntry(CCB_DEBUGLOGS,           "DebgLogs",
            (void *)FLASH_DEBUG_LOG_START_ADDRESS,
            (FLASH_DEBUG_LOG_NUM_SECTORS * CCB_FLASH_LARGE_SECTOR_SIZE), NULL);

    /*************************************************************************/

    FillDdrEntry(CCB_CACHEDISKBAYMAP, "ChDBayMp", cacheDiskBayMap,
                 CACHE_SIZE_DISK_BAY_MAP, NULL);
    FillDdrEntry(CCB_CACHEDISKBAYPATHS, "ChDBayPa", cacheDiskBayPaths,
                 CACHE_SIZE_DISK_BAY_PATHS, NULL);
    FillDdrEntry(CCB_CACHEPDISKMAP, "ChPDMap ", cachePDiskMap,
                 CACHE_SIZE_PDISK_MAP, NULL);
    FillDdrEntry(CCB_CACHEPDISKFAILMAP, "ChPDFMap", cachePDiskFailMap,
                 CACHE_SIZE_PDISK_MAP, NULL);
    FillDdrEntry(CCB_CACHEPDISKREBUILDMAP, "ChPDRMap", cachePDiskRebuildMap,
                 CACHE_SIZE_PDISK_MAP, NULL);
    FillDdrEntry(CCB_CACHEPDISKPATHS, "ChPDPath", cachePDiskPaths,
                 CACHE_SIZE_PDISK_PATHS, NULL);
#ifndef NO_VDISK_CACHE
    FillDdrEntry(CCB_CACHEVIRTUALDISKMAP, "ChVDMap ", cacheVirtualDiskMap,
                 CACHE_SIZE_VDISK_MAP, NULL);
    FillDdrEntry(CCB_CACHEVDISKCOPYMAP, "ChVDCMap", cacheVDiskCopyMap,
                 CACHE_SIZE_VDISK_MAP, NULL);
    FillDdrEntry(CCB_CACHEVDISKMIRRORMAP, "ChVDMMap", cacheVDiskMirrorMap,
                 CACHE_SIZE_VDISK_MAP, NULL);
#else   /* NO_VDISK_CACHE */
    FillDdrEntry(CCB_CACHEVIRTUALDISKMAP, "ChVDMap ", &cacheVDiskBuffer_DMC.cacheVDiskMap_DMC,
                 sizeof(cacheVDiskBuffer_DMC.cacheVDiskMap_DMC), NULL);
    FillDdrEntry(CCB_CACHEVDISKCOPYMAP, "ChVDCMap", &cacheVDiskBuffer_DMC.cacheVDiskCopyMap_DMC,
                 sizeof(cacheVDiskBuffer_DMC.cacheVDiskCopyMap_DMC), NULL);
    FillDdrEntry(CCB_CACHEVDISKMIRRORMAP, "ChVDMMap", &cacheVDiskBuffer_DMC.cacheVDiskMirrorMap_DMC,
                 sizeof(cacheVDiskBuffer_DMC.cacheVDiskMirrorMap_DMC), NULL);
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
    FillDdrEntry(CCB_CACHERAIDMAP, "ChRDMap ", cacheRaidMap, CACHE_SIZE_RAID_MAP, NULL);
#else   /* NO_RAID_CACHE */
    FillDdrEntry(CCB_CACHERAIDMAP, "ChRDMap ", cacheRaidBuffer_DMC.cacheRaidMap_DMC,
                 sizeof(cacheRaidBuffer_DMC.cacheRaidMap_DMC), NULL);
#endif  /* NO_RAID_CACHE */
    FillDdrEntry(CCB_CACHESERVERMAP, "ChSRMap ", cacheServerMap,
                 CACHE_SIZE_SERVER_MAP, NULL);
    FillDdrEntry(CCB_CACHETARGETMAP, "ChTGTMap", cacheTargetMap,
                 CACHE_SIZE_TARGET_MAP, NULL);

    /*************************************************************************/

    FillDdrEntry(CCB_CACHEDISKBAYS, "ChDBays ", &cacheDiskBays,
                 CACHE_SIZE_DISK_BAYS, DDRCopyFromActiveCache);
#ifndef NO_TARGET_CACHE
    FillDdrEntry(CCB_CACHETARGETS, "ChTGTs  ", &cacheTargets,
                 CACHE_SIZE_TARGETS, DDRCopyFromActiveCache);
#else   /* NO_TARGET_CACHE */
NOTDONEYET
    FillDdrEntry(CCB_CACHETARGETS, "ChTGTs  ", &cacheTargets,
                 CACHE_SIZE_TARGETS, DDRCopyFromMemory);
#endif  /* NO_TARGET_CACHE */
    FillDdrEntry(CCB_CACHEFELOOPSTATS, "ChFELp  ", &cacheFELoopStats,
                 CACHE_SIZE_FE_LOOP_STATS, DDRCopyFromActiveCache);
    FillDdrEntry(CCB_CACHEBELOOPSTATS, "ChBELp  ", &cacheBELoopStats,
                 CACHE_SIZE_BE_LOOP_STATS, DDRCopyFromActiveCache);
#ifndef NO_PDISK_CACHE
    FillDdrEntry(CCB_CACHEPHYSICALDISKS, "ChPD    ", &cachePhysicalDisks,
                 CACHE_SIZE_PHYSICAL_DISKS, DDRCopyFromActiveCache);
#else   /* NO_PDISK_CACHE */
    FillDdrEntry(CCB_CACHEPHYSICALDISKS, "ChPD    ", cachePDiskBuffer_DMC.cachePDiskBuffer_DMC,
                 sizeof(cachePDiskBuffer_DMC.cachePDiskBuffer_DMC), DDRCopyFromMemory);
#endif  /* NO_RAID_CACHE */
#ifndef NO_VDISK_CACHE
    FillDdrEntry(CCB_CACHEVIRTUALDISKS, "ChVD    ", &cacheVirtualDisks,
                 CACHE_SIZE_VIRTUAL_DISKS, DDRCopyFromActiveCache);
#else   /* NO_VDISK_CACHE */
    FillDdrEntry(CCB_CACHEVIRTUALDISKS, "ChVD    ", cacheVDiskBuffer_DMC.cacheVDiskBuffer_DMC,
                 sizeof(cacheVDiskBuffer_DMC.cacheVDiskBuffer_DMC), DDRCopyFromMemory);
#endif  /* NO_RAID_CACHE */
#ifndef NO_RAID_CACHE
    FillDdrEntry(CCB_CACHERAIDS, "ChRaids ", &cacheRaids,
                 CACHE_SIZE_RAIDS, DDRCopyFromActiveCache);
#else   /* NO_RAID_CACHE */
    FillDdrEntry(CCB_CACHERAIDS, "ChRaids ", cacheRaidBuffer_DMC.cacheRaidBuffer_DMC,
                 sizeof(cacheRaidBuffer_DMC.cacheRaidBuffer_DMC), DDRCopyFromMemory);
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
    FillDdrEntry(CCB_CACHESERVERS, "ChSrv   ", &cacheServers,
                 CACHE_SIZE_SERVERS, DDRCopyFromActiveCache);
#else   /* NO_SERVER_CACHE */
    FillDdrEntry(CCB_CACHESERVERS, "ChSrv   ", cacheServerBuffer_DMC.cacheServerBuffer_DMC,
                 sizeof(cacheServerBuffer_DMC.cacheServerBuffer_DMC), DDRCopyFromMemory);
#endif  /* NO_SERVER_CACHE */

    /*************************************************************************/

    FillDdrEntry(CCB_STATSPROC, "StProc  ", gBigBuffer, 0, DDRStatsProc);
    FillDdrEntry(CCB_STATSPCI, "StPCI   ", gBigBuffer, 0, DDRStatsPCI);
    FillDdrEntry(CCB_STATSENVIRON, "StEnvirn", gBigBuffer, 0, DDRStatsEnviron);
    FillDdrEntry(CCB_STATSSERVER, "StServer", gBigBuffer, 0, DDRStatsServer);
    FillDdrEntry(CCB_STATSVDISK, "StVDisk ", gBigBuffer, 0, DDRStatsVDisk);
    FillDdrEntry(CCB_STATSCACHEDEVICE, "StChDev ", gBigBuffer, 0, DDRStatsCacheDevices);
    FillDdrEntry(CCB_STATSLOOP, "StLoop  ", gBigBuffer, 0, DDRStatsLoop);
    FillDdrEntry(CCB_STATSFCALCOUNTERS, "StFCALCt", gBigBuffer, 0, DDRStatsFCALCounters);

    /*************************************************************************/

    FillDdrEntry(CCB_CMD_RECORD_TBL, "CmdRcdTb",
                 commandRecordTable,
                 (COMMAND_RECORD_TABLE_SIZE * sizeof(PI_COMMAND_RECORD)), NULL);

    FillDdrEntry(CCB_NVRAM, "CcbNvram", (void *)CCB_NVRAM_BASE, CCB_NVRAM_SIZE, NULL);

    FillDdrEntry(CCB_NVRAM_PERS_STORE, "CcbXSSA ",
                 (void *)XSSA_PERS_DATA_BASE, XSSA_PERS_DATA_SIZE, NULL);

    FillDdrEntry(CCB_PROC_DDR_TABLES, "ProcDdrT", gBigBuffer, 0, DDRProcDDRTables);

    FillDdrEntry(CCB_FIRMWARE_VERSIONS, "FW Vers ", gBigBuffer, 0, DDRFWVersions);

    FillDdrEntry(CCB_TIMESTAMP, "TimeStmp", gBigBuffer, 0, DDRTimeStamp);

    FillDdrEntry(CCB_NVRAM_COPIES, "NVR Cpys",
                 (void *)BASE_FLASH_BACKTRACE_ADDRESS, (2 * 256 * 1024), NULL);

    FillDdrEntry(CCB_FCM_COUNTERS, "FCMCount", gBigBuffer,
                 FCM_COUNTER_MAP_SIZE, DDRFCMCounters);

    FillDdrEntry(CCB_VCG_INFO, "VCGInfo ", gBigBuffer, 0, DDRVCGInfo);

    FillDdrEntry(CCB_TARGET_RESOURCE_LIST, "TgtResLt", gBigBuffer,
                 0, DDRTargetResourceList);

    FillDdrEntry(CCB_FID_LIST, "FID List", gBigBuffer, 0, DDRFIDList);

    FillDdrEntry(CCB_STATS_CCB, "Stat CCB", &CCBStats, sizeof(CCB_STATS_STRUCTURE), NULL);

    FillDdrEntry(CCB_MIRROR_PARTNER_LIST, "MP List ", gBigBuffer,
                 0, DDRMirrorPartnerList);

    FillDdrEntry(CCB_LINUX_FILE_READ, "Linux RD", gBigBuffer,
                 BIG_BUFFER_SIZE, DDRLinuxFileOpen);

    FillDdrEntry(CCB_LINUX_FILE_READ_PAMLOG, "LinRdPam", gBigBuffer,
                 BIG_BUFFER_SIZE, DDRLinuxFileReadPamLog);

    FillDdrEntry(CCB_LINUX_FILE_READ_LINLOG, "LinRdLin", gBigBuffer,
                 BIG_BUFFER_SIZE, DDRLinuxFileReadLinuxLog);

    FillDdrEntry(CCB_LINUX_FILE_READ_RAIDLOG, "LinRdRad", gBigBuffer,
                 BIG_BUFFER_SIZE, DDRLinuxFileReadRaidLog);

    FillDdrEntry(CCB_LINUX_FILE_READ_CORESUM, "LinRdCsm", gBigBuffer,
                 BIG_BUFFER_SIZE, DDRLinuxFileCoreSummary);

    FillDdrEntry(CCB_LINUX_FILE_READ_CORES, "LinRdCor", gBigBuffer,
                 BIG_BUFFER_SIZE, DDRLinuxFileCores);

    FillDdrEntry(CCB_LINUX_FILE_READ_QLCORES, "LinRdQLCor", gBigBuffer,
                 BIG_BUFFER_SIZE, DDRLinuxFileQLCores);

    FillDdrEntry(CCB_LINUX_FILE_READ_APPLOG, "LinRdApp", gBigBuffer,
                 BIG_BUFFER_SIZE, DDRLinuxFileReadAppLog);

    FillDdrEntry(CCB_LINUX_FILE_READ_SMP, "LinRsrvd", 0x0, 0x0, NULL);

    for (i = CCB_LINUX_FILE_READ_RESERVE_START; i <= CCB_LINUX_FILE_READ_END; i++)
    {
        FillDdrEntry(i, "LinRsrvd", 0x0, 0x0, NULL);
    }

    FillDdrEntry(CCB_BAY_SES_DATA, "BaySESDt", gBigBuffer,
                 BIG_BUFFER_SIZE, DDRBaySESData);
#if ISCSI_CODE
    FillDdrEntry(CCB_ISCSI_STATS, "iSCSIStats", gBigBuffer,
                 BIG_BUFFER_SIZE, DDRiSCSIStats);
#endif  /* ISCSI_CODE */
    FillDdrEntry(CCB_ASYNC_REP, "AsyncRep", gBigBuffer, BIG_BUFFER_SIZE, DDRAsyncRep);
}   /* End of InitDdrTable */

/*****************************************************************************
** Function:    FetchFIDVersion
**
** Description: Determine the FID version.
**
** Inputs:      fid - the fid in question.
**
** Returns:     fid version.
**
******************************************************************************/
UINT8 FetchFIDVersion(UINT32 fid)
{
    UINT8       ver;

    /*
     * Since most FIDs are the default (0), the quickest way to accomplish
     * this versioning is to simply create a switch / case statement for those
     * FIDs that deviate from the default.  If this gets unwieldy, we can
     * always create a table of fid versions.
     */
    switch (fid)
    {
            /*
             * Version 1 FIDs
             */
        case 2:                /* Back End NVRAM   */
        case 3:                /* Front End NVRAM  */
        case 256:              /* CCB Trace Buffer */
        case 286:
        case 299:              /* FCM Counters     */
        case 353:              /* Bay SES Data     */
        case 354:              /* iSCSI Stats      */
        case 355:              /* Async Replication */
        case 545:              /* FE NVRAM Part 5  */
        case 550:              /* FE IRAM          */
        case 551:              /* BE IRAM          */
        case 801:              /* BE NVRAM Part 5  */
        case 806:              /* FE IRAM          */
        case 807:              /* BE IRAM          */
            ver = 1;
            break;

            /*
             * Version 0 FIDs (everything else)
             */
        default:
            ver = 0;
            break;
    }

    return ver;
}

/*****************************************************************************
** Function Name:   DDRGetPCBDump()
**
** Returns:         number of bytes formatted
******************************************************************************/
static UINT32 DDRGetPCBDump(UNUSED void *addr, UNUSED UINT32 len)
{
    /*
     * Go get the PCB dump stuff
     */
    return GetPCBDump(PROCESS_CCB);
}

/*****************************************************************************
** Function Name:   DDRGetProfileDump()
**
** Returns:         number of bytes formatted
******************************************************************************/
static UINT32 DDRGetProfileDump(UNUSED void *addr, UNUSED UINT32 len)
{
    /*
     * Go get the profile dump stuff
     */
    return GetProfileDump(PROCESS_CCB);
}

/*****************************************************************************
** Function Name:   DDRGetTraceDump()
**
** Returns:         number of bytes formatted
******************************************************************************/
static UINT32 DDRGetTraceDump(UNUSED void *addr, UNUSED UINT32 len)
{
    /*
     * Go get the trace dump stuff
     */
    return GetTraceDump(PROCESS_CCB);
}

#if defined(NO_VDISK_CACHE) || defined(NO_RAID_CACHE)
/*****************************************************************************
** Function Name:   DDRCopyFromMemory()
**
** Returns:         number of bytes formatted
******************************************************************************/
static UINT32 DDRCopyFromMemory(void *addr, UINT32 len)
{
    if (addr == NULL)
    {
        return 0;           /* If can not copy due to address of zero. */
    }

    /* Check the length to make sure we do not overwrite the bigBuffer. */
    len = MIN(len, BIG_BUFFER_SIZE);

    /* Copy into gBigBuffer since that is where is always goes. */
    memcpy(gBigBuffer, addr, len);

    return len;
}   /* End of DDRCopyFromMemory */
#endif  /* defined(NO_VDISK_CACHE) || defined(NO_RAID_CACHE) */

/*****************************************************************************
** Function Name:   DDRCopyFromActiveCache()
**
** Returns:         number of bytes formatted
******************************************************************************/
static UINT32 DDRCopyFromActiveCache(void *addr, UINT32 len)
{
    UINT8 *from;

    if (addr == NULL)
    {
        return 0;           /* If can not copy due to address of zero. */
    }
    from = *(UINT8 **)addr;
    if (from == NULL)
    {
        return 0;           /* If can not copy due to address of zero. */
    }

#ifndef NO_PDISK_CACHE
    if (((UINT32)(addr)) == ((UINT32)(&cachePhysicalDisks)))
    {
        RefreshPhysicalDisks();
    }
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
    if (((UINT32)(addr)) == ((UINT32)(&cacheVirtualDisks)))
    {
        RefreshVirtualDisks();
    }
#endif  /* NO_VDISK_CACHE */

    /* Check the length to make sure we do not overwrite the bigBuffer. */
    len = MIN(len, BIG_BUFFER_SIZE);

    /* Copy into gBigBuffer since that is where is always goes. */
    memcpy(gBigBuffer, from, len);

    return len;
}   /* End of DDRCopyFromActiveCache */

/*****************************************************************************
** Function Name:   DDRVCGInfo()
**
** Description:     Gather VCG Info an assemble in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRVCGInfo(void *pAddr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    XIO_PACKET  reqPacket = { &reqHdr, NULL };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    INT32       rc;
    UINT8      *pStartBuffer = pAddr;
    UINT8      *pBuf = pAddr;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    /* Set up for the VCG Info call */
    reqPacket.pHeader->commandCode = PI_VCG_INFO_CMD;
    reqPacket.pHeader->length = 0;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRVCGInfo FAILED: rc=0x%X\n", rc);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRVCGInfo returning %d bytes.\n",
                rspPacket.pHeader->length);
        memcpy(pBuf, rspPacket.pPacket, rspPacket.pHeader->length);
        pBuf += rspPacket.pHeader->length;
    }

    /* Done */
    Free(rspPacket.pPacket);

    return (INT8 *)pBuf - (INT8 *)pStartBuffer;
}

/*****************************************************************************
** Function Name:   DDRTargetResourceList()
**
** Description:     Gather Target Resource List and assemble in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRTargetResourceList(void *pAddr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    PI_TARGET_RESOURCE_LIST_REQ reqData;
    XIO_PACKET  reqPacket = { &reqHdr, (UINT8 *)&reqData };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    INT32       rc;
    UINT8      *pStartBuffer = pAddr;
    UINT8      *pBuf = pAddr;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    /* Set up for the Target Resource List call */
    reqPacket.pHeader->commandCode = PI_TARGET_RESOURCE_LIST_CMD;
    reqPacket.pHeader->length = sizeof(reqData);
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqData.sid = 0;            /* Start at SID 0   */
    reqData.tid = 0xFFFF;       /* Get all targets  */
    reqData.listType = (SERVERS_ACTIVE + WWN_FORMAT);

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRTargetResourceList FAILED: rc=0x%X\n", rc);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRTargetResourceList returning %d bytes.\n",
                rspPacket.pHeader->length);
        memcpy(pBuf, rspPacket.pPacket, rspPacket.pHeader->length);
        pBuf += rspPacket.pHeader->length;
    }

    /* Done */
    Free(rspPacket.pPacket);

    return (INT8 *)pBuf - (INT8 *)pStartBuffer;
}

#if ISCSI_CODE

/*****************************************************************************
** Function Name:   DDRiSCSIStats()
**
** Description:     Gather iSCSI Stats and assemble in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRiSCSIStats(void *pAddr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    MRGETSESSIONS_REQ reqData;
    UINT8      *pCacheEntry = NULL;
    XIO_PACKET  reqPacket = { &reqHdr, (UINT8 *)&reqData };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    INT32       rc;
    UINT8      *pStartBuffer = pAddr;
    UINT8      *pBuf = pAddr;
    UINT16      numSessions = 0;
    UINT16      numTargets = 0;
    UINT32      plen;
    UINT32      remlen = 0;
    UINT32      flen;
    UINT8       i;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    DDRCopyFromActiveCache(cacheTargets, CACHE_SIZE_TARGETS);
    pCacheEntry = cacheTargets;
    for (i = 0; i < cacheTargetsCount; i++)
    {
        if (BIT_TEST(((MRGETTARG_RSP *)pCacheEntry)->opt, 7))
        {
            /*
             * Set up for the Target Resource List call
             */
            reqPacket.pHeader->commandCode = PI_ISCSI_SESSION_INFO;
            reqPacket.pHeader->length = sizeof(reqData);
            reqPacket.pHeader->packetVersion = 1;
            rspPacket.pHeader->packetVersion = 1;

            reqData.tid = ((MRGETTARG_RSP *)pCacheEntry)->tid;
            reqData.rsvd = 0;

            /* Issue the command through the packet command handler */
            rc = PacketCommandHandler(&reqPacket, &rspPacket);
            if (rc != PI_GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "DDRiSCSIStats FAILED: rc=0x%X\n", rc);
            }
            else
            {
#if 0
                dprintf(DPRINTF_DEFAULT, "DDRiSCSIStats receiving %d bytes.\n",
                        rspPacket.pHeader->length);
#endif  /* 0 */
                numSessions += ((MRGETSESSIONS_RSP *)(rspPacket.pPacket))->numSessions;
                if (((MRGETSESSIONS_RSP *)(rspPacket.pPacket))->numSessions > 0)
                {
                    if (numTargets == 0)
                    {
                        memcpy(pBuf, rspPacket.pPacket, rspPacket.pHeader->length);
#if 0
                        dprintf(DPRINTF_DEFAULT, "DDRiSCSIStats: session on %s tgt.\n",
                                ((MRGETSESSIONS_RSP *)(pBuf))->sessionInfo[0].targetName);
#endif  /* 0 */
                        remlen = rspPacket.pHeader->length;
                        flen = 12 + (((MRGETSESSIONS_RSP *)(rspPacket.pPacket))->numSessions) * sizeof(MRSESSION);
                        pBuf += flen;
                        remlen -= flen;
                    }
                    else
                    {
#if 0
                        memcpy(pBuf,
                               ((MRGETSESSIONS_RSP *)(rspPacket.pPacket))->sessionInfo,
                               rspPacket.pHeader->length - 12);
                        dprintf(DPRINTF_DEFAULT, "DDRiSCSIStats: session on %s tgt.\n",
                                ((MRSESSION *)(pBuf))->targetName);
                        pBuf += ((rspPacket.pHeader->length) - 12);
#endif  /* 0 */
                        plen = (((MRGETSESSIONS_RSP *)(rspPacket.pPacket))->numSessions) * sizeof(MRSESSION);
                        memcpy(pBuf,
                               ((MRGETSESSIONS_RSP *)(rspPacket.pPacket))->sessionInfo,
                               plen);
#if 0
                        dprintf(DPRINTF_DEFAULT, "DDRiSCSIStats:rec %d bytes session on %s tgt.\n",
                                plen, ((MRSESSION *)(pBuf))->targetName);
#endif  /* 0 */
                        pBuf += plen;
                    }
                    numTargets++;
                }
            }
            /*
             * Done
             */
            Free(rspPacket.pPacket);
            //break;
        }
        pCacheEntry += ((MRGETTARG_RSP *)pCacheEntry)->header.len;
    }
    ((MRGETSESSIONS_RSP *)(pStartBuffer))->numSessions = numSessions;
#if 0
    for (i = 0; i < numSessions; i++)
    {
        dprintf(DPRINTF_DEFAULT, "DDRiSCSIStats: cum. session on tgt %s.\n",
                ((MRGETSESSIONS_RSP *)(pStartBuffer))->sessionInfo[i].targetName);
    }
#endif  /* 0 */
    pBuf += remlen;
    dprintf(DPRINTF_DEFAULT, "DDRiSCSIStats returning %d bytes.\n",
            (INT8 *)pBuf - (INT8 *)pStartBuffer);
    return (INT8 *)pBuf - (INT8 *)pStartBuffer;
}
#endif  /* ISCSI_CODE */

/*****************************************************************************
** Function Name:   DDRFIDList()
**
** Description:     Return the list of FIDs that should be gathered on
**                  a snapshot dump.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRFIDList(void *pAddr, UNUSED UINT32 len)
{
    UINT8      *pStartBuffer = pAddr;
    UINT8      *pBuf = pAddr;


    /*
     * Copy the count of FIDs into the buffer and increment the buffer pointer.
     */
    *((UINT16 *)pBuf) = dimension_of(fidList);
    pBuf += sizeof(UINT16);

    /*
     * Copy fidList[] into the buffer and increment the buffer pointer.
     */
    memcpy(pBuf, fidList, sizeof(fidList));
    pBuf += sizeof(fidList);


    return (INT8 *)pBuf - (INT8 *)pStartBuffer;
}

/*****************************************************************************
** Function Name:   DDRStatsProc()
**
** Description:     Gather Proc stats assemble in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRStatsProc(void *addr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    XIO_PACKET  reqPacket = { &reqHdr, NULL };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    INT32       rc;
    UINT8      *startBufP = addr;
    UINT8      *bufP = addr;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    /* Set up for the stats call */
    reqPacket.pHeader->commandCode = PI_STATS_PROC_CMD;
    reqPacket.pHeader->length = 0;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsProc: Failed to retrieve Proc stats.\n");
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsProc: Returning %d bytes.\n",
                rspPacket.pHeader->length);
        memcpy(bufP, rspPacket.pPacket, rspPacket.pHeader->length);
        bufP += rspPacket.pHeader->length;
    }

    /* Done */
    Free(rspPacket.pPacket);
    return (INT8 *)bufP - (INT8 *)startBufP;
}

/*****************************************************************************
** Function Name:   DDRStatsPCI()
**
** Description:     Gather PCI stats assemble in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRStatsPCI(void *addr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    XIO_PACKET  reqPacket = { &reqHdr, NULL };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    INT32       rc;
    UINT8      *startBufP = addr;
    UINT8      *bufP = addr;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    /* Set up for the stats call */
    reqPacket.pHeader->commandCode = PI_STATS_PCI_CMD;
    reqPacket.pHeader->length = 0;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsPCI: Failed to retrieve PCI stats.\n");
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsPCI: Returning %d bytes.\n",
                rspPacket.pHeader->length);
        memcpy(bufP, rspPacket.pPacket, rspPacket.pHeader->length);
        bufP += rspPacket.pHeader->length;
    }

    /* Done */
    Free(rspPacket.pPacket);
    return (INT8 *)bufP - (INT8 *)startBufP;
}


/*****************************************************************************
** Function Name:   DDRStatsEnviron()
**
** Description:     Gather Environmental stats assemble in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRStatsEnviron(void *addr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    XIO_PACKET  reqPacket = { &reqHdr, NULL };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    INT32       rc;
    UINT8      *startBufP = addr;
    UINT8      *bufP = addr;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    /* Set up for the stats call */
#ifdef ENABLE_NG_HWMON
    reqPacket.pHeader->commandCode = PI_ENV_II_GET_DATA_CMD;
#else   /* ENABLE_NG_HWMON */
    reqPacket.pHeader->commandCode = PI_STATS_ENVIRONMENTAL_CMD;
#endif  /* ENABLE_NG_HWMON */
    reqPacket.pHeader->length = 0;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsEnviron: Failed to retrieve Environmental stats.\n");
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsEnviron: Returning %d bytes.\n",
                rspPacket.pHeader->length);
        memcpy(bufP, rspPacket.pPacket, rspPacket.pHeader->length);
        bufP += rspPacket.pHeader->length;
    }

    /* Done */
    Free(rspPacket.pPacket);
    return (INT8 *)bufP - (INT8 *)startBufP;
}

/*****************************************************************************
** Function Name:   DDRStatsVDisk()
**
** Description:     Gather VDisk stats assemble in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRStatsVDisk(void *addr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    XIO_PACKET  reqPacket = { &reqHdr, NULL };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    INT32       rc;
    UINT8      *startBufP = addr;
    UINT8      *bufP = addr;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    /* Set up for the stats call */
    reqPacket.pHeader->commandCode = PI_STATS_VDISK_CMD;
    reqPacket.pHeader->length = 0;
    reqPacket.pHeader->packetVersion = 2;
    rspPacket.pHeader->packetVersion = 2;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsVDisk: Failed to retrieve VDisk stats.\n");
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsVDisk: Returning %d bytes.\n",
                rspPacket.pHeader->length);
        memcpy(bufP, rspPacket.pPacket, rspPacket.pHeader->length);
        bufP += rspPacket.pHeader->length;
    }

    /* Done */
    Free(rspPacket.pPacket);
    return (INT8 *)bufP - (INT8 *)startBufP;
}


/*****************************************************************************
** Function Name:   DDRStatsCacheDevices()
**
** Description:     Gather CacheDevice stats assemble in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRStatsCacheDevices(void *addr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    XIO_PACKET  reqPacket = { &reqHdr, NULL };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    INT32       rc;
    UINT8      *startBufP = addr;
    UINT8      *bufP = addr;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    /* Set up for the stats call */
    reqPacket.pHeader->commandCode = PI_STATS_CACHE_DEVICES_CMD;
    reqPacket.pHeader->length = 0;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsCacheDevices: Failed to retrieve CacheDevice stats.\n");
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsCacheDevices: Returning %d bytes.\n",
                rspPacket.pHeader->length);
        memcpy(bufP, rspPacket.pPacket, rspPacket.pHeader->length);
        bufP += rspPacket.pHeader->length;
    }

    /* Done */
    Free(rspPacket.pPacket);
    return (INT8 *)bufP - (INT8 *)startBufP;
}


/*****************************************************************************
** Function Name:   DDRStatsLoop()
**
** Description:     Gather Loop stats assemble in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/

/*
**  Format of output buffer:
**
**  UINT32              lenFeLoopStats;
**  PI_STATS_LOOPS_RSP  feLoopStats;
**  UINT32              lenBeLoopStats;
**  PI_STATS_LOOPS_RSP  beLoopStats;
*/

static UINT32 DDRStatsLoop(void *addr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    PI_STATS_LOOPS_REQ reqData = { PORT_STATS_RLS, {0, 0, 0} }; /* extended loop data */
    XIO_PACKET  reqPacket = { &reqHdr, (UINT8 *)&reqData };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    INT32       rc;
    UINT8      *startBufP = addr;
    UINT8      *bufP = addr;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    /* Set up for the stats call */
    reqPacket.pHeader->commandCode = PI_STATS_FRONT_END_LOOP_CMD;
    reqPacket.pHeader->length = sizeof(reqData);
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsLoop: Failed to retrieve FE Loop stats.\n");

        *(UINT32 *)bufP = sizeof(PI_STATS_LOOPS_RSP);
        bufP += sizeof(UINT32);

        memset(bufP, 0, sizeof(PI_STATS_LOOPS_RSP));
        bufP += sizeof(PI_STATS_LOOPS_RSP);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsLoop: Copying %d bytes to buffer (FE).\n",
                rspPacket.pHeader->length);

        *(UINT32 *)bufP = rspPacket.pHeader->length;
        bufP += sizeof(UINT32);

        memcpy(bufP, rspPacket.pPacket, rspPacket.pHeader->length);
        bufP += rspPacket.pHeader->length;
    }

    /* Done with the first returned packet */
    Free(rspPacket.pPacket);

    /* Set up for the stats call */
    reqPacket.pHeader->commandCode = PI_STATS_BACK_END_LOOP_CMD;
    reqPacket.pHeader->length = sizeof(reqData);

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsLoop: Failed to retrieve BE Loop stats.\n");

        *(UINT32 *)bufP = sizeof(PI_STATS_LOOPS_RSP);
        bufP += sizeof(UINT32);

        memset(bufP, 0, sizeof(PI_STATS_LOOPS_RSP));
        bufP += sizeof(PI_STATS_LOOPS_RSP);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsLoop: Copying %d bytes to buffer (BE).\n",
                rspPacket.pHeader->length);

        *(UINT32 *)bufP = rspPacket.pHeader->length;
        bufP += sizeof(UINT32);

        memcpy(bufP, rspPacket.pPacket, rspPacket.pHeader->length);
        bufP += rspPacket.pHeader->length;
    }

    /* Done */
    Free(rspPacket.pPacket);
    return (INT8 *)bufP - (INT8 *)startBufP;
}


/*****************************************************************************
** Function Name:   DDRStatsServer()
**
** Description:     Gather Server stats assemble in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/

/*
**  Format of output buffer:
**
**  UINT16    count;
**  UINT16    sid_list[count];
**
**  PI_STATS_SERVER_REQ   sid_list[0];
**  PI_STATS_SERVER_REQ   sid_list[1];
**       :                  :
**       :                  :
**  PI_STATS_SERVER_REQ   sid_list[count-1];
*/

static UINT32 DDRStatsServer(void *addr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    XIO_PACKET  reqPacket = { &reqHdr, NULL };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    PI_STATS_SERVER_REQ statsSrvReq = { 0, 0 };
    PI_LIST_RSP *pSrvList = NULL;
    UINT16      count;
    UINT32      size;
    INT32       rc;
    UINT32      i;
    UINT8      *startBufP = addr;
    UINT8      *bufP = addr;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    /* Get the list of Servers */
    reqPacket.pHeader->commandCode = PI_SERVER_LIST_CMD;
    reqPacket.pHeader->length = 0;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsServer: Failed to retrieve Server list.\n");
        /* If you can't get this to work you are SOL. Exit now...  */
        return 0;
    }

    /* Copy in the count */
    pSrvList = (PI_LIST_RSP *)rspPacket.pPacket;
    count = *(UINT16 *)bufP = pSrvList->count;
    bufP += (sizeof(UINT16));

    size = sizeof(UINT16) * count;

    dprintf(DPRINTF_DEFAULT, "DDRStatsServer: numSrv %d\n", count);

    /* Copy in the device list */
    memcpy(bufP, pSrvList->list, size);
    bufP += size;

    /* Retrieve the server stats each server, copy data in */
    reqPacket.pHeader->commandCode = PI_STATS_SERVER_CMD;
    reqPacket.pHeader->length = sizeof(statsSrvReq);
    reqPacket.pPacket = (UINT8 *)&statsSrvReq;

    for (i = 0; i < count; i++)
    {
        /* Get the sid */
        statsSrvReq.sid = pSrvList->list[i];

        /* Issue the command through the packet command handler */
        rc = PacketCommandHandler(&reqPacket, &rspPacket);

        if (rc != PI_GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "DDRStatsServer: Failed to retrieve server stats for sid %d, rc %d.\n",
                    statsSrvReq.sid, rc);

            /* Copy in a zeroed out structure on failure */
            memset(bufP, 0, sizeof(PI_STATS_SERVER_RSP));
            bufP += sizeof(PI_STATS_SERVER_RSP);
        }
        else
        {
            /* Copy in the server stats data */
            memcpy(bufP, rspPacket.pPacket, sizeof(PI_STATS_SERVER_RSP));
            bufP += sizeof(PI_STATS_SERVER_RSP);

            /* Free the response data if it exists */
            Free(rspPacket.pPacket);
        }
    }

    /* Done */
    Free(pSrvList);
    return (INT8 *)bufP - (INT8 *)startBufP;
}


/*****************************************************************************
** Function Name:   DDRStatsFCALCounters()
**
** Description:     Gather pdisk loop counter stats and assemble the list
**                  in the "big" buffer.
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/

/*
**  Format of output buffer:
**
**  UINT16    count;
**  UINT16    pid_list[count];
**
**  MRGETPINFO_RSP        pid_list[0];
**  PI_DEBUG_SCSI_CMD_RSP pid_list[0];
**
**  MRGETPINFO_RSP        pid_list[1];
**  PI_DEBUG_SCSI_CMD_RSP pid_list[1];
**       :                  :
**       :                  :
**  MRGETPINFO_RSP        pid_list[count-1];
**  PI_DEBUG_SCSI_CMD_RSP pid_list[count-1];
*/

#define MAX_LOG_SENSE_DATA_LEN   256
#define MAX_LOG_SENSE_RETURN_LEN (MAX_LOG_SENSE_DATA_LEN + sizeof(PI_DEBUG_SCSI_CMD_RSP))

static UINT32 DDRStatsFCALCounters(void *addr, UNUSED UINT32 len)
{
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    XIO_PACKET  reqPacket = { &reqHdr, NULL };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    PI_DEBUG_SCSI_CMD_REQ scsiReq;
    PI_DEBUG_SCSI_CMD_RSP badScsiRsp;
    MRGETPINFO_RSP pdInfo,
               *ptrPdInfo;
    PI_LIST_RSP *pDevList = NULL;
    INT32       rc;
    UINT16      count;
    UINT16      pid;
    UINT32      size;
    UINT32      i;
    UINT8      *startBufP = addr;
    UINT8      *bufP = addr;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));
    memset(&scsiReq, 0, sizeof(scsiReq));
    memset(&badScsiRsp, 0, sizeof(badScsiRsp));

    /* Get the list of PDisks */
    reqPacket.pHeader->commandCode = PI_PDISK_LIST_CMD;
    reqPacket.pHeader->length = 0;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);
    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "DDRStatsFCALCounters: Failed to retrieve PDisk list.\n");
        /* If you can't get this to work you are SOL. Exit now... */
        return 0;
    }

    /*
     * Copy the response data pointer and the set it to null,
     * since are now using it for the device list (it will be freed later).
     */
    pDevList = (PI_LIST_RSP *)rspPacket.pPacket;
    rspPacket.pPacket = NULL;

    /* Copy in the count */
    count = *(UINT16 *)bufP = pDevList->count;
    bufP += (sizeof(UINT16));

    size = sizeof(UINT16) * count;

    dprintf(DPRINTF_DEFAULT, "DDRStatsFCALCounters: numDev %d\n", count);

    /* Copy in the device list */
    memcpy(bufP, pDevList->list, size);
    bufP += size;

    /* Retrieve the loop counters for each device, copy data in */
    reqPacket.pHeader->commandCode = PI_DEBUG_SCSI_COMMAND_CMD;
    reqPacket.pHeader->length = sizeof(scsiReq);
    reqPacket.pPacket = (UINT8 *)&scsiReq;
    scsiReq.cdbLen = 10;
    memcpy(scsiReq.cdb, "\x4D\x00\x4D\x00\x00\x00\x00\x01\x00\x00", scsiReq.cdbLen);

    for (i = 0; i < count; i++)
    {
        while (EL_TestInProgress())
        {
            TaskSleepMS(1000);      /* Wait for a second before continuing. */
        }

        /* Get the pid */
        pid = pDevList->list[i];

        /* Go get the actual pdisk info if available, else use the cache */
        ptrPdInfo = PhysicalDisk(pid);
        if (ptrPdInfo)
        {
            pdInfo = *ptrPdInfo;
            Free(ptrPdInfo);
            rc = PI_GOOD;
        }
        else
        {
            rc = GetPDiskInfoFromPid(pid, &pdInfo);
        }

        /* Copy the pdisk info data into the buffer */
        memcpy(bufP, &pdInfo, sizeof(pdInfo));
        bufP += sizeof(pdInfo);

        /* Go issue the scsi log sense command */
        if (rc == PI_GOOD)
        {

            if (!(pdInfo.pdd.devType == PD_DT_FC_DISK ||
                  pdInfo.pdd.devType == PD_DT_ECON_ENT))
            {
                /* Not a supported drive for FCAL counters */
                dprintf(DPRINTF_DEFAULT, "DDRStatsFCALCounters: Unsupported drive for stats - pid %d\n",
                        pid);
                /* Copy in a zeroed out area for this pid */
                size = sizeof(badScsiRsp);
                memcpy(bufP, &badScsiRsp, size);
                bufP += size;
            }
            else
            {
                scsiReq.wwnLun.wwn = pdInfo.pdd.wwn;
                scsiReq.wwnLun.lun = pdInfo.pdd.lun;

                /* Issue the SCSI command to the selected device */
                dprintf(DPRINTF_DEFAULT, "DDRStatsFCALCounters: retrieving loop counter stats for pid %d\n",
                        pid);
                rc = PacketCommandHandler(&reqPacket, &rspPacket);

                if (rc != PI_GOOD)
                {
                    dprintf(DPRINTF_DEFAULT, "DDRStatsFCALCounters: Failed to retrieve Log Sense Data from pid %d, rc %d.\n",
                            pid, rc);
                }

                switch (rc)
                {
                    case PI_GOOD:
                        /* If successful, copy the returned data to the buffer */
                        size = MAX_LOG_SENSE_RETURN_LEN;
                        ((PI_DEBUG_SCSI_CMD_RSP *)rspPacket.pPacket)->length = MAX_LOG_SENSE_DATA_LEN;
                        memcpy(bufP, (PI_DEBUG_SCSI_CMD_RSP *)rspPacket.pPacket, size);
                        bufP += size;
                        break;

                    case PI_ERROR:
                        /* If error, copy in sense data anyway (Make sure length is 0) */
                        ((PI_DEBUG_SCSI_CMD_RSP *)rspPacket.pPacket)->length = 0;
                        size = sizeof(badScsiRsp);
                        memcpy(bufP, (PI_DEBUG_SCSI_CMD_RSP *)rspPacket.pPacket, size);
                        bufP += size;
                        break;

                    default:
                        /* Copy in a zeroed out area for this pid */
                        size = sizeof(badScsiRsp);
                        memcpy(bufP, &badScsiRsp, size);
                        bufP += size;
                        break;
                }

                /* Free the response data if it exists */
                Free(rspPacket.pPacket);
            }
        }
    }

    Free(pDevList);
    return (INT8 *)bufP - (INT8 *)startBufP;
}


/*****************************************************************************
** Function Name:   DDREmpty()
**
** Returns: zero for number of bytes formatted.
******************************************************************************/
static UINT32 DDREmpty(UNUSED void *addr, UNUSED UINT32 len)
{
    return 0;
}

/*****************************************************************************
** Function Name:   DDRProcDDRTables()
**
** Description:     Read Proc DDR Tables.
**
** Input:           addr    - to write the data
**                  length  - amount of data to write (unused)
**
** Returns:         length in bytes of formatted data.
**
******************************************************************************/

/*
 *  Format of output buffer:
 *
 *  UINT8 eyeCatcher[16];
 *  PROC_DDR_TABLE  feTable;
 *  UINT8 eyeCatcher[16];
 *  PROC_DDR_TABLE  beTable;
 *
 */
static UINT32 DDRProcDDRTables(void *addr, UNUSED UINT32 len)
{
    UINT8      *startBufP = addr;
    UINT8      *bufP = addr;
    PROC_DDR_TABLE *pProcDDRT;
    UINT32      length;

    /* FE first */
    memcpy(bufP, "FE DDR TABLE:   ", 16);
    bufP += 16;
    pProcDDRT = (PROC_DDR_TABLE *)FE_DDR_BASE_ADDR;
    length = sizeof(*pProcDDRT) + (pProcDDRT->numEntries * sizeof(PROC_DDR_ENTRY));
    memcpy(bufP, pProcDDRT, length);
    bufP += length;

    /* BE last */
    memcpy(bufP, "BE DDR TABLE:   ", 16);
    bufP += 16;
    pProcDDRT = (PROC_DDR_TABLE *)BE_DDR_BASE_ADDR;
    length = sizeof(*pProcDDRT) + (pProcDDRT->numEntries * sizeof(PROC_DDR_ENTRY));
    memcpy(bufP, pProcDDRT, length);
    bufP += length;

    /* Done */
    return (INT8 *)bufP - (INT8 *)startBufP;
}


/*****************************************************************************
** Function Name:   CpyCacheFWHdr()
**
** Description:     Helper function for DDRFWVersions(). Copies
**                  FW header from cache to output buffer.
******************************************************************************/
static void CpyCacheFWHdr(char **bufP, UINT16 type, const char *name)
{
    MR_FW_HEADER_RSP hdrOut;

    *bufP += sprintf(*bufP, "%s", name);

    GetFirmwareHeader(type, &hdrOut);

    memcpy(*bufP, &hdrOut.fw, sizeof(hdrOut.fw));
    *bufP += sizeof(hdrOut.fw);
}

/*****************************************************************************
** Function Name:   DDRFWVersions()
**
** Description:     Gather all known FW version data (runtime and flash copies).
**
** Input:           addr - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/

/*
**  Format of output buffer:
**
**  UINT32 count;                Number of entries
**  UINT32 reserved[3];
**  UINT8 eyeCatcher[16];        Name of FW 1
**  FW_HEADER header;            FW header 1
**  UINT8 eyeCatcher[16];        Name of FW 2
**  FW_HEADER header;            FW header 2
**            :
*/
static UINT32 DDRFWVersions(void *addr, UNUSED UINT32 len)
{
    char       *startBufP = addr;
    char       *bufP = addr;
    UINT32      count = 0;

    /*
     * Make room for the count field. We'll fill it in later.
     */
    memset(bufP, 0, 16);
    bufP += 16;

    /*
     * Get the running FW versions
     */
    CpyCacheFWHdr(&bufP, FW_HDR_TYPE_CCB_RUNTIME, "CCB RUNTIME  RUN");
    count++;
    CpyCacheFWHdr(&bufP, FW_HDR_TYPE_BE_RUNTIME, "BE RUNTIME   RUN");
    count++;
    CpyCacheFWHdr(&bufP, FW_HDR_TYPE_FE_RUNTIME, "FE RUNTIME   RUN");
    count++;

    *(UINT32 *)startBufP = count;       /* Fill in the count field */

    return bufP - startBufP;
}


/*****************************************************************************
** Function Name:   DDRTimeStamp()
**
** Returns:         number of bytes formatted
******************************************************************************/
static UINT32 DDRTimeStamp(UNUSED void *addr, UNUSED UINT32 len)
{
    /*
     * Go get the timestamp from the RTC
     */
    RTC_GetTimeStamp((TIMESTAMP *)gBigBuffer);

    return sizeof(TIMESTAMP);
}


/*****************************************************************************
** Function Name:   DDRFCMCounters()
**
** Description:     Gather the FCM counters
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRFCMCounters(void *addr, UINT32 len)
{
    UINT8      *startBufP = addr;
    UINT8      *bufP = addr;

    /*
     * Make a synthetic event, which causes the FCM counters to be refreshed.
     */
    FCM_CountersMinorStorageEvent();

    /*
     * Return the FCM data to the requester
     */
    bufP += FCM_CountersGetBinaryData(addr, len);

    return (INT8 *)bufP - (INT8 *)startBufP;
}


/*****************************************************************************
** Function Name:   DDRMirrorPartnerList()
**
** Description:     Gather the Mirror Partner Information
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRMirrorPartnerList(void *addr, UNUSED UINT32 len)
{
    PI_VCG_GET_MP_LIST_RSP *pMPList;
    UINT32      size;

    /*
     * Request the mirror partner information from Proc
     */
    pMPList = SM_GetMirrorPartnerList();

    if (pMPList != NULL)
    {
        /*
         * Calculate amount of returned data
         */
        size = sizeof(*pMPList) + (pMPList->count * sizeof(MRGETMPLIST_RSP_INFO));

        /*
         * Copy it to the output buffer
         */
        memcpy(addr, pMPList, size);
        Free(pMPList);
    }
    else
    {
        size = 0;
    }

    /*
     * Done
     */
    return size;
}

/*****************************************************************************
** Function Name:   DDRisLinuxFileReadFid()
**
** Description:     Checks fid to see if it is a Linux File to read
**
** Input:           fid     - fid to check
**
** Returns:         1 on SUCCESS, 0 on Failure.
**
******************************************************************************/
UINT8 DDRisLinuxFileReadFid(UINT32 fid)
{
    return ((fid >= CCB_FID) && (fid < FE_FID) &&
            ((fid & 0xFF) >= CCB_LINUX_FILE_READ_START) &&
            ((fid & 0xFF) <= CCB_LINUX_FILE_READ_END));
}

/*****************************************************************************
** Function Name:   DDRLinuxFileSet()
**
** Description:     Set the Linux file to read
**
** Input:           fid    - fid being used
**                  addr   - of where to write from
**                  length - of data to write
**
** Returns:         GOOD on SUCCESS, ERROR on Failure.
**
******************************************************************************/
INT32 DDRLinuxFileSet(const void *addr, UINT32 len)
{
    INT32       rc = GOOD;
    char        cmdBuf[160];

    /*
     * If the file was somehow left open, try to close it.
     */
    if (linFile)
    {
        DDRLinuxFileClose();
    }

    /*
     * Set the filename.
     */
    if (len < (MAX_LINUX_FNAME_SIZE - 1))
    {
        strncpy(linuxReadFileName, (char *)addr, len);
        linuxReadFileName[len] = '\0';

        /*
         * Create the cmd buffer.
         */
        sprintf(cmdBuf, "./xioFidScript %s", linuxReadFileName);
        dprintf(DPRINTF_DEFAULT, "DDRLinuxFileSet: [%s]\n", cmdBuf);

        /*
         * Make the system call
         */
        rc = XK_System(cmdBuf);

        /*
         * Check the status of the call.
         */
        if (rc != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "DDRLinuxFileSet - xioFidScript returned %d for file %s\n",
                    rc, linuxReadFileName);
            rc = ERROR;
            remove(LINUX_ZIPPED_FILENAME);
        }

        /* Clear out the filename */
        memset(cmdBuf, 0x00, 160);
        memset(linuxReadFileName, 0x00, MAX_LINUX_FNAME_SIZE);
    }

    else
    {
        rc = ERROR;
    }

    /*
     * Done
     */
    return rc;
}

/*****************************************************************************
** Function Name:   DDRLinuxFileOpen()
**
** Description:     Opem Linux file and return the size.
**
** Input:           addr   - address of buffer
**                  length - of buffer
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRLinuxFileOpen(void *addr, UINT32 len)
{
    UINT32      size = 0;
    UINT32      sizeRead = 0;

    /*
     * If the file was somehow left open, try to close it.
     */
    if (linFile)
    {
        DDRLinuxFileClose();
    }

    /*
     * attempt to open the file.
     */
    linFile = fopen(LINUX_ZIPPED_FILENAME, "r");

    /*
     * If we successfully opened the file, continue.
     */
    if (linFile)
    {
        /*
         * Read in the file.
         */
        while ((sizeRead = fread((void *)((UINT32)addr), 1, len, linFile)))
        {
            size += sizeRead;
        }

        /*
         * Seek back to the beginning of the file.
         */
        fseek(linFile, 0, SEEK_SET);

        /*
         * If the file is empty, close it.
         */
        if (size == 0)
        {
            DDRLinuxFileClose();
        }
    }

    /*
     * Could not open file.
     */
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRLinuxFileOpen - Could not open file %s!",
                LINUX_ZIPPED_FILENAME);
    }

    return size;
}

/*****************************************************************************
** Function Name:   DDRLinuxFileClose()
**
** Description:     Opem Linux file and return the size.
**
** Input:           addr   - address of buffer
**                  length - of buffer
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
void DDRLinuxFileClose(void)
{
    if (linFile)                /* Make sure we have a file name to read.  */
    {
        /* Close the file. */
        Fclose(linFile);
    }
    else                        /* No file specified. */
    {
        dprintf(DPRINTF_DEFAULT, "DDRLinuxFileClose - File not opened!");
    }

    /*
     * Clear out the filename
     */
    remove(LINUX_ZIPPED_FILENAME);
    linFile = NULL;
}

/*****************************************************************************
** Function Name:   DDRLinuxFileRead()
**
** Description:     Read a Linux file into the given location bigbuffer
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
UINT32 DDRLinuxFileRead(void *addr, UINT32 len)
{
    UINT32      size = 0;
    UINT32      sizeRead = 0;

    /*
     * If we successfully opened the file, continue.
     */
    if (linFile)
    {
        /*
         * Read in the file.
         */
        while ((sizeRead = fread((void *)((UINT32)addr + size),
                                 1, (len - size), linFile)))
        {
            size += sizeRead;
        }
    }

    /*
     * File not opened.
     */
    else
    {
        dprintf(DPRINTF_DEFAULT, "DDRLinuxFileRead - File not opened!");
    }

    /*
     * Done
     */
    return size;
}

/*****************************************************************************
** Function Name:   DDRLinuxFileReadPamLog()
**
** Description:     Read a Pam Log file into the given location bigbuffer
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRLinuxFileReadPamLog(void *addr, UINT32 len)
{
    /*
     * Set the file, and read it up
     */
    if (GOOD == DDRLinuxFileSet(LINUX_PAM_LOGS, strlen(LINUX_PAM_LOGS)))
    {
        return DDRLinuxFileOpen(addr, len);
    }

    return 0;
}

/*****************************************************************************
** Function Name:   DDRLinuxFileReadLinuxLog()
**
** Description:     Read a Linux system logs
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRLinuxFileReadLinuxLog(void *addr, UINT32 len)
{
    /*
     * Set the file, and read it up
     */
    if (GOOD == DDRLinuxFileSet(LINUX_SYSTEM_LOGS, strlen(LINUX_SYSTEM_LOGS)))
    {
        return DDRLinuxFileOpen(addr, len);
    }

    return 0;
}

/*****************************************************************************
** Function Name:   DDRLinuxFileReadRaidLog()
**
** Description:     Read Raid Logs
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRLinuxFileReadRaidLog(void *addr, UINT32 len)
{
    /*
     * Set the file, and read it up
     */
    if (GOOD == DDRLinuxFileSet(LINUX_RAID_LOGS, strlen(LINUX_RAID_LOGS)))
    {
        return DDRLinuxFileOpen(addr, len);
    }

    return 0;
}

/*****************************************************************************
** Function Name:   DDRLinuxFileCoreSummary()
**
** Description:     Read Core Summaries
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRLinuxFileCoreSummary(void *addr, UINT32 len)
{
    /*
     * Set the file, and read it up
     */
    if (GOOD == DDRLinuxFileSet(LINUX_CORE_SUMMARY, strlen(LINUX_CORE_SUMMARY)))
    {
        return DDRLinuxFileOpen(addr, len);
    }

    return 0;
}

/*****************************************************************************
** Function Name:   DDRLinuxFileQLCores()
**
** Description:     Read qlogic Core Summaries
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRLinuxFileQLCores(void *addr, UINT32 len)
{
    /*
     * Set the file, and read it up
     */
    if (GOOD == DDRLinuxFileSet(LINUX_QLOGIC_CORES, strlen(LINUX_QLOGIC_CORES)))
    {
        return DDRLinuxFileOpen(addr, len);
    }

    return 0;
}

/*****************************************************************************
** Function Name:   DDRLinuxFileCores()
**
** Description:     Read Cores
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRLinuxFileCores(void *addr, UINT32 len)
{
    /*
     * Set the file, and read it up
     */
    if (GOOD == DDRLinuxFileSet(LINUX_CORES, strlen(LINUX_CORES)))
    {
        return DDRLinuxFileOpen(addr, len);
    }

    return 0;
}

/*****************************************************************************
** Function Name:   DDRLinuxFileReadAppLog()
**
** Description:     Read Application logs
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/
static UINT32 DDRLinuxFileReadAppLog(void *addr, UINT32 len)
{
    /*
     * Set the file, and read it up
     */
    if (GOOD == DDRLinuxFileSet(LINUX_APP_LOGS, strlen(LINUX_APP_LOGS)))
    {
        return DDRLinuxFileOpen(addr, len);
    }

    return 0;
}

/*****************************************************************************
** Function Name:   DDRBaySESData()
**
** Description:     Gather the Bay SES Data
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/

/*
**  Format of output buffer:
**
**  UINT32         count;           // number of bays found
**
**  -- The following two sections are repeated for each bay found --
**
**  UINT32         length;          // length of next section (eyecatcher + data)
**  CHAR           eyecatcher1[8];  // "SES_DEV "
**  SES_DEVICE     data1;           // the SES_DEVICE data structure
**
**  UINT32         length;          // length of next section (eyecatcher + data)
**  CHAR           eyecatcher2[8];  // "PAGE_02 "
**  SES_PAGE_02    data2;           // the PAGE 2 data
**
**  UINT32         length;          // length of next section (eyecatcher + data)
**  CHAR           eyecatcher3[8];  // "PAGE_80 "
**  SES_P80_XTEX   data3;           // the PAGE 80 data
**
**  UINT32         length;          // length of next section (eyecatcher + data)
**  CHAR           eyecatcher4[8];  // "PAGE_81 "
**  SES_P80_XTEX   data4;           // the PAGE 81 data
*/
static UINT32 DDRBaySESData(void *addr, UNUSED UINT32 len)
{
#if defined(MODEL_3000) || defined(MODEL_7400)
    PSES_DEVICE pSES = NULL;
    PSES_PAGE_02 pPage2 = NULL;
    PSES_P80_XTEX pPage80_81 = NULL;
    UINT32      freePage2;
    UINT32      length = 0;
#endif /* MODEL_3000 || MODEL_7400 */
    char       *pBuf = addr;
    UINT32     *pCount = addr;

    /*
     * Initialize the count field. This gets updated as we go.
     */
    *pCount = 0;
    pBuf += sizeof(UINT32);

#if defined(MODEL_3000) || defined(MODEL_7400)
    /*
     * If pSES is NULL the SES data is not available for some reason.
     * Return a string stating this - it makes life easier for the test
     * guys.
     */
    pSES = GetSESList();
    if (pSES == NULL)
    {
        return sizeof(pCount);
    }

    /*
     * If a valid pointer is available follow the pointer to the end
     * of the data (SES data is a linked list).
     */
    while (pSES != NULL)
    {
        /*
         * Increment the count field
         */
        *pCount += 1;

        /*
         * Copy in the main SES_DEVICE data structure
         */
        length = sizeof(SES_DEVICE);
        if ((((INT8 *)pBuf - (INT8 *)addr) + (length + 8)) > len)
        {
            break;              /* we're out of room */
        }

        *(UINT32 *)pBuf = length + 8;   /* 8 is for eyecatcher */
        pBuf += sizeof(UINT32);

        /*
         * Add eyecatcher
         */
        sprintf(pBuf, "SES_DEV");
        pBuf += 8;

        /*
         * Then data
         */
        memcpy(pBuf, pSES, length);
        pBuf += length;

        /*
         * Go get a fresh copy of the page 2 data.
         */
        if ((pSES->devStat == PD_OP) &&
            (NULL != (pPage2 = GetSESPageByWWN(SES_CMD_STATUS, pSES->WWN, pSES->LUN))))
        {
            freePage2 = TRUE;
            dprintf(DPRINTF_DEFAULT, "DDRBaySESData: Returning new PAGE_02 data\n");
        }

        /*
         * If get GetSESPageByWWN() failed, see if there is saved page2 data.
         */
        else
        {
            freePage2 = FALSE;
            pPage2 = pSES->OldPage2;
            if (pPage2)
            {
                dprintf(DPRINTF_DEFAULT, "DDRBaySESData: Returning old PAGE_02 data\n");
            }
        }

        /*
         * If the page 2 pointer is valid capture this info as well.
         */
        if (pPage2 != NULL)
        {
            length = ntohs(pPage2->Length) + 4; /* 4 is for PageCode, Status and Length */
        }
        else
        {
            length = 0;
        }

        if ((((INT8 *)pBuf - (INT8 *)addr) + (length + 8)) > len)
        {
            break;              /* we're out of room */
        }

        *(UINT32 *)pBuf = length + 8;   /* 8 is for eyecatcher */
        pBuf += sizeof(UINT32);

        /*
         * Add eyecatcher
         */
        sprintf(pBuf, "PAGE_02");
        pBuf += 8;

        /*
         * Then data
         */
        memcpy(pBuf, pPage2, length);
        pBuf += length;

        /*
         * Free the page 2 data buffer if necessary.
         */
        if (freePage2)
        {
            Free(pPage2);
        }

        /*
         * If the page 80 pointer is valid capture the data.
         */
        pPage80_81 = pSES->OldPage80;
        if (pPage80_81 != NULL)
        {
            length = ntohs(pPage80_81->Length) + 4;     /* 4 is for PageCode,
                                                         * Status and Length */
        }
        else
        {
            length = 0;
        }

        if ((((INT8 *)pBuf - (INT8 *)addr) + (length + 8)) > len)
        {
            break;              /* we're out of room */
        }

        *(UINT32 *)pBuf = length + 8;   /* 8 is for eyecatcher */
        pBuf += sizeof(UINT32);

        /*
         * Add eyecatcher
         */
        sprintf(pBuf, "PAGE_80");
        pBuf += 8;

        /*
         * Then data
         */
        memcpy(pBuf, pPage80_81, length);
        pBuf += length;

        /*
         * If the page 81 pointer is valid capture the data.
         */
        pPage80_81 = pSES->OldPage81;
        if (pPage80_81 != NULL)
        {
            length = ntohs(pPage80_81->Length) + 4;     /* 4 is for PageCode,
                                                         * Status and Length */
        }
        else
        {
            length = 0;
        }

        if ((((INT8 *)pBuf - (INT8 *)addr) + (length + 8)) > len)
        {
            break;              /* we're out of room */
        }

        *(UINT32 *)pBuf = length + 8;   /* 8 is for eyecatcher */
        pBuf += sizeof(UINT32);

        /*
         * Add eyecatcher
         */
        sprintf(pBuf, "PAGE_81");
        pBuf += 8;

        /*
         * Then data
         */
        memcpy(pBuf, pPage80_81, length);
        pBuf += length;

        /*
         * Move to the next structure in the list
         */
        pSES = pSES->NextSES;
    }
#else  /* MODEL_3000 || MODEL_7400 */
    /*
     * We will not have any bays. So returning 0 count
     * to the fid
     */
#endif /* MODEL_3000 || MODEL_7400 */
    return (INT8 *)pBuf - (INT8 *)addr;
}

/*****************************************************************************
** Function Name:   DDRAsyncRep()
**
** Description:     Gather the Async Replication data
**
** Input:           addr   - of where to write the data
**                  length - of data to write
**
** Returns:         length in bytes of assembled data.
**
******************************************************************************/

/*
**  Format of output buffer:
**
**  UINT32         count;           // number of apools found
**
**  -- The following is repeated for each apool found --
**
**  UINT32         length;          // length of data
**  char           data[length];    // the apool data structure
**
*/
static UINT32 DDRAsyncRep(void *addr, UINT32 len)
{

    UINT32      workinglen;     /**< Length of output as created. */
    MRASYNCREP_REQ *inPkt;      /**< MRP Request from CCB to BE. */
    MRASYNCREP_RSP *outPkt;     /**< MRP Response from BE to CCB. */
    void       *retData;        /**< MRP Data location. */
    UINT16      rc;
    UINT32     *count;
    char       *buf;

    if (len < sizeof(UINT32))
    {
        dprintf(DPRINTF_DEFAULT, "DDRAsyncRep: Input length too short (%u)\n", len);
        return (0);
    }
    inPkt = MallocWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));

    inPkt->blen = sizeof(APOOL) + sizeof(UINT32) + MAX_ALINKS * sizeof(ALINK);

    retData = MallocSharedWC(inPkt->blen);
    inPkt->bptr = retData;

    /*
     * Send the request to BE. Timeouts and task switches are handled.
     */
    rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRGETASYNC, outPkt, sizeof(*outPkt), 35000);

    /*
     * If there was an error, free the buffer allocated and return count zero.
     */
    if (rc != PI_GOOD || outPkt->status != GOOD ||
        outPkt->retLen > sizeof(APOOL) + sizeof(UINT32) + MAX_ALINKS * sizeof(ALINK))
    {
        UINT32     *Apoll_count;        /**< First word is count of apools. */

        dprintf(DPRINTF_DEFAULT, "DDRAsyncRep: ERROR -- rc=%u, status=%u, Len=%u\n",
                rc, outPkt->status, outPkt->retLen);
        if (rc != PI_TIMEOUT)
        {
            Free(inPkt);
            Free(outPkt);
            DelayedFree(MRGETASYNC, retData);
        }
        Apoll_count = addr;
        *Apoll_count = 0;
        return (sizeof(UINT32));
    }

    /*
     * Possibly limit output buffer size for copy.
     */
    workinglen = outPkt->retLen;
    if (workinglen > (len - sizeof(UINT32)))
    {
        workinglen = len - sizeof(UINT32);
    }
    count = addr;
    *count = outPkt->count;
    buf = (char *)(count + 1);
    memcpy(buf, retData, workinglen);

    Free(inPkt);
    Free(outPkt);
    Free(retData);

    return (workinglen + sizeof(UINT32));
}                               /* End of DDRAsyncRep */

/*****************************************************************************
**
** Function Name:   InitDMCtable()  initialize the Direct Memory Copy table.
**
** Inputs:  None
**
** Returns: None
**
******************************************************************************/
void InitCCBDMCtable(void)
{
    struct DMC *entry;

    /* The amount of NVRAM space -- total, used, must reserve. */
    entry = DMC_CCB + CCB_DMC_nvramp2percent;
    Free_DMC_Lock(entry);
    memcpy(entry->id, "nvramP2%", 8);
    entry->copy_addr = &g_nvramp2percentused;
    entry->copy_length = sizeof(g_nvramp2percentused);

#ifdef NO_RAID_CACHE
    /* The ccb "cached" Raid information table. */
    entry = DMC_CCB + CCB_DMC_raidcache;
    memcpy(entry->id, "raid DMC", 8);
    entry->copy_addr = &cacheRaidBuffer_DMC;
    entry->copy_length = sizeof(cacheRaidBuffer_DMC);
    Free_DMC_Lock(entry);
#endif  /* NO_RAID_CACHE */

#ifdef NO_VDISK_CACHE
    /* The ccb "cached" VDisk information table. */
    entry = DMC_CCB + CCB_DMC_vdiskcache;
    memcpy(entry->id, "vdiskDMC", 8);
    entry->copy_addr = &cacheVDiskBuffer_DMC;
    entry->copy_length = sizeof(cacheVDiskBuffer_DMC);
    Free_DMC_Lock(entry);
#endif  /* NO_VDISK_CACHE */

}   /* End of InitCCBDMCtable */

/*****************************************************************************
**
** Function Name:   DMC()  Copy data using Direct Memory Copy table.
**
** Inputs:  length  - Pointer to return length (zero upon entry)
**          id      - Return pointer to 8 chars to receive "name" of entry.
**          fid     - Index into *DMC_CCB array.
**          buffer  - Pointer to buffer to receive data.
**          maxlen  - Maximum length of buffer receiving data.
**
** Returns: PI_GOOD if ok, else PI error.
**          leftToSend (see above)
**          id (see above)
**
******************************************************************************/
INT32 DMC(INT32 *length, char *id, UINT32 fid, void *buffer, UINT32 maxlen)
{
    struct DMC *entry = DMC_CCB +fid;
    INT32       rc;

    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);

    /* Validate entry  and that it has data. */
    if (fid >= CCB_DMC_MAX || entry->copy_length == 0)
    {
        rc = PI_INVALID_CMD_CODE;
    }
    else if (maxlen != 0 && entry->copy_length > maxlen)
    {
        rc = PI_PARAMETER_ERROR;
    }
    else
    {
        /* Copy data. */
        if (length != NULL)
        {
            *length = entry->copy_length;
        }
        memcpy(buffer, entry->copy_addr, entry->copy_length);

        /* Copy in entry name. */
        if (id != NULL)
        {
            memcpy(id, entry->id, 8);
        }
        rc = PI_GOOD;
    }

    /* Done with lock. */
    Free_DMC_Lock(entry);
    return rc;
}   /* End of DMC */

/****************************************************************************/
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
