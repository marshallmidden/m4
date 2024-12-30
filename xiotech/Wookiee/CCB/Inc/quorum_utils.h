/* $Id: quorum_utils.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       quorum_utils.h
** MODULE TITLE:    Header file for quorum_utils.c
**
** DESCRIPTION:     This file contains the prototypes for a number a access
**                  routines related to quorum area data structures.
**
** Copyright (c) 2001-2010 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _QUORUM_UTILS_H_
#define _QUORUM_UTILS_H_

#include "codeburn.h"
#include "ipc_common.h"
#include "nvram.h"
#include "quorum.h"
#include "serial_num.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

extern UINT16 GetCommunicationsSlot(UINT32 controllerSN);

extern bool TestforMaster(UINT32 controllerSN);

extern UINT32 GetControllerSN(UINT16 slotID);

extern PATH_TYPE GetTransportType(UINT32 ipAddress);

extern UINT32 SerialNumberToIPAddress(UINT32 controllerSN, PATH_TYPE pt);

extern UINT32 IPAddressToSerialNumber(UINT32 ipAddress);

extern UINT32 LoadMasterConfiguration(void);

extern UINT32 LoadControllerMap(void);

extern UINT32 ReloadMasterConfigWithRetries(void);

extern UINT32 ReadCommAreaWithRetries(UINT16 slotID, QM_CONTROLLER_COMM_AREA *commPtr);

extern UINT32 ReadAllCommunicationsWithRetries(QM_CONTROLLER_COMM_AREA commPtr[]);

extern UINT32 ReadFailureDataWithRetries(unsigned long controllerSN,
                                         QM_FAILURE_DATA *failurePtr);

extern UINT32 WriteElectionDataWithRetries(unsigned long controllerSN,
                                           QM_ELECTION_DATA *electionPtr);

extern UINT32 WriteFailureDataWithRetries(unsigned long controllerSN,
                                          QM_FAILURE_DATA *failurePtr);

#define GetMyControllerSN() (CntlSetup_GetControllerSN())

#define PartOfVCG() ((GetMyControllerSN() != GetSerialNumber(SYSTEM_SN)) \
                      && (GetSerialNumber(SYSTEM_SN) ==  Qm_GetVirtualControllerSN()))

extern INT32 ReadFWCompatIndexWithRetries(FW_COMPAT_DATA *fwCompatData);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _QUORUM_UTILS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
