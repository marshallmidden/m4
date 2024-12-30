/* $Id: timer.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file   timer.c
**
**  @brief  Timer control functions
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "timer.h"

#include "ccb_hw.h"
#include "error_handler.h"
#include "idr_structure.h"
#include "i82559.h"
#include "kernel.h"
#include "PI_Utils.h"
#include "SerBuff.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"

#include "debug_files.h"
#include "L_Signal.h"
#include <asm/bitops.h>
#include <sys/time.h>
#include "rtc.h"

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void Timer0_Routine(INT32 sig, UINT32 data);

/*****************************************************************************
** private variables
*****************************************************************************/
static volatile unsigned long timer0Enabled = 0;
static UINT32 timer0TimeStamp = 0;
static UINT32 timer0SkewDiff = 125000;  /* 125ms, 125000usec */

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
volatile UINT32 K_t1cnt = 0;    /* Timer 1 count                            */
UINT32      K_timel = 0;        /* Kernel time counter                      */

/*****************************************************************************
** Code Start
*****************************************************************************/

/*****************************************************************************
** FUNCTION NAME: TimerWait
**
** PARAMETERS:  counts - Number of milliseconds to wait
**
** DESCRIPTION: Function uses Timer1 to count milliseconds
**
** RETURNS:     Nothing, after waiting for 'counts' milliseconds
******************************************************************************/
void TimerWait(UINT32 counts)
{
    usleep(counts * 1000);
}


/*****************************************************************************
** FUNCTION NAME: Timer0_Routine
**
** PARAMETERS:  Nothing
**
** DESCRIPTION: This gets called from the default Timer0 interrupt
**              service routine.  This function is responsible for
**              flashing the heartbeat LED on the front panel.
**
** RETURNS:     Nothing
******************************************************************************/
static void Timer0_Routine(UNUSED INT32 sig, UNUSED UINT32 data)
{
    if (test_bit(0, &timer0Enabled))
    {
        /*
         * Increment the kernel timers.
         */
        K_t1cnt += 6;
        ++K_timel;

        /*
         * Call the kernel to wake any sleeping tasks.
         */
        XK_CheckSleepers();
    }
}


/**
******************************************************************************
**
**  @brief  This function turns on the hardware timer
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
void TimerEnable(void)
{
    struct itimerval timer125;

    timer0TimeStamp = RTC_GetLongTimeStamp();

    /*
     * Add the Linux Signal handler for SIGALRM and have it point to
     * Timer0_Routine.  The setitimer will signal the SIGALRM and call
     * our function.
     */
    if (L_SignalHandlerAdd(SIGALRM, (XKSIGHNDLR)Timer0_Routine, true) == ERROR)
    {
        dprintf(DPRINTF_DEFAULT, "%s: ERROR Setting Timer0_Routine\n",
                    __func__);
    }

    /*
     * Set up the inputs for the itimer.
     * it_value.tv_sec      - secs to wait before sending SIGALRM.
     * it_value.tv_usec     - usecs to wait before sending SIGALRM
     * it_interval.tv_sec   - secs to reset itimer to after we have finished handling it.
     * it_interval.tv_usec  - usecs to reset itimer to after we have finished handling it.
     *
     * We will set itimer to generate SIGALRM every 125ms and
     * then reset itself to 125ms.
     *
     * NOTE: The timer will not be reset until we are done handling it.
     *       this could result in some skew.  Plus Linux gives a max
     *       10ms possible delay before sending the signal to us.
     */
    timer125.it_value.tv_sec = 0;
    timer125.it_value.tv_usec = timer0SkewDiff;
    timer125.it_interval.tv_sec = 0;
    timer125.it_interval.tv_usec = timer0SkewDiff;

    set_bit(0, &timer0Enabled);     /* Set the bit enabling the timer */

    /*
     * Set the itimer.
     */
    if (setitimer(ITIMER_REAL, &timer125, NULL) < 0)
    {
        dprintf(DPRINTF_DEFAULT, "%s: ERROR Setting setitimer\n", __func__);
    }
}


/**
******************************************************************************
**
**  @brief  This function turns off the hardware timer
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
void TimerDisable(void)
{
    struct itimerval timer125;

    /*
     * Don't completely stop the timers when disabled,
     * just remove their interrupt routines.
     */

    clear_bit(0, &timer0Enabled);   /* Clear the bit disabling the timer */

    /*
     * Set up the inputs for the itimer.
     * it_value.tv_sec      - secs to wait before sending SIGALRM.
     * it_value.tv_usec     - usecs to wait before sending SIGALRM
     * it_interval.tv_sec   - secs to reset itimer to after we have finished handling it.
     * it_interval.tv_usec  - usecs to reset itimer to after we have finished handling it.
     *
     * We will set itimer all to 0 to disable it.
     */
    timer125.it_interval.tv_sec = 0;
    timer125.it_interval.tv_usec = 0;
    timer125.it_value.tv_sec = 0;
    timer125.it_value.tv_usec = 0;

    /*
     * Set the itimer.
     */
    if (setitimer(ITIMER_REAL, &timer125, NULL) < 0)
    {
        dprintf(DPRINTF_DEFAULT, "%s: ERROR Setting setitimer\n", __func__);
    }
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
