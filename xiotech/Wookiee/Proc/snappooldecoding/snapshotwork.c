// #define M4_OGERS
#define SNAPPOOL 100
/* ========================================================================== */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>

/* ------------------------------------------------------------------------ */
#include "vdd.h"
#include "RL_RDD.h"
#include "ss_nv.h"
#include "XIO_Macros.h"
/* ------------------------------------------------------------------------ */
#define p_MallocC(a,b,c)    calloc(1,a)
#define p_Free(a,b,c,d)     free(a)
/* ------------------------------------------------------------------------ */
extern int fd;
extern off_t g_capacity;

/* ------------------------------------------------------------------------ */
void restore_ss_nv_task(void);          /* referenced from snapshottest.c */

/* Forward reference */
void print_oger(OGER *oger);
/* ------------------------------------------------------------------------ */
// static struct OGER_NV oger_nv;

static struct OGER *gnv_oger_array;
static struct SS_HEADER_NV *gnv_header_array;
static UINT8    gss_version;

static UINT8    DEF_dog_table[(MAX_OGER_COUNT_GT2TB/8)] = {1};  /* number bytes */
static UINT32   DEF_oger_cnt;

/* ------------------------------------------------------------------------ */
static struct VDX gVDX;
/* ------------------------------------------------------------------------ */
// Number of right fork tasks, by SSMS.
static int g_which_fork_count = 0;
static int             right_fork_count[MAX_SNAPSHOT_COUNT];

// Number of tasks forked for reading SSMSs.
static int num_ssms_read_tasks = 0;

// Number of tasks forked for reading OGERs.
// static int             num_oger_read_tasks = 0;

// Number of ogers read for an SSMS.
static int             ogers_read_per_ssms[MAX_SNAPSHOT_COUNT];

// Number of OGERs restored from SSMS reading.
static int             num_ogers_from_ssms = 0;

// OGER count from SSMS reading (which snappool is the index).
static int             cuml_ssms_oger_cnt = 0;

static SSMS *DEF_ssms_table[MAX_SNAPSHOT_COUNT]; /* SSMS table for the first snappool */

// The Oger queue is for all SSMSs and for both pool_id's, when there are
// too many OGER read tasks created.
static struct oger_task_queue {
    struct oger_task_queue *next;
    OGER *oger_to_read;
    OGER **N_poger_table;
    int pool_id;
    int which_fork_count;
} *oger_task_queue_first = NULL;

/* ------------------------------------------------------------------------ */
/* Global flag to indicate a duplicate oger found. */
static UINT32          g_duplicate_oger = FALSE;

/* ------------------------------------------------------------------------ */
SS_HEADER_NV *read_ss_header_nv(off_t block);
SS_HEADER_NV *read_ss_header_nv(off_t block)
{
    ssize_t lth;
    int i;
    off_t offset;
    SS_HEADER_NV *ss_header_nv = (SS_HEADER_NV *)calloc(1, sizeof(SS_HEADER_NV));

    offset = block * 512;
    lseek(fd, offset, SEEK_SET);
fprintf(stderr, "%s: lseek(%ld) -- block %ld\n", __func__, offset, block);
    lth = read(fd, ss_header_nv, sizeof(*ss_header_nv));
    if (lth < 0)
    {
        perror("read error in read_ss_header_nv");
        exit(1);
    }
    if (lth != sizeof(*ss_header_nv))
    {
        perror("read too short error in read_ss_header_nv");
        fprintf(stderr, "read %zd bytes out of %zd\n", lth, sizeof(ss_header_nv));
        exit(1);
    }
    fprintf(stderr, "SS_HEADER_NV:\n");
    fprintf(stderr, "    version     =%5d Version of this record\n", ss_header_nv->version);
    fprintf(stderr, "    header_magic= 0x%02x Magic number to indicate initial condition of header NV\n", ss_header_nv->header_magic);
    fprintf(stderr, "    spool_vid   =%5d Ordinal of this header\n", ss_header_nv->spool_vid);
    fprintf(stderr, "    length      =%5d Length of this header\n", ss_header_nv->length);
    fprintf(stderr, "    ssms_count  =%5d Number of ssms_nv's stored\n", ss_header_nv->ssms_count);
    fprintf(stderr, "    ssms_offset =%5d First ssms_nv offset\n", ss_header_nv->ssms_offset);
    fprintf(stderr, "    oger_offset =%5d First oger_nv offset\n", ss_header_nv->oger_offset);

    fprintf(stderr, "    ord_map[%3d]       Bitmap of stored ssms_nvs\n", MAX_SNAPSHOT_COUNT);
    for (i = 0; i< MAX_SNAPSHOT_COUNT; i++)
    {
        if ((i % 16) == 0) {
            fprintf(stderr, "%4d=", i);
        }
        fprintf(stderr, "%c0x%02x", ss_header_nv->ord_map[i] == 0 ?' ':'*', ss_header_nv->ord_map[i]);
        if ((i % 16) == 15) {
            fprintf(stderr, "\n");
        }
    }

//     fprintf(stderr, "    reserved[8] =0x%08x %08x %08x %08x %08x %08x %08x %08x\n", ss_header_nv->reserved[0],
//    ss_header_nv->reserved[1], ss_header_nv->reserved[2], ss_header_nv->reserved[3], ss_header_nv->reserved[4],
//    ss_header_nv->reserved[5], ss_header_nv->reserved[6], ss_header_nv->reserved[7]);

    fprintf(stderr, "    crc         =0x%08x\n", ss_header_nv->crc);
    return(ss_header_nv);
}   /* End of read_ss_header_nv() */

/* ------------------------------------------------------------------------ */
void print_lth(void *bufv, unsigned int lth);
void print_lth(void *bufv, unsigned int lth)
{
    unsigned int i;
    unsigned char *buf = (unsigned char *)bufv;

    fprintf(stderr, "Data:\n");
    for (i = 0; i < lth; i++)
    {
        if ((i % 32) == 0) {
            fprintf(stderr, "%4d=", i);
        }
        fprintf(stderr, " %02x", buf[i]);
        if ((i % 32) == 31) {
            fprintf(stderr, "\n");
        }
    }
    if ((i % 32) != 0)
    {
        fprintf(stderr, "\n");
    }
}   /* End of print_lth */

