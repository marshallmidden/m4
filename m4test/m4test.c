#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <ncurses.h>
/* ------------------------------------------------------------------------ */
#include <termios.h>
static struct termios original_termios;
static char     original_termios_gotten = 0;

static int      terminal_col;                   /* columns on terminal. */
static int      terminal_row;                   /* rows on terminal. */
/* ------------------------------------------------------------------------ */
#include "m4test.h"
/*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#define INLENGTH        (15*10)
/*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/* Array of G_NBLKS long. */
#define BLK_CHANGED     0
#define BLK_UNCHANGED   1
static unsigned char g_alter[G_NBLKS];  /* 0 for changed, 1 for not changed. */
static clock_t  g_lacc[G_NBLKS];        /* last access - clock if used. */
/*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
static unsigned char global_do_not_write = 0;

/* ======================================================================== */
/* forward. */
static void restore_original_termios(void);
/* ------------------------------------------------------------------------ */
NORETURN static void exit_perror(const char *str)
{
    perror(str);
    endwin();
    restore_original_termios();
    exit(1);
}    /* End of exit_perror */

/* ------------------------------------------------------------------------ */
static void restore_original_termios(void)
{
    if (original_termios_gotten == 0)
    {
        return;
    }
    original_termios.c_lflag |= ICANON;
    original_termios.c_lflag |= ECHO;
    original_termios_gotten = 0;
    if (tcsetattr(0, TCSADRAIN, &original_termios) < 0)
    {
        exit_perror("tcsetattr ICANON");
    }
}                                       /* End of restore_original_termios */

/* ------------------------------------------------------------------------ */
static void get_original_termios(void)
{
    if (isatty(0))
    {
        if (tcgetattr(0, &original_termios) < 0)
        {
            exit_perror("tcgetattr()");
        }
        original_termios_gotten = 1;
    }

    if (original_termios_gotten != 0)
    {
        original_termios.c_lflag &= ~ICANON;
        original_termios.c_lflag &= ~ECHO;
        original_termios.c_cc[VMIN] = 1;
        original_termios.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &original_termios) < 0)
        {
            exit_perror("tcsetattr ICANON");
        }
    }
}                                       /* End of get_original_termios */

/* ------------------------------------------------------------------------ */
static char get_single_ch(void)
{
    char            buf = 0;

    if (read(0, &buf, 1) < 0)
    {
        exit_perror("read()");
    }
    return (buf);
}                                       /* End of get_single_ch */

/* ------------------------------------------------------------------------ */
static void write_pointer_file(void)
{
    size_t          wl;

    if (global_do_not_write == 1)
    {
        return;
    }
    if (fseeko(pointer_file, 0, SEEK_SET) != 0) /* Go to start of file. */
    {
        exit_perror("fseeko pointer_file failure:");
    }
    wl = fwrite(&storage_used, sizeof(storage_used), 1, pointer_file);
    if (wl < 1)                         /* Error occurred. */
    {
        exit_perror("fwrite of number of storage entries failed:");
    }
    fflush(pointer_file);
}                                       /* End of write_pointer_file */

/* ------------------------------------------------------------------------ */
static void write_prefix_table_file(void)
{
    size_t          wl;

    if (global_do_not_write == 1)
    {
        return;
    }
    if (fseeko(prefix_table_file, 0, SEEK_SET) != 0)    /* Go to start of file. */
    {
        exit_perror("fseeko prefix_table_file failure:");
    }
    wl = fwrite(prefix_table, sizeof(*prefix_table), SIZE_PREFIX_TABLE, prefix_table_file);
    size_t s = SIZE_PREFIX_TABLE;
    if (wl < s)                         /* Error occurred. */
    {
        exit_perror("fwrite of prefix_table entries failed:");
    }
    fflush(prefix_table_file);
}                                       /* End of write_prefix_table_file */

/* ------------------------------------------------------------------------ */
static void din(unsigned int starting_record, unsigned int storage)
{
    size_t          rl;

    /* Go to place in file. */
    if (fseeko(storage_file, starting_record * RECLTH, SEEK_SET) != 0)
    {
        exit_perror("fseeko of storage_file failure (din):");
    }
    rl = fread(&storage_buffer[storage * W_PER_RECLTH], sizeof(*storage_buffer),
               W_PER_RECLTH, storage_file);
    if (rl < W_PER_RECLTH)              /* Error occurred. */
    {
        memset(&storage_buffer[storage * W_PER_RECLTH + rl], 0,
               (W_PER_RECLTH - rl) * sizeof(*storage_buffer));
    }
}                                       /* End of din */

/* ------------------------------------------------------------------------ */
static void dout(unsigned int starting_record, unsigned int storage)
{
    size_t          wl;

    /* Go to place in file. */
    if (fseeko(storage_file, starting_record * RECLTH, SEEK_SET) != 0)
    {
        exit_perror("fseeko of storage_file failure (dout):");
    }
    wl = fwrite(&storage_buffer[storage * W_PER_RECLTH], sizeof(*storage_buffer),
                W_PER_RECLTH, storage_file);
    if (wl < W_PER_RECLTH)              /* Error occurred. */
    {
        exit_perror("fwrite of storage_file failed (dout):");
    }
}                                       /* End of dout */

/* ------------------------------------------------------------------------ */
static void cleanup(void)
{
    unsigned int    work2;
    unsigned char   flag = 0;

    for (work2 = 0; work2 < G_NBLKS; work2++)
    {
        if (g_alter[work2] == BLK_CHANGED)
        {
            if (flag == 0)
            {
                write_pointer_file();   /* Update variable storage_used on disk. */
                flag = 1;
            }
            dout(g_blk[work2], work2);
            g_alter[work2] = BLK_UNCHANGED;
        }
    }                                   /* End of for */
    if (flag == 1)
    {
        fflush(storage_file);
    }
}                                       /* End of cleanup */

