/* $Id: EL_DiskMap.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       EL_DiskMap.h
**
**  @brief      Header file for EL_DiskMap.c
**
**  Election code worker functions to track diskMap connectivity.  These
**  should only be called by the election code.
**
**  Copyright (c) 2003 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _EL_DISK_MAP_H_
#define _EL_DISK_MAP_H_

#include "FIO.h"
#include "MR_Defs.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
typedef struct
{
    UINT8       mapValid:1;     /* diskMaps are valid (R3 packet received)  */
    UINT8       responseValid:1;        /* Response packet is valid                 */
    UINT8       reserved:6;
} ELECTION_DISK_MAP_FLAGS_BITS;

typedef union
{
    UINT8       value;
    ELECTION_DISK_MAP_FLAGS_BITS bits;
} ELECTION_DISK_MAP_FLAGS;

typedef struct
{
    ELECTION_DISK_MAP_FLAGS flags;
    FIO_DISK_MAP local;
    FIO_DISK_MAP intersect;
} ELECTION_DISK_MAP;

typedef struct
{
    ELECTION_DISK_MAP slotList[MAX_CONTROLLERS];
} ELECTION_DISK_MAP_LIST;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void EL_DiskMapResetList(void);
extern void EL_DiskMapInvalidateList(void);

extern UINT32 EL_DiskMapGet(ELECTION_DISK_MAP *diskMapPtr);
extern UINT32 EL_DiskMapReceive(ELECTION_DISK_MAP *diskMapPtr, UINT32 packetCommSlot);
extern UINT32 EL_DiskMapUpdateFIOMap(void);
extern UINT32 EL_DiskMapSlotOverlap(UINT16 slotNumber);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _EL_DISK_MAP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
