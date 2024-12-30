/* $Id: online.c 160881 2013-04-06 03:22:35Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       online.c
**
**  @brief      Support for bringing the system online.
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "online.h"
#include "CT_defines.h"
#include "pdd.h"
#include "defbe.h"
#include "def_con.h"
#include "dev.h"
#include "ecodes.h"
#include "misc.h"
#include "miscbe.h"
#include "sgl.h"
#include "ssms.h"
#include "scd.h"
#include "dcd.h"
#include "cor.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "ddr.h"

#include <byteswap.h>
#include <stdio.h>
#include <time.h>
/*
******************************************************************************
** Private defines
******************************************************************************
*/
#define AssertDebug(x)

/*
******************************************************************************
** Private variables
******************************************************************************
*/

static UINT32 ON_Inquiry_Pending;
static UINT32 ON_DriveInits;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

void StartOninitDrive(PDD * pddlist[], UINT32 pddlistcount, UINT8 TestType);
void ON_InitializeDrive(UINT32 foo UNUSED, UINT32 bar UNUSED,PDD * pddlist[], UINT32 pddlistcount,void *wwn, UINT8 TestType);
extern void CT_LC_ON_InitializeDrive(int);
extern void o$inquire(UINT32,UINT32,UINT32,PDD*);
void ON_Inquire(UINT32 foo UNUSED, UINT32 bar UNUSED,PDD * pddlist[], UINT32 pddlistcount,void *wwn, UINT8 TestType);
void ON_InquireAll(PDD * pddlist[], UINT32 pddlistcount, UINT8 TestType);
extern void CT_LC_ON_Inquire(int);
#if defined(MODEL_7000) || defined(MODEL_4700)
extern void ISE_LogEvent(UINT16 bayID,UINT8 busyFlag);
#endif /* MODEL_7000 || MODEL_4700 */
void ON_UpdateVDisks(void);
void ON_BEBusy(UINT16 pid,UINT32 TimetoFail, UINT8 justthisonepid);
void ON_BEClear(UINT16 pid,UINT8 justthisonepid);
void ON_TURSuccess(UINT16 pid);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      To provide a common means of setting/clearing byte 2 mode sense
**              select pages parameters based upon the changeable parameter
**              mask.
**
**              The requested mode sense page is accessed for to determine
**              changeable parameters. The mode sense page is accessed again
**              for current values. The byte 2 values are altered by the bits
**              to set and the bits to clear parameters as permitted by
**              the changeable parameters mask. A mode select is then performed
**              to establish the new parameters.
**
**              If the page is the verify recovery or the error recovery pages,
**              we will also modify the recovery time limit if it is not the
**              value we want.
**
**
**  @param      PRP_TEMPLATE *pTmpl - template address
**  @param      PDD *pPDD           - PDD
**  @param      UINT32 setBits      - bits to set as an OR mask
**  @param      UINT32 clearBits    - bits to clear as an AND mask
**
**  @return     UINT32 status (EC_OK if success)
**
**  @attention  The original prototype for this function was returning
**              an ILT pointer. But then the same pointer is freed
**              before exiting. So, the calling function ends up getting
**              an invalid ILT pointer returned from this function.
**              Returning the status made more sense to me, so changed
**              the return valuse to be a status.
**              The following is the functional design discription:
** \code
**              {
**                  Get the mode page changeable params
**                  Get the mode page current params
**                  change the params as per the setBits & clearBits
**                  send Mode Select to set the params
**                  cleanup and return the status
**              }
** \endcode
**
******************************************************************************
**/
UINT32 ON_ModeSenseSelect(PRP_TEMPLATE* pTmpl, PDD* pPDD,
                          UINT32 setBits, UINT32 clearBits)
{
    ILT                    *pILT = NULL;
    PRP                    *pPRP = NULL;
    UINT32                  currentParams = 0;
    UINT32                  changeableParams = 0;
    UINT32                  newParams = 0;
    UINT32                  status = EC_OK;
    UINT8                   page;
    MODE_PAGE_HEADER       *pHeader;
    MODE_PAGE              *pPagePtr;
    UINT32                  changeableIntervalTimer = 0;
    UINT32                  currentIntervalTimer = 0;
    UINT8                   setIntervalTimer = FALSE;

    if ((pTmpl == NULL) || (pPDD == NULL))
    {
        /* Validate the input */
        AssertDebug(pTmpl);
        AssertDebug(pPDD);
        return(EC_INV_FUNC);
    }
    /*
     * Check if the page is even supported first.  If it is not supported,
     * don't consider it an error, just return.
     */
    pILT = ON_GenReq(&gTemplateMSAll, pPDD, NULL, &pPRP);
    ON_QueReq(pILT);
    status = MSC_ChkStat(pPRP);

    /*
     * Parse the data looking for the page we want.  If the page is not
     * supported, set IO error and bubble out of the function without
     * doing anything else.
     */
    if (status != EC_OK)
    {
        /* Release the request. */
        ON_RelReq(pILT);
        return status;
    }

    page = pTmpl->cmd[MS_PAGECODE] & MS_PAGECODE_MASK;

    if ((UINT32)pPRP->pSGL == 0xfeedf00d)
    {
        fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
        abort();
    }
    pHeader = (MODE_PAGE_HEADER *)((SGL_DESC *)(pPRP->pSGL + 1))->addr;

    /*
     * Check in the header to see if a block descriptor is present.
     * Set the page pointer to start at the first page based on the
     * header and descriptor.
     */
    pPagePtr = (MODE_PAGE *)((UINT8 *)(pHeader + 1) + bswap_16(pHeader->blockDescLength));

    for (status = EC_IO_ERR;
        (UINT8 *)pPagePtr < (UINT8 *)(pHeader) + pHeader->dataLength - sizeof(pHeader->dataLength);
        pPagePtr = (MODE_PAGE *)(&pPagePtr->data[pPagePtr->length]))
    {
        if (page == (pPagePtr->pageCode & MS_PAGECODE_MASK))
        {
            status = EC_OK;
            break;
        }
    }

    /* Release the request. */
    ON_RelReq(pILT);

    /*
     * The status will be set to IO Error if this page is not supported
     * by the device we are trying to change.  In this case, we will fall
     * out of the function without changing anything.
     */
    if (status != EC_OK)
    {
        return status;
    }

    /*
     * Send the Mode Sense (10) command - Page Control Field (PCF) is
     * set to "Return Changeable Parameters".
     */
    pILT = ON_GenReq(pTmpl, pPDD, NULL, &pPRP);
    ON_QueReq(pILT);
    status = MSC_ChkStat(pPRP);

    if (status != EC_OK)
    {
        return status;
    }

    if ((UINT32)pPRP->pSGL == 0xfeedf00d)
    {
        fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
        abort();
    }
    pHeader = (MODE_PAGE_HEADER *)((SGL_DESC *)(pPRP->pSGL + 1))->addr;

    pPagePtr = (MODE_PAGE *)((UINT8 *)(pHeader + 1) + bswap_16(pHeader->blockDescLength));

    /* Get the changeable params */
    changeableParams = *(UINT32 *)&pPagePtr->data[MSD_PARAMS];

    /*
     * Only for Informational exceptions control page, get the Interval timer
     * changeable bits, to find whether the it can be changed.
     */
    if (page == IEC_PAGE_CODE)
    {
        changeableIntervalTimer = *(UINT32 *)&pPagePtr->data[MSD_PARAMS+2];
    }

    /* Change the PCF to "Return current Parameters" and send the cmd */
    pPRP->cmd[2] = (pPRP->cmd[MS_PAGECODE] & MS_PAGECODE_MASK) | PCF_CURRENT;

    ON_QueReq(pILT);

    status = MSC_ChkStat(pPRP);

    if (status != EC_OK)
    {
        return status;
    }

    /* Save the current params */
    currentParams = *(UINT32 *)&pPagePtr->data[MSD_PARAMS];
    currentIntervalTimer = bswap_32(*(UINT32 *)&pPagePtr->data[MSD_PARAMS + 2]);

    /* Modify the Params according to the setBits & clearBits */
    newParams = (~changeableParams | clearBits) &
                ((changeableParams & setBits) | currentParams);

    *(UINT32 *)&pPagePtr->data[MSD_PARAMS] = newParams;

    /*
     * Make sure the Interval timer is changeable and it is less than
     * what we are going to set.
     */
    if ((changeableIntervalTimer == 0xffffffff) &&
        (currentIntervalTimer < MODE_IEC_SET_INTERVAL_TIMER))
    {
        /* Set the Interval timer to 0x1770 (6000) */
        *(UINT32 *)&pPagePtr->data[MSD_PARAMS + 2]  = bswap_32(MODE_IEC_SET_INTERVAL_TIMER);
        setIntervalTimer = TRUE;
    }

    /*
     * Check to see if the saveable bit is set in the page.  If
     * not, then there is no need to send the page since it will
     * be reset on the next power cycle.
     */
    if (pPagePtr->pageCode & MSD_PS_BIT)
    {
        pPRP->cmd[1] = 0x11;

        pPagePtr->pageCode &= ~MSD_PS_BIT;

        /*
         * Reset the Number of blocks, Dev params & Sense data
         * length to 0
         */
        pPRP->cmd[2]    = 0x0;
        pHeader->dataLength = 0;
        pHeader->devSpecParm = 0;

        if (pHeader->blockDescLength != 0)
        {
            ((MODE_PAGE_BLOCK_DESC*)(pHeader + 1))->densityCode = 0;
            ((MODE_PAGE_BLOCK_DESC*)(pHeader + 1))->numBlocks[0] = 0;
            ((MODE_PAGE_BLOCK_DESC*)(pHeader + 1))->numBlocks[1] = 0;
            ((MODE_PAGE_BLOCK_DESC*)(pHeader + 1))->numBlocks[2] = 0;
        }

        /*
         * If the desired params are not the same as the current params,
         * then change the cmd to Mode Select (10) and ship it out.
         * Cleanup before returning!
         */
        if ((newParams != currentParams) || (setIntervalTimer == TRUE))
        {
            pPRP->cmd[0] = 0x55;
            pPRP->func = PRP_OUTPUT;
            ON_QueReq(pILT);
            status = MSC_ChkStat(pPRP);
        }
    }

    ON_RelReq(pILT);

    return status;
}

