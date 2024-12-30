/* $Id: EL.c 159521 2012-07-26 18:08:55Z marshall_midden $ */
/*============================================================================
** FILE NAME:       EL.c
** MODULE TITLE:    Bigfoot election module
**
** DESCRIPTION:     The functions in this module are used for coordinating
**                  and controlling elections.
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#include "EL.h"

#include "AsyncEventHandler.h"
#include "CacheSize.h"
#include "cps_init.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL_Disaster.h"
#include "EL_DiskMap.h"
#include "EL_KeepAlive.h"
#include "EL_Strings.h"
#include "FIO.h"
#include "FIO_Maps.h"
#include "ipc_heartbeat.h"
#include "ipc_packets.h"
#include "ipc_sendpacket.h"
#include "fm.h"
#include "i82559.h"
#include "kernel.h"
#include "LargeArrays.h"
#include "logdef.h"
#include "logview.h"
#include "misc.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "PI_Utils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "realtime.h"
#include "rm.h"
#include "sm.h"
#include "serial_num.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "X1_AsyncEventHandler.h"
#include "CT_history.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

/* Election ACM macros */
#define EL_GetACMNode(i)          (electionACM.node[i])
#define EL_SetACMNode(i,a)        (electionACM.node[i] = a)

/* Election state macros */
#define EL_GetPreviousState()     (previousElectionState)
#define EL_GetCurrentState()      (currentElectionState)
#define EL_GetNextState()         (nextElectionState)

#define EL_SetPreviousState(x)    (previousElectionState = x)
#define EL_SetCurrentState(x)     (currentElectionState = x)
#define EL_SetNextState(x)        (nextElectionState = x)

/* Defines for the election packet message type */
#define IPC_ELECT_CONTACT_CONTROLLER    0
#define IPC_ELECT_NOTIFY_SLAVE          1
#define IPC_ELECT_TIMEOUT               2

typedef struct _CONTACT_CONTROLLER_CALLBACK_PARMS
{
    IPC_CALL_BACK_RESULTS callbackResults;
    IPC_PACKET *rxPacketPtr;
    UINT8       controllerSlotNumber;
} CONTACT_CONTROLLER_CALLBACK_PARMS, *PCONTACT_CONTROLLER_CALLBACK_PARMS;

typedef struct _ELECTION_CONTACT_CONTROLLER
{
    IPC_PACKET *txPacketPtr;
    IPC_PACKET *rxPacketPtr;
    CONTACT_CONTROLLER_CALLBACK_PARMS callback;
} ELECTION_CONTACT_CONTROLLER;

/* Defines for tracking contact counts */
typedef struct CONTACT_COUNT_STRUCT
{
    UINT8       highestContactCount;
    UINT8       highestContactCountTies;
} CONTACT_COUNT;

typedef struct CONTACT_PATH_COUNT_STRUCT
{
    CONTACT_COUNT masterCapability;
    CONTACT_COUNT iconConnectivity;
    CONTACT_COUNT ethernet;
    CONTACT_COUNT fibre;
    CONTACT_COUNT quorum;
    CONTACT_COUNT goodPorts;
} CONTACT_PATH_COUNT;

typedef enum EL_DURATION_LOG_ENUM
{
    EL_DURATION_LOG_START = 1,
    EL_DURATION_LOG_RESTART = 2,
    EL_DURATION_LOG_FINISH = 3,
} EL_DURATION_LOG;

/* Election task flag macros */
#define ET_STARTING                             (1 << 0)
#define ET_IN_PROGRESS                          (1 << 1)
#define ET_ENDING                               (1 << 2)
#define ET_CONTACT_ALL_CONTROLLERS              (1 << 3)
#define ET_NOTIFY_SLAVES                        (1 << 4)
#define ET_TIMEOUT_CONTROLLERS                  (1 << 5)
#define ET_CONTINUE_FROM_TIMEOUT_COMPLETE       (1 << 6)
#define ET_STATE_CHANGE_IN_PROGRESS             (1 << 7)
#define ET_QUORUM_ELECTION_PACKET               (1 << 8)
#define ET_PACKET_RECEPTION_IN_PROGRESS         (1 << 9)
#define ET_RESTART                              (1 << 10)
#define ET_PREVIOUSLY_ACTIVE                    (1 << 11)

#define SetElectionStartingFlag(x)              ((x == TRUE) ? (electionTaskFlags |= ET_STARTING) : (electionTaskFlags &= ~ET_STARTING))
#define SetElectionInProgressFlag(x)            ((x == TRUE) ? (electionTaskFlags |= ET_IN_PROGRESS) : (electionTaskFlags &= ~ET_IN_PROGRESS))
#define SetElectionContactAllControllersFlag(x) ((x == TRUE) ? (electionTaskFlags |= ET_CONTACT_ALL_CONTROLLERS) : (electionTaskFlags &= ~ET_CONTACT_ALL_CONTROLLERS))
#define SetElectionNotifySlavesFlag(x)          ((x == TRUE) ? (electionTaskFlags |= ET_NOTIFY_SLAVES) : (electionTaskFlags &= ~ET_NOTIFY_SLAVES))
#define SetElectionTimeoutControllersFlag(x)    ((x == TRUE) ? (electionTaskFlags |= ET_TIMEOUT_CONTROLLERS) : (electionTaskFlags &= ~ET_TIMEOUT_CONTROLLERS))
#define SetContinueFromTimeoutCompleteFlag(x)   ((x == TRUE) ? (electionTaskFlags |= ET_CONTINUE_FROM_TIMEOUT_COMPLETE) : (electionTaskFlags &= ~ET_CONTINUE_FROM_TIMEOUT_COMPLETE))
#define SetStateChangeInProgressFlag(x)         ((x == TRUE) ? (electionTaskFlags |= ET_STATE_CHANGE_IN_PROGRESS) : (electionTaskFlags &= ~ET_STATE_CHANGE_IN_PROGRESS))
#define SetQuorumElectionPacketFlag(x)          ((x == TRUE) ? (electionTaskFlags |= ET_QUORUM_ELECTION_PACKET) : (electionTaskFlags &= ~ET_QUORUM_ELECTION_PACKET))
#define SetPacketReceptionInProgress(x)         ((x == TRUE) ? (electionTaskFlags |= ET_PACKET_RECEPTION_IN_PROGRESS) : (electionTaskFlags &= ~ET_PACKET_RECEPTION_IN_PROGRESS))
#define SetElectionRestartFlag(x)               ((x == TRUE) ? (electionTaskFlags |= ET_RESTART) : (electionTaskFlags &= ~ET_RESTART))
#define SetPreviouslyActiveFlag(x)              ((x == TRUE) ? (electionTaskFlags |= ET_PREVIOUSLY_ACTIVE) : (electionTaskFlags &= ~ET_PREVIOUSLY_ACTIVE))

#define TestElectionStartingFlag()              ((electionTaskFlags & ET_STARTING) ? TRUE : FALSE)
#define TestElectionInProgressFlag()            ((electionTaskFlags & ET_IN_PROGRESS) ? TRUE : FALSE)
#define TestElectionEndingFlag()                ((electionTaskFlags & ET_ENDING) ? TRUE : FALSE)
#define TestElectionContactAllControllersFlag() ((electionTaskFlags & ET_CONTACT_ALL_CONTROLLERS) ? TRUE : FALSE)
#define TestElectionNotifySlavesFlag()          ((electionTaskFlags & ET_NOTIFY_SLAVES) ? TRUE : FALSE)
#define TestElectionTimeoutControllersFlag()    ((electionTaskFlags & ET_TIMEOUT_CONTROLLERS) ? TRUE : FALSE)
#define TestContinueFromTimeoutCompleteFlag()   ((electionTaskFlags & ET_CONTINUE_FROM_TIMEOUT_COMPLETE) ? TRUE : FALSE)
#define TestStateChangeInProgressFlag()         ((electionTaskFlags & ET_STATE_CHANGE_IN_PROGRESS) ? TRUE : FALSE)
#define TestQuorumElectionPacketFlag()          ((electionTaskFlags & ET_QUORUM_ELECTION_PACKET) ? TRUE : FALSE)
#define TestPacketReceptionInProgress()         ((electionTaskFlags & ET_PACKET_RECEPTION_IN_PROGRESS) ? TRUE : FALSE)
#define TestElectionRestartFlag()               ((electionTaskFlags & ET_RESTART) ? TRUE : FALSE)
#define TestPreviouslyActiveFlag()              ((electionTaskFlags & ET_PREVIOUSLY_ACTIVE) ? TRUE : FALSE)

/*
** Timeout duration defines (milliseconds)
** NOTE: These are WAY too long, caused by the quorum I/O being so freakin' slow.
*/
/* Add in how many milliseconds the BE/FE discovery is allowed to take. */
#define TIMEFACTOR  10000
/* The number of times during the timeout period to "sleep". (i.e. about 1/3 to 1/2 a second typically). */
#define CHECK_FOR_TIMEOUT_GRANULARITY               33
#define CHECK_FOR_INACTIVATE_TIMEOUT_GRANULARITY    (WAIT_FOR_INACTIVATE_TIMEOUT / 1000)

#define CONTACT_ALL_CONTROLLERS_TIMEOUT             (TIMEFACTOR+2000)
#define CONTACT_ALL_CONTROLLERS_RESPONSE_TIMEOUT    (CONTACT_ALL_CONTROLLERS_TIMEOUT + 1000)
#define CONTACT_ALL_CONTROLLERS_COMPLETE_TIMEOUT    (CONTACT_ALL_CONTROLLERS_RESPONSE_TIMEOUT + 5000)

#define TIMEOUT_CONTROLLERS_TIMEOUT                 (TIMEFACTOR+1000)
#define TIMEOUT_CONTROLLERS_RESPONSE_TIMEOUT        (TIMEOUT_CONTROLLERS_TIMEOUT + 1000)
#define TIMEOUT_CONTROLLERS_COMPLETE_TIMEOUT        (TIMEOUT_CONTROLLERS_RESPONSE_TIMEOUT + 5000)

#define NOTIFY_SLAVES_TIMEOUT                       (TIMEFACTOR+1000)
#define NOTIFY_SLAVES_RESPONSE_TIMEOUT              (NOTIFY_SLAVES_TIMEOUT + 1000)
#define NOTIFY_SLAVES_COMPLETE_TIMEOUT              (NOTIFY_SLAVES_RESPONSE_TIMEOUT + 5000)

#define CHECK_MASTER_TIMEOUT                        (CONTACT_ALL_CONTROLLERS_TIMEOUT + CONTACT_ALL_CONTROLLERS_RESPONSE_TIMEOUT + 5000)
#define WAIT_FOR_MASTER_TIMEOUT                     (NOTIFY_SLAVES_TIMEOUT + NOTIFY_SLAVES_RESPONSE_TIMEOUT + 10000)
#define WAIT_FOR_INACTIVATE_TIMEOUT                 30000

#define GetCACCSTimeoutCounter()                    (caccsTimeoutCounter)
#define SetCACCSTimeoutCounter(x)                   (caccsTimeoutCounter = x)

#define GetTCTimeoutCounter()                       (tcTimeoutCounter)
#define SetTCTimeoutCounter(x)                      (tcTimeoutCounter = x)

#define GetTCCSTimeoutCounter()                     (tccsTimeoutCounter)
#define SetTCCSTimeoutCounter(x)                    (tccsTimeoutCounter = x)

/* Other general definitions */
#define TEMP_STRING_LENGTH                          50
#define ICON_CONNECTIVITY_VALID_LIMIT               30  /* Number of seconds from last icon contact */
#define ELECTION_START_IO_TIMEOUT                   500 /* Start I/O MRP timeout (in milliseconds)  */
#define ELECTION_STOP_IO_TIMEOUT                    500 /* Stop I/O MRP timeout (in milliseconds)   */


/*****************************************************************************
** Private variables
*****************************************************************************/
static QM_CONTROLLER_COMM_AREA myControllerCommArea LOCATE_IN_SHMEM;
static UINT32 myControllerSerialNumber = 0;
static UINT32 electionTaskFlags = 0;
static UINT16 myControllerCommSlot = ACM_NODE_UNDEFINED;
static UINT16 previousMasterCommSlot = ACM_NODE_UNDEFINED;
static UINT8 myNumberOfControllersContactedEthernet = 0;
static UINT8 myNumberOfControllersContactedFibre = 0;
static UINT8 myNumberOfControllersContactedQuorum = 0;
static UINT8 myNumberOfControllersNotified = 0;
static UINT8 myNumberOfControllersTimedOut = 0;
static UINT8 contactedByMaster = FALSE;
static ELECTION_DATA_STATE previousElectionState = ED_STATE_END_TASK;
static ELECTION_DATA_STATE currentElectionState = ED_STATE_END_TASK;
static ELECTION_DATA_STATE nextElectionState = ED_STATE_END_TASK;
static ELECTION_CONTACT_CONTROLLER contactControllerList[MAX_CONTROLLERS];
static ELECTION_CONTACT_CONTROLLER notifyControllerList[MAX_CONTROLLERS];
static ELECTION_CONTACT_CONTROLLER timeoutControllerList[MAX_CONTROLLERS];
static UINT32 electionStopIOCount = 0;
static UINT32 caccsTimeoutCounter = CHECK_FOR_TIMEOUT_GRANULARITY;
static UINT32 tccsTimeoutCounter = CHECK_FOR_TIMEOUT_GRANULARITY;
static UINT32 tcTimeoutCounter = CHECK_FOR_TIMEOUT_GRANULARITY;
static QM_ACTIVE_CONTROLLER_MAP electionACM = { {-1}, {-1} };
static UINT8 masterCapabilityArray[128];        /* Uninitialized - used by CACCS */
static UINT8 iconConnectivityArray[128];        /* Uninitialized - used by CACCS */
static UINT8 goodPortCountArray[128];   /* Uninitialized - used by CACCS */
static UINT8 ethernetContactCountArray[128];    /* Uninitialized - used by CACCS */
static UINT8 fibreContactCountArray[128];       /* Uninitialized - used by CACCS */
static UINT8 quorumContactCountArray[128];      /* Uninitialized - used by CACCS */

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void EL_BeginState(void);
static void EL_CheckMastershipAbilityState(void);
static void EL_ContactAllControllersState(void);
static void EL_ContactAllControllersCallback(TASK_PARMS *parms);
static void EL_ContactAllControllersCompleteState(void);
static void EL_WaitForMasterState(void);
static void EL_CheckMasterState(void);
static void EL_NotifySlavesState(void);
static void EL_NotifySlavesCallback(TASK_PARMS *parms);
static void EL_FailedState(void);
static void EL_FinishedState(void);
static void EL_TimeoutControllersState(void);
static void EL_TimeoutControllersCallback(TASK_PARMS *parms);
static void EL_TimeoutControllersCompleteState(void);
static void EL_ControlTaskStateError(void);
static void EL_LogMessage(INT32, INT32, UINT32);
static UINT32 EL_RebuildACM(UINT16 masterCommSlot);
static UINT32 EL_SetContactMapItem(UINT16, ELECTION_DATA_CONTACT_MAP_ITEM);
static UINT32 EL_TakeMastership(void);
static void EL_WaitForStateChangeComplete(void);
static UINT8 EL_ClearContactMap(void);
static UINT8 EL_CheckMastershipAbility(void);
static UINT8 EL_CheckICONConnectivity(void);
static UINT32 EL_IsElectionSerialNewer(ELECTION_SERIAL *compareElectionSerialPtr,
                                       ELECTION_SERIAL *againstElectionSerialPtr);
static UINT32 EL_IsElectionSerialOlder(ELECTION_SERIAL *compareElectionSerialPtr,
                                       ELECTION_SERIAL *againstElectionSerialPtr);
