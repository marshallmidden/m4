/* $Id: ficb.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       ficb.h
**
**  @brief      Firmaware Initialization Control Block descriptors
**
**  To provide a common means of defining the FICB.
**
**  Copyright (c) 1996-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _FICB_H_
#define _FICB_H_

#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

typedef struct
{
    UINT32      vcgID;              /* Virtual controller group ID          */
    UINT32      cSerial;            /* Controller serial number             */
    UINT32      seq;                /* Sequence number                      */
    UINT32      ccbIPAddr;          /* CCB IP address                       */
                                    /* QUAD BOUNDARY                    *****/
    UINT32      mirrorPartner;      /* Mirroring partner for resync         */
    UINT8       rsvd20[12];         /* Reserved                             */
                                    /* QUAD BOUNDARY                    *****/
    UINT8       vcgName[16];        /* VCG name                             */
                                    /* QUAD BOUNDARY                    *****/
} FICB;

#endif /* _FICB_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
