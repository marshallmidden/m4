/* $Id: CmdLayers.c 145157 2010-08-05 20:40:33Z m4 $*/
/*===========================================================================
** FILE NAME:       CmdLayers.c
** MODULE TITLE:    CCB Command Layers
**
** DESCRIPTION:     Defines the command layering withing the CCB code
**
** Copyright (c) 2001-2009  XIOtech Corporation.  All rights reserved.
**==========================================================================*/
#include "CmdLayers.h"
#include "LogOperation.h"
#include "ParmVal.h"
#include "RMCmdHdl.h"
#include "PktCmdHdl.h"
#include "PacketInterface.h"
#include "XIO_Const.h"

/* Private function prototypes. */
static INT32 RMCommandHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmValidationHandler(struct _XIO_PACKET *pReqPacket,
                                   struct _XIO_PACKET *pRspPacket);

/*----------------------------------------------------------------------------
** Function:    PortServerCommandHandler
**
** Description: This is the top of the CCB packet interface command chain.
**              In general all users should call through this layer unless
**              they have a valid reason not to along with a signed note
**              from the powers that be and their friends.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 PortServerCommandHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

    /* Pre-process command before passing to next handler */

    rc = ParmValidationHandler(pReqPacket, pRspPacket);

    /* Post-process as required */

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmValidationHandler
**
** Description: Validate input parms before passing the command to
**              the next layer.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmValidationHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

    /* Pre-process command before passing to next handler */
    rc = ParmValidationPreProcessImpl(pReqPacket, pRspPacket);

    /*
     * Based on the results of the parameter validation pre-processor either
     * call the next layer or return.
     */
    if (rc == GOOD)
    {
        rc = RMCommandHandler(pReqPacket, pRspPacket);
    }

    /* Post-process as required */
    if (rc == GOOD)
    {
        rc = ParmValidationPostProcessImpl(pReqPacket, pRspPacket);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    RMCommandHandler
**
** Description: Resource Manager Command Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 RMCommandHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc;
    INT32       second_rc;

    /* Pre-process command before passing to next handler */
    rc = RMCommandHandlerPreProcessImpl(pReqPacket, pRspPacket);

    /*
     * Based on the results from Resource Manager pre-processor either
     * call the next layer or return.
     */
    if (rc == GOOD)
    {
        rc = PacketCommandHandler(pReqPacket, pRspPacket);
    }

    /* Post-process as required */
    second_rc = RMCommandHandlerPostProcessImpl(pReqPacket, pRspPacket);
    rc = (rc == GOOD) ? second_rc : rc;

    /* If a valid command code was found, log the operation. */
    if (rc != PI_INVALID_CMD_CODE)
    {
        /* Log this operation */
        LogOperation(pReqPacket, pRspPacket);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PacketCommandHandler
**
** Description: This is the bottom of the command chain.  This layer
**              makes the calls to processor board via the link layer.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 PacketCommandHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

    /* Pre-process command before passing to next handler */
    rc = PacketCommandHandlerPreProcessImpl(pReqPacket, pRspPacket);

    /*
     * Based on the results from Resource Manager pre-processor either
     * call the next layer or return.
     */
    if (rc == GOOD)
    {
        rc = PacketCommandHandlerImpl(pReqPacket, pRspPacket);
    }

    /* Post-process as required */
    if (rc == GOOD)
    {
        rc = PacketCommandHandlerPostProcessImpl(pReqPacket, pRspPacket);
    }
    else
    {
        /*
         * If the command failed but it was a label command we still want
         * to do some post processing but do not change the return code.
         */
        if (pReqPacket->pHeader->commandCode == PI_PDISK_LABEL_CMD)
        {
            PacketCommandHandlerPostProcessImpl(pReqPacket, pRspPacket);
        }
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
