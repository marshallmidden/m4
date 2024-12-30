/* $Id: nvram.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       nvram.h
**
**  @brief      Handles the NVRAM configuration
**
**  Copyright (c) 2003-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _NVRAM_H_
#define _NVRAM_H_

#include "XIO_Types.h"
#include "nvr.h"

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern UINT8   gOrphanLogged[MAX_RAIDS];

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void NV_BuildLocalImage(NVRL* pLocalImage);
extern UINT32 NV_CalcLocalImageSize(void);
extern UINT32 NV_GetLclImageSize(NVRL* pLocalImage);
extern void NV_GetVCGID(NVRII* pNvramImage);
extern UINT8 NV_P2ChkSumChk(NVRII* pNvramImage);
extern void NV_P2UpdateNvram(void);
extern void NV_ProcessFSys(NVRFSYS* pReport, UINT8 master);
extern void NV_RefreshNvram(NVRII* pNvramImage);
extern void NV_ReorderPDDs(NVRII* nvramImage);
extern void NV_RestoreNvram(NVRII* pNvramImage, PDX* pPDX, UINT8 initialLoad, UINT8 restartCopies, UINT32 caller);
extern void NV_SendFSys(UINT16 master);
extern void NV_UpdateLocalImage(NVRL* pLocalImage);
extern void is_nvram_p2_initialized(void);

/* extern void DEF_Slink_Delete(UINT32 i); */
extern void DEF_Slink_Delete(VDD *pvDD);

/*
** These functions are currently in assembly
*/
extern void  NV_P2Update(void);
extern void  NV_P2UpdateConfig(void);
extern void NV_SendRefresh(void);

#endif /* _NVRAM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
