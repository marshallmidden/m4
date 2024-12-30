/* $Id: flightrecorder.h 156018 2011-05-27 16:01:36Z m4 $ */
/**
******************************************************************************
**
**  @file       flightrecorder.h
**
**  @brief      FlightRecorder
**
**  Copyright 2006-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef FLIGHTRECORDER_H_
#define FLIGHTRECORDER_H_

#include "XIO_Types.h"
#include "ilt.h"

#define FLIGHTRECORDER_VERSION 0x3

#define FLIGHTRECORDER_DEFAULT_RECORDS 200000

#ifdef FRTVIEW
#define PREILTADD()   uaha6.
#else
#define PREILTADD()
#endif
/* flightrecord types */
#define FR_UNUSED             0

#define FR_PRP_QUEUED_AT_HEAD 1  /* 0x01 queued at head of newioque, data is newioque length */
#define FR_PRP_QUEUED_AT_TAIL 2  /* 0x02 queued at tail of newioque, data is newioque length */
#define FR_PRP_MIRROR_DISCARD 3  /* 0x03 queued at tail of newioque, data is newioque length */
#define FR_PRP_COMPLETE       4  /* 0x04 PRP being completed out of physical, data is PRP reqStatus */
#define FR_PRP_RETRY_DECIDED  5  /* 0x05 PRP queued to errioque for future retry, data is errioque length */
#define FR_PRP_RETRY_QUEUED   6  /* 0x06 PRP moved from errioque to head of newioque, data is newioque length */
#define FR_PRP_FAILED         7  /* 0x07 PRP failed out of phsyical when device failed, data is PRP reqStatus */

#define FR_PDRIVER_ISSUED     8  /* 0x08 command sent to device driver (isp/sg), data is new orc */
#define FR_PDRIVER_COMPLETE   9  /* 0x09 command completion read from device driver (isp/sg), data is SG status */

#define FR_RRP_EXEC           10 /* 0x0a RRP executed by RAID, no data */
#define FR_RRP_COMPLETE       11 /* 0x0b RRP completed by RAID, data is RRP reqStatus */

#define FR_PRP_CANCELED       15  /* 0x0f PRP being cancelled */
#define FR_PRP_JOINED         16  /* 0x10 PRP being joined with another */

#define FR_CACHE_EXEC     20    /* c$exec */
#define FR_CACHE_IOEXEC   21    /* c$ioexec */
#define FR_CACHE_EXEC_COMPLETE 22    /* wc$io_compl */

#define FR_CACHE_READ_HIT            23 /* wc$ReadFullCacheHit */
#define FR_CACHE_WRITE_HIT_RESIDENT  24 /* wc$resident_write */
#define FR_CACHE_WRITE_RESIDENT_DATA 25 /* C$getwdata in wc$resident_write */
#define FR_CACHE_WRITE_HIT_DIRTY     26 /* wc_dirty_io */
#define FR_CACHE_WRITE_DIRTY_DATA    27 /* C$getwdata in wc$dirty_io */
#define FR_CACHE_WRITE_CACHEABLE     28 /* wc$process_io */
#define FR_CACHE_WRITE_DATA          29 /* C$getwdata in wc$process_io */

#define FR_CACHE_NC_READ                 31 /* do_nc_op */
#define FR_CACHE_NC_READ_COMPLETE        32 /* ncrcomp1 */
#define FR_CACHE_NC_WRITE_SGL            33 /* do_nc_op */
#define FR_CACHE_NC_WRITE_SGL_COMPLETE   34 /* ndopcomp */
#define FR_CACHE_NC_WRITE_PROXY          35 /* do_nc_op */
#define FR_CACHE_NC_WRITE_PROXY_DATA     36 /* ncwcomp1 */
#define FR_CACHE_NC_WRITE_PROXY_COMPLETE 37 /* ncwcomp2 */

/* flush */
#define FR_CACHE_FLUSH                     38 /* wc$FlushCoalesce */
#define FR_CACHE_FLUSH_COMPLETE            39 /* wc$FlushIOComplete*/

#define FR_CACHE_BACKGROUND_FLUSH          60 /* wc$FlushCoalesce */
#define FR_CACHE_BACKGROUND_FLUSH_COMPLETE 61 /* wc$BgFlushComplete */

#define FR_VRP_EXEC           40 /* v$exec */
#define FR_VRP_COMPLETE       41 /* v$comp */
#define FR_VRP_SCOMPLETE      42 /* v$vscomp VRP morphed into RRP, fields may be bogus */

#define FR_MAG_READ           50 /* MAG$read_com */
#define FR_MAG_WRITE          51 /* MAG$write_com */
#define FR_MAG_READ_COMPLETE  52 /* mag1_MAGcomp */
#define FR_MAG_WRITE_COMPLETE 53 /* mag1_MAGcomp */


/** enable bits for the different layers of the system */
#define FR_ENABLE_MAG       (1 << 5)
#define FR_ENABLE_CACHE     (1 << 4)
#define FR_ENABLE_VIRTUAL   (1 << 3)
#define FR_ENABLE_RAID      (1 << 2)
#define FR_ENABLE_PHYSICAL  (1 << 1)
#define FR_ENABLE_PDRIVER   (1 << 0)

typedef struct vrp_flight_record {
    UINT8  type;
    UINT8  status;
    UINT16 id;
    UINT32 length;
    UINT64 sda;
    UINT64 tsc;
    UINT16 function;
    UINT8  options;
    UINT8  path;
    UINT32 addr;
    UINT8  bytes[0];
} vrp_flight_record_t;

