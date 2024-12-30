/* $Id: xk_init.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       xk_init.h
**
**  @brief      Header for the module to initialize the CCB on Linux.
**
**  Copyright (c) 2004-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _XK_INIT_H_
#define _XK_INIT_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern INT32 pamPid;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void XK_Init(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _XK_INIT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
