/* $Id: mode.c 157452 2011-08-03 13:00:14Z m4 $ */
/*============================================================================
** FILE NAME:       mode.c
** MODULE TITLE:    Mode control
**
** DESCRIPTION:
**      Implementation of mode settable bits which control features within
**      the ccb and the proc. Functions to provide access and control
**      of these mode settable features.
**
** Copyright (c) 2002-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "mode.h"

#include "debug_files.h"
#include "LOG_Defs.h"
#include "mach.h"

#include "MR_Defs.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "XIO_Std.h"
#include "XIO_Const.h"

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
MODEDATA    modeData =
{
    {                               /* MODEDATA_CCB ccb */
        0,                          /* bits             */
        INITIAL_DPRINTF_MODE_BITS,  /* bitsDPrintf      */
        0,                          /* rsvd1            */
        0                           /* rsvd2            */
    },
    {                               /* MODEDATA_PROC proc */
        {0, 0, 0, 0}                /* word               */
    }
};

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 ProcModeSet(MODEDATA *modeDataPtr, MODEDATA *modeMaskPtr, UINT8 proc);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    InitModeData
**
** Description: Initialize the mode data information.
**
** Inputs:      NONE
**
** Returns:     NONE
**
**--------------------------------------------------------------------------*/
void InitModeData(void)
{
    memset(&modeData, 0x00, sizeof(modeData));

    /*
     * Set the default state of the mode data
     */
    if (SuicideDisableSwitch())
    {
        SetModeBit(MD_CONTROLLER_SUICIDE_DISABLE | MD_FM_RESTART_DISABLE);
    }

    if (DiagPortsEnableSwitch())
    {
        SetModeBit(MD_DIAG_PORTS_ENABLE);
    }

    modeData.ccb.bitsDPrintf = INITIAL_DPRINTF_MODE_BITS;
}


/*----------------------------------------------------------------------------
** Function:    ModeGet
**
** Description: Get the mode parameters
**
** Inputs:      modeDataPtr - pointer to store mode data
**
** Returns:     0 = Good
**              !0 = Error
**
**--------------------------------------------------------------------------*/
INT32 ModeGet(MODEDATA *modeDataPtr)
{
    /*
     * Copy out the latest mode data
     */
    *modeDataPtr = modeData;

    return PI_GOOD;
}


/*----------------------------------------------------------------------------
** Function:    ModeSet
**
** Description: Get the mode parameters
**
** Inputs:      modeDataPtr - pointer to  mode data
**              modeMaskPtr - pointer to mode mask
**
** Returns:     0 = Good
**              !0 = Error
**
**--------------------------------------------------------------------------*/
INT32 ModeSet(MODEDATA *modeDataPtr, MODEDATA *modeMaskPtr)
{
    UINT32      changedBits;
    INT32       rc = PI_GOOD;
    UINT32      i;

    /*
     * Examine mode bits that require a specific action when their state
     * changes and invoke those actions here.
     */
    changedBits = (modeBits & modeMaskPtr->ccb.bits) ^ (modeMaskPtr->ccb.bits & modeDataPtr->ccb.bits);


#if 0                           /* Sample Code */
    if (changedBits & MD_IPC_HEARTBEAT_DISABLE)
    {
        /*
         * If the bit was set, it is now cleared. Otherwise it is now
         * set (perform required action).
         */
        if (TestModeBit(MD_IPC_HEARTBEAT_DISABLE))
        {
            /* perform cleared action */
        }
        else
        {
            /* perform set action    */
        }
    }
#endif  /* 0 */


    /*
     * Clear all bits associated with the passed mask and then set any
     * requested bits.
     */

    modeBits &= ~modeMaskPtr->ccb.bits;
    modeBits |= (modeMaskPtr->ccb.bits & modeDataPtr->ccb.bits);

    modeData.ccb.bitsDPrintf &= ~modeMaskPtr->ccb.bitsDPrintf;
    modeData.ccb.bitsDPrintf |= (modeMaskPtr->ccb.bitsDPrintf &
                                 modeDataPtr->ccb.bitsDPrintf);

    modeData.ccb.rsvd1 &= ~modeMaskPtr->ccb.rsvd1;
    modeData.ccb.rsvd1 |= (modeMaskPtr->ccb.rsvd1 & modeDataPtr->ccb.rsvd1);

    modeData.ccb.rsvd2 &= ~modeMaskPtr->ccb.rsvd2;
    modeData.ccb.rsvd2 |= (modeMaskPtr->ccb.rsvd2 & modeDataPtr->ccb.rsvd2);

    /*
     * Send the Proc mode data and mask over to the proc through an MRP to
     * both the front end and the backend.
     */
    rc = ProcModeSet(modeDataPtr, modeMaskPtr, PROCESS_BE);

    if (rc == PI_GOOD)
    {
        rc = ProcModeSet(modeDataPtr, modeMaskPtr, PROCESS_FE);
    }

    /*
     * If the proc commands were successful, update the Proc mode data
     */
    if (rc == PI_GOOD)
    {
        for (i = 0; i < sizeof(modeData.proc) / sizeof(UINT32); ++i)
        {
            modeData.proc.word[i] &= ~modeMaskPtr->proc.word[i];
            modeData.proc.word[i] |= modeMaskPtr->proc.word[i] & modeDataPtr->proc.word[i];
        }
    }

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    ProcModeSet
**
** Description: Get the mode parameters
**
** Inputs:      modeDataPtr - pointer to  mode data
**              modeMaskPtr - pointer to mode mask
**
** Returns:     0 = Good
**              !0 = Error
**
**--------------------------------------------------------------------------*/
static INT32 ProcModeSet(MODEDATA *modeDataPtr, MODEDATA *modeMaskPtr, UINT8 proc)
{
    INT32       rc = PI_GOOD;
    MR_MODE_SET_REQ *ptrInPkt = NULL;
    MR_GENERIC_RSP *ptrOutPkt = NULL;
    UINT32      commandCode;

    /* Set up the Command code based on the requested processor */
    if (proc == PROCESS_FE)
    {
        commandCode = MRFEMODEPAGE;
    }
    else
    {
        commandCode = MRBEMODEPAGE;
    }

    /* Allocate memory for the MRP input and output packets. */
    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    if ((ptrInPkt != NULL) && (ptrOutPkt != NULL))
    {
        /* Copy in the mode data and the mask bits */
        memcpy(ptrInPkt->modeData, &modeDataPtr->proc, sizeof(ptrInPkt->modeData));
        memcpy(ptrInPkt->modeMask, &modeMaskPtr->proc, sizeof(ptrInPkt->modeMask));

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
        rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), commandCode,
                        ptrOutPkt, sizeof(*ptrOutPkt), GetGlobalMRPTimeout());
    }
    else
    {
        rc = PI_MALLOC_ERROR;
    }

    /* Free memory before returning */
    if (ptrInPkt != NULL)
    {
        Free(ptrInPkt);
    }

    if ((ptrOutPkt != NULL) && (rc != PI_TIMEOUT))
    {
        Free(ptrOutPkt);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    SetProcSuicide
**
** Description: Set the PROC suicide settings to match the CCB.
**
** Inputs:      NONE
**
** Returns:     NONE
**
**--------------------------------------------------------------------------*/
void SetProcSuicide(void)
{
    MODEDATA    mData;

    if (SuicideDisableSwitch())
    {
        memcpy(&mData, &modeData, sizeof(mData));
        mData.proc.word[0] = 1;
        ModeSet(&mData, &mData);
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
