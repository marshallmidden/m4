/* $Id: li_evt.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**  @file       li_evt.h
**
**  @brief      Definitions for Linux Interface for event operations
**
**  Definitions for Linux interface code for event operations.
**
** Copyright 2004-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef __LI_EVT_H__
#define __LI_EVT_H__

#include <XIO_Types.h>

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
**  Public function prototypes
******************************************************************************
*/
#ifndef CCB_RUNTIME_CODE
extern void    LI_SchedIRQ(unsigned long active);
#endif  /* !CCB_RUNTIME_CODE */
extern void    LI_RegisterEvent(int evt, void (*f)(UINT32), UINT32 val);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif  /* __LI_EVT_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
