/* $Id: ss_nv.c 160003 2012-10-04 22:05:51Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       ss_nv.c
**
**  @brief      Snapshot nonvolatile storage management
**
**  To provide support for the async replication pool vdisk. This includes
**  all management functions and variables
**
**  Copyright (c) 2007 - 2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "nvram.h"
#include "misc.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "ilt.h"
#include "virtual.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CT_defines.h"
#include "ecodes.h"
#include "def.h"
#include "vdd.h"
#include "qu.h"
#include "defbe.h"
#include "RL_PSD.h"
#include "RL_RDD.h"
#include "ss_nv.h"
#include "ss_nv_funcs.h"
#include "ilt.h"
#include "vrp.h"
#include "system.h"
#include "pm.h"
#include "ecodes.h"
#include "def_lun.h"
#include "pcb.h"
#include "online.h"
#include "rebuild.h"
#include "ss_comp.h"
#include "cor.h"
#include "dcd.h"
#include "mem_pool.h"
#include "ddr.h"
#include "misc.h"

/*
******************************************************************************
** Private definitions
******************************************************************************
*/

#define MAX_SSMS_TASKS_ALLOWED  15      // Allow at most 15 SSMS read tasks.
#define MAX_OGER_TASKS_ALLOWED  15      // Allow at most 15 OGER read tasks.

#ifndef PERF
// #define DUMP_OGER_BITMAP    // TURN THIS OFF IN PERF BUILD
// #define M4_OGERS
#endif
void D_init_slrm(UINT32, UINT32, SSMS *);
extern void ss_invalidate_snapshot(UINT16 ss_vid);

/*
******************************************************************************
** Private data structures
******************************************************************************
*/

// The Oger queue is for all SSMSs and for both pool_id's, when there are
// too many OGER read tasks created.
struct oger_task_queue {
    struct oger_task_queue *next;
    int oger_to_read;
    OGER **N_poger_table;
    int pool_id;
    int which_fork_count;
} *oger_task_queue_first = NULL;

/*
******************************************************************************
** Private variables
******************************************************************************
*/
PCB            *ss_restore_pcb = NULL;
UINT32          snappool_owner[2] = { 0, 0 };
// OGER count from SSMS reading (which snappool is the index).
int             cuml_ssms_oger_cnt[2] = { 0, 0};
// Number of tasks forked for reading SSMSs (which snappool is the index).
int             num_ssms_read_tasks[2] = {0, 0};
// Number of tasks forked for reading OGERs (which snappool is the index).
int             num_oger_read_tasks[2] = {0, 0};
// Number of OGERs restored from SSMS reading (which snappool is the index).
int             num_ogers_from_ssms[2] = {0, 0};
// Number of right fork tasks, by SSMS.
int             g_which_fork_count[2] = {0, 0};
int             right_fork_count[2][MAX_SNAPSHOT_COUNT];
// Number of ogers read for an SSMS.
int             ogers_read_per_ssms[2][MAX_SNAPSHOT_COUNT];

/* Count of outstanding nv requests to virtual. */
UINT32          outst_nv_reqs;

/* Global flag to indicate a duplicate oger found. */
UINT32          g_duplicate_oger = FALSE;

/* Version of snappool: 0=none, 2=old, 3=new. */
UINT8            gss_version[2] = {0, 0};

/*
******************************************************************************
** Public variables defined in other files
******************************************************************************
*/
extern UINT32   DEF_ssms_table[MAX_SNAPSHOT_COUNT * 2];
extern UINT8    DEF_dog_table[(MAX_OGER_COUNT_GT2TB/8) * 2];  /* number bytes, 2 snappools. */
extern OGER    *gnv_oger_array[2];
extern SS_HEADER_NV *gnv_header_array[2];
extern UINT32   DEF_oger_cnt[2];

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
ILT            *alloc_request(UINT32 buffer_size, UINT16 function, OGER *nv_oger);
static OGER_NV *read_oger_nv(UINT16 ordinal, ILT *ilt, OGER *nv_oger);
void            restore_ss_nv_task(void);
void            ss_waitformp_task(void);
void            ss_read_ssms(UINT32 dummy0 UNUSED, UINT32 dummy1 UNUSED,
                  UINT32 ordinal, OGER *nv_oger, int pool_id, void *Npot);
void            ss_disown_snappool(UINT32 dummy0 UNUSED, UINT32 dummy1 UNUSED,
                  UINT16 spool_id);
void            Task_read_r_oger_from_nv(UINT32 dummy0 UNUSED, UINT32 dummy1 UNUSED,
                  int ordinal, OGER **N_poger_table, OGER *nv_oger, int pool_id,
                  int which_fork_count, SSMS *ssms);
void            ss_tmpsuspend_allsnapshots(UINT16 spool_vid);
static void     ss_reset_tmpsuspend_allsnapshots(void);

/*
******************************************************************************
** Public function prototypes - defined in other files
******************************************************************************
*/
UINT16          apool_get_tid(VDD *pVDD);
extern void     CT_LC_restore_ss_nv_task(void);
extern void     CT_LC_ss_read_ssms(void);
extern void     CT_LC_ss_disown_snappool(void);
extern void     CT_LC_Task_read_r_oger_from_nv(void);
extern void     DEF_Kill_SS_Structs(SSMS *);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

static void dump_bitmap(UINT8 *ogertable, const char *str, int pool_id)
{
    int i;
    int j;
    int max_oger_count;

    fprintf(stderr, "SSDBG: DUMPING OGER BITMAP %s, POOLID %d\n", str, pool_id);
    if (gss_version[pool_id] != SS_NV_VERSION_GT2TB)
    {
        max_oger_count = MAX_OGER_COUNT;
    }
    else
    {
        max_oger_count = MAX_OGER_COUNT_GT2TB;
    }
    for (i = 0; i < max_oger_count / 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            fprintf(stderr, "%d", (ogertable[i] >> j) & 1);
        }
        if ((i % 8) == 7)
        {
            fprintf(stderr, "\n");
        }
        else
        {
            fprintf(stderr, " ");
        }
    }
}                                       /* End of dump_bitmap */


/**
******************************************************************************
**
**  @brief      Allocate and initialize the all the SSMS region map tables.
**
**  Allocates many Region Map tables and initialize last one for remainder.
**
**  @param remaining_rm   = Number of segments to initialize in last RM.
**  @param number_full_rm = Number of region maps to create.
**  @param ssms           = SSMS to put region maps into.
**
**  @return     None.
**
******************************************************************************
*/
void D_init_slrm(UINT32 remaining_rm, UINT32 number_full_rm, SSMS *ssms)
{
    struct RM *rm;
    struct SM *sm;
    unsigned int i;
    unsigned int j;

    /* Do full RM's first. */
    for (i = 0; i < number_full_rm; i++)
    {
        /* Assign memory for private memory RM. */
        rm = (struct RM *)p_MallocC(sizeof(*rm), __FILE__, __LINE__);
        /* Set SSMS regmap array for this region. */
        ssms->ssm_regmap[i] = rm;

        /* Do full SM's. */
        for (j = 0; j < MAXRMCNT; j++)
        {
            /* Allocate a segment map table in private memory (SM). */
            sm = (struct SM *)p_MallocC(sizeof(*sm), __FILE__, __LINE__);
            /* Full segment map */
            sm->cnt = REGSIZE_SEG;
            memset(sm->segments, 0xff, SMTBLSIZE);  /* Note: segments defined as /4. */

            rm->regions[j] = sm;                    /* Link segment map into RM */
        }
    }
    if (remaining_rm == 0)
    {
        return;
    }

    i = number_full_rm;

    /* Assign memory for private memory RM. */
    rm = (struct RM *)p_MallocC(sizeof(*rm), __FILE__, __LINE__);
    /* Set SSMS regmap array for this region. */
    ssms->ssm_regmap[i] = rm;

    /* Now do remaining region map, which is partially filled. */
    j = 0;
    for (;;)
    {
        /* Allocate a segment map table in private memory (SM). */
        sm = (struct SM *)p_MallocC(sizeof(*sm), __FILE__, __LINE__);
        if (remaining_rm >= REGSIZE_SEG)            /* Full segment map is required */
        {

            rm->regions[j] = sm;     /* Link segment map into RM */
            sm->cnt = REGSIZE_SEG;                  /* save segment count */

            memset(sm->segments, 0xff, SMTBLSIZE);  /* Note: segments defined as /4. */

            j++;                                    /* To next region */
            remaining_rm = remaining_rm - REGSIZE_SEG; /* subtract segments for this region */
            if (remaining_rm == 0)
            {
                return;
            }
        }
        else
        {
            /* partial segment map required */
            rm->regions[j] = sm;                    /* Link segment map into RM */
            sm->cnt = remaining_rm;                 /* save segment count */

            memset((void *)sm->segments, 0xff, (remaining_rm >> 5) * 4); /* # 32 bit segments */

            i = (remaining_rm >> 5);
            remaining_rm = remaining_rm & 0x1f;     /* Get number of bits remaining */
            if (remaining_rm != 0)
            {
                /* Set last segment word in table. */
                sm->segments[i] = 0xffffffff << (32-remaining_rm);
            }
            return;
        }
    }
}   /* End of D_init_slrm */


/**
******************************************************************************
**
**  @brief      Clear the snapshot nonvolatile data
**
**  This function clears the snapshot nvram by writing a header with a bad
**  checksum to the OGER containing the nv data.
**
**  @return     Status -- 0 is good else vrp status code
**
******************************************************************************
*/
UINT32 clear_all_ss_nv(OGER *nv_oger)
{
    SS_HEADER_NV   *header_nv;
    ILT            *ilt_header = NULL;
    VRP            *vrp_header;
    SGL            *sgl_header;
    SGL_DESC       *desc_header;
    int             retry_count = SNAP_NVRETRY;
    UINT32          ret_val;
    UINT8          *oger_table;

    if (nv_oger == NULL)
    {
        fprintf(stderr, "SSDBG: %s nvoger is NULL\n", __func__);
        return (EC_INV_DEV);
    }

    if (gnv_oger_array[nv_oger->ogr_vid & 0x1] == NULL)
    {
        fprintf(stderr, "SSDBG: %s gnv_oger_array is NULL\n", __func__);
        // Exit if the oger_nv
        return (EC_ABORT);
    }
    // Allocate a header update request.
    do
    {
        ilt_header = alloc_request((UINT32)sizeof(SS_HEADER_NV), VRP_OUTPUT, nv_oger);
        vrp_header = (VRP *)(ilt_header->ilt_normal.w0);
        sgl_header = vrp_header->pSGL;
        vrp_header->startDiskAddr = 0;
        desc_header = (SGL_DESC *)(sgl_header + 1);
        header_nv = (SS_HEADER_NV *)desc_header->addr;

        // Clear out the data in the header
        memset(header_nv, 0, sizeof(SS_HEADER_NV));

        // Queue the request to virtual layer with wait
        outst_nv_reqs++;
        EnqueueILTW((void *)V_que, ilt_header);
        outst_nv_reqs--;
        ret_val = vrp_header->status;
        // Free the ILT, VRP, and SGL for the header
        s_Free(sgl_header, sizeof(SGL) + sizeof(SGL_DESC) + desc_header->len, __FILE__, __LINE__);
        vrp_header->pSGL = NULL;
        PM_RelILTVRPSGL(ilt_header, vrp_header);

        if (ret_val == EC_OK)
        {
            break;
        }
        else
        {
            fprintf(stderr, "%s: clear_all_ssnv failed retries remaining = %d\n", __func__, retry_count);
        }

    } while (--retry_count > 0);

    /* Clear the global DOG table and write it into the NV area. */
    oger_table = &DEF_dog_table[(nv_oger->ogr_vid & 0x1)*MAX_OGER_COUNT_GT2TB/8];
    memset(oger_table, 0, MAX_OGER_COUNT_GT2TB/8);
    // Mark the first OGER as allocated -- where all the SSMS_NV and OGER_NV are stored.
    oger_table[0] = 0x1;

    // Free the global nv header struct for this snappool
    if (gnv_header_array[nv_oger->ogr_vid & 0x1])
    {
        p_Free(gnv_header_array[nv_oger->ogr_vid & 0x1], sizeof(SS_HEADER_NV), __FILE__, __LINE__);
        gnv_header_array[nv_oger->ogr_vid & 0x1] = NULL;
    }

    // Free up the nv_oger and clear it's pointer.
    gnv_oger_array[nv_oger->ogr_vid & 0x1] = NULL;
    p_Free(nv_oger, sizeof(OGER), __FILE__, __LINE__);

    return (ret_val);
}                                      /* End of clear_all_ss_nv */


