/* $Id: rm_misc.c 159549 2012-07-27 15:13:43Z marshall_midden $ */
/*============================================================================
**
**   File Name:              rm_misc.c
**   Module Title:           Resource Manager Miscellaneous functions
**
**   This file contains routines that the Resource Manager utilizes for
**   miscellaneous functions.
**
**   These are:
**
**   DLInsert, DLDelete                   Doubly linked list management functions
**
**   rmGetTargetCacheStatus               Returns TRUE if a particular target has any cache-enabled VIDS
**   RMUpdateVID                          Given a VID, creates target structs if necessary and updates caching status
**   rmCleanup                            Deallocates all internal structures
**   rmAddController                      Adds a controller to the internal configuration
**   rmDeleteController                   Deletes a controller from the internal configuration
**   rmAddInterface                       Adds an interface to the internal configuration
**   rmDeleteInterface                    Deletes an interface from the internal configuration
**   rmAddTarget                          Adds a target to the internal configuration
**   rmDeleteTarget                       Deletes a target from the internal configuration
**   rmTelInit                            Tells RMInit action to perform
**   RMWaitState                          Performs wait until particular RM state reached
**   rmFindInterface                      Searches controller for requested interface (channel) ID
**   rmFindController                     Searches config for requested controller SSN
**   rmFindTarget                         Searches config for requested target ID
**   rmVerifyFileSystemReady              Verifies file system ready; waits if not
**   rmGetMirrorPartner                   Obtains the mirror partner serial num for a given controller
**   RMCheckControllerFailed              Returns TRUE if requested controller SSN is failed
**
**  Copyright (c) 2002  XIOtech Corporation.  All rights reserved.
**
**==========================================================================*/

#include "MR_Defs.h"
#include "PktCmdHdl.h"
#include "PacketInterface.h"
#include "ipc_packets.h"

#include "AsyncEventHandler.h"
#include "CmdLayers.h"
#include "cps_init.h"
#include "LOG_Defs.h"
#include "fm.h"
#include "kernel.h"
#include "logdef.h"
#include "PI_CmdHandlers.h"
#include "PI_Target.h"
#include "PI_Utils.h"
#include "PI_VDisk.h"
#include "PortServer.h"
#include "XIO_Std.h"
#include "rm.h"
#include "debug_files.h"
#include "pcb.h"
#include "serial_num.h"
#include "sm.h"
#include "quorum.h"
#include "trace.h"

extern volatile RMOpState RMInitState;
extern volatile RMOpState RMCurrentState;
extern volatile PCB *pRMInitMgrPcb;     /* RM Init/Shutdown manager PCB */
extern volatile UINT32 RMShutdownRequested;     /* Shutdown pending flag */
extern volatile unsigned char rmElectionMaster; /* TRUE = we are the master */


/*****************************************************************************
**  FUNCTION NAME: RMCheckControllerFailed
**
**  PARAMETERS: SSN of controller
**
**  RETURNS:    TRUE/FALSE  TRUE if controller is failed
**
******************************************************************************/
UINT32 RMCheckControllerFailed(UINT32 SSN)
{
    UINT32      index1;
    UINT32      j;
    UINT32      failedCtrl = TRUE;

    /*
     * Check if the controller is active.  Check
     * each entry in the active controller map.
     */
    for (index1 = 0; index1 < Qm_GetNumControllersAllowed(); ++index1)
    {
        /*
         * Get the controller serial number for this entry
         * in the controller configuration map.
         */
        if (SSN == CCM_ControllerSN(index1))
        {
            /*
             * Check if the controller is active.  Check
             * each entry in the acitve controller map
             * and look for this index.
             */
            for (j = 0; j < Qm_GetNumControllersAllowed(); ++j)
            {
                /*
                 * Get the index for this entry in the active controller map.
                 * Is this the right index?
                 */
                if (index1 == Qm_GetActiveCntlMap(j))
                {
                    /*
                     * This is the right index.  The controller is active.
                     * Exit the loop.
                     */
                    failedCtrl = FALSE;
                    break;
                }
            }
        }
    }

    return (failedCtrl);
}


