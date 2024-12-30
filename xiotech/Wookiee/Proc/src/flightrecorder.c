/* $Id: flightrecorder.c 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       flightrecorder.c
**
**  @brief      FlightRecorder debugging tool
**
**  Copyright 2006-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "CT_defines.h"
#include "L_Signal.h"
#include "flightrecorder.h"
#include "vrp.h"
#include "rrp.h"
#include "prp.h"
#include "dev.h"
#include "sgl.h"
#include "pdd.h"

/* global variables */
flight_record_t *flight_record = NULL;

/* bits controlling what type of records to save, FR_ENABLE_* */
UINT32 flightrecorder_enable_bits = 0;


/* private macros */
#define get_tsc()       ({ unsigned long long __scr; \
                           __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})

#define compiler_barrier()   __asm__ __volatile__("" ::: "memory")

#define FR_NO_REQUEST       0
#define FR_REQUEST_START    SIGRTMAX-3
#define FR_REQUEST_STOP    (SIGRTMAX-2)
#define FR_REQUEST_RESTART (SIGRTMAX-1)
#define FR_REQUEST_ROTATE  (SIGRTMAX)

#ifdef FLIGHTRECORDER
extern void CT_LC_flightrecorder_task(int);

/* private variables */
static flight_record_t        *records;
static flight_record_t        *records_end;
static flight_record_header_t *header;
static int                     fd          = -1;
static UINT32                  num_records = FLIGHTRECORDER_DEFAULT_RECORDS;
static UINT32                  num_bytes   = 0;
static char                   *pathname    = NULL;
static sig_atomic_t            request     = 0;
static PCB                    *pcb         = NULL;

static void flightrecorder_sighandler(INT32 signo, UINT32 data UNUSED, siginfo_t *info UNUSED, UINT32 unused UNUSED)
{
    request = signo;
    if (pcb && (TaskGetState(pcb) == PCB_NOT_READY))
    {
#ifdef HISTORY_KEEP
CT_history_pcb("flightrecorder_sighandler setting ready pcb", (UINT32)pcb);
#endif
        TaskSetState(pcb, PCB_READY);
    }
}

static int load_config(UINT32 *pbits, UINT32 *precords, UINT32 *pbytes, char **ppath)
{
    FILE *f;
    char *b;
    char *l;
    UINT32 config_bits = 0;
    UINT32 shared_bytes = 0;
    UINT32 shared_records = FLIGHTRECORDER_DEFAULT_RECORDS;
    UINT32 specific_bytes = 0;
    UINT32 specific_records = 0;
    char *config_path = NULL;
    char buffer[512];
    char line[512];

    if (fd >= 0)
        return 0;

    if ((f = fopen("/opt/xiotech/procdata/flightrecorder.cfg", "r")))
    {
        while (fgets(buffer, sizeof(buffer) - 1, f))
        {
            b = buffer;
            l = line;

            /* strip whitespace and comments */
            while (*b && (*b != '\n') && (*b != '#'))
            {
                if (!isspace(*b))
                    *l++ = *b;
                b++;
            }
            *l = '\0';
            l = line;

            /* skip empty lines */
            if (*l == '\0')
                continue;

            if (strncasecmp(l, "records=", 8) == 0)
            {
                shared_records = strtoul(l + 8, NULL, 0);
            }
            else if (strncasecmp(l, "bytes=", 6) == 0)
            {
                shared_bytes = strtoul(l + 6, NULL, 0);
            }
#  ifdef FRONTEND
            else if (strncasecmp(l, "frontend_records=", 17) == 0)
            {
                specific_records = strtoul(l + 17, NULL, 0);
            }
            else if (strncasecmp(l, "frontend_bytes=", 15) == 0)
            {
                specific_bytes = strtoul(l + 15, NULL, 0);
            }
            else if (strncasecmp(l, "frontend_file=", 14) == 0)
            {
                config_path = strdup(l + 14);
            }
#  else
            else if (strncasecmp(l, "backend_records=", 16) == 0)
            {
                specific_records = strtoul(l + 16, NULL, 0);
            }
            else if (strncasecmp(l, "backend_bytes=", 14) == 0)
            {
                specific_bytes = strtoul(l + 14, NULL, 0);
            }
            else if (strncasecmp(l, "backend_file=", 13) == 0)
            {
                config_path = strdup(l + 13);
            }
#  endif
            else if (strncasecmp(l, "enable=", 7) == 0)
            {
                config_bits = strtoul(l + 7, NULL, 0);
            }
        }

        if (pbits)
            *pbits = config_bits;
        if (ppath)
            *ppath = config_path;
        if (precords)
            *precords = (specific_records ? specific_records : shared_records);
        if (pbytes)
            *pbytes = (specific_bytes ? specific_bytes : shared_bytes);

        fclose(f);

        return 1;
    }

    return 0;
}

