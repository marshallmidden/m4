/* $Id: virtual.c 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       virtual.c
**
**  @brief      Virtual support routines written in c.
**
**  Copyright (c) 2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <byteswap.h>
#include <string.h>
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "CT_defines.h"
#include "defbe.h"
#include "ilt.h"
#include "prp.h"
#include "rrp.h"
#include "system.h"
#include "scsi.h"
#include "ecodes.h"
#include "sgl.h"
#include "pcp.h"
#include "mem_pool.h"
#include "virtual.h"
#include "flightrecorder.h"
#include "CT_defines.h"
#include "misc.h"
#include "ddr.h"

/* ------------------------------------------------------------------------ */
extern UINT32 ct_R$que;        // Just need the address of the queue.
extern UINT32 ct_v$vmcomp;     // Just need the address of the completion routine.
extern UINT32 ct_v$vscomp;     // Just need the address of the completion routine.
extern UINT32 ct_D$ctlrqst_cr; // Just need the address of the completion routine.
extern void V$xque(ILT *);

/* ------------------------------------------------------------------------ */
/* Called from assembler routine only. */
void v_exec_2(UINT32 function, VRP *vrp, UINT32 strategy, UINT32 SN_addr,
              UINT32 sectors, VDD *vdd, SGL *sgl, ILT *ilt);

/* Called to assembler from here only. */
extern void CM_Log_Completion(UINT32, COR *);
extern void CM$ctlrqstq(ILT *);
extern void K_comp(struct ILT *);
extern UINT32 CCSM$get_cwip(void);

/* ------------------------------------------------------------------------
 * Public variables not in any header files
 * ------------------------------------------------------------------------ */
/* Array of queue to use, indexed by VID, default to V_exec_qu. */
QU   *V_primap[MAX_VIRTUAL_DISKS] = {[0 ... MAX_VIRTUAL_DISKS-1] = &V_exec_qu};

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   v_io_match_cnt
**
**  @brief  Count the number of matching ILT/VRPs queued to the provided VDD.
**
**  This routine compares the ILT/VRPs queued to the specified VDD with the
**  specified inbound VSDA and length checking for matches. It count the number
**  of ILT/VRPs queued to the VDD and returns this count to the calling routine.
**
**  @param  vrp             - VRP address
**  @param  IO_length       - inbound I/O length
**  @param  vdd             - Associated VDD address
**
**  @return match count.
**
***********************************************************************
**/
UINT32 v_io_match_cnt(VRP *vrp, UINT32 IO_length, VDD *vdd)
{
    UINT32  ret = 0;                    /* Preload match count return with 0. */
    ILT    *ilt;
    VRP    *v;

    ilt = vdd->pOutHead;                /* First ILT/VRP on list. */
    while (ilt != 0)
    {
        v = (VRP *)((ilt-1)->ilt_normal.w0);    /* Associated VRP address. */
        ilt = ilt->fthd;                /* Next ILT/VRP on list. */

        /* If VRP SDA and length matches, I/O matches. */
        if (v->startDiskAddr == vrp->startDiskAddr)
        {
            if (v->length == IO_length)
            {
                ret++;                  /* Increment the match count. */
            }
        }
    }
    return ret;
}   /* End of v_io_match_cnt */

/**
******************************************************************************
**
**  @name   v_genmrrp
**
**  @brief  Submit the next RRP request, and associate with primary ILT.
**
**  To provide a common means of generating and submitting next RRP request
**  from the given parameters. Associate RRP request with the primary ILT.
**
**  An ILT, RRP and SGL are allocated and initialized. The RRP request is
**  queued to the RAID module for further processing.
**
**  @param  strategy            - strategy/function
**  @param  sectors             - RDD segment sector count
**  @param  rdd_sector_offset   - RDD segment sector offset
**  @param  rdd                 - RDD segment
**  @param  byte_offset         - byte offset into VRP SGL
**  @param  remaining_sectors   - total remaining sector count
**  @param  vdd                 - VDD
**  @param  sgl                 - VRP SGL
**  @param  ilt                 - primary ILT
**
**  @return none.
**
// PRIMARY ILT USAGE:
//      CALLER AREA
//      ___________
//      W0 = VRP
//
//      CALLEE AREA
//      ___________
//      W1 = pending RRP count
//      W4 = VDD
//      W5 = composite status
//
// SECONDARY ILT USAGE:
//      CALLER AREA
//      ___________
//      W0 = RRP
//      W3 = primary ILT
//      W4 = VDD
***********************************************************************
**/

