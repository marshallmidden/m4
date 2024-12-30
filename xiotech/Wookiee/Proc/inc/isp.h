/* $Id: isp.h 158811 2011-12-20 20:42:56Z m4 $ */
/**
******************************************************************************
**
**  @file       isp.h
**
**  @brief      ISP definiation
**
**  To provide definitions for the QLogic ISP 2x00 Fibre Channel chip.
**
**  Copyright (c) 2002-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef ISP_H
#define ISP_H

#include "fabric.h"
#include "options.h"
#include "pcb.h"
#include "qcb.h"
#include "system.h"
#include "target.h"
#include "XIO_Types.h"
#include "globalOptions.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define NO_LID      0xFFFF          /* Not logged in, no Loop ID            */
#define NO_CONNECT  0xFFFF          /* Not connected, no Loop ID            */
#define NO_PORTID   0x80000000
#define NO_TID      0xFFFF
#ifdef MULTI_ID
#define HANDLE_DB_SIZE 128
#else
#define HANDLE_DB_SIZE 1
#endif

/*
** ISP command codes (operational firmware)
*/
#define ISP_NOOP    0x00            /* No Operation                         */
#define ISP_LRAM    0x01            /* Load RAM                             */
#define ISP_EFRM    0x02            /* Execute Firmware                     */
#define ISP_DRAM    0x03            /* Dump RAM                             */
#define ISP_MBXT    0x06            /* Mailbox Register Test                */
#define ISP_VCSM    0x07            /* Verify Checksum                      */
#define ISP_GFRM    0x08            /* Get Firmware Version                 */
#define ISP_DRRM    0x0A            /* Dump RISC RAM                        */
#define ISP_LRRME   0x0B            /* Load RISC RAM Extended               */
#define ISP_DRRME   0x0C            /* Dump RISC RAM Extended               */
#define ISP_CFR     0x0E            /* Checksum Firmware                    */
#define ISP_CIRQ    0x10            /* Initialize Request Queue             */
#define ISP_CISQ    0x11            /* Initialize Response Queue            */
#define ISP_EIOC    0x12            /* Execute Command IOCB                 */
#define ISP_SWST    0x13            /* Set Wake-Up Threshold                */
#define ISP_SFRM    0x14            /* Stop Firmware                        */
#define ISP_AIOC    0x15            /* Abort Command IOCB                   */
#define ISP_AABD    0x16            /* Abort Device                         */
#define ISP_AABT    0x17            /* Abort Target                         */
#define ISP_IRST    0x18            /* Reset Loop                           */
#define ISP_STPQ    0x19            /* Stop Queue                           */
#define ISP_STAQ    0x1A            /* Start Queue                          */
#define ISP_SSTQ    0x1B            /* Single Step Queue                    */
#define ISP_ABTQ    0x1C            /* Abort Queue                          */
#define ISP_GTQS    0x1D            /* Get Queue Status                     */
#define ISP_GTFS    0x1F            /* Get Firmware Status                  */
#define ISP_GTID    0x20            /* Get Loop ID                          */
#define ISP_GTRT    0x22            /* Get Retry Count                      */
#define ISP_GFO     0x28            /* Get Firmware Options                 */
#define ISP_GTPQ    0x29            /* Get Port Queue Parameters            */
#define ISP_FSE     0x2A            /* Force System Error                   */
#define ISP_STRT    0x32            /* Set Retry Count                      */
#define ISP_SFO     0x38            /* Set Firmware Options                 */
#define ISP_LPB     0x40            /* Loop Port Bypass                     */
#define ISP_LPE     0x41            /* Loop Port Enable                     */
#define ISP_GRC     0x42            /* Get Resource Counts                  */
#define ISP_EGTPD   0x47            /* Enhanced Get port database           */
#define ISP_IFRT    0x48            /* Initialize Firmware (multi target)   */
#define ISP_GVPD    0x49            /* Get VP database                      */
#define ISP_GVPE    0x4A            /* Get VP database entry                */
#define ISP_EA64    0x54            /* Execute Command IOCB A64             */
#define ISP_GLOUT   0x56            /* Port logout                          */
#define ISP_DHB     0x5B            /* Driver Heartbeat                     */
#define ISP_FHB     0x5C            /* Firmware Heartbeat                   */
#define ISP_DR      0x5D            /* Data Rate                            */
#define ISP_IFRM    0x60            /* Initialize Firmware                  */
#define ISP_LIP     0x62            /* Initiate LIP                         */
#define ISP_GTPM    0x63            /* Get FC-AL Position Map               */
#define ISP_GTPD    0x64            /* Get port database                    */
#define ISP_CACA    0x65            /* Clear ACA                            */
#define ISP_TRST    0x66            /* Target Reset                         */
#define ISP_CTS     0x67            /* Clear Task Set                       */
#define ISP_ATS     0x68            /* Abort Task Set                       */
#define ISP_GFWS    0x69            /* Get Firmware State                   */
#define ISP_GPNM    0x6A            /* Get Port Name                        */
#define ISP_GLKS    0x6B            /* Get Link Status                      */
#define ISP_LIPR    0x6C            /* Lip Reset                            */
#define ISP_GLSP    0x6D            /* Get private link statistics          */
#define ISP_GSNS    0x6E            /* Send SNS                             */
#define ISP_GLFP    0x6F            /* Login Fabric Port                    */
#define ISP_GDFP    0x71            /* Logout Fabric Port                   */
#define ISP_LIPL    0x72            /* LIP followed by Login                */
#define ISP_LINIT   0x72            /* LIP followed by Login                */
#define ISP_LGIN    0x74            /* Login Loop Port                      */
#define ISP_GPNL    0x75            /* Get Port/Node Name List              */
#define ISP_GIDL    0x7C            /* Get ID List                          */
#define ISP_LFA     0x7D            /* Send LFA                             */
#define ISP_LUR     0x7E            /* Logical Unit Reset                   */

/*
** Mailbox Command Complete Status Codes
*/
#define ISP_CMDC    0x4000          /* Command Successful                   */
#define ISP_CMDI    0x4001          /* Invalid Command                      */
#define ISP_CMDE    0x4005          /* Command Error                        */
#define ISP_CPE     0x4006          /* Command Parameter Error              */
#define ISP_PIU     0x4007          /* Port ID Used                         */
#define ISP_LIU     0x4008          /* Loop ID Used                         */
#define ISP_AIU     0x4009          /* Alls ID in use                       */
#define ISP_NLI     0x400A          /* FFFFFC (SNS) not logged in           */
#define ISP_INVDEVPORT     0x4090          /* Invalid device port           */
#define ISP_PID     0x4091          /* Not issued, PID is invalid           */
#define ISP_ILT     0x4092          /* ILT is incorrect                     */
#define ISP_QRP     0x4093          /* QRP is incorrect                     */
#define ISP_NLP     0x4094          /* Not Issued, not loop mode            */
#define ISP_NOSW    0x4095          /* Not Issued, fabric is not supported  */
#define ISP_MBTO    0x4096          /* Mailbox Timeout                      */
#define ISP_RSTD    0x4097          /* Not Issued, port is being reset/dump */
#define ISP_LIPIP   0x4098          /* Not Issued, Lip is progress          */
#define ISP_CRST    0x4099          /* Not Issued, port is being reset      */

