/* $Id: sgl.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       sgl.h
**
**  @brief      Scatter/Gather List
**
**  To provide a common means of defining the Scatter/Gather List
**  elements.
**
**  Copyright (c) 1996-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _SGL_H_
#define _SGL_H_

#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/*
** Equates for the Scatter/Gather Flag Byte
*/
#define SG_BUFFER_ALLOC 7    /* bit 7 = Buffer has already been allocated   */

/*
** Equates for SGL size
*/
#define SG_BORROWED_BIT BIT31   /* Borrowed                                 */

/*
** Pre-header (hidden quadword) - used by m_asglbuf/M$rsglbuf
*/
#define SG_ALLOC_LEN (MGRAN + 1)    /* Precede real struct by 16 bytes */

/*
** Direction of Transfer byte
*/
#define SG_DIR_IN   0           /* Data coming into the controller          */
#define SG_DIR_OUT  1           /* Data going out of the controller         */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/*
** Begin structure (header) ----------------------------------------
*/
typedef struct SGL
{
    UINT16 scnt;                        /* Descriptor count             <s> */
    UINT8  owners;                      /* Ownership count (non-zero)   <b> */
    UINT8  flag;                        /* Flag byte                    <b> */
    UINT32 size;                        /* Size of SGL                  <w> */
} SGL;

/*
** Begin structure (descriptor) ------------------------------------
*/
typedef struct SGL_DESC
{
    void *addr;                         /* Address                      <w> */
    union
    {
        UINT32 len;                     /* Length (max of 24 bits)      <w> */
        struct
        {
            UINT8   lenReal[3];         /* Actual can only be 24 bits  3<b> */
            UINT8   direction;          /* Direction of Transfer        <b> */
        };
    };
} SGL_DESC;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SGL_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
