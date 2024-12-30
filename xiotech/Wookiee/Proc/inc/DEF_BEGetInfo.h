/* $Id: DEF_BEGetInfo.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       DEF_BEGETINFO.h
**
**  @brief      Back end fetch type structure
**
**  Copyright (c) 2004-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DEF_BEGETINFO_H_
#define _DEF_BEGETINFO_H_

#include "globalOptions.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

struct MR_PKT;
struct VDD;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT32 DEF_GetMData(struct MR_PKT *pMRP);
extern UINT32 DEF_GetEData(struct MR_PKT *pMRP);
extern UINT32 DEF_GetPData(struct MR_PKT *pMRP);
extern UINT32 DEF_GetRAIDDeviceData(struct MR_PKT *pMRP);
extern UINT32 DEF_GetVirtualData(struct MR_PKT *pMRP);
extern UINT32 DEF_GetExtendVirtualData (struct MR_PKT *pMRP);
extern UINT32 DEF_GetMList(struct MR_PKT *pMRP);
extern UINT32 DEF_GetEList(struct MR_PKT *pMRP);
extern UINT32 DEF_GetPList(struct MR_PKT *pMRP);
extern UINT32 DEF_GetRList(struct MR_PKT *pMRP);
extern UINT32 DEF_GetSList(struct MR_PKT *pMRP);
extern UINT32 DEF_GetVList(struct MR_PKT *pMRP);
extern UINT32 DEF_GetTList(struct MR_PKT *pMRP);

extern UINT32 DEF_GetVdiskRedundancy(struct MR_PKT *pMRP);
extern UINT32 DEF_IsBayRedundant(struct VDD *);

#endif /* _DEF_BEGETINFO_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
