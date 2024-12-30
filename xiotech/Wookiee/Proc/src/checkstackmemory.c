#ifdef CT2_DEBUG

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "CT_defines.h"
#include "pcb.h"
#include "kernel.h"
#include <string.h>

/* ------------------------------------------------------------------------ */
extern PCB *K_pcborg;
extern char ct_stack[64*16 + 512];      /* in fe_main.c or be_main.c */

/* ------------------------------------------------------------------------ */
#define I960_STACK_CHECK_SIZE       (TOTAL_STACK_SIZE/8)
/* CHECK that 1/4 of i960 stack is left "free". */
#define FREE_I960_STACK_CHECK_SIZE  (I960_STACK_CHECK_SIZE/4)
#define C_STACK_CHECK_SIZE          (pc_CT_C_stack_size/4)
/* CHECK that 1/8 of C stack is left "free". */
#define FREE_C_STACK_CHECK_SIZE     (C_STACK_CHECK_SIZE/8)

/* ------------------------------------------------------------------------ */
static void handle_error (int i960ORc, int errorCount, UINT32 ii, const char *s, PCB *pPCB);
static void handle_error (int i960ORc, int errorCount, UINT32 ii, const char *s, PCB *pPCB)
{
    if (errorCount == 1)
    {
        fprintf(stderr, "******** checkstackmemory:\n");
        if (i960ORc == 0)
            fprintf (stderr, "c ");
        else
            fprintf (stderr, "i960 ");
        fprintf(stderr, "stack messed up %s: %s\n", s, pPCB->pc_fork_name);
        fprintf(stderr, "pPCB=%p\n", pPCB);
        fprintf(stderr, "pPCB->thd=%p\n", pPCB->pc_thd);
        fprintf(stderr, "pPCB->global=%x\n", pPCB->pc_global);
        fprintf(stderr, "pPCB->pri=%d\n", pPCB->pc_pri);
        fprintf(stderr, "pPCB->stat=%x\n", pPCB->pc_stat);
        fprintf(stderr, "pPCB->rsreg=%x\n", pPCB->pc_rsreg);
        fprintf(stderr, "pPCB->time=%x\n", pPCB->pc_time);
    }
    /* Limit to 50 lines printed */
    if (errorCount <= 50)
    {
        if (i960ORc == 0)
        {
            fprintf(stderr, "pc_c_stack[%d] = 0x%08x (%d)\n", ii, pPCB->pc_c_stack[ii], pPCB->pc_c_stack[ii]);
        }
        else
        {
            ii = TOTAL_STACK_SIZE - 4 - 4 * ii;
            fprintf(stderr, "pc_stack[%d] = 0x%08x (%d)\n", ii, *(UINT32 *)&pPCB->pc_stack[ii],
                    *(UINT32 *)&pPCB->pc_stack[ii]);
        }
    }
}

/* ------------------------------------------------------------------------ */
void checkstackmemory(const char *s, UINT32 pcbAddr)
{
    if (pcbAddr == 0)
    {
        return;
    }

    int     errorCount;
    UINT32  ii;
    UINT32  *ulPtr;
    PCB     *pPCB = (PCB *)pcbAddr;

    errorCount = 0;
    for ( ii = 0; ii < FREE_C_STACK_CHECK_SIZE; ii ++)
    {
        if (pPCB->pc_c_stack[ii] != ii)
        {
            errorCount++;
            handle_error (0, errorCount, ii, s, pPCB);
        }
    }
    if (errorCount != 0)
    {
        fprintf (stderr, "******** Total of %d differences\n", errorCount);
        abort();
    }
    ulPtr = (UINT32 *)&pPCB->pc_stack[TOTAL_STACK_SIZE - 4];
    for (ii = 0; ii < FREE_I960_STACK_CHECK_SIZE; ii++, ulPtr--)
    {
        if (*ulPtr != ii)
        {
            errorCount++;
            handle_error (1, errorCount, ii, s, pPCB);
        }
    }
    if (errorCount != 0)
    {
        fprintf (stderr, "******** Total of %d differences\n", errorCount);
        abort();
    }
}

