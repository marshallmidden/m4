/* $Id: dlmbe.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       dlmbe.h
**
**  @brief      DLM layer header file
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DLMBE_H_
#define _DLMBE_H_

#include "XIO_Types.h"

#include "ldd.h"
#include "vlar.h"

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void DLM_ClrLDD(LDD* pLDD);
extern LDD* DLM_GetLDD(void);
extern void DLM_PutLDD(LDD* pLDD);
extern void DLM_ClearLDDIndx(void);

#endif /* _DLMBE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
