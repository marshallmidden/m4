/* $Id: options.h 145407 2010-08-10 17:26:37Z m4 $ */
/**
******************************************************************************
**
**  @file       options.h
**
**  @brief      Build options
**
**  This contains conditional compiler options.
**  This file is a subset of options.inc - make sure the two files stay in sync.
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include "XIO_Const.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define DEBUG_FLIGHTREC         /* Global flight recorder */

#define DEBUG_FLIGHTREC_D       /* Define */
#define  LINUX_VER_NVP6_MM      TRUE

#define LINUX_VER_NEWMP TRUE     /*MP Info*/
/*#define DEBUG_NEWMP TRUE */        /*MP Debug */

/**
**  Software Detected Fault Options: FALSE = Firmware Alert Log Event
**                                   TRUE = Error Trap
**/
#define SW_FAULT_DEBUG          TRUE

/**
**  @name   Kernel options
**/
/* @{ */
#define KERNEL_DEBUG_5      FALSE   /**< Enable check for PCB_READY tasks
                                     **  before suspending                  */
/* @} */

/**
**  @name   Trace Fault
**          This is a circular queue of pointers, usually of the call path.
**          The queue is pointed to by tf_queue which uses a QCB structure.
**          The functions K$tf_on and K$tf_off also allow user control of
**          this trace buffer.
**/
/* @{ */
#define TRACE_FAULT_ALLOC_SIZE      (128 * 4)   /**< Allocation sizeof trace
                                                 **  fault buffer           */
#define TRACE_FAULT_MODE            0x00000008  /**< Mode bits to trace
                                                 **  bit 1 = instruction
                                                 **  bit 2 = branch
                                                 **  bit 3 = call
                                                 **  bit 4 = return
                                                 **  bit 5 = prereturn
                                                 **  bit 6 = supervisor
                                                 **  bit 7 = mark           */
/* @} */

/**
**  @name   Flight recorder - see fr.h for description
**/
/* @{ */
#define DEBUG_FLIGHT_REC        TRUE    /**< Enable flight recorder basics  */
#define FLIGHT_REC_ALLOC_SIZE   (FR_SIZE * 4000)    /**< sizeof allocated
                                                     **  flight recorder    */
#ifdef PERF
/**
** Performance code options
**/
#define DEBUG_FLT_REC_DEFINE        TRUE    /**< Define                     */
#define DEBUG_FLT_REC_ONLINE        TRUE    /**< Online                     */
#define DEBUG_FLT_REC_HOT_SPARE     TRUE    /**< Hot spare                  */

#define DEBUG_FLT_REC_CDRV          FALSE   /**< Cdriver                    */
#define DEBUG_FLT_REC_CACHE         FALSE   /**< Cache                      */
#define DEBUG_FLT_REC_LINK          FALSE   /**< Link layer                 */
#define DEBUG_FLT_REC_VIRTUAL       FALSE   /**< Virtual layer              */
#define DEBUG_FLT_REC_RAID          FALSE   /**< RAID layer                 */
#define DEBUG_FLT_REC_DLM           FALSE   /**< DLM layer                  */
#define DEBUG_FLT_REC_PHY           FALSE   /**< Physical layer             */
#define DEBUG_FLT_REC_PHY_DISC      TRUE    /**< Physical discovery layer   */
#define DEBUG_FLT_REC_PHY_REC       TRUE    /**< Physical recovery          */
#define DEBUG_FLT_REC_ISP           FALSE   /**< ISP layer                  */
#define DEBUG_FLIGHTREC_FD          TRUE    /**< Fabric Discovery           */
#define DEBUG_FLT_REC_MISC          FALSE   /**< Misc layer                 */

#define DEBUG_FLT_REC_MEMORY        FALSE   /**< Malloc and mrel tracing    */
#define DEBUG_FLT_REC_XCHANG        FALSE   /**< Context switch timing      */
#define DEBUG_FLT_REC_TIME          FALSE   /**< Time stamping              */

#else
/**
** Debug code options
**/
#define DEBUG_FLT_REC_DEFINE        FALSE    /**< Define                     */
#define DEBUG_FLT_REC_ONLINE        FALSE    /**< Online                     */
#define DEBUG_FLT_REC_HOT_SPARE     FALSE    /**< Hot spare                  */

