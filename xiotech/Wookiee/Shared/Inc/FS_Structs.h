/* $Id: FS_Structs.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       FS_Structs.h
**
**  @brief      Internal File System common data structures
**
**  To provide a common means of defining the internal file system
**  data structure.
**
**  Copyright (c) 1996-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
/**
**  @defgroup _FS_Structs_H_ Internal File System data structures
**  @{
**/
#ifndef _FS_STRUCTS_H_
#define _FS_STRUCTS_H_

#include "FW_Header.h"

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
#define MRP_FSYSOP_TIMEOUT (30*1000)            /* 30 sec                   */

#define BLOCK_SZ                512

#define FS_DIRNAME_LEN    10        /**< Directory name length              */

/* @} */

/**
**  @name   File names
**/
/* @{ */
#define FS_FNAME_DIR                "Directory "
#define FS_FNAME_LABEL              "XIO Label "
#define FS_FNAME_BENVRAM            "BE NVRAM  "
#define FS_FNAME_FENVRAM            "FE NVRAM  "
#define FS_FNAME_CCBNVRAM           "CCB NVRAM "
#define FS_FNAME_SCRATCH            "ST Scratch"
#define FS_FNAME_QM_MASTER_CONFIG   "MASTER_CON"
#define FS_FNAME_QM_CONTORLLER_MAP  "CONTRL_MAP"
#define FS_FNAME_QM_COMM_AREA       "COMM_AREA "
#define FS_FNAME_FW_COMPAT_DATA     "FW CMP DAT"
#define FS_FNAME_RSVD_10            "UNUSED  10"
#define FS_FNAME_COPY_NVRAM         "COPY_NVRAM"
#define FS_FNAME_RSVD_12            "UNUSED  12"
#define FS_FNAME_RSVD_13            "UNUSED  13"
#define FS_FNAME_RSVD_14            "UNUSED  14"
#define FS_FNAME_RSVD_15            "UNUSED  15"
#define FS_FNAME_RSVD_16            "UNUSED  16"
#define FS_FNAME_RSVD_17            "UNUSED  17"
#define FS_FNAME_RSVD_18            "UNUSED  18"
#define FS_FNAME_RSVD_19            "UNUSED  19"
#define FS_FNAME_RSVD_20            "UNUSED  20"
#define FS_FNAME_RSVD_21            "UNUSED  21"
#define FS_FNAME_RSVD_22            "UNUSED  22"
#define FS_FNAME_CKP_DIRECTORY      "CKP_DIRECT"
/* @} */

/**
**  @name   File IDs
**
**          The file IDs that the BE processor needs to recognize are limited
**          to the file ID of the directory itself and the files that it uses.
**          This is a complete list for completeness and will probably be
**          replicated in the CCB code.
**/
/* @{ */
/*
** Preset by the back end when a file system is created.
*/
#define FS_FID_DIRECTORY                0   /**< Directory file FID         */
#define FS_FID_DEVLABEL                 1   /**< Label file FID             */
#define FS_FID_BE_NVRAM                 2   /**< Back end NVRAM file FID    */
#define FS_FID_FE_NVRAM                 3   /**< Front end NVRAM file FID   */
#define FS_FID_CCB_NVRAM                4   /**< UNUSED IN BIGFOOT          */
#define FS_FID_BE_SCRATCH               5   /**< Scratch file for POST FID  */

