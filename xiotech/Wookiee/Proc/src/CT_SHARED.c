/* $Id: CT_SHARED.c 144139 2010-07-14 19:46:01Z m4 $ */
/**
******************************************************************************
**
**  @file       CT_SHARED.c
**
**  @brief      Additional 'c' code for x86 system with code translation.
**
**  The signal handler for timer interrupt is coded here.
**
**  Copyright (c) 2004-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "CT_defines.h"
#include "L_Misc.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "LKM_layout.h"
#include "system.h"
#include "pcb.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/

#ifdef HISTORY_KEEP
unsigned int CT_NO_HISTORY = 0;
extern int CHECK_LOCAL_MEMORY_first_case;
#endif /* HISTORY_KEEP */
#if defined(HISTORY_KEEP) || defined(RREG_PATTERN_CHECK)
extern int CHECK_RREG_PATTERN_first_case;
#endif /* HISTORY_KEEP || RREG_PATTERN_CHECK */
#ifdef CT2_DEBUG
unsigned int CT_NOCHECK_STACK = 0;
#endif  /* CT2_DEBUG */

char    int_ct_stack[64*16 + 512];

ulong   errGreg[16];
ulong   errTrapAddr = SHARELOC + 0x10000;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

void SIGNAL_HUP(int);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      k$tint - the handler for the Linux timer signal().
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/

#ifdef PROC_CODE
/* ------------------------------------------------------------------------ */
extern UINT32       K_rrstate;
extern UINT32       K_rrtimer;
extern UINT32       K_time;
extern PCB         *K_tmrexec_pcb;

#define rr_idle     0
#define rr_start    1
#define rr_running  2

/* ------------------------------------------------------------------------ */
volatile ulong sigAlarmEntered = 0;
void SIGNAL_ALRM(int sig UNUSED)
{

    if (K_tmrexec_pcb == NULL)
    {
        return;
    }
    if (sigAlarmEntered++ == 0 )
    {
#ifdef HISTORY_KEEP
        UINT32 save_hist_keep_flag = CT_NO_HISTORY;
        CT_NO_HISTORY = 1;              /* Turn off history during timer. */
#endif /* HISTORY_KEEP */
        /* Switch to different i960 stack during signal handling */

        K_ii.time++;
        if (K_rrstate == rr_idle) {             /* If we are idle. */
            if (K_rrtimer >=  K_ii.time) {      /* If we are in the next window. */
                K_rrstate = rr_start;           /* Set round robin scheduling. */
            }
        }
        K_time += QUANTUM;                      /* Adjust executive time period. */
        TaskSetState(K_tmrexec_pcb, PCB_READY); /* Set timer task ready to execute. */
        kernel_up_counter++;
        if (kernel_sleep) {
            CT_ioctl(CT_xiofd, XIO3D_SENDEVT, LLMyEvent);
        }
#ifdef HISTORY_KEEP
        CT_NO_HISTORY = save_hist_keep_flag;    /* Put flag back. */
#endif /* HISTORY_KEEP */
    }
    else
    {
        fprintf(stderr, "%s+-+-+-+- SIGALRM already in progress Returning -+-+-+-+\n", L_MsgPrefix);
    }
    sigAlarmEntered--;
}

/* ------------------------------------------------------------------------ */
extern void  errorLinux(void);
extern ulong e_etrapped;
UINT32  trap_ebp;
siginfo_t   *trap_sinf;
ulong   errLinuxSignal = 0;
ulong   sigUsr1Stack;

void SIGNAL_ERRTRAP(int sig, UINT32 data, siginfo_t *sinfo, UINT32 ebp)
{
    struct  itimerval stopTimer;
    struct  itimerval currentTimer;

#ifdef HISTORY_KEEP
    CT_NO_HISTORY = 1;               /* stop taking history log */
#endif /* HISTORY_KEEP */
#if defined(HISTORY_KEEP) || defined(RREG_PATTERN_CHECK)
    CHECK_RREG_PATTERN_first_case = 1;  /* Do not abort() on R register problem. */
#endif /* HISTORY_KEEP || RREG_PATTERN_CHECK */

    /*
    ** Set the values to stop the timer.
    */
    stopTimer.it_interval.tv_sec = 0;
    stopTimer.it_interval.tv_usec = 0;
    stopTimer.it_value.tv_sec = 0;
    stopTimer.it_value.tv_usec = 0;

    if ( e_etrapped++ == 0 )
    {
        trap_ebp = ebp;
        trap_sinf = sinfo;

        /*
        ** Stop the timer.
        */
        setitimer(ITIMER_REAL, &stopTimer, &currentTimer);

        /* Ignore all signals */
        signal(XIO_PLATFORM_SIGNAL, SIG_IGN);

        /* Set the global sig handler */
        errLinuxSignal = sig;

        /* save the registers */
        memcpy((byte*)errGreg, (byte*)greg, sizeof(errGreg));

        fprintf (stderr, "%s+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n",L_MsgPrefix);
        fprintf (stderr, "%s+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n",L_MsgPrefix);
        fprintf (stderr, "%s+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n",L_MsgPrefix);
        fprintf(stderr, "%sCaught Linux Error Trap ( %s )\n",  L_MsgPrefix,
                L_LinuxSignalToString(sig, data));

        /* Switch to different i960 stack during signal handling */
        sigUsr1Stack = fp;
        fp = (ulong) &int_ct_stack[0];
        sp = (ulong)fp + 64;
        errorLinux();       /* Linux generated error trap */
        fp = (ulong)sigUsr1Stack;
    }
}

/* ------------------------------------------------------------------------ */
NORETURN
void SIGNAL_HUP(int sig UNUSED)
{
    fprintf(stderr, "%sreceived HUP (kill -1) signal, shutting down.\n", L_MsgPrefix);
    exit(0);
}
#endif /* PROC_CODE */

/* ------------------------------------------------------------------------ */
#ifdef GCOV
int CT_read(int d, void *b, unsigned int s)
{
    return(read(d,b,s));
}

/* ------------------------------------------------------------------------ */
int CT_write(int d, void *b, unsigned int s)
{
    return(write(d,b,s));
}

/* ------------------------------------------------------------------------ */
int CT_ioctl(int d, unsigned long r, unsigned int v)
{
    return(ioctl(d,r,v));
}
#endif  /* GCOV */

/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
