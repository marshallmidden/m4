#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <time.h>

/* ------------------------------------------------------------------------ */
// #define INCLUDE_NVME
// #define NVME_PARTITIONS
/* ------------------------------------------------------------------------ */
#define FT_OTHER            (unsigned char)0
#define FT_BLOCK            (unsigned char)1
#define FT_CHAR             (unsigned char)2

#define TRANSPORT_UNKNOWN   0
#define TRANSPORT_FC        1
#define TRANSPORT_ISCSI     2

#define NVME_HOST_NUM       0x7fff              /* 32767, high to avoid SCSI host numbers */

#ifdef PATH_MAX
#define LMAX_PATH           PATH_MAX
#else
#define PATH_MAX            2048
#define LMAX_PATH           2048
#endif

#ifdef NAME_MAX
#define LMAX_NAME    (NAME_MAX + 1)
#else
#define LMAX_NAME    256
#endif

#define LMAX_DEVPATH (LMAX_NAME + 128)
#define UINT64_LAST  ((uint64_t) ~0)

/* ------------------------------------------------------------------------ */
/*
   For SCSI 'h' is host_num, 'c' is channel, 't' is target, 'l' is LUN is
   uint64_t and lun_arr[8] is LUN as 8 byte array. For NVMe, h=0x7fff
   (NVME_HOST_NUM) and displayed as 'N'; 'c' is Linux's NVMe controller
   number, 't' is NVMe Identify controller CTNLID field, and 'l' is
   namespace id (1 to (2**32)-1) rendered as a little endian 4 byte sequence
   in lun_arr, last 4 bytes are zeros. invalidate_hctl() puts -1 in
   integers, 0xff in bytes.
 */
struct addr_hctl
{
    int      h;                                 /* if h==0x7fff, display as 'N' for NVMe */
    int      c;
    int      t;
    uint64_t l;                                 /* SCSI: Linux word flipped; NVME: uint32_t */
    uint8_t  lun_arr[8];                        /* T10, SAM-5 order; NVME: little endian */
};

static const char /*@observer@*/ *scsi_short_device_types[32] =
{
    "disk   ", "tape   ", "printer", "process", "worm   ", "cd/dvd ",
    "scanner", "optical", "mediumx", "comms  ", "(0xa)  ", "(0xb)  ",
    "storage", "enclosu", "sim dsk", "opti rd", "bridge ", "osd    ",
    "adi    ", "sec man", "zbc    ", "(0x15) ", "(0x16) ", "(0x17) ",
    "(0x18) ", "(0x19) ", "(0x1a) ", "(0x1b) ", "(0x1c) ", "(0x1e) ",
    "wlun   ", "no dev "
};

/* Device node list: contains the information needed to match a node with a sysfs class device. */
#define DEV_NODE_LIST_ENTRIES 16U
enum dev_type
{
    BLK_DEV, CHR_DEV
};

struct dev_node_entry
{
    unsigned int  maj;
    unsigned int  min;
    enum dev_type type;
    time_t        mtime;
    char          name[LMAX_DEVPATH];
};

struct dev_node_list
{
    struct dev_node_list *next;
    unsigned int          count;
    struct dev_node_entry nodes[DEV_NODE_LIST_ENTRIES];
};

/*
   WWN here is extracted from /dev/disk/by-id/wwn-<WWN> which is
   created by udev 60-persistent-storage.rules using ID_WWN_WITH_EXTENSION.
   The udev ID_WWN_WITH_EXTENSION is the combination of char wwn[17] and
   char wwn_vendor_extension[17] from struct scsi_id_device. This macro
   defines the maximum length of char-array needed to store this wwn including
   the null-terminator.
 */
#define DISK_WWN_MAX_LEN 35

struct disk_wwn_node_entry
{
    char wwn[DISK_WWN_MAX_LEN];                 /* '0x' + wwn<128-bit> + */
    /* <null-terminator> */
    char disk_bname[12];
};

#define DISK_WWN_NODE_LIST_ENTRIES 16U
struct disk_wwn_node_list
{
    struct disk_wwn_node_list *next;
    unsigned int               count;
    struct disk_wwn_node_entry nodes[DISK_WWN_NODE_LIST_ENTRIES];
};

struct item_t
{
    char     name[LMAX_NAME];
    unsigned char ft;
    unsigned char d_type;
};
static struct item_t non_sg;
static struct item_t aa_sg;
static struct item_t aa_first;

/* These are okay, due to their scope range. */
static const char             *iscsi_dir_name;
static const struct addr_hctl *iscsi_target_hct;
static int  iscsi_tsession_num;

/* ------------------------------------------------------------------------ */
#ifdef INCLUDE_NVME
/*
   Want safe, 'n += snprintf(b + n, blen - n, ...)' style sequence of
   functions. Returns number number of chars placed in cp excluding the
   trailing null char. So for cp_max_len > 0 the return value is always
   < cp_max_len; for cp_max_len <= 1 the return value is 0 and no chars
   are written to cp. Note this means that when cp_max_len = 1, this
   function assumes that cp[0] is the null character and does nothing
   (and returns 0).
 */
/* GNU prototyping must be separate than function. */
static int scnpr(char *cp, size_t cp_max_len, const char *fmt, ...)
                 __attribute__ ((format(printf, 3, 4)));
static int scnpr(char *cp, size_t cp_max_len, const char *fmt, ...)
{
    va_list args;
    int     n;

    if (cp_max_len < 2)
    {
        return(0);
    }

    va_start(args, fmt);
    n = vsnprintf(cp, cp_max_len, fmt, args);
    va_end(args);
    return((n < (int)cp_max_len) ? n : (int)cp_max_len - 1);
}   /* End of scnpr */
#endif  // INCLUDE_NVME

/* ------------------------------------------------------------------------ */
/* trims whitespace. */
static size_t trim_lead_trail(char *s)
{
    size_t n;
    char  *p = s;

    /* sanity checks */
    if (s == NULL)
    {
        return(0);
    }
    n = strlen(p);
    if (n == 0)
    {
        return(0);
    }
    do
    {
        if (p[n - 1] != '\n')
        {
            break;
        }
        /* Toss trailing newline. */
        n--;
        p[n] = '\0';
    } while (n != 0);

    while (n > 0 && isspace((int)p[n - 1]) != 0)
    {
        p[--n] = '\0';
    }

    while (*p != '\0' && isspace((int)*p) != 0)
    {
        ++p;
        --n;
    }
    memmove(s, p, n + 1);                       /* Get rid of leading spaces. */
    return(n);
}   /* End of trim_lead_trail */

/* ------------------------------------------------------------------------ */
/*
   Returns true if dirent entry is either a symlink or a directory
   starting_with given name. If starting_with is NULL choose all that are
   either symlinks or directories other than . or .. (own directory or
   parent) . Can be tricked cause symlink could point to .. (parent), for
   example. Otherwise return false.
 */
static bool dir_or_link(const struct dirent *s, const char *starting_with)
{
    size_t len = strlen(s->d_name);

    if ((unsigned char)DT_LNK == s->d_type)
    {
        if (starting_with != NULL)
        {
            return(0 == strncmp(s->d_name, starting_with, strlen(starting_with)));
        }
        return(true);
    }
    if ((unsigned char)DT_DIR != s->d_type)
    {
        return(false);
    }

    /* Assume can't have zero length directory name */
    if (starting_with != NULL)
    {
        return(0 == strncmp(s->d_name, starting_with, strlen(starting_with)));
    }
    if (len > 2)
    {
        return(true);
    }
    if (s->d_name[0] == '.')
    {
        if (len == 1)
        {
            return(false);                  /* this directory: '.' */
        }
        if ('.' == s->d_name[1])
        {
            return(false);                  /* parent: '..' */
        }
    }
    return(true);
}   /* End of dir_or_link */

/* ------------------------------------------------------------------------ */
/*
   Copies (dest_maxlen - 1) or less chars from src to dest. Less chars are
   copied if '\0' char found in src. As long as dest_maxlen > 0 then dest
   will be '\0' terminated on exit. If dest_maxlen < 1 then does nothing.
 */
static void my_strcopy(/*@out@*/ char *dest, const char *src, size_t dest_maxlen)
{
    const char *lp;

    dest[0] = '\0';
    if (dest_maxlen < 1)
    {
        return;
    }

    /* If no terminating null character. */
    lp = (const char *)memchr(src, 0, dest_maxlen);

    if (lp == NULL)
    {
        memcpy(dest, src, dest_maxlen - 1);
        /* Set a terminating null character. */
        dest[dest_maxlen - 1] = '\0';
    }
    else
    {
        /* Copy the terminating null character. */
        memcpy(dest, src, (lp - src) + 1);
    }
}   /* End of my_strcopy */

/* ------------------------------------------------------------------------ */
#ifdef INCLUDE_NVME
static uint64_t lun_word_flip(uint64_t in)
{
    int      k;
    uint64_t res = 0;

    for (k = 0; ; ++k)
    {
        res |= (in & 0xffff);
        if (k > 2)
        {
            break;
        }
        res <<= 16;
        in >>= 16;
    }
    return(res);
}   /* End of lun_word_flip */

/* ------------------------------------------------------------------------ */
static void tag_lun_helper(int *tag_arr, int kk, int num)
{
    int j;

    for (j = 0; j < num; ++j)
    {
        tag_arr[(2 * kk) + j] = ((kk > 0) && (0 == j)) ? 2 : 1;
    }
}   /* End of tag_lun_helper */

/* ------------------------------------------------------------------------ */
/*
   Tag lun bytes according to SAM-5 rev 10. Write output to tag_arr assumed
   to have at least 8 ints. 0 in tag_arr means this position and higher can
   be ignored; 1 means print as is; 2 means print with separator
   prefixed. Example: lunp: 01 22 00 33 00 00 00 00 generates tag_arr
   of 1, 1, 2, 1, 0 ... 0 and might be printed as 0x0122_0033.
 */
