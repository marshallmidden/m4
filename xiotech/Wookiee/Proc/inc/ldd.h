/* $Id: ldd.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       ldd.h
**
**  @brief      Linked device descriptors
**
**  To provide a common means of defining the Linked Device Description
**  (LDD) structure.
**
**  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _LDD_H_
#define _LDD_H_

#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/**
**  @name LDD Device type field definitions
**  @{
**/
#define LD_MLD          PD_MLD      /**< MAGNITUDE link device              */
#define LD_FTD          PD_FTD      /**< Foreign target device              */
/*@}*/

/**
**  @name LDD State field definitions
**  @{
**/
#define LDD_ST_UNDEF    0           /**< Undefined (non-existent)           */
#define LDD_ST_OP       1           /**< Operational                        */
#define LDD_ST_INOP     2           /**< In-operable                        */
#define LDD_ST_UNINIT   3           /**< Uninitialized                      */
#define LDD_ST_PTERM    4           /**< Pending termination                */
#define LDD_ST_TERM     5           /**< Terminated                         */
/*@}*/

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/** Linked Device Descriptors                                               */
typedef struct LDD
{
    UINT8       class;              /* Device class                         */
    UINT8       state;              /* LDD state code                       */
    UINT8       pathMask;           /* Path mask                            */
                                    /*  Path # bit mask map                 */
                                    /*  0 = path blocked                    */
                                    /*  1 = path enabled                    */
    UINT8       pathPri;            /* Path priority                        */
                                    /*  Path # bit map                      */
                                    /*  checked if path not blocked         */
                                    /*  0 = secondary path                  */
                                    /*  1 = primary path                    */
    UINT32      owner;              /* Current Owning Controller            */
    UINT64      devCap;             /* Device capacity                      */
                                    /* QUAD BOUNDARY                    *****/
    UINT8       vendID[8];          /* Vendor ID                            */
    UINT32      rev;                /* Revision                             */
    UINT16      altid;              /* Alternate ID                         */
    UINT8       rsvd20[2];          /* Reserved                             */
                                    /* QUAD BOUNDARY                    *****/
    UINT8       prodID[16];         /* Product ID                           */
                                    /* QUAD BOUNDARY                    *****/
    UINT8       serial[12];         /* Serial number                        */
    UINT8       rsvd60[4];          /* Reserved                             */
                                    /* QUAD BOUNDARY                    *****/
    UINT16      lun;                /* LUN                                  */
    UINT16      baseVDisk;          /* Base MAG VDisk number                */
    UINT8       baseCluster;        /* Base MAG cluster number              */
    UINT8       rsvd69[3];          /* Reserved                             */
    UINT32      baseSN;             /* Base MAG serial number               */
    UINT8       rsvd76[4];          /* Reserved                             */
                                    /* QUAD BOUNDARY                    *****/
    UINT8       baseName[16];       /* Base device name                     */
                                    /* QUAD BOUNDARY                    *****/
    UINT64      baseNode;           /* Base MAG node name                   */
    UINT8       rsvd88[8];          /* Reserved                             */
                                    /* QUAD BOUNDARY                    *****/
    struct TPMT* pTPMTHead;         /* Associated TPMT list head pointer    */
    struct TPMT* pLastTPMT;         /* Last path TPMT used                  */
    struct ILT* pAILTHead;          /* Active ILT op list head pointer      */
    struct ILT* pAILTTail;          /* Active ILT op list tail pointer      */
                                    /* QUAD BOUNDARY                    *****/
    struct PCB* pPCB;               /* Active process PCB pointer           */
    UINT8       flag1;              /* Process evant flag #1                */
    UINT8       lock;               /* LDD lock                             */
    UINT8       numPaths;           /* Number of paths                      */
    UINT8       rsvd119;            /* Reserved                             */
    UINT16      ord;                /* LDD ordinal                          */
    UINT16      retryCount;         /* Error retry ILT count                */
    UINT8       rsvd124[4];         /* Reserved                             */
                                    /* QUAD BOUNDARY                    *****/
} LDD;

#endif /* _LDD_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
