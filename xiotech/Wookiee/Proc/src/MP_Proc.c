/* $Id: MP_Proc.c 145021 2010-08-03 14:16:38Z m4 $ */
/**
*******************************************************************************
**
** @file: MP_Proc.c
**
**       To provide a means of getting the mirror partner information in order to
**       handle the situation where the mirror partner is Bigfoot or any other
**       controller.(DSC upgrade/downgrade handling in PROC)
**
**  Copyright (c) 2004-2008 Xiotech Corporation.  All rights reserved.
**
********************************************************************************
**/
#include "MP_Proc.h"
#include "MP_ProcProto.h"

#include "CA_CacheFE.h"
#include "ficb.h"
#include "kernel.h"
#include "LL_LinuxLinkLayer.h"
#include "MR_Defs.h"
#include "nva.h"
#include "nvram.h"
#include "options.h"
#include "pcb.h"
#include "system.h"
#include "wcache.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define  BF_WCACHESIZE              0

#define MP_DEFAULT_SERIAL_NUM       0
#define MP_NVP3_BE_DEFAULT_SIZE     NVRAM_P3_SIZE
#define MP_NVP4_FE_DEFAULT_SIZE     NVRAM_P4_SIZE
#define MP_NVP4_BE_DEFAULT_SIZE     NVRAM_P4_SIZE
#define MP_NVP5_FE_DEFAULT_SIZE     NVRAM_P5_SIZE
#define MP_NVP5_BE_DEFAULT_SIZE     NVRAM_P5_SIZE
#define MP_WCACHE_DEFAULT_SIZE      0
#define MP_FLAGS_DEFAULT            0   /*Default controller type is wookiee*/
#define MP_BATTERY_HEALTH_DEFAULT   1

/*
******************************************************************************
** Private variables
******************************************************************************
*/
#ifdef FRONTEND
/*
** This instance contain the resync and configuration that this Controller
** supports. This gets never changed.
*/
MP_MIRROR_PARTNER_INFO gMPControllerOriginalConfig =
{
    MP_DEFAULT_SERIAL_NUM,
    MP_NVP3_BE_DEFAULT_SIZE,
    MP_NVP4_FE_DEFAULT_SIZE,
    MP_NVP4_BE_DEFAULT_SIZE,
    MP_NVP5_FE_DEFAULT_SIZE,
    MP_NVP5_BE_DEFAULT_SIZE,
    MP_WCACHE_DEFAULT_SIZE,
    MP_FLAGS_DEFAULT,
    MP_BATTERY_HEALTH_DEFAULT,
    {0}                         /* Initialize reserved array to all zeros */
};

MP_MIRROR_PARTNER_INFO bfDefaults =
{
    0,
    NVRAM_P3_SIZE,
    NVRAM_P4_SIZE,
    NVRAM_P4_SIZE,
    NVRAM_P5_SIZE,
    NVRAM_P5_SIZE,
    BF_WCACHESIZE,
    MP_CONTROLLER_TYPE_BF,
    MP_BATTERY_HEALTH_DEFAULT,
    {0}                 /* Initialize reserved array to all zeros */
};

#endif

/*
** FE to BE communication area
*/
MRSETMPCONFIGBE_REQ *packetFromFE;
MRSETMPCONFIGBE_RSP *pRetdata;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
/*
** This instance contain the resync and configuration information of the
** controller with respect to its mirror partner. Initially it is set to
** default values and gets updated during mirror partner handshaking by
** CCB.
*/
MP_MIRROR_PARTNER_INFO gMPControllerCurrentConfig =
{
    MP_DEFAULT_SERIAL_NUM,
    MP_NVP3_BE_DEFAULT_SIZE,
    MP_NVP4_FE_DEFAULT_SIZE,
    MP_NVP4_BE_DEFAULT_SIZE,
    MP_NVP5_FE_DEFAULT_SIZE,
    MP_NVP5_BE_DEFAULT_SIZE,
    MP_WCACHE_DEFAULT_SIZE,
    MP_FLAGS_DEFAULT,
    MP_BATTERY_HEALTH_DEFAULT,
    {0}                         /* Initialize reserved array to all zeros */
};

/*****************************************************************************
** Public function prototypes not in header files
*****************************************************************************/
extern void NVA_ReInitialize (void);
extern void LL_SendPacket(void *pReq, UINT32 reqSize, UINT32 cmdCode,
                          void *pRsp, UINT32 rspLen, void *pCallback,
                          UINT32 param);
extern void WC_batHealth(UINT8 board, UINT8 state);
extern void WC_RestoreData(void);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/

