/* $Id: rm_realloc.c 160802 2013-03-22 16:39:33Z marshall_midden $ */
/*===========================================================================
** FILE NAME:       rm_realloc.c
** MODULE TITLE:    Resource Manager Reallocation Task
**
** DESCRIPTION:     This file contains the RM Reallocation Task and
**                  associated functions.
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/

#include "AsyncEventHandler.h"
#include "CacheSize.h"
#include "CmdLayers.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "fm.h"
#include "ipc_cmd_dispatcher.h"
#include "logdef.h"
#include "logging.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "pcb.h"
#include "PI_CmdHandlers.h"
#include "PI_Server.h"
#include "PI_Target.h"
#include "PI_Utils.h"
#include "PI_VDisk.h"
#include "PI_WCache.h"
#include "PktCmdHdl.h"
#include "PortServer.h"
#include "PR.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rm.h"
#include "RMCmdHdl.h"
#include "rm_val.h"
#include "rtc.h"
#include "serial_num.h"
#include "sm.h"
#include "trace.h"
#include "X1_AsyncEventHandler.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

/** Resource Manager Port List Information                                  */
typedef struct RM_PORT_LIST
{
    UINT32      ctrl;           /* Controller                               */
    UINT32      mirrorPartner;  /* Current mirror partner                   */
    UINT32      srcMirrorPartner;       /* Source mirror partner                    */
    UINT32      newMirrorPartner;       /* New mirror partner                       */
    UINT32      newSrcMirrorPartner;    /* New Source mirror partner                */
    UINT16      validPorts;     /* Ports that exist                         */
    UINT16      goodPorts;      /* Good Ports                               */
    UINT16      onlinePorts;    /* Online Ports                             */
    UINT8       initializing;   /* Initialization in progress               */
    UINT8       active;         /* Controller is active                     */
    UINT8       numberTargets;  /* Number of targets assigned to controller */
    UINT8       failBack;       /* Fail-back to this Controller             */
    UINT8       rsvd;
    UINT8       validEntry;     /* Entry is valid                           */
} RM_PORT_LIST;

/** Resource Manager Failure List Information                               */
typedef struct RM_FAILURE_LIST
{
    UINT16      ports;              /**< Ports involved in this failure     */
    UINT8       failBack;           /**< Flag for failover or failback      */
    UINT8       fromActive;         /**< Flag for active state of FROM      */
    UINT32      from;               /**< FROM controller serial number      */
    RM_PORT_LIST *pFromPort;        /**< Port list for FROM controller      */
    UINT32      to;                 /**< TO controller serial number        */
} RM_FAILURE_LIST;

/** Resource Manager Reset List Information                                 */
typedef struct RM_RESET_LIST
{
    UINT32      controllerSN;       /**< Controller serial number for reset */
    UINT8       rsvd4[2];           /**< RESERVED                           */
    UINT8       port;               /**< Ports involved in this reset       */
    UINT8       option;             /**< Option for how to reset            */
} RM_RESET_LIST;

/** Resource Manager Raid Resync Information                                */
typedef struct RM_RAID_RESYNC_LIST
{
    UINT32      controllerSN;       /**< Controller serial number for resync*/
    MR_LIST_RSP *pList;             /**< Raid List                          */
} RM_RAID_RESYNC_LIST;

/** Resource Manager Change Mirror Partner Counter Start Value (15 seconds) */
#define RM_CHANGE_MP_COUNTER    15

/** REALLOC Defrag start delay value (60 seconds)                           */
#define RM_DEFRAG_DELAY         60

/*****************************************************************************
** Private variables
*****************************************************************************/
static PCB *pRMReallocTask = NULL;      /* RM resource reallocation task PCB */
static UINT32 rmConfigChanged;

static PCB *gpChangeMirrorPartnerTask = NULL;
static UINT8 gChangeMirrorPartnerTaskFlag = false;
static UINT8 gChangeMirrorPartnerTaskCounter;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
extern volatile UINT32 RMShutdownRequested;
extern UINT8 gAllDevMissAtOtherDCN;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static RM_PORT_LIST *RM_Ports(void);
static UINT32 RM_GoodPortCheck(RM_PORT_LIST *portList, UINT32 owner,
                               UINT32 bitPorts, UINT32 requireOnline);
static RM_PORT_LIST *RM_FindEntry(RM_PORT_LIST *portList, UINT32 controllerSN);
static UINT32 RM_ActiveCtrlCheck(RM_PORT_LIST *portList, UINT32 owner);
static UINT32 RM_AddFailureAction(RM_FAILURE_LIST *failureList, UINT32 from,
                                  RM_PORT_LIST *pFromPort, UINT32 to,
                                  UINT32 bitPorts, UINT8 failBack);
static UINT32 RM_CheckLinkedTargets(RM_PORT_LIST *portList, PI_TARGETS_RSP *targetList,
                                    MRGETTARG_RSP *targetInfo, UINT32 newOwner);
static UINT32 RM_FindNextActive(RM_PORT_LIST *portList, UINT32 controllerSN);
static UINT32 RM_GetMirrorPartners(RM_PORT_LIST *portList);
static RM_PORT_LIST *RM_FindControllerWithTarget(PARALLEL_REQUEST *pTargetRequests,
                                                 RM_PORT_LIST *portList,
                                                 UINT32 owner, UINT16 tid);
static void RM_CheckControllers(void);
static bool RM_VerifyCheckControllerData(RM_PORT_LIST *pPortList,
                                         PARALLEL_REQUEST *pPR_Targets,
                                         PARALLEL_REQUEST *pPR_MPInfo);
static void RM_FillControllerTargetCounts(PI_TARGETS_RSP *targetList,
                                          RM_PORT_LIST *portList,
                                          PARALLEL_REQUEST *pPR_Targets);
static RM_FAILURE_LIST *RM_BuildFailureList(PI_TARGETS_RSP *targetList,
                                            RM_PORT_LIST *portList,
                                            PARALLEL_REQUEST *pPR_Targets);
static RM_RAID_RESYNC_LIST *RM_BuildRaidResyncList(RM_FAILURE_LIST *pFailureList,
                                                   PI_TARGETS_RSP *pTargets);
static void RM_ProcessFailures(RM_FAILURE_LIST *failureList, RM_PORT_LIST *portList,
                               RM_RAID_RESYNC_LIST *pResyncList,
                               PARALLEL_REQUEST *pPR_MPInfo);
static void RM_ProcessFailure_FlushWOMP(RM_FAILURE_LIST *pFailureList,
                                        RM_PORT_LIST *pPortList);
static INT32 RM_FailControllers(RM_FAILURE_LIST *pFailureList);
static INT32 RM_FailController(UINT32 oldOwner, UINT32 newOwner, UINT32 failBack,
                               UINT32 ports);
static void RM_AddToResetList(RM_RESET_LIST *pResetList, UINT32 controllerSN, UINT8 port,
                              UINT8 option);
static INT32 RM_ResetInterfaces(RM_FAILURE_LIST *pFailureList, RM_PORT_LIST *pPortList);
static void RM_ProcessFailure_FlushBEWC(RM_FAILURE_LIST *pFailureList,
                                        RM_PORT_LIST *pPortList);
static void RM_ResyncControllers(RM_FAILURE_LIST *pFailureList, RM_PORT_LIST *pPortList,
                                 RM_RAID_RESYNC_LIST *pResyncList);
static void RM_RaidResyncController(UINT32 controllerSN, UINT32 raidOwnerSN,
                                    RM_RAID_RESYNC_LIST *pResyncList);
static PARALLEL_REQUEST *RM_BuildMirrorPartnerChanges(RM_FAILURE_LIST *pFailureList,
                                                      RM_PORT_LIST *pPortList,
                                                      PARALLEL_REQUEST *pPR_MPInfo);
static bool RM_QueryMirrorPartnerChanges(PARALLEL_REQUEST *pRequests,
                                         UINT8 mpResponseMask);
static void RM_ChangeMirrorPartners(RM_FAILURE_LIST *pFailureList,
                                    RM_PORT_LIST *pPortList,
                                    PARALLEL_REQUEST *pPR_MPInfo);
static void RM_CheckOnlinePorts(RM_PORT_LIST *portList);
static void RM_RepairMirrorPartners(RM_PORT_LIST *portList, PARALLEL_REQUEST *pPR_MPInfo);
static UINT32 RM_FindMirrorPartner(RM_PORT_LIST *portList, UINT32 controllerSN);
static MP_MIRROR_PARTNER_INFO *RM_FindMPInfo(PARALLEL_REQUEST *pPR_MPInfo,
                                             UINT32 controllerSN);
static PI_TARGETS_RSP *RM_FindTargets(PARALLEL_REQUEST *pTargetRequests,
                                      UINT32 controllerSN);
static PI_TARGET_INFO_RSP *RM_FindTarget(PARALLEL_REQUEST *pTargetRequests,
                                         UINT32 controllerSN, UINT16 tid);
static void RM_DumpPortList(RM_PORT_LIST *portList);
static void RM_DumpActions(RM_FAILURE_LIST *pFailureList);
static void RM_ReallocTask(TASK_PARMS *parms);
static void RM_ChangeMirrorPartnerTaskStart(void);
static void RM_ChangeMirrorPartnerTaskCancel(void);
static void RM_ChangeMirrorPartnerTask(TASK_PARMS *parms);
static void RM_ProcessFailure_InvalidateWC(RM_FAILURE_LIST *pFailureList,
                                           UNUSED RM_PORT_LIST *pPortList);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*****************************************************************************
**  FUNCTION NAME: RmStartReallocTask
**
**  PARAMETERS: None.
**
**  RETURNS:    None.
**
**  COMMENTS:
**      Starts the RM Reallocation task.
**      Note:  Realloc task acquires RM busy and holds it until all of
**             its actions are complete (or shutdown is requested).
**
******************************************************************************/
void RmStartReallocTask(void)
{
    if (pRMReallocTask == NULL)
    {
        /*
         * Start the Realloc task
         */
        pRMReallocTask = TaskCreate(RM_ReallocTask, NULL);
    }
    else
    {
        /*
         * Signal to the running Realloc Task that another
         * configuration change occured and must be handled.
         */
        rmConfigChanged = TRUE;
        dprintf(DPRINTF_RM, "RmStartReallocTask: Realloc task already running!\n");
    }
}


/*****************************************************************************
**  FUNCTION NAME: RM_Ports
**
**  PARAMETERS: None.
**
**  RETURNS:    None.
**
**  COMMENTS:   This function gets the list of good port for all active
**              controllers.  This if peformed by sending a PortList MRP
**              to all active controllers.
**
**              Warning: caller must free memory pointed
**                       to by the return value
**
******************************************************************************/
static RM_PORT_LIST *RM_Ports(void)
{
    UINT32      index1;
    UINT32      j;
    UINT32      count = 0;
    UINT32      Configindex = ACM_NODE_UNDEFINED;
    UINT32      ControllerSSN;
    RM_PORT_LIST *portList = NULL;
    PI_PORT_LIST_RSP *ptrList = NULL;
    UINT32      size;
    PR_PORT_LIST_PARAM param;
    PARALLEL_REQUEST *pRequests;

    if (Qm_GetNumControllersAllowed() <= 0)
    {
        return NULL;
    }

    /*
     * Initialize the parallel configuration update template.
     */
    memset(&param, 0x00, sizeof(param));
    param.processor = PROCESS_FE;
    param.type = PORTS_STATUS;

    /*
     * Allocate the parallel request buffers.
     */
    pRequests = PR_AllocTemplate(0, &param, sizeof(param));

    /*
     * Send the PORTS parallel requests.
     */
    PR_SendRequests(PR_DEST_ACTIVE, PR_TYPE_PORTS, pRequests, PR_SendTaskPorts);

    /*
     * Loop through the port request data and see if anything that should
     * be available is not.  Basically, if the request structure has a
     * controller serial number it should have port information.  If it
     * doesn't, it is not valid and is an error.
     */
    for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
    {
        /*
         * Does this request have a controller serial number but no
         * port data?
         */
        if (pRequests[index1].controllerSN != 0 && pRequests[index1].pData == NULL)
        {
            LogMessage(LOG_TYPE_DEBUG, "REALLOC-Missing port information (0x%x)",
                       pRequests[index1].controllerSN);

            /*
             * Free the parallel requests, this will NULL out the
             * pRequests pointer and force us to skip the next
             * section which interrogates the request information.
             */
            PR_Release(&pRequests);
            break;
        }
    }

    /*
     * If we have port request information, loop through it and fill
     * out the port list used for the return data.
     */
    if (pRequests)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Valid port information");

        /*
         * Allocate space for the port list.
         */
        size = sizeof(*portList) * (Qm_GetNumControllersAllowed() + 1);
        portList = MallocWC(size);

        /*
         * For each active controller, get a list of good ports.
         */
        for (index1 = 0; index1 < Qm_GetNumControllersAllowed(); ++index1)
        {
            /*
             * Get the controller serial number for this entry
             * in the controller configuration map.
             */
            ControllerSSN = CCM_ControllerSN(index1);

            /*
             * Does a controller exist at this slot?
             */
            if (ControllerSSN == 0)
            {
                /*
                 * Continue to the next controller index.
                 */
                continue;
            }

            /*
             * Fill in the controller serial number.
             */
            portList[count].ctrl = ControllerSSN;
            portList[count].mirrorPartner = 0;
            portList[count].srcMirrorPartner = 0;
            portList[count].newMirrorPartner = 0;
            portList[count].newSrcMirrorPartner = 0;

            /*
             * Check if the controller is active.  Check
             * each entry in the acitve controller map
             * and look for this index.
             */
            for (j = 0; j < Qm_GetNumControllersAllowed(); ++j)
            {
                /*
                 * Get the index for this entry in the
                 * acitve controller map.
                 */
                Configindex = Qm_GetActiveCntlMap(j);

                /*
                 * IS this the right index?
                 */
                if (Configindex == index1)
                {
                    /*
                     * This is the right index.  Exit the loop.
                     */
                    break;
                }
                else
                {
                    /*
                     * This is not the right index.
                     */
                    Configindex = ACM_NODE_UNDEFINED;
                }
            }

            /*
             * Check for valid controller index.
             */
            if (Configindex != ACM_NODE_UNDEFINED)
            {
                for (j = 0; j < MAX_CONTROLLERS; ++j)
                {
                    if (pRequests[j].controllerSN == ControllerSSN)
                    {
                        ptrList = (PI_PORT_LIST_RSP *)pRequests[j].pData;
                        break;
                    }
                }

                /*
                 * Indicate this as an active controller.
                 */
                portList[count].active = TRUE;
            }
            else
            {
                /*
                 * Indicate this as an inactive (failed) controller.
                 */
                portList[count].active = FALSE;

                /*
                 * Don't get the port list for failed controllers.
                 */
                ptrList = NULL;
            }

            /*
             * Do we have a port list to process?
             */
            if (ptrList != NULL)
            {
                /*
                 * Set a bit for each port in the list.
                 */
                portList[count].validPorts = ptrList->list[0];
                portList[count].goodPorts = ptrList->list[1];
                portList[count].onlinePorts = ptrList->list[2];
                portList[count].initializing = (UINT8)ptrList->list[3];
            }
            else
            {
                /*
                 * No good ports exists on a failed controller.
                 */
            }

            /*
             * Mark this entry as a valid entry.
             */
            portList[count].validEntry = TRUE;

            /*
             * Increment the count of controllers.
             */
            ++count;
        }

        /*
         * Free the parallel requests
         */
        PR_Release(&pRequests);
    }

    return portList;
}


