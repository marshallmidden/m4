/* $Id: raid5.c 156019 2011-05-27 16:09:41Z m4 $ */
/**
******************************************************************************
**
**  @file       raid.c
**
**  @brief      Raid support routines written in c.
**
**  Copyright (c) 2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include "XIO_Std.h"
#include "CT_defines.h"
#include "prp.h"
#include "rrp.h"
#include "sgl.h"
#include "r5s.h"
#include "RL_RDD.h"
#include "ecodes.h"
#include "mem_pool.h"

/* ------------------------------------------------------------------------ */
/* Routines used externally -- Function prototypes, until they turn static. */
void    r_initrrbr5s(RRB *rrb, RPN *rpn, R5S *r5s);
UINT32 r_ulrrb(RPN **ret_rpn, RRB *next_rrb);
void r_actrpn(int, RPN *);
void r_initread(RRB *rrb, RPN *rpn, R5S *r5s);

/* ------------------------------------------------------------------------ */
/* Completion routines defined externally. */
extern UINT32 ct_r$r5a6rcomp;   // Just need the address of the completion routine.
extern UINT32 ct_r$r5fdwcomp;   // Just need the address of the completion routine.
extern UINT32 ct_r$r5fpdwcomp;  // Just need the address of the completion routine.
extern UINT32 ct_r$r5fpwcomp;   // Just need the address of the completion routine.
extern UINT32 ct_r$r5sdpr4comp; // Just need the address of the completion routine.
extern UINT32 ct_r$r5sdr3comp;  // Just need the address of the completion routine.
extern UINT32 ct_r$r5srrcomp;   // Just need the address of the completion routine.
extern UINT32 ct_r$r5frrcomp;   // Just need the address of the completion routine.
extern UINT32 ct_r$r5drcomp;    // Just need the address of the completion routine.
extern UINT32 ct_r$r5prrcomp;   // Just need the address of the completion routine.

/* ------------------------------------------------------------------------ */
/* Routines defined externally. */
extern SGL *m_asglbuf(UINT32);
extern void r$sxorsgls(RRB *);
extern UINT32 r$r5a6msglrw(RRB *, UINT32);
extern UINT32 r$r5a6msglwr(RRB *, UINT32);
extern UINT32 r$rrrb(RRB *);
extern UINT32 R$checkForR5PSDRebuilding(PSD *);

/* ------------------------------------------------------------------------ */
/* Queue and queue pointers defined externally. */
extern QU *P_que;               /* Points to the physical queue. */
extern QU R_r5exec_qu;          /* Raid 5 executive queue. */

/* ------------------------------------------------------------------------ */
/* Routines used externally. */
UINT32 r_initwrite(RRB *rrb, RPN *rpn, R5S *r5s);
void r_insrrb(RRB *rrb, UINT64 mso, UINT64 ms, UINT32 function, RDD *rdd,
              UINT32 sn, RRP *rrp UNUSED, ILT *ilt);

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_ulrrb
**
**  @brief  Unlink a specific RRB from its associated RPN.
**
**  The starting PSD, parity PSD and wrap PSD are located and stored into the
**  RPN for the specified maxi stripe.
**
// Input g2 = RRB.
**  @param  next_rrb - Pointer to RRB.
**
// Output g1 = TRUE if RPN empty.
// Output g3 = RPN.
**  @return TRUE if RPN empty.
**  @return ret_rpn = RPN.
**
******************************************************************************
**/

UINT32 r_ulrrb(RPN ** ret_rpn, RRB *next_rrb)
{
    RRB        *fthd;
    RRB        *bthd;

    /* Prepare to remove RRB from RPN. */
    fthd = next_rrb->fthd;      /* Dequeue candidate RRB from RPN. */
    bthd = next_rrb->bthd;      /* Dequeue candidate RRB from RPN. */
    *ret_rpn = next_rrb->rpn;   /* Get corresponding RPN. */

    /* Clear links. */
    next_rrb->fthd = 0;
    next_rrb->bthd = 0;

    if (fthd == bthd)
    {
        /* Dequeue single RRB from queue. Clear RRB threads in RPN. */
        (*ret_rpn)->rrbhead = fthd;
        (*ret_rpn)->rrbtail = bthd;
        return (TRUE);          /* Indicate empty RPN. */
    }

    if (fthd == 0)
    {
        /* Dequeue RRB from queue tail. */
        (*ret_rpn)->rrbtail = bthd;     /* Set up new queue tail. */
        bthd->fthd = fthd;              /* Close link last RRB. */
    }
    else if (bthd == 0)
    {
        /* Dequeue RRB from queue head. */
        (*ret_rpn)->rrbhead = fthd;     /* Set up new queue head. */
        fthd->bthd = bthd;              /* Close link with 1st RRB. */
    }
    else
    {
        /* Dequeue RRB from mid-queue. */
        fthd->bthd = bthd;
        bthd->fthd = fthd;
    }
    return (FALSE);             /* Indicate RPN not empty. */
}                               /* End of r_ulrrb */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_joinn2n
**
**  @brief  Join a non-system RRB with a non-system RRB.
**
**  A new system RRB is created from the two non-system RRBs. The non-system
**  RRBs are used to create the join thread and a new SGL is created for the
**  system RRB.
**
// Input g1 = older non-system RRB
// Input g2 = newer non-system RRB
// Output g2 = new system RRB
**  @param  older_rrb   - older non-system RRB.
**  @param  newer_rrb   - newer non-system RRB.
**
**  @return new system RRB.
**
******************************************************************************
**/

static RRB *r_joinn2n(RRB *older_rrb, RRB *newer_rrb)
{
    SGL        *sgl1;
    SGL        *sgl2;
    RRB        *new_rrb = get_rrb();    /* Assign new system RRB. */

#ifdef M4_DEBUG_RRB
    CT_history_printf("%s%s:%u get_rrb 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__,
                      new_rrb);
#endif /* M4_DEBUG_RRB */

    new_rrb->flags = older_rrb->flags | newer_rrb->flags;

    /* Create join thread. */
    new_rrb->jointhd = older_rrb;
    newer_rrb->fthd = 0;
    older_rrb->fthd = newer_rrb;

    /* Set up logical disk addresses. */
    if (older_rrb->lsda == newer_rrb->leda)
    {
        /* Process backward join. */
        sgl1 = newer_rrb->sgl;
        sgl2 = older_rrb->sgl;
        new_rrb->lsda = newer_rrb->lsda;
        new_rrb->leda = older_rrb->leda;
    }
    else
    {
        /* Process forward join. */
        sgl1 = older_rrb->sgl;
        sgl2 = newer_rrb->sgl;
        new_rrb->lsda = older_rrb->lsda;
        new_rrb->leda = newer_rrb->leda;
    }
    /* Merge SGLs, and set up new SGL. */
    new_rrb->sgl = PM_MergeSGL(&new_rrb->sglsize, sgl1, sgl2);

    return (new_rrb);
}                               /* End of r_joinn2n */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_joins2n
**
**  @brief  Join a system RRB with a non-system RRB.
**
**  The surviving RRB receives the addition of the non-system RRB to the join
**  thread. The surviving RRB is updated to reflect the concatenation. Both
**  SGLs are merged into a single new SGL.
**
// Input g1 = non-system RRB
// Input g2 = surviving system RRB
**  @param  non_system_rrb  - non-system RRB.
**  @param  surviving_rrb   - surviving system RRB.
**
**  @return none.
**
******************************************************************************
**/

static void r_joins2n(RRB *non_system_rrb, RRB *surviving_rrb)
{
    SGL        *sgl1;
    SGL        *sgl2;
    RRB        *s_jointhd;
    SGL        *s_sgl;
    UINT32      s_sglsize;

    s_jointhd = surviving_rrb->jointhd;
    s_sgl = surviving_rrb->sgl;
    s_sglsize = surviving_rrb->sglsize;

    /* Add RRB to join thread. */
    surviving_rrb->jointhd = non_system_rrb;
    non_system_rrb->fthd = s_jointhd;

    if (surviving_rrb->lsda == non_system_rrb->leda)
    {
        /* Process forward join. */
        sgl1 = non_system_rrb->sgl;
        sgl2 = s_sgl;
        surviving_rrb->lsda = non_system_rrb->lsda;
    }
    else
    {
        /* Process backward join. */
        sgl1 = s_sgl;
        sgl2 = non_system_rrb->sgl;
        surviving_rrb->leda = non_system_rrb->leda;
    }

    /* Merge SLGs, and set up new SGL. */
    surviving_rrb->sgl = PM_MergeSGL(&surviving_rrb->sglsize, sgl1, sgl2);

    /* Release old SGL if appropriate. */
    if (BIT_TEST(s_sglsize, 31) == 0)      /* If not borrowed SGL. */
    {
        /* Release SGL memory. */
        s_Free(s_sgl, s_sglsize, __FILE__, __LINE__);
    }
}                               /* End of r_joins2n */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_joins2s
**
**  @brief  Joing a system RRB with another system RRB.
**
**  The surviving RRB receives the concatenation of both join threads. The
**  surviving RRB is updated to reflect the concatenation of both RRBs. Both
**  SGLs are merged into a single SGL. The old SGLs are released back to the
**  system. The non-surviving RRB is released back to the system.
**
// Input g1 = non-surviving system RRB
// Input g2 = surviving system RRB
**  @param  expedited   - TRUE if we are expediting.
**  @param  rpn         - Pointer to RPN
**
**  @return none.
**
******************************************************************************
**/

static void r_joins2s(RRB *non_surviving_rrb, RRB *surviving_rrb)
{
    RRB        *prev_rrb;
    RRB        *next_rrb;
    SGL        *sgl1;
    SGL        *sgl2;
    SGL        *s_sgl;
    UINT32      s_sglsize;
    SGL        *n_sgl;
    UINT32      n_sglsize;

    next_rrb = surviving_rrb->jointhd;  /* Origin of join thread. */

    /* Get last valid RRB. */
    for (;;)
    {
        prev_rrb = next_rrb;
        next_rrb = next_rrb->fthd;      /* Get next RRB. */
        if (next_rrb == 0)
        {
            break;
        }
    }

    /* Merge join threads. */
    next_rrb = non_surviving_rrb->jointhd;
    prev_rrb->fthd = next_rrb;

    /* Save original SGL and size. */
    s_sgl = surviving_rrb->sgl;
    s_sglsize = surviving_rrb->sglsize;
    n_sgl = non_surviving_rrb->sgl;
    n_sglsize = non_surviving_rrb->sglsize;

    /* Set up logical disk addresses. */
    if (surviving_rrb->lsda == non_surviving_rrb->leda)
    {
        /* Process forward join, new LSDA is non-surviving LSDA. */
        surviving_rrb->lsda = non_surviving_rrb->lsda;
        sgl1 = n_sgl;                   /* Pass 1st SGL. */
        sgl2 = s_sgl;                   /* Pass 2nd SGL. */
    }
    else
    {
        /* Process backward join, new LEDA is non-surviving LSDA. */
        surviving_rrb->leda = non_surviving_rrb->leda;
        sgl1 = s_sgl;                   /* Pass 1st SGL. */
        sgl2 = n_sgl;                   /* Pass 2nd SGL. */
    }

    /* Merge SLGs, and set up new SGL. */
    surviving_rrb->sgl = PM_MergeSGL(&surviving_rrb->sglsize, sgl1, sgl2);

    /* Release old SGLs if appropriate. */
    if (BIT_TEST(s_sglsize, 31) == 0)   /* If not borrowed SGL. */
    {
        /* Release SGL memory. */
        s_Free(s_sgl, s_sglsize, __FILE__, __LINE__);
    }
    if (BIT_TEST(n_sglsize, 31) == 0)   /* If not borrowed SGL. */
    {
        /* Release SGL memory. */
        s_Free(n_sgl, n_sglsize, __FILE__, __LINE__);
    }

    /* Release non-surviving RRB. */
#ifdef M4_DEBUG_RRB
    CT_history_printf("%s%s:%u put_rrb 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__,
                      non_surviving_rrb);
#endif /* M4_DEBUG_RRB */
    put_rrb(non_surviving_rrb);
}                               /* End of r_joins2s */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_preprpn
**
**  @brief  Initialize the RPN with PSD information for specified maxi stripe.
**
**  The starting PSD, parity PSD and wrap PSD are located and stored into the
**  RPN for the specified maxi stripe.
**
// Input g3  = RPN
// Input g5  = maxi stripe
// Input g11 = RDD
**  @param  rpn - Pointer to RPN.
**  @param  ms  - Maxi stripe.
**  @param  rdd - Pointer to RDD
**
**  @return none.
**
******************************************************************************
**/

static void r_preprpn(RPN *rpn, UINT32 ms, RDD *rdd)
{
    UINT32      line;
    UINT32      line_offset;
    UINT32      temp;
    PSD        *first_psd;
    PSD        *parity_psd;
    PSD        *wrap_psd;

    rpn->stripe = ms;           /* Record stripe. */

    /* Calculate line and line offset. */
    temp = ms * rdd->depth;
    line = temp / rdd->psdCnt;
    line_offset = temp % rdd->psdCnt;

    /* Calculate PSDA offset for start of maxi-stripe. */
    rpn->spsda = (UINT64)rdd->sps * line;

    /* Lookup 1st PSD of maxi-stripe and PSD of parity. */
    first_psd = rdd->extension.pPSD[line_offset];
    rpn->rdd = rdd;             /* Set up RDD. */
    if ((rdd->psdCnt % rdd->depth) == 0)
    {
        /* maxi-strip is evenly divisiable. */
        /* Calculate parity device offset within maxi-stripe based upon line/4. */
        /* Adjust parity device offset by maxi-stripe line offset. */
        temp = ((line / 4) % rdd->depth) + line_offset;
        if (temp >= rdd->psdCnt)
        {
            /* Adjust parity PSD offset into next line. */
            temp = temp - rdd->psdCnt;
        }
        parity_psd = rdd->extension.pPSD[temp];
    }
    else
    {
        parity_psd = first_psd; /* Use 1st PSD of maxi-stripe for parity. */
    }

    /* Clear wrap PSD if start and wrap PSD equivalence. */
    wrap_psd = rdd->extension.pPSD[0];
    wrap_psd = (first_psd == wrap_psd) ? 0 : wrap_psd;

    /* Record striping and PSD information. */
    rpn->spsd = first_psd;
    rpn->ppsd = parity_psd;
    rpn->wpsd = wrap_psd;
    /* Calculate base LSDA. */
    rpn->lsda = (UINT64)ms *rdd->spu;

    /* Clear lock and active fields. */
    rpn->lock = 0;
    rpn->act = 0;
}                               /* End of r_preprpn */

/* ------------------------------------------------------------------------ */