/**
 ******************************************************************************
 **
 **  @brief      Replace and existing PDD with a new PDD and frees the old one
 **
 **  @param      PDD * oldPDD - existing pdd to replace
 **  @param      PDD *  NewPdd - new pdd to put in array
 **  @param      PDD *GlobalArray[] - array we are working with, PDX MDX or EDX
 **
 **
 ******************************************************************************
 **/
void ON_MigrateOldPDDtoNewPdd(PDD * oldPDD,PDD * NewPdd,PDD *GlobalArray[])
{
    UINT8 pid;

    pid = oldPDD->pid;
    NewPdd->geoLocationId = oldPDD->geoLocationId;
    NewPdd->geoFlags = oldPDD->geoFlags;
    NewPdd->hangCount = oldPDD->hangCount;
    NewPdd->devStat = oldPDD->devStat;
    NewPdd->pid = pid;
    DC_RelPDD(oldPDD);
    GlobalArray[pid] = NewPdd;
}

/**
 ******************************************************************************
 **
 **  @brief      generates a list of unique NN from a given list of PDDs
 **
 **  @param      PDD * pddlist[] - array of PDDs to process
 **  @param      UINT32 pddlistcount - number off PDDs in the array
 **  @param      UINT64 * wwnlist - array to fill with unique WWN
 **
 **  @return    number of entries in the wwnlist
 **
 ******************************************************************************
 **/
