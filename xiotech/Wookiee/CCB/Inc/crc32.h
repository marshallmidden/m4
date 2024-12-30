/* $Id: crc32.h 122127 2010-01-06 14:04:36Z m4 $ */
/*===========================================================================
** FILE NAME:       crc32.h
** MODULE TITLE:    Header file for crc32.c
**
** DESCRIPTION:     The functions in this module are used for CRC'ing the
**                  firmware images, as well as some miscellaneous functions
**                  that are also related to firmware images.
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _CRC32_H_
#define _CRC32_H_

#include "FW_Header.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/**********************************************************************
** FUNCTION NAME: CRC32
**
** PARAMETERS:  *buffer - Starting address where the CRC algorithm
**                        is to begin
**              length - Number of bytes to include in the CRC
**
** DESCRIPTION: Calculate the CRC value
**
** RETURNS:     The CRC value calculated over the given range
**********************************************************************/
extern UINT32 CRC32(void *buffer, UINT32 length);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _CRC32_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