/* --- Algorithm 1 -----------------------------------------------------
 *
 *  Process full stripe write w/ good data and parity drives
 *
 *  In this type of operation, all of the drives in the stripe have a full
 *  stripe worth of data to be written. In this case, it is faster to take all
 *  of the data blocks, xor them together, and write this data out as the
 *  parity than it is to calculate the deltas to the parity. Therefore, we
 *  will start up all of the writes except for the parity write and will start
 *  the xor operation all at once. When the parity operation is complete, the
 *  parity data will be written out. The completion routines will release the
 *  data buffers once the xor operation or the writes have been completed.
 *  Note that the buffers are required until both the parity operation (xor)
 *  and the write are complete.
 *
 *  All devices that are part of this stripe are currently defined as
 *  operational. The writes for each data drive will be initiated, the parity
 *  will be calculated after which the parity will be written to the parity
 *  drive.
 */

static void raid_Algorithm_1(RRB *rrb, RPN *rpn, R5S *r5s)
{
    PSD *working_PSD;
    UINT32 sgl_ptr_offset;
    UINT32 outstanding_request_count;
    UINT64 obpsda;
    ILT *new_ilt;
    PRP *new_prp;
    SGL *new_sgl;
    ILT *save_ilt = 0;

    working_PSD = r5s->spsd;                    /* Start with current PSD. */
    sgl_ptr_offset = 0;                         /* Clear RRB SGL ptr offset. */
    outstanding_request_count = r5s->depth;     /* Get stripe width (3, 5 or 9). */
    rrb->orc = outstanding_request_count;       /* Set up outstanding request count. */
    obpsda = r5s->obpsda;                       /* Get stripe base PSDA. */

    /* Process next device in stripe */
    for (;;)
    {
        /* Generate ILT/PRP. */
        new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
        CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
        new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
        CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
        new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

        new_ilt->ilt_normal.w3 = (UINT32)rrb;            /* Set up RRB. */
        new_ilt->ilt_normal.w4 = (UINT32)working_PSD;    /* Set up RRB. */

        /* Build the PRP. */
        r_bldprp(new_prp, working_PSD, obpsda, r5s->sps, r5s->function, r5s->strategy);

        if (rpn->ppsd == working_PSD)
        {
            /* Process parity device. */
            save_ilt = new_ilt;
            rrb->uu.pwilt = new_ilt;            /* Record parity ILT. */

            new_sgl = m_asglbuf(r5s->sps << 9); /* Allocate parity buffer. */

            rrb->sglarray[0] = new_sgl;         /* Set up destination SGL in RRB. */
            new_prp->pSGL = new_sgl;
            BIT_SET(new_prp->sglSize, 31);      /* Set as borrowed SGL. */

            /* An SGL buffer is allocated for storing the XORed data, with the call m_asglbuf
             * This is linked to destination SGL pointer in RRB and SGL pointer in PRP
             * created when ILT/RRP are allocated.
             */

//     rrb->rbsgl0   = pRRB->pDstSGL = pSGL (new_sgl)
//     new_prp->pr_sglptr = pPRP->pSGL    = pSGL
// pPRP is already linked to pNewILT (i.e. parity write ILT)

            /* The SGL descriptor count in this case always one. The size of the buffer created
             * is (pR5S->sps)*512 bytes i.e. sectors/stripe
             */
        }
        else
        {
            /* Process data device, generate new SGL for this request. */
            new_sgl = m_gensgl(r5s->sps, r5s->bo, rrb->sgl);

            sgl_ptr_offset++;                           /* Bump for next RRB SGL. */
            rrb->sglarray[sgl_ptr_offset] = new_sgl;    /* Set up next src SGL in RRB. */
            new_prp->pSGL = new_sgl;            /* Link SGL to PRP. */
            BIT_SET(new_prp->sglSize, 31);      /* Set as borrowed SGL. */
            r5s->bo += (r5s->sps << 9);         /* Adjust SGL byte offset. */

            /* Queue physical request (data) without wait. */
            EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r5fdwcomp);
        }

        /* For each of the data device request (as many devices depends on depth 2,4,8)
         * A new SGL is generated from original SGL buffer and is linked to each source
         * SGL in RRB.
         */

// pRRB->pSrc1SGL  - Points to SGL buffer related to Ist data device request in the stripe.
// pRRB->pSrc2SGL  - Points to SGL buffer related to 2nd data device request in stripe.
// The size of each request is (pR5S->sps*512), and each request is queued to Physical layer.

        /* Advance to next device. */
        outstanding_request_count--;        /* Adjust remaining device count. */
        if (outstanding_request_count == 0)
        {
            break;                          /* Exit if done. */
        }
        working_PSD = working_PSD->npsd;    /* Advance to next PSD. */
        if (rpn->wpsd == working_PSD)
        {
            obpsda += r5s->sps;             /* Adjust base PSDA. */
        }
    }

    /* Set SGL terminator. */
    rrb->sglarray[sgl_ptr_offset + 1] = 0;

    /* Now XOR the data buffers stored in source SGL pointers of RRB. And the result
     * gets stored in the buffer pointed to by destination SGL of RRB. The same buff
     * is already linked to PRP of parity write ILT (stored in save_ilt(pNewILT) )
     */
    r$sxorsgls(rrb);                    /* XOR the SGLs. */

    /* Queue Physical request (parity) without wait. */
    EnqueueILT((void *)P_que, save_ilt, (void *)&ct_r$r5fpwcomp);
}   /* End of raid_Algorithm_1 */

/* ------------------------------------------------------------------------ */

/* --- Algorithm 2 -----------------------------------------------------
 *
 *  Process full stripe write w/ bad data drive
 *
 *  In this algorithm, we have all of the data for a full stripe just as we
 *  did in algorithm one. The main difference here is that the one data drive
 *  is not operational. In this case, all we do is process it exactly as in
 *  algorithm one but skip one write operation. The same care must be taken in
 *  the xor and write data protection.
 *
 *  All but one data device and the parity device that are a part of this
 *  stripe are currently defined as operational. The writes for each data
 *  drive (except for the failed data drive) will be initiated, the parity
 *  will be calculated after which the parity will be written to parity drive.
 */

static void raid_Algorithm_2(RRB *rrb, RPN *rpn, R5S *r5s, PSD *failing_psd)
{
    UINT64 sda;
    PSD *working_PSD;
    UINT32 sgl_ptr_offset;
    SGL *new_sgl;
    ILT *new_ilt;
    PRP *new_prp;
    ILT *save_ilt = 0;

    sda = r5s->obpsda;                      /* Get stripe base PSDA. */
    working_PSD = r5s->spsd;                /* Get starting PSD. */
    sgl_ptr_offset = 0;                     /* Clear RRB SGL ptr offset. */

    /* Process next device in stripe. */
    for (;;)
    {
        if (working_PSD == failing_psd)     /* If failed device. */
        {
            /* Process bad data device, generate new SGL for this request. */
            new_sgl = m_gensgl(r5s->sps, r5s->bo, rrb->sgl);

            rrb->sglarray[sgl_ptr_offset + 1] = new_sgl;    /* Set up next src SGL in RRB. */
            sgl_ptr_offset++;               /* Bump for next RRB SGL. */
            r5s->bo += (r5s->sps << 9);     /* Adjust SGL byte offset. */
        }
        else
        {
            rrb->orc++;                     /* Bump outstanding request count. */

            /* Generate ILT/PRP. */
            new_ilt = get_ilt();            /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
            CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
            new_prp = get_prp();            /* Assign PRP. */
#ifdef M4_DEBUG_PRP
            CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
            new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

            new_ilt->ilt_normal.w3 = (UINT32)rrb;   /* Set up RRB. */
            new_ilt->ilt_normal.w4 = (UINT32)working_PSD;   /* Set up PSD. */

            /* Build the PRP. */
            r_bldprp(new_prp, working_PSD, sda, r5s->sps, r5s->function, r5s->strategy);

            if (working_PSD == rpn->ppsd)   /* If not data device. */
            {
                /* Process parity device. */
                save_ilt = new_ilt;         /* Save parity ILT. */
                rrb->uu.pwilt = new_ilt;       /* Record parity write ILT. */

                new_sgl = m_asglbuf(r5s->sps << 9);

                rrb->sglarray[0] = new_sgl; /* Set up dst SGL in RRB. */
                new_prp->pSGL = new_sgl;
                new_prp->sglSize = new_sgl->size | (1 << 31);    /* Set as borrowed. */
            }
            else
            {
                /* Process good data device, Generate new SGL for this request. */
                new_sgl = m_gensgl(r5s->sps, r5s->bo, rrb->sgl);

                /* Set up next src SGL in RRB and new PRP. */
                rrb->sglarray[sgl_ptr_offset + 1] = new_sgl;
                new_prp->pSGL = new_sgl;
                new_prp->sglSize = new_sgl->size | (1 << 31);   /* Set as borrowed. */

                /* Queue physical request (data) without wait. */
                EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r5fdwcomp);

                sgl_ptr_offset++;           /* Bump for next RRB SGL. */
                r5s->bo += (r5s->sps << 9); /* Adjust SGL byte offset. */
            }
        }

        /* Advance to next device. */
        if (r5s->rsc == 0)                  /* If complete, exit loop. */
        {
            break;
        }
        working_PSD = working_PSD->npsd;    /* Advance to next PSD. */
        if (working_PSD == rpn->wpsd)       /* If wrapping PSD. */
        {
            sda += r5s->sps;                /* Adjust base PSDA. */
        }
        r5s->rsc = r5s->rsc - r5s->sps;     /* Update remaining sector count. */
    }

    /* Set SGL terminator. */
    rrb->sglarray[sgl_ptr_offset + 1] = 0;  /* Set up SGL terminator. */

    /* Now XOR the data buffers stored in source SGL pointers of RRB. And the result
     * gets stored in the buffer pointed to by destination SGL of RRB. The same buff
     * is already linked to PRP of parity write ILT (stored in save_ilt(pNewILT) )
     * Here the XOR is being done for all the data SGLs, including bad device.
     */
    r$sxorsgls(rrb);                        /* XOR the SGLs. */

    /* Queue Physical request (parity) without wait. */
    EnqueueILT((void *)P_que, save_ilt, (void *)&ct_r$r5fpwcomp);
}   /* End of raid_Algorithm_2 */

/* ------------------------------------------------------------------------ */

/*  --- Algorithm 3 -----------------------------------------------------
 *
 *  Initiate single stripe write (1 read, 2 writes over 3 drives)
 *
 *  This algorithm is used exclusively for 3 drive wide stripes (R5/3) and is
 *  only used when all 3 devices are defined as operational. The data to be
 *  written is contained entirely on one data device.
 *
 *  The data is spread across the drives as follows:
 *      D0 - parity write (host data xor read data from D2).
 *      D1 - data write (host data).
 *      D2 - read data from remaining drive in stripe.
 */

static void raid_Algorithm_3(RRB *rrb, RPN *rpn, R5S *r5s)
{
    UINT64 sda;
    int device_count = 0;                   /* Clear processed device count. */
    PSD *working_PSD;
    SGL *new_sgl;
    ILT *new_ilt;
    PRP *new_prp;
    UINT32  tmp_SGL_size;
    ILT *reconstruction_ilt = 0;
    ILT *save_parity_ilt = NULL;
    ILT *save_data_ilt = NULL;

    sda = r5s->obpsda;                      /* Get stripe base PSDA. */
    working_PSD = r5s->spsd;                /* Get starting PSD. */
    rrb->sglarray[3] = 0;                   /* Set RRB SGL terminator. */

    /* Allocate reconstruction/parity buffer of remaining sector count. */
    new_sgl = m_asglbuf(r5s->rsc << 9);

    rrb->orc = 2;                           /* Set outstanding request count to 2. */

    /* Process next device in stripe. */
    for (;;)
    {
        /* Generate ILT/PRP. */
        new_ilt = get_ilt();                /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
        CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
        new_prp = get_prp();                /* Assign PRP. */
#ifdef M4_DEBUG_PRP
        CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
        new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

        new_ilt->ilt_normal.w3 = (UINT32)rrb;   /* Set up RRB. */
        new_ilt->ilt_normal.w4 = (UINT32)working_PSD;   /* Set up PSD. */
        if (r5s->cpsd == working_PSD)       /* If data device for write. */
        {
            /* Process data device for write. */
            new_ilt->ilt_normal.w5 = FALSE; /* Clear parity flag. */
            save_data_ilt = new_ilt;        /* Save data write ILT. */

            /* Build the PRP. */
            r_bldprp(new_prp, working_PSD, sda + r5s->cso, r5s->rsc, r5s->function, r5s->strategy);

            rrb->sglarray[1] = rrb->sgl;    /* Set up src SGL in RRB. */
            tmp_SGL_size = rrb->sgl->size;  /* Link SGL to PRP, to set borrowed flag. */
            new_prp->pSGL = rrb->sgl;       /* Link SGL to PRP. */
        }
        else if (working_PSD != rpn->ppsd)
        {
            /* Process data device for reconstruction. */
            reconstruction_ilt = new_ilt;   /* Save reconstruction read ILT. */

            /* Build the PRP. */
            r_bldprp(new_prp, working_PSD, sda + r5s->cso, r5s->rsc, RRP_INPUT, r5s->strategy);

            rrb->sglarray[2] = new_sgl;     /* Set up src SGL in RRB. */
            tmp_SGL_size = new_sgl->size;   /* Link SGL to PRP, to set borrowed flag. */
            new_prp->pSGL = new_sgl;        /* Link SGL to PRP. */
        }
        else
        {
            /* Process parity device. */
            new_ilt->ilt_normal.w5 = TRUE;  /* Set parity flag. */
            save_parity_ilt = new_ilt;      /* Save parity ILT. */
            rrb->uu.pwilt = new_ilt;        /* Save parity ILT. */

            /* Build the PRP. */
            r_bldprp(new_prp, working_PSD, sda + r5s->cso, r5s->rsc, r5s->function, r5s->strategy);

            tmp_SGL_size = new_sgl->size;   /* Link SGL to PRP, to set borrowed flag. */
            rrb->sglarray[0] = new_sgl;     /* Set up dst SGL in RRB. */
            new_prp->pSGL = new_sgl;        /* Link SGL to PRP. */
        }

        new_prp->sglSize = tmp_SGL_size | (1 << 31);    /* Indicate as borrowed. */

        /* Advance to next device. */
        device_count++;                     /* Adjust proceeded device count. */
        if (device_count == 3)
        {
            break;                          /* If complete. */
        }
        working_PSD = working_PSD->npsd;    /* Advance to next PSD. */
        if (working_PSD == rpn->wpsd)       /* If wrap PSD. */
        {
            sda += r5s->sps;                /* Adjust base PSDA. */
        }
    }

    /* Queue reconstruction read request. */
    reconstruction_ilt->ilt_normal.w5 = (UINT32)save_data_ilt;  /* Set up data write ILT. */
    reconstruction_ilt->ilt_normal.w6 = (UINT32)save_parity_ilt; /* Set up parity write ILT. */

    /* Queue reconstruction read request without wait. */
    EnqueueILT((void *)P_que, reconstruction_ilt, (void *)&ct_r$r5sdr3comp);
}   /* End of raid_Algorithm_3 */

