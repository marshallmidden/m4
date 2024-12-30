/* $Id: idriver.c 161368 2013-07-29 14:53:10Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       idriver.c
**
**  @brief      Front-end Initiator driver
**
**  Copyright (c) 2008-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>

#include "isp.h"
#include "pm.h"
#include "mem_pool.h"
#include "CT_defines.h"
#include "ilt.h"
#include "icimt.h"
#include "cimt.h"
#include "tmt.h"
#include "tlmt.h"
#include "scsi.h"
#include "fc.h"
#include "misc.h"
#include "portdb.h"
#include "ecodes.h"

#include "XIO_Macros.h"
#include "XIO_Std.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

#if (MAX_ISE_LUNS > 126)
#define DISC_BUFSIZE        (16+8*MAX_ISE_LUNS)
#else   /* (MAX_ISE_LUNS > 126) */
#define DISC_BUFSIZE        1024
#endif  /* (MAX_ISE_LUNS > 126) */
#define SCAN_DELAY          250
#define SCAN_RETRY          5
#define GAN_RETRY_CNT       2
#define SCSI_FCP            0x01
#define I_DISPRI            174
#define I_SCANPRI           174
#define I_TIMERPRI          180

/* Timeout for TMT not in active state (discovery & inactive) 30 secs (=250ms*120) */
#define TARGET_TO          120
#define TARGET_DEL          1

/* PID Address formats as defined in the FCP spec */
#define AF_PORT              0
#define AF_AREA              1
#define AF_DOMAIN            2
#define AF_FABRIC            3

/* Scan state defines */
typedef enum scan_state
{
    SCAN_INIT = 0,
    SCAN_LOGIN,
    SCAN_TUR,
    SCAN_SSU,
    SCAN_INQ,
    SCAN_RLUN,
    SCAN_RC,
    SCAN_DONE,
    SCAN_ABORT,
    SCAN_RESTART
} scan_state;

char stateStr[10][16] =
{
    "SCAN_INIT\0 ",
    "SCAN_LOGIN\0",
    "SCAN_TUR\0",
    "SCAN_SSU\0",
    "SCAN_INQ\0",
    "SCAN_RLUN\0",
    "SCAN_RC\0",
    "SCAN_DONE\0",
    "SCAN_ABORT\0",
    "SCAN_RESTART\0"
};

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define CDB_SET_CODE(cdb, code)  (((UINT8 *)cdb)[0] = code)

#define PID_AF(p)       (((p) & 0xff))
#define PORT_ALPA(p)    (((p) & 0xffffff00))
#define AREA_ALPA(p)    (((p) & 0x00ffff00))
#define DOM_ALPA(p)     (((p) & 0x0000ff00))

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern CIMT     *cimtDir[MAX_CIMT];
extern ICIMT    *I_CIMT_dir[MAX_ICIMT];
extern TGD      *T_tgdindx[MAX_TARGETS];

extern void CT_LC_tScan(void *);
extern void CT_LC_cbScan(UINT32, void*);
extern void CT_LC_t_iTimer(UINT16);
extern void CT_LC_t_fcDiscover(UINT16, void*);
extern void CT_LC_t_iscsiDiscover(UINT16, void*);

extern UINT32   isp_registerFc4(UINT8 port, UINT32 fc4Type, TAR *pTar);
extern void     ISP_initiate_io (ILT* pILT);
extern UINT32   ISP_IsMyWWN(UINT64 *);

extern void     I_queTLMT(ICIMT *pICIMT, TLMT *pTLMT);
extern UINT16   I_get_lid(UINT8 port);
extern void     I_put_lid(UINT8 port, UINT16 lid);
extern void     I_removeLUN(TLMT * pTLMT, TMT *pTMT, ICIMT *pICIMT);
extern void     APL_StartIO(ICIMT *pICIMT, TLMT *pTLMT);
extern void     APL_TimeoutILT(ILT *pILT, ICIMT *pICIMT, TMT *pTMT, TLMT *pTLMT);
extern void     APL_AbortILT(ILT *pILT, ICIMT *pICIMT, TMT *pTMT, TLMT *pTLMT);

extern UINT32   LLD_CheckTargetDel(TMT *);
extern void     LLD_TargetOffline(TMT *);
extern void     LLD_TargetOnline(TMT *);

UINT8 FT_DEFINED = 0;               /* If Foreign Target is turned on (non-zero). */
/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
void I_ChangeNotification(UINT16 port, UINT32 pid);
void I_ChangeDB(UINT16 port, UINT16 lid, UINT16 code);
void I_Offline(UINT16 port);
void I_Online(UINT16 port, UINT16 lid);
void I_RemoveTarget(TMT *pTMT);
void I_Rescan(TMT *pTMT);
UINT32 I_GanLoop(UINT16 port, UINT32 pid);

void t_iTimer(UINT32 a UNUSED, UINT32 b UNUSED, UINT16 port);
void t_iscsiDiscover(UINT32 a UNUSED, UINT32 b UNUSED, UINT16 port, TAR *pTAR);
void t_fcDiscover(UINT32 a UNUSED, UINT32 b UNUSED, UINT16 port, TAR *pTAR);

void tScan(UINT32 a, UINT32 b, TMT *pTMT);
void cbScan(UINT32 status, ILT *pILT);

void i_login(ILT* pILT);
void i_tur(ILT* pILT);
void i_inq (ILT* pILT);
void i_rluns (ILT* pILT);
void i_rc (ILT* pILT);

void snd_tur(ILT *pILT, UINT16 lun);
void snd_ssu(ILT *pILT, UINT16 lun);
void snd_inq(ILT *pILT, UINT16 lun);
void snd_rluns(ILT *pILT, UINT16 lun);
void snd_rc(ILT *pILT, UINT16 lun);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Timer Task
**
**              Marks all the TMTs on the port as BADBOYs, runs ganloop
**              and then removes the TMTs that are still BADBOYs
**
**  @param      UINT16     - Port
**
**  @return     none
**
******************************************************************************
**/
NORETURN
void t_iTimer(UINT32 a UNUSED, UINT32 b UNUSED, UINT16 port)
{
    ILT *ilt;
    ILT *pILT;
    TMT *tmt;
    TMT *pTMT;
    TLMT *tlmt;
    TLMT *pTLMT;
    ICIMT *pICIMT = I_CIMT_dir[port];

    while (1)
    {
        TaskSleepMS(250);

        /* If reset on this port is in progress, do nothing! */
        if (BIT_TEST(resilk, port))
        {
            continue;
        }
        /*
        ** Check all the TLMTs in the active list for outstanding task (ILT)
        ** timeouts. If the task timed out, call the callback function with
        ** timeout error. If the TLMT state is 'delete' and if all the
        ** outstanding tasks are taken care of, release the TLMT
        */
        pTLMT = pICIMT->actqhd;
        while (pTLMT != NULL)
        {
            tlmt = pTLMT;
            pTLMT = pTLMT->flink;

            if (BIT_TEST(resilk, port))
            {
                 break;
            }
            if ((tlmt->tmt->state == tmstate_deltar)
                    && (tlmt->state != tlmstate_delete))
            {
                    pTMT = tlmt->tmt;
                    fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: TLMT(0x%08x) wrong state(%d) in TMT for deletion!!!\n",
                        (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name, (UINT32)tlmt, tlmt->state);
                    tlmt->state = tlmstate_delete;
            }

            /*
            ** If there are tasks on the work queue, process the timeouts:
            ** decrement the timer value and if zero, invoke the timeout
            ** event handler for that task
            */
            if ((pILT = tlmt->whead) != NULL)
            {
                while (pILT != NULL)
                {
                    ilt = pILT;
                    pILT = pILT->fthd;
                    if (BIT_TEST(resilk, port))
                    {
                         break;
                    }

                    ilt->ilt_scan.scan_tmr1 -= (ilt->ilt_scan.scan_tmr1 > 0) ? 1 : 0;
                    if (ilt->ilt_scan.scan_tmr1 == 0)
                    {
                        APL_TimeoutILT(ilt, pICIMT, tlmt->tmt, tlmt);
                        /*
                        ** A possible task switch in the isp layers could
                        ** potentially cream the next ILT before this task
                        ** is re-scheduled; resulting in freed pointer access.
                        ** Marshall ran into a similar situation with his
                        ** scary FE testing. So, added logic to validate the
                        ** ILT against tlmt's whead here...
                        */
                        for(ilt = tlmt->whead;
                                ((ilt != NULL) && (ilt != pILT));
                                ilt = ilt->fthd);
                        /*
                        ** If pILT is not found, it will be NULL and would
                        ** result in exiting the while loop
                        */
                        pILT = ilt;
                    }
                }
            }
            /*
            ** Process the abort queue only if the work queue is empty. This
            ** is to avoid the situation where the tasks that got put to the
            ** abort queue in the timeout handler will not be reprocessed
            ** immediately. For all the tasks in the abort queue call abort
            ** handler.
            */
            else if ((pILT = tlmt->ahead) != NULL)
            {
                while (pILT != NULL)
                {
                    ilt = pILT;
                    pILT = pILT->fthd;

                    if (BIT_TEST(resilk, port))
                    {
                         break;
                    }

                    ilt->ilt_scan.scan_tmr1 -= (ilt->ilt_scan.scan_tmr1 > 0) ? 1 : 0;
                    if (ilt->ilt_scan.scan_tmr1 == 0)
                    {
                        APL_AbortILT(ilt, pICIMT, tlmt->tmt, tlmt);
                        /*
                        ** A possible task switch in the isp layers could
                        ** potentially cream the next ILT before this task
                        ** is re-scheduled; resulting in freed pointer access.
                        ** Marshall ran into a similar situation with his
                        ** scary FE testing. So, added logic to validate the
                        ** ILT against tlmt's ahead here...
                        */
                        for(ilt = tlmt->ahead;
                                ((ilt != NULL) && (ilt != pILT));
                                ilt = ilt->fthd);
                        /*
                        ** If pILT is not found, it will be NULL and would
                        ** result in exiting the while loop
                        */
                        pILT = ilt;
                    }
                }

            }

            /*
            ** If the TLMT state is delete and if the work and abort queues
            ** are empty, remove it.
            */
            if ((tlmt->state == tlmstate_delete)
                    && (tlmt->whead == NULL)
                    && (tlmt->ahead == NULL))
            {
                I_removeLUN(tlmt, tlmt->tmt, pICIMT);
            }
        }

        /* Now process timeouts for the TMTs. */
        pTMT = pICIMT->tmtQ;
        while (pTMT != NULL)
        {
            tmt = pTMT;
            pTMT = pTMT->tmtLink;
            if (tmt->tmr0 > 0)
            {
                tmt->tmr0 = (tmt->tmr0 == 1) ? tmt->tmr0 : tmt->tmr0 - 1;
                if(tmt->tmr0 == 1)
                {
                    I_RemoveTarget(tmt);
                }
            }
        }
    }
}

