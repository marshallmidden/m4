/* $Id: AsyncEventHandler.h 130553 2010-03-04 17:33:12Z mdr $*/
/**
******************************************************************************
**
**  @file   AsyncEventHandler.h
**
**  @brief  Asynchronous Event Handler
**
**  Handle events coming from the back end or front end processor to the CCB.
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _ASYNCEVENTHANDLER_H_
#define _ASYNCEVENTHANDLER_H_

#include "ipc_packets.h"
#include "logging.h"
#include "PortServer.h"
#include "slink.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
typedef struct NEW_SERVER_t
{
    UINT8       channel;        /* Channel this server is on                */
    UINT8       rsvd;           /* RESERVED                                 */
    UINT16      targetId;       /* Target ID this new server is on          */
    UINT32      owner;          /* Owner                                    */
    UINT64      wwn;            /* World Wide Name                          */
    UINT8       i_name[256];    /* iSCSI Server name                        */
} NEW_SERVER;

typedef struct MRP_t
{
    UINT16      mr_funct;       /* Function                             */
    UINT8       mr_version;     /* Command Version                      */
    UINT8       rsvd1;          /* RESERVED                             */
    UINT8       rsvd2[8];       /* RESERVED                             */
    void       *mr_pptr;        /* MRP pointer across the PCI bus       */
    UINT32      mr_ralloclen;   /* Return data allocation length        */
    void       *mr_rptr;        /* Return data pointer                  */
    void       *mr_ptr;         /* Command Parameters packet pointer    */
    UINT32      mr_len;         /* Command Parameters packet length     */
} MRP;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void AsyncEventHandler(MRP *ptrMRP, LOG_MRP *logMRP);
extern void TerminateBroadcasts(void);
extern void AsyncEventBroadcast(IPC_BROADCAST *pBroadcast);
extern void SendAsyncEvent(INT32 eventType, INT32 dataLen, void *data);
extern UINT32 ConvertBroadcastEventToCommandCode(UINT16 eventType);
extern void PDiskDefragOperation(UINT16 pid, UINT32 delay);
extern INT32 PDiskDefragControl(UINT16 control, UINT16 pid, UINT16 rid,
                                UINT64 sda, MRDEFRAGMENT_RSP * pResponse);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _ASYNCEVENTHANDLER_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
