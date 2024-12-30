/* $Id: def_lun.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       def_lun.h
**
**  @brief      Define functions Pertaining to LUN mappings
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DEF_LUN_H_
#define _DEF_LUN_H_

#include "XIO_Types.h"

#include "def.h"
#include "MR_Defs.h"
#include "sdd.h"

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define DL_ExtractDCN(x)    ((x) & 0xF)

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT8 DL_ClearLUNMap(MR_PKT* pMRP);
extern void DL_DeleteLVM(SDD* sdd);
extern UINT8 DL_SetLUNMap(MR_PKT* pMRP);
extern void DL_removeLnk(SDD * sdd);
extern UINT16 DL_insertLnk(UINT16 dsid, UINT16 ssid);
extern void DL_SetVDiskOwnership(void);
extern UINT32 DL_AmIOwner(UINT16 vid);
extern void *DEF_build_slink_structures( UINT64 devCap, UINT32 vid, UINT32 scorvid);

#endif /* _DEF_LUN_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
