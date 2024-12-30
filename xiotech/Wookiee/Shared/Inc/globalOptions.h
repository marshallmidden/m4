/* $Id: globalOptions.h 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       options.h
**
**  @brief      Build options
**
**  This contains conditional compiler options.
**  This file is a subset of options.inc - make sure the two files stay in sync.
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _GLOBALOPTIONS_H_
#define _GLOBALOPTIONS_H_

#include "XIO_Const.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define CM_IM_DEBUG               FALSE
#define GR_MYDEBUG1               FALSE
#define GR_GEORAID15_DEBUG        FALSE

#define ISCSI_CODE                TRUE
#ifdef FRONTEND
#define FE_ISCSI_CODE             ISCSI_CODE
#else
#define FE_ISCSI_CODE             FALSE
#endif

#ifdef FRONTEND
#define FE_ICL                    FALSE
#define ICL_DEBUG                 FALSE  /* Enable or Disable  Debug messages for ICL port */
#else  /* BACK END     */
#define FE_ICL                    FALSE
#define ICL_DEBUG                 FALSE  /* Enable or Disable Debug messages for ICL port */
#endif

#if defined(MODEL_7000) || defined(MODEL_7400)
#define DISABLE_LOCAL_RAID_MONITORING
#endif  /* defined(MODEL_7000) || defined(MODEL_7400) */
#if defined(MODEL_7000) || defined(MODEL_4700)
#define DISABLE_WRITE_SAME  1
#endif /* MODEL_7000 || MODEL_4700 */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _GLOBALOPTIONS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
