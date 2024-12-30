/* $Id: kernel.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file   kernel.h
**
**  @brief  'C' overlay for kernel.as
**
**  Copyright (c) 2001, 2009-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef KERNEL_H
#define KERNEL_H

#include "ccb_flash.h"
#include "crc32.h"
#include "heap.h"
#include "pcb.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif


/*****************************************************************************
** Public defines
*****************************************************************************/
#define CCBRuntimeFWHAddr               ( CCB_RUNTIME_FW_HEADER_ADDR )

/* Constants to use when referring to the different processors */
#define PROCESS_CCB                   0
#define PROCESS_FE                    1
#define PROCESS_BE                    2
#define PROCESS_ALL                   3


/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void K$TraceEvent(unsigned int id, unsigned int data);


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* KERNEL_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