/* ------------------------------------------------------------------------ */
void print_data(off_t block);
void print_data(off_t block)
{
    off_t offset;
    char buf[512];
    ssize_t lth;

    offset = block * 512;
    lseek(fd, offset, SEEK_SET);
fprintf(stderr, "%s: lseek(%ld) -- block %ld\n", __func__, offset, block);
    lth = read(fd, buf, 512);
    if (lth < 0)
    {
        perror("read error in read_ss_header_nv");
        exit(1);
    }
    if (lth != 512)
    {
        perror("read too short error in read_ss_header_nv");
        fprintf(stderr, "read %zd bytes out of %zd\n", lth, 512);
        exit(1);
    }
    print_lth((void*)buf, 512);
}   /* End of read_ss_header_nv() */
/* ------------------------------------------------------------------------ */
void print_ssms(const char *str, SSMS *ssms);
void print_ssms(const char *str, SSMS *ssms)
{
    int i;
    UINT32 j;
    UINT8 m;

    fprintf(stderr, "print_ssms: %s @%p\n", str, ssms);

    fprintf(stderr, "    ssm_link     =%p Next SSMS\n", ssms->ssm_link);
    fprintf(stderr, "    ssm_synchead =%p First sync record\n", ssms->ssm_synchead);
    fprintf(stderr, "    ssm_synctail =%p Tail sync record\n", ssms->ssm_synctail);
    for (m = 0; m < 32; m++)
    {
        if (ssms->ssm_regmap[m] != NULL)
        {
            fprintf(stderr, "    ssm_regmap[%d] =%p Pointer to region map\n", m, ssms->ssm_regmap[m]);
            fprintf(stderr, "    RM type      =%5d Region table type\n", ssms->ssm_regmap[m]->type);
            fprintf(stderr, "    RM lastmerge =%5d Last merge timestamp\n", ssms->ssm_regmap[m]->lastmerge);

            int total = 0;
            for (i = 0; i < MAXRMCNT; i++)
            {
                if (ssms->ssm_regmap[m]->regions[i] != NULL) {
                    int k;
                    int l = 0;
                    char pbuffer[512];

                    fprintf(stderr, "  %d RM cnt       =%5d Number of segments\n", i, ssms->ssm_regmap[m]->regions[i]->cnt);
                    total += ssms->ssm_regmap[m]->regions[i]->cnt;
                    pbuffer[0] = '\0';
                    for (j = 0; j < (ssms->ssm_regmap[m]->regions[i]->cnt + 31)/32; j++)
                    {
                        if ((j % 8) == 0) {
                            fprintf(stderr, "%4d=", j);
                        }
                        fprintf(stderr, "%c0x%08x", ssms->ssm_regmap[m]->regions[i]->segments[j] == 0xffffffff ?' ':'*', ssms->ssm_regmap[m]->regions[i]->segments[j]);
                        for (k = 0; k < 32; k++)
                        {
                          if (!BIT_TEST(ssms->ssm_regmap[m]->regions[i]->segments[j], k))
                          {
                            l++;
                            if (l > 10)
                            {
                                sprintf(pbuffer, "%s\n     ", pbuffer);
                                l = 0;
                            }
                            sprintf(pbuffer, "%s %d", pbuffer, (i*8192 + j*32 + (31-k)) * 1024*2);
                          }
                        }
                        if ((j % 8) == 7) {
                            fprintf(stderr, "\n");
                            if (pbuffer[0] != '\0')
                            {
                              fprintf(stderr, "Block%s\n", pbuffer);
                              pbuffer[0] = '\0';
                            }
                        }
                    }
                    if ((j % 8) != 0)
                    {
                        fprintf(stderr, "\n");
                        if (pbuffer[0] != '\0')
                        {
                          fprintf(stderr, "Block%s\n", pbuffer);
                          pbuffer[0] = '\0';
                        }
                    }
                }
            }
            fprintf(stderr, "There are %d segment bits available, %lld clear (in use).\n", total,
            (gVDX.vdd[ssms->ssm_srcvid]->devCap * 512/(1024*1024)) - total);
        }
    }
    fprintf(stderr, "    ssm_srcvid   =%5d VID of source Vdisk\n", ssms->ssm_srcvid);
    fprintf(stderr, "    ssm_ssvid    =%5d VID of snapshot Vdisk\n", ssms->ssm_ssvid);
    fprintf(stderr, "    ssm_stat     = 0x%02x Status of the snapshot\n", ssms->ssm_stat);
    fprintf(stderr, "    ssm_flags    = 0x%02x Status of the snapshot\n", ssms->ssm_flags);
    fprintf(stderr, "    ssm_prefowner=%5d Preferred owner (0 or 1)\n", ssms->ssm_prefowner);
//    fprintf(stderr, "    ssm_res1     = 0x%02x Reserved 1 byte\n", ssms->ssm_res1);
    fprintf(stderr, "    ssm_frstoger =%p Pointer to first OGer\n", ssms->ssm_frstoger);
    fprintf(stderr, "    ssm_tailoger =%p Pointer to tail OGer\n", ssms->ssm_tailoger);
    fprintf(stderr, "ssm_prev_tailoger=%p Pointer to previous tail OGer\n", ssms->ssm_prev_tailoger);
    fprintf(stderr, "    ssm_ogercnt  =%5d Number of OGers\n", ssms->ssm_ogercnt);
    fprintf(stderr, "    ssm_ordinal  =%5d Ordinal of this SSMS\n", ssms->ssm_ordinal);
//    fprintf(stderr, "    res2[4]      =0x%02x%02x%02x%02x Reserved 4 bytes\n", ssms->res2[0], ssms->res2[1], ssms->res2[2], ssms->res2[3]);

    fprintf(stderr, " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n");
    OGER *next = ssms->ssm_frstoger;
    for (i = 0; i < ssms->ssm_ogercnt; i++)
    {
        print_oger(next);
        next = next->ogr_link;
    }
//    fprintf(stderr, " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n");

}   /* End of print_ssms */

/* ------------------------------------------------------------------------ */
static void read_ssms_nv(SSMS_NV *ssms_nv, UINT64 block)
{
    ssize_t lth;
    off_t offset;

    offset = block * 512;
    lseek(fd, offset, SEEK_SET);
fprintf(stderr, "%s: lseek(%ld) -- block %lld\n", __func__, offset, block);
    lth = read(fd, (void*)ssms_nv, sizeof(*ssms_nv));
    if (lth < 0)
    {
        perror("read error in read_ssms_nv");
        exit(1);
    }
    if (lth != sizeof(*ssms_nv))
    {
        perror("read too short error in read_ssms_nv");
        fprintf(stderr, "read %zd bytes out of %zd\n", lth, sizeof(*ssms_nv));
        exit(1);
    }
// print_lth((void*)ssms_nv, sizeof(*ssms_nv));
#if 1
    fprintf(stderr, "SSMS_NV(%lld):\n", block);
    fprintf(stderr, "    ordinal      =%5d Ordinal of this SSMS\n", ssms_nv->ordinal);
    fprintf(stderr, "    srcvid       =%5d VID of the source\n", ssms_nv->srcvid);
    fprintf(stderr, "    ssvid        =%5d VID of the snapshot\n", ssms_nv->ssvid);
    fprintf(stderr, "    notused1     =0x%08x Not used\n", ssms_nv->notused1);
    fprintf(stderr, "    firstoger    =%5d Ordinal of first OGer\n", ssms_nv->firstoger);
    fprintf(stderr, "    tailoger     =%5d Ordinal of tail OGer\n", ssms_nv->tailoger);
    fprintf(stderr, "    prev_tailoger=%5d Ordinal of previous tail OGer\n", ssms_nv->prev_tailoger);
    fprintf(stderr, "    ogercnt      =%5d Number of OGers\n", ssms_nv->ogercnt);
    fprintf(stderr, "    status       = 0x%02x Status of the snapshot\n", ssms_nv->status);
    fprintf(stderr, "    version      =%5d Version of this record\n", ssms_nv->version);
    fprintf(stderr, "    prefowner    =%5d Preferred owner (0 or 1)\n", ssms_nv->prefowner);
//    fprintf(stderr, "    reserved     =0x%02x Alignment\n", ssms_nv->reserved);
//    fprintf(stderr, "    resw[8]      =0x%08x%08x%08x%08x%08x%08x%08x%08x Extra space\n", ssms_nv->resw[0], ssms_nv->resw[1], ssms_nv->resw[2], ssms_nv->resw[3], ssms_nv->resw[4], ssms_nv->resw[5], ssms_nv->resw[6], ssms_nv->resw[7]);
    fprintf(stderr, "    crc          =0x%08x\n", ssms_nv->crc);
#endif  /* 0 */
}   /* End of read_ssms_nv() */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**  @brief      Finds the next populated SSMS in the NV header map.
**
**  This function will search the SSMS map in an NV header for the ordinal of
**  the next populated SSMS.  If the end is reached -1 is returned.  If a 0xfffe
**  is passed in as the last ordinal then the search will take place from the
**  beginning of the map.
**
**  @return     Ordinal of the next SSMS or -1 if not found.
******************************************************************************
*/
static UINT16 find_next(SS_HEADER_NV *ss_header_nv, UINT16 last_ordinal)
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
        if (ss_header_nv->ord_map[test_ordinal])
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

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**  @brief      We need to read a right OGER, fork, or queue if too many.
**
**  @param      oger_to_read     -- which oger to read.
**  @param      N_poger_table    -- memory holding all ogers for a snappool.
**  @param      nv_oger          -- First oger for snappool.
**  @param      pool_id          -- snappool 0 or 1 (for controller) -- from vid.
**  @param      which_fork_count -- index into array of right task counts for SSMS.
**
**  @return     None.
******************************************************************************
*/
static void right_task_queue(int oger_to_read, OGER** N_poger_table, int which_fork_count)
{
    // Must do count first, before task create or queue, so that original SSMS
    // read will wait for all OGER reads to finish.
    right_fork_count[which_fork_count] += 1;
// fprintf(stderr, "%s:%u right_fork_count[%d]=%d\n", __func__, __LINE__, which_fork_count, right_fork_count[which_fork_count]);

    // If only a few OGER read tasks, fork, else put it on a queue.
        struct oger_task_queue *oger_task_queue;

        oger_task_queue = p_MallocC(sizeof(struct oger_task_queue), __FILE__, __LINE__);
        oger_task_queue->oger_to_read = N_poger_table[oger_to_read]->ogr_rightch;
        oger_task_queue->N_poger_table = N_poger_table;
        oger_task_queue->pool_id = 0;
        oger_task_queue->which_fork_count = which_fork_count;
        oger_task_queue->next = oger_task_queue_first;
        oger_task_queue_first = oger_task_queue;
// fprintf(stderr, "%s:%u putting oger_to_read=%d on OGER task queue\n", __func__, __LINE__, oger_to_read);
}                                       /* End of right_task_queue */

