/* $Id: async_nv.h 161678 2013-09-18 19:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       async_nv.h
**
**  @brief      To provide a means of handling Apool NV records.
**
**  Copyright (c) 2007 - 2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _ASYNC_NV_H_
#define _ASYNC_NV_H_

#include "vdd.h"
#include "system.h"
#include "XIO_Types.h"
#include "qu.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define   MAX_APOOL_EXP 5          /**< Maximum number of apool expansions*/
#define   APOOL_NV_DEBUG FALSE
#define   APOOL_OWNERSHIP_PRIORITY 205
#define   ASYNC_NVUPDATE_PRIORITY 156

/*
** NVRAM update types
*/
#define  AR_UPDATE_ALL          0
#define  AR_IMPLICIT            1
#define  AR_IMPLICIT_RSP        2
#define  AR_APOOL_EXPAND        3
#define  AR_ALINK_INIT          4
#define  AR_ALINK_DELETE        5
#define  AR_UPDATE_HEAD         6
#define  AR_UPDATE_TAIL         7

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** Non-volatile structures for Apool
**/

typedef struct APOOL_NV_HEADER
{
    UINT64  sequence_count;              /**< Current sequence count.*/
    UINT16  status;                      /**< Status bit field.*/
    UINT16  apool_id;                    /**< ID of this apool.*/
    UINT16  cur_head_element;            /**< Element containing current head pointer.*/
    UINT16  cur_tail_element;            /**< Element containing current tail pointer.*/
    UINT64  apool_size;                  /**< Total size of this apool in sectors.*/
    UINT32  time_threshold;              /**< Time bursting threshold.*/
    UINT32  mb_threshold;                /**< Mega Byte bursting threshold.*/
    UINT64  last_seq_count;              /**< Last read sequence count.*/
    UINT16  alink_count;                 /**< Number of alinks using this apool.*/
    UINT16  element_count;               /**< Number of elements in this apool.*/
    UINT16  element_entry_size;           /**< Element size         */
    UINT8   version;                     /**< Version of this record*/
    UINT8   reserved[3];                 /**< Reserved bytess.*/

} APOOL_NV_HEADER;

typedef struct APOOL_NV_ELEMENT
{
    UINT16  vid;                    /**< VID where this element resides.*/
    UINT16  status;                 /**< Current status bits */
    UINT16  apool_id;               /**< ID of the owning apool */
    UINT16  jump_to_element;        /**< The element to jump to */
    UINT64  length;                 /**< Length of this emement in sectors.*/
    UINT64  sda;                    /**< SDA for this element.*/
    UINT64  head;                   /**< Head pointer in sectors.*/
    UINT64  tail;                   /**< Tail pointer in sectors.*/
    UINT32  attributes;             /**< Place holder for things like compression, etc.*/
    UINT64  jump_offset;            /**< Offset from which shadow head jumped from. */
} APOOL_NV_ELEMENT;

typedef struct APOOL_NV_IMAGE
{
    UINT8 id[4];                             /**< Identification string */
    APOOL_NV_HEADER header;                  /**< Apool NV Header       */
    APOOL_NV_ELEMENT element[MAX_APOOL_EXP]; /**< Apool NV elements     */

}APOOL_NV_IMAGE;

/*
******************************************************************************
** Public variables
******************************************************************************
*/

extern APOOL_NV_IMAGE gTempImage;
extern UINT32 gAsyncNVRecovered;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT32 AR_SendAsyncNVToMirrorPartner(UINT16 update_type);
extern void AR_VerifyApoolNVImage (void);
extern UINT32 AR_RecoverApoolNVImage(UINT32);
extern void AR_SaveNVtoFile(void);
extern void AR_StartOwnershipTask(UINT16 apool_id, UINT16 vid);
extern void AR_ClearAsyncNVRAM(void);
extern void AR_NVUpdateAllfields(void);
extern void AR_DisownApool(UINT16 apool_id);
extern UINT32 AR_NVUpdate (UINT16 update_type);
extern UINT32 AR_ReceiveNVUpdate(APOOL_NV_IMAGE* pPkt, UINT16 update_type);
extern void AR_NVUpdateHeadFields(void);
extern void AR_NVUpdateTailFields(void);
extern void logSPOOLevent(UINT8 event_code, UINT8 severity,UINT32 errorCode,UINT32 value1,UINT32 value2);
extern void CT_LC_AR_NVUpdateTask(void);
extern void AR_QueueAsyncNVUpdate (UINT16 updateType);
extern QU gAsyncNVque;
void  ss_invalidate_snapshot(UINT16 ss_vid);
#endif /* _ASYNC_NV_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