static UINT32 EL_HowManyHaveTalkedToSlot(UINT16 controllerCommSlot);
static UINT32 EL_StartIO(void);
static UINT32 EL_StopIO(void);
static UINT32 EL_GetFEPortCount(UINT32 *countPtr);
static UINT32 EL_WaitForInactivateComplete(void);
static UINT32 EL_DurationLog(EL_DURATION_LOG logType);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name:  EL_ControlTask
**
**  Inputs:         EL_GetCurrentState()
**
**  Modifies:       ElectionInProgressFlag() - TRUE or FALSE
**
**  Returns:        Nothing
**--------------------------------------------------------------------------*/
static void EL_ControlTask(UNUSED TASK_PARMS *parms)
{
    dprintf(DPRINTF_ELECTION, "CT: Control task running\n");

    /*
     * Enter the state machine
     */
    while ((EL_GetCurrentState() != ED_STATE_END_TASK) ||
           (TestElectionRestartFlag() == TRUE))
    {
        switch (EL_GetCurrentState())
        {
            case ED_STATE_END_TASK:
                break;

            case ED_STATE_BEGIN_ELECTION:
                /*
                 * Set the IN_PROGRESS flag to indicate that the control
                 * task is running.
                 */
                SetElectionInProgressFlag(TRUE);

                EL_BeginState();
                break;

            case ED_STATE_CHECK_MASTERSHIP_ABILITY:
                EL_CheckMastershipAbilityState();
                break;

            case ED_STATE_TIMEOUT_CONTROLLERS:
                EL_TimeoutControllersState();
                break;

            case ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE:
                EL_TimeoutControllersCompleteState();
                break;

            case ED_STATE_CONTACT_ALL_CONTROLLERS:
                EL_ContactAllControllersState();
                break;

            case ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE:
                EL_ContactAllControllersCompleteState();
                break;

            case ED_STATE_WAIT_FOR_MASTER:
                EL_WaitForMasterState();
                break;

            case ED_STATE_CHECK_MASTER:
                EL_CheckMasterState();
                break;

            case ED_STATE_NOTIFY_SLAVES:
                EL_NotifySlavesState();
                break;

            case ED_STATE_FAILED:
                EL_FailedState();
                break;

            case ED_STATE_FINISHED:
                EL_FinishedState();
                break;

            default:
                EL_ControlTaskStateError();
                break;
        }

        /*
         * Suspend the election control task to allow other tasks to run
         * if the election is not ending.
         */
        if (EL_GetCurrentState() != ED_STATE_END_TASK ||
            TestElectionRestartFlag() == TRUE)
        {
            TaskSleepMS(20);
        }
    }

    /*
     * Invalidate the entire diskMapList, now that this election is finished.
     * By not doing an EL_DiskMapResetList here, it gives us the ability to
     * look at the diskMap after the election has completed.
     */
    EL_DiskMapInvalidateList();

    /*
     * Log the duration - election finish time
     */
    EL_DurationLog(EL_DURATION_LOG_FINISH);

    /*
     * Clear the IN_PROGRESS flag to indicate that the control task is ending.
     */
    SetElectionInProgressFlag(FALSE);

    dprintf(DPRINTF_ELECTION, "CT: Control task ending\n");
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_ControlTaskCommitStateChange
**
**  Inputs:         EL_GetCurrentState()
**
**  Modifies:       currentElectionState
**                  myControllerCommArea.electionStateSector.electionData.state
**
**  Returns:        GOOD
**                  ERROR
**
**  NOTE:           Some other tasks (ClearContactMap, CheckMastershipAbility)
**                  rely on this function to perform a WriteElectionData of
**                  the stuff stored in the myControllerCommArea structure.
**--------------------------------------------------------------------------*/
static void EL_ControlTaskCommitStateChange(void)
{
    UINT8       failThisController = FALSE;
    char        tempString1[TEMP_STRING_LENGTH];
    char        tempString2[TEMP_STRING_LENGTH];

    if (EL_GetCurrentState() != EL_GetNextState())
    {
        /*
         * Check this controller's failure data, to make sure it hasn't been
         * failed by another controller during the election process.
         */
        if (ReadFailureDataWithRetries(myControllerSerialNumber, &myControllerCommArea.failStateSector.failureData) == 0)
        {
            if (myControllerCommArea.failStateSector.failureData.state != FD_STATE_FAILED)
            {
                /*
                 * Modify the election state to the new value
                 */
                myControllerCommArea.electionStateSector.electionData.state = EL_GetNextState();

                /*
                 * Write new election state to quorum.
                 */
                if (WriteElectionDataWithRetries(myControllerSerialNumber, &myControllerCommArea.electionStateSector.electionData) == 0)
                {
                    /*
                     * Update the file system 'good disk' map
                     */
                    EL_DiskMapUpdateFIOMap();

                    /*
                     * Log the state change
                     */
                    EL_GetElectionStateString(tempString1, EL_GetCurrentState(), sizeof(tempString1));

                    EL_GetElectionStateString(tempString2, EL_GetNextState(), sizeof(tempString2));

                    dprintf(DPRINTF_ELECTION, "CTCSC: Changed states from %s to %s\n", tempString1, tempString2);

                    /*
                     * Make a log entry detailing the change of the election state
                     */
                    EL_LogMessage(LOG_ELECTION_STATE_CHANGE, 4, ((EL_GetNextState() << 8) | EL_GetCurrentState()));

                    /*
                     * Remember the new control task state
                     */
                    EL_SetPreviousState(EL_GetCurrentState());
                    EL_SetCurrentState(EL_GetNextState());
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "CTCSC: Controller write of new election state FAILED\n");

                    /*
                     * Force the FAILED state on a quorum write error
                     */
                    failThisController = TRUE;
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "CTCSC: Controller read FAILED state from its failureData.state\n");

                /*
                 * Force the election to the FAILED state - another controller failed this controller
                 */
                failThisController = TRUE;
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "CTCSC: Controller unable to read its comm slot (Slot %d)\n", myControllerCommSlot);

            /*
             * Force the FAILED state on a quorum read error
             */
            failThisController = TRUE;
        }

        /*
         * If we've determined that this controller needs to fail because
         * a quorum read/write error above, make sure that the controller
         * isn't already in the process of handling an election failure.
         */
        if ((failThisController == TRUE) &&
            !((EL_GetCurrentState() == ED_STATE_FAILED) && (EL_GetNextState() == ED_STATE_FINISHED)) &&
            !((EL_GetCurrentState() == ED_STATE_FINISHED) && (EL_GetNextState() == ED_STATE_END_TASK)))
        {
            dprintf(DPRINTF_ELECTION, "CTCSC: Forcing controller to FAILED election state\n");

            EL_SetPreviousState(EL_GetCurrentState());
            EL_SetCurrentState(ED_STATE_FAILED);
            myControllerCommArea.electionStateSector.electionData.state = ED_STATE_FAILED;
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_BeginState
**
**  Description:    Do any work that needs to be done before stating an
**                  election cycle.  Before kicking off the actual election,
**                  we need to make certain that all other tasks are prepared
**                  for a switch between being a master or slave controller.
**
**  Inputs:         None
**
**  Modifies:       electionData.contactMap
**
**  Returns:        Nothing
**--------------------------------------------------------------------------*/
static void EL_BeginState(void)
{
    UINT32      returnCode = GOOD;

    /*
     * Coming from states:
     *   ED_STATE_END_TASK        - Normal election entry
     *   ED_STATE_WAIT_FOR_MASTER - New master starting election before slave is finihsed
     *                              with previous election - see PacketReceptionHandler
     *
     * Going to states:
     *   ED_STATE_CONTACT_ALL_CONTROLLERS
     *   ED_STATE_FAILED (error encountered)
     */

    /*
     * Let the other tasks know that the election is IN_PROGRESS
     */
    if ((returnCode == GOOD) && (EL_NotifyOtherTasks(ELECTION_IN_PROGRESS) != GOOD))
    {
        dprintf(DPRINTF_ELECTION, "BS: EL_NotifyOtherTasks returned ERROR\n");

        returnCode = ERROR;
    }

    /*
     * Clear the election data contactMap array.  This is also cleared in
     * the StartControlTask function, but we'll do it again here for the
     * case where the election is being restarted.
     */
    if ((returnCode == GOOD) && (EL_ClearContactMap() != GOOD))
    {
        dprintf(DPRINTF_ELECTION, "BS: EL_ClearContactMap returned ERROR\n");

        returnCode = ERROR;
    }

    /*
     * Copy the ACM from the quorum's masterConfig into the
     * election code's protected ACM structure.
     */
    memcpy(&electionACM, &masterConfig.activeControllerMap, sizeof(electionACM));

    /*
     * Rebuild this controller's copy of the active controller map before
     * entering the election.
     */
    if ((returnCode == GOOD) && (EL_RebuildACM(Qm_GetActiveCntlMap(0)) != GOOD))
    {
        dprintf(DPRINTF_ELECTION, "BS: EL_RebuildACM returned ERROR\n");

        returnCode = ERROR;
    }

    if (returnCode == GOOD)
    {
        /*
         * Set the default flag values before leaving the begin state.
         */
        SetElectionContactAllControllersFlag(FALSE);
        SetElectionNotifySlavesFlag(FALSE);
        SetElectionTimeoutControllersFlag(FALSE);
        SetContinueFromTimeoutCompleteFlag(FALSE);
        SetQuorumElectionPacketFlag(TRUE);

        /*
         * Everything is initialized, so enter the election.
         */
        EL_ChangeState(ED_STATE_CHECK_MASTERSHIP_ABILITY);
    }
    else
    {
        /*
         * Go directly to the FAILED state on a quorum write failure.
         */
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_CheckMastershipAbilityState
**
**  Description:    Do any work that needs to be done before stating an
**                  election cycle.  Before kicking off the actual election,
**                  we need to make certain that all other tasks are prepared
**                  for a switch between being a master or slave controller.
**
**  Inputs:         None
**
**  Modifies:       electionData.contactMap
**
**  Returns:        Nothing
**--------------------------------------------------------------------------*/
static void EL_CheckMastershipAbilityState(void)
{
    UINT8       returnCode = GOOD;

    char        tempString[TEMP_STRING_LENGTH];

    /*
     * Coming from states:
     *   ED_STATE_BEGIN_ELECTION
     *
     * Going to states:
     *   ED_STATE_CONTACT_ALL_CONTROLLERS
     *   ED_STATE_FAILED (error encountered)
     */
    if (EL_GetPreviousState() == ED_STATE_BEGIN_ELECTION)
    {

    }
    else
    {
        /*
         * Illegal transition into ED_STATE_CHECK_MASTERSHIP_ABILITY state
         */
        EL_ControlTaskStateError();
        returnCode = ERROR;
    }

    /*
     * Check all of this controller's connectivity such that we can
     * tell if this controller is capable of becoming master of the VCG.
     */
    if (returnCode == GOOD)
    {
        returnCode = EL_CheckMastershipAbility();

        EL_GetMastershipAbilityString(tempString,
                                      myControllerCommArea.electionStateSector.electionData.mastershipAbility,
                                      sizeof(tempString));

        dprintf(DPRINTF_ELECTION, "CMAS: This controller's mastership ability is: %s\n",
                tempString);
    }

    /*
     * Check to see if this controller is connected to an ICON
     */
    if (returnCode == GOOD)
    {
        returnCode = EL_CheckICONConnectivity();

        EL_GetICONConnectivityString(tempString,
                                     myControllerCommArea.electionStateSector.electionData.iconConnectivity,
                                     sizeof(tempString));

        dprintf(DPRINTF_ELECTION, "CMAS: This controller's ICON connectivity is: %s\n",
                tempString);
    }

    /*
     * Modify the portCount to show how many FE ports are online
     */
    if (returnCode == GOOD)
    {
        /*
         * If this function call fails, don't abort the election.  GetFEPortCount
         * will return a value of zero if it is unable to get the data it needs,
         * which is acceptable for the election to continue running.
         */
        EL_GetFEPortCount(&myControllerCommArea.electionStateSector.electionData.portCount);

        dprintf(DPRINTF_ELECTION, "CMAS: This controller has %d ports online\n",
                myControllerCommArea.electionStateSector.electionData.portCount);
    }

    if (returnCode == GOOD)
    {
        /*
         * Everything is initialized, so enter the election.
         */
        EL_ChangeState(ED_STATE_CONTACT_ALL_CONTROLLERS);
    }
    else
    {
        /*
         * Go directly to the FAILED state on a quorum write failure.
         */
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_ContactAllControllersState
**--------------------------------------------------------------------------*/
static void EL_ContactAllControllersState(void)
{
    /*
     * Coming from states:
     *   ED_STATE_CHECK_MASTERSHIP_ABILITY
     *   ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE
     *
     * Going to states:
     *   ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE
     *   ED_STATE_FAILED (error encountered)
     */
    SESSION    *contactControllerSessionPtr = NULL;
    UINT32      contactControllerSerialNumber = 0;
    UINT32      timeoutCounter = 0;
    UINT8       activeControllerCounter = 0;
    UINT8       contactControllerSlot = 0;
    UINT8       returnCode = GOOD;
    UINT8       retries;

#ifndef ELECTION_SWAP_MASTERSHIP
    ETHERNET_LINK_STATUS ethernetLinkStatus =
    {   /* ETHERNET_LINK_STATUS ethernetLinkStatus */
        {                       /* ETHERNET_LINK_STATUS_BITS bits */
         0,                     /* linkStatus        */
         0,                     /* wireSpeed         */
         0,                     /* duplexMode        */
         0,                     /* reserved          */
         0                      /* linkStatusChange  */
         }
    };
#endif  /* ELECTION_SWAP_MASTERSHIP */

    if (EL_GetPreviousState() == ED_STATE_CHECK_MASTERSHIP_ABILITY)
    {

    }
    else if (EL_GetPreviousState() == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE)
    {
        dprintf(DPRINTF_ELECTION, "CACS: Previous election attempt timed out\n");
    }
    else
    {
        /*
         * Illegal transition into CONTACT_ALL_CONTROLLERS state
         */
        EL_ControlTaskStateError();
        returnCode = ERROR;
    }

    if (returnCode == GOOD)
    {
        /*
         * Clear the variable that tracks the number of controllers that
         * will respond to the contact packets.
         */
        myNumberOfControllersContactedEthernet = 0;
        myNumberOfControllersContactedFibre = 0;
        myNumberOfControllersContactedQuorum = 0;

        /*
         * Set flag to indicate to ContactAllControllersCallback that we're in the
         * ContactAllControllers function.  We need to communicate to the
         * callback function that we're now going to be expecting the callbacks,
         * so it needs to count the responses.  Without this flag being set,
         * the callback event will be ignored.
         */
        SetElectionContactAllControllersFlag(TRUE);

        dprintf(DPRINTF_ELECTION, "CACS: electionSerial.current: %d\n",
                myControllerCommArea.electionStateSector.electionData.electionSerial.current);
    }

    /*
     * Send election-start messages to all active controllers
     * in the controller group.  In the event that a controller is
     * non-responsive, time out and continue without the
     * dead controller.
     */
    for (activeControllerCounter = 0;
         ((activeControllerCounter < ACM_GetActiveControllerCount(&electionACM)) &&
          (EL_GetCurrentState() == ED_STATE_CONTACT_ALL_CONTROLLERS) &&
          (returnCode == GOOD));
         activeControllerCounter++)
    {
        /*
         * Get the slot number for the active controller that is to be contacted
         */
        contactControllerSlot = EL_GetACMNode(activeControllerCounter);

        /*
         * Get the serial number for the controller that is to be contacted
         */
        contactControllerSerialNumber = GetControllerSN(contactControllerSlot);

        /*
         * Validate the controller serial number
         */
        if (contactControllerSerialNumber != 0)
        {
            /*
             * Make certain that the controller doesn't send a packet to itself.
             */
            if (contactControllerSerialNumber != myControllerSerialNumber)
            {
                /*
                 * Indicate to all controllers that we're contacting the target controller
                 */
                if (EL_SetContactMapItem(contactControllerSlot, ED_CONTACT_MAP_CONTACTING) == GOOD)
                {
                    /*
                     * Get the session pointer for the controller being contacted
                     */
                    contactControllerSessionPtr = GetSession(contactControllerSerialNumber);

                    /*
                     * Validate the session pointer
                     */
                    if (contactControllerSessionPtr != NULL)
                    {
                        if (TestQuorumElectionPacketFlag() == TRUE)
                        {
                            /*
                             * Create a a quorumable election packet
                             */
                            contactControllerList[activeControllerCounter].txPacketPtr =
                                CreatePacket(PACKET_IPC_ELECT_QUORUM, sizeof(IPC_ELECT_QUORUM), __FILE__, __LINE__);
                        }
                        else
                        {
                            /*
                             * Create a non-quorumable election packet
                             */
                            contactControllerList[activeControllerCounter].txPacketPtr =
                                CreatePacket(PACKET_IPC_ELECT, sizeof(IPC_ELECT), __FILE__, __LINE__);
                        }

                        /*
                         * Fill in the current election serial numbers for this controller
                         */
                        contactControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.electionSerial.starting =
                            myControllerCommArea.electionStateSector.electionData.electionSerial.starting;

                        contactControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.electionSerial.current =
                            myControllerCommArea.electionStateSector.electionData.electionSerial.current;

                        contactControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.messageType =
                            IPC_ELECT_CONTACT_CONTROLLER;

                        contactControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.electionTaskState =
                            ED_STATE_CONTACT_ALL_CONTROLLERS;

                        /*
                         * Place this controller's current diskMap into the transmit packet
                         */
                        EL_DiskMapGet(&contactControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR3.diskMap);

                        /*
                         * Allocate the receive packet header/data structure
                         * Note: The callback function is responsible for freeing the memory allocated
                         *       for the rxPacket header and data pointers.
                         */
                        contactControllerList[activeControllerCounter].rxPacketPtr = MallocWC(sizeof(IPC_PACKET));

                        contactControllerList[activeControllerCounter].rxPacketPtr->header = NULL;
                        contactControllerList[activeControllerCounter].rxPacketPtr->data = NULL;

                        /*
                         * Place the pointer to the receive packet into the callback parameters
                         * so that EL_PacketReceptionHandler can analyze the rxPacket data.
                         */
                        contactControllerList[activeControllerCounter].callback.rxPacketPtr = contactControllerList[activeControllerCounter].rxPacketPtr;

                        /*
                         * Place activeControllerCounter into the callback parameters so that the receive
                         * packet handler can identify which controller the response was sent from.
                         */
                        contactControllerList[activeControllerCounter].callback.controllerSlotNumber = contactControllerSlot;

                        /*
                         * Send the packet
                         */
                        dprintf(DPRINTF_ELECTION, "CACS: Sending packet to slot %d\n",
                                contactControllerSlot);

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacket with rxPacket of %p\n", __FILE__, __LINE__, __func__, contactControllerList[activeControllerCounter].rxPacketPtr);
#endif  /* HISTORY_KEEP */

                        retries = 2;        /* Ethernet, Fiber(1), Disk Quorum(2) */
                        do
                        {
                            Free(contactControllerList[activeControllerCounter].rxPacketPtr->data);

                            contactControllerList[activeControllerCounter].callback.callbackResults.result =
                                IpcSendPacket(contactControllerSessionPtr, SENDPACKET_ANY_PATH,
                                              contactControllerList[activeControllerCounter].txPacketPtr,
                                              contactControllerList[activeControllerCounter].rxPacketPtr,
                                              (void *)EL_ContactAllControllersCallback,
                                              (void *)&contactControllerList[activeControllerCounter].callback,
                                              &contactControllerList[activeControllerCounter].callback.callbackResults,
                                              CONTACT_ALL_CONTROLLERS_TIMEOUT);

                        } while (contactControllerList[activeControllerCounter].callback.callbackResults.result == SENDPACKET_NO_PATH && (retries--) > 0);

                        /* Deallocate the transmit packet header/data memory */
                        FreePacket(&contactControllerList[activeControllerCounter].txPacketPtr, __FILE__, __LINE__);
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "CACS: contactControllerSessionPtr is NULL\n");

                        /*
                         * This isn't necessarily a fatal error... it just means that we
                         * don't have a connection to the controller.  If this controller
                         * assumes mastership, the controller that we can't connect to
                         * will get failed when we rebuild the ACM.
                         */
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "CACS: Error writing contact map item\n");

                    returnCode = ERROR;
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "CACS: Controller not sending packet to itself (slot %d)\n", contactControllerSlot);

#ifdef ELECTION_SWAP_MASTERSHIP
                if (myControllerCommSlot == previousMasterCommSlot)
                {
                    dprintf(DPRINTF_ELECTION, "CACS: Marking slot %d as CONTACTED_FIBRE\n", contactControllerSlot);
                    /*
                     * Set the item in the contactMap that corresponds to the controller that
                     * made the callback.  This will be looked at later to determine all of
                     * the controllers that are participating in the election.
                     */
                    if (EL_SetContactMapItem(contactControllerSlot, ED_CONTACT_MAP_CONTACTED_FIBRE) == GOOD)
                    {
                        /*
                         * Increment the response counter, since the controller will not
                         * be counted in ContactControllerCallback.
                         */
                        myNumberOfControllersContactedFibre++;
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "CACS: Error writing contact map item\n");

                        returnCode = ERROR;
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "CACS: Marking slot %d as CONTACTED_ETHERNET\n", contactControllerSlot);

                    /*
                     * Set the item in the contactMap that corresponds to the controller that
                     * made the callback.  This will be looked at later to determine all of
                     * the controllers that are participating in the election.
                     */
                    if (EL_SetContactMapItem(contactControllerSlot, ED_CONTACT_MAP_CONTACTED_ETHERNET) == GOOD)
                    {
                        /*
                         * Increment the response counter, since the controller will not
                         * be counted in ContactControllerCallback.
                         */
                        myNumberOfControllersContactedEthernet++;
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "CACS: Error writing contact map item\n");

                        returnCode = ERROR;
                    }
                }
#else   /* ELECTION_SWAP_MASTERSHIP */
                /*
                 * Check to see if this CCB's ethernet link status is UP.  If
                 * it is, mark ourselves as being CONTACTED_ETHERNET; if not
                 * then mark ourselves as being CONTACTED_FIBRE.  In the case
                 * where someone pulls on of the controllers' ethernet wires
                 * we want to be able to tell which CCB still has the
                 * possibility of contacting the outside world.  The ICON
                 * connectivity isn't enough, since it has a granularity of
                 * about thirty seconds (not enough to see LINK changes).
                 *
                 * NOTE: A bad return from EthernetLinkMonitor doesn't cause
                 * the election to fail... it just causes us to report
                 * only FIBRE connectivity.
                 */
                if ((EthernetLinkMonitor(&ethernetLinkStatus) == GOOD) &&
                    (ethernetLinkStatus.bits.linkStatus == LINK_STATUS_UP))
                {
                    dprintf(DPRINTF_ELECTION, "CACS: Ethernet link is up\n");
                    dprintf(DPRINTF_ELECTION, "CACS: Marking slot %d as CONTACTED_ETHERNET\n", contactControllerSlot);

                    /*
                     * Set the item in the contactMap that corresponds to the controller that
                     * made the callback.  This will be looked at later to determine all of
                     * the controllers that are participating in the election.
                     */
                    if (EL_SetContactMapItem
                        (contactControllerSlot,
                         ED_CONTACT_MAP_CONTACTED_ETHERNET) == GOOD)
                    {
                        /*
                         * Increment the response counter, since the controller will not
                         * be counted in ContactControllerCallback.
                         */
                        myNumberOfControllersContactedEthernet++;
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "CACS: Error writing contact map item\n");

                        returnCode = ERROR;
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "CACS: Ethernet link is down or got error\n");
                    dprintf(DPRINTF_ELECTION, "CACS: Marking slot %d as CONTACTED_FIBRE\n", contactControllerSlot);
                    /*
                     * Set the item in the contactMap that corresponds to the controller that
                     * made the callback.  This will be looked at later to determine all of
                     * the controllers that are participating in the election.
                     */
                    if (EL_SetContactMapItem(contactControllerSlot, ED_CONTACT_MAP_CONTACTED_FIBRE) == GOOD)
                    {
                        /*
                         * Increment the response counter, since the controller will not
                         * be counted in ContactControllerCallback.
                         */
                        myNumberOfControllersContactedFibre++;
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "CACS: Error writing contact map item\n");

                        returnCode = ERROR;
                    }
                }
#endif  /* ELECTION_SWAP_MASTERSHIP */
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "CACS: Invalid controller serial number\n");

            returnCode = ERROR;
        }
    }

    /*
     * After the election has gone through the CONTACT_ALL_CONTROLLERS
     * phase once, turn off the quorumable election packets such that we
     * don't take the timeout penalty if a controller isn't responsive.
     */
    SetQuorumElectionPacketFlag(FALSE);

    /*
     * Go to the next election state
     */
    if (returnCode == GOOD)
    {
        if (EL_GetCurrentState() == ED_STATE_CONTACT_ALL_CONTROLLERS)
        {
            /*
             * Wait for all controllers to check in (or a predetermined
             * amount of time in the case of a timeout) for all controllers
             * to check in (using ContactAllControllersCallback) before
             * transitioning out of the CONTACT_ALL_CONTROLLERS state.
             * Chop the timeout into segments, and check for completion
             * each time the process get awakened.
             */
            timeoutCounter = CHECK_FOR_TIMEOUT_GRANULARITY;
            while (((myNumberOfControllersContactedEthernet +
                     myNumberOfControllersContactedFibre +
                     myNumberOfControllersContactedQuorum) <
                    ACM_GetActiveControllerCount(&electionACM)) && (timeoutCounter > 0))
            {
                timeoutCounter--;

                if (timeoutCounter > 0)
                {
                    TaskSleepMS(CONTACT_ALL_CONTROLLERS_RESPONSE_TIMEOUT / CHECK_FOR_TIMEOUT_GRANULARITY);
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "CACS: Timeout waiting for all controllers\n");
                    dprintf(DPRINTF_ELECTION, "  (Got %d out of %d controllers)\n",
                            (myNumberOfControllersContactedEthernet +
                             myNumberOfControllersContactedFibre +
                             myNumberOfControllersContactedQuorum),
                            ACM_GetActiveControllerCount(&electionACM));
                }
            }

            /*
             * Make sure the FIRMWARE_UPDATE_INACTIVE controller does not
             * continue with the election any further than the
             * CONTACT_ALL_CONTROLLERS phase.  It's a pseudo-participator in
             * the election where it's invisible to the other controllers in the
             * election, but it still follows along with the election process.
             */
            if (myControllerCommArea.failStateSector.failureData.state == FD_STATE_FIRMWARE_UPDATE_INACTIVE ||
                myControllerCommArea.failStateSector.failureData.state == FD_STATE_INACTIVATED ||
                myControllerCommArea.failStateSector.failureData.state == FD_STATE_DISASTER_INACTIVE)
            {
                if (EL_NotifyOtherTasks(ELECTION_INACTIVE) == GOOD)
                {
                    dprintf(DPRINTF_ELECTION, "CACS: Preventing inactive controller from continuing election\n");

                    /*
                     * Invalidate the currentMasterID in the DRAM copy of the
                     * masterConfig record.  By setting this to zero, TestForMaster
                     * will return TRUE, even though the controller is inactive.
                     */
                    Qm_SetMasterControllerSN(0);

                    /*
                     * Invalidate the IP address of the master in the
                     * DRAM copy of the masterConfig record.
                     */
                    Qm_SetIPAddress(0);

                    /*
                     * Switch states now that all active controllers in the
                     * controller group have been notified.
                     */
                    EL_ChangeState(ED_STATE_FINISHED);
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "CACS: Error notifying tasks about being inactive\n");

                    /*
                     * Switch states now that all active controllers in the
                     * controller group have been notified.
                     */
                    EL_ChangeState(ED_STATE_FAILED);
                }
            }
            else
            {
                /*
                 * Switch states now that all active controllers in the
                 * controller group have been notified.
                 */
                EL_ChangeState(ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE);
            }
        }
    }

    /*
     * Clear the flag that tells the callback function to handle
     * the timeout responses so that the callbacks that occur after
     * this time are ignored.
     */
    SetElectionContactAllControllersFlag(FALSE);

    if (returnCode != GOOD)
    {
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_ContactAllControllersCompleteState
**--------------------------------------------------------------------------*/
static void EL_ContactAllControllersCompleteState(void)
{
    /*
     * Coming from states:
     *   ED_STATE_CONTACT_ALL_CONTROLLERS
     *
     * Going to states:
     *   ED_STATE_CHECK_MASTER (possible master)
     *   ED_STATE_WAIT_FOR_MASTER (not master)
     *   ED_STATE_FAILED (error encountered)
     */
    CONTACT_PATH_COUNT contactPathCount;
    UINT8       activeMapCounter = 0;
    UINT8       contactMapCounter = 0;
    UINT8       readyControllerCounter = 0;
    UINT8       unresponsiveControllerCounter = 0;
    UINT8       isMyControllerNewMaster = FALSE;
    UINT8       isTheNewMasterFound = FALSE;
    UINT8       returnCode = GOOD;
    UINT8       commSlot = 0;

    char        tempString1[TEMP_STRING_LENGTH];
    char        tempString2[TEMP_STRING_LENGTH];

#ifdef ELECTION_INJECT_TIMEOUT_DURING_CACCS
    static UINT8 caccsTimeoutInjectionCounter = 4;
#endif  /* ELECTION_INJECT_TIMEOUT_DURING_CACCS */

#ifdef ELECTION_INJECT_MASTERSHIP_COLLISION
    static UINT8 mastershipCollisionInjectionCounter = 4;
#endif  /* ELECTION_INJECT_MASTERSHIP_COLLISION */

#ifdef ELECTION_INJECT_FRAGMENTATION_CHECK_FAILURE
    static UINT8 fragmentationCheckInjectionCounter = 4;
#endif  /* ELECTION_INJECT_FRAGMENTATION_CHECK_FAILURE */

    memset(&contactPathCount, 0, sizeof(contactPathCount));

    /*
     * Set state timeout counter value
     */
    SetCACCSTimeoutCounter(CHECK_FOR_TIMEOUT_GRANULARITY);

    /*
     * Wait for all other controllers to complete their
     * CONTACT_ALL_CONTROLLERS state (or a predetermined
     * amount of time in the case of a timeout). Chop the
     * timeout into tenths, and check for completion
     * each time the process get awakened.
     */
#ifdef ELECTION_INJECT_TIMEOUT_DURING_CACCS
    if (caccsTimeoutInjectionCounter > 0)
    {
        if (K_timel & 1)
        {
            caccsTimeoutInjectionCounter--;

            dprintf(DPRINTF_ELECTION, "CACCS: ***** CACC injection counter is now %d *****\n", caccsTimeoutInjectionCounter);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "CACCS: ***** Attempting to inject timeout during CACC state *****\n");
        caccsTimeoutInjectionCounter = 4;
        SetCACCSTimeoutCounter(1);
    }
#endif  /* ELECTION_INJECT_TIMEOUT_DURING_CACCS */

    while ((readyControllerCounter < ACM_GetActiveControllerCount(&electionACM)) &&
           (EL_GetCurrentState() == ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE) &&
           (GetCACCSTimeoutCounter() > 0) &&
           (isTheNewMasterFound == FALSE) && (returnCode == GOOD))
    {
        /*
         * Reset the contact counters (inside the loop)
         */
        readyControllerCounter = 0;
        unresponsiveControllerCounter = 0;

        contactPathCount.masterCapability.highestContactCount = 0;
        contactPathCount.masterCapability.highestContactCountTies = 0;

        contactPathCount.iconConnectivity.highestContactCount = 0;
        contactPathCount.iconConnectivity.highestContactCountTies = 0;

        contactPathCount.ethernet.highestContactCount = 0;
        contactPathCount.ethernet.highestContactCountTies = 0;

        contactPathCount.fibre.highestContactCount = 0;
        contactPathCount.fibre.highestContactCountTies = 0;

        contactPathCount.quorum.highestContactCount = 0;
        contactPathCount.quorum.highestContactCountTies = 0;

        contactPathCount.goodPorts.highestContactCount = 0;
        contactPathCount.goodPorts.highestContactCountTies = 0;

        /*
         * Read the communications area belonging to the controller we need to analyze
         */
        if (ReadAllCommunicationsWithRetries(gElectCommAreas) == 0)
        {
            /*
             * Scan through the election data for all controllers
             */
            for (activeMapCounter = 0;
                 ((activeMapCounter < ACM_GetActiveControllerCount(&electionACM)) && (returnCode == GOOD));
                 activeMapCounter++)
            {
                commSlot = EL_GetACMNode(activeMapCounter);

                EL_GetElectionStateString(tempString1,
                                          gElectCommAreas[commSlot].electionStateSector.electionData.state,
                                          sizeof(tempString1));

                dprintf(DPRINTF_ELECTION, "CACCS: Slot %d state is %s\n", commSlot, tempString1);

                /*
                 * Reset this controller's contact counters to zero
                 */
                masterCapabilityArray[commSlot] = 0;
                iconConnectivityArray[commSlot] = 0;
                ethernetContactCountArray[commSlot] = 0;
                fibreContactCountArray[commSlot] = 0;
                quorumContactCountArray[commSlot] = 0;
                goodPortCountArray[commSlot] = 0;

                /*
                 * Check to be sure the controller is NOT inactive before
                 * looking at its contact map information.
                 */
                if ((gElectCommAreas[commSlot].failStateSector.failureData.state != FD_STATE_UNUSED) &&
                    (gElectCommAreas[commSlot].failStateSector.failureData.state != FD_STATE_FAILED) &&
                    (gElectCommAreas[commSlot].failStateSector.failureData.state != FD_STATE_FIRMWARE_UPDATE_INACTIVE) &&
                    (gElectCommAreas[commSlot].failStateSector.failureData.state != FD_STATE_INACTIVATED) &&
                    (gElectCommAreas[commSlot].failStateSector.failureData.state != FD_STATE_DISASTER_INACTIVE))
                {
                    /*
                     * Check the electionSerial before looking at the contact map.  It is
                     * possible that the controller is dead, and is not responsive, but
                     * still has valid contact map information.  In this case, we'll have timed
                     * out that controller, and its serial number will be outdated.  In any case,
                     * the controller must have the correct election serial number in
                     * order for this controller to look at its contacts.  If the electionSerial
                     * is valid, scan the election contact map, counting the number of good
                     * contacts.  If the target controller isn't done with the CONTACT phase,
                     * then leave its contact count at zero.  Also count the number of
                     * controllers that are ready to proceed with the election.
                     */
                    if ((gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current ==
                         myControllerCommArea.electionStateSector.electionData.electionSerial.current) &&
                        ((gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE) ||
                         (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_CHECK_MASTER) ||
                         (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_NOTIFY_SLAVES) ||
                         (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_WAIT_FOR_MASTER)))
                    {
                        readyControllerCounter++;

                        /*
                         * Check if the controller is capable of taking mastership
                         */
                        if (gElectCommAreas[commSlot].electionStateSector.electionData.mastershipAbility == ED_MASTERSHIP_ABILITY_QUALIFIED)
                        {
                            masterCapabilityArray[commSlot] = 1;
                        }

                        /*
                         * Check if the controller has connectivity with the icon
                         */
                        if (gElectCommAreas[commSlot].electionStateSector.electionData.iconConnectivity == ED_ICON_CONNECTIVITY_CONNECTED)
                        {
                            iconConnectivityArray[commSlot] = 1;
                        }

                        /*
                         * Find the number of ethernet contacts that each slot has found
                         */
                        for (contactMapCounter = 0; contactMapCounter < MAX_CONTROLLERS; contactMapCounter++)
                        {
                            if (gElectCommAreas[commSlot].electionStateSector.electionData.contactMap[contactMapCounter] ==
                                ED_CONTACT_MAP_CONTACTED_ETHERNET)
                            {
                                ethernetContactCountArray[commSlot]++;
                            }
                            else if (gElectCommAreas[commSlot].electionStateSector.electionData.contactMap[contactMapCounter] ==
                                     ED_CONTACT_MAP_CONTACTED_FIBRE)
                            {
                                fibreContactCountArray[commSlot]++;
                            }
                            else if (gElectCommAreas[commSlot].electionStateSector.electionData.contactMap[contactMapCounter] ==
                                     ED_CONTACT_MAP_CONTACTED_QUORUM)
                            {
                                quorumContactCountArray[commSlot]++;
                            }
                        }

                        /*
                         * Check how many good FE ports the controller has.
                         * Bounds check the portCount value, since the electionData
                         * sectors written by R1 controllers have garbage in this area.
                         */
                        goodPortCountArray[commSlot] = gElectCommAreas[commSlot].electionStateSector.electionData.portCount;

                        if (goodPortCountArray[commSlot] > MAX_FE_PORTS)
                        {
                            goodPortCountArray[commSlot] = 0;
                        }

                        /*
                         * Print out some important information
                         */
                        EL_GetMastershipAbilityString(tempString1,
                                                      gElectCommAreas[commSlot].electionStateSector.electionData.mastershipAbility,
                                                      sizeof(tempString1));

                        EL_GetICONConnectivityString(tempString2,
                                                     gElectCommAreas[commSlot].electionStateSector.electionData.iconConnectivity,
                                                     sizeof(tempString2));

                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d: Mastership: %s, ICON: %s, goodPorts: %d\n",
                                commSlot, tempString1, tempString2, goodPortCountArray[commSlot]);

                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d:   %d Ethernet, %d Fibre, %d Quorum\n",
                                commSlot, ethernetContactCountArray[commSlot],
                                fibreContactCountArray[commSlot],
                                quorumContactCountArray[commSlot]);

                        /*
                         * Track the controllers that have mastership ability, keeping
                         * track of the number of controllers that are capable (tied).
                         */
                        if (masterCapabilityArray[commSlot] >
                            contactPathCount.masterCapability.highestContactCount)
                        {
                            dprintf(DPRINTF_ELECTION, "CACCS: Slot %d set new mastership ability\n", commSlot);

                            contactPathCount.masterCapability.highestContactCount = masterCapabilityArray[commSlot];

                            contactPathCount.iconConnectivity.highestContactCount = iconConnectivityArray[commSlot];

                            contactPathCount.ethernet.highestContactCount = ethernetContactCountArray[commSlot];

                            contactPathCount.fibre.highestContactCount = fibreContactCountArray[commSlot];

                            contactPathCount.quorum.highestContactCount = quorumContactCountArray[commSlot];

                            contactPathCount.goodPorts.highestContactCount = goodPortCountArray[commSlot];

                            contactPathCount.masterCapability.highestContactCountTies = 0;
                            contactPathCount.iconConnectivity.highestContactCountTies = 0;
                            contactPathCount.ethernet.highestContactCountTies = 0;
                            contactPathCount.fibre.highestContactCountTies = 0;
                            contactPathCount.quorum.highestContactCountTies = 0;
                            contactPathCount.goodPorts.highestContactCountTies = 0;
                        }
                        else if (masterCapabilityArray[commSlot] == contactPathCount.masterCapability.highestContactCount)
                        {
                            dprintf(DPRINTF_ELECTION, "CACCS: Slot %d is tied for mastership ability\n", commSlot);

                            contactPathCount.masterCapability.highestContactCountTies++;

                            /*
                             * Track the controllers that have mastership ability, keeping
                             * track of the number of controllers that are capable (tied).
                             */
                            if (iconConnectivityArray[commSlot] > contactPathCount.iconConnectivity.highestContactCount)
                            {
                                dprintf(DPRINTF_ELECTION, "CACCS: Slot %d set new icon connectivity\n", commSlot);

                                contactPathCount.iconConnectivity.highestContactCount = iconConnectivityArray[commSlot];

                                contactPathCount.ethernet.highestContactCount = ethernetContactCountArray[commSlot];

                                contactPathCount.fibre.highestContactCount = fibreContactCountArray[commSlot];

                                contactPathCount.quorum.highestContactCount = quorumContactCountArray[commSlot];

                                contactPathCount.goodPorts.highestContactCount = goodPortCountArray[commSlot];

                                contactPathCount.iconConnectivity.highestContactCountTies = 0;
                                contactPathCount.ethernet.highestContactCountTies = 0;
                                contactPathCount.fibre.highestContactCountTies = 0;
                                contactPathCount.quorum.highestContactCountTies = 0;
                                contactPathCount.goodPorts.highestContactCountTies = 0;
                            }
                            else if (iconConnectivityArray[commSlot] == contactPathCount.iconConnectivity.highestContactCount)
                            {
                                dprintf(DPRINTF_ELECTION, "CACCS: Slot %d is tied for icon connectivity\n", commSlot);

                                contactPathCount.iconConnectivity.highestContactCountTies++;

                                /*
                                 * Track the controllers that have the highest ethernet contact count,
                                 * keeping track of ties for the highest number of contacts.
                                 */
                                if (ethernetContactCountArray[commSlot] > contactPathCount.ethernet.highestContactCount)
                                {
                                    dprintf(DPRINTF_ELECTION, "CACCS: Slot %d set new highest ethernet contact count\n", commSlot);

                                    contactPathCount.ethernet.highestContactCount = ethernetContactCountArray[commSlot];

                                    contactPathCount.fibre.highestContactCount = fibreContactCountArray[commSlot];

                                    contactPathCount.quorum.highestContactCount = quorumContactCountArray[commSlot];

                                    contactPathCount.goodPorts.highestContactCount = goodPortCountArray[commSlot];

                                    contactPathCount.ethernet.highestContactCountTies = 0;
                                    contactPathCount.fibre.highestContactCountTies = 0;
                                    contactPathCount.quorum.highestContactCountTies = 0;
                                    contactPathCount.goodPorts.highestContactCountTies = 0;
                                }
                                else if (ethernetContactCountArray[commSlot] == contactPathCount.ethernet.highestContactCount)
                                {
                                    dprintf(DPRINTF_ELECTION, "CACCS: Slot %d is tied for highest ethernet contact count\n", commSlot);

                                    contactPathCount.ethernet.highestContactCountTies++;

                                    /*
                                     * Track the controllers that have the highest fibre contact count,
                                     * keeping track of ties for the highest number of contacts.
                                     */
                                    if (fibreContactCountArray[commSlot] > contactPathCount.fibre.highestContactCount)
                                    {
                                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d set new highest fibre contact count\n", commSlot);
                                        contactPathCount.fibre.highestContactCount = fibreContactCountArray[commSlot];

                                        contactPathCount.quorum.highestContactCount = quorumContactCountArray[commSlot];

                                        contactPathCount.goodPorts.highestContactCount = goodPortCountArray[commSlot];

                                        contactPathCount.fibre.highestContactCountTies = 0;
                                        contactPathCount.quorum.highestContactCountTies = 0;
                                        contactPathCount.goodPorts.highestContactCountTies = 0;
                                    }
                                    else if (fibreContactCountArray[commSlot] == contactPathCount.fibre.highestContactCount)
                                    {
                                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d is tied for highest fibre contact count\n", commSlot);
                                        contactPathCount.fibre.highestContactCountTies++;

                                        /*
                                         * Track the controllers that have the highest quorum contact count,
                                         * keeping track of ties for the highest number of contacts.
                                         */
                                        if (quorumContactCountArray[commSlot] > contactPathCount.quorum.highestContactCount)
                                        {
                                            dprintf(DPRINTF_ELECTION, "CACCS: Slot %d set new highest quorum contact count\n", commSlot);

                                            contactPathCount.quorum.highestContactCount = quorumContactCountArray[commSlot];

                                            contactPathCount.goodPorts.highestContactCount = goodPortCountArray[commSlot];

                                            contactPathCount.quorum.highestContactCountTies = 0;
                                            contactPathCount.goodPorts.highestContactCountTies = 0;
                                        }
                                        else if (quorumContactCountArray[commSlot] == contactPathCount.quorum.highestContactCount)
                                        {
                                            dprintf(DPRINTF_ELECTION, "CACCS: Slot %d is tied for highest quorum contact count\n", commSlot);

                                            contactPathCount.quorum.highestContactCountTies++;

                                            /*
                                             * Track the controllers that have the highest number of targets,
                                             * keeping track of ties for the highest number of contacts.
                                             */
                                            if (goodPortCountArray[commSlot] > contactPathCount.goodPorts.highestContactCount)
                                            {
                                                dprintf(DPRINTF_ELECTION, "CACCS: Slot %d set new goodPorts count\n", commSlot);

                                                contactPathCount.goodPorts.highestContactCount = goodPortCountArray[commSlot];

                                                contactPathCount.goodPorts.highestContactCountTies = 0;
                                            }
                                            else if (goodPortCountArray[commSlot] == contactPathCount.goodPorts.highestContactCount)
                                            {
                                                dprintf(DPRINTF_ELECTION, "CACCS: Slot %d is tied for goodPorts count\n", commSlot);

                                                contactPathCount.goodPorts.highestContactCountTies++;
                                            }
                                            else
                                            {
                                                dprintf(DPRINTF_ELECTION, "CACCS: Slot %d has fewer goodPorts\n", commSlot);
                                            }
                                        }
                                        else
                                        {
                                            dprintf(DPRINTF_ELECTION, "CACCS: Slot %d has fewer quorum contacts\n", commSlot);
                                        }
                                    }
                                    else
                                    {
                                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d has fewer fibre contacts\n", commSlot);
                                    }
                                }
                                else
                                {
                                    dprintf(DPRINTF_ELECTION, "CACCS: Slot %d has fewer ethernet contacts\n", commSlot);
                                }
                            }
                            else
                            {
                                dprintf(DPRINTF_ELECTION, "CACCS: Slot %d has less icon conntectivity\n", commSlot);
                            }
                        }
                        else
                        {
                            dprintf(DPRINTF_ELECTION, "CACCS: Slot %d has less mastership ability\n", commSlot);
                        }
                    }
                    else if ((EL_IsElectionSerialNewer(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                               &gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial) == 1) &&
                             ((gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_TIMEOUT_CONTROLLERS) ||
                              (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE)))
                    {
                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d is still processing the timeout condition\n", commSlot);
                    }
                    else if ((EL_IsElectionSerialOlder(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                               &gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial) == 2) &&
                             ((gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_CHECK_MASTER) ||
                              (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_NOTIFY_SLAVES)))
                    {
                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d has taken the mastership path\n", commSlot);

                        /*
                         * If the controller has continued with the election,
                         * we'll mark it as ready just so we don't keep waiting
                         * for it to timeout.
                         */
                        readyControllerCounter++;

                        /*
                         * Set the flag to indicate that a master has been found
                         */
                        isTheNewMasterFound = TRUE;
                    }
                    else if ((EL_HowManyHaveTalkedToSlot(commSlot) == 0) &&
                             ((EL_IsElectionSerialNewer(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                                &gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial) > 0) ||
                              (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_FAILED) ||
                              (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_FINISHED)))
                    {
                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d no longer part of election\n", commSlot);

                        dprintf(DPRINTF_ELECTION, "CACCS:   LocalSN: %d  SlotSN: %d\n",
                                myControllerCommArea.electionStateSector.electionData.electionSerial.current,
                                gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current);

                        dprintf(DPRINTF_ELECTION, "CACCS:   Contacts: %d\n", EL_HowManyHaveTalkedToSlot(commSlot));

                        /*
                         * If the controller is no longer participating in the election,
                         * we'll mark it as ready just so we don't need to keep waiting
                         * for it to timeout.
                         */
                        readyControllerCounter++;
                    }
                    else if ((EL_IsElectionSerialOlder(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                               &gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial) == 1) &&
                             (myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] != ED_CONTACT_MAP_CONTACTED_ETHERNET) &&
                             (myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] != ED_CONTACT_MAP_CONTACTED_FIBRE) &&
                             (myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] != ED_CONTACT_MAP_CONTACTED_QUORUM) &&
                             ((gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_NOTIFY_SLAVES) ||
                              (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_TIMEOUT_CONTROLLERS) ||
                              (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE)))
                    {
                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d has looks to have died after taking mastership\n", commSlot);

                        /*
                         * If the controller is no longer participating in the election,
                         * we'll mark it as ready just so we don't need to keep waiting
                         * for it to timeout.
                         */
                        readyControllerCounter++;
                    }
                    else if (EL_IsElectionSerialOlder(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                              &gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial) > 0)
                    {
                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d has a newer electionSerial.current - we missed it!\n", commSlot);

                        /*
                         * Check that this controller's electionSerial.current isn't outdated.
                         * If it is, we missed a timeout notification, so we should fail.
                         */
                        returnCode = ERROR;
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "CACCS: Slot %d hasn't reached CONTACT_COMPLETE yet\n", commSlot);

                        /*
                         * Check to see if any controllers have been able to talk
                         * to the controller that's not yet ready.
                         */
                        if (EL_HowManyHaveTalkedToSlot(commSlot) == 0)
                        {
                            dprintf(DPRINTF_ELECTION, "CACCS: Slot %d is not responsive to any controllers in the VCG\n", commSlot);

                            unresponsiveControllerCounter++;
                        }
                        else
                        {
                            dprintf(DPRINTF_ELECTION, "CACCS: Slot %d was responsive to a controller in the VCG\n", commSlot);
                        }
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "CACCS: Slot %d is inactive, skipping contact count\n", commSlot);

                    /*
                     * If the controller is no longer participating in the election,
                     * we'll mark it as ready just so we don't need to keep waiting
                     * for it to timeout.
                     */
                    readyControllerCounter++;
                }
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "CACCS: Error returned from ReadAllCommunications\n");

            returnCode = ERROR;
        }

        /*
         * Only do a k$twait if we'll stay in the while loop when we wake up.
         */
        if ((readyControllerCounter < ACM_GetActiveControllerCount(&electionACM)) &&
            (EL_GetCurrentState() == ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE) &&
            (GetCACCSTimeoutCounter() > 0) && (returnCode == GOOD))
        {
            SetCACCSTimeoutCounter(GetCACCSTimeoutCounter() - 1);

            if (GetCACCSTimeoutCounter() > 0)
            {
                TaskSleepMS(CONTACT_ALL_CONTROLLERS_COMPLETE_TIMEOUT / CHECK_FOR_TIMEOUT_GRANULARITY);
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "CACCS: Timeout waiting for all controllers to complete\n");
            }
        }
    }

    /*
     * At this point, we're done waiting for all of the other controllers and
     * we're ready to either decide our new rank in the VCG, or to timeout and
     * restart the election process.
     */
    if ((EL_GetCurrentState() == ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE) && (returnCode == GOOD))
    {
        /*
         * Display the current contactMap information for this controller
         */
        for (activeMapCounter = 0;
             activeMapCounter < ACM_GetActiveControllerCount(&electionACM);
             activeMapCounter++)
        {
            commSlot = EL_GetACMNode(activeMapCounter);

            EL_GetContactMapStateString(tempString1,
                                        myControllerCommArea.electionStateSector.electionData.contactMap[commSlot],
                                        sizeof(tempString1));

            dprintf(DPRINTF_ELECTION, "CACCS: ContactMap slot %d: %s\n", commSlot, tempString1);
        }

        /*
         * Look to see if another controller has gone master.  This path
         * should only be followed on specific types of timeout conditions,
         * otherwise mastership should be determined by the connectivity.
         */
        if (isTheNewMasterFound == TRUE)
        {
            dprintf(DPRINTF_ELECTION, "CACCS: Another controller has taken the master path\n");

            if (myControllerCommSlot == previousMasterCommSlot)
            {
                dprintf(DPRINTF_ELECTION, "CACCS: This controller is switching to slave\n");

                returnCode = EL_NotifyOtherTasks(ELECTION_SWITCHING_TO_SLAVE);
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "CACCS: This controller is staying a slave\n");

                returnCode = EL_NotifyOtherTasks(ELECTION_STAYING_SLAVE);
            }
        }
        else if (readyControllerCounter == ACM_GetActiveControllerCount(&electionACM))
        {
            /*
             * Check if this controller has timed out while waiting for the other
             * controllers to complete their CONTACT_ALL_CONTROLLERS phase.
             * If the controller has not timed out waiting for all of the controllers
             * to reach the CONTACT_ALL_CONTROLLERS_COMPLETE phase, establish its rank
             * in the VCG based upon its contacts that were found during the
             * CONTACT_ALL_CONTROLLERS phase.
             */

            dprintf(DPRINTF_ELECTION, "CACCS: All controllers are COMPLETE\n");

            /*
             * Determine if the controller should be the new master by looking
             * at the number of contacts generated in the CONTACT_ALL_CONTROLLERS phase.
             * Give the mastership to the controller with the highest number of contacts.
             * In the case of a tie, preference should be given to the controller that
             * was the previous master (if it's participating in the election), otherwise
             * give the new mastership to the controller that was closest to master
             * in the active control map that also has the highest number of contacts.
             */
            if ((masterCapabilityArray[myControllerCommSlot] >= contactPathCount.masterCapability.highestContactCount) &&
                (iconConnectivityArray[myControllerCommSlot] >= contactPathCount.iconConnectivity.highestContactCount) &&
                (ethernetContactCountArray[myControllerCommSlot] >= contactPathCount.ethernet.highestContactCount) &&
                (fibreContactCountArray[myControllerCommSlot] >= contactPathCount.fibre.highestContactCount) &&
                (quorumContactCountArray[myControllerCommSlot] >= contactPathCount.quorum.highestContactCount) &&
                (goodPortCountArray[myControllerCommSlot] >= contactPathCount.goodPorts.highestContactCount))
            {
                dprintf(DPRINTF_ELECTION, "CACCS: Looking at connectivity to determine VCG rank\n");

                /*
                 * Check if the previous master can take mastership again
                 */
                if ((previousMasterCommSlot != ACM_NODE_UNDEFINED) &&
                    ((masterCapabilityArray[previousMasterCommSlot] >= contactPathCount.masterCapability.highestContactCount) &&
                     (iconConnectivityArray[previousMasterCommSlot] >= contactPathCount.iconConnectivity.highestContactCount) &&
                     (ethernetContactCountArray[previousMasterCommSlot] >= contactPathCount.ethernet.highestContactCount) &&
                     (fibreContactCountArray[previousMasterCommSlot] >= contactPathCount.fibre.highestContactCount) &&
                     (quorumContactCountArray[previousMasterCommSlot] >= contactPathCount.quorum.highestContactCount) &&
                     (goodPortCountArray[previousMasterCommSlot] >= contactPathCount.goodPorts.highestContactCount)))
                {
                    /*
                     * Previous master is going to take mastership again.  Check if this
                     * controller was the previous master so we know where to transition to.
                     */
                    if (myControllerCommSlot == previousMasterCommSlot)
                    {
                        dprintf(DPRINTF_ELECTION, "CACCS: This controller will attempt to retain mastership\n");

                        isMyControllerNewMaster = TRUE;
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "CACCS: Previous master (slot %d) should keep mastership\n", previousMasterCommSlot);

                        returnCode = EL_NotifyOtherTasks(ELECTION_STAYING_SLAVE);
                    }
                }
                else if ((contactPathCount.masterCapability.highestContactCountTies == 0) ||
                         (contactPathCount.iconConnectivity.highestContactCountTies == 0) ||
                         (contactPathCount.ethernet.highestContactCountTies == 0) ||
                         (contactPathCount.fibre.highestContactCountTies == 0) ||
                         (contactPathCount.quorum.highestContactCountTies == 0) ||
                         (contactPathCount.goodPorts.highestContactCountTies == 0))
                {
                    /*
                     * Not tied with any other controllers
                     */
                    dprintf(DPRINTF_ELECTION, "CACCS: This controller will attempt to take mastership (best connectivity)\n");

                    isMyControllerNewMaster = TRUE;
                }
                else
                {
                    /*
                     * Tied with at least one other controller.
                     * Find who we're tied with.  If we're tied with the controller
                     * that was previously master, then let it should have taken control
                     * up above.  The tied controller with the lowest (closest to master)
                     * position in the active control map wins.
                     */
                    for (activeMapCounter = 0;
                         ((activeMapCounter < ACM_GetActiveControllerCount(&electionACM)) && (isTheNewMasterFound == FALSE));
                         activeMapCounter++)
                    {
                        commSlot = EL_GetACMNode(activeMapCounter);

                        if ((masterCapabilityArray[commSlot] >= contactPathCount.masterCapability.highestContactCount) &&
                            (iconConnectivityArray[commSlot] >= contactPathCount.iconConnectivity.highestContactCount) &&
                            (ethernetContactCountArray[commSlot] >= contactPathCount.ethernet.highestContactCount) &&
                            (fibreContactCountArray[commSlot] >= contactPathCount.fibre.highestContactCount) &&
                            (quorumContactCountArray[commSlot] >= contactPathCount.quorum.highestContactCount) &&
                            (goodPortCountArray[commSlot] >= contactPathCount.goodPorts.highestContactCount))
                        {
                            isTheNewMasterFound = TRUE;

                            if (commSlot == myControllerCommSlot)
                            {
                                dprintf(DPRINTF_ELECTION, "CACCS: This controller will attempt to take mastership (closest to previous master)\n");

                                isMyControllerNewMaster = TRUE;
                            }
                            else
                            {
                                dprintf(DPRINTF_ELECTION, "CACCS: Controller in slot %d should attempt to take mastership (closer to previous master)\n", commSlot);

                                returnCode = EL_NotifyOtherTasks(ELECTION_STAYING_SLAVE);
                            }
                        }
                    }
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "CACCS: Another controller has better connectivity\n");

                if (myControllerCommSlot == previousMasterCommSlot)
                {
                    dprintf(DPRINTF_ELECTION, "CACCS: This controller is switching to slave\n");

                    returnCode = EL_NotifyOtherTasks(ELECTION_SWITCHING_TO_SLAVE);
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "CACCS: This controller is staying a slave\n");

                    returnCode = EL_NotifyOtherTasks(ELECTION_STAYING_SLAVE);
                }
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "CACCS:   %d ready, %d unresponsive, %d total\n",
                    readyControllerCounter, unresponsiveControllerCounter,
                    ACM_GetActiveControllerCount(&electionACM));

            if ((readyControllerCounter + unresponsiveControllerCounter) == ACM_GetActiveControllerCount(&electionACM))
            {
                dprintf(DPRINTF_ELECTION, "CACCS: All *responsive* controllers are COMPLETE\n");
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "CACCS: Not all responsive controllers are COMPLETE\n");
            }

            /*
             * We've timed out waiting for all the controllers to respond.
             * Force the election to go through a timeout phase.
             */
            EL_ChangeState(ED_STATE_TIMEOUT_CONTROLLERS);
        }
    }

    /*
     * If the controller is a possible master, switch states to
     * check if it's capable of being a master controller.
     * Otherwise switch states so that we can wait for the
     * master controller to contact this controller.
     */
    if ((EL_GetCurrentState() == ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE) && (returnCode == GOOD))
    {
        if (isMyControllerNewMaster == TRUE)
        {
            dprintf(DPRINTF_ELECTION, "CACCS: This controller is trying to become master\n");

            EL_ChangeState(ED_STATE_CHECK_MASTER);
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "CACCS: This controller is becoming a slave\n");

#ifdef ELECTION_INJECT_MASTERSHIP_COLLISION
            if (mastershipCollisionInjectionCounter > 0)
            {
                dprintf(DPRINTF_ELECTION, "CACCS: ***** Mastership collision injection counter is now %d *****\n", mastershipCollisionInjectionCounter);

                mastershipCollisionInjectionCounter--;
                contactedByMaster = FALSE;
                EL_ChangeState(ED_STATE_WAIT_FOR_MASTER);
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "CACCS: ***** Attempting to inject mastership collision *****\n");

                mastershipCollisionInjectionCounter = 4;
                EL_ChangeState(ED_STATE_CHECK_MASTER);
            }
