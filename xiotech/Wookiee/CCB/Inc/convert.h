/* $Id: convert.h 126080 2010-02-03 22:59:28Z mdr $ */
/*============================================================================
** FILE NAME:       convert.h
** MODULE TITLE:    Header file for convert.c
**
** Copyright (c) 2001, 2009-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#ifndef _CONVERT_H_
#define _CONVERT_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif


/*****************************************************************************
** Public defines
*****************************************************************************/
#define C_to_F(x)   ((((x)*9)/5)+32)

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern unsigned char upper_case(unsigned char ascii_code);
extern unsigned short Binary2BCD(unsigned short num);
extern unsigned short BCD2Binary(unsigned short num);
extern UINT16 BCD2Short(UINT16 bcd);
extern UINT16 ShortToBCD(UINT16 num);
extern UINT32 atoh(char *, INT32 *);
extern UINT32 charval(INT8, char *);

/*----------------------------------------------------------------------------
**  Function:   strtoupper
**
**  Comments:   Converts a NULL terminated string to upper case.
**
**  Parameters: str - String to convert.
**
**  Returns:    pointer to str.
**
**  NOTES:      NONE.
**
**--------------------------------------------------------------------------*/
extern char *strtoupper(char *str);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _CONVERT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