/* ------------------------------------------------------------------------ */
void print_oger(OGER *oger)
{
    int i;

    fprintf(stderr, "OGER(%p):\n", oger);
    fprintf(stderr, "    vid         =%5d VID of the snappool\n", oger->ogr_vid);
    fprintf(stderr, "    segcnt      =%5d Number of segs\n", oger->ogr_segcnt);
    fprintf(stderr, "    sda         =%5lld SDA of Oger in snappool\n", oger->ogr_sda);
    fprintf(stderr, "    ord         =%5d Ordinal of this OGer\n", oger->ogr_ord);
    fprintf(stderr, "    maxpr       =%5d Max probes done on insert\n", oger->ogr_maxpr);
    fprintf(stderr, "    ssvid       =%5d Snapshot VID of this OGER\n", oger->ogr_ssvid);
    fprintf(stderr, "    stat        = 0x%02x Status of this OGer\n", oger->ogr_stat);
    fprintf(stderr, "    leftch      =%p OGER of left child\n", oger->ogr_leftch);
    fprintf(stderr, "    rightch     =%p OGER of right child\n", oger->ogr_rightch);
    fprintf(stderr, "    parent      =%p OGER of the parent\n", oger->ogr_parent);
    fprintf(stderr, "    sdakey      =%5d SDA key value\n", oger->ogr_sdakey);

    fprintf(stderr, "    segfield[%3d]      Segment bitfield\n", SEGSPEROGER/8);
    for (i = 0; i< SEGSPEROGER/8; i++)
    {
        if ((i % 16) == 0) {
            fprintf(stderr, "%4d=", i);
        }
        fprintf(stderr, "%c0x%02x", oger->ogr_segfld[i] == 0 ?' ':'*', oger->ogr_segfld[i]);
        if ((i % 16) == 15) {
            fprintf(stderr, "\n");
        }
    }

    fprintf(stderr, "    sdamap[%d]       SDA map\n", SEGSPEROGER);
    for (i = 0; i< SEGSPEROGER; i++)
    {
        if ((i % 8) == 0) {
            fprintf(stderr, "%4d=", i);
        }
        fprintf(stderr, "%c0x%08x", oger->ogr_sdamap[i] == 0 ?' ':'*', oger->ogr_sdamap[i]);
        if ((i % 8) == 7) {
            fprintf(stderr, "\n");
        }
    }

    if (oger->ogr_leftch != 0)
    {
        fprintf(stderr, " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n");
        print_oger(oger->ogr_leftch);
    }

    if (oger->ogr_rightch != 0)
    {
        fprintf(stderr, " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n");
        print_oger(oger->ogr_rightch);
    }
}   /* End of print_oger */

/**
******************************************************************************
**  @brief      Read a specific OGER_NV from the NV data.
**
**  This function will read a specific OGER_NV into a new volatile structure.
**
**  @return     Pointer to the new OGer or NULL if unsuccessful.
******************************************************************************
*/
static void read_oger_nv(void *oger_input, UINT64 block)
{
    ssize_t lth;
    off_t offset;
    OGER_NV *oger_nv = (OGER_NV *)oger_input;
    OGER_NV_GT2TB *oger_nv_gt2tb = (OGER_NV_GT2TB *)oger_input;

    offset = block * 512;
    lseek(fd, offset, SEEK_SET);
fprintf(stderr, "%s: lseek(%ld) -- block %lld\n", __func__, offset, block);
// NOTE: both are the same size.
    lth = read(fd, (void*)oger_nv, sizeof(OGER_NV));
    if (lth < 0)
    {
        perror("read error in read_oger_nv");
        exit(1);
    }
    if (lth != sizeof(OGER_NV))
    {
        perror("read too short error in read_oger_nv");
        fprintf(stderr, "read %zd bytes out of %zd\n", lth, sizeof(OGER_NV));
        exit(1);
    }
// print_lth((void*)oger_nv, sizeof(OGER_NV));
#if 1
    if (gss_version == SS_NV_VERSION_GT2TB)
    {
        fprintf(stderr, "OGER_NV_GT2TB(%lld):\n", block);
    } else {
        fprintf(stderr, "OGER_NV(%lld):\n", block);
    }
    fprintf(stderr, "    vid         =%5d VID of the snappool\n", oger_nv->vid);
    fprintf(stderr, "    leftch      =%5d Ordinal of left child\n", oger_nv->leftch);
    fprintf(stderr, "    rightch     =%5d Ordinal of right child\n", oger_nv->rightch);
    fprintf(stderr, "    parent      =%5d Ordinal of the parent\n", oger_nv->parent);
    fprintf(stderr, "    ordinal     =%5d Ordinal of this OGer\n", oger_nv->ordinal);
    fprintf(stderr, "    maxprobe    =%5d Max probes done on insert\n", oger_nv->maxprobe);
    fprintf(stderr, "    segcount    =%5d Number of segs stored\n", oger_nv->segcount);
    fprintf(stderr, "    version     =%5d Version of this record\n", oger_nv->version);
    fprintf(stderr, "    status      = 0x%02x Status of this OGer\n", oger_nv->status);
    int i;
    if (gss_version != SS_NV_VERSION_GT2TB)
    {
        fprintf(stderr, "    sda         =%5d SDA of Oger in snappool\n", oger_nv->sda);
        fprintf(stderr, "    sdakey      =%5d SDA key value\n", oger_nv->sdakey);

        fprintf(stderr, "    segfield[%3d]      Segment bitfield\n", SEGSPEROGER/8);
        for (i = 0; i< SEGSPEROGER/8; i++)
        {
            if ((i % 16) == 0) {
                fprintf(stderr, "%4d=", i);
            }
            fprintf(stderr, "%c0x%02x", oger_nv->segfield[i] == 0 ?' ':'*', oger_nv->segfield[i]);
            if ((i % 16) == 15) {
                fprintf(stderr, "\n");
            }
        }

        fprintf(stderr, "    sdamap[%d]       SDA map\n", SEGSPEROGER);
        for (i = 0; i< SEGSPEROGER; i++)
        {
            if ((i % 8) == 0) {
                fprintf(stderr, "%4d=", i);
            }
            fprintf(stderr, "%c0x%08x", oger_nv->sdamap[i] == 0 ?' ':'*', oger_nv->sdamap[i]);
            if ((i % 8) == 7) {
                fprintf(stderr, "\n");
            }
        }

        fprintf(stderr, "    next        =%5d Ordinal of next OGER\n", oger_nv->next);
//    fprintf(stderr, "    reserved[4] =0x%08x %08x %08x %08x\n", oger_nv->reserved[0], oger_nv->reserved[1], oger_nv->reserved[2],
//    oger_nv->reserved[3]);
        fprintf(stderr, "    crc         =0x%08x\n", oger_nv->crc);
    } else {
        fprintf(stderr, "    sda         =%5lld SDA of Oger in snappool\n", oger_nv_gt2tb->sda);
        fprintf(stderr, "    sdakey      =%5d SDA key value\n", oger_nv_gt2tb->sdakey);

        fprintf(stderr, "    segfield[%3d]      Segment bitfield\n", SEGSPEROGER/8);
        for (i = 0; i< SEGSPEROGER/8; i++)
        {
            if ((i % 16) == 0) {
                fprintf(stderr, "%4d=", i);
            }
            fprintf(stderr, "%c0x%02x", oger_nv_gt2tb->segfield[i] == 0 ?' ':'*', oger_nv_gt2tb->segfield[i]);
            if ((i % 16) == 15) {
                fprintf(stderr, "\n");
            }
        }

        fprintf(stderr, "    sdamap[%d]       SDA map\n", SEGSPEROGER);
        for (i = 0; i< SEGSPEROGER; i++)
        {
            if ((i % 8) == 0) {
                fprintf(stderr, "%4d=", i);
            }
            fprintf(stderr, "%c0x%08x", oger_nv_gt2tb->sdamap[i] == 0 ?' ':'*', oger_nv_gt2tb->sdamap[i]);
            if ((i % 8) == 7) {
                fprintf(stderr, "\n");
            }
        }

        fprintf(stderr, "    next        =%5d Ordinal of next OGER\n", oger_nv_gt2tb->next);
    }
#endif  /* 0 */
}                                      /* End of read_oger_nv */

