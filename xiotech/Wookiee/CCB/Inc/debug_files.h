/* $Id: debug_files.h 140034 2010-05-06 18:11:58Z mdr $ */
/**
******************************************************************************
**
**  @file   debug_files.h
**
**  @brief  Header file for debug_files.c
**
**  Debugging function to write messages to a file
**
**  Copyright (c) 2001-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef DEBUG_FILES_H
#define DEBUG_FILES_H

#include "mode.h"

/*****************************************************************************
** Public defines
*****************************************************************************/

/* #define DEBUG_SERIAL_BUFFERED_WRITE */

/*
** 'dprintf' Constants ------------------> corresponding mode bit
**
** Note: mapping to MD_DPRINTF_OFF is a 'hard' off. You must map the
** corresponding DPRINTF_xxx constant to a real bit and recompile to
** enable these statements to print.
*/

/* DPRINTF_DEFAULT should always print, although it can be shut off */
#define DPRINTF_DEFAULT                     MD_DPRINTF_DEFAULT

/* Others mapped as needed */
#define DPRINTF_ASYNC_CLIENT                MD_DPRINTF_DEFAULT
#define DPRINTF_ASYNC_EVENT_HANDLER         MD_DPRINTF_DEFAULT

#define DPRINTF_CACHE_REFRESH               MD_DPRINTF_CACHE_REFRESH
#define DPRINTF_CACHEMGR                    MD_DPRINTF_DEFAULT
#define DPRINTF_CACHE_INVALIDATE            MD_DPRINTF_CACHE_REFRESH

#define DPRINTF_CODE_UPDATE                 MD_DPRINTF_DEFAULT
#define DPRINTF_CPSINIT                     MD_DPRINTF_DEFAULT
#define DPRINTF_ELECTION                    MD_DPRINTF_ELECTION
#define DPRINTF_ELECTION_VERBOSE            MD_DPRINTF_ELECTION_VERBOSE
#define DPRINTF_ENCRYPTION_MD5              MD_DPRINTF_MD5

#define DPRINTF_ETHERNET_LINK_MONITOR       MD_DPRINTF_ETHERNET
#define DPRINTF_ETHERNET_TASK               MD_DPRINTF_ETHERNET

#define DPRINTF_EXECMRP                     MD_DPRINTF_DEFAULT

#define DPRINTF_FM_SLAVE                    MD_DPRINTF_DEFAULT

#define DPRINTF_FCM                         MD_DPRINTF_FCALMON

#define DPRINTF_HEALTH_MONITOR              MD_DPRINTF_DEFAULT

#define DPRINTF_HWM                         MD_DPRINTF_I2C
#define DPRINTF_HWM_FUNCTIONS               MD_DPRINTF_I2C

#define DPRINTF_I2C_DEVICE_STATUS           MD_DPRINTF_I2C
#define DPRINTF_IPMI                        MD_DPRINTF_IPMI
#define DPRINTF_IPMI_IO                     MD_DPRINTF_IPMI_VERBOSE
#define DPRINTF_IPMI_COMMAND                MD_DPRINTF_IPMI_VERBOSE

#define DPRINTF_INITPROCCOMM                MD_DPRINTF_DEFAULT

#define DPRINTF_IPC_COMMAND_DISPATCHER      MD_DPRINTF_IPC
#define DPRINTF_IPC_MSG_DELIVERY            MD_DPRINTF_IPC_MSGS
#define DPRINTF_IPC_SENDPACKET              MD_DPRINTF_IPC
#define DPRINTF_IPC_SERVER                  MD_DPRINTF_IPC
#define DPRINTF_IPC_SESSION_MANAGER         MD_DPRINTF_IPC

#define DPRINTF_PI_PROTOCOL                 MD_DPRINTF_DEFAULT
#define DPRINTF_PRINT_LOGS                  MD_DPRINTF_DEFAULT

#define DPRINTF_QUORUM                      MD_DPRINTF_DEFAULT
#define DPRINTF_QUORUMCOMM                  MD_DPRINTF_DEFAULT
#define DPRINTF_QUORUMUTILS                 MD_DPRINTF_DEFAULT

#define DPRINTF_RM                          MD_DPRINTF_RM
#define DPRINTF_RMCMDHDL                    MD_DPRINTF_RM
#define DPRINTF_RMVAL                       MD_DPRINTF_RM

#define DPRINTF_SCSICMD                     MD_DPRINTF_PI_COMMANDS

#define DPRINTF_SES                         MD_DPRINTF_SES
#define DPRINTF_SES_LED_CONTROL             MD_DPRINTF_SES

#define DPRINTF_SHIM                        MD_DPRINTF_DEFAULT
#define DPRINTF_SHUTDOWN_CHECK_IP           MD_DPRINTF_DEFAULT
#define DPRINTF_SM_HB                       MD_DPRINTF_SM_HB

#define DPRINTF_X1_COMMANDS                 MD_DPRINTF_X1_COMMANDS
#define DPRINTF_X1_PROTOCOL                 MD_DPRINTF_X1_PROTOCOL

#define DPRINTF_PI_COMMANDS                 MD_DPRINTF_PI_COMMANDS

#define DPRINTF_XSSA_DEBUG                  MD_DPRINTF_XSSA_DEBUG

#define DPRINTF_PROC_PRINTF                 MD_DPRINTF_PROC_PRINTF

#define DPRINTF_RAIDMON                     MD_DPRINTF_RAIDMON

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
#if defined(PAM)
#define dprintf(l, ...) PAM_Printf(l, __VA_ARGS__)
extern void PAM_Printf(UINT32 levelconst, char *format, ...);
#else
#include "serial.h"

#define dprintf(l, ...) _DPrintf(l, __VA_ARGS__)
extern void _DPrintf( UINT32 level,
                     const char *fmt, ...) __attribute__ ((__format__(__printf__, 2, 3)));
#endif

extern UINT32 EthDebugPort(void);

#endif /* DEBUG_FILES_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
