/* $Id: MM_mgr.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       MM_mgr.c
**
**  @brief      Memory management
**
**  Memory management functions etc.
**
**  Copyright (c) 2004-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "debug_files.h"
#include "XIO_Const.h"
#include "error_handler.h"
#include "misc.h"

#include <stdio.h>
#include <string.h>

#include "heap.h"
#include "memory.h"

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/

/* NOTE: these are in the bss section, and thus zero upon startup. */
extern struct fmm P_ram;            /* Free memory management structure         */
struct fmm K_ncdram;         /* Free shared memory management structure  */
extern void MM_PrintStats(void);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Print the heap control block statistics
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void MM_PrintStats(void)
{
    char        buf[1024];
    char       *pBuf = buf;

    pBuf += sprintf(pBuf, "\nP_ram:\n");
    pBuf += sprintf(pBuf, "  fmm_first.thd                  = %p\n", P_ram.fmm_first.thd);
    pBuf += sprintf(pBuf, "  fmm_first.len                  = %u bytes\n", P_ram.fmm_first.len);
    pBuf += sprintf(pBuf, "  fmm_fms->fms_Available_memory  = %u bytes\n", P_ram.fmm_fms->fms_Available_memory);
    pBuf += sprintf(pBuf, "  fmm_fms->fms_Maximum_available = %u bytes\n", P_ram.fmm_fms->fms_Maximum_available);
    pBuf += sprintf(pBuf, "  fmm_fms->fms_Minimum_available = %u bytes\n", P_ram.fmm_fms->fms_Minimum_available);
    pBuf += sprintf(pBuf, "\n");
    dprintf(DPRINTF_DEFAULT, "%s", buf);

    pBuf += sprintf(pBuf, "\nK_ncdram:\n");
    pBuf += sprintf(pBuf, "  fmm_first.thd                  = %p\n", K_ncdram.fmm_first.thd);
    pBuf += sprintf(pBuf, "  fmm_first.len                  = %u bytes\n", K_ncdram.fmm_first.len);
    pBuf += sprintf(pBuf, "  fmm_fms->fms_Available_memory  = %u bytes\n", K_ncdram.fmm_fms->fms_Available_memory);
    pBuf += sprintf(pBuf, "  fmm_fms->fms_Maximum_available = %u bytes\n", K_ncdram.fmm_fms->fms_Maximum_available);
    pBuf += sprintf(pBuf, "  fmm_fms->fms_Minimum_available = %u bytes\n", K_ncdram.fmm_fms->fms_Minimum_available);
    pBuf += sprintf(pBuf, "\n");
    dprintf(DPRINTF_DEFAULT, "%s", buf);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