/* ------------------------------------------------------------------------ */
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
int ss_clr_seg_bit(UINT32 seg_num, SSMS *ssms);
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

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**  @brief  Read an OGER entry from NV (in the snapshot vdisk).
**
**  @param  ordinal         -- The number of the OGER to read.
**  @param  N_poger_table   -- Memory to hold all ogers for a snappool.
**  @param  nv_oger         -- First oger for snappool (the header).
**  @param  which_fork_count - Count of right tasks for this SSMS.
**  @param  ssms            - Pointer to SSMS
******************************************************************************
*/
void read_oger_from_nv(int ordinal, OGER **N_poger_table, OGER *nv_oger, int which_fork_count, SSMS *ssms);
void read_oger_from_nv(int ordinal, OGER **N_poger_table, OGER *nv_oger, int which_fork_count, SSMS *ssms)
{
// fprintf(stderr, "%s:%u Enter(ordinal=%d, %p, %p, which_fork_count=%d, %p)\n", __func__, __LINE__, ordinal, N_poger_table, nv_oger, which_fork_count, ssms);
    OGER           *oger;
    UINT32          y;
    UINT32         oger_read_fail = FALSE;
    OGER            readithere;
    struct OGER_NV *oger_nv;
    struct OGER_NV_GT2TB *oger_nv_gt2tb;

    int             oger_to_read = ordinal;

    // Get structures to issue a read request.

    while (1)
    {
        if (oger_to_read && N_poger_table[oger_to_read])
        {
            fprintf(stderr, "%s: Duplicate oger, ord=%d, vid1=%d, vid2=%d\n",
                __func__, oger_to_read, N_poger_table[oger_to_read]->ogr_ssvid,
                ssms->ssm_ssvid);
            oger_to_read = 0;
            g_duplicate_oger = TRUE;
            continue;
        }

        if ((!oger_to_read) || (oger_read_fail == TRUE))
        {
            // This right fork processing has terminated.
            right_fork_count[which_fork_count] -= 1;
// fprintf(stderr, "%s:%u right_fork_count[%d]=%d\n", __func__, __LINE__, which_fork_count, right_fork_count[which_fork_count]);

            // If no OGER tasks on queue, exit.
            if (oger_task_queue_first == NULL)
            {
// fprintf(stderr, "%s:%u oger_task_queue_first == NULL\n", __func__, __LINE__);
                break;
            }

            // Take first queued task's parameters off queue, and dequeue.
            oger_to_read = (int)((unsigned long)oger_task_queue_first->oger_to_read & 0xffffffff);
            N_poger_table = oger_task_queue_first->N_poger_table;
            which_fork_count = oger_task_queue_first->which_fork_count;
            nv_oger = &readithere;

            // Free up memory and forward queue.
            struct oger_task_queue *next = oger_task_queue_first->next;

            free(oger_task_queue_first);
            oger_task_queue_first = next;
// fprintf(stderr, "%s:%u continuing OGER read for oger_to_read=%d\n", __func__, __LINE__, oger_to_read);
            continue;
        }

        UINT64 startDiskAddr;
        if (gss_version == SS_NV_VERSION_GT2TB)
        {
            oger_nv_gt2tb = calloc(1,sizeof(*oger_nv_gt2tb));
            oger_nv = (OGER_NV *)oger_nv_gt2tb;
            startDiskAddr = FIRST_OGER_GT2TB + ((ordinal-1) * OGER_NV_ALLOC_GT2TB);
        }
        else
        {
            oger_nv = calloc(1,sizeof(*oger_nv));
            startDiskAddr = FIRST_OGER + (ordinal * OGER_NV_ALLOC);
        }
        read_oger_nv(oger_nv, startDiskAddr);
        ogers_read_per_ssms[which_fork_count] += 1;
// fprintf(stderr, "%s:%u ogers_read_per_ssms[%d]=%d\n", __func__, __LINE__, which_fork_count, ogers_read_per_ssms[which_fork_count]);

        // Allocate an OGer struct for the runtime software
        oger = (OGER *)calloc(1, sizeof(OGER));
        N_poger_table[oger_to_read] = oger;

        // Populate the OGer from the NV data.
        oger->ogr_vid = oger_nv->vid;
        oger->ogr_leftch = (OGER *)(unsigned long)oger_nv->leftch;
        oger->ogr_rightch = (OGER *)(unsigned long)oger_nv->rightch;
        oger->ogr_parent = (OGER *)(unsigned long)oger_nv->parent;
        oger->ogr_ord = oger_nv->ordinal;
        oger->ogr_maxpr = oger_nv->maxprobe;
        oger->ogr_segcnt = oger_nv->segcount;
        if (gss_version != SS_NV_VERSION_GT2TB)
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
                (void)ss_clr_seg_bit(oger->ogr_sdamap[y], ssms);
            }
        }

        num_ogers_from_ssms += 1;
// fprintf(stderr,"%s:%u num_ogers_from_ssms=%d\n", __func__, __LINE__, num_ogers_from_ssms);
// print_oger(oger);

        // If left oger entry is null, use this task to read possible right.
        if ((unsigned long)N_poger_table[oger_to_read]->ogr_leftch == 0)
        {
            oger_to_read = (unsigned long)N_poger_table[oger_to_read]->ogr_rightch;
        }
        else
        {
            // If a right oger entry exists, fork task to read it.
            if ((unsigned long)N_poger_table[oger_to_read]->ogr_rightch != 0)
            {
                right_task_queue(oger_to_read, N_poger_table, which_fork_count);
            }
            // Want to read new left oger next.
            oger_to_read = (unsigned long)N_poger_table[oger_to_read]->ogr_leftch;
        }
    }

    // Free the ILT, VRP, and SGL for the OGer
//    s_Free(sgl_oger, sizeof(SGL) + sizeof(SGL_DESC) + desc_oger->len, __func__, __LINE__);
//    vrp_oger->pSGL = NULL;
//    PM_RelILTVRPSGL(ilt_oger, vrp_oger);
}                                      /* End of read_oger_from_nv */

/* ------------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------------ */
void compute_ss_spareseness(SSMS *ssms);
void compute_ss_spareseness(SSMS *ssms)
{       
    UINT32  segs_available; 
    VDD     *vdd;
    UINT8   percent;
            
    segs_available = ss_available_segs(ssms);
    vdd = gVDX.vdd[ssms->ssm_ssvid];
    if (!vdd)   
    {               
        return; 
    }       

    percent = (UINT8)(segs_available * 100 / (vdd->devCap >> SECPOWER));
    vdd->scpComp = percent;
}   /* End of compute_ss_spareseness */