#define DEBUG_FLT_REC_CDRV          FALSE    /**< Cdriver                    */
#define DEBUG_FLT_REC_CACHE         FALSE    /**< Cache                      */
#define DEBUG_FLT_REC_LINK          FALSE    /**< Link layer                 */
#define DEBUG_FLT_REC_VIRTUAL       FALSE    /**< Virtual layer              */
#define DEBUG_FLT_REC_RAID          FALSE    /**< RAID layer                 */
#define DEBUG_FLT_REC_DLM           FALSE    /**< DLM layer                  */
#define DEBUG_FLT_REC_PHY           FALSE    /**< Physical layer             */
#define DEBUG_FLT_REC_PHY_DISC      TRUE    /**< Physical discovery layer   */
#define DEBUG_FLT_REC_PHY_REC       TRUE    /**< Physical recovery          */
#define DEBUG_FLT_REC_ISP           FALSE    /**< ISP layer                  */
#define DEBUG_FLIGHTREC_FD          TRUE    /**< Fabric Discovery           */
#define DEBUG_FLT_REC_MISC          FALSE    /**< Misc layer                 */

#define DEBUG_FLT_REC_MEMORY        FALSE    /**< Malloc and mrel tracing    */
#define DEBUG_FLT_REC_XCHANG        FALSE    /**< Context switch timing      */
#define DEBUG_FLT_REC_TIME          FALSE   /**< Time stamping              */

#endif
/* @} */

/**
**  @name   Cache Options
**/
/* @{ */
#define WC_ENABLE                   TRUE    /**< FALSE = Write Cache not allowed
                                             **  TRUE  = WC can be enabled  */

#define RBI_DEBUG                   FALSE   /**< RBI Debug code             */
#define RBI_DEBUG_NODEMAX           FALSE   /**< RBI Node Max Debug Code    */

#define WC_MIRROR_ERROR_DISABLE     FALSE   /**< TRUE = Disable Errors being
                                             **  seen on Mirror Operations.
                                             **  FALSE = Enable Errors being
                                             **  seen on Mirror Operations. */

#define NO_REMOTE_MIRROR            FALSE   /**< Disables sending DGs to MP */
/* @} */

/**
**  @name   Initiator Options
**/
/* @{ */
#ifdef  BACKEND

/**
**  Back end
**/
#define MULTI_ID                FALSE   /**< Define multi-target support    */
#define INITIATOR               TRUE    /**< Initiator code always enabled
                                         **  on BE                          */
#define MAG_TO_MAG              TRUE    /**< MAGNITUDE to MAGNITUDE support */
#define ITRACES                 FALSE   /**< Always for the Backend         */
#define DLM_FE_DRIVER           FALSE   /**< Disable FE DLM Driver Tester   */

#else

/**
**  Front end
**/
#define MULTI_ID                TRUE    /**< Define multi-target support    */
#define INITIATOR               TRUE    /**< Initiator code for FE          */

    #if INITIATOR

    #define MAG_TO_MAG          TRUE    /**< TRUE = Enable SAN Links support */
    #define VALIDATE            TRUE    /**< Validate PSF calls (initiator function) */
    #define EX_COPY             FALSE   /**< EXTENDED COPY command support  */
    #define EX_COPY_TEST        FALSE   /**< EXTENDED COPY test support     */
    #define ITRACES             TRUE    /**< Initiator traces               */
    #define DLM_FE_DRIVER       FALSE   /**< Enable FE DLM Driver Tester    */

    #else
/**
** Do mot modify these values - these REQUIRE initiator support
**/
    #define MAG_TO_MAG          FALSE   /**< MAGNITUDE to MAGNITUDE support */
    #define VALIDATE            FALSE   /**< Validate PSF calls             */
    #define EX_COPY             FALSE   /**< EXTENDED COPY command support  */
    #define EX_COPY_TEST        FALSE   /**< EXTENDED COPY test support     */
    #define ITRACES             FALSE   /**< Enable Initiator traces        */
    #define DEBUG_TRACE_STOP_2  FALSE   /**< Disable trace when path is
                                         **  lost to self                   */
    #define DEBUG_TRACE_STOP_3  FALSE   /**< Disable trace when there
                                         **  are no more alpa'a             */
    #define DEBUG_TRACE_STOP_4  FALSE   /**< Disable traces when any
                                         **  target is lost                 */
    #define DEBUG_TRACE_STOP_5  FALSE   /**< Disable traces on port
                                         **  unavailable                    */
    #define DLM_FE_DRIVER       FALSE   /**< Disable FE DLM Driver Tester   */

    #endif

    #if ITRACES
    #define DEBUG_TRACE_STOP_2  FALSE   /**< Disable trace when path is
                                         **  lost to self                   */
    #define DEBUG_TRACE_STOP_3  FALSE   /**< Disable trace when there
                                         **  are no more alpa'a             */
    #define DEBUG_TRACE_STOP_4  FALSE   /**< Disable traces when any
                                         **  target is lost                 */
    #define DEBUG_TRACE_STOP_5  FALSE   /**< Disable traces on port
                                         **  unavailable                    */
    #else

    #define DEBUG_TRACE_STOP_2  FALSE   /**< Disable trace when path is
                                         **  lost to self                   */
    #define DEBUG_TRACE_STOP_3  FALSE   /**< Disable trace when there
                                         **  are no more alpa'a             */
    #define DEBUG_TRACE_STOP_4  FALSE   /**< Disable traces when any
                                         **  target is lost                 */
    #define DEBUG_TRACE_STOP_5  FALSE   /**< Disable traces on port
                                         **  unavailable                    */
    #endif

