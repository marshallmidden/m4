/* $Id: icl.c 145879 2010-08-18 18:48:06Z m4 $ */

/**
******************************************************************************
**
**  @file       icl.c
**
**  @brief      InterController Link (ICL) 'C' functions
**
**  This contains function related to ICL target creation and management.
**
**  Copyright (c) 2006-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <byteswap.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/sockios.h>

#include "def.h"
#include "deffe.h"
#include "datagram.h"
#include "dtmt.h"
#include "mlmt.h"
#include "ficb.h"
#include "fsl.h"
#include "icl.h"
#include "isp.h"
#include "ilt.h"
#include "loop.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "misc.h"
#include "options.h"
#include "OS_II.h"
#include "pcb.h"
#include "pm.h"
#include "xl.h"
#include "XIO_Types.h"
#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "mem_pool.h"
#include "CT_defines.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define ICL_DLMPATHALGO_ROUNDROBIN    0
#define ICL_DLMPATHALGO_ICL50PERCENT  1
#define ICL_DLMPATHALGO_ICL100PERCENT 2
#define ICL_DLMPATHALGO_DISPLAY       3

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/
UINT32 dlmPathStat[5][5];

/*
******************************************************************************
** Private variables
******************************************************************************
*/
UINT32 iclIdentificationDone = 0;
char   iclIfrName[5] = "icl0"; /* equivalent to p2n[] of other interfaces */

ICL_TGD_INFO  icl_TgdInfo[]=
           {
             {ICL_TID0, 0xc0a8c896 /* 192.168.200.150 */, 0xFFFFFF00 /* 255.255.255.0 */ }, /* cntrl#0 */
             {ICL_TID1, 0xc0a8c8fa /* 192.168.200.250 */, 0xFFFFFF00 /* 255.255.255.0 */ }  /* cntrl#1 */
           };
typedef DTMT *(*DlmPathHandlerFuncP)(ILT *);

DTMT *pIclDtmt = NULL;

UINT8  configDlmPathAlgo = ICL_DLMPATHALGO_ROUNDROBIN; //default value
UINT8  currentDlmPathAlgo = ICL_DLMPATHALGO_ROUNDROBIN;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern INT32 tsl_netlink_init(void);
extern UINT8 iscsiGenerateParameters(I_TGD *pParamSrc);
extern void ISP_GenOffline(UINT32);
extern void     C_recv_scsi_io(ILT *ilt);
extern void     I_recv_rinit(UINT32 port);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
void ICL_CreateItgdInfo(TGD *pTGD, int cNo);
void ICL_CreateIclTarget(UINT32 cserial, UINT16 tid, int cNo);
UINT64 ICL_getpname(UINT32 cserial, UINT8 port);
UINT64 ICL_getNodeName(UINT32 cserial, UINT8 port);
UINT8 ICL_GetDlmPathStats(MR_PKT *pMRP);
UINT8 ICL_DlmPathSelectionAlgorithm(MR_PKT *pMRP);
DTMT *ICL_GetDlmPathThruRoundRobin(ILT *pILT);
DTMT *ICL_GetDlmPathThruWeightedRR_1(ILT *pILT);
DTMT *ICL_GetDlmPathThruWeightedRR_2(ILT *pILT);
void   ICL_SetDlmPathAlgorithm(void);
DTMT *ICL_FindNextDtmt(MLMT *pMLMT, DTMT *pDtmt, UINT8 *pStartOverFlag);
DTMT *ICL_GetNextDtmt(MLMT *pMLMT, DTMT *pDtmt, UINT8 *pStartOverFlag);

DlmPathHandlerFuncP ICL_DlmPathSelectionHandler = ICL_GetDlmPathThruRoundRobin;

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      This function checks whether the ICL port is existing on the
**              system. This does not check whether the port is up or down.
**              This gets called from kernel, before creating the ICL target.
**              This in turn calls the tsl module provided function to identify
**              the ICL interface. The tsl function sets the global variable
**              iclPortExists to TRUE, if the ICL interface is existing.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void ICL_CheckIclExistence(void)
{
   iclIdentificationDone = 0;
   tsl_netlink_init();
   iclIdentificationDone = 1;
}