typedef struct rprp_flight_record {
    UINT8  type;
    UINT8  function;
    UINT16 id;
    UINT32 length;
    UINT64 sda;
    UINT64 tsc;
    UINT32 data;
    UINT32 addr;
    UINT8  bytes[0];
} rprp_flight_record_t;

typedef struct common_flight_record {
    UINT8  type;
    UINT8  rsvd1;
    UINT16 id;
    UINT32 length;
    UINT64 sda;
    UINT64 tsc;
    UINT32 rsvd2;
    UINT32 addr;
    UINT8  bytes[0];
} common_flight_record_t;

typedef union flight_record {
    vrp_flight_record_t    v;
    rprp_flight_record_t   rp;
    common_flight_record_t c;
} flight_record_t;

typedef struct flight_record_header {
    char signature[16];
    UINT32 header_size;
    UINT32 version;
    UINT32 num_pages;
    UINT32 record_size;
    UINT32 num_records;
    UINT32 cpu_speed;
    UINT64 starting_tsc;
    UINT32 model;
    UINT32 num_bytes;
    UINT32 pad2[2];
} flight_record_header_t;

/* global due to inlining, but should only be modified by flightrecorder_ functions */
extern flight_record_t *flight_record;
extern UINT32 flightrecorder_enable_bits;

/* called once by assembly */
extern int flightrecorder_init(void);

/* called from the flightrecorder task, or perhaps someday from ccbcl */
extern int flightrecorder_open(UINT32 enable_bits, UINT32 num_records, UINT32 num_bytes, const char *pathname);
extern void flightrecorder_restart(void);
extern void flightrecorder_change(UINT32 enable_bits);
extern void flightrecorder_close(void);

#ifdef FLIGHTRECORDER

#ifndef highly_likely
#  define highly_likely(x)    __builtin_expect(!!(x), 1)
#endif

#ifndef always_inline
#   define always_inline      inline __attribute__((__always_inline__))
#endif

extern __attribute__((regparm(3))) void flightrecorder_add_vrp_data(UINT8 type, void *rp, void *data);
extern __attribute__((regparm(3))) void flightrecorder_add_vrp_nodata(UINT8 type, void *rp);
extern __attribute__((regparm(3))) void flightrecorder_add_vrp(UINT8 type, void *rp);
extern __attribute__((regparm(3))) void flightrecorder_add_rrp(UINT8 type, void *rp, UINT32 data);
extern __attribute__((regparm(3))) void flightrecorder_add_prp(UINT8 type, void *rp, UINT32 data);

static always_inline void record_mag(UINT8 type, void *vrp)
{
    if (flightrecorder_enable_bits & FR_ENABLE_MAG)
    {
        if (highly_likely(vrp))
            flightrecorder_add_vrp_nodata(type, vrp);
    }
}

static always_inline void record_cache_data(UINT8 type, void *vrp, void *data)
{
    if (flightrecorder_enable_bits & FR_ENABLE_CACHE)
    {
        if (highly_likely(vrp))
            flightrecorder_add_vrp_data(type, vrp, data);
    }
}

static always_inline void record_cache(UINT8 type, void *vrp)
{
    if (flightrecorder_enable_bits & FR_ENABLE_CACHE)
    {
        if (highly_likely(vrp))
            flightrecorder_add_vrp(type, vrp);
    }
}

static always_inline void record_virtual(UINT8 type, void *vrp)
{
    if (flightrecorder_enable_bits & FR_ENABLE_VIRTUAL)
    {
        if (highly_likely(vrp))
            flightrecorder_add_vrp(type, vrp);
    }
}

static always_inline void record_virtual_ilt(UINT8 type, ILT *ilt)
{
    if (highly_likely(ilt))
        record_virtual(type, (void *)((ilt-1)->PREILTADD()ilt_normal.w0));
}

static always_inline void record_raid(UINT8 type, void *rrp, UINT32 data)
{
    if (flightrecorder_enable_bits & FR_ENABLE_RAID)
    {
        if (highly_likely(rrp))
            flightrecorder_add_rrp(type, rrp, data);
    }
}

static always_inline void record_physical(UINT8 type, void *prp, UINT32 data)
{
    if (flightrecorder_enable_bits & FR_ENABLE_PHYSICAL)
    {
        if (highly_likely(prp))
            flightrecorder_add_prp(type, prp, data);
    }
}

static always_inline void record_physical_ilt(UINT8 type, ILT *ilt, UINT32 data)
{
    if (highly_likely(ilt))
        record_physical(type, (void *)((ilt-1)->PREILTADD()ilt_normal.w0), data);
}

static always_inline void record_pdriver(UINT8 type, void *prp, UINT32 data)
{
    if (flightrecorder_enable_bits & FR_ENABLE_PDRIVER)
    {
        if (highly_likely(prp))
            flightrecorder_add_prp(type, prp, data);
    }
}

static always_inline void record_pdriver_ilt(UINT8 type, ILT *ilt, UINT32 data)
{
    if (highly_likely(ilt))
        record_pdriver(type, (void *)((ilt-1)->PREILTADD()ilt_normal.w0), data);
}

#else

#define record_mag(type, vrp)
#define record_cache_data(type, vrp, data)
#define record_cache_nodata(type, vrp)
#define record_cache(type, vrp)
#define record_virtual(type, vrp)
#define record_virtual_ilt(type, ilt)
#define record_raid(type, rrp, data)
#define record_physical(type, prp, data)
#define record_physical_ilt(type, ilt, data)
#define record_pdriver(type, prp, data)
#define record_pdriver_ilt(type, ilt, data)

#endif

#endif  /* FLIGHTRECORDER_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
