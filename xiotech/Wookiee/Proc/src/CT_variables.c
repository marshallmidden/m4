/* $Id: CT_variables.c 144139 2010-07-14 19:46:01Z m4 $ */
/**
******************************************************************************
**
**  @file       CT_variables.c
**
**  @brief      Define variables for code translation and some needed routines.
**
**  Someplace needs to define the variables added for "c".  In addition, there
**  are some routines that are needed by the code translator.
**
**  Copyright (c) 2004-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "XIO_Macros.h"
#include "CT_defines.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static int ICON_gie = 0;        /* state of interrupts - start as enabled. */

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/

/* Define variables that are in CT_defines.h */
unsigned long CT_fork_tmp;       /* temp for fork process name */
ulong CT_main_esp;               /* main program current stack location */
ulong CT_main_ebp;               /* main program current frame location */
ulong CT_start_esp;              /* K$start stack location */
ulong CT_start_ebp;              /* K$start frame pointer */
ulong CT_stack_copy_size;        /* size of stack. */
ulong CT_ebp_diff;               /* where is ebp pointing */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      i960_fault - simulate the i960 fault instructions.
**
**              Printout what type of fault is being emulated, then abort().
**
**  @param      cmp0     - The first compare value.
**  @param      cmp1     - The second compare value.
**  @param      type     - Type of comparison done (0 through 5 for ==, ...).
**  @param      filename - i960 assembler file containing the fault command.
**  @param      lineno   - Line number in i960 assembler file.
**
**  @return     none
**
**  @attention  Calls abort().
**
******************************************************************************
**/

asm(".set _i960_fault, i960_fault");
asm(".global _i960_fault");
/* cmp_tmp0, cmp_tmp1, 0 == | 1 != | 2 < | 3 <= | 4 > | 5 >= */
NORETURN
void i960_fault(const ulong cmp0 UNUSED,
                const ulong cmp1 UNUSED,
                const ulong type, const char *filename, const int lineno)
{
  fprintf(stderr, "fault (%s:%i): ", filename, lineno);
  switch (type) {
    case 0:
      write(2, "==\n", 3);
      break;
    case 1:
      write(2, "!=\n", 3);
      break;
    case 2:
      write(2, "<\n", 2);
      break;
    case 3:
      write(2, "<=\n", 3);
      break;
    case 4:
      write(2, ">\n", 2);
      break;
    case 5:
      write(2, ">=\n", 3);
      break;
    default:
      fprintf(stderr, "unknown input 0x%lx (%ld)\n", type, type);
  }
  abort();
}

/**
******************************************************************************
**
**  @brief      i960_generate_fault - extended divide error.
**
**              If divide by zero, generate a fault.
**
**  @param      string   - A string to print out.
**  @param      filename - i960 assembler file containing the problem command.
**  @param      lineno   - Line number in i960 assembler file.
**
**  @return     none
**
**  @attention  Calls abort().
**
******************************************************************************
**/

/* Generate fault, string passed says what it is. */
NORETURN
void i960_generate_fault(const char *string, const char *filename, int lineno)
{
  fprintf(stderr, "internal fault (%s:%d): %s\n", filename, lineno, string);
  abort();
}


/**
******************************************************************************
**
**  @brief      i960_intctl - attempt to handle the intctl i960 instruction.
**
**              Does not really handle enable or disable of interrupts.
**
**  @param      src  - Source register value from instruction.
**  @param      dst  - Address of destination from instruction.
**
**  @return     none -- although dst is changed.
**
**  @attention  Static variable ICON_gie contains emulated interrupt state.
**
******************************************************************************
**/

asm(".set _i960_intctl, i960_intctl");
asm(".global _i960_intctl");
void i960_intctl(ulong src, ulong *dst)
{
  int previous_ICON_gie = ICON_gie;

  switch (src) {
    case 0:
/*         write(2, "intctl disable interrupts\n", 26); */
      ICON_gie = 1;
      break;
    case 1:
/*         write(2, "intctl enable interrupts\n", 25); */
      ICON_gie = 0;
      break;
    case 2:
/*         write(2, "intctl return interrupts\n", 25); */
      break;
    default:
      fprintf(stderr, "intctl unknown input 0x%lx (%ld)\n", src, src);
      break;
  }
  if (previous_ICON_gie == 0) {
    *dst = 1;           /* return 1 for enabled */
  } else {
    *dst = 0;           /* return 0 for disabled */
  }
}


/**
******************************************************************************
**
**  @brief      i960_intdis - handle i960 intdis (disable interrupt) command.
**
**              Does not really disable interrupts.
**
**  @param      none.
**
**  @return     none.
**
**  @attention  Static variable ICON_gie contains emulated interrupt state.
**
******************************************************************************
**/

asm(".set _i960_intdis, i960_intdis");
asm(".global _i960_intdis");
void i960_intdis(void)
{
/*     write(2, "intdis ", 7); */
/*     write(2, "disable interrupts", 18); */
/*     write(2, "\n", 1); */
  ICON_gie = 1;
}


/**
******************************************************************************
**
**  @brief      i960_inten - handle i960 inten (enable interrupt) command.
**
**              Does not really enable interrupts.
**
**  @param      none.
**
**  @return     none.
**
**  @attention  Static variable ICON_gie contains emulated interrupt state.
**
******************************************************************************
**/

