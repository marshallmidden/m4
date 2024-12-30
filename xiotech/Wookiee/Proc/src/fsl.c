/* $Id: fsl.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       fsl.c
**
**  @brief      This is a Front-end Sub-Layer for iSCSI
**
**  Copyright (c) 2005-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "def.h"
#include "fsl.h"
#include "ilt.h"
#include "isp.h"
#include "LOG_Defs.h"
#include "cdriver.h"
#include "ecodes.h"
#include "magdrvr.h"
#include "misc.h"
#include "pm.h"
#include "portdb.h"
#include "system.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "xli.h"
#include "xl.h"
#include "icimt.h"
#include "icl.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <byteswap.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <scsi/sg.h>
#include <limits.h>

#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "iscsi_tsl.h"
#include "iscsi_tmf.h"
#include "loop.h"
#include "prociscsid.h"
#include "tmt.h"
#include "lsmt.h"
#include "ltmt.h"
#include "mem_pool.h"
#include "CT_defines.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
/*
** For iscsi to scsi interface
*/
#define     R_BIT                           0x40
#define     W_BIT                           0x20
#define     RW_BIT                          0x60

/*
** Task attributes for iSCSI and SAM
*/
#define     IS_UNTAGGED                     0x00
#define     IS_SIMPLE                       0x01
#define     IS_ORDERED                      0x02
#define     IS_HOQ                          0x03
#define     IS_ACA                          0x04

#define     SAM_SIMPLE                      0x00
#define     SAM_HOQ                         0x01
#define     SAM_ORDERED                     0x02
#define     SAM_ACA                         0x04
#define     SAM_UNTAGGED                    0x0

#define     IFSL_DISC_PRI                   150
#define     I_NAA_DOT                       "naa."
#define     I_EUI_DOT                       "eui."

#define    EC_CONN_ERROR                   0xabcde
typedef struct ISCSI_PARAMS
{
    struct ISCSI_PDU     *pPdu;     /* w0 */
    struct CONNECTION    *pConn;    /* w1 */
    struct SGL           *pSgl;     /* w2 */
    UINT8                *pCdb;     /* w3 */
    UINT16               tsih;      /* w4 */
    UINT16               rsvd1;     /* w4 */
    UINT16               lun;       /* w5 */
    UINT16               tpgt;      /* w5 */
    UINT8                cdbLen;    /* w6 */
    UINT8                flags;     /* w6 */
    UINT16               cmdType;   /* w6 */
    UINT16               portId;    /* w7 */
    UINT16               rsvd2;     /* w7 */
}ISCSI_PARAMS;

#ifndef SG_FLAG_DIRECT_ONLY
#  define SG_FLAG_DIRECT_ONLY 8
#endif

extern void convertIntoReadableData(UINT64 nodeName,UINT8 *readableFormat);
extern void tsl_cleanupEvents(void * pIDD);

void    I_recv_offline(UINT32 port);
UINT32  fsl_iscsidTxRx(request *pReq, response *pRsp);
void    fsl_buildReq(IDD *pIDD, request *pReq);
/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
void fsl_tlogout(UINT32 a , UINT32 b , IDD *pIDD);
void fsl_tlogin(UINT32 a , UINT32 b , ILT *pILT);
void fsl_tmonitor(void);

#define PORT_WWN(tid,port) (T_tgdindx[(tid)]->nodeName | bswap_64((UINT64)(WWN_F_PORT | (port << 16)) << 32))

/*
******************************************************************************
** Public functions - NOT externed in a header file
******************************************************************************
*/
extern void     isp_build_targets(UINT8);
extern void     ISP_LoopDown(UINT32);
extern void     I_recv_rinit(UINT32 port);
extern void     C_recv_scsi_io(ILT *ilt);

extern void     CT_LC_isp_loopEventHandler(int);
extern void     CT_LC_fsl_tlogin(void *);
extern void     CT_LC_fsl_tlogout(void *);
extern void     CT_LC_fsl_tmonitor(void);
extern void     CT_LC_ISCSI_LoopUp(UINT8);

extern INT32    convertTargetNameInTo64bit(UINT8 *string,INT32 size, UINT64 *pTargetName);
extern void     KernelDispatch (UINT32 returnCode, ILT *pILT,  void *pPtr, UINT32 w0);
extern void     isp_thread_ilt(UINT8, ILT *);
extern ILT     *isp_unthread_ilt(UINT8, ILT *);

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern ICIMT   *I_CIMT_dir[MAX_ICIMT];
extern TGD     *T_tgdindx[MAX_TARGETS];
extern SDD     *S_sddindx[MAX_SERVERS];

extern CA       C_ca;                       /* Cache Structure */
extern PCB     *isplepcb[MAX_PORTS];        /* Port loop event PCB + ICL */
extern UINT32   LoopUp [MAX_PORTS + MAX_ICL_PORTS];

IDD    *gIDX[MAX_PORTS+MAX_ICL_PORTS][MAX_DEV];
PCB    *gMonTask = NULL;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void  fsl_LogEvent(UINT8 port, UINT32 event, UINT32 reason);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Initialize all the iSCSI structures pertaining to the port.
**
**              This function is called during bootup. Allocate and
**              initialize all the required structures for iSCSI.
**
**  @param      UINT8  - Port
**
**  @return     none
**
**  @attention  This is a task
**
******************************************************************************
**/
void fsl_Init(UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port)
{
    UINT32 i = 0;

    /*
    ** Wait for the define to send over the system information.
    */
    while (!(BIT_TEST(K_ii.status,II_SN))
            || !(BIT_TEST(K_ii.status,II_SERVER))
            || !(BIT_TEST(K_ii.status,II_ISCSI))
            || !(BIT_TEST(K_ii.status,II_VDMT)))
    {
        TaskSleepMS(125);
    }

    tsl_if_set(port, MY_IFUP);              /* Bring up the port            */
    BIT_CLEAR(ispofflfail, port);
    BIT_CLEAR(ispfail, port);               /* clear the fail bit           */

    if(lpmap[port] == NULL)
    {
        lpmap[port] = (UINT8 *)s_MallocC(LOOP_MAP_SIZE, __FILE__, __LINE__);
    }
    for(i = 0; i < LOOP_MAP_SIZE; i++)
    {
        lpmap[port][i] = 0xff;
    }
    /*
    ** Initialize the port
    */
    fsl_InitPort(port);

    BIT_CLEAR(resilk, port);        /* Clear the port reset interlock bit   */

}/* fsl_Init */

/**
******************************************************************************
**
**  @brief      Initialize the Tar structure from the Tgd configuration record.
**
**              Memory is allocated for a Tar strucrture if needed.
**              The Tar structure is initialized taking into account
**              clustered targets.
**
**  @param      UINT8  - Port
**
**  @return     none
**
******************************************************************************
**/
extern void ISCSI_LoopUp (UINT8 port);

UINT32 fsl_InitPort(UINT8 port)
{
    TAR *pTar;
    UINT32 retVal = 0;

#if ICL_DEBUG
    if (port == ICL_PORT)
    {
        fprintf(stderr,"<%s:%s> ICL port\n",__FILE__, __func__);
    }
#endif

    BIT_CLEAR(ispOnline, port);             /* Set port offline             */
    BIT_CLEAR(isprena, port);               /* Clear the port init bit      */

    /*
     * Build the list of targets that will be used to
     * configure this instance of the Qlogic chip.
     */
    isp_build_targets(port);

    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        pTar->portID = 0x0;
        pTar->tsih = 1;
        if (BIT_TEST(ispCp, port))
        {
#if ICL_DEBUG
            if(ICL_IsIclPort(port))
            {
                fprintf(stderr,"<%s:%s>ICL port - now it is control port\n",__FILE__, __func__);
            }
#endif
            BIT_SET(pTar->opt, TARGET_ENABLE);
            BIT_CLEAR(pTar->opt, TARGET_ACTIVE);
            BIT_CLEAR(pTar->opt, TARGET_SESSIONS);
            pTar->ipAddr = 0x0;
            BIT_SET(isprena, port);             /* Set the port initialized bit */

            if(port == ICL_PORT)
            {
                ICL_LogEvent(LOG_ICLPORT_INIT_OK);
            }
            else
            {
                fsl_LogEvent(port, LOG_PORT_UP, ISP_INIT_OK);
            }
            tsl_if_set(port, MY_IFUP);
            TaskSleepMS(100);

            /* Wait if ISCSI_LoopUp process is being created. */
            while (LoopUp[port] == (UINT32)-1)
            {
                TaskSleepMS(50);
            }

            if(tsl_IsPortUp(port) && LoopUp[port] == 0)
            {
                void *tmp_addr;
                CT_fork_tmp = (unsigned long)"ISCSI_LoopUp";
                LoopUp[port] = (UINT32)-1;      // Flag task being created.
                tmp_addr = TaskCreate3(C_label_referenced_in_i960asm(ISCSI_LoopUp), 32, port);
                LoopUp[port] = (UINT32)tmp_addr;
            }
            return(retVal);
        }
        else if ( (BIT_TEST(iscsimap, port)) || (ICL_IsIclPort(port)))
        {
            /* Set the Target as enabled */
            BIT_SET(pTar->opt, TARGET_ENABLE);

            /*
             * Invoke the iSCSI & tsl init for the port here...
             * on successful initialization, set the target as enabled.
             */
            if((pTar->tid < MAX_TARGETS) && (pTar->ipAddr != 0x0))
            {
                tsl_addAddr(port, pTar->ipAddr,pTar->tid,pTar->ipPrefix);
                pTar->vpID = pTar->tid;
                ispPortAssignment[pTar->tid] = port;
            }
        }
        else
        {
            /* VERY BAD - Notify the CCB - TBD */
            BIT_CLEAR(pTar->opt, TARGET_ENABLE);
            retVal = 1;
        }
    }

    /* All done. Send a log message for initialization completed successfully */
    if (retVal == 0)
    {
        tsl_if_set(port, MY_IFDOWN);        /* Bring down the port          */
        BIT_SET(isprena, port);             /* Set the port initialized bit */
        tsl_if_set(port, MY_IFUP);          /* Bring up the port            */

        /* Wait if isp_loopEventHandler process is being created. */
        while (isplepcb[port] == (PCB *)-1)
        {
            TaskSleepMS(50);
        }

        /*
         * Fork process to waits for the loop to
         * come online following initialization.
         * We don't need to handle any loop events/port events for ICL port
         */
        if ((ICL_PORT != port) && ( isplepcb[port] == NULL) )
        {
            CT_fork_tmp = (unsigned long)"isp_loopEventHandler";
            isplepcb[port] = (PCB *)-1;     // Flag task being created.
            isplepcb[port] = TaskCreate3(C_label_referenced_in_i960asm(isp_loopEventHandler),
                            K_xpcb->pc_pri+1, port);

            /* Context switch to allow this process to start running.  */
            TaskSwitch();
        }

        if(port == ICL_PORT)
        {
            ICL_LogEvent(LOG_ICLPORT_INIT_OK);
        }
        else
        {
            fsl_LogEvent(port, LOG_PORT_UP, ISP_INIT_OK);
        }
    }
    else
    {
        if(port == ICL_PORT)
        {
            tsl_if_set(port, MY_IFDOWN);        /* Bring down the port      */
            ICL_LogEvent(LOG_ICLPORT_INIT_FAILED);
        }
        else
        {
            BIT_SET(ispfail, port);             /* set the fail bit           */
            BIT_SET(ispofflfail, port);         /* set the fail bit           */
            tsl_if_set(port, MY_IFDOWN);        /* Bring down the port        */
            fsl_LogEvent(port, LOG_PORT_INIT_FAILED, retVal);
        }
    }

    return retVal;
}/* fsl_InitPort */