/*
** QLogic iocb Status codes for status type 0
*/
#define QLOGIC_STATUS_GOOD              0x00
#define QLOGIC_STATUS_DMAERROR          0x02
#define QLOGIC_STATUS_TRANSPORT_ERR     0x03
#define QLOGIC_STATUS_RESET             0x04
#define QLOGIC_STATUS_TASKABORT         0x05
#define QLOGIC_STATUS_TIMEOUT           0x06
#define QLOGIC_STATUS_OVERRUN           0x07
#define QLOGIC_STATUS_DATA_ASSEMB_ERR   0x11
#define QLOGIC_STATUS_ABORTED_BY_TARG   0x13
#define QLOGIC_STATUS_UNDERRUN          0x15
#define QLOGIC_STATUS_QUEUEFULL         0x1c
#define QLOGIC_STATUS_PORTUNAVAIL       0x28
#define QLOGIC_STATUS_PORTLOGGEDOUT     0x29
#define QLOGIC_STATUS_PORTCHANGED       0x2A
#define QLOGIC_STATUS_FW_RESOURCE_OUT   0x2C
#define QLOGIC_STATUS_TMF_OVERRUN       0x30

/*
** ISP Control/Status Register
*/
#define ISP_CMS     5               /* bits 5 and 4 are Module Select bits. */
#define ISP_CM1     4               /*   00 = RISC registers
                                    **   01 = FB registers
                                    **   02 = FPM registers (RISC I/O 100-13F)
                                    **   03 = FPM registers (RISC I/O 140-17F)
                                    ** FPM registers only available when RISC
                                    ** is in pause mode.
                                    */
#define ISP_C64     2               /* 1 = 64 bit PCI slot (read only)      */
#define ISP_CFE     1               /* 1 = FLASH BIOS program mode          */
#define ISP_CSR     0               /* ISP soft reset.  Cleared by hardware
                                    **  when reset complete.                */

/*
** Host Command and Control (HCCR) bit definitions
*/
/* Bits 15-12 command codes                                                 */
#define ISP_HCSETTM 0xF             /* Set chip test mode (do not use)      */
#define ISP_HCSETPE 0xE             /* Force Parity Error                   */
#define ISP_HCSPWPE 0xA             /* Enable Write Parity                  */
#define ISP_HCCLRRI 0x7             /* Clear RISC Interrupt                 */
#define ISP_HCCLRHI 0x6             /* Clear Host Interrupt                 */
#define ISP_HCSETHI 0x5             /* Set Host Interrupt                   */
#define ISP_HCSINRI 0x4             /* Single Step RISC                     */
#define ISP_HCRELRI 0x3             /* Release RISC                         */
#define ISP_HCPAURI 0x2             /* Pause RISC                           */
#define ISP_HCRESRI 0x1             /* Reset RISC                           */
#define ISP_HCNOOPR 0x0             /* No operation                         */
/* Bits 11-0 in HCCR register                                               */
#define ISP_HCRPARE 11              /* RISC Data Parity Error               */
#define ISP_HCPARB3 10              /* Parity Enable - Bank 2 and 3         */
#define ISP_HCPARB2 9               /* Parity Enable - Bank 1               */
#define ISP_HCPARB1 8               /* Parity Enable - Bank 0               */
#define ISP_HCHOSTI 7               /* Host Interrupt                       */
#define ISP_HCRESET 6               /* Reset                                */
#define ISP_HCPAUSE 5               /* Pause Mode                           */
#define ISP_HCENTBE 4               /* External Breakpoint Enable           */
#define ISP_HCEBKP1 3               /* Breakpoint 1 Enable                  */
#define ISP_HCEBKP0 2               /* Breakpoint 0 Enable                  */
#define ISP_HCINTOB 1               /* Enable Interrupt on Breakpoint       */
#define ISP_HCRESRV 0               /* Reserved                             */

/*
** ISP Asynchronous Event Codes
**/
#define ISP_ASBASE  0x8000          /* Base of Async Codes                  */
#define ISP_ASPSYE  0x8002          /* Unrecoverable Error                  */
#define ISP_ASPRTE  0x8003          /* Request Transfer Error               */
#define ISP_ASPRSE  0x8004          /* Response Transfer Error              */
#define ISP_ASPRQW  0x8005          /* Request Queue Wake-Up                */
#define ISP_ASPLIP  0x8010          /* LIP occurred                         */
#define ISP_ASPFCU  0x8011          /* Loop Up                              */
#define ISP_ASPFCD  0x8012          /* Loop Down                            */
#define ISP_ASPLPR  0x8013          /* LIP Reset                            */
#define ISP_ASPDBE  0x8014          /* Port Database Changed                */
#define ISP_ASPCNE  0x8015          /* Fabric Change Notification           */
#define ISP_LIPRCV  0x8016          /* LIP received                         */
#define ISP_ASPLIE  0x8017          /* Loop Initialization Errors           */
#define ISP_ASPPTP  0x8030          /* Connected in Point-to-Point Mode     */
#define ISP_ASPNRS  0x8040          /* RIO no response                      */
#define ISP_ASPFRD  0x8048          /* Frame Dropped                        */
#define ISP_ASPQFL  0x8049          /* Queue Full                           */
#define ISP_ASAILI  0x80FF          /* Illegal Interrupt                    */

/*
** ISP online event codes
*/
#define ISPOLPU 0x11                /* Online due to Loop UP event          */
#define ISPODBC 0x14                /* Online due to DB change event        */
#define ISPOSCN 0x15                /* Online due to State Change Notification*/
#define ISPOFTE 0xf0                /* Online due to Foreign target enable  */
#define ISPOLOP 0xf1                /* Online due to Loss of Port           */

/*
** Connection type return by get loop id
*/
#define LOOP                0       /* Loop (no fabric)                     */
#define FL_PORT             1       /* FL_Port in loop                      */
#define N_PORT              2       /* N_Port to N_Port (point-to-point)    */
#define F_PORT              3       /* F_Port (point-to-point)              */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
/* Firmware type */

#define iSCSI               0x4953435369LL  /* "iSCSI" firmware             */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

struct ILT;

typedef struct ISP_2400
{
    UINT32 volatile fBIOSAddr;      /* Flash BIOS address                   */
    UINT32 volatile fBIOSData;      /* Flash BIOS data                      */
    UINT32 volatile cntl;           /* ISP Control/Status                   */
    UINT32 volatile intc;           /* ISP to PCI Interrupt Control         */
    UINT32 volatile ints;           /* ISP to PCI Interrupt Status          */
    UINT32 volatile rsvd1[2];       /* Reserved 1                           */
    UINT32 volatile reqQIP;         /* Request Queue In Ptr                 */
    UINT32 volatile reqQOP;         /* Request Queue OUT Ptr                */
    UINT32 volatile rspQIP;         /* Response Queue In Ptr                */
    UINT32 volatile rspQOP;         /* Response Queue OUT Ptr               */
    UINT32 volatile preqQIP;        /* priority Request Queue In Ptr        */
    UINT32 volatile preqQOP;        /* priority Request Queue OUT Ptr       */
    UINT32 rsvd2[2];                /* Reserved2                            */
    UINT32 volatile atioQIP;        /* ATIO Queue In Ptr                    */
    UINT32 volatile atioQOP;        /* ATIO Queue OUT Ptr                   */
    UINT32 volatile r2HStat;        /* RISC to Host status register         */
    UINT32 volatile hccr;           /* Host command&control register        */
    UINT32 volatile gpIOD;          /* General purpose I/O data             */
    UINT32 volatile gpIOE;          /* General purpose I/O enable           */
    UINT32 volatile IObase;         /* IO Base register                     */
    UINT32 rsvd3[10];               /* Reserved3                            */
    UINT16 volatile mBox[32];       /* Mailbox Registers                    */
} ISP_2400;

