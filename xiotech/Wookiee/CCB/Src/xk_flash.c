/* $Id: xk_flash.c 130553 2010-03-04 17:33:12Z mdr $ */
/**
******************************************************************************
**
**  @file   xk_flash.c
**
**  @brief  CCB flash functions
**
**  This file contains all the necessary functions for
**  working with the CCB/Linux emulated flash parts.
**
**  Copyright (c) 2004-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "ccb_hw.h"
#include "mach.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "xk_mapmemfile.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

/*****************************************************************************
** Private function prototypes
*****************************************************************************/

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/

/* Flash sector size definitions */
UINT32  BASE_FLASH = 0;

/* CCB flash log space (4 sectors) */
UINT32  FLASH_LOG_START_ADDRESS = 0;

/* CCB Device Configuration Information (1 sector) */
UINT32  FLASH_DEVICE_CONFIG_START_ADDRESS = 0;

/* Backtrace data copy space (2 sectors) */
UINT32  BASE_FLASH_BACKTRACE_ADDRESS = 0;

/* CCB debug flash log space (6 sectors) */
UINT32  FLASH_DEBUG_LOG_START_ADDRESS = 0;

/* From ccb_flash.c */
volatile CCB_FLASH *CCBFlashBase = 0;


/*****************************************************************************
** Private variables
*****************************************************************************/
static const UINT32 CCBFlashSectors[CCB_FLASH_NUMBER_OF_SECTORS + 1] =
{
    0x000000,                   /* SA0   - Bank 2 - 64kB sector */
    0x010000,                   /* SA1   - Bank 2 - 64kB sector */
    0x020000,                   /* SA2   - Bank 2 - 64kB sector */
    0x030000,                   /* SA3   - Bank 2 - 64kB sector */
    0x040000,                   /* SA4   - Bank 2 - 64kB sector */
    0x050000,                   /* SA5   - Bank 2 - 64kB sector */
    0x060000,                   /* SA6   - Bank 2 - 64kB sector */
    0x070000,                   /* SA7   - Bank 2 - 64kB sector */
    0x080000,                   /* SA8   - Bank 2 - 64kB sector */
    0x090000,                   /* SA9   - Bank 2 - 64kB sector */
    0x0A0000,                   /* SA10  - Bank 2 - 64kB sector */
    0x0B0000,                   /* SA11  - Bank 2 - 64kB sector */
    0x0C0000,                   /* SA12  - Bank 2 - 64kB sector */
    0x0D0000,                   /* SA13  - Bank 2 - 64kB sector */
    0x0E0000,                   /* SA14  - Bank 2 - 64kB sector */
    0x0F0000,                   /* SA15  - Bank 2 - 64kB sector */
    0x100000,                   /* SA16  - Bank 2 - 64kB sector */
    0x110000,                   /* SA17  - Bank 2 - 64kB sector */
    0x120000,                   /* SA18  - Bank 2 - 64kB sector */
    0x130000,                   /* SA19  - Bank 2 - 64kB sector */
    0x140000,                   /* SA20  - Bank 2 - 64kB sector */
    0x150000,                   /* SA21  - Bank 2 - 64kB sector */
    0x160000,                   /* SA22  - Bank 2 - 64kB sector */
    0x170000,                   /* SA23  - Bank 2 - 64kB sector */
    0x180000,                   /* SA24  - Bank 2 - 64kB sector */
    0x190000,                   /* SA25  - Bank 2 - 64kB sector */
    0x1A0000,                   /* SA26  - Bank 2 - 64kB sector */
    0x1B0000,                   /* SA27  - Bank 2 - 64kB sector */
    0x1C0000,                   /* SA28  - Bank 2 - 64kB sector */
    0x1D0000,                   /* SA29  - Bank 2 - 64kB sector */
    0x1E0000,                   /* SA30  - Bank 2 - 64kB sector */
    0x1F0000,                   /* SA31  - Bank 2 - 64kB sector */
    0x200000,                   /* SA32  - Bank 2 - 64kB sector */
    0x210000,                   /* SA33  - Bank 2 - 64kB sector */
    0x220000,                   /* SA34  - Bank 2 - 64kB sector */
    0x230000,                   /* SA35  - Bank 2 - 64kB sector */
    0x240000,                   /* SA36  - Bank 2 - 64kB sector */
    0x250000,                   /* SA37  - Bank 2 - 64kB sector */
    0x260000,                   /* SA38  - Bank 2 - 64kB sector */
    0x270000,                   /* SA39  - Bank 2 - 64kB sector */
    0x280000,                   /* SA40  - Bank 2 - 64kB sector */
    0x290000,                   /* SA41  - Bank 2 - 64kB sector */
    0x2A0000,                   /* SA42  - Bank 2 - 64kB sector */
    0x2B0000,                   /* SA43  - Bank 2 - 64kB sector */
    0x2C0000,                   /* SA44  - Bank 2 - 64kB sector */
    0x2D0000,                   /* SA45  - Bank 2 - 64kB sector */
    0x2E0000,                   /* SA46  - Bank 2 - 64kB sector */
    0x2F0000,                   /* SA47  - Bank 2 - 64kB sector */
    0x300000,                   /* SA48  - Bank 1 - 64kB sector */
    0x310000,                   /* SA49  - Bank 1 - 64kB sector */
    0x320000,                   /* SA50  - Bank 1 - 64kB sector */
    0x330000,                   /* SA51  - Bank 1 - 64kB sector */
    0x340000,                   /* SA52  - Bank 1 - 64kB sector */
    0x350000,                   /* SA53  - Bank 1 - 64kB sector */
    0x360000,                   /* SA54  - Bank 1 - 64kB sector */
    0x370000,                   /* SA55  - Bank 1 - 64kB sector */
    0x380000,                   /* SA56  - Bank 1 - 64kB sector */
    0x390000,                   /* SA57  - Bank 1 - 64kB sector */
    0x3A0000,                   /* SA58  - Bank 1 - 64kB sector */
    0x3B0000,                   /* SA59  - Bank 1 - 64kB sector */
    0x3C0000,                   /* SA60  - Bank 1 - 64kB sector */
    0x3D0000,                   /* SA61  - Bank 1 - 64kB sector */
    0x3E0000,                   /* SA62  - Bank 1 - 64kB sector */

    0x3F0000,                   /* SA63  - Bank 1 - 8kB sector */
    0x3F2000,                   /* SA64  - Bank 1 - 8kB sector */
    0x3F4000,                   /* SA65  - Bank 1 - 8kB sector */
    0x3F6000,                   /* SA66  - Bank 1 - 8kB sector */
    0x3F8000,                   /* SA67  - Bank 1 - 8kB sector */
    0x3FA000,                   /* SA68  - Bank 1 - 8kB sector */
    0x3FC000,                   /* SA69  - Bank 1 - 8kB sector */
    0x3FE000,                   /* SA70  - Bank 1 - 8kB sector */

    0x400000                    /* Not a sector - This marks the end of sector 70 */
};

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief  Writes to "flash"
**
**  Programs the flash devices starting at address CCBFlash, from buffer Source,
**  for Length words. The user MUST ensure that if necessary the underlying
**  sectors have been erased prior to programming new data.
**
**  @param  flash - CCBFlash destination address
**  @param  source - source of flash data
**  @param  words - length of data to program (in flash words)
**
**  @return GOOD (always)
**
******************************************************************************
**/
UINT32 CCBFlashProgramData(CCB_FLASH *flash, CCB_FLASH *source, UINT32 words)
{
    memcpy(flash, source, 4 * words);

    MEM_FlushMapFile(flash, 4 * words);

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Get adress of flash from sector number
**
**  Pass in the sector you wish to program or erase and the corresponding
**  address in the flash memory will be stored into the location pointed
**  to by the second parameter.
**
**  @param  sector - flash sector number
**  @param  fp  - Pointer to location to get address
**
**  @return GOOD or ERROR
**  @return *fp Set to the address corresponding to the sector number
**
******************************************************************************
**/
UINT32 CCBFlashGetAddressFromSector(UINT32 sector, CCB_FLASH **fp)
{
    if (sector >= CCB_FLASH_NUMBER_OF_SECTORS)
    {
        *fp = NULL;
        return ERROR;
    }

    /*
     * Takes adresses of sectors from table and multiplies by 4 because
     * the flash is set up word wide, but the table is for byte wide devices.
     */
    *fp = (CCB_FLASH *)((CCBFlashSectors[sector] * sizeof(CCB_FLASH)) + BASE_FLASH);

    return GOOD;
}


/*----------------------------------------------------------------------------
**  Function Name: CCBFlashGetSectorFromAddress
**
**  Comments:  Pass in the address you wish to program or erase, and the base
**             address of the flash memory, get back a flash sector to pass to
**             the rest of the routines.
**
**  Parameters: FlashBase - CCBFlashBase base address
**              flashAddressPtr - flash address to find sector for
**
**  Modifies:   *sector - Sector number corresponding to the address
**
**  Returns:    GOOD or ERROR
**--------------------------------------------------------------------------*/
UINT32 CCBFlashGetSectorFromAddress(CCB_FLASH *flashAddressPtr, UINT32 *sector)
{
    UINT32      sectorCounter = 0;
    UINT32      sectorBaseAddress = 0;

    /*
     * Check that the address is, in fact, located in the flash address range.
     */
    if (((UINT32)flashAddressPtr >= BASE_FLASH) &&
        ((UINT32)flashAddressPtr < (BASE_FLASH + SIZE_FLASH)))
    {
        /*
         * Traverse the sector address table - looking for the sector that the address
         * fits into.  If it's not found in the 'for' loop, then the address is in the
         * last sector, so return that.
         */
        for (sectorCounter = 0; sectorCounter < CCB_FLASH_NUMBER_OF_SECTORS;
             sectorCounter++)
        {
            /*
             * Find the size of the current sector for use in comparison below.
             */
            sectorBaseAddress = (CCBFlashSectors[sectorCounter] * sizeof(CCB_FLASH)) + BASE_FLASH;

            /*
             * Find the sector that contains the address that has been passed in.
             */
            if (((UINT32)flashAddressPtr >= sectorBaseAddress) &&
                ((UINT32)flashAddressPtr <
                 (sectorBaseAddress + CCBFlashGetSizeOfSector(sectorCounter))))
            {
                *sector = sectorCounter;
                return GOOD;
            }
        }
    }

    *sector = 0;
    return ERROR;
}

/*----------------------------------------------------------------------------
**  Function Name: CCBFlashGetSizeOfSector
**
**  Comments:   Pass in the sector you wish to find the size of, and this
**              function will tell you the size of your sector.
**
**  Parameters: Sector - flash sector number
**
**  Returns:    Size of specified sector
**--------------------------------------------------------------------------*/
UINT32 CCBFlashGetSizeOfSector(UINT32 sector)
{
    UINT32      sectorSize = 0;

    if (sector >= CCB_FLASH_NUMBER_OF_SECTORS)
    {
        return (0);
    }

    /*
     * Takes adresses of sectors from table, multiplies by 4 (the flash is
     * set up word wide, but the table is for byte wide devices.
     */
    sectorSize = CCBFlashSectors[sector + 1] * sizeof(CCB_FLASH);
    sectorSize -= CCBFlashSectors[sector] * sizeof(CCB_FLASH);

    return (sectorSize);
}

/**
******************************************************************************
**
**  @brief  Erase a single flash sector
**
**  @param  sectorNum - flash sector number
**
**  @return GOOD except when sectorNum is out of range
**
******************************************************************************
**/
UINT32 CCBFlashEraseSector(UINT32 sectorNum)
{
    CCB_FLASH   *flashPtr = (CCB_FLASH *)CCBFlashBase;
    UINT32      byteCounter;

    /*
    ** Compute the sector offset for the flash device and invoke the flash
    ** erase function.
    */
    if (CCBFlashGetAddressFromSector(sectorNum, &flashPtr) != GOOD)
    {
        return ERROR;
    }

    byteCounter = CCBFlashGetSizeOfSector(sectorNum);
    memset(flashPtr, 0xFF, byteCounter);

    /* Flush the flash */
    MEM_FlushMapFile(flashPtr, byteCounter);

    return GOOD;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
