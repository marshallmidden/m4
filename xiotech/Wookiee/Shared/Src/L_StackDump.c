/* $Id: L_StackDump.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       L_StackDump.c
**
**  @brief      Provide stack contents for crash dump.
**
**  This routine provides the raw stack content for a crash dump.
**  Accessing the PC and SP at the time of the crash is not a documented
**  thing to do, so kernel revisions could invalidate this code.
**
**  Copyright (c) 2005-2008 Xiotech Corporation. All Rights reserved.
**
******************************************************************************
**/

#include "XIO_Std.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "L_StackDump.h"

// The following is set in the linker.
extern ulong __executable_start;

/**
******************************************************************************
** Code Start
******************************************************************************
**/

/**
******************************************************************************
**
**  @brief      count_records - Count records in file
**
**              Count the records in a file, returning the count.
**
**  @param      f - FILE pointer to file
**
**  @param      marker - Marker for the start of each record
**
**  @return     Number of Markers seen in the file
**
******************************************************************************
**/

static int count_records(FILE *f, const char marker[])
{
    int count = 0;
    int ch;
    int matchix;

    for (ch = fgetc(f); ch != EOF; ch = fgetc(f))
    {
        if (ch == EOF)
        {
            break;
        }
        matchix = 0;
        while (marker[matchix] && ch == marker[matchix++])
        {
            ch = fgetc(f);
        }
        if (marker[matchix] == 0)
        {
            ++count;
        }
        while (ch != EOF && ch != '\n')
        {
            ch = fgetc(f);
        }
    }
    return count;
}


/**
******************************************************************************
**
**  @brief      Recopy - recopy file, omitting first record.
**
**              Recopy a file, omitting the first record to make room
**              for the next record at the end.
**
**  @param      fn - Path to file to be recopied
**
**  @param      tfn - Path to temporary file to use.
**
**  @param      marker - Marker for the start of each record
**
**  @return     none
**
******************************************************************************
**/

static void Recopy(const char *fn, const char *tfn, const char marker[])
{
    FILE *cfp;
    FILE *tfp;
    int include_count = 0;
    int ch;
    int matchix;

    if (rename(fn, tfn))
    {
        fprintf(stderr, "Unable to rename %s to %s, errno=%d\n", fn, tfn, errno);
        exit(EXIT_FAILURE);
    }
    tfp = fopen(tfn, "r");
    if (!tfp)
    {
        fprintf(stderr, "Unable to open temp %s for reading, errno=%d\n", tfn, errno);
        exit(EXIT_FAILURE);
    }
    cfp = fopen(fn, "w");
    if (!cfp)
    {
        fprintf(stderr, "Unable to create file %s, errno=%d\n", fn, errno);
        exit(EXIT_FAILURE);
    }

    for (ch = fgetc(tfp); ch != EOF; ch = fgetc(tfp))
    {
        matchix = 0;
        while (marker[matchix] && ch == marker[matchix++])
        {
            ch = getc(tfp);
        }
        if (marker[matchix] == 0 && ++include_count == 2)
        {
            fputs(marker, cfp); /* Add marker for first record to copy */
            break;
        }
        while (ch != EOF && ch != '\n')
        {
            ch = getc(tfp);
        }
    }
    while (ch != EOF)
    {
        putc(ch, cfp);
        ch = getc(tfp);
    }
#ifdef CCB_RUNTIME_CODE
    Fclose(tfp);
    Fclose(cfp);
#else
    fclose(tfp);
    fclose(cfp);
#endif
    if (unlink(tfn))
    {
        fprintf(stderr, "Unable to unlink temp file %s, errno=%d\n", tfn, errno);
        exit(EXIT_FAILURE);
    }
}


/**
******************************************************************************
**
**  @brief      L_rotate - Rotate history and summary files.
**
**              Rotate history file, appending summary file to it.
**
**  @param      summary - path to summary file
**
**  @param      history - path to history file
**
**  @param      tempfn - path to temporary file
**
**  @param      marker - record marker string in history file
**
**  @return     none
**
******************************************************************************
**/

