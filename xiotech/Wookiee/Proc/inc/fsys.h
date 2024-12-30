/* $Id: fsys.h 157933 2011-09-07 18:08:39Z m4 $ */
/**
******************************************************************************
**
**  @file       fsys.h
**
**  @brief      File System Definition
**
**              Define the file system used for keeping internal information
**              on the disk drives which are part of the system.
**
**  Copyright (c) 2000 - 2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _FSYS_H_
#define _FSYS_H_

#include "XIO_Types.h"

#include "ilt.h"
#include "FS_Structs.h"
#include "system.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name   System wide parameters for reserved area.
**
**          NOTE THAT THE FILE SYSTEM UPDATE WILL BE TRUNCATED TO
**          ROUGHLY WHERE THE LAST FILE LIES.  IF THERE ARE ADDITIONAL
**          FILES USED, THIS MUST BE ADJUSTED!
**
**/
/*@{*/
#define FSYS_UPDATE_LEN     (RESERVED_AREA_SIZE / 2)
/*@}*/

/**
**  @name   File IDs
**
**          The file IDs that the BE processor needs to recognize are limited
**          to the file ID of the directory itself and the files that it uses.
**          This is a complete list for completeness and will probably be
**          replicated in the CCB code.
**/
/*@{*/
#define FID_DIR             0x00    /**< Directory FID                      */
#define FID_LABEL           0x01    /**< Label sector                       */
#define FID_BE_NVRAM        0x02    /**< Back end NVRAM                     */
#define FID_FE_NVRAM        0x03    /**< Front end NVRAM                    */
#define FID_CCB_NVRAM       0x04    /**< CCB NVRAM                          */
#define FID_SCRATCH         0x05    /**< Scratch area for POST              */
#define FID_COPY            0x0B    /**< Resync copy NVRAM                  */
/*@}*/

/**
**  @name   File size and starting location constants
**/
/*@{*/
#define FID_DIR_SIZE        0x09    /**< Directory file size                */
#define FID_DIR_START       0x00    /**< Directory file start               */

#define FID_LABEL_SIZE      (FS_SIZE_LABEL + 1)            /**< Label file size*/
#define FID_LABEL_START     (FID_DIR_SIZE + FID_DIR_START) /**< Starting LBA*/

#define FID_BE_NVRAM_SIZE   (NVRAM_P2_SIZE_SECTORS + 1) /**< BE NVRAM size  */
#define FID_BE_NVRAM_START  (FID_LABEL_SIZE  + FID_LABEL_START) /**< BE start */

#define FID_FE_NVRAM_SIZE   (NVRAM_P2_SIZE_SECTORS + 1) /**< FE NVRAM size  */
#define FID_FE_NVRAM_START  (FID_BE_NVRAM_SIZE  + FID_BE_NVRAM_START) /**< FE start */

#define FID_CCB_NVRAM_SIZE  (NVRAM_P2_SIZE_SECTORS + 1) /**< CCB NVRAM size  */
#define FID_CCB_NVRAM_START (FID_FE_NVRAM_SIZE  + FID_FE_NVRAM_START) /**< CCB start */

#define FID_SCRATCH_SIZE    (FS_SIZE_SCRATCH + 1)     /**< Scratch size  */
#define FID_SCRATCH_START   (FID_CCB_NVRAM_SIZE  + FID_CCB_NVRAM_START) /**< Scratch start */
/*@}*/

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

struct PDD;

/**
** File system multi-write record structure
**/
typedef struct FSYS_MWRT
{
    UINT32  activeCnt;                      /**< Active write counter       */
    UINT32  goodCnt;                        /**< Good write counter         */
    UINT32  errorCnt;                       /**< Write error counter        */
    UINT32  goodMapAddr;                    /**< Address of good write map  */
} FSYS_MWRT;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern void FS_InitDirectory(struct PDD *pPDD);

extern UINT32 FS_MultiRead(UINT32 fid, void* pBuf, UINT32 len,
                    UINT32 confirm, UINT32 offset, UINT32 dummy,
                    UINT32 mapAddr);

extern UINT32 FS_MultiWrite(UINT32 fid, void* pBuf, UINT32 len, UINT32 mapAddr,
                UINT32 offset, UINT32* pGoodCount, UINT32* pErrCount);

extern UINT32 FS_ReadFile(UINT32 fid, void* pBuf, UINT32 len,
                    PDD* pPDD, UINT32 offset);

extern UINT32 FS_UpdateFS(struct PDD *pPDD, UINT32);

extern UINT32 FS_WriteFile(UINT32 fid, void* pBuf, UINT32 len,
                    struct PDD *pPDD, UINT32 offset);

extern void FS$que(UINT32 dummy, ILT *pILT);

#endif /* _FSYS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