/* ------------------------------------------------------------------------ */

/*  --- Algorithm 4 -----------------------------------------------------
 *
 *  Initiate single stripe write (2 reads, 2 writes over 2 drives)
 *
 *  This is the classic read old data, read old parity, write new data,
 *  generate masks, apply mask to old parity and write new parity operation.
 *
 *  This algorithm is used for 5 and 9 wide drive stripes when both the
 *  targeted data and parity drives are defined as operational and the data to
 *  be written is contained entirely on one device. This algorithm is also
 *  used for 3 drive wide stripes when the non-targeted data drive is defined
 *  as not operational.
 */

static void raid_Algorithm_4(RRB *rrb, RPN *rpn, R5S *r5s)
{
    PSD *working_PSD;
    SGL *new_sgl;
    ILT *new_ilt;
    PRP *new_prp;
    int device_count = 0;               /* Clear processed device count. */
    ILT *save_ilt;
    ILT *reconstruction_ilt;

    working_PSD = r5s->spsd;            /* Get starting PSD. */
    rrb->orc = 4;                       /* Set outstanding request count (2 reads + 2 writes). */
    rrb->sglarray[3] = rrb->sgl;        /* Set up src SGL in RRB. */
    rrb->sglarray[4] = 0;               /* Set RRB SGL terminator. */

    /* Preallocate parity ILT/PRP. */
    /* Generate ILT/PRP. */
    new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
    CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
    new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

    new_ilt->ilt_normal.w3 = (UINT32)rrb;   /* Set up RRB. */
    new_ilt->ilt_normal.w6 = (UINT32)new_ilt;       /* Link parity ILT to parity ILT. */
    save_ilt = new_ilt;                 /* Save parity ILT/PRP. */
    rrb->uu.pwilt = new_ilt;               /* Save parity ILT/PRP. */

    /* Preallocate data ILT/PRP. */
    new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
    CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
    new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

    new_ilt->ilt_normal.w3 = (UINT32)rrb;   /* Set up RRB. */
    new_ilt->ilt_normal.w5 = (UINT32)new_ilt;   /* Link data ILT to data ILT. */
    new_ilt->ilt_normal.w6 = (UINT32)save_ilt;       /* Link parity ILT to data ILT. */
    save_ilt->ilt_normal.w5 = (UINT32)new_ilt;      /* Link data ILT to parity ILT. */
    reconstruction_ilt = new_ilt;

    /* Process next device in stripe */
    for (;;)
    {
        int flag = 0;

        if (r5s->cpsd == working_PSD)       /* If selected data device for write. */
        {
            /* Process data device */
            new_prp = (PRP *)reconstruction_ilt->ilt_normal.w0;       /* Get data PRP. */
            new_ilt = reconstruction_ilt;   /* Get data ILT. */

            /* Build the PRP. */
            r_bldprp(new_prp, working_PSD, r5s->obpsda + r5s->cso, r5s->rsc, RRP_INPUT, r5s->strategy);

            new_sgl = m_asglbuf(r5s->rsc << 9); /* Allocate buffer for read. */

            rrb->sglarray[2] = new_sgl;     /* Set up src SGL in RRB. */
        }
        else if (working_PSD == rpn->ppsd)
        {
            /* Process parity device. */
            new_prp = (PRP *)save_ilt->ilt_normal.w0;       /* Get parity PRP. */
            new_ilt = save_ilt;                 /* Get parity ILT. */

            /* Build the PRP. */
            r_bldprp(new_prp, working_PSD, r5s->obpsda + r5s->cso, r5s->rsc, RRP_INPUT, r5s->strategy);

            new_sgl = m_asglbuf(r5s->rsc << 9); /* Allocate parity reconstruction buffer. */

            rrb->sglarray[0] = new_sgl;         /* Set up dst SGL in RRB (new parity) */
            rrb->sglarray[1] = new_sgl;         /* Set up src SGL in RRB (old parity) */
        }
        else
        {
            flag = 1;                           /* Flag nothing to process below. */
        }

        if (flag == 0)                      /* If did something above. */
        {
            new_ilt->ilt_normal.w4 = (UINT32)working_PSD;   /* Set up PSD. */
            new_prp->pSGL = new_sgl;        /* Link SGL to PRP. */
            new_prp->sglSize = new_sgl->size | (1 << 31);   /* Indicate SGL as borrowed. */

            device_count++;                 /* Adjust processed device count. */

            /* Queue physical request without wait. */
            EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r5sdpr4comp);
        }

        /* Advance to next device. */
        if (2 == device_count)              /* If complete */
        {
            return;
        }
        working_PSD = working_PSD->npsd;    /* Advance to next PSD. */
        if (rpn->wpsd == working_PSD)       /* If wrapping PSD. */
        {
            r5s->obpsda += r5s->sps;        /* Adjust base PSDA. */
        }
    }
}   /* End of raid_Algorithm_4 */

/* ------------------------------------------------------------------------ */

/*  --- Algorithm 5 -----------------------------------------------------
 *
 *  Initial sgl stripe write (n-2 reads and 1 write over n-1 drives)
 *
 *  This algorithm is used for 3, 5 or 9 wide drive stripes when the targeted
 *  data drive is defined as not operational. The data to be written would
 *  have been contained entirely on the non-operational device.
 *
 *  In this case, the remaining n-1 data drives are read and the host data
 *  and the n-1 data are xored to create the new parity data.
 */

static void raid_Algorithm_5(RRB *rrb, RPN *rpn, R5S *r5s, PSD *failing_psd)
{
    PSD *working_PSD;
    SGL *new_sgl;
    ILT *new_ilt;
    PRP *new_prp;
    ILT *save_ilt = 0;                  /* Clear save_ilt = parity write ILT. */
    ILT *reconstruction_ilt = 0;        /* Clear reconstructing ILT submistion thread. */
    UINT32  sgl_ptr_offset = 0;         /* Clear sgl_ptr_offset = RRB SGL ptr offset. */

    working_PSD = r5s->spsd;            /* Get starting PSD. */
    rrb->sglarray[1] = rrb->sgl;        /* Set up src SGL in RRB. */
    rrb->orc = r5s->depth - 1;          /* Set up outstanding request count. */

    /* Process next device in stripe. */
    for (;;)
    {
        if (working_PSD != failing_psd) /* If not failed device. */
        {
            /* Generate ILT/PRP. */
            new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
            CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
            new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
            CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
            new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

            new_ilt->ilt_normal.w3 = (UINT32)rrb;   /* Set up RRB. */
            new_ilt->ilt_normal.w4 = (UINT32)working_PSD;   /* Set up PSD. */

            if (working_PSD != rpn->ppsd)       /* If data device. */
            {
                /* Process good data device, Build the PRP. */
                r_bldprp(new_prp, working_PSD, r5s->obpsda + r5s->cso, r5s->rsc, RRP_INPUT, r5s->strategy);

                new_sgl = m_asglbuf(r5s->rsc << 9); /* Allocate buffer for recovery. */

                /* Link SGL to PRP. */
                rrb->sglarray[sgl_ptr_offset + 2] = new_sgl;    /* Set up next src SGL in RRB. */
                new_prp->pSGL = new_sgl;
                new_prp->sglSize = new_sgl->size | (1 << 31);   /* Set as borrowed. */

                sgl_ptr_offset++;               /* Bump for next RRB SGL. */

                /* Link physical request to reconstruction thread. */
                new_ilt->fthd = reconstruction_ilt; /* Link ILT to reconstruction. */
                reconstruction_ilt = new_ilt;   /* Save reconstruction ILT. */
            }
            else
            {
                /* Process parity device. */
                rrb->uu.pwilt = new_ilt;       /* Record parity ILT. */
                save_ilt = new_ilt;         /* Save parity ILT. */

                /* Build the PRP. */
                r_bldprp(new_prp, working_PSD, r5s->obpsda + r5s->cso, r5s->rsc, r5s->function, r5s->strategy);

                new_sgl = m_asglbuf(r5s->rsc << 9); /* Allocate buffer for parity. */
                new_prp->pSGL = new_sgl;
                rrb->sglarray[0] = new_sgl; /* Set up dst SGL in RRB. */
                new_prp->sglSize = new_sgl->size | (1 << 31);   /* Set as borrowed. */
            }
        }

        /* Advance to next device. */
        if (save_ilt != 0) {            /* If parity ILT defined. */
            if (r5s->depth == (sgl_ptr_offset + 2))
            {
                break;
            }
        }
        working_PSD = working_PSD->npsd;    /* Advance to next PSD. */
        if (rpn->wpsd == working_PSD)       /* If wrap PSD */
        {
            r5s->obpsda += r5s->sps;        /* Adjust base PSDA. */
        }
    }

    /* Set SGL terminator. */
    rrb->sglarray[sgl_ptr_offset + 2] = 0;    /* Set SGL terminator. */

    /* Queue all physical requests (reconstruction data). */
    for (;;)
    {
        new_ilt = reconstruction_ilt;                   /* Save current ILT. */
        reconstruction_ilt = reconstruction_ilt->fthd;  /* Get next ILT in thread. */
        new_ilt->ilt_normal.w6 = (UINT32)save_ilt;   /* Save parity write ILT. */

        /* Queue all physical requests (reconstruction data) without wait. */
        EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r5srrcomp);

        if (reconstruction_ilt == 0)    /* If no more reconstruction ILTs. */
        {
            break;
        }
    }
}   /* End of raid_Algorithm_5 */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   r_r5a6fread
**
**  @brief  Generate a full read ILT request for algorithm 6.
**
**  An ILT/PRP combination is generated for the full stripe read. The PRP is
**  initialized with information taken from the PSD and R5S. A full stripe
**  read buffer and SGL is allocated to contain the read data. The SGL is
**  linked into both the PRP and the RRB as the next source SGL. The ILT is
**  inserted at the head of the read submission queue.
**
**  @param  rrb     - Pointer to RRB
**  @param  psd     - Pointer to PSD
**  @param  sgl_cnt - Index to next SGL
**  @param  r5s     - Pointer to R5S
**
**  @return none
**
******************************************************************************
**/

static void r_r5a6fread(RRB *rrb, PSD *psd, UINT32 sgl_cnt, R5S *r5s)
{
    ILT *new_ilt;
    PRP *new_prp;
    SGL *new_sgl;

    /* Generate ILT/PRP. */
    new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
    CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
    new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

    /* Set up RRB in ilt. */
    new_ilt->ilt_normal.w3 = (UINT32)rrb;
    /* Set up PSD in ilt. */
    new_ilt->ilt_normal.w4 = (UINT32)psd;

    /* Build the PRP. */
    r_bldprp(new_prp, psd, r5s->bpsda, r5s->sps, RRP_INPUT, r5s->strategy);

    /* Allocate partial read buffer and SGL. */
    new_sgl = m_asglbuf(r5s->sps << 9);

    new_prp->pSGL = new_sgl;
    new_prp->sglSize = new_sgl->size | (1 << 31);   /* Flag as borrowed SGL. */
    rrb->sglarray[sgl_cnt+1] = new_sgl;

    /* Insert data read ILT into head of read submission queue. */
    new_ilt->fthd = rrb->rsq;
    rrb->rsq = new_ilt;
}   /* End of r_r5a6fread */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   r_r5a6parity
**
**  @brief  Generate the parity write ILT request for algorithm 6.
**
**  An ILT/PRP combination is generated for the full stripe parity write. The
**  ILT is recorded in the RRB as the parity write ILT. The PRP is initialized
**  with information taken from the PSD and R5S. A full stripe buffer and SGL
**  is allocated to contain the parity data. This SGL is linked into both the
**  PRP and the RRB as the parity SGL.
**
**  @param  rrb     - Pointer to RRB
**  @param  psd     - Pointer to PSD
**  @param  r5s     - Pointer to R5S
**
**  @return none
**
******************************************************************************
**/

static void r_r5a6parity(RRB *rrb, PSD *psd, R5S *r5s)
{
    ILT *new_ilt;
    PRP *new_prp;
    SGL *new_sgl;

    /* Generate ILT/PRP. */
    new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
    CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
    new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

    /* Set up RRB in ilt. */
    new_ilt->ilt_normal.w3 = (UINT32)rrb;
    /* Set up PSD in ilt. */
    new_ilt->ilt_normal.w4 = (UINT32)psd;
    /* Save parity write ILT. */
    rrb->uu.pwilt = new_ilt;

    /* Build the PRP. */
    r_bldprp(new_prp, psd, r5s->bpsda, r5s->sps, r5s->function, r5s->strategy);

    /* Allocate partial read buffer and SGL. */
    new_sgl = m_asglbuf(r5s->sps << 9);

    new_prp->pSGL = new_sgl;
    new_prp->sglSize = new_sgl->size | (1 << 31);   /* Flag as borrowed SGL. */
    /* Set up destination SGL in rrb. */
    rrb->sglarray[0] = new_sgl;
}   /* End of r_r5a6parity */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   r_r5a6pfread
**
**  @brief  Generate a partial fore read ILT request for algorithm 6.
**
**  An ILT/PRP combination is generated for the partial fore stripe read. The
**  PRP is initialized with information taken from the PSD and R5S. A partial
**  stripe read buffer and SGL is allocated to contain the read data. The ILT
**  is inserted at the head of the read submission queue.
**
**  @param  rrb      - Pointer to RRB
**  @param  psd      - Pointer to PSD
**  @param  r5s      - Pointer to R5S
**
**  @return none
**
******************************************************************************
**/

static void r_r5a6pfread(RRB *rrb, PSD *psd, R5S *r5s)
{
    ILT *new_ilt;
    PRP *new_prp;
    SGL *new_sgl;

    /* Generate ILT/PRP. */
    new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
    CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
    new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

    /* Set up RRB in ilt. */
    new_ilt->ilt_normal.w3 = (UINT32)rrb;
    /* Set up PSD in ilt. */
    new_ilt->ilt_normal.w4 = (UINT32)psd;

    /* Build the PRP. */
    r_bldprp(new_prp, psd, r5s->bpsda, r5s->cso, RRP_INPUT, r5s->strategy);

    /* Allocate partial read buffer and SGL. */
    new_sgl = m_asglbuf(r5s->cso << 9);

    new_prp->pSGL = new_sgl;
    new_prp->sglSize = new_sgl->size | (1 << 31);   /* Flag as borrowed SGL. */
    rrb->fmsgl = new_sgl;

    /* Insert data read ILT into head of read submission queue. */
    new_ilt->fthd = rrb->rsq;
    rrb->rsq = new_ilt;
}   /* End of r_r5a6pfread */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   r_r5a6fwrite
**
**  @brief  Generate a full data write ILT request for algorithm 6.
**
**  An ILT/PRP combination is generated for the full stripe write. The PRP is
**  initialized with information taken from the PSD and R5S. A partial stripe
**  SGL is allocated to contain the write data. The ILT is linked into the
**  head of the write ILT submission queue.
**
**  @param  rrb      - Pointer to RRB
**  @param  psd      - Pointer to PSD
**  @param  offset   - Byte offset into original SGL
**  @param  sgl_cnt  - Count of which source SGL we are working on (0 through 7).
**  @param  orig_sgl - Pointer to original SGL
**  @param  r5s      - Pointer to R5S
**
**  @return UINT32   - Updated byte offset into original SGL
**
******************************************************************************
**/

