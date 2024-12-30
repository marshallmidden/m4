/* $Id: RL_RDD.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       RL_RDD.h
**
**  @brief      RAID device descriptors
**
**  To provide a common means of defining the RAID Device Description
**  (RDD) structure.
**
**  Copyright (c) 1996-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
/* ATTENTION:
 *   If fields of these data structures are changed, it is probably necessary
 *   to update the Direct Memory Copy (DMC) raid information via:
 *       BIT_SET(DMC_bits, CCB_DMC_raidcache);   // Flag raid data has changed.
*/

/**
**  @defgroup _RL_RDD_H_ RAID Device Descriptors
**  @{
**/
#ifndef _RL_RDD_H_
#define _RL_RDD_H_

#include "XIO_Types.h"

#include "XIO_Const.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/**
**  @name Type Field Definitions
**  @{
**/
#define RD_STD          0           /**< Standard device type               */
#define RD_RAID0        1           /**< RAID 0 device type                 */
#define RD_RAID1        2           /**< RAID 1 device type                 */
#define RD_RAID5        3           /**< RAID 5 device type                 */
#define RD_RAID10       4           /**< RAID 10 device type                */
#define RD_LINKDEV      5           /**< Link RAID (SAN Links) device type  */
#define RD_SLINKDEV     6           /**< Snapshot device type               */
/* @} */

/**
**  @name Status Field Definitions
**  @{
**/
#define RD_NONX         0x00        /**< Non-existent                       */
#define RD_INOP         0x01        /**< Inoperative                        */
#define RD_UNINIT       0x02        /**< Uninitialized                      */
                                    /**< 03 previously Scheduled for init   */
#define RD_INIT         0x04        /**< Initializing                       */
#define RD_ERROR        0x05        /**< Error occurred                     */
#define RD_OP           0x10        /**< Operational                        */
#define RD_DEGRADED     0x11        /**< Degraded                           */
/* @} */

/*
**  @name Additional Status Bit Definitions
**  @{
**/
#define RD_A_PARITY            0    /**< 0x01 Parity scan required          */
#define RD_A_REBUILD           1    /**< 0x02 Rebuild required              */
#define RD_A_UNINIT            2    /**< 0x04 Uninitialized                 */
#define RD_A_TERMBG            3    /**< 0x08 Terminate all background processing */
#define RD_A_R5SRIP            4    /**< 0x10 RAID 5 Stripe Resync In Progress */
#define RD_A_REBUILD_WRITES    5    /**< 0x20 Rebuild before writes required */
#define RD_A_LOCAL_IMAGE_IP    6    /**< 0x40 Local Image Update in Progress.
                                     **  No Writes are allowed to the RAID
                                     **  while this bit is on               */
/* @} */

/*
**  @name Parity Checker Bit Definitions
**  @{
**/
#define RD_PC_NEWCMD    31          /**< Parity checker has a new command   */
#define RD_PC_DEFAULT   7           /**< Use default value                  */
#define RD_PC_MARKED    6           /**< RDD is marked for scanning         */
#define RD_PC_CORRUPT   5           /**< Enable parity corruption           */
#define RD_PC_SPECIFIC  4           /**< Test a specific RID                */
#define RD_PC_CLEARLOG  3           /**< Clear log before start             */
#define RD_PC_1PASS     2           /**< Single pass (versus infinite)      */
#define RD_PC_CORRECT   1           /**< Correct errors                     */
#define RD_PC_ENABLE    0           /**< Enable parity checking             */
#define RD_PC_DEFCTL    0x8000004F  /**< Default control word:
                                     ** enable, correct, 1 pass, clear logs,
                                     ** marked RDDs                         */
/* @} */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/** Get RAID Information Extensions                                         */
typedef struct RDD_EXT
{
    UINT16  pid;            /**< PID                                        */
    UINT8   pidstat;        /**< PID status                                 */
    UINT8   rpc;            /**< Rebuild percent complete                   */
    UINT8   pidastat;       /**< PID astatus                                */
    UINT8   rsvd2[3];       /**< RESERVED                                   */
} RDD_EXT;

/** RAID Device Descriptor                                                  */
typedef struct RDD
{
    UINT16      rid;        /**< RAID ID                                    */
    UINT8       type;       /**< RAID type                                  */
    UINT8       status;     /**< Device status                              */
    UINT8       depth;      /**< Mirror depth/stripe width
                             **     For RAID 1/10 it contains
                             **     the mirror depth.
                             **     \n
                             **     For RAID 5 it contains the
                             **     stripe width of 3, 5 or 9.
                             **                                             */
    UINT8       pctRem;     /**< Initialize percent remaining               */
    UINT16      psdCnt;     /**< Count of PSD entries for this device       */
    UINT32      sps;        /**< Sectors per stripe for RAID 0, 5, 10       */
    UINT32      spu;        /**< Sectors per unit
                             **     For RAID 10 it contains
                             **     (psdcnt*sps).
                             **     \n
                             **     For RAID 5 it contains
                             **     (depth-1)*sps.
                             **                                             */
                            /* QUAD */
    UINT64      devCap;     /**< Device capacity                            */
    struct RDD *pNRDD;      /**< Next RDD in this VDisk                     */
    UINT16      vid;        /**< Virtual ID of owner                        */
    UINT16      frCnt;      /**< Failed/rebuild count                       */
                            /* QUAD */
    UINT32      error;      /**< Error count                                */
    UINT32      qd;         /**< Queue depth                                */
    UINT32      rps;        /**< Avg req/sec (last second)                  */
    UINT32      avgSC;      /**< Avg sector count (last second)             */
                            /* QUAD */
    UINT64      rReq;       /**< Read request count                         */
    UINT64      wReq;       /**< Write request count                        */
                            /* QUAD */
    UINT64      llsda;      /**< Locked LSDA (RAID 1,10)                    */
    UINT64      lleda;      /**< Locked LEDA (RAID 1,10)                    */
                            /* QUAD */
    UINT32      iprocs;     /**< Init RAID processes                        */
    UINT32      ierrors;    /**< Init RAID errors                           */
    UINT64      isectors;   /**< Init RAID sectors                          */
                            /* QUAD */
    UINT32      misComp;    /**< Parity errors in this RAID                 */
    UINT32      pardrv;     /**< Parity checks with missing drive           */
    UINT16      defLock;    /**< Defragmentation lock count                 */
    UINT8       aStatus;    /**< Additional status                          */
    UINT8       r5SROut;    /**< RAID 5 Stripe Resync Outstanding Count     */
    UINT32      notMirroringCSN; /**< Controller SN of RAID Not Mirroring Info*/
                            /* QUAD */
    UINT32      sprCnt;     /**< Sample period request count                */
    UINT32      spsCnt;     /**< Sample period sector  count                */
    struct RPN *pRPNHead;   /**< RPN thread head (RAID 5)                   */
    struct VLOP *vlop;      /**< Associated VLOP address                    */
                            /* QUAD */
    union
    {
        ZeroArray(struct PSD *, pPSD);
        ZeroArray(RDD_EXT, extList);
    } extension;
} RDD;

/** RDD index table                                                         */
typedef struct RDX
{
#if 0
    UINT32          maxIndx;        /**< current max index                  */
#endif
    UINT16          count;          /**< Number of RAID devices             */
    UINT8           rsvd2[2];       /**< RESERVED                           */
    RDD            *rdd[MAX_RAIDS]; /**< Array of RAID devices              */
} RDX;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _RL_RDD_H_ */
/* @} */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
