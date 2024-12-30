/* $Id: DEF_SOS.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       DEF_SOS.h
**
**  @brief      Segment Optimization Structure
**
**  To provide a common means of defining the Segment Optimization
**  Structure.
**
**  Copyright (c) 1996-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DEF_SOS_H_
#define _DEF_SOS_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name   Segment Optimization Structure - flag field bit definitions
**  @{
**/
#define SOS_CANCEL          0   /**< Cancel this defrag                     */
#define SOS_STEP_DONE       1   /**< This step was completed                */
#define SOS_STEP_REC        2   /**< This step was recorded                 */
#define SOS_RESTART         3   /**< Restart the SOS                        */
#define SOS_TIMER_DELAY     4   /**< Delaying due to defrag conflict        */
#define SOS_ACTIVE          5   /**< This SOS is active on this controller  */
#define SOS_PREPARED        6   /**< This SOS has been prepared             */
#define SOS_TERMINATE       7   /**< Cannot continue defragmentation        */
#define SOS_INCOMPLETE      8   /**< Error in processing of read/writes     */
#define SOS_PREP_FAIL       9   /**< Prepare for failover                   */
/* @} */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

typedef struct SOS
{
    struct SOS *next;           /**< Next entry in SOS list                 */
    UINT16      pid;            /**< PID for defrag drive                   */
    UINT16      flags;          /**< Flags (see above)                      */
    UINT32      remain;         /**< Blocks remaining                       */
    UINT32      total;          /**< Blocks total to be done                */
                                /**< QUAD BOUNDARY                      *****/
    struct PDD *pdd;            /**< PDD being defragmented                 */
    UINT32      asda;           /**< Alternate starting disk address        */
    UINT16      count;          /**< Count of entries                       */
    UINT16      current;        /**< Current entry number                   */
    struct PCB *pcb;            /**< PCB of driver task                     */
                                /**< QUAD BOUNDARY                      *****/
} SOS;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _DEF_SOS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
