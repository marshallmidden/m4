/* $Id: nvd.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       nvd.h
**
**  @brief      Non-Volatile Diagnostic Data storage
**
**  To show the format of diagnostic data located in Part 5 of
**  NVRAM.  This data is stored whenever an errortrap occurs and
**  will aid in diagnosis of the failure.
**
**  Copyright (c) 1996 - 2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _NVD_H_
#define _NVD_H_

#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name   Part 5 NVRAM Entries
**/
/*@{*/
#define NVD_CONSTANTS               0       /**< Constants                  */
#define NVD_INTERNAL_INFO           1       /**< Internal information       */
#define NVD_FW_HDR_RUN              2       /**< Runtime firmware header    */
#define NVD_FW_HDR_BOOT             3       /**< Boot firmware header       */
#define NVD_FW_HDR_DIAG             4       /**< Diag firmware header       */
#define NVD_FLIGHT_REC              5       /**< Flight recorder            */
#define NVD_MRP                     6       /**< MRP trace                  */
#define NVD_DEFINE_QUEUE            7       /**< Define exec que            */
#define NVD_DEFINE_QUEUE_PCB        8       /**< Define que PCB             */
#define NVD_CURRENT_PCB             9       /**< Currently running PCB      */
#define NVD_TARGET_DEF              10      /**< Target definition          */
#define NVD_TARGET_STRUCT           11      /**< Target structure           */
#define NVD_LLDMT_DIR               12      /**< LLDMT directory            */
#define NVD_LLDMT                   13      /**< LLDMT                      */
#define NVD_DTMT                    14      /**< DTMT                       */
#define NVD_TPMT                    15      /**< TPMT                       */
#define NVD_MLMT                    16      /**< MLMT                       */
#define NVD_LQCS                    17      /**< Link layer Q control struct*/
#define NVD_IRQ_Q0                  18      /**< ISP Request Queue 0        */
#define NVD_IRP_Q0                  19      /**< ISP Response Queue 0       */
#define NVD_IRQ_Q1                  20      /**< ISP Request Queue 1        */
#define NVD_IRP_Q1                  21      /**< ISP Response Queue 1       */
#define NVD_IRQ_Q2                  22      /**< ISP Request Queue 2        */
#define NVD_IRP_Q2                  23      /**< ISP Response Queue 2       */
#define NVD_IRQ_Q3                  24      /**< ISP Request Queue 3        */
#define NVD_IRP_Q3                  25      /**< ISP Response Queue 3       */

#ifdef BACKEND
#define NVD_BE_IRAM                 26      /**< BE IRAM                    */
#define NVD_DEFRAG                  27      /**< Defrag trace               */
#define NVD_PHY_EXEC_QUEUE          28      /**< Physical exec que          */
#define NVD_PHY_EXEC_PCB            29      /**< Physical exec PCB          */
#define NVD_PHY_CMPL_QUEUE          30      /**< Physical completion que    */
#define NVD_PHY_CMPL_PCB            31      /**< Physical completion PCB    */
#define NVD_RAID_EXEC_QUEUE         32      /**< Raid exec que              */
#define NVD_RAID_EXEC_PCB           33      /**< Raid exec PCB              */
#define NVD_RAID5_EXEC_QUEUE        34      /**< Raid 5 exec que            */
#define NVD_RAID5_EXEC_PCB          35      /**< Raid 5 exec PCB            */
#define NVD_VIRTUAL_EXEC_QUEUE      36      /**< Virtual exec que           */
#define NVD_VIRTUAL_EXEC_PCB        37      /**< Virtual exec PCB           */
#define NVD_RAID_INIT_EXEC_QUEUE    38      /**< Raid Init exec que         */
#define NVD_RAID_INIT_EXEC_PCB      39      /**< Raid Init exec PCB         */
#define NVD_XOR_CMPL_EXEC_QUEUE     40      /**< XOR completion exec que    */
#define NVD_XOR_CMPL_EXEC_PCB       41      /**< XOR completion exec PCB    */
#define NVD_XOR_EXEC_QUEUE          42      /**< XOR exec que               */
#define NVD_XOR_EXEC_PCB            43      /**< XOR exec PCB               */
#define NVD_FILE_SYS_EXEC_QUEUE     44      /**< File system exec que       */
#define NVD_FILE_SYS_EXEC_PCB       45      /**< File system exec PCB       */
#define NVD_RAID_ERROR_EXEC_QUEUE   46      /**< Raid error exec que        */
#define NVD_RAID_ERROR_EXEC_PCB     47      /**< Raid error exec PCB        */
#define NVD_VDD                     48      /**< VDDs                       */
#define NVD_INQUIRE_PCB             49      /**< Inquire PCB                */
#define NVD_INQUIRE_PDD             50      /**< Inquire PDD                */
#endif