static void flightrecorder_rotate_file(void)
{
    UINT32  config_bits;
    UINT32  config_records;
    UINT32  config_bytes;
    char   *config_path = NULL;
    char backup[512];

    flightrecorder_close();
    sprintf(backup, "%s.old", pathname);
    unlink(backup);
    rename(pathname, backup);
    if (load_config(&config_bits, &config_records, &config_bytes, &config_path))
        flightrecorder_open(config_bits, config_records, config_bytes, config_path);
    else
        flightrecorder_open(flightrecorder_enable_bits, num_records, num_bytes, pathname);

    if (config_path)
        free(config_path);
}


NORETURN void flightrecorder_task(void *unused UNUSED);
NORETURN
void flightrecorder_task(void *unused UNUSED)
{
    for (;;)
    {
        int     the_request = request;
        UINT32  config_bits;
        UINT32  config_records;
        UINT32  config_bytes;
        char   *config_path = NULL;

        TaskSetState(pcb, PCB_NOT_READY);
        request = FR_NO_REQUEST;
        compiler_barrier();

        if (the_request == FR_REQUEST_START)
        {
            if (fd < 0)
            {
                fprintf(stderr, "flightrecorder start\n");

                if (load_config(&config_bits, &config_records, &config_bytes, &config_path))
                    flightrecorder_open(config_bits, config_records, config_bytes, config_path);
                else
                    flightrecorder_open(0xFFFFFFFF, FLIGHTRECORDER_DEFAULT_RECORDS, 0, NULL);

                if (config_path)
                    free(config_path);
            }
        }
        else if (the_request == FR_REQUEST_STOP)
        {
            if (fd >= 0)
            {
                fprintf(stderr, "flightrecorder stop %s\n", pathname);
                flightrecorder_close();
            }
        }
        else if (the_request == FR_REQUEST_RESTART)
        {
            if (fd >= 0)
            {
                fprintf(stderr, "flightrecorder restart %s\n", pathname);
                flightrecorder_restart();
            }
        }
        else if (the_request == FR_REQUEST_ROTATE)
        {
            if (fd >= 0)
            {
                fprintf(stderr, "flightrecorder rotate %s\n", pathname);
                flightrecorder_rotate_file();
            }
        }

        TaskSwitch();
    }
}

#endif

int flightrecorder_init(void)
{
#ifdef FLIGHTRECORDER
    UINT32  config_bits    = 0;
    UINT32  config_records = FLIGHTRECORDER_DEFAULT_RECORDS;
    UINT32  config_bytes   = 0;
    char   *config_path    = NULL;

    if (fd >= 0)
        return -1;

    load_config(&config_bits, &config_records, &config_bytes, &config_path);

#  ifdef FRONTEND
    if (config_bits & (FR_ENABLE_MAG|FR_ENABLE_CACHE))
    {
        /* start the frontend flightrecorder */
        flightrecorder_open(config_bits, config_records, config_bytes, config_path);
    }
#  else
    if (config_bits & (FR_ENABLE_VIRTUAL|FR_ENABLE_RAID|FR_ENABLE_PHYSICAL|FR_ENABLE_PDRIVER))
    {
        /* start the backend flightrecorder */
        flightrecorder_open(config_bits, config_records, config_bytes, config_path);
    }
#  endif

    if (config_path)
        free(config_path);

    /* create the task regardless, since the recorder may be started later
     * with a signal.
     */
    if (pcb == NULL)
    {
        CT_fork_tmp = (UINT32)"flightrecorder";
        pcb = TaskCreatePerm2(C_label_referenced_in_i960asm(flightrecorder_task), 50);
        /* set signal handlers for dynamic requests */
        L_SignalHandlerAdd(FR_REQUEST_START,  flightrecorder_sighandler, 0);
        L_SignalHandlerAdd(FR_REQUEST_STOP, flightrecorder_sighandler, 0);
        L_SignalHandlerAdd(FR_REQUEST_RESTART, flightrecorder_sighandler, 0);
        L_SignalHandlerAdd(FR_REQUEST_ROTATE,  flightrecorder_sighandler, 0);
    }

    if (pathname)
        fprintf(stderr, "flightrecorder initialized and started %s\n", pathname);
    else
        fprintf(stderr, "flightrecorder initialized\n");

    return 0;
#else
    fprintf(stderr, "flightrecorder not supported\n");
    return -1;
#endif
}

