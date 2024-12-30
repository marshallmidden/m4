/* $Id: ss_ssms.c 159966 2012-10-01 23:20:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       ss_ssms.c
**
**  @brief      Snapshot code converted from assembly
**
**  Provides C implementations of some snapshot operations.
**
**  Copyright (c) 2008-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
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
#include "ss_comp.h"
#include "pm.h"
#include "LOG_Defs.h"
#include "vdd.h"
#include "defbe.h"
#include "mem_pool.h"
#include "ddr.h"
#include "misc.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/
/* TRB...move this into common include later...it is also in apool.h */
extern void logSPOOLevent(UINT8 event_code, UINT8 severity,UINT32 errorCode,UINT32 value1,UINT32 value2);
extern void K_comp(struct ILT *);

/*
******************************************************************************
** Public variables
******************************************************************************
*/
QU      SS_cow_qu;
SSMS    *syg_ssms;
SSMS    *bldsgi_ssms;
extern UINT8    gss_version[2];

/*
******************************************************************************
** Private and external function prototypes
******************************************************************************
*/
NORETURN void cow_q_task(void);
extern struct OGER *D$alloc_oger(UINT32 vid);
extern void access_snapshot(struct SSMS *, struct ILT *, UINT64 sda);
struct OGER *htsearch(UINT32 *slot, UINT32 seg, struct SSMS *);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*
******************************************************************************
**
**  @brief  Task to initiate copy-on-write operations
**
******************************************************************************
*/

NORETURN void cow_q_task(void)
{
    ILT     *ilt;
    SGL     *sgl;
    VRP     *vrp;
    SYNC    *syn;
    ssmdep  *ssms_ic;

    for (;;)
    {
        TaskSetMyState(PCB_NOT_READY);
        TaskSwitch();

        for (;;)
        {
            while (cow_orc >= MAX_COW_ORC)
            {
                TaskSleepMS(1);
            }

            ilt = QU_DequeReqILT(&SS_cow_qu);
            if (!ilt)
            {
                break;
            }

            sgl = m_asglbuf(SEGSIZE_SEC * BYTES_PER_SECTOR);
            vrp = (VRP *)ilt->ilt_normal.w0;       /* Get VRP pointer from ILT */
            vrp->pSGL = sgl;
            vrp->sglSize = sizeof(SGL) + sizeof(SGL_DESC);

            syn = (SYNC *)ilt->ilt_normal.w5;      /* Get SYNC pointer from ILT */
            if (!syn)
            {
                fprintf(stderr, "%s: No SYNC record?\n", __func__);
                continue;
            }

            ssms_ic = syn->syn_iscnt;
            if (ssms_ic)
            {
                UINT32  segix = syn->syn_segnum & SEG_ACC_MASK;

                ssms_ic->ssmdep_cow_seg[segix].sgl = sgl;
            }

            ++cow_orc;
            EnqueueILT((void *)V_que, ilt, ilt->cr);
        }
    }
}   /* End of cow_q_task */


/*
******************************************************************************
**
**  @brief  Queue a request to the copy on write task
**
**  @param  ilt - The ILT to queue
**
**  @return none
**
******************************************************************************
*/

static void q2_cow_task(ILT *ilt)
{
    QU_EnqueReqILT(ilt, &SS_cow_qu);
}   /* End of q2_cow_task */


/*
******************************************************************************
**
**  @brief  Calculate segment bits
**
**  @param  seg    - Pointer to receive first segment number.
**  @param  sda    - Vdisk SDA.
**  @param  eda    - Vdisk EDA.
**  @param  devCap - Size of Vdisk in sectors.
**  @param  cm     - 0 if for copy manager, 1 for snapshot.
**
**  @return Number of segments
**
******************************************************************************
*/

UINT32 cal_seg_bit(UINT32 *seg, UINT64 sda, UINT64 eda, UINT64 devCap, UINT8 cm)
{
    /* We want the number of times greater than 2TB. */
    if (cm == 0)
    {
        int extra = devCap >> 32;
        sda >>= (SEC2SEG_SF + extra);
        eda >>= (SEC2SEG_SF + extra);
    }
    else
    {
        sda >>= SEC2SEG_SF;
        eda >>= SEC2SEG_SF;
    }

    *seg = sda;                     /* NOTE: limited to 22 bits for copy manager */
/* We know that the range difference is small -- we can't do a very big I/O in the BE! */
    return (eda - sda + 1);
}   /* End of cal_seg_bit */


