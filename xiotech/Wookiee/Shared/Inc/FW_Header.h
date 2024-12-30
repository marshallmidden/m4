/* $Id: FW_Header.h 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       FW_Header.h
**
**  @brief      This file defines the common FW header variables and
**              structures.
**
**  All FW modules that are downloaded to a system need this FW header
**  pre-pended to them to define CRC's, load locations etc. Refer to
**  FW_HeaderSrc.c for a better explanation of the how the FW header is
**  built and used.
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef FW_HEADER_H
#define FW_HEADER_H

#include "XIO_Types.h"

#ifdef CCB_BOOT_CODE
 #include "FW_Sections.h"
#endif

#ifdef CCB_RUNTIME_CODE
 #include "ccb_flash.h"
#endif

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/* FW Header constants */
#define MAGIC_NUMBER            0x0FFCCADE  /* Magic Number for a fw header */
#define PROD_BIGFOOT            0x00001000  /* Product ID for Bigfoot       */
#define PROD_HYPERNODE          0x00002000  /* Product ID for Hypernode     */
#define PROD_750                0x00002750  /* Product ID for 750           */
#define PROD_4700               0x00004700  /* Product ID for 4000 looking like 7000 */
#define PROD_7000               0x00007000  /* Product ID for Nitrogen      */
#define PROD_7400               0x00007400  /* Product ID for 7000 looking like 4000 */
#define SYSTEM_RELEASE_ENGR     0x72676E45  /* "Engr" in 4 byte hex         */
#define I960_BR_FWD_128         0x08000080

#if   defined(MODEL_3000)
 #define PROD_ID PROD_HYPERNODE
#elif   defined(MODEL_4700)
 #define PROD_ID PROD_4700
#elif   defined(MODEL_7000)
 #define PROD_ID PROD_7000
#elif   defined(MODEL_7400)
 #define PROD_ID PROD_7400
#elif defined(XIO_XWS)
    /* Compiling for EWOK / ICON Services. FW header constants are not needed
    *  by EWOK
    */
#else
 #error  "MODEL not recognized"
#endif

/* Processor build specific constants */

#ifdef CCB_RUNTIME_CODE
 #define TARG_ID                 TARG_ID_CCB
#endif

#ifdef FRONTEND
 #define TARG_ID                 TARG_ID_FE
#endif

#ifdef BACKEND
 #define TARG_ID                 TARG_ID_BE
#endif

/*
** TARGET ID's
*/
/* CCB firmware: */
#define TARG_ID_CCB             0x101       /* Target ID for CCB            */

/* FE firmware: */
#define TARG_ID_FE              0x002       /* Target ID for Front End      */
#define TARG_ID_FE_DIAG         0x102       /* Target ID for Front End Diag */
#define TARG_ID_FE_BOOT         0x202       /* Target ID for Front End Boot */
#define TARG_ID_FE_QLOGIC       0x302       /* Target ID for FE QLogic code */
#define TARG_ID_FE_NVRAM        0x402       /* Target ID for FE NVRAM image */

/* BE firmware/nvram: */
#define TARG_ID_BE              0x003       /* Target ID for Back End       */
#define TARG_ID_BE_DIAG         0x103       /* Target ID for Back End Diag  */
#define TARG_ID_BE_BOOT         0x203       /* Target ID for Back End Boot  */
#define TARG_ID_BE_QLOGIC       0x303       /* Target ID for BE QLogic code */
#define TARG_ID_BE_NVRAM        0x403       /* DEPRECATED */

/* FE/BE Common firmware: */
#define TARG_ID_COMMON_DIAG     0x104       /* Target ID for FE/BE Diag     */
#define TARG_ID_COMMON_BOOT     0x204       /* Target ID for FE/BE Boot     */
#define TARG_ID_COMMON_QLOGIC   0x304       /* Target ID for FE/BE QLogic   */

/* New stuff for Hypernode */
#define TARG_ID_PLATFORM_RPM    0x105       /* TID for platform apps rpm    */
#define TARG_ID_NULL            0x205       /* TID for NULL file (discard)  */

