/* $Id: qcb.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       qcb.h
**
**  @brief      Queue Control Block
**
**  Copyright (c) 1996-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _QCB_H_
#define _QCB_H_

#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

typedef struct QCB
{
    UINT32 * begin;                 /* Org of circular queue ptr            */
    UINT32 * in;                    /* Insert pointer                       */
    UINT32 * out;                   /* Remove pointer                       */
    UINT32 * end;                   /* End of circular que + 1              */
} QCB;

#endif /* _QCB_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
