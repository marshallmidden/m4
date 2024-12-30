/* $Id: ss_comp.c 159966 2012-10-01 23:20:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       ss_comp.c
**
**  @brief      Snapshot code converted from assembly
**
**  Provides C implementations of snapshot completion routines.
**
**  Copyright (c) 2008-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "ss_comp.h"
#include "ssms.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "ilt.h"
#include "QU_Library.h"
#include <stdio.h>
#include <string.h>
#include "CT_defines.h"
#include "ecodes.h"
#include "virtual.h"
#include "qu.h"
#include "ss_nv.h"
#include "ss_nv_funcs.h"
#include "pm.h"
#include "LOG_Defs.h"
#include "vdd.h"
#include "defbe.h"
#include "misc.h"
#include "nvram.h"
#include "cm.h"
#include "ss_comp.h"
#include "mem_pool.h"           /* Needed for put_sm() */
#include "ddr.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/

#define NUMBER_SS_WRITE_COMPLETORS  19
#define ISE_SPECIAL FALSE

QU SS_comp_worker_queues[NUMBER_SS_WRITE_COMPLETORS];
/* TRB...move this into common include later...it is also in apool.h */
extern void logSPOOLevent(UINT8 event_code, UINT8 severity,UINT32 errorCode,UINT32 value1,UINT32 value2);
extern void ss_invalidate_snapshot(UINT16 ss_vid);
extern void K_comp(struct ILT *);

/*
******************************************************************************
** Public variables
******************************************************************************
*/
UINT32  cow_orc;
static UINT32 ss_errcomp_write_count_time = 0;      /* 1/8th second timer to allow messages again. */
static UINT32 ss_errcomp_buffer_overflow_time = 0;  /* 1/8th second timer to allow messages again. */

/*
******************************************************************************
** Public variables - defined in other files
******************************************************************************
*/
extern UINT8    d_bitcnt[256];
extern OGER     *gnv_oger_array[2];

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
UINT8   is_access_to_ss(ILT *ilt);
static void ss_src_acc_proc(ILT *ilt);
static void ss_wcomp_HandleIseBusy(ILT *ilt);
extern struct OGER *htsearch(UINT32 *slot, UINT32 seg, struct SSMS *);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*
******************************************************************************
**
**  @brief  Create the snapshot completion routines.
**
**  @param  none.
**
**  @return none.
**
******************************************************************************
*/

void create_snapshot_completers(void)
{
    int i;
    char buf[25];

    for (i = 0; i < NUMBER_SS_WRITE_COMPLETORS; i++)
    {
        CT_fork_tmp = (ulong)&buf[0];
        snprintf(buf, 25, "ss$comp_worker_%02d", i);
        /*
         * Do not need to set saved pcb to -1 for flagging that it was started
         * as this is one time startup code.
         */
        SS_comp_worker_queues[i].pcb = TaskCreatePerm3(C_label_referenced_in_i960asm(ss$comp_worker), SS_WRITE_COMPLETION_PRIORITY, i);
    }
}   /* End of create_snapshot_completers */


/*
******************************************************************************
**
**  @brief  Queue the ILT onto queue and start task.
**
**  @param  ilt     - pointer to ILT to put onto queue.
**  @param  queue   - pointer to QU.
**
**  @return none.
**
**  @attention  This replaces i960 routine K$cque.
**
**  @example:   # Note: g1 = ilt
**                      lda     P_comp_qu,r11
**                      b       K$cque
**
**   becomes:
**              c       q2_completer_task(g1, (QU *)&P_comp_qu);
**                      ret
**
******************************************************************************
*/

void q2_completer_task(ILT *ilt, QU *queue)
{
    ILT *prev_tail = queue->tail;

    if (prev_tail == NULL)        /* If nothing on queue. */
    {
        queue->head = ilt;          /* Head points to this new ILT. */
/* NOTE: that head & tail for ILT are in same place as QU structure. *WHIMPER* */
        prev_tail = (ILT *)queue;
        if (queue->pcb != NULL)     /* If process exists. */
        {
            if (TaskGetState(queue->pcb) == PCB_NOT_READY)  /* If process not ready. */
            {
#ifdef HISTORY_KEEP
                CT_history_pcb("q2_completer_task setting ready pcb", (UINT32)queue->pcb);
#endif  /* HISTORY_KEEP */
                TaskSetState(queue->pcb, PCB_READY);        /* Make process ready. */
            }
        }
    }
    else                            /* else something is on queue. */
    {
        queue->tail->fthd = ilt;    /* Last tail points to this new ILT. */
    }
    queue->qcnt++;                  /* Increment number of entries on queue. */
    queue->tail = ilt;              /* Tail of queue is to this new ILT. */
    ilt->fthd = NULL;               /* Last entry has no forward pointer. */
    ilt->bthd = prev_tail;          /* Last entry has no backward pointer. */
}   /* End of q2_completer_task */


/*
******************************************************************************
**
**  @brief  Find shortest snapshot completion queue and insert ILT onto it.
**
**  @param  ilt     - pointer to ILT to put onto queue.
**
**  @return none.
**
******************************************************************************
*/

void q2_snapshot_completer(UINT32 dummy0 UNUSED, ILT *ilt)
{
    int     i;
    SSMS    *ssms;

    /*
     * This algorithm will keep snapshot COW's for the same vdisk with the same task.
     */
    ssms = (SSMS *)ilt->ilt_normal.w6;

    /*
     * Do a mod to get the remainder -- and send to that completer task queue.
     */
    i = ssms->ssm_ordinal % NUMBER_SS_WRITE_COMPLETORS;
    q2_completer_task(ilt, &SS_comp_worker_queues[i]);
}   /* End of queue_ilt_onto_snapshow_queue */


/*
******************************************************************************
**
**  @brief  Provides a task-switchable way for snapshot write completions.
**
**  @param  i   - Which queue index we are to run with.
**
**  @return none.
**
**  @attention  This is a task.
**
******************************************************************************
*/

NORETURN void ss$comp_worker(UINT32 dummy0 UNUSED, UINT32 dummy1 UNUSED, int i)
{
    ILT *ilt;

    while (1)
    {
        /*
         * If nothing to do, set task to not ready, i.e. waiting for something to do.
         */
        if (SS_comp_worker_queues[i].head == NULL)
        {
            TaskSetMyState(PCB_NOT_READY);
            TaskSwitch();
        }

        /*
         * See if something to do.
         */
        ilt = SS_comp_worker_queues[i].head;
        if (ilt != NULL)
        {
            SS_comp_worker_queues[i].qcnt--;
            SS_comp_worker_queues[i].head = ilt->fthd;
            /*
             * See if nothing more on queue.
             */
            if (SS_comp_worker_queues[i].tail == ilt)
            {
                SS_comp_worker_queues[i].tail = NULL;
            }

            if (ilt->ilt_normal.w7 == (UINT32)ss_rcomp)
            {
                ss_rcomp(ilt);
            }
            else
            {
                ss_wcomp(ilt);
            }
            /*
             * There is nothing else to do, this is the top level of ILT.
             */
        }
    }
}   /* End of ss$comp_worker */


/*
******************************************************************************
**
**  @brief  Remove an SSMS from a source VDD
**
**  @param  ssms - Pointer to SSMS to remove
**  @param  src_vdd - Pointer to source VDD
**
**  @return TRUE if found and removed, FALSE if not found
**
******************************************************************************
*/

