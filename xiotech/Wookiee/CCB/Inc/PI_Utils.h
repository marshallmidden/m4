/* $Id: PI_Utils.h 144042 2010-07-12 15:54:51Z m4 $ */
/*===========================================================================
** FILE NAME:       PI_Utils.h
** MODULE TITLE:    Packet Interface Utility Functions
**
** DESCRIPTION:     Server that handles packet communication between the
**                  CCB and the XMC.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _PI_UTILS_H_
#define _PI_UTILS_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

struct PCB;
struct MR_LIST_RSP;

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** Command record for Packet Interface commands.
*/
typedef struct _PI_COMMAND_RECORD
{
    UINT8       fence[16];      /* Fence to make easier to read             */
    /* QUAD */
    struct PCB *callerPCB;
    UINT32      timeout;        /* Timeout                                  */
    void       *commandBufferIn;
    void       *commandBufferOut;
    /* QUAD */
    UINT8       state;
    UINT8       completion;
    UINT8       timeoutAction;  /* Action on timeout with the outbuffer     */
    UINT8       rsvd35;         /* RESERVED                                 */
    UINT32      commandCode;
    UINT8       rsvd40[8];      /* RESERVED                                 */
    /* QUAD */
} PI_COMMAND_RECORD;

/*
** MRP command timeout value.  State definitions for state and completion.
*/
#define MRP_STD_TIMEOUT                 10000   /* 10 sec timeout   */
#define MRP_PROC_CODEBURN_TIMEOUT       60000   /* 60 sec timeout   */
#define MRP_UPDATE_DRIVE_BAY_TIMEOUT    600000  /* 10 min timeout   */
#define TMO_NONE                        0xFFFFFFFF      /* NO TIMEOUT       */


#define PI_COMMAND_RECORD_STATE_EXECUTING       0
#define PI_COMMAND_RECORD_STATE_TIMEOUT         1
#define PI_COMMAND_RECORD_STATE_COMPLETE        2

#define PI_COMMAND_RECORD_COMPLETION_COMPLETE   0
#define PI_COMMAND_RECORD_COMPLETION_TIMEOUT    1

#define PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_FREE    0
#define PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE    1

/*
** PHGetList() can either return just the "header" information with the number
** of devices or the entire list of devices.  A flag is ORed with the
** MRP function code to form the listType input to PHGetList().
*/
#define GET_NUMBER_ONLY     0x0000
#define GET_LIST            0x8000
#define MRP_MASK            0x7FFF

/*
** Number of entries in the command record table (exported below)
*/
#define COMMAND_RECORD_TABLE_SIZE   1000        /* number of saved entries */

/*****************************************************************************
** Public variables
*****************************************************************************/
extern const char *procName[3];
extern PI_COMMAND_RECORD commandRecordTable[];

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_ExecMRP
**
** Description: This function handles everything necessary to send a
**              command to the TBolt (with a designated timeout).
**
**              The high bit on 'timeout_mS' indicates an initialization
**              request.  Initialization requests occur at power up to
**              determine when a processor finally comes ready.
**
**              PI_ExecMRP rules:
**                - INPUT PACKET
**                   - When caller is done with the input packet
**                     free using "free"
**                   - If input packet contains a PCI address and
**                     the MRP timed out the PCI buffer must be
**                     freed using "DelayedFree".
**                - OUTPUT PACKET
**                   - If the MRP return code is TIMEOUT the output
**                     packet must not be freed.  The free will be
**                     done by the MRP Timeout handling.
**                   - If the MRP return code is not TIMEOUT the
**                     caller must free the output packet using
**                     "free".
**
** Inputs:      a == void* commandBufferIn
**              b == UINT32 commandBufferInSize,
**              c == UINT32 commandCode,
**              d == void* commandBufferOut,
**              e == UINT32 commandBufferOutSize,
**              f == UINT32 timeoutMS
**
** Returns:     PI_GOOD, PI_ERROR, PI_TIMEOUT or one of the other
**              PI_ return codes.
**
** WARNING:     There are rules that need to be followed for the buffers
**              passed into this function.  The rules are stated above
**              and must be followed to ensure system integrity.
**
** NOTE:        Check RC on all malloc() when this function is re-written.
**--------------------------------------------------------------------------*/
#define PI_ExecMRP(a,b,c,d,e,f)     PI_ExecuteMRP(a,b,c,d,e,f,PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_FREE)

/*----------------------------------------------------------------------------
** Function:    PI_ExecuteMRP
**
** Description: This function handles everything necessary to send a
**              command to the TBolt (with a designated timeout).
**
**              The high bit on 'timeout_mS' indicates an initialization
**              request.  Initialization requests occur at power up to
**              determine when a processor finally comes ready.
**
**              PI_ExecuteMRP rules:
**                - INPUT PACKET
**                   - When caller is done with the input packet
**                     free using "free"
**                   - If input packet contains a PCI address and
**                     the MRP timed out the PCI buffer must be
**                     freed using "DelayedFree".
**                - OUTPUT PACKET
**                   - If the MRP output packet is statically allocated, then
**                     timeoutAction must equal
**                         PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE
**                   - If the MRP output packet is dynamically allocated, then
**                     timeoutAction must equal
**                         PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_FREE
**                   - If the MRP return code is TIMEOUT the output
**                     packet must not be freed.  The free will be
**                     done by the MRP Timeout handling.
**                   - If the MRP return code is not TIMEOUT and
**                     the output packet was dynamically allocated,
**                     then the caller must free the output packet using
**                     "free".
**
** Inputs:      void* commandBufferIn
**              UINT32 commandBufferInSize,
**              UINT32 commandCode,
**              void* commandBufferOut,
**              UINT32 commandBufferOutSize,
**              UINT32 timeoutMS,
**              UINT8  timeoutAction)
**
** Returns:     PI_GOOD, PI_ERROR, PI_TIMEOUT or one of the other
**              PI_ return codes.
**
** WARNING:     There are rules that need to be followed for the buffers
**              passed into this function.  The rules are stated above
**              and must be followed to ensure system integrity.
**
** NOTE:        Check RC on all malloc() when this function is re-written.
**--------------------------------------------------------------------------*/
extern INT32 PI_ExecuteMRP(void *commandBufferIn,
                           UINT32 commandBufferInSize,
                           UINT32 commandCode,
                           void *commandBufferOut,
                           UINT32 commandBufferOutSize,
                           UINT32 timeoutMS, UINT8 timeoutAction);

extern UINT16 numDevs_From_listType(UINT16);

extern struct MR_LIST_RSP *PI_GetList(UINT16 startID, UINT16 listType);

extern void InitProcessorComm(void);
extern bool ProcessorCommReady(INT32 processor);

extern void DelayedFree(UINT32 mrpCmd, void *ptr);

extern INT32 UnwindStackCCB(struct PCB *, char *);

extern bool BEBlocked(void);
extern bool FEBlocked(void);

extern INT32 ProcessorQuickTest(UINT32 mrpCmd);

extern void SetGlobalMRPTimeout(UINT32 timeout);
extern UINT32 GetGlobalMRPTimeout(void);

extern void SetGlobalIPCTimeout(UINT32 timeout);
extern UINT32 GetGlobalIPCTimeout(void);
extern UINT8 GetControllerType(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _PI_UTILS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