/* Disk Bay and Disk Drive types: */
#define TARG_ID_DRIVE_BAY       0x10F       /* XIOtech Disk Bay Firmware    */
#define TARG_ID_EUROLOGIC_BAY   0x30F       /* Disk Bay Firmware            */
#define TARG_ID_DISK_DRIVE      0x40F       /* Disk Drive Firmware          */
#define TARG_ID_ADAPTEC_SATA_BAY    0x50F   /* Adaptec SATA Bay Firmware    */
#define TARG_ID_ADAPTEC_SATA_ES_BAY 0x60F   /* Adaptec SATA ES Module FW    */
#define TARG_ID_XYRATEX_SBOD_BAY 0x70F      /* Xyratex SBOD I/O Module FW   */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/*
** The FW header timestamp format
*/
typedef struct FW_HEADER_TIMESTAMP
{
    UINT16 year;                /**< Build time (year)                        */
    UINT8  month;               /**< Build time (month)                       */
    UINT8  date;                /**< Build time (day of month)                */
    UINT8  day;                 /**< Build time (day of week)                 */
    UINT8  hours;               /**< Build time (hours)                       */
    UINT8  minutes;             /**< Build time (minutes)                     */
    UINT8  seconds;             /**< Build time (seconds)                     */
} FW_HEADER_TIMESTAMP;

/*
** The actual FW header
*/
typedef struct FW_HEADER
{
    /* QUADs 0,1 */
    UINT32 branch;              /**< The first 8 words are reserved for       */
    UINT32 rsvd_0x00[7];        /**< assemble code instructions to branch to
                                     the FW entry point (immediately
                                     following this header.                   */
    /* QUAD 2 */
    UINT32 magicNumber;         /**< FW Magic Number                          */
    UINT32 rsvd_0x24;
    UINT32 productID;           /**< Product ID                               */
    UINT32 target;              /**< Target within product                    */

    /* QUAD 3 */
    UINT32 revision;            /**< Main Revision Identifier                 */
    UINT32 revCount;            /**< Secondary Rev ID - usually build count   */
    UINT32 buildID;             /**< First 4 letters of builder's name        */
    UINT32 systemRelease;       /**< The associated System Release Level      */

    /* QUAD 4 */
    FW_HEADER_TIMESTAMP timeStamp; /**< Build time                            */
    UINT32 rsvd_0x48;
    UINT32 burnSequence;        /**< Incremented in the CCB flash every time
                                     a new version of this same FW is burned
                                     to flash.                                */
    /* QUAD 5 */
    UINT32 ccbAddrA;            /**< CCB Flash address, copy A                */
    UINT32 ccbAddrB;            /**< CCB Flash address, copy B                */
    UINT32 targAddr;            /**< Target Flash address                     */
    UINT32 length;              /**< Length of binary image (incl header)     */

    /* QUADs 6,7 */
    UINT32 checksum;            /**< CRC32 of binary data (not incl header)   */
    UINT8  fwCompatIndex;       /**< FW compatibility index - the release to
                                     release compatibility of this FW *set*
                                     This is only used in the CCBRun FW hdr.  */

    UINT8  fwBackLevelIndex;    /**< FW backlevel index - an indication of if
                                     a backlevel to a previous FW *SET* is
                                     allowed or not.  This is only used in
                                     the CCBRun FW header.                    */

    UINT8  fwSequencingIndex;   /**< FW sequencing index - an indication of if
                                     a forward or backlevel to a FW *SET* is
                                     allowed or not. If the fwSequencingIndex
                                     of the *SET* you are going TO is the same
                                     or 1 off of where you are now, the update
                                     will be allowed (assuming the above
                                     inicies allow it).  This is only used in
                                     the CCBRun FW header.                    */
    UINT8  rsvd_0x67;
    UINT16 fwMajorRel;          /**< FW Major Release Level                   */
    UINT16 fwMinorRel;          /**< FW Minor Release Level                   */
    UINT32 rsvd_0x6C[4];
    UINT32 hdrCksum;            /**< CRC32 of this header (not binary data)   */

} FW_HEADER;

/* The FW identifier structure in "Config Journal" entries */

typedef struct FW_DATA
{
    UINT32 revision;            /* Firmware Revision                        */
    UINT32 revCount;            /* Firmware Revision Counter                */
    UINT32 buildID;             /* Who / where firmware was built           */
    FW_HEADER_TIMESTAMP timeStamp; /* Time Firmware was built               */
} FW_DATA;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern FW_HEADER fwHeader;


#ifdef __cplusplus
#pragma pack(pop)
#endif

/**
** vi:sw=4 ts=4 expandtab
*/

#endif /* FW_HEADER_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
