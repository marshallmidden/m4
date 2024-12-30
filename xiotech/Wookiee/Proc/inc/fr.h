/* $Id: fr.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       fr.h
**
**  @brief      Flight Recorder Definitions
**
**  To define operation types and queueing structure for a flight recorder.
**
**  The flight recorder allows the ILTs and VRPs/RRPs/PRPs be tracked
**  as they move across the layers. Generally these are saved at the
**  execs and completions routines in a layer. The entries are saved in
**  a circular queue.
**
**  Usage:
**  - enable the DEBUG_FLIGHTREC flag in options.inc.
**  - adjust the queue depth if needed in fr.inc (fr_asize).
**  - enable any other DEBUG_FLIGHTREC flags:
**      - MEMORY traces malloc, mrel, and deferred mrel
**      - TIME adds 4 additional words of data described below.
**
**  Saving Data to the queue:
**  - enclose the entry in the DEBUG_FLIGHTREC assembler flag
**  - fill in the fr_parmN fields with the values you want saved
**    - byte 0 of fr_parm0 is the type defined below.
**    - fr_parm1 should be the ILT when appropriate
**    - fr_parm2 should be the VRP/RRP/SRP/PRP when appropriate
**    - fr_parm3 is user defined
**
**  Viewing the data:
**  - data variable fr_queue points to the circular queue pointers
**  - the 3 words of queue pointers are BEGIN, NEXT, BEGIN, and END
**  - the data immediately follows the queue pointers
**      - words 0-3 are parms 0-3
**      If DEBUG_FLIGHTREC_TIME is enabled 4 additional words are added:
**      - word 4 is the timer interrupt tick counter from the Kii struct
**      - word 5 is the incremental time in useconds
**      - word 6-7 are spares
**
**  Copyright (c) 2000 - 2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _FR_H_
#define _FR_H_

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name   Flight Recorder Types - These go in byte 0 of fr_parm0.
**/
/*@{*/
#define FR_DLM_RRP          0xE0        /**< DLM - rrp received             */
#define FR_DLM_MSGE         0xE1        /**< DLM - message execution        */
#define FR_DLM_DGSEND       0xE2        /**< DLM - datagram send            */
#define FR_DLM_QSRP         0xE3        /**< DLM - Queued to BE SRP Exec    */
#define FR_DLM_BESRP        0xE4        /**< DLM - BE SRP Executive         */
#define FR_DLM_QDRP         0xE5        /**< DLM - Queue to DRP Executive   */
#define FR_DLM_DRP          0xE6        /**< DLM - DRP Executive            */
/*@}*/

