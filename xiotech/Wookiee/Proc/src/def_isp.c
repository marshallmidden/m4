/* $Id: def_isp.c 159524 2012-07-26 20:30:11Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       def_isp.c
**
**  @brief      ISP Define functions
**
**  To provide support of ISP related MRP requests.
**
**  Copyright (c) 2003-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#include "def_isp.h"

#include "fabric.h"
#if FE_ISCSI_CODE
#include "fsl.h"
#endif
#include "isp.h"
#include "kernel.h"
#include "loop.h"
#include "misc.h"
#include "MR_Defs.h"
#include "pcb.h"
#include "portdb.h"
#include "system.h"
#include "target.h"
#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"

#ifdef FRONTEND
#include "cdriver.h"
#include "cimt.h"
#include "OS_II.h"
#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"

#endif

#ifdef BACKEND
#include "defbe.h"
#include "dev.h"
#include "nvr.h"
#endif
#include <stdio.h>
#include <string.h>
#include <byteswap.h>

#include "CT_change_routines.h"
#include "icl.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

#define WAIT_FOR_LOGIN 2
#define READY          3

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static UINT8        config_set;
static ISP_CONFIG   config;
#ifdef FRONTEND
VPD     *vpd[MAX_PORTS];                /* Virtual Port Database            */
ISP2400_VPDB_PORTCFG    *vpd_2400db[MAX_PORTS]; /* Virtual Port Database    */
#endif

/*
 *      This table is used to translate from AL-PA addresses to the
 *      7-bit Loop Identifier. The value of 127 indicates unassigned values.
 *
 *      From manual FCAL, Annex K.
 */
static const UINT8 alpa2lid_tbl[256] =
{
    /*          0/8 1/9 2/a 3/b 4/c 5/d 6/e 7/f     */

    126, 125, 124, 127, 123, 127, 127, 127,     /* 00 - 07 */
    122, 127, 127, 127, 127, 127, 127, 121,     /* 08 - 0f */
    120, 127, 127, 127, 127, 127, 127, 119,     /* 10 - 17 */
    118, 127, 127, 117, 127, 116, 115, 114,     /* 18 - 1f */
    127, 127, 127, 113, 127, 112, 111, 110,     /* 20 - 27 */
    127, 109, 108, 107, 106, 105, 104, 127,     /* 28 - 2f */
    127, 103, 102, 101, 100,  99,  98, 127,     /* 30 - 37 */
    127,  97,  96, 127,  95, 127, 127, 127,     /* 38 - 3f */
    127, 127, 127,  94, 127,  93,  92,  91,     /* 40 - 47 */
    127,  90,  89,  88,  87,  86,  85, 127,     /* 48 - 4f */
    127,  84,  83,  82,  81,  80,  79, 127,     /* 50 - 57 */
    127,  78,  77, 127,  76, 127, 127, 127,     /* 58 - 5f */
    127, 127, 127,  75, 127,  74,  73,  72,     /* 60 - 67 */
    127,  71,  70,  69,  68,  67,  66, 127,     /* 68 - 6f */
    127,  65,  64,  63,  62,  61,  60, 127,     /* 70 - 77 */
    127,  59,  58, 127,  57, 127, 127, 127,     /* 78 - 7f */
     56,  55,  54, 127,  53, 127, 127, 127,     /* 80 - 87 */
     52, 127, 127, 127, 127, 127, 127,  51,     /* 88 - 8f */
     50, 127, 127, 127, 127, 127, 127,  49,     /* 90 - 97 */
     48, 127, 127,  47, 127,  46,  45,  44,     /* 97 - 9f */
    127, 127, 127,  43, 127,  42,  41,  40,     /* a0 - a7 */
    127,  39,  38,  37,  36,  35,  34, 127,     /* a8 - af */
    127,  33,  32,  31,  30,  29,  28, 127,     /* b0 - b7 */
    127,  27,  26, 127,  25, 127, 127, 127,     /* b8 - bf */
    127, 127, 127,  24, 127,  23,  22,  21,     /* c0 - c7 */
    127,  20,  19,  18,  17,  16,  15, 127,     /* c8 - cf */
    127,  14,  13,  12,  11,  10,   9, 127,     /* d0 - d7 */
    127,   8,   7, 127,   6, 127, 127, 127,     /* d8 - df */
      5,   4,   3, 127,   2, 127, 127, 127,     /* e0 - e7 */
      1, 127, 127, 127, 127, 127, 127,   0,     /* e8 - ef */
    127, 127, 127, 127, 127, 127, 127, 127,     /* f0 - f7 */
    127, 127, 127, 127, 127, 127, 127, 127      /* f8 - ff */
};