/*
******************************************************************************
**
**  @brief  determine if a segment bit is set.
**
**  @param  seg  - segment bit number
**  @param  ssms - SSMS address
**
**  @return TRUE if Segment map bit set, else FALSE.
**
******************************************************************************
*/
static int D_chk_seg_bit(UINT32 seg, SSMS *ssms)
{
    struct SM *sm;
    int     Index;
    UINT32  extra = seg >> 21;
    UINT32  within = seg - (extra << 21); 

    if (ssms->ssm_regmap[extra] == 0) {
        return(FALSE);                  /* If no region map table address */
    }

    sm = ssms->ssm_regmap[extra]->regions[within >> (SMBITS2WRD_SF + SMWRD2REG_SF)];
    if (sm == 0) {
        return(FALSE);                  /* If no segment map */
    }

    Index = (within >> SMBITS2WRD_SF) & SMWRDIDX_MASK;
    if (sm->segments[Index] & (1 << (31-(within & 0x1f)))) {
        return(TRUE);                   /* segment bit is set */
    }
    return(FALSE);                      /* segment map bit clear */
}   /* End of D_chk_seg_bit */

/*
******************************************************************************
**
**  @brief  Count number of unpopulated segments
**
**  @param  ssms - Pointer to ssms
**  @param  seg - Segment number
**  @param  nseg - Number of segments
**
**  @return Number of dirty segments
**
******************************************************************************
*/

static UINT32 count_unpopulated_segments(SSMS *ssms, UINT32 seg, UINT32 nsegs)
{
    UINT32  cnt;

    for (cnt = 0; nsegs; ++seg, --nsegs)
    {
        if (segment_not_populated(seg, ssms))
        {
            ++cnt;
        }
    }
    return cnt;
}   /* End of count_unpopulated_segments */


/*
******************************************************************************
**
**  @brief  Search sync groups
**
**  @param  ssms - Pointer to SSMS
**  @param  seg - Segment number
**
**  @return Pointer to sync record found, or NULL
**
******************************************************************************
*/

SYNC *search_sync_groups(SSMS *ssms, const UINT32 seg)
{
    SYNC    *psync;

    for (psync = ssms->ssm_synchead; psync; psync = psync->syn_link)
    {
        if (psync->syn_segnum == seg)
        {
            return psync;
        }
    }
    return NULL;
}   /* End of search_sync_groups */


/*
******************************************************************************
**
**  @brief  Append sync record to deplist or acclist
**
**  @param  found_sync - Pointer to SYNC record found in list
**  @param  psync - Pointer to SYNC record to append
**
**  @return none
**
******************************************************************************
*/

static void append_to_list(SYNC *found_sync, SYNC *psync)
{
    if (BIT_TEST(found_sync->syn_state, syn_state_acc_src) &&
        BIT_TEST(psync->syn_state, syn_state_acc_src) &&
        found_sync->syn_deplst == NULL)
    {
        while (found_sync->syn_acclst)
        {
            found_sync = found_sync->syn_acclst;
        }
        found_sync->syn_acclst = psync;
        return;
    }

    /* Since it did not go into the acclst, be sure that the src_acc bit is clear. */
    BIT_CLEAR(psync->syn_state, syn_state_acc_src);
    while (found_sync->syn_deplst)
    {
        found_sync = found_sync->syn_deplst;
    }
    found_sync->syn_deplst = psync;
}   /* End of append_to_list */


/*
******************************************************************************
**
**  @brief  Compute a slot number based on an input key.
**
**          Accepts a key that is applied to the double hash function to determine
**          a slot number. The probe count is used as the scaler to the second
**          hash function. In this case h(k) takes the form:
**              h(k) = ((key mod prime1) + probe_count*(key mod prime2)|1)mod 1024
**          Note: the second hash function must always return an odd number.
**
**  @param  key (SDA) -- value to hash
**  @param  probe_count
**
**  @return slot number
**
******************************************************************************
*/

static UINT32 d_hash_key(UINT32 key, int probe_count)
{
    UINT32 hash1 = key % H1_PRIME;          /* Perform first hash function */
    /* Make sure second hash always returns an odd number. */
    UINT32 hash2 = (key % H2_PRIME) | 1;    /* Perform second hash function */

    /* Trim the number to fit within the hash. */
    return((hash1 + hash2 * probe_count) % SEGSPEROGER);
}   /* End of d_hash_key */


/*
******************************************************************************
**
**  @brief  Check an OGER slot for occupation
**
**  @param  slot - Pointer to UINT32 to slot
**  @param  oger - Oger to check
**
**  @return Bit 31 set if slot is occupied, otherwise input slot.
**
******************************************************************************
*/
static UINT32 d_chkslot(UINT32 slot, OGER *oger)
{
    if ((oger->ogr_segfld[slot / 8] & (1 << (slot % 8))) != 0) {
        slot |=  (1 << 31);             /* Set bit 31 indicating occupied */
    }
    return(slot);
}   /* End of d_chkslot */