/*
**  frt_h_xx codes:
**
**                                     O$hotswap
**  00d0,   0,     0,   0                Start of hot swap task invocation
**  01d0,   0,     0,   0                At top of hot swap
**  02d0,   0/pid, pdd, old dev status   Missing drive
**  03d0,   0,     0,   0                All inquiries completed
**
**                                     O$inquire
**  00d1,   0/pid, pdd, 0                Start of inquire
**  01d1,   0/pid, pdd, pdx addr         Prior to init_drv call
**  02d1,   0/pid, pdd, new status       After init_drv for drive previously operable
**  03d1,   0/pid, pdd, new status       After init_drv for drive previously not operable
**
**                                       o$init_drv
**  00d2,   0/pid, pdd, test type        o$init_drv - start
**  01d2,   0/pid, pdd, status           o$init_drv - end (0|misc|dev|post status)
**
**                                     Misc rebuild and device status
**  00d3, rid/pid, psd, stat|astat       O$hsparepsd - hot spare PSD call
**  01d3,   0,     0,   0,               d$p2update
**  02d3, rid/pid, psd, True/False       rb$canrebuild
**  03d3, rid/pid, psd, answer           RB$canspare - (answer is yes=0/errcode)
**  04d3, caph,   capl, hspare pdd       rb$findhotspare - find hspare call
**  05d3, rid/0,   rdd, rd|rda|0|0       RB_setraidstat - set raid status/astatus
**  06d3, rid/pid, rdd, 0|0|ps|psa       RB_setpsdstat - set PSD status/astatus
**  07d3, rid/0,retstat,rd|rda|0|0       rb$getraiderrorstat - get raid error status
**  08d3, rid/pid, rdd, psd              D_startinitraid - start RAID initialization
**  09d3, rid/pid, pdd, True/False       d$inopchk - inoperable check
**  0ad3, rid/pid, 0,   True/False       D$defchk - active definition check
**  0bd3, failpid, hspid, options        RB$faildev - fail a device MRP
**  0cd3, pNVRSOS, 0,   True/False       N_processSOS
** 0dd3,   0,     0,   0                N_p2update_nvram
**  0ed3, pNVRII,  pPDX,True/False       N_restorenvram
**
**                                     Rebuild
**  00d4, rid/pid, psd, rdd              rb$rebuildpsd - start
**  01d4, rid/pid, psd, rbr              rb$rebuildpsd - rebuild will take place
**  02d4, rid/pid, psd, hspare pid       rb$redirectpsd - redirect PSD
**  03d4, rid/pid, psd, rbr              rb$psd_rebuilder - start of PSD rebuild
**  04d4, rid/pid, psd, rbr              rb$psd_rebuilder - done with PSD rebuild - good
**  05d4, rid/pid, psd, errcode          rb$psd_rebuilder - done with PSD rebuild - all
**  06d4,   0/pid, pdd, rbr              rb$psd_rebuilder - done with PID rebuild
**  07d4, rid/0,   0,   rbr              RB$cancel_rebld - cancel rebuild
**
**                                     RAID error handler
**  00d5, rid/pid, psd, rd|rda|ps|psa    rb$rerror - end - RAID error detected
**  01d5,   0,       0, start/end        rb$rerror_exec - RAID error exec
**  02d5, rid/0,   rdd, ilt              rb$rerror_exec - RAID error exec - each ILT
**  03d5,  ilt,    prp, #/q/r/s prpstat  rb$rerror - start - RAID error detected
*/

/**
**  @name   Hotswap
**/
/*@{*/
#define FR_H_HSWAP0     0x00D0
#define FR_H_HSWAP1     0x01D0
#define FR_H_HSWAP2     0x02D0
#define FR_H_HSWAP3     0x03D0
/*@}*/

/**
**  @name   Inquire
**/
/*@{*/
#define FR_H_INQUIRE0   0x00D1
#define FR_H_INQUIRE1   0x01D1
#define FR_H_INQUIRE2   0x02D1
#define FR_H_INQUIRE3   0x03D1
/*@}*/

/**
**  @name   Init drive
**/
/*@{*/
#define FR_H_INITDRV0   0x00D2
#define FR_H_INITDRV1   0x01D2
/*@}*/

/**
**  @name   Misc functions
**/
/*@{*/
#define FR_H_MISC0      0x00D3
#define FR_H_MISC1      0x01D3
#define FR_H_MISC2      0x02D3
#define FR_H_MISC3      0x03D3
#define FR_H_MISC4      0x04D3
#define FR_H_MISC5      0x05D3
#define FR_H_MISC6      0x06D3
#define FR_H_MISC7      0x07D3
#define FR_H_MISC8      0x08D3
#define FR_H_MISC9      0x09D3
#define FR_H_MISCA      0x0AD3
#define FR_H_MISCB      0x0BD3
#define FR_H_MISCC      0x0CD3
#define FR_H_MISCD      0x0DD3
#define FR_H_MISCE      0x0ED3
#define FR_H_MISCF      0x0FD3
/*@}*/

