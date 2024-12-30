/* $Id: PI_Stats.h 159129 2012-05-12 06:25:16Z marshall_midden $ */
/*===========================================================================
** FILE NAME:       PI_Stats.h
** MODULE TITLE:    Packet Interface for Stats and Enviromental commands
**
** DESCRIPTION:     These functions handle requests for physical disk
**                  information.
**
** Copyright (c) 2001 - 2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/

#ifndef __PI_STATS_H__
#define __PI_STATS_H__

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern PI_STATS_CACHE_DEVICES_RSP *StatsCacheDevices(void);

#if defined(MODEL_7000) || defined(MODEL_4700)
extern void ISESESStatusHandle(ISE_SES_DEVICE * pise, struct ise_info_version_2 *ise);
extern struct ise_stats_version_2 ise_data[MAX_DISK_BAYS];
extern INT32 SendBeaconIseCommand(UINT16 bayid, UINT16 command, UINT8 light_on_off);
#endif /* MODEL_7000 || MODEL_4700 */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PI_STATS_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
