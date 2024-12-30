/* $Id: ccb_hw.c 130553 2010-03-04 17:33:12Z mdr $ */
/**
******************************************************************************
**
**  @file       ccb_hw.c
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

/* XIOtech includes */
#include "ccb_hw.h"

#include "ccb_flash.h"
#include "nvram_structure.h"
#include "xssa_structure.h"

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT32 BASE_NVRAM = 0;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
UINT32      XSSA_PERS_DATA_BASE = 0;
UINT32      CCB_NVRAM_BASE = 0;

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Initialize all the NVRAM pointers.
**
**  @param      baseNVRAM - pointer to base NVRAM.
**
**  @return     none
**
******************************************************************************
**/
void HW_InitNVRAM(UINT8 *baseNVRAM)
{
    BASE_NVRAM = (UINT32)baseNVRAM;
    XSSA_PERS_DATA_BASE = BASE_NVRAM;
    CCB_NVRAM_BASE = BASE_NVRAM + XSSA_PERS_DATA_SIZE;
    xssaData = (XSSA_STRUCTURE *)XSSA_PERS_DATA_BASE;
    nvramData = (NVRAM_STRUCTURE *)CCB_NVRAM_BASE;
}

/**
******************************************************************************
**
**  @brief      Initialize all the flash pointers.
**
**  @param      baseFlash - pointer to base flash.
**
**  @return     none
**
******************************************************************************
**/
void HW_InitFlash(UINT8 *baseFlash)
{
    BASE_FLASH = (UINT32)baseFlash;

    /* CCB flash log space (4 sectors) */
    FLASH_LOG_START_ADDRESS = (BASE_FLASH + 0x00600000);
    /* #define FLASH_LOG_SECTOR_OFFSET             24  */

    /* CCB Device Configuration Information (1 sector) */
    FLASH_DEVICE_CONFIG_START_ADDRESS = (BASE_FLASH + 0x00C40000);

    /* Backtrace data copy space (2 sectors) */
    BASE_FLASH_BACKTRACE_ADDRESS = (BASE_FLASH + 0x00C80000);

    /* CCB debug flash log space (6 sectors) */
    FLASH_DEBUG_LOG_START_ADDRESS = (BASE_FLASH + 0x00D00000);
    /* #define FLASH_DEBUG_LOG_SECTOR_OFFSET       52  */

    /* From ccb_flash.c */
    CCBFlashBase = (CCB_FLASH *)BASE_FLASH;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
