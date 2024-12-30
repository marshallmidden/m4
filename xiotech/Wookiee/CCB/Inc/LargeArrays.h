/* $Id: LargeArrays.h 145021 2010-08-03 14:16:38Z m4 $ */
/*============================================================================
** FILE NAME:       LargeArrays.h
**
** DESCRIPTION:     A single place to define all arrays larger than 64k.
**                  Since the compiler thows a warning for large arrays
**                  this allows us to selectively ignore this warning
**                  when compiling this file.
**
** Copyright (c) 2002-2010 XIOtech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _LARGE_ARRAYS_H_
#define _LARGE_ARRAYS_H_

#include "CacheSize.h"
#include "FCM_Counters.h"
#include "i82559.h"
#include "mutex.h"
#include "trace.h"
#include "XIO_Types.h"
#include "PortServer.h"
#include "quorum.h"
#include "StatsSize.h"
#include "ddr.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define BIG_BUFFER_SIZE     (SIZE_32MEG)        /* 32 Meg total */

/*****************************************************************************
** Public variables
*****************************************************************************/

/* The "Big Buffer" and associated mutex. */
extern char gBigBuffer[BIG_BUFFER_SIZE];
extern MUTEX bigBufferMutex;

/* Trace buffer */
extern UINT8 traceBuffer[TRACE_SIZE];

/* ------------------------------------------------------------------------ */
/* Cache Manager (cachemgr) buffers */
/* #ifndef -- old methodology. */
/* #else -- New DMC method. */
#ifndef NO_PDISK_CACHE
extern UINT8 cachePhysicalDisks1[CACHE_SIZE_PHYSICAL_DISKS];
extern UINT8 cachePhysicalDisks2[CACHE_SIZE_PHYSICAL_DISKS];
#else   /* NO_PDISK_CACHE */
extern struct DMC_pdisk_structure cachePDiskBuffer_DMC LOCATE_IN_SHMEM;
#endif  /* NO_PDISK_CACHE */

#ifndef NO_VDISK_CACHE
extern UINT8 cacheVirtualDisks1[CACHE_SIZE_VIRTUAL_DISKS];
extern UINT8 cacheVirtualDisks2[CACHE_SIZE_VIRTUAL_DISKS];
#else   /* NO_VDISK_CACHE */
extern struct DMC_vdisk_structure cacheVDiskBuffer_DMC LOCATE_IN_SHMEM;
#endif  /* NO_VDISK_CACHE */

#ifndef NO_RAID_CACHE
extern UINT8 cacheRaids1[CACHE_SIZE_RAIDS];
extern UINT8 cacheRaids2[CACHE_SIZE_RAIDS];
#else   /* NO_RAID_CACHE */
extern struct DMC_raid_structure cacheRaidBuffer_DMC;
#endif  /* NO_RAID_CACHE */

#ifndef NO_SERVER_CACHE
extern UINT8 cacheServers1[CACHE_SIZE_SERVERS];
extern UINT8 cacheServers2[CACHE_SIZE_SERVERS];
#else   /* NO_SERVER_CACHE */
extern struct DMC_server_structure cacheServerBuffer_DMC LOCATE_IN_SHMEM;
#endif  /* NO_SERVER_CACHE */
/* ------------------------------------------------------------------------ */

/*
** Entire quroum communications area buffer(s).  These are defined
** to be arrays of QM_CONTROLLER_COMM_AREA structures and can be
** used when reading the entire communications area from the quorum
** using the ReadAllCommunications function.
**
** gElectCommAreas  - Used only by the election code.
** gMQMCommAreas    - Used only by the MasterQuorumManager code.
*/
extern QM_CONTROLLER_COMM_AREA gElectCommAreas[MAX_CONTROLLERS];

extern QM_IPC_MAILBOX gIPCMailbox[QM_MAX_CONTROLLERS];

/* Fibre channel monitor - drive counter maps */
extern FCM_BAY_DATA bayDataList0[MAX_DISK_BAYS];
extern FCM_BAY_DATA bayDataList1[MAX_DISK_BAYS];
extern FCM_BAY_DATA bayDataList2[MAX_DISK_BAYS];

extern FCM_BAY_DATA backupBayDataList0[MAX_DISK_BAYS];
extern FCM_BAY_DATA backupBayDataList1[MAX_DISK_BAYS];
extern FCM_BAY_DATA backupBayDataList2[MAX_DISK_BAYS];
extern FCM_BAY_DATA backupBayDataList3[MAX_DISK_BAYS];

extern FCM_HAB_DATA habDataList0[MAX_BE_PORTS];
extern FCM_HAB_DATA habDataList1[MAX_BE_PORTS];
extern FCM_HAB_DATA habDataList2[MAX_BE_PORTS];

extern FCM_HAB_DATA backupHabDataList0[MAX_BE_PORTS];
extern FCM_HAB_DATA backupHabDataList1[MAX_BE_PORTS];
extern FCM_HAB_DATA backupHabDataList2[MAX_BE_PORTS];
extern FCM_HAB_DATA backupHabDataList3[MAX_BE_PORTS];

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _LARGE_ARRAYS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
