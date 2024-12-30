#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "isns.h"
#include "nvr.h"
#include "target.h"

/* ------------------------------------------------------------------------ */
extern void NV_P2ChkSum(NVRII *);

/* ------------------------------------------------------------------------ */
#define match(x,y)  (strncmp(x, y, strlen(y)) == 0)
/* ------------------------------------------------------------------------ */
#if defined(BE_ASCII)
static int byte_on_in_file = 0;
#else   /* BE_ASCII */
#endif  /* BE_ASCII */
/* ------------------------------------------------------------------------ */
static char buffer[BUFSIZ];
static char *line = buffer;
/* ------------------------------------------------------------------------ */
static UINT8 T_opt[1024*16];
/* ------------------------------------------------------------------------ */
#if defined(BE_ASCII)
/* Windows sometimes reads less than you wanted, and you have to read again
   to get the next part of the data. Just like a network read. *grumble* */
static int GET_DATA(void *binary, size_t length)
{
    size_t lthread = 0;
    int err;

    while (lthread < length)
    {
        err = read(0, binary, length - lthread);
        if (err < 0)
        {
            return (err);
        }
        lthread += err;
        byte_on_in_file += err;
    }
    return (lthread);
}   /* End of GET_DATA */

/* ------------------------------------------------------------------------ */
/* Get NVRAM Part 2 header for all variable entries. */
static int do_hdr(struct NVRH *hdr)
{
    int             err;

    err = GET_DATA((void *)hdr, sizeof(*hdr));

    if (err == 0)
    {
        return (0);                     /* End of file */
    }
    else if (err != (int)sizeof(*hdr))
    {
        fprintf(stderr, "do_hdr only read %d bytes (wanted %u)\n", err, sizeof(*hdr));
        exit (1);
    }
    return (err);
}                                      /* End of do_hdr */

/* ------------------------------------------------------------------------ */
static void do_string(const char *str)
{
    printf("%s", str);
}   /* End of do_string */

/* ------------------------------------------------------------------------ */
static void do_string_eoln(const char *str)
{
    printf("%s", str);
}   /* End of do_string_eoln */

/* ------------------------------------------------------------------------ */
static void do_16x(UINT64 *i, int j)
{
    while (j > 0)
    {
        printf("%016Lx", *i);
        j--;
        i++;
    }
}   /* End of do_16x */

/* ------------------------------------------------------------------------ */
static void do_8x(UINT32 *i, int j)
{
    while (j > 0)
    {
        printf("%08x", *i);
        j--;
        i++;
    }
}   /* End of do_8x */

/* ------------------------------------------------------------------------ */
static void do_4hx(UINT16 *i, int j)
{
    while (j > 0)
    {
        printf("%04x", *i);
        j--;
        i++;
    }
}   /* End of do_4hx */

/* ------------------------------------------------------------------------ */
static void do_2hhx(UINT8 *i, int j)
{
    while (j > 0)
    {
        printf("%02x", *i);
        j--;
        i++;
    }
}   /* End of do_2hhx */

/* ------------------------------------------------------------------------ */
static void do_16u(UINT64 *i, int j)
{
    while (j > 0)
    {
        printf("%Lu", *i);
        if (j > 1) { printf(" "); }
        j--;
        i++;
    }
}   /* End of do_16u */

/* ------------------------------------------------------------------------ */
static void do_u(UINT32 *i, int j)
{
    while (j > 0)
    {
        printf("%u", *i);
        if (j > 1) { printf(" "); }
        j--;
        i++;
    }
}   /* End of do_u */

/* ------------------------------------------------------------------------ */
static void do_hu(UINT16 *i, int j)
{
    while (j > 0)
    {
        printf("%u", *i);
        if (j > 1) { printf(" "); }
        j--;
        i++;
    }
}   /* End of do_hu */

/* ------------------------------------------------------------------------ */
static void do_hhu(UINT8 *i, int j)
{
    while (j > 0)
    {
        printf("%u", *i);
        if (j > 1) { printf(" "); }
        j--;
        i++;
    }
}   /* End of do_hhu */

/* ------------------------------------------------------------------------ */
static void do_c(UINT8 *i, int j)
{
    while (j > 0)
    {
        fflush(stdout);
        write(1, i, 1);
        line++;
        j--;
        i++;
    }
}   /* End of do_c */
#endif  /* BE_ASCII */

/* ======================================================================== */

/* ------------------------------------------------------------------------ */
#if !defined(BE_ASCII)
static int linecount = 1;

static void GET_LINE(void)
{
    char *c = buffer;
    ssize_t err;

    line = buffer;
    for (;;)
    {
        *c = '\0';
        err = read(0, c, 1);
        if (err == 0)
        {
            fprintf(stderr, "EOF reached on input!\n");
            fprintf(stderr, "linecount=%d\n", linecount);
            exit (1);
        }
        if (err < 0)
        {
            perror("read");
            fprintf(stderr, "linecount=%d\n", linecount);
            exit (1);
        }
        if (err != 1)
        {
            fprintf(stderr, "read: huh, err=%d\n", err);
            fprintf(stderr, "linecount=%d\n", linecount);
            exit (1);
        }
        if (*c == '\n')
        {
            c++;
            *c = '\0';
            linecount++;
            break;
        }
        c++;
    }
// fprintf(stderr, "line=");
// write(2, line, c - buffer -1);
// fprintf(stderr, "\n");
}   /* End of GET_LINE */

/* ------------------------------------------------------------------------ */
static void do_string(const char *str)
{
    if (strncmp(line, str, strlen(str)) == 0)
    {
        line += strlen(str);
        return;
    }
    fprintf(stderr, "do_string did not match (%s):(%s)\n", line, str);
    fprintf(stderr, "linecount=%d\n", linecount);
    abort();
}   /* End of do_string */

/* ------------------------------------------------------------------------ */
static void do_string_eoln(const char *str)
{
    do_string(str);
    if (line[0] != '\0')
    {
        fprintf(stderr, "do_string_eoln is not at eoln (%s):(%s)\n", line, str);
        fprintf(stderr, "linecount=%d\n", linecount);
        abort();
    }
    GET_LINE();
}   /* End of do_string_eoln */

/* ------------------------------------------------------------------------ */
static void do_16x(UINT64 *i, int j)
{
    while (j > 0)
    {
        int lth=-2;
        int err;

        err = sscanf(line, "%16Lx%n", i, &lth);
        if (err != 1 || lth != 16)
        {
            fprintf(stderr, "do_16x error, (%s):(%16Lx)\n", line, *i);
            fprintf(stderr, "linecount=%d\n", linecount);
            abort();
        }
        line += lth;
        j--;
        i++;
    }
}   /* End of do_16x */

/* ------------------------------------------------------------------------ */
static void do_8x(UINT32 *i, int j)
{
    while (j > 0)
    {
        int lth=-2;
        int err;

        err = sscanf(line, "%8x%n", i, &lth);
        if (err != 1 || lth != 8)
        {
            fprintf(stderr, "do_8x error, (%s):(%08x)\n", line, *i);
            fprintf(stderr, "linecount=%d\n", linecount);
            abort();
        }
        line += lth;
        j--;
        i++;
    }
}   /* End of do_8x */

/* ------------------------------------------------------------------------ */
static void do_4hx(UINT16 *i, int j)
{
    while (j > 0)
    {
        int lth=-2;
        int err;

        err = sscanf(line, "%4hx%n", i, &lth);
        if (err != 1 || lth != 4)
        {
            fprintf(stderr, "do_4hx error, (%s):(%04x)\n", line, *i);
            fprintf(stderr, "linecount=%d\n", linecount);
            abort();
        }
        line += lth;
        j--;
        i++;
    }
}   /* End of do_4hx */

/* ------------------------------------------------------------------------ */
static void do_2hhx(UINT8 *i, int j)
{
    while (j > 0)
    {
        int lth=-2;
        int err;

        err = sscanf(line, "%2hhx%n", i, &lth);
        if (err != 1 || lth != 2)
        {
            fprintf(stderr, "do_2hhx error, (%s):(%02x)\n", line, *i);
            fprintf(stderr, "linecount=%d\n", linecount);
            abort();
        }
        line += lth;
        j--;
        i++;
    }
}   /* End of do_2hhx */

/* ------------------------------------------------------------------------ */
static void do_16u(UINT64 *i, int j)
{
    while (j > 0)
    {
        int lth=-2;
        int err;

        err = sscanf(line, "%Lu%n", i, &lth);
        if (err != 1 || lth <= 0)
        {
            fprintf(stderr, "do_16u error, (%s):(%Lu)\n", line, *i);
            fprintf(stderr, "linecount=%d\n", linecount);
            abort();
        }
        line += lth;
        j--;
        i++;
    }
}   /* End of do_16u */

/* ------------------------------------------------------------------------ */
static void do_u(UINT32 *i, int j)
{
    while (j > 0)
    {
        int lth=-2;
        int err;

        err = sscanf(line, "%u%n", i, &lth);
        if (err != 1 || lth <= 0)
        {
            fprintf(stderr, "do_u error, (%s):(%u)\n", line, *i);
            fprintf(stderr, "linecount=%d\n", linecount);
            abort();
        }
        line += lth;
        j--;
        i++;
    }
}   /* End of do_u */

/* ------------------------------------------------------------------------ */
static void do_hu(UINT16 *i, int j)
{
    while (j > 0)
    {
        int lth=-2;
        int err;

        err = sscanf(line, "%hu%n", i, &lth);
        if (err != 1 || lth <= 0)
        {
            fprintf(stderr, "do_hu error, (%s):(%u)\n", line, *i);
            fprintf(stderr, "linecount=%d\n", linecount);
            abort();
        }
        line += lth;
        j--;
        i++;
    }
}   /* End of do_hu */

/* ------------------------------------------------------------------------ */
static void do_hhu(UINT8 *i, int j)
{
    while (j > 0)
    {
        int lth=-2;
        int err;

        err = sscanf(line, "%hhu%n", i, &lth);
        if (err != 1 || lth <= 0)
        {
            fprintf(stderr, "do_hhu error, (%s):(%u)\n", line, *i);
            fprintf(stderr, "linecount=%d\n", linecount);
            abort();
        }
        line += lth;
        j--;
        i++;
    }
}   /* End of do_hhu */

