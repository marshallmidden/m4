/* $Id: L_Signal.c 144671 2010-07-26 16:04:12Z m4 $ */
/**
******************************************************************************
**
**  @file       L_signal.c
**
**  @brief      This module will handle signal handling.
**
**  Copyright (c) 2004-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
/* XIOtech includes */
#include "L_Signal.h"

#include "XIO_Const.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#ifndef PAM
#include "XIO_Std.h"
#else    /* PAM */
extern const char *L_MsgPrefix;
#endif  /* PAM */

/* Linux includes */
#include <signal.h>
#include <stdio.h>

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/
typedef struct _L_SIGNAL_ACTION
{
    XKSIGHNDLR          handler;
    struct sigaction    curAction;
    struct sigaction    oldAction;
} L_SIGNAL_ACTION;

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static L_SIGNAL_ACTION xkSignal[L_SIGNAL_MAX_SIGNALS] /* = { {{0}} } */;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void L_SignalHandler(int sig, siginfo_t *sigInfo, UNUSED void *ucontext);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Add a Linux Signal handler.
**
**              Add a Linux Signal handler.
**
**  @param      signal  - Signal number.
**  @param      func    - Function to call upon signal.
**  @param      mask    - true = mask off while handling.
**
**  @return     GOOD    - on success
**  @return     ERROR   - on failure
**
******************************************************************************
**/
INT32 L_SignalHandlerAdd(INT32 sigNo, XKSIGHNDLR func, bool mask)
{
    struct sigaction *action;
    struct sigaction *oldAction = NULL;

    /* Make sure the signal is in the valid range of signals. */
    if (sigNo > L_SIGNAL_MAX_SIGNALS)
    {
        fprintf(stderr, "%sL_SignalHandlerAdd - Signal %d out of range\n", L_MsgPrefix, sigNo);
        return ERROR;
    }

    /* Set the new action up. */
    action = &xkSignal[sigNo].curAction;

    /* We do not need any special flags. And set the Signal Handler. */
    action->sa_flags = SA_SIGINFO;

    /* Set the internal Signal Handler. */
    action->sa_sigaction = L_SignalHandler;

    /* Fill the empty set of signals to disable while we are handling this one. */
    if ( sigemptyset(&action->sa_mask) < 0 )
    {
        fprintf(stderr, "%sL_SignalHandlerAdd - Signal %d sigemptyset FAILED\n", L_MsgPrefix, sigNo);
        return ERROR;
    }

    /* If we want to mask this signal to disable during handling, do it here. */
    if (mask)
    {
        if ( sigaddset(&action->sa_mask, sigNo) < 0 )
        {
            fprintf(stderr, "%sL_SignalHandlerAdd - Signal %d sigaddset FAILED\n", L_MsgPrefix, sigNo);
            return ERROR;
        }
    }

    /* Add the function to the table. */
    /*
     * We want to take a snapshot of the original signal handler.
     * This way, if we want to disable the signal later, we have
     * a way to revert back to the original handler.  So here we
     * check if the external handler function is NULL.  If it is,
     * the original Linux function handler is in tact.  So we
     * will save it here.  If we already have an external function
     * handler, that means that we have already saved the old
     * Linux function handler, and we don't want to overwrite it
     * with our internal handler.
     */
    if (xkSignal[sigNo].handler == NULL)
    {
        oldAction = &xkSignal[sigNo].oldAction;
    }

    /* Make it so.  Set the internal signal handler. If it fails, exit. */
    if ( sigaction(sigNo, action, oldAction) < 0)
    {
        fprintf(stderr, "%sL_SignalHandlerAdd - Signal %d sigaction FAILED\n", L_MsgPrefix, sigNo);
        return ERROR;
    }
    xkSignal[sigNo].handler = func;

    /* Return the results to the caller. */
    return GOOD;
}

/**
******************************************************************************
**
**  @brief      Signal a Linux Process
**
**  @param      pid     - Process ID to send signal
**              signal  - Signal number.
**              data    - 4 bytes of free data. Use wisely.
**
**  @return     GOOD    - on success
**  @return     ERROR   - on failure
**
******************************************************************************
**/
INT32 L_SignalProcess(INT32 pid, INT32 sigNo, UINT32 sigData )
{
    INT32 rc;

    /* Make sure the signal is in the valid range of signals. */
    if (sigNo >= L_SIGNAL_MAX_SIGNALS)
    {
        fprintf(stderr, "%sL_SignalProcess - Signal %d out of range\n", L_MsgPrefix, sigNo);
        return ERROR;
    }

    /* Send the signal. */
    rc = sigqueue(pid, sigNo, ((union sigval)((void*)sigData)));

    /* Return the results to the caller. */
    return rc;
}

/*
******************************************************************************
**              PRIVATE FUNCTIONS (PROTOTYPES ABOVE)                        **
******************************************************************************
*/
#ifdef HISTORY_KEEP
extern unsigned int CT_NOCHECK_STACK;
#endif  /* HISTORY_KEEP */

/**
******************************************************************************
**
**  @brief      Signal Handler
**
**              Internal signal handler that gets called when we need to
**              handle a signal.  This internal signal handler will call the
**              external signal handler set up by L_SignalHandlerAdd().
**
**  @param      sig         - signal number.
**  @param      sigInfo     -
**  @param      ucontext    - unused.
**
**  @return     none
**
******************************************************************************
**/
static void L_SignalHandler(int sig, siginfo_t* sigInfo, UNUSED void* ucontext)
{
    UINT32  ebp;

#if defined(HISTORY_KEEP) && defined(CT2_DEBUG)
    CT_NOCHECK_STACK = 2;                   /* Do not re-check stack status. */
#endif  /* HISTORY_KEEP && CT2_DEBUG */

    asm("movl %%ebp, %0" : "=r" (ebp) : );  /* Get ebp value */

    /* Make sure the signal is in the valid range of signals. */
    if (sig >= L_SIGNAL_MAX_SIGNALS)
    {
        fprintf(stderr, "%sL_SignalHandler - Signal %d out of range\n", L_MsgPrefix, sig);
        return;
    }

    /* Make sure that we already have an external signal handler. */
    if (!xkSignal[sig].handler)
    {
        fprintf(stderr, "%sL_SignalHandler - Unhandled Signal %d\n", L_MsgPrefix, sig);
        return;
    }

    /* Call the external signal handler. */
    if ( sig == SIGINT )
    {
        fprintf(stderr, "%sL_SignalHandler - received signal %d from pid %d\n",
                L_MsgPrefix, sig, sigInfo->si_pid);
    }
    xkSignal[sig].handler(sig, (UINT32)sigInfo->si_value.sival_int, sigInfo, ebp);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