int remove_ssms_from_src_vdd(SSMS *ssms, VDD *src_vdd)
{
    SSMS    *prev_ssms;
    SSMS    *next_ssms;

    if(!src_vdd)
    {
        // Source VDD doesn't exist
        return(FALSE);
    }
    if(!ssms)
    {
        // SSMS doesn't exist
        return(FALSE);
    }

    next_ssms = src_vdd->vd_outssms;
    prev_ssms = NULL;
    while (next_ssms && next_ssms != ssms)
    {
        prev_ssms = next_ssms;
        next_ssms = next_ssms->ssm_link;
    }

    if (!next_ssms)         /* If ssms not found */
    {
        return FALSE;       /* Indicate ssms not found */
    }

    if (!prev_ssms)         /* If first in list */
    {
        src_vdd->vd_outssms = ssms->ssm_link;
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    }
    else
    {
        prev_ssms->ssm_link = ssms->ssm_link;
    }

    if (!ssms->ssm_link)    /* If last in list */
    {
        src_vdd->vd_outssmstail = prev_ssms;
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    }

    ssms->ssm_link = 0;     /* Clear any link remaining in the SSMS */

    return TRUE;
}   /* End of remove_ssms_from_src_vdd */


/*
******************************************************************************
**
**  @brief  Clears a bit in a segment map.
**
**          This routine will clear a bit in a segment map for a SSMS.
**
**  @param  seg_num - Segment bit we attempted to clear.
**  @param  ssms    - Pointer to SSMS.
**  @param  ret     - Pointer to SSMS.
**   ret    0 - if bit was cleared and everything is ok.
**   ret    1 - No region map (NULL).
**   ret    2 - Region map section NULL.
**   ret    3 - Bit was already cleared.
**   ret    4 - Bit cleared, but region count was already zero.
**   ret    0,3,4 | 0x100 - Above for lower values, but rgn->cnt==0 and bits set!?
**  @param  f       - File containing call to this routine.
**  @param  func    - Function name containing call to this routine.
**  @param  line    - Line of file containing call to this routine.
**
**
******************************************************************************
*/
void print_seg_clear_error(UINT32 seg_num, SSMS *ssms, int ret,
                           const char *f, const char *func, unsigned int line)
{
    int         extra = seg_num >> 21;
    struct RM   *rm = ssms->ssm_regmap[extra];
    UINT32      seg_bit_num = 31 - (seg_num & 0x1F);
    UINT32      reg_word_ix = seg_num >> (SMBITS2WRD_SF + SMWRD2REG_SF);
    UINT32      seg_word_ix = (seg_num >> SMBITS2WRD_SF) & SMWRDIDX_MASK;
    struct SM  *rgn = NULL;

    if (rm != NULL)
    {
        rgn = rm->regions[reg_word_ix];
    }

    const char *str1;
    switch (ret & ~0x100)
    {
      case 0:
        str1 = "no error";      /* No error. */
        break;
      case 1:
        str1 = "no region map in ssms";
        break;
      case 2:
        str1 = "ssms->ssm_regmap[extra]->regions[reg_word_ix] is null";
        break;
      case 3:
        str1 = "bit not set, already clear";
        break;
      case 4:
        str1 = "bit cleared, but region count already zero";
        break;
      default:
        str1 = "UNKNOWN ERROR";
        break;
    }

    const char *str2;
    if ((ret & ~0x100) == 0)
    {
        str2 = NULL;
    }
    else
    {
        str2 = " and region count 0, but bits are still set";
    }
    fprintf(stderr, "%s:%s:%d ss_clr_seg_bit returned(0x%x) %s%s, "
                    "ssms=%p seg_num=%d reg_word_ix=%d seg_word_ix=%d bit=%d rgn=%p\n",
                    f, func, line, ret, str1, str2,
                    ssms, seg_num, reg_word_ix, seg_word_ix, seg_bit_num, rgn);
}   /* End of print_seg_clear_error */


/*
******************************************************************************
**
**  @brief  Clears a bit in a segment map.
**
**          This routine will clear a bit in a segment map for a SSMS.
**
**  @param  seg_num - Segment bit to clear
**  @param  ssms - Pointer to SSMS
**
**  @return 0 - if bit was cleared and everything is ok.
**  @return 1 - No region map (NULL).
**  @return 2 - Region map section NULL.
**  @return 3 - Bit was already cleared.
**  @return 4 - Bit cleared, but region count was already zero.
**  @return 0,3,4 | 0x100 - Above for lower values, but rgn->cnt==0 and bits set!?
**
******************************************************************************
*/
int ss_clr_seg_bit(UINT32 seg_num, SSMS *ssms)
{
    int         bit_cleared;
    UINT32      seg_bit_num;
    UINT32      seg_word_ix;
    UINT32      reg_word_ix;
    struct RM   *rm;
    struct SM   *rgn;
    int         extra = seg_num >> 21;

    rm = ssms->ssm_regmap[extra];
    if (!rm)
    {
        return (1);         /* Do nothing if no region map */
    }

    seg_bit_num = 31 - (seg_num & 0x1F);
    reg_word_ix = seg_num >> (SMBITS2WRD_SF + SMWRD2REG_SF);
    seg_word_ix = (seg_num >> SMBITS2WRD_SF) & SMWRDIDX_MASK;
    rgn = rm->regions[reg_word_ix];
    if (!rgn)
    {
        return (2);         /* Return that rgn is null */
    }

    /* The segment map exists. Get the appropriate segment word and clear bit. */
    if (!BIT_TEST(rgn->segments[seg_word_ix], seg_bit_num))
    {
        bit_cleared = 3;    /* Return that bit was already cleared */
    }
    else
    {
        BIT_CLEAR(rgn->segments[seg_word_ix], seg_bit_num);

        /* Decrement remaining segment count */
        if (rgn->cnt)
        {
            --rgn->cnt;
            bit_cleared = 0;    /* Return that everything is ok. */
        }
        else
        {
            bit_cleared = 4;    /* Return that rgn->cnt was already zero. */
        }
    }

    int     ix;

    for (ix = 0; ix < (SMTBLSIZE / 4); ++ix)
    {
        if (rgn->segments[ix])
        {
            if (!rgn->cnt)
            {
                bit_cleared |= 0x100;   /* Return that rgn->cnt zero, but bits set. */
            }
            return (bit_cleared);       /* Return that bits are still set. */
        }
    }

    /* The segment map is clear, deallocate the segment map */
    p_Free(rgn, sizeof(SM), __FILE__, __LINE__);
    rm->regions[reg_word_ix] = NULL;

    return (bit_cleared);               /* Return previously determined exit value. */
}   /* End of ss_clr_seg_bit */


/*
******************************************************************************
**
**  @brief  Returns available segment count
**
**  @param  seg_num - Segment bit to clear
**  @param  ssms - Pointer to SSMS
**
**  @return available segment count
**
******************************************************************************
*/
static UINT32 ss_available_segs(SSMS *ssms)
{
    UINT32  segs_available = 0;
    struct RM   *rm;
    int     i;
    int     j;

    for (j = 0; j < 32; j++)
    {
        rm = ssms->ssm_regmap[j];
        if (rm)
        {
            for (i = 0; i < MAXRMCNT; i++)
            {
                if (rm->regions[i])
                {
                    segs_available += rm->regions[i]->cnt;
                }
            }
        }
    }

    return segs_available;
}   /* End of ss_available_segs */