/*****************************************************************************
**  FUNCTION NAME: rmTelInit
**
**  COMMENTS:
**      Set RMInit state to indicated action.  Overwrite any
**      pending action.  If task is not already active, fork it.
******************************************************************************/
void rmTelInit(RMOpState OpState)
{
    /* Overwrite previous state */
    RMInitState = OpState;

    if (pRMInitMgrPcb == NULL)
    {
        /* Spawn init/shutdown task */
        pRMInitMgrPcb = TaskCreate(RMInit, NULL);
    }
}

/*****************************************************************************
**  FUNCTION NAME: RMStateWait
**
**  COMMENTS:
**      Performs <RMWAITTIME> ms waits until RM operational
**      state is equal to indicated state
******************************************************************************/
void RMStateWait(RMOpState OpState)
{
    dprintf(DPRINTF_RM, "RM: RMWaitState entered\n");

    while (RMCurrentState != OpState)
    {
        /* Wait for indicated time interval */
        TaskSleepMS(RMWAITTIME);
    }
}

/*****************************************************************************
**  FUNCTION NAME: rmSetBusy
**
**  COMMENTS:
**      Checks RM operational state.
**
**      If RMCurrentState == RMRUNNING and RMShutdownRequested != TRUE,
**      set RMCurrentState to RMBUSY and return with RMOK.
**
**      Else, if shutdown requested or RMCurrentState != RMBUSY,
**      return with RMERROR.
**
**      Else, if RMCurrentState == RMBUSY, wait until it is != RMBUSY,
**      or shutdown is requested, then follow the first two rules.
******************************************************************************/
RMErrCode rmSetBusy(void)
{
    while (RMCurrentState == RMBUSY && RMShutdownRequested != TRUE)
    {
        /* Wait for indicated time interval */
        TaskSleepMS(RMWAITTIME);
    }

    if (RMShutdownRequested == TRUE || RMCurrentState != RMRUNNING)
    {
        /* RM not ready to process command */
        return (RMNOTREADY);
    }

    RMCurrentState = RMBUSY;

    return (RMOK);
}

/*****************************************************************************
**  FUNCTION NAME: rmClearBusy
**
**  COMMENTS:
**      Set state to RMRUNNING if BUSY; otherwise do nothing.
******************************************************************************/
void rmClearBusy(void)
{
    if (RMCurrentState == RMBUSY)
    {
        RMCurrentState = RMRUNNING;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "rmClearBusy: Called with RMCurrentState != RMBUSY!\n");
    }
}

/*----------------------------------------------------------------------------
**  Function Name: rmInitWaitForBEReady
**
**  Description:
**      Loops until BE II status says full define.
**
**--------------------------------------------------------------------------*/
void rmInitWaitForBEReady(void)
{
    dprintf(DPRINTF_DEFAULT, "rmInitWaitForBEReady - Waiting for power-up complete...\n");

    do
    {
        TaskSleepMS(1000);
    } while (!PowerUpComplete());

    dprintf(DPRINTF_DEFAULT, "rmInitWaitForBEReady - Power-up complete...continuing RM initialization.\n");
}

/*----------------------------------------------------------------------------
**  Function Name: rmVerifyFileSystemReady
**
**  Description:
**      Loops until owned drive count > 0 or RM shutdown requested.
**
**  Note:  Shamelessly lifted from <cps_init.c>
**
**--------------------------------------------------------------------------*/
void rmVerifyFileSystemReady(void)
{
    dprintf(DPRINTF_RM, "RM: Waiting for file system to become ready...\n");

    while (Qm_GetOwnedDriveCount() == 0 &&
           RMShutdownRequested != TRUE && rmElectionMaster == TRUE)
    {
        TaskSleepMS(5000);
    }

    dprintf(DPRINTF_RM, "RM: Exiting wait for owned drives\n");
}



/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
