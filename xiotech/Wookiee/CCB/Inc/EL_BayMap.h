/* $Id: EL_BayMap.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       EL_BayMap.h
**
**  @brief      Header file for EL_BayMap.c
**
**  Election code worker functions to track diskMap connectivity.  These
**  should only be called by the election code.
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _EL_BAYMAP_H_
#define _EL_BAYMAP_H_

#include "EL_DiskMap.h"
#include "FIO.h"
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
    UINT8       bayPresent:1;
    UINT8       disksPresent:1;
    UINT8       reserved:6;
} ELECTION_BAY_FLAGS_BITS;

typedef union
{
    UINT8       value;
    ELECTION_BAY_FLAGS_BITS bits;
} ELECTION_BAY_FLAGS;

typedef struct
{
    ELECTION_BAY_FLAGS flags;
    UINT32      goodCount;
    UINT32      unknownCount;
    UINT32      unlabeledCount;
    UINT32      totalCount;
} ELECTION_BAY_DATA;

typedef struct
{
    UINT8       mapValid:1;
    UINT8       reserved:7;
} ELECTION_BAY_MAP_FLAGS_BITS;

typedef union
{
    UINT8       value;
    ELECTION_BAY_MAP_FLAGS_BITS bits;
} ELECTION_BAY_MAP_FLAGS;

typedef struct
{
    ELECTION_BAY_MAP_FLAGS flags;
    ELECTION_BAY_DATA unknownBay;
    ELECTION_BAY_DATA map[MAX_DISK_BAYS];
} ELECTION_BAY_MAP;


typedef enum
{
    BAY_MAP_DISK_STATE_UNKNOWN = 0,
    BAY_MAP_DISK_STATE_WRITABLE = 1,
    BAY_MAP_DISK_STATE_UNWRITABLE = 2,
    BAY_MAP_DISK_STATE_UNLABELED = 3,
} BAY_MAP_DISK_STATE;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT32 EL_BayMapDump(ELECTION_BAY_MAP *bayMapPtr);
extern UINT32 EL_BayMapUpdate(ELECTION_BAY_MAP *bayMapPtr);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _EL_BAYMAP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