/* ------------------------------------------------------------------------ */
void initstackmemory (UINT32 pcbAddr)
{
    PCB     *pPCB = (PCB *)pcbAddr;
    UINT32  ii;
    UINT32  *ulPtr;

    if (pPCB == NULL)
    {
        return;
    }
/* Initialize all of c stack for "viewing" in crashes. */
    for (ii = 0; ii < C_STACK_CHECK_SIZE; ii++)
    {
        pPCB->pc_c_stack[ii] = ii;
    }
/* Initialize all of i960 stack for "viewing" in crashes. */
    ulPtr = (UINT32 *)&pPCB->pc_stack[TOTAL_STACK_SIZE - 4];
    for (ii = 0; ii < I960_STACK_CHECK_SIZE; ii++, ulPtr--)
    {
        *ulPtr = ii;
    }
}

/* ------------------------------------------------------------------------ */
/* CT_NOCHECK_STACK == 0 means to check stack.
                    == 1 means to clear for next time and return immediately.
                    == 2 means to return immediately.
 */

void check_c_i960_locations_ok(void)
{
    /* If we haven't started running yet. */
    if (K_xpcb == NULL
#ifdef HISTORY_KEEP
                       || CT_NOCHECK_STACK != 0
#endif  /* HISTORY_KEEP */
                                                )
    {
#ifdef HISTORY_KEEP
        if (CT_NOCHECK_STACK == 1)
        {
            CT_NOCHECK_STACK = 0;
        }
#endif  /* HISTORY_KEEP */
        return;
    }

    static bool do_check = true;

    /* If we have aborted already, do not check and abort again. */
    if (do_check == false)
    {
        return;
    }

    /* If routine is kernel.as between TaskSleepMS and TaskReadyByState, do
       not check stack -- problem is -- how do we know where we are!? */


    /* g15 (fp) = current frame pointer, i.e. where r registers are on stack. */
    /* r0 (pfp) = previous frame pointer. */
    /* r1 (sp) = current stack poitner. */
    /* i960 stack moves upwards, therefore (sp) is most used location. */

    /* Check that fp, pfp, sp all in current i960 stack area. */
    if (g15 < (ulong)&(K_xpcb->pc_sf0.sf_pfp) &&
        !(g15 >= (ulong)&(ct_stack[0]) && g15 <= (ulong)&(ct_stack[64*16 + 512])))
    {
        do_check = false;
        fprintf(stderr, "fp/g15 (0x%08lx) less than PCB 1st stack frame (%p).\n",
                        g15, &(K_xpcb->pc_sf0.sf_pfp));
        abort();
    }
    if (g15 >= (ulong)&(K_xpcb->pc_stack[TOTAL_STACK_SIZE]))
    {
        do_check = false;
        fprintf(stderr, "g15/fp (0x%08lx) greater than PCB last stack location (%p).\n",
                        g15, &(K_xpcb->pc_stack[TOTAL_STACK_SIZE]));
        abort();
    }

    if (r0 < (ulong)&(K_xpcb->pc_sf0.sf_pfp))
    {
        do_check = false;
        fprintf(stderr, "pfp/r0 (0x%08lx) less than PCB 1st stack frame (%p).\n",
                        r0, &(K_xpcb->pc_sf0.sf_pfp));
        abort();
    }
    if (r0 >= (ulong)&(K_xpcb->pc_stack[TOTAL_STACK_SIZE]))
    {
        do_check = false;
        fprintf(stderr, "pfp/r0 (0x%08lx) greater than PCB last stack location (%p).\n",
                        r0, &(K_xpcb->pc_stack[TOTAL_STACK_SIZE]));
        abort();
    }

    if (r1 < (ulong)&(K_xpcb->pc_sf0.sf_pfp) &&
        !(r1 >= (ulong)&(ct_stack[0]) && r1 <= (ulong)&(ct_stack[64*16 + 512])))
    {
        do_check = false;
        fprintf(stderr, "sp/r1 (0x%08lx) less than PCB 1st stack frame (%p).\n",
                        r1, &(K_xpcb->pc_sf0.sf_pfp));
        abort();
    }
    if (r1 >= (ulong)&(K_xpcb->pc_stack[TOTAL_STACK_SIZE]))
    {
        do_check = false;
        fprintf(stderr, "sp/r1 (0x%08lx) greater than PCB last stack location (%p).\n",
                        r1, &(K_xpcb->pc_stack[TOTAL_STACK_SIZE]));
        abort();
    }

    /* Check %esp, %ebp for reasonableness next. */
    UINT32 esp_reg = get_esp();
    if (esp_reg < (ulong)&(K_xpcb->pc_c_stack[0]))
    {
        do_check = false;
        fprintf(stderr, "esp register (0x%08x) less than start of c stack (%p).\n",
                        esp_reg, &(K_xpcb->pc_c_stack[0]));
        abort();
    }
    if (esp_reg >= (ulong)&(K_xpcb->pc_c_stack[pc_CT_C_stack_size/4]) &&
        !(esp_reg >= CT_main_esp-1024 && esp_reg <= CT_main_esp))
    {
        do_check = false;
        fprintf(stderr, "esp register (0x%08x) greater than end of c stack (%p).\n",
                        esp_reg, &(K_xpcb->pc_c_stack[0]));
        abort();
    }

    UINT32 ebp_reg = get_ebp();
    if (ebp_reg < (ulong)&(K_xpcb->pc_c_stack[0]))
    {
        do_check = false;
        fprintf(stderr, "ebp register (0x%08x) less than start of c stack (%p).\n",
                        ebp_reg, &(K_xpcb->pc_c_stack[0]));
        abort();
    }
    if (ebp_reg >= (ulong)&(K_xpcb->pc_c_stack[pc_CT_C_stack_size/4]))
    {
        do_check = false;
        fprintf(stderr, "ebp register (0x%08x) greater than end of c stack (%p).\n",
                        ebp_reg, &(K_xpcb->pc_c_stack[0]));
        abort();
    }
}