/*
******************************************************************************
**
**  @brief  Remove a group of sync records linked to an SSMS
**
**          Given a pointer to a master sync record and an SSMS, this
**          routine removes all sync records in the masters group from
**          the doubly-linked list contained in the SSMS.
**
**  @param  ssms - Pointer to SSMS
**  @param  master_sync - Pointer to master sync record
**
**  @return none
**
******************************************************************************
*/
static void ss_rem_group(SSMS *ssms, SYNC *master_sync)
{
    SYNC    *prev_sync;
    SYNC    *cur_sync;

    cur_sync = master_sync;
    prev_sync = master_sync->syn_prev;

    /* Find all of the members of this sync group */

    while ((cur_sync = cur_sync->syn_link) != NULL)
    {
        if (cur_sync->syn_master != master_sync)
        {
            break;
        }
    }

    /*
     * There are 4 cases to support.
     * Case 1 Master is at the head and more follow
     * Case 2 Master is head but none follow
     * Case 3 Master is at tail but has preceding records
     * Case 4 Master is nested between records
     */
    if (prev_sync == NULL)
    {
        /* Master is at the head */
        ssms->ssm_synchead = cur_sync;
        if (cur_sync == NULL)
        {
            ssms->ssm_synctail = NULL;
        }
        else
        {
            cur_sync->syn_prev = NULL;
        }
    }
    else
    {
        /* Master is not at the head */
        prev_sync->syn_link = cur_sync;
        if (cur_sync == NULL)
        {
            ssms->ssm_synctail = prev_sync;
        }
        else
        {
            cur_sync->syn_prev = prev_sync;
        }
    }
}   /* End of ss_rem_group */


/*
******************************************************************************
**
**  @brief  Compute snapshot sparseness
**
**  @param  ssms - Pointer to ssms
**
**  @return None
**
******************************************************************************
*/
void compute_ss_spareseness(SSMS *ssms)
{
    UINT32  segs_available;
    VDD     *vdd;
    UINT8   percent;

    segs_available = ss_available_segs(ssms);
    vdd = V_vddindx[ssms->ssm_ssvid];
    if (!vdd)
    {
        return;
    }

    percent = (UINT8)(segs_available * 100 / (vdd->devCap >> SECPOWER));
    vdd->scpComp = percent;
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
}   /* End of compute_ss_spareseness */


/*
******************************************************************************
**
**  @brief  Clean up inter-SSMS dependency record
**
**  @param  ssms_ic - Pointer to inter-SSMS dependency record
**
**  @return none
**
******************************************************************************
*/
void clean_up_ssms_ic(ssmdep *ssms_ic)
{
    int     segix;

    for (segix = 0; segix < MAX_SEG_ACCESS; ++segix)
    {
        if (ssms_ic->ssmdep_cow_seg[segix].sgl)
        {
            PM_RelSGLWithBuf(ssms_ic->ssmdep_cow_seg[segix].sgl);
            ssms_ic->ssmdep_cow_seg[segix].sgl = NULL;
            if (--cow_orc > 10000)
            {
                fprintf(stderr, "%s: cow_orc underflow %08x\n",
                    __func__, cow_orc);
            }
        }
    }
    p_Free(ssms_ic, sizeof(*ssms_ic), __FILE__, __LINE__);
}   /* End of clean_up_ssms_ic */


/*
******************************************************************************
**
**  @brief  Remove and free sync records in group
**
**  @param  ssms - Pointer to SSMS
**  @param  master - Pointer to sync record
**  @param  cow_sda - Snappool SDA of the COW
**
**  @return Pointer to ssmdep record
**
******************************************************************************
*/
static ssmdep   *free_syncs_in_group(SSMS *ssms, SYNC *master)
{
    SYNC        *syn;
    ssmdep      *iscnt;

    ss_rem_group(ssms, master);

    /* Free each of the sync records in this group */

    syn = master;
    iscnt = syn->syn_iscnt;
    do
    {
        SYNC    *next_sync;

        if (iscnt != syn->syn_iscnt)
        {
            fprintf(stderr, "%s: Varying syn_iscnt values in group\n",
                __func__);
        }
        iscnt = syn->syn_iscnt;  /* ?? Weird to set this in loop?? */
        next_sync = syn->syn_link;
        p_Free(syn, sizeof(*syn), __FILE__, __LINE__);
        syn = next_sync;
    }
    while (syn && syn->syn_master == master);

    return iscnt;
}   /* End of free_syncs_in_group */


/*
******************************************************************************
**
**  @brief  Complete dependent sync records
**
**  @param  ssms - Pointer to SSMS
**  @param  syn - Pointer to sync record
**  @param  cow_sda - Snappool SDA of the COW
**
**  @return none
**
******************************************************************************
*/
static void complete_dependent_syncs(SSMS *ssms, SYNC *syn, UINT64 cow_sda)
{
    ILT     *dep_ilt = syn->syn_firstilt;
    SYNC    *master_sync = syn->syn_master;
    ILT     *orig_ilt = master_sync->syn_firstilt;

    /* Set the bit in the master sync record to indicate this one is complete */
    BIT_SET(master_sync->syn_map, syn->syn_thcount);

    /*
     * If the dep_ilt pointer has a magic number in it then it must be a
     * split ILT and needs to be submitted now even though the sync
     * group may not be complete.
     */

    if (dep_ilt && dep_ilt->ilt_normal.w2 == SS_MAGIC)
    {
        VRP     *split_vrp = (VRP *)dep_ilt->ilt_normal.w0;

        /* Adjust the SDA for accessing the segment */
        split_vrp->startDiskAddr = (split_vrp->startDiskAddr % SEGSIZE_SEC) + cow_sda;

        /* Find the vid of the owning snappool */
        split_vrp->vid = gnv_oger_array[ssms->ssm_prefowner]->ogr_vid;

        /* Set the special processing bit */
        BIT_SET(split_vrp->function, VRP_SNAPSHOT_SPECIAL);

// EnqueueILT requires g0,g1,g2 saved -- this routine only called from ss_wcomp, which saves.
        EnqueueILT((void *)V_que, dep_ilt, (void *)dep_ilt->cr);
    }

    /*
     * If all of the syncs in this group are not complete then return.
     */
    if (d_bitcnt[master_sync->syn_map] != master_sync->syn_count)
    {
        if (dep_ilt && dep_ilt->ilt_normal.w2 == SS_MAGIC)
        {
            dep_ilt->ilt_normal.w2 = 0;            /* Clear magic number */
        }
        return;
    }

    ssmdep      *ssms_ic;

    /*
     * All of the sync records for this group are complete,
     * so remove them from the list and free them.
     */
    ssms_ic = free_syncs_in_group(ssms, master_sync);

    /*
     * If this dep_ilt was a split ilt then return otherwise
     * the dep_ilt tied to the master sync must be submitted.
     */
    if (dep_ilt && dep_ilt->ilt_normal.w2 == SS_MAGIC)
    {
        dep_ilt->ilt_normal.w2 = 0;                /* Clear magic number */
        return;
    }

    /*
     * Check the inter SSMS dependency counter.  If it is now 0 then free it.
     * If it is not 0 then return and don't submit the group ILT.
     */
    if (ssms_ic)
    {
        if (--ssms_ic->ssmdep_cnt != 0)
        {
            return;
        }
        clean_up_ssms_ic(ssms_ic);
    }

    /*
     * This is not a split ILT so it must be submitted since the sync
     * group is now complete.
     */

    VRP     *orig_vrp = (VRP *)orig_ilt->ilt_normal.w0;
    VDD     *vdd = V_vddindx[orig_vrp->vid];

    if (vdd && !vdd->vd_incssms)
    {
        /* This is not a snapshot so set the special processing bit */
        BIT_SET(orig_vrp->function, VRP_SNAPSHOT_SPECIAL);
    }

// EnqueueILT changes g0,g1,g2 -- this routine only called from ss_wcomp, which saves.
    EnqueueILT((void *)V_que, orig_ilt, (void *)orig_ilt->cr);
}   /* End of complete_dependent_syncs */


