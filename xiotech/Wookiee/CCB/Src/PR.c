/* $Id: PR.c 156532 2011-06-24 21:09:44Z m4 $*/
/**
******************************************************************************
**
**  @file       PR.c
**
**  @brief      Parallel Request Execution
**
**  This file contains the coding required to support sending parallel
**  requests to two or more controllers in a DSC.
**
**  Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "PR.h"
#include "logging.h"
#include "PacketInterface.h"
#include "PI_Stats.h"
#include "PI_Target.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "RMCmdHdl.h"
#include "rm_val.h"
#include "sm.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define PR_ALLOC_SIZE   (sizeof(PARALLEL_REQUEST) * MAX_CONTROLLERS)

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void PR_SendRequestsAdd(PARALLEL_REQUEST *pParallelRequests, UINT32 controllerSN);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Allocates the required memory for a parallel request.  This
**              allocates enough space for MAX_CONTROLLERS requests.
**
**  @param      none
**
**  @return     PARALLEL_REQUEST* - Pointer to the start of the array of
**                                  parallel request structures.
**
**  @attention  User must free the memory returned from this function.  An
**              easy way is ot use the PR_Release function which will also
**              free the underlying data buffers.
**
******************************************************************************
**/
PARALLEL_REQUEST *PR_Alloc(void)
{
    return MallocWC(PR_ALLOC_SIZE);
}


/**
******************************************************************************
**
**  @brief      Allocates the required memory for a parallel request.  This
**              allocates enough space for MAX_CONTROLLERS requests and
**              fills the parameter information with the given template
**              data.
**
**  @param      UINT32 paramTemplate - Value to use as the template for
**                                     the param value in the structure.
**  @param      void* pParamTemplate - Pointer to the template to use for
**                                     all requests.
**  @param      UINT32 lParamTemplate - Length/Size of the template pointed
**                                      to by pParamTemplate.
**  @return     PARALLEL_REQUEST* - Pointer to the start of the array of
**                                  parallel request structures.
**
**  @attention  User must free the memory returned from this function.  An
**              easy way is ot use the PR_Release function which will also
**              free the underlying data buffers.
**
******************************************************************************
**/
PARALLEL_REQUEST *PR_AllocTemplate(UINT32 paramTemplate, void *pParamTemplate,
                                   UINT32 lParamTemplate)
{
    PARALLEL_REQUEST *pRequest;
    UINT32      index1;

    /*
     * Allocate the base parallel request structures.
     */
    pRequest = PR_Alloc();

    for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
    {
        /*
         * Copy the parameter template for the UINT32 parameter.
         */
        pRequest[index1].param = paramTemplate;

        /*
         * If there is a parameter buffer template then duplicate
         * that buffer for this entry.
         */
        if (pParamTemplate)
        {
            pRequest[index1].pParam = MallocW(lParamTemplate);
            memcpy(pRequest[index1].pParam, pParamTemplate, lParamTemplate);
        }
    }

    return pRequest;
}


/**
******************************************************************************
**
**  @brief      Releases the memory associated with a parallel request
**              array and the array itself.
**
**  @param      PARALLEL_REQUEST** ppRequest
**                  - pointer to a pointer to the start of an array of
**                    parallel request structures.  This array must be
**                    allocated to contain MAX_CONTROLLERS elements.
**
**  @return     none
**
**  @attention  This function requires that there be MAX_CONTROLLERS items
**              in the array.  It will free the underlying data buffers
**              stored in each PARALLEL_REQUEST element and then free
**              the array itself.
**
**              It is easier if users use both the PR_Alloc and PR_Release
**              since they build and free the correct buffers.
**
******************************************************************************
**/
void PR_Release(PARALLEL_REQUEST **ppRequest)
{
    UINT32      index1;
    PARALLEL_REQUEST *pRequest;

    if (*ppRequest != NULL)
    {
        pRequest = *ppRequest;

        /*
         * Free the parameter and data buffers
         */
        for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
        {
            Free(pRequest[index1].pParam);
            Free(pRequest[index1].pData);
        }

        Free(*ppRequest);
    }
}


