/* $Id: debug.c 145079 2010-08-04 19:09:12Z mdr $ */
/***********************************************************************
#
#  NAME: debug.c (Debug structures on the jtag debugger)
#
#  DESCRIPTION:
#
#       This creates variables of the defined structure type so that the
#       debugger can display the structures with pneumonics.
#
#  Notes:
#
#       Put arrays on top.
#       Keep the structures in alphabetical order.
#       All arrays and structures start with the name dbg_.
#       All types end with _t.
#       UINT32 doesn't display properly in the debugger so can't be used.
#
#       To view these on the jtag debugger:
#       - open a global memory window
#       - address the dbg_xx pointer variable you want
#       - type in the address of the start of the data
#       - click on 'Ptr'
#       - hide the class and type with commands 'showclass off' and
#         'showtype off'
#
#  Copyright (c) 1996-2008 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "XIO_Const.h"
#include "rb.h"

/**********************************************************************/
/*
**  Data Structures Definitions
*/
/**********************************************************************/

/*
**  --------------------------------------------------------------------
**  DLM Data Structure Usage                               Name     Usage
**  --------------------------------------------------------------------
**
**  datagram.inc  (Req/resp datagram message definitions)  dgrq_    dlm, lld
**                                                         dgrs_    dlm, lld
**                                                         DLM0_    dlm
**  di.inc (Device Links Information Definitions]          di_      dlm
**  dlmio.inc (Data-link Manager I/O definitions)          dlmio_   dlm
**  dspif.inc  (Datagram services provider interface defs) dscX_    dlm
**                                                         dssX_    dlm
**  dtmt.inc  (Data-link Manager Target Management Table)  dtmt_    dlm
**                                                         dml_     dlm
**                                                         dft_     dlm
**  ftdd.inc  (Foreign Target Device Database)             ftdd_    dlm, lld
**  ftdt.inc  (Foreign Target Descriptor Table)            ftdt_    dlm, lld
**  ftospb.inc  (Foreign Target Open Session Param. Block) ftos_    dlm, lld
**  ldd.inc (Linked Device Description definitions)        ldd_     dlm
**  ldi.inc (Linked Device Information)                **  li_      dlm
**  li.inc (VDisk Lock Information Definitions]        **  li_      dlm
**  lldmt.inc (DLM/LLD nest level #2 of the ILT)           lldmt_   dlm
**  lvm.inc (LUN to VDisk mapping)                         lv_      define,dlm,magdrvr
**  magdd.inc  (MAGNITUDE Device Database)                 magdd_   dlm
**  magdt.inc  (MAGNITUDE Descriptor Table)                magdt_   lld, dlm
**  mlmt.inc  (MAGNITUDE Link Management Table)            mlmt_    dlm
**  mlospb.inc (MAGNITUDE Link Open Session Param. Block)  mlos_    dlm, lld
**  srp.inc (Secondary Request Packet)                     sr_      dlm,lld,cache,magdrvr,excopy
**  tpmt.inc (Target Path Management Table definitions)    tpm_     dlm
**  vdd.inc (Virtual Device Description)                   vd_      define,dlm,online,virtual,
**  vdx.inc (Virtual Device indeX)                         vx_      define,dlm,online
**  vlar.inc  (VLink Association Record definitions)       vlar_    dlm
**  vldefs.inc (Virtual Link Definitions]                  vl_      dlm
**  vlop.inc (VLink operation management table definitions)vlop_    dlm
**
**  -------------------------------------------------------------------
**  LLD Data Structure Usage                               Name     Usage
**  -------------------------------------------------------------------
**
** x cimt.inc  (Channel interface management table)         ci_      cdriver, lld, magdrvr
**  datagram.inc  (Req/resp datagram message definitions)  dgrq_    dlm, lld
**                                                         dgrs_    dlm, lld
**                                                         LLD0_    lld
**  ftdd.inc  (Foreign Target Device Database)             ftdd_    dlm, lld
**  ftdt.inc  (Foreign Target Descriptor Table)            ftdt_    dlm, lld
**  ftospb.inc  (Foreign Target Open Session Param. Block) ftos_    dlm, lld
**  ilmt.inc  (Initiator/LUN management table)             ilm_     magdrvr, lld, cdriver
** x imt.inc  (Initiator management table)                  im_      magdrvr, lld, cdriver
**  inqmag.inc  (MAGNITUDE link INQUIRY CDB definitions)   inqinfo_ lld
**  irp.inc (Initiator Request Packet)                     irp_     apldrvr, lld, excopy
**  lld.inc  (Link-level Driver Definitions)               lirp1_   lld
**                                                         lirp2_   lld
**                                                         lvrp2_   lld
**                                                         sdg_     lld
**                                                         sndg2_   lld
**                                                         sndg3_   lld
**  lldmt.inc (DLM/LLD nest level #2 of the ILT)           lldmt_   dlm
**  lsmt.inc  (Link-level Driver Session Management Table) lsmt_    lld
** x ltmt.inc  (Link-level Driver Target Management Table)  ltmt_    lld, magdrvr
**  magdt.inc  (MAGNITUDE Descriptor Table)                magdt_   lld, dlm
**  mlospb.inc (MAGNITUDE Link Open Session Param. Block)  mlos_    dlm, lld
**  srp.inc (Secondary Request Packet)                     sr_      dlm,lld,cache,magdrvr,excopy
** x tlmt.inc (Target/LUN Management Table)                 tlm_     apldrvr,idriver,lld
** x tmt.inc (Target Management Table)                      tm_      apldrvr,idriver,lld
**  vdmt.inc  (Virtual device management table)            vdm_     magdrvr,lld,excopy
**
**  --------------------------------------------------------------------
**  IDRIVER Data Structure Usage                           Name     Usage
**  --------------------------------------------------------------------
**
**  gan.inc (Get All Next Command Response)                gan_     idriver
** x icimt.inc (Initiator Channel Interface Management Tbl) ici_     apldrv, idriver, itraces
**  ismt.inc (Initiator Session Management Table)          ism_     apldrvr, idriver
**  tlmt.inc (Target/LUN Management Table)                 tlm_     apldrvr,idriver,lld
**  tmt.inc (Target Management Table)                      tm_      apldrvr,idriver,lld
**  xli.inc (Translation Level Initiator table)             xliXXXX  apldrvr,idriver,isp
**
**  --------------------------------------------------------------------
**  APLDRVR Data Structure Usage                           Name     Usage
**  -----------------------------------------------------------------------------------
**
**  irp.inc (Initiator Request Packet)                     irp_     apldrvr, lld, excopy
**  ismt.inc (Initiator Session Management Table)          ism_     apldrvr, idriver
**  tlmt.inc (Target/LUN Management Table)                 tlm_     apldrvr,idriver,lld
**  tmt.inc (Target Management Table)                      tm_      apldrvr,idriver,lld
**  xli.inc (Translation Level Initiator table)             xliXXXX  apldrvr,idriver,isp
**
**  --------------------------------------------------------------------
**  All Data Structures Definitions                        Name     Usage
**  --------------------------------------------------------------------
**
**  cd.inc (Chain Descriptor)                              cd_      raid
**  cev.inc (Create / Expand Virtual Device)               ce_      definebe
**  chn.inc (BE Channel Device)                            ch_
**  ci.inc (Cache Information)                             ca_      cache
**  cimt.inc  (Channel interface management table)         ci_      cdriver, lld, magdrvr
**  cqueue.inc (Write Cache Queue definitions)             cXXXX    cache
**  daml.inc (Disk Allocation Map Link)                    da_
**  datagram.inc  (Req/resp datagram message definitions)  dgrq_    dlm, lld
**                                                         dgrs_    dlm, lld
**                                                         DLM0_    dlm
**                                                         LLD0_    lld
**  def.inc (DEFine)                                       mr_ ...
**  dev.inc (BE attached physical device)                  dv_
**  di.inc (Device Links Information Definitions]          di_      dlm
**  dlmio.inc (Data-link Manager I/O definitions)          dlmio_   dlm
**  dma.inc (DMA definitions)                              dma_
**  dspif.inc  (Datagram services provider interface defs) dscX_    dlm
**                                                         dssX_    dlm
**  dtmt.inc  (Data-link Manager Target Management Table)  dtmt_    dlm
**                                                         dml_     dlm
**                                                         dft_     dlm
**  errlog.inc  (Error log data structures & definitions)  err_     cdriver, magdrvr
**  excopy.inc  (Extended Copy Process definitions)        ecr_     excopy
**  fh.inc  (Firmware Header)                              ft_      (none)
**                                                         fl_      define
**                                                         fh_      define
**  ficb.inc (Firmware Initialization Control Block)       fi_
**  fls.inc  (FCAL Link Status)                            fl_      (none)
**  fmm.inc (Free Memory Management)                       fm_
**  fms.inc (Free Memory Statistics)                       fs_
**  fmtunit.inc  (FORMAT UNIT process definitions)         fpmt_
**                                                         fu3_
**                                                         fusl2_
**  fr.inc (Flight Recorder)                               fr_
**  fsys.inc (File System Definition)                      fsd_
**                                                         fsh_
**                                                         fsm_
**  ftdd.inc  (Foreign Target Device Database)             ftdd_    dlm, lld
**  ftdt.inc  (Foreign Target Descriptor Table)            ftdt_    dlm, lld
**  ftospb.inc  (Foreign Target Open Session Param. Block) ftos_    dlm, lld
**  gan.inc (Get All Next Command Response)                gan_     idriver
**  hcb.inc (Host Control Block)                           hc_      (none)
**  cdb.inc (SCSI CDB)                                     cdb_     (none)
**  icimt.inc (Initiator Channel Interface Management Tbl) ici_     apldrv, idriver, itraces
**  ii.inc (Internal Information)                          ii_
**  ilmt.inc  (Initiator/LUN management table)             ilm_     magdrvr, lld, cdriver
**  ilt.inc (InterLayer Transport)                         il_      (all)
**  imt.inc  (Initiator management table)                  im_      magdrvr, lld, cdriver
**  inqmag.inc  (MAGNITUDE link INQUIRY CDB definitions)   inqinfo_ lld
**  iocb.inc (QLogic ISP 2x00 I/O command block)           ioXXXX
**  irp.inc (Initiator Request Packet)                     irp_     apldrvr, lld, excopy
**  ismt.inc (Initiator Session Management Table)          ism_     apldrvr, idriver
**  isp2100.inc (ISP2100 data structure)                   ispXXXX
**  ist.inc  (Initiator Session Table definitions)         ist_     excopy
**  itrace.inc  (Initiator trace record)                    trr_     cdriver,magdrvr,itraces
**  ldd.inc (Linked Device Description definitions)        ldd_     dlm
**  ldi.inc (Linked Device Information)                **  li_      dlm
**  li.inc (VDisk Lock Information Definitions]        **  li_      dlm
**  lidt.inc (Loop ID Table)                               lt_      (none)
**  lld.inc  (Link-level Driver Definitions)               lirp1_   lld
**                                                         lirp2_   lld
**                                                         lvrp2_   lld
**                                                         sdg_     lld
**                                                         sndg2_   lld
**                                                         sndg3_   lld
**  lldmt.inc (DLM/LLD nest level #2 of the ILT)           lldmt_   dlm
**  lsmt.inc  (Link-level Driver Session Management Table) lsmt_    lld
**  lstats.inc (link layer statistics)                     ls_
**  ltmt.inc  (Link-level Driver Target Management Table)  ltmt_    lld, magdrvr
**  lvm.inc (LUN to VDisk mapping)                         lv_      define,dlm,magdrvr
**  magdd.inc  (MAGNITUDE Device Database)                 magdd_   dlm
**  magdt.inc  (MAGNITUDE Descriptor Table)                magdt_   lld, dlm
**  mc.inc (Memory Chain)                                  mc_
**  mlmt.inc  (MAGNITUDE Link Management Table)            mlmt_    dlm
**  mlospb.inc (MAGNITUDE Link Open Session Param. Block)  mlos_    dlm, lld
**  nva.inc (Non-Volatile Activity)                        nv_
**  nvac.inc (Non-Volatile Activity Control)               nc_
**  nvr.inc (Non-Volatile Ram) - OBOLSETE                  nr_
**  pcb.inc (Process Control Block)                        pc_
**  pdd.inc (Physical Device Definition)                   pd_
**  pdx.inc (Physical Device Index table)                  px_
**  portdb.inc (QLogic Port Data Base)                     pdbXXXX
**  portname.inc (FCAL port ID to Server Name translation) pn_
**  prp.inc (Physical Request Packet)                      pr_
**  psd.inc (Physical Segment Description)                 ps_
**  queue_control.h (Queue Block and queuing control)      qc_      LL_LinuxLinkLayer.c
**                                                         qb_      LL_LinuxLinkLayer.c
**  qrp.inc (QLogic ISP 2x00 Request Packet (QRP))         qrpXXXX
**  qu.inc (Queue structures used with executive process)  qu_
**  r5s.inc (Raid 5 Striping)                              r5_
**  rb.inc (Red-Black Interval Tree structure)             rbXXXX   cache
**  rbr.inc (ReBuild Record)                               rbr_
**  rdd.inc (RAID Device Description)                      rd_
**  rdx.inc (RAID Device Index table)                      rx_
**  rpn.inc (RAID Parity Node)                             rp_
**  rrb.inc (RAID Request Block)                           rb_
**  rrp.inc (RAID Request Packet)                          rr_
**  scb.inc (SCRIPT Control Block)                         sc_
**                                                         sme_
**  sdd.inc (Server Device Definition)                     sd_
**  sdx.inc (Server Device Index table)                    sx_
**  sgl.inc (Scatter/Gather List)                          sg_
**  srp.inc (Secondary Request Packet)                     sr_      dlm,lld,cache,magdrvr,excopy
**  tlmt.inc (Target/LUN Management Table)                 tlm_     apldrvr,idriver,lld
**  tmt.inc (Target Management Table)                      tm_      apldrvr,idriver,lld
**  tpmt.inc (Target Path Management Table definitions)    tpm_     dlm
**  trace.inc  (Trace data structures & definitions)       trr_     cdriver,magdrvr,itraces
**  vcd.inc (Virtual Cache Definition)                     vc_      cache
**  vdd.inc (Virtual Device Description)                   vd_      define,dlm,online,magdrvr,virtual
**  vdmt.inc  (Virtual device management table)            vdm_     magdrvr,lld,excopy
**  vdx.inc (Virtual Device indeX)                         vx_      define,dlm,online
**  vlar.inc  (VLink Association Record definitions)       vlar_    dlm
**  vldefs.inc (Virtual Link Definitions]                  vl_      dlm
**  vlop.inc (VLink operation management table definitions)vlop_    dlm
**  vqd.inc (Virtual inQuiry Data)                         vq_      magdrvr
**  vrp.inc (Virtual Request Packet)                       vr_
**  wcache.inc (Write caching related structures)          tg_      cache
**                                                         wt_
**  xdl.inc (Xiotech Device Label)                         xd_
**  xli.inc (Translation Level Initiator table)             xliXXXX  apldrvr,idriver,isp
**  zonefc.inc  (FCAL zoning feature)                      zninq2_  (none)
**                                                         zninact2_(none)
*/

