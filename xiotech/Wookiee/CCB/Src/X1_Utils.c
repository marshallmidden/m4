/* $Id: X1_Utils.c 143845 2010-07-07 20:51:58Z mdr $ */
/*===========================================================================
** FILE NAME:       X1_Utils.c
** MODULE TITLE:    X1 Packet Utilities
**
** DESCRIPTION:     Utility functions for X1 request packets.
**
** Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/

#include "CacheManager.h"
#include "CachePDisk.h"
#include "CmdLayers.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "HWM.h"
#include "LargeArrays.h"
#include "quorum_utils.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "X1_Packets.h"
#include "X1_Structs.h"
#include "X1_Utils.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define NUM_FW_TYPE     8       /* # of firmware type in fwTypeList[] below. */

/*****************************************************************************
** Private variables
*****************************************************************************/

/*
** *** NOTE *** The order of firmware type in this array must match the
** definition of X1_FWVERSIONS in X1_Structs.h.
** For Hypernode, everything unused is set to an equivalent Runtime.
*/
static UINT16 fwTypeList[] = {
    FW_HDR_TYPE_BE_RUNTIME,
    FW_HDR_TYPE_BE_RUNTIME,
    FW_HDR_TYPE_BE_RUNTIME,
    FW_HDR_TYPE_FE_RUNTIME,
    FW_HDR_TYPE_FE_RUNTIME,
    FW_HDR_TYPE_FE_RUNTIME,
    FW_HDR_TYPE_CCB_RUNTIME,
    FW_HDR_TYPE_CCB_RUNTIME
};

static bool bFWVersionsValid = false;
static X1_FWVERSIONS gFWVersions;

/*****************************************************************************
** Code Start
*****************************************************************************/


/*----------------------------------------------------------------------------
** Function:    GetFirmwareVersions
**
** Description: Get the firmware versions for the system
**
** Inputs:      X1_FWVERSIONS *     pointer to X1 firmware versions struct.
**                                  Memory is allocated by the caller.
**
** Outputs:     X1_FWVERSIONS *     Firmware version info copied into caller's
**                                  buffer.
**
**--------------------------------------------------------------------------*/
void GetFirmwareVersions(X1_FWVERSIONS *fwv)
{
    MR_FW_HEADER_RSP *pFWVersion = NULL;
    UINT32      systemRelease = 0;
    INT32       revision;
    char        revStr[5];
    UINT16      i;

    if (bFWVersionsValid)
    {
        memcpy(fwv, &gFWVersions, sizeof(X1_FWVERSIONS));
    }
    else
    {
        /*
         * Allocate memory for the firmware version info returned from
         * a packet request.
         */
        pFWVersion = MallocWC(sizeof(MR_FW_HEADER_RSP));

        /*
         * Get version information for each firmware type available
         * through the packet interface.  The first firmware type is
         * the System Release.  This is filled in after the rest of the
         * firmware versions are obtained.
         */
        for (i = 0; i < NUM_FW_TYPE; i++)
        {
            GetFirmwareHeader(fwTypeList[i], pFWVersion);

            /*
             * To return the revision in Mag format convert the
             * ASCII characters into hex.  If the conversion fails
             * revision will still contain the original ASCII and
             * this will be returned.
             */
            revision = pFWVersion->fw.revision;
            memcpy(revStr, &pFWVersion->fw.revision, sizeof(pFWVersion->fw.revision));

            revStr[4] = 0;      /* Terminate the string so atoh() behaves */
            atoh(revStr, &revision);

            /*
             * The X1_FWVERSIONS struct contains multiple X1_VERSION_ENTRY
             * structs.  The array notation is a little trick to get the
             * various version entries in a loop.  This assumes that
             * the order of the versions in X1_FWVERSIONS is the same order
             * we are retrieving them here.  The first X1_VERSION_ENTRY
             * is for the System Release version which is handled later.
             *
             */
            ((X1_VERSION_ENTRY *)fwv)[i + X1_VERSION_BE_BOOT].verMajMin = revision;
            ((X1_VERSION_ENTRY *)fwv)[i + X1_VERSION_BE_BOOT].fwCompatIndex = pFWVersion->fw.fwCompatIndex;
            ((X1_VERSION_ENTRY *)fwv)[i + X1_VERSION_BE_BOOT].fwBackLevelIndex = pFWVersion->fw.fwBackLevelIndex;
            ((X1_VERSION_ENTRY *)fwv)[i + X1_VERSION_BE_BOOT].fwSequencingIndex = pFWVersion->fw.fwSequencingIndex;

            /*
             * Copy the ASCII revision value into the tag field.
             */
            strncpy(((X1_VERSION_ENTRY *)fwv)[i + X1_VERSION_BE_BOOT].tag,
                    (char *)&pFWVersion->fw.revision, sizeof(pFWVersion->fw.revision));

            /*
             * If systemRelease is still at the initialized value, set it
             * from the firmware header data.
             */
            if (systemRelease == 0)
            {
                systemRelease = pFWVersion->fw.systemRelease;
            }

            /*
             * systemRelease must match for all firmware components in
             * the system.  If it does not the systemRelease level
             * is returned as "Engr" (Engineering level).
             */
            if (systemRelease != pFWVersion->fw.systemRelease)
            {
                systemRelease = SYSTEM_RELEASE_ENGR;
            }
        }

        Free(pFWVersion);

        /*
         * Fill in the System Release firmware info after determining the
         * levels of all other firmware in the system.
         * Copy the ASCII systemRelease value into the tag field of the
         * response packet.
         */
        strncpy(((X1_VERSION_ENTRY *)fwv)[X1_VERSION_SYSTEM].tag,
                (char *)&systemRelease, sizeof(systemRelease));

        /*
         * The Major / Minor revision information is extracted from the
         * CCB header. This is plugged into the System Release structure.
         */
        ((X1_VERSION_ENTRY *)fwv)[X1_VERSION_SYSTEM].verMajMin = *(UINT32 *)&(((FW_HEADER *)CCBRuntimeFWHAddr)->fwMajorRel);

        /*
         * Check if we have valid firmware version information.
         */
        if (fwv->system.verMajMin != 0 &&
            fwv->beBoot.verMajMin != 0 &&
            fwv->beDiag.verMajMin != 0 &&
            fwv->beRun.verMajMin != 0 &&
            fwv->feBoot.verMajMin != 0 &&
            fwv->feDiag.verMajMin != 0 &&
            fwv->feRun.verMajMin != 0 &&
            fwv->ccbBoot.verMajMin != 0 &&
            fwv->ccbRun.verMajMin != 0)
        {
            /*
             * Copy the firmware version information into the global copy.
             */
            memcpy(&gFWVersions, fwv, sizeof(X1_FWVERSIONS));

            /*
             * The global copy of the firmware information is now valid.
             */
            bFWVersionsValid = true;
        }
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