/* ------------------------------------------------------------------------ */
#if 0
void checkallstacks(void)
{
    /* If we haven't started running yet, exit. */
    if (K_xpcb == NULL)
    {
        return;
    }

    PCB     *pPCB;
    int     ii = 0;

    pPCB = (PCB *)K_pcborg;
    while (pPCB != NULL && (PCB*)pPCB->pc_thd != (PCB*)K_pcborg)
    {
        fprintf (stderr, "checkallstacks: Task %d\n", ++ii);
        checkstackmemory ("checkallstacks", (UINT32)pPCB);
        pPCB = (PCB *)pPCB->pc_thd;
    }
}

#endif /* 0 */
/* ------------------------------------------------------------------------ */

#include <stdio.h>
#include "pcb.h"
#include "kernel.h"

void check_pcb_chain(void)
{
    /* If we haven't started running yet, exit. */
//    if (K_xpcb == NULL)
//    {
//        return;
//    }

    PCB     *pPCB;

    pPCB = K_pcborg;
    while (pPCB != NULL && pPCB->pc_thd != K_pcborg) {
        int i;
        for (i = 0; i< XIONAME_MAX; i++) {
            if (pPCB->pc_fork_name[i] < 0x20 && pPCB->pc_fork_name[i] != 0) {
                abort();
            }
        }
        if (pPCB->pc_thd == NULL)
        {
            abort();
        }
        pPCB = pPCB->pc_thd;
    }
}

#endif /* CT2_DEBUG */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