/* ------------------------------------------------------------------------ */
static pft find_prefix_table(char *check_chars, pft *where)
{
    unsigned int i;

    *where = 0;
    for (i = 0; i < LPREFIX; i++)
    {
        *where = (*where * ARRAYLTH_PREFIX_VALUE) + ((check_chars[i] == 0)?0: (check_chars[i] - 'a' + 1));
    }
    return (prefix_table[*where].storage_num);
}                                       /* End of find_prefix_table */

/* ------------------------------------------------------------------------ */
static void add_prefix_table(pft *where)
{
    if (prefix_table[*where].storage_num != 0)
    {
        fprintf(stderr, "%s:%u:%s error in prefix table handling.\n", __FILE__, __LINE__, __func__);
	endwin();
	restore_original_termios();
        exit(1);
    }
    prefix_table[*where].storage_num = ONLY_IN_PREFIX_TABLE;

    write_prefix_table_file();          /* Save changes to prefix_table. */
}                                       /* End of add_prefix_table */

/* ------------------------------------------------------------------------ */
#if 0
s=19683
prefix_entry 1177   'app' -> 00000001 (      1)
prefix_entry 1488   'bac' -> 00000007 (      7)
prefix_entry 1499   'ban' -> 00000003 (      3)
Number of prefix entries used=3 small=3 empty=19680
#endif // 0

/* Need to search through prefix_table for one or more found. */
/* Return first entry found. */
/* Returns count = number of entries found. */

static pft count_find_prefix_table(unsigned char *count, char *check_chars, unsigned char wordchr)
{
    pft             i;
    pft             first = 0;          /* Not checked upon use -- count used instead. */
    pft             v_min;
    pft             v_max;
    unsigned int    match = 0;

    v_min = 0;
    v_max = 0;
    for (i = 0; i < LPREFIX; i++)
    {
        v_min *= ARRAYLTH_PREFIX_VALUE;
        v_max *= ARRAYLTH_PREFIX_VALUE;
        if (i <= wordchr)
        {
            v_min += ((check_chars[i] == 0) ? 0 : (check_chars[i] - 'a' + 1));
            v_max = v_min;
        }
        else
        {
            v_max += ('z' - 'a' + 1);
        }
    }                                   /* End of for */
    *count = 0;
    for (i = v_min; i <= v_max; i++)
    {
        if (prefix_table[i].storage_num != NOT_IN_PREFIX_TABLE)
        {
	    if (wordchr == (LPREFIX-1))
	    {
		*count = 1;
		return(i);
	    }
	    /* 2nd and additional characters. */
	    unsigned int s = 1;
	    int k;
	    for (k = 0; k < LPREFIX-1; k++)
	    {
		s *= ARRAYLTH_PREFIX_VALUE;
	    }
	    unsigned int j;
	    unsigned int l;
	    for (k = 0; k <= wordchr; k++)
	    {
		s /= ARRAYLTH_PREFIX_VALUE;
	    }
	    k = (i / s);
	    j = k/ARRAYLTH_PREFIX_VALUE;
	    l = k - (j * ARRAYLTH_PREFIX_VALUE);
	    if (*count == 0)
	    {
		*count = 1;
		match = l;
	    }
	    else
	    {
	        if (match != l)
		{
		    *count = 2;
		    return(first);
		}
            }
            first = i;
        }
    }                                   /* End of for */
    return (first);
}                                       /* End of count_find_prefix_table */

/* ------------------------------------------------------------------------ */
static void getword(pft storage_num, struct worder *w_from_storage)
{
    int             work;
    unsigned int    work1;
    unsigned int    i;
    clock_t         work3;
    unsigned int    read_block;

    work = storage_num / W_PER_RECLTH;
    work1 = storage_num - (W_PER_RECLTH * work);        /* Entry inside of block buffer. */

    /* Is block already present? */
    for (i = 0; i < G_NBLKS; i++)
    {
        if (g_blk[i] == work)
        {
            *w_from_storage = storage_buffer[(W_PER_RECLTH * i) + work1];
            g_lacc[i] = time(0);
            return;
        }
    }                                   /* End of for */

    /* See if anything needs to be written to disk. */
    cleanup();

    /* Find block buffer to overwrite. */
    read_block = 1;                     /* Never use the first one of 0. */
    work3 = g_lacc[1];                  /* Second ones last access time. */
    for (i = 0; i < G_NBLKS; i++)
    {
        if (g_blk[i] < 0)               /* If storage block unused. */
        {
            read_block = i;
            break;
        }
        if (work3 >= g_lacc[i] && i != 0)       /* Get oldest one. */
        {
            work3 = g_lacc[i];
            read_block = i;
        }
    }                                   /* End of for */
    i = read_block;                     /* Use oldest, non-first block. */
    g_blk[i] = work;
    din(work, i);
    g_alter[i] = BLK_UNCHANGED;

    *w_from_storage = storage_buffer[(W_PER_RECLTH * i) + work1];
    g_lacc[i] = time(0);
}                                       /* End of getword */

/* ------------------------------------------------------------------------ */
static void putword(pft storage_num, struct worder w_from_storage)
{
    int             work;
    unsigned int    work1;
    unsigned int    work2;

    do
    {
        work = storage_num / W_PER_RECLTH;
        work1 = storage_num - (W_PER_RECLTH * work);
        for (work2 = 0; work2 < G_NBLKS; work2++)
        {
            if (g_blk[work2] == work)
            {
                /* put entry into block buffer. */
                storage_buffer[(W_PER_RECLTH * work2) + work1] = w_from_storage;
                g_alter[work2] = BLK_CHANGED;
                g_lacc[work2] = time(0);
                return;
            }
        }                               /* End of for */
        /* If not found in block buffer */
        struct worder sworder;          /* Ignore fetched word. */
        getword(storage_num, &sworder);  /* Get block buffer filled. */
    } while (1);
}                                       /* End of putword */

