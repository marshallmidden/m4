/* $Id: defrag.c 161041 2013-05-08 15:16:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       defrag.c
**
**  @brief      Disk defragment support
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "defrag.h"

#include "daml.h"
#include "def.h"
#include "def_con.h"
#include "defbe.h"
#include "ecodes.h"
#include "error.h"
#include "ficb.h"
#include "fr.h"
#include "ilt.h"
#include "kernel.h"
#include "LL_LinuxLinkLayer.h"
#include "lvm.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "miscbe.h"
#include "nvr.h"
#include "nvram.h"
#include "online.h"
#include "pcb.h"
#include "pmbe.h"
#include "prp.h"
#include "rebuild.h"
#include "RL_PSD.h"
#include "RL_RDD.h"
#include "rrp.h"
#include "sdd.h"
#include "scsi.h"
#include "system.h"
#include "target.h"
#include "pdd.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "CT_defines.h"
#include "mem_pool.h"
#include "ddr.h"
#include "misc.h"

#include <string.h>
#include <byteswap.h>

/* The following are for created tasks. */
extern void CT_LC_DF_VerifyLastRspTask(int);
extern void CT_LC_DF_VerifyLastRspCompleter(int);

/*
******************************************************************************
** Private defines
******************************************************************************
*/
#define DDR_FROM_CCB     0x0000
#define DDR_TO_CCB       0x0001
#define DDR_ASYNC        0x0002
#define DDR_INTERNAL     0x0004
#define DDR_VERIFY       0x0008

#define BIAS_500_AU     1000000
#define BIAS_1000_AU     500000
#define BIAS_UNLIMITED   100000

#define AU_100           (100 * DISK_SECTOR_ALLOC)
#define AU_500           (500 * DISK_SECTOR_ALLOC)
#define AU_1000         (1000 * DISK_SECTOR_ALLOC)

/*
******************************************************************************
** Private variables
******************************************************************************
*/
UINT32              gDFDebugIndx = 0;
UINT32              gDFVerTaskCnt = 0;
UINT32              gDFVerTaskErrCnt = 0;
UINT32              gDFVerTaskGoodCnt = 0;
UINT16              gDFVerTaskFailedPID = 0xFFFF;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
MRDEFRAGMENT_RSP    gDFLastRsp;
UINT8               gDFCancel = FALSE;
PCB*                gDFVerifyTask = NULL;
DDR                 gDFDebug[DDR_MAX] LOCATE_IN_SHMEM;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
void   DF_TraceDefrag(UINT16 direction,
                      MRDEFRAGMENT_REQ* preq,
                      MRDEFRAGMENT_RSP* prsp,
                      UINT16 action);

void   DF_DefragAll(void);

UINT16 DF_PreparePID(UINT16 PID, UINT8 checkExactsOnly);
UINT16 DF_PrepareAll(void);