/* ------------------------------------------------------------------------ */
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
void D_init_slrm(UINT32 remaining_rm, UINT32 number_full_rm, SSMS *ssms);
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
/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**  @brief      Read an SSMS entry from the NV (in the snapshot vdisk).
**
**  @param      ordinal     -- Which SSMS to read.
**  @param      Npot        -- memory to hold all possible ogers for a snappool.
**
**  @return     None.
******************************************************************************
*/
void ss_read_ssms(UINT32 ordinal, OGER *p2nv_oger, OGER **N_poger_table);
void ss_read_ssms(UINT32 ordinal, OGER *p2nv_oger, OGER **N_poger_table)
{
    SSMS           *ssms;
    int             oger_to_read;
    int             which_fork_count;
    SSMS_NV        *ssms_nv;

    // We need a unique counter for the right forks, for each SSMS.
    which_fork_count = g_which_fork_count;
    g_which_fork_count++;
// fprintf(stderr, "%s:%u g_which_fork_count=%d\n", __func__, __LINE__, g_which_fork_count);
    right_fork_count[which_fork_count] = 0;
// fprintf(stderr, "%s:%u right_fork_count[%d]=%d\n", __func__, __LINE__, which_fork_count, right_fork_count[which_fork_count]);
    ogers_read_per_ssms[which_fork_count] = 0;
// fprintf(stderr, "%s:%u ogers_read_per_ssms[%d]=%d\n", __func__, __LINE__, which_fork_count, ogers_read_per_ssms[which_fork_count]);

    // Allocate the ILT, VRP, and SGL for the SSMS retrieval(s)
    ssms_nv = calloc(1, sizeof(SSMS_NV));
    UINT64 startDiskAddr;
    if (gss_version == SS_NV_VERSION_GT2TB)
    {
        startDiskAddr = FIRST_SSMS_GT2TB + (ordinal * SSMS_NV_ALLOC_GT2TB);
    }
    else
    {
        startDiskAddr = FIRST_SSMS + (ordinal * SSMS_NV_ALLOC);
    }
    read_ssms_nv(ssms_nv, startDiskAddr);

// fprintf(stderr, "%s:%d: SSMS read CRC=%08x, vid=%d, ssms ord=%d\n", __func__, __LINE__, ssms_nv->crc, p2nv_oger->ogr_vid, ordinal);

    // Allocate an SSMS and fill in the values.
    ssms =  p_MallocC(sizeof(SSMS), __FILE__, __LINE__);
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
// print_ssms("gotten from NV:", ssms);

    oger_to_read = ssms_nv->firstoger;

    // Read up all of the OGers.
    if (oger_to_read != 0)
    {
        right_fork_count[which_fork_count] = 1;
// fprintf(stderr, "%s:%u right_fork_count[%d]=%d\n", __func__, __LINE__, which_fork_count, right_fork_count[which_fork_count]);
        read_oger_from_nv(oger_to_read, N_poger_table, p2nv_oger, which_fork_count, ssms);
    }

    // Wait for forked tasks to complete -- note: it might complete before us, check first.
    while (right_fork_count[which_fork_count] != 0)
    {
        // Set our task to "waiting for right OGER read tasks to complete".
//        TaskSetMyState(PCB_SS_R_OGER_READS);
        // Wait for all right oger tasks to complete.
//        TaskSwitch();
fprintf(stderr, "%s:%u sleep(1) -- right_fork_count[%d]=%d\n", __func__, __LINE__, which_fork_count, right_fork_count[which_fork_count]);
        sleep(1);
    }

    ssms->ssm_frstoger = N_poger_table[ssms_nv->firstoger];
    ssms->ssm_tailoger = N_poger_table[ssms_nv->tailoger];
    ssms->ssm_prev_tailoger = N_poger_table[ssms_nv->prev_tailoger];
    ssms->ssm_ogercnt = ssms_nv->ogercnt;
    ssms->ssm_ordinal = ssms_nv->ordinal;
    ssms->ssm_prefowner = ssms_nv->prefowner;
    cuml_ssms_oger_cnt += ssms->ssm_ogercnt;

    // Fill in the entry in the DEF_ssms_table
    if (!DEF_ssms_table[ssms->ssm_ordinal])
    {
        DEF_ssms_table[ssms->ssm_ordinal] = ssms;
    }

    gVDX.vdd[ssms->ssm_ssvid]->vd_incssms = ssms;
// fprintf(stderr, "%s:%u-%s vd_incssms set for %d to %p\n", __FILE__,__LINE__,__func__, ssms->ssm_ssvid, ssms);
    if (gVDX.vdd[ssms->ssm_srcvid]->vd_outssmstail)
    {
        gVDX.vdd[ssms->ssm_srcvid]->vd_outssmstail->ssm_link = ssms;
    }
    else
    {
        gVDX.vdd[ssms->ssm_srcvid]->vd_outssms = ssms;
    }
    gVDX.vdd[ssms->ssm_srcvid]->vd_outssmstail = ssms;


//    finished_with_SSMS_task(0);
    num_ssms_read_tasks--;
    if (num_ssms_read_tasks == 0)
    {
fprintf(stderr, "%s:%u All SSMS tasks finished.\n", __func__, __LINE__);
    }
    else
    {
// fprintf(stderr, "%s:%u a SSMS task finished (%d left).\n", __func__, __LINE__, num_ssms_read_tasks);
    }

    if (ogers_read_per_ssms[which_fork_count] == ssms->ssm_ogercnt)
    {
        fprintf(stderr, "SSDBG: Restored SSMS VID = %d\n", ssms->ssm_ssvid);
        compute_ss_spareseness(ssms);
    }
    else
    {
        // The proper number of OGers has not been restored for this SSMS so it must be set to inop.
        fprintf(stderr, "SSMS OGer restored count failed - Expected=%08X: Got=%08X", ssms->ssm_ogercnt,
            ogers_read_per_ssms[which_fork_count]);
    }
}                                       /* End of ss_read_ssms */

/* ------------------------------------------------------------------------ */
void restore_read_ssms(SS_HEADER_NV *header_nv, OGER *nv_oger, OGER *N_poger_table[MAX_OGER_COUNT]);
void restore_read_ssms(SS_HEADER_NV *header_nv, OGER *nv_oger, OGER *N_poger_table[MAX_OGER_COUNT])
{
    UINT16          ordinal;

    /* We need a unique counter for the right forks, for each SSMS -- initialize. */
    g_which_fork_count = 0;
// fprintf(stderr, "%s:%u g_which_fork_count=%d\n", __func__, __LINE__, g_which_fork_count);

    /* For each SSMS in the map retrieve the proper data. */
    ordinal = 0xffff;
    while ((ordinal = find_next(header_nv, ordinal)) != 0xffff)
    {
fprintf(stderr, "%s:%u restore_read_ssms, ordinal=%d, num_ssms_read_tasks=%d\n", __func__, __LINE__, ordinal, num_ssms_read_tasks);

        /* Increment number of read SSMS tasks outstanding. */
        num_ssms_read_tasks += 1;

// fprintf(stderr, "%s:%u restore_read_ssms creating task ss_read_ssms for task %d\n", __func__, __LINE__, ordinal);
        /* Fork task to read the ssms entry. */
//        CT_fork_tmp = (unsigned long)"ss_read_ssms";
//        TaskCreate6(C_label_referenced_in_i960asm(ss_read_ssms), K_xpcb->pc_pri,
//                    ordinal, (int)nv_oger, 0, (UINT32)N_poger_table);
        ss_read_ssms(ordinal, nv_oger, N_poger_table);
        for (;;) {
            if (num_ssms_read_tasks != 0) {
                sleep(1);
            }
            else
            {
                break;
            }
        }
// fprintf(stderr, "%s:%u restore_read_ssms created task ss_read_ssms for ordinal %d\n", __func__, __LINE__, ordinal);
    }
    
fprintf(stderr, "%s:%u All SSMS tasks finished!\n", __func__, __LINE__);

    if (g_duplicate_oger == TRUE)
    {
       fprintf(stderr, "SSDBG: Fatal error - Duplicate oger found, Invalidating all the snapshots\n");
       abort();
    }
}   /* End of restore_read_ssms */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**  @brief      Read the header from the NV (in the snapshot vdisk).
**
**  @return     0   - if no snapshots using this snappool.
**              1   - if snapshot crc is bad, and must invalidate all snapshots.
**             -1   - if everything is ok.
******************************************************************************
*/
int ss_restore_validate_header(SS_HEADER_NV *header_nv, UINT16 spool_vid);
int ss_restore_validate_header(SS_HEADER_NV *header_nv, UINT16 spool_vid)
{
    SS_HEADER_NV   *local_header_nv;

    // Allocate a local NV header structure if needed.
    local_header_nv = gnv_header_array;
    if (!local_header_nv)
    {
        local_header_nv = calloc(1, sizeof(SS_HEADER_NV));
        gnv_header_array = local_header_nv;
    }

    gss_version = header_nv->version;

    // The CRC was good so update the local structure with the data from NV
    memcpy(local_header_nv, header_nv, sizeof(SS_HEADER_NV));

    // Check if any snapshots using this snappool.
    if (header_nv->header_magic == SS_HEADER_MAGIC)
    {
        fprintf(stderr, "SSDBG:%s No Snapshots created on this snappool %d\n", __func__, spool_vid);
        return (0);
    }
    return (-1);
}                                       /* End of ss_restore_validate_header */

