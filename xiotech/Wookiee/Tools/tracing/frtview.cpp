#define FRTVIEW
#define FRONTEND       // for ILTNEST1 (larger than BACKEND or CCB.

#ifndef __USE_GNU
#  define __USE_GNU
#endif

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#  define __STDC_LIMIT_MACROS
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "XIO_Types.h"
#include "flightrecorder.h"       
#include "rrp.h"

#include <iostream>
#include <map>

using namespace std;

/* maybe someday the C library will have decent data structures. */
typedef uint32_t trace_index_t;
typedef uint32_t record_number_t;
typedef pair<trace_index_t, record_number_t> record_info_t;
typedef std::multimap<uint32_t, record_info_t> record_map_t;
typedef record_map_t::iterator record_map_iterator_t;
typedef pair<record_map_iterator_t, record_map_iterator_t> record_map_range_t;


typedef struct trace_file {
    char                   *pathname;
    void                   *addr;
    common_flight_record_t *records;
    common_flight_record_t *records_end;
    flight_record_header_t *header;
    int                     fd;
    uint32_t                num_bytes;
    uint64_t                starting_tsc;
    unsigned long           num_records;
    unsigned long           valid_records;
    unsigned long           invalid_records;
    record_map_t            outstanding;
    bool                    wrapped;
    trace_index_t           current;
    trace_index_t           first;
    uint64_t                first_tsc;
    trace_index_t           last;
    uint64_t                last_tsc;
} trace_file_t;

bool show_data_bytes = false;

/* private functions */

static int add_prp(char *s, rprp_flight_record_t *record, const char *layer, const char *operation)
{
    char *t = s;
    int show_blocks = 0;

    t += sprintf(t, "%5s %-16s  pid %2u ", layer, operation, record->id);
    
    /* PRPs pass the CDB opcode as the function value.  length (PRP
     * reqBytes) should always be a multiple of the block size for the
     * SBC commands handled specially, but check anyway.
     */
    switch (record->function)
    {
        case 0x28:
        case 0x88:
            t += sprintf(t, "  read");
            if ((record->length % 512) == 0)
                show_blocks = 1;
            break;
        case 0x2E:
        case 0x8E:
            t += sprintf(t, "verify");
            if ((record->length % 512) == 0)
                show_blocks = 1;
            break;
        case 0x2A:
        case 0x8A:
            t += sprintf(t, " write");
            if ((record->length % 512) == 0)
                show_blocks = 1;
            break;
        case 0x41:
        case 0x93:
            t += sprintf(t, "wrsame");
            if ((record->length % 512) == 0)
                show_blocks = 1;
            break;
        case 0x2F:
        case 0x8F:
            t += sprintf(t, "wr/vfy");
            if ((record->length % 512) == 0)
                show_blocks = 1;
            break;
        default:
            t += sprintf(t, "   cdb");
            break;
    }

    t += sprintf(t, " 0x%02x  sda %12" PRIu64, record->function, record->sda);
    if (show_blocks)
        t += sprintf(t, "  blocks %5u  ", record->length / 512);
    else
        t += sprintf(t, "  bytes %6u  ", record->length);

    return (t - s);
}

static int add_rrp(char *s, rprp_flight_record_t *record, const char *layer, const char *operation)
{
    char *t = s;

    t += sprintf(t, "%5s %-16s  rid %2u ", layer, operation, record->id);

    switch (record->function)
    {
        case RRP_INPUT:
            t += sprintf(t, "  read");
            break;
        case RRP_OUTPUT:
            t += sprintf(t, " write");
            break;
        case RRP_VERIFY_DATA:
            t += sprintf(t, "verify");
            break;
        case RRP_OUTPUT_VERIFY:
            t += sprintf(t, "wr/vfy");
            break;
        default:
            t += sprintf(t, "  func");
            break;
    }

    t += sprintf(t, " 0x%02x  sda %12" PRIu64 "  blocks %5u  ",
                 record->function, record->sda, record->length);

    return (t - s);
}