/**
******************************************************************************
**
**  @brief      This function creates the icl targets if the icl port exists.
**              The TGD structures are allocated and filled with relevant
**              information in all the members for both the master and slave
**              controllers.  It also allocates an iTGD structure, initializes
**              it and links it with TGD. It also invokes the iscsii text
**              module provided function to generate the target parameter table
**              for the ICL target.
**
**              This function gets called from kernel.as (during startup) for
**              each of the controller. And each ICL target's information is
**              created in both the controllers. Since the ICL configuration is
**              not needed to be saved in NVRAM, for every powerup the ICL target
**              is created and initialized. ICL is also like anyother iSCSI
**              target , but used only for communication between the two con-
**              trollers of a DSC. Hence only the required fileds are initia-
**              lised.
**
**              Also, in contrast to the iSCSI/FC targets, the ICL target is
**              created and initilised only in the Front End Processor. Also
**              the ICL target is  stored in the global target table making
**              it visible for all the target configuration related functions
**              in FE. Since BE does not contain the ICL target information
**              either user or CCB can't see it. Only the FE contains this
**              information.
**
**              This gets called from the kernel, after ICL interface identi-
**              cation, but before starting up the ISP.
**
**  @param      none
**
**  @return     none
**
**  @attention
**            The ICL target configuration creation/initialisation func-
**            tionality is taken from DEF_CreateCtrl(defbe.c),
**            ON_CreateDefaults(online.as), DEF_ValidateTarget(defbe.c),
**            D_updrmttarg(definebe.as), o$createtargetWWN(),
**            DEF_ConfigTar(deffe.c), DEF_CreateTargetInfo(def_iscsi.c),
**            DEF_UpdRmtTgInfo(def_iscsi.c), DEF_UpdateTgInfo(def_iscsi.c)
**            and iscsiGenerateParamters(iscsi_text.c)
******************************************************************************
**/
void ICL_CreateIclTargets(void)
{
    int cNo;
    UINT32 cserial;
    UINT32 vcgID;

    if(iclPortExists)
    {
#if ICL_DEBUG
    fprintf(stderr, "<%s:%s>ICL exists, creating the ICL targets\n", __FILE__, __func__);
#endif
        vcgID = K_ficb->vcgID;
        if(vcgID == 0)
        {
            vcgID = (K_ficb->cSerial)>>4;
        }

        /*
        ** Create ICL targets related to   both the controllers
        */
        for (cNo = 0; cNo < 2; ++cNo)
        {
            cserial = (vcgID << 4) | cNo;
            ICL_CreateIclTarget(cserial, icl_TgdInfo[cNo].tid, cNo);
        }
    }
}
/**
******************************************************************************
**
**  @brief      This function creates the ICL target and initializes it for the
**              specified controller.
**
**  @param      cserial - Controller serial number
**              tid     - ICL target ID (8 for master - 9 for slave)
**              cNo     - Controller sequential number (0 or 1)
**
**  @return     none
**
******************************************************************************
**/
void ICL_CreateIclTarget(UINT32 cserial, UINT16 tid, int cNo)
{
    TGD *pTGD;


    /*
    ** Allocate the ICL target instance and fill the structure.
    */
    pTGD = DEF_AllocTarg();
    gTDX.tgd[tid] = pTGD;
    gTDX.count++;

    pTGD->tid = tid;
    pTGD->port = ICL_PORT;
    pTGD->prefPort = ICL_PORT;
    pTGD->altPort  = ICL_PORT;
    pTGD->prefOwner = cserial;
    pTGD->owner     = cserial;
    pTGD->cluster   = NO_CLUSTER;

    /*
    ** Set world wide names for the target.
    */
    pTGD->portName = ICL_getpname(pTGD->owner, pTGD->port);
    pTGD->nodeName = ICL_getNodeName(pTGD->owner, pTGD->port);

    /*
    ** Enable the target.
    */
    pTGD->opt = 0;
    BIT_SET(pTGD->opt, TARGET_ENABLE);

    /*
    ** Set target type  -- this is iSCSI type target, but  used for
    ** ICL only.
    */
    BIT_SET(pTGD->opt, TARGET_ISCSI);
    BIT_SET(pTGD->opt, TARGET_ICL);

    /*
    ** Create Target information (fill iTGD)
    */
    ICL_CreateItgdInfo(pTGD, cNo);

    /*
    ** Set IP Address/Prefix, gateway in TGD from iTGD.
    */
    pTGD->ipPrefix = MSC_Mask2Prefix(bswap_32(pTGD->itgd->ipMask));
    pTGD->ipAddr   = pTGD->itgd->ipAddr;
    pTGD->ipGw     = pTGD->itgd->ipGw;

     /*
     ** Generate iSCSI parameter tables
     */
     iscsiGenerateParameters(pTGD->itgd);

#if ICL_DEBUG
    fprintf(stderr, "<%s:%s>ICL..target is created successfully CNo=%u tid =%u\n",
                   __FILE__, __func__, cserial, (UINT32)tid);
#endif
}

/**
******************************************************************************
**
**  @brief      This function creates the iSCSI target information for the
**              ICL target.This information is fixed and not user configurable.
**              We may need to change these static values in the code itself,
**              if necessary.
**
**  @param      pTgd -- ICL target poiter
**              cNo  -- Controller sequential number (0 or 1)
**
**  @return     none
**
**  @attention  Equivalent to DEF_CreateTargetInfo (def_iscsi.c)

******************************************************************************
**/
void ICL_CreateItgdInfo(TGD *pTGD, int cNo)
{

    pTGD->itgd = (I_TGD *)s_MallocC(sizeof(I_TGD), __FILE__, __LINE__);
#if ICL_DEBUG
    fprintf(stderr, "<%s:%s>creating the ICL target Info with Ip etc.\n",
                   __FILE__, __func__);
#endif

    /*
    ** Update iSCSI/ICL target information  (itgd) with default values
    */
    pTGD->itgd->tid =  pTGD->tid;

    pTGD->itgd->ipAddr= bswap_32(icl_TgdInfo[cNo].ipAddr);

#if ICL_DEBUG
    fprintf(stderr, "<>ICL..Ip address=%x\n", bswap_32(pTGD->itgd->ipAddr));
#endif

    pTGD->itgd->ipMask= bswap_32(icl_TgdInfo[cNo].netMask);
#if ICL_DEBUG
    fprintf(stderr, "<>ICL..Ip mask=%x\n", bswap_32(pTGD->itgd->ipMask));
#endif

     pTGD->itgd->ipGw = 0x0;
#if ICL_DEBUG
    fprintf(stderr, "<>ICL..Gateway=%x\n", bswap_32(pTGD->itgd->ipGw));
#endif


    pTGD->itgd->maxConnections=1;
    pTGD->itgd->initialR2T=1;
    pTGD->itgd->immediateData=0;
    pTGD->itgd->dataSequenceInOrder=1;
    pTGD->itgd->dataPDUInOrder=1;
    pTGD->itgd->ifMarker=0;
    pTGD->itgd->ofMarker=0;
    pTGD->itgd->errorRecoveryLevel=0;

    /*
    ** GroupTag must be TID ???? <TBD>
    */
    pTGD->itgd->targetPortalGroupTag=1;
    pTGD->itgd->maxBurstLength=262144;
    pTGD->itgd->firstBurstLength=65536;
    pTGD->itgd->defaultTime2Wait=2;
    pTGD->itgd->defaultTime2Retain=20;
    pTGD->itgd->maxOutstandingR2T=1;
    pTGD->itgd->maxRecvDataSegmentLength=65536;
    pTGD->itgd->maxSendDataSegmentLength=8192;
    pTGD->itgd->ifMarkInt=0;
    pTGD->itgd->ofMarkInt=0;
    pTGD->itgd->headerDigest=0;
    pTGD->itgd->dataDigest=0;
    pTGD->itgd->authMethod=0;

    /*
    ** If ICL port does not need to support jumbo frames, set MTU SIZE as 1500
    ** (default),
    ** for the time being , assume it needs..hence set 9000
    */
    pTGD->itgd->mtuSize=1500;

    strcpy((char *)pTGD->itgd->tgtAlias, "");

    /*
    ** Mask is to prevent user to configure parameter whose support is not
    ** built in STACK  user cannot configure any of the parameters-mask is zero
    */
    pTGD->i_mask = 0x00;

    pTGD->itgd->numUsers = 0;
    pTGD->itgd->chapInfo = NULL;

}/* DEF_CreateTargetInfo */