/**
******************************************************************************
**
**  @param      UINT8  - Port
**
******************************************************************************
**/
UINT32 fsl_ResetPort(UINT8 port, UINT32 reason)
{
    TAR *pTar;
    ILT *ilt;
    UINT32 retVal = 0;

    if (iscsimap == 0 && !(ICL_IsIclPort(port)))
    {
        return 0;
    }

    if (BIT_TEST(resilk, port))
    {
        return 0;
    }

    switch (reason)
    {
        case ISP_RESET_AND_INIT_LOG:        /**< Reset and initialize       */
        case ISP_RESET_ONLY_LOG:            /**< Reset only                 */
        {
            /* Send a log message */
            if(ICL_PRT(port))
            {
                ICL_LogEvent(LOG_ICLPORT_CHIPRESET);
            }
            else
            {
                fsl_LogEvent(port, LOG_ISP_CHIP_RESET, reason);
            }
        }
        break;

        case ISP_RESET_INIT_OFFLINE:        /**< Reset and init if offline  */
        case ISP_RESET_ONLY_OFFLINE:        /**< Reset only if offline      */
        {
            if(BIT_TEST(ispOnline, port))
            {
                return (1);
            }
        }
        case ISP_RESET_AND_INIT:            /**< Reset and initialize       */
        case ISP_RESET_ONLY:                /**< Reset only                 */
        default:
        break;
    }

    /* Set the port reset interlock bit */
    BIT_SET(resilk, port);

    /*
     * Generate LOOP DOWN to the proc and handle all the out-standing requests
     * for this port
     */
    if(ICL_IsIclPort(port))
    {
        ICL_LoopDown(port);
    }
    else
    {
        ISP_LoopDown((UINT32)port);
    }

    /*
     * Generate reset/init event to the proc so that all the out-standing IMTs
     * are deleted & also call the corresponding function in the initiator
     * path to delete the IMTs on this port
     */
    ilt = get_ilt();                               /* get an ILT w/wait                */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    ilt->ilt_normal.w0 = ISP_RESET_INIT_CMD;       /* store command byte               */
    ilt->ilt_normal.w0 |= port << 8;               /* store Chip ID                    */
    ilt->ilt_normal.w1 = 0;                        /* store Initiator ID               */
    ilt->ilt_normal.w2 = 0;                        /* store Initiator ID               */
    ilt[1].misc = (UINT32) ilt;
    ilt->cr = NULL;                                /* No completion handler            */
    ++ilt;                                         /* Get next level of ILT            */

    /* No completion routine, cdriver will release ilt */
    ilt->cr = NULL;                                /* No completion handler            */
    C_recv_scsi_io(ilt);

    /* If the tar structure changes on us, we might have to restart this operation. */
  restart:
    BIT_CLEAR(isprena, port);

    /* Reset all the targets on this port */
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        /* Remove the Target IP address from the port. */
        if (pTar->tid < MAX_TARGETS && pTar->ipAddr != 0x0)
        {
            /* NOTE: iscsi_SessionCleanup may task switch. */
            UINT32 save_tar_link_abort = tar_link_abort[port];

            ispPortAssignment[pTar->tid] = port;

            iscsi_SessionCleanup(pTar);            /* May task switch */

            /* If tar list got smaller, then the tar list may not be valid, restart. */
            if (save_tar_link_abort != tar_link_abort[port])
            {
                goto restart;
            }
            ispPortAssignment[pTar->tid] = 0xff;
        }

    }

    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        /* Remove the Target IP address from the port. */
        if (pTar->tid < MAX_TARGETS && pTar->ipAddr != 0x0)
        {
            ispPortAssignment[pTar->tid] = port;
            tsl_delAddr(port, pTar->ipAddr, pTar->tid,pTar->ipPrefix,pTar->ipGw);
            pTar->ipAddr = 0x0;
            ispPortAssignment[pTar->tid] = 0xff;
        }

        /* Indicate target is disabled. */
        BIT_CLEAR(pTar->opt, TARGET_ENABLE);
    }

    /* If reason requires initializing the port, do so */
    switch (reason)
    {
        case ISP_RESET_INIT_OFFLINE:        /**< Reset and init if offline  */
        case ISP_RESET_AND_INIT_LOG:        /**< Reset and initialize       */
        case ISP_RESET_AND_INIT:            /**< Reset and initialize       */
        {
#if INITIATOR
            /* Invoke Initiator level */
            I_recv_rinit(port);
#endif
            retVal = fsl_InitPort(port);
        }
        break;

        case ISP_RESET_ONLY:                /**< Reset only                 */
        case ISP_RESET_ONLY_LOG:            /**< Reset only                 */
        case ISP_RESET_ONLY_OFFLINE:        /**< Reset only if offline      */
        default:
        break;
    }

    /* Set the port reset interlock bit */
    BIT_CLEAR(resilk, port);

    return (retVal);
}/* fsl_ResetPort */

/**
******************************************************************************
**
**  @brief      Check if the target is an iSCSI target
**
**              Takes target id as the input param
**
**  @param      UINT16 tid - target id
**
**  @return     UINT8:  TRUE -  if iSCSI target
**                      FALSE - otherwise
**
******************************************************************************
**/
UINT8 fsl_TargetType(UINT16 tid)
{
    UINT8 retVal = FALSE;

    if(iclPortExists && (ICL_TARGET(tid)))
    {
        retVal = TRUE;
    }
    else
    {
        if(ispPortAssignment[tid] < MAX_PORTS)
        retVal = (BIT_TEST(iscsimap, ispPortAssignment[tid]));
    }
    return retVal;
}/* fsl_TargetType */

/**
******************************************************************************
**
**  @brief      Check if the port is an iSCSI port
**
**              Takes port number as the input param
**
**  @param      UINT8 port - port number
**
**  @return     UINT8:  TRUE -  if iSCSI target
**                      FALSE - otherwise
**
******************************************************************************
**/
UINT8 fsl_PortType(UINT8 port)
{
    return (BIT_TEST(iscsimap, port));
}/* fsl_PortType */

/**
******************************************************************************
**
**  @brief      Check if the port is an iSCSI port
**
**              Takes port number as the input param
**
**  @param      UINT8 port - port number
**
**  @return     UINT8:  TRUE -  if iSCSI target
**                      FALSE - otherwise
**
******************************************************************************
**/
void fsl_SrvLogin(UINT16 tid, UINT16 tsih, UINT64 wwn)
{
    UINT16 sid = 0;
    IMT *pIMT = NULL;
    IMT *pPrev = NULL;
    CIMT *pCIMT = NULL;
    UINT8 *i_name = NULL;
    UINT32 maxPorts = MAX_PORTS;

    if(iclPortExists)
    {
        maxPorts = MAX_PORTS+MAX_ICL_PORTS;
    }

    if (ICL_TARGET(tid))
    {
        fprintf(stderr,"ISCSI_DEBUG: %s: ICL- tid(%d) tsih(%d) sid(%llx)\n", __func__, tid,tsih,wwn);
    }
    else
    {
        fprintf(stderr,"ISCSI_DEBUG: %s: tid(%d) tsih(%d) sid(%llx)\n", __func__, tid,tsih,wwn);
    }

    if((i_name = iSCSI_GetSrvName(tsih, tid)) == NULL)
    {
        fprintf(stderr,"ISCSI_DEBUG: %s: EXIT tid(%d) tsih(%d) sid(%llx)\n", __func__, tid,tsih,wwn);
        return;
    }
    /*
    ** check to see if this is xioInitiator
    */
    if(fsl_is_xioInit(i_name))
    {
        /*
        ** This is a xiotech initiator
        */
        UINT16 i = 0;
        IDD *pIDD = NULL;
        UINT16 port = ispPortAssignment[tid];

        convertTargetNameInTo64bit(i_name,20, &wwn);
        fprintf(stderr,"ISCSI_DEBUG: %s: iSCSI name <> wwn = %llx\n", __func__,wwn);

        for(i = 0; i < MAX_DEV; i++)
        {
            if ((pIDD = gIDX[port][i]) != NULL &&
                T_tgdindx[tid] &&
                pIDD->i_name == PORT_WWN(tid,port) &&
                pIDD->t_pname == wwn &&
                pIDD->flags == (1 << IDF_READY))
            {
                BIT_SET(pIDD->flags, IDF_RETRY);
                fsl_updatePaths();
                break;
            }
        }
    }

    /*
    ** Process all the server records.
    */
    for (sid = 0; sid < MAX_SERVERS; ++sid)
    {
        /*
        ** check if the server record exists
        */
        if((S_sddindx[sid] != NULL)
            && (S_sddindx[sid]->tid == tid)
            && (strncmp((char *)i_name, (char *)S_sddindx[sid]->i_name, 254) == 0))
        {
            if(S_sddindx[sid]->wwn != wwn)
            {
                fprintf(stderr, "ISCSI_DEBUG: %s: sid=%u old_wwn=%llx new_wwn=%llx\n",
                        __func__, sid, S_sddindx[sid]->wwn, wwn);
                S_sddindx[sid]->wwn = wwn;
                /*
                ** Update to the CCB
                */
                fsl_UpdSID(wwn, sid);
            }
            break;
        }
    }

    if(ispPortAssignment[tid] < maxPorts)
    {
        pCIMT = cimtDir[ispPortAssignment[tid]];
        if(sid >= MAX_SERVERS)
        {
            for(pIMT = pCIMT->imtHead; pIMT != NULL; pIMT = pIMT->link)
            {
                if((pIMT->tid == tid) && (pIMT->mac == wwn))
                {
                    /*
                    ** Remove IMT from the active list
                    */
                    if(pCIMT->imtHead == pIMT)
                    {
                        pCIMT->imtHead = pIMT->link;
                    }
                    else
                    {
                        pPrev->link = pIMT->link;
                    }
                    pIMT->fcAddr = tsih;
                    pIMT->mac  = wwn;
                    pIMT->link = NULL;
                    break;
                }
                pPrev = pIMT;
            }
        }
        else
        {
            /*
            ** Check the active IMT list
            */
            for(pIMT = pCIMT->imtHead; pIMT != NULL; pIMT = pIMT->link)
            {
                if((pIMT->tid == tid) && (pIMT->sid == sid))
                {
                    /*
                    ** Remove IMT from the active list
                    */
                    if(pCIMT->imtHead == pIMT)
                    {
                        pCIMT->imtHead = pIMT->link;
                    }
                    else
                    {
                        pPrev->link = pIMT->link;
                    }
                    MAG_SrvLogout(pCIMT, pIMT);
                    pIMT->fcAddr = tsih;
                    pIMT->mac  = wwn;
                    pIMT->link = NULL;
                    break;
                }
                pPrev = pIMT;
            }
            /*
            ** Check the inactive IMT list
            */
            for(pIMT = C_imt_head; pIMT != NULL; pIMT = pIMT->link)
            {
                if((pIMT->tid == tid) && (pIMT->sid == sid))
                {
                    pIMT->fcAddr = tsih;
                    pIMT->mac  = wwn;
                }
            }
        }
    }
}/* fsl_SrvLogin */

/**
******************************************************************************
**
**  @brief      fsl_SrvLogout
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void fsl_SrvLogout(UINT16 tid, UINT16 tsih, UINT64 wwn)
{
    IMT *pIMT;
    IMT *pPrev = NULL;
    CIMT *pCIMT;
    UINT8 port;
    UINT8 *i_name = NULL;

    if (ICL_TARGET(tid))
    {
        fprintf(stderr,"ISCSI_DEBUG: %s: ICL- tid(%d) tsih(%d) sid(%llx)\n", __func__, tid,tsih,wwn);
    }
    else
    {
        fprintf(stderr,"ISCSI_DEBUG: %s: tid(%d) tsih(%d) sid(%llx)\n", __func__, tid,tsih,wwn);
    }

    if(((port = ispPortAssignment[tid]) == 0xff)
            || ((i_name = iSCSI_GetSrvName(tsih, tid)) == NULL))
    {
        fprintf(stderr,"ISCSI_DEBUG: %s: EXIT port %d i_name = %s tid(%d) tsih(%d) sid(%llx)\n", __func__, port,i_name, tid,tsih,wwn);
        return;
    }

    pCIMT = cimtDir[port];
    /*
    ** Check the inactive IMT list
    */
    for(pIMT = pCIMT->imtHead; pIMT != NULL; pIMT = pIMT->link)
    {
        if((pIMT->tid == tid)
                && (pIMT->mac == wwn))
        {
            if((pIMT->sid < MAX_SERVERS)
                    && (S_sddindx[pIMT->sid] != NULL)
                    && (strncmp((char *)i_name, (char *)S_sddindx[pIMT->sid]->i_name, 254) == 0))
            {
                fsl_logServer(wwn, tid, S_sddindx[pIMT->sid]->i_name, FSL_LOGOUT);
            }
            /*
            ** Remove IMT from the active list
            */
            if(pCIMT->imtHead == pIMT)
            {
                pCIMT->imtHead = pIMT->link;
            }
            else
            {
                pPrev->link = pIMT->link;
            }
            MAG_SrvLogout(pIMT->cimt, pIMT);
            pIMT->link = NULL;
            break;
        }
        pPrev = pIMT;
    }
}/* fsl_SrvLogout */

/**
******************************************************************************
**
**  @brief      find the IMT
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void fsl_UpdName(IMT *pIMT)
{
    UINT8 *i_name = NULL;

    if((i_name = iSCSI_GetSrvName(pIMT->fcAddr, pIMT->tid)) != NULL)
    {
        memcpy(pIMT->i_name, i_name, 254);
    }
}/* fsl_UpdName */