/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
MRFEPORTNOTIFY_REQ mpn;

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      To process the Get Device List MRP
**
**              To provide a means of dumping the ISP2x00 registers and sram
**              for debugging by qlogic.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DI_GetDeviceList(MR_PKT *pMRP)
{
    UINT16 i;                   /* Loop variables                       */
    MRGETDVLIST_REQ *mgcs;      /* Pointer to input MRP                 */
    UINT8  port;                /* Port Number                          */
    UINT16 ndevs;               /* Number of devices found              */
    UINT16 count;               /* Device Count                         */
    UINT16 maxCount;            /* Maximum number of devices in list    */
    MRGETDVLIST_INFO *pList;    /* Returned list of devices             */
    PNNL *buffer;
#ifdef FRONTEND
    ISP2400_VPDB_PORTCFG *vp24Info;
    TAR *pTar;
#endif
    UINT8  retStatus;
    UINT16 firmwareState;
    ISP2400_VPORT_ICB *pVPICB;

    /* If tar list gets changed, we might need to retry. */
  restart:
    count = 1;                  /* Initialize device count. */
    retStatus = DEOK;           /* Default to return status of OK. */
    /* Get pointer to Parm block address */
    mgcs = (MRGETDVLIST_REQ *)pMRP->pReq;

    /* First, grab the return data address and length allowed */
    pList = (MRGETDVLIST_INFO *)(pMRP->pRsp + 1);
    pMRP->pRsp->rspLen = pMRP->rspLen;
    maxCount = ((UINT16) pMRP->rspLen - sizeof(MR_RSP_PKT)) / sizeof(MRGETDVLIST_INFO);

    /* Fill in the fixed part of the structure */
    pMRP->pRsp->status = DEOK;               /* Prep good return value       */

    port = mgcs->port;  /* Get port number from the input packet */

#if FE_ISCSI_CODE
    if (BIT_TEST(iscsimap, port))
    {
        return DEINVOP;
    }
#endif

    /* Check if this QLogic instance exists. */
    if (port < ispmax && BIT_TEST(isprena, port))
    {
        /* Get the firmware State */
        pVPICB = (ISP2400_VPORT_ICB *)ispstr[port]->icbStr;
        firmwareState = ISP_GetFirmwareState(port);

        if (firmwareState == READY || firmwareState == WAIT_FOR_LOGIN)
        {
            /* Get Loop ID and Port ID for this QLogic Instance */
            ISP_GetLoopId(port,0);
        }
        else
        {
            /* Firmware state results to no connection */
            ispConnectionType[port] = NO_CONNECT;
        }

        if (maxCount == 0)
        {
            retStatus = DETOOMUCHDATA;
        }
        else
        {
            /* Copy data to the output packet */
            if (ispConnectionType[port] == NO_CONNECT)
            {
                pList->lid    = NO_LID;
                pList->portID = NO_PORTID;
            }
            else
            {
                pList->lid    = ispLid[port];
                pList->portID = portid[port];
            }

            pList->mst     = (UINT8) firmwareState;
            pList->sst     = (UINT8) ispConnectionType[port];

            pList->wwnPort = pVPICB->nicb.portWWN;
            pList->wwnNode = pVPICB->nicb.nodeWWN;

            ++pList;    /* Increment to next device in list */
        }

#ifdef FRONTEND
        /* Check for Multi-ID Code */
        if (BIT_TEST(ispmid, port))
        {
            /* Allocate non-cachable DRAM buffer for VP database */
            if (vpd_2400db[port] == NULL)
            {
                vpd_2400db[port] = s_MallocW(sizeof(ISP2400_VPDB_PORTCFG) * 32, __FILE__, __LINE__);
            }

            /* Read the virtual port database */
            ndevs = ISP2400_GetVPDatabase(port, vpd_2400db[port]);

            /* Check if any additional virtual ports exists */
            for (i = 1; i < ndevs; ++i)
            {
                /* Check if there is too much data for the return packet */
                if (count >= maxCount)
                {
                    retStatus = DETOOMUCHDATA;
                }
                else
                {
                    vp24Info = &vpd_2400db[port][i];

                    if ((vp24Info->vpstatus & VPDB_VP_ENABLED) == 0)
                    {
                        pList->lid    = NO_LID;
                        pList->portID = NO_PORTID;
                    }
                    else
                    {
                        pList->lid    = alpa2lid_tbl[vp24Info->portid& 0xff];
                        pList->portID = vp24Info->portid;
                    }

                    /* Copy data to the output packet. */
                    pList->mst     = (UINT16)vp24Info->vpstatus;
                    pList->sst     = (UINT16)vp24Info->vpoptions;
                    pList->wwnPort  = bswap_64(vp24Info->portWWN) >> 32;
                    pList->wwnPort |= bswap_64(vp24Info->portWWN) << 32;
                    pList->wwnNode  = bswap_64(vp24Info->nodeWWN) >> 32;
                    pList->wwnNode |= bswap_64(vp24Info->nodeWWN) << 32;

                    /* Increment to next device in list. */
                    ++pList;
                }
                ++count; /* Increment the device count */
            }
        } /* if multi id code*/
#endif
        if (ispConnectionType[port] != NO_CONNECT)
        {
            /* Allocate non-cachable DRAM buffer for ID List. */
            buffer = s_MallocW( sizeof(PNNL) * MAX_DEV, __FILE__, __LINE__);

            /* Issue Get Port/Node Name List command to the Qlogic chip instance */
#ifdef FRONTEND
            for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
#endif  /* FRONTEND */
            {
                /* NOTE: ISP_GetPortNodeNameList may task switch. */
                UINT32 save_tar_link_abort = tar_link_abort[port];
#ifdef FRONTEND
                ndevs = ISP_GetPortNodeNameList(port, pTar->vpID, buffer);
#else
                ndevs = ISP_GetPortNodeNameList(port, 0, buffer);
#endif
                /* Check if devices are logged in. */
                if (ndevs != 0)
                {
                    for (i = 0; i < ndevs; ++i)
                    {
                        /* Check if there is too much data for the return packet. */
                        if ((count + i) >= maxCount)
                        {
                            retStatus = DETOOMUCHDATA;
                            break;
                        }

                        /* Copy data to the output packet */
                        pList->lid     = buffer[i].lid;
                        pList->mst     = buffer[i].mst;
                        pList->sst     = buffer[i].sst;
                        pList->portID  = buffer[i].portID;
                        pList->wwnPort = bswap_64(buffer[i].pdn);
                        pList->wwnNode = bswap_64(buffer[i].ndn);

                        ++pList;    /* Increment to next device in list */
                    }
                    /* Increment the count by the number of devices */
                    count += ndevs;
                }
                /* If tar list got smaller, then the linked list may not be valid, restart. */
                if (save_tar_link_abort != tar_link_abort[port])
                {
                    s_Free(buffer, sizeof(PNNL) * MAX_DEV, __FILE__, __LINE__);   /* Release ID List buffer */
                    goto restart;
                }
            }
            s_Free(buffer, sizeof(PNNL) * MAX_DEV, __FILE__, __LINE__);   /* Release ID List buffer */
        }
    }
    else
    {
        retStatus = DEINVCHAN;  /* Channel is not valid */
    }

    /* Store the number of devices found in the return packet */
    pMRP->pRsp->nDevs = count;
    pMRP->pRsp->status = retStatus;

    return retStatus;
} /* DI_GetDeviceList */


