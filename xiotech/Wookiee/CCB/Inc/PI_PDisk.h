/* $Id: PI_PDisk.h 122127 2010-01-06 14:04:36Z m4 $*/
/*===========================================================================
** FILE NAME:       PI_PDisk.h
** MODULE TITLE:    Physical Disk Commands
**
** DESCRIPTION:     These functions handle requests for physical disk
**                  information.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/

#ifndef __PI_PDISK_H__
#define __PI_PDISK_H__

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*----------------------------------------------------------------------------
** Name:    PhysicalDisks
**
** Desc:    This method will retrieve the physical disk information for all
**          physical disks.
**
** Inputs:  NONE
**
** Returns: Physical Disks response packet.
**
** WARNING: The caller of this method will need to free the response packet
**          after they have finished using it.
**--------------------------------------------------------------------------*/
extern PI_PDISKS_RSP *PhysicalDisks(void);

/*----------------------------------------------------------------------------
** Name:    PhysicalDisk
**
** Desc:    This method will retrieve the physical disk information for a
**          physical disk.
**
** Inputs:  UINT16 pid - Phyiscal disk identifier.
**
** Returns: Physical Disk response packet.
**
** WARNING: The caller of this method will need to free the response packet
**          after they have finished using it.
**--------------------------------------------------------------------------*/
extern PI_PDISK_INFO_RSP *PhysicalDisk(UINT16 pid);

/*----------------------------------------------------------------------------
** Name:    PhysicalDisk_PIDFromWWN
**
** Desc:    This method will retrieve the PID for a device with
**          the given WWN.
**
** Inputs:  UINT64 WWN - World wide name of the device
**          UINT16 LUN - Lun of the device
**
** Returns: UINT16 PID of the device or 0xFFFF if the device does not
**                 exist or the retrieval failed..
**
** WARNING:
**--------------------------------------------------------------------------*/
extern UINT16 PhysicalDisk_PIDFromWWN(UINT64 WWN, UINT16 LUN);


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PI_PDISK_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
