
#include <stdio.h>
#include <string.h>
#include "CT_defines.h"

/* ------------------------------------------------------------------------ */
void __cyg_profile_func_enter(void *executing_routine, void *exit_to);
void __cyg_profile_func_exit(void *executing_routine, void *exit_to);
/* ------------------------------------------------------------------------ */

static char buf[1024];

/* ------------------------------------------------------------------------ */
void __cyg_profile_func_enter(void *executing_routine, void *exit_to)
{
#ifdef CT2_DEBUG
check_c_i960_locations_ok();
#endif  /* CT2_DEBUG */
    if (CT_NO_HISTORY == 1) {           /* If not do take "history", don't */
        return;
    }
#ifdef CT2_DEBUG
check_pcb_chain();
#endif  /* CT2_DEBUG */
    snprintf(buf, sizeof(buf), "*** Entering routine=0x%08x, exit_to=0x%08x\n",
            (UINT32)executing_routine, (UINT32)exit_to);
    buf[sizeof(buf)-1] = '\0';
    CT_history(buf);
}

/* ------------------------------------------------------------------------ */
void __cyg_profile_func_exit(void *executing_routine, void *exit_to)
{
#ifdef CT2_DEBUG
check_c_i960_locations_ok();
#endif  /* CT2_DEBUG */
    if (CT_NO_HISTORY == 1) {           /* If not do take "history", don't */
        return;
    }
#ifdef CT2_DEBUG
check_pcb_chain();
#endif  /* CT2_DEBUG */
    snprintf(buf, sizeof(buf), "***  Exiting routine=0x%08x, exit_to=0x%08x\n",
            (UINT32)executing_routine, (UINT32)exit_to);
    buf[sizeof(buf)-1] = '\0';
    CT_history(buf);
}

/* ------------------------------------------------------------------------ */
/* End of file cyg.c */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
