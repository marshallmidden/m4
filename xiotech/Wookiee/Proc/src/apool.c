/* $Id: apool.c 161678 2013-09-18 19:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       apool.c
**
**  @brief      Async pool manager
**
**  To provide support for the async replication pool vdisk. This includes
**  all management functions and variables
**
**  Copyright (c) 2007-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "async_nv.h"
#include "ccsm.h"
#include "kernel.h"
#include "LOG_Defs.h"
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
#include <time.h>
#include "pm.h"
#include "CT_defines.h"
#include "ecodes.h"
#include "apool.h"
#include "defbe.h"
#include "def.h"
#include "vdd.h"
#include "cor.h"
#include "dcd.h"
#include "scd.h"
#include "cm.h"
#include "lvm.h"
#include "qu.h"
#include "defbe.h"
#include "RL_PSD.h"
#include "RL_RDD.h"
#include "ldd.h"
#include "mem_pool.h"
#include "ddr.h"

// Comment this in if you want to fprintf debug messages to print out.
// #define NOT_WORKING

#ifdef NOT_WORKING
  // Print whole apool structure when shadow/head/tai/current changes, etc.
  // #define M4_EXTRA_PRINTING

  // If need to debug ILT/VRP queuing/completion/etc.
  #define ILT_PRINTING

  // If need to debug the mover task processing.
  #define DEBUG_MOVER_TASK

  // Following adds printing when head/tail{,.shadow} are updated.
#endif  /* NOT_WORKING */

//#define APOOL_HEAD_TAIL_UPDATING_PRINTING

// Some extra checks for things "bad" that are assumed to be ok.
#define M4_EXTRA_DEBUGGING

/*
******************************************************************************
** Private data structures
******************************************************************************
*/
typedef struct apool_hijack_save
{
    APOOL_PUT_RECORD *put_record;
    void   *hijack_completion_routine;
    VRP     hijack_vrp_saved;
} APOOL_HIJACK;

/*
******************************************************************************
** Private variables
******************************************************************************
*/
extern UINT8    gApoolActive;
extern UINT8    gAsyncApoolOwner;
APOOL           apool;
int             g_apool_initialized = FALSE;
static UINT8    last_event = 0xff;


UINT32          gApoolOwner = 0xffffffff;
APOOL_PUT_RECORD *last_errored_put_record = NULL;
UINT64          shadow_sequence_count = ~0ULL;
UINT64          loc_last_seq_count = ~0ULL;
UINT32          mover_que_depth = 0;
UINT32          mover_que_data;
UINT32          mover_que_limit = 4500;
UINT16          mover_task_status = 0;
UINT16          mover_task_waiting;
UINT16          ap_mover_paused = 0;
UINT16          ap_reject_puts = 0;
UINT16          deferred_unpause = FALSE;
UINT32          get_record_count = 0;
UINT32          base_max_que_depth = MOVER_MAX_QUE_DEPTH;
UINT32          apool_overlap_count = 0;
UINT64          first_overlap_time = 0;
UINT32          overlap_vid = 0xffff;
UINT32          copy_req_count = 0;


#ifdef APOOL_HEAD_TAIL_UPDATING_PRINTING
FILE   *dav_file= NULL;
#endif  /* APOOL_HEAD_TAIL_UPDATING_PRINTING */

/*
******************************************************************************
** Public variables defined in other files
******************************************************************************
*/
extern APOOL_NV_IMAGE gApoolnvImage;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static UINT8 apool_get_percent_full(UINT16 apool_id);

/*
******************************************************************************
** Public function prototypes - defined in other files
******************************************************************************
*/
extern void CCSM_Copy_ended(COR* cor);
extern void CM_InhibitPoll(COR*);
extern void CM_ResumePoll(COR*);

/* Forward reference. */
void apool_event_percent_full(UINT16 apool_id, UINT8 percent_full);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
 ******************************************************************************
 **
 ** @brief      Show alink count inconsistency
 **
 ** @param      count - Residual count value
 ** @param      alp - Residual pointer
 ** @param      func - Function that detected the inconsistency
 **
 ** @return     None
 **
 ******************************************************************************
 */
static void show_alink_inconsistency(UINT16 count, ALINK *alp, const char *func)
{
    static UINT16   reps = 0;
    static UINT16   saved_count = 0;
    static ALINK    *saved_alp = NULL;

    ++reps;
    if (saved_count != count || saved_alp != alp || reps > 500)
    {
        saved_count = count;
        saved_alp = alp;
        fprintf(stderr, "%s: alink_count inconsistency, al_count=%d, "
            "alink_head=%p, reps=%d\n", func, count, alp, reps);
        if (reps != 1)
        {
            reps = 1;
        }
    }
}

/**
 ******************************************************************************
 **
 **  @brief      apool_stop / start
 **
 **  This function will stop/pause the mover task
 **
 **  @return     None
 **
 ******************************************************************************
 */
void apool_stop(void)
{
    ap_mover_paused++;
    while(mover_que_depth != get_record_count)
    {
        TaskSleepMS(1);
    }
}

void apool_start(void)
{
    if(ap_mover_paused)
    {
        ap_mover_paused--;
    }
}

/**
 ******************************************************************************
 **
 **  @brief      apool_set/get/calc_max_ap_data
 **
 **  This function will set or get the value of the max_ap_data variable
 **
 **  @return     None
 **
 ******************************************************************************
 */
void apool_set_max_ap_data(UINT32 new_val)
{
    max_ap_data = new_val;
}

void apool_set_max_usr_ap_data(UINT32 new_val)
{
    max_usr_ap_data = new_val;
}

static UINT32 apool_calc_max_ap_data(void)
{
    UINT32 ret_val;

    // Keep 5 minutes of copy data in the buffer based on the last seconds
    // mover task flush rate.
    ret_val = apool.blocks_per_sec * 300;
    // Convert to MBytes
    ret_val = ret_val / 2048;

    // Bound the output between 1 GByte and max_usr_ap_data MBytes.
    // (unless max_usr_ap_data is less than 1 GB)
    if (ret_val < 1024)
    {
        ret_val = 1024;
    }

    if (ret_val > max_usr_ap_data)
    {
        ret_val = max_usr_ap_data;
    }

    if (!max_usr_ap_data)
    {
        // If the user has set the max_ap_data to 0 then return a really
        // big number, effectively turning off any copy throttling.
        ret_val = 0xfffffff;
    }
    return ret_val;
}

/**
 ******************************************************************************
 **
 **  @brief     Return size of all empty elements of apool
 **
 **  @param     apoolid - Number of apool
 **
 **  @return    Total size of empty elements
 **
 ******************************************************************************
 */
static UINT64   empty_element_size(UINT16 apoolid UNUSED)
{
    int i;
    UINT64  size = 0;

    for (i = 0; i < apool.element_count; i++)
    {
        if (apool.element[i].head == apool.element[i].tail)
        {
            size += apool.element[i].length;
        }
    }
    return size;
}

/**
 ******************************************************************************
 **
 **  @brief      apool_set_qd
 **
 **  This function sets the max mover task queuedepth allowed.
 **  The value passed in must be in the range of 2 to 200.
 **
 **  @return     None
 **
 ******************************************************************************
 */
void apool_set_qd(UINT32 new_val)
{
    if(new_val <= 200 && new_val >= 2)
    {
        base_max_que_depth = new_val;
    }
}

/**
 ******************************************************************************
 **
 **  @brief      apool_set_data_sz
 **
 **  This function sets the max mover task outstanding data (sectors) allowed.
 **  The value passed in must be in the range of 64KB and 4.5MB.
 **
 **  @return     None
 **
 ******************************************************************************
 */
void apool_set_data_sz(UINT32 new_val)
{
    if(new_val <= 9216 && new_val >= 128)
    {
        mover_que_limit = new_val;
    }
}

/**
******************************************************************************
**
**  @brief      Free all alinks
**
**  This function frees all alinks.
**
**  @return     None
**
******************************************************************************
*/

static void free_all_alinks(void)
{
    ALINK   *alp;
    INT16   al_count = apool.alink_count;

    while (apool.alink_head && al_count > 0)
    {
        alp = apool.alink_head->next;
        s_Free(apool.alink_head, sizeof(ALINK), __FILE__, __LINE__);
        apool.alink_head = alp;
        --al_count;
    }
    if (apool.alink_head != 0 || al_count != 0)
    {
        show_alink_inconsistency(al_count, apool.alink_head, __func__);
    }
    apool.alink_head = apool.alink_tail = NULL;
    apool.alink_count = 0;
}

/**
******************************************************************************
**
**  @brief      Put data into the apool
**
**  This function accepts an ILT/VRP and routes the request along with metadata
**  to the apool.
**
**
**  @param      dummy       - unused
**  @param      ilt         - ilt to use.
**  @param      apool_id    - Apool working on.
**
**  @return     Status, 0 for good, else bad
**
******************************************************************************
*/
UINT32 apool_put(UINT32 dummy UNUSED, ILT *ilt, UINT16 apool_id)
{
    SGL            *new_sgl;
    SGL            *old_sgl;
    SGL_DESC       *desc1;
    VRP            *vrp;
    APOOL_META_DATA *meta_data;
    APOOL_PUT_RECORD *put_record;
    ILT            *prev_ilt = ilt - 1;
    APOOL_HIJACK   *save_them;
    UINT16          func;
    static UINT8    not_enough_space_print = 0;
    static UINT32   event_counter = 0;

    // Check the reject flag and exit with error if set.  Also check to make
    // sure nobody is trying to read the alink
    vrp = (VRP *)prev_ilt->ilt_normal.w0;
    func = vrp->function & 0x1F;
    if (func == VRP_OUTPUT_VERIFY)
    {
        func = VRP_OUTPUT;
        vrp->function &= ~0x1F;
        vrp->function |= func;
    }
    if (ap_reject_puts || func != VRP_OUTPUT)
    {
        if (func != VRP_OUTPUT)
        {
            fprintf(stderr, "%s: Unexpected function %d\n",
                    __func__, vrp->function);
        }
        vrp = (VRP *)prev_ilt->ilt_normal.w0;
        vrp->status = EC_BUSY;
        return EC_BUSY;
    }

    // Check to make sure the shadow sequence count is initialized.
    if (shadow_sequence_count == ~0ULL)
    {
        shadow_sequence_count = apool.sequence_count;
    }

    // Allocate a new SGL that is big enough to hold one more descriptor than the original.
    if ((UINT32)vrp->pSGL == 0xfeedf00d)
    {
        fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
        abort();
    }
    old_sgl = vrp->pSGL;
    new_sgl = (SGL *)s_MallocW(old_sgl->size + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);

    // Allocate a structure to save hijacked values.
    save_them = (APOOL_HIJACK *)s_MallocW(sizeof(APOOL_HIJACK), __FILE__, __LINE__);

    // Get pointers
    desc1 = (SGL_DESC *)(new_sgl + 1);
    meta_data = (APOOL_META_DATA *)((UINT8*)new_sgl + old_sgl->size + sizeof(SGL_DESC));

    // Copy old SGL onto new one plus offset 2
    bcopy(old_sgl, (UINT8*)new_sgl + sizeof(SGL_DESC), old_sgl->size);

    // Save away original vrp values.
    bcopy(vrp, &save_them->hijack_vrp_saved, sizeof(VRP));
    // Clear completion routine, for possible structure viewing.
    save_them->hijack_completion_routine = (void *)NULL;
    // Clear put_record, for possible structure viewing.
    save_them->put_record = (APOOL_PUT_RECORD *)NULL;
    // Hijack gen2 for our purposes -- a structure holding original values.
    vrp->gen3 = (UINT32)save_them;

    // Fill in the new SGL information
    new_sgl->scnt = old_sgl->scnt + 1;
    new_sgl->owners = 0;
    new_sgl->flag = 0;
    new_sgl->size = old_sgl->size + sizeof(SGL_DESC);
    desc1->addr = (void *)meta_data;
    desc1->len = BYTES_PER_SECTOR;


    // Fill in the meta data
    meta_data->length = vrp->length;
    meta_data->sda = vrp->startDiskAddr;
    meta_data->vid = vrp->vid;
    meta_data->sequence_count = ++shadow_sequence_count;
    meta_data->attributes = (1 << TIME_STAMP_VALID);
    meta_data->dummy = 0;
    meta_data->time_stamp = (UINT32)(K_ii.time);

    // If this VRP is a copy request then set the meta data flag indicating so
    if(vrp->gen1 == SUPER_MAGIC_FLAG)
    {
        vrp->gen1 = 0;
        meta_data->attributes |= (1 << COPY_DATA);
//        fprintf(stderr,"Setting metadata copy_data flag.\n");
    }

    // Update SGL pointer with our new data.
    vrp->pSGL = new_sgl;
    vrp->sglSize = new_sgl->size;

    // Add 1 to the length for the meta data
    vrp->length += 1;

    // Clear the special processing bit in the function
    vrp->function &= (0xffff ^ (1 << VRP_SPECIAL));


    // Update the shadow head pointer so more writes can be queued
    if (apool_update_shadow_head(vrp, apool_id) != AOK)
    {
        apool_event_percent_full(apool.id, 100);
        // There was a problem with space in the apool so restore the VRP with
        // saved values, return VRP with an error and free newly allocated sgl
        // and saved values memory.
        if (not_enough_space_print == 0)
        {
            fprintf(stderr, "apool_put: not enough space in apool, return error.\n");
            not_enough_space_print = 1;
        }

        // Decrement the shadow sequence counter that was incremented above
        shadow_sequence_count--;

        // Restore original vrp values.
        bcopy(&save_them->hijack_vrp_saved, vrp, sizeof(VRP));
        vrp->status = EC_QUEUE_FULL;
        s_Free(new_sgl, new_sgl->size + BYTES_PER_SECTOR, __FILE__, __LINE__);
        s_Free(save_them, sizeof(APOOL_HIJACK), __FILE__, __LINE__);
        apool.status |= (1 << MR_APOOL_FULL);

        // Return error, let virtual handle error for previous ilt.
        return(EC_RETRY);
    }
    else
    {
        apool.status &= ~(1 << MR_APOOL_FULL);
        not_enough_space_print = 0;
        if(++event_counter%100)
        {
            apool_event_percent_full(apool.id, apool_get_percent_full(apool.id));
        }
    }


    // Assign a put record
    put_record = apool_assign_put_record(prev_ilt, apool_id);

    // The following is the vid and length of the original write to alink
    put_record->vid = meta_data->vid;
    put_record->length = meta_data->length;

    // Save away the put_record.
    save_them->put_record = put_record;

    // Save away the original completion routine
    save_them->hijack_completion_routine = prev_ilt->cr;
    // Replace the completion routine with mine.
    prev_ilt->cr = (void *)C_label_referenced_in_i960asm(apool_put_cr);

#ifdef ILT_PRINTING
fprintf(stderr, "apool_put: Queing to Virtual ILT=%p CR0=%p VRP=%p V_que=%p\n", prev_ilt, save_them->hijack_completion_routine, vrp, V_que);
#endif  /* ILT_PRINTING */

    return (AOK);
}

/**
******************************************************************************
**
**  @brief      Completion routine for writes to apool
**
**  This function cleans up after a write completes to the apool.
**
**  @param      dummy   - Unused.
**  @param      ilt     - The ilt with this completion routine.
**
**  @return     none
**
******************************************************************************
*/
void apool_put_cr(UINT32 dummy UNUSED, ILT *ilt)
{
    VRP            *vrp;
    SGL            *new_sgl;
    APOOL_PUT_RECORD *put_record;
    APOOL_HIJACK   *save_them;
    void           *original_cr;
    UINT8           save_vrp_status;
    UINT32          save_g0 = g0;
    UINT32          save_g1 = g1;

#ifdef M4_EXTRA_PRINTING
fprintf(stderr, "apool_put_cr ILT %p\n", ilt);
#endif /* M4_EXTRA_PRINTING */

    vrp = (VRP *)ilt->ilt_normal.w0;
    save_vrp_status = vrp->status;

    // Get the location of all saved values.
    save_them = (APOOL_HIJACK *)vrp->gen3;
    put_record = save_them->put_record;
    put_record->state |= (1 << REQUEST_COMPLETE);
    if ((UINT32)vrp->pSGL == 0xfeedf00d)
    {
        fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
        abort();
    }
    new_sgl = vrp->pSGL;

    s_Free(new_sgl, new_sgl->size + BYTES_PER_SECTOR, __FILE__, __LINE__);

    // Restore original vrp values.
    bcopy(&save_them->hijack_vrp_saved, vrp, sizeof(VRP));
    original_cr = save_them->hijack_completion_routine;
    s_Free(save_them, sizeof(APOOL_HIJACK), __FILE__, __LINE__);

#ifdef ILT_PRINTING
fprintf(stderr, "apool_put_cr: ILT %p, VRP %p\n", ilt, vrp);
#endif  /* ILT_PRINTING */

    //Release the put record
    put_record->status = save_vrp_status;
    put_record->old_cr = original_cr;
    apool_release_put_record(put_record, original_cr);

    if (save_vrp_status == AOK)
    {
        apool.status &= ~(1 << MR_APOOL_BAD_APOOL);
    }

    g0 = save_g0;
    g1 = save_g1;
}