/**
******************************************************************************
**
**  @brief      We need to read a right OGER, fork, or queue if too many.
**
**  @param      oger_to_read     -- which oger to read.
**  @param      N_poger_table    -- memory holding all ogers for a snappool.
**  @param      nv_oger          -- First oger for snappool.
**  @param      pool_id          -- snappool 0 or 1 (for controller) -- from vid.
**  @param      which_fork_count -- index into array of right task counts for SSMS.
**
**  @return     None.
**
******************************************************************************
*/
static void right_task_queue(int oger_to_read, OGER** N_poger_table, OGER *nv_oger,
                             int pool_id, int which_fork_count, SSMS *ssms)
{
    // Must do count first, before task create or queue, so that original SSMS
    // read will wait for all OGER reads to finish.
    right_fork_count[pool_id][which_fork_count] += 1;

    // If only a few OGER read tasks, fork, else put it on a queue.
    if (num_oger_read_tasks[pool_id] >= MAX_OGER_TASKS_ALLOWED)
    {
        struct oger_task_queue *oger_task_queue;

        oger_task_queue = p_MallocC(sizeof(struct oger_task_queue), __FILE__, __LINE__);
        oger_task_queue->oger_to_read = (int)N_poger_table[oger_to_read]->ogr_rightch;
        oger_task_queue->N_poger_table = N_poger_table;
        oger_task_queue->pool_id = pool_id;
        oger_task_queue->which_fork_count = which_fork_count;
        oger_task_queue->next = oger_task_queue_first;
        oger_task_queue_first = oger_task_queue;
// fprintf(stderr, "putting oger_to_read=%d on OGER task queue\n", oger_to_read);
    }
    else
    {
        // Increment number of read OGER tasks outstanding.
        num_oger_read_tasks[pool_id] += 1;

        // Fork task to read the right oger entry.
        CT_fork_tmp = (unsigned long)"Task_read_r_oger_from_nv";
        TaskCreate8(C_label_referenced_in_i960asm(Task_read_r_oger_from_nv), K_xpcb->pc_pri,
            (int)N_poger_table[oger_to_read]->ogr_rightch,
            (UINT32)N_poger_table, (UINT32)nv_oger, pool_id, which_fork_count,
            (UINT32)ssms);
    }
}                                       /* End of right_task_queue */


/**
******************************************************************************
**
**  @brief  Read an OGER entry from NV (in the snapshot vdisk).
**
**  @param  ordinal         -- The number of the OGER to read.
**  @param  N_poger_table   -- Memory to hold all ogers for a snappool.
**  @param  nv_oger         -- First oger for snappool (the header).
**  @param  pool_id         -- snappool 0 or 1 (for controller) -- from vid.
**  @param  which_fork_count - Count of right tasks for this SSMS.
**  @param  ssms            - Pointer to SSMS
**
**  @return None.
**
******************************************************************************
*/
static void read_oger_from_nv(int ordinal, OGER **N_poger_table, OGER *nv_oger,
                              int pool_id, int which_fork_count, SSMS *ssms)
{
    ILT            *ilt_oger;
    VRP            *vrp_oger;
    SGL            *sgl_oger;
    SGL_DESC       *desc_oger;
    OGER_NV        *oger_nv;
    OGER_NV_GT2TB  *oger_nv_gt2tb;
    OGER           *oger;
    UINT32          y;
    UINT32          oger_read_fail = FALSE;

    int             oger_to_read = ordinal;

    // Get structures to issue a read request.
/* NOTE: sizeof(OGER_NV_GT2TB) <= sizeof(OGER_NV) */
    ilt_oger = alloc_request((UINT32)sizeof(OGER_NV), VRP_INPUT, nv_oger);
    vrp_oger = (VRP *)(ilt_oger->ilt_normal.w0);
    sgl_oger = vrp_oger->pSGL;
    desc_oger = (SGL_DESC *)(sgl_oger + 1);

    while (1)
    {
        if (oger_to_read && N_poger_table[oger_to_read])
        {
            VDD *vdd;

            fprintf(stderr, "%s: Duplicate oger, ord=%d, vid1=%d, vid2=%d\n",
                __func__, oger_to_read, N_poger_table[oger_to_read]->ogr_ssvid,
                ssms->ssm_ssvid);
            ss_invalidate_snapshot(N_poger_table[oger_to_read]->ogr_ssvid);
            vdd = gVDX.vdd[N_poger_table[oger_to_read]->ogr_ssvid];
            if (vdd && vdd->vd_incssms)
            {
                remove_ssms_from_src_vdd(vdd->vd_incssms,
                        gVDX.vdd[vdd->vd_incssms->ssm_srcvid]);
            }

            ss_invalidate_snapshot(ssms->ssm_ssvid);
            remove_ssms_from_src_vdd(ssms, gVDX.vdd[ssms->ssm_srcvid]);
            oger_to_read = 0;
            g_duplicate_oger = TRUE;
            continue;
        }

        if ((!oger_to_read) || (oger_read_fail == TRUE))
        {
            // This right fork processing has terminated.
            right_fork_count[pool_id][which_fork_count] -= 1;
            if (right_fork_count[pool_id][which_fork_count] == 0)
            {
                // Notify that the right fork processing has terminated, if all are gone.
                TaskReadyByState(PCB_SS_R_OGER_READS);
            }

            // If no OGER tasks on queue, exit.
            if (oger_task_queue_first == NULL)
            {
                break;
            }

            // Take first queued task's parameters off queue, and dequeue.
            oger_to_read = oger_task_queue_first->oger_to_read;
            N_poger_table = oger_task_queue_first->N_poger_table;
            pool_id = oger_task_queue_first->pool_id;
            which_fork_count = oger_task_queue_first->which_fork_count;
            nv_oger = gnv_oger_array[pool_id];

            // Free up memory and forward queue.
            struct oger_task_queue *next = oger_task_queue_first->next;

            p_Free(oger_task_queue_first, sizeof(struct oger_task_queue), __FILE__, __LINE__);
            oger_task_queue_first = next;
 //fprintf(stderr, "continuing OGER read for oger_to_read=%d\n", oger_to_read);
            continue;
        }

        oger_nv = read_oger_nv((UINT16)oger_to_read, ilt_oger, nv_oger);
        // If any problems reading oger, exit.
        if (!oger_nv)
        {
            ss_invalidate_snapshot(ssms->ssm_ssvid);
            remove_ssms_from_src_vdd(ssms, gVDX.vdd[ssms->ssm_srcvid]);
            oger_read_fail = TRUE;
            continue;
        }

        /* Increment count of ogers read for this ssms. */
        ogers_read_per_ssms[pool_id][which_fork_count] += 1;

        // Allocate an OGER struct for the runtime software
        oger = (OGER *)p_MallocC(sizeof(OGER), __FILE__, __LINE__);
        N_poger_table[oger_to_read] = oger;

        // Populate the OGER from the NV data.
        oger->ogr_vid = oger_nv->vid;
        oger->ogr_leftch = (OGER *)(UINT32)oger_nv->leftch;
        oger->ogr_rightch = (OGER *)(UINT32)oger_nv->rightch;
        oger->ogr_parent = (OGER *)(UINT32)oger_nv->parent;
        oger->ogr_ord = oger_nv->ordinal;
        oger->ogr_maxpr = oger_nv->maxprobe;
        oger->ogr_segcnt = oger_nv->segcount;
        /* Everything under here changes for version 2 or version 3. */
        if (oger_nv->version != SS_NV_VERSION_GT2TB)
        {
            oger->ogr_stat = oger_nv->status;
            oger->ogr_sda = oger_nv->sda;
            oger->ogr_sdakey = oger_nv->sdakey;
            memcpy(oger->ogr_segfld, oger_nv->segfield, SEGSPEROGER / 8);
            memcpy(oger->ogr_sdamap, oger_nv->sdamap, SEGSPEROGER * 4);
            oger->ogr_link = (OGER *)(UINT32)oger_nv->next;
        }
        else
        {
            oger_nv_gt2tb = (OGER_NV_GT2TB*)oger_nv;
            oger->ogr_stat = oger_nv_gt2tb->status;
            oger->ogr_sda = oger_nv_gt2tb->sda;
            oger->ogr_sdakey = oger_nv_gt2tb->sdakey;
            memcpy(oger->ogr_segfld, oger_nv_gt2tb->segfield, SEGSPEROGER / 8);
            memcpy(oger->ogr_sdamap, oger_nv_gt2tb->sdamap, SEGSPEROGER * 4);
            oger->ogr_link = (OGER *)(UINT32)oger_nv_gt2tb->next;
        }
        oger->ogr_ssvid = ssms->ssm_ssvid;

        for (y = 0; y < SEGSPEROGER; y++)
        {
            if (BIT_TEST(oger->ogr_segfld[y / 8], y % 8))
            {
                int seg_cleared = ss_clr_seg_bit(oger->ogr_sdamap[y], ssms);
                if (seg_cleared != 0)
                {
                    print_seg_clear_error(oger->ogr_sdamap[y], ssms, seg_cleared, __FILE__, __func__, __LINE__);
                }
            }
        }
        num_ogers_from_ssms[pool_id] += 1;

        // If left oger entry is null, use this task to read possible right.
        if ((int)N_poger_table[oger_to_read]->ogr_leftch == 0)
        {
            oger_to_read = (int)N_poger_table[oger_to_read]->ogr_rightch;
        }
        else
        {
            // If a right oger entry exists, fork task to read it.
            if ((int)N_poger_table[oger_to_read]->ogr_rightch != 0)
            {
                right_task_queue(oger_to_read, N_poger_table, nv_oger, pool_id,
                        which_fork_count, ssms);
            }
            // Want to read new left oger next.
            oger_to_read = (int)N_poger_table[oger_to_read]->ogr_leftch;
        }
    }

    // Free the ILT, VRP, and SGL for the OGER
    s_Free(sgl_oger, sizeof(SGL) + sizeof(SGL_DESC) + desc_oger->len, __FILE__, __LINE__);
    vrp_oger->pSGL = NULL;
    PM_RelILTVRPSGL(ilt_oger, vrp_oger);
}                                      /* End of read_oger_from_nv */


/**
******************************************************************************
**
**  @brief  Task to read right OGER entry from NV (in the snapshot vdisk).
**
**  @param  header_nv   -- pointer to header for ss nv.
**  @param  nv_oger     -- First oger for snappool.
**  @param  pool_id     -- snappool 0 or 1 (for controller) -- from vid.
**  @param  N_poger_table -- memory to hold all possible ogers for a snappool.
**  @param  which_fork_count -- count of right tasks for this SSMS.
**  @param  SSMS        - Pointer to SSMS.
**
**  @return None.
**
******************************************************************************
*/
void Task_read_r_oger_from_nv(UINT32 dummy0 UNUSED, UINT32 dummy1 UNUSED,
                              int oger_to_read, OGER **N_poger_table, OGER *nv_oger,
                              int pool_id, int which_fork_count, SSMS *ssms)
{
    read_oger_from_nv(oger_to_read, N_poger_table, nv_oger, pool_id,
            which_fork_count, ssms);

    // One less oger task exists.
    num_oger_read_tasks[pool_id] -= 1;
}                                       /* End of Task_read_r_oger_from_nv */


/**
******************************************************************************
**
**  @brief      We are finished with an SSMS task, possibly start another.
**
**  @param      ordinal     -- Which SSMS to read.
**  @param      header_nv   -- pointer to header for ss nv.
**  @param      nv_oger     -- First oger for snappool.
**  @param      pool_id     -- snappool 0 or 1 (for controller) -- from vid.
**  @param      Npot        -- memory to hold all possible ogers for a snappool.
**
**  @return     None.
**
**  @attention  This is one of MAX_SSMS_TASKS_ALLOWED possible such tasks.
**
******************************************************************************
*/
static void finished_with_SSMS_task(int pool_id)
{
    // If we have a bunch of SSMS read tasks and are waiting for another, allow it.
    if (num_ssms_read_tasks[pool_id] == MAX_SSMS_TASKS_ALLOWED)
    {
// fprintf(stderr, "Allow more SSMS tasks.\n");
        // Start all tasks "waiting for SSMS read tasks to complete".
        TaskReadyByState(PCB_SS_SSMS_MORE_TASKS);
    }

    // One less task exists
    num_ssms_read_tasks[pool_id] -= 1;
    if (num_ssms_read_tasks[pool_id] == 0)
    {
// fprintf(stderr, "All SSMS tasks finished.\n");
        // Start all tasks "waiting for SSMS read tasks to complete".
        TaskReadyByState(PCB_SS_SSMS_READ);
    }
// fprintf(stderr, "a SSMS task finished (%d left).\n", num_ssms_read_tasks[pool_id]);
}                                       /* End of finished_with_SSMS_task */


