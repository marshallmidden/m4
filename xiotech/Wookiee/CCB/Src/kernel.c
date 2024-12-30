/* $Id: kernel.c 130553 2010-03-04 17:33:12Z mdr $ */
/**
******************************************************************************
**
**  @file   kernel.c
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <sys/time.h>
#include <errno.h>
#include "kernel.h"
#include "debug_files.h"
#include "trace.h"

/*
******************************************************************************
** Code Start
******************************************************************************
*/
void K$TraceEvent(unsigned int id, unsigned int data)
{
    TRACE_EVENT *pTraceEvent;
    struct timeval tv;

    /*
     * If the run flag is clear, just exit.
     */
    if (evQueue.evRunFlag == 0)
    {
        return;
    }

    /*
     * Get the current time.
     */
    if (gettimeofday(&tv, NULL) != 0)
    {
        tv.tv_sec = 0xffffffff;
        tv.tv_usec = errno;
    }

    /*
     * Store the ID, data, and a time stamp.
     */
    pTraceEvent = evQueue.evNextP++;
    if (evQueue.evNextP == evQueue.evEndP)
    {
        evQueue.evNextP = evQueue.evBaseP;
    }

    pTraceEvent->id = id;
    pTraceEvent->data = data;
    pTraceEvent->tCoarse = tv.tv_sec;
    pTraceEvent->tFine = tv.tv_usec;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
