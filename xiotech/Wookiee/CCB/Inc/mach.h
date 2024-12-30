/* $Id: mach.h 143020 2010-06-22 18:35:56Z m4 $ */
/*===========================================================================
** FILE NAME:       mach.h
** MODULE TITLE:    Header file for mach.c
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _MACH_H_
#define _MACH_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
typedef volatile struct MACH_STRUCT
{
    UINT8       systemStatus0;  /* Read / (Write to clear bit 0)    */
    UINT8       diagSwitchesStatus;     /* Read only    */
    UINT8       flashSwitchesStatus;    /* Read only    */
    UINT8       boardMachRevStatus;     /* Read only    */
    UINT8       frontPanelControl;      /* Read / Write */
    UINT8       miscControl;    /* Read / Write */
    UINT8       heartbeatToggleControl; /* Read / Write */
    UINT8       watchDogReTriggerControl;       /* Read / Write */
} MACH;

#define MACH_NUM_REGS           8       /* Number of Registers        */

/* frontPanelControl register bit definitions */
#define REG_FPLEDS_HEARTBEAT    (1 << 0)        /* heartbeat - read only     */
#define REG_FPLEDS_COMM_FAULT   (1 << 4)        /* communication fault       */
#define REG_FPLEDS_OFFLINE      (1 << 5)        /* controller offline        */
#define REG_FPLEDS_ATTENTION    (1 << 6)        /* controller attention      */

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern UINT32 SuicideDisableSwitch(void);
extern UINT32 DiagPortsEnableSwitch(void);
#ifdef PAM
extern UINT32 PAMHeartbeatDisableSwitch(void);
#endif  /* PAM */

/*****************************************************************************
** Public variables
*****************************************************************************/
extern MACH *const mach;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _MACH_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