/*
******************************************************************************
**
**  @brief  Assign an SDA to an OGER slot
**
**      Accepts an OGER,SDA, and a slot count. The appropriate segment map bit
**      is set indicating that the slot is ocupied. The SDA value is stored in
**      the slot as well.
**
**  @param  seg  - Segment number
**  @param  slot - Slot Number
**  @param  oger - Pointer to OGER
**
**  @return none
**
******************************************************************************
*/
static void d_asgslot(UINT32 seg, UINT32 slot, OGER *oger)
{
    UINT8 ogthrsh = (gss_version[oger->ogr_vid & 1] == SS_NV_VERSION_GT2TB) ? OGTHRSH_GT2TB : OGTHRSH;
    if ((oger->ogr_segcnt + 1) >= ogthrsh) {
        oger->ogr_stat = ogr_stat_full;         /* Set the status to full. */
    }
    oger->ogr_segcnt++;                         /* Update the segment count */
    oger->ogr_sdamap[slot] = seg;               /* Store the new segment */
    oger->ogr_segfld[slot/8] |= (1 << (slot % 8));
}   /* End of d_asgslot */

/*
******************************************************************************
**
**  @param  slot - Pointer to UINT32 to get found slot
**  @param  seg  - Segment number
**  @param  ssms - Pointer to SSMS
**
**  @return OGER and slot (above).
**
******************************************************************************
*/

OGER *htsearch(UINT32 *slot, UINT32 seg, SSMS *ssms)
{
    OGER *oger = ssms->ssm_frstoger;        /* Current search OGER */
    int   probe_count = 0;

    /* Loop until the hash function either returns a match or an empty slot. */
    for (;;)
    {
        if (oger == 0)
        {
            *slot = 0;
            return(0);                      /* return empty slot */
        }
        for (;;)
        {
            UINT32 slot_found;

            if (probe_count > oger->ogr_maxpr)
            {
                break;                      /* if max probe count exceeded */
            }

            /* Check to see if the slot is filled. */
            slot_found = d_chkslot(d_hash_key(seg, probe_count), oger);
            if ((slot_found & 0x80000000) == 0)
            {
                break;                      /* Slot is not occupied, check the next OGER */
            }
            /* Clear the occupied bit */
            slot_found = slot_found & 0x7fffffff;
            if (oger->ogr_sdamap[slot_found] == seg)
            {
                *slot = slot_found;
                return(oger);
            }
            probe_count++;
        }

        /* Advance to the next OGER */
        probe_count = 0;                    /* Reinitialize the probe count. */
        if (seg > oger->ogr_sdakey)
        {
            oger = oger->ogr_rightch;       /* Get the right child. */
        }
        else
        {
            oger = oger->ogr_leftch;        /* Get the left child. */
        }
    }
}   /* End of htsearch */

/*
******************************************************************************
**
**  @brief  Insert a segment into the hash tree,
**
**      Accepts a SSMS and SDA. The OGER hash tree is searched for the proper
**`     insertion point. This function returns the OGER as well as the slot.
**
**  @param  slot - Pointer to UINT32 to get found slot
**  @param  seg  - Segment number
**  @param  ssms - Pointer to SSMS
**
**  @return oger
**
******************************************************************************
*/
OGER *htinsert(UINT32 *slot, int *new_oger, UINT32 seg, SSMS *ssms)
{
    struct OGER *oger;
    struct OGER *working;
    int probe_count = 0;                                        /* Initialize the probe_count */
    UINT32 value;

    oger = ssms->ssm_frstoger;                                  /* Get the Root OGER */
    *new_oger = 0;                                              /* Flag old OGER. */

    for (;;)
    {
        if (oger->ogr_stat == ogr_stat_full) {
            /* This OGER is full so traverse the tree. */
            if (seg > oger->ogr_sdakey) {
                if (oger->ogr_rightch != 0) {
                    oger = oger->ogr_rightch;
                } else {
                    working = D$alloc_oger(oger->ogr_vid);
                    if (working == 0)                           /* If cannot allocate */ {
                        *slot = 0;                              /* No slot number */
                        return(0);                              /* Clear OGER on error */
                    }
                    *new_oger = 1;                              /* Flag new OGER. */
                    probe_count = 0;                            /* Clear the probe_count */

                    working->ogr_parent = oger;                 /* Link back to parent */
                    oger->ogr_rightch = working;                /* This is the right child */
                    ssms->ssm_prev_tailoger = ssms->ssm_tailoger;   /* Store the previous tailoger into SSMS */
                    ssms->ssm_tailoger->ogr_link = working;     /* Link in the new one */
                    ssms->ssm_tailoger = working;               /* Update tail */
                    ssms->ssm_ogercnt++;                        /* Increment the OGER count */
                    working->ogr_vid = oger->ogr_vid;           /* Store VID into new child */
                    working->ogr_sdakey = oger->ogr_sdakey + (oger->ogr_sdakey / 2);     /* Store SDA key for this OGER */
                    oger = working;                             /* Update oger we are processing */
                }
            }
            else if (oger->ogr_leftch == 0) {
                working = D$alloc_oger(oger->ogr_vid);          /* Allocate a new OGER */
                if (working == 0)                               /* If cannot allocate */ {
                    *slot = 0;                                  /* No slot number */
                    return(0);                                  /* Clear OGER on error */
                }
                *new_oger = 1;                                  /* Flag new OGER. */
                probe_count = 0;                                /* Clear the probe_count */

                working->ogr_parent = oger;                     /* Link back to parent */
                oger->ogr_leftch = working;                     /* This is the left child */
                ssms->ssm_prev_tailoger = ssms->ssm_tailoger;   /* Store the previous tailoger into SSMS */
                ssms->ssm_tailoger->ogr_link = working;         /* Link in the new one */
                ssms->ssm_tailoger = working;                   /* Update tail */
                ssms->ssm_ogercnt++;                            /* Increment the OGER count */
                working->ogr_vid = oger->ogr_vid;               /* Store VID into new child */
                working->ogr_sdakey = oger->ogr_sdakey / 2;     /* Store SDA key for this OGER */
                oger = working;                                 /* Update oger we are processing */
            } else {
                oger = oger->ogr_leftch;
            }
        }
        else
        {
            /* This OGER is not full, find an open slot for this key. */
            for (;;)
            {
                value = d_hash_key(seg, probe_count);
                value = d_chkslot(value, oger);                 /* Check if slot is open */
                if ((value & (1 << 31)) == 0) {
                    break;                                      /* Exit if slot is open */
                }
                probe_count++;                                  /* Increment the probe_count */
            }
            d_asgslot(seg, value, oger);                        /* Assign this SDA to this slot */
            if (probe_count > oger->ogr_maxpr) {
                oger->ogr_maxpr = probe_count;                  /* Update the maximum probe count */
            }
            *slot = value;                                      /* Return slot number */
            return(oger);
        }
    }
}   /* End of htinsert */


