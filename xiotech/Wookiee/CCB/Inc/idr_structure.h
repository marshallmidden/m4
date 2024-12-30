/* $Id: idr_structure.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       idr_structure.h
** MODULE TITLE:    Header file for idr_structure.c
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IDR_STRUCTURE_H_
#define _IDR_STRUCTURE_H_

#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
typedef struct IDR_STRUCTURE_STRUCT
{
    /*
     ** Protected space for interrupt vector cache
     */
    void        (*cached_interrupt_vector_nmi)(void);
    void        (*cached_interrupt_vector_0x12)(void);
    void        (*cached_interrupt_vector_0x22)(void);
    void        (*cached_interrupt_vector_0x32)(void);
    void        (*cached_interrupt_vector_0x42)(void);
    void        (*cached_interrupt_vector_0x52)(void);
    void        (*cached_interrupt_vector_0x62)(void);
    void        (*cached_interrupt_vector_0x72)(void);
    void        (*cached_interrupt_vector_0x82)(void);
    void        (*cached_interrupt_vector_0x92)(void);
    void        (*cached_interrupt_vector_0xA2)(void);
    void        (*cached_interrupt_vector_0xB2)(void);
    void        (*cached_interrupt_vector_0xC2)(void);
    void        (*cached_interrupt_vector_0xD2)(void);
    void        (*cached_interrupt_vector_0xE2)(void);
    void        (*cached_interrupt_vector_0xF2)(void);

    /* Vector space for the user ISR handlers */
    void        (*nmi_handler)(void);
    void        (*xint0_handler)(void);
    void        (*xint1_handler)(void);
    void        (*xint2_handler)(void);
    void        (*xint3_handler)(void);
    void        (*xint4_handler)(void);
    void        (*xint5_handler)(void);
    void        (*xint6_handler)(void);
    void        (*xint7_handler)(void);
    void        (*xint_timer0_handler)(void);
    void        (*xint_timer1_handler)(void);

    /* Boot status flags */
    UINT32      bootStatus;

    /* Runtime code status flags */
    UINT32      runtimeStatus;

    /* Boot code background timer counter */
    UINT32      timerCounter;

    /* Size of DRAM attached to CCB */
    UINT32      sizeDRAM;

    /* Pointer to current PRCB table */
    UINT32      prcbPtr;

    /* IMSK register restore value used by the interrupt service routines */
    UINT32      imskRestore;

    /* Processor state (normal, fault, interrupt, nmi) */
    UINT32      processorState;

    /* Reset status flags */
    UINT32      resetStatus;
} IDR_STRUCTURE;

/******
** IDRData status flag macros and definitions
******/

/* Soft reset detection pattern */
#define IDR_SOFT_RUNTIME_ERROR_PATTERN  0xABACADAB
#define IDR_CONTROLLER_SUICIDE_PATTERN  0xDEADDEAD

/* Bit definitions for the bootStatus and runtimeStatus words */
#define IDR_SOFT_RESET                  (1 << 0)
#define IDR_WATCHDOG_RESET              (1 << 1)
#define IDR_DIAG_JUMPER                 (1 << 2)
#define IDR_SERIAL_INIT                 (1 << 3)
#define IDR_ETHERNET_INIT               (1 << 4)
#define IDR_TIMER_INIT                  (1 << 5)
#define IDR_I2C_INIT                    (1 << 6)
#define IDR_COORDINATED_BOOT_GOOD       (1 << 7)
#define IDR_21555_BRIDGE_PRESENT        (1 << 8)
#define IDR_FE_BOOT_CODE                (1 << 9)
#define IDR_FE_DIAG_CODE                (1 << 10)
#define IDR_FE_RUNTIME_CODE             (1 << 11)
#define IDR_BE_BOOT_CODE                (1 << 12)
#define IDR_BE_DIAG_CODE                (1 << 13)
#define IDR_BE_RUNTIME_CODE             (1 << 14)
#define IDR_UPDATE_TBOLT_CODE           (1 << 15)
#define IDR_FULL_RANGE_DRAM_TEST        (1 << 16)
#define IDR_RUNNING_FROM_DRAM           (1 << 17)
#define IDR_NVRAM_BATTERY_GOOD          (1 << 18)
#define IDR_FE_QL2200_SINGLE_CODE       (1 << 19)
#define IDR_BE_QL2200_SINGLE_CODE       (1 << 20)
#define IDR_FE_QL2300_SINGLE_CODE       (1 << 21)
#define IDR_BE_QL2300_SINGLE_CODE       (1 << 22)
#define IDR_FE_QL2200_MULTI_CODE        (1 << 23)
#define IDR_FE_QL2300_MULTI_CODE        (1 << 24)
#define IDR_DISABLE_NMI_TRACE           (1 << 25)

/* Value definitions for the processorState field */
#define IDR_PROCESSOR_STATE_DEAD_LOOP  -1
#define IDR_PROCESSOR_STATE_NORMAL      0
#define IDR_PROCESSOR_STATE_FAULT       1
#define IDR_PROCESSOR_STATE_INTERRUPT   2
#define IDR_PROCESSOR_STATE_NMI         3

/*
** Status bitfield pointers:
**   Boot code looks at IDRData.bootStatus
**   Runtime code looks at IDRData.runtimeStatus
*/
#define IDR_STATUS_FLAGS        IDRData.runtimeStatus

/*
** IDR_SERIAL_INIT
*/
#define SetSerialInitFlag(status)                                           \
    ( ((status) == TRUE) ?                                                  \
      (IDR_STATUS_FLAGS |= IDR_SERIAL_INIT) :                               \
      (IDR_STATUS_FLAGS &= ~IDR_SERIAL_INIT) )

#define TestSerialInitFlag( )                                               \
    ( (IDR_STATUS_FLAGS & IDR_SERIAL_INIT) ?                                \
      (TRUE) : (FALSE) )

/*
** IDR_COORDINATED_BOOT_GOOD
*/
#define TestCoordinatedBootGoodFlag( )                                      \
    ( (IDR_STATUS_FLAGS & IDR_COORDINATED_BOOT_GOOD) ?                      \
      (TRUE) : (FALSE) )

/*
** IDR_RUNNING_FROM_DRAM
*/
#define SetRunningFromDRAMFlag(status)                                      \
    ( (status == TRUE) ?                                                    \
      (IDR_STATUS_FLAGS |= IDR_RUNNING_FROM_DRAM) :                         \
      (IDR_STATUS_FLAGS &= ~IDR_RUNNING_FROM_DRAM) )


/*****
** Processor state macros
*****/
#define SetProcessorState(state)                                            \
    ( IDRData.processorState = (state) )

#define GetProcessorState( )                                                \
    ( IDRData.processorState )

/*****************************************************************************
** Public variables
*****************************************************************************/
extern IDR_STRUCTURE IDRData;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IDR_STRUCTURE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