/**
******************************************************************************
**
**  @brief      To process the Get Port List MRP
**
**              This MRP returns a list of ports.  This list may be
**              subsetted with only initialized or failed ports.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DI_GetPortList(MR_PKT *pMRP)
{
    UINT16              port;           /* Port Number                      */
    UINT16              *pList;         /* pointer to returned list         */
    UINT16              count = 0;      /* number of ports                  */
    UINT16              maxCount;       /* Max. number of devices in list   */
    UINT32              bitPort;        /* MAXISP - Bit mask for a port     */
    UINT16              requestType;    /* Request Type from MRP            */
    UINT8               retStatus = DEOK;
    MRGETPORTLIST_REQ   *mpp;

    /* Get pointer to Parm block address */
    mpp = (MRGETPORTLIST_REQ *)pMRP->pReq;
    requestType = mpp->type;

    /* First, grab the return data address and length allowed. */
    pList = (UINT16 *)(pMRP->pRsp + 1);
    pMRP->pRsp->rspLen = pMRP->rspLen;
    maxCount = ((UINT16)pMRP->rspLen - sizeof(MR_RSP_PKT)) / sizeof(UINT16);

    /* Check for valid option */
#ifdef FRONTEND
    if (requestType > PORTS_STATUS &&
        requestType != PORTS_WITH_TARGETS &&
        requestType != PORTS_NO_TARGETS)
#else   /* FRONTEND */
    if (requestType > PORTS_OFFLINE)
