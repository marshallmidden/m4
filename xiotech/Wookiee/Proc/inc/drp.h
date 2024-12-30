/* $Id: drp.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       drp.h
**
**  @brief      DLM Request Packet
**
**    To provide a common means of defining the DLM Request
**    Packet (DRP) definitions.
**
**  Copyright (c) 2001-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _DRP_H_
#define _DRP_H_

#include "datagram.h"                   /* Datagram definitions             */
#include "sgl.h"                        /* Scatter/Gather List definitions  */
#include "XIO_Types.h"                  /* Xiotech Standard Type definitions*/

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/*
** Maximum number of SGLs allowed in a DRP
**
** NOTE: The minimum for number of reads or writes is 2 - assumptions in the
**  code require this
*/
#define DR_MAX_SGL_READ     2           /* Max of 2 Read SGLs allowed in DRP */
#define DR_MAX_SGL_WRITE    2           /* Max of 2 Write SGLs in a DRP     */
#define DR_MAX_SGLS         4           /* Max number of SGLs allowed in DRP */

/*
** Normal DRP function code definitions ----------------------------
*/
#define DR_CCB_TO_DLM       0x0700          /* CCB to DLM Datagram          */
#define DR_DLM_TO_CCB       0x0701          /* DLM to CCB Datagram          */
#define DR_CACHE_TO_DLM     0x0710          /* Cache to DLM Datagram        */
#define DR_DLM_TO_CACHE     0x0711          /* DLM to Cache Datagram        */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define DLM_REQ_SIZE    sizeof(struct DLM_REQ)

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

typedef struct DLM_REQ
{
    UINT16      func;                   /* Function                         */
    UINT8       rsvd1;                  /* reserved 1 (used by Link 960)    */
    UINT8       status;                 /* Status                           */
    struct SGL* sglPtr;                 /* SGL for discontiguous data       */
    UINT8       timeout;                /* DRP timeout (in seconds)         */
    UINT8       issueCnt;               /* DRP Issue count                  */
                                        /*  (1 = do not retry on error)     */
    UINT16      rsvd2;                  /* reserved 2                       */
    UINT32      pPtr;                   /* Packet physical address          */
    UINT32      rspLength;              /* Response length (hdr & extended) */
    struct DATAGRAM_RSP * rspAddress;   /* Response address                 */
    struct DATAGRAM_REQ * reqAddress;   /* Request address                  */
    UINT32      reqLength;              /* Request len (hdr & ext)          */
} DLM_REQ;

#endif /* _DRP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
