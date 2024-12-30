/* $Id: debug_struct.c 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       debug_struct.c
** MODULE TITLE:    Debug structure displays
**
** DESCRIPTION:     Implementation of debug structure displays.
**
** Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "debug_struct.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include "ccb_statistics.h"
#include "convert.h"
#include "debug_files.h"
#include "LargeArrays.h"
#include "LL_Stats.h"
#include "mutex.h"
#include "nvram.h"
#include "quorum.h"
#include "ses.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

#include <byteswap.h>

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    DisplayMasterConfigStruct
**
** Description: Display Master Configuration  structure
**
** Returns:     len  - length of display string
**
** WARNING:     Uses Global Big Buffer
**
**--------------------------------------------------------------------------*/
UINT32 DisplayMasterConfigStruct(void)
{
    UINT32      i;

    /*
     * gBigBuffer is available here since the "bigBufferMutex" is locked
     * by the calling function.
     */
    char       *bufP = gBigBuffer;

    /*
     * Create a string for the master configuration structure
     */
    bufP += sprintf(bufP, "schema:                  %d\n", Qm_GetSchema());
    bufP += sprintf(bufP, "magicNumber:             0x%x\n", Qm_GetMagicNumber());
    bufP += sprintf(bufP, "virtualControllerSN:     0x%x\n", Qm_GetVirtualControllerSN());
    bufP += sprintf(bufP, "electionSerial:          %d\n", Qm_GetElectionSerial());
    bufP += sprintf(bufP, "currentMasterID:         0x%x\n", Qm_GetMasterControllerSN());
    bufP += sprintf(bufP, "numControllersInVCG:     %d\n", Qm_GetNumControllersAllowed());

    for (i = 0; i < dimension_of(masterConfig.communicationsKey); ++i)
    {
        bufP += sprintf(bufP, "communicationsKey[%02d]:   0x%x\n",
                        i, masterConfig.communicationsKey[i]);
    }

    bufP += sprintf(bufP, "ipAddress:               0x%x\n", Qm_GetIPAddress());
    bufP += sprintf(bufP, "gatewayAddress:          0x%x\n", Qm_GetGateway());
    bufP += sprintf(bufP, "subnetMask:              0x%x\n", Qm_GetSubnet());

    for (i = 0; i < MAX_CONTROLLERS; ++i)
    {
        bufP += sprintf(bufP, "activeControllerMap[%02d]: 0x%x\n",
                        i, Qm_GetActiveCntlMap(i));
    }

    bufP += sprintf(bufP, "defragActive:            0x%x\n", Qm_GetDefragActive());
    bufP += sprintf(bufP, "defragPID:               0x%x\n", Qm_GetDefragPID());

    bufP += sprintf(bufP, "ownedDriveCount:         %d\n", Qm_GetOwnedDriveCount());

    bufP += sprintf(bufP, "keepAlive\n");
    bufP += sprintf(bufP, "  schema                 0x%x\n", masterConfig.keepAlive.header.schema);
    bufP += sprintf(bufP, "  flags                  0x%x\n", masterConfig.keepAlive.header.flags.value);

    for (i = 0; i < dimension_of(masterConfig.keepAlive.keepAliveList); ++i)
    {
        bufP += sprintf(bufP, "  keepAliveList[%02d]:     0x%x\n",
                        i, masterConfig.keepAlive.keepAliveList[i].value);
    }

    bufP += sprintf(bufP, "CRC:                     0x%x\n", Qm_GetCRC());

    return (INT8 *)bufP - (INT8 *)gBigBuffer;
}