/**
******************************************************************************
**
**  @brief      Change Notification (RSCN) processing routine
**
**              1. If online discovery task is active, return
**              2. If AF_PORT pid, retrieve the TMT and set badboy flag bit
**              3. If AF_AREA pid, retrieve the TMTs and set badboy flag bit
**              4. call ganLoop to discover/rediscover the targets
**              5. Remove the TMT(s) that are still marked as badboys
**
**  @param      UINT16  - port
**              UINT32  - alpa
**
**  @return     none
**
******************************************************************************
**/
void I_ChangeNotification(UINT16 port, UINT32 pid)
{
    TMT *pTMT;

    /* If discovery due to online is in progress, ignore the event */
    if (I_CIMT_dir[port]->pDisc != NULL)
    {
        return;
    }
    /*
    ** Mark the respective TMT(s), if found as BADBOY(s). The GanLoop will
    ** reset the BADBOY flagbit if the target is discovered and runs a
    ** target scan.
    */
    if (PID_AF(pid) == AF_PORT)
    {
        /*
        ** Address Format type 0 pid, find the TMT for the given alpa/pid
        ** and mark it as BADBOY
        */
        for(pTMT = I_CIMT_dir[port]->tmtQ; pTMT != NULL; pTMT = pTMT->tmtLink)
        {
            if ((PORT_ALPA(pid) == pTMT->alpa)
                    && (pTMT->state != tmstate_deltar))
            {
                BIT_SET(pTMT->flag, TMFLG_BADBOY);
                break;
            }
        }
    }
    else if (PID_AF(pid) == AF_AREA)
    {
        /*
        ** Address Format type 1 pid, find all the TMTs that match the
        ** given type 0 alpa/pid and mark them as BADBOYs
        */
        for(pTMT = I_CIMT_dir[port]->tmtQ; pTMT != NULL; pTMT = pTMT->tmtLink)
        {
            if ((AREA_ALPA(pid) == AREA_ALPA(pTMT->alpa))
                    && (pTMT->state != tmstate_deltar))
            {
                BIT_SET(pTMT->flag, TMFLG_BADBOY);
            }
        }

    }
    else if (PID_AF(pid) == AF_DOMAIN)
    {
        /*
        ** Address Format type 2 pid, find all the TMTs that match the
        ** given type 0 alpa/pid and mark them as BADBOYs
        */
        for(pTMT = I_CIMT_dir[port]->tmtQ; pTMT != NULL; pTMT = pTMT->tmtLink)
        {
            if ((DOM_ALPA(pid) == DOM_ALPA(pTMT->alpa))
                    && (pTMT->state != tmstate_deltar))
            {
                BIT_SET(pTMT->flag, TMFLG_BADBOY);
            }
        }

    }
    else
    {
        /* Address Format type 3 pid, Mark all the TMTs on the port as BADBOYs */
        fprintf (stderr, "[%s:%d\t%s]: port:%d pid:0x%08x alpa:0x%08x FABRIC ADDRESS FORMAT 3!!!\n",__FILE__, __LINE__,__func__ ,port, pid, PORT_ALPA(pid));
        for(pTMT = I_CIMT_dir[port]->tmtQ; pTMT != NULL; pTMT = pTMT->tmtLink)
        {
            if (pTMT->state != tmstate_deltar)
            {
                BIT_SET(pTMT->flag, TMFLG_BADBOY);
            }
        }
    }

    /* GAN Loop function */
    I_GanLoop(port, pid);

    /*
    ** Search if any BADBOY TMTs are still present and delete them. Compare the
    ** pid/alpa to make sure that the TMTs marked as badboy(s) by this function
    ** before ganloop are the ones being deleted.
    */
    pTMT = I_CIMT_dir[port]->tmtQ;
    while (pTMT != NULL)
    {
        if (BIT_TEST(pTMT->flag, TMFLG_BADBOY) &&
            (((PID_AF(pid) == AF_PORT) && (pTMT->alpa == PORT_ALPA(pid))) ||
             ((PID_AF(pid) == AF_AREA) && (AREA_ALPA(pTMT->alpa) == AREA_ALPA(pid))) ||
             ((PID_AF(pid) == AF_DOMAIN) && (DOM_ALPA(pTMT->alpa) == DOM_ALPA(pid))) ||
             (PID_AF(pid) == AF_FABRIC)))
        {
            /* Mark TMT for deletion by timer task. */
            pTMT->tmr0 = TARGET_DEL;
        }
        pTMT = pTMT->tmtLink;
    }
}

