/* $Id: rip.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       rip.h
**
**  @brief      Raid Initialization Packet
**
**  Copyright (c) 1996-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _RIP_H_
#define _RIP_H_

#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name   General definitions
**/
/*@{*/
#define RIP_MAX_ACT     8       /**< Maximum number of active RAID
                                 **  inits allowed at a time                */
/*@}*/

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** Raid Initialization Packet data structure
**/
typedef struct RIP
{
    UINT32  link;       /**< Link to next RIP                               */
    UINT32  rdd;        /**< RDD of Raid to get initialized                 */
    UINT32  psd;        /**< PSD to get initialized                         */
    UINT32  rid;        /**< Raid ID                                        */
} RIP;

#endif /* _RIP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
