/* $Id: magdrvr.h 145892 2010-08-18 23:06:18Z m4 $ */
/**
******************************************************************************
**
**  @file       magdrvr.h
**
**  @brief      Mag Driver Layer
**
**  To provide a device driver element to process host to MAGNITUDE
**  SCSI-FCP events and to manage their completion.
**
**  Copyright (c) 2003-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _MAGDRVR_H_
#define _MAGDRVR_H_

struct IMT;
struct CIMT;
union SCSI_COMMAND_FORMAT;
struct ILMT;
struct MAG_TBL;
struct SGL;
struct ILT;
struct INL2;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern struct IMT *mag_imt_head;
extern struct IMT *mag_imt_tail;
extern struct IMT *C_imt_head;
extern struct IMT *C_imt_tail;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void MAG_SrvLogout(struct CIMT *pCIMT, struct IMT *pIMT);
extern void mag1_vfymedia(union SCSI_COMMAND_FORMAT *, UINT8, struct IMT *,
                          struct ILMT *, struct ILT *, struct INL2 *);
extern void mag1_undef(union SCSI_COMMAND_FORMAT *, struct IMT *,
                          struct ILT *, struct INL2 *);
extern void lld1_undef(union SCSI_COMMAND_FORMAT *, struct IMT *,
                          struct ILT *, struct INL2 *);
extern void mag1_cmdcom(struct MAG_TBL const *, struct ILT *, struct INL2 *);


#endif /* _MAGDRVR_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