#endif  /* FRONTEND */
    {
        /* Invalid Option */
        retStatus = DEINVOPT;
    }
    else
    {
        if (requestType == PORTS_STATUS)
        {
            /* Initialize the port list for no ports. */
            for (count = 0; count < maxCount; ++count)
            {
                pList[count] = 0;
            }

            /*
             * The list has four entries, the first for the
             * valid ports, the second for the good ports,
             * third for the online ports, and the fourth
             * entry to indicate if initialization is complete.
             */
            count = 4;

            /* Make sure we have enough space for the return data. */
            if (count > maxCount)
            {
                /* Set the return status to "too much data". */
                retStatus = DETOOMUCHDATA;
            }
        }

        /* Scan all posible ports. */
        for (port = 0; port < ispmax; ++port)
        {
            /* Check if port exists. */
#if FE_ISCSI_CODE
            if(fsl_PortType(port) || (ispstr[port] != NULL))
#else   /* FE_ISCSI_CODE */
            if (ispstr[port] != NULL)
#endif /*FE_ISCSI_CODE*/
            {
                /* Generate a bit mask for this port */
                bitPort = 1 << port;

                /* IS the port status request? */
                if (requestType == PORTS_STATUS)
                {
                    if (retStatus == DEOK)
                    {
                        /*
                         * Store the bit corresponding to the port number
                         * in the return packet.
                         */
                        pList[0] |= bitPort;

                        /*
                         * Check if the port is GOOD, that is the port is
                         * not failed and the port has not been offline
                         * long enough to be consider failed.
                         */
                        if ((ispfail & bitPort) == 0 && (ispofflfail & bitPort) == 0)
                        {
                            pList[1] |= bitPort;
                        }

                        /* Check if the port is online. */
                        if ((ispOnline & bitPort) != 0)
                        {
                            pList[2] |= bitPort;
                        }

#ifdef FRONTEND
                        if ((K_ii.status & (1<<II_CINIT)) == 0)
                        {
                            /*
                             * Indicate cache initialization is in progress.
                             * Since the system cannot process I/O until the
                             * cache is initialized, the CCB is informed
                             * of this state to decide when to fail-over/back.
                             */
                            pList[3] = 1;
                        }
                        else
#endif  /* FRONTEND */
                        {
                            /* Indicate cache initialization is complete. */
                            pList[3] = 0;
                        }
                    }
                    continue;
                }

                /* Check for initialized port. */
                else if (requestType == PORTS_INITIALIZED)
                {
                    /* Skip if not initialized. */
                    if ((isprena & bitPort) == 0)
                    {
                        continue;
                    }
                }

                /* Check for failed port. */
                else if (requestType == PORTS_FAILED)
                {
                    /* Skip if not failed. */
                    if ((ispfail & bitPort) == 0)
                    {
                        continue;
                    }
                }

                /*
                 * Check for port have not been initialzed or are in
                 * the process of being initialized.
                 */
                else if (requestType == PORTS_INITIALIZING)
                {
                    /*
                     * Skip if not initialzed or in process of being
                     * initialized, i.e. skip if not enabled.
                     */
                    if ((isprena & bitPort) != 0)
                    {
                        continue;
                    }
                }

                /*
                 * Check for good ports i.e ports not failed and that
                 * have not failed the loop down check.
                 */
                else if (requestType == PORTS_GOOD)
                {
                    /*
                     * Skip if port is failed or has been offline long
                     * enough to be consider failed.
                     */
                    if ((ispfail & bitPort) != 0 || (ispofflfail & bitPort) != 0)
                    {
                        continue;
                    }
                }
                /*
                 * Check for port that is initialzed and the loop is
                 * not down; i.e. the loop is up.
                 */
                else if (requestType == PORTS_ONLINE)
                {
                    /* Skip if port is offline or not initialized. */
                    if ((ispOnline & bitPort) == 0 || (isprena & bitPort) == 0)
                    {
                        continue;
                    }
                }

                /* Check for port that is initialzed and the loop is down. */
                else if (requestType == PORTS_OFFLINE)
                {
                    /* Skip if port is online or not initialized. */
                    if ((ispOnline & bitPort) != 0)
                    {
                        continue;
                    }
                }

                /* Check for port that are tagged as failed. */
                else if (requestType == PORTS_FAILMARK)
                {
                    /* Skip if port is not tagged as failed. */
                    if (ispFailedPort[port] == FALSE)
                    {
                        continue;
                    }
                }

#ifdef FRONTEND
                /* Check for port with any targets. */
                else if (requestType == PORTS_WITH_TARGETS)
                {
                    /*
                     * Does this port have control port configured?
                     * Check for a valid target number.  Note that
                     * control ports use target number of 0xFFFF.
                     */
                    if ((tar[port] != NULL) && ((tar[port]->tid >= MAX_TARGETS) || (!BIT_TEST(ispOnline, port))))
                    {
                        continue;
                    }
                }

                /* Check for port without any targets. */
                else if (requestType == PORTS_NO_TARGETS)
                {
                    /*
                     * Does this port have target configured?
                     * Check for a valid target number.  Note that
                     * control ports use target number of 0xFFFF.
                     */
                    if ((tar[port] != NULL) && (tar[port]->tid < MAX_TARGETS) && (BIT_TEST(ispOnline, port)))
                    {
                        continue;
                    }
                }
#endif  /* FRONTEND */

                /*
                 * Check if the count exceeds the amount of data that is
                 * allowed to be returned.
                 */
                if (count > maxCount)
                {
                    /* Set the return status to "too much data". */
                    retStatus = DETOOMUCHDATA;
                }
                else
                {
                    /* Store the port number in the return packet. */
                    pList[count] = port;
                }

                /* Increment the port count */
                ++count;
            }
        }
    }

    /* Store the number of devices found in the return packet. */
    pMRP->pRsp->nDevs = count;
    pMRP->pRsp->status = retStatus;

    return retStatus;
} /* DI_GetPortList */