/**
******************************************************************************
**
**  @brief      find the IMT
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
IMT *fsl_findIMT(UINT64 wwn, UINT16 tid, UINT16 tsih)
{
    IMT *pIMT   = NULL;
    UINT8 *i_name = NULL;

#if ICL_DEBUG
    if(ICL_TARGET(tid))
    {
        fprintf(stderr,"<%s>ICL.. entered.\n", __func__);
    }
#endif

    if(gRegTargetsOK == FALSE)
    {
        if (M_chk4XIO(wwn) == 0)
        {
#if ICL_DEBUG
            if(ICL_TARGET(tid))
            {
                fprintf(stderr,"<%s>ICL.. not a XIO Initiator\n", __func__);
            }
#endif
            /*
            ** Not a XIO Initiator - do not allow access to regular initiators until
            ** the gRegTargetsOK flag is set to TRUE.
            */
            pIMT = (IMT *)0xffffffff;
        }
#if ICL_DEBUG
        if(ICL_TARGET(tid))
        {
            fprintf(stderr,"<%s>ICL.. XIO Initiator ????\n", __func__);
        }
#endif
    }
    if((pIMT != (IMT *)0xffffffff) && ((i_name = iSCSI_GetSrvName(tsih, tid)) != NULL))
    {
        /*
        ** Check the inactive IMT list
        */
        for(pIMT = C_imt_head; pIMT != NULL; pIMT = pIMT->link)
        {
            if((pIMT->tid == tid)
                        && (strncmp((char *)i_name, (char *)pIMT->i_name, 254) == 0))
            {
                if(pIMT->mac != wwn)
                {
                    pIMT->mac = wwn;
                }
                if(pIMT->fcAddr != tsih)
                {
                    pIMT->fcAddr = tsih;
                }
                if ((pIMT->sid < MAX_SERVERS)
                            && (S_sddindx[pIMT->sid] != NULL)
                            && (S_sddindx[pIMT->sid]->wwn != wwn))
                {
                    S_sddindx[pIMT->sid]->wwn = wwn;
                    /*
                    ** Update to the CCB
                    */
                    fsl_UpdSID(wwn, pIMT->sid);
                }
                break;
            }
        }
    }
    return (pIMT);
}/* fsl_findIMT */

/**
******************************************************************************
**
**  @brief      To provide a standard means of sending MRP to BE with new mirror
**              partner information (MRSETMPCONFIGBE).
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void fsl_UpdSID(UINT64 wwn, UINT16 sid)
{
    UINT32 size = 0;
    MRUPDSID_REQ *pReq;
    LOG_IPC_BROADCAST_PKT *eldn;

    size = sizeof(LOG_IPC_BROADCAST_PKT) + sizeof(MRUPDSID_REQ);
    eldn = (LOG_IPC_BROADCAST_PKT *)s_MallocC(size, __FILE__, __LINE__);
    pReq = (MRUPDSID_REQ *)(eldn->data);

    eldn->header.event = LOG_IPC_BROADCAST;

    eldn->subEvent  = EB_ISID_SUB;              /* Subevent type             */
    eldn->bcastType = EB_TO_ALL;                /* Broadcast Type            */
    eldn->serialNum = 0;                        /* Serial number to send to  */
    eldn->dataSize  = sizeof(MRUPDSID_REQ);     /* Size of data to follow    */

    pReq->sid = sid;
    pReq->wwn = wwn;

    /*
     * Note: completion routine frees memory later.
     */
    MSC_LogMessageRel(eldn, size);
}/* fsl_UpdSID */

/**
******************************************************************************
**
**  @brief      Check if the port is an iSCSI port
**
**  @param      UINT8 port - port number
**
**  @return     UINT8:  TRUE -  if iSCSI target
**                      FALSE - otherwise
**
******************************************************************************
**/
static void fsl_LogEvent(UINT8 port, UINT32 event, UINT32 reason)
{
    LOG_PORT_EVENT_PKT eldn;                /* Log event */

    /* Send a log message */
    eldn.header.event = event;
    eldn.data.port = port;
    eldn.data.proc = 0;
    eldn.data.reason = reason;

    /* Note: message is short, and L$send_packet copies into the MRP. */
    MSC_LogMessageStack(&eldn, sizeof(LOG_PORT_EVENT_PKT));
}   /* fsl_LogEvent */

/**
******************************************************************************
**
**  @brief      Check if the port is an iSCSI port
**
**              Takes port number as the input param
**
**  @param      UINT8 port - port number
**
**  @return     UINT8:  TRUE -  if iSCSI target
**                      FALSE - otherwise
**
******************************************************************************
**/

void fsl_LogZoneInquiry(IMT *pIMT)
{
    UINT32 size = 0;
    LOG_ZONE_INQUIRY_PKT *elzi;               /* Log event        */

    size = sizeof(LOG_ZONE_INQUIRY_PKT);
    elzi = (LOG_ZONE_INQUIRY_PKT *)s_MallocC(size, __FILE__, __LINE__);
    /*
    ** Send a log message
    */
    elzi->header.event = LOG_ZONE_INQUIRY;
    elzi->channel      = pIMT->cimt->num;
    elzi->targetId     = pIMT->tid;
    elzi->owner        = K_ficb->cSerial;
    elzi->wwn          = pIMT->mac;
    memcpy(elzi->i_name, pIMT->i_name, 254);
    /*
     * Note: completion routine frees memory later.
     */
    MSC_LogMessageRel(elzi, size);
}/* fsl_LogZoneEnquiry */



/**********************************************************************/
TAR *fsl_get_tar(UINT16 tid)
{
    TAR *pTar;
    UINT8 port;

    if ( (iclPortExists) && ICL_TARGET(tid))
    {
        return (tar[ICL_PORT]);
    }

    port = ispPortAssignment[tid];
    if((port < MAX_PORTS) && (BIT_TEST(iscsimap,port)))
    {
        for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
        {
            if(pTar->tid == tid)
            {
                return pTar;
            }
        }
    }
    return NULL;
}

#define TWO_RAISE_TO_16  65536

UINT16 fsl_gen_tsih(UINT16 tid)
{
    SESSION     *pSsnNode = NULL;
    TAR *pTar = NULL;

    if((pTar = fsl_get_tar(tid)) != NULL)
    {
        if(pTar->tsih_seed == 0)
        {
            pTar->tsih_seed++;
        }
        if(pTar->ssn_cnt == (TWO_RAISE_TO_16 - 2))
        {
            char tmp_buff[100] = {0};
            sprintf(tmp_buff, "ISCSI_DEBUG: %s() Max sessions (%d) on TID (%d) reached\n", __func__, pTar->ssn_cnt,tid);
            iscsi_dprintf(tmp_buff);
            return 0;
        }
        /*
        ** match the tsih in the existing session list
        */
        for(pSsnNode = (SESSION*)pTar->portID; pSsnNode != NULL; pSsnNode = pSsnNode->pNext)
        {
            /*
            ** Get the session and compare isid
            */
            if(pSsnNode->tsih == pTar->tsih_seed)
            {
                pTar->tsih_seed++;
                if(pTar->tsih_seed == 0)
                {
                    pTar->tsih_seed++;
                }
            }
        }
        return pTar->tsih_seed++;
    }
    return 1;
}/* fsl_gen_tsih */
/**
******************************************************************************
**
**  @brief          iSCSI stack to C driver interface function.
**                  Primary ILT is created in stack and passed to fsl
**                  and from that second level of ILT if filled as per C driver
**                  requirement and passed to C Driver.
**
**  @param          ILT*    pILT
**
**  @return         none
**
******************************************************************************
**/
void fsl_iscsi_scsi(ILT* pILT)
{
    ILT    *pPrevILT;
    UINT8  taskAttrib;
    ILT    *piScsiILT;

    /*
    ** Validate the input
    */
    if (!pILT)
    {
        printf ("From Interface function:  error No valid input ILT ");
        return;
    }

    /*
    ** The ILT expected here is 2nd level, i.e. in which we fill data directly.
    ** Get info from Primary ILT and update second level of ILT.
    */

    piScsiILT                =  pILT -1;

    pILT->scsi_cdm.scchipi            = (UINT8)piScsiILT->iscsi_def.portId;
    pILT->scsi_cdm.sclun              = piScsiILT->iscsi_def.lun;
    pILT->scsi_cdm.scinit             = piScsiILT->iscsi_def.tsih;
    pILT->scsi_cdm.scvpid             = piScsiILT->iscsi_def.tid;
    pILT->scsi_cdm.scrxid             = bswap_32(piScsiILT->iscsi_def.pPdu->bhs.itt);

    /*
    ** Check for the command type for TMF(task management function)
    */
    if(piScsiILT->iscsi_def.cmdType == 0x02)
    {
        pILT->scsi_cdm.sccommand = ISP_IMMED_NOTIFY_CMD;
        pILT->inotify_2400.instatus  = 0x36;

        /*
        ** clear requeset not complete for ISP_IMMED_NOTIFY_CMD
        */
        pILT->scsi_cdm.scrcn      = 0;
        if(piScsiILT->iscsi_def.pPdu)
        {
            pILT->scsi_cdm.sctaskf = piScsiILT->iscsi_def.flags;
            /*
             ** the exchange id for the command will be referenced task tag which is at the position of ttt
             ** if task is ABORT TASK (0x80)
             */
            if(piScsiILT->iscsi_def.flags == 0x80)
            {
                pILT->scsi_cdm.scrxid  = bswap_32(piScsiILT->iscsi_def.pPdu->bhs.ttt);
            }
        }
    }
    else
    {
        pILT->scsi_cdm.sccommand  = ISP_SCSI_CMD;
        pILT->scsi_cdm.sctaskf    = 0;
        pILT->ilt_normal.w4         = (UINT32)(piScsiILT->iscsi_def.pCdb);
        pILT->scsi_cdm.scdatalen  = getScsiexpDataTxLen(piScsiILT->iscsi_def.pPdu);

        /*
        ** set request not complete FLAG before sending it to PROC for non-immediate
        ** command otherwise clear this is done in case of FC
        */
        pILT->scsi_cdm.scrcn      = 1;
        /*
        ** Get the task code from iSCSI ILT
        */

        taskAttrib = (piScsiILT->iscsi_def.flags) & 0x07;
        switch(taskAttrib)
        {
            case IS_UNTAGGED:
            {
                pILT->scsi_cdm.sctaskc = SAM_UNTAGGED;
                break;
            }
            case IS_SIMPLE:
            {
                pILT->scsi_cdm.sctaskc = SAM_SIMPLE;
                break;
            }
            case IS_ORDERED:
            {
                pILT->scsi_cdm.sctaskc = SAM_ORDERED;
                break;
            }
            case IS_HOQ:
            {
                pILT->scsi_cdm.sctaskc = SAM_HOQ;
                break;
            }
            case IS_ACA:
            {
                pILT->scsi_cdm.sctaskc = SAM_ACA;
                break;
            }
            default:
            {
                pILT->scsi_cdm.sctaskc = 0;
                break;
            }
        }

        if (piScsiILT->iscsi_def.flags & R_BIT)
        {
            pILT->scsi_cdm.scexecc = 0x02;
        }
        else if (piScsiILT->iscsi_def.flags & W_BIT)
        {
            pILT->scsi_cdm.scexecc = 0x01;
        }
        else if (piScsiILT->iscsi_def.flags & RW_BIT)
        {
            pILT->scsi_cdm.scexecc = 0x03;
        }
        else
        {
            pILT->scsi_cdm.scexecc = 0x00;
        }
    }
    /*
     ** Fill completion routine pointer. C_label_in_... function call is is to save
     ** 'C call back routine' in cr(), the parameter in the function call is the
     ** actual call back routine, called from asm.
     ** This is for 'asm' to 'C' converter.
     ** If called and callback both are in 'C', no need to call this C_label_... function.
     */
    pPrevILT = pILT;
    if(piScsiILT->iscsi_def.cmdType == 0x02)
    {
        /* in case of task management the call back function is different.
         ** This callback function will send the response as well as
         ** clear the ILT created at iscsi layer.
         */
        pILT->cr = NULL;
    }
    else
    {
        pILT->cr = (void*)C_label_referenced_in_i960asm(fsl_ilt_cb);
    }

    /*
    ** Bump ILT to next level, fill completion pointer, present ILT pointer in misc,
    ** and call Cdriver
    */
    pILT        = (pILT + 1);
    pILT->cr    = NULL;
    pILT->misc  = (UINT32)pPrevILT;
    C_recv_scsi_io(pILT);
}

/*
 *********************************************************************************
 ** @brief   Call back routine for all SCSi command ILTs except for Task amnagement
 **
 ** @param   ILT *pILT     Ilt pointer
 ** @param   UINT32  status
 **
 ** @return  none
 **
 *********************************************************************************
 */
