/* $Id: debug_struct.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       mode.h
** MODULE TITLE:    Header file for mode.c
**
** DESCRIPTION:
**      Definition of mode settable bits which control features within
**      the ccb and the proc. Functions to provide access and control
**      of these mode settable features.
**
** Copyright (c) 2002-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _DEBUG_STRUCT_H_
#define _DEBUG_STRUCT_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define DISPLAY_STRUCT_MASTER_CONFIG            0x0001
#define DISPLAY_STRUCT_SOCKET_STATS             0x0002
#define DISPLAY_STRUCT_SES_DEVICE               0x0003
#define DISPLAY_STRUCT_SNAPSHOT_DIR             0x0004
#define DISPLAY_STRUCT_CCB_STATS                0x0005

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    DisplayMasterConfigStruct
**
** Description: Display Master Configuration  structure
**
** Inputs:
**
** Returns:     len  - length of display string
**
** WARNING:     Uses Global Big Buffer
**
**--------------------------------------------------------------------------*/
extern UINT32 DisplayMasterConfigStruct(void);

/*----------------------------------------------------------------------------
** Function:    DisplaySocketStats
**
** Description: Display status of all Treck sockets
**
** Inputs:
**
** Returns:     len  - length of display string
**
** WARNING:     Uses Global Big Buffer
**
**--------------------------------------------------------------------------*/
extern UINT32 DisplaySocketStats(char **buffer, UINT32 showFlags);

/*----------------------------------------------------------------------------
** Function:    DisplaySESDeviceStruct
**
** Description: Display the first entry in SESList
**
** Inputs:
**
** Returns:     len  - length of display string
**
** WARNING:     Uses Global Big Buffer
**
**--------------------------------------------------------------------------*/
extern UINT32 DisplaySESDeviceStruct(void);

/*----------------------------------------------------------------------------
** FUNCTION NAME:   DisplayCCBStatistics
**
** DESCRIPTION:
**
** INPUTS:      None
**
** WARNING:     Uses Global Big Buffer
**
** RETURNS:     len  - length of display string
**--------------------------------------------------------------------------*/
extern UINT32 DisplayCCBStatistics(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _DEBUG_STRUCT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