/**
******************************************************************************
**
**  @brief      Process a Loop Primitive MRP
**
**              This function is called when a Loop Primitive MRP is sent.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/

UINT8 DI_LoopPrimitive(MR_PKT *pMRP)
{
#ifdef BACKEND
    PDD                 *pdd = NULL;    /* Pointer to a PDD */
    UINT32              i;              /* Loop counter */
#endif  /* BACKEND */
    UINT8               port = 0;       /* Port Number */
    UINT16               lid = 0;        /* Loop ID */
    MRLOOPPRIMITIVE_REQ *mlp;           /* Pointer to the input of the MRP */
    UINT32              qStatus = 0;
    UINT8               retStatus = DEOK;

    /*
    ** Get a pointer to the MRP's parameter address
    */
    mlp = (MRLOOPPRIMITIVE_REQ *) pMRP->pReq;

    if (mlp->option == MLPRESLOOP ||
        mlp->option == MLPRESLIDPORT ||
        mlp->option == MLPLOGINLID ||
        mlp->option == MLPLOGOUTLID ||
        mlp->option == MLPTRSTLID ||
        mlp->option == MLPLPBLID ||
        mlp->option == MLPLPELID ||
        mlp->option == MLPDRPLID ||
        mlp->option == MLPINITLIP)
    {
        /*
        ** Get port number and Lid from the input packet.
        */
        port = mlp->port;
        lid  = mlp->lid;

        if (port > ispmax)
        {
            /*
            ** The specified port is not valid.
            */
            retStatus = DEINVCHAN;
        }
#if FE_ISCSI_CODE
        if(BIT_TEST(iscsimap, port))
        {
            retStatus = DEINVOPT;
        }
#endif  /* FE_ISCSI_CODE */
    }
#ifdef BACKEND
    else if (mlp->option == MLPSIDPIDRES ||
        mlp->option == MLPLOGINPID ||
        mlp->option == MLPLOGOUTPID ||
        mlp->option == MLPTRSTPID ||
        mlp->option == MLPLPBPID ||
        mlp->option == MLPLPEPID ||
        mlp->option == MLPDRPPID)
    {
        /*
        ** Get the pdd pointer from the pdd table
        */
        pdd = P_pddindx[ mlp->id ];

        if (pdd != NULL)
        {
            /*
            ** Get port number and Lid from the PDD.
            */
            port = pdd->channel;
            lid = pdd->id;
        }
        else
        {
            /*
            ** The specified physical device does not exist.
            */
            retStatus = DENONXDEV;
        }
    }
#endif  /* BACKEND */

    else
    {
        /*
        ** The specified option is not valid.
        */
        retStatus = DEINVOPT;
    }

    if (retStatus == DEOK)
    {
        /*
        ** Check if this QLogic port is enabled.
        */
        if (port < ispmax && BIT_TEST(isprena, port))
        {
            /*
            ** Based upon what the option is, run different sections of code
            */
            switch (mlp->option)
            {
                case MLPRESLOOP:
                    /*
                    ** LIP Reset an entire loop (e.g. LIP(FF,al_ps))
                    */
                    qStatus = ISP_LipReset(port, NO_LID);
                    break;

                case MLPRESLIDPORT:
                case MLPSIDPIDRES:
                    /*
                    ** LIP Reset a port (e.g. LIP(al_pd,al_ps))
                    */
                    qStatus = ISP_LipReset(port, lid);
                    break;

                case MLPINITLIP:
                    /*
                    ** Initiate LIP on a loop (e.g. LIP(F7,al_ps))
                    */
                    qStatus = ISP_initiateLip(port);
                    break;

                case MLPLOGINLID:
                case MLPLOGINPID:
                    /*
                    ** Login a port (loop or fabric)
                    */
                    qStatus = ISP_Login(port, lid);
                    break;

                case MLPLOGOUTLID:
                case MLPLOGOUTPID:
                    /*
                    ** Logout a port specified by a LID
                    */
                    qStatus = ISP_PortLogout(port, lid);
                    break;

                case MLPTRSTLID:
                case MLPTRSTPID:
                    /*
                    ** Send target reset to specified LID
                    */
                    qStatus = ISP_TargetReset(port, lid);
                    break;

                case MLPLPBLID:
                case MLPLPBPID:
                    /*
                    ** Send Loop Port Bypass to specified LID
                    */
                    qStatus = ISP_LoopPortBypass(port, lid);
                    break;

                case MLPLPELID:
                case MLPLPEPID:
                    /*
                    ** Send Loop Port Enable to specified LID
                    */
                    qStatus = ISP_LoopPortEnable(port, lid);
                    break;
#ifdef BACKEND
                case MLPDRPLID:
                {
                    /*
                    ** Find PID for this LID.
                    */
                    for (i = 0; i < MAX_PHYSICAL_DISKS; ++i)
                    {
                        /*
                        ** Is this the right LID and port.
                        */
                        if (P_pddindx[ i ]->pDev->pLid[port] == lid)
                        {
                            pdd = P_pddindx[ i ];

                            /*
                            ** Set the port as unavailable for
                            ** device with specified port and LID.
                            */
                            pdd->pDev->pLid[port] = NO_CONNECT;
                            pdd->pDev->sLid[port] = NO_CONNECT;
                            break;
                        }
                    }

                    /*
                    ** Was a PDD found?
                    */
                    if (pdd == NULL)
                    {
                        /*
                        ** The specified physical device does not exist.
                        */
                        retStatus = DENONXDEV;
                    }
                }
                break;

                case MLPDRPPID:
                {
                    /*
                    ** Scan through all the ports.
                    */
                    for (i = 0; i < MAX_PORTS; ++i)
                    {
                        /*
                        ** Check if this port is currently in use.
                        */
                        if (i != pdd->pDev->port)
                        {
                            /*
                            ** Set the port as unavailable when
                            ** not currently the active path
                            */
                            pdd->pDev->pLid[i] = NO_CONNECT;
                        }
                        /*
                        ** Set the secondary path as not connected.
                        */
                        pdd->pDev->sLid[i] = NO_CONNECT;
                    }
                }
                break;
#endif  /* BACKEND */
                default :
                    retStatus = DEINVOPT;
                    break;
            }

            /*
            ** Check if the ISP command completed sucessfully.
            */
            if ((qStatus & 0xFF) != 0)
            {
                retStatus = DEFAILED;
            }
        }
        else
        {
            /*
            ** Channel is not valid
            */
            retStatus = DEINVCHAN;
        }
    }

    return (UINT8)retStatus;
} /* DI_LoopPrimitive */