void fsl_ilt_cb(UINT32 status UNUSED, ILT *pILT)
{
    /*
     ** This an ILT call back routine, will be called from SCSI module, from here
     ** iSCSI routine is to to be called, where all clean up process (release like
     **  ILT, PDU, Connection,) will be done.
     */
    if(pILT->scsi_cdm.imt != NULL)
    {
        ((IMT*)(pILT->scsi_cdm.imt))->qDepth--;
    }
    iscsiCr(pILT - 1);
}
/*
*********************************************************************************
** @brief   This an Task management ILT call back routine, will be called from
**          SCSI module,iSCSI response will be sent and clean up process
**          (release like ILT, PDU, Connection,) will be done.
**
** @param    ILT    *pILT     Ilt pointer
** @param    UINT32  status
**
** @return    none
**
*********************************************************************************
*/
void fsl_tmf_cb1(UINT32 status, ILT *pILT);
void fsl_tmf_cb1(UINT32 status, ILT *pILT)
{
    fsl_tmf_cb(status, pILT);
}
void fsl_tmf_cb(UINT32 status UNUSED, ILT *pILT)
{
    ISCSI_PDU           *pPdu = NULL;
    CONNECTION          *pConn = NULL;
    ILT                 *piSCSI_ILT = NULL;
    TMF_RESPONSE        TmfResponse = TMF_FUNCTION_COMPLETE;
    TMF_REQUEST         function;
    INT32               retVal = 0;

    /*
    ** Get the iSCSI ILT
    */
    if((ILT*)pILT->misc)
    {
        piSCSI_ILT = pILT - 1;

    }
    else
    {
        fprintf(stderr,"ISCSI_DEBUG: ERROR: The iscsi ILT is null\n");
        return;
    }

    pPdu  = piSCSI_ILT->iscsi_def.pPdu;
    pConn = piSCSI_ILT->iscsi_def.pConn;

    if(piSCSI_ILT->iscsi_def.cmdType == INIOP_TASK_MGMT_REQ)
    {
        function = GET_TMF_FUNCTION(pPdu->bhs.flags);
        TmfResponse    = TMF_FUNCTION_COMPLETE;
        /*
        ** remove the TMF command from command Queue
        */
        fprintf(stderr, "ISCSI_DEBUG: TMF response function = %d from %s on Target %d ITT %x\n",
            function, pConn->pSession->params.initiatorName, pConn->pTPD->tid,
            bswap_32(((ISCSI_GENERIC_HDR)pPdu->bhs).itt));

        if (function == TMF_WARM_RESET)
        {
            retVal = CreateTMFResponseAndSend(pConn, (ISCSI_TMF_REQ_HDR*)&(pPdu->bhs), TmfResponse, iscsi_crTmfWarm);
        }
        else if (function == TMF_COLD_RESET)
        {
            TAR *pTar = NULL;
            /*
            ** cleanup target
            */
            if((pTar = fsl_get_tar(piSCSI_ILT->iscsi_def.tid)) != NULL)
            {
                retVal = CreateTMFResponseAndSend(pConn, (ISCSI_TMF_REQ_HDR*)&(pPdu->bhs), TmfResponse, iscsi_crTmfCold);
            }
            else
                fprintf(stderr,"ISCSI_DEBUG: %s: TARGET COLD RESET, NULL TAR for tid %d\n", __func__, piSCSI_ILT->iscsi_def.tid);
        }
        else
        {
            retVal = CreateTMFResponseAndSend(pConn, (ISCSI_TMF_REQ_HDR*)&(pPdu->bhs), TmfResponse, iscsi_crRelILT);
        }
        /*
        ** clearing the TMF ILT
        */
        iscsiCr(pILT - 1);
    }
}

/**
******************************************************************************
**
**  @brief      Log Event to CCB when an iSCSI Initiator logs in/ out
**
**  @param      sid
**              tid
**              tsih
**              initiator Name
**              SessionType
**
**  @return     none
**
******************************************************************************
**/
void fsl_logServer(UINT64 wwn, UINT16 tid, UINT8* i_name, UINT8 state)
{
    UINT32 size = 0;
    LOG_SERVER_LOGGED_IN_OP_PKT *elzi;               /* Log event        */

    size = sizeof(LOG_SERVER_LOGGED_IN_OP_PKT);
    elzi = (LOG_SERVER_LOGGED_IN_OP_PKT *)s_MallocC(size, __FILE__, __LINE__);

    /*
    ** Send a log message
    */
    elzi->header.event = LOG_SERVER_LOGGED_IN_OP;
    elzi->tid = tid;
    elzi->loginState= state;
    elzi->wwn = wwn;
    memcpy(elzi->i_name, i_name, 254);
    /*
     * Note: completion routine frees memory later.
     */
    MSC_LogMessageRel(elzi, size);
}/* fsl_logServer */


void iscsi_dprintf(const char *fmt, ...)
{
    va_list arglist;
    UINT32 size;
    LOG_ISCSI_GENERIC_PKT  *elzi;

    size = sizeof(LOG_ISCSI_GENERIC_PKT);
    elzi = (LOG_ISCSI_GENERIC_PKT*)s_MallocC(size, __FILE__, __LINE__);

    va_start(arglist, fmt);
    vsnprintf((char *)elzi->msgStr, sizeof(elzi->msgStr), fmt, arglist);
    va_end(arglist);

    /*
    ** Send a log message
    */
    elzi->header.event = LOG_ISCSI_GENERIC;
    /*
     * Note: completion routine frees memory later.
     */
    MSC_LogMessageRel(elzi, size);
}


/*****************************************************************************
********************** CODE TO SUPPORT iSCSI INITIATOR ***********************
*****************************************************************************/

#define SGLDATA_LEN_MASK   0x00FFFFFF

/**
******************************************************************************
**
**  @brief      Is Initiator Function Present
**
**  @param      char* - Initiator Name
**              char* - Target Name
**
**  @return     IDD - Pointer to IDD Structure
**
******************************************************************************
**/
UINT32 fsl_isIFP(UINT16 port, UINT16 lid)
{
    if (ICL_IsIclPort((UINT8)port))
    {
        return TRUE;
    }

    return ((BIT_TEST(iscsimap, port)) ? TRUE : BIT_TEST(portdb[port][lid].prliw3, 5));
}/* fsl_isIFP */

/**
******************************************************************************
**
**  @brief     fsl_updateTMT
**             This is for updating TMT in gIDX
**
**  @param     port - port
**  @param     lid  - lid
**  @param     pTMT - TMT pointer
**
**  @return     none
**
******************************************************************************
**/
void fsl_updateTMT(UINT16 port, UINT16 lid, TMT *pTMT)
{
#if ICL_DEBUG
    if(ICL_IsIclPort(port))
    {
        fprintf(stderr,"<%s:%s> ICL port\n",__FILE__, __func__);
    }
#endif

    if((BIT_TEST(iscsimap, port) || ICL_IsIclPort(port)) && (lid < MAX_DEV) && (gIDX[port][lid] != NULL))
    {
        if((gIDX[port][lid]->pTMT = pTMT) == NULL)
        {
            BIT_CLEAR(gIDX[port][lid]->flags, IDF_PATH);
        }
        else
        {
            BIT_SET(gIDX[port][lid]->flags, IDF_PATH);
            pTMT->lid = lid;
        }
    }
}/* fsl_updateTMT */


/**
******************************************************************************
**
**  @brief    fsl_getPortWWN
**            This is for retrieving port WWN
**
**
**  @param     port - port
**  @param     lid  - lid
**
**  @return     none
**
******************************************************************************
**/
UINT64 fsl_getPortWWN(UINT16 port, UINT16 lid)
{
    if( (BIT_TEST(iscsimap, port)) || (ICL_IsIclPort(port)) )
    {
        return ((gIDX[port][lid] != NULL) ? (gIDX[port][lid]->t_pname) : 0x0);
    }
    else {
        return (portdb[port][lid].pdn);
    }
}/* fsl_getPortName */

/**
******************************************************************************
**
**  @brief    fsl_getNodeWWN
**            This is for retrieving node WWN
**
**
**  @param     port - port
**  @param     lid  - lid
**
**  @return     none
**
******************************************************************************
**/
UINT64 fsl_getNodeWWN(UINT16 port, UINT16 lid)
{
    if( (BIT_TEST(iscsimap, port)) || (ICL_IsIclPort (port)) )
    {
        return ((gIDX[port][lid] != NULL) ? (gIDX[port][lid]->t_name) : 0x0);
    }
    else {
        return (portdb[port][lid].ndn);
    }
}/* fsl_getNodeName */


/**
******************************************************************************
**
**  @return     none
**
******************************************************************************
**/
void fsl_assocIMT_LTMT(IMT *pIMT, LTMT *pLTMT)
{
    IDD  *pIDD  = NULL;
    TMT  *pTMT  = NULL;
    CIMT *pCIMT = NULL;

    if(pIMT == NULL)
    {
        pLTMT->pIMT = NULL;
        if(((pCIMT = pLTMT->pCIMT) != NULL) && ((pTMT = pLTMT->pTMT) != NULL))
        {
            for(pIMT = pCIMT->imtHead; pIMT != NULL; pIMT = pIMT->link)
            {
                if ((pIDD = gIDX[pTMT->chipID][pTMT->lid]) != NULL &&
                    pIDD->t_pname == pIMT->mac &&
                    T_tgdindx[pIMT->vpID] &&
                    pIDD->i_name == PORT_WWN(pIMT->vpID, pCIMT->num))
                {
                    pLTMT->pIMT = pIMT;
                    pIMT->ltmt = pLTMT;
                    break;
                }
            }
        }
    }
    else if(pLTMT == NULL)
    {
        pIMT->ltmt = NULL;
        if((pCIMT = pIMT->cimt) != NULL)
        {
            for(pLTMT = pCIMT->ltmtHead; pLTMT != NULL; pLTMT = pLTMT->link)
            {
                if ((pTMT = pLTMT->pTMT) != NULL &&
                    (pIDD = gIDX[pTMT->chipID][pTMT->lid]) != NULL &&
                    pIDD->t_pname == pIMT->mac &&
                    T_tgdindx[pIMT->vpID] &&
                    pIDD->i_name == PORT_WWN(pIMT->vpID, pCIMT->num))
                {
                    pLTMT->pIMT = pIMT;
                    pIMT->ltmt = pLTMT;
                    break;
                }
            }
        }
    }
}/* fsl_assocIMT_LTMT */

/**
******************************************************************************
**
**  @return     none
**
******************************************************************************
**/
void fsl_resetLPMap(UINT16 port)
{
    if(BIT_TEST(iscsimap, port)|| ICL_IsIclPort (port))
    {
        UINT32 i;

        fprintf(stderr,"[%s:%d\t%d:-] %s\n", __FILE__, __LINE__, port, __func__);

        for(i = 0; i < MAX_DEV; i++)
        {
            if(gIDX[port][i] != NULL)
            {
                if((gIDX[port][i]->flags == 0)
                     || (gIDX[port][i]->flags == (1 << IDF_READY))
                     || (BIT_TEST(gIDX[port][i]->flags, IDF_RETRY)))
                {
                    s_Free((void *)gIDX[port][i], sizeof(IDD), __FILE__, __LINE__);
                }
                else if(gIDX[port][i]->flags == (1 << IDF_INIT))
                {
                    gIDX[port][i]->flags = (1 << IDF_FREE);
                }
                else if(BIT_TEST(gIDX[port][i]->flags, IDF_LOGIN))
                {
                    BIT_SET(gIDX[port][i]->flags, IDF_ABORT);
                    fprintf(stderr,"[%s:%d\t%d:%d] %s: gIDX[%d][%d]=%p flags=%x\n",
                            __FILE__, __LINE__,port,i, __func__, port,i, gIDX[port][i],(gIDX[port][i])->flags);
                }
                else
                {
                    BIT_SET(gIDX[port][i]->flags, IDF_FREE);
                    fprintf(stderr,"[%s:%d\t%d:-] %s calling fsl_logout %u %u\n",
                        __FILE__, __LINE__, port, __func__, port, i);
                    fsl_logout(port,i);
                }
                gIDX[port][i] = NULL;
            }
        }
        if (lpmap[port])
        {
            for (i =0; i< LOOP_MAP_SIZE; i++)
            {
                lpmap[port][i] = 0xff;
            }
        }
    }
}/* fsl_resetLPMap */