/*
******************************************************************************
**
**  @brief  Routine to allocate an ILT/VRP combo for ss
**
**  @return Pointer to allocated ILT
**
******************************************************************************
*/

static ILT *alloc_ss_vrp_ilt(void)
{
    ILT     *ilt;
    VRP     *vrp;

    /* Allocate a VRP/ILT/SN for the read and write requests */
    ilt = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    vrp = get_vrp();
#ifdef M4_DEBUG_VRP
CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, vrp);
#endif /* M4_DEBUG_VRP */
    ilt->ilt_normal.w0 = (UINT32)vrp;
    memset(vrp, 0, sizeof(*vrp));

    return ilt;
}   /* End of alloc_ss_vrp_ilt */


/*
******************************************************************************
**
**  @brief  Routine to set up ss access SGL
**
**  @param  orig - Pointer to original VRP
**  @param  new - Pointer to new VRP
**  @param  offset - Offset into original buffer in sectors
**  @param  size - Size of operation in sectors
**
**  @return Pointer to allocated ILT
**
******************************************************************************
*/

static void create_ss_sgl(VRP *orig, VRP *new, UINT32 offset, UINT32 size)
{
    if (!orig->pSGL)    /* If no original SGL, then no SGL for the new VRP */
    {
        new->pSGL = NULL;
        new->sglSize = 0;
        return;
    }

    if ((UINT32)orig->pSGL == 0xfeedf00d)   /* If no SGL, BUT cachefe has changed it. */
    {
        new->pSGL = NULL;
        new->sglSize = 0;
        return;
    }

    /* If the original VRP has an SGL, make one for the new VRP too */
    SGL         *sgl;
    SGL_DESC    *ndesc;
    SGL_DESC    *odesc;

    /* Set up to access the original buffer directly */
    sgl = m_asglbuf(0);
    odesc = (SGL_DESC *)(orig->pSGL + 1);
    ndesc = (SGL_DESC *)(sgl + 1);
    ndesc->addr = (void *)((UINT8 *)odesc->addr + offset * BYTES_PER_SECTOR);
    ndesc->len = size * BYTES_PER_SECTOR;
    new->pSGL = sgl;
    new->sglSize = sizeof(SGL) + sizeof(SGL_DESC);
}   /* End of create_ss_sgl */


/*
******************************************************************************
**
**  @brief  Routine to start COW operation
**
**  @param  ssms - Pointer to SSMS for the snapshot
**  @param  ilt - Pointer to ILT (which includes VRP) for operation
**  @param  seg - Segment number
**  @param  psync - Pointer to associated SYNC record
**
**  @return none
**
******************************************************************************
*/