static void tag_lun(const uint8_t *lunp, int *tag_arr)
{
    bool    next_level;
    int     k;
    uint8_t a_method;
    uint8_t bus_id;
    uint8_t len_fld;
    uint8_t e_a_method;
    uint8_t not_spec[2] = { 0xff, 0xff };

    if (tag_arr == NULL)
    {
        return;
    }
    if (lunp == NULL)
    {
        return;
    }
    if (memcmp(lunp, not_spec, sizeof(not_spec)) == 0)
    {
        for (k = 0; k < 2; ++k)
        {
            tag_arr[k] = 1;
        }
        return;
    }

    for (k = 0; k < 4; ++k, lunp += 2)
    {
        next_level = false;
        a_method = (lunp[0] >> 6) & 0x3;

        switch (a_method)
        {
          case 0:                               /* peripheral device addressing method */
              bus_id = lunp[0] & 0x3f;
              if (bus_id != 0)
              {
                  next_level = true;
              }
              tag_lun_helper(tag_arr, k, 2);
              break;

          case 1:                               /* flat space addressing method */
              tag_lun_helper(tag_arr, k, 2);
              break;

          case 2:                               /* logical unit addressing method */
              tag_lun_helper(tag_arr, k, 2);
              break;

          case 3:                               /* extended logical unit addressing method */
              len_fld = (lunp[0] & 0x30) >> 4;
              e_a_method = lunp[0] & 0xf;
              if (len_fld == 0 && e_a_method == 1)
              {
                  tag_lun_helper(tag_arr, k, 2);
              }
              else if (len_fld == 1 && e_a_method == 2)
              {
                  tag_lun_helper(tag_arr, k, 4);
              }
              else if (len_fld == 2 && e_a_method == 2)
              {
                  tag_lun_helper(tag_arr, k, 6);
              }
              else if (len_fld == 3 && e_a_method == 0xf)
              {
                  tag_arr[2 * k] = (k > 0) ? 2 : 1;
              }
              else
              {
                  if (len_fld < 2)
                  {
                      tag_lun_helper(tag_arr, k, 4);
                  }
                  else
                  {
                      tag_lun_helper(tag_arr, k, 6);
                      if (len_fld == 3)
                      {
                          tag_arr[(2 * k) + 6] = 1;
                          tag_arr[(2 * k) + 7] = 1;
                      }
                  }
              }
              break;

          default:
              tag_lun_helper(tag_arr, k, 2);
              break;
        }
        if (!next_level)
        {
            break;
        }
    }
}   /* End of tag_lun */
#endif  // INCLUDE_NVME

/* ------------------------------------------------------------------------ */
#ifdef INCLUDE_NVME
/*
   Bits 3, 2, 1, 0 in sel_mask select the h, c, t, l components respectively.
   Bits 4+5 of sel_mask convey the --lunhex option selecting l (LUN) in
   hex. Generates string of the form %d:%d:%d with a colon between
   components, returns 4th argument.
 */
static void tuple2string(const struct addr_hctl *tp, int sel_mask, size_t blen, char *b)
{
    bool got1 = false;
    bool is_nvme = (NVME_HOST_NUM == tp->h);
    int  n = 0;

    if ((0x8 & sel_mask) != 0)
    {
        if (is_nvme)
        {
            n += scnpr(b + n, blen - n, "N");
        }
        else
        {
            n += scnpr(b + n, blen - n, "%d", tp->h);
        }
        got1 = true;
    }
    if ((0x4 & sel_mask) != 0)
    {
        n += scnpr(b + n, blen - n, "%s%d", got1 ? ":" : "", tp->c);
        got1 = true;
    }
    if ((0x2 & sel_mask) != 0)
    {
        n += scnpr(b + n, blen - n, "%s%d", got1 ? ":" : "", tp->t);
        got1 = true;
    }

    if ((!is_nvme) && (0x1 & sel_mask) != 0)
    {
        uint8_t lunhex = ((uint16_t)sel_mask >> 4) & 0x3;

        switch (lunhex)
        {
          case 1:
            {                                       /* -x (--lunhex) format */
                int ta;
                int k;
                int tag_arr[16];

                memset(&tag_arr[0], 0, sizeof(tag_arr));
                n += scnpr(b + n, blen - n, "%s0x", got1 ? ":" : "");
                tag_lun(tp->lun_arr, tag_arr);

                for (k = 0; k < 8; ++k)
                {
                    ta = tag_arr[k];
                    if (ta <= 0)
                    {
                        /*@loopbreak@*/
                        break;
                    }
                    n += scnpr(b + n, blen - n, "%s%02x", ta > 1 ? "_" : "",
                               tp->lun_arr[k]);
                }
            }
            break;

          case 2:
          case 3:
            if (got1)
            {
                n += scnpr(b + n, blen - n, ":0x%016lx", lun_word_flip(tp->l));
            }
            else
            {
                n += scnpr(b + n, blen - n, "0x%016lx", lun_word_flip(tp->l));
            }
            break;

          case 0:
            if (tp->l == UINT64_LAST)
            {
                n += scnpr(b + n, blen - n, "%s", got1 ? ":-1" : "-1");
            }
            else
            {
                if (got1)
                {
                    n += scnpr(b + n, blen - n, ":0x%016lx", tp->l);
                }
                else
                {
                    n += scnpr(b + n, blen - n, "0x%016lx", tp->l);
                }
            }
            break;
        }
        return;
    }

    if ((0x1 & sel_mask) != 0)
    {                                           /* now must be NVMe */
        uint8_t lunhex = ((uint16_t)sel_mask >> 4) & 0x3;

        if (lunhex == 1)
        {                                       /* -x (--lunhex) format */
            n += scnpr(b + n, blen - n, "%s0x", got1 ? ":" : "");
            n += scnpr(b + n, blen - n, "%04x", (uint32_t)tp->l);
        }
        else if (lunhex > 1)
        {                                       /* -xx (--lunhex twice) */
            n += scnpr(b + n, blen - n, "%s0x", got1 ? ":" : "");
            n += scnpr(b + n, blen - n, "%08x", (uint32_t)tp->l);
        }
        else if (tp->l == UINT32_MAX)
        {
            n += scnpr(b + n, blen - n, "%s", got1 ? ":-1" : "-1");
        }
        else
        {
            n += scnpr(b + n, blen - n, "%s%x", got1 ? ":" : "", (uint32_t)tp->l);
        }
    }
    return;
}   /* End of tuple2string */
#endif  // INCLUDE_NVME

/* ------------------------------------------------------------------------ */
/* Compare <host:controller:target:lun> tuples (aka <h:c:t:l> or hctl) */
static int cmp_hctl(const struct addr_hctl *le, const struct addr_hctl *ri)
{
    if (le->h == ri->h)
    {
        if (le->c == ri->c)
        {
            if (le->t == ri->t)
            {
                return((le->l == ri->l) ? 0 : ((le->l < ri->l) ? -1 : 1));
            }
            return((le->t < ri->t) ? -1 : 1);
        }
        return((le->c < ri->c) ? -1 : 1);
    }
    return((le->h < ri->h) ? -1 : 1);
}   /* End of cmp_hctl */

/* ------------------------------------------------------------------------ */
/*
   Return 1 for directory entry that is link or directory (other than
   a directory name starting with dot). Else return 0.
 */
static int first_dir_scan_select(const struct dirent *s)
{
    if (aa_first.ft != FT_OTHER)
    {
        return(0);
    }
    if (!dir_or_link(s, (const char *)NULL))
    {
        return(0);
    }
    my_strcopy(aa_first.name, s->d_name, LMAX_NAME);
    aa_first.ft = FT_CHAR;              /* dummy */
    aa_first.d_type = s->d_type;
    return(1);
}   /* End of first_dir_scan_select */

/* ------------------------------------------------------------------------ */
/* Selects symlinks and directories that don't start with "." */
static int sub_dir_scan_select(const struct dirent *s)
{
    return((dir_or_link(s, (const char *)NULL)) ? 1 : 0);
}   /* End of sub_dir_scan_select */

/* ------------------------------------------------------------------------ */
/*
   Return 1 for directory entry that is link or directory (other than a
   directory name starting with dot) that contains "block". Else return 0.
 */
static int block_dir_scan_select(const struct dirent *s)
{
    return((dir_or_link(s, "block")) ? 1 : 0);
}   /* End of block_dir_scan_select */

/* ------------------------------------------------------------------------ */
typedef int (*dirent_select_fn)(const struct dirent *);

/*
   Scans directory dir_name, selecting elements on the basis of fn (NULL
   select all), into an unsorted list. The first item is assumed to be
   directories (or symlinks to) and it is appended, after a '/' to dir_name.
   Then if sub_str is found in that dir_name, it selects items that are
   directories or symlinks, the first of which is appended, after a '/',
   to dir_name. If conditions are met true is return, elae false.
 */
static bool sub_scan(char *dir_name, const char *sub_str, dirent_select_fn fn)
{
    int             num;
    int             k;
    size_t          len;
    struct dirent **name_list;

    num = scandir(dir_name, &name_list, fn, NULL);
    if (num <= 0)
    {
        return(false);
    }

    len = strlen(dir_name);
    if (len >= LMAX_PATH)
    {
        return(false);
    }

    snprintf(dir_name + len, LMAX_PATH - len, "/%s", name_list[0]->d_name);

    for (k = 0; k < num; ++k)
    {
        free(name_list[k]);
    }
    free(name_list);

    if (strstr(dir_name, sub_str) == NULL)
    {
        num = scandir(dir_name, &name_list, sub_dir_scan_select, NULL);
        if (num <= 0)
        {
            return(false);
        }

        len = strlen(dir_name);
        if (len >= LMAX_PATH)
        {
            return(false);
        }

        snprintf(dir_name + len, LMAX_PATH - len, "/%s", name_list[0]->d_name);

        for (k = 0; k < num; ++k)
        {
            free(name_list[k]);
        }
        free(name_list);
    }
    return(true);
}   /* End of sub_scan */

/* ------------------------------------------------------------------------ */
/*
   Scan for block:sdN or block/sdN directory in
   /sys/bus/scsi/devices/h:c:i:l
 */
static bool block_scan(char *dir_name)
{
    return(sub_scan(dir_name, "block:", block_dir_scan_select));
}   /* End of block_scan */

/* ------------------------------------------------------------------------ */
/*
   Scan for directory entry that is either a symlink or a directory. Returns
   number found or -1 for error.
 */
static int scan_for_first(const char *dir_name)
{
    int             num;
    int             k;
    struct dirent **name_list;

    aa_first.ft = FT_OTHER;
    num = scandir(dir_name, &name_list, first_dir_scan_select, NULL);
    if (num < 0)
    {
        char errpath[LMAX_PATH];

        snprintf(errpath, sizeof(errpath), "%s: scandir: %s", __func__, dir_name);
        perror(errpath);
        return(-1);
    }

    for (k = 0; k < num; ++k)
    {
        free(name_list[k]);
    }
    free(name_list);
    return(num);
}   /* End of scan_for_first */