void fsl_initLPMap(UINT16 port)
{
    UINT32 i, j;
    IDD *pIDD = NULL;
    request *pReq = NULL;
    response *pRsp = NULL;

    fprintf(stderr,"[%s:%d\t%d:-] %s\n", __FILE__, __LINE__, port, __func__);

    if ((tar[port] == NULL)
              || (T_tgdindx[tar[port]->tid] == NULL)
              || (ispPortAssignment[tar[port]->tid] != port)
              || (T_tgdindx[tar[port]->tid]->prefPort != port))
    {
        return;
    }

    pReq = (request *)s_MallocC(sizeof(request), __FILE__, __LINE__);
    pRsp = (response *)s_MallocC(sizeof(response), __FILE__, __LINE__);

    /*
    ** Create IDDs for intra-DSC targets
    */
    for (i = 0, j = 0; i < MAX_TARGETS; ++i)
    {
        if((T_tgdindx[i] != NULL)
                   && (T_tgdindx[i]->owner != K_ficb->cSerial)
                   && ((T_tgdindx[i]->prefPort == port) || (T_tgdindx[i]->altPort == port)))
        {

            /*
            ** Make sure that no iSCSI port tries to login to ICL port.
            ** Only ICL port logins to ICL port on the other controller.
            */
            if (ICL_IsIclPort(port))
            {
#if ICL_DEBUG
                fprintf(stderr,"<%s:%s>source port = ICL\n",__FILE__, __func__);
#endif
                /*
                ** Login trial is from ICL port of this controller..
                ** Selected target on other controller must be ICL target
                */
                if(! (ICL_TARGET(T_tgdindx[i]->tid)))
                {
                    /*
                    ** selected target is not ICL..Go for next one
                    */
                    continue;
                }
#if ICL_DEBUG
                fprintf(stderr,"<%s:%s>ICL..target = %u selected\n",
                         __FILE__, __func__,(UINT32)(T_tgdindx[i]->tid));
#endif
            }
            else
            {
                /*
                ** Login trail is from non-ICL port of this controller.
                ** Slected target on the other controller should not be ICL target
                **/
                if( ICL_TARGET(T_tgdindx[i]->tid))
                {
                    /*
                    ** Selected target is ICL, so go for next one
                    */
                    continue;
                }
            }

            /*
            ** Alloc & initialize the IDD
            */
            if(pIDD == NULL)
            {
                pIDD = (IDD *)s_MallocC(sizeof(IDD), __FILE__, __LINE__);
            }
            memset((void *)pIDD, 0, sizeof(IDD));

#if defined(MODEL_7000) || defined(MODEL_4700)
            pIDD->t_name  = T_tgdindx[i]->nodeName;
#else  /* MODEL_7000 || MODEL_4700 */
            /*
            ** Target is clustered.  Obtain the node WWN from the clustered target.
            ** else - Obtain the node WWN from the target configuration record.
            */
            if (T_tgdindx[i]->cluster < MAX_TARGETS && T_tgdindx[T_tgdindx[i]->cluster] != NULL)
            {
                pIDD->t_name  = T_tgdindx[T_tgdindx[i]->cluster]->nodeName;
            }
            else
            {
#if ICL_DEBUG
                if(ICL_IsIclPort(port))
                {
                    fprintf(stderr,"<%s:%s>ICL. not clustered\n", __FILE__, __func__);
                }
#endif
                if (T_tgdindx[i | 0x02])
                {
                    pIDD->t_name  = T_tgdindx[i | 0x02]->nodeName;
                }
                else
                {
                    fprintf(stderr, "FE %s:%u target=%d, but T_tgdindx[%d] pointer is NULL\n",
                            __func__, __LINE__, i, i | 0x02);
                    pIDD->t_name  = T_tgdindx[i]->nodeName;
                }
            }
#endif /* MODEL_7000 || MODEL_4700 */
            pIDD->t_pname = PORT_WWN(T_tgdindx[i]->tid,T_tgdindx[i]->prefPort);
            pIDD->t_ip    = T_tgdindx[i]->ipAddr;
            pIDD->i_name  = PORT_WWN(tar[port]->tid,port);
            pIDD->i_ip    = T_tgdindx[tar[port]->tid]->ipAddr;
            pIDD->ptg     = i;
            pIDD->sg_fd   = -1;
            pIDD->port    = port;
            pIDD->lid     = j;
            BIT_SET(pIDD->flags, IDF_INIT);
            gIDX[port][j]  = pIDD;

            /*
            ** Add the target to the iscsid dbm, if not existing in the database
            ** if the IP address changes then ip will be updated in the database
            */
            pReq->command = X_MGMT_IPC_TARGET_ADD;
            fsl_buildReq(pIDD, pReq);
            if((fsl_iscsidTxRx(pReq, pRsp) != EC_OK) || (pRsp->status != 0))
            {
               fprintf(stderr,"[%s:%d\t%d:%d] %s TARGET_ADD FAILED st=%d i=0x%llx t=0x%llx\n",
                                    __FILE__, __LINE__, pIDD->port, pIDD->lid, __func__, pRsp->status, pIDD->t_name, pIDD->i_name);
               /*
               ** Target is not deleted in db so no login possible
               */
                s_Free((void *)pIDD, sizeof(IDD), __FILE__, __LINE__);
                gIDX[port][j]  = NULL;
                pIDD = NULL;
                continue;
            }
            if(pIDD->flags != (1 << IDF_INIT))
            {
                s_Free((void *)pIDD, sizeof(IDD), __FILE__, __LINE__);
                gIDX[port][j]  = NULL;
                pIDD = NULL;
                break;
            }
            lpmap[port][j] = j;

            /*
             * Wait if fsl_tmonitor process is being created.
             */
            while (gMonTask == (PCB *)-1)
            {
                TaskSleepMS(50);
            }

            if(gMonTask == NULL)
            {
                CT_fork_tmp = (unsigned long)"fsl_tmonitor";
                gMonTask = (PCB *)-1;
                gMonTask = TaskCreate2(C_label_referenced_in_i960asm(fsl_tmonitor), 135);
            }
            fprintf(stderr,"[%s:%d\t%d:%d] %s: gIDX[%d][%d]=0x%x\n",__FILE__, __LINE__,port,j, __func__, port,j,(UINT32)pIDD);
            BIT_CLEAR(pIDD->flags, IDF_INIT);
            BIT_SET(pIDD->flags, IDF_READY);
            I_login(pIDD->port, pIDD->lid);
            pIDD = NULL;
            j++;
        }
    }

    /*
    ** Create IDDs for inter-DSC targets for Geo-Rep * Future Release
    ** VLinks configuration is not necessary for ICL.. it should be avoided.
    */

    if(pIDD != NULL)
    {
        s_Free((void *)pIDD, sizeof(IDD), __FILE__, __LINE__);
    }
    /*
    ** Free the Req & Rsp pointers
    */
    s_Free((void *)pReq, sizeof(request), __FILE__, __LINE__);
    s_Free((void *)pRsp, sizeof(response), __FILE__, __LINE__);
}/* fsl_initLPMap */


void fsl_updatePaths(void)
{
    /*
    ** In a FC+iSCSI mixed mode, ispmap != iscsimap. We dont want
    ** to activate the initiator path in the mixed environments.
    */
    if (ispmap != iscsimap)
    {
        return;
    }
    /*
     * Wait if fsl_tmonitor process is being created.
     */
    while (gMonTask == (PCB *)-1)
    {
        TaskSleepMS(50);
    }

    if(gMonTask == NULL)
    {
        CT_fork_tmp = (unsigned long)"fsl_tmonitor";
        gMonTask = (PCB *)-1;
        gMonTask = TaskCreate2(C_label_referenced_in_i960asm(fsl_tmonitor), 135);
    }
    if (TaskGetState(gMonTask) == PCB_NOT_READY)
    {
#ifdef HISTORY_KEEP
CT_history_pcb("fsl_updatePaths setting ready pcb", (UINT32)gMonTask);
#endif
        TaskSetState(gMonTask, PCB_READY);
    }
}

/**
******************************************************************************
**
**  @return     none
**
******************************************************************************
**/
NORETURN
void fsl_tmonitor(void)
{
    UINT16 port = 0;
    UINT32 lid = 0;
    IDD *pIDD  = NULL;
    UINT32 maxPorts = MAX_PORTS;

    fprintf(stderr,"[%s:%d] %s\n", __FILE__, __LINE__,  __func__);

    if(iclPortExists)
    {
        maxPorts = MAX_PORTS+MAX_ICL_PORTS;
    }

    while(1)
    {
        for(port = 0; port < maxPorts; port++)
        {
            if( ( (BIT_TEST(iscsimap, port)) || (ICL_PRT(port)) )
                             && (BIT_TEST(ispOnline, port))
                             && (!BIT_TEST(C_ca.status, CA_SHUTDOWN))
                             && (tar[port] != NULL)
                             && (tar[port]->tid < MAX_TARGETS)
                             && (ispPortAssignment[tar[port]->tid] == port))
            {

                if(((pIDD = gIDX[port][0]) == NULL))
                {
                    fsl_initLPMap(port);
                }

                for(lid = 0; lid < MAX_DEV; lid++)
                {
                    if(((pIDD = gIDX[port][lid]) != NULL)
                        && (BIT_TEST(pIDD->flags, IDF_RETRY)))
                    {
                        BIT_CLEAR(pIDD->flags, IDF_RETRY);
//                        if(! ICL_IsIclPort(pIDD->port))
//                        {
//                            fprintf(stderr,"[%s:%d\t%d:%d]: %s 0x%x f=0x%x\n",
//                                 __FILE__, __LINE__, pIDD->port, pIDD->lid, __func__, (UINT32)pIDD, pIDD->flags);
//                        }
                        I_login(pIDD->port, pIDD->lid);
                    }
                }
            }
        }
        TaskSetMyState(PCB_NOT_READY);
        TaskSwitch();
    }
}/* fsl_tmonitor */

/**
******************************************************************************
**
**  @return     none
**
******************************************************************************
**/
void fsl_logout(UINT16 port, UINT16 lid)
{
    IDD *pIDD;

    if (lid > MAX_DEV)
    {
        return;
    }

    pIDD = gIDX[port][lid];

    if((pIDD != NULL) && (!BIT_TEST(pIDD->flags, IDF_LOGOUT)))
    {
        BIT_SET(pIDD->flags, IDF_LOGOUT);
        CT_fork_tmp = (unsigned long)"fsl_tlogout";
        pIDD->pTLogoutPcb = (void *)TaskCreate3(C_label_referenced_in_i960asm(fsl_tlogout), 35, (UINT32)pIDD);
        fprintf(stderr,"[%s:%d\t%d:%d]: %s %p created tlogout pcb %p\n",
                __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->pTLogoutPcb);
        TaskSwitch();
        fprintf(stderr,"[%s:%d\t%d:%d]: %s %p returning from TaskSwitch\n",
                __FILE__, __LINE__, port, lid, __func__, pIDD);
    }
}/* fsl_logout */

