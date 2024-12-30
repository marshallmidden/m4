/* $Id: RM_headers.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       RM_Headers.h
** MODULE TITLE:    Headers for RM functions
**
** DESCRIPTION:     This header used by many resource manager modules and
**                  contains both public and semi-private prototypes.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _RM_HEADERS_H_
#define _RM_HEADERS_H_

#include "XIO_Types.h"
#include "PacketInterface.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern bool RM_IsReallocRunning(void);
extern bool RM_IsElectionRunning(void);
extern RMOpState RMGetState(void);

/* Prototypes for FM-to-RM Manager calls */
extern UINT8 RM_ElectionNotify(UINT8 ElectionState);    /* Tell RM of election status change */

/* Prototypes for FM-to-RM Manager calls */
extern RMErrCode RMFailController(UINT32 SSN);  /* Fail a particular controller */

extern UINT32 RMCheckControllerFailed(UINT32 SSN);      /* Return TRUE if controller failed */
extern void RMInit(TASK_PARMS *parms);
extern void RMStateWait(RMOpState OpState);     /* Wait until RM in a particular state */
extern void RmStartReallocTask(void);   /* Start RM realloc task */
extern void RM_RestoreAndCheckForTargets(void);
extern void RM_WaitForResyncInactivateOperations(UINT32 timeout);

/*****************************************************************************
** Semi-private function prototypes
*****************************************************************************/
/* Templates for local subroutines */
extern void rmTelInit(RMOpState OpState);       /* Tell RMInit task action to take */
extern void rmInitWaitForBEReady(void); /* Wait until BE ready */
extern void rmVerifyFileSystemReady(void);      /* Wait until file system ready */
extern RMErrCode rmSetBusy(void);       /* Set RMBUSY status */
extern void rmClearBusy(void);  /* Clear RMBUSY status */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _RM_HEADERS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