/* ------------------------------------------------------------------------ */
static int non_sg_dir_scan_select(const struct dirent *s)
{
    size_t len;

    if (non_sg.ft != FT_OTHER)
    {
        return(0);
    }
    if (!dir_or_link(s, (const char *)NULL))
    {
        return(0);
    }

    if (strncmp("scsi_changer", s->d_name, 12) == 0)
    {
        my_strcopy(non_sg.name, s->d_name, LMAX_NAME);
        non_sg.ft = FT_CHAR;
        non_sg.d_type = s->d_type;
        return(1);
    }
    if (strncmp("block", s->d_name, 5) == 0)
    {
        my_strcopy(non_sg.name, s->d_name, LMAX_NAME);
        non_sg.ft = FT_BLOCK;
        non_sg.d_type = s->d_type;
        return(1);
    }
    if (strcmp("tape", s->d_name) == 0)
    {
        my_strcopy(non_sg.name, s->d_name, LMAX_NAME);
        non_sg.ft = FT_CHAR;
        non_sg.d_type = s->d_type;
        return(1);
    }
    if (strncmp("scsi_tape:st", s->d_name, 12) == 0)
    {
        len = strlen(s->d_name);
        if (isdigit((int)s->d_name[len - 1]) != 0)
        {
            /* want 'st<num>' symlink only */
            my_strcopy(non_sg.name, s->d_name, LMAX_NAME);
            non_sg.ft = FT_CHAR;
            non_sg.d_type = s->d_type;
            return(1);
        }
        return(0);
    }
    if (strncmp("onstream_tape:os", s->d_name, 16) == 0)
    {
        my_strcopy(non_sg.name, s->d_name, LMAX_NAME);
        non_sg.ft = FT_CHAR;
        non_sg.d_type = s->d_type;
        return(1);
    }
    return(0);
}   /* End of non_sg_dir_scan_select */

/* ------------------------------------------------------------------------ */
/* Returns number found or -1 for error */
static int non_sg_scan(const char *dir_name)
{
    int             num;
    int             k;
    struct dirent **name_list;

    non_sg.ft = FT_OTHER;
    num = scandir(dir_name, &name_list, non_sg_dir_scan_select, NULL);
    if (num < 0)
    {
        char errpath[LMAX_PATH];

        snprintf(errpath, sizeof(errpath), "%s: scandir: %s", __func__, dir_name);
        perror(errpath);
        return(-1);
    }

    for (k = 0; k < num; ++k)
    {
        free(name_list[k]);
    }
    free(name_list);
    return(num);
}   /* End of non_sg_scan */

/* ------------------------------------------------------------------------ */
static int sg_dir_scan_select(const struct dirent *s)
{
    if (aa_sg.ft != FT_OTHER)
    {
        return(0);
    }
    if (dir_or_link(s, "scsi_generic"))
    {
        my_strcopy(aa_sg.name, s->d_name, LMAX_NAME);
        aa_sg.ft = FT_CHAR;
        aa_sg.d_type = s->d_type;
        return(1);
    }
    return(0);
}   /* End of sg_dir_scan_select */

/* ------------------------------------------------------------------------ */
/*
   Returns number of directories or links starting with "scsi_generic"
   found or -1 for error.
 */
static int sg_scan(const char *dir_name)
{
    int             num;
    int             k;
    struct dirent **name_list;

    aa_sg.ft = FT_OTHER;
    num = scandir(dir_name, &name_list, sg_dir_scan_select, NULL);
    if (num < 0)
    {
        return(-1);
    }

    for (k = 0; k < num; ++k)
    {
        free(name_list[k]);
    }
    free(name_list);
    return(num);
}   /* End of sg_scan */

/* ------------------------------------------------------------------------ */
static int iscsi_target_dir_scan_select(const struct dirent *s)
{
    size_t      off;
    char        buff[LMAX_PATH];
    struct stat a_stat;

    if (dir_or_link(s, "session"))
    {
        iscsi_tsession_num = atoi(s->d_name + 7);
        my_strcopy(buff, iscsi_dir_name, LMAX_PATH);
        off = strlen(buff);
        snprintf(buff + off, sizeof(buff) - off,
                 "/%s/target%d:%d:%d", s->d_name, iscsi_target_hct->h,
                 iscsi_target_hct->c, iscsi_target_hct->t);

        if (stat(buff, &a_stat) >= 0 && S_ISDIR((mode_t)a_stat.st_mode))
        {
            return(1);
        }
        return(0);
    }
    return(0);
}   /* End of iscsi_target_dir_scan_select */

/* ------------------------------------------------------------------------ */
static int iscsi_target_scan(const char *dir_name, const struct addr_hctl *hctl)
{
    int             num;
    int             k;
    struct dirent **name_list;

    iscsi_dir_name = dir_name;
    iscsi_target_hct = hctl;
    iscsi_tsession_num = -1;
    num = scandir(dir_name, &name_list, iscsi_target_dir_scan_select, NULL);
    if (num < 0)
    {
        return(-1);
    }

    for (k = 0; k < num; ++k)
    {
        free(name_list[k]);
    }
    free(name_list);
    return(num);
}   /* End of iscsi_target_scan */

/* ------------------------------------------------------------------------ */
/*
   If 'dir_name'/'base_name' is a directory chdir to it. If that is successful
   return true, else false
 */
static bool if_directory_chdir(const char *dir_name, const char *base_name)
{
    char        b[LMAX_PATH];
    struct stat a_stat;

    snprintf(b, sizeof(b), "%s/%s", dir_name, base_name);
    if (stat(b, &a_stat) < 0)
    {
        return(false);
    }
    if (S_ISDIR((mode_t)a_stat.st_mode))
    {
        if (chdir(b) < 0)
        {
            return(false);
        }
        return(true);
    }
    return(false);
}   /* End of if_directory_chdir */

/* ------------------------------------------------------------------------ */
/*
   If 'dir_name'/generic is a directory chdir to it. If that is successful
   return true. Otherwise look a directory of the form
   'dir_name'/scsi_generic:sg<n> and if found chdir to it and return true.
   Otherwise return false.
 */
static bool if_directory_ch2generic(const char *dir_name)
{
    char        b[LMAX_PATH];
    struct stat a_stat;

    snprintf(b, sizeof(b), "%s/%s", dir_name, "generic");
    if (stat(b, &a_stat) >= 0 && S_ISDIR((mode_t)a_stat.st_mode))
    {
        if (chdir(b) < 0)
        {
            return(false);
        }
        return(true);
    }

    /* No "generic", so now look for "scsi_generic:sg<n>" */
    if (sg_scan(dir_name) != 1)
    {
        return(false);
    }

    snprintf(b, sizeof(b), "%s/%s", dir_name, aa_sg.name);
    if (stat(b, &a_stat) < 0)
    {
        return(false);
    }
    if (S_ISDIR((mode_t)a_stat.st_mode))
    {
        if (chdir(b) < 0)
        {
            return(false);
        }
        return(true);
    }
    return(false);
}   /* End of if_directory_ch2generic */

/* ------------------------------------------------------------------------ */
/*
   If 'dir_name'/'base_name' is found places corresponding value in 'value'
   and returns true . Else returns false.
 */
static bool get_value(const char *dir_name, const char *base_name, 
                      /*@out@*/ char *value,
                      size_t max_value_len)
{
    int     fd;
    char    b[LMAX_PATH];
    ssize_t n;

    snprintf(b, sizeof(b), "%s/%s", dir_name, base_name);

    value[0] = '\0';                            /* assume empty */
    if ((fd = open(b, O_RDONLY)) < 0)
    {
//        perror("open");                       // Commented out - no output for errors.
        return(false);
    }

    n = read(fd, value, max_value_len);
    if (n < 0)     /* if error */
    {
//        perror("read");                       // Commented out - no output for errors.
        close(fd);
        return(true);
    }
    if (n < (int)max_value_len)
    {
        value[n] = '\0';                        /* Make sure last character is null. */
    }
    else
    {
        value[max_value_len - 1] = '\0';        /* Make sure last character is null. */
    }

    close(fd);
    (void)trim_lead_trail(value);
    return(true);
}   /* End of get_value */

/* ------------------------------------------------------------------------ */
#ifdef INCLUDE_NVME
/*
   If 'dir_name'/'base_name' is found print base_name=%s of value.
 */
static void print_value_as_is(const char *dir_name, const char *base_name)
{
    char        buff[LMAX_PATH];
    bool        ret;

    ret = get_value(dir_name, base_name, buff, sizeof(buff));
    if (ret)
    {
        printf("%s\n",  buff);
    }
    else
    {
        printf("-\n");
    }
}   /* End of print_value_as_is */
#endif  // INCLUDE_NVME

/* ------------------------------------------------------------------------ */
/*
   If 'dir_name'/'base_name' is found print base_name=%s of value.
 */
static void print_value_exists(const char *dir_name, const char *base_name)
{
    char        buff[LMAX_PATH];
    bool        ret;

    ret = get_value(dir_name, base_name, buff, sizeof(buff));
    if (ret)
    {
        printf("%s=%s\n", base_name, buff);
    }
}   /* End of print_value_exists */

/* ------------------------------------------------------------------------ */
/*
   Allocate dev_node_list and collect info on every char and block devices
   in /dev but not its subdirectories. This list excludes symlinks, even if
   they are to devices.
 */
static void collect_dev_nodes(struct dev_node_list **dev_node_listhead)
{
    struct dirent         *dep;
    DIR                   *dirp;
    struct dev_node_list  *cur_list;
    struct dev_node_list  *prev_list;
    struct dev_node_entry *cur_ent;
    char                   device_path[LMAX_DEVPATH];
    struct stat            stats;

    if (*dev_node_listhead != NULL)
    {
        return;                                 /* already collected nodes */
    }

    *dev_node_listhead = (struct dev_node_list *)malloc(sizeof(struct dev_node_list));
    if (*dev_node_listhead == NULL)
    {
        exit(EXIT_FAILURE);
    }
    memset(*dev_node_listhead, 0, sizeof(struct dev_node_list));

    cur_list = *dev_node_listhead;
//    cur_list->next = NULL;
//    cur_list->count = 0;

    dirp = opendir("/dev");
    if (dirp == NULL)
    {
        return;
    }

    while (true)
    {
        dep = readdir(dirp);
        if (dep == NULL)
        {
            break;
        }

        snprintf(device_path, sizeof(device_path), "/dev/%s", dep->d_name);
        /* device_path[LMAX_PATH] = '\0'; */

        /* This will bypass all symlinks in /dev */
        if (lstat(device_path, &stats) < 0)
        {
            continue;
        }

        /* Skip non-block/char files. */
        if (!S_ISBLK((mode_t)stats.st_mode) && !S_ISCHR((mode_t)stats.st_mode))
        {
            continue;
        }

        /* Add to the list. */
        if (cur_list->count >= DEV_NODE_LIST_ENTRIES)
        {
            prev_list = cur_list;
            cur_list = (struct dev_node_list *)malloc(sizeof(struct dev_node_list));
            if (cur_list == NULL)
            {
                exit(EXIT_FAILURE);
            }
            memset(cur_list, 0, sizeof(struct dev_node_list));
            prev_list->next = cur_list;
//            cur_list->next = NULL;
//            cur_list->count = 0;
        }
        cur_ent = &cur_list->nodes[cur_list->count];
        cur_ent->maj = major(stats.st_rdev);
        cur_ent->min = minor(stats.st_rdev);

        if (S_ISBLK((mode_t)stats.st_mode))
        {
            cur_ent->type = BLK_DEV;
        }
        else if (S_ISCHR((mode_t)stats.st_mode))
        {
            cur_ent->type = CHR_DEV;
        }
        cur_ent->mtime = stats.st_mtime;
        my_strcopy(cur_ent->name, device_path, sizeof(cur_ent->name));

        cur_list->count++;
    }
    closedir(dirp);
}   /* End of collect_dev_nodes */