/* ------------------------------------------------------------------------ */
enum find_return
{
    FR_NO_INPUT,                        /* Error, no input word. */
                                        /* fillin points to prefix entry. */
    FR_NOT_FOUND_PREFIX_TABLE_ADDED,    /* Not found in prefix table, added. */
                                        /* fillin points to prefix entry. */
                                        /* We know it is longer than 3 characters. */
    FR_FOUND_SHORT_WORD,                /* Found short word. */
                                        /* fillin points to prefix entry. */
    FR_FOUND_SHORT_WORD_NEW,            /* Found short word. */
                                        /* fillin points to prefix entry. */
    FR_PREFIX_SHORT_WORD_THIS_LONGER,   /* Found LPREFIX character entry. */
                                        /* fillin points to prefix entry. */
                                        /* Need a "zero" for end of last, and add */
                                        /* longer words characters to storage. */
    FR_FOUND_LONG_WORD,                 /* Found long word (> LPREFIX). */
                                        /* fillin points to prefix entry. */
                                        /* storage_num points to last storage entry. */
    FR_LONG_WORD_LONGER,                /* Found long word (> LPREFIX), and longer */
                                        /* than existing one. i.e. add "zero". */
                                        /* fillin points to prefix entry. */
                                        /* storage_num points to storage entry. */
    FR_NOT_FOUND_LONG_WORD,             /* Not found in storage. */
                                        /* fillin points to prefix entry. */
                                        /* storage_num points to last storage entry, */
                                        /* or the one to add another_let to. */
};

static enum find_return findword(pft *fillin, unsigned int *inat, char *in, pft *storage_num)
{
    unsigned int    i;
    struct worder   w_from_storage;

    *inat = 0;                          /* inat set to character last found. */
    if (in[0] == '\0')
    {
        return (FR_NO_INPUT);
    }
    *inat = LPREFIX - 1;
    for (i = 1; i < LPREFIX; i++)
    {
        if (in[i] == '\0')
        {
            *inat = i - 1;
            break;
        }
    }                                   /* End of for */

    *storage_num = 0;
    *fillin = 0;                        /* No fillin (not checked for, just initialize to something). */
    if (storage_used == 0)
    {
        /* Start at 1. 0 means end of next list or letters. */
        storage_used = 1;
    }

    *storage_num = find_prefix_table(&in[0], fillin);   /* Find in prefix_table. */
    if (*storage_num == NOT_IN_PREFIX_TABLE)    /* Not found in prefix table. */
    {
        add_prefix_table(fillin);       /* Add to prefix_table. */
        /* See if a short word existing only in prefix table. */
        for (i = 1; i <= LPREFIX; i++)
        {
            if (in[i] == '\0')
            {
                return (FR_FOUND_SHORT_WORD_NEW);
            }
        }                               /* End of for */
        return (FR_NOT_FOUND_PREFIX_TABLE_ADDED);
    }

    if (*storage_num == ONLY_IN_PREFIX_TABLE)
    {
        /* See if a short word existing only in prefix table. */
        for (i = 1; i <= LPREFIX; i++)
        {
            if (in[i] == '\0')
            {
                return (FR_FOUND_SHORT_WORD);
            }
        }                               /* End of for */
        *inat = LPREFIX - 1;
        return (FR_PREFIX_SHORT_WORD_THIS_LONGER);
    }
    /* At this point, we have found an entry.  */
    *inat = LPREFIX;

    /* At this point, we know we have a long word in storage and 'in'. */
    /* Check if next character in input is found. */
    do
    {
        /* Check for storage entry match. */
        do
        {
            getword(*storage_num, &w_from_storage);
            if (w_from_storage.let != in[*inat])
            {
                if (w_from_storage.another_let == 0)
                {
                    return (FR_NOT_FOUND_LONG_WORD);
                }
                *storage_num = w_from_storage.another_let;
                continue;
            }
            if (in[*inat] == 0)
            {
                return (FR_FOUND_LONG_WORD);
            }
            if (w_from_storage.further_let != 0)    /* not end of word in storage */
            {
                *storage_num = w_from_storage.further_let;
                break;                  /* Go find next input character. */
            }
            if (in[*inat + 1] == 0)
            {
                return (FR_FOUND_LONG_WORD);
            }
            return (FR_LONG_WORD_LONGER);       /* New longer word found. */
        } while (1);
        (*inat)++;
    } while (1);
}                                       /* End of findword */

/* ------------------------------------------------------------------------ */
/* Return: 0 means duplicate word. */
enum add_return
{
    AR_NO_INPUT,                        /* Error, no input word. */
    AR_DUPLICATE,                       /* Tried to add duplicate word. */
    AR_ADDED_WORD,                      /* Added word. */
};

static enum add_return addword(char *in)
{
    enum find_return find_ret;
    pft             fillin;
    unsigned int    inat;
    pft             storage_num;        /* list returned pointer (storage). */
    struct worder   w_from_storage;

