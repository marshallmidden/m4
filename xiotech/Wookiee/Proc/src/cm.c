/* $Id: cm.c 161430 2013-08-24 00:02:54Z marshall_midden $ */
/**
******************************************************************************
**
**  @file   cm.c
**
**  @brief  C portions of the Copy Manager.
**
**  Copyright (c) 2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <string.h>
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "QU_Library.h"
#include "CT_defines.h"
#include "apool.h"
#include "ecodes.h"
#include "sgl.h"
#include "vdd.h"
#include "virtual.h"
#include "scr.h"
#include "pm.h"
#include "mem_pool.h"


/**
******************************************************************************
**  Definitions
******************************************************************************
**/

#define DEBUG   1

enum qpts { qpt_high, qpt_norm, qpt_low, qpt_num };


/**
******************************************************************************
**  Data structures
******************************************************************************
**/
typedef struct QPT
{
    QU      *que;
    UINT8   cnt;    /* Number of active scr's */
    UINT8   limit;  /* scr limit */
    UINT8   unused[2];
}   QPT;


/**
******************************************************************************
**  Variables
******************************************************************************
**/

static UINT32   cm_rcpstate;    /* Copy read op comp. process state errors */
static UINT32   cm_rcpstatus;   /* Copy read op comp. status errors */
static UINT32   cm_rcpabort;    /* Copy read op comp. read aborts */
static UINT32   cm_wcpstate;    /* Copy write op comp. process state errors */
static UINT32   cm_wcpstatus;   /* Copy write op comp. errors */
static UINT32   cm_wcpabort;    /* COpy write op comp. aborts */

extern QPT  cm_exec_qpt[qpt_num];
extern QPT  cm_exec_qpt_low;
extern QPT  cm_exec_qpt_norm;
extern QPT  cm_exec_qpt_high;
extern QU       cm_wc_qu;
extern QU       cm_rc_qu;
extern UINT32   ct_CM$rc_que;
extern UINT32   ct_CM$wc_que;
extern UINT32   ct_cm$up1comp;


/**
******************************************************************************
**  Function prototypes
******************************************************************************
**/
NORETURN void cm_rd_comp(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED);
NORETURN void cm_wr_comp(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED);
NORETURN void cm_exec(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED);
ILT *build_sec_vrp(SCD *, UINT32 func, VRP *, ILT *);
ILT *cm_wp2_copy(SCD *, UINT32 func, VRP *, ILT *);

extern void CCSM$reg_sync(UINT32, COR *);
extern void complete_scr(void *, SCRP1 *);
extern void K_comp(ILT *);
extern UINT32 cal_seg_bit(UINT32 *seg, UINT64 sda, UINT64 eda, UINT64 devCap, UINT8 cmorss);

/*
******************************************************************************
**
**  @brief  Calculate VDA of segment
**
**  @param  seg - Segment number
**
**  @return VDA of segment
**
******************************************************************************
*/
static UINT64 seg2vda(UINT64 seg)
{
    return seg << SEC2SEG_SF;
}


/**
******************************************************************************
**
**  @brief  Clear a segment bit
**
**  @param  seg_num - Segment number
**  @param  cor     - Pointer to COR
**
**  @return none
**
******************************************************************************
**/
static void clr_segment_bit(UINT32 seg_num, COR *cor)
{
    RM      *rm = cor->rmaptbl;
    SM      *sm;
    UINT32  seg_bit;
    UINT32  region_ix;
    UINT32  seg_ix;

    if (!rm)
    {
        return;
    }

    region_ix = seg_num >> (SMBITS2WRD_SF + SMWRD2REG_SF);
    seg_bit = seg_num & 0x1F;
    seg_ix = (seg_num >> SMBITS2WRD_SF) & SMWRDIDX_MASK;
    sm = rm->regions[region_ix];
    if (!sm)
    {
        return;
    }

    if (BIT_TEST(sm->segments[seg_ix], 31 - seg_bit))
    {
        BIT_CLEAR(sm->segments[seg_ix], 31 - seg_bit);
        if (sm->cnt)
        {
            --sm->cnt;
        }
    }

    /* See if this segment map is now clear */
    for (seg_ix = 0; seg_ix < SMTBLSIZE / 4; ++seg_ix)
    {
        if (sm->segments[seg_ix])
        {
            return;
        }
    }

    /* Segment map is clear, deallocate it */

    rm->regions[region_ix] = NULL;  /* Clear pointer to segment map */
#ifdef M4_DEBUG_SM
    fprintf(stderr, "%s: put_sm %p\n", __func__, sm);
#endif /* M4_DEBUG_SM */

    put_sm(sm);

    CCSM$reg_sync(region_ix, cor);  /* Inform CCSM of region sync */
}


