/* $Id: ccb_hw.h 122127 2010-01-06 14:04:36Z m4 $ */
/*===========================================================================
**
** FILE NAME:       ccb_hw.h
** MODULE TITLE:    CCB hardware description header
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _CCB_HW_H_
#define _CCB_HW_H_

#include "XIO_Const.h"
#include "XIO_Types.h"

/*****************************************************************************
** Public defines
*****************************************************************************/
#define SIZE_NVRAM                              (SIZE_128K - 8) /* Minus 8 bytes protects clock registers */
#define SIZE_FLASH                              (SIZE_16MEG)

#include "FW_Header.h"
#include "LKM_layout.h"

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

#define CCB_RUNTIME_FW_HEADER_ADDR          ((UINT32)&fwHeader)
#define BASE_DRAM                           (CCB_PCI_START)

extern void HW_InitNVRAM(UINT8 *baseNVRAM);
extern void HW_InitFlash(UINT8 *baseFlash);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _CCB_HW_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