static int add_vrp(char *s, vrp_flight_record_t *record, const char *layer, const char *operation)
{
    char *t = s;

    t += sprintf(t, "%5s %-16s  vid %2u ", layer, operation, record->id);

    switch (record->function)
    {
        case VRP_INPUT:
            t += sprintf(t, "  read");
            break;
        case VRP_OUTPUT:
            t += sprintf(t, " write");
            break;
        case VRP_VERIFY:
            t += sprintf(t, "verify");
            break;
        case VRP_OUTPUT_VERIFY:
            t += sprintf(t, "wr/vfy");
            break;
        default:
            t += sprintf(t, "  func");
            break;
    }

    t += sprintf(t, " 0x%02x  sda %12" PRIu64 "  blocks %5u  ",
                 record->function, record->sda, record->length);

    return (t - s);
}

static inline common_flight_record_t *get_record(trace_file_t *trace, unsigned long index)
{
    common_flight_record_t *r;

    r = (common_flight_record_t *)((uint8_t *)trace->records + 
                                   (index * (sizeof(*r) + trace->num_bytes)));
    return r;
}


static int add_delta(trace_file_t *trace, char *s, common_flight_record_t *record, const char *str, bool erase, 
                     uint8_t type1, uint8_t type2 = FR_UNUSED)
{
    record_map_iterator_t p;
    record_map_range_t range;
    common_flight_record_t *r;
    char *t = s;
    uint64_t delta;
    double msec;
    
    range = trace->outstanding.equal_range((uint32_t)record->addr);
    for (p = range.first; p != range.second; )
    {
        r = get_record(trace, p->second.first);
        if (r && ((r->type == type1) || (r->type == type2)))
        {
            delta = record->tsc - r->tsc;
            t += sprintf(t, "  %16s %8u  delta %12" PRIu64, 
                         str, p->second.second, delta);

            /* CPU speed is in MHz = 1000 * 1000 ticks per second */
            msec = (double)delta / ((double)trace->header->cpu_speed * 1000);
            t += sprintf(t, "   msec %12.6f", msec);

            if (erase)
                trace->outstanding.erase(p++);
            else
                ++p;
        }
        else
            ++p;
    }

    return (t - s);
}

/* if the trace has a valid record that hasn't been processed yet,
 * return it, otherwise return NULL.  skip over unused records and
 * logically discarded records.
 */
static common_flight_record_t *current_record(trace_file_t *trace)
{
    common_flight_record_t *record = get_record(trace, trace->current);

    if (trace->valid_records == 0)
        return NULL;

    if ((trace->first > trace->last) && (trace->current >= trace->first) && !trace->wrapped)
    {
        /* need to handle wrap-around at num_records */
        while (trace->current < trace->num_records)
        {
            if ((record->type != FR_UNUSED) && (record->tsc >= trace->starting_tsc))
                return record;

            record++;
            trace->current++;
        }

        trace->wrapped = true;
        trace->current = 0;
        record = (common_flight_record_t *)trace->records;
    }

    while (trace->current <= trace->last)
    {
        if ((record->type != FR_UNUSED) && (record->tsc >= trace->starting_tsc))
            return record;
        
        record++;
        trace->current++;
    }

    return NULL;
}


/* process one record from the trace, and remember that we've processed it.
 * returns the number of lines printed out.
 */