/**
******************************************************************************
**
**  @brief  Check if a segment bit is set
**
**  @param  seg_num - Segment number to check
**  @param  cor     - Pointer to COR
**
**  @return TRUE if segment bit set, FALSE otherwise
**
******************************************************************************
**/
// NOTE: seg_num is 22 bits maximum.
static int  chk_segment_bit(UINT32 seg_num, COR *cor)
{
    RM      *rm = cor->rmaptbl;
    SM      *sm;
    UINT32  seg_bit;
    UINT32  region_ix;
    UINT32  seg_ix;

    if (!rm)
    {
        return FALSE;
    }

    region_ix = seg_num >> (SMBITS2WRD_SF + SMWRD2REG_SF);
    seg_bit = seg_num & 0x1F;
    seg_ix = (seg_num >> SMBITS2WRD_SF) & SMWRDIDX_MASK;
    sm = rm->regions[region_ix];
    if (!sm)
    {
        return FALSE;
    }

    return BIT_TEST(sm->segments[seg_ix], 31 - seg_bit);
}


/**
******************************************************************************
**
**  @brief  Register a new scmt to the cm
**
**  @param  cm      - Pointer to the CM structure
**  @param  scmt    - Pointer to the new scmt
**
**  @return none
**
******************************************************************************
**/
static void register_scmt(CM *cm, SCMT *scmt)
{
    SCMT *this_scmt;
    SCMT *prev_scmt = NULL;

    for (this_scmt = cm->scmts; this_scmt; this_scmt = this_scmt->link)
    {
        prev_scmt = this_scmt;
    }

    if (prev_scmt)
    {
        prev_scmt->link = scmt;     /* Add to end of list */
    }
    else
    {
        cm->scmts = scmt;           /* Start list with this one */
    }
}


/**
******************************************************************************
**
**  @brief  Build a vrp and sgl for a copy read operation
**
**  @param  scio    - Pointer to SCIO1 structure
**
**  @return none
**
******************************************************************************
**/
static void build_copy_read_vrp(SCIO1 *scio)
{
    VRP     *vrp = (VRP *)(((ILT *)scio)->ilt_normal.w0);
    UINT32  sglSize;
    SGL     *sgl;

    vrp->function = VRP_COPYINPUT;          /* Set function */
    vrp->strategy = scio->scr1->strategy;   /* Set strategy */
    vrp->status = 0;                        /* Clear status */
    vrp->vid = scio->scr1->srcvdd->vid;     /* Set VID */
    vrp->length = scio->scme->numsec;       /* Set length -- note: is small */
    vrp->startDiskAddr = scio->scme->vdsda; /* Save starting disk address */

    sglSize = vrp->length << 9;             /* Compute buffer size */
    sgl = m_asglbuf(sglSize);               /* Allocate SGL and buffer */
    vrp->pSGL = sgl;                        /* Set SGL ptr */
    vrp->sglSize = sglSize;                 /* Set VRP size */
    vrp->gen2 = 0;                          /* Clear vr_use2 flags */
}


