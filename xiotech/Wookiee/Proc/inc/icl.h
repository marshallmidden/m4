/* $Id: icl.h 145879 2010-08-18 18:48:06Z m4 $ */
/**
******************************************************************************
**
**  @file       icl.h
**
**  @brief      Intercontroller Link related functions and variable definitions.
**
**  Copyright (c) 2006-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _ICL_H_
#define _ICL_H_

#include "XIO_Types.h"
struct DTMT;
struct ILT;
struct VRP;
struct MLMT;

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name   Constants can be grouped and a group name applied.
**/

/*
**  ICL port and ICL target numbers
**  Also see the corresponding asm definitions in system.inc
**/
#define ICL_TID0      8       /* ICL Target on controller#0  */
#define ICL_TID1      9       /* ICL Target on controller#1  */
#define ICL_PORT      4

#define ICL_TARGET(tid)    (((tid) == ICL_TID0) || ((tid) == ICL_TID1))
#define ICL_PRT(port)      ((port) == ICL_PORT)

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

#ifdef FRONTEND

typedef struct ICL_TGD_INFO
{
    UINT16 tid;
    UINT32 ipAddr;
    UINT32 netMask;
}ICL_TGD_INFO;

extern ICL_TGD_INFO icl_TgdInfo[];
#endif  /* FRONTEND */

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern UINT32 iclPortExists;

#ifdef FRONTEND
extern UINT32 iclIdentificationDone;
extern char iclIfrName[5];

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void ICL_CheckIclExistence(void);
extern void ICL_CreateIclTargets(void);
extern UINT32 ICL_IsIclPort(UINT8 port);
extern UINT32 ICL_IsIclMagLinkVRP(struct VRP *);
extern void ICL_SetIclMagLinkFlag(struct VRP *);
extern void ICL_ClearIclMagLinkFlag(struct VRP *);
extern void ICL_SendDg_Dump1(struct ILT *, struct DTMT *);
extern void ICL_LogEvent(UINT8 portState);
extern UINT32 ICL_IsVLinkDatagram(UINT32 srcCncSerial, UINT32 destCncSerial);
extern UINT32 ICL_GetIfaceIpAddr(char *intfHandle);
extern UINT32 ICL_GetIfaceNetMask(char *intfHandle);
extern UINT32 ICL_GetIfaceGateway(char *intfHandle);
extern void ICL_PortReset(UINT32);
extern void ICL_UpdateDmlPathStats(struct DTMT *pDtmt);
extern void ICL_LoopDown(UINT32 port);
extern struct DTMT *ICL_GetTargetPath(struct ILT *pILT);
extern void  ICL_NotifyIclPathMade(struct DTMT *pDtmt);
extern void  ICL_NotifyIclPathLost(struct DTMT *pDtmt);
extern void  ICL_Offline(void);
extern UINT8 ICL_onlyIclPathExists(struct MLMT *pMLMT);
#endif  /* FRONTEND */

#endif /* _ICL_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