UINT16 DF_VerifyLastRsp(void);
void   DF_VerifyLastRspCompleter(UINT32 pPCB, UINT32 pri);
void   DF_VerifyLastRspTask(UINT32 pPCB, UINT32 pri, PSD* pPSD);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Process an defrag request MRP.
**
**              The packet size, channel and ID are validated.  If OK, the
**              specified device is checked for redundancy.  If so, the PSD
**              segments are compressed and the device is scheduled for a
**              rebuild operation assuming compression is possible.
**
**  @param      MRP     - Pointer to a the request.
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DF_Defragment(MR_PKT *pMRP)
{
    UINT8               retCode = DEOK;
    UINT16              action  = 0;
    MRDEFRAGMENT_REQ   *pReq    = (MRDEFRAGMENT_REQ*)pMRP->pReq;
    MRDEFRAGMENT_RSP   *pRsp    = (MRDEFRAGMENT_RSP*)pMRP->pRsp;
    PSD                *pPSD;
    UINT16              raidID;
    UINT8               orphanDeleted = FALSE;

    /*
    ** Set the return data size.  We can set it to what was passed in since
    ** the return parm length was validated in the validation of this command
    ** and must be exactly the size needed.
    */
    pRsp->header.len = pMRP->rspLen;

    /*
    ** Debug log the input.
    */
    DF_TraceDefrag(DDR_FROM_CCB, pReq, pRsp, action);

    /*
    ** If the PID is not in the proper range or it is not a defrag all,
    ** then report invalid PID.  If the device does not exist or is
    ** inoperable, report that too.
    */
    if(pReq->pdiskID >= MAX_PHYSICAL_DISKS)
    {
        if(pReq->pdiskID == MRDEFRAGMENT_ORPHANS)
        {
            /*
            ** Search for orphans and if they are found, terminate them.
            */
            for(raidID = 0; raidID < MAX_RAIDS; ++raidID)
            {
                if(DEF_IsOrphanRAID(raidID))
                {
                    DC_DeleteRAID(gRDX.rdd[raidID]);
                    gOrphanLogged[raidID] = FALSE;
                    orphanDeleted = TRUE;
                }
            }

            if(orphanDeleted)
            {
                /*
                ** Save it to NVRAM.
                */
                NV_P2UpdateConfig();
            }

            /*
            ** Shortcut the exit.
            */
            return(retCode);
        }
        else if(pReq->pdiskID != MRDEFRAGMENT_ALL_PID)
        {
            retCode = DEINVPID;
        }
    }
    else if(gPDX.pdd[pReq->pdiskID] == NULL)
    {
        retCode = DENONXDEV;
    }
    else if(gPDX.pdd[pReq->pdiskID]->devStat != PD_OP)
    {
        retCode = DEINOPDEV;
    }
    else if(gPDX.pdd[pReq->pdiskID]->devClass != PD_DATALAB)
    {
        retCode = DENOTDATALAB;
    }


    /*
    ** May want to add a check for defragging if we can determine
    ** a disk is defragging easily.  I don't really want to parse
    ** through all of the RAIDs to find this PID or any other in
    ** the case of a defrag all.  There is a bit available in the
    ** PDD misc status that could be used for defrag determination.
    */


    /*
    ** Not that the validation of a single PID or a defrag all is
    ** done, check the operation being requested and act accordingly.
    */
    if(retCode == DEOK)
    {
        if(pReq->control == MRDEFRAGMENT_PREPARE)
        {
            /*
            ** Find a PID / RID combination to move.  Also
            ** make sure that a rebuild or defrag in process
            ** is checked since we do not want to start a
            ** move if one is running.  This should be checked
            ** by the CCB also, but add a check for safety sake.
            */
            action = 0x1000;

            /*
            ** Prep the status to no move possible.
            */
            gDFLastRsp.status = MRDEFRAGMENT_NOMOVE;
            gDFCancel = FALSE;

            if(gDFVerTaskCnt != 0)
            {
                action += 0x0100;
                retCode = DEACTIVEDEF;
            }
            else
            {
                if(pReq->pdiskID != MRDEFRAGMENT_ALL_PID)
                {
                    action += 0x0200;

                    if((gPDX.pdd[pReq->pdiskID] != NULL) &&
                       (gPDX.pdd[pReq->pdiskID]->devStat == PD_OP))
                    {
                        retCode = DF_PreparePID(pReq->pdiskID, FALSE);

                        /*
                        ** Debug log the completion.
                        */
                        DF_TraceDefrag(DDR_INTERNAL, NULL, NULL, 0x5000 + retCode);
                    }
                    else
                    {
                        retCode = DEINOPDEV;
                    }
                }
                else
                {
                    action += 0x0300;
                    retCode = DF_PrepareAll();

                    /*
                    ** Debug log the completion.
                    */
                    DF_TraceDefrag(DDR_INTERNAL, NULL, NULL, 0x6000 + retCode);
                }
            }
        }
        else if(pReq->control == MRDEFRAGMENT_VERIFY)
        {
            /*
            ** Check the PID / RID combination against the last
            ** prepared value and then start the verification
            ** process if the check passes.
            */
            action = 0x2000;

            if(gDFVerTaskCnt != 0)
            {
                action += 0x0100;
                retCode = DEACTIVEDEF;
            }
            else
            {
                action += 0x0200;
                retCode = DF_VerifyLastRsp();
            }
        }
        else if(pReq->control == MRDEFRAGMENT_MOVE)
        {
            /*
            ** Check the PID / RID combination against the last
            ** prepared value and then start the move if the
            ** verification passed.
            */
            action = 0x3000;

            if((gDFLastRsp.status  != MRDEFRAGMENT_OK) ||
               (gDFLastRsp.pdiskID != pReq->pdiskID) ||
               (gDFLastRsp.raidID  != pReq->raidID) ||
               (gDFLastRsp.sda     != pReq->sda))
            {
                action += 0x0100;
                gDFLastRsp.status = MRDEFRAGMENT_MISMATCH;
            }
            else if(gDFVerTaskCnt != 0)
            {
                action += 0x0200;
                retCode = DEACTIVEDEF;
            }
            else
            {
                /*
                ** Get the PSD to be moved.  Move it by deallocating
                ** the space in the DAML, then assigning the new space.
                ** Also move the SDA in the PSD and put it into a
                ** rebuild state.  Save it to NVRAM so that it can be
                ** propagated to the other controllers.
                */
                action += 0x0300;
                pPSD = gRDX.rdd[gDFLastRsp.raidID]->extension.pPSD[0];

                while(pPSD->pid != gDFLastRsp.pdiskID)
                {
                    pPSD = pPSD->npsd;
                }

                DA_Release(pPSD);

                /*
                ** Now assign it in the new spot.
                */
                DA_DAMAsg(gPDX.pdd[gDFLastRsp.pdiskID]->pDAML,
                          gDFLastRsp.sda / DISK_SECTOR_ALLOC,
                          pPSD->sLen / DISK_SECTOR_ALLOC,
                          pPSD);

                DA_DAMSum(gPDX.pdd[gDFLastRsp.pdiskID]->pDAML);

                /*
                ** Set to rebuild.  Set the PSD alternate status to show a
                ** defrag.  We can't set the status to defrag since it will
                ** be in a rebuild state once in the PSD rebuilder function.
                */
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                BIT_SET(pPSD->aStatus, PSDA_REBUILD);
                BIT_SET(pPSD->aStatus, PSDA_DEFRAG);

                /* Change the SDA in the PSD to complete the move. */
                pPSD->sda = gDFLastRsp.sda;
                RB_SearchForFailedPSDs();

                /*
                ** Save it to NVRAM.
                */
                NV_P2UpdateConfig();
            }
        }
        else if(pReq->control == MRDEFRAGMENT_STOP)
        {
            /*
            ** Stop any defrags running.  It is not an error to
            ** get this and not have a defrag running.  The main
            ** effect is to stop a verification pass if it is
            ** running.  The move cannot be stopped since it is
            ** actually a rebuild that has already been started.
            ** Make sure the last respsonse is cleared so that
            ** any next step will fail.
            */
            action = 0x3000;
            DF_CancelDefrag();
            DF_ClearLastResp();
        }
        else
        {
            action = 0x4000;
            retCode = DEINVOPT;
        }
    }

    /*
    ** Load up the response.  If the return code is OK, then the response
    ** global contains the return data.
    */
    if(retCode == DEOK)
    {
        pRsp->pdiskID = gDFLastRsp.pdiskID;
        pRsp->raidID  = gDFLastRsp.raidID;
        pRsp->sda     = gDFLastRsp.sda;
        pRsp->status  = gDFLastRsp.status;
    }

    /*
    ** Debug log the completion.
    */
    DF_TraceDefrag(DDR_TO_CCB, pReq, pRsp, action);

    return(retCode);
}


