/* $Id: FCM.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       FCM.h
** MODULE TITLE:    Common header file for the FCM component.
**
** DESCRIPTION:     Fibre Channel Monitor
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _FCM_H_
#define _FCM_H_

#include "XIO_Std.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/* Defines that correspond to the proc code's definitions in isp.h */
#define ASPLIP 0x8010           /* LIP occurred                         */

/*
** Update FCM_GetEventTypeString with any changes to this ENUM
*/
typedef enum FCM_EVENT_TYPE_ENUM
{
    FCM_ET_UNDEFINED = 0,
    FCM_ET_RESTORED = 1,
    FCM_ET_DROPPED_FRAME = 2,
    FCM_ET_LOOP_RESET = 3,
    FCM_ET_RSCN = 4,
} FCM_EVENT_TYPE;

/*
** Update FCM_GetConditionCodeString with any changes to this ENUM
*/
typedef enum FCM_CONDITION_CODE_ENUM
{
    FCM_CONDITION_CODE_UNDEFINED = 0,
    FCM_CONDITION_CODE_GOOD = 1,
    FCM_CONDITION_CODE_BURST = 2,
    FCM_CONDITION_CODE_RATE = 3,
} FCM_CONDITION_CODE;

typedef enum FCM_PROCESSOR_ENUM
{
    FCM_PROCESSOR_UNDEFINED = 0,
    FCM_PROCESSOR_FE = 1,
    FCM_PROCESSOR_BE = 2,
} FCM_PROCESSOR;


/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void FCM_MonitorTask(TASK_PARMS *parms);
extern void FCM_AddEvent(FCM_PROCESSOR whichEnd, UINT8 port, FCM_EVENT_TYPE type);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _FCM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
