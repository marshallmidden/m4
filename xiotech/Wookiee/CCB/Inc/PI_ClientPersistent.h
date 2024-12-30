/* $Id: PI_ClientPersistent.h 143020 2010-06-22 18:35:56Z m4 $ */
/******************************************************************************
**
**  @file       PI_ClientPersistent.h
**
**  @brief      Header for the function prototypes of persistent data.
**
**  Copyright (c) 2004-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _PI_CLIENT_PERSISTENT_H_
#define _PI_CLIENT_PERSISTENT_H_

#include "XIO_Types.h"

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern MUTEX gMgtListMutex;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void appendDefaultPath(char *pathname, char *fname);
extern INT32 AddMgtStructure(char *recname, UINT32 reclen, UINT8 isResync);
extern INT32 UpdateMgtStructure(char *recname, UINT32 reclen, UINT8 isResync);
extern INT32 CreateClientRecord(char *fname);
extern INT32 WriteClientRecord(char *fname, UINT32 start, UINT32 *nbytes, void *buffer,
                               INT32 truncate_lth);
extern INT32 GetClientRecordSize(const char *fname, UINT32 *fsize);
extern INT32 TransferClientDataToSlave(UINT32 serNum);
extern void DeleteAllPersistentFiles(void);
extern void SyncClientData(void);
extern INT32 Client_InitMgtFile(const char *fname);
extern void RemoveAllLocks(INT32 lockfd);
extern INT32 GetLatestClientData(IPC_LATEST_PERSISTENT_DATA *pClientCmd);
extern void InitClientPersistent(void);

#endif /* _PI_CLIENT_PERSISTENT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
