/* $Id: ses.h 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       ses.h
**
**  @brief      SCSI Enclosure Services
**
**  Copyright (c) 2003-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _SES_H_
#define _SES_H_

#include "SES_Structs.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define SES_NO_BAY_ID       0xFFFF  /* ID for bay not defined.              */

/*
** The following constants are indicies into the devFlags array in the
** device information maps.
*/
#define SES_DI_DEV_TYPE         0   /* Flag for device type (SATA, FC, etc) */
#define SES_DI_MISC             1   /* Misc flags (bit positional)          */

/*
** Bit masks for the SES_DI_MISC flags field.
*/
#define SES_DI_MISC_SLOT_PAGE1  0x01   /* Discovery of drive slot via page 1   */
#define SES_DI_MISC_SLOT_INQ    0x02   /* Discovery of drive slot via inq 83   */
#define SES_DI_MISC_DIRECT      0x10   /* Directly addressable drive bay       */
#define SES_DI_MISC_EURO_PAGE1  0x20   /* Eurologic-type bay for page 1        */
#define SES_DI_MISC_XYRATEX     0x40   /* Xyratex SBOD                         */
#define SES_DI_MISC_VITESSE     0x80   /* Vitesse SAS expander                 */
#define SES_SLOT0               0   /* Slot 0                               */
#define SES_SLOT15             15   /* Slot 15                              */

/* Wait time before attempting to bypass a hung drive */
#define HANG_WAIT            2000   /* 2 seconds                            */

/* Hang Count thresholds for determining drive bypass                       */
#define LOW_HANG_THRESHOLD      5   /* 5 failed ops                         */
#define HIGH_HANG_THRESHOLD    15   /* 15 failed ops                        */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
struct PDD;
struct PCB;
struct DEV;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern struct PCB *S_bgppcb;
extern SES_DEV_INFO_MAP *SES_DevInfoMaps;
extern UINT16 SES_DIMEntries;
extern PSES_P82_XTEX SES_Page82[MAX_DISK_BAYS];
extern PSES_P82_XTEX SES_Page83[MAX_DISK_BAYS];

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void SES_BackGroundProcess(void);
extern void SES_UpdatePages(void);
extern void SES_StartBGProcess_c(void);
extern UINT8 SES_DirectlyAddressable(struct PDD *pSES);
extern UINT8 SES_GetDeviceType(struct PDD *pPDD);
extern void SES_PollDriveForEnclosure(UINT32 foo, UINT32 bar, struct PDD *pPDD, volatile UINT32* cnt, UINT32 logEntry);
extern void SES_GetDirectEnclosure(UINT32 foo, UINT32 bar, struct PDD *pdd, volatile UINT32* cnt, UINT32 logEntry);
extern void SES_GetSlotWithInq(UINT32 foo, UINT32 bar, struct PDD *pdd, volatile UINT32* cnt);
extern void SES_BypassCtrl(struct PDD *pdd);

#if defined(MODEL_7000) || defined(MODEL_4700)
extern void ISE_HandleStopIOSmartEvent (UINT8);
extern void ISE_HandleResumeIOSmartEvent (void);
extern int ISE_GetPage85(struct DEV *device, UINT32 *iseip1, UINT32 *iseip2, int retries);
#endif /* MODEL_7000 || MODEL_4700 */

#endif /* _SES_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
