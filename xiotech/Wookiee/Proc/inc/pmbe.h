/* $Id: pmbe.h 157461 2011-08-03 15:21:36Z m4 $ */
/**
******************************************************************************
**
**  @file       pmbe.h
**
**  @brief      Packet Management for the BE
**
**  Copyright (c) 2003-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _PMBE_H_
#define _PMBE_H_

#include "XIO_Types.h"

#include "ficb.h"
#include "ilt.h"
#include "prp.h"
#include "rpn.h"
#include "rrp.h"

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void PM_RelILTPRPSGL(ILT* pILT);
extern ILT* PM_AllocILTRRPW(RRP **ppRRP);

#endif /* _PMBE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