static void start_cow(SSMS *ssms, ILT *ilt, UINT32 seg, SYNC *psync)
{
    VRP     *vrp;

    vrp = (VRP *)ilt->ilt_normal.w0;

    /* Initialize some VRP parameters */
    vrp->function = VRP_INPUT | (1 << VRP_SNAPSHOT_SPECIAL);
    if (V_vddindx[ssms->ssm_srcvid]->pDCD)
    {
        // Set the copy bypass bit since the SS source is a copy destination.
        vrp->function = vrp->function | (1 << VRP_SPECIAL);
    }
    vrp->vid = ssms->ssm_srcvid;
    vrp->length = SEGSIZE_SEC;
    vrp->startDiskAddr = (UINT64)seg * SEGSIZE_SEC;

    /* The SGL is allocated in the cow task */
    ilt->cr = (void *)C_label_referenced_in_i960asm(q2_snapshot_completer);
    ilt->ilt_normal.w5 = (UINT32)psync;
    ilt->ilt_normal.w6 = (UINT32)ssms;
    ilt->ilt_normal.w7 = (UINT32)ss_rcomp;     // Indicate that this is a COW read

    /*
     ** The misc member of the ILT is used to store two values:
     ** bits 0-15 holds the COW retry count and
     ** bits 15-31 holds the slot number (which is used to compute
     ** the SDA if the write to snapshot is retried).
     */
    ilt->misc |= SNAP_COWRETRY;

    ssmdep  *ssms_ic = psync->syn_iscnt;

    if (ssms_ic)
    {
        unsigned    segix = seg & SEG_ACC_MASK;
        unsigned    ix;
        ILT         **ilts = ssms_ic->ssmdep_cow_seg[segix].ilts;

        for (ix = 0; ix < ssms_ic->ssmdep_max; ++ix)
        {
            if (!ilts[ix])
            {
                ilts[ix] = ilt;     /* Save ilt for use by ss_rcomp */
                break;
            }
        }
        if (ix >= ssms_ic->ssmdep_max)
        {
            fprintf(stderr, "%s: ilt array overflow??\n", __func__);
        }
        if (ssms_ic->ssmdep_cow_seg[segix].ilt)
        {
            /*
             * If a COW has already been started for this segment, don't
             * start another one. Let the read completion initiate the
             * other writes.
             */
            return;
        }
        ssms_ic->ssmdep_cow_seg[segix].ilt = ilt;
    }

    q2_cow_task(ilt);
}   /* End of start_cow */


/*
******************************************************************************
**
**  @brief  Routine to start deferred COW operation
**
**  @param  syn - Pointer to associated SYNC record
**
**  @return none
**
******************************************************************************
*/

void start_deferred_cow(SYNC *syn)
{
    ILT     *newilt;

    newilt = alloc_ss_vrp_ilt();
    syn->syn_rdreq = newilt;
    start_cow(syn->syn_ssms, newilt, syn->syn_segnum, syn);
}   /* End of start_deferred_cow */


/*
******************************************************************************
**
**  @brief  Routine to create sync records
**
**  @param  master - Pointer to master record, or NULL if this is it
**  @param  ssms - Pointer to SSMS for the snapshot
**  @param  ilt - Pointer to ILT (which includes VRP) for operation
**  @param  seg - Segment number
**  @param  ssms_ic - Pointer to ssmdep
**  @param  unpop_segs - Number of unpopulated segments
**  @param  nsegs - Number of segments
**  @param  acc_src - TRUE if snapshot access to source
**
**  @return none
**
******************************************************************************
*/

static SYNC *create_sync_record(SYNC *master, SSMS *ssms, ILT *ilt, UINT32 seg,
            ssmdep *ssms_ic, UINT32 unpop_segs, UINT32 nsegs, int acc_src)
{
    SYNC    *psync;
    SYNC    *tmp_sync;
    ILT     *newilt;

    psync = p_MallocC(sizeof(*psync), __FILE__, __LINE__);
    psync->syn_ssms = ssms;
    psync->syn_iscnt = ssms_ic;

    if (acc_src)
    {
        BIT_SET(psync->syn_state, syn_state_acc_src);
    }
    if (!master)
    {
        master = psync;
    }

    psync->syn_firstilt = ilt;
    psync->syn_master = master;
    psync->syn_segnum = seg;
    psync->syn_count = unpop_segs;
    psync->syn_thcount = nsegs;

    newilt = NULL;

    /* See if this segment is a member of another group */
    tmp_sync = search_sync_groups(ssms, seg);
    if (tmp_sync)
    {
        append_to_list(tmp_sync, psync);
    }
    else if (!acc_src)
    {
        newilt = alloc_ss_vrp_ilt();
        psync->syn_rdreq = newilt;
    }

    tmp_sync = ssms->ssm_synctail;
    if (tmp_sync)
    {
        tmp_sync->syn_link = psync;
    }
    else
    {
        ssms->ssm_synchead = psync;
    }
    ssms->ssm_synctail = psync;
    psync->syn_prev = tmp_sync;

    if (newilt)
    {
        start_cow(ssms, newilt, seg, psync);
    }
    return psync;
}   /* End of create_sync_record */


/*
******************************************************************************
**
**  @brief  Create SPLIT record
**
**  @param  ssms - Pointer to SSMS
**  @param  ilt - Pointer to original ILT
**  @param  master - Pointer to master ILT
**  @param  segs - Segment count
**
**  @return Pointer to created SPLIT record
**
******************************************************************************
*/

