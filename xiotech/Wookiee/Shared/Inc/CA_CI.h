/* $Id: CA_CI.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       CA_CI.h
**
**  @brief      Cache Information
**
**  To provide a common means of providing a consistent definition
**  of cache statistics.
**
**  Copyright (c) 1996 - 2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
/**
**  @defgroup _CA_CI_H_ Cache Information
**  @{
**/
#ifndef _CA_CI_H_
#define _CA_CI_H_

#include "XIO_Types.h"
#include "globalOptions.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name Cache Status Definitions
**  @{
**/
#define CA_ENA              0       /* b0 = 0 Global Cache Disabled         */
                                    /*    = 1 Global Cache Enabled          */
#define CA_DIS_IP           1       /* b1 = 0 No disable in progress        */
                                    /*    = 1 Global Cache Disable in Prog  */
#define CA_ENA_PEND         2       /* b2 = 0 No enable pending             */
                                    /*    = 1 Cache Enable is pending       */
#define CA_NWAYMIRROR       3       /* b3 = 0 1-way mirror to this ctrl BE  */
                                    /*    = 1 N-way mirror to other ctrl BE */
#define CA_MIRRORBROKEN     4       /* b4 = 0 N-way mirror working          */
                                    /*    = 1 N-way mirror broken           */
#define CA_ERROR            5       /* b5 = 0 No errors detected            */
                                    /*    = 1 Errors detected using Cache   */
#define CA_SHUTDOWN         6       /* b6 = 0 Cache not shutdown            */
                                    /*    = 1 Cache shutdown                */
#define CA_HALT_BACKGROUND  7       /* b7 = 0 Background Flush normal       */
                                    /*    = 1 No Background Flush allowed   */
#define CA_NO_MORE_DATA     0x72    /* Bits = 6, 5, 4, and 1 should not     */
                                    /*  allow more data to come into cache  */
                                    /*  for any VID                         */
/* @} */

/**
**  @name Cache Status 1 Definitions - bits used in Write Cache data Restore
**  @{
**/
#define CA_RESTORE_DATA     0       /* b0 = 0 WC Data from NV not Restored  */
                                    /*    = 1 WC Data from NV is Restored   */
                                    /* b1-b7 Reserved                       */
/* @} */

/**
**  @name Cache Status 2 Definitions - bits used in disabling Write Cache
**  @{
**/
#define CA_MY_BATTERY       0       /* b0 = 0 My Battery is Good            */
                                    /*    = 1 My Battery is Bad             */
#define CA_MIRROR_BATTERY   1       /* b1 = 0 Mirror Batt is Good (Mine or MP)*/
                                    /*    = 1 Mirror Batt is Bad (Mine or MP) */
                                    /* b2-b6 Reserved                       */
#define CA_TEMP_DISABLE     7       /* b7 = 0 Normal Write Caching          */
                                    /*    = 1 Temp Disable WC               */
/* @} */

/**
**  @name Battery Status
**  @{
**/
#define CA_BAT_GOOD         0       /**< battery good                       */
#define CA_BAT_LOW          1       /**< battery low                        */
/* @} */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/** Cache Information - Need to rename to CI during C conversion */
typedef struct CA
{
    UINT8   status;             /**< Cache status                           */
    UINT8   status1;            /**< Cache status 1                         */
    UINT8   status2;            /**< Cache status 2                         */
    UINT8   stopCnt;            /**< Stop I/O Count                         */
    UINT32  size;               /**< Cache size                             */
    UINT32  maxcwr;             /**< Maximum cached write                   */
    UINT32  maxsgl;             /**< Mazimum num of SGLs/op                 */
    UINT32  numTags;            /**< Total number of tags                   */
    UINT32  tagsDirty;          /**< Current num of tags dirty              */
    UINT32  tagsResident;       /**< Current # of tags resident             */
    UINT32  tagsFree;           /**< Current num of tags free               */
    UINT32  tagsFlushIP;        /**< Current # tags flush in progr          */
    UINT32  numBlks;            /**< Total number of blocks                 */
    UINT32  blocksDirty;        /**< Current num of dirty blocks            */
    UINT32  blocksResident;     /**< Current # of blocks res                */
    UINT32  blocksFree;         /**< Current # of blocks free               */
    UINT32  blocksFlushIP;      /**< Num blocks flush in prog               */
    UINT8   rsvd56[8];          /**< RESERVED                               */
} CA;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _CA_CI_H_ */
/* @} */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
