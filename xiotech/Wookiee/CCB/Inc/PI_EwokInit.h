/* $Id: PI_EwokInit.h 122127 2010-01-06 14:04:36Z m4 $ */
/******************************************************************************
**
**  @file       PI_EwokInit.h
**
**  @brief      Header for the initialize the ewok persistent data.
**
**  Header for initialize and lock routines for ewok persistent data
**
**  Copyright (c) 2004-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _PI_EWOK_INIT_H_
#define _PI_EWOK_INIT_H_

#include "XIO_Types.h"

extern INT16 Client_InitMgtFile(const char *fname);
extern void RemoveAllLocks(INT32 lockfd);
extern INT32 PI_ClientPersistentDataControl(XIO_PACKET *pReqPacket,
                                            XIO_PACKET *pRspPacket);

#endif /* _PI_EWOK_INIT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