static SPLIT *create_split(SSMS *ssms, ILT *ilt, ILT *master, UINT32 segs)
{
    SPLIT   *spl;

    spl = p_MallocC(sizeof(*spl), __FILE__, __LINE__);
    spl->spl_ssms = ssms;
    spl->spl_segcnt = segs;
    spl->spl_stat = EC_OK;
    spl->spl_finished = 0;
    spl->spl_master = master;
    spl->spl_origilt = ilt;
    spl->spl_srcvid = ssms->ssm_srcvid;
    spl->spl_ssvid = ssms->ssm_ssvid;

    return spl;
}   /* End of create_split */


/*
******************************************************************************
**
**  @brief  Write snapshot source.
**
**          This routine is involved in handling writes to a snapshot source
**          Vdisks. It takes a ILT/VRP/SN input combo and constructs a
**          group of sync record needed to push the data from the source to
**          the snapshot OGER. All of the necessary sub-structures are created
**          at this time as well.
**
**  @param  ssms - Pointer to SSMS
**  @param  ssms_ic - Pointer to SSMS interdependency counter
**  @param  ilt - Pointer to incoming ILT
**  @param  sda - SDA
**
**  @return none
**
******************************************************************************
*/

static void do_write_ss_source(SSMS *ssms, ssmdep *ssms_ic, ILT *ilt, UINT64 sda)
{
    VRP     *vrp;
    UINT32  seg;
    UINT32  nsegs;
    SYNC    *master;
    SYNC    *psync;
    UINT32  unpop_segs;
    static char reenter = 0;

    if (reenter)
    {
        fprintf(stderr, "%s: Re-entered - crashing now\n", __func__);
        abort();
    }
    reenter = 1;
    syg_ssms = ssms;    /* Indicate what ssms we are working on */

    --ilt;              /* Back original ILT up to preserve sanity */

    /*
     *  First determine the starting segment and the number of segments.
     *  Use these numbers to construct each Sync record.
     */
    vrp = (VRP *)ilt->ilt_normal.w0;       /* Get the VRP */
    /* NOTE: use snapshot cal_seg_bit() format. */
    nsegs = cal_seg_bit(&seg, sda, sda + vrp->length - 1, gVDX.vdd[vrp->vid]->devCap, 1);
    master = NULL;

    unpop_segs = count_unpopulated_segments(ssms, seg, nsegs);

    /* For each segment, create a sync record if there isn't one */
    for (; nsegs; --nsegs, ++seg)
    {
        if (segment_is_populated(seg, ssms))
        {
            continue;
        }
        psync = create_sync_record(master, ssms, ilt, seg, ssms_ic, unpop_segs, nsegs, FALSE);
        if (!master)
        {
            master = psync;
        }
    }

    /* If no sync records have been allocated, re-issue the original ILT */
    if (master)
    {
        goto out;
    }

    if (ssms_ic)    /* If inter-dependency count to handle */
    {
        if (--ssms_ic->ssmdep_cnt)  /* If not decremented to zero */
        {
            goto out;
        }
        clean_up_ssms_ic(ssms_ic);  /* Free once it is zero */
    }

    vrp->function |= 1 << VRP_SNAPSHOT_SPECIAL;
    EnqueueILT((void *)V_que, ilt, ilt->cr);

out:
    reenter = 0;
    syg_ssms = 0;
}   /* End of do_write_ss_source */


/*
******************************************************************************
**
**  @brief  Access snapshot VDisk.
**
**          This routine is involved in performing accesses to snapshots. It
**          takes a ILT/VRP input combo and constructs a group of sync
**          records needed to push the data from the source to the snapshot
**          OGER as needed. At the same time new ILT/VRP's are constructed.
**
**  @param  ssms - Pointer to SSMS
**  @param  ilt - Pointer to incoming ILT
**  @param  sda - SDA
**
**  @return none
**
******************************************************************************
*/

