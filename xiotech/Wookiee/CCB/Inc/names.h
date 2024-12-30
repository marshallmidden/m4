/* $Id: names.h 143845 2010-07-07 20:51:58Z mdr $*/
/*============================================================================
** FILE NAME:       names.h
** MODULE TITLE:    Header file for names.c
**
** DESCRIPTION:     The old names.h was renamed to names_old.h
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _NAMES_H_
#define _NAMES_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** The Bigfoot processor requests names from the CCB via async events.
** The defines below are for the lengths of these name strings.
*/
#define NAMES_CONTROLLER_LEN_MAX        20
#define NAMES_VDEVICE_LEN_MAX           52

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    ControllerNodeCluster_IsNameSet
**
** Description: Checks if the CNC name has been initialized.
**
** Inputs:      NONE
**
** Returns:     true if the name has been initialized, false othewise.
**
**--------------------------------------------------------------------------*/
extern bool ControllerNodeCluster_IsNameSet(void);

/*----------------------------------------------------------------------------
** Function:    ControllerNodeCluster_SetDefault
**
** Description: Set the controller node cluster default name (CNCnnnnnnnnnn).
**
** Inputs:      NONE
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
extern INT32 ControllerNodeCluster_SetDefaultName(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _NAMES_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