#ifdef  FRONTEND
static void MP_SendMPConfigMRPtoBE(void);
static void MP_SetToMinimumConfigFE(MP_MIRROR_PARTNER_INFO *pConf);
static void MP_BEConfigCompleter(UINT32 retCode, struct ILT *pILT, MR_PKT *pMRP,
                                 UINT32 param);
#endif  /* FRONTEND */


/*
******************************************************************************
** Code Start
******************************************************************************
*/

#ifdef FRONTEND

/**
******************************************************************************
**
**  @brief      To provide a standard means of setting the mirror partner
**              information in FE when the MRSETMPCONFIGFE MRP is received.
**              The definefe calls this function when it receives MRP from CCB.
**
**  @param      pMRP    : MRP packet
**
**  @return     UINT32 status
**
**  @attention
**
**
** MRP Function Code: MRSETMPCONFIGFE 0x511 (mrfeassignmp)
******************************************************************************
**/
UINT32 MP_SetMPConfigFE(MR_PKT *pMRP)
{
    MP_MIRROR_PARTNER_INFO* pMirrorInfo;
    UINT32                  status;
    UINT32                  newSerialNo;
    MRSETMPCONFIGFE_RSP*    pRsp;

    /*
    ** get the mirror partner configuration from MRP.
    */
    pMirrorInfo = (MP_MIRROR_PARTNER_INFO*)pMRP->pReq;
    pRsp        = (MRSETMPCONFIGFE_RSP *)(pMRP->pRsp);

#ifdef DEBUG_NEWMP
    fprintf(stderr, "MP_SetMPConfigFE:  Enter, Req pkt ptr: %p Rsp Pkt ptr: %p\n",
            pMirrorInfo, pRsp);
#endif /*DEBUG_NEWMP*/

    pRsp->oldSerialNumber = K_ficb->mirrorPartner;

    /*
    **  Determine if the Mirror Partner can change. Ignore no communications path
    **  to the Mirror Partner at this time.
    */
    if ((status = CA_AcceptMPChange(pMirrorInfo->serialNo)) != DEOK)
    {
#ifdef DEBUG_NEWMP
        fprintf(stderr,"MP_SetMPConfigFE:  Returning Error\n");
#endif /*DEBUG_NEWMP*/

        if (status != DENOCOMM)
        {
            return(status);
        }
    }

    /*
    ** All looks OK, change the mirror partner.
    ** Check, whether the present(new) MP is Bigfoot and existing(current)
    ** MP is also Bigfoot.
    */
    if (BIT_TEST(pMirrorInfo->flags,MP_USE_BFDEFAULTS_BIT))
    {
#ifdef DEBUG_NEWMP
        fprintf(stderr,"MP_SetMPConfigFE:  Mirror Partner is Bigfoot\n");
#endif /*DEBUG_NEWMP*/

        /*
        ** New MP is Bigfoot.. Now check whether the existing one is also BF
        */
        if (! MP_IS_BIGFOOT)
        {
            /*
            ** Now bigfoot, earlier it was non-bigfoot.
            ** Copy Bigfoot constants to the "gMPControllerCurrentConfig"
            */
            memcpy((void*)&gMPControllerCurrentConfig,(void*)&bfDefaults,
            sizeof(MP_MIRROR_PARTNER_INFO));

            /*
            ** Make note that the configuration has been changed, by setting
            ** the configuration change bit.
            */
            BIT_SET (gMPControllerCurrentConfig.flags,MP_CONFIG_CHANGE_BIT);

            /*
            ** Reinitialize the NVRAM control structures with respect to changed
            ** configuration.
            */
            NVA_ReInitialize();

            /*
            ** The mirror partner is Bigfoot. Make note of it by setting the
            ** controller type bit.
            */
            BIT_SET (gMPControllerCurrentConfig.flags,MP_CONTROLLER_TYPE_BIT);
        }
        else
        {
           /*
            ** No change  in configuration is needed - So clear the config
            ** change bit.
            */
            BIT_CLEAR (gMPControllerCurrentConfig.flags,MP_CONFIG_CHANGE_BIT);
        }
    }
    else
    {
        /*
        ** Now the current MP is non-Bigfoot. So clear the controller type bit
        */
        BIT_CLEAR(gMPControllerCurrentConfig.flags,MP_CONTROLLER_TYPE_BIT);

        /*
        ** Compare new configuration with the existing one
        */
        if ((gMPControllerCurrentConfig.nvramP3SizeBE != pMirrorInfo->nvramP3SizeBE) ||
            (gMPControllerCurrentConfig.nvramP4SizeFE != pMirrorInfo->nvramP4SizeFE) ||
            (gMPControllerCurrentConfig.nvramP4SizeBE != pMirrorInfo->nvramP4SizeBE) ||
            (gMPControllerCurrentConfig.nvramP5SizeFE != pMirrorInfo->nvramP5SizeFE) ||
            (gMPControllerCurrentConfig.nvramP5SizeBE != pMirrorInfo->nvramP5SizeBE) ||
            (gMPControllerCurrentConfig.wCacheSize != pMirrorInfo->wCacheSize))
        {
#ifdef DEBUG_NEWMP
            fprintf(stderr, "MP_SetMPConfigFE:  There is a change in MP Config...\n");
#endif /*DEBUG_NEWMP*/

            /*
            ** Make note that the configuration has been changed, by setting
            ** the configuration change bit.
            */
            BIT_SET(gMPControllerCurrentConfig.flags,MP_CONFIG_CHANGE_BIT);

            /*
            ** UPdate the current configuration instance and Re-initialize the
            ** NVRAM Control structures.
            */
            MP_SetToMinimumConfigFE (pMirrorInfo);
            NVA_ReInitialize ();
        }
        else
        {
            /*
            ** No change in the configuration data.
            ** So clear config_change bit in the current config instance.
            */
            BIT_CLEAR (gMPControllerCurrentConfig.flags,MP_CONFIG_CHANGE_BIT);
        }
    }

    /*
    ** Check the new MP serial number.
    */
    newSerialNo =  pMirrorInfo->serialNo;

    if (newSerialNo == 0)
    {
        /*
        ** Clear the serial number change bit in current config - to let BE
        ** not to update.
        */
        BIT_CLEAR (gMPControllerCurrentConfig.flags, MP_SERIAL_NUM_CHANGE_BIT);
    }
    else
    {
        /*
        ** Copy new serial number into Global configuration struct
        */
#ifdef DEBUG_NEWMP
        fprintf(stderr, "MP_SetMPConfigFE:  newMPSerial = 0x%X, own-serail= 0x%X, oldMP = 0x%X\n",
                newSerialNo, K_ficb->cSerial, K_ficb->mirrorPartner);
#endif /*DEBUG_NEWMP*/

        gMPControllerCurrentConfig.serialNo = newSerialNo;

        /*
        ** Update the mirror partner serial number and other cache related
        ** flags.
        */
        CA_SetMirrorPartnerFE_1(newSerialNo);

        /*
        ** Set the serial number change bit in current config - to let BE
        ** update it.
        */
        BIT_SET (gMPControllerCurrentConfig.flags, MP_SERIAL_NUM_CHANGE_BIT);
    }

    /**
    ** Send an MRP to BE with updated information (current config instance),
    ** only when there is any change(config or serial no) in FE side.
    */

    if ((BIT_TEST(gMPControllerCurrentConfig.flags,MP_CONFIG_CHANGE_BIT) ) ||
        (BIT_TEST(gMPControllerCurrentConfig.flags,MP_SERIAL_NUM_CHANGE_BIT)) )
    {
        MP_SendMPConfigMRPtoBE();
    }

    /*
    ** Perform cache related operations(cache enable OP, error PCB state update.
    */
    CA_SetMirrorPartnerFE_2();

    WC_batHealth(BATTERY_BOARD_BE, pMirrorInfo->batteryHealth);
    WC_RestoreData();

    return(DEOK);
}
#endif /*FRONTEND*/


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      To provide a standard means of sending MRP to BE with new mirror
**              partner information (MRSETMPCONFIGBE).
**
**  @param      none
**
**  @return     none
**
**  @attention
**
**
******************************************************************************
**/
static void MP_SendMPConfigMRPtoBE(void)
{
#ifdef DEBUG_NEWMP
    fprintf(stderr, "MP_SendMPConfigMRPtoBE:  Entry\n");
#endif /*DEBUG_NEWMP*/

    packetFromFE = (MRSETMPCONFIGBE_REQ*)s_MallocC(sizeof(MRSETMPCONFIGBE_REQ), __FILE__, __LINE__);
    pRetdata     = (MRSETMPCONFIGBE_RSP*)s_MallocC(sizeof(MRSETMPCONFIGBE_RSP), __FILE__, __LINE__);

    memcpy((void*)(packetFromFE),
           (void*)&gMPControllerCurrentConfig,
           sizeof(MP_MIRROR_PARTNER_INFO));

#ifdef DEBUG_NEWMP
    fprintf(stderr, "MP_SendMPConfigMRPtoBE:  ReqPtr: %p, RetPtr: %p\n",
            packetFromFE, pRetdata);
#endif /*DEBUG_NEWMP*/

    LL_SendPacket(packetFromFE,                 /* Packet address             */
                  sizeof(MRSETMPCONFIGBE_REQ),  /* Mirror Partner packet size */
                  MRSETMPCONFIGBE,              /* function code              */
                  pRetdata,                     /* return data address        */
                  sizeof(MRSETMPCONFIGBE_RSP),  /* Return data size           */
                  (void*)&MP_BEConfigCompleter, /* Completer function         */
                  0);                           /* user parameter             */

#ifdef DEBUG_NEWMP
    fprintf(stderr, "MP_SendMPConfigMRPtoBE:  Exit\n");
#endif /*DEBUG_NEWMP*/
}