/* ------------------------------------------------------------------------ */
static void dump_bitmap(UINT8 *ogertable, const char *str, int pool_id)
{
    int i;
    int j;
    int max_oger_count;

    fprintf(stderr, "SSDBG: DUMPING OGER BITMAP %s, POOLID %d\n", str, pool_id);
    if (gss_version != SS_NV_VERSION_GT2TB)
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

/* ------------------------------------------------------------------------ */
void dump_ssms_table(void);
void dump_ssms_table(void)
{
    int i;
    int count = 0;

    fprintf(stderr, "DUMPING DEF_ssms_table\n");
    for (i = 0; i < MAX_SNAPSHOT_COUNT; i++)
    {
        if ((i % 8) == 0)
        {
            fprintf(stderr, "%4d", i);
        }
        fprintf(stderr, " 0x%08x", (UINT32)DEF_ssms_table[i]);
        if (DEF_ssms_table[i] != 0)
        {
            count++;
        }
        if ((i % 8) == 7)
        {
            fprintf(stderr, "\n");
        }
    }

    fprintf(stderr, "There are %d active Snapshots. . . . . . . . . . . . . . . . . . . . . .\n", count);

    for (i = 0; i < MAX_SNAPSHOT_COUNT; i++)
    {
        if (DEF_ssms_table[i] != 0)
        {
            print_ssms("", DEF_ssms_table[i]);
        }
    }
}   /* End of dump_ssms_table */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**  @brief      Restore all snapshot nv data
**
**  This function restores the entire contents of snapshot NV data.  New
**  structures are created as needed.
**
**  @return     Status -- 0 is good else vrp status code
******************************************************************************
*/
void restore_snap_data(void);
void restore_snap_data(void)
{
// fprintf(stderr, "restore_snap_data Enter\n");
    /* This table is for pointers to OGers that we read during restore. */
    OGER           **N_poger_table;

    SS_HEADER_NV   *header_nv;

    OGER           *oger;

    OGER           *nv_oger;
    int             N_restored_ogers = 0;
    UINT32          i;
    UINT32          j;

    UINT64          devCap = g_capacity;                /* vdd->devCap; */
    UINT64          capacity = devCap;                  /* vdd->devCap; */
    int             validate_header_check;

    // No ogers read yet.
    cuml_ssms_oger_cnt = 0;
    // No ogers restored from SSMS reading.
    num_ogers_from_ssms = 0;

    // Allocate an OGER for the NV data if doesn't exist already
    if (!gnv_oger_array)
    {
        oger = (OGER *)calloc(1, sizeof(OGER));
        /* This is very interesting. It sets oger->ogr_sda to zero for the first one. Etc. */
        /* A hidden side effect of the s_MallocC and this assignment. */
        gnv_oger_array = oger;
        oger->ogr_vid = SNAPPOOL;
    }

    nv_oger = gnv_oger_array;
    if (nv_oger->ogr_vid != SNAPPOOL)
    {
        fprintf(stderr, "ERROR?? restore_snap_data snappool %d, now %d\n", nv_oger->ogr_vid, SNAPPOOL);
        abort();
    }

    // Read up the header
    header_nv = read_ss_header_nv(nv_oger->ogr_sda);

// ...........................................................................

    // Validate snappool header.
    validate_header_check = ss_restore_validate_header(header_nv, SNAPPOOL);

    // If no snapshots created on this snappool, exit.
    if (validate_header_check == 0) { abort(); }
    // If crc bad.
    if (validate_header_check == 1) { abort(); }

// ...........................................................................
    // Allocate table for reading all the OGers.
    UINT32 max_oger_count;
    if (gss_version != SS_NV_VERSION_GT2TB)
    {
        max_oger_count = MAX_OGER_COUNT;
    }
    else
    {
        max_oger_count = MAX_OGER_COUNT_GT2TB;
    }
    N_poger_table = (void *)p_MallocC(sizeof(OGER*) * max_oger_count, __FILE__, __LINE__);

    // Read and restore the ssms -- walks SSMS and reads the OGERs.
    restore_read_ssms(header_nv, nv_oger, N_poger_table);

    UINT8 *N_dog_table = (UINT8 *)p_MallocC(max_oger_count / 8, __FILE__, __LINE__);
    N_dog_table[0] = 0x1;               /* Set snappool header as in use. */

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
            N_poger_table[i]->ogr_leftch = N_poger_table[(int)((unsigned long)N_poger_table[i]->ogr_leftch & 0xffffffff)];
            N_poger_table[i]->ogr_rightch = N_poger_table[(int)((unsigned long)N_poger_table[i]->ogr_rightch & 0xffffffff)];
            N_poger_table[i]->ogr_parent = N_poger_table[(int)((unsigned long)N_poger_table[i]->ogr_parent & 0xffffffff)];
            N_poger_table[i]->ogr_link = N_poger_table[(int)((unsigned long)N_poger_table[i]->ogr_link & 0xffffffff)];
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
    i = capacity % 8;           // Check for partial byte at end of DEF_dog_table.
    j = capacity / 8;           // Get which byte.

    if (i != 0)
    {
        N_dog_table[j] |= (0xff << i);
    }

// ---------------------------------------------------------------------------
//  Copy bitmap into DEF_dog_table.
    bcopy(N_dog_table, DEF_dog_table, max_oger_count / 8);
#ifdef DUMP_OGER_BITMAP
dump_bitmap(N_dog_table, "restore_snap_data", SNAPPOOL);
#endif  /* DUMP_OGER_BITMAP */

    // Get rid of allocated memory.
    p_Free(N_dog_table, max_oger_count/8, __FILE__, __LINE__);
    p_Free(N_poger_table, sizeof(OGER*) * max_oger_count, __FILE__, __LINE__);

    // Check to see that all SSMS added oger counts matches the actual count of OGers restored.
    if (cuml_ssms_oger_cnt != N_restored_ogers)
    {
        // TODO find out which SSMS is missing OGERs, invalidate the SS, Log it, and free the OGers.
        fprintf(stderr, "SSDBG: restore_all_ss_nv:  Holy SSMS! Expected %d, Got %d\n", cuml_ssms_oger_cnt, N_restored_ogers);
    }
    fprintf(stderr, "restore_all_ss_nv:  Found %d OGers\n", N_restored_ogers);
    DEF_oger_cnt = N_restored_ogers;

fprintf(stderr, "%s:%u snappool percent full=%lld\n", __func__, __LINE__, ((N_restored_ogers +(N_restored_ogers!=0)) * 100ULL / (devCap >> SS_SEC2GB_SF)));
}                                      /* End of restore_snap_data */

/* ------------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------------ */
/* The following structure converts an 8 bit value into a numerical representation */
/* of the first unset(cleared) bit number, 0 based. */
static UINT8 dog_nxtopn[] = {
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,7,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,255
};

/* ------------------------------------------------------------------------ */
/*  Allocate an OGer (One Gig-er) structure. Initialize for the snapshot    */
/*  SSMS. OGer is assigned from the OGer pool contained in a single Vdisk   */
/*  that contains all of the OGers.                                         */
/*                                                                          */
/*  INPUT: snappool_vid = Snappool Vid                                                */
/*  OUTPUT: OGer                                                            */
/* ------------------------------------------------------------------------ */
OGER *D$alloc_oger(UINT16 ss_vid);
OGER *D$alloc_oger(UINT16 ss_vid)
{
    UINT64 gb;
    int ix;
    int bitsleft;
    int dogix;
    int percentage;
    struct OGER *new_oger;
    UINT8 bitpos;
    int oger_ordinal;

    gb = gVDX.vdd[ss_vid]->devCap / (2048*1024);                          /* convert sectors to GB. (21 bits gone) */
    bitsleft = gb % 8;                                  /* Check for partial byte */

    ix = gb / 8;
// fprintf(stderr, "%s:%u devCap=%lld, gb=%lld, ix=%d, bitsleft=%d\n", __func__, __LINE__, gVDX.vdd[ss_vid]->devCap, gb, ix, bitsleft);
    if (ix > (MAX_OGER_COUNT/8)) {
        fprintf(stderr,"%s:%u D$alloc_oger ix=%d\n", __func__, __LINE__, ix);
        abort();
    }

    if (bitsleft != 0)                                  /* If remainder */
    {
        /* There is a remainder so we need to mark the top bits of the last byte as used (set). */
        for (; bitsleft < 8; bitsleft++)
        {
            DEF_dog_table[ix] |= 1 << bitsleft;
        }
        ix++;
    }

    for (dogix = 0; dogix < ix; dogix++)
    {
        if (DEF_dog_table[dogix] != 0xff)
        {
            break;
        }
    }
    if (dogix >= ix)
    {
        return(NULL);                                   /* No free OGers were found, exit */
    }

// fprintf(stderr, "%s:%u OGer free DEF_dog_table[%d]=0x%02x\n", __func__, __LINE__, dogix, DEF_dog_table[dogix]);
    /* Found a free OGer, allocate it and return. */
    bitpos = dog_nxtopn[DEF_dog_table[ix]];             /* Compute first clear bit position */
    if (ix & (1 << (bitpos & 0x1f)))                    /*  This is an error condition */
    {
fprintf(stderr, "%s:%u No free OGers were found!\n", __func__, __LINE__);
        return(NULL);                                   /* No free OGers were found, exit */
    }

    ix = ix | (1 << bitpos);
    DEF_dog_table[dogix] = ix;                          /* Update the OGer table */

    oger_ordinal = bitpos * (dogix * 8);                /* OGer ordinal */

    new_oger = calloc(1, sizeof(*new_oger));            /* Assign memory for OGER */
    new_oger->ogr_sda = (UINT64)oger_ordinal * OGERSIZE; /* Calculate SDA */
    new_oger->ogr_segcnt = 0;                           /* Store the number of segments */
// fprintf(stderr,"%s:%u ogr_ord=%d new_oger=calloc()=0x%08x\n", __func__, __LINE__, oger_ordinal, (UINT32)new_oger);
    new_oger->ogr_ord = oger_ordinal;                   /* Store the OGER ordinal */

    DEF_oger_cnt++;

    /* calculate percent of spool used */
    gb = gVDX.vdd[ss_vid]->devCap / (2048*1024);          /* convert sectors to GB. (21 bits gone) */
    if (gb == 0) {
        return(new_oger);
    }
    percentage = ((DEF_oger_cnt + 1) * 100) / gb;

// fprintf(stderr,"%s:%u ALLOC OGER: percentage=%d\n", __func__, __LINE__, percentage);

    return(new_oger);
}   /* End of D$alloc_oge() */


/* ------------------------------------------------------------------------ */
/* #**********************************************************************          */ 
/* #                                                                                */ 
/* #  NAME: DEF_build_slink_structures                                              */ 
/* #                                                                                */ 
/* #  PURPOSE:                                                                      */ 
/* #       Once a SNAPSHOT is created, this routine builds the necessary            */ 
/* #       snapshot structures.                                                     */ 
/* #                                                                                */ 
/* #  INPUT:                                                                        */ 
/* #       g0/g1 = size of SS in sectors                                            */ 
/* #       g2 = ss vid                                                              */ 
/* #       g3 = source vid                                                          */ 
/* #                                                                                */ 
/* #  OUTPUT:                                                                       */ 
/* #       g0 = SSMS                                                                */ 
/* #                                                                                */ 
/* #**********************************************************************          */ 
SSMS *DEF_build_slink_structures(UINT64 devcap, UINT32 ss_vid, UINT32 src_vid);
SSMS *DEF_build_slink_structures(UINT64 devcap, UINT32 ss_vid, UINT32 src_vid)
{
    struct SSMS *new_ssms;
    struct OGER *new_oger;
    UINT32  segment;
    int r4;

    /* Find the next open slot for an ssms. */ 
    for (r4 = 0; r4 < MAX_SNAPSHOT_COUNT; r4++) {
        if (DEF_ssms_table[r4] == 0) {
            break;
        }
    }
    if (r4 >= MAX_SNAPSHOT_COUNT) {
fprintf(stderr,"%s:%u r4=%d  MAX_SNAPSHOT_COUNT=%d\n", __func__, __LINE__, r4, MAX_SNAPSHOT_COUNT);
        return(NULL);
    }

    /* Allocate a SSMS (snapshot management structure). */
    new_ssms = calloc(1, sizeof(*new_ssms));

    DEF_ssms_table[r4] = new_ssms;                  /* Update slot with ssms pointer */
    new_ssms->ssm_ordinal = r4;                     /* Store the ordinal in the SSMS */

    new_ssms->ssm_prefowner = 0;                    /* Store the preferred owner. */

    /* Allocate a region map and dirty all segments. */ 
    UINT32 number_full_rm = devcap >> 32;           /* Number of "extra" (full RM) */
    UINT32 remaining_rm = (devcap & 0xffffffffULL);
    remaining_rm = (remaining_rm + SEGSIZE_SEC - 1) / SEGSIZE_SEC;
    D_init_slrm(remaining_rm, number_full_rm, new_ssms);

    /* Allocate an OGer and associate it with this snapshot. */ 
    new_oger = D$alloc_oger(gnv_oger_array->ogr_vid);

    new_ssms->ssm_frstoger = new_oger;              /* Update the first OGer */
    new_ssms->ssm_tailoger = new_oger;              /* Update tail OGer */

    (void)cal_seg_bit(&segment, devcap/2, devcap/2, devcap, 1);
    new_oger->ogr_sdakey = segment;
    new_oger->ogr_vid = gnv_oger_array->ogr_vid;    /* assign snappool vid to this oger */

    new_ssms->ssm_ogercnt = 1;                      /* Update the OGer count */
fprintf(stderr,"%s:%u CreateSS: SSVID=%d, SRCVID=%d, SSMS=%p OGER=%p sdakey=%d\n", __func__, __LINE__, ss_vid, src_vid, new_ssms, new_oger, segment);

    /* Update snapshot and source SSMS pointers. */
    gVDX.vdd[ss_vid]->scorVID = src_vid;
    new_ssms->ssm_srcvid = src_vid;
    new_ssms->ssm_ssvid = ss_vid;

    gVDX.vdd[ss_vid]->vd_incssms = new_ssms;
// fprintf(stderr, "%s:%u-%s vd_incssms set for %d to %p\n", __FILE__,__LINE__,__func__, ss_vid, new_ssms);

    if (gVDX.vdd[src_vid]->vd_outssms == 0)
    {
// fprintf(stderr, "%s:%u vd_outssms set\n", __func__, __LINE__);
        gVDX.vdd[src_vid]->vd_outssms = new_ssms;
    }
    else
    {
// fprintf(stderr, "%s:%u ssm_link set\n", __func__, __LINE__);
        gVDX.vdd[src_vid]->vd_outssmstail->ssm_link = new_ssms;
        gVDX.vdd[src_vid]->vd_outssmstail = new_ssms;
    }
    gVDX.vdd[ss_vid]->scpComp = 100;

    return(new_ssms);
}   /* end of DEF_build_slink_structures() */

/* ------------------------------------------------------------------------ */
void setup_vdd(UINT16 vid, UINT64 devcap, UINT16 attr, UINT16 rid, UINT8 type, UINT16 sps_or_src_snapshot);
void setup_vdd(UINT16 vid, UINT64 devcap, UINT16 attr, UINT16 rid, UINT8 type, UINT16 sps_or_src_snapshot)
{
    struct VDD *new_vdd;
    struct RDD *new_rdd;

    new_vdd = calloc(1,sizeof(*new_vdd));
    gVDX.vdd[vid] = new_vdd;
    new_vdd->vid = vid;
    new_vdd->devCap = devcap;
    new_vdd->attr = attr;

    new_rdd = calloc(1,sizeof(*new_rdd));
    new_vdd->pRDD = new_rdd;
    new_rdd->rid = rid;
    new_rdd->type = type;
    new_rdd->sps = sps_or_src_snapshot;
    new_rdd->devCap = devcap;
    new_rdd->vid = vid;
}   /* End of setup_vdd() */

/* ------------------------------------------------------------------------ */
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
fprintf(stderr, "D_chk_seg_bit ssm_regmap[%d]->regions[%d >> %d = %d]=%p\n", extra, within, SMBITS2WRD_SF + SMWRD2REG_SF, within >> (SMBITS2WRD_SF + SMWRD2REG_SF), sm);
    if (sm == 0) {
        return(FALSE);                  /* If no segment map */
    }

    Index = (within >> SMBITS2WRD_SF) & SMWRDIDX_MASK;
fprintf(stderr, "D_chk_seg_bit sm->segments[%d](=0x%08x) & (1 << (31-(%d & 0x1f)))=0x%08x = 0x%08x\n", Index, sm->segments[Index], within, (1 << (31-(within & 0x1f))), sm->segments[Index] & (1 << (31-(within & 0x1f))));
    if (sm->segments[Index] & (1 << (31-(within & 0x1f)))) {
        return(TRUE);                   /* segment bit is set */
    }
    return(FALSE);                      /* segment map bit clear */
}   /* End of D_chk_seg_bit */