/* ------------------------------------------------------------------------ */
/* Free dev_node_list. */
static void free_dev_node_list(struct dev_node_list **dev_node_listhead)
{
    if (*dev_node_listhead != NULL)
    {
        struct dev_node_list *cur_list;
        struct dev_node_list *next_list;

        cur_list = *dev_node_listhead;
        while (cur_list != NULL)
        {
            next_list = cur_list->next;
            free(cur_list);
            cur_list = next_list;
        }
        *dev_node_listhead = NULL;
    }
}   /* End of free_dev_node_list */

/* ------------------------------------------------------------------------ */
/*
   Given a path to a class device, find the most recent device node with
   matching major/minor and type. Outputs to node which is assumed to be at
   least LMAX_NAME bytes long. Returns true if match found, false otherwise.
 */
static bool get_dev_node(const char *wd, /*@out@*/ char *node, enum dev_type type,
                         struct dev_node_list **dev_node_listhead)
{
    bool                   match_found = false;
    unsigned int           k = 0;
    unsigned int           maj;
    unsigned int           min;
    time_t                 newest_mtime = 0;
    struct dev_node_entry *cur_ent;
    struct dev_node_list  *cur_list;
    char                   value[LMAX_NAME];

    /* Assume 'node' is at least 2 bytes long. */
    memcpy(node, "-", 2);

    if (*dev_node_listhead == NULL)
    {
        collect_dev_nodes(dev_node_listhead);
        if (*dev_node_listhead == NULL)
        {
            return(false);
        }
    }

    /* Get the major/minor for this device. */
    if (!get_value(wd, "dev", value, LMAX_NAME))
    {
        return(false);
    }
    sscanf(value, "%u:%u", &maj, &min);

    /* Search the node list for the newest match on this major/minor. */
    cur_list = *dev_node_listhead;
    while (true)
    {
        if (k >= cur_list->count)
        {
            cur_list = cur_list->next;
            if (cur_list == NULL)
            {
                break;
            }
            k = 0;
        }
        cur_ent = &cur_list->nodes[k];
        k++;

        if (maj == cur_ent->maj && min == cur_ent->min && type == cur_ent->type)
        {
            if (!match_found ||
                difftime(cur_ent->mtime, newest_mtime) > 0.0)
            {
                newest_mtime = cur_ent->mtime;
                my_strcopy(node, cur_ent->name, LMAX_NAME);
            }
            match_found = true;
        }
    }

    return(match_found);
}   /* End of get_dev_node */

/* ------------------------------------------------------------------------ */
/*
   Allocate disk_wwn_node_list and collect info on every node in
   /dev/disk/by-id/scsi-* that does not contain "part" . Returns
   number of wwn nodes collected, 0 for already collected and
   -1 for error.
 */
static void collect_disk_wwn_nodes(struct disk_wwn_node_list **disk_wwn_node_listhead)
{
    ssize_t k;
    int     num = 0;
    struct disk_wwn_node_list  *cur_list;
    struct disk_wwn_node_list  *prev_list;
    struct disk_wwn_node_entry *cur_ent;
    DIR           *dirp;
    struct dirent *dep;
    char           device_path[PATH_MAX + 1];
    char           symlink_path[PATH_MAX + 1];
    struct stat    stats;

    if (*disk_wwn_node_listhead != NULL)
    {
        return;                            /* already collected nodes */
    }

    *disk_wwn_node_listhead = (struct disk_wwn_node_list *)malloc(sizeof(struct disk_wwn_node_list));
    if (*disk_wwn_node_listhead == NULL)
    {
        exit(EXIT_FAILURE);
    }
    memset(*disk_wwn_node_listhead, 0, sizeof(struct disk_wwn_node_list));

    cur_list = *disk_wwn_node_listhead;
    dirp = opendir("/dev/disk/by-id");
    if (dirp == NULL)
    {
        return;
    }

    while (true)
    {
        dep = readdir(dirp);
        if (dep == NULL)
        {
            break;
        }
        if (memcmp("scsi-", dep->d_name, 5) != 0)
        {
            continue;                           /* needs to start with "scsi-" */
        }
        if (strstr(dep->d_name, "part") != NULL)
        {
            continue;                           /* skip if contains "part" */
        }
        if (dep->d_name[5] != '3' && dep->d_name[5] != '2' && dep->d_name[5] != '8')
        {
            continue;                           /* skip for invalid identifier */
        }

        snprintf(device_path, PATH_MAX, "/dev/disk/by-id/%s", dep->d_name);
        device_path[PATH_MAX] = '\0';
        if (lstat(device_path, &stats) < 0)
        {
            continue;
        }
        if (!S_ISLNK((int)stats.st_mode))
        {
            continue;                           /* Skip non-symlinks */
        }
        if ((k = readlink(device_path, symlink_path, PATH_MAX)) < 1)
        {
            continue;
        }
        symlink_path[k] = '\0';

        /* Add to the list. */
        if (cur_list->count >= DISK_WWN_NODE_LIST_ENTRIES)
        {
            prev_list = cur_list;
            cur_list = (struct disk_wwn_node_list *)malloc(sizeof(struct disk_wwn_node_list));
            if (cur_list == NULL)
            {
                exit(EXIT_FAILURE);
            }
            memset(cur_list, 0, sizeof(struct disk_wwn_node_list));
            prev_list->next = cur_list;
        }
        cur_ent = &cur_list->nodes[cur_list->count];
        my_strcopy(cur_ent->wwn, "0x", 2);
        my_strcopy(cur_ent->wwn + 2, dep->d_name + 5, sizeof(cur_ent->wwn) - 2);
        my_strcopy(cur_ent->disk_bname, basename(symlink_path), sizeof(cur_ent->disk_bname));
        cur_list->count++;
        ++num;
    }
    closedir(dirp);
    return;
}   /* End of collect_disk_wwn_nodes */

/* ------------------------------------------------------------------------ */
/* Free disk_wwn_node_list. */
static void free_disk_wwn_node_list(struct disk_wwn_node_list **disk_wwn_node_listhead)
{
    if (*disk_wwn_node_listhead != NULL)
    {
        struct disk_wwn_node_list *cur_list;
        struct disk_wwn_node_list *next_list;

        cur_list = *disk_wwn_node_listhead;
        while (cur_list != NULL)
        {
            next_list = cur_list->next;
            free(cur_list);
            cur_list = next_list;
        }
        *disk_wwn_node_listhead = NULL;
    }
}   /* End of free_disk_wwn_node_list */

/* ------------------------------------------------------------------------ */
/*
   Given a path to a class device, find the most recent device node with
   matching major/minor. Returns true if match found, false otherwise.
 */
static bool get_disk_wwn(const char *wd, /*@out@*/ char *wwn_str, size_t max_wwn_str_len,
                         struct disk_wwn_node_list **disk_wwn_node_listhead)
{
    unsigned int k = 0;
    char        *bn;
    struct disk_wwn_node_list  *cur_list;
    struct disk_wwn_node_entry *cur_ent;
    char name[LMAX_PATH];

    wwn_str[0] = '\0';
    my_strcopy(name, wd, sizeof(name));
    name[sizeof(name) - 1] = '\0';
    bn = basename(name);

    if (*disk_wwn_node_listhead == NULL)
    {
        collect_disk_wwn_nodes(disk_wwn_node_listhead);
        if (*disk_wwn_node_listhead == NULL)
        {
            return(false);
        }
    }

    cur_list = *disk_wwn_node_listhead;
    while (true)
    {
        if (k >= cur_list->count)
        {
            cur_list = cur_list->next;
            if (cur_list == NULL)
            {
                break;
            }
            k = 0;
        }
        cur_ent = &cur_list->nodes[k];
        k++;

        if (strcmp(cur_ent->disk_bname, bn) == 0)
        {
            my_strcopy(wwn_str, cur_ent->wwn, max_wwn_str_len);
            wwn_str[max_wwn_str_len - 1] = '\0';
            return(true);
        }
    }
    return(false);
}   /* End of get_disk_wwn */

/* ------------------------------------------------------------------------ */
/*
   Obtain the SCSI ID of a disk.
   @dev_node: Device node of the disk, e.g. "/dev/sda".
   Return value: pointer to the SCSI ID if lookup succeeded or NULL if lookup
   failed.
   Note: The caller must free the returned buffer with free().
 */
static /*@null@*/ char *get_disk_scsi_id(const char *dev_node)
{
    char          *scsi_id;
    DIR           *dir;
    struct dirent *entry;
    char           holder[LMAX_PATH + 6];
    char           sys_block[LMAX_PATH];
    struct stat    stats;
    dev_t          st_rdev;

    if (stat(dev_node, &stats) < 0)
    {
        goto next;
    }
    st_rdev = (dev_t)stats.st_rdev;
    if (chdir("/dev/disk/by-id") < 0)
    {
        goto next;
    }
    dir = opendir("/dev/disk/by-id");
    if (dir == NULL)
    {
        goto next;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (stat(entry->d_name, &stats) >= 0 &&
            (dev_t)stats.st_rdev == st_rdev &&
            strncmp(entry->d_name, "scsi-", strlen("scsi-")) == 0)
        {
            scsi_id = strdup(entry->d_name + strlen("scsi-"));
            return(scsi_id);
        }
    }
    closedir(dir);
  next:

    snprintf(sys_block, sizeof(sys_block), "/sys/class/block/%s/holders", dev_node + 5);
    dir = opendir(sys_block);
    if (dir == NULL)
    {
        return NULL;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        snprintf(holder, sizeof(holder), "/dev/%s", entry->d_name);
        scsi_id = get_disk_scsi_id(holder);
        if (scsi_id != NULL)
        {
            break;
        }
    }
    closedir(dir);
    return(scsi_id);
}   /* End of get_disk_scsi_id */

/* ------------------------------------------------------------------------ */
#define VPD_DEVICE_ID   0x83
#define VPD_ASSOC_LU    0
#define VPD_ASSOC_TPORT 1
#define TPROTO_ISCSI    5

/* ------------------------------------------------------------------------ */
/*
   Iterates to next designation descriptor in the device identification
   VPD page. The 'initial_desig_desc' should point to start of first
   descriptor with 'page_len' being the number of valid bytes in that
   and following descriptors. To start, 'off' should point to a negative
   value, thereafter it should point to the value yielded by the previous
   call. If 0 returned then 'initial_desig_desc + *off' should be a valid
   descriptor; returns -1 if normal end condition and -2 for an abnormal
   termination. Matches association, designator_type and/or code_set when
   any of those values are greater than or equal to zero.
 */
