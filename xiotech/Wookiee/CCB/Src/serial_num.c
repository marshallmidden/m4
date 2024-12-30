/* $Id: serial_num.c 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       serial_num.c
** MODULE TITLE:    Get & Set the system serial number.
**
** DESCRIPTION:     Routines to Get & Set the system serial number.
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "serial_num.h"

#include "ccb_flash.h"
#include "LOG_Defs.h"
#include "kernel.h"

#include "MR_Defs.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "sm.h"
#include "XIO_Std.h"
#include "nvram.h"

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static UINT16 AssignSystemInfo(MRASSIGNSYSINFO_REQ *pAssignSysInfo,
                               MRASSIGNSYSINFO_RSP *pAssignSysInfoOut);

/*****************************************************************************
** Code Start
*****************************************************************************/

/**********************************************************************
*                                                                     *
*  Name:        GetSerialNumber()                                     *
*                                                                     *
*  Description: Gets the Thunderbolt system and controller serial     *
*               numbers and caches them locally.                      *
*                                                                     *
*  Input:       int which - system or controller serial num.          *
*                                                                     *
*  Returns:     UINT32 - the requested serial num. 0 if failure.      *
*                                                                     *
**********************************************************************/
UINT32 GetSerialNumber(int which)
{
    UINT32      sn = 0;

    switch (which)
    {
        case CONTROLLER_SN:
            sn = CntlSetup_GetControllerSN();
            break;

        case SYSTEM_SN:
            sn = CntlSetup_GetSystemSN();
            break;
    }

    return sn;
}

/**********************************************************************
*                                                                     *
*  Name:        GetProcSerialNumbers()                                *
*                                                                     *
*  Description: Gets the system and controller serial numbers from    *
*               the PROC.                                             *
*                                                                     *
*  Input:       UINT32* pSystemSN - Pointer to memory to put          *
*                                   the system serial number.         *
*                                                                     *
*               UINT32* pControllerSN - Pointer to memory to put      *
*                                       the controller serial         *
*                                       number.                       *
*                                                                     *
*  Returns:     GOOD or ERROR                                         *
*                                                                     *
**********************************************************************/
UINT16 GetProcSerialNumbers(UINT32 *pSystemSN, UINT32 *pControllerSN)
{
    MRASSIGNSYSINFO_REQ assignSysInfo;
    MRASSIGNSYSINFO_RSP assignSysInfoOut;
    UINT16      rc = PI_GOOD;

    *pSystemSN = 0;
    *pControllerSN = 0;

    memset(&assignSysInfo, 0x00, sizeof(assignSysInfo));
    memset(&assignSysInfoOut, 0x00, sizeof(assignSysInfoOut));

    assignSysInfo.op = ASSIGNSYSINFO_POLL;

    rc = AssignSystemInfo(&assignSysInfo, &assignSysInfoOut);

    /* At this point, we should have received the return packet. */
    if (rc == PI_GOOD)
    {
        *pSystemSN = assignSysInfoOut.sserial;
        *pControllerSN = assignSysInfoOut.cserial;
    }

    return rc;
}

/**********************************************************************
*                                                                     *
*  Name:        UpdateProcSerialNumber()                              *
*                                                                     *
*  Description: Updates the given serial number in the PROC.          *
*                                                                     *
*  Input:       UINT8 which - which serial number to change           *
*               UINT32 serNum - the system serial number              *
*                                                                     *
*  Returns:     GOOD or ERROR                                         *
*                                                                     *
**********************************************************************/
UINT16 UpdateProcSerialNumber(UINT8 which, UINT32 serNum)
{
    MRASSIGNSYSINFO_REQ assignSysInfo;
    MRASSIGNSYSINFO_RSP assignSysInfoOut;
    UINT16      rc = PI_GOOD;

    memset(&assignSysInfo, 0x00, sizeof(assignSysInfo));
    memset(&assignSysInfoOut, 0x00, sizeof(assignSysInfoOut));

    if (which == CONTROLLER_SN)
    {
        assignSysInfo.op = ASSIGNSYSINFO_CNTL_SN;
    }
    else if (which == SYSTEM_SN)
    {
        assignSysInfo.op = ASSIGNSYSINFO_SERIAL;
    }

    assignSysInfo.sserial = serNum;

    rc = AssignSystemInfo(&assignSysInfo, &assignSysInfoOut);

    return rc;
}

