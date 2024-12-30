/* $Id: xssa_structure.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       xssa_structure.h
** MODULE TITLE:    Header file for xssa_structure.c
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _XSSA_STRUCTURE_H_
#define _XSSA_STRUCTURE_H_

#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
typedef struct _XSSA_STRUCTURE
{
    UINT8       data[SIZE_64K - 1];     /* Minus one to make COFF file limit */
} XSSA_STRUCTURE;

/*****************************************************************************
** Public variables
*****************************************************************************/
#define XSSAData (*xssaData)
extern XSSA_STRUCTURE *xssaData;


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _XSSA_STRUCTURE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