static record_number_t process_record(trace_file_t *trace, record_number_t number)
{
    common_flight_record_t *crecord = current_record(trace);
    rprp_flight_record_t *rprecord = (rprp_flight_record_t *)crecord;
    vrp_flight_record_t *vrecord = (vrp_flight_record_t *)crecord;
    trace_index_t index = trace->current++;
    char *s;
    char text[2048];
    char temp[64];
    
    if (crecord->type == FR_UNUSED)
        return 0;
    
    s = text;
    s += sprintf(s, "%8u:  %08x  ", number, crecord->addr);

    switch (crecord->type)
    {
        case FR_PRP_QUEUED_AT_HEAD:
            s += add_prp(s, rprecord, "PHYS", "queue@head");
            if (trace->header->model == 750)
                sprintf(temp, "newio_depth %3u", rprecord->data);
            else
                sprintf(temp, " ");
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(rprecord->addr, make_pair(index, number)));
            break;
        case FR_PRP_QUEUED_AT_TAIL:
            s += add_prp(s, rprecord, "PHYS", "queue@tail");
            if (trace->header->model == 750)
                sprintf(temp, "newio_depth %3u", rprecord->data);
            else
                sprintf(temp, " ");
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(rprecord->addr, make_pair(index, number)));
            break;
        case FR_PRP_MIRROR_DISCARD:
            s += add_prp(s, rprecord, "PHYS", "discard");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", rprecord->tsc);
            /* show delta from queue */
            s += add_delta(trace, s, crecord, "queue", true, FR_PRP_QUEUED_AT_HEAD, FR_PRP_QUEUED_AT_TAIL);
            break;
        case FR_PRP_COMPLETE:
            s += add_prp(s, rprecord, "PHYS", "complete");
            sprintf(temp, "PHYS_reqStatus %3u", rprecord->data);
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            /* show delta from queue */
            s += add_delta(trace, s, crecord, "queue", true, FR_PRP_QUEUED_AT_HEAD, FR_PRP_QUEUED_AT_TAIL);
            break;
        case FR_PRP_JOINED:
            s += add_prp(s, rprecord, "PHYS", "joined");
            sprintf(temp, "                  ");
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            /* show delta from queue */
            s += add_delta(trace, s, crecord, "queue", true, FR_PRP_QUEUED_AT_HEAD, FR_PRP_QUEUED_AT_TAIL);
            break;
        case FR_PRP_CANCELED:
            s += add_prp(s, rprecord, "PHYS", "canceled");
            sprintf(temp, "                  ");
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            /* show delta from queue */
            s += add_delta(trace, s, crecord, "queue", true, FR_PRP_QUEUED_AT_HEAD, FR_PRP_QUEUED_AT_TAIL);
            break;
        case FR_PRP_RETRY_DECIDED:
            s += add_prp(s, rprecord, "PHYS", "error@tail");
            if (trace->header->model == 750)
                sprintf(temp, "errio_depth %3u", rprecord->data);
            else
                sprintf(temp, "                  ");
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            /* show delta from queue */
            s += add_delta(trace, s, crecord, "queue", false, FR_PRP_QUEUED_AT_HEAD, FR_PRP_QUEUED_AT_TAIL);
            break;
        case FR_PRP_RETRY_QUEUED:
            s += add_prp(s, rprecord, "PHYS", "retry@head");
            if (trace->header->model == 750)
                sprintf(temp, "newio_depth %3u", rprecord->data);
            else
                sprintf(temp, "                  ");
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            /* show delta from retry decided */
            s += add_delta(trace, s, crecord, "error", true, FR_PRP_RETRY_DECIDED);
            break;
        case FR_PRP_FAILED:
            s += add_prp(s, rprecord, "PHYS", "failed");
            sprintf(temp, "PHYS_reqStatus %3u", rprecord->data);
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            /* show delta from queue */
            s += add_delta(trace, s, crecord, "queue", true, FR_PRP_QUEUED_AT_HEAD, FR_PRP_QUEUED_AT_TAIL);
            break;
        case FR_PDRIVER_ISSUED:
            s += add_prp(s, rprecord, "PDRV", (trace->header->model == 750) ? "sent-sg" : "sent-isp");
            if (trace->header->model == 750)
                sprintf(temp, "sg_depth %3u", rprecord->data);
            else
                sprintf(temp, "                  ");
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(rprecord->addr, make_pair(index, number)));
            /* show delta from queue */
            s += add_delta(trace, s, crecord, "queue", false, FR_PRP_QUEUED_AT_HEAD, FR_PRP_QUEUED_AT_TAIL);
            break;
        case FR_PDRIVER_COMPLETE:
            s += add_prp(s, rprecord, "PDRV", (trace->header->model == 750) ? "sg-done" : "isp-done");
            if (trace->header->model == 750)
                sprintf(temp, "sg_status 0x%08x", rprecord->data);
            else
                sprintf(temp, "                  ");
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            /* show delta from command issue */
            s += add_delta(trace, s, crecord, 
                           (trace->header->model == 750) ? "sent-sg" : "sent-isp", 
                           true, FR_PDRIVER_ISSUED);
            break;
        case FR_RRP_EXEC:
            s += add_rrp(s, rprecord, "RAID", "execute");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", rprecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(rprecord->addr, make_pair(index, number)));
            break;
        case FR_RRP_COMPLETE:
            s += add_rrp(s, rprecord, "RAID", "complete");
            sprintf(temp, "RAID_reqStatus %3u", rprecord->data);
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, rprecord->tsc);
            /* show delta from execute */
            s += add_delta(trace, s, crecord, "execute", true, FR_RRP_EXEC);
            break;
        case FR_CACHE_NC_READ:
            s += add_vrp(s, vrecord, "CACHE", "nc-read");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            /* show delta from exec */
            s += add_delta(trace, s, crecord, "exec", false, FR_CACHE_EXEC);
            break;
        case FR_CACHE_NC_READ_COMPLETE:
            s += add_vrp(s, vrecord, "CACHE", "nc-read-comp");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from read */
            s += add_delta(trace, s, crecord, "nc-read", true, FR_CACHE_NC_READ);
            break;
        case FR_CACHE_NC_WRITE_SGL:
            s += add_vrp(s, vrecord, "CACHE", "nc-write-sgl");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            /* show delta from exec */
            s += add_delta(trace, s, crecord, "exec", false, FR_CACHE_EXEC);
            break;
        case FR_CACHE_NC_WRITE_SGL_COMPLETE:
            s += add_vrp(s, vrecord, "CACHE", "ncwrite-sgl-comp");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from write */
            s += add_delta(trace, s, crecord, "nc-write-sgl", true, FR_CACHE_NC_WRITE_SGL);
            break;
        case FR_CACHE_NC_WRITE_PROXY:
            s += add_vrp(s, vrecord, "CACHE", "nc-write-proxy");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            /* show delta from exec */
            s += add_delta(trace, s, crecord, "exec", false, FR_CACHE_EXEC);
            break;
        case FR_CACHE_NC_WRITE_PROXY_DATA:
            s += add_vrp(s, vrecord, "CACHE", "ncw-proxy-data");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from write */
            s += add_delta(trace, s, crecord, "ncw-proxy", false, FR_CACHE_NC_WRITE_PROXY);
            break;
        case FR_CACHE_NC_WRITE_PROXY_COMPLETE:
            s += add_vrp(s, vrecord, "CACHE", "ncw-proxy-comp");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from write */
            s += add_delta(trace, s, crecord, "nc-write-proxy", true, FR_CACHE_NC_WRITE_PROXY);
            break;
        case FR_CACHE_READ_HIT:
            s += add_vrp(s, vrecord, "CACHE", "read-hit");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from exec */
            s += add_delta(trace, s, crecord, "exec", false, FR_CACHE_EXEC);
            break;
        case FR_CACHE_WRITE_HIT_RESIDENT:
            s += add_vrp(s, vrecord, "CACHE", "write-resident");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            /* show delta from exec */
            s += add_delta(trace, s, crecord, "exec", false, FR_CACHE_EXEC);
            break;
        case FR_CACHE_WRITE_RESIDENT_DATA:
            s += add_vrp(s, vrecord, "CACHE", "resident-data");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from hit */
            s += add_delta(trace, s, crecord, "write-resident", true, FR_CACHE_WRITE_HIT_RESIDENT);
            break;
        case FR_CACHE_WRITE_HIT_DIRTY:
            s += add_vrp(s, vrecord, "CACHE", "write-dirty");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            break;
        case FR_CACHE_WRITE_DIRTY_DATA:
            s += add_vrp(s, vrecord, "CACHE", "write-dirty-data");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from hit */
            s += add_delta(trace, s, crecord, "write-dirty", true, FR_CACHE_WRITE_HIT_DIRTY);
            break;
        case FR_CACHE_WRITE_CACHEABLE:
            s += add_vrp(s, vrecord, "CACHE", "write");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            /* show delta from exec */
            s += add_delta(trace, s, crecord, "exec", false, FR_CACHE_EXEC);
            break;
        case FR_CACHE_WRITE_DATA:
            s += add_vrp(s, vrecord, "CACHE", "write-data");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from write */
            s += add_delta(trace, s, crecord, "write", true, FR_CACHE_WRITE_CACHEABLE);
            break;
        case FR_CACHE_FLUSH:
            s += add_vrp(s, vrecord, "CACHE", "flush");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            break;
        case FR_CACHE_FLUSH_COMPLETE:
            s += add_vrp(s, vrecord, "CACHE", "flush-complete");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from flush */
            s += add_delta(trace, s, crecord, "flush", true, FR_CACHE_FLUSH);
            break;
        case FR_CACHE_BACKGROUND_FLUSH:
            s += add_vrp(s, vrecord, "CACHE", "backgrnd-flush");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            break;
        case FR_CACHE_BACKGROUND_FLUSH_COMPLETE:
            s += add_vrp(s, vrecord, "CACHE", "bgrnd-flush-comp");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from background flush */
            s += add_delta(trace, s, crecord, "backgrnd-flush", true, FR_CACHE_BACKGROUND_FLUSH);
            break;
        case FR_CACHE_EXEC:
            s += add_vrp(s, vrecord, "CACHE", "exec");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            break;
        case FR_CACHE_IOEXEC:
            s += add_vrp(s, vrecord, "CACHE", "ioexec");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from exec */
            s += add_delta(trace, s, crecord, "exec", false, FR_CACHE_EXEC);
            break;
        case FR_CACHE_EXEC_COMPLETE:
            s += add_vrp(s, vrecord, "CACHE", "exec-complete");
            sprintf(temp, "vrp_status %3u", vrecord->status);
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, vrecord->tsc);
            /* show delta from exec */
            s += add_delta(trace, s, crecord, "exec", true, FR_CACHE_EXEC);
            break;
        case FR_VRP_EXEC:
            s += add_vrp(s, vrecord, "VIRT", "execute");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            break;
        case FR_VRP_COMPLETE:
            s += add_vrp(s, vrecord, "VIRT", "complete");
            sprintf(temp, "vrp_status %3u", vrecord->status);
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, vrecord->tsc);
            /* show delta from exec */
            s += add_delta(trace, s, crecord, "execute", true, FR_VRP_EXEC);
            break;
        case FR_VRP_SCOMPLETE:
            s += add_vrp(s, vrecord, "VIRT", "scomplete");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            /* show delta from exec */
            s += add_delta(trace, s, crecord, "execute", true, FR_VRP_EXEC);
            break;
        case FR_MAG_WRITE:
            s += add_vrp(s, vrecord, "MAG", "write");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            break;
        case FR_MAG_READ:
            s += add_vrp(s, vrecord, "MAG", "read");
            s += sprintf(s, "%20s  tsc %" PRIu64, " ", vrecord->tsc);
            trace->outstanding.insert(record_map_t::value_type(vrecord->addr, make_pair(index, number)));
            break;
        case FR_MAG_READ_COMPLETE:
            s += add_vrp(s, vrecord, "MAG", "read-complete");
            sprintf(temp, "vrp_status %3u", vrecord->status);
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, vrecord->tsc);
            /* show delta from read */
            s += add_delta(trace, s, crecord, "read", true, FR_MAG_READ);
            break;
        case FR_MAG_WRITE_COMPLETE:
            s += add_vrp(s, vrecord, "MAG", "write-complete");
            sprintf(temp, "vrp_status %3u", vrecord->status);
            s += sprintf(s, "%20s  tsc %" PRIu64, temp, vrecord->tsc);
            /* show delta from write */
            s += add_delta(trace, s, crecord, "write", true, FR_MAG_WRITE);
            break;
        default:
            /* FIXME: print whole record + bytes in hex, since this shows up more often
             * than I would expect.
             */
            fprintf(stderr, "%s record index %lu unexpected type %u\n", 
                    trace->pathname, index, crecord->type);
            return 0;
    }

    printf("%s\n", text);
    if (trace->num_bytes)
    {
        switch (crecord->type)
        {
            case FR_MAG_READ:
            case FR_MAG_WRITE:
            case FR_MAG_READ_COMPLETE:
            case FR_MAG_WRITE_COMPLETE:
            case FR_CACHE_EXEC:
            case FR_CACHE_IOEXEC:
            case FR_CACHE_EXEC_COMPLETE:
                printf("\n");
                break;
            default:
            {
                uint32_t bytes = trace->num_bytes;
                UINT8 *buffer = crecord->bytes;
                
                s = text;
                s += sprintf(s, "                      hex = ");
                while (bytes--)
                {
                    s += sprintf(s, "%02x ", *buffer++);
                }
                
                bytes = trace->num_bytes;
                buffer = crecord->bytes;
                s += sprintf(s, " ascii = ");
                while (bytes--)
                {
                    if (isprint(*buffer))
                        s += sprintf(s, "%c", *buffer);
                    else
                        s += sprintf(s, ".");
                    buffer++;
                }
        
                printf("%s\n\n", text);
            }
        }
    }
    else if (show_data_bytes)
    {
        /* one of the other trace files has data bytes, so for
         * consistency print a blank line for this trace.
         */
        printf("\n");
    }
    return 1;
}