/*----------------------------------------------------------------------------
** Function:    DisplaySocketStats
**
** Description: Display status of all network sockets
**
** Returns:     len  - length of display string
**
** WARNING:     The caller is responsible for freeing the malloc'd buffer.
**
**--------------------------------------------------------------------------*/
#define MAX_NUM_SOCKETS_TO_DISPLAY 100
UINT32 DisplaySocketStats(char **buffer, UINT32 showFlags)
{
    char       *bufP = (char *)*buffer;

    FILE       *pF;
    char        pLine[256];     /* this should be more than we need */
    INT32       rc;
    char       *pOffset;
    UINT32      slot;
    char        locIPstr[33];
    UINT32      locIP;
    UINT32      locPort;
    char        remIPstr[33];
    UINT32      remIP;
    UINT32      remPort;
    UINT32      state;
    UINT32      txQ;
    UINT32      rxQ;
    const char *stName = NULL;
    char        localIP[30];
    char        peerIP[30];
    UINT32      count = MAX_NUM_SOCKETS_TO_DISPLAY;
    UINT32      ipv6 = 0;

    /*
     * Sanity check the input buffer
     */
    if (buffer == NULL)
    {
        return 0;
    }
    /* num sockets x char per line */
    *buffer = MallocWC(MAX_NUM_SOCKETS_TO_DISPLAY * 100);
    bufP = (char *)*buffer;

    bufP += sprintf(bufP, "Proto Recv-Q Send-Q  Local Address:Port      "
                    "Peer Address:Port       State\n");
    count--;

    /*
     * Read and parse /proc/net/tcp to get the TCP ONLY socket statistics
     */
    do
    {
        if (ipv6)
        {
            pF = fopen("/proc/net/tcp6", "r");
        }
        else
        {
            pF = fopen("/proc/net/tcp", "r");
        }
        if (pF)
        {
            while (fgets(pLine, 256, pF) && count)
            {
                /*
                 * Change the :'s to spaces.  I know there is a way to do this
                 * with scanf, but I haven't figured it out (and had time to test
                 * it). This seems safer anyway.
                 */
                pOffset = pLine;
                while (1)
                {
                    pOffset = index(pOffset, ':');
                    if (pOffset)
                    {
                        *pOffset++ = ' ';
                    }
                    else
                    {
                        break;
                    }
                }

                /*
                 * We only really care about parameters 1-7
                 */
                rc = sscanf(pLine, "%x %s %x %s %x %x %x %x",
                            &slot,
                            locIPstr, &locPort, remIPstr, &remPort, &state, &txQ, &rxQ);

                if (ipv6)
                {
                    sscanf(&locIPstr[24], "%x", &locIP);
                    sscanf(&remIPstr[24], "%x", &remIP);
                }
                else
                {
                    sscanf(locIPstr, "%x", &locIP);
                    sscanf(remIPstr, "%x", &remIP);
                }

                if (rc == 8)
                {
                    switch (state)
                    {
                        case TCP_ESTABLISHED:
                            stName = "ESTABLISHED";
                            break;

                        case TCP_SYN_SENT:
                            stName = "SYN_SENT";
                            break;

                        case TCP_SYN_RECV:
                            stName = "SYN_RECV";
                            break;

                        case TCP_FIN_WAIT1:
                            stName = "FIN_WAIT1";
                            break;

                        case TCP_FIN_WAIT2:
                            stName = "FIN_WAIT2";
                            break;

                        case TCP_TIME_WAIT:
                            stName = "TIME_WAIT";
                            break;

                        case TCP_CLOSE:
                            stName = "CLOSE";
                            break;

                        case TCP_CLOSE_WAIT:
                            stName = "CLOSE_WAIT";
                            break;

                        case TCP_LAST_ACK:
                            stName = "LAST_ACK";
                            break;

                        case TCP_LISTEN:
                            stName = "LISTEN";
                            break;

                        case TCP_CLOSING:
                            stName = "CLOSING";
                            break;

                        default:
                            stName = "UNKNOWN";
                            break;
                    }

                    if (state == TCP_LISTEN && !showFlags)
                    {
                        continue;
                    }

                    sprintf(localIP, "%s:%u",
                            inet_ntoa(*(struct in_addr *)(&locIP)), locPort);
                    if (state == TCP_LISTEN)
                    {
                        sprintf(peerIP, "%s:*", inet_ntoa(*(struct in_addr *)(&remIP)));
                    }
                    else
                    {
                        sprintf(peerIP, "%s:%u",
                                inet_ntoa(*(struct in_addr *)(&remIP)), remPort);
                    }

                    bufP += sprintf(bufP, "TCP   %6u %6u  %-23s %-23s %-7s\n",
                                    rxQ, txQ, localIP, peerIP, stName);

                    count--;
                }
            }

            Fclose(pF);
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "DisplaySocketStats: couldn't open /proc/net/tcp%s, errno %d\n",
                    ipv6 ? "6" : "", errno);
        }

        ipv6++;

        /*
         * Loop back up and do the ipv6 connections
         */
    } while (ipv6 < 2);

    /*
     * Add in the UDP debug port manually
     */
    if (showFlags)
    {
        remIP = GetDebugConsoleIPAddr();
        if (remIP)
        {
            remPort = EthDebugPort();
            sprintf(peerIP, "%s:%u", inet_ntoa(*(struct in_addr *)(&remIP)), remPort);
            bufP += sprintf(bufP, "UDP   %6u %6u  %-23s %-23s %-7s\n",
                            0, 0, "0.0.0.0:0", peerIP, "");
        }
    }

    bufP += sprintf(bufP, "\n");

    return (bufP - (char *)*buffer);
}


