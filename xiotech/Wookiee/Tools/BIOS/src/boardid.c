/* $Id: boardid.c 51790 2008-05-14 18:59:20Z mdr $ */
/**
******************************************************************************
**
**  @file       boardid.c
**
**  @brief      Produce a return code that identifies the motherboard.
**
**  @author     Michael McMaster
**
**  @date       09/27/2004
**
**  This program generates the following return codes:
**      0   - Unknown motherboard
**      1   - X5DP8
**      2   - X6DH8-XG2
**      3   - X6DHR-3G2
**      255 - Error
**
**  Copyright (c) 2004-2008 Xiotech Corporation. All rights reserved.
**     
******************************************************************************
**/

#undef _POSIX_SOURCE
#undef __STRICT_ANSI__
#undef  __dj_ENFORCE_ANSI_FREESTANDING
#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>


/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
/**
**  @name UNSIGNED TYPE DEFINITIONS
**  @{
**/
typedef unsigned short      uint16_t;
typedef unsigned char       uint8_t;
/*@}*/ 


/** 
**  @name   DMI table location constants
**/
/*@{*/ 
#define DMI_SEARCH_TABLE_START  0x000F0000
#define DMI_SEARCH_TABLE_END    0x000FFFFF
#define DMI_SEARCH_TABLE_LENGTH (DMI_SEARCH_TABLE_END - DMI_SEARCH_TABLE_START)
/*@}*/ 

/** 
**  @name   SMBIOS and DMI strings
**/
/*@{*/ 
#define DMI_SEARCH_STRING_SM    "_SM_"
#define DMI_SEARCH_STRING_DMI   "_DMI_"
/*@}*/ 

/** 
**  @name   SMBIOS structure types
**/
/*@{*/ 
#define TYPE_BIOS_INFORMATION       0
#define TYPE_SYSTEM_INFORMATION     1
#define TYPE_BASE_BOARD_INFORMATION 2
/*@}*/ 

/** 
**  @name   Various other defines
**/
/*@{*/ 
#define GOOD                        0
#define ERROR                       1
/*@}*/ 

/** 
**  @name   Motherboard string identifiers
**/
/*@{*/ 
#define STRING_SUPERMICRO_X5DP8     "X5DP8"
#define STRING_SUPERMICRO_X6DH8_XG2 "X6DH8-XG2"
#define STRING_SUPERMICRO_X6DHR_3G2 "X6DHR-8G2/X6DHR-TG"
#define STRING_SUPERMICRO_X7DWE     "X7DWE"
/*@}*/ 

/** 
**  @name   Board type identifiers (return values)
**/
/*@{*/ 
#define BOARD_TYPE_UNKNOWN              0
#define BOARD_TYPE_SUPERMICRO_X5DP8     1
#define BOARD_TYPE_SUPERMICRO_X6DH8_XG2 2
#define BOARD_TYPE_SUPERMICRO_X6DHR_3G2 3
#define BOARD_TYPE_SUPERMICRO_X7DWE     4
/*@}*/ 

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))



/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/

#define packed  __attribute__((__packed__))


/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/
typedef struct packed
{
    uint8_t     anchorString[5] packed; /* Bytes 0 - 4   */
    uint8_t     checksum packed;        /* Byte 5        */
    uint16_t    tableLength packed;     /* Bytes 6 - 7   */
    uint8_t     *pTableAddress packed;  /* Bytes 8 - 11  */
    uint16_t    structureCount packed;  /* Bytes 12 - 13 */
    uint8_t     bcdRevision packed;     /* Byte 14       */
} INTERMEDIATE_ENTRY_POINT;

typedef struct packed
{
    char        anchorString[4] packed;     /* Bytes 0 - 3   */
    uint8_t     checksum packed;            /* Byte 4        */
    uint8_t     entryPointLength packed;    /* Byte 5        */
    uint8_t     majorRevision packed;       /* Byte 6        */
    uint8_t     minorRevision packed;       /* Byte 7        */
    uint16_t    maximumStructureSize packed;    /* Bytes 8 - 9   */
    uint8_t     entryPointRevision packed;  /* Byte 10       */
    uint8_t     formatData[5] packed;       /* Bytes 11 - 15 */
    INTERMEDIATE_ENTRY_POINT intermediate packed;   /* Bytes 16 - 30 */
} ENTRY_POINT;

/* SMBIOS structures */
typedef struct packed
{
    uint8_t     type packed;        /* Byte 0        */
    uint8_t     length packed;      /* Byte 1        */
    uint16_t    handle packed;      /* Bytes 2 - 3   */
} SMBIOS_HEADER;

