/* $Id: daml.h 148621 2010-10-05 17:49:17Z m4 $ */
/**
******************************************************************************
**
**  @file       daml.h
**
**  @brief      Drive Allocation Map structures
**
**  To provide a common means of defining the Drive Allocation Map
**  Structure.
**
**  Copyright (c) 1996-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _DAML_H_
#define _DAML_H_

#include "cev.h"
#include "RL_PSD.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/*
**  Flag field bit definitions
*/
#define DAB_DIRTY       0           /* Dirty bit.  Must rebuild all.        */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

#define DAML_BASE_SIZE      (sizeof(struct DAML))
#define DAMLX_SIZE          (sizeof(struct DAMLX))
#define DAML_SIZE           (((MAX_RAIDS + 1) * DAMLX_SIZE) + DAML_BASE_SIZE)

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef struct DAMLX
{
    UINT32      auGap;              /* Gap in allocation units              */
    UINT32      auSda;              /* Starting address in allocation units */
    UINT32      auSLen;             /* Segment length in allocation units   */
    UINT16      auRID;              /* RAID ID                              */
    UINT8       rsvd14[2];          /* Reserved                             */
                                    /* QUAD BOUNDARY                    *****/
} DAMLX;

typedef struct DAML
{
    struct PDD *pdd;                /* PDD being mapped                     */
    UINT16      count;              /* Number of entries in DAML            */
    UINT8       flags;              /* Flags                                */
    UINT8       rsvd05;             /* Reserved                             */
    UINT32      largest;            /* Largest available segment (in AUs)   */
    UINT32      total;              /* Total available segment (in AUs)     */
                                    /* QUAD BOUNDARY                    *****/
    UINT16      firstGap;           /* Location of first gap                */
    UINT8       rsvd20[14];         /* Reserved                             */
                                    /* QUAD BOUNDARY                    *****/
    ZeroArray(DAMLX,damlx);         /* Extensions                           */
} DAML;

typedef struct DEF_RDA
{
    struct DAML *daml[MAX_PHYSICAL_DISKS+1];
} DEF_RDA;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void DA_Alloc(CEV *pCEV, PSD *pPSD, UINT32 au);
extern void DA_CalcSpace(UINT16 pid, UINT8 force);
extern void DA_DAMAsg(DAML *pDAM, UINT32 startau, UINT32 au, PSD *pPSD);
extern DAML *DA_DAMBuild(UINT16 pid);
extern void DA_DAMDirty(UINT16 pid);
extern void DA_DAMSum(DAML *pDAM);
extern PSD *DA_Release(PSD *pPSD);

#endif /* _DAML_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