/*
******************************************************************************
**
**  @brief  Free a snapshot request
**
**          Release and SGL with buffer and the associated ILT and VRP
**
**  @param  ilt - Pointer to ILT
**
**  @return none
**
******************************************************************************
*/
static void free_ss_request(ILT *ilt)
{
    VRP *vrp = (VRP *)ilt->ilt_normal.w0;
    SGL *sgl = vrp->pSGL;

    vrp->pSGL = NULL;
    vrp->sglSize = 0;

    if ((UINT32)sgl == 0xfeedf00d)
    {
        fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
        abort();
    }
    if (sgl)    /* If no sgl, nothing to free */
    {
        // PM_RelSGLWithBuf changes g0.
        PM_RelSGLWithBuf(sgl);
    }

// PM_RelILTVRPSGL changes g0,g1.
    PM_RelILTVRPSGL(ilt, vrp);
}   /* End of free_ss_request */


/*
******************************************************************************
**
**  @brief  Is a ILT/VRP directed at a snapshot.
**
**          If the VDD of the VID has an inc_ssms then it is a snapshot.
**
**  @param  ilt - Pointer to ILT
**
**  @return TRUE/FALSE
**
******************************************************************************
*/
UINT8   is_access_to_ss(ILT *ilt)
{
    VDD    *vdd;

    if (ilt->ilt_normal.w2 == SS_MAGIC)
    {
        // This is a split request it can only be to a snapshot
        return TRUE;
    }

    vdd = V_vddindx[((VRP*)(ilt->ilt_normal.w0))->vid];
    if (vdd && vdd->vd_incssms)
    {
        // This vdisk is a snapshot
        return TRUE;
    }
    return FALSE;
}   /* End of is_access_to_ss */


/*
******************************************************************************
**
**  @brief  Complete an errored request associated with a sync record
**
**          This routine processes the completion of a write or read
**          operation associated with a sync record.
**
**  @param  ilt - Pointer to ILT
**  @param  err - Error code
**
**  @return none
**
******************************************************************************
*/
static void ss_errcomp(ILT *ilt, UINT16 err)
{
    VRP     *vrp;
    SYNC    *syn;
    SYNC    *syn_next;
    SSMS    *ssms;
    UINT32  regsave[15];
    UINT8   io2_snapshot;
    UINT8   status;

    memcpy(&regsave, &g0, sizeof(regsave));     /* Save all g registers. */

    vrp = (VRP *)ilt->ilt_normal.w0;           /* Get VRP from ILT */
    syn = (SYNC *)ilt->ilt_normal.w5;          /* Get the SYNC from ILT */
    ssms = (SSMS *)ilt->ilt_normal.w6;         /* Get the SSMS from ILT */

    VDD     *vdd;

    status = vrp->status;           /* Get VRP status */

    if (status == EC_OK)
    {
        status = err;               /* Set some sort of error status */
    }
#ifdef ISE_SPECIAL
    else
    {
        fprintf(stderr, "<ISE-DEBUG> ss_errcmp ECSTATUS --status %x vid=%x svid=%x\n", status,vrp->vid,ssms->ssm_ssvid);
    }
#endif
    syn->syn_status = status;       /* Set sync status to bad also */

    /* Set the status of the snapshot VDisk to inop if IO is to source */
    io2_snapshot = is_access_to_ss(syn->syn_master->syn_firstilt);
    if (!io2_snapshot && status != EC_BEBUSY)
    {
        UINT8   pref_owner;
        UINT16  ss_vid;

        ss_vid = ssms->ssm_ssvid;
        vdd = V_vddindx[ssms->ssm_ssvid];
        pref_owner = ssms->ssm_prefowner;
        if (!vdd)
        {
            goto out;
        }

        ss_invalidate_snapshot(ss_vid);
        remove_ssms_from_src_vdd(ssms, V_vddindx[ssms->ssm_srcvid]);

        /* Log a message to the CCB indicating a problem with the snapshot */
        //TRB: need to change the last 0 to the snappool VID...
        logSPOOLevent(SS_BUFFER_BAD, LOG_AS_ERROR, 2, pref_owner & 1, ss_vid);
    }

// free_ss_request changes g0,g1 -- this routine saves and restores all g registers.
    if (syn->syn_iscnt)
    {
        vrp->pSGL = NULL;       /* Clear SGL, because it is shared */
    }
    free_ss_request(ilt);       /* Release the ILT and SGL */

    /*
     * Update the master sync record to show that this segment is complete.
     * Also follow the list of dependent sync records and complete each
     * as required.
     */

    do
    {
        /*
        ** If the I/O is  to source
        ** If the error is due to PROACTIVEBUSY/ISEUPGRADE/SPINDOWN/, can we treat this segment is complete
        ** as in other cases..In other cases, this is OK, because anyway the snapshot is becoming INOP.
        ** But whereas in our case, we are not making snspshot INOP and are going to use it later. In
        ** such cases we should not treat this segment as complete..
        **
        ** Think, we simply complete the original vrp with BUSY and don't do any other thing.
        ** Even don't try to send the original VRP (source) down, it may get succeed, because the source
        ** might have been stripped across the ISEs which are in operational state. Means on the one hand we
        ** are keeping the snapshot operational, but not writing any thing, but trying to suceed
        ** the original VRP..which leads to data integrity issues.
        ** So don't submit it down, just complete it...
        */

        SYNC    *master;
        ILT     *group_ilt;
        ssmdep  *iscnt;

        syn_next = syn->syn_deplst;
        master = syn->syn_master;
        master->syn_status = status;
        group_ilt = master->syn_firstilt;
        BIT_SET(master->syn_map, syn->syn_thcount);
        if (d_bitcnt[master->syn_map] != master->syn_count)
        {
#if ISE_SPECIAL
            fprintf(stderr, "<ISE-DEBUG> ss_errcmp-not match with master sync count =%x take next sync record\n",\
                    master->syn_count);
#endif
            continue;
        }

        /*
         * All of the sync records in this group are complete,
         * so remove them from the list and free them.
         */

        iscnt = free_syncs_in_group(ssms, master);

        /* Reissue the original VRP */

        if (!is_access_to_ss(group_ilt))
        {
            /*
             * If this group_ilt was a split ilt then return otherwise
             * the group_ilt tied to the master sync must be submitted.
             */
            if (group_ilt && group_ilt->ilt_normal.w2 == SS_MAGIC)
            {
#if ISE_SPECIAL
                fprintf(stderr, "<ISE-DEBUG> ss_errcmp...group ilt is split.. take next sync record\n");
#endif
                group_ilt->ilt_normal.w2 = 0;                /* Clear magic number */
                continue;
            }

            /*
             * Figure out if we have any SSMS (parallel move
             * from source to multi-snaps) dependencies.
             */
            if (iscnt)
            {
                if (--iscnt->ssmdep_cnt != 0)
                {
#if ISE_SPECIAL
                    fprintf(stderr, "<ISE-DEBUG> ss_errcmp..ssms dep count=%x.take next sync record\n",iscnt->ssmdep_cnt);
#endif
                    continue;

                }
#if ISE_SPECIAL
                fprintf(stderr, "<ISE-DEBUG> ss_errcmp...cleanup ssms iscnt=%p..\n",iscnt);
#endif
                clean_up_ssms_ic(iscnt);
            }

            /* If this is an original VRP we need to adjust the nesting level */

            VRP *orig_vrp;

            orig_vrp = (VRP *)group_ilt->ilt_normal.w0;
            vdd = V_vddindx[orig_vrp->vid];
            if (vdd->vd_incssms == 0)
            {
                BIT_SET(orig_vrp->function, VRP_SNAPSHOT_SPECIAL);
            }
            if (status == EC_BEBUSY)
            {
                fprintf(stderr, "<ISE-DEBUG> ss_errcmp completing  original vrp %p- vid=%x with busy\n",\
                        orig_vrp,orig_vrp->vid);
                /*
                ** Since, COR/COW is failed during proactive busy/ISE upgrade condition,
                ** don't proceed with original source write request also. Complete it
                ** to server with BUSY status, so that server will freshly try it again
                */
                orig_vrp->status = EC_BEBUSY;
                K_comp(group_ilt + 1);
            }
            else
            {
#if ISE_SPECIAL
                fprintf(stderr, "<ISE-DEBUG> ss_errcmp enqueing original vrp %p- vid=%x\n", orig_vrp,orig_vrp->vid);
#endif
                // EnqueueILT requires g0,g1,g2 saved.
                EnqueueILT((void *)V_que, group_ilt, (void *)group_ilt->cr);
            }
        }
        else    // Access is to a SS, so complete the original with error
        {
            VRP *vrp_to_error;

            vrp_to_error = (VRP *)group_ilt->ilt_normal.w0;
            if ((status == EC_BEBUSY) || (status == EC_RETRYRC))
            {
                vrp_to_error->status = status;
            }
            else
            {
                vrp_to_error->status = EC_RETRY;
            }
            fprintf(stderr, "<ISE-DEBUG> ss_errcmp access to SS  completing original vrp %p with status=%x - vid=%x\n",\
                    vrp_to_error,vrp_to_error->status, vrp_to_error->vid);
            K_comp(group_ilt + 1);
        }
    }
    while ((syn = syn_next) != NULL);

out:
    memcpy(&g0, &regsave, sizeof(regsave));     /* Restore g registers */
}   /* End of ss_errcomp */