/*****************************************************************************
**  FUNCTION NAME: RM_GoodPortCheck
**
**  PARAMETERS: Pointer to a list of good ports.
**              Controller to check.
**              Bitmask of ports to check.
**
**  RETURNS:    FALSE - no good ports exists.
**              TRUE - A good port exist on the specified controller
**
**  COMMENTS:   This checks that good ports exist on the specified
**              controller.
**
******************************************************************************/
static UINT32 RM_GoodPortCheck(RM_PORT_LIST *portList, UINT32 owner,
                               UINT32 bitPorts, UINT32 requireOnline)
{
    UINT32      retValue = FALSE;

    /*
     * Examine each entry in the List
     */
    while (portList->validEntry == TRUE)
    {
        /*
         * Check for matching controller serial number.
         */
        if (portList->ctrl == owner)
        {
            if (requireOnline == FALSE)
            {
                /*
                 * Check for good ports.
                 */
                if ((bitPorts & portList->goodPorts) != 0)
                {
                    /*
                     * A good port exists on the desired controller.
                     */
                    retValue = TRUE;
                }
            }
            else
            {
                /*
                 * Check for ports that good and online.
                 */
                if ((bitPorts & portList->goodPorts & portList->onlinePorts) != 0)
                {
                    /*
                     * A good port exists on the desired controller.
                     */
                    retValue = TRUE;
                }
            }

            /*
             * Found desired controller, exit loop.
             */
            break;
        }

        /*
         * Advance to next entry.
         */
        ++portList;
    }

    /*
     * Return value of TRUE means the specified target has a good
     * port available on the specified controller.
     */
    return retValue;
}


/**
******************************************************************************
**
**  @brief      Finds the port list for the specified controller.
**
**  @param      portList - controller
**  @param      controllerSN - controller
**
**  @return     port list pointer.
**
******************************************************************************
**/
static RM_PORT_LIST *RM_FindEntry(RM_PORT_LIST *portList, UINT32 controllerSN)
{
    /*
     * Examine each entry in the MRP
     */
    while (portList->validEntry == TRUE)
    {
        /*
         * Check for matching controller serial number.
         */
        if (portList->ctrl == controllerSN)
        {
            /*
             * Found desired controller, exit loop.
             */
            break;
        }

        /*
         * Advance to next entry.
         */
        ++portList;
    }

    if (portList->validEntry == FALSE)
    {
        portList = NULL;
    }

    return portList;
}


/*****************************************************************************
**  FUNCTION NAME: RM_ActiveCtrlCheck
**
**  PARAMETERS: Pointer to a list of good ports.
**              Controller to check.
**
**  RETURNS:    None.
**
**  COMMENTS:   Checks if the specified controller is in the port list
**              and the controller is active.
**
**
******************************************************************************/
static UINT32 RM_ActiveCtrlCheck(RM_PORT_LIST *portList, UINT32 owner)
{
    UINT32      retValue = FALSE;

    /*
     * Examine each entry in the MRP
     */
    while (portList->validEntry == TRUE)
    {
        /*
         * Check for matching controller serial number.
         */
        if (portList->ctrl == owner)
        {
            /*
             * Check that the controller is active.
             */
            if (portList->active != FALSE)
            {
                /*
                 * Indicate Good controller.
                 */
                retValue = TRUE;
            }

            /*
             * Found desired controller, exit loop.
             */
            break;
        }

        /*
         * Advance to next entry.
         */
        ++portList;
    }

    return retValue;
}


/*****************************************************************************
**  FUNCTION NAME: RM_AddFailureAction
**
**  PARAMETERS: Pointer to the failure list.
**              Port to move from.
**              Port to move to.
**              Bitmask of ports affected.
**              Fail-back indicator.
**
**  RETURNS:    None.
**
******************************************************************************/
static UINT32 RM_AddFailureAction(RM_FAILURE_LIST *failureList, UINT32 from,
                                  RM_PORT_LIST *pFromPort, UINT32 to,
                                  UINT32 bitPorts, UINT8 failBack)
{
    UINT32      count = 0;
    UINT32      found = FALSE;

    if (pFromPort == NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-From port is NULL");
    }
    else if (pFromPort->ctrl != from)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-From port (0x%x), From controller (0x%x)",
                   pFromPort->ctrl, from);
    }

    /*
     * Check if this entry already exists.
     */
    while (failureList[count].to != 0)
    {
        /*
         * Check all fields for a match.
         */
        if (failureList[count].from == from &&
            failureList[count].to == to && failureList[count].failBack == failBack)
        {
            /*
             * A matching record is found.
             */
            found = TRUE;
            break;
        }

        /*
         * Advance to next entry.
         */
        ++count;
    }

    /*
     * Was an existing entry found?
     */
    if (found == FALSE)
    {
        /*
         * Fill in the entry.
         */
        if (pFromPort != NULL)
        {
            failureList[count].fromActive = pFromPort->active;
        }
        else
        {
            failureList[count].fromActive = FALSE;
        }

        failureList[count].from = from;
        failureList[count].pFromPort = pFromPort;
        failureList[count].to = to;
        failureList[count].failBack = failBack;
    }

    /*
     * Indicate the ports affected.
     */
    failureList[count].ports |= (UINT16)bitPorts;

    /*
     * Return the entry number used.
     */
    return count;
}


/*****************************************************************************
**  FUNCTION NAME: RM_CheckLinkedTargets
**
**  PARAMETERS: Pointer to a list of targets.
**              target ID to check if other targets are linked to.
**              Controller owning specified target.
**              Pointer to a list of good ports.
**
**  RETURNS:    None.
**
******************************************************************************/
static UINT32 RM_CheckLinkedTargets(RM_PORT_LIST *portList, PI_TARGETS_RSP *targetList,
                                    MRGETTARG_RSP *targetInfo, UINT32 newOwner)
{
    UINT32      index1;
    MRGETTARG_RSP *linkedTargetInfo;
    UINT32      bitPorts;
    UINT32      retValue = TRUE;
    UINT32      requireOnline;

    /*
     * If moving target to the preferred owner, require the ports
     * to be online in addition to being good.
     */
    requireOnline = (newOwner == targetInfo->powner);

    /*
     * Check if this target is linked to another target.
     */
    if (targetInfo->cluster < MAX_TARGETS)
    {
        /*
         * Get the target information for the clustered target.
         */
        linkedTargetInfo = &targetList->targetInfo[targetInfo->cluster];

        /*
         * Indicate the ports where a least one port must be GOOD.
         */
        bitPorts = (1 << linkedTargetInfo->pport) | (1 << linkedTargetInfo->aport);

        dprintf(DPRINTF_RM, "RM_CheckLinkedTargets: target %u 0x%x ports 0x%x\n",
                linkedTargetInfo->tid, newOwner, bitPorts);

        /*
         * If a good port exists on the specifed controller, this
         * target can be moved to the specifed controller.
         */
        if (RM_GoodPortCheck(portList, newOwner, bitPorts, requireOnline) == FALSE)
        {
            /*
             * No good ports exist on the controller we want to move
             * the targets to.  Check if any Good ports exist on the
             * controller the targets currently reside.
             */
            if (RM_GoodPortCheck(portList,
                                 linkedTargetInfo->owner,
                                 bitPorts, requireOnline) == TRUE)
            {
                retValue = FALSE;
            }
        }
    }
    else
    {
        /*
         * Examine all the targets.
         */
        for (index1 = 0; index1 < targetList->count; ++index1)
        {
            /*
             * Get the target information.
             */
            linkedTargetInfo = &targetList->targetInfo[index1];

            /*
             * Check if this target is clustered to the specified target
             */
            if (linkedTargetInfo->cluster == targetInfo->tid)
            {
                /*
                 * Indicate the ports where a least one port must be GOOD.
                 */
                bitPorts = (1 << linkedTargetInfo->pport) |
                    (1 << linkedTargetInfo->aport);

                dprintf(DPRINTF_RM, "RM_CheckLinkedTargets: target %u 0x%x ports 0x%x\n",
                        linkedTargetInfo->tid, newOwner, bitPorts);

                /*
                 * If a good port exists on the specifed controller, this
                 * target can be moved back to the specifed controller.
                 */
                if (RM_GoodPortCheck(portList,
                                     newOwner, bitPorts, requireOnline) == FALSE)
                {
                    /*
                     * No good ports exist on the controller we want to move
                     * the targets to.  Check if any Good ports exist on the
                     * controller the targets currently reside.
                     */
                    if (RM_GoodPortCheck(portList,
                                         linkedTargetInfo->owner,
                                         bitPorts, requireOnline) == TRUE)
                    {
                        retValue = FALSE;
                        break;
                    }
                }
            }
        }
    }

    /*
     * Return value of TRUE means there are no linked targets, or all linked
     * targets have a good port available on the specified controller or
     * no good ports exist for the linked targets on the current controller.
     */
    return retValue;
}


/**
******************************************************************************
**
**  @brief      This function search the Active Controller map for the
**              specified controller.  The next active cotroller inA
**              in the Controller Configuration Map is returned.
**
**  @param      portList -  port list database
**              controllerSN - controller
**
**  @return     INT32 - return status
**
******************************************************************************
**/
static UINT32 RM_FindNextActive(RM_PORT_LIST *portList, UINT32 controllerSN)
{
    UINT32      index1;
    UINT32      j;
    UINT32      configIndex = ACM_NODE_UNDEFINED;
    UINT32      nextActiveController = 0;
    UINT32      validPorts;
    UINT32      goodPorts;
    RM_PORT_LIST *thisPort;

    /*
     * Get the list of valid ports for this controller.  The
     * active controller that is selected must have a good port
     * that corresponds to the valid ports on the specified controller.
     */
    thisPort = RM_FindEntry(portList, controllerSN);
    if (thisPort != NULL)
    {
        /*
         * Get the valid and good ports.
         */
        validPorts = thisPort->validPorts;
        goodPorts = thisPort->goodPorts;
    }
    else
    {
        /*
         * This should never happen, but let's require all ports
         * to be valid and good.
         */
        validPorts = 0xF;
        goodPorts = 0xF;
    }

    /*
     * Find the specified controller in the CCM.
     */
    for (index1 = 0; index1 < Qm_GetNumControllersAllowed(); ++index1)
    {
        /*
         * Get the controller serial number for this entry
         * in the controller configuration map.
         */
        if (CCM_ControllerSN(index1) == controllerSN)
        {
            /*
             * This is the right index.  Exit the loop.
             */
            configIndex = index1;
            break;
        }
    }

    /*
     * The configIndex is the index of the specified controller
     * in the CCM.  Check if this index is valid, i.e. the
     * controller is active.  If the controller is active,
     * a mirror partner need to be found.
     */
    if (configIndex != ACM_NODE_UNDEFINED)
    {
        /*
         * Find the next available controller in the ACM.
         */
        for (index1 = configIndex + 1; configIndex != index1;)
        {
            /*
             * Check if the controller is active.  Check
             * each entry in the active controller map
             * and look for this index.
             */
            for (j = 0; j < Qm_GetNumControllersAllowed(); ++j)
            {
                /*
                 * Check if this entry in the ACM is for the
                 * desired entry in the CCM.
                 */
                if (Qm_GetActiveCntlMap(j) == index1)
                {
                    /*
                     * Get the port entry for this controller.
                     */
                    thisPort = RM_FindEntry(portList, CCM_ControllerSN(index1));

                    /*
                     * Check if this controller is active and at least one
                     * good port exists for each set of valid ports.
                     *
                     * If the controller passed in (controllerSN) has
                     * a good port between 0 and 1 then the next active
                     * controller must have a good port between 0 an 1.
                     *
                     * The same holds true for ports between 2 and 3.
                     */
                    if (thisPort->active != FALSE &&
                        ((thisPort->goodPorts & 0x3) != 0 ||
                         (goodPorts & 0x3) == 0) &&
                        ((thisPort->goodPorts & 0xC) != 0 || (goodPorts & 0xC) == 0))
                    {
                        /*
                         * This is the right index.  Exit the loop.
                         */
                        nextActiveController = CCM_ControllerSN(index1);
                        break;
                    }
                }
            }

            /*
             * Was the next active controller found?
             */
            if (nextActiveController != 0)
            {
                /*
                 * Next active controller found, exit the loop.
                 */
                break;
            }

            /*
             * Wrap back to zero.
             */
            if (++index1 >= Qm_GetNumControllersAllowed())
            {
                index1 = 0;
            }
        }
    }

    dprintf(DPRINTF_RM, "RM_FindNextActive: 0x%x->0x%x\n",
            controllerSN, nextActiveController);

    return nextActiveController;
}

