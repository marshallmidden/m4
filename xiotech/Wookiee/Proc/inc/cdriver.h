/* $Id: cdriver.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       cdriver.h
**
**  @brief      CDriver header file
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _CDRIVER_H_
#define _CDRIVER_H_

#include "XIO_Types.h"

#include "cimt.h"
#include "ilt.h"
#include "imt.h"
#include "target.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
extern CIMT *cimtDir[MAX_CIMT];

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern TAR* CD_FindTarget(UINT8 virtualPortID);

#endif /* _CDRIVER_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