/**
******************************************************************************
**
**  @brief  Build SCMTE structures for segment copy
**
**  @param  scmt    - Pointer to SCMT
**  @param  scr1    - Pointer to SCRP1
**
**  @return Pointer to list of SCMTE structures
**
******************************************************************************
**/
static SCMTE    *build_scmte_list(SCMT *scmt, SCRP1 *scr1)
{
    const UINT64    devcap = scr1->srcvdd->devCap;  /* Source vdisk capacity */
    const UINT32    extras = devcap >> 32;  /* Number of 1mb reads to do */
    UINT64  sda;
    UINT32  length = SEGSIZE_SEC / CPSEGNUM;    /* Size of each read (128k) */
    UINT32  number2do = CPSEGNUM << extras;
    SCMTE   *first = NULL;
    SCMTE   *prev = NULL;

    /* Calculate Starting Disk Address. */
    sda = scmt->segnum;
    sda <<= SEC2SEG_SF + extras;

    while (number2do-- && sda < devcap)
    {
        UINT64  eda = sda + length - 1; /* Calculate EDA */
        SCMTE   *scmte = get_scmte();   /* Allocate SCMTE */
        SCIO1   *scio = get_scio();     /* Allocate SCIO (sci1 level) */

        /* If the calculated EDA is greater than the device capacity, change
         * length and recalculate the EDA.
         */
        if (eda >= devcap)
        {
            length = devcap - sda;
            eda = sda + length - 1;
        }

#ifdef M4_DEBUG_SCMTE
        fprintf(stderr, "%s%s:%u get_scmte %p\n", FEBEMESSAGE, __FILE__, __LINE__, scmte);
#endif /* M4_DEBUG_SCMTE */

        scmte->link = NULL;             /* Clear link to next scme */
        if (first)                      /* If this is not the first scmte */
        {
            prev->link = scmte;         /* Link this one to previous */
        }
        else
        {
            first = scmte;              /* Set first SCMTE address */
        }
        prev = scmte;

#ifdef M4_DEBUG_SCIO
        fprintf(stderr, "%s%s:%u get_scio %p\n", FEBEMESSAGE, __FILE__, __LINE__, scio);
#endif /* M4_DEBUG_SCIO */

        scio->cm = scr1->cm;
        scio->scmt = scmt;
        scio->scr1 = scr1;
        scio->scr2 = scmt->scr2;
        scio->scme = scmte;             /* Save scme address. */

        scmte->ilt = (ILT *)scio;       /* Save scio address. */
        scmte->vdsda = sda;
        scmte->vdeda = eda;
        scmte->numsec = length;         /* Save length - note: is small. */
        scmte->retry = 2;               /* Save retry counter. */

        build_copy_read_vrp(scio);

        sda += length;                  /* Next SDA = current SDA + length. */
    }

    return first;
}


/**
******************************************************************************
**
**  @brief  To provide a means of starting a secondary copy operation.
**
**  @param  scr1    - Pointer to SCRP1
**
**  @return SCMT address
**
******************************************************************************
**/
static SCMT *cm_build_scmt(SCRP1 *scr1)
{
    SCMT    *scmt = get_scmt();         /* Allocate SCMT */
    ILT     *ilt = (ILT *)scr1;

#ifdef M4_DEBUG_SCMT
    fprintf(stderr, "%s: get_scmt %p\n", FEBEMESSAGE, __func__, scmt);
#endif /* M4_DEBUG_SCMT */

    scmt->scr2 = (SCRP2 *)(ilt + 1);    /* Save scr2 address */
    scmt->segnum = scr1->segnum;        /* Save segment number */

    scmt->headscme = build_scmte_list(scmt, scr1);  /* Build scmte structures */

    register_scmt(scr1->cm, scmt);

    return scmt;
}                               /* End of cm_build_scmt */