#endif
/* @} */

/**
**  @name   DLM module conditional assembly equates
**/
/* @{ */
#define MAGNITUDE               FALSE
/* @} */

/**
**  Online processing
**/
#define NO_DELAY_ONLINE         FALSE   /**< Do not wait for CCB in online
                                         **  processing                     */
#define DRIVE_POLLING           TRUE    /**< Periodically poll drives       */

/**
**  MAGDRVR/CDRIVER  processing
**/
#define CDRIVER_MSGS            TRUE

/**
**  MAGDRVR/CDRIVER module trace definitions -
**  This saves code trace data by channel #. The CIMT has pointers to the
**  trace area. See traces.h
**/
#ifndef PERF
#define TRACES                  TRUE    /**< Traces                         */
#endif  /* PERF */
#define DEFAULT_TRACE_FLAG      0x069E  /**< Default trace flag setting     */

/**
**  @name   MAGDRVR/CDRIVER module error log definitions
**/
/* @{ */
#define ERROR_LOG               FALSE       /**< Error log                  */
#define TRACE_ERROR_LOG         FALSE       /**< Save traces in error
                                             **  log record                 */
#define DEFAULT_ERROR_LOG_FLAG  0x0000000F  /**< Default error log flag
                                             **  setting                    */
/* @} */

/**
**  @name   Physical Options
**/
/* @{ */
#define PHY_LOG_RETRY           TRUE
/* @} */

/**
**  @name   ISP Options
**/
/* @{ */
#define ISP_CP_DESC_LID         TRUE    /**< Control port descending
                                         **  Loop ID search                 */
#define ISP_DISABLE_VPORTS      FALSE   /**<  Fail0over via disable VPs     */
#define ISP_DEBUG               TRUE
#define ISP_RESET_ERROR_TRAP    TRUE    /**< Enables errtrap on port reset  */
#define ISP_INIT_MSG            TRUE    /**< Log msg when port init ok      */
#ifdef  BACKEND
#define ISP_ERROR_INJECT        TRUE    /**< Enable Error Inject feature BE */
#else /* FRONTEND */
#define ISP_ERROR_INJECT        FALSE   /**< Disable Error Inject feature FE*/
#endif
/* @} */

/**
**  @name   Heartbeat Tracing Options
**/
/* @{ */
#define DEBUG_HBEAT_REC         TRUE        /**< Enable heart beat tracing  */
#define HBEAT_ALLOC_SIZE        (16 * 1024) /**< Sizeof heartbeat recorder  */
/* @} */

/**
**  @name   Doug K. & Larry D. Options
**/
/* @{ */

/**
**  Special INQUIRY data format option flags
**/
#define SPEC_INQ_NEW            TRUE    /**< New style special INQUIRY data
                                         **  format                         */

#if SPEC_INQ_NEW
#define SPEC_INQ_OLD            FALSE   /**< Old style special INQUIRY data */
#else
#define SPEC_INQ_OLD            TRUE    /**< Old style special INQUIRY data */
#endif

/**
**  Target-only mode flags
**/
#define DJK_TO_MODE             TRUE    /**< Target-only mode logic         */

/**
**  Start Performance Build, flags to be disabled
**/
#ifdef PERF

#undef  SW_FAULT_DEBUG
#define SW_FAULT_DEBUG          FALSE   /**< Software Detected Fault Debug  */

#undef  ERRTRAP_DEBUG
#define ERRTRAP_DEBUG           FALSE   /**< How to handle jumps to .errtrap*/

#undef  KERNEL_DEBUG_5
#define KERNEL_DEBUG_5          FALSE   /**< Check for ready tasks          */

#undef  ITRACES
#define ITRACES                 FALSE   /**< Disable Initiator traces       */
#undef  CDRIVER_MSGS
#define CDRIVER_MSGS            FALSE
#undef  PHY_LOG_RETRY
#define PHY_LOG_RETRY           FALSE
#undef  ISP_DEBUG
#define ISP_DEBUG               FALSE
#undef  ISP_RESET_ERRTRAP
#define ISP_RESET_ERRTRAP       FALSE   /**< Disable errtrap on port reset  */
#undef  ISP_ERROR_INJECT
#define ISP_ERROR_INJECT        FALSE   /**< Disable Error Inject feature   */
#undef  DEBUG_HBEATREC
#define DEBUG_HBEATREC          FALSE   /**< Disable heart beat tracing     */

#endif

#endif /* _OPTIONS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