/* ------------------------------------------------------------------------ */
static void do_c(UINT8 *i, int j)
{
    while (j > 0)
    {
        *i = line[0];
        line++;
        j--;
        i++;
    }
}   /* End of do_c */
#endif  /* !BE_ASCII */

/* ------------------------------------------------------------------------ */
#if defined(BE_ASCII)
#define GET_LINE()
#else   /* !BE_ASCII */
#define GET_DATA(x,y)
#endif  /* !BE_ASCII */

/* ------------------------------------------------------------------------ */
static struct NVR *do_NVRII(struct NVRII *nvrii)
{
    GET_LINE();
    GET_DATA((void *)nvrii, sizeof(*nvrii));

    do_string_eoln("NVRII\n");

// NOTE: Must fix checksum later.
    do_string("  cSum2            = 0x");
    do_8x(&nvrii->cSum2, 1);
    do_string_eoln("\t\t\tChecksum Part II\n");

    do_string("  rsvd4[4]         = 0x");
    do_2hhx(&nvrii->rsvd4[0], 4);
    do_string_eoln("\t\t\tReserved\n");

    do_string("  vers             = 0x");
    do_4hx(&nvrii->vers, 1);
    do_string_eoln("\t\t\tVersion\n");

    do_string("  rev              = 0x");
    do_4hx(&nvrii->rev, 1);
    do_string_eoln("\t\t\tRevision\n");

    do_string("  length           = ");
    do_u(&nvrii->length, 1);
    do_string_eoln("\t\t\tLength of entire structure\n");

    do_string("  magic            = 0x");
    do_4hx(&nvrii->magic, 1);
    do_string_eoln("\t\t\tMagic number\n");

    do_string("  revision         = ");
    do_hhu(&nvrii->revision, 1);
    do_string_eoln("\t\t\t\tNVRAM PII revision\n");

    do_string("  defLabel         = ");
    do_hhu(&nvrii->defLabel, 1);
    do_string_eoln("\t\t\t\tDefault label\n");

    do_string("  seq              = ");
    do_u(&nvrii->seq, 1);
    do_string_eoln("\t\t\t\tSequence number\n");

    do_string("  gPri             = ");
    do_hhu(&nvrii->gPri, 1);
    do_string_eoln("\t\t\t\tGlobal priority\n");

    do_string("  rsvd25           = 0x");
    do_2hhx(&nvrii->rsvd25, 1);
    do_string_eoln("\t\t\tReserved\n");

    do_string("  ften             = 0x");
    do_2hhx(&nvrii->ften, 1);
    do_string_eoln("\t\t\tForeign target enable map\n");

    do_string("  rsvd27           = 0x");
    do_2hhx(&nvrii->rsvd27, 1);
    do_string_eoln("\t\t\tReserved\n");

    do_string("  vcgID            = ");
    do_u(&nvrii->vcgID, 1);
    do_string_eoln("\t\t\tVirtual controller group ID\n");

    do_string("  whql             = ");
    do_hhu(&nvrii->whql, 1);
    do_string_eoln("\t\t\t\twhql compliance enable (T/F)\n");

    do_string("  rsvd33[11]       = 0x");
    do_2hhx(&nvrii->rsvd33[0], 11);
    do_string_eoln("\tReserved\n");

    do_string("  scrubOpt         = ");
    do_hhu(&nvrii->scrubOpt, 1);
    do_string_eoln("\t\t\t\tScrubbing enable (T/F)\n");

    do_string("  glCache          = ");
    do_hhu(&nvrii->glCache, 1);
    do_string_eoln("\t\t\t\tGlobal cache enable (T/F)\n");

    do_string("  glVdPriority     = ");
    do_hhu(&nvrii->glVdPriority, 1);
    do_string_eoln("\t\t\t\tGlobal VDisk Priority enable (T/F)\n");

    do_string("  glPdAutoFailback = ");
    do_hhu(&nvrii->glPdAutoFailback, 1);
    do_string_eoln("\t\t\t\tGlobal VDisk Priority enable (T/F)\n");

    do_string("  name[16]         = '");
    do_c(&nvrii->name[0], 16);
    do_string_eoln("'\t\tVCG name\n");

    return ((struct NVR *)(nvrii + 1));
}                                      /* End of do_NVRII */

/* ------------------------------------------------------------------------ */
/* Get and print NVRP structure. */
static struct NVR *do_NVRP(struct NVR *nvr)
{
    struct NVRP    *phys = &nvr->u.phys;

    GET_DATA((void *)phys, sizeof(*phys));

    do_string("NVRP type=0x");
    do_2hhx(&nvr->hdr.recType, 1);
    do_string(" status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  pid              = ");
    do_hu(&phys->pid, 1);
    do_string_eoln("\t\t\t\tPhysical drive ID\n");

    do_string("  class            = ");
    do_hhu(&phys->class, 1);
    do_string_eoln("\t\t\t\tDevice class\n");

    do_string("  channel          = ");
    do_hhu(&phys->channel, 1);
    do_string_eoln("\t\t\t\tChannel device is installed in\n");

    do_string("  fcID             = ");
    do_u(&phys->fcID, 1);
    do_string_eoln("\t\t\t\tFibre channel ID\n");

    do_string("  sSerial          = ");
    do_u(&phys->sSerial, 1);
    do_string_eoln("\t\t\tSystem serial number\n");

    do_string("  prodID[16]       = '");
    do_c(&phys->prodID[0], 16);
    do_string_eoln("'\t\tProduct ID\n");

    do_string("  vendID[8]        = '");
    do_c(&phys->vendID[0], 8);
    do_string_eoln("'\t\t\tVendor ID\n");

    do_string("  serial[12]       = '");
    do_c(&phys->serial[0], 12);
    do_string_eoln("'\t\tSerial number\n");

    do_string("  wwn              = ");
    do_16x(&phys->wwn, 1);
    do_string_eoln("\t\tWorld wide name\n");

    do_string("  lun              = ");
    do_u(&phys->lun, 1);
    do_string_eoln("\t\t\t\tLogical unit number\n");

    do_string("  miscStat         = 0x");
    do_2hhx(&phys->miscStat, 1);
    do_string_eoln("\t\t\tMiscellaneous status from PDD\n");

    do_string("  dName[4]         = ");
    do_c(&phys->dName[0], 2);
    do_hhu(&phys->dName[2], 1);
    do_string("-");
    do_hhu(&phys->dName[3], 1);
    do_string_eoln("\t\t\tPositioning information\n");

    do_string("  hsDName[4]       = ");
    do_c(&phys->hsDName[0], 2);
    do_hhu(&phys->hsDName[2], 1);
    do_string("-");
    do_hhu(&phys->hsDName[3], 1);
    do_string_eoln("\t\t\tPosition info if used hot spare\n");

    do_string("  geoLocation      = ");
    do_hhu(&phys->geoLocation, 1);
    do_string_eoln("\t\t\t\tGEO Location ID\n");

    do_string("  flags            = 0x");
    do_hhu(&phys->flags, 1);
    do_string_eoln("\n");

    do_string("  rsvd53[5]        = 0x");
    do_2hhx(&phys->rsvd53[0], 5);
    do_string_eoln("\tReserved\n");