/**
******************************************************************************
**
**  @brief      This function search the Active Controller map for the
**              specified controller.  A mirror partner is selected for
**              this controller.  The mirror partner selected is the
**              next active controller in the Controller Configuration Map
**
**  @param      portList - port list database
**              controllerSN - controller
**
**  @return     INT32 - return status
**
******************************************************************************
**/
static UINT32 RM_FindMirrorPartner(RM_PORT_LIST *portList, UINT32 controllerSN)
{
    RM_PORT_LIST *thisPort = NULL;
    RM_PORT_LIST *mpPortList = NULL;
    UINT32      mirrorPartner;

    /*
     * Make sure the port list exists.
     */
    if (portList != NULL)
    {
        /*
         * Find the list of ports entry for this controller.
         */
        thisPort = RM_FindEntry(portList, controllerSN);
    }

    /*
     * If the desired entry was not found, this controller
     * has no mirror partner.
     */
    if (thisPort == NULL)
    {
        mirrorPartner = 0;
    }
    /*
     * Check if a new partner has already been selected for this controller.
     */
    else if (thisPort->newMirrorPartner != 0)
    {
        mirrorPartner = thisPort->newMirrorPartner;
    }
    else
    {
        /*
         * Get the current mirror partner for this controller.
         */
        mirrorPartner = thisPort->mirrorPartner;

        /*
         * Check if mirror partner does not exist or if the mirror
         * partner is set to self.  If so, a new mirror partner
         * is selected.  The mirror partner selected is the next
         * controller in the Active Controller Map.
         */
        if (mirrorPartner == 0 ||
            mirrorPartner == controllerSN || !RM_ActiveCtrlCheck(portList, mirrorPartner))
        {
            /*
             * Find the next active controller in the CCM.
             */
            mirrorPartner = RM_FindNextActive(portList, controllerSN);

            /*
             * Was a mirror partner found?
             *
             * If there was not a next active controller and
             * the current mirror partner for this controller
             * is not itself, set it to itself.
             */
            if (mirrorPartner != 0)
            {
                /*
                 * Update the mirror partner of the specifed controller.
                 */
                thisPort->newMirrorPartner = mirrorPartner;

                /*
                 * Update the controller whose is the mirror partner
                 * to indicate the new source mirror partner.
                 */
                mpPortList = RM_FindEntry(portList, mirrorPartner);

                if (mpPortList != NULL)
                {
                    mpPortList->newSrcMirrorPartner = controllerSN;

                    /*
                     * Check if the new mirror partner is already
                     * assigned as someone's mirror partner
                     */
                    if (mpPortList->srcMirrorPartner != 0)
                    {
                        /*
                         * The specified controller becomes the mirror
                         * partner of the controller who currently is the
                         * mirror partner of the controller selected
                         * as the mirror partner of the specified controller.
                         */
                        thisPort->newSrcMirrorPartner = mpPortList->srcMirrorPartner;

                        /*
                         * Update the controller whose mirror partner is
                         * changing.
                         */
                        thisPort = RM_FindEntry(portList, mpPortList->srcMirrorPartner);
                        if (thisPort != NULL)
                        {
                            thisPort->newMirrorPartner = controllerSN;
                        }

                        dprintf(DPRINTF_RM, "RM_FindMirrorPartner: 0x%x->0x%x->0x%x\n",
                                mpPortList->srcMirrorPartner, controllerSN, mirrorPartner);
                    }
                    else
                    {
                        dprintf(DPRINTF_RM, "RM_FindMirrorPartner: %u->%u\n",
                                controllerSN, mirrorPartner);
                    }
                }
            }
            else if (thisPort->mirrorPartner != controllerSN)
            {
                /*
                 * There is not a next active controller so make
                 * sure this controller is mirroring to itself.
                 */
                mirrorPartner = controllerSN;
                thisPort->newMirrorPartner = mirrorPartner;
            }
        }
    }

    return mirrorPartner;
}

/**
******************************************************************************
**
**  @brief      This function search initializes the port list with the
**              current values of the mirror partner.  Both the mirror
**              partner and the source mirror partner (the controller who
**              has the mirror partner set to this controller).
**
**  @param      portList - port list database
**
**  @return     INT32 - return status
**
******************************************************************************
**/
static UINT32 RM_GetMirrorPartners(RM_PORT_LIST *portList)
{
    INT32       rc = PI_GOOD;
    UINT16      indexMPList;
    PI_VCG_GET_MP_LIST_RSP *pMPList = NULL;

    /*
     * Get the mirror partner list
     */
    pMPList = SM_GetMirrorPartnerList();

    if (pMPList != NULL)
    {
        /*
         * Process each entry in the port list database.
         */
        while (portList->validEntry == TRUE)
        {
            /*
             * Examine each entry in the mirror partner list.
             */
            for (indexMPList = 0; indexMPList < pMPList->count; ++indexMPList)
            {
                /*
                 * Check for the controller whose mirror partner is
                 * this controller by comparing serial numbers.
                 */
                if (portList->ctrl == pMPList->list[indexMPList].source)
                {
                    portList->mirrorPartner = pMPList->list[indexMPList].dest;
                }

                /*
                 * Check for the controller who is the mirror partner of
                 * this controller by comparing serial numbers.
                 */
                if (portList->ctrl == pMPList->list[indexMPList].dest)
                {
                    /*
                     * If the source mirror partner for this controller
                     * has not yet been set (value of 0) or if it is
                     * currently set to itself, update the value with
                     * the new value, Otherwise leave it alone since
                     * it indicates someone else is mirroring to this
                     * controller.
                     */
                    if (portList->srcMirrorPartner == 0 ||
                        portList->srcMirrorPartner == portList->ctrl)
                    {
                        portList->srcMirrorPartner = pMPList->list[indexMPList].source;
                    }
                }
            }

            /*
             * Advance to next entry.
             */
            ++portList;
        }

        Free(pMPList);
    }
    else
    {
        rc = PI_ERROR;
    }

    return (rc);
}

/*****************************************************************************
**  @brief      This function checks to see if the specified target is
**              still on a port on the old owner.  This indicates the
**              reset to the qlogic adapter did not occur.
**
**              This function is called when the port number in
**              target information read from the current owner is invalid.
**              All other controller are searched for the one that owns
**              this controller.
**
**  @param      portList - port list database
**
**  @return     INT32 - old owner
**
**
******************************************************************************/
static RM_PORT_LIST *RM_FindControllerWithTarget(PARALLEL_REQUEST *pTargetRequests,
                                                 RM_PORT_LIST *portList,
                                                 UINT32 owner, UINT16 tid)
{
    PI_TARGET_INFO_RSP *pResponse;
    RM_PORT_LIST *pPortList = NULL;

    /*
     * Examine each entry in the Port List until an old owner is found.
     */
    while (portList->validEntry == TRUE && pPortList == NULL)
    {
        /*
         * Check that the controller is active and not the current owner.
         */
        if (portList->active && portList->ctrl != owner)
        {
            /*
             * Request target information.
             */
            pResponse = RM_FindTarget(pTargetRequests, portList->ctrl, tid);

            if (pResponse != NULL)
            {
                /*
                 * Is the target still on a port on this controller?
                 */
                if (pResponse->port < MAX_FE_PORTS)
                {
                    pPortList = portList;
                }
            }
        }

        /*
         * Advance to next entry.
         */
        ++portList;
    }

    return pPortList;
}


/*****************************************************************************
**  FUNCTION NAME: RM_CheckControllers
**
**  PARAMETERS: None.
**
**  RETURNS:    None.
**
**  COMMENTS:
**
**      RM Check Controllers.
**
**      Get the list of Good ports and the list of targets.
**
**      Checks if there are targets that need to be moved back
**      to the preferred port for those targets.
**
**      Checks if there are targets that need to be removed from
**      a failed port.
**
**      These target movements (if any) are place in a failure list.
**      The failure list is then processing to preform the
**      movement of targets.
**
******************************************************************************/
static void RM_CheckControllers(void)
{
    RM_PORT_LIST *portList = NULL;
    PI_TARGETS_RSP *targetList = NULL;
    RM_FAILURE_LIST *failureList = NULL;
    PARALLEL_REQUEST *pPR_Targets = NULL;
    PARALLEL_REQUEST *pPR_MPInfo = NULL;
    RM_RAID_RESYNC_LIST *pResyncList = NULL;
    UINT32      index1;

    /*
     * Loop here until we get good port and target information from
     * each of the controllers in the active controller map.
     */
    while (FOREVER)
    {
        /*
         * If there is an election in progress, break out of the loop
         * with no valid port or target lists.  The election completion
         * will cause the reallocation to run again.
         */
        if (RM_IsElectionRunning() == TRUE)
        {
            LogMessage(LOG_TYPE_DEBUG, "REALLOC-Election in progress, exit reallocation");

            break;
        }

        /*
         * Check if the FE queue is currently blocked, if it is we will
         * just wait and check again.  This is to prevent multiple requests
         * for port information getting backed up while the FE is blocked.
         */
        if (FEBlocked())
        {
            /*
             * Give up the processor and then continue back to the
             * top of the loop.
             */
            TaskSleepMS(20);
            continue;
        }

        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Request realloc info from all controllers");

        /*
         * Get the list of 'Good' ports for all controllers.
         */
        portList = RM_Ports();

        /*
         * If the port list was not retrieved successfully loop again
         * and retry the retrieval.
         */
        if (portList == NULL)
        {
            LogMessage(LOG_TYPE_DEBUG, "REALLOC-Port information invalid, retry");

            /*
             * Give up the processor and then continue back to the
             * top of the loop.
             */
            TaskSleepMS(20);
            continue;
        }

        /*
         * Get the mirror partner for all the controllers.
         *
         * If the mirror partners are not successfully retrieved
         * loop again and retry the retrieval.
         */
        if (RM_GetMirrorPartners(portList) != PI_GOOD)
        {
            LogMessage(LOG_TYPE_DEBUG, "REALLOC-Mirror partners invalid, retry");

            /*
             * Free the port list information, it will be retrieved
             * again when the loop continues.
             */
            Free(portList);

            /*
             * Give up the processor and then continue back to the
             * top of the loop.
             */
            TaskSleepMS(20);
            continue;
        }

        /*
         * Allocate the parallel request buffers.
         */
        pPR_Targets = PR_Alloc();

        /*
         * Send the PORTS parallel requests.
         */
        PR_SendRequests(PR_DEST_ACTIVE, PR_TYPE_TARGETS, pPR_Targets, PR_SendTaskTargets);

        /*
         * Allocate the parallel request buffers.
         */
        pPR_MPInfo = PR_Alloc();

        /*
         * Send the PORTS parallel requests.
         */
        PR_SendRequests(PR_DEST_ACTIVE,
                        PR_TYPE_GETMPCONFIG,
                        pPR_MPInfo, PR_SendTaskGetMirrorPartnerConfig);

        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Received realloc info from all controllers");

        /*
         * Verify the data we have received.  If all data is good
         * then we can break out of the loop and use the information.
         * If the data is not good, release the memory and continue
         * the loop.
         */
        if (RM_VerifyCheckControllerData(portList, pPR_Targets, pPR_MPInfo))
        {
            LogMessage(LOG_TYPE_DEBUG, "REALLOC-Realloc info from all controllers valid");

            break;
        }

        /*
         * Release the port list, targets and MP information since it is
         * not valid.  If an election is in progress we will exit out
         * and stop the reallocation.  If an election is not in progress
         * we will retrieve the information again and check if it is
         * valid.
         */
        Free(portList);
        PR_Release(&pPR_Targets);
        PR_Release(&pPR_MPInfo);
        /*
         * If there is an election in progress, break out of the loop
         * with no valid port or target lists.  The election completion
         * will cause the reallocation to run again.
         */
        if (RM_IsElectionRunning() == TRUE)
        {
            LogMessage(LOG_TYPE_DEBUG, "REALLOC-Election in progress, exit reallocation");

            return;
        }
    }

    /*
     * Get the list of targets for this controller.
     */
    if (pPR_Targets != NULL)
    {
        targetList = RM_FindTargets(pPR_Targets, GetMyControllerSN());
    }

    /*
     * If the configuration has changed, an election is in progress,
     * the portlist is NULL or the targetlist is NULL we can't continue
     * so skip the processing.
     */
    if (!rmConfigChanged &&
        RM_IsElectionRunning() == FALSE && portList != NULL && targetList != NULL)
    {
        /*
         * Dump the port information to debug (serial console or logs)
         */
        RM_DumpPortList(portList);

        /*
         * Count the number targets on each controller.
         */
        RM_FillControllerTargetCounts(targetList, portList, pPR_Targets);

        /*
         * Build the failure list using the port and target
         * information.
         */
        failureList = RM_BuildFailureList(targetList, portList, pPR_Targets);

        /*
         * If there are failures build the raid resync list.  This is
         * the list of raids that cannot be stripe resync'd and need
         * a full stripe resync.
         */
        if (failureList[0].to != 0)
        {
            /*
             * Build the raid resync list.
             */
            pResyncList = RM_BuildRaidResyncList(failureList, targetList);
        }

        /*
         * If the configuration has not changed and an election is
         * not in progress, process the failures.
         */
        if (!rmConfigChanged && RM_IsElectionRunning() == FALSE)
        {
            /*
             * Is there any action to perform?
             */
            if (failureList[0].to != 0)
            {
                RM_ProcessFailures(failureList, portList, pResyncList, pPR_MPInfo);
            }
            else
            {
                RM_RepairMirrorPartners(portList, pPR_MPInfo);
            }

            /*
             * If there is more than one controller still in the group
             * we need to check if the master has lost all of its FE
             * ports.  If so then an election is required.
             *
             * If there is only one controller, we need to make sure the
             * temporary cache disable bit has been cleared.
             */
            if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) > 1)
            {
                RM_CheckOnlinePorts(portList);
            }
            else
            {
                SM_TempDisableCache(GetMyControllerSN(),
                                    PI_MISC_CLRTDISCACHE_CMD, TEMP_DISABLE_MP, 0);
            }
        }

        /*
         * Free memory allocated for the failure list.
         */
        Free(failureList);
    }

    /*
     * Free memory allocated for the port list.
     */
    Free(portList);

    if (pResyncList)
    {
        /*
         * Free the resync list.
         */
        for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
        {
            if (pResyncList->controllerSN != 0)
            {
                Free(pResyncList->pList);
            }
        }

        Free(pResyncList);
    }

    /*
     * Free the parallel requests
     */
    PR_Release(&pPR_Targets);
    PR_Release(&pPR_MPInfo);
}