/**
**  @name   Rebuild functions
**/
/*@{*/
#define FR_H_RBLD0      0x00D4
#define FR_H_RBLD1      0x01D4
#define FR_H_RBLD2      0x02D4
#define FR_H_RBLD3      0x03D4
#define FR_H_RBLD4      0x04D4
#define FR_H_RBLD5      0x05D4
#define FR_H_RBLD6      0x06D4
#define FR_H_RBLD7      0x07D4
#define FR_H_RBLD8      0x08D4       /* SearchforfailedPSD - start          */
#define FR_H_RBLD9      0x09D4       /* SearchforfailedPSD - end            */
#define FR_H_RBLDA      0x0AD4       /* Update Raid Rebuild Write Status    */
/*@}*/

/**
**  @name   Raid error
**/
/*@{*/
#define FR_H_RERR0      0x00D5
#define FR_H_RERR1      0x01D5
#define FR_H_RERR2      0x02D5
#define FR_H_RERR3      0x03D5
/*@}*/

/**
**  @name   Kernel
**/
/*@{*/
#define FR_K_MALLOC     0xC0        /**< memory allocation         */
#define FR_K_MREL       0xC1        /**< memory release            */
#define FR_K_DMREL      0xC2        /**< deferred memory release   */
#define FR_K_XCHANG     0xC3        /**< context switch timing     */
/*@}*/

/**
**  @name   Misc
**/
/*@{*/
#define FR_M_RIP        0xB0
#define FR_M_AIPW       0xB1
#define FR_M_AILT       0xB2        /**< Misc - allocate an ILT             */
#define FR_M_RILT       0xB3        /**< Misc - release an ILT              */
/*@}*/

#define FR_A_           0xA0        /**< AplDriver                          */

#define FR_I_           0x90        /**< IDriver                            */

#define FR_CD_FINDIMT   0x80        /**< CDriver/MagDriver                  */

#define FR_WC_WRPEXEC   0x70        /**< Write Cache - WRP Executive        */
#define FR_WC_WRPQUE    0x71        /**<    - WRP Queue Function            */
#define FR_WC_TDEXEC    0x72        /**<    - TDis Executive                */
#define FR_WC_VDISKTDIS 0x73        /**<    - VDisk Temporary Disable funct */
#define FR_WC_VDDISCMP  0x74        /**<    - VDisk Disable Complete        */

/**
**  @name   Cache
**/
/*@{*/
#define FR_C_QUE        0x60        /**< Queued to Cache Overlapped         */
#define FR_C_EXECB      0x61        /**< Initial Overlapped Check           */
#define FR_C_QIO        0x62        /**< Queued to I/O Executive            */
#define FR_C_EXECIO     0x63        /**< I/O Executive                      */
#define FR_C_NCR1       0x64        /**< Non-cached Read Complete 1         */
#define FR_C_NCR2       0x65        /**< Non-cached Read Complete 2         */
#define FR_C_NCW1       0x66        /**< Non-cached Write Complete 1        */
#define FR_C_NCW2       0x67        /**< Non-cached Write Complete 2        */
#define FR_C_NLC        0x68        /**< Next Level Completion              */
#define FR_C_LOWER      0x69        /**< Call lower function                */
#define FR_C_UPPER      0x6A        /**< Call Upper function                */
#define FR_C_WDCOMP     0x6B        /**< Write Cache Data complete          */
#define FR_C_IOCOMP     0x6C        /**< I/O Complete (or Write Comp)       */
#define FR_C_QUEDRP     0x6D        /**< Que to the DRP Executive           */
#define FR_C_DRPEXEC    0x6E        /**< DRP Executive                      */
#define FR_C_DRPCOMP    0x6F        /**< DRP Completion                     */
/*@}*/

/**
**  @name   Link
**/
/*@{*/
#define FR_L_QUE        0x50        /**< Link                               */
#define FR_L_EXECO      0x51        /**< outbound VRP exec                  */
#define FR_L_EXECI      0x52        /**< inbound VRP exec                   */
#define FR_L_COMP       0x53        /**< completion routine                 */
#define FR_L_EXECC      0x54        /**< completion exec                    */
#define FR_L_SYNC       0x55        /**< processor sync                     */
/*@}*/