/*
** ISP Control/Status Register bit definitions
*/
#define ISP2400CSR_FLASH_NVRAM_ERR      0x40000
#define ISP2400CSR_DMA_ACTIVE           0x20000
#define ISP2400CSR_DMA_CONTROL          0x10000
#define ISP2400CSR_FUNCTION_NUM         0x80000
/*
** Busmode bits 11-8
*/
#define ISP2400CSR_PCI_BUS_MASK         0xF00

#define ISP2400CSR_PCI_BUSM1_33MHZ    0x000
#define ISP2400CSR_PCI_BUSM1_66MHZ    0x100
#define ISP2400CSR_PCI_BUSM1_100MHZ   0x200
#define ISP2400CSR_PCI_BUSM1_133MHZ   0x300
#define ISP2400CSR_PCI_BUSM2_66MHZ    0x500
#define ISP2400CSR_PCI_BUSM2_100MHZ   0x600
#define ISP2400CSR_PCI_BUS2_133MHZ    0x700
#define ISP2400CSR_PCI_66MHZ          0x800
/*
** write burst bits 5-4
*/
#define ISP2400CSR_WRITE_BURST_MASK         0x30

#define ISP2400CSR_WRITE_512BURST_COUNT   0x00
#define ISP2400CSR_WRITE_1024BURST_COUNT  0x10
#define ISP2400CSR_WRITE_2048BURST_COUNT  0x20
#define ISP2400CSR_WRITE_4096BURST_COUNT  0x30

#define ISP2400CSR_64BIT_PCI                   0x4
#define ISP2400CSR_NVRAM_ENABLE                0x2
#define ISP2400CSR_SOFT_RESET                  0x1

/*
** ISP-PCI  interrupt control register
*/
#define ISP2400ICR_ENABLE_RISC_PCI   0x00000008 /* Enable RISC interrupts on PCI */

/*
** ISP-PCI  interrupt status register
*/
#define ISP2400ISR_RISC_PCI_REQ      0x00000008 /* RISC to PCI interrupt request */

/*
** RISC to HOST status register
*/
#define ISP2400RHS_RISC_REQ      0x8000 /* RISC to Host interrupt */
#define ISP2400RHS_RISC_PAUSED   0x100  /* RISC to Host Paused */
#define ISP2400RHS_RISC_STATUS_MASK 0xFF    /* RISC interrupt status mask */
#define ISP2400RHS_STATUS_ROM_MBX_COMP  0x1 /* ROM mailbox complete */
#define ISP2400RHS_STATUS_ROM_MBX_ERR   0x2 /* ROM mailbox error */
#define ISP2400RHS_STATUS_MBX_COMP  0x10    /* Mailbox command complete */
#define ISP2400RHS_STATUS_MBX_ERR   0x11    /* Mailbox command error */
#define ISP2400RHS_STATUS_EVENT     0x12    /* Async event */
#define ISP2400RHS_STATUS_RESP      0x13    /* Response queue update */
#define ISP2400RHS_STATUS_ATIO      0x1C    /* ATIO queue update */
#define ISP2400RHS_STATUS_ATIO_RESP 0x1D    /* ATIO & response queue update */

/*
** HCCR (Host Command and Control Register)
** Bits 31-28 Command Field
** Bits 3-0   Command Parameter
*/

/*
** Commands
*/
#define ISP2400HCCR_NOOP                  0x00000000
#define ISP2400HCCR_SET_RISC_RESET        0x10000000 /* Set Risc Reset */
#define ISP2400HCCR_CLEAR_RISC_RESET      0x20000000 /* Clear  Risc Reset */
#define ISP2400HCCR_SET_RISC_PAUSE        0x30000000 /* Set RISC Pause */
#define ISP2400HCCR_REL_RISC_PAUSE        0x40000000 /* Release RISC Pause */
#define ISP2400HCCR_HOST_TO_RISC_INTR     0x50000000 /* Host to RISC interrupt */
#define ISP2400HCCR_CLR_HOST_TO_RISC_INTR 0x60000000 /* Clear Host to RISC interrupt */
#define ISP2400HCCR_CLEAR_RISC_INTR       0xA0000000 /* Clear the RISC to PCI interrupt*/

/*
** HCCRS (Host Command and Control Status) information
*/
#define ISP2400HCCRS_HOST_TO_RISC_INTR    0x40
#define ISP2400HCCRS_RISC_RESET           0x20
#define ISP2400HCCRS_RISC_PAUSE           0x10

/*
** GPIO Data register
*/
#define ISP2400GPIOD_LED_UPDATE_MASK      0x1C0000
#define ISP2400GPIOD_DATA_UPDATE_MASK     0x30000
#define ISP2400GPIOD_SFP_PRESENT          0x80
#define ISP2400GPIOD_LED_CONTROL_MASK     0x1C
#define ISP2400GPIOD_ENABLE_MASK          0x3

/*
** GPIO Enable Register
*/
#define ISP2400GPIOE_ENABLE_UPDATE_MASK     0x30000
#define ISP2400GPIOE_ENABLE_MASK            0x3
#define ISP2400GPIOE_SFP_PRESENT_MASK       0x800000
#define ISP2400GPIOE_SFP_PR_ENABLE          0x80

/*
** ICB Q store bits
*/
#define ICB2400_REQ_QSTORE         0
#define ICB2400_RES_QSTORE         1
#define ICB2400_ATIO_QSTORE        2

typedef struct ICB_2400
{
    UINT16  version;                 /* Version                             */
    UINT16  rsv;                     /* Reserved                            */
    UINT16  framePayloadSize;        /* Frame Payload Size                  */
    UINT16  ext;                     /* Execution Throttle                  */
    UINT16  exc;                     /* Exchange Count                      */
    UINT16  hardAddress;             /* HardAddress                         */
    UINT64  portWWN;                 /*  Port Name                          */
    UINT64  nodeWWN;                 /*  Node Name                          */
    UINT16  rsi;                     /* Response Queue Inpointer            */
    UINT16  rqo;                     /* Request Queue Outpointer            */
    UINT16  lrtc;                    /* login Retry Count                   */
    UINT16  prqo;                    /* Priority Request Queue Outpointer   */
    UINT16  rsl;                     /* Response Queue Length               */
    UINT16  rql;                     /* Request Queue Length                */
    UINT16  linknos;                 /* linkdown on NOS/OLS                 */
    UINT16  prql;                    /* Priority Request Queue Length       */
    UINT32  reqa[2];                 /* Request Queue Address               */
    UINT32  rsqa[2];                 /* Response Queue Address              */
    UINT32  preqa[2];                /* Priority Request Queue Address      */
    UINT16  rsvd[4];                 /* Reserved                            */
    UINT16  atioqin;                 /* ATIO Queue in pointer               */
    UINT16  atioqlen;                /* ATIO Queue length                   */
    UINT32  atioqaddr[2];            /* ATIO Queue Address                  */
    UINT16  intd;                    /* Interrupt Delay Timer               */
    UINT16  loginTimeout;            /* Login time out                      */
    UINT32  fwo1;                    /* Firmware Options1                   */
    UINT32  fwo2;                    /* Firmware Options2                   */
    UINT32  fwo3;                    /* Firmware Options3                   */
    UINT16  rsvd1[12];               /* Reserved                            */
} ICB_2400;

