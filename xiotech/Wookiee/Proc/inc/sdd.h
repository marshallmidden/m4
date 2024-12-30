/* $Id: sdd.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       sdd.h
**
**  @brief      Server device descriptors
**
**  To provide a common means of defining the Server Device Description
**  (SDD) structure.
**
**  Copyright (c) 2001-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _SDD_H_
#define _SDD_H_

#include "XIO_Types.h"
#include "globalOptions.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/*
**  Status field definitions
*/
#define SD_NONX         0           /* Non-existent                         */
#define SD_OPER         1           /* Operational                          */
#define SD_INOP         2           /* Nonoperational                       */

/*
** Server attribute bit definitions
*/
#define SD_UNMANAGED    0           /* Server is unmanaged                  */
#define SD_SELTARGET    1           /* Select target in this sdd for mapping*/
#define SD_XIO         30           /* Server is XIOtech Controller         */
#define SD_DEFAULT     31           /* Server has default LUN mappings      */

/*
******************************************************************************
** Public variables
******************************************************************************
*/

typedef struct SDD
{
    UINT16      sid;                /**< Server ID                          */
    UINT16      numLuns;            /**< Number of LUNs                     */
    UINT16      tid;                /**< Target ID for this server          */
    UINT8       status;             /**< Server status                      */
    UINT8       pri;                /**< HAB priority                       */
    UINT32      attrib;             /**< Server attributes                  */
    UINT32      session;            /**< Session identifier                 */
                                    /**< QUAD BOUNDARY                  *****/
    UINT64      reqCnt;             /**< Server request count               */
    UINT16      linkedSID;          /**< Linked Server ID                   */
    UINT8       rsvd26[2];          /**< Reserved                           */
    UINT32      owner;              /**< Serial number of owning controller */
                                    /**< QUAD BOUNDARY                  *****/
    UINT64      wwn;                /**< World wide name of server          */
    UINT8       rsvd40[8];          /**< Reserved                           */
                                    /**< QUAD BOUNDARY                  *****/
    UINT8       name[16];           /**< Server name                        */
                                    /**< QUAD BOUNDARY                  *****/
#if ISCSI_CODE
    UINT8       i_name[256];        /**< iSCSI Server name                  */
#endif
    struct LVM* lvm;                /**< LUN mappings                       */
    struct LVM* ilvm;               /**< Invisible LUN mappings (LUN FF)    */
                                    /**< Do not change the order of the two */
                                    /**< lvm pointers.  Order is important. */
} SDD;

/** SDD index table                                                         */
typedef struct SDX
{
    UINT16          count;          /**< Number of servers                  */
    UINT8           rsvd2[2];       /**< RESERVED                           */
    SDD*            sdd[MAX_SERVERS]; /**< Array of servers                 */
} SDX;

#endif /* _SDD_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