/**
******************************************************************************
**
**  @brief      Initializes the apool.
**
**  This function initialized the apool, tasks, and managing variables.
**
**  @param      apool_id    - The apool we are working with.
**
**  @return     none
**
**  @attention  register g3 is changed when CCSM_Copy_ended is called via
**              apool_break_async_mirrors.
**              This is ok on: 2007-10-12 -- Marshall Midden
**
******************************************************************************
*/
void apool_init(UINT16 apool_id, UINT16 apool_vid)
{
    VDD            *vdd;
    int             i = 0;

#ifdef APOOL_HEAD_TAIL_UPDATING_PRINTING
    if (dav_file == NULL)
    {
        dav_file = fopen("/var/log/xiotech/dav.log","w");
    }
#endif  /* APOOL_HEAD_TAIL_UPDATING_PRINTING */

    if (!g_apool_initialized)
    {
        apool.version = ASYNC_COMPAT_VERSION;

        vdd = gVDX.vdd[apool_vid];

        // Try to recover the NVRAM
        AR_RecoverApoolNVImage(0);

        // Free any existing alink structures
        free_all_alinks();

        if (gAsyncNVRecovered)
        {
            //NVRAM was recoverd so stuff the values
            apool.id = gApoolnvImage.header.apool_id;
            apool.status = gApoolnvImage.header.status;
            apool.cur_head_element = gApoolnvImage.header.cur_head_element;
            apool.cur_tail_element = gApoolnvImage.header.cur_tail_element;
            apool.head_shadow = gApoolnvImage.element[gApoolnvImage.header.cur_head_element].head;
            apool.tail_shadow = gApoolnvImage.element[gApoolnvImage.header.cur_tail_element].tail;
            apool.cur_head_shadow_element = gApoolnvImage.header.cur_head_element;
            apool.cur_tail_shadow_element = gApoolnvImage.header.cur_tail_element;
            apool.get_record_head = NULL;
            apool.get_record_tail = NULL;
            apool.length = vdd->devCap;
            apool.sequence_count = gApoolnvImage.header.sequence_count;
            apool.last_seq_count = gApoolnvImage.header.last_seq_count;
            loc_last_seq_count = apool.last_seq_count;
            apool.mover_task_pcb = NULL;
            apool.element_count = gApoolnvImage.header.element_count;
            apool.alink_count = 0;          // gApoolnvImage.header.alink_count;
            apool.version = gApoolnvImage.header.version;
            for (i = 0; i < MAX_ELEMENTS; i++)
            {
                apool.element[i].apool_id = apool.id;
                apool.element[i].status = gApoolnvImage.element[i].status;
                apool.element[i].vid = gApoolnvImage.element[i].vid;
                apool.element[i].jump_to_element = gApoolnvImage.element[i].jump_to_element;
                apool.element[i].length = gApoolnvImage.element[i].length;
                apool.element[i].sda = gApoolnvImage.element[i].sda;
                apool.element[i].head = gApoolnvImage.element[i].head;
                apool.element[i].tail = gApoolnvImage.element[i].tail;
                apool.element[i].jump_offset = gApoolnvImage.element[i].jump_offset;
            }
        }
        else
        {
            // NVRAM was not recovered
            memset(&apool,0,sizeof(APOOL));

            // At this point the there is an error condition with the NVRAM recovery
            //so we need to break all mirrors.
fprintf(stderr, "apool_init: No NVRAM, breaking all mirrors.\n");
            apool_break_async_mirrors(0xffff);

            // Initialize the apool structure
            apool_init_struct(apool_id,apool_vid);
        }

        // Construct alink structures for each alink existing in the system
        for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
        {
            if ((gVDX.vdd[i] != NULL) && (gVDX.vdd[i]->pDCD != NULL))
            {
               if (BIT_TEST(gVDX.vdd[i]->attr, VD_BASYNCH) && BIT_TEST(gVDX.vdd[i]->attr,VD_BVLINK))
               {
                  alink_init(i);
               }
            }
        }

        // Set a status bit to indicate that initialization is in progress.
        // This will signal the apool_put routine to reject all incoming requests.
        ap_reject_puts |= (1 << MR_REJECT_PUTS_INIT);

        // Validate the apool once the virtual layer is online
        while (V_exec_qu.pcb == NULL)
        {
fprintf(stderr, "apool_init: waiting for virtual exec.\n");
            TaskSleepMS(1000);
        }
fprintf(stderr, "apool_init: calling apool_validate.\n");
        get_record_count = 0;
        apool_validate(apool_vid);

        CT_fork_tmp = (unsigned long)"Mover Task";
        apool.mover_task_pcb = (PCB *)-1;       // Flag task being created.
        apool.mover_task_pcb = TaskCreate2(C_label_referenced_in_i960asm(mover_task), MOVER_PRIORITY);
        CT_fork_tmp = (unsigned long)"Mover Stats Task";
        apool.stats_task_pcb = TaskCreate2(C_label_referenced_in_i960asm(mover_stats_task), STATS_PRIORITY);

        logAPOOLevent(AP_MOVER_STARTED, LOG_AS_INFO, 0, 0, 0);
    }
    g_apool_initialized = TRUE;
    last_event = 0xff;
    // Let the virtual requests come into the apool now.
    ap_reject_puts = 0;

#ifdef M4_EXTRA_PRINTING
PRINT_APOOL_STRUCTURE;
#endif /* M4_EXTRA_PRINTING */
}

/**
******************************************************************************
**
**  @brief      Initializes the apool structure.
**
**  This function initialized the apool structure only.
**
**  @param      apool_id    - The apool we are working with.
**
**  @return     none
**
**
******************************************************************************
*/
void apool_init_struct(UINT16 apool_id, UINT16 apool_vid)
{
    VDD            *vdd;
    int             i = 0;
    time_t          cur_time;

    apool.version = ASYNC_COMPAT_VERSION;

    vdd = gVDX.vdd[apool_vid];

    memset(&apool,0,sizeof(APOOL));
    apool.id = apool_id;
    apool.length = vdd->devCap;
    apool.element_count = 1;
    apool.element[0].vid = apool_vid;
    apool.element[0].length = vdd->devCap;
    apool.last_seq_count = ~0ULL;
    shadow_sequence_count = ~0ULL;
    loc_last_seq_count = ~0ULL;
    mover_que_depth = 0;
    mover_task_status = 0;
    ap_mover_paused = 0;
    ap_reject_puts = 0;
    g_apool_initialized = FALSE;

    // Seed the sequence count with the current time shifted
    // up into the upper bits of the sequence count.
    time(&cur_time);
    cur_time = (cur_time - (cur_time & 0x10000000) + 0x10000000) & SEQ_CNT_OVERFLOW_MASK;
    apool.sequence_count = (UINT64)((UINT64)(cur_time) << 32);

    for (i = 0; i < MAX_ELEMENTS; i++)
    {
        apool.element[i].jump_to_element = ~0;
        apool.element[i].jump_offset = ~0ULL;
    }

    // Set a status bit to indicate that initialization is in progress.
    // This will signal the apool_put routine to reject all incoming requests.
    ap_reject_puts |= (1 << MR_REJECT_PUTS_INIT);

    // Finally save the new data to NVRAM.
    AR_NVUpdate(AR_UPDATE_ALL);
#ifdef M4_EXTRA_PRINTING
PRINT_APOOL_STRUCTURE;
#endif /* M4_EXTRA_PRINTING */
}

/**
******************************************************************************
**
**  @brief      Get the shadow head pointer of the apool
**
**  This function returns the value in sectors of the head pointer. The head
**  pointer is the insertion point for incoming data.
**
**  @param      vid         - Pointer to vid for current shadow head.
**  @param      apool_id    - the apool we are working on.
**
**  @return     shadow head pointer, and returns vid.
**
******************************************************************************
*/
UINT64 apool_get_shadow_head(UINT16 *vid, UINT16 apool_id UNUSED)
{
    *vid = apool.element[apool.cur_head_shadow_element].vid;
    return (apool.head_shadow + apool.element[apool.cur_head_shadow_element].sda);
}

/**
******************************************************************************
**
**  @brief      Get the head pointer of the apool
**
**  This function returns the value in sectors of the head pointer. The head
**  pointer is the insertion point for incoming data.
**
**  @param      vid         - Pointer to vid for current shadow head.
**  @param      apool_id    - the apool we are working on.
**
**  @return     shadow head pointer, and returns vid.
**
******************************************************************************
*/
UINT64 apool_get_head(UINT16 *vid, UINT16 apool_id UNUSED)
{
    *vid = apool.element[apool.cur_head_element].vid;
    return (apool.element[apool.cur_head_element].head + apool.element[apool.cur_head_element].sda);
}

/**
******************************************************************************
**
**  @brief      Get the tail pointer of the apool
**
**  This function the value in sectors of the tail pointer. The tail pointer
**  is the data extraction point.
**
**  @param      vid         - Pointer to vid for current tail element.
**  @param      apool_id    - the apool we are working on.
**
**  @return     current tail pointer, and returns vid.
**
******************************************************************************
*/
UINT64 apool_get_tail(UINT16 *vid, UINT16 apool_id UNUSED)
{
    *vid = apool.element[apool.cur_tail_element].vid;
    return (apool.element[apool.cur_tail_element].tail +
        apool.element[apool.cur_tail_element].sda);
}

/**
******************************************************************************
**
**  @brief      Get the tail shadow pointer of the apool
**
**              This function the value in sectors of the tail shodow pointer.
**              The tail pointer is the data extraction point.
**
**  @param      vid         - Pointer to vid for current tail shadow element.
**  @param      apool_id    - the apool we are working on.
**
**  @return     tail shadow pointer, and returns vid.
**
******************************************************************************
*/
UINT64 apool_get_shadow_tail(UINT16 *vid, UINT16 apool_id UNUSED)
{
    if ((apool.element[apool.cur_tail_shadow_element].length - apool.tail_shadow)< SLOP_SECTORS)
    {
        // If the shadow tail was in the slop sectors zone then wrap it around.
        apool.tail_shadow = 0ULL;
    }
    *vid = apool.element[apool.cur_tail_shadow_element].vid;
    return (apool.tail_shadow + apool.element[apool.cur_tail_shadow_element].sda);
}

