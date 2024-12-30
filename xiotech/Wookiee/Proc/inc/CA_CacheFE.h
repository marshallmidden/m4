/* $Id: CA_CacheFE.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       CA_CacheFE.h
**
**  @brief      Cachefe related function prototypes and Macro definitions
**              for CA_CacheFE.c
**
**  Copyright (c) 2004-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _CA_CACHEFE_H_
#define _CA_CACHEFE_H_

#include "CA_CI.h"
#include "MR_Defs.h"
#include "pcb.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"

/*
********************************************************************************
** Public defines - constants
********************************************************************************
*/
#define  GLOBAL_CACHE_ENABLE 0xFFFFFFFF

/*
********************************************************************************
** Public function prototypes
********************************************************************************
*/

extern UINT32 CA_AcceptMPChange (UINT32 serialNo);
extern void   CA_SetMirrorPartnerFE_1 (UINT32 newMPSerialNo);
extern UINT32 CA_SetMirrorPartnerFE_2 (void);

extern UINT8 CA_SetTempDisableWC(MR_PKT* pMRP);
extern UINT8 CA_ClearTempDisableWC(MR_PKT* pMRP);
extern UINT8 CA_QueryTDisableDone(MR_PKT* pMRP);

extern UINT32 CA_CheckOpsOutstanding(void);

#endif /* _CA_CACHEFE_H_*/

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