/*----------------------------------------------------------------------------
** Function:    DisplaySESDeviceStruct
**
** Description: Display the SESList - SES Device and page 2
**
** Inputs:      none
**
** Returns:     len  - length of display string
**
** WARNING:     Uses Global Big Buffer
**
**--------------------------------------------------------------------------*/
UINT32 DisplaySESDeviceStruct(void)
{
    PSES_DEVICE pSES = NULL;
    SES_WWN_TO_SES *pDevMap = NULL;
    SES_WWN_TO_SES *pCurrent = NULL;
    UINT32      i;
    UINT32      finalChars;
    UINT32      wwnHigh;
    UINT32      wwnLow;
    UINT32      mapLength = 0;

    /*
     * gBigBuffer is available here since the "bigBufferMutex" is locked
     * by the calling function.
     */
    char       *bufP = gBigBuffer;

    /*
     * If a valid pointer to the list exists display the structure.
     */
    pSES = GetSESList();

    /*
     * If pSES is NULL the SES data is not available for some reason.
     * Return a string stating this - it makes life easier for the test
     * guys.
     */
    if (pSES == NULL)
    {
        bufP += sprintf(bufP, "No back end (SES) data is available at this time.\n");
    }

    /*
     * If a valid pointer is available follow the pointer to the end
     * of the data (SES data is a linked list).
     */
    while (pSES != NULL)
    {
        /*
         * Create a string for the SES Device structure
         */
        bufP += sprintf(bufP, "pNextSESDevice: %p\n", pSES->NextSES);

        wwnHigh = bswap_32((UINT32)(pSES->WWN));
        wwnLow = bswap_32((UINT32)((pSES->WWN) >> 32));
        bufP += sprintf(bufP, "WWN           : %08x %08x", wwnHigh, wwnLow);

        /* Also display as characters */
        bufP += sprintf(bufP, "  (");
        for (i = 0; i < 4; i++)
        {
            bufP += sprintf(bufP, "%c", ((char)(wwnHigh >> (24 - (8 * i)))));
        }
        for (i = 0; i < 4; i++)
        {
            bufP += sprintf(bufP, "%c", ((char)(wwnLow >> (24 - (8 * i)))));
        }
        bufP += sprintf(bufP, ")\n");

        bufP += sprintf(bufP, "SupportedPages: 0x%08x\n", pSES->SupportedPages);
        bufP += sprintf(bufP, "FCID          : 0x%08x\n", pSES->FCID);
        bufP += sprintf(bufP, "Generation    : 0x%08x\n", pSES->Generation);
        bufP += sprintf(bufP, "Channel       : 0x%02hhx\n", pSES->Channel);
        bufP += sprintf(bufP, "PID           : 0x%04hx\n", pSES->PID);
        bufP += sprintf(bufP, "LUN           : 0x%04hx\n", pSES->LUN);
        bufP += sprintf(bufP, "TotalSlots    : 0x%04hx\n", pSES->TotalSlots);
        bufP += sprintf(bufP, "pOldPage2     : %p\n", pSES->OldPage2);
        bufP += sprintf(bufP, "Revision      : 0x%08x", (*(UINT32 *)(pSES->pd_rev)));

        /* Also display revision as characters */
        bufP += sprintf(bufP, "  (");
        for (i = 0; i < 4; i++)
        {
            bufP += sprintf(bufP, "%c", pSES->pd_rev[i]);
        }
        bufP += sprintf(bufP, ")");

        bufP += sprintf(bufP, "\nDevice Type   : 0x%02hhx\n", pSES->devType);

        bufP += sprintf(bufP, "\n\nMap[]:\n");
        bufP += sprintf(bufP, "      ");
        for (i = 0; i < 16; i++)
        {
            bufP += sprintf(bufP, "0x%02X ", i);
        }

        bufP += sprintf(bufP, "\n");
        bufP += sprintf(bufP, "      ");
        for (i = 0; i < 16; i++)
        {
            bufP += sprintf(bufP, "---- ");
        }

        for (i = 0; i <= SES_ET_MAX_VAL; i++)
        {
            if ((i % 16) == 0)
            {
                bufP += sprintf(bufP, "\n0x%02X: ", i);
            }
            bufP += sprintf(bufP, "0x%02hhX ", pSES->Map[i]);
        }

        bufP += sprintf(bufP, "\n\nSlots[]:\n");
        bufP += sprintf(bufP, "      ");
        for (i = 0; i < 16; i++)
        {
            bufP += sprintf(bufP, "0x%02X ", i);
        }

        bufP += sprintf(bufP, "\n");
        bufP += sprintf(bufP, "      ");
        for (i = 0; i < 16; i++)
        {
            bufP += sprintf(bufP, "---- ");
        }

        for (i = 0; i <= SES_ET_MAX_VAL; i++)
        {
            if ((i % 16) == 0)
            {
                bufP += sprintf(bufP, "\n0x%02X: ", i);
            }
            bufP += sprintf(bufP, "0x%02hhX ", pSES->Slots[i]);
        }

        /*
         * Get the device map for this disk bay.  The map contains
         * the WWN, PID and slot for each PDisk in the bay.
         */
        mapLength = GetDeviceMap(pSES, &pDevMap);

        if (mapLength)
        {
            bufP += sprintf(bufP, "\n\nDevice map for this bay\n\n");
            bufP += sprintf(bufP, "PID   Slot  WWN\n");
            bufP += sprintf(bufP, "----  ----  -----------------\n");
        }

        pCurrent = pDevMap;

        for (i = 0; i < mapLength / sizeof(SES_WWN_TO_SES); i++)
        {
            bufP += sprintf(bufP, "%04hX  ", pCurrent->PID);
            bufP += sprintf(bufP, "%04hX  ", pCurrent->Slot);

            wwnHigh = bswap_32((UINT32)(pCurrent->WWN));
            wwnLow = bswap_32((UINT32)((pCurrent->WWN) >> 32));
            bufP += sprintf(bufP, "%08X %08X  ", wwnHigh, wwnLow);

            bufP += sprintf(bufP, "\n");

            /* Move to the next entry */
            pCurrent++;
        }

        /*
         * Free the memory used for the device map.
         */
        Free(pDevMap);

        /*
         * If the page 2 pointer is valid display this info as well.
         */
        if (pSES->OldPage2 != NULL)
        {
            bufP += sprintf(bufP, "\n\nPage 2\n");

            bufP += sprintf(bufP, "PageCode  : 0x%02hhX\n",
                        pSES->OldPage2->PageCode);
            bufP += sprintf(bufP, "Status    : 0x%02hhX\n",
                        pSES->OldPage2->Status);
            bufP += sprintf(bufP, "Length    : 0x%04hX\n",
                        bswap_16(pSES->OldPage2->Length));

            bufP += sprintf(bufP, "Generation: 0x%08X\n",
                        bswap_32(pSES->OldPage2->Generation));

            bufP += sprintf(bufP, "\nControl elements\n");

            for (i = 0; i < pSES->TotalSlots; i++)
            {
                bufP += sprintf(bufP, "0x%02X: ", i);
                bufP += sprintf(bufP, "0x%02hhX  ",
                                pSES->OldPage2->Control[i].CommonCtrl);
                bufP += sprintf(bufP, "0x%02hhX  ",
                                pSES->OldPage2->Control[i].Ctrl.Generic.Slot);
                bufP += sprintf(bufP, "0x%02hhX  ",
                                pSES->OldPage2->Control[i].Ctrl.Generic.Ctrl1);
                bufP += sprintf(bufP, "0x%02hhX  ",
                                pSES->OldPage2->Control[i].Ctrl.Generic.Ctrl2);

                bufP += sprintf(bufP, "\n");
            }
        }

        bufP += sprintf(bufP, "\n");

        if (pSES->OldPage2 != NULL)
        {
            finalChars = 28;
        }
        else
        {
            finalChars = 85;
        }

        for (i = 0; i < finalChars; i++)
        {
            bufP += sprintf(bufP, "=");
        }

        bufP += sprintf(bufP, "\n\n");

        /*
         * Move to the next structure in the list
         */
        pSES = pSES->NextSES;
    }

    return (INT8 *)bufP - (INT8 *)gBigBuffer;
}