#define ISP_REQ_QUE_SIZE   512
#define ISP_RES_QUE_SIZE   256
#define ISP_ATIO_QUE_SIZE  512

/* 2400 ICB firmware option defines
 **
 */
#define ISP_ICB_VERSION         0x1
#define ISP_EXEC_THROTTLE       1024
#define ISP_EXCHANGE_COUNT      1024
#ifdef FRONTEND
#define ISP_LOGIN_RETRY_COUNT   255
#else
#define ISP_LOGIN_RETRY_COUNT   5
#endif
#define ISP_LINKDOWN_TIMEOUT    30
#define ISP_MAXFRAME_SIZE       2048

/* Bit definitions for fwoptions1 field in ICB */
#define ISP_NAME_OPTION         14
#define ISP_FULL_LOGIN_LIP      13
#define ISP_PREV_LID            11
#define ISP_DESLID_SEARCH       10
#define ISP_DISABLE_INITIAL_LIP 9
#define ISP_INITIATOR_MODE      5
#define ISP_TARGET_MODE         4
#define ISP_FULL_DUPLEX         2
#define ISP_FAIRNESS            1
#define ISP_HARD_LID            0

/* Bit definitions for fwoptions2 field in ICB */
#define ISP_FCTAPE_ENABLE      12
#define ISP_FCSP_ENABLE        11
#define ISP_ENABLE_ACK0        9
#define ISP_ENABLE_CLS2        8
#define ISP_NMODE_HARDID       7
#define ISP_CONN_OPTION_BIT1   6
#define ISP_CONN_OPTION_BIT2   5
#define ISP_CONN_OPTION_BIT3   4

/* Connection Bit Values(bits 4 5 6)
 ** 0 - Loop Only
 ** 1 - Point-to_point Only
 ** 2 - Loop Preferred,otherwise point-to-point
 ** 3-7 - Reserved
 */
#define ISP_MODE_BIT1          3
#define ISP_MODE_BIT2          2
#define ISP_MODE_BIT3          1
#define ISP_MODE_BIT4          0

/*  Operational Mode  Values(bits 0 1 2 3)
 ** 0   - ZIO(Zero Input Operation) Disable
 ** 1-4 - Reserved
 ** 5   - ZIO mode 5 is enabled
 ** 6   - ZIO mode 6 is enabled
 ** 7-F - Reserved
 */

 /*Bit definetions for fwooptions3 field in ICB */
#define ISP_75OHM_TERMINATION          16
#define ISP_DATA_RATE_BIT3             15
#define ISP_DATA_RATE_BIT2             14
#define ISP_DATA_RATE_BIT1             13
/*  Data Rate Bit Values(bits 15, 14, 13)
 ** 0  -  1 Gbps
 ** 1  -  2 Gbps
 ** 2  -  Autonegotiated
 ** 3  -  4 Gbps
 ** 4-7-  Reserved
 */
#define ISP_ENABLE_FCP_XFER_RDY_RELOFFSET  9
#define ISP_DISABLE_PLOGI_LOCAL_LOOP       7
#define ISP_ENAABLE_OUTOFORDER_FRAME_HANDLING 6

#define ISP_FCP_RSP_PAYLOADBIT1          5
#define ISP_FCP_RSP_PAYLOADBIT2          4
/*  FCP_RSP pay load bits(5,4)
 ** 0  -  Reserved
 ** 1  -  12 bytes of Zero
 ** 2  -  24 bytes of Zero
 ** 3  -  32 bytes
 */
#define ISP_SOFTID_ONLY                  1

 /* Bit definitions for additional firmware options Mailbox Command */

 /* Incoming reg1 options */

#define ISP_ENABLE_PUREX        10
#define ISP_DISABLE_LED         6
#define ISP_ASYNC_8016          0

/* Incoming reg2 options */
#define ISP_ENABLE_CLASS2       5

/* Incoming reg3 options */
#define ISP_ABORTIO_LINKDOWN   14

/* MULTI-ID related structure definitions */

#define ISP2400_MAX_VPORTS  125

typedef struct ISP2400_VPORT_CONFIG {
    UINT16 reserved;               /* reserved                             */
    UINT8  opts;                   /* vport options                        */
    UINT8  hardAddress;            /* hard/previous ID                     */
    UINT64 portWWN;                /*  Port Name                           */
    UINT64 nodeWWN;                /*  Node Name                           */
} ISP2400_VPORT_CONFIG;

typedef struct ISP2400_VPORT_ICB {
    struct ICB_2400 nicb;          /* Normal initialization control block  */
    UINT16 vpcnt;                  /* Virtual port count                   */
    UINT16 gvpopts;                /* global vport options                 */
                                   /* 125 vport config blocks              */
    struct ISP2400_VPORT_CONFIG vpcfg[ISP2400_MAX_VPORTS];
} ISP2400_VPORT_ICB;

/* Bit definitions for options field in VPort ICB */
#define ISPVP_TARGET_DISABLED     5
#define ISPVP_INITIATOR_ENABLED   4
#define ISPVP_ENABLE              3
#define ISPVP_ID_NOACQUIRE        2
#define ISPVP_PREV_ID             1
#define ISPVP_HARD_ID             0

typedef struct ISP2400_VPDB_PORTCFG {

    UINT16 vpstatus;
    UINT8  vpoptions;
    UINT8  vphardAddress;
    UINT64 portWWN;
    UINT64 nodeWWN;
    UINT32 portid;
} ISP2400_VPDB_PORTCFG;

/* Bit definitions for vpstatus field
** Bits 15-4 Reserved
*/
#define VPDB_NOT_PARTICIPATING    0x8
#define VPDB_ID_ACQUIRED          0x2
#define VPDB_VP_ENABLED           0x1

typedef struct ISP2400_VPDB {
    struct ISP2400_VPDB_PORTCFG ISP2400_VPDB_PORT[ISP2400_MAX_VPORTS];
} ISP2400_VPDB;

typedef struct IOCB_HDR {
    UINT8   iocb_type;
    UINT8   iocb_count;
    UINT8   iocb_sysdefined;
    UINT8   iocb_status;
    UINT32  iocbhandle;
} IOCB_HDR;

/*
*   2400 data segment descriptor
*/
typedef struct DSEG_DESC
{
    UINT64      addr;
    UINT32      length;
} DSEG_DESC;

