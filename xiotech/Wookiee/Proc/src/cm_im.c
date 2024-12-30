/* $Id: cm_im.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
*******************************************************************************
**
** @file: cm_im.c
**
** @brief:
**          Copy Manager Instant Mirror functionality.
**
**  Copyright (c) 2008-2010 XIOtech Corporation.  All rights reserved.
**
********************************************************************************
**/
#include "cm.h"
#include "ccsm.h"
#include "cor.h"
#include "CT_defines.h"

#include "defbe.h"
#include "ecodes.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "misc.h"
#include "nvram.h"
#include "pcb.h"
#include "rrp.h"
#include "system.h"

#include "vdd.h"

#include "stdio.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "ddr.h"
#include "misc.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/

/*
** The following Vdisk map contains the VID list for which Instant
** Mirror related configuration change  happened. When a particular
** bit is set, it presents the Instant Mirror configu change happens
** for that vdisk.
*/
UINT8 gImConfigChangeVdMap[(MAX_VIRTUAL_DISKS+7)/8];

/*
** PCB related to Instat Mirror related configuration change saving process
** in NVRAM
*/
PCB*  gImConfigChangeNvSaveTaskPCB=NULL;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
extern void CT_LC_CM_ImConfigChangeNvSaveTask(int);
void        CM_ImConfigChangeNvSaveTask(void);
void        CM_ImConfigChangeNvSave(VDD *pVDD);
void        CM_LogEvent(UINT8 eventType,VDD *pSrcVDD);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*
******************************************************************************
**
**  @brief      This function performs the process Instant Mirror config change
**              updation in both the DCNs.
**
**              This gets  called  when  there is  change in the instant mirror
**              flag  related to  a particular vdisk. This is  currently called
**              in I/O completion path when the first write request is success-
**              fully completed on a particular vdisk.
**
**              This function propagates the changes to the  other DCN and also
**              optionally  initiates the process of  saving the  configuration
**              in NVRAM.
**
**              If the DCN is Master, the configuration is  optionally saved in
**              NVRAM  and a  datagram  containing  the  changes is sent to the
**              slave controller. If the DCN is Slave, the  changes in the con-
**              figuration are simply passed to the Masster DCN.
**
**              For sending the  datagram, ccsmdata gram process is being used
**              The CCB datagram  event type 5 is used for sending the informa-
**              tion related to Instant Mirror config changes.
**
**
**  @param      pVDD          - Pointer to Vdisk.
**              requireNvSave - Flag indicates whether to save in NVRAM  or not
**
**  @return     None
******************************************************************************
*/
void CM_ImConfigChangeUpdate(VDD *pVDD, UINT8 requireNvSave)
{
    CM_CONFUPDT_DATAGRAM configDgm;
    UINT32 save_g0;
    UINT32 save_g1;
    UINT32 save_g2;
    UINT32 save_g3;
    UINT32 save_g4;
    UINT32 broadcastType = DGBTMASTER;

    if (BIT_TEST(K_ii.status, II_MASTER) == TRUE)
    {
        /* Initiate the process of saving in NVRAM, if opted so. */
        if(requireNvSave)
        {
            CM_ImConfigChangeNvSave(pVDD);
        }
        broadcastType = DGBTSLAVE;
    }

    if(BIT_TEST(K_ii.status, II_CCBREQ))
    {
        /*
         * Both DCNs are alive. Send a broadcast message to other DCN, informing to
         * update the configuration of VDDs owned by this controller.
         */
        configDgm.dgmEvent.len     = sizeof(CM_CONFUPDT_DATAGRAM);
        configDgm.dgmEvent.type    = CCSM_ET_CCBG;      /* CCB gram event type=5 */
        configDgm.dgmEvent.fc      = CCSM_GR_IMCUPDT;   /* Event function code   */
        configDgm.dgmEvent.sendsn  = K_ficb->cSerial;

        configDgm.data.instMirror.vid = pVDD->vid;
        configDgm.data.instMirror.imEnableFlag = CM_VdiskInstantMirrorAllowed(pVDD);
        configDgm.data.instMirror.requireNvSave = requireNvSave;

        /*
         * Save the global registers going to be used by report event
         * function
         */
        save_g0 = g0;            /* used for record address */
        save_g1 = g1;            /* used for extension size */
        save_g2 = g2;            /* used to store event type */
        save_g3 = g3;            /* used to store broadcast type*/
        save_g4 = g4;            /* used to store serial number*/

        DEF_ReportEvent(&configDgm, sizeof(CM_CONFUPDT_DATAGRAM), PUTDG, broadcastType, 0);

        /* Restore back the global registers. */
        g0 = save_g0;
        g1 = save_g1;
        g2 = save_g2;
        g3 = save_g3;
        g4 = save_g4;
    }
}