/**
******************************************************************************
**
**  @brief      Determine the RID to be moved, if any, for a disk.
**
**              Go through the DAML for this disk and find any gaps.  If
**              no gaps exist, return no defragmentation.  If a gap exists,
**              attempt to find the best segment to fit into it.  This is
**              done via a single pass looking for one that fits or a
**              second pass looking for any two segments that fit.  If
**              neither of these two is the case, then just slip the one
**              next to the gap up.
**
**  @param      pid - Physical disk ID of disk being checked
**  @param      checkExactsOnly - Boolean - T check for exact hole filling, F - any
**
**  @return     DEOK if no errors, else appropriate error
**
**  @attention  modifies last repsonse
**
******************************************************************************
**/
UINT16 DF_PreparePID(UINT16 pid, UINT8 checkExactsOnly)
{
    UINT16      retCode = DEOK;
    UINT16      idx0, idx1;
    UINT32      auGapSize;
    DAML       *pDAML;
    PSD        *pTempPSD;

    /* First make sure the DAML is up to date. */
    DA_DAMBuild(pid);

    pTempPSD = DC_ConvPDD2PSD(gPDX.pdd[pid]);

    if(pTempPSD == NULL)
    {
        retCode = DEPIDNOTUSED;
    }
    else if(BIT_TEST(gPDX.pdd[pid]->miscStat, PD_MB_SCHEDREBUILD) ||
            BIT_TEST(gPDX.pdd[pid]->miscStat, PD_MB_REBUILDING))
    {
        retCode = DEACTREBUILD;
    }
    else if(RB_CanSpare(pTempPSD) != DEOK)
    {
        retCode = DEINSUFFREDUND;
    }
    else if(gPDX.pdd[pid]->tas != gPDX.pdd[pid]->las)
    {
        pDAML = gPDX.pdd[pid]->pDAML;
        auGapSize = pDAML->damlx[pDAML->firstGap].auGap;

        /*
        ** Now search for an exact match from the last descriptor to the
        ** gap location.
        */
        for(idx0 = pDAML->count - 1;
            (idx0 != pDAML->firstGap) && (gDFLastRsp.status != MRDEFRAGMENT_OK);
            --idx0)
        {
            if(pDAML->damlx[idx0].auSLen == auGapSize)
            {
                gDFLastRsp.status  = MRDEFRAGMENT_OK;
                gDFLastRsp.pdiskID = pid;
                gDFLastRsp.raidID  = pDAML->damlx[idx0].auRID;
                gDFLastRsp.sda     = pDAML->damlx[pDAML->firstGap].auSda +
                                     pDAML->damlx[pDAML->firstGap].auSLen;
            }
        }

        /*
        ** Now, if necessary perform a secondary search for a case
        ** where two RAIDs may fit.  This does not have to track
        ** both since one will be put into the gap and then the next
        ** pass will pick up the second one.
        */
        for(idx0 = pDAML->count - 1;
            (idx0 != pDAML->firstGap + 1) && ((gDFLastRsp.status != MRDEFRAGMENT_OK));
            --idx0)
        {
            for(idx1 = idx0 - 1;
                (idx1 != pDAML->firstGap) && ((gDFLastRsp.status != MRDEFRAGMENT_OK));
                --idx1)
            {
                if((pDAML->damlx[idx0].auSLen + pDAML->damlx[idx1].auSLen) == auGapSize)
                {
                    gDFLastRsp.status  = MRDEFRAGMENT_OK;
                    gDFLastRsp.pdiskID = pid;
                    gDFLastRsp.raidID  = pDAML->damlx[idx0].auRID;
                    gDFLastRsp.sda     = pDAML->damlx[pDAML->firstGap].auSda +
                                         pDAML->damlx[pDAML->firstGap].auSLen;
                }
            }
        }

        /*
        ** Now we will either have a single or double hole fill or we will not.
        ** If we don't, then grab the segment right after the first gap and
        ** set it up.  If the exact match parm is True, then skip this and
        ** return no move possible.
        */
        if((gDFLastRsp.status != MRDEFRAGMENT_OK) && !checkExactsOnly)
        {
            gDFLastRsp.status  = MRDEFRAGMENT_OK;
            gDFLastRsp.pdiskID = pid;
            gDFLastRsp.raidID  = pDAML->damlx[pDAML->firstGap+1].auRID;
            gDFLastRsp.sda     = pDAML->damlx[pDAML->firstGap].auSda +
                                 pDAML->damlx[pDAML->firstGap].auSLen;
        }
    }

    /*
    ** If we found a move, then we have to multiply the starting address
    ** by the AU size since we grabbed the sda in AUs from the DAML.
    */
    if(gDFLastRsp.status == MRDEFRAGMENT_OK)
    {
        gDFLastRsp.sda *= (UINT64)DISK_SECTOR_ALLOC;
    }

    return(retCode);
}