static int sg_vpd_dev_id_iter(const uint8_t *initial_desig_desc, int page_len, int *off,
                       int v_assoc, int v_desig_type, int v_code_set)
{
    const uint8_t *bp;
    int            k;
    uint8_t        c_set;
    uint8_t        assoc;
    uint8_t        desig_type;

    for (k = *off, bp = initial_desig_desc; (k + 3) < page_len;)
    {
        k = k < 0 ? 0 : k + bp[k + 3] + 4;
        if ((k + 4) > page_len)
        {
            break;
        }
        c_set = (bp[k] & 0xf);
        if (v_code_set >= 0 && v_code_set != c_set)
        {
            continue;
        }
        assoc = ((bp[k + 1] >> 4) & 0x3);
        if (v_assoc >= 0 && v_assoc != assoc)
        {
            continue;
        }
        desig_type = (bp[k + 1] & 0xf);
        if (v_desig_type >= 0 && v_desig_type != desig_type)
        {
            continue;
        }
        *off = k;
        return(0);
    }
    return((k == page_len) ? -1 : -2);
}   /* End of sg_vpd_dev_id_iter */

/* ------------------------------------------------------------------------ */
/*
   Fetch logical unit (LU) name given the device name in the form:
   h:c:t:l tuple string (e.g. "2:0:1:0"). This is fetched via sysfs (lk 3.15
   and later) in vpd_pg83. For later ATA and SATA devices this may be its
   WWN. Normally take the first found in this order: NAA, EUI-64 then SCSI
   name string. However if a SCSI name string is present and the protocol is
   iSCSI (target port checked) then the SCSI name string is preferred. If
   none of the above are present then check for T10 Vendor ID
   (designator_type=1) and use if available.
 */
static void get_lu_name(const char *devname, char *b, ssize_t b_len)
{
    int         fd;
    ssize_t     res;
    uint16_t    len;
    uint8_t     dlen;
    uint8_t     sns_dlen;
    int         off;
    uint8_t     k;
    int         n;
    uint8_t    *bp;
    char       *cp;
    char        buff[LMAX_DEVPATH];
    uint8_t     u[512];
    char        u_sns[512];
    struct stat a_stat;

    b[0] = '\0';
    snprintf(buff, sizeof(buff), "/sys/class/scsi_device/%s/device/vpd_pg83", devname);
    if (!((stat(buff, &a_stat) >= 0) && S_ISREG((mode_t)a_stat.st_mode)))
    {
        return;
    }
    if ((fd = open(buff, O_RDONLY)) < 0)
    {
        return;
    }
    res = read(fd, u, sizeof(u));
    if (res <= 8)
    {
        close(fd);
        return;
    }
    close(fd);
    if (VPD_DEVICE_ID != u[1])
    {
        return;
    }

    /* Need to get a Big Endian 16 bit value from vpd_pg83. */
    len = (u[2] << 8) | u[3];

    if ((len + 4) != res)
    {
        return;
    }
    bp = u + 4;
    cp = b;
    off = -1;

    /* SCSI name string (sns), UTF-8 */
    if (0 == sg_vpd_dev_id_iter(bp, len, &off, VPD_ASSOC_LU, 8, 3))
    {
        sns_dlen = bp[off + 3];
        memcpy(u_sns, bp + off + 4, sns_dlen);
        /* now want to check if this is iSCSI */
        off = -1;
        /* SCSI name string (sns), UTF-8 */
        if (0 == sg_vpd_dev_id_iter(bp, len, &off, VPD_ASSOC_TPORT, 8, 3))
        {
            if ((0x80 & bp[1]) != 0 && TPROTO_ISCSI == (bp[0] >> 4))
            {
// printf("NOT-VERIFIED in vpd page 83 as iSCSI\n");
                snprintf(b, b_len, "%.*s", sns_dlen, u_sns);
                return;
            }
        }
    }
    else
    {
        memset(u_sns, 0, sizeof(u_sns));
        sns_dlen = 0;
    }

    /* NAA, binary */
    if (0 == sg_vpd_dev_id_iter(bp, len, &off, VPD_ASSOC_LU, 3, 1))
    {
        dlen = bp[off + 3];
        if (!(dlen == 8 || dlen == 16))
        {
            return;
        }
        if ((n = snprintf(cp, b_len, "naa.")) >= b_len)
        {
            n = b_len - 1;
        }
        cp += n;
        b_len -= n;
        for (k = 0; k < dlen && b_len > 1; ++k)
        {
            snprintf(cp, b_len, "%02x", bp[off + 4 + k]);
            cp += 2;
            b_len -= 2;
        }
    }
    /* EUI, binary */
    else if (0 == sg_vpd_dev_id_iter(bp, len, &off, VPD_ASSOC_LU, 2, 1))
    {
        dlen = bp[off + 3];
        if (!(dlen == 8 || dlen == 12 || dlen == 16))
        {
            return;
        }
        if ((n = snprintf(cp, b_len, "eui.")) >= b_len)
        {
            n = b_len - 1;
        }
        cp += n;
        b_len -= n;
        for (k = 0; ((k < dlen) && (b_len > 1)); ++k)
        {
            snprintf(cp, b_len, "%02x", bp[off + 4 + k]);
            cp += 2;
            b_len -= 2;
        }
    }
    /* UUID, binary */
    else if (0 == sg_vpd_dev_id_iter(bp, len, &off, VPD_ASSOC_LU, 0xa, 1))
    {
        dlen = bp[off + 3];
        if (((bp[off + 4] >> 4) & 0xf) != 1 || dlen != 18)
        {
            snprintf(cp, b_len, "??");
        }
        else
        {
            if ((n = snprintf(cp, b_len, "uuid.")) >= b_len)
            {
                n = b_len - 1;
            }
            cp += n;
            b_len -= n;
// printf("NOT-VERIFIED in vpd page 83 as UUID\n");
            for (k = 0; (k < 16) && (b_len > 1); ++k)
            {
                if (k == 4 || k == 6 || k == 8 || k == 10)
                {
                    snprintf(cp, b_len, "-");
                    ++cp;
                    --b_len;
                }
                snprintf(cp, b_len, "%02x", bp[off + 6 + k]);
                cp += 2;
                b_len -= 2;
            }
        }
    }
    else if (sns_dlen > 0)
    {
        snprintf(b, b_len, "%.*s", sns_dlen, u_sns);
    }
    /* T10 vendor ID, ASCII or UTF */
    else if ((0 == sg_vpd_dev_id_iter(bp, len, &off, VPD_ASSOC_LU, 0x1, -1)) &&
             ((bp[off] & 0xf) > 1 /* ASCII or UTF */))
    {
        dlen = bp[off + 3];
        if (dlen < 8)
        {
            return;                          /* must have 8 byte T10 vendor id */
        }
        if ((n = snprintf(cp, b_len, "t10.")) >= b_len)
        {
            n = b_len - 1;
        }
        cp += n;
        b_len -= n;
// printf("NOT-VERIFIED in vpd page 83 as T10 vendor ID\n");
        snprintf(cp, b_len, "%.*s", dlen, (char *)(bp + off + 4));
    }
    return;
}   /* End of get_lu_name */

/* ------------------------------------------------------------------------ */
/*
   Parse colon_list into host/channel/target/lun ("hctl") array, return true
   if successful, else false. colon_list should point at first character of
   hctl (i.e. a digit) and yields a new value in *outp when true returned.
 */
static bool parse_colon_list(const char *colon_list, struct addr_hctl *outp)
{
    int         k;
    uint64_t    z;
    const char *elem_end;
    uint8_t    *p;

    if (colon_list == NULL || outp == NULL)
    {
        return(false);
    }
    if ((char)toupper((uint8_t)*colon_list) == 'N')
    {
        outp->h = NVME_HOST_NUM;
    }
    else if (sscanf(colon_list, "%d", &outp->h) != 1)
    {
        return(false);
    }
    if ((elem_end = strchr(colon_list, ':')) == NULL)
    {
        return(false);
    }
    colon_list = elem_end + 1;
    if (sscanf(colon_list, "%d", &outp->c) != 1)
    {
        return(false);
    }
    if ((elem_end = strchr(colon_list, ':')) == NULL)
    {
        return(false);
    }
    colon_list = elem_end + 1;
    if (sscanf(colon_list, "%d", &outp->t) != 1)
    {
        return(false);
    }
    if ((elem_end = strchr(colon_list, ':')) == NULL)
    {
        return(false);
    }
    colon_list = elem_end + 1;
    if (sscanf(colon_list, "%lu", &outp->l) != 1)
    {
        return(false);
    }
    z = outp->l;
    for (k = 0; k < 8; k += 2, z >>= 16)
    {
        /* Need to big endian 16 bit values in lun array. */
        p = outp->lun_arr + k;
        p[0] = (z >> 8) & 0xff;
        p[1] = z & 0xff;
    }
    return(true);
}   /* End of parse_colon_list */

/* ------------------------------------------------------------------------ */
/*
   Given the transport_id of the SCSI device (LU) associated with 'devname'
   output additional information.
 */
static void transport_tport_longer(const char *devname, int transport_id)
{
    char *cp;
    char  path_name[LMAX_DEVPATH];
    char  buff[LMAX_DEVPATH];
    char  b2[LMAX_DEVPATH];
    char  wd[LMAX_PATH];

    snprintf(path_name, sizeof(path_name), "/sys/class/scsi_device/%s", devname);
    my_strcopy(buff, path_name, sizeof(buff));

    switch (transport_id)
    {
      case TRANSPORT_FC:
          printf("transport=FC\n");

          if (!if_directory_chdir(path_name, "device"))
          {
              return;
          }
          if (getcwd(wd, sizeof(wd)) == NULL)
          {
              return;
          }
          cp = strrchr(wd, '/');
          if (cp == NULL)
          {
              return;
          }
          *cp = '\0';
          cp = strrchr(wd, '/');
          if (cp == NULL)
          {
              return;
          }
          *cp = '\0';
          cp = basename(wd);
          snprintf(buff, sizeof(buff), "%s%s", "fc_remote_ports/", cp);
          if (if_directory_chdir(wd, buff))
          {
              if (getcwd(buff, sizeof(buff)) == NULL)
              {
                  return;
              }
          }
          else
          {                                     /* newer transport */
              /* /sys  /class/fc_remote_ports/  rport-x:y-z  / */
              snprintf(buff, sizeof(buff), "/sys/class/fc_remote_ports/%s/", cp);
          }
          snprintf(b2, sizeof(b2), "%s%s", path_name, "/device/");
          print_value_exists(b2, "vendor");

          printf("%s\n", cp);                 /* rport */

          print_value_exists(buff, "node_name");
          print_value_exists(buff, "port_name");            /* */
          print_value_exists(buff, "port_id");              /* */
          print_value_exists(buff, "port_state");
          print_value_exists(buff, "roles");
          print_value_exists(buff, "scsi_target_id");
          print_value_exists(buff, "supported_classes");
          print_value_exists(buff, "fast_io_fail_tmo");
          print_value_exists(buff, "dev_loss_tmo");
          break;

      case TRANSPORT_ISCSI:
          printf("transport=iSCSI\n");

          snprintf(buff, sizeof(buff), "/sys/class/iscsi_session/session%d", iscsi_tsession_num);
          print_value_exists(buff, "targetname");
          print_value_exists(buff, "tpgt");
          print_value_exists(buff, "initiatorname");
          print_value_exists(buff, "state");
          print_value_exists(buff, "data_pdu_in_order");
          print_value_exists(buff, "data_seq_in_order");
          print_value_exists(buff, "erl");
          print_value_exists(buff, "first_burst_len");
          print_value_exists(buff, "initial_r2t");
          print_value_exists(buff, "max_burst_len");
          print_value_exists(buff, "max_outstanding_r2t");
          print_value_exists(buff, "recovery_tmo");
          break;

      default:
//          printf("No transport information\n");
          break;
    }
}   /* End of transport_tport_longer */

