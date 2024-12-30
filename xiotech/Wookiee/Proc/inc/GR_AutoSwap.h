/* $Id: GR_AutoSwap.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       GR_AutoSwap.h
**
**  Copyright (c) 2005-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _GR_AUTOSWAP_H_
#define _GR_AUTOSWAP_H_

#include "ccsm.h"
#include "XIO_Types.h"
#include "vdd.h"
#include "nvr.h"

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

/**
** Autoswap decision macros
**/
#define GR_ASWAP_ALLOWED    0
#define GR_ASWAP_NOT_ALLOWED    1
#define GR_ASWAPBACK_ALLOWED    2
#define GR_ASWAPBACK_NOT_ALLOWED   3

/**
#define GR_MAX_ASWAPS 256
**/
#define GR_MAX_ASWAPS 16

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** This structure is used to store the info related to the raid swap to be
** performed by the master controller ccsm. This is same as the swap define
** event structure.
**/
typedef struct CCSM_E_GRS
{
    UINT32      len;            /* event length                             */
    UINT8       type;           /* event type code                          */
    UINT8       fc;             /* event function code                      */
    UINT16      seq;            /* sequence #                               */
    UINT32      sendsn;         /* sender's serial #                        */

} CCSM_E_GRS;

typedef struct GR_ASWAP
{
    UINT32 copy_reg_id;      /*registration id*/
    UINT32 cmsn;    /*CM serial number*/

    UINT8  type;    /*swap type code */
                    /* 0 = swap and mirror*/
                    /* 1 = swap and break*/
    UINT8  rsvd[3];    /* Reserved */
} GR_ASWAP;

/**
** This structure is the Swap packet format
** Note: There is a structure defined for packet
**/
typedef struct GR_SEND_SWAP_PKT
{
    CCSM_E_GRS ccsm_event;       /* CCSM Event Header*/
    GR_ASWAP swap_info;      /* GeoRaid Swap Information*/
} GR_SEND_SWAP_PKT;

/**
** This structure is created to store the copy swap completion information
** to send it to the copy owner controller from master.
**/
typedef struct GR_SWAP_COMP
{
    UINT32 copy_reg_id;
    UINT32 cmsn;
    UINT8 status;
    UINT8 rsvd[3];
} GR_SWAP_COMP;

typedef struct GR_SWAP_COMP_PKT
{
    CCSM_E_GRS ccsm_event;
    GR_SWAP_COMP swap_comp_info;
} GR_SWAP_COMP_PKT;

typedef struct GR_NVRAM_PKT
{
    CCSM_E_GRS        nvram_event;
    UINT16            srcVid;
    GR_GeoRaidNvrInfo srcVal;
    UINT16            destVid;
    GR_GeoRaidNvrInfo destVal;
    UINT8             rsvd[2];
} GR_NVRAM_PKT;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT32 GR_AutoSwap(VDD* , UINT8 , UINT8);
extern void  GR_AutoSwapBack (VDD*, VDD*);
extern void GR_AutoSwapComp(VDD* , VDD* , UINT8);
extern void GR_SetSyncFlagOnMirrors   (VDD *);
extern void GR_ClearSyncFlagOnMirrors (VDD *);
extern void GR_GenerateSrcFailEvent   (COR *);
extern UINT32 GR_AnyMirrorsExist      (VDD *);
#endif /* _GR_AUTOSWAP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
