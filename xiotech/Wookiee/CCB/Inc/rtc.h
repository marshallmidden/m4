/* $Id: rtc.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       rtc.h
** MODULE TITLE:    Header file for rtc.c
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _RTC_H_
#define _RTC_H_

#include "ccb_hw.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define HOURS_IN_ONE_DAY        24
#define MINUTES_IN_ONE_HOUR     60
#define SECONDS_IN_ONE_MINUTE   60

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: RTC_GetSystemSeconds
**
**  Comments:   Get the system seconds from the real time clock.
**
**  Parameters: NONE
**
**  Returns:    system seconds value from the cached RTC timestamp.
**
**--------------------------------------------------------------------------*/
extern UINT32 RTC_GetSystemSeconds(void);

/*----------------------------------------------------------------------------
**  Function Name: RTC_NewerTimeStamp
**
**  Comments:   Checks two timestamps and returns the newest.
**
**  Parameters: time1 - first timestamp
**              time2 - second timestamp
**
**  Returns:    pointer to newer timestamp
**
**  Note:       time1 is returned if the timestamps are identical
**
**--------------------------------------------------------------------------*/
extern TIMESTAMP_PTR RTC_NewerTimeStamp(TIMESTAMP_PTR time1, TIMESTAMP_PTR time2);

/*----------------------------------------------------------------------------
**  Function Name: RTC_GetTimeStamp
**
**  Comments:   Get the timestamp information from the real time clock.
**
**  Parameters: timePtr - pointer to timestamp structure
**
**--------------------------------------------------------------------------*/
extern void RTC_GetTimeStamp(TIMESTAMP_PTR timestampPtr);

/*----------------------------------------------------------------------------
**  Function Name: RTC_GetLongTimeStamp
**
**  Comments:   Get a long timestamp from the real time clock.
**
**  Parameters: timePtr - pointer to timestamp structure
**
**  Returns:    time in seconds
**
**--------------------------------------------------------------------------*/
extern UINT32 RTC_GetLongTimeStamp(void);

/*----------------------------------------------------------------------------
**  Function Name: RTC_SetTime
**
**  Comments:   Sets the real time clocks time and date.
**
**  Parameters: timePtr - pointer to timestamp structure
**
**--------------------------------------------------------------------------*/
extern void RTC_SetTime(TIMESTAMP_PTR timePtr);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _RTC_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
