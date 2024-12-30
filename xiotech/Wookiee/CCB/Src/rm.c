/* $Id: rm.c 159549 2012-07-27 15:13:43Z marshall_midden $ */
/*
   File Name:              rm.c
   Module Title:           Resource Manager

   Resource Manager

   Provides management of global resources in a multi-controller environment.

   These resources consist of the following:

   Controllers
   Interfaces
   Targets
   Hotspares
   Cache Mirroring
   Write I/O intent mirroring

   Resource Manager is only intended to be used
   on one controller in a multi-controller configuration, nominally called
   the "Master Controller".


   Resource Manager provides two interfaces to accomplish its task.
   The first is from the user interface and the second is from the
   Failure Manager (see <fm.c>).  The user interface allows the user
   interface to manually manipulate resources.  This includes:


   Add a controller to the configuration
   Remove a controller from the configuration
   Fail a controller
   Unfail a controller
   Create a target on a particular interface
   Delete a target from a particular interface
   Move a target from one interface to another (need not be on the same controller)
   Lock a target (Prevents movement from controller to another)
   Unlock target (Allow movement from controller to another)
   Fail an interface
   Unfail an interface
   Add a hotspare
   Remove a hotspare
   Fail a physical drive
   Unfail a physical drive
   Add a cache mirror to another controller
   Remove a cache mirror to another controller
   Move a cache mirror from one controller to another controller
   Add a write I/O intent mirror to another controller
   Remove a write I/O intent mirror to another controller
   Move a write I/O mirror from one controller to another controller


   Note that some commands imply others, for example failing a controller would
   imply failing all its interfaces on that controller.  Failing an interface
   would imply moving all targets on that interface to another controller.


   The interface to Failure Manager implements the following commands:

   Fail a controller
   Fail an interface
   Fail a physical drive (request a hotspare)
   Fail a physical drive (failure of a hotspare device)
*/

#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "MR_Defs.h"
#include "debug_files.h"
#include "ipc_packets.h"
#include "PktCmdHdl.h"
#include "RMCmdHdl.h"
#include "PacketInterface.h"
#include "EL.h"
#include "LOG_Defs.h"
#include "logdef.h"
#include "logging.h"
#include "pcb.h"
#include "PI_CmdHandlers.h"
#include "PI_Target.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "XIO_Std.h"
#include "rm.h"
#include "rm_val.h"
#include "serial_num.h"
#include "sm.h"
#include "trace.h"
#include "X1_AsyncEventHandler.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define RM_IsShuttingDown()     (RMCurrentState != RMINIT || \
                                 RMShutdownRequested == TRUE || \
                                 rmElectionMaster != TRUE)

/*****************************************************************************
** Private variables
*****************************************************************************/
static unsigned char rmElectionIsInProgress = FALSE;    /* DO NOT CHANGE this default value! */

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/

/*  Globally used variables within RM module */
volatile unsigned char rmElectionMaster = FALSE;        /* DO NOT CHANGE this default value! */

volatile RMOpState RMCurrentState = RMDOWN;     /* Current RM state */
volatile UINT32 RMShutdownRequested = FALSE;    /* Global shutdown requested flag */
volatile RMOpState RMInitState = RMNONE;        /* Pending init/shutdown action */
volatile PCB *pRMInitMgrPcb = NULL;     /* RM Init/Shutdown manager PCB */

/*****************************************************************************
** Private functions used locally only.
*****************************************************************************/
static void rmInit(void);       /* Perform RM initialization */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Function to inquire if reallocation is currently
**              running.
**
**  @param      none
**
**  @return     true if reallocation is running, false otherwise.
**
******************************************************************************
**/
bool RM_IsReallocRunning(void)
{
    bool        bRunning = false;

    if (RMGetState() != RMRUNNING && RMGetState() != RMDOWN)
    {
        bRunning = true;
    }

    return bRunning;
}


/**
******************************************************************************
**
**  @brief      Function to inquire if reallocation believes an election
**              is currently running.
**
**  @param      none
**
**  @return     true if election is running, false otherwise.
**
******************************************************************************
**/
bool RM_IsElectionRunning(void)
{
    return (rmElectionIsInProgress);
}