static UINT32 r_r5a6fwrite(RRB *rrb, PSD *psd, UINT32 offset, UINT32 sgl_cnt, SGL *orig_sgl, R5S *r5s)
{
    ILT *new_ilt;
    PRP *new_prp;
    SGL *new_sgl;

    /* Generate ILT/PRP. */
    new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
    CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
    new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

    /* Set up RRB in ilt. */
    new_ilt->ilt_normal.w3 = (UINT32)rrb;
    /* Set up PSD in ilt. */
    new_ilt->ilt_normal.w4 = (UINT32)psd;

    /* Build the PRP. */
    r_bldprp(new_prp, psd, r5s->bpsda, r5s->sps, r5s->function, r5s->strategy);

    /* Allocate SGL. */
    new_sgl = m_gensgl(r5s->sps, offset, orig_sgl);

    /* Link SGL to PRP. */
    new_prp->pSGL = new_sgl;
    new_prp->sglSize = new_sgl->size;
    /* Save into appropriate source sgl of RRB. */
    rrb->sglarray[sgl_cnt+1] = new_sgl;

    /* Insert data write ILT into head of write submission queue. */
    new_ilt->fthd = rrb->wsq;
    rrb->wsq = new_ilt;

    /* Update remaining sector count. */
    r5s->rsc = r5s->rsc - r5s->sps;

    /* Update byte offset into SGL. */
    r5s->bo = r5s->sps << 9;

    offset += (r5s->sps << 9);

    return(offset);
}   /* End of r_r5a6fwrite */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_r5a6pwrite
**
**  @brief  Generate a partial data write ILT request for algorithm 6.
**
**  An ILT/PRP combination is generated for the partial stripe write. The PRP
**  is initialized with information taken from the PSD and R5S. A partial
**  stripe SGL is allocated to contain the write data. The ILT is linked into
**  the head of the write ILT submission queue.
**
**  @param  rrb     - Pointer to RRB
**  @param  psd     - Pointer to PSD
**  @param  offset  - Byte offset into original SGL
**  @param  sgl     - Pointer to SGL
**  @param  r5s     - Pointer to R5S
**
**  @return UINT32  - Updated byte offset into original SGL
**
******************************************************************************
**/

static UINT32 r_r5a6pwrite(RRB *rrb, PSD *psd, UINT32 offset, SGL *sgl, R5S *r5s)
{
    ILT *new_ilt;
    PRP *new_prp;
    SGL *new_sgl;
    UINT64 sda;
    UINT32 sectors;

    /* Generate ILT/PRP. */
    new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
    CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
    new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

    /* Set up RRB in ilt. */
    new_ilt->ilt_normal.w3 = (UINT32)rrb;
    /* Set up PSD in ilt. */
    new_ilt->ilt_normal.w4 = (UINT32)psd;

    /* Starting disk address is current sector offset plus base SDA offset. */
    sda = r5s->bpsda + r5s->cso;
    sectors = (0 == r5s->cso) ? r5s->rsc : (r5s->sps - r5s->cso);

    /* Build the PRP. */
    r_bldprp(new_prp, psd, sda, sectors, r5s->function, r5s->strategy);

    /* Allocate SGL. */
    new_sgl = m_gensgl(sectors, offset, sgl);

    /* Link SGL to PRP. */
    new_prp->pSGL = new_sgl;
    new_prp->sglSize = new_sgl->size;

    /* Insert data write ILT into head of write submission queue. */
    new_ilt->fthd = rrb->wsq;
    rrb->wsq = new_ilt;

    /* Update remaining sector count. */
    r5s->rsc = r5s->rsc - sectors;

    /* Update byte offset into SGL. */
    r5s->bo = sectors << 9;

    offset += (sectors << 9);

    return(offset);
}   /* End of r_r5a6pwrite */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   r_r5a6paread
**
**  @brief  Generate a partial aft read ILT request for algorithm 6.
**
**  An ILT/PRP combination is generated for the partial aft stripe read. The
**  PRP is initialized with information taken from the PSD and R5S. A partial
**  stripe read buffer and SGL is allocated to contain the read data. The ILT
**  is inserted at the head of the read submission queue.
**
**  @param  rrb      - Pointer to RRB
**  @param  psd      - Pointer to PSD
**  @param  r5s      - Pointer to R5S
**
**  @return none
**
******************************************************************************
**/

static void r_r5a6paread(RRB *rrb, PSD *psd, R5S *r5s)
{
    ILT *new_ilt;
    PRP *new_prp;
    SGL *new_sgl;

    /* Generate ILT/PRP. */
    new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
    new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
    CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
    new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

    /* Set up RRB in ilt. */
    new_ilt->ilt_normal.w3 = (UINT32)rrb;
    /* Set up PSD in ilt. */
    new_ilt->ilt_normal.w4 = (UINT32)psd;

    /* Build the PRP. */
    r_bldprp(new_prp, psd, r5s->bpsda + r5s->cso, r5s->sps - r5s->cso, RRP_INPUT, r5s->strategy);

    /* Allocate partial read buffer and SGL. */
    new_sgl = m_asglbuf((r5s->sps - r5s->cso) << 9);

    new_prp->pSGL = new_sgl;
    rrb->arsgl = new_sgl;
    new_prp->sglSize = new_sgl->size | (1 << 31);   /* Flag as borrowed SGL. */

    /* Insert data read ILT into head of read submission queue. */
    new_ilt->fthd = rrb->rsq;
    rrb->rsq = new_ilt;
}   /* End of r_r5a6paread */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   r_initrrb
**
**  @brief  Sets up next split RRB utilizing the original RRB and parameters.
**
**  The current RRB is initialized with either passed parameters or information
**  taken from the original RRB. A new SGL is assigned, initialized and linked
**  into the current RRB. The return parameters are set up for the next call
**  to this routine.
**
**  @param  *offset     - current byte offset into original SGL.
**  @param  *sectors    - current sector count.
**  @param  *lsda       - current LSDA.
**  @param  *remaining  - total remaining sector count.
**  @param  rrb         - current RRB
**  @param  rpn         - RPN
**  @param  orig_sgl    - original SGL
**  @param  stripe_size - stripe size
**  @param  orig_rrb    - original (parent) RRB
**
**  @return *offset     - current byte offset into original SGL
**  @return *sectors    - current sector count
**  @return *lsda       - current LSDA
**  @return *remaining  - total remaining sector count
**
******************************************************************************
**/

static void r_initrrb(UINT32 *offset, UINT32 *sectors, UINT64 *lsda, UINT32 *remaining,
                      RRB *rrb, RPN *rpn, SGL *orig_sgl, UINT32 stripe_size, RRB *orig_rrb)
{
    /* Set up LSDA, LEDA and outstanding RRB count in parent. */
    rrb->lsda = *lsda;                      /* Set up LSDA. */
    /* Note: prrb structure has two uses. a) count, b) RRB pointer. */
    /* Bump outstanding RRB count. */
    orig_rrb->prrb = (RRB *)(((UINT32)orig_rrb->prrb) + 1);
    rrb->leda = *sectors + *lsda;           /* Set up LEDA. */
    rrb->rpn = rpn;                         /* Set up RPN pointer. */

    /* Generate and link in new SGL. */
    rrb->sgl = m_gensgl(*sectors, *offset, orig_sgl);

    rrb->sglsize = rrb->sgl->size;          /* Set up SGL pointer and size in RRB. */
    rrb->rdd = orig_rrb->rdd;               /* Set up RDD. */
    rrb->ilt = orig_rrb->ilt;               /* Set up ILT. */
    rrb->psda = orig_rrb->psda;             /* Set up Parity SDA. */
    rrb->peda = orig_rrb->peda;             /* Set up Parity EDA. */
    /* NOTE: prrb structure has two uses, a) count, b) RRB pointer. */
    rrb->prrb = orig_rrb;                   /* Set up link to parent RRB. */
    rrb->type = orig_rrb->type;             /* Set up type. */
    /* Set up status as having parent, and having error recovery invoked. */
    rrb->stat = (1 << RB_PARENT) | (1 << RB_RECOV);

    /* Update return parameters. */
    *remaining = *remaining - *sectors;     /* Update total remaining sector count. */
    if (*remaining != 0)
    {
        *offset += (*sectors << 9);         /* Update byte offset into original SGL. */
        *lsda += *sectors;                  /* Update LSDA for next RRB. */
        /* Update current sector count for next RRB. */
        *sectors = (stripe_size >= *remaining) ? *remaining : stripe_size;
    }
}   /* End of r_initrrb */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   r_splitrrb
**
**  @brief  Split a single write RRB into two or more RRBs.
**
**  The specified RRB is split into two or more separate RRBs where each RRB
**  resolves to a single data PSD. The original RRB is then released.
**
**  Each RRB is then linked to the original RPN in the same relative position
**  as the original RRB.
**
**  @param  rrb - current RRB
**  @param  rpn - RPN
**  @param  r5s - R5S structure
**
**  @return **RRB thread.
**
******************************************************************************
**/

