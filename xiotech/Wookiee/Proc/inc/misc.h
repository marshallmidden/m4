/* $Id: misc.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       misc.h
**
**  @brief      'C' overlay for misc.as and msc.c
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _MISC_H_
#define _MISC_H_

#include "XIO_Types.h"

#include "ficb.h"
#include "ilt.h"
#include "options.h"

struct RRP;
struct PRP;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern UINT32 M_chk4XIO(UINT64);
extern UINT32 MSC_CRC32(void *pdata, UINT32 length);
extern UINT8  MSC_Mask2Prefix(UINT32 mask);
extern INT32  Copy_2_DMC(UINT32, void *);
extern void   Copy_Raid_DMC(void);
extern void   Copy_VDisk_DMC(void);
extern UINT64 DMC_bits;

#ifdef DEBUG_FLIGHTREC
extern void MSC_FlightRec(UINT32 parm0, UINT32 parm1, UINT32 parm2, UINT32 parm3);
#endif  /* DEBUG_FLIGHTREC */

extern void MSC_LogMessageRel(void *pLogEntry, UINT32 size);
extern void MSC_LogMessageStack(void *pLogEntry, UINT32 size);
extern int MSC_MemCmp(void *pSource1, void *pSource2, UINT32 length);
extern void MSC_NMIClear(void);
extern void MSC_QueHBeat(ILT *pILT);
extern void MSC_SoftFault(void *pData);
extern void M$que_hbeat(UINT32 dummy, ILT *pILT);

#ifdef BACKEND
extern void Process_DMC_delayed(void);
extern void Process_DMC(void);
#endif  /* BACKEND */

#ifndef PERF
extern void print_prp(struct PRP *);
extern void print_rrp(struct RRP *);
extern void print_nonzero_ilt(ILT *);
#endif  /* !PERF */

#endif /* _MISC_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