/*
*   Main iocb for issuing commands for 24xx cards.
*/
typedef struct FCP_CMD_IU_TARG
{
    UINT8       lun[8];
    UINT8       crn;
    UINT8       tag_mode;
    UINT8       task_flags;
    UINT8       rw_flags;
    UINT8       cdb[16];
    UINT32      fcp_dl;
}   FCP_CMD_IU_TARG;

typedef struct FCP_CMD_IU
{
    UINT8       lun[8];
    UINT8       rw_flags;
    UINT8       task_flags;
    UINT8       tag_mode;
    UINT8       crn;
    UINT8       cdb[16];
    UINT32      fcp_dl;
}   FCP_CMD_IU;

/*
**   Main iocb for issuing commands for 24xx cards.
*/
typedef struct CMIO7_IOCB   {
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      iocbhandle;
    UINT16      nphandle;
    UINT16      timeout;
    UINT16      dseg_count;
    UINT16      rsv;
    FCP_CMD_IU  iu;
    UINT16      alpa15_0;
    UINT8       alpa24_16;
    UINT8       vpindex;
    DSEG_DESC   dseg0;
} CMIO7_IOCB;

/*
* IOCB for multiple data segment ops.
*/
typedef struct CMIO6_IOCB   {
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      iocbhandle;
    UINT16      nphandle;
    UINT16      timeout;
    UINT16      dseg_count;
    UINT16      rsp_dsd_len;
    UINT8       lun[8];
    UINT16      control_flags;
    UINT16      fcp_cmd_iu_len;
    UINT64      fcp_cmd_addr;
    UINT64      fcp_rsp_addr;
    UINT32      total_bytes;
    UINT16      alpa15_0;
    UINT8       alpa24_16;
    UINT8       vpindex;
    DSEG_DESC   fcp_data_dsd;
} CMIO6_IOCB;

typedef struct ISP2400_STATUS_TYPE_0
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      iocbhandle;
    UINT16      compStatus;
    UINT16      ox_id;
    UINT32      TranResidLength;
    UINT16      rsv0;
    UINT16      stateFlags;
    UINT16      rsv1;
    UINT16      scsiStatus;
    UINT32      residLength;
    UINT32      senseLength;
    UINT32      respLength;
    UINT8       rspData[28];
}   ISP2400_STATUS_TYPE_0;

typedef struct ISP2400_STATUS_TYPE_0_CONT
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT8       data[60];
} ISP2400_STATUS_TYPE_0_CONT;

typedef struct ISP2400_ABORT_IOCB
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      iocbhandle;
    UINT16      nphandle;
    UINT16      abortoptions;
    UINT32      iocbhandletobeaborted;
    UINT32      rsv0[8];
    UINT16      alpa15_0;
    UINT8       alpa24_16;
    UINT8       vpindex;
    UINT32      rsv2[3];
} ISP2400_ABORT_IOCB;

#define ISP2400_TMF_LUN_RESET        0x10
#define ISP2400_TMF_ABORT_TASK_SET   0x08
#define ISP2400_TMF_CLEAR_TASK_SET   0x04
#define ISP2400_TMF_TARGET_RESET     0x02
#define ISP2400_TMF_CLEAR_ACA        0x01
#define ISP2400_TMF_ABORT_QUEUE      0x80000008
#define ISP2400_TMF_ABORT_TARGET     0x80000002
typedef struct ISP2400_TASK_MANAGEMENT
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      iocbhandle;
    UINT16      nphandle;
    UINT16      rsv0;
    UINT16      delay;
    UINT16      timeout;
    UINT8       lun[8];
    UINT32      controlflags;
    UINT32      rsv1[5];
    UINT16      alpa15_0;
    UINT8       alpa24_16;
    UINT8       vpindex;
    UINT32      rsv2[3];
} ISP2400_TASK_MANAGEMENT;

typedef struct ISP2400_CTIO7
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      iocbhandle;
    UINT16      nphandle;
    UINT16      timeout;
    UINT16      dseg_count;
    UINT8       vpid;
    UINT8       add_flags;

    UINT32      alpa;
    UINT32      exchangeaddr;

    UINT16      rsv;
    UINT16      flags;
    UINT32      residXferLen;
    UINT16      ox_id;
    UINT16      scsi_status;
    UINT32      rsv0[7];
} ISP2400_CTIO7;

typedef struct ISP2400_CTIO7_MODE0
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      iocbhandle;
    UINT16      nphandle;
    UINT16      timeout;
    UINT16      dseg_count;
    UINT8       vpid;
    UINT8       add_flags;
    UINT32      alpa;
    UINT32      exchangeaddr;
    /*rest unique to mode 0*/
    UINT16      rsv;
    UINT16      flags;
    UINT32      residXferLen;
    UINT16      ox_id;
    UINT16      scsi_status;
    UINT32      relOffset;
    UINT32      rsv1;
    UINT32      xferLen;
    UINT32      rsv2;
    DSEG_DESC   dseg0;
} ISP2400_CTIO7_MODE0;

#define CTIO7_STATUS_OK           0x01
#define CTIO7_STATUS_ABORTED      0x02
#define CTIO7_STATUS_ERROR        0x04
#define CTIO7_STATUS_INVALID      0x06
#define CTIO7_STATUS_INVALID_EX   0x08
#define CTIO7_STATUS_OVERRUN      0x09
#define CTIO7_STATUS_TIMEOUT      0x0B
#define CTIO7_STATUS_LIP_RCV      0x0E
#define CTIO7_STATUS_DMA_ERR      0x10
#define CTIO7_STATUS_DATA_ASM_ERR 0x11
#define CTIO7_STATUS_UNDERRUN     0x15
#define CTIO7_STATUS_PORT_UNAVAIL 0x28
#define CTIO7_STATUS_PORT_LOGOUT  0x29
#define CTIO7_STATUS_PORT_CHANGE  0x2A
#define CTIO7_STATUS_SRR_RCV      0x45

typedef struct ISP2400_CTIO7_STATUS
{
  UINT8       entryType;
  UINT8       entryCount;
  UINT8       sysdef;
  UINT8       entryStatus;
  UINT32      iocbhandle;
  UINT16      status;
  UINT16      timeout;
  UINT16      dseg_count;
  UINT8       rsv0[6];
  UINT32      exchangeaddr;

  UINT16      rsv1;
  UINT16      flags;
  UINT32      residXferLen;
  UINT16      ox_id;
  UINT16      rsv2[15];
} ISP2400_CTIO7_STATUS;

typedef struct ISP2400_CTIO7_MODE1
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      iocbhandle;
    UINT16      nphandle;
    UINT16      timeout;
    UINT16      dseg_count;
    UINT8       vpid;
    UINT8       add_flags;
    UINT32      alpa;
    UINT32      exchangeaddr;
    /*rest unique to mode 1*/
    UINT16      sense_length;
    UINT16      flags;
    UINT32      residXferLen;
    UINT16      ox_id;
    UINT16      scsi_status;
    UINT16      resp_length;
    UINT16      rsv0;
    UINT8       sesnedata[0x18];
} ISP2400_CTIO7_MODE1;

