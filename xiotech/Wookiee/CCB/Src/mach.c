/* $Id: mach.c 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       mach.c
** MODULE TITLE:    Lattice Semiconductor 'Mach' CPLD
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "mach.h"

#include "ccb_hw.h"
#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "debug_files.h"

/*****************************************************************************
** Private variables
*****************************************************************************/
static MACH machDummy;          /* bss section, thus is zero for startup. */

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
MACH *const mach = (MACH *const)&machDummy;

/*****************************************************************************
** Routines not externed in any header file
*****************************************************************************/
#ifdef PAM
extern UINT32 PAMHeartbeatDisableSwitch(void);
#endif /* PAM */

/*****************************************************************************
** Code Start
*****************************************************************************/

/**********************************************************************
*
*  FUNCTION NAME: ParseCCBCfg
*
*  PARAMETERS:    pOption - the option string you want to search for
*
*  DESCRIPTION:   Searches the 'ccb.cfg' file for the specified option.
*
*  RETURNS:       Returns TRUE if option found, FALSE otherwise.
*
**********************************************************************/
static UINT32 ParseCCBCfg(const char *pOption)
{
    FILE       *pF;
    char        buf[256];       /* this should be more than we need */
    char       *pLine;
    UINT32      len;
    UINT32      foundIt = FALSE;

    if (pOption)
    {
        len = strlen(pOption);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "ParseCCBCfg: pOption == NULL\n");
        return FALSE;
    }

    /*
     * Read and parse /opt/xiotech/ccbdata/ccb.cfg.
     */
    pF = fopen("/opt/xiotech/ccbdata/ccb.cfg", "r");
    if (pF)
    {
        while (fgets(buf, 256, pF))
        {
            /*
             * Skip whitespace
             */
            pLine = buf;
            while (*pLine)
            {
                if (*pLine == ' ' || *pLine == '\t')
                {
                    pLine++;
                    continue;
                }

                break;
            }

            /*
             * See if pOption is listed
             */
            if (strncmp(pLine, pOption, len) == 0)
            {
                /*
                 * Found it!
                 */
                foundIt = TRUE;
                break;
            }
        }

        Fclose(pF);
    }

    dprintf(DPRINTF_DEFAULT, "%s == %s\n", pOption, foundIt ? "TRUE" : "FALSE");

    return foundIt;
}

UINT32 SuicideDisableSwitch(void)
{
    return ParseCCBCfg("SUICIDE_DISABLE");
}

UINT32 DiagPortsEnableSwitch(void)
{
    return ParseCCBCfg("DIAG_PORTS_ENABLE");
}

#ifdef PAM
UINT32 PAMHeartbeatDisableSwitch(void)
{
    return ParseCCBCfg("PAM_HEARTBEAT_DISABLE");
}
#endif /* PAM */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
