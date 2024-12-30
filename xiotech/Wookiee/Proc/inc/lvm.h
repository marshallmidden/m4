/* $Id: lvm.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       lvm.h
**
**  @brief      LUN to virtual disk ID mapping
**
**  To provide a common means of defining the mapping of a LUN to a
**  virtual disk.
**
**  Copyright (c) 1996-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _LVM_H_
#define _LVM_H_

#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef struct LVM
{
    struct LVM* nlvm;       /* Next LVM entry               */
    UINT16      lun;        /* Logical unit number          */
    UINT16      vid;        /* Virtual disk ID              */
    UINT8       rsvd8[8];   /* Reserved                     */
                            /* QUAD BOUNDARY                    *****/
} LVM;

#endif /* _LVM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