    nvr->hdr.recLen = (sizeof(NVRH) + sizeof(NVRP) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRP */

/* ------------------------------------------------------------------------ */
/* Get and print NVRSX structure. */
static NVX *do_NVRSX(struct NVX *nvxrec)
{
    struct NVRSX *nvrsx = &nvxrec->u.serv;

    GET_DATA((void *)nvrsx, sizeof(*nvrsx));

    do_string_eoln("NVRSX\n");

    do_string("  vid              = ");
    do_hu(&nvrsx->vid, 1);
    do_string_eoln("\t\t\t\tVirtual device ID\n");

    do_string("  lun              = ");
    do_hu(&nvrsx->lun, 1);
    do_string_eoln("\t\t\t\tLUN the VID is mapped onto\n");

    return ((NVX *)((UINT32)nvxrec + sizeof(NVRSX)));
}   /* End of do_NVRSX */

/* ------------------------------------------------------------------------ */
/* Get and print NVRSX2 structure. */
static NVX *do_NVRSX2(struct NVX *nvxrec)
{
    struct NVRSX2 *nvrsx2 = &nvxrec->u.serv2;

    GET_DATA((void *)nvrsx2, sizeof(*nvrsx2));

    do_string_eoln("NVRSX2\n");

    do_string("  i_name[256]      = '");
    do_c(&nvrsx2->i_name[0], 256);
    do_string_eoln("'    iSCSI Server Name\n");

    return ((NVX *)((UINT32)nvxrec + sizeof(NVRSX2)));
}   /* End of do_NVRSX2 */

/* ------------------------------------------------------------------------ */
/* Get and print NVRS structure. */
static struct NVR *do_NVRS(struct NVR *nvr)
{
    struct NVRS *serv = &nvr->u.serv;
    struct NVX *nvxrec;
    int i;

    GET_DATA((void *)serv, sizeof(*serv));

    do_string("NVRS status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  sid              = ");
    do_hu(&serv->sid, 1);
    do_string_eoln("\t\t\t\tServer ID\n");

    do_string("  nLuns            = ");
    do_hu(&serv->nLuns, 1);
    do_string_eoln("\t\t\t\tNumber of LUNs mapped in server\n");

    do_string("  tid              = ");
    do_hu(&serv->tid, 1);
    do_string_eoln("\t\t\t\tTarget server is mapped to\n");

    do_string("  stat             = 0x");
    do_2hhx(&serv->stat, 1);
    do_string_eoln("\t\t\tStatus\n");

    do_string("  pri              = ");
    do_hhu(&serv->pri, 1);
    do_string_eoln("\t\t\t\tServer priority\n");

    do_string("  owner            = ");
    do_u(&serv->owner, 1);
    do_string_eoln("\t\t\tOwning controller\n");

    do_string("  wwn              = 0x");
    do_16x(&serv->wwn, 1);
    do_string_eoln("\t\tWorld wide name\n");

    do_string("  attrib           = 0x");
    do_8x(&serv->attrib, 1);
    do_string_eoln("\t\t\tServer attributes\n");

    do_string("  linkedSID        = 0x");
    do_4hx(&serv->linkedSID, 1);
    do_string_eoln("\t\t\tLinked Server ID\n");

    do_string("  rsvd30[2]        = 0x");
    do_2hhx(&serv->rsvd30[0], 2);
    do_string_eoln("\t\t\tReserved\n");

    do_string("  name[16]         = '");
    do_c(&serv->name[0], 16);
    do_string_eoln("'\tName\n");

    nvxrec = (NVX *)((UINT32)nvr + sizeof(NVRS) + sizeof(NVRH));

    /* Process extended server (NVRSX) records */
    for (i = 0; i < serv->nLuns; i++)
    {
        nvxrec = do_NVRSX(nvxrec);
    }
    
    if (BIT_TEST(T_opt[serv->tid], TARGET_ISCSI))
    {
        /* Process extended server (iSCSI) (NVRSX2) records */
        nvxrec = do_NVRSX2(nvxrec);
    }

    nvr->hdr.recLen = (((UINT32)nvxrec - (UINT32)nvr) + 0xF) & 0xFFF0;
    return ((struct NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRS */

/* ------------------------------------------------------------------------ */
/* Get and print NVRRX structure. */
static NVX *do_NVRRX(struct NVX *nvxrec, int *i)
{
    struct NVRRX *nvrrx = &nvxrec->u.raid;
    int psd = *i;

    GET_DATA((void *)nvrrx, sizeof(*nvrrx));

    do_string("NVRRX pid ");
    do_u(&psd, 1);
    do_string_eoln("\n");

    do_string("  pid              = ");
    do_hu(&nvrrx->pid, 1);
    do_string_eoln("\t\t\t\tPhysical device ID\n");

    do_string("  status           = 0x");
    do_2hhx(&nvrrx->status, 1);
    do_string_eoln("\t\t\tPSD status\n");

    do_string("  aStatus          = 0x");
    do_2hhx(&nvrrx->aStatus, 1);
    do_string_eoln("\t\t\tAdditional status\n");

    do_string("  sda              = ");
    do_u(&nvrrx->sda, 1);
    do_string_eoln("\t\t\tStarting disk address of PSD\n");

    return ((struct NVX *)((UINT32)nvxrec + sizeof(NVRRX)));
}   /* End of do_NVRRX */

/* ------------------------------------------------------------------------ */
/* Get and print NVRR structure. */
static struct NVR *do_NVRR(struct NVR *nvr)
{
    struct NVRR *raid = &nvr->u.raid;
    struct NVX *nvxrec;
    int i;

    GET_DATA((void *)raid, sizeof(*raid));

    do_string("NVRR status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  rid              = ");
    do_hu(&raid->rid, 1);
    do_string_eoln("\t\t\t\tRAID device ID\n");

    do_string("  type             = ");
    do_hhu(&raid->type, 1);
    do_string_eoln("\t\t\t\tRAID device type\n");

    do_string("  depth            = ");
    do_hhu(&raid->depth, 1);
    do_string_eoln("\t\t\t\tRAID device depth\n");

    do_string("  vid              = ");
    do_hu(&raid->vid, 1);
    do_string_eoln("\t\t\t\tVDisk ID to which RAID belongs\n");

    do_string("  devCount         = ");
    do_hu(&raid->devCount, 1);
    do_string_eoln("\t\t\t\tNumber of physical devices in RAID\n");

    do_string("  sps              = ");
    do_u(&raid->sps, 1);
    do_string_eoln("\t\t\tSectors per stripe\n");

    do_string("  devCap           = ");
    do_16u(&raid->devCap, 1);
    do_string_eoln("\t\tDevice capacity\n");

    do_string("  spu              = ");
    do_u(&raid->spu, 1);
    do_string_eoln("\t\t\t\tSectors per unit\n");

    do_string("  sLen             = ");
    do_u(&raid->sLen, 1);
    do_string_eoln("\t\t\tSegment length\n");

    do_string("  rLen             = ");
    do_u(&raid->rLen, 1);
    do_string_eoln("\t\t\t\tRebuild len for rebuild in progress\n");

    do_string("  aStatus          = 0x");
    do_2hhx(&raid->aStatus, 1);
    do_string_eoln("\t\t\tAdditional status\n");

    do_string("  rsvd[3]          = 0x");
    do_2hhx(&raid->rsvd[0], 3);
    do_string_eoln("\t\t\tReserved\n");

    do_string("  notMirrorCSN     = ");
    do_u(&raid->notMirrorCSN, 1);
    do_string_eoln("\t\t\t\tNot Mirroring Controller Serial Number\n");

    do_string("  owner            = ");
    do_u(&raid->owner, 1);
    do_string_eoln("\t\t\tOwning controller at time of save\n");

    /* Process extended raid (NVRRX) records */
    nvxrec = (NVX *)((UINT32)nvr + sizeof(NVRR) + sizeof(NVRH));

    for (i = 0; i < raid->devCount; i++)
    {
        nvxrec = do_NVRRX(nvxrec, &i);
    }

    nvr->hdr.recLen = (((UINT32)nvxrec - (UINT32)nvr) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRR */

/* ------------------------------------------------------------------------ */
/* Get and print NVRRX structure. */
static NVX *do_NVRRX_GT2TB(struct NVX *nvxrec, int *i)
{
    struct NVRRX_GT2TB *nvrrx = &nvxrec->u.raidGT2TB;
    int psd = *i;

    GET_DATA((void *)nvrrx, sizeof(*nvrrx));

    do_string("NVRRX_GT2TB pid ");
    do_u(&psd, 1);
    do_string_eoln("\n");

    do_string("  pid              = ");
    do_hu(&nvrrx->pid, 1);
    do_string_eoln("\t\t\t\tPhysical device ID\n");

    do_string("  status           = 0x");
    do_2hhx(&nvrrx->status, 1);
    do_string_eoln("\t\t\tPSD status\n");

    do_string("  aStatus          = 0x");
    do_2hhx(&nvrrx->aStatus, 1);
    do_string_eoln("\t\t\tAdditional status\n");

    do_string("  sda              = ");
    do_16u(&nvrrx->sda, 1);
    do_string_eoln("\t\t\tStarting disk address of PSD\n");

    return ((struct NVX *)((UINT32)nvxrec + sizeof(NVRRX_GT2TB)));
}   /* End of do_NVRRX_GT2TB */

/* ------------------------------------------------------------------------ */
/* Get and print NVRR structure. */
static struct NVR *do_NVRR_GT2TB(struct NVR *nvr)
{
    struct NVRR_GT2TB *raid = &nvr->u.raidGT2TB;
    struct NVX *nvxrec;
    int i;

    GET_DATA((void *)raid, sizeof(*raid));

    do_string("NVRR_GT2TB status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  rid              = ");
    do_hu(&raid->rid, 1);
    do_string_eoln("\t\t\t\tRAID device ID\n");

    do_string("  type             = ");
    do_hhu(&raid->type, 1);
    do_string_eoln("\t\t\t\tRAID device type\n");

    do_string("  depth            = ");
    do_hhu(&raid->depth, 1);
    do_string_eoln("\t\t\t\tRAID device depth\n");

    do_string("  vid              = ");
    do_hu(&raid->vid, 1);
    do_string_eoln("\t\t\t\tVDisk ID to which RAID belongs\n");

    do_string("  devCount         = ");
    do_hu(&raid->devCount, 1);
    do_string_eoln("\t\t\t\tNumber of physical devices in RAID\n");

    do_string("  sps              = ");
    do_u(&raid->sps, 1);
    do_string_eoln("\t\t\tSectors per stripe\n");

    do_string("  devCap           = ");
    do_16u(&raid->devCap, 1);
    do_string_eoln("\t\tDevice capacity\n");

    do_string("  spu              = ");
    do_u(&raid->spu, 1);
    do_string_eoln("\t\t\t\tSectors per unit\n");

    do_string("  sLen             = ");
    do_16u(&raid->sLen, 1);
    do_string_eoln("\t\t\tSegment length\n");

    do_string("  aStatus          = 0x");
    do_2hhx(&raid->aStatus, 1);
    do_string_eoln("\t\t\tAdditional status\n");

    do_string("  rsvd[3]          = 0x");
    do_2hhx(&raid->rsvd[0], 3);
    do_string_eoln("\t\t\tReserved\n");

    do_string("  notMirrorCSN     = ");
    do_u(&raid->notMirrorCSN, 1);
    do_string_eoln("\t\t\t\tNot Mirroring Controller Serial Number\n");

    do_string("  owner            = ");
    do_u(&raid->owner, 1);
    do_string_eoln("\t\t\tOwning controller at time of save\n");

    /* Process extended raid (NVRRX) records */
    nvxrec = (NVX *)((UINT32)nvr + sizeof(NVRR_GT2TB) + sizeof(NVRH));

    for (i = 0; i < raid->devCount; i++)
    {
        nvxrec = do_NVRRX_GT2TB(nvxrec, &i);
    }

    nvr->hdr.recLen = (((UINT32)nvxrec - (UINT32)nvr) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRR_GT2TB */

/* ------------------------------------------------------------------------ */
/* Get and print NVRVX1 structure. */
static struct NVX *do_NVRVX1(struct NVX *nvxrec)
{
    struct NVRVX1 *nvrvx1 = &nvxrec->u.virt1;

    GET_DATA((void *)nvrvx1, sizeof(*nvrvx1));

    do_string_eoln("NVRVX1\n");

    do_string("  rid              = ");
    do_hu(&nvrvx1->rid, 1);
    do_string_eoln("\t\t\t\tRAID device ID\n");

    return ((struct NVX *)((UINT32)nvxrec + sizeof(NVRVX1)));
}   /* End of do_NVRVX1 */

/* ------------------------------------------------------------------------ */
/* Get and print NVRVX3 structure. */
static struct NVX *do_NVRVX3(struct NVX *nvxrec)
{
    struct NVRVX3 *nvrvx3 = &nvxrec->u.virt3;

    GET_DATA((void *)nvrvx3, sizeof(*nvrvx3));

    do_string_eoln("NVRVX3\n");

    do_string("  srcSN            = ");
    do_u(&nvrvx3->srcSN, 1);
    do_string_eoln("\t\t\t\tSource controller serial number\n");

    do_string("  srcCluster       = ");
    do_hhu(&nvrvx3->srcCluster, 1);
    do_string_eoln("\t\t\t\tSource controller cluster number\n");

    do_string("  srcVDisk         = ");
    do_hhu(&nvrvx3->srcVDisk, 1);
    do_string_eoln("\t\t\t\tSource controller vdisk number\n");

    do_string("  attr             = ");
    do_hhu(&nvrvx3->attr, 1);
    do_string_eoln("\t\t\t\tAttributes\n");

    do_string("  poll             = ");
    do_hhu(&nvrvx3->poll, 1);
    do_string_eoln("\t\t\t\tVLink poll timer count\n");

    do_string("  repVID           = ");
    do_hu(&nvrvx3->repVID, 1);
    do_string_eoln("\t\t\t\tReported VDisk number\n");

    do_string("  name             = ");
    do_c(&nvrvx3->name[0], 52);
    do_string_eoln("\t\t\t\tName\n");

    do_string("  agnt             = ");
    do_u(&nvrvx3->agnt, 1);
    do_string_eoln("\t\t\t\tAgent serial number\n");

    return ((struct NVX *)((UINT32)nvxrec + sizeof(NVRVX3)));
}   /* End of do_NVRVX2 */

/* ------------------------------------------------------------------------ */
/* Get and print NVRV structure. */
static struct NVR *do_NVRV(struct NVR *nvr)
{
    struct NVRV *virt = &nvr->u.virt;
    struct NVX *nvxrec;
    int i;
    UINT8 bitfield;

    GET_DATA((void *)virt, sizeof(*virt));

    do_string("NVRV status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  vid              = ");
    do_hu(&virt->vid, 1);
    do_string_eoln("\t\t\t\tVDisk device ID\n");

    do_string("  dRaidCnt         = ");
    do_hhu(&virt->dRaidCnt, 1);
    do_string_eoln("\t\t\t\tVDisk deferred RAID count\n");

    do_string("  raidCnt          = ");
    do_hhu(&virt->raidCnt, 1);
    do_string_eoln("\t\t\t\tVDisk RAID count\n");

    do_string("  devCap           = ");
    do_16u(&virt->devCap, 1);
    do_string_eoln("\t\t\tDevice capacity\n");

    do_string("  attr             = 0x");
    do_4hx(&virt->attr, 1);
    do_string_eoln("\t\t\tVDisk attribute\n");

    do_string("  vlarCnt          = ");
    do_hhu(&virt->vlarCnt, 1);
    do_string_eoln("\t\t\t\tVDisk VLAR count\n");

//    GR_GeoRaidNvrInfo grInfo;       /* Georaid information */
// typedef struct GR_GeoRaidNvrInfo
// {
    do_string("  vdOpState:3      = 0x");
    bitfield = virt->grInfo.vdOpState;
    do_2hhx(&bitfield, 1);
    virt->grInfo.vdOpState = bitfield;
    do_string_eoln("\n");

    do_string("  permFlags:4      = 0x");
    bitfield = virt->grInfo.permFlags;
    do_2hhx(&bitfield, 1);
    virt->grInfo.permFlags = bitfield;
    do_string_eoln("\n");

    do_string("  rsvd     :1      = 0x");
    bitfield = virt->grInfo.rsvd;
    do_2hhx(&bitfield, 1);
    virt->grInfo.rsvd = bitfield;
    do_string_eoln("\n");

// } GR_GeoRaidNvrInfo;
    do_string("  rsvd16[3]        = 0x");
    do_2hhx(&virt->rsvd16[0], 3);
    do_string_eoln("\n");

    do_string("  breakTime        = ");
    do_u(&virt->breakTime, 1);
    do_string_eoln("\t\t\t\tMirror Break Time\n");

    do_string("  createTime       = ");
    do_u(&virt->createTime, 1);
    do_string_eoln("\t\t\tVDisk create time\n");

    do_string("  priority         = ");
    do_hhu(&virt->priority, 1);
    do_string_eoln("\t\t\t\tPriority of this particular Vdisk\n");

    do_string("  name[16]         = '");
    do_c(&virt->name[0], 16);
    do_string_eoln("'\tVdisk name\n");

    /* Process extended raid (NVRRX) records */
    nvxrec = (NVX *)((UINT32)nvr + sizeof(NVRR) + sizeof(NVRH));

    for (i = 0; i < virt->raidCnt; i++)
    {
        nvxrec = do_NVRVX1(nvxrec);
    }

// don't do deferred raids.
// Now need VLARs.
    for (i = 0; i < virt->vlarCnt; i++)
    {
        nvxrec = do_NVRVX3(nvxrec);
    }

// printf("nvr->hdr.recLen=0x%x (%d)\n", nvr->hdr.recLen, nvr->hdr.recLen);
    nvr->hdr.recLen = (((UINT32)nvxrec - (UINT32)nvr) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRV */

/* ------------------------------------------------------------------------ */
/* Get and print NVRTX1 structure. */
static struct NVX *do_NVRTX1(struct NVX *nvxrec)
{
    struct NVRTX1 *nvrtx1 = &nvxrec->u.chapInfo;

    GET_DATA((void *)nvrtx1, sizeof(*nvrtx1));

    do_string_eoln("NVRTX1\n");

    do_string("  sname[256]               = '");
    do_c(&nvrtx1->sname[0], 256);
    do_string_eoln("'\n");

    do_string("  secret1[32]               = '");
    do_c(&nvrtx1->secret1[0], 32);
    do_string_eoln("'\n");

    do_string("  secret2[32]               = '");
    do_c(&nvrtx1->secret2[0], 32);
    do_string_eoln("'\n");

    return ((struct NVX *)((UINT32)nvxrec + sizeof(NVRTX1)));
}   /* End of do_NVRVX1 */

/* ------------------------------------------------------------------------ */
/* Get and print NVRTX structure. */
static struct NVX *do_NVRTX(struct NVX *nvxrec)
{
    struct NVRTX *nvrtx = &nvxrec->u.targ;
    unsigned int i;

    GET_DATA((void *)nvrtx, sizeof(*nvrtx));

    do_string_eoln("NVRTX\n");

    struct I_TGD *i_tgd = &nvrtx->i_tgd;

    do_string("  tid              = 0x");
    do_4hx(&i_tgd->tid, 1);
    do_string_eoln("\t\t\ttarget id\n");

    do_string("  ipAddr           = 0x");
    do_8x(&i_tgd->ipAddr, 1);
    do_string_eoln("\t\t\tip address\n");

    do_string("  ipMask           = 0x");
    do_8x(&i_tgd->ipMask, 1);
    do_string_eoln("\t\t\tip subnet mask\n");

    do_string("  ipGw             = 0x");
    do_8x(&i_tgd->ipGw, 1);
    do_string_eoln("\t\t\tip gateway\n");

    do_string("  maxConnections   = ");
    do_hu(&i_tgd->maxConnections, 1);
    do_string_eoln("\t\t\t\tmaximum number connection iSCSI session can have\n");

    do_string("  initialR2T       = ");
    do_hhu(&i_tgd->initialR2T, 1);
    do_string_eoln("\t\t\t\tif yes then target does not support unsolicited data\n");

    do_string("  immediateData    = ");
    do_hhu(&i_tgd->immediateData, 1);
    do_string_eoln("\t\t\t\twhich controls the unsolicited data\n");

    do_string("  dataSequenceInOrder = ");
    do_hhu(&i_tgd->dataSequenceInOrder, 1);
    do_string_eoln("\t\t\tif yes data offset must be in sequence\n");

    do_string("  dataPDUInOrder   = ");
    do_hhu(&i_tgd->dataPDUInOrder, 1);
    do_string_eoln("\t\t\t\tif yes data PDU sequence must be in non-decreasing sequence\n");

    do_string("  ifMarker         = ");
    do_hhu(&i_tgd->ifMarker, 1);
    do_string_eoln("\t\t\t\tused to turn on/off marker from target to initiator\n");

    do_string("  ofMarker         = ");
    do_hhu(&i_tgd->ofMarker, 1);
    do_string_eoln("\t\t\t\tused to turn on/off marker from initiator to target\n");

    do_string("  errorRecoveryLevel = ");
    do_hhu(&i_tgd->errorRecoveryLevel, 1);
    do_string_eoln("\t\t\tError Recovery Level\n");

    do_string("  targetPortalGroupTag = 0x");
    do_4hx(&i_tgd->targetPortalGroupTag, 1);
    do_string_eoln("\t\t\tdeclared by target, by default it is same as tid\n");

    do_string("  maxBurstLength   = ");
    do_u(&i_tgd->maxBurstLength, 1);
    do_string_eoln("\t\t\tmaximum SCSI data payload in bytes in a data sequence\n");

    do_string("  firstBurstLength = ");
    do_u(&i_tgd->firstBurstLength, 1);
    do_string_eoln("\t\t\tmaximum amount in bytes for unsolicited data\n");

    do_string("  defaultTime2Wait = ");
    do_hu(&i_tgd->defaultTime2Wait, 1);
    do_string_eoln("\t\t\t\tminimum seconds to wait after unexpected connection termination\n");

    do_string("  defaultTime2Retain = ");
    do_hu(&i_tgd->defaultTime2Retain, 1);
    do_string_eoln("\t\t\tmaximum seconds after Time2Wait before reassignment is possible\n");

    do_string("  maxOutstandingR2T = ");
    do_hu(&i_tgd->maxOutstandingR2T, 1);
    do_string_eoln("\t\t\t\tmaximum number of outstanding R2T per task\n");

    do_string("  maxRecvDataSegmentLength = ");
    do_u(&i_tgd->maxRecvDataSegmentLength, 1);
    do_string_eoln("\t\tmaximum Data to receive in a PDU\n");

    do_string("  ifMarkInt        = ");
    do_hu(&i_tgd->ifMarkInt, 1);
    do_string_eoln("\t\t\t\tvalue of interval for ifMarker\n");

    do_string("  ofMarkInt        = ");
    do_hu(&i_tgd->ofMarkInt, 1);
    do_string_eoln("\t\t\t\tvalue of interval for ofMarker\n");

    do_string("  headerDigest     = 0x");
    do_hhu(&i_tgd->headerDigest, 1);
    do_string_eoln("\t\t\tdigest type to use for Header\n");

    do_string("  dataDigest       = 0x");
    do_hhu(&i_tgd->dataDigest, 1);
    do_string_eoln("\t\t\tdigest type to use for Data\n");

    do_string("  authMethod       = 0x");
    do_2hhx(&i_tgd->authMethod, 1);
    do_string_eoln("\t\t\tfor authentication method\n");

    do_string("  mtuSize          = ");
    do_u(&i_tgd->mtuSize, 1);
    do_string_eoln("\t\t\tMaximum Transfer Unit for jumbo frames\n");

    do_string("  tgtAlias[32]     = ");
    do_c(&i_tgd->tgtAlias[0], 32);
    do_string_eoln("\ttarget alias\n");

    do_string("  maxSendDataSegmentLength = ");
    do_u(&i_tgd->maxSendDataSegmentLength, 1);
    do_string_eoln("\t\tinitiator side maxRecvDataSegmentLength\n");

    do_string("  numUsers         = ");
    do_u(&i_tgd->numUsers, 1);
    do_string_eoln("\t\t\t\tTotal number of initiators configured for CHAP authentication\n");

    do_string("  *chapInfo        = 0x");
    do_8x((int *)&i_tgd->chapInfo, 1);
    do_string_eoln("\t\t\tPointer to CHAP information\n");

    do_string("  rsvd1[2]         = 0x");
    do_2hhx(&i_tgd->rsvd1[0], 2);
    do_string_eoln("\t\t\tReserved\n");

/* Need to use i_tgd->numUsers to get number of NVRTX1 records. */
    nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRTX));

    for (i = 0; i < i_tgd->numUsers; i++)
    {
        nvxrec = do_NVRTX1(nvxrec);
    }

    return ((NVX *)nvxrec);
}   /* End of do_NVRTX */

/* ------------------------------------------------------------------------ */
/* Get and print NVRT structure. */
static struct NVR *do_NVRT(struct NVR *nvr)
{
    struct NVRT *targ = &nvr->u.targ;
    struct NVX *nvxrec;

    GET_DATA((void *)targ, sizeof(*targ));

    do_string("NVRT status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  tid              = 0x");
    do_4hx(&targ->tid, 1);
    do_string_eoln("\t\t\tTarget ID\n");

    do_string("  port             = ");
    do_hhu(&targ->port, 1);
    do_string_eoln("\t\t\t\tPort mapped onto\n");

    do_string("  opt              = 0x");
    do_2hhx(&targ->opt, 1);
    do_string_eoln("\t\t\tOptions\n");

    T_opt[targ->tid] = targ->opt;

    do_string("  fcid             = 0x");
    do_2hhx(&targ->fcid, 1);
    do_string_eoln("\t\t\tFibre channel ID\n");

    do_string("  rsvd9            = 0x");
    do_2hhx(&targ->rsvd9, 1);
    do_string_eoln("\t\t\tReserved\n");

    do_string("  lock             = 0x");
    do_2hhx(&targ->lock, 1);
    do_string_eoln("\t\t\tLocked target indicator\n");

    do_string("  ipPrefix         = 0x");
    do_2hhx(&targ->ipPrefix, 1);
    do_string_eoln("\t\t\tIP prefix for a classless IP address\n");

    do_string("  owner            = ");
    do_u(&targ->owner, 1);
    do_string_eoln("\t\t\tOwning controller\n");

//  union {
    do_string("  portName         = 0x");
    do_16x(&targ->portName, 1);
    do_string_eoln("\t\tPort world wide name\n");

//    struct {
    do_string("  ipAddr           = 0x");
    do_8x(&targ->ipAddr, 1);
    do_string_eoln("\t\t\tTarget IP Address\n");

    do_string("  ipGw             = 0x");
    do_8x(&targ->ipGw, 1);
    do_string_eoln("\t\t\tDefault Gateway IP Address\n");

//    };
//  };
    do_string("  nodeName         = 0x");
    do_16x(&targ->nodeName, 1);
    do_string_eoln("\t\tNode world wide name\n");

    do_string("  prefOwner        = ");
    do_u(&targ->prefOwner, 1);
    do_string_eoln("\t\t\tPreferred owner\n");

    do_string("  cluster          = 0x");
    do_4hx(&targ->cluster, 1);
    do_string_eoln("\t\t\tCluster\n");

    do_string("  rsvd2            = 0x");
    do_4hx(&targ->rsvd2, 1);
    do_string_eoln("\t\t\tReserved\n");

    do_string("  prefPort         = ");
    do_hhu(&targ->prefPort, 1);
    do_string_eoln("\t\t\t\tPreferred port\n");

    do_string("  altPort          = ");
    do_hhu(&targ->altPort, 1);
    do_string_eoln("\t\t\t\tAlternate port\n");

    do_string("  rsvd38[2]        = 0x");
    do_2hhx(&targ->rsvd38[0], 2);
    do_string_eoln("\t\t\tReserved\n");

    do_string("  i_mask           = 0x");
    do_8x(&targ->i_mask, 1);
    do_string_eoln("\t\t\tReserved\n");

    nvxrec = (NVX *)((UINT32)nvr + sizeof(NVRH) + sizeof(NVRT));

    if (BIT_TEST(targ->opt, TARGET_ISCSI))
    {
        nvxrec = do_NVRTX(nvxrec);
    }

    /* NOTE: This one is not rounded to 16 byte boundary.! */
    nvr->hdr.recLen = (UINT32)nvxrec - (UINT32)nvr;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRT */

/* ------------------------------------------------------------------------ */
/* Get and print NVRX structure. */
static struct NVR *do_NVRX(struct NVR *nvr)
{
    struct NVRX *lddx = &nvr->u.lddx;
// fprintf(stderr, "%s NOTDONEYET -- fix formatting\n", __func__);
// fprintf(stdout, "%s NOTDONEYET -- fix formatting\n", __func__);

    GET_DATA((void *)lddx, sizeof(*lddx));

    do_string("NVRX status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  lid              = ");
    do_hu(&lddx->lid, 1);
    do_string_eoln("\t\t\t\tLDD ID\n");

    do_string("  pathMask         = 0x");
    do_2hhx(&lddx->pathMask, 1);
    do_string_eoln("\t\t\t\tPath mask\n");

    do_string("  pathPri          = ");
    do_hhu(&lddx->pathPri, 1);
    do_string_eoln("\t\t\t\tPath priority\n");

    do_string("  devCap           = ");
    do_16u(&lddx->devCap, 1);
    do_string_eoln("\t\t\t\tDevice capacity\n");

    do_string("  serial[12]       = '");
    do_c(&lddx->serial[0], 12);
    do_string_eoln("'\t\t\t\tSerial number\n");

    do_string("  baseVDisk        = ");
    do_hu(&lddx->baseVDisk, 1);
    do_string_eoln("\t\t\t\tBase virtual disk number\n");

    do_string("  baseCluster      = ");
    do_hhu(&lddx->baseCluster, 1);
    do_string_eoln("\t\t\t\tBase cluster number\n");

    do_string("  state            = 0x");
    do_2hhx(&lddx->state, 1);
    do_string_eoln("\t\t\t\tLDD sate\n");

    do_string("  baseNode         = 0x");
    do_16x(&lddx->baseNode, 1);
    do_string_eoln("\t\tBase node world wide name\n");

    do_string("  baseSN           = ");
    do_u(&lddx->baseSN, 1);
    do_string_eoln("\t\t\t\tBase serial number\n");

    do_string("  lun              = ");
    do_hu(&lddx->lun, 1);
    do_string_eoln("\t\t\t\tLUN\n");

    do_string("  rsvd30[2]        = 0x");
    do_2hhx(&lddx->rsvd30[0], 2);
    do_string_eoln("\t\t\t\tReserved\n");

    do_string("  baseName[16]     = '");
    do_c(&lddx->baseName[0], 16);
    do_string_eoln("'\t\t\t\tBase device name\n");

    do_string("  owner            = ");
    do_u(&lddx->owner, 1);
    do_string_eoln("\t\t\t\tOwner of the LDD\n");

    do_string("  rsvd36[12]       = 0x");
    do_2hhx(&lddx->rsvd36[0], 12);
    do_string_eoln("\t\t\t\tReserved\n");

    nvr->hdr.recLen = (sizeof(NVRH) + sizeof(NVRX) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRX */

/* ------------------------------------------------------------------------ */
/* Get and print NVRF structure. */
static struct NVR *do_NVRF(struct NVR *nvr)
{
    struct NVRF *lddf = &nvr->u.lddf;
fprintf(stderr, "%s NOTDONEYET -- fix formatting\n", __func__);
fprintf(stdout, "%s NOTDONEYET -- fix formatting\n", __func__);

    GET_DATA((void *)lddf, sizeof(*lddf));

    do_string("NVRF status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  lid              = ");
    do_hu(&lddf->lid, 1);
    do_string_eoln("\t\t\t\tLDD ID\n");

    do_string("  pathMask         = 0x");
    do_2hhx(&lddf->pathMask, 1);
    do_string_eoln("\t\t\t\tPath mask\n");

    do_string("  pathPri          = ");
    do_hhu(&lddf->pathPri, 1);
    do_string_eoln("\t\t\t\tPath priority\n");

    do_string("  devCap           = ");
    do_16u(&lddf->devCap, 1);
    do_string_eoln("\t\t\t\tDevice capacity\n");

    do_string("  serial[12]       = '");
    do_c(&lddf->serial[0], 12);
    do_string_eoln("'\t\t\t\tSerial number\n");

    do_string("  vendID[8]        = '");
    do_c(&lddf->vendID[0], 8);
    do_string_eoln("'\t\t\t\tVendor ID string\n");

    do_string("  prodID[16]       = '");
    do_c(&lddf->prodID[0], 16);
    do_string_eoln("'\t\t\t\tProduct ID string\n");

    do_string("  rev              = ");
    do_u(&lddf->rev, 1);
    do_string_eoln("\t\t\t\tRevision\n");

    do_string("  lun              = ");
    do_hu(&lddf->lun, 1);
    do_string_eoln("\t\t\t\tLUN\n");

    do_string("  rsvd54[6]        = 0x");
    do_2hhx(&lddf->rsvd54[0], 6);
    do_string_eoln("\t\t\t\tReserved\n");

    do_string("  owner            = ");
    do_u(&lddf->owner, 1);
    do_string_eoln("\t\t\t\tOwner of the LDD\n");

    nvr->hdr.recLen = (sizeof(NVRH) + sizeof(NVRF) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRF */

/* ------------------------------------------------------------------------ */
/* Get and print NVRM structure. */
static struct NVR *do_NVRM(struct NVR *nvr)
{
    struct NVRM *mirror = &nvr->u.mirror;

    GET_DATA((void *)mirror, sizeof(*mirror));

    do_string("NVRM status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  mySerial         = ");
    do_u(&mirror->mySerial, 1);
    do_string_eoln("\t\t\tSerial number of controller\n");

    do_string("  myPartner        = ");
    do_u(&mirror->myPartner, 1);
    do_string_eoln("\t\t\tSerial number of partner controller\n");

    do_string("  rsvd12[4]        = 0x");
    do_2hhx(&mirror->rsvd12[0], 4);
    do_string_eoln("\t\t\tReserved\n");

    nvr->hdr.recLen = (sizeof(NVRH) + sizeof(NVRM) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRM */

/* ------------------------------------------------------------------------ */
/* Get and print NVRW structure. */
static struct NVR *do_NVRW(struct NVR *nvr)
{
    struct NVRW *workset = &nvr->u.workset;
    int i;
    char ws[80];

    GET_DATA((void *)workset, sizeof(*workset));

    do_string("NVRW status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    for (i = 0; i < DEF_MAX_WORKSETS; i++)
    {
        sprintf(ws, "  workset[%d].name[%d]         = '", i, DEF_WS_NAME_SIZE);
        do_string(ws);
        do_c(&workset->workset[i].name[0], DEF_WS_NAME_SIZE);
        do_string_eoln("'\tWorkset name\n");

        sprintf(ws, "  workset[%d].vBlkBitmap[%d]    = 0x", i, DEF_WS_VB_MAP_SIZE);
        do_string(ws);
        do_2hhx(&workset->workset[i].vBlkBitmap[0], DEF_WS_VB_MAP_SIZE);
        do_string_eoln("\t\tVBlock bitmap\n");

        sprintf(ws, "  workset[%d].serverBitmap[%d] = 0x", i, DEF_WS_S_MAP_SIZE);
        do_string(ws);
        do_2hhx(&workset->workset[i].serverBitmap[0], DEF_WS_S_MAP_SIZE);
        do_string_eoln("\tServer bitmap\n");

        sprintf(ws, "  workset[%d].defaultVPort     = ", i);
        do_string(ws);
        do_hhu(&workset->workset[i].defaultVPort, 1);
        do_string_eoln("\t\tDefault VPort\n");
    }

    nvr->hdr.recLen = (sizeof(NVRH) + sizeof(NVRW) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRW */

/* ------------------------------------------------------------------------ */
/* Get and print NVDMCR structure. */
static struct NVR *do_NVDMCR(struct NVR *nvr)
{
    struct NVDMCR *dmcr = &nvr->u.dmcr;

    GET_DATA((void *)dmcr, sizeof(*dmcr));

    do_string("NVDMCR status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  rid              = ");
    do_u(&dmcr->rid, 1);
    do_string_eoln("\t\t\t\tnext registration ID of\n");

    do_string("  cr_pri           = ");
    do_hhu(&dmcr->cr_pri, 1);
    do_string_eoln("\t\t\tCOR priority\n");

    do_string("  pr_pri           = ");
    do_hhu(&dmcr->pr_pri, 1);
    do_string_eoln("\t\t\t\tproc priority\n");

    do_string("  rsvd06[10]       = 0x");
    do_2hhx(&dmcr->rsvd06[0], 10);
    do_string_eoln("\treserved\n");

/*    nvr->hdr.recLen = (sizeof(NVRH) + sizeof(NVDMCR) + 0xF) & 0xFFF0; */
    nvr->hdr.recLen = sizeof(NVRH) + sizeof(NVDMCR);
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVDMCR */

/* ------------------------------------------------------------------------ */
/* Get and print NVCOPY structure. */
static struct NVR *do_NVCOPY(struct NVR *nvr)
{
    struct NVCOPY *copycfg = &nvr->u.copycfg;
// fprintf(stderr, "%s NOTDONEYET -- fix formatting\n", __func__);
// fprintf(stdout, "%s NOTDONEYET -- fix formatting\n", __func__);

    GET_DATA((void *)copycfg, sizeof(*copycfg));

    do_string("NVCOPY status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  svid             = ");
    do_hu(&copycfg->svid, 1);
    do_string_eoln("\t\t\t\tsource VID\n");

    do_string("  stype            = ");
    do_hhu(&copycfg->stype, 1);
    do_string_eoln("\t\t\t\tsource SCD type\n");

    do_string("  shidx            = ");
    do_hhu(&copycfg->shidx, 1);
    do_string_eoln("\t\t\t\tsrc phs1/2 upd hdlrs idx\n");

    do_string("  dvid             = ");
    do_hu(&copycfg->dvid, 1);
    do_string_eoln("\t\t\t\tdest VID\n");

    do_string("  dtype            = ");
    do_hhu(&copycfg->dtype, 1);
    do_string_eoln("\t\t\t\tdesnssnt SCD type\n");

    do_string("  dhidx            = ");
    do_hhu(&copycfg->dhidx, 1);
    do_string_eoln("\t\t\t\tdest phs1/2 upd hdlrs idx\n");

    do_string("  tsegs            = ");
    do_u(&copycfg->tsegs, 1);
    do_string_eoln("\t\t\t\ttotal segments\n");

    do_string("  rid              = ");
    do_u(&copycfg->rid, 1);
    do_string_eoln("\t\t\t\tcopy registration ID\n");

    do_string("  rcsn             = ");
    do_u(&copycfg->rcsn, 1);
    do_string_eoln("\t\t\t\tCM MAG serial number\n");

    do_string("  rcscl            = ");
    do_hhu(&copycfg->rcscl, 1);
    do_string_eoln("\t\t\t\tCM source cl num\n");

    do_string("  rcsvd            = ");
    do_hhu(&copycfg->rcsvd, 1);
    do_string_eoln("\t\t\t\tCM source Vdisk num\n");

    do_string("  rcdcl            = ");
    do_hhu(&copycfg->rcdcl, 1);
    do_string_eoln("\t\t\t\tCM destination cl num\n");

    do_string("  rcdvd            = ");
    do_hhu(&copycfg->rcdvd, 1);
    do_string_eoln("\t\t\t\tCM destination Vdisk num\n");

    do_string("  rssn             = ");
    do_u(&copycfg->rssn, 1);
    do_string_eoln("\t\t\t\tCopy source serial num\n");

    do_string("  rdsn             = ");
    do_u(&copycfg->rdsn, 1);
    do_string_eoln("\t\t\t\tCopy dest serial num\n");

    do_string("  rscl             = ");
    do_hhu(&copycfg->rscl, 1);
    do_string_eoln("\t\t\t\tCopy source cl num\n");

    do_string("  rsvd             = ");
    do_hhu(&copycfg->rsvd, 1);
    do_string_eoln("\t\t\t\tCopy source Vdisk num\n");

    do_string("  rdcl             = ");
    do_hhu(&copycfg->rdcl, 1);
    do_string_eoln("\t\t\t\tCopy dest cl num\n");

    do_string("  rdvd             = ");
    do_hhu(&copycfg->rdvd, 1);
    do_string_eoln("\t\t\t\tCopy dest Vdisk num\n");

    do_string("  gid              = ");
    do_hhu(&copycfg->gid, 1);
    do_string_eoln("\t\t\t\tuser defined group ID\n");

    do_string("  cr_crstate       = 0x");
    do_hhu(&copycfg->cr_crstate, 1);
    do_string_eoln("\t\t\t\tcor copy registration state\n");

    do_string("  cr_cstate        = 0x");
    do_hhu(&copycfg->cr_cstate, 1);
    do_string_eoln("\t\t\t\tcor  cstate\n");

    do_string("  cm_type          = ");
    do_hhu(&copycfg->cm_type, 1);
    do_string_eoln("\t\t\t\tCM  type\n");

    do_string("  cm_pri           = ");
    do_hhu(&copycfg->cm_pri, 1);
    do_string_eoln("\t\t\t\tCM  priority\n");

    do_string("  cm_mtype         = ");
    do_hhu(&copycfg->cm_mtype, 1);
    do_string_eoln("\t\t\t\tCM copy type/mirror type\n");

    do_string("  rsvd2a[2]        = 0x");
    do_2hhx(&copycfg->rsvd2a[0], 2);
    do_string_eoln("\t\t\t\treserve\n");

    do_string("  powner           = ");
    do_u(&copycfg->powner, 1);
    do_string_eoln("\t\t\t\tprimary owning controller s/n\n");

    do_string("  sowner           = ");
    do_u(&copycfg->sowner, 1);
    do_string_eoln("\t\t\t\tsecondary owning controller s/n\n");

    do_string("  rsvd34[12]       = 0x");
    do_2hhx(&copycfg->rsvd34[0], 12);
    do_string_eoln("\t\t\t\treserve\n");

    do_string("  label[16]        = '");
    do_c(&copycfg->label[0], 16);
    do_string_eoln("'\t\t\t\tcopy label\n");

    do_string("  nssn             = ");
    do_u(&copycfg->nssn, 1);
    do_string_eoln("\t\t\t\t  new source sn\n");

    do_string("  ndsn             = ");
    do_u(&copycfg->ndsn, 1);
    do_string_eoln("\t\t\t\t  new destination sn\n");

    do_string("  cssn             = ");
    do_u(&copycfg->cssn, 1);
    do_string_eoln("\t\t\t\t  current (old) source sn\n");

    do_string("  cdsn             = ");
    do_u(&copycfg->cdsn, 1);
    do_string_eoln("\t\t\t\t  current (old) dest sn\n");

//    nvr->hdr.recLen = (sizeof(NVRH) + sizeof(NVCOPY) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVCOPY */

/* ------------------------------------------------------------------------ */
/* Get and print NVRISNS structure. */
static struct NVR *do_NVRISNS(struct NVR *nvr)
{
    NVRISNS *isns = &nvr->u.isns;
    int i;
    char ws[80];

    GET_DATA((void *)isns, sizeof(*isns));

    do_string("NVRISNS status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  gflags           = 0x");
    do_8x(&isns->gflags, 1);
    do_string_eoln("\t\t\tGlobal Flags\n");

    do_string("  rsvd             = 0x");
    do_8x(&isns->rsvd, 1);
    do_string_eoln("\t\t\tCreate a QUAD boundary\n");

//    struct {
    for (i = 0; i < MAX_ISNS_SERVERS; i++)
    {
        sprintf(ws, "  srv[%d].ip        = 0x", i);
        do_string(ws);
        do_8x(&isns->srv[i].ip, 1);
        do_string_eoln("\t\t\tServer IP Address\n");

        sprintf(ws, "  srv[%d].port      = ", i);
        do_string(ws);
        do_hu(&isns->srv[i].port, 1);
        do_string_eoln("\t\t\t\tServer TCP/UDP Port\n");

        sprintf(ws, "  srv[%d].sflags    = 0x", i);
        do_string(ws);
        do_4hx(&isns->srv[i].sflags, 1);
        do_string_eoln("\t\t\tServer Flags\n");

//#ifdef FRONTEND
//        PCB         *pt_iSNS;               /* iSNS Server task PCB             */
//#endif /* FRONTEND */
//    } srv[MAX_ISNS_SERVERS];
    }

    nvr->hdr.recLen = (sizeof(NVRH) + sizeof(NVRISNS) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRISNS */

/* ------------------------------------------------------------------------ */
/* Get and print NVRPRX structure. */
static struct NVX *do_NVRPRX(struct NVX *nvxrec)
{
    struct NVRPRX *nvrprx = &nvxrec->u.prsvx;

    GET_DATA((void *)nvrprx, sizeof(*nvrprx));

    do_string_eoln("NVRPRX\n");

    do_string("  sid              = ");
    do_hu(&nvrprx->sid, 1);
    do_string_eoln("\t\t\t\tsid of initiator\n");

    do_string("  tid              = ");
    do_hhu(&nvrprx->tid, 1);
    do_string_eoln("\t\t\t\ttid\n");

    do_string("  lun              = ");
    do_hhu(&nvrprx->lun, 1);
    do_string_eoln("\t\t\t\tLUN\n");

    do_string("  key[8]           = 0x");
    do_2hhx(&nvrprx->key[0], 8);
    do_string_eoln("\t\t8 Byte Registration key\n");

    return ((NVX *)((UINT32)nvxrec + sizeof(NVRPRX)));
}   /* End of do_NVRPRX */

/* ------------------------------------------------------------------------ */
/* Get and print NVRPR structure. */
static struct NVR *do_NVRPR(struct NVR *nvr)
{
    struct NVRPR *prsv = &nvr->u.prsv;
    int i;
    struct NVX *nvxrec;

    GET_DATA((void *)prsv, sizeof(*prsv));

    do_string("NVRPR status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  vid              = ");
    do_hu(&prsv->vid, 1);
    do_string_eoln("\t\t\t\tVID\n");

    do_string("  sid              = ");
    do_hu(&prsv->sid, 1);
    do_string_eoln("\t\t\t\tsid of initiator holding reservation\n");

    do_string("  scope            = ");
    do_hhu(&prsv->scope, 1);
    do_string_eoln("\t\t\t\tscope of reservation\n");

    do_string("  type             = ");
    do_hhu(&prsv->type, 1);
    do_string_eoln("\t\t\t\ttype of reservation\n");

    do_string("  regCount         = ");
    do_hhu(&prsv->regCount, 1);
    do_string_eoln("\t\t\t\tnumber of registrations\n");

    do_string("  rsvd             = 0x");
    do_2hhx(&prsv->rsvd, 1);
    do_string_eoln("\t\t\treserved\n");

    /* Process extended server (NVRSX) records */
    nvxrec = (NVX *)((UINT32)nvr + sizeof(NVRPR) + sizeof(NVRH));

    for (i = 0; i < prsv->regCount; i++)
    {
        nvxrec = do_NVRPRX(nvxrec);
    }
    
    nvr->hdr.recLen = (((UINT32)nvxrec - (UINT32)nvr) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRPR */

/* ------------------------------------------------------------------------ */
/* Get and print NVRLRDD structure. */
static struct NVR *do_NVRLRDD(struct NVR *nvr)
{
    struct NVRLRDD *lraid = &nvr->u.lraid;
fprintf(stderr, "%s NOTDONEYET -- fix formatting\n", __func__);
fprintf(stdout, "%s NOTDONEYET -- fix formatting\n", __func__);

    GET_DATA((void *)lraid, sizeof(*lraid));

    do_string("NVRLRDD status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  rid              = ");
    do_hu(&lraid->rid, 1);
    do_string_eoln("\t\t\t\tRAID ID\n");

    do_string("  psdCnt           = ");
    do_hu(&lraid->psdCnt, 1);
    do_string_eoln("\t\t\t\tNumber of PSDs\n");

    do_string("  aStatus          = 0x");
    do_2hhx(&lraid->aStatus, 1);
    do_string_eoln("\t\t\t\tAlternate status\n");

    do_string("  rsvd[3]          = 0x");
    do_2hhx(&lraid->rsvd[0], 3);
    do_string_eoln("\t\t\t\tPad\n");

    nvr->hdr.recLen = (sizeof(NVRH) + sizeof(NVRLRDD) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRLRDD */

/* ------------------------------------------------------------------------ */
/* Get and print NVRLRDD2 structure. */
static struct NVR *do_NVRLRDD2(struct NVR *nvr)
{
    struct NVRLRDD2 *lraid2 = &nvr->u.lraid2;
fprintf(stderr, "%s NOTDONEYET -- fix formatting\n", __func__);
fprintf(stdout, "%s NOTDONEYET -- fix formatting\n", __func__);

    GET_DATA((void *)lraid2, sizeof(*lraid2));

    do_string("NVRLRDD2 status=0x");
    do_2hhx(&nvr->hdr.status, 1);
    do_string_eoln("\n");
    /* -------------------------------------------------------------------- */

    do_string("  rid              = ");
    do_hu(&lraid2->rid, 1);
    do_string_eoln("\t\t\t\tRAID ID\n");

    do_string("  psdCnt           = ");
    do_hu(&lraid2->psdCnt, 1);
    do_string_eoln("\t\t\t\tNumber of PSDs\n");

    do_string("  aStatus          = 0x");
    do_2hhx(&lraid2->aStatus, 1);
    do_string_eoln("\t\t\t\tAlternate status\n");

    do_string("  rsvd13[3]        = 0x");
    do_2hhx(&lraid2->rsvd13[0], 3);
    do_string_eoln("\t\t\t\tPad\n");

    do_string("  notMirroringCSN  = ");
    do_u(&lraid2->notMirroringCSN, 1);
    do_string_eoln("\t\t\t\tNot Mirroring Controller Serial Num\n");

    do_string("  rsvd20[12]       = 0x");
    do_2hhx(&lraid2->rsvd20[0], 12);
    do_string_eoln("\t\t\t\tPad\n");

    nvr->hdr.recLen = (sizeof(NVRH) + sizeof(NVRLRDD2) + 0xF) & 0xFFF0;
    return ((NVR *)((UINT32)nvr + nvr->hdr.recLen));
}   /* End of do_NVRLRDD2 */

/* ------------------------------------------------------------------------ */
#if defined(BE_ASCII)
/* Main program for BEascii */
int main(void)
{
    struct NVR     *nvrec;
    struct NVR     *orig;
    int             err;
    int             current_byte_on;
    NVRII *p2;

    p2 = malloc(NVRAM_P2_SIZE);
    memset((void *)p2, 0, NVRAM_P2_SIZE);

    nvrec = do_NVRII(p2);

    current_byte_on = byte_on_in_file;
    err = do_hdr(&nvrec->hdr);
    while (err > 0)
    {
        orig = nvrec;
// fprintf(stderr, "byte_on_in_file=%d (0x%x) -- recType=0x%02x, recLen=0x%x (%d)\n", byte_on_in_file, byte_on_in_file, orig->hdr.recType, orig->hdr.recLen, orig->hdr.recLen);
printf("\nbyte_on_in_file=%d (0x%x) -- recType=0x%02x, recLen=0x%x (%d)\n", byte_on_in_file, byte_on_in_file, orig->hdr.recType, orig->hdr.recLen, orig->hdr.recLen);
        switch (orig->hdr.recType)
        {
            case NRT_EOF:             /* End of file record                  */
                printf("NRT_EOF\n");
                fprintf(stderr, "NRT_EOF\n");
                err = 0;               /* FLAG DONE */
                break;

            case NRT_SERVER:          /* server devices                      */
                nvrec = do_NVRS(orig);
                break;

            case NRT_RAID:            /* RAID devices                        */
                nvrec = do_NVRR(orig);
                break;

            case NRT_RAID_GT2TB:      /* RAID devices                        */
                nvrec = do_NVRR_GT2TB(orig);
                break;

            case NRT_VIRT:            /* virtual devices                     */
                nvrec = do_NVRV(orig);
                break;

            case NRT_PHYS:            /* physical devices                    */
                nvrec = do_NVRP(orig);
                break;

            case NRT_SES:             /* SES devices                         */
                nvrec = do_NVRP(orig);
                break;

            case NRT_MISC:            /* misc devices                        */
                nvrec = do_NVRP(orig);
                break;

            case NRT_TARGET:          /* targets                             */
                nvrec = do_NVRT(orig);
                break;

            case NRT_XLDD:            /* XIOtech LDD                         */
                nvrec = do_NVRX(orig);
                break;

            case NRT_FLDD:            /* foreign LDD                         */
                nvrec = do_NVRF(orig);
                break;

            case NRT_MIRROR:          /* mirror partner                      */
                nvrec = do_NVRM(orig);
                break;

            case NRT_WORKSET:         /* workset definitions                 */
                nvrec = do_NVRW(orig);
                break;

            case NRT_DMCR:            /* default copy configuration record   */
                nvrec = do_NVDMCR(orig);
                break;

            case NRT_COPYCFG:         /* copy configuration record           */
                nvrec = do_NVCOPY(orig);
                break;

            case NRT_ISNS:            /* iSNS configuration record           */
                nvrec = do_NVRISNS(orig);
                break;

            case NRT_PRES:            /* Persistent LUN Reserve record       */
                nvrec = do_NVRPR(orig);
                break;

            case NRL_RAID:            /* RAID records in local image Version 1 */
                nvrec = do_NVRLRDD(orig);
                break;

//            case NRL_PHYS:            /* Physical devices in local image      */
//                nvrec = do_NVRLXPDD(orig);
//                break;

            case NRL_ENCL:            /* Device enclosure in local image      */
                fprintf(stderr, "NOTDONEYET recType = 0x%02x (%d) NRL_ENCL\n", orig->hdr.recType,
                        orig->hdr.recType);
                fprintf(stdout, "NOTDONEYET recType = 0x%02x (%d) NRL_ENCL\n", orig->hdr.recType,
                        orig->hdr.recType);
                break;

            case NRL_RAID2:           /* RAID records in local image Version 2 */
                nvrec = do_NVRLRDD2(orig);
                break;

            default:
                fprintf(stderr, "unknown recType = 0x%02x (%d)\n",
                        orig->hdr.recType, orig->hdr.recType);
                fprintf(stderr, "        recLen = 0x%04x (%d)\n",
                        orig->hdr.recLen, orig->hdr.recLen);
                fprintf(stderr, "        status = 0x%02x (%d)\n",
                        orig->hdr.status, orig->hdr.status);
                fprintf(stderr, "byte_on_in_file=%d (0x%x)\n", byte_on_in_file, byte_on_in_file);
                exit(1);
        }
        if (err > 0)
        {
            int i;

            /* The records are aligned on 16 byte boundaries. */
            if (byte_on_in_file < current_byte_on + orig->hdr.recLen)
            {
                if (byte_on_in_file + 15 < current_byte_on + orig->hdr.recLen)
                {
                    fprintf(stderr, "Record way too short, not decoded properly! %d\n",
                            byte_on_in_file - (current_byte_on + orig->hdr.recLen));

                    fprintf(stderr, "orig->hdr @ 0x%08x\n", current_byte_on);

                    fprintf(stderr, "recType = 0x%02x (%d)\n",
                            orig->hdr.recType, orig->hdr.recType);
                    fprintf(stderr, " recLen = 0x%04x (%d)\n",
                            orig->hdr.recLen, orig->hdr.recLen);
                    fprintf(stderr, " status = 0x%02x (%d)\n",
                            orig->hdr.status, orig->hdr.status);
                    fprintf(stderr, "byte_on_in_file=%d (0x%x)\n", byte_on_in_file, byte_on_in_file);
                    abort();
                }
#if 0
                else if (byte_on_in_file != current_byte_on + orig->hdr.recLen)
                {
                    fprintf(stderr, "Record too short by %d\n",
                            byte_on_in_file - (current_byte_on + orig->hdr.recLen));

                    fprintf(stderr, "orig->hdr @ 0x%08x\n", current_byte_on);

                    fprintf(stderr, "recType = 0x%02x (%d)\n",
                            orig->hdr.recType, orig->hdr.recType);
                    fprintf(stderr, " recLen = 0x%04x (%d)\n",
                            orig->hdr.recLen, orig->hdr.recLen);
                    fprintf(stderr, " status = 0x%02x (%d)\n",
                            orig->hdr.status, orig->hdr.status);
                    fprintf(stderr, "byte_on_in_file=%d (0x%x)\n", byte_on_in_file, byte_on_in_file);
                }
#endif  /* 0 */
                while (byte_on_in_file < current_byte_on + orig->hdr.recLen)
                {
                    err = GET_DATA((void *)&i, 1);
                }
            }
            else if (byte_on_in_file > current_byte_on + orig->hdr.recLen)
            {
                fprintf(stderr, "Record too long, too much read %d\n",
                        byte_on_in_file - (current_byte_on + orig->hdr.recLen));
                fprintf(stderr, "recType = 0x%02x (%d)\n",
                        orig->hdr.recType, orig->hdr.recType);
                fprintf(stderr, " recLen = 0x%04x (%d)\n",
                        orig->hdr.recLen, orig->hdr.recLen);
                fprintf(stderr, " status = 0x%02x (%d)\n",
                        orig->hdr.status, orig->hdr.status);
                fprintf(stderr, "byte_on_in_file=%d (0x%x)\n", byte_on_in_file, byte_on_in_file);
                abort();
            }
            current_byte_on = byte_on_in_file;
            err = do_hdr(&nvrec->hdr);
        }
    }

    exit(0);
}
#endif  /* BE_ASCII */

/* ------------------------------------------------------------------------ */
#if !defined(BE_ASCII)
/* Main program for BEbinary */
int main(void)
{
    struct NVR     *nvrec;
    int             err;
    int i;
    char *dst;
    NVRII *p2;
//    struct NVR *start_nvrec;    /* For printing where in nvrec we are. */

    p2 = malloc(NVRAM_P2_SIZE);
    memset((void *)p2, 0, NVRAM_P2_SIZE);

    nvrec = do_NVRII(p2);
    
    nvrec = (struct NVR *)(p2 + 1);
//    start_nvrec = (struct NVR *)p2;

    while (1)
    {
        if (match(line, "\n"))    /* Allow line saying where on in file. */
        {
            GET_LINE();
        }
        if (match(line, "byte_on_in_file="))    /* Allow line saying where on in file. */
        {
            GET_LINE();
        }
// fprintf(stderr, "before do_??? (0x%x)\n", (UINT32)nvrec - (UINT32)start_nvrec);

        nvrec->hdr.status = 0;
        if (match(line, "NRT_EOF\n"))
        {
            nvrec->hdr.recLen = sizeof(NVRH);
            nvrec->hdr.recType = NRT_EOF;
            p2->length = (UINT32)(nvrec) - (UINT32)p2 + sizeof(NVRH);
            fprintf(stderr, "Length of PII nvram image = %d\n", p2->length);
            NV_P2ChkSum(p2);
            dst = (char *)nvrec + sizeof(NVRH);
            for (i = 0; i < 124; i++)
            {
                dst[i] = 0xff;
            }
            for (; i < 256; i++)
            {
                dst[i] = 0;
            }
            err = write(1, (void *)p2, NVRAM_P2_SIZE);
            if (err != NVRAM_P2_SIZE) {
                fprintf(stderr, "NVRII write problem = %d\n", err);
                fprintf(stderr, "linecount=%d\n", linecount);
                abort();
            }
            break;
        }
        else if (match(line, "NVRS "))
        {
            nvrec->hdr.recLen = sizeof(NVRS) + sizeof(NVRH);    // Changed with all server records.
            nvrec->hdr.recType = NRT_SERVER;
            nvrec = do_NVRS(nvrec);
        }
        else if (match(line, "NVRP "))
        {
            nvrec->hdr.recLen = sizeof(NVRP) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_PHYS;
            nvrec = do_NVRP(nvrec);
        }
        else if (match(line, "NVRR "))
        {
            nvrec->hdr.recLen = sizeof(NVRR) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_RAID;
            nvrec = do_NVRR(nvrec);
        }
        else if (match(line, "NVRR_GT2TB "))
        {
            nvrec->hdr.recLen = sizeof(NVRR_GT2TB) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_RAID_GT2TB;
            nvrec = do_NVRR_GT2TB(nvrec);
        }
        else if (match(line, "NVRV "))
        {
            nvrec->hdr.recLen = sizeof(NVRV) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_VIRT;
            nvrec = do_NVRV(nvrec);
        }
        else if (match(line, "NVRT "))
        {
            nvrec->hdr.recLen = sizeof(NVRT) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_TARGET;
            nvrec = do_NVRT(nvrec);
        }
        else if (match(line, "NVRX "))
        {
            nvrec->hdr.recLen = sizeof(NVRX) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_XLDD;
            nvrec = do_NVRX(nvrec);
        }
        else if (match(line, "NVRF "))
        {
            nvrec->hdr.recLen = sizeof(NVRF) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_FLDD;
            nvrec = do_NVRF(nvrec);
        }
        else if (match(line, "NVRM "))
        {
            nvrec->hdr.recLen = sizeof(NVRM) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_MIRROR;
            nvrec = do_NVRM(nvrec);
        }
        else if (match(line, "NVRW "))
        {
            nvrec->hdr.recLen = sizeof(NVRW) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_WORKSET;
            nvrec = do_NVRW(nvrec);
        }
        else if (match(line, "NVDMCR "))
        {
            nvrec->hdr.recLen = sizeof(NVDMCR) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_DMCR;
            nvrec = do_NVDMCR(nvrec);
        }
        else if (match(line, "NVCOPY "))
        {
            nvrec->hdr.recLen = sizeof(NVDMCR) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_COPYCFG;
            nvrec = do_NVCOPY(nvrec);
        }
        else if (match(line, "NVRISNS "))
        {
            nvrec->hdr.recLen = sizeof(NVDMCR) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_ISNS;
            nvrec = do_NVRISNS(nvrec);
        }
        else if (match(line, "NVRPR "))
        {
            nvrec->hdr.recLen = sizeof(NVRPR) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_PRES;
            nvrec = do_NVRPR(nvrec);
        }
        else if (match(line, "NVRLRDD "))
        {
            nvrec->hdr.recLen = sizeof(NVRLRDD) + sizeof(NVRH);
            nvrec->hdr.recType = NRL_RAID;
            nvrec = do_NVRLRDD(nvrec);
        }
        else if (match(line, "NVRLRDD2 "))
        {
            nvrec->hdr.recLen = sizeof(NVRLRDD2) + sizeof(NVRH);
            nvrec->hdr.recType = NRL_RAID2;
            nvrec = do_NVRLRDD2(nvrec);
        }
        else
        {
                fprintf(stderr, "unknown Line: %s", line);
                fprintf(stderr, "linecount=%d\n", linecount);
                exit(1);
        }
    }

    fprintf(stderr, "linecount=%d\n", linecount);
    exit(0);
}
#endif  /* BE_ASCII */

/* ------------------------------------------------------------------------ */
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