/*
** CCB defined FID's
*/
#define FS_FID_QM_MASTER_CONFIG         6
#define FS_FID_QM_CONTROLLER_MAP        7
#define FS_FID_QM_COMM_AREA             8
#define FS_FID_FW_COMPAT_DATA           9
#define FS_FID_RSVD_10                  10
#define FS_FID_COPY_NVRAM               11   /* Larry Dean */
#define FS_FID_RSVD_12                  12
#define FS_FID_RSVD_13                  13
#define FS_FID_RSVD_14                  14
#define FS_FID_RSVD_15                  15
#define FS_FID_RSVD_16                  16
#define FS_FID_RSVD_17                  17
#define FS_FID_RSVD_18                  18
#define FS_FID_RSVD_19                  19
#define FS_FID_RSVD_20                  20
#define FS_FID_RSVD_21                  21
#define FS_FID_RSVD_22                  22
#define FS_FID_CKP_DIRECTORY            23
#define FS_FID_CKP_MASTER_CONFIG        24  /* 32 consecutive FIDS */
#define FS_FID_CKP_CONTROLLER_MAP       (FS_FID_CKP_MASTER_CONFIG + FS_NUM_SNAPSHOT_FIDS)
#define FS_FID_CKP_BE_NVRAM             (FS_FID_CKP_CONTROLLER_MAP + FS_NUM_SNAPSHOT_FIDS)

/*
** Insert new fids above this one, and move this one down as appropriate.
*/
#define FS_FID_FIRST_EMPTY              (FS_FID_CKP_BE_NVRAM+FS_NUM_SNAPSHOT_FIDS)
#define FS_FID_LAST_EMPTY               255
/* @} */

/**
**  @name   File Sizes
**
**          The file sizes are the size of the file less the header sector.
**          This size is in blocks.
**
**/
/* @{ */
#define FS_SIZE_DIRECTORY               8
#define FS_SIZE_LABEL                   1
#define FS_SIZE_BENVRAM                 (0x170000 / BLOCK_SIZ)
#define FS_SIZE_FENVRAM                 (0x170000 / BLOCK_SIZ)
#define FS_SIZE_CCB_NVRAM               (0x170000 / BLOCK_SIZ)
#define FS_SIZE_SCRATCH                 256
#define FS_SIZE_QM_MASTER_CONFIG        8
#define FS_SIZE_QM_CONTROLLER_MAP       24
#define FS_SIZE_QM_COMM_AREA            1024
#define FS_SIZE_FW_COMPAT_DATA          1
#define FS_SIZE_RSVD_FID_10             1023
#define FS_SIZE_COPY_NVRAM              256
#define FS_SIZE_RSVD_FID_12             32
#define FS_SIZE_RSVD_FID_13             8
#define FS_SIZE_RSVD_FID_14             128
#define FS_SIZE_RSVD_FID_15             64
#define FS_SIZE_RSVD_FID_16             32
#define FS_SIZE_RSVD_FID_17             1
#define FS_SIZE_RSVD_FID_18             32
#define FS_SIZE_RSVD_FID_19             128
#define FS_SIZE_RSVD_FID_20             128
#define FS_SIZE_RSVD_FID_21             64
#define FS_SIZE_RSVD_FID_22             64
#define FS_SIZE_CKP_DIRECTORY           (4*FS_NUM_SNAPSHOT_FIDS) /* 128 */
/* @} */

#define FS_NUM_SNAPSHOT_FIDS            32
#define FS_NUM_NAME_FIDS                10  /* Number of FIDs for naming    */
#define FS_NUM_VLINK_NAME_FIDS          4   /* Number of FIDs for to VLinks */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
**  @name   File System Directory Entry Structure
**/
/* @{ */
typedef struct FS_DIR_ENTRY
{
    UINT8   fileName[FS_DIRNAME_LEN];      /**< Name of the file              */
    UINT16  lbaCount;                      /**< No. of blocks occupied by file*/
    UINT32  lbaOffset;                     /**< Starting (LBA) of the file    */
} FS_DIR_ENTRY;

/**
** @name     File System File Header structure
**/
typedef struct FS_FILE_HEAER
{
    UINT8   fileName[FS_DIRNAME_LEN];      /**< Name of the file              */
    UINT8   rsvd10[6];
    UINT32  fid;                           /**< File ID                       */
    UINT32  length;                        /**< File Length                   */
    UINT8   rsvd24[4];
    UINT32  dataCRC;                       /**< Data CRC                      */
    FW_HEADER_TIMESTAMP timeStamp;         /**< Time stamp                    */
    UINT8   rsvd40[468];
    UINT32  hdrCRC;                        /**< Header CRC                    */
} FS_FILE_HEADER;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SES_STRUCTS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