    /* Find and if not found add to prefix_table -- fillin set. */
    find_ret = findword(&fillin, &inat, in, &storage_num);
    switch (find_ret)
    {
        case FR_NO_INPUT:
            return (AR_NO_INPUT);

        case FR_FOUND_SHORT_WORD:
        case FR_FOUND_LONG_WORD:
            return (AR_DUPLICATE);

        case FR_FOUND_SHORT_WORD_NEW:
            return (AR_ADDED_WORD);

        case FR_PREFIX_SHORT_WORD_THIS_LONGER:
            /* Existing prefix table word is 3 characters long, but this one is */
            /* longer. Must add storage entry for '\0' in let, with further_let 0. */
            /* another_let points to another storage entry that has the non-zero */
            /* character in let, and further_let pointing to other characters (if */
            /* non-zero), and another_let 0. */

            prefix_table[fillin].storage_num = storage_used;
            write_prefix_table_file();

          add_zero_then_rest_of_word:
            /* Add the zero for end of prefix table word. */
            storage_num = storage_used;
            storage_used++;
            w_from_storage.let = '\0';
            w_from_storage.further_let = 0;
            w_from_storage.another_let = storage_used;
            putword(storage_num, w_from_storage);
            goto add_rest;

        case FR_NOT_FOUND_PREFIX_TABLE_ADDED:
            /* Prefix was added. We know there are more than 3 characters. */
            /* Note: inat set to LPREFIX-1 */
            /* storage_num = 0; */
            /* fillin = prefix table entry that was added. */

            prefix_table[fillin].storage_num = storage_used;
            write_prefix_table_file();
          add_rest: /* Add rest of characters in word. */
            do
            {
                inat++;
                storage_num = storage_used;
                storage_used++;
                w_from_storage.let = in[inat];
                if (in[inat + 1] == '\0') {
                    w_from_storage.further_let = 0;
                }
                else
                {
                    w_from_storage.further_let = storage_used;
                }
                w_from_storage.another_let = 0;
                putword(storage_num, w_from_storage);
            } while (in[inat + 1] != '\0');
            return (AR_ADDED_WORD);                 /* added. */

        case FR_NOT_FOUND_LONG_WORD:
            /* Not found in storage. */
            /* storage_num points to last storage entry, */
            /* or the one to add another_let to. */
            /* Add another_letter to check location in storage. */
            getword(storage_num, &w_from_storage);
            w_from_storage.another_let = storage_used;
            putword(storage_num, w_from_storage);
            inat--;                     /* position pointer for upcoming ++ */
            goto add_rest;

        case FR_LONG_WORD_LONGER:
            /* Long word, and new word is longer than existing. */
            /* storage_num points to storage entry. */
            getword(storage_num, &w_from_storage);
            w_from_storage.further_let = storage_used;
            putword(storage_num, w_from_storage);
            goto add_zero_then_rest_of_word;

        default:
            fprintf(stderr, "findword return not handled (%d)\n", find_ret);
	    endwin();
	    restore_original_termios();
            exit(1);
    }                                   /* End of switch */
}                                       /* End of addword */

/* ------------------------------------------------------------------------ */
static void enter(void)
{
    unsigned int    i;
    enum add_return add_ret;
    char            in[INLENGTH];

    do
    {
	erase();
	refresh();
        printw("Word to enter: > ");
	refresh();
        memset(&in[0], 0, INLENGTH);
	restore_original_termios();
        if (fgets(&in[0], INLENGTH, stdin) == NULL)
        {
	    endwin();
            exit(0);                    /* exit on end of file with good return */
        }
	get_original_termios();

        for (i = 0; i < INLENGTH && in[i] != '\0'; i++)
        {
            while (in[i] == ' ')        /* delete spaces */
            {
                memmove(&in[i], &in[i + 1], INLENGTH - i - 1);
                in[INLENGTH - 1] = '\0';
            }                           /* End of while */
            if (in[i] == '\n')          /* delete end of line */
            {
                in[i] = '\0';
                break;
            }
        }                               /* End of for */
	refresh();
        if (in[0] != '\0')
        {
            add_ret = addword(in);
            cleanup();
            if (add_ret == AR_NO_INPUT)
            {
                printw("No input?\n");
            }
            else if (add_ret == AR_DUPLICATE)
            {
                printw("Already exists -- found (%s).\n", in);
            }
            else
            {
                printw("Added new word -- (%s).\n", in);
            }
        }
        else
        {
            break;
        }
	refresh();
    } while (1);
}                                       /* End of enter */

/* ------------------------------------------------------------------------ */
static void print_arrow(void)
{
    printw("Fake Marshall Midden Special Arrow > ");
}                                       /* End of print_arrow */

/* ------------------------------------------------------------------------ */
static void single_char_possibilities(void)
{
    unsigned int    i;
    pft             prefix_on;
    unsigned int    increment;
    unsigned char   flag = 0;
    char            wrd;
    
    increment = ARRAYLTH_PREFIX_VALUE;
    for (i = 1; i < (LPREFIX -1); i++)
    {
        increment = increment * ARRAYLTH_PREFIX_VALUE;
    }

    for (prefix_on = 0; prefix_on < SIZE_PREFIX_TABLE; prefix_on += increment)
    {
        if (prefix_table[prefix_on].storage_num == NOT_IN_PREFIX_TABLE)
        {
            continue;                   /* No entry. */
        }
        unsigned int k = prefix_on / increment;
        wrd = (k == 0) ? '*' : 'A' + k - 1;
        printw("%s%c", (flag==0)?"":",", wrd);
        flag = 1;
    }                                   /* End of for */
}                                       /* End of single_char_possibilities */