/**
******************************************************************************
**
**  @brief      Change DB processing routine
**
**              This function handles the portdb change notification for the port
**
**  @param      UINT16  - port
**              UINT16  - lid
**              UINT16  - code:
**                        4 - PLOGI complete
**                        6 - PRLI complete
**                        7 - Port logged out
**
**  @return     none
**
******************************************************************************
**/
void I_ChangeDB(UINT16 port, UINT16 lid, UINT16 code)
{
    UINT32 rVal;
    TMT *pTMT = I_CIMT_dir[port]->tmdir[lid];

    /* If discovery due to online is in progress, ignore the event */
    if (I_CIMT_dir[port]->pDisc != NULL)
    {
        return;
    }

    /* If the lid is not valid or if we do not have a target for this lid, ignore */
    if ((lid == NO_LID) || (lid > MAX_DEV))
    {
        return;
    }

    /* Update the portdb array */
    if ((rVal = ISP_GetPortDB(port, lid, 0)) != ISP_CMDC)
    {
//        fprintf (stderr, "[%s:%d\t%s]: GetPortDB: %d 0x%x %d rVal(0x%x) tmt(0x%08x)\n",__FILE__, __LINE__,__func__ ,port, lid, code, rVal, (UINT32)pTMT);
        return;
    }
    if(code == 4)
    {
        /*
        ** PLOGI complete! If TMT is NULL for (the lid, evaluate the wwns in
        ** the portdb and create new TMT for scan if required
        */
        if(pTMT == NULL)
        {
            /*
            ** Check to see if there exists a TMT for the corresponding ALPA. If it does,
            ** ignore this event and exit.
            */
            UINT32 alpa = bswap_32(isp_handle2alpa(port, lid));

            for ( pTMT = I_CIMT_dir[port]->tmtQ; pTMT != NULL; pTMT = pTMT->tmtLink )
            {
                if (( pTMT->alpa == alpa )
                        && (( portdb[port][lid].pdn == pTMT->P_name )
                        && ( portdb[port][lid].ndn == pTMT->N_name )))
                {
#ifndef PERF
                    fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: changeDB - GanLoop in progress!!!!\n",
                        (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name);
#endif
                    break;
                }
            }

            /*
            ** If the WWNs belong to one of our own DCN or if not a XIO node or
            ** if not SCSI_FCP type, we are not interested!
            ** Also check if this is a XIO BE ports - TODO
            */
            if((pTMT == NULL)
                    && (ISP_IsMyWWN((void *)&portdb[port][lid].pdn) != TRUE)
                    && (ISP_IsMyWWN((void *)&portdb[port][lid].ndn) != TRUE)
                    && (M_chk4XIO(portdb[port][lid].pdn) != 0))
            {
                TLMT *pTLMT;

                /* TMT not found - assign a new one and start LUN Scan task on it. */
                pTMT = get_tmt();
#ifdef M4_DEBUG_TMT
fprintf(stderr, "%s%s:%u get_tmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pTMT);
#endif /* M4_DEBUG_TMT */
                memset((void *)pTMT, 0, sizeof(TMT));
                pTMT->icimtPtr = I_CIMT_dir[port];
                pTMT->state    = tmstate_discovery;
                pTMT->chipID   = port;
                pTMT->lid      = lid;
                pTMT->alpa     = bswap_32(isp_handle2alpa(port, lid));
                pTMT->N_name   = portdb[port][lid].ndn;
                pTMT->P_name   = portdb[port][lid].pdn;
                pTMT->tmtLink  = I_CIMT_dir[port]->tmtQ;
                I_CIMT_dir[port]->tmtQ   = pTMT;

                /* Allocate a TLMT for default lun (0) */
                pTLMT = get_tlmt();
#ifdef M4_DEBUG_TLMT
fprintf(stderr, "%s%s:%u get_tlmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pTLMT);
#endif /* M4_DEBUG_TLMT */
                memset((void *)pTLMT, 0, sizeof(TLMT));
                pTLMT->tmt   = pTMT;
                pTLMT->icimt = pTMT->icimtPtr;
                pTLMT->lun   = 0;
                pTLMT->state = tlmstate_discovery;
                pTMT->tlmtDir[0] = pTLMT;
                I_queTLMT(I_CIMT_dir[port], pTLMT);

//                fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: ChangeDB - New Target\n",
//                        (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name);

                CT_fork_tmp = (unsigned long)"tScan";
                BIT_SET(pTMT->flag, TMFLG_DISC);
                TaskCreate3(C_label_referenced_in_i960asm(tScan), I_DISPRI, (UINT32)pTMT);
            }
        }
        else if ((portdb[port][lid].pdn == pTMT->P_name)
                && (portdb[port][lid].ndn == pTMT->N_name))
        {
            /*
            ** we have the TMT that matches the Node's WWNs in the portdb.
            ** If alpa changed, initiate a rescan
            */
            BIT_CLEAR(pTMT->flag, TMFLG_BADBOY);
            if (isp_handle2alpa(port, lid) != bswap_32(pTMT->alpa))
            {
//                fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: ChangeDB Rescan: 0x%04x:0x%08x\n",
//                        (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name,
//                        lid,  bswap_32(isp_handle2alpa(port, lid)));

                if (pTMT->state == tmstate_discovery)
                {
                    /*
                    ** Alpa or LID changed! Update the SCAN state to restart
                    ** and update the scan state.
                    */
                    if(pTMT->pILT != NULL)
                    {
                        pTMT->alpa = bswap_32(isp_handle2alpa(port, lid));
                        pTMT->pILT->ilt_scan.scan_state = SCAN_RESTART;
                    }
                }
                else
                {
                    if (pTMT->state != tmstate_active)
                    {
                        pTMT->state = tmstate_discovery;
                        pTMT->lid   = lid;
                    }

                    /* Start the tScan task to initiate a target scan. */
                    pTMT->alpa  = bswap_32(isp_handle2alpa(port, lid));
                    CT_fork_tmp = (unsigned long)"tScan";
                    BIT_SET(pTMT->flag, TMFLG_DISC);
                    TaskCreate3(C_label_referenced_in_i960asm(tScan), I_DISPRI, (UINT32)pTMT);
                }
            }
        }
    }
    else if ((code == 7) && (pTMT != NULL))
    {
        /*
        ** Port Logged Out!!! If scan ILT is present in TMT, restart the scan
        ** else check the TMT state. If active and valid LID,
        ** Check if we have any scan task running for the TMT and restart it.
        ** The scan will retrieve the switch SNS data for the Node and if
        ** still present, will do a complete rescan. Otherwise, the TMT will
        ** be inactivated and deleted
        */
        BIT_CLEAR(pTMT->flag, TMFLG_BADBOY);
        if (pTMT->state == tmstate_discovery)
        {
            if (pTMT->pILT != NULL)
            {
                pTMT->pILT->ilt_scan.scan_state = SCAN_RESTART;
            }
        }
        else
        {
            if (pTMT->state != tmstate_active)
            {
                pTMT->state = tmstate_discovery;
                pTMT->lid   = NO_LID;
            }

            /* Start the tScan task */
            CT_fork_tmp = (unsigned long)"tScan";
            BIT_SET(pTMT->flag, TMFLG_DISC);
            TaskCreate3(C_label_referenced_in_i960asm(tScan), I_DISPRI, (UINT32)pTMT);
        }

//        fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: Port Logged Off\n",
//                (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name);
    }
}

/**
******************************************************************************
**
**  @brief      Port online event processing routine
**
**              This function identifies all the Initiator nodes on the
**              given port and forks one task for each Initiator node
**              to discover the targets.
**
**  @param      UINT16  - Port
**              UINT16  - LID
**
**  @return     none
**
******************************************************************************
**/
void I_Online(UINT16 port, UINT16 lid)
{
    TAR *pTAR = NULL;

//    fprintf (stderr, "[%s:%d\t%s]: port(0x%x)\n",__FILE__, __LINE__,__func__ ,port);

    I_CIMT_dir[port]->mylid  = lid;
    I_CIMT_dir[port]->state  = Cs_online;
    I_CIMT_dir[port]->mypid  = bswap_32(isp_handle2alpa(port, lid));

    /* If task is being created, wait. */
    while (I_CIMT_dir[port]->pTimer == (PCB *)-1)
    {
        TaskSleepMS(50);
    }

    /* If timer task is not created yet, spawn the task */
    if(I_CIMT_dir[port]->pTimer == NULL)
    {
        I_CIMT_dir[port]->pTimer = (PCB *)-1;       // Flag task being created.
        CT_fork_tmp = (unsigned long)"t_iTimer";
        I_CIMT_dir[port]->pTimer  = (PCB *)TaskCreate3(C_label_referenced_in_i960asm(t_iTimer), I_TIMERPRI, port);
    }

    /* If task is being created, wait. */
    while (I_CIMT_dir[port]->pDisc == (PCB *)-1)
    {
        TaskSleepMS(50);
    }

    /* If no active discovery task is running, create a new one */
    if (I_CIMT_dir[port]->pDisc == NULL)
    {
        I_CIMT_dir[port]->pDisc = (PCB *)-1;        // Flag task being created.
        CT_fork_tmp = (unsigned long)"t_fcDiscover";
        I_CIMT_dir[port]->pDisc  = (PCB *)TaskCreate4(C_label_referenced_in_i960asm(t_fcDiscover), I_SCANPRI, port, (UINT32)pTAR);
    }

    /* Update the CIMT state */
    cimtDir[port]->iState = CIS_ON;
}

/**
******************************************************************************
**
**  @brief      Port offline event processing routine
**
**              This function handles the port offline event for the Initiator nodes.
**              All the TMTs and the corresponding TLMTs' states are updated to inactive
**              and all the tScan tasks are terminated.
**
**  @param      UINT16  - Port
**
**  @return     none
**
******************************************************************************
**/
void I_Offline(UINT16 port)
{
    TMT *pTMT;

//    fprintf (stderr, "[%s:%d\t%s]: port(0x%x)\n",__FILE__, __LINE__,__func__ ,port);

    I_CIMT_dir[port]->state = Cs_offline;

    pTMT = I_CIMT_dir[port]->tmtQ;

    /* Set the states of all the TMTs for deletion. */
    while (pTMT != NULL)
    {
        /* Mark TMT for deletion by timer task. */
        pTMT->tmr0 = TARGET_DEL;
        pTMT = pTMT->tmtLink;
    }

    /* Update the CIMT state */
    cimtDir[port]->iState = CIS_OFF;

    /*
    ** If discovery task is running, change the OCB state to active
    ** so that the task can exist due to port offline
    */
    if (I_CIMT_dir[port]->pDisc != NULL)
    {
        if (TaskGetState(I_CIMT_dir[port]->pDisc) == PCB_NOT_READY)
        {
#ifdef HISTORY_KEEP
CT_history_pcb("I_Offline setting ready pcb", (UINT32)I_CIMT_dir[port]->pDisc);
#endif
            TaskSetState(I_CIMT_dir[port]->pDisc, PCB_READY);
        }
    }
}

/**
******************************************************************************
**
**  @brief      Removes the given TMT from the system
**
**              Aborts any outstanding SCAN for the target; checks with LLD
**              if OK to delete the TMT and takes appropriate actions to
**              delete the TMT
**
**  @param      TMT *pTMT
**
**  @return     none
**
******************************************************************************
**/
void I_RemoveTarget(TMT *pTMT)
{
    UINT32 i;
    ILT *pILT = NULL;
    TMT *tmt = NULL;
    ICIMT *pICIMT = I_CIMT_dir[pTMT->chipID];

    /* If the TMT is already prepared for removal, skip the preparation part */
    if(pTMT->state != tmstate_deltar)
    {
        fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llx]: Removing Target\n",
            (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name);
        /*
        ** If the discovery is in progress, retrieve the scan ILT for this TMT
        ** and abort it
        */
        if (pTMT->pILT != NULL)
        {
            pTMT->pILT->ilt_scan.scan_state = SCAN_ABORT;
        }

        /*
        ** Zero the WWNs so that it will not be picked and processed
        ** on ISP events while it is being deleted. If the TMT lid is
        ** valid, clear the TMT from ICIMT's tmdir and release the lid
        ** to the free list. Set the TMT timer to TARGET_DEL (1) so that
        ** the apl timer task will retry to delete the target in the next
        ** pass in case the LLD is not ready to delete the target yet
        ** and update the tmt state
        */
        pTMT->P_name = 0x0;
        pTMT->N_name = 0x0;
        pTMT->tmr0 = TARGET_DEL;
        pTMT->state = tmstate_deltar;
        BIT_CLEAR(pTMT->flag, TMFLG_BADBOY);
        if(pTMT->lid != NO_LID)
        {
            /*
            ** If the port is online, logout to clear out all outstanding reqs
            ** in the qlogic
            */
            if (BIT_TEST(ispOnline,pTMT->chipID))
            {
                ISP_LogoutFabricPort(pTMT->chipID, pTMT->lid, pICIMT->mypid);
            }
            pICIMT->tmdir[pTMT->lid] = NULL;
        }

        /*
        ** for each TLMT (lun) present, reset the req timeout for all the
        ** outstanding reqs in the work queue. The timer task will clean these
        ** up for us. Also update the tlmt state
        */
        for(i = 0; i < MAX_LUNS; i++)
        {
            if(pTMT->tlmtDir[i] != NULL)
            {
                pTMT->tlmtDir[i]->state = tlmstate_delete;
                for(pILT = pTMT->tlmtDir[i]->whead; pILT != NULL; pILT = pILT->fthd)
                {
                    pILT->ilt_scan.scan_tmr1 = 0;
                }
            }
        }
    }

    /* Check with LLD if it is OK to delete the TMT. */
    if ((BIT_TEST(pTMT->flag, TMFLG_DISC) == FALSE)
                            && (LLD_CheckTargetDel(pTMT)))
    {
        /* for each TLMT (lun) present, call removeLUN to clean up */
        for(i = 0; i < MAX_LUNS; i++)
        {
            if(pTMT->tlmtDir[i] != NULL)
            {
                return;
            }
        }
        fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: RemoveTarget\n",
            (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name);

        /* Remove TMT from the ICIMT's tmtQ list */
        if(pICIMT->tmtQ == pTMT)
        {
            pICIMT->tmtQ = pTMT->tmtLink;
        }
        else
        {
            for(tmt = pICIMT->tmtQ; tmt != NULL; tmt = tmt->tmtLink)
            {
                if(tmt->tmtLink == pTMT)
                {
                    tmt->tmtLink = pTMT->tmtLink;
                    pTMT->tmtLink = NULL;
                    break;
                }
            }
        }

        /* Inform LLD that the target is gone and release the TMT */
        LLD_TargetOffline(pTMT);
        I_put_lid(pTMT->chipID, pTMT->lid);
#ifdef M4_DEBUG_TMT
fprintf(stderr, "%s%s:%u put_tmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pTMT);
#endif /* M4_DEBUG_TMT */
        put_tmt(pTMT);
    }
}

/**
******************************************************************************
**
**  @brief      Rescan a given target
**
**              This function initiates a rescan for a given target
**
**  @param      TMT *pTMT
**
**  @return     none
**
******************************************************************************
**/
void I_Rescan(TMT *pTMT)
{
//    fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: I_Rescan\n",
//            (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name);
    /*
    ** If the TMT is already in discovery state, reset the scan ilt state
    ** to restart. Otherwise, launch a new scan task.
    */
    if(pTMT->state == tmstate_discovery)
    {
        if (pTMT->pILT != NULL)
        {
            pTMT->pILT->ilt_scan.scan_state = SCAN_RESTART;
        }
    }
    else
    {
        if (pTMT->state != tmstate_active)
        {
            pTMT->state = tmstate_discovery;
        }
        CT_fork_tmp = (unsigned long)"tScan";
        BIT_SET(pTMT->flag, TMFLG_DISC);
        TaskCreate3(C_label_referenced_in_i960asm(tScan), I_DISPRI, (UINT32)pTMT);
    }
}

/**
******************************************************************************
**
**  @brief      Discovers the targets visible to the (Initiator) node on iSCSI port
**
**              Retrieves the GeoRep configuration and does a 'Send Targets'
**              discovery.
**              For each target that is discovered, creates a TMT (if does not
**              exist) and forks a task per TMT to scan for LUNs.
**
**  @param      UINT16     - Port
**              TAR *pTAR  - Target Node pointer
**
**  @return     none
**
******************************************************************************
**/
void t_iscsiDiscover(UINT32 a UNUSED, UINT32 b UNUSED, UINT16 port, TAR *pTAR)
{
   /* TODO */

    /*
    **  1. Create TMTs for the mirror partner targets
    **  2. for each TMT fork a scan task for LUN scan.
    **  3. for each configured GeoRep_i IP do 'SendTargets' discovery
    **  4. for each target discovered, create TMT and fork
    **     a scan task for LUN scan.
    */
    fprintf (stderr, "[%s:%d\t%s]: port(0x%x) tar(0x%x)\n",__FILE__, __LINE__,__func__ ,port, (UINT32)pTAR);
}

/**
******************************************************************************
**
**  @brief      Discovers the targets visible to the (Initiator) node on FC port
**
**              Marks all the TMTs on the port as BADBOYs, runs ganloop
**              and then removes the TMTs that are still BADBOYs
**
**  @param      UINT16     - Port
**              TAR *pTAR  - Target Node pointer
**
**  @return     none
**
******************************************************************************
**/
void t_fcDiscover(UINT32 a UNUSED, UINT32 b UNUSED, UINT16 port, TAR *pTAR)
{
    TMT *pTMT;
    UINT32 count = 0;

//    fprintf (stderr, "[%s:%d\t%s]: port(0x%x) tar(0x%x)\n",__FILE__, __LINE__,__func__ ,port, (UINT32)pTAR);

    /* Set all the TMTs on the port as BADBOYs */
    for(pTMT = I_CIMT_dir[port]->tmtQ; pTMT != NULL; pTMT = pTMT->tmtLink)
    {
        if(pTMT->state != tmstate_deltar)
        {
            BIT_SET(pTMT->flag, TMFLG_BADBOY);
        }
    }

    /* Wait for 250ms to let the fabric settle */
    TaskSleepMS(SCAN_DELAY);

    /* While the port is still ONLINE... */
    while(BIT_TEST(ispOnline,port))
    {
        /*
        ** Register the Initiator for this Node and run ganloop. If the
        ** ganloop is successful, exit the loop. Otherwise, delay and then
        ** repeate the registration and ganloop operations
        */
        isp_registerFc4(port, 0x100, pTAR);
        if(I_GanLoop(port, 0) == ISP_CMDC)
        {
            break;
        }

        /*
        ** If the retries exceed the count, bail out. This is to avoid the
        ** situation where the logic gets into a continues loop when GAN fails
        ** In this case, there is something wrong with the switch and we want
        ** to avoid the forever loop. If the switch condition is rectified,
        ** we will in any case get new switch events on which we can always
        ** discover the targets. Added this logic after Marshall ran into
        ** such situation on his scary FE setup!
        */
        count++;
        if(count > GAN_RETRY_CNT)
        {
            break;
        }
        TaskSleepMS(SCAN_DELAY/2);
    }

    /* Delete the TMTs that are still badboys!!! */
    pTMT = I_CIMT_dir[port]->tmtQ;
    while (pTMT != NULL)
    {
        if (BIT_TEST(pTMT->flag, TMFLG_BADBOY))
        {
            /* Mark TMT for deletion by timer task. */
            pTMT->tmr0 = TARGET_DEL;
        }
        pTMT = pTMT->tmtLink;
    }
    I_CIMT_dir[port]->pDisc = NULL;
}


/**
******************************************************************************
**
**  @brief      The GAN-LOOP
**
**              This function retrieves the complete information of
**              all the nodes registered with the switch's SNS, and for
**              each XIO target discovered, creates/updates the TMT and
**              spawns a tScan task.
**
**  @param      UINT16     - Port
**              UINT32     - PID
**
**  @return     none
**
******************************************************************************
**/

UINT32 I_GanLoop(UINT16 port, UINT32 pid)
{
    UINT32 start_pid = 0;
    UINT32 rVal = ISP_CMDC;
    CT_GAN_RESP *pGanRsp = NULL;

    pGanRsp = (CT_GAN_RESP *)s_MallocC(sizeof(CT_GAN_RESP), __FILE__, __LINE__);
    memset((void *)pGanRsp, 0, sizeof(CT_GAN_RESP));

    /*
    ** If the alpa supplied is not 0, initialize the starting alpa
    ** to 1 - the given alpa. The GAN will retrieve the information
    ** of the next valid alpa's details from SNS
    */
    if ((pid != 0) && (PID_AF(pid) != AF_FABRIC))
    {
        UINT32  alpa_start = bswap_32(PORT_ALPA(pid)) - 1;

        pGanRsp->gan_pid = bswap_32(alpa_start);
    }

    /* While the port is still online.... */
    while(BIT_TEST(ispOnline,port))
    {
        rVal = isp2400_sendctGAN(port, (UINT32)pGanRsp, bswap_32(PORT_ALPA(pGanRsp->gan_pid)));
        if (rVal != ISP_CMDC)
        {
            fprintf (stderr, "[%s:%d\t%s]: port(0x%x) pid(0x%x) rVal(0x%x)\n",__FILE__, __LINE__,__func__ ,port, pid, rVal);
            break;
        }

        /* Validate the port type here */
        if(PORT_ALPA(pGanRsp->gan_pid) == 0x0)
        {
            /* Invalid port ID, return error; */
            fprintf (stderr, "[%s:%d\t%s]: port(0x%x) pid(0x%x)\n",__FILE__, __LINE__,__func__ ,port, pid);
            rVal = ISP_CMDE;
            break;
        }
        if ((pid == 0) || (PID_AF(pid) == AF_FABRIC))
        {
            if (start_pid == 0)
            {
                /* Initialize the start PID for the loop */
                start_pid = PORT_ALPA(pGanRsp->gan_pid);
            }
            else if(PORT_ALPA(pGanRsp->gan_pid) == start_pid)
            {
                /* End of the GAN SCAN! Exit */
                break;
            }
        }
        else
        {
            /*
            ** If initial alpa is specified (non zero), check the PID in the
            ** GAN response. If it falls outside the scope of the ganloop
            ** request, exit the loop.
            */
            if (((PID_AF(pid) == AF_PORT) && (PORT_ALPA(pGanRsp->gan_pid) != PORT_ALPA(pid)))
                    || ((PID_AF(pid) == AF_AREA) && (AREA_ALPA(pGanRsp->gan_pid) != AREA_ALPA(pid)))
                    || ((PID_AF(pid) == AF_DOMAIN) && (DOM_ALPA(pGanRsp->gan_pid) != DOM_ALPA(pid))))
            {
                break;
            }
        }

        /*
        ** If the WWNs belong to one of our own DCN or if not a XIO node or
        ** if not SCSI_FCP type, we are not interested!
        ** Also check if this is a XIO BE ports - TODO
        */
        if ((ISP_IsMyWWN((void *)&pGanRsp->gan_Nname) != TRUE)
                && (ISP_IsMyWWN((void *)&pGanRsp->gan_Pname) != TRUE)
                && (pGanRsp->gan_fc4Types[2] == SCSI_FCP)
                && (M_chk4XIO(pGanRsp->gan_Pname) != 0 || FT_DEFINED != 0))
        {
            TMT *pTMT = NULL;

            /* Search the corresponding TMT from the ICIMT list */
            for(pTMT = I_CIMT_dir[port]->tmtQ; pTMT != NULL; pTMT = pTMT->tmtLink)
            {
                if((pGanRsp->gan_Pname == pTMT->P_name)
                        && (pGanRsp->gan_Nname == pTMT->N_name))
                {
                    BIT_CLEAR(pTMT->flag, TMFLG_BADBOY);

                    /*
                    ** Found the TMT - when in discovery or active state if the alpa
                    ** did not change, we don't have anything to do!
                    */
                    if (((pTMT->state == tmstate_discovery)
                            || (pTMT->state == tmstate_active))
                            && (pTMT->alpa == PORT_ALPA(pGanRsp->gan_pid)))
                    {
                        break;
                    }
                    fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: GanLoop: %d - 0x%08x\n",
                                (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name,
                                pTMT->state, PORT_ALPA(pGanRsp->gan_pid));

                    /*
                    ** We have to scan the the target. If already in discovery, change
                    ** the state to RESTART. Otherwise, update the tmt state and
                    ** start the tScan task for this TMT.
                    */
                    pTMT->alpa  = PORT_ALPA(pGanRsp->gan_pid);
                    pTMT->lid   = NO_LID;

                    if(pTMT->state == tmstate_discovery)
                    {
                        if (pTMT->pILT != NULL)
                        {
                            pTMT->pILT->ilt_scan.scan_state = SCAN_RESTART;
                        }
                    }
                    else
                    {
                        /* Start the tScan task to initiate a target scan. */
                        pTMT->state = tmstate_discovery;
                        CT_fork_tmp = (unsigned long)"tScan";
                        BIT_SET(pTMT->flag, TMFLG_DISC);
                        TaskCreate3(C_label_referenced_in_i960asm(tScan), I_DISPRI, (UINT32)pTMT);
                    }
                break;
                }
            }

            /* If TMT not found. Create a new TMT and spawn off a SCAN task */
            if(pTMT == NULL)
            {
                TLMT *pTLMT;

                /* TMT not found - assign a new one and start LUN Scan task on it.  */
                pTMT = get_tmt();
#ifdef M4_DEBUG_TMT
fprintf(stderr, "%s%s:%u get_tmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pTMT);
#endif /* M4_DEBUG_TMT */
                memset((void *)pTMT, 0, sizeof(TMT));
                pTMT->icimtPtr = I_CIMT_dir[port];
                pTMT->state    = tmstate_discovery;
                pTMT->chipID   = port;
                pTMT->lid      = NO_LID;
                pTMT->alpa     = PORT_ALPA(pGanRsp->gan_pid);
                pTMT->N_name   = pGanRsp->gan_Nname;
                pTMT->P_name   = pGanRsp->gan_Pname;
                pTMT->tmtLink  = I_CIMT_dir[port]->tmtQ;
                I_CIMT_dir[port]->tmtQ   = pTMT;

                /* Allocate a TLMT for default lun (0) */
                pTLMT = get_tlmt();
#ifdef M4_DEBUG_TLMT
fprintf(stderr, "%s%s:%u get_tlmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pTLMT);
#endif /* M4_DEBUG_TLMT */
                memset((void *)pTLMT, 0, sizeof(TLMT));
                pTLMT->tmt   = pTMT;
                pTLMT->icimt = pTMT->icimtPtr;
                pTLMT->lun   = 0;
                pTLMT->state = tlmstate_discovery;
                pTMT->tlmtDir[0] = pTLMT;
                I_queTLMT(I_CIMT_dir[port], pTLMT);

//                fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: GanLoop - New Target\n",
//                        (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name);

                CT_fork_tmp = (unsigned long)"tScan";
                BIT_SET(pTMT->flag, TMFLG_DISC);
                TaskCreate3(C_label_referenced_in_i960asm(tScan), I_DISPRI, (UINT32)pTMT);
            }
        }
    }

    /* Clean up the resources used */
    s_Free((void *)pGanRsp, sizeof(CT_GAN_RESP), __FILE__, __LINE__);
    return(rVal);
}

/**
******************************************************************************
**
**  @brief      scan callback function
**
**              Callback function for the SCSI reqs issued to scan the LUNs
**
**  @param      UINT32 status
**              ILT *pILT
**
**  @return     none
**
**  @attention  This is a task
**
******************************************************************************
**/

void cbScan(UINT32 status, ILT *pILT)
{
    /*
    ** Update the status to the ILT and READY the task tscan task - the pcb is
    ** contained within the ILT
    */
    if(pILT->misc)
    {
        pILT->ilt_scan.scan_status = status;
        if (TaskGetState((PCB *)pILT->misc) == PCB_NOT_READY)
        {
            TaskSetState((PCB *)pILT->misc, PCB_READY);
#ifdef HISTORY_KEEP
CT_history_pcb("cbScan setting ready pcb", (UINT32)pILT->misc);
#endif
        }
    }
    else
    {
        /*
        ** This should never happen if everything works right!!! Add a
        ** debug statement here!
        */
        fprintf (stderr, "[%s:%d\t%s]:tScan pcb is NULL in ILT 0x%08x!!\n",__FILE__, __LINE__,__func__, (UINT32)pILT);
        fprintf (stderr, "[%s:%d\t%s]0x%08x: stus:0x%08x stat:%d tag:0x%04x rtry:%d t1:%d t2:%d tmt:0x%08x xli:0x%08x\n",
                                        __FILE__, __LINE__, __func__,(UINT32)pILT,
                                        pILT->ilt_scan.scan_status,
                                        pILT->ilt_scan.scan_state,
                                        pILT->ilt_scan.scan_tagID,
                                        pILT->ilt_scan.scan_retry,
                                        pILT->ilt_scan.scan_tmr1,
                                        pILT->ilt_scan.scan_tmr2,
                                        pILT->ilt_scan.scan_tmt,
                                        pILT->ilt_scan.scan_xli);
    }
}


/**
******************************************************************************
**
**  @brief      Scans for LUNs behind a given target node
**
**              This is the main target scan task. It logs in to the given
**              target and scans for LUNs. TLMTs are created for each LUN
**              discovered. Once the Scan is complete, updates the LLD
**
**  @param      TMT *pTMT
**
**  @return     none
**
**  @attention  This is a task
**
******************************************************************************
**/
void tScan(UINT32 a UNUSED, UINT32 b UNUSED, TMT *pTMT)
{
    UINT32 i;
    ILT *pILT = NULL;
    XLI *pXLI = NULL;
    SGL *pSGL = NULL;
    SGL_DESC *pDESC = NULL;
    UINT16 port = pTMT->chipID;
    ICIMT *pICIMT = I_CIMT_dir[port];

    if (pTMT->state == tmstate_deltar)
    {
        BIT_CLEAR(pTMT->flag, TMFLG_DISC);
        return;
    }

    /*
    ** Allocate the discovery resources for this TMT and
    ** initialize the various fields
    */
    pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
    pSGL = m_asglbuf(DISC_BUFSIZE);
    pXLI = get_xli();
#ifdef M4_DEBUG_XLI
fprintf(stderr, "%s%s:%u get_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pXLI);
#endif /* M4_DEBUG_XLI */
    pDESC = (SGL_DESC *)(pSGL + 1);

    pXLI->xlisglnum_sav = pSGL->scnt;
    pXLI->xlisglptr     = (SGL *)pDESC;
    pXLI->xlicdbptr     = (struct CDB *)pXLI->xlicdb;
    pXLI->xlitarget     = pTMT->lid;
    pXLI->xlichipid     = port;
    pXLI->xlilun        = 0;
    pXLI->xlitime       = 5;
    pXLI->xlidatadir    = XLI_DATA_READ;
    pXLI->xlifcflgs     = (1 << XLISIMQ);

    pILT->cr            = (void *)C_label_referenced_in_i960asm(cbScan);
    pILT->misc          = (UINT32)K_xpcb;
    pILT->ilt_scan.scan_tmt   = (UINT32)pTMT;
    pILT->ilt_scan.scan_xli   = (UINT32)pXLI;
    pILT->ilt_scan.scan_retry = SCAN_RETRY;
    pILT->ilt_scan.scan_nerr  = 0;
    pILT->ilt_scan.scan_state = (pTMT->state == tmstate_active) ? SCAN_RESTART : SCAN_INIT;

    pTMT->tmr0      = 0;
    pTMT->pILT      = pILT;

    for(i = 0; i < MAX_LUNS; i++)
    {
        if(pTMT->tlmtDir[i] != NULL)
        {
            pTMT->tlmtDir[i]->state = tlmstate_inactive;
        }
    }
    pICIMT->nscan   += 1;

    /* Main task loop */
    while (pILT->misc == (UINT32)K_xpcb)
    {
        switch(pILT->ilt_scan.scan_state)
        {
            case SCAN_RESTART:
            {
                /*
                ** update the scan state first - this is because some of the
                ** functions called from here could potentially taskswitch
                ** and this is to avoid overwriting the state which some
                ** other path could potentially change!
                */
                pILT->ilt_scan.scan_state = SCAN_INIT;

                /* If valid LID is present, clear the TMT entry in ICIMT */
                if (pTMT->lid != NO_LID)
                {
                    pICIMT->tmdir[pTMT->lid] = NULL;
                }
                /*
                ** If TMT is active, wait until the LLD is ready to for offline
                ** and inform the LLD that the target is offline
                */
                if (pTMT->state == tmstate_active)
                {
                    pTMT->state = tmstate_discovery;
                    while (LLD_CheckTargetDel(pTMT) == FALSE)
                    {
                        TaskSleepMS(100);
                    }
                    LLD_TargetOffline(pTMT);
                }
                break;
            }
            case SCAN_INIT:
            {
                /*
                ** Scan Init - delays the scan process to let the fabric
                ** to settle down and Switch's SNS db updated
                */
                pTMT->tmr0  = 0;
                pILT->ilt_scan.scan_state = SCAN_LOGIN;
                TaskSleepMS(SCAN_DELAY);
                break;
            }
            case SCAN_LOGIN:
            {
                /* login to the target node (PLOGI) */
                i_login(pILT);
                break;
            }
            case SCAN_TUR:
            {
                /* Process the TUR response */
                i_tur(pILT);
                break;
            }
            case SCAN_INQ:
            {
                /* Process the INQUIRY response */
                i_inq(pILT);
                break;
            }
            case SCAN_RLUN:
            {
                /* Process the REPORT LUNS response */
                i_rluns(pILT);
                break;
            }
            case SCAN_RC:
            {
                /* Process the READ CAPACITY response */
                i_rc(pILT);
                break;
            }
            case SCAN_DONE:
            {
                /*
                ** Scan complete. Inform the LLD that the target is online
                ** exit the scan task and clean up the resources
                */
                pTMT->state = tmstate_active;
                pTMT->pILT = NULL;
                LLD_TargetOnline(pTMT);
                for(i = 0; i < MAX_LUNS; i++)
                {
                    if ((pTMT->tlmtDir[i] != NULL)
                            && (pTMT->tlmtDir[i]->state == tlmstate_active))
                    {
                        APL_StartIO(pICIMT, pTMT->tlmtDir[i]);
                    }
                }
                pILT->misc = 0x0;
                break;
            }
            case SCAN_ABORT:
            {
                /*
                ** Terminate the scan task by exiting out of the main loop.
                ** The resources allocates will be cleared before exiting the
                ** task function
                */
                pTMT->pILT = NULL;
                pILT->misc = 0x0;
                break;
            }
            default:
            {
                break;
            }
        }
        if (TaskGetState(K_xpcb) != PCB_READY)
        {
            TaskSwitch();
        }
    }

    /* Release the resources */
    if(pILT->ilt_scan.scan_xli)
    {
        if(((XLI *)pILT->ilt_scan.scan_xli)->xlisglptr)
        {
            PM_RelSGLWithBuf(((SGL *)((XLI *)pILT->ilt_scan.scan_xli)->xlisglptr) - 1);
        }
#ifdef M4_DEBUG_XLI
fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT->ilt_scan.scan_xli);
#endif /* M4_DEBUG_XLI */
        put_xli((XLI*)(pILT->ilt_scan.scan_xli));
        pILT->ilt_scan.scan_xli = 0;
    }

    /* Remove the ILT from TMT and update the active scan count in ICIMT */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
    put_ilt(pILT);
    pICIMT->nscan = (pICIMT->nscan == 0) ? 0 : pICIMT->nscan - 1;
    BIT_CLEAR(pTMT->flag, TMFLG_DISC);
}

/*******************************************************************************************
***                        Response Processing functions                                 ***
*******************************************************************************************/

/**
******************************************************************************
**
**  @brief      Processes the Test Unit Ready response
**
**
**              For FC, issue the fabric login (PLOGI). If successful, update the
**              LID in the TMT and issue TUR for default LUN. If failed due to target
**              non-existing (PLOGI error), wait for 125(?)ms and reissue the login
**
**              For iSCSI, Login to the target IP. If successful, identify the sg
**              device associated with the target's default lun (0), open the device
**              save the handle as 'alpa' in the TMT and issue TUR cmd for default LUN.
**              If failed, take appropriate action based on the reason for failure.
**
**
**  @param      ILT*    - ILT
**
**  @return     none
**
******************************************************************************
**/

void i_login(ILT* pILT)
{
    UINT32 rVal = 0;
    UINT32 rParam = 0;
    TMT *pTMT = (TMT *)pILT->ilt_scan.scan_tmt;
    UINT16 port = pTMT->chipID;

    /* If lid not present, acquire one */
    if(pTMT->lid == NO_LID)
    {
        pTMT->lid = I_get_lid(port);
    }
    rParam = pTMT->lid;

    /* Login to the target */
    rVal = ISP_LoginFabricPort(pTMT->chipID, &rParam, bswap_32(pTMT->alpa));
    pILT->ilt_scan.scan_status = rVal;

    /* Check if the state is still LOGIN! This is to avoid the case where an
     * 'offline' event could have knocked out the TMT from under us and we
     * don't know about the aborted scan task until we get back to the tScan
     * main loop. I'm able to create this situation by resetting all FE ports
     * on all the DSCs' DCNs at the same time. */
    if (pILT->ilt_scan.scan_state != SCAN_LOGIN)
    {
        return;
    }

    switch(rVal & 0xffff)
    {
        case ISP_CMDC:
        {
            /* Update portdb and check if the corresponding node supports target
             * mode. If not, remove the TMT */
            if((rVal = ISP_GetPortDB(port, pTMT->lid, 0)) != ISP_CMDC)
            {
                fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: GetPortDB FAILED(0x%x)!!!\n",
                        (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name, rVal);
            }
            if(BIT_TEST(portdb[port][pTMT->lid].prliw3, 4))
            {
                /* Login success & node supports target mode; send the TUR cmd for lun 0 */
                pTMT->tmr0  = TARGET_TO;
                snd_tur(pILT, 0);
                TaskSetMyState(PCB_NOT_READY);
            }
            else if (pILT->ilt_scan.scan_state == SCAN_LOGIN)
            {
                /* Check if the state is still LOGIN! This is to avoid the case where an
                 * 'offline' event could have knocked out the TMT from under us and we
                 * don't know about the aborted scan task until we get back to the tScan
                 * main loop. I'm able to create this situation by resetting all FE ports
                 * on all the DSCs' DCNs at the same time. */
#if 0
                fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: No target support on the NODE!!!\n",
                        (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name);
#endif  /* 0 */

                /* Mark TMT for deletion by timer task. */
                pTMT->tmr0 = TARGET_DEL;

                pILT->ilt_scan.scan_state = SCAN_ABORT;
            }
        }
        break;
        case ISP_LIU:
        {
            /* Specified LID in use. Get a different lid and retry */
            pTMT->lid = I_get_lid(port);
        }
        break;
        case ISP_PIU:
        {
            /* Port using a different LID. The correct LID is returned in the
             * 'lid' param. Update the LID and reissue the login. */
            pTMT->lid = (UINT16)rParam;
        }
        break;
        default :
        {
            /* Error occured! Check to see if the corresponding target is still
             * registered with the Switch's SNS - after a delay */
            if(isp_GidPN(pTMT->chipID, pTMT->P_name, &rParam) == QLOGIC_STATUS_GOOD)
            {
                /* The target still exists - update the returned alpa in tmt and relogin */
                if (pILT->ilt_scan.scan_nerr % 32  == 0)
                {
                    /* Print once every 32 occurances */
                    fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: login ERROR (0x%x) GidPN SUCCESS (0x%0x)\n",
                            (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name, rVal, rParam);
                }
                pILT->ilt_scan.scan_nerr++;
                pTMT->alpa = rParam;
                pILT->ilt_scan.scan_status = SCS_NORM;
                pILT->ilt_scan.scan_state = SCAN_INIT;
            }
            else if (pILT->ilt_scan.scan_state == SCAN_LOGIN)
            {
                /* Check if the state is still LOGIN! This is to avoid the case where an
                 * 'offline' event could have knocked out the TMT from under us and we
                 * don't know about the aborted scan task until we get back to the tScan
                 * main loop. I'm able to create this situation by resetting all FE ports
                 * on all the DSCs' DCNs at the same time. Target does not exist - terminate
                 * the scan and TMT */
                fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: login ERROR (0x%x) GidPN FAILED!!!\n",
                        (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name, rVal);
                /* Mark TMT for deletion by timer task. */
                pTMT->tmr0 = TARGET_DEL;

                pILT->ilt_scan.scan_state = SCAN_ABORT;
            }
        }
    }
}

/**
******************************************************************************
**
**  @brief      Processes the Test Unit Ready response
**
**              if TUR failed, send start/stop unit (SSU) cmd to the target device.
**              if TUR successful, send INQ cmd to the target device.
**
**  @param      ILT*    - ILT
**
**  @return     none
**
******************************************************************************
**/
void i_tur(ILT* pILT)
{
    XLI *pXLI = (XLI *)pILT->ilt_scan.scan_xli;
    TMT *pTMT = (TMT *)pILT->ilt_scan.scan_tmt;

    /* If TUR success, send INQUIRY cmd */
    if ((pILT->ilt_scan.scan_status == SCS_NORM)
            || (pILT->ilt_scan.scan_status == SCS_RESC)
            || (pILT->ilt_scan.scan_status == EC_RES_CONFLICT))
    {
        snd_inq(pILT, pXLI->xlilun);
        TaskSetMyState(PCB_NOT_READY);
    }
    else if ((pILT->ilt_scan.scan_retry > 1)
            && ((pILT->ilt_scan.scan_status == SCS_ECHK)
            || (pILT->ilt_scan.scan_status == EC_CHECK)
            || (pILT->ilt_scan.scan_status == SCS_BUSY)
            || (pILT->ilt_scan.scan_status == EC_BUSY)))
    {
        /* If check condition, just reissue the cmd. We may have to validate the
         * check condition here! */
        ISP_GetPortDB(pTMT->chipID, pTMT->lid, 0);
        pILT->ilt_scan.scan_retry -= 1;
        snd_tur(pILT, pXLI->xlilun);
        TaskSetMyState(PCB_NOT_READY);
    }
    else
    {
        /* Bad error. Restart the login proces. A logout before login (in RESCAN
         * state) helps clear this issue most of the time. */
#ifndef PERF
        fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: tScan ILT(0x%x):%s %d 0x%x\n",
                (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name,
                (UINT32)pILT, stateStr[pILT->ilt_scan.scan_state],
                pXLI->xlilun, pILT->ilt_scan.scan_status);
#endif
        pILT->ilt_scan.scan_status = SCS_NORM;
        pILT->ilt_scan.scan_retry = SCAN_RETRY;
        pILT->ilt_scan.scan_state = SCAN_RESTART;
    }
}

/**
******************************************************************************
**
**  @brief      Processes the Inquiry response
**
**              On success update the vendor ID, product ID, version and SN in TLMT. if LUN = 0
**              (default LUN), update the vendor ID, product ID, version and SN in TMT.
**              If the peripheral qualifier is 0 & device type is 0 (direct access), issue
**              READCAPACITY cmd. Else, if LUN = 0, issue Report LUNs cmd - otherwise retrieve the
**              next TLMT from the TMT's tlmtDir (valid TLMT after this LUN in the array) and
**              issue TUR cmd
**
**  @param      ILT*    - ILT
**
**  @return     none
**
******************************************************************************
**/

void i_inq (ILT* pILT)
{
    UINT16 i;
    XLI *pXLI = (XLI *)pILT->ilt_scan.scan_xli;
    TMT *pTMT = (TMT *)pILT->ilt_scan.scan_tmt;
    TLMT *pTLMT = NULL;
    UINT8 *pData = ((SGL_DESC*)pXLI->xlisglptr)->addr;

    /* If INQ success, Process it */
    if ((pILT->ilt_scan.scan_status == SCS_NORM)
            ||  (pILT->ilt_scan.scan_status == EC_UNDERRUN))
    {
        /* If TLMT for (the LUN is missing, allocate a new TLMT and update it */
        if((pTLMT = pTMT->tlmtDir[pXLI->xlilun]) == NULL)
        {
            pTLMT = get_tlmt();
#ifdef M4_DEBUG_TLMT
fprintf(stderr, "%s%s:%u get_tlmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pTLMT);
#endif /* M4_DEBUG_TLMT */
            memset((void *)pTLMT, 0, sizeof(TLMT));
            pTLMT->tmt   = pTMT;
            pTLMT->icimt = pTMT->icimtPtr;
            pTLMT->lun   = pXLI->xlilun;
            I_queTLMT(pTMT->icimtPtr, pTLMT);
            pTMT->tlmtDir[pXLI->xlilun] = pTLMT;
        }
        pTLMT->state  = tlmstate_active;
        pTLMT->pdt    = pData[0] & 0x1F;
        if(pTLMT->pdt != 0 && pTLMT->pdt != 0x1f)   /* NOTE: 0x1f (31) is an ISE -- ignore it. */
        {
            UINT8 *p = pData;
            fprintf(stderr, "### BAD PERIPHERAL DEVICE TYPE (0x%x) in INQ response from wwn(0x%016llX) lun(%d) ###\n",
                                    pTLMT->pdt, pTMT->P_name, pTLMT->lun);
            fprintf(stderr, ">> %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11]);
            fprintf(stderr, ">> %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                       p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19], p[20], p[21], p[22], p[23]);
            fprintf(stderr, ">> %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                       p[24], p[25], p[26], p[27], p[28], p[29], p[30], p[31], p[32], p[33], p[34], p[35]);
            fprintf(stderr, ">> %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                       p[36], p[37], p[38], p[39], p[40], p[41], p[42], p[43], p[44], p[45], p[46], p[47]);
        }
        pTLMT->snlen  = 12;
        pTLMT->dvflgs = pData[7];

        /* get vendor ID, product ID, Version, SN from response */
        strncpy((char *)pTLMT->venid, (char *)&pData[8], 8);
        strncpy((char *)pTLMT->proid, (char *)&pData[16], 16);
        strncpy((char *)pTLMT->version, (char *)&pData[32], 4);
        strncpy((char *)pTLMT->sn, (char *)&pData[36], 12);
        if ((pData[36] | pData[37] | pData[38] | pData[39] | pData[40] | pData[41] | pData[42] | pData[43] | pData[44] | pData[45] | pData[46] | pData[47]) == 0) {
          *(UINT64*)pTLMT->sn = bswap_64(pTMT->P_name);
        }

        /* If default LUN (0), update the TMT with INQ data */
        if(pXLI->xlilun == 0)
        {
            pTMT->tmr0  = 0;
            pTMT->pdt    = pData[0] & 0x1F;
            pTMT->icimtPtr->tmdir[pTMT->lid] = pTMT;

            /* get vendor ID, product ID, Version, SN from response */
            strncpy(pTMT->venid, (char *)&pData[8], 8);
            strncpy(pTMT->proid, (char *)&pData[16], 16);
            strncpy(pTMT->version, (char *)&pData[32], 4);
            strncpy(pTMT->sn, (char *)&pData[36], 12);
            if ((pData[36] | pData[37] | pData[38] | pData[39] | pData[40] | pData[41] | pData[42] | pData[43] | pData[44] | pData[45] | pData[46] | pData[47]) == 0) {
              *(UINT64*)pTMT->sn = bswap_64(pTMT->P_name);
            }
        }

        /* If Device present, Direct Access, send ReadCapacity cmd */
        if(pData[0] == 0x0)
        {
            snd_rc(pILT, pXLI->xlilun);
            TaskSetMyState(PCB_NOT_READY);
        }
        else
        {
            /* If default LUN (0), send ReportLUNs cmd */
            if(pXLI->xlilun == 0)
            {
                snd_rluns(pILT, pXLI->xlilun);
                TaskSetMyState(PCB_NOT_READY);
            }
            else
            {
                /* Send the TUR for the next available lun. If no additional
                 * LUNs are present, change the scan state to DONE */
                pILT->ilt_scan.scan_state = SCAN_DONE;
                for(i = pXLI->xlilun + 1; i < MAX_LUNS; i++)
                {
                    if(pTMT->tlmtDir[i] != NULL)
                    {
                        if (pTMT->tlmtDir[i]->state == tlmstate_discovery)
                        {
                            snd_tur(pILT, i);
                            TaskSetMyState(PCB_NOT_READY);
                            break;
                        }
                        else
                        {
                            pTMT->tlmtDir[i]->state = tlmstate_delete;
                        }
                    }
                }
            }
        }
    }
    else if ((pILT->ilt_scan.scan_retry > 1)
            && ((pILT->ilt_scan.scan_status == SCS_ECHK)
            || (pILT->ilt_scan.scan_status == EC_CHECK)
            || (pILT->ilt_scan.scan_status == SCS_BUSY)
            || (pILT->ilt_scan.scan_status == EC_BUSY)))
    {
        /* If check condition, just reissue the cmd. We may have to validate
         * the check condition here! */
        ISP_GetPortDB(pTMT->chipID, pTMT->lid, 0);
        pILT->ilt_scan.scan_retry -= 1;
        snd_inq(pILT, pXLI->xlilun);
        TaskSetMyState(PCB_NOT_READY);
    }
    else
    {
        /* Bad error. Restart the login proces. A logout before login (in RESCAN
         * state) helps clear this issue most of the time. */
#ifndef PERF
        fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: tScan ILT(0x%x):%s %d 0x%x\n",
                (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name,
                (UINT32)pILT, stateStr[pILT->ilt_scan.scan_state],
                pXLI->xlilun, pILT->ilt_scan.scan_status);
#endif
        pILT->ilt_scan.scan_status = SCS_NORM;
        pILT->ilt_scan.scan_retry = SCAN_RETRY;
        pILT->ilt_scan.scan_state = SCAN_RESTART;
    }
}

/**
******************************************************************************
**
**  @brief      Processes the Report LUN response
**
**              Create TLMTs if not already existing for all the LUNs reported. Retrieve the
**              first TLMT for lun > 0 and issue TUR cmd.
**              Remove the existing TLMTs for missing LUNs (remove LUNs).
**
**  @param      ILT*    - ILT
**
**  @return     none
**
******************************************************************************
**/
void i_rluns (ILT* pILT)
{
    UINT32 i;
    UINT32 len;
    UINT16 lun;
    TLMT *pTLMT = NULL;
    XLI *pXLI = (XLI *)pILT->ilt_scan.scan_xli;
    TMT *pTMT = (TMT *)pILT->ilt_scan.scan_tmt;
    UINT32 *pData = ((SGL_DESC*)pXLI->xlisglptr)->addr;

    /* If RLUNS success, Process it */
    if ((pILT->ilt_scan.scan_status == SCS_NORM)
            ||  (pILT->ilt_scan.scan_status == EC_UNDERRUN))
    {
        /* Retrieve the lun list lenght and process the LUNS */
        len = bswap_32(*pData) / 8;
        pData += 2;
//        fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]:LUNS (%d) <=>",
//                (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name, len);
        for(i = 0; i < len; i++)
        {
            lun = (UINT16)(*pData >> 8);
//            fprintf(stderr," %04d ", lun);
            pData += 2;
            if((lun != 0) && (lun < MAX_LUNS))
            {
                if((pTLMT = pTMT->tlmtDir[lun]) == NULL)
                {
                    /* Allocate a new TLMT and initialize the fields */
                    pTLMT = get_tlmt();
#ifdef M4_DEBUG_TLMT
fprintf(stderr, "%s%s:%u get_tlmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pTLMT);
#endif /* M4_DEBUG_TLMT */
                    memset((void *)pTLMT, 0, sizeof(TLMT));
                    pTLMT->tmt   = pTMT;
                    pTLMT->icimt = pTMT->icimtPtr;
                    pTLMT->lun   = lun;
                    pTLMT->state = tlmstate_discovery;
                    I_queTLMT(pTMT->icimtPtr, pTLMT);
                    pTMT->tlmtDir[lun] = pTLMT;
                }
                else
                {
                    /* TLMT present - update the state */
                    pTLMT->state = tlmstate_discovery;
                }
            }
        }
//        fprintf(stderr,"\n");

        /* Send the TUR for the next (first non-zero) lun. If no additional LUNs
         * are present, change the scan state to DONE */
        pILT->ilt_scan.scan_state = SCAN_DONE;
        for(i = 1; i < MAX_LUNS; i++)
        {
            if(pTMT->tlmtDir[i] != NULL)
            {
                if (pTMT->tlmtDir[i]->state == tlmstate_discovery)
                {
                    snd_tur(pILT, i);
                    TaskSetMyState(PCB_NOT_READY);
                    break;
                }
                else
                {
                    pTMT->tlmtDir[i]->state = tlmstate_delete;
                }
            }
        }
    }
    else if ((pILT->ilt_scan.scan_retry > 1)
            && ((pILT->ilt_scan.scan_status == SCS_ECHK)
            || (pILT->ilt_scan.scan_status == EC_CHECK)
            || (pILT->ilt_scan.scan_status == SCS_BUSY)
            || (pILT->ilt_scan.scan_status == EC_BUSY)))
    {
        /* If check condition, just reissue the cmd. We may have to validate
         * the check condition here! */
        ISP_GetPortDB(pTMT->chipID, pTMT->lid, 0);
        pILT->ilt_scan.scan_retry -= 1;
        snd_rluns(pILT, pXLI->xlilun);
        TaskSetMyState(PCB_NOT_READY);
    }
    else
    {
        /* Bad error. Restart the login proces. A logout before login (in RESCAN
         * state) helps clear this issue most of the time. */
#ifndef PERF
        fprintf (stderr, "tmt(0x%x)[%d:0x%04x:0x%08x:0x%016llX]: tScan ILT(0x%x):%s %d 0x%x\n",
                (UINT32)pTMT, pTMT->chipID, pTMT->lid, pTMT->alpa, pTMT->P_name,
                (UINT32)pILT, stateStr[pILT->ilt_scan.scan_state],
                pXLI->xlilun, pILT->ilt_scan.scan_status);
#endif
        pILT->ilt_scan.scan_status = SCS_NORM;
        pILT->ilt_scan.scan_retry = SCAN_RETRY;
        pILT->ilt_scan.scan_state = SCAN_RESTART;
    }
}

/**
******************************************************************************
**
**  @brief      Processes the Read Capacity response
**
**              On success, update the block size and block cnt in TLMT and ...
**              ... If LUN = 0, issue Report LUNs cmd - otherwise retrieve the
**              next TLMT from the TMT's tlmtDir (valid TLMT after this LUN in the array) and
**              issue TUR cmd
**
**  @param      ILT*    - ILT
**
**  @return     none
**
******************************************************************************
**/

void i_rc (ILT* pILT)
{
    UINT16 i;
    XLI *pXLI = (XLI *)pILT->ilt_scan.scan_xli;
    TMT *pTMT = (TMT *)pILT->ilt_scan.scan_tmt;
    SGL_DESC *pDESC = (SGL_DESC*)pXLI->xlisglptr;

    /* If RLUNS success, Process it */
    if ((pILT->ilt_scan.scan_status == SCS_NORM)
            || (pILT->ilt_scan.scan_status == SCS_RESC)
            || (pILT->ilt_scan.scan_status == EC_RES_CONFLICT)
            ||  (pILT->ilt_scan.scan_status == EC_UNDERRUN))
    {
        if ((pTMT->tlmtDir[pXLI->xlilun] != NULL)
                && ((pILT->ilt_scan.scan_status == SCS_NORM)
                ||  (pILT->ilt_scan.scan_status == EC_UNDERRUN)))
        {
/* NOTDONEYET blkcnt is 64 bits, this means that we need to implement read capacity 16 SCSI command. */
            pTMT->tlmtDir[pXLI->xlilun]->blkcnt = bswap_32(((UINT32 *)pDESC->addr)[0]);
            pTMT->tlmtDir[pXLI->xlilun]->blksz  = bswap_32(((UINT32 *)pDESC->addr)[1]);
        }
    }

    /* If default LUN (0), send ReportLUNs cmd */
    if(pXLI->xlilun == 0)
    {
        snd_rluns(pILT, pXLI->xlilun);
        TaskSetMyState(PCB_NOT_READY);
    }
    else
    {
        /* Send the TUR for the next available lun. If no additional LUNs are
         * present, change the scan state to DONE */
        pILT->ilt_scan.scan_state = SCAN_DONE;
        for(i = pXLI->xlilun + 1; i < MAX_LUNS; i++)
        {
            if(pTMT->tlmtDir[i] != NULL)
            {
                if (pTMT->tlmtDir[i]->state == tlmstate_discovery)
                {
                    snd_tur(pILT, i);
                    TaskSetMyState(PCB_NOT_READY);
                    break;
                }
                else
                {
                    pTMT->tlmtDir[i]->state = tlmstate_delete;
                }
            }
        }
    }
}


/*******************************************************************************************
***                              Utillity functions                                      ***
*******************************************************************************************/

void snd_tur(ILT *pILT, UINT16 lun)
{
    TMT *pTMT = (TMT *)pILT->ilt_scan.scan_tmt;
    XLI *pXLI = (XLI *)pILT->ilt_scan.scan_xli;

    /* Send the TEST UNIT READY cmd */
    memset(pXLI->xlicdb, 0, 16);
    memset((void *)(((SGL_DESC*)pXLI->xlisglptr)->addr), 0, DISC_BUFSIZE);
    CDB_SET_CODE(pXLI->xlicdb, SCC_TESTUNR);
    pXLI->xlidatadir = XLI_DATA_NONE;
    pXLI->xlilun     = lun;
    pXLI->xlisglnum  = 0;
    pXLI->xlitarget  = pTMT->lid;
    (pILT + 1)->misc = (UINT32)pXLI;
    pILT->ilt_scan.scan_state = SCAN_TUR;
    pILT->ilt_scan.scan_status = SCS_NORM;
    ISP_initiate_io(pILT+1);
}

void snd_ssu(ILT *pILT, UINT16 lun)
{
    TMT *pTMT = (TMT *)pILT->ilt_scan.scan_tmt;
    XLI *pXLI = (XLI *)pILT->ilt_scan.scan_xli;

    /* Send the START/STOP UINT cmd */
    memset(pXLI->xlicdb, 0, 16);
    memset((void *)(((SGL_DESC*)pXLI->xlisglptr)->addr), 0, DISC_BUFSIZE);
    pXLI->xlicdb[0] = SCC_SSU;
    pXLI->xlicdb[4] = 0x01;
    pXLI->xlidatadir = XLI_DATA_NONE;
    pXLI->xlilun     = lun;
    pXLI->xlisglnum  = 0;
    pXLI->xlitarget  = pTMT->lid;
    (pILT + 1)->misc = (UINT32)pXLI;
    pILT->ilt_scan.scan_status = SCS_NORM;
    pILT->ilt_scan.scan_state = SCAN_SSU;
    ISP_initiate_io(pILT+1);
}

void snd_inq(ILT *pILT, UINT16 lun)
{
    TMT *pTMT = (TMT *)pILT->ilt_scan.scan_tmt;
    XLI *pXLI = (XLI *)pILT->ilt_scan.scan_xli;

    /* Send the INQ cmd */
    memset(pXLI->xlicdb, 0, 16);
    memset((void *)(((SGL_DESC*)pXLI->xlisglptr)->addr), 0, DISC_BUFSIZE);
    CDB_SET_CODE(pXLI->xlicdb, SCC_INQUIRY);
    pXLI->xlicdb[3]  = 0; /* For SPC-2 compatibility */
    pXLI->xlicdb[4]  = ((DISC_BUFSIZE - 1) & 0x00ff);
    pXLI->xlidatadir = XLI_DATA_READ;
    pXLI->xlisglnum  = pXLI->xlisglnum_sav;
    pXLI->xlilun     = lun;
    pXLI->xlitarget  = pTMT->lid;
    (pILT + 1)->misc = (UINT32)pXLI;
    pILT->ilt_scan.scan_status = SCS_NORM;
    pILT->ilt_scan.scan_state = SCAN_INQ;
    ISP_initiate_io(pILT+1);
}

void snd_rluns(ILT *pILT, UINT16 lun)
{
    TMT *pTMT = (TMT *)pILT->ilt_scan.scan_tmt;
    XLI *pXLI = (XLI *)pILT->ilt_scan.scan_xli;

    /* Send the REPORT LUNs cmd */
    memset(pXLI->xlicdb, 0, 16);
    memset((void *)(((SGL_DESC*)pXLI->xlisglptr)->addr), 0, DISC_BUFSIZE);
    CDB_SET_CODE(pXLI->xlicdb, SCC_REPLUNS);
    pXLI->xlicdb[6]  = ((DISC_BUFSIZE & 0xff000000) >> 24);
    pXLI->xlicdb[7]  = ((DISC_BUFSIZE & 0x00ff0000) >> 16);
    pXLI->xlicdb[8]  = ((DISC_BUFSIZE & 0x0000ff00) >> 8);
    pXLI->xlicdb[9]  = (DISC_BUFSIZE & 0x000000ff);
    pXLI->xlidatadir = XLI_DATA_READ;
    pXLI->xlisglnum  = pXLI->xlisglnum_sav;
    pXLI->xlilun     = lun;
    pXLI->xlitarget  = pTMT->lid;
    (pILT + 1)->misc = (UINT32)pXLI;
    pILT->ilt_scan.scan_status = SCS_NORM;
    pILT->ilt_scan.scan_state = SCAN_RLUN;
    ISP_initiate_io(pILT+1);
}

void snd_rc(ILT *pILT, UINT16 lun)
{
    TMT *pTMT = (TMT *)pILT->ilt_scan.scan_tmt;
    XLI *pXLI = (XLI *)pILT->ilt_scan.scan_xli;

    /* Send the READCAPACITY cmd */
    memset(pXLI->xlicdb, 0, 16);
    memset((void *)(((SGL_DESC*)pXLI->xlisglptr)->addr), 0, DISC_BUFSIZE);
    pXLI->xlicdb[0] = SCC_READCAP;
    pXLI->xlidatadir = XLI_DATA_READ;
    pXLI->xlilun     = lun;
    pXLI->xlitarget  = pTMT->lid;
    pXLI->xlisglnum  = pXLI->xlisglnum_sav;
    (pILT + 1)->misc = (UINT32)pXLI;
    pILT->ilt_scan.scan_status = SCS_NORM;
    pILT->ilt_scan.scan_state = SCAN_RC;
    ISP_initiate_io(pILT+1);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