/**
******************************************************************************
**
**  @brief      This function checks whether the given port is ICL port or not
**              If the port number is ICL_PORT (4) and the port exists flag set,
**              it returns TRUE otherwise FALSE.
**
**  @param      port -- port number
**
**  @return     TRUE or FALSE
**
******************************************************************************
**/
UINT32 ICL_IsIclPort (UINT8 port)
{
    if (iclPortExists && (ICL_PRT(port)))
    {
        return TRUE;
    }
    return FALSE;
}

/**
******************************************************************************
**
**  @brief      This function checks whether the MAG link VRP is related to ICL.
**
**  @param      pVRP -- Pointer to VRP
**
**  @return     TRUE / FALSE
**
******************************************************************************
**/
UINT32 ICL_IsIclMagLinkVRP(VRP *pVRP)
{
    UINT32 retVal = FALSE;

    if(BIT_TEST(pVRP->options, VRP_FEICL_MAGLINK) == TRUE)
    {
        retVal = TRUE;
    }
    return retVal;
}

/**
******************************************************************************
**
**  @brief      This function sets the MAG Link VRP as generated for ICL port.
**              The LLD module generates various VRPs to monitor/establish/
**              remove the paths between the Front End ports (iSCSI and ICL).
**              This special bit set in VRP enables the DLM module to know
**              that it is related to ICL port.
**
**  @param      pVRP --  Pointer to VRP
**
**  @return     none
**
******************************************************************************
**/
void ICL_SetIclMagLinkFlag(VRP *pVRP)
{
    BIT_SET(pVRP->options, VRP_FEICL_MAGLINK);
}

/**
******************************************************************************
**
**  @brief      This function clears the special bit indicating ICL in VRP.
**
**  @param      pVRP -- Pointer to VRP
**
**  @return     none
**
******************************************************************************
**/
void ICL_ClearIclMagLinkFlag(VRP *pVRP)
{
    if (BIT_TEST(pVRP->options, VRP_FEICL_MAGLINK))
    {
        BIT_CLEAR(pVRP->options, VRP_FEICL_MAGLINK);
    }
}

/**
******************************************************************************
**
**  @brief      This function checks whether the Datagram is related to VLink.
**
**  @param      Src controllerSN ,  Destination controllerSN
**
**  @return     TRUE / FALSE
**
******************************************************************************
**/
UINT32 ICL_IsVLinkDatagram(UINT32 srcCncSerial, UINT32 destCncSerial)
{
    UINT32 srcVcgID;
    UINT32 destVcgID;
    UINT32 retVal = FALSE;

    /*
    ** Extract VCG ID (DSC or Group) that the source controller belongs.
    */
    srcVcgID = srcCncSerial >> 4;

    /*
    ** Extract VCG ID (DSC or Group) that the destination  controller belongs.
    */
    destVcgID = destCncSerial >> 4;

    /*
    ** Check if the request (datagram) is between different DSCs..If so this
    ** is VLink DataGram.
    */
    if (srcVcgID != destVcgID)
    {
        retVal = TRUE;
    }
    return(retVal);
}

/**
******************************************************************************
**
**  @brief      Sends various ICL port event Log messages to CCB.
**
**  @param      portState
**
**  @return     none
**
******************************************************************************
**/
void ICL_LogEvent(UINT8 portState)
{
    LOG_ICL_EVENT_PKT iclLog;

    iclLog.header.event = LOG_ICL_PORT_EVENT;
    iclLog.data.portState = portState;

    /* Note: message is short, and L$send_packet copies into the MRP. */
    MSC_LogMessageStack(&iclLog, sizeof(LOG_ICL_EVENT_PKT));
}

/**
******************************************************************************
**
**  @brief      This function gets IP address of the specified interface.
**
**  @param      intfHandle -- pointer to interface name
**
**  @return     IP address
**
******************************************************************************
**/
UINT32 ICL_GetIfaceIpAddr( char *intfHandle)
{
    unsigned int  ipAddr = 0;
    int  rc;
    struct ifreq ifReq;
    int portSrvSocket;

    portSrvSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (portSrvSocket < 0)
    {
        fprintf(stderr, "GetIpAddressFromInterface: socket() failed\n");
        return 0;
    }
    memset(&ifReq, 0, sizeof(ifReq));
    strncpy(ifReq.ifr_name, intfHandle, IFNAMSIZ);

    rc = ioctl(portSrvSocket, SIOCGIFADDR, &ifReq);

    if (rc)
    {
        fprintf(stderr, "GetIpAddressFromInterface: ioctl() failed\n");
    }
    else
    {
        ipAddr = (unsigned int)(((struct sockaddr_in *)&ifReq.ifr_addr)->sin_addr.s_addr);
    }
    close(portSrvSocket);
    return ipAddr;              /* returns IP Addr in network order */
}

