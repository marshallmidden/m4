/* $Id: Snapshot.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       Snapshot.h
** MODULE TITLE:    Header file for snapshot.c /
**                  Simple File System Functions
**
** DESCRIPTION:     Utility functions for reading and writing configuration
**                  snapshots.
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _SNAPSHOT_H_
#define _SNAPSHOT_H_

#include "XIO_Std.h"
#include "XIO_Types.h"
#include "mutex.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** Snapshot "types"
*/
#define SNAPSHOT_TYPE_MANUAL        1
#define SNAPSHOT_TYPE_POWERUP       2
#define SNAPSHOT_TYPE_SHUTDOWN      3
#define SNAPSHOT_TYPE_CONFIGCHG     4
#define SNAPSHOT_TYPE_HOTSPARE      5

/*
** Snapshot save/restore "flags"
*/
#define SNAPSHOT_FLAG_MASTER_CONFIG 0x01
#define SNAPSHOT_FLAG_CTRL_MAP      0x02
#define SNAPSHOT_FLAG_BE_NVRAM      0x04
#define SNAPSHOT_FLAG_ALL           0xFFFFFFFF

/*
** Snapshot entry states
*/
#define SNAPSHOT_STATUS_NOOP        0   /* indicates "No change" when     */
                                        /* passed as change paramter.     */
#define SNAPSHOT_STATUS_OPEN        1   /* Never been used,     reusable  */
#define SNAPSHOT_STATUS_DELETED     2   /* Deleted by user,     reusable  */
#define SNAPSHOT_STATUS_INUSE       3   /* In use,              reusable  */
#define SNAPSHOT_STATUS_ERROR       4   /* error occr'd on save, reusable */
#define SNAPSHOT_STATUS_KEEP        5   /* In use,          non-reusable  */
#define SNAPSHOT_STATUS_CRC         6   /* In use, a CRC error was detected */
                                        /* on this entry however. reusable  */

/*
** Miscellaneous
*/
#define SNAPSHOT_MAGIC_NUMBER       0xC0EDBABE
#define SNAPSHOT_SCHEMA_1           0x10101010

#define SNAPSHOT_DESCRIPTION_LEN    196

#define SNAPSHOT_DEFAULT_DELAY_TIME (10 * 60)   /* 10 min */

/*****************************************************************************
** Public variables
*****************************************************************************/
extern MUTEX configJournalMutex;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern INT32 TakeSnapshot(UINT32 type, const char *description);
extern void TakeSnapshotTask(TASK_PARMS *parms);
extern INT32 LoadSnapshot(UINT32, UINT32 flags);
extern INT32 ChangeSnapshot(UINT32, UINT32 status, char *description);
extern INT32 ReadSnapshotDirectory(char *buffer, UINT32 length);
extern void InitSnapshotFID(void);
extern UINT32 SizeofSnapshotDirectory(void);
extern void DelayedSnapshot(UINT32 delay);
extern UINT32 DisplaySnapshotDirectoryVerbose(void);
extern UINT32 DisplaySnapshotDirectory(char **buffer);
extern UINT32 DisplaySnapshotDirectoryEntry(char **buffer, UINT32);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SNAPSHOT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