/* ------------------------------------------------------------------------ */
/* List one SCSI device (LU) on a line. */
static void one_sdev_entry(const char *dir_name, const char *devname,
                           struct dev_node_list **dev_node_listhead,
                           struct disk_wwn_node_list **disk_wwn_node_listhead)
{
    int              type;
    char             buff[LMAX_DEVPATH];
    char             extra[LMAX_DEVPATH];
    char             value[LMAX_NAME];
    char             wd[LMAX_PATH];
    uint64_t         blk512s;
    char             blkdir[LMAX_DEVPATH];
    int              lbs = 0;
    char             bb[32];
    char            *scsi_id;
    int              transport_id;
    struct addr_hctl hctl;
    struct stat      a_stat;


    /* Determine type of transport protocol. */
    if (parse_colon_list(devname, &hctl))
    {
        /* Check for FC host. */
        snprintf(buff, sizeof(buff), "/sys/class/fc_host/host%d", hctl.h);
        if (stat(buff, &a_stat) >= 0 && S_ISDIR((mode_t)a_stat.st_mode))
        {
            transport_id = TRANSPORT_FC;
        }
        else
        {
            /* Check for iSCSI device. */
            snprintf(buff, sizeof(buff), "/sys/class/iscsi_host/host%d/device", hctl.h);
            if (stat(buff, &a_stat) >= 0 && S_ISDIR((mode_t)a_stat.st_mode))
            {
                if (iscsi_target_scan(buff, &hctl) == 1)
                {
                    transport_id = TRANSPORT_ISCSI;
                }
            }
            else
            {
                /* transport_id = TRANSPORT_UNKNOWN; */
                return;
            }
        }
    }
    else
    {
        /* transport_id = TRANSPORT_UNKNOWN; */
        return;
    }

    printf("\n");
    printf("SCSI_DEVICE=%s\n", devname);

    snprintf(buff, sizeof(buff), "%s/%s", dir_name, devname);
    if (get_value(buff, "type", value, sizeof(value)))
    {
        if (sscanf(value, "%d", &type) == 1)
        {
            if (type < 0 || type > 31)
            {
                printf("type=%d\n", type);
            }
            else
            {
                printf("type=%s\n", scsi_short_device_types[type]);
            }
        }
    }

    get_lu_name(devname, value, sizeof(value));
    if (strlen(value) > 0)
    {
        printf("lu_name=%s\n", value);
    }

    if (non_sg_scan(buff) == 1)
    {
        if (non_sg.d_type == (unsigned char)DT_DIR)
        {
            snprintf(wd, sizeof(wd), "%s/%s", buff, non_sg.name);
            if (scan_for_first(wd) == 1)
            {
                my_strcopy(extra, aa_first.name, sizeof(extra));
            }
            else
            {
                memset(extra, 0, sizeof(extra));
                printf("unexpected scan_for_first error");
                wd[0] = '\0';
            }
        }
        else
        {
            my_strcopy(wd, buff, sizeof(wd));
            my_strcopy(extra, non_sg.name, sizeof(extra));
        }

        if (wd[0] != '\0' && (if_directory_chdir(wd, extra)))
        {
            if (getcwd(wd, sizeof(wd)) == NULL)
            {
                printf("getcwd error");
                wd[0] = '\0';
            }
        }

        if (wd[0] != '\0')
        {
            char          dev_node[LMAX_NAME];          /* Initialized in get_dev_node. */
            char          wwn_str[DISK_WWN_MAX_LEN];
            enum dev_type typ;

            typ = (FT_BLOCK == non_sg.ft) ? BLK_DEV : CHR_DEV;
            if (BLK_DEV == typ && 
                get_disk_wwn(wd, wwn_str, sizeof(wwn_str), disk_wwn_node_listhead))
            {
// printf("%s NOT-VERIFIED providing wwn\n", __func__);
                printf("wwn=%s\n", wwn_str);
            }

            (void)get_dev_node(wd, dev_node, typ, dev_node_listhead);
            printf("dev_node=%s\n", dev_node);

            if (get_value(wd, "dev", value, sizeof(value)))
            {
                printf("device_number=%s\n", value);
            }

            scsi_id = get_disk_scsi_id(dev_node);
            if (scsi_id != NULL)
            {
                printf("disk_uuid=%s\n", scsi_id);
            }
        }
    }

    if (if_directory_ch2generic(buff))
    {
        if (getcwd(wd, sizeof(wd)) == NULL)
        {
            printf("generic_dev error");
        }
        else
        {
            char dev_node[LMAX_NAME];           /* Initialized in get_dev_node. */

            if (get_dev_node(wd, dev_node, CHR_DEV, dev_node_listhead))
            {
                printf("sg_device=%s\n", dev_node);
            }
            if (get_value(wd, "dev", value, sizeof(value)))
            {
                printf("sg_device_number=%s\n", value);
            }
        }
    }

    my_strcopy(blkdir, buff, sizeof(blkdir));
    value[0] = '\0';
    if ((type == 0) && block_scan(blkdir) &&
          if_directory_chdir(blkdir, ".") &&
          get_value(".", "size", value, sizeof(value)))
    {
        blk512s = atoll(value);
        if (get_value(".", "queue/logical_block_size", bb, sizeof(bb)))
        {
            lbs = atoi(bb);
            if (lbs < 1)
            {
                printf("size=%s,[lbs<1 ?]\n", value);
            }
            else if (lbs == 512)
            {
                printf("size=%s,512\n", value);
            }
            else
            {
                int64_t byts = 512 * blk512s;

                printf("size=%ld,%d\n", (long)(byts / lbs), lbs);
            }
        }
        else
        {
            printf("size=%s,512\n", value);
        }
    }

    transport_tport_longer(devname, transport_id);

    print_value_exists(buff, "device_blocked");
    print_value_exists(buff, "device_busy");
    print_value_exists(buff, "dh_state");
    print_value_exists(buff, "model");
    print_value_exists(buff, "queue_depth");
    print_value_exists(buff, "queue_type");
    print_value_exists(buff, "rev");
    print_value_exists(buff, "scsi_level");
    print_value_exists(buff, "state");
    print_value_exists(buff, "timeout");
/*    print_value_exists(buff, "type");     -- Done above, with character name if in range. */
}   /* End of one_sdev_entry */

/* ------------------------------------------------------------------------ */
#ifdef NVME_PARTITIONS
static int ndev_dir_scan_select3(const struct dirent *s)
{
    int      cdev_minor;
    char    *cn;
    uint32_t nsid;
    char    *cp;
    uint32_t npart;

    /* What to do about NVMe controller CNTLID field? */
    if (strncmp(s->d_name, "nvme", 4) != 0)
    {
        return(0);
    }

    /* nvme#n# */
    cn = strchr(s->d_name + 4, 'n');
    if (cn == NULL)
    {
        return(0);
    }

    /* nvme#n#p# */
    cp = strchr(s->d_name + 4, 'p');
    if (cp == NULL)
    {
        return(0);
    }

    /* There are three numbers total, before and after the n and p. */
    if (sscanf(s->d_name + 4, "%d", &cdev_minor) == 1 &&
        sscanf(cn + 1, "%u", &nsid) == 1 &&
        sscanf(cp + 1, "%u", &npart) == 1)
    {
        return(1);
    }
    return(0);
}   /* End of ndev_dir_scan_select3 */
#endif /* NVME_PARTITIONS */

/* ------------------------------------------------------------------------ */
/*
   This is a compare function for numeric sort based on hctl tuple.
   Returns -1 if (a->d_name < b->d_name) ; 0 if they are equal
   and 1 otherwise.
 */
#ifdef NVME_PARTITIONS
static int ndev_scandir_sort(const struct dirent **a, const struct dirent **b)
{
    const char      *lnam = (*a)->d_name;
    const char      *rnam = (*b)->d_name;

    return(strcmp(lnam, rnam));
}   /* End of ndev_scandir_sort */
#endif /* NVME_PARTITIONS */

/* ------------------------------------------------------------------------ */
#ifdef NVME_PARTITIONS
static void one_nnp_entry(const char *nvme_ctl_nnp, const char *nvme_ns_rel,
                           struct dev_node_list **dev_node_listhead)
{
    char             buff[LMAX_DEVPATH];
    char             value[LMAX_NAME];
    char             dev_node[LMAX_NAME + 16];          /* Initialized in get_dev_node. */

    printf("\n");

    snprintf(buff, sizeof(buff), "%s/%s", nvme_ctl_nnp, nvme_ns_rel);
    printf("NVME_PARTITION=%s\n", buff);

    print_value_exists(buff, "alignment_offset");
    print_value_exists(buff, "discard_alignment");
    print_value_exists(buff, "partition");
    print_value_exists(buff, "ro");
    print_value_exists(buff, "start");
    print_value_exists(buff, "size");

    if (get_dev_node(buff, dev_node, BLK_DEV, dev_node_listhead))
    {
        printf("dev_node=%s\n", dev_node);
    }
    if (get_value(buff, "dev", value, sizeof(value)))
    {
        printf("device_number=%s\n", value);
    }
}   /* End of one_ndev_entry */
#endif /* NVME_PARTITIONS */

/* ------------------------------------------------------------------------ */
/*
   This is a compare function for numeric sort based on hctl tuple.
   Returns -1 if (a->d_name < b->d_name) ; 0 if they are equal
   and 1 otherwise.
 */