/**
******************************************************************************
**
**  @brief  Reissue read to source for copy
**
**  @param  scio1   - Pointer to SCIO1
**
**  @return none
**
******************************************************************************
**/
static void reissue_read(SCIO1 *scio1)
{
    VRP     *vrp = scio1->vrp;
    SCMTE   *scme = scio1->scme;
    CM      *cm = scio1->cm;
    COR     *cor;
    UINT16  src_vid;

    cor = cm->cor;
    src_vid = COR_GetRSVID(cm->cor);
    scme->pstate = PS_READ;         /* Reset process state to read */
    vrp->status = 0;
    vrp->function = VRP_COPYINPUT;
    vrp->vid = src_vid;
    vrp->startDiskAddr = scme->vdsda;
    vrp->length = scme->numsec;
    vrp->gen2 = 0;                  /* Clear vr_use2 */
    EnqueueILT((void *)V_que, (ILT *)scio1, (void *)&ct_CM$rc_que);
}


/**
******************************************************************************
**
**  @brief  Process read completion VRP from source VDisk
**
**  Checks for correct process state, checks for abort read
**  process state and if so reissues the read operation, sets
**  up to issue the write operation.
**
**  A separate completion routine handles the completion of write
**  VRP requests.
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
NORETURN void cm_rd_comp(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED)
{
    ILT     *ilt;
    SCIO1   *scio1;
    VRP     *vrp;
    SCMTE   *scme;
    CM      *cm;

    for (;;)
    {
        TaskSwitch();           /* Exchange processes */

        ilt = QU_DequeReqILT(&cm_rc_qu);    /* Get next queued request */
        if (!ilt)
        {
            TaskSetMyState(PCB_NOT_READY);
            continue;
        }

        scio1 = (SCIO1 *)ilt;
        vrp = scio1->vrp;
        scme = scio1->scme;
        cm = scio1->cm;

        switch (scme->pstate)       /* Check state of completion */
        {
        case PS_READ:
            if (vrp->status == EC_OK)
            {
                /*
                 * Read completion status was good. Set up to write data to
                 * destination VID. The SDA may have been corrupted so it
                 * should be reset to it's original value.
                 *
                 * Crash - JIRA SAN-1772 - 2009-10-08 -- BE CM structure
                 * used after freed eaeaeaea.
                 */
                scme->pstate = PS_WRITE;
                vrp->function = VRP_COPYOUTPUT;
                vrp->vid = COR_GetRDVID(cm->cor);
                vrp->startDiskAddr = scme->vdsda;
                vrp->length = scme->numsec;
                vrp->gen2 = 0;

                /*
                 * If the dest VDD is an alink then mark the VRP so that
                 * the apool mover task can identify it and decrement
                 * the copy data counter.
                 */

                VDD *vdd = cm->cor->destvdd;

                if (vdd && BIT_TEST(vdd->attr, VD_BASYNCH))
                {
                    vrp->gen1 = SUPER_MAGIC_FLAG;
                }
                EnqueueILT((void *)V_que, ilt, (void *)&ct_CM$wc_que);
                continue;
            }

            /* Read completion status error */

            ++cm_rcpstatus;
            if (scme->retry--)
            {
                reissue_read(scio1);
                continue;
            }

            /* Retry count expired */

            scio1->scr2->status = SCR1ST_SERR;
            scme->pstate = PS_RDFAIL;
            QU_EnqueReqILT(ilt, (void *)&cm_wc_qu);
            continue;

        case PS_ABREAD:
            ++cm_rcpabort;          /* Count read aborts */
            if (scme->retry)
            {
                reissue_read(scio1);
                continue;
            }
            break;

        default:                    /* Should not happen */
            ++cm_rcpstate;          /* Count impossible event */
            scio1->scr2->status = SCR1ST_LLFUERR;
            scme->pstate = PS_RDFAIL;
            QU_EnqueReqILT(ilt, (void *)&cm_wc_qu);
            continue;
        }
    }
}