static UINT32 GenerateWWNList(PDD * pddlist[], UINT32 pddlistcount,UINT64 * wwnlist)
{
    UINT32 i;
    UINT32 j;
    UINT32 wwnlistcount;

    wwnlistcount = 0;

    // Parse out all unique nodenames
    for (i = 0; i < pddlistcount; i++)
    {
        // Is it already in the list?
        for (j = 0; j < pddlistcount; j++)
        {
            if (wwnlist[j] == pddlist[i]->wwn)
            {
                break;
            }
        }

        // Didn't match lets add it
        if (j >= pddlistcount)
        {
            wwnlist[wwnlistcount] = pddlist[i]->wwn;
            wwnlistcount++;
        }
    }
    return wwnlistcount;
}

/**
 ******************************************************************************
 **
 **  @brief      Calls o$inquire for all pdds in pddlist that match the wwn
 **
 **  @param      PDD * pddlist[] - array of PDDs to process
 **  @param      UINT32 pddlistcount - number off PDDs in the array
 **  @param      void *pwwn - pointer to 64bit WWN - this is screwy because
 **                 the taskcreate can't do 64 bit parameters.
 **  @param      UINT8 TestType - passsed to o$inquire
 **
 ******************************************************************************
 **/
void ON_Inquire(UINT32 foo UNUSED, UINT32 bar UNUSED, PDD *pddlist[],
                UINT32 pddlistcount, void *pwwn, UINT8 TestType)
{
    UINT32 i;
    UINT64 wwn;
    wwn = *(UINT64*)pwwn;
    for (i = 0; i < pddlistcount; i++)
    {
        if (pddlist[i]->wwn == wwn)
        {
            o$inquire(0,0,TestType,pddlist[i]);
        }
    }
    ON_Inquiry_Pending--;
}

/**
 ******************************************************************************
 **
 **  @brief      starts one ON_Inquire for each unique WWN waits
 **              for them all to complete
 **
 **  @param      PDD * pddlist[] - array of PDDs to process
 **  @param      UINT32 pddlistcount - number of PDDs in the array
 **  @param      UINT8 TestType - controls the thouroughness of o$inquire
 **
 ******************************************************************************
 **/
void ON_InquireAll(PDD * pddlist[], UINT32 pddlistcount, UINT8 TestType)
{

    UINT32 wwnlistcount;
    UINT64 * wwnlist;
    UINT32 i;

    if (pddlistcount == 0)
    {
        return;
    }
    wwnlist = s_MallocC(pddlistcount * sizeof(*wwnlist), __FILE__, __LINE__);
    wwnlistcount = GenerateWWNList(pddlist,pddlistcount,wwnlist);

    ON_Inquiry_Pending = wwnlistcount;
    for (i = 0; i < wwnlistcount; i++)
    {
        CT_fork_tmp = (unsigned long)"ON_Inquire";
        TaskCreate6(C_label_referenced_in_i960asm(ON_Inquire),
                    ON_INIT_PRI, (UINT32)pddlist, pddlistcount, (UINT32)&wwnlist[i], TestType);
    }
    while (ON_Inquiry_Pending)
    {
        TaskSleepMS(250);
    }
    TaskReadyByState(PCB_FILE_SYS_CLEANUP);  // Signal file system update/cleanup
    s_Free(wwnlist, pddlistcount * sizeof(*wwnlist), __FILE__, __LINE__);
}

/**
 ******************************************************************************
 **
 **  @brief      Calls ON_InitDrive for all pdds in pddlist that match the wwn
 **
 **  @param      PDD * pddlist[] - array of PDDs to process
 **  @param      UINT32 pddlistcount - number off PDDs in the array
 **  @param      void *pwwn - pointer to 64bit WWN - this is screwy because
 **                 the taskcreate can't do 64 bit parameters.
 **  @param      UINT8 TestType - passsed to ON_InitDrive
 **
 ******************************************************************************
 **/
