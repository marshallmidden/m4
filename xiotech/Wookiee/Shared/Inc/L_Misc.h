/* $Id: L_Misc.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       L_Misc.h
**
**  @brief      Miscellaneous Linux Usability Functions.
**
**  Copyright (c) 2004-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef _L_MISC_H_
#define _L_MISC_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern volatile char *L_LinuxSignalToString(INT32 sigNo, UINT32 data);
extern INT32 SetCleanShutdown(void);
extern INT32 GetCleanShutdown(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _L_MISC_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