/**
******************************************************************************
**
**  @brief      To provide a standard means of retrieving the statistic and
**              configuration data from the FE loop statistics information area.
**
**              This function will dump the statistical data
**              from the loop stats information area.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/

UINT8 DI_PortStats(MR_PKT* pMRP)
{
    MRPORT_REQ * mfl;      /* Pointer to input MRP                 */
    UINT8  port;           /* Port Number                          */
    struct MRPORT_RSP * stats;
    UINT32 bitMask;        /* MAXISP - Bit mask for a port         */
#ifdef FRONTEND
    UINT32 maxTargets;
    struct TAR * pTar;
#endif  /* FRONTEND */
    UINT8  retStatus = DEOK;

    /* Get pointer to Parm block address */
    mfl = (MRPORT_REQ *)pMRP->pReq;

    /* Get pointer to return data and set return data length. */
    stats = (MRPORT_RSP *)pMRP->pRsp;
    stats->header.len = sizeof(*stats);

    /* Get port number from the input packet. */
    port = mfl->port;

    if ((port >= MAX_PORTS) || (isprev[port] == NULL))
    {
        /* Invalid Port.  */
        retStatus = DEINVCHAN;
    }
    else if (pMRP->rspLen < stats->header.len)
    {
        /* There is too much data for the return packet. */
        retStatus = DETOOMUCHDATA;
    }
    else
    {
#ifdef FRONTEND
        if (cimtDir[port] == NULL)
        {
            return DEINVCHAN;
        }
        /* Point to the correct CIMT and grab the number of hosts. */
        stats->numhosts = (UINT8)cimtDir[port]->numHosts;
#endif  /* FRONTEND */

        /* Get the primary port's Loop ID. */
        stats->lid = ispLid[port];

        /* Get ISP revision information. */
        *((ISP_REV *) &stats->rev) = *isprev[port];

        /* Generate a it mask for this port */
        bitMask = 1 << port;

        /*
         * Clear the area where the link status data (RLS ELS)
         * and port state are stored.
         */
        memset(&stats->rls, 0, sizeof(stats->rls));
        stats->state = 0;

        /* Check for failed port. */
        if ((ispfail & bitMask) != 0)
        {
            /* This port is failed. */
            stats->state = PORT_FAIL;
        }
        /* Check for Initialized port. */
        else if ((isprena & bitMask) == 0)
        {
            /* This port is not initialized. */
            stats->state = PORT_UNINT;
        }
        else if ((ispofflfail & bitMask) != 0)
        {
            /* This port has been offline too long. */
            stats->state = PORT_OFFLINEFAIL;
        }
        /* Check for Offline port (loop down). */
        else if ((ispOnline & bitMask) == 0)
        {
            /* This port is offline (loop down). */
            stats->state = PORT_OFFLINE;
        }
#if FE_ISCSI_CODE
        else if (!BIT_TEST(iscsimap, port)
                        && ((mfl->option & PORT_STATS_RLS) != 0))
#else   /* FE_ISCSI_CODE */
        else if ((mfl->option & PORT_STATS_RLS) != 0)
#endif  /* FE_ISCSI_CODE */
        {
            /* Get link status data */
            ISP2400_GetLinkStatistics(port, &stats->rls);
        }

        /* Clear the number of targets before counting them. */
        stats->numtarg = 0;

#ifdef FRONTEND

        /* Calculate the maximum target that will fit in the return packet. */
        maxTargets = (pMRP->rspLen - sizeof(*stats)) / sizeof(stats->target[0]);

        /*
         * Traverse the target list for this port.  Place each target ID
         * into the list if it fits.  If it does not fit, change the error code
         * to indicate such, but continue to parse the list so that the full
         * count can be obtained in order for the caller to call again with the
         * correct size.
         */
        for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
        {
            /*
             * Check for a valid target number.  Note that
             * control ports use target number of 0xFFFF.
             */
            if (pTar->tid < MAX_TARGETS && (pTar->opt & (1 << TARGET_ENABLE)) != 0)
            {
                /* Check if there is room in the packet. */
                if (stats->numtarg < maxTargets)
                {
                    /* Copy the target number to the packet. */
                    stats->target[stats->numtarg] = (UINT8) pTar->tid;

                    /* Increase length of data in packet for this target. */
                    stats->header.len += sizeof(stats->target[0]);
                }
                else
                {
                    /* There is too much data for the return packet. */
                    retStatus = DETOOMUCHDATA;
                }

                /* Increment the target count. */
                ++stats->numtarg;
            }
        }
#endif  /* FRONTEND */
        /* Fetch the GPIOD settings (especially the sensed J2 value in bit 2) */
#if FE_ISCSI_CODE
        if (!BIT_TEST(iscsimap, port))
#endif  /* FE_ISCSI_CODE */
        {
            stats->GPIOD = (UINT8)ispGPIOD[port];
        }
    }

    /* Set the return status in the return packet. */
    stats->header.status = retStatus;

    return retStatus;
} /* DI_PortStats */