/*
******************************************************************************
**
**  @brief  Complete a write request for a COW.
**
**          This routine is the completion handler for writes to the snapshot
**          issued for a copy on write (COW).
**
**  @param  ilt - Pointer to incoming ILT.
**
**  @return none
**
******************************************************************************
*/
void ss_wcomp(ILT *ilt)
{
    SSMS        *ssms;
    VRP         *vrp;
    SYNC        *syn;
    OGER        *nv_oger;
    int         seg_cleared;
    UINT32      update_status = EC_OK;
    UINT64      cow_sda;
    UINT32      regsave[15];
    VDD         *vdd = NULL;
    memcpy(regsave, &g0, sizeof(regsave));      /* Save all g registers. */

    vrp = (VRP *)ilt->ilt_normal.w0;
    syn = (SYNC *)ilt->ilt_normal.w5;
    ssms = (SSMS *)ilt->ilt_normal.w6;
    cow_sda = vrp->startDiskAddr;   /* Save COW snappool SDA */
    vdd = gVDX.vdd[vrp->vid];

    if (vdd && (BIT_TEST(vdd->attr, VD_BBEBUSY)))
    {
        fprintf(stderr,"<<ISE-DEBUG>>ss_wcomp-- UPgrading..COW failure..handling vrpstatus=%x vdd %x status %x\n",
               vrp->status,vrp->vid, vdd->status);
        ss_wcomp_HandleIseBusy(ilt);
        goto out2;
    }

    if (vrp->status != EC_OK)
    {
        UINT32  retry_count;

        retry_count = XIO_LSW(ilt->misc);

        if (retry_count == 0)
        {
            if (ss_errcomp_write_count_time < K_ii.time)
            {
                fprintf(stderr, "%s%s:%u %s write(COW) vrp status %d to SS vid %d failed (limit messages to 3 minutes)\n",
                        FEBEMESSAGE, __FILE__, __LINE__, __func__, vrp->status, vrp->vid);
                ss_errcomp_write_count_time = K_ii.time + (8 * 60 * 3);  /* limit to one every 3 minutes. */
            }
            ss_errcomp(ilt, EC_IO_ERR);
            goto out2;
        }
        --retry_count;
#ifdef ISE_SPECIAL
        fprintf(stderr, "%s:(%d)--<ISE-DEBUG> write(COW) to SS  vid %x failed, retries remaining=%d\n",
                    __func__,__LINE__, vrp->vid, retry_count);
#endif

        ilt->misc = (ilt->misc & 0xFFFF0000) | XIO_LSW(retry_count);

        /* Requeue the request to Virtual */

        vrp->function = VRP_OUTPUT;
        vrp->status = EC_OK;
        vrp->vid = syn->syn_oger->ogr_vid;
        vrp->length = SEGSIZE_SEC;
// NOTDONEYET SNAPSHOT -- does this (misc) slot being limited to 16 bits cause problem GT2TB
        vrp->startDiskAddr = (XIO_MSW(ilt->misc) << SECPOWER) + syn->syn_oger->ogr_sda;
        EnqueueILT((void *)V_que, ilt, (void *)C_label_referenced_in_i960asm(q2_snapshot_completer));
        goto out2;
    }

    ilt->misc = 0;          /* ?? Should this only clear the retry count?? */
    BIT_SET(syn->syn_state, syn_state_done);    /* Set the state to done */
    if (syn->syn_iscnt)
    {
        vrp->pSGL = NULL;       /* Clear because it is shared */
    }
    else
    {
        if (--cow_orc > 10000)
        {
            fprintf(stderr, "%s: cow_orc underflow %d\n", __func__, cow_orc);
        }
    }
    seg_cleared = ss_clr_seg_bit(syn->syn_segnum, ssms);    /* Clear segment bit */
    if (seg_cleared != 0)
    {
        print_seg_clear_error(syn->syn_segnum, ssms, seg_cleared, __FILE__, __func__, __LINE__);
    }

    /* Update the NV data in the proper order */
    nv_oger = gnv_oger_array[ssms->ssm_prefowner];

    if (BIT_TEST(syn->syn_state, syn_state_new_oger))
    {
        /* A new OGER was allocated so update it and parent */
        update_status = update_oger_nv(syn->syn_oger, nv_oger);
        if (update_status != EC_OK)
        {
            if(update_status == EC_BEBUSY)
            {
                fprintf(stderr,"<ISE-DEBUG>ss_wcomp-1 UPgrading..COW failure..handling\n");
                ss_wcomp_HandleIseBusy(ilt);
            }
            else
            {
                logSPOOLevent(SS_OGER_NV_UPDATE_BAD, LOG_AS_ERROR, 14, 0, 0);
                ss_errcomp(ilt, EC_SNAP_NVERR); /* Complete with error */
            }
            goto out2;
        }

        update_status = update_oger_nv(syn->syn_oger->ogr_parent, nv_oger);
        if (update_status != EC_OK)
        {
            if(update_status == EC_BEBUSY)
            {
                fprintf(stderr,"<ISE-DEBUG>ss_wcomp-2 UPgrading..COW failure..handling\n");
                ss_wcomp_HandleIseBusy(ilt);
            }
            else
            {
                logSPOOLevent(SS_OGER_NV_UPDATE_BAD, LOG_AS_ERROR, 13, 0, 0);
                ss_errcomp(ilt, EC_SNAP_NVERR); /* Complete with error */
            }
            goto out2;
        }

        if (ssms->ssm_prev_tailoger != syn->syn_oger->ogr_parent)
        {
            update_status = update_oger_nv(ssms->ssm_prev_tailoger, nv_oger);
            if (update_status != EC_OK)
            {
                if(update_status == EC_BEBUSY)
                {
                    fprintf(stderr,"<ISE-DEBUG>ss_wcomp-3 UPgrading..COW failure..handling\n");
                    ss_wcomp_HandleIseBusy(ilt);
                }
                else
                {
                    logSPOOLevent(SS_OGER_NV_UPDATE_BAD, LOG_AS_ERROR, 12, 0, 0);
                    ss_errcomp(ilt, EC_SNAP_NVERR); /* Complete with error */
                }
                goto out2;
            }
        }

        update_status = update_ssms_nv(ssms, nv_oger);  /* Update the SSMS */
        if(update_status == EC_BEBUSY)
        {
            fprintf(stderr,"<ISE-DEBUG>ss_wcomp-4 UPgrading..COW failure..handling\n");
            ss_wcomp_HandleIseBusy(ilt);
            goto out2;
        }
        //TBD -- Other error case handling missing originally.(NOT OK and NOT EC_BEBUSY case)...
    }
    else
    {
        /* Update just the OGER that was modified */
        update_status = update_oger_nv(syn->syn_oger, nv_oger);
        if (update_status != EC_OK)
        {
            if(update_status == EC_BEBUSY)
            {
                fprintf(stderr,"<ISE-DEBUG>ss_wcomp-5 UPgrading..COW failure..handling\n");
                ss_wcomp_HandleIseBusy(ilt);
            }
            else
            {
                logSPOOLevent(SS_OGER_NV_UPDATE_BAD, LOG_AS_ERROR, 11, 0, 0);
                ss_errcomp(ilt, EC_SNAP_NVERR); /* Complete with error */
            }
            goto out2;
        }
    }

    if ((seg_cleared & ~0x100) == 0)    /* We cleared a bit, recompute percentage. */
    {
        compute_ss_spareseness(ssms);
    }

    /*
     * Update the Master sync record to show that this segment is complete.
     * Also follow the list of dependent sync records and complete each as
     * required.
     */

    while (syn)
    {
        SYNC    *dependent_sync;

        dependent_sync = syn->syn_deplst;   /* Save next dependent sync */

        complete_dependent_syncs(ssms, syn, cow_sda);

        syn = dependent_sync;           /* Move to next dependent sync */
    }

// free_ss_request changes g0,g1.
    free_ss_request(ilt);                   /* Release the ILT/VRP/SGL */
out2:
    memcpy(&g0, regsave, sizeof(regsave));      /* Restore all g registers. */
}   /* End of ss_wcomp */