static RRB **r_splitrrb(RRB *rrb, RPN *rpn, R5S *r5s)
{
    UINT32 offset = 0;                      /* Offset into original SGL. */
    RRB   *new_rrb;
    RRB   *first_rrb;
    RRB   *last_rrb;
    UINT32 remaining;
    UINT32 sectors;
    UINT64 lsda;

    /* Initialize outstanding RRB count within parent RRB. */
    rrb->prrb = NULL;

    lsda = rrb->lsda;                       /* Get LSDA. */
    remaining = rrb->leda - lsda;           /* Calculate total sector count. */
    sectors = r5s->sps - (lsda % r5s->sps); /* Calculate sectors in 1st RRB. */

    /* Generate 1st split RRB. */
    first_rrb = get_rrb();                  /* Save first split RRB. */
#ifdef M4_DEBUG_RRB
    CT_history_printf("%s%s:%u get_rrb 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, new_rrb);
#endif /* M4_DEBUG_RRB */
    last_rrb = first_rrb;                   /* Save last split RRB. */
    r_initrrb(&offset, &sectors, &lsda, &remaining, first_rrb, rpn, rrb->sgl, r5s->sps, rrb);
    first_rrb->flags = rrb->flags;          /* Set flags. */

    for (;;)
    {
        /* Generate next split RRB. */
        new_rrb = get_rrb();
#ifdef M4_DEBUG_RRB
        CT_history_printf("%s%s:%u get_rrb 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, new_rrb);
#endif /* M4_DEBUG_RRB */
        new_rrb->bthd = last_rrb;           /* Link with previous split RRB. */
        last_rrb->fthd = new_rrb;
        last_rrb = new_rrb;                 /* Save last split RRB. */

        r_initrrb(&offset, &sectors, &lsda, &remaining, last_rrb, rpn, rrb->sgl, r5s->sps, rrb);
        last_rrb->flags = rrb->flags;       /* Set flags. */
        if (remaining == 0)
        {
            break;
        }
    }

    /* Remove the original RRB from the RPN and link in the split RRBs in its place. */
    RRB *orig_rrb_fthd = rrb->fthd;
    RRB *orig_rrb_bthd = rrb->bthd;

    if (orig_rrb_bthd == 0)                 /* If linked directly from RPN. */
    {
        rpn->rrbhead = first_rrb;            /* Link 1st split RRB to RPN. */
        first_rrb->bthd = orig_rrb_bthd;
    }
    else
    {
        orig_rrb_bthd->fthd = first_rrb;    /* Link first split RRB to previous. */
        first_rrb->bthd = orig_rrb_bthd;
    }
    last_rrb->fthd = orig_rrb_fthd;         /* Link last split RRB to remainder of thread. */
    if (orig_rrb_fthd != 0)                 /* If not end of thread. */
    {
        orig_rrb_fthd->bthd = last_rrb;     /* Link remainder of thread to split RRBs. */
    }
    else
    {
        rpn->rrbtail = last_rrb;            /* Set up RRB queue tail. */
    }
    return(&rpn->rrbhead);                  /* Set to rescan all RRBs. */
}   /* End of r_splitrrb */

/* ------------------------------------------------------------------------ */

/*  --- Algorithm 6 -----------------------------------------------------
 *
 *  Initiate partial stripe write w/ multiple data drives
 *
 *  If any failing PSDs exist within the full stripe encompassing this
 *  operation and/or the RRB indicates that error recovery has been invoked,
 *  the original RRB is split into separate requests with each request
 *  referencing only a single data drive. These requests are then resubmitted
 *  for processing by algorithms 4 and/or 5.
 */

static void raid_Algorithm_6(RRB *rrb, RPN *rpn, R5S *r5s, PSD *failing_psd)
{
    PSD *working_PSD;
    ILT *new_ilt;
    int device_count;
    ILT *save_ilt = 0;                  /* Clear save_ilt = parity write ILT. */
    UINT32  sgl_ptr_index = 0;          /* Clear sgl_ptr_index = RRB SGL ptr offset. */
    UINT32 sgl_byte_offset = 0;
    int flag_to_read_requests = 0;      /* If should process read requests now flag. */

    /* Check for error recovery not having been invoked, and no failing PSD for RPN. */
    if (BIT_TEST(rrb->stat, RB_RECOV) || failing_psd != 0)
    {
        /*
         * Split RRB into multiple RRBs w/ each targeting a single data drive,
         * algorithms 4 and/or 5 will be used upon resubmission.
         */
        /* NOTE: r_splitrrb returns RRB** and this is OK, as written (*sigh*) */
        (void)r_splitrrb(rrb, rpn, r5s);
        return;
    }

    /*
     * All devices that are a part of this stripe are currently defined as
     * being operational. The parity for this operation is constructed from
     * the data being written and additional data read from those areas that
     * are not being written.
     */
    rrb->wsq = 0;                       /* Clear write ILT submission queues. */
    rrb->rsq = 0;                       /* Clear read ILT submission queues. */
    rrb->fmsgl = 0;                     /* Clear fore merged SGLs. */
    rrb->amsgl = 0;                     /* Clear aft merged SGLs. */
    rrb->frsgl = 0;                     /* Clear fore read SGLs. */
    rrb->arsgl = 0;                     /* Clear aft read SGLs. */

    /* Get original base physical SDA into Origin of stripe. */
    r5s->bpsda = r5s->obpsda;
    working_PSD = r5s->spsd;            /* Get starting PSD. */
    sgl_byte_offset = 0;                /* Clear byte offset into SGL. */
    sgl_ptr_index = 0;                  /* Clear sgl_ptr_index = RRB SGL ptr offset. */
    device_count = r5s->depth;          /* Get stripe depth (3, 5, or 9) */

    /*
     * Process fore devices in stripe (devices with their stripe not being
     * fully written preceding and possibly including the first data device in
     * the stripe being written). If the parity device is encountered, that
     * device will be processed at this time.
     */
     for (;;)
     {
        if (working_PSD == rpn->ppsd)
        {
            /* Process parity device. */
            r_r5a6parity(rrb, working_PSD, r5s);
        }
        else if (working_PSD != r5s->cpsd)
        {
            /* Process full fore device. */
            r_r5a6fread(rrb, working_PSD, sgl_ptr_index, r5s);
            sgl_ptr_index++;                   /* Advance RRB SGL pointer index. */
        }
        else
        {
            if (r5s->cso != 0)
            {
                /* Process partial fore device. */
                r_r5a6pfread(rrb, working_PSD, r5s);
            }
            break;
        }

        /* Advance to next device. */
        working_PSD = working_PSD->npsd;    /* Advance to next PSD. */
        device_count--;                     /* Adjust remaining device count. */
        if (working_PSD == rpn->wpsd)       /* If wrap PSD */
        {
            r5s->bpsda += r5s->sps;         /* Update base physical SDA by stripe size. */
        }
    }

    /*
     * Process core devices in stripe (data devices with their stripe being
     * fully or partially written). If the parity device is encountered, that
     * device will be processed at this time.
     */
     for (;;)
     {
        if (working_PSD == rpn->ppsd)
        {
            /* Process parity device. */
            r_r5a6parity(rrb, working_PSD, r5s);
        }
        else if (r5s->cso != 0)
        {
            /* Process partial fore core device write. */
            sgl_byte_offset = r_r5a6pwrite(rrb, working_PSD, sgl_byte_offset, rrb->sgl, r5s);

            /* Merge partial fore read and partial fore write SGLs for XOR. */
            sgl_ptr_index = r$r5a6msglrw(rrb, sgl_ptr_index);       /* Merge read and write SGL. */
        }
        else if (r5s->rsc >= r5s->sps)
        {
            /* Process full core device write. */
            sgl_byte_offset = r_r5a6fwrite(rrb, working_PSD, sgl_byte_offset, sgl_ptr_index, rrb->sgl, r5s);
            sgl_ptr_index++;                   /* Advance RRB SGL pointer index. */
        }
        else
        {
            /* Routine r_r5a6pwrite() changes the remaining sector count in the r5s structure. */
            UINT32 save_remaining_sector_count = r5s->rsc;

            /* Process partial aft core device write. */
            sgl_byte_offset = r_r5a6pwrite(rrb, working_PSD, sgl_byte_offset, rrb->sgl, r5s);
            r5s->cso = save_remaining_sector_count;   /* Update current sector offset. */
            break;
        }

        /* Advance to next device. */
        device_count--;                     /* Adjust remaining device count. */
        if (device_count == 0)
        {
            flag_to_read_requests = 1;
            break;
        }
        working_PSD = working_PSD->npsd;    /* Advance to next PSD. */
        if (rpn->wpsd == working_PSD)       /* If wrap PSD. */
        {
            r5s->bpsda += r5s->sps;         /* Update base physical SDA by stripe size. */
        }
        r5s->cso = 0;                       /* Reset current sector offset. */
        if (r5s->rsc == 0)                   /* If no more remaining sectors. */
        {
            break;
        }
    }

    /*
     * Process aft devices in stripe (devices with their stripe not being
     * fully written following and possibly including the last data device in
     * the stripe being written). If the parity device is encountered, that
     * device will be processed at this time.
     */
    if (flag_to_read_requests == 0)
    {
        if (r5s->cso != 0)
        {
            /* Process partial aft read. */
            r_r5a6paread(rrb, working_PSD, r5s);

            /* Merge partial aft write and partial aft read SGLs for XOR. */
            sgl_ptr_index = r$r5a6msglwr(rrb, sgl_ptr_index);        /* Merge write and read SGL. */

            r5s->cso = 0;                       /* Reset current sector offset. */
            goto    iw840;                      /* Ugly Ugly Ugly. */
        }

        /* Full aft read. */
        /* Check for parity device. */
        for (;;)
        {
            if (working_PSD == rpn->ppsd)           /* If parity device. */
            {
                /* Process parity device. */
                r_r5a6parity(rrb, working_PSD, r5s);
            }
            else
            {
                /* Process full aft device. */
                r_r5a6fread(rrb, working_PSD, sgl_ptr_index, r5s);
                sgl_ptr_index++;                   /* Advance RRB SGL pointer index. */
            }
iw840: /* Ugly Ugly Ugly. */
            /* Advance to next device. */
            device_count--;                     /* Adjust remaining device count. */
            if (device_count == 0)
            {
                break;                          /* If complete. */
            }
            working_PSD = working_PSD->npsd;    /* Advance to next PSD. */
            if (working_PSD == rpn->wpsd)       /* If not wrap PSD. */
            {
                r5s->bpsda += r5s->sps;         /* Update base physical SDA by stripe size. */
            }
        }
    }

    /*
     * Queue all read requests from read submission queue located in RRB.
     * When all of the read requests have completed, the data for the entire
     * stripe will be XORed and the requests from the write submission queue
     * will be submitted by the completion routine.
     */
    new_ilt = rrb->rsq;                 /* Get 1st request. */
    device_count = 0;                   /* Initialize queued request count. */

    /* Submit next read request. */
    for (;;)
    {
        save_ilt = new_ilt->fthd;           /* Stage next request. */
        device_count++;                     /* Bump queued request count. */

        /* Queue next read request without wait. */
        EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r5a6rcomp);

        new_ilt = save_ilt;                 /* Swap next to current request. */
        if (save_ilt == 0)
        {
            break;                          /* If no more to do. */
        }
    }
    rrb->sglarray[sgl_ptr_index + 1] = 0;    /* Set RRB SGL terminator. */
    rrb->orc = device_count;            /* Set outstanding request count. */
    rrb->rsq = 0;                       /* Clear read submission/completion queue. */
}   /* End of raid_Algorithm_6 */

/* ------------------------------------------------------------------------ */

/*--- Algorithm 7 -----------------------------------------------------
 *
 *  Process full or partial stripe write w/ bad parity drive
 *
 *  All data devices that are a part of this stripe are currently defined as
 *  operational. The parity device, however, is not operational. The writes
 *  for each data drive will be initiated without regard for the parity device.
 *  Since the parity isn't being updated, there is no requirement to log this
 *  activity into NVRAM.
 */

static void raid_Algorithm_7(RRB *rrb, RPN *rpn, R5S *r5s)
{
    PSD *working_PSD;
    ILT *new_ilt;
    PRP *new_prp;
    UINT32 sectors;
    SGL *new_sgl;

    /* Start with current PSD. */
    working_PSD = r5s->cpsd;

    /* Process next device in stripe. */
    for (;;)
    {
        if (working_PSD != rpn->ppsd)           /* If not parity device. */
        {
            /* Process data device. */
            rrb->orc++;                         /* Bump outstanding request count. */

            /* Generate ILT/PRP. */
            new_ilt = get_ilt();                    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
            CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
            new_prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
            CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, new_prp);
#endif /* M4_DEBUG_PRP */
            new_ilt->ilt_normal.w0 = (UINT32)new_prp;   /* Link PRP to ILT. */

            new_ilt->ilt_normal.w3 = (UINT32)rrb;   /* Set up RRB. */
            new_ilt->ilt_normal.w4 = (UINT32)working_PSD;   /* Set up PSD. */

            sectors = r5s->sps - r5s->cso;      /* Get remaining sector count. */
            sectors = (r5s->rsc > sectors) ? sectors : r5s->rsc;
            /* Build the PRP. */
            r_bldprp(new_prp, working_PSD, r5s->bpsda + r5s->cso, sectors, r5s->function, r5s->strategy);

            /* Generate new SGL for this request. */
            new_sgl = m_gensgl(sectors, r5s->bo, rrb->sgl);
            new_prp->sglSize = new_sgl->size;        /* Link SGL to PRP. */
            new_prp->pSGL = new_sgl;

            r5s->bo += sectors << 9;

            /* Queue physical request (data) without wait. */
            EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r5fpdwcomp);

            /* Update remaining sector count. */
            if (r5s->rsc == sectors)
            {
                return;                         /* Flag that RPN still exists. */
            }
            r5s->rsc = r5s->rsc - sectors;      /* Update remaining sector count. */
            r5s->cso = 0;                       /* Clear current sector offset. */
        }
        /* Advance to next device. */
        working_PSD = working_PSD->npsd;    /* Advance to next PSD. */
        if (rpn->wpsd == working_PSD) {     /* If wrap PSD. */
            r5s->bpsda += r5s->sps;         /* Adjust base PSDA. */
        }
    }
}   /* End of raid_Algorithm_7 */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_initwrite
**
**  @brief  Initiate the actual physical I/O for a RAID5 write operation.
**
**  The R5S structure which is passed along with the RRB and RPN is used as a
**  scratchpad for generating the physical I/O operations which are required
**  for I/O to a RAID level 5 stripe.
**
**  Two basic write techniques are employed. The first technique is used
**  whenever a full stripe write is requested. This technique accommodates
**  three variations, a write with good data and parity devices, a write with
**  good data and bad parity devices, and a write with a single bad data
**  device and good parity device.
**
**  The second technique is used whenever a partial stripe write is requested.
**  This technique accommodates four variations, a 3 drive write with good
**  data and parity devices, a 5 or 9 drive write with good data and parity
**  devices, a write with good data and bad parity devices, and a write with
**  a single bad data device and good parity device.
**
**  Each of these techniques and their variations employ multiple completion
**  routines to handle the postprocessing of completed physical I/O requests.
**
**  @param  rrb     - Pointer to RRB
**  @param  rpn     - Pointer to RPN
**  @param  r5s     - Pointer to R5S
**
**  @return 0 if RPN not deleted, 1 if RPN was deleted.
**
******************************************************************************
**/

UINT32 r_initwrite(RRB *rrb, RPN *rpn, R5S *r5s)
{
    RDD *rdd = rrb->rdd;
    PSD *failing_psd;

    if (BIT_TEST(rdd->aStatus, RD_A_LOCAL_IMAGE_IP))
    {
        /*
         * Writes are not allowed to the RAID device, return RETRY status to
         * allow the op to be retried from the Upper Layers.
         */
        rrb->rstat = EC_RETRY;          /* Set up return status. */
        return(r$rrrb(rrb));           /* 1 if RPN was released. */
    }

    /* Initialize RRB and R5S structures. */
    r_initrrbr5s(rrb, rpn, r5s);
    failing_psd = r5s->fpsd;        /* NOTE: r12 !! */

    /*
     * Due to a Hotspare taking several minutes to determine if it should be
     * done, check to see if a failed PSD is part of this RAID. If so,
     * determine if the RAID is set up to rebuild. If set up to rebuild and is
     * in the process of rebuilding, then let the write go as if no failed psd
     * (assumes stripe was rebuilt already). If not set up to rebuild and a
     * Failed PSD has occurred, then treat like nothing is wrong and issue the
     * op to the failed PSD. This will then get the RAID into the state that
     * is required for the HotSpare or Rebuild to work properly. If this is
     * not done, the failed PSD could come back on line with old data and no
     * rebuild will occur causing a miscompare or parity corruption.
     */
    if (0 == r5s->fpsd)
    {
        /* All PSDs are good. */
    }
    else if (RD_OP == rdd->status)
    {
        /* RDD is operational with bad PSD. */
        /* Make it look like no failing PSD. */
        failing_psd = 0;
        r5s->fpsd = failing_psd;
    }
    else if (RD_DEGRADED != rdd->status)
    {
        /* RDD has other problems - continue. */
        if (rpn->ppsd == failing_psd)
        {
            /* Failing parity PSD. */
            raid_Algorithm_7(rrb, rpn, r5s);
            return(0);                             /* Flag that RPN still exists. */
        }
    }
    else
    {
        /* Write not from source vdisk/cache/virtual. May be from rebuild or
         * any other module. We were not tagging/tracking it anyway. */

        /* This is the write having previous rebuild was sent- no problem. */
        /* Jif, not "write without previous rebuild -- no problem. */
        if (!(rrb->flags == 0 || BIT_TEST(rrb->flags, 1) || !BIT_TEST(rrb->flags, 0)))
        {
            /* Clear out flags. */
            rrb->flags = 0;

            /* Save the status for requestor. */
            rrb->rstat = EC_SPECIAL;
            return (r$rrrb(rrb));           /* 1 if RPN was repleased. */
        }

        /* Clear out flags. */
        rrb->flags = 0;

        /* Determine if failing_psd is in rebuilding state. */
        if (R$checkForR5PSDRebuilding(failing_psd) == TRUE)
        {
            /*JRebuilding - Stripe already rebuilt. */
            /* Make it look like no failing PSD. */
            failing_psd = 0;
            r5s->fpsd = failing_psd;
        }
        else if (!BIT_TEST(rdd->aStatus, RD_A_REBUILD))
        {
            /* RDD not flagged for rebuild. */
            /* Make it look like no failing PSD. */
            failing_psd = 0;
            r5s->fpsd = failing_psd;
        }
        else if (BIT_TEST(failing_psd->aStatus, PSDA_REBUILD))
        {
            /* PSD is in Rebuilding State. */
            if (rpn->ppsd == failing_psd)
            {
                /* Failing parity PSD. */
                raid_Algorithm_7(rrb, rpn, r5s);
                return(0);                             /* Flag that RPN still exists. */
            }
        }
        else
        {
            /* Make it look like no failing PSD. */
            failing_psd = 0;
            r5s->fpsd = failing_psd;
        }
    }

    /* Continue as normal. */
    if (r5s->fs == FALSE)
    {
        /* Partial stripe -- determine type of partial stripe write. */
        if (r5s->sdd == FALSE)
        {
            /* If no SGL disk indicator. */
            raid_Algorithm_6(rrb, rpn, r5s, failing_psd);
        }
        /* Initiate partial stripe write w/ sgl good data drive. */
        else if (r5s->cpsd == failing_psd)
        {
            /* If Current data PSD failure. */
            raid_Algorithm_5(rrb, rpn, r5s, failing_psd);
        }
        else if (failing_psd != 0 || rpn->rdd->depth != 3)
        {
            /* If any PSD failure, or not 3 drive. */
            raid_Algorithm_4(rrb, rpn, r5s);
        }
        else
        {
            raid_Algorithm_3(rrb, rpn, r5s);
        }
    }
    else if (0 == failing_psd)
    {
        /* If not failing PSD. */
        raid_Algorithm_1(rrb, rpn, r5s);
    }
    else
    {
        raid_Algorithm_2(rrb, rpn, r5s, failing_psd);
    }
    return(0);                             /* Flag that RPN still exists. */
}   /* End of r_initwrite() */


