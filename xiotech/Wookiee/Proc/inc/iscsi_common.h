/* $Id: iscsi_common.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_common.h
 **
 **  @brief      This file has common utility functions related to memory
 **
 **  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/

#ifndef __ISCSI_COMMON_H
#define __ISCSI_COMMON_H

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define ISCSI_HDR_LEN               48

#define TARGOP_TASK_MGMT_RESP       0x22

#define LUN_MASK_0X3F               0x3f
#define DATA_LEN_MASK               0x00ffffff
#define AHS_LEN_MASK                0xff000000

/*
** Initiator opcodes (INIOP
*/
#define INIOP_NOP_OUT               0x00 /* Initiator OPcode NOP OUT */
#define INIOP_SCSI_CMD              0x01
#define INIOP_TASK_MGMT_REQ         0x02
#define INIOP_LOGIN_REQ             0x03
#define INIOP_TEXT_REQ              0x04
#define INIOP_SCSI_DATA_OUT         0x05
#define INIOP_LOGOUT_REQ            0x06
#define INIOP_SNACK_REQ             0x10
/*
** Target opcodes(TAROP
*/
#define TARGOP_NOP_IN                       0x20 /* Target OPcode NOP IN */
#define TARGOP_SCSI_RESP                    0x21
#define TARGOP_LOGIN_RESP                   0x23
#define TARGOP_TEXT_RESP                    0x24
#define TARGOP_SCSI_DATA_IN                 0x25
#define TARGOP_LOGOUT_RESP                  0x26
#define TARGOP_R2T                          0x31
#define TARGOP_ASYNC_MSG                    0x32
#define TARGOP_REJECT                       0x3f

#define TARGOP_LOGOUT_FLAGS                 0x80
#define TARGOP_REJECT_FLAGS                 0x80
#define TARGOP_RESERVE_BIT                  0x80
/*
** bif operations
*/
#define IMM_BIT                     0x40 /* Immediate bit           */
#define IMM_BIT_UNSET               0xbf /* to clear IMM bit to 0   */
#define F_BIT                       0x80 /* F bit pos               */
#define C_BIT                       0x40 /* C bit pos in text req   */
#define ISCSI_OPCODE                0x3f /* iSCSI opcode part       */
#define TR_BIT                      0x80 /* Transmit bit            */
#define TR_BIT_UNSET                0x7f
#define CSG_SHIFT                   2    /* for shifting CSG bits   */


#define XIO_SUCCESS                 1
#define XIO_FAILURE                 0
#define XIO_CONN_CLOSE              255

#define XIO_TRUE                    1
#define XIO_FALSE                   0

#define XIO_ONE                     1
#define XIO_ZERO                    0

/*
** TRACE related
*/

// #define TRACE_LEVEL                 3 /* in order to get all traces make it 5,0 for no trace */

#define TRACE_GERROR                4 /* General error e.g.,send/recv/malloc failures */
#define TRACE_ISCSI_WARNING         3 /* iSCSI related Warnings */
#define TRACE_ISCSI_ERR             2 /* iSCSI related Errors */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

#define xioMemcpy     memcpy
#define xioMemcmp     memcmp
#define xioStrcmp     strcmp
#define xioStrcpy     strcpy
#define xioStrlen     strlen
#define PRINT         fprintf

// #define TRACE_ENTRY_EXIT(level,args...)  PRINT(stderr,args);
#define TRACE_ENTRY_EXIT(level,args...)

// #define TRACE(level,args...)
//         if(level <= TRACE_LEVEL)
//         {
//             PRINT(stderr,"%s:%d", __FUNCTION__,__LINE__);
//             PRINT(stderr,args);
//         }

#define TR_PRINT          10
#define PR_ENABLE         10 /* 0 for not printing, 10 to print */
// #define TRACE_PRINT(level,args...)
//         if(level == TR_PRINT)
//         {
//             PRINT("%s:%d", __FUNCTION__,__LINE__);
//             PRINT(args);
//         }


#define IS_IMMBIT_SET(opcode)   ((opcode) & IMM_BIT)
#define IS_F_BIT_SET(opcode)       ((opcode) & F_BIT)
#define IS_C_BIT_SET(opcode)       ((opcode) & C_BIT)

#define IS_TR_BIT_SET(opcode)       ((opcode) & TR_BIT)
#define CLEAR_IMMBIT(opcode)       ((opcode) & IMM_BIT_UNSET)
#define GET_ISCSI_OPCODE(opcode) ((opcode) & ISCSI_OPCODE)
#define GET_CSG(opcode)        (((opcode) & CSG_OPCODE) >> CSG_SHIFT)
#define GET_NSG(opcode)        ((opcode) & NSG_OPCODE)
#define CLEAR_TR_BIT(opcode)     ((opcode) & TR_BIT_UNSET)
#define BIT_UNSET(param,bit)   ((param) & ~(1 << (bit)))

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern INT32 stringCompare( UINT8* s1, UINT8* s2);

#endif  /* __ISCSI_COMMON_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