static void ss_wcomp_HandleIseBusy(ILT *ilt)
{
    VRP         *vrp;
    SYNC        *syn;

    vrp = (VRP *)ilt->ilt_normal.w0;
    syn = (SYNC *)ilt->ilt_normal.w5;

    /*
    ** make vrp status to explicitly ISEBUSY, since, the upgrade event might have occured
    ** between raid completion and vrp completion processes in which case the rrp status
    ** is not set to busy and ultimately vrp status also. Hence make it busy..
    */
    //if (vrp->status != EC_RETRY)
    vrp->status = EC_BEBUSY;
    ilt->misc = 0;
    if (syn->syn_iscnt)
    {
        vrp->pSGL = NULL;       /* Clear SGL, because it is shared */
    }
    ss_errcomp(ilt, EC_BEBUSY);
}   /* End of ss_wcomp_HandleIseBusy */


/*
******************************************************************************
**
**  @brief  Begin write phase of COW
**
**  @param  ssms - Pointer to SSMS structure
**  @param  syn - Pointer to SYNC structure
**  @param  ilt - The ILT of the request
**
**  @return none
**
******************************************************************************
*/
static void start_cow_write(SSMS *ssms, SYNC *syn, ILT *ilt)
{
    VRP     *vrp;
    OGER    *oger;
    int     new_oger;
    UINT32  slot;

    vrp = (VRP *)ilt->ilt_normal.w0;
    ilt->ilt_normal.w7 = 0;                /* Indicate write completion */

    BIT_SET(syn->syn_state, syn_state_wr);
    vrp->function = VRP_OUTPUT;

    /* Insert the segment into the hash tree */

    oger = htsearch(&slot, syn->syn_segnum, ssms);
    if (!oger)
    {
        oger = htinsert(&slot, &new_oger, syn->syn_segnum, ssms);
        if (!oger)
        {
            ssmdep  *ssms_ic = syn->syn_iscnt;  /* Save for subsequent check */

            /* NOTE: a whole bunch will happen just as the buffer fills, but the snapshots go INOP and stop. */
            if (ss_errcomp_buffer_overflow_time < K_ii.time)
            {
                fprintf(stderr, "%s%s:%u %s Snappool overflowed with vid %d (limit messages to 3 minutes)\n",
                        FEBEMESSAGE, __FILE__, __LINE__, __func__, vrp->vid);
                logSPOOLevent(SS_BUFFER_OVERFLOW, LOG_AS_DEBUG, 20, 0, 0);
                ss_errcomp_buffer_overflow_time = K_ii.time + (8 * 60 * 3);  /* limit to one every 3 minutes. */
            }
            ss_errcomp(ilt, EC_SNAP_FULL);
            if (!ssms_ic)
            {
                if (--cow_orc > 10000)
                {
                    fprintf(stderr, "%s: cow_orc underflow %08x\n",
                        __func__, cow_orc);
                }
            }
            return;
        }

        if (new_oger)
        {
            BIT_SET(syn->syn_state, syn_state_new_oger);
        }
    }

    syn->syn_oger = oger;
    vrp->vid = oger->ogr_vid;
    vrp->length = SEGSIZE_SEC;      /* 1 MB in sectors */
    vrp->startDiskAddr = (slot * SEGSIZE_SEC) + oger->ogr_sda;

    if(is_access_to_ss(syn->syn_firstilt))
    {
        VRP     *orig_vrp;
        orig_vrp = (VRP*)syn->syn_firstilt->ilt_normal.w0;
        orig_vrp->vid = oger->ogr_vid;
        orig_vrp->startDiskAddr = oger->ogr_sda + (slot * SEGSIZE_SEC) + (orig_vrp->startDiskAddr % SEGSIZE_SEC);
    }

    /*
     ** Save this slot number in misc member of ILT, where:
     ** bits 0-15 is the COW retry count
     ** bits 15-31 is the slot number, which will be used to
     ** compute the SDA in the write to snapshot is retried. (max of 1024)
     */
    ilt->misc = (slot << 16) | SNAP_COWRETRY;
// EnqueueILT requires g0,g1,g2 saved.
    EnqueueILT((void *)V_que, ilt, (void *)C_label_referenced_in_i960asm(q2_snapshot_completer));
}   /* End of start_cow_write */


