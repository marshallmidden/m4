/* ------------------------------------------------------------------------ */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>

#include "m4test.h"
/* ------------------------------------------------------------------------ */
static void din(int starting_record, int storage)
{
    size_t rl;

    /* Go to place in file. */
    if (fseeko(storage_file, starting_record * RECLTH, SEEK_SET) != 0)
    {
        perror("fseeko of storage_file failure (din):");
        exit(1);
    }
    rl = fread(&storage_buffer[storage * W_PER_RECLTH], sizeof(*storage_buffer), W_PER_RECLTH, storage_file);
    if (rl < W_PER_RECLTH)                              /* Error occurred. */
    {
        memset(&storage_buffer[storage * W_PER_RECLTH + rl], 0, (W_PER_RECLTH - rl) * sizeof(*storage_buffer));
    }
}   /* End of din */

/* ------------------------------------------------------------------------ */
static void print_prefix_entries(void)
{
    unsigned int small = 0;
    unsigned int empty = 0;
    unsigned int used = 0;
    unsigned int entry;
    unsigned int chr_in_wrd;
    unsigned int s = SIZE_PREFIX_TABLE;
    char a[LPREFIX + 1];
    a[LPREFIX] = '\0';

fprintf(stderr, "s=%d\n", s);;
    for (entry = 0; entry < s; entry++)
    {
        if (prefix_table[entry].storage_num == NOT_IN_PREFIX_TABLE)
        {
            empty++;
            continue;
        }
        used++;
        for (chr_in_wrd = 0; chr_in_wrd < LPREFIX; chr_in_wrd++)
        {
            int j;
            int l;

            if (chr_in_wrd == 0)
            {
                j = entry/ARRAYLTH_PREFIX_VALUE;
                l = entry - (j * ARRAYLTH_PREFIX_VALUE);
            }
            else
            {
                /* 2nd and additional characters. */
		unsigned int k;
                unsigned int p = 1;
                for (k = 0; k < chr_in_wrd; k++)
                {
                    p *= ARRAYLTH_PREFIX_VALUE;
                }
                k = entry / p;
                j = k / ARRAYLTH_PREFIX_VALUE;
                l = k - (j * ARRAYLTH_PREFIX_VALUE);
            }
            a[LPREFIX - chr_in_wrd -1] = (l == 0) ? ' ' : 'a' + l - 1;
        }
        if (prefix_table[entry].storage_num != ONLY_IN_PREFIX_TABLE)
        {
            small++;
        }
        printf("prefix_entry %-6d '%s' -> %08x (%7d)\n", entry, a, prefix_table[entry].storage_num, prefix_table[entry].storage_num);
    }
    printf("Number of prefix entries used=%d small=%d empty=%d\n", used, small, empty);
}   /* End of print_prefix_entries */

/* ------------------------------------------------------------------------ */
static void print_storage_file(void)
{
    unsigned int i;
    int buffer = 0;
    unsigned int j;
    char a;

    for (i = 0; i < (storage_used - 1); i += W_PER_RECLTH)
    {
        din(buffer, 0);

        for (j = 0; j < W_PER_RECLTH; j++)
        {
            if ((buffer * W_PER_RECLTH + j) >= storage_used)
            {
                return;
            }

            if (storage_buffer[j].let == '\0') { a = ' '; }
            else { a = isprint((int)storage_buffer[j].let) ? storage_buffer[j].let : '?'; }

            printf("storage_block %03d-%03d(%07ld) '%c'  further_let=%07d    another_let=%07d\n", buffer, j, buffer*W_PER_RECLTH + j,
                    a, storage_buffer[j].further_let, storage_buffer[j].another_let);
        }

        buffer++;
    }
}   /* End of print_storage_file */

/* ------------------------------------------------------------------------ */
static void initget(void)
{
    unsigned int work2;
    size_t   rl;

    /* Get memory for prefix table. */
    prefix_table = malloc(SIZE_PREFIX_TABLE * sizeof(*prefix_table));
    if (prefix_table == NULL)
    {
        perror("malloc -- not enough memory for prefix_table:");
        exit(1);
    }
    memset(prefix_table, 0, SIZE_PREFIX_TABLE * sizeof(*prefix_table));

    /* Read in prefix table. */
    prefix_table_file = fopen(PREFIX_TABLE_FILENAME, "rb+");
    if (prefix_table_file == NULL)
    {
        perror("fopen of prefix_table_file failed:");
        exit(1);
    }

    int s = SIZE_PREFIX_TABLE;

    rl = fread(prefix_table, sizeof(*prefix_table), s, prefix_table_file);
    if (rl > 0 && rl < (size_t)s)               /* NOT initialized */
    {
        fprintf(stderr, "prefix_table read (%ld) < supposed #%d entries (meaning %ld)\n",
                rl, s, sizeof(*prefix_table) * s);
        perror("fread prefix_table entries failed:");
        exit(1);
    }

/*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    /* Read in pointer_file for 'storage' storage_used variable. */
    pointer_file = fopen(POINTER_FILENAME, "rb+");
    if (pointer_file == NULL)
    {
        perror("fopen of pointer_file failed:");
        exit(1);
    }
    rl = fread(&storage_used, sizeof(storage_used), 1, pointer_file);
    if (rl < 1)                                         /* Error occurred */
    {
        storage_used = 0;                               /* No 'storage' either. */
        fprintf(stderr, "number of storage entries initialized\n");
    }

/*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    /* Get memory for disk buffers saved in memory. */
    storage_buffer = (struct worder *)malloc(MEM_STORAGE);      /* Memory for buffers gotten. */
    if (storage_buffer == NULL)
    {
        perror("malloc -- not enough memory for worder:");
        exit(1);
    }

    storage_file = fopen(STORAGE_FILENAME, "rb+");
    if (storage_file == NULL)
    {
        perror("fopen of storage_file failed:");
        exit(1);
    }

    for (work2 = 0; work2 < G_NBLKS; work2++)
    {
        g_blk[work2] = -1;                              /* No block buffer data in this area. */
    }
}   /* End of initget */

/* ------------------------------------------------------------------------ */
int main(int argc UNUSED, char **argv UNUSED)
{
    int i;

    SIZE_PREFIX_TABLE = 1;
    for (i = 0; i< LPREFIX; i++)
    {
        SIZE_PREFIX_TABLE *= ARRAYLTH_PREFIX_VALUE;
    }                                   /* End of for */

    initget();

    printf("------------------------------------------------------------------------------\n");
    print_prefix_entries();
    printf("------------------------------------------------------------------------------\n");
    print_storage_file();

    exit(0);
}   /* End of main */

/* ------------------------------------------------------------------------ */
/*
 * vi: sw=4 ts=8 expandtab
 */
/* End of dumptables.c */
