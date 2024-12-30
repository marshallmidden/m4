/* $Id: PacketStats.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       PacketStats.h
** MODULE TITLE:    Packet Statistics
**
** DESCRIPTION: Packet statistics information gathered for PI, X1 and MRP
**              packets.
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _PACKETSTATS_H_
#define _PACKETSTATS_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define PACKET_TYPE_PI      1
#define PACKET_TYPE_X1      2
#define PACKET_TYPE_X1_VDC  3
#define PACKET_TYPE_X1_BF   4
#define PACKET_TYPE_MRP     5
#define PACKET_TYPE_IPC     6

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void IncPacketCount(UINT32 type, UINT32 packetID);
extern UINT8 *GetPacketStatsPointer(UINT32 *statsLength);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _PACKETSTATS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
