/* $Id: ise.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       ise.h
**
**  Copyright (c) 2008-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _ISE_H_
#define _ISE_H_

#include "XIO_Types.h"
#include "globalOptions.h"
#include "XIO_Macros.h"
#include "prp.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef struct ISE_CDB_DATA1
{
    UINT8 opCode;
    UINT8 reserved;
    UINT8 page;
    UINT8 bayId;
} ISE_CDB_DATA1;

#endif /* ISE_H_*/

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