/* ------------------------------------------------------------------------ */
static void  second_char_possibilities(unsigned int chr_in_wrd, char *wrd)
{
    pft             i;
    pft             v_min;
    pft             v_max;
    unsigned char   flag = 0;
    unsigned int    increment = 1;

    for (i = LPREFIX-1; i > chr_in_wrd; i--)
    {
        increment = increment * ARRAYLTH_PREFIX_VALUE;
    }

    v_min = 0;
    v_max = 0;
    for (i = 0; i < LPREFIX; i++)
    {
        v_min = (v_min * ARRAYLTH_PREFIX_VALUE);
        v_max = (v_max * ARRAYLTH_PREFIX_VALUE);
        if (i < chr_in_wrd)
        {
            v_min += ((wrd[i] == 0) ? 0 : (wrd[i] - 'a' + 1));
            v_max = v_min;
        }
        else
        {
            v_max += ('z' - 'a' + 1);
        }
    }                                   /* End of for */
    for (i = v_min; i <= v_max; i += increment)
    {
        unsigned int    j;
        unsigned char   p_fnd = 0;
        for (j = 0; j < increment; j++)
        {
            if (prefix_table[i+j].storage_num != NOT_IN_PREFIX_TABLE)
            {
                p_fnd = 1;
                break;
            }
        }                               /* End of for */
        if (p_fnd == 0)
        {
            continue;
        }
        unsigned int k = i / increment;
        k = k % ARRAYLTH_PREFIX_VALUE;
        char l = (k == 0) ? '*' : 'a' + k - 1;
        printw("%s%c", (flag==0)?"":",", l);
        flag = 1;
    }                                   /* End of for */
}                                       /* End of second_char_possibilities */

/* ------------------------------------------------------------------------ */
static void storage_char_possibilities(pft storage_num)
{
    struct worder   worder;
    char            c;
    unsigned char   flag = 0;

    getword(storage_num, &worder);  /* Get block buffer filled. */

    c = (worder.let == 0) ? '*' : worder.let;
    printw("%s%c", (flag==0)?"":",", c);
    flag = 1;

    /* Now do the another_let pointer. */
    while (worder.another_let != 0)
    {
        getword(worder.another_let, &worder);  /* Get block buffer filled. */
        c = (worder.let == 0) ? '*' : worder.let;
        printw("%s%c", (flag==0)?"":",", c);
        flag = 1;
    }
}                                       /* End of storage_char_possibilities */

/* ------------------------------------------------------------------------ */
static void tab_pressed(char *in, unsigned int inat, char *wrd UNUSED, unsigned int chr_in_wrd, pft ostorage_num)
{
    printw("\n");
    printw("Possibilities: ");

    if (chr_in_wrd == 0)
    {
        single_char_possibilities();
    }
    else if (chr_in_wrd < LPREFIX)
    {
        second_char_possibilities(chr_in_wrd, wrd);
    }
    else
    {
        storage_char_possibilities(ostorage_num);
    }

    /* need to get screen back to input arrow looking thing. */
    printw("\n");
    print_arrow();
    if (inat > 0)
    {
        printw("%s", in);
    }
}                                       /* End of tab_pressed */

/* ------------------------------------------------------------------------ */
/* NOTDONEYET -- something fishy with erasing "sometimes". Stack/storage_num not quite right. */
/* Example is Hellish, using tab at 'Hell', then 'i' and tab, then 's' and tab, then 'n' tab -- nothing. */
/* and then erase. Then tab -- and it is wrong -- giving an 'h' instead of 'l,n'. */

/* Output is new inat. */
static void eraser(unsigned int *inat, char *in,
                   unsigned char *wordchr, char *wrd,
                   pft *ostorage_num, unsigned char *putcap,
                   unsigned int *stakpt, unsigned int *st_inat,
                   pft *ost_list, unsigned char *st_putcap)
{
    unsigned int    work;

    for (work = *inat; work > st_inat[*stakpt]; work--)
    {
        (*wordchr)--;
        wrd[*wordchr] = '\0';
        in[work-1] = 0;
        printw("\b \b");
    }                                   /* End of for */

    *inat = st_inat[*stakpt];
    *putcap = st_putcap[*stakpt];
    (*stakpt)--;
    *ostorage_num = ost_list[*stakpt];  /* -1 ? */
}                                       /* End of eraser */

/* ------------------------------------------------------------------------ */
#define MAX_STAKPT      200