/**
******************************************************************************
**
**  @brief      To provide a common means of freeing memory used by
**              MRSETMPCONFIGBE MRP handler
**
**  @param      retCode, Pointer to ILT, Pointer MRP, param if any
**
**  @return     none
**
**  @attention
**
******************************************************************************
**/
static void MP_BEConfigCompleter(UINT32 retCode UNUSED,
                        struct ILT *pILT UNUSED,
                        MR_PKT *pMRP UNUSED, UINT32 param UNUSED)
{
    s_Free(packetFromFE, sizeof(MRSETMPCONFIGBE_REQ), __FILE__, __LINE__);
    s_Free(pRetdata, sizeof(MRSETMPCONFIGBE_RSP), __FILE__, __LINE__);

#ifdef DEBUG_NEWMP
    fprintf(stderr,"<MP_Proc.c>MP_BEConfigCompleter:Exit\n");
#endif /*DEBUG_NEWMP*/
}
#endif /*FRONTEND*/


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      To provide a standard means of handling MRGETMPCONFIGFE
**              fromm CCB. The mirror partner information will be copied to MRP
**              by  FE when the  MRP is received.
**
**  @param      pMRP    : MRP packet
**
**  @return     UINT32 status
**
**  @attention
**
**
** MRP Function code : MRGETMPCONFIGFE 0x529 (new)
******************************************************************************
**/
UINT32 MP_GetMPConfigFE (MR_PKT *pMRP)
{
    UINT32                  status = DEOK;
    MRGETMPCONFIGFE_RSP* pGetConfigRsp;

    pGetConfigRsp  = (MRGETMPCONFIGFE_RSP*)(pMRP->pRsp);

#ifdef DEBUG_NEWMP
    fprintf(stderr, "<MP_Proc.c> Function code received in MP_GetMPConfigFE:%x\n",
            pMRP->function);
#endif /*DEBUG_NEWMP*/

    /*
    ** Set the serial number of this(our) controller
    */
    gMPControllerOriginalConfig.serialNo = K_ficb->cSerial;

    /*
    ** Set the return length and status in the MRP response header
    */
    pGetConfigRsp->header.len = sizeof(MRGETMPCONFIGFE_RSP);

#ifdef DEBUG_NEWMP
    fprintf(stderr, "<MP_Proc.c> MP_GetMPConfigFE rsp size:%d\n",
            pGetConfigRsp->header.len);
#endif /*DEBUG_NEWMP*/

    pGetConfigRsp->header.status = DEOK;

    /*
    ** Copy the original controller configuration info to the MRP response
    ** Data space.
    */
    memcpy(&(pGetConfigRsp->mirrorPartnerInfo),
            &gMPControllerOriginalConfig,
            sizeof(MP_MIRROR_PARTNER_INFO));

#ifdef DEBUG_NEWMP
    fprintf(stderr, "<MP_Proc.c> MP_GetMPConfigFE:Serial Num:%x,P3NVRAM Size%x\n",
            pGetConfigRsp->mirrorPartnerInfo.serialNo,
            pGetConfigRsp->mirrorPartnerInfo.nvramP3SizeBE);
#endif /*DEBUG_NEWMP*/

    return(status);
}
#endif /*FRONTEND*/


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      To provide a standard means of setting the current mirror
**              partner info structure of a controller to minimum of it's MP
**              and it's current config.
**
**  @param      pointer to MP_MIRROR_PARTNER_INFO
**
**  @return     none
**
******************************************************************************
**/
static void MP_SetToMinimumConfigFE(MP_MIRROR_PARTNER_INFO *pConf)
{
    /*
    ** Set minimums of the two global instances (current config and MP values)
    ** into current config instance.
    */
    if (gMPControllerOriginalConfig.nvramP3SizeBE >= pConf->nvramP3SizeBE)
    {
        gMPControllerCurrentConfig.nvramP3SizeBE = pConf->nvramP3SizeBE;
    }

    if (gMPControllerOriginalConfig.nvramP4SizeFE >= pConf->nvramP4SizeFE)
    {
        gMPControllerCurrentConfig.nvramP4SizeFE = pConf->nvramP4SizeFE;
    }

    if (gMPControllerOriginalConfig.nvramP4SizeBE >= pConf->nvramP4SizeBE)
    {
        gMPControllerCurrentConfig.nvramP4SizeBE = pConf->nvramP4SizeBE;
    }

    if (gMPControllerOriginalConfig.nvramP5SizeFE >= pConf->nvramP5SizeFE)
    {
        gMPControllerCurrentConfig.nvramP5SizeFE = pConf->nvramP5SizeFE;
    }

    if (gMPControllerOriginalConfig.nvramP5SizeBE >= pConf->nvramP5SizeBE)
    {
        gMPControllerCurrentConfig.nvramP5SizeBE = pConf->nvramP5SizeBE;
    }

    if (gMPControllerOriginalConfig.wCacheSize >= pConf->wCacheSize)
    {
        gMPControllerCurrentConfig.wCacheSize = pConf->wCacheSize;
    }
}
#endif /*FRONTEND*/