void access_snapshot(SSMS *ssms, ILT *ilt, UINT64 sda)
{
    VRP     *vrp;
    UINT32  seg;
    UINT32  nsegs;
    UINT32  seg_count;
    SYNC    *psync;
    ILT     *save_master = NULL;
    UINT16  vfunc;
    UINT32  cuml_sec;
    char    is_read = FALSE;
    char    is_like_read = FALSE;
    char    is_write = FALSE;
    char    acc_src;
    static char reenter = 0;
    VDD     *vdd;

    if (reenter)
    {
        fprintf(stderr, "%s: Re-entered - crashing now\n", __func__);
        abort();
    }
    reenter = 1;
    bldsgi_ssms = ssms;

    --ilt;              /* Back up original ILT to preserve sanity */
    cuml_sec = 0;

    /*
     *  First determine the starting segment and the number of segments.
     *  Use these numbers to construct each Sync record.
     */
    vrp = (VRP *)ilt->ilt_normal.w0;       /* Get the VRP */
    vfunc = vrp->function & 0xFF;
    vdd   = gVDX.vdd[vrp->vid];            /* Get Snapshot VDD */
    /* Categorize operation appropriately */

    switch (vfunc)
    {
    case VRP_INPUT:
    case VRP_COPYINPUT:
        is_read = TRUE;
        /* Fall-through to set is_like_read also */

    case VRP_VERIFY_CHK_WORD:
    case VRP_VERIFY:
        is_like_read = TRUE;
        vdd->sprCnt++;          /* Increment Sample period request count */
        vdd->rReq++;            /* Increment total READ request count    */
        vdd->spsCnt += vrp->length; /* Accumulate Sample period sector count */
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        break;

    case VRP_OUTPUT:
    case VRP_COPYOUTPUT:
    case VRP_OUTPUT_VERIFY:
        is_write = TRUE;
        vdd->sprCnt++;          /* Increment Sample period request count */
        vdd->wReq++;            /* Increment Total Write request count   */
        vdd->spsCnt += vrp->length; /* Accumulate Sample period sector count */
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        break;

    default:
        fprintf(stderr, "%s: Unexpected VRP function %d\n", __func__, vfunc);
        vrp->status = EC_OK;
        K_comp(ilt + 1);        /* Complete the request */
        goto out;               /* Go to exit */
    }
    /* NOTE: use snapshot cal_seg_bit() format. */
    nsegs = cal_seg_bit(&seg, sda, sda + vrp->length - 1, vdd->devCap, 1);
    seg_count = nsegs;

    /*
     * For each segment create a sync record unless the segment is already
     * in sync. Also, if there is more than one segment, separate ILT/VRP/SGL
     * combos will need to be constructed for each segment regardless of if
     * there is a sync record for that segment or not.
     */

    for (; nsegs; --nsegs, ++seg)
    {
        psync = NULL;
        acc_src = FALSE;
        if (segment_not_populated(seg, ssms))
        {
            psync = create_sync_record(NULL, ssms, ilt, seg, NULL, 1, nsegs, is_like_read);
            acc_src = BIT_TEST(psync->syn_state, syn_state_acc_src);
        }

        UINT32  size;
        UINT32  size_left;
        UINT32  seg_offset;

        /*
         * If the request spans more than one segment, then allocate a new
         * ILT/VRP/SGL and point the new SGL to the original buffer.
         */
        seg_offset = 0;
        size = SEGSIZE_SEC;             /* 1MB in sectors */
        size_left = vrp->length - cuml_sec;
        if (size_left < size)
        {
            size = size_left;
        }

        ILT     *newilt = NULL;
        VRP     *newvrp = NULL;

        if (seg_count != 1)
        {
            if (seg_count == nsegs) /* If first segment */
            {
                seg_offset = sda % SEGSIZE_SEC;
                size = SEGSIZE_SEC - seg_offset;
            }
            newilt = alloc_ss_vrp_ilt();
            newvrp = (VRP *)newilt->ilt_normal.w0;
            if (psync)
            {
                psync->syn_firstilt = newilt;
            }
            if (seg_count == nsegs) /* If first segment */
            {
                save_master = newilt;
            }

            create_ss_sgl(vrp, newvrp, cuml_sec, size);

            newvrp->function = vfunc | (1 << VRP_SNAPSHOT_SPECIAL);
            newvrp->length = size;
            newvrp->startDiskAddr = seg_offset; /* !!! Used later!!! */

            /*
             * Create a split ILT and queue it up to the split
             * completion routine.
             */
            newilt->ilt_normal.w5 = (UINT32)psync;
            newilt->ilt_normal.w6 = (UINT32)ssms;
            newilt->cr = acc_src ?
                    (void *)C_label_referenced_in_i960asm(ss_splsrccomp)
                :   (void *)C_label_referenced_in_i960asm(ss_splcomp);

            SPLIT   *spl;

            spl = create_split(ssms, ilt, save_master, seg_count);
            newilt->ilt_normal.w2 = SS_MAGIC;
            newilt->ilt_normal.w3 = (UINT32)spl;
            cuml_sec += size;
        }
        else if (acc_src)
        {
            size = vrp->length;
            newilt = alloc_ss_vrp_ilt();
            newvrp = (VRP *)newilt->ilt_normal.w0;
//            psync->syn_firstilt = newilt + 1;
//            save_master = newilt;

            create_ss_sgl(vrp, newvrp, cuml_sec, size);

            newvrp->vid = ssms->ssm_srcvid;
            newvrp->function = vfunc | (1 << VRP_SNAPSHOT_SPECIAL);
            newvrp->length = size;
            newvrp->startDiskAddr = sda % SEGSIZE_SEC;
            newilt->ilt_normal.w5 = (UINT32)psync;
            newilt->ilt_normal.w6 = (UINT32)ssms;
            newilt->cr = (void *)C_label_referenced_in_i960asm(ss_srccomp);
        }

        OGER    *oger = NULL;
        UINT32  slot;

        if (is_write && psync && !acc_src)
        {
        }
        else if (!acc_src)
        {
            /*
             * If a sync record exists, we don't want to submit the
             * request to the virtual layer yet.
             */
            if (psync)
            {
                continue;
            }
            oger = htsearch(&slot, seg, ssms);
            if (!oger)
            {
                /* Read from source. */
                psync = create_sync_record(NULL, ssms, ilt, seg, NULL, 1, nsegs, is_like_read);
                acc_src = TRUE;

                size = vrp->length;
                newilt = alloc_ss_vrp_ilt();
                newvrp = (VRP *)newilt->ilt_normal.w0;

                create_ss_sgl(vrp, newvrp, cuml_sec, size);

                newvrp->vid = ssms->ssm_srcvid;
                newvrp->function = vfunc | (1 << VRP_SNAPSHOT_SPECIAL);
                newvrp->length = size;
                newvrp->startDiskAddr = sda % SEGSIZE_SEC;
                newilt->ilt_normal.w5 = (UINT32)psync;
                newilt->ilt_normal.w6 = (UINT32)ssms;
                newilt->cr = (void *)C_label_referenced_in_i960asm(ss_srccomp);
            }
        }

        ILT     *final_ilt;
        VRP     *final_vrp;
        UINT32  final_offset;

        if (seg_count == 1 && !acc_src)
        {
            final_ilt = ilt;
            final_vrp = vrp;
            final_offset = sda % SEGSIZE_SEC;   /* Sector offset in segment */
        }
        else
        {
            final_ilt = newilt;
            final_vrp = newvrp;
            final_offset = newvrp->startDiskAddr;
        }

        if (acc_src)
        {
            final_vrp->vid = ssms->ssm_srcvid;
            final_vrp->startDiskAddr = (seg * SEGSIZE_SEC) + final_offset;
            if (V_vddindx[ssms->ssm_srcvid]->pDCD)
            {
                // Set the copy bypass bit since the SS source is a copy destination.
                final_vrp->function |= 1 << VRP_SPECIAL;
            }
        }
        else if (!psync)
        {
            final_vrp->vid = oger->ogr_vid;
            final_vrp->startDiskAddr = oger->ogr_sda + (slot * SEGSIZE_SEC) + final_offset;
        }
        if (!psync || acc_src)
        {
            EnqueueILT((void *)V_que, final_ilt, final_ilt->cr);
        }
    }

out:
    reenter = 0;
    bldsgi_ssms = 0;
}   /* End of access_snapshot */


