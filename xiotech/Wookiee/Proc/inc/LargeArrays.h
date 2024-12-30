/* $Id: LargeArrays.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       LargeArrays.h
**
**  @brief      Large arrays (those > 64K) allocated here.
**
**  A single place to define all arrays larger than 64k.  Since the compiler
**  throws a warning for large arrays this allows us to selectively ignore this
**  warning when compiling this file.
**
**  Copyright (c) 2004-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _LARGEARRAYS_H_
#define _LARGEARRAYS_H_

#include "XIO_Types.h"

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern UINT8 gProfileBuffer[];

#endif /* _LARGEARRAYS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