/* Type 00 - BIOS Information */
typedef struct packed
{
    SMBIOS_HEADER   header packed;
    uint8_t         venderStrIx packed;
    uint8_t         biosVersionStrIx packed;
    uint16_t        biosStartingAddressSegment packed;
    uint8_t         biosReleaseDateStrIx packed;
    uint8_t         biosROMSize packed;
    unsigned long long  biosCharacteristics packed;
    uint8_t         extensionBytes[0] packed;
} BIOS_INFORMATION;

/* Type 01 - System Information */
typedef struct packed
{
    SMBIOS_HEADER   header packed;
    uint8_t         manufacturerStrIx packed;
    uint8_t         productNameStrIx packed;
    uint8_t         versionStrIx packed;
    uint8_t         serialNumberStrIx packed;
    uint8_t         uuid[16] packed;
    uint8_t         wakeUpType packed;
} SYSTEM_INFORMATION;

/* Type 02 - Base Board Information */
typedef struct packed
{
    SMBIOS_HEADER   header packed;
    uint8_t         manufacturerStrIx packed;
    uint8_t         productNameStrIx packed;
    uint8_t         versionStrIx packed;
    uint8_t         serialNumberStrIx packed;
    uint8_t         assetTagStrIx packed;
    uint8_t         featureFlags packed;
    uint8_t         chassisLocationStrIx packed;
    uint16_t        chassisHandle packed;
    uint8_t         boardType packed;
    uint8_t         objectCount packed;
    uint16_t        objectHandles[0] packed;
} BASE_BOARD_INFORMATION;

/* Collection of all defined structures */
typedef union
{
    BIOS_INFORMATION        bios packed;
    SYSTEM_INFORMATION      system packed;
    BASE_BOARD_INFORMATION  board packed;
} STRUCTURE_UNION;


/*
******************************************************************************
** Private variables
******************************************************************************
*/

static char buf1[0x10000];
static char buf2[0x10000];

const char  *board_names[] =    /* Names of boards */
{
    [BOARD_TYPE_SUPERMICRO_X5DP8] = STRING_SUPERMICRO_X5DP8,
    [BOARD_TYPE_SUPERMICRO_X6DH8_XG2] = STRING_SUPERMICRO_X6DH8_XG2,
    [BOARD_TYPE_SUPERMICRO_X6DHR_3G2] = STRING_SUPERMICRO_X6DHR_3G2,
    [BOARD_TYPE_SUPERMICRO_X7DWE] = STRING_SUPERMICRO_X7DWE,
};


/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/


/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static int Checksum(uint8_t *pBuffer, unsigned length);

static SMBIOS_HEADER *FindNextStructure(SMBIOS_HEADER *pHeader);

static char *GetStringByIndex(SMBIOS_HEADER *pHeader, unsigned index);

static int PrintStructure(SMBIOS_HEADER *pHeader);


/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*
 * @brief   GetTable
 *
 *          This function reads the DPMI table
 *
 * @param   pTable - Physical address of table
 *
 * @param   tableSize - Size of table in bytes
 *
 * @param   buf - Address of buffer to receive table
 *
 * @param   bufSize - Size of buffer
 *
 * @return  Returns GOOD on success, ERROR on failure
 */

int GetTable(void *pTable, int tableSize, char *buf, int bufSize)
{
    __dpmi_meminfo  mi;
    int selector;
    int i;

    if (tableSize > bufSize)
    {
        printf("Table too large (%d)\n", tableSize);
        return ERROR;
    }
    mi.address = (unsigned long)pTable;
    mi.size = (tableSize + 4095) & ~4095;
    __dpmi_physical_address_mapping(&mi);
    selector = __dpmi_allocate_ldt_descriptors(1);
    __dpmi_set_segment_base_address(selector, mi.address);
    __dpmi_set_segment_limit(selector, mi.size - 1);

    for (i = 0; i < tableSize; ++i)
    {
        buf[i] = _farpeekb(selector, i);
    }
    return GOOD;
}

/**
******************************************************************************
**
**  @brief      BoardID main
**
**              This function controls the execution.  It first locates
**              the SMBIOS/DMI table, and then calls the structure
**              decoders to print the data.
**
**  @param      none
**
**  @return     Numeric value for system board type (see defines)
**
**  @attention  none
**
******************************************************************************
**/

