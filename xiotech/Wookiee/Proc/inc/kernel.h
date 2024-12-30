/* $Id: kernel.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       kernel.h
**
**  @brief      Header file for items in kernel.as.
**
**  @date       02/22/2002
**
**  Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "ficb.h"
#include "OS_II.h"
#include "pcb.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Public variables
******************************************************************************
*/

extern FICB  *K_ficb;
extern II     K_ii;
extern UINT32 K_poffset;
extern PCB   *K_xpcb;
extern PCB   *K_pcborg;
extern UINT32 kernel_sleep;
extern UINT32 kernel_up_counter;
extern UINT32 kernel_switch_counter;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
// #ifndef PERF
// extern void CheckTasksOK(void);
// #endif  /* PERF */

#endif /* _KERNEL_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