/**
******************************************************************************
**
**  @brief      This function gets the netmask of the interface.
**
**  @param      Pointer to the interface name
**
**  @return     Netmask
**
******************************************************************************
**/
UINT32 ICL_GetIfaceNetMask( char *intfHandle)
{
    unsigned int  netMask = 0;
    int  rc;
    struct ifreq ifReq;
    int portSrvSocket;

    portSrvSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (portSrvSocket < 0)
    {
        fprintf(stderr, "GetIpAddressFromInterface: socket() failed\n");
        return 0;
    }
    memset(&ifReq, 0, sizeof(ifReq));
    strncpy(ifReq.ifr_name, intfHandle, IFNAMSIZ);

    rc = ioctl(portSrvSocket, SIOCGIFNETMASK, &ifReq);

    if (rc)
    {
        fprintf(stderr, "GetIpAddressFromInterface: ioctl() failed\n");
    }
    else
    {
        netMask = (unsigned int)(((struct sockaddr_in *)&ifReq.ifr_netmask)->sin_addr.s_addr);
    }

    close(portSrvSocket);
    return netMask;                 /* returns netmask in network order */
}


/**
******************************************************************************
**
**  @brief      Gets the Gateway address of an interface
**
**  @param      Pointer to the interface name
**
**  @return     Gateway address
**
******************************************************************************
**/
UINT32 ICL_GetIfaceGateway( char *intfHandle)
{
    UINT32 ipAddr = 0;
    INT32 rc;

    FILE *pF;
    CHAR pLine[256]; /* this should be more than we need */
    CHAR pIntf[9];
    UINT32 dest;
    UINT32 gateway;

    /*
    ** Read and parse /proc/net/route to get default gateway
    */
    pF = fopen("/proc/net/route", "r");
    if (pF)
    {
        while ( fgets((char *)pLine, 256, pF))
        {
            rc = sscanf((char *)pLine, "%8s %x %x", pIntf, &dest, &gateway);

            if ((rc == 3) && (strcmp((char *)pIntf, intfHandle) == 0) && (dest == 0))
            {
                ipAddr = gateway;
                break;
            }
        }

        fclose(pF);
    }
    else
    {
        fprintf(stderr, "GetGatewayFromInterface: couldn't open /proc/net/route\n");
    }

    return ipAddr;
}

/**
******************************************************************************
**
**  @brief      Dumps the details of the path (DTMT) selecting for sending the
**              datagram to the other controller.
**
**  @param      pILT  --
**              pDTMT --
**
**  @return     none
**
******************************************************************************
**/
void ICL_SendDg_Dump1(ILT *pILT, DTMT *pDTMT)
{
    UINT32 destSN;  /* dest MAG serial available in DATAGRAM Pkt */
    UINT32 ficbSN;  /* my MAG serial from KICB */
    UINT32 destSN_le;
    UINT8  dtmtState;
    UINT32 lldid;
    DATAGRAM_REQ *pDatagramReq;
    ILT   *pILTlldmt;
    UINT8  lldmtChannel;

    pDatagramReq = (DATAGRAM_REQ *)((pILT-1)->ilt_normal.w0);
    destSN = pDatagramReq->dstSN;

    destSN_le = bswap_32(destSN);     /* little-endian */
    ficbSN = K_ficb->cSerial;
    dtmtState = pDTMT->state;
    pILTlldmt = pDTMT->pILTlldmt;     /* Associated Link Level Driver */
    lldmtChannel = (UINT8)(pILTlldmt->ilt_normal.w3); /* Channel/path associated with */

    fprintf(stderr, "<ICL_SendDg_Dump1> -----------------------------------------------------------\n");

    fprintf(stderr,
    "mySN=%x  destSN=%x tgt controller-path= %u tgt-serialNum=%x, this controller path=%u\n",
    ficbSN, destSN_le, pDTMT->td.xio.path, pDTMT->td.xio.serialNum, lldmtChannel);
    fprintf(stderr, "ipaddress=%x   icl-flag=%s state = %s\n",
            pDTMT->td.xio.ip, ((pDTMT->iclFlag) ? "TRUE" : "FALSE"),
            ((pDTMT->state == 0) ? "OPERATIONAL" : "Otherthan-OP"));
    fprintf(stderr, "<ICL_SendDg_Dump1> -----------------------------------------------------------\n");


    /*
    ** LTMT is stored in vr_mle_lldid of VRP in lld$send_MLE function.
    ** This is stored in dtmt_lldid member in dlm$MLE function at the
    ** time of DTMT creation.
    */
    lldid = pDTMT->lldid; /* LTMT address */

}

/**
******************************************************************************
**
**  @brief      Generates a Port Name for a given serial number and port  of
**              a controller.
**
**  @param      UINT32 - controller serial number
**              UINT8  - port
**
**  @return     UINT64 - Port Name
**
******************************************************************************
**/

UINT64 ICL_getpname(UINT32 cserial, UINT8 port)
{
    UINT64 pname;

    pname = ((UINT64)XIO_OUI << 24) | (cserial & 0xFFFFFF);
    pname |= (UINT64)(WWN_F_PORT | (port << 16)) << 32;
    pname = bswap_64(pname);
    return pname;
}

/**
******************************************************************************
**
**  @brief      Generates a Node  Name for a given serial number and port  of
**              controller
**
**  @param      UINT32 - controller serial number
**              UINT8  - port
**
**  @return     UINT64 - Port Name
**
******************************************************************************
**/

UINT64 ICL_getNodeName(UINT32 cserial, UINT8 port)
{
    UINT64 nname;

    nname = ((UINT64)XIO_OUI << 24) | (cserial & 0xFFFFFF);
    nname |= (UINT64)(WWN_F_NODE | (port << 16)) << 32;
    nname = bswap_64(nname);
    fprintf(stderr, "ISCSI_DEBUG: ICL_getNodeName = %llx\n", nname);
    return nname;
}

