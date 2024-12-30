/* $Id: li_evt.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**  @file       li_evt.c
**
**  @brief      Linux Interface for event operations
**
**  Linux interface code for event operations. Events include interrupt
**  notification as well as inter-process events.
**
** Copyright 2004-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdint.h>
#include <li_evt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <LKM_layout.h>
#include "xio3d.h"

extern int  CT_xiofd;       /* xio3d file descriptor */
#ifndef GCOV
#define CT_ioctl ioctl
#else
extern int CT_ioctl(int, unsigned long, unsigned int);
#endif

/*
******************************************************************************
** Private structures
******************************************************************************
*/
struct evt
{
    void   (*func)(UINT32);
    UINT32 value;
};


/*
******************************************************************************
** Private variables
******************************************************************************
*/
static struct evt   events[32];


#ifndef CCB_RUNTIME_CODE
/*
******************************************************************************
** LI_SchedIRQ
**
**  @brief  Process events
**
**  This function processes all delivered IRQs.
**
******************************************************************************
**/

void LI_SchedIRQ(unsigned long active)
{
    int i;

    while ((i = ffs((signed)active)) != 0)
    {
        if (i > XIO3D_MAX_IRQS)
        {
            return;
        }
        i -= 1;
        active &= ~(1 << i);    /* Clear bit for this event */
        if (events[i].func)
        {
            events[i].func(events[i].value);
        }
    }
}
#endif  /* !CCB_RUNTIME_CODE */


/*
******************************************************************************
** LI_RegisterEvent
**
**  @brief  Register for an event
**
**  This function registers a function for an event.
**
******************************************************************************
**/

void    LI_RegisterEvent(int evt, void (*f)(UINT32), UINT32 val)
{
    int         err;

    if (evt < 0 || evt > 31)
    {
        fprintf(stderr, "LI_RegisterEvent: evt out of range, evt=%d\n", evt);
        return;
    }
    if (events[evt].func != 0)
    {
        fprintf(stderr, "LI_RegisterEvent: event %d previously registered\n",
                evt);
    }
    fprintf(stderr, "LI_RegisterEvent: event %d registered with %p, value=0x%08X\n",
            evt, f, val);
    events[evt].value = val;
    events[evt].func = f;

    /*
    ** If this is an event (as opposed to an IRQ), generate the call to
    ** register this event with Linux.
    */

    if (evt >= XIO3D_EVT_BASE)
    {
        err = CT_ioctl(CT_xiofd, XIO3D_REGEVT, evt);
        if (err != 0)
        {
            fprintf (stderr, "Failed to register event %d\n", evt);
            perror ("Event registration failed");
        }
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