/**
******************************************************************************
**
**  @brief      Search through the port and target information and
**              verify there is data for each of the controllers in
**              the active controller map.
**
**  @param      portList - port list database
**  @param      pPR_Targets - Parallel request structure containing the
**                            target information for all controllers
**                            available.
**  @param      pPR_MPInfo - Parallel request structure containing the
**                           mirror partner information for all controllers
**                           available.
**
**  @return     none
**
******************************************************************************
**/
static bool RM_VerifyCheckControllerData(RM_PORT_LIST *pPortList,
                                         PARALLEL_REQUEST *pPR_Targets,
                                         PARALLEL_REQUEST *pPR_MPInfo)
{
    bool        bGood = true;
    UINT32      cActive;
    UINT32      index1;
    UINT32      controllerSN;

    if (pPortList != NULL && pPR_Targets != NULL)
    {
        cActive = ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr());

        for (index1 = 0; index1 < cActive; ++index1)
        {
            /*
             * Get the controller serial number from the controller
             * configuration map based on the active controller map
             * index value.
             */
            controllerSN = CCM_ControllerSN(Qm_GetActiveCntlMap(index1));

            if (RM_FindEntry(pPortList, controllerSN) == NULL ||
                RM_FindTargets(pPR_Targets, controllerSN) == NULL ||
                RM_FindMPInfo(pPR_MPInfo, controllerSN) == NULL)
            {
                bGood = false;
                break;
            }
        }
    }
    else
    {
        bGood = false;
    }

    return bGood;
}


/**
******************************************************************************
**
**  @brief      Search through the port and target information and
**              count the number of targets on each controller.
**
**  @param      targetList - this controllers target list
**  @param      portList - port list database
**  @param      pPR_Targets - Parallel request structure containing the
**                            target information for all controllers
**                            available.
**
**  @return     none
**
******************************************************************************
**/
static void RM_FillControllerTargetCounts(PI_TARGETS_RSP *targetList,
                                          RM_PORT_LIST *portList,
                                          PARALLEL_REQUEST *pPR_Targets)
{
    UINT32      index1;
    PI_TARGET_INFO_RSP *targetInfo;
    RM_PORT_LIST *thisPort = NULL;
    PI_TARGET_INFO_RSP *pResponse;

    /*
     * Count the number targets on each controller.
     */
    for (index1 = 0; index1 < targetList->count; ++index1)
    {
        /*
         * Get the target information.
         */
        targetInfo = &targetList->targetInfo[index1];

        /*
         * Find the controller that owns this target.
         */
        thisPort = RM_FindEntry(portList, targetInfo->owner);

        /*
         * Check if this target is associated for an undefined controller.
         */
        if (thisPort == NULL)
        {
            continue;
        }

        /*
         * Target info need to refreshed if the owner controller is
         * active but not the master.  The port information is
         * only valid on the controller that owns the target.
         */
        if (targetInfo->owner != GetMyControllerSN())
        {
            /*
             * Check if the target is on the master controller.
             */
            if (targetInfo->port < MAX_FE_PORTS)
            {
                /*
                 * The port is invalid on the owning controller
                 * since the target is still on the master.
                 */
                targetInfo->port = MAX_FE_PORTS;
            }
            else if (thisPort->active != FALSE)
            {
                /*
                 * Request target information.
                 */
                pResponse = RM_FindTarget(pPR_Targets,
                                          targetInfo->owner, targetInfo->tid);

                if (pResponse != NULL)
                {
                    /*
                     * Refresh the port information.
                     */
                    targetInfo->port = pResponse->port;
                }
            }
        }

        /*
         * Increment the number if targets owned by this controller.
         */
        ++thisPort->numberTargets;
    }
}


/**
******************************************************************************
**
**  @brief      Search through the port and target information and
**              determine what failures or corrections need to be
**              addressed and add them to the failure list.
**
**  @param      targetList - this controllers target list
**  @param      portList - port list database
**  @param      pPR_Targets - Parallel request structure containing the
**                            target information for all controllers
**                            available.
**
**  @return     RM_FAILURE_LIST* - Pointer to the failure list containing
**                                 the work to do for this reallocation.
**
**  @attention  The failure list returned must be freed by the calling
**              function.
**
******************************************************************************
**/
static RM_FAILURE_LIST *RM_BuildFailureList(PI_TARGETS_RSP *targetList,
                                            RM_PORT_LIST *portList,
                                            PARALLEL_REQUEST *pPR_Targets)
{
    UINT32      index1;
    RM_FAILURE_LIST *failureList;
    PI_TARGET_INFO_RSP *targetInfo;
    RM_PORT_LIST *thisPort = NULL;
    UINT32      bitPorts;
    UINT32      mirrorPartner;

    /*
     * Allocate and clear memory for failure list.  An
     * entry is allocated for each target, which should
     * be way more than we need.
     */
    failureList = MallocWC(sizeof(*failureList) * targetList->count);

    /*
     * Examine all targets as long as we have not identified that the
     * configuration has changed.
     *
     */
    for (index1 = 0; rmConfigChanged == FALSE && index1 < targetList->count; ++index1)
    {
        /*
         * Get the target information.
         */
        targetInfo = &targetList->targetInfo[index1];

        /*
         * Gind the controller that owns this target.
         */
        thisPort = RM_FindEntry(portList, targetInfo->owner);

        /*
         * Check if this target is associated for an undefined controller.
         */
        if (thisPort == NULL)
        {
            continue;
        }

        /*
         * Indicate the ports where a least one port must be GOOD.
         */
        bitPorts = (1 << targetInfo->pport) | (1 << targetInfo->aport);

        /*
         * The destination controller is the mirror partner.
         */
        mirrorPartner = thisPort->mirrorPartner;

        dprintf(DPRINTF_RM, "RM_BuildFailureList: Target %u, Ports 0x%x, MP 0x%x->0x%x\n",
                targetList->targetInfo[index1].tid,
                bitPorts, targetInfo->owner, mirrorPartner);

        /*
         * Check if the current controller is active.  Targets are
         * moved off of controllers that are not active.
         */
        if (thisPort->active == FALSE)
        {
            /*
             * Check if this controller has a mirror partner to
             * failover to other than itself.
             *
             * The destination controller is the mirror partner.
             *
             * Check if the destination controller is active.
             */
            if (mirrorPartner != 0 &&
                mirrorPartner != thisPort->ctrl &&
                RM_ActiveCtrlCheck(portList, mirrorPartner) != FALSE)
            {
                dprintf(DPRINTF_RM, "RM_BuildFailureList: Controller not active, fail-over partner 0x%x\n",
                        mirrorPartner);

                /*
                 * The current owner is not active.  The mirror
                 * partner is active.  We don't which port are
                 * good on the mirror partner.  A active controller
                 * is better than an inactive controller.
                 */
                RM_AddFailureAction(failureList, targetInfo->owner,
                                    thisPort, mirrorPartner, 0xF, FALSE);
            }
            else
            {
                /*
                 * Find the next active controller in the CCM.
                 */
                mirrorPartner = RM_FindNextActive(portList, thisPort->ctrl);

                /*
                 * Was a mirror partner found?
                 */
                if (mirrorPartner != 0)
                {
                    dprintf(DPRINTF_RM, "RM_BuildFailureList: Controller/MP not active, fail-over partner 0x%x\n",
                            mirrorPartner);

                    /*
                     * The current owner is not active.  The mirror
                     * partner is not set or inactive.  Another active
                     * controller was selected to fail over to. A active
                     * controller is better than an inactive controller.
                     *
                     * Note that raid-5 vdisks must be resynced.
                     */
                    RM_AddFailureAction(failureList, targetInfo->owner,
                                        thisPort, mirrorPartner, 0xF, FALSE);
                }
                else
                {
                    dprintf(DPRINTF_RM, "RM_BuildFailureList: Controller/MP not active, fail-over partner not available\n");

                    /*
                     * We should always find a controller for the targets
                     * so if we didn't we should restart the reallocation
                     * processing.
                     */
                    rmConfigChanged = TRUE;
                }
            }
        }

        /*
         * Check if the target is assigned to a port on this controller.
         */
        else if (targetInfo->port >= MAX_FE_PORTS &&
                 (thisPort->goodPorts & bitPorts) != 0)
        {
            thisPort = RM_FindControllerWithTarget(pPR_Targets, portList,
                                                   targetInfo->owner, targetInfo->tid);

            if (thisPort != NULL)
            {
                dprintf(DPRINTF_RM, "RM_BuildFailureList: Fail-back from 0x%x to 0x%x\n",
                        thisPort->ctrl, targetInfo->owner);

                /*
                 * Check if the target is on a controller that
                 * does not own this target
                 */
                RM_AddFailureAction(failureList, thisPort->ctrl,
                                    thisPort, targetInfo->owner, 0, TRUE);
            }
            else
            {
                /*
                 * The target is not owned by any controllers.  If the
                 * current target owner and the preferred target owner
                 * are the same and that controller is active we will
                 * cause another failback for that controller to cause
                 * the reallocation to occur.
                 */
                if (targetInfo->owner == targetInfo->powner &&
                    RM_ActiveCtrlCheck(portList, targetInfo->owner))
                {
                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Fail-back 0x%x since targets have been abandoned.",
                               targetInfo->owner);

                    RM_AddFailureAction(failureList, 0, NULL, targetInfo->owner, 0xF, TRUE);
                }
                else
                {
                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Failed to find controller with targets (0x%x, 0x%x)",
                               targetInfo->owner, targetInfo->tid);

                    /*
                     * If the preferred owner of the target is active
                     * reset its interfaces.
                     */
                    if (RM_ActiveCtrlCheck(portList, targetInfo->powner))
                    {
                        ResetInterfaceFE(targetInfo->powner,
                                         RESET_PORT_NEEDED, RESET_PORT_INIT);
                    }

                    /*
                     * If the current owner of the target is active and
                     * different than the preferred owner reset its
                     * interfaces.
                     */
                    if (targetInfo->owner != targetInfo->powner &&
                        RM_ActiveCtrlCheck(portList, targetInfo->owner))
                    {
                        ResetInterfaceFE(targetInfo->owner,
                                         RESET_PORT_NEEDED, RESET_PORT_INIT);
                    }

                    /*
                     * We should always find a controller for the targets
                     * so if we didn't we should restart the reallocation
                     * processing.
                     */
                    rmConfigChanged = TRUE;
                }
            }
        }

        /*
         * Check if the target is not owned by it's
         * preferred owner and the a GOOD/Online port exists
         * on the preferred owner.  This target can
         * be moved back to the preferred owner.
         */
        else if (targetInfo->owner != targetInfo->powner &&
                 RM_GoodPortCheck(portList, targetInfo->powner, bitPorts, TRUE) == TRUE)
        {
            /*
             * Check that all linked target (if any) also
             * have a GOOD port on the preferred owner.
             */
            if (RM_CheckLinkedTargets(portList, targetList,
                                      targetInfo, targetInfo->powner) == TRUE)
            {
                dprintf(DPRINTF_RM, "RM_BuildFailureList: Fail-back from 0x%x to preferred owner 0x%x\n",
                        targetInfo->owner, targetInfo->powner);

                /*
                 * The preferred owner has GOOD ports.  Create
                 * a record to move any targets back to the
                 * preferred owner of the target.
                 */
                RM_AddFailureAction(failureList, targetInfo->owner,
                                    thisPort, targetInfo->powner, bitPorts, TRUE);

                /*
                 * Indicate fail-back is occuring to this controller.
                 */
                RM_FindEntry(portList, targetInfo->powner)->failBack = TRUE;

                /*
                 * Find a new mirror partner for the controller
                 * receiving the targets.
                 */
                RM_FindMirrorPartner(portList, targetInfo->powner);
            }
            else
            {
                dprintf(DPRINTF_RM, "RM_BuildFailureList: Linked target ports not GOOD, fail-back not yet possible\n");
            }
        }
        /*
         * Check if there are not any GOOD ports on the
         * controller that currently owns this target.
         */
        else if (RM_GoodPortCheck(portList, targetInfo->owner, bitPorts, FALSE) == FALSE)
        {
            /*
             * Check if this controller has a mirror partner to
             * failover to other than itself.
             */
            if (mirrorPartner == 0 ||
                mirrorPartner == thisPort->ctrl ||
                !RM_ActiveCtrlCheck(portList, mirrorPartner))
            {
                /*
                 * Find the next active controller in the CCM.
                 */
                mirrorPartner = RM_FindNextActive(portList, thisPort->ctrl);

                dprintf(DPRINTF_RM, "RM_BuildFailureList: MirrorPartner not active, next available MirrorPartner is 0x%x\n",
                        mirrorPartner);
            }

            /*
             * Check if the controller we are moving resources
             * to is an active controller with GOOD ports.
             */
            if (RM_GoodPortCheck(portList, mirrorPartner, bitPorts, FALSE) != FALSE)
            {
                /*
                 * Check that GOOD ports are available on the
                 * mirror partner for any linked targets.
                 * If ports do not exist for linked targets
                 * (i.e. the host bus adapter is not installed
                 * in that slot), then ignore linked targets.
                 */
                if (RM_CheckLinkedTargets(portList, targetList,
                                          targetInfo, mirrorPartner) == TRUE)
                {
                    dprintf(DPRINTF_RM, "RM_BuildFailureList: Fail-over to MP (0x%x)\n",
                            mirrorPartner);

                    /*
                     * The current owner does not have any good ports.
                     * Create a record to move any targets to the
                     * mirror partner.
                     */
                    RM_AddFailureAction(failureList, targetInfo->owner,
                                        thisPort, mirrorPartner, bitPorts, FALSE);
                }
                else
                {
                    dprintf(DPRINTF_RM, "RM_BuildFailureList: Linked target ports not GOOD, fail-over not yet possible\n");
                }
            }
            else
            {
                dprintf(DPRINTF_RM, "RM_BuildFailureList: MirrorPartner does not have GOOD ports, fail-over not yet possible\n");
            }
        }
    }

    return failureList;
}


