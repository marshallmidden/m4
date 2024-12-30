/* $Id: L_Signal.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       xk_signal.h
**
**  @brief      Header for the module to allow us to handle signals.
**
**  Copyright (c) 2004-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _L_SIGNAL_H_
#define _L_SIGNAL_H_

/* XIOtech includes */
#include "XIO_Types.h"

/* Linux includes */
#include <signal.h>

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define  L_SIGNAL_MAX_SIGNALS      64 /* Max signals we will handle */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
#ifndef LOG_SIMULATOR
typedef void (*XKSIGHNDLR)(INT32 sigNo, UINT32 data, siginfo_t *, UINT32);
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
#ifndef LOG_SIMULATOR
extern INT32 L_SignalHandlerAdd(INT32 sigNo, XKSIGHNDLR func, bool mask);
extern INT32 L_SignalProcess(INT32 pid, INT32 sigNo, UINT32 sigData);
#endif

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _L_SIGNAL_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
