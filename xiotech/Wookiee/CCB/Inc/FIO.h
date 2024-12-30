/* $Id: FIO.h 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       FIO.h
** MODULE TITLE:    File System Functions
**
** DESCRIPTION:     Utility functions for reading and writing files from disk
**
y**==========================================================================*/
#ifndef _FIO_H_
#define _FIO_H_

#include "ccb_flash.h"
#include "FS_Structs.h"
#include "mutex.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** API identifiers for SendFileioErrTask()
*/
#define FILEIO_WRITE                1
#define FILEIO_READ                 2
#define FILEIO_FID2FID              3

/*
 * Macro variations on ReadFileBaseFunc()
 */
#define ReadFile(fid, buf, len)                         ReadFileBaseFunc(fid, 0, buf, len, FALSE)
#define ReadFileAtOffset(fid, off, buf, len)            ReadFileBaseFunc(fid, off, buf, len, FALSE)
#define ReadFileAtOffsetBlockMode(fid, off, buf, len)   ReadFileBaseFunc(fid, off, buf, len, TRUE)

/*****************************************************************************
** Public variables
*****************************************************************************/

/* FileIO mutex */
extern MUTEX fileIOMutex;
extern MUTEX fileSystemMutex;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*
** These functions are already defined by Windows with different parameters.
*/
#ifndef _WIN32
extern INT32 WriteFile(UINT32 fileID, void *buffer, UINT32 length);
extern UINT32 GetFileSize(INT32 fid);
#endif

extern INT32 WriteFileAtOffset(UINT32 fileID, UINT32 sectorOffset, void *buffer,
                               UINT32 length);
extern INT32 ReadFileBaseFunc(UINT32 fileID, UINT32 sectorOffset, void *buffer,
                              UINT32 lengthOfBuf, UINT32 useMyBuffer);
extern INT32 RefreshDirectory(void);
extern INT32 FileSystemInitialized(void);

extern UINT32 GetFileSizeInBlocksIncHeader(INT32 fid);

extern void SendFileioErr(INT32 type, UINT8 api, INT32 rc, UINT8 status, UINT32 fid,
                          UINT16 wr_good, UINT16 wr_err);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _FIO_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