/**
******************************************************************************
**
**  @brief      Read an SSMS entry from the NV (in the snapshot vdisk).
**
**  @param      ordinal     -- Which SSMS to read.
**  @param      header_nv   -- pointer to header for ss nv.
**  @param      nv_oger     -- First oger for snappool.
**  @param      pool_id     -- snappool 0 or 1 (for controller) -- from vid.
**  @param      Npot        -- memory to hold all possible ogers for a snappool.
**
**  @return     None.
**
**  @attention  This is one of MAX_SSMS_TASKS_ALLOWED possible such tasks.
**
******************************************************************************
*/
void ss_read_ssms(UINT32 dummy0 UNUSED, UINT32 dummy1 UNUSED,
                  UINT32 ordinal, OGER *nv_oger, int pool_id, void *Npot)
{
    OGER           **N_poger_table = Npot;
    ILT            *ilt_ssms = NULL;
    VRP            *vrp_ssms = NULL;
    SGL            *sgl_ssms = NULL;
    SGL_DESC       *desc_ssms = NULL;
    SSMS_NV        *ssms_nv;

    UINT32          crc;
    SSMS           *ssms = NULL;
    VDD            *vdd;
    int             oger_to_read;
    int             which_fork_count;

    // We need a unique counter for the right forks, for each SSMS.
    which_fork_count = g_which_fork_count[pool_id];
    g_which_fork_count[pool_id]++;
    right_fork_count[pool_id][which_fork_count] = 0;
    ogers_read_per_ssms[pool_id][which_fork_count] = 0;

    // Allocate the ILT, VRP, and SGL for the SSMS retrieval(s)
    ilt_ssms = alloc_request((UINT32)sizeof(SSMS_NV), VRP_INPUT, nv_oger);
    vrp_ssms = (VRP *)(ilt_ssms->ilt_normal.w0);
    sgl_ssms = vrp_ssms->pSGL;
    desc_ssms = (SGL_DESC *)(sgl_ssms + 1);
    ssms_nv = (SSMS_NV *)desc_ssms->addr;

    if (gss_version[pool_id] == SS_NV_VERSION_GT2TB)
    {
        vrp_ssms->startDiskAddr = FIRST_SSMS_GT2TB + (ordinal * SSMS_NV_ALLOC_GT2TB);
    }
    else
    {
        vrp_ssms->startDiskAddr = FIRST_SSMS + (ordinal * SSMS_NV_ALLOC);
    }
    vrp_ssms->function = VRP_INPUT;
    vrp_ssms->vid = nv_oger->ogr_vid;

    outst_nv_reqs++;
    EnqueueILTW((void *)V_que, ilt_ssms);
    outst_nv_reqs--;
    if (vrp_ssms->status != EC_OK)
    {
        // TODO log some serious message here
        fprintf(stderr, "[%s:%d] SSDBG:SSMS Read from NVRAM failed\n", __func__, __LINE__);
        goto Abort_ssms_read;
    }

    // Verify the CRC for the SSMS
    crc = MSC_CRC32(ssms_nv, sizeof(SSMS_NV) - sizeof(ssms_nv->crc));
#ifdef M4_DEBUG_SS_CRC
    fprintf(stderr, "%s:%d: SSMS read CRC=%08x, computed CRC=%08x, vid=%d, ord=%d, sda=%lld\n",
            __func__, __LINE__,
            ssms_nv->crc, crc, nv_oger->ogr_vid, ordinal, nv_oger->ogr_sda);
#endif /* M4_DEBUG_SS_CRC */
    if (crc != ssms_nv->crc ||
        (ssms_nv->version != SS_NV_VERSION && ssms_nv->version != SS_NV_VERSION_GT2TB))
    {
        // TODO Log something here.
        fprintf(stderr, "%s: SSDBG:SSMS CRC/version failed - Expected=%08X: Got=%08X, "
            "vid=%d, ord=%d\n", __func__,
            crc, ssms_nv->crc, nv_oger->ogr_vid, ordinal);
        fprintf(stderr, "%s: SSDBG: OGER_NV: ordinal=%d, srcvid=%d, ssvid=%d, "
            "firstoger=%d, tailoger=%d, prev_tailoger=%d, "
            "ogercnt=%d, status=%d, version=%08X, prefowner=%08X\n",
            __func__, ssms_nv->ordinal, ssms_nv->srcvid, ssms_nv->ssvid,
            ssms_nv->firstoger, ssms_nv->tailoger,
            ssms_nv->prev_tailoger, ssms_nv->ogercnt, ssms_nv->status,
            ssms_nv->version, ssms_nv->prefowner);

        goto Abort_ssms_read;
    }

    // The following logic ensures that we actually have a source and ss vid.
    if (ssms_nv->srcvid >= MAX_VIRTUALS ||
        ssms_nv->ssvid >= MAX_VIRTUALS ||
        V_vddindx[ssms_nv->srcvid] == 0 ||
        V_vddindx[ssms_nv->ssvid] == 0)
    {
        goto Abort_ssms_read;
    }

    // Allocate an SSMS and fill in the values.
    ssms = p_MallocC(sizeof(SSMS), __FILE__, __LINE__);
    ssms->ssm_link = NULL;
    ssms->ssm_synchead = NULL;
    ssms->ssm_synctail = NULL;
    ssms->ssm_srcvid = ssms_nv->srcvid;
    ssms->ssm_ssvid = ssms_nv->ssvid;
    ssms->ssm_stat = ssms_nv->status;
    ssms->ssm_flags = 0;

    // Create a region map and copy all of the segment maps into it.
    UINT32 number_full_rm = gVDX.vdd[ssms->ssm_ssvid]->devCap >> 32;
    UINT32 remaining_rm = ((gVDX.vdd[ssms->ssm_ssvid]->devCap & 0xffffffffULL) + SEGSIZE - 1) / SEGSIZE;
    D_init_slrm(remaining_rm, number_full_rm, ssms);

    oger_to_read = ssms_nv->firstoger;

    // Read up all of the OGERs.
    if (oger_to_read != 0)
    {
        right_fork_count[pool_id][which_fork_count] = 1;
        read_oger_from_nv(oger_to_read, N_poger_table, nv_oger, pool_id,
                which_fork_count, ssms);
    }

    // Wait for forked tasks to complete -- note: it might complete before us, check first.
    while (right_fork_count[pool_id][which_fork_count] != 0)
    {
        // Set our task to "waiting for right OGER read tasks to complete".
        TaskSetMyState(PCB_SS_R_OGER_READS);
        // Wait for all right oger tasks to complete.
        TaskSwitch();
    }

    ssms->ssm_frstoger = N_poger_table[ssms_nv->firstoger];
    ssms->ssm_tailoger = N_poger_table[ssms_nv->tailoger];
    ssms->ssm_prev_tailoger = N_poger_table[ssms_nv->prev_tailoger];
    ssms->ssm_ogercnt = ssms_nv->ogercnt;
    ssms->ssm_ordinal = ssms_nv->ordinal;
    ssms->ssm_prefowner = ssms_nv->prefowner;
    cuml_ssms_oger_cnt[pool_id] += ssms->ssm_ogercnt;

    // Update the SCOR VID of VDD from SSMS
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    gVDX.vdd[ssms->ssm_ssvid]->scorVID = ssms->ssm_srcvid;

    // Fill in the entry in the ssms_table
    if (!DEF_ssms_table[ssms->ssm_ordinal +
                        (MAX_SNAPSHOT_COUNT * (ssms->ssm_prefowner & 0x1))])
    {
        DEF_ssms_table[ssms->ssm_ordinal +
                       (MAX_SNAPSHOT_COUNT * (ssms->ssm_prefowner & 0x1))] = (UINT32)ssms;
    }

    // Link the SSMS structs to the VDD's
    vdd = V_vddindx[ssms->ssm_ssvid];
    vdd->vd_incssms = ssms;

    vdd = V_vddindx[ssms->ssm_srcvid];
    if (vdd->vd_outssmstail)
    {
        vdd->vd_outssmstail->ssm_link = ssms;
    }
    else
    {
        vdd->vd_outssms = ssms;
    }
    vdd->vd_outssmstail = ssms;
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

  Abort_ssms_read:
    // Free the ILT, VRP, and SGL for the ssms
    s_Free(sgl_ssms, sizeof(SGL) + sizeof(SGL_DESC) + desc_ssms->len, __FILE__, __LINE__);
    vrp_ssms->pSGL = NULL;
    PM_RelILTVRPSGL(ilt_ssms, vrp_ssms);

    finished_with_SSMS_task(pool_id);
    // Update the percentage sparseness for this snapshot

    if (ssms)
    {
        if (ogers_read_per_ssms[pool_id][which_fork_count] == ssms->ssm_ogercnt)
        {
            fprintf(stderr, "SSDBG: Restored SSMS VID = %d\n", ssms->ssm_ssvid);

            // Set this snapshot VDD to operational if good
            vdd = V_vddindx[ssms->ssm_ssvid];
            if (vdd->status == SS_IOSUSPEND)
            {
                fprintf(stderr, "SSDBG:%s indicate SS vid %d to OP\n",
                    __func__, vdd->vid);
                BIT_SET(ssms->ssm_flags, ssm_flags_setop);
            }

            compute_ss_spareseness(ssms);
        }
        else
        {
            // The proper number of OGERs has not been restored for this SSMS so it must be set to inop.
            fprintf(stderr, "SSMS OGER restored count failed - Expected=%08X: Got=%08X", ssms->ssm_ogercnt,
                ogers_read_per_ssms[pool_id][which_fork_count]);
            ss_invalidate_snapshot(ssms->ssm_ssvid);
            remove_ssms_from_src_vdd(ssms, V_vddindx[ssms->ssm_srcvid]);
        }
    }
}                                       /* End of ss_read_ssms */


/**
******************************************************************************
**
**  @brief      Finds the next populated SSMS in the NV header map.
**
**  This function will search the SSMS map in an NV header for the ordinal of
**  the next populated SSMS.  If the end is reached -1 is returned.  If a 0xfffe
**  is passed in as the last ordinal then the search will take place from the
**  beginning of the map.
**
**  @return     Ordinal of the next SSMS or -1 if not found.
**
******************************************************************************
*/
static UINT16 find_next(SS_HEADER_NV *header, UINT16 last_ordinal)
{
    UINT16          test_ordinal;

    switch (last_ordinal)
    {
        case 0xFFFF:
            test_ordinal = 0;
            break;

        case MAX_SNAPSHOT_COUNT:
            test_ordinal = 0xffff;
            break;

        default:
            test_ordinal = last_ordinal + 1;
            break;
    }

    while (test_ordinal < MAX_SNAPSHOT_COUNT)
    {
        if (header->ord_map[test_ordinal])
        {
            // This SSMS is populated
            break;
        }
        test_ordinal++;
    }
    if (test_ordinal >= MAX_SNAPSHOT_COUNT)
    {
        test_ordinal = 0xffff;
    }

    return (test_ordinal);
}                                      /* End of find_next */


/**
******************************************************************************
**
**  @brief      Read and restore the SSMS from the NV (in the snapshot vdisk).
**
**  @param      header_nv   -- pointer to header for ss nv.
**  @param      nv_oger     -- First oger for snappool.
**  @param      pool_id     -- snappool 0 or 1 (for controller) -- from vid.
**  @param      poger_table -- memory to hold all possible ogers for a snappool.
**
**  @return     Number of entries read.
**
******************************************************************************
*/
static void restore_read_ssms(SS_HEADER_NV *header_nv, OGER *nv_oger,
                                OGER **N_poger_table, int pool_id)
{
    UINT16          ordinal;
    int             some_tasks_created = FALSE;

    // We need a unique counter for the right forks, for each SSMS -- initialize.
    g_which_fork_count[pool_id] = 0;

    // For each SSMS in the map retrieve the proper data.

    ordinal = 0xffff;
    while ((ordinal = find_next(header_nv, ordinal)) != 0xffff)
    {
// fprintf(stderr, "restore_read_ssms, ordinal=%d, num_ssms_read_tasks[%d]=%d\n", ordinal, pool_id, num_ssms_read_tasks[pool_id]);
        while (num_ssms_read_tasks[pool_id] == MAX_SSMS_TASKS_ALLOWED)
        {
// fprintf(stderr, "restore_read_ssms waiting for SSMS task to finish??\n");
            // Set our task to "waiting for a SSMS task to finish".
            TaskSetMyState(PCB_SS_SSMS_MORE_TASKS);
            // Wait for one task to complete.
            TaskSwitch();
        }

        // Increment number of read SSMS tasks outstanding.
        num_ssms_read_tasks[pool_id] += 1;

// fprintf(stderr, "restore_read_ssms creating task ss_read_ssms for task %d\n", ordinal);
        // Fork task to read the ssms entry.
        CT_fork_tmp = (unsigned long)"ss_read_ssms";
        TaskCreate6(C_label_referenced_in_i960asm(ss_read_ssms), K_xpcb->pc_pri,
                    ordinal, (int)nv_oger, pool_id, (UINT32)N_poger_table);
        some_tasks_created = TRUE;
// fprintf(stderr, "restore_read_ssms created task ss_read_ssms for ordinal %d\n", ordinal);
    }

    while (num_ssms_read_tasks[pool_id] != 0)
    {
// fprintf(stderr, "All tasks started, waiting ssms_read_tasks[%d]=%d\n", pool_id, num_ssms_read_tasks[pool_id]);
        // Set our task to "waiting for SSMS read tasks to complete".
        TaskSetMyState(PCB_SS_SSMS_READ);
        // Wait for all tasks to complete, knowing that we run before the last one starts.
        TaskSwitch();
    }
    fprintf(stderr, "All SSMS tasks finished!\n");

    if (g_duplicate_oger == TRUE)
    {
       fprintf(stderr, "SSDBG: Fatal error - Duplicate oger found, Invalidating all the snapshots\n");
       ss_invalidate_allsnapshots(nv_oger->ogr_vid);
       g_duplicate_oger = FALSE;
    }

    if (some_tasks_created)
    {
        // If some SSMS restores have been done then update the header just in case of error on any SSMS restore.
        update_header_nv(gnv_header_array[pool_id], nv_oger);
    }
}   /* End of restore_read_ssms */


