/* $Id: MP_ProcProto.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       MP_ProcProto.h
**
**  @brief      Mirror Partner Information Prototypes
**
**  To provide a common means of functon prototypes for mpi.c file
**
**  Copyright (c) 2004-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/

#ifndef _MP_PROCPROTO_H
#define _MP_PROCPROTO_H

#if 0
#include "XIO_Types.h"
#endif
#include "MR_Defs.h"

/*
** Function Prototypes
*/
#ifdef FRONTEND
extern UINT32 MP_SetMPConfigFE (MR_PKT* pMRP);
extern UINT32 MP_GetMPConfigFE (MR_PKT* pMRP);
#endif /*FRONTEND*/

#ifdef BACKEND
extern UINT32 MP_SetMPConfigBE (MR_PKT* pMRP);
#endif

#endif /*_MP_PROCPROTO_H*/

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