/*
******************************************************************************
**
**  @brief  Get maximum ssmdep completion index
**
**          This routine is needed to determine the maximum index, because
**          ss_errcomp can free the ssmdep structure when the last entry
**          is processed.
**
**  @param  ssms_ic - Pointer to ssmdep structure
**  @param  segix - Segment number
**
**  @return none
**
******************************************************************************
*/

static UINT16   get_max_ssmdep_ix(ssmdep *ssms_ic, UINT32 segix)
{
    ILT         **ilts = ssms_ic->ssmdep_cow_seg[segix].ilts;
    UINT16      ssmdep_max = ssms_ic->ssmdep_max;
    UINT16      max_ix = 0;
    int         ix;

    if (ssmdep_max > dimension_of(ssms_ic->ssmdep_cow_seg[0].ilts))
    {
        fprintf(stderr, "%s: Bad ssmdep_max=%08X\n", __func__, ssmdep_max);
        ssmdep_max = 0;
    }

    for (ix = 0; ix < ssmdep_max; ++ix)
    {
        if (ilts[ix])
        {
            max_ix = ix + 1;
        }
    }

    return max_ix;
}   /* End of get_max_ssmdep_ix */


/*
******************************************************************************
**
**  @brief  Process read completion from snapshot source VDisk
**
**          Checks to see that the read completed normally. If so sets
**          up to issue the write operation to the snapshot vdisk.
**
**          A separate completion routine handles the completion of write
**          requests.
**
**  @param  ilt - The ILT of the request that was split
**
**  @return none
**
******************************************************************************
*/

void ss_rcomp(ILT *ilt)
{
    VRP     *vrp;
    SYNC    *syn;
    SSMS    *ssms;
    UINT16  retry_count;
    UINT32  regsave[15];
    UINT16  error=EC_IO_ERR;
    VDD     *vdd = NULL;
    memcpy(&regsave, &g0, sizeof(regsave)); /* Save g registers */
    vrp = (VRP *)ilt->ilt_normal.w0;
    syn = (SYNC *)ilt->ilt_normal.w5;
    ssms = (SSMS *)ilt->ilt_normal.w6;
    vdd  = gVDX.vdd[vrp->vid];

    if (vdd && (BIT_TEST(vdd->attr, VD_BBEBUSY)))
    {
        vrp->status = EC_BEBUSY;  // ISE upgrade case
        ilt->misc = 0;
        error      = EC_BEBUSY;
    }

    retry_count = XIO_LSW(ilt->misc);

    if (vrp->status != EC_OK)
    {
        if (retry_count == 0)
        {
            /* Read completion status error, probably kill snapshot here */
            if (vrp->status == EC_BEBUSY)
            {
                fprintf(stderr, "%s: <ISE-DEBUG> Read(COW) to Source failed-VRP status %x, vdd  %x status %x\n",
                        __func__, vrp->status, vrp->vid, vdd->status);
            }
            else
            {
                fprintf(stderr, "%s: Read(COW) to Source failed with retries VRP status %x, vdd  %x status %x\n",
                      __func__,vrp->status, vrp->vid, vdd->status);

                logSPOOLevent(SS_READ_SRC_BAD, LOG_AS_ERROR, 10, 0, ssms->ssm_srcvid);
            }

            ssmdep  *ssms_ic = syn->syn_iscnt;

            if (ssms_ic)
            {
                unsigned    ix;
                int         segix = syn->syn_segnum & SEG_ACC_MASK;
                ILT         **ilts = ssms_ic->ssmdep_cow_seg[segix].ilts;
                UINT16      ssmdep_max = get_max_ssmdep_ix(ssms_ic, segix);

                for (ix = 0; ix < ssmdep_max; ++ix)
                {
                    ILT     *silt = ilts[ix];

                    ilts[ix] = NULL;
                    if (silt)
                    {
                        ((VRP *)silt->ilt_normal.w0)->status = vrp->status;
                        ss_errcomp(silt, error);
                    }
                }
            }
            else
            {
                ss_errcomp(ilt, error);
                if (--cow_orc > 10000)
                {
                    fprintf(stderr, "%s: cow_orc underflow %08x\n",
                        __func__, cow_orc);
                }
            }
        }
        else
        {
            retry_count--;
            ilt->misc = (ilt->misc & 0xffff0000) | (0x0000ffff & retry_count);
            vrp->status = EC_OK;
            vrp->function = VRP_INPUT | (1 << VRP_SNAPSHOT_SPECIAL);
            vrp->vid = ssms->ssm_srcvid;
            vrp->length = SEGSIZE_SEC;
            vrp->startDiskAddr = (UINT64)syn->syn_segnum * SEGSIZE_SEC;
            fprintf(stderr, "%s: Read(COW) to Source failed, retries remaining %d\n",
                    __func__, retry_count);
// EnqueueILT requires g0,g1,g2 saved.
            EnqueueILT((void *)V_que, ilt, (void *)ilt->cr);
        }
        goto out;
    }

    /*
     * Read completion status was good. Set up to write data to
     * destination VID.
     */

    ssmdep  *ssms_ic = syn->syn_iscnt;

    if (ssms_ic)
    {
        unsigned    ix;
        int         segix = syn->syn_segnum & SEG_ACC_MASK;
        SGL         *sgl = ssms_ic->ssmdep_cow_seg[segix].sgl;
        ILT         **ilts = ssms_ic->ssmdep_cow_seg[segix].ilts;
        UINT16      ssmdep_max = get_max_ssmdep_ix(ssms_ic, segix);

        for (ix = 0; ix < ssmdep_max; ++ix)
        {
            ILT     *wilt = ilts[ix];

            ilts[ix] = NULL;
            if (wilt)
            {
                VRP     *wvrp = (VRP *)wilt->ilt_normal.w0;
                SYNC    *wsync = (SYNC *)wilt->ilt_normal.w5;
                SSMS    *wssms = (SSMS *)wilt->ilt_normal.w6;

                if ((UINT32)sgl == 0xfeedf00d)
                {
                    fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
                    abort();
                }
                wvrp->pSGL = sgl;
                wvrp->sglSize = sizeof(SGL) + sizeof(SGL_DESC);
                start_cow_write(wssms, wsync, wilt);
            }
        }
    }
    else
    {
        start_cow_write(ssms, syn, ilt);
    }

out:
    memcpy(&g0, &regsave, sizeof(regsave));     /* Restore g registers */
}   /* End of ss_rcomp */


