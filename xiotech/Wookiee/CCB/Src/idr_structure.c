/* $Id: idr_structure.c 122127 2010-01-06 14:04:36Z m4 $ */
/*===========================================================================
** FILE NAME:       idr_structure.c
** MODULE TITLE:    i960 internal RAM useage
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "idr_structure.h"

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
IDR_STRUCTURE IDRData =
{
    NULL,                       /* cached_interrupt_vector_nmi  */
    NULL,                       /* cached_interrupt_vector_0x12 */
    NULL,                       /* cached_interrupt_vector_0x22 */
    NULL,                       /* cached_interrupt_vector_0x32 */
    NULL,                       /* cached_interrupt_vector_0x42 */
    NULL,                       /* cached_interrupt_vector_0x52 */
    NULL,                       /* cached_interrupt_vector_0x62 */
    NULL,                       /* cached_interrupt_vector_0x72 */
    NULL,                       /* cached_interrupt_vector_0x82 */
    NULL,                       /* cached_interrupt_vector_0x92 */
    NULL,                       /* cached_interrupt_vector_0xA2 */
    NULL,                       /* cached_interrupt_vector_0xB2 */
    NULL,                       /* cached_interrupt_vector_0xC2 */
    NULL,                       /* cached_interrupt_vector_0xD2 */
    NULL,                       /* cached_interrupt_vector_0xE2 */
    NULL,                       /* cached_interrupt_vector_0xF2 */
    NULL,                       /* nmi_handler                  */
    NULL,                       /* xint0_handler                */
    NULL,                       /* xint1_handler                */
    NULL,                       /* xint2_handler                */
    NULL,                       /* xint3_handler                */
    NULL,                       /* xint4_handler                */
    NULL,                       /* xint5_handler                */
    NULL,                       /* xint6_handler                */
    NULL,                       /* xint7_handler                */
    NULL,                       /* xint_timer0_handler          */
    NULL,                       /* xint_timer1_handler          */
    0,                          /* bootStatus                   */
    IDR_COORDINATED_BOOT_GOOD,  /* runtimeStatus                */
    0,                          /* timerCounter                 */
    SIZE_64MEG,                 /* sizeDRAM                     */
    0,                          /* prcbPtr                      */
    0,                          /* imskRestore                  */
    IDR_PROCESSOR_STATE_NORMAL, /* processorState               */
    0,                          /* resetStatus                  */
};

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