asm(".set _i960_inten, i960_inten");
asm(".global _i960_inten");
void i960_inten(void)
{
/*     write(2, "inten ", 6); */
/*     write(2, "enable interrupts", 17); */
/*     write(2, "\n", 1); */
  ICON_gie = 0;
}


/**
******************************************************************************
**
**  @brief      i960_dcctl - print that an i960 dcctl instruction was reached.
**
**              Not done yet.
**
**  @param      src1 - Source 1 register value from instruction.
**  @param      src2 - Source 2 register value from instruction.
**  @param      dst  - Address of destination from instruction.
**
**  @return     none.
**
**  @attention  This routine does nothing but print a message.
**
******************************************************************************
**/

asm(".set _i960_dcctl, i960_dcctl");
asm(".global _i960_dcctl");
void i960_dcctl(ulong src1 UNUSED, ulong src2 UNUSED, ulong *dst UNUSED)
{
  write(2, "dcctl ", 6);
  write(2, "not decoded yet.", 16);
  write(2, "\n", 1);
}

/**
******************************************************************************
**
**  @brief      i960_flushreg - routine for i960 flushreg instruction.
**
**              Does nothing. Does not apply for x86.
**
**  @param      none.
**
**  @return     none.
**
**  @attention  This routine does nothing on an x86.
**
******************************************************************************
**/

asm(".set _i960_flushreg, i960_flushreg");
asm(".global _i960_flushreg");
void i960_flushreg(void)
{
/*     write(2, "flushreg ", 9); */
/*     write(2, "not done yet.", 13); */
/*     write(2, "\n", 1); */
}


/**
******************************************************************************
**
**  @brief      i960_icctl - print that an i960 icctl instruction was reached.
**
**              Not done yet.
**
**  @param      src1 - Source 1 register value from instruction.
**  @param      src2 - Source 2 register value from instruction.
**  @param      dst  - Address of destination from instruction.
**
**  @return     none.
**
**  @attention  This routine does nothing but print a message.
**
******************************************************************************
**/

asm(".set _i960_icctl, i960_icctl");
asm(".global _i960_icctl");
void i960_icctl(ulong src1 UNUSED, ulong src2 UNUSED, ulong *dst UNUSED)
{
  write(2, "icctl ", 6);
  write(2, "not decoded yet.", 16);
  write(2, "\n", 1);
}


/**
******************************************************************************
**
**  @brief      i960_sysctl - print that an i960 sysctl instruction was reached.
**
**              Not done yet.
**
**  @param      src1 - Source 1 register value from instruction.
**  @param      src2 - Source 2 register value from instruction.
**  @param      dst  - Address of destination from instruction.
**
**  @return     none.
**
**  @attention  This routine does nothing but print a message.
**
******************************************************************************
**/

asm(".set _i960_sysctl, i960_sysctl");
asm(".global _i960_sysctl");
void i960_sysctl(ulong src1 UNUSED, ulong src2 UNUSED, ulong *dst UNUSED)
{
  write(2, "sysctl ", 7);
  write(2, "not decoded yet.", 16);
  write(2, "\n", 1);
}


/**
******************************************************************************
**
**  @brief      i960_modpc - print that an i960 modpc instruction was reached.
**
**              Not done yet.
**
**  @param      src1 - Source 1 register value from instruction.
**  @param      src2 - Source 2 register value from instruction.
**  @param      dst  - Address of destination from instruction.
**
**  @return     none.
**
**  @attention  This routine does nothing but print a message.
**
******************************************************************************
**/

asm(".set _i960_modpc, i960_modpc");
asm(".global _i960_modpc");
void i960_modpc(ulong src1 UNUSED, ulong src2 UNUSED, ulong *dst UNUSED)
{
  write(2, "modpc ", 6);
  write(2, "not decoded yet.", 16);
  write(2, "\n", 1);
}


/*
******************************************************************************
** Specially placed include file.  This is here to not affect above routines.
******************************************************************************
*/

#include <ctype.h>

/*
******************************************************************************
** Routines that are needed from "c" library, but have an underscore
   tacked on via -fleading-underscore of gcc.
******************************************************************************
*/

int CT_isspace_CT(int);
int CT_strlen_CT(const char *);
int CT_strncmp_CT(char *, char *, unsigned int);
char *CT_strncpy_CT(char *, char *, unsigned int);
void *CT_memcpy_CT(void *, void *, unsigned int);
void *CT_memset_CT(void *, int , unsigned int);
void *CT_memmove_CT(void *, void *, unsigned int);


int CT_isspace_CT(int s) { return(isspace(s)); }
int CT_strlen_CT(const char *s) { return(strlen(s)); }
int CT_strncmp_CT(char *s1, char *s2, unsigned int l) { return(strncmp(s1,s2,l)); }
char *CT_strncpy_CT(char *s1, char *s2, unsigned int l) { return(strncpy(s1,s2,l)); }
void *CT_memcpy_CT(void *d, void *s, unsigned int l) { return(memcpy(d,s,l)); }
void *CT_memset_CT(void *d, int c, unsigned int l) { return(memset(d,c,l)); }
void *CT_memmove_CT(void *d, void *s, unsigned int l) { return(memmove(d,s,l)); }

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
/* End of file CT_defines.h */