int main()
{
    int     systemType      = BOARD_TYPE_UNKNOWN;
    int     returnCode      = ERROR;
    uint8_t *pSearch        = buf1;
    uint8_t *pTable         = NULL;
    uint8_t *pEnd;
    char    *pString;
    int     counter;
    SMBIOS_HEADER *pHeader;
    INTERMEDIATE_ENTRY_POINT    *pIntermediate  = NULL;
    BIOS_INFORMATION            *pBIOSInfo      = NULL;
    BASE_BOARD_INFORMATION      *pBoardInfo     = NULL;

    printf("Get DPMI memory info\n");
    dosmemget(DMI_SEARCH_TABLE_START, DMI_SEARCH_TABLE_LENGTH, buf1);
    pEnd = pSearch + DMI_SEARCH_TABLE_LENGTH;

    /*
    ** Scan through the memory space where the DMI table might be located.
    ** We're looking for specific string patterns, with a good checksum.
    ** See the SMBIOS/DMI specification for more details.
    */
    while (pSearch <= pEnd && returnCode != GOOD)
    {
        if (strncmp(pSearch, DMI_SEARCH_STRING_SM, strlen(DMI_SEARCH_STRING_SM)) == 0)
        {
            ENTRY_POINT *pEntry = (ENTRY_POINT *)pSearch;

			printf("SMBIOS");

            if (pEntry->entryPointLength < 0x20 &&
                Checksum(pSearch, pEntry->entryPointLength) == GOOD)
            {
			    printf("  Found\n");
                pIntermediate = &pEntry->intermediate;
                returnCode = GetTable(pIntermediate->pTableAddress,
                        pIntermediate->tableLength, buf2, 0x10000);
                pTable = buf2;
            }
            else
            {
			    printf("  Checksum failed\n");
            }
        }
        else if (strncmp(pSearch, DMI_SEARCH_STRING_DMI, strlen(DMI_SEARCH_STRING_DMI)) == 0)
		{
			printf("Legacy DMI");
            pIntermediate = (INTERMEDIATE_ENTRY_POINT *)pSearch;

            if (Checksum(pSearch, 0x0F) == GOOD)
            {
			    printf("  Found\n");
                returnCode = GetTable(pIntermediate->pTableAddress,
                    pIntermediate->tableLength, buf2, 0x10000);
                pTable = buf2;
            }
            else
            {
			    printf("  Checksum failed\n");
            }
		}

        /* Move forward another 16 bytes (one 'paragraph') */

        pSearch += 16;
    }

    /*
    ** If we've managed to find the SMBIOS/DMI table, then decode and
    ** print the data.
    */
    if (returnCode != GOOD)
    {
        return BOARD_TYPE_UNKNOWN;
    }

    counter = pIntermediate->structureCount;
    pHeader = (SMBIOS_HEADER *)pTable;

    printf("SMBIOS table address: %p\n", pHeader);
    printf("Structure count:      %d\n", counter);

    /*  Show what we found */

    while (pHeader && counter > 0)
    {
        --counter;

        if (pHeader->type == TYPE_BIOS_INFORMATION)
        {
            pBIOSInfo = (BIOS_INFORMATION *)pHeader;
        }
        else if (pHeader->type == TYPE_BASE_BOARD_INFORMATION)
        {
            pBoardInfo = (BASE_BOARD_INFORMATION *)pHeader;
        }
        PrintStructure(pHeader);

        pHeader = FindNextStructure(pHeader);
    }

    /* Decode which type of system board is in the system */

    if (!pBoardInfo)
    {
        return BOARD_TYPE_UNKNOWN;
    }

    pString = GetStringByIndex(&pBoardInfo->header, pBoardInfo->productNameStrIx);

    if (!pString)
    {
        return BOARD_TYPE_UNKNOWN;
    }

    printf("Looking for product string [%s]\n", pString);

    for (systemType = 0; systemType < ARRAY_SIZE(board_names); ++systemType)
    {
        if (!board_names[systemType])
        {
            continue;
        }
        if (strstr(pString, board_names[systemType]))
        {
            printf("Found %s\n", pString);
            return systemType;
        }
    }

    printf("Unknown system type [%s]\n", pString);

    return BOARD_TYPE_UNKNOWN;
}


/**
******************************************************************************
**
**  @brief      Compute a checksum
**
**              This is a simple checksum function
**
**  @param      pBuffer - pointer to where to start checksumming
**  @param      length  - number of bytes to checksum
**
**  @return     GOOD (checksum is zero) or ERROR (non-zero)
**
******************************************************************************
**/
static int Checksum(uint8_t *pBuffer, unsigned length)
{
	uint8_t     checksum    = 0;
	unsigned    counter;
	
	for (counter = 0; counter < length; counter++)
	{
		checksum += pBuffer[counter];
	}

    return checksum == 0 ? GOOD : ERROR;
}


