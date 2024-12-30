/* $Id: rbr.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       rbr.h
**
**  @brief      Rebuild Records descriptors
**
**  To provide a common means of defining the Rebuild Record Description
**  (RBR) structure.
**
**  Copyright (c) 2003-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _RBR_H_
#define _RBR_H_

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/*
**  status field definitions
*/
#define RB_NORM             0       /* Normal (either waiting or rebuilding)*/
#define RB_DELAY            1       /* Rebuild delayed                      */
#define RB_ABORT            2       /* Stop using this RBR                  */

/*
** RBR status definitions
*/
#define     RBR_ST_NORMAL   0
#define     RBR_ST_DELAY    1
#define     RBR_ST_CANCEL   2

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

typedef struct RBR
{
    struct PSD *psd;                /* PSD pointer                          */
    UINT16      rid;                /* RAID ID                              */
    UINT8       status;             /* RBR status                           */
    UINT8       delayTime;          /* Delay time remaining                 */
    UINT64      rlen;               /* Rebuild length left to do            */
                                    /* QUAD BOUNDARY                    *****/
    struct RBR *nextRBR;            /* Next RBR to do                       */
    struct PDD *pdd;                /* PDD of the drive being rebuilt       */
    UINT32      delayLogTime;       /* Seconds delayed before log           */
    UINT8       rsvd28[4];          /* Reserved                             */
                                    /* QUAD BOUNDARY                    *****/
} RBR;

#endif /* _RBR_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