void fsl_tlogout(UINT32 a UNUSED, UINT32 b UNUSED, IDD *pIDD)
{
    UINT16 port = 0, lid = 0;

    port =  pIDD->port;
    lid  =  pIDD->lid;

    if(BIT_TEST(pIDD->flags, IDF_DEVICE))
    {
        ILT *pILT = NULL;
        INT32 reset = SG_SCSI_RESET_DEVICE;
        int rc;

        fprintf(stderr,"[%s:%d\t%d:%d]: %s IDF_DEVICE %p f=0x%x\n",
                              __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags);
        BIT_CLEAR(pIDD->flags, IDF_DEVICE);
        tsl_ev_del(pIDD->sg_fd, EPOLLIN, fsl_sgio_cb, NULL);
        tsl_cleanupEvents((void *)pIDD);

        /*
        ** Before completing the outstanding requests back to the PROC
        ** layers, we need to flush out these requests from the kernel
        ** layers. For this, we issue a SCSI RESET to the sg dev. If
        ** the reset fails, we wait 1 sec before reissuing the RESET.
        ** Once the RESET is either successful or no longer necessary,
        ** complete the outstanding reqs back the PROC layer.
        */
        while(BIT_TEST(pIDD->flags, IDF_SESSION))
        {
            rc = ioctl(pIDD->sg_fd, SG_SCSI_RESET, &reset);
            if (rc >= 0)
            {
                fprintf(stderr,"[%s:%d\t%d:%d]: %s SG_SCSI_RESET %s complete rc %d\n",
                        __FILE__, __LINE__, port, lid, __func__, pIDD->sg_name, rc);
                BIT_CLEAR(pIDD->flags, IDF_SESSION);
                break;
            }
            else if (errno == ENODEV)
            {
                fprintf(stderr,"[%s:%d\t%d:%d]: %s SG_SCSI_RESET %s ENODEV, continuing\n",
                        __FILE__, __LINE__, port, lid, __func__, pIDD->sg_name);
                BIT_CLEAR(pIDD->flags, IDF_SESSION);
                break;
            }
            else if (errno == EIO)
            {
                /*
                ** Ran into a case where the transport driver returns error due to a reset on
                ** terminated session - the event for that termination was for some misterious
                ** reason never received from iscsid. In this case, the reset logic is re-issuing
                ** the reset and ends up in this state forever until rebooted. When this happened,
                ** it was in this state for more than 48 hrs!!! Adding a check to bail out. CQT #17122
                */
                fprintf(stderr,"[%s:%d\t%d:%d]: %s SG_SCSI_RESET %s EIO, continuing\n",
                        __FILE__, __LINE__, port, lid, __func__, pIDD->sg_name);
                break;
            }
            else {
                fprintf(stderr,"[%s:%d\t%d:%d]: %s SG_SCSI_RESET %s failed, errno %d %s\n",
                        __FILE__, __LINE__, port, lid, __func__, pIDD->sg_name, errno, strerror(errno));
                /*
                ** While this task is in this wait state, if the kernel sends a connection error
                ** event to the iSCSID, the tsl event handler will change the state of this
                ** task to READY - That way we avoid sitting in this wait state when the
                ** connection termination occurs even though SCSI RESET failed.
                */
                TaskSleepMS(1000);
            }
        }

        pIDD->pTLogoutPcb = NULL;
        close(pIDD->sg_fd);
        pIDD->sg_fd = 0;
        memset(pIDD->sg_name, 0, 16);

        fprintf(stderr,"[%s:%d\t%d:%d]: %s IDF_DEVICE %p f=0x%x ready to complete ILTs\n",
                              __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags);
        while((pILT = pIDD->iltQ) != NULL)
        {
            pIDD->iltQ = pILT->fthd;
            if(pIDD->iltQ)
            {
                pIDD->iltQ->bthd = NULL;
            }
            /*
            ** Free the sense buffer and iovector,which are saved on 4th Level ILT, in sgTx
            */
            if (pILT->ilt_normal.w2 != 0)
                s_Free((void*)pILT->ilt_normal.w2, sizeof(SNS), __FILE__, __LINE__);

            if (pILT->ilt_normal.w0 != 0)
                s_Free((void*)pILT->ilt_normal.w0, pILT->ilt_normal.w1, __FILE__, __LINE__);
            /*
            ** iltQ has the 4th level ILT. 3rd level is what was on ispthread & 2nd level
            ** has the proc callback.
            */
            if (isp_unthread_ilt(pIDD->port, (pILT -1)))
            {
                KernelDispatch(0xff, (pILT - 2), (void *)SGE_LOGOUT, 0);
            }
        }
        fprintf(stderr,"[%s:%d\t%d:%d]: %s IDF_DEVICE %p f=0x%x done completing ILTs\n",
                              __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags);
    }
    if(BIT_TEST(pIDD->flags, IDF_PATH))
    {
        fprintf(stderr,"[%s:%d\t%d:%d]: %s IDF_PATH %p f=0x%x\n",
                      __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags);
        BIT_CLEAR(pIDD->flags, IDF_PATH);
        if(pIDD->pTMT != NULL)
        {
            I_logout(pIDD->pTMT);
            pIDD->pTMT = NULL;
        }
    }
    if(BIT_TEST(pIDD->flags, IDF_SESSION))
    {
        UINT32 rVal;
        request  *pReq = (request *)s_MallocC(sizeof(request), __FILE__, __LINE__);
        response *pRsp = (response *)s_MallocC(sizeof(response), __FILE__, __LINE__);

        fprintf(stderr,"[%s:%d\t%d:%d]: %s IDF_SESSION %p f=0x%x\n",
                              __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags);

        pReq->command = X_MGMT_IPC_SESSION_CLOSE;
        fsl_buildReq(pIDD, pReq);
        rVal = fsl_iscsidTxRx(pReq, pRsp);
        BIT_CLEAR(pIDD->flags, IDF_SESSION);
        fprintf(stderr,"[%s:%d\t%d:%d]: %s IDF_SESSION %p f=0x%x request complete rval %u status %u\n",
                __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags, rVal, pRsp->status);
        s_Free((void *)pReq, sizeof(request), __FILE__, __LINE__);
        s_Free((void *)pRsp, sizeof(response), __FILE__, __LINE__);
    }

    BIT_CLEAR(pIDD->flags, IDF_LOGOUT);

    if(BIT_TEST(pIDD->flags, IDF_FREE))
    {
        fprintf(stderr,"[%s:%d\t%d:%d]: %s IDF_FREE %p f=0x%x\n",
                              __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags);
        pIDD->flags = 0x0;
        s_Free((void *)pIDD, sizeof(IDD), __FILE__, __LINE__);
        pIDD = 0x0;
    }
    else
    {
        pIDD->pTLogoutPcb = NULL;
        BIT_SET(pIDD->flags, IDF_RETRY);
        fprintf(stderr,"[%s:%d\t%d:%d]: %s EXIT %p f=0x%x\n",
                              __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags);
        if (TaskGetState(gMonTask) == PCB_NOT_READY)
        {
#ifdef HISTORY_KEEP
CT_history_pcb("fsl_tlogout setting ready pcb", (UINT32)gMonTask);
#endif
            TaskSetState(gMonTask, PCB_READY);
        }
    }
}/* fsl_tlogout */

/**
******************************************************************************
**
**  @return     none
**
******************************************************************************
**/
void fsl_buildReq(IDD *pIDD, request *pReq)
{
#if ICL_DEBUG
        if (pIDD->port == ICL_PORT)
        {
            fprintf(stderr,"<%s:%s>ICL port RequestCmd:%d\n",__FILE__, __func__,pReq->command);
        }
#endif
    switch(pReq->command)
    {
        case X_MGMT_IPC_TARGET_ADD:
        {
            wwn2naa(pIDD->t_name, pReq->u.target.name);
            pReq->u.target.pgtag = pIDD->ptg;
            pReq->u.target.ip    = pIDD->t_ip;
            pReq->u.target.port  = 3260;
        }
        break;
        case X_MGMT_IPC_SESSION_CLOSE:
        case X_MGMT_IPC_SESSION_LOGIN:
        {
            wwn2naa(pIDD->t_name, (UINT8 *)pReq->u.o_session.target_name);
            wwn2naa(pIDD->i_name, (UINT8 *)pReq->u.o_session.initiator_name);
            pReq->u.o_session.pgtag        = pIDD->ptg;
            pReq->u.o_session.initiator_ip = pIDD->i_ip;
        }
        break;
        case X_MGMT_IPC_DISCOVERY:
        {
            wwn2naa(pIDD->i_name, pReq->u.discovery.initiator_name);
            pReq->u.discovery.initiator_ip = pIDD->i_ip;
            pReq->u.discovery.target_ip    = pIDD->t_ip;
            pReq->u.discovery.port         = 3260;
        }
        break;
        case X_MGMT_IPC_TARGET_SHOW:
        default:
        {
            /* TBD */
        }
        break;
    }
}/* fsl_buildReq */

/**
******************************************************************************
**
**  @return     none
**
******************************************************************************
**/
void fsl_login(ILT *pILT)
{
    IDD *pIDD = NULL;

    if(((pIDD = gIDX[(pILT - 1)->out_initiator.oil1_chpid][(pILT - 1)->out_initiator.oil1_lid]) == NULL)
                                          || (pIDD->flags != (1 << IDF_READY)))
    {
        KernelDispatch(SGE_IOE, pILT, (void *)SGE_IOE, 0);
    }
    else
    {
        BIT_SET(pIDD->flags, IDF_LOGIN);
        CT_fork_tmp = (unsigned long)"fsl_tlogin";
        TaskCreate3(C_label_referenced_in_i960asm(fsl_tlogin), 35, (UINT32)pILT);
        TaskSwitch();
    }
}/* fsl_login */

void fsl_tlogin(UINT32 a UNUSED, UINT32 b UNUSED, ILT *pILT)
{
    IDD *pIDD = NULL;
    UINT32 rVal = EC_OK;
    request *pReq = NULL;
    response *pRsp = NULL;
    UINT32 status = SGE_ME;
    UINT16 port = 0;
    UINT16 lid = 0;

    port = (pILT - 1)->out_initiator.oil1_chpid;
    lid = (pILT - 1)->out_initiator.oil1_lid;

//    if(! ICL_IsIclPort(port))
//    {
//        fprintf(stderr,"[%s:%d\t%d:%d]: %s\n", __FILE__, __LINE__, port, lid, __func__);
//    }

    pIDD = gIDX[(pILT - 1)->out_initiator.oil1_chpid][(pILT - 1)->out_initiator.oil1_lid];
    if((pIDD == NULL) || (!BIT_TEST(ispOnline, pIDD->port))
                      || (pIDD->flags != ((1 << IDF_LOGIN) | (1 << IDF_READY))))
    {
        KernelDispatch(SGE_IOE, pILT, (void *)SGE_IOE, 0);
        return;
    }

    pReq = (request *)s_MallocC(sizeof(request), __FILE__, __LINE__);
    pRsp = (response *)s_MallocC(sizeof(response), __FILE__, __LINE__);

    /*
    ** Check to see if the IP address of the target port has changed. If it did,
    ** Remove the previous target record and add the updated target record to
    ** the database before proceeding with the login.
    */
    if (T_tgdindx[pIDD->ptg] && pIDD->t_ip != T_tgdindx[pIDD->ptg]->ipAddr)
    {
        /*
        ** Update the ip in IDD and Add the target to the iscsid dbm
        */
        pIDD->t_ip = T_tgdindx[pIDD->ptg]->ipAddr;
        pReq->command = X_MGMT_IPC_TARGET_ADD;
        fsl_buildReq(pIDD, pReq);
        if ((fsl_iscsidTxRx(pReq, pRsp) != EC_OK) || (pRsp->status != 0))
        {
            BIT_CLEAR(pIDD->flags, IDF_LOGIN);
            BIT_SET(pIDD->flags, IDF_RETRY);
            fprintf(stderr,"[%s:%d\t%d:%d] %s TARGET_ADD FAILED st=%d i=0x%llx t=0x%llx\n",
                                __FILE__, __LINE__, pIDD->port, pIDD->lid, __func__, pRsp->status, pIDD->t_name, pIDD->i_name);
            s_Free((void *)pReq, sizeof(request), __FILE__, __LINE__);
            s_Free((void *)pRsp, sizeof(response), __FILE__, __LINE__);
            KernelDispatch(SGE_IOE, pILT, (void *)SGE_IOE, 0);
            if (TaskGetState(gMonTask) == PCB_NOT_READY)
            {
#ifdef HISTORY_KEEP
CT_history_pcb("fsl_tlogin setting ready pcb", (UINT32)gMonTask);
#endif
                TaskSetState(gMonTask, PCB_READY);
            }
            return;
        }
    }

    /*
    ** Do Login Now
    */
    pReq->command = X_MGMT_IPC_SESSION_LOGIN;
    fsl_buildReq(pIDD, pReq);
    rVal = fsl_iscsidTxRx(pReq, pRsp);
    BIT_CLEAR(pIDD->flags, IDF_LOGIN);

//    if(! ICL_IsIclPort(port))
//    {
//        fprintf(stderr,"[%s:%d\t%d:%d]: %s post LOGIN 0x%x f=0x%x\tr=%d rs=%d\n",
//                              __FILE__, __LINE__, port, lid, __func__, (UINT32)pIDD, pIDD->flags, rVal, pRsp->status);
//    }
    if(BIT_TEST(pIDD->flags, IDF_ABORT))
    {
        fprintf(stderr,"[%s:%d\t%d:%d]: %s post LOGIN ABORT SET 0x%x f=0x%x\n",
                              __FILE__, __LINE__, port, lid, __func__, (UINT32)pIDD,pIDD->flags);
        pIDD->flags = 0x0;
        s_Free((void *)pIDD, sizeof(IDD), __FILE__, __LINE__);
        pIDD = 0x0;
    }
    else if(rVal != EC_OK)
    {
        BIT_SET(pIDD->flags, IDF_RETRY);
        fprintf(stderr,"[%s:%d\t%d:%d]: %s %p f=0x%x setting retry due to request error %d\n",
                __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags, rVal);
        status = SGE_ME;
    }
    else if(BIT_TEST(pIDD->flags, IDF_LOGOUT))
    {
        BIT_SET((pILT - 1)->out_initiator.oil1_flag, OIFLG_ABRT);
    }
    else if((pRsp->status == MGMT_IPC_ERR_SESSON_EXIST)
                || (BIT_TEST((pILT - 1)->out_initiator.oil1_flag, OIFLG_ABRT)))
    {
        BIT_SET(pIDD->flags, IDF_SESSION);
        fprintf(stderr,"[%s:%d\t%d:%d]: %s %p f=0x%x session exists or abort, calling fsl_logout %u %u\n",
                __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags, pIDD->port, pIDD->lid);
        fsl_logout(pIDD->port, pIDD->lid);
    }
    else if(pRsp->status == 0)
    {
        BIT_SET(pIDD->flags, IDF_SESSION);
        sprintf((char *)pIDD->sg_name, "/dev/%s",pRsp->u.session.device_name);
        if((pIDD->sg_fd = open((char *)pIDD->sg_name,O_RDWR | O_NONBLOCK)) > 0)
        {
            BIT_SET(pIDD->flags, IDF_DEVICE);
            tsl_ev_add(pIDD->sg_fd, EPOLLIN, fsl_sgio_cb, pIDD);
            status = SGE_SUCCESS;
            fprintf(stderr,"[%s:%d\t%d:%d]: %s %p f=0x%x\tdev=%s\n",
                              __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags, pIDD->sg_name);
        }
        else {
            fprintf(stderr,"[%s:%d\t%d:%d]: %s %p f=0x%x\tdev=%s open failed, calling fsl_logout %u %u\n",
                    __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags, pIDD->sg_name, pIDD->port, pIDD->lid);
            fsl_logout(pIDD->port, pIDD->lid);
        }
    }
    else {
        BIT_SET(pIDD->flags, IDF_RETRY);
//        if(! ICL_IsIclPort(port))
//        {
//            fprintf(stderr,"[%s:%d\t%d:%d]: %s %p f=0x%x setting retry due to response status %d\n",
//                __FILE__, __LINE__, port, lid, __func__, pIDD, pIDD->flags, pRsp->status);
//        }
    }

    s_Free((void *)pReq, sizeof(request), __FILE__, __LINE__);
    s_Free((void *)pRsp, sizeof(response), __FILE__, __LINE__);

    KernelDispatch(status, pILT, (void *)status, 0);
    if(pIDD && (BIT_TEST(pIDD->flags, IDF_RETRY)))
    {
        if (TaskGetState(gMonTask) == PCB_NOT_READY)
        {
#ifdef HISTORY_KEEP
CT_history_pcb("fsl_tlogin #2 setting ready pcb", (UINT32)gMonTask);
#endif
            TaskSetState(gMonTask, PCB_READY);
        }
    }
}/* fsl_tlogin */