static void ss_reset_tmpsuspend_allsnapshots(void)
{
    // Find each source VDD and make it operational if:
    VDD    *vdd;
    RDD    *rdd;
    int     i;

    for (i = 0; i < MAX_VIRTUALS; i++)
    {
        vdd = gVDX.vdd[i];
        if (!vdd || vdd->status != SS_IOSUSPEND)
        {
            continue;
        }

        rdd = vdd->pRDD;
        if (!rdd)
        {
            continue;
        }

        /* Move vdisks to Operational */

        if (rdd->type != RD_SLINKDEV)
        {
            fprintf(stderr, "SSDBG:%s moving SRC vid %d to OP\n",
                    __func__, vdd->vid);
            vdd->status = VD_OP;
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        }
        else if (vdd->vd_incssms &&
            BIT_TEST(vdd->vd_incssms->ssm_flags, ssm_flags_setop))
        {
            fprintf(stderr, "SSDBG:%s moving SS vid %d to OP\n",
                    __func__, i);
            vdd->status = VD_OP;
            BIT_CLEAR(vdd->vd_incssms->ssm_flags, ssm_flags_setop);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        }
        else
        {
            /* Invalidate the snapshot, SSMS CRC failed case */
            ss_invalidate_snapshot(i);
            if (vdd->vd_incssms)
            {
                remove_ssms_from_src_vdd(vdd->vd_incssms,
                        gVDX.vdd[vdd->vd_incssms->ssm_srcvid]);
            }
        }
    }
}   /* End of ss_reset_tmpsuspend_allsnapshots */



/**
******************************************************************************
**
**  @brief      Read the header from the NV (in the snapshot vdisk).
**
**  @return     0   - if no snapshots using this snappool.
**              1   - if snapshot crc is bad, and must invalidate all snapshots.
**             -1   - if everything is ok.
**
******************************************************************************
*/
static int ss_restore_validate_header(SS_HEADER_NV *header_nv, int pool_id, UINT16 spool_vid)
{
    SS_HEADER_NV   *local_header_nv;
    UINT32          crc;

    // Allocate a local NV header structure if needed.
    local_header_nv = gnv_header_array[pool_id];
    if (!local_header_nv)
    {
        local_header_nv = p_MallocC(sizeof(SS_HEADER_NV), __FILE__, __LINE__);
        gnv_header_array[pool_id] = local_header_nv;
    }

    // Check the crc of the header.
    crc = MSC_CRC32(header_nv, sizeof(SS_HEADER_NV) - sizeof(header_nv->crc));
    if (crc != header_nv->crc ||
        (header_nv->version != SS_NV_VERSION && header_nv->version != SS_NV_VERSION_GT2TB))
    {
        // TODO Log something here.
        fprintf(stderr, "SSDBG: %s: Bad header. CRC Read %08x, Exp %08x, version=%d\n",
                __func__, header_nv->crc, crc, header_nv->version);
        // SS_ERR_LOG Log message to CCB
        gss_version[pool_id] = 0;
        return (1);
    }
    gss_version[pool_id] = header_nv->version;

    // The CRC was good so update the local structure with the data from NV
    memcpy(local_header_nv, header_nv, sizeof(SS_HEADER_NV));

    // Check if any snapshots using this snappool.
    if (header_nv->header_magic == SS_HEADER_MAGIC)
    {
        fprintf(stderr, "SSDBG:%s No Snapshots created on this snappool %d\n",
                __func__, spool_vid);
        return (0);
    }

    return (-1);
}                                       /* End of ss_restore_validate_header */

/**
******************************************************************************
**
**  @brief      Restore all snapshot nv data
**
**  This function restores the entire contents of snapshot NV data.  New
**  structures are created as needed.
**
**  @return     Status -- 0 is good else vrp status code
**
******************************************************************************
*/
void restore_snap_data(UINT16 spool_vid)
{
    /*
     * This table is for pointers to OGERs that we read during restore.
     */
    OGER           **N_poger_table;

    SS_HEADER_NV   *header_nv;
    ILT            *ilt_header = NULL;
    VRP            *vrp_header;
    SGL            *sgl_header;
    SGL_DESC       *desc_header;

    UINT32          ret_val;
    OGER           *oger;

    OGER           *nv_oger;
    int             N_restored_ogers = 0;
    UINT32          i;
    UINT32          j;
    int             retry_count = SNAP_NVRETRY;
    int             pool_id = spool_vid & 0x1;

    VDD             *vdd = V_vddindx[spool_vid];
    UINT64          capacity = vdd->devCap;
    int             validate_header_check;

// This will temporarily invalidate Snapshots and source VDDs
    ss_tmpsuspend_allsnapshots(spool_vid);

    // No ogers read yet.
    cuml_ssms_oger_cnt[pool_id] = 0;
    // No ogers restored from SSMS reading.
    num_ogers_from_ssms[pool_id] = 0;

    // Allocate an OGER for the NV data if doesn't exist already
    if (!gnv_oger_array[pool_id])
    {
        oger = p_MallocC(sizeof(OGER), __FILE__, __LINE__);
        /* This is very interesting. It sets oger->ogr_sda to zero for the first one. Etc. */
        /* A hidden side effect of the p_MallocC and this assignment. */
        gnv_oger_array[pool_id] = oger;
        oger->ogr_vid = spool_vid;
    }

    nv_oger = gnv_oger_array[pool_id];
    if (nv_oger->ogr_vid != spool_vid)
    {
        fprintf(stderr, "ERROR?? restore_snap_data snappool vid[%d] %d, now %d\n", pool_id, nv_oger->ogr_vid, spool_vid);
    }

    do
    {
        // Read up the header
        ilt_header = alloc_request((UINT32)sizeof(SS_HEADER_NV), VRP_INPUT, nv_oger);
        vrp_header = (VRP *)ilt_header->ilt_normal.w0;
        sgl_header = vrp_header->pSGL;
        vrp_header->startDiskAddr = nv_oger->ogr_sda;

        desc_header = (SGL_DESC *)(sgl_header + 1);
        header_nv = (SS_HEADER_NV *)desc_header->addr;

        // Queue the request to virtual layer with wait
        outst_nv_reqs++;
        EnqueueILTW((void *)V_que, ilt_header);
        outst_nv_reqs--;
        ret_val = vrp_header->status;

        // Exit retry loop if everything is ok.
        if (ret_val == EC_OK)
        {
            break;
        }

        // Something wrong with reading header during this loop, retry.
        // Free the ILT, VRP, and SGL for the header (regotten on retry).
        s_Free(sgl_header, sizeof(SGL) + sizeof(SGL_DESC) + desc_header->len, __FILE__, __LINE__);
        vrp_header->pSGL = NULL;
        PM_RelILTVRPSGL(ilt_header, vrp_header);
        fprintf(stderr, "%s: Reading of Header NV failed, retries left = %d\n", __func__, retry_count);
        if (retry_count < SNAP_NVRETRY -1)
        {
            // If we have already retried twice, add more delay for each retry.
            // This will help let config propagations catch up.
            TaskSleepMS((SNAP_NVRETRY - retry_count)*50);
        }
    } while (--retry_count > 0);

    // If can not read header after retries, inop all snapshots on this snappool.
    if (ret_val != EC_OK)
    {
        goto Abort_header_read;
    }

// ---------------------------------------------------------------------------

    // Validate snappool header.
    validate_header_check = ss_restore_validate_header(header_nv, pool_id, spool_vid);

    // If no snapshots created on this snappool, exit.
    if (validate_header_check == 0)
    {
        goto Abort_header_processing;
    }

    // If crc bad.
    if (validate_header_check == 1)
    {
        ret_val = EC_CHECKSUM_ERR;
        update_header_nv_magic(nv_oger->ogr_vid);
        goto Abort_header_processing;
    }

// ---------------------------------------------------------------------------
    // Allocate table for reading all the OGERs.
// MAX_OGER_COUNT       -- 4* 2048=  8192
// MAX_OGER_COUNT_GT2TB -- 4*65536=262144
    UINT32 max_oger_count;
    if (gss_version[pool_id] != SS_NV_VERSION_GT2TB)
    {
        max_oger_count = MAX_OGER_COUNT;
    }
    else
    {
        max_oger_count = MAX_OGER_COUNT_GT2TB;
    }
    N_poger_table = (void *)p_MallocC(sizeof(OGER*) * max_oger_count, __FILE__, __LINE__);

    // Read and restore the ssms -- walks SSMS and reads the OGERs.
    restore_read_ssms(header_nv, nv_oger, N_poger_table, pool_id);

    UINT8 *N_dog_table = (UINT8 *)p_MallocC(max_oger_count / 8, __FILE__, __LINE__);
    N_dog_table[0] = 0x1;               /* Set snappool header as in use (first 1gb). */

    // Loop through all of the ogers and fill in the tree pointers.
#ifdef M4_OGERS
fprintf(stderr, "N_OGER tree pointers:\n");
#endif  /* M4_OGERS */
    for (i = 0; i < max_oger_count; i++)
    {
        if (N_poger_table[i])
        {
            BIT_SET(N_dog_table[i / 8], i % 8);
#ifdef M4_OGERS
fprintf(stderr, "   %d=> leftch=0x%x, rightch=0x%x, parent=0x%x, link=0x%x\n", i,
    (int)N_poger_table[i]->ogr_leftch, (int)N_poger_table[i]->ogr_rightch,
    (int)N_poger_table[i]->ogr_parent, (int)N_poger_table[i]->ogr_link);
#endif  /* M4_OGERS */
            N_poger_table[i]->ogr_leftch = N_poger_table[(int)N_poger_table[i]->ogr_leftch];
            N_poger_table[i]->ogr_rightch = N_poger_table[(int)N_poger_table[i]->ogr_rightch];
            N_poger_table[i]->ogr_parent = N_poger_table[(int)N_poger_table[i]->ogr_parent];
            N_poger_table[i]->ogr_link = N_poger_table[(int)N_poger_table[i]->ogr_link];
#ifdef M4_OGERS
fprintf(stderr, "   %d:> leftch=0x%x, rightch=0x%x, parent=0x%x, link=0x%x\n", i,
    (int)N_poger_table[i]->ogr_leftch, (int)N_poger_table[i]->ogr_rightch,
    (int)N_poger_table[i]->ogr_parent, (int)N_poger_table[i]->ogr_link);
#endif  /* M4_OGERS */
            N_restored_ogers++;
        }
    }
    fprintf(stderr, "N_restored_ogers=%d\n", N_restored_ogers);

    //
    // Now we need to set the trailing bits in the bitmap, those that are
    // beyond the length of the number of GB % 8 of snappool itself.
    //

    capacity = capacity >> SS_SEC2GB_SF;  // Convert sectors to GB (divide by 2048*1024).
    i = capacity % 8;           // Check for partial byte at end of dog_table.
    j = capacity / 8;           // Get which byte.

    if (i != 0)
    {
        N_dog_table[j] |= (0xff << i);
    }

// ---------------------------------------------------------------------------
//  Copy bitmap into dog_table.
    bcopy(N_dog_table, DEF_dog_table + (pool_id * (MAX_OGER_COUNT_GT2TB/ 8)), max_oger_count / 8);
#ifdef DUMP_OGER_BITMAP
dump_bitmap(N_dog_table, "restore_snap_data", spool_vid);
#endif  /* DUMP_OGER_BITMAP */

    // Get rid of allocated memory.
    p_Free(N_dog_table, max_oger_count/8, __FILE__, __LINE__);
    p_Free(N_poger_table, sizeof(OGER*) * max_oger_count, __FILE__, __LINE__);

    // Check to see that all SSMS added oger counts matches the actual count of OGERs restored.
    if (cuml_ssms_oger_cnt[pool_id] != N_restored_ogers)
    {
        // TODO find out which SSMS is missing OGERs, invalidate the SS, Log it, and free the OGERs.
        fprintf(stderr, "SSDBG: restore_all_ss_nv:  Holy SSMS! Expected %d, Got %d\n", cuml_ssms_oger_cnt[pool_id], N_restored_ogers);
    }
    fprintf(stderr, "restore_all_ss_nv:  Found %d OGERs\n", N_restored_ogers);
    DEF_oger_cnt[pool_id] = N_restored_ogers;

    if (vdd)
    {
        vdd->scpComp = (UINT8)((N_restored_ogers + (N_restored_ogers != 0)) * 100ULL / (vdd->devCap >> SS_SEC2GB_SF));
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    }

// ---------------------------------------------------------------------------
    // Now clean up the data structures (and do not invalidate the snapshots).
  Abort_header_processing:
    // Free the ILT, VRP, and SGL for the header
    s_Free(sgl_header, sizeof(SGL) + sizeof(SGL_DESC) + desc_header->len, __FILE__, __LINE__);
    vrp_header->pSGL = NULL;
    PM_RelILTVRPSGL(ilt_header, vrp_header);

  Abort_header_read:
    // Read header NV or read oger bitmap failed, invalidate all the snapshot VDDs.
    if (ret_val != EC_OK)
    {
        fprintf(stderr, "%s: Restore snap data encountered a fatal error in reading header/ogermap data\
               moving all the snapshots on the snappool %d to permanent INOP state\n", __func__, spool_vid);
        /* SS_ERR_LOG  log the message to CCB */
        ss_invalidate_allsnapshots(spool_vid);
    }

    // Reset IOSUSPEND state on all SS and SRC vdisks
    ss_reset_tmpsuspend_allsnapshots();
}                                      /* End of restore_snap_data */