typedef struct ISP2400_ATIO7
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT16      fcpcmdlen;
    UINT32      exchangeaddr;
    UINT8       r_ctl;
    UINT8       d_id[3];
    UINT8       cs_ctl;
    UINT8       s_id[3];
    UINT8       type;
    UINT8       f_ctl[3];
    UINT8       seq_id;
    UINT8       df_ctl;
    UINT16      seq_cnt;
    UINT16      ox_id;
    UINT16      rx_id;
    UINT32      parameter;
    FCP_CMD_IU_TARG  iu;
} ISP2400_ATIO7;

#define ISP2400_INOTIFY_ELS 0x46

typedef struct ISP2400_INOTIFY
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      rsv0;
    UINT16      nphandle;
    UINT16      rsv1;
    UINT16      flags;
    UINT16      srr_rx_id;
    UINT16      status;
    UINT8       status_subcode;
    UINT8       rsv2;
    UINT32      rcv_exchangeaddr;
    UINT16      srr_offest15_0;
    UINT16      srr_offest31_16;
    UINT16      srr_iu;
    UINT16      srr_ox_id;
    UINT8       rsv3[0x13];
    UINT8       vpid;
    UINT32      rsv4;
    UINT32      alpa;
    UINT16      rsv5;
    UINT16      ox_id;
} ISP2400_INOTIFY;


typedef struct ISP2400_NTACK
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      iocbhandle;
    UINT16      nphandle;
    UINT16      rsv1;
    UINT16      flags;
    UINT16      srr_rx_id;
    UINT16      status;
    UINT8       status_subcode;
    UINT8       rsv2;
    UINT32      rcv_exchangeaddr;
    UINT16      srr_offest15_0;
    UINT16      srr_offest31_16;
    UINT16      srr_iu;
    UINT16      srr_flags;
    UINT8       rsv3[0x13];
    UINT8       vpid;
    UINT8       srr_rejectvu;
    UINT8       srr_rejectexp;
    UINT8       srr_reject;
    UINT8       rsv4;
    UINT16      rsv5[3];
    UINT16      ox_id;
} ISP2400_NTACK;

typedef struct ISP2400_ABTS
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT8       rsv[6];
    UINT16      nphandle;
    UINT16      controlflags;
    UINT16      softype;
    UINT32      rcv_exchangeaddr;
    UINT8       d_id[3];
    UINT8       r_ctl;
    UINT8       s_id[3];
    UINT8       cs_ctl;
    UINT8       f_ctl[3];
    UINT8       type;
    UINT16      seq_cnt;
    UINT8       df_ctl;
    UINT8       seq_id;
    UINT16      rx_id;
    UINT16      ox_id;
    UINT32      parameter;
    UINT32      rsv2[4];
    UINT32      abort_exchange;
} ISP2400_ABTS;

typedef struct ISP2400_ABTS_ACK
{
    UINT8       entryType;
    UINT8       entryCount;
    UINT8       sysdef;
    UINT8       entryStatus;
    UINT32      iocbhandle;
    UINT8       rsv[2];
    UINT16      nphandle;
    UINT16      controlflags;
    UINT16      softype;
    UINT32      rcv_exchangeaddr;

    UINT8       d_id[3];
    UINT8       r_ctl;
    UINT8       s_id[3];
    UINT8       cs_ctl;
    UINT8       f_ctl[3];
    UINT8       type;
    UINT16      seq_cnt;
    UINT8       df_ctl;
    UINT8       seq_id;
    UINT16      rx_id;
    UINT16      ox_id;
    UINT32      parameter;
    UINT32      payload[3];
    UINT32      rsv2;
    UINT32      abort_exchange;
} ISP2400_ABTS_ACK;

#ifdef FRONTEND
/*
** 2400 Report ID Acquisition IOCB (0x32)
*/
typedef struct VPRID_IOCB
{
    UINT8   entryType;
    UINT8   entryCount;
    UINT8   sysdef;
    UINT8   entryStatus;
    UINT32  iocbHandle;
    UINT16  vpCount;
    UINT8   vpindex;
    UINT8   vpstatus;
    UINT8   pid[3];
    UINT8   format;
    UINT16  vpindexmap[8];
    UINT8   rsvd[32];
} VPRID_IOCB;
/*
** Virtual Port Control IOCB (0x30)
*/

typedef struct ISP2400_VPCONTROL {
  UINT8   entryType;
  UINT8   entryCount;
  UINT8   sysdef;
  UINT8   entryStatus;
  UINT32  handle;
  UINT16  vpindxfailed;
  UINT16  status;
  UINT8   command;
  UINT8   vpCount;
  UINT16  vpindexmap[8];
  UINT8   rsvd[32];
} ISP2400_VPCONTROL;

#endif  /* FRONTEND */

/*
** Mailbox IOCB (0x39)
*/
typedef struct ISP2400_MBIOCB
{
  UINT8   entryType;
  UINT8   entryCount;
  UINT8   sysdef;
  UINT8   entryStatus;
  UINT32  handle;
  UINT16  mbox[28];
} ISP2400_MBIOCB;

/*
**  CT Pass-Through IOCB Structure Definitions
*/
#define CT_PASSTHRU_IOCB             0x29
#define SNS_NPORT_HANDLE     0x7FC
#define FABIC_CNTL_NPORT_HANDLE 0x7FD
#define FABIC_PORT_NPORT_HANDLE 0x7FE
typedef struct CTPASSTHRU_IOCB {
        UINT8   entryType;
        UINT8   entryCount;
        UINT8   sysdef;
        UINT8   entryStatus;
        UINT32  iocbHandle;
        UINT16  cmplstatus;
        UINT16  nportHandle;
        UINT16  cmddsdCount;
        UINT8   vpindex;
        UINT8   rsvd;
        UINT16  cmdTimeout;
        UINT16  rsvd1;
        UINT16  rspdsdCount;
        UINT8   rsvd2[10];
        UINT32  rsptotalBytes;
        UINT32  cmdtotalBytes;
        UINT32  dsd0[2];
        UINT32  dsd0Length;
        UINT32  dsd1[2];
        UINT32  dsd1Length;
} CTPASSTHRU_IOCB;

#define ELS_PASSTHRU_IOCB             0x53
typedef struct ELSPASSTHRU_IOCB
{
        UINT8   entryType;
        UINT8   entryCount;
        UINT8   sysdef;
        UINT8   entryStatus;
        UINT32  iocbHandle;
        UINT16  rsvd0;/*cmplstatus; */
        UINT16  nportHandle;
        UINT16  TxDsdCount;
        UINT8   vpindex;
        UINT8   SOFType;
        UINT32  RxXchgAddr;
        UINT16  RxDsdCount;
        UINT8   ElsOpcode;
        UINT8   rsvd1;
        UINT16  Did0_15;
        UINT8   Did23_16;
        UINT8   rsvd2;
        UINT16  rsvd3;
        UINT16  ControlFlags;
        UINT32  RxtotalBytes; /* union UINT32 DataByteCount;  */
        UINT32  TxtotalBytes; /* union UINT32 errorSubcode1; */
        UINT32  Txdsd[2];     /* union UINT32 errorSubcode2; */
        UINT32  TxdsdLength;
        UINT32  Rxdsd[2];
        UINT32  RxdsdLength;
} ELSPASSTHRU_IOCB;