/**
******************************************************************************
**
**  @return     none
**
******************************************************************************
**/
UINT32 fsl_iscsidTxRx(request *req, response *rsp)
{
    UINT32 fd;
    INT32 rVal = EC_OK;
    INT32 current, balance;
    struct sockaddr_un addr;
    request *pReq = req;
    response *pRsp = rsp;

    if((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) > 0)
    {
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_LOCAL;
        memcpy((char *)&addr.sun_path + 1, "ISCSIADM_ABSTRACT_NAMESPACE", strlen("ISCSIADM_ABSTRACT_NAMESPACE"));

        TSL_SetNonBlocking (fd);

        if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        {
            fprintf(stderr,"ISCSI_DEBUG: !!!! !!!! !!! %s: connect ERROR: errno(%d, %s)\n",
                   __func__, errno,strerror(errno));
            rVal = EC_CONN_ERROR;
        }
        else
        {
            tsl_ev_add(fd, EPOLLIN, fsl_iscsidCb, (void *)K_xpcb);
            if(write(fd, (char *)pReq, sizeof(request)) != sizeof(request))
            {
                fprintf(stderr,"ISCSI_DEBUG: !!!! !!!! !!! %s: write ERROR: errno(%d, %s)\n",
                        __func__, errno,strerror(errno));
                rVal = EC_RETRY;
            }
            else
            {
                /*
                ** Set Task to wait for events
                */
                current = 0;
                balance = sizeof(response);
                while (1)
                {
                    TaskSetMyState(PCB_NOT_READY);
                    TaskSwitch();

                    rVal = read(fd, (char*)pRsp + current, balance);
                    if(rVal < 0)
                    {
                        if((errno != EINTR) && (errno != EAGAIN))
                        {
                            fprintf(stderr,"ISCSI_DEBUG: fsl error in read: %d %s\n", errno, strerror(errno));
                            rVal = EC_RETRY;
                            break;
                        }
                    }
                    else if(rVal != balance)
                    {
                        current += rVal;
                        balance -= rVal;
                    }
                    else {
                        rVal = EC_OK;
                        break;
                    }
                }
            }
            tsl_ev_del(fd, EPOLLIN, fsl_iscsidCb, NULL);
            tsl_cleanupEvents((void *)K_xpcb);
        }
        close(fd);
    }
    else
    {
        fprintf(stderr,"ISCSI_DEBUG: !!!! !!!! !!! %s: socket ERROR: errno(%d, %s)\n",
                __func__, errno,strerror(errno));
        rVal = EC_RETRY;
    }
    return ((UINT32)rVal);
}/* fsl_iscsidTxRx */


/**
******************************************************************************
**
**  @brief      Callback function for iscsidTxRx
**
**  @param      UINT32 - events
**              void * - pRef
**
**  @return     none
**
******************************************************************************
**/
void fsl_iscsidCb(UINT32 events UNUSED, void *pRef)
{
    if(pRef != NULL)
    {
        if (TaskGetState((PCB *)pRef) == PCB_NOT_READY)
        {
#ifdef HISTORY_KEEP
CT_history_pcb("fsl_iscsidCb setting ready pcb", (UINT32)pRef);
#endif
            TaskSetState((PCB *)pRef, PCB_READY);
        }
    }
}/* fsl_iscsidCb */

/**
******************************************************************************
**
******************************************************************************
**/
UINT32 fsl_sgTx(ILT *pILT)
{
    XLI *pXLI = NULL;
    IDD *pIDD = NULL;
    UINT32 rVal = EC_OK;
    sg_io_hdr_t  sg_io_reqhdr;
    int i=0, res = 0;
    UINT32 iovec_size=0;
    sg_iovec_t *io = NULL;

    pXLI = (XLI*)(pILT->misc);

    if(((pIDD = gIDX[pXLI->xlichipid][pXLI->xlitarget]) == NULL)
             || (!BIT_TEST(pIDD->flags, IDF_DEVICE))
             || (BIT_TEST(pIDD->flags, IDF_LOGOUT))
             || BIT_TEST(pIDD->flags, IDF_ABORT))
    {
        /*
        ** call kernel dispatch with 0xff
        */
        KernelDispatch(0xff, (pILT - 1), (void *)SGE_LOGOUT, 0);
        return EC_IO_ERR;
    }
    pILT->ilt_normal.w0 = 0x0;
    memset(&sg_io_reqhdr,0,sizeof(sg_io_hdr_t));
    /*
    ** Fill the sg_ioreq header
    */
    sg_io_reqhdr.interface_id  = 'S';
    sg_io_reqhdr.cmd_len       = 16; /* 16 byte cdb this hardcoding to be removed */
    sg_io_reqhdr.cmdp          = (UINT8*)(pXLI->xlicdbptr);
    sg_io_reqhdr.mx_sb_len     = sizeof(SNS);
    sg_io_reqhdr.sbp           = (UINT8 *)s_MallocC(sizeof(SNS), __FILE__, __LINE__);
    sg_io_reqhdr.timeout       = INT_MAX;
    sg_io_reqhdr.usr_ptr       = pILT;


    switch(pXLI->xlisglnum)
    {
        case 0:
            sg_io_reqhdr.dxferp      = NULL;
            sg_io_reqhdr.dxfer_len   = 0;
            sg_io_reqhdr.iovec_count = 0;
        break;
        case 1:
            sg_io_reqhdr.flags      |= SG_FLAG_DIRECT_IO | SG_FLAG_DIRECT_ONLY;
            sg_io_reqhdr.dxferp      = (void *)((SGL_DESC *)pXLI->xlisglptr)->addr;
            sg_io_reqhdr.dxfer_len   = ((SGL_DESC *)pXLI->xlisglptr)->len & 0x00ffffff;
            sg_io_reqhdr.iovec_count = 0;
        break;
        default:
        {
            SGL_DESC *pSD = (SGL_DESC *)pXLI->xlisglptr;
            io = (sg_iovec_t *)s_MallocC(sizeof(sg_iovec_t)*pXLI->xlisglnum, __FILE__, __LINE__);

            sg_io_reqhdr.dxfer_len = 0;
            for(i = 0; i < pXLI->xlisglnum; i++)
            {
                io[i].iov_base          = pSD[i].addr;
                io[i].iov_len           = pSD[i].len & 0x00ffffff;
                sg_io_reqhdr.dxfer_len += pSD[i].len & 0x00ffffff;
            }
            /*
            **  pointer to SGL
            */
            sg_io_reqhdr.flags      |= SG_FLAG_DIRECT_IO  | SG_FLAG_DIRECT_ONLY;
            sg_io_reqhdr.dxferp      = (void *)io;
            sg_io_reqhdr.iovec_count = pXLI->xlisglnum;
        }
        break;
    }

    switch(pXLI->xlidatadir & DATA_DIR_MASK)
    {
        case XLI_DATA_READ:
            sg_io_reqhdr.dxfer_direction = SG_DXFER_FROM_DEV;
            break;
        case XLI_DATA_WRITE:
            sg_io_reqhdr.dxfer_direction = SG_DXFER_TO_DEV;
            break;
        case XLI_DATA_NONE:
        default:
            sg_io_reqhdr.dxfer_direction = SG_DXFER_NONE;
            break;
    }
    sg_io_reqhdr.dxfer_len = (( sg_io_reqhdr.dxfer_len + 3) & ~0x03);
    pILT->ilt_normal.w1 = sg_io_reqhdr.dxfer_len;

    if((res = write(pIDD->sg_fd, &sg_io_reqhdr, sizeof(sg_io_reqhdr)))
                                                    != sizeof(sg_io_hdr_t))
    {
        int save_errno = errno;

        pIDD->werr++;
        if((sg_io_reqhdr.iovec_count != 0) && (sg_io_reqhdr.dxferp != NULL))
        {
            s_Free(sg_io_reqhdr.dxferp, (sizeof(sg_iovec_t) * sg_io_reqhdr.iovec_count), __FILE__, __LINE__);
        }
        s_Free(sg_io_reqhdr.sbp, sizeof(SNS), __FILE__, __LINE__);
        fprintf(stderr,"ISCSI_DEBUG: %s: write ERROR: res(%d)  errno(%d, %s)\n",
                        __func__, res, save_errno,strerror(save_errno));
      /*
      ** If the write fails because of sg device disappears, report all the pending
      ** ILTs  with error to the proc for proper cleanup,otherwise  que the ILT to the
      ** retry task in proc.
      */
        switch(save_errno)
        {
            case ENODEV:
            {
                KernelDispatch(0xff, (pILT - 1), (void *)SGE_LOGOUT, 0);
                fprintf(stderr,"ISCSI_DEBUG: %s: write errno(%d, %s), calling fsl_logout %u %u\n",
                        __func__, errno,strerror(errno), pIDD->port, pIDD->lid);
                fsl_logout(pIDD->port, pIDD->lid);
            }
            break;
            case EAGAIN:
            case EINTR:
            case EFAULT:
            case EINVAL:
            default:
                 KernelDispatch(0x8d, (pILT - 1), (void *)SGE_IOE, 0);
            break;
        }
        rVal = EC_LGOFF;
    }
    else
    {
        pIDD->werr = 0;
        /*
        ** Add this ILT to the active list.
        */
        isp_thread_ilt(pXLI->xlichipid, pILT);
        /*
         ** Add the next level(4th) ILT to pIDD->iltQ
         */
        pILT = pILT + 1;
        memset(pILT, 0, sizeof (ILT));
        if((sg_io_reqhdr.iovec_count != 0) && (sg_io_reqhdr.dxferp != NULL))
        {
            iovec_size = (sizeof(sg_iovec_t) * sg_io_reqhdr.iovec_count);
            pILT->ilt_normal.w0 = (UINT32)io;
            pILT->ilt_normal.w1 = (UINT32)iovec_size;
            pILT->ilt_normal.w2 = (UINT32)sg_io_reqhdr.sbp;
        }
        if(pIDD->iltQ)
        {
            pILT->fthd = pIDD->iltQ;
            pIDD->iltQ->bthd = pILT;
            pIDD->iltQ = pILT;
            pILT->bthd = NULL;
        }else
        {
            pIDD->iltQ = pILT;
            pILT->fthd = NULL;
            pILT->bthd = NULL;
        }
    }
    return rVal;
}/* fsl_sgTx */


