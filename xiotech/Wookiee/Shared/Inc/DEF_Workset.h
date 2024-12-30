/* $Id: DEF_Workset.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       DEF_Workset.h
**
**  @brief      Workset data structure
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DEF_WORKSET_H_
#define _DEF_WORKSET_H_

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
#define DEF_MAX_WORKSETS            MAX_WORKSETS

#define DEF_WS_NAME_SIZE            16
#define DEF_WS_VB_MAP_SIZE          2
#define DEF_WS_S_MAP_SIZE           32

#define DEF_WS_DEFAULT_VPORT_INIT   0xFF    /**< Initial defaultVPort value */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** Workset data structure
**/
typedef struct DEF_WORKSET
{
  UINT8     name[DEF_WS_NAME_SIZE];             /**< Workset name           */
  UINT8     vBlkBitmap[DEF_WS_VB_MAP_SIZE];     /**< VBlock bitmap          */
  UINT8     serverBitmap[DEF_WS_S_MAP_SIZE];    /**< Server bitmap          */
  UINT8     defaultVPort;                       /**< Default VPort          */
} DEF_WORKSET;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _DEF_WORKSET_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