static int open_trace_file(const char *pathname, trace_file_t *trace)
{
    struct stat s;

    trace->pathname = strdup(pathname);
    trace->fd = open(pathname, O_RDWR);
    if (trace->fd == -1)
    {
        fprintf(stderr, "flightrecorder open of trace %s failed, error %d %s\n", 
                pathname, errno, strerror(errno));
        exit(1);
    }

    memset(&s, 0x0, sizeof(s));
    if (fstat(trace->fd, &s) < 0)
    {
        fprintf(stderr, "couldn't stat %s, error %d %s \n", pathname, errno, strerror(errno));
        exit(1);
    }

    trace->addr = mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, trace->fd, 0);
    if (trace->addr == MAP_FAILED)
    {
        fprintf(stderr, "mmap of trace %s failed, error %d %s\n", 
                pathname, errno, strerror(errno));
        exit(1);
    }

    /* well, more-or-less sequential, since the deltas look backward */
    madvise(trace->addr, s.st_size, MADV_SEQUENTIAL);

    trace->header = (flight_record_header_t *)trace->addr;
    if (strcmp(trace->header->signature, "flightrecorder\n") != 0)
    {
        fprintf(stderr, "trace %s missing signature, wrong file?\n", pathname);
        exit(1);
    }

    if (trace->header->header_size != sizeof(flight_record_header_t))
    {
        fprintf(stderr, "expected header size %u, file header size %u for %s\n",
                sizeof(flight_record_header), trace->header->header_size, pathname);
        exit(1);
    }

    switch (trace->header->version)
    {
        case 0x01:
        case 0x02:
        case 0x03:
            /* we can read these versions correctly. You'll have to update this
             * whenever the version number in flightrecord.h changes.  Be sure
             * to remove the old versions if you change the reader in a way
             * that would prevent it from reading them.
             */
            break;
        default:
            fprintf(stderr, "expected version %u, file version %u for %s\n",
                    FLIGHTRECORDER_VERSION, trace->header->version, pathname);
            exit(1);
    }

    if (trace->header->record_size != sizeof(flight_record_t))
    {
        fprintf(stderr, "expected record size %u, file record size %u for %s\n",
                sizeof(flight_record_header), trace->header->header_size, pathname);
        exit(1);
    }

    /* in case we're mmapping a trace still being written to, copy the
     * header fields that are important to us.
     */
    trace->starting_tsc = trace->header->starting_tsc;
    trace->num_records = trace->header->num_records;
    trace->num_bytes = trace->header->num_bytes;

    if (trace->num_bytes)
        show_data_bytes = true;

    trace->records = (common_flight_record_t *)((uint8_t *)trace->addr + trace->header->header_size);
    trace->records_end = (common_flight_record_t *)((uint8_t *)trace->addr + trace->header->header_size +
                                                    (trace->num_records * (sizeof(flight_record_t) + 
                                                                           trace->num_bytes)));

    return 0;
}

