/* $Id: error_handler.h 143766 2010-07-06 12:06:32Z m4 $ */
/*============================================================================
** FILE NAME:       error_handler.c
** MODULE TITLE:    Header file for error_handler.c
**
** DESCRIPTION:     The functions in this module are used for handling
**                  unexpected hardware errors.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "led.h"
#include "nvram_structure.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/********************************************************************
** Begin error codes used in the boot code base
********************************************************************/
#define EVENT_MFG_CLEAN_DONE    0x22    /* MFG clean completed succesfully  */

/********************************************************************
** Error codes
*********************************************************************
** COMMON    - LEVEL E EVENT CODES
**           - Fault / Error Codes
*/
#define EVENT_FAULT0            0xE0    /* parallel/override fault 0 was received   */
#define EVENT_FAULT1            0xE1    /* trace fault 1 was received               */
#define EVENT_FAULT2            0xE2    /* operation fault 2 was received           */
#define EVENT_FAULT3            0xE3    /* arithmetic fault 3 was received          */
#define EVENT_RES_FAULT         0xE4    /* one of the reserved faults was received  */
#define EVENT_FAULT5            0xE5    /* constraint fault 5 was received          */
#define EVENT_FAULT7            0xE7    /* protection fault 7 was received          */
#define EVENT_DRAM_ECC_FAILED   0xE9    /* DRAM failed ECC multi-bit error          */
#define EVENT_FAULTA            0xEA    /* type fault A was received                */
#define EVENT_FATAL_NMI         0xEB    /* Unrecoverable NMI occurred               */
#define EVENT_DRAM_FAILED       0xEC    /* DRAM failed a test                       */
#define EVENT_TRAP_STACK        0xED    /* trap stack pointer in procedures table   */
#define EVENT_UNHANDLED_NMI     0xEE    /* NMI ocurred without NMI handler declared */
#define EVENT_UNEXP_IRQ         0xEF    /* An unexpected irq was received           */

/*
** COMMON    - LEVEL F EVENT CODES
**           - Types of controller suicides
*/
#define EVENT_CONTROLLER_SUICIDE    0xF1        /* This CCB is to remain offline (generic)      */
#define EVENT_MANUAL_SUICIDE        0xF2        /* Manual suicide: This CCB is to remain offline */
#define EVENT_BAD_FREE              0xF3        /* double free() or bad fence detected          */
#define EVENT_MISSED_PCI_HEARTBEAT  0xF4        /* Intra-controller heartbeat failure           */
#define EVENT_QUORUM_FAILURE        0xF5        /* Quorum I/O failure                           */
#define EVENT_CONTROLLER_FAILED     0xF6        /* Controller read FAILED state                 */
#define EVENT_IPC_OFFLINE           0xF7        /* IPC communications are offline               */
#define EVENT_FORCED_OFFLINE        0xF8        /* Controller was forced offline                */
#define EVENT_ELECTION_FAILED       0xF9        /* Controller failed as a result of an election */
#define EVENT_ASSERT_FAILED         0xFA        /* assert() failure                             */
#define EVENT_DISASTER              0xFB        /* Controller failed as a result of an disaster */

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern ERRORTRAP_DATA_CPU_REGISTERS cpuRegisters;

extern NORETURN void error08(void);
extern NORETURN void error11(void);
extern NORETURN void DeadLoop(UINT8 newErrorCode, UINT8 forceControllerSuicide);
extern void DeadLoopInterrupt(INT32 sig, UINT32 data, siginfo_t * sinfo, UINT32 ebp);

/***
** Modelines
** vi:sw=4 ts=4 expandtab
**/

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* ERROR_HANDLER_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