void ON_InitializeDrive(UINT32 foo UNUSED, UINT32 bar UNUSED,PDD * pddlist[], UINT32 pddlistcount,void *pwwn, UINT8 TestType)
{
    UINT32 i;
    UINT64 wwn;

    wwn = *(UINT64*)pwwn;
    for (i = 0; i < pddlistcount; i++)
    {
        if (pddlist[i]->wwn == wwn)
        {
            O_drvinits++;
            ON_InitDrive (pddlist[i], TestType, &O_drvinits);
        }
    }
    ON_DriveInits--;
    TaskReadyByState(PCB_WAIT_SEM_1);   // Ready semaphore processes
}

/**
 ******************************************************************************
 **
 **  @brief      starts one ON_InitializeDrive for each unique WWN waits
 **              for them all to complete
 **
 **  @param      PDD * pddlist[] - array of PDDs to process
 **  @param      UINT32 pddlistcount - number off PDDs in the array
 **  @param      UINT8 TestType - controls the thouroughness of o$inquire
 **
 ******************************************************************************
 **/
void StartOninitDrive(PDD * pddlist[], UINT32 pddlistcount, UINT8 TestType)
{
    UINT32 wwnlistcount;
    UINT64 * wwnlist;
    UINT32 i;

    wwnlist = s_MallocC(pddlistcount * sizeof(*wwnlist), __FILE__, __LINE__);
    wwnlistcount = GenerateWWNList(pddlist,pddlistcount,wwnlist);

    ON_DriveInits = wwnlistcount;
    for (i = 0; i < wwnlistcount; i++)
    {
        CT_fork_tmp = (unsigned long)"ON_InitializeDrive";
        TaskCreate6(C_label_referenced_in_i960asm(ON_InitializeDrive),
                    ON_INIT_PRI,(UINT32)pddlist,pddlistcount,(UINT32)&wwnlist[i],TestType);
    }
    while (ON_DriveInits != 0 || O_drvinits != 0)
    {
        TaskSetMyState(PCB_WAIT_SEM_1);
        TaskSwitch();
    }
    s_Free(wwnlist, pddlistcount * sizeof(*wwnlist), __FILE__, __LINE__);
}

/**
    Busy code
 **/
/**
 ******************************************************************************
 **
 **  @brief      Checks if a VDisk is in a BUSY state and returns TRUE/FALSE
 **
 **              Walks thru the PSDs in the RDDs and checks if any of the
 **              Ccorresponding PIDs are in BUSY state. If any PID the VID is
 **              stripped on is BUSY, the function returns TRUE/ else FALSE
 **
 **  @param      UINT16 vid
 **
 **  @return     TRUE/FALSE
 **
 ******************************************************************************
 **/
