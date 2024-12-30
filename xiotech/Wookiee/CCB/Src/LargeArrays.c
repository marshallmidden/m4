/* $Id: LargeArrays.c 145133 2010-08-05 12:43:28Z m4 $ */
/*============================================================================
** FILE NAME:       LargeArrays.c
**
** DESCRIPTION:     A single place to define all arrays larger than 64k.
**                  Since the compiler thows a warning for large arrays
**                  this allows us to selectively ignore this warning
**                  when compiling this file.
**
** Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "ddr.h"
#include "LargeArrays.h"

/* The "Big Buffer" and associated mutex. */
char        gBigBuffer[BIG_BUFFER_SIZE];
MUTEX       bigBufferMutex;

/* Trace buffer */
UINT8       traceBuffer[TRACE_SIZE];

/* ------------------------------------------------------------------------ */
/* Cache Manager (cachemgr) buffers */
/* #ifndef -- Old things. */
/* #else -- New DMC method. */
#ifndef NO_PDISK_CACHE
UINT8       cachePhysicalDisks1[CACHE_SIZE_PHYSICAL_DISKS] LOCATE_IN_SHMEM;
UINT8       cachePhysicalDisks2[CACHE_SIZE_PHYSICAL_DISKS] LOCATE_IN_SHMEM;
#else   /* NO_PDISK_CACHE */
struct DMC_pdisk_structure cachepdiskBuffer_DMC LOCATE_IN_SHMEM;
#endif  /* NO_PDISK_CACHE */

#ifndef NO_VDISK_CACHE
UINT8       cacheVirtualDisks1[CACHE_SIZE_VIRTUAL_DISKS] LOCATE_IN_SHMEM;
UINT8       cacheVirtualDisks2[CACHE_SIZE_VIRTUAL_DISKS] LOCATE_IN_SHMEM;
#else   /* NO_VDISK_CACHE */
struct DMC_vdisk_structure cacheVDiskBuffer_DMC LOCATE_IN_SHMEM;
#endif  /* NO_VDISK_CACHE */

#ifndef NO_RAID_CACHE
UINT8       cacheRaids1[CACHE_SIZE_RAIDS] LOCATE_IN_SHMEM;
UINT8       cacheRaids2[CACHE_SIZE_RAIDS] LOCATE_IN_SHMEM;
#else   /* NO_RAID_CACHE */
struct DMC_raid_structure cacheRaidBuffer_DMC LOCATE_IN_SHMEM;
#endif  /* NO_RAID_CACHE */

#ifndef NO_SERVER_CACHE
UINT8       cacheServers1[CACHE_SIZE_SERVERS] LOCATE_IN_SHMEM;
UINT8       cacheServers2[CACHE_SIZE_SERVERS] LOCATE_IN_SHMEM;
#else   /* NO_SERVER_CACHE */
struct DMC_server_structure cacheserverBuffer_DMC LOCATE_IN_SHMEM;
#endif  /* NO_SERVER_CACHE */
/* ------------------------------------------------------------------------ */

/*
** Entire quorum communications area buffer(s).  These are defined
** to be arrays of QM_CONTROLLER_COMM_AREA structures and can be
** used when reading the entire communications area from the quorum
** using the ReadAllCommunications function.
**
** gElectCommAreas  - Used only by the election code.
** gIPCMailbox      - Used only by the MasterQuorumManager code.
*/
QM_CONTROLLER_COMM_AREA gElectCommAreas[MAX_CONTROLLERS] LOCATE_IN_SHMEM;

/* gIPCMailbox      - Used only by the QuorumManager code. */
QM_IPC_MAILBOX gIPCMailbox[QM_MAX_CONTROLLERS] LOCATE_IN_SHMEM;

/* Fibre channel monitor - drive counter map */
FCM_BAY_DATA bayDataList0[MAX_DISK_BAYS];
FCM_BAY_DATA bayDataList1[MAX_DISK_BAYS];
FCM_BAY_DATA bayDataList2[MAX_DISK_BAYS];

FCM_BAY_DATA backupBayDataList0[MAX_DISK_BAYS];
FCM_BAY_DATA backupBayDataList1[MAX_DISK_BAYS];
FCM_BAY_DATA backupBayDataList2[MAX_DISK_BAYS];
FCM_BAY_DATA backupBayDataList3[MAX_DISK_BAYS];

FCM_HAB_DATA habDataList0[MAX_BE_PORTS];
FCM_HAB_DATA habDataList1[MAX_BE_PORTS];
FCM_HAB_DATA habDataList2[MAX_BE_PORTS];

FCM_HAB_DATA backupHabDataList0[MAX_BE_PORTS];
FCM_HAB_DATA backupHabDataList1[MAX_BE_PORTS];
FCM_HAB_DATA backupHabDataList2[MAX_BE_PORTS];
FCM_HAB_DATA backupHabDataList3[MAX_BE_PORTS];

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
