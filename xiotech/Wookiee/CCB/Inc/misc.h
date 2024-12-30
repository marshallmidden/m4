/* $Id: misc.h 143845 2010-07-07 20:51:58Z mdr $ */
/**
******************************************************************************
**
**  @file   misc.h
**
**  @brief  Header file for misc.c
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _MISC_H_
#define _MISC_H_

#include "OS_II.h"
#include "SES_Structs.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "MR_Defs.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines - Macros
*****************************************************************************/

/*
** Translates addresses to/from PCI address given a processor offset.
**
** Use CalcPCIOffset to get the correct offset for a processor.
*/
#define ToPCIAddr(addr, offset)     ((char *)(addr)-(int)(offset))

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/* Print out the sizeof() a structure */
#define PrintSizeof(S) dprintf(DPRINTF_DEFAULT, "sizeof(" #S ") = 0x%X/%u bytes\n", sizeof(S), sizeof(S))

extern INT32 CalcPCIOffset(int proc);
extern void ProcessResetTask(TASK_PARMS *parms);
extern void ProcessReset(UINT32 which);

extern void dumpByteArray2(UINT32 dprintfLvl,
                           const char *name, void *theArray, int theArrayLength);

extern void LoadProcAddresses(void);
extern II  *GetProcAddress_BEII(void);
extern II  *GetProcAddress_FEII(void);
extern void *GetProcAddress_FENVA(void);
extern UINT32 GetLength_FENVA(void);

extern void LoadDeviceConfig(UINT16 *pCount, SES_DEV_INFO_MAP ** ppDevConfig, int shared);
extern void SaveDeviceConfig(UINT16 count, SES_DEV_INFO_MAP * pDevConfig);

extern UINT32 GetCPUCount(void);
extern void GetKernelVersion(char *pBuf, UINT32 len);

extern UINT32 BufferBoardShutdownControl(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _MISC_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
