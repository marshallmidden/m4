/* $Id: realtime.h 130553 2010-03-04 17:33:12Z mdr $ */
/**
******************************************************************************
**
**  @file   realtime.h
**
**  @brief  Header file for realtime.c
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _REALTIME_H_
#define _REALTIME_H_

#include "XIO_Types.h"
#include <time.h>

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
typedef struct LAST_ACCESS_STRUCT
{
    UINT32      systemSeconds;  /* System seconds at access time         */
    UINT8       lastAccessValid;        /* Tells if we've been accessed at all   */
} LAST_ACCESS;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: ConvertFromMilitaryTime
**
**  Comments:   Converts a BCD military hours to standard 12 hour time (am/pm)
**
**  Parameters:  hours  -  2-digit BCD number
**
**  Returns:     UINT8  - hours in 12 hour format
**
**--------------------------------------------------------------------------*/
extern UINT8 ConvertFromMilitaryTime(UINT8 hours);

/*----------------------------------------------------------------------------
**  Function Name: SetClockFromSysTime
**
**  Comments:   Sets the real time clock from the system time.
**
**  Parameters: sysTime - system seconds
**
**--------------------------------------------------------------------------*/
extern void SetClockFromSysTime(const time_t sysTime);

/*----------------------------------------------------------------------------
**  Function Name: RefreshLastAccess
**
**  Comments:   Refreshes the last access time.  This is the time that
**              the controller was contacted by the CCBE or XMC.
**
**  Parameters: NONE
**
**--------------------------------------------------------------------------*/
extern void RefreshLastAccess(void);

/*----------------------------------------------------------------------------
**  Function Name: TimeSinceLastAccess
**
**  Comments:   Retrieve the time since the last access to this controller
**              from the CCBE or XMC.
**
**  Returns:    system seconds since last access by the CCBE or XMC.
**
**--------------------------------------------------------------------------*/
extern UINT32 TimeSinceLastAccess(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _REALTIME_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
