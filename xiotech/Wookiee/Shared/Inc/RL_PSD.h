/* $Id: RL_PSD.h 152890 2011-01-20 21:01:48Z m4 $ */
/**
******************************************************************************
**
**  @file       RL_PSD.h
**
**  @brief      Physical segment descriptors
**
**  To provide a common means of defining the Physical Segment Description
**  (PSD) structure.
**
**  Copyright (c) 1996-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

/* ATTENTION:
 *   If fields ps_pid, ps_status, or ps_astatus are changed, it is probably
 *   necessary to update the Direct Memory Copy (DMC) raid information via:
 *      BIT_SET(DMC_bits, CCB_DMC_raidcache);   // Flag raid data has changed.
 */
/**
**  @defgroup _RL_PSD_H_ Physical Segment Descriptors
**  @{
**/
#ifndef _RL_PSD_H_
#define _RL_PSD_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name Status Field Definitions
**  @{
**/
#define PSD_NON_EXIST       0x00    /**< Non-existent                       */
#define PSD_INOP            0x01    /**< Inoperable                         */
#define PSD_UNINIT          0x02    /**< Uninitialized                      */
                                    /**< 0x03 - (previously sched for init) */
#define PSD_INIT            0x04    /**< Initializing                       */
#define PSD_ERROR           0x05    /**< Error occurred                     */
#define PSD_OP              0x10    /**< Operational                        */
#define PSD_DEGRADED        0x11    /**< Degraded                           */
#define PSD_REBUILD         0x12    /**< Rebuild needed                     */
/* @} */

/**
**  @name Additional Status Bit Definitions
**  @{
**/
#define PSDA_PARITY             0   /**< 0x01 RAID 5 requires parity check  */
#define PSDA_REBUILD            1   /**< 0x02 Rebuilding                    */
#define PSDA_UNINIT             2   /**< 0x04 Uninitialized                 */
#define PSDA_DEFRAG             3   /**< 0x08 Defragmenting                 */
#define PSDA_ERROR              4   /**< 0x10 Error                         */
#define PSDA_HOT_SPARE          5   /**< 0x20 Hotspare needed               */
#define PSDA_HOT_SPARE_REQD     6   /**< 0x40 Hotspare required             */
#define PSDA_REBUILD_WAIT       7   /**< 0x80 Wait for PDD operative        */
/* @} */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
**  Physical segment descriptors
**/
typedef struct PSD
{
    UINT16      pid;            /**< Device ID for this segment             */
    UINT8       status;         /**< Segment status                         */
    UINT8       pbFail;         /**< Previous boot fail T/F                 */
    struct PSD *npsd;           /**< Next PSD entry (circular)              */
    UINT16      rid;            /**< RAID ID associated with segment        */
    UINT8       aStatus;        /**< Additional status                      */
    UINT8       rsvd10[5];      /**< Reserved                               */
                                /**< QUAD BOUNDARY                      *****/
    UINT64      sda;            /**< Starting disk address                  */
    UINT64      sLen;           /**< Segment length                         */
                                /**< QUAD BOUNDARY                      *****/
    UINT64      rLen;           /**< Rebuild length                         */
    UINT8       rsvd40[8];      /**< Reserved                               */
                                /**< QUAD BOUNDARY                      *****/
} PSD;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _RL_PSD_H_ */
/* @} */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