static int scan_trace(trace_file_t *trace)
{
    unsigned long n = 0;
    common_flight_record_t *record = trace->records;

    trace->first = 0;
    trace->last = 0;
    trace->first_tsc = UINT64_MAX;
    trace->last_tsc = 0x0;
    trace->valid_records = 0;
    trace->invalid_records = 0;

    while (record < trace->records_end)
    {
        if (record->type == FR_UNUSED)
        {
            trace->invalid_records++;
        }
        else if (trace->starting_tsc && (record->tsc < trace->starting_tsc))
        {
            /* ignore logically discarded records */
            trace->invalid_records++;
        }
        else
        {
            trace->valid_records++;
            if (record->tsc < trace->first_tsc)
            {
                trace->first = n;
                trace->first_tsc = record->tsc;
            }
            if (record->tsc > trace->last_tsc)
            {
                trace->last = n;
                trace->last_tsc = record->tsc;
            }
        }

        n++;
        record = (common_flight_record_t *)((uint8_t *)record + sizeof(*record) + trace->num_bytes);
    }

    trace->current = trace->first;
    trace->wrapped = false;
    
    if (trace->valid_records)
    {
        if ((trace->starting_tsc != trace->header->starting_tsc) ||
            (get_record(trace, trace->first)->tsc != trace->first_tsc) ||
            (get_record(trace, trace->last)->tsc != trace->last_tsc))
        {
            /*
             * The delta calculations go insane if records are
             * overwritten after being processed, because we lose
             * chronological ordering and lookups of outstanding
             * requests won't work properly.  This check will only
             * catch the most obvious of overwrites, but it might at
             * least provoke people into eventually reading the README
             * file.
             */
            fprintf(stderr, "Viewing files being written is not supported!\n");
            fprintf(stderr, "Make a copy of the trace file and view the copy instead.\n");
            exit(1);
        }
    }
    
    printf("# trace %s: version %u, num_records %lu, record_size %u, num_bytes %u\n",
           trace->pathname, trace->header->version, 
           trace->num_records, trace->header->record_size, trace->num_bytes);

    printf("# trace %s: cpu speed %u MHz, model %u, starting tsc %" PRIu64 "\n",
           trace->pathname, trace->header->cpu_speed, trace->header->model, trace->starting_tsc);
    
    printf("# trace %s: %lu valid records, %lu invalid records, tsc range covers %8.6f seconds\n",
           trace->pathname, trace->valid_records, trace->invalid_records,
           (double)(trace->last_tsc - trace->first_tsc) / 
                    ((double)trace->header->cpu_speed * 1000.0 * 1000.0));
    
    printf("# trace %s: first record at index %u tsc %" PRIu64 ", last index %u tsc %" PRIu64 "\n", 
           trace->pathname, trace->first, trace->first_tsc, trace->last, trace->last_tsc);

    printf("#\n");

    return 0;
}

