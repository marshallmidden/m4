/* $Id: L_Clone.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       L_Clone.h
**
**  @brief      Header file for L_Clone.c
**
**  Copyright (c) 2003-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _L_CLONE_H_
#define _L_CLONE_H_

#ifdef CLONE
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void CloneCurrentThread(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* CLONE */
#endif /* _L_CLONE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
