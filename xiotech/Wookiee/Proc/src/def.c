/* $Id: def.c 144409 2010-07-20 21:31:17Z m4 $ */
/**
******************************************************************************
**
**  @file       def.c
**
**  @brief      Define C functions
**
**  This module provides a single code location for code used on the
**  FE and BE.
**
**  Copyright (c) 2002-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/

#include "XIO_Types.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "system.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "def.h"
#include "misc.h"
#include "pcb.h"
#include "sdd.h"
#include "target.h"

#ifdef BACKEND
#include "defbe.h"
#else
#include "deffe.h"
#endif

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      To provide a standard means of looking up a WWN to determine
**              if the SID is registered.
**
**              This function will look through the SDX table for a WWN and
**              target ID match.
**
**  @param      wwn         - World Wide Name for server
**  @param      tid         - Target ID
**  @param      newServer   - 0 = only return SID for servers that are managed
**                            1 = return SID for any server that is found
**
**  @return     SID (0xFFFFFFFF if not found)
**
******************************************************************************
**/
UINT32 DEF_WWNLookup(UINT64 wwn, UINT16 tid, UINT32 newServer, UINT8 *i_name)
{
    UINT32 rc;
    UINT32 sid;
    SDD   *sdd;

    /*
     * Check for a valid target ID.  Control port have a target ID
     * of 0xFFFF.  Server are not assigned to control ports.
     */
    if (tid < MAX_TARGETS && T_tgdindx[tid] != NULL)
    {
        /* Check if this is a XIOtech Controller */
        rc = M_chk4XIO(wwn);

        /*
         * If the WWN indicates a Bigfoot controller, ignore the
         * nibble of the WWN where the port number is stored and
         * and nibble where the controller ID is store (the least
         * significant digit of the serial number).
         */
        if (rc != 0)
        {
            wwn &= 0xF0FFFFFFFFFFF0FFLL;
        }

        /*
         * Check all server SDD records, searching for a WWN and
         * target ID match
         */
        for (sid = 0; sid < MAX_SERVERS; ++sid)
        {
            sdd = S_sddindx[sid];

            if (sdd != NULL)
            {
                /* Check for matching WWN and target ID. */
#if ISCSI_CODE
                if ((sdd->tid == tid)
                            && ((!BIT_TEST(T_tgdindx[tid]->opt, TARGET_ISCSI)
                            && (sdd->wwn == wwn))
                            || (BIT_TEST(T_tgdindx[tid]->opt, TARGET_ISCSI)
                            && (i_name != NULL)
                            && (strncmp((char *)i_name, (char *)sdd->i_name, 254) == 0))))
#else
                if (sdd->wwn == wwn && sdd->tid == tid)
#endif
                {
                    /* Check if this is a new (unmanaged) server. */
                    if (newServer != FALSE &&
                        (sdd->attrib & (1<<SD_UNMANAGED)) != 0)
                    {
                        /* Ignore server with the new bit set. */
                        return(0xFFFFFFFF);
                    }
                    break;
                }
            }
        }

        /* Was a Server SDD record found? */
        if (sid >= MAX_SERVERS)
        {
            /* Indicate server was not found. */
            return(0xFFFFFFFF);
        }
    }
    else
    {
        /* Indicate server was not found. */
        return(0xFFFFFFFF);
    }

    return sid;
}   /* End of DEF_WWNLookup */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
