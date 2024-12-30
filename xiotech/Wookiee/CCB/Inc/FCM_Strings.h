/* $Id: FCM_Strings.h 137382 2010-04-19 16:02:18Z mdr $ */
/*============================================================================
** FILE NAME:       FCM_Strings.h
** MODULE TITLE:    Header file for FCM_Strings.c
**
** Copyright (c) 2001  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _FCM_STRINGS_H_
#define _FCM_STRINGS_H_

#include "FCM.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void FCM_GetEventTypeString(char *stringPtr, FCM_EVENT_TYPE eventType,
                                   UINT8 stringLength);
extern void FCM_GetConditionCodeString(char *stringPtr, FCM_CONDITION_CODE eventType,
                                       UINT8 stringLength);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _FCM_STRINGS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
