/* $Id: virtual.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       virtual.h
**
**  @brief      Virtual layer header file
**
**  Copyright (c) 2003-2008 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _VIRTUAL_H_
#define _VIRTUAL_H_

#include "XIO_Types.h"

#include "qu.h"
#include "ilt.h"
#include "vdd.h"

struct VRP;
struct VDD;
struct SGL;
struct ILT;

/*
******************************************************************************
** Public variables
******************************************************************************
*/

extern UINT32 V_orc;
extern QU   V_exec_qu;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern void V$que(UINT32 dummy, ILT *pILT);
extern void V_que(UINT32 dummy, ILT *pILT);
extern void call_comp_routine(UINT32, ILT*);
// NOTE: g0 and g1 are returns. Deal with it.
extern UINT32 v_callx(void *,UINT32,UINT32,void *,UINT32,UINT32,UINT32,struct VRP *,struct VDD *,struct SGL *,struct ILT *);
#endif /* _VIRTUAL_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
