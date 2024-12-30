/* $Id: deffe.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       deffe.h
**
**  @brief      FE Define operations
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DEFFE_H_
#define _DEFFE_H_

#include "XIO_Types.h"
#include "XIO_Const.h"

#include "sdd.h"
#include "target.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
extern TGD*   T_tgdindx[MAX_TARGETS];

extern SDD*   S_sddindx[MAX_SERVERS];

extern SDX    gSDX;
extern TDX    gTDX;

#endif /* _DEFFE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