/* Returns the number of characters typed (inat). */
static void type(char *in, unsigned int *inat)
{
    unsigned char   waskeypressed;      /* 1 if key was pressed, else 0. */
    unsigned char   putcap;             /* If should capitalize the next character. */
    unsigned char   oputcap;            /* Saved -- if key is bad. */
/*  Need to know about putcap "saved". For erase, or bad keys and resetting. */

    char            pressedkey;
    char            showkey = '?';

    char            mkey;               /* my key - working lower case key, from wherever. */
    unsigned char   chr_in_wrd;         /* Index into wrd array. */
    char            wrd[INLENGTH];      /* The word working on (lower case). */

    unsigned char   prefix_count;       /* Used for searching through prefix table for wrd. */
    pft             first_entry;        /* First prefix entry found. */

    pft             storage_num;        /* list returned pointer (storage). */
    pft             ostorage_num;       /* DOCUMENT THIS VARIABLE. */

    struct worder   w_from_storage;     /* 'storage' entry for beyond prefix table. */

    /* Used when a key was pressed -- verses automatically printed because of uniqueness. */
    unsigned int    stakpt;             /* pointer into a stack (the st. variables) */
    unsigned int    st_inat[MAX_STAKPT];   /* start of input before automatic entry occurred. */
    pft             st_olist[MAX_STAKPT];  /* The 'ostorage' entry/number working on. */
    unsigned char   st_putcap[MAX_STAKPT]; /* erase of first character might have to put capital on it. */

    /* Initialize to nothing in arrow buffer, need a capital for first key. */
    *inat = 0;
    memset(&in[0], 0, INLENGTH);
    oputcap = putcap = 1;

  start1:
    memset(&wrd[0], 0, INLENGTH);
    chr_in_wrd = 0;                             /* No word working on yet. */
    ostorage_num = 0;
    storage_num = 0;
    stakpt = 0;
    st_inat[stakpt] = *inat;
    st_olist[stakpt] = ostorage_num;
    st_putcap[stakpt] = putcap;
    oputcap = putcap;

  pause1:
    storage_num = ostorage_num;
    refresh();
    pressedkey = get_single_ch();
    waskeypressed = 1;                          /* pressed key */
    if (pressedkey == '\t')                     /* tab key */
    {
        tab_pressed(in, *inat, wrd, chr_in_wrd, ostorage_num);
        goto pause1;
    }
    if (pressedkey == '\b' || pressedkey == 0x7f)   /* backspace or delete key */
    {
      erase1:
        if (chr_in_wrd == 0)
        {
            goto start1;
        }
        eraser(inat, in, &chr_in_wrd, wrd, &ostorage_num, &putcap, &stakpt, st_inat, st_olist, st_putcap);
        if (chr_in_wrd == 0)
        {
            goto start1;
        }
        goto pause1;
    }
    if (pressedkey == '\n' || pressedkey == '\r' ||                     /* return or new-line */
        pressedkey == ' ' || pressedkey == ',' || pressedkey == '.')    /* space, comma, or period */
    {
        if (chr_in_wrd == 0 && (pressedkey == '\n' || pressedkey == '\r'))
        {
            return;
        }
        mkey = 0;                               /* Flag end of word wanted. */
    }
    else if (pressedkey <= 0x1f)                /* ignore other function keys */
    {
        goto pause1;
    }
    else
    {
        mkey = (char)tolower((int)pressedkey);
        if (putcap == 1)
        {
            showkey = (char)toupper((int)mkey); /* Turn from lower case to upper case. */
            putcap = 0;
        }
        else
        {
            showkey = mkey;
        }
    }

  runthru1:
    if (chr_in_wrd < LPREFIX)
    {
        wrd[chr_in_wrd] = mkey;

        first_entry = count_find_prefix_table(&prefix_count, wrd, chr_in_wrd);
        if (prefix_count == 0)
        {
            wrd[chr_in_wrd] = 0;                /* Delete key pressed from wrd. */
            putcap = oputcap;                   /* Restore putcap. */
            goto pause1;
        }
        if (mkey == '\0')
        {
            goto done1;
        }
        ostorage_num = prefix_table[first_entry].storage_num;
        if (waskeypressed != 0)
        {
            stakpt++;
            st_inat[stakpt] = *inat;
            st_olist[stakpt] = ostorage_num;
            st_putcap[stakpt] = oputcap;
            oputcap = putcap;
            waskeypressed = 0;
        }
        in[*inat] = showkey;
        (*inat)++;
        chr_in_wrd++;
        printw("%c", showkey);
        if (prefix_count != 1)                  /* Two or more found. */
        {
            goto pause1;
        }
        if (chr_in_wrd < LPREFIX)
        {
            int k;
            unsigned int j;
            unsigned int l;

            /* 2nd and additional characters. */
            unsigned int s = 1;
            for (k = 0; k < LPREFIX - chr_in_wrd -1; k++)
            {
                s *= ARRAYLTH_PREFIX_VALUE;
            }
            k = (first_entry / s);
            j = k/ARRAYLTH_PREFIX_VALUE;
            l = k - (j * ARRAYLTH_PREFIX_VALUE);
            showkey = (l == 0) ? 0 : 'a' + l - 1;

            mkey = showkey;
            goto runthru1;
        }
        storage_num = ostorage_num = prefix_table[first_entry].storage_num;
    }
    else
    {
        /* runthru2 */
        getword(ostorage_num, &w_from_storage);
        if (w_from_storage.another_let != 0)    /* More than one possibility */
        {
            if (waskeypressed == 0)             /* not key pressed */
            {
                goto pause1;
            }
            do
            {
                if (w_from_storage.let == mkey)
                {
                    if (mkey == '\0')
                    {
                        goto done1;
                    }
                    ostorage_num = storage_num = w_from_storage.further_let;
                    break;
                }
                if (w_from_storage.another_let == 0)
                {
                    storage_num = ostorage_num;
                    goto pause1;
                }
                ostorage_num = w_from_storage.another_let;
                getword(ostorage_num, &w_from_storage);
            } while (1);
        }

      putin2:
        if (waskeypressed != 0)
        {
            stakpt++;
            st_inat[stakpt] = *inat;
            st_olist[stakpt] = ostorage_num;
            st_putcap[stakpt] = oputcap;
            oputcap = putcap;
            waskeypressed = 0;
        }
        wrd[chr_in_wrd] = mkey;
        chr_in_wrd++;
        in[*inat] = showkey;
        (*inat)++;

        printw("%c", showkey);
        if (w_from_storage.further_let == 0)    /* Not a longer word. */
        {
            waskeypressed = 0;                  /* not key pressed */
            goto done1;                         /* At end of word. */
        }
        ostorage_num = storage_num = w_from_storage.further_let;
        st_olist[stakpt] = ostorage_num;
    }

   /* auto2 */
    getword(storage_num, &w_from_storage);
    if (w_from_storage.another_let == 0)        /* If no other letters. */
    {
        ostorage_num = storage_num;
        st_olist[stakpt] = ostorage_num;
        mkey = showkey = w_from_storage.let;
        goto putin2;
    }
    goto pause1;

    /* Waiting for key while at end of word. */
    do
    {
	refresh();
        pressedkey = get_single_ch();
        waskeypressed = 1;
        if (pressedkey == '\t')                         /* tab key */
        {
            printw("\n");
            printw("At end of word\n");
            print_arrow();
            if (*inat > 0)
            {
                printw("%s", in);
            }
            continue;
        }
        if (pressedkey == '\b' || pressedkey == 0x7f)
        {
            goto erase1;
        }
        if ((pressedkey != '\n' && pressedkey != '\r' &&                        /* return or new-line */
             pressedkey != ' ' && pressedkey != ',' && pressedkey != '.') && /* space, comma, or period */
            pressedkey <= 0x1f)                                         /* ignore other function keys */
        {
            continue;
        }

        /* At end of word. */
      done1:
        if (pressedkey == '\n' || pressedkey == '\r' ||                 /* return or new-line */
            pressedkey == ' ' || pressedkey == ',' || pressedkey == '.')        /* space, comma, or period */
        {
            switch (pressedkey)
            {
                case ' ':
                    printw("%c", ' ');
                    in[*inat] = ' ';
                    (*inat)++;
                    goto start1;

                case ',':
                    printw("%c", ',');
                    in[*inat] = ',';
                    (*inat)++;
                    printw("%c", ' ');
                    in[*inat] = ' ';
                    (*inat)++;
                    goto start1;

                case '.':
                    printw("%c", '.');
                    in[*inat] = '.';
                    (*inat)++;
                    printw("%c", ' ');
                    in[*inat] = ' ';
                    (*inat)++;
                    putcap = 1;                 /* Put a shift in next first character! */
                    goto start1;

                case '\n':
                case '\r':
                    return;

                default: ;
            }
        }
    } while (1);
}                                       /* End of type */

