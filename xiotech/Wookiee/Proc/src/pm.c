/* $Id: pm.c 144582 2010-07-23 19:53:49Z mdr $ */
/**
******************************************************************************
**
**  @file       pm.c
**
**  @brief      Packet Managment support routines written in c.
**
**  Copyright (c) 2009-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "CT_defines.h"
#include "sgl.h"
#include "defbe.h"
#include "ilt.h"
#include "pm.h"
#include "prp.h"
#include "rrp.h"
#include "ecodes.h"
#include "CT_defines.h"

/* Buffer granularity - 1 */
#define BGRAN   15
/* Precede real struct by 16 bytes. */
#define sg_alloc_len    (MGRAN + 1)
/* We allocate on 64 byte boundaries. */
#define MGRAN   63              /* Memory granularity. */

/* Combined size of SGL header, descriptor, buffer and granularity. */
#define SGALLOCADJ  ((BGRAN+1) + ((((sizeof(SGL)+sizeof(SGL_DESC)+sg_alloc_len+MGRAN)/(MGRAN+1))-1)*(MGRAN+1)))


#ifdef BACKEND
/**
******************************************************************************
**
**  @name   m_gensgl
**
**  @brief  Generate new SGL from given SGL with byte offset and sector count.
**
**  A new SGL is allocated. This SGL is constructed with information taken from
**  the original SGL and modified by the byte offset. The ownership count for
**  the new SGL is initialized to one. Each additional owner of this SGL must
**  increment this count by one in those instances where this SGL is shared.
**
**  @param  sectors     - Sector count
**  @param  sgl_offset  - byte offset into original SGL (usually multiple of 512
**  @param  sgl         - Original SGL
**
**  @return New SGL
**
***********************************************************************
**/

SGL *m_gensgl(UINT32 sectors, UINT32 sgl_offset, SGL *sgl)
{
    SGL_DESC   *starting_desc;
    UINT32      byte_offset;
    UINT32      descriptor_count;
    UINT32      bytes;
    SGL_DESC   *ending_desc;
    UINT32      len;
    UINT32      new_sgl_size;
    SGL        *new_sgl;
    SGL_DESC   *new_sgl_desc;
    UINT32      sgl_len;

    /* Initialize search for starting descriptor. */
    starting_desc = (SGL_DESC *)(sgl + 1);  /* Descriptor follows header. */
    byte_offset = sgl_offset;               /* Get byte offset into SGL. */

    /* Get to the right SGL for this offset. */
    while (byte_offset >= starting_desc->len)
    {
        byte_offset -= starting_desc->len;
        starting_desc++;                    /* Move to next descriptor. */
    }

    /* Calculate the number of descriptors required. */
    descriptor_count = 1;
    bytes = sectors * BYTES_PER_SECTOR; /* Get bytes from sectors. */
    ending_desc = starting_desc;
    len = starting_desc->len - byte_offset;      /* Offset byte. */

    /* Keep going forward on descriptors till ending is found. */
    while (len < bytes)
    {
        ending_desc++;                  /* To next descriptor. */
        bytes -= len;                   /* Adjust transfer count. */
        len = ending_desc->len;         /* Next descriptor length. */
        descriptor_count++;             /* bump descriptor count. */
    }

    /* Compute size of new SGL. */
    new_sgl_size = descriptor_count * sizeof(SGL_DESC) + sizeof(SGL);

    /* Allocate and initialize a new SGL. */
    new_sgl = s_MallocW(new_sgl_size, __FILE__, __LINE__);
    new_sgl->scnt = descriptor_count;   /* Save number of descriptors. */
    new_sgl->owners = 1;                /* One owner. */
    new_sgl->flag = 0;                  /* No flags set. */
    new_sgl->size = new_sgl_size;       /* Save size of allocation. */

    /* Location of new first descriptor. */
    new_sgl_desc = (SGL_DESC *)(new_sgl + 1);

    /* There are three cases to handle.
       a) 1 SGL_DESC in new SGL (modify addr and len of SGL_DESC).
       b) 2 SGL_DESC in new SGL (modify 1st addr and len, terminate with 2nd).
       c) 3 or more in new SGL (modify first addr and len, copy middle, terminate last).
     */

    bytes = sectors * BYTES_PER_SECTOR; /* Get total bytes in new SGL from sectors. */

    /* Calculated modified first SGL_DESC xfer start addr, and len. */
    new_sgl_desc->addr = (void *)((UINT32)starting_desc->addr + byte_offset);
    if (descriptor_count == 1)
    {
        new_sgl_desc->len = bytes;
        return new_sgl;
    }

    /* Modify xfer length of first descriptor. */
    sgl_len = starting_desc->len - byte_offset;
    new_sgl_desc->len = sgl_len;        /* Store length in first descriptor. */
    bytes -= sgl_len;                   /* Adjust remaining xfer byte count. */
    starting_desc++;                    /* Advance to next sgl descriptor. */
    new_sgl_desc++;                     /* To next new sgl descriptor. */
    /* Copy the rest of the descriptors (except last). */
    while ((--descriptor_count) > 1)
    {
        new_sgl_desc->addr = starting_desc->addr;
        sgl_len = starting_desc->len;
        new_sgl_desc->len = sgl_len;
        bytes -= sgl_len;               /* Decrement xfer count. */
        starting_desc++;                /* Advance to next sgl descriptor. */
        new_sgl_desc++;                 /* To next new descriptor. */
    }
    /* Store last (or second if only two) descriptor. */
    new_sgl_desc->addr = starting_desc->addr;
    new_sgl_desc->len = bytes;

    return new_sgl;
}   /* End of m_gensgl */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   PM_MergeSGL
**
**  @brief  Merge two separate SGLs into a single new SGL.
**
**  A new SGL of the correct size is allocated and initialized with the two
**  descriptors taken from the two SGLs.
**
**  @param  lth_new_sgl - Pointer to where to store the new SGLs length (or NULL).
**  @param  pSGL1       - Pointer to first SGL.
**  @param  pSGL2       - Pointer to the seconds SGL.
**
**  @return SGL *new_sgl
**  @return UINT32 *lth_new_sgl
**
***********************************************************************
**/