/**********************************************************************
*                                                                     *
*  Name:        SetIPAddress()                                        *
*                                                                     *
*  Description: Sets the Thunderbolt IP Address                       *
*                                                                     *
*  Input:       UINT32 ipaddr - the IP Address                        *
*                                                                     *
*  Returns:     0 on success. !0 on failure.                          *
*                                                                     *
**********************************************************************/
UINT32 InitIPAddress(UINT32 ipaddr)
{
    MRASSIGNSYSINFO_REQ assignSysInfo;
    MRASSIGNSYSINFO_RSP assignSysInfoOut;
    UINT16      rc = PI_GOOD;

    memset(&assignSysInfo, 0x00, sizeof(assignSysInfo));
    memset(&assignSysInfoOut, 0x00, sizeof(assignSysInfoOut));

    assignSysInfo.op = ASSIGNSYSINFO_IPADDR;
    assignSysInfo.ipaddr = ipaddr;

    rc = AssignSystemInfo(&assignSysInfo, &assignSysInfoOut);

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a method of initializing the cached value for
**              this controller's mirror partner.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void InitCachedMirrorPartnerSN(void)
{
    MRASSIGNSYSINFO_REQ assignSysInfo;
    MRASSIGNSYSINFO_RSP assignSysInfoOut;
    UINT16      rc = PI_GOOD;

    memset(&assignSysInfo, 0x00, sizeof(assignSysInfo));
    memset(&assignSysInfoOut, 0x00, sizeof(assignSysInfoOut));

    assignSysInfo.op = ASSIGNSYSINFO_POLL;

    rc = AssignSystemInfo(&assignSysInfo, &assignSysInfoOut);

    /* At this point, we should have received the return packet. */
    if (rc == PI_GOOD)
    {
        gCurrentMirrorPartnerSN = assignSysInfoOut.mp;
    }
}


/**
******************************************************************************
**
**  @brief      To provide a method of retrieving the cached mirror partner
**              for this controller.
**
**  @param      none
**
**  @return     UINT32 - Serial number of this controller's mirror partner.
**
******************************************************************************
**/
UINT32 GetCachedMirrorPartnerSN(void)
{
    return gCurrentMirrorPartnerSN;
}


/*----------------------------------------------------------------------------
** Function:    AssignSystemInfo
**
** Description: Executes the assign system information MRP with the given
**              input and output packets.
**
** Inputs:      pAssignSysInfo - Input packet to use in the assign system
**                               information MRP.
**              pAssignSysInfoOut - Output packet to use in the assign system
**                                  information MRP.
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static UINT16 AssignSystemInfo(MRASSIGNSYSINFO_REQ *pAssignSysInfo,
                               MRASSIGNSYSINFO_RSP *pAssignSysInfoOut)
{
    MRASSIGNSYSINFO_REQ *ptrInPkt = NULL;
    MRASSIGNSYSINFO_RSP *ptrOutPkt = NULL;
    UINT16      rc = PI_ERROR;

    /* Allocate memory for MRP input & output packets. */
    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /* Copy the input packet information from the input packet */
    memcpy(ptrInPkt, pAssignSysInfo, sizeof(*ptrInPkt));

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRASSIGNSYSINFO,
                    ptrOutPkt, sizeof(*ptrOutPkt), MRP_STD_TIMEOUT);

    /* At this point, we should have received the return packet. */
    if (rc == PI_GOOD)
    {
        /* Copy the output packet information to the output packet */
        memcpy(pAssignSysInfoOut, ptrOutPkt, sizeof(*pAssignSysInfoOut));
    }

    /* Free the input packet */
    Free(ptrInPkt);

    /*
     * Only free the output packet if the request did NOT timeout.
     * On a timeout the memory must remain available in case the
     * request eventually completes.
     */
    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    return (rc);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