/*
**  Control Flag bit definitions
*/

#define ELS_PAYLOADBIT1         13
#define ELS_PAYLOADBIT2         14
#define ELS_PAYLOADBIT3         15
/*
**Bit 15,14,13 payload descriptor
** 0     Originator
** 1     Responder
** 2     Responder
** 3     Responder
** 4-7   Reserved
*/
#define ELS_CLEAR_PASSTHRU_PENDING      12
#define ELS_INCLUDE_FRAME_HEADER        11
#define ELS_SOFI3               (1 << 4)
#define ELS_SOFI2               (3 << 4)

typedef struct ELS_SCR
{
    UINT8    opcode;
    UINT8    parms[3];
    UINT32   Function;
} ELS_SCR;

typedef struct SCR_LSACC
{
    UINT8    AccCode;
    UINT8    params[3];
} SCR_LSACC;

typedef struct LS_LSRJT
{
    UINT16      params;
    UINT16      RjCode;
    UINT8       rsvd;
    UINT8       ReasonCode;
    UINT8       ReasonExpln;
    UINT8       VendorUnique;
} SCR_LSRJT;

typedef struct
{
    UINT32 vendID;                  /* Vendor ID                            */
    UINT32 model;                   /* Vendor model                         */
    UINT16 revLvl;                  /* Revision Level of ISP                */
    UINT16 rscLvl;                  /* RISC revision Level                  */
    UINT16 fpmLvl;                  /* FB & FPM revision levels             */
    UINT16 romLvl;                  /* RISC ROM revision level              */
    UINT64 type;                    /* Firmware type (ef/efm)               */
    UINT16 fwMajor;                 /* ISP firmware major revision          */
    UINT16 fwMinor;                 /* ISP firmware minor revision          */
    UINT16 fwSub;                   /* ISP firmware subminor revision       */
    UINT16 fwAttrib;                /* ISP firmware attribute               */
    UINT16 dataRate;                /* Data Rate (1G/2G)                    */
    UINT32 endMemAddr;              /* Ending memory Addr(Used in Debug
                                       Dump Procedure                       */
} ISP_REV;

typedef struct ISP_STR
{
    ISP_2400 *baseAd;               /* ISP PCI base address                 */
    QCB    *reqQue;                 /* ISP request QCB location             */
    QCB    *resQue;                 /* ISP response QCB location            */
    ICB_2400 *icbStr;               /* ISP ICB location                     */
    volatile UINT16 *mBox;          /* ISP mailbox base address             */
    void   *rsvd[4];                /* used by asm but not C code           */
    QCB    *atioQue;                /* ISP response QCB location            */
} ISP_STR;

typedef struct FLS
{
    UINT32  lifCnt;                 /* Link Failure Count                   */
    UINT32  lssCnt;                 /* Loss of Sync Count                   */
    UINT32  lsgCnt;                 /* Loss of Signal Count                 */
    UINT32  pspEc;                  /* Primitive Seq error count            */
    UINT32  ivTqc;                  /* Inv. Xmission Word Count             */
    UINT32  ivCrc;                  /* Invalid CRC count                    */
} FLS;

typedef struct ISP_DUMP
{
    void    *addr;                  /* Address where sram dump is stored    */
    UINT32  length;                 /* Length of sram dump                  */
    UINT16  reason;                 /* Reason code for dump                 */
    UINT8   count;                  /* Dump count                           */
    UINT8   debug;                  /* Debug dump progress indicator        */
    UINT32  timestamp;              /* Time stamp of dump                   */
} ISP_DUMP;

/*
** Port/Node Name List Data Layout for 2300
*/
typedef struct PNNL
{
    UINT64 ndn;                     /* Node Name                            */
    UINT64 pdn;                     /* Port Name                            */
    UINT16 lid;                     /* Loop ID                              */
    UINT8  opt;                     /* Options                              */
    UINT8  cnt;                     /* Control  (do not use)                */
    UINT8  mst;                     /* Master State                         */
    UINT8  sst;                     /* Slave State                          */
    UINT8  had;                     /* Hard address from ADISC              */
    UINT8  rsvd;                    /* Reserved                             */
    UINT32 portID;                  /* Port ID                              */
    UINT16 prliW0;                  /* prli ser parm word 0                 */
    UINT16 prliW3;                  /* prli ser parm word 3                 */
} PNNL;

/*
** Port/Node Name List Data Layout for 2400
*/
typedef struct PNNL_2400
{
    UINT16  flags;
    UINT8   curloginstate;
    UINT8   lastloginstate;
    UINT32  hardaddr;
    UINT8   portid[3];
    UINT8   seqid;
    UINT16  porttimer;
    UINT16  nporthandle;
    UINT16  rcvdatasize;
    UINT16  rsv0;
    UINT16  prliW0;
    UINT16  prliW3;
    UINT64  portname;
    UINT64  nodename;
} PNNL_2400;

/*
** Get port database structure for the 2400 chip
** not it is identical to PNNL_2400 expcept for the
** 6 reserved words on the end.
*/
typedef struct PDB_2400
{
    UINT16  flags;
    UINT8   curloginstate;
    UINT8   lastloginstate;
    UINT32  hardaddr;
    UINT8   portid[3];
    UINT8   seqid;
    UINT16  portTimer;
    UINT16  nporthandle;
    UINT16  recvDataSize;
    UINT16  rsv0;
    UINT16  prliw0;
    UINT16  prliw3;
    UINT64  portname;
    UINT64  nodename;
    UINT32  rsv1[6];
} PDB_2400;

/*
** Virtual Port Database
*/
typedef struct VPD
{
    UINT8  opt;                     /* Options                              */
    UINT8  had;                     /* Hard address from ADISC              */
    UINT64 pdn;                     /* Port Name                            */
    UINT64 ndn;                     /* Node Name                            */
    UINT16 status;                  /* Status                               */
    UINT16 reserved1;               /* Reserved                             */
    UINT16 lid;                     /* Loop ID                              */
    UINT16 lunFlags;                /* LUN flags                            */
    UINT16 crc;                     /* command resource count               */
    UINT16 inrc;                    /* immediate notify resource count      */
    UINT16 lunTimeout;              /* LUN timeout                          */
    UINT16 reserved2;               /* Reserved                             */
    UINT16 alpa;                    /* ALPA                                 */
    UINT16 reserved3;               /* Reserved                             */
    UINT16 reserved4;               /* Reserved                             */
} VPD;

