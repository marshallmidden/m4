/* $Id: L_CCBCrashDump.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       L_CCBCrashDump.h
**
**  @brief      Crash dump definitions.
**
**  Crash dump, summarization and rotation definitions.
**
**  Copyright (c) 2005-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _L_CCBCRASHDUMP_H_
#define _L_CCBCRASHDUMP_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void L_CCBCrashDump(int sig, siginfo_t * sinfo, UINT32 ebp);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _L_CCBCRASHDUMP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
