/* $Id: X1_Workset.h 143845 2010-07-07 20:51:58Z mdr $ */
/**
******************************************************************************
**
**  @file       X1_Workset.h
**
**  @brief      X1 Workset functions
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _X1_WORKSET_H_
#define _X1_WORKSET_H_

#include "DEF_Workset.h"
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
** flag constants for UpdateWorksetServerMap()
** The values below are used by the X1 interface and must not be changed!
**/
#define ADD_SERVER_TO_WORKSET_MAP       1   /**< Add the server to the map  */
#define REMOVE_SERVER_FROM_WORKSET_MAP  0   /**< Remove server from the map */

/**
** flag constants for UpdateWorksetVBlockMap()
** The values below are used by the X1 interface and must not be changed!
**/
#define ADD_VBLOCK_TO_WORKSET_MAP       1   /**< Add the VBlock to the map  */
#define REMOVE_VBLOCK_FROM_WORKSET_MAP  0   /**< Remove VBlock from the map */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern INT32 InitWorksetDefaultVPort(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _X1_WORKSET_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