/**
******************************************************************************
**
**  @brief      Update the tail pointer of the apool
**
**  This function will update both the RAM copy and the NVRAM copy of the
**  apool tail pointer
**
**  @param      get_record  - The APOOL_GET_RECORD.
**  @param      apool_id    - The apool we are working on.
**
**  @return     none
**
******************************************************************************
*/
void apool_update_tail(APOOL_GET_RECORD *get_record, UINT16 apool_id UNUSED)
{
    UINT64          req_length;

    //Adjust the length to reflect the added sector of metadata
    req_length = get_record->length + 1UL;
    if (apool.element[apool.cur_tail_element].tail != apool.element[apool.cur_tail_element].head)
    {
        if ((apool.element[apool.cur_tail_element].length -
           apool.element[apool.cur_tail_element].tail) < SLOP_SECTORS)
        {
            apool.element[apool.cur_tail_element].tail = 0ULL;
        }

    }

    if (apool.element[apool.cur_tail_element].tail >
        apool.element[apool.cur_tail_element].head)
    {
        //The tail is greater than the head (case 1)
        // Note: no equal check, otherwise makes circular queue empty.
        if ((apool.element[apool.cur_tail_element].length -
             (apool.element[apool.cur_tail_element].tail + req_length)) >= SLOP_SECTORS)
        {
            //There is enough room to advance the tail
            apool.element[apool.cur_tail_element].tail += req_length;
        }
        else
        {
            // Tail must wrap around to the top.
            apool.element[apool.cur_tail_element].tail = 0ULL;
            if (apool.element[apool.cur_tail_element].head ==
                apool.element[apool.cur_tail_element].tail)
            {
                // The head and tail are equal check to see if the element was previously full
                if (apool.element[apool.cur_tail_element].status & (1 << MR_APOOL_ELEMENT_FULL))
                {
                    // Clear the full flag since the element is now empty
                    apool.element[apool.cur_tail_element].status &= ~(1 << MR_APOOL_ELEMENT_FULL);

                    // Only jump if there is another element to work on.
                    if (apool.cur_tail_element != apool.cur_head_element)
                    {
                        // The element was full so tail must jump to the next element if available.
                        apool.cur_tail_element = apool.element[apool.cur_tail_element].jump_to_element;
                        apool.element[apool.cur_head_element].jump_to_element = ~0;
                        if ((INT64)(apool.element[apool.cur_tail_element].length
                            - apool.element[apool.cur_tail_element].tail) < SLOP_SECTORS)
                        {
                            apool.element[apool.cur_tail_element].tail = 0ULL;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Head is greater than tail (Case 2)
        apool.element[apool.cur_tail_element].tail += req_length;

        if (apool.element[apool.cur_tail_element].head ==
            apool.element[apool.cur_tail_element].tail)
        {
            // The head and tail are equal check to see if the element was previously full
            if (apool.element[apool.cur_tail_element].status & (1 << MR_APOOL_ELEMENT_FULL))
            {
                // Clear the full flag since the element is now empty
                apool.element[apool.cur_tail_element].status &= ~(1 << MR_APOOL_ELEMENT_FULL);

                // Only jump if there is another element to work on.
                if (apool.cur_tail_element != apool.cur_head_element)
                {
                    // The element was full so tail must jump to the next element if available.
                    apool.cur_tail_element = apool.element[apool.cur_tail_element].jump_to_element;
                    apool.element[apool.cur_head_element].jump_to_element = ~0;
                    if ((INT64)(apool.element[apool.cur_tail_element].length
                        - apool.element[apool.cur_tail_element].tail) < SLOP_SECTORS)
                    {
                        apool.element[apool.cur_tail_element].tail = 0ULL;
                    }
                }
            }
        }
    }

    /* Update the apool full flag if there is only one element. */
    if(apool.element_count == 1)
    {
        apool.status &= ~(1 << MR_APOOL_FULL);
    }
    AR_NVUpdate(AR_UPDATE_TAIL);

#ifdef APOOL_HEAD_TAIL_UPDATING_PRINTING
    fprintf(dav_file,"T , H %d:0x%-16.16llx, T %d:0x%-16.16llx, SH %d:0x%-16.16llx, ST %d:0x%-16.16llx, SQCNT 0x%-16.16llx\n",
        apool.cur_head_element, apool.element[apool.cur_head_element].head,
        apool.cur_tail_element, apool.element[apool.cur_tail_element].tail,
        apool.cur_head_shadow_element, apool.head_shadow,
        apool.cur_tail_shadow_element, apool.tail_shadow,
        get_record->sequence_count);
#endif  /* APOOL_HEAD_TAIL_UPDATING_PRINTING */
}

/**
******************************************************************************
**
**  @brief      Update the tail shadow pointer of the apool
**
**  This function will update the RAM copy of the apool tail pointer.
**
**  @param      get_record  - The APOOL_GET_RECORD.
**  @param      apool_id    - The apool we are working on.
**
**  @return     none
**
******************************************************************************
*/
void apool_update_shadow_tail(APOOL_GET_RECORD *get_record, UINT16 apool_id UNUSED)
{
    UINT64          req_length;

    //Adjust the length to reflect the added sector of metadata
    req_length = get_record->length + 1UL;


    if (apool.tail_shadow > apool.element[apool.cur_tail_shadow_element].head)
    {
        //The shadow tail is greater than the head (case 1)
        if ((INT64)(apool.element[apool.cur_tail_shadow_element].length - (apool.tail_shadow + req_length)) >= SLOP_SECTORS)
        {
            //There is enough room to advance the shadow tail
            apool.tail_shadow += req_length;
        }
        else
        {
            // Tail shadow must wrap around to the top.
            apool.tail_shadow = 0ULL;
            if (apool.element[apool.cur_tail_shadow_element].head == apool.tail_shadow)
            {
                // The head and tail are equal check to see if the element was previously full
                if (apool.element[apool.cur_tail_shadow_element].status & (1 << MR_APOOL_ELEMENT_FULL))
                {
                    // Only jump if there is another element to work on.
                    if (apool.cur_tail_shadow_element != apool.cur_head_element)
                    {
                        apool.cur_tail_shadow_element = apool.element[apool.cur_tail_shadow_element].jump_to_element;

                        // Update the shadow to the tail of the element jumped to.
                        apool.tail_shadow = apool.element[apool.cur_tail_shadow_element].tail;

                        if ((INT64)(apool.element[apool.cur_tail_shadow_element].length
                            - apool.tail_shadow) < SLOP_SECTORS)
                        {
                            apool.tail_shadow = 0ULL;
                        }
                     }
                }
            }
        }
    }
    else
    {
        // Head is greater than tail (Case 2)
        apool.tail_shadow += req_length;
        if (apool.element[apool.cur_tail_shadow_element].head == apool.tail_shadow)
        {
            // The head and tail are equal check to see if the element was previously full
            if (apool.element[apool.cur_tail_shadow_element].status & (1 << MR_APOOL_ELEMENT_FULL))
            {
                // Only jump if there is another element to work on.
                if (apool.cur_tail_shadow_element != apool.cur_head_element)
                {
                    apool.cur_tail_shadow_element = apool.element[apool.cur_tail_shadow_element].jump_to_element;

                    // Update the shadow to the tail of the element jumped to.
                    apool.tail_shadow = apool.element[apool.cur_tail_shadow_element].tail;

                    if ((INT64)(apool.element[apool.cur_tail_shadow_element].length
                        - apool.tail_shadow) < SLOP_SECTORS)
                    {
                        apool.tail_shadow = 0ULL;
                    }
                }
            }
        }
    }
#ifdef APOOL_HEAD_TAIL_UPDATING_PRINTING
fprintf(dav_file,"ST, H %d:0x%-16.16llx, T %d:0x%-16.16llx, SH %d:0x%-16.16llx, ST %d:0x%-16.16llx, SQCNT 0x%-16.16llx\n",
    apool.cur_head_element, apool.element[apool.cur_head_element].head,
    apool.cur_tail_element, apool.element[apool.cur_tail_element].tail,
    apool.cur_head_shadow_element, apool.head_shadow,
    apool.cur_tail_shadow_element, apool.tail_shadow,
    loc_last_seq_count);
#endif  /* APOOL_HEAD_TAIL_UPDATING_PRINTING */
}

/**
******************************************************************************
**
**  @brief      Update the apool head pointer
**
**  This function accepts a length parameter and adds it to the current head
**  pointer.
**
**  @param      length      - The length of the data being added to apool.
**  @param      apool_id    - The apool we are working on.
**
**  @return     none
**
******************************************************************************
*/
void apool_update_head(UINT64 length, UINT16 apool_id UNUSED)
{
    UINT64          tmp_tail;
    UINT64          tmp_head;
    UINT64          tmp_size;

    // Check to see that the movers last_sequence_count has been initialized.
    // If not then set it to the current apool sequence count -1.  This enables
    // the mover to have a known starting value
    if (apool.last_seq_count == ~0ULL)
    {
        apool.last_seq_count = apool.sequence_count - 1;
    }
    if (loc_last_seq_count == ~0ULL)
    {
        loc_last_seq_count = apool.sequence_count - 1;
    }

    // Check the current value of head pointer to see if this is the spot where the
    // shadow head jumped to the next element.  If so then the head must follow.
    if (apool.element[apool.cur_head_element].head ==
        apool.element[apool.cur_head_element].jump_offset)
    {
        // Mark this element as full and clear the jump offset
        apool.element[apool.cur_head_element].status |= (1 << MR_APOOL_ELEMENT_FULL);
        apool.element[apool.cur_head_element].jump_offset = ~0ULL;

        apool.cur_head_element = apool.element[apool.cur_head_element].jump_to_element;
    }

    // Get the pointers for the current head element.
    tmp_tail = apool.element[apool.cur_head_element].tail;
    tmp_head = apool.element[apool.cur_head_element].head;
    tmp_size = apool.element[apool.cur_head_element].length;

    if (tmp_tail > tmp_head)
    {
        // Tail is greater than head. Advance the head.
        apool.element[apool.cur_head_element].head = tmp_head + length;
    }
    else
    {
        // head is greater than or equal to the tail
        /* Make sure to leave SLOP_SECTORS at end of disk. */
        if ((INT64)(tmp_size - tmp_head) >= SLOP_SECTORS)
        {
            // Advance the head
            apool.element[apool.cur_head_element].head = tmp_head + length;
        }
        else
        {
            /* Must wrap head, because there is not enough slop. */
            if ((INT64)(tmp_tail - length) > 0)
            {
                // Wrap the head around
                apool.element[apool.cur_head_element].head = length;
            }
        }
    }
    AR_NVUpdate(AR_UPDATE_HEAD);
#ifdef APOOL_HEAD_TAIL_UPDATING_PRINTING
fprintf(dav_file,"H , H %d:0x%-16.16llx, T %d:0x%-16.16llx, SH %d:0x%-16.16llx, ST %d:0x%-16.16llx, SQCNT 0x%-16.16llx\n",
    apool.cur_head_element, apool.element[apool.cur_head_element].head,
    apool.cur_tail_element, apool.element[apool.cur_tail_element].tail,
    apool.cur_head_shadow_element, apool.head_shadow,
    apool.cur_tail_shadow_element, apool.tail_shadow,
    apool.sequence_count);
#endif  /* APOOL_HEAD_TAIL_UPDATING_PRINTING */
}

/**
******************************************************************************
**
**  @brief      Update the apool shadow head pointer.
**
**  This function accepts a length parameter and adds it to the current
**  shadow head pointer.
**
**  @param      length      - The length to increment the shadow head pointer.
**  @param      apool_id    - The apool we are working on.
**
**  @return     AOK         - If everything is ok.
**  @return     else error.
**
******************************************************************************
*/
UINT16 apool_update_shadow_head(VRP *vrp, UINT16 apool_id)
{
    UINT64          tmp_tail;
    UINT64          tmp_sh_head;
    UINT64          tmp_size;
    UINT16          tmp_elem;
    UINT16          ret_val = AOK;

    tmp_tail = apool.element[apool.cur_head_shadow_element].tail;
    tmp_sh_head = apool.head_shadow;
    tmp_size = apool.element[apool.cur_head_shadow_element].length;

    /* Different checks depending on tail location relative to head. */
    if (tmp_tail > tmp_sh_head)
    {
        // Tail is greater than head
        // Note: no equal check, otherwise makes circular queue empty.
        if ((tmp_sh_head + vrp->length) < tmp_tail)
        {
            // Get vdisk and offset in vdisk as to where to write information.
            vrp->startDiskAddr = apool_get_shadow_head(&vrp->vid, apool_id);

            // Advance the head
            apool.head_shadow = tmp_sh_head + vrp->length;
        }
        else
        {
            apool.element[apool.cur_head_shadow_element].status |= (1 << MR_APOOL_ELEMENT_FULL);

            tmp_elem = find_first_empty();

            // Check to see if proposed element is completely empty.
            if (tmp_elem == MAX_ELEMENTS )
            {
                // The proposed element is not empty, don't jump to it, apool full.
                ret_val = APOOL_FULL_ERROR;
                apool.status |= (1 << MR_APOOL_FULL);
                apool.element[apool.cur_head_shadow_element].jump_offset = ~0ULL;
                apool.element[apool.cur_head_shadow_element].jump_to_element = ~0;
            }
            else
            {
                // Set the shadow head jump offset.  This will indicate to the update head
                // routine when to jump to the next element.
                if(apool.element[apool.cur_head_shadow_element].jump_to_element != tmp_elem)
                {
                    apool.element[apool.cur_head_shadow_element].jump_offset = tmp_sh_head;
                }
                // Clear the full flag
                apool.status &= ~(1 << MR_APOOL_FULL);

                // Advance the element and update the shadow pointer to new elem head
                apool.element[apool.cur_head_shadow_element].jump_to_element = tmp_elem;
                apool.cur_head_shadow_element = tmp_elem;
                apool.head_shadow = apool.element[apool.cur_head_shadow_element].head;

                // Get vdisk and offset in vdisk as to where to write information.
                vrp->startDiskAddr = apool_get_shadow_head(&vrp->vid, apool_id);

                    // Advance the shadow head pointer in the new element.  Care must be taken
                    // to wrap the pointer if need be.
                    if ((apool.element[apool.cur_head_shadow_element].length -
                        apool.head_shadow) < SLOP_SECTORS)
                    {
                        apool.head_shadow = 0ULL;
                        // Update the vrp with the new address
                        vrp->startDiskAddr = apool_get_shadow_head(&vrp->vid, apool_id);
                    }
                    apool.head_shadow += vrp->length;
            }
            // Update NVRAM to store the jump_offset
            AR_NVUpdate(AR_UPDATE_HEAD);
        }
    }
    else
    {
        // head is greater than or equal to the tail
        /* Make sure to leave SLOP_SECTORS at end of disk. */
        if ((INT64)(tmp_size - tmp_sh_head) >= SLOP_SECTORS)
        {
            // Get vdisk and offset in vdisk as to where to write information.
            vrp->startDiskAddr = apool_get_shadow_head(&vrp->vid, apool_id);

            // Enough slop, advance the shadow head
            apool.head_shadow = tmp_sh_head + vrp->length;
        }
        else
        {
            /* Must wrap head, because there is not enough slop. */
            /* Is there enough room before tail to hold this data? */
            /* NOTE: no equal check, otherwise makes circular queue empty. */
            if ((INT64)(tmp_tail - vrp->length) > 0)
            {
                // Wrap the shadow head around.
                apool.head_shadow = 0ULL;
                // Get vdisk and offset in vdisk as to where to write information.
                vrp->startDiskAddr = apool_get_shadow_head(&vrp->vid, apool_id);
                apool.head_shadow += vrp->length;
            }
            else
            {
                apool.element[apool.cur_head_shadow_element].status |= (1 << MR_APOOL_ELEMENT_FULL);

                tmp_elem = find_first_empty();

                // Check to see if proposed element is completely empty.
                if (tmp_elem == MAX_ELEMENTS)
                {
                    // The proposed element is not empty, don't jump to it, apool full.
                    ret_val = APOOL_FULL_ERROR;
                    apool.status |= (1 << MR_APOOL_FULL);
                    apool.element[apool.cur_head_shadow_element].jump_offset = ~0ULL;
                    apool.element[apool.cur_head_shadow_element].jump_to_element = ~0;
                }
                else
                {

                    // Set the shadow head jump offset.  This will indicate to the update head
                    // routine when to jump to the next element.
                    if(apool.element[apool.cur_head_shadow_element].jump_to_element != tmp_elem)
                    {
                        apool.element[apool.cur_head_shadow_element].jump_offset = tmp_sh_head;
                    }

                    // Clear the full flag
                    apool.status &= ~(1 << MR_APOOL_FULL);

                    // Advance the element and update the shadow pointer
                    apool.element[apool.cur_head_shadow_element].jump_to_element = tmp_elem;
                    apool.cur_head_shadow_element = tmp_elem;
                    apool.head_shadow = apool.element[apool.cur_head_shadow_element].head;

                    // Get vdisk and offset in vdisk as to where to write information.
                    vrp->startDiskAddr = apool_get_shadow_head(&vrp->vid, apool_id);

                    // Advance the shadow head pointer in the new element.  Care must be taken
                    // to wrap the pointer if need be.
                    if ((apool.element[apool.cur_head_shadow_element].length -
                        apool.head_shadow) < SLOP_SECTORS)
                    {
                        apool.head_shadow = 0ULL;
                        // Update the vrp with the new address
                        vrp->startDiskAddr = apool_get_shadow_head(&vrp->vid, apool_id);
                    }
                    apool.head_shadow += vrp->length;
                }
                // Update NVRAM to store the jump_offset and flags
                AR_NVUpdate(AR_UPDATE_HEAD);
            }
        }
    }
#ifdef APOOL_HEAD_TAIL_UPDATING_PRINTING
  if (ret_val == AOK)
  {
    fprintf(dav_file,"SH, H %d:0x%-16.16llx, T %d:0x%-16.16llx, SH %d:0x%-16.16llx, ST %d:0x%-16.16llx, SQCNT 0x%-16.16llx\n",
        apool.cur_head_element, apool.element[apool.cur_head_element].head,
        apool.cur_tail_element, apool.element[apool.cur_tail_element].tail,
        apool.cur_head_shadow_element, apool.head_shadow,
        apool.cur_tail_shadow_element, apool.tail_shadow,
        shadow_sequence_count);
  }
#endif  /* APOOL_HEAD_TAIL_UPDATING_PRINTING */
    return (ret_val);
}

/**
******************************************************************************
**
**  @brief      Get the number sectors of data in the apool.
**
**  This function returns the number of sectors between the shadow tail and
**  the head pointers to find the number of sectors of data and metadata in
**  the apool (approximately, potential of wrapped SLOP_SECTORS added).
**
**  @param      apool_id    - The apool we are working on.
**
**  @return     The length of work (number of sectors) on the apool.
**
******************************************************************************
*/
UINT64 apool_work_2_do(UINT16 apool_id UNUSED)
{
    UINT16          element;
    UINT64          total_work;

    element = apool.cur_tail_shadow_element;

    // Initialize the work length for the current shadown tail element, before loop.
    // Nothing to do in this element.
    if (apool.tail_shadow == apool.element[element].head)
    {
        total_work = 0;
    }
    else if (apool.tail_shadow > apool.element[element].head)
    {
        // This actually counts the slop area as work
        total_work = apool.element[element].length - apool.tail_shadow;
        total_work += apool.element[element].head;
    }
    else
    {
        total_work = apool.element[element].head - apool.tail_shadow;
    }

    /* Get length used in all other element queues. */
    while (1)
    {
        element++;
        if (element >= apool.element_count)
        {
            element = 0;
        }
        if (element == apool.cur_tail_shadow_element)
        {
            break;
        }
        // The shadow tail pointer is not in this element
        if (apool.element[element].head == apool.element[element].tail)
        {
            /* total_work += 0; */
        }
        else if (apool.element[element].tail > apool.element[element].head)
        {
            total_work += apool.element[element].length - apool.element[element].tail;
            total_work += apool.element[element].head;
        }
        else
        {
            total_work += apool.element[element].head - apool.element[element].tail;
        }
    }
    return (total_work);
}

/**
******************************************************************************
**
**  @brief      Start/pause vdisks that are alinks.
**
**  @param      apool_id    - The apool we are working on.
**  @param      funct       - Routine to do the real work.
**
**  @return     none
**
******************************************************************************
*/
static void apool_auto_alinks_start_or_pause(UINT16 apool_id UNUSED, void(funct)(COR*))
{
    UINT16  index_D;
    VDD    *pVDD;
    COR    *pCOR;
    CM     *pCM;

    /* Loop through all vdisks looking for an ALink. */
    for (index_D = 0; index_D < MAX_VIRTUAL_DISKS; index_D++)
    {
        pVDD = gVDX.vdd[index_D];
        if (pVDD != NULL)
        {
            /* If a vlink  and the async attribute is set. */
            if (BIT_TEST(pVDD->attr, VD_BVLINK) &&
                BIT_TEST(pVDD->attr, VD_BASYNCH))
            {
                /* Proceed only if the VDD is a destination copy device and */
                /* the Copy registration exists. */
                if ((DCD *)(pVDD->pDCD)== NULL)
                {
                    continue;
                }
                /* Get the cor address related to this mirror. */
                pCOR = ((DCD *)(pVDD->pDCD))->cor;
                if (pCOR == NULL)
                {
                    continue;
                }
                /* Don't process if mirror is user paused. */
                if(pCOR->crstate == CRST_USERSUSP)
                {
                    continue;
                }
                /* Get the CM address from the cor. */
                pCM = pCOR->cm;
                if (pCM == NULL)
                {
                    continue;
                }
                /* And finally pass cor address as g3 to the .as code. */
/* #       g3 = COR address of local copy operation to process */
/* Save and restore them, "just in case". No registers are changed by routines. */
                UINT32 save_g3 = g3;
                funct(pCOR);
                g3 = save_g3;
            }
        }
    }
}   /* End of apool_auto_alinks_start_or_pause */

/**
******************************************************************************
**
**  @brief      Pause vdisks that are alinks, buffer getting full.
**              Actual auto-pause will be done by Copy manager when the writes
**              to apool fail based on the ap_reject_puts flag we set in this
**              function. The CM_InhibitPoll function just pauses polling done by
**              CM when it goes auto-pause state. The polling will be resumed
**              when we want to start the copy when the apool becomes free.
**
**
**  @param      apool_id    - The apool we are working on.
**
**  @return     none
**
******************************************************************************
*/
static void apool_auto_pause_alinks(UINT16 apool_id)
{
    apool_auto_alinks_start_or_pause(apool_id, CM_InhibitPoll);
    ap_reject_puts |= (1 << MR_REJECT_PUTS_AP);
}   /* End of apool_auto_pause_alinks */

/**
******************************************************************************
**
**  @brief      Start vdisks that are alinks, buffer getting empty.
**
**  If buffers are empty, call cm$pksnd_local_poll to start data transfer.
**
**  @param      apool_id    - The apool we are working on.
**
**  @return     none
**
******************************************************************************
*/
static void apool_auto_start_alinks(UINT16 apool_id)
{
    if(ap_reject_puts & (1 << MR_REJECT_PUTS_ERR))
    {
        deferred_unpause = TRUE;
    }
    else
    {
        apool_auto_alinks_start_or_pause(apool_id, CM_ResumePoll);
        ap_reject_puts = 0;
    }
}

/**
******************************************************************************
**
**  @brief      Get the percent full of the apool.
**
**  This function returns the percent full of the apool. It only considers
**  the usable areas of the snappool, which are the empty elements and
**  the current element being filled at the shadow head. This gives a more
**  useful idea of how close the apool is to stopping further requests,
**  regardless of the actual capacity.
**
**  @param      apool_id    - The apool we are working on.
**
**  @return     percent full
**
******************************************************************************
*/
static UINT8 apool_get_percent_full(UINT16 apool_id)
{
    UINT32  percent_free;
    UINT64  avail_free;
    UINT64  avail_space;
    APOOL_ELEMENT   *ele;

    avail_free = avail_space = empty_element_size(apool_id);

    ele = &apool.element[apool.cur_head_shadow_element];

    avail_space += ele->length;
    if (apool.head_shadow == ele->tail)
    {
        avail_free += ele->length;
    }
    else if (apool.head_shadow > ele->tail)
    {
        avail_free += ele->length - apool.head_shadow;
        avail_free += ele->tail - SLOP_SECTORS;
    }
    else
    {
        avail_free += ele->tail - apool.head_shadow;
        // They can come very close to each other in this case, but not equal.
        // In fact, since we have 1 sector of metadata, this means -2 for free.
        avail_free -= 2;
    }

    if (avail_space)
    {
        percent_free = (100ULL * avail_free) / avail_space;
    }
    else
    {
        percent_free = 0;
    }

    /* Might be possible with slop to go over 100 percent. */
    /* Really? Let's find out... */
    if (percent_free > 100)
    {
        fprintf(stderr, "%s: percent_free > 100, %u\n",
                __func__, percent_free);
        percent_free = 100;
    }

    return 100 - percent_free;
}

/**
******************************************************************************
**
**  @brief      If percent full changes threshold level, send async event.
**
**  This function may send an async event to icon, if percent full threshold
**  level changes.  The levels are:
**      lastevent 100 - 100 percent full
**      lastevent  50 - Greater than or equal to 50 percent full,
**      lastevent  30 - Less than 30 percent full.
**
**  @param      apool_id    - The apool we are working on.
**
**  @return     percent full
**
**  @attention  registers g3 and g4 are changed when CM_SuspendCopy_auto and
**              CM_pksnd_local_poll are called.
**
******************************************************************************
*/
#define FULL            100
#define THREE_FOURTHS   75
#define HALF            50
#define THIRD           33
#define EMPTY           0

void apool_event_percent_full(UINT16 apool_id, UINT8 percent_full)
{
    static UINT8 last_percent_full = 0;
    UINT32       is_filling;

/* Messages are:
 *      Async Buffer is empty       (only if not already empty.)
 *      Async Buffer is <33% full   (only if > 50 or FULL have happened).
 *      Async Buffer is 50-75% full (only if >=99, or < 30% currently).
 *      Async Buffer is 100% full   (only if not currently full.)
 * last_event values: 100, 75, 50, 33, 0
 */
    if(last_event == 0xff)
    {
        last_event = EMPTY;
        last_percent_full = 0;
        apool_auto_start_alinks(apool_id);
        return;
    }
    if(percent_full > last_percent_full)
    {
        is_filling = TRUE;
    }else if(percent_full < last_percent_full)
    {
        is_filling = FALSE;
    }else
    {
        /* No change so return. */
        return;
    }
    last_percent_full = percent_full;

    if (percent_full >= 99)                 /* Currently full. */
    {
        if (is_filling && last_event != FULL)    /* If not previously full, log event. */
        {
            /* The apool has just filled up so pause the mirrors. */
            logAPOOLevent(AP_ASYNC_BUFFER_FULL, LOG_AS_ERROR, 1, apool_id, apool.element[0].vid);
            last_event = FULL;
            apool_auto_pause_alinks(apool_id);
        }
    }
    else if (percent_full >= 50)            /* Currently >= 50%. */
    {
        if (!is_filling && last_event == FULL)   /* If previously full and now draining */
        {
            if ((percent_full <= 75) && !(apool.status & (1 << MR_APOOL_FULL)))
            {
                /* The apool was full and now has drained below 75% so unpause mirrors. */
                logAPOOLevent(AP_ASYNC_BUFFER_FULL_WARN, LOG_AS_WARNING, 1, apool_id, apool.element[0].vid);
                last_event = THREE_FOURTHS;
                apool_auto_start_alinks(apool_id);
            }
        }
        else if (is_filling && last_event < HALF)          /* If is rising from somewhere below half full */
        {
            /* The apool is filling up and has just crossed the 50% mark so log a message. */
            logAPOOLevent(AP_ASYNC_BUFFER_FULL_WARN, LOG_AS_WARNING, 1, apool_id, apool.element[0].vid);
            last_event = HALF;
        }
    }
    else if (percent_full == 0)             /* Currently empty. */
    {
        if (last_event != EMPTY)           /* Used to be full-ish. */
        {
            logAPOOLevent(AP_ASYNC_BUFFER_EMPTY, LOG_AS_DEBUG, 1, apool_id, apool.element[0].vid);
            last_event = EMPTY;
            if(!is_filling && ap_reject_puts)
            {
                apool_auto_start_alinks(apool_id);
            }
        }
    }
    else if (percent_full < 33)             /* Currently less than 33. */
    {
        if (last_event != THIRD && last_event != EMPTY)
        {
            logAPOOLevent(AP_ASYNC_BUFFER_FULL_OK, LOG_AS_INFO, 1, apool_id, apool.element[0].vid);
            last_event = THIRD;
            if(!is_filling && ap_reject_puts)
            {
                apool_auto_start_alinks(apool_id);
            }
        }
    }
    else                                    /* Currently 33 to 50 */
    {
        if (last_event >= 99)               /* Used to be full. */
        {
            /* This state should never happen. It means that it went from full to less than half without
               hitting any states inbetween.  Restart the mirrors. */
            logAPOOLevent(AP_ASYNC_BUFFER_FULL_WARN, LOG_AS_WARNING, 1, apool_id, apool.element[0].vid);
            last_event = HALF;              /* Move to 50. */
            apool_auto_start_alinks(apool_id);
        }
    }
}

/**
 ******************************************************************************
 **
 **  @brief      Mover Statistics Task.
 **
 **  This task collects statistics for the apool mover task.
 **
 **  @param      none
 **
 **  @return     none
 **
 ******************************************************************************
 */
void mover_stats_task(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED)
{
    UINT32 alpha = 30;
    UINT32 beta = 100 - alpha;
    static UINT32 last_output = 0;
    UINT32 output = 0;

    while(apool.mover_task_pcb)
    {
        // Exponential filter output
        output = (last_output * beta) + (alpha * apool.blocks_last_second);
        output = output / 100;

        last_output = output;
        apool.blocks_per_sec = output;
        apool.blocks_last_second = 0;
        if(gVDX.vdd[apool.element[0].vid])
        {
            gVDX.vdd[apool.element[0].vid]->breakTime = apool.blocks_per_sec;
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        }
        // Update the copy throttle mechanism threshold for the new bandwidth
        apool_set_max_ap_data(apool_calc_max_ap_data());
        TaskSleepMS(1000);
    }
    apool.stats_task_pcb = NULL;
    apool.blocks_last_second = 0;
}

/**
******************************************************************************
**
**  @brief      Mover Task.
**
**  This goes through the Apool and flushes data out to the appropriate V/Alink.
**
**  @param      none
**
**  @return     none
**
**  @attention  register g3 is changed when CCSM_Copy_ended is called via
**              apool_break_async_mirrors, through apool_get().
**              This is ok on: 2007-10-12 -- Marshall Midden
**
******************************************************************************
*/
void mover_task(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED)
{
    UINT32          flush_status;
    UINT8           links_up_down = 0;      /*< Assume links up event was last. */

    // Unpause the mover task
    ap_mover_paused = 0;
#ifdef M4_EXTRA_PRINTING
PRINT_APOOL_STRUCTURE;
#endif /* M4_EXTRA_PRINTING */
fprintf(stderr, "mover_task: started\n");

    while (1)
    {
        while (!ap_mover_paused)
        {
            // Sleep for a ms
            TaskSleepMS(1);
#ifdef DEBUG_MOVER_TASK
fprintf(stderr, "mover_task: awakened\n");
#endif  /* DEBUG_MOVER_TASK */

            // Exit loop so task can be killed, if requested to do so.
            if (BIT_TEST(mover_task_status, MR_APOOL_KILL_MOVER) ||
                    apool.mover_task_pcb == NULL)
            {
#ifdef DEBUG_MOVER_TASK
fprintf(stderr, "mover_task: requested to be killed\n");
fprintf(stderr,"mover_task: Apool Status=0X%x, Apool Mover PCB =0X%x\n", (UINT32)apool.status, (UINT32)(apool.mover_task_pcb));
#endif  /* DEBUG_MOVER_TASK */
                break;
            }

                    while (apool_work_2_do(0))
                    {
#ifdef DEBUG_MOVER_TASK
fprintf(stderr, "mover_task: calling apool_get\n");
#endif  /* DEBUG_MOVER_TASK */
                        if(ap_mover_paused)
                        {
                            //Check for mover task paused
                            break;
                        }
                        if(BIT_TEST(gVDX.vdd[apool.element[apool.cur_tail_shadow_element].vid]->attr, VD_BBEBUSY))
                        {
                            // APool is in BUSY state - try later
                            TaskSleepMS(5000);
                            break;
                        }

                        // If the queue depth to the remote side is not too high then flush some more.
                        if (mover_que_depth <= base_max_que_depth)   // was MOVER_MAX_QUE_DEPTH)
                        {
                            flush_status = apool_get(0);
#ifdef DEBUG_MOVER_TASK
fprintf(stderr, "mover_task: return from apool_get(%u)\n", flush_status);
#endif  /* DEBUG_MOVER_TASK */

                            if (flush_status != AOK)
                            {
#ifdef DEBUG_MOVER_TASK
fprintf(stderr, "mover_task: bad status from apool_get (%u)\n", flush_status);
#endif  /* DEBUG_MOVER_TASK */

                                if (flush_status == EC_UNINIT_DEV)
                                {
                                    logAPOOLevent(AP_ASYNC_BUFFER_BAD, LOG_AS_ERROR,
                                            1, apool.id, apool.element[0].vid);
                                    BIT_SET(mover_task_status, MR_APOOL_KILL_MOVER);
                                    break;
                                }
                                else if (flush_status == EC_LINK_FAIL)
                                {
                                    // The request timed out or the connection was not available
                                    if (links_up_down == 0)
                                    {
                                        logAPOOLevent(AP_ASYNC_LINKS_DOWN, LOG_AS_WARNING,
                                                1, apool.id, apool.element[0].vid);
                                        links_up_down = 1;
                                    }
                                    // sleep for 30 seconds and try again.
                                    TaskSleepMS(30000);
                                    continue;
                                }
                                else if ((flush_status == EC_RETRY)
                                        || (flush_status == EC_RETRYRC))
                                {
                                    mover_task_waiting = 1;
                                    TaskSetMyState(PCB_NOT_READY);
                                    TaskSwitch();
                                    mover_task_waiting = 0;
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            mover_task_waiting = 1;
                            TaskSetMyState(PCB_NOT_READY);
                            TaskSwitch();
                            mover_task_waiting = 0;
                            continue;
                        }

                        // Kill this task if requested to do so
                        if (BIT_TEST(mover_task_status, MR_APOOL_KILL_MOVER) ||
                                apool.mover_task_pcb == NULL)
                        {
                            break;
                        }
                        if (links_up_down == 1)
                        {
                            logAPOOLevent(AP_ASYNC_LINKS_OK, LOG_AS_WARNING,
                                    1, apool.id, apool.element[0].vid);
                            links_up_down = 0;
                        }
                    }
        }

        // Kill this task if it is paused.
        if (BIT_TEST(mover_task_status, MR_APOOL_KILL_MOVER) ||
                apool.mover_task_pcb == NULL)
        {
            BIT_CLEAR(mover_task_status, MR_APOOL_KILL_MOVER);
            logAPOOLevent(AP_MOVER_STOPPED, LOG_AS_INFO, 1, apool.id, apool.element[0].vid);
fprintf(stderr, "mover_task: Exiting\n");
            break;
        }
        TaskSleepMS(1000);
    }

    //BIT_CLEAR(apool.status, MR_APOOL_KILL_MOVER);
    BIT_CLEAR(mover_task_status, MR_APOOL_KILL_MOVER);
    apool.mover_task_pcb = NULL;
}

/**
******************************************************************************
**
**  @brief      Validate the apool pointers.
**
**  This function will read from the apool to validate that the current head
**  pointer is
**
**  @param      apool_id    - The apool we are working on.
**
**  @return     none
**
**  @attention  register g3 is changed when CCSM_Copy_ended is called via
**              apool_break_async_mirrors.
**              This is ok on: 2007-10-12 -- Marshall Midden
**
******************************************************************************
*/
void apool_validate(UINT16 apool_id)
{
    ILT            *ilt;
    VRP            *vrp;
    SGL            *sgl;
    SGL_DESC       *desc;
    APOOL_META_DATA *meta_data;
    int             up_to_date = FALSE;
    int             num_reads = 0;
    int             retry_count;

PRINT_APOOL_STRUCTURE;

    if(apool.put_record_head)
    {
        fprintf(stderr,"Apool_validate found put record(s)\n");
    }

    // Allocate an ILT and VRP
    ilt = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    vrp = get_vrp();
#ifdef M4_DEBUG_VRP
CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, vrp);
#endif /* M4_DEBUG_VRP */

    // Allocate an SGL for the meta data
    sgl = s_MallocW(sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
    desc = (SGL_DESC *)(sgl + 1);
    meta_data = (APOOL_META_DATA *)(desc + 1);

    ilt->ilt_normal.w0 = (UINT32)vrp;
    memset(vrp, 0, sizeof(VRP));

    // Fill in the SGL information
    sgl->scnt = 1;
    sgl->owners = 1;
    sgl->flag = 0;
    sgl->size = sizeof(SGL) + sizeof(SGL_DESC);
    desc->addr = (void *)meta_data;
    desc->len = BYTES_PER_SECTOR;

    // Update VRP information
    vrp->pSGL = sgl;
    vrp->sglSize = sgl->size;


    while (!up_to_date)
    {

        // Make sure we are still the owner.
        if (!gAsyncApoolOwner)
        {
           break;
        }

        retry_count = 3;
        while(retry_count)
        {
            vrp->length = 1;
            vrp->function = VRP_INPUT;
            vrp->startDiskAddr = apool_get_head(&(vrp->vid), apool_id);

    fprintf(stderr, "apool_validate: fetching metadata.\n");
            //Queue the request to virtual layer with wait
            EnqueueILTW((void *)V_que, ilt);

            // Break the mirrors if you can't read from the apool.
            if (vrp->status != AOK)
            {
                if(--retry_count)
                {
                    // Do a retry
                    continue;
                }
    fprintf(stderr, "apool_validate: got bad vrp status %u for ILT %p, Breaking all mirrors.\n", vrp->status, ilt);
                logAPOOLevent(AP_BREAK_ALL_ASYNC_MIRRORS, LOG_AS_ERROR, 1, apool_id, 0);
                apool_break_async_mirrors(0xffff);
                apool.status |= (1 << MR_APOOL_BAD_APOOL);
                goto error_out;
            }
            else
            {
                apool.status &= ~(1 << MR_APOOL_BAD_APOOL);
                break;
            }
        }

        retry_count = 3;
        while(retry_count)
        {
            // Re-write the meta data.
            vrp->length = 1;
            vrp->function = VRP_OUTPUT;
            vrp->startDiskAddr = apool_get_head(&(vrp->vid), apool_id);
            //Queue the request to virtual layer with wait
            EnqueueILTW((void *)V_que, ilt);
            if(vrp->status != AOK)
            {
                if(--retry_count)
                {
                    continue;
                }
            }
            break;
        }

        num_reads++;

        if (meta_data->sequence_count == apool.sequence_count + 1)
        {
            // Found valid data in this slot this means that the
            // data in NVRAM is stale.  Advance the head pointer
            // until the end is found.

            // Update the apool sequence count
            apool.sequence_count += 1;
            shadow_sequence_count = apool.sequence_count;
            // Advance the shadow head to handle the jumping case
            vrp->length = meta_data->length + 1;
            if(apool_update_shadow_head(vrp,apool_id) != AOK)
            {
                // Do not allow the shadow head to advance beyond the tail
fprintf(stderr, "apool_validate: Head overwrote tail, Breaking all mirrors.\n");
                logAPOOLevent(AP_BREAK_ALL_ASYNC_MIRRORS, LOG_AS_ERROR, 1, apool_id, 0);
                apool_break_async_mirrors(0xffff);
                apool.status |= (1 << MR_APOOL_BAD_APOOL);
                break;
            }
            // Advance the head and update NVRAM
            apool_update_head(meta_data->length + 1, apool_id);

        }
        else
        {
            loc_last_seq_count = apool.last_seq_count;
            shadow_sequence_count = apool.sequence_count;
            up_to_date = TRUE;
        }
    }

error_out:
    // Update NVRAM with the possible new values.
    AR_NVUpdate(AR_UPDATE_ALL);

    // Free the small sgl and allocate a new one to hold the data
    s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
    vrp->pSGL = NULL;
    PM_RelILTVRPSGL(ilt, vrp);
fprintf(stderr, "apool_validate: Validated with %d reads.\n", num_reads);
    PRINT_APOOL_STRUCTURE;
}

void dumpem(void)
{
#if 0
    /*
    ** Retaining this code so that it can be used for debugging when required.
    */
    APOOL_GET_RECORD *cur_get_record;
    int cnt;

    cur_get_record = apool.get_record_head;
    cnt=0;
    while (cur_get_record != NULL)
    {
      fprintf(stderr,"[%d:%llx-%x,%x]",cnt,cur_get_record->sda,(int)cur_get_record->state,(int)cur_get_record->status);
      cur_get_record=cur_get_record->p_next;
      cnt++;
    }
    fprintf(stderr,"\n");
#endif /* 0 */
}

/**
******************************************************************************
**
**  @brief  Start mover task if needed
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void StartMover(void)
{
    if (mover_task_waiting && apool.mover_task_pcb)
    {
        if (TaskGetState(apool.mover_task_pcb) == PCB_NOT_READY)
        {
#ifdef HISTORY_KEEP
CT_history_pcb("StartMover setting ready pcb", (UINT32)apool.mover_task_pcb);
#endif  /* HISTORY_KEEP */
            TaskSetState(apool.mover_task_pcb, PCB_READY);
        }
        mover_task_waiting = 0;
    }
}

/**
******************************************************************************
**
**  @brief      Get data from the apool.
**
**  This function pulls data from the next slot in the apool and flushes it
**  to the appropriate Vlink.
**
**  @param      apool_id    - The apool we are working on.
**
**  @return     status of get attempt
**
**  @attention  register g3 is changed when CCSM_Copy_ended is called via
**              apool_break_async_mirrors.
**              This is ok on: 2007-10-12 -- Marshall Midden
**
******************************************************************************
*/
UINT32 apool_get(UINT16 apool_id)
{
    static UINT32   message_about_tossing = 0;
    UINT32          tossing_reason = 0;
    ILT            *ilt;
    VRP            *vrp;
    SGL            *sgl;
    SGL_DESC       *desc;
    APOOL_META_DATA *meta_data;
    APOOL_META_DATA *persistent_meta_data;
    APOOL_GET_RECORD *get_record;
    UINT16          cur_vid;
    UINT64          cur_sda;
    VDD            *vdd = NULL;
    RDD            *rdd = NULL;
    LDD            *ldd = NULL;
    PSD            *psd = NULL;
    int             toss_it = FALSE;
    int             slp_inc = 0;
    ALINK          *alink;
    int             i = 3;
    UINT64          cur_time = 0;

    if(apool.status & (1 << MR_APOOL_FLUSH_ERROR))
    {
        // If there was a flush error then signal the mover task to
        // wait a bit before trying another get.
        apool.status &= ~(1 << MR_APOOL_FLUSH_ERROR);
        return(EC_LINK_FAIL);
    }

    // Check for stalled get records.  Monitor the count of outstanding
    // records and if reached pause and restart from oldest record.
    if(get_record_count > base_max_que_depth + 200)
    {
        dumpem();
        fprintf(stderr,"apool_get: Found too many get records, count = %d\n", get_record_count);
        // Wait ten minutes or so.
        for(slp_inc=0;slp_inc<4000;slp_inc++)
        {
          TaskSleepMS(125);
          if(get_record_count < base_max_que_depth + 150)
          {
             fprintf(stderr,"apool_get: recovered at slp_inc=%d\n", slp_inc);
             break;
          }
        }
    }

    // Allocate an ILT and VRP
    ilt = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    vrp = get_vrp();
#ifdef M4_DEBUG_VRP
CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, vrp);
#endif /* M4_DEBUG_VRP */

    ilt->ilt_normal.w0 = (UINT32)vrp;
    memset(vrp, 0, sizeof(VRP));

    // Allocate an SGL for the meta data
    sgl = s_MallocW(sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
#ifdef M4_EXTRA_DEBUGGING
memset(sgl, 0xee, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR);
#endif  /* M4_EXTRA_DEBUGGING */
    desc = (SGL_DESC *)(sgl + 1);
    meta_data = (APOOL_META_DATA *)(desc + 1);

    // Fill in the SGL information
    sgl->scnt = 1;
    sgl->owners = 0;
    sgl->flag = 0;
    sgl->size = sizeof(SGL) + sizeof(SGL_DESC);
    desc->addr = (void *)meta_data;
    desc->len = BYTES_PER_SECTOR;

    // Update VRP information
    vrp->pSGL = sgl;
    vrp->sglSize = sgl->size;
    vrp->length = 1;
    vrp->function = VRP_INPUT;
    vrp->startDiskAddr = apool_get_shadow_tail(&(vrp->vid), apool_id);
    cur_vid = vrp->vid;
    cur_sda = vrp->startDiskAddr;

#ifdef ILT_PRINTING
fprintf(stderr, "apool_get: queueing meta data read ILT %p\n", ilt);
#endif  /* ILT_PRINTING */
    while(i)
    {

        //Queue the request to virtual layer with wait
        EnqueueILTW((void *)V_que, ilt);
        if(vrp->status != AOK)
        {
            fprintf(stderr, "apool_get: retry meta data read ILT %p\n", ilt);
            vrp->length = 1;
            vrp->function = VRP_INPUT;
            vrp->vid = cur_vid;
            vrp->startDiskAddr= cur_sda;
            i--;
            continue;
        }
        else
        {
            break;
        }
    }

    if (vrp->status != AOK)
    {
        UINT8 save_vrp_status = vrp->status;
        if ((vrp->status != EC_BEBUSY)
                  && (vrp->status != EC_RETRY)
                  && (vrp->status != EC_RETRYRC))
        {
            fprintf(stderr, "apool_get: got bad metadata read status, vrp->status=%d, Breaking all mirrors.\n", vrp->status);
            // Break all mirrors, clean up, and return.
            logAPOOLevent(AP_BREAK_ALL_ASYNC_MIRRORS, LOG_AS_ERROR, 2, apool_id, 0);
            apool_break_async_mirrors(0xffff);
            apool.status |= (1 << MR_APOOL_BAD_APOOL);

            // Kill the apool and this thread
            apool_delete(apool.element[0].vid);
            mover_task_status |= (1 << MR_APOOL_KILL_MOVER);
        }
        s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
        vrp->pSGL = NULL;
        PM_RelILTVRPSGL(ilt, vrp);
        return (save_vrp_status);
    }
    else
    {
        apool.status &= ~(1 << MR_APOOL_BAD_APOOL);
    }

    UINT32  length = meta_data->length;

    if (mover_que_data && length + mover_que_data > mover_que_limit)
    {
        s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
        vrp->pSGL = NULL;
        PM_RelILTVRPSGL(ilt, vrp);
        return EC_RETRY;
    }


    // Assign a get_record
    get_record = apool_assign_get_record(ilt, apool_id);
    ilt->ilt_normal.w3 = (UINT32)get_record;

    // Restore the VID and SDA since the RAID layer may have changed them.
    vrp->vid = cur_vid;
    vrp->startDiskAddr = cur_sda;

    // Check the flush error bit for the apool.  If it is set then release resources and return
    if(apool.status & (1 << MR_APOOL_FLUSH_ERROR))
    {
        get_record->status = MR_APOOL_FLUSH_ERROR;
        get_record->state |= (1 << REQUEST_COMPLETE);
        apool_release_get_record(get_record, apool_id);
        s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
        vrp->pSGL = NULL;
        PM_RelILTVRPSGL(ilt, vrp);
        return (MR_APOOL_FLUSH_ERROR);
    }

    // Put the meta data into a new buffer and save in ILT
    persistent_meta_data = s_MallocW(sizeof(APOOL_META_DATA), __FILE__, __LINE__);
    *persistent_meta_data = *meta_data;
    ilt->ilt_normal.w1 = (UINT32)persistent_meta_data;
//    fprintf(stderr,"apool_get per_meta_data=%p, p_attr=0x%x, attr=0x%x\n",persistent_meta_data,persistent_meta_data->attributes,meta_data->attributes);

    // Check for the proper sequence count value in the metadata.
    // If it is not correct then the apool must be corrupt so break
    // all mirrors.
    if (loc_last_seq_count == ~0ULL)
    {
        loc_last_seq_count = persistent_meta_data->sequence_count;
    }
    else if (persistent_meta_data->sequence_count != loc_last_seq_count +1)
    {
        // Apool is corrupt or pointers got screwed up.
        fprintf(stderr, "apool_get: got bad sequence count, read: %llu, expected: %llu, Breaking all mirrors.\n",
             persistent_meta_data->sequence_count, loc_last_seq_count+1);
        logAPOOLevent(AP_BREAK_ALL_ASYNC_MIRRORS, LOG_AS_ERROR, 3, apool_id, 0);
        apool_break_async_mirrors(0xffff);
        apool.status |= (1 << MR_APOOL_BAD_APOOL);

        // Clean up local allocations.
        get_record->status = APOOL_WRITE_ERROR;
        get_record->state |= (1 << REQUEST_COMPLETE);
        apool_release_get_record(get_record, apool_id);
        s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
        s_Free(persistent_meta_data, sizeof(APOOL_META_DATA), __FILE__, __LINE__);
        vrp->pSGL = NULL;
        PM_RelILTVRPSGL(ilt, vrp);

        // Kill the apool and this thread
        apool_delete(apool.element[0].vid);
        mover_task_status |= (1 << MR_APOOL_KILL_MOVER);

        return (EC_UNINIT_DEV);
    }

    get_record->length = persistent_meta_data->length;
    get_record->sda = persistent_meta_data->sda;
    get_record->sequence_count = persistent_meta_data->sequence_count;
    get_record->vid = persistent_meta_data->vid;
    get_record->state |= (1 << READ_REAL_DATA);
    get_record->status = vrp->status;

    // Check for overlap of outstanding writes.
    while (apool_check_for_overlap(get_record, apool_id) == TRUE)
    {
        if(!first_overlap_time)
        {
            //This is a new overlap condition
            first_overlap_time = get_tsc();
            apool_overlap_count++;
        }
        else
        {
            cur_time = get_tsc();
            //Check for a timeout condition.
            if(((cur_time - first_overlap_time)/SYS_TSC_ONE_SEC_DELAY) < 25ULL)
            {
                get_record->status = EC_OVERLAP;
                get_record->state |= (1 << REQUEST_COMPLETE);
                apool_release_get_record(get_record, apool_id);
                s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
                s_Free(persistent_meta_data, sizeof(APOOL_META_DATA), __FILE__, __LINE__);
                vrp->pSGL = NULL;
                PM_RelILTVRPSGL(ilt, vrp);
                return (EC_RETRY);
            }
            else
            {
                // At this point 25 s has elapsed, we need to return with error
                get_record->status = OVERLAP_TIMEOUT;
                fprintf(stderr, "apool_get: Overlap write timeout 25 secnds - ILT %p\n", get_record->ilt);
                get_record->state |= (1 << REQUEST_COMPLETE);
                apool_release_get_record(get_record, apool_id);
                s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
                s_Free(persistent_meta_data, sizeof(APOOL_META_DATA), __FILE__, __LINE__);
                vrp->pSGL = NULL;
                PM_RelILTVRPSGL(ilt, vrp);
                first_overlap_time = 0;
                return (EC_TIMEOUT);
            }
        }
    }
    first_overlap_time = 0;

    // Update the last sequence count
    loc_last_seq_count = persistent_meta_data->sequence_count;

    // Do all of the necessary checks. Validate sequence count, make sure mirror is active,
    vdd = gVDX.vdd[get_record->vid];

    // Make sure the vdisk still exists
    if (!vdd)
    {
        toss_it = TRUE;
        tossing_reason = NO_VDD;
    }
    else
    {
        // Check to make sure the destination is an alink
        if (!(vdd->attr & (1 << VD_BASYNCH)))
        {
            toss_it = TRUE;
            tossing_reason = NOT_ALINK;
        }

        // Check to make sure the mirror exists
        if ( ((DCD*)(gVDX.vdd[get_record->vid]->pDCD) == NULL) ||
            (((DCD*)(gVDX.vdd[get_record->vid]->pDCD))->cor == NULL) )
        {
            toss_it = TRUE;
            tossing_reason = NO_MIRROR;
        }
    }

    // Make sure there is a valid alink structure.
    if ((alink = apool_get_alink(apool_id, get_record->vid)) == NULL)
    {
        if (TRUE == apool_check_n_init_alink(gVDX.vdd[get_record->vid]))
        {
            fprintf(stderr, "ASYNC:%s New alink added VID=%d\n", __func__, get_record->vid);
        }
        else
        {
            toss_it = TRUE;
            tossing_reason = NO_ALINK;
        }
    }
    else
    {
        // Check the sequence count to make sure it is within reason.
        if (!apool_validate_alink_seq_count(get_record->sequence_count, alink))
        {
            toss_it = TRUE;
            tossing_reason = BAD_SEQ_COUNT;
        }
    }

    if (toss_it)
    {
        // Decrement copy counter if needed.
        if(BIT_TEST(persistent_meta_data->attributes,COPY_DATA) && cur_ap_data)
        {
            // 8 copy related VRPs make up one segment (MByte) of copy data
            if((++copy_req_count%8) == 0)
            {
                cur_ap_data -= 1;
                copy_req_count = 0;
            }
        }

        // Clean up local allocations.
        if (message_about_tossing++ == 0)
        {
            fprintf(stderr, "Tossing data from apool. Reason: %d\n",tossing_reason);
        }
        get_record->status = AOK;
        get_record->state |= (1 << REQUEST_COMPLETE);
        apool_update_shadow_tail(get_record, apool_id);
        apool_release_get_record(get_record, apool_id);
        s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
        s_Free(persistent_meta_data, sizeof(APOOL_META_DATA), __FILE__, __LINE__);
        vrp->pSGL = NULL;
        PM_RelILTVRPSGL(ilt, vrp);
        return(AOK);
    }
    if (message_about_tossing > 1)
    {
        fprintf(stderr, "Tossed %u data from apool.\n", message_about_tossing);
    }
    message_about_tossing = 0;

    // Check for a good remote connection
    vdd = gVDX.vdd[meta_data->vid];
    if (vdd)
    {
        rdd = vdd->pRDD;
    }
    if (rdd)
    {
        psd = rdd->extension.pPSD[0];
    }
    if (psd)
    {
        ldd = DLM_lddindx[psd->pid];
    }
    if (ldd)
    {
        if (ldd->state != LDD_ST_OP)
        {
            // Clean up local allocations.
            get_record->status = APOOL_WRITE_ERROR;
            get_record->state |= (1 << REQUEST_COMPLETE);
            apool_release_get_record(get_record, apool_id);
            s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
            s_Free(persistent_meta_data, sizeof(APOOL_META_DATA), __FILE__, __LINE__);
            vrp->pSGL = NULL;
            PM_RelILTVRPSGL(ilt, vrp);

            // Remote connection is down
            apool.status |= (1 << MR_APOOL_NO_REMOTE_CONNECTION);
            return(EC_LINK_FAIL);
        }
        else
        {
            apool.status &= ~(1 << MR_APOOL_NO_REMOTE_CONNECTION);
        }
    }


    // Check if the flush error bit is set.  If so then release structs and return error.
    if(apool.status & (1 << MR_APOOL_FLUSH_ERROR))
    {
        get_record->status = MR_APOOL_FLUSH_ERROR;
        get_record->state |= (1 << REQUEST_COMPLETE);
        apool_release_get_record(get_record, apool_id);
        s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
        s_Free(persistent_meta_data, sizeof(APOOL_META_DATA), __FILE__, __LINE__);
        vrp->pSGL = NULL;
        PM_RelILTVRPSGL(ilt, vrp);
        return (MR_APOOL_FLUSH_ERROR);
    }

    // Update the last flushed time stamp for this alink. Externalize this via the VDD
    if (alink && BIT_TEST(persistent_meta_data->attributes,TIME_STAMP_VALID))
    {
        alink->last_time_stamp = persistent_meta_data->time_stamp;
        if (vdd)
        {
            vdd->lastAccess = (UINT32)(alink->last_time_stamp);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        }
    }

    // Update the shadow tail pointer
    apool_update_shadow_tail(get_record, apool_id);
#ifdef M4_EXTRA_PRINTING
PRINT_APOOL_STRUCTURE;
#endif /* M4_EXTRA_PRINTING */

    // Free the small sgl and allocate a new one to hold the data
    s_Free(sgl, sizeof(SGL) + sizeof(SGL_DESC) + BYTES_PER_SECTOR, __FILE__, __LINE__);
    sgl = s_MallocW(sizeof(SGL) + sizeof(SGL_DESC) + (persistent_meta_data->length * BYTES_PER_SECTOR), __FILE__, __LINE__);

    desc = (SGL_DESC *)(sgl + 1);

    // Fill in the SGL information
    sgl->scnt = 1;
    sgl->owners = 0;
    sgl->flag = 0;
    sgl->size = sizeof(SGL) + sizeof(SGL_DESC);
    desc->addr = (void *)(desc + 1);
    desc->len = persistent_meta_data->length * BYTES_PER_SECTOR;

    // Modify the VRP to do the read
    vrp->startDiskAddr++;
    vrp->length = persistent_meta_data->length;
    vrp->pSGL = sgl;
    vrp->function = VRP_INPUT;


#ifdef ILT_PRINTING
fprintf(stderr, "apool_get: queueing real data read ILT %p.\n", ilt);
#endif  /* ILT_PRINTING */

    // Queue to virtual layer.
    EnqueueILT((void *)V_que, ilt,
               (void *)C_label_referenced_in_i960asm(apool_data_read_cr));

    // Increment the remote outstanding request count.
    mover_que_depth++;
    mover_que_data += length;

    return (vrp->status);
}

/**
******************************************************************************
**
**  @brief      Apool data read completion routine
**
**  This handles the next steps after reading of data is complete.
**
**  @param      dummy   - unused
**  @param      ilt     - pointer to the top level of the ILT
**
**  @return     none
**
******************************************************************************
*/
void apool_data_read_cr(UINT32 dummy UNUSED, ILT *ilt)
{
    VRP            *vrp;
    APOOL_GET_RECORD *get_record;
    APOOL_META_DATA *meta_data;
    UINT32  save_g0 = g0;
    UINT32  save_g1 = g1;
    UINT32  save_g2 = g2;


    // Check the status of the data read
    vrp = (VRP *)ilt->ilt_normal.w0;

    get_record = (APOOL_GET_RECORD *)ilt->ilt_normal.w3;

    // Retrieve the meta data
    meta_data = (APOOL_META_DATA *)ilt->ilt_normal.w1;

    if (vrp->status != AOK)
    {
        UINT32  length = meta_data->length;

        // The status of the read is bad.  When the get record is released
        // the shadow tail will be reset so the request will eventually be
        // retried.
fprintf(stderr, "apool_data_read_cr: ilt %p vrp %p has bad status %u\n", ilt, vrp, vrp->status);
        get_record->status = vrp->status;
        get_record->state |= (1 << REQUEST_COMPLETE);
        apool_release_get_record(get_record, 0);
        if ((UINT32)vrp->pSGL == 0xfeedf00d)
        {
            fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
            abort();
        }
        s_Free(vrp->pSGL, sizeof(SGL) + sizeof(SGL_DESC) + (length * BYTES_PER_SECTOR), __FILE__, __LINE__);
        vrp->pSGL = NULL;
        s_Free(meta_data, sizeof(APOOL_META_DATA), __FILE__, __LINE__);
        PM_RelILTVRPSGL(ilt, vrp);
        mover_que_depth--;
        mover_que_data -= length;
        StartMover();
        goto restore_g_registers;
    }

    // Change the state of the get record
    get_record->state |= (1 << WRITE_REAL_DATA);
    get_record->status = vrp->status;


    // Modify the VRP to point to the V/Alink
    vrp = (VRP *)ilt->ilt_normal.w0;
    vrp->vid = meta_data->vid;
    vrp->function = VRP_OUTPUT;
    vrp->startDiskAddr = meta_data->sda;

    // Set the bit to bypass the apool logic
    vrp->function |= (1 << APOOL_BYPASS);

    // Set the special processing bit
    vrp->function |= (1 << VRP_SPECIAL);

#ifdef ILT_PRINTING
fprintf(stderr, "apool_data_read_cr: queueing original data Write ILT %p\n", ilt);
#endif  /* ILT_PRINTING */

    // Submit to Virtual layer
    EnqueueILT((void *)V_que, ilt,
               (void *)C_label_referenced_in_i960asm(apool_valink_data_write_cr));
restore_g_registers:
    g0 = save_g0;
    g1 = save_g1;
    g2 = save_g2;

}

/**
******************************************************************************
**
**  @brief      V/Alink data write completion routine.
**
**  This handles the next steps after writing of data is complete.
**
**  @param      dummy   - unused.
**  @param      ilt     - pointer to the top level of the ILT.
**
**  @return     none
**
******************************************************************************
*/
void apool_valink_data_write_cr(UINT32 dummy UNUSED, ILT *ilt)
{
    VRP            *vrp;
    APOOL_META_DATA *meta_data;
    APOOL_GET_RECORD *get_record;

    UINT32  save_g0 = g0;
    UINT32  save_g1 = g1;
    UINT32  save_g2 = g2;

    // Check the status of the data write
    vrp = (VRP *)ilt->ilt_normal.w0;

    get_record = (APOOL_GET_RECORD *)ilt->ilt_normal.w3;

    // Retrieve the meta data
    meta_data = (APOOL_META_DATA *)ilt->ilt_normal.w1;

    if (vrp->status != AOK)
    {
        // Since vlinks are kindof flakey we should
        // retry this request a few times before reseting the
        // shadow tail pointer.
fprintf(stderr, "apool_valink_data_write_cr: Alink VRP status %d, VRP %p ILT %p\n", vrp->status, vrp, ilt);
        if (--(get_record->retry_count))
        {
fprintf(stderr, "apool_valink_data_write_cr: retry ILT %p\n",ilt);
            vrp->status = AOK;
            vrp->function = VRP_OUTPUT;
            vrp->function |= (1 << APOOL_BYPASS);
            vrp->function |= (1 << VRP_SPECIAL);
            // Submit to Virtual layer
            EnqueueILT((void *)V_que, ilt,
                       (void *)C_label_referenced_in_i960asm(apool_valink_data_write_cr));
        }
        else
        {
            UINT32  length = meta_data->length;

            // Free resources and exit
            get_record->status = vrp->status;
            get_record->state |= (1 << REQUEST_COMPLETE);
            apool_release_get_record(get_record, 0);
            if ((UINT32)vrp->pSGL == 0xfeedf00d)
            {
                fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
                abort();
            }
            s_Free(vrp->pSGL, sizeof(SGL) + sizeof(SGL_DESC) + (length * BYTES_PER_SECTOR), __FILE__, __LINE__);
            vrp->pSGL = NULL;
            s_Free(meta_data, sizeof(APOOL_META_DATA), __FILE__, __LINE__);
            PM_RelILTVRPSGL(ilt, vrp);
            mover_que_depth--;
            mover_que_data -= length;
            StartMover();
        }
        goto restore_g_registers;
    }
    else
    {
        apool.status &= ~(1 << MR_APOOL_NO_REMOTE_CONNECTION);
        // Decrement the copy data counter if the request was copy related
        if(BIT_TEST(meta_data->attributes,COPY_DATA) && cur_ap_data)
        {
            // 8 copy related VRPs make up one segment (MByte) of copy data
            if((++copy_req_count%8) == 0)
            {
                cur_ap_data -= 1;
                copy_req_count = 0;
            }
        }
    }


    // Change the state of the get record
    get_record->state |= (1 << FLUSH_COMPLETE);
    get_record->status = vrp->status;

    // Free the ILT/VRP/SGL and meta data structure
#ifdef ILT_PRINTING
fprintf(stderr, "apool_valink_data_write_cr: Apool flushed IO VRP %p ILT %p to vlink\n", vrp, ilt);
#endif  /* ILT_PRINTING */

    get_record->state |= (1 << REQUEST_COMPLETE);
    apool_release_get_record(get_record, 0);
    meta_data = (APOOL_META_DATA *)ilt->ilt_normal.w1;
    if ((UINT32)vrp->pSGL == 0xfeedf00d)
    {
        fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
        abort();
    }

    UINT32  length = meta_data->length;

    s_Free(vrp->pSGL, sizeof(SGL) + sizeof(SGL_DESC) + (length * BYTES_PER_SECTOR), __FILE__, __LINE__);
    vrp->pSGL = NULL;
    s_Free(meta_data, sizeof(APOOL_META_DATA), __FILE__, __LINE__);
    PM_RelILTVRPSGL(ilt, vrp);
    mover_que_depth--;
    mover_que_data -= length;
    StartMover();

restore_g_registers:
    g0 = save_g0;
    g1 = save_g1;
    g2 = save_g2;
}

/**
******************************************************************************
**
**  @brief      Assign an apool get record.
**
**  This allocates and initializes an apool_get record.
**
**  @param      ilt         - ILT we are working with.
**  @param      apool_id    - Apool we are working with.
**
**  @return     APOOL_GET_RECORD.
**
******************************************************************************
*/
APOOL_GET_RECORD *apool_assign_get_record(ILT *ilt, UINT16 apool_id UNUSED)
{
    APOOL_GET_RECORD *get_record;
    VRP            *vrp;

    get_record = (APOOL_GET_RECORD *)s_MallocW(sizeof(APOOL_GET_RECORD), __FILE__, __LINE__);
    get_record->ilt = ilt;
    vrp = (VRP *)ilt->ilt_normal.w0;
    get_record->sequence_count = ~0ULL;
    get_record->state = (1 << READ_META_DATA);
    get_record->status = INCOMPLETE;
    get_record->vid = vrp->vid;
    get_record->p_next = NULL;
    get_record->retry_count = 2;
    // p_prev set below in both cases.

    //Insert the record at the tail
    if (apool.get_record_head == NULL)
    {
        apool.get_record_head = get_record;
        get_record->p_prev = NULL;          /* apool.get_record_tail is NULL */
    }
    else
    {
        apool.get_record_tail->p_next = get_record;
        get_record->p_prev = apool.get_record_tail;
    }
    apool.get_record_tail = get_record;
    get_record_count++;
    return (get_record);
}

/**
******************************************************************************
**
**  @brief      Release an apool get record.
**
**  This frees an apool_get record if it is the oldest record to complete.
**  If so, the apool pointers need to be updated.
**
**  @param      get_record  - The apool get record to free/release.
**  @param      apool_id    - The apool we are working on.
**
**  @return     none
**
******************************************************************************
*/
void apool_release_get_record(APOOL_GET_RECORD *get_record, UINT16 apool_id)
{
    APOOL_GET_RECORD *cur_record;
    static UINT32     event_counter = 0;
    ALINK            *pAlink;


    // First handle the case where this is an overlapping get record.
    // If this is the case then it is guaranteed that this will be the last
    // record on the list.  Unlink the get record and release it.
    if(get_record->status == EC_OVERLAP)
    {
        apool.get_record_tail = get_record->p_prev;
        get_record->p_prev->p_next = NULL;
        if(get_record_count)
        {
            get_record_count--;
        }
        s_Free(get_record, sizeof(*get_record), __FILE__, __LINE__);
        return;
    }

    // If this is the first record to fail then mark each outstanding
    // record as needing to be purged. This will signal the mover to toss out this IO.
    if ((get_record->status != AOK) && !(get_record->state & (1 << PURGE_ALL_REMAINING)))
    {
        cur_record = apool.get_record_head;
        while (cur_record != NULL)
        {
            cur_record->state |= (1 << PURGE_ALL_REMAINING);
            cur_record = cur_record->p_next;
        }
        apool_update_error(apool_id);

        // Reset the last read sequence count to the last read non-volatile value
        loc_last_seq_count = apool.last_seq_count;

        // Log error occurred.
        logAPOOLevent(AP_ASYNC_BUFFER_IO_BAD, LOG_AS_DEBUG, 0, apool.id, (UINT32)get_record->status);
    }
    if ((get_record->status == AOK)&& !(get_record->state & (1 << PURGE_ALL_REMAINING)))
    {
        apool.status &= ~(1 << MR_APOOL_FLUSH_ERROR);
    }

    if (apool.get_record_head == get_record)
    {
        // Free all of the records that have been completed.
        while ((get_record != NULL) && (get_record->state & (1 << REQUEST_COMPLETE)))
        {
            // Advance the get_record queue
            apool.get_record_head = get_record->p_next;

            if (get_record->p_next == NULL)
            {
                apool.get_record_tail = NULL;
            }
            else
            {
                get_record->p_next->p_prev = NULL;
            }


            //Only update the tail if the request was successful
            if ((get_record->status == AOK) &&
                !(get_record->state & (1 << PURGE_ALL_REMAINING)))
            {
                // Update the last read non-volatile sequence count
                apool.last_seq_count = get_record->sequence_count;
                apool_update_tail(get_record, apool_id);
                if(++event_counter%100)
                {
                    apool_event_percent_full(apool.id, apool_get_percent_full(apool.id));
                }

                // Update the number of blocks sent
                apool.blocks_last_second += (UINT32)get_record->length;

                // Update the sector count contained in the apool for this alink.
                pAlink = apool_get_alink(apool_id, get_record->vid);
                if (pAlink == NULL)
                {
                    if (TRUE == apool_check_n_init_alink(gVDX.vdd[get_record->vid]))
                    {
                        fprintf(stderr, "ASYNC:%s New alink added VID=%d\n", __func__, get_record->vid);
                    }
                }
                else
                {
                    // Store it in the alink struct
                    if(pAlink->sectors_outstanding >= get_record->length)
                    {
                        pAlink->sectors_outstanding -= get_record->length;
                    }
                    else
                    {
                        pAlink->sectors_outstanding = 0ULL;
                    }
                }
#ifdef M4_EXTRA_PRINTING
PRINT_APOOL_STRUCTURE;
#endif /* M4_EXTRA_PRINTING */
            }
            if(get_record_count)
            {
                get_record_count--;
            }
            s_Free(get_record, sizeof(*get_record), __FILE__, __LINE__);

            // Update the get_record pointer
            get_record = apool.get_record_head;
        }
    }
    else
    {
        // Do nothing until the oldest record is completed
    }
}

/**
******************************************************************************
**
**  @brief      Check for overlap of a get record.
**
**  This checks a get_record to see if any existing get records have overlaps
**  in the SDA for the original VID (VLink).
**
**  @param      get_record  - APOOL_GET_RECORD to check for overlap.
**  @param      apool_id    - The apool we are working on.
**
**  @return     TRUE or FALSE
**
******************************************************************************
*/
UINT8 apool_check_for_overlap(APOOL_GET_RECORD *get_record, UINT16 apool_id UNUSED)
{
    APOOL_GET_RECORD *cur_get_record;
    UINT64          cur_eda;
    UINT64          check_eda;

    check_eda = get_record->sda + get_record->length;

    cur_get_record = apool.get_record_head;
    while (cur_get_record != NULL)
    {
        if ((cur_get_record->vid == get_record->vid)&&
            (cur_get_record->ilt != get_record->ilt))
        {
            cur_eda = cur_get_record->sda + cur_get_record->length;
            if ((cur_get_record->sda <= get_record->sda) &&
                (get_record != cur_get_record))
            {
                if (cur_eda > get_record->sda)
                {
                    overlap_vid = get_record->vid;
                    return(TRUE);
                }
            }
            else
            {
                if ((cur_get_record->sda < check_eda) && (get_record != cur_get_record))
                {
                    overlap_vid = get_record->vid;
                    return(TRUE);
                }
            }
        }
        cur_get_record = cur_get_record->p_next;
    }
    return (FALSE);
}

/**
******************************************************************************
**
**  @brief      Updates the apool pointers in case of an error.
**
**  This will update the tail pointer and shadow pointer. It will also put
**  the apool into an error state.
**
**  @param      apool_id    - The apool that had the error.
**
**  @return     none
**
******************************************************************************
*/
void apool_update_error(UINT16 apool_id UNUSED)
{
    apool.tail_shadow = apool.element[apool.cur_tail_element].tail;
    apool.cur_tail_shadow_element = apool.cur_tail_element;
    apool.status |= (1 << MR_APOOL_FLUSH_ERROR);
    loc_last_seq_count = apool.last_seq_count;
#ifdef M4_EXTRA_PRINTING
PRINT_APOOL_STRUCTURE;
#endif /* M4_EXTRA_PRINTING */
fprintf(stderr,"ASYNC - update_error - element.length=%lld, element_head=%llx, element_tail=%llx\n",apool.element[0].length,apool.element[0].head, apool.element[0].tail);
    AR_NVUpdate(AR_UPDATE_TAIL);
}

/**
******************************************************************************
**
**  @brief      Assign an apool put record.
**
**  This allocates and initializes an apool_put record.
**
**  @param      ilt         - The ilt we are working with.
**  @param      apool_id    - The apool we are working on.
**
**  @return     pointer to an APOOL_PUT_RECORD
**
******************************************************************************
*/
APOOL_PUT_RECORD *apool_assign_put_record(ILT *ilt, UINT16 apool_id UNUSED)
{
    APOOL_PUT_RECORD *put_record;
    VRP            *vrp;

    put_record = s_MallocW(sizeof(APOOL_PUT_RECORD), __FILE__, __LINE__);
    put_record->ilt = ilt;
    vrp = (VRP *)ilt->ilt_normal.w0;
    put_record->sequence_count = ~0ULL;
    put_record->state = (1 << WRITE_REAL_AND_META);
    put_record->status = INCOMPLETE;
    put_record->p_next = NULL;
    // p_prev set below in both cases.

    //Insert the record at the tail
    if (apool.put_record_head == NULL)
    {
        apool.put_record_head = put_record;
        put_record->p_prev = NULL;          /* apool.put_record_tail is NULL */
    }
    else
    {
        apool.put_record_tail->p_next = put_record;
        put_record->p_prev = apool.put_record_tail;
    }
    apool.put_record_tail = put_record;
    return (put_record);
}

/**
******************************************************************************
**
**  @brief      Release an apool put record.
**
**  This frees an apool_put record if it is the oldest record to complete.
**  If so, the apool pointers need to be updated.
**
**  @param      put_record  - The APOOL_PUT_RECORD to free/release.
**
**  @return     none
**
******************************************************************************
*/
void apool_release_put_record(APOOL_PUT_RECORD *put_record, void *old_comp_rtn UNUSED)
{
    APOOL_PUT_RECORD *cur_record;
    VRP            *vrp;
    static UINT8    in_release_put_record;
    ALINK           *alink;

    if (in_release_put_record)
    {
        fprintf(stderr, "%s: Re-entering %s\n", __func__, __func__);
    }
    in_release_put_record = 1;

    // If this is the first record to fail then mark each outstanding
    // record as needing to be purged. This way as the records are
    // completed the VRPs should be marked with bad status. This should
    // force the initiator to retry the IO.
    if ((put_record->status != AOK) && !(put_record->state & (1 << PURGE_ALL_REMAINING)))
    {
        cur_record = apool.put_record_head;
        while (cur_record != NULL)
        {
            cur_record->state |= (1 << PURGE_ALL_REMAINING);
            if (cur_record->p_next == NULL)
            {
                // Set the reject puts flag to stop incoming IO to apool until the
                // currently outstanding records purge.  This will prevent data corruption.
                ap_reject_puts |= (1 << MR_REJECT_PUTS_ERR);

                // This is the last one on the list so store this record pointer
                // so that when it finally completes it will clear the reject flag.
                last_errored_put_record = cur_record;
                break;
            }
            cur_record = cur_record->p_next;
        }
        apool_update_put_error(0);

        logAPOOLevent(AP_ASYNC_BUFFER_IO_BAD, LOG_AS_DEBUG, 1, apool.id, (UINT32)put_record->status);
    }

    if ((put_record->status == AOK)&& !(put_record->state & (1 << PURGE_ALL_REMAINING)))
    {
        apool.status &= ~(1 << MR_APOOL_FILL_ERROR);
    }

    // If this request is the head (oldest) then release it and traverse the list
    // looking for others to release.
    if (apool.put_record_head == put_record)
    {
        // Free all of the records that have been completed.
        while ((put_record != NULL) && (put_record->state & (1 << REQUEST_COMPLETE)))
        {
            vrp = (VRP *)put_record->ilt->ilt_normal.w0;
            if (put_record->state & (1 << PURGE_ALL_REMAINING))
            {
                // Since the purge all bit is set we need to set the status
                // of the VRP to error if not already
                if (vrp->status == EC_OK)
                {
                    vrp->status = EC_BUSY;
                }
            }

            // Update the head pointer if successful
            if ((put_record->status == AOK) &&
                !(put_record->state & (1 << PURGE_ALL_REMAINING)))
            {
                apool.sequence_count++;
                apool_update_head(vrp->length + 1, 0);
#ifdef M4_EXTRA_PRINTING
PRINT_APOOL_STRUCTURE;
#endif /* M4_EXTRA_PRINTING */
            }

            // Update the sector count contained in the apool for this alink.
            alink = apool_get_alink(apool.id, put_record->vid);
            if (alink == NULL)
            {
                if (TRUE == apool_check_n_init_alink(gVDX.vdd[put_record->vid]))
                {
                    fprintf(stderr, "ASYNC:%s New alink added VID=%d\n", __func__, put_record->vid);
                }
            }
            else
            {
                alink->sectors_outstanding += put_record->length;
            }

            // If this was the last record that was errored or marked for purge
            // then clear the reject flag to allow new requests in the apool.
            if (put_record == last_errored_put_record)
            {
                if(deferred_unpause)
                {
                    apool_auto_alinks_start_or_pause(0, CM_ResumePoll);  // DSE fix hardcode id later
                    deferred_unpause = FALSE;
                    ap_reject_puts = 0;
                }
                ap_reject_puts &= ~(1 << MR_REJECT_PUTS_ERR);
                last_errored_put_record = NULL;
            }

            // Remove from the put_record queue
            apool.put_record_head = put_record->p_next;
            if (put_record->p_next == NULL)
            {
                apool.put_record_tail = NULL;
            }
            else
            {
                put_record->p_next->p_prev = NULL;
            }

#ifdef ILT_PRINTING
fprintf(stderr, "apool_release_put_record: Release put record ILT %p PR status 0x%x\n", put_record->ilt, put_record->status);
fprintf(stderr, "call_comp_routine %p\n", old_comp_rtn);
#endif  /* ILT_PRINTING */
            call_comp_routine((UINT32)put_record->old_cr, put_record->ilt);

            s_Free(put_record, sizeof(APOOL_PUT_RECORD), __FILE__, __LINE__);

            // Update the put_record pointer
            put_record = apool.put_record_head;
        }
    }
    else
    {
        // Do nothing until the oldest record is completed
    }

    in_release_put_record = 0;
}

/**
******************************************************************************
**
**  @brief      Updates the apool head pointers in case of an error.
**
**  This will update the head pointer and shadow pointer. It will also put the
**  apool into an error state.
**
**  @param      apool_id    - The apool we are working on.
**
**  @return     none
**
******************************************************************************
*/
void apool_update_put_error(UINT16 apool_id UNUSED)
{
    apool.head_shadow = apool.element[apool.cur_head_element].head;
    apool.cur_head_shadow_element = apool.cur_head_element;
    apool.status |= (1 << MR_APOOL_FILL_ERROR);
    shadow_sequence_count = apool.sequence_count;

#ifdef M4_EXTRA_PRINTING
PRINT_APOOL_STRUCTURE;
#endif /* M4_EXTRA_PRINTING */
fprintf(stderr,"ASYNC - update_put_error - element.length=%lld, element_head=%llx, element_tail=%llx\n",apool.element[0].length,apool.element[0].head, apool.element[0].tail);
    AR_NVUpdate(AR_UPDATE_HEAD);
}

/**
******************************************************************************
**
**  @brief      Test a sequence count for a request being flushed from the
**              apool
**
**  This function accepts a sequence count and an alink pointer.  The
**  sequence count is compared to the one stored in the alink structure.
**  The count will be kept if it is in the vicinity of the count stored in
**  the alink.
**
**  @param      id              - The apool we are working on.
**  @param      new_seq_count   - The sequence count to test
**
**  @return     TRUE            - If ok to keep.
**  @return     FALSE           - not ok to keep.
**
******************************************************************************
*/
UINT8 apool_validate_alink_seq_count(UINT64 new_seq_count,ALINK *alink)
{

    if (!((new_seq_count >> 32) & SEQ_CNT_OVERFLOW_MASK))
    {
        // This is a bogus sequence counter because it doesn't have the
        // proper seeding.
        return(FALSE);
    }

    if (new_seq_count >= alink->sequence_count)
    {
        // Error if the count is MUCH MUCH greater than that stored in the
        // alink structure.
        if ((new_seq_count - alink->sequence_count) > MUCH_MUCH_VALUE)
        {
            return(FALSE);
        }
        else
        {
            return(TRUE);
        }
    }
    else
    {
        // Error if the count is MUCH MUCH less than that stored in the
        // alink structure.  Note -- since the alink sequence count is
        // not stored in nvram it can happen that the value stored in the
        // alink structure is greater than values retrieved from the apool.
        if ((alink->sequence_count - new_seq_count) > MUCH_MUCH_VALUE)
        {
            return(FALSE);
        }
        else
        {
            return(TRUE);
        }
    }
}

/**
******************************************************************************
**
**  @brief      Test an incoming sequence count to see if is suitable to store
**              in nvram
**
**  This function accepts an apool id and a sequence count.  The sequence count
**  is compared to the current one in memory to see if it is reasonable.  Since
**  the sequence count is seeded with a scaled up time value if the apoool is
**  destroyed and recreated the new sequence count will be drastically different
**  than the current one.  If it is drastically different we need to save it
**  to nvram.
**
**  @param      id              - The apool we are working on.
**  @param      new_seq_count   - The sequence count to test
**
**  @return     TRUE            - If ok to keep.
**  @return     FALSE           - not ok to keep.
**
******************************************************************************
*/
UINT8 apool_validate_seq_count(UINT64 new_seq_count,UINT16 apool_id UNUSED)
{
    if (!((new_seq_count >> 32) & SEQ_CNT_OVERFLOW_MASK))
    {
        // This is a bogus sequence counter because it doesn't have the
        // proper seeding.
        fprintf(stderr,"Apool Validate Seq Count: Bad seed %llu\n",new_seq_count);
        return(FALSE);
    }

    if (new_seq_count == 0ULL)
    {
        // The mirror partner is trying to clear out this NVRAM.
        return(TRUE);
    }

    if (new_seq_count >= apool.sequence_count)
    {
        // Always keep it if greater than current.
        return(TRUE);
    }
    else
    {
        // Since the count is less than the current the only way we should
        // keep it is if it is MUCH MUCH less than the current.  This would
        // indicate that the apool had been recreated.  To beat this check
        // the user would have create,delete,then recreate all under
        // 30 seconds.
        if ((apool.sequence_count - new_seq_count) > MUCH_MUCH_VALUE)
        {
            return(TRUE);
        }
        else
        {
            fprintf(stderr,"Apool Validate Seq Count: New seq %llu, not much less than current %llu\n",new_seq_count,apool.sequence_count);
            return(FALSE);
        }
    }
}

/**
******************************************************************************
**
**  @brief      Test to see if an apool can be expanded
**
**  This function accepts an apool id and checks to see if that apool can be
**  expanded.
**
**  @param      id          - The apool we are working on.
**
**  @return     TRUE        - If ok to expand.
**  @return     FALSE       - not ok to expand.
**
******************************************************************************
*/
UINT16 apool_can_expand(UINT16 apool_id UNUSED)
{
    if (apool.element_count < MAX_ELEMENTS)
    {
        return(TRUE);
    }
    logAPOOLevent(AP_ASYNC_BUFFER_NOEXPAND, LOG_AS_ERROR, 0, apool.id ,apool.element_count);
    return(FALSE);
}

/**
******************************************************************************
**
**  @brief      Expand an apool
**
**  This function accepts a length parameter and adds it to the current
**  shadow head pointer.
**
**  @param      new_size    - The new size of the apool vdisk.
**  @param      vid         - The apool we are working on.
**
**  @return     AOK         - If everything is ok.
**  @return     else error.
**
******************************************************************************
*/
UINT16 apool_expand(UINT64 new_size, UINT16 vid)
{
    VDD *pVDD;

    pVDD = V_vddindx[vid];

    /* First make sure we are really working on the apool */
    if (pVDD == NULL)
    {
       return(DEINOPDEV);
    }
    if (BIT_TEST(pVDD->attr,VD_BVLINK) || !BIT_TEST(pVDD->attr,VD_BASYNCH))
    {
       return(DEINOPDEV);   /* could make this more unique later */
    }
    /* Now make sure we aren't allowing too many expands */
    if (apool.element_count >= MAX_ELEMENTS)
    {
        return(MAX_ELEMENTS_ERROR);
    }

    //Fill in the element structure
    apool.element[apool.element_count].sda =
        apool.element[apool.element_count - 1].sda +
        apool.element[apool.element_count - 1].length;

    apool.element[apool.element_count].vid =
        apool.element[apool.element_count - 1].vid;

    apool.element[apool.element_count].status = AOK;

    apool.element[apool.element_count].head = 0;

    apool.element[apool.element_count].tail = 0;

    apool.element[apool.element_count].apool_id =
        apool.element[apool.element_count - 1].apool_id;

    apool.element[apool.element_count].length =
        new_size - apool.element[apool.element_count].sda;

    apool.element[apool.element_count].jump_offset = ~0ULL;

    apool.element[apool.element_count].jump_to_element = ~0;

    apool.element_count++;

    apool.length = new_size;

fprintf(stderr,"ASYNC - apool_expand - element.length=%lld, element_head=%llx, element_tail=%llx\n",apool.element[0].length,apool.element[0].head, apool.element[0].tail);
    logAPOOLevent(AP_ASYNC_BUFFER_EXPANDED, LOG_AS_INFO, 0, apool.id ,(UINT32)(new_size/2048L));

    AR_NVUpdate(AR_APOOL_EXPAND);

    return(AOK);
}

/**
******************************************************************************
**
**  @brief      Initialize an alink
**
**  This function accepts a vid parameter and initializes an alink
**  structure and links it to the apool
**
**  @param      vid    - The virtual id of the alink.
**
**  @return     AOK
**
******************************************************************************
*/
UINT16 alink_init(UINT16 vid)
{
    ALINK *alink;

    logAPOOLevent(AP_ALINK_SET, LOG_AS_INFO, 0, vid ,0);

    alink = s_MallocW(sizeof(ALINK), __FILE__, __LINE__);
    alink->next = NULL;
    alink->vid = vid;
    alink->status = AOK;
    alink->sectors_outstanding = 0UL;
    alink->sequence_count = apool.sequence_count;
    alink->last_time_stamp = 0UL;
    alink->apool_id = apool.id;
    alink->apool_percent_consumed = 0;

    if (apool.alink_head == NULL)
    {
        apool.alink_head = alink;
    }
    if (apool.alink_tail != NULL)
    {
        apool.alink_tail->next = alink;
    }
    apool.alink_tail = alink;
    apool.alink_count++;
    AR_NVUpdate(AR_ALINK_INIT);
    return(AOK);
}

/**
******************************************************************************
**
**  @brief      Delete an alink
**
**  This function accepts a vid parameter and deallocates the alink structure
**  if it exists and unlinks it from the apool.
**
**  @param      vid    - The virtual id of the alink.
**
**  @return     none
**
******************************************************************************
*/
void alink_delete(UINT16 vid)
{
    ALINK *alink, *prev_alink;

    logAPOOLevent(AP_ALINK_UNSET, LOG_AS_INFO, 0, vid ,0);

    alink = apool.alink_head;
    prev_alink = NULL;
    while (alink)
    {
        if (alink->vid == vid)
        {
            if (prev_alink)
            {
                prev_alink->next = alink->next;
            }
            if (apool.alink_head == alink)
            {
                apool.alink_head = alink->next;
            }
            if (apool.alink_tail == alink)
            {
                apool.alink_tail = prev_alink;
            }
            apool.alink_count--;
            s_Free(alink, sizeof(ALINK), __FILE__, __LINE__);
            AR_NVUpdate(AR_ALINK_DELETE);
            return;
        }
        prev_alink = alink;
        alink = alink->next;
    }
fprintf(stderr, "alink_delete: requested alink to be deleted not found (%u).\n", vid);
}

ALINK *apool_get_alink(UINT16 apool_id UNUSED, UINT16 vid)
{
    ALINK       *alink = apool.alink_head;
    INT16       al_count = apool.alink_count;

    while (alink && al_count > 0)
    {
        if (alink->vid == vid)
        {
            return alink;
        }
        alink = alink->next;
        --al_count;
    }
    if (alink != NULL || al_count != 0)
    {
        show_alink_inconsistency(al_count, alink, __func__);
    }
    return NULL;
}

/**
******************************************************************************
**
**  @brief      Delete an apool
**
**  This function accepts a vid parameter and deallocates the apool structure
**  if it exists and clears nvram.
**
**  @param      vid    - The virtual id of the alink.
**
**  @return     none
**
******************************************************************************
*/
void apool_kill_task(UINT16 vid UNUSED)
{
    // Clear NVRAM here.  This should be propagated to the remote controller
    // so that the remote user task (if active) will kill itself.
    AR_ClearAsyncNVRAM();

    /*
     * If mover task is in the process of being created, wait till it is.
     */
    while (apool.mover_task_pcb == (PCB *)-1)
    {
        TaskSleepMS(50);       // Wait for the mover task to start.
    }

    // If there is a mover task make sure it is active and have it kill itself.
    if (apool.mover_task_pcb)
    {
        if (TaskGetState(apool.mover_task_pcb) == PCB_NOT_READY)
        {
#ifdef HISTORY_KEEP
CT_history_pcb("apool_kill_task setting ready pcb", (UINT32)apool.mover_task_pcb);
#endif  /* HISTORY_KEEP */
            TaskSetState(apool.mover_task_pcb, PCB_READY);
        }
        mover_task_status |= (1 << MR_APOOL_KILL_MOVER);
        while (mover_task_status & (1 << MR_APOOL_KILL_MOVER))
        {
            // Wait for the mover to exit
            TaskSleepMS(100);
        }
    }

    // Note -- there shouldn't be any get/put records or alink structs at this point
    // If there are they will be memory leaked and probably cause a crash sometime if
    // they are ever used again.
    if (apool.get_record_head || apool.put_record_head || apool.alink_head)
    {
        // TODO log something here
    }

    // TODO release the alinks if any
    free_all_alinks();

    // Clear the apool and element structs.
    memset(&apool,0,sizeof(APOOL));

    // Signal that the apool can be initialized again
    g_apool_initialized = FALSE;
    gApoolActive = FALSE;
    gAsyncApoolOwner = FALSE;
    gApoolOwner = 0xffffffff;
    last_errored_put_record = NULL;
    shadow_sequence_count = ~0ULL;

    // Clear NVRAM again here since the mover related work could have updated
    // pointers and such.
    AR_ClearAsyncNVRAM();
}

void apool_delete(UINT16 vid)
{
    logAPOOLevent(AP_ASYNC_BUFFER_UNSET, LOG_AS_INFO, 0, apool.id, apool.element[0].vid);
    CT_fork_tmp = (unsigned long)"Apool Kill Task";
    TaskCreate3(C_label_referenced_in_i960asm(apool_kill_task), KILL_PRIORITY,(int)vid);
}

void apool_disown(UINT16 apool_id UNUSED)
{
    PRINT_APOOL_STRUCTURE;

    /*
     * If mover task is in the process of being created, wait till it is.
     */
    while (apool.mover_task_pcb == (PCB *)-1)
    {
        TaskSleepMS(50);       // Wait for the mover task to start.
    }

    if (apool.mover_task_pcb)
    {
        mover_task_status |= (1 << MR_APOOL_KILL_MOVER);
    }

    // Wait for the mover task to complete
    while (mover_task_status & (1 << MR_APOOL_KILL_MOVER))
    {
        // Wait for the mover to exit
        TaskSleepMS(100);
    }

    // Wait for all of the put records to complete
    while(apool.put_record_head)
    {
        // Wait for the mover to exit
        TaskSleepMS(100);
    }

    // Wait for all of the get records to complete
    while (apool.get_record_head)
    {
        // Wait for the mover to exit
        TaskSleepMS(100);
    }

    free_all_alinks();

    apool.last_seq_count = ~0ULL;
    loc_last_seq_count = ~0ULL;
    last_errored_put_record = NULL;
    shadow_sequence_count = ~0ULL;
    cur_ap_data = 0;

    last_event = 0xff;
    g_apool_initialized = FALSE;
}

/**
******************************************************************************
**
**  @brief      Copy Apool data structures for fid read.
**
**  @param      ap      - Pointer to APOOL to copy to buffer.
**  @param      where   - Handle to buffer location to put data, updated.
**  @param      Left    - Pointer to Length of buffer available, updated.
**
**  @return     status  DEOK - ok, or error elsewise.
**
******************************************************************************
**/
UINT8 Copy_in_Apool(APOOL *ap, char **where, UINT32 *Left)
{
    UINT32          Length;
    UINT32         *lth;
    INT16           al_count;   /* Number of alinks */
    char           *buf;
    ALINK          *alp;

    al_count = ap->alink_count;
    Length = sizeof(APOOL) + (al_count * sizeof(ALINK));
    /* Make sure enough room in buffer for data and "length" before it. */
    if ((Length + sizeof(UINT32)) > *Left)
    {
        *Left = 0;                      /* Nothing left to use of buffer. */
        return (DETOOMUCHDATA);
    }

    /* Compute the percent full for this apool. */
    apool.percent_full = apool_get_percent_full(ap->id);

    /* Output format is length of data, followed by data. */

    /* Copy in the sum of the apool structure and the ALINK's. */
    lth = (UINT32 *)*where;
    *lth = Length;

    /* Copy in the apool structure. */
    buf = (char *)(lth + 1);
    memcpy(buf, ap, sizeof(APOOL));

    /* Copy in the ALINK's. */
    buf += sizeof(APOOL);               /* Set for ALINK data. */

    alp = ap->alink_head;               /* Pointer to first ALINK. */
    while (alp != NULL && al_count > 0)
    {
        memcpy(buf, alp, sizeof(ALINK));
        buf += sizeof(ALINK);           /* Set for next ALINK data. */
        alp = alp->next;                /* Go to next ALINK. */
        --al_count;
    }
    if (al_count != 0 || alp != NULL)
    {
        show_alink_inconsistency(al_count, alp, __func__);
    }

    /* Set buffer pointer to next available location. */
    *where = *where + Length;           /* Set for next apool. */

    /* Set length left to include the leading length of the data. */
    *Left = *Left - (Length + sizeof(UINT32));

    return (DEOK);
}                                       /* End of Copy_in_Apool */

/**
******************************************************************************
**
**  @brief      Get Async replication data (MRP).
**
**  @param      pMRP    - MRP packet.
**
**  @return     status  DEOK - ok, or error elsewise.
**
******************************************************************************
**/
UINT8 DEF_AsyncRep(MR_PKT *pMRP)
{
    UINT8           retVal;
    MRASYNCREP_REQ *pReq = (MRASYNCREP_REQ *)(pMRP->pReq);
    MRASYNCREP_RSP *pRsp = (MRASYNCREP_RSP *)(pMRP->pRsp);
    UINT32          Apool_count = 0;
    char           *buf;
    UINT32          LengthLeft = pReq->blen;

    if (pReq->bptr == NULL)
    {
        retVal = DEBADDAM;
    }
    else if (LengthLeft < sizeof(UINT32))
    {
        retVal = DETOOMUCHDATA;
    }
    else
    {
        buf = (char *)pReq->bptr;
        Apool_count++;
        retVal = Copy_in_Apool(&apool, &buf, &LengthLeft);
    }
    /*
     * Set up MRP response packet variables.
     */
    pRsp->count = Apool_count;
    pRsp->retLen = pReq->blen - LengthLeft;
    pRsp->status = retVal;

    return (retVal);
}                                       /* End of DEF_AsyncRep */

/**
******************************************************************************
**
**  @brief      Log APOOL (Async replication) tokenized message to CCB.
**
**  @param      UINT8 event_code - this is a sub event code that defines the message (see logview.c)
**              UINT8 severity   - 0=info, 1=warning, 2=error, 4=debug
**              UINT32 errorCode - application specific error code
**              UINT32 value1    - application specific data value
**              UINT32 value2    - application specific data value
**
**  @return     none
**
**  @attention  Also sets the event bit for async replication.
**
******************************************************************************
**/
void logAPOOLevent(UINT8 event_code, UINT8 severity,UINT32 errorCode,UINT32 value1,UINT32 value2)
{
    LOG_APOOLEVENT_PKT  logbuffer;

    switch(severity)
    {
        case 0:
            logbuffer.header.event = LOG_APOOL_CHANGE_I;
            break;
        case 1:
            logbuffer.header.event = LOG_APOOL_CHANGE_W;
            break;
        case 2:
            logbuffer.header.event = LOG_APOOL_CHANGE_E;
            break;
        case 4:
            logbuffer.header.event = LOG_APOOL_CHANGE_D;
            break;
        default:
            break;
    }
    logbuffer.data.sub_event_code = event_code;
    logbuffer.data.errorCode = errorCode;
    logbuffer.data.value1 = value1;
    logbuffer.data.value2 = value2;
    /*
     * Note: message is short, and L$send_packet copies into the MRP.
     */
    MSC_LogMessageStack(&logbuffer, sizeof(LOG_APOOLEVENT_PKT));
}                                       /* End of logAPOOLevent */

/**
******************************************************************************
**
**  @brief      Log SPOOL (SNAPSHOT POOL) tokenized message to CCB.
**
**  @param      UINT8 event_code - this is a sub event code that defines the message (see logview.c)
**              UINT8 severity   - 0=info, 1=warning, 2=error, 4=debug
**              UINT32 errorCode - application specific error code
**              UINT32 value1    - application specific data value
**              UINT32 value2    - application specific data value
**
**  @return     none
**
**  @attention  Also sets the event bit for snapshot.
**              NOTE: move this routine to snapshot.c when we convert snapshot to C
**
******************************************************************************
**/
void logSPOOLevent(UINT8 event_code, UINT8 severity,UINT32 errorCode,UINT32 value1,UINT32 value2)
{
    LOG_SPOOLEVENT_PKT  logbuffer;

    switch(severity)
    {
        case 0:
            logbuffer.header.event = LOG_SPOOL_CHANGE_I;
            break;
        case 1:
            logbuffer.header.event = LOG_SPOOL_CHANGE_W;
            break;
        case 2:
            logbuffer.header.event = LOG_SPOOL_CHANGE_E;
            break;
        case 4:
            logbuffer.header.event = LOG_SPOOL_CHANGE_D;
            break;
        default:
            break;
    }
    logbuffer.data.sub_event_code = event_code;
    logbuffer.data.errorCode = errorCode;
    logbuffer.data.value1 = value1;
    logbuffer.data.value2 = value2;
    logbuffer.data.skip_notify = 0;
    /*
     * Note: message is short, and L$send_packet copies into the MRP.
     */
    MSC_LogMessageStack(&logbuffer, sizeof(LOG_SPOOLEVENT_PKT));
}                                       /* End of logSPOOLevent */

/**
******************************************************************************
**
**  @brief      To provide a means of breaking all (or just one) asynch mirror.
**
**  @param      UINT16 vid - Virtual identifier of destination to break (0xffff=all)
**
**  @return     Return TRUE if no errors, else returns FALSE
**
**  @attention  register g3 is changed when CCSM_Copy_ended is called.
**              This is ok on: 2007-10-12 -- Marshall Midden
**
******************************************************************************
**/
UINT16 apool_break_async_mirrors(UINT16 vid)
{
    int   index_D;
    VDD  *ptVDD;
    COR  *pCOR;
    int   do_all=0;

#ifdef APOOL_HEAD_TAIL_UPDATING_PRINTING
    if(dav_file)
    {
        fclose(dav_file);
    }
#endif  /* APOOL_HEAD_TAIL_UPDATING_PRINTING */
    PRINT_APOOL_STRUCTURE;
    /* Now, determine if it is a VDisk or a VLink that we are dealing with */
    if ( vid == 0xffff )
    {
        do_all=1;
    }
    else
    {
        do_all = 0;
        ptVDD = gVDX.vdd[vid];
        if (ptVDD == NULL)
        {
            return(FALSE);
        }
    }

    /* loop through all possible vdisks looking for an ALink       */
    for (index_D = 0; index_D < MAX_VIRTUAL_DISKS; index_D++)
    {
        if ( !do_all && index_D != vid)
        {
            continue;
        }
        ptVDD = gVDX.vdd[index_D];
        if (ptVDD != NULL)
        {
            /* if we are a vlink  and the async attribute is set */
            if (BIT_TEST(ptVDD->attr, VD_BVLINK) &&
                BIT_TEST(ptVDD->attr, VD_BASYNCH) )    /* dest an alink? */
            {
                /* Proceed only if the VDD is  a destination copy device */
                /* and the Copy registration exists                      */
                if ((DCD *)(ptVDD->pDCD)== NULL)
                {
                    continue;
                }
                /* get the cor address related to this mirror            */
                pCOR = ( (DCD *)(ptVDD->pDCD))->cor;
                if (pCOR == NULL)
                {
                    continue;
                }
                /* and finally pass the cor address as g3 into the .as code */
                CCSM_Copy_ended(pCOR);
            }
        }
    }

    /*
    ** Wait till the copies actually break.
    */
    for(index_D = 0; index_D < MAX_VIRTUAL_DISKS; index_D++)
    {
        ptVDD = gVDX.vdd[index_D];
        if ((ptVDD != NULL) &&
            BIT_TEST(ptVDD->attr, VD_BVLINK) &&
            BIT_TEST(ptVDD->attr, VD_BASYNCH)&&
            ((DCD*)(ptVDD->pDCD)!= NULL) &&
            ((DCD*)(ptVDD->pDCD)->cor !=NULL)
           )
        {
            fprintf(stderr,"apool_break_async_mirrors: Waiting for mirrors to break\n");
            TaskSleepMS(1000);
            --index_D; /* Check the same destination vid once again */
        }
    }

    return TRUE;
}

/**
******************************************************************************
**
**  @brief     To check whether the association to this target is OK or not.
**             This function is called when we get server associate(map lun) MRP.
**
**  @param     vid, tid
**
**  @return    status
**
******************************************************************************
**/
UINT32 apool_validate_server_association(UINT16 vid, UINT16 tid)
{
    VDD    *pVDD;
    VDD    *pDestVDD;
    COR    *pCOR;
    UINT32  status = DEOK;

    pVDD = V_vddindx[vid];
    pCOR = ((SCD*)(pVDD->pSCDHead))->cor;
    pDestVDD = pCOR->destvdd;

    if ((pDestVDD != NULL) &&
        BIT_TEST(pDestVDD->attr,VD_BVLINK) &&
        BIT_TEST(pDestVDD->attr,VD_BASYNCH))
    {
        status = apool_validate_target (tid);
    }
    return(status);
}

/**
******************************************************************************
**
**  @brief     To check whether the copy is allowed if the destination device is
**             alink. This function allows the copy operation only if th target
**             is in apool owning target list.
**
**  @param     SrcVDD, DestVDD pointers
**
**  @return    status
**
******************************************************************************
**/
UINT32 apool_validate_async_copy(VDD *pSrcVDD, VDD *pDestVDD)
{
    UINT32 status = DEOK;
    UINT16 tid;

    if ((pDestVDD != NULL) &&
        BIT_TEST(pDestVDD->attr,VD_BVLINK) &&
        BIT_TEST(pDestVDD->attr,VD_BASYNCH))
    {

        tid = apool_get_tid (pSrcVDD);

        if (tid != 0xffff)
        {
           // there is server association to this vdisk
           status = apool_validate_target (tid);
        }
        else
        {
           status = DEOK;
        }
        fprintf(stderr,"ASYNC-apool_validate_async_copy: status=%u\n", status);
    }
    return(status);
}

/**
******************************************************************************
**
**  @brief     To check whether the set attribute operation is allowed or not
**             based on the target that is owning the vdisk.
**
**  @param     vid
**
**  @return    status
**
******************************************************************************
**/
UINT32 apool_validate_attr(UINT16 vid)
{
    COR* pCOR;
    UINT32 status = DEOK;
    UINT16 tid=0xffff;

    if (BIT_TEST(gVDX.vdd[vid]->attr, VD_BVLINK) && (gVDX.vdd[vid]->pDCD != NULL))
    {
        pCOR = ((DCD*)(gVDX.vdd[vid]->pDCD))->cor;

        if ((pCOR != NULL) && (pCOR->srcvdd != NULL))
        {
            //There is copy associated to this vlink, check whether the src vdisk
            //of this copy is associated to a server. If it is, validate the target

            tid = apool_get_tid (pCOR->srcvdd);
            if (tid != 0xffff)
            {
                status = apool_validate_target (tid);
            }
        }
    }
fprintf(stderr,"%s: ret status = %u\n",__func__,status);
//Probably we may need to send a different status when the set attribute shouldn't be allowed
//due to invalid target ...

    return(status);
}

/**
******************************************************************************
**
**  @brief     Finds whether the given target is allowed to own the async copy/
**             mirrors.
**
**  @param     tid
**
**  @return    status
**
******************************************************************************
**/
UINT32 apool_validate_target (UINT16 target_id)
{
    UINT32 status = DEINVTID;

    if (gApoolOwner == 0xffffffff)
    {
       /* Apool owner is not set yet, set it to the preferred owner of the target */
       gApoolOwner = T_tgdindx[target_id]->prefOwner;
       status = DEOK;
    }
    else if (gApoolOwner == T_tgdindx[target_id]->prefOwner)
    {
       /* The target's preferred owner is same as the Apool owner, so allow the assignment*/
       status = DEOK;
    }
    else if (T_tgdindx[target_id]->prefOwner != (gApoolOwner ^ 1))
    {
        // The preferred owner is not this controller or the partner controller
        // It must be set to no owner (whatever that constant is, I couldn't find it
        // so I did this hack).
        status = DEOK;
    }
    return(status);
}

/**
******************************************************************************
**
**  @brief     get tid of a given vdd if the server association is present.
**
**
**  @param     VDD pointer
**
**  @return    tid - if the association is present
**             0xffff - association is not there.
**
******************************************************************************
**/
UINT16 apool_get_tid(VDD *pVDD)
{
    UINT16 tid = 0xffff;
    UINT32 i;
    SDD   *sdd;
    LVM   *lvm;

    for (i = 0; (i < MAX_SERVERS) && (tid==0xffff); ++i)
    {
        if (NULL != (sdd = gSDX.sdd[i]))
        {
            /* Check both the visible and invisible mappings. */
            for (lvm = sdd->lvm; lvm != NULL; lvm = lvm->nlvm)
            {
                if (pVDD->vid == lvm->vid)
                {
                    // this vdisk is mapped to a server
                    tid = sdd->tid;
                    break;
                }
            }
            if (tid == 0xffff)
            {
                // check in the invisible lvm.
                for (lvm = sdd->ilvm; lvm != NULL; lvm = lvm->nlvm)
                {
                    if (pVDD->vid == lvm->vid)
                    {
                        tid = sdd->tid;
                        break;
                    }
                }
            }
        }
     }
     return(tid);
}

/**
******************************************************************************
**
**  @brief     Find the first empty element
**
**  @param     none
**
**  @return    element id to jump to (0 based) else MAX_ELEMENTS if none
**
******************************************************************************
**/
UINT16 find_first_empty(void)
{
    int i;

    for (i=0;i<apool.element_count;i++)
    {
        if (apool.element[i].head == apool.element[i].tail)
        {
            return(i);
        }
    }
    return(MAX_ELEMENTS);
}

/**
******************************************************************************
**
**  @brief     Check whether the VDD is alink and if it is, call alink_init
**
**  @param     VDD
**
**  @return    none
**
******************************************************************************
**/
UINT32 apool_check_n_init_alink(VDD *vdd)
{
    if ((vdd != NULL) &&
        (vdd->pDCD != NULL)&&
        BIT_TEST(vdd->attr, VD_BASYNCH) &&
        BIT_TEST(vdd->attr,VD_BVLINK))
    {
         alink_init(vdd->vid);
         return(TRUE);
    }
    return (FALSE);
}

/**
******************************************************************************
**
**  @brief     Print the apool structure nice and simply for debugging.
**
**  @param     none
**
**  @return    none
**
******************************************************************************
**/

void apool_structure_print(const char *str, unsigned long linenumber)
{
    int i;

    /* Compute the percent full for this apool. */
    apool.percent_full = apool_get_percent_full(0);

    fprintf(stderr, "APOOL: %s:%lu\n", str, linenumber);
    fprintf(stderr, "  id=%u status=%u percent_full=%u, length=%llu mover_task_pcb=%p\n",
                          apool.id, apool.status, apool.percent_full, apool.length, apool.mover_task_pcb);
    fprintf(stderr, "  cur_head_element=%u cur_tail_element=%u\n",
                          apool.cur_head_element, apool.cur_tail_element);
    fprintf(stderr, "  cur_head_shadow_element=%u cur_tail_shadow_element=%u\n",
                          apool.cur_head_shadow_element, apool.cur_tail_shadow_element);
    fprintf(stderr, "  head_shadow=%llu tail_shadow=%llu\n",
                          apool.head_shadow, apool.tail_shadow);
/* Not currently printing out the APOOL_GET/PUT_RECORD's. */
    fprintf(stderr, "  sequence_count=%llu\n", apool.sequence_count);
    fprintf(stderr, "  alink_count=%u, element_count=%u, alink_head=%p, alink_tail=%p\n",
                          apool.alink_count, apool.element_count, apool.alink_head, apool.alink_tail);
    fprintf(stderr, "  last_seq_count=%llu, last_loc_seq_count=%llu\n",
                          apool.last_seq_count, loc_last_seq_count);
    fprintf(stderr, "  shadow_sequence_count=%llu, mover_que_depth=%u, mover_que_data=%u\n",
                          shadow_sequence_count, mover_que_depth, mover_que_data);
    fprintf(stderr, "  get_record_head=%p, get_record_tail=%p, put_record_head=%p, put_record_tail=%p\n",
                          apool.get_record_head, apool.get_record_tail, apool.put_record_head, apool.put_record_tail);

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    if (apool.element_count == 0)
    {
        fprintf(stderr, "  No elements in use.\n");
    }
    for (i = 0; i < apool.element_count; i++)
    {
        fprintf(stderr, "  Element %u: apool_id=%u, status=%u, vid=%u, length=%llu\n",
                        i, apool.element[i].apool_id, apool.element[i].status,
                        apool.element[i].vid, apool.element[i].length);
        fprintf(stderr, "    sda=%llu, head=%llu, tail=%llu\n",
                        apool.element[i].sda, apool.element[i].head, apool.element[i].tail);
        fprintf(stderr, "    jump offset=%llu, jump to element=%u\n",
                        apool.element[i].jump_offset, apool.element[i].jump_to_element);
    }
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    if (apool.alink_count == 0)
    {
        fprintf(stderr, "  No alinks in use.\n");
    }

    ALINK   *p = apool.alink_head;
    INT16   al_count = apool.alink_count;

    while (p && al_count > 0)
    {
        fprintf(stderr, "  Alink %u: next=%p, vid=%u, status=%u\n",
                        al_count, p->next, p->vid, p->status);
        fprintf(stderr, "    sectors_outstanding=%llu, last_sequence_count=%llu\n",
                        p->sectors_outstanding, p->sequence_count);
        fprintf(stderr, "    last_time_stamp=%llu, apool_percent_consumed=%u, id=%u\n",
                        p->last_time_stamp, p->apool_percent_consumed, p->apool_id);
        p = p->next;
        --al_count;
    }
    if (p != NULL || al_count != 0)
    {
        show_alink_inconsistency(al_count, p, __func__);
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