/**
******************************************************************************
**
**  @brief      Prepare a PID/RID combo based upon all drives.
**
**  @param
**
**  @return     DEOK if no errors, else appropriate error
**
**  @attention  modifies last repsonse
**
******************************************************************************
**/
UINT16 DF_PrepareAll(void)
{
    UINT16      retCode = DEOK;
    UINT16      pid;
    PDD        *pPDD = NULL;
    PDD        *pCandidate;
    UINT32      score = 0;
    UINT32      candidateScore = 0;
    PSD        *pTempPSD = NULL;

    /*
    **
    **
    ** JLW - Look at putting this into the scoring loop.  Perhaps bump the
    **       score of any PDD that can have a gap filled over ones that cannot.
    **
    **
    ** Loop through all PDDs and find the one to defrag next.
    **
    ** The following criteria is used:
    **
    **  1)  The drive must be "defragable".  That is, operational, labelled,
    **      not in a rebuild state, etc.
    **  2)  Check each drive for an exact match in the gap.  If there is a
    **      drive with an exact match, then return it.
    **  3)  Else, chose the drive with the highest score based upon the
    **      TAS and LAS difference.
    */
    for(pid = 0;
        (pid < MAX_PHYSICAL_DISKS) && (gDFLastRsp.status != MRDEFRAGMENT_OK);
        ++pid)
    {
        pPDD = gPDX.pdd[pid];

        if((pPDD != NULL) &&
           (pPDD->devStat == PD_OP) &&
           (pPDD->devClass == PD_DATALAB))
        {
            DF_PreparePID(pid, TRUE);
        }
    }

    /*
    ** If we did not find an exact match, then the LastRsp will indicate this.
    ** Do a candidate scoring.
    */
    if(gDFLastRsp.status != MRDEFRAGMENT_OK)
    {
        pCandidate = NULL;
        candidateScore = score = 0;

        for(pid = 0; pid < MAX_PHYSICAL_DISKS; ++pid)
        {
            pPDD = gPDX.pdd[pid];

            if((pPDD != NULL) &&
               (pPDD->devStat == PD_OP) &&
               (pPDD->devClass == PD_DATALAB) &&
               !BIT_TEST(pPDD->miscStat, PD_MB_SCHEDREBUILD) &&
               !BIT_TEST(pPDD->miscStat, PD_MB_REBUILDING))
            {
                pTempPSD = DC_ConvPDD2PSD(pPDD);

                if(RB_ActDefCheck(pPDD) &&
                   (pPDD->tas != pPDD->las) &&
                   (pTempPSD != NULL) &&
                   (RB_CanSpare(pTempPSD) == DEOK))
                {
                    /*
                    ** At this point the current drive is a candidate.
                    ** Each candidate gets a rating based on the total
                    ** regained space and size of the current largest
                    ** available space.
                    */
                    score = pPDD->tas - pPDD->las;

                    if((score < AU_100) || (pPDD->las < AU_100))
                    {
                        score = 1;
                    }
                    else if(score < AU_500)
                    {
                        score += BIAS_500_AU;
                    }
                    else if(score < AU_1000)
                    {
                        score += BIAS_1000_AU;
                    }
                    else
                    {
                        score += BIAS_UNLIMITED;
                    }

                    if(score > candidateScore)
                    {
                        candidateScore = score;
                        pCandidate = pPDD;
                    }
                }
            }
        }

        /*
        ** If there is a candidate, then get its movement to do and
        ** return it.
        */
        if(pCandidate != NULL)
        {
            DF_PreparePID(pCandidate->pid, FALSE);
        }
    }

    return(retCode);
}


