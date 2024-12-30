/* $Id: serial.h 122127 2010-01-06 14:04:36Z m4 $ */
/*===========================================================================
** FILE NAME:       serial.h
** MODULE TITLE:    Header file for serial.c
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef SERIAL_H
#define SERIAL_H

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

#define TRANSMIT                1
#define RECEIVE                 2

/* ASCII characters */
#define BELL    0x07
#define BKSP    0x08
#define CRLF    0x0D
#define SPACE   0x20
#define TILDE   0x7E
#define DEL     0x7F

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*****************************************************************************
** FUNCTION NAME: SerialInit
**
** PARAMETERS:  None
**
** DESCRIPTION: Initialize and enable both channels to known state
**
** RETURNS:     GOOD
**              ERROR
******************************************************************************/
extern UINT32 SerialInit(void);

/*****************************************************************************
** FUNCTION NAME: SerialWrite
**
** PARAMETERS:  *buffer
**              length
**
** DESCRIPTION: Write 'length' number of bytes to the port.
**
** RETURNS:     GOOD
******************************************************************************/
extern UINT32 SerialWrite(const char *, UINT32);

/*****************************************************************************
** FUNCTION NAME: SerialPutChar
**
** PARAMETERS:  writeChar
**
** DESCRIPTION: Transmit a single character
**
** RETURNS:     GOOD
**              ERROR
******************************************************************************/
extern UINT32 SerialPutChar(char);

/*****************************************************************************
** Public variables
*****************************************************************************/
extern INT32 userPortHandle;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* SERIAL_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
