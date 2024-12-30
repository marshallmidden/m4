#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/stat.h>
#include <limits.h>

/* ------------------------------------------------------------------------ */
#define SECT_128MB   (128 * 1024 * 1024 / 512)
/* ------------------------------------------------------------------------ */
static const char short_opts[] = "h";

static const struct option long_opts[] = {
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

/* ------------------------------------------------------------------------ */
/*
 * We preserve all sectors read in a chain - some of these will
 * have to be modified and written back.
 */
static struct sector
{
    struct sector  *next;
    unsigned long long sectornumber;
    int             to_be_written;
    char            data[512];
}              *sectorhead;

static struct geometry
{
    unsigned long long total_size;     /* in sectors */
    unsigned long   cylindersize;      /* in sectors */
    unsigned long   heads;
    unsigned long   sectors;
    unsigned long   cylinders;
    unsigned long   start;
} B, F;

typedef struct
{
    unsigned char   h;
    unsigned char   s;
    unsigned char   c;
} __attribute__ ((packed)) chs;        /* has some c bits in s */

static chs      zero_chs = { 0, 0, 0 };

typedef struct
{
    unsigned long   h;
    unsigned long   s;
    unsigned long   c;
} longchs;

static longchs  zero_longchs;

/* MS/DOS partition */
struct partition
{
    unsigned char   bootable;          /* 0 or 0x80 */
    chs             begin_chs;
    unsigned char   sys_type;
    chs             end_chs;
    unsigned int    start_sect;        /* starting sector counting from 0 */
    unsigned int    nr_sects;          /* nr of sectors in partition */
} __attribute__ ((packed));

/* Roughly speaking, Linux doesn't use any of the above fields except
   for partition type, start sector and number of sectors. (However,
   see also linux/drivers/scsi/fdomain.c.)
   The only way partition type is used (in the kernel) is the comparison
   for equality with EXTENDED_PARTITION (and these Disk Manager types). */

struct part_desc
{
    unsigned long long start;
    unsigned long long size;
    unsigned long long sector;
    unsigned long long offset;         /* disk location of this info */
    struct partition p;
    struct part_desc *ep;              /* extended partition containing this one */
    int             ptype;
#define DOS_TYPE    0
#define BSD_TYPE    1
};

static struct disk_desc
{
    struct part_desc partitions[512];
    int             partno;
} oldp;

#define BSD_DISKMAGIC   (0x82564557UL)
#define BSD_MAXPARTITIONS       16
#define BSD_FS_UNUSED           0

struct bsd_disklabel
{
    unsigned int    d_magic;
    char            d_junk1[4];
    char            d_typename[16];
    char            d_packname[16];
    char            d_junk2[92];
    unsigned int    d_magic2;
    char            d_junk3[2];
    unsigned short  d_npartitions;     /* number of partitions in following */
    char            d_junk4[8];
    struct bsd_partition
    {                                  /* the partition table */
        unsigned int    p_size;        /* number of sectors in partition */
        unsigned int    p_offset;      /* starting sector */
        unsigned int    p_fsize;       /* filesystem basic fragment size */
        unsigned char   p_fstype;      /* filesystem type, see below */
        unsigned char   p_frag;        /* filesystem fragments per block */
        unsigned short  p_cpg;         /* filesystem cylinders per group */
    } d_partitions[BSD_MAXPARTITIONS]; /* actually may be more */
};

/* ------------------------------------------------------------------------ */
#define EMPTY_PARTITION        0
#define EXTENDED_PARTITION     5
#define WIN98_EXTENDED      0x0f
#define DM6_AUX1PARTITION   0x51
#define DM6_AUX3PARTITION   0x53
#define DM6_PARTITION       0x54
#define EZD_PARTITION       0x55
#define LINUX_EXTENDED      0x85
#define BSD_PARTITION       0xa5
#define NETBSD_PARTITION    0xa9

struct systypes
{
    unsigned char   type;
    char           *name;
};

static struct systypes i386_sys_types[] =
{
	{0x00, "Empty"},
	{0x01, "FAT12"},
	{0x02, "XENIX root"},
	{0x03, "XENIX usr"},
	{0x04, "FAT16 <32M"},
	{0x05, "Extended"},		/* DOS 3.3+ extended partition */
	{0x06, "FAT16"},		/* DOS 16-bit >=32M */
	{0x07, "HPFS/NTFS/exFAT"},	/* OS/2 IFS, eg, HPFS or NTFS or QNX or exFAT */
	{0x08, "AIX"},		/* AIX boot (AIX -- PS/2 port) or SplitDrive */
	{0x09, "AIX bootable"},	/* AIX data or Coherent */
	{0x0a, "OS/2 Boot Manager"},/* OS/2 Boot Manager */
	{0x0b, "W95 FAT32"},
	{0x0c, "W95 FAT32 (LBA)"},/* LBA really is `Extended Int 13h' */
	{0x0e, "W95 FAT16 (LBA)"},
	{0x0f, "W95 Ext'd (LBA)"},
	{0x10, "OPUS"},
	{0x11, "Hidden FAT12"},
	{0x12, "Compaq diagnostics"},
	{0x14, "Hidden FAT16 <32M"},
	{0x16, "Hidden FAT16"},
	{0x17, "Hidden HPFS/NTFS"},
	{0x18, "AST SmartSleep"},
	{0x1b, "Hidden W95 FAT32"},
	{0x1c, "Hidden W95 FAT32 (LBA)"},
	{0x1e, "Hidden W95 FAT16 (LBA)"},
	{0x24, "NEC DOS"},
	{0x27, "Hidden NTFS WinRE"},
	{0x39, "Plan 9"},
	{0x3c, "PartitionMagic recovery"},
	{0x40, "Venix 80286"},
	{0x41, "PPC PReP Boot"},
	{0x42, "SFS"},
	{0x4d, "QNX4.x"},
	{0x4e, "QNX4.x 2nd part"},
	{0x4f, "QNX4.x 3rd part"},
	{0x50, "OnTrack DM"},
	{0x51, "OnTrack DM6 Aux1"},	/* (or Novell) */
	{0x52, "CP/M"},		/* CP/M or Microport SysV/AT */
	{0x53, "OnTrack DM6 Aux3"},
	{0x54, "OnTrackDM6"},
	{0x55, "EZ-Drive"},
	{0x56, "Golden Bow"},
	{0x5c, "Priam Edisk"},
	{0x61, "SpeedStor"},
	{0x63, "GNU HURD or SysV"},	/* GNU HURD or Mach or Sys V/386 (such as ISC UNIX) */
	{0x64, "Novell Netware 286"},
	{0x65, "Novell Netware 386"},
	{0x70, "DiskSecure Multi-Boot"},
	{0x75, "PC/IX"},
	{0x80, "Old Minix"},	/* Minix 1.4a and earlier */
	{0x81, "Minix / old Linux"},/* Minix 1.4b and later */
	{0x82, "Linux swap / Solaris"},
	{0x83, "Linux"},
	{0x84, "OS/2 hidden C: drive"},
	{0x85, "Linux extended"},
	{0x86, "NTFS volume set"},
	{0x87, "NTFS volume set"},
	{0x88, "Linux plaintext"},
	{0x8e, "Linux LVM"},
	{0x93, "Amoeba"},
	{0x94, "Amoeba BBT"},	/* (bad block table) */
	{0x9f, "BSD/OS"},		/* BSDI */
	{0xa0, "IBM Thinkpad hibernation"},
	{0xa5, "FreeBSD"},		/* various BSD flavours */
	{0xa6, "OpenBSD"},
	{0xa7, "NeXTSTEP"},
	{0xa8, "Darwin UFS"},
	{0xa9, "NetBSD"},
	{0xab, "Darwin boot"},
	{0xaf, "HFS / HFS+"},
	{0xb7, "BSDI fs"},
	{0xb8, "BSDI swap"},
	{0xbb, "Boot Wizard hidden"},
	{0xbe, "Solaris boot"},
	{0xbf, "Solaris"},
	{0xc1, "DRDOS/sec (FAT-12)"},
	{0xc4, "DRDOS/sec (FAT-16 < 32M)"},
	{0xc6, "DRDOS/sec (FAT-16)"},
	{0xc7, "Syrinx"},
	{0xda, "Non-FS data"},
	{0xdb, "CP/M / CTOS / ..."},/* CP/M or Concurrent CP/M or
					   Concurrent DOS or CTOS */
	{0xde, "Dell Utility"},	/* Dell PowerEdge Server utilities */
	{0xdf, "BootIt"},		/* BootIt EMBRM */
	{0xe1, "DOS access"},	/* DOS access or SpeedStor 12-bit FAT
					   extended partition */
	{0xe3, "DOS R/O"},		/* DOS R/O or SpeedStor */
	{0xe4, "SpeedStor"},	/* SpeedStor 16-bit FAT extended
					   partition < 1024 cyl. */
	{0xeb, "BeOS fs"},
	{0xee, "GPT"},		/* Intel EFI GUID Partition Table */
	{0xef, "EFI (FAT-12/16/32)"},/* Intel EFI System Partition */
	{0xf0, "Linux/PA-RISC boot"},/* Linux/PA-RISC boot loader */
	{0xf1, "SpeedStor"},
	{0xf4, "SpeedStor"},	/* SpeedStor large partition */
	{0xf2, "DOS secondary"},	/* DOS 3.3+ secondary */
	{0xfb, "VMware VMFS"},
	{0xfc, "VMware VMKCORE"},	/* VMware kernel dump partition */
	{0xfd, "Linux raid autodetect"},/* New (2.2.x) raid partition with
					       autodetect using persistent
					       superblock */
	{0xfe, "LANstep"},		/* SpeedStor >1024 cyl. or LANstep */
	{0xff, "BBT"},		/* Xenix Bad Block Table */

	{ 0, 0 }
};

/* ------------------------------------------------------------------------ */
/* return partition name */
static char *partname(char *dev, int pno, int lth)
{
    static char bufp[PATH_MAX];
    char *p;
    int w, wp;

    w = strlen(dev);
    p = "";

    if (isdigit(dev[w-1]))
	p = "p";

    wp = strlen(p);

    if (lth) {
	snprintf(bufp, sizeof(bufp), "%*.*s%s%-2u", lth-wp-2, w, dev, p, pno);
    } else {
	snprintf(bufp, sizeof(bufp), "%.*s%s%-2u", w, dev, p, pno);
    }
    return bufp;
}

/* ------------------------------------------------------------------------ */
/*
 * sseek: seek to specified sector - return 0 on failure
 * Note: we use 512-byte sectors here, irrespective of the hardware ss.
 */
static int sseek(char *dev, int fd, unsigned long s)
{
    off_t           in;
    off_t           out;

    in = ((off_t) s << 9);

    if ((out = lseek(fd, in, SEEK_SET)) != in)
    {
        perror("lseek");
        fflush(stdout);
        fprintf(stderr, "seek error on %s - cannot seek to %lu\n", dev, s);
        return 0;
    }

    if (in != out)
    {
        fflush(stdout);
        fprintf(stderr, "seek error: wanted 0x%08x%08x, got 0x%08x%08x\n",
              (unsigned int)(in >> 32), (unsigned int)(in & 0xffffffff),
              (unsigned int)(out >> 32), (unsigned int)(out & 0xffffffff));
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------------ */
static void free_sectors(void)
{
    struct sector  *s;

    while (sectorhead)
    {
        s = sectorhead;
        sectorhead = s->next;
        free(s);
    }
}

/* ------------------------------------------------------------------------ */
static struct sector *get_sector(char *dev, int fd, unsigned long long sno)
{
    struct sector  *s;

    for (s = sectorhead; s; s = s->next)
    {
        if (s->sectornumber == sno)
        {
            return s;
        }
    }

    if (!sseek(dev, fd, sno))
    {
        return 0;
    }

    s = (struct sector *)malloc(sizeof(struct sector));

    if (read(fd, s->data, sizeof(s->data)) != sizeof(s->data))
    {
        if (errno)                     /* 0 in case we read past end-of-disk */
        {
            perror("read");
        }
        fflush(stdout);
        fprintf(stderr, "read error on %s - cannot read sector %llu\n", dev, sno);
        free(s);
        return 0;
    }

    s->next = sectorhead;
    sectorhead = s;
    s->sectornumber = sno;
    s->to_be_written = 0;
    return s;
}

/* ------------------------------------------------------------------------ */
static int msdos_signature(struct sector *s)
{
    unsigned char  *data = (unsigned char *)s->data;

    if (data[510] == 0x55 && data[511] == 0xaa)
    {
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------------ */
static int write_sectors(char *dev, int fd)
{
    struct sector  *s;

    for (s = sectorhead; s; s = s->next)
    {
        if (s->to_be_written)
        {
            if (!sseek(dev, fd, s->sectornumber))
            {
                return 0;
            }
            if (write(fd, s->data, sizeof(s->data)) != sizeof(s->data))
            {
                perror("write");
                fflush(stdout);
                fprintf(stderr, "write error on %s - cannot write sector %llu\n", dev, s->sectornumber);
                return 0;
            }
            s->to_be_written = 0;
        }
    }
    return 1;
}

/* ------------------------------------------------------------------------ */
static chs longchs_to_chs(longchs aa, struct geometry G)
{
    chs             a;

    if (aa.h < 256 && aa.s < 64 && aa.c < 1024)
    {
        a.h = aa.h;
        a.s = aa.s | ((aa.c >> 2) & 0xc0);
        a.c = (aa.c & 0xff);
    }
    else if (G.heads && G.sectors)
    {
        a.h = G.heads - 1;
        a.s = G.sectors | 0xc0;
        a.c = 0xff;
    }
    else
    {
        a = zero_chs;
    }
    return a;
}

/* ------------------------------------------------------------------------ */
static longchs chs_to_longchs(chs a)
{
    longchs         aa;

    aa.h = a.h;
    aa.s = (a.s & 0x3f);
    aa.c = (a.s & 0xc0);
    aa.c = (aa.c << 2) + a.c;
    return aa;
}

/* ------------------------------------------------------------------------ */
static longchs ulong_to_longchs(unsigned long sno, struct geometry G)
{
    longchs         aa;

    if (G.heads && G.sectors && G.cylindersize)
    {
        aa.s = 1 + sno % G.sectors;
        aa.h = (sno / G.sectors) % G.heads;
        aa.c = sno / G.cylindersize;
        return aa;
    }
    else
    {
        return zero_longchs;
    }
}

/* ------------------------------------------------------------------------ */
static chs ulong_to_chs(unsigned long sno, struct geometry G)
{
    return longchs_to_chs(ulong_to_longchs(sno, G), G);
}

/* ------------------------------------------------------------------------ */
static int is_equal_chs(chs a, chs b)
{
    return (a.h == b.h && a.s == b.s && a.c == b.c);
}

/* ------------------------------------------------------------------------ */
static struct geometry get_fdisk_geometry_one(struct part_desc *p)
{
    struct geometry G;
    chs b = p->p.end_chs;
    longchs bb = chs_to_longchs(b);

    memset(&G, 0, sizeof(struct geometry));
    G.heads = bb.h + 1;
    G.sectors = bb.s;
    G.cylindersize = G.heads * G.sectors;
    return G;
}

/* ------------------------------------------------------------------------ */
static int get_fdisk_geometry(struct disk_desc *z)
{
    struct part_desc *p;
    int pno, agree;
    struct geometry G0, G;

    memset(&G0, 0, sizeof(struct geometry));
    agree = 0;
    for (pno = 0; pno < z->partno; pno++) {
        p = &(z->partitions[pno]);
        if (p->size != 0 && p->p.sys_type != 0)
        {
            G = get_fdisk_geometry_one(p);
            if (!G0.heads)
            {
                G0 = G;
                agree = 1;
            }
            else if (G.heads != G0.heads || G.sectors != G0.sectors)
            {
                agree = 0;
                break;
            }
        }
    }
    F = (agree ? G0 : B);
    return (F.sectors != B.sectors || F.heads != B.heads);
}

/* ------------------------------------------------------------------------ */
/* List of partition types */
static const char *sysname(unsigned char type)
{
    struct systypes *s;

    for (s = i386_sys_types; s->name; s++)
    {
        if (s->type == type)
        {
            return (s->name);
        }
    }
    return ("Unknown");
}

/* ------------------------------------------------------------------------ */
static int p_is_extended(unsigned char type)
{
    return (type == EXTENDED_PARTITION || type == LINUX_EXTENDED || type == WIN98_EXTENDED);
}

/* ------------------------------------------------------------------------ */
static int is_bsd(unsigned char type)
{
    return (type == BSD_PARTITION || type == NETBSD_PARTITION);
}

/* ------------------------------------------------------------------------ */
/* Unfortunately, partitions are not aligned, and non-Intel machines
   are unhappy with non-aligned integers. So, we need a copy by hand. */
static int copy_to_int(unsigned char *cp)
{
    unsigned int    m;

    m = *cp++;
    m += (*cp++ << 8);
    m += (*cp++ << 16);
    m += (*cp++ << 24);
    return m;
}

/* ------------------------------------------------------------------------ */
static void copy_from_int(int m, char *cp)
{
    *cp++ = (m & 0xff);
    m >>= 8;
    *cp++ = (m & 0xff);
    m >>= 8;
    *cp++ = (m & 0xff);
    m >>= 8;
    *cp++ = (m & 0xff);
}

/* ------------------------------------------------------------------------ */
static void copy_to_part(char *cp, struct partition *p)
{
    p->bootable = *cp++;
    p->begin_chs.h = *cp++;
    p->begin_chs.s = *cp++;
    p->begin_chs.c = *cp++;
    p->sys_type = *cp++;
    p->end_chs.h = *cp++;
    p->end_chs.s = *cp++;
    p->end_chs.c = *cp++;
    p->start_sect = copy_to_int((unsigned char *)cp);
    p->nr_sects = copy_to_int((unsigned char *)cp + 4);
}

/* ------------------------------------------------------------------------ */
static void copy_from_part(struct partition *p, char *cp)
{
    *cp++ = p->bootable;
    *cp++ = p->begin_chs.h;
    *cp++ = p->begin_chs.s;
    *cp++ = p->begin_chs.c;
    *cp++ = p->sys_type;
    *cp++ = p->end_chs.h;
    *cp++ = p->end_chs.s;
    *cp++ = p->end_chs.c;
    copy_from_int(p->start_sect, cp);
    copy_from_int(p->nr_sects, cp + 4);
}

/* ------------------------------------------------------------------------ */
/* find Linux name of this partition, assuming that it will have a name */
static int index_to_linux(int pno, struct disk_desc *z)
{
    int             i;
    int             ct = 1;
    struct part_desc *p = &(z->partitions[0]);

    for (i = 0; i < pno; i++, p++)
    {
        if (i < 4 || (p->size > 0 && !p_is_extended(p->p.sys_type)))
        {
            ct++;
        }
    }
    return ct;
}

/* ------------------------------------------------------------------------ */
static int linux_to_index(int lpno, struct disk_desc *z)
{
    int             i;
    int             ct = 0;
    struct part_desc *p = &(z->partitions[0]);

    for (i = 0; i < z->partno && ct < lpno; i++, p++)
    {
        if ((i < 4 || (p->size > 0 && !p_is_extended(p->p.sys_type))) && ++ct == lpno)
        {
            return i;
        }
    }
    return -1;
}

/* ------------------------------------------------------------------------ */
static void out_partition(char *dev, struct part_desc *p, struct disk_desc *z, struct geometry G)
{
    unsigned long long start;
    unsigned long long end;
    unsigned long long size;
    int             pno;
    int             lpno;

    pno = p - &(z->partitions[0]);     /* our index */
    lpno = index_to_linux(pno, z);     /* name of next one that has a name */
    if (pno == linux_to_index(lpno, z)) /* was that us? */
    {
        printf("%s ", partname(dev, lpno, 10));  /* yes */
    }
    else
    {
        return;
    }

    start = p->start;
    end = p->start + p->size - 1;
    size = p->size;

    if (p->ptype != DOS_TYPE || p->p.bootable == 0)
    {
        printf("   ");
    }
    else if (p->p.bootable == 0x80)
    {
        printf(" * ");
    }
    else
    {
        printf(" ? ");                 /* garbage */
    }

    printf("%*llu ", 9, start);
    if (end == (unsigned long long)(-1))
    {
        printf("%*s ", 9, "-");
    }
    else
    {
        printf("%*llu ", 9, end);
    }
    printf("%*llu ", 10, size);

    if (p->ptype == DOS_TYPE)
    {
        printf(" %2x  %s\n", p->p.sys_type, sysname(p->p.sys_type));
    }
    else
    {
        printf("\n");
    }

    /* Is chs as we expect? */
    if (p->ptype == DOS_TYPE)
    {
        chs             a;
        chs             b;
        longchs         aa;
        longchs         bb;

        a = (size ? ulong_to_chs(start, G) : zero_chs);
        b = p->p.begin_chs;
        aa = chs_to_longchs(a);
        bb = chs_to_longchs(b);
        if (a.s && !is_equal_chs(a, b))
        {
            warnx("\t\tstart: (c,h,s) expected (%ld,%ld,%ld) found (%ld,%ld,%ld)\n",
                  aa.c, aa.h, aa.s, bb.c, bb.h, bb.s);
        }
        a = (size ? ulong_to_chs(end, G) : zero_chs);
        b = p->p.end_chs;
        aa = chs_to_longchs(a);
        bb = chs_to_longchs(b);
        if (a.s && !is_equal_chs(a, b))
        {
            warnx("\t\tend: (c,h,s) expected (%ld,%ld,%ld) found (%ld,%ld,%ld)\n",
                  aa.c, aa.h, aa.s, bb.c, bb.h, bb.s);
        }
        if (G.cylinders && G.cylinders < 1024 && bb.c > G.cylinders)
        {
            warnx("partition ends on cylinder %ld, beyond the end of the disk\n", bb.c);
        }
    }
}

/* ------------------------------------------------------------------------ */
static void out_partitions(char *dev, struct disk_desc *z)
{
    int             pno;

    if (z->partno == 0)
    {
        warnx("No partitions found\n");
        return;
    }

    get_fdisk_geometry(z);
    printf(" C/H/S=*/%ld/%ld (below in sectors)\n",
           F.heads, F.sectors);

//    printf("Units: sectors of 512 bytes\n");
    printf("   Device Boot    Start       End   #sectors  Id  System\n");
    for (pno = 0; pno < z->partno; pno++)
    {
        out_partition(dev, &(z->partitions[pno]), z, F);
    }
}

/* ------------------------------------------------------------------------ */
static void extended_partition(char *dev, int fd, struct part_desc *ep, struct disk_desc *z)
{
    char           *cp;
    struct sector  *s;
    unsigned long long start;
    unsigned long long here;
    unsigned long long next;
    int             i;
    int             moretodo = 1;
    struct partition p;
    struct part_desc *partitions = &(z->partitions[0]);
    size_t          pno = z->partno;

    here = start = ep->start;

    while (moretodo)
    {
        moretodo = 0;
        if (!(s = get_sector(dev, fd, here)))
        {
            break;
        }
        if (!msdos_signature(s))
        {
            fflush(stdout);
            fprintf(stderr, "ERROR: sector %llu does not have an msdos signature\n", s->sectornumber);
            break;
        }
        cp = s->data + 0x1be;

        if (pno + 4 >= sizeof(struct part_desc))
        {
            warnx("too many partitions - ignoring those past nr (%zu)\n", pno - 1);
            break;
        }

        next = 0;

        for (i = 0; i < 4; i++, cp += sizeof(struct partition))
        {
            partitions[pno].sector = here;
            partitions[pno].offset = cp - s->data;
            partitions[pno].ep = ep;
            copy_to_part(cp, &p);
            if (p_is_extended(p.sys_type))
            {
                partitions[pno].start = start + p.start_sect;
                if (next)
                {
                    warnx("tree of partitions?\n");
                }
                else
                {
                    next = partitions[pno].start;       /* follow `upper' branch */
                }
                moretodo = 1;
            }
            else
            {
                partitions[pno].start = here + p.start_sect;
            }
            partitions[pno].size = p.nr_sects;
            partitions[pno].ptype = DOS_TYPE;
            partitions[pno].p = p;
            pno++;
        }
        here = next;
    }
    z->partno = pno;
}

/* ------------------------------------------------------------------------ */
static void bsd_partition(char *dev, int fd, struct part_desc *ep, struct disk_desc *z)
{
    struct bsd_disklabel *l;
    struct bsd_partition *bp;
    struct bsd_partition *bp0;
    unsigned long long start = ep->start;
    struct sector  *s;
    struct part_desc *partitions = &(z->partitions[0]);
    size_t          pno = z->partno;

    if (!(s = get_sector(dev, fd, start + 1)))
    {
        return;
    }
    l = (struct bsd_disklabel *)(s->data);
    if (l->d_magic != BSD_DISKMAGIC || l->d_magic2 != BSD_DISKMAGIC)
    {
        return;
    }

    bp = bp0 = &l->d_partitions[0];
    while (bp - bp0 < BSD_MAXPARTITIONS && bp - bp0 < l->d_npartitions)
    {
        if (pno + 1 >= sizeof(struct part_desc))
        {
            warnx("too many partitions - ignoring those " "past nr (%zu)\n", pno - 1);
            break;
        }
        if (bp->p_fstype != BSD_FS_UNUSED)
        {
            partitions[pno].start = bp->p_offset;
            partitions[pno].size = bp->p_size;
            partitions[pno].sector = start + 1;
            partitions[pno].offset = (char *)bp - (char *)bp0;
            partitions[pno].ep = 0;
            partitions[pno].ptype = BSD_TYPE;
            pno++;
        }
        bp++;
    }
    z->partno = pno;
}

/* ------------------------------------------------------------------------ */
static int msdos_partition(char *dev, int fd, unsigned long start, struct disk_desc *z)
{
    int             i;
    char           *cp;
    struct partition pt;
    struct sector  *s;
    struct part_desc *partitions = &(z->partitions[0]);
    int             pno = z->partno;
    int             bsd_later = 1;
    unsigned short  sig;
    unsigned short  magic;

    if (!(s = get_sector(dev, fd, start)))
    {
        return 0;
    }
    if (!msdos_signature(s))
    {
        return 0;
    }
    cp = s->data + 0x1be;
    copy_to_part(cp, &pt);

    /* If I am not mistaken, recent kernels will hide this from us,
     * so we will never actually see traces of a Disk Manager */
    if (pt.sys_type == DM6_PARTITION
        || pt.sys_type == EZD_PARTITION
        || pt.sys_type == DM6_AUX1PARTITION || pt.sys_type == DM6_AUX3PARTITION)
    {
        warnx("detected Disk Manager - unable to handle that\n");
        return 0;
    }

    memcpy(&sig, s->data + 2, sizeof(sig));
    if (sig <= 0x1ae)
    {
        memcpy(&magic, s->data + sig, sizeof(magic));
        if (magic == 0x55aa && (1 & *(unsigned char *)(s->data + sig + 2)))
        {
            warnx("DM6 signature found - giving up\n");
            return 0;
        }
    }

    for (pno = 0; pno < 4; pno++, cp += sizeof(struct partition))
    {
        partitions[pno].sector = start;
        partitions[pno].offset = cp - s->data;
        copy_to_part(cp, &pt);
        partitions[pno].start = start + pt.start_sect;
        partitions[pno].size = pt.nr_sects;
        partitions[pno].ep = 0;
        partitions[pno].p = pt;
    }

    z->partno = pno;

    for (i = 0; i < 4; i++)
    {
        if (p_is_extended(partitions[i].p.sys_type))
        {
            if (!partitions[i].size)
            {
                warnx("strange..., an extended partition of size 0?\n");
                continue;
            }
            extended_partition(dev, fd, &partitions[i], z);
        }
        if (!bsd_later && is_bsd(partitions[i].p.sys_type))
        {
            if (!partitions[i].size)
            {
                warnx("strange..., a BSD partition of size 0?\n");
                continue;
            }
            bsd_partition(dev, fd, &partitions[i], z);
        }
    }

    if (bsd_later)
    {
        for (i = 0; i < 4; i++)
        {
            if (is_bsd(partitions[i].p.sys_type))
            {
                if (!partitions[i].size)
                {
                    warnx("strange..., a BSD partition of size 0?\n");
                    continue;
                }
                bsd_partition(dev, fd, &partitions[i], z);
            }
        }
    }
    return 1;
}

/* ------------------------------------------------------------------------ */
static void get_partitions(char *dev, int fd, struct disk_desc *z)
{
    z->partno = 0;

    if (!msdos_partition(dev, fd, 0, z))
    {
        warnx(" %s: unrecognized partition table type\n", dev);
    }
}

/* ------------------------------------------------------------------------ */
/* Compute starting sector of a partition inside an extended one */
/* ep is 0 or points to surrounding extended partition */
static void compute_start_sect(struct part_desc *p)
{
    if (p->p.sys_type == EMPTY_PARTITION && p->size == 0)
    {
        p->p.start_sect = 0;
        p->p.begin_chs = zero_chs;
        p->p.end_chs = zero_chs;
    }
    else
    {
        p->p.start_sect = p->start;
        p->p.begin_chs = ulong_to_chs(p->start, F);
        p->p.end_chs = ulong_to_chs(p->start + p->size - 1, F);
    }
    p->p.nr_sects = p->size;
}

/* ------------------------------------------------------------------------ */
static int write_partitions(char *dev, int fd, struct disk_desc *z)
{
    struct sector    *s;
    struct part_desc *partitions = &(z->partitions[0]);
    struct part_desc *p;
    int               pno = z->partno;

    for (p = partitions; p < partitions + pno; p++)
    {
        s = get_sector(dev, fd, p->sector);
        if (!s)
        {
            return 0;
        }
        s->to_be_written = 1;
        if (p->ptype == DOS_TYPE)
        {
            if (p->p.start_sect != 0)
            {
                p->start += SECT_128MB;
                compute_start_sect(p);
            }
            copy_from_part(&(p->p), s->data + p->offset);
            s->data[510] = 0x55;
            s->data[511] = (unsigned char)0xaa;
        }
    }
    if (!write_sectors(dev, fd))
    {
        fflush(stdout);
        fprintf(stderr, "Failed writing the partition on %s\n", dev);
        return 0;
    }
    if (fsync(fd))
    {
        perror(dev);
        fflush(stdout);
        fprintf(stderr, "Failed writing the partition on %s\n", dev);
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------------ */
static void usage(FILE *out)
{
    fputs("\nUsage:\n", out);
    fprintf(out, " %s [options] <device> [...]\n", __FILE__);

    fputs("\nOptions:\n", out);
    fputs(" -h, --help                display this help text and exit\n", out);

    exit(out == stderr ? EXIT_FAILURE : EXIT_SUCCESS);
}

/* ------------------------------------------------------------------------ */
static void do_fdisk(char *dev)
{
    int             exit_status = 0;
    int             fd;
    struct stat     statbuf;
    struct disk_desc *z;

    if (stat(dev, &statbuf) < 0)
    {
        perror(dev);
        errx(EXIT_FAILURE, "Fatal error: cannot find %s", dev);
    }
    fd = open(dev, O_RDWR);
    if (fd < 0)
    {
        perror(dev);
        errx(EXIT_FAILURE, "cannot open %s read-write", dev);
    }

    z = &oldp;

    free_sectors();
    get_partitions(dev, fd, z);

    printf("Old partition table:");

    out_partitions(dev, z);

    if (write_partitions(dev, fd, z))
    {
        printf("Successfully wrote 128MB increased new partition table.\n");
    }
    else
    {
        exit_status = 1;
    }

    sync();                            /* superstition */
    sync();                            /* superstition */

    printf("\n");

    z = &oldp;

    free_sectors();
    get_partitions(dev, fd, z);

    printf("New partition table:");
    out_partitions(dev, z);

    exit(exit_status);
}

/* ------------------------------------------------------------------------ */
// Main program follows.
int main(int argc, char **argv)
{
    int             c;
    char           *dev;

    if (argc < 1)
    {
        errx(EXIT_FAILURE, "no command?");
    }

    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
    {
        switch (c)
        {
            case 'h':
                usage(stdout);
            default:
                usage(stderr);
        }
    }
    if (optind == argc)
    {
        usage(stderr);
    }

    if (optind != argc - 1)
    {
        errx(EXIT_FAILURE, "can specify only one device");
    }
    dev = argv[optind];

    do_fdisk(dev);

    return 0;
}

/* ------------------------------------------------------------------------ */
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
