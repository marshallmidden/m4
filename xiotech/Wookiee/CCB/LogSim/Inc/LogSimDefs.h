/* $Id: LogSimDefs.h 137382 2010-04-19 16:02:18Z mdr $ */
/**
******************************************************************************
**
**  @file   LogSimDefs.h
**
**  @brief  Log Simulator definitions
**
**  Definitions for the log simulator.
**
**  Copyright (c) 2001-2003, 2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef LOGSIM_DEFS_H
#define LOGSIM_DEFS_H

#include <stdlib.h>

#include "XIO_Types.h"
#include "XIO_Std.h"

/*****************************************************************************
** Public defines
*****************************************************************************/

#define K$xchang()

#ifdef  MallocWC
#undef  MallocWC
#endif

#ifdef  Free
#undef  Free
#endif
#define Free            free    

#ifdef  TaskSleepMS
#undef  TaskSleepMS
#endif
#define TaskSleepMS(a)  _sleep(a)

#define SIGUSR1     10
#define __SIGRTMIN  32
#ifdef _WIN32
#define snprintf    _snprintf
#endif  /* _WIN32 */

#ifdef  DebugPrintf
#undef  DebugPrintf
#endif
#define DebugPrintf     printf

typedef void            *ttUserInterface;
typedef void            *ttUserBuffer;
typedef ttUserBuffer    *ttUserBufferPtr;

#define K_timel         0

/*****************************************************************************
** Public variables
*****************************************************************************/

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

#endif /* LOGSIM_DEFS_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
