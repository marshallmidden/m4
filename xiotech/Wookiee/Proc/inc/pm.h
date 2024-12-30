/* $Id: pm.h 144582 2010-07-23 19:53:49Z mdr $ */
/**
******************************************************************************
**
**  @file       PM.h
**
**  @brief      Packet Management
**
**  Copyright (c) 2003-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _PM_H_
#define _PM_H_

#include "XIO_Types.h"
#include "ilt.h"
#include "sgl.h"
#include "vrp.h"

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern void PM_RelILTVRPSGL(ILT *pILT, VRP *pVRP);

extern SGL *m_asglbuf(UINT32 requestedBytes);
extern void PM_RelSGLWithBuf(SGL *pSGL);

extern void PM_ClearSGL(SGL *pSGL);
extern void PM_MoveSGL(SGL *pSourceSGL, SGL *pDestinationSGL);

#endif /* _PM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
