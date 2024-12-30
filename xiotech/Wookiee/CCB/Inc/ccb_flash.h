/* $Id: ccb_flash.h 130553 2010-03-04 17:33:12Z mdr $ */
/**
******************************************************************************
**
**  @file    ccb_flash.h
**
**  @brief   Header file for ccb_flash.c
**
**  Copyright (c) 2001, 2009-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _CCB_FLASH_H_
#define _CCB_FLASH_H_

#include "XIO_Types.h"

#include "mutex.h"

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
typedef UINT32 CCB_FLASH;

/* Flash sector size definitions */
#define CCB_FLASH_NUMBER_OF_SECTORS         71
#define CCB_FLASH_FIRST_SECTOR_NUM          0
#define CCB_FLASH_LARGE_SECTOR_SIZE         (SIZE_64K * 4)

/*
** CCB Flash Sector Allocation Table
*/

/* CCB runtime code */
extern UINT32 BASE_FLASH;

/* CCB flash log space (4 sectors) */
extern UINT32 FLASH_LOG_START_ADDRESS;

#define FLASH_LOG_SECTOR_OFFSET             24
#define FLASH_LOG_NUM_SECTORS               4
#define LS_NUM_SECTORS                      4
#define LS_SECTOR_SIZE                      CCB_FLASH_LARGE_SECTOR_SIZE

/* CCB Device Configuration Information (1 sector) */
extern UINT32 FLASH_DEVICE_CONFIG_START_ADDRESS;

#define FLASH_DEVICE_CONFIG_SECTOR_OFFSET   49

/* Backtrace data copy space (2 sectors) */
extern UINT32 BASE_FLASH_BACKTRACE_ADDRESS;

/* CCB debug flash log space (6 sectors) */
extern UINT32 FLASH_DEBUG_LOG_START_ADDRESS;

#define FLASH_DEBUG_LOG_SECTOR_OFFSET       52
#define FLASH_DEBUG_LOG_NUM_SECTORS         6
#define LS_NUM_SECTORS_DEBUG                6
#define LS_SECTOR_SIZE_DEBUG                CCB_FLASH_LARGE_SECTOR_SIZE

/* Static data sector */
extern UINT32 BASE_FLASH_CCB_STATIC_DATA;

#define CCB_STATIC_DATA_SECTOR_OFFSET       59

/* From ccb_flash.c */
extern volatile CCB_FLASH *CCBFlashBase;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern UINT32 CCBFlashGetAddressFromSector(UINT32, CCB_FLASH **);
extern UINT32 CCBFlashGetSectorFromAddress(CCB_FLASH *, UINT32 *);
extern UINT32 CCBFlashProgramData(CCB_FLASH *, CCB_FLASH *, UINT32);
extern CCB_FLASH CCBFlashReadWord(UINT32 offset);
extern UINT32 CCBFlashEraseSector(UINT32);
extern UINT32 CCBFlashGetSizeOfSector(UINT32 sector);

/*****************************************************************************
** Public variables
*****************************************************************************/

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _CCB_FLASH_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
