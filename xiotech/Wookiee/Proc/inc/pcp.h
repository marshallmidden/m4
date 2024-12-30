/* $Id: pcp.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       pcp.h
**
**  @brief      Process Control Packet
**
**      This file defines the data structures and definitions to support
**      the process control operations.
**
**  Copyright (c) 2004-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef _PCP_H_
#define _PCP_H_

struct CM;
struct COR;

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/* Completion status code definitions */
#define    PCPST_OK            0        /* successful                       */
#define    PCPST_LLFUERR       1        /* process error                    */
#define    PCPST_POLLINHIBIT   2        /* poll inhibit                     */
#define    PCPST_TASKEND       0xff     /* CM task termination              */

/* Request function code (pcp2_function) definitions */
#define    PCPFC_UPDATEERR     1        /* update error                     */
#define    PCPFC_SRCCPYMV      2        /* source copy device moved         */
#define    PCPFC_DSTCPYMV      3        /* source copy device moved         */
#define    PCPFC_SWAP          4        /* swap raids of mirror             */
#define    PCPFC_BREAK         5        /* break mirror/copy                */
#define    PCPFC_PAUSE         6        /* pause mirror/copy                */
#define    PCPFC_RESUME        7        /* resume mirror/copy               */
#define    PCPFC_ABORT         8        /* abort mirror/copy                */
#define    PCPFC_POLL          9        /* poll assoc. remote MAGNITUDES    */

#define    PCPFC_AUTOBREAK     0x0c     /* auto break                       */
#define    PCPFC_AUTOABORT     0x0d     /* auto abort                       */

#define    PCPFC_OWNERACQ      0x10     /* Ownership Acquired task          */
#define    PCPFC_SPNDCPYTASK   0x11     /* Suspend copy task                */
#define    PCPFC_INSTANTMIRROR 0x12     /* Instant Mirror  ## VIJAY         */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/*
        Process Control Packet Level #1 Definition
        -----------------------------------------------
        This table defines the data structure used by a client to manage a
        segment copy request. This level is for the exclusive use of the client.

        This data structure is built and maintained in an ILT.
 */
struct PCP1 {
    struct PCP1 *link;                  /* Link list field. */
    UINT32       resv1;                 /* Originally backward thread. */
    UINT32       resv2;                 /* Originally pcb of caller. */
    void        *cr;                    /* Completion handler routine. */

    struct PCB  *pcb;                   /* Client process pcb address.  (w0) */
    UINT8        status;                /* Completion status. (w1+0) */
    UINT8        rtstate;               /* Requested task state. (w1+1) */
    UINT8        resv3[2];              /* Two reserved bytes. (w1+2 and w1+3) */

    UINT32       reg1;                  /* w2 */
    UINT32       reg2;                  /* w3 */
    UINT32       reg3;                  /* w4 */
    UINT32       reg4;                  /* w5 */
    UINT32       reg5;                  /* w6 */
    struct CM   *cm;                    /* CM address (w7) */
};

/*
        Process Control Packet Level #2 Definition
        -----------------------------------------------
        This table defines the data structure used by a client to request
        a process control operation.

        This level is built by the client but is used by the process services
        to manage the request. This data structure is inside an ILT.
 */
struct PCP2 {
    struct PCP2 *link;                  /* Link list field. */
    UINT32       resv1;                 /* Originally backward thread. */
    UINT8        status;                /* Completion status. (pcb+0) */
    UINT8        function;              /* Requested function. (pcb+1) */
                                        /* 0 = none. */
    UINT8        resv2[2];              /* Two reserved bytes. (pcb+2 and pcb+3) */

    void        *handler;               /* Vsync handler script. */
                                        /* 0 = none. */

    UINT32       rqctrlsn;              /* Requestion controller s/n. (w0) */
    UINT32       rid;                   /* Copy reg. ID.  (w1) */
    UINT32       rcsn;                  /* Copy reg. MAG SN.  (w2) */

    UINT8        rcscl;                 /* Copy reg. MAG src cluster.  (w3+0) */
    UINT8        rcsvd;                 /* Copy reg. MAG src vdisk.  (w3+1) */
    UINT8        rcdcl;                 /* Copy reg. MAG dst cluster.  (w3+2) */
    UINT8        rcdvd;                 /* Copy reg. MAG dst vdisk.  (w3+3) */

    UINT32       rssn;                  /* Copy reg. src MAG SN. (w4) */
    UINT32       rdsn;                  /* Copy reg. dst MAG SN. (w5) */

    UINT8        rscl;                  /* Copy reg. src MAG cluster (w6+0) */
    UINT8        rsvd;                  /* Copy reg. src MAG vdisk (w6+1) */
    UINT8        rdcl;                  /* Copy reg. dst MAG cluster (w6+2) */
    UINT8        rdvd;                  /* Copy reg. dst MAG vdisk (w6+3) */
    struct COR  *cor;                   /* CM address (w7) */
};

#endif /* _PCP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