#else   /* ELECTION_INJECT_MASTERSHIP_COLLISION */
            /*
             * Initialize the election globals for becoming a slave controller,
             * which are used in the WAIT_FOR_MASTER state.  Do this here instead
             * of at the beginning of WaitForMasterState so that an incoming packet
             * from the new master doesn't get lost.  The new master may send a
             * CONTACT_SLAVE packet as soon as it sees this controller switch
             * to the WAIT_FOR_MASTER state.
             */
            contactedByMaster = FALSE;

            EL_ChangeState(ED_STATE_WAIT_FOR_MASTER);
#endif  /* ELECTION_INJECT_MASTERSHIP_COLLISION */
        }
    }

    if (returnCode == GOOD)
    {
        /*
         * Check for any disaster conditions before continuing.
         */
        if (EL_DisasterCheck() == GOOD)
        {
            dprintf(DPRINTF_ELECTION, "CACCS: Disaster check passed\n");
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "CACCS: Disaster check failed - stop election and take action\n");

            EL_DisasterTakeAction("System check failed");
            returnCode = ERROR;
        }

#ifdef ELECTION_INJECT_FRAGMENTATION_CHECK_FAILURE
        if (returnCode == GOOD)
        {
            if (fragmentationCheckInjectionCounter > 0)
            {
                if (K_timel & 1)
                {
                    dprintf(DPRINTF_ELECTION, "CACCS: ***** Fragmentation check injection counter is now %d *****\n", fragmentationCheckInjectionCounter);

                    fragmentationCheckInjectionCounter--;
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "CACCS: ***** Attempting to inject fragmentation check failure *****\n");

                fragmentationCheckInjectionCounter = 4;
                returnCode = ERROR;
            }
        }