/*
******************************************************************************
**
**  @brief      This function gets called when the CCSM datagram receive
**              process gets the Instant Mirror related changed configuration
**              datagram sent by other DCN.
**
**              It first updates the Instant Mirror configuration in this DCN.
**              And if the DCN is master it inititates the process of saving
**              in NVRAM, if the sender opted so. The Nvram saving process
**              will take saving the config in both the Master and slave.
**
**  @param      pConfigDgm - Pointer to instant mirror configuration datagram.
**
**  @return     None
******************************************************************************
*/
void CM_ImConfigChangeDgmRecv(CM_CONFUPDT_DATAGRAM *pConfigDgm)
{
    VDD *pVDD;

#if CM_IM_DEBUG
    fprintf(stderr,"<CM_IM>%s-Received datagram for vid-%x option=%x\n", __func__,
            (pConfigDgm->data).instMirror.vid,(pConfigDgm->data).instMirror.imEnableFlag);
#endif  /* CM_IM_DEBUG */

    pVDD = gVDX.vdd[(pConfigDgm->data).instMirror.vid];

    if (pVDD != NULL)
    {
        if (pConfigDgm->data.instMirror.imEnableFlag)
        {
            CM_VdiskEnableInstantMirror(pVDD);
        }
        else
        {
            CM_VdiskDisableInstantMirror(pVDD);
        }

        /*
        **  Initiate the process of saving in NVRAM, if the DCN is master and nvsave
        **  require flag is set
        */
        if (BIT_TEST(K_ii.status, II_MASTER) == TRUE && pConfigDgm->data.instMirror.requireNvSave)
        {
            CM_ImConfigChangeNvSave(pVDD);
        }
    }
}

/*
******************************************************************************
**
**  @brief      This function performs the NVRam saving instant mirror config.
**              The NV saving will be done in a separate task, as the instant
**              mirror flags gets changed in the I/O completion path.
**
**              A global  vdisk map that represents the information about the
**              vdisks (for which   instant  mirror  configuration  has  been
**              changed), will  be updated in  this function. This global map
**              is utilized by NVram saving task to decide whether to perform
**              p2update config or not. It  also facilitates  the NVram  save
**              task to optimize number of NV P2  update calls when the inst-
**              mirror configuration is  getting changed for number of vdisks
**              during I/O.
**
**              After updating the global vdisk map, with  the current vdisk,
**              it ready the NVram save task (if it is not running).
**
**              process gets the Instant Mirror related changed configuration
**              datagram sent by other DCN.
**
**              It first updates the Instant Mirror configuration in this DCN.
**              And if the DCN is master it inititates  the process of saving
**              in NVRAM, if the sender opted so.
**
**  @param      pVDD - Pointer to the virtual disk.
**
**  @return     None
**
**  @attention
**              Refer 'CM_ImConfigChangeNvSaveTask()' task.
**              Also refer NVram P2 configuration save function
**              (NV_P2GenerateImage)
******************************************************************************
*/
void CM_ImConfigChangeNvSave(VDD *pVDD)
{
    UINT32 save_g0;
    UINT32 save_g1;

    /* Mark the VDD has Instant mirror config change */
    BIT_SET(gImConfigChangeVdMap[(pVDD->vid)/8],(pVDD->vid)%8);

#if CM_IM_DEBUG
    fprintf(stderr,"<CM_IM>%s-Entered with vid-%x\n", __func__, pVDD->vid);
#endif  /* CM_IM_DEBUG */
    if (gImConfigChangeNvSaveTaskPCB == NULL)
    {
        /*
        ** It is important to preserve g0 and g1 values when making the taskCreate call, as this
        ** is getting called from virtual layer completer routine(in case of Master Controller)
        ** and the virtual layer should have its ILT in g1 when the control goes back to virtual
        ** layer for completing the I/O request.
        */

        save_g0 = g0;
        save_g1 = g1;
#if CM_IM_DEBUG
        fprintf(stderr,"<CM_IM>%s-Forking the nv save task\n",__func__);
#endif  /* CM_IM_DEBUG */
        CT_fork_tmp = (unsigned long)"CM_ImConfigChangeNvSaveTask";
        gImConfigChangeNvSaveTaskPCB = TaskCreate2(C_label_referenced_in_i960asm(CM_ImConfigChangeNvSaveTask),
               160);

        g0 = save_g0;
        g1 = save_g1;
    }

    /*
    ** Ready the task, if not ready.
    */
    if (TaskGetState(gImConfigChangeNvSaveTaskPCB) == PCB_NOT_READY)
    {
#if CM_IM_DEBUG
        fprintf(stderr,"<CM_IM>%s-Readying the task\n",__func__);
#endif  /* CM_IM_DEBUG */
#ifdef HISTORY_KEEP
CT_history_pcb("CM_ImConfigChangeNvSave setting ready pcb", (UINT32)gImConfigChangeNvSaveTaskPCB);
#endif  /* HISTORY_KEEP */
        TaskSetState(gImConfigChangeNvSaveTaskPCB, PCB_READY);
    }
    else
    {
#if CM_IM_DEBUG
        fprintf(stderr,"<CM_IM>%s-Task running-current change also includes\n",
        __func__);
#endif  /* CM_IM_DEBUG */
    }
}