void L_rotate(const char *summary, const char *history,
    const char *tempfn, const char marker[])
{
    FILE *sf;
    FILE *hf;
    int history_count = 0;
    int ch;

    sf = fopen(summary, "r");
    if (!sf)
    {
        return;
    }

    hf = fopen(history, "r");
    if (!hf)
    {
        hf = fopen(history, "w");   /* Create history file */
        if (!hf)
        {
            fprintf(stderr, "Cannot create summary history %s, errno=%d\n",
                history, errno);
            exit(EXIT_FAILURE);
        }
        history_count = 1;
    }
    else
    {
        history_count = count_records(hf, LOG_PREPEND);
#ifdef CCB_RUNTIME_CODE
        Fclose(hf);
#else
        fclose(hf);
#endif
        if (history_count >= LOG_MAX_HISTORY_COUNT)
        {
            Recopy(history, tempfn, LOG_PREPEND);
        }
        hf = fopen(history, "a");
        if (!hf)
        {
            fprintf(stderr, "Cannot append summary history %s, errno=%d\n",
                history, errno);
            exit(EXIT_FAILURE);
        }
    }

    /*
     * At this point, sf is open for reading and hf
     * is ready to be appended to.
     */

    fputs(marker, hf);
    putc('\n', hf);
    for (ch = getc(sf); ch != EOF; ch = getc(sf))
    {
        putc(ch, hf);
    }
#ifdef CCB_RUNTIME_CODE
    Fclose(sf);
    Fclose(hf);
#else
    fclose(sf);
    fclose(hf);
#endif
}


/**
******************************************************************************
**
**  @brief      L_memdump - Dump a range of memory.
**
**              Dump a range of memory in hex, excluding zeros.
**
**  @param      *ofp - Pointer to FILE to get output
**
**  @param      @start - Beginning of range to dump
**
**  @param      @end - End of range to dump
**
**  @return     none
**
******************************************************************************
**/

void    L_memdump(FILE *ofp, void *start, void *end)
{
    unsigned long   *isp;
    unsigned long   *lsp;
    int     i;
    char    zsp = 0;

    // Make sure dumping something somewhat reasonable.
    if (start < (void *)&__executable_start || end < (void *)&__executable_start)
    {
        return;
    }
    isp = (unsigned long *)(((unsigned long)start + 3) & ~3);
    lsp = (unsigned long *)(((unsigned long)end + 3) & ~3);
    for (i = 0; i < 250 && isp <= lsp; ++i)
    {
        unsigned long   d1, d2, d3, d4;

        d1 = isp[0];
        d2 = isp[1];
        d3 = isp[2];
        d4 = isp[3];
        if (d1 == 0 && d2 == 0 && d3 == 0 && d4 == 0)
        {
            if (zsp == 0)
            {
                fprintf(ofp, "          <0..0>\n");
                zsp = 1;
            }
        }
        else
        {
            fprintf(ofp, "%08lx: %08lx %08lx %08lx %08lx\n",
                (unsigned long)isp, d1, d2, d3, d4);
            zsp = 0;
        }
        isp += 4;
    }
}


/**
******************************************************************************
**
**  @brief      L_StackDump - Dump raw stack.
**
**              Dump raw stack for crash dump.
**
**  @param      sdArgs - Pointer to Struct StackDumpArgs.
**
**  @return     none
**
******************************************************************************
**/

void L_StackDump(struct StackDumpArgs *sdArgs)
{
    unsigned long *fsp;
    unsigned long *lsp;
    unsigned long *isp;
    unsigned long *pc;
    FILE    *sfp;
    UINT32  ebp;
    int i;
    int k;

    sfp = sdArgs->sfp;
    ebp = sdArgs->ebp;
    pc = sdArgs->pc;
    fsp = sdArgs->fsp;
    isp = fsp;
    lsp = fsp;
    fprintf(sfp, "---status ebp=0x%08x, sp=0x%08x, pc=0x%08x, si_addr=0x%08x\n",
        ebp, (UINT32)isp, (UINT32)pc, (UINT32)(sdArgs->saddr));
    isp = (unsigned long *)ebp;
    k = 0;
    for (i = 0; ((UINT32)isp > 0x08048000) && i < 64; ++i)
    {
        if (isp)
        {
            lsp = isp;
        }
        if (isp && isp[0] && (isp[0] <= (unsigned long)isp ||
            ((unsigned long)isp & 3) != 0 ||
            ((unsigned long)isp[0] & 3) != 0))
        {
            fprintf(sfp, "bad stack, isp=0x%08x, isp[0]=0x%08x\n",
                (UINT32)isp, (UINT32)isp[0]);
            break;
        }
        k++;
        if ((UINT32)isp > 0x08048000)
        {
            UINT32 routine;
            routine = *(int*)(((char*)isp)+4);
            fprintf(sfp, " c_frame %d ebp=0x%08x  routine @ 0x%08x\n", i+1, (UINT32)isp, routine);
        }
        isp = (unsigned long *) isp[0];
    }
    fprintf(sfp, "---stack from %08x to %08x\n", (UINT32)fsp, (UINT32)lsp);
    L_memdump(sfp, fsp, lsp);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
