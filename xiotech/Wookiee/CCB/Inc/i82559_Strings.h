/* $Id: i82559_Strings.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       i82559_Strings.h
** MODULE TITLE:    Header file for i82559_Strings.c
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _I82559_STRINGS_H_
#define _I82559_STRINGS_H_

#include "i82559.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: EthernetGetLinkStatusString
**
**  Comments:
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 stateNumber - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**
**  Returns:       Nothing
**--------------------------------------------------------------------------*/
extern void EthernetGetLinkStatusString(char *stringPtr, LINK_STATUS stateNumber,
                                        UINT8 stringLength);

/*----------------------------------------------------------------------------
**  Function Name: EthernetGetWireSpeedString
**
**  Comments:
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 stateNumber - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**
**  Returns:       Nothing
**--------------------------------------------------------------------------*/
extern void EthernetGetWireSpeedString(char *stringPtr, WIRE_SPEED stateNumber,
                                       UINT8 stringLength);

/*----------------------------------------------------------------------------
**  Function Name: EthernetGetDuplexModeString
**
**  Comments:
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 stateNumber - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**
**  Returns:       Nothing
**--------------------------------------------------------------------------*/
extern void EthernetGetDuplexModeString(char *stringPtr, DUPLEX_MODE stateNumber,
                                        UINT8 stringLength);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _I82559_STRINGS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