/*----------------------------------------------------------------------------
** Function:    RMGetState
**
** Description: Returns the current state of resource manager.
**
** Inputs:      NONE
**
** Returns:     RMOpState value for the current state of resource manager.
**
**--------------------------------------------------------------------------*/
RMOpState RMGetState(void)
{
    return RMCurrentState;
}


/*
   Failure Manager Calls

   RMErrCode RMFailController(UINT32 SSN)
       Searches for the controller with serial number SSN and
       performs a fail action.  This fails all the controller's
       interfaces and moves its targets to other controllers.

*/

/*****************************************************************************
**  FUNCTION NAME: RMFailController
**
**  PARAMETERS: SSN - The SN of the controller the interface exists on
**
**  RETURNS:    RMErrCode  (RMOK if successful)
**
**  COMMENTS:   May block if waiting for RM resources.
******************************************************************************/
RMErrCode RMFailController(UNUSED UINT32 SSN)
{
    RMErrCode   ReturnCode = RMERROR;

    TraceEvent(TRACE_RM, TRACE_RM_FAIL_CONTROLLER);

    /* RM must be able to run */
    if (RMCurrentState == RMRUNNING || RMCurrentState == RMBUSY)
    {
        ReturnCode = RMOK;

        /* Kick off reallocation task */
        RmStartReallocTask();
    }

    return (ReturnCode);
}

/*  Internal functions */

/*****************************************************************************
**  FUNCTION NAME: RMInit
**
**  PARAMETERS: None.
**
**  RETURNS:    None.
**
**  COMMENTS:
**   Forked as a task.
**
**   Perform RM initialization/shutdown functions:
**
**   Init:
**
**   1).  Get list of targets
**   2).  Derive controller, interface, and target type by doing
**        target info MRPs for each target retrieved
**   3).  Get list of physical drives, and retrieve status of each drive
**   4).  Build internal structures for:
**        a). Controllers in VCG
**        b). Interfaces in each controller
**        c). Targets on each interface, get lock and pairing status
**        d). Hotspare list
**   5).  Reconcile cache mirror partners
**   6).  Retrieve checkpointed action information and restart uncompleted
**        actions
**
**   Shutdown:
**
**   1).  Free controller/interface/target structures
**   2).  Free hotspare structures
**
**   Initialization obtains its knowledge about interfaces based on targets
**   configured for them.  If an interface has no targets, RM will not know
**   about them.   *** Future: Extract list of good interfaces from PROC **
******************************************************************************/
void RMInit(UNUSED TASK_PARMS *parms)
{
    RMOpState   OpState = RMNONE;

    TraceEvent(TRACE_RM, TRACE_RM_INIT_SHUTDOWN);

    /*
     * Take action based on value of RMInitState.  Set
     * RMInitState to <RMNONE> before taking action.
     */
    dprintf(DPRINTF_RM, "RM: RMInit entered.\n");

    do
    {
        /* Capture current init state, if any */
        OpState = RMInitState;

        /* Clear old state */
        RMInitState = RMNONE;

        switch (OpState)
        {
            case (RMINIT):
                /* Insure shutdown requested flag not set */
                RMShutdownRequested = FALSE;

                /* If RM not shut down, ignore initialize request. */
                if ((RMCurrentState == RMDOWN || RMCurrentState == RMNONE) &&
                    rmElectionMaster == TRUE)
                {
                    /* Set operation state to initializing */
                    RMCurrentState = RMINIT;

                    rmInit();

                    /* Set operation state to operational */
                    RMCurrentState = RMRUNNING;
                }
                break;

            case (RMSHUTDOWN):
                /* Flag shutdown requested */
                RMShutdownRequested = TRUE;

                if (RMCurrentState == RMBUSY)
                {
                    /* Wait until RM state is running */
                    RMStateWait(RMRUNNING);
                }

                /* Set operation state to shutting down */
                RMCurrentState = RMSHUTDOWN;

                SM_Cleanup();

                /* Set operation state to down */
                RMCurrentState = RMDOWN;

                RMShutdownRequested = FALSE;
                break;

            case RMNONE:
            case RMRUNNING:
            case RMBUSY:
            case RMDOWN:
            default:
                break;
        }
        /* Check for additional work during init/shutdown */
    } while (RMInitState != RMNONE);

    /* This task is terminating, clear out PCB entry */
    pRMInitMgrPcb = NULL;

    dprintf(DPRINTF_RM, "RM: RMInit exiting.\n");

    TraceEvent(TRACE_RM, TRACE_RM_INIT_SHUTDOWN);
}

