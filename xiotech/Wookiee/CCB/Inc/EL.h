/* $Id: EL.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       EL.h
** MODULE TITLE:    Common header file for the election (EL) component
**
** DESCRIPTION:     The functions in this module are used for coordinating
**                  and controlling bigfoot elections.
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _EL_H_
#define _EL_H_

#include "quorum.h"
#include "ipc_common.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** Election task state value definitions
** WARNING: Any changes to this list also require changes to all of the
**          election notify functions.
** NOTE: Please update EL_GetTaskStateString when anything is changed.
*/
enum _CONTROLLER_ELECTION_STATE
{
    ELECTION_NOT_YET_RUN = 0,
    ELECTION_STARTING = 1,
    ELECTION_IN_PROGRESS = 2,
    ELECTION_STAYING_MASTER = 3,
    ELECTION_SWITCHING_TO_MASTER = 4,
    ELECTION_STAYING_SLAVE = 5,
    ELECTION_SWITCHING_TO_SLAVE = 6,
    ELECTION_STAYING_SINGLE = 7,
    ELECTION_SWITCHING_TO_SINGLE = 8,
    ELECTION_AM_MASTER = 9,
    ELECTION_AM_SLAVE = 10,
    ELECTION_AM_SINGLE = 11,
    ELECTION_FINISHED = 12,
    ELECTION_FAILED = 13,
    ELECTION_INACTIVE = 14,
    ELECTION_DISASTER = 15
};

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern UINT32 EL_DoElection(void);
extern UINT32 EL_DoElectionNonBlocking(void);
extern IPC_PACKET *EL_PacketReceptionHandler(IPC_PACKET *);

extern ELECTION_DATA_STATE EL_GetCurrentElectionState(void);
extern FAILURE_DATA_STATE EL_GetFailureState(UINT8 ctlrIdx);
extern UINT32 EL_TestInProgress(void);
extern void EL_ChangeState(ELECTION_DATA_STATE newElectionState);
extern UINT8 EL_NotifyOtherTasks(UINT8 newTaskState);
extern UINT32 EL_CheckKeepAliveConnectivity(void);
extern void EL_BringUpInterfaceAllControllers(PATH_TYPE path);


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _EL_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
