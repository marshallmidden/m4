/* $Id: PI_DiskBay.h 122127 2010-01-06 14:04:36Z m4 $*/
/*===========================================================================
** FILE NAME:       PI_DiskBay.h
** MODULE TITLE:    Disk Bay Commands
**
** DESCRIPTION:     These functions handle requests for disk bay
**                  information.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef __PI_DISKBAY_H__
#define __PI_DISKBAY_H__

#include "PacketInterface.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

extern PI_DISK_BAYS_RSP *DiskBays(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PI_DISKBAY_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