/**
******************************************************************************
**
**  @brief      Updates the NV data for one OGER
**
**  This function will update the nonvolatile data pertaining to a specific OGER.
**
**  @return     Status -- 0 is good else vrp status code
**
******************************************************************************
*/
UINT32 update_oger_nv(OGER *oger, OGER *nv_oger)
{
    OGER_NV        *oger_nv;
    OGER_NV_GT2TB  *oger_nv_gt2tb;
    ILT            *ilt_oger = NULL;
    VRP            *vrp_oger;
    SGL            *sgl_oger;
    SGL_DESC       *desc_oger;
    UINT32          ret_val;
    int             retry_count = SNAP_NVRETRY;
    VDD            *vdd = NULL;

    do
    {
        // Allocate an OGER update request.
/* NOTE: sizeof(OGER_NV_GT2TB) <= sizeof(OGER_NV) */
        ilt_oger = alloc_request((UINT32)sizeof(OGER_NV), VRP_OUTPUT, nv_oger);
        vrp_oger = (VRP *)ilt_oger->ilt_normal.w0;
        sgl_oger = vrp_oger->pSGL;
        if (gss_version[oger->ogr_vid & 1] == SS_NV_VERSION_GT2TB)
        {
            vrp_oger->startDiskAddr = FIRST_OGER_GT2TB + ((oger->ogr_ord-1) * OGER_NV_ALLOC_GT2TB);
        }
        else
        {
            vrp_oger->startDiskAddr = FIRST_OGER + (oger->ogr_ord * OGER_NV_ALLOC);
        }
        desc_oger = (SGL_DESC *)(sgl_oger + 1);
        oger_nv = (OGER_NV *)desc_oger->addr;

        // Copy the data from the OGER to the OGER_NV
        oger_nv->vid = oger->ogr_vid;
        if (oger->ogr_leftch)
        {
            oger_nv->leftch = oger->ogr_leftch->ogr_ord;
        }
        else
        {
            oger_nv->leftch = 0;
        }
        if (oger->ogr_rightch)
        {
            oger_nv->rightch = oger->ogr_rightch->ogr_ord;
        }
        else
        {
            oger_nv->rightch = 0;
        }
        if (oger->ogr_parent)
        {
            oger_nv->parent = oger->ogr_parent->ogr_ord;
        }
        else
        {
            oger_nv->parent = 0;
        }
        oger_nv->ordinal = oger->ogr_ord;
        oger_nv->maxprobe = oger->ogr_maxpr;
        oger_nv->segcount = oger->ogr_segcnt;
        if (gss_version[oger->ogr_vid & 1] == SS_NV_VERSION_GT2TB)
        {
            oger_nv_gt2tb = (OGER_NV_GT2TB *)oger_nv;
            oger_nv_gt2tb->version = SS_NV_VERSION_GT2TB;
            oger_nv_gt2tb->status = oger->ogr_stat;
            oger_nv_gt2tb->sda = oger->ogr_sda;
            oger_nv_gt2tb->sdakey = oger->ogr_sdakey;
            memcpy(oger_nv_gt2tb->segfield, oger->ogr_segfld, SEGSPEROGER / 8);
            memcpy(oger_nv_gt2tb->sdamap, oger->ogr_sdamap, SEGSPEROGER * 4);
            if (oger->ogr_link)
            {
                oger_nv_gt2tb->next = oger->ogr_link->ogr_ord;
            }
            else
            {
                oger_nv_gt2tb->next = 0;
            }
        }
        else
        {
            oger_nv->version = SS_NV_VERSION;
            oger_nv->status = oger->ogr_stat;
            oger_nv->sda = oger->ogr_sda;
            oger_nv->sdakey = oger->ogr_sdakey;
            memcpy(oger_nv->segfield, oger->ogr_segfld, SEGSPEROGER / 8);
            memcpy(oger_nv->sdamap, oger->ogr_sdamap, SEGSPEROGER * 4);
            if (oger->ogr_link)
            {
                oger_nv->next = oger->ogr_link->ogr_ord;
            }
            else
            {
                oger_nv->next = 0;
            }
        }

        // Compute the CRC for the OGER_NV struct -- matches OGER_NV_GT2TB location.
        oger_nv->crc = MSC_CRC32(oger_nv, sizeof(OGER_NV) - sizeof(oger_nv->crc));

        // For Proactive busy handling purpose
        vdd = gVDX.vdd[vrp_oger->vid];
        // Queue the request to virtual layer with wait
        outst_nv_reqs++;
        EnqueueILTW((void *)V_que, ilt_oger);
        outst_nv_reqs--;
        ret_val = vrp_oger->status;

        // Free the ILT, VRP, and SGL for the header
        s_Free(sgl_oger, sizeof(SGL) + sizeof(SGL_DESC) + desc_oger->len, __FILE__, __LINE__);
        vrp_oger->pSGL = NULL;
        PM_RelILTVRPSGL(ilt_oger, vrp_oger);

        if (vdd && (BIT_TEST(vdd->attr, VD_BBEBUSY)))
        {
            fprintf(stderr,"<ISE-DEBUG>update_oger_nv ISE upgrade/busy - vdd status %x retval %x\n", vdd->status,ret_val);
            return(EC_BEBUSY);
        }

        if (ret_val == EC_OK)
        {
            break;
        }
        else
        {
            fprintf(stderr, "%s: update oger_nv failed retries remaining = %d, retstatus %x\n", __func__, retry_count,ret_val);
        }
    } while (--retry_count > 0);

    return (ret_val);
}                                      /* End of update_oger_nv */


/**
******************************************************************************
**
**  @brief   Moves the specified snapshot to permanent inop state
**
**  This function will invalidate the snapshot  and updates its operational
**  state and propagates across the controllers
**
**  @return    none
**
******************************************************************************
*/

void ss_invalidate_snapshot(UINT16 ss_vid)
{
    RDD            *rdd;
    PSD            *psd;
    VDD            *vdd;

    vdd = gVDX.vdd[ss_vid];
    if (vdd == NULL)
    {
        fprintf(stderr, "%s: Snapshot VDD is NULL\n", __func__);
        return;
    }

    fprintf(stderr, "%s: Moving Snapshot VID %d to permanent INOP\n", __func__, ss_vid);
    vdd->status = VD_INOP;
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

    rdd = vdd->pRDD;
    psd = *((PSD **)(rdd + 1));

    if (psd != NULL)
    {
        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        psd->status = PSD_INOP;
    }

    RB_setraidstat(rdd);
    RB_SetVirtStat();
    vdd = gVDX.vdd[ss_vid];
    if (vdd)
    {
        vdd->status = VD_INOP;
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    }

    if (BIT_TEST(K_ii.status, II_MASTER))
    {
        NV_P2UpdateConfig();
        NV_SendRefresh();
    }
    else
    {
        NV_P2Update();
    }
}                                      /* End of ss_invalidate_snapshot */

/**
******************************************************************************
**
**  @brief   Moves all the  snapshots related to a snappool to permanent
**           inop state
**
**  This function will invalidates all the snapshots related to a particular
**  snappool
**
**  @return    none
**
******************************************************************************
*/

void ss_invalidate_allsnapshots(UINT16 spool_vid)
{
    UINT16   pool_id = spool_vid&0x1;
    int      j = 0;
    RDD     *rdd = NULL;
    PSD     *psd = NULL;

    for (j = 0; j < MAX_VIRTUAL_DISKS; ++j)
    {
        if (gVDX.vdd[j] != NULL)
        {
            rdd = gVDX.vdd[j]->pRDD;
            if (rdd != NULL)
            {
                if ((rdd->type == RD_SLINKDEV) && (rdd->depth == pool_id))
                {
                    fprintf(stderr, "%s: Moving Snapshot VID %d to permanent INOP\n", __func__, j);
                    psd = *((PSD* *)(rdd + 1));
                    if (psd != NULL)
                    {
                        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                        psd->status = PSD_INOP;
                        RB_setraidstat(rdd);
                    }
                }
            }

         }/* End vdd != NULL */

    } /* End of for loop */

    RB_SetVirtStat();

    for (j = 0; j < MAX_VIRTUAL_DISKS; ++j)
    {
        if (!gVDX.vdd[j])
        {
            continue;
        }

        rdd = gVDX.vdd[j]->pRDD;
        if (!rdd || rdd->type != RD_SLINKDEV || rdd->depth != pool_id)
        {
            continue;
        }

        fprintf(stderr, "%s: Really moving Snapshot VID %d to permanent INOP\n",
                __func__, j);
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        gVDX.vdd[j]->status = VD_INOP;
    }

    if (BIT_TEST(K_ii.status, II_MASTER))
    {
        NV_P2UpdateConfig();
        NV_SendRefresh();
    }
    else
    {
        NV_P2Update();
    }
}                                      /* End of ss_invalidate_allsnapshots */

/**
******************************************************************************
**
**  @brief      Updates the NV data for one SSMS
**
**  This function will update the nonvolatile data pertaining to a specific
**  SSMS.
**
**  @return     Status -- 0 is good else vrp status code
**
******************************************************************************
*/
UINT32 update_ssms_nv(SSMS *ssms, OGER *nv_oger)
{
    SSMS_NV        *ssms_nv;
    ILT            *ilt_ssms = NULL;
    VRP            *vrp_ssms;
    SGL            *sgl_ssms;
    SGL_DESC       *desc_ssms;
    UINT32          ret_val;
    int             retry_count = SNAP_NVRETRY;
    VDD             *vdd = NULL;

    do
    {
        // Allocate a SSMS update request.
        ilt_ssms = alloc_request((UINT32)sizeof(SSMS_NV), VRP_OUTPUT, nv_oger);
        vrp_ssms = (VRP *)ilt_ssms->ilt_normal.w0;
        sgl_ssms = vrp_ssms->pSGL;
        if (gss_version[nv_oger->ogr_vid & 1] == SS_NV_VERSION_GT2TB)
        {
            vrp_ssms->startDiskAddr = nv_oger->ogr_sda + FIRST_SSMS_GT2TB + (ssms->ssm_ordinal * SSMS_NV_ALLOC_GT2TB);
        }
        else
        {
            vrp_ssms->startDiskAddr = nv_oger->ogr_sda + FIRST_SSMS + (ssms->ssm_ordinal * SSMS_NV_ALLOC);
        }
        desc_ssms = (SGL_DESC *)(sgl_ssms + 1);
        ssms_nv = (SSMS_NV *)desc_ssms->addr;

        /* Copy the source vid of the snapshot */
        ssms->ssm_srcvid = gVDX.vdd[ssms->ssm_ssvid]->scorVID;
        // Copy the data from the SSMS to the SSMS_NV
        ssms_nv->srcvid = ssms->ssm_srcvid;
        if (gss_version[nv_oger->ogr_vid & 1] == SS_NV_VERSION_GT2TB)
        {
            ssms_nv->version = SS_NV_VERSION_GT2TB;
        }
        else
        {
            ssms_nv->version = SS_NV_VERSION;
        }
        ssms_nv->ssvid = ssms->ssm_ssvid;
        ssms_nv->status = ssms->ssm_stat;
        ssms_nv->ordinal = ssms->ssm_ordinal;
        ssms_nv->prefowner = ssms->ssm_prefowner;

        if (ssms->ssm_frstoger)
        {
            ssms_nv->firstoger = ssms->ssm_frstoger->ogr_ord;
        }
        else
        {
            ssms_nv->firstoger = 0;
        }

        if (ssms->ssm_tailoger)
        {
            ssms_nv->tailoger = ssms->ssm_tailoger->ogr_ord;
        }
        else
        {
            ssms_nv->tailoger = 0;
        }

        if (ssms->ssm_prev_tailoger)
        {
            ssms_nv->prev_tailoger = ssms->ssm_prev_tailoger->ogr_ord;
        }
        else
        {
            ssms_nv->prev_tailoger = 0;
        }

        ssms_nv->ogercnt = ssms->ssm_ogercnt;

        // Compute the CRC for the SSMS_NV struct
        ssms_nv->crc = MSC_CRC32(ssms_nv, sizeof(SSMS_NV) - sizeof(ssms_nv->crc));
#ifdef M4_DEBUG_SS_CRC
    fprintf(stderr, "%s:%d: SSMS update CRC=%08x, vid=%d, ord=%d, sda=%lld\n",
            __func__, __LINE__,
            ssms_nv->crc, nv_oger->ogr_vid, ssms->ssm_ordinal, nv_oger->ogr_sda);
#endif /* M4_DEBUG_SS_CRC */

        // For ise proactive busy purpose
        vdd = gVDX.vdd[vrp_ssms->vid];
        // Queue the request to virtual layer with wait
        outst_nv_reqs++;
        EnqueueILTW((void *)V_que, ilt_ssms);
        outst_nv_reqs--;
#ifdef M4_DEBUG_SS_CRC
    fprintf(stderr, "%s:%d: SSMS update DONE CRC=%08x, vid=%d, ord=%d, sda=%lld\n",
            __func__, __LINE__,
            ssms_nv->crc, nv_oger->ogr_vid, ssms->ssm_ordinal, nv_oger->ogr_sda);
#endif /* M4_DEBUG_SS_CRC */

        ret_val = vrp_ssms->status;

        // Free the ILT, VRP, and SGL for the header
        s_Free(sgl_ssms, sizeof(SGL) + sizeof(SGL_DESC) + desc_ssms->len, __FILE__, __LINE__);
        vrp_ssms->pSGL = NULL;
        PM_RelILTVRPSGL(ilt_ssms, vrp_ssms);
        if (vdd && (BIT_TEST(vdd->attr, VD_BBEBUSY)))
        {
            fprintf(stderr,"<ISE-DEBUG>update_ssms_nv ISE upgrading/busy vdd status %x retval %x\n", vdd->status,ret_val);
            return(EC_BEBUSY);
        }

        if (ret_val == EC_OK)
        {
            break;
        }
        else
        {
            fprintf(stderr, "%s: update ssms_nv failed retries remaining = %d, retvalue %x\n", __func__, retry_count,ret_val);
        }
    } while (--retry_count > 0);

    return (ret_val);
}                                      /* End of update_ssms_nv */

