/* $Id: ispc.c 164645 2014-11-26 18:14:40Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       ispc.c
**
**  @brief      ISP 'C' functions
**
**  To provide configuration and other miscellaneous hardware-related services
**  for the QLogic ISP24XX chip.
**
**  Copyright (c) 2001 - 2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "def.h"
#include "dev.h"
#include "options.h"
#include "chn.h"
#ifdef BACKEND
#include "fabric.h"
#include "loop.h"
#include "prp.h"
#include "ise.h"
#else  /* BACKEND */
#include "cdriver.h"
#include "ilmt.h"
#include "icimt.h"
#endif /* BACKEND */
#include "ecodes.h"
#include "fc.h"
#include "ficb.h"
#include "fr.h"
#if FE_ISCSI_CODE
#include "fsl.h"
#endif /* FE_ISCSI_CODE */
#include "CT_defines.h"
#include "icl.h"
#include "ilt.h"
#include "isp.h"
#include "kernel.h"
#include "li_pci.h"
#include "XIO_Macros.h"
#include "LOG_Defs.h"
#include "loop.h"
#include "LL_LinuxLinkLayer.h"
#include "misc.h"
#include "MR_Defs.h"
#include "nvr.h"
#include "OS_II.h"
#include "pcb.h"
#include "pm.h"
#include "portdb.h"
#include "qrp.h"
#include "QU_Library.h"
#include "scsi.h"
#include "string.h"
#include "system.h"
#include "target.h"
#include "XIO_Types.h"
#include "XIO_Const.h"
#include "XIO_Std.h"
#include "xli.h"
#include "xl.h"

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <byteswap.h>

extern struct pci_devs gPCIdevs[XIO3D_NIO_MAX];
extern struct CHN* P_chn_ind[MAX_PORTS];
#include "CT_change_routines.h"
#include "mem_pool.h"
#include <time.h>

extern char *millitime_string(void);

/* The following are for created tasks. */
extern void CT_LC_isp_loopEventHandler(int);
extern void CT_LC_isp_portOnlineHandler(int);
extern void CT_LC_isp_registerVports(int);
extern void CT_LC_isp_resetProcess(int);
/*extern void CT_LC_isp_loopEventHandler(int);*/

/* External routines. */
extern void SES_StartBGProcess_c(void);

/* NOTE: The following copied from logdef.h in CCB/Inc. */
typedef unsigned char CDB_T;
#define CDB_LENGTH      16

/* NOTE: The following copied from logdef.h in CCB/Inc. */
/* IOCB Timeout */
struct LOG_IOCB_TO_PKT
{
    UINT8       port;
    UINT8       rsvd1;
    UINT8       proc;
    UINT8       rsvd2;
    UINT8       iocb;
    UINT8       rsvd3;
    UINT32      ilt;
    UINT16      lun;
    UINT32      lid;            /* Loop ID  */
    UINT64      wwn;
    CDB_T       cdb[CDB_LENGTH];
};

/* Complete log packet for IOCB timeout. */
struct ISP_IOCB_TO_PKT
{
    struct LOG_HEADER_PKT header;
    struct LOG_IOCB_TO_PKT timeout;
};

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define KABOOM  UINT32 *kaboom=NULL; kaboom[0]=1;
#define ISP2400_TARGET_IO_LOG   FALSE
#define ISP_TARGET_IO_TIMING    FALSE
#define ISP_DEBUG_INFO          FALSE

#define POWER_UP_WAIT       60  /* 60 seconds                   */
#define EFM                 0x6D6665    /* "efm" firmware               */
#define EF                  0x6665      /* "ef"  firmware               */
#define MID                 0x64696D    /* "mid"  firmware              */

#define VPINDX_MAPSIZE      16

#define MID_FW              0x4000
#define ATTRIB_MID_2400     0x0004
#define BUFFER_SIZE         64
#define HEADER_SIZE         128

#define EMERGENCY_IOCB      1
#define AE8017EN            0x0080
#define AE8048EN            2
#define AE8049EN            0x2000
#define LIPIMMED            0x20
#define INTERNAL_ABORT      1

#define IOCB_SIZE           64

#define STATUS0             0x03        /* Status type 0            */
#define MARKER_IOCB         0x04        /* marker                   */
#define ATIO7               0x06        /* accept target io 6       */
#define INOTIFY             0x0D        /* immediate notify         */
#define NTACK               0x0E        /* notify acknowledge       */
#define STATUS0_CONT        0x10        /* Status type 0 contination */
#define CTIO7               0x12        /* Continue Taget IO 7      */
#define TMFIOCB             0x14        /* Task Managment IOCB      */
#define CMIO7               0x18        /* command type 7 iocb      */
#define VPCTRL              0x30        /* Virtual Port Control IOCB */
#define MVPC                0x31        /* Modify Virtual Port Config IOCB      */
#define IMRIA               0x32        /* Report ID acquisition IOCB           */
#define ABORTIOCB           0x33        /* ABORT IOCB IOCB          */
#define MSIOCB              0x29        /* Management Server IOCB   */
#define MBIOCB              0x39        /* Mailbox IOCB             */
#define CMIO6               0x48        /* command type 7 iocb      */
#define PLOGXIOCB           0x52        /* LOGIN LOGOUT IOCB        */
#define ELS_PASS_IOCB       0x53        /* ELS pass through IOCB    */
#define ABTSRCV             0x54        /* ABTS received            */
#define ABTSACK             0x55        /* ABTS ack                 */
#define LINIT               0x7000      /* Loop Initialize (LINIT) subcommand   */

#define IOCB_WITH_WAIT_ILT_TYPE 0xDA

#define OPTNBIT             1   /* PLOGI issued only when not logged in */

#define QLDUMP      1

#ifdef  BACKEND
#define LC_PROCSTR "be"
#else  /* BACKEND */
#define LC_PROCSTR "fe"
#endif /* BACKEND */

/*
******************************************************************************
** Defines - Copies from physical_isp.c
******************************************************************************
*/
// #define RECOVERY_LIPRESET_ONCE 1
#define RECOVERY_DO_TARGET_RESET 2

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define hey(i)          { ++ispdebug[i]; }

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/
/* mpn defined in def_isp.c */
extern struct MRFEPORTNOTIFY_REQ mpn;

/* Login/Logout IOCB (0x52) */
struct portlogoiocb_t
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      handle;
    UINT16      status;
    UINT16      lid;
    UINT16      controlflags;
    UINT8       vpindex;
    UINT8       rsvd0;
    UINT16      portid0_15;
    UINT8       portid16_23;
    UINT8       rspsize;
    UINT32      portname;
    UINT64      portWWN;
    UINT32      rsvd1[8];
};

/* Login/Logout IOCB (0x52) */
#define ISP2400PLIO_NOLINK        0x01
#define ISP2400PLIO_NOIOCB        0x02
#define ISP2400PLIO_NOXCB         0x03
#define ISP2400PLIO_CMDFAILED     0x04
#define ISP2400PLIO_NOFABRIC      0x05
#define ISP2400PLIO_FWNOTREADY    0x07
#define ISP2400PLIO_NOTLOGGEDIN   0x09
#define ISP2400PLIO_NOPCB         0x0A

#define ISP2400PLIO_ELSREJECT     0x18
#define ISP2400PLIO_CMDPARMERR    0x19
#define ISP2400PLIO_PIDUSED       0x1A
#define ISP2400PLIO_NPORTUSED     0x1B
#define ISP2400PLI_NOHANDLE       0x1C
#define ISP2400PLI_NOFLOGIACC     0x1F

struct portlogiiocb_t
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      handle;
    UINT16      status;
    UINT16      lid;
    UINT16      controlflags;
    UINT8       vpindex;
    UINT8       rsvd0;
    UINT16      portid0_15;
    UINT8       portid16_23;
    UINT8       rspsize;
    UINT32      commonfeatures;
    UINT32      ioparameter1;
    UINT32      rsvd2[9];
};

/* Asycnronous Event Entry */
struct ev_t
{
    UINT32      event;
    UINT32      data;
};

/*#define STEVE_DEBUG*/

#ifdef STEVE_DEBUG
struct
{
    UINT32     *start;
    UINT32     *now;
    UINT32     *end;
    UINT32      rsvd;
} qtrace[4];
#endif /* STEVE_DEBUG */

struct fwhb_t
{
    UINT32      enabled;
    UINT32      previous;
    UINT32      current;
    UINT32      rsvd;
}           (*ispfwhb)[MAX_PORTS] = NULL;

struct
{
    UINT32      timeStamp;
    UINT32      reason;
    UINT32      vpMap;
    UINT32      rsvd;
} ispResetLog[MAX_PORTS];


#ifdef  FRONTEND
static void printBuffer1(UINT8 *ptr, INT32 length)
{
    int         i;

    if (ptr == NULL)
    {
        return;
    }

    fprintf(stderr, "\nPRINTING BEGINS on pointer %p\n", ptr);
    for (i = 0; i < length; i = i + 4)
    {
        fprintf(stderr, "%08X  ----- 0x%02hx 0x%02hx 0x%02hx 0x%02hx    ", i,
                ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
        i += 4;
        fprintf(stderr, "0x%02hx 0x%02hx 0x%02hx 0x%02hx    ",
                ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
        i += 4;
        fprintf(stderr, "0x%02hx 0x%02hx 0x%02hx 0x%02hx    ",
                ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
        i += 4;
        fprintf(stderr, "0x%02hx 0x%02x 0x%02hx 0x%02hx\n",
                ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
    }
    fprintf(stderr, "\nPRINTING FINISH\n");
}
#endif /* FRONTEND */


/*
******************************************************************************
** Private variables
******************************************************************************
*/

#ifdef STEVE_DEBUG
UINT32      mailboxIssue[4];
UINT32      mailboxDone[4];
UINT32      ispdebug[16];
UINT32      mbxIndx;
UINT16      mbxFailed[32][16];
UINT32     *isp_respBuffer[MAX_PORTS];
#endif /* STEVE_DEBUG */

UINT32      ispPow = 0;         /* MAXISP - Power on Wait           */

#ifdef FRONTEND
UINT32      ispCp;              /* Initialized with Control Port    */
UINT32      iscsimap = 0;       /* iSCSI Port bitmap                */
UINT8       serverdbOwnerbyVpid[MAX_PORTS][MAX_DEV];
#endif /* FRONTEND */
PORT_ID_LIST *nphandleDB[MAX_PORTS][HANDLE_DB_SIZE];    /*nphandle handle db for front end target code */
UINT32      nphandledbentrycount[MAX_PORTS][HANDLE_DB_SIZE];

#define FABRIC

PCB        *isplepcb[MAX_PORTS];        /* Port loop event PCB              */
PCB        *isppfpcb[MAX_PORTS];        /* Port failure handler PCB         */
PCB        *isponpcb;           /* Port online handler PCB          */
UINT32      ispLid[MAX_PORTS];  /* HBA loop ID                      */
UINT32      ispConnectionType[MAX_PORTS];       /* Connection Type                  */
UINT32      ispprc[MAX_PORTS];  /* port reset count                 */
UINT32      ispSysErr[MAX_PORTS];       /* system error count               */
UINT8       ispInterrupt[MAX_PORTS];    /* ISP interrupt flag               */
UINT16      ispLipIssued[MAX_PORTS];    /* ISP LIP issued                   */
UINT8       ispFailedPort[MAX_PORTS];
UINT32      ispOnline;          /* MAXISP bitmap                    */
UINT32      ispofflfail;        /* MAXISP bitmap                    */
UINT8       ispPortAssignment[MAX_TARGETS];     /* Target port assignment           */
UINT32      onlineTimer[MAX_PORTS];
/* Anchors for Targets + for ICL target */
TAR        *tar[MAX_PORTS + MAX_ICL_PORTS];
/* Abort of tar[] linked list processing flag. */
UINT32      tar_link_abort[MAX_PORTS + MAX_ICL_PORTS];
ISP_REV    *isprev[MAX_PORTS + MAX_ICL_PORTS];  /* ISP revision information         */

UINT32      id_acquired;        /* VP ID acquired                   */
UINT32      ispSeq;

#ifdef FABRIC
UINT8       fc4Failed[MAX_PORTS];       /* Register FC-4 failed flag        */
PCB        *fc4pcb[MAX_PORTS];  /* FC4 registration PCB             */
#endif /* FABRIC */
UINT32      ispLastLIP[MAX_PORTS];
PDB        *portdb[MAX_PORTS];  /* Port database anchors            */
QU          isp_RCVIO_queue[MAX_PORTS];


struct
{
    UINT16      commandResourceCount;
    UINT16      immedNotifyCount;
    UINT16      exchangeBufferCount;
    UINT16      originalExchangeBufferCount;
    UINT16      iocbCount;
    UINT16      originalIocbCount;
} isprc[MAX_PORTS];

struct
{
    UINT16      originalfreeTXCBbufferCount;
    UINT16      currentfreeTXCBbufferCount;
    UINT16      currentfreeXCBbufferCount;
    UINT16      originalfreeXCBbufferCount;
    UINT16      currentfreeIOCBbuffercount;
    UINT16      originalfreeIOCBbuffercount;
} isprc_2400[MAX_PORTS];


struct ISP_DUMP ispdump[MAX_PORTS];     /* ISP dump data structs            */

/* #define ISP_DEBUG_FWSTATE */
#ifdef ISP_DEBUG_FWSTATE
UINT16      gFirmwareState;     /* ISP fw state (on force sys err)  */
#endif /* ISP_DEBUG_FWSTATE */

/* Mailbox command timeout values. */
const UINT8 isp_mb_timeout[] =
{
    10, 10, 10, 10, 10, 10, 10, 10,     /* Mailbox 0x00 - 0x07  */
    10, 10, 10, 10, 10, 10, 10, 10,     /* Mailbox 0x08 - 0x0F  */
    10, 10, 10, 10, 10, 20, 10, 10,     /* Mailbox 0x10 - 0x17  */
    10, 10, 10, 10, 10, 10, 10, 10,     /* Mailbox 0x18 - 0x1F  */
    10, 10, 10, 10, 10, 10, 10, 10,     /* Mailbox 0x20 - 0x27  */
    10, 10, 10, 10, 10, 10, 10, 10,     /* Mailbox 0x28 - 0x2F  */
    10, 10, 10, 10, 10, 10, 10, 10,     /* Mailbox 0x30 - 0x37  */
    10, 10, 10, 10, 10, 10, 10, 10,     /* Mailbox 0x38 - 0x3F  */
    10, 10, 10, 10, 10, 10, 10, 20,     /* Mailbox 0x40 - 0x47  */
    10, 10, 10, 10, 10, 10, 10, 10,     /* Mailbox 0x48 - 0x4F  */
    10, 10, 10, 10, 60, 10, 10, 10,     /* Mailbox 0x50 - 0x57  */
    10, 10, 10, 10, 10, 10, 10, 10,     /* Mailbox 0x58 - 0x5F  */
    10, 10, 10, 20, 10, 10, 10, 10,     /* Mailbox 0x60 - 0x67  */
    10, 10, 10, 20, 10, 10, 60, 60,     /* Mailbox 0x68 - 0x6F  */
    10, 60, 10, 10, 10, 10, 10, 10,     /* Mailbox 0x70 - 0x77  */
    10, 10, 10, 10, 10, 10, 10, 10      /* Mailbox 0x78 - 0x7F  */
};

#ifndef PERF
/* Mailbox command execution time. */
UINT8       isp_mb_time[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x00 - 0x07  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x08 - 0x0F  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x10 - 0x17  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x18 - 0x1F  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x20 - 0x27  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x28 - 0x2F  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x30 - 0x37  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x38 - 0x3F  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x40 - 0x47  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x48 - 0x4F  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x50 - 0x57  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x58 - 0x5F  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x60 - 0x67  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x68 - 0x6F  */
    0, 0, 0, 0, 0, 0, 0, 0,     /* Mailbox 0x70 - 0x77  */
    0, 0, 0, 0, 0, 0, 0, 0      /* Mailbox 0x78 - 0x7F  */
};
#endif /* PERF */

#ifdef  QLDUMP
static struct
{
    QRP        *head;
    QRP        *tail;
} saved_isp_cmds[MAX_PORTS];
#endif /* QLDUMP */

/*
** 2400 work queue. This queue orders work for the response
** and atio queues by capturing qindex values at the time
** of the corresponding interrupt.
**
** Each entry in the work queue is the risc2host status value.
*/
#define MAX_2400_WORK   512

static struct
{
    UINT16      in;
    UINT16      out;
    UINT32      work[MAX_2400_WORK];
} isp2400_queue[MAX_PORTS];

/*
 *      This table is used to translate from the 7-bit Loop Identifier
 *      to AL-PA addresses. The value of 127 indicates unassigned values.
 *
 *      From manual FCAL, Annex K.
 */
static const UINT8 lid2alpa_tbl[128] = {
    /*          0/8  1/9  2/a  3/b  4/c  5/d  6/e  7/f      */

    0xef, 0xe8, 0xe4, 0xe2, 0xe1, 0xe0, 0xdc, 0xda,     /* 00 - 07 */
    0xd9, 0xd6, 0xd5, 0xd4, 0xd3, 0xd2, 0xd1, 0xce,     /* 08 - 0f */
    0xcd, 0xcc, 0xcb, 0xca, 0xc9, 0xc7, 0xc6, 0xc5,     /* 10 - 17 */
    0xc3, 0xbc, 0xba, 0xb9, 0xb6, 0xb5, 0xb4, 0xb3,     /* 18 - 1f */
    0xb2, 0xb1, 0xae, 0xad, 0xac, 0xab, 0xaa, 0xa9,     /* 20 - 27 */
    0xa7, 0xa6, 0xa5, 0xa3, 0x9f, 0x9e, 0x9d, 0x9b,     /* 28 - 2f */
    0x98, 0x97, 0x90, 0x8f, 0x88, 0x84, 0x82, 0x81,     /* 30 - 37 */
    0x80, 0x7c, 0x7a, 0x79, 0x76, 0x75, 0x74, 0x73,     /* 38 - 3f */
    0x72, 0x71, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69,     /* 40 - 47 */
    0x67, 0x66, 0x65, 0x63, 0x5c, 0x5a, 0x59, 0x56,     /* 48 - 4f */
    0x55, 0x54, 0x53, 0x52, 0x51, 0x4e, 0x4d, 0x4c,     /* 50 - 57 */
    0x4b, 0x4a, 0x49, 0x47, 0x46, 0x45, 0x43, 0x3c,     /* 58 - 5f */
    0x3a, 0x39, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31,     /* 60 - 67 */
    0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x27, 0x26,     /* 68 - 6f */
    0x25, 0x23, 0x1f, 0x1e, 0x1d, 0x1b, 0x18, 0x17,     /* 70 - 77 */
    0x10, 0x0f, 0x08, 0x04, 0x02, 0x01, 0x00, 0xff      /* 78 - 7f */
};

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern TGD *T_tgdindx[MAX_TARGETS];
extern QRP *volatile ispdefq[8];
extern UINT8 dbflags[MAX_PORTS][MAX_DEV];
extern UINT32 sessids[MAX_PORTS];

#ifdef FRONTEND
extern UINT8 intlock[MAX_PORTS];
extern UINT32 req_cnt;
extern ICIMT *I_CIMT_dir[MAX_ICIMT];
#endif /* FRONTEND */
extern UINT32 ispaywt;          /* MAXISP - bitmap */
extern UINT32 isprqwt;          /* MAXISP - bitmap */

#ifdef FABRIC
extern UINT32 ispfflags;        /* MAXISP - bitmap */
extern UINT32 fc4flgs;          /* MAXISP - bitmap */
#endif /* FABRIC */
UINT8       gRegTargetsOK = FALSE;      /* Flag to show OK for Regular  Targets on FE */

extern UINT16 hba_q_cnt[MAX_PORTS];
extern ILT *ilthead[];
extern ILT *ilttail[];

PCB        *atiomonpcb[MAX_PORTS];      /* PCB to handle ATIO requests by port. */

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
void        isp$complete_io(UINT32 status, ILT * ilt, void *iocb);

#ifdef FRONTEND
void        I_recv_online(UINT32 port, UINT32 lid, UINT32);
void        I_recv_offline(UINT32 port);
UINT16      I_get_lid(UINT8 port);
void        I_ChangeNotification(UINT16 port, UINT32 alpa);
void        I_ChangeDB(UINT16 port, UINT16 lid, UINT16 code);
void        C_recv_scsi_io(ILT * ilt);
void        DLM$PortReady(void);
void        ispcompio(void);
void        ISP_NotifyAck(UINT8 port, ILT * pILT);
#else
void ON_BEBusy(UINT16 pid, UINT32 TimetoFail, UINT8 justthisonepid);
void ON_BEClear(UINT16 pid, UINT8 justthisonepid);
#endif /* FRONTEND */

static void isp_check_thread(UINT8 port);
void        isp_loopEventHandler(UINT32 a, UINT32 b, UINT8 port);
void        isp_getResourceCounts(UINT8 port, UINT16 vpindex);
void        isp_thread_ilt(UINT8, ILT *);
ILT        *isp_unthread_ilt(UINT8, ILT *);
#ifdef BACKEND
void        print_scsi_cmd(DEV *, PRP *, const char *);
#endif  /* BACKEND */
ILT        *isp_unthread_ilt_1(ILT *);
void        isp_portOnlineHandler(void);
void        isp_registerVports(UINT32 a, UINT32 b, UINT8 port);
UINT32      isp_registerFc4(UINT8 port, UINT32 fc4Type, TAR * pTar);
void        isp_process_mbx(UINT8, UINT16);
void        isp_pciConfigError(UINT32, UINT32, UINT8, UINT32, UINT32);
void        isp_ClearFC4TypesRegistration(UINT32);
void        ISP_GenOffline(UINT32);
void        ISP_LoopDown(UINT32);
void        ISP_ProcessOnline(UINT32, struct ev_t *);
void        ISP_LoopUp(UINT8, struct ev_t *);
void        ISP_monitor_async(UINT32, UINT32, UINT8);
void        ISP_monitor_atio(UINT32, UINT32, UINT8);
void        isp_exec_cmd_sri(UINT8, QRP *, UINT32);
UINT32      isp_removeAll(UINT8, TAR *);
static UINT32 exec_iocb_wait(UINT8 , ILT * , UINT32 , UINT16 *, UINT32 );
#ifdef FRONTEND
void        ISP_RCV_IO_Queue_exec(UINT32, UINT32, UINT8);
static UINT32 isp2400_processVpControl(UINT8, struct ISP2400_VPCONTROL *);
static void isp_processIdAcquisition(UINT8 port, VPRID_IOCB * iocb);
void        isp2400_processIdAcquisition(UINT8 port, VPRID_IOCB * iocb);
static void isp2400_abort_exchange_unknown(UINT16 port, UINT32 exchange,
                                           UINT16 ox_id, UINT8 attribute2400,
                                           UINT32 alpa);
void        isp2400_process_inotify(UINT16 port, ISP2400_INOTIFY * iocb);
void        isp2400_process_atio7(UINT16 port, ISP2400_ATIO7 * iocb);
UINT32      ISP_ChkIfPeerTarget(UINT64 wwn);
static void isp2400_process_abts(UINT16 port, ISP2400_ABTS * iocb);
static UINT32 isp2400_process_ctio7(UINT16 port UNUSED, ISP2400_CTIO7_STATUS * iocb);
#endif /* FRONTEND */
static UINT32 ISP2400_HandleElsIocb(UINT8 port, void *iocb);
UINT16      isp_findVpIndex(UINT8, UINT16);
UINT16      isp_getDataRate(UINT8);
UINT16      isp_SetFirmwareOptions(UINT8);
UINT16      isp2400_GetAndSetFirmwareOptions(UINT8);
UINT16     *isp_get_iocb(UINT8, UINT32 *);
TGD        *isp_findTarget(UINT8);
void        isp_resetProcess(UINT32, UINT32, UINT8, UINT32, UINT32 *, UINT32 *);
void        isp_startResetProcess(UINT32, UINT32 *, UINT32);
void        isp_build_targets(UINT8);
void        isp_MailboxTimeoutCheck(UINT32);
void        isp_monitorFwhb(void);
UINT16      isp_loadQFW(UINT8, UINT16);
static UINT32 isp2400_verifyFW(UINT8, QRP *, UINT32);
static UINT32 isp2400_loadQFW(UINT8, QRP *);
static UINT16 isp2400_sendTMF(UINT16, UINT16, UINT16, UINT32);
UINT32      ISP2400_SendSCR(UINT8 port, UINT16 vpindex);
void        i_wait(UINT32);

#ifdef QLDUMP
static void ISP2400_DumpQL(UINT8 port, UINT32 reason);
static void isp2400_GetRegs(UINT32 *pBuf, volatile UINT32 *pReg, UINT32 count);
#endif /* QLDUMP */
UINT16      isp_ChipSoftReset(UINT32);
static UINT32 ISP2400_HandleAsyncEvent(UINT32 port, UINT32 r2hstatus);
UINT32      ISP2400_HandleResponseQUpdate(UINT32 port, UINT32 indx);
UINT32      ISP2400_HandleATIOQUpdate(UINT32 port, UINT32 indx);
static UINT32 ISP2400_SoftReset(UINT32);
UINT16      isp_PauseRISC(UINT32);
UINT32      isp_softReset(UINT32, UINT32);
UINT16      ISP_lunReset(UINT8, UINT32);
UINT16      ISP_AbortIOCB(UINT8, UINT32, UINT16, UINT32);
UINT16      ISP_AbortQueue(UINT8, UINT32, UINT16);
UINT16      ISP_ClearTaskSet(UINT8, UINT32, UINT16);
UINT16      ISP_AbortTaskSet(UINT8, UINT32, UINT16);
void        isp_portFailureHandler(UINT32, UINT32, UINT8);
void        isp_reportOnlinePorts(UINT32);
UINT32      isp_mailboxIOCB(UINT8, UINT32, QRP *);
static void isp_processMailboxIOCB(UINT8, struct ISP2400_MBIOCB *);
UINT32      isp2400_ReadGPIOD(UINT8);
void        isp_exec_cmd(UINT8, QRP *, UINT32);
UINT32      isp_sendLFA(UINT8, void *, UINT16, UINT16);
UINT32      ISP_BypassDevice(PDD *);
UINT32      isp_gidPt(UINT8, UINT32);

static UINT16 isp2400_iocbstatustype0(UINT16 port, ISP2400_STATUS_TYPE_0 * iocb);
UINT32      isp_preferredPortCriteria(TGD * pTgd);
UINT32      isp_alternatePortCriteria(TGD * pTgd);

extern int  L_GetFileSize(const char *);
extern void PM_RelILT2(ILT * pILT);

extern void isp$AbortIocbTask(void);
extern void KernelDispatch(UINT32 returnCode, ILT * pILT, void *pPtr, UINT32 w0);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

static inline UINT32 pdbPidToALPA(UINT32 input)
{
    return (input >> 16) | (input << 16);
}


static UINT64 myhtonll(UINT64 input)
{
    union
    {
        UINT64      output;
        UINT32      words[2];
    } tmp;

    tmp.words[1] = (UINT32)(input & 0x00000000FFFFFFFFll);
    tmp.words[0] = (UINT32)(input >> 32);
    tmp.words[0] = htonl(tmp.words[0]);
    tmp.words[1] = htonl(tmp.words[1]);
    return tmp.output;
}

#ifdef FRONTEND
static UINT64 longlongWordSwap(UINT64 input)
{
    union
    {
        UINT64      output;
        UINT32      words[2];
    } tmp, tmp2;

    tmp2.output = input;
    tmp.words[0] = tmp2.words[1];
    tmp.words[1] = tmp2.words[0];
    return tmp.output;
}
#endif
#ifdef STEVE_DEBUG
void logFailMailbox(QRP * qrp)
{
    UINT32      index = mbxIndx & 0x1F;

    mbxFailed[index][0] = (UINT16)mbxIndx;
    mbxFailed[index][1] = (UINT16)timestamp;
    mbxFailed[index][2] = qrp->imbr[0];
    mbxFailed[index][3] = qrp->imbr[1];
    mbxFailed[index][4] = qrp->imbr[2];
    mbxFailed[index][5] = qrp->imbr[3];
    mbxFailed[index][6] = qrp->imbr[6];
    mbxFailed[index][7] = qrp->imbr[7];
    mbxFailed[index][8] = (UINT16)mbxIndx;
    mbxFailed[index][9] = qrp->iChip;
    mbxFailed[index][10] = qrp->ombr[0];
    mbxFailed[index][11] = qrp->ombr[1];
    mbxFailed[index][12] = qrp->ombr[2];
    mbxFailed[index][13] = qrp->ombr[3];
    mbxFailed[index][14] = qrp->ombr[6];
    mbxFailed[index][15] = qrp->ombr[7];

    ++mbxIndx;
}
#endif /* STEVE_DEBUG */


static void dumpiocb(UINT16 *iocb)
{
    UINT32      i;

    for (i = 0; i < 32; ++i)
    {
        fprintf(stderr, "    iocb word %02d:  %04hX\n", i, iocb[i]);
    }
}


static void isp_reset_nphandleDB(UINT16 port)
{
    UINT32      i;

    for (i = 0; i < HANDLE_DB_SIZE; ++i)
    {
        nphandledbentrycount[port][i] = 0;
    }
}


/**
******************************************************************************
**
**  @brief  Modify QLogic Mailbox Registers based on a bitmap in a QRP.
**
**          The mailbox registers for the indicated QLogic chip instance are
**          updated from the appropriate fields in the supplied QRP. The
**          fields to modify are signaled by set bits in the <qrpiregs>
**          field in the QRP.
**
**          Note: Mailbox Register 0 is always updated.
**
**  @param  qrp - Pointer to a QRP structure
**
**  @return none
**
******************************************************************************
**/
static void isp_mmbox(QRP * qrp)
{
    UINT16      i;
    UINT32      regBitmap;
    volatile UINT16 *mmAddr;
    UINT8       port;

    port = qrp->iChip;              /* Set the chip instance in the QRP. */

    mmAddr = ispstr[port]->mBox;    /* Get PCI address of mailbox registers */

    mmAddr[0] = qrp->imbr[0];       /* Always store mailbox register 0 */

    regBitmap = qrp->iRegs >> 1;    /* Get bitmap and shift for 2nd register */

    /*
     * Copy from the QRP structure to the mailbox registers until
     * the register to copy bitmap is zero.
     */
    for (i = 1; regBitmap != 0; ++i)
    {
        if ((regBitmap & 1) == 1)       /* Is the bit set for this register */
        {
            mmAddr[i] = qrp->imbr[i];   /* Set Mailbox register 0-15 */
        }
        regBitmap >>= 1;        /* Shift bit map for next register. */
    }

#ifdef STEVE_DEBUG
    mailboxIssue[port] = qrp->imbr[0];
#endif /* STEVE_DEBUG */

    /* Set command submission in HCCR. */
    struct ISP_2400 *isp24xx_baseAddr;

    isp24xx_baseAddr = (ISP_2400 *)ispstr[port]->baseAd;
    FORCE_WRITE_BARRIER;
    isp24xx_baseAddr->hccr = ISP2400HCCR_HOST_TO_RISC_INTR;

    /* Set the timeout value. */
    qrp->timeout = timestamp + isp_mb_timeout[qrp->imbr[0]];
}


/**
******************************************************************************
**
**  @brief  isp_process_mbx
**
**  @param  port - QLogic chip instance ordinal (0-3).
**  @param  status - Command complete status (mailbox 0)
**
**  @return none
**
******************************************************************************
**/
void isp_process_mbx(UINT8 port, UINT16 status)
{
    UINT16      i;
    QRP        *head;
    UINT32      regBitmap;
    volatile UINT16 *mmAddr;

    /*
     ** Get PCI address of mailbox registers for this chip.
     */
    mmAddr = ispstr[port]->mBox;

    /*
     ** Get head of deferred queue
     */
    head = ispdefq[port * 2];

    /*
     ** The head pointer should never be NULL.
     */
    if (head == NULL)
    {
        if (!BIT_TEST(isprena, port) && BIT_TEST(resilk, port))
        {
            /* we got an interupt during reset possibly from a command
             * That has been cleared out. This statement is for debug
             */
            fprintf(stderr, "%s: Spurious MailBox interupt during reset on %s port %d status %04X\n",
                    __func__, FEBEMESSAGE, port, status);
            return;
        }
        /*
         ** Set an asynchronous event for the invalid head pointer.
         */
        asyqa[port]->in[0] = ISP_ASAILI;
        asyqa[port]->in[1] = ispstr[port]->baseAd->r2HStat;

        /*
         ** Increment queue pointer.
         */
        asyqa[port]->in += 2;

        /*
         ** Wake-up asynchronous monitor task
         */
        if (rtpcb[(port * 2) + 1] && TaskGetState(rtpcb[(port * 2) + 1]) == PCB_ISP_WAIT)
        {
#ifdef HISTORY_KEEP
CT_history_pcb("isp_process_mbx setting ready pcb", (UINT32)(rtpcb[(port * 2) + 1]));
#endif /* HISTORY_KEEP */
            TaskSetState(rtpcb[(port * 2) + 1], PCB_READY);
        }

        /*
         ** Clear tail pointer
         */
        ispdefq[(port * 2) + 1] = NULL;

        return;
    }

    /*
     ** Get retrieve regs bitmask
     */
    regBitmap = head->oRegs >> 1;

    /*
     ** Always store mailbox register 0
     */
    head->ombr[0] = status;

#ifdef STEVE_DEBUG
    mailboxDone[port] = mailboxIssue[port];
    mailboxIssue[port] = status;
#endif /* STEVE_DEBUG */

#ifndef PERF
    /*
     ** Calculate the execution time.
     */
    i = isp_mb_timeout[head->imbr[0]] - (head->timeout - timestamp);

    /*
     **  Store if largest execution time for this command type.
     */
    if (i > isp_mb_time[head->imbr[0]])
    {
        isp_mb_time[head->imbr[0]] = (UINT8)i;
    }
#endif /* PERF */

    /*
     ** Store the remainder of the mailbox registers
     */
    for (i = 1; regBitmap != 0; ++i)
    {
        /*
         ** Check if the bit is set for this mailbox register
         */
        if ((regBitmap & 1) == 1)
        {
            /*
             ** Retrieve Mailbox register
             */
            head->ombr[i] = mmAddr[i];
        }

        /*
         ** Shift bit map for next register.
         */
        regBitmap >>= 1;
    }

    if (status != ISP_CMDC)
    {
        /* don't print out handle already used errors from FLOGI command */
        /* don't print out failed get port dB for not logged in */
        if ( ((head->imbr[0] == ISP_GLFP) && (head->ombr[0] == ISP_PIU || head->ombr[0] == ISP_LIU)) ||
             (head->imbr[0] == ISP_GTPD && head->ombr[0] == ISP_CPE)
           )
        {
            /* I set this up like this instead of simple "if" so other errors could be filtered out
             * if necessary */
            /*DumpMboxError = 0; */
        }
        else
        {
            fprintf(stderr, "%s:  FAILED mailbox command %s port=%d, cmd=%04x, status=%04x\n    ",
                    __func__,FEBEMESSAGE, port, head->imbr[0], status);
            for (i = 0; i < 32; i++)
            {
                if (BIT_TEST(head->iRegs, i))
                {
                    fprintf(stderr, "imbr[%d]=%04x ", i, head->imbr[i]);
                }
            }
            fprintf(stderr, "\n    ");
            for (i = 0; i < 32; i++)
            {
                if (BIT_TEST(head->oRegs, i))
                {
                    fprintf(stderr, "ombr[%d]=%04x ", i, head->ombr[i]);
                }
            }
            fprintf(stderr, "\n");
        }
    }

#if 0
    /*
     ** Log the failed mailbox command
     */
    if (head->ombr[0] != ISP_CMDC)
    {
        logFailMailbox(head);
    }
#endif /* STEVE_DEBUG */

    /*
     ** Check for a process associated with this QRP
     */
    if (head->pPCB != 0 && TaskGetState(head->pPCB) == PCB_QLMBX_RSP_WAIT + port)
    {
        /*
         ** Set task status to ready
         */
#ifdef HISTORY_KEEP
        CT_history_pcb("isp_process_mbx #2 setting ready pcb", (UINT32)(head->pPCB));
#endif /* HISTORY_KEEP */
        TaskSetState(head->pPCB, PCB_READY);
    }

    /*
     ** Set command completed flag in QRP to TRUE
     */
    head->stFlag = TRUE;

    /*
     ** Remove QRP from thread
     */
    ispdefq[port * 2] = head = head->pFThd;

    /*
     ** Check if request is at end of the list.
     */
    if (head == NULL)
    {
        /*
         ** Request was at end of list; clear tail pointer
         */
        ispdefq[(port * 2) + 1] = NULL;
    }
    else
    {
#ifdef STEVE_DEBUG
        ispdebug[0] = timestamp;
        ispdebug[1] = resilk;
        ispdebug[2] = ispOnline;
        ispdebug[4] = ispstr[port]->baseAd->cntl;
        ispdebug[5] = ispstr[port]->baseAd->intc;
        ispdebug[6] = ispstr[port]->baseAd->ints;
        ispdebug[7] = ispstr[port]->baseAd->sema;
        ispdebug[8] = ispstr[port]->baseAd->hccr;
#endif /* STEVE_DEBUG */
        /*
         ** Deferred QRP exists; submit it.
         */
        isp_mmbox(head);
    }
}


/**
******************************************************************************
**
**  @brief      Send PCI config log message to CCB
**
**              Formats the message for the PCI Config error and
**              sends the message to the CCB.
**
**  @param      a       - Not used
**  @param      b       - Not used
**  @param      port    - QLogic chip instance (0-3)
**  @param      offset  - PCI offset
**  @param      count   - Number bytes being written
**
**  @return     none
**
******************************************************************************
**/
void isp_pciConfigError(UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port, UINT32 offset, UINT32 count)
{
    LOG_PORT_EVENT_PKT eldn;

    /*
     ** Log the PCI write error
     */
    eldn.header.event = LOG_PCI_CFG_ERR;

    eldn.data.port = port;
#ifdef FRONTEND
    eldn.data.proc = 0;
#else  /* FRONTEND */
    eldn.data.proc = 1;
#endif /* FRONTEND */
    eldn.data.reason = offset;
    eldn.data.count = count;
    /*
     * Note: message is short, and L$send_packet copies into the MRP.
     */
    MSC_LogMessageStack(&eldn, sizeof(LOG_PORT_EVENT_PKT));
}

#ifdef FABRIC
/**
******************************************************************************
**
**  @brief      Clear FC-4 Types Registration (to force a new registration)
**
**      This routine clears the flags that indicate an FC-4 Types registration
**      has been completed with the switch, thus forcing a new registration.
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     none
**
******************************************************************************
**/
void isp_ClearFC4TypesRegistration(UINT32 port)
{
#ifdef FRONTEND
    TAR        *pTar;

    /* Traverse the target list for this port and clear the flags. */
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        pTar->flags = FALSE;
    }
#endif /* FRONTEND */
    /* Force FC-4 Types (re-)registration of this port with the switch. */
    fc4flgs &= ~(1 << port);
}
#endif /* FABRIC */

/**
******************************************************************************
**
**  @brief      Generate an offline event for the Translation Layer.
**
**              This routine generates an ILT that is passed to the Translation
**              Layer. Parameters in this ILT tell the Translation Layer that
**              an offline event has occurred for this chip instance.
**              The host counter is cleared for the indicated QLogic instance.
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     none
**
******************************************************************************
**/
void ISP_GenOffline(UINT32 port)
{
#ifdef FRONTEND
    ILT        *ilt;
#endif /* FRONTEND */

#ifdef FABRIC
    BIT_CLEAR(ispfflags, port); /* Clear Fabric available bit */
#endif /* FABRIC */

    BIT_CLEAR(ispOnline, port); /* Clear online bit */

#ifdef FRONTEND
    BIT_CLEAR(id_acquired, port);       /* Clear Virtual Port IDs acquired */

    /*
     ** Offline; generate ILT for passage to Translation Layer
     */
    ilt = get_ilt();            /* get an ILT w/wait            */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    ilt->ilt_normal.w0 = ISP_OFF_LINE_CMD;      /* store cmd byte               */
    ilt->ilt_normal.w0 |= port << 8;    /* store Chip ID                */
    ilt->ilt_normal.w2 = 0;     /* Clear request completed flag */
    ilt->cr = NULL;             /* No completion handler        */
    ilt[1].misc = (UINT32)ilt;  /* store pointer to parm struct */
    ++ilt;                      /* Get next level of ILT        */

    /*
     ** No completion routine, cdriver will release ilt
     */
    ilt->cr = NULL;             /* No completion handler    */

    C_recv_scsi_io(ilt);        /* Invoke Translation Layer. */

#if INITIATOR
    I_recv_offline(port);       /* Invoke Initiator level */
#endif /* INITIATOR */
#endif /* FRONTEND */
}                               /* ISP_GenOffline */


/**
******************************************************************************
**
**  @brief      Loop Offline Handler.
**
**              Processes & responds to ISP Loop Down events 8012(h).
**              Note that this is also artificially invoked during
**              Chip Reset processing.
**
**              Loop UP LED cleared
**              ISP_GenOffline called.
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     none
**
******************************************************************************
**/
void ISP_LoopDown(UINT32 port)
{
#ifdef FABRIC
    /*
     ** Force FC-4 Types (re-)registration with the switch.
     */
#if FE_ISCSI_CODE
    if (!(BIT_TEST(iscsimap, port)))
#endif /* FE_ISCSI_CODE */
    {
        isp_ClearFC4TypesRegistration(port);
    }
#endif /* FABRIC */

#ifdef BACKEND
    /*
     ** Check if initial DEVice discovery is done
     */
    if (BIT_TEST(K_ii.status, II_PHY))
#endif /* BACKEND */
        isp_reset_nphandleDB(port);

    /*
     ** Ensure the chip isn't being reset (which probably generated this event)
     */
    if (BIT_TEST(resilk, port) == 0)
    {
        LOG_PORT_EVENT_PKT G_eldn;

        /*
         ** Send debug log entry to CCB.
         */
        G_eldn.header.event = LOG_LOOP_DOWN;
        G_eldn.data.port = port;
        G_eldn.data.reason = 0;
#ifdef FRONTEND
        G_eldn.data.proc = 0;
#else  /* FRONTEND */
        G_eldn.data.proc = 1;
#endif /* FRONTEND */
        G_eldn.data.count = 0;
        /*
         * Note: message is short, and L$send_packet copies into the MRP.
         */
        MSC_LogMessageStack(&G_eldn, sizeof(LOG_PORT_EVENT_PKT));
    }

#ifdef BACKEND
    /*
     ** Set loop down request for this port
     */
    P_chn_ind[port]->state = (1 << CH_LOOP_DN_REQ);

    /*
     ** Clear notify request for this port
     */
    BIT_CLEAR(F_notifyreq, port);

    /*
     ** Start the Port Monitor process
     */
    F_startPortMonitor(port);
#endif /* BACKEND */

    /*
     ** Check for non-zero port notification settings
     */
    if (mpn.loopDownToNotify != 0)
    {
        /*
         ** If the port is not marked as failed (i.e. do not initialize),
         ** then fork a process that waits for the loop to come online
         ** following initialization.
         */

        /*
         * Wait if isp_loopEventHandler process is being created.
         */
        while (isplepcb[port] == (PCB *)- 1)
        {
            TaskSleepMS(50);
        }

        if ((ispFailedPort[port] == FALSE) && (isplepcb[port] == NULL))
        {
            CT_fork_tmp = (unsigned long)"isp_loopEventHandler";
            isplepcb[port] = (PCB *)- 1;        // Flag task being created.
            isplepcb[port] = TaskCreate3(C_label_referenced_in_i960asm(isp_loopEventHandler),
                                         K_xpcb->pc_pri - 1, port);
        }
    }

    ISP_GenOffline(port);       /* Generate offline event */
}                               /* ISP_LoopDown */


/**
******************************************************************************
**
**  @brief      Loop Online Handler.
**
**              Processes & responds to ISP Loop UP events (8011h).
**
**              Loop UP LED lit
**              Port database flags cleared.
**              <isp$process_online> called.
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      entry   - asynchronous event
**
**  @return     none
**
******************************************************************************
**/
void ISP_ProcessOnline(UINT32 port, struct ev_t *entry)
{
    UINT32      i;
    UINT32      entryEvent;
    UINT32      entryData;
    UINT32     *loopMap;

    entryEvent = entry->event;
    entryData = entry->data;

#ifdef BACKEND
    fprintf(stderr, "%s %s P: %d E:%08X  D:%08X\n", __func__,FEBEMESSAGE, port, entryEvent, entryData);
    /*
     ** Is this a Loop Up or Fabric Change Notification Event?
     */
    if (entryEvent == ISP_ASPCNE)
    {
        /*
         ** Check if connected to switch (fabric).
         */
        if (ISP_IsFabricMode(port) != FALSE)
        {
            /*
             ** Set RSCN request for this port
             */
            FAB_InsertRSCNEvent(port, entryData);
            BIT_SET(P_chn_ind[port]->state, CH_RSCN_REQ);
        }
    }

#if defined(MODEL_7000) || defined(MODEL_4700)
     if (entryEvent == ISP_ASPFCU || entryEvent == ISP_ASPDBE)
#endif /* MODEL_7000 || MODEL_4700 */
    {
        /*
        ** Set loop up request for this port
        */
        BIT_SET(P_chn_ind[port]->state, CH_LOOP_UP_REQ);
    }

    /*
     ** Check for any devices connected to this loop
     */
    if (P_chn_ind[port]->devCnt == 0)
    {
        P_chn_ind[port]->wait = ONLINE_DELAY;   /* Set loop up settle time */
    }

    F_startPortMonitor(port);   /* Start the Port Monitor process */
#endif /* BACKEND */

    if (entryEvent == ISP_ASPCNE)
    {
        LOG_RSCN_PKT elrscn;
        LOG_RSCN_DAT *elrscnData;

        elrscnData = &elrscn.data;
        elrscn.header.event = LOG_RSCN; /* RSCN notification (8015). */

        elrscnData->port = port;
#ifdef FRONTEND
        elrscnData->proc = 0;
#else  /* FRONTEND */
        elrscnData->proc = 1;
#endif /* FRONTEND */
        elrscnData->addressFormat = (UINT8)(entryData >> 8);
        elrscnData->portId = ((entryData << 16) | (entryData >> 16)) & 0xFFFFFF;

        /*
         * Note: message is short, and L$send_packet copies into the MRP.
         */
        MSC_LogMessageStack(&elrscn, sizeof(LOG_RSCN_PKT));
    }
    else
    {
        /* Log the Online event */

        LOG_LOOPUP_PKT elup;
        LOG_LOOPUP_DAT *elupData;
        UINT32      size;

        elupData = &elup.data;
        memset(&elup, 0, sizeof(elup));
        elupData->port = port;
#ifdef BACKEND
        elupData->proc = 1;
        if (ISP_IsFabricMode(port))
        {
            BIT_SET(elupData->flags, PORT_FABRIC_MODE_BIT);
        }
        else
        {
            BIT_CLEAR(elupData->flags, PORT_FABRIC_MODE_BIT);
        }
#endif /* BACKEND */
        if (entryEvent == ISP_ASPDBE)
        {
            /*
             ** Port Database changed (8014).
             ** LOG_PORTDBCHANGED_DAT is a #define to LOG_LOOPUP_DAT.
             */
            elup.header.event = LOG_PORTDBCHANGED;
        }
        else
        {
            /*
             ** Fibre Channel Loop Up (8011)
             */
            /*
             ** Send log entry to CCB.
             ** This is a struct LOG_LOOPUP_PKT/LOG_LOOPUP_DAT.
             */
            elup.header.event = LOG_LOOPUP;
#ifdef BACKEND
            /*
             ** Check if initial drive startup hI_recv_onlineas been done and
             ** SES_BackgroundProcess has been allowed to run.
             */
            if (BIT_TEST(K_ii.status, II_SESBKRUN))
            {
                /* Start up backgroup SES process. */
                SES_StartBGProcess_c();
            }
#endif /* BACKEND */
        }

        elupData->failed = ispFailedPort[port];
        elupData->lid = (UINT16)entryData;
        elupData->state = (UINT16)(entryData >> 16);

        /*
         ** Copy the first 56 bytes (14 words) of the loop map to the log packet
         ** (one word at a time).
         */
        loopMap = (UINT32 *)lpmap[port];
        for (i = 0; i < sizeof(elupData->lpmap) / 4; ++i)
        {
            ((UINT32 *)(elupData->lpmap))[i] = loopMap[i];
        }

        /*
         ** Size of packet depends on size of loop map. Round up to a word.
         ** (CCB will round it up otherwise, padding with zeroes, whereas our
         ** AL_PA map is normally padded with 0xFF.)
         */
        size = sizeof(elup) - sizeof(elupData->lpmap) + 1 + elupData->lpmap[0];
        size = ((size + 3) / 4) * 4;

        if (size > sizeof(elup))
        {
            size = sizeof(elup);
        }

        /*
         * Note: message is short, and L$send_packet copies into the MRP.
         */
        MSC_LogMessageStack(&elup, size);
    }

#if defined(FRONTEND) && defined(INITIATOR)
    /* Perform XL online processing */

    if ((entryEvent == ISP_ASPCNE) && (I_CIMT_dir[port]->state == Cs_online))
    {
        I_ChangeNotification(port, bswap_32((entryData << 16) | (entryData >> 16)));
    }
    else if ((entryEvent == ISP_ASPDBE) && (I_CIMT_dir[port]->state == Cs_online))
    {
        I_ChangeDB(port, (UINT16)entryData, (UINT16)(entryData >> 16));
    }
    else if(!(BIT_TEST(iscsimap, port)) || (ispmap == iscsimap))
    {
        /*
        ** Patch ("if" condition) added to avoid invoking the initiator
        ** path for iSCSI in mixed mode. (ATS-158)
        */
        I_recv_online(port, ispLid[port], (UINT8)entryEvent);
    }
#endif /* defined(FRONTEND) && defined(INITIATOR) */
}


/**
******************************************************************************
**
**  @brief  Loop Online Handler.
**
**          Processes ISP Loop Up events (AENs 8011h, 8014h, or 8015h).
**
**          Loop Up LED lit.
**          Port database flags cleared (FE).
**          <ISP_ProcessOnline> called, to continue online processing.
**
**  @param  port - QLogic chip instance (0-3)
**  @param  entry - asynchronous event
**
**  @return none
**
******************************************************************************
**/
void ISP_LoopUp(UINT8 port, struct ev_t *entry)
{
    UINT32      bitPort = 1 << port;

#ifdef FRONTEND
    UINT32      i;
    UINT32     *pFlags;
    TAR        *pTar;
#endif /* FRONTEND */
    /* port is being reset so return
     *  there will be another loop up after reset completes.
     */
    if (!BIT_TEST(isprena, port))
    {
        return;
    }
    /*
     ** Get loop ID and port ID.
     ** If none assigned yet (0x4005 error) the port isn't truly up yet,
     ** or if the command fails due to a timeout or reset in progress (0x4096,
     ** 0x4097, 0x4099 AENs), don't continue with online processing until a
     ** future online event occurs, and we have been assigned a LID.
     */
    /*  Is this a Loop Up Event? */
    if (entry->event == ISP_ASPFCU)
    {
        //let the fabric settle a bit before we use it
        TaskSleepMS(125);
    }
    if (ISP_GetLoopId(port,0) != ISP_CMDC)
    {
     //   ISP_LoopDown(port);
        return;
    }

#ifdef FRONTEND
    /* ISP_GetLoopId may task switch, and invalidate tar[] list. */
  restart:
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        if (pTar->vpID != 0)
        {
            /* NOTE: ISP_GetLoopId may task switch. */
            UINT32 save_tar_link_abort = tar_link_abort[port];

            ISP_GetLoopId(port,pTar->vpID);
            /* If tar list got smaller, then the linked list may not be valid, restart. */
            if (save_tar_link_abort != tar_link_abort[port])
            {
                goto restart;
            }
        }
    }

    pFlags = (UINT32 *)dbflags[port];
    for (i = 0; i < MAX_DEV / 4; ++i)
    {
        *pFlags = FALSE;        /* Clear byte indicating database entry is empty */
        ++pFlags;
    }

#endif /* FRONTEND */
    isp_reset_nphandleDB(port);

    /*
      * If this is a transition from offline => online,
      * then set the online bit and start the 10 second timer for this port
      */
    if ((ispOnline & bitPort) == 0)
    {
        ispOnline |= bitPort;
        onlineTimer[port] = 10;

        /* Wait if isp_portOnlineHandler process is being created. */
        while (isponpcb == (PCB *)- 1)
        {
            TaskSleepMS(50);
        }

        /* Fork online monitor process, if necessary. */
        if (isponpcb == NULL)
        {
            CT_fork_tmp = (unsigned long)"isp_portOnlineHandler";
            isponpcb = (PCB *)- 1;      // Flag that task is being created.
            isponpcb = TaskCreate2(C_label_referenced_in_i960asm(isp_portOnlineHandler), ISPPONPRI_PRIORITY);
        }
    }

    /*  Is this a Loop Up Event? */
    if (entry->event == ISP_ASPFCU)
    {
        /*
         * Store the data rate
         * 0 = 1 GB, 1 = 2 GB
         */
        isprev[port]->dataRate = (UINT8)entry->data + 1;
        fprintf(stderr, "DATA RATE %s Port %d speed bit %d\n", FEBEMESSAGE, port, isprev[port]->dataRate);

        /* Wait if isp_loopEventHandler process is being created. */
        while (isplepcb[port] == (PCB *)- 1)
        {
            TaskSleepMS(50);
        }

        /* Ready the loop event process since the loop is up. */
        if (isplepcb[port] != NULL)
        {
            if (TaskGetState(isplepcb[port]) == PCB_NOT_READY)
            {
#ifdef HISTORY_KEEP
CT_history_pcb("ISP_LoopUp setting ready pcb", (UINT32)isplepcb[port]);
#endif /* HISTORY_KEEP */
                TaskSetState(isplepcb[port], PCB_READY);
            }
        }
    }

    ISP_GetPositionMap(port);   /* Request FC-AL position map from QLogic */

#ifdef FABRIC
    /* Check if connected FL_port or F_port. */
    if (ispConnectionType[port] == FL_PORT || ispConnectionType[port] == F_PORT)
    {
        ispfflags |= bitPort;   /* Set fabric found flag */

        if ((fc4flgs & bitPort) == 0)
        {
            /*
             * Setup FC-4 parameters to allow SCSI target operation. Only do
             * this if online event occurred. This is done to prevent
             * multiple FC-4 registrations.
             *
             * For MULTI_ID mode (multi-id) pass through all VPs defined and
             * register them as SCSI targets. Do not do this for the
             * primary port.
             *
             * In MULTI_ID mode this is done with a separate task spawned to
             * process the registration. This is because the VPs cannot be
             * processed until a Report ID Acquisition IOCB is received.
             */
#ifdef BACKEND
            if (isp_registerFc4(port, 0x1100, NULL) == GOOD)
            {
                fc4flgs |= bitPort;     /* Set as FC-4 registered. */
            }
#else  /* BACKEND */
            /* Wait if isp_registerVports process is being created. */
            while (fc4pcb[port] == (PCB *)- 1)
            {
                TaskSleepMS(50);
            }

            if (fc4pcb[port] == NULL)
            {
                CT_fork_tmp = (unsigned long)"isp_registerVports";
                fc4pcb[port] = (PCB *)- 1;      // Flag task being created.
                fc4pcb[port] = TaskCreate3(C_label_referenced_in_i960asm(isp_registerVports),
                                           ISPREGVP_PRIORITY, port);
            }
#endif /* BACKEND */
        }
    }
    else
    {
        ispfflags &= ~bitPort;  /* Clear fabric found flag */
    }
#endif /* FABRIC */

    /* Continue processing of Online event. */
    ISP_ProcessOnline(port, entry);
}                               /* ISP_LoopUp */


/**
******************************************************************************
**
**  @brief  QLogic ISP2400 ATIO queue processing code.
**
**          This function is called with the ordinal of a QLogic ISP 2400 chip
**          and the "in" value to process up to.
**
**  @param  port - QLogic chip instance (0-3)
**  @param  in - new in value for queue processing
**  @param  session - Current session to detect QLogic resets
**
**  @return Count of entries processed, -1 if reset detected
**
******************************************************************************
**/
static int isp2400_process_atio_que(UINT8 port, UINT32 in, UINT32 session)
{
    QCB        *atioQ = (QCB *)ispstr[port]->atioQue;
    UINT32     *qin;
    int         touchedatioq = 0;

    qin = atioQ->begin + in;

    while (qin != atioQ->out)
    {
        UINT16      i;
        IOCB_HDR   *hdr;

        ++touchedatioq;
        hdr = (IOCB_HDR *)atioQ->out;
        switch (hdr->iocb_type)
        {
#ifdef FRONTEND
            case ATIO7:
                isp2400_process_atio7(port, (ISP2400_ATIO7 *)hdr);
                break;

            case INOTIFY:
                isp2400_process_inotify(port, (ISP2400_INOTIFY *)hdr);
                break;
#endif /* FRONTEND */

            default:
                fprintf(stderr, "Unknown iocb in ATIO queue\n");
                dumpiocb((UINT16 *)hdr);

                /*
                 ** Dump qlogic registers and reset chip. This includes
                 ** asynchronous events System error, Request transfer error,
                 ** Response transfer error, and Firmware detected error.
                 */
                ISP_DumpQL(port, ISP_BAD_IOCB_TYPE);
                break;
        }

        /* The following is necessary to handle QLogic reset conditions */

        if (session != sessids[port])
        {
            return -1;
        }

        for (i = 0; i < hdr->iocb_count; i++)
        {
            /* Update pointers */
            atioQ->out += 16;
            /* Check for wrap condition */
            if (atioQ->out >= atioQ->end)
            {
                atioQ->out = atioQ->begin;
            }
        }
    }

    return touchedatioq;
}

static void isp_log_iocb(UINT8 port, UINT32 reason, void *iocb)
{
    LOG_IOCB_PKT TmpStackMessage;

    TmpStackMessage.data.port = port;
#ifdef FRONTEND
    TmpStackMessage.data.feorbe = 0;
#else
    TmpStackMessage.data.feorbe = 1;
#endif
    TmpStackMessage.data.reason = reason;

    TmpStackMessage.header.event = LOG_IOCB;
    memcpy(&TmpStackMessage.data, iocb, 64);
    MSC_LogMessageStack(&TmpStackMessage, sizeof(LOG_IOCB_PKT));
}

/**
****************************************************************************
**
**  @brief      Verify iocbhandle is a valid ILT.
**
**  @param      ilt - Pointer to ILT.
**
**  @return     TRUE/FALSE
**
******************************************************************************
**/
static UINT32 verify_iocbhandle(ILT* ilt)
{
    if (!ilt)               /* Zero is valid. */
    {
        return(TRUE);
    }

#if defined(BACKEND)
#define MEM_start   (ILT*)(BACK_END_PCI_START)
#define MEM_end     (ILT*)(NVRAM_BASE)
#endif  /* BACKEND */
#if defined(FRONTEND)
#define MEM_start   (ILT*)(FRONT_END_PCI_START)
#define MEM_end     (ILT*)(CCB_PCI_START)
#endif  /* FRONTEND */
    if (ilt < MEM_start || ilt >= MEM_end)  /* Must be in shared memory. */
    {
        return(FALSE);                      /* Allow byteswap check next. */
    }

    /* Now check if really an ILT. */
    /* Un-nest the many levels, checking for allocation on 64 byte boundary, */
    /* ILTNEST = array size of ILTs that make up an ILT. */
    int cnt = 0;
    /* New memory pool with pre-post patterns of 32 bytes, knowledge that */
    /* an ILT was on a 64 byte aligned memory location is counted on. */
    while ((((UINT32)ilt & 63) != 0) && (cnt < ILTNEST))
    {
        ilt = ilt - 1;
        cnt = cnt + 1;
        if (cnt == ILTNEST)
        {
#ifndef PERF
        abort();
#else   /* PERF */
        return(FALSE);
#endif  /* PERF */
        }
    }

    /* Then check that the pre header is right -- for an ILT. */
    struct before_after *b = (struct before_after *)((UINT32)ilt) - 1;
    if (b->pre1 != 0xdddddddd ||
        *((UINT32*)(b->str)) != 0x00544c49 ||        /* "ILT" */
        *(((UINT32*)(b->str)) + 1) != 0x00000000 ||
        b->used_or_free != 1 ||                     /* used */
        b->length != ILT_SIZE ||                    /* FE = 11*52, BE = 7*52 */
        b->pre2 != 0xdddddddd)
    {
#ifndef PERF
        abort();
#else   /* PERF */
        return(FALSE);
#endif  /* PERF */
    }
#ifndef PERF
    /* Then check that the post header is right -- for an ILT. */
    b = (struct before_after *)(((UINT32)ilt + (ILT_SIZE + 63)) & ~0x3f);
    if (b->pre1 != 0xdcdcdcdc ||
        b->used_or_free != 1 ||                     /* used */
        b->length != ILT_SIZE ||                    /* FE = 11*52, BE = 7*52 */
        b->pre2 != 0xdcdcdcdc)
    {
//        return(FALSE);
        abort();
    }
#endif  /* PERF */
    return(TRUE);
}   /* End of verify_iocbhandle */

/**
 ******************************************************************************
 **
 **  @brief calls apropriate routines to handle new IOCBs from the qlogic
 **
 **  @param port - QLogic chip instance (0-3)
 **  @param iocbhdr - un processed iocb
 **
 ******************************************************************************
 */
#define PROC_IOCB_FLAG_DOCOMPLETION 0x01
#define PROC_IOCB_FLAG_FREE_ILT     0x02
static void isp_process_iocb(UINT8 port, IOCB_HDR * iocbhdr)
{
    ILT        *ilt;
    UINT32      retval = EC_OK;
    UINT32      flags;

    flags = 0;
    switch (iocbhdr->iocb_type)
    {
#ifdef FRONTEND
        case CTIO7:
            /* target IO conitnuation */
            retval = isp2400_process_ctio7(port, (ISP2400_CTIO7_STATUS *)iocbhdr);
            flags = PROC_IOCB_FLAG_DOCOMPLETION;
            break;

        case ABTSRCV:
            /* ABTS received */
            isp2400_process_abts(port, (ISP2400_ABTS *)iocbhdr);
            return;

        case ABTSACK:
            flags = PROC_IOCB_FLAG_FREE_ILT;
            break;

        case NTACK:
            /* NT ACk is special since it doesn't observe proper levels */
            flags = PROC_IOCB_FLAG_DOCOMPLETION;
            break;

        case IMRIA:
            isp_processIdAcquisition(port, (VPRID_IOCB *)iocbhdr);
            return;

        case VPCTRL:
            isp2400_processVpControl(port, (ISP2400_VPCONTROL *)iocbhdr);
            flags = PROC_IOCB_FLAG_DOCOMPLETION;
            break;
#endif  /* FRONTEND */

        case STATUS0_CONT:
        case MARKER_IOCB:
            /* this is a NO OP */
            return;

        case STATUS0:
            retval = isp2400_iocbstatustype0(port, (ISP2400_STATUS_TYPE_0 *)iocbhdr);
            flags = PROC_IOCB_FLAG_DOCOMPLETION;
            break;

        case MBIOCB:
            isp_processMailboxIOCB(port, (ISP2400_MBIOCB *)iocbhdr);
            flags = PROC_IOCB_FLAG_DOCOMPLETION;
            break;

        case ELS_PASS_IOCB:
            ISP2400_HandleElsIocb(port, iocbhdr);
            return;

        case PLOGXIOCB:
        case MSIOCB:
        case ABORTIOCB:
            ilt = (ILT *)iocbhdr->iocbhandle;
            if (!verify_iocbhandle(ilt))
            {
                ILT *tmpILT = (ILT*)bswap_32((UINT32)ilt);
                if (!verify_iocbhandle(tmpILT))
                {
                    fprintf(stderr, "%s%s:%u %s nphandle is not an ILT (%p), ignoring\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
                    break;
                }
                else
                {
                    fprintf(stderr, "%s%s:%u %s nphandle was byteswapped (%p), correcting...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
                    ilt = tmpILT;
                }
            }
            ilt--;
            /* see exec_iocb_wait for why this is done */
            memcpy((void *)ilt->ilt_normal.w2, iocbhdr, 64);
            flags = PROC_IOCB_FLAG_DOCOMPLETION;
            break;

        default:
            goto proc_iocb_dump;
    }

    /*
     * This is the common completion code we can do both options
     * call a completion routine and/or free the ilt
     */
    ilt = (ILT *)iocbhdr->iocbhandle;
    if (!verify_iocbhandle(ilt))
    {
        ILT *tmpILT = (ILT*)bswap_32((UINT32)ilt);
        if (!verify_iocbhandle(tmpILT))
        {
            fprintf(stderr, "%s%s:%u %s nphandle is not an ILT (%p), processing as zero...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            ilt = 0;
        }
        else
        {
            fprintf(stderr, "%s%s:%u %s nphandle was byteswapped (%p), correcting...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            ilt = tmpILT;
        }
    }

    if (ilt == NULL)
    {
#ifdef DEBUG
        abort();
#endif
        return;
    }
    if (isp_unthread_ilt(port, ilt) == NULL)
    {
        goto proc_iocb_dump;
    }
    ilt--;
    if (flags & PROC_IOCB_FLAG_DOCOMPLETION)
    {
        if (ilt->cr == NULL)
        {
            retval = ISP_NO_ILT_CMPL;
            goto proc_iocb_dump;
        }
        // giant hack so FE completion routines work they expect the iocb
        // to be in g11.
        g11 = (UINT32)iocbhdr;
        KernelDispatch(retval, ilt, iocbhdr, 0);
    }
    if (flags & PROC_IOCB_FLAG_FREE_ILT)
    {
        put_ilt(ilt);
    }
    return;

proc_iocb_dump:
    isp_log_iocb(port, retval, iocbhdr);
    ISP2400_DumpQL(port, retval);
}

/**
******************************************************************************
**
**  @brief  QLogic ISP2400 response queue processing code.
**
**          This function is called with the ordinal of a QLogic ISP 2400 chip
**          and the "in" value to process up to.
**
**  @param  port - QLogic chip instance (0-3)
**  @param  in - new in value for queue processing
**  @param  session - Current session to detect QLogic resets
**
**  @return Count of entries processed, -1 if reset detected
**
******************************************************************************
**/
static int isp2400_process_response_que(UINT8 port, UINT32 in, UINT32 session)
{
    QCB        *resQ = (QCB *)ispstr[port]->resQue;
    UINT32     *qin;
    int         touchedresq = 0;

    qin = resQ->begin + in;

    /* Response que */
    while (qin != resQ->out)
    {
        UINT16      i;
        IOCB_HDR   *hdr;

        ++touchedresq;
        hdr = (IOCB_HDR *)resQ->out;
        if (hdr->iocb_type == CTIO7)
        {
            ISP2400_CTIO7 *iocb = (ISP2400_CTIO7 *)hdr;

            if ((iocb->nphandle != 1 && iocb->nphandle != 0x02 && iocb->nphandle != 0x08) ||
                iocb->entryStatus != 0)
            {
                fprintf(stderr, "%s: ERROR %04X entry stat %02X exchange %08X "
                        "vpid %d resid length %08X iocb 0x%08X ilt 0x%08X "
                        "in 0x%08X\n",
                        __func__, iocb->nphandle, iocb->entryStatus,
                        iocb->exchangeaddr, iocb->vpid, iocb->residXferLen,
                        (UINT32)iocb, iocb->iocbhandle, in);
            }
        }
        isp_process_iocb(port, hdr);

        /* The following is necessary to handle QLogic reset conditions */

        if (session != sessids[port])
        {
            return -1;
        }

        for (i = 0; i < hdr->iocb_count; ++i)
        {
            /* Update pointers */
            resQ->out += 16;
            /* Check for wrap condition */
            if (resQ->out >= resQ->end)
            {
                resQ->out = resQ->begin;
            }
        }
    }

    return touchedresq;
}


/**
******************************************************************************
**
**  @brief  QLogic ISP2400 ATIO queue monitor code.
**
**          This task is started with the ordinal of a QLogic ISP 2400 chip.
**          This task then handles all IOCBs posted to the ATIO queue
**          by the firmware;
**
**  @param  a - Not used
**  @param  b - Not used
**  @param  port - QLogic chip instance (0-3)
**
**  @return none
**
******************************************************************************
**/

NORETURN void ISP_monitor_atio(UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port)
{
    struct ISP_2400 *pISP24 = (ISP_2400 *)ispstr[port]->baseAd;
    UINT32      qindex;
    UINT32      currentSession;
    UINT8       touchedatioq;
    UINT8       touchedresq;
    UINT32      fairnesscount;
    char        reset_wait_msg = 0;

    TaskSetMyState(PCB_ISP_WAIT);

    while (TRUE)
    {
        TaskSwitch();
        if (BIT_TEST(resilk, port))
        {
            TaskSleepMS(25);
            if (!reset_wait_msg)
            {
                fprintf(stderr, "%s: Reset set on port %d\n", __func__, port);
                reset_wait_msg = 1;
            }
            continue;
        }
        reset_wait_msg = 0;

        /* If in failed state go back to sleep */
        /* If empty go to sleep */
        if (isp2400_queue[port].in == isp2400_queue[port].out ||
            BIT_TEST(ispfail, port) || !BIT_TEST(isprena, port))
        {
            TaskSetMyState(PCB_ISP_WAIT);
            continue;
        }
        TaskSetMyState(PCB_READY);
#ifdef HISTORY_KEEP
CT_history_pcb("ISP_monitor_atio setting ready pcb", (UINT32)(K_xpcb));
#endif /* HISTORY_KEEP */

        currentSession = sessids[port]; /* Get current Session ID */
        touchedatioq = 0;
        touchedresq = 0;
        fairnesscount = 0;

        while (fairnesscount < 64 && isp2400_queue[port].in != isp2400_queue[port].out)
        {
            UINT16      tmpout;
            UINT32      work;
            int         rc;

            tmpout = isp2400_queue[port].out;
            work = isp2400_queue[port].work[tmpout];
            ++tmpout;
            if (tmpout >= MAX_2400_WORK)
            {
                tmpout = 0;
            }
            isp2400_queue[port].out = tmpout;

            switch (work & 0xFF)
            {
                case ISP2400RHS_STATUS_RESP:
                case ISP2400RHS_STATUS_ATIO_RESP:
                    rc = isp2400_process_response_que(port, (work & 0xFFFF0000) >> 12, currentSession);
                    if (rc > 0)
                    {
                        touchedresq = 1;
                    }
                    break;

                case ISP2400RHS_STATUS_ATIO:
                    rc = isp2400_process_atio_que(port, (work & 0xFFFF0000) >> 12, currentSession);
                    if (rc > 0)
                    {
                        touchedatioq = 1;
                    }
                    break;

                default:
                    if (work)
                    {
                        fprintf(stderr, "%s: corrupted work queue, value=%08x\n", __func__, work);
                    }
                    rc = 0;
                    break;
            }

            /* If reset was detected during processing, break out of loop. */

            if (rc < 0)
            {
                break;
            }

            fairnesscount += rc;
        }

        /* If resetting go to sleep */
        if (currentSession != sessids[port])
        {
            TaskSetMyState(PCB_ISP_WAIT);
            continue;
        }

        /* Update q out index pointer in the HW for real */

        if (touchedatioq)
        {
            QCB        *atioQ = (QCB *)ispstr[port]->atioQue;

            qindex = atioQ->out - atioQ->begin; /* Get offset in units of UINT32 */
            qindex = qindex >> 4;       /* Div by 16 because iocb size is 64 */
            FORCE_WRITE_BARRIER;
            pISP24->atioQOP = qindex;   /* Write register */
        }
        if (touchedresq)
        {
            QCB        *resQ = (QCB *)ispstr[port]->resQue;

            qindex = resQ->out - resQ->begin;   /* Get offset in units of UINT32 */
            qindex = qindex >> 4;       /* Div by 16 because iocb size is 64 */
            FORCE_WRITE_BARRIER;
            pISP24->rspQOP = qindex;    /* Write register */
        }
    }
}


/**
******************************************************************************
**
**  @brief  QLogic ISP2x00 asynchronous event monitor code.
**
**          This task is started with the ordinal of a QLogic ISP 2x00 chip.
**          This task then handles all asynchronous mailbox events
**          as posted by the interrupt service routine.
**
**  @param  a - Not used
**  @param  b - Not used
**  @param  port - QLogic chip instance (0-3)
**
**  @return none
**
******************************************************************************
**/
NORETURN void ISP_monitor_async(UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port)
{
    UINT32      currentSession;
    struct ev_t *entry;
    UINT32      event;
    UINT32      eventData;
    UINT32      i = port * 2 + 1;
    LOG_PORT_EVENT_PKT eldn;
    QCB        *Que;
    UINT32      qindex;
    struct ISP_2400 *pISP24base = ispstr[port]->baseAd;

    /* Store PCB for interrupt routine. */

    rtpcb[i] = K_xpcb;
    while (TRUE)
    {
        /* Set task status to wait for request. */

        TaskSetState(rtpcb[i], PCB_ISP_WAIT);

        /*
         ** Check if the queue is empty. Note that the process status is set
         ** to wait prior to checking the queue in case an interrupt occurs.
         ** The interrupt will set the process status ready when an event
         ** is placed on the queue.
         */

        if ((asyqa[port]->in == asyqa[port]->out) ||
            BIT_TEST(ispfail, port) || !BIT_TEST(isprena, port))
        {
            TaskSwitch();       /* Yield CPU until an asynchronous event occurs. */
        }
        else
        {
            /*
             ** The asynchronous event queue is not empty.
             ** Set task status to ready and process the queue.
             */
            TaskSetState(rtpcb[i], PCB_READY);
#ifdef HISTORY_KEEP
CT_history_pcb("ISP_monitor_async setting ready pcb", (UINT32)(rtpcb[i]));
#endif /* HISTORY_KEEP */
        }

        currentSession = sessids[port]; /* Get current Session ID */

        while (asyqa[port]->in != asyqa[port]->out)
        {
            entry = (struct ev_t *)asyqa[port]->out;
            event = entry->event;
            eventData = entry->data;

#ifdef DEBUG_FLT_REC_ISP
            MSC_FlightRec(0xAE1F, port, event, eventData);
#endif /* DEBUG_FLT_REC_ISP */
            switch (event)
            {
                case ISP_ASPFCU:       /* Loop UP */
                    ISP_LoopUp(port, entry);
                    break;

                case ISP_ASPFCD:       /* Loop DOWN */
                    ISP_LoopDown(port);
                    break;

                case ISP_LIPRCV:       /* LIP received         */
                case ISP_ASPLPR:       /* LIP Reset            */
                case ISP_ASPLIP:       /* LIP occurred         */
#ifdef ISP_DEBUG_FWSTATE
                    /* Get firmware state */

                    entry->event |= (UINT32)ISP_GetFirmwareState(port) << 16;
#endif /* ISP_DEBUG_FWSTATE */
                    eldn.header.event = LOG_LIP;        /* Log the LIP event */
                    eldn.data.port = port;
#ifdef FRONTEND
                    eldn.data.proc = 0;
#else  /* FRONTEND */
                    eldn.data.proc = 1;
#endif /* FRONTEND */
                    eldn.data.reason = event + (eventData << 16);
                    eldn.data.count = eventData;

                    /*
                     * Note: message is short, and L$send_packet copies into the MRP.
                     */
                    MSC_LogMessageStack(&eldn, sizeof(LOG_PORT_EVENT_PKT));

                    isp_reset_nphandleDB(port);

                    if (event == ISP_ASPLPR)    /* Is this a LIP Reset? */
                    {
                        ISP_GenOffline(port);
#ifdef FRONTEND
                        /*
                         ** Issue a Marker IOCB to this port, and
                         ** reset the LIP Issued condition.
                         */
                        ISP_SubmitMarker(port, 2, 0, 0);
                        ispLipIssued[port] = 0xFFFF;
#else  /* FRONTEND */
                        /* Set LIP Reset indicator for this port. */

                        ispLipIssued[port] = 0x100;

                        /* Set loop down request for this channel */

                        BIT_SET(P_chn_ind[port]->state, CH_LOOP_DN_REQ);

                        /* Clear notify request for this channel */

                        BIT_CLEAR(F_notifyreq, port);

                        F_startPortMonitor(port);       /* Start the port monitor */
#endif /* FRONTEND */
                    }           /* LIP Reset */

                    /*
                     ** Reset the last LIP timestamp for this port
                     ** to the current timestamp.
                     */
                    ispLastLIP[port] = timestamp;
#ifdef FABRIC
                    /* Force FC-4 Types (re-)registration with the switch. */

                    isp_ClearFC4TypesRegistration(port);
#endif /* FABRIC */
                    break;

                case ISP_ASPDBE:       /* Port Database Changed */
                    fprintf(stderr,"%s %s E:%04X  D:%08X \n",__func__,FEBEMESSAGE,event,eventData);
                    ISP_LoopUp(port, entry);
                    break;

                case ISP_ASPCNE:       /* Change Notification (Fabric RSCN) */
                    ISP_LoopUp(port, entry);
                    break;

                case ISP_ASPPTP:       /* Connnected Point-to-point */
                    break;

                case ISP_ASPFRD:       /* Frame Dropped */
                    eldn.header.event = LOG_FRAMEDROPPED;
                    eldn.data.port = port;
#ifdef FRONTEND
                    eldn.data.proc = 0;
#else  /* FRONTEND */
                    eldn.data.proc = 1;
#endif /* FRONTEND */
                    eldn.data.reason = ISP_ASPFRD;
                    eldn.data.count = eventData;
                    /*
                     * Note: message is short, and L$send_packet copies into the MRP.
                     */
                    MSC_LogMessageStack(&eldn, sizeof(LOG_PORT_EVENT_PKT));
                    break;

                case ISP_ASPLIE:       /* Loop Initialization Errors */
                    /* Log the Loop Initialization Error */

                    eldn.header.event = LOG_LIP;
                    eldn.data.port = port;
#ifdef FRONTEND
                    eldn.data.proc = 0;
#else  /* FRONTEND */
                    eldn.data.proc = 1;
#endif /* FRONTEND */
                    eldn.data.reason = event + (eventData << 16);
                    eldn.data.count = eventData;

                    /*
                     * Note: message is short, and L$send_packet copies into the MRP.
                     */
                    MSC_LogMessageStack(&eldn, sizeof(LOG_PORT_EVENT_PKT));
                    break;

                case ISP_ASPQFL:       /* Queue Full */
                    isp_getResourceCounts(port, 0);
                    break;

                case ISP_ASPNRS:
                    /* Update atio pointers */
                    qindex = pISP24base->atioQIP;
                    qindex = qindex << 4;
                    Que = (QCB *)ispstr[port]->atioQue;
                    Que->in = Que->begin + qindex;

                    /* Update response pointers */
                    qindex = pISP24base->rspQIP;
                    qindex = qindex << 4;
                    Que = (QCB *)ispstr[port]->resQue;
                    Que->in = Que->begin + qindex;

                    if (atiomonpcb[port] &&
                        TaskGetState(atiomonpcb[port]) == PCB_ISP_WAIT)
                    {
#ifdef HISTORY_KEEP
CT_history_pcb("ISP_monitor_async #2 setting ready pcb", (UINT32)(atiomonpcb[port]));
#endif /* HISTORY_KEEP */
                        TaskSetState(atiomonpcb[port], PCB_READY);
                    }
                    break;

                default:
#ifdef ISP_DEBUG_FWSTATE
                    /*
                     ** Get firmware state (saved in global firmwareState, if
                     ** this was a "forced" system error)
                     */
                    entry->event |= (UINT32)gFirmwareState << 16;
                    fprintf(stderr, "%s Fatal AEN 0x%04X occurred, firmware state = 0x%04X\n",FEBEMESSAGE, event, gFirmwareState);
#endif /* ISP_DEBUG_FWSTATE */
                    fprintf(stderr, "%s FATAL AEN Port %d Event %08X dumping ql\n",FEBEMESSAGE, port,entry->event);
                    /*
                     ** To check the OMB1=0 for the system error for
                     ** the work around for qlogic parity error
                     */
                    if ((event == ISP_ASPSYE) && ((eventData & 0x0000FFFF) == 0))
                    {
                        event = ISP_FATAL_ERROR;
                    }

                    /*
                     ** Dump qlogic registers and reset chip. This includes
                     ** asynchronous events System error, Request transfer error,
                     ** Response transfer error, and Firmware detected error.
                     */
                    ISP_DumpQL(port, event);
                    break;
            }

            /* The following is necessary to handle QLogic reset conditions */
            if (currentSession != sessids[port])
            {
                /* Do not process any additional entries */

                break;
            }

            /* Advance OUT pointer, and wrap if necessary. */

            asyqa[port]->out += 2;
            if (asyqa[port]->out >= asyqa[port]->end)
            {
                asyqa[port]->out = asyqa[port]->begin;
            }
        }                       /* while */

        /*
         ** Check for the asynchronous event queue stall flag set.
         ** This flag is set if an asynchronous event interrupt occurs
         ** while the queue is full.
         */
        if (BIT_TEST(ispaywt, port))
        {
            BIT_CLEAR(ispaywt, port);   /* Stall flag is set; clear it */

            /*
             ** Asynchronous queue is set big enough that it should never
             ** overflow. Reset the chip if this occurs.
             */

            ISP_ResetChip(port, ISP_ASYNC_Q_OVERFLOW);
        }
    }
}                               /* ISP_monitor_async */


/**
******************************************************************************
**
**  @brief      Submit a command to a QLogic ISP 2x00 chip instance.
**
**              A QLogic Request Packet (QRP) is submitted to the indicated
**              QLogic ISP 2x00 chip. Register g0 is the ordinal of the chip
**              for submission of the request. This routine always sets
**              the process status to wait for command complete; it only
**              gives up the CPU/blocks if register g2 is set to TRUE.
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      * qrp   - Pointer to QRP structure
**  @param      wait    - TRUE = wait requested for command completion
**
**  @return     none
**
******************************************************************************
**/
void isp_exec_cmd_sri(UINT8 port, QRP * qrp, UINT32 wait)
{


    qrp->iChip = port;          /* Set the chip instance in the QRP. */

    qrp->stFlag = FALSE;        /* Set command complete flag in QRP to FALSE */

    qrp->ombr[0] = 0;           /* Invalidate the output mailbox register 0 */

    /* Check for deferred command queue established */

    if (ispdefq[port * 2] == 0)
    {
        isp_mmbox(qrp);         /* Lookup ISP structure for this chip */

        ispdefq[port * 2] = qrp;        /* Set as head of queue */
    }
    else
    {
        /* Add request to deferred queue. */

        ispdefq[(port * 2) + 1]->pFThd = qrp;
    }

    ispdefq[(port * 2) + 1] = qrp;      /* Link this request to tail of chain. */

    qrp->pFThd = NULL;          /* Clear forward pointer in QRP */

    if (wait)                   /* Check if wait requested. */
    {
        qrp->pPCB = K_xpcb;     /* Set PCB into QRP. */

        if (qrp->stFlag == FALSE)
        {
            /*
             ** Set task status to wait until command completes and
             ** Give up CPU until command completes if wait requested.
             */
            TaskSetMyState(PCB_QLMBX_RSP_WAIT + port);
            TaskSwitch();
        }

        /*
         ** Did the mailbox command fail?
         ** Debug builds - always log the "failure" (some are considered normal).
         ** PERF builds - only log unexpected failures.
         */
#if 0
        //failed MBoxes are not log messages the fpritnf in process_mbx can do what we need for debug
        if (qrp->ombr[0] != ISP_CMDC)
        {
            UINT32      i;
#ifdef PERF
            if ((qrp->ombr[0] != ISP_PIU && qrp->ombr[0] != ISP_LIU) ||
                (qrp->imbr[0] == ISP_GTPD && qrp->ombr[0] == ISP_CPE))
#endif /* PERF */
            {
                LOG_MB_FAILED_PKT embf;

                /* When mailbox command fails, send debug message to CCB. */

                embf.header.event = LOG_MB_FAILED;
                embf.data.port = port;
#ifdef FRONTEND
                embf.data.proc = 0;
#else  /* FRONTEND */
                embf.data.proc = 1;
#endif /* FRONTEND */
                embf.data.iregs = qrp->iRegs;
                embf.data.oregs = qrp->oRegs;

                for (i = 0; i < dimension_of(embf.data.imbr); ++i)
                {
                    embf.data.imbr[i] = qrp->imbr[i];
                    embf.data.ombr[i] = qrp->ombr[i];
                }
#ifdef DEBUG_FLT_REC_ISP
                /* 0x58424D1F = 'XBM'<<8|0x1F */
                MSC_FlightRec(0x58424D1F, port, *(UINT32 *)embf.data.imbr,
                              *(UINT32 *)embf.data.ombr);
#endif /* DEBUG_FLT_REC_ISP */
                /*
                 * Note: message is short, and L$send_packet copies into the MRP.
                 */
                MSC_LogMessageStack(&embf, sizeof(LOG_MB_FAILED_PKT));
            }
        }
#endif
    }
    else
    {
        qrp->pPCB = NULL;       /* Do not associate this process with this QRP */
    }
}                               /* isp_exec_cmd_sri */


/**
******************************************************************************
**
**  @brief      Submit a command to a QLogic ISP 2x00 chip instance.
**
**              A QLogic Request Packet (QRP) is submitted to the indicated
**              QLogic ISP 2x00 chip. Register g0 is the ordinal of the chip
**              for submission of the request. This routine always sets
**              the process status to wait for command complete; it only
**              gives up the CPU/blocks if register g2 is set to TRUE.
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      * qrp   - Pointer to QRP structure
**  @param      wait    - TRUE = wait requested for command completion
**
**  @return     none
**
******************************************************************************
**/
void isp_exec_cmd(UINT8 port, QRP * qrp, UINT32 wait)
{
    static int  cmdCount;

    if ((qrp->imbr[0] == 0x6e || qrp->imbr[0] == 0x64) && cmdCount < 20)
    {
#if ISP_DEBUG_INFO
        fprintf(stderr, "isp_exec_cmd: %04x command from %p\n",
                qrp->imbr[0], __builtin_return_address(0));
#endif /* ISP_DEBUG_INFO */
        ++cmdCount;
    }

    /*
     ** Check if reset interlock is clear.
     */
    if (BIT_TEST(isprena, port) != 0)
    {
        isp_exec_cmd_sri(port, qrp, wait);
    }
    else
    {
        /* Set command as not Issued, port is not enabled */
        qrp->ombr[0] = ISP_CRST;

        qrp->stFlag = TRUE;     /* Set command as complete */
    }
}

/**
******************************************************************************
**
**  @brief      Issues the Get Resource Counts mailbox command to retrieve
**              the immediate notify and command resource counts.
**
**      The Get Resource Counts command returns the current command and
**      immediate notify resource counts, current and original free exchange
**      buffer counts, and current and original free IOCB buffer counts.
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      vpindex - Virtual Port Index
**
**  @return     none
**
******************************************************************************
**/
void isp_getResourceCounts(UINT8 port, UINT16 vpindex
#ifndef FRONTEND
                           UNUSED
#endif                          /* FRONTEND */
    )
{
    QRP        *qrp;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    /* Execute Get Resource Counts command */

    qrp->imbr[0] = ISP_GRC;
    qrp->imbr[1] = 0;           /* LUN 2200 only */
#ifdef FRONTEND
    qrp->imbr[9] = vpindex;     /* VP index 2300 mid only */
    qrp->iRegs = 0x203;         /* set modify mailbox reg 0, 1, 9 */
#else  /* FRONTEND */
    qrp->iRegs = 0x3;           /* set modify mailbox reg 0, 1 */
#endif /* FRONTEND */
    qrp->oRegs = 0x4CF;         /* set retrieve 0-3, 6-7, 10 mailbox regs */

    isp_exec_cmd(port, qrp, TRUE);

    if (qrp->ombr[0] == ISP_CMDC)
    {
        isprc_2400[port].originalfreeTXCBbufferCount = qrp->ombr[1];
        isprc_2400[port].currentfreeTXCBbufferCount = qrp->ombr[2];
        isprc_2400[port].currentfreeXCBbufferCount = qrp->ombr[3];
        isprc_2400[port].originalfreeXCBbufferCount = qrp->ombr[6];
        isprc_2400[port].currentfreeIOCBbuffercount = qrp->ombr[7];
        isprc_2400[port].originalfreeIOCBbuffercount = qrp->ombr[10];
#ifdef FRONTEND
        fprintf(stderr, "isp_getResourceCounts:FE ");
#else  /* FRONTEND */
        fprintf(stderr, "isp_getResourceCounts:BE ");
#endif /* FRONTEND */
        UINT32      i;

        for (i = 0; i < 11; i++)
        {
            fprintf(stderr, "ombr %d %04hX ", i, qrp->ombr[i]);
        }
        fprintf(stderr, "\n");
    }

    put_qrp(qrp);               /* Release QRP. */
}                               /* isp_getResourceCounts */


/**
******************************************************************************
**
**  @brief  Get the Loop ID for a given QLogic instance.
**
**          This routine retrieves the Loop ID for a QLogic instance as
**          passed in register g0. The Loop ID is placed into ispLid[].
**          The Port ID (only valid if connected to FL-Port) is placed
**          into portid[].
**
**  @param  port - QLogic chip instance (0-3)
**
**  @return Completion status
**
******************************************************************************
**/
UINT16 ISP_GetLoopId(UINT8 port,UINT8 vpid)
{
    QRP        *qrp;
    UINT16      retValue;
#ifdef FRONTEND
    TAR        *pTar;
#endif
    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    /* Execute Get Loop ID command */

    qrp->imbr[0] = ISP_GTID;
    qrp->imbr[1] = 0;           /* EFM Virtual Port ID */
    qrp->imbr[9] = vpid;           /* MID Virtual Port ID */
    qrp->oRegs = 0xCF;          /* Set retrieve 0-3, 6, 7 mailbox regs */

    qrp->iRegs = 0x201;         /* Set modify mailbox reg 0 and 9 */

    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];    /* Get return status of mailbox command. */

    if (retValue != ISP_CMDC)   /* Did the command fail? */
    {
        /* The command failed. Invalidate Loop ID and Port ID. */

        ispConnectionType[port] = NO_CONNECT;
        ispLid[port] = NO_CONNECT;
        portid[port] = NO_PORTID;
        goto err_exit;
    }
#ifdef BACKEND
    if (P_chn_ind[port])
    {
        P_chn_ind[port]->id = ispLid[port];     /* Set port's LID in CHN */
    }
#endif /* BACKEND */
    if (vpid == 0)
    {
        //only set for vpid 0
        /* Save the Loop ID and Port ID from response */
        ispLid[port] = (UINT16)qrp->ombr[1];
        /* Get topology */
        ispConnectionType[port] = qrp->ombr[6];
        portid[port] = qrp->ombr[2] + ((UINT32)qrp->ombr[3] << 16);
    }

#ifdef FRONTEND
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        if (pTar->vpID == vpid)
        {
            pTar->portID = qrp->ombr[2] + ((UINT32)qrp->ombr[3] << 16);
            //fprintf(stderr,"SMW Port %d vpid %d tid %d has aquired portid %06X \n",port,pTar->tid,pTar->vpID,pTar->portID );
        }
    }
#endif /* FRONTEND */

err_exit:
    put_qrp(qrp);               /* Release QRP. */

    return retValue;
}

/**
******************************************************************************
**
**  @brief      Issues the "Register FC4 Type" SNS subcommand
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      fc4Type - FC4 type.
**  @param      pTar - pointer to Tar structure
**
**  @return     return status
**
******************************************************************************
**/
UINT32 isp_registerFc4(UINT8 port, UINT32 fc4Type, TAR * pTar
#ifdef BACKEND
                       UNUSED
#endif                          /* BACKEND */
    )
{
    UINT16      retValue;

#ifdef FRONTEND
    if (pTar)
    {
        retValue = isp2400_sendctRFT_ID(port, fc4Type, pTar->portID, pTar->entry);
        return retValue;
    }
#endif /* FRONTEND */

    retValue = isp2400_sendctRFT_ID(port, fc4Type, portid[port], 0);
    return retValue;
}


#ifdef BACKEND
/**
******************************************************************************
**
**  @brief      Issues the Send LFA mailbox command.
**
**      The LFA (Loop Fabric Address) mailbox command enables the host system
**      to send loop management subcommands LINIT, LPC, and LSTS.
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      buffer  - LFA control block for subcommand
**  @param      length  - LFA control block length (16-bit words)
**  @param      vpindex - Virtual Port Index
**
**  @return     return status
**
******************************************************************************
**/
UINT32 isp_sendLFA(UINT8 port, void *buffer, UINT16 length, UINT16 vpindex UNUSED)
{
    QRP        *qrp;
    UINT16      retValue;
    union
    {
        void       *ptr;
        UINT16      s[2];
    } pBuffer;

    pBuffer.ptr = buffer;

    qrp = get_qrp();            /* Allocate a QRP to issue the mailbox command */

    qrp->imbr[0] = ISP_LFA;
    qrp->imbr[1] = length;      /* control block length (16 bit words)  */
    qrp->imbr[2] = pBuffer.s[1] + (UINT16)(K_poffset >> 16);
    qrp->imbr[3] = pBuffer.s[0];
    qrp->imbr[6] = 0;           /* Clear bits 63-32 of the buffer addr  */
    qrp->imbr[7] = 0;
#ifdef FRONTEND
    qrp->imbr[9] = vpindex;     /* VP index 2300 MID only               */
    qrp->iRegs = 0x2CF;         /* set modify mailbox reg 0-3, 6, 7, and 9 */
#else
    qrp->iRegs = 0x0CF;         /* set modify mailbox reg 0-3, 6, and 7   */
#endif
    qrp->oRegs = 0x03;          /* set retrieve mailbox reg 0, 1         */

    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];

    put_qrp(qrp);               /* Release QRP. */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Issues the "Loop Initialize (LINIT)" LFA subcommand
**
**      The Loop Initialize (LINIT) subcommand requests loop initialization
**      on a remote fabric loop. The remote FL_Port connected to the loop
**      addressed by the LFA originates a LIP of the type specified in buffer
**      offsets 26 & 27 (lip4 & lip3) of the system request buffer.
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      loopID  - Loop ID of the remote target of the LIP.
**
**  @return     return status
**
******************************************************************************
**/
UINT32 ISP_LoopInitialize(UINT8 port, UINT32 lid)
{
    UINT32      retValue;
    union
    {
        UINT16      s[2];
        UINT32      word;
    } portID;
    struct
    {
        UINT16      rspLength;  /* Response buffer length (4 words) */
        UINT16      rsvd1;      /* Reserved                         */
        UINT64      rspBufferAddr;      /* Response buffer addr, bits 63-0  */
        UINT16      subCmdLength;       /* Subcommand length (4 words)      */
        UINT16      rsvd2;      /* Reserved                         */
        UINT32      lfa;        /* LFA (from GID_PT)                */
        UINT16      subCmd;     /* LINIT subcommand                 */
        UINT16      rsvd3;      /* Reserved                         */
        UINT16      login;      /* Initialization function          */
        /*   0000h = normal init            */
        /*   0001h = force login (FLOGI)    */
        UINT8       lip4;       /* 4th character of LIP sequence    */
        UINT8       lip3;       /* 3rd character of LIP sequence    */
    }          *reqBuffer;      /* LINIT request buffer, 28 bytes   */
    struct
    {
        UINT32      responseCode;       /* LS_ACC = 02000000h or            */
        /* LS_RJT = 01000000h               */
        UINT8       rsvd1[3];   /* Reserved                         */
        UINT8       status;     /* Status                           */
        /*   01h = LINIT comp. successfully */
        /*   02h = LINIT failed             */
    }          *rspBuffer;      /* LINIT response buffer, 8 bytes   */

    portID.s[1] = (UINT16)portdb[port][lid].pid;
    portID.s[0] = (UINT16)(portdb[port][lid].pid >> 16);


    /*
     ** Verify that the port ID is valid, i.e. nonzero
     ** For performance (customer) builds, ignore the request, as it will cause
     ** a mailbox command timeout anyway. For debug builds, fault here.
     ** See defect TB0lt00010505 for more details.
     */
    if (portID.word == 0)
    {
#ifndef PERF
        abort();
#else  /* PERF */
        return ISP_PID;
#endif /* PERF */
    }

    /* Allocate (and clear) memory for request and response buffers */

    rspBuffer = s_MallocC(sizeof(*rspBuffer), __FILE__, __LINE__);
    reqBuffer = s_MallocC(sizeof(*reqBuffer), __FILE__, __LINE__);

    /* Fill in request buffer. */

    reqBuffer->rspLength = sizeof(*rspBuffer) / 2;
    reqBuffer->rspBufferAddr = (UINT32)rspBuffer + K_poffset;
    reqBuffer->subCmdLength = 4;
    reqBuffer->lfa = portID.word & 0xFFFFFF00;
    reqBuffer->subCmd = LINIT;
    reqBuffer->login = 0;

    /* Specify Selective LIP(al_pd, al_ps) where al_ps is the switch = 0x00 */

    reqBuffer->lip4 = 0x00;     /* al_ps */
    reqBuffer->lip3 = (UINT8)portID.word;       /* al_pd */

    /* Send command to remote fabric address */

    retValue = isp_sendLFA(port, (void *)reqBuffer, sizeof(*reqBuffer) / 2, 0);
#ifdef DEBUG_FLT_REC_ISP
    MSC_FlightRec(FR_ISP_LINIT, port, portID.word, retValue);
#endif /* DEBUG_FLT_REC_ISP */

    /* If GOOD return value, check for an LS_ACC response code. */

    if (retValue == ISP_CMDC && rspBuffer->status != 0x01)
    {
        retValue = rspBuffer->status;
    }

    /* Release memory used. */

    s_Free(reqBuffer, sizeof(*reqBuffer), __FILE__, __LINE__);
    s_Free(rspBuffer, sizeof(*rspBuffer), __FILE__, __LINE__);

    return retValue;
}
#endif /* BACKEND */


#ifdef FRONTEND

/**
******************************************************************************
**
**  @brief      Get Virtual Port Database
**
**  @param      port - QLogic chip instance (0-3)
**  @param      vpindex - virtual port
**  @param      vport - config data.
**
**  @return     return status
**
******************************************************************************
**/
static UINT16 isp2400_get_vpdatabase(UINT8 port, UINT8 vpindex, ISP2400_VPDB_PORTCFG * vport)
{
    QRP        *qrp;
    UINT32      retValue;
    union
    {
        void       *ptr;
        UINT16      s[2];
    } pBuffer;

    /** Get loop ID of the virtual port.
    ** issue a getvport by index Mbox for each vp
    */
    qrp = get_qrp();
    if (!qrp)
    {
        return 1;
    }

    pBuffer.ptr = vport;
    qrp->imbr[0] = 0x004A;
    qrp->imbr[2] = pBuffer.s[1] + (UINT16)(K_poffset >> 16);
    qrp->imbr[3] = pBuffer.s[0];
    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;
    qrp->imbr[9] = vpindex;

    qrp->iRegs = 0x2CD;         /* Set modify mailbox reg 0, 2, 3, 6, 7 */
    qrp->oRegs = 0x1;           /* Set retrieve 0 mailbox regs */

    isp_exec_cmd_sri(port, qrp, TRUE);

    /* Check return status of mailbox command. */

    if (qrp->ombr[0] == ISP_CMDC)
    {
        retValue = 0;           /* set good return value */
    }
    else
    {
        /* Return getvport by index failed */

        fprintf(stderr, "Return getvport by index failed\n");

        retValue = 1;
    }

    put_qrp(qrp);               /* Release QRP. */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Sets the virtual port ID acquired for each target..
**
**      The data returned from the Report ID Acquisition IOCB is
**      processed to set the virtual port ID in the CIMT and the
**      TGD structures. Each VP_ACQUIRED_ID is extracted and using
**      the TGD index from the associated TAR structure, the
**      virtual port ID is set in the corresponding TGD structure.
**
**  @param      port - QLogic chip instance (0-3)
**  @param      iocb - IOCB pointer.
**
**  @return     none
**
******************************************************************************
**/
static void isp_processIdAcquisition(UINT8 port, VPRID_IOCB * riocb)
{
    ISP2400_VPDB_PORTCFG *vport;
    TAR        *pTar;

    /* Handle the format 0 amd format1  ReportId acquisition IOCB */
    if (riocb->format == 1)
    {
        UINT32      portID;

        vport = s_MallocC(sizeof(*vport), __FILE__, __LINE__);
#if ISP_DEBUG_INFO
        fprintf(stderr, "%s: PORT %d format 1\n", __func__, port);
#endif /* ISP_DEBUG_INFO */
        portID = riocb->pid[0] | (riocb->pid[1] << 8) | (riocb->pid[2] << 16);
        isp2400_get_vpdatabase(port, riocb->vpindex, vport);
#if ISP_DEBUG_INFO
        fprintf(stderr, "db for vpid %d alpa %06X nodename %llX portname %llX\n",
                riocb->vpindex, portID, vport->nodeWWN, vport->portWWN);
#endif /* ISP_DEBUG_INFO */

        vport->portWWN = myhtonll(vport->portWWN);
        vport->portWWN = longlongWordSwap(vport->portWWN);
        for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
        {
            if (pTar->portName == vport->portWWN)
            {
                break;
            }
        }
        if (pTar != NULL)
        {

            pTar->portID = portID;
            pTar->vpID = riocb->vpindex;
            /*
             ** Loop ID changed, so we'll need to (re-)register FC-4 Types.
             ** Clear the flags for this target.
             */
            pTar->flags = FALSE;

            /*
             ** Check if target ID is valid. The control port
             ** uses a target ID of 0xFFFF
             */
            fprintf(stderr, " Matched tar struct tid: %X opt %X status %X\n", pTar->tid, pTar->opt, vport->vpstatus);
            if (pTar->tid < MAX_TARGETS)
            {
                /* Update the port assignment for this target */

                ispPortAssignment[pTar->tid] = port;
            }
            else if (BIT_TEST(pTar->opt, TARGET_ENABLE) && (vport->vpstatus & VPDB_NOT_PARTICIPATING))
            {
                /* Invalidate Port ID and VP ID */

                pTar->vpID = NO_CONNECT;
                pTar->portID = NO_PORTID;

                /* Is this target currently assigned to this port? */

                if (pTar->tid < MAX_TARGETS && ispPortAssignment[pTar->tid] == port)
                {
                    /* Update the port assignment for this target */

                    ispPortAssignment[pTar->tid] = 0xFF;
                }
            }                   /* end else if */
        }
        BIT_SET(id_acquired, port);     /* Indicate virtual port ID acquired */
        s_Free(vport, sizeof(*vport), __FILE__, __LINE__);
    }
    if (riocb->format == 0)
    {
        isp2400_processIdAcquisition(port, riocb);
    }
}                               /* isp_processIdAcquisition */
#endif /* FRONTEND */


#ifdef FABRIC
/**
******************************************************************************
**
**  @brief      Register FC4 types for each virtual port
**
**  @param      port - QLogic chip instance (0-3)
**
**  @return     none
**
******************************************************************************
**/
void isp_registerVports(UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port)
{
    TAR        *pTar;
    UINT16      retValue;
//    UINT32      retValue_2400;

    /* Clear Register FC-4 failed flag */
    fc4Failed[port] = FALSE;

    /* Check for RIA IOCB received. If not, wait 1/4 second and retry. */
    while (!BIT_TEST(id_acquired, port))
    {
        TaskSleepMS(250);
    }

    /* If tar list got changed, we might need to retry. */
  restart:
    /* Traverse the target list for this port. */
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {

        if (pTar->tid < MAX_TARGETS && BIT_TEST(pTar->opt, TARGET_ENABLE) &&
            pTar->flags == FALSE)
        {
            /* NOTE: ISP_LoginLoopPort will task switch. */
            UINT32 save_tar_link_abort = tar_link_abort[port];

            /*
             * All targets are in the same domain, so the upper two byte
             * of the port ID are the same. The lower byte is the ALPA.
             */
             pTar->portID |= portid[port] & 0xFFFF00;

            /*
             * In 2400, there are two types of reportID acquisitions
             * Format0 and Format1. After receiving the Format0 we will populate
             * all our TAR structures(similar to 2300). After receiving the
             * Format1 we need to logon to SNS for that particular vpindex(Ref. ISP2400
             * interface document), before registering with the Fabric as a FC4 Target.
             */
//            retValue_2400 = ISP_LoginLoopPort(port, SNS_NPORT_HANDLE, pTar->vpID);
            ISP_LoginLoopPort(port, SNS_NPORT_HANDLE, pTar->vpID);

            /* If tar list got smaller, then the linked list may not be valid, restart. */
            if (save_tar_link_abort != tar_link_abort[port])
            {
                goto restart;
            }

            if (pTar->vpID)
            {
//                retValue_2400 = ISP2400_SendSCR(port, pTar->vpID);
                ISP2400_SendSCR(port, pTar->vpID);

                /* If tar list got smaller, then the linked list may not be valid, restart. */
                if (save_tar_link_abort != tar_link_abort[port])
                {
                    goto restart;
                }
            }

            /* Register this VP as SCSI target. */
#if ISP_DEBUG_INFO
            fprintf(stderr, "%s: calling isp_registerFc4 PORT %d alpa %06X vpid %02X\n",
                    __func__, port, pTar->portID, pTar->vpID);
#endif /* ISP_DEBUG_INFO */
            retValue = isp_registerFc4(port, 0x100, pTar);

            /* If tar list got smaller, then the linked list may not be valid, restart. */
            if (save_tar_link_abort != tar_link_abort[port])
            {
                goto restart;
            }
            /* Check if the command completed successfully. */
            if (retValue == GOOD)
            {
                /* Set flag indicating this port registered its FC-4 type. */
                pTar->flags = TRUE;
            }
            else
            {
                /* Set flag indicating the registration failed. */
                BIT_SET(fc4Failed[port], pTar->entry);
            }
        }
    }

    /* Check if Register FC-4 failed. */
    if (fc4Failed[port] == FALSE)
    {
        BIT_SET(fc4flgs, port); /* Set as FC-4 registered. */
    }

    /* Clear FC-4 registration PCB */
    fc4pcb[port] = NULL;
}                               /* isp_registerVports */
#endif /* FABRIC */


/**
******************************************************************************
**
**  @brief update request queue in pointer
**
**  @param      port - QLogic chip instance (0-3)
**  @param      rqip   new index
**
**  @return     none
**
******************************************************************************
**/
static inline void update_rqip(UINT8 port, UINT32 rqip)
{
    ISP_2400   *isp2400hwptr = (ISP_2400 *)ispstr[port]->baseAd;

    FORCE_WRITE_BARRIER;
    isp2400hwptr->reqQIP = rqip;
}


UINT32 ISP2400_SendSCR(UINT8 port, UINT16 vpindex)
{
    UINT32      rqip;
    UINT32      retValue = 0;
    ILT        *ilt;
    ELSPASSTHRU_IOCB *elsiocb;
    ELS_SCR    *pSCR;
    SCR_LSACC  *plsacc;

    pSCR = s_MallocC(sizeof(*pSCR), __FILE__, __LINE__);
    plsacc = s_MallocC(sizeof(*plsacc), __FILE__, __LINE__);
    ilt = get_ilt();            /* get an ILT w/wait                */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */

    ilt->ilt_normal.w0 = (UINT32)pSCR;
    ilt->ilt_normal.w1 = (UINT32)plsacc;
    elsiocb = (ELSPASSTHRU_IOCB *)isp_get_iocb(port, &rqip);
    if (elsiocb != NULL)
    {
        memset(elsiocb, 0, sizeof(*elsiocb));
    }
    else
    {
        s_Free(pSCR, sizeof(*pSCR), __FILE__, __LINE__);
        s_Free(plsacc, sizeof(*plsacc), __FILE__, __LINE__);
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
        put_ilt(ilt);
        fprintf(stderr, "%s: elsiocb is NULL\n", __func__);
        return 1;
    }

    pSCR->Function = 0x03000000;        /* scr request */
    pSCR->opcode = 0x62;

    /* ELS pass thru iocb type */

    elsiocb->entryType = 0x53;
    elsiocb->entryCount = 1;
    elsiocb->nportHandle = 0x7fD;
    elsiocb->TxDsdCount = 1;    /* Will only transmit one DSD */
    elsiocb->vpindex = vpindex;
    elsiocb->SOFType = ELS_SOFI3;       /* SOFi3 is used */
    elsiocb->RxDsdCount = 1;    /* Always receives one DSD */
    elsiocb->ElsOpcode = 0x62;  /* SCR opcode */
    elsiocb->Did0_15 = 0xFFFD;
    elsiocb->Did23_16 = 0XFF;
    elsiocb->ControlFlags = 0x0000;
    elsiocb->RxtotalBytes = 4;
    elsiocb->TxtotalBytes = 8;
    elsiocb->Txdsd[0] = LI_GetPhysicalAddr((UINT32)pSCR);
    elsiocb->Txdsd[1] = 0;
    elsiocb->TxdsdLength = 8;
    elsiocb->Rxdsd[0] = LI_GetPhysicalAddr((UINT32)plsacc);
    elsiocb->Rxdsd[1] = 0;
    elsiocb->RxdsdLength = 4;
    elsiocb->iocbHandle = (UINT32)ilt;

    /* Submit IOCB by updating Request Queue IN pointer */

    update_rqip(port, rqip);

    return retValue;
}

static UINT32 ISP2400_HandleElsIocb(UINT8 port, void *iocb)
{
    ILT        *ilt;
    ELSPASSTHRU_IOCB *elsiocb = (ELSPASSTHRU_IOCB *)iocb;

    ISP_GetFirmwareState(port);

#if ISP_DEBUG_INFO
    fprintf(stderr, "%s PORT %d vpid %d iocb stat %02X\n",
            __func__, port, elsiocb->vpindex, elsiocb->entryStatus);
#endif /* ISP_DEBUG_INFO */
    ilt = (ILT *)elsiocb->iocbHandle;
    if (!verify_iocbhandle(ilt))
    {
        ILT *tmpILT = (ILT*)bswap_32((UINT32)ilt);
        if (!verify_iocbhandle(tmpILT))
        {
            fprintf(stderr, "%s%s:%u %s nphandle is not an ILT (%p), ignoring...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            return GOOD;
        }
        else
        {
            fprintf(stderr, "%s%s:%u %s nphandle was byteswapped (%p), correcting...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            ilt = tmpILT;
        }
    }
    s_Free((void *)ilt->ilt_normal.w0, sizeof(ELS_SCR), __FILE__, __LINE__);
    s_Free((void *)ilt->ilt_normal.w1, sizeof(SCR_LSACC), __FILE__, __LINE__);
    put_ilt(ilt);
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    return GOOD;
}


/**
******************************************************************************
**
**  @brief      Issues the get virtual port database mailbox command to
**              retrieve the virtual port data base.
**
**              More details on this function go here.
**
**  @param      port        - QLogic chip instance (0-3)
**  @param      * buffer    - pointer to a buffer for the Virtual Port Database
**
**  @return     Number of entries in virtual port database
**
******************************************************************************
**/
UINT16 ISP_GetVPDatabase(UINT8 port, VPD * buffer)
{
    QRP        *qrp;
    UINT16      retValue;
    union
    {
        VPD        *ptr;
        UINT16      s[2];
    } pBuffer;

    /*  Get the pointer to the destination buffer. */

    pBuffer.ptr = buffer;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_GVPD;    /*  Execute Get Virtual Port Database command */
    qrp->iRegs = 0xCD;          /* set modify mailbox reg 0, 2, 3, 6, 7     */
    qrp->oRegs = 0x03;          /* set retrieve mailbox reg 0 and 1     */
    qrp->imbr[2] = pBuffer.s[1] + (UINT16)(K_poffset >> 16);
    qrp->imbr[3] = pBuffer.s[0];
    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;

    isp_exec_cmd(port, qrp, TRUE);

    if (qrp->ombr[0] == ISP_CMDC)       /* If the command was successful */
    {
        retValue = qrp->ombr[1];        /*  Get the VP entry count. */
    }
    else
    {
        retValue = 0;           /* Command failed, set VP entry to zero. */
    }

    put_qrp(qrp);               /* Release QRP. */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Get the corresponding index for a specified virtual port ID.
**
**              This routine search the target record for the specified port
**              and returns the index of the target entry with the specified
**              virtual port ID.
**
**  @param      port    - Port
**  @param      vpid    - VPID
**
**  @return     Index for a input virtual port ID
**
******************************************************************************
**/
UINT16 isp_findVpIndex(UINT8 port, UINT16 vpid)
{
    TAR        *pTar = tar[port];
    UINT16      i = 0;

    /* Check all target structures. */
    while (pTar != NULL)
    {
        /* Check if this the target with the specified Loop ID */
        if (pTar->vpID == vpid)
        {
            i = pTar->entry;
            break;
        }

        /* Increment to the next target in the linked list. */
        pTar = pTar->fthd;
    }
    return i;
}


/**
******************************************************************************
**
**  @brief      Copies a 2400 port database entry to a 2300 entry
**
**  @param      pdb_2400    - source data
**  @param      pdb_2300    - dest data
**
**  @return  none
**
******************************************************************************
**/
static void convert_2400pdb(PDB_2400 * pdb_2400, PDB * pdb_2300)
{
    pdb_2300->opt = pdb_2400->flags & 0xff;
    pdb_2300->cnt = 0;
    pdb_2300->sst = pdb_2400->lastloginstate;
    pdb_2300->mst = pdb_2400->curloginstate;
    pdb_2300->had = pdb_2400->hardaddr;
    pdb_2300->pid = (UINT32)pdb_2400->portid[0] |
                    (UINT32)(pdb_2400->portid[1] << 24) |
                    (UINT32)(pdb_2400->portid[2] << 16);
    pdb_2300->ndn = pdb_2400->nodename;
    pdb_2300->pdn = pdb_2400->portname;
    pdb_2300->recvDataSize = pdb_2400->recvDataSize;
    pdb_2300->portTimer = pdb_2400->portTimer;
    pdb_2300->nextSeqID = pdb_2400->seqid;
    pdb_2300->prliw0 = pdb_2400->prliw0;
    pdb_2300->prliw3 = pdb_2400->prliw3;
    pdb_2300->lid = pdb_2400->nporthandle;

    /*
     * The other 2300 fields do not have comparible data
     * they do not seem to get examined anywhere in the code
     */
}


/**
******************************************************************************
**
**  @brief      Get the QLogic Port Database for a given Loop ID.
**
**              This routine retrieves the Port Database for the loop ID
**              passed to this routine in register g1. The Port Database is
**              stored in the preallocated table space for the chip instance
**              (in register g0) from the anchor stored in the table <_portdb>.
**
**              The flag corresponding to the Loop ID is set in the Database
**              flags indicating that this initiator is in the database.
**
**              The Server Database is searched for the Port Name as received
**              in the Port Database and an ordinal is assigned to the
**              Loop ID that was passed in g1; this ordinal is stored in the
**              Loop ID-to-server ordinal database.
**
**  @param      port    - Port
**  @param      lid     - LID
**  @param      vpid    - VPID
**
**  @return     return status.
**
******************************************************************************
**/
UINT16 ISP_GetPortDB(UINT8 port, UINT32 lid, UINT16 vpid
#ifndef FRONTEND
                     UNUSED
#endif                          /* FRONTEND */
    )
{
    QRP        *qrp;
    UINT16      retValue;
    PDB        *ptr;
    PDB_2400   *pdb_2400 = NULL;
    union
    {
        PDB        *ptr;
        UINT16      s[2];
    } pBuffer;

#ifdef FRONTEND
    ILT        *ilt;
    UINT32      i;
    UINT32      ownedbyvpidcheck;
#endif

    /* Get the pointer to the destination buffer. */

#if ISP_DEBUG_INFO
    fprintf(stderr, "%s: PORT %d lid %04X vpid %02X\n", __func__, port, lid, vpid);
#endif /* ISP_DEBUG_INFO */

    ptr = portdb[port] + lid;
    pdb_2400 = s_MallocC(sizeof(*pdb_2400), __FILE__, __LINE__);
    pBuffer.ptr = (PDB *)pdb_2400;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_GTPD;    /* Execute Get Port Database command */
    qrp->oRegs = 0x3;           /* Set retrieve mailbox regs 0       */
    qrp->imbr[1] = lid;         /* Loop ID                           */

#ifdef FRONTEND
    /* If this is the MID code, store the VP index in mailbox register 9. */
    qrp->iRegs = 0x6FF;         /* Set modify mailbox reg 0-10   */
    qrp->imbr[9] = vpid;        /* MID virtual port     */
#else  /* FRONTEND */
    qrp->iRegs = 0x6FF;         /* Set modify mailbox reg 0-3, 6-7, 10      */
#endif /* FRONTEND */

    qrp->imbr[2] = pBuffer.s[1] + (UINT16)(K_poffset >> 16);
    qrp->imbr[3] = pBuffer.s[0];
    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;
    qrp->imbr[4] = 0;           /* portid 0-15  not used in this mode, 2400 only */
    qrp->imbr[5] = 0;           /* portid 16-23 not used in this mode, 2400 only */
    qrp->imbr[10] = 0;          /* Options 0; 2400 only */

    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];    /* Get completion status. */

    put_qrp(qrp);               /* Release QRP. */

    if (retValue == ISP_CMDC)   /* If a Good completion */
    {
        convert_2400pdb(pdb_2400, ptr);
#ifdef FRONTEND
        /*
         ** Check if this LID was previously used by a different initiator (WWN)
         */
        if (servdb[port][lid] != 0 && ptr->pdn != servdb[port][lid])
        {
#if ISP_DEBUG_INFO
            fprintf(stderr, "%s: PORT first check %d lid %04X vpid %02X %016llX %016llX\n",
                    __func__, port, lid, vpid, ptr->pdn, servdb[port][lid]);
#endif /* ISP_DEBUG_INFO */
            ilt = get_ilt();    /* Get an ILT w/wait    */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
            ilt->ilt_normal.w0 = ISP_LOG_OFF_CMD;       /* Store command byte   */
            ilt->ilt_normal.w0 |= port << 8;    /* Store Chip ID        */
            ilt->ilt_normal.w1 = lid;   /* Store Initiator ID   */
            ilt[1].misc = (UINT32)ilt;
            ilt->cr = NULL;     /* No completion handler            */
            ++ilt;              /* Get next level of ILT            */

            /* No completion routine, cdriver will release ilt */

            ilt->cr = NULL;     /* No completion handler            */

            C_recv_scsi_io(ilt);
        }

        /* Check if this WWN is stored at any other LID. */

        for (i = 0; i < MAX_DEV; ++i)
        {
            if (serverdbOwnerbyVpid[port][i] == vpid)
            {
                ownedbyvpidcheck = 1;
            }
            else
            {
                ownedbyvpidcheck = 0;
            }

            if (ptr->pdn == servdb[port][i] && i != lid && ownedbyvpidcheck)
            {
                /*
                 ** This WWN used to be at this. Logoff
                 ** this LID
                 */
#if ISP_DEBUG_INFO
                fprintf(stderr, "%s: PORT second check %d lid %04X vpid %02X %016llX\n",
                        __func__, port, lid, vpid, ptr->pdn);
#endif /* ISP_DEBUG_INFO */
                ilt = get_ilt();        /* get an ILT w/wait */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
                ilt->ilt_normal.w0 = ISP_LOG_OFF_CMD;   /* store command byte */
                ilt->ilt_normal.w0 |= port << 8;        /* store Chip ID */
                ilt->ilt_normal.w1 = i; /* store Initiator ID */
                ilt[1].misc = (UINT32)ilt;
                ilt->cr = NULL; /* No completion handler */
                ++ilt;          /* Get next level of ILT */

                /* No completion routine, cdriver will release ilt */

                ilt->cr = NULL;

                C_recv_scsi_io(ilt);

                /* Set the server's port name in the server database. */

                servdb[port][i] = 0;

                /* Set byte indicating database entry is empty */

                dbflags[port][i] = FALSE;
                serverdbOwnerbyVpid[port][i] = 0xff;
            }
        }

        /* Set the server's port name in the server database. */

        servdb[port][lid] = ptr->pdn;

        dbflags[port][lid] = TRUE;      /* Indicate database updated */

        serverdbOwnerbyVpid[port][lid] = vpid;
    }
    else
    {
        ilt = get_ilt();        /* get an ILT w/wait */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
        ilt->ilt_normal.w0 = ISP_LOG_OFF_CMD;   /* store command byte */
        ilt->ilt_normal.w0 |= port << 8;        /* store Chip ID */
        ilt->ilt_normal.w1 = lid;       /* store Initiator ID */
        ilt[1].misc = (UINT32)ilt;
        ilt->cr = NULL;         /* No completion handler */
        ++ilt;                  /* Get next level of ILT */

        /* No completion routine, cdriver will release ilt */
        ilt->cr = NULL;         /* No completion handler */

        C_recv_scsi_io(ilt);

        /* Set the server's port name in the server database. */

        servdb[port][lid] = 0;

        /* Set byte indicating database entry is empty */

        dbflags[port][lid] = FALSE;
        serverdbOwnerbyVpid[port][lid] = 0xff;
#endif /* FRONTEND */
    }

    s_Free(pdb_2400, sizeof(*pdb_2400), __FILE__, __LINE__);
    return retValue;
}


/**
******************************************************************************
**
**  @brief  Issue the Enhanced Get Port Database mailbox command.
**
**          This routine retrieves the Port Database for the loop ID passed
**          to this routine in register g1. The Port Database is stored in
**          the preallocated table space for the chip instance (in register
**          g0) from the anchor stored in the table <_portdb>.
**
**          This Enhanced Get Port Database command will also attempt to
**          login the loop device if the state is not 'Port Logged In'
**          (loop only, not fabric - we login manually in that case).
**
**  @param  port - Port
**  @param  lid - LID/nphandle
**  @param  alpa - alpa
**
**  @return UINT32  - ISP completion code (15:0) and add'l info (31:16)
**                        4000h = ISP_CMDC = Successful command completion
**                        4005h = ISP_CMDE = Command Error, add'l info:
**                            0001h = The link or loop is not operational
**                            0002h = ISP fw cannot allocate an IOCB resource
**                            0003h = ISP fw cannot allocate exch. ctrl blk res.
**                            0004h = ISP fw didn't receive ACC for a PLOGI or
**                                    PRLI within the E_D_TOV
**                            0005h = No fabric loop port (fabric only)
**                            0006h = Remote device does not support target fn
**                            0007h = ISP fw is not in a ready state
**                            000Dh = Login Reject Error (LS_RJT received);
**                                    see OMB 6 & 7.
**
******************************************************************************
**/
UINT32 ISP_EnhancedGetPortDB(UINT8 port, UINT32 lid, UINT32 alpa)
{
    QRP        *qrp;
    UINT32      retValue;
    PDB        *ptr;
    PDB_2400   *pdb_2400;
    union
    {
        PDB        *ptr;
        UINT16      s[2];
    } pBuffer;

    ptr = portdb[port] + lid;   /* Get the pointer to the destination buffer. */

    pdb_2400 = s_MallocC(sizeof(*pdb_2400), __FILE__, __LINE__);
    pBuffer.ptr = (PDB *)pdb_2400;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_GTPD;    /* Execute Enhanced Get Port Database command */
    qrp->imbr[4] = (UINT16)(alpa & 0xFFFF);     /* Portid 0-15  not used in this mode, 2400 only */
    qrp->imbr[5] = (UINT16)(alpa >> 16);        /* Portid 16-23 not used in this mode, 2400 only */
    qrp->iRegs = 0x6FF;         /* Set modify mailbox reg 0-3, 4-7, 9-10 */
    qrp->oRegs = 0x3;           /* Set retrieve mailbox regs 0-1,    */
    qrp->imbr[1] = lid;         /* Loop ID (bits 15-0)              */

    qrp->imbr[2] = pBuffer.s[1] + (UINT16)(K_poffset >> 16);
    qrp->imbr[3] = pBuffer.s[0];
    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;

    /* Check for fabric mode. Login only when in non-fabric mode. */

    if (BIT_TEST(ispfflags, port) == 0)
    {
        qrp->imbr[10] = 1;      /*login (bit 0) */
    }
    else
    {
        qrp->imbr[10] = 0;      /*login (bit 0) */
    }

    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];    /* Get completion status. */

    if (retValue == ISP_CMDC)
    {
        convert_2400pdb(pdb_2400, ptr);

        /* If a command is successful, get master state. */

        retValue |= ptr->mst << 16;
    }
    else if (retValue == ISP_CMDE)
    {
        /* If a command error occurred, get additional status from mailbox 1. */
        retValue |= qrp->ombr[1] << 16;
    }

    put_qrp(qrp);               /* Release QRP. */

    s_Free(pdb_2400, sizeof(*pdb_2400), __FILE__, __LINE__);

    return retValue;
}


/**
******************************************************************************
**
**  @brief  Issue the Enhanced Get Port Database mailbox command.
**
**          This routine retrieves the Port Database for the loop ID passed
**          to this routine in register g1. The Port Database is stored in
**          the preallocated table space for the chip instance (in register
**          g0) from the anchor stored in the table <_portdb>.
**
**          This Enhanced Get Port Database command will also attempt to
**          login the loop device if the state is not 'Port Logged In'
**          (loop only, not fabric - we login manually in that case).
**
**  @param  port - Port
**  @param  lid - LID/nphandle
**
**  @return UINT32  - ISP completion code (15:0) and add'l info (31:16)
**                        4000h = ISP_CMDC = Successful command completion
**                        4005h = ISP_CMDE = Command Error, add'l info:
**                            0001h = The link or loop is not operational
**                            0002h = ISP fw cannot allocate an IOCB resource
**                            0003h = ISP fw cannot allocate exch. ctrl blk res.
**                            0004h = ISP fw didn't receive ACC for a PLOGI or
**                                    PRLI within the E_D_TOV
**                            0005h = No fabric loop port (fabric only)
**                            0006h = Remote device does not support target fn
**                            0007h = ISP fw is not in a ready state
**                            000Dh = Login Reject Error (LS_RJT received);
**                                    see OMB 6 & 7.
**
******************************************************************************
**/
UINT32 ISP_EnhancedGetPortDBHdl(UINT8 port, UINT32 lid)
{
    PDB        *ptr;
    UINT32      alpa;

    /* Get the pointer to the destination buffer. */

    ptr = portdb[port] + lid;
    alpa = pdbPidToALPA(ptr->pid);

    return ISP_EnhancedGetPortDB(port, lid, alpa);
}


/**
******************************************************************************
**
**  @brief      Get the Loop position map for a given QLogic instance.
**
**              This routine retrieves the Loop position map for a QLogic
**              instance as passed in register g0. The loop position map
**              is stored in lpmap
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     Completion status
**
******************************************************************************
**/
UINT16 ISP_GetPositionMap(UINT8 port)
{
    QRP        *qrp;
    union
    {
        UINT8      *lpmap;
        UINT16      s[2];
    } pBuffer;
    UINT16      retValue;

    /* Check if connected Loop or FL_port. */

    if (ispConnectionType[port] == LOOP || ispConnectionType[port] == FL_PORT)
    {
        pBuffer.lpmap = lpmap[port];

        qrp = get_qrp();        /* Allocate a QRP to issue mailbox commands */

        /* Execute Get FC-AL Position Map command */

        qrp->imbr[0] = ISP_GTPM;
        qrp->imbr[2] = pBuffer.s[1] + (UINT16)(K_poffset >> 16);
        qrp->imbr[3] = pBuffer.s[0];
        qrp->imbr[6] = 0;
        qrp->imbr[7] = 0;
        qrp->iRegs = 0xCD;      /* set modify mailbox reg 0, 2, 3, 6, 7  */
        qrp->oRegs = 0x03;      /* set retrieve 0, 1 mailbox regs     */

        isp_exec_cmd(port, qrp, TRUE);

        retValue = qrp->ombr[0];        /* Get return status of mailbox command. */

        put_qrp(qrp);           /* Release QRP. */
    }
    else
    {
        retValue = ISP_NLP;     /* This port is point-to-point, not loop. */
    }

    if (retValue != ISP_CMDC)   /* If the command failed */
    {
        /* The loop map is empty. */

        lpmap[port][0] = 0;
        lpmap[port][1] = 0xFF;
        lpmap[port][2] = 0xFF;
        lpmap[port][3] = 0xFF;
    }

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Issues the get data rate mailbox command to retrieve
**              the current data rate (1 Gb or 2 Gb).
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      none    - use if there are no params
**
**  @return     return status.
**
******************************************************************************
**/
UINT16 isp_getDataRate(UINT8 port)
{
    QRP        *qrp;
    UINT16      retValue;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_DR;      /* Execute Data Rate command */
    qrp->imbr[1] = 0;           /* Get data Rate */
    qrp->imbr[2] = 2;           /* Auto Negotiate */
    qrp->iRegs = 0x7;           /* set modify mailbox reg 0-2 */
    qrp->oRegs = 0x3;           /* set retrieve 0-1 mailbox regs */

    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];    /* Get return status of mailbox command. */

    if (retValue == ISP_CMDC)   /* Did the command complete successfully? */
    {
        /* Get data rate from outgoing mailbox register. */
        isprev[port]->dataRate = qrp->ombr[1] + 1;
    }
    else
    {
        /* Get completion status from outgoing mailbox register. */
        isprev[port]->dataRate = 0;
    }

    put_qrp(qrp);               /* Release QRP. */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Issues the Get Port/Node Name List mailbox command.
**
**  @param      port        - Port
**  @param      vpindex     - VP Index
**  @param      * buffer    - Pointer to destination buffer
**
**  @return     Number of devices
**
******************************************************************************
**/
UINT16 ISP_GetPortNodeNameList(UINT8 port, UINT8 vpindex, PNNL * buffer)
{
    QRP        *qrp;
    PNNL_2400  *gpnnl_2400;
    UINT32      i;
    union
    {
        PNNL       *ptr;
        UINT16      s[2];
    } pBuffer;
    UINT16      ndevs;

    /* Get the pointer to the destination buffer. */

    gpnnl_2400 = s_MallocC(sizeof(PNNL_2400) * MAX_DEV, __FILE__, __LINE__);
    pBuffer.ptr = (PNNL *)gpnnl_2400;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    /* Execute Get Port/Node Name List command */

    qrp->imbr[0] = ISP_GPNL;

    /*
     ** bit 0=0 - return port nanes
     ** bit 1=1 - return loop ID 0-FF
     ** bit 2=1 - return port data
     ** bit 3=1 - return SCSI initiators.
     */
    qrp->imbr[1] = 0xC;
    qrp->imbr[8] = 0xffff;
    qrp->oRegs = 0x7;           /* set retrieve 0-2 mailbox regs */
    qrp->iRegs = 0x3CF;         /* set modify mailbox reg 0-3, 6-9 */
    qrp->imbr[2] = pBuffer.s[1] + (UINT16)(K_poffset >> 16);
    qrp->imbr[3] = pBuffer.s[0];
    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;
    qrp->imbr[9] = vpindex;     /* MID virtual port */

    isp_exec_cmd(port, qrp, TRUE);

    /* Check if the command completed successfully. */

    if (qrp->ombr[0] == ISP_CMDC)
    {
        ndevs = qrp->ombr[1] / 0x28;    /* Get the number of devices. */
    }
    else
    {
        ndevs = 0;              /* Set no devices if command failed. */
    }

    for (i = 0; i < ndevs; i++)
    {
        buffer[i].ndn = gpnnl_2400[i].nodename;
        buffer[i].pdn = gpnnl_2400[i].portname;
        buffer[i].lid = gpnnl_2400[i].nporthandle & 0x7fff;     /* get rid of ip flag bit */
        buffer[i].opt = gpnnl_2400[i].flags & 0xff;
        /* no data for this */
        buffer[i].cnt = 0;
        buffer[i].mst = gpnnl_2400[i].lastloginstate;
        buffer[i].sst = gpnnl_2400[i].curloginstate;
        /* this maybe an issue 24 bits to 8 */
        buffer[i].had = gpnnl_2400[i].hardaddr & 0xff;
        buffer[i].portID = (UINT32)gpnnl_2400[i].portid[0] |
            (UINT32)(gpnnl_2400[i].portid[1] << 8) |
            (UINT32)(gpnnl_2400[i].portid[2] << 16);
        buffer[i].prliW0 = gpnnl_2400[i].prliW0;
        buffer[i].prliW3 = gpnnl_2400[i].prliW3;
    }

    put_qrp(qrp);               /* Release QRP. */

    /* Release temp buffer for 2400. */

    s_Free(gpnnl_2400, sizeof(PNNL_2400) * MAX_DEV, __FILE__, __LINE__);

    return ndevs;
}


/**
******************************************************************************
**
**  @brief      Issues the Get Firmwate State mailbox command.
**
**  @param      port        - Port
**
**  @return     Firmware state
**
******************************************************************************
**/
UINT16 ISP_GetFirmwareState(UINT8 port)
{
    QRP        *qrp;
    UINT16      firmwareState;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_GFWS;    /* Execute Get Firmware State command */
    qrp->iRegs = 0x1;           /* set modify mailbox reg 1      */
    qrp->oRegs = 0x3;           /* set retrieve 0-1 mailbox regs */

    isp_exec_cmd(port, qrp, TRUE);

    if (qrp->ombr[0] == ISP_CMDC)       /* Was the command successful? */
    {
        firmwareState = qrp->ombr[1];   /* Get the firmware state. */
    }
    else
    {
        firmwareState = 0xFFFF; /* Status is not available. */
    }

    put_qrp(qrp);               /* Release QRP. */

    return firmwareState;
}


/**
******************************************************************************
**
**  @brief      Issues the Get Link Statistics mailbox command.
**
**      This command requests the link status of the chip
**
**  @param      port        - Port
**  @param      * buffer    - Pointer to destination buffer
**
**  @return     return status.
**
******************************************************************************
**/
UINT16 ISP2400_GetLinkStatistics(UINT8 port, void *buffer)
{
    QRP        *qrp;
    union
    {
        void       *ptr;
        UINT16      s[2];
        UINT32      l;
    } pBuffer;
    UINT16      retValue;

// fprintf(stderr, "%s%s:%u port=%d isp2400=0x%02x isp2500=0x%02x\n", FEBEMESSAGE, __func__, __LINE__, port, isp2400, isp2500);
    if (!(BIT_TEST(isp2400, port) || BIT_TEST(isp2500, port)))
    {
        return ISP_CMDI;
    }

    /* Get the pointer to the destination buffer. */

    pBuffer.l = LI_GetPhysicalAddr((unsigned long)buffer);

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_GLSP;    /* Execute Get Link Statistics command */
    qrp->imbr[2] = pBuffer.s[1];
    qrp->imbr[3] = pBuffer.s[0];

    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;
    qrp->imbr[8] = 6;
    qrp->imbr[10] = 0;

    qrp->iRegs = 0x5CD;         /* Set modify mailbox regs 0, 2-3, 6-7, 10    */
    qrp->oRegs = 0x3;           /* Set retrieve mailbox reg 0               */

    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];    /* Get return status of mailbox command. */

    put_qrp(qrp);               /* Release QRP. */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Issues the Get Link Status mailbox command.
**
**      This command requests the link status from any loop port logged in
**      to the ISP23xx. This command also provides the link status of the chip
**      when the loop ID is that of the ISP23xx. The Get Link Status command
**      causes the ISP23xx firmware to issue an RLS ELS request to the
**      specified loop ID. The RLS response is then transferred to the
**      specified system buffer.
**
**  @param      port        - Port
**  @param      lid         - LID
**  @param      * buffer    - Pointer to destination buffer
**
**  @return     return status.
**
******************************************************************************
**/
UINT16 ISP_GetLinkStatus(UINT8 port, UINT32 lid, void *buffer)
{
    QRP        *qrp;
    union
    {
        void       *ptr;
        UINT16      s[2];
        UINT32      l;
    } pBuffer;
    UINT16      retValue;

    /* Get the pointer to the destination buffer. */

    pBuffer.l = LI_GetPhysicalAddr((unsigned long)buffer);

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_GLKS;    /* Execute Get Link Status command */
    qrp->imbr[1] = lid;         /* Loop ID (bits 15-0)                  */
    qrp->imbr[2] = pBuffer.s[1];
    qrp->imbr[3] = pBuffer.s[0];

    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;
    qrp->imbr[10] = 0;
    qrp->iRegs = 0x4DF;         /* set modify mailbox regs 0-3, 4, 6-7, 10 */
    qrp->oRegs = 0x3;           /* set retrieve mailbox reg 0           */
    qrp->imbr[4] = 0;           /*set port to 0 */

    isp_mailboxIOCB(port, lid, qrp);

    retValue = qrp->ombr[0];    /* Get return status of mailbox command. */

    put_qrp(qrp);               /* Release QRP. */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Setup and execute the Set Firmware Options mailbox command (38h)
**
**              This mailbox command allows the specification of additional
**              ISP firmware options beyond those contained in the
**              Initialization Control Block (ICB).
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     Completion status
**
******************************************************************************
**/
UINT16 isp_SetFirmwareOptions(UINT8 port)
{
    QRP        *qrp;
    UINT16      retValue;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_GFO;     /* Execute Get Firmware options */
    qrp->iRegs = 0x1;           /* set modify mailbox reg 0      */
    qrp->oRegs = 0xF;           /* set retrieve 0-3 mailbox regs */

    isp_exec_cmd_sri(port, qrp, TRUE);

    if (qrp->ombr[0] == ISP_CMDC)       /* Did the command complete successfully? */
    {
        /*
         ** Enable Loop Initialization Error (AE 8017)
         ** Enable LIP Immediately after loss of sync
         ** Enable Frame Dropped Error (AE 8048)
         */
        qrp->imbr[1] = qrp->ombr[1] | AE8017EN | LIPIMMED;
        qrp->imbr[2] = qrp->ombr[2];
        qrp->imbr[3] = qrp->ombr[3] | AE8048EN;

        qrp->imbr[0] = ISP_SFO; /* Execute Set Firmware options */
        qrp->iRegs = 0xF;       /* set modify mailbox reg 0-3     */
        qrp->oRegs = 0x1;       /* set retrieve 0 mailbox regs    */

        isp_exec_cmd_sri(port, qrp, TRUE);
    }

    if (qrp->ombr[0] == ISP_CMDC)       /* Was the command successful? */
    {
        retValue = 0;
    }
    else
    {
        /* Return set firmware options failed. */

        retValue = ISP_SET_FW_OPT_ERROR;
    }

    put_qrp(qrp);               /* Release QRP. */

    return retValue;
}


/**
******************************************************************************
**
**  @brief To provide a means of obtaining a pointer to an available entry
**         on the request queue for a particular ISP2x00 instance.
**
**         This routine returns a pointer in 'rqip' to an available IOCB
**         entry on the request queue for an ISP2x00 instance.
**
**         This routine may block if the request queue is full. It will awaken
**         when the queue has dropped to <MAXTHRESH> (see <isp2100.inc>)
**         If any tasks are on the waiting list when an IOCB is requested the
**         calling task will be blocked even if IOCBs are available. This
**         is to prevent one task from monopolizing the IOCB resource when
**         other tasks are waiting.
**
**         NOTE: This routine is intended for single-threaded usage for
**         any particular ISP instance. Undefined results may occur if
**         multiple tasks concurrently request IOCBs from the same ISP instance.
**
**         This problem can be obviated if the task does not relinquish the CPU
**         between obtaining an IOCB and submitting it to the ISP.
**
**         Register g0 is returned as the pointer to the IOCB on the request
**         queue; register g0 is the offset into the queue that is to be
**         written to the mailbox register when the IOCB is submitted.
**         The task is responsible for updating the ISP mailbox register
**         with this offset when the IOCB is to be submitted.
**
**  @param      port - QLogic chip instance (0-3)
**  @param      rqip - pointer to request queue IN ptr
**
**  @return     Completion status
**
******************************************************************************
**/

UINT16     *isp_get_iocb(UINT8 port, UINT32 *rqip)
{
    UINT32     *iocb;
    UINT32     *inPtr;
    UINT32     *outPtr;
    ISP_2400   *isp2400hwptr = (ISP_2400 *)ispstr[port]->baseAd;

    while (1)
    {
        /* Check for reset interlock set. If true, loop until it is cleared. */
        if (!BIT_TEST(isprena, port))
        {
            TaskSleepMS(125);
            continue;
        }

        if (BIT_TEST(ispfail, port))    /* If this port has failed */
        {
            return NULL;
        }

        if (BIT_TEST(isprqwt, port))
        {
            TaskSetMyState(PCB_QLOGIC_WAIT + port);
            TaskSwitch();
            continue;
        }

        /*
         ** Attempt assignment of IOCB
         ** Increment In Ptr
         */
        iocb = ispstr[port]->reqQue->in;
        inPtr = iocb + 16;

        /* Check for wrap. */

        if (inPtr >= ispstr[port]->reqQue->end)
        {
            inPtr = ispstr[port]->reqQue->begin;
        }

        /* Refresh current OUT ptr from hardware */

        outPtr = ispstr[port]->reqQue->begin + (isp2400hwptr->reqQOP * 16);

        ispstr[port]->reqQue->out = outPtr;

        if (inPtr == outPtr)    /* If queue full condition */
        {
            /* Request queue is full, set wait flag and retry assignment. */
            BIT_SET(isprqwt, port);
            continue;
        }

        if (outPtr > ispstr[port]->reqQue->end)
        {
            fprintf(stderr, "%s: reset chip %d due to invalid outPtr\n", __func__, port);
            ISP_ResetChip(port, ISP_RESP_Q_PTR_ERROR);
            continue;
        }

        ispstr[port]->reqQue->in = inPtr;       /* Store new IN pointer */

        /* Determine offset into queue */

        *rqip = (UINT32)inPtr - (UINT32)ispstr[port]->reqQue->begin;
        *rqip /= IOCB_SIZE;

        /* Assignment was sucessful. */
        break;
    }

    return (UINT16 *)iocb;
}


#ifdef FRONTEND

/**
******************************************************************************
**
**  @brief      Find a target record for the specified port.
**
**              This routine parses a linked list of targets to be configured
**              and finds the first target for the specified port.
**
**  @param      port    - Chip instance of QLogic ISP2x00 (0-3).
**
**  @return     Pointer to a target structure
**
******************************************************************************
**/
TGD        *isp_findTarget(UINT8 port)
{
    UINT32      i;
    TGD        *pTgd;
    TGD        *foundTgd = NULL;
    UINT32      cserial = K_ficb->cSerial;

    /* Checking for entries */

    for (i = 0; i < MAX_TARGETS; ++i)
    {
        pTgd = T_tgdindx[i];

        /*
         ** Check for correct owner and port number.
         ** Check preferred port.
         ** When not preferred port and preferred port is failed,
         ** check alternate port.
         */
        if (pTgd && pTgd->owner == cserial && (pTgd->prefPort == port ||
            (ispFailedPort[pTgd->prefPort] != FALSE && pTgd->altPort == port)))
        {
            foundTgd = pTgd;
            break;
        }
    }

    /* Return target found. When a target is not found, NULL is returned */

    return foundTgd;
}


/**
******************************************************************************
**
**  @brief      Determine if the target can be placed on the preferred port.
**
**              The status of the preferred port and the alternate
**              port are examined to determine if the target can
**              be placed on it preferred port.
**
**  @param      * pTgd  - Pointer to target structure.
**
**  @return     TRUE if preferred port can be used, FALSE otherwise.
**
******************************************************************************
**/
UINT32 isp_preferredPortCriteria(TGD * pTgd)
{
    /*
     ** The target is placed on the preferred port if
     ** the preferred port is not marked as failed and
     ** the preferred port exists and
     ** the preferred port initialized okay or
     ** the alternate port does not exist or
     ** the alternate port failed to initialize or
     ** the alternate port is marked as failed
     */
    return (ispFailedPort[pTgd->prefPort] == FALSE &&
            isprev[pTgd->prefPort] != NULL &&
            !BIT_TEST(ispfail, pTgd->prefPort)) ||
            isprev[pTgd->altPort] == NULL ||
            ispFailedPort[pTgd->altPort] != FALSE ||
            BIT_TEST(ispfail, pTgd->altPort);
}


/**
******************************************************************************
**
**  @brief      Determine if the target can be placed on the alternate port.
**
**              The status of the preferred port and the alternate
**              port are examined to determine if the target can
**              be placed on it preferred port.
**
**  @param      * pTgd  - Pointer to target structure.
**
**  @return     TRUE if preferred port can be used, FALSE otherwise.
**
******************************************************************************
**/
UINT32 isp_alternatePortCriteria(TGD * pTgd)
{
    /*
     ** The target is placed on the alternate port if
     ** the alternate port is not marked as failed and
     ** the alternate port exists and
     ** the alternate port initialized okay and
     ** the perferred port does not exist or
     ** the perferred port failed to initialize or
     ** the perferred port is marked as failed
     */
    return ispFailedPort[pTgd->altPort] == FALSE &&
        isprev[pTgd->altPort] != NULL &&
        !BIT_TEST(ispfail, pTgd->altPort) &&
        (isprev[pTgd->prefPort] == NULL ||
         ispFailedPort[pTgd->prefPort] != FALSE || BIT_TEST(ispfail, pTgd->prefPort));
}


/**
******************************************************************************
**
**  @brief      Reset the qlogic adapter.
**
**              The process resets the qlogic adapter and
**              decrement the count of processes when complete.
**
**  @param      a               - Not used
**  @param      b               - Not used
**  @param      port            - Port
**  @param      reasonCode      - Passed to ISP_ResetChip
**  @param      * resetCount    - Count of processes
**
**  @return     none
**
******************************************************************************
**/
void isp_resetProcess(UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port, UINT32 reasonCode, UINT32 *resetCount, UINT32 *vpMap)
{
    ispResetLog[port].timeStamp = timestamp;
    ispResetLog[port].reason = reasonCode;
    ispResetLog[port].vpMap = 0;

#ifdef DEBUG_FLT_REC_ISP
    MSC_FlightRec(0x1F, port, 0, vpMap ? vpMap[port] : 0);
#endif /* DEBUG_FLT_REC_ISP */

    ISP_ResetChip(port, reasonCode);    /* Reset the Qlogic adapter */

    --(*resetCount);            /* Decrement count of processes */

    if (*resetCount == 0)       /* Check for zero */
    {
        /* Indicate all processes have completed */

        TaskReadyByState(PCB_HOST_RESET_WAIT);
    }
}


/**
******************************************************************************
**
**  @brief      Fork process that will reset the Qlogic adpater.
**
**              A process if forked for each QLogic adapter to be
**              reset. A count of process is maintained and
**              wait for zero indicating all processes have completed.
**
**  @param      portMap     - MAXISP - Bit map of ports to reset.
**  @param      reasonCode  - Passed to ISP_ResetChip
**
**  @return     none
**
******************************************************************************
**/
void isp_startResetProcess(UINT32 portMap, UINT32 *vpMap, UINT32 reasonCode)
{
    UINT32      port;
    UINT32      resetCount = 0;

    if (portMap == 0)
    {
        return;
    }

    /* Examine all ports */

    for (port = 0; port < MAX_PORTS; ++port)
    {
        /* Does this port need to be reset */

        if (BIT_TEST(portMap, port))
        {
            ++resetCount;       /* Increment count of processes */

            CT_fork_tmp = (unsigned long)"isp_resetProcess";
            TaskCreate6(C_label_referenced_in_i960asm(isp_resetProcess),
                        0x14, port, reasonCode, (UINT32)&resetCount, (UINT32)vpMap);
            TaskSwitch();
        }
    }

    while (resetCount != 0)     /* Wait until all resets have completed */
    {
        /* Set process to wait for signal and Exchange process */

        TaskSetMyState(PCB_HOST_RESET_WAIT);
        TaskSwitch();
    }
}


/**
******************************************************************************
**
**  @brief      Reset any ports that have a pending target configuration.
**
**              First all ports are scan to find ports that have targets
**              loaded that are no longer configured. These are labelled
**              as 'from' ports since targets will be moving from these ports.
**
**              Second all targets are scanned to find targets that are
**              not currently loaded into the port. These are labelled
**              as 'to' ports since targets will be moving to these ports.
**
**              Finally, the 'from' ports are reset followed by resetting
**              the 'to' ports.
**
**  @param      reinit  - TRUE to initialize the 'from' ports
**
**  @return     none
**
******************************************************************************
**/
void ISP_FindPortToReset(UINT32 reinit)
{
    UINT32      port;
    UINT32      target;
    TAR        *pTar;
    TGD        *tgd;
    UINT32      fromPort = 0;   /* MAXISP - bitmap */
    UINT32      toPort = 0;     /* MAXISP - bitmap */

    /* Invalidate all port assignment. Current assignment map is refreshed. */
    for (target = 0; target < MAX_TARGETS; ++target)
    {
        if (iclPortExists && (ICL_TARGET(target)))
        {
            continue;
        }
        ispPortAssignment[target] = 0xFF;
    }

    /* Traverse the target list for this port */
    for (port = 0; port < MAX_PORTS; ++port)
    {
        /* Check if port exists */
        if (isprev[port] == NULL)
        {
            continue;
        }

        /* Traverse the target list for this port */
        for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
        {
            /*
             * Check for a valid target number. Note that
             * control ports use target number of 0xFFFF.
             */
            if (pTar->tid < MAX_TARGETS && BIT_TEST(pTar->opt, TARGET_ENABLE))
            {
                tgd = T_tgdindx[pTar->tid];     /* Point to the target record */

                /*
                 * Check if the target associated with this tar record
                 * is owned by another controller.
                 * Check if the target is currently on a failed port.
                 *
                 * A reset is not performed if the alternate port
                 * does not exist or has failed to initialize.
                 * Check if the target is currently on the alternate port.
                 *
                 * A reset is not performed if the preferred port
                 * does not exist or is marked as failed.
                 */
                if (tgd &&
                    (tgd->owner != K_ficb->cSerial ||
                     (tgd->prefPort == port && isp_alternatePortCriteria(tgd)) ||
                     (tgd->altPort == port && isp_preferredPortCriteria(tgd))))
                {
                    /*
                     * The target belongs on another controller,
                     * this port need to be reset.
                     * The target is being moved from the this port
                     * to the alternate port. This port need to be reset.
                     * The target is being moved from the this port
                     * to the preferred port. This port need to be reset.
                     */
                    BIT_SET(fromPort, port);

                    /* Invalidate port assignment */
                    ispPortAssignment[pTar->tid] = 0xFF;
                }
                else
                {
                    /* Update the port assignment for this target */
                    ispPortAssignment[pTar->tid] = port;
                }
            }
        }
    }

    /*
     * Examine all targets and for each target check if associated
     * fibre channel adapter is configured for that target.
     */
    for (target = 0; target < MAX_TARGETS; ++target)
    {
        if (iclPortExists && ICL_TARGET(target))
        {
            continue;
        }

        tgd = T_tgdindx[target];        /* Point to the target record */

        /* Does this target record exist and does it belong on this controller. */
        if (tgd == NULL || tgd->owner != K_ficb->cSerial)
        {
            continue;
        }

        /* Get the primary port number the target is currently configured for. */
        port = tgd->prefPort;

        /* Check if the primary port is missing or failed */
        if (isprev[port] == NULL || isp_preferredPortCriteria(tgd) == FALSE)
        {
            /*
             * Get the alternate port number the target
             * is currently configured for.
             */
            port = tgd->altPort;

            /* Check if the alternate port is missing or failed */
            if (isprev[port] == NULL || isp_alternatePortCriteria(tgd) == FALSE)
            {
                /* No non-failed ports exist for this target */
                continue;
            }
        }

        /* Does the current port match the desired port? */
        if (port < MAX_PORTS && ispPortAssignment[target] != port)
        {
            /* The target belongs on this port, this port need to be reset. */
            BIT_SET(toPort, port);
        }
    }

    /* Reset (but don't initialize) the 'from' ports, if any */
    if (fromPort != 0)
    {
        isp_startResetProcess(fromPort, NULL, ISP_RESET_ONLY);
    }

    /* Reset the 'to' ports, if any */
    if (toPort != 0)
    {
        isp_startResetProcess(toPort, NULL, ISP_RESET_AND_INIT);
    }

    /* Initialize the 'from' ports, if any */
    if (fromPort != 0 && reinit != FALSE)
    {
        isp_startResetProcess(fromPort, NULL, ISP_RESET_AND_INIT);
    }
}


/**
******************************************************************************
**
**  @brief      Initialize the Tar structure from the Tgd configuration record.
**
**              Memory is allocated for a Tar strucrture if needed.
**              The Tar structure is initialized taking into account
**              clustered targets.
**
**  @param      * TAR* - Pointer to Tar structure.
**  @param      * TGD* - Pointer to Tgd configuration record.
**  @param      UINT8  - Previously-assigned Loop ID (or NO_CONNECT)
**
**  @return     none
**
******************************************************************************
**/
void isp_newTarget(TAR * pTar, TGD * pTgd, UINT8 prevID)
{
    /* Check if the target is clustered to another target */

    if (pTgd->cluster < MAX_TARGETS && T_tgdindx[pTgd->cluster] != NULL)
    {
        /*
         ** Target is clustered.
         ** Obtain the node WWN from the clustered target.
         */
#if defined(MODEL_7000) || defined(MODEL_4700)
        if (BIT_TEST(iscsimap, pTgd->port))
        {
            pTar->nodeName = pTgd->nodeName;
        }
        else
#endif /* MODEL_7000 || MODEL_4700 */
        {
            pTar->nodeName = T_tgdindx[pTgd->cluster]->nodeName;
        }
#if ICL_DEBUG
        if (pTgd->port == ICL_PORT)
        {
            fprintf(stderr, "<%s:%s>ICL port...clustered to another target()\n",
                    __FILE__, __func__);
        }
#endif /* ICL_DEBUG */

    }
    else
    {
        /*
         ** Target is not clustered.
         ** Obtain the node WWN from the target configuration record.
         */
#if defined(MODEL_3000) || defined(MODEL_7400)
        if (BIT_TEST(iscsimap, pTgd->port))
        {
            if (T_tgdindx[pTgd->tid | 0x02])
            {
                pTar->nodeName = T_tgdindx[pTgd->tid | 0x02]->nodeName;
            }
            else
            {
                fprintf(stderr, "%s%s:%u pTgd->tid=%d, but T_tgdindx[%d] pointer is NULL\n",
                        FEBEMESSAGE, __func__, __LINE__, pTgd->tid, pTgd->tid | 0x02);
                pTar->nodeName = pTgd->nodeName;
            }
        }
        else
#endif /* defined(MODEL_3000) || defined(MODEL_7400) */
        {
            pTar->nodeName = pTgd->nodeName;
        }
#if ICL_DEBUG
        if (pTgd->port == ICL_PORT)
        {
            fprintf(stderr, "<%s:%s>ICL port...Not clustered()\n", __FILE__, __func__);
        }
#endif /* ICL_DEBUG */

    }

    /*
     ** Obtain the port WWN from the target configuration record.
     ** In fact copies the ipAddress and ipGW into pTar from pTGD
     */
    pTar->portName = pTgd->portName;

#if FE_ISCSI_CODE
    pTar->ipPrefix = pTgd->ipPrefix;
#endif /* FE_ISCSI_CODE */

    /* Initialize the remaining fields of the tar structure */

    pTar->tid = pTgd->tid;      /* target ID                    */
    pTar->opt = pTgd->opt;      /* Options                      */
    pTar->vpID = NO_CONNECT;    /* Virtual Port ID (LID)        */
    pTar->portID = NO_PORTID;   /* Port ID                      */

    /*
     ** If a Hard ID is NOT specified, AND a valid Prev LID is specified, then
     ** set the Previously Assigned ID flag to use the LID during the LIPA phase.
     */
    if (!BIT_TEST(pTar->opt, TARGET_HARD_ID) && prevID != 0xFF)
    {
        BIT_SET(pTar->opt, TARGET_PREV_ID);
        pTar->hardID = prevID;
    }
    else
    {
        /*
         ** Hard Loop ID specified, or no previously-assigned Loop ID specified
         */
        pTar->hardID = pTgd->fcid;
    }

    pTar->flags = FALSE;        /* Clear flag so this port registers its FC4 type */
}


/**
******************************************************************************
**
**  @brief      Generate a list of targets to be configured for a interface.
**
**  This routine is passed the chip ordinal. It parses the list of configured
**  targets for a single or redundant configuration and generates a linked
**  list (see <target.inc>) of targets to configure for the given interface.
**
**  Note that target 0 is generated independently from the configured target
**  list. This target is generated from the serial number of the controller
**  and the interface (chip instance).
**
**  @param      port    - Chip instance of QLogic ISP24xx. (0 to 3)
**
**  @return     None.
**
** NOTE: This routine does not normally task switch -- Malloc's might, but
**       only for adding a new entry -- which should be OK.
**
******************************************************************************
**/

void isp_build_targets(UINT8 port)
{
    TAR        *pTar;
    TAR        *fthd;
    TGD        *pTgd;
    TGD        *firstTgd = NULL;
    int         i;
    UINT32      cserial = K_ficb->cSerial;
    UINT8       prevID[MAX_TARGETS];
    TAR        *pTempTar;

#if FE_ICL
    if (ICL_IsIclPort(port))
    {
        fprintf(stderr, "%s: path PORT %d\n", __func__, port);
        return;
    }
#endif /* FE_ICL */

    pTar = tar[port];           /* Get pointer to tar linked-list for this channel */

    /*
     * Initialize the list of previously-assigned Loop IDs, one WORD at a time.
     * NOTE:  This assumes MAX_TARGETS is a multiple of 4.
     */
    for (i = 0; i < MAX_TARGETS / 4; ++i)
    {
        ((UINT32 *)prevID)[i] = 0xFFFFFFFF;
    }

    /*
     * Allocate an element for the target, if necessary, and store anchor ptr
     * to the target structure. The first element is permanently allocated.
     */
    if (pTar == NULL)
    {
        tar[port] = pTar = s_MallocC(sizeof(struct TAR) | BIT31, __FILE__, __LINE__);
    }
    else
    {
        /*
         * Walk the tar list and record the previously-assigned Loop IDs,
         * for re-use if these targets still reside on this port.
         */
        pTempTar = pTar;
        do
        {
            if (pTempTar->tid < MAX_TARGETS)
            {
                prevID[pTempTar->tid] = pTempTar->vpID;
            }
            pTempTar = pTempTar->fthd;
        } while (pTempTar != NULL);
    }

    /*
     * If the CCB has said it is OK to put the regular targets on the FE, then
     * go ahead and look for valid target assignments.
     * Else only put on a Control Port.
     */
#ifdef  WC_ENABLE
    if (gRegTargetsOK == TRUE || BIT_TEST(iscsimap, port))
#endif /* WC_ENABLE */
    {
        /*
         * Examine each target record checking for one which prefers to be
         * located on this channel. If found, this preferred target is
         * configured first so it becomes the primary port (i.e. initiator
         * and target capability).
         */
        for (i = 0; i < MAX_TARGETS; ++i)
        {
            pTgd = T_tgdindx[i];

            /* Check for a matching preferred channel and owner */
            if (pTgd != NULL &&
                pTgd->prefPort == port &&
                isp_preferredPortCriteria(pTgd) &&
                pTgd->prefOwner == cserial &&
                pTgd->owner == cserial)
            {
                isp_newTarget(pTar, pTgd, prevID[pTgd->tid]);

                /* Set the current port assignment for this target ID */
                ispPortAssignment[pTgd->tid] = port;

                /* This is the first matching target record. Break off search. */
                firstTgd = pTgd;
                break;
            }                   /* Matching preferred channel and owner */
        }

        /*
         * Examine each target record checking for matching target record.
         * Any additional ports configured on this adapter are secondary ports
         * and do not have initiator capability.
         */
        for (i = 0; i < MAX_TARGETS; ++i)
        {
            pTgd = T_tgdindx[i];

            /*
             * If a target record exists at this index, check if the channel and
             * owner are for this adapter. Also check for alternate port.
             * To use the alt port, the primary port must be failed or missing.
             * If the alt port is missing, then a target whose preferred owner
             * is not this controller is allowed to use this port.
             */
            if (pTgd != NULL &&
                pTgd != firstTgd &&
                pTgd->owner == cserial &&
                ((pTgd->prefPort == port && isp_preferredPortCriteria(pTgd)) ||
                 (pTgd->altPort == port && isp_alternatePortCriteria(pTgd))))
            {
                /* Check if this is the first target for this adapter */
                if (firstTgd != NULL)
                {
                    /* Check for an existing target structure to use */
                    if (pTar->fthd == NULL)
                    {
                        /* Allocate element for target */
                        pTar->fthd = s_MallocC(sizeof(struct TAR), __FILE__, __LINE__);
                    }

                    /* The entry number increments for each element in the list. */
                    pTar->fthd->entry = pTar->entry + 1;
                    pTar = pTar->fthd;  /* Go to the next target in the list */
                }
                else            /* first target */
                {
                    firstTgd = pTgd;    /* The first matching target record */
                }

                /* Initialize the target structure */
                isp_newTarget(pTar, pTgd, prevID[pTgd->tid]);

                /* Set the current port assignment */
                ispPortAssignment[pTgd->tid] = port;
            }                   /* match found */
        }
    }

    if (firstTgd == NULL)
    {
        /*
         * No targets exist for this port, so set up a control port.
         * Control ports do not have target IDs.
         */
        pTar->tid = NO_TID;     /* Invalid target ID            */
        pTar->entry = 0;        /* Control port is entry 0      */

        /*
         * Setup node and port name.
         *
         * Port ID: 216cOOOOOOFZssss  where O = OUI s = serial number F = family
         * Node ID: 206cOOOOOOFZssss  c = channel number and Z = zero
         *
         * Create the world wide names for the target.
         */
        pTar->portName = ((UINT64)XIO_OUI << 24) | (cserial & 0xFFFFFF);
        pTar->portName |= (UINT64)(WWN_C_PORT | (port << 16)) << 32;
        pTar->portName = bswap_64(pTar->portName);
        pTar->nodeName = (pTar->portName & 0xFFFFFFFF00000000ULL) | bswap_32(WWN_C_NODE);

        /* Setup options for target 0 */
        pTar->opt = (1 << TARGET_ENABLE);       /* Enable target                */
        pTar->vpID = NO_CONNECT;
        pTar->hardID = 0;
        pTar->portID = NO_PORTID;

        BIT_SET(ispCp, port);   /* Indicate this port is a control port */
    }
    else
    {
        /*
         * Indicate this port is not a control port.
         * Also clear the control port delay flag.
         */
        BIT_CLEAR(ispCp, port);
    }

#if FE_ISCSI_CODE
    if (BIT_TEST(iscsimap, port))
    {
        pTar->portID = 0x0;             /* Port ID */
        pTar->ssn_cnt = 0;              /* iscsi session count */
        isprev[port]->type = iSCSI;     /* "iSCSI" firmware */
    }
#endif /* FE_ISCSI_CODE */

    /* Check for any additional (unused) targets on the linked list */
    if (pTar->fthd)
    {
        /* NOTE: The tar[] list is no longer valid, if another task has task-switched. */
        tar_link_abort[port]++;         /* Mark that the list has truncated. */
        fthd = pTar->fthd;

        /* Unlink these targets from the list by clearing the forward ptr */
        pTar->fthd = NULL;

        /* Walk the remaining target list and release them */
        do
        {
            pTar = fthd;
            fthd = pTar->fthd;
            s_Free(pTar, sizeof(struct TAR), __FILE__, __LINE__);
        } while (fthd != NULL);
    }
}   /* End of isp_build_targets */

#endif /* FRONTEND */


void isp_MailboxTimeoutCheck(UINT32 port)
{
    QRP        *head;           /* QRP at head of queue                 */
    UINT32      i;              /* loop counter                         */

    head = ispdefq[port * 2];   /* Get QRP at the head of the queue */

    if (!head)                  /* If no mailbox command */
    {
        return;
    }

    /*
     ** Check if mailbox command has timed out and is
     ** still at the head of the queue. Note that
     ** the interrupt code changes the QRP stored
     ** at the head of the queue.
     */
    if (head->timeout <= timestamp && head == ispdefq[port * 2])
    {
#ifdef STEVE_DEBUG
        logFailMailbox(head);
#endif /* STEVE_DEBUG */

        head->ombr[0] = ISP_MBTO;       /* Set "timeout" return code in QRP */

        /* When mailbox command times out, send debug message to CCB */

        LOG_MB_FAILED_PKT embf;

        embf.header.event = LOG_MB_FAILED;
        embf.data.port = port;
#ifdef FRONTEND
        embf.data.proc = 0;
#else  /* FRONTEND */
        embf.data.proc = 1;
#endif /* FRONTEND */
        embf.data.iregs = head->iRegs;
        embf.data.oregs = head->oRegs;

        for (i = 0; i < dimension_of(embf.data.imbr); ++i)
        {
            embf.data.imbr[i] = head->imbr[i];
            embf.data.ombr[i] = ISP_MBTO;
        }

        /* Note: message is short, and L$send_packet copies into the MRP */

        MSC_LogMessageStack(&embf, sizeof(LOG_MB_FAILED_PKT));

        /* Mailbox Command timed out, reset chip */

        ISP_DumpQL(port, ISP_MAILBOX_TIMEOUT);
    }
}


/**
******************************************************************************
**
**  @brief  Monitor firmware heartbeat
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
NORETURN void isp_monitorFwhb(void)
{
    UINT8       port;           /* Port Number                          */
//    struct fwhb_t (*pFwhb)[MAX_PORTS];  /* Heart beat pointer       */
    UINT32      i;              /* loop counter                         */
    QRP        *head;           /* QRP at head of queue                 */
    struct ev_t *entry;
    UINT32      maxPorts = ispmax;

    /* Allocate non-cacheable memory for heart beat structure */

    ispfwhb = s_MallocC(sizeof(*ispfwhb), __FILE__, __LINE__);

//    pFwhb = ispfwhb;            /* Set local firmware heart beat pointer */

    while (TRUE)                /* Do forever */
    {
#if FE_ISCSI_CODE
        if (iclPortExists)
        {
            maxPorts = ispmax + 1;
        }
#endif /* FE_ISCSI_CODE */
        /* Scan all possible ports */

        for (port = 0; port < maxPorts; ++port)
        {
#if FE_ISCSI_CODE
            if (BIT_TEST(ispmap, port) && !BIT_TEST(iscsimap, port) && !ICL_PRT(port))
#else  /* FE_ISCSI_CODE */
            if (BIT_TEST(ispmap, port))
#endif /* FE_ISCSI_CODE */
            {
                /* Check for stalled ILTs for this instance */

                isp_check_thread(port);

                /* Get QRP at the head of the queue */

                head = ispdefq[port * 2];

                if (head)       /* If a mailbox command */
                {
                    /*
                     ** Check if mailbox command has timed out and is
                     ** still at the head of the queue. Note that
                     ** the interrupt code changes the QRP stored
                     ** at the head of the queue.
                     */
                    if (head->timeout <= timestamp && head == ispdefq[port * 2])
                    {
#ifdef STEVE_DEBUG
                        logFailMailbox(head);
#endif /* STEVE_DEBUG */
                        /* Set "timeout" return code in QRP */

                        head->ombr[0] = ISP_MBTO;

                        /*
                         ** When mailbox command times out, send debug message
                         ** to CCB.
                         */
                        LOG_MB_FAILED_PKT embf;

                        embf.header.event = LOG_MB_FAILED;
                        embf.data.port = port;
#ifdef FRONTEND
                        embf.data.proc = 0;
#else  /* FRONTEND */
                        embf.data.proc = 1;
#endif /* FRONTEND */
                        embf.data.iregs = head->iRegs;
                        embf.data.oregs = head->oRegs;

                        for (i = 0; i < dimension_of(embf.data.imbr); ++i)
                        {
                            embf.data.imbr[i] = head->imbr[i];
                            embf.data.ombr[i] = ISP_MBTO;
                        }

                        /*
                         * Note: message is short, and L$send_packet copies into the MRP.
                         */
                        MSC_LogMessageStack(&embf, sizeof(LOG_MB_FAILED_PKT));

                        /* Mailbox Command timed out, reset chip */

                        ISP_DumpQL(port, ISP_MAILBOX_TIMEOUT);
                    }
                }

                if ((timestamp & 3) != 0)       /* Check once every 4 seconds */
                {
                    continue;
                }

                if (BIT_TEST(isprena, port))    /* If port is initialized */
                {
                    /*
                     ** Check that the heart beats are being updated.
                     ** Heart beats are updated once a second.
                     */
                    if ((*ispfwhb)[port].previous != (*ispfwhb)[port].current ||
                        ispInterrupt[port] != 0)
                    {
                        /* Store the last firmware heart beat */

                        (*ispfwhb)[port].previous = (*ispfwhb)[port].current;

                        /*
                         ** Clear the flag that indicates an interrupt occurred.
                         */
                        ispInterrupt[port] = 0;
                    }
                    else
                    {
                        /* Don't check heart beat when it is not enabled */

                        if ((*ispfwhb)[port].enabled == FALSE)
                        {
                            continue;
                        }

                        /* Heart beat has not changed, reset chip */

                        ISP_DumpQL(port, ISP_FW_HBEAT_FAIL);
                    }
                }

                /* Check for outstanding asychronous event */

                if (asyqa[port] != 0 && asyqa[port]->out != asyqa[port]->in)
                {
                    if (asyqa[port]->in == asyqa[port]->begin)
                    {
                        entry = (struct ev_t *)asyqa[port]->end - 1;
                    }
                    else
                    {
                        entry = (struct ev_t *)asyqa[port]->in - 1;
                    }

                    /*
                     ** Check if this entry is not the current one being
                     ** processed by the monitor async task (out pointer is
                     ** not pointing to this entry) and this is system error.
                     */
                    if (entry != (struct ev_t *)asyqa[port]->out &&
                        entry->event == ISP_ASPSYE)
                    {
                        /* System Error detected, reset chip */

                        ISP_DumpQL(port, ISP_ASPSYE);
                    }
                }
            }
        }

        TaskSleepMS(1000);      /* Wait for one second and then check again */

        ++timestamp;            /* Increment timestamp */
    }
}


/**
******************************************************************************
**
**  @brief  Test the mailbox registers.
**
**  @param  port - QLogic chip instance (0-3)
**  @param  qrp - Pointer to QRP structure
**
**  @return 0=GOOD, all else is an error.
**
******************************************************************************
**/
static UINT16 ISP_MailboxTest(UINT8 port, QRP * qrp)
{
    int         i;

    qrp->imbr[0] = ISP_MBXT;    /* Mailbox Test command         */

    /* Issue test mailbox command */
    qrp->iRegs = 0xFFFFFFFF;    /* Set modify all registers (0-31)   */
    qrp->oRegs = 0xFFFFFFFF;    /* Set retrieve all registers (0-31) */

    isp_exec_cmd_sri(port, qrp, TRUE);

    if (qrp->ombr[0] != ISP_CMDC)       /* Check for a BAD completion */

    {
        return ISP_MBOX_TEST_FAIL;      /* Mailbox Test Command failed */
    }

    /* Verify input and output registers match */

    for (i = 1; i < 32; ++i)
    {
        if (qrp->ombr[i] != qrp->imbr[i])
        {
            return ISP_MBOX_TEST_FAIL;  /* A mismatch was detected */
        }
    }

    return GOOD;
}


/***
******************************************************************************
**
**  @brief  Test interface to Qlogic card.
**
**  @param  port - QLogic chip instance (0-3)
**  @param  qrp - pointer to qrp to use for commands
**
**  @return 0 = GOOD, otherwise error
**
******************************************************************************
**/
static UINT16 isp_test_interface(UINT8 port, QRP * qrp)
{
    int         i;
    UINT16      retValue;

    /* Test interface alive (no-op command) */

    qrp->imbr[0] = ISP_NOOP;
    qrp->iRegs = 0x1;           /* Set modify mailbox reg 0     */
    qrp->oRegs = 0x1;           /* Set retrieve mailbox reg 0   */

    isp_exec_cmd_sri(port, qrp, TRUE);
    if (qrp->ombr[0] != ISP_CMDC)
    {
        return ISP_IF_DEAD;     /* No-op command failed */
    }

    /* Test mailbox interface, write test pattern */

    for (i = 1; i < 16; ++i)
    {
        qrp->imbr[i] = 0x0101 << (i - 1);
    }

    for (i = 16; i < 32; ++i)
    {
        qrp->imbr[i] = 0xFFFF << (i - 16);
    }

    /* Issue test mailbox command */

    retValue = ISP_MailboxTest(port, qrp);
    if (retValue != GOOD)
    {
        return retValue;
    }

    /* Write all ones */

    for (i = 1; i < 32; ++i)
    {
        qrp->imbr[i] = 0xFFFF;
    }

    /* Issue test mailbox command */

    retValue = ISP_MailboxTest(port, qrp);
    if (retValue != GOOD)
    {
        return retValue;
    }

    /* Write all zeros */

    for (i = 1; i < 32; ++i)
    {
        qrp->imbr[i] = 0;
    }

    /* Issue test mailbox command */

    retValue = ISP_MailboxTest(port, qrp);

    return retValue;
}


/**
******************************************************************************
**
**  @brief  Load Qlogic firmware to Qlogic card.
**
**  @param  port    - QLogic chip instance (0-3)
**  @param  fwType  - Firmware type
**
**  @return 0=GOOD, all else is an error.
**
******************************************************************************
**/
UINT16 isp_loadQFW(UINT8 port, UINT16 fwType UNUSED)
{
    QRP        *qrp;
    UINT16      retValue;

    onlineTimer[port] = 10;     /* Initialize the online timer to 10 seconds */

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    /* Test QLogic interface */

    retValue = isp_test_interface(port, qrp);
    if (retValue != GOOD)
    {
        put_qrp(qrp);           /* Release QRP */
        fprintf(stderr, "%s: port=%d, Test Interface failed, possible HW failure\n",
                __func__, port);
        return retValue;
    }

    retValue = isp2400_loadQFW(port, qrp);

    put_qrp(qrp);               /* Release QRP */

#ifdef FRONTEND
    /*
     * Build the list of targets that will be used to
     * configure this instance of the Qlogic chip.
     */
    isp_build_targets(port);

    {
        TAR        *pTar;

        for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
        {
            if (BIT_TEST(pTar->opt, TARGET_ISCSI))
            {
                return ISP_FATAL_ERROR;
            }
        }
    }

    /* Init servdb owner table not sure where else I can do it */
    {
        UINT32      i;

        for (i = 0; i < MAX_DEV; ++i)
        {
            serverdbOwnerbyVpid[port][i] = 0xFF;
        }
    }
#endif /* FRONTEND */

    if (retValue == GOOD)
    {
        /* Wait if isp_loopEventHandler process is being created */

        while (isplepcb[port] == (PCB *)- 1)
        {
            TaskSleepMS(50);
        }

        /*
         ** Fork process to waits for the loop to
         ** come online following initialization.
         */
        if (isplepcb[port] == NULL)
        {
            CT_fork_tmp = (unsigned long)"isp_loopEventHandler";
            isplepcb[port] = (PCB *)- 1;        // Flag task being created.
            isplepcb[port] = TaskCreate3(C_label_referenced_in_i960asm(isp_loopEventHandler),
                                         K_xpcb->pc_pri + 1, port);

            /* Context switch to allow this process to start running */

            TaskSwitch();
        }
    }

    fprintf(stderr, "%s: returning %u for port %d\n", __func__, retValue, port);

    return retValue;
}


/**
******************************************************************************
**
**  @brief  Wait wait for a specified number of nanoseconds
**
**  @param  nanos - number of nanoseconds to wait
**
**  @return     none
**
******************************************************************************
**/

static void nanodelay(UINT32 nanos)
{
    UINT64      start = get_tsc();
    UINT64      now;
    UINT32      ticks;

    ticks = (ct_cpu_speed * nanos) / 1000;

    do
    {
        now = get_tsc();
    } while ((now - start) < ticks);
}


/**
******************************************************************************
**
**  @brief  Wait for a specified number of microseconds
**
**  @param  us - microseconds to wait
**
**  @return none
**
******************************************************************************
**/
void i_wait(UINT32 us)
{
    nanodelay(1000 * us);
}


/**
******************************************************************************
**
**  @brief      Issue the mailbox command to force system error (8002)
**
**              This is a debug/development function which will force the ISP
**              to generate a AEN 0x8002 (system error), to which this driver
**              reacts by forcing a QLogic dump.
**
**  @param      port    - Chip instance of QLogic ISP2x00 (0-3).
**
**  @return     rc      - ISP_CMDC if successful, OMB0 otherwise
**
******************************************************************************
**/
UINT16 ISP_ForceSystemError(UINT8 port)
{
    QRP        *qrp;
    UINT16      rc;

#ifdef ISP_DEBUG_FWSTATE
    /* Get firmware state, wait until "waiting for login" (2) or "ready" (3) */

    do
    {
        gFirmwareState = ISP_GetFirmwareState(port);
    } while (gFirmwareState != 2 && gFirmwareState != 3);
    fprintf(stderr, "%s: Firmware state = 0x%04X, delay 2s then force system error...\n",
            __func__, gFirmwareState);

    /* Wait about 2 seconds */

    TaskSleepMS(2000);
#endif /* ISP_DEBUG_FWSTATE */

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_FSE;
    qrp->iRegs = 0x1;           /* Set modify mailbox reg 0      */
    qrp->oRegs = 0x1;           /* Set retrieve mailbox reg 0    */

    /* Execute Force System Error mailbox command */

    isp_exec_cmd_sri(port, qrp, TRUE);

    /*
     ** Preserve command status & release QRP. 0x8002 is the expected status,
     ** or maybe our contrived 0x4097 value also. Change these to success.
     ** NOTE:  If the system faults due to the 0x8002 AEN (e.g. debug code),
     **        then we may never reach this point.
     */
    rc = qrp->ombr[0];
    if (rc == ISP_ASPSYE || rc == ISP_RSTD)
    {
        rc = ISP_CMDC;
    }

    put_qrp(qrp);

    return rc;
}


/**
******************************************************************************
**
**  @brief      Soft Reset the ISP chip
**
**              Perform a soft reset of the ISP chip via the control/status reg.
**              Retry/timeout the attempt if unsuccessful.
**
**  @param      port    - Chip instance of QLogic ISP2x00 (0-3).
**
**  @return     UINT16  - c/s register content, masked for bit 0 (0x0001)
**
******************************************************************************
**/
UINT16 isp_ChipSoftReset(UINT32 port)
{
    ISP_2400   *pISP;
    UINT32      readRetry;
    UINT32      resetRetry;
    UINT16      csMask;

    /* Set pointer to PCI offset of QLogic ISP adapter */

    pISP = ispstr[port]->baseAd;

    /*
     ** Outer Loop - perform a soft reset of the ISP chip, retrying the reset
     **              if the hardware doesn't respond as expected.
     */
    resetRetry = 3;
    do
    {
        /* Perform a soft reset of the ISP2x00 */

        pISP->cntl = 1 << ISP_CSR;
        FORCE_WRITE_BARRIER;

        /*
         ** Read the ISP control/status register (after a short delay),
         ** checking for the soft reset bit to be cleared by the hardware.
         */
        readRetry = 31;
        do
        {
            i_wait(31);         /* Short delay prior to reading control/status reg */
            csMask = pISP->cntl & (1 << ISP_CSR);
        } while (csMask != 0 && --readRetry > 0);
    } while (csMask != 0 && --resetRetry > 0);

    return csMask;
}


/**
******************************************************************************
**
**  @brief      Pause the ISP RISC processor
**
**              Pause the ISP RISC processor via the HCCR register.
**              Retry/timeout the attempt if unsuccessful.
**
**  @param      port    - Chip instance of QLogic ISP2x00 (0-3).
**
**  @return     UINT16  - hccr register content, masked for bit 5 (0x0020)
**
******************************************************************************
**/
UINT16 isp_PauseRISC(UINT32 port)
{
    ISP_2400   *pISP_2400;
    UINT32      readRetry;
    UINT32      pauseRetry;
    UINT16      hccrMask;

    /* Set pointer to PCI offset of QLogic ISP adapter */

    pISP_2400 = ispstr[port]->baseAd;

    /*
     ** Outer Loop - perform a Pause RISC processor on the ISP chip,
     **              retrying the pause if the HW doesn't respond as expected.
     */
    pauseRetry = 3;
    do
    {
        /* Pause the RISC processor */

        pISP_2400->hccr = ISP2400HCCR_SET_RISC_PAUSE;
        FORCE_WRITE_BARRIER;

        /*
         ** Read the ISP Host Command & Control register (after a short delay)
         ** checking for the Paused bit to be set by the hardware.
         */
        readRetry = 31;
        do
        {
            i_wait(31);         /* Short delay prior to reading host cmd/control reg */
            hccrMask = pISP_2400->hccr & ISP2400HCCRS_RISC_PAUSE;
        } while (hccrMask == 0 && --readRetry > 0);
    } while (hccrMask == 0 && --pauseRetry > 0);

    return hccrMask;
}


/**
******************************************************************************
**
**  @brief      Soft reset the ISP2x00 adapter
**
**              Perform a soft reset of the ISP23xx using the procedure
**              outlined in the 2300 Series Firmware Interface Specification.
**
**  @param      UINT32  - port - Chip instance of QLogic ISP2x00 (0-3).
**  @param      UINT32  - checkResilk - boolean, honor or ignore resilk state
**
**  @return     UINT32  - 0 = successfully reset
**                        n = failed - error code
**
******************************************************************************
**/
UINT32 isp_softReset(UINT32 port, UINT32 checkResilk)
{
    TAR        *pTar;
    UINT32      returnCode = 0;

    /* Disable checking the firmware heartbeats */

    (*ispfwhb)[port].enabled = FALSE;

    /* If specified by input parameter, check for a reset already in progress */

    if (checkResilk && BIT_TEST(resilk, port))
    {
        return returnCode;
    }

    /*
     ** Set the reset interlock to prevent overlapped resets.
     ** Clear enable bit for this port to disallow IOCB and QRP processing.
     */
    BIT_SET(resilk, port);
    BIT_CLEAR(isprena, port);

    /* Disable the ISP2xxx as a PCI bus master */

    if (ISP2400_SoftReset(port) != 0)
    {
        fprintf(stderr, "2400 soft reset failure\n");
        returnCode = 1;
        goto exit;
    }

exit:
    /* Traverse the target list for this port */

    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        /*
         * Check for a valid target number. Note that
         * control ports use target number of 0xFFFF.
         */
        if (pTar->tid < MAX_TARGETS)
        {
            /* Invalidate the port assignment for this target */
            ispPortAssignment[pTar->tid] = 0xFF;
        }
    }

    return returnCode;
}


#ifdef QLDUMP
/**
******************************************************************************
**
**  @brief  Get the specified ISP register contents.
**
**          To provide a means of dumping the specified ISP2x00 registers.
**
**  @param  pBuf - destination buffer pointer
**  @param  pReg - source register pointer
**  @param  count - number of short words to copy
**
**  @return New destination address (updated pBuf)
**
******************************************************************************
**/
static UINT16 *isp_GetRegs(UINT16 *pBuf, volatile UINT16 *pReg, UINT32 count)
{
    /*
     ** Copy source (short) to destination (short), incrementing both, and
     ** repeat until the count is exhausted.
     */
    do
    {
        *pBuf++ = *pReg++;
    } while (--count > 0);

    return pBuf;
}


/**
******************************************************************************
**
**  @brief      Get the specified ISP register contents.
**
**              To provide a means of dumping the specified ISP2400 registers.
**
**  @param      pBuf    - destination buffer pointer
**  @param      pReg    - source register pointer
**  @param      count   - number of words to copy
**
**  @return     none
**
******************************************************************************
**/
static void isp2400_GetRegs(UINT32 *pBuf, volatile UINT32 *pReg, UINT32 count)
{
    /*
     ** Copy source to destination, incrementing both, and
     ** repeat until the count is exhausted.
     */
    do
    {
        *pBuf++ = *pReg++;
    } while (--count > 0);
}


/**
******************************************************************************
**
**  @brief  Save isp commands
**
**  @param  port - Chip instance of QLogic ISP2x00 (0-3).
**
**  @return none
**
******************************************************************************
**/
static void save_isp_commands(UINT8 port)
{
    if (saved_isp_cmds[port].head)
    {
        fprintf(stderr, "%s: Commands already saved, port=%d\n", __func__, port);
    }
    saved_isp_cmds[port].head = ispdefq[port * 2];
    saved_isp_cmds[port].tail = ispdefq[(port * 2) + 1];
    ispdefq[port * 2] = NULL;
    ispdefq[(port * 2) + 1] = NULL;
}


/**
******************************************************************************
**
**  @brief  Restore saved isp commands
**
**  @param  port - Chip instance of QLogic ISP2x00 (0-3).
**  @param  head - head of command queue to link
**  @param  tail - tail of command queue to link
**
**  @return none
**
******************************************************************************
**/
static void restore_isp_commands(UINT8 port)
{
    QRP        *qtmp;
    QRP        *head;

    qtmp = ispdefq[port * 2];
    if (qtmp)
    {
        fprintf(stderr, "%s: Commands unexpectedly on queue, port=%d!\n", __func__, port);
    }
    head = saved_isp_cmds[port].head;
    ispdefq[port * 2] = head;
    if (head)
    {
        saved_isp_cmds[port].tail->pFThd = qtmp;
    }
    ispdefq[(port * 2) + 1] = saved_isp_cmds[port].tail;
    saved_isp_cmds[port].head = NULL;
    saved_isp_cmds[port].tail = NULL;
}
#endif /* QLDUMP */


/**
******************************************************************************
**
**  @brief  Dump the ISP2x00 registers and sram for debugging by qlogic.
**
**          The ISP registers and SRAM are dumped to memory, and the chip
**          is then reset and also re-initialized, if specified.
**
**  @param  port - Chip instance of QLogic ISP2x00 (0-3).
**  @param  reason - Reason code
**
**  @return none
**
******************************************************************************
**/
void ISP_DumpQL(UINT8 port, UINT32 reason)
{
#ifdef QLDUMP                   /* Disable QL dumps in PERFormance build */
    ISP2400_DumpQL(port, reason);
#else  /* QLDUMP */
    /* Reset/Reinitialize ISP */

    if (ISP_ResetChip(port, reason) == 0)
    {
        ispdump[port].debug |= 0x80;    /* Success */
    }
    else
    {
        ispdump[port].debug |= 0x40;    /* Failure */
    }
#endif /* QLDUMP */
}


#ifdef  QLDUMP
/*
******************************************************************************
**
**  @brief  Read memory from 2400 HBA for dump
**
**  @param  port - Port number
**  @param  raddr - RISC address to read from
**  @param  haddr - Host address to get data - must be virtual and in shm
**  @param  nwds - Number of words to read
**
******************************************************************************
*/
static void ISP2400_GetMem(UINT8 port, UINT32 raddr, UINT32 haddr, UINT32 nwds)
{
    struct ISP_2400 *isp24 = ispstr[port]->baseAd;
    QRP        *qrp;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    /* Fill in command to read memory */

    qrp->imbr[0] = 0x000C;
    qrp->imbr[1] = XIO_LSW(raddr);
    qrp->imbr[2] = XIO_MSW((UINT32)haddr + K_poffset);
    qrp->imbr[3] = XIO_LSW((UINT32)haddr);
    qrp->imbr[4] = XIO_MSW(nwds);
    qrp->imbr[5] = XIO_LSW(nwds);       /* No. of words to read */
    qrp->imbr[6] = 0;           /* Bits 63 - 48 of dest. addr. */
    qrp->imbr[7] = 0;           /* Bits 47 - 32 of dest. addr. */
    qrp->imbr[8] = XIO_MSW(raddr);

    qrp->iRegs = 0x1FF;         /* Set modify mailbox reg 0-8 */
    qrp->oRegs = 0x1;           /* Set retrieve mailbox reg 0 */

    isp_exec_cmd_sri(port, qrp, FALSE);

    while ((isp24->ints & 8) == 0)      /* Wait for command to complete */
    {
        i_wait(125);
    }

    isp24->hccr = ISP2400HCCR_CLEAR_RISC_INTR;  /* Clear RISC interrupt */
    FORCE_WRITE_BARRIER;

    isp_process_mbx(port, isp24->mBox[0]);      /* Get output mailbox registers */

    if (qrp->ombr[0] != ISP_CMDC)       /* Check return value */
    {
        unsigned int i;

        fprintf(stderr, "%s: Command failed with status 0x%X\n", __func__, qrp->ombr[0]);

        /* Send debug message to CCB */

        LOG_MB_FAILED_PKT embf;

        embf.header.event = LOG_MB_FAILED;
        embf.data.port = port;
#ifdef FRONTEND
        embf.data.proc = 0;
#else  /* FRONTEND */
        embf.data.proc = 1;
#endif /* FRONTEND */
        embf.data.iregs = qrp->iRegs;
        embf.data.oregs = qrp->oRegs;

        for (i = 0; i < dimension_of(embf.data.imbr); ++i)
        {
            embf.data.imbr[i] = qrp->imbr[i];
            embf.data.ombr[i] = qrp->ombr[i];
        }
#ifdef DEBUG_FLT_REC_ISP
        /* 0x58424D1F = 'XBM'<<8|0x1F */
        MSC_FlightRec(0x58424D1F, port, *(UINT32 *)embf.data.imbr,
                      *(UINT32 *)embf.data.ombr);
#endif /* DEBUG_FLT_REC_ISP */
        /* Note: message is short, and L$send_packet copies into the MRP */

        MSC_LogMessageStack(&embf, sizeof(LOG_MB_FAILED_PKT));
    }

    put_qrp(qrp);               /* Release QRP */
}


/**
******************************************************************************
**
**  @brief  Dump QLogic queue
**
**  @param  port - port number
**  @param  que - Pointer to queue
**  @param  name - Name of queue
**
**  @return none
**
******************************************************************************
*/
static void dump_isp_queue(int port, QCB * que, const char *name)
{
    FILE       *fd;
    size_t      size;
    char        fn[80];

    snprintf(fn, sizeof(fn), "/var/log/dump/qldmp" LC_PROCSTR "%s%d.bin", name, port);
    fd = fopen(fn, "w");
    if (!fd)
    {
        fprintf(stderr, "%s: Failed to open %s with %s\n", __func__, fn, strerror(errno));
        return;
    }

    size = (unsigned)que->end - (unsigned)que->begin;
    size = fwrite(que->begin, size, 1, fd);
    if (size != 1)
    {
        fprintf(stderr, "%s: Failed to write %s\n", __func__, fn);
    }
    fclose(fd);
}


/**
******************************************************************************
**
**  @brief  Dump QLogic 2400
**
**  @param  Port number of 2400 to dump
**  @param  Reason for dumping
**
**  @return None
**
******************************************************************************
*/
static void ISP2400_DumpQL(UINT8 port, UINT32 reason)
{
    int         i;
    FILE       *fd;
    size_t      sz;
    struct ISP_2400 *isp24 = ispstr[port]->baseAd;
    UINT32      buf[64];
    UINT16      sbuf[32];
    char        fn[80];
    static volatile UINT32 buflock;
    static UINT32 lbuf[0x40000] LOCATE_IN_SHMEM;

    fprintf(stderr, "%s%s: port %d Reason 0x%4.4X\n", FEBEMESSAGE, __func__, port, reason);

    if (ispdump[port].length == 0)
    {
        ispdump[port].debug = 1;
        ispdump[port].addr = NULL;
        ispdump[port].length = 1;
    }

#ifdef DEBUG_FLT_REC_ISP
    MSC_FlightRec(0xDDD00DDD | (port << 12), 0, ispdump[port].length,
                  reason | (ispdump[port].count << 16) | (ispdump[port].debug << 24));
#endif /* DEBUG_FLT_REC_ISP */

    ispdump[port].reason = reason;
    ++ispdump[port].count;
    ispdump[port].timestamp = timestamp;

    snprintf(fn, sizeof(fn), "/var/log/dump/qldmp" LC_PROCSTR "%d.bin", port);

    /*
     ** If the reset interlock bit is set, wait for it to be cleared.
     ** Then set the bit for this port to disallow IOCB & QRP processing.
     */
    int count = 0;
    while (BIT_TEST(resilk, port))
    {
        if (count == 0)
        {
            fprintf(stderr, "waiting for reset interlock bit on port %d\n", port);
        }
        count++;
        if (count >= 960)       /* 120 seconds */
        {
            fprintf(stderr, "Still waiting for reset interlock bit on port %d, abort!\n", port);
            abort();
        }
        TaskSleepMS(125);
    }
    BIT_SET(resilk, port);

    fd = fopen(fn, "w");
    if (!fd)
    {
        BIT_CLEAR(resilk, port);
        ispdump[port].debug = 2;
        fprintf(stderr, "%s: Aborted dump, open of %s failed with %s\n",
                __func__, fn, strerror(errno));
        ISP_ResetChip(port, reason);
        return;
    }

    /* Write to dump file */
    sz = fwrite(isprev[port], sizeof(*isprev[port]), 1, fd);
    if (sz == 0)
    {
        goto fwrite_err;
    }
    fprintf(stderr, "%s: dump port %d wrote ISP revision data\n", __func__, port);

    /* Pause the RISC processor. Get the registers if successful */
    if (isp_PauseRISC(port) != 0)
    {
#define ISP32_PTR(i, o) ((volatile UINT32 *)(i) + (o) / sizeof(UINT32))

        ispdump[port].debug |= 0x10;
        buf[0] = isp24->r2HStat;

        /* Get Hostinterface registers */
        isp2400_GetRegs(&buf[1], &isp24->fBIOSAddr, 32);

#define ARR_DUMP(arr, n) sz = fwrite(arr, sizeof(arr[0]), n, fd); \
        if (sz != (n)) { goto fwrite_err; }

        ARR_DUMP(buf, 33);      /* Write to dump file */
        fprintf(stderr, "%s: dump port %d ISP Hostinterface registers\n", __func__, port);

        isp24->intc = 0;        /* Disable Interrupts */

        /* Shadow Registers */
        isp24->IObase = 0x0F70; /* IObase is at PCI offset 54h */
        FORCE_WRITE_BARRIER;
        for (i = 0; i < 7; ++i)
        {
            *ISP32_PTR(isp24, 0xF0) = 0xB0000000 + (i << 20);
            isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xFC), 1);
            ARR_DUMP(buf, 1);   /* Write to dump file */
        }
        fprintf(stderr, "%s: dump port %d ISP Shadow Registers\n", __func__, port);

        /* MailBox Registers */
        isp_GetRegs(&sbuf[0], &isp24->mBox[0], 32);

        ARR_DUMP(sbuf, 32);     /* Write to dump file */
        fprintf(stderr, "%s: dump port %d ISP MailBox registers\n", __func__, port);

        /* Transfer Sequence Registers */
        for (i = 0; i < 8; ++i)
        {
            isp24->IObase = 0xBF00 + (i << 4);
            FORCE_WRITE_BARRIER;
            isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);
            ARR_DUMP(buf, 16);  /* Write to dump file */
        }
        fprintf(stderr, "%s: dump port %d ISP Transfer Sequence registers #1\n", __func__, port);

        isp24->IObase = 0xBFE0;
        FORCE_WRITE_BARRIER;
        isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);

        isp24->IObase = 0xBFF0;
        FORCE_WRITE_BARRIER;
        isp2400_GetRegs(&buf[16], ISP32_PTR(isp24, 0xC0), 16);

        ARR_DUMP(buf, 32);      /* Write to dump file */
        fprintf(stderr, "%s: dump port %d ISP Transfer Sequence registers #2\n", __func__, port);

        /* Receive Sequence Registers */
        for (i = 0; i < 8; ++i)
        {
            isp24->IObase = 0xFF00 + (i << 4);
            FORCE_WRITE_BARRIER;
            isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);
            ARR_DUMP(buf, 16);  /* Write to dump file */
        }
        fprintf(stderr, "%s: dump port %d ISP Receive Sequence registers #1\n", __func__, port);

        isp24->IObase = 0xFFD0;
        FORCE_WRITE_BARRIER;
        isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);

        isp24->IObase = 0xFFE0;
        FORCE_WRITE_BARRIER;
        isp2400_GetRegs(&buf[16], ISP32_PTR(isp24, 0xC0), 16);

        isp24->IObase = 0xFFF0;
        FORCE_WRITE_BARRIER;
        isp2400_GetRegs(&buf[32], ISP32_PTR(isp24, 0xC0), 16);
        fprintf(stderr, "%s: dump port %d ISP Receive Sequence registers #2\n", __func__, port);

        /* Command DMA Registers */
        isp24->IObase = 0x7100;
        FORCE_WRITE_BARRIER;
        isp2400_GetRegs(&buf[48], ISP32_PTR(isp24, 0xC0), 16);

        ARR_DUMP(buf, 64);      /* Write to dump file */
        fprintf(stderr, "%s: dump port %d ISP Command DMA registers\n", __func__, port);

        /* DMA Queues */
        isp24->IObase = 0x7200;
        FORCE_WRITE_BARRIER;
        isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 8);
        isp2400_GetRegs(&buf[8], ISP32_PTR(isp24, 0xE4), 7);

        isp24->IObase = 0x7300;
        FORCE_WRITE_BARRIER;
        isp2400_GetRegs(&buf[15], ISP32_PTR(isp24, 0xC0), 8);
        isp2400_GetRegs(&buf[23], ISP32_PTR(isp24, 0xE4), 7);

        isp24->IObase = 0x7400;
        FORCE_WRITE_BARRIER;
        isp2400_GetRegs(&buf[30], ISP32_PTR(isp24, 0xC0), 8);
        isp2400_GetRegs(&buf[38], ISP32_PTR(isp24, 0xE4), 7);

        ARR_DUMP(buf, 45);      /* Write to dump file */
        fprintf(stderr, "%s: dump port %d ISP DMA queues\n", __func__, port);

        /* Transmit DMA Registers */
        for (i = 0; i < 11; ++i)
        {
            isp24->IObase = 0x7600 + (i << 4);
            FORCE_WRITE_BARRIER;
            isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);
            ARR_DUMP(buf, 16);  /* Write to dump file */
        }
        fprintf(stderr, "%s: dump port %d ISP Transmit DMA Registers\n", __func__, port);

        /* Receive DMA Registers */
        for (i = 0; i < 4; ++i)
        {
            isp24->IObase = 0x7700 + (i << 4);
            FORCE_WRITE_BARRIER;
            isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);
            ARR_DUMP(buf, 16);  /* Write to dump file */
        }
        fprintf(stderr, "%s: dump port %d ISP Receive DMA Registers\n", __func__, port);

        /* RISC Registers */
        for (i = 0; i < 8; ++i)
        {
            isp24->IObase = 0x0F00 + (i << 4);
            FORCE_WRITE_BARRIER;
            isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);
            ARR_DUMP(buf, 16);
        }
        fprintf(stderr, "%s: dump port %d ISP RISC Registers\n", __func__, port);

        /* Local Memory Controller Registers */
        for (i = 0; i < 7; ++i)
        {
            isp24->IObase = 0x3000 + (i << 4);
            FORCE_WRITE_BARRIER;
            isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);
            ARR_DUMP(buf, 16);
        }
        fprintf(stderr, "%s: dump port %d ISP Local Memory Control Registers\n", __func__, port);

        /* Fibre Protocol Module Registers */
        for (i = 0; i < 12; ++i)
        {
            isp24->IObase = 0x4000 + (i << 4);
            FORCE_WRITE_BARRIER;
            isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);
            ARR_DUMP(buf, 16);
        }
        fprintf(stderr, "%s: dump port %d ISP Fibre Protocol Module Registers\n", __func__, port);

        /* Frame Buffer Registers */
        for (i = 0; i < 5; ++i)
        {
            isp24->IObase = 0x6000 + (i << 4);
            FORCE_WRITE_BARRIER;
            isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);
            ARR_DUMP(buf, 16);
        }
        fprintf(stderr, "%s: dump port %d ISP Frame Buffer Registers\n", __func__, port);

        isp24->IObase = 0x6100;
        FORCE_WRITE_BARRIER;
        isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);
        ARR_DUMP(buf, 16);

        for (i = 0; i < 5; ++i)
        {
            isp24->IObase = 0x6130 + (0x20 * i);
            FORCE_WRITE_BARRIER;
            isp2400_GetRegs(&buf[0], ISP32_PTR(isp24, 0xC0), 16);
            ARR_DUMP(buf, 16);
        }
        fprintf(stderr, "%s: dump port %d ISP Frame Buffer Registers completed \n", __func__, port);
#undef  ISP32_PTR
    }
    else
    {
        buf[0] = 0;
        for (i = 0; i < 1301; ++i)
        {
            ARR_DUMP(buf, 1);
        }
        fprintf(stderr, "%s: Pause RISC on port %d failed\n", __func__, port);
    }
#undef  ARR_DUMP

    /* Soft Reset the ISP2400 */
    if (isp_softReset(port, FALSE) == 0)
    {
        ispdump[port].debug |= 0x20;
    }

    save_isp_commands(port);

    if (buflock)                /* If buffer lock taken, wait for it */
    {
        /* Wait for buffer lock to clear */
        fprintf(stderr, "%s: Waiting for buffer lock on port %d\n", __func__, port);
        while (buflock)
        {
            TaskSleepMS(10);
        }
        fprintf(stderr, "%s: Got buflock on port %d\n", __func__, port);
    }
    buflock = 1;                /* Take buffer lock */

#define MEM_DUMP(arr, n) sz = fwrite(arr, sizeof(arr[0]), n, fd); \
        if (sz != (n)) { goto memwrite_err; }

    ISP2400_GetMem(port, 0x20000, (UINT32)&lbuf, 0x2000);

    MEM_DUMP(lbuf, 0x2000);     /* Write to dump file */
    fprintf(stderr, "%s: dump port %d ISP Memory @ 0x20000 for 0x2000\n", __func__, port);

    sz = isprev[port]->endMemAddr - 0x00100000 + 1;
    ISP2400_GetMem(port, 0x00100000, (UINT32)&lbuf, sz);

    MEM_DUMP(lbuf, sz);
    fprintf(stderr, "%s: dump port %d ISP Memory @ 0x00100000 for 0x%lx\n", __func__, port, sz);

    buflock = 0;                /* Release buffer lock */

    fclose(fd);                 /* Close dump file */
    fprintf(stderr, "%s: dump port %d complete phase 1 (of 5)\n", __func__, port);
#undef  MEM_DUMP

    dump_isp_queue(port, ispstr[port]->reqQue, "req");
    dump_isp_queue(port, ispstr[port]->resQue, "res");
    dump_isp_queue(port, ispstr[port]->atioQue, "atio");
    dump_isp_queue(port, asyqa[port], "asyqa");
    fprintf(stderr, "%s: dump port %d complete phase 5 (of 5) -- reason=0x%4.4X\n", __func__, port, reason);

    FORCE_WRITE_BARRIER;
    isp24->hccr = ISP2400HCCR_CLEAR_RISC_INTR;  /* Clear RISC interrupt */
    FORCE_WRITE_BARRIER;

    // ME-89
    //The card does not recover from the risc paused errror with a chip reset.
    //The reset will succeed and we will think the card is working but it is not.
    //It needs a reboot. The theory is that the PCI RESET clears the error.
    //Support wants this for all dumps.
    // ME-625 ... DSC in field did not return from ISP_ResetChip() routine. Must be
    // tied up in an iwait() while loop. Don't even try on a 10D. Let PAM do it or the
    // reboot.
    if (reason == ISP_RISC_PAUSED)
    {
        //flush file buffers to disk
        fprintf(stderr, "%s: doing sync() for port %d before errExit\n", __func__, port);
        sync();
        fprintf(stderr, "%s: dump port %d doing sync() then errExit\n", __func__, port);
        errExit(ERR_EXIT_BIOS_REBOOT);
    }

    /* Clear the reset interlock so reset chip will execute */
    BIT_CLEAR(resilk, port);

#ifdef DEBUG_FLT_REC_ISP
    MSC_FlightRec(0xDDD00DDD | (port << 12), 0, ispdump[port].length,
                  reason | (ispdump[port].count << 16) | (ispdump[port].debug << 24));
#endif /* DEBUG_FLT_REC_ISP */

    restore_isp_commands(port);

    if (ISP_ResetChip(port, reason) == 0)       /* Reset/Reinitialize ISP */
    {
        ispdump[port].debug |= 0x80;    /* Success */
    }
    else
    {
        ispdump[port].debug |= 0x40;    /* Failure */
    }

    return;

fwrite_err:
    fprintf(stderr, "%s: Aborted dump, fwrite failed, returning %ld\n", __func__, sz);
    goto err;

memwrite_err:
    fprintf(stderr, "%s: Aborted dump, memory write failed, returning %ld\n",
            __func__, sz);
    buflock = 0;                /* Release buffer lock */
err:
    /* Clear the reset interlock so reset chip will execute */
    BIT_CLEAR(resilk, port);
    fclose(fd);
    ispdump[port].debug = 2;
    if(reason == ISP_RISC_PAUSED)
    {
        sync();
        fprintf(stderr, "%s: dump err port %d doing sync() then errExit\n", __func__, port);
        errExit(ERR_EXIT_BIOS_REBOOT);
    }
    ISP_ResetChip(port, reason);
}
#endif /* QLDUMP */


/**
******************************************************************************
**
**  @brief  Issues the Abort Target mailbox command
**
**  @param  port - port number
**  @param  lid - loop id/handle
**
**  @return mailbox completion status
**
******************************************************************************
**/
UINT16 ISP_AbortTarget(UINT8 port, UINT32 lid)
{
    return isp2400_sendTMF(port, lid, 0, ISP2400_TMF_ABORT_TARGET);
}


/**
******************************************************************************
**
**  @brief  Issues the Initiate LUN Reset mailbox command.
**
**  @param  port - Port number
**  @param  lid - LID/handle
**
**  @return return status.
**
******************************************************************************
**/
UINT16 ISP_lunReset(UINT8 port, UINT32 lid)
{
    return isp2400_sendTMF(port, lid, 0, ISP2400_TMF_LUN_RESET);
}


/**
******************************************************************************
**
**  @brief      Aborts a particular command IOCB.
**
**              Abort Command IOCB aborts a particular command IOCB
**              that has been submitted previously to the QLogic instance.
**              The identifier for this IOCB is the ILT address that was
**              passed to the <ISP$initiate_io> routine. The Loop ID
**              is required as well as the LUN of the target command to abort.
**
**  @param      port    - Port
**  @param      lid     - LID
**  @param      LUN for the command
**  @param      ILT address stored in IOCB
**
**  @return     return status.
**
******************************************************************************
**/
UINT16 ISP_AbortIOCB(UINT8 port, UINT32 lid, UINT16 lun UNUSED, UINT32 ilt)
{
    UINT16      retValue;
    ISP2400_ABORT_IOCB *iocb;
    PDB        *portdbptr;
    UINT32      alpa;
    ILT *       abortilt;
    UINT32      rqip;
    ISP2400_ABORT_IOCB * RespIOCB;

    abortilt = get_ilt();
    RespIOCB = s_MallocC(sizeof(*RespIOCB), __FILE__, __LINE__);

    iocb = (ISP2400_ABORT_IOCB *)isp_get_iocb(port, &rqip);
    memset(iocb, 0, IOCB_SIZE);
    portdbptr = portdb[port] + lid;
    iocb->entryType = ABORTIOCB;
    iocb->entryCount = 1;
    iocb->nphandle = lid;
    iocb->abortoptions = 0;     /* send ABTS */
    iocb->iocbhandletobeaborted = ilt;
    alpa = pdbPidToALPA(portdbptr->pid);
    iocb->alpa15_0 = alpa & 0xFFFF;
    iocb->alpa24_16 = (UINT8)(alpa >> 16);
    abortilt++;
    iocb->iocbhandle = (UINT32)abortilt;
    abortilt--;
    retValue = exec_iocb_wait(port, abortilt, rqip,(UINT16*) RespIOCB, 5);
    if ((retValue & 0xFF) == 0)
    {
        /* The nphandle is reused as the status for this command */
        if (RespIOCB->nphandle)
        {
            retValue = ISP_CMDE;
        }
    }
    s_Free(RespIOCB, sizeof(*RespIOCB), __FILE__, __LINE__);
    put_ilt(abortilt);
    return retValue;
}


/**
******************************************************************************
**
**  @brief  Aborts all IOCB for specified device.
**
**          Abort Queue aborts all IOCB for the specified device
**          that has been submitted previously to the QLogic instance.
**          The Loop ID and LUN are required of the IOCBs to abort.
**
**  @param  port - Port number
**  @param  lid - LID/nphandle
**  @param  LUN for the command
**
**  @return mailbox completion status
**
******************************************************************************
**/
UINT16 ISP_AbortQueue(UINT8 port, UINT32 lid, UINT16 lun)
{
    return isp2400_sendTMF(port, lid, lun, ISP2400_TMF_ABORT_QUEUE);
}


/**
******************************************************************************
**
**  @brief  Aborts all tasks from all initiators in the specified task set.
**
**          Abort Command IOCB aborts a particular command IOCB
**          that has been submitted previously to the QLogic instance.
**          The identifier for this IOCB is the ILT address that was
**          passed to the <ISP$initiate_command> routine. The Loop ID
**          is required as well as the LUN of the target command to abort.
**
**  @param  port - Port number
**  @param  lid - LID/nphandle
**  @param  LUN for the command
**
**  @return mailbox completion status
**
******************************************************************************
**/
UINT16 ISP_ClearTaskSet(UINT8 port, UINT32 lid, UINT16 lun)
{
    return isp2400_sendTMF(port, lid, lun, ISP2400_TMF_CLEAR_TASK_SET);
}


/**
******************************************************************************
**
**  @brief  Aborts all tasks in the task set for this QLogic instance.
**
**          Aborts all tasks in the task set for this QLogic instance.
**          All pending or executing commands are returned with
**          Command Aborted status
**
**  @param  port - Port number
**  @param  lid - LID/nphandle
**  @param  LUN for the command
**
**  @return mailbox completion status
**
******************************************************************************
**/
UINT16 ISP_AbortTaskSet(UINT8 port, UINT32 lid, UINT16 lun)
{
    return isp2400_sendTMF(port, lid, lun, ISP2400_TMF_ABORT_TASK_SET);
}


/**
******************************************************************************
**
**  @brief  Issues the Initiate Target Reset mailbox command.
**
**  @param  port - Port number
**  @param  lid - LID/nphandle
**
**  @return mailbox completion status
**
******************************************************************************
**/
UINT16 ISP_TargetReset(UINT8 port, UINT32 lid)
{
    return isp2400_sendTMF(port, lid, 0, ISP2400_TMF_TARGET_RESET);
}


/**
******************************************************************************
**
**  @brief      Issues the Initiate LIP mailbox command.
**
**      This command executes a Loop Initialization Procedure (LIP).
**      In point-to-point mode, this command executes link initialization.
**
**  @param      port    - Port
**
**  @return     return status.
**
******************************************************************************
**/
UINT16 ISP_initiateLip(UINT8 port)
{
    QRP        *qrp;
    UINT16      retValue;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_LINIT;   /* Execute Initiate LIP command */

    qrp->iRegs = 0xf;           /* Set modify mailbox reg 0, 1, 2, 3       */
    qrp->oRegs = 0x1;           /* Set retrieve mailbox reg 0           */
    qrp->imbr[1] = 0x10;        /* Perform link initialization          */
    qrp->imbr[2] = 0xF7F7;      /* Set lip type                         */
    qrp->imbr[3] = 0;

    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];    /* Get completion status */

    put_qrp(qrp);               /* Release QRP */

    return retValue;
}


/**
******************************************************************************
**
**  @brief  Issues the Initiate LIP Reset mailbox command.
**
**  @param  port - Port number
**  @param  lid - LID/nphandle (NO_LID implies all devices)
**
**  @return mailbox command completion status.
**
******************************************************************************
**/
UINT16 ISP_LipReset(UINT8 port, UINT32 lid)
{
    QRP        *qrp;
    UINT16      retValue;

#ifdef BACKEND
    /*
     ** Check if fabric mode
     ** LID NO_LID implies a reset of all local loop devices was requested.
     */
    if (ispConnectionType[port] == F_PORT ||
        (ispConnectionType[port] == FL_PORT && !isp_isdevicelocal(port, lid, 0)))
    {
        if (lid != NO_LID && lid != NO_CONNECT)
        {
            /* Fabric mode - send LINIT to remote fabric address */

            return ISP_LoopInitialize(port, lid);
        }
        else
        {
            return ISP_CMDC;
        }
        /*
         ** Don't LIP Reset all devices when in fabric mode.
         ** LIP the local loop instead.
         */
        /*   Don't break our connection to the switch  just because we are having issues
         **   with a drive on a remote loop.
         *    return ISP_initiateLip(port);
         */
    }
#endif /* BACKEND */

    /* Handle local loop devices */
    if (ispLipIssued[port] != 0xFFFF)
    {
        /* LIP is currently in process of being issued */

        return ISP_LIPIP;
    }

    ispLipIssued[port] = lid;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    UINT16      lipbyte3and4;

    qrp->imbr[0] = ISP_LINIT;
    qrp->iRegs = 0xF;           /* Set modify mailbox reg 0-3   */
    qrp->oRegs = 0x1;           /* Set retrieve mailbox reg 0   */
    qrp->imbr[1] = 0x10;        /* Perform link initialization  */
    lipbyte3and4 = portid[port] & 0xFF; /* Our alpa  */
    if (lid == NO_LID)
    {
        lipbyte3and4 |= 0xFF00; /* Reset all    */
    }
    else
    {
        /* Target alpa to reset */

        lipbyte3and4 |= (isp_handle2alpa(port, lid) & 0xFF) << 8;
    }
    qrp->imbr[2] = lipbyte3and4;        /* Set lip type         */
    qrp->imbr[3] = 2;           /* Delay period in seconds  */

    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];    /* Get completion status */

    put_qrp(qrp);               /* Release QRP */

    if (retValue != ISP_CMDC)   /* Did the mailbox command fail? */
    {
        ispLipIssued[port] = 0xFFFF;
    }

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Issues the login loop port mailbox command.
**
**  @param      port    - Port
**  @param      lid     - LID
**
**  @return     UINT32  - ISP completion code (15:0) and add'l info (31:16)
**                        4000h = ISP_CMDC = Successful command completion
**                        4005h = ISP_CMDE = Command Error, add'l info:
**                            000Dh = Login Reject Error (LS_RJT received)
**
******************************************************************************
**/
UINT32 ISP_LoginLoopPort(UINT8 port, UINT32 lid, UINT8 vpid)
{
    UINT32      retValue;
    struct portlogiiocb_t *iocb;
    struct portlogiiocb_t *RespIOCB;
    UINT32      alpa;
    ILT         *ilt;
    UINT32      rqip;
    /* Get an IOCB for the request */

    switch (lid)
    {
        case SNS_NPORT_HANDLE:
            alpa = 0xFFFFFC;
            break;

        case FABIC_CNTL_NPORT_HANDLE:
            alpa = 0xFFFFFD;
            break;

        case FABIC_PORT_NPORT_HANDLE:
            alpa = 0xFFFFFE;
            break;

        default:
            alpa = isp_handle2alpa(port, lid);
    }
    ilt = get_ilt();
    RespIOCB = (struct portlogiiocb_t *)s_MallocC(IOCB_SIZE, __FILE__, __LINE__);

    iocb = (struct portlogiiocb_t *)isp_get_iocb(port,&rqip);
    if (iocb == NULL)
    {
        put_ilt(ilt);
        s_Free(iocb, sizeof(*RespIOCB), __FILE__, __LINE__);
        return ISP_CMDE;
    }
    iocb->entryType = PLOGXIOCB;
    iocb->entryCount = 1;
    iocb->lid = lid;
    iocb->controlflags = 0x10;  /* bit     option
                                 * 0-3     cmd 0 = plogi
                                 * 4      conditional Login
                                 */
    iocb->portid0_15 = alpa & 0xFFFF;
    iocb->portid16_23 = alpa >> 16;
    iocb->vpindex = vpid;

    /* Execute the command */
    ilt++;
    iocb->handle = (UINT32)ilt;
    ilt--;
    retValue = exec_iocb_wait(port, ilt, rqip, (UINT16*)RespIOCB, 30);
    if ((retValue & 0xFF) == 0)
    {
        /* If MB succeeds check IOCB status
         * If error set code to trick upper layer
         */
        if (RespIOCB->status)
        {
            retValue = ISP_CMDE;
        }
    }
    s_Free(RespIOCB, sizeof(*iocb), __FILE__, __LINE__);
    put_ilt(ilt);
    return retValue;
}


/**
******************************************************************************
**
**  @brief  Issues the port logout port mailbox command.
**
**      The Port Logout command issues a port logout command to a specified
**      target device. The LOGO payload is specified by the driver (us).
**
**  @param  port - Port
**  @param  lid - LID
**
**  @return return status.
**
******************************************************************************
**/
UINT16 ISP_PortLogout(UINT8 port, UINT32 lid)
{
    UINT16      retValue;
    struct portlogoiocb_t *iocb;
    ISP2400_VPORT_ICB *pVPICB = (ISP2400_VPORT_ICB *)ispstr[port]->icbStr;
    struct portlogiiocb_t *RespIOCB;
    ILT         *ilt;
    UINT32      rqip;

    RespIOCB = s_MallocC(sizeof(*iocb), __FILE__, __LINE__);     /* Get an IOCB for the request */
    ilt = get_ilt();
    iocb = (struct portlogoiocb_t *)isp_get_iocb(port,&rqip);
    if (iocb == NULL)
    {
        put_ilt(ilt);
        s_Free(RespIOCB, sizeof(*iocb), __FILE__, __LINE__);
        return ISP_CMDE;
    }
    iocb->entryType = PLOGXIOCB;
    iocb->entryCount = 1;
    iocb->lid = lid;
    iocb->controlflags = 0x8;   /* PLOGO */
    iocb->portid0_15 = isp_handle2alpa(port, lid) & 0xFF;
    iocb->portid16_23 = 0;
    iocb->portname = portid[port];
    iocb->portWWN = myhtonll(pVPICB->nicb.portWWN);

    /* Execute the command */
    ilt++;
    iocb->handle = (UINT32)ilt;
    ilt--;
    retValue = exec_iocb_wait(port, ilt, rqip, (UINT16*)RespIOCB, 30);

    if ((retValue & 0xFF) == 0)
    {
        /* If MB succeeds check IOCB status if error set code to trick upper layer */
        if (RespIOCB->status)
        {
            retValue = ISP_CMDE;
        }
    }
    s_Free(RespIOCB, sizeof(*iocb), __FILE__, __LINE__);
    put_ilt(ilt);
    return retValue;
}                               /* ISP_PortLogout */


#ifdef BACKEND
/**
******************************************************************************
**
**  @brief      Bypass the specified device on all available ports.
**
**              The Loop Port Bypass (LPB) primitive will be issued to this
**              device on all available ports. The DEV record is examined
**              to determine the necessary LIDs for the existing paths.
**
**  @param      PDD* - Pointer to specified device's PDD
**
**  @return     UINT32 - 0 = Successful completion, bypassed on all paths
**                       1 = Failure, invalid parameter or nonexistent device
**              4005h = Command error. The loop could not be acquired.
**              4006h = Command parameter error. The specified loop ID is not
**                      logged in or is invalid.
**
******************************************************************************
**/
UINT32 ISP_BypassDevice(PDD * pPDD)
{
    LOG_PORT_EVENT_PKT eldn;    /* Log event        */
    struct DEV *pDev;           /* DEVice record    */
    UINT8       port;           /* Port number      */
    UINT32      lid;            /* Loop ID          */
    UINT16      qStatus;        /* Status from LPB  */
    UINT32      rc;             /* Return code      */

    rc = 1;

    if (!pPDD)                  /* If no PDD pointer */
    {
        return 1;
    }

    pDev = pPDD->pDev;          /* Get DEVice pointer from PDD */

    if (!pDev)                  /* If no DEV pointer */
    {
        return 1;
    }

    /* Should we validate any failure status for this device? */

    /* Issue Loop Port Bypass commands to device for all available paths */

    for (port = 0; port < MAX_PORTS; ++port)
    {
        lid = pDev->pLid[port];
        if (lid == NO_CONNECT)
        {
            continue;
        }

        /* Send Loop Port Bypass to valid port/LID pair */

        qStatus = ISP_LoopPortBypass(port, lid);

        /* What to do with qStatus? */
        /* Setbit for port? */
        rc = qStatus & 0xFF;

        /* Log the Bypass Device action */

        eldn.header.event = LOG_BYPASS_DEVICE;
        eldn.data.port = port;
        eldn.data.proc = 1;
        eldn.data.reason = qStatus;
        eldn.data.count = lid;

        /* Note: message is short, and L$send_packet copies into the MRP */

        MSC_LogMessageStack(&eldn, sizeof(eldn));
    }

    return rc;
}
#endif /* BACKEND */


/**
******************************************************************************
**
**  @brief      Issues the loop port bypass mailbox command.
**
**              The Loop Port Bypass command transmits the LPByx primitive
**              sequence on the loop. The y in the LPByx primitive represents
**              the AL_PA of the device to be bypassed; the x represents the
**              AL_PA of the ISP.
**
**              Note that a loop ID value of FFh will bypass all loop ports.
**
**  @param      UINT8 port - ISP Port
**  @param      UINT32 lid - Loop ID (lower 8 bits) of device to be bypassed
**
**  @return     4000h = Command complete. The command completed successfully.
**              4005h = Command error. The loop could not be acquired.
**              4006h = Command parameter error. The specified loop ID is not
**                      logged in or is invalid.
**
******************************************************************************
**/
UINT16 ISP_LoopPortBypass(UINT8 port, UINT32 lid)
{
    QRP        *qrp;
    UINT16      retValue;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_LPB;     /* Execute Loop Port Bypass command */
    qrp->iRegs = 0x03;          /* Set modify mailbox reg 0-1       */
    qrp->oRegs = 0x03;          /* Set retrieve 0, 1 mailbox regs    */
    if (lid == 0xFF)
    {
        qrp->imbr[1] = 0xFF;    /* ALPA      */
    }
    else
    {
        qrp->imbr[1] = isp_handle2alpa(port, lid) & 0xFF;       /* ALPA      */
    }

    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];    /* Get completion status */

    put_qrp(qrp);               /* Release QRP */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Issues the loop port enable port mailbox command.
**
**              The Loop Port Enable command transmits the LPEyx primitive
**              sequence on the loop. The y in the LPEyx primitive represents
**              the AL_PA of the device to be enabled; the x represents the
**              AL_PA of the ISP.
**
**              Note that a loop ID value of FFh will enable all loop ports.
**
**  @param      UINT8 port - ISP Port
**  @param      UINT32 lid - Loop ID (lower 8 bits) of device to be enabled
**
**  @return     4000h = Command complete. The command completed successfully.
**              4005h = Command error. The loop could not be acquired.
**              4006h = Command parameter error. The specified loop ID is not
**                      logged in or is invalid.
**
******************************************************************************
**/
UINT16 ISP_LoopPortEnable(UINT8 port, UINT32 lid)
{
    QRP        *qrp;
    UINT16      retValue;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_LPE;     /* Execute Loop Port Enable command */
    qrp->iRegs = 0x03;          /* Set modify mailbox reg 0-1           */
    qrp->oRegs = 0x03;          /* Set retrieve 0, 1 mailbox regs        */
    if (lid == 0xFF)
    {
        qrp->imbr[1] = 0xFF;    /* ALPA      */
    }
    else
    {
        qrp->imbr[1] = isp_handle2alpa(port, lid) & 0xFF;       /* ALPA      */
    }

    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];    /* Get completion status */

    put_qrp(qrp);               /* Release QRP */

    return retValue;
}


#ifdef FRONTEND

/**
******************************************************************************
**
**  @brief      ISP_GetServersLogonList
**
**              More details on this function go here.
**
**  @param      targetId    - Target ID
**  @param      * list      - List pointer
**
**  @return     Number of servers that are logged on.
**
******************************************************************************
**/
UINT16 ISP_GetServersLogonList(UINT16 targetId, UINT8 *list)
{
    UINT8       port;
    PNNL       *buffer;
    UINT32      ndevs;
    UINT32      i;
    UINT32      sid;

    port = ispPortAssignment[targetId];
#if FE_ISCSI_CODE
    /* Ignore iscsi port */

    if (BIT_TEST(iscsimap, port))
    {
        return 0;
    }
#endif /* FE_ISCSI_CODE */

    /* Allocate non-cachable DRAM buffer for ID List */

    buffer = s_MallocW(sizeof(PNNL) * MAX_DEV, __FILE__, __LINE__);

    /* Issue Get Port/Node Name List command to the Qlogic chip instance */

    ndevs = ISP_GetPortNodeNameList(port, 0, buffer);

    /* Check if devices are logged in */

    if (ndevs != 0)
    {
        for (i = 0; i < ndevs; ++i)
        {
            /*
             ** Look up the server ID for this WWN and target.
             ** Include new server in the lookup.
             */
            sid = DEF_WWNLookup(bswap_64(buffer[i].pdn), targetId, TRUE, NULL);

            /* Record this server ID */

            if (sid < MAX_SERVERS)
            {
                list[sid] = TRUE;
            }
        }
    }

    s_Free(buffer, sizeof(PNNL) * MAX_DEV, __FILE__, __LINE__);       /* Release ID List buffer */

    return ndevs;
}


/**
******************************************************************************
**
**  @brief      Indicate if a port is ready.
**
**              Ready means that the port is not being reset and has
**              been online long to be considered good and ready for use.
**
**  @param      port    - Port number - 0xFF for all ports.
**
**  @return     0 = not ready, 0xXX - bitmap of devices that are ready,
**              0xFF - all are ready
**
******************************************************************************
**/
UINT32 ISP_IsReady(UINT32 port)
{
    UINT32      retValue;
    UINT32      bitPort;        /* Bit mask for a port                  */
    UINT8       testPort;       /* Port being tested in loop            */

    /* Check if this is for all paths or a single path */

    if (port > MAX_PORTS)
    {
        bitPort = ispmap;       /* This is for all existing paths */
    }
    else
    {
        bitPort = (1 << port);  /* This is for a single path */
    }

    /* Initially assume the ports are ready until we find out otherwise */

    retValue = bitPort;

    /*
     ** Check if reset interlock bit is set for any port,
     ** or if any port has not completed the initial power on wait.
     */
    if (resilk != 0)
    {
        retValue &= ~resilk;
    }

    /* Check if any port has not completed the initial power on wait */

    if ((ispPow & bitPort) != bitPort)
    {
        retValue &= ispPow;
    }

    /* Check if any port is offline */

    if ((ispOnline & bitPort) != bitPort)
    {
        retValue &= ispOnline;
    }

    /* Check for loop not being online following an offline */

    for (testPort = 0; testPort < MAX_PORTS; ++testPort)
    {
        /* Check if timer is active */

        if (BIT_TEST(bitPort, testPort) && onlineTimer[testPort] != 0)
        {
            BIT_CLEAR(retValue, testPort);
        }
    }

    /* Check if all ports that exist are ready */

    if (port > MAX_PORTS && retValue == ispmap)
    {
        retValue = 0xFFFFFFFF;  /* Indicate all existing ports are ready */
    }

    return retValue;
}
#endif /* FRONTEND */


/**
******************************************************************************
**
**  @brief      isp_loopEventHandler
**
**  @param      a       - Not used.
**  @param      b       - Not used.
**  @param      port    - Port number
**
**  @return     none
**
******************************************************************************
**/
void isp_loopEventHandler(UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port)
{
    UINT32      retryCount;
    UINT32      bitPort;        /* MAXISP - Bit mask for a port         */
    UINT32      counter;        /* power up wait counter                */

    /* Generate a bit mask for this port */

    bitPort = 1 << port;

    /* Check if this is the first initialization of the port */

    if ((ispPow & bitPort) == 0)
    {
        /* Check if port is offline */

        if ((ispOnline & bitPort) == 0)
        {
            /* Initialize power up wait counter */

            counter = POWER_UP_WAIT;

            /* Wait specified time period for the loop to come up */

            while ((ispOnline & bitPort) == 0 && counter != 0)
            {
                /* Wait a second and decrement counter */

                TaskSleepMS(1000);
                --counter;
            }

            /* Check if port is still offline */

            if ((ispOnline & bitPort) == 0)
            {
                LOG_PORT_EVENT_PKT G_eldn;

                /*
                 ** Send warning message to CCB. The CCB may take additional
                 ** action at this time, such as failing this port.
                 */
                G_eldn.header.event = LOG_PORT_EVENT;
                G_eldn.data.port = port;
#ifdef FRONTEND
                G_eldn.data.proc = 0;
#else  /* FRONTEND */
                G_eldn.data.proc = 1;
#endif /* FRONTEND */
                G_eldn.data.reason = ISP_NO_LOOP_ERROR;
                G_eldn.data.count = 0;
                /*
                 * Note: message is short, and L$send_packet copies into the MRP.
                 */
                MSC_LogMessageStack(&G_eldn, sizeof(G_eldn));

                /*
                 ** Set the flag indicating the port has been offline
                 ** long enough to be consider failed.
                 */
                ispofflfail |= bitPort;
            }
        }

        BIT_SET(ispPow, port);  /* Set the Power Up Wait bit */
    }
    /* Check if wait period from loop down until port reset is non-zero */
    else if (mpn.loopDownToNotify != 0 && (ispOnline & bitPort) == 0 &&
             (ispofflfail & bitPort) == 0)
    {
        retryCount = mpn.resetRetries;  /* Get the number of retries allowed */

        /* Wait specified time period from loop down until port reset */
        TaskSleepMS(mpn.loopDownToNotify);

        /*
         ** Check if retry count is non-zero,
         ** if time delay between reset and loop check is non-zero,
         ** and if the port is still offline.
         */
        while (retryCount > 0 && mpn.resetToNotify != 0 && (ispOnline & bitPort) == 0)
        {
            LOG_PORT_EVENT_PKT G_eldn;

            --retryCount;       /* Decrement the retry count */

            /* Send debug log entry to CCB */

            G_eldn.header.event = LOG_LPDN_RETRY;
            G_eldn.data.port = port;
            G_eldn.data.reason = ISP_LOOP_DOWN_RETRY;
#ifdef FRONTEND
            G_eldn.data.proc = 0;
#else  /* FRONTEND */
            G_eldn.data.proc = 1;
#endif /* FRONTEND */
            G_eldn.data.count = mpn.resetRetries - retryCount;
            /*
             * Note: message is short, and L$send_packet copies into the MRP.
             */
            MSC_LogMessageStack(&G_eldn, sizeof(G_eldn));

            /* Check if this port is still valid */

            if ((ispmap & bitPort) == 0)
            {
                break;
            }

            if ((resilk & bitPort) == 0)        /* Is port being reset */
            {
                if ((isprena & bitPort) != 0)   /* Is port enabled */
                {
                    /*
                     ** Attempt to clear up the loop down
                     ** by doing a soft reset of the Qlogic chip
                     */
                    ISP_ResetChip(port, ISP_RESET_INIT_OFFLINE);
                }
                else
                {
                    /* Port is not enabled. Quit loop event recovery */

                    break;
                }
            }

            /* Wait specified time period for the loop to come back up */

            TaskSleepMS(mpn.resetToNotify);
        }

        if ((ispOnline & bitPort) == 0) /* Check if port is still offline */
        {
            LOG_PORT_EVENT_PKT G_eldn;

            /*
             ** Send warning message to CCB. The CCB may take additional
             ** action at this time, such as failing this port.
             */
            G_eldn.header.event = LOG_PORT_EVENT;
            G_eldn.data.port = port;
#ifdef FRONTEND
            G_eldn.data.proc = 0;
#else  /* FRONTEND */
            G_eldn.data.proc = 1;
#endif /* FRONTEND */
            G_eldn.data.reason = ISP_LOOP_DOWN_RETRY_ERROR;
            G_eldn.data.count = mpn.resetRetries - retryCount;
            /*
             * Note: message is short, and L$send_packet copies into the MRP.
             */
            MSC_LogMessageStack(&G_eldn, sizeof(G_eldn));

            /*
             ** Set the flag indicating the port has been offline
             ** long enough to be consider failed.
             */
            ispofflfail |= bitPort;
        }
    }

    isplepcb[port] = NULL;      /* Clear loop event process pointer */
}


/**
******************************************************************************
**
**  @brief      isp_portFailureHandler
**
**  @param      a       - Not used.
**  @param      b       - Not used.
**  @param      port    - Port number
**
**  @return     none
**
******************************************************************************
**/
void isp_portFailureHandler(UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port)
{
    if (mpn.loopDownToReset != 0)       /* Check if port failure timer is set */
    {
        /* Wait specified time period following the port reset */

        TaskSleepMS(mpn.softResetPeriod);
    }

    /* Clear the error counters */

    ispprc[port] = 0;
    ispSysErr[port] = 0;

    isppfpcb[port] = NULL;      /* Clear port failure handler process pointer */
}


/**
******************************************************************************
**
**  @brief      Send notifications to the CCB for failed port(s) coming online.
**
**      Loop through the bit mask of previously failed ports that are now online
**      and send the event notifications to the CCB, which will take action
**      to unfail the ports.
**
**  @param      UINT32 port mask
**
**  @return     none
**
******************************************************************************
**/
void isp_reportOnlinePorts(UINT32 portEventBits)
{
    UINT32      port;

    for (port = 0; port < MAX_PORTS; ++port)    /* Check each port */
    {
        /* Generate a log event for each port indicated in the bit mask */

        if (BIT_TEST(portEventBits, port))
        {
            LOG_PORT_EVENT_PKT G_eldn;

            G_eldn.header.event = LOG_PORT_EVENT;
#ifdef FRONTEND
            G_eldn.data.proc = 0;
#else  /* FRONTEND */
            G_eldn.data.proc = 1;
#endif /* FRONTEND */
            G_eldn.data.reason = ISP_LOOP_UP;
            G_eldn.data.count = 0;
            G_eldn.data.port = port;
            /*
             * Note: message is short, and L$send_packet copies into the MRP.
             */
            MSC_LogMessageStack(&G_eldn, sizeof(G_eldn));
        }
    }
}


/**
******************************************************************************
**
**  @brief  Check port online
**
**      This routine checks a port, processing one that is online or
**      which have an online timer pending. When a port's online timer
**      expires, this event is aggregated with any other pending online events
**      to be reported as a group to the CCB, which then unfails them.
**
**  @param  port - Port number to check
**  @param  portEventBits - Pointer to port event bits to update
**  @param  portEventTimer - Pointer to port event timer to update
**
**  @return TRUE if still waiting to report this port online
**
******************************************************************************
**/
static UINT32 check_port_online(UINT8 port, UINT32 *portEventBits, UINT32 *portEventTimer)
{
    /*
     ** Check if port exists.
     ** Check if port is online.
     ** Check if timer is active.
     */
    if (isprev[port] == NULL || !BIT_TEST(ispOnline, port) || onlineTimer[port] == 0)
    {
        return FALSE;
    }

#ifdef FRONTEND
    /* For iSCSI control port, bypass the logic and report online */

    if (BIT_TEST(ispCp, port) && BIT_TEST(iscsimap, port))
        // if (BIT_TEST(iscsimap, port))
    {
        /*
         ** Clear the flag indicating the port has been online
         ** long enough to be consider good.
         */
        BIT_CLEAR(ispofflfail, port);

        /*
         ** Set bit to notify the CCB of this port online event.
         ** Start the event aggregation timer, if not running.
         */
        BIT_SET(*portEventBits, port);
        onlineTimer[port] = 0;
        *portEventTimer = 0;
        return TRUE;
    }

    /*
     ** Wait until cache is initialized before
     ** starting the count down timer.
     */
    if (BIT_TEST(K_ii.status, II_CINIT) == 0)
    {
        /* Cache not init yet, set the flag to continue this loop */

        return TRUE;
    }
#endif /* FRONTEND */

    /* Decrement and check if the online timer is still non-zero */

    if (--onlineTimer[port] != 0)
    {
        /* Timer hasn't expired - set flag to continue this loop */

        return TRUE;
    }

    /* Check for a failed port */

    if (BIT_TEST(ispofflfail, port) ||
#ifdef FRONTEND
        BIT_TEST(ispCp, port) ||
#endif /* FRONTEND */
        ispFailedPort[port] != FALSE)
    {
        /*
         ** Clear the flag indicating the port has been online
         ** long enough to be consider good.
         */
        BIT_CLEAR(ispofflfail, port);

        /*
         ** Set bit to notify the CCB of this port online event.
         ** Start the event aggregation timer, if not running.
         */
        BIT_SET(*portEventBits, port);
        if (*portEventTimer == 0)
        {
            *portEventTimer = 5;        /* 5 seconds */
        }
    }

#ifdef FRONTEND
    /*
     ** If this is not a control port, request DLM to check
     ** all its paths.
     */
#if FE_ISCSI_CODE
    if (BIT_TEST(ispCp, port) == 0 && BIT_TEST(iscsimap, port) == 0)
#else  /* FE_ISCSI_CODE */
    if (BIT_TEST(ispCp, port) == 0)
#endif /* FE_ISCSI_CODE */
    {
        DLM$PortReady();
    }
#endif /* FRONTEND */

    return FALSE;
}


/**
******************************************************************************
**
**  @brief  Notify the CCB when a failed port has been online for a while.
**
**      This routine loops through all existing ports, processing those online
**      ports which have online timers pending. When a port's online timer
**      expires, this event is aggregated with any other pending online events
**      to be reported as a group to the CCB, which then unfails them.
**      Any stragglers will be reported separately.
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
void isp_portOnlineHandler(void)
{
    UINT32      flag;
    UINT8       port;
    UINT32      portEventBits;  /* Bit mask for ports which came online */
    UINT32      portEventTimer; /* Timer to aggregate port online events */

    /*
     ** Initialize the bit mask for the ports which have come online.
     ** Zero the aggregation timer also.
     */
    portEventBits = 0;
    portEventTimer = 0;

    do
    {
        flag = FALSE;           /* Clear flag */

        /* Decrement port event timer, if it's running already */

        if (portEventTimer > 0)
        {
            --portEventTimer;
        }

        for (port = 0; port < MAX_PORTS; ++port)        /* Check each port */
        {
            flag |= check_port_online(port, &portEventBits, &portEventTimer);
        }

        /* Is flag set?  (Still work to do if so.) */

        if (flag)
        {
            if (portEventBits != 0 && portEventTimer == 0)
            {
                /*
                 ** Event timer expired.
                 ** Send log message(s) to CCB, to take action to unfail port(s).
                 ** Stragglers will be reported later, if/when they come online.
                 */
                isp_reportOnlinePorts(portEventBits);
                portEventBits = 0;
            }

            /* Wait one second */

            TaskSleepMS(1000);
        }
    } while (flag);

    if (portEventBits != 0)
    {
        /*
         ** All ports are now online.
         ** Send log message(s) to CCB, to take action to unfail port(s).
         */
        isp_reportOnlinePorts(portEventBits);
    }

    isponpcb = NULL;            /* Clear port failure handler process pointer */
}


/**
******************************************************************************
**
**  @brief      Issue an ISP mailbox command via an IOCB structure.
**
**  @param      port    - Port number
**
**  @return     status
**
******************************************************************************
**/
UINT32 isp_mailboxIOCB(UINT8 port, UINT32 lid UNUSED, QRP * qrp)
{
    UINT32      retValue = ERROR;
    ILT        *ilt;
    ISP2400_MBIOCB *iocb;
    UINT16      regBitmap;
    UINT32      i;
    UINT32      rqip;

    /*
     ** Get an ILT with wait. Get an ILT prior to an IOCB so no
     ** task switch may occur after an IOCB is obtained.
     */
    ilt = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */

    /* Get an IOCB for the request */

    iocb = (ISP2400_MBIOCB *)isp_get_iocb(port, &rqip);

    /* Did we sucessfully get an IOCB? */

    if (iocb)
    {
        ilt->ilt_normal.w0 = MBIOCB;    /* store command byte */
        ilt->ilt_normal.w0 |= port << 8; /* store Chip ID */
        ilt->ilt_normal.w1 = (UINT32)qrp;
        ilt->ilt_normal.w2 = ++ispSeq;
        ilt->misc = (UINT32)K_xpcb;     /* Store PCB */
        ilt->cr = QWComp;               /* Completion handler */
        ++ilt;                          /* Get next level of ILT */

        /* Initialize the Mailbox IOCB */

        iocb->entryType = MBIOCB;
        iocb->entryCount = 1;
        iocb->entryStatus = 0;
        iocb->sysdef = ispSeq;
        iocb->handle = (UINT32)ilt;

        regBitmap = qrp->iRegs; /* Get register bitmap */

        /* Zero out unused registers starting with the second register */

        for (i = 1; i < 12; ++i)
        {
            regBitmap >>= 1;    /* Shift bit map for next register */

            /* Check if the bit is set for this mailbox register */

            if ((regBitmap & 1) == 0)
            {
                qrp->imbr[i] = 0;
            }
        }

        /* Store input mailbox registers 0, 1, 2, 3, 6, ,7 ,9 and 10 */

        for (i = 0; i < 11; i++)
        {
            iocb->mbox[i] = qrp->imbr[i];
        }

        /* Submit IOCB by updating Request Queue IN pointer */

        update_rqip(port, rqip);

        ilt->isp_defs.isp_iocb_type = 0;    /* Set for isp_check_thread. */
        ilt->isp_defs.isp_timeout = 30; /* Set 30 second timeout in ILT */

        isp_thread_ilt(port, ilt);      /* Add this ILT to the active list */

        /*
         ** Set process to wait for signal and
         ** Wait until IOCB has completed.
         */
        TaskSetMyState(PCB_NOT_READY);
        TaskSwitch();

        retValue = ilt->ilt_normal.w0;

        --ilt;                  /* Move back one level */
    }

#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    put_ilt(ilt);               /* Release the ILT */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Process the completion of a Mailbox IOCB.
**
**      This function handles the receipt of a mailbox IOCB, signalling the
**      completion of the specified mailbox command. If the mailbox command
**      executed, the content of the outgoing mailbox registers are copied back
**      into the specified QRP.
**
**  @param      port    - Port number
**
**  @return     Status
**
******************************************************************************
**/
static void isp_processMailboxIOCB(UINT8 port
#ifdef PERF
                                   UNUSED
#endif                          /* PERF */
                                   , struct ISP2400_MBIOCB *iocb)
{
    QRP        *qrp;
    ILT        *ilt;
    UINT32      i;


    ilt = (ILT *)iocb->handle;  /* Get value of ILT from IOCB */
    if (!verify_iocbhandle(ilt))
    {
        ILT *tmpILT = (ILT*)bswap_32((UINT32)ilt);
        if (!verify_iocbhandle(tmpILT))
        {
            fprintf(stderr, "%s%s:%u %s nphandle is not an ILT (%p), processing as zero...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            ilt = 0;
        }
        else
        {
            fprintf(stderr, "%s%s:%u %s nphandle was byteswapped (%p), correcting...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            ilt = tmpILT;
        }
    }

    /* Check if ILT is defined */
    if (!ilt)
    {
        /* ILT in IOCB is invalid something is really broken stop before we
         * corrupt something
         */
        abort();
    }
    --ilt;                      /* Move back one level */

    if ((UINT8)ilt->ilt_normal.w0 != MBIOCB)
    {
        ilt->ilt_normal.w0 = ISP_ILT;   /* ILT in IOCB is invalid */
        return;
    }

    qrp = (QRP *)ilt->ilt_normal.w1;    /* Get Qrp from ILT */

    if (!qrp)                   /* If no QRP */
    {
        ilt->ilt_normal.w0 = ISP_QRP;   /* QRP in ILT is invalid */
        return;
    }

    for (i = 0; i < 11; i++)
    {
        qrp->ombr[i] = iocb->mbox[i];
    }

#ifndef PERF
    /* Log the failed mailbox command */

    if (qrp->ombr[0] != ISP_CMDC)
    {
        /* When mailbox command fails, send debug message to CCB */

        LOG_MB_FAILED_PKT embf;

        embf.header.event = LOG_MB_FAILED;
        embf.data.port = port;
#ifdef FRONTEND
        embf.data.proc = 0;
#else  /* FRONTEND */
        embf.data.proc = 1;
#endif /* FRONTEND */
        embf.data.iregs = qrp->iRegs;
        embf.data.oregs = qrp->oRegs;

        for (i = 0; i < dimension_of(embf.data.imbr); ++i)
        {
            embf.data.imbr[i] = qrp->imbr[i];
            embf.data.ombr[i] = qrp->ombr[i];
        }
#ifdef DEBUG_FLT_REC_ISP
        /* 0x58424D1F = 'XBM'<<8|0x1F */
        MSC_FlightRec(0x58424D1F, port,
                      *(UINT32 *)embf.data.imbr, *(UINT32 *)embf.data.ombr);
#endif /* DEBUG_FLT_REC_ISP */
        /* Note: message is short, and L$send_packet copies into the MRP */

        MSC_LogMessageStack(&embf, sizeof(embf));
    }
#endif /* PERF */

    ilt->ilt_normal.w0 = DEOK;
}


#ifdef BACKEND
/**
******************************************************************************
**
**  @brief  Returns true if port is in fabric mode
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
UINT32 ISP_IsFabricMode(UINT32 port)
{
    return BIT_TEST(ispfflags, port);
}
#endif /* BACKEND */


/**
******************************************************************************
**
**  @brief  login a 24-bit fabric ID into a port number managed by the ISP.
**
**      This routine is used to login a particular 24-bit fabric ID into
**      a port number managed by the ISP.
**
**      Change: Following additional status information is available in
**      Mailbox 2 when Mailbox 0 is 4005h and Mailbox 1 is 0004h:
**
**      00h = Command failed while issuing PDISC
**      01h = Command failed while waiting for PDISC response.
**      02h = Command failed while issuing PLOGI.
**      03h = Command failed while waiting for PLOGI response.
**      04h = Command failed while issuing PRLI.
**      05h = Command failed while waiting for PRLI response.
**      06h = Command failed while logged in. (This should not happen)
**      07h = Command failed. Port unavailable. PCB reinitialized due to LOGO.
**      08h = Command failed while issuing PRLO.
**      09h = Command failed while waiting for PRLO response.
**      0Ah = Command failed while issuing LOGO.
**      0Bh = Command failed while waiting for LOGO response.
**
**  @param  port - Port number
**  @param  lid - Loop ID to log in
**  @param  portID - 24-bit fabric port ID
**
**  @return ISP completion code
**                      - 4000h = ISP_CMDC = Successful command completion
**
******************************************************************************
**/

static UINT32 ISP_LoginFabricPortVPID(UINT8 port, UINT32 *lid, UINT32 portID, UINT8 vpid)
{
    UINT32      retValue;
    struct portlogiiocb_t *iocb;
    struct portlogiiocb_t *RespIOCB;
    ILT         *ilt;
    UINT32      rqip;

#if ISP_DEBUG_INFO
    fprintf(stderr, "%s: PORT %d portid %06X lid %04X\n", __func__, port, portID, *lid);
#endif /* ISP_DEBUG_INFO */

    if (BIT_TEST(ispfflags, port) == 0)
    {
        return ISP_NOSW;        /* Not fabric - return error */
    }
    ilt = get_ilt();
    RespIOCB = s_MallocC(sizeof(*iocb), __FILE__, __LINE__);
    iocb = (struct portlogiiocb_t *)isp_get_iocb(port,&rqip);
    if (iocb == NULL)
    {
        put_ilt(ilt);
        s_Free(RespIOCB, sizeof(*iocb), __FILE__, __LINE__);
        return ISP_CMDE;
    }

    iocb->entryType = PLOGXIOCB;
    iocb->entryCount = 1;
    iocb->lid = *lid;
    iocb->vpindex = vpid;
#ifdef BACKEND
    iocb->controlflags = 0x90;  /* only login if not already logged in this
                                 * gives the same behavior as the OPTNBIT */
    /* bit 7 enables control of some of the common service parameters
     ** specificaly 32-16 of word 1 to set the BB-credit bit.
     ** this bit shoudl be ignored by the drive according to the spec.
     ** unfortunately the FC to SATA card we use in the SBOD checks it and complains.
     ** the fc drives seem to properly ignore this bit
     */
    iocb->commonfeatures = 0x8800 << 16;        /* set Continuously increasing relative offset
                                                 ** and BB credit management
                                                 */
#else  /* BACKEND */
    iocb->controlflags = 0x10;  /* only login if not already logged in this
                                 * gives the same behavior as the OPTNBIT */
#endif /* BACKEND */
    iocb->portid0_15 = (UINT16)(portID & 0x0000ffff);
    iocb->portid16_23 = (UINT8)(portID >> 16);

    /* Execute the login */
    ilt++;
    iocb->handle = (UINT32)ilt;
    ilt--;
    retValue = exec_iocb_wait(port, ilt, rqip, (UINT16*)RespIOCB, 30);

    if (RespIOCB->status != 0 || (retValue & 0xFF) != 0)
    {
        if (RespIOCB->status != 0x31 && RespIOCB->commonfeatures != 0x1a)
        {
            fprintf(stderr, "%s: comp portid %06X lid %d retValue %08X status %04X error %08X io1 %08X\n",
                    __func__, portID, *lid, retValue, RespIOCB->status, RespIOCB->commonfeatures, RespIOCB->ioparameter1);
        }
    }

    if ((retValue & 0xFF) == 0)
    {
        /* If MB succeeds check IOCB status if error set code to trick upper layer */

        switch (RespIOCB->status)
        {
            case 0x00:
                break;

            case 0x28:
                retValue = ISP_CMDE | 0x00050000;
                break;

            case 0x29:
                retValue = ISP_CMDE;
                break;

            case 0x31:
                switch (RespIOCB->commonfeatures)
                {
                    case 0x01:
                    case 0x02:
                    case 0x03:
                    case 0x04:
                    case 0x05:
                        retValue = (RespIOCB->status << 16) | ISP_CMDE;
                        break;

                    case 0x18:
                        retValue = ISP_CMDE | 0x000D0000;
                        break;

                    case 0x1A:
                        retValue = ISP_PIU | (RespIOCB->ioparameter1 << 16);
                        *lid = RespIOCB->ioparameter1;      //parameter 1;
                        break;

                    case 0x1B:
                        retValue = ISP_LIU | (RespIOCB->ioparameter1 << 16);
                        *lid = RespIOCB->ioparameter1;      //parameter 1;
                        break;

                    case 0x1C:
                        retValue = ISP_AIU;
                        break;

                    case 0x1F:
                    case 0x07:
                    case 0x09:
                    case 0x0A:
                    case 0x19:
                    default:
                        retValue = ISP_CPE;
                        break;
                }
                break;

            default:
                retValue = ISP_CMDE;
        }
    }
    else
    {
        retValue = ISP_CPE;
    }
    s_Free(RespIOCB, sizeof(struct portlogiiocb_t), __FILE__, __LINE__);
    put_ilt(ilt);
    return retValue;
}                               /* ISP_LoginFabricPort */


/**
 ******************************************************************************
 **
 **  @brief      exec_iocb_wait - execute an IOCB and wait for it to finish.
 **
 **  @attention ILT usage
 **             lvl 0   w1  status - set to indicate ILT was aborted by us not the qlogic.
 **                     w2  pointer - to lvl 2 w0 - set by completion routine
 **                     cr  QWComp - wakes this routine up
 **                     misc PCB   - this PCB to wake up.
 **             lvl 1   w0  ilt type, technically just byte 0
 **                     w6 ILT timeout
 **  @param     port    - Port number
 **  @param     ilt     - ILT being used
 **  @param     rqip    - Request queue index
 **  @param     RespIOCB- Buffer to copy the response IOCB into
 **  @param     timeout - timeout to use for ILT. see check thread
 **
 **  @return    ISP_CMDE - 0x4005 Command failed, probably by isp reset.
 **  @return    ISP_CMDC - 0x4000 Command success, The IOCB can still have
 **                               failed it needs checked.
 **
 ** @attention  THIS FUNCTION MAY NOT TASK SWITCH UNTIL AFTER update_rqip HAS
 **             BEEN CALLED
 **
 ******************************************************************************
 **/

static UINT32 exec_iocb_wait(UINT8 port, ILT * ilt, UINT32 rqip, UINT16 *RespIOCB, UINT32 timeout)
{
    UINT32      i;

    /*  SMW- this is a check to see if we are creating a deadlock
     **  atiomonpcb is the one that gets the response and wakes this thread up
     **  causing obvious  problemms if we are asleep here
     **  wrap this in an #if DEBUG at some point
     */

    for (i = 0; i < MAX_PORTS; i++)
    {
        if (atiomonpcb[i] == K_xpcb)
        {
            fprintf(stderr, "%s:%d DEADLOCK detector !BOOM!", __func__, __LINE__);
            abort();
        }
    }

    ilt->ilt_normal.w1 = 0;     /* clear error code */
    ilt->ilt_normal.w2 = (UINT32)RespIOCB;
    ilt->misc = (UINT32)K_xpcb; /* Store PCB */
    ilt->cr = QWComp;           /* Completion handler */
    ilt++;                      /* goto level 1 */

    /* Submit IOCB by updating Request Queue IN pointer */

    update_rqip(port, rqip);

    ilt->isp_defs.isp_iocb_type = IOCB_WITH_WAIT_ILT_TYPE;  /* set type for ilt check thread */
    ilt->isp_defs.isp_timeout = timeout;        /* Set default timeout in ILT */

    isp_thread_ilt(port, ilt);  /* Add this ILT to the active list */

    /*
     ** Set process to wait for signal and
     ** Wait until IOCB has completed. QWCOMP does
     ** not check the state before setting to ready
     */
    TaskSetMyState(PCB_IOCB_WAIT);
    TaskSwitch();
    ilt--;                      // goto lvl 0

//     fprintf(stderr, "%s:%d SMW PORT %d ILT 0x%p\n",
//             __func__, __LINE__, port, ilt);

    if (ilt->ilt_normal.w1 != 0)
    {
        /* Fill in response iocb to prevent confusion */
        memset(RespIOCB, 0xff, IOCB_SIZE);

        return ISP_CMDE;
    }

    return ISP_CMDC;
}


/**
 ******************************************************************************
 **
 **  @brief  login a 24-bit fabric ID into a port number managed by the ISP.
 **
 **      This routine is used to login a particular 24-bit fabric ID into
 **      a port number managed by the ISP. Unlike ISP_LoginFabricPortVPID
 **     this routine uses the IOCB queues to issue the command allowing more
 **     parrallelism.
 **
 **      Change: Following additional status information is available in
 **      Mailbox 2 when Mailbox 0 is 4005h and Mailbox 1 is 0004h:
 **
 **      00h = Command failed while issuing PDISC
 **      01h = Command failed while waiting for PDISC response.
 **      02h = Command failed while issuing PLOGI.
 **      03h = Command failed while waiting for PLOGI response.
 **      04h = Command failed while issuing PRLI.
 **      05h = Command failed while waiting for PRLI response.
 **      06h = Command failed while logged in. (This should not happen)
 **      07h = Command failed. Port unavailable. PCB reinitialized due to LOGO.
 **      08h = Command failed while issuing PRLO.
 **      09h = Command failed while waiting for PRLO response.
 **      0Ah = Command failed while issuing LOGO.
 **      0Bh = Command failed while waiting for LOGO response.
 **
 **  @param  port - Port number
 **  @param  lid - Loop ID to log in
 **  @param  portID - 24-bit fabric port ID
 **
 **  @return ISP completion code
 **                      - 4000h = ISP_CMDC = Successful command completion
 **
 ******************************************************************************
 **/

UINT32 ISP_LoginFabricPortIOCB(UINT8 port, UINT32 *lid, UINT32 portID, UINT8 vpid)
{
    UINT32      retValue;
    struct portlogiiocb_t *iocb;
    UINT16     *RespIOCB;
    UINT32      rqip;
    ILT        *ilt;

    if (BIT_TEST(ispfflags, port) == 0)
    {
        return ISP_NOSW;        /* Not fabric - return error */
    }

    ilt = get_ilt();            /* get an ILT w/wait                */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    RespIOCB = s_MallocC(IOCB_SIZE, __FILE__, __LINE__);

//     fprintf(stderr, "%s:%d SMW PORT %d portid %06X lid %04X ILT 0x%p\n",
//             __func__, __LINE__, port, portID, *lid, ilt);

    iocb = (struct portlogiiocb_t *)isp_get_iocb(port, &rqip);

    if (iocb == NULL)
    {
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
        put_ilt(ilt);
        s_Free(RespIOCB, IOCB_SIZE, __FILE__, __LINE__);
        return ISP_CMDE;
    }

    memset(iocb, 0, IOCB_SIZE);
    iocb->entryType = PLOGXIOCB;
    iocb->entryCount = 1;
    iocb->lid = *lid;
    iocb->vpindex = vpid;
#ifdef BACKEND
    iocb->controlflags = 0x90;  /* only login if not already logged in this
                                 * gives the same behavior as the OPTNBIT */
    /* bit 7 enables control of some of the common service parameters
     ** specificaly 32-16 of word 1 to set the BB-credit bit.
     ** this bit shoudl be ignored by the drive according to the spec.
     ** unfortunately the FC to SATA card we use in the SBOD checks it and complains.
     ** the fc drives seem to properly ignore this bit
     */
    iocb->commonfeatures = 0x8800 << 16;        /* set Continuously increasing relative offset
                                                 ** and BB credit management
                                                 */
#else  /* BACKEND */
    iocb->controlflags = 0x10;  /* only login if not already logged in this
                                 * gives the same behavior as the OPTNBIT */
#endif /* BACKEND */
    iocb->portid0_15 = (UINT16)(portID & 0x0000ffff);
    iocb->portid16_23 = (UINT8)(portID >> 16);
    ilt++;
    iocb->handle = (UINT32)ilt;
    ilt--;
    retValue = exec_iocb_wait(port, ilt, rqip, RespIOCB, 30);
    iocb = (struct portlogiiocb_t *)RespIOCB;

    if (iocb->status != 0 || (retValue & 0xFF) != 0)
    {
        if (iocb->status != 0x31 && iocb->commonfeatures != 0x1a)
        {
            fprintf(stderr, "%s: comp portid %06X lid %d retValue %08X status %04X error %08X io1 %08X\n",
                    __func__, portID, *lid, retValue, iocb->status, iocb->commonfeatures,
                    iocb->ioparameter1);
        }
    }

    if ((retValue & 0xFF) == 0)
    {
        /* If MB succeeds check IOCB status if error set code to trick upper layer */

        switch (iocb->status)
        {
            case 0x00:
                break;

            case 0x28:
                retValue = ISP_CMDE | 0x00050000;
                break;

            case 0x29:
                retValue = ISP_CMDE;
                break;

            case 0x31:
                switch (iocb->commonfeatures)
                {
                    case 0x01:
                    case 0x02:
                    case 0x03:
                    case 0x04:
                    case 0x05:
                        retValue = (iocb->status << 16) | ISP_CMDE;
                        break;

                    case 0x18:
                        retValue = ISP_CMDE | 0x000D0000;
                        break;

                    case 0x1A:
                        retValue = ISP_PIU | (iocb->ioparameter1 << 16);
                        *lid = iocb->ioparameter1;      //parameter 1;
                        break;

                    case 0x1B:
                        retValue = ISP_LIU | (iocb->ioparameter1 << 16);
                        *lid = iocb->ioparameter1;      //parameter 1;
                        break;

                    case 0x1C:
                        retValue = ISP_AIU;
                        break;

                    case 0x1F:
                    case 0x07:
                    case 0x09:
                    case 0x0A:
                    case 0x19:
                    default:
                        retValue = ISP_CPE;
                        break;
                }
                break;

            default:
                retValue = ISP_CMDE;
        }
    }
    else
    {
        retValue = ISP_CPE;
    }

    s_Free(RespIOCB, IOCB_SIZE, __FILE__, __LINE__);
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    put_ilt(ilt);

    return retValue;
}


/**
 ******************************************************************************
 **
 **  @brief      ISP_LoginFabricPort
 **              shell function for back end calls to ISP_LoginFabricPortVPID
 **              to hide vpid
 **
 **  @param  port - Port number
 **  @param  lid - Loop ID to log in
 **  @param  portID - 24-bit fabric port ID
 **
 **  @return     UINT32  - 4000h = ISP_CMDC = Successful command completion
 **                        400Xh = error see ISP_LoginFabricPortVPID for detail
 **
 ******************************************************************************
 **/
UINT32 ISP_LoginFabricPort(UINT8 port, UINT32 *lid, UINT32 portID)
{
    return ISP_LoginFabricPortVPID(port, lid, portID, 0);
}


/**
******************************************************************************
**
**  @brief      ISP_Login
**
**  @param      port    - Port number
**  @param      lid     - Loop ID to log in
**
**  @return     UINT32  - ISP completion code (15:0) and add'l info (31:16)
**                        4000h = ISP_CMDC = Successful command completion
**                        4005h = ISP_CMDE, see add'l info
**
******************************************************************************
**/
UINT32 ISP_Login(UINT8 port, UINT32 lid)
{
    /* Check if fabric mode */

    if ((ispConnectionType[port] == FL_PORT && !isp_isdevicelocal(port, lid, 0)) ||
        ispConnectionType[port] == F_PORT)
    {
        /* Perform fabric login */
        return ISP_LoginFabricPort(port, &lid, portdb[port][lid].pid);
    }

    /* Perform loop login */

    return ISP_LoginLoopPort(port, lid, 0);
}


#ifdef BACKEND
/**
 ******************************************************************************
 **
 **  @brief      ISP_PDisc
 **
 **              This function performs a PDISC operation
 **
 **  @param      port     - Port number
 **
 **  @return
 **
 ******************************************************************************
 **/
UINT32 ISP_PDisc(UINT8 port, UINT32 handle)
{
    UINT32      retValue;
    PDB        *pPortDb = portdb[port] + handle;
    UINT32      alpa;
    struct portlogoiocb_t *iocb;
    UINT16     *RespIOCB;
    ISP2400_VPORT_ICB *pVPICB = (ISP2400_VPORT_ICB *)ispstr[port]->icbStr;
    UINT32      rqip;
    ILT        *ilt;

    alpa = pdbPidToALPA(pPortDb->pid);

    ilt = get_ilt();            /* get an ILT w/wait                */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    RespIOCB = s_MallocC(IOCB_SIZE, __FILE__, __LINE__);

    /* Get an IOCB for the request */

    iocb = (struct portlogoiocb_t *)isp_get_iocb(port, &rqip);

    if (iocb == NULL)
    {
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
        put_ilt(ilt);
        s_Free(RespIOCB, IOCB_SIZE, __FILE__, __LINE__);
        return ISP_CMDE;
    }
    memset(iocb, 0, IOCB_SIZE);
    iocb->entryType = PLOGXIOCB;
    iocb->entryCount = 1;
    iocb->lid = handle;
    iocb->controlflags = 0x02;  /* PDISC */
    iocb->portname = portid[port];
    iocb->vpindex = 0;
    iocb->portid0_15 = (UINT16)(alpa & 0xFFFF);
    iocb->portid16_23 = (UINT8)(alpa >> 16);
    iocb->portWWN = myhtonll(pVPICB->nicb.portWWN);

    ilt++;
    iocb->handle = (UINT32)ilt;
    ilt--;
    retValue = exec_iocb_wait(port, ilt, rqip, RespIOCB, 30);

    iocb = (struct portlogoiocb_t *)RespIOCB;
    if ((retValue & 0xFF) == 0)
    {
        /* If MB succeeds check IOCB status if error set code to trick upper layer */
        if (iocb->status)
        {
//             fprintf(stderr, "%s:%d port %d handle %04X status %04X\n",
//                     __func__, __LINE__, port, handle, iocb->status);
            retValue = ISP_CMDE;
        }
    }

    s_Free(RespIOCB, IOCB_SIZE, __FILE__, __LINE__);
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    put_ilt(ilt);
    return retValue;
}
#endif /* BACKEND */


/**
******************************************************************************
**
**  @brief      ISP_GetFWQueueDepth
**
**              This function calls isp_getResourceCounts for all targets and
**              uses the values returned to calculates the queue depth for
**              an HBA.
**
**  @param      port     - Port number
**
**  @return     Queue Depth of the controller
**
******************************************************************************
**/
UINT16 ISP_GetFWQueueDepth(UINT8 port)
{
    TAR        *pTar;
    UINT16      qDepth = 0;

    /* Traverse the target list for this port and add up the counts */
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        isp_getResourceCounts(port, pTar->entry);
        /*
         * 0xFE is what the resource counts get initialized to in isp.as
         * It should be defined as a constant, once we figure out if it
         * is right. It works for now.
         */
        qDepth += 0xFE - isprc[port].commandResourceCount;
    }
    return qDepth;
}                               /* ISP_GetFWQueueDepth */


/**
******************************************************************************
**
**  @brief      ISP_LogoutFabricPort
**
**      The Logout Fabric Port mailbox command logs out the specified
**      registered fabric port (Loop ID 0x81 - 0xFF) in the port database.
**      This command removes a port that de-registered from the name server
**      as reported by a change notification (RSCN) async event (0x8015).
**
**  @param      port     - Port number
**  @param      lid      - Loop ID to log out
**
**  @return     status
**
**  @attention  No check is made for the port being logged in and
**              no error is returned if no port was logged into this ID.
**
******************************************************************************
**/
UINT32 ISP_LogoutFabricPort(UINT8 port, UINT32 lid, UINT32 ourAlpa
#ifdef BACKEND
                            UNUSED
#endif                          /* BACKEND */
    )
{
    UINT32      retValue;
    PDB        *pPortDb = portdb[port] + lid;
    UINT32      alpa;
    struct portlogoiocb_t *iocb;
    UINT16     *RespIOCB;
    ISP2400_VPORT_ICB *pVPICB = (ISP2400_VPORT_ICB *)ispstr[port]->icbStr;
    UINT32      rqip;
    ILT        *ilt;

    /* Get an IOCB for the request */

    ilt = get_ilt();            /* get an ILT w/wait                */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    RespIOCB = s_MallocC(IOCB_SIZE, __FILE__, __LINE__);

    alpa = pdbPidToALPA(pPortDb->pid);

    iocb = (struct portlogoiocb_t *)isp_get_iocb(port, &rqip);
    if (!iocb)
    {
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
        put_ilt(ilt);
        s_Free(RespIOCB, IOCB_SIZE, __FILE__, __LINE__);
        return ISP_CMDE;
    }

    memset(iocb, 0, IOCB_SIZE);
    iocb->entryType = PLOGXIOCB;
    iocb->entryCount = 1;
    iocb->lid = lid;
    iocb->controlflags = 0x98;  /* LOGO + implicit logout + free handle(lid) */


#ifdef FRONTEND
    TAR        *pTar;

    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        if (pTar->portID == ourAlpa)
        {
            break;
        }
    }

    if (pTar)
    {
        iocb->portname = ourAlpa;
        iocb->vpindex = pTar->vpID;
    }
    else
    {
        iocb->portname = portid[port];
        iocb->vpindex = 0;
    }
#else  /* FRONTEND */
    iocb->portname = portid[port];
    iocb->vpindex = 0;
#endif /* FRONTEND */

    iocb->portid0_15 = (UINT16)(alpa & 0xFFFF);
    iocb->portid16_23 = (UINT8)(alpa >> 16);
    iocb->portWWN = myhtonll(pVPICB->nicb.portWWN);
//     fprintf(stderr, "%s port %d lid %04X ouralpa %06X vpid %d\n",
//              __func__, port, lid, alpa, iocb->vpindex);
    ilt++;
    iocb->handle = (UINT32)ilt;
    ilt--;
    retValue = exec_iocb_wait(port, ilt, rqip, RespIOCB, 30);

    iocb = (struct portlogoiocb_t *)RespIOCB;

    if ((retValue & 0xFF) == 0)
    {
        /* If MB succeeds check IOCB status if error set code to trick upper layer */
        if (iocb->status)
        {
            retValue = ISP_CMDE;
        }
    }

    s_Free(RespIOCB, IOCB_SIZE, __FILE__, __LINE__);
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    put_ilt(ilt);

    return retValue;
}



/**
******************************************************************************
**
**  @brief      Issues the CT information unit subcommand
**
**  @param      port - QLogic chip instance (0-3)
**  @param      ct_request - request buffer
**  @param      request_length - request len in bytes
**  @param      ct_response - buffer for response
**  @param      response_length - response buffer size in bytes
**  @param      vpindex - vpindex to use
**
**  @return     return status
**
******************************************************************************
**/
static UINT32 isp2400_send_SNS_sub(UINT32 port, void *ct_request, UINT32 request_length,
                               void *ct_response, UINT32 response_length, UINT8 vpindex)
{
    CTPASSTHRU_IOCB *pIOCB;
    UINT32      retValue;
    UINT32      rqip;
    ILT        *ilt;
    UINT16     *RespIOCB;

    ilt = get_ilt();            /* get an ILT w/wait                */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    RespIOCB = s_MallocC(IOCB_SIZE, __FILE__, __LINE__);

    /* There can be no possible task switches after an iocb has been allocated. */
    pIOCB = (CTPASSTHRU_IOCB *)isp_get_iocb(port, &rqip);
    if (!pIOCB)
    {
        return ISP_CMDE;
    }
    memset(pIOCB, 0, IOCB_SIZE);

    /* Populate CT PassThru IOCB */

    pIOCB->entryType = CT_PASSTHRU_IOCB;
    pIOCB->entryCount = 1;
    pIOCB->nportHandle = SNS_NPORT_HANDLE;
    pIOCB->cmdTimeout = 10;     /*30 RVISIT need to find optimum timeout value */
    pIOCB->cmddsdCount = 1;
    pIOCB->rspdsdCount = 1;
    pIOCB->vpindex = vpindex;

    pIOCB->rsptotalBytes = response_length;
    pIOCB->cmdtotalBytes = request_length;
    pIOCB->dsd0[0] = LI_GetPhysicalAddr((UINT32)ct_request);
    pIOCB->dsd0[1] = 0;
    pIOCB->dsd1[0] = LI_GetPhysicalAddr((UINT32)ct_response);
    pIOCB->dsd1[1] = 0;
    pIOCB->dsd0Length = request_length;
    pIOCB->dsd1Length = response_length;

    /* Execute the CT PassThru IOCB */

    ilt++;
    pIOCB->iocbHandle = (UINT32)ilt;
    ilt--;
    retValue = exec_iocb_wait(port, ilt, rqip, RespIOCB, 30);

    pIOCB = (CTPASSTHRU_IOCB *)RespIOCB;

//    fprintf(stderr, "%s port %d retValue %04X pIOCB->cmplstatus %04X \n",
//           __func__, port, retValue, pIOCB->cmplstatus);

    /* modify return code */
    if (pIOCB->cmplstatus != 0 || (retValue & 0xFF) != 0)
    {
        if ((retValue & 0xFF) == 0)
        {
            /* If MB succeeds check IOCB status if error set code to trick upper layer */

            switch (pIOCB->cmplstatus)
            {
                case 0x02:
                    retValue = ISP_CMDE;
                    break;

                case 0x06:
                    retValue = ISP_CMDE;
                    break;

                case 0x07:
                    retValue = ISP_CMDE;
                    break;

                case 0x15:
                    retValue = GOOD;
                    break;

                case 0x28:
                    retValue = ISP_CMDE | 0x00050000;
                    break;

                case 0x29:
                    retValue = ISP_NLI;
                    break;

                case 0x2A:
                    retValue = ISP_CMDE;
                    break;

                case 0x2C:
                    retValue = ISP_CMDE;
                    break;

                default:
                    fprintf(stderr, "%s: Unknown status = %04x, port %d\n",
                            __func__, pIOCB->cmplstatus, port);
            }
        }
        else
        {
            retValue = ISP_CMDE;
        }
    }
    else
    {
        retValue = GOOD;
    }

    s_Free(RespIOCB, IOCB_SIZE, __FILE__, __LINE__);
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    put_ilt(ilt);
    return retValue;
}

/**
 ******************************************************************************
 **
 **  @brief      Issues the CT information unit subcommand
 **              will attempt to relogin to the name server once if NLI error
 **
 **  @param      port - QLogic chip instance (0-3)
 **  @param      ct_request - request buffer
 **  @param      request_length - request len in bytes
 **  @param      ct_response - buffer for response
 **  @param      response_length - response buffer size in bytes
 **  @param      vpindex - vpindex to use
 **
 **  @return     return status
 **
 ******************************************************************************
 **/

static UINT32 isp2400_send_SNS(UINT32 port, void *ct_request, UINT32 request_length,
                               void *ct_response, UINT32 response_length, UINT8 vpindex)

{
    UINT32 retValue;
    UINT32 fabrichandle;
    retValue = isp2400_send_SNS_sub(port, ct_request, request_length, ct_response, response_length, vpindex);

    if (retValue == ISP_NLI)
    {
        fabrichandle = SNS_NPORT_HANDLE;
        retValue = ISP_LoginFabricPortIOCB(port, &fabrichandle, 0xFFFFFC, 0) & 0xffff;
        if ( retValue == ISP_CMDC)
        {
            //clear response just in case
            memset(ct_response, 0, response_length);
            retValue = isp2400_send_SNS_sub(port, ct_request, request_length, ct_response, response_length, vpindex);
        }
        else
        {
            retValue = ISP_CMDE;
        }
    }
    return retValue;
}
/* CT COMMUNICATION WITH THE FE FABRIC */
/**
******************************************************************************
**
**  @brief      Issues the Send RFT to the Fabric
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      fc4Type - FC4 type
**  @param      portId  - 24bit portID
**  @param      vpindex - Virtual Port Index
**
**  @return     return status
**
******************************************************************************
**/
UINT32 isp2400_sendctRFT_ID(UINT8 port, UINT32 fc4Type, UINT32 portID, UINT32 vpindex)
{
    UINT32      retValue = GOOD;

    CT_RFTID_REQ *pRidreq;
    CT_RFTID_RESP *pRidresp;

    /* Allocate the Memory for the structures */

    pRidreq = s_MallocC(sizeof(*pRidreq), __FILE__, __LINE__);
    pRidresp = s_MallocC(sizeof(*pRidresp), __FILE__, __LINE__);

    /* Populate the RFTID req header */

    pRidreq->ctreq_hdr.gsRevision = FC_GS_REVISION;
    pRidreq->ctreq_hdr.gsType = FC_GS_TYPE_DIRECTORY_SERVICE;
    pRidreq->ctreq_hdr.gsSubType = FC_GS_SUBTYPE_NAME_SERVICE;
    pRidreq->ctreq_hdr.ct_command = htons(RFT_ID);
    pRidreq->ctreq_hdr.ct_residual_size = 0;

    /* Populate the RFTID request */

    pRidreq->portid = htonl(portID);
    pRidreq->fc4Types[0] = htonl(fc4Type);

    /* Execute the CT PassThru IOCB */

    retValue = isp2400_send_SNS(port, pRidreq, RFTID_REQ_LEN,
                                pRidresp, RFTID_RESP_LEN, vpindex);

    if (retValue != GOOD)
    {
        fprintf(stderr, "%s: ERROR portid %06X retValue %08X ct_status %04X\n",
                __func__, portID, retValue, pRidresp->ctrsp_hdr.ct_response_code);
    }

    /* Free the buffers */

    s_Free(pRidreq, sizeof(*pRidreq), __FILE__, __LINE__);
    s_Free(pRidresp, sizeof(*pRidresp), __FILE__, __LINE__);

    return retValue;
}


/**
******************************************************************************
**
**  @brief  Issues the GANXT to the Fabric
**
**  @param  port - QLogic chip instance (0-3)
**  @param  buffer - Response buffer(filled by fabric)
**  @param  portId - 24bit portID
**
**  @return return status
**
******************************************************************************
**/
UINT32 isp2400_sendctGAN(UINT16 port, UINT32 buffer, UINT32 portID)
{
    UINT32      retValue = ISP_CMDC;
    CT_GAN_REQ *pGanreq;
    CT_GAN_RESP *pGanresp = (CT_GAN_RESP *)buffer;

    //fprintf(stderr, "%s with port %X, buffer %X portID %X\n",
    //        __func__, port, buffer, portID);

    pGanreq = s_MallocC(sizeof(*pGanreq), __FILE__, __LINE__);       /* Get Memory for the structures */

    memset(pGanresp, 0, sizeof(*pGanresp));     /* Clear the response buffer */

    /* Populate the GAN req header */

    pGanreq->ctreq_hdr.gsRevision = FC_GS_REVISION;
    pGanreq->ctreq_hdr.gsType = FC_GS_TYPE_DIRECTORY_SERVICE;
    pGanreq->ctreq_hdr.gsSubType = FC_GS_SUBTYPE_NAME_SERVICE;
    pGanreq->ctreq_hdr.ct_command = htons(GAN_NXT);
    pGanreq->ctreq_hdr.ct_residual_size = htons((GAN_NXT_RSP_LEN - 16) / 4);

    pGanreq->portid = htonl(portID);    /* Populate the GAN request */

    /* Execute the CT PassThru IOCB */

    retValue = isp2400_send_SNS(port, pGanreq, GAN_NXT_REQ_LEN,
                                pGanresp, GAN_NXT_RSP_LEN, 0);

    /* this is silly but ISP$get_all_next expects different
     * returns than other SNS commands
     */
    if (retValue == GOOD)
    {
        retValue = ISP_CMDC;
    }

#if ISP_DEBUG_INFO
    if (retValue != ISP_CMDC)
    {
        fprintf(stderr, "%s:  ERROR portid %06X  retValue %08X ct_status %04X\n",
                __func__, portID, retValue, pGanresp->ctrsp_hdr.ct_response_code);
    }
#endif /* ISP_DEBUG_INFO */

    /* Free the buffer */
    s_Free(pGanreq, sizeof(*pGanreq), __FILE__, __LINE__);

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Issues the "Remove All" SNS subcommand
**
**  @param      port - QLogic chip instance (0-3)
**  @param      pTar - pointer to Tar structure
**
**  @return     return status
**
******************************************************************************
**/
UINT32 isp_removeAll(UINT8 port, TAR * pTar)
{
    UINT16      retValue;
    CT_DA_ID_REQ *ct_request;
    CT_DA_ID_RESP *respBuffer;

    respBuffer = s_MallocC(sizeof(*respBuffer), __FILE__, __LINE__);
    ct_request = s_MallocC(sizeof(*ct_request), __FILE__, __LINE__);

    /* Populate the GPN_ID req header */

    ct_request->ctreq_hdr.gsRevision = FC_GS_REVISION;
    ct_request->ctreq_hdr.gsType = FC_GS_TYPE_DIRECTORY_SERVICE;
    ct_request->ctreq_hdr.gsSubType = FC_GS_SUBTYPE_NAME_SERVICE;
    ct_request->ctreq_hdr.ct_command = htons(DA_ID);
    ct_request->ctreq_hdr.ct_residual_size = 0;
    ct_request->portid = pTar->portID;

    /* Send command to Simple Name Server */

    retValue = isp2400_send_SNS(port, ct_request, sizeof(*ct_request),
                                respBuffer, sizeof(*respBuffer), pTar->vpID);
    s_Free(ct_request, sizeof(*ct_request), __FILE__, __LINE__);

    /* If GOOD return value, check for a FS_ACC response code */

    if (retValue == GOOD && respBuffer->ctrsp_hdr.ct_response_code != FS_ACC)
    {
        retValue = respBuffer->ctrsp_hdr.ct_response_code;
    }

    s_Free(respBuffer, sizeof(*respBuffer), __FILE__, __LINE__);      /* Release memory used */

    return retValue;
}


/**
 ******************************************************************************
 **
 **  @brief      isp_GidNN
 **
 **              This function issues the 'Get Port Identifier from node name'
 **              GPN_ID command to the Simple Name Server.
 **              - It turns a 64 bit nodetname into an ALPA
 **
 **  @param      port     - Port number
 **  @param      NodeName - node name
 **  @param      portId   - Pointer to copy returned alpa to
 **
 **  @return     GOOD - success
 **
 ******************************************************************************
 **/
UINT32 isp_GidNN(UINT8 port, UINT64 NodeName, UINT32 *portId)
{
    UINT16      retValue;
    CT_GID_NN_REQ *ct_request;
    CT_GID_NN_RESP *respBuffer;

    respBuffer = s_MallocC(sizeof(*respBuffer), __FILE__, __LINE__);
    ct_request = s_MallocC(sizeof(*ct_request), __FILE__, __LINE__);

    /* Populate the GPN_ID req header */

    ct_request->ctreq_hdr.gsRevision = FC_GS_REVISION;
    ct_request->ctreq_hdr.gsType = FC_GS_TYPE_DIRECTORY_SERVICE;
    ct_request->ctreq_hdr.gsSubType = FC_GS_SUBTYPE_NAME_SERVICE;
    ct_request->ctreq_hdr.ct_command = htons(GID_NN);
    ct_request->ctreq_hdr.ct_residual_size = 0;
    ct_request->NodeName = NodeName;

    /* Send command to Simple Name Server */

    retValue = isp2400_send_SNS(port, ct_request, sizeof(*ct_request),
                                respBuffer, sizeof(*respBuffer), 0);
    s_Free(ct_request, sizeof(*ct_request), __FILE__, __LINE__);

    /* If GOOD return value, check for a FS_ACC response code */

    if (retValue == GOOD && respBuffer->ctrsp_hdr.ct_response_code != FS_ACC)
    {
        retValue = respBuffer->ctrsp_hdr.ct_response_code;
#if ISP_DEBUG_INFO
        fprintf(stderr, "%s:%d port %d ct_reason_code %02X ct_reason_code_expln %02x\n",
                __func__, __LINE__, port, respBuffer->ctrsp_hdr.ct_reason_code,
                respBuffer->ctrsp_hdr.ct_reason_code_expln);
#endif
    }
    else if (retValue == GOOD)
    {
        /*
         ** Get port ID from Response Buffer. We take the first one if there is
         ** more than one we are mis zoned on the BE
         */
        *portId = bswap_32(respBuffer->portid[0] & 0xFFFFFF00);
        if ((respBuffer->portid[0] & 0x80) == 0)
        {
            fprintf(stderr,
                    "%s:%d port %d multiple paths to NodeName %016llX are we zoned wrong??\n",
                    __func__, __LINE__, port, NodeName);
        }
    }

    s_Free(respBuffer, sizeof(*respBuffer), __FILE__, __LINE__);      /* Release memory used */

    return retValue;
}


/**
 ******************************************************************************
 **
 **  @brief      isp_GidPN
 **
 **              This function issues the 'Get Port Identifier'
 **              GPN_ID command to the Simple Name Server.
 **              - It turns a 64 bit portname into an ALPA
 **
 **  @param      port     - Port number
 **  @param      portName - Port name
 **  @param      portId   - Pointer to copy returned alpa to
 **
 **  @return     GOOD - success
 **
 ******************************************************************************
 **/
UINT32 isp_GidPN(UINT8 port, UINT64 portName, UINT32 *portId)
{
    UINT16      retValue;
    CT_GID_PN_REQ *ct_request;
    CT_GID_PN_RESP *respBuffer;

    respBuffer = s_MallocC(sizeof(*respBuffer), __FILE__, __LINE__);
    ct_request = s_MallocC(sizeof(*ct_request), __FILE__, __LINE__);

    /* Populate the GPN_ID req header */

    ct_request->ctreq_hdr.gsRevision = FC_GS_REVISION;
    ct_request->ctreq_hdr.gsType = FC_GS_TYPE_DIRECTORY_SERVICE;
    ct_request->ctreq_hdr.gsSubType = FC_GS_SUBTYPE_NAME_SERVICE;
    ct_request->ctreq_hdr.ct_command = htons(GID_PN);
    ct_request->ctreq_hdr.ct_residual_size = 0;
    ct_request->portName = portName;

    /* Send command to Simple Name Server */

    retValue = isp2400_send_SNS(port, ct_request, sizeof(*ct_request),
                                respBuffer, sizeof(*respBuffer), 0);
    s_Free(ct_request, sizeof(*ct_request), __FILE__, __LINE__);

    /* If GOOD return value, check for a FS_ACC response code */

    if (retValue == GOOD && respBuffer->ctrsp_hdr.ct_response_code != FS_ACC)
    {
        retValue = respBuffer->ctrsp_hdr.ct_response_code;
    }
    else if (retValue == GOOD)
    {
        *portId = respBuffer->portid;
    }

    s_Free(respBuffer, sizeof(*respBuffer), __FILE__, __LINE__);      /* Release memory used */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      isp_gpnId
**
**              This function issues the 'Get Port Name'
**              GPN_ID command to the Simple Name Server.
**
**  @param      port    - Port number
**
**  @return     status
**
******************************************************************************
**/
UINT32 isp_gpnId(UINT8 port, UINT32 portId, UINT64 *portName)
{
    UINT16      retValue;
    CT_GPN_ID_REQ *ct_request;
    CT_GPN_ID_RESP *respBuffer;

    respBuffer = s_MallocC(sizeof(*respBuffer), __FILE__, __LINE__);
    ct_request = s_MallocC(sizeof(*ct_request), __FILE__, __LINE__);

    /* Populate the GPN_ID req header */

    ct_request->ctreq_hdr.gsRevision = FC_GS_REVISION;
    ct_request->ctreq_hdr.gsType = FC_GS_TYPE_DIRECTORY_SERVICE;
    ct_request->ctreq_hdr.gsSubType = FC_GS_SUBTYPE_NAME_SERVICE;
    ct_request->ctreq_hdr.ct_command = htons(GPN_ID);
    ct_request->ctreq_hdr.ct_residual_size = 0;
    ct_request->portid = htonl(portId);

    /* Send command to Simple Name Server */

    retValue = isp2400_send_SNS(port, ct_request, sizeof(*ct_request),
                                respBuffer, sizeof(*respBuffer), 0);
    s_Free(ct_request, sizeof(*ct_request), __FILE__, __LINE__);

    /* If GOOD return value, check for a FS_ACC response code */

    if (retValue == GOOD && respBuffer->ctrsp_hdr.ct_response_code != FS_ACC)
    {
        retValue = respBuffer->ctrsp_hdr.ct_response_code;
    }
    else if (retValue == GOOD)
    {
        *portName = respBuffer->portName;
    }

    s_Free(respBuffer, sizeof(*respBuffer), __FILE__, __LINE__);      /* Release memory used */

    return retValue;
}


#ifdef BACKEND

/**
******************************************************************************
**
**  @brief      isp_gnnId
**
**              This function issues the 'Get Node Name'
**              GNN_ID command to the Simple Name Server.
**
**  @param      port    - Port number
**
******************************************************************************
**/
UINT32 isp_gnnId(UINT8 port, UINT32 portId, UINT64 *nodeName)
{
    UINT16      retValue;
    CT_GNN_ID_REQ *ct_request;
    CT_GNN_ID_RESP *respBuffer;

    respBuffer = s_MallocC(sizeof(*respBuffer), __FILE__, __LINE__);
    ct_request = s_MallocC(sizeof(*ct_request), __FILE__, __LINE__);

    /* Populate the GPN_ID req header */

    ct_request->ctreq_hdr.gsRevision = FC_GS_REVISION;
    ct_request->ctreq_hdr.gsType = FC_GS_TYPE_DIRECTORY_SERVICE;
    ct_request->ctreq_hdr.gsSubType = FC_GS_SUBTYPE_NAME_SERVICE;
    ct_request->ctreq_hdr.ct_command = htons(GNN_ID);
    ct_request->ctreq_hdr.ct_residual_size = 0;
    ct_request->portid = htonl(portId);

    /* Send command to Simple Name Server */

    retValue = isp2400_send_SNS(port, ct_request, sizeof(*ct_request),
                                respBuffer, sizeof(*respBuffer), 0);
    s_Free(ct_request, sizeof(*ct_request), __FILE__, __LINE__);

    /* If GOOD return value, check for a FS_ACC response code */

    if (retValue == GOOD && respBuffer->ctrsp_hdr.ct_response_code != FS_ACC)
    {
        retValue = respBuffer->ctrsp_hdr.ct_response_code;
        fprintf(stderr, "%s:%d port %d ct_reason_code %02X ct_reason_code_expln %02x\n",
                __func__, __LINE__, port, respBuffer->ctrsp_hdr.ct_reason_code,
                respBuffer->ctrsp_hdr.ct_reason_code_expln);
    }
    else
    {
        *nodeName = respBuffer->nodeName;
    }

    s_Free(respBuffer, sizeof(*respBuffer), __FILE__, __LINE__);      /* Release memory used */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      isp_gnnFt
**
**              This function issues the 'Get Port ID and Node Name'
**              GNN_FT command to the Simple Name Server.
**
**  @param      port     - QLogic chip instance ordinal (0-3).
**
**  @return     none
**
******************************************************************************
**/
UINT32 isp_gnnFt(UINT8 port)
{
    UINT16      retValue;
    UINT32      count;
    UINT32      i;
    UINT32      portId;
    struct nst_t *nst;
    CT_GNN_FT_REQ *ct_request;
    CT_GNN_FT_RESP *respBuffer;

//    fprintf(stderr, "%s:%d port %d \n", __func__, __LINE__, port);
    respBuffer = s_MallocC(sizeof(*respBuffer), __FILE__, __LINE__);
    ct_request = s_MallocC(sizeof(*ct_request), __FILE__, __LINE__);

    /* Populate the GPN_ID req header */

    ct_request->ctreq_hdr.gsRevision = FC_GS_REVISION;
    ct_request->ctreq_hdr.gsType = FC_GS_TYPE_DIRECTORY_SERVICE;
    ct_request->ctreq_hdr.gsSubType = FC_GS_SUBTYPE_NAME_SERVICE;
    ct_request->ctreq_hdr.ct_command = htons(GNN_FT);

    /*
     * Actually set maximum amount of data we can get since it might
     * exceed our buffer this is the size of the respBuffer minus the
     * ct header in 32 bit words
     */

    ct_request->ctreq_hdr.ct_residual_size = htons((sizeof(*respBuffer) - sizeof(CTIU_PREAMBLE)) / 4);
    ct_request->fc4Protocol = 8;

    /* Send command to Simple Name Server */

    retValue = isp2400_send_SNS(port, ct_request, sizeof(*ct_request),
                                respBuffer, sizeof(*respBuffer), 0);
    s_Free(ct_request, sizeof(*ct_request), __FILE__, __LINE__);

    /* If GOOD return value, check for a FS_ACC response code */

    if (retValue == GOOD && respBuffer->ctrsp_hdr.ct_response_code != FS_ACC)
    {
        retValue = respBuffer->ctrsp_hdr.ct_response_code;
    }
    else if (retValue == GOOD)
    {
        if (fabNameServerTable[port] != NULL)
        {
            /* Find the devices that currently exist in the table */

            for (i = 0; i < fabNameServerCount[port]; ++i)
            {
                nst = &fabNameServerTable[port][i];
                if (nst->lid < NO_LID)
                {
                    nst->status = 0x9999;
                }
            }

            /* Determine the number of entries */

            for (count = 0; count < GNN_MAX; ++count)
            {
                /* Get port ID from Response Buffer */

                portId = bswap_32(respBuffer->device[count].portId & 0xFFFFFF00);

                /* Find the devices that currently exist in the table */

                for (i = 0; i < fabNameServerCount[port]; ++i)
                {
                    nst = &fabNameServerTable[port][i];
                    if (nst->portId == portId)
                    {
                        if (nst->nodeName == respBuffer->device[count].nodeName)
                        {
                            nst->status = 0x2222;
                        }
                        break;
                    }
                }

                /*
                 ** Check for bit 7 of the control field set which
                 ** indicates the last entry received.
                 */
                if ((respBuffer->device[count].portId & 0x80) == 0x80)
                {
                    ++count;    /* Increment the count for the last device */
                    break;
                }
            }

            /*
             * fprintf(stderr, "%s:%d payload port %d \n",
             * __func__, __LINE__, port);
             * for (i = 0; i < count; ++i)
             * {
             * fprintf(stderr, "    NN %016llX Portid %06X \n",
             * respBuffer->device[i].nodeName,
             * respBuffer->device[i].portId);
             * }
             */

            /*
             ** Was the response buffer truncated?
             **         ?????  HOW DO WE HANDLE THIS ?????
             */
            if (respBuffer->ctrsp_hdr.ct_residual_size != 0)
            {
                count = GNN_MAX;
            }

            /* Find the devices that disappeared */

            for (i = 0; i < fabNameServerCount[port]; ++i)
            {
                /*
                 ** Check for the state that indicates the device
                 ** was not found in the response buffer.
                 */
                nst = &fabNameServerTable[port][i];
                if (nst->status == 0x9999)
                {
                    if (nst->lid < NO_LID)
                    {
                        /* Issue a fabric logout to this Port ID */
                        nst->status = ISP_LogoutFabricPort(port, nst->lid, portid[port]);
                        if (nst->status == ISP_CMDC)
                        {
                            /*
                             ** This LID is no longer is use.
                             ** Return the LID to the unused pool.
                             */
                            FAB_putLid(port, nst->lid);
                            nst->lid = NO_LID;
                        }
#ifdef DEBUG_FLIGHTREC_FD
                        MSC_FlightRec(0xE726, port, 0, nst->portId);
#endif /* DEBUG_FLIGHTREC_FD */
                        nst->status = 0x3333;
                    }
                }
            }

            if (sizeof(struct nst_t) * count > fabNameServerSize[port])
            {
                /*
                 ** Release the memory used by the name server database.
                 ** A larger table needs to be allocated.
                 */
                s_Free(fabNameServerTable[port], fabNameServerSize[port], __FILE__, __LINE__);
                fabNameServerTable[port] = NULL;
            }
            else
            {
                /* Clear the entire name server database */

                memset(fabNameServerTable[port], 0, (signed)fabNameServerSize[port]);
            }
        }

        /* Was the response buffer truncated? */

        else if (respBuffer->ctrsp_hdr.ct_residual_size != 0)
        {
            /*
             ** Count is set to the maximum when a residual size exists.
             ** The last entry in the buffer will not have bit 7 of
             ** the control field set. Indicate to the caller more
             ** entries exist. ????
             */
            count = GNN_MAX;
        }
        else
        {
            /* Determine the number of entries */

            for (count = 0; count < GNN_MAX; ++count)
            {
                /*
                 ** Check for bit 7 of the control field set which
                 ** indicates the last entry received.
                 */
                if ((respBuffer->device[count].portId & 0x80) == 0x80)
                {
                    ++count;    /* Increment the count for the last device */
                    break;
                }
            }
        }

        if (fabNameServerTable[port] == NULL)
        {
            /* Allocate a name server table */

            fabNameServerSize[port] = sizeof(struct nst_t) * count;
            fabNameServerTable[port] = s_MallocC(fabNameServerSize[port], __FILE__, __LINE__);
        }

        /* Store the number of entries and fill in the name server database */

        fabNameServerCount[port] = count;
        for (i = 0; i < count; ++i)
        {
            nst = &fabNameServerTable[port][i];
            nst->portId = bswap_32(respBuffer->device[i].portId & 0xFFFFFF00);
            nst->nodeName = respBuffer->device[i].nodeName;
            nst->lid = NO_LID;
            nst->status = 0x1111;
        }
    }

    s_Free(respBuffer, sizeof(*respBuffer), __FILE__, __LINE__);      /* Release memory used */

    return retValue;
}


/**
******************************************************************************
**
**  @brief      isp_gidPt
**
**              This function issues the 'Get Port ID 4'
**              GID_PT command to the Simple Name Server.
**
**  @param      port    - Port number
**  @param      portType - ??
**
**  @return     none
**
******************************************************************************
**/
UINT32 isp_gidPt(UINT8 port, UINT32 portType)
{
    UINT16      retValue;
    CT_GID_PT_REQ *ct_request;
    CT_GID_PT_RESP *respBuffer;

    respBuffer = s_MallocC(sizeof(*respBuffer), __FILE__, __LINE__);
    ct_request = s_MallocC(sizeof(*ct_request), __FILE__, __LINE__);

    /* Populate the GPN_ID req header */

    ct_request->ctreq_hdr.gsRevision = FC_GS_REVISION;
    ct_request->ctreq_hdr.gsType = FC_GS_TYPE_DIRECTORY_SERVICE;
    ct_request->ctreq_hdr.gsSubType = FC_GS_SUBTYPE_NAME_SERVICE;
    ct_request->ctreq_hdr.ct_command = htons(GID_PT);
    ct_request->ctreq_hdr.ct_residual_size = 0;
    ct_request->portType = (UINT8)portType;

    /* Send command to Simple Name Server */

    retValue = isp2400_send_SNS(port, ct_request, sizeof(*ct_request),
                                respBuffer, sizeof(*respBuffer), 0);
    s_Free(ct_request, sizeof(*ct_request), __FILE__, __LINE__);

    /* If GOOD return value, check for a FS_ACC response code */

    if (retValue == GOOD && respBuffer->ctrsp_hdr.ct_response_code != FS_ACC)
    {
        retValue = respBuffer->ctrsp_hdr.ct_response_code;
    }

    s_Free(respBuffer, sizeof(*respBuffer), __FILE__, __LINE__);      /* Release memory used */

    return retValue;
}                               /* isp_gidPt */
#endif /* BACKEND */


/**
******************************************************************************
**
**  @brief      isp2400_ReadGPIOD
**
**              This function returns the value of the ISP GPIOD register.
**
**  @param      port   - QLogic chip instance ordinal (0-3).
**
**  @return     UINT32 - GPIOD register
**
**  @attention  CAUTION:  Do not invoke this routine while the ISP RISC proc
**                        is operational - this can cause collisions with the
**                        resident ISP firmware resulting in laser degradation
**                        and/or other unpredictable and undesireable results!
**
******************************************************************************
**/
UINT32 isp2400_ReadGPIOD(UINT8 port)
{
    UINT32      temp24GPIOE;
    UINT32      temp24GPIOD;
    struct ISP_2400 *pISP24base = ispstr[port]->baseAd;

    temp24GPIOE = pISP24base->gpIOE;
    if (temp24GPIOE != 0)
    {
        pISP24base->gpIOE = 0;
    }

    temp24GPIOD = pISP24base->gpIOD;

    if (temp24GPIOE != 0)
    {
        pISP24base->gpIOE = temp24GPIOE;
    }

    return temp24GPIOD;
}                               /* isp2400_ReadGPIOD */


/**
***************************************************************************
**
**  @brief  Interrupt Service Routine for 2400 HBA
**
**  @param  port - QLogic chip instance (0-3)
**
**  @return 0=GOOD, all else is an error.
**
******************************************************************************
**/
void ISP2400_IntrServiceRoutine(UINT32 port)
{
    UINT32      risc2host_status;
    UINT8       risc_int_status;
    UINT32      hccr;
    struct ISP_2400 *isp24 = ispstr[port]->baseAd;
    int         reqQdiff;

    risc2host_status = isp24->r2HStat;
    if (BIT_TEST(isprqwt, port))
    {
        reqQdiff = isp24->reqQOP - isp24->reqQIP;
        if (reqQdiff < 0)
        {
            reqQdiff += ISP_REQ_QUE_SIZE;
        }
        if (reqQdiff != 1)
        {
            BIT_CLEAR(isprqwt, port);
            TaskReadyByState(PCB_QLOGIC_WAIT + port);
        }
    }


    if (risc2host_status & ISP2400RHS_RISC_PAUSED)
    {
        hccr = isp24->hccr;
        fprintf(stderr, "RISC paused hccr = %X  r2Hstatus %X DUMPING the ISP\n",
                hccr, isp24->r2HStat);
        ISP_DumpQL(port, ISP_RISC_PAUSED);
        return;
    }

    if ((risc2host_status & ISP2400RHS_RISC_REQ) == 0)
    {
        return;
    }

    risc_int_status = risc2host_status & ISP2400RHS_RISC_STATUS_MASK;
    if (risc_int_status == 0)
    {
        return;
    }
    ispInterrupt[port] = 0xFF;  /* Indicate interrupt happened */

    switch (risc_int_status)
    {
        case ISP2400RHS_STATUS_ROM_MBX_COMP:
        case ISP2400RHS_STATUS_ROM_MBX_ERR:
        case ISP2400RHS_STATUS_MBX_COMP:
        case ISP2400RHS_STATUS_MBX_ERR:
            isp_process_mbx(port, isp24->mBox[0]);
            break;

        case ISP2400RHS_STATUS_EVENT:
            ISP2400_HandleAsyncEvent(port, risc2host_status);
            break;

        case ISP2400RHS_STATUS_RESP:
        case ISP2400RHS_STATUS_ATIO:
        case ISP2400RHS_STATUS_ATIO_RESP:
            do
            {
                UINT16      tmpin;

                tmpin = isp2400_queue[port].in;
                isp2400_queue[port].work[tmpin] = risc2host_status;
                ++tmpin;
                risc2host_status = 0;
                if (tmpin >= MAX_2400_WORK)
                {
                    tmpin = 0;
                }
                if (tmpin == isp2400_queue[port].out)
                {
                    /* Queue is full */
                    fprintf(stderr, "%s: isp2400_queue full, port %d\n", __func__, port);
                    break;
                }

                isp2400_queue[port].in = tmpin;
                if (risc_int_status == ISP2400RHS_STATUS_ATIO_RESP)
                {
                    risc_int_status = ISP2400RHS_STATUS_ATIO;
                    risc2host_status = (isp24->atioQIP << 16) | risc_int_status;
                }
            } while (risc2host_status);

            if (atiomonpcb[port] && TaskGetState(atiomonpcb[port]) == PCB_ISP_WAIT)
            {
                TaskSetState(atiomonpcb[port], PCB_READY);
#ifdef HISTORY_KEEP
CT_history_pcb("ISP2400_IntrServiceRoutine setting ready pcb", (UINT32)(atiomonpcb[port]));
#endif /* HISTORY_KEEP */
            }
            break;

        default:
            fprintf(stderr, "UNKNOWN EVENT RECEIVED\n");
            break;
    }

    FORCE_WRITE_BARRIER;
    isp24->hccr = ISP2400HCCR_CLEAR_RISC_INTR;

}                               /* ISP_IntrServiceRoutine */


/**
****************************************************************************
**
**  @brief  Load Qlogic firmware to Qlogic 2400 card.
**
**  @param  port - QLogic chip instance (0-3)
**  @param  QRP - pointer to QRP
**
**  @return     0=GOOD, all else is an error.
**
******************************************************************************
**/
static UINT32 isp2400_loadQFW(UINT8 port, QRP * qrp)
{
    union
    {
        UINT32     *ptr;
        UINT16      s[2];
    } pBuffer;

    UINT32      retValue;
    INT32       length = 0;
    UINT32      riscAddr;
    UINT32      riscAddr1 = 0;
    UINT32      total_length = 0;
    const char *fileName;       /* File name */
    const char *fileName_2400;
    const char *fileName_2500;
    UINT32      file_len = 0;
    int         fileDesc;
    int         nread;
    INT32       i;

#if MULTI_ID
    fileName_2400 = "2400mid.bin";
    fileName_2500 = "2500mid.bin";
#else  /* MULTI_ID */
    fileName_2400 = "2400.bin";
    fileName_2500 = "2500.bin";
#endif /* MULTI_ID */

// fprintf(stderr, "%s%s:%u port=%d isp2400=0x%02x isp2500=0x%02x\n", FEBEMESSAGE, __func__, __LINE__, port, isp2400, isp2500);
    if (BIT_TEST(isp2500, port))
    {
        fileName = fileName_2500;
    }
    else
    {
        fileName = fileName_2400;
    }

    pBuffer.ptr = s_MallocW(BUFFER_SIZE * 2, __FILE__, __LINE__);

    /* Open the file */

// fprintf(stderr,"%s%s:%u %s loading firmware on port %d -- %s\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, port, fileName);

    fileDesc = open(fileName, O_RDONLY);
    if (fileDesc == -1)
    {
        /*
         ** Print an error and continue.
         ** No harm done unless we try to access this image.
         */
        fprintf(stderr, "%s: open failed with %d - %s\n",
                __func__, errno, strerror(errno));

        s_Free(pBuffer.ptr, BUFFER_SIZE * 2, __FILE__, __LINE__);     /* Release firmware buffer */

        return 1;
    }

    /* Calculate the total file length */

    file_len = lseek(fileDesc, 0, SEEK_END) - lseek(fileDesc, 0, SEEK_SET);

    while (total_length < file_len)
    {
        /*
         ** Seek to the current byte offset, first time through this is zero
         ** so we seek to start of the file. Second time through the seek is
         ** to the start of the second array of RISC code.
         */
        lseek(fileDesc, total_length, SEEK_SET);

        /*
         ** Read in the file until 128 bytes
         */
        nread = read(fileDesc, pBuffer.ptr, BUFFER_SIZE * 2);
        if (nread <= 0)
        {
            fprintf(stderr, "%s: read of file %s failed with %d - %s\n",
                    __func__, fileName, errno, strerror(errno));

            retValue = 1;
            goto err_exit;
        }

        /*
         ** Save the length and RISC address for this portion of the firmware
         ** load. The 2400 binary has two separate RISC address and two
         ** separate codes to load.
         */
        length = ntohl(pBuffer.ptr[3]);
        riscAddr = ntohl(pBuffer.ptr[2]);
        if (riscAddr1 == 0)
        {
            fprintf(stderr,
                    "%s: in QL2400 loader, loading %s at riscAddr %X length %X "
                    "for port %X ver %d.%d.%d\n",
                    __func__, fileName, riscAddr, length, port,
                    ntohl(pBuffer.ptr[4]), ntohl(pBuffer.ptr[5]), ntohl(pBuffer.ptr[6]));
        }
        else
        {
            fprintf(stderr,
                    "%s: in QL2400 loader, loading %s at riscAddr %X length %X "
                    "for port %X\n", __func__, fileName, riscAddr, length, port);
        }

        lseek(fileDesc, total_length, SEEK_SET);
        total_length += length * 4;

        /*
         ** Save the first RISC address so it can be used in the verify
         ** step at the end.
         */
        if (riscAddr1 == 0)
        {
            riscAddr1 = riscAddr;
        }

        /*
         ** Extract the length of the image from the header.
         ** Convert from byte count to 32 bit word count.
         ** Subtract the header length.
         */
        if (length == 0)
        {
            /* No firmware found */

            fprintf(stderr, "%s: returning ISP_NO_FW_FOUND\n", __func__);
            retValue = ISP_NO_FW_FOUND;
            goto err_exit;
        }

        /* Load the desired firmware image */

        while (length > 0)
        {
            nread = read(fileDesc, pBuffer.ptr, BUFFER_SIZE * 2);
            if (nread <= 0)
            {
                fprintf(stderr,
                        "%s: read of file %s failed with %d - %s\n",
                        __func__, fileName, errno, strerror(errno));
                retValue = 1;
                goto err_exit;
            }

            for (i = 0; i < BUFFER_SIZE / 2; i++)
            {
                pBuffer.ptr[i] = htonl(pBuffer.ptr[i]);
            }

            qrp->imbr[0] = ISP_LRRME;   /* Execute Load RISC RAM command */
            qrp->imbr[1] = riscAddr & 0xffff;
            qrp->imbr[2] = pBuffer.s[1] + (UINT16)(K_poffset >> 16);
            qrp->imbr[3] = pBuffer.s[0];
            qrp->imbr[6] = 0;
            qrp->imbr[7] = 0;
            qrp->imbr[8] = riscAddr >> 16;
            qrp->imbr[4] = XIO_MSW((length > BUFFER_SIZE / 2) ? BUFFER_SIZE / 2 : length);
            qrp->imbr[5] = XIO_LSW((length > BUFFER_SIZE) / 2 ? BUFFER_SIZE / 2 : length);

            qrp->iRegs = 0x1FF; /* set modify mailbox reg 0-8   */
            qrp->oRegs = 0x1;   /* set retrieve mailbox reg 0   */

            isp_exec_cmd_sri(port, qrp, TRUE);

            if (qrp->ombr[0] != ISP_CMDC)       /* Check return value */
            {
                /* Set Bad return value and exit */

                retValue = ISP_LOAD_RAM_ERROR;
                goto err_exit;
            }

            /* Setup for next piece of firmware */

            if (length > BUFFER_SIZE / 2)
            {
                riscAddr += BUFFER_SIZE / 2;
                length -= BUFFER_SIZE / 2;
            }
            else
            {
                length = 0;
            }
        }
    }

    s_Free(pBuffer.ptr, BUFFER_SIZE * 2, __FILE__, __LINE__); /* Release firmware buffer */

    close(fileDesc);            /* Close the file descriptor */

    retValue = isp2400_verifyFW(port, qrp, riscAddr1);

    return retValue;

err_exit:
    close(fileDesc);            /* Close the file descriptor */

    s_Free(pBuffer.ptr, BUFFER_SIZE * 2, __FILE__, __LINE__); /* Release firmware buffer */

    return retValue;
}


/**
****************************************************************************
**
**  @brief      Verify the firmware loaded into the Qlogic 2400 Card
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      QRP     - pointer to QRP
**  @param      riscAddr- RISC address
**
**  @return     0=GOOD, all else is an error.
**
******************************************************************************
**/
static UINT32 isp2400_verifyFW(UINT8 port, QRP * qrp, UINT32 riscAddr)
{
    ISP_2400   *isp2400hwptr = ispstr[port]->baseAd;
    UINT32      retValue;

    qrp->imbr[0] = ISP_VCSM;    /* Verify Checksum command */
    qrp->imbr[1] = XIO_MSW(riscAddr);
    qrp->imbr[2] = XIO_LSW(riscAddr);
    qrp->iRegs = 0x7;           /* Set modify mailbox reg 0-3   */
    qrp->oRegs = 0x7;           /* Set retrieve mailbox reg 0-3 */

    isp_exec_cmd_sri(port, qrp, TRUE);

    if (qrp->ombr[0] != ISP_CMDC)       /* Check return value */
    {
        return ISP_FW_CHK_SUM_ERROR;    /* Verify firmware checksum failed */
    }

    qrp->imbr[0] = ISP_EFRM;    /* Execute Firmware command */
    qrp->imbr[1] = XIO_MSW(riscAddr);
    qrp->imbr[2] = XIO_LSW(riscAddr);
    qrp->imbr[3] = 0;
    qrp->iRegs = 0xF;           /* Set modify mailbox reg 0-3   */
    qrp->oRegs = 0x3;           /* Set retrieve mailbox reg 0   */

    isp_exec_cmd_sri(port, qrp, TRUE);

    if (qrp->ombr[0] != ISP_CMDC)       /* Check return value */
    {
        return ISP_EXEC_FW_ERROR;       /* Execute Firmware Command Failed */
    }

    qrp->imbr[0] = ISP_GFRM;    /* Execute About Firmware command */
    qrp->iRegs = 0x1;           /* Set modify mailbox reg 0 */
    qrp->oRegs = 0x7F;          /* Set retrieve mailbox reg 0-6 */
    isp_exec_cmd_sri(port, qrp, TRUE);

    if (qrp->ombr[0] != ISP_CMDC)
    {
        return ISP_ABOUT_FW_ERROR;      /* About Firmware Command Failed */
    }

    /* Store revision information */

    isprev[port]->fwMajor = qrp->ombr[1];
    isprev[port]->fwMinor = qrp->ombr[2];
    isprev[port]->fwSub = qrp->ombr[3];
    isprev[port]->endMemAddr = (qrp->ombr[5] << 16) | qrp->ombr[4];
    isprev[port]->fwAttrib = qrp->ombr[6];

    if ((isprev[port]->fwAttrib & ATTRIB_MID_2400) != 0)
    {
        isprev[port]->type = MID;
    }

    retValue = isp_test_interface(port, qrp);
    if (retValue != GOOD)
    {
        fprintf(stderr, "%s:  port=%d, Test Interfaced failed, possible HW failure\n",
                __func__, port);
        return retValue;
    }

    /*
     ** Clear the request queue IN pointer
     ** and response queue OUT pointer
     */
    FORCE_WRITE_BARRIER;
    isp2400hwptr->reqQIP = 0;
    isp2400hwptr->rspQOP = 0;
    isp2400hwptr->atioQOP = 0;
    isp2400_queue[port].in = isp2400_queue[port].out = 0;
    FORCE_WRITE_BARRIER;

    return GOOD;
}


/**
****************************************************************************
**
**  @brief      Allocate the ICB(Initialization Control Block)
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     ICB address, else error
**
******************************************************************************
**/
UINT32 ISP2400_SetupInit(UINT32 port UNUSED)
{
    ISP2400_VPORT_ICB *pVPICB;

    pVPICB = s_MallocC(sizeof(struct ISP2400_VPORT_ICB) | BIT31, __FILE__, __LINE__);

    if (pVPICB == NULL)
    {
        fprintf(stderr, "VPICB malloc failed\n");
        return 0;
    }

    return (UINT32)pVPICB;
}


/**
****************************************************************************
**
**  @brief      initializes the  ICB(Initialization Control Block) and loads into
**              the Qlogic card <2400HBA>
**
**              More details on this function go here.
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     GOOD else error
**
******************************************************************************
**/
UINT32 ISP2400_InitFW(UINT32 port)
{
    QRP        *qrp;
    QCB        *pQCB = NULL;
    union
    {
        void       *ptr;
        UINT16      s[2];
    } pBuffer;

    /* Firmware options for ICB */

    UINT32      fwoptns1 = 0;
    UINT32      fwoptns2 = 0;
    UINT32      fwoptns3;

    /* Additional Firmware options */

    UINT32      adnlfwops1 = 0;
    UINT32      adnlfwops2 = 0;
    UINT32      adnlfwops3 = 0;

    UINT32      retValue;
    ISP2400_VPORT_ICB *pVPICB = NULL;

#if !MULTI_ID && defined(FRONTEND)
    TGD        *pTGD = NULL;
#endif /* !MULTI_ID && defined(FRONTEND) */

    qrp = get_qrp();

#if !MULTI_ID && defined(FRONTEND)
    pTGD = isp_findTarget(port);
#endif /* !MULTI_ID && defined(FRONTEND) */

    pVPICB = (ISP2400_VPORT_ICB *)ispstr[port]->icbStr;
    memset(pVPICB->vpcfg, 0, sizeof(ISP2400_VPORT_CONFIG) * ISP2400_MAX_VPORTS);

    /* Execute Set Firmware options */

    BIT_CLEAR(adnlfwops1, ISP_DISABLE_LED);
    BIT_CLEAR(adnlfwops1, ISP_ASYNC_8016);

    qrp->imbr[0] = ISP_SFO;
    qrp->imbr[1] = adnlfwops1;
    qrp->imbr[2] = adnlfwops2;
    qrp->imbr[3] = adnlfwops3;
    qrp->iRegs = 0xF;           /* Set modify mailbox reg 0-3     */
    qrp->oRegs = 0x3;           /* Set retrieve 0 mailbox regs    */

    isp_exec_cmd_sri(port, qrp, TRUE);

    /* Check return status of mailbox command */

    if (qrp->ombr[0] != ISP_CMDC)
    {
        return ISP_SET_FW_OPT_ERROR;    /* Return set firmware options failed */
    }

#if ISP_DEBUG_INFO
    fprintf(stderr, "%s: PORT %d set fw options %04X\n", __func__, port, qrp->ombr[0]);
#endif /* ISP_DEBUG_INFO */

    /* Enable/Disable Initiator Mode */

#if INITIATOR
    BIT_CLEAR(fwoptns1, ISP_INITIATOR_MODE);
#else  /* INITIATOR */
    BIT_SET(fwoptns1, ISP_INITIATOR_MODE);
#endif /* INITIATOR */

#ifdef TARGET
    BIT_SET(fwoptns1, ISP_TARGET_MODE);
#endif /* TARGET */

    BIT_CLEAR(fwoptns1, ISP_FULL_LOGIN_LIP);    /* No Full Login after LIP */

    BIT_SET(fwoptns1, ISP_FULL_DUPLEX); /* Set Full Duplex Mode */

#if defined(BACKEND)
    BIT_CLEAR(fwoptns1, ISP_FAIRNESS);  /* Disable Fairness Mode */
#else  /* BACKEND */
    BIT_SET(fwoptns1, ISP_FAIRNESS);    /* Enable Fairness Mode */
#endif /* BACKEND */

    BIT_SET(fwoptns1, ISP_NAME_OPTION); /* Set Name Option */

    BIT_CLEAR(fwoptns1, ISP_PREV_LID);  /* Disable Previous Assigned LoopID */

#if MULTI_ID && ISP_CP_DESC_LID
    if (BIT_TEST(ispCp, port) == TRUE)
    {
        BIT_SET(fwoptns1, ISP_DESLID_SEARCH);
    }
#else  /* MULTI_ID && ISP_CP_DESC_LID */
    BIT_CLEAR(fwoptns1, ISP_DESLID_SEARCH);
#endif /* MULTI_ID && ISP_CP_DESC_LID */

    BIT_CLEAR(fwoptns1, ISP_DISABLE_INITIAL_LIP);       /* Enable initial LIP */

    /*
     ** At this point all these options(fw2 and fw3) are
     ** applied to both FE and BE needs to revisit these
     ** options after discusssion with Steve - RVISIT
     */

#if !MULTI_ID
    /*
     ** Set to Loop Preferred Connection(otherwise P2P)
     ** bits 456 of fwoptins2(ICB) set to 0x2
     */
    BIT_SET(fwoptns2, ISP_CONN_OPTION_BIT2);
#else  /* !MULTI_ID */
#if 0
    /* set to point to point
     *  This requires NPIV to be enabled on the switch
     */
    BIT_SET(fwoptns2, ISP_CONN_OPTION_BIT3);
    BIT_CLEAR(fwoptns2, ISP_CONN_OPTION_BIT2);
    BIT_CLEAR(fwoptns2, ISP_CONN_OPTION_BIT1);
#else
    /* Set to Loop only mode */
    // need to add a get topology func like GetPortConfig
    BIT_CLEAR(fwoptns2, ISP_CONN_OPTION_BIT3);
    BIT_CLEAR(fwoptns2, ISP_CONN_OPTION_BIT2);
    BIT_CLEAR(fwoptns2, ISP_CONN_OPTION_BIT1);
#endif /* !MULTI_ID */
#endif

//      /* turn on zio bits 0-3 =5 or 6*/
//      BIT_SET(fwoptns2, 1);
//      BIT_SET(fwoptns2, 3);

    /* Set the DataRate bits */

    {
        UINT8       cfgix;
        static uint8_t cvt_cfg[] =
        {
            [ISP_CONFIG_AUTO] = 0x82,
            [ISP_CONFIG_1] = 0x80,
            [ISP_CONFIG_2] = 0x81,
            [ISP_CONFIG_4] = 0x83,
            [ISP_CONFIG_8] = 0x84,
        };

        cfgix = GetPortConfig(port);
//        fprintf(stderr, "%s: Port %d cfgix=%d\n", __func__, port, cfgix);
        if (cfgix >= dimension_of(cvt_cfg))
        {
//            fprintf(stderr, "%s: Port %d force default\n", __func__, port);
            cfgix = ISP_CONFIG_AUTO;
        }
        fwoptns3 = cvt_cfg[cfgix];
        if (!fwoptns3)
        {
//            fprintf(stderr, "%s: Port %d config invalid, force default\n", __func__, port);
            fwoptns3 = cvt_cfg[ISP_CONFIG_AUTO];
        }
        fwoptns3 &= 0x7F;
        fwoptns3 <<= 13;
//        fprintf(stderr, "%s: fwoptns3=%04X\n", __func__, fwoptns3);
    }

    /*
     ** Enable 50 ohm Termination
     ** Confirmed with qlogic.
     */
    BIT_CLEAR(fwoptns3, ISP_75OHM_TERMINATION);

    /* Enable automatic local logins */

    BIT_CLEAR(fwoptns3, ISP_DISABLE_PLOGI_LOCAL_LOOP);

    /*
     ** Set up size of response frame for good completion of target io,
     ** Set to 24 bytes of 0
     */
    BIT_SET(fwoptns3, ISP_FCP_RSP_PAYLOADBIT1);

#if !MULTI_ID
#if defined(FRONTEND)
    if (BIT_TEST(pTGD->opt, TARGET_HARD_ID) == TRUE)
    {
        BIT_SET(fwoptns1, ISP_HARD_LID);
    }

    pVPICB->nicb.hardAddress = MIN(pTGD->fcid, 0x7D);
#else  /* FRONTEND */
    pVPICB->nicb.hardAddress = 0;
#endif /* FRONTEND */
#endif /* !MULTI_ID */

    /* Populate the ICB(Initialization Control Block) */

    pVPICB->nicb.version = ISP_ICB_VERSION;
    pVPICB->nicb.framePayloadSize = ISP_MAXFRAME_SIZE;
    pVPICB->nicb.ext = ISP_EXEC_THROTTLE;

#ifdef MULTI_ID
    pVPICB->nicb.exc = ISP_EXCHANGE_COUNT;      /* Maximum Exchange Count */
#endif /* MULTI_ID */

    pVPICB->nicb.lrtc = ISP_LOGIN_RETRY_COUNT;
    pVPICB->nicb.linknos = ISP_LINKDOWN_TIMEOUT;
    pVPICB->nicb.intd = 0;
    pVPICB->nicb.loginTimeout = 0;

#if defined(BACKEND)
    UINT32      cserial = K_ficb->cSerial;

/*RVISIT fill the wwn values in the case of not multiid */
    pVPICB->nicb.portWWN = ((UINT64)XIO_OUI << 24) | (cserial & 0xFFFFFF);
    pVPICB->nicb.portWWN |= (UINT64)(WWN_B_PORT | (port << 16)) << 32;
    pVPICB->nicb.portWWN = bswap_64(pVPICB->nicb.portWWN);
    pVPICB->nicb.nodeWWN = (pVPICB->nicb.portWWN & 0xFFFFFFFF00000000LL) |
                bswap_32(WWN_B_NODE);
#endif /* BACKEND */

    /* Set up request, response, ATIO Q IN/OUT pointers */

    pVPICB->nicb.rqo = 0;
    pVPICB->nicb.rsi = 0;
    pVPICB->nicb.atioqin = 0;

    pVPICB->nicb.rql = ISP_REQ_QUE_SIZE;
    pVPICB->nicb.rsl = ISP_RES_QUE_SIZE;
    pVPICB->nicb.atioqlen = ISP_ATIO_QUE_SIZE;

    /* Set up options */

    pVPICB->nicb.fwo1 = fwoptns1;
    pVPICB->nicb.fwo2 = fwoptns2;
    pVPICB->nicb.fwo3 = fwoptns3;

    /* Get the request QCB from ISP Struct(ISPSTR) */

    pQCB = ispstr[port]->reqQue;
    pQCB->in = pQCB->begin;
    pQCB->out = pQCB->begin;

    /* Get the response QCB from ISP Struct(ISPSTR) */

    pQCB = ispstr[port]->resQue;
    pQCB->in = pQCB->begin;
    pQCB->out = pQCB->begin;

    /* Get the ATIO QCB from ISP Struct(ISPSTR) */

    pQCB = ispstr[port]->atioQue;
    pQCB->in = pQCB->begin;
    pQCB->out = pQCB->begin;

    isp2400_queue[port].in = isp2400_queue[port].out = 0;

    hba_q_cnt[port] = 0;        /* Set HBA Queue counter */


#if MULTI_ID
    if (ISP2400_BuildVPICB(port) != GOOD)
    {
        fprintf(stderr, "ISP2400_BuildVPICB failed\n");
        return 1;
    }
#endif /* MULTI_ID */

    pBuffer.ptr = pVPICB;

#if MULTI_ID
    qrp->imbr[0] = ISP_IFRT;
#else  /* MULTI_ID */
    qrp->imbr[0] = ISP_IFRM;
#endif /* MULTI_ID */

    qrp->imbr[2] = pBuffer.s[1] + (UINT16)(K_poffset >> 16);
    qrp->imbr[3] = pBuffer.s[0];
    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;
    qrp->iRegs = 0xCD;          /* Set modify mailbox reg 0,2,3,6,7 */
    qrp->oRegs = 0x1;           /* Set retrieve 0 mailbox regs */

    isp_exec_cmd_sri(port, qrp, TRUE);

    if (qrp->ombr[0] == ISP_CMDC)       /* Check return status of mailbox command */
    {
        retValue = GOOD;        /* Set good return value */
    }
    else
    {
        retValue = ISP_INIT_MBOX_ERROR; /* Return set firmware options failed */
    }

#if ISP_DEBUG_INFO
    fprintf(stderr, "%s: PORT %d init ICB %04X\n", __func__, port, qrp->ombr[0]);
#endif /* ISP_DEBUG_INFO */

    put_qrp(qrp);               /* Release QRP */

    return retValue;
}


/**
****************************************************************************
**
**  @brief      Helper function for 2400, stores the req/res/atio Q addresses
**              ICB.<2400>
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      Queloc  - Q address
**
**  @return     GOOD else error
**
******************************************************************************
**/

UINT32 ISP2400_IcbStore(UINT32 port, UINT32 Queloc)
{
    ISP2400_VPORT_ICB *pVPICB;

    pVPICB = (ISP2400_VPORT_ICB *)ispstr[port]->icbStr;

    if (BIT_TEST(icb2400, ICB2400_REQ_QSTORE))
    {
        pVPICB->nicb.reqa[0] = Queloc;
        pVPICB->nicb.reqa[1] = 0;
        BIT_CLEAR(icb2400, 0);
    }
    else if (BIT_TEST(icb2400, ICB2400_RES_QSTORE))
    {
        pVPICB->nicb.rsqa[0] = Queloc;
        pVPICB->nicb.rsqa[1] = 0;
        BIT_CLEAR(icb2400, 1);
    }
    else if (BIT_TEST(icb2400, ICB2400_ATIO_QSTORE))
    {
        pVPICB->nicb.atioqaddr[0] = Queloc;
        pVPICB->nicb.atioqaddr[1] = 0;
        BIT_CLEAR(icb2400, 2);
    }
    else
    {
        fprintf(stderr, "%s: ERROR NO Q Address Set to STORE\n", __func__);
    }
    return GOOD;
}


/**
****************************************************************************
**
**  @brief      Populates the VP configuration in ICB
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     GOOD else error
**
******************************************************************************
**/

UINT32 ISP2400_BuildVPICB(UINT32 port)
{
    ISP2400_VPORT_ICB *pVPICB;
    UINT32      vpcounter = 0;
    UINT32      totalvps = 0;
    UINT32      i = 0;
    UINT32      vpoptions = 0;
    TAR        *pTar = tar[port];

    pVPICB = (ISP2400_VPORT_ICB *)ispstr[port]->icbStr;

    /* Populate the primary port */

#if ISP_DEBUG_INFO
    fprintf(stderr, "%s: populating primary PORT %d VPORT %d\n",
            __func__, port, vpcounter);
    fprintf(stderr, "        node name %llX\n", pTar->nodeName);
    fprintf(stderr, "        port name %llX\n", pTar->portName);
#endif /* ISP_DEBUG_INFO */
    pVPICB->nicb.nodeWWN = pTar->nodeName;
    pVPICB->nicb.portWWN = pTar->portName;

    /* Be careful not to overwrite global options already set */

    if (BIT_TEST(pTar->opt, TARGET_HARD_ID) == TRUE)
    {
        BIT_SET(pVPICB->nicb.fwo1, ISP_HARD_LID);
    }

    if (BIT_TEST(pTar->opt, TARGET_PREV_ID) == TRUE)
    {
        BIT_SET(pVPICB->nicb.fwo1, ISP_PREV_LID);
    }
    BIT_SET(pVPICB->nicb.fwo1, ISP_TARGET_MODE);

    if (pTar->vpID < 126)
    {
        pVPICB->nicb.hardAddress = pTar->vpID;
    }

    /*
     ** Clear the global vpoptions all reserved fields
     ** must be set to zero as per ISP2400 spec
     */
    pVPICB->gvpopts = 0;

    /* Populate the Virtual Ports information */

    for (i = 1; i < MAX_TARGETS_PER_PORT; i++)
    {
        pTar = pTar->fthd;      /* Advance to the next target in the linked list */
        if (pTar == NULL)
        {
            break;
        }
        ++totalvps;

        /* Populate the first VP */

#if ISP_DEBUG_INFO
        fprintf(stderr, "%s: populating PORT %d VPORT %d\n", __func__, port, vpcounter);
        fprintf(stderr, "        node name %llX\n", pTar->nodeName);
        fprintf(stderr, "        port name %llX\n", pTar->portName);
#endif /* ISP_DEBUG_INFO */

        pVPICB->vpcfg[vpcounter].nodeWWN = pTar->nodeName;
        pVPICB->vpcfg[vpcounter].portWWN = pTar->portName;

        if (BIT_TEST(pTar->opt, TARGET_HARD_ID) == TRUE)
        {
            BIT_SET(vpoptions, ISPVP_HARD_ID);
        }

        if (BIT_TEST(pTar->opt, TARGET_PREV_ID) == TRUE)
        {
            BIT_SET(vpoptions, ISPVP_PREV_ID);
        }

        if (BIT_TEST(pTar->opt, TARGET_ENABLE) == TRUE)
        {
            BIT_SET(vpoptions, ISPVP_ENABLE);
            BIT_CLEAR(vpoptions, ISPVP_TARGET_DISABLED);
        }

        pVPICB->vpcfg[vpcounter].opts = vpoptions;
        if (pTar->vpID < 126)
        {
            pVPICB->vpcfg[vpcounter].hardAddress = pTar->vpID;
        }

#if ISP_DEBUG_INFO
        fprintf(stderr, "        options   %02X hardaddr %02X\n",
                pVPICB->vpcfg[vpcounter].opts, pVPICB->vpcfg[vpcounter].hardAddress);
#endif /* ISP_DEBUG_INFO */
        ++vpcounter;
        if (totalvps >= ISP2400_MAX_VPORTS)
        {
            break;
        }
    }

    /* Update the global vp count */

#if ISP_DEBUG_INFO
    fprintf(stderr, "%s: Setting total vports %d\n", __func__, totalvps);
#endif /* ISP_DEBUG_INFO */
    pVPICB->vpcnt = totalvps;

    return GOOD;
}


UINT32 ISP2400_ResetChip(UINT32 port)
{
    return ISP2400_SoftReset(port);
}


/**
****************************************************************************
**
**  @brief      Soft Reset the ISP2400
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     GOOD else error
**
******************************************************************************
**/

/**  As perthe specification the following steps are applied to soft reset the ISP2400
 **  1. Set the LED Control Bits of GPIOD to 0
 **  2. Stop the DMA activity by setting DMA Shutdwon Control Bit
 **  3. Monitor the DMA activity by polling the DMA Active Status
 **  4. Apply the soft reset to ISP
 **  5. Wait for ISP to reset
 **/
static UINT32 ISP2400_SoftReset(UINT32 port)
{
    struct ISP_2400 *isp24;
    UINT32      dmaResetCount;
    UINT32      gpiodata;

    fprintf(stderr, "%s with port %d\n", __func__, port);
    isp24 = ispstr[port]->baseAd;

    /* Set the LED Control Bits of GPIOD to 0 */

    gpiodata = isp24->gpIOD;
    gpiodata |= ISP2400GPIOD_LED_UPDATE_MASK;
    gpiodata &= ~ISP2400GPIOD_LED_CONTROL_MASK;
    isp24->gpIOD = gpiodata;
    FORCE_WRITE_BARRIER;

    /* Stop the DMA activity by setting DMA Shutdown Control Bit */

    dmaResetCount = 30;

    isp24->cntl = ISP2400CSR_DMA_CONTROL /*| ISP2400CSR_WRITE_4096BURST_COUNT */ ;
    FORCE_WRITE_BARRIER;

    do
    {
        if ((isp24->cntl & ISP2400CSR_DMA_ACTIVE) == 0)
        {
            break;
        }
        i_wait(50);             /* Short delay prior to reading control/status reg */
    } while (--dmaResetCount > 0);

    if (isp24->cntl & ISP2400CSR_DMA_ACTIVE)
    {
        fprintf(stderr, "%s: DMA still active, port=%d\n", __func__, port);
        return 1;
    }

    /*
     * Set mbox[0] non-zero just to be sure that we can see the ROM firmware
     * clear it after the soft reset.
     */
    isp24->mBox[0] = 0xFFFF;

    /* Perform a soft reset of the ISP2400 */

    isp24->cntl = ISP2400CSR_SOFT_RESET | ISP2400CSR_DMA_CONTROL;
                  /* | ISP2400CSR_WRITE_4096BURST_COUNT; */
    FORCE_WRITE_BARRIER;

    i_wait(120);
    isp2400_queue[port].in = isp2400_queue[port].out = 0;

    /*
     ** Wait until the RISC processor is ready by checking
     ** for Mailbox register 0 equal to zero.
     */
    while (isp24->mBox[0] != 0)
    {
        i_wait(125);
    }

    return 0;
}


/**
****************************************************************************
**
**  @brief  ISP2400_HandleAsyncEvent
**
**  @param  port - QLogic chip instance (0-3)
**  @param  r2hstatus - Risc to host status register contents
**
**  @return GOOD else error
**
******************************************************************************
**/
static UINT32 ISP2400_HandleAsyncEvent(UINT32 port, UINT32 r2hstatus)
{
    UINT32     *tmpin;
    QCB        *que;
    struct ISP_2400 *pISP24 = ispstr[port]->baseAd;
    UINT16      event_code;

    event_code = (UINT16)(r2hstatus >> 16);

#ifdef  FRONTEND
    /*
     ** In the case of loop down or LIP reset event, set the
     ** LIP interlock for this port.
     */
    if (event_code == ISP_ASPFCD || event_code == ISP_ASPLPR)
    {
        intlock[port] = 0xFF;
    }
#endif /* FRONTEND */

    que = asyqa[port];          /* Populate the async event code, subcode */

    tmpin = que->in + 2;        /* Advance IN pointer, and wrap if necessary */
    if (tmpin >= que->end)
    {
        tmpin = que->begin;
    }
    if (tmpin == que->out)
    {
        BIT_SET(ispaywt, port); /* Set async event que stall flag */
        return FAIL;
    }

    /*
     ** Do special handling for frame dropped events. When the queue is
     ** not empty and the previous event was also a queue dropped, simply
     ** increment the count in the second word of that entry. This avoids
     ** flooding the async event queue with frame dropped events, that we
     ** really can't do much with anyway.
     */
    if (event_code == ISP_ASPFRD && que->in != que->out)
    {
        UINT32     *prev;

        /* Find previous entry in event queue */

        prev = que->in;
        if (prev == que->begin)
        {
            prev = que->end;
        }
        prev -= 2;

        /* Was the previous entry a frame dropped as well? */

        if (*prev == ISP_ASPFRD)
        {
            ++prev[1];
            goto wake;
        }
        que->in[0] = event_code;
        que->in[1] = 0;
        que->in = tmpin;
        goto wake;
    }

    que->in[0] = event_code;    //pISP24->mBox[0];
    que->in[1] = (pISP24->mBox[2] << 16) | pISP24->mBox[1];
    que->in = tmpin;

wake:
    /* Wake up the async event handler thread */

    if (rtpcb[(port << 1) + 1] && TaskGetState(rtpcb[(port << 1) + 1]) == PCB_ISP_WAIT)
    {
#ifdef HISTORY_KEEP
CT_history_pcb("ISP2400_HandleAsyncEvent setting ready pcb", (UINT32)(rtpcb[(port << 1) + 1]));
#endif /* HISTORY_KEEP */
        TaskSetState(rtpcb[(port << 1) + 1], PCB_READY);
    }

    return GOOD;
}


#ifdef FRONTEND

void isp2400_processIdAcquisition(UINT8 port, VPRID_IOCB * rid_iocb)
{
    TAR        *pTar;
    UINT32      retValue;
    UINT32      i = 0;
    UINT16      vpacquired = rid_iocb->vpCount & 0x00FF;
    ISP2400_VPDB_PORTCFG *vport;

    vport = s_MallocC(sizeof(*vport), __FILE__, __LINE__);
#if ISP_DEBUG_INFO
    fprintf(stderr, "Process id acquisition for port %d, IOCB %p\n", port, rid_iocb);
#endif /* ISP_DEBUG_INFO */

  /* If tar list got changed, we might need to retry. */
  restart:

    if (vpacquired != 0)        /* Check for virtual port ID entries */
    {
        pTar = tar[port];       /* Get first target record */

        for (i = 0; i < MAX_TARGETS_PER_PORT; ++i)
        {
            if (i == 0)         /* Is this first time through loop? */
            {
                /* Get loop ID of the primary port */
                retValue = isp2400_get_vpdatabase(port, 0, vport);
                /* NOTE: first pointer is always valid. */
                pTar->portID = (vport->portid & 0xff) | (portid[port] & 0xffff00);
            }
            else
            {
                if (BIT_TEST(rid_iocb->vpindexmap[(i - 1) / VPINDX_MAPSIZE], ((i - 1) % VPINDX_MAPSIZE)))
                {
                    /* NOTE: isp2400_get_vpdatabase will task switch. */
                    UINT32 save_tar_link_abort = tar_link_abort[port];

                    retValue = isp2400_get_vpdatabase(port, i, vport);

                    /* If tar list changed, then our pointer may not be valid, restart. */
                    if (save_tar_link_abort != tar_link_abort[port])
                    {
                        goto restart;
                    }

                    if (retValue == 0)
                    {
                        pTar->portID = (vport->portid & 0xff) | (portid[port] & 0xffff00);
                    }
                }
                else
                {
                    break;
                }
            }                   /* end else not first time thru loop */

#if ISP_DEBUG_INFO
            fprintf(stderr, "pTar %p nodename %llX portname %llX\n",
                    pTar, pTar->nodeName, pTar->portName);
            fprintf(stderr, "db for vpid %d alpa %06X nodename %llX portname %llX\n",
                    i, vport->portid, vport->nodeWWN, vport->portWWN);
#endif /* ISP_DEBUG_INFO */

            pTar->vpID = i;     /* Store the vpid ID in the Target structure */

            /*
             * Loop ID changed, so we'll need to (re-)register FC-4 Types.
             * Clear the flags for this target.
             */
            pTar->flags = FALSE;

            /*
             * Check if target ID is valid. The control port
             * uses a target ID of 0xFFFF
             */
            if (pTar->tid < MAX_TARGETS)
            {
                /* Update the port assignment for this target */
                ispPortAssignment[pTar->tid] = port;
            }

            /* Check for non-particpipating */
            else if (BIT_TEST(pTar->opt, TARGET_ENABLE) && (vport->vpstatus & VPDB_NOT_PARTICIPATING))
            {
                /* Invalidate Loop ID and Port ID */
                pTar->vpID = NO_CONNECT;
                pTar->portID = NO_PORTID;

                /* Is this target currently assigned to this port? */
                if (pTar->tid < MAX_TARGETS && ispPortAssignment[pTar->tid] == port)
                {
                    /* Update the port assignment for this target */
                    ispPortAssignment[pTar->tid] = 0xFF;
                }
            }                   /* end else if */

            /* Increment to the next target in the linked list */
            pTar = pTar->fthd;

            /* Check if end of list reached */
            if (pTar == NULL)
            {
                break;
            }
        }                       /* end for MAX TARGETS */
    }                           /* end if vp acquired */

    BIT_SET(id_acquired, port); /* Indicate virtual port ID acquired */

    s_Free(vport, sizeof(*vport), __FILE__, __LINE__);
}
#endif /* FRONTEND */


/**
******************************************************************************
**
**  @brief      Issues the get virtual port database mailbox command to
**              retrieve the virtual port data base.
**
**              More details on this function go here.
**
**  @param      port        - QLogic chip instance (0-3)
**  @param      * buffer    - pointer to a buffer for the Virtual Port Database
**
**  @return     Number of entries in virtual port database
**
******************************************************************************
**/
UINT16 ISP2400_GetVPDatabase(UINT8 port, ISP2400_VPDB_PORTCFG * buffer)
{
    QRP        *qrp;
    UINT16      retValue;
    union
    {
        ISP2400_VPDB_PORTCFG *ptr;
        UINT16      s[2];
    } pBuffer;

    pBuffer.ptr = buffer;       /* Get the pointer to the destination buffer */

    qrp = get_qrp();            /* Allocate a QRP structure to issue mailbox commands */

    /* Execute Get Virtual Port Database command */

    qrp->imbr[0] = ISP_GVPD;
    qrp->iRegs = 0xCD;          /* set modify mailbox reg 0,2,3,6,7     */
    qrp->oRegs = 0x03;          /* set retrieve mailbox reg 0 and 1     */
    qrp->imbr[2] = pBuffer.s[1] + (UINT16)(K_poffset >> 16);
    qrp->imbr[3] = pBuffer.s[0];
    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;

    isp_exec_cmd(port, qrp, TRUE);

    if (qrp->ombr[0] == ISP_CMDC)       /* Did the command complete successfully? */
    {
        retValue = qrp->ombr[1];        /* Get the VP entry count */
    }
    else
    {
        retValue = 0;           /* Command failed, set VP entry to zero */
    }

    put_qrp(qrp);               /* Release QRP */

    return retValue;
}


#ifdef FRONTEND
UINT8 ISP_IsPrimaryPort(UINT16 port, UINT16 vpID)
{
// fprintf(stderr, "%s%s:%u port=%d isp2400=0x%02x isp2500=0x%02x\n", FEBEMESSAGE, __func__, __LINE__, port, isp2400, isp2500);
    if (BIT_TEST(isp2400, port) || BIT_TEST(isp2500, port))
    {
        if (!vpID)
        {
            return 1;
        }
        return 0;
    }

    if (BIT_TEST(iscsimap, port) || ICL_IsIclPort((UINT8)port))
    {
        if (T_tgdindx[vpID] != NULL && T_tgdindx[vpID]->prefPort == port)
        {
            return 1;
        }
        return 0;
    }

    return 0;
}
#endif /* FRONTEND */

#if 0
this isn't used but I don't want to delete it SMW
In some cases using this to send commands that send frames over the wire can cause
the mailbox queue to back up if the IO gets lost on the fabric or even if it times out
spending 30 seconds with a blocked MB is bad.
/**
****************************************************************************
**
**  @brief  Executes the "Execute IOCB" mailbox command
**
**  @param  port - QLogic chip instance (0-3)
**  @param  iocb - iocb to be executed - response iocb is copied over it
**
**  @return Mailbox command completion status code
**
******************************************************************************
**/
static UINT16 isp2400_execute_iocb_mb(UINT16 port, void *iocb)
{
    QRP        *qrp;
    UINT16      retValue;
    UINT32      pciaddr;

    qrp = get_qrp();

    qrp->imbr[0] = ISP_EA64;    /* Execute iocb command */
    qrp->iRegs = 0xCF;          /* Set modify mailbox reg 0-3, 6-7   */
    qrp->oRegs = 0x03;          /* Set retrieve 0-1 mailbox regs     */
    qrp->imbr[1] = 0;           /* Offset to response iocb */
    pciaddr = (UINT32)iocb + K_poffset;
    qrp->imbr[2] = (UINT16)(pciaddr >> 16);
    qrp->imbr[3] = (UINT16)(pciaddr & 0xFFFF);
    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;
    isp_exec_cmd(port, qrp, TRUE);

    retValue = qrp->ombr[0];    /* Get completion status */

    put_qrp(qrp);               /* Release QRP */

    return retValue;
}

#endif
/**
****************************************************************************
**
**  @brief      Executes the "Get ID list" mailbox command
**
**              Retrieves a list of alpas and their handles.
**              and lists them, used for debuging so far.
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     none
**
******************************************************************************
**/

static void GetIDList(UINT16 port, UINT8 vpid)
{
    QRP        *qrp;
    int         i;
    UINT32      pciaddr;
    PORT_ID_LIST *idlist;

    if (vpid > HANDLE_DB_SIZE)
    {
        fprintf(stderr, "%s: port %d invalid vpid %d\n", __func__, port, vpid);
        return;
    }

    qrp = get_qrp();            /* Allocate a QRP */

    if (!nphandleDB[port][vpid])
    {
        nphandleDB[port][vpid] = s_MallocC(BIT31 | (sizeof(PORT_ID_LIST) * MAX_DEV), __FILE__, __LINE__);
    }

    idlist = s_MallocC(sizeof(PORT_ID_LIST) * MAX_DEV, __FILE__, __LINE__);

    qrp->imbr[0] = ISP_GIDL;    /* Execute iocb command */

    pciaddr = (UINT32)idlist;
    pciaddr += K_poffset;
    qrp->iRegs = 0x3CD;         /* Set modify mailbox reg 0, 2-3, 6-9   */
    qrp->imbr[1] = 0;
    qrp->imbr[2] = (UINT16)(pciaddr >> 16);
    qrp->imbr[3] = (UINT16)(pciaddr & 0xFFFF);
    qrp->imbr[6] = 0;
    qrp->imbr[7] = 0;
    qrp->imbr[8] = sizeof(PORT_ID_LIST) * MAX_DEV;

    qrp->oRegs = 0x07;          /* Set retrieve 0-2 mailbox regs  */
    qrp->imbr[9] = vpid;
    isp_exec_cmd(port, qrp, TRUE);

    if (qrp->ombr[0] != ISP_CMDC)       /* Check completion status */
    {
        /* If the MB command failed set the count to 0 */

        nphandledbentrycount[port][vpid] = 0;
        fprintf(stderr, "%s: ombr[0] %04X ombr[1] %04X ombr[2] %04X\n",
                __func__, qrp->ombr[0], qrp->ombr[1], qrp->ombr[2]);
        goto out;
    }

    nphandledbentrycount[port][vpid] = qrp->ombr[1];
//     fprintf(stderr, "%s: Port %d table count %d\n", __func__, port, qrp->ombr[1]);

    /*
     ** The following loop goes backwards because on the 2300 it
     ** converts the smaller 6-byte 2300 entries into 8-byte
     ** 2400-style entries. Doing it backwards allows this to be
     ** done in-place. Of course "i" must be signed for this to
     ** work as well.
     */
    for (i = 0; i < qrp->ombr[1]; i++)
    {
        /* Mask off reserved data */
        idlist[i].nphandle &= 0x7FF;
        idlist[i].alpa &= 0x00FFFFFF;
#if 0
         fprintf(stderr, "%s: port %d, [%d]= hdl=%03x, alpa=%06x\n",
             __func__, port, i, idlist[i].nphandle, idlist[i].alpa);
#endif
    }

    /* Put the new table in place */

    memcpy(nphandleDB[port][vpid], idlist, sizeof(PORT_ID_LIST) * MAX_DEV);

out:
    put_qrp(qrp);               /* Release QRP & temporary ID list */
    s_Free(idlist, sizeof(PORT_ID_LIST) * MAX_DEV, __FILE__, __LINE__);
}


/**
****************************************************************************
**
**  @brief      issues a scsi command to the qlogic
**
**      This routine receives an ILT pointer that contains parameters
**      necessary to perform an I/O operation as requested. These parameters,
**      and their mapping to the ILT fields are as follows: (see <ilt.h>)
**
**
**  FRONTEND:
**      xli = (XLI *)ilt->misc; = pointer to struct as follows:
**
**      xlichipi                    Chip instance for this operation
**      xlifcflgs                   FC-AL flags for this I/O
**      xlitarget                   Target for this I/O
**      xlilun                      LUN for this I/O
**      xlitime                     Timeout in seconds
**      xlidatadir                  Data direction
**      xlicdbptr                   Pointer to SCSI CDB
**      xlisglpt                    Pointer to SGL for operation
**      xlisglnum                   Number of SGL elements
**      otil2_cr                    Completion routine to call after processing
**
**  BACKEND:
**      r_prp-ILTBIAS-ILTBIAS (prev prev ILT nest level) -> PRP structure.
**
**      If the SGL has more than 1 element a iocb type 6 is used otherwise a
**      type 7 is used. If using type 6 becomes a perfomance issue we may
**      have to look at using coninuation blocks with type 7. This
**      significantly increases the complexity of the isp_get_iocb routine
**      which has to get consecutive blocks.
**      Since isp_get_iocb can sleep it cannot simply be called twice in a row.
**
**      This IOCB is then submitted to the indicated QLogic instance.
**
**  @param  port - QLogic chip instance (0-3)
**  @param  ilt - iocb block to work on
**  @param  dev - pointer to DEV structure on backend only
**
**  @return status - qlogic status
**
******************************************************************************
**/
UINT64      type_7_count[MAX_PORTS];
UINT64      type_6_count[MAX_PORTS];

#define IOCB_TRACKING FALSE

UINT16 isp2400_initiate_io(UINT16 port, ILT * ilt,
#ifndef  BACKEND
                           UNUSED
#endif                          /* BACKEND */
                           DEV * dev)
{
    CMIO7_IOCB *iocb;
    CMIO6_IOCB *iocb6;
    ISP2400_STATUS_TYPE_0 *statiocb;
    UINT8      *cdb;
    UINT16      lun;
    UINT32      retValue;
    UINT32      rqip;
    SGL_DESC   *sgdesc = NULL;
    UINT32      sgdesc_count = 0;
    FCP_CMD_IU *iu = NULL;
    DSEG_DESC  *dsd_list = NULL;
    UINT8       rw_flags = 0;
    UINT8       tag_mode;
    UINT32      cdblen;
    UINT32      alpa;
    PDB        *portdbptr;
    UINT16      nphandle;
    UINT32      i;
    UINT8       vpindex;
    UINT32     *swapper;
#ifdef BACKEND
    UINT32      currentSession;

    currentSession = sessids[port];     /* Get current Session ID */
#endif /* BACKEND */

/*
*** I broke this out like this for readability
*** hopefully the compiler cleans this up.
*/
#ifdef BACKEND
    PRP        *prp;

    prp = (PRP *)(ilt - 2)->ilt_normal.w0;

    cdb = &(prp->cmd[0]);
    cdblen = prp->cBytes;
    nphandle = prp->id;
    lun = prp->lun;

#if ISP_DEBUG_INFO
    //fprintf(stderr, " prp cmd %p ", &prp->cmd[0]);
    //fprintf(stderr, "\n    0 cdb %02hX rw_flags %02hX func %02hX prpflags %02X sgl %p\n", cdb[0], rw_flags, prp->func, prp->flags, prp->pSGL);
#endif /* ISP_DEBUG_INFO */
    if (prp->func != PRP_CTL && prp->pSGL != NULL)
    {
        if ((UINT32)prp->pSGL == 0xfeedf00d)
        {
            fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
            abort();
        }
        sgdesc = (SGL_DESC *)&prp->pSGL[1];
        sgdesc_count = prp->pSGL->scnt;

        /* Get direction */
        if (prp->func == PRP_INPUT)
        {
            rw_flags = 0x02;    /* Read */
        }
        else if (prp->func == PRP_OUTPUT)
        {
            rw_flags = 0x01;    /* Write */
        }
    }
    else
    {
        rw_flags = 0;
    }

    if (BIT_TEST(prp->flags, PRP_ORT))
    {
        tag_mode = 2;
    }
    else
    {
        tag_mode = 0;           /* Simple */
    }
    vpindex = 0;
#else  /* BACKEND */
    XLI        *xli;

    xli = (XLI *)ilt->misc;
    cdb = (UINT8 *)(xli->xlicdbptr);
    cdblen = 16;
    nphandle = xli->xlitarget;
    vpindex = 0;                /* Is this a problem xli struct does not have vpid */
    lun = xli->xlilun;
    if (xli->xlisglnum)
    {
        sgdesc = (SGL_DESC *)xli->xlisglptr;
        sgdesc_count = xli->xlisglnum;
    }

    /* Get direction */
    switch (xli->xlidatadir & DATA_DIR_MASK)
    {
        case XLI_DATA_READ:
            rw_flags = 0x02;    /* Read */
            break;

        case XLI_DATA_WRITE:
            rw_flags = 0x01;    /* Write */
            break;

        case XLI_DATA_NONE:
        default:
            rw_flags = 0;
            break;
    }
    tag_mode = xli->xlifcflgs & 0x7;
#endif /* BACKEND */

    portdbptr = portdb[port] + nphandle;
    /* Can this even happen?? */
    if (portdbptr == NULL)
    {
//        fprintf(stderr, "%s calling ISP_EnhancedGetPortDB\n", __func__);
#ifdef FRONTEND
        ISP_GetPortDB(port, nphandle, vpindex);
#else  /* FRONTEND */
        if (FAB_ReValidateConnection(port, prp->pDev) != ISP_CMDC)
        {
            retValue = ERROR;
            goto isp2400_cancel_io;
        }
//      ISP_EnhancedGetPortDB(port, nphandle, pdbPidToALPA(prp->pDev->portId[port]));
#endif /* FRONTEND */
    }

    if (sgdesc_count > 1)
    {
        iu = s_MallocC(sizeof(FCP_CMD_IU), __FILE__, __LINE__);
        dsd_list = s_MallocC(sizeof(DSEG_DESC) * (sgdesc_count + 1), __FILE__, __LINE__);
    }

#if IOCB_TRACKING
    ilt->isp_defs.cmio7cpy = s_MallocC(IOCB_SIZE, __FILE__, __LINE__);
#endif /* IOCB_TRACKING */

#ifdef BACKEND
    /*
     *  if something strange has happened to the handle or WWN
     *  or the port has been reset since starting return an error
     */
    if (prp->pDev->pLid[port] != nphandle ||
        portdbptr->ndn != prp->pDev->nodeName || currentSession != sessids[port])
    {
        fprintf(stderr, "%s: SESSION OR HANDLE OR DEVICE MISMATCH PORT %d\n",
                __func__, port);
        fprintf(stderr, "       prp->pDev->pLid[%d] %04X nphandle %04X \n",
                port, prp->pDev->pLid[port], nphandle);
        fprintf(stderr, "       portdbptr->ndn %016llX prp->pDev->nodeName %016llX\n",
                portdbptr->ndn, prp->pDev->nodeName);
        fprintf(stderr, "       currentSession %d sessids[port] %d flags %08X DEV port %d \n",
                currentSession, sessids[port], prp->flags, prp->pDev->port);
        if (sgdesc_count > 1)
        {
            s_Free(iu, sizeof(FCP_CMD_IU), __FILE__, __LINE__);
            if (sgdesc_count)
            {
                s_Free(dsd_list, sizeof(DSEG_DESC) * (sgdesc_count + 1), __FILE__, __LINE__);
            }
        }

        retValue = EC_CANCEL;
        goto isp2400_cancel_io;
    }

    if (BIT_TEST(dev->flags, DV_QLTIMEOUTEMULATE))
    {
        /* Fill in ILT values for isp_check_thread */
        ilt->isp_defs.isp_iocb_type = CMIO7;
        ilt->isp_defs.isp_timeout = prp->timeout;
        ilt->isp_defs.isp_dsd_list = dsd_list;
        ilt->isp_defs.isp_fcp_cmd_iu = iu;
        /* Do not get an iocb -- we are doing nothing on the qlogic port. */
        fprintf(stderr, "%s: DV_QLTIMEOUTEMULATE -- initiate I/O pid=%u ILT=%p PRP=%p timeout=%u cmd=0x%02x\n",
                millitime_string(), dev->pdd->pid, ilt, prp, ilt->isp_defs.isp_timeout, prp->cmd[0]);
        goto QL_TIMEOUT_EMULATE;
    }
#endif /* BACKEND */

    iocb = (CMIO7_IOCB *)isp_get_iocb(port, &rqip);

    /* Did we sucessfully get an IOCB? if yes no swapping till we issue it */
    if (iocb == NULL)
    {
        fprintf(stderr, "%s: isp_get_iocb returned null\n", __func__);
        if (sgdesc_count > 1)
        {
            s_Free(iu, sizeof(FCP_CMD_IU), __FILE__, __LINE__);
            if (sgdesc_count)
            {
                s_Free(dsd_list, sizeof(DSEG_DESC) * (sgdesc_count + 1), __FILE__, __LINE__);
            }
        }

        retValue = EC_PORT_FAIL;
        goto isp2400_cancel_io;
    }

    memset(iocb, 0, IOCB_SIZE);     /* Clear iocb */

    /* Generic setup */
    iocb->entryCount = 1;
    iocb->iocbhandle = (UINT32)ilt;
    iocb->nphandle = nphandle;
#ifdef FRONTEND
    iocb->timeout = xli->xlitime;
    ilt->isp_defs.isp_timeout = xli->xlitime + 10;
#else  /* FRONTEND */
    iocb->timeout = prp->timeout;
    ilt->isp_defs.isp_timeout = prp->timeout + 3;       /* A short qlogic timeout, not 30 extra seconds. */
#endif /* FRONTEND */
    iocb->dseg_count = sgdesc_count;
    alpa = pdbPidToALPA(portdbptr->pid);
    iocb->alpa15_0 = alpa & 0xFFFF;
    iocb->alpa24_16 = (UINT8)(alpa >> 16);
    iocb->vpindex = vpindex;
    iocb6 = (CMIO6_IOCB *)iocb;

#ifdef DEBUG_FLT_REC_ISP
    UINT32      fr_parm1,
                fr_parm2,
                fr_parm3;

    memcpy(&fr_parm1, &cdb[0], 4);
    memcpy(&fr_parm2, &cdb[4], 4);
    memcpy(&fr_parm3, &cdb[8], 4);
    MSC_FlightRec(FR_ISP_CDB, fr_parm1, fr_parm2, fr_parm3);
#endif /* DEBUG_FLT_REC_ISP */

    /* Set up data pointers and SGL arrays etc. */

    if (sgdesc_count > 1)
    {
        type_6_count[port]++;
        iocb6->entryType = CMIO6;
        //iocb6->rsp_dsd_len = 0;
        iocb6->fcp_cmd_iu_len = sizeof(FCP_CMD_IU);
        iocb6->fcp_cmd_addr = LI_GetPhysicalAddr((UINT32)iu);
        iocb6->control_flags = rw_flags;
        iocb6->fcp_data_dsd.addr = LI_GetPhysicalAddr((UINT32)dsd_list);
        iocb6->fcp_data_dsd.length = sizeof(DSEG_DESC) * (sgdesc_count + 1);
        iocb6->control_flags |= 0x0004;     /* Set dsd list indicator bit */
        //iocb6->fcp_rsp_addr = 0;

        /* The lun appears in 2 places in type 6 -shrug- */
        iocb6->lun[3] = lun >> 8;
        iocb6->lun[2] = lun & 0x00ff;

        ilt->isp_defs.isp_dsd_list = dsd_list;
        ilt->isp_defs.isp_fcp_cmd_iu = iu;
    }
    else
    {
        type_7_count[port]++;
        iocb->entryType = CMIO7;
        iu = &iocb->iu;
        dsd_list = &(iocb->dseg0);
        ilt->isp_defs.isp_dsd_list = 0;
        ilt->isp_defs.isp_fcp_cmd_iu = 0;
    }

    /* Process sgl list */
    for (i = 0, iu->fcp_dl = 0; i < sgdesc_count; ++i)
    {
        dsd_list[i].addr = LI_GetPhysicalAddr((UINT32)(sgdesc[i].addr));
        dsd_list[i].length = sgdesc[i].len & 0x00ffffff;
        iu->fcp_dl += dsd_list[i].length;
    }

    if (sgdesc_count > 1)
    {
        iocb6->total_bytes = iu->fcp_dl;
    }

    /* Scsi command and lun stuff */

    iu->lun[3] = lun >> 8;
    iu->lun[2] = lun & 0x00ff;
    iu->rw_flags = rw_flags;
    //iu->task_flags = 0; already 0
    iu->tag_mode = tag_mode;

    //iu->crn      = 0; already 0

#ifdef BACKEND

#if 0
#ifndef PERF
 if ((dev != NULL && dev->pdd != NULL) && dev->pdd->ses == 7) {
    print_scsi_cmd(dev, prp, "");
 }
#endif  /* !PERF */
#endif   /* 0 */

/* THIS IS WHERE ALL SCSI COMMANDS GOING OUT THE BE PASS THROUGH */
    if (cdb[0] == SCC_WRITE_6)          /* write-6 */
    {
        if ((cdb[1] & 0x1f) == 0 && cdb[2] == 0 && cdb[3] < 9)
        {
            fprintf(stderr, "LOW BLOCKS WRITE-6 pid=%d lun=%d SCSI COMMAND %2.2x %2.2x%2.2x%2.2x %2.2x %2.2x\n",
                    dev->pdd->pid, dev->lun, cdb[0], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5]);
        }
    }
    else if (cdb[0] == SCC_WRITEXT)     /* write-10 */
    {
        if (cdb[2] == 0 && cdb[3] == 0 && cdb[4] == 0 && cdb[5] < 9)
        {
            fprintf(stderr, "LOW BLOCKS WRITE-10 pid=%d lun=%d SCSI COMMAND %2.2x %2.2x %2.2x%2.2x%2.2x%2.2x %2.2x %2.2x%2.2x %2.2x\n",
                    dev->pdd->pid, dev->lun, cdb[0], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5],
                    cdb[6], cdb[7], cdb[8], cdb[9]);
        }
    }
    else if (cdb[0] == SCC_WRTSAME_10)  /* write-same-10 */
    {
        if (cdb[2] == 0 && cdb[3] == 0 && cdb[4] == 0 && cdb[5] < 9)
        {
            fprintf(stderr, "LOW BLOCKS WRITE-SAME-10 pid=%d lun=%d SCSI COMMAND %2.2x %2.2x %2.2x%2.2x%2.2x%2.2x %2.2x %2.2x%2.2x %2.2x\n",
                    dev->pdd->pid, dev->lun, cdb[0], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5],
                    cdb[6], cdb[7], cdb[8], cdb[9]);
        }
    }
//    else if (cdb[0] == SCC_LOGSENSE)     /* Log Sense */
//    {
//        fprintf(stderr, "LOG SENSE page0x%02x-%02x pid=%d lun=%d SCSI COMMAND %2.2x %2.2x %2.2x%2.2x%2.2x%2.2x %2.2x %2.2x%2.2x %2.2x\n",
//                    cdb[2], cdb[3], dev->pdd->pid, dev->lun,
//                    cdb[0], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5],
//                    cdb[6], cdb[7], cdb[8], cdb[9]);
//    }
    else if (cdb[0] == SCC_WRITE_12)    /* write-12 */
    {
        if (cdb[2] == 0 && cdb[3] == 0 && cdb[4] == 0 && cdb[5] < 9)
        {
            fprintf(stderr, "LOW BLOCKS WRITE-12 pid=%d lun=%d SCSI COMMAND %2.2x %2.2x %2.2x%2.2x%2.2x%2.2x %2.2x%2.2x%2.2x%2.2x %2.2x %2.2x\n",
                    dev->pdd->pid, dev->lun, cdb[0], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5],
                    cdb[6], cdb[7], cdb[8], cdb[9], cdb[10], cdb[11]);
        }
    }
    else if (cdb[0] == SCC_WRITE_16)    /* write-16 */
    {
        if (cdb[2] == 0 && cdb[3] == 0 && cdb[4] == 0 && cdb[5] == 0 &&
            cdb[6] == 0 && cdb[7] == 0 && cdb[8] == 0 && cdb[9] < 9)
        {
            fprintf(stderr, "LOW BLOCKS WRITE-16 pid=%d lun=%d SCSI COMMAND %2.2x %2.2x %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x %2.2x%2.2x%2.2x%2.2x %2.2x %2.2x\n",
                    dev->pdd->pid, dev->lun, cdb[0], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5], cdb[6], cdb[7], cdb[8], cdb[9],
                    cdb[10], cdb[11], cdb[12], cdb[13], cdb[14], cdb[15]);
        }
    }
    else if (cdb[0] == SCC_WRITELONG)   /* write-long-10 */
    {
        fprintf(stderr, "WRITE-LONG-10 pid=%d lun=%d SCSI COMMAND %2.2x %2.2x %2.2x%2.2x%2.2x%2.2x %2.2x %2.2x%2.2x %2.2x\n",
                dev->pdd->pid, dev->lun, cdb[0], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5],
                cdb[6], cdb[7], cdb[8], cdb[9]);
    }
    else if (cdb[0] == SCC_SERVOUT_16)  /* write-long-16 */
    {
        fprintf(stderr, "WRITE-LONG-16 pid=%d lun=%d SCSI COMMAND %2.2x %2.2x %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x %2.2x%2.2x %2.2x%2.2x %2.2x %2.2x\n",
                dev->pdd->pid, dev->lun, cdb[0], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5], cdb[6], cdb[7], cdb[8], cdb[9],
                cdb[10], cdb[11], cdb[12], cdb[13], cdb[14], cdb[15]);
    }
#endif  /* BACKEND */
    iu->cdb[0] = cdb[3];
    iu->cdb[1] = cdb[2];
    iu->cdb[2] = cdb[1];
    iu->cdb[3] = cdb[0];
    iu->cdb[6] = cdb[5];
    iu->cdb[7] = cdb[4];
    if (cdblen > 6)
    {
        iu->cdb[4] = cdb[7];
        iu->cdb[5] = cdb[6];

        iu->cdb[10] = cdb[9];
        iu->cdb[11] = cdb[8];
    }
    if (cdblen > 10)
    {
        iu->cdb[8] = cdb[11];
        iu->cdb[9] = cdb[10];
    }
    if (cdblen > 12)
    {
        iu->cdb[12] = cdb[15];
        iu->cdb[13] = cdb[14];
        iu->cdb[14] = cdb[13];
        iu->cdb[15] = cdb[12];
    }

    /* Need to byte swap iu for cmio6 */
    if (sgdesc_count > 1)
    {
        swapper = (UINT32 *)iu;
        for (i = 0; i < 8; ++i)
        {
            swapper[i] = htonl(swapper[i]);
        }
    }
    /* Done with iu */
    ilt->isp_defs.isp_iocb_type = CMIO7;

#if IOCB_TRACKING
    memcpy(ilt->isp_defs.cmio7cpy, iocb, IOCB_SIZE);
#endif /* IOCB_TRACKING */

    /* Submit IOCB by updating Request Queue IN pointer */

    update_rqip(port, rqip);

#ifdef BACKEND
QL_TIMEOUT_EMULATE:             /* Do following for emulating qlogic timeout. */
    if (dev != prp->pDev)
    {
        fprintf(stderr, "%s: DEV * DO NOT MATCH dev=%p, prp->pDev=%p\n",
                __func__, dev, prp->pDev);
    }
    ilt->isp_defs.isp_dev = dev;    /* Save DEV record pointer */
    dev->orc++;                 /* Increment outstanding request count */
    dev->pdd->qd++;             /* Increment PDD queue depth */
#endif /* BACKEND */

    /* Add this ILT to the active list */

    ilt->isp_defs.isp_timestamp = timestamp;
    isp_thread_ilt(port, ilt);
    return(EC_OK);

isp2400_cancel_io:
    /* Allocate fake iocb */
    statiocb = s_MallocC(sizeof(*statiocb), __FILE__, __LINE__);
    statiocb->entryStatus = 0x2;
    statiocb->compStatus = QLOGIC_STATUS_TIMEOUT;
    statiocb->scsiStatus = 0x0;
    ilt--;                      /* Unnest ilt 1 level */
#ifdef BACKEND
    prp->qLogicStatus = QLOGIC_STATUS_TIMEOUT;
    prp->reqStatus = EC_CANCEL;
    prp->scsiStatus = EC_OK;
#endif /* BACKEND */
    isp$complete_io(EC_PORT_FAIL, ilt, statiocb);

    s_Free(statiocb, sizeof(*statiocb), __FILE__, __LINE__);  /* Free fake iocb */

    return retValue;
}


/**
****************************************************************************
**
**  @brief      processes the status type 0 iocb isp2400_iocbstatustype0
**
**              processes response iocb
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      iocb    - iocb to be looked at.
**
**  @return     GOOD else error
**
**  @attention  while the continuation IOCBs are evaluated here. They are left
**              on the buffer. They are handled as noops by the isp$processiocb
**              function. This way we dont have to mess with the response queue
**
******************************************************************************
**/
static UINT16 isp2400_iocbstatustype0(UINT16 port, ISP2400_STATUS_TYPE_0 * iocb)
{
    UINT16      retValue = EC_OK;
    ILT        *ilt = (ILT *)iocb->iocbhandle;
    UINT8       scsiStatus;
    UINT32      sgdesc_count = 0;
#ifdef BACKEND
    ISP2400_STATUS_TYPE_0_CONT *iocbcontinue;
    QCB        *Que = (QCB *)ispstr[port]->resQue;
#endif /* BACKEND */
    UINT32     *swapptr;
    UINT32      i;
#if defined(BACKEND) && (defined(MODEL_7000) || defined(MODEL_4700))
    time_t t;
#endif

    if (!verify_iocbhandle(ilt))
    {
        ILT *tmpILT = (ILT*)bswap_32((UINT32)ilt);
        if (!verify_iocbhandle(tmpILT))
        {
            fprintf(stderr, "%s%s:%u %s STATUS_0 nphandle is not an ILT (%p), exiting routine.\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            return(EC_UNDET_SCSI_STAT);
        }
        else
        {
            fprintf(stderr, "%s%s:%u %s STATUS_0 nphandle was byteswapped (%p), correcting...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            ilt = tmpILT;
        }
    }


#ifdef BACKEND
    if (iocb->entryCount > 1)   /* Deal with continuation iocb */
    {
        fprintf(stderr, "%s: continuation io present\n", __func__);

        if ((Que->out + 16) >= Que->end)
        {
            iocbcontinue = (ISP2400_STATUS_TYPE_0_CONT *)Que->begin;
        }
        else
        {
            iocbcontinue = (ISP2400_STATUS_TYPE_0_CONT *)Que->out + 16;
        }
    }
    else
    {
        iocbcontinue = NULL;
    }
#endif /* BACKEND */

    scsiStatus = iocb->scsiStatus & 0xFF;
    // check for the status 0 response to send TMF if so we need to copy the iocb and
    // bail out of here.
    if (ilt->isp_defs.isp_iocb_type == IOCB_WITH_WAIT_ILT_TYPE)
    {
        ilt--;
        memcpy((void*)ilt->ilt_normal.w2,iocb,64);
        return retValue;
    }
#ifdef BACKEND
    UINT32      sensebytes;

    PRP        *prp;

    prp = (PRP *)(ilt - 2)->ilt_normal.w0;
    prp->scsiStatus = scsiStatus;
    prp->pDev->orc--;

    prp->pDev->pdd->qd--;
    prp->qLogicStatus = iocb->compStatus;

    if ((prp->func != PRP_CTL) && (prp->pSGL != NULL))
    {
        if ((UINT32)prp->pSGL == 0xfeedf00d)
        {
            fprintf(stderr,"%s%s:%u %s sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__, __func__);
            abort();
        }
        sgdesc_count = prp->pSGL->scnt;
    }

    /* Deal with sense data */

    if (iocb->senseLength && (iocb->scsiStatus & 0x0200))
    {
        if (iocb->respLength >= 28)             /* If bad response length */
        {
            fprintf(stderr,"%s%s:%u %s respLength=0x%8.8x\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, iocb->respLength);
// #ifdef BACKEND
            prp->reqStatus = EC_UNDET_SCSI_STAT;
// #endif /* BACKEND */
            return(EC_UNDET_SCSI_STAT);
        }
        sensebytes = 28 - iocb->respLength;
        memcpy(prp->sense, iocb->rspData + iocb->respLength, sensebytes);
        prp->reqSenseBytes = iocb->senseLength;

        if (iocb->entryCount > 1 && sensebytes < sizeof(SNS))
        {
            memcpy(prp->sense + sensebytes, iocbcontinue->data, sizeof(SNS) - sensebytes);
        }

        swapptr = (UINT32 *)prp->sense;
        for (i = 0; i < (sizeof(SNS) / 4); i++)
        {
            swapptr[i] = ntohl(swapptr[i]);
        }
    }
#else  /* BACKEND */
    XLI        *xli;

    xli = (XLI *)ilt->misc;
    sgdesc_count = xli->xlisglnum;

    if (iocb->senseLength && iocb->scsiStatus & 0x0200)
    {
        if (iocb->respLength >= 28)             /* If bad response length */
        {
            fprintf(stderr,"%s%s:%u %s respLength=0x%8.8x\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, iocb->respLength);
// #ifdef BACKEND
//             prp->reqStatus = EC_UNDET_SCSI_STAT;
// #endif /* BACKEND */
            return(EC_UNDET_SCSI_STAT);
        }
        swapptr = (UINT32 *)iocb->rspData + iocb->respLength;
        for (i = 0; i < 6; i++)
        {
            swapptr[i] = ntohl(swapptr[i]);
        }
    }
#endif /* BACKEND */

    switch (scsiStatus)
    {
        case SCS_NORM:
            retValue = EC_OK;
            break;

        case SCS_ECHK:
            retValue = EC_CHECK;
#if defined(BACKEND) && (defined(MODEL_7000) || defined(MODEL_4700))
        if (prp->sense[12] == 0x5d)
        {
            time(&t);
            fprintf(stderr, "<ISE-%02d (port:%d wwn:0x%llX) EVENT pid:%d [PAB : 0x%x : 0x%x : 0x%x] > - %s",
                            prp->pDev->pdd->ses, port, prp->pDev->pdd->wwn,
                            prp->pDev->pdd->pid, prp->sense[12],
                            prp->sense[13], prp->sense[15], ctime(&t));
            /*
            ** Special event from ISE. We are only interested in 0x81, 0x82, 0x85 & 0x86
            */
            if ((prp->sense[15] == 0x81) || (prp->sense[15] == 0x85))
            {
                ON_BEBusy(prp->pDev->pdd->pid, 90, 0);
                retValue = EC_BEBUSY;
                goto out;
            }
            else if ((prp->sense[15] == 0x82) || (prp->sense[15] == 0x86))
            {
                ON_BEClear(prp->pDev->pdd->pid, 0);
                retValue = EC_BEBUSY;
                goto out;
            }
        }
        else if ((prp->sense[12] == 0x81)
                    && (!BIT_TEST(prp->pDev->pdd->flags, PD_BEBUSY)))
        {
            /*
            ** Special response for TUR cmd when ISE is in BUSY condition
            */
            time(&t);
            fprintf(stderr, "<ISE-%02d (port:%d wwn:0x%llX) EVENT pid:%d [ TUR Rsp : 0x%x : 0x%x ]> - %s",
                            prp->pDev->pdd->ses, port, prp->pDev->pdd->wwn,
                            prp->pDev->pdd->pid, prp->sense[12], prp->sense[13], ctime(&t));
            ON_BEBusy(prp->pDev->pdd->pid, 90, 0);
            retValue = EC_BEBUSY;
            goto out;
        }
        /*
        ** If the command is 'GetLogpage, don't override
        ** the return status and propagate EC_CHECK to define layer.
        ** based on the return value and the check condition the CCB
        ** needs to retry this command
        */
        else if(prp->cmd[0] == 0x4D)
        {
            fprintf(stderr, "<ispc.c>- check condition for ISE Get Log Page....\n");
            goto out;
        }
#endif /* defined(BACKEND) && (defined(MODEL_7000) || defined(MODEL_4700) */
#if defined(BACKEND) && (defined(MODEL_3000) || defined(MODEL_7400))
            /*
             ** If the command is 'send diagnostic command, don't override
             ** the return status and propagate EC_CHECK to define layer.
             ** This return value is needed to CCB to retry the command
             ** during fw upgrade on 750 SATA drives in an SBOD Bay, which
             ** normally sees the check condition with ENCLOSURE BUSY during
             ** bay fw upgrade.
             */
            if (prp->cmd[0] == 0x1D)
            {
                fprintf(stderr, "<ispc.c>- check condition for Send Diagnostic..\n");
                goto out;
            }

#endif /* defined(BACKEND) && (defined(MODEL_3000) || defined(MODEL_7400)) */
            break;

        case SCS_BUSY:
#if defined(BACKEND) && (defined(MODEL_7000) || defined(MODEL_4700))
        if (BIT_TEST(prp->pDev->pdd->flags, PD_BEBUSY) == FALSE)
        {
            /*
            ** Special event from ISE. We are only interested in 0x81, 0x82, 0x85 & 0x86
            */
            time(&t);
            fprintf(stderr, "<ISE-%02d (port:%d wwn:0x%llX) EVENT pid:%d [ SCS_BUSY Rsp ]> - %s",
                                        prp->pDev->pdd->ses, port, prp->pDev->pdd->wwn,
                                        prp->pDev->pdd->pid, ctime(&t));
            ON_BEBusy(prp->pDev->pdd->pid, 90, 0);
        }
        retValue = EC_BEBUSY;
        goto out;
#else  /* defined(BACKEND) && (defined(MODEL_7000) || defined(MODEL_4700)) */
            retValue = EC_BUSY;
            break;
#endif /* defined(BACKEND) && (defined(MODEL_7000) || defined(MODEL_4700)) */

        case SCS_RESC:
            retValue = EC_RES_CONFLICT;
            break;

        case SCS_QUEF:
            retValue = EC_QUEUE_FULL;
            break;

        default:
            retValue = EC_UNDET_SCSI_STAT;
            break;
    }

    switch (iocb->compStatus)
    {
        case QLOGIC_STATUS_GOOD:
            // do nothing
            //retValue= EC_OK;
            break;

        case QLOGIC_STATUS_DMAERROR:
            retValue = EC_DMA;
            break;

        case QLOGIC_STATUS_TRANSPORT_ERR:
            retValue = EC_TRANSPORT;
            break;

        case QLOGIC_STATUS_RESET:
            retValue = EC_LIP_RESET;
#ifdef FRONTEND
            ISP_SubmitMarker(port, 2, 0, 0);
#endif /* FRONTEND */
            break;

        case QLOGIC_STATUS_TASKABORT:
        case QLOGIC_STATUS_ABORTED_BY_TARG:
            retValue = EC_CMD_ABORT;
            break;

        case QLOGIC_STATUS_TIMEOUT:
            retValue = EC_TIMEOUT;
            break;

        case QLOGIC_STATUS_OVERRUN:
            retValue = EC_OVERRUN;
            break;

        case QLOGIC_STATUS_DATA_ASSEMB_ERR:
            retValue = EC_TRANSPORT;
            break;

        case QLOGIC_STATUS_UNDERRUN:
#ifdef FRONTEND
            if (retValue == EC_OK)
            {
                retValue = EC_UNDERRUN;
            }
#endif /* FRONTEND */
#ifdef BACKEND
            //don't overwrite check conditions.
            if (retValue != EC_CHECK)
            {
                retValue = EC_UNDERRUN;
                if (prp->flags & PRP_SLI_BIT)
                {
                    retValue = EC_OK;
                }
            }
#endif /* BACKEND */
            break;

        case QLOGIC_STATUS_PORTUNAVAIL:
#ifdef BACKEND
            if (prp->pDev->pdd && BIT_TEST(prp->pDev->pdd->flags,PD_BEBUSY))
            {
                retValue = EC_BEBUSY;
                break;
            }
#endif
            retValue = EC_NONX_DEV;
            break;

        case QLOGIC_STATUS_PORTLOGGEDOUT:
        case QLOGIC_STATUS_PORTCHANGED:
#ifdef BACKEND
            if (prp->pDev->pdd && BIT_TEST(prp->pDev->pdd->flags,PD_BEBUSY))
            {
                retValue = EC_BEBUSY;
                break;
            }
#endif
            retValue = EC_LGOFF;
            break;

        default:
            retValue = EC_IOCB_ERR;
            fprintf(stderr, "%s: Unmapped iocberror %02X\n", __func__, iocb->compStatus);
            break;
    }

#if ISP_DEBUG_INFO
    if (retValue != EC_OK && retValue != EC_UNDERRUN && retValue != EC_CHECK)
    {
        fprintf(stderr,
                "STATUS TYPE 0:ERROR PORT %d scsi_stat %02X iocberror %02X ox_id %04X ret_val %02x\n",
                port, scsiStatus, iocb->compStatus, iocb->ox_id, retValue);

#ifdef FRONTEND
        UINT8      *cdb = (UINT8 *)xli->xlicdbptr;

        fprintf(stderr, "    handle %04X  alpa %06X\n    CDB:",
                xli->xlitarget, isp_handle2alpa(port, xli->xlitarget));

        for (i = 0; i < 16; i++)
        {
            fprintf(stderr, "%02X ", cdb[i]);
        }
        fprintf(stderr, "\n");
#endif /* FRONTEND */
    }
#endif /* ISP_DEBUG_INFO */

#ifdef BACKEND
out:
    prp->reqStatus = retValue;
#endif /* BACKEND */

    /* Clean up allocated mem */
    if (ilt->isp_defs.isp_dsd_list && sgdesc_count > 1)
    {
        s_Free(ilt->isp_defs.isp_dsd_list, sizeof(DSEG_DESC) * (sgdesc_count + 1), __FILE__, __LINE__);
    }

    if (ilt->isp_defs.isp_fcp_cmd_iu)
    {
        s_Free(ilt->isp_defs.isp_fcp_cmd_iu, sizeof(FCP_CMD_IU), __FILE__, __LINE__);
    }

#if IOCB_TRACKING
    if (ilt->isp_defs.cmio7cpy)
    {
        s_Free(ilt->isp_defs.cmio7cpy, IOCB_SIZE, __FILE__, __LINE__);
    }
#endif /* IOCB_TRACKING */

    return retValue;
}


/**
****************************************************************************
**
**  @brief  Send task management to target
**
**  @param  port - QLogic chip instance (0-3)
**  @param  nphandle - target
**  @param  action - action to be taken
**
**  @return  mailbox completion status
**
******************************************************************************
**/
static UINT16 isp2400_sendTMF(UINT16 port, UINT16 nphandle, UINT16 lun, UINT32 action)
{
    UINT16      retValue;
    ISP2400_TASK_MANAGEMENT *iocb;
    PDB        *portdbptr = portdb[port] + nphandle;
    UINT32      alpa;
    ISP2400_TASK_MANAGEMENT *RespIOCB;
    ILT         *ilt;
    UINT32      rqip;

//    fprintf(stderr, "%s:%d  port=%d, Handle=%04x action=%08X\n", __func__, __LINE__, port, nphandle, action);

    RespIOCB = s_MallocC(sizeof(*iocb), __FILE__, __LINE__);     /* Allocate iocb */
    ilt = get_ilt();
    iocb = (ISP2400_TASK_MANAGEMENT *)isp_get_iocb(port,&rqip);
    if (iocb == NULL)
    {
        put_ilt(ilt);
        s_Free(RespIOCB, sizeof(*iocb), __FILE__, __LINE__);
        return ISP_CMDE;
    }
    iocb->entryType = TMFIOCB;
    iocb->entryCount = 1;
    iocb->nphandle = nphandle;
    iocb->delay = 0;
    iocb->timeout = 10;
    iocb->controlflags = action;
    iocb->lun[2] = lun >> 8;
    iocb->lun[3] = lun & 0xFF;
    alpa = pdbPidToALPA(portdbptr->pid);
    iocb->alpa15_0 = alpa & 0xFFFF;
    iocb->alpa24_16 = (UINT8)(alpa >> 16);

    ilt++;
    iocb->iocbhandle = (UINT32)ilt;
    ilt--;
    retValue = exec_iocb_wait(port, ilt, rqip, (UINT16*)RespIOCB, 30);

    if ((retValue & 0xFF) == 0)
    {
        /* The nphandle is reused as the status for this command */
        if (RespIOCB->nphandle)
        {
            retValue = ISP_CMDE;
        }
    }

    /* Submit a marker */
    if (retValue == ISP_CMDC)
    {
        switch (action)
        {
            case ISP2400_TMF_TARGET_RESET:
            case ISP2400_TMF_ABORT_TARGET:
                ISP_SubmitMarker(port, 1, nphandle, lun);
                break;

            case ISP2400_TMF_LUN_RESET:
            case ISP2400_TMF_CLEAR_TASK_SET:
            case ISP2400_TMF_ABORT_TASK_SET:
            case ISP2400_TMF_ABORT_QUEUE:
                ISP_SubmitMarker(port, 0, nphandle, lun);
                break;

            default:
                break;
        }
    }
    s_Free(RespIOCB, sizeof(*iocb), __FILE__, __LINE__);
    put_ilt(ilt);
    return retValue;
}


/**
****************************************************************************
**
**  @brief      Turn an alpa into a nport handle
**
**              This needs a hash table or something to improve performance
**              if the alpa is not in the port database a database update
**              will be attempted.
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      alpa    - alpa to match
**  @param      vpid    - vpid
**
**  @return     nphandle else NO_ID 0x7FFF
**
******************************************************************************
**/
UINT16 isp_alpa2handle(UINT8 port, UINT32 alpa, UINT8 vpid)
{
    UINT32      i;
    int         called_getidlist = FALSE;
    PORT_ID_LIST *idlist = nphandleDB[port][vpid];

    if (vpid > HANDLE_DB_SIZE)
    {
        fprintf(stderr, "%s: port %d invalid vpid %d\n", __func__, port, vpid);
        return NO_LID;
    }

    do
    {
        if (!idlist || !nphandledbentrycount[port][vpid])
        {
            GetIDList(port, vpid);
            called_getidlist = TRUE;
            idlist = nphandleDB[port][vpid];
        }

        /* If still NULL exit */
        if (!idlist)
        {
            fprintf(stderr, "%s: port %d failed to get idlist\n", __func__, port);
            return NO_LID;
        }

        for (i = 0; i < nphandledbentrycount[port][vpid]; i++)
        {
            if (idlist[i].alpa == alpa)
            {
                return idlist[i].nphandle;
            }
        }

        /* If not found list might be stale, force update */

        if (!called_getidlist)
        {
            nphandledbentrycount[port][vpid] = 0;
        }
    } while (!called_getidlist);

#if 0
    fprintf(stderr, "%s: port %d Failed to find alpa %06x vpid %d\n",
            __func__, port, alpa, vpid);
//    for (i = 0; i < nphandledbentrycount[port][vpid]; i++)
//    {
//        fprintf(stderr, "port %d handle %04X portid %06X\n", port, idlist[i].nphandle,idlist[i].alpa );
//    }
#endif  /* 0 */
    return NO_LID;
}


/**
****************************************************************************
**
**  @brief      Turn a handle into an alpa
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      nphandle - handle to match
**
**  @return     nphandle else NO_ID 0x7FFF
**
******************************************************************************
**/
UINT32 isp_handle2alpa(UINT8 port, UINT32 nphandle)
{
#ifdef FRONTEND
    if (BIT_TEST(iscsimap, port))
    {
        return fsl_getPID(port, (UINT16)nphandle);
    }
#endif /* FRONTEND */

    PDB        *pPortDb = portdb[port] + nphandle;

    return pdbPidToALPA(pPortDb->pid);
}

#ifdef FRONTEND
/**
****************************************************************************
**
**  @brief      process ctio7 response iocb
**
**              just needs to free alocated dsdlist
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      iocb    - iocb to examine
**
**  @return     none
**
******************************************************************************
**/
#define CTIO7_FLAG_SEND_STATUS  0x8000
#define CTIO7_FLAG_TERM_XCHG    0x4000
#define CTIO7_FLAG_MODE1        0x0040
#define CTIO7_FLAG_DSDLIST      0x0004
#define CTIO7_FLAG_DATA2INIT    0x0002
#define CTIO7_FLAG_DATA2CTRL    0x0001
#define CTIO7_FLAG_DATAMASK     0x0003

#define DSD_LIST_PRESENT          0x01
#define XCHG_TERMINATED           0x02

#define SYS_DEF_CTIO7_NOILTNEST   0x01
#define SYS_DEF_CTIO7_ILTNEST     0x02

#define ISP2400_TARG_IO_TRACKING FALSE

static UINT32 isp2400_process_ctio7(UINT16 port UNUSED, ISP2400_CTIO7_STATUS * iocb)
{
    ILT        *ilt = (ILT *)iocb->iocbhandle;
    ILT        *iltXL = NULL;
    ILT        *iltsc = NULL;

    if (!verify_iocbhandle(ilt))
    {
        ILT *tmpILT = (ILT*)bswap_32((UINT32)ilt);
        if (!verify_iocbhandle(tmpILT))
        {
            fprintf(stderr, "%s%s:%u %s CMIO7 nphandle is not an ILT (%p), processing as zero...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            ilt = 0;
        }
        else
        {
            fprintf(stderr, "%s%s:%u %s CMIO7 nphandle was byteswapped (%p), correcting...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            ilt = tmpILT;
        }
    }

    /*FRee the dsdlist if there is one */
    /*np handle is the status in the response iocb */
#if 0
    if (iocb->nphandle != 1 || iocb->entryStatus != 0)
    {
        fprintf(stderr, "%s: ERROR %04X entry stat %02X exchange %08X vpid %d "
                "resid length %08X iocb 0x%08X ilt 0x%08X\n",
                __func__, iocb->status, iocb->entryStatus,
                iocb->exchangeaddr, iocb->vpid, iocb->residXferLen,
                (UINT32)iocb, iocb->iocbhandle);
    }
#endif /* 0 */

    if (ilt && iocb->sysdef == SYS_DEF_CTIO7_ILTNEST)
    {
        iltXL = (ILT *)ilt->misc;
        if (iltXL)
        {
            iltsc = iltXL->fc_xl.xl_pILT;
        }
        if (iltsc)
        {
            iltsc--;
#if ISP_TARGET_IO_TIMING
            UINT32 delta;
            UINT32 delta2;
            delta = timestamp - iltsc->scsi_2400_dsd.ts;
            delta2 = timestamp - iltsc->scsi_2400_dsd.ts2;
#endif
            if (iltsc->scsi_2400_dsd.isp2400_dsdlist)
            {
                s_Free(iltsc->scsi_2400_dsd.isp2400_dsdlist,
                     sizeof(DSEG_DESC) * (iltsc->scsi_2400_dsd.isp2400_sgdesc_count + 1), __FILE__, __LINE__);
                iltsc->scsi_2400_dsd.isp2400_dsdlist = NULL;
                iltsc->scsi_2400_dsd.isp2400_sgdesc_count = 0;
#if ISP2400_TARG_IO_TRACKING
                s_Free(iltsc->ctio7_cpy, IOCB_SIZE, __FILE__, __LINE__);
#endif /* ISP2400_TARG_IO_TRACKING */

            }
            iltsc++;
#if ISP_TARGET_IO_TIMING
            if  (delta > 5 || delta2 > 5)
            {
                fprintf(stderr,"SMW port %d time overrun delta %d delta2 %d ilt %p ox_id %04X %s \n",port,delta,delta2, iltsc,
                        iltsc->scsi_cdm.scsi_cdm_attr.scox_id,__func__);
            }
#endif
        }
    }

    switch (iocb->status)       // handle is the status
    {
        case CTIO7_STATUS_OK:
            return EC_OK;

        case CTIO7_STATUS_ABORTED:
            return EC_CMD_ABORT;

        case CTIO7_STATUS_INVALID:
            return EC_INV_FUNC;

        case CTIO7_STATUS_INVALID_EX:
            return EC_INV_RX_ID;

        case CTIO7_STATUS_OVERRUN:
            return EC_OVERRUN;

        case CTIO7_STATUS_TIMEOUT:
            return EC_TIMEOUT;

        case CTIO7_STATUS_LIP_RCV:
            return EC_LIP_RESET;

        case CTIO7_STATUS_DMA_ERR:
            return EC_DMA;

        case CTIO7_STATUS_DATA_ASM_ERR:
            return EC_IOCB_ERR;

        case CTIO7_STATUS_UNDERRUN:
            return EC_UNDERRUN;

        case CTIO7_STATUS_PORT_UNAVAIL:
            return EC_NONX_DEV;

        case CTIO7_STATUS_PORT_LOGOUT:
        case CTIO7_STATUS_PORT_CHANGE:
            return EC_LGOFF;

        case CTIO7_STATUS_ERROR:
        case CTIO7_STATUS_SRR_RCV:
            return EC_IO_ERR;

        default:
            isp_log_iocb(port, EC_IO_ERR, iocb);
            return EC_IO_ERR;
    }
}


/**
****************************************************************************
**
**  @brief  Fills in an iocb of ctio7
**
**          The command is started by the caller. ISP$receive_io
**
**  @param  port - QLogic chip instance (0-3)
**  @param  ILT - ilt
**  @param  iocb - iocb to fill
**
**  @return     none
**
******************************************************************************
**/

#define     LOCKED      1
#define     UNLOCKED    0

UINT32 isp2400_build_ctio7(UINT16 port, ILT * ilt)
{
    ILT        *iltXL = (ILT *)ilt->misc;
    ILT        *status_ilt;
    ILT        *iltsc = iltXL->fc_xl.xl_pILT;
    ISP2400_CTIO7 *iocb;
    ISP2400_CTIO7 *status_iocb = NULL;
    ISP2400_CTIO7_MODE0 *iocb0;
    ISP2400_CTIO7_MODE1 *iocb1;
    SGL_DESC   *sgdesc = NULL;
    UINT32      sgdesc_count;
    DSEG_DESC  *dsd_list = NULL;
    UINT32      i;
    UINT32      rqip;
    UINT32      status_rqip;
    UINT32     *swapptr;
#if ISP2400_TARG_IO_TRACKING
    iltsc--;
    iltsc->ctio7_cpy = s_MallocC(IOCB_SIZE, __FILE__, __LINE__);
    iltsc++;
#endif /* ISP2400_TARG_IO_TRACKING */
#if ISP_TARGET_IO_TIMING
    UINT32 delta;
    UINT32 cdb0;
    iltsc--;
    delta = timestamp - iltsc->scsi_2400_dsd.ts;
    iltsc->scsi_2400_dsd.ts2 = timestamp;
    cdb0 = iltsc->scsi_2400_dsd.cdb[0];
    iltsc++;
    if  (delta > 5)
    {
        if (iltXL->fc_xl.xl_fcflgs & 0x01)
        {
            fprintf(stderr,"SMW port %d time overrun delta %d ilt %p ox_id %04X  cdb %02X  %s done \n",port,delta, iltsc,
                iltsc->scsi_cdm.scsi_cdm_attr.scox_id,cdb0,__func__);
        }
        else
        {
            fprintf(stderr,"SMW port %d time overrun delta %d ilt %p ox_id %04X  cdb %02X  %s not done \n",port,delta, iltsc,
                iltsc->scsi_cdm.scsi_cdm_attr.scox_id,cdb0,__func__);
        }
    }
#endif
    /* Need to malloc ddsdlist before iocb */
    sgdesc_count = iltXL->fc_xl.xl_sgllen;
    if (sgdesc_count > 1)
    {
        dsd_list = s_MallocC(sizeof(DSEG_DESC) * (sgdesc_count + 1), __FILE__, __LINE__);
    }

    status_ilt = get_ilt();     /* Get the status ILT to avoid an exchange later */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)status_ilt);
#endif /* M4_DEBUG_ILT */

    iocb = (ISP2400_CTIO7 *)isp_get_iocb(port, &rqip);
    if (!iocb)
    {
#if ISP2400_TARG_IO_TRACKING
        iltsc--;
        s_Free(iltsc->ctio7_cpy, IOCB_SIZE, __FILE__, __LINE__);
        iltsc++;
#endif /* ISP2400_TARG_IO_TRACKING */
        if (sgdesc_count > 1)
        {
            s_Free(dsd_list, sizeof(DSEG_DESC) * (sgdesc_count + 1), __FILE__, __LINE__);
        }
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)status_ilt);
#endif /* M4_DEBUG_ILT */
        put_ilt(status_ilt);
        return EC_IOCB_LIP_INT;
    }

    iocb0 = (ISP2400_CTIO7_MODE0 *)iocb;
    iocb1 = (ISP2400_CTIO7_MODE1 *)iocb;

    /* Clear iocb */
    memset(iocb, 0, IOCB_SIZE);

    /* Set up iocb fileds */
    iocb->entryType = CTIO7;
    iocb->entryCount = 1;
    iocb->sysdef = 0;
    iocb->iocbhandle = (UINT32)ilt;
    iocb->nphandle = iltsc->scsi_cdm.scinit;
    iocb->timeout = 30;
    ilt->isp_defs.isp_iocb_type = 0x17;     /* I am setting this to ctio2 */
    ilt->isp_defs.isp_timeout = 40;
    iocb->vpid = iltsc->inotify_2400.invpid;

    /* Access port database to find alpa of initiator */
    iocb->alpa = isp_handle2alpa(port, iltsc->scsi_cdm.scinit);
    iocb->exchangeaddr = iltsc->scsi_cdm.scrxid;
    iocb->flags = iltsc->scsi_cdm.scsi_cdm_attr.attribute2400 << 9;     /* 4 into bits 9-12 */
    iocb->ox_id = iltsc->scsi_cdm.scsi_cdm_attr.scox_id;

    iocb->flags &= 0x7fff;      /* Clear the Send status flag for now. */

    if (iltXL->fc_xl.xl_cmd == XL_DATA2INIT)
    {
        iocb->flags |= CTIO7_FLAG_DATA2INIT;
    }
    else if (iltXL->fc_xl.xl_cmd == XL_DATA2CTRL)
    {
        iocb->flags |= CTIO7_FLAG_DATA2CTRL;
    }

    /* Basics are now set up need to set up data and/ or response */
    /* If done */
    if (iltXL->fc_xl.xl_fcflgs & 0x01)
    {
        req_cnt--;              /* Decrement request counter */

        iltsc->scsi_cdm.scrcn = 0;      /* Clear not done flag */
        iocb->scsi_status = iltXL->fc_xl.xl_scsist;
        if ((iocb->flags & CTIO7_FLAG_DATAMASK) == 0 && iocb->scsi_status == 0)
        {
            iocb->flags |= CTIO7_FLAG_SEND_STATUS;
        }
    }

    /*
     * If sense data or status set mode 1 if mode 1 is set status will
     * be good no matter what we set it to.
     */
    if (iltXL->fc_xl.xl_snslen == 0 && iltXL->fc_xl.xl_scsist != 0)
    {
        iocb->flags |= CTIO7_FLAG_MODE1;
        iocb->flags |= CTIO7_FLAG_SEND_STATUS;

        /*
         * SAN-1386 this specific change was done for the VMWare
         * ESX certification. The VMware drivers requires the
         * RUR/ROR needs to be set when non scsi-check statuses are
         * returned  and driver is unable to send the requested byte to
         * initiator.
         */

        /* Set residual length values */
        if (iltXL->fc_xl.xl_reslen > 0)
        {
            /* Set underflow indicator bit */
            iocb->scsi_status |= 0x0800;        //bit 11
            iocb->residXferLen = iltXL->fc_xl.xl_reslen;
        }
        if (iltXL->fc_xl.xl_reslen < 0)
        {
            /* Set overflow indicator bit */
            iocb->scsi_status |= 0x0400;        //bit 10
            /* Convert data to positive value and save */
            iocb->residXferLen = -iltXL->fc_xl.xl_reslen;
        }
        if (sgdesc_count > 1)
        {
            s_Free(dsd_list, sizeof(DSEG_DESC) * (sgdesc_count + 1), __FILE__, __LINE__);
        }
    }
    else if (iltXL->fc_xl.xl_snslen)
    {
        iocb->flags |= CTIO7_FLAG_MODE1;
        iocb->flags |= CTIO7_FLAG_SEND_STATUS;
        memcpy(iocb1->sesnedata, iltXL->fc_xl.xl_pSNS, 0x18);
        swapptr = (UINT32 *)iocb1->sesnedata;
        for (i = 0; i < 6; i++)
        {
            swapptr[i] = htonl(swapptr[i]);
        }

        iocb->scsi_status |= 0x0200;    /* Set sense length valid */
        if (iltXL->fc_xl.xl_snslen > 0x18)
        {
            iocb1->sense_length = 0x18;
            fprintf(stderr, "%s to much sense data (%d) truncating\n",
                    __func__, iltXL->fc_xl.xl_snslen);
            printBuffer1(iltXL->fc_xl.xl_pSNS, iltXL->fc_xl.xl_snslen);
        }
        else
        {
            iocb1->sense_length = iltXL->fc_xl.xl_snslen;
        }

        /*
         * SAN-1567 this specific change was done for the VMWare
         * ESX 3.5 Update 4.
         */

        /* Set residual length values */
        if (iltXL->fc_xl.xl_reslen > 0)
        {
            /* Set underflow indicator bit */
            iocb->scsi_status |= 0x0800;        //bit 11
            iocb->residXferLen = iltXL->fc_xl.xl_reslen;
        }
        if (iltXL->fc_xl.xl_reslen < 0)
        {
            /* Set overflow indicator bit */
            iocb->scsi_status |= 0x0400;        //bit 10
            /* Convert data to positive value and save */
            iocb->residXferLen = -iltXL->fc_xl.xl_reslen;
        }
        if (sgdesc_count > 1)
        {
            s_Free(dsd_list, sizeof(DSEG_DESC) * (sgdesc_count + 1), __FILE__, __LINE__);
        }
    }
    else if (iocb->flags & CTIO7_FLAG_DATAMASK)
    {
        /* Set up data */
        /* Get sgl information from command */
        sgdesc = (SGL_DESC *)iltXL->fc_xl.xl_pSGL;

        /* If more than 1 entry need a sgl list in memory allocate it */
        if (sgdesc_count > 1)
        {
            iocb->flags |= CTIO7_FLAG_DSDLIST;
            iocb0->dseg0.addr = LI_GetPhysicalAddr((UINT32)dsd_list);
            iocb0->dseg0.length = sizeof(DSEG_DESC) * (sgdesc_count + 1);
            iltsc--;
            iltsc->scsi_2400_dsd.isp2400_dsdlist = dsd_list;
            iltsc->scsi_2400_dsd.isp2400_sgdesc_count = sgdesc_count;
            iltsc++;
            iocb0->sysdef = SYS_DEF_CTIO7_ILTNEST;
            // iltsc->scsi_cdm.scsi_cdm_attr.isp2400flags = DSD_LIST_PRESENT;
        }
        else
        {
            iltsc--;
            iltsc->scsi_2400_dsd.isp2400_dsdlist = NULL;
            iltsc->scsi_2400_dsd.isp2400_sgdesc_count = 0;
            iltsc++;
            iltsc->scsi_cdm.scsi_cdm_attr.isp2400flags = 0;
            dsd_list = &iocb0->dseg0;
        }

        /* Copy values into dsdlist */
        for (i = 0; i < sgdesc_count; i++)
        {
            dsd_list[i].addr = LI_GetPhysicalAddr((UINT32)(sgdesc[i].addr));
            dsd_list[i].length = sgdesc[i].len & 0x00ffffff;
            iocb0->xferLen += dsd_list[i].length;
        }

        iocb0->dseg_count = sgdesc_count;       /* Set segment counter */
        iocb0->relOffset = iltXL->fc_xl.xl_reloff;      /* Set relative offset */
    }

#if ISP2400_TARG_IO_TRACKING
    iltsc--;
    memcpy(iltsc->ctio7_cpy, iocb, IOCB_SIZE);
    iocb0->sysdef = SYS_DEF_CTIO7_ILTNEST;
    iltsc++;
#endif /* ISP2400_TARG_IO_TRACKING */

    /* Submit IOCB by updating Request Queue IN pointer */
    update_rqip(port, rqip);

    isp_thread_ilt(port, ilt);  /* Add this ILT to the active list */

    /* Submit a status IOCB if done and data was transferred and there was not a check condition */
    if ((iltXL->fc_xl.xl_fcflgs & 0x01) && (iocb->flags & CTIO7_FLAG_DATAMASK) &&
        !iltXL->fc_xl.xl_snslen)
    {
        status_iocb = (ISP2400_CTIO7 *)isp_get_iocb(port, &status_rqip);
        if (!status_iocb)
        {
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)status_ilt);
#endif /* M4_DEBUG_ILT */
            put_ilt(status_ilt);
            return EC_IOCB_LIP_INT;
        }

        status_ilt->cr = PM_RelILT2;
        status_ilt++;
        memset(status_iocb, 0, IOCB_SIZE);
        status_ilt->misc = 0;
        status_iocb->entryType = CTIO7;
        status_iocb->entryCount = 1;
        status_iocb->sysdef = 0;
        status_iocb->iocbhandle = (UINT32)status_ilt;
        status_iocb->nphandle = iltsc->scsi_cdm.scinit;
        status_iocb->timeout = 30;
        status_ilt->isp_defs.isp_iocb_type = 0x17;  /* I am setting this to ctio2 */
        status_ilt->isp_defs.isp_timeout = 40;
        status_iocb->vpid = iltsc->inotify_2400.invpid;

        /* Access port database to find alpa of initiator */
        status_iocb->alpa = isp_handle2alpa(port, iltsc->scsi_cdm.scinit);
        status_iocb->exchangeaddr = iltsc->scsi_cdm.scrxid;
        status_iocb->flags = iltsc->scsi_cdm.scsi_cdm_attr.attribute2400 << 9;  /* 4 into bits 9-12 */
        status_iocb->ox_id = iltsc->scsi_cdm.scsi_cdm_attr.scox_id;
        status_iocb->flags |= CTIO7_FLAG_MODE1;
        status_iocb->scsi_status = iltXL->fc_xl.xl_scsist;
        status_iocb->flags |= CTIO7_FLAG_SEND_STATUS;

        /* Set residual length values */
        if (iltXL->fc_xl.xl_reslen > 0)
        {
            /* Set underflow indicator bit */
            status_iocb->scsi_status |= 0x0800; //bit 11
            status_iocb->residXferLen = iltXL->fc_xl.xl_reslen;
        }
        if (iltXL->fc_xl.xl_reslen < 0)
        {
            /* Set overflow indicator bit */
            status_iocb->scsi_status |= 0x0400; //bit 10
            /* Convert data to positive value and save */
            status_iocb->residXferLen = -iltXL->fc_xl.xl_reslen;
        }

        update_rqip(port, status_rqip);
        isp_thread_ilt(port, status_ilt);
    }
    else
    {
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)status_ilt);
#endif /* M4_DEBUG_ILT */
        put_ilt(status_ilt);
    }

    return EC_OK;
}


/**
****************************************************************************
**
**  @brief     fill in a ntack iocb
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      iocb    - iocb to fill
**
**  @return     none
**
******************************************************************************
**/
void isp2400_build_ntack(UINT16 port, ILT * ilt, ISP2400_NTACK * iocb)
{
    ISP2400_CTIO7_MODE0 *iocb0 = (ISP2400_CTIO7_MODE0 *)iocb;
    ISP2400_ABTS_ACK *iocbabts = (ISP2400_ABTS_ACK *)iocb;

    ilt->inotify_2400.inrcn = 0;
    ilt->inotify_2400.incommand = NTACK;
    ilt->cr = NULL;

    /* Clear iocb */
    memset(iocb, 0, IOCB_SIZE);

    if (ilt->inotify_2400.fakeinotify == CTIO7)
    {
        /* Need to make a CTIO instead we were tricking magdriver */

        //  fprintf(stderr, "fakeinotify CTIO7\n");
        iocb0->entryType = CTIO7;
        iocb0->entryCount = 1;
        iocb0->sysdef = SYS_DEF_CTIO7_NOILTNEST;
        iocb0->nphandle = ilt->scsi_cdm.scinit;
        iocb0->timeout = 10;
        iocb0->vpid = ilt->scsi_cdm.scvpid;
        iocb0->dseg_count = 0;

        iocb0->alpa = isp_handle2alpa(port, ilt->scsi_cdm.scinit);
        iocb0->exchangeaddr = ilt->inotify_2400.inseqid;
        iocb->flags = ilt->scsi_cdm.scsi_cdm_attr.attribute2400 << 9;   /* 4 into bits 9-12 */
        iocb0->flags |= CTIO7_FLAG_SEND_STATUS;
        iocb0->ox_id = ilt->inotify_2400.inox_id;
        iocb0->scsi_status = 0;
        ilt--;
        ilt->scsi_cdm.sccommand = ISP_SCSI_CMD_TMF;
        ilt->cr = (void *)C_label_referenced_in_i960asm(isp2400_targetiocb);
        ilt++;
        iocb0->iocbhandle = (UINT32)ilt;
    }
    else if (ilt->inotify_2400.fakeinotify == ABTSRCV)
    {
        // fprintf(stderr, "fakeinotify ABTSRCV ilt %p\n", ilt);
        UINT8       tempalpa[3];

        ilt->scsi_cdm.sccommand = ABTSACK;

        /* Will add case for ABTS */
        iocbabts->entryType = ABTSACK;
        iocbabts->entryCount = 1;
        iocbabts->nphandle = ilt->inotify_2400.ininit;
        iocbabts->rcv_exchangeaddr = ilt->inotify_2400.inseqid;
        iocbabts->iocbhandle = (UINT32)ilt;
        ilt--;
        iocbabts->softype = ilt->ilt_normal.w0;
        memcpy(iocbabts->d_id, &ilt->ilt_normal.w1, 24);

        /* Need to swap did and sid */
        memcpy(tempalpa, iocbabts->d_id, 3);
        memcpy(iocbabts->d_id, iocbabts->s_id, 3);
        memcpy(iocbabts->s_id, tempalpa, 3);

        /* Fixup r_ctl to bs_acc */
        iocbabts->r_ctl = 0x84;
        iocbabts->f_ctl[2] = 0x98;      /* set fcntl bit 16 */
        iocbabts->seq_id = 0xFF;
        iocbabts->payload[0] = 0x00000000;
        iocbabts->payload[1] = ilt->ilt_normal.w5;
        iocbabts->payload[2] = 0x0000FFFF;
        iocbabts->abort_exchange = ilt->ilt_normal.w7;

        /* Update ilt fields */
        ilt->cr = PM_RelILT2;
    }
    else
    {
        // fprintf(stderr, "NTACK to FW\n");
        /* Fill in values */
        iocb->entryType = NTACK;
        iocb->entryCount = 1;
        iocb->iocbhandle = (UINT32)ilt;
        iocb->nphandle = ilt->inotify_2400.ininit;
        iocb->flags = 0;
        iocb->status = ilt->inotify_2400.instatus;
        iocb->status_subcode = ilt->inotify_2400.instatsubcode;

        iocb->vpid = ilt->inotify_2400.invpid;
        iocb->ox_id = ilt->inotify_2400.inox_id;
        iocb->rcv_exchangeaddr = ilt->inotify_2400.inseqid;
    }
    /* Caller will place on iocb queue */
}


/**
****************************************************************************
**
**  @brief  Clean up imt commands for 2400
**
**  @param  port - QLogic chip instance
**  @param  imt - Pointer to imt to clean up
**
**  @return none
**
******************************************************************************
**/
static void isp2400_cleanup_imt(UINT8 port, IMT * imt)
{
    ILT        *ilt;
    ILMT       *ilmt;
    UINT32      i;

    for (i = 0; i < MAX_LUNS; ++i)      /* Process all of the LUNs on this target */
    {
        ilmt = imt->ilmtDir[i];
        if (!ilmt)
        {
            continue;
        }

        for (ilt = ilmt->whead; ilt; ilt = ilt->fthd)
        {
            --ilt;              /* ilt is at level 2 */
            isp_abort_exchange_ilt(port, ilt);
            ++ilt;              /* Put back so fthd works */
        }
    }
}


/**
****************************************************************************
**
**  @brief  Process immediate notify response iocb
**
**          The command is started by the caller. ISP$receive_io
**
**  @param  port - QLogic chip instance (0-3)
**  @param  iocb - iocb to examine
**
**  @return none
**
******************************************************************************
**/
void isp2400_process_inotify(UINT16 port, ISP2400_INOTIFY * iocb)
{
    ILT        *ilt;
//     fprintf(stderr, "%s: %s port %d status %04X subcode %02X handle %02X alpa %06X\n",
//             __func__, FEBEMESSAGE,port,iocb->status, iocb->status_subcode,
//             iocb->nphandle, iocb->alpa);
    switch (iocb->status_subcode)
    {
        case 0x03:
        case 0x20:
        case 0x05:
        case 0x50:  /*PDISC*/
            break;

        default:
            fprintf(stderr, "%s: %s port %d status %04X subcode %02X handle %02X alpa %06X\n",
                    __func__, FEBEMESSAGE,port,iocb->status, iocb->status_subcode,
                    iocb->nphandle, iocb->alpa);
            break;
    }

    if (iocb->status == ISP2400_INOTIFY_ELS)
    {
        IMT        *imt;

        switch (iocb->status_subcode)
        {
            case FC_ELS_PLOGI:
            case FC_ELS_PRLI:
            case FC_ELS_LOGO:
                imt = cimtDir[port]->imtHead;
                while (imt && imt->fcAddr != iocb->nphandle)
                {
                    imt = imt->link;
                }
                if (imt)
                {
                    isp2400_cleanup_imt(port, imt);
                }

                break;

            default:
                break;
        }
    }

    ilt = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    ilt->cr = PM_RelILT2;
    ilt++;
    ilt->inotify_2400.incommand = ISP_IMMED_NOTIFY_CMD;
    ilt->inotify_2400.inchipi = port;
    ilt->inotify_2400.inlun = 0;
    ilt->inotify_2400.ininit = iocb->nphandle;
    ilt->inotify_2400.invpid = iocb->vpid;
    ilt->inotify_2400.instatus = iocb->status;
    ilt->inotify_2400.instatsubcode = iocb->status_subcode;
    ilt->inotify_2400.inseqid = iocb->rcv_exchangeaddr;
    ilt->inotify_2400.inrcn = 0;
    ilt->inotify_2400.inox_id = iocb->ox_id;
    ilt->inotify_2400.fakeinotify = 0;
    ilt[1].misc = (UINT32)ilt;
    ilt->cr = PM_RelILT2;
    ISP_NotifyAck(port, ilt);
}


/**
****************************************************************************
**
**  @brief      completion routine for target io
**
**              if a command was aborted we need to cancel the exchange
**              on the hw by generating a CTIO7
**
**  @param      port    - QLogic chip instance (0-3)
**  @param      ilt     - completed ilt at in1 lvl;
**
**  @return     none
**
******************************************************************************
**/
void isp2400_targetiocb(UINT32 status UNUSED, ILT * ilt)
{
    IMT        *imt;
    UINT16      port = ilt->scsi_cdm.scchipi;
    if (ilt->scsi_cdm.sccommand == ISP_SCSI_CMD)
    {
        imt = (IMT *)ilt->scsi_cdm.imt;
        if (imt)
        {
// ? crash -- imt is freed, but this uses it.
            imt->qDepth--;
        }
        /*don't decrement if 0 */
        if (hba_q_cnt[port])
        {
            hba_q_cnt[port]--;
        }
        ilt--;
        if (ilt->scsi_2400_dsd.isp2400_dsdlist)
        {
            s_Free(ilt->scsi_2400_dsd.isp2400_dsdlist,
                 sizeof(DSEG_DESC) * (ilt->scsi_2400_dsd.isp2400_sgdesc_count + 1), __FILE__, __LINE__);
            ilt->scsi_2400_dsd.isp2400_dsdlist = NULL;
            ilt->scsi_2400_dsd.isp2400_sgdesc_count = 0;
        }
    }

#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    put_ilt(ilt);
}


static void isp2400_gen_queue_full(UINT16 port, ILT * ilt)
{
    UINT32      rqip;
    ISP2400_CTIO7_MODE1 *iocb;

    // fprintf(stderr, "%s: exchange %08X\n", __func__, ilt->scsi_cdm.scrxid);
    iocb = (ISP2400_CTIO7_MODE1 *)isp_get_iocb(port, &rqip);
    if (iocb == NULL)
    {
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
        put_ilt(ilt);
        return;
    }

    memset(iocb, 0, IOCB_SIZE);

    iocb->entryType = CTIO7;
    iocb->entryCount = 1;
    iocb->sysdef = SYS_DEF_CTIO7_NOILTNEST;
    iocb->nphandle = ilt->scsi_cdm.scinit;
    iocb->timeout = 10;
    iocb->vpid = ilt->scsi_cdm.scvpid;
    iocb->alpa = isp_handle2alpa(port, ilt->scsi_cdm.scinit);
    iocb->exchangeaddr = ilt->scsi_cdm.scrxid;
    iocb->flags = ilt->scsi_cdm.scsi_cdm_attr.attribute2400 << 9;       /* 4 into bits 9-12 */
    iocb->flags |= CTIO7_FLAG_SEND_STATUS;
    iocb->flags |= CTIO7_FLAG_MODE1;
    iocb->ox_id = ilt->scsi_cdm.scsi_cdm_attr.scox_id;
    iocb->scsi_status = SCS_QUEF;

    /* Submit IOCB by updating Request Queue IN pointer */

    ilt->scsi_cdm.imt = 0;
    ilt->cr = (void *)C_label_referenced_in_i960asm(isp2400_targetiocb);
    ilt++;
    iocb->iocbhandle = (UINT32)ilt;

    update_rqip(port, rqip);

    ilt->isp_defs.isp_iocb_type = CTIO7;
    ilt->isp_defs.isp_timeout = 15;
    isp_thread_ilt(port, ilt);
}


/**
******************************************************************************
**
**  @brief  Abort an exchange
**
**  @param  port - Point number
**  @param  exchange - Address of the exchange to abort
**  @param  nphandle - Handle to initiator
**  @param  vpid - Virtual port id
**  @param  ox_id
**  @param  attribute2400
**  @param  alpa
**
**  @return none
*/
static void isp2400_abort_exchange(UINT16 port, UINT32 exchange,
                                   UINT16 nphandle, UINT16 vpid, UINT16 ox_id,
                                   UINT8 attribute2400, UINT32 alpa)
{
    UINT32      rqip;
    ISP2400_CTIO7 *iocb;
    ILT        *abortilt;

    abortilt = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)abortilt);
#endif /* M4_DEBUG_ILT */
    iocb = (ISP2400_CTIO7 *)isp_get_iocb(port, &rqip);
    if (iocb == NULL)
    {
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)abortilt);
#endif /* M4_DEBUG_ILT */
        put_ilt(abortilt);
        return;
    }

    memset(iocb, 0, IOCB_SIZE);
    iocb->entryType = CTIO7;
    iocb->entryCount = 1;
    iocb->sysdef = SYS_DEF_CTIO7_NOILTNEST;
    iocb->nphandle = nphandle;
    iocb->timeout = 10;
    iocb->vpid = vpid;

    if (nphandle == 0xFFFF && vpid == 0xFF)
    {
        iocb->alpa = alpa;
    }
    else
    {
        iocb->alpa = isp_handle2alpa(port, nphandle);
    }
    iocb->exchangeaddr = exchange;
    iocb->flags = attribute2400 << 9;   /* 4 into bits 9-12 */
    iocb->flags |= CTIO7_FLAG_TERM_XCHG;
    iocb->ox_id = ox_id;
    abortilt->scsi_cdm.sccommand = CTIO7;
    abortilt->cr = (void *)C_label_referenced_in_i960asm(isp2400_targetiocb);

#if ISP_DEBUG_INFO
    fprintf(stderr, "%s PORT %d nphandle %04X exchange %08X vpid %02X\n",
            __func__, port, nphandle, exchange, vpid);
#endif /* ISP_DEBUG_INFO */

    /* Submit IOCB by updating Request Queue IN pointer */

    abortilt++;
    iocb->iocbhandle = (UINT32)abortilt;

    update_rqip(port, rqip);

    abortilt->isp_defs.isp_iocb_type = CTIO7;
    abortilt->isp_defs.isp_timeout = 15;
    isp_thread_ilt(port, abortilt);
}


/**
******************************************************************************
**
**  @brief  Abort an exchange identified by an ILT
**
**  @param  port - Port number
**  @param  ilt - ilt associated with the exchange to abort
**
**  @return none
*/
void isp_abort_exchange_ilt(UINT16 port, ILT * ilt)
{
    isp2400_abort_exchange(port, ilt->scsi_cdm.scrxid, ilt->scsi_cdm.scinit,
                           ilt->scsi_cdm.scvpid, ilt->scsi_cdm.scsi_cdm_attr.scox_id,
                           ilt->scsi_cdm.scsi_cdm_attr.attribute2400, 0);
}


/**
******************************************************************************
**
**  @brief  Abort an unknown exchange
**
**  @param  port - Port number
**  @param  exchange - exchange to abort
**  @param  ox_id - another id
**  @param  attribute2400
**  @param  alpa
**
**  @return none
*/
static void isp2400_abort_exchange_unknown(UINT16 port, UINT32 exchange,
                                           UINT16 ox_id, UINT8 attribute2400, UINT32 alpa)
{
    isp2400_abort_exchange(port, exchange, 0xFFFF, 0xFF, ox_id, attribute2400, alpa);
}


/**
******************************************************************************
**
**  @brief  Abort an exchange identified by an iocb ABTS
**
**  @param  port - Port number
**  @param  iocb - iocb associated with the exchange to abort
**  @param  nphandle - nphandle to use to abort
**  @param  vpid - Virtual port id associated with the exchange
**
**  @return none
*/
static void isp2400_abort_exchange_iocb_abts(UINT16 port, ISP2400_ABTS * iocb,
                                             UINT16 nphandle, UINT16 vpid)
{
    isp2400_abort_exchange(port, iocb->abort_exchange, nphandle, vpid, iocb->ox_id, 0, 0);
}



static void isp2400_tmfclearlun(UINT16 port, ILMT * ilmt)
{
    ILT        *ilt;

//    fprintf(stderr, "%s: ILMT %p\n", __func__, ilmt);
    /* Tasks are stored at inl2 nest level */
    /* Oddly this ilt list is null terminated */
    for (ilt = ilmt->whead; ilt != NULL; ilt = ilt->fthd)
    {
        ilt--;
        fprintf(stderr, "%s: <DO LOOP> ILT %p\n", __func__, ilt);
        if (ilt->scsi_cdm.sccommand == ISP_SCSI_CMD)
        {
            isp_abort_exchange_ilt(port, ilt);
        }
        ilt++;
    }
}


/**
****************************************************************************
**
**  @brief  Process TMF
**
**          Cancels active exchanges on the card as appropriate
**          for TMF type.
**
**  @param  port    - QLogic chip instance (0-3)
**  @param  ilt    -  TMF we received.
**
**  @return none
**
******************************************************************************
**/
static void isp2400_tmfprocess(UINT16 port, ILT * ilt)
{
    IMT        *imt;
    ILMT       *ilmt;
    UINT32      i;

    /* Get the IMT */

    /* Target reset reset everything */
    if (ilt->scsi_cdm.sctaskf == 0x20)
    {
        /* Walk imts */
        for (imt = cimtDir[port]->imtHead; imt != NULL; imt = imt->link)
        {
            /* Walk luns */

            for (i = 0; i < MAX_LUNS; i++)
            {
                ilmt = imt->ilmtDir[i];
                if (ilmt != NULL)
                {
                    isp2400_tmfclearlun(port, ilmt);
                }
            }
        }
    }
    else
    {
        for (imt = cimtDir[port]->imtHead; imt != NULL; imt = imt->link)
        {
            if (imt->fcAddr == ilt->scsi_cdm.scinit)
            {
                break;
            }
        }

        if (imt)
        {
            /* Clear aca */
            if (ilt->scsi_cdm.sctaskf == 0x40)
            {
                /* do nothing */
            }
            /* LUN reset */
            /* clear task set */
            /* abort task set */
            if ((ilt->scsi_cdm.sctaskf & 0x16) && ilt->scsi_cdm.sclun < MAX_LUNS)
            {
                /* Get luns */
                ilmt = imt->ilmtDir[ilt->scsi_cdm.sclun];
                if (ilmt != NULL)
                {
                    isp2400_tmfclearlun(port, ilmt);
                }
            }
        }
    }
}


/**
****************************************************************************
**
**  @brief  Empty atio queue
**
**  @param  port - QLogic chip instance (0-3)
**
**  @return none
**
******************************************************************************
**/
static void isp2400_empty_atio_que(UINT16 port)
{
    struct ISP_2400 *pISP24 = ispstr[port]->baseAd;
    UINT16      tmpout;
    UINT32      work;
    UINT32      hwin;
    int         rc;

    /*
     ** First, nullify any queued-up interrupt work for the atio
     ** queue, because we will do it all from here.
     */
    if (BIT_TEST(ispfail, port) || !BIT_TEST(isprena, port))
    {
        return;
    }

    tmpout = isp2400_queue[port].out;
    hwin = pISP24->atioQIP;
    while (tmpout != isp2400_queue[port].in)
    {
        work = isp2400_queue[port].work[tmpout];
        switch (work & 0xFF)
        {
            case ISP2400RHS_STATUS_ATIO:
                isp2400_queue[port].work[tmpout] = 0;   /* Nullify operation */
                break;

            default:
                break;
        }
        ++tmpout;
        if (tmpout >= MAX_2400_WORK)
        {
            tmpout = 0;
        }
    }

    rc = isp2400_process_atio_que(port, hwin << 4, sessids[port]);
    if (rc > 0)                 /* If work was done, update hardware out pointer */
    {
        QCB        *atioQ = (QCB *)ispstr[port]->atioQue;

        FORCE_WRITE_BARRIER;
        pISP24->atioQOP = (atioQ->out - atioQ->begin) >> 4;     /* Write register */
    }
}


/**
****************************************************************************
**
**  @brief  Process ABTS
**
**          Process an abort task. Kill the io on the card
**          then generate a Inotify ilt to send to uppper layer.
**
**  @param  port - QLogic chip instance (0-3)
**  @param  iocb - iocb to examine
**
**  @return none
**
******************************************************************************
**/
static void isp2400_process_abts(UINT16 port, ISP2400_ABTS * iocb)
{
    ILT        *ilt;
    ILT        *iltsc = NULL;
    IMT        *imt;
    ILMT       *ilmt;
    UINT32      i;
    UINT32      rqip;
    UINT8       vpid;
    ISP2400_ABTS_ACK *iocbabts;
    TAR        *pTar;
    UINT32      alpa_t;
//    UINT32      alpa_i;

    /* First, empty the atio queue, per QLogic spec */

    isp2400_empty_atio_que(port);

    /* Find ilt to abort */
    alpa_t = (UINT32)(iocb->d_id[0] | (iocb->d_id[1] << 8) | (iocb->d_id[2] << 16));
//    alpa_i = (UINT32)(iocb->s_id[0] | (iocb->s_id[1] << 8) | (iocb->s_id[2] << 16));
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        if (pTar->portID == alpa_t)
        {
            break;
        }
    }

    if (pTar)
    {
        vpid = pTar->vpID;
    }
    else
    {
        vpid = 0xFF;
    }

//    fprintf(stderr, "%s PORT %d  - I alpa: %06X T alpa: %06X abort EX: %08X handle: %04X\n",
//            __func__, port, alpa_i, alpa_t, iocb->abort_exchange, iocb->nphandle);

    if (iocb->abort_exchange != 0xFFFFFFFF)
    {
        /* Chug through ilmt imt tables */
        iltsc = NULL;
        for (imt = cimtDir[port]->imtHead; imt != NULL; imt = imt->link)
        {
            if (imt->fcAddr == iocb->nphandle)
            {
                break;
            }
        }

        if (imt)
        {
            //fprintf(stderr, "%s: matched imt %p\n", __func__, imt);
            /* I dont have a lun value so I have to search everything */
            for (i = 0; i < MAX_LUNS; ++i)
            {
                ilmt = imt->ilmtDir[i];
                if (ilmt == NULL)
                {
                    continue;
                }

                for (ilt = ilmt->whead; ilt != NULL; ilt = ilt->fthd)
                {
                    /* ilt is at lvl 2 */
                    ilt--;
                    if (ilt->scsi_cdm.scrxid == iocb->abort_exchange)
                    {
                        iltsc = ilt;
                        break;
                    }
                    /* Put back so fthd works */
                    ilt++;
                }

                if (iltsc)
                {
                    break;
                }
            }
        }
    }

    ilt = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    if (iltsc && iltsc->scsi_cdm.scrxid == iocb->abort_exchange)
    {
#if ISP_DEBUG_INFO
        fprintf(stderr, "%s matched an ILT %p\n", __func__, iltsc);
#endif /* ISP_DEBUG_INFO */

        isp_abort_exchange_ilt(port, iltsc);
        ilt->ilt_normal.w0 = iocb->softype;
        memcpy(&(ilt->ilt_normal.w1), iocb->d_id, 24);
        ilt->ilt_normal.w7 = iocb->abort_exchange;

        /* Build fake inotify ilt to make upper layers behave correctly */
        ilt++;

        ilt->inotify_2400.incommand = ISP_IMMED_NOTIFY_CMD;
        ilt->inotify_2400.inchipi = port;
        ilt->inotify_2400.inlun = iltsc->iscsi_def.lun;
        ilt->inotify_2400.ininit = iocb->nphandle;
        ilt->inotify_2400.invpid = iltsc->scsi_cdm.scvpid;
        ilt->inotify_2400.inseqid = iocb->rcv_exchangeaddr;
        ilt->inotify_2400.instatus = 0x20;      /* Abort task */
        ilt->inotify_2400.instatsubcode = 0;
        ilt->inotify_2400.fakeinotify = ABTSRCV;
        ilt->inotify_2400.inrcn = 0;
        ilt->inotify_2400.inflags = 0;
        ilt->inotify_2400.inox_id = iocb->ox_id;
        ilt->cr = NULL;
        ilt[1].misc = (UINT32)ilt;

        ilt++;
        ilt->cr = NULL;
        C_recv_scsi_io(ilt);
        return;
    }

#if ISP_DEBUG_INFO
    fprintf(stderr, "%s No ILT matched\n", __func__);
#endif /* ISP_DEBUG_INFO */

    if (iocb->abort_exchange != 0xFFFFFFFF)
    {
        isp2400_abort_exchange_iocb_abts(port, iocb, iocb->nphandle, vpid);
    }

    iocbabts = (ISP2400_ABTS_ACK *)isp_get_iocb(port, &rqip);
    if (!iocbabts)
    {
        fprintf(stderr, "%s: Failed to get iocb\n", __func__);
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
        put_ilt(ilt);
        return;
    }

    memset(iocbabts, 0, IOCB_SIZE);
    iocbabts->entryType = ABTSACK;
    iocbabts->entryCount = 1;
    iocbabts->nphandle = iocb->nphandle;
    // iocbabts->controlflags = 0x0001;  /* Abort exchange nope this aborts the abts not the exchange we want*/
    iocbabts->softype = iocb->softype;
    iocbabts->rcv_exchangeaddr = iocb->rcv_exchangeaddr;
    iocbabts->r_ctl = 0x84;
    iocbabts->d_id[0] = iocb->s_id[0];
    iocbabts->d_id[1] = iocb->s_id[1];
    iocbabts->d_id[2] = iocb->s_id[2];

    iocbabts->s_id[0] = iocb->d_id[0];
    iocbabts->s_id[1] = iocb->d_id[1];
    iocbabts->s_id[2] = iocb->d_id[2];

    iocbabts->f_ctl[2] = 0x98;

    iocbabts->seq_id = 0xFF;
    iocbabts->seq_cnt = iocb->seq_cnt + 1;
    iocbabts->ox_id = iocb->ox_id;
    iocbabts->rx_id = iocb->rx_id;
    iocbabts->parameter = iocb->parameter;
    iocbabts->abort_exchange = iocb->abort_exchange;
    iocbabts->payload[0] = 0x00000000;
    iocbabts->payload[1] = iocb->rx_id | (iocb->ox_id << 16);
    iocbabts->payload[2] = 0x0000FFFF;

    ilt->cr = PM_RelILT2;
    ilt++;
    iocbabts->iocbhandle = (UINT32)ilt;
    ilt->isp_defs.isp_iocb_type = ABTSACK;
    ilt->isp_defs.isp_timeout = 15;
    isp_thread_ilt(port, ilt);

    /* Submit IOCB by updating Request Queue IN pointer */

    update_rqip(port, rqip);
}

/**
 ******************************************************************************
 **
 **  @brief      isp_queue_rcviof
 **             queues an ilt for the rcvio exec process
 **
 **  @param      port     - Port number
 **  @param      ilt      - ilt to queueu
 **
 ******************************************************************************
 **/
static void isp_queue_rcviof(UINT8 port,ILT * ilt)
{
    QU_EnqueReqILT(ilt,&isp_RCVIO_queue[port]);
}
/**
 ******************************************************************************
 **
 **  @brief     ISP_RCV_IO_Queue_exec
 **             drains the isp_RCVIO_queue
 **
 **  @param      port     - Port number
 **
 ******************************************************************************
 **/
NORETURN
void ISP_RCV_IO_Queue_exec(UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port)
{
    ILT * ilt;
    while (1)
    {
        if (isp_RCVIO_queue[port].head == NULL)
        {
            TaskSetMyState(PCB_NOT_READY);
            TaskSwitch();
            continue;
        }
        ilt = QU_DequeReqILT(&isp_RCVIO_queue[port]);
        if (ilt != NULL)
        {
            if (ilt->inotify_2400.incommand == ISP_IMMED_NOTIFY_CMD)
            {
                isp2400_tmfprocess(port, ilt);
            }
            ilt++;
            ilt->cr = NULL;
            C_recv_scsi_io(ilt);
        }
    }
}

/**
****************************************************************************
**
**  @brief  process atio iocb
**
**          This is where new commands received by the qlogic card
**          are first processed by the isp layer.
**
**  @param  port - QLogic chip instance (0-3)
**  @param  iocb - iocb to examine
**
**  @return none
**
******************************************************************************
**/
void isp2400_process_atio7(UINT16 port, ISP2400_ATIO7 * iocb)
{
    ILT        *ilt;
    ILT        *ilt0;
    UINT32      alpa_t,
                alpa_i;
    UINT16      nphandle_i = NO_LID;
    TAR        *pTar;

    if (iocb->entryCount > 1)
    {
        fprintf(stderr, "%s: continuation IOCB\n", __func__);
    }

    intlock[port] = 0;          /* Clear interlock for this instance */
    req_cnt++;                  /* bump request counter */
    hba_q_cnt[port]++;          /* Increment the HBA queue counter */

    /* This isnt so bad since it is only couple deep and at worst 8 deep */
    alpa_t = (UINT32)(iocb->d_id[2] | (iocb->d_id[1] << 8) | (iocb->d_id[0] << 16));
    alpa_i = (UINT32)(iocb->s_id[2] | (iocb->s_id[1] << 8) | (iocb->s_id[0] << 16));
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {

        if (pTar->portID == alpa_t)
        {
            break;
        }
    }
    if (pTar != NULL)
    {
        nphandle_i = isp_alpa2handle(port, alpa_i, pTar->vpID);
        if (nphandle_i != NO_LID && dbflags[port][nphandle_i] == FALSE)
        {
            ISP_GetPortDB(port, nphandle_i, pTar->vpID);
        }
    }
    if (pTar != NULL && nphandle_i != NO_LID)
    {
        ilt = get_ilt();
        ilt->scsi_2400_dsd.ts = timestamp;

        ilt0 = ilt;
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
        ilt++;
        ilt->scsi_cdm.sccommand = ISP_SCSI_CMD;
        ilt->scsi_cdm.scchipi = port;
        ilt->scsi_cdm.sclun = (iocb->iu.lun[0] << 8) | iocb->iu.lun[1];
        ilt->scsi_cdm.scinit = nphandle_i;
        ilt->scsi_cdm.scvpid = pTar->vpID;
        /*
         **  note this isn't really the rx_id but the address
         **  provided by the 2400 that is used for exchange tracking
         */
        ilt->scsi_cdm.scrxid = iocb->exchangeaddr;

        ilt->scsi_cdm.cdb = (UINT32)ilt0->scsi_2400_dsd.cdb;
        memcpy(ilt0->scsi_2400_dsd.cdb,iocb->iu.cdb,16);
        ilt->scsi_cdm.sctaskc = iocb->iu.tag_mode;
        ilt->scsi_cdm.scexecc = iocb->iu.rw_flags;
        ilt->scsi_cdm.sctaskf = iocb->iu.task_flags;
        ilt->scsi_cdm.scdatalen = ntohl(iocb->iu.fcp_dl);
#if ISP2400_TARGET_IO_LOG
        fprintf(stderr, "%s: PORT %d I %06X T %06X lun %04X nphandle %04X vpid %02X exchange %08X\n",
                __func__, port, alpa_i, alpa_t, ilt->scsi_cdm.sclun, nphandle_i,
                ilt->scsi_cdm.scvpid, iocb->exchangeaddr);
        fprintf(stderr, "    cdb ");
        int i;
        for (i = 0; i < 16; ++i)
        {
            fprintf(stderr, "%02hX ", iocb->iu.cdb[i]);
        }
        fprintf(stderr, "  DL %08X\n", ilt->scsi_cdm.scdatalen);
#endif /* ISP2400_TARGET_IO_LOG */
        ilt->scsi_cdm.scrcn = 1;
        ilt->cr = (void *)C_label_referenced_in_i960asm(isp2400_targetiocb);
        ilt[1].misc = (UINT32)ilt;
        ilt->scsi_cdm.scsi_cdm_attr.attribute2400 = iocb->fcpcmdlen >> 12;
        ilt->scsi_cdm.scsi_cdm_attr.scox_id = ntohs(iocb->ox_id);
        if (iocb->exchangeaddr == 0xffffffff)
        {
            isp2400_gen_queue_full(port, ilt);
        }
        else
        {
            if (ilt->scsi_cdm.sctaskf)
            {
                /*
                 *   Time to lie to upper layer the 2400 handle TMF differently than the 2300
                 *   need to generate am immediate notify command to make magdrv
                 *   behave properly on a TMF.
                 */
//                fprintf(stderr, "%s: TMF flags set %08X ILT %p\n",
//                        __func__, ilt->scsi_cdm.sctaskf, ilt);
                ilt->inotify_2400.incommand = ISP_IMMED_NOTIFY_CMD;
                ilt->inotify_2400.instatus = 0x36;      /* TMF flags set */
                ilt->inotify_2400.instatsubcode = 0;
                ilt->inotify_2400.inrcn = 0;
                ilt->inotify_2400.inflags = ilt->scsi_cdm.sctaskf;
                ilt->inotify_2400.inox_id = ntohs(iocb->ox_id);
                ilt->inotify_2400.fakeinotify = CTIO7;
                ilt->cr = NULL;
                /* Unincrement counters */
                req_cnt--;
                hba_q_cnt[port]--;
#if 0
                isp_queue_rcviof(port,ilt);
            }
            else
            {
                ilt++;
                ilt->cr = NULL;
                C_recv_scsi_io(ilt);
            }
#else
            }
            isp_queue_rcviof(port,ilt);
#endif
        }
    }
    else
    {
//        UINT32      lid;
//        UINT32      retval;

#if 0
        if (pTar == NULL)
        {
            fprintf(stderr, "%s: ERROR received command for unknown initiator/target i:%06X T:%06X TAR: %p exchg: %08X\n",
                    __func__, alpa_i, alpa_t, pTar, iocb->exchangeaddr);
            for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
            {
                fprintf(stderr, "       Tid %d vpid %d tar portid %06X  \n",pTar->tid,pTar->vpID,pTar->portID);
            }
        }
        else
        {
            fprintf(stderr, "%s: ERROR received command for unknown initiator/target i:%06X T:%06X vpid: %d exchg: %08X\n",
                    __func__, alpa_i, alpa_t, pTar->vpID, iocb->exchangeaddr);
        }
#endif  /* 0 */
        isp2400_abort_exchange_unknown(port, iocb->exchangeaddr, iocb->ox_id,
                                       (iocb->fcpcmdlen & 0xF000) >> 3, alpa_i);
//        lid = I_get_lid(port);
//        if (pTar != NULL)
//        {
//            retval = ISP_LoginFabricPortVPID(port, &lid, alpa_i, pTar->vpID);
//        }
//        else
//        {
//            retval = ISP_LoginFabricPort(port, &lid, alpa_i);
//        }
//        fprintf(stderr, "%s: port %d Called fabric login returns %04X\n",
//                __func__, port, retval);
    }
}
#endif /* FRONTEND */


/**
******************************************************************************
**
**  @brief      Get, Setup and execute the Set Firmware Options mailbox command (28h,38h)
**
**              GetAdditionalFirmware options command reads the current state
**              state of the firmware options set by the Set Additional
**              Firmware options command.
**
**              Note:(SetAdditional Firmware Options Command, must be issued
**              before the initialization of the Firmware ICB (Initialization Contro Block),
**              which is done, in ISP2400_InitFW()function. If this command is issued
**              after the initialization of the Firmware, (the case we are discussing here),
**              then SetAdditional Firmware Options Command, handles only Disable LED Control bit.
**
**              This function enables the LED control bit.
**              (If LED control bit is enabled, the system driver has to handle the
**               LED control pins(GPIOD Register bits 4-2), if is disabled the firmware
**               handles the LED control bits(Default case for our ISP).
**
**
**  @param      port    - QLogic chip instance (0-3)
**
**  @return     Completion status
**
******************************************************************************
**/
UINT16 isp2400_GetAndSetFirmwareOptions(UINT8 port)
{
    QRP        *qrp;
    UINT16      retValue;

    qrp = get_qrp();            /* Allocate a QRP to issue mailbox commands */

    qrp->imbr[0] = ISP_GFO;     /* Execute Get Firmware options */
    qrp->iRegs = 0x1;           /* set modify mailbox reg 0      */
    qrp->oRegs = 0xF;           /* set retrieve 0-3 mailbox regs */

    isp_exec_cmd_sri(port, qrp, TRUE);

    if (qrp->ombr[0] == ISP_CMDC)       /* Did the command complete successfully? */
    {
        BIT_SET(qrp->ombr[1], ISP_DISABLE_LED);

        qrp->imbr[1] = qrp->ombr[1];

        qrp->imbr[0] = ISP_SFO; /* Execute Set Firmware options */
        qrp->iRegs = 0xF;       /* set modify mailbox reg 0-3     */
        qrp->oRegs = 0x1;       /* set retrieve 0 mailbox regs    */

        isp_exec_cmd_sri(port, qrp, TRUE);
    }

    if (qrp->ombr[0] == ISP_CMDC)       /* If normal completion */
    {
        retValue = 0;
    }
    else
    {
        /* Return set firmware options failed */

        retValue = ISP_SET_FW_OPT_ERROR;
    }

    put_qrp(qrp);               /* Release QRP */

    return retValue;
}


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      Processes Virtual Port IOCB received from ISP firmware.
**
**  @param      port - QLogic chip instance (0-3)
**  @param      iocb - IOCB pointer.
**
**  @return     return status
**
******************************************************************************
**/
static UINT32 isp2400_processVpControl(UINT8 port, ISP2400_VPCONTROL * iocb)
{
    UINT16      retValue = DEOK;
    UINT32      i;
    TAR        *pTar;
    ILT        *ilt;

    ilt = (ILT *)iocb->handle;  /* Get value of ILT from IOCB */
    if (!verify_iocbhandle(ilt))
    {
        ILT *tmpILT = (ILT*)bswap_32((UINT32)ilt);
        if (!verify_iocbhandle(tmpILT))
        {
            fprintf(stderr, "%s%s:%u %s nphandle is not an ILT (%p), processing as zero...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            ilt = 0;
        }
        else
        {
            fprintf(stderr, "%s%s:%u %s nphandle was byteswapped (%p), correcting...\n", FEBEMESSAGE, __FILE__, __LINE__, __func__, ilt);
            ilt = tmpILT;
        }
    }

    if (ilt)                    /* Check if ILT is defined */
    {
        ilt->ilt_normal.w0 = iocb->status;      /* Store completion status in IOCB */

        if (iocb->status != 0)  /* Check for GOOD completion status */
        {
            retValue = 94;      /* Indicate Bad completion status */
        }
    }
    else
    {
        retValue = 95;          /* ILT in IOCB is invalid */
    }

#ifdef DEBUG_FLT_REC_ISP
    MSC_FlightRec(0x101F, port, iocb->vpCount, iocb->status);
#endif /* DEBUG_FLT_REC_ISP */

    if (iocb->vpCount == 0)     /* If no virtual port ID entries */
    {
        return retValue;
    }

    ISP_GetLoopId(port,0);        /* Get loop ID and port ID */

    /* If tar list gets changed, we might need to retry. */
  restart:
    /* Traverse the target list for this port */
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        for (i = 1; i < iocb->vpCount; ++i)
        {
            /* Is loop ID valid? */
            if (!BIT_TEST(iocb->vpindexmap[(i - 1) / VPINDX_MAPSIZE], ((i - 1) % VPINDX_MAPSIZE)))
            {
                continue;
            }

            if (i != pTar->entry)
            {
                continue;
            }

            if (iocb->command == 0)
            {
                /* Update the port assignment for this target */
                ispPortAssignment[pTar->tid] = port;

                /* Indicate target is enabled */
                BIT_SET(pTar->opt, TARGET_ENABLE);
            }
            else
            {
                /* NOTE: get_ilt, maybe C_recv_scsi_io may task switch. */
                UINT32 save_tar_link_abort = tar_link_abort[port];
                /* Is this target currently assigned to this port? */
                if (ispPortAssignment[pTar->tid] == port)
                {
                    /* Update the port assignment for this target */
                    ispPortAssignment[pTar->tid] = 0xFF;
                }

                /* Generate ILT for passage to Translation Layer */
                ilt = get_ilt();        /* get an ILT w/wait */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */

                /* If tar list changed, then the linked list may not be valid, restart. */
                if (save_tar_link_abort != tar_link_abort[port])
                {
                    put_ilt(ilt);       /* Release the ILT, try again. */
                    goto restart;
                }

                ilt->ilt_normal.w0 = ISP_DISABLE_VPORT_CMD;     /* store cmd byte */
                ilt->ilt_normal.w0 |= port << 8;        /* store Chip ID */
                ilt->ilt_normal.w1 = pTar->vpID << 8;   /* store Virtual port ID */
                ilt[1].misc = (UINT32)ilt;
                ilt->cr = NULL; /* No completion handler */
                ++ilt;          /* Get next level of ILT */

                /* No completion routine, cdriver will release ilt */
                ilt->cr = NULL; /* No completion handler    */

                C_recv_scsi_io(ilt);    /* Invoke Translation Layer */

                /* If tar list changed, then the linked list may not be valid, restart. */
                if (save_tar_link_abort != tar_link_abort[port])
                {
                    goto restart;
                }
                /* Indicate target is disabled */
                BIT_CLEAR(pTar->opt, TARGET_ENABLE);

                /* Invalidate Loop ID and Port ID */
                pTar->vpID = NO_CONNECT;
                pTar->portID = NO_PORTID;
            }
        }
    }

    return retValue;
}


/**
******************************************************************************
**  @brief Check to see if the given WWN is a target within the same DSC
**
**  @param wwn
**
**  @return 1 - if yes
**
**  @return 0 - otherwise
**
******************************************************************************
**/
UINT32 ISP_ChkIfPeerTarget(UINT64 wwn)
{
    UINT32      rVal = 0;
    UINT32      wwn_msw;
    UINT32      wwn_lsw;

    if (M_chk4XIO(wwn))
    {
        wwn_lsw = (UINT32)(wwn >> 32);
        wwn_msw = (UINT32)(wwn & 0x00000000ffffffff);
        wwn_msw = ntohl(wwn_msw);
        wwn_lsw = ntohl(wwn_lsw);
        if ((wwn_msw & 0xFFF000F0) == WWN_B_PORT)
        {
            rVal = 0;
        }
        else if ((wwn_msw & 0xFFF000F0) == WWN_F_PORT || (wwn_msw & 0xFFF000F0) == WWN_C_PORT)
        {
            if ((wwn_lsw & 0xFFFF0) == (K_ficb->cSerial & 0x0ffff0))
            {
                rVal = 1;
            }
        }
    }
    return rVal;
}
#endif /* FRONTEND */


/**
******************************************************************************
**
**  @brief  is a given device on the local loop or not
**
**  @param  port - QLogic chip instance (0-3)
**  @param  nphandle - handle to get alpa for
**  @param  vpid - Vport id
**
**  @return 1 - device is local
**
**  @return 0 - device is remote, ie through a switch
**
******************************************************************************
*/
UINT32 isp_isdevicelocal(UINT8 port, UINT32 nphandle, UINT8 vpid UNUSED)
{
    UINT32      device_alpa;

    device_alpa = isp_handle2alpa(port, nphandle) & 0xFFFF00;
    return device_alpa == 0 || device_alpa == (portid[port] & 0xFFFF00);
}

/**
******************************************************************************
**
**  @brief  Check for timeout on ilthead/ilttail list.
**
**  To provide a means of traversing the chain of ILTs that represent
**  outstanding I/Os. This routine is to be called once per time interval.
**  A count is maintained in the ILT representing how many intervals the ILT
**  has been on the thread; if a predetermined threshold is reached the qlogic
**  code is restarted.
**
**  This routine traverses the list of ILTs for a particular chip instance.
**  It decrements a counter stored in byte 1 of <il_w7> in the ILT; this
**  counter is presumed to have been initialized to a default value when the
**  ILT was threaded onto the list. If count reaches zero, reset qlogic chip.
**
**  @param  port - QLogic chip instance (0-3)
**
**  @returns none
**
******************************************************************************
**/
static void isp_check_thread(UINT8 port)
{
    ILT        *tail = (ILT *)&ilttail[port * 2];   /* Get tail "ilt" for this port. */
    QCB        *qcb;            /* For checking unprocessed requests. */
    CMIO7_IOCB *iocb7;          /* Hold the iocb we are looking for ILT in. */
    UINT32     *work;           /* The iocb working on. */
    ILT        *next;           /* The ILT on ilthead/ilttail to check. */
    int         flag;
    ILT        *next_ilt = ilthead[port * 2];   /* initialize -- just in case. */

    for (next = ilthead[port * 2]; next_ilt != NULL && next_ilt != tail; next = next_ilt)
    {
        next_ilt = next->fthd;
#ifdef BACKEND
        if (next->isp_defs.isp_iocb_type == CMIO7)  // Only these are real I/O's to a drive.
        {
            DEV        *dev = next->isp_defs.isp_dev;

            if (dev != NULL && BIT_TEST(dev->flags, DV_QLTIMEOUTEMULATE))
            {
                /* Emulate qlogic timeout. */
                if (next->isp_defs.isp_timeout <= timestamp)
                {
                    PRP        *prp;

                    prp = (PRP *)(next - 2)->ilt_normal.w0;
                    prp->pDev->orc--;
                    prp->pDev->pdd->qd--;

                    isp_unthread_ilt(port, next);       /* Do not do a qlogic reset in any case. */

                    fprintf(stderr, "%s: DV_QLTIMEOUTEMULATE pid=%u ILT=%p PRP=%p (timestamp=%d isp_timeout=%d)\n", millitime_string(), dev->pdd->pid, next, prp, timestamp, next->isp_defs.isp_timeout);

                    next->isp_defs.isp_timeout = 0;     /* force emulated timeout. */
                    prp->qLogicStatus = QLOGIC_STATUS_TIMEOUT;
                    prp->reqStatus = EC_TIMEOUT;
                    prp->scsiStatus = SCS_NORM;

                    /* Timeout passing ILT unnested 1 level. */
                    isp$complete_io(EC_TIMEOUT, next - 1, NULL);
                    continue;
                }
            }
        }
#endif /* BACKEND */
        if (next->isp_defs.isp_timeout >= timestamp)
        {
            continue;           /* continue if no timeout. */
        }

        /* Check if command being timed out is on the response queue. */
        qcb = ispstr[port]->resQue;
        flag = 0;
        work = qcb->out;        /* Start the loop. */
        /* Check if IOCB response queue is empty by comparing IN and OUT ptrs. */
        while (qcb->in != work)
        {
            iocb7 = (CMIO7_IOCB *)work;
            if ((UINT32)next == iocb7->iocbhandle)
            {
                flag = 1;
                break;          /* Exit if ILT yet to be processed. */
            }
            work = (UINT32 *)(iocb7 + 1);       /* Increment to next IOCB. */
            if (work == qcb->end)       /* Check for wrap. */
            {
                work = qcb->begin;      /* Wrap to start of queue. */
            }
        }
        if (flag == 1)
        {
            continue;           /* Do not timeout if ILT yet to be processed. */
        }

#ifdef BACKEND
        /* Check if this is the first time the timer expired for this ILT. If this is
         * the first time, abort the IOCB, else reset the Qlogic chip.
         */
        if (next->isp_defs.isp_timeout != 0)
        {
            UINT32      regsave[6];     /* Save registers g0 through g5. */

            /* Clear timeout first time so the second time can be detected. */
            next->isp_defs.isp_timeout = 0;

            if (next->isp_defs.isp_iocb_type == CMIO7 &&    /* Only these are real I/O's to a drive. */
                next->isp_defs.isp_dev != NULL)             /* Double check -- above implies dev is set. */
            {
                /* Timer expired. Abort the task set for this device. This needs to
                 * be done in a new task because the timeout may have been caused by a
                 * dead qlogic port. Calling it directly here would suspend the thread
                 * indefinitely preventing us from resetting a dead port. */
                memcpy(&regsave, &g0, sizeof(regsave));     /* Save g0 through g5. */
                CT_fork_tmp = (ulong) "isp$AbortIocbTask";
                fprintf(stderr, "AbortIocbTask pid=%u, port=%d lun=%d lid=%d\n",
                        next->isp_defs.isp_dev->pdd->pid, next->isp_defs.isp_dev->port,
                        next->isp_defs.isp_dev->lun, next->isp_defs.isp_dev->lid);
                TaskCreate6(&isp$AbortIocbTask, PEXECPRI, (UINT32)next->isp_defs.isp_dev->lun,
                            (UINT32)next, (UINT32)next->isp_defs.isp_dev->port,
                            (UINT32)next->isp_defs.isp_dev->lid);
                memcpy(&g0, &regsave, sizeof(regsave));     /* Restore g0 thru g5. */
            }
        }
        else
#endif /* BACKEND */
            /* Timer expired; reset the chip. */
        {
            UINT32      regsave[15];    /* Save registers g0 thru g14. */

            memcpy(&regsave, &g0, sizeof(regsave));     /* Save g0 thru g14. */

            struct ISP_IOCB_TO_PKT m;

#ifdef BACKEND
            if (next->isp_defs.isp_dev != 0 && next->isp_defs.isp_dev->pdd != 0)
            {
                fprintf(stderr, "%ssecond timeout resetting qlogic chip pid=%u\n",
                        FEBEMESSAGE, next->isp_defs.isp_dev->pdd->pid);
            }
#endif /* BACKEND */
            memset(&m, 0, sizeof(m));
            m.header.event = LOG_IOCBTO;
            m.timeout.iocb = next->isp_defs.isp_iocb_type;      /* NOTE: limited to 8 bits. */
            m.timeout.ilt = (UINT32)next;       /* Save the ILT that timed out. */
#ifdef FRONTEND
            m.timeout.proc = 0; /* Indicate Front End qlogic timeout. */
            if (m.timeout.iocb == CMIO7)
            {
                m.timeout.lid = ((XLI *)(next->misc))->xlitarget;   /* target LID */
                m.timeout.lun = ((XLI *)(next->misc))->xlilun;      /* lun */
                m.timeout.wwn = servdb[port][m.timeout.lid];        /* Port world wide name */
                /* Save the CDB */
                memcpy(&m.timeout.cdb[0], &((XLI *)(next->misc))->xlicdb[0], sizeof(m.timeout.cdb));
            }
#else  /* FRONTEND */
            m.timeout.proc = 1; /* Indicate Back End qlogic timeout. */
            if (next->isp_defs.isp_dev != 0)
            {
                m.timeout.lid = next->isp_defs.isp_dev->lid;        /* Device ID (LID) */
                m.timeout.lun = next->isp_defs.isp_dev->lun;        /* Device lun */
                m.timeout.wwn = next->isp_defs.isp_dev->nodeName;   /* Device node WWN */
            }
            else
            {
                fprintf(stderr, "%ssecond timeout is not on a CMIO7 with dev set\n", FEBEMESSAGE);
            }
            /* Save the CDB */
            memcpy(&m.timeout.cdb[0], &((PRP *)((next - 2)->ilt_normal.w0))->cmd[0], sizeof(m.timeout.cdb));
#endif /* FRONTEND */
            m.timeout.port = port;      /* Store the port number. */
            MSC_LogMessageStack(&m, sizeof(m));
            ISP_DumpQL(port, ISP_CMD_TIME_OUT);

            memcpy(&g0, &regsave, sizeof(regsave));     /* Restore g0 thru g14. */
            break;              /* Can not continue, everything is hosed. */
        }
    }
}

/**
*******************************************************************************
**
**  @brief  To thread an ILT onto a doubly linked list per ISP chip instance.
**
**  Port contains the chip instance ordinal (0-MAXISP) and ilt contains the
**  ILT address at the FCAL nesting level. This routine places the ILT onto
**  the ilthead/ilttail lists for the ISP instance.
**
**  The timeout counter byte in <i_timeout> of the current nesting level of
**  the ILT is set to the default value.
**
**  @param  port    - Chip instance of QLogic ISP2x00 (0-MAXISP).
**  @param  ilt     - ILT address at FCAL nesting level.
**
**  @return none
**
*******************************************************************************
**/
void isp_thread_ilt(UINT8 port, ILT * ilt)
{
    ILT        *tail = (ILT *)&ilttail[port * 2];   /* Get tail "ilt" for this port. */
    ILT        *current_tail = tail->bthd;          /* Get current tail. */

#ifdef DEBUG_FLT_REC_ISP
    MSC_FlightRec(FR_ISP_THREAD | (port << 8) | ((timestamp & 0xffff) << 16),
                  (UINT32)ilt, (UINT32)tail, (UINT32)current_tail);
#endif /* DEBUG_FLT_REC_ISP */

    tail->bthd = ilt;           /* Set the tail to this ILT. */
    ilt->fthd = tail;           /* Set forward ptr to tail "ilt". */
    ilt->bthd = current_tail;   /* Set backward ptr to current tail ilt. */
    current_tail->fthd = ilt;   /* Last ilt points to new one. */
    ilt->isp_defs.isp_timeout += timestamp;     /* Convert to isp timestamp format time. */
}

/**
*******************************************************************************
**
**  @brief  To remove an ILT from a doubly linked list per ISP chip instance.
**
**  Port contains the chip instance ordinal (0-MAXISP) and ilt contains the
**  ILT address at the FCAL nesting level. This routine removes the ILT from
**  the ilthead/ilttail lists for the ISP instance.
**
**  If bad ILT, reset the qlogic chip.
**
**  @param  port    - Chip instance of QLogic ISP2x00 (0-MAXISP).
**  @param  ilt     - ILT address at FCAL nesting level.
**
**  @return ILT pointer passed in, NULL if ILT is invalid.
**
*******************************************************************************
**/
ILT        *isp_unthread_ilt(UINT8 port, ILT * ilt)
{
    ILT        *current_bthd = ilt->bthd;
    ILT        *current_fthd = ilt->fthd;

    if (current_fthd == NULL || current_bthd == NULL ||
        current_bthd->fthd != ilt || current_fthd->bthd != ilt)
    {
        UINT32      regsave[15];        /* Save registers g0 thru g14. */

        memcpy(&regsave, &g0, sizeof(regsave)); /* Save g0 thru g14. */
#if ISP_DEBUG
        ISP_DumpQL(port, ISP_ILT_THREAD_ERROR);
#else  /* ISP_DEBUG */
        ISP_ResetChip(port, ISP_ILT_THREAD_ERROR);
#endif /* ISP_DEBUG */
        memcpy(&g0, &regsave, sizeof(regsave)); /* Restore g0 thru g14. */
        return (NULL);          /* Invalidate ILT pointer (return value) */
    }

#ifdef DEBUG_FLT_REC_ISP
    MSC_FlightRec(FR_ISP_UNTH | (port << 8) | ((timestamp & 0xffff) << 16),
                  (UINT32)ilt, (UINT32)ilt->fthd, (UINT32)ilt->bthd);
#endif /* DEBUG_FLT_REC_ISP */

    /* Set forward pointer for previous ILT to forward pointer of this one. */
    current_bthd->fthd = current_fthd;
    /* Set backward pointer for next ILT to point to previous ILT. */
    current_fthd->bthd = current_bthd;
    return (ilt);               /* Return input ILT pointer (i.e. good) */
}

/**
*******************************************************************************
**
**  @brief  To remove an ILT from a doubly linked list per ISP chip instance.
**
**  Port contains the chip instance ordinal (0-MAXISP) and ilt contains the
**  ILT address at the FCAL nesting level. This routine removes the ILT from
**  the ilthead/ilttail lists for the ISP instance.
**
**  If bad ILT, exit -- DO NOT reset the qlogic chip (to prohibit loops).
**
**  @param  ilt     - ILT address at FCAL nesting level.
**
**  @return ILT pointer passed in, NULL if ILT is invalid.
**
*******************************************************************************
**/
ILT        *isp_unthread_ilt_1(ILT * ilt)
{
    ILT        *current_bthd = ilt->bthd;
    ILT        *current_fthd = ilt->fthd;

    if (current_fthd == NULL || current_bthd == NULL ||
        current_bthd->fthd != ilt || current_fthd->bthd != ilt)
    {
        return (NULL);          /* Invalidate ILT pointer (return value) */
    }

    /* Set forward pointer for previous ILT to forward pointer of this one. */
    current_bthd->fthd = current_fthd;
    /* Set backward pointer for next ILT to point to previous ILT. */
    current_fthd->bthd = current_bthd;
    return (ilt);               /* Return input ILT pointer (i.e. good) */
}

#ifdef BACKEND
/****************************************************************************/
void print_scsi_cmd(DEV *dev, PRP *prp, const char *mes)
{
    UINT8  *cdb = &(prp->cmd[0]);

    if (dev != NULL && dev->pdd != NULL) {
        int printpid = dev->pdd->pid;
        int ses = dev->pdd->ses;
        int slot = dev->pdd->slot;
        switch (cdb[0])
        {
            case SCC_TESTUNR:    fprintf(stderr, "%s%s SCSI command Test Unit Ready PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_REWIND:     fprintf(stderr, "%s%s SCSI command RECALIBRATE or REWIND PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_REQSENSE:   fprintf(stderr, "%s%s SCSI command REQUEST SENSE PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_FORMAT:     fprintf(stderr, "%s%s SCSI command FORMAT UNIT PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READBLKLMT: fprintf(stderr, "%s%s SCSI command READ BLOCK LIMITS PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_REASSIGNBK: fprintf(stderr, "%s%s SCSI command REASSIGN BLOCKS PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READ_6:     fprintf(stderr, "%s%s SCSI command READ (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRITE_6:    fprintf(stderr, "%s%s SCSI command WRITE (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SEEK_6:     fprintf(stderr, "%s%s SCSI command SEEK (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READ_RVS_6: fprintf(stderr, "%s%s SCSI command READ REVERSE (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRITEFLM_6: fprintf(stderr, "%s%s SCSI command WRITE FILEMARKS (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SPACE_6:    fprintf(stderr, "%s%s SCSI command SPACE (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_INQUIRY:    fprintf(stderr, "%s%s SCSI command INQUIRY PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_VERIFY_6:   fprintf(stderr, "%s%s SCSI command VERIFY (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_RCVRBUFDAT: fprintf(stderr, "%s%s SCSI command RECOVER BUFFERED DATA PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_MODESLC:    fprintf(stderr, "%s%s SCSI command MODE SELECT PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_RESERVE_6:  fprintf(stderr, "%s%s SCSI command RESERVE (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_RELEASE_6:  fprintf(stderr, "%s%s SCSI command RELEASE (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_COPY:       fprintf(stderr, "%s%s SCSI command COPY PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_ERASE_6:    fprintf(stderr, "%s%s SCSI command ERASE (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_MODESNS:    fprintf(stderr, "%s%s SCSI command Mode Sense (6) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SSU:        fprintf(stderr, "%s%s SCSI command Start/Stop Unit or Load/Unload PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_RCVDIAG:    fprintf(stderr, "%s%s SCSI command RECEIVE DIAGNOSTIC RESULTS PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SNDDIAG:    fprintf(stderr, "%s%s SCSI command SEND DIAGNOSTIC PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_MEDIUM_PAR: fprintf(stderr, "%s%s SCSI command PREVENT/ALLOW MEDIUM REMOVAL PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READFORMAT: fprintf(stderr, "%s%s SCSI command READ FORMAT CAPACITIES (MMC) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SET_WINDOW: fprintf(stderr, "%s%s SCSI command SET WINDOW PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READCAP:    fprintf(stderr, "%s%s SCSI command Read Capacity (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READEXT:    fprintf(stderr, "%s%s SCSI command Read (10) lba=0x%x lth=0x%x PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes,
                                                bswap_32(((SCSI_READ_EXTENDED*)&cdb[0])->lba),
                                                bswap_16(((SCSI_READ_EXTENDED*)&cdb[0])->numBlocks), printpid, ses, slot); break;
            case SCC_READGEN:    fprintf(stderr, "%s%s SCSI command READ GENERATION PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRITEXT:    fprintf(stderr, "%s%s SCSI command Write (10) lba=0x%x lth=0x%x PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes,
                                                bswap_32(((SCSI_WRITE_EXTENDED*)&cdb[0])->lba),
                                                bswap_16(((SCSI_WRITE_EXTENDED*)&cdb[0])->numBlocks), printpid, ses, slot); break;
            case SCC_SEEK_10:    fprintf(stderr, "%s%s SCSI command SEEK (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_ERASE_10:   fprintf(stderr, "%s%s SCSI command ERASE (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READUPDBLK: fprintf(stderr, "%s%s SCSI command READ UPDATED BLOCK PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRTVRFY_10: fprintf(stderr, "%s%s SCSI command WRITE AND VERIFY (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_VERIMED:    fprintf(stderr, "%s%s SCSI command VERIFY MEDIA (10) lba= 0x%x lth=0x%x PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes,
                                                bswap_32(((SCSI_VERIFY_10*)&cdb[0])->lba),
                                                bswap_16(((SCSI_VERIFY_10*)&cdb[0])->numBlocks), printpid, ses, slot); break;
            case SCC_SCHHIGH_10: fprintf(stderr, "%s%s SCSI command SEARCH DATA HIGH (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SCHEQ_10:   fprintf(stderr, "%s%s SCSI command SEARCH DATA EQUAL (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SCHLOW_10:  fprintf(stderr, "%s%s SCSI command SEARCH DATA LOW (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SETLMTS_10: fprintf(stderr, "%s%s SCSI command SET LIMITS (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_PRFETCH_10: fprintf(stderr, "%s%s SCSI command PRE-FETCH (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SYNCCAC_10: fprintf(stderr, "%s%s SCSI command SYNCHRONIZE CACHE (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_LKCACHE_10: fprintf(stderr, "%s%s SCSI command LOCK/UNLOCK CACHE (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_RDDFTDT_10: fprintf(stderr, "%s%s SCSI command READ DEFECT DATA (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_MEDIUMSCAN: fprintf(stderr, "%s%s SCSI command MEDIUM SCAN PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_COMPARE:    fprintf(stderr, "%s%s SCSI command COMPARE PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_COPYVERIFY: fprintf(stderr, "%s%s SCSI command COPY AND VERIFY PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRITEBUF:   fprintf(stderr, "%s%s SCSI command WRITE BUFFER PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READBUF:    fprintf(stderr, "%s%s SCSI command READ BUFFER PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_UPDATEBLK:  fprintf(stderr, "%s%s SCSI command UPDATE BLOCK PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READLONG:   fprintf(stderr, "%s%s SCSI command READ LONG PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRITELONG:  fprintf(stderr, "%s%s SCSI command WRITE LONG PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_CHANGEDEF:  fprintf(stderr, "%s%s SCSI command CHANGE DEFINITION PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRTSAME_10: fprintf(stderr, "%s%s SCSI command WRITE SAME (10) lba=0x%x lth=0x%x PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes,
                                                bswap_32(((SCSI_WRITE_SAME*)&cdb[0])->lba),
                                                bswap_16(((SCSI_WRITE_SAME*)&cdb[0])->numBlocks), printpid, ses, slot); break;
            case SCC_GETDENSITY: fprintf(stderr, "%s%s SCSI command REPORT DENSITY SUPPORT PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_PLAYAUDIO:  fprintf(stderr, "%s%s SCSI command PLAY AUDIO (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_GETCONFIG:  fprintf(stderr, "%s%s SCSI command GET CONFIGURATION PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_PLAYAUDMSF: fprintf(stderr, "%s%s SCSI command PLAY AUDIO MSF PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_EVENTNOTFY: fprintf(stderr, "%s%s SCSI command GET EVENT STATUS NOTIFICATION PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_PAUSERSUME: fprintf(stderr, "%s%s SCSI command PAUSE / RESUME PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_LOGSELECT:  fprintf(stderr, "%s%s SCSI command LOG SELECT PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_LOGSENSE:   fprintf(stderr, "%s%s SCSI command LOG SENSE PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_XDWRITE_10: fprintf(stderr, "%s%s SCSI command XDWRITE (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_XPWRITE_10: fprintf(stderr, "%s%s SCSI command XPWRITE (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_XDREAD_10:  fprintf(stderr, "%s%s SCSI command XDREAD (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_XDWRTRD_10: fprintf(stderr, "%s%s SCSI command XDWRITEREAD (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SNDOPCINFO: fprintf(stderr, "%s%s SCSI command SEND OPC INFORMATION PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_MDSELCT_10: fprintf(stderr, "%s%s SCSI command MODE SELECT (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_RESERVE_10: fprintf(stderr, "%s%s SCSI command RESERVE (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_RELEASE_10: fprintf(stderr, "%s%s SCSI command RELEASE (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_REPAIRTRCK: fprintf(stderr, "%s%s SCSI command REPAIR TRACK PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_MODESNS_10: fprintf(stderr, "%s%s SCSI command MODE SENSE (10) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_CLOSETRACK: fprintf(stderr, "%s%s SCSI command CLOSE TRACK / SESSION PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_RDBUFCAP:   fprintf(stderr, "%s%s SCSI command READ BUFFER CAPACITY PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SENDCUESHT: fprintf(stderr, "%s%s SCSI command SEND CUE SHEET PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_PRRSERVIN:  fprintf(stderr, "%s%s SCSI command PERSISTENT RESERVE IN PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_PRSERVOUT:  fprintf(stderr, "%s%s SCSI command PERSISTENT RESERVE OUT PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_EXTCDB:     fprintf(stderr, "%s%s SCSI command EXTENDED CDB PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_VARLTHCDB:  fprintf(stderr, "%s%s SCSI command VARIABLE LENGTH CDB PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_XDWRT_16:   fprintf(stderr, "%s%s SCSI command XDWRITE EXTENDED (16) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_REBUILD_16: fprintf(stderr, "%s%s SCSI command REBUILD (16) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_REGENER_16: fprintf(stderr, "%s%s SCSI command REGENERATE (16) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_EXTENDCOPY: fprintf(stderr, "%s%s SCSI command EXTENDED COPY PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_RECCOPYRLT: fprintf(stderr, "%s%s SCSI command RECEIVE COPY RESULTS PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_ATAPTHR_16: fprintf(stderr, "%s%s SCSI command ATA COMMAND PASS THROUGH (16) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_A_CTL_IN:   fprintf(stderr, "%s%s SCSI command ACCESS CONTROL IN PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_A_CTL_OUT:  fprintf(stderr, "%s%s SCSI command ACCESS CONTROL OUT PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READ_16:    fprintf(stderr, "%s%s SCSI command READ (16) lba=0x%llx lth=0x%x PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes,
                                                bswap_64(((SCSI_WRITE_16*)&cdb[0])->lba),
                                                bswap_32(((SCSI_WRITE_16*)&cdb[0])->numBlocks), printpid, ses, slot); break;
            case SCC_WRITE_16:    fprintf(stderr, "%s%s SCSI command WRITE (16) lba=0x%llx lth=0x%x PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes,
                                                bswap_64(((SCSI_WRITE_16*)&cdb[0])->lba),
                                                bswap_32(((SCSI_WRITE_16*)&cdb[0])->numBlocks), printpid, ses, slot); break;
            case SCC_ORWRITE:    fprintf(stderr, "%s%s SCSI command ORWRITE PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READ_ATTR:  fprintf(stderr, "%s%s SCSI command READ ATTRIBUTE PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRITE_ATTR: fprintf(stderr, "%s%s SCSI command WRITE ATTRIBUTE PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRTVRFY_16: fprintf(stderr, "%s%s SCSI command WRITE AND VERIFY (16) lba=0x%llx lth=0x%x PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes,
                                                bswap_64(((SCSI_WRITE_16*)&cdb[0])->lba),
                                                bswap_32(((SCSI_WRITE_16*)&cdb[0])->numBlocks), printpid, ses, slot); break;
            case SCC_VERIFY_16:  fprintf(stderr, "%s%s SCSI command VERIFY (16) lba=0x%llx lth=0x%x PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes,
                                                bswap_64(((SCSI_WRITE_16*)&cdb[0])->lba),
                                                bswap_32(((SCSI_WRITE_16*)&cdb[0])->numBlocks), printpid, ses, slot); break;
            case SCC_PRFETCH_16: fprintf(stderr, "%s%s SCSI command PRE-FETCH (16) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SYNCCAC_16: fprintf(stderr, "%s%s SCSI command SYNCHRONIZE CACHE (16) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SPACE_16:   fprintf(stderr, "%s%s SCSI command SPACE (16) or LOCK UNLOCK CACHE (16) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRTSAME_16: fprintf(stderr, "%s%s SCSI command WRTSAME (16) lba=0x%llx lth=0x%x PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes,
                                                bswap_64(((SCSI_WRITESAME_16*)&cdb[0])->lba),
                                                bswap_32(((SCSI_WRITESAME_16*)&cdb[0])->numBlocks), printpid, ses, slot); break;
            case SCC_READCAP_16: fprintf(stderr, "%s%s SCSI command READ CAPACITY (16) [Service action In (16)] PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SERVOUT_16: fprintf(stderr, "%s%s SCSI command SERVICE ACTION OUT (16) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_REPLUNS:    fprintf(stderr, "%s%s SCSI command REPORT LUNS PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_ATAPTHR_12: fprintf(stderr, "%s%s SCSI command ATA COMMAND PASS THROUGH (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SECPROTIN:  fprintf(stderr, "%s%s SCSI command SECURITY PROTOCOL IN PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_REPORTOPCD: fprintf(stderr, "%s%s SCSI command REPORT SUPPORTED OPCODES PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_MAINTOUT:   fprintf(stderr, "%s%s SCSI command MAINTENANCE (OUT) (REPORT_KEY) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_MOVEMEDIUM: fprintf(stderr, "%s%s SCSI command MOVE MEDIUM PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_EXCHMEDIUM: fprintf(stderr, "%s%s SCSI command EXCHANGE MEDIUM PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_MVMEDATTCH: fprintf(stderr, "%s%s SCSI command MOVE MEDIUM ATTACHED PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READ_12:    fprintf(stderr, "%s%s SCSI command READ (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SERVICEOUT: fprintf(stderr, "%s%s SCSI command SERVICE ACTION OUT (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRITE_12:   fprintf(stderr, "%s%s SCSI command WRITE (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SERVICEIN:  fprintf(stderr, "%s%s SCSI command SERVICE ACTION IN (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_ERASE_12:   fprintf(stderr, "%s%s SCSI command ERASE (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READDVDSTR: fprintf(stderr, "%s%s SCSI command READ DVD STRUCTURE PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_WRTVRFY_12: fprintf(stderr, "%s%s SCSI command WRITE AND VERIFY (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_VERIFY_12:  fprintf(stderr, "%s%s SCSI command VERIFY (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SCHHIGH_12: fprintf(stderr, "%s%s SCSI command SEARCH DATA HIGH (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SCHEQ_12:   fprintf(stderr, "%s%s SCSI command SEARCH DATA EQUAL (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SCHLOW_12:  fprintf(stderr, "%s%s SCSI command SEARCH DATA LOW (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SETLMTS_12: fprintf(stderr, "%s%s SCSI command SET LIMITS (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READSTATCH: fprintf(stderr, "%s%s SCSI command READ ELEMENT STATUS ATTACHED PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SECPROTOUT: fprintf(stderr, "%s%s SCSI command SECURITY PROTOCOL OUT PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SENDVOLTAG: fprintf(stderr, "%s%s SCSI command SEND VOLUME TAG PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_RDDFTDT_12: fprintf(stderr, "%s%s SCSI command READ DEFECT DATA (12) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READSTATUS: fprintf(stderr, "%s%s SCSI command READ ELEMENT STATUS PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_READCDMSF:  fprintf(stderr, "%s%s SCSI command READ CD MSF PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_REDGRPIN:   fprintf(stderr, "%s%s SCSI command REDUNDANCY GROUP (IN) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_REDGRPOUT:  fprintf(stderr, "%s%s SCSI command REDUNDANCY GROUP (OUT) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SPARE_IN:   fprintf(stderr, "%s%s SCSI command SPARE (IN) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_SPARE_OUT:  fprintf(stderr, "%s%s SCSI command SPARE (OUT) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_VOLUME_IN:  fprintf(stderr, "%s%s SCSI command VOLUME SET (IN) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            case SCC_VOLUME_OUT: fprintf(stderr, "%s%s SCSI command VOLUME SET (OUT) PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot); break;
            default: fprintf(stderr, "%s%s SCSI command not decoded PID=%d SES=%d SLOT=%d\n", FEBEMESSAGE, mes, printpid, ses, slot);
        }
    }
}   /* End of print_scsi_cmd */
#endif  /* BACKEND */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