/*
******************************************************************************
**
**  @brief      This is the task entry function for NVram save task.
**
**              It goes through the entire gloabl vdisk map (containing vids
**              for which instant mirror configuration is changed) and check
**              for any VDD change.  If so it performs  NVram P2  update and
**              clears the global vdisk map. SO with a single P2 update call
**              the  configuration  change occured for  number of  vdisks is
**              being saved into NVram.
**
**              Also this  global vdisk  map  gets updated(cleared) in Nvram
**              P2 config saving function for those Virtual disks which  are
**              getting updated with  latest changed  configuration (configu-
**              ration changed after NVram P2 call  is made), so as to avoid
**              redundant  call of  NVram P2 update call. This is  in  place
**              because there is  possible task contex switch after NVram P2
**              upate call(NV_P2UpdateConfig) and before actual NVram update
**              (real updation is done in NV_P2GenerateImage()), the instant
**              mirror  configuration of some more vdisks  might get changed
**              (especially during  high I/O load). Since  these changes are
**              also included in the actual NVram updation, the Nv Save task
**              will  not  redundantly perform P2 Update  call again for the
**              vdisks changed after it made a call to NV_P2UpdateConfig().
**
**              Once P2 update is done, it again goes through the vd map for
**              any new updates,and if there is any, it again performs NVram
**              P2 update.
**
**              Once the update is done for specified set of vdisks,the task
**              will be set into Not Ready  state and keeps  waiting for the
**              signal when the configuration change occurs again.
**
**  @param      None
**
**  @return     None
**
**  @attention
**              Refer 'CM_ImConfigChangeNvSave()'.
**              Also refer NVram P2 configuration save function.
**              (NV_P2GenerateImage)
******************************************************************************
*/
NORETURN
void CM_ImConfigChangeNvSaveTask(void)
{
    VDD    *pVDD;
    UINT32  anyChange;
    UINT16  vindex;

    while (FOREVER)
    {
        anyChange = FALSE;
        for (vindex = 0; vindex < MAX_VIRTUAL_DISKS; vindex++)
        {
            pVDD = gVDX.vdd[vindex];
            if (pVDD != NULL)
            {
                if (BIT_TEST(gImConfigChangeVdMap[(pVDD->vid)/8],(pVDD->vid)%8) == TRUE)
                {
#if CM_IM_DEBUG
                    fprintf(stderr,"<CM_IM>%s- Config change happens for vid=%x\n",__func__,pVDD->vid);
#endif  /* CM_IM_DEBUG */
                    anyChange = TRUE;
                    BIT_CLEAR(gImConfigChangeVdMap[(pVDD->vid)/8],(pVDD->vid)%8);
                }
            }
        }

        if(anyChange)
        {
#if CM_IM_DEBUG
            fprintf(stderr,"<CM_IM>%s- calling P2UpdateConfig\n",__func__);
#endif  /* CM_IM_DEBUG */
            NV_P2UpdateConfig();
        }
        else
        {
           /*
           ** Set process to wait for signal.
           */
#if CM_IM_DEBUG
           fprintf(stderr,"<CM_IM>%s-Task state set to notready..\n",__func__);
#endif  /* CM_IM_DEBUG */
           TaskSetMyState(PCB_NOT_READY);
           TaskSwitch();
        }
    } //End of While
}


/*
******************************************************************************
**
**  @brief      This function resets the instant mirror flags of a specified
**              vdisk.
**
**              This gets called  during I/O  completion  path (from virtual
**              layer)  when  the first  write  happened  on the vdisk. This
**              clears  the  instant mirror flag( if is not already cleared)
**              and thereby  denies the capability of establishing the inst-
**              ant mirror for this vdisk (as source vdisk).
**
**  @param      None
**
**  @return     None
******************************************************************************
*/
void CM_ResetInstantMirrorFlags(VDD *pVDD, RRP *pRRP)
{
    /* Check whether the instant Mirror capability is set for VDD. */
    if (CM_VdiskInstantMirrorAllowed(pVDD))
    {
        if (pRRP->function == RRP_OUTPUT || pRRP->function == RRP_OUTPUT_VERIFY)
        {
            if (pRRP->status == EC_OK)
            {
                 /* Now Disable Instant Mirror capability for VDD. */
#if CM_IM_DEBUG
                 fprintf(stderr,"<CM_IM>%s-vid=%x-Disabling inst mirror capability\n",__func__,pVDD->vid);
#endif  /* CM_IM_DEBUG */
                 CM_VdiskDisableInstantMirror(pVDD);
#if CM_IM_DEBUG
                 fprintf(stderr,"<CM_IM>%s-vid=%x-updating config change\n",__func__,pVDD->vid);
#endif  /* CM_IM_DEBUG */
                 CM_ImConfigChangeUpdate(pVDD ,TRUE);
             }
         }
    }
}