/*----------------------------------------------------------------------------
** FUNCTION NAME:   DisplayCCBStatistics
**
** INPUTS:      None
**
** WARNING:     Uses Global Big Buffer
**
** RETURNS:     len  - length of display string
**--------------------------------------------------------------------------*/
#define SEC_IN_DAY        (60*60*24)
#define SEC_IN_HOUR       (60*60)
#define SEC_IN_MINUTE     (60)

UINT32 DisplayCCBStatistics(void)
{
    /*
     * gBigBuffer is available here since the "bigBufferMutex" is locked
     * by the calling function.
     */
    char       *bufP = gBigBuffer;
    UINT32      secs = CCBStats.hrcount >> 3;
    UINT32      days;
    UINT32      hrs;
    UINT32      mins;

    days = secs / SEC_IN_DAY;
    secs -= days * SEC_IN_DAY;

    hrs = secs / SEC_IN_HOUR;
    secs -= hrs * SEC_IN_HOUR;

    mins = secs / SEC_IN_MINUTE;
    secs -= mins * SEC_IN_MINUTE;

    bufP += sprintf(bufP, "CCB Statistics\n");
    bufP += sprintf(bufP, "  Uptime:                     %u day(s) %02u:%02u:%02u\n",
                    days, hrs, mins, secs);

    bufP += sprintf(bufP, "  Heap\n");
    bufP += sprintf(bufP, "    Heap Size:                %u\n", CCBStats.maxavl);
    bufP += sprintf(bufP, "    Cur Available:            %u\n", CCBStats.memavl);
    bufP += sprintf(bufP, "    Low Water Mark:           %u\n", CCBStats.minavl);
    bufP += sprintf(bufP, "    Wait Count:               %u\n", CCBStats.memwait);
    bufP += sprintf(bufP, "    Outstanding Mallocs:      %u\n", CCBStats.memcnt);

    bufP += sprintf(bufP, "  ILTs\n");
    bufP += sprintf(bufP, "    Total ILTs Available:     %u\n", CCBStats.iltcnt);
    bufP += sprintf(bufP, "    Maximum ILTs Used:        %u\n", CCBStats.iltmax);

    bufP += sprintf(bufP, "  Tasks\n");
    bufP += sprintf(bufP, "    Active Task Count:        %u\n", CCBStats.pcbcnt);

    bufP += sprintf(bufP, "  Link Layer\n");
    bufP += sprintf(bufP, "    Outbound VRP Count:       %u\n", CCBStats.vrpOCount);
    bufP += sprintf(bufP, "    Inbound VRP Count:        %u\n", CCBStats.vrpICount);
    bufP += sprintf(bufP, "    Total VRPs Sent:          %u\n", CCBStats.vrpOTotal);
    bufP += sprintf(bufP, "    Total VRPs Received:      %u\n", CCBStats.vrpITotal);

    bufP += sprintf(bufP, "  Firmware\n");
    bufP += sprintf(bufP, "    Version:                  %c%c%c%c\n",
                    ((UINT8 *)&CCBStats.fwvers)[0], ((UINT8 *)&CCBStats.fwvers)[1],
                    ((UINT8 *)&CCBStats.fwvers)[2], ((UINT8 *)&CCBStats.fwvers)[3]);

    bufP += sprintf(bufP, "    Revision:                 %c%c%c%c\n\n",
                    ((UINT8 *)&CCBStats.fwcomp)[0], ((UINT8 *)&CCBStats.fwcomp)[1],
                    ((UINT8 *)&CCBStats.fwcomp)[2], ((UINT8 *)&CCBStats.fwcomp)[3]);

    bufP += sprintf(bufP, "Ethernet Statistics\n");
    bufP += sprintf(bufP, "  Eyecatcher:                 %s\n",
                    CCBStats.ethernetCounters.eyecatcher);
    bufP += sprintf(bufP, "  Interface:                  %s\n",
                    CCBStats.ethernetCounters.interface);
    bufP += sprintf(bufP, "  Receive\n");
    bufP += sprintf(bufP, "    Bytes:                    %u\n",
                    CCBStats.ethernetCounters.rxBytes);
    bufP += sprintf(bufP, "    Packets:                  %u\n",
                    CCBStats.ethernetCounters.rxPackets);
    bufP += sprintf(bufP, "    Errors:                   %u\n",
                    CCBStats.ethernetCounters.rxErrs);
    bufP += sprintf(bufP, "    Dropped:                  %u\n",
                    CCBStats.ethernetCounters.rxDrop);
    bufP += sprintf(bufP, "    Overrun:                  %u\n",
                    CCBStats.ethernetCounters.rxFifo);
    bufP += sprintf(bufP, "    Frame:                    %u\n",
                    CCBStats.ethernetCounters.rxFrame);
    bufP += sprintf(bufP, "    Compressed:               %u\n",
                    CCBStats.ethernetCounters.rxCompressed);
    bufP += sprintf(bufP, "    Multicast:                %u\n",
                    CCBStats.ethernetCounters.rxMulticast);
    bufP += sprintf(bufP, "  Transmit\n");
    bufP += sprintf(bufP, "    Bytes:                    %u\n",
                    CCBStats.ethernetCounters.txBytes);
    bufP += sprintf(bufP, "    Packets:                  %u\n",
                    CCBStats.ethernetCounters.txPackets);
    bufP += sprintf(bufP, "    Errors:                   %u\n",
                    CCBStats.ethernetCounters.txErrs);
    bufP += sprintf(bufP, "    Dropped:                  %u\n",
                    CCBStats.ethernetCounters.txDrop);
    bufP += sprintf(bufP, "    Overrun:                  %u\n",
                    CCBStats.ethernetCounters.txFifo);
    bufP += sprintf(bufP, "    Collisions:               %u\n",
                    CCBStats.ethernetCounters.txColls);
    bufP += sprintf(bufP, "    Carrier:                  %u\n",
                    CCBStats.ethernetCounters.txCarrier);
    bufP += sprintf(bufP, "    Compressed:               %u\n",
                    CCBStats.ethernetCounters.txCompressed);
    bufP += sprintf(bufP, "\n");

    return ((INT8 *)bufP - (INT8 *)gBigBuffer);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