/**
******************************************************************************
**
**  @brief      Add a new SSMS to the NV data
**
**  This function accepts a pre-allocated SSMS and stores it's information to
**  NV data.
**
**  @return     Status -- 0 is good else vrp status code
**
******************************************************************************
*/
UINT32 add_ssms_nv(SSMS *ssms, OGER *nv_oger)
{
    UINT32          ret_val;

    if (ssms == NULL)
    {
        fprintf(stderr, "%s:Received NULL SSMS\n", __func__);
        return (EC_INV_DEV);
    }

    ret_val = update_ssms_nv(ssms, nv_oger);

    if (ret_val == EC_OK)
    {
        ret_val = update_header_nv(gnv_header_array[nv_oger->ogr_vid & 0x1], nv_oger);
    }
    return (ret_val);
}                                      /* End of add_ssms_nv */


/**
******************************************************************************
**
**  @brief      Updates the NV data for the header
**
**  This function will update the nonvolatile data pertaining to a specific
**  header.
**
**  @return     Status -- 0 is good else vrp status code
**
******************************************************************************
*/
UINT32 update_header_nv(SS_HEADER_NV *src_header_nv, OGER *nv_oger)
{
    SS_HEADER_NV   *dest_header_nv;
    ILT            *ilt_header = NULL;
    VRP            *vrp_header;
    SGL            *sgl_header;
    SGL_DESC       *desc_header;
    UINT32          ret_val;
    UINT32          ssms_count = 0;
    UINT16          i;
    int             retry_count = SNAP_NVRETRY;

    if (src_header_nv == NULL)
    {
        fprintf(stderr, "%s: src_header_nv is NULL\n", __func__);
        return (EC_NONX_DEV);
    }
    if ((gVDX.vdd[src_header_nv->spool_vid] == NULL))
    {
        fprintf(stderr, "%s: SnapPool Vdisk is NULL(deleted) %d\n", __func__, src_header_nv->spool_vid);
        return (EC_NONX_DEV);
    }

    do
    {
        // Allocate a SSMS update request.
        ilt_header = alloc_request((UINT32)sizeof(SS_HEADER_NV), VRP_OUTPUT, nv_oger);
        vrp_header = (VRP *)ilt_header->ilt_normal.w0;
        sgl_header = vrp_header->pSGL;
        vrp_header->startDiskAddr = nv_oger->ogr_sda + HEADER_OFFSET;
        desc_header = (SGL_DESC *)(sgl_header + 1);
        dest_header_nv = (SS_HEADER_NV *)desc_header->addr;

        // Update the local NV header with the information from the ram structs.
        src_header_nv->header_magic = SS_HEADER_MAGIC;
        src_header_nv->spool_vid = nv_oger->ogr_vid;
        switch (gss_version[nv_oger->ogr_vid & 1])
        {
            case SS_NV_VERSION:
                src_header_nv->version = SS_NV_VERSION;
                src_header_nv->ssms_offset = FIRST_SSMS;
                src_header_nv->oger_offset = FIRST_OGER;
                break;

            default:
                src_header_nv->version = SS_NV_VERSION_GT2TB;
                src_header_nv->ssms_offset = FIRST_SSMS_GT2TB;
                src_header_nv->oger_offset = FIRST_OGER_GT2TB;
        }
        gss_version[nv_oger->ogr_vid & 1] = src_header_nv->version;
        src_header_nv->length = sizeof(SS_HEADER_NV);

        for (i = 0; i < MAX_SNAPSHOT_COUNT; i++)
        {
            if (DEF_ssms_table[i + (MAX_SNAPSHOT_COUNT * (src_header_nv->spool_vid & 0x1))])
            {
                src_header_nv->ord_map[i] = 0xff;
                ssms_count++;
            }
            else
            {
                src_header_nv->ord_map[i] = 0x00;
            }
        }

        src_header_nv->ssms_count = ssms_count;
        if (ssms_count)
        {
            src_header_nv->header_magic = 0;    /* Indicate snapshots exist */
        }

        // Copy the data from the SSMS to the SSMS_NV
        memcpy(dest_header_nv, src_header_nv, sizeof(SS_HEADER_NV));

        // Compute the CRC for the SSMS_NV struct
        dest_header_nv->crc = MSC_CRC32(dest_header_nv, sizeof(SS_HEADER_NV) - sizeof(dest_header_nv->crc));

        // Queue the request to virtual layer with wait
        outst_nv_reqs++;
        EnqueueILTW((void *)V_que, ilt_header);
        outst_nv_reqs--;

        ret_val = vrp_header->status;

        // Free the ILT, VRP, and SGL for the header
        s_Free(sgl_header, sizeof(SGL) + sizeof(SGL_DESC) + desc_header->len, __FILE__, __LINE__);
        vrp_header->pSGL = NULL;
        PM_RelILTVRPSGL(ilt_header, vrp_header);

        if (ret_val == EC_OK)
        {
            break;
        }
        else
        {
            fprintf(stderr, "%s: update header failed retries remaining = %d\n", __func__, retry_count);
        }
    } while (--retry_count > 0);

    return (ret_val);
}                                      /* End of update_header_nv */

/**
******************************************************************************
**
**  @brief      Updates the NV data for the header with Magic number
**
**  This function will update the nonvolatile data pertaining to a specific
**  header.
**
**  @return     Status -- 0 is good else vrp status code
**
******************************************************************************
*/

UINT32 update_header_nv_magic(UINT16 spool_vid)
{
    SS_HEADER_NV   *dest_header_nv;
    ILT            *ilt_header = NULL;
    VRP            *vrp_header;
    SGL            *sgl_header;
    SGL_DESC       *desc_header;
    SGL_DESC       *desc;
    UINT32          ret_val;
    UINT32          buffer_size;
    UINT32          aligned_size;
    int             retry_count = SNAP_NVRETRY;

    do
    {
        buffer_size = sizeof(SS_HEADER_NV);

        // Allocate an ILT and VRP
        ilt_header = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, ilt_header);
#endif /* M4_DEBUG_ILT */
        vrp_header = get_vrp();
#ifdef M4_DEBUG_VRP
CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, vrp_header);
#endif /* M4_DEBUG_VRP */

        // Allocate an SGL for the data
        aligned_size = 512 * (buffer_size / 512);
        aligned_size += (buffer_size % 512) ? 512 : 0;
        sgl_header = s_MallocC(sizeof(SGL) + sizeof(SGL_DESC) + aligned_size, __FILE__, __LINE__);
        desc = (SGL_DESC *)(sgl_header + 1);

        ilt_header->ilt_normal.w0 = (UINT32)vrp_header;
        memset(vrp_header, 0, sizeof(VRP));

        // Fill in the SGL information
        sgl_header->scnt = 1;
        sgl_header->owners = 1;
        sgl_header->flag = 0;
        sgl_header->size = sizeof(SGL) + sizeof(SGL_DESC);
        desc->addr = (void *)(desc + 1);
        desc->len = aligned_size;

        // Update VRP information
        vrp_header->pSGL = sgl_header;
        vrp_header->sglSize = sgl_header->size;
        vrp_header->function = VRP_OUTPUT;
        vrp_header->vid = spool_vid;
        vrp_header->length = aligned_size / 512;
        vrp_header->startDiskAddr = HEADER_OFFSET;

        desc_header = (SGL_DESC *)(sgl_header + 1);
        dest_header_nv = (SS_HEADER_NV *)desc_header->addr;

        // Update the local NV header with the information from the ram structs.
        dest_header_nv->header_magic = SS_HEADER_MAGIC;

        switch (gss_version[spool_vid & 1])
        {
            case SS_NV_VERSION:
                dest_header_nv->version = SS_NV_VERSION;
                dest_header_nv->ssms_offset = FIRST_SSMS;
                dest_header_nv->oger_offset = FIRST_OGER;
                break;

            default:
                dest_header_nv->version = SS_NV_VERSION_GT2TB;
                dest_header_nv->ssms_offset = FIRST_SSMS_GT2TB;
                dest_header_nv->oger_offset = FIRST_OGER_GT2TB;
        }
        gss_version[spool_vid & 1] = dest_header_nv->version;
        dest_header_nv->spool_vid = spool_vid;
        dest_header_nv->length = sizeof(SS_HEADER_NV);

        // Compute the CRC for the SSMS_NV struct
        dest_header_nv->crc = MSC_CRC32(dest_header_nv, sizeof(SS_HEADER_NV) - sizeof(dest_header_nv->crc));

        // Queue the request to virtual layer with wait
        outst_nv_reqs++;
        EnqueueILTW((void *)V_que, ilt_header);
        outst_nv_reqs--;

        ret_val = vrp_header->status;

        // Free the ILT, VRP, and SGL for the header
        s_Free(sgl_header, sizeof(SGL) + sizeof(SGL_DESC) + desc_header->len, __FILE__, __LINE__);
        vrp_header->pSGL = NULL;
        PM_RelILTVRPSGL(ilt_header, vrp_header);

        if (ret_val == EC_OK)
        {
            fprintf(stderr, "SSDBG: Magic header write successful\n");
            break;
        }
        else
        {
            fprintf(stderr, "%s: SSDBG: update header Magic failed retries remaining = %d\n", __func__, retry_count);
        }
    } while (--retry_count > 0);

    return (ret_val);
}                                      /* End of update_header_nv_magic */


/**
******************************************************************************
**
**  @brief      Allocate an ILT/VRP/SGL for a vdisk IO request.
**
**  This function will allocate an ILT, VRP, and SGL of specified size.  Some
**  of the VRP parameters are initialized.
**
**  @return     Pointer to the new ILT or NULL if unsuccessful.
**
******************************************************************************
*/
ILT            *alloc_request(UINT32 buffer_size, UINT16 function, OGER *nv_oger)
{
    ILT            *ilt = NULL;
    VRP            *vrp;
    SGL            *sgl;
    SGL_DESC       *desc;
    UINT32          aligned_size;

    // Allocate an ILT and VRP
    ilt = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    vrp = get_vrp();
#ifdef M4_DEBUG_VRP
CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, vrp);
#endif /* M4_DEBUG_VRP */

    // Allocate an SGL for the data
    aligned_size = 512 * (buffer_size / 512);
    aligned_size += (buffer_size % 512) ? 512 : 0;
    sgl = s_MallocC(sizeof(SGL) + sizeof(SGL_DESC) + aligned_size, __FILE__, __LINE__);
    desc = (SGL_DESC *)(sgl + 1);

    ilt->ilt_normal.w0 = (UINT32)vrp;
    memset(vrp, 0, sizeof(VRP));

    // Fill in the SGL information
    sgl->scnt = 1;
    sgl->owners = 1;
    sgl->flag = 0;
    sgl->size = sizeof(SGL) + sizeof(SGL_DESC);
    desc->addr = (void *)(desc + 1);
    desc->len = aligned_size;

    // Update VRP information
    vrp->pSGL = sgl;
    vrp->sglSize = sgl->size;
    vrp->function = function;
    vrp->vid = nv_oger->ogr_vid;
    vrp->length = aligned_size / 512;

    return (ilt);
}                                      /* End of alloc_request */