/**
******************************************************************************
**
**  @brief      This function resets the ICL port when it is down. This deallo-
**              cates all the related structures. ALso deletes the DTMT paths
**              related to this port.
**
**  @param      iclPort -- ICL port number (4)
**
**  @return     None
**
******************************************************************************
**/
void ICL_PortReset(UINT32 iclPort UNUSED)
{
    ILT *ilt;

    /*
    ** Generate reset/init event to the proc so that all the out-standing IMTs
    ** are deleted & also call the corresponding function in the initiator
    ** path to delete the IMTs on this port
    */
    ilt = get_ilt();                               /* get an ILT w/wait                */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    ilt->ilt_normal.w0 = ISP_RESET_INIT_CMD;       /* store command byte               */
    ilt->ilt_normal.w0 |= ICL_PORT << 8;           /* store Chip ID                    */
    ilt->ilt_normal.w1 = 0;                        /* store Initiator ID               */
    ilt->ilt_normal.w2 = 0;                        /* store Initiator ID               */
    ilt[1].misc = (UINT32)ilt;
    ilt->cr = NULL;                     /* No completion handler            */
    ++ilt;                              /* Get next level of ILT            */

    /*
    ** No completion routine, cdriver will release ilt
    */
    ilt->cr = NULL;                     /* No completion handler            */
    C_recv_scsi_io(ilt);

#if INITIATOR
#if ICL_DEBUG
    fprintf(stderr, "<%s>calling I_recv_rinit() for ICL port\n", __func__);
#endif
    /*
    ** Invoke Initiator level
    */
    I_recv_rinit(ICL_PORT);
#endif
}

/**
******************************************************************************
**
**  @brief   Upadtes the number of DLM packets transferred through a perticular
**           DLM path
**
**  @param   DTMT pointer
**
**  @return  none
**
******************************************************************************
**/
void ICL_UpdateDmlPathStats(DTMT *pDtmt)
{
    UINT32 lldmtChannel;
    UINT32 dmlPath;
    ILT   *pILTlldmt;

    pILTlldmt = pDtmt->pILTlldmt;     /* Associated Link Level Driver */
    lldmtChannel = (UINT8)(pILTlldmt->ilt_normal.w3); /* path associated with this controller */

    dmlPath=  pDtmt->td.xio.path;      /* path associated with target controller */
    dlmPathStat[lldmtChannel][dmlPath]++;
}

/**
******************************************************************************
**
**  @brief   MRP Handler for 'dlmpathstats' command
**
**  @param   MRP packet pointer
**
**
**  @return status
**
******************************************************************************
**/
UINT8 ICL_GetDlmPathStats(MR_PKT *pMRP)
{
    UINT16 index1=0;
    UINT16 thisCntPort;
    UINT16 tgtCntPort;
    MRDLMPATHSTATS_RSP *pRSP;

    pRSP = (MRDLMPATHSTATS_RSP *)(pMRP->pRsp);

    for (thisCntPort = 0; thisCntPort < MAX_PORTS+1; thisCntPort++)
    {
        for( tgtCntPort = 0; tgtCntPort < MAX_PORTS+1; tgtCntPort++)
        {
            (pRSP->pathStats)[index1++] = dlmPathStat[thisCntPort][tgtCntPort];
        }
    }

    pRSP->header.len = sizeof(MRDLMPATHSTATS_RSP);
    return DEOK;
}

/**
******************************************************************************
**
**  @brief      Sends ICL port loop down event to CCB. ALso Generate offline
**              event to ISP that ultimately logs out the sessions and delete
**              the FE paths from this port.
**
**  @param      port number   (ICL port number)
**
**  @return     None
**
**  @attention  Equivalent to ISP_LoopDown(ispc.c)
******************************************************************************
**/
void ICL_LoopDown(UINT32 port)
{
    /*
    ** Notify ICL path lost and reset dlm path selection algorithm
    */
    ICL_NotifyIclPathLost(pIclDtmt);

    /*
    ** Ensure the chip isn't being reset (which probably generated this event)
    */
    if ((resilk & (1<<port)) == 0)
    {
        ICL_LogEvent(LOG_ICLPORT_LOOPDOWN);
    }

    /*
    ** Generate offline event
    */
    ISP_GenOffline(port);

} /* ICL_LoopDown */

/**
****************************************************************************
**    Load Balancing Mechanism of DLM paths.
****************************************************************************
**/

/**
******************************************************************************
**
**  @brief     This function gets the path (DTMT) to the other controller for
**             sending the datagram. The DTMT selection will be done based on
**             various conditions like
**
**             (a) when specific path is requested
**             (b) Any path is requested
**             (c) When last used DTMT is provided.
**
**             Also the DTMT is selected from DTMT chain based on various
**             load balance algorithms like round robin, ICL 50% weighted
**             and ICL 100%  weighted based on  the algorithm choosen.
**             When there is no  ICL path, round robin algorithm is
**             automatically selected.
**
**  @param     ILT - contains information related to DTMT and MLMT.
**
**  @return    DTMT (path)choosen
**
******************************************************************************
**/
DTMT *ICL_GetTargetPath(ILT *pILT)
{
    UINT8 path;
    DTMT *pRequestedDTMT;

    DTMT *pDtmtSelected = NULL;

    path = (UINT8)(pILT->ilt_normal.w1);
    pRequestedDTMT = (DTMT *)(pILT->ilt_normal.w4);

    /*
    ** If the caller did not specify anypath and requested DTMT
    ** is not specified.. choose the selected Load Balance
    ** algorithm.
    */
    if ((DG_PATH_ANY == path) && (NULL == pRequestedDTMT))
    {
        pDtmtSelected = ICL_DlmPathSelectionHandler(pILT);
    }
    else
    {
        /*
        ** Caller either specified specific path (or) a specific
        ** DTMT is requested to be selected for sending the DG
        ** Call round robin(default) handler directly
        */
        pDtmtSelected = ICL_GetDlmPathThruRoundRobin(pILT);
    }

    return (pDtmtSelected);
}

