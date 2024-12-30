/* $Id: nva.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       nva.h
**
**  @brief      To provide a means of handling NVA records.
**
**              Define common routines and information for handling NVA
**              records. This file also defines an NVA entry structure.
**
**  Copyright (c) 2000 - 2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _NVA_H_
#define _NVA_H_

#include "system.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/*     Note: Px_NVA_NUM must be a multiple of 32 !!!                        */
#define P4_NVA_NUM   ((NVRAM_P4_SIZE - sizeof(NVA_HEADER)) / sizeof(NVA))
                                        /* Number of P4 NVA entries         */

/*
** These new macros are for wookiee, where total size of Part-4 and 3 NVRAM is
** variable based on the type of mirror partner controller.
*/
#define NUM_OF_P4_NVA_WK  ((gMPNvramP4Size - sizeof(NVA_HEADER))/sizeof(NVA))
#define NUM_OF_P3_NVA_WK  ((gMPNvramP3Size - sizeof(NVA_HEADER))/sizeof(NVA))

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/*
** NVA Header
*/
typedef struct NVA_HEADER
{
    UINT32      checkSum;           /* Checksum                             */
    UINT8       reserved2[508];     /* Reserved2 - must be multiple of 32!!
                                        Total Header = 512 bytes            */
} NVA_HEADER;

/*
** NVA Entry
*/
typedef struct NVA
{
     UINT16     id;         /* RAID or Virtual                              */
     UINT16     reserved;
     UINT32     length;     /* Length in blocks - 0 means entry not used    */
     UINT64     lsda;       /* Logical starting disk address                */
} NVA;

/**
**  Non-Volatile Activity Control structure -
**  aids in the dynamic assignment and release of NVA entries.
**/
typedef struct NVA_CONTROL
{
    void*   addr;           /**< Addr of 1st NVA record                     */
    void*   checkSum;       /**< Addr of checksum in NVRAM                  */
    void*   mapBase;        /**< Base of NVA map                            */
    void*   mapPtr;         /**< Pointer into NVA map                       */

    UINT32  shadowCheckSum; /**< Shadowed checksum                          */
    UINT32  current;        /**< Current available NVAs                     */
    UINT32  min;            /**< Minimum available NVAs                     */
    UINT32  numWaiting;     /**< Number waiting                             */
} NVA_CONTROL;

#endif /* _NVA_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