/*
******************************************************************************
**
**  @brief  Process split request completion to a snapshot VDisk
**
**          Checks for completion of all requests in the group. Releases
**          resources as needed.
**
**  @param  ilt - The ILT of the request that was split
**
**  @return none
**
******************************************************************************
*/

void ss_splcomp(UINT32 dummy UNUSED, ILT *ilt)
{
    SPLIT   *split;
    ILT     *master_ilt;
    SPLIT   *master_split;
    ILT     *orig_ilt;
    SSMS    *ssms;
    SYNC    *syn;
    VRP     *vrp;
    VRP     *orig_vrp;
    VRP     *master_vrp;
    UINT32  status;
    UINT32  regsave[15];

    memcpy(regsave, &g0, sizeof(regsave));       /* Save all g registers. */

    split = (SPLIT *)ilt->ilt_normal.w3;
    master_ilt = split->spl_master;
    orig_ilt = split->spl_origilt;
    master_split = (SPLIT *)master_ilt->ilt_normal.w3;
    ssms = split->spl_ssms;
    vrp = (VRP *)ilt->ilt_normal.w0;
    orig_vrp = (VRP *)orig_ilt->ilt_normal.w0;
    master_vrp = (VRP *)master_ilt->ilt_normal.w0;
    syn = (SYNC *)ilt->ilt_normal.w5;

    status = vrp->status;       /* Get the status of the request */
    if (status != EC_OK)
    {
        // The status of this portion of the split is bad, update
        // the master split status and log a message.
        master_split->spl_stat = status;
    }

    ++master_split->spl_finished;

    /* If all have finished, then call the original ILTs completion routine */

    if (master_split->spl_finished == master_split->spl_segcnt)
    {
        orig_vrp->status = master_split->spl_stat;  /* Update original VRP */
        K_comp(orig_ilt + 1);           /* Complete it */
    }

    /* If this is not the master, then free the ILT/VRP/SGL and SPLIT */

    if (ilt != master_ilt)
    {
        p_Free(split, sizeof(*split), __FILE__, __LINE__);
// free_ss_request changes g0,g1.
        free_ss_request(ilt);       /* Free the ILT/VRP/SGL */
    }

    /* If all have finished, then free the master ILT/VRP/SGL and SPLIT */

    if (master_split->spl_finished == master_split->spl_segcnt)
    {
        p_Free(master_split, sizeof(*master_split), __FILE__, __LINE__);
        free_ss_request(master_ilt);
    }

    memcpy(&g0, regsave, sizeof(regsave));  /* Restore all g registers. */
}   /* End of ss_splcomp */


/*
******************************************************************************
**
**  @brief  Process source access request completion to a snapshot VDisk
**
**          Processes completions of snapshot access that were directed to
**          the source VDisk.
**
**  @param  ilt - The ILT of the request that was completed
**
**  @return none
**
******************************************************************************
*/

static void ss_src_acc_proc(ILT *ilt)
{
    SSMS        *ssms;
    SYNC        *syn;
    UINT32      seg;
    char        start_cows = FALSE;

    syn = (SYNC *)ilt->ilt_normal.w5;
    ssms = (SSMS *)ilt->ilt_normal.w6;
    seg = syn->syn_segnum;

    SYNC    *fsyn;
    SYNC    *psyn1;

    fsyn = search_sync_groups(ssms, seg);

    if (!fsyn)
    {
        fprintf(stderr, "%s: sync record for seg %d not found\n", __func__, seg);
        abort();   /* Crash */
    }

    psyn1 = fsyn->syn_prev;

    if (!BIT_TEST(fsyn->syn_state, syn_state_acc_src))
    {
        fprintf(stderr, "%s: First sync not acc src\n", __func__);
        return;
    }

    SYNC    *tsyn;
    SYNC    *psyn2;

    psyn2 = NULL;
    for (tsyn = fsyn; tsyn; tsyn = tsyn->syn_acclst)
    {
        if (tsyn == syn)
        {
            break;
        }
        psyn2 = tsyn;
    }

    if (!tsyn)
    {
        fprintf(stderr, "%s: sync record %p not found!\n", __func__, syn);
        return;
    }

    /*
     * At this point, the record has been found, and the parent is
     * accessible so the record can be deleted wherever it is.
     */

    if (fsyn == syn)    /* If first source access completing */
    {
        /* If another source access exists */
        if (syn->syn_acclst)
        {
            syn->syn_acclst->syn_deplst = syn->syn_deplst;
            /* Free and go */
        }
        else
        {
            tsyn = syn->syn_deplst;
            start_cows = TRUE;
        }
    }
    else if (psyn2)
    {
        psyn2->syn_acclst = syn->syn_acclst;
        /* Free and go */
    }
    syn->syn_acclst = NULL;
    syn->syn_deplst = NULL;

    if (syn != syn->syn_master)
    {
        fprintf(stderr, "%s: syn (%p) != syn_master (%p)\n",
            __func__, syn, syn->syn_master);
    }

    ssmdep      *iscnt;

    iscnt = free_syncs_in_group(ssms, syn->syn_master);

    if (iscnt)
    {
        fprintf(stderr, "%s: iscnt not NULL - %p\n", __func__, iscnt);
    }

    /*
     * Start any COWs for this segment if all of the source accesses
     * have been completed. We need to check that there are any, of
     * course.
     */

    if (start_cows && tsyn)
    {
        start_deferred_cow(tsyn);
    }
}   /* End of ss_src_acc_proc */


/*
******************************************************************************
**
**  @brief  Process source access request completion to a snapshot VDisk
**
**          Processes completions of snapshot access that were directed to
**          the source VDisk.
**
**  @param  ilt - The ILT of the request that was completed
**
**  @return none
**
******************************************************************************
*/

void ss_srccomp(UINT32 dummy UNUSED, ILT *ilt)
{
    UINT32  status;
    SYNC    *syn;
    ILT     *orig_ilt;
    VRP     *orig_vrp;
    UINT32  regsave[15];

    memcpy(&regsave, &g0, sizeof(regsave));     /* Save all g registers */

    status = ((VRP *)ilt->ilt_normal.w0)->status;
    syn = (SYNC *)ilt->ilt_normal.w5;
    orig_ilt = syn->syn_master->syn_firstilt;
    orig_vrp = (VRP *)orig_ilt->ilt_normal.w0;

    ss_src_acc_proc(ilt);       /* Process source access completion */
    free_ss_request(ilt);

    orig_vrp->status = status;      /* Update original VRP */
    K_comp(orig_ilt + 1);           /* Complete it */

    memcpy(&g0, &regsave, sizeof(regsave));     /* Restore all g registers */
}   /* End of ss_srccomp */


/*
******************************************************************************
**
**  @brief  Process split request snapshot completion to a source snapshot VDisk
**
**          Checks for completion of all requests in the group. Releases
**          resources as needed.
**
**  @param  ilt - The ILT of the request that was split
**
**  @return none
**
******************************************************************************
*/

void ss_splsrccomp(UINT32 dummy UNUSED, ILT *ilt)
{
    UINT32  regsave[15];

    memcpy(&regsave, &g0, sizeof(regsave));     /* Save all g registers */

    ss_src_acc_proc(ilt);       /* Process source access completion */
    ss_splcomp(0, ilt);         /* Complete split request */

    memcpy(&g0, &regsave, sizeof(regsave));     /* Restore all g registers */
}   /* End of ss_splsrccomp */


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
