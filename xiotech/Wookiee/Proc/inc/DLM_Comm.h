/* $Id: DLM_Comm.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       DLM_Comm.h
**
**  @brief      Data Link Manager - Common (FE and BE) functions
**
**  To provide support for the Data-link Manager logic which
**  supports XIOtech Controller-to-XIOtech Controller functions
**  and services for Fibre communications.
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DLM_COMM_H_
#define _DLM_COMM_H_

#include "mlmt.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern MLMT* DLM_FindController(UINT32 controllerSerialNumber);

#endif /* _DLM_COMM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