/* ------------------------------------------------------------------------ */
static void typer(void)
{
    unsigned int    inat;
    char            in[INLENGTH];
    char            key;

    do
    {
	erase();
	refresh();
        printw("\n");
        printw("Type in words. This program will not let you type in misspelled words. This\n");
        printw("means it must know all words you could type...  unfortunately it doesn't know\n");
        printw("many suffices or plurals, so must make do with that. 'tab' for possibilities.\n");
        printw("\n");
        print_arrow();

        type(in, &inat);

        printw("  ok\n");
        printw("Input='%s'\n", in);
	refresh();

        key = get_single_ch();
        if (key == 'q')
        {
	    endwin();
	    restore_original_termios();
            exit(0);                 /* quit. */
        }
    } while (key == '\n' || key == '\r');
}                                       /* End of typer */

/* ------------------------------------------------------------------------ */
static void read_dataset(char *filename)
{
    FILE           *dataset_file;
    enum add_return add_ret;
    unsigned int    i;
    char            in[INLENGTH];
    unsigned int    count_added = 0;
    unsigned int    count_duplicated = 0;

    dataset_file = fopen(filename, "r");
    if (dataset_file == NULL)
    {
        exit_perror("fopen dataset file failure:");
    }

    global_do_not_write = 1;            /* Do not write prefix files till finished. */

    printw("Reading dataset '%s' ...\n", filename);

    while (1)
    {
        memset(&in[0], 0, INLENGTH);
        if (fgets(&in[0], INLENGTH, dataset_file) == NULL)
        {
            fclose(dataset_file);
            break;                      /* End Of File */
        }

        for (i = 0; i < INLENGTH && in[i] != '\0'; i++)
        {
            while (in[i] == ' ')        /* delete spaces */
            {
                memmove(&in[i], &in[i + 1], INLENGTH - i - 1);
            }
            if (in[i] == '\n')          /* delete end of line */
            {
                in[i] = '\0';
                break;
            }
        }                               /* End of for */

        if (in[0] == '0')
        {
            printw("Done adding words from dataset.\n");
            return;
        }

        add_ret = addword(in);
        cleanup();
        if (add_ret == AR_NO_INPUT)
        {
            printw("No input?\n");
        }
        else if (add_ret == AR_ADDED_WORD)
        {
            count_added++;
        }
        else
        {
            count_duplicated++;
        }
    }                                   /* End of while */
    printw("Added %d new word%c, tried to add %d duplicated word%c\n", 
            count_added, (count_added != 1)?'s':'\0',
            count_duplicated, (count_duplicated != 1)?'s':'\0');

    global_do_not_write = 0;

    write_pointer_file();
    write_prefix_table_file();
}                                       /* End of read_dataset */

/* ------------------------------------------------------------------------ */
NORETURN static void terminate(int code UNUSED)
{
    cleanup();
    endwin();
    restore_original_termios();
    exit(0);
}                                       /* End of terminate */

/* ------------------------------------------------------------------------ */
static void print_storage(char *in, unsigned int *inat, pft storage_num)
{
    struct worder   worder;
    unsigned int i;

    getword(storage_num, &worder);  /* Get block buffer filled. */
    if (worder.let == '\0')
    {
        if (worder.further_let != 0)
        {
//            printw(" -- ERROR let == '\\0' but further_let == %d\n", worder.further_let);
            printf(" -- ERROR let == '\\0' but further_let == %d\n", worder.further_let);
        }
        else
        {
            for (i = 0; i <= *inat; i++)
            {
//                printw("%c", in[i]);
                printf("%c", in[i]);
            }
//            printw("\n");
            printf("\n");
//            refresh();
        }
    }
    else
    {
        (*inat)++;
        in[*inat] = worder.let;
        print_storage(in, inat, worder.further_let);
        (*inat)--;
    }

    /* Now do the another_let pointer. */
    if (worder.another_let != 0)
    {
        print_storage(in, inat, worder.another_let);
    }
}                                       /* End of print_storage */

