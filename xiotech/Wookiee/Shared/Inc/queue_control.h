/* $Id: queue_control.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       qcs.h
**
**  @brief      Queue Control Block
**
**  Copyright (c) 1996-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _QUEUE_CONTROL_H_
#define _QUEUE_CONTROL_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name   Constants for the flags field in the QUEUE_BLOCK struct
**/
/* @{ */
#define QB_EMPTY    1   /**< bit 0 - queue empty                            */
#define QB_FULL     2   /**< bit 1 - queue completely full                  */
#define QB_PBLOCK   4   /**< bit 2 - processes waiting on queue resources   */
/* @} */

/**
**  @name   Constants for the flags field in the QUEUE_CONTROL struct
**/
/* @{ */
#define QCS_BUSY0        0       /**< queue handler 0 busy flag              */
#define QCS_BUSY1        1       /**< queue handler 1 busy flag              */
#define QCS_BUSY2        2       /**< queue handler 2 busy flag              */
#define QCS_BUSY3        3       /**< queue handler 3 busy flag              */
#define QCS_BUSY4        4       /**< queue handler 4 busy flag              */
#define QCS_BUSY5        5       /**< queue handler 5 busy flag              */
#define QCS_BUSY6        6       /**< queue handler 6 busy flag              */
#define QCS_BUSY7        7       /**< queue handler 7 busy flag              */
/* @} */

/**
**  @name   Constants for the status field in the QUEUE_CONTROL struct.
**          These indicate there are queued entries ready to process for
**          this queue set up highest priority to lowest priority.
**/
/* @{ */
#define QCS_0            0
#define QCS_1            1
#define QCS_2            2
#define QCS_3            3
#define QCS_4            4
#define QCS_5            5
#define QCS_6            6
#define QCS_7            7
/* @} */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** Queue Block
**/
typedef struct QUEUE_BLOCK
{
    volatile UINT8      qb_flags;   /**< Status/Operation flags */
    UINT8               qb_ord;     /**< QUEUE_BLOCK ordinal */
    volatile UINT16     qb_size;    /**< Number of queued ILTs */
    UINT16              qb_lowWater;/**< Low water mark */
    UINT16              qb_max;     /**< Maximum queue entries */
    UINT32              qb_pstat;   /**< Proc stat code for blocked PCBs */
    volatile struct ILT *pqb_First; /**< First ILT in chain */
    volatile struct ILT *pqb_Last;  /**< Last ILT in chain */
} QUEUE_BLOCK;

/**
** Queue Control
**/
typedef struct QUEUE_CONTROL
{
    volatile UINT8  qc_flags;          /**< Operation flags                            */
    volatile UINT8  qc_status;         /**< Queue status bits                          */
    volatile UINT16 qc_numEntries;     /**< Number of queued entries in all QBs        */
    struct PCB      *pqc_PCB[8];       /**< PCBs of handler tasks */
    QUEUE_BLOCK     *pqc_QB[8];        /**< QBs of handler tasks */
} QUEUE_CONTROL;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _QUEUE_CONTROL_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