/**
******************************************************************************
**
**  @return     none
**
******************************************************************************
**/
void fsl_sgio_cb(UINT32 events UNUSED, void *pRef)
{
    IDD *pIDD = NULL;
    sg_io_hdr_t ioh;
    ILT *pILT = NULL;
    ILT *pfILT = NULL;
    ILT *psILT = NULL;
    int res = 0;

    if(pRef == NULL)
    {
        return;
    }
    pIDD = (IDD *)pRef;
    if((!BIT_TEST(ispOnline, pIDD->port))
                || (BIT_TEST(pIDD->flags, IDF_LOGOUT))
                || (!BIT_TEST(pIDD->flags, IDF_DEVICE)))
    {
        return;
    }
    memset((void *)&ioh, 0, sizeof(sg_io_hdr_t));
    if((res = read(pIDD->sg_fd, &ioh, sizeof(ioh))) == sizeof(sg_io_hdr_t))
    {
        pIDD->rerr = 0;
        if((ioh.iovec_count != 0) && (ioh.dxferp != NULL))
        {
            s_Free(ioh.dxferp, (sizeof(sg_iovec_t) * ioh.iovec_count), __FILE__, __LINE__);
        }

        /*
        ** Remove this ILT from the active list.
        */
        if (((pILT = (ILT *)ioh.usr_ptr) == NULL) ||
            ((pILT = isp_unthread_ilt(pIDD->port, (ILT *)ioh.usr_ptr)) == NULL))
        {
            fprintf(stderr, "%s: unthread FAILED port = %d ILT=0x%X\n",
                    __func__, pIDD->port, (UINT32)ioh.usr_ptr);
            s_Free(ioh.sbp, sizeof(SNS), __FILE__, __LINE__);
            return;
        }

        /*
        ** remove this ILT from IDD iltQ
        */
        pILT += 1;
        if(pILT->bthd)
        {
            pILT->bthd->fthd = pILT->fthd;
            if(pILT->fthd)
                pILT->fthd->bthd = pILT->bthd;
        }
        else
        {
            pIDD->iltQ = pILT->fthd;
            if(pIDD->iltQ)
                pIDD->iltQ->bthd = NULL;
        }

        pILT -= 1;
        psILT = (pILT - 1);
        pfILT = (psILT -1);
        pfILT->ilt_normal.w6 = (UINT32)ioh.sbp;
        pfILT->ilt_normal.w7 = (UINT32)ioh.resid;

        /*
        ** Check if the ILT is aborted by the PROC. If yes, return the ILT
        ** with SGE_ABORT status
        */
        if(pILT->ilt_normal.w0 & 0x01)
        {
            s_Free(ioh.sbp, sizeof(SNS), __FILE__, __LINE__);
            pILT->ilt_normal.w2 = 0x0;
            pILT->ilt_normal.w0 &= ~0x01;
            KernelDispatch(0, (pILT - 1),  (void *)SGE_ABORT, 0);
        }
        else if((ioh.info & SG_INFO_OK_MASK) == SG_INFO_OK)
        {
            s_Free(ioh.sbp, sizeof(SNS), __FILE__, __LINE__);
            pILT->ilt_normal.w2 = 0x0;
            KernelDispatch(0, (pILT - 1),  (void *)SGE_SUCCESS, 0);
        }
        else
        {
            if(ioh.sb_len_wr != 0)
            {
                /*
                ** Check SCSI SNS for status. We will need to pass the SNS buf
                ** to the upper layers here. Take care to release that SNS buf
                ** after the error handling is done - by saving it in some
                ** level of the ILT that will be returned back to the PROC
                */
                UINT32 w2 = pILT->ilt_normal.w2;
                UINT32 cr = (UINT32)(pILT-1)->cr;
                KernelDispatch(SGE_CS, (pILT - 1),  (void *)SGE_CS, 0);
                s_Free(ioh.sbp, sizeof(SNS), __FILE__, __LINE__);
                if (w2 != 0)
                {
fprintf(stderr, "%s:%u This overwrites a freed ILT in some cases (cr-1)=0x%x w2=0x%08x\n", __func__, __LINE__, cr, w2);
                    pILT->ilt_normal.w2 = 0x0;
                }
            }
            else
            {
                UINT32 astatus = 0;

                if((astatus = ioh.masked_status) != 0x0)
                {
                    s_Free(ioh.sbp, sizeof(SNS), __FILE__, __LINE__);
                    pILT->ilt_normal.w2 = 0x0;
                    KernelDispatch((astatus << 1), (pILT - 1), (void *)SGE_NOCS, 0);
                }
                else if((astatus = ioh.host_status) != 0x0)
                {
                    /*
                    ** Host status defined in the following table is sent to the
                    ** proc as IO error.
                    ** error    Description
                    ** -----    -----------
                    ** 0x00     OK
                    ** 0x01     No Connection
                    ** 0x02     BUS Busy
                    ** 0x03     IO Timed out
                    ** 0x04     Target not responding
                    ** 0x05     IO Abort
                    ** 0x06     Parity Error
                    ** 0x07     Internal Error
                    ** 0x08     SCSI bus reset
                    ** 0x09     Got an interrupt
                    ** 0x0a     Passthrough Error
                    ** 0x0b     Low level driver wants a retry
                    ** 0x0c     Retry without decrementing count
                    ** 0x0d     Requeue cmd without decrementing count
                    */
                    s_Free(ioh.sbp, sizeof(SNS), __FILE__, __LINE__);
                    pILT->ilt_normal.w2 = 0x0;
                    astatus = (ioh.host_status == 0x0) ? 0x0 : (ioh.host_status | 0x80);
                    // fprintf(stderr, "host_status is %d\n",ioh.host_status);
                    KernelDispatch(astatus, (pILT - 1), (void *)SGE_IOE, 0);
                }
                else
                {
                    /*
                    ** Driver status defined in the following table is sent to the
                    ** proc as misc error.
                    ** error    Description
                    ** -----    -----------
                    ** 0x00     OK
                    ** 0x01     Driver Busy
                    ** 0x02     Driver Soft Error
                    ** 0x03     Driver Media Error
                    ** 0x04     Driver Error
                    ** 0x05     Driver Invalid Error
                    ** 0x06     Driver Timeout Error
                    ** 0x07     Driver Hard Error
                    ** 0x08     Driver Sense Error
                    ** 0x10     Suggest Retry
                    ** 0x20     Suggest Abort
                    ** 0x30     Suggest Remap
                    ** 0x40     Suggest Die
                    ** 0x80     Suggest Sense
                    */
                    astatus = (ioh.driver_status == 0x0) ? 0xff : ioh.driver_status;
                    s_Free(ioh.sbp, sizeof(SNS), __FILE__, __LINE__);
                    pILT->ilt_normal.w2 = 0x0;
                    // fprintf(stderr, "driver_status is %d\n",ioh.driver_status);
                    KernelDispatch(astatus, (pILT - 1), (void *)SGE_ME, 0);
                }
            }
        }
    }
    else {
        pIDD->rerr++;
        switch(errno)
        {
            case ENODEV:
                {
                    fprintf(stderr,"ISCSI_DEBUG: %s: read errno(%d, %s), calling fsl_logout %u %u\n",
                            __func__, errno,strerror(errno), pIDD->port, pIDD->lid);
                    fsl_logout(pIDD->port, pIDD->lid);
                }
                break;
            case EAGAIN:
            case EINTR:
            case EFAULT:
            case EINVAL:
            default:
                break;
        }
    }
}/* fsl_sgio_cb */

/**
******************************************************************************
**
**  @brief     Generates a iSCSI Name from the Port Name
**
**  @param     UINT64 - Port Name
**             UINT8*  - iSCSI Name
**
**  @return    none
**
******************************************************************************
**/
void wwn2naa(UINT64 wwn, UINT8 *pNaa)
{
    strcpy((char *)pNaa,"naa.");
    convertIntoReadableData(wwn,pNaa+4);
}   /* wwn2naa */

/**
******************************************************************************
**
**  @brief      Converts given initiator name to wwn and checks if it is a
**              xiotech initiator or not.
**
**  @param      UINT8*  - Initiator name
**
**  @return     TRUE - when wwn is of xio controller type, FALSE otherwise.
**
******************************************************************************
**/
bool fsl_is_xioInit(UINT8 *i_name)
{
    UINT64 wwn = 0;

    /*
     * Convert the initiator name to 64 bit wwn format and check whether it is
     * xio-initiator or not
     */
    convertTargetNameInTo64bit(i_name,20, &wwn);
    if (M_chk4XIO(wwn) == 0)
    {
        /* Not a XIO Initiator, return false */
        return FALSE;
    }
    return TRUE;
}

IDD *getIdd(UINT16 portId, UINT16 devId)
{
    if(gIDX[portId][devId] != NULL)
    {
        return(gIDX[portId][devId]);
    }
    return(NULL);
}

UINT32 fsl_getPID(UINT16 port, UINT16 lid)
{
    if(gIDX[port][lid] != NULL)
    {
        return (gIDX[port][lid]->t_ip);
    }
    return 0xffffffff;
}   /* fsl_getPID */

void dumpTMT(UINT16 port);
void dumpTMT(UINT16 port)
{
    UINT16 i = 0, j = 0;

    fprintf(stderr, "%s:port(%02d) cimt(0x%08X) icimt(0x%08X)\n", __func__, port, (UINT32)cimtDir[port], (UINT32)I_CIMT_dir[port]);

    if((cimtDir[port] == NULL) || (I_CIMT_dir[port] == NULL))
        return;

    fprintf(stderr, "CIMT(0x%08X): port(%02d) state(%d) istate(%d)\n", (UINT32)cimtDir[port],
                                                        cimtDir[port]->num,
                                                        cimtDir[port]->state,
                                                        cimtDir[port]->iState);

    fprintf(stderr, "I_CIMT(0x%08X): port(%02d) lid(0x%04X) pid(0x%06X) state(%d)\n",
                                                        (UINT32)I_CIMT_dir[port],
                                                        I_CIMT_dir[port]->chpid,
                                                        I_CIMT_dir[port]->mylid,
                                                        I_CIMT_dir[port]->mypid,
                                                        I_CIMT_dir[port]->state);

    for(i = 0; i < MAX_DEV; i++)
    {
        if(I_CIMT_dir[port]->tmdir[i] != NULL)
        {
            fprintf(stderr,"\t%02d. 0x%08X chpid(%01d) lid(0x%04X) alpa(0x%06X) pn(0x%016llX) nn(0x%16llX) st(%02d) fg(0x%02X)\n",i,
                                                        (UINT32)I_CIMT_dir[port]->tmdir[i],
                                                        ((TMT *)I_CIMT_dir[port]->tmdir[i])->chipID,
                                                        ((TMT *)I_CIMT_dir[port]->tmdir[i])->lid,
                                                        ((TMT *)I_CIMT_dir[port]->tmdir[i])->alpa,
                                                        ((TMT *)I_CIMT_dir[port]->tmdir[i])->P_name,
                                                        ((TMT *)I_CIMT_dir[port]->tmdir[i])->N_name,
                                                        ((TMT *)I_CIMT_dir[port]->tmdir[i])->state,
                                                        ((TMT *)I_CIMT_dir[port]->tmdir[i])->flag);
            if(((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt != NULL)
            {
                fprintf(stderr,"\t\tLTMT(0x%08X): cimt(0x%08X) tmt(0x%08X) imt(0x%08X) pn(0x%016llX) lst(%02d)\n",
                                                        (UINT32)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt,
                                                        (UINT32)((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pCIMT,
                                                        (UINT32)((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pTMT,
                                                        (UINT32)((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pIMT,
                                                        ((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pname,
                                                        ((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->linkState);
                if(((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pIMT != NULL)
                {
                    fprintf(stderr,"\t\tIMT(0x%08X): ltmt(0x%08X) fcaddr(0x%04X) vpid(0x%04X) flags(0x%02X) mac(0x%016llX)\n",
                                                        (UINT32)((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pIMT,
                                                        (UINT32)((IMT *)((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pIMT)->ltmt,
                                                        ((IMT *)((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pIMT)->fcAddr,
                                                        ((IMT *)((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pIMT)->vpID,
                                                        ((IMT *)((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pIMT)->flags,
                                                        ((IMT *)((LTMT *)((TMT *)I_CIMT_dir[port]->tmdir[i])->ltmt)->pIMT)->mac);
                }
            }
            for(j = 0; j < MAX_LUNS; j++)
            {
                if(((TMT *)I_CIMT_dir[port]->tmdir[i])->tlmtDir[j] != NULL)
                {
                    fprintf(stderr,"\t\t\t%02d. 0x%08X\n", j, (UINT32)((TMT *)I_CIMT_dir[port]->tmdir[i])->tlmtDir[j]);
                }
            }
        }
    }
}/* dumpTMT */


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