/**
******************************************************************************
**
**  @brief     This function returns the next DTMT using the round robin algo-
**             rithm to the caller function. The DTMT chain may contain ICL or
**             FE paths or mixture of two. This is the default algorithm that
**             is used , irrespective of the fact whether ICL exists or not.
**
**  @param     ILT - contains information related to DTMT and MLMT.
**
**  @return    DTMT (path)choosen through round robin
**
******************************************************************************
**/
DTMT *ICL_GetDlmPathThruRoundRobin(ILT *pILT)
{
    DATAGRAM_REQ *pReqMsgHdr;
    MLMT         *pMLMT;
    DTMT         *pLastUsedDtmt;
    DTMT         *pRequestedDTMT;
    DTMT         *pLastUsedDtmtForDg;
    DTMT         *pNextDtmt = NULL;
    UINT8         startOverFlag = FALSE;
    UINT8         path;

    pMLMT = (MLMT *)(pILT->ilt_normal.w0);  /* associated MLMT r12 */
    path = (UINT8)(pILT->ilt_normal.w1);

    /*
    ** Check if the last DTMT used is available in ILT
    */
    pLastUsedDtmt=((DTMT*)(pILT->ilt_normal.w2));

    if ((pLastUsedDtmt=(DTMT*)(pILT->ilt_normal.w2)) != NULL)
    {
        pNextDtmt = ICL_FindNextDtmt(pMLMT, pLastUsedDtmt, &startOverFlag);
    }
    /*
    ** Check if any specific DTMT(path) is requested to use
    */
    else if ((pRequestedDTMT = (DTMT *)(pILT->ilt_normal.w4)) != NULL)
    {
        pNextDtmt = pRequestedDTMT;
    }
    /*
    ** Check If  DTMT Last Used to send a DG Msg is available in MLMT
    */
    else if ((pLastUsedDtmtForDg = (DTMT*)(pMLMT->pDTMTLastUsedDG)) != NULL)
    {
        pNextDtmt = ICL_FindNextDtmt(pMLMT, pLastUsedDtmtForDg, &startOverFlag);
    }
    else
    {
        pNextDtmt = pMLMT->pDTMTHead; /* Get the first path in list*/
        startOverFlag = TRUE;
    }

    /*
    ** Now validate the DTMT (operating state)
    */
    pReqMsgHdr = (DATAGRAM_REQ *)((pILT-1)->ilt_normal.w0);  /* g2 */

    while (pNextDtmt != NULL)
    {
        if (DTMT_STATE_OP == pNextDtmt->state)
        {
            if((DG_PATH_ANY == path) || (pNextDtmt->td.xio.path == path))
            {
                break;
            }
        }
        else if( DTMT_STATE_NOT_OP == pNextDtmt->state)
        {
            if(DLM1name == pReqMsgHdr->srvName)
            {
                if(DLM1_fc_polldgp == pReqMsgHdr->fc)
                {
                    if((DG_PATH_ANY == path) || (pNextDtmt->td.xio.path == path))
                    {
                        break;
                    }
                }
            }
        }
        pNextDtmt = ICL_GetNextDtmt(pMLMT, pNextDtmt, &startOverFlag);
    }
#if 0
    if(pNextDtmt != NULL)
    {
        pILT->ilt_normal.w2 = (UINT32)pNextDtmt;
        pMLMT->pDTMTLastUsedDG = pNextDtmt;
    }
#endif
    return (pNextDtmt);
}

/**
******************************************************************************
**
**  @brief     This function returns the next DTMT using the weighted round
**             robin-1 algorithm. This algorithm is selected when there exists
**             ICL path.  In this algorithm the ICL path shares 50% of the load
**             and the other paths (FE) shares the rest of 50% load.
**
**  @param     ILT - contains information related to DTMT and MLMT.
**
**  @return    DTMT (path) selected weithed roundrobin-1 algorithm
**
******************************************************************************
**/
DTMT *ICL_GetDlmPathThruWeightedRR_1(ILT *pILT)
{
    MLMT         *pMLMT;
    DTMT         *pDtmt;
    DTMT         *pNextDtmt = NULL;
    UINT8         startOverFlag = FALSE;
    static UINT8  iclPathUsed = FALSE;

    /*
    ** This function will never receive the datagrams meant for specific paths
    ** (like poll datagram)
    ** Also this function will not receive the datagrams related to requested
    ** DTMTs.
    */

    if (iclPathUsed == FALSE)
    {
        iclPathUsed = TRUE;
        return (pIclDtmt);
    }

    iclPathUsed = FALSE;

    pMLMT      = (MLMT *)(pILT->ilt_normal.w0);  /* associated MLMT r12 */

    /*
    ** Check if the last DTMT used is available in ILT
    */
    pDtmt=((DTMT*)(pILT->ilt_normal.w2));

    if ((pDtmt=(DTMT*)(pILT->ilt_normal.w2)) != NULL)
    {
        pNextDtmt = ICL_FindNextDtmt(pMLMT, pDtmt, &startOverFlag);
    }

    /*
    ** Check If  DTMT Last Used to send a DG Msg is available in MLMT
    */
    else if ((pDtmt = (DTMT*)(pMLMT->pDTMTLastUsedDG)) != NULL)
    {
        pNextDtmt = ICL_FindNextDtmt(pMLMT, pDtmt, &startOverFlag);
    }
    else
    {
        pNextDtmt = pMLMT->pDTMTHead; /* Get the first path in list*/
        startOverFlag = TRUE;
    }

    /*
    ** Now validate the DTMT (operating state) and ensure that it is not ICL path.
    */
    while (pNextDtmt != NULL)
    {
        if ((DTMT_STATE_OP == pNextDtmt->state) && (FALSE == pNextDtmt->iclFlag))
        {
            break;
        }
        pNextDtmt = ICL_GetNextDtmt(pMLMT, pNextDtmt, &startOverFlag);
    }
#if 0
    if(pNextDtmt != NULL)
    {
        pILT->ilt_normal.w2 = (UINT32)pNextDtmt;
        pMLMT->pDTMTLastUsedDG = pNextDtmt;
    }
#endif
    return (pNextDtmt);
}