/* ------------------------------------------------------------------------ */
/*
******************************************************************************
**
**  @brief  Check an OGer slot for occupation
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

/* ------------------------------------------------------------------------ */
/*
******************************************************************************
**
**  @brief  Compute a slot number based on an input key.
**
**          Accepts a key that is applied to the double hash function to determine
**          a slot number. The probe count is used as the scaler to the second
**          hash function. In this case h(k) takes the form:
**              h(k) = ((key mod prime1) + probe_count(key mod prime2))mod 1024
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

/* ------------------------------------------------------------------------ */
/*
******************************************************************************
**
**  @brief  Glue routine to call d$htsearch
**
**  @param  slot - Pointer to UINT32 to get found slot
**  @param  seg  - Segment number
**  @param  ssms - Pointer to SSMS
**
**  @return OGer and slot (above).
**
******************************************************************************
*/

OGER *htsearch(UINT32 *slot, UINT32 seg, SSMS *ssms);
OGER *htsearch(UINT32 *slot, UINT32 seg, SSMS *ssms)
{
// fprintf(stderr, "htsearch(%p, %d, %p)\n", slot, seg, ssms);
    OGER *oger = ssms->ssm_frstoger;        /* Current search OGER */
    int   probe_count;
    int   max_probe_count;

    probe_count = 0;

    /* Loop until the hash function either returns a match or an empty slot. */
    for (;;)
    {
        if (oger == 0)
        {
            *slot = 0;
fprintf(stderr, "htsearch, oger is zero, exiting with oger(%p) and slot=0\n", oger);
            return(oger);                   /* return empty slot */
        }
        max_probe_count = oger->ogr_maxpr;
        for (;;)
        {
            UINT32 slot_found;

            if (probe_count > max_probe_count)
            {
fprintf(stderr, "htsearch, probe_count(%d) > max_probe_count(%d)\n", probe_count, max_probe_count);
                break;                      /* if max probe count exceeded */
            }

            /* Check to see if the slot is filled. */
            slot_found = d_chkslot(d_hash_key(seg, probe_count), oger);
fprintf(stderr, "htsearch, slot_found=0x%08x\n", slot_found);
            if ((slot_found & 0x80000000) == 0)
            {
fprintf(stderr, "htsearch, slot_found=0\n");
                break;                      /* Slot is not occupied, check the next OGer */
            }
            /* Clear the occupied bit */
            slot_found = slot_found & 0x7fffffff;
fprintf(stderr, "htsearch, oger->ogr_sdamap[%d]=%d (match seg(%d)?)\n", slot_found, oger->ogr_sdamap[slot_found], seg);
            if (oger->ogr_sdamap[slot_found] == seg)
            {
                *slot = slot_found;
fprintf(stderr, "htsearch, exiting with oger(%p) and slot=%d\n", oger, slot_found);
                return(oger);
            }
            probe_count++;
        }

        /* Advance to the next OGER */
fprintf(stderr, "htsearch, advance to next oger\n");
        probe_count = 0;                    /* Reinitialize the probe count. */
        if (seg > oger->ogr_sdakey)
        {
fprintf(stderr, "htsearch, seg(%d) > oger->ogr_sdakey(%d) go right to %p\n", seg, oger->ogr_sdakey, oger->ogr_rightch);
            oger = oger->ogr_rightch;       /* Get the right child. */
        }
        else
        {
fprintf(stderr, "htsearch, seg(%d) <= oger->ogr_sdakey(%d) go left to %p\n", seg, oger->ogr_sdakey, oger->ogr_leftch);
            oger = oger->ogr_leftch;        /* Get the left child. */
        }
    }
}   /* End of htsearch */

