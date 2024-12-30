/* $Id: LKM_layout.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       LKM_layout.h
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef _LKM_LAYOUT_H_
#define _LKM_LAYOUT_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/* NOTE: Please change Proc/inc/system.inc if these are changed. */
/* NOTE: Please change CCB/Src/rm_val.c if these are changed. */

/**
**  @name   Base addresses for the three processes
**/
/* @{ */

#define NVRAM_BASESIZE          0x00400000      /* Size of mapped NVRAM area */


#define FE_BASE_ADDR            FRONT_END_PCI_START
#define BE_BASE_ADDR            BACK_END_PCI_START
#define CCB_BASE_ADDR           CCB_PCI_START

#ifdef FRONTEND
#define SHARELOC                FRONT_END_PCI_START
#else
#ifdef BACKEND
#define SHARELOC                BACK_END_PCI_START
#else
#define SHARELOC                CCB_PCI_START
#endif
#endif

/* Which memory section is for what use in the xio3d memory. */
#define XIO_CCB ccb_shm.rgn // 0
#define XIO_BE  be_shm.rgn  // 1
#define XIO_FE  fe_shm.rgn  // 2
/* @} */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

#define  WOOKIEE_ADMIN_UID   0
#define  WOOKIEE_ADMIN_GID   0
#define  WOOKIEE_USER_UID    1000 /* Wookiee */
#define  WOOKIEE_USER_GID    1000 /* Wookiee */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
struct app_shm
{
    const char  *file;  /* Path to hugetlb file */
    const char  *name;  /* Name of region */
    const int   prot;   /* Needed protection */
    int     rgn;    /* Underlying region number */
    int     fd;     /* File descriptor */
};

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern struct app_shm   ccb_shm;
extern struct app_shm   be_shm;
extern struct app_shm   fe_shm;
extern struct app_shm   xio3d_shm;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _LKM_LAYOUT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