/**
******************************************************************************
**
**  @brief     This function returns the next DTMT using the weighted round
**             robin-2 algorithm. This algorithm is selected when there exists
**             ICL path.  This algorithm  always chooses the ICL path. Means
**             entire(100%) dlm traffic is passed through ICL path alone
**
**  @param     ILT - contains information related to DTMT and MLMT.
**
**  @return    DTMT (path) selected weithed roundrobin-2 algorithm
**
******************************************************************************
**/
DTMT *ICL_GetDlmPathThruWeightedRR_2(ILT *pILT UNUSED)
{
    return (pIclDtmt);  /* 100 % ICL path */
}

/**
******************************************************************************
**
**  @brief     This function returns next DTMT to the specified DTMT in the
**             DTMT chain (contained in MLMT), after verifying whether the
**             the specified DTMT is available in the list. If available sele-
**             cts the next DTMT. If either specified DTMT is not in the list
**             es the end of the list, then chooses the head DTMT.
**
**  @param     MLMT -- MLMT related to this controller(contain DTMT chain)
**             DTMT -- DTMT specified
**             pStartOverFlag -- flag to indicate the list is already traver-
**                               sed once and wrapped to the beginning.
**
**  @return    next DTMT
**
******************************************************************************
**/
DTMT *ICL_FindNextDtmt(MLMT *pMLMT, DTMT *pDtmt, UINT8 *pStartOverFlag)
{
    DTMT *pNextDtmt = NULL;
    DTMT *pDtmt1;

    /*
    ** Check if the specified  entry is available in the list
    */

    pDtmt1 = pMLMT->pDTMTHead;  /* Get the first path in list*/
    while (pDtmt1 != NULL )     /* There is another path        */
    {
        if(pDtmt == pDtmt1)
        {
            pNextDtmt = pDtmt1->td.xio.pNext;
            break;
        }
        pDtmt1 = pDtmt1->td.xio.pNext; /* Get next DTMT in MLMT */
    }

    if ((pNextDtmt == NULL) && (*pStartOverFlag == FALSE))
    {
        pNextDtmt = pMLMT->pDTMTHead; /* Get the first path in list*/
        *pStartOverFlag = TRUE;       /* Start over occurred       */
    }

    return (pNextDtmt);
}

/**
******************************************************************************
**
**  @brief     This function returns next DTMT to the specified DTMT in the
**             DTMT chain (contained in MLMT). If it reaches the end of
**             chain returns the head DTMT.
**
**  @param     MLMT -- MLMT related to this controller(contain DTMT chain)
**             DTMT -- DTMT specified
**             pStartOverFlag -- flag to indicate the list is already traver-
**                               sed once and wrapped to the beginning.
**
**  @return    next DTMT
**
******************************************************************************
**/
DTMT *ICL_GetNextDtmt(MLMT *pMLMT, DTMT *pDtmt, UINT8 *pStartOverFlag)
{
    DTMT *pNextDtmt;

    pNextDtmt = pDtmt->td.xio.pNext;
    if((pNextDtmt == NULL) && (*pStartOverFlag == FALSE))
    {
        pNextDtmt = pMLMT->pDTMTHead; /* Get the first path in list*/
        *pStartOverFlag = TRUE;       /* Start over occurred       */
    }
    return (pNextDtmt);
}

/**
******************************************************************************
**
**  @brief    This function gets invoked when the user selected a specified
**            load balancing algorithm.
**
**  @param    MRP - Pointer to the MRP containing the load balancing algorhm
**                  type
**
**  @return   status
**
******************************************************************************
**/

UINT8 ICL_DlmPathSelectionAlgorithm(MR_PKT *pMRP)
{
    UINT8  status = DEOK;
    MRDLMPATHSELECTIONALGO_REQ  *pReq = (MRDLMPATHSELECTIONALGO_REQ *)(pMRP->pReq);
    MRDLMPATHSELECTIONALGO_RSP  *pRsp = (MRDLMPATHSELECTIONALGO_RSP *)(pMRP->pRsp);

    switch(pReq->algoType)
    {
        case ICL_DLMPATHALGO_ROUNDROBIN:
             configDlmPathAlgo = ICL_DLMPATHALGO_ROUNDROBIN;
             fprintf(stderr, "<ICL..>ICL Path algo ==  roundrobin.....\n");
             pRsp->algoType = configDlmPathAlgo;
             ICL_SetDlmPathAlgorithm();
             break;

        case ICL_DLMPATHALGO_ICL50PERCENT:
             if (pIclDtmt)
             {
                 configDlmPathAlgo = ICL_DLMPATHALGO_ICL50PERCENT;
                 fprintf(stderr, "<ICL..>ICL Path algo == ICL 50%% weighted roundrobin.....\n");
                 pRsp->algoType = configDlmPathAlgo;
                 ICL_SetDlmPathAlgorithm();
             }
             else
             {
                 fprintf(stderr, "<ICL..>ICL Path is not there...\n");
             }
             break;

        case ICL_DLMPATHALGO_ICL100PERCENT:
             if (pIclDtmt)
             {
                 configDlmPathAlgo = ICL_DLMPATHALGO_ICL100PERCENT;
                 fprintf(stderr, "<ICL..>ICL Path algo == ICL 100%% ...\n");
                 pRsp->algoType = configDlmPathAlgo;
                 ICL_SetDlmPathAlgorithm();
             }
             else
             {
                 fprintf(stderr, "<ICL..>ICL Path is not there...\n");
             }
             break;

        case ICL_DLMPATHALGO_DISPLAY:
             pRsp->algoType = currentDlmPathAlgo;
             break;

        default :
             status = DEINVOPT;
             break;
    }

    pRsp->header.len = sizeof(MRDLMPATHSELECTIONALGO_RSP);
    return status;
}