static void rmInit(void)
{
    TraceEvent(TRACE_RM, TRACE_RM_INIT);

    /* Wait until BE ready before proceeding with initialization */
    rmInitWaitForBEReady();

    /*
     * Wait until file system ready before we proceed. (waits until
     * controller owns drives).  Exits afterward if election caused
     * master status to change, or shutdown requested.
     */
    rmVerifyFileSystemReady();

    /* Dont proceed if shutting down */
    if (RM_IsShuttingDown())
    {
        TraceEvent(TRACE_RM, TRACE_RM_INIT);
        return;
    }

    dprintf(DPRINTF_RM, "rmInit: Starting SM init\n");
    TaskCreate(SM_Init, NULL);

    /* Dont proceed if shutting down */
    if (RM_IsShuttingDown())
    {
        TraceEvent(TRACE_RM, TRACE_RM_INIT);
        return;
    }

    /* Set state to RUNNING before kicking off any of other tasks. */
    RMCurrentState = RMRUNNING;

    dprintf(DPRINTF_RM, "RM: Starting reallocation/redistribution process\n");
    RmStartReallocTask();

    /*
     * Start group validation since something triggered RM to
     * go through initialization again (election, controller
     * failure, etc.).
     */
    RM_StartGroupValidation(VAL_TYPE_NORMAL);

    TraceEvent(TRACE_RM, TRACE_RM_INIT);
}

/*****************************************************************************
**  FUNCTION NAME: RMElectionNotify
**
**  PARAMETERS: ElectionState
**
**  RETURNS:    char
**
**  COMMENTS:   Receives notification messages from the election process
**              and tracks the current master/slave state.
******************************************************************************/
UINT8 RM_ElectionNotify(UINT8 ElectionState)
{
    UINT8       returnCode = GOOD;

    switch (ElectionState)
    {
        case ELECTION_STARTING:
            rmElectionIsInProgress = TRUE;
            rmTelInit(RMSHUTDOWN);
            break;

        case ELECTION_NOT_YET_RUN:
        case ELECTION_IN_PROGRESS:
        case ELECTION_STAYING_SINGLE:
        case ELECTION_SWITCHING_TO_SINGLE:
        case ELECTION_STAYING_MASTER:
        case ELECTION_SWITCHING_TO_MASTER:
        case ELECTION_STAYING_SLAVE:
        case ELECTION_SWITCHING_TO_SLAVE:
            break;

        case ELECTION_AM_SINGLE:
        case ELECTION_AM_MASTER:
            rmElectionMaster = TRUE;
            break;

        case ELECTION_AM_SLAVE:
        case ELECTION_INACTIVE:
        case ELECTION_FAILED:
        case ELECTION_DISASTER:
            rmElectionMaster = FALSE;
            break;

        case ELECTION_FINISHED:
            /*
             * The election is finished so clear the in-progress indicator.
             */
            rmElectionIsInProgress = FALSE;

            /*
             * If this is the master controller then startup RM if it is not
             * already running.  If this is the slave, shutdown RM if it is
             * running.
             */
            if (rmElectionMaster == TRUE)
            {
                /* Master, start up RM */
                rmTelInit(RMINIT);

#ifdef ENABLE_GLM_SERVER
                /* Start GLM    */
                XK_System("/etc/init.d/GlmServer start &>/tmp/GLMServer_start_stat");
#endif  /* ENABLE_GLM_SERVER */
            }
            else
            {
                /* Not master, shut down */
                rmTelInit(RMSHUTDOWN);

#ifdef ENABLE_GLM_SERVER
                /* Stop GLM     */
                XK_System("/etc/init.d/GlmServer stop &>/tmp/GLMServer_stop_stat");
#endif  /* ENABLE_GLM_SERVER */
            }
            break;

        default:
            returnCode = ERROR;
            break;
    }

    return (returnCode);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