/**
******************************************************************************
**
**  @brief      Verify segments for a movement.
**
**              The RAID segment to be moved is examined and each of
**              the disks that would be involved in the rebuild are
**              verified using the SCSI verify command.
**
**  @param      None - gDFLastRsp is used for the starting points.
**
**  @return     None
**
**  @attention  Uses global variables
**
**                  gDFVerifyTask
**                  gDFLastRsp
**                  gDFVerTaskCnt
**                  gDFVerTaskErrCnt
**                  gDFVerTaskGoodCnt
**
******************************************************************************
**/
UINT16 DF_VerifyLastRsp(void)
{
    RDD    *pRDD;
    UINT16  location;
    UINT16  startIndex;
    UINT16  index_D;
    UINT8   numToDo = 0;

    /*
     * Determine which drives need to have a verification pass.  We will
     * "over-verify" on RAIDs that have a non-integer number of stripes
     * in the PSD count since the complexity of determining exactly where
     * the data lies is not worth the effort.
     *
     * If there is an integer number of stripes in one pass through the
     * RAID (psd count is evenly divisible by depth), then just verify
     * the PSDs in the stripe.  If we are not evenly divisible, then we
     * have to verify the drives before and after the drive being moved.
     */
    location = 0;
    gDFVerTaskCnt = 0;
    gDFVerTaskErrCnt = 0;
    gDFVerTaskGoodCnt = 0;
    gDFVerTaskFailedPID = 0xFFFF;

    if ((gDFLastRsp.status != MRDEFRAGMENT_CLEAR) &&
        (NULL != (pRDD = gRDX.rdd[gDFLastRsp.raidID])))
    {
        while (pRDD->extension.pPSD[location]->pid != gDFLastRsp.pdiskID)
        {
            ++location;
        }

        /*
         * Now start up a task for each PSD that we wish to
         * verify.  For a RAID with the psdcnt divisible by
         * the depth, we only have to do the stripe the PSD
         * at the location is in.  Otherwise, we have to do
         * the location minus depth to location plus depth.
         */
        if ((pRDD->psdCnt % pRDD->depth) == 0)
        {
            numToDo = pRDD->depth - 1;
            startIndex = (location / pRDD->depth) * pRDD->depth;
        }
        else if (pRDD->psdCnt <= (((pRDD->depth - 1) * 2) + 1))
        {
            numToDo = pRDD->psdCnt - 1;
            startIndex = 0;
        }
        else
        {
            numToDo = (pRDD->depth - 1) * 2;

            if (location >= (pRDD->depth - 1))
            {
                startIndex = location - pRDD->depth + 1;
            }
            else
            {
                startIndex = location + pRDD->psdCnt - (pRDD->depth - 1);
            }
        }

        for (index_D = startIndex; numToDo > 0; )
        {
            if (index_D != location)
            {
                --numToDo;
                ++gDFVerTaskCnt;
                CT_fork_tmp = (unsigned long)"DF_VerifyLastRspTask";
                TaskCreate3(C_label_referenced_in_i960asm(DF_VerifyLastRspTask),
                           REBUILD_PRIORITY,
                           (UINT32)pRDD->extension.pPSD[index_D]);
            }

            if (++index_D == pRDD->psdCnt)
            {
                index_D = 0;
            }
        }

        /*
         * Start a completer task to monitor the PSD verifications and
         * report back a log message when we are all done.
         */
        ++gDFVerTaskCnt;
        CT_fork_tmp = (unsigned long)"DF_VerifyLastRspCompleter";
        gDFVerifyTask = TaskCreate2(C_label_referenced_in_i960asm(DF_VerifyLastRspCompleter), REBUILD_PRIORITY);
        return DEOK;
    }
    return DEINVRID;
}


