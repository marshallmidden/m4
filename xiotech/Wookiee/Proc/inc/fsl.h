/* $Id: fsl.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       fsl.h
**
**  @brief      This is a Front-end Sub-Layer for iSCSI
**
**  Copyright (c) 2005-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#ifndef _FSL_ISCSI_H_
#define _FSL_ISCSI_H_

#include "target.h"
#include "XIO_Types.h"

struct TMT;
struct IMT;
struct ILT;
struct LTMT;

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define FSL_LOGIN   0
#define FSL_LOGOUT  1

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define MAX_IDDS    128

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
#define MAX_WERR                 3
#define MAX_RERR                 3

/*
** IDD FLAGS
*/
#define   IDF_INIT               0
#define   IDF_READY              1
#define   IDF_LOGIN              2
#define   IDF_SESSION            3
#define   IDF_DEVICE             4
#define   IDF_PATH               5
#define   IDF_LOGOUT             6
#define   IDF_ABORT              7
#define   IDF_FREE               8
#define   IDF_RETRY              15

typedef struct _initiator_device_descrpitor
{
    UINT16  flags;
    UINT8   werr;
    UINT8   rerr;
    UINT8   lid;
    UINT8   ptg;
    UINT16  port;
    INT32   sg_fd;
    UINT8   sg_name[16];
    UINT64  i_name;
    UINT64  t_name;
    UINT64  t_pname;
    UINT32  t_ip;
    UINT32  i_ip;
    struct TMT *pTMT;
    struct ILT *iltQ;
    void*   pTLogoutPcb;
} IDD;

/*
** Errors returned from SG interface to the proc level
*/
#define SGE_SUCCESS     4   /* Success      */
#define SGE_CS          8   /* Check Condition  */
#define SGE_NOCS        12  /* non check status */
#define SGE_IOE         16  /* IO Error     */
#define SGE_ME          20  /* Miscellaneous error  */
#define SGE_ABORT       24  /* Abort from iSCSI */
#define SGE_ONLINE      28  /* Session UP       */
#define SGE_OFFLINE     32  /* Session Down     */
#define SGE_TIMEOUT     36  /* Timer Expiration */
#define SGE_TASKABORT   40  /* Task Abort       */

#define SGE_STOP        44  /* STOP Event       */
#define SGE_CONTINUE    48  /* CONTINUE Event   */
#define SGE_LOGOUT      52  /* LOGOUT   Event   */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern void iscsi_dprintf(const char *fmt, ...) __attribute__((format(printf,1,0)));

extern void    fsl_Init(UINT32 a, UINT32 b, UINT8 port);
extern UINT32  fsl_InitPort(UINT8 port);
extern UINT32  fsl_ResetPort(UINT8 port, UINT32 reason);
extern UINT8   fsl_PortType(UINT8 port);
extern UINT8   fsl_TargetType(UINT16 tid);
extern void    fsl_SrvLogin(UINT16 tid, UINT16 tsih, UINT64 wwn);
extern void    fsl_SrvLogout(UINT16 tid, UINT16 tsih, UINT64 wwn);
extern struct IMT *fsl_findIMT(UINT64 wwn, UINT16 tid, UINT16 tsih);
extern void    fsl_UpdName(struct IMT *pIMT);
extern void    fsl_UpdSID(UINT64 wwn, UINT16 sid);
extern void    fsl_LogZoneInquiry(struct IMT *pIMT);
extern void    fsl_logServer(UINT64 wwn, UINT16 tid, UINT8* i_name, UINT8 state);
extern TAR     *fsl_get_tar(UINT16 tid);
extern UINT16  fsl_gen_tsih(UINT16 tid);
extern void    fsl_iscsi_scsi(struct ILT* pIlt);
extern void    fsl_ilt_cb(UINT32 status, struct ILT *pSecILT);
extern void    fsl_tmf_cb(UINT32 status, struct ILT *pSecILT);
extern UINT64  iSCSI_GetISID(UINT16 tid, UINT16 tsih);
extern UINT8   *iSCSI_GetSrvName(UINT16 tsih, UINT16 tid);
extern void    wwn2naa(UINT64 wwn, UINT8 *pNaa);
extern void    fsl_iscsidCb(UINT32 events, void *pRef);
extern UINT32  fsl_isIFP(UINT16 port, UINT16 lid);
extern UINT32  fsl_sgTx(struct ILT *pILT);
extern void    fsl_sgio_cb(UINT32 events, void *pRef);
extern UINT64  fsl_getPortWWN(UINT16 port, UINT16 lid);
extern UINT64  fsl_getNodeWWN(UINT16 port, UINT16 lid);
extern void    fsl_resetLPMap(UINT16 port);
extern void    fsl_initLPMap(UINT16 port);
extern void    fsl_login(struct ILT *pILT);
extern void    fsl_logout(UINT16 port, UINT16 lid);
extern void    fsl_updateTMT(UINT16 port, UINT16 lid, struct TMT *pTMT);
extern void    fsl_assocIMT_LTMT(struct IMT *pIMT, struct LTMT *pLTMT);

extern void CT_LC_fsl_ilt_cb(UINT32, struct ILT *pIlt);
extern void CT_LC_fsl_tmf_cb(UINT32 status, struct ILT *pSecILT);
extern bool fsl_is_xioInit(UINT8* i_name);
extern IDD* getIdd(UINT16 portId, UINT16 devId);

extern void I_logout(struct TMT *pTMT);
extern void I_login(UINT16 port, UINT16 lid);
extern void fsl_updatePaths(void);
extern UINT32 fsl_getPID(UINT16 port, UINT16 lid);

#endif /* _FSL_ISCSI_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
