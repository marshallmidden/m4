/* $Id: def.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       def.h
**
**  @brief      DEFine descriptions
**
**      To provide a common means of platform packet definitions,
**      including those commands issued to the DEFINE module by the CCB.
**
**  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef DEF_H
#define DEF_H

#include "XIO_Types.h"

#include "ilt.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "DEF_iSCSI.h"
#include "sdd.h"
#include "target.h"

/*
** Event broadcast constants
*/
#define EB_SOS_SUB          0       /**< SOS subevent                        */
#define EB_SCMT_SUB         1       /**< SCMT subevent                       */
#define EB_LOCAL_IMG_SUB    2       /**< Put local image                     */
#define EB_DG_SUB           3       /**< Datagram Subevent                   */
#define EB_FILE_SYS_SUB     4       /**< File system report                  */
#define EB_LDD_SUB          5       /**< Send LDD Subevent                   */
#define EB_ISID_SUB         6       /**< Send iSCSI server WWN Subevent      */
                                    /**< 7 is used by GR code  - Raghu       */
#define EB_PRR_SUB          8       /**< Send PRR Subevent                   */

#define EB_SOS_SIZE         32      /**< 32 bytes for SOS                    */
#define EB_SCMT_SIZE        52      /**< 52 bytes for SCMT                   */
#define EB_DG_SIZE          52      /**< 52 bytes for DataGram               */
#define EB_FILE_SYS_SIZE    48      /**< 48 bytes for FSYS                   */

#define EB_TO_MASTER        0x0001  /**< Broadcast to master                 */
#define EB_TO_SLAVE         0x0002  /**< Broadcast to slaves other than self */
#define EB_TO_SELF          0x0004  /**< Broadcast to self if slave          */
#define EB_TO_SPEC_CTRL     0x0008  /**< Broadcast to specific controller    */
#define EB_TO_OTHERS        0x0010  /**< Broadcast to all others (not self)  */

#define EB_TO_ALL           0x0007  /**< Broadcast to all controllers        */

/*
** Event broadcast structure
*/
typedef struct LOG_IPC_BROADCAST_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT16          subEvent;       /**< Subevent type                      */
    UINT16          bcastType;      /**< Broadcast Type                     */
    UINT32          serialNum;      /**< Serial number to send to           */
    UINT32          dataSize;       /**< Size of data to follow             */
    UINT8           data[0];        /**< Data Packet                        */
} LOG_IPC_BROADCAST_PKT;

/**
**  NewServer log request structure
**/
typedef struct LOG_ZONE_INQUIRY_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT8   channel;        /**< Channel this server is on                  */
    UINT8   rsvd;           /**< RESERVED                                   */
    UINT16  targetId;       /**< Target ID                                  */
    UINT32  owner;          /**< Owner                                      */
    UINT64  wwn;            /**< World wide name                            */
    UINT8   i_name[256];    /**< iSCSI Server name                          */
} LOG_ZONE_INQUIRY_PKT;

/*
**  ISP Chip reset , Port init failure ------------------------------
*/
#define ISP_RESET_AND_INIT          0       /**< Reset and initialize       */
#define ISP_RESET_ONLY              1       /**< Reset only                 */
#define ISP_RESET_INIT_OFFLINE      2       /**< Reset and init if offline  */
#define ISP_RESET_ONLY_OFFLINE      3       /**< Reset only if offline      */
#define ISP_RESET_AND_INIT_LOG      4       /**< Reset and initialize       */
#define ISP_RESET_ONLY_LOG          5       /**< Reset only                 */
/* NOTE:  Log events are generated for values > 3                           */
#define ISP_INIT_OK                 0x0000  /**< Interface init completed OK*/

#define ISP_FW_HBEAT_FAIL           0x0101  /**< Firmware Heartbeat failure */
#define ISP_CMD_TIME_OUT            0x0102  /**< Command timeout            */
#define ISP_FATAL_ERROR             0x0103  /**< Fatal Error                */
#define ISP_ASYNC_Q_OVERFLOW        0x0104  /**< Async queue overlow        */
#define ISP_BAD_IOCB_TYPE           0x0105  /**< Bad IOCB type              */
#define ISP_NO_ILT_IN_IOCB          0x0106  /**< No ILT in IOCB             */
#define ISP_NO_ILT_CMPL             0x0107  /**< No ILT completion routine  */
#define ISP_ILT_THREAD_ERROR        0x0108  /**< ILT unthread error         */
#define ISP_REQ_Q_PTR_ERROR         0x0109  /**< Req Q IN ptr out of range  */
#define ISP_RESP_Q_PTR_ERROR        0x010A  /**< Resp Q OUT ptr out of range */
#define ISP_MAILBOX_TIMEOUT         0x010B  /**< Mailbox timeout            */
#define ISP_INV_GAN_PORT_ID         0x010C  /**< Bad Port ID in GAN rsp     */
#define ISP_RISC_PAUSED             0x010D  /**< ISP RISC PAUSED     */