/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   r_initrrbr5s
**
**  @brief  Initialize the RRB and R5S structures prior to Raid 5 I/O.
**
**  The RRB and R5S are initialized in preparation for constructing RAID 5
**  physical I/O operations. The RRB serves as a focal point for the physical
**  I/Os that need to be performed for this particular stripe. The R5S serves
**  as a scratchpad for generating the specific physical I/O operations for
**  this stripe. The information within the R5S is aligned with the operation
**  defined within the RRB.
**
**  @param  rrb - current RRB
**  @param  rpn - RPN
**  @param  r5s - R5S structure
**
**  @return none
**
******************************************************************************
**/

void    r_initrrbr5s(RRB *rrb, RPN *rpn, R5S *r5s)
{
    UINT32 depth;
    PSD *first_psd;
    PSD *psd;
    UINT32 sps = rrb->rdd->sps;             /* Sectors per stripe. */
    UINT64 spsda = rpn->spsda;              /* Base PSDA for stripe. */
    ILT *ilt;
    RRP *rrp;

    /* Initialize RRB. */
    rrb->orc = 0;                           /* Initialize outstanding requests. */
    rrb->rstat = EC_OK;                     /* Default request status is OK. */
    BIT_SET(rrb->stat, RB_ACT);             /* Set request as active. */

    /* Initialize R5S. */
    depth = rrb->rdd->depth;                /* Get stripe width. */
    psd = rpn->spsd;                        /* Get starting PSD. */
    first_psd = psd;
    r5s->spsd = first_psd;                  /* Initialize starting PSD. */
    r5s->depth = depth;                     /* Initialize depth (stripe width). */

    for (;;)
    {
        if (first_psd->status != PSD_OP)    /* If not operational ... */
        {
            if (first_psd->status != PSD_REBUILD)
            {
                break;
            }
            if (spsda >= first_psd->rLen)   /* If requested area not rebuilt. */
            {
                break;
            }
        }
        first_psd = first_psd->npsd;        /* Link to next PSD. */
        depth--;
        if (rpn->wpsd == first_psd)         /* Check for wrap PSD. */
        {
            spsda += sps;
        }
        if (0 >= depth)
        {
            first_psd = 0;                  /* Clear failed PSD. */
            break;
        }
    }

    r5s->fpsd = first_psd;                  /* Set up failed PSD. */
    ilt = rrb->ilt;                         /* Get primary ILT. */
    rrp = (RRP *)((ilt - 1)->ilt_normal.w0); /* Get RRP. */

    /* Clone RRP parameters into R5S. */
    spsda = rpn->spsda;                     /* Get base physical SDA for stripe. */
    r5s->obpsda = spsda;                    /* Set original base physical SDA. */
    r5s->function = rrp->function;          /* Set up function code. */
    r5s->strategy = rrp->strategy;          /* Set up strategy. */
    r5s->sps = sps;                         /* Set up sectors per stripe. */
    r5s->bo = 0;                            /* Clear bye offset into SGL. */
    r5s->rsc =  rrb->leda - rrb->lsda;      /* Calculate sector count. */

    /* Locate starting data PSD for this I/O operation */
    UINT32 starting_lsda;

    /* Calculate starting LSDA offset into maxi-stripe. */
    starting_lsda = rrb->lsda - rpn->lsda;

    for (;;)
    {
        if (psd != rpn->ppsd)
        {
            if (starting_lsda < sps)
            {
                break;
            }
            starting_lsda -= sps;           /* Adjust starting LSDA offset. */
        }
        psd = psd->npsd;                    /* Link to next PSD. */
        if (psd == rpn->wpsd)               /* If wrap PSD. */
        {
            spsda += sps;                   /* Adjust base physical SDA. */
        }
    }

    /* Starting PSD found. */
    r5s->bpsda = spsda;                     /* Set up base physical SDA. */
    r5s->cso = starting_lsda;               /* Set up current stripe offset. */
    r5s->cpsd = psd;                        /* Set up current PSD. */

    /* Set up full stripe indicator. */
    r5s->fs = (rrb->rdd->spu == r5s->rsc) ? TRUE : FALSE;

    if (r5s->fs != TRUE)                    /* If not full stripe. */
    {
        /* Set up single disk indicator. */
        r5s->sdd =  ((sps - starting_lsda) >= r5s->rsc) ? TRUE : FALSE;
    }
}   /* End of r_initrrbr5s */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_optrpn
**
**  @brief  Optimize and RRB being attached to an existing RPN node.
**
**  If the RRB represents a write request then the RRB thread attached to the
**  RPN is searched for a possible write cancellation candidate. A write
**  cancellation occurs whenever the specified RRB completely overlaps an
**  existing write RRB that is attached to the RPN. A system RRB is then
**  created with both the specified and candidate RRBs attached. An exit is
**  then made with an indication of successful optimization.
**
**  If write cancellation has not occurred from the previous step, the RRB
**  thread is searched for a possible join candidate. A join is enacted
**  whenever two like requests reference a combined contiguous set of disk
**  addresses. This is accomplished by creating a system RRB with the
**  specified and candidate RRBs attached. An exit is then made with an
**  indication of successful optimization.
**
**  The write cancellation and join optimization algorithms always insure that
**  write precedence is enforced. Write precedence occurs whenever a series of
**  intermixed read and write requests or just write requests are destined to
**  any common disk addresses. The outcome of these I/O operations must be in
**  the same order that they were originally issued.
**
**  The RRB thread emanating from the RPN is constructed with the newest
**  requests occupying the tail of the thread.
**
//      g0  = RPN
//      g2  = RRB
**  @param  rpn     - Pointer to RPN
**  @param  rrb     - Pointer to RRB
**
**  @return TRUE    if new RRB - (optimization occurred)
**          FALSE   if original RRB - (optimization failed)
**  @return ret_rrb original or new RRB
**  @return ret_rpn RPN
//      g1 = TRUE  if new RRB - (optimization occurred)
//           FALSE if original RRB - (optimization failed)
//      g2 = original or new RRB
//      g3 = RPN
******************************************************************************
**/