static int sdev_scandir_sort(const struct dirent **a, const struct dirent **b)
{
    const char      *lnam = (*a)->d_name;
    const char      *rnam = (*b)->d_name;
    struct addr_hctl left_hctl;
    struct addr_hctl right_hctl;

    if (!parse_colon_list(lnam, &left_hctl))
    {
        fprintf(stderr, "%s: left parse failed: %.20s\n", __func__, lnam != NULL ? lnam : "<null>");
        return(-1);
    }

    if (!parse_colon_list(rnam, &right_hctl))
    {
        fprintf(stderr, "%s: right parse failed: %.20s\n", __func__, rnam != NULL ? rnam : "<null>");
        return(1);
    }

    return(cmp_hctl(&left_hctl, &right_hctl));
}   /* End of sdev_scandir_sort */

/* ------------------------------------------------------------------------ */
#ifdef INCLUDE_NVME
/*
   List one NVMe namespace (NS) on a line.
   Directory: /sys/class/nvme/nvme[0-9]+/nvme#n#/  -- which points to something like:
   /sys/devices/pci0000:80/0000:80:02.2/0000:82:00.0/0000:83:01.0/0000:84:00.0/nvme/nvme#
 */

static void one_ndev_entry(const char *nvme_ctl_abs, const char *nvme_ns_rel,
                           struct dev_node_list **dev_node_listhead)
{
    int              cdev_minor = 0;
    int              cntlid = 0;
    int              sel_mask = 0xf;
    uint32_t         nsid = 0;
    char            *cp;
    char             buff[LMAX_DEVPATH];
    char             buff1[LMAX_DEVPATH];
    char             value[LMAX_NAME];
    char             dev_node[LMAX_NAME + 16];          /* Initialized in get_dev_node. */
    char             devname[64];
    struct addr_hctl hctl;
    uint64_t         blk512s;
    int              lbs = 0;
    char             bb[80];
#ifdef NVME_PARTITIONS
    int              num3;
    int              k;
    struct dirent  **name_list3;
#endif /* NVME_PARTITIONS */

    printf("\n");

    if (strncmp(nvme_ns_rel, "nvme", 4) != 0 ||           /* If doesn't start with nvme */
        sscanf(nvme_ns_rel + 4, "%d", &cdev_minor) != 1)  /* If no number */
    {
        fprintf(stderr, "%s: unable to find %s in %s\n", __func__, "cdev_minor", nvme_ns_rel);
    }

    if (get_value(nvme_ctl_abs, "cntlid", value, sizeof(value)))
    {
        if (sscanf(value, "%d", &cntlid) != 1)
        {
            fprintf(stderr, "%s: trying to decode: %s as " "cntlid\n", __func__, value);
        }
    }
    else
    {
        fprintf(stderr, "%s: unable to find %s under %s\n", __func__, "cntlid", nvme_ctl_abs);
    }

    cp = strrchr(nvme_ns_rel, 'n');
    if (cp == NULL || ('v' == *(cp + 1)) || sscanf(cp + 1, "%u", &nsid) != 1)
    {
        fprintf(stderr, "%s: unable to find nsid in %s\n", __func__, nvme_ns_rel);
    }

    /* Make a tuple for nvme. */
    hctl.h = NVME_HOST_NUM;
    hctl.c = cdev_minor;
    hctl.t = cntlid;

    /* Need to change byte order if on little endian machines -- else does nothing. */
    hctl.lun_arr[0] = nsid & 0xff;
    hctl.lun_arr[1] = (nsid >> 8) & 0xff;
    hctl.lun_arr[2] = (nsid >> 16) & 0xff;
    hctl.lun_arr[3] = (nsid >> 24) & 0xff;
    memset(hctl.lun_arr + 4, 0, 4);          /* Only 4 bytes long, zero other 4 bytes */
    hctl.l = nsid;

    memset(devname, 0, sizeof(devname));
    tuple2string(&hctl, sel_mask, sizeof(devname), devname);

    printf("NVME_DEVICE=%s\n", devname);

    snprintf(buff, sizeof(buff), "%s/%s", nvme_ctl_abs, nvme_ns_rel);
    print_value_exists(buff, "wwid");

    snprintf(buff1, sizeof(buff1), "%s/device", buff);
    print_value_exists(buff1, "transport");

    snprintf(buff1, sizeof(buff1), "%s/device/device", buff);
    print_value_exists(buff1, "subsystem_vendor");
    print_value_exists(buff1, "subsystem_device");

    printf("nsid=%u\n", nsid);

    if (get_dev_node(buff, dev_node, BLK_DEV, dev_node_listhead))
    {
        printf("dev_node=%s\n", dev_node);
    }
    if (get_value(buff, "dev", value, sizeof(value)))
    {
        printf("device_number=%s\n", value);
    }
    if (get_value(buff, "size", value, sizeof(value)))
    {
        blk512s = atoll(value);

        if (get_value(buff, "queue/logical_block_size", bb, sizeof(bb)))
        {
            lbs = atoi(bb);
            if (lbs < 1)
            {
                printf("size=%s,[lbs<1 ?]\n", value);
            }
            else if (lbs == 512)
            {
                printf("size=%s,512\n", value);
            }
            else
            {
                int64_t byts = 512 * blk512s;

                printf("size=%ld,%d\n", (long)(byts / lbs), lbs);
            }
        }
        else
        {
            printf("size=%s,512\n", value);
        }
    }

    print_value_exists(buff, "capability");
    print_value_exists(buff, "ext_range");
    print_value_exists(buff, "hidden");
    print_value_exists(buff, "range");
    print_value_exists(buff, "removable");
    print_value_exists(buff, "ro");
    print_value_exists(buff, "uuid");

    snprintf(buff1, sizeof(buff1), "%s/queue", buff);
    print_value_exists(buff1, "nr_requests");
    print_value_exists(buff1, "read_ahead_kb");
    print_value_exists(buff1, "write_cache");
    print_value_exists(buff1, "logical_block_size");
    print_value_exists(buff1, "physical_block_size");

    /* buff1 no longer needed. */

#ifdef NVME_PARTITIONS
    /* find all /sys/class/nvme/nvme#/nvme#n#/nvme#n#p# */
    num3 = scandir(buff, &name_list3, ndev_dir_scan_select3, ndev_scandir_sort);
    if (num3 < 0)
    {
        return;
    }
    for (k = 0; k < num3; ++k)
    {
        one_nnp_entry(buff, name_list3[k]->d_name, dev_node_listhead);
        free(name_list3[k]);
    }
    free(name_list3);
#endif /* NVME_PARTITIONS */

}   /* End of one_ndev_entry */

/* ------------------------------------------------------------------------ */
static void one_nhost_entry(const char *dir_name, const char *nvme_ctl_rel)
{
    char        buff[LMAX_DEVPATH];

    printf("\n");
    printf("DEVICE=%s\n", nvme_ctl_rel);

    snprintf(buff, sizeof(buff), "%s%s", dir_name, nvme_ctl_rel);
    print_value_as_is(buff, "uevent");
    print_value_exists(buff, "transport");
    print_value_exists(buff, "address");
    print_value_exists(buff, "subsysnqn");
    print_value_exists(buff, "cntlid");
    print_value_exists(buff, "state");
    print_value_exists(buff, "firmware_rev");
    print_value_exists(buff, "model");
    print_value_exists(buff, "serial");

    snprintf(buff, sizeof(buff), "%s%s/device", dir_name, nvme_ctl_rel);
    print_value_exists(buff, "current_link_speed");
    print_value_exists(buff, "current_link_width");
    print_value_exists(buff, "local_cpulist");
    print_value_exists(buff, "numa_node");
    print_value_exists(buff, "subsystem_vendor");
    print_value_exists(buff, "subsystem_device");
}   /* End of one_nhost_entry */
#endif  // INCLUDE_NVME

/* ------------------------------------------------------------------------ */
static void one_host_entry(const char *devname)
{
    char         buff[LMAX_DEVPATH];
    char         value[LMAX_NAME];
    char         wd[LMAX_PATH];
    char        *bn;
    char         buff_transport[LMAX_DEVPATH];
    struct stat  a_stat;
    char        *cp;
    char         tmp[LMAX_PATH];
    char         bname[LMAX_NAME];

    snprintf(buff, sizeof(buff), "/sys/class/scsi_host/%s", devname);
    if (get_value(buff, "proc_name", value, sizeof(value)) &&
        strncmp(value, "<NULL>", 6) != 0 &&
        strncmp(value, "(null)", 6) != 0)
    {
        (void)trim_lead_trail(value);
        /* Ignore ahci devices. */
        if (strcmp(value, "ahci") == 0)
        {
            return;
        }
        printf("\n");
        printf("HOST=%s\n", devname);
        printf("proc_name=%s\n", value);
    }
    else
    {
        printf("\n");
        printf("HOST=%s\n", devname);
        if (if_directory_chdir(buff, "device/../driver"))
        {
            if (getcwd(wd, sizeof(wd)) == NULL)
            {
                printf("driver=(null)\n");
            }
            else
            {
                bn = basename(wd);
                printf("driver=%s\n", bn);
            }
        }
        else
        {
            printf("proc_name=%s\n", value);    // Note: nothing, or <NULL> or (null).
        }
    }

    /* FC host */
    snprintf(buff_transport, sizeof(buff_transport), "/sys/class/fc_host/%s", devname);
    if (stat(buff_transport, &a_stat) >= 0 && S_ISDIR((mode_t)a_stat.st_mode))
    {
        my_strcopy(tmp, buff, sizeof(tmp));
        cp = basename(tmp);
        my_strcopy(bname, cp, sizeof(bname));
        bname[sizeof(bname) - 1] = '\0';

        printf("transport=FC\n");

        snprintf(tmp, sizeof(tmp), "%s/device/fc_host/%s", buff, bname);
        if (stat(tmp, &a_stat) < 0)
        {
            printf("no fc_host directory\n");
        }
        else
        {
            print_value_exists(tmp, "active_fc4s");
            print_value_exists(tmp, "supported_fc4s");
            print_value_exists(tmp, "dev_loss_tmo");
            print_value_exists(tmp, "fabric_name");
            print_value_exists(tmp, "maxframe_size");
            print_value_exists(tmp, "max_npiv_vports");
            print_value_exists(tmp, "npiv_vports_inuse");
            print_value_exists(tmp, "node_name");
            print_value_exists(tmp, "port_name");
            print_value_exists(tmp, "port_id");
            print_value_exists(tmp, "port_state");
            print_value_exists(tmp, "port_type");
            print_value_exists(tmp, "speed");
            print_value_exists(tmp, "supported_speeds");
            print_value_exists(tmp, "supported_classes");
            print_value_exists(tmp, "symbolic_name");
            print_value_exists(tmp, "tgtid_bind_type");
        }
    }
    else
    {
        /* iSCSI host */
        snprintf(buff_transport, sizeof(buff_transport), "/sys/class/iscsi_host/%s", devname);
        print_value_exists(buff_transport, "ipaddress");
    }

// snprintf(buff, sizeof(buff), "/sys/class/scsi_host/%s", devname); // Done above.
    print_value_exists(buff, "active_mode");
    print_value_exists(buff, "beacon");
    print_value_exists(buff, "can_queue");
    print_value_exists(buff, "cmd_per_lun");
    print_value_exists(buff, "driver_version");
    print_value_exists(buff, "fw_version");
    print_value_exists(buff, "host_busy");
    print_value_exists(buff, "isp_name");
    print_value_exists(buff, "link_power_management_policy");
    print_value_exists(buff, "link_state");
    print_value_exists(buff, "max_supported_speed");
    print_value_exists(buff, "model_desc");
    print_value_exists(buff, "model_name");
    print_value_exists(buff, "pci_info");
    print_value_exists(buff, "port_no");
    print_value_exists(buff, "port_speed");
    print_value_exists(buff, "serial_num");
    print_value_exists(buff, "sg_tablesize");
    print_value_exists(buff, "state");
    print_value_exists(buff, "supported_mode");
    print_value_exists(buff, "thermal_temp");
    print_value_exists(buff, "unchecked_isa_dma");
    print_value_exists(buff, "unique_id");
}   /* End of one_host_entry */

