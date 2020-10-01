/* ------------------------------------------------------------------------ */
#define NORETURN __attribute__((noreturn))
#define UNUSED   __attribute__((unused)) /*@unused@*/
#define PACKED   __attribute__ ((packed))
/* ------------------------------------------------------------------------ */
typedef uint32_t  pft;                          /* The size of a prefix table entry. */
static uint32_t   SIZE_PREFIX_TABLE;            /* array length */
/* ------------------------------------------------------------------------ */
/* The 8 byte structure in the disk 'storage' file. */
struct worder
{
    char     let;
    pft      further_let;
    pft      another_let;
} PACKED w;
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  */
#define MEM_STORAGE     (256*1024)              /* 2^16 Size of memory for buffers (storage). */
#define RECLTH          (8*1024)                /* Record length of 'storage'. */
#define G_NBLKS         (MEM_STORAGE / RECLTH)  /* Number of buffers. */
static struct worder *storage_buffer;           /* The storage buffer (disk). */
#define W_PER_RECLTH    (RECLTH / sizeof(struct worder))
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  */
#define POINTER_FILENAME "pointer_file"
static FILE *pointer_file;                      /* Pointer for storage_file. */
static pft   storage_used;                      /* The next "storage" place is unused. 0 means empty list. */
/*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#define PREFIX_TABLE_FILENAME "prefix_table_file"
static FILE *prefix_table_file;                 /* The FILE pointer to the prefix_table_file. */
/*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#define STORAGE_FILENAME "storage_file"
static FILE *storage_file;                      /* The FILE pointer to the 'storage' file. */
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  */
/* Array of G_NBLKS long. */
static int g_blk[G_NBLKS];                      /* Block or "storage buffer". */
/*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/* The prefix search table */
#define LPREFIX         3                       /* Number of chars in prefix "search" table */
#define ARRAYLTH_PREFIX_VALUE 27                /* Letters plus null/termination of 0. */

#define NOT_IN_PREFIX_TABLE 0                   /* Value for no entry in prefix table. */
#define ONLY_IN_PREFIX_TABLE 0xffffffff         /* This value means there is no 'storage' entry. */

static struct prefix_table
{
    pft      storage_num;                       /* The pointer to where to continue. */
} *prefix_table;                                /* The table. */

/* ------------------------------------------------------------------------ */
/*
 * vi: sw=4 ts=8 expandtab
 */
/* End of m4test.h */