void v_genmrrp(UINT32 strategy, UINT32 sectors,
               UINT64 rdd_sector_offset, RDD *rdd,
               UINT32 byte_offset, UINT32 remaining_sectors UNUSED,
               VDD *vdd, SGL *sgl, ILT *ilt)
{
    VRP    *vrp;
    ILT    *new_ilt;
    RRP    *new_rrp;

    ilt->ilt_normal.w1++;                   /* Bump pending RRP count. */
    vrp = (VRP *)((ilt-1)->ilt_normal.w0);  /* Get VRP pointer. */

    /* Get new ILT/RRP. */
    new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)new_ilt);
#endif  /* M4_DEBUG_ILT */
    new_rrp = get_rrp();                    /* Allocate an RRP. */
#ifdef M4_DEBUG_RRP
    CT_history_printf("%s%s:%u get_rrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, new_rrp);
#endif  /* M4_DEBUG_RRP */
    new_ilt->ilt_normal.w0 = (UINT32)new_rrp;   /* Link to RRP in ILT. */

    new_rrp->flags = vrp->gen2;             /* Save vr_use2 value in new RRP. */
    vrp->gen2 = 0;                          /* Clear value in this VRP. */
    new_ilt->ilt_normal.w3 = (UINT32)ilt;   /* Link primary ILT to ILT. */
    new_rrp->function = strategy;           /* Set up function and strategy. */
    new_rrp->length = sectors;              /* Set up sector length. */
    new_rrp->rid = rdd->rid;                /* Set up RAID ID. */
    new_rrp->startDiskAddr = rdd_sector_offset; /* Set up RRP SDA. */

    /* Check if should generate a new SGL. */
    if (sgl != NULL)
    {
        SGL *new_sgl = m_gensgl(sectors, byte_offset, sgl);

        new_rrp->sglSize = new_sgl->size;   /* Set up SGL size in RRP. */
        new_rrp->pSGL = new_sgl;            /* Set up SGL pointer. */
    }
    else
    {
        new_rrp->sglSize = 0;               /* Set up SGL size in RRP. */
        new_rrp->pSGL = NULL;               /* Set up SGL pointer. */
    }

    new_ilt->ilt_normal.w4 = (UINT32)vdd;   /* Save VDD in new ILT. */

    /* Queue RRP request. */
    EnqueueILT((void *)&ct_R$que, new_ilt, (void *)&ct_v$vmcomp);
}   /* End of v_genmrrp */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   v_gensrrp
**
**  @brief  Submit the next RRP request, and associate with primary ILT.
**
**  To provide a means of generating and submitting a single RRP request
**  from the given parameters. RRP is constructed on top of the VRP.
**
**  The VRP pointer is moved to the next layer of the ILT and modified to
**  resemble an RRP. The RRP request is queued to the RAID layer for further
**  processing.
**
**  @param  strategy            - strategy/function
**  @param  sectors             - RDD segment sector count
**  @param  rdd_sector_offset   - RDD segment sector offset
**  @param  rdd                 - RDD segment
**  @param  remaining_sectors   - total remaining sector count
**  @param  vdd                 - VDD
**  @param  sgl                 - VRP SGL
**  @param  ilt                 - primary ILT
**
**  @return none.
**
// ILT USAGE:
//      CALLER AREA
//      ___________
//      W0 = VRP
//      CALLEE AREA
//      ___________
//      W0 = RRP
//      W1 = null
//      W2 = null
//      W3 = VDD
***********************************************************************
**/