/**
******************************************************************************
**
**  @brief      Read a specific OGER_NV from the NV data.
**
**  This function will read a specific OGER_NV into a new volatile structure.
**
**  @return     Pointer to the new OGER or NULL if unsuccessful.
**
******************************************************************************
*/
static OGER_NV  *read_oger_nv(UINT16 ordinal, ILT *ilt, OGER *nv_oger)
{
    VRP            *vrp;
    OGER_NV        *oger_nv;
    SGL_DESC       *desc;
    UINT32          crc;
    int             retry_count = SNAP_NVRETRY;
    UINT32          ret_val;

    do
    {
        vrp = (VRP *)ilt->ilt_normal.w0;
        if (gss_version[nv_oger->ogr_vid & 1] == SS_NV_VERSION_GT2TB)
        {
            vrp->startDiskAddr = FIRST_OGER_GT2TB + ((ordinal-1) * OGER_NV_ALLOC_GT2TB);
        }
        else
        {
            vrp->startDiskAddr = FIRST_OGER + (ordinal * OGER_NV_ALLOC);
        }
        vrp->function = VRP_INPUT;
        vrp->vid = nv_oger->ogr_vid;

        if ((UINT32)vrp->pSGL == 0xfeedf00d)
        {
            fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
            abort();
        }
        desc = (SGL_DESC *)((SGL *)(vrp->pSGL) + 1);
        oger_nv = (OGER_NV *)desc->addr;

        outst_nv_reqs++;
        EnqueueILTW((void *)V_que, ilt);
        outst_nv_reqs--;
        ret_val = vrp->status;

        if (ret_val == EC_OK)
        {
            // Compute the CRC for the OGER_NV struct
            crc = MSC_CRC32(oger_nv, sizeof(OGER_NV) - sizeof(oger_nv->crc));
            if (crc != oger_nv->crc ||
                (oger_nv->version != SS_NV_VERSION && oger_nv->version != SS_NV_VERSION_GT2TB))
            {
                // TODO log something here
                fprintf(stderr, "%s: OGER_NV CRC/version check failed "
                        "size %d, CRC=%08X expected %08x, version=%d\n",
                        __func__, sizeof(OGER_NV), crc, oger_nv->crc,
                        oger_nv->version);
                oger_nv = NULL;
            }
            //  Read successful break the loop
            break;
        }
        else
        {
            fprintf(stderr, "%s: OGER_NV Read failed, retries left = %d.\n", __func__, retry_count);
        }

    } while (--retry_count > 0);

    if (retry_count == 0)
    {
        fprintf(stderr, "%s: OGER_NV Read failed after retries.\n", __func__);
        oger_nv = NULL;
    }

    return (oger_nv);
}                                      /* End of read_oger_nv */


/**
******************************************************************************
**
**  @brief      Find the owning controller WRT a snappool.
**
**  This function will find the controller owner the proper snappool to use
**  for the Vid that is passed in.
**
**  @return     0, 1, or 0xffff when no snappool found.
**
******************************************************************************
*/
UINT16 find_owning_dcn(UINT16 vid)
{
    UINT16          tid;
    UINT16          nowner = 0xffff;
    UINT16          towner;
    VDD            *pVDD;
    UINT16          snappool_present[2];
    int             i;

    pVDD = V_vddindx[vid];
    snappool_present[0] = 0;
    snappool_present[1] = 0;

    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        if (gVDX.vdd[i] != NULL)
        {
            if (BIT_TEST(gVDX.vdd[i]->attr, VD_BSNAPPOOL))
            {
                snappool_present[i & 1] = 1;
            }
        }
    }

    if (!snappool_present[0] && !snappool_present[1])
    {
        // If no snappool is defined then return so
        return (nowner);
    }

    // First check to see if this source is assigned to a server.  If so,
    // then make sure this controller owns the appropriate snappool.  If
    // this controller doesn't own the appropriate snappool then return f's.
    // Be sure to check the owner of the copy tree if this source is the
    // destination of a mirror.
    for(;;)
    {
        tid = apool_get_tid(pVDD);
        if (tid != 0xffff)
        {
            if (T_tgdindx[tid])
            {
                // The source is assigned to a server so get the controller
                // number of the preferred owner.
                towner = T_tgdindx[tid]->prefOwner & 0x1;
                if (!snappool_present[towner])
                {
                    return (nowner);
                }
                if (gnv_oger_array[towner])
                {
                    return (towner);
                }
                // The other controller owns it (if it has a snappool).
                if (snappool_present[DL_ExtractDCN(K_ficb->cSerial) ^ 1])
                {
                    towner = DL_ExtractDCN(K_ficb->cSerial) ^ 1;
                }
                else
                {
                    towner = 0xffff;
                }
                return (towner);
            }
            fprintf(stderr, "%s%s:%u Server for vdisk %d exists, but T_tgdindx[%d] pointer NULL.\n",
                    FEBEMESSAGE, __func__, __LINE__, pVDD->vid, tid);
            break;
        }
        else if (pVDD->pDCD)
        {
            // This is the destination of a copy so test it's source
            pVDD = pVDD->pDCD->cor->srcvdd;
            if (pVDD)
            {
                continue;
            }
        }
        else
        {
            break;
        }
    }

    // Next see if this controller is the master and it has it's own snappool.
    // If so then return this controller as owner.
    if (BIT_TEST(K_ii.status, II_MASTER))
    {
        // This is the master.
        towner = DL_ExtractDCN(K_ficb->cSerial);
        if (gnv_oger_array[towner])
        {
            // This controller has a snappool so it is the owner
            return (towner);
        }
        else if (BIT_TEST(K_ii.status, II_CCBREQ))
        {
            // The other controller is up so it has to be the owner
            towner = DL_ExtractDCN(K_ficb->cSerial) ^ 1;
            return (towner);
        }
    }

    // Now check to see if the other controller has a snappool but
    // this controller has aquired ownership of it.
    if (gnv_oger_array[DL_ExtractDCN(K_ficb->cSerial) ^ 1])
    {
        // The other controller has a snappool
        towner = DL_ExtractDCN(K_ficb->cSerial) ^ 1;
        return (towner);
    }
    // Lastly, see if this controller has a snappool that can be used.
    else if (gnv_oger_array[DL_ExtractDCN(K_ficb->cSerial)])
    {
        // This controller is not the master but it has a snappool
        towner = DL_ExtractDCN(K_ficb->cSerial);
        return (towner);
    }

    return (nowner);
}                                      /* End of find_owning_dcn */


/**
******************************************************************************
**
**  @brief      Starts the Snapshot restore task if it is not running.
**
**  @param      spool_vid
**
**  @return     none
**
******************************************************************************
**/
void restore_all_ss_nv()
{
    if (ss_restore_pcb == NULL)
    {
        /*
         ** First indicate that we need to reject incoming requests to the snappool
         ** even though the structure isn't initialized yet.
         */
//        fprintf(stderr, "ssnv restore pcb is null spawning the task\n");
        CT_fork_tmp = (unsigned long)"restore_ss_nv_task";
        ss_restore_pcb = (PCB *)-1;     /* Flag taskcreate started. */
        ss_restore_pcb = TaskCreate2(C_label_referenced_in_i960asm(restore_ss_nv_task),
                                     SS_RESTORE_PRI);
    }
}                                      /* End of restore_all_ss_nv */


/**
******************************************************************************
**
**  @brief      Determines if my_dcn should own a snappool with preferred
**              owner of test_dcn.
**
**  @param      my_dcn
**
**  @param      test_dcn
**
**  @return     TRUE/FASLE
**
**  @attention  *** NOTE *** This code does not cover the case that a
**              controller has failed and the surviving controller is rebooted
**              when at least one of the controllers doesn't have any targets
**              associated.  It would be ideal to know which Vports this
**              controller currently owns.  I couldn't find that information.
**
******************************************************************************
**/
int i_should_own_sp(int my_dcn, int test_dcn)
{
    int test_dcn_has_targets = FALSE;
    int k;
    int ret_val = FALSE;

    // Look through the target list to see if my_dcn controller owns
    // targets with preferred owner of test_dcn controller.
    for (k=0;k< MAX_TARGETS;k++)
    {
        // For each target...
        if (T_tgdindx[k] != NULL)
        {
            if ((int)DL_ExtractDCN(T_tgdindx[k]->prefOwner) == test_dcn)
            {
                // The test_dcn controller is the preffered owner of this target.
                test_dcn_has_targets = TRUE;
                if ((int)DL_ExtractDCN(T_tgdindx[k]->owner) == my_dcn)
                {
                    // This controller currently owns this target.
                    ret_val = TRUE;
                }
            }
        }
    }

    if (!test_dcn_has_targets && (my_dcn == test_dcn))
    {
        // This controller has no targets (vdisks associated to servers).
        ret_val = TRUE;
    }

    return(ret_val);
}   /* End of i_should_own_sp */


