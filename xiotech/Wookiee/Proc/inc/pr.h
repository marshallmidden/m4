/* $Id: pr.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       pr.h
**
**  @brief      Reserve and release (both persistent and non-persistent)
**
**  To provide a common means of storing reservations (persistent and otherwise)
**
**  Copyright (c) 2007-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _PR_H_
#define _PR_H_

#include "MR_Defs.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

#define MAX_KEYS             32         /* MAX keys supported                        */

/* Reservation types */
#define RESV_NONE            0x0        /* No reservation present                    */
#define RESV_WR_EXCL         0x1        /* Write exclusive                           */
#define RESV_EXCL_ACC        0x3        /* Exclusive access                          */
#define RESV_WR_EXCL_RO      0x5        /* Write exclusive - registrants only        */
#define RESV_EXCL_ACC_RO     0x6        /* Exclusive access - registrants only       */
#define RESV_WR_EXCL_AR      0x7        /* Write exclusive - all registrants         */
#define RESV_EXCL_ACC_AR     0x8        /* Exclusive access - all registrants        */
#define RESV_NON_PERSISTENT  0xf        /* Reservation using reserve(6)/reserve(10)  */

/* PR error codes */
#define PRERR_RESV_OK          0x0
#define PRERR_RESV_CONF        0x1
#define PRERR_INV_PARAM_LEN    0x2
#define PRERR_INVFL_CDB        0x3
#define PRERR_INV_RELEASE      0x4
#define PRERR_INVFL_PARAM_LIST 0x5
#define PRERR_INSUFF_REG_RES   0x6
#define PRERR_IN_PROGRESS      0xffff

/* PR Unit Attention Code Bits & ilmt flag3 bits*/
#define PR_UA_RESV_PREEMPTED   0
#define PR_UA_RESV_RELEASED    1
#define PR_UA_REG_PREEMPTED    2

#define PR_CFGRETRIEVE         7

/*
** PR in commands
*/
#define   PRESV_IN_READ_KEYS             0x00
#define   PRESV_IN_READ_RESERVATION      0x01
#define   PRESV_IN_REPORT_CAPABILITIES   0x02
#define   PRESV_IN_READ_FULL_STATUS      0x03
/*
** PR out commands
*/
#define   PRESV_OUT_REGISTER             0x00
#define   PRESV_OUT_RESERVE              0x01
#define   PRESV_OUT_RELEASE              0x02
#define   PRESV_OUT_CLEAR                0x03
#define   PRESV_OUT_PREEMPT              0x04
#define   PRESV_OUT_PREEMPT_AND_ABORT    0x05
#define   PRESV_OUT_REGISTER_AND_IGNORE  0x06
#define   PRESV_OUT_REGISTER_AND_MOVE    0x07

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/*
 * REGKEYS structure is used to store a registered key. This is also used to
 * identify a path (I_T_L) and it's registered key.
 */
typedef struct REGKEYS
{
    UINT16    vid;                /* vdisk ID                             */
    UINT16    sid;                /* Server ID                            */
    UINT8     tid;                /* Target ID                            */
    UINT8     lun;                /* Lun #                                */
    UINT16    rsvd;               /* For quad boundary                    */
    UINT8     key[8];             /* Registration key                     */
} REGKEYS;

/*
 * RESV structure is used to store all registered keys for a particular
 * vdisk and also a reservation (if present).
 */
typedef struct RESV
{
    UINT16   vid;                     /* vdisk ID                                       */
    UINT8    scope:4;                 /* Scope of Reservation (currently only LU_SCOPE) */
    UINT8    resvType:4;              /* Type of reservation                            */
    UINT8    rsvd;                    /* Just padding it up                             */
    REGKEYS  *keyset[MAX_KEYS];       /* Set of keys registered for vdisk               */
    UINT16   maxIdx;                  /* Total # of keys registered                     */
    INT16    rsvdIdx;                 /* Index of key in 'keyset' which is
                                           granted reservation                          */
} RESV;

#ifdef BACKEND
extern MRSETPRES_REQ* gRsvData[MAX_VIRTUAL_DISKS];
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
#ifdef BACKEND
extern UINT8 DEF_UpdatePres (MR_PKT* pMRP);
#endif

#ifdef FRONTEND
extern UINT8 get_pr_data (UINT16 vid, MRPRGET_RSP * p_resp);
extern UINT8 pr_cfgClear(UINT16 vid);
extern UINT8 pr_cfgcomp(UINT16 vid, UINT16 sid, INT32 rc);
extern UINT8 pr_cfgChange(UINT16 vid, UINT16 sid, UINT16 flags);
#endif

#endif /* _PR_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
