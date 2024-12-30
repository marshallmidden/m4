/* $Id: ipc_security.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_security.h
** MODULE TITLE:    Header file for ipc_security.c
**
** DESCRIPTION:     Specfication of the ipc packet security features
**
** Copyright (c) 2001-2010 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IPC_SECURITY_H_
#define _IPC_SECURITY_H_

#include "XIO_Types.h"
#include "ipc_packets.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

extern bool CheckHeaderMD5(IPC_PACKET *packet, UINT8 *key);
extern bool CheckDataMD5(IPC_PACKET *packet);
extern bool CreateMD5Signature(IPC_PACKET *packet, UINT8 *key);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPC_SECURITY_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