#endif  /* ELECTION_INJECT_FRAGMENTATION_CHECK_FAILURE */
    }

    /*
     * Fail if anything went wrong
     */
    if (returnCode != GOOD)
    {
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_WaitForMasterState
**--------------------------------------------------------------------------*/
static void EL_WaitForMasterState(void)
{
    /*
     * Coming from states:
     *   ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE
     *   ED_STATE_CHECK_MASTER
     *   ED_STATE_TIMEOUT_CONTROLLERS
     *   ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE
     *
     * Going to states:
     *   ED_STATE_FINISHED (election complete)
     *   ED_STATE_TIMEOUT_CONTROLLERS (timeout)
     *   ED_STATE_FAILED (missed or failed election)
     */
    QM_CONTROLLER_COMM_AREA *masterCommArea;
    UINT32      timeoutCounter = CHECK_FOR_TIMEOUT_GRANULARITY;
    UINT32      returnCode = GOOD;
    UINT8       waitForMaster = TRUE;
    char        tempString[TEMP_STRING_LENGTH] = { 0 };

#ifdef ELECTION_INJECT_TIMEOUT_DURING_WFMS_FINISHED
    static UINT8 wfmsFinishedTimeoutInjectionCounter = 4;
#endif  /* ELECTION_INJECT_TIMEOUT_DURING_WFMS_FINISHED */

    UINT8       acmCounter = 0;

#ifdef ELECTION_INJECT_TIMEOUT_DURING_WFMS
    static UINT8 wfmsTimeoutInjectionCounter = 4;

    if (wfmsTimeoutInjectionCounter > 0)
    {
        if (K_timel & 1)
        {
            wfmsTimeoutInjectionCounter--;

            dprintf(DPRINTF_ELECTION, "WFMS: ***** WFMS injection counter is now %d *****\n", wfmsTimeoutInjectionCounter);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "WFMS: ***** Attempting to inject timeout during WFM state *****\n");

        wfmsTimeoutInjectionCounter = 4;
        timeoutCounter = 1;
    }
#endif  /* ELECTION_INJECT_TIMEOUT_DURING_WFMS */

    /*
     * Wait for the EL_PacketReceptionHandler to receive
     * the contact-by-master packet.  It will set the contactedByMaster
     * variable to TRUE when it sees this packet.
     */
    while ((contactedByMaster != TRUE) &&
           (EL_GetCurrentState() == ED_STATE_WAIT_FOR_MASTER) && (timeoutCounter > 0))
    {
        timeoutCounter--;

        if (timeoutCounter > 0)
        {
            TaskSleepMS(WAIT_FOR_MASTER_TIMEOUT / CHECK_FOR_TIMEOUT_GRANULARITY);

            /*
             * After this task wakes up, check to see if this controller has been
             * failed by the new master... don't wait until we timeout waiting for
             * the new master to check the failureData.
             */
            if (contactedByMaster != TRUE)
            {
                /*
                 * Check this controller's failure data, to make sure it hasn't been
                 * failed by another controller during the election process.  Don't
                 * use the 'with retries' version of the ReadFailureData, since it
                 * could take longer to complete than is desired.  A bad return from
                 * the read here shouldn't directly cause the controller to suicide.
                 */
                if (ReadFailureData(myControllerSerialNumber, &myControllerCommArea.failStateSector.failureData) == 0)
                {
                    EL_GetFailureDataStateString(tempString,
                                                 myControllerCommArea.failStateSector.failureData.state,
                                                 sizeof(tempString));

                    dprintf(DPRINTF_ELECTION, "WFMS: This controller failureData.state is %s\n", tempString);

                    if (myControllerCommArea.failStateSector.failureData.state == FD_STATE_FAILED)
                    {
                        /*
                         * Force the election to the FAILED state - another controller failed this controller
                         */
                        EL_ChangeState(ED_STATE_FAILED);
                    }
                }
                else
                {
                    /*
                     * Don't die simply because we couldn't read the quorum.  We can
                     * try to read this again in the next cycle though the wait loop.
                     */
                    dprintf(DPRINTF_ELECTION, "WFMS: Controller unable to read its comm slot (Slot %d)\n", myControllerCommSlot);
                }
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "WFMS: Timeout waiting for master to notify slave\n");
        }
    }

    if (EL_GetCurrentState() == ED_STATE_WAIT_FOR_MASTER)
    {
        if (contactedByMaster == TRUE)
        {
            dprintf(DPRINTF_ELECTION, "WFMS: This controller has been contacted by master\n");

            /*
             * Set local cached controller failure state
             */
            SetControllerFailureState(FD_STATE_OPERATIONAL);

            /*
             * The master has contacted us, so load the new configuration and
             * start up all the slave tasks.
             */
            if (LoadMasterConfiguration() == 0)
            {
                /*
                 * Save the new masterConfig into our NVRAM
                 */
                StoreMasterConfigToNVRAM();

                /*
                 * Copy the ACM from the quorum's masterConfig into the
                 * election code's protected ACM structure.
                 */
                memcpy(&electionACM, &masterConfig.activeControllerMap, sizeof(electionACM));

                /*
                 * Dump the ACM portion of the new masterConfig.  Do not
                 * rebuild the ACM, as the code has other dependencies upon
                 * that data structure.
                 */
                dprintf(DPRINTF_ELECTION, "WFMS: New masterConfig loaded\n");
                dprintf(DPRINTF_ELECTION, "WFMS:   New election serial: %08x\n",
                        Qm_GetElectionSerial());

                for (acmCounter = 0; acmCounter < MAX_CONTROLLERS; acmCounter++)
                {
                    dprintf(DPRINTF_ELECTION, "WFMS:   New ACM[%d] - Slot %d\n", acmCounter, EL_GetACMNode(acmCounter));
                }

                /*
                 * We need to wait here for the master to complete its election.  We want
                 * to prevent this slave from being able to start another election before
                 * the new master is ready.  We'll wait for the new master to complete for
                 * the same duration that it waits for the slaves to respond to its
                 * CONTACT_SLAVES message.  We'll check the master's state by reading its
                 * comm area (quorum), since it'll update it when exiting from the election.
                 */
                timeoutCounter = CHECK_FOR_TIMEOUT_GRANULARITY;

#ifdef ELECTION_INJECT_TIMEOUT_DURING_WFMS_FINISHED
                if (wfmsFinishedTimeoutInjectionCounter > 0)
                {
                    if (K_timel & 1)
                    {
                        wfmsFinishedTimeoutInjectionCounter--;

                        dprintf(DPRINTF_ELECTION, "WFMS: ***** WFMSF injection counter is now %d *****\n", wfmsFinishedTimeoutInjectionCounter);
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "WFMS: ***** Attempting to inject timeout during WFMS(F) state *****\n");

                    wfmsFinishedTimeoutInjectionCounter = 4;
                    timeoutCounter = 1;
                }
#endif  /* ELECTION_INJECT_TIMEOUT_DURING_WFMS_FINISHED */

                masterCommArea = MallocSharedWC(sizeof(*masterCommArea));
                while ((waitForMaster == TRUE) && (timeoutCounter > 0) &&
                       (EL_GetCurrentState() == ED_STATE_WAIT_FOR_MASTER) &&
                       (returnCode == GOOD))
                {
                    /*
                     * Read the new master's comm area to pick up its current election state.
                     */
                    if (ReadCommAreaWithRetries(Qm_GetActiveCntlMap(0), masterCommArea) == 0)
                    {
                        dprintf(DPRINTF_ELECTION, "WFMS: Master's serial number: %d/%d\n",
                                masterCommArea->electionStateSector.electionData.electionSerial.starting,
                                masterCommArea->electionStateSector.electionData.electionSerial.current);

                        /*
                         * Check to see that the master controller is still running, and wait
                         * until it completes the election before allowing this controller to
                         * run as a slave.  If the master gets stuck, the operation should
                         * time out and a new election will need to clean up the mess.
                         */
                        if (masterCommArea->failStateSector.failureData.state == FD_STATE_FAILED)
                        {
                            dprintf(DPRINTF_ELECTION, "WFMS: Master failureData.state is FAILED\n");

                            waitForMaster = FALSE;
                        }
                        else if (masterCommArea->electionStateSector.electionData.state == ED_STATE_END_TASK)
                        {
                            dprintf(DPRINTF_ELECTION, "WFMS: Master electionData is END_TASK (done with election)\n");

                            waitForMaster = FALSE;
                        }
                        else if (masterCommArea->electionStateSector.electionData.electionSerial.current != Qm_GetElectionSerial())
                        {
                            dprintf(DPRINTF_ELECTION, "WFMS: Master electionSerial has changed (started another election)\n");

                            waitForMaster = FALSE;
                        }
                        else
                        {
                            dprintf(DPRINTF_ELECTION, "WFMS: Waiting for master to complete election\n");

                            timeoutCounter--;

                            if (timeoutCounter > 0)
                            {
                                TaskSleepMS(NOTIFY_SLAVES_COMPLETE_TIMEOUT / CHECK_FOR_TIMEOUT_GRANULARITY);
                            }
                            else
                            {
                                dprintf(DPRINTF_ELECTION, "WFMS: Timeout waiting for master to complete\n");
                            }
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "WFMS: Error reading masterCommArea\n");

                        returnCode = ERROR;
                    }
                }
                Free(masterCommArea);
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "WFMS: Error reading the new master config\n");

                returnCode = ERROR;
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "WFMS: Timeout waiting for new master\n");

            /*
             * We've timed out waiting for the master to complete
             * its election, so restart the election and continue.
             */
            EL_ChangeState(ED_STATE_TIMEOUT_CONTROLLERS);
        }
    }

    if ((EL_GetCurrentState() == ED_STATE_WAIT_FOR_MASTER) && (returnCode == GOOD))
    {
        /*
         * Now that the master is ready (or has timed out) notify the other tasks
         * so that we function as a slave.  If the master has timed out, starting
         * up the slave tasks will eventually cause another election to occur,
         * and the master timeout will get cleaned up.
         */
        if (EL_NotifyOtherTasks(ELECTION_AM_SLAVE) == GOOD)
        {
            EL_ChangeState(ED_STATE_FINISHED);
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "WFMS: Error notifying tasks about being a slave\n");

            returnCode = ERROR;
        }
    }

    if (returnCode != GOOD)
    {
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_CheckMasterState
**--------------------------------------------------------------------------*/
static void EL_CheckMasterState(void)
{
    /*
     * Coming from states:
     *   ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE
     *
     * Going to states:
     *   ED_STATE_NOTIFY_SLAVES (verified master, has control of VCG)
     *   ED_STATE_WAIT_FOR_MASTER (another master found)
     *   ED_STATE_FAILED (this controller failed)
     */
    UINT32      timeoutCounter = CHECK_FOR_TIMEOUT_GRANULARITY;
    UINT16      myACMNode = ACM_NODE_UNDEFINED;
    UINT8       readyControllerCounter = 0;
    UINT8       activeMapCounter = 0;
    UINT8       takeMastershipFlag = TRUE;
    UINT8       returnCode = GOOD;
    UINT8       commSlot = 0;
    char        tempString[TEMP_STRING_LENGTH];

#ifdef ELECTION_INJECT_TIMEOUT_DURING_CMS
    static UINT8 cmsTimeoutInjectionCounter = 4;

    if (cmsTimeoutInjectionCounter > 0)
    {
        if (K_timel & 1)
        {
            cmsTimeoutInjectionCounter--;

            dprintf(DPRINTF_ELECTION, "CMS: ***** CM injection counter is now %d *****\n",
                    cmsTimeoutInjectionCounter);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "CMS: ***** Attempting to inject timeout during CM state *****\n");

        cmsTimeoutInjectionCounter = 4;
        timeoutCounter = 1;
    }
#endif  /* ELECTION_INJECT_TIMEOUT_DURING_CMS */

    /*
     * Wait for all other controllers to detect that another
     * controller is going to become their new master.
     * Chop the timeout into segments, and check for completion
     * each time the process get awakened.
     */
    while ((readyControllerCounter < ACM_GetActiveControllerCount(&electionACM)) &&
           (EL_GetCurrentState() == ED_STATE_CHECK_MASTER) &&
           (takeMastershipFlag == TRUE) && (timeoutCounter > 0) && (returnCode == GOOD))
    {
        readyControllerCounter = 0;

        if (ReadAllCommunicationsWithRetries(gElectCommAreas) == 0)
        {
            /*
             * Scan through the election data for all controllers and see that they
             * also agree that this controller is the new master.
             */
            for (activeMapCounter = 0;
                 ((activeMapCounter < ACM_GetActiveControllerCount(&electionACM)) &&
                  (takeMastershipFlag == TRUE) && (returnCode == GOOD));
                 activeMapCounter++)
            {
                commSlot = EL_GetACMNode(activeMapCounter);

                if (commSlot != myControllerCommSlot)
                {
                    EL_GetElectionStateString(tempString,
                                              gElectCommAreas[commSlot].electionStateSector.electionData.state,
                                              sizeof(tempString));

                    dprintf(DPRINTF_ELECTION, "CMS: Slot %d state is %s\n", commSlot, tempString);

                    /*
                     * Check to be sure the controller is not failed before looking at
                     * its contact map information.
                     */
                    if (gElectCommAreas[commSlot].failStateSector.failureData.state != FD_STATE_FAILED)
                    {
                        /*
                         * Check the electionSerial.current before looking at the contact map.  It is
                         * possible that the controller is dead, and is not responsive, but
                         * still has valid contact map information.  In this case, we'll have timed
                         * out that controller, and its serial number will be outdated.  In any case,
                         * the controller must have the correct election serial number in
                         * order for this controller to look at its contacts.  If the electionSerial.current
                         * is valid, look at the controller and count it as being 'ready' only if
                         * is in the WAIT_FOR_MASTER state.  Also, if the controller
                         * is in the END_TASK state, it means that the controller
                         * failed the election, or is already done for some reason.
                         * In either case, it's no longer participating in the election,
                         * so we need to count it as ready so we can continue.
                         */
                        if ((gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current ==
                             myControllerCommArea.electionStateSector.electionData.electionSerial.current) &&
                            (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_WAIT_FOR_MASTER))
                        {
                            dprintf(DPRINTF_ELECTION, "CMS: Slot %d is ready\n", commSlot);

                            readyControllerCounter++;
                        }
                        else if ((gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current ==
                                  myControllerCommArea.electionStateSector.electionData.electionSerial.current) &&
                                 (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_NOTIFY_SLAVES))
                        {
                            /*
                             * If we discover that another controller is also thinking that
                             * it can take over mastership, then we need to gracefully handle
                             * this case.  We will look at the controller's slot number
                             * and use it to decide which controller gets mastership.  We will
                             * give mastership to the controller with the lowest slot number.
                             * Do not mark the controller as being ready, though, since this
                             * situation will get resolved by one of the conflicting controllers
                             * switching to slave (WAIT_FOR_MASTER).
                             */
                            dprintf(DPRINTF_ELECTION, "CMS: Slot %d has already taken mastersip\n", commSlot);

                            takeMastershipFlag = FALSE;
                        }
                        else if ((gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current ==
                                  myControllerCommArea.electionStateSector.electionData.electionSerial.current) &&
                                 (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_CHECK_MASTER))
                        {
                            /*
                             * If we discover that another controller is also thinking that
                             * it can take over mastership, then we need to gracefully handle
                             * this case.  We will look at the controller's slot number
                             * and use it to decide which controller gets mastership.  We will
                             * give mastership to the controller with the lowest ACM position.
                             * Do not mark the controller as being ready, though, since this
                             * situation will get resolved by one of the conflicting controllers
                             * switching to slave (WAIT_FOR_MASTER).
                             */
                            dprintf(DPRINTF_ELECTION, "CMS: Slot %d is also wanting to become master\n", commSlot);

                            if ((ACM_GetNodeBySN(&electionACM, &myACMNode, myControllerSerialNumber) == GOOD) &&
                                (myACMNode <= activeMapCounter))
                            {
                                dprintf(DPRINTF_ELECTION, "CMS: This controller outranks other controller (closer to master)\n");
                            }
                            else
                            {
                                dprintf(DPRINTF_ELECTION, "CMS: This controller is outranked by other controller (farther from master)\n");

                                takeMastershipFlag = FALSE;
                            }

                            dprintf(DPRINTF_ELECTION, "CMS:   MyACMNode: %d  OtherNode: %d\n", myACMNode, activeMapCounter);
                        }
                        else if ((EL_HowManyHaveTalkedToSlot(commSlot) == 0) &&
                                 ((EL_IsElectionSerialNewer(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                                    &gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial) > 0) ||
                                  (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_FAILED) ||
                                  (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_FINISHED)))
                        {
                            dprintf(DPRINTF_ELECTION, "CMS: Slot %d no longer part of election\n", commSlot);

                            dprintf(DPRINTF_ELECTION, "CMS:   LocalSN: %d  SlotSN: %d\n",
                                    myControllerCommArea.electionStateSector.electionData.electionSerial.current,
                                    gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current);

                            dprintf(DPRINTF_ELECTION, "CMS:   Contacts: %d\n", EL_HowManyHaveTalkedToSlot(commSlot));

                            /*
                             * If the controller is no longer participating in the election,
                             * we'll mark it as ready just so we don't need to keep waiting
                             * for it to timeout.
                             */
                            readyControllerCounter++;
                        }
                        else if ((EL_IsElectionSerialOlder(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                                   &gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial) == 1) &&
                                 (myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] != ED_CONTACT_MAP_CONTACTED_ETHERNET) &&
                                 (myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] != ED_CONTACT_MAP_CONTACTED_FIBRE) &&
                                 (myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] != ED_CONTACT_MAP_CONTACTED_QUORUM) &&
                                 ((gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_NOTIFY_SLAVES) ||
                                  (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_TIMEOUT_CONTROLLERS) ||
                                  (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE)))
                        {
                            dprintf(DPRINTF_ELECTION, "CMS: Slot %d has looks to have died after taking mastership\n", commSlot);

                            /*
                             * If the controller is no longer participating in the election,
                             * we'll mark it as ready just so we don't need to keep waiting
                             * for it to timeout.
                             */
                            readyControllerCounter++;
                        }
                        else if (EL_IsElectionSerialNewer(&gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial,
                                  &myControllerCommArea.electionStateSector.electionData.electionSerial) > 0)
                        {
                            dprintf(DPRINTF_ELECTION, "CMS: Slot %d has a newer electionSerial.current - we missed it!\n", commSlot);

                            /*
                             * Check that this controller's electionSerial.current isn't outdated.
                             * If it is, we missed a timeout notification, so we should fail.
                             */
                            returnCode = ERROR;
                        }
                        else
                        {
                            dprintf(DPRINTF_ELECTION, "CMS: Slot %d hasn't reached WAIT_FOR_MASTER yet\n", commSlot);
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "CMS: Slot %d is FAILED, skipping master check\n", commSlot);

                        /*
                         * If the controller is no longer participating in the election,
                         * we'll mark it as ready just so we don't need to keep waiting
                         * for it to timeout.
                         */
                        readyControllerCounter++;
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "CMS: This controller might take mastership\n");

                    readyControllerCounter++;
                }
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "CMS: Error returned from ReadAllCommunications\n");

            returnCode = ERROR;
        }

        /*
         * Keep checking for all the controllers until the timeout
         * has been satisfied.  If the controller doesn't respond
         * within the timeout window, it'll get failed when this
         * controller picks up the mastership and rebuilds the ACM.
         */
        if ((readyControllerCounter < ACM_GetActiveControllerCount(&electionACM)) &&
            (EL_GetCurrentState() == ED_STATE_CHECK_MASTER) &&
            (takeMastershipFlag == TRUE) && (returnCode == GOOD))
        {
            timeoutCounter--;

            if (timeoutCounter > 0)
            {
                TaskSleepMS(CHECK_MASTER_TIMEOUT / CHECK_FOR_TIMEOUT_GRANULARITY);
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "CMS: Done waiting in CheckMaster\n");
            }
        }
    }

    if ((EL_GetCurrentState() == ED_STATE_CHECK_MASTER) && (returnCode == GOOD))
    {
        if (takeMastershipFlag == TRUE)
        {
            if (myControllerCommSlot == previousMasterCommSlot)
            {
                if (EL_NotifyOtherTasks(ELECTION_STAYING_MASTER) != GOOD)
                {
                    dprintf(DPRINTF_ELECTION, "CMS: Error notifying tasks about STAYING_MASTER\n");

                    returnCode = ERROR;
                }
            }
            else
            {
                if (EL_NotifyOtherTasks(ELECTION_SWITCHING_TO_MASTER) != GOOD)
                {
                    dprintf(DPRINTF_ELECTION, "CMS: Error notifying tasks about SWITCHING_TO_MASTER\n");

                    returnCode = ERROR;
                }
            }

            if (returnCode == GOOD)
            {
                /*
                 * Take control of the VCG
                 */
                if (EL_TakeMastership() == GOOD)
                {
                    /*
                     * Tell all the other tasks that this controller is the new master.
                     */
                    if (EL_NotifyOtherTasks(ELECTION_AM_MASTER) == GOOD)
                    {
                        /*
                         * Switch states to notify all slaves of the new mastership
                         */
                        EL_ChangeState(ED_STATE_NOTIFY_SLAVES);
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "CMS: Error notifying tasks about AM_MASTER\n");

                        returnCode = ERROR;
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "CMS: Error returned from TaskMastership\n");

                    returnCode = ERROR;
                }
            }
        }
        else
        {
            /*
             * Mastership conflict - this controller is going slave
             */
            if (myControllerCommSlot == previousMasterCommSlot)
            {
                if (EL_NotifyOtherTasks(ELECTION_SWITCHING_TO_SLAVE) != GOOD)
                {
                    dprintf(DPRINTF_ELECTION, "CMS: Error notifying tasks about SWITCHING_TO_SLAVE\n");

                    returnCode = ERROR;
                }
            }
            else
            {
                if (EL_NotifyOtherTasks(ELECTION_STAYING_SLAVE) != GOOD)
                {
                    dprintf(DPRINTF_ELECTION, "CMS: Error notifying tasks about STAYING_SLAVE\n");

                    returnCode = ERROR;
                }
            }

            if (returnCode == GOOD)
            {
                /*
                 * Initialize the election globals for becoming a slave controller,
                 * which are used in the WAIT_FOR_MASTER state.  Do this here instead
                 * of at the beginning of WaitForMasterState so that an incoming packet
                 * from the new master doesn't get lost.  The new master may send a
                 * CONTACT_SLAVE packet as soon as it sees this controller switch
                 * to the WAIT_FOR_MASTER state.
                 */
                contactedByMaster = FALSE;

                EL_ChangeState(ED_STATE_WAIT_FOR_MASTER);
            }
        }
    }

    if (returnCode != GOOD)
    {
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_NotifySlavesState
**--------------------------------------------------------------------------*/
static void EL_NotifySlavesState(void)
{
    /*
     * Coming from states:
     *   ED_STATE_CHECK_MASTER
     *
     * Going to states:
     *   ED_STATE_FINISHED
     *   ED_STATE_FAILED (error encountered)
     */
    SESSION    *notifyControllerSessionPtr = NULL;
    UINT32      notifyControllerSerialNumber = 0;
    UINT32      timeoutCounter = 0;
    UINT8       activeControllerCounter = 0;
    UINT8       notifyControllerSlot = 0;
    UINT8       returnCode = GOOD;
    UINT8       retries;

#ifdef ELECTION_INJECT_TIMEOUT_DURING_NSS
    static UINT8 nssTimeoutInjectionCounter = 4;

    if (nssTimeoutInjectionCounter > 0)
    {
        if (K_timel & 1)
        {
            nssTimeoutInjectionCounter--;

            dprintf(DPRINTF_ELECTION, "NSS: ***** NS injection counter is now %d *****\n", nssTimeoutInjectionCounter);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "NSS: ***** Attempting to inject timeout during NSS state *****\n");

        nssTimeoutInjectionCounter = 4;
        timeoutCounter = 1;
    }
#endif  /* ELECTION_INJECT_TIMEOUT_DURING_NSS */

    if (EL_GetPreviousState() == ED_STATE_CHECK_MASTER)
    {

    }
    else
    {
        /*
         * Illegal transition into NOTIFY_SLAVES
         */
        EL_ControlTaskStateError();
        returnCode = ERROR;
    }

    if (returnCode == GOOD)
    {
        /*
         * Clear the variable that tracks the number of controllers that
         * will respond to the notify packets.
         */
        myNumberOfControllersNotified = 0;

        /*
         * Set flag to indicate to NotifySlavesCallback that we're in the
         * NotifySlavesState function.  We need to communicate to the callback
         * function that we're now going to be expecting the callbacks, so it needs
         * to count the responses.  Without this flag being set, the callback event
         * will be ignored.
         */
        SetElectionNotifySlavesFlag(TRUE);

        dprintf(DPRINTF_ELECTION, "NSS: Notifying all remaining slaves\n");

        /*
         * Send election-start messages to all active controllers
         * in the controller group.  In the event that a controller is
         * non-responsive, time out and continue without the
         * dead controller.
         */
        for (activeControllerCounter = 0;
             ((activeControllerCounter < ACM_GetActiveControllerCount(&electionACM)) &&
              (EL_GetCurrentState() == ED_STATE_NOTIFY_SLAVES) && (returnCode == GOOD));
             activeControllerCounter++)
        {
            /*
             * Get the slot number for the active controller that is to be notified
             */
            notifyControllerSlot = EL_GetACMNode(activeControllerCounter);

            /*
             * Get the serial number for the controller that is to be notified
             */
            notifyControllerSerialNumber = GetControllerSN(notifyControllerSlot);

            /*
             * Validate the controller serial number
             */
            if (notifyControllerSerialNumber != 0)
            {
                /*
                 * Make certain that the controller doesn't send a packet to itself.
                 */
                if (notifyControllerSerialNumber != myControllerSerialNumber)
                {
                    /*
                     * Indicate to all controllers that we're notifying the target controller
                     */
                    if (EL_SetContactMapItem(notifyControllerSlot, ED_CONTACT_MAP_NOTIFY_SLAVE) == GOOD)
                    {
                        /*
                         * Get the session pointer for the controller being notified
                         */
                        notifyControllerSessionPtr = GetSession(notifyControllerSerialNumber);

                        /*
                         * Validate the session pointer
                         */
                        if (notifyControllerSessionPtr != NULL)
                        {
                            /*
                             * Create an election packet
                             */
                            notifyControllerList[activeControllerCounter].txPacketPtr = CreatePacket(PACKET_IPC_ELECT, sizeof(IPC_ELECT), __FILE__, __LINE__);

                            /*
                             * Fill in the current election serial numbers for this controller
                             */
                            notifyControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.electionSerial.starting =
                                myControllerCommArea.electionStateSector.electionData.electionSerial.starting;

                            notifyControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.electionSerial.current =
                                myControllerCommArea.electionStateSector.electionData. electionSerial.current;

                            notifyControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.messageType =
                                IPC_ELECT_NOTIFY_SLAVE;

                            notifyControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.electionTaskState =
                                ED_STATE_NOTIFY_SLAVES;

                            /*
                             * Place this controller's current diskMap into the transmit packet
                             */
                            EL_DiskMapGet(&notifyControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR3.diskMap);

                            /*
                             * Allocate the receive packet header/data structure
                             * Note: The callback function is responsible for freeing the memory allocated
                             *       for the rxPacket header and data pointers.
                             */
                            notifyControllerList[activeControllerCounter].rxPacketPtr = MallocWC(sizeof(IPC_PACKET));

                            notifyControllerList[activeControllerCounter].rxPacketPtr->header = NULL;
                            notifyControllerList[activeControllerCounter].rxPacketPtr->data = NULL;

                            /*
                             * Place the pointer to the receive packet into the callback parameters
                             * so that EL_PacketReceptionHandler can analyze the rxPacket data.
                             */
                            notifyControllerList[activeControllerCounter].callback.rxPacketPtr =
                                notifyControllerList[activeControllerCounter].rxPacketPtr;

                            /*
                             * Place activeControllerCounter into the callback parameters so that the receive
                             * packet handler can identify which controller the response was sent from.
                             */
                            notifyControllerList[activeControllerCounter].callback.controllerSlotNumber = notifyControllerSlot;

                            /*
                             * Send the packet
                             */
                            dprintf(DPRINTF_ELECTION, "NSS: Sending packet to slot %d\n", notifyControllerSlot);

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacket with rxPacket of %p\n", __FILE__, __LINE__, __func__, notifyControllerList[activeControllerCounter].rxPacketPtr);
#endif  /* HISTORY_KEEP */

                            retries = 2;        /* Ethernet, Fiber(1), Disk Quorum(2) */
                            do
                            {
                                Free(notifyControllerList[activeControllerCounter].rxPacketPtr->data);

                                notifyControllerList[activeControllerCounter].callback.callbackResults.result =
                                IpcSendPacket(notifyControllerSessionPtr,                               /* Session */
                                SENDPACKET_ANY_PATH,                                                    /* Requested Path */
                                notifyControllerList[activeControllerCounter].txPacketPtr,              /* Transmit */
                                notifyControllerList[activeControllerCounter].rxPacketPtr,              /* Receive */
                                (void *)EL_NotifySlavesCallback,                                        /* Callback function */
                                (void *)&notifyControllerList[activeControllerCounter].callback,        /* Callback parameters */
                                &notifyControllerList[activeControllerCounter].callback.callbackResults, /* Callback results */
                                NOTIFY_SLAVES_RESPONSE_TIMEOUT);                                        /* Timeout */
                            } while (notifyControllerList[activeControllerCounter].callback.callbackResults.result == SENDPACKET_NO_PATH && (retries--) > 0);

                            /* Deallocate the transmit packet header/data memory */
                            FreePacket(&notifyControllerList[activeControllerCounter].txPacketPtr, __FILE__, __LINE__);
                        }
                        else
                        {
                            dprintf(DPRINTF_ELECTION, "NSS: notifyControllerSessionPtr is NULL\n");

                            /*
                             * We won't be getting a callback from the controller, so mark it as being
                             * notified such that we don't need to time out.
                             */
                            myNumberOfControllersNotified++;

                            /*
                             * Indicate to all controllers that we failed to notify the target controller
                             */
                            if (EL_SetContactMapItem(notifyControllerSlot, ED_CONTACT_MAP_NOTIFY_SLAVE_FAILED) != GOOD)
                            {
                                dprintf(DPRINTF_ELECTION, "NSS: Error writing contact map item\n");

                                returnCode = ERROR;
                            }
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "NSS: Error writing contact map item\n");

                        returnCode = ERROR;
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "NSS: Controller not sending packet to itself (slot %d)\n", notifyControllerSlot);

                    dprintf(DPRINTF_ELECTION, "NSS: Marking slot %d as MASTER_CONTROLLER\n", notifyControllerSlot);

                    /*
                     * Set the item in the contactMap that corresponds to the controller that
                     * made the callback.  This will be looked at later to determine all of
                     * the controllers that are participating in the election.
                     */
                    if (EL_SetContactMapItem(notifyControllerSlot, ED_CONTACT_MAP_MASTER_CONTROLLER) == GOOD)
                    {
                        /*
                         * Increment the response counter, since the controller will not
                         * be counted in NotifyControllerCallback.
                         */
                        myNumberOfControllersNotified++;
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "NSS: Error writing contact map item\n");

                        returnCode = ERROR;
                    }
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "NSS: Invalid controller serial number\n");

                returnCode = ERROR;
            }
        }
    }

    if ((EL_GetCurrentState() == ED_STATE_NOTIFY_SLAVES) && (returnCode == GOOD))
    {
        /*
         * Wait for all controllers to check in (or a predetermined
         * amount of time in the case of a timeout) for all controllers
         * to check in (using NotifySlavesCallback) before transitioning
         * out of the NOTIFY_SLAVES state.  Chop the timeout into segments,
         * and check for completion each time the process get awakened.
         */
        timeoutCounter = CHECK_FOR_TIMEOUT_GRANULARITY;
        while ((myNumberOfControllersNotified < ACM_GetActiveControllerCount(&electionACM)) &&
               (EL_GetCurrentState() == ED_STATE_NOTIFY_SLAVES) && (timeoutCounter > 0))
        {
            timeoutCounter--;

            if (timeoutCounter > 0)
            {
                TaskSleepMS(NOTIFY_SLAVES_COMPLETE_TIMEOUT / CHECK_FOR_TIMEOUT_GRANULARITY);
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "NSS: Timeout waiting for all controllers\n");
                dprintf(DPRINTF_ELECTION, "  (Got %d out of %d controllers)\n",
                        myNumberOfControllersNotified,
                        ACM_GetActiveControllerCount(&electionACM));
            }
        }

        if (EL_GetCurrentState() == ED_STATE_NOTIFY_SLAVES)
        {
            /*
             * Switch states now that all active controllers in the
             * controller group have been notified.
             */
            EL_ChangeState(ED_STATE_FINISHED);
        }
    }

    /*
     * Clear the flag that tells the callback function to handle
     * the timeout responses so that the callbacks that occur after
     * this time are ignored.
     */
    SetElectionNotifySlavesFlag(FALSE);

    if (returnCode != GOOD)
    {
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_NotifySlavesCallback
**--------------------------------------------------------------------------*/
static void EL_NotifySlavesCallback(TASK_PARMS *parms)
{
    PCONTACT_CONTROLLER_CALLBACK_PARMS callbackPtr = (PCONTACT_CONTROLLER_CALLBACK_PARMS)parms->p1;
    UINT8       callbackSlotNumber = 0;
    ELECTION_DATA_CONTACT_MAP_ITEM contactType = ED_CONTACT_MAP_NOTIFY_SLAVE_FAILED;

    char        tempString[TEMP_STRING_LENGTH];

    if (callbackPtr != NULL)
    {
        /*
         * Find which communications slot corresponds with the controller that
         * made the callback.
         */
        callbackSlotNumber = callbackPtr->controllerSlotNumber;

        if (callbackSlotNumber == myControllerCommSlot)
        {
            /*
             * A controller should never receive a callback from its own slot.
             * If some software bug causes the condition, however, make sure
             * we throw the packet away.
             */
            dprintf(DPRINTF_ELECTION, "NSCB: Callback from self (slot %d) ignored\n", callbackSlotNumber);
        }
        else if (callbackSlotNumber >= MAX_CONTROLLERS)
        {
            /*
             * Timeouts seem to show up with slot 255.  Filter out all slots
             * that are outside of the acceptable range.  If MAX_CONTROLLERS is
             * four, then we want to allow slots 0-3.
             */
            dprintf(DPRINTF_ELECTION, "NSCB: Callback from out-of-range slot (slot %d)\n", callbackSlotNumber);
        }
        else
        {
            EL_GetSendPacketResultString(tempString,
                                         callbackPtr->callbackResults.result,
                                         sizeof(tempString));

            dprintf(DPRINTF_ELECTION, "NSCB: Callback from slot %d result: %s\n", callbackSlotNumber, tempString);

            /*
             * Make certain that the controller is still in the
             * 'NOTIFY_SLAVES' state before marking the controller
             * that's making the callback as participating in the election.
             */
            if (TestElectionNotifySlavesFlag() == TRUE)
            {
                /*
                 * Check that IPC has returned a non-NULL rxPacketPtr... it appears
                 * as though it will be NULL for the case where IPC has a TIMEOUT.
                 */
                if ((callbackPtr->rxPacketPtr != NULL) && (callbackPtr->rxPacketPtr->data != NULL))
                {
                    /*
                     * Make certain that the callback is for the election that this
                     * controller is currently participating in.  If the election
                     * serial number doesn't match, then don't count it as a valid response.
                     * When the slave responds to the new master, its election serial
                     * number will be one less than the new master's election serial number.
                     */
                    if (EL_IsElectionSerialOlder(&callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionSerial,
                         &myControllerCommArea.electionStateSector.electionData.electionSerial) == 2)
                    {
                        /*
                         * Make certain that the callback has a good result that
                         * corresponds to one of the acceptable routes.  If not, the
                         * callback response is treated as a timeout (non-participate).
                         */
                        if ((callbackPtr->callbackResults.result == SENDPACKET_ETHERNET) ||
                            (callbackPtr->callbackResults.result == SENDPACKET_FIBRE) ||
                            (callbackPtr->callbackResults.result == SENDPACKET_QUORUM))
                        {
                            /*
                             * Track the new response from the controller that is doing the callback
                             */
                            myNumberOfControllersNotified++;

                            dprintf(DPRINTF_ELECTION, "NSCB: Marking slot %d as NOTIFIED\n", callbackSlotNumber);

                            /*
                             * Set the item in the contactMap that corresponds to the controller that
                             * made the callback.  This will be looked at later to determine all of
                             * the controllers that are participating in the election.
                             */
                            contactType = ED_CONTACT_MAP_SLAVE_NOTIFIED;

                            /*
                             * If the callback packet's responseValid bit is set, then
                             * process the diskMap in the response packet.
                             */
                            if ((callbackPtr->rxPacketPtr->header->length == sizeof(IPC_ELECT_R3)) &&
                                (callbackPtr->rxPacketPtr->data->elect.ipcElectR3.diskMap.flags.bits.responseValid == TRUE))
                            {
                                dprintf(DPRINTF_ELECTION, "NSCB: Response diskMap is valid\n");

                                EL_DiskMapReceive(&callbackPtr->rxPacketPtr->data->elect.ipcElectR3.diskMap, callbackSlotNumber);
                            }
                        }
                        else
                        {
                            /*
                             * Packet result is not the expected type
                             */
                            EL_GetSendPacketResultString(tempString,
                                                         callbackPtr->callbackResults.result,
                                                         sizeof(tempString));

                            dprintf(DPRINTF_ELECTION, "NSCB: Unexpected result in callback: result=%d (%s)\n",
                                    callbackPtr->callbackResults.result, tempString);

                            /*
                             * Set the item in the contactMap that corresponds to the controller that
                             * made the callback.  This will be looked at later to determine all of
                             * the controllers that are participating in the election.
                             */
                            contactType = ED_CONTACT_MAP_NOTIFY_SLAVE_FAILED;
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "NSCB: Invalid election serial number from slot %d\n", callbackSlotNumber);

                        dprintf(DPRINTF_ELECTION, "    Controller: %d  Packet: %d\n",
                                myControllerCommArea.electionStateSector.electionData.electionSerial.current,
                                callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionSerial.current);

                        /*
                         * Set the item in the contactMap that corresponds to the controller that
                         * made the callback.  This will be looked at later to determine all of
                         * the controllers that are participating in the election.
                         */
                        contactType = ED_CONTACT_MAP_NOTIFY_SLAVE_FAILED;

                        /*
                         * If we're in the process of notifying the slaves, then this
                         * controller is the new master controller, and should have the
                         * absolute newest election serial number.  If it ever happens
                         * that another controller gets a newer number, we need to fail
                         * because something is massively wrong.
                         */
                        if (EL_IsElectionSerialNewer(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                             &callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionSerial) > 0)
                        {
                            dprintf(DPRINTF_ELECTION, "NSCB: This serial is newer - stay running\n");
                        }
                        else
                        {
                            dprintf(DPRINTF_ELECTION, "NSCB: Newer serial number received - fail\n");

                            /*
                             * Force the FAILED state on a quorum write error
                             */
                            EL_ChangeState(ED_STATE_FAILED);
                        }
                    }
                }
                else
                {
                    /*
                     * It appears as though IPC has timed the packet out
                     */
                    dprintf(DPRINTF_ELECTION, "NSCB: NULL rxPacketPtr from slot %d\n", callbackSlotNumber);

                    /*
                     * Set the item in the contactMap that corresponds to the controller that
                     * made the callback.  This will be looked at later to determine all of
                     * the controllers that are participating in the election.
                     */
                    contactType = ED_CONTACT_MAP_NOTIFY_SLAVE_TIMEOUT;
                }

                EL_GetContactMapStateString(tempString, contactType, sizeof(tempString));

                dprintf(DPRINTF_ELECTION, "NSCB: Marking slot %d as %s\n", callbackSlotNumber, tempString);

                /*
                 * Write the contactType to the quorum
                 */
                if (EL_SetContactMapItem(callbackSlotNumber, contactType) != GOOD)
                {
                    /*
                     * Force the FAILED state on a quorum write error
                     */
                    EL_ChangeState(ED_STATE_FAILED);
                }
            }
            else
            {
                /*
                 * The controller that made the callback responded too late to the
                 * 'NOTIFY_SLAVES', so its not going to participate in the election.
                 */
                dprintf(DPRINTF_ELECTION, "NSCB: Callback from slot %d ignored\n", callbackSlotNumber);
            }
        }

        /* Deallocate the receive packet memory */
        FreePacket(&callbackPtr->rxPacketPtr, __FILE__, __LINE__);
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "NSCB: Callback pointer is NULL\n");

        /*
         * Force the FAILED state on a quorum write error
         */
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_FailedState
**
**  Description: The election process is ending beacuse of an error.
**               Clean up any details that pertain to a failing controller.
**
**--------------------------------------------------------------------------*/
static void EL_FailedState(void)
{
    /*
     * Coming from states:
     *   All election states
     *   PacketReceptionHandler
     *
     * Going to states:
     *   ED_STATE_FINISHED
     *
     * NOTE: This is also the method that the election will exit on
     *       any unexpected error, so don't analyze the state
     *       transition for validity.
     */

    /*
     * Set local cached controller failure state
     */
    SetControllerFailureState(FD_STATE_FAILED);

#ifdef ELECTION_TESTING
    dprintf(DPRINTF_ELECTION, "FS: Controller FAILED - pause to flush debug output\n");

    TaskSleepMS(5000);
#endif /* ELECTION_TESTING */

    /*
     * Let all the other multi-controller tasks know that
     * this controller is going away.
     */
    EL_NotifyOtherTasks(ELECTION_FAILED);

    /*
     * Switch states to FINISHED so the election task can clean
     * up and details that pertain to an election ending with
     * any type of exit condition.
     */
    EL_ChangeState(ED_STATE_FINISHED);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_FinishedState
**--------------------------------------------------------------------------*/
static void EL_FinishedState(void)
{
    UINT32      returnCode = GOOD;

    /*
     * Coming from states:
     *   ED_STATE_WAIT_FOR_MASTER (controller is now slave - exit)
     *   ED_STATE_NOTIFY_SLAVES (controller is now master - exit)
     *   ED_STATE_FAILED (error - exit)
     *
     * Going to states:
     *   ED_STATE_END_TASK
     */
    if ((EL_GetPreviousState() == ED_STATE_NOTIFY_SLAVES) ||
        (EL_GetPreviousState() == ED_STATE_WAIT_FOR_MASTER))
    {
        /*
         * Becoming new master or slave (active)
         */
        dprintf(DPRINTF_ELECTION, "FINISHED: Setting previouslyActive flag\n");
        SetPreviouslyActiveFlag(TRUE);
    }
    else if ((EL_GetPreviousState() == ED_STATE_CONTACT_ALL_CONTROLLERS) &&
             ((myControllerCommArea.failStateSector.failureData.state == FD_STATE_FIRMWARE_UPDATE_INACTIVE) ||
              (myControllerCommArea.failStateSector.failureData.state == FD_STATE_INACTIVATED) ||
              (myControllerCommArea.failStateSector.failureData.state == FD_STATE_DISASTER_INACTIVE)))
    {
        /*
         * Inactive controller exiting election
         */
        dprintf(DPRINTF_ELECTION, "FINISHED: Clearing previouslyActive flag\n");
        SetPreviouslyActiveFlag(FALSE);

        /*
         * Wait for the other controller(s) to complete the election we just started
         */
        EL_WaitForInactivateComplete();
    }
    else if (EL_GetPreviousState() == ED_STATE_FAILED)
    {
        /*
         * This is here so that we pass the state transition requirements.
         * Details pertaining to a failed election were cleaned up in the
         * EL_FailedState function.
         */
    }
    else
    {
        /*
         * Illegal transition into FINISHED state
         */
        EL_ControlTaskStateError();
        returnCode = ERROR;
    }

#ifdef ELECTION_SLOW_FINISH
    while ((returnCode == GOOD) && (K_timel & 1) && (EL_GetCurrentState() == ED_STATE_FINISHED))
    {
        dprintf(DPRINTF_ELECTION, "FINISHED: ***** Injecting slow finish *****\n");

        TaskSleepMS(1000);
    }
#endif  /* ELECTION_SLOW_FINISH */

    /*
     * If all is well with the controller, or the controller has already
     * processed the FAILED state, allow the election to end gracefully.
     */
    if ((returnCode == GOOD) || (EL_GetPreviousState() == ED_STATE_FAILED))
    {
        dprintf(DPRINTF_ELECTION, "FINISHED: Shutting down the election task\n");

        /*
         * Switch states to kill the election control task.
         */
        EL_ChangeState(ED_STATE_END_TASK);

        /*
         * Reset this controller's contact map
         */
        EL_ClearContactMap();

        /*
         * Make sure the keepAlive unfail flag is cleared.
         */
        EL_KeepAliveSetUnfail(FALSE);

        /*
         * If we've completed an election without failing, then make sure
         * the disaster safeguard is bypassed.
         */
        if ((EL_GetPreviousState() != ED_STATE_FAILED) && (EL_DisasterCheckSafeguard() != GOOD))
        {
            EL_DisasterBypassSafeguard();
        }

        /*
         * Allow the server I/O to resume, now that we no longer need to have
         * high-priority (low latency) access to the quorum.
         */
        if (EL_StartIO() != GOOD)
        {
            dprintf(DPRINTF_ELECTION, "FINISHED: EL_StartIO return is not GOOD\n");

            returnCode = ERROR;
        }
    }

    if ((returnCode != GOOD) && (EL_GetPreviousState() != ED_STATE_FAILED))
    {
        /*
         * This controller encountered an error in the FINISHED state
         * and it has not previously gone through the FAILED state,
         * so send the controller through the FAILED state.
         */
        EL_ChangeState(ED_STATE_FAILED);
    }
    else
    {
        /*
         * Let the other tasks know that the election process is complete.
         * NOTE: We're performing the notification as late as possible in
         *       this function.  Doing this here means that there's less
         *       work before the election task is destroyed, and it's
         *       desirable for the FINISHED message to be presented as close
         *       to the end of the election as possible.
         */
        EL_NotifyOtherTasks(ELECTION_FINISHED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_TimeoutControllersState
**--------------------------------------------------------------------------*/
static void EL_TimeoutControllersState(void)
{
    /*
     * Coming from states:
     *   ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE
     *   ED_STATE_WAIT_FOR_MASTER
     *   PacketReceptionHandler
     *
     * Going to states:
     *   ED_STATE_CONTACT_ALL_CONTROLLERS
     *   ED_STATE_WAIT_FOR_MASTER
     *   ED_STATE_FAILED (error encountered)
     */
    SESSION    *timeoutControllerSessionPtr = NULL;
    UINT32      timeoutControllerSerialNumber = 0;
    UINT8       activeControllerCounter = 0;
    UINT8       timeoutControllerSlot = 0;
    UINT8       returnCode = GOOD;
    UINT8       retries;

    dprintf(DPRINTF_ELECTION, "TC: Handling controller communication timeout\n");

    /*
     * Clear the variable that tracks the number of controllers that
     * will respond to the timeout packets.
     */
    myNumberOfControllersTimedOut = 0;

    /*
     * Set flag to indicate to TimeoutControllersCallback that we're in the
     * TimeoutControllersState function.  We need to communicate to the
     * callback function that we're now going to be expecting the callbacks,
     * so it needs to count the responses.  Without this flag being set,
     * the callback event will be ignored.
     */
    SetElectionTimeoutControllersFlag(TRUE);

    /*
     * Send election-start messages to all active controllers
     * in the controller group.  In the event that a controller is
     * non-responsive, time out and continue without the
     * dead controller.
     */
    for (activeControllerCounter = 0;
         ((activeControllerCounter < ACM_GetActiveControllerCount(&electionACM)) &&
          (EL_GetCurrentState() == ED_STATE_TIMEOUT_CONTROLLERS) && (returnCode == GOOD));
         activeControllerCounter++)
    {
        /*
         * Get the slot number for the active controller that is to be contacted
         */
        timeoutControllerSlot = EL_GetACMNode(activeControllerCounter);

        /*
         * Get the serial number for the controller that is to be contacted
         */
        timeoutControllerSerialNumber = GetControllerSN(timeoutControllerSlot);

        /*
         * Validate the controller serial number
         */
        if (timeoutControllerSerialNumber != 0)
        {
            /*
             * Make certain that the controller doesn't send a packet to itself.
             */
            if (timeoutControllerSerialNumber != myControllerSerialNumber)
            {
                /*
                 * Get the session pointer for the controller being contacted
                 */
                timeoutControllerSessionPtr = GetSession(timeoutControllerSerialNumber);

                /*
                 * Validate the session pointer
                 */
                if (timeoutControllerSessionPtr != NULL)
                {
                    /*
                     * Create an election packet
                     */
                    timeoutControllerList[activeControllerCounter].txPacketPtr =
                        CreatePacket(PACKET_IPC_ELECT, sizeof(IPC_ELECT), __FILE__, __LINE__);

                    /*
                     * Fill in the current election serial numbers for this controller
                     */
                    timeoutControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.electionSerial.starting =
                        myControllerCommArea.electionStateSector.electionData.electionSerial.starting;

                    timeoutControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.electionSerial.current =
                        myControllerCommArea.electionStateSector.electionData.electionSerial.current;

                    timeoutControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.messageType = IPC_ELECT_TIMEOUT;

                    timeoutControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR1.electionTaskState =
                        ED_STATE_TIMEOUT_CONTROLLERS;

                    /*
                     * Place this controller's current diskMap into the transmit packet
                     */
                    EL_DiskMapGet(&timeoutControllerList[activeControllerCounter].txPacketPtr->data->elect.ipcElectR3.diskMap);

                    /*
                     * Allocate the receive packet header/data structure
                     * Note: The callback function is responsible for freeing the memory allocated
                     *       for the rxPacket header and data pointers.
                     */
                    timeoutControllerList[activeControllerCounter].rxPacketPtr = MallocWC(sizeof(IPC_PACKET));

                    timeoutControllerList[activeControllerCounter].rxPacketPtr->header = NULL;
                    timeoutControllerList[activeControllerCounter].rxPacketPtr->data = NULL;

                    /*
                     * Place the pointer to the receive packet into the callback parameters
                     * so that EL_PacketReceptionHandler can analyze the rxPacket data.
                     */
                    timeoutControllerList[activeControllerCounter].callback.rxPacketPtr =
                        timeoutControllerList[activeControllerCounter].rxPacketPtr;

                    /*
                     * Place activeControllerCounter into the callback parameters so that the receive
                     * packet handler can identify which controller the response was sent from.
                     */
                    timeoutControllerList[activeControllerCounter].callback.controllerSlotNumber = timeoutControllerSlot;

                    /*
                     * Send the packet
                     */
                    dprintf(DPRINTF_ELECTION, "TC: Sending packet to slot %d\n", timeoutControllerSlot);

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacket with rxPacket of %p\n", __FILE__, __LINE__, __func__, timeoutControllerList[activeControllerCounter].rxPacketPtr);
#endif  /* HISTORY_KEEP */

                    retries = 2;        /* Ethernet, Fiber(1), Disk Quorum(2) */
                    do
                    {
                        Free(timeoutControllerList[activeControllerCounter].rxPacketPtr->data);

                        timeoutControllerList[activeControllerCounter].callback.callbackResults.result =
                            IpcSendPacket(timeoutControllerSessionPtr,                              /* Session */
                            SENDPACKET_ANY_PATH,                                                    /* Requested Path */
                            timeoutControllerList[activeControllerCounter].txPacketPtr,             /* Transmit */
                            timeoutControllerList[activeControllerCounter].rxPacketPtr,             /* Receive */
                            (void *)EL_TimeoutControllersCallback,                                  /* Callback function */
                            (void *)&timeoutControllerList[activeControllerCounter].callback,       /* Callback parameters */
                            &timeoutControllerList[activeControllerCounter].callback.callbackResults, /* Callback results */
                            TIMEOUT_CONTROLLERS_TIMEOUT);                                           /* Timeout */
                    } while (timeoutControllerList[activeControllerCounter].callback.callbackResults.result == SENDPACKET_NO_PATH && (retries--) > 0);

                    /* Deallocate the transmit packet header/data memory */
                    FreePacket(&timeoutControllerList[activeControllerCounter].txPacketPtr, __FILE__, __LINE__);
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "TC: timeoutControllerSessionPtr is NULL\n");

                    /*
                     * This isn't necessarily a fatal error... it just means that we
                     * don't have a connection to the controller.  If this controller
                     * assumes mastership, the controller that we can't connect to
                     * will get failed when we rebuild the ACM.
                     */
                    myNumberOfControllersTimedOut++;

                    /*
                     * Indicate to all controllers that we failed to timeout the target controller
                     */
                    if (EL_SetContactMapItem(timeoutControllerSlot, ED_CONTACT_MAP_TIMEOUT_CONTROLLER_FAILED) != GOOD)
                    {
                        dprintf(DPRINTF_ELECTION, "TC: Error writing contact map item\n");

                        returnCode = ERROR;
                    }
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "TC: Controller not sending packet to itself (slot %d)\n", timeoutControllerSlot);

                dprintf(DPRINTF_ELECTION, "TC: Marking slot %d as CONTROLLER_TIMED_OUT\n", timeoutControllerSlot);

                /*
                 * Set the item in the contactMap that corresponds to the controller that
                 * made the callback.  This will be looked at later to determine all of
                 * the controllers that are participating in the election.
                 */
                if (EL_SetContactMapItem(timeoutControllerSlot, ED_CONTACT_MAP_CONTROLLER_TIMED_OUT) == GOOD)
                {
                    /*
                     * Increment the response counter, since the controller will not
                     * be counted in ContactControllerCallback.
                     */
                    myNumberOfControllersTimedOut++;
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "TC: Error writing contact map item\n");

                    returnCode = ERROR;
                }
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "TC: Invalid controller serial number\n");

            returnCode = ERROR;
        }
    }

    if ((EL_GetCurrentState() == ED_STATE_TIMEOUT_CONTROLLERS) && (returnCode == GOOD))
    {
        /*
         * Wait for all controllers to check in (or a predetermined
         * amount of time in the case of a timeout) for all controllers
         * to check in (using TimeoutControllersCallback).  Chop the timeout
         * into segments, and check for completion each time the process
         * get awakened.
         */
        SetTCTimeoutCounter(CHECK_FOR_TIMEOUT_GRANULARITY);

        while ((myNumberOfControllersTimedOut < ACM_GetActiveControllerCount(&electionACM)) &&
               (GetTCTimeoutCounter() > 0) &&
               (EL_GetCurrentState() == ED_STATE_TIMEOUT_CONTROLLERS))
        {
            SetTCTimeoutCounter(GetTCTimeoutCounter() - 1);

            if (GetTCTimeoutCounter() > 0)
            {
                TaskSleepMS(TIMEOUT_CONTROLLERS_RESPONSE_TIMEOUT / CHECK_FOR_TIMEOUT_GRANULARITY);
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "TC: Timeout sending timeout message to all controllers\n");
                dprintf(DPRINTF_ELECTION, "  (Got %d out of %d controllers)\n",
                        myNumberOfControllersTimedOut,
                        ACM_GetActiveControllerCount(&electionACM));
            }
        }
    }

    /*
     * Clear the flag that tells the callback function to handle
     * the timeout responses so that the callbacks that occur after
     * this time are ignored.
     */
    SetElectionTimeoutControllersFlag(FALSE);

    if ((EL_GetCurrentState() == ED_STATE_TIMEOUT_CONTROLLERS) && (returnCode == GOOD))
    {
        /*
         * Reset the SetContinueFromTimeoutCompleteFlag before transitioning
         * into the TIMEOUT_CONTROLLERS_COMPLETE state.  In the event that
         * another controller proceeds with the election while we're still
         * waiting for all of the controllers to complete the timeout, we'll
         * use this flag to keep up with the other controller.
         */
        SetContinueFromTimeoutCompleteFlag(FALSE);

        /*
         * Controllers are timed out, so restart the contact phase
         */
        EL_ChangeState(ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE);
    }

    if (returnCode != GOOD)
    {
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_TimeoutControllersCallback
**
**  Returns:       Nothing
**--------------------------------------------------------------------------*/
static void EL_TimeoutControllersCallback(TASK_PARMS *parms)
{
    PCONTACT_CONTROLLER_CALLBACK_PARMS callbackPtr = (PCONTACT_CONTROLLER_CALLBACK_PARMS)parms->p1;
    UINT8       callbackSlotNumber = 0;
    ELECTION_DATA_CONTACT_MAP_ITEM contactType = ED_CONTACT_MAP_TIMEOUT_CONTROLLER_FAILED;

    char        tempString[TEMP_STRING_LENGTH];

    if (callbackPtr != NULL)
    {
        /*
         * Find which communications slot corresponds with the controller that
         * made the callback.
         */
        callbackSlotNumber = callbackPtr->controllerSlotNumber;

        if (callbackSlotNumber == myControllerCommSlot)
        {
            /*
             * A controller should never receive a callback from its own slot.
             * If some software bug causes the condition, however, make sure
             * we throw the packet away.
             */
            dprintf(DPRINTF_ELECTION, "TCCB: Callback from self (slot %d) ignored\n", callbackSlotNumber);
        }
        else if (callbackSlotNumber >= MAX_CONTROLLERS)
        {
            /*
             * Timeouts seem to show up with slot 255.  Filter out all slots
             * that are outside of the acceptable range.  If MAX_CONTROLLERS is
             * four, then we want to allow slots 0-3.
             */
            dprintf(DPRINTF_ELECTION, "TCCB: Callback from out-of-range slot (slot %d)\n", callbackSlotNumber);
        }
        else
        {
            EL_GetSendPacketResultString(tempString,
                                         callbackPtr->callbackResults.result,
                                         sizeof(tempString));

            dprintf(DPRINTF_ELECTION, "TCCB: Callback from slot %d result: %s\n", callbackSlotNumber, tempString);

            /*
             * Check if the election code is expecting callbacks from TIMEOUT packets.
             */
            if (TestElectionTimeoutControllersFlag() == TRUE)
            {
                /*
                 * Check that IPC has returned a non-NULL rxPacketPtr... it appears
                 * as though it will be NULL for the case where IPC has a TIMEOUT.
                 */
                if ((callbackPtr->rxPacketPtr != NULL) && (callbackPtr->rxPacketPtr->data != NULL))
                {
                    /*
                     * Make certain that the callback is for the election that this
                     * controller is currently participating in.  If the election
                     * serial number doesn't match, then don't count it as a valid
                     * response.
                     */
                    if (callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionSerial.current ==
                        myControllerCommArea.electionStateSector.electionData.electionSerial.current)
                    {
                        /*
                         * Make certain that the callback has a good result that
                         * corresponds to one of the acceptable routes.  If not, the
                         * callback response is treated as a timeout (non-participate).
                         * Set the item in the contactMap that corresponds to the controller that
                         * made the callback.
                         */
                        switch (callbackPtr->callbackResults.result)
                        {
                            case SENDPACKET_ETHERNET:
                            case SENDPACKET_FIBRE:
                            case SENDPACKET_QUORUM:
                                contactType = ED_CONTACT_MAP_CONTROLLER_TIMED_OUT;
                                break;

                            case SENDPACKET_TIME_OUT:
                            case SENDPACKET_NO_PATH:
                            case SENDPACKET_ANY_PATH:
                            default:
                                contactType = ED_CONTACT_MAP_TIMEOUT_CONTROLLER_FAILED;
                                break;
                        }

                        /*
                         * Increment the variable that tracks how many controllers we're received
                         * 'acknowledges' from about our transmission of the TIMEOUT packets.
                         */
                        myNumberOfControllersTimedOut++;

                        /*
                         * If the callback packet's responseValid bit is set, then
                         * process the diskMap in the response packet.
                         */
                        if ((callbackPtr->rxPacketPtr->header->length == sizeof(IPC_ELECT_R3)) &&
                            (callbackPtr->rxPacketPtr->data->elect.ipcElectR3.diskMap.flags.bits.responseValid == TRUE))
                        {
                            dprintf(DPRINTF_ELECTION, "TCCB: Response diskMap is valid\n");

                            EL_DiskMapReceive(&callbackPtr->rxPacketPtr->data->elect.ipcElectR3.diskMap, callbackSlotNumber);
                        }
                    }
                    else if ((EL_IsElectionSerialOlder(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                               &callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionSerial) == 2) &&
                             ((callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionTaskState == ED_STATE_CHECK_MASTER) ||
                              (callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionTaskState == ED_STATE_NOTIFY_SLAVES)))
                    {
                        dprintf(DPRINTF_ELECTION, "TCCB: New master appears slow\n");

                        /*
                         * Check for a CHECK_MASTER or NOTIFY_SLAVE response from the
                         * other controller and extend the timeout if one is detected.
                         * NOTE: This doesn't count as a true 'timed out' controller,
                         * since the 'going master' controller ignores the TIMEOUT
                         * message once it's taken mastership.
                         */
                        if (EL_GetCurrentElectionState() == ED_STATE_TIMEOUT_CONTROLLERS)
                        {
                            dprintf(DPRINTF_ELECTION, "TCCB: Extending TIMEOUT_CONTROLLERS timeout value\n");

                            /*
                             * Extend the TIMEOUT_CONTROLLERS timeout counter
                             */
                            SetTCTimeoutCounter(CHECK_FOR_TIMEOUT_GRANULARITY);
                        }
                        else if (EL_GetCurrentElectionState() ==
                                 ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE)
                        {
                            dprintf(DPRINTF_ELECTION, "TCCB: Extending TIMEOUT_CONTROLLERS_COMPLETE timeout value\n");

                            /*
                             * Extend the TIMEOUT_CONTROLLERS_COMPLETE timeout counter.
                             * We sometimes get this callback after we've transitioned
                             * into the COMPLETE state (IPC being slow), so we need to
                             * extend the COMPLETE state's timeout.
                             */
                            SetTCCSTimeoutCounter(CHECK_FOR_TIMEOUT_GRANULARITY);
                        }

                        /*
                         * If the callback packet's responseValid bit is set, then
                         * process the diskMap in the response packet.
                         */
                        if ((callbackPtr->rxPacketPtr->header->length == sizeof(IPC_ELECT_R3)) &&
                            (callbackPtr->rxPacketPtr->data->elect.ipcElectR3.diskMap.flags.bits.responseValid == TRUE))
                        {
                            dprintf(DPRINTF_ELECTION, "TCCB: Response diskMap is valid\n");

                            EL_DiskMapReceive(&callbackPtr->rxPacketPtr->data->elect.ipcElectR3.diskMap, callbackSlotNumber);
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "TCCB: Invalid election serial number from slot %d (%d)\n",
                                callbackSlotNumber,
                                callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionSerial.current);

                        contactType = ED_CONTACT_MAP_TIMEOUT_CONTROLLER_FAILED;
                    }
                }
                else
                {
                    /*
                     * It appears as though IPC has timed the packet out
                     */
                    dprintf(DPRINTF_ELECTION, "TCCB: NULL rxPacketPtr from slot %d\n", callbackSlotNumber);

                    contactType = ED_CONTACT_MAP_TIMEOUT_CONTROLLER_FAILED;
                }

                EL_GetContactMapStateString(tempString, contactType, sizeof(tempString));

                dprintf(DPRINTF_ELECTION, "TCCB: Marking slot %d as %s\n", callbackSlotNumber, tempString);

                /*
                 * Write the contactType to the quorum
                 */
                if (EL_SetContactMapItem(callbackSlotNumber, contactType) != GOOD)
                {
                    /*
                     * Force the FAILED state on a quorum write error
                     */
                    EL_ChangeState(ED_STATE_FAILED);
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "TCCB: Callback from slot %d ignored\n", callbackSlotNumber);
            }
        }

        /* Deallocate the receive packet memory */
        FreePacket(&callbackPtr->rxPacketPtr, __FILE__, __LINE__);
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "TCCB: Callback pointer is NULL\n");

        /*
         * Force the FAILED state on a quorum write error
         */
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_TimeoutControllersCompleteState
**--------------------------------------------------------------------------*/
static void EL_TimeoutControllersCompleteState(void)
{
    /*
     * Coming from states:
     *   ED_STATE_TIMEOUT_CONTROLLERS
     *
     * Going to states:
     *   ED_STATE_CONTACT_ALL_CONTROLLERS
     *   ED_STATE_WAIT_FOR_MASTER
     *   ED_STATE_FAILED (error encountered)
     */
    UINT8       activeMapCounter = 0;
    UINT8       readyControllerCounter = 0;
    UINT8       unresponsiveControllerCounter = 0;
    UINT8       returnCode = GOOD;
    UINT8       commSlot = 0;
    char        tempString[TEMP_STRING_LENGTH];

#ifdef ELECTION_INJECT_TIMEOUT_DURING_TCCS
    static UINT8 tccsTimeoutInjectionCounter = 4;
#endif  /* ELECTION_INJECT_TIMEOUT_DURING_TCCS */

    if (EL_GetPreviousState() == ED_STATE_TIMEOUT_CONTROLLERS)
    {

    }
    else
    {
        /*
         * Illegal transition into ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE state
         */
        EL_ControlTaskStateError();
        returnCode = ERROR;
    }

    /*
     * Set state timeout counter value
     */
    SetTCCSTimeoutCounter(CHECK_FOR_TIMEOUT_GRANULARITY);

    /*
     * Wait for all other controllers to complete their
     * TIMEOUT_CONTROLLERS state (or a predetermined
     * amount of time in the case of a timeout). Chop the
     * timeout into pieces, and check for completion
     * each time the process get awakened.
     */
#ifdef ELECTION_INJECT_TIMEOUT_DURING_TCCS
    if (tccsTimeoutInjectionCounter > 0)
    {
        if (K_timel & 1)
        {
            tccsTimeoutInjectionCounter--;

            dprintf(DPRINTF_ELECTION, "TCCS: ***** TCCS injection counter is now %d *****\n", tccsTimeoutInjectionCounter);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "TCCS: ***** Attempting to inject timeout during TCC state *****\n");

        tccsTimeoutInjectionCounter = 4;
        SetTCCSTimeoutCounter(1);
    }
#endif  /* ELECTION_INJECT_TIMEOUT_DURING_TCCS */

    while ((readyControllerCounter < ACM_GetActiveControllerCount(&electionACM)) &&
           (EL_GetCurrentState() == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE) &&
           (TestContinueFromTimeoutCompleteFlag() == FALSE) &&
           (GetTCCSTimeoutCounter() > 0) && (returnCode == GOOD))
    {
        /*
         * Reset the contact counters (inside the loop)
         */
        readyControllerCounter = 0;
        unresponsiveControllerCounter = 0;

        /*
         * Read the communications area belonging to the controller we need to analyze
         */
        if (ReadAllCommunicationsWithRetries(gElectCommAreas) == 0)
        {
            /*
             * Scan through the election data for all controllers
             */
            for (activeMapCounter = 0;
                 ((activeMapCounter < ACM_GetActiveControllerCount(&electionACM)) &&
                  (TestContinueFromTimeoutCompleteFlag() == FALSE) && (returnCode == GOOD));
                 activeMapCounter++)
            {
                commSlot = EL_GetACMNode(activeMapCounter);

                /*
                 * Check to be sure the controller is not failed before looking at
                 * its contact map information.
                 */
                if (gElectCommAreas[commSlot].failStateSector.failureData.state != FD_STATE_FAILED)
                {
                    EL_GetElectionStateString(tempString,
                                              gElectCommAreas[commSlot].electionStateSector.electionData.state,
                                              sizeof(tempString));

                    dprintf(DPRINTF_ELECTION, "TCCS: Slot %d state is %s\n", commSlot, tempString);

                    /*
                     * Check the electionSerial.current before looking at the contact map.  It is
                     * possible that the controller is dead, and is not responsive, but
                     * still has valid contact map information.  In this case, we will time
                     * out that controller, and its serial number will get outdated.  If
                     * the electionSerial.current is valid, look to see if the target controller
                     * has reached the TIMEOUT_CONTROLLER_COMPLETE phase.
                     */
                    if ((gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current ==
                         myControllerCommArea.electionStateSector.electionData.electionSerial.current) &&
                        (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE))
                    {
                        readyControllerCounter++;
                    }
                    else if ((EL_HowManyHaveTalkedToSlot(commSlot) == 0) &&
                             ((EL_IsElectionSerialNewer(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                                &gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial) > 0) ||
                              (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_FAILED) ||
                              (gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_FINISHED)))
                    {
                        dprintf(DPRINTF_ELECTION, "TCCS: Slot %d no longer part of election\n", commSlot);

                        dprintf(DPRINTF_ELECTION, "TCCS:   LocalSN: %d  SlotSN: %d\n",
                                myControllerCommArea.electionStateSector.electionData.electionSerial.current,
                                gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current);

                        dprintf(DPRINTF_ELECTION, "TCCS:   Contacts: %d\n", EL_HowManyHaveTalkedToSlot(commSlot));

                        /*
                         * If the controller is no longer participating in the election,
                         * we'll mark it as ready just so we don't need to keep waiting
                         * for it to timeout.
                         */
                        readyControllerCounter++;
                    }
                    else if (EL_IsElectionSerialOlder(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                              &gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial) == 1)
                    {
                        dprintf(DPRINTF_ELECTION, "TCCS: Slot %d has an electionSerial.current one newer than ours\n", commSlot);

                        dprintf(DPRINTF_ELECTION, "    Controller: %d  Slot: %d\n",
                                myControllerCommArea.electionStateSector.electionData.electionSerial.current,
                                gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current);

                        dprintf(DPRINTF_ELECTION, "TCCS:   It's probably done with the timeout and has continued\n");
                    }
                    else if ((gElectCommAreas[commSlot].electionStateSector.electionData.state == ED_STATE_NOTIFY_SLAVES) &&
                             (EL_IsElectionSerialOlder(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                               &gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial) == 2))
                    {
                        dprintf(DPRINTF_ELECTION, "TCCS: Slot %d is in NOTIFY_SLAVES state has an electionSerial.current two newer than ours\n", commSlot);

                        dprintf(DPRINTF_ELECTION, "    Controller: %d  Slot: %d\n",
                                myControllerCommArea.electionStateSector.electionData.electionSerial.current,
                                gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current);

                        dprintf(DPRINTF_ELECTION, "TCCS:   It's probably stuck in NOTIFY_SLAVES state\n");
                    }
                    else if (EL_IsElectionSerialNewer(&gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial,
                              &myControllerCommArea.electionStateSector.electionData.electionSerial) > 0)
                    {
                        dprintf(DPRINTF_ELECTION, "TCCS: Slot %d has a newer electionSerial.current - we missed it!\n", commSlot);

                        dprintf(DPRINTF_ELECTION, "    Controller: %d  Slot: %d\n",
                                myControllerCommArea.electionStateSector.electionData.electionSerial.current,
                                gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current);

                        /*
                         * Check that this controller's electionSerial.current isn't outdated.
                         * If it is, we missed a timeout notification, so we should fail.
                         */
                        returnCode = ERROR;
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "TCCS: Slot %d hasn't reached TIMEOUT_CONTROLLERS_COMPLETE yet\n", commSlot);

                        /*
                         * Check to see if any controllers have been able to talk
                         * to the controller that's not yet ready.
                         */
                        if (EL_HowManyHaveTalkedToSlot(commSlot) == 0)
                        {
                            dprintf(DPRINTF_ELECTION, "TCCS: Slot %d is not responsive to any controllers in the VCG\n", commSlot);

                            unresponsiveControllerCounter++;
                        }
                        else
                        {
                            dprintf(DPRINTF_ELECTION, "TCCS: Slot %d was responsive to a controller in the VCG\n", commSlot);
                        }
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "TCCS: Slot %d is FAILED, skipping contact count\n", commSlot);

                    /*
                     * If the controller is no longer participating in the election,
                     * we'll mark it as ready just so we don't need to keep waiting
                     * for it to timeout.
                     */
                    readyControllerCounter++;
                }
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "TCCS: Error returned from ReadAllCommunications\n");

            returnCode = ERROR;
        }

        /*
         * Only do a k$twait if we'll stay in the while loop when we wake up.
         */
        if ((readyControllerCounter < ACM_GetActiveControllerCount(&electionACM)) &&
            (EL_GetCurrentState() == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE) &&
            (TestContinueFromTimeoutCompleteFlag() == FALSE) &&
            (GetTCCSTimeoutCounter() > 0) && (returnCode == GOOD))
        {
            SetTCCSTimeoutCounter(GetTCCSTimeoutCounter() - 1);

            if (GetTCCSTimeoutCounter() > 0)
            {
                TaskSleepMS(TIMEOUT_CONTROLLERS_COMPLETE_TIMEOUT /
                            CHECK_FOR_TIMEOUT_GRANULARITY);
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "TCCS: Timeout waiting for all controllers to reach timeoutComplete\n");
            }
        }
    }

    /*
     * Switch states to restart the election.
     */
    if ((EL_GetCurrentState() == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE) && (returnCode == GOOD))
    {
        /*
         * Display the current contactMap information for this controller
         */
        for (activeMapCounter = 0;
             activeMapCounter < ACM_GetActiveControllerCount(&electionACM);
             activeMapCounter++)
        {
            commSlot = EL_GetACMNode(activeMapCounter);

            EL_GetContactMapStateString(tempString,
                                        myControllerCommArea.electionStateSector.electionData.contactMap[commSlot],
                                        sizeof(tempString));

            dprintf(DPRINTF_ELECTION, "TCCS: ContactMap slot %d: %s\n", commSlot, tempString);
        }

        if (TestContinueFromTimeoutCompleteFlag() == TRUE)
        {
            dprintf(DPRINTF_ELECTION, "TCCS: packetReceptionHandler told us to stop waiting\n");
        }
        else
        {
            /*
             * Check if this controller has timed out while waiting for the other
             * controllers to complete their TIMEOUT_CONTROLLERS phase.
             */
            if (readyControllerCounter == ACM_GetActiveControllerCount(&electionACM))
            {
                dprintf(DPRINTF_ELECTION, "TCCS: All controllers are TIMEOUT_COMPLETE\n");
            }
            else if ((readyControllerCounter + unresponsiveControllerCounter) ==
                     ACM_GetActiveControllerCount(&electionACM))
            {
                dprintf(DPRINTF_ELECTION, "TCCS: All *responsive* controllers are TIMEOUT_COMPLETE\n");
                dprintf(DPRINTF_ELECTION, "TCCS:   %d ready, %d unresponsive\n",
                        readyControllerCounter, unresponsiveControllerCounter);
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "TCCS: Proceeding even though all controllers are not TIMEOUT_COMPLETE\n");
            }
        }

        dprintf(DPRINTF_ELECTION, "TCCS: Writing newer electionSerial.current: %d\n",
                myControllerCommArea.electionStateSector.electionData.electionSerial.current);

        /*
         * Increment our electionSerial.current to indicate that we've progressed beyond
         * the timeout condition.  Any controllers not up to our electionSerial.current
         * will eventually be removed from the group.
         */
        myControllerCommArea.electionStateSector.electionData.electionSerial.current++;

        /*
         * We're restarting an election from a timeout condition.  We have
         * a bunch of residual data in the contactMap, so before we proceed,
         * clear it out.  This function writes myControllerCommArea.electionStateSector.electionData
         * to the quorum, which commits the new electionSerial.current to the commArea.
         */
        if (EL_ClearContactMap() == GOOD)
        {
            /*
             * Controllers are timed out, so restart the contact phase
             */
            EL_ChangeState(ED_STATE_CONTACT_ALL_CONTROLLERS);
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "TCCS: Error when clearing contactMap\n");

            returnCode = ERROR;
        }
    }

    if (returnCode != GOOD)
    {
        EL_ChangeState(ED_STATE_FAILED);
    }

    /*
     * Make sure that the SetContinueFromTimeoutCompleteFlag is false before
     * we leave the TIMEOUT_CONTROLLERS state.
     */
    SetContinueFromTimeoutCompleteFlag(FALSE);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_ControlTaskStateError
**
**  Modifies:      currentElectionState
**
**  Returns:       Nothing
**--------------------------------------------------------------------------*/
static void EL_ControlTaskStateError(void)
{
    /*
     * An unexpected error condition was detected in the election control
     * task.  Gather and log as much info as possible.
     */
    char        tempString1[TEMP_STRING_LENGTH];
    char        tempString2[TEMP_STRING_LENGTH];

    dprintf(DPRINTF_ELECTION, "CTSE: Election state transition error (from %d to %d)\n",
            EL_GetCurrentState(), EL_GetNextState());

    EL_GetTaskStateString(tempString1, EL_GetCurrentState(), sizeof(tempString1));

    EL_GetTaskStateString(tempString2, EL_GetNextState(), sizeof(tempString2));

    dprintf(DPRINTF_ELECTION, "CTSE: From %s to %s\n", tempString1, tempString2);

    EL_LogMessage(LOG_ILLEGAL_ELECTION_STATE, 4, ((EL_GetCurrentState() << 8) || EL_GetNextState()));

    /*
     * Switch states to kill the election control task.
     */
    EL_ChangeState(ED_STATE_FAILED);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_LogMessage
**
**  Description:    Generates and sends an alert message.  Creates an MRP
**                  type structure and sends it out.  This function currently
**                  only handles 1 parameter in the MRP.
**
**  Inputs:         messageType - type of alert
**                  length      - length of parameter data (0 or 4)
**                  parm1       - data in first parameter
**
**  Returns:        Nothing
**--------------------------------------------------------------------------*/
static void EL_LogMessage(INT32 messageType, INT32 length, UINT32 parm1)
{
    SendAsyncEvent(messageType, length, &parm1);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_BringUpInterfaceAllControllers
**
**  Description:   Calls BringUpInterface() on all controllers in the ACM.
**                 BringUpInterface() checks for a good connection before
**                 actually "bringing a new one up". If there is a good
**                 connection, it simply exits.
**
**  Inputs:        PATH_TYPE type - either SENDPACKET_ETHERNET
**                                  or SENDPACKET_FIBRE
**
**  Returns:       void
**--------------------------------------------------------------------------*/
void EL_BringUpInterfaceAllControllers(PATH_TYPE path)
{
    UINT32      count;
    UINT32      configIndex;
    UINT32      controllerSN;
    SESSION    *pSession = NULL;

    dprintf(DPRINTF_DEFAULT, "EL_BringUpInterfaceAllControllers: enter\n");

    for (count = 0; count < Qm_GetNumControllersAllowed(); count++)
    {
        configIndex = Qm_GetActiveCntlMap(count);

        if (configIndex != ACM_NODE_UNDEFINED)
        {
            controllerSN = cntlConfigMap.cntlConfigInfo[configIndex].controllerSN;

            /*
             * If the serial number is zero or it is myself, skip this entry.
             */
            if (controllerSN == 0 || controllerSN == GetMyControllerSN())
            {
                continue;
            }

            pSession = GetSession(controllerSN);

            if (pSession != NULL)
            {
                /*
                 * Bring up ethernet interface. BringUpInterface() checks for
                 * a good connection before actually "bringing a new one up".
                 * If there is a good connection, it simply exits.
                 */
                BringUpInterface(pSession, path);
            }
        }
    }
}

/*----------------------------------------------------------------------------
**  Function Name: EL_StartControlTask
**
**  Modifies:      myControllerSerialNumber
**                 myElectionState
**                 currentElectionState
**                 previousElectionState
**
**  Returns:       GOOD  - Election in progress
**                 ERROR - Failed to start election
**--------------------------------------------------------------------------*/
static UINT32 EL_StartControlTask(UNUSED TASK_PARMS *parms)
{
    UINT32      returnCode = GOOD;
#ifdef ELECTION_INJECT_SLOW_START
    static UINT8 slowStartInjectionCounter = 4;
#endif  /* ELECTION_INJECT_SLOW_START */
    char        tempString[TEMP_STRING_LENGTH];

    dprintf(DPRINTF_ELECTION, "SCT: Starting the election control task\n");

    /* Check if another task is already attempting to start an election */
    if (TestElectionStartingFlag() != TRUE)
    {
        /* Set the starting flag */
        SetElectionStartingFlag(TRUE);

        /*
         * Check if the election control task is already started before forking
         * the control task.
         */
        if (TestElectionInProgressFlag() == FALSE)
        {
            /*
             * Semaphore - wait for the previous election to finish
             * before starting the new election.  Without this, it an
             * election packet comes in at just the right time, the
             * PacketReceptionHandler would start a new election
             * while the EndControlTask was waiting for it to finish.
             */
            while (TestElectionEndingFlag() == TRUE)
            {
                dprintf(DPRINTF_ELECTION, "SCT: Waiting for the previous election to end\n");

                TaskSleepMS(20);
            }

            /*
             * Prevent an inactive controller from calling any elections beyond
             * the one in which it went inactive.  We'll use the standard
             * GetControllerFailureState functions here instead of the
             * myControllerCommArea data, since ReadCommAreaWithRetries hasn't
             * been called yet.
             */
            if ((TestPreviouslyActiveFlag() == TRUE) ||
                ((GetControllerFailureState() != FD_STATE_INACTIVATED) &&
                 (GetControllerFailureState() != FD_STATE_FIRMWARE_UPDATE_INACTIVE) &&
                 (GetControllerFailureState() != FD_STATE_DISASTER_INACTIVE)))
            {
                /*
                 * Initialize the state change semaphore before forking the
                 * election control task.
                 */
                SetStateChangeInProgressFlag(FALSE);

                /* Prepare to log the duration - election starting time */
                EL_DurationLog(EL_DURATION_LOG_START);

                /* Stop the server I/O to give priority to the election's quorum I/O. */
                if (EL_StopIO() == GOOD)
                {
                    /* Force a reload of the master configuration record */
                    if (ReloadMasterConfigWithRetries() == GOOD)
                    {
                        /* Load the private variables to be used for this election. */
                        myControllerSerialNumber = GetMyControllerSN();
                        myControllerCommSlot = GetCommunicationsSlot(myControllerSerialNumber);

                        /*
                         * Try to bring up the ethernet interface to the
                         * other controllers if it is down.
                         */
                        EL_BringUpInterfaceAllControllers(SENDPACKET_ETHERNET);

                        /*
                         * Read the current election state (in the communications slot area)
                         * from the quorum.  If the reads fail for any reason, then don't
                         * start the election process.
                         */
                        if (ReadCommAreaWithRetries(myControllerCommSlot, &myControllerCommArea) == 0)
                        {
                            /*
                             * Before starting the election process, make sure that the
                             * master hasn't failed us.
                             */
                            if ((myControllerCommArea.failStateSector.failureData.state != FD_STATE_UNUSED) &&
                                (myControllerCommArea.failStateSector.failureData.state != FD_STATE_FAILED))
                            {
                                /*
                                 * Check that this controller is in a state where we want it
                                 * to be capable of starting an election.
                                 */
                                if ((myControllerCommArea.failStateSector.failureData.state == FD_STATE_OPERATIONAL) ||
                                    (myControllerCommArea.failStateSector.failureData.state == FD_STATE_POR) ||
                                    (myControllerCommArea.failStateSector.failureData.state == FD_STATE_ADD_CONTROLLER_TO_VCG) ||
                                    (myControllerCommArea.failStateSector.failureData.state == FD_STATE_FIRMWARE_UPDATE_ACTIVE) ||
                                    (myControllerCommArea.failStateSector.failureData.state == FD_STATE_FIRMWARE_UPDATE_INACTIVE) ||
                                    (myControllerCommArea.failStateSector.failureData.state == FD_STATE_UNFAIL_CONTROLLER) ||
                                    (myControllerCommArea.failStateSector.failureData.state == FD_STATE_INACTIVATED) ||
                                    (myControllerCommArea.failStateSector.failureData.state == FD_STATE_ACTIVATE) ||
                                    (myControllerCommArea.failStateSector.failureData.state == FD_STATE_DISASTER_INACTIVE))
                                {
                                    /*
                                     * Modify the mastershipAbility to show that this controller has
                                     * not tested its mastership capability for this election.
                                     */
                                    myControllerCommArea.electionStateSector.electionData.mastershipAbility =
                                        ED_MASTERSHIP_ABILITY_NOT_TESTED;

                                    /*
                                     * Modify the iconConnectivity to show that this controller has
                                     * not tested its connectivity to the icon for this election.
                                     */
                                    myControllerCommArea.electionStateSector.electionData.iconConnectivity =
                                        ED_ICON_CONNECTIVITY_NOT_TESTED;

                                    /*
                                     * Modify the portCount to show that this controller has
                                     * not checked how many FE ports it currently owns.
                                     */
                                    myControllerCommArea.electionStateSector.electionData.portCount = 0;

                                    /*
                                     * Make sure that our commArea in the quorum has our current electionSerial.current.
                                     * NOTE: Bump the electionSerial.current by one to distingsuish the election
                                     * (which should start as a result of this packet) from the previous
                                     * election's serial number.  But leave electionSerial.starting at the
                                     * value that we've read from the quorum.  This will get written at the
                                     * next election state change.
                                     */
                                    myControllerCommArea.electionStateSector.electionData.electionSerial.starting = Qm_GetElectionSerial();

                                    myControllerCommArea.electionStateSector.electionData.electionSerial.current =
                                        Qm_GetElectionSerial() + 1;

                                    dprintf(DPRINTF_ELECTION, "SCT: electionSerial starting: %d, current: %d\n",
                                            myControllerCommArea.electionStateSector.electionData.electionSerial.starting,
                                            myControllerCommArea.electionStateSector.electionData.electionSerial.current);

#ifdef ELECTION_INJECT_SLOW_START
                                    if (slowStartInjectionCounter > 0)
                                    {
                                        if (K_timel & 1)
                                        {
                                            slowStartInjectionCounter--;

                                            dprintf(DPRINTF_ELECTION, "SCT: ***** Slow start injection counter is now %d *****\n", slowStartInjectionCounter);
                                        }
                                    }
                                    else
                                    {
                                        dprintf(DPRINTF_ELECTION, "SCT: ***** Attempting to inject slow start condition *****\n");

                                        slowStartInjectionCounter = 4;
                                        TaskSleepMS(4000);
                                    }
#endif  /* ELECTION_INJECT_SLOW_START */

                                    /*
                                     * Tell the other tasks running on this controller that an
                                     * election is about to begin so they are prepared to have
                                     * the VCG being in flux.
                                     */
                                    if (EL_NotifyOtherTasks(ELECTION_STARTING) == GOOD)
                                    {
                                        /*
                                         * Initialize the entire diskMapList.  This clears
                                         * ONLY the invalid entries, so if this election
                                         * was started by a call from PacketRecptionHandler,
                                         * we retain the diskList from that incoming packet.
                                         */
                                        EL_DiskMapResetList();

                                        /* Reinitialize (clear) this controller's contact map */
                                        EL_ClearContactMap();

                                        /* Initialize the election control task variables */
                                        SetElectionRestartFlag(FALSE);
                                        EL_ChangeState(ED_STATE_BEGIN_ELECTION);
                                        contactedByMaster = FALSE;

                                        /* Fork the election control task */
                                        TaskCreate(EL_ControlTask, NULL);

                                        /*
                                         * Wait for the control task to set the IN_PROGRESS flag
                                         * to TRUE.  This flag indicates when the control task
                                         * is forked and running.
                                         */
                                        while (TestElectionInProgressFlag() == FALSE)
                                        {
                                            TaskSleepMS(20);
                                        }

                                        dprintf(DPRINTF_ELECTION, "SCT: Election task forked and running\n");
                                    }
                                    else
                                    {
                                        dprintf(DPRINTF_ELECTION, "SCT: Error notifying other tasks about election starting\n");

                                        returnCode = ERROR;
                                    }
                                }
                                else
                                {
                                    EL_GetFailureDataStateString(tempString,
                                                                 myControllerCommArea.failStateSector.failureData.state,
                                                                 sizeof(tempString));

                                    dprintf(DPRINTF_ELECTION, "SCT: This controller's failureData.state indicates controller can't start an election\n");
                                    dprintf(DPRINTF_ELECTION, "SCT:   failureData.state is: %s\n", tempString);

                                    returnCode = ERROR;
                                }
                            }
                            else
                            {
                                EL_GetFailureDataStateString(tempString,
                                                             myControllerCommArea.failStateSector.failureData.state,
                                                             sizeof(tempString));

                                dprintf(DPRINTF_ELECTION, "SCT: This controller's failureData.state indicates controller should be offline\n");
                                dprintf(DPRINTF_ELECTION, "SCT:   failureData.state is: %s\n", tempString);

                                returnCode = ERROR;
                            }
                        }
                        else
                        {
                            dprintf(DPRINTF_ELECTION, "SCT: Error reading myControllerCommArea\n");

                            returnCode = ERROR;
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "SCT: Error reading masterConfig\n");

                        returnCode = ERROR;
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "SCT: EL_StopIO return is not GOOD\n");

                    returnCode = ERROR;
                }
            }
            else
            {
                /*
                 * If an inactive controller does try to call additional
                 * elections, return GOOD status but do not actually start the
                 * election.  The controller is inactive and we'd like to keep
                 * it running (don't suicide if at all possible).
                 */
                dprintf(DPRINTF_ELECTION, "SCT: Preventing inactive controller election\n");
                dprintf(DPRINTF_ELECTION, "SCT:   PreviouslyActiveFlag:   %d\n", TestPreviouslyActiveFlag());

                EL_GetFailureDataStateString(tempString, GetControllerFailureState(), sizeof(tempString));

                dprintf(DPRINTF_ELECTION, "SCT:   ControllerFailureState: %s (%d)\n", tempString, GetControllerFailureState());
            }
        }
        else
        {
            /*
             * Election already in progress.  This could happen if the
             * controller received an election packet at nearly the same
             * time that the code asked for an election.
             */
            dprintf(DPRINTF_ELECTION, "SCT: Election task is already in progress\n");
        }

        /* Clear the starting flag */
        SetElectionStartingFlag(FALSE);
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "SCT: Another task is already starting an election (wait for start to complete)\n");

        /* Don't return until the other task has finished starting the election. */
        while (TestElectionStartingFlag() == TRUE)
        {
            TaskSleepMS(20);
        }

        /*
         * At the point where the election is no longer
         * starting, if the ElectionInProgress flag isn't TRUE, then
         * the other task got an error when it tried to start the
         * election.  In this case, to be consistent, we need to also
         * return an error to our task.
         */
        if (TestElectionInProgressFlag() == TRUE)
        {
            dprintf(DPRINTF_ELECTION, "SCT: Another task started an election\n");
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "SCT: Another task had problems starting an election\n");

            returnCode = ERROR;
        }
    }

    /*
     * If the control task was not started successfully, report
     * the error through the EL_ChangeState as a failure
     * and return the error to the caller.
     */
    if (returnCode != GOOD)
    {
        dprintf(DPRINTF_ELECTION, "SCT: Election control task could not be started - failing controller\n");

        EL_ChangeState(ED_STATE_FAILED);
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_RestartElection
**
**  Description: This function can only be called if the election task
**               is already running.  This does NOT spawn a new task.
**
**  Inputs:     masterConfig - make sure this is up-to-date
**
**  Modifies:
**
**  Returns:    GOOD
**              ERROR
**--------------------------------------------------------------------------*/
static UINT32 EL_RestartElection(void)
{
    UINT32      returnCode = ERROR;

    char        tempString[TEMP_STRING_LENGTH];

    /*
     * Set flag so control task doesn't close the election task
     */
    SetElectionRestartFlag(TRUE);

    /*
     * Load the private variables to be used for this election.
     */
    myControllerSerialNumber = GetMyControllerSN();
    myControllerCommSlot = GetCommunicationsSlot(myControllerSerialNumber);

    /*
     * Read the current election state (in the communications slot area)
     * from the quorum.  If the reads fail for any reason, then don't
     * start the election process.
     */
    if (ReadCommAreaWithRetries(myControllerCommSlot, &myControllerCommArea) == 0)
    {
        /*
         * Before starting the election process, make sure that the
         * master hasn't failed us.
         */
        if ((myControllerCommArea.failStateSector.failureData.state != FD_STATE_UNUSED) &&
            (myControllerCommArea.failStateSector.failureData.state != FD_STATE_FAILED))
        {
            /*
             * Check that this controller is in a state where we want it
             * to be capable of starting an election.
             */
            if ((myControllerCommArea.failStateSector.failureData.state == FD_STATE_OPERATIONAL) ||
                (myControllerCommArea.failStateSector.failureData.state == FD_STATE_POR) ||
                (myControllerCommArea.failStateSector.failureData.state == FD_STATE_ADD_CONTROLLER_TO_VCG) ||
                (myControllerCommArea.failStateSector.failureData.state == FD_STATE_FIRMWARE_UPDATE_ACTIVE) ||
                (myControllerCommArea.failStateSector.failureData.state == FD_STATE_FIRMWARE_UPDATE_INACTIVE) ||
                (myControllerCommArea.failStateSector.failureData.state == FD_STATE_UNFAIL_CONTROLLER) ||
                (myControllerCommArea.failStateSector.failureData.state == FD_STATE_INACTIVATED) ||
                (myControllerCommArea.failStateSector.failureData.state == FD_STATE_ACTIVATE) ||
                (myControllerCommArea.failStateSector.failureData.state == FD_STATE_DISASTER_INACTIVE))
            {
                /*
                 * Modify the mastershipAbility to show that this controller has
                 * not tested its mastership capability for this election.
                 */
                myControllerCommArea.electionStateSector.electionData.mastershipAbility = ED_MASTERSHIP_ABILITY_NOT_TESTED;

                /*
                 * Modify the iconConnectivity to show that this controller has
                 * not tested its connectivity to the icon for this election.
                 */
                myControllerCommArea.electionStateSector.electionData.iconConnectivity = ED_ICON_CONNECTIVITY_NOT_TESTED;

                /*
                 * Modify the portCount to show that this controller has
                 * not checked how many FE ports it currently owns.
                 */
                myControllerCommArea.electionStateSector.electionData.portCount = 0;

                /*
                 * Make sure that our commArea in the quorum has our current electionSerial.current.
                 * NOTE: Bump the electionSerial.current by one to distingsuish the election
                 * (which should start as a result of this packet) from the previous
                 * election's serial number.  But leave electionSerial.starting at the
                 * value that we've read from the quorum.
                 */
                myControllerCommArea.electionStateSector.electionData.electionSerial.starting = Qm_GetElectionSerial();

                myControllerCommArea.electionStateSector.electionData.electionSerial.current = Qm_GetElectionSerial() + 1;

                dprintf(DPRINTF_ELECTION, "RE: electionSerial starting: %d, current: %d\n",
                        myControllerCommArea.electionStateSector.electionData.electionSerial.starting,
                        myControllerCommArea.electionStateSector.electionData.electionSerial.current);

                /*
                 * Write new mastershipAbility to quorum
                 */
                if (WriteElectionDataWithRetries(myControllerSerialNumber, &myControllerCommArea.electionStateSector.electionData) == 0)
                {
                    /*
                     * Update the file system 'good disk' map
                     */
                    EL_DiskMapUpdateFIOMap();

                    /*
                     * Tell the other tasks running on this controller that an
                     * election is about to begin so they are prepared to have
                     * the VCG being in flux.
                     */
                    if (EL_NotifyOtherTasks(ELECTION_STARTING) == GOOD)
                    {
                        /*
                         * Initialize the election control task variables
                         */
                        EL_ChangeState(ED_STATE_BEGIN_ELECTION);
                        returnCode = GOOD;

                        /*
                         * Log the duration - election restart time
                         */
                        EL_DurationLog(EL_DURATION_LOG_RESTART);
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "RE: Error notifying other tasks about restarting election\n");
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "RE: Controller write of new mastershipAbility FAILED\n");
                }

                /*
                 * Discard the old diskMap data
                 */
                EL_DiskMapInvalidateList();
                EL_DiskMapResetList();
            }
            else
            {
                EL_GetFailureDataStateString(tempString,
                                             myControllerCommArea.failStateSector.failureData.state,
                                             sizeof(tempString));

                dprintf(DPRINTF_ELECTION, "RE: This controller's failureData.state indicates controller can't start an election\n");
                dprintf(DPRINTF_ELECTION, "RE:   failureData.state is: %s\n", tempString);
            }
        }
        else
        {
            EL_GetFailureDataStateString(tempString,
                                         myControllerCommArea.failStateSector.failureData.state,
                                         sizeof(tempString));

            dprintf(DPRINTF_ELECTION, "RE: This controller's failureData.state indicates controller should be offline\n");
            dprintf(DPRINTF_ELECTION, "RE:   failureData.state is: %s\n", tempString);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "RE: Error reading myControllerCommArea\n");
    }

    /*
     * Make sure that the controller goes to the FAILED state if we're
     * unable to restart the election.
     */
    if (returnCode != GOOD)
    {
        EL_NotifyOtherTasks(ELECTION_FAILED);
    }

    /*
     * Clear flag so control task is able to close the task when it wants
     */
    SetElectionRestartFlag(FALSE);

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_ChangeState
**
**  Inputs:         Requires that myControllerCommArea be up-to-date
**
**  Modifies:       myControllerCommArea
**--------------------------------------------------------------------------*/
void EL_ChangeState(ELECTION_DATA_STATE newElectionState)
{
    char        tempString1[TEMP_STRING_LENGTH];
    char        tempString2[TEMP_STRING_LENGTH];
    UINT8       stateChangeWait = FALSE;

    /* Check EL_StateChange semaphore before proceeding */
    while ((TestElectionInProgressFlag() == TRUE) && (TestStateChangeInProgressFlag() == TRUE))
    {
        if (stateChangeWait == FALSE)
        {
            dprintf(DPRINTF_ELECTION, "CS: Waiting for another state change to complete\n");
            stateChangeWait = TRUE;
        }
        TaskSleepMS(20);
    }

    /*
     * Turn on EL_StateChange semaphore.  This gets cleared by the
     * control task, once it's able to process the state change.
     */
    SetStateChangeInProgressFlag(TRUE);

    EL_GetElectionStateString(tempString1, EL_GetCurrentState(), sizeof(tempString1));

    EL_GetElectionStateString(tempString2, newElectionState, sizeof(tempString2));

    dprintf(DPRINTF_ELECTION, "CS: Attempting to change states from %s to %s\n", tempString1, tempString2);

    /*
     * Only allow state changes if the election task is running.  If the election
     * task is not running, we'll only allow a transition into the BEGIN_ELECTION
     * state, since that's how the election task gets running.
     */
    if ((TestElectionInProgressFlag() == TRUE) || (newElectionState == ED_STATE_BEGIN_ELECTION))
    {
        if ((EL_GetCurrentState() == ED_STATE_BEGIN_ELECTION) &&
            (newElectionState != ED_STATE_CHECK_MASTERSHIP_ABILITY) &&
            (newElectionState != ED_STATE_FAILED))
        {
            /*
             * Make sure that any transitions from the ED_STATE_BEGIN_ELECTION
             * state are going to either the ED_STATE_CHECK_MASTERSHIP_ABILITY state
             * or the FAILED state.
             */
            EL_GetElectionStateString(tempString1, newElectionState, sizeof(tempString1));

            dprintf(DPRINTF_ELECTION, "CS: Preventing transition from BEGIN_ELECTION state to %s\n", tempString1);
        }
        else if ((newElectionState == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE) &&
                 (EL_GetCurrentState() != ED_STATE_TIMEOUT_CONTROLLERS))
        {
            /*
             * Make sure that any transitions to the ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE
             * state are coming from the ED_STATE_TIMEOUT_CONTROLLERS state.
             * We can change into the ED_STATE_TIMEOUT_CONTROLLERS state outside
             * the normal election tasks (through packetReceptionHandler) so we
             * need to intercept invalid transitions here.
             */
            EL_GetElectionStateString(tempString1, EL_GetCurrentState(), sizeof(tempString1));

            dprintf(DPRINTF_ELECTION, "CS: Preventing transition to TIMEOUT_CONTROLLERS_COMPLETE state from %s\n", tempString1);
        }
        else if ((EL_GetCurrentState() == ED_STATE_TIMEOUT_CONTROLLERS) &&
                 (newElectionState != ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE) &&
                 (newElectionState != ED_STATE_WAIT_FOR_MASTER) &&
                 (newElectionState != ED_STATE_FAILED))
        {
            /*
             * Make sure that any transitions from the ED_STATE_TIMEOUT_CONTROLLERS
             * state are going to either the ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE state,
             * the WAIT_FOR_MASTER state, or the FAILED state.  We can change into
             * the ED_STATE_TIMEOUT_CONTROLLERS state outside the normal election
             * tasks (through packetReceptionHandler) so we need to intercept invalid
             * transitions here.
             */
            EL_GetElectionStateString(tempString1, newElectionState, sizeof(tempString1));

            dprintf(DPRINTF_ELECTION, "CS: Preventing transition from TIMEOUT_CONTROLLERS state to %s\n", tempString1);
        }
        else if ((EL_GetCurrentState() == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE) &&
                 (newElectionState != ED_STATE_CONTACT_ALL_CONTROLLERS) &&
                 (newElectionState != ED_STATE_WAIT_FOR_MASTER) &&
                 (newElectionState != ED_STATE_FAILED))
        {
            /*
             * Make sure that any transitions from the ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE
             * state are going to either the ED_STATE_CONTACT_ALL_CONTROLLERS state,
             * the WAIT_FOR_MASTER state, or the FAILED state.  We can change into
             * the ED_STATE_TIMEOUT_CONTROLLERS state outside the normal election
             * tasks (through packetReceptionHandler) so we need to intercept invalid
             * transitions here.
             */
            EL_GetElectionStateString(tempString1, newElectionState, sizeof(tempString1));

            dprintf(DPRINTF_ELECTION, "CS: Preventing transition from TIMEOUT_CONTROLLERS_COMPLETE state to %s\n", tempString1);
        }
        else if ((EL_GetCurrentState() == ED_STATE_FAILED) && (newElectionState != ED_STATE_FINISHED))
        {
            /*
             * Make sure that any transitions from the FAILED state are going
             * to the FINISHED state.  The FAILED-to-FINSHED transition exists
             * only in the EL_FailedState function, so we know it has
             * handled the failure.
             */
            EL_GetElectionStateString(tempString1, newElectionState, sizeof(tempString1));

            dprintf(DPRINTF_ELECTION, "CS: Preventing transition from FAILED state to %s\n", tempString1);
        }
        else if ((EL_GetCurrentState() == ED_STATE_FINISHED) &&
                 (((EL_GetPreviousState() == ED_STATE_FAILED) &&
                   (newElectionState != ED_STATE_END_TASK)) ||
                  ((EL_GetPreviousState() != ED_STATE_FAILED) &&
                   (newElectionState != ED_STATE_FAILED) &&
                   (newElectionState != ED_STATE_END_TASK))))
        {
            /*
             * Make sure that any transitions from the FINISHED state are going
             * to the FAILED or END_TASK state.  The FINISHED-to-END_TASK
             * transition exists only in the EL_FinishedState function,
             * so we know it has handled the ending of the election.  The
             * FINISHED-to-FAILED transition can happen, but this must allow
             * the transition to occur only once.
             */
            EL_GetElectionStateString(tempString1, newElectionState, sizeof(tempString1));

            dprintf(DPRINTF_ELECTION, "CS: Preventing transition from FINISHED state to %s\n", tempString1);
        }
        else
        {
            /*
             * Check that we're actually changing states.  If we're not, then
             * don't change... doing so would cause us to lose the previousState.
             */
            if (newElectionState != EL_GetCurrentState())
            {
                /*
                 * Indicate that we need to proceed to the next state
                 */
                EL_SetNextState(newElectionState);
                EL_ControlTaskCommitStateChange();
            }
        }
    }
    else
    {
        EL_GetElectionStateString(tempString1, newElectionState, sizeof(tempString1));

        dprintf(DPRINTF_ELECTION, "CS: Election task is NOT RUNNING, but state change to %s is requested\n", tempString1);

        /*
         * This condition could happen if the election hasn't started on
         * controller, it receives an election packet, and it fails when trying
         * to read the masterConfig data.  We need to tell the other tasks that
         * we should fail, but don't actually change the election state.
         */
        if (newElectionState == ED_STATE_FAILED)
        {
            dprintf(DPRINTF_ELECTION, "CS: Notifying other tasks of FAIL request\n");

            EL_NotifyOtherTasks(ELECTION_FAILED);
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "CS: Throwing away state change, since election task isn't running\n");
        }
    }

    /* Clear the state change semaphore */
    SetStateChangeInProgressFlag(FALSE);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_DoElection
**
**  Returns:       GOOD  - Election has been run
**                 ERROR - Failed to start election
**--------------------------------------------------------------------------*/
UINT32 EL_DoElection(void)
{
    dprintf(DPRINTF_ELECTION, "DE: Code requested an election (blocking)\n");

    /* Initiate the election process and do not return until complete */
    if (EL_StartControlTask(NULL) != GOOD)
    {
        return ERROR;
    }

    /* Wait for the election control task to indicate completion */
    while (TestElectionInProgressFlag() == TRUE)
    {
        TaskSleepMS(20);
    }
    return GOOD;
}


/*----------------------------------------------------------------------------
**  Function Name: EL_DoElectionNonBlocking
**
**  Returns:       GOOD  - Election started (or already in progress)
**                 ERROR - Failed to start election
**--------------------------------------------------------------------------*/
UINT32 EL_DoElectionNonBlocking(void)
{
    dprintf(DPRINTF_ELECTION, "DE: Code requested an election (non-blocking)\n");

    return (EL_StartControlTask(NULL));
}


/*----------------------------------------------------------------------------
**  Function Name: EL_PacketReceptionHandler
**
**  Returns:       Nothing
**--------------------------------------------------------------------------*/
IPC_PACKET *EL_PacketReceptionHandler(IPC_PACKET *packetPtr)
{
    QM_MASTER_CONFIG *tempMasterConfig = NULL;
    ELECTION_SERIAL electionSerial = { 0, 0 };
    UINT16      thisControllerACMNode = ACM_NODE_UNDEFINED;
    UINT16      thisControllerCommSlot = ACM_NODE_UNDEFINED;
    UINT16      otherControllerCommSlot = ACM_NODE_UNDEFINED;
    UINT8       stateChangeWait = FALSE;
    UINT8       diskMapPacket = FALSE;
    UINT8       processDiskMapPacket = FALSE;
    char        tempString[TEMP_STRING_LENGTH];
#ifdef ELECTION_INJECT_SLOW_RESPONSE
    static UINT8 slowResponseInjectionCounter = 4;
#endif  /* ELECTION_INJECT_SLOW_RESPONSE */

    if ((packetPtr != NULL) && (packetPtr->header != NULL) && (packetPtr->data != NULL))
    {
#ifdef ELECTION_INJECT_SLOW_RESPONSE
        if (slowResponseInjectionCounter > 0)
        {
            if (K_timel & 1)
            {
                slowResponseInjectionCounter--;

                dprintf(DPRINTF_ELECTION, "PRH: ***** Slow response injection counter is now %d *****\n", slowResponseInjectionCounter);
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "PRH: ***** Attempting to inject slow response condition *****\n");

            slowResponseInjectionCounter = 4;
            TaskSleepMS(4000);
        }
#endif  /* ELECTION_INJECT_SLOW_RESPONSE */

        /*
         * Load up a copy of this controller's commSlot.  Since packetReception
         * might be occurring outside of the election task, the global
         * 'myControllerCommSlot' might not be valid.  Also, avoid the possibility
         * of modifying the myControllerCommSlot while the election task is running.
         */
        thisControllerCommSlot = GetCommunicationsSlot(GetMyControllerSN());
        otherControllerCommSlot = GetCommunicationsSlot(packetPtr->header->ccbSerialNumber);

        dprintf(DPRINTF_ELECTION, "PRH: Packet received from controller in slot %d\n", otherControllerCommSlot);

        EL_GetElectionStateString(tempString,
                                  packetPtr->data->elect.ipcElectR1.electionTaskState,
                                  sizeof(tempString));

        dprintf(DPRINTF_ELECTION, "PRH:   Sender   - Serial: %d/%d  State: %s\n",
                packetPtr->data->elect.ipcElectR1.electionSerial.starting,
                packetPtr->data->elect.ipcElectR1.electionSerial.current, tempString);

        /* Check the length of the packet to see if it contains the diskMap data */
        if (packetPtr->header->length == sizeof(IPC_ELECT_R3))
        {
            dprintf(DPRINTF_ELECTION, "PRH:   Packet has diskMap: %d bytes\n", packetPtr->header->length);

            diskMapPacket = TRUE;
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "PRH:   Packet does not have diskMap: %d bytes\n", packetPtr->header->length);
        }

        /*
         * Process only packets that are not from this controller.  I've run
         * into a couple instances where IPC receives a packet on the controller
         * that sent the packet.  Don't know why, but protect from it here.
         */
        if (thisControllerCommSlot != otherControllerCommSlot)
        {
            /* Check PacketReceptionHandler semaphore before proceeding */
            while (TestPacketReceptionInProgress() == TRUE)
            {
                if (stateChangeWait == FALSE)
                {
                    dprintf(DPRINTF_ELECTION, "PRH: Waiting for another packet reception to complete\n");
                    stateChangeWait = TRUE;
                }

                TaskSleepMS(20);
            }

            /* Turn on PacketReceptionHandler semaphore */
            SetPacketReceptionInProgress(TRUE);

            /* Wait for any election state changes to be committed before going on. */
            EL_WaitForStateChangeComplete();

            /*
             * If the election control task is already running, look at the
             * myControllerCommArea.electionStateSector.electionData.electionSerial.current
             * for the electionSerial.current value.  If it's not already running, read up
             * the masterConfig and use that copy's electionSerial value.
             */
            if ((TestElectionInProgressFlag() == TRUE) && (EL_GetCurrentElectionState() != ED_STATE_END_TASK))
            {
                /* Tell the code to process the diskMap, if one exists */
                processDiskMapPacket = TRUE;

                /* Use the election code's serial number data (not masterConfig) */
                dprintf(DPRINTF_ELECTION, "PRH: Using election masterConfig\n");

                electionSerial.starting = myControllerCommArea.electionStateSector.electionData.electionSerial.starting;

                electionSerial.current = myControllerCommArea.electionStateSector.electionData.electionSerial.current;
            }
            else
            {
                /*
                 * Check if we can use the DRAM copy of the masterConfig, or
                 * if we need to fetch a fresh version from the disks.  We want
                 * to avoid a file system read if we can.  Since we may not have
                 * called the StopIO yet, the read might take a very long time.
                 */
                if ((PowerUpComplete() == TRUE) &&
                    (ACM_GetNodeBySN(Qm_ActiveCntlMapPtr(), &thisControllerACMNode, myControllerSerialNumber) == GOOD) &&
                    (myControllerCommArea.failStateSector.failureData.state == FD_STATE_OPERATIONAL))
                {
                    dprintf(DPRINTF_ELECTION, "PRH: Using resident masterConfig - ACM node %d\n", thisControllerACMNode);

                    /*
                     * Bump electionSerial.current by one to distingsuish the election
                     * (which should start as a result of this packet) from the
                     * previous election's serial number.  But leave electionSerial.starting
                     * at the value that we currently have in memory.
                     */
                    electionSerial.starting = Qm_GetElectionSerial();
                    electionSerial.current = Qm_GetElectionSerial() + 1;
                }
                else
                {
                    /*
                     * The file system read is unavoidable.  Read a temporary
                     * version of the master configuration record.
                     */
                    tempMasterConfig = MallocSharedWC(sizeof(*tempMasterConfig));
                    if (ReadMasterConfiguration(tempMasterConfig) == 0)
                    {
                        dprintf(DPRINTF_ELECTION, "PRH: Loaded electionSerial.current from tempMasterConfig\n");

                        /*
                         * Bump electionSerial.current by one to distingsuish the election
                         * (which should start as a result of this packet) from the
                         * previous election's serial number.  But leave electionSerial.starting
                         * at the value that was read from the quorum.
                         */
                        electionSerial.starting = tempMasterConfig->electionSerial;
                        electionSerial.current = tempMasterConfig->electionSerial + 1;
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "PRH: Unable to load electionSerial.current from tempMasterConfig\n");

                        /*
                         * In case we're not acting upon the failure state, put this
                         * controller's masterConfig.electionSerial into the response
                         * packet structure.
                         */
                        electionSerial.starting = Qm_GetElectionSerial();
                        electionSerial.current = Qm_GetElectionSerial();

                        /* This controller can't read from the quorum, so fail it. */
                        EL_ChangeState(ED_STATE_FAILED);
                    }
                    Free(tempMasterConfig);
                }
            }

            EL_GetElectionStateString(tempString, EL_GetCurrentElectionState(), sizeof(tempString));

            dprintf(DPRINTF_ELECTION, "PRH:   Receiver - Serial: %d/%d  State: %s\n",
                    electionSerial.starting, electionSerial.current, tempString);

            /* Process the election packet */
            switch (packetPtr->data->elect.ipcElectR1.messageType)
            {
                case IPC_ELECT_CONTACT_CONTROLLER:
                    /*
                     * This controller has received a 'CONTACT_CONTOLLER' message, which
                     * means that another controller is trying to determine which other
                     * controllers are still active in the VCG.  Start the election task
                     * if it's not already running, and work with the other controllers
                     * in the VCG to determine the new master.
                     */
                    dprintf(DPRINTF_ELECTION, "PRH: Message type is IPC_ELECT_CONTACT_CONTROLLER\n");

                    if (TestElectionInProgressFlag() == TRUE)
                    {
                        dprintf(DPRINTF_ELECTION, "PRH: Election control task running\n");

                        /* React to the incoming election packet */
                        if (electionSerial.current == packetPtr->data->elect.ipcElectR1.electionSerial.current)
                        {
                            if (EL_GetCurrentElectionState() == ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE)
                            {
                                dprintf(DPRINTF_ELECTION, "PRH: Extending CONTACT_ALL_CONTROLLERS_COMPLETE timeout value\n");

                                /* Extend the CONTACT_ALL_CONTROLLERS_COMPLETE timeout counter */
                                SetCACCSTimeoutCounter(CHECK_FOR_TIMEOUT_GRANULARITY);
                            }
                        }
                        else if (EL_IsElectionSerialNewer(&electionSerial, &packetPtr->data->elect.ipcElectR1.electionSerial) > 0)
                        {
                            dprintf(DPRINTF_ELECTION, "PRH: This controller has a newer election serial number\n");
                        }
                        else if ((EL_GetCurrentElectionState() == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE) &&
                                 (EL_IsElectionSerialOlder(&electionSerial, &packetPtr->data->elect.ipcElectR1.electionSerial) == 1))
                        {
                            /*
                             * Another controller has timed out and has already gone on to the
                             * next CONTACT_ALL_CONTROLLERS phase, which has an electionSerial.current
                             * that's one higher than what we currently have.  This is an
                             * acceptable condition, so don't fail the election.
                             */
                            dprintf(DPRINTF_ELECTION, "PRH: Incoming packet's electionSerial.current is one newer\n");
                            dprintf(DPRINTF_ELECTION, "PRH:   Another controller has continued from a timeout\n");

                            /*
                             * Artificially inflate this controller's electionSerial.current so that
                             * the controller we're responding to accepts the return packet
                             * as a valid contact.
                             */
                            electionSerial.current++;

                            /*
                             * Tell the TimeoutControllersComplete function to stop waiting
                             * and to move on with the election.
                             */
                            SetContinueFromTimeoutCompleteFlag(TRUE);

                            /*
                             * Invalidate this controller's diskMapList so that it's
                             * cleaned up after the timeout condition.
                             */
                            EL_DiskMapInvalidateList();
                            EL_DiskMapResetList();
                        }
                        else if (EL_GetCurrentElectionState() == ED_STATE_WAIT_FOR_MASTER)
                        {
                            if (EL_IsElectionSerialOlder(&electionSerial, &packetPtr->data->elect.ipcElectR1.electionSerial) == 3)
                            {
                                dprintf(DPRINTF_ELECTION, "PRH: Incoming packet's electionSerial.current is three newer\n");
                                dprintf(DPRINTF_ELECTION, "PRH:   Another controller has called for an election while\n");
                                dprintf(DPRINTF_ELECTION, "PRH:   this controller is still in the WAIT_FOR_MASTER state\n");

                                /*
                                 * There's a small window where the master is finished with
                                 * its election before the slave has completed its election
                                 * and is in the ED_STATE_WAIT_FOR_MASTER state.
                                 *
                                 * We need to cover the case so that we respond correctly
                                 * when the master controller starts another election before
                                 * this slave has completed the previous election.
                                 *
                                 * We need to reload the masterConfig that's stored in the
                                 * quorum, so that we start the election with the same ACM
                                 * as the master controller we were waiting for.
                                 */
                                if (LoadMasterConfiguration() == 0)
                                {
                                    /*
                                     * Adjust this controller's electionSerial.current so that the
                                     * controller we're responding to accepts the return
                                     * packet as a valid contact.  This electionSerial.current
                                     * should be identical to what it will be once we reach
                                     * the BEGIN_ELECTION state.
                                     */
                                    electionSerial.current = Qm_GetElectionSerial() + 1;

                                    /*
                                     * Restart the election.  This kicks the state machine
                                     * back to BEGIN_ELECTION, as well as reinitializes a
                                     * bunch of variables that we use during the election.
                                     */
                                    EL_RestartElection();
                                }
                                else
                                {
                                    /* The controller couldn't read the quorum, so fail. */
                                    dprintf(DPRINTF_ELECTION, "PRH: Unable to LoadMasterConfiguration\n");

                                    EL_ChangeState(ED_STATE_FAILED);
                                }
                            }
                        }
                        else if (((EL_GetCurrentElectionState() == ED_STATE_FINISHED) &&
                                  (myControllerCommArea.failStateSector.failureData.state != FD_STATE_FAILED)) &&
                                 (EL_IsElectionSerialOlder(&electionSerial, &packetPtr->data->elect.ipcElectR1.electionSerial) == 3))
                        {
                            dprintf(DPRINTF_ELECTION, "PRH: Incoming packet's electionSerial.current is three newer\n");
                            dprintf(DPRINTF_ELECTION, "PRH:   Another controller has called for an election while\n");
                            dprintf(DPRINTF_ELECTION, "PRH:   this controller is still in the FINISHED state\n");

                            /*
                             * There's a small window where the master is finished with
                             * its election before the slave has completed its election
                             * and is in the ED_STATE_FINISHED state.
                             *
                             * We need to cover the case so that we respond correctly
                             * when the master controller starts another election before
                             * this slave has completed the previous election.
                             *
                             * We we have already loaded the current masterConfig that's
                             * stored in the quorum when we were in the WAIT_FOR_MASTER
                             * state, so our election serial should be one older than
                             * what's on the incoming packet.
                             */

                            /*
                             * Adjust this controller's electionSerial.current so that the
                             * controller we're responding to accepts the return
                             * packet as a valid contact.  This electionSerial.current
                             * should be identical to what it will be once we reach
                             * the BEGIN_ELECTION state.
                             */
                            electionSerial.current = Qm_GetElectionSerial() + 1;

                            /*
                             * Restart the election.  This kicks the state machine
                             * back to BEGIN_ELECTION, as well as reinitializes a
                             * bunch of variables that we use during the election.
                             */
                            EL_RestartElection();
                        }
                        else if (((EL_GetCurrentElectionState() == ED_STATE_FINISHED) &&
                                  (myControllerCommArea.failStateSector.failureData.state != FD_STATE_FAILED)) &&
                                 (Qm_GetMasterControllerSN() == GetMyControllerSN()) &&
                                 (electionSerial.current == packetPtr->data->elect.ipcElectR1.electionSerial.starting))
                        {
                            dprintf(DPRINTF_ELECTION, "PRH: electionSerial.current == incoming electionSerial.starting\n");
                            dprintf(DPRINTF_ELECTION, "PRH:   A slave has called for an election before\n");
                            dprintf(DPRINTF_ELECTION, "PRH:   this controller got through the FINISHED state\n");

                            /*
                             * There's a small window where a slave can time out waiting
                             * for the master to complete the FINISHED state. When this
                             * occurs, the master needs to catch up to the other slaves
                             * so it doesn't get failed.
                             *
                             * We need to cover the case so that we respond correctly
                             * when the slave controller starts another election before
                             * the master has completed the FINISHED state on the prior
                             * election.
                             *
                             * We we have already loaded the current masterConfig that's
                             * stored in the quorum when we took mastership, so there's
                             * no need to do it again.
                             */

                            /*
                             * Adjust this controller's electionSerial.current so that the
                             * controller we're responding to accepts the return
                             * packet as a valid contact.  This electionSerial.current
                             * should be identical to what it will be once we reach
                             * the BEGIN_ELECTION state.
                             */
                            electionSerial.current = Qm_GetElectionSerial() + 1;

                            /*
                             * Restart the election.  This kicks the state machine
                             * back to BEGIN_ELECTION, as well as reinitializes a
                             * bunch of variables that we use during the election.
                             */
                            EL_RestartElection();
                        }
                        else if (((EL_GetCurrentElectionState() == ED_STATE_END_TASK) &&
                                  (myControllerCommArea.failStateSector.failureData.state != FD_STATE_FAILED)) &&
                                 (electionSerial.current == packetPtr->data->elect.ipcElectR1.electionSerial.starting))
                        {
                            dprintf(DPRINTF_ELECTION, "PRH: electionSerial.current == incoming electionSerial.starting\n");
                            dprintf(DPRINTF_ELECTION, "PRH:   Another controller has called for an election before\n");
                            dprintf(DPRINTF_ELECTION, "PRH:   this controller's election task ended\n");

                            /*
                             * There's a small window where a controller is in the END_TASK
                             * state, but the task hasn't closed.  When this occurs, the
                             * controller needs to catch up to the others so it doesn't
                             * get failed.
                             *
                             * We we have already loaded the current masterConfig that's
                             * stored in the quorum before we went to the END_TASK state,
                             * so there's no need to do it again.
                             */

                            /*
                             * Adjust this controller's electionSerial.current so that the
                             * controller we're responding to accepts the return
                             * packet as a valid contact.  This electionSerial.current
                             * should be identical to what it will be once we reach
                             * the BEGIN_ELECTION state.
                             */
                            electionSerial.current = Qm_GetElectionSerial() + 1;

                            /*
                             * Restart the election.  This kicks the state machine
                             * back to BEGIN_ELECTION, as well as reinitializes a
                             * bunch of variables that we use during the election.
                             */
                            EL_RestartElection();
                        }
                        else if ((EL_GetCurrentElectionState() == ED_STATE_TIMEOUT_CONTROLLERS) &&
                                 (EL_IsElectionSerialOlder(&electionSerial, &packetPtr->data->elect.ipcElectR1.electionSerial) == 1))
                        {
                            /*
                             * Another controller has timed out and has already gone on to the
                             * next CONTACT_ALL_CONTROLLERS phase, which has an electionSerial.current
                             * that's one higher than what we currently have.  This controller is
                             * not yet finished with the TIMEOUT_CONTROLLERS state, which means
                             * that it's lagging behind the other controller.  We need fail this
                             * controller here, so that a marginal (slow) controller gets removed
                             * ASAP, so it doesn't take the DSC down.
                             */
                            dprintf(DPRINTF_ELECTION, "PRH: This controller is too slow - fail\n");

                            EL_ChangeState(ED_STATE_FAILED);
                        }
                        else
                        {
                            /* The controller missed an election, so fail. */
                            dprintf(DPRINTF_ELECTION, "PRH: Election serial number out of sequence\n");

                            EL_ChangeState(ED_STATE_FAILED);
                        }
                    }
                    else
                    {
                        /*
                         * If the election task is not already running, check to
                         * see if we should spawn the election startup task or fail.
                         * This controller's election serial should not be any
                         * older than the incoming packet's election serial.
                         */
                        if (EL_IsElectionSerialOlder(&electionSerial, &packetPtr->data->elect.ipcElectR1.electionSerial) == 0)
                        {
                            dprintf(DPRINTF_ELECTION, "PRH: Election serial numbers pass inspection\n");

                            /* Tell the code to process the diskMap, if one exists */
                            processDiskMapPacket = TRUE;

                            /* Start the election task */
                            dprintf(DPRINTF_ELECTION, "PRH: Forking EL_StartControlTask\n");
                            TaskCreate((void *)EL_StartControlTask, NULL);
                        }
                        else
                        {
                            /* The controller missed an election, so fail. */
                            dprintf(DPRINTF_ELECTION, "PRH: Election serial number out of sequence\n");

                            EL_ChangeState(ED_STATE_FAILED);
                        }
                    }
                    break;

                case IPC_ELECT_NOTIFY_SLAVE:
                    /*
                     * This controller has received a 'NOTIFY_SLAVE' message, which means
                     * that another controller thinks this controller is a slave.  Check
                     * that we are expecting to be a slave, then switch to being a slave.
                     */
                    dprintf(DPRINTF_ELECTION, "PRH: Message type is IPC_ELECT_NOTIFY_SLAVE\n");

                    /*
                     * The controller should have the election task running when
                     * receiving the NOTIFY packet, so fail if it doesn't.
                     */
                    if (TestElectionInProgressFlag() == TRUE)
                    {
                        /*
                         * When the slave responds to the new master, its election serial
                         * number will be two less than the new master's election serial number.
                         */
                        if (EL_IsElectionSerialOlder(&electionSerial, &packetPtr->data->elect.ipcElectR1.electionSerial) == 2)
                        {
                            dprintf(DPRINTF_ELECTION, "PRH: Processing NOTIFY_SLAVE packet\n");

                            /*
                             * Check that we're in the WAIT_FOR_MASTER state before handling
                             * a NOTIFY_SLAVE packet.
                             */
                            if (EL_GetCurrentElectionState() == ED_STATE_WAIT_FOR_MASTER)
                            {
                                dprintf(DPRINTF_ELECTION, "PRH: Slave has been notified by master\n");

                                contactedByMaster = TRUE;
                            }
                            else if (EL_GetCurrentElectionState() == ED_STATE_CHECK_MASTER)
                            {
                                /*
                                 * This controller thought that it might take mastership, but
                                 * another controller beat it out.  In this case, force the
                                 * slave condition, and catch up with the new master.
                                 */
                                dprintf(DPRINTF_ELECTION, "PRH: NOTIFY_SLAVE while in CHECK_MASTER state - go slave\n");

                                contactedByMaster = TRUE;
                                EL_ChangeState(ED_STATE_WAIT_FOR_MASTER);
                            }
                            else if ((EL_GetCurrentElectionState() == ED_STATE_TIMEOUT_CONTROLLERS) ||
                                     (EL_GetCurrentElectionState() == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE))
                            {
                                /*
                                 * This controller was following the timeout branch, but
                                 * another controller has become master and is continuing.
                                 * Catch back up to the new master controller ASAP.
                                 */
                                dprintf(DPRINTF_ELECTION, "PRH: NOTIFY_SLAVE packet is preempting the timeout condition\n");

                                contactedByMaster = TRUE;
                                EL_ChangeState(ED_STATE_WAIT_FOR_MASTER);
                            }
                            else if (EL_GetCurrentElectionState() == ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE)
                            {
                                /*
                                 * This controller was still in the ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE
                                 * state, but another controller has become master and is continuing.
                                 * Catch back up to the new master controller ASAP.
                                 * This partially covers the case where the master goes theough
                                 * the CHECK_MASTER and NOTIFY_SLAVES state before the slave
                                 * controller sees the new master.
                                 */
                                dprintf(DPRINTF_ELECTION, "PRH: NOTIFY_SLAVE packet arrived before WAIT_FOR_MASTER state\n");

                                contactedByMaster = TRUE;
                                EL_ChangeState(ED_STATE_WAIT_FOR_MASTER);
                            }
                            else
                            {
                                dprintf(DPRINTF_ELECTION, "PRH: NOTIFY_SLAVE packet is unexpected - fail\n");

                                EL_ChangeState(ED_STATE_FAILED);
                            }
                        }
                        else
                        {
                            /* The controller missed an election, so fail. */
                            dprintf(DPRINTF_ELECTION, "PRH: Election serial number out of sequence - fail\n");

                            EL_ChangeState(ED_STATE_FAILED);
                        }
                    }
                    else
                    {
                        /*
                         * The controller probably missed an election.
                         * There's a special case here where a dying master may have sent a
                         * NOTIFY message after this controller completed the election.
                         * Watch for this special case, and ignore the NOTIFY if
                         * this happened.  Due to the way the election serial numbers
                         * are incremented, this controller's serial number should be
                         * greater than the dying master's by at least two.
                         */
                        dprintf(DPRINTF_ELECTION, "PRH: NOTIFY_SLAVE while election task is not running\n");

                        if (EL_IsElectionSerialNewer(&electionSerial,
                                                     &packetPtr->data->elect.ipcElectR1.electionSerial) >= 2)
                        {
                            dprintf(DPRINTF_ELECTION, "PRH: NOTIFY sent by dying master - ignore\n");
                        }
                        else
                        {
                            dprintf(DPRINTF_ELECTION, "PRH: Another controller is master - fail\n");
                            EL_ChangeState(ED_STATE_FAILED);
                        }
                    }
                    break;

                case IPC_ELECT_TIMEOUT:
                    /*
                     * This controller has received a 'TIMEOUT' message, which means
                     * that another controller has timed out during its election
                     * process.
                     */
                    dprintf(DPRINTF_ELECTION, "PRH: Message type is IPC_ELECT_TIMEOUT\n");

                    /*
                     * The controller should have the election task running when
                     * receiving the TIMEOUT packet.  We can get by with ignoring
                     * this packet if the election isn't in progress, since the
                     * following CONTACT packet will either cause the election
                     * to start or the controller to fail.
                     */
                    if (TestElectionInProgressFlag() == TRUE)
                    {
                        if (electionSerial.current == packetPtr->data->elect.ipcElectR1.electionSerial.current)
                        {
                            dprintf(DPRINTF_ELECTION, "PRH: Received an ELECT_TIMEOUT packet\n");

                            if (EL_GetCurrentElectionState() == ED_STATE_TIMEOUT_CONTROLLERS)
                            {
                                dprintf(DPRINTF_ELECTION, "PRH: Extending TIMEOUT_CONTROLLERS_COMPLETE timeout value\n");

                                /* Extend the TIMEOUT_CONTROLLERS timeout counter */
                                SetTCTimeoutCounter(CHECK_FOR_TIMEOUT_GRANULARITY);
                            }
                            else if (EL_GetCurrentElectionState() == ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE)
                            {
                                dprintf(DPRINTF_ELECTION, "PRH: Extending TIMEOUT_CONTROLLERS_COMPLETE timeout value\n");

                                /* Extend the TIMEOUT_CONTROLLERS_COMPLETE timeout counter */
                                SetTCCSTimeoutCounter(CHECK_FOR_TIMEOUT_GRANULARITY);
                            }
                            else if ((EL_GetCurrentElectionState() == ED_STATE_WAIT_FOR_MASTER) &&
                                     (contactedByMaster == TRUE))
                            {
                                /*
                                 * Do nothing.  Leave this timeout up to the WAIT_FOR_MASTER
                                 * code on this controller.  Don't allow an incoming packet
                                 * to change this controller's election state in this case.
                                 */
                                dprintf(DPRINTF_ELECTION, "PRH: Contacted by master - ignore ELECT_TIMEOUT packet\n");
                            }
                            else
                            {
                                dprintf(DPRINTF_ELECTION, "PRH: Switch to TIMEOUT_CONTROLLERS state\n");

                                EL_ChangeState(ED_STATE_TIMEOUT_CONTROLLERS);
                            }
                        }
                        else if (EL_IsElectionSerialNewer(&electionSerial, &packetPtr->data->elect.ipcElectR1.electionSerial) > 0)
                        {
                            dprintf(DPRINTF_ELECTION, "PRH: This controller has a newer election serial number\n");
                        }
                        else
                        {
                            /* The controller missed an election, so fail. */
                            dprintf(DPRINTF_ELECTION, "PRH: Election serial is out of sequence - fail\n");

                            EL_ChangeState(ED_STATE_FAILED);
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "PRH: Election not in progress - ignoring TIMEOUT message\n");

                        /*
                         * Note: This is probably a fatal condition, but the controller
                         *   will have to fall out later.  If the other controller
                         *   is calling for the timeout and we're not running an election,
                         *   then this controller will fall out when the CONTACT_CONTROLLER
                         *   message comes from the other controller.
                         *   This condition should only happen if this controller is
                         *   *extremely* delayed in starting the election task.
                         */
                    }
                    break;

                default:
                    /*
                     * This controller has received a message that isn't defined.
                     * This is a catch-all so that debug messages can be displayed
                     * and the problem found.  Since we don't know exactly what
                     * type of packet this is, don't attempt to handle it.
                     */
                    dprintf(DPRINTF_ELECTION, "PRH: Message type %d is UNKNOWN\n",
                            packetPtr->data->elect.ipcElectR1.messageType);
                    break;
            }

            /*
             * Change the packet so that the controller that sent this packet knows
             * what election serial number this controller is using and what state
             * it's currently in.
             */
            packetPtr->data->elect.ipcElectR1.electionSerial.starting = electionSerial.starting;
            packetPtr->data->elect.ipcElectR1.electionSerial.current = electionSerial.current;
            packetPtr->data->elect.ipcElectR1.electionTaskState = EL_GetCurrentElectionState();

            /*
             * Fill in our diskMap, but only if the packet is of the type and
             * size that can handle the diskMap data.
             */
            if (diskMapPacket == TRUE)
            {
                /* Process the incoming packet's diskMap data, if indicated */
                if (processDiskMapPacket == TRUE)
                {
                    EL_DiskMapReceive(&packetPtr->data->elect.ipcElectR3.diskMap, otherControllerCommSlot);
                }

                /* Place this controller's current diskMap into the response packet */
                dprintf(DPRINTF_ELECTION, "PRH: Copying diskMap into response packet\n");

                EL_DiskMapGet(&packetPtr->data->elect.ipcElectR3.diskMap);

                /*
                 * Set the flag to indicate that the response packet contains
                 * a valid diskMap from this controller.
                 */
                packetPtr->data->elect.ipcElectR3.diskMap.flags.bits.responseValid = TRUE;
            }

            /* Turn off PacketReceptionHandler semaphore */
            SetPacketReceptionInProgress(FALSE);
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "PRH: Ignoring message from own CommSlot\n");
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "PRH: NULL pointer in incoming packet - ignored\n");
        dprintf(DPRINTF_ELECTION, "PRH: packetPtr:         %p\n", packetPtr);

        if (packetPtr != NULL)
        {
            dprintf(DPRINTF_ELECTION, "PRH: packetPtr->header: %p\n", packetPtr->header);
            dprintf(DPRINTF_ELECTION, "PRH: packetPtr->data:   %p\n", packetPtr->data);
        }
    }

    return (packetPtr);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_ContactAllControllersCallback
**
**  Returns:       Nothing
**--------------------------------------------------------------------------*/
static void EL_ContactAllControllersCallback(TASK_PARMS *parms)
{
    PCONTACT_CONTROLLER_CALLBACK_PARMS callbackPtr = (PCONTACT_CONTROLLER_CALLBACK_PARMS)parms->p1;
    UINT8       callbackSlotNumber = 0;
    ELECTION_DATA_CONTACT_MAP_ITEM contactType = ED_CONTACT_MAP_CONTACT_FAILED;

    char        tempString[TEMP_STRING_LENGTH];

    if (callbackPtr != NULL)
    {
        /*
         * Find which communications slot corresponds with the controller that
         * made the callback.
         */
        callbackSlotNumber = callbackPtr->controllerSlotNumber;

        if (callbackSlotNumber == myControllerCommSlot)
        {
            /*
             * A controller should never receive a callback from its own slot.
             * If some software bug causes the condition, however, make sure
             * we throw the packet away.
             */
            dprintf(DPRINTF_ELECTION, "CACCB: Callback from self (slot %d) ignored\n", callbackSlotNumber);
        }
        else if (callbackSlotNumber >= MAX_CONTROLLERS)
        {
            /*
             * Timeouts seem to show up with slot 255.  Filter out all slots
             * that are outside of the acceptable range.  If MAX_CONTROLLERS is
             * four, then we want to allow slots 0-3.
             */
            dprintf(DPRINTF_ELECTION, "CACCB: Callback from out-of-range slot (slot %d)\n", callbackSlotNumber);
        }
        else
        {
            EL_GetSendPacketResultString(tempString,
                                         callbackPtr->callbackResults.result,
                                         sizeof(tempString));

            dprintf(DPRINTF_ELECTION, "CACCB: Callback from slot %d result: %s\n", callbackSlotNumber, tempString);

            /*
             * Make certain that the controller is still in the
             * 'CONTACT_ALL_CONTROLLERS' state before marking the controller
             * that's making the callback as participating in the election.
             */
            if (TestElectionContactAllControllersFlag() == TRUE)
            {
                /*
                 * Check that IPC has returned a non-NULL rxPacketPtr... it appears
                 * as though it will be NULL for the case where IPC has a TIMEOUT.
                 */
                if ((callbackPtr->rxPacketPtr != NULL) && (callbackPtr->rxPacketPtr->data != NULL))
                {
                    /*
                     * Make certain that the callback is for the election that this
                     * controller is currently participating in.  If the election
                     * serial number doesn't match, then don't count it as a valid
                     * response.
                     */
                    if (callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionSerial.current ==
                        myControllerCommArea.electionStateSector.electionData.electionSerial.current)
                    {
                        /*
                         * Set the item in the contactMap that corresponds to the controller that
                         * made the callback.  This will be looked at later to determine all of
                         * the controllers that are participating in the election.
                         */
                        switch (callbackPtr->callbackResults.result)
                        {
                            case SENDPACKET_ETHERNET:
                                contactType = ED_CONTACT_MAP_CONTACTED_ETHERNET;
                                myNumberOfControllersContactedEthernet++;
                                break;

                            case SENDPACKET_FIBRE:
                                contactType = ED_CONTACT_MAP_CONTACTED_FIBRE;
                                myNumberOfControllersContactedFibre++;
                                break;

                            case SENDPACKET_QUORUM:
                                contactType = ED_CONTACT_MAP_CONTACTED_QUORUM;
                                myNumberOfControllersContactedQuorum++;
                                break;

                            case SENDPACKET_TIME_OUT:
                            case SENDPACKET_NO_PATH:
                            case SENDPACKET_ANY_PATH:
                            default:
                                contactType = ED_CONTACT_MAP_CONTACT_TIMEOUT;
                                break;
                        }

                        /*
                         * If the callback packet's responseValid bit is set, then
                         * process the diskMap in the response packet.
                         */
                        if ((callbackPtr->rxPacketPtr->header->length == sizeof(IPC_ELECT_R3)) &&
                            (callbackPtr->rxPacketPtr->data->elect.ipcElectR3.diskMap.flags.bits.responseValid == TRUE))
                        {
                            EL_DiskMapReceive(&callbackPtr->rxPacketPtr->data->elect.ipcElectR3.diskMap, callbackSlotNumber);
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "CACCB: Invalid election serial number from slot %d (%d)\n",
                                callbackSlotNumber,
                                callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionSerial.current);

                        EL_GetElectionStateString(tempString,
                                                  callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionTaskState,
                                                  sizeof(tempString));

                        dprintf(DPRINTF_ELECTION, "CACCB:   State %s\n", tempString);

                        contactType = ED_CONTACT_MAP_CONTACT_FAILED;

                        /*
                         * Check if this controller is in an older election
                         * cycle than the other currently is in.  If it is, then
                         * fail, but only pay attention to the packets that were
                         * successfully transported over Ethernet/Fibre/Quorum
                         * from controllers that weren't currently running their
                         * election tasks.
                         */
                        switch (callbackPtr->callbackResults.result)
                        {
                            case SENDPACKET_ETHERNET:
                            case SENDPACKET_FIBRE:
                            case SENDPACKET_QUORUM:
                                if ((callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionTaskState == ED_STATE_END_TASK) &&
                                    (EL_IsElectionSerialOlder(&myControllerCommArea.electionStateSector.electionData.electionSerial,
                                      &callbackPtr->rxPacketPtr->data->elect.ipcElectR1.electionSerial) > 0))
                                {
                                    dprintf(DPRINTF_ELECTION, "CACCB: This controller's election serial is outdated - FAIL\n");

                                    EL_ChangeState(ED_STATE_FAILED);
                                }
                                break;

                            case SENDPACKET_TIME_OUT:
                            case SENDPACKET_NO_PATH:
                            case SENDPACKET_ANY_PATH:
                            default:
                                break;
                        }
                    }
                }
                else
                {
                    /*
                     * Set the item in the contactMap that corresponds to the controller that
                     * made the callback.  This will be looked at later to determine all of
                     * the controllers that are participating in the election.
                     */
                    dprintf(DPRINTF_ELECTION, "CACCB: NULL rxPacketPtr from slot %d\n", callbackSlotNumber);

                    contactType = ED_CONTACT_MAP_CONTACT_TIMEOUT;
                }

                EL_GetContactMapStateString(tempString, contactType, sizeof(tempString));

                dprintf(DPRINTF_ELECTION, "CACCB: Marking slot %d as %s\n", callbackSlotNumber, tempString);

                /* Write the contactType to the quorum */
                if (EL_SetContactMapItem(callbackSlotNumber, contactType) != GOOD)
                {
                    /* Force the FAILED state on a quorum write error */
                    EL_ChangeState(ED_STATE_FAILED);
                }
            }
            else
            {
                /*
                 * The controller that made the callback responded too late to the
                 * 'CONTACT_ALL_CONTROLLERS', so its not going to participate in
                 * the election.
                 */
                if (callbackPtr->callbackResults.result != SENDPACKET_TIME_OUT)
                {
                    dprintf(DPRINTF_ELECTION, "CACCB: Callback from slot %d ignored\n", callbackSlotNumber);
                }
            }
        }

        /* Deallocate the receive packet memory */
        FreePacket(&callbackPtr->rxPacketPtr, __FILE__, __LINE__);
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "CACCB: Callback pointer is NULL\n");

        /* Force the FAILED state on a quorum write error */
        EL_ChangeState(ED_STATE_FAILED);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_RebuildACM
**
**  Description: This function scans the controller communications areas
**               looking for all operational controllers.  When found, it
**               places the slot number into the active controller map.
**
**  Inputs:      masterCommSlot - Communications slot of master controller
**
**  Modifies:    Modifies active controller map (masterConfig)
**
**  Returns:     GOOD  - Active controller map rebuilt without error
**               ERROR - Error encountered while rebuilding active controller
**                       map.  Map is reset to default values.
**--------------------------------------------------------------------------*/
static UINT32 EL_RebuildACM(UINT16 masterCommSlot)
{
    UINT32      returnCode = GOOD;
    UINT16      slotCounter = 0;
    UINT8       activeControllerCounter = 0;

    char        tempString[TEMP_STRING_LENGTH];

    dprintf(DPRINTF_ELECTION, "RACM: Rebuilding the ACM\n");

    if (ReadAllCommunicationsWithRetries(gElectCommAreas) == 0)
    {
        /* Remember which slot belongs to the previous master controller */
        previousMasterCommSlot = masterCommSlot;

        /*
         * Give the masterCommSlot the first change to take the head of the
         * ACM, since this is where the master resides in the ACM.
         * Read the controller communications slot entry
         */
        if ((gElectCommAreas[previousMasterCommSlot].failStateSector.failureData.state == FD_STATE_OPERATIONAL) ||
            (gElectCommAreas[previousMasterCommSlot].failStateSector.failureData.state == FD_STATE_POR) ||
            (gElectCommAreas[previousMasterCommSlot].failStateSector.failureData.state == FD_STATE_ADD_CONTROLLER_TO_VCG) ||
            (gElectCommAreas[previousMasterCommSlot].failStateSector.failureData.state == FD_STATE_FIRMWARE_UPDATE_ACTIVE) ||
            (gElectCommAreas[previousMasterCommSlot].failStateSector.failureData.state == FD_STATE_UNFAIL_CONTROLLER) ||
            (gElectCommAreas[previousMasterCommSlot].failStateSector.failureData.state == FD_STATE_VCG_SHUTDOWN) ||
            (gElectCommAreas[previousMasterCommSlot].failStateSector.failureData.state == FD_STATE_ACTIVATE))
        {
            dprintf(DPRINTF_ELECTION, "RACM: Previous master is still active (Slot %d)\n", masterCommSlot);

            EL_GetFailureDataStateString(tempString,
                                         gElectCommAreas[previousMasterCommSlot].failStateSector.failureData.state,
                                         sizeof(tempString));

            dprintf(DPRINTF_ELECTION, "RACM: ACM[%d]: Slot %d - %s\n", activeControllerCounter, masterCommSlot, tempString);

            /* Indicate in the master configuration that the slot is active */
            EL_SetACMNode(activeControllerCounter, masterCommSlot);

            /* Increment the number of active controllers */
            activeControllerCounter++;
        }
        else
        {
            /* Indicate that the previous master controller is inactive */
            previousMasterCommSlot = ACM_NODE_UNDEFINED;

            EL_GetFailureDataStateString(tempString,
                                         gElectCommAreas[previousMasterCommSlot].failStateSector.failureData.state,
                                         sizeof(tempString));

            dprintf(DPRINTF_ELECTION, "RACM: Previous master is inactive (Slot %d) - %s\n", masterCommSlot, tempString);
        }

        dprintf(DPRINTF_ELECTION, "RACM: previousMasterCommSlot: %d\n", previousMasterCommSlot);

        /*
         * Traverse the controller communications area, looking for all controllers
         * that are currently marked as operational.
         */
        for (slotCounter = 0; ((slotCounter < MAX_CONTROLLERS) && (returnCode == GOOD)); slotCounter++)
        {
            /*
             * We've already looked at the previous master's slot, so skip
             * it this time through.
             */
            if (slotCounter != masterCommSlot)
            {
                if ((gElectCommAreas[slotCounter].failStateSector.failureData.state == FD_STATE_OPERATIONAL) ||
                    (gElectCommAreas[slotCounter].failStateSector.failureData.state == FD_STATE_POR) ||
                    (gElectCommAreas[slotCounter].failStateSector.failureData.state == FD_STATE_ADD_CONTROLLER_TO_VCG) ||
                    (gElectCommAreas[slotCounter].failStateSector.failureData.state == FD_STATE_FIRMWARE_UPDATE_ACTIVE) ||
                    (gElectCommAreas[slotCounter].failStateSector.failureData.state == FD_STATE_UNFAIL_CONTROLLER) ||
                    (gElectCommAreas[slotCounter].failStateSector.failureData.state == FD_STATE_VCG_SHUTDOWN) ||
                    (gElectCommAreas[slotCounter].failStateSector.failureData.state == FD_STATE_ACTIVATE))
                {
                    EL_GetFailureDataStateString(tempString,
                                                 gElectCommAreas[slotCounter].failStateSector.failureData.state,
                                                 sizeof(tempString));

                    dprintf(DPRINTF_ELECTION, "RACM: ACM[%d]: Slot %d - %s\n", activeControllerCounter, slotCounter, tempString);

                    /* Indicate in the master configuration that the slot is active */
                    EL_SetACMNode(activeControllerCounter, slotCounter);

                    /* Increment the number of active controllers */
                    activeControllerCounter++;
                }
                else
                {
                    EL_GetFailureDataStateString(tempString,
                                                 gElectCommAreas[slotCounter].failStateSector.failureData.state,
                                                 sizeof(tempString));

                    dprintf(DPRINTF_ELECTION, "RACM: Inactive controller (Slot %d) - %s\n", slotCounter, tempString);
                }

            }

            if (slotCounter != myControllerCommSlot)
            {
                /*
                 * See if a controller possibly wrote a state to less drives than we see.
                 * If it did, re-write the failure state so that if any drives were missed when
                 * the other controller wrote them they will be written now.
                 */
                if ((gElectCommAreas[slotCounter].failStateSector.failureData.state != FD_STATE_UNUSED) &&
                    ((gElectCommAreas[myControllerCommSlot].failStateSector.failureData.state == FD_STATE_OPERATIONAL) ||
                     (gElectCommAreas[myControllerCommSlot].failStateSector.failureData.state == FD_STATE_POR) ||
                     (gElectCommAreas[myControllerCommSlot].failStateSector.failureData.state == FD_STATE_ADD_CONTROLLER_TO_VCG) ||
                     (gElectCommAreas[myControllerCommSlot].failStateSector.failureData.state == FD_STATE_FIRMWARE_UPDATE_ACTIVE) ||
                     (gElectCommAreas[myControllerCommSlot].failStateSector.failureData.state == FD_STATE_UNFAIL_CONTROLLER) ||
                     (gElectCommAreas[myControllerCommSlot].failStateSector.failureData.state == FD_STATE_VCG_SHUTDOWN) ||
                     (gElectCommAreas[myControllerCommSlot].failStateSector.failureData.state == FD_STATE_ACTIVATE)))
                {
                    FIO_DISK_MAP readMap = { 0 };

                    /*
                     * The readmap is an intersection of disks seen by all controllers.
                     * If we see more disks than are in the readmap, then another
                     * controller cannot see all the disks that we do.
                     */
                    if (FIO_GetReadableDiskMap(&readMap) == GOOD)
                    {
                        ELECTION_DISK_MAP diskMap;

                        /*
                         * Get the disks we can see and compare the number with the disks
                         * in the readmap.  If we have more, re-write the failed state.
                         */
                        if ((EL_DiskMapGet(&diskMap) == GOOD) &&
                            (FIO_GetNumDiskMapDisks(&readMap) < FIO_GetNumDiskMapDisks(&diskMap.local)))
                        {
                            dprintf(DPRINTF_ELECTION, "RACM: Rewriting failure state (Slot %d) - %s\n", slotCounter, tempString);
                            WriteFailureDataWithRetries(GetControllerSN(slotCounter),
                                                        &gElectCommAreas[slotCounter].failStateSector.failureData);
                        }
                    }
                }
            }
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "RACM: Error returned from ReadAllCommunications\n");

        returnCode = ERROR;
    }


    /*
     * Check if any errors were encountered in rebuild.  If any errors
     * were encountered, transition to the FAILED state.
     */
    if (returnCode == GOOD)
    {
        dprintf(DPRINTF_ELECTION, "RACM: %d active controller(s)\n", activeControllerCounter);

        /*
         * If there were no errors, clear the unused portion of the active
         * controller map and write it to the quorum.
         */
        for (slotCounter = activeControllerCounter; slotCounter < MAX_CONTROLLERS; slotCounter++)
        {
            /* Indicate in the master configuration that the slot is not valid */
            EL_SetACMNode(slotCounter, ACM_NODE_UNDEFINED);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "RACM: Error while rebuilding the ACM\n");

        EL_ChangeState(ED_STATE_FAILED);
    }

    return (returnCode);
}

/*----------------------------------------------------------------------------
**  Function Name: EL_SetContactMapItem
**
**  Parameters: slotNumber - slot number of the item to read from the
**                           local controller's election data array
**              newState   - desired state of the item
**
**  Returns:    GOOD -  Item set successfully
**              ERROR - Error setting item
**--------------------------------------------------------------------------*/
static UINT32 EL_SetContactMapItem(UINT16 slotNumber,
                                   ELECTION_DATA_CONTACT_MAP_ITEM newState)
{
    UINT32      returnCode = ERROR;

    if (slotNumber < MAX_CONTROLLERS)
    {
        /*
         * Update the gElectCommAreas structure.  We want to keep this controller's
         * commArea data fresh, and since gElectCommAreas only gets refreshed with
         * a ReadAllCommunications, we need to update our controller's data here.
         */
        gElectCommAreas[myControllerCommSlot].electionStateSector.electionData.contactMap[slotNumber] = newState;

        /* Write the contact map (contained in the election data) to the quorum. */
        myControllerCommArea.electionStateSector.electionData.contactMap[slotNumber] = newState;

        if (WriteElectionDataWithRetries(myControllerSerialNumber, &myControllerCommArea.electionStateSector.electionData) == 0)
        {
            /* Update the file system 'good disk' map */
            EL_DiskMapUpdateFIOMap();

            returnCode = GOOD;
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "SCMI: WriteElectionData returned ERROR\n");
        }
    }

    if (returnCode != GOOD)
    {
        dprintf(DPRINTF_ELECTION, "SCMI: Failed to alter contact map slot %d\n", slotNumber);
    }

    return (returnCode);
}

/*----------------------------------------------------------------------------
**  Function Name: EL_TakeMastership
**
**  Comments:   This changes the state of the controller so that it can
**              read/write to the master configuration record.
**              NOTE: This does not write the new master config record.  A
**                    call to SaveMasterConfig must be done after this.
**
**  Returns:    GOOD -  Mastership acquired successfully
**              ERROR - Error acquiring mastership
**--------------------------------------------------------------------------*/
static UINT32 EL_TakeMastership(void)
{
    UINT32      returnCode = GOOD;
    LOG_CONTROLLER_FAIL_PKT logMsg;
    UINT8       activeMapCounter = 0;
    UINT8       commSlot = 0;
    UINT8       failedControllerFlag = FALSE;

    memset(&logMsg, 0, sizeof(logMsg));

    dprintf(DPRINTF_ELECTION, "TM: This controller is taking mastership\n");

    /* Set local cached controller failure state */
    SetControllerFailureState(FD_STATE_OPERATIONAL);

    /*
     * Switch to master mode so the controller can write to the master
     * configuration record.
     */
    Qm_SetMasterControllerSN(myControllerSerialNumber);

    /* Place our IP address into the masterConfig record */
    Qm_SetIPAddress(SerialNumberToIPAddress(myControllerSerialNumber, SENDPACKET_ETHERNET));

    /*
     * Increment our electionSerial.current by two in order to distinguish the
     * TakeMastership from the TimoutController behavior, which only bumps the
     * electionSerial by one.  Save into our copy of the masterConfig record.
     */
    myControllerCommArea.electionStateSector.electionData.electionSerial.current += 2;
    Qm_SetElectionSerial(myControllerCommArea.electionStateSector.electionData.electionSerial.current);

    /*
     * Fail all controllers that didn't respond to the election.  We only need to
     * search the controllers that are listed in the active control map, since
     * all the other controllers will already be FD_STATE_FAILED.
     */
    for (activeMapCounter = 0;
         (returnCode == GOOD) && (activeMapCounter < ACM_GetActiveControllerCount(&electionACM));
         activeMapCounter++)
    {
        commSlot = EL_GetACMNode(activeMapCounter);

        /*
         * Scan through the election data that was gathered in the CONTACT_ALL_CONTROLLERS phase
         * and take action based upon what response we received when this controller tried to
         * contact the other controller.  Fail any and all controllers that didn't respond to us
         * (the new master) by means either the ethernet or fibre paths.  Make sure that any
         * good controllers are marked at OPERATIONAL.
         */
        if (((myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] == ED_CONTACT_MAP_CONTACTED_ETHERNET) ||
             (myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] == ED_CONTACT_MAP_CONTACTED_FIBRE)) &&
            (gElectCommAreas[commSlot].failStateSector.failureData.state != FD_STATE_FAILED) &&
            (gElectCommAreas[commSlot].electionStateSector.electionData.state != ED_STATE_FAILED))
        {
            dprintf(DPRINTF_ELECTION, "TM: Making controller in slot %d operational\n", commSlot);

            if (gElectCommAreas[commSlot].failStateSector.failureData.state != FD_STATE_OPERATIONAL)
            {
                gElectCommAreas[commSlot].failStateSector.failureData.state = FD_STATE_OPERATIONAL;

                /*
                 * Modify the controller's failStateSector to indicate that it is operational
                 * to the rest of the control group.  This eventually gets read by the ACMRebuild function
                 * and the controller is added to the active controller group.  This should make any
                 * controller that was previously marked as ADD_CONTROLLER_TO_VCG, FIRMWARE_UPDATE_ACTIVE,
                 * or UNFAIL_CONTROLLER are now seen as being OPERATIONAL.
                 */
                if (WriteFailureDataWithRetries(GetControllerSN(commSlot), &gElectCommAreas[commSlot].failStateSector.failureData) != 0)
                {
                    dprintf(DPRINTF_ELECTION, "TM: Read error making controller in slot %d operational\n", commSlot);

                    returnCode = ERROR;
                }
            }
        }
        else
        {
            /*
             * Set a flag to indicate that the master has failed a controller
             * during this election.  We'll use this later to determine if we
             * need to save the backtrace data or not.
             */
            failedControllerFlag = TRUE;

            /* Perform the failure action */
            dprintf(DPRINTF_ELECTION, "TM: Failing controller in slot %d\n", commSlot);

            gElectCommAreas[commSlot].failStateSector.failureData.state = FD_STATE_FAILED;

            /*
             * Modify the failed controller's failStateSector to indicate the failure to the
             * rest of the control group.  This eventually gets read by the ACMRebuild function and the
             * controller is withdrawn from the active controller group (made inactive - FAILED).
             */
            if (WriteFailureDataWithRetries(GetControllerSN(commSlot), &gElectCommAreas[commSlot].failStateSector.failureData) != 0)
            {
                dprintf(DPRINTF_ELECTION, "TM: Write error failing controller in slot %d\n", commSlot);

                returnCode = ERROR;
            }
            else
            {
                /* Generate log of controller being failed (R1 data) */
                logMsg.r2.r1.controllerSN = GetMyControllerSN();
                logMsg.r2.r1.failedControllerSN = GetControllerSN(commSlot);

                /* Fill in a reason why the master is failing this controller (R2 data) */
                if (myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] == ED_CONTACT_MAP_CONTACTED_QUORUM)
                {
                    strncpy((char *)logMsg.r2.reasonString, "POOR COMM", sizeof(logMsg.r2.reasonString));
                }
                else if (myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] == ED_CONTACT_MAP_CONTACT_FAILED)
                {
                    strncpy((char *)logMsg.r2.reasonString, "MISMATCH", sizeof(logMsg.r2.reasonString));
                }
                else if (myControllerCommArea.electionStateSector.electionData.contactMap[commSlot] == ED_CONTACT_MAP_CONTACT_TIMEOUT)
                {
                    strncpy((char *)logMsg.r2.reasonString, "COMM TMO", sizeof(logMsg.r2.reasonString));
                }
                else
                {
                    strncpy((char *)logMsg.r2.reasonString, "NO COMM", sizeof(logMsg.r2.reasonString));
                }

                /* Fill in the contactType for this slot (R3 data) */
                logMsg.contactType = myControllerCommArea.electionStateSector.electionData.contactMap[commSlot];

                SendAsyncEvent(LOG_CONTROLLER_FAIL, sizeof(logMsg), &logMsg);
            }
        }
    }

    /* Rebuild the active controller map */
    if (returnCode == GOOD)
    {
        if (EL_RebuildACM(myControllerCommSlot) == GOOD)
        {
            /*
             * Copy the ACM from the election code's protected ACM structure
             * into the quorum's masterConfig.  The call to RebuildACM above
             * updates only the electionACM, but we also need to have it
             * update the copy that'll be written to the masterConfig on disk.
             */
            memcpy(&masterConfig.activeControllerMap, &electionACM, sizeof(masterConfig.activeControllerMap));
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "TM: Failed to rebuild the ACM\n");

            returnCode = ERROR;
        }
    }

    /* Write the new master configuration record to NVRAM and disk */
    if (returnCode == GOOD)
    {
        SaveMasterConfig();

        /*
         * Save the backtrace information.  Doing this here allows us to see
         * both sides of the election suicide (from the failing controller, and
         * from the new master controller).  Without this, we only get backtrace
         * from the controller that we failed, and not from the new master.
         */
        if (failedControllerFlag == TRUE)
        {
            CopyBacktraceDataToNVRAM();
        }
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_NotifyOtherTasks
**
**  Returns:        GOOD
**                  ERROR
**--------------------------------------------------------------------------*/
UINT8 EL_NotifyOtherTasks(UINT8 newTaskState)
{
    UINT8       returnCode = GOOD;

    char        tempString[TEMP_STRING_LENGTH];

    EL_GetTaskStateString(tempString, newTaskState, sizeof(tempString));
    dprintf(DPRINTF_ELECTION, "NOT: Notifying other tasks - state is %s\n", tempString);

    /*
     * Put calls to all tasks that need to be notified
     * of election state changes here.
     */
    if (RM_ElectionNotify(newTaskState) != GOOD)
    {
        dprintf(DPRINTF_ELECTION, "NOT: Resource Manager notify failure\n");

        returnCode = ERROR;
    }

    if (HealthMonitor_ElectionNotify(newTaskState) != GOOD)
    {
        dprintf(DPRINTF_ELECTION, "NOT: Health Monitor notify failure\n");

        returnCode = ERROR;
    }

    if (SlaveFailure_ElectionNotify(newTaskState) != GOOD)
    {
        dprintf(DPRINTF_ELECTION, "NOT: Slave Failure Manager notify failure\n");

        returnCode = ERROR;
    }

    if (X1_ElectionNotify(newTaskState) != GOOD)
    {
        dprintf(DPRINTF_ELECTION, "NOT: X1 Interface notify failure\n");

        returnCode = ERROR;
    }

    if (IPC_ElectionNotify(newTaskState) != GOOD)
    {
        dprintf(DPRINTF_ELECTION, "NOT: X1 IPC_ElectionNotify failure\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_GetCurrentElectionState
**
**  Comments:      This allows funnctions outside this module to query
**                 the state of the election task, without endangering
**                 the private currentElectionState variable.
**
**  Parameters:    None
**
**  Returns:       ELECTION_DATA_STATE stateNumber
**--------------------------------------------------------------------------*/
ELECTION_DATA_STATE EL_GetCurrentElectionState(void)
{
    /* Wait for any election state change to finish before returning */
    EL_WaitForStateChangeComplete();

    return (EL_GetCurrentState());
}


/*----------------------------------------------------------------------------
**  Function Name: EL_WaitForStateChangeComplete
**
**  Comments:      This function simply waits for any state change that's
**                 in progress to complete before returning.  If no state
**                 change is in progress, then it returns immediately.
**
**  Parameters:    None
**
**  Returns:       Nothing
**--------------------------------------------------------------------------*/
static void EL_WaitForStateChangeComplete(void)
{
    /* Wait for any election state change to finish before returning */
    while (TestStateChangeInProgressFlag() == TRUE)
    {
        TaskSleepMS(20);
    }
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_ClearContactMap
**
**  Inputs:         None
**
**  Modifies:       myControllerCommArea - NOT WRITTEN TO QUORUM
**
**  Returns:        GOOD  - contactMap cleared and written to quorum
**                  ERROR - problem encountered writing to quorum
**--------------------------------------------------------------------------*/
static UINT8 EL_ClearContactMap(void)
{
    UINT8       returnCode = GOOD;
    UINT8       contactMapCounter;

    /* Clear the election data contactMap array before entering the election */
    for (contactMapCounter = 0;
         contactMapCounter < dimension_of(myControllerCommArea.electionStateSector.electionData.contactMap);
         contactMapCounter++)
    {
        myControllerCommArea.electionStateSector.electionData.contactMap[contactMapCounter] = ED_CONTACT_MAP_NO_ACTIVITY;
    }

    /*
     * We do not need to write this out to the quorum, since the
     * election state change code will write the electionData out
     * to the quorum for us.  This will eliminate a redundant
     * write to the quorum.
     */
    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_CheckMastershipAbility
**
**  Description:    Check all of this controller's connectivity such that we
**                  can tell if this controller is capable of becoming master
**                  of the VCG.  This does not need to include checking
**                  connectivity with the ICON.
**
**  Inputs:         None
**
**  Modifies:       myControllerCommArea.electionStateSector.electionData.mastershipAbility
**
**  Returns:        GOOD  - Mastership capability checked without error
**                  ERROR - Error while checking mastership capability
**--------------------------------------------------------------------------*/
static UINT8 EL_CheckMastershipAbility(void)
{
    UINT8       returnCode = GOOD;

    /*
     * Test the criteria that the master controller needs to meet in order
     * become a master controller.  If all of the criteria are met, then
     * set the mastershipAbility to QUALIFIED.
     */
    if ((myControllerCommArea.failStateSector.failureData.state == FD_STATE_OPERATIONAL) ||
        (myControllerCommArea.failStateSector.failureData.state == FD_STATE_POR) ||
        (myControllerCommArea.failStateSector.failureData.state == FD_STATE_FIRMWARE_UPDATE_ACTIVE))
    {
        dprintf(DPRINTF_ELECTION, "CMA: Marking this controller as QUALIFIED\n");

        myControllerCommArea.electionStateSector.electionData.mastershipAbility = ED_MASTERSHIP_ABILITY_QUALIFIED;
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "CMA: Marking this controller as NOT_QUALIFIED due to failureData.state\n");

        myControllerCommArea.electionStateSector.electionData.mastershipAbility = ED_MASTERSHIP_ABILITY_NOT_QUALIFIED;
    }

    /*
     * We do not need to write this out to the quorum, since the
     * election state change code will write the electionData out
     * to the quorum for us.  This will eliminate a redundant
     * write to the quorum.
     */
    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_CheckICONConnectivity
**
**  Description:    Check all of this controller's connectivity to the ICON
**
**  Inputs:         None
**
**  Modifies:       myControllerCommArea.electionStateSector.electionData.iconConnectivity
**
**  Returns:        GOOD  - icon connectivity checked without error
**                  ERROR - Error while checking icon connectivity
**--------------------------------------------------------------------------*/
static UINT8 EL_CheckICONConnectivity(void)
{
    UINT32      timeElapsedSinceLastAccess = 0;
    UINT8       returnCode = GOOD;

    /*
     * Test the criteria that any controller needs to meet in order
     * determine if they have connectivity to the icon.  If all of the criteria
     * are met, then set the iconConnectivity to CONNECTED.
     */
    timeElapsedSinceLastAccess = TimeSinceLastAccess();

    dprintf(DPRINTF_ELECTION, "CIC: %d seconds elapsed since last ICON/CCBE access\n", timeElapsedSinceLastAccess);

    if (timeElapsedSinceLastAccess <= ICON_CONNECTIVITY_VALID_LIMIT)
    {
        dprintf(DPRINTF_ELECTION, "CIC: Marking this controller as CONNECTED\n");

        myControllerCommArea.electionStateSector.electionData.iconConnectivity = ED_ICON_CONNECTIVITY_CONNECTED;
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "CIC: Marking this controller as NOT_CONNECTED\n");

        myControllerCommArea.electionStateSector.electionData.iconConnectivity = ED_ICON_CONNECTIVITY_NOT_CONNECTED;
    }

    /*
     * We do not need to write this out to the quorum, since the
     * election state change code will write the electionData out
     * to the quorum for us.  This will eliminate a redundant
     * write to the quorum.
     */
    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_IsElectionSerialNewer
**
**  Returns:       0    - if serial numbers are newer or equal
**                 >0   - if compareSerial is newer than againstSerial
**--------------------------------------------------------------------------*/
static UINT32 EL_IsElectionSerialNewer(ELECTION_SERIAL *compareElectionSerialPtr,
                                       ELECTION_SERIAL *againstElectionSerialPtr)
{
    UINT32      returnValue = 0;

    if ((compareElectionSerialPtr != NULL) && (againstElectionSerialPtr != NULL))
    {
        dprintf(DPRINTF_ELECTION, "IESN: CS: %d, AS: %d, CC: %d, AC: %d\n",
                compareElectionSerialPtr->starting, againstElectionSerialPtr->starting,
                compareElectionSerialPtr->current, againstElectionSerialPtr->current);

        /* Check for compareElectionSerial wrap */
        if (compareElectionSerialPtr->current >= compareElectionSerialPtr->starting)
        {
            /*
             * compareElectionSerial did not wrap, now check for
             * againstElectionSerial wrap
             */
            if (againstElectionSerialPtr->current >= againstElectionSerialPtr->starting)
            {
                /*
                 * compareElectionSerial did NOT wrap,
                 * againstElectionSerial did NOT wrap
                 *
                 * compareElectionSerialPtr->starting = 0x00000000
                 * againstElectionSerialPtr->starting = 0x00000000
                 *
                 * compareElectionSerialPtr->current  = 0x00000015
                 * againstElectionSerialPtr->current  = 0x00000010
                 */
                if (compareElectionSerialPtr->current > againstElectionSerialPtr->current)
                {
                    returnValue = compareElectionSerialPtr->current - againstElectionSerialPtr->current;
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION_VERBOSE, "IESN: againstElectionSerial wrapped\n");

                /*
                 * compareElectionSerial did NOT wrap, againstElectionSerial wrapped
                 * NOTE: compareElectionSerial is older than againstElectionSerial,
                 * so this will always return zero
                 *
                 * compareElectionSerialPtr->starting = 0xFFFFFFF0
                 * againstElectionSerialPtr->starting = 0xFFFFFFF0
                 *
                 * compareElectionSerialPtr->current  = 0xFFFFFFF4
                 * againstElectionSerialPtr->current  = 0x00000001
                 */
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION_VERBOSE, "IESN: compareElectionSerial wrapped\n");

            /* compareElectionSerial wrapped, now check for againstElectionSerial wrap */
            if (againstElectionSerialPtr->current >= againstElectionSerialPtr->starting)
            {
                dprintf(DPRINTF_ELECTION_VERBOSE, "IESN: compareElectionSerial wrapped, againstElectionSerial did NOT wrap\n");

                /*
                 * compareElectionSerial wrapped, againstElectionSerial did NOT wrap
                 * NOTE: compareElectionSerial is newer than againstElectionSerial,
                 * but account for the wrap condition when finding how much older.
                 *
                 * compareElectionSerialPtr->starting = 0xFFFFFFF0
                 * againstElectionSerialPtr->starting = 0xFFFFFFF0
                 *
                 * compareElectionSerialPtr->current  = 0x00000001
                 * againstElectionSerialPtr->current  = 0xFFFFFFF4
                 */
                returnValue = 0xFFFFFFFF - againstElectionSerialPtr->current + compareElectionSerialPtr->current + 1;
            }
            else
            {
                /*
                 * compareElectionSerial wrapped, againstElectionSerial wrapped
                 *
                 * compareElectionSerialPtr->starting = 0xFFFFFFF0
                 * againstElectionSerialPtr->starting = 0xFFFFFFF0
                 *
                 * compareElectionSerialPtr->current  = 0x00000015
                 * againstElectionSerialPtr->current  = 0x00000010
                 */
                if (compareElectionSerialPtr->current > againstElectionSerialPtr->current)
                {
                    returnValue = compareElectionSerialPtr->current - againstElectionSerialPtr->current;
                }
            }
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "IESN: compareElectionSerialPtr or againstElectionSerialPtr is NULL\n");
    }

    dprintf(DPRINTF_ELECTION, "IESN: Returning %d\n", returnValue);

    return (returnValue);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_IsElectionSerialOlder
**
**  Returns:       0    - if serial numbers are newer or equal
**                 >0   - if compareSerial is older than againstSerial
**--------------------------------------------------------------------------*/
static UINT32 EL_IsElectionSerialOlder(ELECTION_SERIAL *compareElectionSerialPtr,
                                       ELECTION_SERIAL *againstElectionSerialPtr)
{
    UINT32      returnValue = 0;

    if ((compareElectionSerialPtr != NULL) && (againstElectionSerialPtr != NULL))
    {
        dprintf(DPRINTF_ELECTION, "IESO: CS: %d, AS: %d, CC: %d, AC: %d\n",
                compareElectionSerialPtr->starting, againstElectionSerialPtr->starting,
                compareElectionSerialPtr->current, againstElectionSerialPtr->current);

        /* Check for compareElectionSerial wrap */
        if (compareElectionSerialPtr->current >= compareElectionSerialPtr->starting)
        {
            /*
             * compareElectionSerial did not wrap, now check for
             * againstElectionSerial wrap
             */
            if (againstElectionSerialPtr->current >= againstElectionSerialPtr->starting)
            {
                /*
                 * compareElectionSerial did NOT wrap,
                 * againstElectionSerial did NOT wrap
                 *
                 * compareElectionSerialPtr->starting = 0x00000000
                 * againstElectionSerialPtr->starting = 0x00000000
                 *
                 * compareElectionSerialPtr->current  = 0x00000010
                 * againstElectionSerialPtr->current  = 0x00000015
                 */
                if (againstElectionSerialPtr->current > compareElectionSerialPtr->current)
                {
                    returnValue = againstElectionSerialPtr->current - compareElectionSerialPtr->current;
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION_VERBOSE, "IESO: againstElectionSerial wrapped\n");

                /*
                 * compareElectionSerial did NOT wrap, againstElectionSerial wrapped
                 * NOTE: compareElectionSerial is older than againstElectionSerial,
                 * but account for the wrap condition when finding how much older.
                 *
                 * compareElectionSerialPtr->starting = 0xFFFFFFF0
                 * againstElectionSerialPtr->starting = 0xFFFFFFF0
                 *
                 * compareElectionSerialPtr->current  = 0xFFFFFFF4
                 * againstElectionSerialPtr->current  = 0x00000001
                 */
                returnValue = 0xFFFFFFFF - compareElectionSerialPtr->current + againstElectionSerialPtr->current + 1;
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION_VERBOSE, "IESO: compareElectionSerial wrapped\n");

            /* compareElectionSerial wrapped, now check for againstElectionSerial wrap */
            if (againstElectionSerialPtr->current >= againstElectionSerialPtr->starting)
            {
                dprintf(DPRINTF_ELECTION_VERBOSE, "IESO: compareElectionSerial wrapped, againstElectionSerial did NOT wrap\n");

                /*
                 * compareElectionSerial wrapped, againstElectionSerial did NOT wrap
                 * NOTE: compareElectionSerial is newer than againstElectionSerial,
                 * so this will always return zero
                 *
                 * compareElectionSerialPtr->starting = 0xFFFFFFF0
                 * againstElectionSerialPtr->starting = 0xFFFFFFF0
                 *
                 * compareElectionSerialPtr->current  = 0x00000001
                 * againstElectionSerialPtr->current  = 0xFFFFFFF4
                 */
            }
            else
            {
                /*
                 * compareElectionSerial wrapped, againstElectionSerial wrapped
                 *
                 * compareElectionSerialPtr->starting = 0xFFFFFFF0
                 * againstElectionSerialPtr->starting = 0xFFFFFFF0
                 *
                 * compareElectionSerialPtr->current  = 0x00000010
                 * againstElectionSerialPtr->current  = 0x00000015
                 */
                if (againstElectionSerialPtr->current > compareElectionSerialPtr->current)
                {
                    returnValue = againstElectionSerialPtr->current - compareElectionSerialPtr->current;
                }
            }
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "IESO: compareElectionSerialPtr or againstElectionSerialPtr is NULL\n");
    }

    dprintf(DPRINTF_ELECTION, "IESO: Returning %d\n", returnValue);

    return (returnValue);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_HowManyHaveTalkedToSlot
**
**  Inputs:     controllerCommSlot
**
**  Returns:    Number of controllers in the VCG that were able to
**              communicate with the controller in the specified slot.
**--------------------------------------------------------------------------*/
static UINT32 EL_HowManyHaveTalkedToSlot(UINT16 controllerCommSlot)
{
    UINT32      contactCount = 0;
    UINT32      returnCode = GOOD;
    UINT8       activeMapCounter = 0;
    UINT8       commSlot = 0;

    dprintf(DPRINTF_ELECTION, "HMHTTS: Counting how many controllers have contacted slot %d\n", controllerCommSlot);

    /* Scan through the election data for all controllers */
    for (activeMapCounter = 0;
         activeMapCounter < ACM_GetActiveControllerCount(&electionACM);
         activeMapCounter++)
    {
        commSlot = EL_GetACMNode(activeMapCounter);

        /* Don't look to see if the controller could contact itself */
        if (controllerCommSlot != commSlot)
        {
            /*
             * Check to be sure the controller is not failed before looking at
             * its contact map information.
             */
            if (gElectCommAreas[commSlot].failStateSector.failureData.state != FD_STATE_FAILED)
            {
                if (gElectCommAreas[commSlot].electionStateSector.electionData.electionSerial.current ==
                    myControllerCommArea.electionStateSector.electionData.electionSerial.current)
                {
                    if ((gElectCommAreas[commSlot].electionStateSector.electionData.state != ED_STATE_FINISHED) &&
                        (gElectCommAreas[commSlot].electionStateSector.electionData.state != ED_STATE_FAILED) &&
                        (gElectCommAreas[commSlot].electionStateSector.electionData.state != ED_STATE_END_TASK))
                    {
                        if ((gElectCommAreas[commSlot].electionStateSector.electionData.contactMap[controllerCommSlot] == ED_CONTACT_MAP_CONTACTED_ETHERNET) ||
                            (gElectCommAreas[commSlot].electionStateSector.electionData.contactMap[controllerCommSlot] == ED_CONTACT_MAP_CONTACTED_FIBRE) ||
                            (gElectCommAreas[commSlot].electionStateSector.electionData.contactMap[controllerCommSlot] == ED_CONTACT_MAP_CONTACTED_QUORUM) ||
                            (gElectCommAreas[commSlot].electionStateSector.electionData.contactMap[controllerCommSlot] == ED_CONTACT_MAP_CONTROLLER_TIMED_OUT) ||
                            (gElectCommAreas[commSlot].electionStateSector.electionData.contactMap[controllerCommSlot] == ED_CONTACT_MAP_SLAVE_NOTIFIED))
                        {
                            dprintf(DPRINTF_ELECTION, "HMHTTS: Slot %d has contacted slot %d\n", commSlot, controllerCommSlot);

                            contactCount++;
                        }
                        else
                        {
                            dprintf(DPRINTF_ELECTION, "HMHTTS: Slot %d has NOT contacted slot %d\n", commSlot, controllerCommSlot);
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "HMHTTS: Slot %d is no longer in the election\n", commSlot);
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "HMHTTS: Slot %d has a different electionSerial.current\n", commSlot);
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "HMHTTS: Slot %d is FAILED\n", commSlot);
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION_VERBOSE, "HMHTTS: Not looking to see if slot %d could contact itself\n", commSlot);
        }
    }

    if (returnCode == GOOD)
    {
        dprintf(DPRINTF_ELECTION, "HMHTTS: %d controllers have contacted slot %d\n",
                contactCount, controllerCommSlot);
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "HMHTTS: Error while counting contacts\n");

        EL_ChangeState(ED_STATE_FAILED);
        contactCount = 0;
    }

    return (contactCount);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_TestInProgress
**
**  Inputs:     None
**
**  Returns:    TRUE  - Election is in progress
**              FALSE - Election is not in progress
**--------------------------------------------------------------------------*/
UINT32 EL_TestInProgress(void)
{
    UINT32      returnCode = FALSE;

    if ((TestElectionStartingFlag() == TRUE) || (TestElectionInProgressFlag() == TRUE) || (TestElectionEndingFlag() == TRUE))
    {
        returnCode = TRUE;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_StartIO
**
**  Inputs:     None
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static UINT32 EL_StartIO(void)
{
    UINT32      returnCode = GOOD;
    UINT32      mrpReturnCode = PI_GOOD;
    MRSTARTIO_RSP *outPktPtr = NULL;
    MRSTARTIO_REQ *inPktPtr = NULL;
    UINT32      electionStopIOCountBackup = electionStopIOCount;

    dprintf(DPRINTF_ELECTION, "STARTIO: Starting I/O\n");

    /* Make sure that the election code has an outstanding stop I/O request */
    if (electionStopIOCount > 0)
    {
        /*
         * Set this to zero so that it's changed before the possibility
         * of a kernel exchange.  If the MRP fails, we'll need to set it
         * back to where it was.  We set it to zero instead of just decrementing
         * it because we're going to clear all outstanding stops generated
         * by the election code.
         */
        electionStopIOCount = 0;

        /* Allocate memory space for the MRP packet */
        inPktPtr = MallocWC(sizeof(*inPktPtr));
        outPktPtr = MallocSharedWC(sizeof(*outPktPtr));

        /* Set the MRP bits to clear one stop I/O request */
        inPktPtr->option = START_IO_OPTION_CLEAR_ALL_STOPS;
        inPktPtr->user = START_STOP_IO_USER_CCB_ELECTION;

        /* Execute the MRP */
        mrpReturnCode = PI_ExecMRP(inPktPtr, sizeof(*inPktPtr), MRSTARTIO,
                                   outPktPtr, sizeof(*outPktPtr), ELECTION_START_IO_TIMEOUT);

        switch (mrpReturnCode)
        {
            case PI_TIMEOUT:
                dprintf(DPRINTF_ELECTION, "STARTIO: StartIO timed out (non-fatal)\n");
                /* FALL THROUGH TO THE PI_GOOD HANDLING SINCE TIMEOUT IS NON-FATAL. */

            case PI_GOOD:
                /*
                 * GOOD or TIMEOUT, either case nothing to do since the
                 * returnCode is initialized to GOOD.
                 */
                break;

            case PI_ERROR:
                /*
                 * The MRP completed, but with ERROR status, but since it
                 * completed we need to free the status packet.  The start I/O
                 * MRP can return a 'stop count already at zero' condition,
                 * which means that the start I/O was received, but the stop
                 * count for this owner is already at zero (no stops outstanding).
                 * If we get this status code, treat it as a good condition, but
                 * put out a debug flag to identify this condition.
                 */
                if (outPktPtr->header.status == DESTOPZERO)
                {
                    dprintf(DPRINTF_ELECTION, "STARTIO: Start count already at zero\n");
                }
                else
                {
                    returnCode = ERROR;
                }
                break;

            default:
                returnCode = ERROR;
                break;
        }

        /* Free the allocated memory. */
        Free(inPktPtr);

        if (mrpReturnCode != PI_TIMEOUT)
        {
            Free(outPktPtr);
        }

        /*
         * Since we pre-decremented the stop counter before the MRP was
         * issued, increment it back to were it was on an ERROR.
         */
        if (returnCode == ERROR)
        {
            electionStopIOCount = electionStopIOCountBackup;
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "STARTIO: electionStopIOCount is <= zero (skipped)\n");
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_StopIO
**
**  Inputs:     None
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static UINT32 EL_StopIO(void)
{
    UINT32      returnCode = GOOD;
    UINT32      mrpReturnCode = PI_GOOD;
    MRSTOPIO_RSP *outPktPtr = NULL;
    MRSTOPIO_REQ *inPktPtr = NULL;

    dprintf(DPRINTF_ELECTION, "STOPIO: Stopping I/O (without cache flush)\n");

    if (electionStopIOCount == 0)
    {
        /*
         * Increment this now so that it's incremented before the possibility
         * of a kernel exchange.  If the MRP fails, we'll need to decrement it
         * back to where it was.
         */
        electionStopIOCount++;

        /* Allocate memory space for the MRP packet */
        inPktPtr = MallocWC(sizeof(*inPktPtr));
        outPktPtr = MallocSharedWC(sizeof(*outPktPtr));

        /* Set the MRP bits to stop all I/O, but not wait for Host I/O to complete */
        inPktPtr->operation = STOP_NO_WAIT_FOR_HOST;
        inPktPtr->intent = STOP_NO_SHUTDOWN;
        inPktPtr->user = START_STOP_IO_USER_CCB_ELECTION;

        /* Execute the MRP */
        mrpReturnCode = PI_ExecMRP(inPktPtr, sizeof(*inPktPtr), MRSTOPIO,
                                   outPktPtr, sizeof(*outPktPtr), ELECTION_STOP_IO_TIMEOUT);

        switch (mrpReturnCode)
        {
            case PI_TIMEOUT:
                dprintf(DPRINTF_ELECTION, "STOPIO: StopIO timed out (non-fatal)\n");
                /* FALL THROUGH TO THE PI_GOOD HANDLING SINCE TIMEOUT IS NON-FATAL. */

            case PI_GOOD:
                /*
                 * GOOD or TIMEOUT, either case nothing to do since the
                 * returnCode is initialized to GOOD.
                 */
                break;

            case PI_ERROR:
                /*
                 * The MRP completed, but with ERROR status, but since it
                 * completed we need to free the status packet.  The stop I/O
                 * MRP can return an 'outstanding ops' condition, which means
                 * that the stop I/O was received, but some operation could not
                 * be stopped since they were already in progress.  If we get this
                 * status code, treat it as a good condition.
                 */
                if (outPktPtr->header.status == DEOUTOPS)
                {
                    dprintf(DPRINTF_ELECTION, "STOPIO: IO stopped, but with outstanding ops\n");
                }
                else
                {
                    returnCode = ERROR;
                }
                break;

            default:
                returnCode = ERROR;
                break;
        }

        /* Free the allocated memory. */
        Free(inPktPtr);

        if (mrpReturnCode != PI_TIMEOUT)
        {
            Free(outPktPtr);
        }

        /*
         * Since we pre-incremented the stop counter before the MRP was
         * issued, decrement it back to were it was on an ERROR.
         */
        if (returnCode == ERROR)
        {
            electionStopIOCount--;
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "STOPIO: electionStopIOCount is non-zero (skipped)\n");
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_GetFailureState
**
**  Comments:      This should be valid at the end of an election, so RM
**                 should be able to use it at that time.
**
**  Parameters:    ctlrIdx - index of the controller in the comm area.
**
**  Returns:       FAILURE_DATA_STATE failureState
**--------------------------------------------------------------------------*/
FAILURE_DATA_STATE EL_GetFailureState(UINT8 ctlrIdx)
{
    FAILURE_DATA_STATE returnState = FD_STATE_UNUSED;

    if (ctlrIdx < dimension_of(gElectCommAreas))
    {
        returnState = gElectCommAreas[ctlrIdx].failStateSector.failureData.state;
    }

    return (returnState);
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_CheckKeepAliveConnectivity
**
**  Inputs:         myControllerCommArea
**
**  Returns:        GOOD  - Connected to a keepAlive controller
**                  ERROR - Not connected to a keepAlive controller
**--------------------------------------------------------------------------*/
UINT32 EL_CheckKeepAliveConnectivity(void)
{
    UINT8       returnCode = ERROR;
    UINT8       slotCounter = 0;
    char        tempString[TEMP_STRING_LENGTH] = { 0 };

    dprintf(DPRINTF_ELECTION, "CKAC: Checking for keepAlive connectivity\n");

    /* Clear the election data contactMap array before entering the election */
    for (slotCounter = 0; slotCounter < MAX_CONTROLLERS; slotCounter++)
    {
        if (EL_KeepAliveTestSlot(slotCounter) == GOOD)
        {
            /* Check to see if we have pdisk overlap with the keepAlive */
            if (EL_DiskMapSlotOverlap(slotCounter) > 0)
            {
                dprintf(DPRINTF_ELECTION, "CKAC: Overlaps with keepAlive (Slot %d)\n",
                        slotCounter);

                /*
                 * Look to see how this controller has contact with the keepAlive.
                 * If the ethernet or fibre link is good, or if the link hasn't
                 * been tested (NO_ACTIVITY) then call it connected.
                 * NOTE: Quorum connectivity doesn't qualify.
                 */
                switch (myControllerCommArea.electionStateSector.electionData.contactMap[slotCounter])
                {
                    case ED_CONTACT_MAP_CONTACTED_ETHERNET:
                    case ED_CONTACT_MAP_CONTACTED_FIBRE:
                    case ED_CONTACT_MAP_NOTIFY_SLAVE:
                    case ED_CONTACT_MAP_SLAVE_NOTIFIED:
                    case ED_CONTACT_MAP_CONTROLLER_TIMED_OUT:
                    case ED_CONTACT_MAP_NO_ACTIVITY:
                        returnCode = GOOD;
                        break;

                    default:
                        EL_GetContactMapStateString(tempString,
                                                    myControllerCommArea.electionStateSector.electionData.contactMap[slotCounter],
                                                    sizeof(tempString));

                        dprintf(DPRINTF_ELECTION, "CKAC: Poor connectivity with keepAlive slot %d (%s)\n",
                                slotCounter, tempString);
                        break;
                }
            }
        }
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_GetFEPortCount
**
**  Inputs:         countPtr - pointer where count is to be returned
**
**  Returns:        GOOD  - countPtr was based upon data returned from FE
**                  ERROR - problem getting data from FE (*countPtr is ZERO)
**--------------------------------------------------------------------------*/
static UINT32 EL_GetFEPortCount(UINT32 *countPtr)
{
    UINT32      returnCode = GOOD;
    PI_PORT_LIST_RSP *portListPtr = NULL;

    ccb_assert(countPtr != NULL, countPtr);

    if (countPtr != NULL)
    {
        /* Get the port list from the FE */
        portListPtr = PortList(GetMyControllerSN(), PROCESS_FE, PORTS_ONLINE);

        if (portListPtr != NULL)
        {
            *countPtr = portListPtr->count;

            /* Free the portList data, as required by the PortList function */
            Free(portListPtr);
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "GFEPC: PortList returned NULL\n");

            *countPtr = 0;
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "GFEPC: countPtr is NULL\n");
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_WaitForInactivateComplete
**
**  Description:    This function is called by an inactive controller that
**                  was responsible for starting an election.  In order to
**                  pick up the updated master configuration information, we
**                  need to wait until the election completes and reread it.
**
**  Inputs:         none
**
**  Modifies:       Nothing
**
**  Returns:        GOOD  - Inactive election is complete
**                  ERROR - problem while waiting for election to complete
**--------------------------------------------------------------------------*/
static UINT32 EL_WaitForInactivateComplete(void)
{
    QM_MASTER_CONFIG *tempMasterConfig;
    UINT32      startingElectionSerial = Qm_GetElectionSerial();
    UINT32      timeoutCounter = CHECK_FOR_INACTIVATE_TIMEOUT_GRANULARITY;
    UINT32      returnCode = ERROR;

    dprintf(DPRINTF_ELECTION, "WFIC: Waiting for inactivate election to complete\n");

    /* Check to see if this election involves more than one controller */
    if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) > 1)
    {
        /* Wait for the masterConfig's electionSerial to change */
        tempMasterConfig = MallocSharedWC(sizeof(*tempMasterConfig));
        while ((ReadMasterConfiguration(tempMasterConfig) == 0) && (returnCode != GOOD) && (timeoutCounter > 0))
        {
            if (tempMasterConfig->electionSerial != startingElectionSerial)
            {
                returnCode = GOOD;
            }
            else
            {
                timeoutCounter--;

                if (timeoutCounter > 0)
                {
                    dprintf(DPRINTF_ELECTION, "WFIC: Inactivate election not yet complete\n");
                    TaskSleepMS(WAIT_FOR_INACTIVATE_TIMEOUT / CHECK_FOR_INACTIVATE_TIMEOUT_GRANULARITY);
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "WFIC: Timeout waiting inactivate election to complete\n");
                }
            }
        }
        Free(tempMasterConfig);

        /*
         * Once the election completes, load the masterConfig into the global
         * data structure.
         */
        if (returnCode == GOOD)
        {
            if (LoadMasterConfiguration() == 0)
            {
                dprintf(DPRINTF_ELECTION, "WFIC: New masterConfig loaded\n");
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "WFIC: Could not load the final masterConfig\n");
                returnCode = ERROR;
            }
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "WFIC: Single controller deactivating - don't wait\n");
        returnCode = GOOD;
    }

    /* Display an informational message */
    if (returnCode == GOOD)
    {
        dprintf(DPRINTF_ELECTION, "WFIC: Inactivate election complete\n");
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "WFIC: Error while waiting for inactivate election\n");
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_DurationLog
**
**  Description:    Generates a debug log message containing the time, in
**                  seconds, it took to perform the election.
**
**  Inputs:         logType - EL_DURATION_LOG (START, RESTART, FINISH)
**
**  Modifies:       Nothing
**
**  Returns:        GOOD  -
**                  ERROR - problem while waiting for election to complete
**--------------------------------------------------------------------------*/
static UINT32 EL_DurationLog(EL_DURATION_LOG logType)
{
    static UINT32 electionStartSeconds = 0;
    UINT32      returnCode = GOOD;
    UINT32      electionFinishSeconds = RTC_GetSystemSeconds();
    UINT32      durationSeconds = 0;

    /* Protect against powerup case where RTC values aren't yet initialized */
    if (electionFinishSeconds > electionStartSeconds)
    {
        durationSeconds = electionFinishSeconds - electionStartSeconds;
    }
    else
    {
        durationSeconds = electionStartSeconds - electionFinishSeconds;
    }

    /* Handle the log type and generate the appropriate messages */
    switch (logType)
    {
        case EL_DURATION_LOG_START:
            if (electionStartSeconds == 0)
            {
                electionStartSeconds = RTC_GetSystemSeconds();
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "DL: Start ERROR - startSeconds non-zero\n");
                returnCode = ERROR;
            }
            break;

        case EL_DURATION_LOG_RESTART:
            if (electionStartSeconds > 0)
            {
                /* Make log message */
                LogMessage(LOG_TYPE_DEBUG, "ELECTION-Restart (ET=%d sec)",
                           durationSeconds);
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "DL: Restart ERROR - startSeconds invalid\n");
                returnCode = ERROR;
            }

            /* Reset the starting seconds to current time */
            electionStartSeconds = RTC_GetSystemSeconds();
            break;

        case EL_DURATION_LOG_FINISH:
            if (electionStartSeconds > 0)
            {
                /* Make log message */
                LogMessage(LOG_TYPE_DEBUG, "ELECTION-Finished (ET=%d sec)",
                           durationSeconds);

                /*
                 * Set electionStartSeconds to indicate that no duration
                 * is being tracked.
                 */
                electionStartSeconds = 0;
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "DL: Finish ERROR - startSeconds invalid\n");
                returnCode = ERROR;
            }
            break;

        default:
            dprintf(DPRINTF_ELECTION, "DL: Unknown duration event type: %d\n", logType);
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