#ifdef FRONTEND
#define NVD_FE_IRAM                 26      /**< FE IRAM                    */
#define NVD_TMT                     27      /**< TMT                        */
#define NVD_TLMT                    28      /**< TLMT                       */
#define NVD_ISMT                    29      /**< ISMT                       */
#define NVD_LTMT                    30      /**< LTMT                       */
#define NVD_LSMT                    31      /**< LSMT                       */
#define NVD_CIMT_DIR                32      /**< CIMT directory             */
#define NVD_CIMT                    33      /**< CIMTs                      */
#define NVD_TRACE_LOG               34      /**< Incoming trace log         */
#define NVD_IMT                     35      /**< IMTs                       */
#define NVD_ILMT                    36      /**< ILMTs                      */
#define NVD_VDMT                    37      /**< VDMTs                      */
#define NVD_SDD                     38      /**< SDDs                       */
#define NVD_LVM                     39      /**< LVMs                       */
#define NVD_ILVM                    40      /**< Invisible LVMs             */
#define NVD_VCD                     41      /**< VCDs                       */
#define NVD_SERVER_DBASE_DIR        42      /**< Server database directory  */
#define NVD_SERVER_DBASE            43      /**< Server database            */
#define NVD_CACHE_QUEUE             44      /**< Cache queue, head and tail */
#define NVD_CACHE_PCB               45      /**< Cache PCB                  */
#define NVD_CACHE_IO_QUEUE          46      /**< Cache IO queue, head & tail*/
#define NVD_CACHE_IO_PCB            47      /**< Cache I/O PCB              */
#define NVD_ICIMT_DIR               48      /**< ICIMT directory            */
#define NVD_ICIMT                   49      /**< ICIMT                      */
#define NVD_INIT_TRACE_LOG          50      /**< Initiator trace log        */
#endif

#define NVD_LABEL_SIZE              8       /**< Pt 5 NVRAM Entry Lbl Size  */
#define NVD_HDR_LABEL_SIZE          8       /**< Pt 5 NVRAM Hdr Lbl Size    */
/*@}*/

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** Part 5 NVRAM header structure
**/
typedef struct NVRAM_P5_HDR
{
    UINT8   idStr[8];       /**< ID str, Diag BE=BE, Diag FE=FE             */
    UINT32  length;         /**< Length of nvram pt 5 area                  */
    UINT32  crc;            /**< CRC of part 5 area                         */

    UINT8   timeStamp[8];   /**< Timestamp                                  */
    UINT8   rsvd24[8];      /*   Reserved                                   */
} NVRAM_P5_HDR;

/**
** Part 5 NVRAM entry structure
**/
typedef struct NVRAM_P5_ENTRY
{
    UINT8   id[8];          /**< ID of data                                 */
    void*   addr;           /**< Address of data                            */
    UINT32  length;         /**< Length of data                             */
} NVRAM_P5_ENTRY;

/**
** Part 5 NVRAM entry structure
**/
typedef struct NVRAM_P5_CONSTANTS
{
    UINT32  maxTargets;     /**< Max # of targets                           */
    UINT32  maxISPs;        /**< Max # of ISPs                              */
    UINT32  maxVDisks;      /**< Max # of vdisks                            */
    UINT32  maxCIMTs;       /**< Max # of CIMTs                             */

    UINT32  maxLUNs;        /**< Max # of LUNs                              */
    UINT32  maxServers;     /**< Max # of servers                           */
    UINT32  maxICIMTs;      /**< Max # of ICIMTs                            */
    UINT32  maxChannels;    /**< Max # of channels                          */

    UINT32  maxRaids;       /**< Max # of raids                             */
    UINT32  maxDevices;     /**< Max # of devices                           */
    UINT32  maxVRPs;        /**< Max # of VRPs                              */
    UINT32  maxDrives;      /**< Max # of drives                            */

    UINT32  maxInterfaces;  /**< Max # of interfaces                        */
    UINT32  maxControllers; /**< Max # of controllers                       */
    UINT32  maxSESs;        /**< Max # of SESs                              */
    UINT32  maxLIDs;        /**< Max # of LIDs                              */
} NVRAM_P5_CONSTANTS;

#endif /* _NVD_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
