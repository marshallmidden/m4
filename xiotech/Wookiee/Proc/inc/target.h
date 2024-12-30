/* $Id: target.h 145983 2010-08-19 19:09:38Z m4 $ */
/**
******************************************************************************
**
**  @file       target.h
**
**  @brief      Target device descriptors
**
**  Copyright (c) 2004-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _TARGET_H_
#define _TARGET_H_

#include "globalOptions.h"
#include "DEF_iSCSI.h"
#include "XIO_Types.h"
#include "XIO_Const.h"
#include "system.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

#define MAX_TARGETS_PER_PORT 31     /* Max targets defined per interface    */
#define CTL_TARGET          0       /* Target defined as control interface  */
#define NO_CLUSTER          0xFFFF  /* Indicates not target clustering      */

/* Bit options for taropt and tdgopt */
#define TARGET_HARD_ID      0       /* 1 = Hard Loop ID, 0 = Soft Loop ID   */
#define TARGET_ENABLE       1       /* 1 = Enabled, 0 = Disabled            */
#define TARGET_PREV_ID      2       /* 1 = Prev Asn Loop ID (LIPA), 0 = none*/
#define TARGET_ICL          3       /* 1 = ICL target                       */
#define TARGET_RESET        4       /* 1 = iSCSI Target with active Sessions*/
#define TARGET_SESSIONS     5       /* 1 = iSCSI Target with active Sessions*/
#define TARGET_ACTIVE       6       /* 1 = iSCSI Target with a listening soc*/
#define TARGET_ISCSI        7       /* 1 = iSCSI Target 0 = FC Target       */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/* Target structure */
typedef struct TAR
{
    struct TAR *fthd;               /* Forward thread                       */
    UINT16      tid;                /* Target ID                            */
    UINT16      entry;              /* Target entry number                  */
    UINT8       opt;                /* Target options                       */
    UINT8       tsih;
    UINT8       hardID;             /* Target hard assigned ID              */
    union {
        struct {
            UINT8   rsvd2;
            UINT64  portName;       /* Target port world wide name          */
        };
#if ISCSI_CODE
        struct {
            UINT8   ipPrefix;       /* IP prefix for a classless IP address */
            UINT32  ipAddr;         /* Target IP Address                    */
            UINT32  ipGw;           /* Default Gateway IP Address           */
        };
#endif
    };
    UINT64      nodeName;           /* Target node world wide name          */
    union {
        UINT32      flags;          /* FC4 registration flag                */
        struct {
            UINT16      tsih_seed;
            UINT16      ssn_cnt;
        };
    };
    UINT32      vpID;               /* Virtual port ID (LID)                */
    UINT32      portID;             /* Port ID                              */
} TAR;

/* An actual TGD */
typedef struct TGD
{
    UINT16      tid;                /* Target ID                            */
    UINT8       port;               /* Port Number                          */
    UINT8       opt;                /* Target options                       */
    UINT8       fcid;               /* FC ID if hard                        */
    UINT8       rsvd5;              /* Reserved                             */
    UINT8       lock;               /* Locked target indicator              */
    union {
        struct {
            UINT8   rsvd7;          /* Reserved                             */
            UINT64  portName;       /* FC port world wide name              */
        };
#if ISCSI_CODE
        struct {
            UINT8   ipPrefix;       /* IP prefix for a classless IP address */
            UINT32  ipAddr;         /* Target IP Address                    */
            UINT32  ipGw;           /* Default Gateway IP Address           */
        };
#endif
    };
                                    /* QUAD BOUNDARY                    *****/
    UINT64      nodeName;           /* FC node world wide name              */
    UINT32      prefOwner;          /* Serial number of preferred owner     */
    UINT32      owner;              /* Serial number of current owner       */
                                    /* QUAD BOUNDARY                    *****/
    UINT16      cluster;            /* Cluster number                       */
    UINT16      rsvd2;              /* Reserved                             */
    UINT8       prefPort;           /* Preferred port                       */
    UINT8       altPort;            /* Alternate port                       */
    UINT8       rsvd3[2];           /* Reserved                             */
    UINT32      i_mask;             /* Configurable Params                  */
    I_TGD      *itgd;               /* iSCSI TGD info                       */
                                    /* QUAD BOUNDARY                    *****/
} TGD;

/* TGD index table */
typedef struct TDX
{
    UINT16      count;              /* Number of target devices             */
    UINT8       rsvd2[2];           /* RESERVED                             */
    TGD        *tgd[MAX_TARGETS];   /* Array of target devices              */
} TDX;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern TAR    *tar[MAX_PORTS+MAX_ICL_PORTS];
extern UINT32  tar_link_abort[MAX_PORTS + MAX_ICL_PORTS];

#endif /* _TARGET_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
