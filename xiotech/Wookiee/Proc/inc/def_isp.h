/* $Id: def_isp.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       def_isp.h
**
**  @brief      Header file for def_isp.c
**
**  Copyright (c) 2003-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DEF_ISP_H_
#define _DEF_ISP_H_

#include "XIO_Types.h"
#include "MR_Defs.h"

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT8 DI_GetDeviceList(MR_PKT* pMRP);
extern UINT8 DI_GetPortList(MR_PKT* pMRP);
extern UINT8 DI_PortStats(MR_PKT* pMRP);
extern UINT8 DI_LoopPrimitive(MR_PKT* pMRP);
extern UINT8 DI_ResetPort(MR_PKT* pMRP);
extern UINT8 DI_SetPortEventNotification(MR_PKT* pMRP);
extern UINT8 DI_SetPortConfig(MR_PKT *);
extern void WaitPortConfig(void);

#ifdef FRONTEND
extern UINT8 DI_FEPortGo(MR_PKT *pMRP);
#endif /* FRONTEND */

#endif /* _DEF_ISP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