/**
******************************************************************************
**
**  @brief      Verify completer function.
**
**              This function will wait for the separate verify tasks
**              to complete and then log the final status for the
**              verification pass.
**
**  @param      None
**
**  @return     None
**
**  @attention  Uses global variables
**
**                  gDFLastRsp
**                  gDFVerTaskCnt
**                  gDFVerTaskErrCnt
**                  gDFVerTaskGoodCnt
**
******************************************************************************
**/
void DF_VerifyLastRspCompleter(UINT32 pPCB UNUSED, UINT32 pri UNUSED)
{
    LOG_DEFRAG_VER_DONE_PKT defragVerDoneEvent;

    while(gDFVerTaskCnt != 1)
    {
        /*
         * Sleep and wait for the verification tasks to wake up this task.
         * We sleep on a timed wait even though the individual tasks are
         * designed to wake us up.  This is a watch-dog type of sleep in
         * case there is some race condition.
         */
        TaskSleepMS (60 * 1000);
    }

    defragVerDoneEvent.header.event   = LOG_DEFRAG_VER_DONE;

    defragVerDoneEvent.data.failedPID = gDFVerTaskFailedPID;
    defragVerDoneEvent.data.pdiskID   = gDFLastRsp.pdiskID;
    defragVerDoneEvent.data.raidID    = gDFLastRsp.raidID;
    defragVerDoneEvent.data.sda       = gDFLastRsp.sda;

    if((gDFVerTaskErrCnt != 0) || (gDFCancel))
    {
        defragVerDoneEvent.data.success = FALSE;
    }
    else
    {
        defragVerDoneEvent.data.success = TRUE;
    }

    /* Send the log message. */
    MSC_LogMessageStack(&defragVerDoneEvent, sizeof(LOG_DEFRAG_VER_DONE_PKT));

    gDFVerTaskCnt = 0;
    gDFCancel = FALSE;
    gDFVerifyTask = NULL;

    /* Debug log the completion. */
    DF_TraceDefrag(DDR_VERIFY, NULL, NULL, 0xBF00 + gDFVerTaskGoodCnt);
}