/**
******************************************************************************
**
**  @brief     This function gets invoked when ICL path has been established.
**             It chooses the algorithm based on the type of algorithm set
**             through configuration or default algorithm. Also sets the
**             gloabl ICL dtmt pointer.
**
**  @param     pDtmt -- Pointer to DTMT (path) newly created.
**
**  @return    none
**
******************************************************************************
**/
void ICL_NotifyIclPathMade(DTMT *pDtmt)
{
    /*
    ** Check if the path established is of ICL.
    */
    if(pDtmt->iclFlag)
    {
        ICL_SetDlmPathAlgorithm();
        pIclDtmt = pDtmt;
    }
}

/**
******************************************************************************
**
**  @brief     This function gets invoked when ICL path has been lost.
**             It clears the global ICL dtmt pointer also sets the load balan-
**             cing algorithm as round robin.
**
**  @param     pDtmt -- Pointer to DTMT (path) newly created.
**
**  @return    none
**
******************************************************************************
**/
void ICL_NotifyIclPathLost(DTMT *pDtmt)
{
    /*
    ** Check if the path lost is of  ICL.
    */
    if((pDtmt) && (pDtmt->iclFlag))
    {
        ICL_DlmPathSelectionHandler =  ICL_GetDlmPathThruRoundRobin;
        currentDlmPathAlgo = ICL_DLMPATHALGO_ROUNDROBIN;
        pIclDtmt = NULL;
    }
}

/**
******************************************************************************
**
**  @brief     This function sets the DLM path selection handler based on the
**             type of the load balancing algorithm chosen. The algorithm
**             choosen is already available with the global variable.
**
**  @param     none
**
**  @return    none
**
******************************************************************************
**/
void ICL_SetDlmPathAlgorithm (void)
{
    switch(configDlmPathAlgo)
    {
        case ICL_DLMPATHALGO_ROUNDROBIN:
            ICL_DlmPathSelectionHandler =  ICL_GetDlmPathThruRoundRobin;
            fprintf(stderr, "<ICL_SetDlmPathAlgo..>Algorithm selected ROUND ROBIN\n");
            break;

        case ICL_DLMPATHALGO_ICL50PERCENT:
            ICL_DlmPathSelectionHandler =  ICL_GetDlmPathThruWeightedRR_1;
            fprintf(stderr, "<ICL_SetDlmPathAlgo..>Algorithm selected : ICL 50 %%--ROUND ROBIN\n");
            break;

        case ICL_DLMPATHALGO_ICL100PERCENT:
            ICL_DlmPathSelectionHandler =  ICL_GetDlmPathThruWeightedRR_2;
            fprintf(stderr, "<ICL_SetDlmPathAlgo..>Algorithm selected : ICL 100 %%--NO FC Paths\n");
            break;

        default:
            break;
    }
    currentDlmPathAlgo = configDlmPathAlgo;
}

/**
******************************************************************************
**
**  @brief  This  function downs and resets the ICL loop when there is no ICL
**          path. This gets called from der_isp.c when RESET_PORT_NEEDED MRP is
**          arrived from CCB. However, in case of ICL, the port should be reset
**          only when the other controller is failed (at which time the ICL path
**          is already lost).  This will avoid the assuming controller to
**          continuously retry for the session on the failed controller.
**
**  @param  None
**
**  @return None
**
******************************************************************************
**/
void ICL_Offline(void)
{
    /*
    ** When there is no ICL-path, down the ICL port.
    */
    if(iclPortExists && (pIclDtmt == NULL))
    {
        ICL_LoopDown(ICL_PORT);
        ICL_PortReset(ICL_PORT);
    }
}

/**
******************************************************************************
**
**  @brief   This  function checks whether the front end DLM communiation path
**           exists is only ICL (no iSCSI path). If so it returns TRUE. If any
**           FE path (iSCSI) exists along with/without ICL, it returns FALSE.
**
**           This is used by FE Communication logic to verify and take some
**           action when ICL path path alone exists between the controllers.
**
**           This functions all the paths (DTMTs) available related to the
**           specified controller (MLMT).If any non-ICL path exists, it returns
**           FALSE, otherwise it returns TRUE.
**
**  @param   pMLMT -- Pointer to MLMT
**
**  @return  TRUE   -- If ICL path alone is existing
**           FALSE  -- If any non-ICL path is existing
**
**  @attention
**           This function assumes non-NULL pointer (pMLMT) as input param.
******************************************************************************
**/
UINT8 ICL_onlyIclPathExists(MLMT *pMLMT)
{
    DTMT *pDtmt;
    UINT8 retVal = TRUE;

    /*
    ** Get first DTMT from MLMT
    */
    pDtmt = pMLMT->pDTMTHead;
    while (pDtmt != NULL )
    {
        /*
        ** Check if this is non-ICL (iSCSI) path
        */
        if ((pDtmt->iclFlag) == FALSE)
        {
            retVal = FALSE;
            break;
        }
        pDtmt = pDtmt->td.xio.pNext;
    }
    return (retVal);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