/**
******************************************************************************
**
**  @brief  Remove a scmt from the active scme list in the CM table if present
**
**  Checks if specified scmt record on active scmt list in the
**  CM table and if so removes it from the list; otherwise
**  just clears the forward pointer field in the specified scmt
**  record and returns to caller.
**
**  @param  scmt    - SCMT address of SCMT to remove from queue
**  @param  cm      - CM address
**
**  @return none
**
******************************************************************************
**/
static void remscmt(SCMT *scmt, CM *cm)
{
    SCMT    *next;
    SCMT    *prev = NULL;

    /* crash - JIRA SAN-1772 - 2009-10-08 -- BE CM structure used after freed eaeaeaea. */
    for (next = cm->scmts; next; prev = next, next = next->link)
    {
        if (next == scmt)   /* If found SCMT to remove */
        {
            if (prev)
            {
                prev->link = next->link;
            }
            else
            {
                cm->scmts = next->link;
            }
            break;
        }
    }

    scmt->link = NULL;
}


/**
******************************************************************************
**
**  @brief  Removes a scme record from the active scme list if present.
**
**  Checks if specified scme record on active scme list and if
**  so removes it from the list; otherwise just clears the
**  forward pointer field in the specified scme record and
**  returns to caller.
**
**  @param  scme    - scme record address to remove
**  @param  scmt    - SCMT
**
**  @return none
**
******************************************************************************
**/
static void remscme(SCMTE *scme, SCMT *scmt)
{
    SCMTE   *next;
    SCMTE   *prev = NULL;

    for (next = scmt->headscme; next; prev = next, next = next->link)
    {
        if (next == scme)
        {
            if (prev)
            {
                prev->link = scme->link;
            }
            else
            {
                scmt->headscme = scme->link;
            }
            break;
        }
    }
    if (!next)
    {
        fprintf(stderr, "%s%s COPY freeing not found SCMTE? %p\n",
            FEBEMESSAGE, __func__, scme);
    }
    put_scmte(scme);
}


/**
******************************************************************************
**
**  @brief  Complete write op
**
**  @param  scio1       - Pointer to SCIO1
**
**  @return none
**
******************************************************************************
**/
static void complete_write_op(SCIO1 *scio1)
{
    CM      *cm = scio1->cm;
    SCMT    *scmt = scio1->scmt;
    VRP     *vrp = scio1->vrp;
    SCMTE   *scme = scio1->scme;
    SGL     *sgl;
    SCRP1   *scr1 = scio1->scr1;

    sgl = vrp->pSGL;

    if ((UINT32)sgl == 0xfeedf00d)
    {
        fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
        abort();
    }

    if (sgl)
    {
        vrp->pSGL = NULL;
        vrp->sglSize = 0;
        PM_RelSGLWithBuf(sgl);
    }

#ifdef  M4_DEBUG_SCIO
    fprintf(stderr, "%s: %p\n", __func__, scio1);
#endif /* M4_DEBUG_SCIO */
    put_scio(scio1);

    remscme(scme, scmt);

    /* Check if SCMTE chain for this SCMT is complete */

    if (scmt->headscme)         /* If more to process */
    {
        return;                 /* Return to go process them */
    }

    /* Copy of the requested segment(s) are complete */

    SCRP2   *scr2 = (SCRP2 *)((ILT *)scr1 + 1);
    void    *cr = scr1->cr;
    UINT8   status = scr2->status;  /* Load up completion status */

    /*
     * Check the SCR priority and decrement the execution count
     * field in the appropriate queue
     */

    switch (scr1->priority)
    {
    case CMP_LOW:
        --cm_exec_qpt_low.cnt;
        break;

    case CMP_NORM:
        --cm_exec_qpt_norm.cnt;
        break;

    default:
        --cm_exec_qpt_high.cnt;
    }

    /* Save the completion status and mark SCR done */

    scr1->status = status;
    scr1->done = TRUE;

    /* Remove registry of SCMT from CM and release SCMT to free pool */

    remscmt(scmt, cm);                  /* Remove from CM chain */
#ifdef  M4_DEBUG_SCMT
    fprintf(stderr, "%s: put_scmt %p, status=%d\n", __func__, scmt, status);
#endif /* M4_DEBUG_SCMT */
    put_scmt(scmt);

    /* If completion status is ok, mark segment(s) complete in RM */

    if (status == SCR1ST_OK)
    {
        clr_segment_bit(scr1->segnum, cm->cor);

        /* Bump the count of segments in the apool if dest VDD is an alink */

        VDD     *vdd = cm->cor->destvdd;

        if (vdd && BIT_TEST(vdd->attr, VD_BASYNCH))
        {
            ++cur_ap_data;
        }
    }

    /* Send SCR to completion handler */

    complete_scr(cr, scr1);
}


