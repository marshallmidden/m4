/* $Id: PI_Target.h 122127 2010-01-06 14:04:36Z m4 $*/
/*===========================================================================
** FILE NAME:       PI_Target.h
** MODULE TITLE:    Target Commands
**
** DESCRIPTION:     These functions handle requests for target
**                  information.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/

#ifndef __PI_TARGET_H__
#define __PI_TARGET_H__

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

extern PI_TARGETS_RSP *Targets(UINT32 controllerSN);
extern PI_TARGET_RESOURCE_LIST_RSP *TargetResourceList(UINT16 tid, UINT8 type);
extern INT32 FailPort(UINT32 port);
extern INT32 UnfailPort(UINT32 port);
extern INT32 FailCtrl(UINT32 oldOwner, UINT32 newOwner, UINT32 failBack, UINT32 ports);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PI_TARGET_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