SGL *PM_MergeSGL(UINT32 *lth_new_sgl, SGL *pSGL1, SGL *pSGL2)
{
    SGL *new_sgl;
    SGL_DESC *new_descr;
    SGL_DESC *first_descr = (SGL_DESC *)(pSGL1 + 1);
    SGL_DESC *second_descr = (SGL_DESC *)(pSGL2 + 1);
    UINT32 count1 = pSGL1->scnt;                /* First descriptor count. */
    UINT32 count2 = pSGL2->scnt;                /* Second descriptor count. */
    UINT32 new_sgl_size = (count1 + count2)*sizeof(SGL_DESC) + sizeof(SGL);

    /* Allocate new SGL. */
    new_sgl = s_MallocW(new_sgl_size, __FILE__, __LINE__);
    new_sgl->scnt = count1 + count2;            /* Set descriptor count. */
    new_sgl->owners = 0;                        /* Clear owners. */
    new_sgl->flag = 0;                          /* Clear flags. */
    new_sgl->size = new_sgl_size;               /* Set sgl size. */

    /* Merge 1st and 2nd SGL into single new SGL. */
    new_descr = (SGL_DESC *)(new_sgl + 1);      /* Position new SGL at 1st descriptor. */
    while (count1 != 0)
    {
        new_descr->addr = first_descr->addr;    /* Copy descriptor addr into new sgl. */
        new_descr->len = first_descr->len;      /* Copy descriptor len into new sgl. */
        first_descr++;
        new_descr++;
        count1--;
    }
    while (count2 != 0)
    {
        new_descr->addr = second_descr->addr;   /* Copy descriptor addr into new sgl. */
        new_descr->len = second_descr->len;     /* Copy descriptor len into new sgl. */
        second_descr++;
        new_descr++;
        count2--;
    }
    if (lth_new_sgl != 0)
    {
        *lth_new_sgl = new_sgl_size;            /* Return new SGL size. */
    }
    return(new_sgl);
}   /* End of PM_MergeSGL */

#endif  /* BACKEND */

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   m_asglbuf
**
**  @brief  Assign a combined SGL and buffer.
**
**  A combined SGL and data buffer are assigned. This routine will block if
**  memory resources are not available.
**
**  NOTE: there is a MGRAM byte pre-header on the SGL.
**
**  @param  byte_count  - Byte count, bit 31 forces permanent allocation.
**
**  @return SGL *new_sgl
**
***********************************************************************
**/

SGL *m_asglbuf(UINT32 byte_count)
{
    UINT32  buffer_size = byte_count & ~(1 << 31);
    byte_count += SGALLOCADJ;
    UINT32  allocated_length = byte_count & ~(1 << 31);
    UINT32 *new_sgl_and_buffer;
    SGL *new_sgl;
    SGL_DESC *new_desc;

    new_sgl_and_buffer = s_MallocW(byte_count, __FILE__, __LINE__);

    /* Save allocated length in SGL pre-header. */
    *new_sgl_and_buffer = allocated_length;

    new_sgl = (SGL *)((char *)new_sgl_and_buffer + sg_alloc_len);

    new_sgl->scnt = 1;                              /* Set descriptor count. */
    new_sgl->owners = 0;                            /* Clear owners. */
    new_sgl->flag = 0;                              /* Clear flags. */
    /* Set sgl size. */
    new_sgl->size = sizeof(SGL) + sizeof(SGL_DESC);

    /* Set up descriptor. */
    new_desc = (SGL_DESC *)(new_sgl + 1);
    new_desc->addr = (void *)(((UINT32)(new_desc + 1) + BGRAN) & ~BGRAN);
    new_desc->len = buffer_size;

    return(new_sgl);
}   /* End of m_asglbuf */


/**
******************************************************************************
**
**  @brief  Release a combined SGL and data buffer
**
**  The combined SGL and associated read buffer/write buffer is
**  released back to the Read DRAM pool.
**
**  This routine can only be called from the process level.
**
**  @param  sgl - SGL
**
******************************************************************************
**/
void PM_RelSGLWithBuf(SGL *sgl)
{
    UINT32  *alloc_ptr = (UINT32 *)((UINT8 *)sgl - SG_ALLOC_LEN);
    UINT32  len = *alloc_ptr;

    /* Release combined SGL and data buffer */

    s_Free(alloc_ptr, len, __FILE__, __LINE__);
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