/**
******************************************************************************
**
**  @brief      Search through the port and target information and
**              determine what failures or corrections need to be
**              addressed and add them to the failure list.
**
**  @param      pFailureList - failure list database
**  @param      pTargetList - this controllers target list
**  @return     RM_RAID_RESYNC_LIST* - Pointer to the raid resync list
**                                     containing the controllers and
**                                     raid identifiers that require
**                                     individual resync requests.
**
**  @attention  The raid resync list returned must be freed by the calling
**              function.
**
******************************************************************************
**/
static RM_RAID_RESYNC_LIST *RM_BuildRaidResyncList(RM_FAILURE_LIST *pFailureList,
                                                   PI_TARGETS_RSP *pTargets)
{
    RM_RAID_RESYNC_LIST *pRaidResyncList;
    MR_LIST_RSP *pList;
    UINT32      index1 = 0;
    UINT32      iList = 0;
    PI_SERVERS_RSP *pServers;
    PI_VDISKS_RSP *pVDisks;

    pServers = Servers(GetMyControllerSN());
    pVDisks = VirtualDisks();

    /*
     * Allocate and clear memory for raid resync list.  An
     * entry is allocated for each raid, which should
     * be way more than we need.
     */
    pRaidResyncList = MallocWC(sizeof(*pRaidResyncList) * MAX_CONTROLLERS);

    while (pFailureList[index1].to != 0)
    {
        if (pFailureList[index1].failBack == FALSE &&
            pFailureList[index1].from != 0 &&
            pFailureList[index1].pFromPort->mirrorPartner != pFailureList[index1].to)
        {
            pList = SM_RaidsOwnedByController(pFailureList[index1].from,
                                              pServers, pTargets, pVDisks);

            /*
             * If there are raids owned by this controller add the list
             * to the resync list.
             */
            if (pList->ndevs > 0)
            {
                pRaidResyncList[iList].controllerSN = pFailureList[index1].from;
                pRaidResyncList[iList].pList = pList;

                LogMessage(LOG_TYPE_DEBUG, "REALLOC-Raids owned by 0x%x: %d",
                           pFailureList[index1].from, pList->ndevs);

                ++iList;
            }
        }
        ++index1;
    }

    /*
     * Free the allocated lists.
     */
    Free(pServers);
    Free(pVDisks);

    return pRaidResyncList;
}


/**
******************************************************************************
**
**  @brief      Process the failures contained in the failure list.
**
**  @param      failureList - failure list database
**  @param      portList - port list database
**  @param      pPR_MPInfo - Parallel request structure containing the
**                           mirror partner information for all controllers
**                           available.
**
**  @return     none
**
******************************************************************************
**/
static void RM_ProcessFailures(RM_FAILURE_LIST *failureList, RM_PORT_LIST *portList,
                               RM_RAID_RESYNC_LIST *pResyncList,
                               PARALLEL_REQUEST *pPR_MPInfo)
{
    INT32       rc = PI_GOOD;
    PARALLEL_REQUEST *pRequests;

    /*
     * Dump the actions to debug (serial console or logs)
     */
    RM_DumpActions(failureList);

    /*
     * Send the FLUSH_WOMP requests.
     */
    RM_ProcessFailure_FlushWOMP(failureList, portList);

    /*
     * Allocate the parallel request buffers.
     */
    pRequests = PR_AllocTemplate(TC_PREP_MOVE, NULL, 0);

    /*
     * Send the PORTS parallel requests.
     */
    PR_SendRequests(PR_DEST_ACTIVE, PR_TYPE_TC_PREP, pRequests, PR_SendTaskTargetControl);

    /*
     * Release the parallel request buffers.
     */
    PR_Release(&pRequests);

    /*
     * Fail or unfail any required controllers.
     */
    rc = RM_FailControllers(failureList);

    if (rc == PI_GOOD)
    {
        rc = RM_ResetInterfaces(failureList, portList);
    }

    if (rc == PI_GOOD)
    {
        if (gAllDevMissAtOtherDCN == 0)
        {
            fprintf(stderr, "<GR>CCB- Resync..controllers...\n");
            RM_ResyncControllers(failureList, portList, pResyncList);
        }
        else
        {
            fprintf(stderr, "<GR>CCB skipping resync initalising...\n");
            gAllDevMissAtOtherDCN = 0;
        }
        fprintf(stderr, "<GR>CCB- Processing Failure- FlushBEWc...\n");
        RM_ProcessFailure_FlushBEWC(failureList, portList);
        fprintf(stderr, "<GR>CCB- Process invalidateWCc...\n");
        RM_ProcessFailure_InvalidateWC(failureList, portList);
        fprintf(stderr, "<GR>CCB- Change Mirror Partners...\n");
        RM_ChangeMirrorPartners(failureList, portList, pPR_MPInfo);

        /*
         * Allocate the parallel request buffers.
         */
        pRequests = PR_AllocTemplate(RESCAN_EXISTING, NULL, 0);

        /*
         * Send the RESCAN DEVICES parallel requests.
         */
        PR_SendRequests(PR_DEST_ACTIVE, PR_TYPE_RESCANDEV, pRequests, PR_SendTaskRescanDevices);

        /*
         * Release the parallel request buffers.
         */
        PR_Release(&pRequests);
    }

    /*
     * Allocate the parallel request buffers.
     */
    pRequests = PR_AllocTemplate(TC_COMP_MOVE, NULL, 0);

    /*
     * Send the PORTS parallel requests.
     */
    PR_SendRequests(PR_DEST_ACTIVE, PR_TYPE_TC_COMP, pRequests, PR_SendTaskTargetControl);

    /*
     * Release the parallel request buffers.
     */
    PR_Release(&pRequests);
}


/**
******************************************************************************
**
**  @brief      Process the FLUSH_WOMP requests for required controllers.
**
**  @param      failureList - failure list database
**  @param      portList - port list database
**
**  @return     none
**
******************************************************************************
**/
static void RM_ProcessFailure_FlushWOMP(RM_FAILURE_LIST *pFailureList,
                                        RM_PORT_LIST *pPortList)
{
    UINT32      index1 = 0;
    RM_PORT_LIST *pThisPort = NULL;
    UINT32      mpcOption;      /* Mirror Partner Control Option    */
    INT32       mpcRC;          /* Mirror Partner Control RC        */

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Flush WOMP for required controllers");

    /*
     * Loop through the failure list looking for fail-overs so
     * we can send the FLUSH_WOMP request to the controllers
     * that are mirroring to the failed controllers.
     */
    while (pFailureList[index1].to != 0)
    {
        /*
         * Only for fail-overs do we need to do anything.
         */
        if (pFailureList[index1].failBack == FALSE)
        {
            /*
             * Find the entry for the failed controller, this will
             * also get us the controller that is mirroring to this
             * failed controller.
             */
            pThisPort = pFailureList[index1].pFromPort;

            /*
             * If the failed controller is being mirrored to by another
             * controller and that other controller is active, then that
             * controller needs to do the FLUSH WOMP.
             */
            if (pThisPort->srcMirrorPartner != 0 &&
                RM_ActiveCtrlCheck(pPortList, pThisPort->srcMirrorPartner))
            {
                /*
                 * Setup the mirror partner control option.
                 */
                mpcOption = MIRROR_PARTNER_CONTROL_OPT_WOMP;

                /*
                 * If a failed controller has a mirror partner, break
                 * the mirror.  The controller who was the mirror partner
                 * of the failed controller no longer has a mirror partner.
                 */
                mpcRC = SM_MirrorPartnerControl(pThisPort->srcMirrorPartner,
                                                pThisPort->ctrl, mpcOption, NULL);

                if (mpcRC != GOOD)
                {
                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Failed MPC (0x%x, 0x%x, 0x%x)",
                               pThisPort->srcMirrorPartner, pThisPort->ctrl, mpcOption);
                }
            }
        }

        /*
         * Move on to the next failure in the list.
         */
        index1++;
    }
}


/**
******************************************************************************
**
**  @brief      Fail or unfail the required controllers.
**
**  @param      failureList - failure list database
**
**  @return     INT32 - PI_GOOD or one of the PI error return codes.
**
******************************************************************************
**/
static INT32 RM_FailControllers(RM_FAILURE_LIST *pFailureList)
{
    UINT32      index1 = 0;
    INT32       rc = PI_GOOD;

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Fail or unfail controllers");

    /*
     * Loop through the failure list looking for fail-overs so
     * we can send the FLUSH_WOMP request to the controllers
     * that are mirroring to the failed controllers.
     */
    while (pFailureList[index1].to != 0)
    {
        if (pFailureList[index1].from != 0)
        {
            rc = RM_FailController(pFailureList[index1].from,
                                   pFailureList[index1].to,
                                   pFailureList[index1].failBack,
                                   pFailureList[index1].ports);

            /*
             * If the fail or unfail could not be successfully
             * processed then exit out and we will have to try
             * the entire reallocation process again.
             */
            if (rc != PI_GOOD)
            {
                LogMessage(LOG_TYPE_DEBUG, "REALLOC-Reset config changed");

                /*
                 * If the FailCtrl MRP failed, set
                 * this flag so resource manager will run again.
                 */
                rmConfigChanged = TRUE;
                break;
            }
        }

        /*
         * Move on to the next failure in the list.
         */
        index1++;
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Fail or unfail a controller and if the request times
**              out wait for the timeout processing to complete.
**
**  @param      UINT32 from
**  @param      UINT32 to
**  @param      UINT32 failBack - Flag indicating whether this is a
**                                failover or failback.
**  @param      UINT32 ports - Bitmask of which ports should be failed
**                             over or failed back.
**
**  @return     INT32 - PI_GOOD or one of the PI error return codes.
**
******************************************************************************
**/
static INT32 RM_FailController(UINT32 from, UINT32 to, UINT32 failBack, UINT32 ports)
{
    INT32       rc = PI_GOOD;

    /*
     * Fail or Unfail the controller.
     */
    rc = FailCtrl(from, to, failBack, ports);

    if (rc != GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Failed to fail controller (0x%x)", rc);
    }

    /*
     * Wait until a MRP executes without a timeout.
     */
    while (rc == PI_TIMEOUT)
    {
        /*
         * Check if RM is still busy, if not stop waiting for the
         * timeout to complete and just go on.
         *
         * This is old code from either Steve Howe or Mark Olson,
         * either way it needs to be investigated as to why we
         * would exit if RM is not busy or what we should really
         * do in the case of a timeout.
         */
        if (RMGetState() != RMBUSY)
        {
            LogMessage(LOG_TYPE_DEBUG, "REALLOC-Timeout wait stopped, RM not BUSY (0x%x)",
                       RMGetState());

            break;
        }

        /*
         * Check if there still are timeouts pending.
         */
        if (BEBlocked() == FALSE)
        {
            /*
             * See if the BE will sucessfully execute a MRP now.
             */
            rc = ProcessorQuickTest(MRNOPBE);

            /*
             * When the Fail Ctrl MRP time outs, the configuration
             * was not propagated to the slave controller.
             * Go it now.
             */
            if (rc == PI_GOOD)
            {
                RMSlavesConfigurationUpdate(X1_ASYNC_FECHANGED, TRUE);
            }
        }
        else
        {
            /*
             * Wait a second and try again.
             */
            TaskSleepMS(1000);
        }
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Add an item to the reset list.
**
**  @param      pResetList - Current reset list
**  @param      controllerSN - Controller to add to the list
**  @param      port - ports to reset for this controller
**  @param      option - option to use for the reset
**
**  @return     none
**
******************************************************************************
**/
static void RM_AddToResetList(RM_RESET_LIST *pResetList, UINT32 controllerSN,
                              UINT8 port, UINT8 option)
{
    UINT32      index1 = 0;
    bool        bFound = false;

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Add to reset list (0x%x, 0x%x, 0x%x)",
               controllerSN, port, option);

    while (pResetList[index1].controllerSN != 0)
    {
        if (pResetList[index1].controllerSN == controllerSN)
        {
            bFound = true;
            break;
        }
        index1++;
    }

    if (!bFound)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Reset Item Added: 0x%x", controllerSN);

        pResetList[index1].controllerSN = controllerSN;
        pResetList[index1].port = port;
        pResetList[index1].option = option;
    }
}


/**
******************************************************************************
**
**  @brief      Reset the FE interfaces on the required controllers.
**
**  @param      failureList - failure list database
**  @param      portList - port list database
**
**  @return     none
**
******************************************************************************
**/
static INT32 RM_ResetInterfaces(RM_FAILURE_LIST *pFailureList,
                                UNUSED RM_PORT_LIST *pPortList)
{
    UINT32      index1 = 0;
    INT32       rc = PI_GOOD;
    UINT32      commSlot;
    RM_RESET_LIST resetList[MAX_CONTROLLERS];

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Reset FE Interfaces");

    memset(&resetList, 0x00, sizeof(RM_RESET_LIST) * MAX_CONTROLLERS);

    while (pFailureList[index1].to != 0)
    {
        if (pFailureList[index1].from != 0)
        {
            if (pFailureList[index1].fromActive)
            {
                RM_AddToResetList(resetList,
                                  pFailureList[index1].from,
                                  RESET_PORT_NEEDED, RESET_PORT_INIT);
            }
            else
            {
                commSlot = GetCommunicationsSlot(pFailureList[index1].from);

                /*
                 * If this is a firmware update inactive controller
                 * we still want to reset the interfaces but use the
                 * NO INIT option to hold them in reset.
                 */
                if (EL_GetFailureState(commSlot) == FD_STATE_FIRMWARE_UPDATE_INACTIVE)
                {
                    RM_AddToResetList(resetList, pFailureList[index1].from,
                                      RESET_PORT_ALL, RESET_PORT_NO_INIT);
                }
            }
        }

        /*
         * Move on to the next failure in the list.
         */
        index1++;
    }

    /*
     * Reset the index to start at the beginning of the failure list.
     */
    index1 = 0;

    /*
     * Loop again but this time add only the destination controllers.
     */
    while (pFailureList[index1].to != 0)
    {
        RM_AddToResetList(resetList, pFailureList[index1].to,
                          RESET_PORT_NEEDED, RESET_PORT_INIT);

        index1++;
    }

    /*
     * Reset the index to start at the beginning of the reset list.
     */
    index1 = 0;

    /*
     * For each valid entry in the reset list, send the reset interfaces
     * request.
     */
    while (resetList[index1].controllerSN != 0)
    {
        rc = ResetInterfaceFE(resetList[index1].controllerSN,
                              resetList[index1].port, resetList[index1].option);

        /*
         * If the interfaces could not be reset and the reset was not
         * for a firmware update inactive controller (these are the only
         * ones that send the request with the NO_INIT option) then
         * signal that we need to run through reallocation again by
         * setting the configuration changed flag.
         */
        if (rc != PI_GOOD && resetList[index1].option != RESET_PORT_NO_INIT)
        {
            LogMessage(LOG_TYPE_DEBUG, "REALLOC-Reset config changed");

            /*
             * If the FailCtrl MRP failed, set
             * this flag so resource manager will run again.
             */
            rmConfigChanged = TRUE;

            break;
        }

        index1++;
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Sends the required Flush BE Write Cache requests to the
**              mirror partner of a failed controller.
**
**  @param      pFailureList - failure list database
**  @param      pPortList - port list database
**  @param      pResyncList - raid resync list database
**
**  @return     none
**
******************************************************************************
**/
static void RM_ProcessFailure_FlushBEWC(RM_FAILURE_LIST *pFailureList,
                                        UNUSED RM_PORT_LIST *pPortList)
{
    UINT32      index1 = 0;
    RM_PORT_LIST *pThisPort = NULL;
    INT32       rc;

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-FlushBEWC");

    while (pFailureList[index1].to != 0)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-FlushBEWC Item: 0x%x, 0x%x",
                   pFailureList[index1].from, pFailureList[index1].failBack);

        if (!pFailureList[index1].failBack)
        {
            /*
             * Get the entry for the controller moving targets to.
             */
            pThisPort = pFailureList[index1].pFromPort;

            /*
             * If the mirror partner for the failing controller
             * is not the same as the destination of the targets
             * then the BE cannot be flushed.
             */
            if (pThisPort->mirrorPartner != pFailureList[index1].to)
            {
                LogMessage(LOG_TYPE_DEBUG, "REALLOC-FlushBEWC not failing to MP (0x%x->0x%x)",
                           pFailureList[index1].from, pFailureList[index1].to);
            }
            else
            {
                rc = WCacheFlushBEWC(pFailureList[index1].to);

                if (rc != GOOD)
                {
                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Failed FlushBEWC (0x%x)",
                               pFailureList[index1].to);
                }
            }
        }

        index1++;
    }
}


/**
******************************************************************************
**
**  @brief      Sends the required Invalidate Write Cache requests to
**              a failed controller that is still active.
**
**  @param      pFailureList - failure list database
**  @param      pPortList - port list database
**  @param      pResyncList - raid resync list database
**
**  @return     none
**
******************************************************************************
**/
static void RM_ProcessFailure_InvalidateWC(RM_FAILURE_LIST *pFailureList,
                                           UNUSED RM_PORT_LIST *pPortList)
{
    UINT32      index1 = 0;
    RM_PORT_LIST *pThisPort = NULL;
    INT32       rc;

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-InvalidateWC");

    while (pFailureList[index1].to != 0)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-InvalidateWC Item: 0x%x, 0x%x",
                   pFailureList[index1].from, pFailureList[index1].failBack);

        if (!pFailureList[index1].failBack)
        {
            /*
             * Get the entry for the controller moving targets to.
             */
            pThisPort = pFailureList[index1].pFromPort;

            /*
             * If the mirror partner for the failing controller
             * is not the same as the destination of the targets
             * then the cache should not be invalidated.
             *
             * Also, if the failing controller is not active it also
             * cannot be invalidated.
             */
            if (pThisPort->mirrorPartner != pFailureList[index1].to)
            {
                LogMessage(LOG_TYPE_DEBUG, "REALLOC-InvalidateWC not failing to MP (0x%x->0x%x)",
                           pFailureList[index1].from, pFailureList[index1].to);
            }
            else if (!pThisPort->active)
            {
                LogMessage(LOG_TYPE_DEBUG, "REALLOC-InvalidateWC not active (0x%x)",
                           pFailureList[index1].from);
            }
            else
            {
                rc = WCacheInvalidate(pFailureList[index1].from, PI_WCACHE_INV_OPT_BEFE);

                if (rc != GOOD)
                {
                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Failed InvalidateWC (0x%x)",
                               pFailureList[index1].to);
                }
            }
        }

        index1++;
    }
}