#ifdef FLIGHTRECORDER
int flightrecorder_open(UINT32 enable_bits, UINT32 n_records, UINT32 n_bytes, const char *path)
{
    int flags;
    UINT32 num_pages;
    UINT32 page_size = getpagesize();
    void *addr;

    /* try to keep flight records aligned reasonably, since unaligned accesses are slow */
    if (n_bytes % 8)
    {
        fprintf(stderr, "flightrecorder error opening %s, sgl bytes %u must be a multiple of 8\n",
                path, n_bytes);
        return -1;
    }

    if (fd >= 0)
    {
        fprintf(stderr, "reopening flightrecorder\n");
        flightrecorder_close();
    }

    if ((path == NULL) || (strcmp(path, "1") == 0))
#  ifdef FRONTEND
        pathname = strdup("/tmp/frontend.frt");
#  else
        pathname = strdup("/tmp/backend.frt");
#  endif
    else
        pathname = strdup(path);

    unlink(pathname);
    fd = open(pathname, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if (fd == -1)
    {
        fprintf(stderr, "flightrecorder open of log %s failed, error %d %s\n",
                pathname, errno, strerror(errno));
        free(pathname);
        pathname = NULL;
        return -1;
    }

    num_pages = ((n_records * (sizeof(flight_record_t) + n_bytes)) +
                 sizeof(flight_record_header_t) + page_size - 1) / page_size;

    n_records = ((num_pages * page_size) - sizeof(flight_record_header_t))
                                / (sizeof(flight_record_t) + n_bytes);

    if (ftruncate(fd, num_pages * page_size) < 0)
    {
        fprintf(stderr, "flightrecorder ftruncate %s to %u pages failed, error %d %s\n",
                pathname, num_pages, errno, strerror(errno));
    }

    /* the 30MB page lock cutoff is somewhat arbitrarily chosen */
    if ((num_pages * page_size) < (30 * 1024 * 1024))
        flags = MAP_SHARED|MAP_LOCKED;
    else
        flags = MAP_SHARED;

    addr = mmap(NULL, num_pages * page_size, PROT_READ|PROT_WRITE, flags, fd, 0);
    if (addr == MAP_FAILED)
    {
        fprintf(stderr, "flightrecorder mmap of log %s failed, error %d %s\n",
                pathname, errno, strerror(errno));
        free(pathname);
        pathname = NULL;
        close(fd);
        fd = -1;
        return -1;
    }

    madvise(addr, num_pages * page_size, MADV_SEQUENTIAL);

    num_records = n_records;
    num_bytes = n_bytes;

    header = addr;
    memset(header, 0x00, sizeof(*header));
    snprintf(header->signature, sizeof(header->signature), "flightrecorder\n");
    header->header_size = sizeof(*header);
    header->version = FLIGHTRECORDER_VERSION;
    header->num_pages = num_pages;
    header->record_size = sizeof(*flight_record);
    header->num_records = num_records;
    header->num_bytes = num_bytes;
    header->starting_tsc = get_tsc();

    if (getenv("CPU_SPEED"))
    {
        header->cpu_speed = (UINT32)strtoul(getenv("CPU_SPEED"), NULL, 0);
    }
    else
    {
        header->cpu_speed = 3200;
    }

    header->model = 3000;

    flight_record = records = (flight_record_t *)((UINT8 *)addr + sizeof(*header));
    records_end = (flight_record_t *)((UINT8 *)addr + sizeof(*header) +
                                      (num_records * (sizeof(*records) + num_bytes)));

    compiler_barrier();
    flightrecorder_enable_bits = enable_bits;

    fprintf(stderr, "flightrecorder opened %s, %u pages, %u records, num_bytes %u, "
            "header %p start %p end %p, enable bits 0x%08x\n",
            pathname, num_pages, num_records, num_bytes, header, records, records_end, enable_bits);

    return 0;
}
#else
int flightrecorder_open(UINT32 enable_bits UNUSED, UINT32 n_records UNUSED, UINT32 n_bytes UNUSED, const char *path UNUSED)
{
    return -1;
}
#endif

#ifdef FLIGHTRECORDER
void flightrecorder_change(UINT32 enable_bits)
{
    if (flight_record)
        flightrecorder_enable_bits = enable_bits;
}
#else
void flightrecorder_change(UINT32 enable_bits UNUSED)
{
}
#endif

void flightrecorder_restart(void)
{
#ifdef FLIGHTRECORDER
    if (header)
        header->starting_tsc = get_tsc();
#endif
}

void flightrecorder_close(void)
{
#ifdef FLIGHTRECORDER
    if (fd >= 0)
    {
        int rc;
        UINT32 num_pages = header->num_pages;

        rc = munmap(header, num_pages);
        if (rc < 0)
            fprintf(stderr, "flightrecorder munmap failed %d, %s\n", errno, strerror(errno));

        close(fd);
        fd = -1;
        if (pathname)
            free(pathname);
        pathname = NULL;
        header = NULL;
        records = records_end = flight_record = NULL;
        flightrecorder_enable_bits = 0x0;
    }
#endif
}


#ifdef FLIGHTRECORDER

static void fill_empty(UINT8 *dst, INT32 count)
{
    UINT32 *word = (UINT32 *)dst;

    while (count > 0)
    {
        *word++ = 0xEFBEADDE;
        count -= 4;
    }
}

__attribute__((regparm(3)))
static void copy_any_sgl_bytes(UINT8 *dst, SGL *sgl)
{
    UINT8 *record_buffer = dst;
    UINT8 *record_end = dst + num_bytes;
    SGL_DESC *desc = (SGL_DESC *)&sgl[1];
    UINT16 count = sgl->scnt;
    UINT32 bytes = num_bytes;

    while ((record_buffer < record_end) && desc->addr && count--)
    {
        UINT32 len = desc->len & 0x00FFFFFF;

        if (bytes < len)
            len = bytes;
        memcpy(record_buffer, desc->addr, len);
        record_buffer += len;
        desc++;
    }

    if (record_buffer < record_end)
        memset(record_buffer, 0xFF, record_end - record_buffer);
}

static always_inline void copy_sgl_bytes(UINT8 *dst, SGL *sgl)
{
    if (highly_likely(sgl) && highly_likely(sgl->scnt >= 1))
    {
        SGL_DESC *desc = (SGL_DESC *)&sgl[1];
        UINT32 len = (desc->len & 0x00FFFFFF);

        /* gcc will inline memcpy when the length is constant, so
         * check for the most likely lengths to keep them fast.
         */
        switch (num_bytes)
        {
            case 8:
                if (highly_likely(len >= 8))
                    memcpy(dst, desc->addr, 8);
                else
                    copy_any_sgl_bytes(dst, sgl);
                break;
            case 16:
                if (highly_likely(len >= 16))
                    memcpy(dst, desc->addr, 16);
                else
                    copy_any_sgl_bytes(dst, sgl);
                break;
            case 24:
                if (highly_likely(len >= 24))
                    memcpy(dst, desc->addr, 24);
                else
                    copy_any_sgl_bytes(dst, sgl);
                break;
            case 32:
                if (highly_likely(len >= 32))
                    memcpy(dst, desc->addr, 32);
                else
                    copy_any_sgl_bytes(dst, sgl);
                break;
            default:
                copy_any_sgl_bytes(dst, sgl);
                break;
        }
    }
    else
    {
        fill_empty(dst, num_bytes);
    }
}

static always_inline void copy_data_bytes(UINT8 *dst, UINT8 *data)
{
    if (highly_likely(data))
    {
        /* gcc will inline memcpy when the length is constant, so
         * check for the most likely lengths to keep them fast.
         */
        switch (num_bytes)
        {
            case 8:
                memcpy(dst, data, 8);
                break;
            case 16:
                memcpy(dst, data, 16);
                break;
            case 32:
                memcpy(dst, data, 32);
                break;
            default:
                memcpy(dst, data, num_bytes);
                break;
        }
    }
    else
    {
        fill_empty(dst, num_bytes);
    }
}



static always_inline void add_vrp_sgl(UINT8 record_type, void *rp, UINT16 function, UINT16 id,
                                      UINT64 sda, UINT32 length,
                                      UINT8 options, UINT8 path, UINT8 status,
                                      SGL *sgl)
{
    vrp_flight_record_t *record = &(flight_record->v);

    record->type = FR_UNUSED;
    compiler_barrier();

    record->status = status;
    record->id = id;
    record->length = length;
    record->sda = sda;
    record->tsc = get_tsc();
    record->function = function;
    record->options = options;
    record->path = path;
    record->addr = (UINT32)rp;

    if (num_bytes)
        copy_sgl_bytes(record->bytes, sgl);

    compiler_barrier();
    record->type = record_type;

    flight_record = (flight_record_t *)(record->bytes + num_bytes);
    if (flight_record >= records_end)
        flight_record = records;
}

static always_inline void add_vrp_data(UINT8 record_type, void *rp, UINT16 function, UINT16 id,
                                       UINT64 sda, UINT32 length,
                                       UINT8 options, UINT8 path, UINT8 status,
                                       UINT8 *data)
{
    vrp_flight_record_t *record = &(flight_record->v);

    record->type = FR_UNUSED;
    compiler_barrier();

    record->status = status;
    record->id = id;
    record->length = length;
    record->sda = sda;
    record->tsc = get_tsc();
    record->function = function;
    record->options = options;
    record->path = path;
    record->addr = (UINT32)rp;

    if (num_bytes)
        copy_data_bytes(record->bytes, data);

    compiler_barrier();
    record->type = record_type;

    flight_record = (flight_record_t *)(record->bytes + num_bytes);
    if (flight_record >= records_end)
        flight_record = records;
}

static always_inline void add_vrp_nodata(UINT8 record_type, void *rp, UINT16 function, UINT16 id,
                                         UINT64 sda, UINT32 length,
                                         UINT8 options, UINT8 path, UINT8 status)
{
    vrp_flight_record_t *record = &(flight_record->v);

    record->type = FR_UNUSED;
    compiler_barrier();

    record->status = status;
    record->id = id;
    record->length = length;
    record->sda = sda;
    record->tsc = get_tsc();
    record->function = function;
    record->options = options;
    record->path = path;
    record->addr = (UINT32)rp;

    if (num_bytes)
        fill_empty(record->bytes, num_bytes);

    compiler_barrier();
    record->type = record_type;

    flight_record = (flight_record_t *)(record->bytes + num_bytes);
    if (flight_record >= records_end)
        flight_record = records;
}


static always_inline void add_rprp(UINT8 record_type, void *rp, UINT8 function, UINT16 id,
                                   UINT64 sda, UINT32 length, UINT32 data, SGL *sgl)
{
    rprp_flight_record_t *record = &(flight_record->rp);

    record->type = FR_UNUSED;
    compiler_barrier();

    record->function = function;
    record->id = id;
    record->length = length;
    record->sda = sda;
    record->tsc = get_tsc();
    record->data = data;
    record->addr = (UINT32)rp;

    if (num_bytes)
        copy_sgl_bytes(record->bytes, sgl);

    compiler_barrier();
    record->type = record_type;

    flight_record = (flight_record_t *)(record->bytes + num_bytes);
    if (flight_record >= records_end)
        flight_record = records;
}


__attribute__((regparm(3)))
void flightrecorder_add_vrp_nodata(UINT8 type, void *rp)
{
    VRP *vrp = rp;

    add_vrp_nodata(type,
                 vrp,
                 vrp->function,
                 vrp->vid,
                 vrp->startDiskAddr,
                 vrp->length,
                 vrp->options,
                 vrp->path,
                 vrp->status);
}

__attribute__((regparm(3)))
void flightrecorder_add_vrp_data(UINT8 type, void *rp, void *data)
{
    VRP *vrp = rp;

    add_vrp_data(type,
                 vrp,
                 vrp->function,
                 vrp->vid,
                 vrp->startDiskAddr,
                 vrp->length,
                 vrp->options,
                 vrp->path,
                 vrp->status,
                 data);
}

__attribute__((regparm(3)))
void flightrecorder_add_vrp(UINT8 type, void *rp)
{
    VRP *vrp = rp;

    add_vrp_sgl(type,
                vrp,
                vrp->function,
                vrp->vid,
                vrp->startDiskAddr,
                vrp->length,
                vrp->options,
                vrp->path,
                vrp->status,
                vrp->pSGL);
}

__attribute__((regparm(3)))
void flightrecorder_add_rrp(UINT8 type, void *rp, UINT32 data)
{
    RRP *rrp = rp;

    add_rprp(type,
             rrp,
             rrp->function,
             rrp->rid,
             rrp->startDiskAddr,
             rrp->length,
             data,
             rrp->pSGL);
}

__attribute__((regparm(3)))
void flightrecorder_add_prp(UINT8 type, void *rp, UINT32 data)
{
    PRP *prp = rp;

    add_rprp(type,
             prp,
             prp->cmd[0],
             (prp->pDev && prp->pDev->pdd) ? prp->pDev->pdd->pid : 0xFFFF,
             prp->sda,
             prp->rqBytes,
             data,
             prp->pSGL);
}

#endif

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
