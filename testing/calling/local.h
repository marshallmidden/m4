
#define get_ebp() ({unsigned long _s_;__asm__ __volatile__("movl %%ebp,%0" : "=r" (_s_)); _s_;})
#define get_esp() ({unsigned long _s_;__asm__ __volatile__("movl %%esp,%0" : "=r" (_s_)); _s_;})