/**
******************************************************************************
**
**  @brief  Process write completion from the destination VDisk
**
**  Checks if in correct process state, checks for errors on
**  write completion, updates CM if successful, checks for
**  raid RD completion and if so initializes the next raid RD,
**  starts next copy sequence.
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
NORETURN void cm_wr_comp(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED)
{
    ILT     *ilt;
    SCIO1   *scio1;
    VRP     *vrp;
    SCMTE   *scme;

    for (;;)
    {
        TaskSwitch();           /* Exchange processes */

        ilt = QU_DequeReqILT(&cm_wc_qu);    /* Get next queued request */
        if (!ilt)
        {
            TaskSetMyState(PCB_NOT_READY);
            continue;
        }

        scio1 = (SCIO1 *)ilt;
        vrp = scio1->vrp;
        scme = scio1->scme;

        switch (scme->pstate)       /* Check state of completion */
        {
        case PS_WRITE:
            if (vrp->status == EC_OK)
            {
                complete_write_op(scio1);
                continue;
            }
            ++cm_wcpstatus;
            if (!scme->retry)       /* If retry count exhausted */
            {
                scio1->scr2->status = SCR1ST_DERR;
                complete_write_op(scio1);
                continue;
            }
            --scme->retry;
            break;

        case PS_ABWRITE:
            ++cm_wcpabort;
            if (!scme->retry)       /* If retry count exhausted */
            {
                scio1->scr2->status = SCR1ST_DERR;
                complete_write_op(scio1);
                continue;
            }
            break;

        case PS_RDFAIL:
            complete_write_op(scio1);
            continue;

        default:
            ++cm_wcpstate;
            scio1->scr2->status = SCR1ST_LLFUERR;
            scme->pstate = PS_DONE;
            QU_EnqueReqILT(ilt, (void *)&cm_wc_qu);
            continue;
        }

        COR     *cor = scio1->cm->cor;

        scme->pstate = PS_READ;
        vrp->function = VRP_COPYINPUT;
        vrp->vid = COR_GetRSVID(cor);
        vrp->startDiskAddr = scme->vdsda;
        vrp->length = scme->numsec;
        vrp->gen2 = 0;
        EnqueueILT((void *)V_que, ilt, (void *)&ct_CM$rc_que);
    }
}


/**
******************************************************************************
**
**  @brief  Queue the SCIO associated with a SCMT to virtual.
**
**  The SCMTE chain is used to queue their associated SCIO (ILT) and
**  VRP to the virtual layer.
**
**  Note: This routine assumes that there is at least 1 SCMTE
**        associated with the SCMT.
**
**  @param  scmt    - Pointer to SCMT
**
******************************************************************************
**/
static void submit_scmt(SCMT *scmt)
{
    SCMTE   *scme;

    for (scme = scmt->headscme; scme; scme = scme->link)
    {
        scme->pstate = PS_READ;     /* Set process state to read */

        EnqueueILT((void *)V_que, scme->ilt, (void *)&ct_CM$rc_que);
    }
}