/**
******************************************************************************
**
**  @brief      Set configuration of FC ports.
**
**              This function will take the information in the input parameters
**              to set the configuration of the FC ports.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DI_SetPortConfig(MR_PKT *mrp)
{
    MRCONFIG_REQ    *req = (MRCONFIG_REQ *)mrp->pReq;

    fprintf(stderr, "%s: Setting FC port configuration\n", __func__);
    config = req->config;

    config_set = TRUE;

    mrp->pRsp->rspLen = mrp->rspLen;    /* Set the return data length */

    return DEOK;
}


/**
******************************************************************************
**
**  @brief      Wait for configuration of FC ports.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void WaitPortConfig(void)
{
    fprintf(stderr, "%s: Waiting for port configuration\n", __func__);
    while (!config_set)
    {
        TaskSleepMS(10);
    }
    fprintf(stderr, "%s: Ports configured\n", __func__);
}


/**
******************************************************************************
**
**  @brief      Get port configuration
**
**  @param      port - Port number
**
**  @return     Configuration value
**
******************************************************************************
**/
UINT8   GetPortConfig(UINT8 port)
{
    if (!config_set)
    {
        fprintf(stderr, "%s: Port %d configuration not set!\n",
                __func__, port);
        return ISP_CONFIG_DEF;
    }

    if (port >= config.count || config.count > ISP_MAX_CONFIG_PORTS)
    {
        fprintf(stderr, "%s: Port (%d) or count (%d) out of range\n",
                __func__, port, config.count);
        return ISP_CONFIG_DEF;
    }

    return config.config[port / 2];
}

/**
******************************************************************************
**
**  @brief      To provide a means of reseting QLogic chips.
**
**              This function will take the information in the input parameters
**              which specifies the channel number of the Qlogic chip to reset.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/

UINT8 DI_ResetPort(MR_PKT *pMRP)
{
    UINT8              port;
    UINT8              reason;
    MRRESETPORT_REQ    *mfl;

    /* Set the return data length. */
    pMRP->pRsp->rspLen = pMRP->rspLen;

    /* Get pointer to Parm block address */
    mfl = (MRRESETPORT_REQ *)pMRP->pReq;

    /* Get the port number and reason code from the input packet. */
    port = mfl->port;
    reason = mfl->option;

#ifdef FRONTEND
    if (mfl->port == RESET_PORT_NEEDED)
    {
        /* Dump the Qlogic registers and reset the port */
        ISP_FindPortToReset(TRUE);

#if FE_ICL
        /* Reset the ICL port, if it is controller failure */
        ICL_Offline();
#endif  /* FE_ICL */
        return DEOK;
    }
    else