/**
******************************************************************************
**
**  @brief      Verify a single drive for a movement.
**
**              This task will run a verify pass on a single pdisk.
**
**  @param      None
**
**  @return     None
**
**  @attention  Uses global variables
**
**                  gDFLastRsp
**                  gDFVerTaskCnt
**                  gDFVerTaskErrCnt
**                  gDFVerTaskGoodCnt
**
******************************************************************************
**/
void DF_VerifyLastRspTask(UINT32 pPCB UNUSED, UINT32 pri UNUSED, PSD *pPSD)
{
    UINT64      currSDA;
    UINT64      eda;
    ILT        *pILT;
    PRP        *pPRP;
    UINT32      prStatus = EC_OK;
    RDD        *pRDD;
    RRP        *pRRP;
    void       *pDataBuffer;

    if((gDFLastRsp.status != MRDEFRAGMENT_CLEAR) &&
       (gRDX.rdd[pPSD->rid] != NULL))
    {
        /*
        ** Set up the initial parms.  We want to check the entire PSD from
        ** the SDA within the PSD up to the length indicated in the RDD.
        */
        currSDA = pPSD->sda;
        eda = currSDA + pPSD->sLen;
        pRDD = gRDX.rdd[pPSD->rid];

        while((currSDA < eda) && (prStatus == EC_OK) && !gDFCancel)
        {
            if ((currSDA & ~0xffffffffULL) != 0ULL) /* 16 byte verify command */
            {
                pILT = ON_GenReq(&gTemplateVerify1_16, gPDX.pdd[pPSD->pid], &pDataBuffer, &pPRP);
                *(UINT64 *)(&pPRP->cmd[2]) = bswap_64(currSDA);
            } else {
                pILT = ON_GenReq(&gTemplateVerify1, gPDX.pdd[pPSD->pid], &pDataBuffer, &pPRP);
                *(UINT32 *)(&pPRP->cmd[2]) = bswap_32(currSDA);
            }

            pPRP->strategy = PRP_LOW;
            pILT->ilt_normal.w4 = (UINT32)(UINT32*)pPSD;
            pPRP->sda = currSDA;
            pPRP->eda = currSDA + DISK_SECTOR_ALLOC;

            ON_QueReq(pILT);

            prStatus = MSC_ChkStat(pPRP);

            /*
            ** Error Handler
            ** Flag the PDD as having an error and queue an ILT to the error
            ** handler. The RAID error handler has the logic to determine if
            ** this is a single drive failure that should be rebuilt or if the
            ** entire RAID has become inoperable.
            */
            if(EC_OK != prStatus)
            {
                RB_RAIDError(pPSD, pRDD, pPRP, pILT);

                ON_RelReq(pILT);

                /* Get new ILT/RRP. */
                pILT = get_ilt();           /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)pILT);
#endif  /* M4_DEBUG_ILT */
                pRRP = get_rrp();           /* Allocate an RRP. */
#ifdef M4_DEBUG_RRP
    CT_history_printf("%s%s:%u get_rrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, pRRP);
#endif  /* M4_DEBUG_RRP */
                pILT->ilt_normal.w0 = (UINT32)pRRP; /* Link to RRP in ILT. */

                pILT = pILT + 1;
                pRRP->pSGL = NULL;
                pILT->ilt_normal.w5 = 0;
                pRRP->function = RRP_VERIFY_DATA;
                pILT->ilt_normal.w4 = (UINT32)(UINT32*)pRDD;
                pILT->cr = (void*)&RB_RAIDErrorComp;

                RB$rerror_que(pILT);
            }
            else
            {
                ON_RelReq(pILT);
            }
            currSDA += DISK_SECTOR_ALLOC;
        }

        /*
        ** Chek the status of verification. If the status is fine, continue.
        ** Otherwise, increment the error count and failed PID and exit.
        */
        if(EC_OK == prStatus)
        {
            ++gDFVerTaskGoodCnt;
        }
        else
        {
            ++gDFVerTaskErrCnt;
        }
    }

    --gDFVerTaskCnt;

    /* Wake up the completer task. */
    if (TaskGetState(gDFVerifyTask) == PCB_NOT_READY ||
        TaskGetState(gDFVerifyTask) == PCB_TIMED_WAIT)
    {
#ifdef HISTORY_KEEP
CT_history_pcb("DF_VerifyLastRspTask setting ready pcb", (UINT32)gDFVerifyTask);
#endif
        TaskSetState(gDFVerifyTask, PCB_READY);
    }

    /*
    ** Debug log the completion.
    */
    DF_TraceDefrag(DDR_VERIFY, NULL, NULL, 0xB000 + gDFVerTaskGoodCnt);
}

/**
******************************************************************************
**
**  @brief      Clear the Last Resp structure.
**
**  @param      None
**
**  @return     None
**
******************************************************************************
**/
void DF_ClearLastResp(void)
{

    gDFLastRsp.status = MRDEFRAGMENT_CLEAR;

    DF_TraceDefrag(DDR_ASYNC, NULL, NULL, 0xA000);
}


/**
******************************************************************************
**
**  @brief      Stop defragment from proceeding to a next step.
**
**  @param      None
**
**  @return     None
**
******************************************************************************
**/
void DF_CancelDefrag(void)
{

    gDFCancel = TRUE;

    DF_ClearLastResp();

    DF_TraceDefrag(DDR_ASYNC, NULL, NULL, 0x9000);
}


/**
******************************************************************************
**
**  @brief      Stop defragment if running on this RID.
**
**              If a defrag step is running on this RAID, then stop the
**              step from running any more.  This is used in cases of
**              RAID specific operations such as a RAID init.
**
**  @param      rid - RAID ID of RAID being checked
**
**  @return     None
**
******************************************************************************
**/
void DF_StopDefragRID(UINT16 rid)
{
    /*
    ** Check the RAID.  If it is in the last response, then set up the
    ** cancel in case a verify is running.  If it is running, it will
    ** stop.  If not, there is no harm.  Also set the last response to
    ** be invalid.
    */
    if(gDFLastRsp.raidID == rid)
    {
        DF_CancelDefrag();
        DF_ClearLastResp();

        /*
        ** Debug log the completion.
        */
        DF_TraceDefrag(DDR_ASYNC, NULL, NULL, 0x8000);
    }
}


