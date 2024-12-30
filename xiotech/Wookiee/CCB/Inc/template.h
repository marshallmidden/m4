/* $Id: template.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       FileName.h
**
**  @brief      A one line description of the file
**
**  A more detailed description of this file.
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _FILENAME_H_
#define _FILENAME_H_

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
**  @name   Constants can be grouped and a group name applied.
**/
/* @{ */
#define CONSTANT_1      0x0000  /**< Doxygen comment this constant          */
#define CONSTANT_2      0x2000  /* This comment won't show in Doxygen       */
#define CONSTANT_3      0x4000
#define CONSTANT_4      0x8000
/* @} */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** Doxygen comments for this data structure
**/
typedef struct MY_STRUCT
{
    UINT16      var1;   /**< Comments on this var                           */
    UINT16      var2;   /* This comment won't show in Doxygen               */
    UINT16      var3;   /**< Multiple line comments should be formatted
                         like this to work in Doxygen                       */
} MY_STRUCT;

/*
******************************************************************************
** Public variables
******************************************************************************
*/

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _FILENAME_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
