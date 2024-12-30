/* $Id: PI_VDisk.h 122127 2010-01-06 14:04:36Z m4 $*/
/**
******************************************************************************
**
**  @file       PI_VDisk.h
**
**  @brief      Virtual Disk Commands
**
**  These functions handle requests for virtual disk information.
**
**  Copyright (c) 1996-2008 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef __PI_VDISK_H__
#define __PI_VDISK_H__

#include "XIO_Types.h"
#include "PacketInterface.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern PI_VDISKS_RSP *VirtualDisks(void);
extern PI_RAIDS_RSP *Raids(void);
extern MR_LIST_RSP *RaidsOwnedByController(UINT32 controllerSN);
extern INT32 CheckForNotMirroringRaid5s(MR_LIST_RSP * pRaidList);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PI_VDISK_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
