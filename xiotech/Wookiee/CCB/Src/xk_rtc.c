/* $Id: xk_rtc.c 145032 2010-08-03 17:01:18Z m4 $ */
/**
******************************************************************************
**
**  @file   xk_rtc.c
**
**  @brief  Real time clock routines
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "rtc.h"

#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"
#else   /* LOG_SIMULATOR */
#include "debug_files.h"
#include "timer.h"
#include "convert.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#endif  /* LOG_SIMULATOR */

#include <time.h>

#ifndef LOG_SIMULATOR
#include <sys/time.h>
#endif  /* LOG_SIMULATOR */

/*****************************************************************************
** Private variables
*****************************************************************************/
#ifndef _WIN32
static TIMESTAMP cachedRealTime = { 0, 0, 0, 0, 0, 0, 0, 0 };
static UINT32   cachedTickCounter = 0;
#endif /* !_WIN32 */

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
#ifndef _WIN32
static TIMESTAMP RTC_GetTimeStampDirect(void);
#endif /* !_WIN32 */

/*****************************************************************************
** Function prototypes that are not in other header files.
*****************************************************************************/
extern UINT64 XK_KernelGetTime(void);
extern void XK_KernelInitTime(void);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: RTC_GetTimeStamp
**
**  Comments:   Get the timestamp information from the real time clock.
**
**  Parameters: timePtr - pointer to timestamp structure
**
**  Returns: void
**
**  Note: The 'systemSeconds' parameter in the TIMESTAMP structure is the
**        system 'uptime' seconds, not seconds since the 'epoch'.  To get
**        epoch seconds, call the RTC_GetLongTimeStamp() function.
**
**--------------------------------------------------------------------------*/
void RTC_GetTimeStamp(TIMESTAMP *timestampPtr)
{
#ifdef _WIN32
    struct tm  *tm_pointer;
    long        tmr = 0;
#endif  /* _WIN32 */

    if (timestampPtr != NULL)
    {
#ifdef _WIN32
        tmr = time(NULL);
        tm_pointer = gmtime(&tmr);

        /* Now update the user structure, with old or new cached data */
        timestampPtr->year = ShortToBCD((UINT16)(tm_pointer->tm_year + 1900));
        timestampPtr->month = (UINT8)ShortToBCD((UINT16)(tm_pointer->tm_mon + 1));
        timestampPtr->date = (UINT8)ShortToBCD((UINT16)(tm_pointer->tm_mday));
        timestampPtr->day = (UINT8)ShortToBCD((UINT16)(tm_pointer->tm_wday + 1));
        timestampPtr->hours = (UINT8)ShortToBCD((UINT16)(tm_pointer->tm_hour));
        timestampPtr->minutes = (UINT8)ShortToBCD((UINT16)(tm_pointer->tm_min));
        timestampPtr->seconds = (UINT8)ShortToBCD((UINT16)(tm_pointer->tm_sec));
        timestampPtr->systemSeconds = tmr;
#else   /* _WIN32 */
        /*
         * read no more than once every 375 mS
         */
        if (K_timel >= (cachedTickCounter + 3))
        {
            cachedTickCounter = K_timel;

            cachedRealTime = RTC_GetTimeStampDirect();
        }

        /*
         * Update the user structure with old or new cached data
         */
        *timestampPtr = cachedRealTime;
        timestampPtr->systemSeconds = K_timel >> 3;
#endif /* _WIN32 */
    }
}


/*----------------------------------------------------------------------------
**  Function Name: RTC_GetSystemSeconds
**
**  Comments:   Get the system seconds from the system --
**              which is K_timel based (not seconds since epoch)
**
**  Parameters: NONE
**
**  Returns:    system seconds value taken from the system uptime counter.
**
**--------------------------------------------------------------------------*/
UINT32 RTC_GetSystemSeconds(void)
{
    return K_timel >> 3;
}


/*----------------------------------------------------------------------------
**  Function Name: RTC_GetLongTimeStamp
**
**  Comments:   Get a long timestamp from the real time clock -- seconds
**              since epoch.
**
**  Parameters: timePtr - pointer to timestamp structure
**
**  Returns:    time in seconds
**
**--------------------------------------------------------------------------*/
UINT32 RTC_GetLongTimeStamp(void)
{
#ifdef LOG_SIMULATOR
    return time(NULL);
#else   /* LOG_SIMULATOR */
    return (UINT32)XK_KernelGetTime();
#endif  /* LOG_SIMULATOR */
}

#ifndef LOG_SIMULATOR