/**
******************************************************************************
**
**  @brief      Sends the required resync requests to the destination
**              controllers.
**
**  @param      pFailureList - failure list database
**  @param      pPortList - port list database
**  @param      pResyncList - raid resync list database
**
**  @return     none
**
******************************************************************************
**/
static void RM_ResyncControllers(RM_FAILURE_LIST *pFailureList,
                                 UNUSED RM_PORT_LIST *pPortList,
                                 RM_RAID_RESYNC_LIST *pResyncList)
{
    UINT32      index1 = 0;
    RM_PORT_LIST *pThisPort = NULL;
    UINT32      mpcOption;      /* Mirror Partner Control Option    */
    INT32       mpcRC;          /* Mirror Partner Control RC        */

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Resync Controllers");

    while (pFailureList[index1].to != 0)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Resync Item: 0x%x, 0x%x",
                   pFailureList[index1].from, pFailureList[index1].failBack);

        if (!pFailureList[index1].failBack)
        {
            /*
             * Get the entry for the controller moving targets to.
             */
            pThisPort = pFailureList[index1].pFromPort;

            /*
             * If the mirror partner for the failing controller
             * is not the same as the destination of the targets
             * then resync all the raids.
             */
            if (pThisPort->mirrorPartner != pFailureList[index1].to)
            {
                RM_RaidResyncController(pFailureList[index1].to,
                                        pFailureList[index1].from, pResyncList);
            }
            else
            {
                /*
                 * Setup the mirror partner control option.
                 */
                mpcOption = MIRROR_PARTNER_CONTROL_OPT_RESYNC_ONLY;

                /*
                 * Resync the Raid 5 mirror records.
                 */
                mpcRC = SM_MirrorPartnerControl(pFailureList[index1].to,
                                                pFailureList[index1].to, mpcOption, NULL);

                if (mpcRC != GOOD)
                {
                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Failed MPC (0x%x, 0x%x, 0x%x)",
                               pFailureList[index1].to,
                               pFailureList[index1].to, mpcOption);
                }
            }
        }

        index1++;
    }
}


/**
******************************************************************************
**
**  @brief      Sends the required resync requests to the destination
**              controllers.
**
**  @param      UINT32 controllerSN - Controller to get the resync requests.
**  @param      UINT32 raidOwnerSN - Owner of the raids that need to
**                                   be resync'd.
**  @param      RM_RAID_RESYNC_LIST* pResyncList - raid resync list database
**
**  @return     none
**
******************************************************************************
**/
static void RM_RaidResyncController(UINT32 controllerSN, UINT32 raidOwnerSN,
                                    RM_RAID_RESYNC_LIST *pResyncList)
{
    INT32       rc = PI_GOOD;
    UINT32      index1;
    RM_RAID_RESYNC_LIST *pRaidResync = NULL;

    /*
     * Find the list corresponding to the raid owner serial number.
     */
    for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
    {
        if (pResyncList[index1].controllerSN == raidOwnerSN)
        {
            pRaidResync = &pResyncList[index1];
            break;
        }
    }

    /*
     * If there is a list, loop through the raid identifiers and send
     * the raid resync request to the destination controller.
     */
    if (pRaidResync != NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Resync Raids (0x%x, 0x%x)",
                   controllerSN, raidOwnerSN);

        rc = SM_RaidResyncController(controllerSN, pRaidResync->pList);

        if (rc != GOOD)
        {
            LogMessage(LOG_TYPE_DEBUG, "REALLOC-Failed resync of raids (0x%x, 0x%x)",
                       controllerSN, raidOwnerSN);
        }
    }
}


/**
******************************************************************************
**
**  @brief      Change the mirror partners based on the failure list.
**
**  @param      failureList - failure list database
**  @param      portList - port list database
**  @param      pPR_MPInfo - Parallel request structure containing the
**                           mirror partner information for all controllers
**                           available.
**
**  @return     none
**
******************************************************************************
**/
static PARALLEL_REQUEST *RM_BuildMirrorPartnerChanges(RM_FAILURE_LIST *pFailureList,
                                                      RM_PORT_LIST *pPortList,
                                                      PARALLEL_REQUEST *pPR_MPInfo)
{
    UINT32      iRequest = 0;
    PARALLEL_REQUEST *pRequests;
    UINT32      index1 = 0;
    RM_PORT_LIST *pThisPort = NULL;
    UINT32      mirrorPartner;
    MP_MIRROR_PARTNER_INFO *pMPInfo;

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Build Mirror Partner Changes");

    /*
     * Allocate the parallel request buffers.
     */
    pRequests = PR_Alloc();

    if (pFailureList != NULL)
    {
        while (pFailureList[index1].to != 0)
        {
            if (!pFailureList[index1].failBack)
            {
                /*
                 * Get the entry for the controller moving targets to.
                 */
                pThisPort = pFailureList[index1].pFromPort;

                /*
                 * If this port is active, set the mirror partner
                 * for this controller to itself, the failover took
                 * its targets away so it shouldn't be mirroring.
                 */
                if (pThisPort->active && pThisPort->ctrl != pThisPort->mirrorPartner)
                {
                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Add MP Change Request (1, %d, 0x%x->0x%x)",
                               iRequest, pThisPort->ctrl, pThisPort->ctrl);


                    /*
                     * Save the controller and its mirror partner
                     * in the parallel request structures.
                     */
                    pRequests[iRequest].controllerSN = pThisPort->ctrl;
                    pRequests[iRequest].param = pThisPort->ctrl;

                    /*
                     * Find the mirror partner information for the new
                     * mirror partner, create a copy of it and save it
                     * in the parallel request structure.
                     */
                    pMPInfo = RM_FindMPInfo(pPR_MPInfo, pThisPort->ctrl);
                    pRequests[iRequest].pParam = MallocW(sizeof(*pMPInfo));
                    memcpy(pRequests[iRequest].pParam, pMPInfo, sizeof(*pMPInfo));

                    /*
                     * Incremement the request count
                     */
                    ++iRequest;
                }

                /*
                 * If there is a controller mirroring to this failed
                 * controller and it is not the controller itself and
                 * that controller is active then that controller
                 * needs to get a new mirror partner.
                 *
                 * Find the next active controller after the failed
                 * controller and use that as the mirror partner.  If
                 * there is not an active controller then set the
                 * mirror partner to itself.
                 *
                 */
                if (pThisPort->srcMirrorPartner != 0 &&
                    pThisPort->srcMirrorPartner != pThisPort->ctrl &&
                    RM_ActiveCtrlCheck(pPortList, pThisPort->srcMirrorPartner))
                {
                    /*
                     * Find the next active controller past the source
                     * mirror parnters current mirror partner.  It may
                     * not be the following controller if multiple
                     * controllers have failed.
                     */
                    mirrorPartner = RM_FindNextActive(pPortList, pThisPort->ctrl);

                    /*
                     * There was not a next active controller, set the
                     * mirror partner to itself.
                     */
                    if (mirrorPartner == 0)
                    {
                        mirrorPartner = pThisPort->srcMirrorPartner;
                    }

                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Add MP Change Request (2, %d, 0x%x->0x%x)",
                               iRequest, pThisPort->srcMirrorPartner, mirrorPartner);

                    /*
                     * Save the controller and its mirror partner
                     * in the parallel request structures.
                     */
                    pRequests[iRequest].controllerSN = pThisPort->srcMirrorPartner;
                    pRequests[iRequest].param = mirrorPartner;


                    /*
                     * Find the mirror partner information for the new
                     * mirror partner, create a copy of it and save it
                     * in the parallel request structure.
                     */
                    pMPInfo = RM_FindMPInfo(pPR_MPInfo, mirrorPartner);
                    pRequests[iRequest].pParam = MallocW(sizeof(*pMPInfo));
                    memcpy(pRequests[iRequest].pParam, pMPInfo, sizeof(*pMPInfo));

                    /*
                     * Incremement the request count
                     */
                    ++iRequest;
                }
            }
            else
            {
                /*
                 * Get the entry for the controller moving targets to.
                 */
                pThisPort = RM_FindEntry(pPortList, pFailureList[index1].to);

                /*
                 * If there is a controller that should be mirroring
                 * to this now active controller add it to the list
                 * of changes required.
                 */
                if (pThisPort->newSrcMirrorPartner != 0)
                {
                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Add MP Change Request (3, %d, 0x%x->0x%x)",
                               iRequest, pThisPort->newSrcMirrorPartner, pThisPort->ctrl);

                    /*
                     * Save the controller and its mirror partner
                     * in the parallel request structures.
                     */
                    pRequests[iRequest].controllerSN = pThisPort->newSrcMirrorPartner;
                    pRequests[iRequest].param = pThisPort->ctrl;

                    /*
                     * Find the mirror partner information for the new
                     * mirror partner, create a copy of it and save it
                     * in the parallel request structure.
                     */
                    pMPInfo = RM_FindMPInfo(pPR_MPInfo, pThisPort->ctrl);
                    pRequests[iRequest].pParam = MallocW(sizeof(*pMPInfo));
                    memcpy(pRequests[iRequest].pParam, pMPInfo, sizeof(*pMPInfo));

                    /*
                     * Incremement the request count
                     */
                    ++iRequest;
                }

                /*
                 * If the now active controller should be mirroring
                 * to another controller add it to the list of changes
                 * required.
                 */
                if (pThisPort->newMirrorPartner != 0)
                {
                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Add MP Change Request (4, %d, 0x%x->0x%x)",
                               iRequest, pThisPort->ctrl, pThisPort->newMirrorPartner);

                    /*
                     * Save the controller and its mirror partner
                     * in the parallel request structures.
                     */
                    pRequests[iRequest].controllerSN = pThisPort->ctrl;
                    pRequests[iRequest].param = pThisPort->newMirrorPartner;

                    /*
                     * Find the mirror partner information for the new
                     * mirror partner, create a copy of it and save it
                     * in the parallel request structure.
                     */
                    pMPInfo = RM_FindMPInfo(pPR_MPInfo, pThisPort->newMirrorPartner);
                    pRequests[iRequest].pParam = MallocW(sizeof(*pMPInfo));
                    memcpy(pRequests[iRequest].pParam, pMPInfo, sizeof(*pMPInfo));

                    /*
                     * Incremement the request count
                     */
                    ++iRequest;
                }
            }

            index1++;
        }
    }
    else
    {
        /*
         * Check all controllers for possible mirror partner changes.
         */
        for (index1 = 0; pPortList[index1].validEntry == TRUE; ++index1)
        {
            /*
             * If the controller is active and it owns targets we
             * need to investigate to see if it has the correct
             * mirror partner.
             *
             * If the controller is active and it does not own targets, make
             * sure it is mirroring to itself.
             */
            if (pPortList[index1].active && pPortList[index1].numberTargets != 0)
            {
                /*
                 * Get the next active controller, this is who the mirror
                 * partner should be so if it is not we need to make some
                 * changes.
                 */
                mirrorPartner = RM_FindNextActive(pPortList, pPortList[index1].ctrl);

                /*
                 * If the next active controller is not available set the
                 * the controller to mirror to itself.
                 */
                if (mirrorPartner == 0)
                {
                    mirrorPartner = pPortList[index1].ctrl;
                }

                /*
                 * If the next active controller is available check if
                 * it is currently our mirror partner, if not we will
                 * be adding an item to the list of changes required.
                 */
                if (mirrorPartner != 0 &&
                    pPortList[index1].mirrorPartner != mirrorPartner)
                {
                    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Add MP Change Request (5, %d, 0x%x->0x%x)",
                               iRequest, pPortList[index1].ctrl, mirrorPartner);

                    /*
                     * Save the controller and its mirror partner
                     * in the parallel request structures.
                     */
                    pRequests[iRequest].controllerSN = pPortList[index1].ctrl;
                    pRequests[iRequest].param = mirrorPartner;

                    /*
                     * Find the mirror partner information for the new
                     * mirror partner, create a copy of it and save it
                     * in the parallel request structure.
                     */
                    pMPInfo = RM_FindMPInfo(pPR_MPInfo, mirrorPartner);
                    pRequests[iRequest].pParam = MallocW(sizeof(*pMPInfo));
                    memcpy(pRequests[iRequest].pParam, pMPInfo, sizeof(*pMPInfo));

                    /*
                     * Incremement the request count
                     */
                    ++iRequest;
                }
            }
            else if (pPortList[index1].active &&
                     pPortList[index1].numberTargets == 0 &&
                     pPortList[index1].mirrorPartner != pPortList[index1].ctrl)
            {
                /*
                 * Controller is active without targets, make sure it is
                 * mirroring to itself.
                 */
                LogMessage(LOG_TYPE_DEBUG, "REALLOC-Add MP Change Request (6, %d, 0x%x->0x%x)",
                           iRequest, pPortList[index1].ctrl, pPortList[index1].ctrl);

                /*
                 * Save the controller and its mirror partner
                 * in the parallel request structures.
                 */
                pRequests[iRequest].controllerSN = pPortList[index1].ctrl;
                pRequests[iRequest].param = pPortList[index1].ctrl;

                /*
                 * Incremement the request count
                 */
                ++iRequest;
            }
        }
    }

    return pRequests;
}


