/* $Id: defrag.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       defrag.h
**
**  @brief      Need brief description
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DEFRAG_H_
#define _DEFRAG_H_

#include "XIO_Types.h"

#include "DEF_SOS.h"
#include "MR_Defs.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define DDR_MAX             128

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef struct DDR
{
    UINT16      direction;
    UINT16      control;
    UINT16      pdiskID;
    UINT16      raidID;
    UINT16      action;
    UINT16      status;
    UINT16      sdaHi;
    UINT16      sdaLo;
} DDR;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern MRDEFRAGMENT_RSP     gDFLastRsp;
extern UINT8                gDFCancel;
extern DDR                  gDFDebug[DDR_MAX];

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void DF_ClearLastResp(void);
extern void DF_CancelDefrag(void);
extern void DF_StopDefragRID(UINT16 rid);
extern UINT8 DF_Defragment(MR_PKT* pMRP);
extern void DF_LogDefragDone(UINT16 pid, UINT16 rid, UINT16 errCode);

#endif /* _DEFRAG_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