/**
******************************************************************************
**
**  @brief      Log defragment completion.
**
**              This function will log when each segment of a PDD has been
**              defragmented (moved).  It will also log the RID passed in
**              which will be a valid RID or will be 0xFFFF to indicate that
**              the entire PDD is done.
**
**  @param      pid - PID of the PDD being completed
**              rid - RAID ID of RAID being checked
**
**  @return     None
**
******************************************************************************
**/
void DF_LogDefragDone(UINT16 pid, UINT16 rid, UINT16 errCode)
{
    LOG_DEFRAG_DONE_PKT defragDoneEvent;

    defragDoneEvent.header.event = LOG_Debug(LOG_DEFRAG_DONE);

    if(errCode != RB_ECOK)
    {
        defragDoneEvent.header.event = LOG_Error(defragDoneEvent.header.event);
    }

    defragDoneEvent.data.rid      = rid;
    defragDoneEvent.data.errCode  = errCode;

    if(gPDX.pdd[pid] != NULL)
    {
        defragDoneEvent.data.wwnPDisk = gPDX.pdd[pid]->wwn;
    }
    else
    {
        defragDoneEvent.data.wwnPDisk = 0;
    }

    /* Send the log message. */
    MSC_LogMessageStack(&defragDoneEvent, sizeof(LOG_DEFRAG_DONE_PKT));

    /*
     * Now check if the drive is completely done.  If so, log again with the
     * RID set to 0xFFFF to indicate we are done.
     */
    if((gPDX.pdd[pid] != NULL) && (gPDX.pdd[pid]->las == gPDX.pdd[pid]->tas))
    {
        defragDoneEvent.data.rid = 0xFFFF;
        defragDoneEvent.header.event = LOG_DEFRAG_DONE;

        /* Send the log message. */
        MSC_LogMessageStack(&defragDoneEvent, sizeof(LOG_DEFRAG_DONE_PKT));
    }
}


/**
******************************************************************************
**
**  @brief      Trace the actions on a defrag entry.
**
**  @param      direction - Command to proc (0x0000) or response from (0xFFFF).
**  @param      req       - Pointer to request structure
**  @param      rsp       - Pointer to response structure
**  @param      action    - What action was taken in the processing
**
**  @return     none
**
******************************************************************************
**/
void DF_TraceDefrag(UINT16 direction,
                    MRDEFRAGMENT_REQ* preq,
                    MRDEFRAGMENT_RSP* prsp,
                    UINT16 action)
{

    /*
    ** Bump the index into the next record.  If we rolled, set it back to
    ** the base of the table.
    */
    if(gDFDebugIndx == (DDR_MAX - 1))
    {
        gDFDebugIndx = 0;
    }
    else
    {
        gDFDebugIndx++;
    }

    gDFDebug[gDFDebugIndx].direction = direction;
    gDFDebug[gDFDebugIndx].action = action;

    /*
    ** Log the data from either the input or the output structure based
    ** upon the direction indicator.
    */
    if(direction == DDR_FROM_CCB)
    {
        gDFDebug[gDFDebugIndx].control = preq->control;
        gDFDebug[gDFDebugIndx].status  = 0;
        gDFDebug[gDFDebugIndx].pdiskID = preq->pdiskID;
        gDFDebug[gDFDebugIndx].raidID  = preq->raidID;
        gDFDebug[gDFDebugIndx].sdaHi   = (UINT16)(preq->sda >> 16);
        gDFDebug[gDFDebugIndx].sdaLo   = (UINT16)(preq->sda & 0xFFFF);
    }
    else if(direction == DDR_TO_CCB)
    {
        gDFDebug[gDFDebugIndx].control = 0;
        gDFDebug[gDFDebugIndx].status  = prsp->status;
        gDFDebug[gDFDebugIndx].pdiskID = prsp->pdiskID;
        gDFDebug[gDFDebugIndx].raidID  = prsp->raidID;
        gDFDebug[gDFDebugIndx].sdaHi   = (UINT16)(prsp->sda >> 16);
        gDFDebug[gDFDebugIndx].sdaLo   = (UINT16)(prsp->sda & 0xFFFF);
    }
    else
    {
        gDFDebug[gDFDebugIndx].control = 0;
        gDFDebug[gDFDebugIndx].status  = 0;
        gDFDebug[gDFDebugIndx].pdiskID = gDFLastRsp.pdiskID;
        gDFDebug[gDFDebugIndx].raidID  = gDFLastRsp.raidID;
        gDFDebug[gDFDebugIndx].sdaHi   = (UINT16)(gDFLastRsp.sda >> 16);
        gDFDebug[gDFDebugIndx].sdaLo   = (UINT16)(gDFLastRsp.sda & 0xFFFF);
    }

    /*
    ** Write a stripe of 0xFFFF to show the current location.
    */
    if(gDFDebugIndx == (DDR_MAX - 1))
    {
        memset(&(gDFDebug[0]), 0xFF, sizeof(DDR));
    }
    else
    {
        memset(&(gDFDebug[gDFDebugIndx + 1]), 0xFF, sizeof(DDR));
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