/**
******************************************************************************
**
**  @brief      Query the controllers requiring a mirror partner change
**              to see if they are in a state where the changes can occur.
**
**  @param      PARALLEL_REQUEST* pRequests - Pointer to a filled out list
**                                            of parallel request structures
**                                            containing the controllers
**                                            and their potential mirror
**                                            partners.
**  @param      UINT8 mpResponseMask - Mask used to deterime what responses
**                                     from the QMPC will still allow the
**                                     mirror partner changes to occur.
**
**  @return     bool - true if the mirror partner changes can occur, false
**                     otherwise.
**
**  @attention  If there is a code level mismatch where a controller does
**              not support the QMPC MRP this function will assume that
**              controller allows the change to occur.
**
******************************************************************************
**/
static bool RM_QueryMirrorPartnerChanges(PARALLEL_REQUEST *pRequests,
                                         UINT8 mpResponseMask)
{
    bool        bAllowChanges = true;
    UINT32      index1;
    UINT8       mpResponse;

    /*
     * Send the mirror partner change requests.
     */
    PR_SendRequests(PR_DEST_SPECIFIC, PR_TYPE_MPC, pRequests,
                    PR_SendTaskQueryMirrorPartnerChange);

    /*
     * Loop through all of the parallel request responses to determine
     * if the mirror partner changes can occur.
     *
     * NOTE: This loop should not exit early as it is also freeing the
     *       data pointers from the QMPC requests (they are no longer
     *       needed).
     */
    for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
    {
        /*
         * If the controller is not valid in this entry, skip to the next.
         */
        if (pRequests[index1].controllerSN == 0)
        {
            continue;
        }

        /*
         * If there was a data pointer returned from the request check
         * out the response value.  The response "anded" with the mask
         * must be zero to be acceptable.
         *
         * If there was not a data pointer assume that the changes
         * cannot take place.
         */
        if (pRequests[index1].pData != NULL)
        {
            mpResponse = ((PI_MISC_QUERY_MP_CHANGE_RSP *)pRequests[index1].pData)->mpResponse;

            LogMessage(LOG_TYPE_DEBUG, "REALLOC-QMPC response (0x%x, 0x%x)",
                       pRequests[index1].controllerSN, mpResponse);

            /*
             * A response from the QMPC of zero is acceptable, anything
             * else indicates that the changes should not be allowed.
             */
            if ((mpResponse & mpResponseMask) != 0)
            {
                LogMessage(LOG_TYPE_DEBUG, "REALLOC-QMPC indicates not to change (0x%x, 0x%x)",
                           pRequests[index1].controllerSN, mpResponse);

                bAllowChanges = false;
            }

            /*
             * The response from the QMPC is no longer needed so free
             * the data and set the pointer back to NULL.  We are reusing
             * the parallel request structures for several calls so this
             * just ensures that the structures are clean before the next
             * requests use them.
             */
            Free(pRequests[index1].pData);
            pRequests[index1].pData = NULL;
        }
        else
        {
            LogMessage(LOG_TYPE_DEBUG, "REALLOC-QMPC NULL response (0x%x)",
                       pRequests[index1].controllerSN);

            bAllowChanges = false;
        }
    }

    return bAllowChanges;
}


/**
******************************************************************************
**
**  @brief      Change the mirror partners based on the failure list.
**
**  @param      failureList - failure list database
**  @param      portList - port list database
**  @param      pPR_MPInfo - Parallel request structure containing the
**                           mirror partner information for all controllers
**                           available.
**
**  @return     none
**
******************************************************************************
**/
static void RM_ChangeMirrorPartners(RM_FAILURE_LIST *pFailureList,
                                    RM_PORT_LIST *pPortList, PARALLEL_REQUEST *pPR_MPInfo)
{
    PARALLEL_REQUEST *pRequests;

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Change Mirror Partners");

    /*
     * Build the change list for the mirror partners, the output is a
     * list of parallel requests.
     */
    pRequests = RM_BuildMirrorPartnerChanges(pFailureList, pPortList, pPR_MPInfo);

    /*
     * Query the controllers to determine if the mirror partner
     * changes can occur now or if they will have to wait.
     *
     * The query function returns true if the changes can occur now
     * and false if they cannot.
     */
    if (!RM_QueryMirrorPartnerChanges(pRequests, MRQMPC_MASK_ALL))
    {
        /*
         * The changes cannot be made now so start the change mirror
         * partner task which will delay and then initiate the changes.
         */
        RM_ChangeMirrorPartnerTaskStart();
    }
    else
    {
        /*
         * The changes are going to be made, make sure the task knows
         * that it is cancelled.
         */
        RM_ChangeMirrorPartnerTaskCancel();

        /*
         * Send the mirror partner change requests.
         */
        PR_SendRequests(PR_DEST_SPECIFIC,
                        PR_TYPE_MPC, pRequests, PR_SendTaskMirrorPartnerControl);
    }

    /*
     * Release the change requests.
     */
    PR_Release(&pRequests);
}


/**
******************************************************************************
**
**  @brief      Check the controllers to see who has online ports.
**
**              This function will ensure that if the current master
**              does not have any online ports and there is a slave
**              that does have online ports an election is run that
**              should change master-ship to the slave controller.
**
**  @param      portList - port list database
**
**  @return     none
**
******************************************************************************
**/
static void RM_CheckOnlinePorts(RM_PORT_LIST *portList)
{
    UINT32      index1;
    bool        bMasterOK = false;
    bool        bSlaveOK = false;

    /*
     * Check to see if the master controller has any ports and if
     * not kick off an election.
     */
    for (index1 = 0; portList[index1].validEntry == TRUE; ++index1)
    {
        /*
         * Is this port list for this controller?
         */
        if (portList[index1].ctrl == GetMyControllerSN())
        {
            /*
             * Is there port online?
             */
            if (portList[index1].onlinePorts > 0)
            {
                /*
                 * Master has FE ports, it doesn't matter what the
                 * slaves have in this case.
                 */
                bMasterOK = true;

                /*
                 * Exit out of the loop since the master/this controller
                 * has been found.
                 */
                break;
            }
        }
        else
        {
            /*
             * If we haven't already found a slave with good FE
             * ports, check this one to see if it has good ports.
             */
            if (!bSlaveOK && portList[index1].onlinePorts > 0)
            {
                /*
                 * Slave has FE ports so we have at least one
                 * slave with ports.
                 */
                bSlaveOK = true;
            }
        }
    }

    /*
     * If the master is not OK (no FE ports) and a slave with ports
     * has been found an election is required and should change the
     * mastership to the slave controller.
     */
    if (!bMasterOK && bSlaveOK)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Master with no FE ports, start election");

        (void)EL_DoElectionNonBlocking();
    }
}


/**
******************************************************************************
**
**  @brief      Repair any broken mirror partner configurations.
**
**  @param      portList - port list database
**  @param      pPR_MPInfo - Parallel request structure containing the
**                           mirror partner information for all controllers
**                           available.
**
**  @return     none
**
******************************************************************************
**/
static void RM_RepairMirrorPartners(RM_PORT_LIST *portList, PARALLEL_REQUEST *pPR_MPInfo)
{
    PARALLEL_REQUEST *pRequests;
    PARALLEL_REQUEST *pTCRequests;
    UINT32      index1;

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Repair Mirror Partners");

    /*
     * Build the change list for the mirror partners, the output is a
     * list of parallel requests.
     */
    pRequests = RM_BuildMirrorPartnerChanges(NULL, portList, pPR_MPInfo);

    if (!RM_QueryMirrorPartnerChanges(pRequests, MRQMPC_MASK_NO_COMM))
    {
        /*
         * The changes cannot be made now so start the change mirror
         * partner task which will delay and then initiate the changes.
         */
        RM_ChangeMirrorPartnerTaskStart();
    }
    else
    {
        /*
         * Allocate the parallel request buffers.
         */
        pTCRequests = PR_AllocTemplate(TC_PREP_MOVE, NULL, 0);

        for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
        {
            if (pRequests[index1].controllerSN != 0)
            {
                pTCRequests[index1].controllerSN = pRequests[index1].controllerSN;
            }
        }

        /*
         * Send the PORTS parallel requests.
         */
        PR_SendRequests(PR_DEST_SPECIFIC, PR_TYPE_TC_PREP,
                        pTCRequests, PR_SendTaskTargetControl);

        /*
         * Release the parallel request buffers.
         */
        PR_Release(&pTCRequests);

        /*
         * Query the controllers to determine if the mirror partner
         * changes can occur now or if they will have to wait.
         *
         * The query function returns true if the changes can occur now
         * and false if they cannot.
         */
        if (!RM_QueryMirrorPartnerChanges(pRequests, MRQMPC_MASK_ALL))
        {
            /*
             * The changes cannot be made now so start the change mirror
             * partner task which will delay and then initiate the changes.
             */
            RM_ChangeMirrorPartnerTaskStart();
        }
        else
        {
            /*
             * The changes are going to be made, make sure the task knows
             * that it is cancelled.
             */
            RM_ChangeMirrorPartnerTaskCancel();

            /*
             * Send the mirror partner change requests.
             */
            PR_SendRequests(PR_DEST_SPECIFIC, PR_TYPE_MPC,
                            pRequests, PR_SendTaskMirrorPartnerControl);
        }

        /*
         * Allocate the parallel request buffers.
         */
        pTCRequests = PR_AllocTemplate(TC_COMP_MOVE, NULL, 0);

        for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
        {
            if (pRequests[index1].controllerSN != 0)
            {
                pTCRequests[index1].controllerSN = pRequests[index1].controllerSN;
            }
        }

        /*
         * Send the PORTS parallel requests.
         */
        PR_SendRequests(PR_DEST_SPECIFIC, PR_TYPE_TC_COMP,
                        pTCRequests, PR_SendTaskTargetControl);

        /*
         * Release the parallel request buffers.
         */
        PR_Release(&pTCRequests);

        /*
         * Release the change requests.
         */
        PR_Release(&pRequests);
    }
}


/**
******************************************************************************
**
**  @brief      This function searches the mirror partner information to find
**              the information for a specific controller.
**
**  @param      pPR_MPInfo - Parallel request structure containing
**                           the mirror partner information for all
**                           controllers available.
**  @param      controllerSN - serial number of the controller.
**
**  @return     MP_MIRROR_PARTNER_INFO* - Pointer to mirror partner
**                                        information for the given
**                                        controller or NULL if there
**                                        is not a match.
**
**  @attention  Do not free the information returned from this
**              function.  The data is still owned by the parallel
**              request structures.
**
******************************************************************************
**/
static MP_MIRROR_PARTNER_INFO *RM_FindMPInfo(PARALLEL_REQUEST *pPR_MPInfo,
                                             UINT32 controllerSN)
{
    PARALLEL_REQUEST *pRequest;
    MP_MIRROR_PARTNER_INFO *pData = NULL;

    if (pPR_MPInfo != NULL)
    {
        pRequest = PR_FindRequest(pPR_MPInfo, controllerSN);

        if (pRequest != NULL)
        {
            pData = (MP_MIRROR_PARTNER_INFO *)pRequest->pData;
        }
    }

    return pData;
}


/**
******************************************************************************
**
**  @brief      This function searches the targets information to find
**              the targets for a specific controller.
**
**  @param      pTargetRequests - Parallel request structure containing
**                                the target information for all
**                                controllers available.
**  @param      controllerSN - serial number of the controller.
**
**  @return     PI_TARGETS_RSP* - Pointer to targets information for
**                                the given controller or NULL if there
**                                is not a match.
**
**  @attention  Do not free the targets information returned from this
**              function.  The data is still owned by the parallel
**              request structures.
**
******************************************************************************
**/
static PI_TARGETS_RSP *RM_FindTargets(PARALLEL_REQUEST *pTargetRequests,
                                      UINT32 controllerSN)
{
    PARALLEL_REQUEST *pRequest;
    PI_TARGETS_RSP *pTargets = NULL;

    if (pTargetRequests != NULL)
    {
        pRequest = PR_FindRequest(pTargetRequests, controllerSN);

        if (pRequest != NULL)
        {
            pTargets = (PI_TARGETS_RSP *)pRequest->pData;
        }
    }

    return pTargets;
}