#ifdef BACKEND
/**
******************************************************************************
**
**  @brief      To provide a standard means of storing the mirror partner
**              information in current configuration instance in BE.
**
**  @param      pMRP    : MRP packet
**
**  @return     UINT32 status
**
**  @attention
**
** MRP Function code: MRSETMPCONFIGBE 0x0402 (MRFESETMP)
******************************************************************************
**/
UINT32 MP_SetMPConfigBE (MR_PKT *pMRP)
{
    MP_MIRROR_PARTNER_INFO* pMirrorPartnerInfo;
    MRSETMPCONFIGBE_RSP*    pRetData;
    UINT32                  status;

    /*
    ** Copy the mirror partner configuration from MRP.
    */
    pMirrorPartnerInfo = (MP_MIRROR_PARTNER_INFO*)(pMRP->pReq);

#ifdef DEBUG_NEWMP
    fprintf(stderr, "MP_SetMPConfigBE:  Req Ptr: %p, Rsp Ptr: %p\n",
            pMirrorPartnerInfo, pMRP->pRsp);
#endif/*DEBUG_NEWMP*/

    if (BIT_TEST (pMirrorPartnerInfo->flags,MP_CONFIG_CHANGE_BIT) )
    {
        /*
        ** Update the values in the current configuration instance.
        ** Re_Initialize NVRAM structures (bit map etc.) in BE.
        */
#ifdef DEBUG_NEWMP
        fprintf(stderr, "MP_SetMPConfigBE:  Config Changed, NVP3Size: %x\n",
                pMirrorPartnerInfo->nvramP3SizeBE);
#endif/*DEBUG_NEWMP*/

        memcpy((void *)&gMPControllerCurrentConfig,(void *)pMirrorPartnerInfo,
                sizeof(MP_MIRROR_PARTNER_INFO) );
                NVA_ReInitialize();
    }

    if (BIT_TEST (pMirrorPartnerInfo->flags,MP_SERIAL_NUM_CHANGE_BIT))
    {
#ifdef DEBUG_NEWMP
        fprintf(stderr, "MP_SetMPConfigBE:  SN Changed, SN: %x\n",
                pMirrorPartnerInfo->serialNo);
#endif /*DEBUG_NEWMP*/

        /*
        ** Save the New Mirror Partner serial number in BE- FICB structure.
        ** Save the changed FICB into part-2 NVRAM.
        */
        K_ficb->mirrorPartner = pMirrorPartnerInfo->serialNo;

        /*
        ** Set mirror partner serianl no. in current config instance.
        */
        MP_SET_MPSERIALNUM(pMirrorPartnerInfo->serialNo);

        /*
        ** Update P2 NVRAM
        */
        NV_P2Update();
    }

    status = DEOK;
    pRetData = (MRSETMPCONFIGBE_RSP*)(pMRP->pRsp);
    pRetData->header.len = sizeof(MRSETMPCONFIGBE_RSP);
    pRetData->header.status = status;

#ifdef DEBUG_NEWMP
    fprintf(stderr, "MP_SetMPConfigBE:  Req Ptr: %p, Rsp Ptr: %p\n",
            pMirrorPartnerInfo, pMRP->pRsp);

    fprintf(stderr, "MP_SetMPConfigBE:  returning status: %x\n",
            status);
#endif /*DEBUG_NEWMP*/

    return(status);
}
#endif /* BACKEND */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