#endif  /* FRONTEND */

    /* Check if resetting all ports. */
    if (mfl->port == RESET_PORT_ALL)
    {
        /* Scan all possible ports to find those that exist. */
        for (port = 0; port < MAX_PORTS; ++port)
        {
            /* Check if the port is enabled. */
            if (isprev[port] != NULL && (resilk & (1<<port)) == 0)
            {
                if (reason == RESET_PORT_MEMDUMP)
                {
#if FE_ISCSI_CODE
                    if(BIT_TEST(iscsimap, port))
                    {
                        return DEINVOP;
                    }
#endif  /* FE_ISCSI_CODE */
                    /* Dump the Qlogic registers and reset the port */
                    ISP_DumpQL(port, 0);
                }
                else if (reason == RESET_PORT_FORCE_SYS_ERR)
                {
#if FE_ISCSI_CODE
                    if(BIT_TEST(iscsimap, port))
                    {
                        return DEINVOP;
                    }
#endif  /* FE_ISCSI_CODE */

                    /* Force a system error. */
                    if (ISP_ForceSystemError(port) != ISP_CMDC)
                    {
                        /* The ISP operation failed. */
                        return DEFAILED;
                    }
                }
                else if (reason == RESET_PORT_NO_INIT)
                {
                    /*
                     * Reset the port and do not re-initialize.
                     * Mark this port as failed so it won't assume any targets,
                     * in case recovery causes it to be re-initialized.
                     */
                    ispFailedPort[port] = TRUE;
                    if (ISP_ResetChip(port, reason) != 0)
                    {
                        /* The ISP reset operation failed. */
                        return DEQRESETFAILED;
                    }
                }
                else
                {
                    if (ISP_ResetChip(port, reason) != 0)
                    {
                        /* The ISP reset operation failed. */
                        return DEQRESETFAILED;
                    }
                }
            }
        }
        return DEOK;
    } /* resetting all ports */

    /* Check if the port exists and is not already being reset. */
    if (port >= MAX_PORTS || isprev[port] == NULL)
    {
        /* The specified port is not valid.  */
        return DEINVCHAN;
    }

    if ((resilk & (1<<port)) != 0)
    {
        /* A reset operation is already in progress. */
        return DEOUTOPS;
    }

    if (reason == RESET_PORT_MEMDUMP)
    {
#if FE_ISCSI_CODE
        if(BIT_TEST(iscsimap, port))
        {
            return DEINVOP;
        }
#endif
        /* Dump the Qlogic registers and reset the port */
        ISP_DumpQL(port, 0);
        return DEOK;
    }

    if (reason == RESET_PORT_FORCE_SYS_ERR)
    {
#if FE_ISCSI_CODE
        if(BIT_TEST(iscsimap, port))
        {
            return DEINVOP;
        }
#endif
        /* Force a system error */
        if (ISP_ForceSystemError(port) != ISP_CMDC)
        {
            /* The ISP operation failed. */
            return DEFAILED;
        }
        return DEOK;
    }

    if (reason == RESET_PORT_NO_INIT)
    {
        /*
         * Reset the port and do not re-initialize.
         * Mark this port as failed so it won't assume any targets,
         * in case recovery causes it to be re-initialized.
         */
        ispFailedPort[port] = TRUE;
        if (ISP_ResetChip(port, reason) != 0)
        {
            /* The ISP reset operation failed. */
            return DEQRESETFAILED;
        }
        return DEOK;
    }

    /* Reset the specified port. */
    if (ISP_ResetChip(port, reason) != 0)
    {
        /* The ISP reset operation failed. */
        return DEQRESETFAILED;
    }
    return DEOK;
} /* DI_ResetPort */

/**
******************************************************************************
**
**  @brief      Process the Set Port Event Notification MRP
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DI_SetPortEventNotification(MR_PKT *pMRP)
{
    MRFEPORTNOTIFY_REQ *inMpn;

    /*
    ** Get pointer to Parm block address
    */
    inMpn = (MRFEPORTNOTIFY_REQ *)pMRP->pReq;

    /*
    ** Copy the data from the MRP.
    */
    mpn = *inMpn;
#ifdef FRONTEND
    mpn.loopDownToNotify = 10000;     /**< Loop down to notify                    */
    mpn.resetToNotify    = 7000;      /**< Reset to notify                        */
#endif

    /*
    ** Set good return status
    */
    pMRP->pRsp->status = DEOK;

    return DEOK;
}

#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      Put Regular Port on FE Fabric
**
**              This function will determine if Cache has initialized
**              successfully yet.  If so, the regular ports will be put on the
**              FE Fabric and the control ports removed (During Cache
**              Initialization, only Control Ports are allowed on the FE).
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DI_FEPortGo(MR_PKT *pMRP UNUSED)
{
    UINT8              retStatus = DEOK;

    /*
    ** Only put on the Control Ports if Cache has completed initialization.
    */
    if ((K_ii.status & II_STATUS_CINIT) != 0)
    {
        /*
        ** Let the other code know it is OK now to put regular targets on the
        ** FE ports.
        */
        gRegTargetsOK = TRUE;

        /*
        ** All appears OK, put the assigned targets onto the FE ports
        */
        ISP_FindPortToReset(FALSE);
    }
    else
    {
        /*
        ** Cache has not completed initialization, return Busy
        */
        retStatus = DEBUSY;
    }

    return retStatus;
}
#endif /* FRONTEND */


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
