/* $Id: ss_comp.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       ss_comp.h
**
**  @brief      Definitions for snapshot completion routines.
**
**  Copyright (c) 2008-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _SS_COMP_H_
#define _SS_COMP_H_
#include "XIO_Std.h"
#include "XIO_Macros.h"

struct ILT;
struct QU;
struct SSMS;
struct VDD;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern void create_snapshot_completers(void);
extern void q2_completer_task(struct ILT *, struct QU *);
extern void q2_snapshot_completer(UINT32, struct ILT *);
extern void ss$comp_worker(UINT32 dummy0, UINT32 dummy1, int);
extern void CT_LC_ss$comp_worker(UINT32 dummy0, UINT32 dummy1, int);
extern void ss_rcomp(struct ILT *);
extern void ss_srccomp(UINT32, struct ILT *);
extern void CT_LC_ss_srccomp(UINT32, struct ILT *);
extern void ss_splcomp(UINT32, struct ILT *);
extern void CT_LC_ss_splcomp(UINT32, struct ILT *);
extern void ss_splsrccomp(UINT32, struct ILT *);
extern void CT_LC_ss_splsrccomp(UINT32, struct ILT *);
extern void CT_LC_q2_snapshot_completer(UINT32, struct ILT *);
extern void ss_wcomp(struct ILT *);
extern void compute_ss_spareseness(struct SSMS *);
extern int  remove_ssms_from_src_vdd(struct SSMS *, struct VDD *);
extern int ss_clr_seg_bit(UINT32, struct SSMS *);
extern void print_seg_clear_error(UINT32, struct SSMS *, int, const char *, const char *, unsigned int);

#endif  /* _SS_COMP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