void v_gensrrp(UINT32 strategy, UINT32 sectors UNUSED, UINT64 rdd_sector_offset,
               RDD *rdd, UINT32 remaining_sectors UNUSED,
               VDD *vdd, SGL *sgl UNUSED, ILT *ilt)
{
    VRP    *vrp;
    RRP    *rrp;

    vrp = (VRP *)((ilt-1)->ilt_normal.w0);  /* Get VRP pointer. */

    /* Update ILT parameters. */
    ilt->ilt_normal.w0 = (UINT32)vrp;       /* w0 is the VRP pointer. */
    ilt->ilt_normal.w1 = 0;                 /* Clear w1 parameter. */
    ilt->ilt_normal.w2 = 0;                 /* Clear w2 parameter. */
    ilt->ilt_normal.w3 = (UINT32)vdd;       /* w3 is the VDD. */

    /* Convert VRP to RRP. */
    rrp = (RRP*)vrp;
    rrp->flags = vrp->gen2;                 /* Move gen2 into flags of RRP. */
// Following line seems wrong to me, but VIJAY has it in.
    vrp->gen2 = 0;                          /* Clear rebuild issued flag. */
    rrp->function = strategy;               /* Set up RRP function/strategy. */
    rrp->rid = rdd->rid;                    /* Set raid ID into RRP. */
    rrp->startDiskAddr = rdd_sector_offset; /* Set up SDA of RRP. */

    /* Queue RRP request. */
    EnqueueILT((void *)&ct_R$que, ilt, (void *)&ct_v$vscomp);
}   /* End of v_gensrrp */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   v_scrrpensrrp
**
**  @brief  Submit the next RRP request when copy present.
**
**  To provide a common means of generating and submitting a single RRP request
**  from the given parameters when an output operation has been requested and
**  sec. copy ops. are active. The RRP request is constructed on top of the VRP
**  and set up to wait for any sec. copy ops. to complete before returning the
**  VRP back to the caller.
**
**  The VRP pointer is moved to the next layer of the ILT and modified to
**  resemble an RRP. The RRP request is queued to the RAID layer for further
**  processing.
**
**  @param  strategy            - strategy/function
**  @param  sectors             - RDD segment sector count
**  @param  rdd_sector_offset   - RDD segment sector offset
**  @param  rdd                 - RDD segment
**  @param  remaining_sectors   - total remaining sector count
**  @param  vdd                 - VDD
**  @param  sgl                 - VRP SGL
**  @param  ilt                 - primary ILT
**
**  @return none.
**
// ILT USAGE:
//      CALLER AREA
//      ___________
//      W0 = VRP
//      CALLEE AREA
//      ___________
//      W0 = RRP
//      W1 = pending RRP count
//      W3 = primary ILT
//      W4 = VDD
//      W5 = composite status
***********************************************************************
**/
void v_gensscrrp(UINT32 strategy, UINT32 sectors UNUSED, UINT64 rdd_sector_offset,
               RDD *rdd, UINT32 remaining_sectors UNUSED,
               VDD *vdd, SGL *sgl UNUSED, ILT *ilt)
{
    VRP    *vrp;
    RRP    *rrp;

    vrp = (VRP *)((ilt-1)->ilt_normal.w0);  /* Get VRP pointer. */

    /* Update ILT parameters. */
    ilt->ilt_normal.w0 = (UINT32)vrp;       /* w0 is the VRP pointer. */
    ilt->ilt_normal.w1++;                   /* Increment Pending I/O count. */
    ilt->ilt_normal.w2 = 0;                 /* Clear w2 parameter. */
    ilt->ilt_normal.w3 = (UINT32)ilt;       /* w3 is the primary ILT. */
    ilt->ilt_normal.w4 = (UINT32)vdd;       /* w4 is the VDD. */
    ilt->ilt_normal.w5 = 0;                 /* w5 is the composite status. */

    /* Convert VRP to RRP. */
    rrp = (RRP*)vrp;
    rrp->flags = vrp->gen2;                 /* Move gen2 into flags of RRP. */
// Following line seems wrong to me, but VIJAY has it in.
    vrp->gen2 = 0;                          /* Clear rebuild issued flag. */
    rrp->function = strategy;               /* Set up RRP function/strategy. */
    rrp->rid = rdd->rid;                    /* Set raid ID into RRP. */
    rrp->startDiskAddr = rdd_sector_offset; /* Set up SDA of RRP. */

    /* Queue RRP request. */
    EnqueueILT((void *)&ct_R$que, ilt, (void *)&ct_v$vmcomp);
}   /* End of v_gensscrrp */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   v_exec_2
**
**  @brief  This routine provides phase 2 processing of incoming VRPs to v$exec.
**
**  This routine schedules the appropriate RRPs to the RAID level and then
**  checks for any secondary copy processes associated with the VRP and
**  schedules them as necessary.
**
**  @param  function    - VRP function code
**  @param  vrp         - VRP address
**  @param  strategy    - VRP function/strategy
**  @param  SN_addr     - primary SN address
**  @param  sectors     - I/O length
**  @param  vdd         - VDD address
**  @param  sgl         - VRP SGL
**  @param  ilt         - primary ILT/VRP address
**
**  @return none.
**
***********************************************************************
**/
void v_exec_2(UINT32 function, VRP *vrp, UINT32 strategy, UINT32 SN_addr,
              UINT32 sectors, VDD *vdd, SGL *sgl, ILT *ilt)
{
    RDD  *rdd;
    UINT64 segment_sector_offset;
    UINT64 remaining_sectors;
    int secondary_ops = 0;
    ILT *link_ilt = 0;

    /* Prepare to generate one or more RRPs as necessary. */

    /* Clear special processing flags. */
    function = function & ~((1 << VRP_SPECIAL) | (1 << APOOL_BYPASS));
    strategy = strategy & ~((1 << VRP_SPECIAL) | (1 << APOOL_BYPASS));

    rdd = vdd->pRDD;                    /* Get first RDD. */
    segment_sector_offset = vrp->startDiskAddr; /* Get SDA. */
    while (segment_sector_offset >= rdd->devCap)
    {
        /* Account for previous raid segment. */
        segment_sector_offset -= rdd->devCap;
        rdd = rdd->pNRDD;               /* Link to next RAID segment. */
    }
    /* Calculate remaining sectors in 1st segment. */
    remaining_sectors = rdd->devCap - segment_sector_offset;

    /* Use smaller of remaining sectors in 1st segment or total transfer count. */
    remaining_sectors = (sectors >= remaining_sectors) ? remaining_sectors : sectors;

    /* Update VDD statistics. */
    vdd->sprCnt++;                      /* Increment sample period request count. */
    vdd->spsCnt += sectors;             /* Increment sample period sector count. */
    vdd->qd++;                          /* Bump queue depth. */

    if (function == VRP_OUTPUT || function == VRP_OUTPUT_VERIFY ||
        function == VRP_REBUILD_CHECK)
    {
        vdd->wReq++;                    /* Update write request count. */
    }
    else
    {
        vdd->rReq++;                    /* Update read request count. */
    }
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

    /* Set up fields in primary ILT/VRP to manage operation. */
    ilt->ilt_normal.w1 = 0;             /* Clear pending count fields. */
    ilt->ilt_normal.w5 = 0;             /* Clear composite status field. */
    ilt->ilt_normal.w6 = 0;             /* Clear CWIP record address. */
    ilt->ilt_normal.w7 = 0;             /* Clear assoc. VDD field. */

    /* If output or output/verify. */
    if (VRP_OUTPUT == function || VRP_OUTPUT_VERIFY == function)
    {
        int     flag_do_new_link = 0;   /* Link ILT/VRP to assoc VDD. */

        /* Virtual request is a WRITE type command. */
        /* Determine if this VRP might be caused by copy operations being wrapped around the axle. */
        if (vdd->pOutHead != NULL)      /* If ILT/VRPs on list. */
        {
            /* Check how many queued ILT/VRPs match the inbound ILT/VRP. */
            if (VIO_MAX <= v_io_match_cnt(vrp, sectors, vdd))
            {
                /* We have detected copy operations being wrapped around the
                   axle. Determine the proper handling to place the copy operation
                   associated with the specified DCD in a suspended state to
                   break the chain before it locks up the MAGNITUDE. Then return
                   the ILT/VRP request with an error indicating the axle has
                   been wrapped. */
                if (vdd->pDCD != 0)
                {
                    struct COR *cor = vdd->pDCD->cor;

                    /* Log Copy Message to CCB for Debug. */
                    CM_Log_Completion(CC_CMSPND, vdd->pDCD->cor);

                    if (cor->cm != 0)
                    {
                        /* Local COR processing to suspend the copy operation
                           associated with this virtual device being a
                           destination copy device. */
                        ILT *new_ilt = get_ilt();   /* Allocate an ILT (PCP) */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, new_ilt);
#endif /* M4_DEBUG_ILT */
                        struct PCP1 *pcp1 = (struct PCP1 *)new_ilt;
                        pcp1->cm = cor->cm;                     /* Save CM address. */
                        pcp1->cr = (void *)&ct_D$ctlrqst_cr;    /* Completion routine. */
                        pcp1->pcb = cor->cm->pcb;               /* Save Task PCB. */
                        pcp1->rtstate = 0;                      /* Clear requested task state. */

                        new_ilt++;                              /* PCP level 2. */
                        struct PCP2 *pcp2 = (struct PCP2 *)new_ilt;
                        pcp2->status = 0;                       /* Clear out status,function. */
                        pcp2->handler = 0;                      /* Clear Vsync handerl script. */
                        pcp2->function = PCPFC_PAUSE;           /* Save PCP function code. */

                        /* Transfer COR information into pcp2. */
                        pcp2->rid = cor->rid;
                        pcp2->rcsn = cor->rcsn;
                        pcp2->rcscl = cor->rcscl;
                        pcp2->rcsvd = cor->rcsvd;
                        pcp2->rcdcl = cor->rcdcl;
                        pcp2->rcdvd = cor->rcdvd;
                        pcp2->rssn = cor->rssn;
                        pcp2->rdsn = cor->rdsn;
                        pcp2->rscl = cor->rscl;
                        pcp2->rsvd = cor->rsvd;
                        pcp2->rdcl = cor->rdcl;
                        pcp2->rdvd = cor->rdvd;

                        pcp2->cor = cor;                        /* Save COR address. */
                        CM$ctlrqstq(new_ilt);                   /* Enqueue the control request. */
                    }
                    else
                    {
                    }
                }
                /* Remote COR processing to suspend the copy operation
                 * associated with this virtual device being a destination
                 * copy device. Place the appropriate error code in the
                 * ILT/VRP and return it to the requestor. */
                V_orc--;                /* Adjust outstanding request count. */
                vdd->qd--;              /* Adjust queue depth. */
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                vrp->status = EC_DATA_FAULT;    /* Save error status in VRP for requestor. */
                record_virtual(FR_VRP_COMPLETE, vrp);
                K_comp(ilt);            /* Return ILT/VRP back to requestor. */
                return;
            }
        }
        else
        {
            /* Determine if this ILT/VRP needs to be queued to the associated
             * VDD and queue it if needed. */
            if (vdd->pDCD == 0)
            {
                flag_do_new_link = 1;   /* Not necessary to queue ILT/VRP to assoc. VDD */
            }
        }

        if (flag_do_new_link == 0)
        {
            /* Queue inbound ILT/VRP to VDD. */
            ilt->ilt_normal.w7 = (UINT32)vdd;   /* Save assoc VDD address. */

            ILT *back_ptr;                  /* Determine tail member. */
            if (vdd->pOutTail != 0)         /* If tail member already exists. */
            {
                back_ptr = vdd->pOutTail;
            }
            else
            {
                back_ptr = (ILT *)&vdd->pOutHead;   /* Pseudo tail member. */
            }
            ilt->fthd = 0;                  /* Clear forward thread field in ILT. */
            vdd->pOutTail = ilt;            /* Save ILT/VRP as new tail member. */
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            back_ptr->fthd = ilt;           /* Link ILT/VRP onto previous tail member. */
            ilt->bthd = back_ptr;           /* Establish backward thread. */
        }

        /* Check if any copy operations associated with this virtual device. */
        SCD *scd = vdd->pSCDHead;
        int save_nvram = 0;
        int flag;

        while (scd != 0)
        {
            /* Make sure there is a p2handler. */
            if (scd->p2handler != 0)
            {
                /* Call copy op. update source handler routine. */
                flag = v_callx(scd->p2handler, 0, 0, scd, strategy, SN_addr, sectors, vrp, vdd, sgl, ilt);
// Output:
//   flag = TRUE/FALSE indicator if need to save NVRAM
//   g1 = secondary update ILT/VRP if write needs to be written to another virtual device
//   g1 = 0 if no secondary ILT/VRP built
// g1 input and output. Deal with it.
                save_nvram |= flag;
                /* Check for secondary ILT/VRP specified. */
                if (g1 != 0)
                {
                    ((ILT*)g1)->fthd = link_ilt;    /* Link ILT/VRP onto secondary op. list. */
                    link_ilt = (ILT*)g1;            /* Save for next operation. */
                    secondary_ops++;                /* Increment secondary op counter. */
                }
            }
            scd = scd->link;                        /* Link to next SCD on list. */
        }

        /* Check for any copy operations that use this device as a Destination Copy Device. */
        /* See if destination copy device is a virtual disk, and has a handler. */
        if (vdd->pDCD != 0 && vdd->pDCD->p2handler != 0)
        {
            /* Call copy op. update handler routine. */
            flag = v_callx(vdd->pDCD->p2handler, 0, 0, vdd->pDCD, strategy, SN_addr, sectors, vrp, vdd, sgl, ilt);
// Output:
//   flag = TRUE/FALSE indicator if need to save NVRAM
//   g1 = secondary update ILT/VRP if write needs to be written to another virtual device
//   g1 = 0 if no secondary ILT/VRP built
// g1 input and output. Deal with it.
            save_nvram |= flag;
            /* Check for secondary ILT/VRP specified. */
            if (g1 != 0)
            {
                ((ILT*)g1)->fthd = link_ilt;    /* ink ILT/VRP onto secondary op. list. */
                secondary_ops++;                /* Increment secondary op counter. */
                link_ilt = (ILT*)g1;            /* Save for next operation. */
            }
        }
        if (link_ilt != 0)
        {
            ilt->ilt_normal.w1 = secondary_ops; /* save secondary copy op. count in primary ILT/VRP */
// Note: TEMPORARY approach for phase 1
//      This routine increments a CWIP record counter and saves a non-zero
//      CWIP record address in the primary ILT to identify that this counter
//      needs to be decremented when the primary I/O completes.
// END TEMPORARY
            ilt->ilt_normal.w6 = CCSM$get_cwip();   /* Get CWIP record count address. */
        }
    }
    strategy = strategy + (RRP_INPUT - 1);      /*  Modify function for RRP. */

    /* Check if ILT/VRP queued to VDD. If so, else condition to generate separate ILF/RRP. */
    /* If multiple RRPs required, do else condition also. */
    if ((VDD *)ilt->ilt_normal.w7 == 0 && remaining_sectors == sectors)
    {
        if (secondary_ops == 0)
        {
            /* Generate/submit single RRP. */
            v_gensrrp(strategy, remaining_sectors, segment_sector_offset, rdd, sectors, vdd, sgl, ilt);
            return;
        }
        /* Generate and submit RRP for processing. */
        v_gensscrrp(strategy, remaining_sectors, segment_sector_offset, rdd, sectors, vdd, sgl, ilt);
    }
    else
    {
        UINT32 byte_offset = 0;                 /* Clear SGL byte offset counter. */

        /* Generate/submit multiple RRPs. */
        ilt->ilt_normal.w4 = (UINT32)vdd;       /* Save VDD in primary ILT. */
        for (;;)
        {
            /* Generate and submit RRP for processing. */
            v_genmrrp(strategy, remaining_sectors, segment_sector_offset, rdd, byte_offset, sectors, vdd, sgl, ilt);
            sectors = sectors - remaining_sectors;  /* Adjust remaining sector count. */
            if (sectors == 0)                   /* Exit if complete. */
            {
                break;
            }
            rdd = rdd->pNRDD;                   /* Link to next RAID segment. */
            segment_sector_offset = 0;          /* Beginning of raid for next sector offset. */

            /* Update byte offset into SGL. */
            byte_offset = byte_offset + (remaining_sectors << 9);
            /* Note: We know that the I/O request is small in size. */
            remaining_sectors = (sectors >= rdd->devCap) ? rdd->devCap : sectors;
        }
        if (secondary_ops == 0)                 /* If no secondary ops. associated with this VRP. */
        {
            return;
        }
    }
    /* Process secondary ops. */
    while (link_ilt != 0)
    {
        ilt = link_ilt + 1;                     /* Next secondary op. to issue at next nest level. */
        link_ilt = link_ilt->fthd;              /* Remove ILT from list. */
// il_w0 = VRP
// il_w2 = Session Node pointer if != 0
        V$xque(ilt);
    }
}   /* End of v_exec_2 */

/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