static UINT32 r_optrpn(RRB **ret_rrb, RPN **ret_rpn, RPN *rpn, RRB *rrb)
{
    UINT64      lsda;
    UINT64      leda;
    RRB        *next_rrb;
    RRB        *new_rrb = 0;
    RRB        *working_rrb;
    RRB        *join_rrb;
    RRB        *last_rrb;

    if (BIT_TEST(rrb->stat, RB_PARENT) || BIT_TEST(rrb->stat, RB_RECOV))
    {
        goto c900;
    }
    lsda = rrb->lsda;
    leda = rrb->leda;

    if (rrb->type == RB_READ)
    {
        goto c100_5;                /* If a read. */
    }
    /* If not write. */
    if (rrb->type != RB_WRITE && rrb->type != RB_WRITEV)
    {
        goto c900;                  /* If not write or verify. */
    }

    /* Check for possible write cancellation. */
    next_rrb = (RRB*)&rpn->rrbhead; /* Get RRB thread origin. */
    for (;;)
    {
        next_rrb = next_rrb->fthd;
        if (next_rrb == 0)
        {
            goto c100_5;            /* If no more RRBs. */
        }

        /* Isolate active, recovery and parent, go to next if any are set. */
        if ((next_rrb->stat & 7) != 0 || next_rrb->type != RB_WRITE)
        {
            continue;
        }

        /* Next entry is not a complete overlap. */
        if (lsda > next_rrb->lsda || leda < next_rrb->leda)
        {
            continue;
        }

        /*
         *  Possible write cancellation candidate has been located - check for
         *  precedence within the remainder of the thread.
         */
        working_rrb = next_rrb;
        for (;;)
        {
            working_rrb = working_rrb->fthd;
            if (0 == working_rrb)
            {
                break;              /* If no more RRB's, exit loop. */
            }
            /* If possible overlap. */
            if ((working_rrb->lsda < next_rrb->leda) &&
                (working_rrb->lsda == next_rrb->lsda ||
                 working_rrb->leda == next_rrb->lsda))
            {
                break;
            }
            if (working_rrb->lsda >= leda || working_rrb->leda <= lsda)
            {
                continue;
            }
            break;
        }
        if (0 == working_rrb)
        {
            break;
        }
        /*  Precedence exists - can not perform write cancellation on this
         *  particular candidate. */
    }

    /* Proceed with write cancellation. */
// input g2 = RRB
// output g1 = true if RPN empty
// output g3 = RPN
    /* Unlink RRB from RPN. */
    (void)r_ulrrb(ret_rpn, next_rrb);

    /* Merge RRBs. */
    join_rrb = rrb->jointhd;
    /* Get candidate join thread. */
    working_rrb = next_rrb->jointhd;
    if (join_rrb != 0)
    {
        /* Candidate non-system RRB. */
        if (0 == working_rrb)
        {
            /* Merge new system RRB with candidate non-system RRB. */
            /* Insert candidate RRB into join thread. */
            rrb->jointhd = next_rrb;
            next_rrb->fthd = join_rrb;
            rrb->sn = 0;            /* Clear SN. */
            *ret_rrb = rrb;
            return (TRUE);          /* Indicate new RRB. */
        }

        /* Merge new system RRB with candidate system RRB. */
        for (;;)
        {
            last_rrb = join_rrb;        /* Save last RRB. */
            join_rrb = join_rrb->fthd;  /* Search for end of thread. */
            if (join_rrb == 0)
            {
                break;
            }
        }

        /* Release candidate SGL if not borrowed. */
        if (!BIT_TEST(next_rrb->sglsize, 31))
        {
            s_Free(next_rrb->sgl, next_rrb->sglsize, __FILE__, __LINE__);     /* Release memory. */
        }

        /* Release candidate RRB. */
        last_rrb->fthd = working_rrb;   /* Merge join threads together. */
        put_rrb(next_rrb);
        /* Release RRB. */
#ifdef M4_DEBUG_RRB
        CT_history_printf("%s%s:%u put_rrb 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, next_rrb);
#endif /* M4_DEBUG_RRB */
        rrb->sn = 0;                /* Clear SN. */
        *ret_rrb = rrb;
        return (TRUE);              /* Indicate new RRB. */
    }

    /* If new non-system RRB. */
    if (working_rrb != 0)
    {
        /* Merge new non-system RRB with candidate system RRB. */
        /* Insert new RRB into join thread. */
        next_rrb->jointhd = rrb;
        rrb->fthd = working_rrb;
        rrb = next_rrb;             /* Use candidate RRB as survivor. */
    }
    else
    {
        /* If non-system candidate RRB, merge 2 non-system RRBs. */
        new_rrb = get_rrb();        /* Assign system RRB. */
#ifdef M4_DEBUG_RRB
        CT_history_printf("%s%s:%u get_rrb 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, new_rrb);
#endif /* M4_DEBUG_RRB */

        /* Link RRBs into system RRB join thread. */
        /* OR these together to perserve the rebuild not sent 0. */
        new_rrb->flags = rrb->flags | next_rrb->flags;
        rrb->fthd = next_rrb;       /* Build joing thread of candidate. */
        new_rrb->jointhd = rrb;
    }

    /* Set up system RRB. */
    new_rrb->lsda = lsda;           /* Set up LSDA in new RRB. */
    new_rrb->leda = leda;           /* Set up LEDA in new RRB. */
    new_rrb->sgl = rrb->sgl;        /* Set up SGL in new RRB. */
    BIT_SET(new_rrb->sglsize, 31);  /* Set up SGL as borrowed in new RRB. */
    new_rrb->sn = 0;                /* Clear SN. */
    *ret_rrb = new_rrb;
    return (TRUE);                  /* Indicate new RRB. */

    /* Check for possible join. */
c100_5:
    next_rrb = (RRB*)&rpn->rrbhead; /* Get RRB thread origin. */
    for (;;)
    {
        int         flag = 0;

        next_rrb = next_rrb->fthd;
        if (next_rrb == 0)
        {
            goto c900;              /* If no more RRBs. */
        }

        /* Isolate active, recovery and parent, go to next if any are set. */
        if ((next_rrb->stat & 7) != 0 || next_rrb->type != rrb->type)
        {
            continue;
        }
        if (next_rrb->lsda != leda)     /* If not forward join. */
        {
            if (next_rrb->leda != lsda) /* If not backward join. */
            {
                continue;
            }
        }

        /*
         *  Possible join candidate has been located - check for precedence within
         *  the remainder of the thread.
         */
        working_rrb = next_rrb;
        for (;;)
        {
            working_rrb = working_rrb->fthd;
            if (working_rrb == 0)
            {
                break;              /* If none. */
            }
            if (rrb->type == RB_READ)
            {
                if (working_rrb->type == RB_READ)
                {
                    continue;
                }
            }

            /* Check for request overlap. */
            if (working_rrb->lsda < next_rrb->leda)
            {
                if (working_rrb->lsda >= next_rrb->lsda ||
                    working_rrb->leda > next_rrb->lsda)
                {
                    /* Partial overlap. */
                    flag = 1;
                    break;
                }
            }
            if (working_rrb->lsda >= leda ||
                working_rrb->leda <= lsda)
            {
                /* No partial overlap. */
                continue;
            }
            break;
        }
        if (flag == 1)
        {
            continue;
        }
        if (working_rrb == 0)
        {
            break;                  /* If none. */
        }
        /* Precedence exists - can not perform a join with this candidate. */
    }

    /* Proceed with join. */
// input g2 = RRB
// output g1 = true if RPN empty
// output g3 = RPN
    /* Unlink RRB from RPN. */
    (void)r_ulrrb(ret_rpn, next_rrb);
    /* Join RRBs. */
    if (rrb->jointhd == 0)          /* If new RRB not system. */
    {
        if (next_rrb->jointhd == 0)
        {
            /* Join new non-system RRB with candidate non-system RRB. */
            // input g1 = older non-system RRB
            // input g2 = newer non-system RRB
            // output g2 = new system RRB
            *ret_rrb = r_joinn2n(next_rrb, rrb);
            (*ret_rrb)->sn = 0;     /* Clear SN. */
        }
        else
        {
            /* Join new non-system RRB with candidate system RRB. */
            // input g1 = non-surviving system RRB
            // input g2 = surviving system RRB
            // no output
            r_joins2n(rrb, next_rrb);
            next_rrb->sn = 0;       /* Clear SN. */
            *ret_rrb = next_rrb;
        }
        return (TRUE);              /* Indicate new RRB. */
    }

    if (next_rrb->jointhd == 0)
    {
        /* Join new system RRB with candidate non-system RRB., */
        // input g1 = non-surviving system RRB
        // input g2 = surviving system RRB
        // no output
        r_joins2n(next_rrb, rrb);
    }
    else
    {
        /* Join new system RRB with candidate system RRB. */
        // input g1 = non-surviving system RRB
        // input g2 = surviving system RRB
        // no output
        r_joins2s(next_rrb, rrb);
    }
    rrb->sn = 0;                    /* Clear SN. */
    *ret_rrb = rrb;
    return (TRUE);                  /* Indicate new RRB. */

c900:
    /* Optimization failed. */
    working_rrb = rpn->rrbtail;     /* Get queue tail. */
    /* If queue not empty. */
    if (working_rrb != 0)
    {
        /* Append RRB to end of queue. */
        working_rrb->fthd = rrb;
    }
    else
    {
        /* Insert RRB into empty queue. */
        working_rrb = 0;            /* Clear backwark link. */
        rpn->rrbhead = rrb;         /* Queue single RRB. */
    }
    rrb->fthd = 0;                  /* Clear forward RRB link. */
    rrb->bthd = working_rrb;        /* Setup RRB backward link. */
    rpn->rrbtail = rrb;
    *ret_rrb = rrb;
    *ret_rpn = rpn;                 /* Return RPN. */
    return (FALSE);                 /* Indicate original RRB. */
}                                   /* End of r_optrpn */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_actrpn
**
**  @brief  Activate a specific RPN so that the RAID 5 Executive can consider.
**
**  If the RPN is already active (i.e. already present in the initiation queue)
**  this routine immediately exits. Otherwise the RPN is appended to the
**  initiation queue and the RAID 5 Executive is activated.
**
**  @param  expedited   - TRUE if we are expediting.
**  @param  rpn         - Pointer to RPN
**
**  @return none.
**
******************************************************************************
**/

void r_actrpn(int expedited, RPN *rpn)
{
    RPN *head_rpn;
    RPN *tail_rpn;
    RPN *fwd_rpn;
    RPN *bwd_rpn;

    /* Check for active RPN. */
    if (rpn->act == TRUE)
    {
        return;                         /* Exit if already activated. */
    }

    /* Insert RPN into RAID 5 Executive initiation queue. */
    head_rpn = (RPN *)R_r5exec_qu.head; /* Get queue head. */
    tail_rpn = (RPN *)R_r5exec_qu.tail; /* Get queue tail. */
    rpn->xpedite = expedited;           /* Set expedite indication. */
    if (head_rpn == 0)                  /* If queue empty. */
    {
        /* Insert RPN into empty queue. */
        fwd_rpn = 0;                    /* Clear fwd link. */
        bwd_rpn = 0;                    /* Clear bwd link. */
        head_rpn = rpn;                 /* Set up queue head. */
        tail_rpn = rpn;                 /* Set up queue tail. */
        rpn->afthd = fwd_rpn;
        rpn->abthd = bwd_rpn;
    }
    else if (expedited == FALSE)        /* If normal RPN. */
    {
        /* Insert RPN at queue tail. */
        tail_rpn->afthd = rpn;          /* Link previous RPN to new. */
        fwd_rpn = 0;                    /* Clear fwd link. */
        bwd_rpn = tail_rpn;             /* Set bwd link. */
        tail_rpn = rpn;                 /* Set up queue tail. */
        rpn->afthd = fwd_rpn;
        rpn->abthd = bwd_rpn;
    }
    else
    {
        /* Insert RPN immediately following any other expedited RPNs. */
        fwd_rpn = head_rpn;             /* Get 1st RPN. */
        bwd_rpn = 0;                    /* Clear bwd link. */
        if (head_rpn->xpedite == TRUE)  /* If expedited. */
        {
            fwd_rpn->abthd = rpn;       /* Set bwd link. */
            head_rpn = rpn;             /* Update queue head. */
            rpn->afthd = fwd_rpn;
            rpn->abthd = bwd_rpn;
        }
        else
        {
            for (;;)
            {
                bwd_rpn = fwd_rpn;              /* Save previous RPN. */
                fwd_rpn = fwd_rpn->afthd;       /* Get next RPN. */
                if (fwd_rpn == 0 || fwd_rpn->xpedite != TRUE)
                {
                    break;
                }
            }
            if (fwd_rpn != 0)
            {
                fwd_rpn->abthd = rpn;           /* Set bwd link. */
                bwd_rpn->afthd = rpn;           /* Set fwd link. */
                rpn->afthd = fwd_rpn;
                rpn->abthd = bwd_rpn;
            }
            else
            {
                /* Insert RPN at queue tail. */
                tail_rpn->afthd = rpn;          /* Link previous RPN to new. */
                fwd_rpn = 0;                    /* Clear fwd link. */
                bwd_rpn = tail_rpn;             /* Set bwd link. */
                tail_rpn = rpn;                 /* Set up queue tail. */
                rpn->afthd = fwd_rpn;
                rpn->abthd = bwd_rpn;
            }
        }
    }

    R_r5exec_qu.qcnt++;                         /* Increment queue count. */

    /* Ready RAID 5 Exec if appropriate. */
    if (TaskGetState(R_r5exec_qu.pcb) == PCB_NOT_READY)
    {
#ifdef HISTORY_KEEP
        CT_history_pcb(".q50 setting ready pcb", (UINT32)R_r5exec_qu.pcb);
#endif  /* HISTORY_KEEP */
        TaskSetState(R_r5exec_qu.pcb, PCB_READY);   /* Set executive ready. */
    }

    /* Set RPN active and update queue structure. */
    R_r5exec_qu.head = (void *)head_rpn;        /* Update queue head. */
    R_r5exec_qu.tail = (void *)tail_rpn;        /* Update queue tail. */
    rpn->act = TRUE;                            /* Set RPN active. */
}   /* End of r_actrpn */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_insrrb
**
**  @brief  Insert an RRB into the proper PRN node doing joins/write cancels.
**
**  By definition an RRB defines an I/O operation which confines itself to a
**  single RAID 5 maxi stripe. RRPs which span multiple stripes are broken
**  into multiple RRB requests.
**
**  This routine causes the specified RRB to be inserted into the proper RPN
**  node. During this process it is possible that one or more write reductions
**  can take place as well as one or more joins. This optimization process
**  can intermingle reductions and joins if necessary.
**
**  On entry the following RRB fields must be initialized by the caller:
**      rb_lsda, rb_leda, rb_sgl, rb_sglsize
**
// #       g2  = RRB
// #       g4  = maxi stripe offset
// #       g5  = maxi stripe
// #       g8  = RRP function code
// #       g11 = RDD
// #       g12 = SN
// #       g13 = RRP
// #       g14 = primary ILT
// REGS DESTROYED: g0, g1, g2,g3
**  @param  rrb      - Pointer to RRB
**  @param  mso      - Maxi stripe offset
**  @param  ms       - Maxi stripe
**  @param  function - RRP function code
**  @param  rdd      - Pointer to RDD
**  @param  sn       - Serial number of controller
**  @param  rrp      - Pointer to RRP
**  @param  ilt      - Pointer to primary ILT
**
**  @return none.
**
******************************************************************************
**/

void r_insrrb(RRB *rrb, UINT64 mso, UINT64 ms, UINT32 function, RDD *rdd,
              UINT32 sn, RRP *rrp UNUSED, ILT *ilt)
{
    UINT64          min_psda;
    UINT64          max_peda;
    UINT64          xfer_size;
    RPN            *prev_rpn;
    RPN            *orig_rpn;
    RPN            *next_rpn;
    RPN            *new_rpn;
#define RRP_IX(x)   ((x) - RRP_BASE)
    static const UINT8  r_rrp2rrb[] =   /* RRP to RRB function translation */
    {
        [RRP_IX(RRP_REBUILD)]       = RB_REBUILD,   /* Rebuild */
        [RRP_IX(RRP_INPUT)]         = RB_READ,      /* Read */
        [RRP_IX(RRP_OUTPUT)]        = RB_WRITE,     /* Write */
        [RRP_IX(RRP_OUTPUT_VERIFY)] = RB_WRITEV,    /* Write/verify */
        [RRP_IX(RRP_PARITY_CHECK)]  = RB_PARITYCHK, /* RAID 5 parity check */
        [RRP_IX(RRP_REBUILD_CHECK)] = RB_REBUILDCHK,/* Rebuild Check */
        [RRP_IX(RRP_VERIFY_CHECKWORD)]  = RB_VERIFYC,   /* Verify checkword */
        [RRP_IX(RRP_VERIFY_DATA)]   = RB_VERIFY,    /* Verify data */
    };

    /* Initialize RRB. */
    rrb->sn = sn;                      /* Set up SN. */
    for (;;)
    {
        rrb->ilt = ilt;                /* Set up primary ILT. */
        rrb->rdd = rdd;                /* Set up RRD. */
        rrb->stat = 0;                 /* Clear bit status field. */

        /* Lookup RRB function code. */
        if (RRP_IX(function) >= dimension_of(r_rrp2rrb))
        {
            fprintf(stderr, "%s: RRP function %d out of range\n",
                __func__, function);
            abort();
        }
        rrb->type = r_rrp2rrb[RRP_IX(function)];

        /* Calculate parity region. */
        xfer_size = rrb->leda - rrb->lsda;     /* Calculate xfer size. */
        min_psda = rdd->sps * ms;      /* Calculate minimum parity PSDA. */
        max_peda = min_psda + rdd->sps; /* Calculate max parity PEDA. */
        if (xfer_size < rdd->sps)
        {
            /* Set partial parity region if possible. */
            if ((rdd->sps - (mso % rdd->sps)) >= xfer_size)
            {
                min_psda += (mso % rdd->sps);   /* Adjust parity PSDA. */
                max_peda = min_psda + xfer_size;        /* Form parity PEDA. */
            }
        }
        rrb->psda = min_psda;          /* Record parity SDA. */
        rrb->peda = max_peda;          /* Record parity EDA. */

        /* Search for existing RPN. Maxi-stripes are in ascending order. */
        orig_rpn = (RPN*)&rdd->pRPNHead;    /* Get 1st RPN link. */
        next_rpn = orig_rpn;
        for (;;)
        {
            prev_rpn = next_rpn;       /* Save previous RPN. */
            next_rpn = next_rpn->fthd; /* Get next RPN. */
            if (next_rpn == 0)
            {
                break;                 /* If no more RPNs. */
            }
            if (ms <= next_rpn->stripe) /* Check for maxi-stripe. */
            {
                break;                 /* If not possible. */
            }
        }
        if (next_rpn == 0 || ms < next_rpn->stripe)     /* If not possible. */
        {
            break;
        }

        /* Optimize existing RPN. */
        next_rpn->lock = TRUE;         /* Lock RPN. */

// # Input next_rpn  = RPN
// # Input g2  = RRB
// # Output g1 = TRUE  if new RRB - FALSE if original RRB
// # Output g2 = original or new RRB
// # Output g3 = RPN
        /* Attempt to optimize RPN. */
        if (r_optrpn(&rrb, &new_rpn, next_rpn, rrb) == FALSE)
        {
            next_rpn->lock = FALSE;    /* Unlock RPN. */
            rrb->rpn = new_rpn;        /* Link RRB to RPN. */
            r_actrpn(FALSE, new_rpn);  /* Activate RPN. */
            return;
        }
        mso = rrb->lsda % rdd->spu;    /* Update mso with new maxi stripe offset. */
        /* RRB has been optimized to start over. */
    }
    /* Generate new RPN. */
    new_rpn = get_rpn();
    /* Assign new RPN. */
#ifdef M4_DEBUG_RPN
    CT_history_printf("%s%s:%u get_rpn 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, new_rpn);
#endif /* M4_DEBUG_RPN */
    r_preprpn(new_rpn, ms, rdd);       /* Initialize RPN. */

    /* Link single RRB to RPN RRB thread. */
    new_rpn->rrbhead = rrb;            /* Link RRB to RPN. */
    new_rpn->rrbtail = rrb;            /* Link RRB to RPN. */
    rrb->fthd = 0;                     /* Clear RRB threads. */
    rrb->bthd = 0;                     /* Clear RRB threads. */

    /* Insert RPN in RDD RPN thread. */
    new_rpn->fthd = next_rpn;          /* Set forward link to next RPN. */
    if (next_rpn != 0)
    {
        next_rpn->bthd = new_rpn;      /* Set backward link in next RPN. */
    }
    prev_rpn->fthd = new_rpn;          /* Set forward link from previous RPN. */
    /*  Clear backward link if queue head. */
    new_rpn->bthd = (orig_rpn == prev_rpn) ? 0 : prev_rpn;

    /* Append RPN to RAID 5 Executive initiation queue. */
    rrb->rpn = new_rpn;                /* Link RRB to RPN. */
    r_actrpn(FALSE, new_rpn);          /* Activate RPN. */
}                                      /* End of r_insrrb */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   read_Algorithm_1
**
**  @brief  Initiate full stripe read w/ recovery.
**
**  Algorithm 1
**
**  The data returned from the operational drives as part of a full stripe
**  read will also be used to reconstruct the data for the drive that has
**  failed when xor'ed with data read from the parity drive. This technique
**  cuts the number of required I/Os in half over the partial stripe read
**  algorithm below.
**
**  A read is initiated for each good device in the stripe including the
**  parity device. When all of these reads have completed, the data from these
**  devices is XOR'ed together to form the data of the failed device.
**
// g2  = RRB
// g3  = RPN
// g14 = R5S (uninitialized)
**  @param  rrb     - Pointer to RRB
**  @param  rpn     - Pointer to RPN
**  @param  r5s     - Pointer to R5S
**
**  @return none
**
******************************************************************************
**/

static void read_Algorithm_1(RRB *rrb, RPN *rpn, R5S *r5s)
{
    UINT32 sgl_ptr_offset;
    PRP *new_prp;
    PSD *working_psd;
    ILT *new_ilt;
    UINT32 depth;
    ILT *ilt_submit;
    ILT *ilt_parity;
    UINT64 psd_addr;
    SGL *new_sgl;
    UINT32 sector_count;

    /* Allocate parity ILT/PRP. */
    psd_addr = r5s->obpsda;             /* Get stripe base PSDA. */
    working_psd = r5s->spsd;            /* Starting PSD. */
    sgl_ptr_offset = 0;                 /* Clear RRB SGL pointer offset. */

    /* Generate ILT/PRP. */
    new_ilt = get_ilt();                /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
    CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)new_ilt);
#endif /* M4_DEBUG_ILT */
    new_prp = get_prp();                /* Assign PRP. */
#ifdef M4_DEBUG_PRP
    CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)new_prp);