/**
******************************************************************************
**
**  @brief      This function searches the targets information to find
**              a specific target for a specific controller.
**
**  @param      pTargetRequests - Parallel request structure containing
**                                the target information for all
**                                controllers available.
**  @param      controllerSN - serial number of the controller.
**  @param      tid - Target ID to find.
**
**  @return     PI_TARGET_INFO_RSP* - Pointer to target information for
**                                    the given controller and target ID
**                                    or NULL if there is not a match.
**
**  @attention  Do not free the target information returned from this
**              function.  The data is still owned by the parallel
**              request structures.
**
******************************************************************************
**/
static PI_TARGET_INFO_RSP *RM_FindTarget(PARALLEL_REQUEST *pTargetRequests,
                                         UINT32 controllerSN, UINT16 tid)
{
    PI_TARGETS_RSP *pTargets;
    UINT32      iTarget;
    PI_TARGET_INFO_RSP *pTarget = NULL;

    pTargets = RM_FindTargets(pTargetRequests, controllerSN);

    if (pTargets != NULL)
    {
        for (iTarget = 0; iTarget < pTargets->count; ++iTarget)
        {
            if (pTargets->targetInfo[iTarget].tid == tid)
            {
                pTarget = &pTargets->targetInfo[iTarget];

                /*
                 * Found the target, exit now.
                 */
                break;
            }
        }
    }

    return pTarget;
}


/**
******************************************************************************
**
**  @brief      Dump the information in the port list to debug.
**
**  @param      portList - port list database
**
**  @return     none
**
******************************************************************************
**/
static void RM_DumpPortList(RM_PORT_LIST *portList)
{
    UINT32      index1;

    for (index1 = 0; index1 < Qm_GetNumControllersAllowed(); ++index1)
    {
        if (portList[index1].validEntry == TRUE)
        {
            if (portList[index1].active == FALSE)
            {
                LogMessage(LOG_TYPE_DEBUG, "REALLOC-Inactive controller 0x%x (MP: 0x%x) ports Valid: %x Good: %x Online: %x",
                           portList[index1].ctrl,
                           portList[index1].mirrorPartner,
                           portList[index1].validPorts,
                           portList[index1].goodPorts, portList[index1].onlinePorts);
            }
            else if (portList[index1].initializing != FALSE)
            {
                LogMessage(LOG_TYPE_DEBUG, "REALLOC-Initializing controller 0x%x (MP: 0x%x) ports Valid: %x Good: %x Online: %x",
                           portList[index1].ctrl,
                           portList[index1].mirrorPartner,
                           portList[index1].validPorts,
                           portList[index1].goodPorts, portList[index1].onlinePorts);
            }
            else
            {
                LogMessage(LOG_TYPE_DEBUG, "REALLOC-Active controller 0x%x (MP: 0x%x) ports Valid: %x Good: %x Online: %x",
                           portList[index1].ctrl,
                           portList[index1].mirrorPartner,
                           portList[index1].validPorts,
                           portList[index1].goodPorts, portList[index1].onlinePorts);
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      Dump the information in the failure list to debug.
**
**  @param      failureList - failure list database
**  @param      count - number of items in the failure list
**
**  @return     none
**
******************************************************************************
**/
static void RM_DumpActions(RM_FAILURE_LIST *pFailureList)
{
    UINT16      index1 = 0;

    /*
     * Check if this entry already exists.
     */
    while (pFailureList[index1].to != 0)
    {
        dprintf(DPRINTF_RM, "REALLOC-%s ports 0x%x from 0x%x to 0x%x\n",
                (pFailureList[index1].failBack == FALSE) ? "Failover" : "Failback",
                pFailureList[index1].ports,
                pFailureList[index1].from, pFailureList[index1].to);

        index1++;
    }
}


/*****************************************************************************
**  FUNCTION NAME: RM_ReallocTask
**
**  PARAMETERS: None.
**
**  RETURNS:    None.
**
**  COMMENTS:
**
**      RM Reallocation task.
**
**      Reallocation task walks the RM configuration tree looking for failed
**      controllers or interfaces.  When one is found the targets are reallocated
**      according to reconfiguration rules.  Note that some situations require
**      that targets remain on a failed controller/interface (notably if cache
**      data cannot be recovered)
**
**      RM lock is assumed to be acquired before this task starts and is released
**      when the operation is complete.
**
**      Note that this task exits if a shutdown is requested during its processing.
**
******************************************************************************/
static void RM_ReallocTask(UNUSED TASK_PARMS *parms)
{
    RMErrCode   rc;
    UINT32      startTime;

    TraceEvent(TRACE_RM, TRACE_RM_REALLOC_TASK);

    startTime = RTC_GetSystemSeconds();

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Reallocation task started");

    /*
     * Initialize and clear config changed flag
     */
    rmConfigChanged = FALSE;

    /*
     * Acquire lock and start reconfig.
     */
    rc = rmSetBusy();

    while (rc == RMOK)
    {
        rmConfigChanged = FALSE;

        /*
         * Check the status of controller and move targets as needed.
         */
        RM_CheckControllers();

        /*
         * Release lock
         */
        rmClearBusy();

        if (rmConfigChanged == TRUE)
        {
            rc = rmSetBusy();
        }
        else
        {
            /*
             * Exit loop, we are done
             */
            break;
        }
    }

    /*
     * Send changed events to the XSSA since something triggered
     * RM to go through initialization again (election, controller
     * failure, etc.).
     */
    SendX1ChangeEvent(X1_ASYNC_PCHANGED | X1_ASYNC_ZCHANGED);

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Reallocation task finished (ET=%u sec)",
               RTC_GetSystemSeconds() - startTime);

    /*
     * If there were defrags active before the reallocation, restart
     * them as they were terminated during the failover.
     */
    if (Qm_GetDefragActive())
    {
        PDiskDefragOperation(Qm_GetDefragPID(), RM_DEFRAG_DELAY);
    }

    /*
     * Re-enable caching for the INACTIVATE user just in case the failover
     * and/or reallocation were caused by a rolling code update or a
     * controller inactivation.
     */
    (void)RMTempDisableCache(PI_MISC_CLRTDISCACHE_CMD,
                             TEMP_DISABLE_INACTIVATE, T_DISABLE_CLEAR_ALL);

    /*
     * Re-enable caching for the CONFIG_PROP user just in case the failover
     * and/or reallocation were caused by an error trap during a configuration
     * change.
     */
    (void)RMTempDisableCache(PI_MISC_CLRTDISCACHE_CMD,
                             TEMP_DISABLE_CONFIG_PROP, T_DISABLE_CLEAR_ALL);

    /*
     * Done with failure actions, clear PCB.
     */
    pRMReallocTask = NULL;

    /*
     * Check for RM config changed here; if TRUE, restart task.
     */
    if ((rmConfigChanged == TRUE) && (RMShutdownRequested != TRUE))
    {
        /* Restart this task */
        RmStartReallocTask();
    }

    TraceEvent(TRACE_RM, TRACE_RM_REALLOC_TASK);
}


/**
******************************************************************************
**
**  @brief      Function to start/restart the change mirror partner task.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void RM_ChangeMirrorPartnerTaskStart(void)
{
    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Change Mirror Partner Task: START");

    /*
     * If the task has not yet been created, create it now.
     */
    if (gpChangeMirrorPartnerTask == NULL)
    {
        gpChangeMirrorPartnerTask = TaskCreate(RM_ChangeMirrorPartnerTask, NULL);
    }

    /*
     * Make sure the flag indicates that the mirror partner changes are
     * required and reset the counter.
     */
    gChangeMirrorPartnerTaskFlag = true;
    gChangeMirrorPartnerTaskCounter = RM_CHANGE_MP_COUNTER;
}


/**
******************************************************************************
**
**  @brief      Function to cancel the change mirror partner task.  This
**              will not kill the task but will set the task flag to
**              indicate the request is no longer needed.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void RM_ChangeMirrorPartnerTaskCancel(void)
{
    /*
     * Log the CANCEL message only if the task is currently running.
     */
    if (gpChangeMirrorPartnerTask != NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Change Mirror Partner Task: CANCEL");
    }

    /*
     * To cancel the mirror partner task just set the flag indicating that
     * the mirror partner changes are required to false.  When the task
     * completes its countdown it will see the flag has been changed and
     * then skip the processing.
     */
    gChangeMirrorPartnerTaskFlag = false;
}


/**
******************************************************************************
**
**  @brief      Task function to change the mirror partners for a DSC.
**              When first started this function will delay for a period
**              of time using a counter which can be reset outside of the
**              task.  This allows other tasks to reset the countdown
**              timer and make the delay longer.  Once the countdown timer
**              expires the function will check if the mirror partner
**              changes are still necessary (based on the flag) and if
**              required restart the reallocation task.
**
**  @param      UINT32 dummy - Required parameter for forking a task.
**
**  @return     none
**
******************************************************************************
**/
static void RM_ChangeMirrorPartnerTask(UNUSED TASK_PARMS *parms)
{
    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Change Mirror Partner Task: BEGIN");

    /*
     * Continue to wait until the countdown reaches zero.  The counter
     * could get reset by another reallocation task running.  This
     * counter ensures that the changes would occur no later than the
     * counter loops from the last time the QMPC request was submitted.
     */
    while (gChangeMirrorPartnerTaskCounter > 0)
    {
        --gChangeMirrorPartnerTaskCounter;

        TaskSleepMS(1000);
    }

    /*
     * If the flag still indicates the mirror partners still need
     * changing, attempt to process the changes.
     */
    if (gChangeMirrorPartnerTaskFlag)
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Change Mirror Partner Task: PROCESS");

        gChangeMirrorPartnerTaskFlag = false;
        rmConfigChanged = TRUE;

        RmStartReallocTask();
    }
    else
    {
        LogMessage(LOG_TYPE_DEBUG, "REALLOC-Change Mirror Partner Task: SKIP");
    }

    /*
     * The task is ending so NULL out the task pointer.
     */
    gpChangeMirrorPartnerTask = NULL;

    LogMessage(LOG_TYPE_DEBUG, "REALLOC-Change Mirror Partner Task: END");
}


/**
******************************************************************************
**
**  @brief      Restores and checks the configuration information until
**              it sees the targets have moved off of this controller.
**              This function is used by inactive controllers (actual
**              inactive and FW Update inactive) to wait until their
**              configuration information has moved.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void RM_RestoreAndCheckForTargets(void)
{
    PI_TARGETS_RSP *pTargets;
    bool        bOwnTargets;
    INT32       i;

    while (TRUE)
    {
        bOwnTargets = false;

        /*
         * Restore the configuration information from the file system.
         */
        SM_NVRAMRestore(MRNOFSYS | MRNOOVERLAY, NULL);

        /*
         * Get the targets information so we can check if we are still
         * the owner of any targets.
         */
        pTargets = Targets(GetMyControllerSN());

        /*
         * Loop through the target information and check the owner
         * of each one to see if it is this controller.
         */
        for (i = 0; pTargets != NULL && i < pTargets->count; ++i)
        {
            /*
             * Is this controller the owner?
             */
            if (pTargets->targetInfo[i].owner == GetMyControllerSN())
            {
                /*
                 * This controller currently owns this target.  This
                 * should be a temporary condition so flag that we
                 * own targets and break out of the targets loop.
                 */
                bOwnTargets = true;
                break;
            }
        }

        /*
         * We don't need this set of target information as it will
         * likely change after the next restore if needed or we will
         * be exiting the function.
         */
        Free(pTargets);

        /*
         * If we don't own targets, we are done, exit out of the while.
         */
        if (!bOwnTargets)
        {
            break;
        }

        /*
         * We still own targets so give the DSC a chance to move the
         * targets by sleeping for a little bit and then looping back
         * to the top to restore the configuration information again.
         */
        TaskSleepMS(1000);
    }
}


/**
******************************************************************************
**
**  @brief      Retrieves the resync operation information and checks to
**              see if there are any operations in progress.  This function
**              will loop here until it sees no operations in progress.
**
**  @param      UINT32 timeout - number of seconds before timing out the
**                               wait loop.
**
**  @return     none
**
******************************************************************************
**/
void RM_WaitForResyncInactivateOperations(UINT32 timeout)
{
    PI_MISC_RESYNCDATA_RSP *pResyncData;
    COR        *cors;
    UINT16      index1;
    UINT32      tmoCounter = 0;

    LogMessage(LOG_TYPE_DEBUG, "RM_WaitForResyncInactivateOperations - ENTER");

    while (tmoCounter < timeout)
    {
        pResyncData = SM_ResyncData(GetMyControllerSN());

        if (pResyncData)
        {
            /*
             * Get a pointer to the start of the array of cors from the response
             * packet (this response has multiple formats but we call it to get
             * the COR array).
             */
            cors = (COR *)pResyncData->data.data;

            for (index1 = 0; index1 < pResyncData->count; ++index1)
            {
                /*
                 * Check if the resync operations have completed.
                 */
                if (cors[index1].ocsecst != OCSE_SI_NORESOURCES)
                {
                    if (cors[index1].rcscl == 0xff && cors[index1].rcsvd == 0xff &&
                        cors[index1].rcdcl == 0xff && cors[index1].rcdvd == 0xff)
                    {
                        LogMessage(LOG_TYPE_DEBUG, "RESYNC-Stale entry..srccl = %d, srcvid = %d, destcl = %d, destvid = %d--ignoring\n",
                                   cors[index1].rcscl, cors[index1].rcsvd,
                                   cors[index1].rcdcl, cors[index1].rcdvd);
                    }
                    else
                    {
                        LogMessage(LOG_TYPE_DEBUG, "RESYNC-Resources still owned (%d)",
                                   cors[index1].rid);

                        /*
                         * This controller still owns some resync resources
                         * so we have to continue to wait.
                         */
                        break;
                    }
                }
            }

            /*
             * If we reached the end of the list of copies it means
             * we did not find any in progress operations and we
             * can exit out of here.
             */
            if (index1 == pResyncData->count)
            {
                LogMessage(LOG_TYPE_DEBUG, "RESYNC-Resources NOT owned, done waiting");

                Free(pResyncData);

                /*
                 * Exit out of the while loop.
                 */
                break;
            }

            Free(pResyncData);
        }
        else
        {
            LogMessage(LOG_TYPE_DEBUG, "RESYNC-Failed to retrieve resync data");
        }

        /*
         * The resync operations have not yet completed so sleep
         * for a little bit and then looping back to start the
         * check again.
         */
        TaskSleepMS(1000);

        /*
         * Increment the timeout counter.
         */
        tmoCounter++;
    }

    /*
     * If the timeout expired log an indication so it can be tracked later.
     */
    if (tmoCounter >= timeout)
    {
        LogMessage(LOG_TYPE_DEBUG, "RESYNC-Timeout waiting for inactivate operations");
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
