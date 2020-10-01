#include <stdio.h>

void __cyg_profile_func_enter(void *executing_routine, void *exit_routine_to)
{
    fprintf(stderr, "*** entering routine=%p, exit_routine_to=%p\n",
	    executing_routine, exit_routine_to);
}

void __cyg_profile_func_exit(void *executing_routine, void *exit_routine_to)
{
    fprintf(stderr, "***  exiting routine=%p, exit_routine_to=%p\n",
	    executing_routine, exit_routine_to);
}