#define ISP_BAD_DEV                 0x0201  /**< Bad device                 */
#define ISP_IF_DEAD                 0x0202  /**< Interface alive failed     */
#define ISP_MBOX_TEST_FAIL          0x0203  /**< Mailbox test failed        */
#define ISP_NO_FW_FOUND             0x0204  /**< No firmware found          */
#define ISP_LOAD_RAM_ERROR          0x0205  /**< Load Ram command failed    */
#define ISP_FW_CHK_SUM_ERROR        0x0206  /**< Vfy firmware chksum failed */
#define ISP_ABOUT_FW_ERROR          0x0207  /**< About firmware cmd failed  */
#define ISP_INIT_MBOX_ERROR         0x0208  /**< Init mailbox command failed*/
#define ISP_SET_FW_OPT_ERROR        0x0209  /**< Set firmware opt cmd failed*/
#define ISP_EXEC_FW_ERROR           0x020A  /**< Execute firmware cmd failed*/

#define ISP_NO_LOOP_ERROR           0x0301  /**< No Loop after power on     */
#define ISP_LOOP_DOWN_RETRY         0x0302  /**< Loop down retry            */
#define ISP_LOOP_DOWN_RETRY_ERROR   0x0303  /**< Loop down retry failed     */

#define ISP_LOOP_UP                 0x0400  /**< Loop Up                    */

/*
**  Begin constants for virtual disk control (REDI Copy) operations.
*/
#define VDC_BREAK_SPEC_COPY         5       /**< Break off specified copy   */
#define VDC_PAUSE_COPY              6       /**< Pause a copy               */
#define VDC_RESUME_COPY             7       /**< Resume a copy              */
#define VDC_ABORT_COPY              8       /**< Abort a copy               */
#define VDC_UPDATE                  9       /**< Update % complete and state*/

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern SDD* DEF_AllocServer(void);
extern TGD* DEF_AllocTarg(void);
extern void DEF_HashLUN(SDD* pSDD, UINT32 lun, UINT32 vid);
extern UINT32 DEF_WWNLookup(UINT64 wwn, UINT16 tid, UINT32 newServer, UINT8 *i_name);
extern void DEF_RmtWait(void);
extern void DEF_RelSDDLVM(SDD* pSDD);

#ifdef FRONTEND
extern UINT8 DEF_UpdateTgInfo(MR_PKT* pMRP);
extern UINT8 DEF_SetChapInfo(MR_PKT* pMRP);
extern UINT8 iSCSIAddUser(MRCHAPCONFIG userInfo);
extern UINT8 iSCSIRemoveUser(MRCHAPCONFIG userInfo);
extern UINT8 DEF_iSNSConfig(MR_PKT *pMRP);
extern void iSNS_Update(void);
#endif /* FRONTEND */

#ifdef BACKEND
extern void DEF_CreateTargetInfo(TGD *pTGD, I_TGD *iTGD);
extern void DEF_CreateChapUserInfo(TGD* pTGD, CHAPINFO* pUserInfo);
extern UINT8 DEF_SetTgInfo(MR_PKT* pMRP);
extern UINT8 DEF_GetTgInfo(MR_PKT* pMRP);
extern void DEF_UpdRmtTgInfo(TGD *pTGD);
extern UINT8 DEF_UpdSID(MR_PKT* pMRP);
extern void iSCSIAddUserBE(MRCHAPCONFIG userInfo);
extern void iSCSIRemoveUserBE(MRCHAPCONFIG userInfo);
extern UINT8 DEF_SetChap(MR_PKT* pMRP);
extern UINT8 DEF_UpdChapInfo(I_TGD* pITGD);
extern UINT8 DEF_GetChap(MR_PKT* pMRP);
extern UINT8 DEF_iSNSSetInfo(MR_PKT *pMRP);
extern UINT8 DEF_iSNSUpdateFE (void);
extern UINT8 DEF_GetIsnsInfo(MR_PKT *pMRP);

#endif /* BACKEND */
extern void D$que(UINT32 dummy, ILT *pILT);

#endif /* DEF_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
