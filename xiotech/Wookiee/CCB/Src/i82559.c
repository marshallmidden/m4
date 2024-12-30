/* $Id: i82559.c 143766 2010-07-06 12:06:32Z m4 $ */
/*============================================================================
** FILE NAME:       i82559.c
** MODULE TITLE:    Intel 82559 Ethernet
**
** DESCRIPTION:     To provide configuration and other miscellaneous
**                  hardware-related services for the Intel 82559 Ethernet
**                  chip.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "i82559.h"

#include "AsyncEventHandler.h"
#include "ccb_statistics.h"
#include "convert.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "i82559_Strings.h"
#include "idr_structure.h"
#include "ipc_heartbeat.h"
#include "ipc_packets.h"
#include "LargeArrays.h"
#include "led.h"
#include "logging.h"
#include "mach.h"
#include "misc.h"
#include "mode.h"
#include "mutex.h"
#include "quorum_utils.h"
#include "timer.h"
#include "trace.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

#include "PortServerUtils.h"
#include <errno.h>
#include <net/if.h>

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/

/*
** In HN, any function that cares about which interface we are using
** should use 'ethernetDriver.interfaceHandle' as defined below. That
** way, if the interface changes, we can update the ETHERNET_DRIVER
** structure with the new handle.
*/
static const char intf[] = "eth0";      /* default */

ETHERNET_DRIVER ethernetDriver =
{
    NULL,           /* *pcbPtr */
    (void *)intf,   /* interfaceHandle */
};

UINT32 stackDmpFull = FALSE;
char stackDmp[4096];

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: EthernetLogMessage
**
**  Description:    Generates and sends an alert message.  Creates an MRP
**                  type structure and sends it out.  This function currently
**                  only handles 1 parameter in the MRP.
**
**  Inputs:         messageType - type of alert
**                  length      - length of parameter data
**                  parm1       - data in first parameter
**
**  Returns:        Nothing
**--------------------------------------------------------------------------*/
static void EthernetLogMessage(INT32 messageType, INT32 length, UINT32 parm1)
{
    SendAsyncEvent(messageType, length, &parm1);
}


/*----------------------------------------------------------------------------
** FUNCTION NAME:   EthernetGetStatistics
**
** INPUTS:          ethernetStatisticsPtr
**                  resetCountersFlag: TRUE = clear internal counters after fetch
**
** RETURNS:         GOOD  -
**                  ERROR -
**--------------------------------------------------------------------------*/
UINT32 EthernetGetStatistics(ETHERNET_STATISTIC_COUNTERS *ethernetStatisticsPtr, UINT32 resetCountersFlag UNUSED)
{
    UINT32      returnCode = GOOD;

    FILE       *pF;
    char        pLine[256];     /* this should be more than we need */
    ETHERNET_STATISTIC_COUNTERS ethCnts;
    INT32       rc = 0;
    char       *pOffset;

    memset(&ethCnts, 0, sizeof(ethCnts));
    /*
     * Read and parse /proc/net/dev to get the ethernet statistics
     */
    pF = fopen("/proc/net/dev", "r");
    if (pF)
    {
        while (fgets(pLine, 256, pF))
        {
            pOffset = index(pLine, ':');
            if (pOffset)
            {
                *pOffset = ' ';
            }
            rc = sscanf(pLine, "%6s %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
                        ethCnts.interface,
                        &ethCnts.rxBytes,
                        &ethCnts.rxPackets,
                        &ethCnts.rxErrs,
                        &ethCnts.rxDrop,
                        &ethCnts.rxFifo,
                        &ethCnts.rxFrame,
                        &ethCnts.rxCompressed,
                        &ethCnts.rxMulticast,
                        &ethCnts.txBytes,
                        &ethCnts.txPackets,
                        &ethCnts.txErrs,
                        &ethCnts.txDrop,
                        &ethCnts.txFifo,
                        &ethCnts.txColls, &ethCnts.txCarrier, &ethCnts.txCompressed);

            if ((rc == 17) &&
                (strcmp(ethCnts.interface, ethernetDriver.interfaceHandle) == 0))
            {
                /*
                 * Indicate that we found the interface we were looking for
                 */
                rc = 99;
                break;
            }
        }

        Fclose(pF);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "EthernetGetStatistics: couldn't open /proc/net/dev, errno %d\n",
                errno);
    }

    if (ethernetStatisticsPtr != NULL)
    {
        if (rc == 99)
        {
            /*
             * Copy in the eyecatcher -- leave room for a NUL terminator
             */
            strncpy(ethCnts.eyecatcher, "ETHST01", sizeof(ethCnts.eyecatcher) - 1);
            *ethernetStatisticsPtr = ethCnts;
        }
        else
        {
            dprintf(DPRINTF_ETHERNET_TASK, "EthernetGetStatistics: Couldn't find requested interface\n");
        }
    }
    else
    {
        dprintf(DPRINTF_ETHERNET_TASK, "EthernetGetStatistics: ethernetStatisticsPtr is NULL\n");
    }

    return (returnCode);
}


