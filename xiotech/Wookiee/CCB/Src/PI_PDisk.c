/* $Id: PI_PDisk.c 157423 2011-08-02 20:30:41Z m4 $*/
/*===========================================================================
** FILE NAME:       PI_PDisk.c
** MODULE TITLE:    Packet Interface for PDisk Commands
**
** DESCRIPTION:     Handler functions for PDisk request packets.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "AsyncEventHandler.h"
#include "CachePDisk.h"
#include "CacheManager.h"
#include "CmdLayers.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "kernel.h"
#include "ipc_packets.h"
#include "ipc_common.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PI_CmdHandlers.h"
#include "PI_Utils.h"
#include "PI_PDisk.h"
#include "PortServer.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "ses.h"
#include "sm.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define TMO_PI_PDISK_LABEL_CMD              20000       /* 20  second timeout */

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void ResetLEDTask(TASK_PARMS *parms);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_PDisks - FROM CACHE
**
** Description: Physical Disks Request Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_PDisksCache(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_PDISKS_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    UINT8       retrycount = 5;

    dprintf(DPRINTF_PI_COMMANDS, "PI_PDisks - from CACHE\n");

    /*
     * Wait until the physical disks cache is not in
     * the process of being updated.  Once it is in
     * that state make sure it is set to in use so
     * a update doesn't start while it is being
     * used.
     */
    CacheStateWaitUpdating(cachePhysicalDisksState);
    CacheStateSetInUse(cachePhysicalDisksState);

    while (PI_IsCacheDirty(1 << PI_CACHE_INVALIDATE_PDISK) && retrycount)
    {
        CacheStateSetNotInUse(cachePhysicalDisksState);
        TaskSleepMS(200);
        retrycount--;
        CacheStateSetInUse(cachePhysicalDisksState);
    }
    if (retrycount == 0)
    {
        fprintf(stderr, "%s ATTENTION:retrycount reached zero\n", __FUNCTION__);
    }

    /*
     * Calculate the size of the output packet.  This will be the size
     * of the physical disks (multiple devices) response packet plus the
     * size of a physical disk info (single device) response packet for each
     * device.
     */
    outPktSize = sizeof(*ptrOutPkt) + CalcSizePDisksCached();

    ptrOutPkt = MallocWC(outPktSize);

    /*
     * Copy the number of devices into the output packet along with the
     * info for each device.
     */
    ptrOutPkt->count = PhysicalDisksCount();
    PhysicalDisksGet(&ptrOutPkt->pdiskInfo);

    CacheStateSetNotInUse(cachePhysicalDisksState);

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = outPktSize;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    PI_PDisks - MRP Requests
**
** Description: Physical Disks Request Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_PDisks(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrList = NULL;
    UINT16      count = 0;
    PI_PDISKS_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };

    dprintf(DPRINTF_PI_COMMANDS, "PI_PDisks - MRPs issued\n");

    /*
     * Get the list of objects.  Always start at the beginning and return
     * the entire list.
     */
    ptrList = PI_GetList(0, (MRGETPLIST | GET_LIST));

    /* If we could get the list use it, otherwise signal an error */
    if (ptrList != NULL)
    {
        /*
         * Calculate the size of the output packet.  This will be the size
         * of the physical disks (multiple devices) response packet plus the
         * size of a physical disk info (single device) response packet for each
         * device.
         */
        outPktSize = sizeof(*ptrOutPkt) + (ptrList->ndevs * sizeof(PI_PDISK_INFO_RSP));

        ptrOutPkt = MallocWC(outPktSize);

        /* Save the number of devices in the output packet */
        ptrOutPkt->count = ptrList->ndevs;

        /*
         * Allocate memory for the request (header and data) and the
         * response header. The response data will be allocated in the called
         * function.
         */
        reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
        reqPacket.pPacket = MallocWC(sizeof(PI_PDISK_INFO_REQ));
        rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
        rspPacket.pPacket = NULL;
        reqPacket.pHeader->packetVersion = 1;
        rspPacket.pHeader->packetVersion = 1;

        /* Fill in the Header */
        reqPacket.pHeader->commandCode = PI_PDISK_INFO_CMD;
        reqPacket.pHeader->length = sizeof(PI_PDISK_INFO_REQ);

        for (count = 0; count < ptrList->ndevs; count++)
        {
            rspPacket.pPacket = NULL;

            /* Setup the ID for this Physical Disk Information Request */
            ((PI_PDISK_INFO_REQ *)reqPacket.pPacket)->id = ptrList->list[count];

            /* Issue the command through the packet command handler */
            rc = PortServerCommandHandler(&reqPacket, &rspPacket);

            /*
             * If the request for the information was successful
             * copy the new data into the response packet.  Otherwise
             * break out of the loop and return an error to the caller.
             */
            if (rc == PI_GOOD)
            {
                memcpy(&ptrOutPkt->pdiskInfo[count],
                       rspPacket.pPacket, sizeof(PI_PDISK_INFO_RSP));
            }
            else
            {
                break;
            }

            if ((rspPacket.pPacket != NULL) && (rc != PI_TIMEOUT))
            {
                Free(rspPacket.pPacket);
            }
        }
    }
    else
    {
        /* The list could not be retrieved so an error must be returned. */
        rc = PI_ERROR;
    }

    /* Free the allocated memory */
    Free(ptrList);
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    if (rc == PI_GOOD)
    {
        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        pRspPacket->pHeader->length = outPktSize;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }
    else
    {
        Free(ptrOutPkt);

        /* Indicate an error condition and no return data in the header. */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_PDiskLabel
**
** Description: Label a PDisk
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_PDiskLabel(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MRLABEL_REQ *ptrInPkt;
    MRLABEL_RSP *ptrOutPkt;
    MRGETPINFO_RSP phyDevOut;

    /*
     * Put the FFFF pattern in the pid, so we can check that we
     * had a successful update from the cached pdisk information.
     */
    phyDevOut.pdd.pid = 0xFFFF;

    /* Get the physical disk information from the cache. */
    PhysicalDiskGet(((PI_PDISK_LABEL_REQ *)pReqPacket->pPacket)->pid, &phyDevOut);

    /* Check that the pids match. If they don't, this pdisk does not exist. */
    if (phyDevOut.pdd.pid != ((PI_PDISK_LABEL_REQ *)pReqPacket->pPacket)->pid)
    {
        rc = PI_ERROR;
        pRspPacket->pPacket = NULL;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = DEINVPID;
    }

    /*
     * The SES and slot information for the physical disk must
     * be valid before the label can take place.
     */
    else if (((PI_PDISK_LABEL_REQ *)pReqPacket->pPacket)->labtype == MLDNOLABEL ||
             (phyDevOut.pdd.ses != 0xFFFF && phyDevOut.pdd.slot != 0xFF))
    {
        /* Allocate the input and output packets for the MRP. */
        ptrInPkt = MallocWC(sizeof(*ptrInPkt));
        ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

        ptrInPkt->pid = ((PI_PDISK_LABEL_REQ *)pReqPacket->pPacket)->pid;
        ptrInPkt->labtype = ((PI_PDISK_LABEL_REQ *)pReqPacket->pPacket)->labtype;
        ptrInPkt->option = ((PI_PDISK_LABEL_REQ *)pReqPacket->pPacket)->option;

        /*
         * If this is a relabel operation there is more work to be done
         * before passing the request through the layers:
         *  - Get the current PDISK information so we can retrieve the
         *    current label type and dname.
         *  - Update the label type to the current label type
         *  - Update the dname
         */
        if (ptrInPkt->labtype == MLDRELABEL)
        {
            /*
             * A relabel command normally just sets the class to the same as it
             * already is...just updates the DNAME.
             */
            ptrInPkt->labtype = phyDevOut.pdd.devClass;

            /*
             * If the SES and slot match the current SES and slot in
             * the DNAME, copy the current DNAME into the input packet.
             */
            if (phyDevOut.pdd.devName[PD_DNAME_CSES] == phyDevOut.pdd.ses &&
                phyDevOut.pdd.devName[PD_DNAME_CSLOT] == phyDevOut.pdd.slot)
            {
                ptrInPkt->dname[PD_DNAME_OSES] = phyDevOut.pdd.devName[PD_DNAME_OSES];
                ptrInPkt->dname[PD_DNAME_OSLOT] = phyDevOut.pdd.devName[PD_DNAME_OSLOT];
                ptrInPkt->dname[PD_DNAME_CSES] = phyDevOut.pdd.devName[PD_DNAME_CSES];
                ptrInPkt->dname[PD_DNAME_CSLOT] = phyDevOut.pdd.devName[PD_DNAME_CSLOT];

                /*
                 * If the device class is a hot spare, then this is a change
                 * from a hot spare to a data drive.
                 */
                if (phyDevOut.pdd.devClass == PD_HOTLAB)
                {
                    ptrInPkt->labtype = MLDDATALABEL;
                }
            }
            else
            {
                /*
                 * The SES and slot do not match the current SES and slot
                 * in the DNAME so the drive must have been moved.
                 *
                 * If the original SES and slot are 'PD' then we need
                 * to update the original SES and slot with the DNAME
                 * current SES and slot to preserve the original values.
                 *
                 * If the drive is now back in its original location,
                 * original SES and slot match the SES and slot, the
                 * DNAME should again start with 'PD'.
                 *
                 * If none of the above conditions existed it means that
                 * the drive has already been acknowledged once and the
                 * original SES and slot have the correct values so just
                 * copy them into the input packet.
                 */
                if (phyDevOut.pdd.devName[PD_DNAME_OSES] == 'P' &&
                    phyDevOut.pdd.devName[PD_DNAME_OSLOT] == 'D')
                {
                    ptrInPkt->dname[PD_DNAME_OSES] = phyDevOut.pdd.devName[PD_DNAME_CSES];
                    ptrInPkt->dname[PD_DNAME_OSLOT] = phyDevOut.pdd.devName[PD_DNAME_CSLOT];
                }
                else if (phyDevOut.pdd.devName[PD_DNAME_OSES] == phyDevOut.pdd.ses &&
                         phyDevOut.pdd.devName[PD_DNAME_OSLOT] == phyDevOut.pdd.slot)
                {
                    ptrInPkt->dname[PD_DNAME_OSES] = 'P';
                    ptrInPkt->dname[PD_DNAME_OSLOT] = 'D';
                }
                else
                {
                    ptrInPkt->dname[PD_DNAME_OSES] = phyDevOut.pdd.devName[PD_DNAME_OSES];
                    ptrInPkt->dname[PD_DNAME_OSLOT] = phyDevOut.pdd.devName[PD_DNAME_OSLOT];
                }

                /*
                 * With any of the conditions above the current SES and slot
                 * need to be filled with the SES and slot information.
                 */
                ptrInPkt->dname[PD_DNAME_CSES] = phyDevOut.pdd.ses;
                ptrInPkt->dname[PD_DNAME_CSLOT] = phyDevOut.pdd.slot;
            }
        }
        else if (ptrInPkt->labtype == MLDFIXDNAME)
        {
            /*
             * The FIXUPDNAME option basically cleans up the DNAME
             * and resets it to PD followed by SES and SLOT.
             */

            /*
             * A relabel command normally just sets the class to the same as it
             * already is...just updates the DNAME.
             */
            ptrInPkt->labtype = phyDevOut.pdd.devClass;

            /* Fill the DNAME with PD SES SLOT. */
            ptrInPkt->dname[PD_DNAME_OSES] = 'P';
            ptrInPkt->dname[PD_DNAME_OSLOT] = 'D';
            ptrInPkt->dname[PD_DNAME_CSES] = (UINT8)phyDevOut.pdd.ses;
            ptrInPkt->dname[PD_DNAME_CSLOT] = phyDevOut.pdd.slot;
        }
        else
        {
            /* If the drive is currently labeled we will copy the dname. */
            if (phyDevOut.pdd.devClass != PD_UNLAB)
            {
                ptrInPkt->dname[PD_DNAME_OSES] = phyDevOut.pdd.devName[PD_DNAME_OSES];
                ptrInPkt->dname[PD_DNAME_OSLOT] = phyDevOut.pdd.devName[PD_DNAME_OSLOT];
                ptrInPkt->dname[PD_DNAME_CSES] = phyDevOut.pdd.devName[PD_DNAME_CSES];
                ptrInPkt->dname[PD_DNAME_CSLOT] = phyDevOut.pdd.devName[PD_DNAME_CSLOT];
            }
            else
            {
                ptrInPkt->dname[PD_DNAME_OSES] = 'P';
                ptrInPkt->dname[PD_DNAME_OSLOT] = 'D';
                ptrInPkt->dname[PD_DNAME_CSES] = (UINT8)phyDevOut.pdd.ses;
                ptrInPkt->dname[PD_DNAME_CSLOT] = phyDevOut.pdd.slot;
            }
        }

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
        rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRLABEL,
                        ptrOutPkt, sizeof(*ptrOutPkt),
                        MAX(GetGlobalMRPTimeout(), TMO_PI_PDISK_LABEL_CMD));

        /* Free the allocated memory for the input packet. */
        Free(ptrInPkt);

        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;
    }
    else
    {
        rc = PI_ERROR;
        pRspPacket->pPacket = NULL;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = DEINVSESSLOT;
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    PI_PDiskBeacon
**
** Description: Beacon the requested physical disk
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_PDiskBeacon(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_PDISK_INFO_RSP *pPDisk = NULL;
    PI_PDISK_BEACON_RSP *ptrOutPkt = NULL;
    IPC_LED_CHANGE *pEvent = NULL;
    INT32       rc = PI_GOOD;
    TASK_PARMS  parms;

    /* Allocate memory for the PDisk Info MRP and the response data. */
    ptrOutPkt = MallocWC(sizeof(*ptrOutPkt));

    /* Get PDisk info to determine the WWN for the requested PID. */
    pPDisk = PhysicalDisk(((PI_PDISK_BEACON_REQ *)(pReqPacket->pPacket))->id);

    /* If the request completed successfully, continue. */
    if (pPDisk != NULL)
    {
        /* Fill in the async event to blink the LED. */
        pEvent = MallocWC(sizeof(*pEvent));

        pEvent->serialNum = GetMyControllerSN();
        memcpy(&pEvent->wwn, &pPDisk->pdd.wwn, sizeof(UINT64));
        pEvent->ledReq = LED_ID_ON;
        EnqueueToLedControlQueue(pEvent);

        /*
         * Fork a task that will wait the requested time and then
         * shut the led off.  Note: 'event' will be freed by the
         * forked task.  We pass in id and duration instead of
         * a pointer to the request packet since this packet gets
         * free'd below.
         */
        parms.p1 = (UINT32)pEvent->serialNum;
        parms.p2 = (UINT32)(pEvent->wwn & 0x00000000FFFFFFFF);
        parms.p3 = (UINT32)((pEvent->wwn >> 32) & 0x00000000FFFFFFFF);
        parms.p4 = (UINT32)((PI_PDISK_BEACON_REQ *)(pReqPacket->pPacket))->duration;
        TaskCreate(ResetLEDTask, &parms);
    }
    else
    {
        rc = PI_ERROR;
        ptrOutPkt->header.status = DEINVPID;
        pRspPacket->pHeader->errorCode = DEINVPID;
    }

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.  There really
     * isn't a return packet for this command but since we need to
     * return something we just copy the status into the response packet.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
    pRspPacket->pHeader->status = rc;

    /*
     * Free memory from valid pointers.  Don't free MRP output memory
     * if the MRP has timed out.
     */
    Free(pPDisk);

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_PDiskQLTimeout
**
** Description: Emulate the Qlogic Timeout on specified pdisk.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_PDiskQLTimeout(XIO_PACKET* pReqPacket, XIO_PACKET* pRspPacket)
{
    UINT16 pid = ((MR_DEVID_REQ*)pReqPacket->pPacket)->id;
    UINT8 flag = ((MR_DEVID_REQ*)pReqPacket->pPacket)->option;

    MRPDISKQLTIMEOUT_REQ*    ptrInPkt;
    MRPDISKQLTIMEOUT_RSP*    ptrOutPkt;
    UINT16              rc;

    ptrInPkt = MallocWC(sizeof(MRPDISKQLTIMEOUT_REQ));
    ptrOutPkt = MallocSharedWC(sizeof(MRPDISKQLTIMEOUT_RSP));

    /* Load the wwn/lun into the input packet and call the MRP
     * to do the wwn/lun to pid translation. */
    ptrInPkt->pid = pid;
    ptrInPkt->flag = flag;

    /* Send the request. This function handles timeout conditions and task
     * switches while waiting. */
    rc = PI_ExecMRP(ptrInPkt, sizeof(MRPDISKQLTIMEOUT_REQ), MRPDISKQLTIMEOUT,
                    ptrOutPkt, sizeof(MRPDISKQLTIMEOUT_RSP), MRP_STD_TIMEOUT);

    Free(ptrInPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    /* Attach no return packet to the response packet. Fill in header length
     * and status fields.*/
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = rc;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_PDiskBypass
**
** Description: Bypass the requested physical disk
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_PDiskBypass(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    SESBypassCtrl(((PI_PDISK_BYPASS_REQ *)pReqPacket->pPacket)->ses,
                  ((PI_PDISK_BYPASS_REQ *)pReqPacket->pPacket)->slot,
                  ((PI_PDISK_BYPASS_REQ *)pReqPacket->pPacket)->setting);

    /*
     * Attach the return data packet (if applicable) to the main response
     * packet.  Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = 0;

    return (PI_GOOD);
}

/**
******************************************************************************
**
**  @brief      Defragment the requested physical disk.
**
**  @param      XIO_PACKET* pReqPacket - pointer to the request packet
**
**  @param      XIO_PACKET* pRspPacket - pointer to the response packet
**
**  @return     PI_GOOD, PI_ERROR or one of the other PI return codes.
**
******************************************************************************
**/
INT32 PI_PDiskDefrag(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    UINT16      pid;
    PI_PDISK_DEFRAG_RSP *pRsp;
    MRDEFRAGMENT_RSP *ptrOutPkt = NULL;
    INT32       rc;
    UINT8       status = DEOK;

    pid = ((PI_PDISK_DEFRAG_REQ *)pReqPacket->pPacket)->id;

    /*
     * Check the PID value.  If the PID indicates this is an orphan
     * clean up, just pass it to the proc code and quit.
     */
    if (pid == MRDEFRAGMENT_ORPHANS)
    {
        rc = StopIO(GetMyControllerSN(),
                    STOP_WAIT_FOR_FLUSH | STOP_NO_BACKGROUND,
                    STOP_NO_SHUTDOWN,
                    START_STOP_IO_USER_CCB_ORPHANS,
                    TMO_DEFRAG_ORPHAN_STOP_IO);

        if (rc == GOOD)
        {
            /*
             * Send the command to the proc code to delete the orphans.
             * This is not a normal defrag operation, so it does not
             * use the normal defrag path.
             */
            ptrOutPkt = MallocWC(sizeof(*ptrOutPkt));

            rc = PDiskDefragControl(0, pid, 0, 0, ptrOutPkt);

            if (rc != PI_TIMEOUT)
            {
                Free(ptrOutPkt);
            }
        }
        else
        {
            status = DEOUTOPS;
        }

        StartIO(GetMyControllerSN(), START_IO_OPTION_CLEAR_ONE_STOP_COUNT,
                START_STOP_IO_USER_CCB_ORPHANS, 0);
    }
    else
    {
        /*
         * Check if there is a defrag already running or if this is
         * a stop operation.
         */
        if (!Qm_GetDefragActive() || pid == MRDEFRAGMENT_STOP_PID)
        {
            /*
             * Start the defrag operation for the given PID but tell it not
             * to delay.
             */
            PDiskDefragOperation(pid, 0);

            /*
             * The defrag operation was started successfully so
             * set the status accordingly.
             */
            status = DEOK;
        }
        else
        {
            /*
             * There is already a defrag operation in progress so
             * set the status accordingly.
             */
            status = DEACTIVEDEF;
        }
    }

    /*
     * Create the dummy reponse packet.  This is to keep compatible
     * with older versions of code that used to use a pass-through
     * packet.
     */
    pRsp = MallocWC(sizeof(*pRsp));
    pRsp->header.status = status;
    pRsp->header.len = sizeof(MR_GENERIC_RSP);

    /*
     * Attach the return data packet (if applicable) to the main response
     * packet.  Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)pRsp;
    pRspPacket->pHeader->length = sizeof(*pRsp);
    pRspPacket->pHeader->status = (status == DEOK ? PI_GOOD : PI_ERROR);
    pRspPacket->pHeader->errorCode = status;

    return (PI_GOOD);
}

/**
******************************************************************************
**
**  @brief      Defragment status
**
**  @param      XIO_PACKET* pReqPacket - pointer to the request packet
**
**  @param      XIO_PACKET* pRspPacket - pointer to the response packet
**
**  @return     PI_GOOD, PI_ERROR or one of the other PI return codes.
**
******************************************************************************
**/
INT32 PI_PDiskDefragStatus(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_PDISK_DEFRAG_STATUS_RSP *pRsp;

    /* Create the reponse packet. */
    pRsp = MallocWC(sizeof(*pRsp));

    /* Fill in the response data from master config. */
    pRsp->pdiskID = Qm_GetDefragPID();
    pRsp->flag = Qm_GetDefragActive();

    /*
     * Attach the return data packet (if applicable) to the main response
     * packet.  Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)pRsp;
    pRspPacket->pHeader->length = sizeof(*pRsp);
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = 0;

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    ResetLEDTask
**
** Description: Reset a PDisk LED after a beacon request
**
** Inputs:      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is the
**                                  serial number, p2 and p3 are the WWN
**                                  of the physical disk and p4 is the
**                                  duration of the beacon.
**
** Returns:     PI_GOOD or PI_ERROR
**
** WARNING:     This function is private to this file.  It is forked by
**              PI_PDiskBeacon() to shut off a timed beacon request.
**
**--------------------------------------------------------------------------*/
static void ResetLEDTask(TASK_PARMS *parms)
{
    IPC_LED_CHANGE *pEvent = NULL;
    UINT32      sn = parms->p1;
    UINT16      duration = parms->p4;
    UINT64      wwn = parms->p3;

    wwn = ((wwn << 32) & 0xFFFFFFFF00000000LL);
    wwn |= parms->p2;

    /* Set a timer for the blink event. */
    TaskSleepMS(((UINT32)duration) * 1000);

    pEvent = MallocWC(sizeof(*pEvent));

    pEvent->serialNum = sn;
    pEvent->wwn = wwn;
    pEvent->ledReq = LED_ID_OFF;
    EnqueueToLedControlQueue(pEvent);
}

/*----------------------------------------------------------------------------
** Function:    PhysicalDisks
**
** Desc:        This method will retrieve the physical disk information for all
**              physical disks.
**
** Inputs:      NONE
**
** Returns:     Physical Disks response packet.
**
** WARNING:     The caller of this method will need to free the response packet
**              after they have finished using it.
**--------------------------------------------------------------------------*/
PI_PDISKS_RSP *PhysicalDisks(void)
{
    UINT32      rc;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    PI_PDISKS_RSP *pResponse = NULL;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = NULL;
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the Header */
    reqPacket.pHeader->commandCode = PI_PDISKS_CMD;
    reqPacket.pHeader->length = 0;

    /* Issue the command through the packet command handler */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc == PI_GOOD)
    {
        ccb_assert(rspPacket.pPacket != NULL, rspPacket.pPacket);

        pResponse = (PI_PDISKS_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "PhysicalDisks - Failed to retrieve the physical disks information.\n");
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return pResponse;
}

/*----------------------------------------------------------------------------
** Function:    PhysicalDisk
**
** Desc:        This method will retrieve the physical disk information for a
**              physical disk.
**
** Inputs:      UINT16 pid - Phyiscal disk identifier.
**
** Returns:     Physical Disk response packet.
**
** WARNING:     The caller of this method will need to free the response packet
**              after they have finished using it.
**--------------------------------------------------------------------------*/
PI_PDISK_INFO_RSP *PhysicalDisk(UINT16 pid)
{
    UINT32      rc;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    PI_PDISK_INFO_RSP *pResponse = NULL;

    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_PDISK_INFO_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_PDISK_INFO_CMD;
    reqPacket.pHeader->length = sizeof(PI_PDISK_INFO_REQ);

    ((PI_PDISK_INFO_REQ *)reqPacket.pPacket)->id = pid;

    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc == PI_GOOD)
    {
        ccb_assert(rspPacket.pPacket != NULL, rspPacket.pPacket);

        pResponse = (PI_PDISK_INFO_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "PhysicalDisk - ERROR: Physical disk information request (pid: 0x%x, rc: 0x%x).\n",
                pid, rc);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return pResponse;
}

/*----------------------------------------------------------------------------
** Function:    PhysicalDisk_PIDFromWWN
**
** Desc:        This method will retrieve the PID for a device with
**              the given WWN.
**
** Inputs:      UINT64 WWN - World wide name of the device
**              UINT16 LUN - Lun of the device
**
** Returns:     UINT16 PID of the device or 0xFFFF if the device does not
**                 exist or the retrieval failed..
**
**--------------------------------------------------------------------------*/
UINT16 PhysicalDisk_PIDFromWWN(UINT64 WWN, UINT16 LUN)
{
    MRWWNLOOKUP_REQ *ptrInPkt;
    MRWWNLOOKUP_RSP *ptrOutPkt;
    UINT16      rc;
    UINT16      pid;

    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /*
     * Load the wwn/lun into the input packet and call the MRP
     * to do the wwn/lun to pid translation.
     */
    ptrInPkt->lun = LUN;
    ptrInPkt->wwn = WWN;

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRWWNLOOKUP,
                    ptrOutPkt, sizeof(*ptrOutPkt), MRP_STD_TIMEOUT);

    Free(ptrInPkt);

    /* If the request completed successfully return the PID. */
    if (rc == PI_GOOD && ptrOutPkt->type == MWLDISK)
    {
        pid = ptrOutPkt->id;
        Free(ptrOutPkt);
    }
    else
    {
        if (rc != PI_TIMEOUT)
        {
            Free(ptrOutPkt);
        }
        pid = 0xFFFF;
    }

    return pid;
}

/*----------------------------------------------------------------------------
** Function:    PI_PDiskAutoFailback - MRP Requests
**
** Description: Physical Disks Request Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_PDiskAutoFailback(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MRPDISKAUTOFAILBACKENABLEDISABLE_RSP *MRPout;
    PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_RSP *rspOut;
    INT32       rc;
    UINT32      timeout;

    if (pReqPacket->pHeader->timeout != 0)
    {
        timeout = pReqPacket->pHeader->timeout;
    }
    else
    {
        timeout = GetGlobalMRPTimeout();
    }
    timeout = MAX(timeout, TMO_NONE);

    /* Allocate memory for the MRP return data. */
    MRPout = MallocSharedWC(sizeof(*MRPout));

    rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length,
                    MRPDISKAUTOFAILBACKENABLEDISABLE, MRPout, sizeof(*MRPout), timeout);

    /* Fill in the header length and status fields. And translate other values for PI. */
    rspOut = MallocSharedWC(sizeof(*rspOut));
    rspOut->mode = MRPout->mode;
    rspOut->status = MRPout->header.status;
    pRspPacket->pPacket = (UINT8 *)rspOut;
    pRspPacket->pHeader->length = sizeof(PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_RSP);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = MRPout->header.status;

    /* Only free MRP output memory if MRP command did not timeout. */
    if (rc != PI_TIMEOUT)
    {
        Free(MRPout);
    }
    return (rc);
}   /* End of PI_PDiskAutoFailback */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
