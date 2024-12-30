/* $Id: codeburn.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       codeburn.h
** MODULE TITLE:    Header file for codeburn.c
**
** DESCRIPTION:     Contains the prototype definitions and data to support
**                  the code burn functions.
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _CODEBURN_H_
#define _CODEBURN_H_

#include "ccb_flash.h"
#include "FW_Header.h"
#include "XIO_Types.h"
#include "FIO.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define FW_COMPAT_DATA_SCHEMA   0x31435746      /* FWC1 -- FW Compat 1 */

/*****************************************************************************
** Public variables
*****************************************************************************/
typedef struct
{
    UINT16      rc;
    UINT16      status;
} RC_STATUS;

typedef struct
{
    UINT32      schema;
    FW_HEADER   ccbHdr;
    UINT8       reserved[BLOCK_SZ - sizeof(UINT32) - sizeof(FW_HEADER) - sizeof(UINT32)];
    UINT32      crc;
} FW_COMPAT_DATA;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: UpdateCode
**
**  Comments:  Validates the firmware header and then updates the flash code on
**             the CCB board, at the location and for the device  described in
**             the firmware header.
**
**  Parameters: fwPtr - dram pointer where firmware header resides.
**
**  Returns:    0   - GOOD Completion
**              !0  - Error
**
**--------------------------------------------------------------------------*/
extern INT32 UpdateCode(FW_HEADER * fwPtr);

/*----------------------------------------------------------------------------
**  Function Name: SCSIWriteBufferMode5 & UpdateEurologicBaySingle
**
**  Comments:  Send the code down to a Drive or Drive Bay for it to burn its
**             own flash.
**
**  Parameters: fwPtr - dram pointer where firmware header resides.
**              wwn - device World Wide Name
**              lun - device Lun
**
**  Returns:    0   - GOOD Completion
**              !0  - Error
**
**--------------------------------------------------------------------------*/
extern RC_STATUS SCSIWriteBufferMode5(FW_HEADER * fwPtr, UINT64 wwn, UINT32 lun);
extern RC_STATUS UpdateEurologicBaySingle(FW_HEADER * fwPtr, UINT64 wwn, UINT32 lun);
extern RC_STATUS UpdateAdaptecSataBaySingle(FW_HEADER * fwPtr, UINT64 wwn, UINT32 lun);
extern RC_STATUS UpdateXyratexBay(FW_HEADER * fwPtr, UINT64 wwn, UINT32 lun);

/*----------------------------------------------------------------------------
**  Function Name: ValidateFWHeader()
**
**  Parameters: fwPtr - pointer to the FW header to verify
**
**  Returns:    0   - GOOD Completion
**              !0  - Error - reason code returned
**
**--------------------------------------------------------------------------*/
extern INT32 ValidateFWHeader(FW_HEADER * fwPtr);

/*----------------------------------------------------------------------------
**  Function Name: TargetName
**
**  Comments:   Return a target name based upon target ID
**
**  Parameters: target ID
**
**  Returns:    Pointer to descriptive string
**
**--------------------------------------------------------------------------*/
extern const char *TargetName(UINT32 target);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _CODEBURN_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