/**
******************************************************************************
**
**  @brief      Adds a controller serial number to the requests if it
**              is not already in the list.
**
**  @param      PARALLEL_REQUEST* pParallelRequests
**                  - pointer to the start of an array of parallel request
**                    structures.  This array must be allocated to contain
**                    MAX_CONTROLLERS elements.
**  @param      UINT32 controllerSN - Serial number of the controller to add.
**
**  @return     none
**
******************************************************************************
**/
static void PR_SendRequestsAdd(PARALLEL_REQUEST *pParallelRequests, UINT32 controllerSN)
{
    UINT32      index1;
    bool        bFound = false;

    for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
    {
        if (pParallelRequests[index1].controllerSN == controllerSN)
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
        {
            if (pParallelRequests[index1].controllerSN == 0)
            {
                pParallelRequests[index1].controllerSN = controllerSN;
                break;
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      Creates tasks to send parallel request to multiple
**              controllers in a DSC.
**
**  @param      PARALLEL_REQUEST* pParallelRequests
**                  - pointer to the start of an array of parallel request
**                    structures.  This array must be allocated to contain
**                    MAX_CONTROLLERS elements.
**  @param      ParallelRequestTask_func requestTask
**                  - Function pointer for the request task to run on
**                    each of the controllers.
**
**  @return     none
**
******************************************************************************
**/
void PR_SendRequests(UINT32 destination, UNUSED UINT32 requestType,
                     PARALLEL_REQUEST *pParallelRequests,
                     ParallelRequestTask_func requestTask)
{
    UINT32      controllerSN;
    PCB        *pSendTask;
    bool        bSendsComplete = false;
    UINT32      index1;
    UINT32      count;
    TASK_PARMS  taskParms;

    /*
     * Depending on the destination type, fill in the parallel requests
     * with the correct serial numbers.
     */

    /*
     * If this request is supposed to go to this controller add it
     * to the list of requests.
     */
    if (destination & PR_DEST_SELF)
    {
        PR_SendRequestsAdd(pParallelRequests, GetMyControllerSN());
    }

    /*
     * If this request is supposed to go to the master controller
     * add the master's serial number to the list of requests.
     */
    if (destination & PR_DEST_MASTER)
    {
        PR_SendRequestsAdd(pParallelRequests, Qm_GetMasterControllerSN());
    }

    if (destination & PR_DEST_ACTIVE)
    {
        count = ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr());

        for (index1 = 0; index1 < count; ++index1)
        {
            /*
             * Get the controller serial number from the controller
             * configuration map based on the active controller map
             * index value.
             */
            controllerSN = CCM_ControllerSN(Qm_GetActiveCntlMap(index1));

            /*
             * If the destination type says to send this only to other
             * controllers, check if this controller serial number is
             * this controller and if it is skip it.
             */
            if (destination & PR_DEST_OTHERS && controllerSN == GetMyControllerSN())
            {
                continue;
            }

            /*
             * If the destination type says to send this only to slave
             * controllers, check if this controller is the master and
             * if it is skip it.
             */
            if (destination & PR_DEST_SLAVES &&
                controllerSN == Qm_GetMasterControllerSN())
            {
                continue;
            }

            PR_SendRequestsAdd(pParallelRequests, controllerSN);
        }
    }
    else if (destination & PR_DEST_OTHERS)
    {
        /*
         * The requests are supposed to go just to the controllers
         * other then this controller, search the controller
         * configuration map and add all the controllers other
         * than this controller.
         */
        for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
        {
            controllerSN = CCM_ControllerSN(index1);

            if (controllerSN == 0 || controllerSN == GetMyControllerSN())
            {
                continue;
            }

            PR_SendRequestsAdd(pParallelRequests, controllerSN);
        }
    }
    else if (destination & PR_DEST_SLAVES)
    {
        /*
         * The requests are supposed to go just to the slaves,
         * search the controller configuration map and add all
         * the controllers other than the master.
         */
        for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
        {
            controllerSN = CCM_ControllerSN(index1);
            if (controllerSN == 0 || controllerSN == Qm_GetMasterControllerSN())
            {
                continue;
            }
            PR_SendRequestsAdd(pParallelRequests, controllerSN);
        }
    }

    /*
     * Loop through the parallel requests and create the task for any
     * entry that has a controller serial number.
     */
    for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
    {
        if (pParallelRequests[index1].controllerSN != 0)
        {
            taskParms.p1 = (UINT32)&pParallelRequests[index1];
            taskParms.p2 = (UINT32)pParallelRequests[index1].controllerSN;
            pSendTask = TaskCreate(*requestTask, &taskParms);

            pParallelRequests[index1].pPCB = pSendTask;
        }
    }

    /* Give up the processor to allow the sends to run. */
    TaskSwitch();

    /*
     * Since we have spawned off tasks to do the configuration
     * propagation we need to wait until they all complete.  Loop
     * here until the PCB list contains all NULL values which
     * indicates all tasks have completed.
     */
    while (!bSendsComplete)
    {

        /*
         * Each time through the loop, assume that the sends are complete
         * and look for ones that are not.
         */
        bSendsComplete = true;

        /*
         * Loop through the PCB list looking for non-NULL entries.  This
         * indicates a task has not yet finished.
         */
        for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
        {
            /*
             * Is the entry NULL, if not then the task has not yet completed.
             */
            if (pParallelRequests[index1].pPCB != NULL)
            {
                bSendsComplete = false;
                TaskSleepMS(20);
                break;
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      Find the entry in the array of parallel requests for a
**              given controller.
**
**  @param      PARALLEL_REQUEST* pParallelRequests
**                  - pointer to the start of an array of parallel request
**                    structures.  This array must be allocated to contain
**                    MAX_CONTROLLERS elements.
**  @param      UINT32 controllerSN - Serial number of the controller.
**
**  @return     PARALLEL_REQUEST* - pointer to an element in the given
**                                  parallel request array or NULL if
**                                  there is not an element for the
**                                  given controller.
**
**  @attention  This function returns a pointer to an element in the
**              parallel request array.  The user must not free this
**              memory since it is still owned by the array.
**
******************************************************************************
**/
PARALLEL_REQUEST *PR_FindRequest(PARALLEL_REQUEST *pParallelRequests, UINT32 controllerSN)
{
    UINT32      iRequest;
    PARALLEL_REQUEST *pRequest = NULL;

    if (pParallelRequests != NULL)
    {
        /*
         * Loop through the requests in the array and find the one
         * for the given controller serial number.
         */
        for (iRequest = 0; iRequest < MAX_CONTROLLERS; ++iRequest)
        {
            if (pParallelRequests[iRequest].controllerSN == controllerSN)
            {
                pRequest = &pParallelRequests[iRequest];

                /*
                 * We found the controller, exit now.
                 */
                break;
            }
        }
    }

    return pRequest;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
**  @attention  This parallel request task fills in the data pointer in
**              the PARALLEL_REQUEST array entry for the controller.  The
**              user must free this memory.
**
******************************************************************************
**/
void PR_SendTaskPorts(TASK_PARMS *parms)
{
    PR_PORT_LIST_PARAM *pParam;
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    pParam = (PR_PORT_LIST_PARAM *)pParallelRequest->pParam;

    /*
     * Get the port list information and save it in the parallel request
     * elements data pointer.
     *
     * NOTE: The port list information may be NULL if the request fails.
     */
    pParallelRequest->pData = PortList(controllerSN, pParam->processor, pParam->type);

    if (pParallelRequest->pData == NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed PORTS (0x%x).", controllerSN);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
**  @attention  This parallel request task fills in the data pointer in
**              the PARALLEL_REQUEST array entry for the controller.  The
**              user must free this memory.
**
******************************************************************************
**/
void PR_SendTaskTargets(TASK_PARMS *parms)
{
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    /*
     * Get the targets information and save it in the parallel request
     * elements data pointer.
     *
     * NOTE: The targets information may be NULL if the request fails.
     */
    pParallelRequest->pData = Targets(controllerSN);

    if (pParallelRequest->pData == NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed TARGETS (0x%x).", controllerSN);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
******************************************************************************
**/
void PR_SendTaskTargetControl(TASK_PARMS *parms)
{
    PI_PROC_TARGET_CONTROL_RSP *pResponse;
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    while (FOREVER)
    {
        pResponse = SM_TargetControl(controllerSN, pParallelRequest->param);

        /*
         * If there are outstanding operations pending the request
         * will be attempted again.
         */
        if (pResponse && pResponse->header.status == DEOUTOPS)
        {
            LogMessage(LOG_TYPE_DEBUG, "PR-Failed TC, OUTOPS (0x%x, 0x%x)",
                       controllerSN, pParallelRequest->param);

            Free(pResponse);
        }
        else
        {
            /*
             * If the request was not successful or it returned something
             * other than OK, log a message.
             */
            if (!pResponse || pResponse->header.status != DEOK)
            {
                LogMessage(LOG_TYPE_DEBUG, "PR-Failed TC (0x%x, 0x%x, 0x%x)",
                           controllerSN, pParallelRequest->param,
                           pResponse ? pResponse->header.status : 0);
            }
            break;
        }
    }

    /*
     * Make sure any remainging response data is freed.
     */
    Free(pResponse);

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to send
**              configuration updates to controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
******************************************************************************
**/
void PR_SendTaskConfigUpdate(TASK_PARMS *parms)
{
    UINT32      rc = GOOD;
    PR_CONFIG_UPDATE_PARAM *pParam;
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    pParam = (PR_CONFIG_UPDATE_PARAM *)pParallelRequest->pParam;

    /* Send the IPC_CONFIGURATION_UPDATE packet to the controller */
    rc = RMSendIpcConfigurationUpdate(controllerSN, pParam->restoreOption,
                                      pParam->reason);

    if (rc != GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed config update (0x%x, 0x%x, 0x%x).",
                   controllerSN, pParam->restoreOption, pParam->reason);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
******************************************************************************
**/
void PR_SendTaskQueryMirrorPartnerChange(TASK_PARMS *parms)
{
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    /*
     * Get the response from the query for mirror partner change and
     * save it in the parallel request elements data pointer.
     */
    pParallelRequest->pData = SM_QueryMirrorPartnerChange(controllerSN,
                                                          pParallelRequest->param);

    if (pParallelRequest->pData == NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed QMPC (0x%x).", controllerSN);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
******************************************************************************
**/
void PR_SendTaskMirrorPartnerControl(TASK_PARMS *parms)
{
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    /*
     * Send the mirror partner control request.  The param value in the
     * parallel request structure has the serial number of mirror partner.
     */
    pParallelRequest->rc = SM_MirrorPartnerControl(controllerSN,
                                                   pParallelRequest->param,
                                                   0, pParallelRequest->pParam);

    if (pParallelRequest->rc != GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed MPC (0x%x, 0x%x, 0x%x)",
                   controllerSN, pParallelRequest->param, 0);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
******************************************************************************
**/
void PR_SendTaskTempDisableCache(TASK_PARMS *parms)
{
    PR_TDISCACHE_PARAM *pParam;
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    pParam = (PR_TDISCACHE_PARAM *)pParallelRequest->pParam;

    /*
     * Send the temporary disable of cache request.  The param value in the
     * parallel request structure has the serial number of mirror partner.
     */
    pParallelRequest->rc = SM_TempDisableCache(controllerSN,
                                               pParallelRequest->param,
                                               pParam->user, pParam->option);

    if (pParallelRequest->rc != GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed TDISCACHE (0x%x, 0x%x, 0x%x, 0x%x)",
                   controllerSN, pParallelRequest->param, pParam->user, pParam->option);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
******************************************************************************
**/
void PR_SendTaskQueryTempDisableCache(TASK_PARMS *parms)
{
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    /*
     * Send the temporary disable of cache request.  The param value in the
     * parallel request structure has the serial number of mirror partner.
     */
    pParallelRequest->pData = SM_QueryTempDisableCache(controllerSN);

    if (pParallelRequest->pData == NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed QTDISCACHE (0x%x)", controllerSN);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
**  @attention  This parallel request task fills in the data pointer in
**              the PARALLEL_REQUEST array entry for the controller.  The
**              user must free this memory.
**
******************************************************************************
**/
void PR_SendTaskStopIO(TASK_PARMS *parms)
{
    PR_STOPIO_PARAM *pParam;
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    pParam = (PR_STOPIO_PARAM *)pParallelRequest->pParam;

    pParallelRequest->rc = StopIO(controllerSN, pParam->operation,
                                  pParam->intent, pParam->user, pParam->tmo);

    if (pParallelRequest->rc != GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed STOPIO (0x%x, 0x%x, 0x%x, 0x%x, 0x%x)",
                   controllerSN, pParam->operation, pParam->intent,
                   pParam->user, pParam->tmo);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
**  @attention  This parallel request task fills in the data pointer in
**              the PARALLEL_REQUEST array entry for the controller.  The
**              user must free this memory.
**
******************************************************************************
**/
void PR_SendTaskStartIO(TASK_PARMS *parms)
{
    PR_STARTIO_PARAM *pParam;
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    pParam = (PR_STARTIO_PARAM *)pParallelRequest->pParam;

    pParallelRequest->rc = StartIO(controllerSN, pParam->option, pParam->user,
                                   pParam->tmo);

    if (pParallelRequest->rc != GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed STARTIO (0x%x, 0x%x, 0x%x, 0x%x)",
                   controllerSN, pParam->option, pParam->user, pParam->tmo);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
******************************************************************************
**/
void PR_SendTaskRescanDevices(TASK_PARMS *parms)
{
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    /*
     * Submit the rescan devcies to the controllers.
     */
    pParallelRequest->rc = SM_RescanDevices(controllerSN, pParallelRequest->param);

    if (pParallelRequest->rc != GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed RESCANDEV (0x%x, 0x%x)",
                   controllerSN, pParallelRequest->param);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Task used by the parallel request execution to request
**              information from controllers.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is a
**                                  pointer to the parallel request structure
**                                  and p2 is the controller serial number
**                                  for the request.
**
**  @return     none
**
**  @attention  This parallel request task fills in the data pointer in
**              the PARALLEL_REQUEST array entry for the controller.  The
**              user must free this memory.
**
******************************************************************************
**/
void PR_SendTaskGetMirrorPartnerConfig(TASK_PARMS *parms)
{
    PARALLEL_REQUEST *pParallelRequest = (PARALLEL_REQUEST *)parms->p1;
    UINT32      controllerSN = parms->p2;

    /*
     * Get the mirror partner configuration information and save it in the
     * parallel request elements data pointer.
     *
     * NOTE: The mirror partner configuration information may be NULL if
     * the request fails.
     */
    pParallelRequest->pData = SM_GetMirrorPartnerConfig(controllerSN);

    if (pParallelRequest->pData == NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "PR-Failed GETMIRRORPARTNERCONFIG (0x%x).",
                   controllerSN);
    }

    /*
     * Set the PCB in the parallel request element to NULL to indicate
     * that this task has finished.
     */
    pParallelRequest->pPCB = NULL;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