/*----------------------------------------------------------------------------
**  Function Name: RTC_SetTime
**
**  Comments:   Sets the real time clocks time and date.
**
**  Parameters: timePtr - pointer to timestamp structure
**
**  Returns:    void
**
**  Note:       Need to run as ROOT to set the time.
**
**--------------------------------------------------------------------------*/
void RTC_SetTime(TIMESTAMP *pTS)
{
    struct tm   tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL };
    time_t      time_seconds = 0;
    struct timeval tv = { 0, 0 };

    struct tm   gtm;
    time_t      gt;
    UINT32      diff;

    if (pTS)
    {
        /*
         * Convert the input from BCD format to binary - 'struct tm' format.
         */
        tm.tm_year = BCD2Binary(pTS->year) - 1900;
        tm.tm_mon = BCD2Binary(pTS->month) - 1;
        tm.tm_mday = BCD2Binary(pTS->date);
        tm.tm_hour = BCD2Binary(pTS->hours);
        tm.tm_min = BCD2Binary(pTS->minutes);
        tm.tm_sec = BCD2Binary(pTS->seconds);

        /*
         * Convert the struct tm format to seconds epoch
         */
        time_seconds = mktime(&tm);

        /*
         * Warning: Extremely convoluted kludge follows...
         *
         * The problem we are fixing here is that mktime() takes in a struct
         * tm in GMT, but makes a conversion to a time_t (seconds) assuming
         * its input is in local time.  In some OS's there is a complementary
         * function to mktime() that will convert assuming GMT, but not in the
         * version of Linux we are using, so we have to adjust for it
         * ourselves.  So below, we pass the same 'time (seconds)' to gmtime()
         * that we got out of mktime(), then run it back through mktime().
         * From this we can determine how many seconds different localtime is
         * from GMT and adjust the main 'time' parameter accordingly.  The
         * nice thing about this method is that junk like which timezone and
         * daylight savings time does not matter, it is all factored out (or
         * subtracted out in this case :-).
         *
         * Pretty kool, er...ugly, huh?
         */
        gtm = *gmtime(&time_seconds);
        gt = mktime(&gtm);
        diff = gt - time_seconds;

        time_seconds -= diff;

        dprintf(DPRINTF_DEFAULT, "RTC_SetTime: time %u sec\n", (UINT32)time_seconds);


        if (time_seconds != 0 && time_seconds != (int)0xFFFFFFFF)
        {
            /*
             * Now set the system clock to this value
             */
            tv.tv_sec = time_seconds;

            /*
             * Set the system time.
             */
            settimeofday(&tv, NULL);

            /*
             * Sync the HW clock.
             */
            XK_System("/sbin/hwclock --systohc --utc");

            /*
             * Reset the kernel time.
             */
            XK_KernelInitTime();

            /*
             * Update the cached timestamp value
             */
            cachedRealTime = RTC_GetTimeStampDirect();
        }
    }
}
#endif /* LOG_SIMULATOR */


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
TIMESTAMP *RTC_NewerTimeStamp(TIMESTAMP *time1, TIMESTAMP *time2)
{
    /*
     * Compare the two timestamps and return the newest time.
     * First compare the years
     */
    if (time1->year != time2->year)
    {
        if (time1->year > time2->year)
        {
            return (time1);
        }
        else
        {
            return (time2);
        }
    }

    /*
     * Next compare the month
     */
    if (time1->month != time2->month)
    {
        if (time1->month > time2->month)
        {
            return (time1);
        }
        else
        {
            return (time2);
        }
    }

    /*
     * Next compare the day of the month
     */
    if (time1->date != time2->date)
    {
        if (time1->date > time2->date)
        {
            return (time1);
        }
        else
        {
            return (time2);
        }
    }

    /*
     * Next compare the hour
     */
    if (time1->hours != time2->hours)
    {
        if (time1->hours > time2->hours)
        {
            return (time1);
        }
        else
        {
            return (time2);
        }
    }

    /*
     * Next compare the minutes
     */
    if (time1->minutes != time2->minutes)
    {
        if (time1->minutes > time2->minutes)
        {
            return (time1);
        }
        else
        {
            return (time2);
        }
    }

    /*
     * Next compare the seconds
     */
    if (time1->seconds != time2->seconds)
    {
        if (time1->seconds > time2->seconds)
        {
            return (time1);
        }
        else
        {
            return (time2);
        }
    }

    /*
     * The times are the same, return time1 for now
     */
    return (time1);
}


#ifndef _WIN32
/*----------------------------------------------------------------------------
**  Function Name: RTC_GetTimeStampDirect
**
**  Comments:   Get the timestamp information directly from the real time
**              clock, bypassing the cached value.
**
**  Parameters: void
**
**  Returns:    returns a TIMESTAMP structure
**
**--------------------------------------------------------------------------*/
static TIMESTAMP RTC_GetTimeStampDirect(void)
{
    struct tm   tstamp;
    time_t      sysSecs;
    TIMESTAMP   ts = { 0, 0, 0, 0, 0, 0, 0, 0 };

    /*
     * Go query the system clock
     */
#ifdef LOG_SIMULATOR
    sysSecs = time(NULL);
#else   /* LOG_SIMULATOR */
    sysSecs = (UINT32)XK_KernelGetTime();
#endif  /* LOG_SIMULATOR */
    tstamp = *gmtime(&sysSecs);

    /*
     * Update the output structure
     */
    ts.year = ShortToBCD((UINT16)(tstamp.tm_year + 1900));
    ts.month = (UINT8)ShortToBCD((UINT16)(tstamp.tm_mon + 1));
    ts.date = (UINT8)ShortToBCD((UINT16)(tstamp.tm_mday));
    ts.day = (UINT8)ShortToBCD((UINT16)(tstamp.tm_wday + 1));
    ts.hours = (UINT8)ShortToBCD((UINT16)(tstamp.tm_hour));
    ts.minutes = (UINT8)ShortToBCD((UINT16)(tstamp.tm_min));
    ts.seconds = (UINT8)ShortToBCD((UINT16)(tstamp.tm_sec));

    return ts;
}
#endif /* !_WIN32 */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