/*
******************************************************************************
**
**  @brief  Access snapshot source VDisk.
**
**          This routine performs writes to snapshot source VDisks. It
**          calls do_write_ss_source for each outstanding snapshot, linking
**          the calls together with an ssmdep counter. The last to complete
**          finally performs the actual source VDisk write.
**
**  @param  ssms - Pointer to SSMS
**  @param  ilt - Pointer to incoming ILT
**  @param  sda - SDA
**
**  @return TRUE if write scheduled, FALSE if to go directly to source
**
******************************************************************************
*/

int write_ss_source(SSMS *ssms, ILT *ilt, UINT64 sda)
{
    int     count_ss;
    ssmdep  *ssms_ic;
    static UINT16   ss_array[MAX_SNAPSHOT_COUNT];

    /*
     * Count the number of interdependency links needed first.
     * Count only operable snapshots.
     */
    for (count_ss = 0; ssms; ssms = ssms->ssm_link)
    {
        UINT16  vid = ssms->ssm_ssvid;          /* Get vid of snapshot */
        VDD     *vdd = gVDX.vdd[vid];

        if (!vdd)           /* If no VDisk, skip it */
        {
            fprintf(stderr, "%s: Missing snapshot vdd %d\n", __func__, vid);
            continue;
        }
        switch (vdd->status)
        {
        case VD_OP:         /* If operable */
            ss_array[count_ss++] = vid;         /* Store vid of snapshot */
            break;

        case VD_INOP:       /* If inoperable */
            break;

        default:
            fprintf(stderr, "%s: Snapshot %d not operational, status=%02X\n",
                __func__, vid, V_vddindx[vid]->status);
        }
    }
    if (count_ss == 0)  /* If no operable snapshots, go to source directly */
    {
        return FALSE;
    }

    ssms_ic = p_MallocC(sizeof(*ssms_ic), __FILE__, __LINE__);
    if (count_ss > (int)dimension_of(ssms_ic->ssmdep_cow_seg[0].ilts))
    {
        fprintf(stderr, "%s: Too many snapshots, count_ss=%d\n", __func__, count_ss);
        abort();
    }
    ssms_ic->ssmdep_cnt = count_ss;
    ssms_ic->ssmdep_max = count_ss;

    while (--count_ss >= 0)
    {
        ssms = gVDX.vdd[ss_array[count_ss]]->vd_incssms;
        do_write_ss_source(ssms, ssms_ic, ilt, sda);
    }
    return TRUE;
}   /* End of write_ss_source */


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/

