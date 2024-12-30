/* $Id: apool.h 161368 2013-07-29 14:53:10Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       apool.h
**
**  @brief      Async pool manager header
**
**  To provide support for the async replication pool vdisk. This includes
**  all management functions and variables
**
**  Copyright (c) 2007-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef APOOL_H
#define APOOL_H
#include "vdd.h"
#include "vrp.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

#define APOOL_VID               0
#define MAX_ELEMENTS            5
#define MAX_ALINKS              47
#define MOVER_PRIORITY          110
#define  STATS_PRIORITY         200
#define MOVER_MAX_QUE_DEPTH     100
#define KILL_PRIORITY           100
#define KILL_TIMEOUT            44
#define SLOP_SECTORS            4096
#define ASYNC_COMPAT_VERSION    1
#define SEQ_CNT_OVERFLOW_MASK   0x3FFFFFFF
#define MUCH_MUCH_VALUE         0x1e00000000ULL

// General status values.
#define AOK                     0
#define INCOMPLETE              1
#define OVERLAP_TIMEOUT         2
#define APOOL_WRITE_ERROR       3
#define APOOL_FULL_ERROR        4
#define MAX_ELEMENTS_ERROR      5

// apool/element status bits.
// See Shared/Inc/MR_Defs.h (doxygen group MR_APOOL_STATUS).

// APOOL_GET_RECORD state bits.
#define READ_META_DATA          0
#define READ_REAL_DATA          1
#define WRITE_REAL_DATA         2
#define FLUSH_COMPLETE          3

// APOOL_PUT_RECORD state bits.
#define PURGE_ALL_REMAINING     4
#define REQUEST_COMPLETE        5
#define WRITE_REAL_AND_META     6

// General purpose log values
#define LOG_AS_INFO             0
#define LOG_AS_WARNING          1
#define LOG_AS_ERROR            2
#define LOG_AS_DEBUG            4

// Tossing apool data reasons
#define NO_VDD                  1
#define NOT_ALINK               2
#define NO_MIRROR               3
#define NO_ALINK                4
#define BAD_SEQ_COUNT           5

// Meta Data Attribute bitfield definitions
#define TIME_STAMP_VALID        0
#define COPY_DATA               1


// Following adds printing of whole apool structure (see code for where).
#define PRINT_APOOL_STRUCTURE   apool_structure_print(__FUNCTION__, __LINE__)

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

// APOOL_META_DATA is prepended to the original data when it is written to the
// apool.
typedef struct APOOL_META_DATA
{
    UINT16  vid;                    // VID of the original request.
    UINT16  dummy;
    UINT64  sda;                    // SDA of the original request.
    UINT64  length;                 // Length of the original request in sectors.
    UINT64  sequence_count;         // Sequence count assigned to this transaction.
    UINT32  attributes;             // Place holder for things like compression, etc.
    UINT32  time_stamp;             // Timestamp when IO received

} APOOL_META_DATA;

// The APOOL_GET_RECORD is used by the mover task to track requests that are
// in progress.
typedef struct APOOL_GET_RECORD
{
    struct APOOL_GET_RECORD *p_next; // Next get record.
    struct APOOL_GET_RECORD *p_prev; // Previous get record.
    UINT16  status;                 // Status of this operation.
    UINT16  state;                  // State of this operation.
    UINT16  vid;                    // vid of associated request.
    UINT16  retry_count;            // Number of times to retry the request
    struct ILT  *ilt;               // Associated ILT.
    UINT64  sda;                    // SDA of original request.
    UINT64  length;                 // Length of original request.
    UINT64  sequence_count;         // Sequence count from meta data.
} APOOL_GET_RECORD;

// The APOOL_PUT_RECORD is used to force chronology of writes to the apool.
// Writes may be queued to the apool but cannot be allowed to complete out of order.
typedef struct APOOL_PUT_RECORD
{
    struct APOOL_PUT_RECORD *p_next; // Next get record.
    struct APOOL_PUT_RECORD *p_prev; // Previous get record.
    UINT16  status;                 // Status of this operation.
    UINT16  state;                  // State of this operation.
    struct ILT  *ilt;               // Associated ILT.
    UINT16  vid;                    // The vid of the original request.
    UINT64  sda;                    // SDA of original request.
    UINT64  length;                 // Length of original request.
    UINT64  sequence_count;         // Sequence count from meta data.
    void    *old_cr;                // The original copletion routine for the request
} APOOL_PUT_RECORD;

// ALINK contains statistics for a given alink.
typedef struct ALINK
{
    struct ALINK *next;             // Pointer to next alink.
    UINT16  vid;                    // Associated vid .
    UINT16  status;
    UINT64  sectors_outstanding;    // Number of sectors consumed in apool.
    UINT64  sequence_count;         // Seq count of the last IO.
    UINT64  last_time_stamp;        // Last time stamp to be removed from apool.
    UINT16  apool_id;               // Id of the owning apool.
    UINT8   apool_percent_consumed; // Percent of apool consumed by this alink.
} ALINK;

// APOOL_ELEMENT is used to represent the part of a Vdisk to be used as an
// apool. Each time an expansion of the apool is done a new element is
// added to the apool.
typedef struct APOOL_ELEMENT
{
    UINT16  apool_id;               // ID of the owning apool.
    UINT16  status;                 // Current status bits .
    UINT16  vid;                    // VID where this element resides.
    UINT16  jump_to_element;        // The element number to jump to (0 based).
    UINT64  length;                 // Length of this emement in sectors.
    UINT64  sda;                    // SDA for this element.
    UINT64  head;                   // Head pointer in sectors.
    UINT64  tail;                   // Tail pointer in sectors.
    UINT64  jump_offset;            // Offset that Shadow head jumped from
} APOOL_ELEMENT;

// APOOL contains all of the data necessary to maintain an apool.
typedef struct APOOL
{
    UINT16  id;                     // ID of this apool.
    UINT16  status;                 // Status bit field.
    UINT8   percent_full;           // Set when fidread 355 done.
    UINT8   version;                // Version of this record (firmware only)
    UINT8   reserved[2];
    UINT16  cur_head_element;       // Element containing current head pointer.
    UINT16  cur_tail_element;       // Element containing current tail pointer.
    UINT16  cur_head_shadow_element; // Element containing current head shadow pointer.
    UINT16  cur_tail_shadow_element; // Element containing current tail shadow pointer.
    UINT64  head_shadow;            // Head shadow pointer in sectors.
    UINT64  tail_shadow;            // Tail shadow pointer in sectors.
    APOOL_GET_RECORD *get_record_head; // Pointer to first (oldest) get record.
    APOOL_GET_RECORD *get_record_tail; // Pointer to last (newest) get record.
    APOOL_PUT_RECORD *put_record_head; // Pointer to first (oldest) put record.
    APOOL_PUT_RECORD *put_record_tail; // Pointer to last (newest) put record.
    UINT64  length;                 // Total size of this apool in sectors.
    UINT64  sequence_count;         // Current sequence count.
    UINT32  time_threshold;         // Time bursting threshold.
    UINT32  mb_threshold;           // Mega Byte bursting threshold.
    struct PCB  *mover_task_pcb;    // PCB of the mover for this apool.
    UINT16  alink_count;            // Number of alinks using this apool.
    UINT16  element_count;          // Number of elements in this apool.
    ALINK  *alink_head;             // Pointer to first alink struct.
    ALINK  *alink_tail;             // Pointer to last alink struct.
    APOOL_ELEMENT element[MAX_ELEMENTS]; // The possible elements.
    UINT64          last_seq_count; // The last seq count read by the mover task.
    struct PCB  *stats_task_pcb;    // PCB of the statistics task for this apool.
    UINT32  blocks_per_sec;         // Blocks flushed to remote side last second.
    UINT32  blocks_last_second;
    UINT8   reserved1[20];
} APOOL;

extern APOOL           apool;
extern UINT16 mover_task_status;
extern UINT32 cur_ap_data;
extern UINT32 max_ap_data;
extern UINT32 max_usr_ap_data;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

/* In apool.c. */
extern UINT32 apool_get(UINT16);
extern UINT64 apool_get_shadow_head(UINT16 *,UINT16);
extern UINT64 apool_get_head(UINT16 *, UINT16);
extern void apool_update_head(UINT64,UINT16);
extern UINT64 apool_work_2_do(UINT16);
extern void apool_update_tail(APOOL_GET_RECORD *,UINT16);
extern void apool_update_error(UINT16);
extern UINT16 apool_update_shadow_head(struct VRP *,UINT16);
extern void apool_update_shadow_tail(APOOL_GET_RECORD *,UINT16);
extern APOOL_GET_RECORD * apool_assign_get_record(struct ILT *, UINT16);
extern void apool_release_get_record(APOOL_GET_RECORD *,UINT16);
extern UINT8 apool_check_for_overlap(APOOL_GET_RECORD *,UINT16);
extern void apool_update_put_error(UINT16);
extern UINT64 apool_get_shadow_tail(UINT16*,UINT16);
extern UINT8 Copy_in_Apool(APOOL *ap, char **where, UINT32 *Left);
extern void logAPOOLevent(UINT8 event_code, UINT8 severity,UINT32 errorCode,UINT32 value1,UINT32 value2);
//extern void logSPOOLevent(UINT8 event_code, UINT8 severity,UINT32 errorCode,UINT32 value1,UINT32 value2);
extern UINT32 apool_put(UINT32, struct ILT *, UINT16);
extern UINT16 apool_break_async_mirrors(UINT16 vid);
extern UINT64 apool_get_tail(UINT16 *,UINT16);
extern void   apool_init(UINT16 apool_id,UINT16 apool_vid);
extern void   apool_init_struct(UINT16 apool_id,UINT16 apool_vid);
extern void   mover_task(UINT32,UINT32);
extern void   mover_stats_task(UINT32,UINT32);
extern void   apool_kill_task(UINT16);
extern void   apool_put_cr(UINT32, struct ILT *);
extern void   apool_data_read_cr(UINT32, struct ILT *);
extern void   apool_valink_data_write_cr(UINT32, struct ILT *);
extern APOOL_PUT_RECORD *apool_assign_put_record(struct ILT *, UINT16);
extern void   apool_release_put_record(APOOL_PUT_RECORD *, void *);
extern UINT16 apool_expand(UINT64, UINT16);
extern UINT16 apool_can_expand(UINT16 apool_id);
UINT32 apool_validate_server_association (UINT16 vid, UINT16 tid);
extern UINT32 apool_validate_async_copy(VDD* pSrcVDD, VDD* pDestVDD);
extern UINT32 apool_validate_attr(UINT16 vid);
extern UINT32 apool_validate_target (UINT16 target_id);
extern UINT16 apool_get_tid (VDD* pVDD);
extern UINT16 alink_init(UINT16);
extern void alink_delete(UINT16);
extern void apool_delete(UINT16);
extern void apool_validate(UINT16);
extern void apool_disown(UINT16 apool_id);
extern UINT8 apool_validate_seq_count(UINT64,UINT16);
extern ALINK * apool_get_alink(UINT16,UINT16);
extern UINT8 apool_validate_alink_seq_count(UINT64,ALINK *);
extern UINT16 find_first_empty(void);
extern UINT32 apool_check_n_init_alink(VDD* vdd);
extern void apool_structure_print(const char *, unsigned long);
extern void dumpem(void);
extern void apool_set_qd(UINT32);
extern void apool_set_data_sz(UINT32);
extern void apool_stop(void);
extern void apool_start(void);
extern void apool_set_max_ap_data(UINT32);
extern void apool_set_max_usr_ap_data(UINT32);

/* These are external routines. */
extern void CT_LC_mover_task(UINT32, UINT32);
extern void CT_LC_mover_stats_task(UINT32, UINT32);
extern void CT_LC_apool_put_cr(UINT32, struct ILT *);
extern void CT_LC_apool_data_read_cr(UINT32, struct ILT *);
extern void CT_LC_apool_valink_data_write_cr(UINT32, struct ILT *);
extern void CT_LC_apool_kill_task(UINT16);

#endif /* APOOL_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