/* ------------------------------------------------------------------------ */
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

void access_snapshot(SSMS *ssms, UINT16 vid, UINT64 sda);
void access_snapshot(SSMS *ssms, UINT16 vid, UINT64 sda)
{
// print_ssms("From access_snapshot", ssms);
fprintf(stderr, "\naccess_snapshot(%p, %d, %lld)\n", ssms, vid, sda);
    UINT32  seg;
    UINT32  nsegs;

    /*
     *  First determine the starting segment and the number of segments.
     *  Use these numbers to construct each Sync record.
     */
    nsegs = cal_seg_bit(&seg, sda, sda, gVDX.vdd[vid]->devCap, 1);  /* Only 1 block to read at a time */
// fprintf(stderr, "cal_seg_bit returned seg=%d, nsegs=%d\n", seg, nsegs);

    /*
     * For each segment create a sync record unless the segment is already
     * in sync. Also, if there is more than one segment, separate ILT/VRP/SGL
     * combos will need to be constructed for each segment regardless of if
     * there is a sync record for that segment or not.
     */
// fprintf(stderr, "segment_not_populated(%d)\n", seg);
    if (D_chk_seg_bit(seg, ssms))
    {
fprintf(stderr, "Block %lld (segment_not_populated) not in SNAPSHOT, access source, ignoring\n", sda);
        return;
    }

    OGER    *oger = NULL;
    UINT32  slot;

    /*
     * If a sync record exists, we don't want to submit the
     * request to the virtual layer yet.
     */
    oger = htsearch(&slot, seg, ssms);
// print_ssms("htsearch: ", ssms);
    if (!oger)
    {
fprintf(stderr, "Isn't more needed here to handle this?? -- read from source vdisk, slot=%d, seg=%d!\n", slot, seg);
return;
        /* ?? Isn't more needed here to handle this?? */
    }

    UINT32  final_offset;

    final_offset = sda % SEGSIZE_SEC;   /* Sector offset in segment */

fprintf(stderr, "startDiskAddr = %lld  vid=%d\n", oger->ogr_sda + (slot * SEGSIZE_SEC) + final_offset, oger->ogr_vid);
print_data(oger->ogr_sda + (slot * SEGSIZE_SEC) + final_offset);
}   /* End of access_snapshot */

/* ------------------------------------------------------------------------ */
void setup_VDX(void);
void setup_VDX(void)
{
    /*             vid,   devCap,   ATTR, RID,type,sps/src_snapshot */
    /* SNAPPOOL = 100 in #define at beginning. */
    setup_vdd(SNAPPOOL,  4235264, 0x2100,   0,   4, 512);           /* Setup snappool. */

    /*        vid,    devCap,   ATTR, RID,type,sps/src_snapshot */
    setup_vdd(101,   2523136, 0x0100,   1,   4, 512);           /* Setup source vdisk 101. */
    setup_vdd(111,   2523136, 0x0100,   3,   6, 101);           /* Setup snapshot 111 pointing to 101. */

}   /* end of setup_VDX() */

/* ------------------------------------------------------------------------ */
void try_access(UINT16 vid);
void try_access(UINT16 vid)
{
//    UINT64 i;

//    for (i = 0; i < gVDX.vdd[vid]->devCap; i++)
//    {
                                          /* SSMS, VID, SDA */
        access_snapshot(gVDX.vdd[vid]->vd_incssms, vid, 0);
        access_snapshot(gVDX.vdd[vid]->vd_incssms, vid, 2097152);
//    }
}   /* end of try_access */

/* ------------------------------------------------------------------------ */
void restore_ss_nv_task(void)
{
    int j = 111;        /* snapshot that we want to read block from */

    setup_VDX();        /* Set up the gVDX entries for snappool, source, and snapshot. */

    restore_snap_data();            /* SNAPPOOL is the snappool, hard coded in #define */

fprintf(stderr, "------------------------------------------------------------------------------\n");
dump_bitmap(DEF_dog_table, "DEF_dog_table", SNAPPOOL);
fprintf(stderr, " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n");
dump_ssms_table();
fprintf(stderr, "------------------------------------------------------------------------------\n");


/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  */
    j = 111;        /* snapshot that we want to read block from */
// fprintf(stderr, "%s:%u-%s vd_incssms set for %d to %p\n", __FILE__,__LINE__,__func__, j, gVDX.vdd[j]->vd_incssms);
    try_access(111);
}   /* End of restore_ss_nv_task() */

/* ------------------------------------------------------------------------ */
/* End of file snapshottest.c */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