int main(int argc, char *argv[])
{
    record_number_t  number     = 0;
    trace_file_t    *traces;
    int              t;
    int              num_traces = argc - 1;
    uint64_t         tsc        = UINT64_MAX;
    trace_file_t    *trace      = NULL;

    if (num_traces < 1)
    {
        fprintf(stderr, "no trace pathnames specified\n");
        exit(1);
    }

    traces = new trace_file_t[num_traces];
    if (traces == NULL)
    {
        fprintf(stderr, "couldn't allocate %d trace files\n", num_traces);
        exit(1);
    }
    
    /* open all the trace files */
    for (t = 0; t < num_traces; t++)
    {
        if (open_trace_file(argv[t+1], &traces[t]) < 0)
            exit(1);
    }

    /* scan all the trace files to find the first and last record in each */
    for (t = 0; t < num_traces; t++)
    {
        if (scan_trace(&traces[t]) < 0)
            exit(1);
    }
    
    /* print out records, sorting by chronological order, and break
     * tsc ties between traces by order of the trace file on the
     * command line.
     */
    number = 1;
    do 
    {
        trace = NULL;
        tsc = UINT64_MAX;

        for (t = 0; t < num_traces; t++)
        {
            common_flight_record_t *record = current_record(&traces[t]);
            if (record != NULL)
            {
                if ((trace == NULL) || (record->tsc < tsc))
                {
                    tsc = record->tsc;
                    trace = &traces[t];
                }
            }
        }
    
        if (trace)
            number += process_record(trace, number);

    } while (trace != NULL);

    return 0;
}