/**********************************************************************/
/*  Name: RBI_NODE                                                    */

struct RBI_NODE_S
{
    UINT32              key;            /* Key, Starting LBA (LSB)      <w>*/
    UINT32              keyHi;          /* Key, Starting LBA (MSB)      <w>*/
    struct RBI_NODE_S   *leftChild;     /* Pointer to right child node  <w>*/
    struct RBI_NODE_S   *rightChild;    /* Pointer to left child node   <w>*/
    struct RBI_NODE_S   *parent;        /* Pointer to parent node       <w>*/
    UINT32              color;          /* Node Color (Red/Black)       <w>*/
    UINT32              endKey;         /* End Key, Ending LBA+1 (LSB)  <w>*/
    UINT32              endKeyHi;       /* End Key, Ending LBA+1 (MSB)  <w>*/
    UINT32              endNode;        /* End Node (LSB)               <w>*/
    UINT32              endNodeHi;      /* End Node (MSB)               <w>*/
    struct RBI_NODE_S   *fthd;
    struct RBI_NODE_S   *bthd;
    struct cache_ilt_t  *iltPtr;        /* Pointer to ILT (data payload)<w>*/
};

extern struct RB_NODE_S nil;

/**********************************************************************/

/*
** Function Prototypes for RB Sanity Check functions
*/
struct RB_NODE_S *RB$locateNextCIntf(struct RB_NODE_S *);
void CheckNodeMaximum(struct RB *pRBIRoot);

