/* $Id: xl.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       xl.h
**
**  @brief      Translation Layer descriptors
**
**  To provide definitions for Translation Layer routines.
**
**  Copyright (c) 2002-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _XL_H_
#define _XL_H_

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define ISP_SCSI_CMD            0x10    /* SCSI CDB received                */
#define ISP_IMMED_NOTIFY_CMD    0x11    /* Immediate Notify received        */
#define ISP_OFF_LINE_CMD        0x12    /* Offline event                    */
#define ISP_LOG_OFF_CMD         0x13    /* initiator logoff                  */
#define ISP_RESET_INIT_CMD      0x14    /* Reset and initialize port event  */
#define ISP_DISABLE_VPORT_CMD   0x15    /* disable virtual port             */
#define ISP_SCSI_CMD_TMF        0x16    /* SCSI TMF received                */
#define ISP_ABTS_RECEIVE        0x54    /* ABTS received                    */

#endif /* _XL_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
