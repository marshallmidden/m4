/* $Id: X1_Packets.h 145451 2010-08-11 16:50:09Z m4 $*/
/**
******************************************************************************
**
**  @file       X1_Packets.h
**
**  @brief      Command Handlers for X1 Packet Interface Commands
**
**  Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef _X1_PACKETS_H_
#define _X1_PACKETS_H_

#include "globalOptions.h"
#include "mode.h"
#include "XIO_Types.h"
#include "XIOPacket.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define X1_PACKET_LENGTH_SIZE           2

/*
** X1 return statuses for X1_VDISK_CFG and X1_CFG_HAB.
*/
#define X1_OP_SUCCESS                   0
#define X1_OP_ENCRYPT_FAILURE           1
#define X1_OP_RMTCFG_DISABLED           2
#define X1_CMD_FAILED                   3

/**
**  @name X1 Compatibility Index
**
**  Roll this compatibility number based on a request from the GUI team.
**      - Rolled from 13 -> 14 in R2/R2.5 for unknown reason.
**
**      - Rolled from 14 -> 15 in R2/R2.5 for firmware update inactive
**        and rolling code update changes.
**
**      - Rolled from 15 -> 17 in R2.5 code for worksets and defrag/rebuild
**        percentage changes.
**
**      - Rolled from 17 -> 18 in R2.5 code for GeoRAID packet support
**
**      - Rolled from 18 -> 19 in R2.5 code: Changes to validation to
**        enable it to be called from the GUI.
**
**      - Rolled from 19 -> 20 in R2.6 code: X1 RAID5 recovery command.
**
**      - Rolled from 20 -> 21 in Rx code: SATA support
**
**      - Rolled from 21 -> 22 in Rx code: CPU Load command
**
**      - Rolled from 22 -> 23 in Rx code: VPort Assignment
**
**      - Rolled from 23 -> 24 in Rx code: Server and HBA Statistics
**
**  @{
**/
#define X1_COMPAT_GEOPOOL           18
#define X1_COMPAT_VALIDATION        19
#define X1_COMPAT_RAID5             20
#define X1_COMPAT_SATA              21
#define X1_COMPAT_CPU_LOAD          22
#define X1_COMPAT_VPORT             23
#define X1_COMPAT_SERVER_HBA_STATS  24
#define X1_COMPAT_DEFRAG_STATUS     25
#define X1_COMPAT_RESYNC            26
#define X1_COMPAT_RESYNC_VSS_FIX    27

#define X1_COMPAT_VPRI_SERVICEABILITY_3000E    28
#define X1_COMPATIBILITY            X1_COMPAT_VPRI_SERVICEABILITY_3000E

/* @} */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _X1_PACKETS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