/*****************************************************************************
** FUNCTION NAME:   EthernetLinkMonitor
**
** PARAMETERS:      None
**
** DESCRIPTION:     This function monitors the state of the i82559's link
**                  indicators.
**
** RETURNS:         GOOD  - Link values read and copied into
**                          structure pointed to by ethernetLinkStatusPtr
**                  ERROR - Error reading/saving link properties and the
**                          link values returned are not accurate
******************************************************************************/
UINT32 EthernetLinkMonitor(ETHERNET_LINK_STATUS *ethernetLinkStatusPtr)
{
    static GENERAL_STATUS_BYTE currentGeneralStatus = { {0, 0, 0, 0} };
    static GENERAL_STATUS_BYTE previousGeneralStatus = { {0, 0, 0, 0} };
    static UINT8 firstTimeReadingLinkStatus = TRUE;

    /*
     * Adding a quick hack here to display link status and notify FM when the
     * link status (up/down) changes.  Perhaps we can add more detail later,
     * but for now this is all we need.
     */
    char        linkStatusString[10];
    INT32       flags;
    TASK_PARMS  intParms;

    if (!ethernetLinkStatusPtr)
    {
        dprintf(DPRINTF_ETHERNET_LINK_MONITOR, "ELM: ethernetLinkStatusPtr is NULL\n");
        return ERROR;
    }

    /* Initialize the output structure */

    memset(ethernetLinkStatusPtr, 0, sizeof(*ethernetLinkStatusPtr));

    /*
     * We can't actually get this info (through ioctl() anyway),
     * so just dummy it up.
     */
    ethernetLinkStatusPtr->bits.wireSpeed = WIRE_SPEED_100_MBPS;
    ethernetLinkStatusPtr->bits.duplexMode = DUPLEX_MODE_FULL;

    /* Save last to previous */

    previousGeneralStatus = currentGeneralStatus;

    /* Go get the available link status flags */

    flags = GetLinkStatusFromInterface(ethernetDriver.interfaceHandle);
    if (flags < 0)
    {
        dprintf(DPRINTF_ETHERNET_LINK_MONITOR, "ELM: Couldn't get link status from kernel\n");
        return ERROR;
    }

    /* Not much we care about in here other than UP and RUNNING */

    if ((flags & IFF_UP) && (flags & IFF_RUNNING))
    {
        ethernetLinkStatusPtr->bits.linkStatus = currentGeneralStatus.bits.linkStatus = LINK_STATUS_UP;
    }
    else
    {
        ethernetLinkStatusPtr->bits.linkStatus = currentGeneralStatus.bits.linkStatus = LINK_STATUS_DOWN;
    }

    /* Modify the linkStatusString bit appropriately */

    if (previousGeneralStatus.bits.linkStatus != currentGeneralStatus.bits.linkStatus)
    {
        ethernetLinkStatusPtr->bits.linkStatusChange = TRUE;
    }

    /* Display the link status strings out to the debug console */

    EthernetGetLinkStatusString(linkStatusString,
                                currentGeneralStatus.bits.linkStatus,
                                sizeof(linkStatusString));

    dprintf(DPRINTF_ETHERNET_LINK_MONITOR, "ELM:   %s\n", linkStatusString);

    /*
     * Send the async event to notify the rest of the system
     * about the change in link status.
     */
    if (ethernetLinkStatusPtr->bits.linkStatusChange == TRUE)
    {
        if (currentGeneralStatus.bits.linkStatus == LINK_STATUS_UP)
        {
            dprintf(DPRINTF_ETHERNET_LINK_MONITOR, "ELM: Sending link up log message\n");

            EthernetLogMessage(LOG_ETHERNET_LINK_UP,
                               sizeof(ethernetLinkStatusPtr->value),
                               (UINT32)ethernetLinkStatusPtr->value);

            /* Report to Failure Manager that link is up. */

            intParms.p1 = (UINT32)GetMyControllerSN();
            intParms.p2 = (UINT32)GetMyControllerSN();
            intParms.p3 = (UINT32)IPC_LINK_TYPE_ETHERNET;
            intParms.p4 = (UINT32)PROCESS_CCB;
            intParms.p5 = (UINT32)IPC_LINK_ERROR_OK;
            UpdateLinkStatus(&intParms);
        }
        else
        {
            dprintf(DPRINTF_ETHERNET_LINK_MONITOR, "ELM: Sending link down log message\n");

            EthernetLogMessage(LOG_ETHERNET_LINK_DOWN,
                               sizeof(ethernetLinkStatusPtr->value),
                               (UINT32)ethernetLinkStatusPtr->value);

            /* Report to Failure Manager that link is down. */

            intParms.p1 = (UINT32)GetMyControllerSN();
            intParms.p2 = (UINT32)GetMyControllerSN();
            intParms.p3 = (UINT32)IPC_LINK_TYPE_ETHERNET;
            intParms.p4 = (UINT32)PROCESS_CCB;
            intParms.p5 = (UINT32)IPC_LINK_ERROR_NO_LINK;
            UpdateLinkStatus(&intParms);
        }
    }

    /* Indicate that we've now read the link status at least one time. */

    firstTimeReadingLinkStatus = FALSE;

    /* The ethernet link properties were able to be read, so return GOOD */

    return GOOD;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
