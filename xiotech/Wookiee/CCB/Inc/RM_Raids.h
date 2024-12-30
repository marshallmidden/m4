/* $Id: RM_Raids.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       RM_Raids.h
**
**  @brief      Resource Manager Raids Management
**
**  To provide management for RAIDS at the CCB level.
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _RM_RAIDS_H_
#define _RM_RAIDS_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void RM_R5StripeResyncMonitorStart(void);
extern bool RM_R5StripeResyncInProgress(void);
extern void RM_R5PowerupReplacement(void);
extern void RM_R5PowerupProcessing(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _RM_RAIDS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