/**
******************************************************************************
**
**  @brief     Check  ownership and restore the  snapshot nv data
**
**  This function does the following two tasks
**
**  a) Owner-ship checking
** This will involve:
**   1)  Check for snappool(s) and determine if this controller needs to own them.
**   2)  If this controller is owner then restore from NV if not already done.
**   3)  For each snapshot that is newly aquired restore the SSMS from NV.
**
**  b) Restore snapshot related NV data
**
******************************************************************************
*/
void restore_ss_nv_task(void)
{
    RDD            *rdd;
    UINT32          i;

    UINT8           got[2] = { 0, 0 };

    // Wait until the virtual layer is online
    while (V_exec_qu.pcb == NULL)
    {
        TaskSleepMS(100);
    }

    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        if (gVDX.vdd[i] != NULL)
        {
            if (BIT_TEST(gVDX.vdd[i]->attr, VD_BSNAPPOOL))
            {
                got[i & 0x1] = 1;
                if (gVDX.vdd[i]->owner == DL_ExtractDCN(K_ficb->cSerial))
                {
                    if (!gnv_oger_array[i & 0x1])
                    {
                        restore_snap_data(i);
                    }
                }
            }
        }
    }

    /* This section of code will handle other cases of snappool and snapshot creation and disowning. */
    if (!got[0] && gnv_oger_array[0])
    {
        // The first snap pool is now just a vdisk
        fprintf(stderr, "SSDBG:The first snappool is now just a vdisk\n");
        clear_all_ss_nv(gnv_oger_array[0]);
    }
    else if (got[0] &&
             gnv_oger_array[0] &&
             (gVDX.vdd[gnv_oger_array[0]->ogr_vid]->owner == (DL_ExtractDCN(K_ficb->cSerial)^1)))
    {
        // The first snappool has disappeared so disown it
        fprintf(stderr, "SSDBG:the first snappool disappeared disown it\n");
        // Fork task to cleanup the local structures.
        CT_fork_tmp = (unsigned long)"ss_disown_snappool";
        TaskCreate3(C_label_referenced_in_i960asm(ss_disown_snappool), K_xpcb->pc_pri, 0);
    }

    if (!got[1] && gnv_oger_array[1])
    {
        // The second snappool is now just a vdisk
        fprintf(stderr, "SSDBG:The second snappool is now just a vdisk\n");
        clear_all_ss_nv(gnv_oger_array[1]);
        // The snappool is just a vdisk, clear the global dog_table
    }
    else if (got[1] &&
             gnv_oger_array[1] &&
             (gVDX.vdd[gnv_oger_array[1]->ogr_vid]->owner == (DL_ExtractDCN(K_ficb->cSerial)^1)))
    {
        // The second snap pool has disappeared so disown it
        fprintf(stderr, "SSDBG:the second snappool disappeared disown it\n");
        // Fork task to cleanup the local structures.
        CT_fork_tmp = (unsigned long)"ss_disown_snappool";
        TaskCreate3(C_label_referenced_in_i960asm(ss_disown_snappool), K_xpcb->pc_pri, 1);
    }

    if (got[1] || got[0])
    {
        /* Find all snapshots and restore them if needed */
        int             j;

        for (j = 0; j < MAX_VIRTUAL_DISKS; ++j)
        {
            if (gVDX.vdd[j] != NULL)
            {
                rdd = gVDX.vdd[j]->pRDD;
                if (rdd != NULL)
                {
                    if (rdd->type == RD_SLINKDEV)
                    {
                        // NOTE: the rdd depth field is used to store the preferred
                        // owning controller.
                        if (gnv_oger_array[rdd->depth])
                        {
                            // Check to see if this is a newly created snapshot or aquiring on failover
                            if ((rdd->depth != DL_ExtractDCN(K_ficb->cSerial)) &&
                                ((gVDX.vdd[j]->status == VD_OP) ||
                                 (gVDX.vdd[j]->status == VD_SCHED || gVDX.vdd[j]->status == VD_INOP)))
                            {
                                // This is a case where this controller is owning the other controllers
                                // snappool and a snapshot has been newly created.  Any existing snapshots
                                // would have already been restored when the snappool ownership was aquired above.
                                // NOTE Even if a snapshot is inop we need to construct data struct for it such
                                // that it can be deleted at some point.  This will free up the OGERs that could be
                                // restored.
                                if (gVDX.vdd[j]->vd_incssms == NULL)
                                {
                                    gVDX.vdd[j]->vd_incssms = DEF_build_slink_structures(gVDX.vdd[j]->devCap,
                                                                                         (UINT32)gVDX.vdd[j]->vid,
                                                                                         (UINT32)rdd->sps);
                                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);/* Flag vdisk data has changed */
                                    if (gVDX.vdd[j]->status == VD_INOP)
                                    {
                                        // Could not build snapshot structures
                                        fprintf(stderr, "[%s:%d] Snapshot (NV Oger update) failed, \
                                                Moving  Snapshot VID %d  to Permanent INOP state\n",
                                                __func__, __LINE__, gVDX.vdd[j]->vid);
                                        ss_invalidate_snapshot(j);
                                        if (gVDX.vdd[j]->vd_incssms)
                                        {
                                            remove_ssms_from_src_vdd(gVDX.vdd[j]->vd_incssms,
                                                gVDX.vdd[gVDX.vdd[j]->vd_incssms->ssm_srcvid]);
                                        }
                                    }
                                    else
                                    {
                                        if (add_ssms_nv(gVDX.vdd[j]->vd_incssms, gnv_oger_array[rdd->depth]) != EC_OK)
                                        {
                                            fprintf(stderr, "[%s:%d] Snapshot (NV SSMS Update) failed, \
                                                     Moving Snapshot VID %d  to Permanent INOP state\n",
                                                    __func__, __LINE__, gVDX.vdd[j]->vid);
                                            ss_invalidate_snapshot(j);
                                            if (gVDX.vdd[j]->vd_incssms)
                                            {
                                                remove_ssms_from_src_vdd(gVDX.vdd[j]->vd_incssms,
                                                    gVDX.vdd[gVDX.vdd[j]->vd_incssms->ssm_srcvid]);
                                            }
                                        }
                                    }
                                }
                            }
                            else if ((gVDX.vdd[j]->vd_incssms == NULL) &&
                                     ((gVDX.vdd[j]->status == VD_OP) ||
                                      (gVDX.vdd[j]->status == VD_SCHED || gVDX.vdd[j]->status == VD_INOP)))
                            {
                                // This is a new snapshot for this controllers snappool.
                                // NOTE Even if a snapshot is inop we need to construct data struct for it such
                                // that it can be deleted at some point.  This will free up the OGERs that could be
                                // restored.
                                gVDX.vdd[j]->vd_incssms = DEF_build_slink_structures(gVDX.vdd[j]->devCap,
                                                                                     (UINT32)gVDX.vdd[j]->vid,
                                                                                     (UINT32)rdd->sps);
                                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                                if (gVDX.vdd[j]->status == VD_INOP)
                                {
                                    // Need to send the CCB log message
                                    fprintf(stderr, "[%s:%d] Snapshot (NV Oger update) failed, \
                                        Moving  Snapshot VID %d  to Permanent INOP state\n",
                                            __func__, __LINE__, gVDX.vdd[j]->vid);
                                    ss_invalidate_snapshot(j);
                                    if (gVDX.vdd[j]->vd_incssms)
                                    {
                                        remove_ssms_from_src_vdd(gVDX.vdd[j]->vd_incssms,
                                            gVDX.vdd[gVDX.vdd[j]->vd_incssms->ssm_srcvid]);
                                    }
                                }
                                else
                                {
                                    if (add_ssms_nv(gVDX.vdd[j]->vd_incssms, gnv_oger_array[rdd->depth]) != EC_OK)
                                    {
                                        fprintf(stderr, "[%s:%d] Snapshot (NV SSMS Update) failed, \
                                           Moving Snapshot VID %d  to Permanent INOP state\n",
                                                __func__, __LINE__, gVDX.vdd[j]->vid);
                                        ss_invalidate_snapshot(j);
                                        if (gVDX.vdd[j]->vd_incssms)
                                        {
                                            remove_ssms_from_src_vdd(gVDX.vdd[j]->vd_incssms,
                                                gVDX.vdd[gVDX.vdd[j]->vd_incssms->ssm_srcvid]);
                                        }
                                    }
                                }
                            }
                        }
                    }                  /* End Slinkdev */
                }                      /* End rdd != NULL */
            }                          /* End vdd[j] */
        }                              /* End of MAX Virtual Disks */
    }                                  /* End of got[1] or got[0] */
    ss_restore_pcb = NULL;
    fprintf(stderr, "Exiting restore_ss_nv_task\n");
}                                      /* End of restore_ss_nv_task */


/**
******************************************************************************
**
**  @brief      ss_disown_snappool, cleanup the local structure references
**              for the snappool that is passed as a paramater.
**
**  @param      spoolid
**
**  @return     none
**
******************************************************************************
**/

void ss_disown_snappool(UINT32 dummy0 UNUSED, UINT32 dummy1 UNUSED, UINT16 spool_id)
{
    clean_vdd_refs(spool_id);
    p_Free(gnv_oger_array[spool_id], sizeof(OGER), __FILE__, __LINE__);
    gnv_oger_array[spool_id] = NULL;
    if (gnv_header_array[spool_id])
    {
        p_Free(gnv_header_array[spool_id], sizeof(SS_HEADER_NV), __FILE__, __LINE__);
        gnv_header_array[spool_id] = NULL;
    }
}                                       /* End of ss_disown_snappool */

/**
******************************************************************************
**
**  @brief      Removes VDD references for SSMS
**
**  @param      prefowner
**
**  @return     none
**
******************************************************************************
**/

void clean_vdd_refs(UINT8 pref_owner)
{
    VDD            *vdd;
    int             i;
    UINT8          *gotit;

    gotit = (UINT8 *)p_MallocC(sizeof(UINT8) * MAX_VIRTUAL_DISKS, __FILE__, __LINE__);
    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        gotit[i] = 0;
        if (gVDX.vdd[i] != NULL)
        {
            vdd = gVDX.vdd[i];
            if (vdd->vd_outssms)
            {
                if (vdd->vd_outssms->ssm_prefowner == pref_owner)
                {
                    gotit[i] = 1;
                }
            }
        }
    }
    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        if (gVDX.vdd[i] != NULL)
        {
            vdd = gVDX.vdd[i];
            if (vdd->vd_incssms)
            {
                if (vdd->vd_incssms->ssm_prefowner == pref_owner)
                {
                    // Remove reference to the SSMS from the SS and free the SSMS and related structs
                    DEF_Kill_SS_Structs(vdd->vd_incssms);
                    vdd->vd_incssms = NULL;
                    vdd->scpComp = 0;
                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                }

            }
            if (gotit[i])
            {
                vdd->vd_outssms = NULL;
                vdd->vd_outssmstail = NULL;
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            }
            if (BIT_TEST(vdd->attr, VD_BSNAPPOOL) && ((vdd->vid & 0x1) == pref_owner))
            {
                // If this is the snappool then clear the percentage.
                vdd->scpComp = 0;
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            }
        }
    }
    p_Free(gotit, sizeof(UINT8) * MAX_VIRTUAL_DISKS, __FILE__, __LINE__);
}                                       /* End of clean_vdd_refs */

/**
******************************************************************************
**
**  @brief   Moves all the  snapshot related VDDs related to a snappool to a
**           temporary state of IO_SUSPENDED.
**
**
**  @return    none
**
******************************************************************************
*/

void ss_tmpsuspend_allsnapshots(UINT16 spool_vid)
{
    UINT16  pool_id = spool_vid & 0x1;
    int     j = 0;
    RDD    *rdd;
    VDD    *vdd;
    UINT16  scor_vid = 0;

    for (j = 0; j < MAX_VIRTUAL_DISKS; ++j)
    {
        if ((vdd = gVDX.vdd[j]) != NULL)
        {
            rdd = vdd->pRDD;
            if (rdd != NULL)
            {
                if ((rdd->type == RD_SLINKDEV) && (rdd->depth == pool_id))
                {
                    if (vdd->status != VD_INOP)
                    {
                        fprintf(stderr, "SSDBG: moving SS vid %d to suspend\n", vdd->vid);
                        vdd->status = SS_IOSUSPEND;
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                    }
                    // Set the status of the source Vdisk to IO suspend as well
                    scor_vid = rdd->sps;
                    if (gVDX.vdd[scor_vid]->status == VD_OP)
                    {
                        fprintf(stderr, "SSDBG: moving SRC vid %d to suspend\n", gVDX.vdd[scor_vid]->vid);
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        gVDX.vdd[scor_vid]->status = SS_IOSUSPEND;
                    }
               }
            }
         }
    }
}   /* End of ss_tmpsuspend_allsnapshots */


/**
******************************************************************************
**
**  @brief      Search for the oger with the given ordinal in all the ogers
**              used for all the SSMS for this snap pool.
**
**  @param      ordinal - Oger ordinal to check for.
**
**  @param      spool_vid - snap pool vid.
**
**  @return    TRUE if duplicate found, FALSE if not
**
******************************************************************************
**/
INT32 ss_check_for_duplicate_oger(INT32 ordinal, UINT16 spool_vid)
{
    INT32   i;
    VDD    *vdd;
    SSMS   *ssms;
    OGER   *oger;
    UINT16  pool_id = spool_vid & 0x1;
    UINT32  oger_count = 0;

    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        vdd = gVDX.vdd[i];
        if (vdd == NULL)
        {
            continue;
        }

        ssms = vdd->vd_incssms;
        if (ssms == NULL)
        {
            continue;
        }

        oger = ssms->ssm_frstoger;
        if ((oger == NULL) ||(oger->ogr_vid != spool_vid))
        {
            continue;
        }

        oger_count = 0;
        while(oger)
        {
            oger_count++;
            if (oger->ogr_ord == ordinal)
            {
                fprintf(stderr,"FOUND DUPLICATE OGER: ORDINAL = %d, SNAP POOL VID = %d\n", ordinal, spool_vid);
                dump_bitmap(DEF_dog_table + (pool_id*(MAX_OGER_COUNT_GT2TB/8)),"ss_check_for_duplicate_oger", spool_vid);
                fprintf(stderr,"SSMS DUMP: %p\n",ssms);
                fprintf(stderr, " SRC VID = %d, SS VID = %d, PREF OWNER = %d\n",
                        ssms->ssm_srcvid, ssms->ssm_ssvid, ssms->ssm_prefowner);
                fprintf(stderr, " First OGER = %p, TAIL OGER = %p, OGER COUNT = %d, SSMS ORDINAL = %d\n",
                        ssms->ssm_frstoger, ssms->ssm_tailoger, ssms->ssm_ogercnt,
                        ssms->ssm_ordinal);

                fprintf(stderr,"OGER DUMP: %p\n ",oger);
                fprintf(stderr, " OGER VID = %d, SEG COUNT = %d, SDA = %lld\n",
                        oger->ogr_vid, oger->ogr_segcnt, oger->ogr_sda);
                fprintf(stderr, " ORDINAL = %d, SS VID = %d, LEFT CH = %p, RIGHT CH = %p\n",
                        oger->ogr_ord, oger->ogr_ssvid, oger->ogr_leftch,
                        oger->ogr_rightch);
                fprintf(stderr, " PARENT = %p, SDA KEY = %d, MAX PR = %d, STATUS = %d\n",
                        oger->ogr_parent, oger->ogr_sdakey,
                        oger->ogr_maxpr, oger->ogr_stat);

                ss_invalidate_snapshot(ssms->ssm_ssvid);
                remove_ssms_from_src_vdd(ssms, gVDX.vdd[ssms->ssm_srcvid]);
                return(TRUE);
            }

            if (oger_count > ssms->ssm_ogercnt)
            {
                fprintf(stderr,"%s: BAD Link in oger list, exit. SS VID = %d -- oger_count(%d) > ssm_ogercnt(%d)\n",__func__,ssms->ssm_ssvid, oger_count, ssms->ssm_ogercnt);
                break;
            }
            oger = oger->ogr_link;
        }
    }
    return(FALSE);
}   /* End of ss_check_for_duplicate_oger */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
