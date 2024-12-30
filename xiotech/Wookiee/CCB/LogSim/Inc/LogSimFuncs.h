/* $Id: LogSimFuncs.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file   LogSimFuncs.h
**
**  @brief  Log Simulator functions
**
**  Copyright (c) 2001-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef LOGSIM_FUNCS_H
#define LOGSIM_FUNCS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "LogSimDefs.h"

#include "MR_Defs.h"
#include "logging.h"
#include "PacketInterface.h"
#include "pcb.h"

#include "ccb_flash.h"
#include "codeburn.h"
#include "convert.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL_Strings.h"
#include "errorCodes.h"
#include "FCM_Strings.h"
#include "HWM_ConditionStrings.h"
#include "HWM_StatusStrings.h"
#include "i82559_Strings.h"
/* #include "isr.h" */
#include "led.h"
#include "logdef.h"
#include "logview.h"
#include "nvram.h"
#include "PortServer.h"
#include "quorum.h"
#include "realtime.h"
#include "rtc.h"
#include "rtc_structure.h"
#include "Snapshot.h"
#include "XIO_Macros.h"
#ifdef LOG_SIMWOOKIEE
    #include "LKM_layout.h"
#endif /* LOG_SIMWOOKIEE */
#include <time.h>

/*****************************************************************************
** Public defines
*****************************************************************************/

/*****************************************************************************
** Public variables
*****************************************************************************/
extern UINT8 logSimMemSpace[LS_SECTOR_SIZE * LS_NUM_SECTORS];
extern UINT8 logSimDebugMemSpace[LS_SECTOR_SIZE_DEBUG * LS_NUM_SECTORS_DEBUG];

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

void LogSimSetMemSpace(LOG_INFO_CB *);
void LogSimSetDebugMemSpace(LOG_INFO_CB *);
void LogSimPrintLogEntry(LOG_HDR *);
UINT32 LogSimLoadCDFile(char *cfile, char *dfile);
UINT32 LogSimLoadFile(char *file);
UINT32 LogSimLoadDebugFile(char *file);
void LogSimPrintAllLogs(char *outFile, char *needle, bool reverse);
void LogSimPrintCustomerLogs(char *outFile, char *needle, bool reverse);
void LogSimPrintDebugLogs(char *outFile, char *needle, bool reverse);
void LogSimSetLogType(char type);
UINT32 LogSimPrintLogs(UINT16 mode, UINT32 sequenceNumber,
                       UINT32 eventCount, char *outFile,
                       char *needle, bool reverse);
void	*MallocWC(UINT32 length);
void InetToAscii(UINT32 ipAddress, char *outputIpAddressString);

#endif /* LOGSIM_FUNCS_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