/**
******************************************************************************
**
**  @brief  Complete copy request
**
**  @param  scr     - Pointer to SCR1P structure
**  @param  status  - Final status of request
**
**  @return none
**
******************************************************************************
**/
static void copy_comp(SCRP1 *scr, UINT32 stat)
{
    ILT     *ilt;

    scr->status = stat;
    ilt = (ILT *)scr;
    ++ilt;
    K_comp(ilt);
}


/**
******************************************************************************
**
**  @brief  Process copy manager executive queues
**
**  @param  none
**
**  @return New process state or zero (PCB_READ) if no change
**
******************************************************************************
**/
static UINT8 process_cm_queues(void)
{
    int     queix;
    UINT8   stat;

    stat = PCB_NOT_READY;   /* Default to a not ready sleep */
    for (queix = 0; queix < qpt_num; ++queix)
    {
        QU      *que;
        SCRP1   *scrp1;

        que = cm_exec_qpt[queix].que;
        if (QU_IsExecQEmpty(que))
        {
            continue;
        }

        if (cm_exec_qpt[queix].cnt >= cm_exec_qpt[queix].limit)
        {
            stat = PCB_IPC_WAIT;
            continue;
        }

        scrp1 = (SCRP1 *)que->head;
        if (pool_scmt.num_free < 1)         /* If not enough SCMTs */
        {
            return PCB_IPC_WAIT;
        }

        scrp1 = (SCRP1 *)QU_DequeReqILT(que);
        ++cm_exec_qpt[queix].cnt;

#if DEBUG
        if (!scrp1->srcvdd || !scrp1->dstvdd || !scrp1->cr || !scrp1->cm ||
                !scrp1->pcb)
        {
            copy_comp(scrp1, EC_INV_FUNC);
            return PCB_READY;
        }

        if (seg2vda(scrp1->segnum) > scrp1->srcvdd->devCap)
        {
            copy_comp(scrp1, EC_INV_SDA);
            return PCB_READY;
        }
#endif /* DEBUG */

        submit_scmt(cm_build_scmt(scrp1));
        return PCB_READY;
    }

    return stat;
}


/**
******************************************************************************
**
**  @brief  Process SCR requests which have been queued to this module.
**
**  The queuing routine CM$que deposits a SCR request into the queue
**  and activates this executive if necessary. This executive
**  extracts the next SCR request from the queue and initiates that
**  request by queuing VRP requests to the VIRTUAL module.
**
**  A separate completion routine handles the completion of VRP requests.
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
NORETURN void cm_exec(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED)
{
    for (;;)
    {
        UINT8   stat;

        TaskSwitch();
        stat = process_cm_queues();
        if (stat != PCB_READY)
        {
            TaskSetMyState(stat);
        }
    }
}


/**
******************************************************************************
**
**  @brief  Allocate an ILT/VRP combination for copies
**
**  @param  none
**
**  @return Pointer to allocated ILT (which also points to VRP)
**
******************************************************************************
**/
static ILT  *cm_aivw(void)
{
    ILT     *ilt;
    VRP     *vrp;

    ilt = get_ilt();
    vrp = get_vrp();
    ilt->ilt_normal.w0 = (UINT32)vrp;
    vrp->gen2 = 0;

    return ilt;
}


