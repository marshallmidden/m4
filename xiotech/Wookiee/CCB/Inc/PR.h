/* $Id: PR.h 143020 2010-06-22 18:35:56Z m4 $*/
/**
******************************************************************************
**
**  @file       PR.h
**
**  @brief      Parallel Request Execution
**
**  This file contains the coding required to support sending parallel
**  requests to two or more controllers in a DSC.
9*
**  Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
/**
**  @defgroup _PR_H_ Parallel Request
**  @{
**/
#ifndef __PR_H__
#define __PR_H__

#include "XIO_Std.h"
#include "XIO_Types.h"

#include "pcb.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/**
**  @name Parallel Request - Type Definitions
**  @{
**/
#define PR_TYPE_GENERIC     0x0000      /**< Generic parallel request       */
#define PR_TYPE_PORTS       0x0001      /**< Port List                      */
#define PR_TYPE_TARGETS     0x0002      /**< Targets                        */
#define PR_TYPE_GETMPCONFIG 0x0003      /**< Get Mirror Partner Config Info */
#define PR_TYPE_TC_PREP     0x0004      /**< Target Control Prepare         */
#define PR_TYPE_TC_COMP     0x0008      /**< Target Control Completion      */
#define PR_TYPE_CONFIG      0x0010      /**< Configuration Update           */
#define PR_TYPE_QMPC        0x0020      /**< Query Mirror Partner Change    */
#define PR_TYPE_MPC         0x0040      /**< Mirror Partner Change          */
#define PR_TYPE_TDISCACHE   0x0080      /**< Temporary Disable of Cache     */
#define PR_TYPE_QTDISCACHE  0x0081      /**< Query Temp Disable of Cache    */
#define PR_TYPE_GLOBALCACHE 0x0100      /**< Stats global cache             */
#define PR_TYPE_STOPIO      0x0200      /**< Stop IO                        */
#define PR_TYPE_STARTIO     0x0201      /**< Start IO                       */
#define PR_TYPE_RESCANDEV   0x0400      /**< Rescan devices                 */
/* @} */

/**
**  @name Parallel Request - Destination Types
**        Bit fields, can set multiple values.
**
**        The code currently does not support all combinations of
**        these types.  See the PR_SendRequests function for the
**        actual code implementation.
**  @{
*/
#define PR_DEST_MASTER      0x0001  /**< Send to the current master
                                     **  controller, even this controller.  */
#define PR_DEST_SLAVES      0x0002  /**< Send to all slave controllers,
                                     **  except this controller.            */
#define PR_DEST_SELF        0x0004  /**< Send to this controller only.      */
#define PR_DEST_SPECIFIC    0x0008  /**< Send to a specific controller,
                                     **  even this controller.              */
#define PR_DEST_OTHERS      0x0010  /**< Send to all controllers other
                                     **  than this controller.              */
#define PR_DEST_ACTIVE      0x0020  /**< Send only to active controllers    */
/* @} */

/** Parallel Request Information                                            */
typedef struct PARALLEL_REQUEST
{
    UINT32      controllerSN;           /**< Controller for this request    */
    PCB        *pPCB;                   /**< PCB for this request task      */
    UINT32      param;                  /**< Parameter for this request     */
    void       *pParam;                 /**< Pointer to parameter structure */
    INT32       rc;                     /**< Return code                    */
    void       *pData;                  /**< Response data pointer          */
    UINT8       rsvd24[8];              /**< RESERVED                       */
} PARALLEL_REQUEST;

/** Parallel Request - Function Prototype                                   */

typedef void (*ParallelRequestTask_func)(TASK_PARMS *parms);
    /**< Forked task parameters
     **  p1 = pointer to parallel request structure
     **  p2 = controller serial number  */

/** Parallel Request - Port List Parameters                                 */
typedef struct PR_PORT_LIST_PARAM
{
    UINT32      processor;              /**< Processor for this request     */
    UINT8       rsvd4[2];               /**< RESERVED                       */
    UINT16      type;                   /**< Type of port list              */
} PR_PORT_LIST_PARAM;

/** Parallel Request - Configuration Update Parameters                      */
typedef struct PR_CONFIG_UPDATE_PARAM
{
    UINT8       rsvd0[3];               /**< RESERVED                       */
    UINT8       restoreOption;          /**< Type of restore operation      */
    UINT32      reason;                 /**< Reason for the update          */
} PR_CONFIG_UPDATE_PARAM;

/** Parallel Request - Temporary Cache Disable Parameters                   */
typedef struct PR_TDISCACHE_PARAM
{
    UINT8       user;                   /**< User                           */
    UINT8       option;                 /**< Clear temp disable option      */
    UINT8       rsvd2[2];               /**< RESERVED                       */
} PR_TDISCACHE_PARAM;

/** Parallel Request - Stop IO Parameters                                   */
typedef struct PR_STOPIO_PARAM
{
    UINT8       operation;              /**< Flush operation requested      */
    UINT8       intent;                 /**< Intent                         */
    UINT8       user;                   /**< User                           */
    UINT8       rsvd2;                  /**< RESERVED                       */
    UINT32      tmo;                    /**< Timeout to use for request     */
} PR_STOPIO_PARAM;

/** Parallel Request - Start IO Parameters                                  */
typedef struct PR_STARTIO_PARAM
{
    UINT8       option;                 /**< Start option value             */
    UINT8       rsvd1;                  /**< RESERVED                       */
    UINT8       user;                   /**< User                           */
    UINT8       rsvd3;                  /**< RESERVED                       */
    UINT32      tmo;                    /**< Timeout to use for request     */
} PR_STARTIO_PARAM;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern PARALLEL_REQUEST *PR_Alloc(void);

extern PARALLEL_REQUEST *PR_AllocTemplate(UINT32 paramTemplate,
                                          void *pParamTemplate, UINT32 lParamTemplate);

extern void PR_Release(PARALLEL_REQUEST **ppRequest);

extern void PR_SendRequests(UINT32 destination,
                            UINT32 requestType,
                            PARALLEL_REQUEST *pParallelRequests,
                            ParallelRequestTask_func requestTask);

extern PARALLEL_REQUEST *PR_FindRequest(PARALLEL_REQUEST *pParallelRequests,
                                        UINT32 controllerSN);

extern void PR_SendTaskPorts(TASK_PARMS *parms);

extern void PR_SendTaskTargets(TASK_PARMS *parms);

extern void PR_SendTaskTargetControl(TASK_PARMS *parms);

extern void PR_SendTaskConfigUpdate(TASK_PARMS *parms);

extern void PR_SendTaskQueryMirrorPartnerChange(TASK_PARMS *parms);

extern void PR_SendTaskMirrorPartnerControl(TASK_PARMS *parms);

extern void PR_SendTaskTempDisableCache(TASK_PARMS *parms);

extern void PR_SendTaskQueryTempDisableCache(TASK_PARMS *parms);

extern void PR_SendTaskStopIO(TASK_PARMS *parms);

extern void PR_SendTaskStartIO(TASK_PARMS *parms);

extern void PR_SendTaskRescanDevices(TASK_PARMS *parms);

extern void PR_SendTaskGetMirrorPartnerConfig(TASK_PARMS *parms);

/* @} */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PR_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