/*
******************************************************************************
**
**  @brief      This function enables the instant mirror capability for a
**              specified vdisk.
**
**              This gets called  during vdisk creation.
**
**  @param      pVDD - Pointer to virtual disk.
**
**  @return     None
**
******************************************************************************
*/
void CM_VdiskEnableInstantMirror(VDD * pVDD)
{
#if CM_IM_DEBUG
    fprintf(stderr,"<CM_IM>%s-Enabling.vid=%x.\n",__func__,pVDD->vid);
#endif  /* CM_IM_DEBUG */
    BIT_SET(pVDD->attr,VD_BINSTMIRROR);
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
}

/*
******************************************************************************
**
**  @brief      This function disables the instant mirror capability for a
**              specified vdisk.
**
**              This gets called  during first successful write on the vdisk.
**
**  @param      pVDD - Pointer to virtual disk.
**
**  @return     None
******************************************************************************
*/
void CM_VdiskDisableInstantMirror(VDD *pVDD)
{
#if CM_IM_DEBUG
    fprintf(stderr,"<CM_IM>%s-Disabling.vid=%x.\n",__func__,pVDD->vid);
#endif  /* CM_IM_DEBUG */
    BIT_CLEAR(pVDD->attr,VD_BINSTMIRROR);
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    CM_LogEvent(CM_INSTANT_MIRROR_DISABLE, pVDD);
}

/*
******************************************************************************
**
**  @brief      This function  checks  whether the specified vdisk has the
**              capablity of allowing Instant Mirror for its copy.
**
**              This gets called  during  copy-mirror creation process.
**
**  @param      pVDD - Pointer to virtual disk.
**
**  @return     None
**
******************************************************************************
*/
UINT32 CM_VdiskInstantMirrorAllowed(VDD *pVDD)
{
    if (pVDD != NULL && BIT_TEST(pVDD->attr,VD_BINSTMIRROR))
    {
        return TRUE;
    }
    return FALSE;
}

/*
******************************************************************************
**
**  @brief      This function performs instant mirror processing. This gets
**              called from CCSM$CCTrans  when the instant mirror event has
**              been sent after ownership is acquired.
**
**  @param      pCM  -- CM  address
**              pCOR -- COR address
**
**  @return     None
******************************************************************************
*/
void CM_InstantMirror (COR* pCOR, CM* pCM)
{
    UINT16 svid = 0xffff;
    UINT16 dvid = 0xffff;

#if CM_IM_DEBUG
    fprintf(stderr,"<CM_IM>%s-Got ownership acquired PCP/resume PCP..special handling..\n",__func__);
#endif  /* CM_IM_DEBUG */

    /* Set the cstate of cm and cor as 'copy active' which is the last state
    ** in the copy-mirror process. In normal procedure this gets done in
    ** SetnTestCpyState function when there is normal copy-mirror process
    ** (not instant mirror). If we don't do it here,the cmstate will be stuck
    ** in No_Resources set initially.
    */

    // Set CM state intitialised to copy active state.
    pCM->copystate = CSTO_COPY;

    // Set COR cstate initialized to copy active state.
    pCOR->copystate = CST_COPY;

    // Fork off task to update FE

    if(pCOR->srcvdd)
    {
        svid= pCOR->srcvdd->vid;
    }
    if(pCOR->destvdd)
    {
        dvid= pCOR->destvdd->vid;
    }


    CT_fork_tmp = (ulong)"V$updFEStatus-from-instMR";
    TaskCreate4(&V_updFEStatus,VUPDFESTATUS_PRIORITY,(UINT32)svid,(UINT32)dvid);
}

void CM_LogEvent(UINT8 eventType, VDD *pSrcVDD)
{
    LOG_CM_EVENT_PKT cmLog;

    cmLog.header.event = LOG_CM_EVENT;
    cmLog.data.eventType = eventType;

    switch(eventType)
    {
        case CM_INSTANT_MIRROR_DISABLE:
             cmLog.data.svid = pSrcVDD->vid;
             cmLog.data.owner = pSrcVDD->owner;
             break;

        default:
             break;
    }

    /* Note: message is short, and L$send_packet copies into the MRP. */
    MSC_LogMessageStack(&cmLog, sizeof(LOG_CM_EVENT_PKT));
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
