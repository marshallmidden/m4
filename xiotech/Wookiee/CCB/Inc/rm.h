/* $Id: rm.h 159549 2012-07-27 15:13:43Z marshall_midden $ */
/*============================================================================
**  File Name:              rm.h
**  Module Title:           Resource Manager header file
**
**  Description:            RM structure definitions
**
**  Copyright (c) 2002-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _RM_H_
#define _RM_H_

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define RMWAITTIME          1000        /* 1s timer interval */

/*****************************************************************************
** Public data structures
*****************************************************************************/

typedef enum rmopstates_t       /* RM operational states */
{
/* RMNONE, RMINIT and RMSHUTDOWN are used for RMInit actions,
   the rest are used to define states the RM takes while performing actions */
    RMNONE,                     /* 0 - Not initialized */
    RMINIT,                     /* 1 - RM Initialize command/ RM is initializing */
    RMSHUTDOWN,                 /* 2 - RM Shutdown command/ RM is shutting down */
    RMRUNNING,                  /* 3 - RM awaiting command, idle */
    RMBUSY,                     /* 4 - RM busy with a command */
    RMDOWN,                     /* 5 - RM down */
} RMOpState;

typedef enum rmretcodes_t       /* RM codes returned by Failure Manager/User Interface calls */
{
    RMOK,                       /* Normal return */
    RMERROR,                    /* Generic error */
    RMTIMEOUT,                  /* RM timed out on command */
    RMCONTROLLER_NOT_FOUND,     /* no controller found for RM operation */
    RMINTERFACE_NOT_FOUND,      /* no interface found for RM operation */
    RMTARGET_NOT_FOUND,         /* no target found for RM operation */
    RMDESTINATION_NOT_FOUND,    /* Destination not found */
    RMNOTREADY,                 /* RM not ready/initialized */
} RMErrCode;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#include "RM_headers.h"


#endif /* _RM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