#endif /* M4_DEBUG_PRP */
    ((ILT *)new_ilt)->ilt_normal.w0 = (UINT32)new_prp; /* Link PRP to ILT. */

    new_ilt->ilt_normal.w3 = (UINT32)rrb;       /* Set up RRB. */
    new_ilt->ilt_normal.w4 = (UINT32)rpn->ppsd; /* Set up PSD from parity PSD. */
    depth = r5s->depth;                 /* Get stripe depth. */
    rrb->orc = depth - 1;               /* Set up outstanding request count. */
    new_ilt->ilt_normal.w2= (UINT32)rrb->sn;     /* Set up SN. */
    ilt_submit = 0;                     /* Clear ILT submission thread. */
    ilt_parity = new_ilt;               /* Save parity ILT. */

    /* Process next device in stripe. */
    for (;;)
    {
        if (working_psd == r5s->fpsd)
        {
            /* Process failed data device. */
            new_ilt = ilt_parity;       /* Restore ILT. */
            new_prp = (PRP *)ilt_parity->ilt_normal.w0;    /* Get parity read PRP. */
            sector_count = r5s->sps;            /* Pass sector count. */

            /* Generate new SGL for this request. */
            new_sgl = m_gensgl(sector_count, r5s->bo, rrb->sgl);
            rrb->sglarray[0] = new_sgl;     /* Set up dst SGL in RRB. */

            /* Set up src1 SGL in RRB - The dst SGL is also used as one of the src SGLs to conserve memory */
            rrb->sglarray[1] = new_sgl;
            new_prp->pSGL = new_sgl;
            new_prp->sglSize = new_sgl->size | (1 << 31);   /* Set SGL as borrowed. */
            r5s->bo += sector_count << 9;   /* Adjust SGL byte count. */

            /* Insert ILT into submission thread. */
            new_ilt->fthd = ilt_submit;     /* Link ILT to submission thread. */
            ilt_submit = new_ilt;
        }
        else if (working_psd != rpn->ppsd)      /* If data device. */
        {
            /* Process good data device. */
            /* Generate ILT/PRP. */
            new_ilt = get_ilt();                /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
            CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)new_ilt);
#endif /* M4_DEBUG_ILT */
            new_prp = get_prp();                /* Assign PRP. */
#ifdef M4_DEBUG_PRP
            CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)new_prp);
#endif /* M4_DEBUG_PRP */
            ((ILT *)new_ilt)->ilt_normal.w0 = (UINT32)new_prp; /* Link PRP to ILT. */

            new_ilt->ilt_normal.w3 = (UINT32)rrb;       /* Set up RRB. */
            new_ilt->ilt_normal.w4 = (UINT32)working_psd;   /* Set up PSD. */
            new_ilt->ilt_normal.w2 = rrb->sn;   /* Set up SN. */
            sector_count = r5s->sps;            /* Pass sector count. */

            /* Build the PRP. */
            r_bldprp(new_prp, working_psd, psd_addr, sector_count, r5s->function, r5s->strategy);

            /* Generate new SGL for this request. */
            new_sgl = m_gensgl(sector_count, r5s->bo, rrb->sgl);

            /* Set up src SGL in RRB. */
            rrb->sglarray[2 + sgl_ptr_offset] = new_sgl;
            sgl_ptr_offset++;                   /* Bump for next RRB SGL. */
            new_prp->pSGL = new_sgl;
            /* Set as borrowed SGL. */
            new_prp->sglSize = new_sgl->size | (1 << 31);
            r5s->bo += sector_count << 9;       /* Adjust SGL byte offset. */
            /* Insert ILT into submission thread. */
            new_ilt->fthd = ilt_submit;         /* Link ILT to submission thread. */
            ilt_submit = new_ilt;
        }
        else
        {
            /* Process parity device. */
            new_ilt = ilt_parity;           /* Get parity ILT. */
            sector_count = r5s->sps;            /* Pass sector count. */
            /* Build the PRP. */
            r_bldprp((PRP *)ilt_parity->ilt_normal.w0, working_psd, psd_addr, sector_count, r5s->function, r5s->strategy);
        }

        /* Advance to next device. */
        depth--;                            /* Adjust remaining device count. */
        if (depth == 0)
        {
            break;                          /* If done. */
        }
        working_psd = working_psd->npsd;    /* Advance to next PSD. */
        if (working_psd == rpn->wpsd)       /* If wrap PSD. */
        {
            psd_addr += r5s->sps;           /* Adjust base PSDA by sectors per stripe. */
        }
    }

    /* Set SGL terminator. */
    rrb->sglarray[2 + sgl_ptr_offset] = 0;

    /* Queue all physical requests. */
    for (;;)
    {
        new_ilt = ilt_submit;               /* Get next ILT in submission thread. */
        ilt_submit = ilt_submit->fthd;      /* Save next ILT. */

        /* Queue physical request (data) without wait. */
        EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r5frrcomp);

        if (ilt_submit == 0)
        {
            break;
        }
    }
}                                      /* End of read_Algorithm_1 */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   read_Algorithm_2
**
**  @brief  Initiate partial stripe read or verify w/ or w/o recovery.
**
**  Algorithm 2
**
**  Initiate partial stripe read or verify w/ or w/o recovery, full stripe
**  read w/o recovery and full stripe verify w/ or w/o recovery
**
// g2  = RRB
// g3  = RPN
// g14 = R5S (uninitialized)
**  @param  rrb     - Pointer to RRB
**  @param  rpn     - Pointer to RPN
**  @param  r5s     - Pointer to R5S
**
**  @return none
**
******************************************************************************
**/

static void read_Algorithm_2(RRB *rrb, RPN *rpn, R5S *r5s)
{
    UINT32 sgl_ptr_offset;
    PRP *new_prp;
    PSD *working_psd;
    ILT *new_ilt;
    UINT64 psd_addr;
    SGL *new_sgl;
    SGL *save_sgl;
    UINT32 remaining_failed;
    UINT32 sector_count;

    /* Process data device. */
    working_psd = r5s->cpsd;        /* Get current PSD. */

    for (;;)
    {
        rrb->orc++;                 /* Bump outstanding request count. */
        if (working_psd != r5s->fpsd)   /* If not failed device. */
        {
            /* Generate ILT/PRP. */
            new_ilt = get_ilt();                 /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
            CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)new_ilt);
#endif /* M4_DEBUG_ILT */
            new_prp = get_prp();                 /* Assign PRP. */
#ifdef M4_DEBUG_PRP
            CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)new_prp);
#endif /* M4_DEBUG_PRP */
            ((ILT *)new_ilt)->ilt_normal.w0 = (UINT32)new_prp;

            new_ilt->ilt_normal.w3 = (UINT32)rrb;   /* Set up RRB. */
            new_ilt->ilt_normal.w4 = (UINT32)working_psd;   /* Set up PSD. */
            new_ilt->ilt_normal.w2 = rrb->sn;       /* Set up the SN. */

            /* Sectors per stripe - current sector offset. */
            sector_count = r5s->sps - r5s->cso;
            sector_count = (r5s->rsc >= sector_count) ? sector_count : r5s->rsc;

            /* Build the PRP. */
            r_bldprp(new_prp, working_psd, r5s->bpsda + r5s->cso, sector_count, r5s->function, r5s->strategy);

            if (RB_VERIFYC == rrb->type)   /* If verify checkword. */
            {
                new_prp->pSGL = 0;          /* Clear SGL. */
                new_prp->sglSize = 0;       /* Used to be r5s->fs -- which is wrong. */
            }
            else
            {
                if (r5s->fs == TRUE ||          /* If full stripe. */
                    r5s->sdd == FALSE)          /* If single data disk indicator. */
                {
                    /* Generate new SGL for this request, and link SGL to PRP. */
                    new_prp->pSGL = m_gensgl(sector_count, r5s->bo, rrb->sgl);
                    new_prp->sglSize = new_prp->pSGL->size;
                }
                else
                {
                    new_prp->pSGL = rrb->sgl;   /* Link original SGL. */
                    new_prp->sglSize = rrb->sgl->size | (1 << 31);  /* Set SGL as borrowed. */
                }
            }

            /* Queue read ILT/PRP to physical layer. */
            EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r5drcomp);
        }
        else
        {
            /* Process failed data device.
             *
             * Generate reconstruction buffer SGL. Data from the recovery reads
             * will be used to XOR into this buffer recreating the data that has
             * been lost. In the case of a verify, this buffer will already contain
             * the "expected" data.
             */
            psd_addr = rpn->spsda;          /* Get stripe base PSDA. */
            sgl_ptr_offset = 0;             /* Clear RRB SGL pointer offset. */

            /* Sectors per stripe - current sector offset. */
            sector_count = r5s->sps - r5s->cso;
            sector_count = (r5s->rsc >= sector_count) ? sector_count : r5s->rsc;

            if (RB_VERIFYC != rrb->type)
            {
                /* Generate new SGL for this request. */
                new_sgl = m_gensgl(sector_count, r5s->bo, rrb->sgl);
            }
            else
            {
                new_sgl = 0;                /* Clear SGL. */
            }
            rrb->sglarray[0] = new_sgl;     /* Set up dst SGL in RRB. */
            save_sgl = new_sgl;             /* Save dst XOR SGL. */

            /* Set outstanding recovery count minus failed device. */
            remaining_failed = rpn->rdd->depth - 1;
            rrb->uu.rorc = remaining_failed;

            PSD *keep_psd = working_psd;     /* Need to remember what we were on. */

            /* Locate 1st recovery device. */
            working_psd = rpn->spsd;        /* Get starting PSD. */
            if (working_psd == r5s->fpsd)   /* If failing PSD. */
            {
                working_psd = working_psd->npsd;    /* Advance to next PSD. */
                if (rpn->wpsd == working_psd)       /* If wrap PSD. */
                {
                    psd_addr += r5s->sps;   /* Adjust base PSDA by sectors per stripe. */
                }
            }

            /* Process next recovery device. */
            for (;;)
            {
                /* Generate ILT/PRP. */
                new_ilt = get_ilt();                 /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
                CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)new_ilt);
#endif /* M4_DEBUG_ILT */
                new_prp = get_prp();                 /* Assign PRP. */
#ifdef M4_DEBUG_PRP
                CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)new_prp);
#endif /* M4_DEBUG_PRP */
                ((ILT *)new_ilt)->ilt_normal.w0 = (UINT32)new_prp;

                new_ilt->ilt_normal.w3 = (UINT32)rrb;   /* Set up RRB. */
                new_ilt->ilt_normal.w4 = (UINT32)working_psd;   /* Set up PSD. */
                new_ilt->ilt_normal.w2 = rrb->sn;       /* Set up the SN. */
                new_ilt->ilt_normal.w5 = (UINT32)save_sgl;      /* Set up dst XOR SGL. */

                /* Build the PRP. */
                r_bldprp(new_prp, working_psd, psd_addr + r5s->cso, sector_count, (rrb->type == RB_VERIFY) ? RRP_INPUT : r5s->function, r5s->strategy);

                if (rrb->type != RB_VERIFYC)
                {
                    /* Allocate recovery buffer. */
                    new_sgl = m_asglbuf(sector_count << 9);

                    /* Set up next src SGL in RRB. */
                    rrb->sglarray[1 + sgl_ptr_offset] = new_sgl;
                    new_prp->pSGL = new_sgl;
                    new_prp->sglSize = new_sgl->size | (1 << 31);  /* Set SGL as borrowed. */
                }
                else
                {
                    new_sgl = 0;                    /* Clear SGL. */
                    rrb->sglarray[1 + sgl_ptr_offset] = 0;
                    new_prp->pSGL = 0;
                    new_prp->sglSize = 0;       /* Used to have top bit set with lower of 4. */
                }
                sgl_ptr_offset++;               /* Bump for next RRB SGL. */

                /* Queue next recovery read ILT/PRP to physical layer, without wait. */
                EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r5prrcomp);

                /* Advance to next recovery device. */
                remaining_failed--;         /* Adjust remaining recovery device count. */
                if (remaining_failed == 0)
                {
                    break;                  /* If recovery complete. */
                }

                for (;;)
                {
                    working_psd = working_psd->npsd;
                    if (rpn->wpsd == working_psd)
                    {
                        /* Adjust base PSDA by stripes per sector. */
                        psd_addr += r5s->sps;
                    }
                    if (working_psd != r5s->fpsd)
                    {
                        break;
                    }
                }
            }

            /* Set SGL terminator. */
            rrb->sglarray[1 + sgl_ptr_offset] = 0;

            working_psd = keep_psd;     /* Restore what we were working on. */
        }

        /* Check for additional devices. */
        if (r5s->rsc == sector_count)
        {
            return;                     /* If complete. */
        }
        r5s->rsc -= sector_count;       /* Update remaining sector count. */
        r5s->bo += sector_count << 9;   /* Adjust SGL byte offset. */
        r5s->cso = 0;                   /* Clear current sector offset. */

        for (;;)
        {
            /* Advance to next device */
            working_psd = working_psd->npsd;    /* Advance to next PSD. */
            if (rpn->wpsd == working_psd)
            {
                r5s->bpsda += r5s->sps; /* Adjust base PSDA by sectors per stripe. */
            }
            if (working_psd != rpn->ppsd)   /* If not parity device. */
            {
                break;
            }
        }
    }
}                                      /* End of read_Algorithm_2 */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_initread
**
**  @brief  Initiate the actual physical I/O for a RAID5 read operation.
**
**  The R5S structure which is passed along with the RRB and RPN is used as a
**  scratchpad for generating the physical I/O operations which are required
**  for I/O to a RAID level 5 stripe.
**
**  Two read techniques are employed. The first technique is used when a full
**  stripe read is requested with a failed data device. Data from the
**  non-failed data devices and parity device are used to reconstruct the data
**  from the failed device. The second technique covers all other scenarios.
**
**  Each technique employs two completion routines to handle the
**  postprocessing of the completed physical I/O requests.
**
// g2  = RRB
// g3  = RPN
// g14 = R5S (uninitialized)
**  @param  rrb     - Pointer to RRB
**  @param  rpn     - Pointer to RPN
**  @param  r5s     - Pointer to R5S
**
**  @return none
**
******************************************************************************
**/

void r_initread(RRB *rrb, RPN *rpn, R5S *r5s)
{
    /* Initialize RRB and R5S structures. */
    r_initrrbr5s(rrb, rpn, r5s);

    if (RB_VERIFYC <= rrb->type ||      /* If checkword or data. */
        r5s->fs == FALSE ||             /* If full stripe indicator. */
        r5s->fpsd == 0 ||               /* If no failing PSD. */
        rpn->ppsd == r5s->fpsd)         /* If failed parity PSD. */
    {
        read_Algorithm_2(rrb, rpn, r5s);
    }
    else
    {
        read_Algorithm_1(rrb, rpn, r5s);
    }
}                                      /* End of r_initread */

/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
