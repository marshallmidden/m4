/* $Id: FIO_Maps.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       FIO_Maps.h
**
**  @brief      Read and write diskMap control for file system
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _FIO_MAPS_H_
#define _FIO_MAPS_H_

#include "EL.h"
#include "FIO.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT32 FIO_GetReadableDiskMap(FIO_DISK_MAP * diskMapPtr);
extern UINT32 FIO_GetWritableDiskMap(FIO_DISK_MAP * diskMapPtr);
extern UINT32 FIO_SetReadableDiskMap(FIO_DISK_MAP * newDiskMapPtr);
extern UINT32 FIO_SetWritableDiskMap(FIO_DISK_MAP * newDiskMapPtr);
extern UINT32 FIO_GetNumDiskMapDisks(FIO_DISK_MAP * diskMapPtr);
extern UINT32 FIO_ResetReadableDiskMap(void);
extern UINT32 FIO_ResetWritableDiskMap(void);
extern void FIO_MapDump(FIO_DISK_MAP * diskMapPtr);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _FIO_MAPS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
