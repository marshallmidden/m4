/* $Id: xk_mapmemfile.h 130553 2010-03-04 17:33:12Z mdr $ */
/**
******************************************************************************
**
**  @file       xk_mapmemfile.h
**
**  @brief      Header for the module to allow us to map memory to files.
**
**  Copyright (c) 2004, 2009-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _XK_MAPMEMFILE_H_
#define _XK_MAPMEMFILE_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/


/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/


/*
******************************************************************************
** Public variables
******************************************************************************
*/

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Initialize mapped memory to file
**
**              Initialize mapped memory to file. If the file does not exist
**              it is created at the length specified. If the file does exist
**              it is retrieved. The pointer returned will be mapped to the
**              file at the start of the file.
**
**  @param      fName   - Name of file to use.
**  @param      length  - Length of Mapped region.
**  @param      fill    - Character pattern to fill if new.
**
**  @return     SUCCESS - pointer to the beginning of the mapped memory
**  @return     FAILURE - NULL
**
**  @attention  All writes to the memory will be written to the file by the OS.
**              If you want to immediately flush call MEM_FlushMapFile
**
******************************************************************************
**/
extern void *MEM_InitMapFile(const char *fName, INT32 length, UINT8 fill, void *start);


/**
******************************************************************************
**
**  @brief      Flush mapped memory to file
**
**  @param      pMemToFlush - pointer to start of mem segment to flush.
**  @param      length      - Length of region to flush.
**
**  @return     none
**
******************************************************************************
**/
extern void MEM_FlushMapFile(void *pMemToFlush, UINT32 length);


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _XK_MAPMEMFILE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