/**
******************************************************************************
**
**  @brief      Finds the next SMIBIOS/DMI structure
**
**              Due to the way the strings are 'invisibly' tacked onto the
**              end of the structures, we need to look for the double-null
**              sequence of bytes.
**
**  @param      pHeader - pointer to the current structure header
**
**  @return     Pointer to next structure (NULL on ERROR)
**
******************************************************************************
**/
static SMBIOS_HEADER *FindNextStructure(SMBIOS_HEADER *pHeader)
{
    uint8_t *pBuffer = (uint8_t *)pHeader;

    if (pHeader == NULL)
    {
        return NULL;
    }

    /* Move past the structure data */
    pBuffer += pHeader->length;

    /*
    ** Look for any strings - these are located after the data.
    ** NOTE: Strings section ends with double NULL
    */
    while (pBuffer[0] != '\0' || pBuffer[1] != '\0')
    {
        if (pBuffer[0] != '\0')
        {
            pBuffer += strlen(pBuffer);
        }
        else
        {
            pBuffer++;
        }
    }

    /* Move past the double NULL */
    pBuffer += 2;

    /* This is the location of the next header */

    pHeader = (SMBIOS_HEADER *)pBuffer;

    return pHeader;
}


/**
******************************************************************************
**
**  @brief      Gets a SMBIOS/DMI string by the index number
**
**              This traverses the SMIBIOS structure, and then parses
**              through the individual strings that are appended to the
**              structure definition.
**
**  @param      pHeader - pointer to a structure header
**  @param      index   - string index number
**
**  @return     pointer to the string (NULL on ERROR)
**
**  @attention  The SMBIOS indexes start at one
**
******************************************************************************
**/
static char *GetStringByIndex(SMBIOS_HEADER *pHeader, unsigned index)
{
    unsigned    stringCounter   = 1;
    char        *pBuffer        = (char *)pHeader;

    if (pHeader == NULL || index < 1)
    {
        return NULL;
    }

    /* Move past the structure data */
    pBuffer += pHeader->length;

    /*
    ** Look for the strings.
    ** NOTE: Strings section ends with double NULL
    */
    while (pBuffer[0] != '\0' || pBuffer[1] != '\0')
    {
        if (pBuffer[0] != '\0')
        {
            if (stringCounter >= index)
            {
                return pBuffer;
            }

            pBuffer += strlen((char *)pBuffer);
            stringCounter++;
        }
        else
        {
            pBuffer++;
        }
    }

    return NULL;
}


/**
******************************************************************************
**
**  @brief      Print the important pieces of some of the SMBIOS structures
**
**              Current printed structures are BIOS, system, and base board
**
**  @param      pHeader - pointer to a structure header
**
**  @return     GOOD (printed) or ERROR (not printed)
**
******************************************************************************
**/
static int PrintStructure(SMBIOS_HEADER *pHeader)
{
    int     returnCode  = ERROR;
    STRUCTURE_UNION *pUnion     = (STRUCTURE_UNION *)pHeader;

    if (pHeader == NULL)
    {
        return returnCode;
    }

    switch (pHeader->type)
    {
    case TYPE_BIOS_INFORMATION:
        printf("BIOS Information\n");
        printf("  Vender:        %s\n",
            GetStringByIndex(pHeader, pUnion->bios.venderStrIx));
        printf("  BIOS version:  %s\n",
            GetStringByIndex(pHeader, pUnion->bios.biosVersionStrIx));
        printf("  BIOS release:  %s\n",
            GetStringByIndex(pHeader, pUnion->bios.biosReleaseDateStrIx));
        returnCode = GOOD;
        break;

    case TYPE_SYSTEM_INFORMATION:
        printf("System Information\n");
        printf("  Manufacturer:  %s\n",
            GetStringByIndex(pHeader, pUnion->system.manufacturerStrIx));
        printf("  Product Name:  %s\n",
            GetStringByIndex(pHeader, pUnion->system.productNameStrIx));
        printf("  Version:       %s\n",
            GetStringByIndex(pHeader, pUnion->system.versionStrIx));
        printf("  Serial Number: %s\n",
            GetStringByIndex(pHeader, pUnion->system.serialNumberStrIx));
        returnCode = GOOD;
        break;

    case TYPE_BASE_BOARD_INFORMATION:
        printf("Base Board Information\n");
        printf("  Manufacturer:  %s\n",
            GetStringByIndex(pHeader, pUnion->board.manufacturerStrIx));
        printf("  Product:       %s\n",
            GetStringByIndex(pHeader, pUnion->board.productNameStrIx));
        printf("  Version:       %s\n",
            GetStringByIndex(pHeader, pUnion->board.versionStrIx));
        printf("  Serial Number: %s\n",
            GetStringByIndex(pHeader, pUnion->board.serialNumberStrIx));
        printf("  Asset Tag:     %s\n",
            GetStringByIndex(pHeader, pUnion->board.assetTagStrIx));
        printf("  Location:      %s\n",
            GetStringByIndex(pHeader, pUnion->board.chassisLocationStrIx));
        returnCode = GOOD;
        break;

    default:
        break;
    }

    return returnCode;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
