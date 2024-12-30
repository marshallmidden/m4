/* $Id: timer.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file   timer.h
**
**  @brief  Header file for timer.c
**
**  Copyright (c) 2001-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef TIMER_H
#define TIMER_H

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
void    TimerEnable(void);
void    TimerDisable(void);
void    TimerWait(UINT32 counts);

/*****************************************************************************
** Public variables
*****************************************************************************/

extern volatile UINT32 K_t1cnt; /* Timer 1 count                            */
#ifndef K_timel
extern UINT32 K_timel;          /* Kernel time counter                      */
#endif

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* TIMER_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