/*
*****************************************************************************
**
**  CheckNodeMaximum
**
**  This routine verifies the NodeMaximum is correct in the tree starting at
**  the root of the RBI tree.  If the NodeMax value is incorrect, this function
**  will abort().
**
*****************************************************************************
*/
void CheckNodeMaximum(struct RB *pRBIRoot)
{
    struct RB  *pRBI = NULL;            /* Working RBI Pointer              */
    struct RB  *pRBIPrev = NULL;        /* Previous RBI already processed   */
    UINT64      leftMax = 0;            /* Left Child Node Max              */
    UINT64      rightMax = 0;           /* Right Child Node Max             */
    UINT64      childMax = 0;           /* Maximum of Left and Right Child  */
    UINT64      allMax = 0;             /* Max of My Keym, Left & Right Nodem*/

    /*
    ** Start at the root and walk the tree verifying the RBI NodeMax value
    ** is correct.  The NodeMax value should the same as the Right Hand Childs
    ** NodeMax (if there is one - if not, then should be the same as the KeyMax
    ** value).
    */
    pRBI = pRBIRoot;                    /* Set up to start at the root      */
    while ((pRBI != (struct RB*)((char *)&nil)) && (pRBI != NULL))
    {
        /*
        ** Check Node Max value for this node.  This nodes NodeMax value should
        ** be the same as Maximum of the Left and Right Childs Node Max.  If
        ** a child is missing, assume the value to be zero.
        */
        if ((pRBI->cLeft != (struct RB*)((char *)&nil)) &&
            (pRBI->cRight != (struct RB*)((char *)&nil)))
        {
            /*
            ** Both a valid Left and Right Child
            */
            leftMax = pRBI->cLeft->nodem;
            rightMax = pRBI->cRight->nodem;
            childMax = (leftMax > rightMax ? leftMax : rightMax);
            allMax = (pRBI->keym > childMax ? pRBI->keym : childMax);
            if (pRBI->nodem != allMax)
            {
                fprintf(stderr, "Abort with Both Children - RBI = %p, Nodem = %llx, Max = %llx\n", pRBI, pRBI->nodem, allMax);
                /*
                ** Problem found - Abort
                */
                abort();
            }
        }
        else if ((pRBI->cLeft == (struct RB*)((char *)&nil)) &&
                 (pRBI->cRight == (struct RB*)((char *)&nil)))
        {
            /*
            ** Neither a Left nor a Right Child
            */
            if (pRBI->nodem != pRBI->keym)
            {
                fprintf(stderr, "Abort with Neither Child - RBI = %p, Nodem = %llx, Keym = %llx\n", pRBI, pRBI->nodem, pRBI->keym);
                /*
                ** Problem found - Abort
                */
                abort();
            }
        }
        else if ((pRBI->cLeft == (struct RB*)((char *)&nil)) &&
                 (pRBI->cRight != (struct RB*)((char *)&nil)))
        {
            /*
            ** Only a Right Child exists
            */
            rightMax = pRBI->cRight->nodem;
            allMax = (pRBI->keym > rightMax ? pRBI->keym : rightMax);
            if (pRBI->nodem != allMax)
            {
                fprintf(stderr, "Abort with Right Child only - RBI = %p, Nodem = %llx, Max = %llx\n", pRBI, pRBI->nodem, allMax);
                /*
                ** Problem found - Abort
                */
                abort();
            }
        }
        else
        {
            /*
            ** Only a Left Child Exists
            */
            leftMax = pRBI->cLeft->nodem;
            allMax = (pRBI->keym > leftMax ? pRBI->keym : leftMax);
            if (pRBI->nodem != allMax)
            {
                fprintf(stderr, "Abort with Left Child only - RBI = %p, Nodem = %llx, Max = %llx\n", pRBI, pRBI->nodem, allMax);
                /*
                ** Problem found - Abort
                */
                abort();
            }
        }

        /*
        ** Find the next node to check.  If next Left Childe exists, follow the
        ** left side of the tree.  Else go right.  If neither, walk back to
        ** where we took the last successful branch.
        */
        if (pRBI->cLeft != (struct RB*)((char *)&nil))
        {
            /*
            ** A Left Child exists, go left
            */
            pRBI = pRBI->cLeft;
        }
        else
        {
            /*
            ** No Left Child.  If a Right Child exists, follow to the right
            */
            if (pRBI->cRight != (struct RB*)((char *)&nil))
            {
                pRBI = pRBI->cRight;
            }
            else
            {
                /*
                ** No Left or Right Child, back up until a path has not been
                ** taken yet
                */
                pRBIPrev = pRBI;
                pRBI = pRBI->bParent;

                while ((pRBIPrev != (struct RB*)((char *)&nil)) &&
                       (pRBI != NULL))
                {
                    /*
                    ** If the Parent's Left Child is the one just looked at,
                    ** go to the Right Child if it exists.  If it does not
                    ** exist, back up one more level of the tree
                    */
                    if (pRBIPrev == pRBI->cLeft)
                    {
                        /*
                        ** The Left Child is the one just looked at, try the
                        ** Right Child.  If it does not exist, back up one
                        ** level and try again
                        */
                        if (pRBI->cRight != (struct RB*)((char *)&nil))
                        {
                            /*
                            ** There is a Right Child, go down that path
                            */
                            pRBIPrev = (struct RB*)((char *)&nil);
                            pRBI = pRBI->cRight;
                        }
                        else
                        {
                            /*
                            ** There is no Right Child, back up a level and
                            ** search for another right child, unless we have
                            ** completed both sides of the tree.
                            */
                            if (pRBI->bParent == NULL)
                            {
                                /*
                                ** All Done with walking the tree
                                */
                                pRBIPrev = (struct RB*)((char *)&nil);
                                pRBI = (struct RB*)((char *)&nil);
                            }
                            else
                            {
                                /*
                                ** More tree to search, back up one level
                                */
                                pRBIPrev = pRBI;
                                pRBI = pRBI->bParent;
                            }
                        }
                    }
                    else
                    {
                        /*
                        ** The Left Child is not the one just looked at.  If
                        ** it was the right child and at the top of the tree,
                        ** all Done.  Else back up one level and try again.
                        */
                        if (pRBIPrev == pRBI->cRight)
                        {
                            /*
                            ** The Previous level is the Right Child just
                            ** looked at.  If at the top of the tree, all done.
                            ** Else back up the tree one more level.
                            */
                            if (pRBI->bParent == NULL)
                            {
                                pRBIPrev = (struct RB*)((char *)&nil);
                                pRBI = (struct RB*)((char *)&nil);
                            }
                            else
                            {
                                /*
                                ** Not at the top, back up one level
                                */
                                pRBIPrev = pRBI;
                                pRBI = pRBI->bParent;
                            }
                        }
                    }
                }
            }
        }
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
