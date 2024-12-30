/* $Id: DLM_fe.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       DLM_fe.h
**
**  @brief      Data Link Manager - Front End functions
**
**  To provide support for the Data-link Manager logic which
**  supports XIOtech Controller-to-XIOtech Controller functions
**  and services for Fibre communications.
**
**  Copyright (c) 2003-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DLM_FE_H_
#define _DLM_FE_H_

#include "XIO_Std.h"

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern void DLM_MirrorPartnerFECommAvailable(void);
extern UINT32 DLM$queryFEcomm(UINT32);

#endif /* _DLM_FE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