/* ------------------------------------------------------------------------ */
static int sdev_dir_scan_select(const struct dirent *s)
{
    if (strncmp(s->d_name, "host", 4) == 0)         /* SCSI host */
    {
        return(0);
    }
    if (strncmp(s->d_name, "target", 6) == 0)       /* SCSI target */
    {
        return(0);
    }
    if (strchr(s->d_name, ':') != NULL)
    {
        return(1);
    }
    return(0);
}   /* End of sdev_dir_scan_select */

/* ------------------------------------------------------------------------ */
#ifdef INCLUDE_NVME
/*
   This is a compare function for numeric sort based on hctl tuple. Similar
   to sdev_scandir_sort() but converts entries like "nvme2" into a hctl tuple.
   Returns -1 if (a->d_name < b->d_name) ; 0 if they are equal
   and 1 otherwise.
 */
static int nhost_scandir_sort(const struct dirent **a, const struct dirent **b)
{
    const char      *lnam = (*a)->d_name;
    const char      *rnam = (*b)->d_name;
    struct addr_hctl left_hctl;
    struct addr_hctl right_hctl;

    memset(&left_hctl, 0, sizeof(left_hctl));
    memset(&right_hctl, 0, sizeof(right_hctl));
    if (strchr(lnam, ':') != NULL)
    {
        if (!parse_colon_list(lnam, &left_hctl))
        {
            fprintf(stderr, "%s: left parse failed: %.20s\n", __func__, lnam != NULL ? lnam : "<null>");
            return(-1);
        }
    }
    else
    {
        if (sscanf(lnam, "nvme%d", &left_hctl.c) == 1)
        {
            left_hctl.h = NVME_HOST_NUM;
            left_hctl.t = 0;
            left_hctl.l = 0;
        }
        else
        {
            fprintf(stderr, "%s: left sscanf failed: %.20s\n", __func__, lnam != NULL ? lnam : "<null>");
            return(-1);
        }
    }

    if (strchr(rnam, ':') != NULL)
    {
        if (!parse_colon_list(rnam, &right_hctl))
        {
            fprintf(stderr, "%s: right parse failed: %.20s\n", __func__, rnam != NULL ? rnam : "<null>");
            return(1);
        }
    }
    else
    {
        if (sscanf(rnam, "nvme%d", &right_hctl.c) == 1)
        {
            right_hctl.h = NVME_HOST_NUM;
            right_hctl.t = 0;
            right_hctl.l = 0;
        }
        else
        {
            fprintf(stderr, "%s: right sscanf failed: %.20s\n", __func__, rnam != NULL ? rnam : "<null>");
            return(-1);
        }
    }
    return(cmp_hctl(&left_hctl, &right_hctl));
}   /* End of nhost_scandir_sort */
#endif  // INCLUDE_NVME

/* ------------------------------------------------------------------------ */
/* List SCSI devices (LUs). */
static void list_sdevices(struct dev_node_list **dev_node_listhead,
                          struct disk_wwn_node_list **disk_wwn_node_listhead)
{
    int             num;
    int             k;
    struct dirent **name_list;
    char            buff[LMAX_DEVPATH];
    char            name[LMAX_NAME];

    snprintf(buff, sizeof(buff), "/sys/bus/scsi/devices");
    num = scandir(buff, &name_list, sdev_dir_scan_select, sdev_scandir_sort);
    if (num < 0)
    {
        snprintf(name, sizeof(name), "%s: scandir: %s", __func__, buff);
        perror(name);
    }

    for (k = 0; k < num; ++k)
    {
        my_strcopy(name, name_list[k]->d_name, sizeof(name));
        one_sdev_entry(buff, name, dev_node_listhead, disk_wwn_node_listhead);
        free(name_list[k]);
    }
    free(name_list);
    free_disk_wwn_node_list(disk_wwn_node_listhead);
}   /* End of list_sdevices */

/* ------------------------------------------------------------------------ */
#ifdef INCLUDE_NVME
static int ndev_dir_scan_select(const struct dirent *s)
{
    int cdev_minor;                             /* /dev/nvme<n> char device minor */

    if (strncmp(s->d_name, "nvme", 4) == 0 &&
        sscanf(s->d_name + 4, "%d", &cdev_minor) == 1)
    {
        return(1);
    }
    return(0);
}   /* End of ndev_dir_scan_select */

/* ------------------------------------------------------------------------ */
static int ndev_dir_scan_select2(const struct dirent *s)
{
    int      cdev_minor;
    uint32_t nsid;
    char    *cp;

    /* What to do about NVMe controller CNTLID field? */
    if (strncmp(s->d_name, "nvme", 4) != 0)
    {
        return(0);
    }

    /* nvme#n# */
    cp = strchr(s->d_name + 4, 'n');
    if (cp == NULL)
    {
        return(0);
    }

    /* There are two numbers, before and after the n. */
    if (sscanf(s->d_name + 4, "%d", &cdev_minor) == 1 &&
        sscanf(cp + 1, "%u", &nsid) == 1)
    {
        return(1);
    }
    return(0);
}   /* End of ndev_dir_scan_select2 */
#endif  // INCLUDE_NVME

/* ------------------------------------------------------------------------ */
#ifdef INCLUDE_NVME
/* List NVME devices (namespaces). */
static void list_ndevices(struct dev_node_list **dev_node_listhead,
                          struct disk_wwn_node_list **disk_wwn_node_listhead)
{
    char            ebuf[120];

    char            buff[LMAX_DEVPATH];
    int             num;
    int             i;
    struct dirent **name_list;

    char            buff2[LMAX_DEVPATH];
    int             num2;
    int             j;
    struct dirent **name_list2;

    snprintf(buff, sizeof(buff), "/sys/class/nvme/");
printf("# buff=%s\n",buff);
    /* find all /sys/class/nvme/nvme# */
    num = scandir(buff, &name_list, ndev_dir_scan_select, nhost_scandir_sort);
    if (num < 0)
    {
        snprintf(ebuf, sizeof(ebuf), "%s: scandir: %s", __func__, buff);
        perror(ebuf);
        return;
    }

    for (i = 0; i < num; ++i)
    {
        snprintf(buff2, sizeof(buff2), "%s%s", buff, name_list[i]->d_name);
        /* find all /sys/class/nvme/nvme#/nvme#n# */
printf("# buff2=%s\n",buff2);
        num2 = scandir(buff2, &name_list2, ndev_dir_scan_select2, sdev_scandir_sort);
        if (num2 < 0)
        {
            snprintf(ebuf, sizeof(ebuf), "%s: scandir" "(2): %s", __func__, buff);
            perror(ebuf);
            /* already freed name_list[i] so move to next */
        }
        else
        {
            for (j = 0; j < num2; ++j)
            {
                one_ndev_entry(buff2, name_list2[j]->d_name, dev_node_listhead);
                free(name_list2[j]);
            }
            free(name_list2);
        }

        one_nhost_entry(buff, name_list[i]->d_name);
        free(name_list[i]);
    }
    free(name_list);
    free_disk_wwn_node_list(disk_wwn_node_listhead);
}   /* End of list_ndevices */
#endif  // INCLUDE_NVME

/* ------------------------------------------------------------------------ */
static int host_dir_scan_select(const struct dirent *s)
{
    if (strncmp("host", s->d_name, 4) == 0)
    {
        return 1;
    }
    return 0;
}   /* End of host_dir_scan_select */

/* ------------------------------------------------------------------------ */
/*
   Returns -1 if (a->d_name < b->d_name) ; 0 if they are equal
   and 1 otherwise.
 */
static int shost_scandir_sort(const struct dirent **a, const struct dirent **b)
{
    unsigned int l;
    unsigned int r;
    const char  *lnam = (*a)->d_name;
    const char  *rnam = (*b)->d_name;

    if (sscanf(lnam, "host%u", &l) != 1)
    {
        return(-1);
    }
    if (sscanf(rnam, "host%u", &r) != 1)
    {
        return(1);
    }
    if (l < r)
    {
        return(-1);
    }
    else if (r < l)
    {
        return(1);
    }
    return(0);
}   /* End of shost_scandir_sort */

/* ------------------------------------------------------------------------ */
static void list_shosts(void)
{
    int             num;
    int             k;
    struct dirent **name_list;
    char            name[LMAX_NAME];

    num = scandir("/sys/class/scsi_host/", &name_list, host_dir_scan_select, shost_scandir_sort);
    if (num < 0)
    {
        snprintf(name, sizeof(name), "%s: scandir: /sys/class/scsi_host/", __func__);
        perror(name);
        return;
    }

    for (k = 0; k < num; ++k)
    {
        my_strcopy(name, name_list[k]->d_name, sizeof(name));
        one_host_entry(name);
        free(name_list[k]);
    }
    free(name_list);
}   /* End of list_shosts */

/* ------------------------------------------------------------------------ */
int main(void)
{
    struct dev_node_list *dev_node_listhead = NULL;
    struct disk_wwn_node_list *disk_wwn_node_listhead = NULL;

    list_sdevices(&dev_node_listhead, &disk_wwn_node_listhead);
    list_shosts();
#ifdef INCLUDE_NVME
    list_ndevices(&dev_node_listhead, &disk_wwn_node_listhead);
#endif  // INCLUDE_NVME

    free_dev_node_list(&dev_node_listhead);

    exit(0);
}   /* End of main */

/* ------------------------------------------------------------------------ *
 vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
 * ------------------------------------------------------------------------ */
