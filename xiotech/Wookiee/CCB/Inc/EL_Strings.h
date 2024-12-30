/* $Id: EL_Strings.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       EL_Strings.h
** MODULE TITLE:    Header file for EL_Strings.c
**
** DESCRIPTION:     The functions in this module are used for coordinating
**                  and controlling bigfoot elections.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _EL_STRINGS_H_
#define _EL_STRINGS_H_

#include "EL.h"
#include "ipc_common.h"
#include "quorum.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void EL_GetElectionStateString(char *stringPtr, ELECTION_DATA_STATE stateNumber,
                                      UINT8 stringLength);
extern void EL_GetContactMapStateString(char *stringPtr,
                                        ELECTION_DATA_CONTACT_MAP_ITEM stateNumber,
                                        UINT8 stringLength);
extern void EL_GetSendPacketResultString(char *stringPtr, PATH_TYPE stateNumber,
                                         UINT8 stringLength);
extern void EL_GetFailureDataStateString(char *stringPtr, FAILURE_DATA_STATE stateNumber,
                                         UINT8 stringLength);
extern void EL_GetMastershipAbilityString(char *stringPtr,
                                          ELECTION_DATA_MASTERSHIP_ABILITY abilityNumber,
                                          UINT8 stringLength);
extern void EL_GetICONConnectivityString(char *stringPtr,
                                         ELECTION_DATA_ICON_CONNECTIVITY
                                         connectivityNumber, UINT8 stringLength);
extern void EL_GetTaskStateString(char *stringPtr, UINT8 taskStateNumber,
                                  UINT8 stringLength);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _EL_STRINGS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
