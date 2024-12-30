/* $Id: ParmVal.h 122127 2010-01-06 14:04:36Z m4 $ */
/*===========================================================================
** FILE NAME:       ParmVal.h
** MODULE TITLE:    Parameter Validation Layer
**
** DESCRIPTION:     Implements the Parameter Validation layer in the CCB code
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef __PRMVALHDL_H__
#define __PRMVALHDL_H__

#include "XIO_Types.h"
#include "PortServer.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

struct _XIO_PACKET;

/*----------------------------------------------------------------------------
** Function:    ParmValidationPreProcessImpl
**
** Description: Implements the Parameter Validation pre-processing function
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
** WARNING:     none
**--------------------------------------------------------------------------*/
extern INT32 ParmValidationPreProcessImpl(struct _XIO_PACKET *pReqPacket,
                                          struct _XIO_PACKET *pRspPacket);

/*----------------------------------------------------------------------------
** Function:    ParmValidationPostProcessImpl
**
** Description: Implements the Parameter Validation post-processing function
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
** WARNING:     none
**--------------------------------------------------------------------------*/
extern INT32 ParmValidationPostProcessImpl(struct _XIO_PACKET *pReqPacket,
                                           struct _XIO_PACKET *pRspPacket);

/*
**  Download drive / drive bay microcode via write buffer.
**  Note: the parm validation routine for this command is called
**  from PI_FWDownload().
*/
extern INT32 ParmCheckWriteBuffer(struct _XIO_PACKET *pReqPacket,
                                  struct _XIO_PACKET *pRspPacket, UINT8 *pBuf);


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PRMVALHDL_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
