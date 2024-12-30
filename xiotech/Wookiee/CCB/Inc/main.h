/* $Id: main.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       main.h
** MODULE TITLE:    Header file for main.c
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _MAIN_H_
#define _MAIN_H_

#include "XIO_Std.h"
#include "XIO_Types.h"
#include "slink.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*****************************************************************************
** FUNCTION NAME: Start
**
** PARAMETERS:  None
**
** DESCRIPTION: Misc system level task initialization
**
******************************************************************************/
void        Start(TASK_PARMS *startParms);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _MAIN_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