/* ------------------------------------------------------------------------ */
static void print_all(void)
{
    unsigned int    inat;
    pft             prefix_on;
    pft             storage_num;
    char            in[INLENGTH];

    erase();
    refresh();
    endwin();

    for (prefix_on = 0; prefix_on < SIZE_PREFIX_TABLE; prefix_on++)
    {
        if (prefix_table[prefix_on].storage_num == NOT_IN_PREFIX_TABLE)
        {
            continue;
        }
        memset(in, 0, INLENGTH);
        for (inat = 0; inat < LPREFIX; inat++)
        {
            unsigned int j;
            unsigned int l;

            if (inat == 0)
            {
                j = prefix_on/ARRAYLTH_PREFIX_VALUE;
                l = prefix_on - (j * ARRAYLTH_PREFIX_VALUE);
            }
            else
            {
                unsigned int k;
                /* 2nd and additional characters. */
                unsigned int p = 1;
                for (k = 0; k < inat; k++)
                {
                    p *= ARRAYLTH_PREFIX_VALUE;
                }
                k = prefix_on / p;
                j = k / ARRAYLTH_PREFIX_VALUE;
                l = k - (j * ARRAYLTH_PREFIX_VALUE);
            }
            in[LPREFIX - inat -1] = (l == 0) ? '\0' : 'a' + l - 1;
        }                               /* End of for */
        inat = LPREFIX - 1;             /* inat set to character last found. */
        if (in[inat] == '\0' || prefix_table[prefix_on].storage_num == ONLY_IN_PREFIX_TABLE)
        {
//            printw("%s\n", in);
//            refresh();
            printf("%s\n", in);
            continue;
        }
        storage_num = prefix_table[prefix_on].storage_num;
        print_storage(in, &inat, storage_num);
    }                                   /* End of for */

    (void)get_single_ch();

//    refresh();
    initscr();
    intrflush(stdscr, FALSE);
    scrollok(stdscr, TRUE);
    /* get the number of rows and columns */
    getmaxyx(stdscr,terminal_row,terminal_col);

}                                       /* End of print_all */

/* ------------------------------------------------------------------------ */
static void choose(void)
{
    char            key;

    do
    {
	erase();
	refresh();
        printw("Run program with file name argument to insert all words into the list.\n");
        printw("\n");
        printw("Choose:\n");
        printw("  a) type - only words in list are allowed - and unique parts autofill.\n");
        printw("  b) enter words one at a time to the list.\n");
        printw("  p) print all in dataset.\n");
        printw("  q) quit.\n");
	refresh();
        key = (char)tolower((int)get_single_ch());
        switch (key)
        {
            case 'a':
                typer();
                break;                  /* Done typing if here. */

            case 'b':
                enter();
                break;                  /* go do more. */

            case 'p':
                print_all();            /* Print out everything. */
                break;                  /* go do more. */

            case 'q':
                return;                 /* quit. */

            default:;
        }
    } while (1);
}                                       /* End of choose */

/* ------------------------------------------------------------------------ */
static void initget(void)
{
    unsigned int    work2;
    size_t          rl;

    /* Get memory for prefix table. */
    prefix_table = malloc(SIZE_PREFIX_TABLE * sizeof(*prefix_table));
    if (prefix_table == NULL)
    {
        exit_perror("malloc -- not enough memory for prefix_table:");
    }
    memset(prefix_table, 0, SIZE_PREFIX_TABLE * sizeof(*prefix_table));

    /* Read in prefix table. */
    prefix_table_file = fopen(PREFIX_TABLE_FILENAME, "rb+");
    if (prefix_table_file == NULL)
    {
        exit_perror("fopen of prefix_table_file failed:");
    }

    rl = fread(prefix_table, sizeof(*prefix_table), SIZE_PREFIX_TABLE, prefix_table_file);
    size_t s = SIZE_PREFIX_TABLE;
    if (rl > 0 && rl < s)               /* NOT initialized */
    {
        printw("prefix_table read (%ld) < supposed #%ld entries (meaning %ld)\n", rl,
                s, sizeof(*prefix_table) * s);
        exit_perror("fread prefix_table entries failed:");
    }
    printw("%ld prefix_table entries read\n", s);
    /*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    /* Read in pointer_file for 'storage' storage_used variable. */
    pointer_file = fopen(POINTER_FILENAME, "rb+");
    if (pointer_file == NULL)
    {
        exit_perror("fopen of pointer_file failed:");
    }
    rl = fread(&storage_used, sizeof(storage_used), 1, pointer_file);
    if (rl < 1)                         /* Error occurred */
    {
        storage_used = 0;               /* No 'storage' yet. */
        printw("number of storage entries initialized\n");
    }
    printw("storage_used = %d\n", storage_used);
    /*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
    /* Get memory for disk buffers saved in memory. */
    storage_buffer = (struct worder *)malloc(MEM_STORAGE);      /* Memory for buffers gotten. */
    if (storage_buffer == NULL)
    {
        exit_perror("malloc -- not enough memory for worder:");
    }

    storage_file = fopen(STORAGE_FILENAME, "rb+");
    if (storage_file == NULL)
    {
        exit_perror("fopen of storage_file failed:");
    }

    for (work2 = 0; work2 < G_NBLKS; work2++)
    {
        g_blk[work2] = -1;              /* No block buffer data in this area. */
        g_alter[work2] = BLK_UNCHANGED;
        g_lacc[work2] = time(0);
    }                                   /* End of for */
}                                       /* End of initget */

/* ------------------------------------------------------------------------ */
int main(int argc, char **argv)
{
    int i;

    SIZE_PREFIX_TABLE = 1;
    for (i = 0; i< LPREFIX; i++)
    {
        SIZE_PREFIX_TABLE *= ARRAYLTH_PREFIX_VALUE;
    }                                   /* End of for */

    /* Put into single character input mode. */
    get_original_termios();

    /* Initialize curses mode. */
    initscr();
    intrflush(stdscr, FALSE);
    scrollok(stdscr, TRUE);

    initget();

    /* get the number of rows and columns */
    getmaxyx(stdscr,terminal_row,terminal_col);

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    {
        (void)signal(SIGINT, terminate);
    }

    for (i = 1; i < argc; i++)
    {
        read_dataset(argv[i]);
    }                                   /* End of for */

    choose();

    endwin();
    restore_original_termios();
    exit(0);
}                                       /* End of main */

/* ------------------------------------------------------------------------ */
/*
 * vi: sw=4 ts=8 expandtab
 */
/* End of m4test.c */