/**
**  @name   Virtual
**/
/*@{*/
#define FR_V_EXEC       0x40
#define FR_V_VSCOMP     0x41
#define FR_V_VMCOMP     0x42
/*@}*/

/**
**  @name   Raid
**/
/*@{*/
#define FR_R_EXEC       0x30        /**< main exec                          */
#define FR_R_COMP       0x31        /**< completion                         */
#define FR_R_PARITY     0x32        /**< parity scan bad                    */
#define FR_R_RRB        0x33        /**< raid 5 insert RRB into RPN         */
#define FR_R_RRBCOMP    0x34        /**< raid 5 complete RRB                */
/*@}*/

/**
**  @name   Physical
**/
/*@{*/
#define FR_P_EXEC       0x20
#define FR_P_COMP       0x21
#define FR_P_CANCEL     0x22
#define FR_P_JOIN       0x23
#define FR_P_GETILT     0x24
#define FR_P_CRTDEV     0x1026
#define FR_P_FNDDEV     0x2026
#define FR_P_INIT_DRV   0x3026
#define FR_P_MVDEV      0x4026
#define FR_P_RMDEV      0x5026
#define FR_P_FINDALT    0x6026
#define FR_P_PURGE      0x7026
#define FR_P_DETACH     0x9026
#define FR_P_REQUP      0xA026
#define FR_P_REQDOWN    0xB026
#define FR_P_DISCOVERY  0xC026
#define FR_P_RESCAN     0xD026
#define FR_P_RSCN       0xE026
#define FR_P_LUNDISC    0xF026
#define FR_P_RETRY      0x27
#define FR_P_CHK_COND   0x28
/*@}*/

/**
**  @name   ISP
**/
/*@{*/
#define FR_ISP_IO       0x10
#define FR_ISP_CDB      0x11
#define FR_ISP_BUFFER   0x12
#define FR_ISP_PDBC     0x13
#define FR_ISP_THREAD   0x14
#define FR_ISP_UNTH     0x15
#define FR_ISP_MARKER   0x16
#define FR_ISP_VPICB    0x17
#define FR_ISP_LINIT    0x18

#define FR_ISP_RESET    0x1F
#define FR_ISP_VPCTRL   0x101F
#define FR_ISP_VPIDACQ  0x301F
#define FR_ISP_REGFC4   0x9201F
#define FR_ISP_RESET_ENTER  0x111F
#define FR_ISP_RESET_CHIP   0x121F
#define FR_ISP_RESET_CLEARMB 0x131F
#define FR_ISP_RESET_CLEARILT 0x141F
#define FR_ISP_RESET_RELEASE_LOCK 0x151F
/*@}*/

/**
**  @name   Entry Constants
**/
/*@{*/
#define FR_PARMS        0               /**< parms 0-3.  This must match the
                                         **  number of parms in misc.as.    */
#ifdef DEBUG_FLIGHTREC_TIME

#define FR_TIME     FR_PARMS + 16       /**< absolute time                  */
#define FR_XPCB     FR_TIME + 8         /**< Executing PCB                  */
#define FR_XPCB_RIP FR_XPCB + 4         /**< Latest RIP of executing PCB    */
#define FR_SIZE     FR_XPCB_RIP + 4     /**< Sizeof a flight recorder entry */

#else

#define FR_SIZE     FR_PARMS + 16       /**< Sizeof a flight recorder entry */

#endif
/*@}*/

/* Misc Functions */
#define FRT_H_MISCC     0x0CD3          /**< N_processSOS                   */
#define FRT_H_MISCD     0x0DD3          /**< N_p2update_nvram               */
#define FRT_H_MISCE     0x0ED3          /**< N_restorenvram                 */

#endif /* _FR_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
