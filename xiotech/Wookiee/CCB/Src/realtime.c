/* $Id: realtime.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file   realtime.c
**
**  @brief  Real time clock function implementation
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "realtime.h"

#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"
#else   /* LOG_SIMULATOR */
#include "ccb_hw.h"
#include "convert.h"
#include "kernel.h"
#include "logging.h"
#include "rtc.h"
#include "time.h"
#include "XIO_Const.h"

/*****************************************************************************
** Private variables
*****************************************************************************/
static LAST_ACCESS lastAccessFromXMC =
{
    0,                          /* systemSeconds                        */
    FALSE                       /* lastAccessValid                      */
};
#endif  /* LOG_SIMULATOR */

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: ConvertFromMilitaryTime
**
**  Comments:   Converts a BCD military hours to standard 12 hour time (am/pm)
**
**  Parameters:  hours  -  2-digit BCD number
**  Returns:     UINT8  - hours in 12 hour format
**
**--------------------------------------------------------------------------*/
UINT8 ConvertFromMilitaryTime(UINT8 hours)
{
    /*
     * Convert 23 hours to 11 pm
     *         22 hours to 10 pm
     */
    if (hours >= 0x22)
    {
        hours -= 0x12;
    }
    /*
     * Convert 21 hours to 9 pm
     *         20 hours to 8 pm
     */
    else if (hours >= 0x20)
    {
        hours -= 0x18;
    }
    /*
     * Convert (19 hours - 13 hours) to (7pm - 1pm)
     */
    else if (hours > 0x12)
    {
        hours -= 0x12;
    }
    /*
     * Convert midnight to 12 am
     */
    else if (hours == 0x00)
    {
        hours = 0x12;
    }

    return (hours);
}

#ifndef LOG_SIMULATOR

/*----------------------------------------------------------------------------
**  Function Name: SetClockFromSysTime
**
**  Comments:   Sets the real time clock from the system time.
**
**  Parameters: sysTime - system seconds
**
**--------------------------------------------------------------------------*/
void SetClockFromSysTime(const time_t sysTime)
{
    struct tm   tm;
    TIMESTAMP   ts;

    /*
     * Convert system seconds to a time structure (decimal)
     */
    tm = *gmtime(&sysTime);

    /*
     * Convert time structure (decimal) into a timestamp structure (BCD)
     * Year is 1900 based, month is 0 based, and day of week is 0 based.
     * Convert to match up with RTC.
     */
    ts.year = ShortToBCD((UINT16)tm.tm_year + 1900);
    ts.month = (UINT8)ShortToBCD((UINT16)tm.tm_mon + 1);
    ts.date = (UINT8)ShortToBCD((UINT16)tm.tm_mday);
    ts.day = (UINT8)ShortToBCD((UINT16)tm.tm_wday + 1);
    ts.hours = (UINT8)ShortToBCD((UINT16)tm.tm_hour);
    ts.minutes = (UINT8)ShortToBCD((UINT16)tm.tm_min);
    ts.seconds = (UINT8)ShortToBCD((UINT16)tm.tm_sec);

    /*
     * Set the clock from the generated timestamp structure
     */
    RTC_SetTime(&ts);
}


/*----------------------------------------------------------------------------
**  Function Name: RefreshLastAccess
**
**  Comments:   Refreshes the last access time.  This is the time that
**              the controller was contacted by the CCBE or XMC.
**
**  Parameters: NONE
**
**  Returns:    NONE
**
**--------------------------------------------------------------------------*/
void RefreshLastAccess(void)
{
    lastAccessFromXMC.systemSeconds = RTC_GetSystemSeconds();
    lastAccessFromXMC.lastAccessValid = TRUE;
}


/*----------------------------------------------------------------------------
**  Function Name: TimeSinceLastAccess
**
**  Comments:   Retrieve the time since the last access to this controller
**              from the CCBE or XMC.
**
**  Parameters: NONE
**
**  Returns:    system seconds since last access by the CCBE or XMC.
**
**--------------------------------------------------------------------------*/
UINT32 TimeSinceLastAccess(void)
{
    UINT32      currentSystemSeconds = RTC_GetSystemSeconds();
    UINT32      systemSecondsSinceLastAccess = 0xFFFFFFFF;

    if (lastAccessFromXMC.lastAccessValid == TRUE)
    {
        /*
         * Check for a wrap condition, where systemSeconds has wrapped, but
         * lastAccessFromXMC.systemSeconds has not.
         */
        if (currentSystemSeconds >= lastAccessFromXMC.systemSeconds)
        {
            systemSecondsSinceLastAccess = currentSystemSeconds - lastAccessFromXMC.systemSeconds;
        }
        else
        {
            /*
             * systemSeconds has wrapped - compensate
             */
            systemSecondsSinceLastAccess = (0xFFFFFFFF - lastAccessFromXMC.systemSeconds + currentSystemSeconds + 1);
        }
    }

    return (systemSecondsSinceLastAccess);
}
#endif /* LOG_SIMULATOR */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