typedef struct PORT_ID_LIST
{
    UINT32  alpa;
    UINT32  nphandle;
} PORT_ID_LIST;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern UINT32 isp2400;                  /* MAXISP - ISP 24xx indicator      */
extern UINT32 isp2500;                  /* MAXISP - ISP 25xx indicator      */
extern UINT32 icb2400;                  /* ICB Q indicator      */
extern UINT32 resilk;                   /* MAXISP - Reset Interlock         */
extern struct ISP_DUMP  ispdump[];      /* ISP dump data structs            */
extern ISP_REV *isprev[MAX_PORTS+MAX_ICL_PORTS]; /* ISP revision information */
extern ISP_STR *ispstr[MAX_PORTS];      /* ISP data structs                 */
extern UINT16           ispmax;         /* Maximum number of adapter        */
extern UINT32           ispmid;         /* MAXISP - ISP Multi ID Code Indicator */
extern UINT8           *lpmap[MAX_PORTS+MAX_ICL_PORTS];
extern UINT32   ispLid[MAX_PORTS];
extern UINT64          *servdb[MAX_PORTS];
#if ISCSI_CODE
extern UINT32           iscsimap;       /* iSCSI Port -bitmap               */
#endif
extern UINT32           ispfail;        /* MAXISP - bitmap                  */
extern UINT32           ispofflfail;    /* MAXISP - bitmap                  */
extern UINT32           ispmap;         /* MAXISP - bitmap                  */
extern UINT32           isprena;        /* MAXISP - bitmap                  */
extern UINT32           ispOnline;      /* MAXISP - bitmap                  */
extern UINT32           ispCp;          /* MAXISP - bitmap                  */
extern UINT32           portid[MAX_PORTS];
extern UINT32  ispConnectionType[MAX_PORTS];
extern QCB             *asyqa[MAX_PORTS];
extern PCB             *rtpcb[MAX_PORTS * 2];
extern UINT16           ispLipIssued[MAX_PORTS]; /* ISP LIP issued          */
extern UINT8            ispFailedPort[MAX_PORTS];
extern UINT8            ispPortAssignment[MAX_TARGETS];
extern UINT32           ispLastLIP[MAX_PORTS];
extern UINT32           timestamp;
extern UINT16           ispGPIOD[MAX_PORTS]; /* ISP GPIOD register contents */
extern UINT8 gRegTargetsOK;    /* Flag to show OK for Regular  Targets on FE*/

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern UINT32 isp_gnnFt(UINT8 port);
extern UINT32 isp_gpnId(UINT8 port, UINT32 portId, UINT64 *portName);
extern UINT32 isp_GidPN(UINT8 port, UINT64 portName,UINT32 * portId );
extern UINT32 isp_GidNN(UINT8 port, UINT64 NodeName,UINT32 * portId );
extern UINT32 isp_gnnId(UINT8 port, UINT32 portId, UINT64 *nodeName);
extern UINT16 ISP_AbortTarget(UINT8 port, UINT32 lid);

extern void   ISP_DumpQL(UINT8 port, UINT32 reason);
extern void   ISP_FindPortToReset(UINT32 reinit);
extern UINT16 ISP_ForceSystemError(UINT8 port);
extern UINT16 ISP_GetFirmwareState(UINT8 port);
extern UINT32 ISP_IsReady(UINT32 port);
extern UINT32 ISP_IsFabricMode(UINT32 port);
extern UINT16 ISP_initiateLip(UINT8 port);
extern UINT16 ISP2400_GetLinkStatistics(UINT8 port, void *buffer);
extern UINT16 ISP_GetLinkStatus(UINT8 port, UINT32 lid, void *pBuffer);
extern UINT16 ISP_GetLoopId(UINT8 port, UINT8 vpid);
extern UINT16 ISP_GetPortDB(UINT8 port, UINT32 lid, UINT16 vPID);
extern UINT32 ISP_EnhancedGetPortDB(UINT8 port, UINT32 lid, UINT32 alpa);
extern UINT32 ISP_EnhancedGetPortDBHdl(UINT8 port, UINT32 lid);
extern UINT16 ISP_GetPortNodeNameList(UINT8 port, UINT8 vpIndex, PNNL *pBuffer);
extern UINT16 ISP_GetPositionMap(UINT8 port);
extern UINT16 ISP_GetServersLogonList(UINT16 targetId, UINT8 *pList);
extern UINT16 ISP_GetVPDatabase(UINT8 port, VPD *pBuffer);
extern UINT16 ISP2400_GetVPDatabase(UINT8 port, ISP2400_VPDB_PORTCFG *pBuffer);
extern UINT16 ISP_LipReset(UINT8 port, UINT32 lid);
extern UINT32 ISP_LoginLoopPort(UINT8 port, UINT32 lid, UINT8 vpid);
extern UINT32 ISP_LoopInitialize(UINT8 port, UINT32 lid);
extern UINT32 ISP_Login(UINT8 port, UINT32 lid);
extern UINT32 ISP_LoginFabricPort(UINT8 port, UINT32 *lid, UINT32 portID);
extern UINT32 ISP_LoginFabricPortIOCB(UINT8 port, UINT32 *lid, UINT32 portID, UINT8 vpid);
extern UINT32 ISP_LogoutFabricPort(UINT8 port,UINT32 lid,UINT32 ourAlpa);
extern UINT16 ISP_LoopPortBypass(UINT8 port, UINT32 lid);
extern UINT16 ISP_LoopPortEnable(UINT8 port, UINT32 lid);
extern UINT16 ISP_PortLogout(UINT8 port, UINT32 lid);
extern UINT32 ISP_ResetChip(UINT8 port, UINT32 reason);
extern void   ISP_SubmitMarker(UINT8 port, UINT16 modifier, UINT32 lid, UINT16 lun);
extern void   ISP2400_IntrServiceRoutine(UINT32);
extern UINT16 ISP_TargetReset(UINT8 port, UINT32 lid);
extern UINT16 ISP_GetFWQueueDepth(UINT8 port);
extern UINT32  ISP2400_SetupInit(UINT32 port);
extern UINT32 ISP2400_IcbStore(UINT32, UINT32);
extern UINT32 ISP2400_BuildVPICB(UINT32 port);
extern UINT32  ISP2400_InitFW(UINT32 port);
extern UINT32  ISP2400_ResetChip(UINT32 port);
extern void isp_newTarget(TAR *, TGD *, UINT8);
extern UINT16 isp2400_initiate_io(UINT16 port, struct ILT *ilt, struct DEV *dev);
extern UINT32 isp2400_sendctRFT_ID(UINT8 port, UINT32 fc4Type, UINT32 portID, UINT32 vpindex);
extern UINT32 isp2400_sendctGAN(UINT16 port, UINT32 buffer, UINT32 portId);

#ifdef FRONTEND
extern UINT32 isp2400_build_ctio7(UINT16 port, struct ILT *ilt);
extern void isp2400_build_ntack(UINT16 port, struct ILT *ilt, ISP2400_NTACK *iocb);
extern UINT8 ISP_IsPrimaryPort(UINT16 port, UINT16 vpID);
extern void isp2400_targetiocb(UINT32 status, struct ILT *pILT);
extern void CT_LC_isp2400_targetiocb(UINT32 status, struct ILT *pILT);
extern void isp_abort_exchange_ilt(UINT16 port, struct ILT *ilt);
#else
extern UINT32  ISP_PDisc(UINT8 port , UINT32 handle);
#endif /* FRONTEND */

extern void FAB_clearLid(UINT8 port);
extern UINT16  isp_alpa2handle(UINT8 port, UINT32 alpa, UINT8 vpid);
extern UINT32  isp_handle2alpa(UINT8 port, UINT32 nphandle);
extern UINT32  isp_isdevicelocal(UINT8 port, UINT32 nphandle, UINT8 vpid);

#endif /* ISP_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
***/