/**
******************************************************************************
**
**  @brief  Build a secondary ILT/VRP for a write to a copy destination
**
**  This routine allocates an ILT/VRP/SN to be used to issue a write
**  operation to a destination copy device.
**
**  @param  scd     - Pointer to SCD/DCD
**  @param  func    - VRP function/strategy
**  @param  privrp  - Pointer to primary VRP
**  @param  priilt  - Pointer to primary ILT
**
**  @return Secondary ILT pointer or NULL if none created
**
******************************************************************************
**/
ILT *build_sec_vrp(SCD *scd, UINT32 func, VRP *privrp, ILT *priilt)
{
    COR     *cor = scd->cor;
    DCD     *dcd;

    if (!cor)
    {
        return NULL;
    }
    if (scd->type >= DCDT_LOCAL)    /* Is it really a DCD? */
    {
        dcd = (DCD *)scd;
    }
    else
    {
        dcd = cor->dcd;
        if (!dcd)
        {
            return NULL;
        }
    }

    UINT16  dvid;

    if (dcd->type == DCDT_REMOTE)
    {
        dvid = (cor->rdcl << 8) | cor->rdvd;
    }
    else
    {
        dvid = (cor->rcdcl << 8) | cor->rcdvd;
    }

    ++cor->uops;
    BIT_SET(func, VRP_SPECIAL);

    ILT     *secilt = cm_aivw();
    VRP     *secvrp = (VRP *)secilt->ilt_normal.w0;

    secvrp->function = func & 0xFFFF;
    secvrp->strategy = func >> 16;
    secvrp->status = 0;
    secvrp->vid = dvid;
    secvrp->length = privrp->length;
    secvrp->startDiskAddr = privrp->startDiskAddr;

    SGL *secsgl = m_gensgl(privrp->length, 0, privrp->pSGL);

    secvrp->pSGL = secsgl;
    secvrp->sglSize = secsgl->size;

    secilt->cr = (void *)&ct_cm$up1comp;
    secilt->ilt_normal.w3 = (UINT32)priilt;
    secilt->ilt_normal.w4 = (UINT32)dcd->vdd;
    secilt->ilt_normal.w5 = (UINT32)cor;

    return secilt;
}


/**
******************************************************************************
**
**  @brief  Write phase 2 copy handler
**
**  @param  scd     - Pointer to SCD/DCD
**  @param  func    - VRP function/strategy
**  @param  privrp  - Pointer to primary VRP
**  @param  priilt  - Pointer to primary ILT
**
******************************************************************************
**/
ILT *cm_wp2_copy(SCD *scd, UINT32 func, VRP *privrp, ILT *priilt)
{
    COR     *cor = scd->cor;
    bool    copy_to_dest = FALSE;
    UINT32  num_segs;
    UINT32  seg;
    UINT32  numsec;
    UINT64  devCap;

    if (!cor)
    {
        return NULL;
    }

    numsec = privrp->length;
    /* seg is a maximum of 22 bits */
    /* NOTE: use copy manager cal_seg_bit() format. */

    if (cor->destvdd != 0)          /* Destination and Source size much match. */
    {
        devCap = cor->destvdd->devCap;
    }
    else
    {
        if (cor->srcvdd == 0)       /* If no destination, use source size. */
        {
            return NULL;
        }
        devCap = cor->srcvdd->devCap;
    }


    num_segs = cal_seg_bit(&seg, privrp->startDiskAddr, privrp->startDiskAddr + numsec - 1, devCap, 0);

    for (; num_segs > 0; ++seg, --num_segs)
    {
        if (!chk_segment_bit(seg, cor))
        {
            copy_to_dest = TRUE;
            continue;
        }

        /*
         * Segment has not been copied. Check to see if it is currently
         * being copied.
         */

        CM      *cm = cor->cm;

        if (!cm || !cm->scmts)
        {
            continue;
        }

        SCMT    *scmt;

        for (scmt = cm->scmts; scmt; scmt = scmt->link)
        {
            if (seg != scmt->segnum)
            {
                continue;
            }

            copy_to_dest = TRUE;

            SCMTE   *scme;

            for (scme = scmt->headscme; scme; scme = scme->link)
            {
                if (privrp->startDiskAddr > scme->vdeda)
                {
                    continue;
                }
                if (privrp->startDiskAddr + numsec - 1 < scme->vdsda)
                {
                    continue;
                }

                switch (scme->pstate)
                {
                case PS_READ:
                    scme->pstate = PS_ABREAD;
                    break;

                case PS_WRITE:
                    scme->pstate = PS_ABWRITE;
                    break;

                default:
                    break;
                }
            }
        }
    }

    if (copy_to_dest)
    {
        return build_sec_vrp(scd, func, privrp, priilt);
    }

    return NULL;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