static UINT32 ON_IsVDiskBusy(UINT16 vid)
{
    UINT16 i;
    RDD   *pRDD;
    PSD  **ppPSD;
    VDD   *pVDD = gVDX.vdd[vid];

    for (pRDD = pVDD->pRDD; pRDD != NULL && pRDD->type != RD_SLINKDEV; pRDD = pRDD->pNRDD)
    {
        ppPSD = ((PSD **)(pRDD + 1));
        for (i = 0; i < pRDD->psdCnt; i++)
        {
            if (gPDX.pdd[ppPSD[i]->pid] != NULL && BIT_TEST(gPDX.pdd[ppPSD[i]->pid]->flags, PD_BEBUSY))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/**
 ******************************************************************************
 **
 **  @brief      Sets the corresponding SS VDisks attrib BUSY bit
 **
 **
 **  @param      UINT16 vid
 **
 ******************************************************************************
 **/
static void busy_SSsrc(UINT16 vid)
{
    SSMS *pSSMS = gVDX.vdd[vid]->vd_outssms;

    while (pSSMS != NULL)
    {
        if (gVDX.vdd[pSSMS->ssm_ssvid] != NULL)
        {
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            BIT_SET(gVDX.vdd[pSSMS->ssm_ssvid]->attr, VD_BBEBUSY);
            fprintf(stderr, "\t\t\t<vid(%d) st(%d) atr(0x%04x) BUSY> SS!!!\n",
                    pSSMS->ssm_ssvid,gVDX.vdd[pSSMS->ssm_ssvid]->status, gVDX.vdd[pSSMS->ssm_ssvid]->attr);
        }
        pSSMS = pSSMS->ssm_link;
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Sets the SS source VDisk attrib BUSY bit
 **
 **
 **  @param      UINT16 vid
 **
 ******************************************************************************
 **/
static void busy_SS(UINT16 vid)
{
    SSMS *pSSMS = gVDX.vdd[vid]->vd_incssms;

    if (pSSMS != NULL && gVDX.vdd[pSSMS->ssm_srcvid] != NULL)
    {
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        BIT_SET(gVDX.vdd[pSSMS->ssm_srcvid]->attr, VD_BBEBUSY);
        fprintf(stderr, "\t\t\t<vid(%d) st(%d) atr(0x%04x) BUSY> SS source!!!\n",
                pSSMS->ssm_srcvid,gVDX.vdd[pSSMS->ssm_srcvid]->status, gVDX.vdd[pSSMS->ssm_srcvid]->attr);
        busy_SSsrc(pSSMS->ssm_srcvid);
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Sets the attrib BUSY bit for all the snapshots and their sources
 **
 **
 **  @param      UINT16 vid
 **
 ******************************************************************************
 **/
static void busy_Spool(void)
{
    UINT16 i;

    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        if ((gVDX.vdd[i] != NULL)
             && ((gVDX.vdd[i]->vd_incssms != NULL)
             || (gVDX.vdd[i]->vd_outssms != NULL)))
        {
            /*
             * TODO - We may have to check for corresponding Snappool VID as
             * we could potentially have two Snappools residing on the same
             * controller on failover
             */
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            BIT_SET(gVDX.vdd[i]->attr, VD_BBEBUSY);
            fprintf(stderr, "\t\t\t<vid(%d) st(%d) atr(0x%04x) BUSY> SS or source!!!\n",
                    i,gVDX.vdd[i]->status, gVDX.vdd[i]->attr);
        }
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Sets the attrib BUSY bit for all the async sources
 **
 **
 **  @param      UINT16 vid
 **
 ******************************************************************************
 **/
static void busy_Apool(void)
{
    UINT16 i;
    VDD *vdd;

    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        if ((gVDX.vdd[i] != NULL)
             && (BIT_TEST(gVDX.vdd[i]->attr, VD_BASYNCH))
             && (BIT_TEST(gVDX.vdd[i]->attr, VD_BDCD))
             && (gVDX.vdd[i]->pDCD != NULL)
             && (gVDX.vdd[i]->pDCD->cor != NULL)
             && ((vdd = gVDX.vdd[i]->pDCD->cor->srcvdd) != NULL))
        {
            /* If the src vid is dest VID for some other src, update that src too!!! */
            do {
                BIT_SET(vdd->attr, VD_BBEBUSY);
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                fprintf(stderr, "\t\t\t<vid(%d) st(%d) atr(0x%04x) BUSY> Async Source!!!\n",
                        vdd->vid,vdd->status, vdd->attr);
            } while (vdd->pDCD != NULL && vdd->pDCD->cor != NULL &&
                    (vdd = vdd->pDCD->cor->srcvdd) != NULL);
        }
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Sets the attrib BUSY bit for Copy source(s)
 **
 **
 **  @param      UINT16 vid
 **
 ******************************************************************************
 **/
static void busy_CopyDest(UINT16 vid)
{
    VDD *vdd;

    vdd = gVDX.vdd[vid];
    while (vdd->pDCD != NULL && vdd->pDCD->cor != NULL &&
           (vdd = vdd->pDCD->cor->srcvdd) != NULL)
    {
        /* TODO - Do we need to check if the source is a SS? */
        BIT_SET(vdd->attr, VD_BBEBUSY);
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        fprintf(stderr, "\t\t\t<vid(%d) st(%d) atr(0x%04x) BUSY> Copy Source!!!\n",
                vdd->vid,vdd->status, vdd->attr);
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Clears the SS source VDisk attrib BUSY bit
 **
 **
 **  @param      UINT16 vid
 **
 ******************************************************************************
 **/
static void clear_SSsrc(UINT16 vid)
{
    OGER *pOGR = NULL;
    SSMS *pSSMS = gVDX.vdd[vid]->vd_outssms;

    /* If SPool is BUSY, return. Else, cleat the SPool BUSY condition. */
    for (pOGR = pSSMS->ssm_frstoger; pOGR != NULL; pOGR = pOGR->ogr_link)
    {
        if (BIT_TEST(gVDX.vdd[pOGR->ogr_vid]->attr, VD_BBEBUSY))
        {
            if (ON_IsVDiskBusy(pOGR->ogr_vid))
            {
                return;
            }
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            BIT_CLEAR(gVDX.vdd[pOGR->ogr_vid]->attr, VD_BBEBUSY);
            fprintf(stderr, "\t\t\t<vid(%d) st(%d) atr(0x%04x) CLEARed> SPool!!!\n",
                    pOGR->ogr_vid,gVDX.vdd[pOGR->ogr_vid]->status, gVDX.vdd[pOGR->ogr_vid]->attr);
        }
    }
    /*
     * If any of the SS VIDs are still BUSY, exit.
     * NOTE: we are checking the condition of the PIDs
     *       on which the VIDs are stripped across and
     *       not the VID attr.
     */
    while (pSSMS != NULL)
    {
        if (ON_IsVDiskBusy(pSSMS->ssm_ssvid))
        {
            return;
        }
        pSSMS = pSSMS->ssm_link;
    }
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    BIT_CLEAR(gVDX.vdd[vid]->attr, VD_BBEBUSY);
    fprintf(stderr, "\t\t\t<vid(%d) st(%d) atr(0x%04x) CLEARed> SS Source!!!\n",
            vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
}

/**
 ******************************************************************************
 **
 **  @brief      Clears the SS VDisk attrib BUSY bit
 **
 **
 **  @param      UINT16 vid
 **
 ******************************************************************************
 **/
static void clear_SS(UINT16 vid)
{
    SSMS *pSSMS = gVDX.vdd[vid]->vd_incssms;

    if (!ON_IsVDiskBusy(pSSMS->ssm_srcvid))
    {
        clear_SSsrc(pSSMS->ssm_srcvid);
        if (!BIT_TEST(gVDX.vdd[pSSMS->ssm_srcvid]->attr, VD_BBEBUSY))
        {
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            BIT_CLEAR(gVDX.vdd[vid]->attr, VD_BBEBUSY);
            fprintf(stderr, "\t\t\t<vid(%d) st(%d) atr(0x%04x) CLEARed> Snapshot!!!\n",
                    vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
        }
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Clears the VDisk attrib BUSY bit for the corresponding source(s)
 **
 **
 **  @param      UINT16 vid
 **
 ******************************************************************************
 **/
static void clear_CopyDest(UINT16 vid)
{
    VDD *vdd;
    SCD *pSCD;

    /*
     * If the VID is also a Copy source, check if any of the dest
     * is BUSY. If yes, don't clear the BUSY state.
     */
    for (pSCD = gVDX.vdd[vid]->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
    {
        if (pSCD->cor != NULL && (vdd = pSCD->cor->destvdd) != NULL &&
            ON_IsVDiskBusy(vdd->vid))
        {
            return;
        }
    }

    /* Clear the BUSY on this VID */
    BIT_CLEAR(gVDX.vdd[vid]->attr, VD_BBEBUSY);
    fprintf(stderr, "\t\t\t<vid(%d) st(%d) atr(0x%04x) CLEARed> Copy dest!!!\n",
            vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);

    /* Walk the source link and update them */
    vdd = gVDX.vdd[vid];
    while (vdd->pDCD != NULL && vdd->pDCD->cor != NULL &&
           (vdd = vdd->pDCD->cor->srcvdd) != NULL)
    {
        if (ON_IsVDiskBusy(vdd->vid))
        {
            return;
        }
        BIT_CLEAR(vdd->attr, VD_BBEBUSY);
        fprintf(stderr, "\t\t\t<vid(%d) st(%d) atr(0x%04x) CLEARed> Copy source!!!\n",
                vdd->vid,vdd->status, vdd->attr);
    }
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
}

/**
 ******************************************************************************
 **
 **  @brief      Checks the given VID and sets the BUSY condition depending
 **              on various factors. It also checks the related VIDs in the case of
 **              Snappool/Apool/SS/Copy sources, etc...
 **
 **              1. If VID is Snappool, busy all Snapshots and corresponding sources
 **              2. If VID is Snapshot, busy corresponding source
 **              3. If VID is Snapshot source, busy corresponding Snapshots
 **              4. If VID is APool, busy all async sources
 **              5. If VID is copy dest, busy corresponding source(s)
 **
 **  @param      UINT16 vid
 **
 **  @return     none
 **
 ******************************************************************************
 **/
static void ON_VIDBusy(UINT16 vid)
{
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    BIT_SET(gVDX.vdd[vid]->attr, VD_BBEBUSY);

    if (BIT_TEST(gVDX.vdd[vid]->attr, VD_BSNAPPOOL))
    {
        /* VID is SPool, busy all Snapshots and corresponding sources */
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) BUSY> SPool!!!\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
        busy_Spool();
    }
    else if (gVDX.vdd[vid]->vd_incssms != NULL)
    {
        /* VID is Snapshot, busy corresponding source */
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) BUSY> Snapshot!!!\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
        busy_SS(vid);
    }
    else if (gVDX.vdd[vid]->vd_outssms != NULL)
    {
        /* VID is Snapshot source, busy corresponding Snapshots */
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) BUSY> SS Source!!!\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
        busy_SSsrc(vid);
    }
    else if (BIT_TEST(gVDX.vdd[vid]->attr, VD_BASYNCH) && !BIT_TEST(gVDX.vdd[vid]->attr, VD_BVLINK))
    {
        /* VID is APool, busy all async sources */
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) BUSY> APool!!!\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
        busy_Apool();
    }
    else if (BIT_TEST(gVDX.vdd[vid]->attr, VD_BDCD))
    {
        /* VID is copy dest, busy corresponding source(s) */
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) BUSY> Copy dest!!!\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
        busy_CopyDest(vid);
    }
    else {
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) BUSY>\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Checks the given VID and clears the BUSY condition depending
 **              on various factors. It also checks the related VIDs in the case of
 **              Snappool/Apool/SS/Copy sources, etc...
 **
 **              1. If VID is Snappool/Apool, just clear it. The Async/SSs and their sources
 **                 will be fixed independently
 **              2. If VID is Snapshot source, clear the VID's BUSY only if
 **                 neither the Snappool nor it's SSs are busy (stripped across busy PIDs).
 **              3. If VID is SS, clear the VID's BUSY only if neither the Snappool
 **                 nor it's source are busy (stripped across busy PIDs).
 **              4. If VID is Copy dest, clear the VID's BUSY ond also update the
 **                 copy sources(s) if they are not busy (stripped across busy PIDs).
 **              5. if non of the above, clear the BUSY state
 **
 **  @param      UINT16 vid
 **
 **  @return     none
 **
 ******************************************************************************
 **/
static void ON_VIDClear(UINT16 vid)
{
    if (BIT_TEST(gVDX.vdd[vid]->attr, VD_BSNAPPOOL) ||
        (BIT_TEST(gVDX.vdd[vid]->attr, VD_BASYNCH) && !BIT_TEST(gVDX.vdd[vid]->attr, VD_BVLINK)))
    {
        /*
         * VID is SPool/Apool, just clear it. The Async/SSs and their sources
         * will be fixed independently
         */
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        BIT_CLEAR(gVDX.vdd[vid]->attr, VD_BBEBUSY);
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) CLEARed> S/A-pool!!!\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
    }
    else if (gVDX.vdd[vid]->vd_outssms != NULL)
    {
        /*
         * VID is Snapshot source, clear the VID's BUSY only if
         * neither the Snappool nor it's SSs are busy (stripped across busy PIDs).
         */
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) CLEAR> SS Source!!!\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
        clear_SSsrc(vid);
    }
    else if (gVDX.vdd[vid]->vd_incssms != NULL)
    {
        /*
         * VID is SS, clear the VID's BUSY only if neither the Snappool
         * nor it's source are busy (stripped across busy PIDs).
         */
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) CLEAR> Snapshot!!!\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
        clear_SS(vid);
    }
    else if (BIT_TEST(gVDX.vdd[vid]->attr, VD_BDCD))
    {
        /*
         * VID is Copy dest, clear the VID's BUSY ond also update the
         * copy sources(s) if they are not busy (stripped across busy PIDs).
         */
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) CLEAR> Copy dest!!!\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
        clear_CopyDest(vid);
    }
    else if (BIT_TEST(gVDX.vdd[vid]->attr, VD_BSCD))
    {
        VDD *vdd;
        SCD *pSCD;

        /*
         * VID is Copy source, Check if any dest VID is BUSY. If yes,
         * don't clear BUSY
         */
        for (pSCD = gVDX.vdd[vid]->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
        {
            if (pSCD->cor != NULL && (vdd = pSCD->cor->destvdd) != NULL &&
                ON_IsVDiskBusy(vdd->vid))
            {
                return;
            }
        }
        BIT_CLEAR(gVDX.vdd[vid]->attr, VD_BBEBUSY);
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) CLEAR> Copy src!!!\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
    }
    else
    {
        /* None of the above, clear the BUSY state */
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        BIT_CLEAR(gVDX.vdd[vid]->attr, VD_BBEBUSY);
        fprintf(stderr, "\t\t<PAB: vid(%d) st(%d) atr(0x%04x) CLEARed>\n",
                vid,gVDX.vdd[vid]->status, gVDX.vdd[vid]->attr);
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Updates all the VDisks' attr based on the PDD flags
 **
 **              This function is called from ON_BEBusy, ON_BEClear
 **              and at the end of BE discovery process.
 **
 **  @param      UINT16 pid
 **
 **  @return     none
 **
 **  @attention  BE Discovery is expected to clear the BUSY condition
 **              on the PDDs discovered in the discovery process before
 **              calling this function.
 **
 ******************************************************************************
 **/
void ON_UpdateVDisks(void)
{
    UINT16 i = 0;

    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        if (gVDX.vdd[i] != NULL)
        {
            if (ON_IsVDiskBusy(i))
            {
                if (!BIT_TEST(gVDX.vdd[i]->attr, VD_BBEBUSY))
                {
                    ON_VIDBusy(i);
                }
            }
            else if (BIT_TEST(gVDX.vdd[i]->attr, VD_BBEBUSY))
            {
                ON_VIDClear(i);
            }
        }
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Sets the BE BUSY conditions on the PID/bay and also on
 **              the corresponding VIDs
 **
 **              This function is called on ON event 0x85 or on a BUSY response
 **              to any cmd from an ISE.  OR on path loss or other conditions
 **              when we want to stop IO to a PID but not fail it.
 **
 **  @param      pid            - Pid to do
 **  @param      justthisonepid - True if only one pid to do.
 **
 **  @return     none
 **
 ******************************************************************************
 **/
static UINT16 busy_pids[MAX_PHYSICAL_DISKS];
void ON_BEBusy(UINT16 pid,UINT32 TimetoFail, UINT8 justthisonepid)
{
    UINT16 i = 0;
    UINT16 count = 0;

    if (gPDX.pdd[pid] != NULL)
    {
        /* Mark all the PIDs residing on this ISE as BUSY */
#if defined(MODEL_7000) || defined(MODEL_4700)
        fprintf(stderr, "\t<ISE-%d (0x%llX)> PIDs [", gPDX.pdd[pid]->ses, gPDX.pdd[pid]->wwn);
#else  /* MODEL_7000 || MODEL_4700 */
        fprintf(stderr, "\t<SES-%d (0x%llX)> PIDs [", gPDX.pdd[pid]->ses, gPDX.pdd[pid]->wwn);
#endif /* MODEL_7000 || MODEL_4700 */
        for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
        {
            // Doing just  one pid so set i to pid.
            if (justthisonepid)
            {
                i = pid;
            }
            if (gPDX.pdd[i] != NULL && gPDX.pdd[i]->wwn == gPDX.pdd[pid]->wwn &&
                gPDX.pdd[i]->devClass != PD_UNLAB)
            {
                fprintf(stderr, " %d", i);
                if (!BIT_TEST(gPDX.pdd[i]->flags,PD_BEBUSY))
                {
                    BIT_SET(gPDX.pdd[i]->flags,PD_BEBUSY);
                    busy_pids[count++] = i;
                    gPDX.pdd[i]->ts_cnt = 0;
                }

                DEV *dev = gPDX.pdd[i]->pDev;

                if (dev != NULL)
                {
                    if (dev->TimetoFail < TimetoFail)
                    {
                        dev->TimetoFail = TimetoFail;
                    }
                    fprintf(stderr, "-%d", dev->TimetoFail);
                }
            }

            // Doing just onepid so exit the for loop.
            if (justthisonepid)
            {
                break;
            }
        }

        fprintf(stderr, " ] BUSY-ed [");

        /* Update the VDisks that are affected */
        if (count)
        {
            for (i = 0; i < count; i++)
            {
                fprintf(stderr, " %d", busy_pids[i]);
            }
            fprintf(stderr, " ]\n");
#if defined(MODEL_7000) || defined(MODEL_4700)
            ISE_LogEvent(gPDX.pdd[pid]->ses, TRUE);
#endif /* MODEL_7000 || MODEL_4700 */
            ON_UpdateVDisks();
        }
        else
        {
            fprintf(stderr, " NONE ]\n");
        }
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Clears the BE BUSY conditions on the PID/bay and also on
 **              the corresponding VIDs
 **
 **              This function is called on ISE event 0x86 or on a good response
 **              to a TUR cmd to an ISE in BUSY condition.
 **
 **  @param      UINT16 pid
 **
 **  @return     none
 **
 ******************************************************************************
 **/
void ON_BEClear(UINT16 pid,UINT8 justthisonepid)
{
    UINT16 i = 0;
    UINT16 count = 0;
    UINT16 clear_pids[MAX_ISE_LUNS + 4];

    if (gPDX.pdd[pid] != NULL)
    {
        /* Clear the BUSY condition on all the PIDs residing on this ISE */
#if defined(MODEL_7000) || defined(MODEL_4700)
        fprintf(stderr, "\t<ISE-%d (0x%llX)> PIDs [", gPDX.pdd[pid]->ses, gPDX.pdd[pid]->wwn);
#else  /* MODEL_7000 || MODEL_4700 */
        fprintf(stderr, "\t<SES-%d (0x%llX)> PIDs [", gPDX.pdd[pid]->ses, gPDX.pdd[pid]->wwn);
#endif /* MODEL_7000 || MODEL_4700 */
        for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
        {
            // Doing just  one pid so set i to pid
            if (justthisonepid)
            {
                i = pid;
            }
            if (gPDX.pdd[i] != NULL && gPDX.pdd[i]->wwn == gPDX.pdd[pid]->wwn)
            {
                fprintf(stderr, " %d", i);
                if (BIT_TEST(gPDX.pdd[i]->flags,PD_BEBUSY))
                {
                    if (gPDX.pdd[i]->pDev != NULL && gPDX.pdd[i]->pDev->port != 0xff)
                    {
                        BIT_CLEAR(gPDX.pdd[i]->flags,PD_BEBUSY);
                        gPDX.pdd[i]->pDev->TimetoFail = 0;
                        if (count >= MAX_ISE_LUNS + 4)
                        {
fprintf(stderr, "\nToo many PIDs found, more than %d reached (MAX_ISE_LUNS + 4)\n", MAX_ISE_LUNS + 4);
                            break;
                        }
                        clear_pids[count++] = i;
                    }
                }
                gPDX.pdd[i]->ts_cnt = 0;
            }

            // Doing just onepid so exit the for loop
            if (justthisonepid)
            {
                break;
            }
        }

        fprintf(stderr, " ] CLEAR-ed [");

        /* Update the VDisks that are affected */
        if (count)
        {
            for (i = 0; i < count; i++)
            {
                fprintf(stderr, " %d", clear_pids[i]);
            }
            fprintf(stderr, " ]\n");
#if defined(MODEL_7000) || defined(MODEL_4700)
            ISE_LogEvent(gPDX.pdd[pid]->ses, FALSE);
#endif /* MODEL_7000 || MODEL_4700 */
            ON_UpdateVDisks();
        }
        else
        {
            fprintf(stderr, " NONE ]\n");
        }
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Called from ONLINE poll completer when a TUR cmd is successful
 **              for a BUSY-ed PID.
 **
 **  @param      UINT16 pid
 **
 **  @return     none
 **
 ******************************************************************************
 **/
void ON_TURSuccess(UINT16 pid)
{
    time_t t;

    if (gPDX.pdd[pid] != NULL)
    {
        /* Clear the BUSY condition on all the PIDs residing on this ISE */
        time(&t);
        gPDX.pdd[pid]->ts_cnt++;
#if defined(MODEL_7000) || defined(MODEL_4700)
        fprintf(stderr, "<ISE-%02d (port:%d wwn:0x%llX) EVENT pid:%d [ TUR Success, %d]> - %s",
                gPDX.pdd[pid]->ses, gPDX.pdd[pid]->pDev->port, gPDX.pdd[pid]->wwn,
                pid, gPDX.pdd[pid]->ts_cnt, ctime(&t));
#else  /* MODEL_7000 || MODEL_4700 */
        fprintf(stderr, "<SES-%02d (port:%d wwn:0x%llX) EVENT pid:%d [ TUR Success, %d]> - %s",
                gPDX.pdd[pid]->ses, gPDX.pdd[pid]->pDev->port, gPDX.pdd[pid]->wwn,
                pid, gPDX.pdd[pid]->ts_cnt, ctime(&t));
#endif /* MODEL_7000 || MODEL_4700 */
        if (gPDX.pdd[pid]->ts_cnt == 10)
        {
            ON_BEClear(pid,0);
        }
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
