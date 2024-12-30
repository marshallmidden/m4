/* $Id: xci_structure.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       xci_structure.h
** MODULE TITLE:    XCI Structure definition
**
** DESCRIPTION:     Xiotech Component Information definition
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _XCI_STRUCTURE_H_
#define _XCI_STRUCTURE_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/**************************************
** I2C - SDIMM Serial Presence Detect (extended)
** NOTE: SPD data is defined by JEDEC industry standard
**       XCI data uses manufacturer specific fields for Xiotech devices
**************************************/
typedef struct XCI_PART_NUMBER_STRUCT
{
    char        modulePartNumber[7];            /* Bytes 73-79  */
    char        moduleDashNumber[4];            /* Bytes 80-83  */
    UINT8       moduleRevisionLetters[2];       /* Byte  84-85  */
    UINT8       reserved[5];                    /* Bytes 86-90  */
} XCI_PART_NUMBER;

typedef struct XCI_XIOTECH_SPECIFIC_DATA_STRUCT
{
    UINT8       reserved[23];   /* Bytes 99-121 (Can include 3rd Party text) */
    UINT32      crc;            /* Bytes 122-125 */
} XCI_XIOTECH_SPECIFIC_DATA;

/* Extended SPD data is defined by Xiotech */
typedef struct XCI_DATA_STRUCT
{
    UINT8       manuJedecId[8];                 /* Bytes 64-71  */
    UINT8       manuLocation;                   /* Byte  72     */
    XCI_PART_NUMBER manuPartNumber;             /* Bytes 73-90  */
    UINT8       revisionCode[2];                /* Bytes 91-92  */
    UINT8       manuYear;                       /* Byte  93     */
    UINT8       manuWeek;                       /* Byte  94     */
    UINT32      asmSerialNumber;                /* Bytes 95-98  */
    XCI_XIOTECH_SPECIFIC_DATA manuSpecificData; /* Bytes 99-125 */
    UINT8       vendorSpecific[2];              /* Bytes 126-127 */
} XCI_DATA;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _XCI_STRUCTURE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
