# Start of file .gdbinit
#
# NOTE: Blank lines in macros may cause unexpected things to print.
#
# set $RELEASE = 730
# set $RELEASE = 830
# set $RELEASE = 840
set $RELEASE = 860
#
set history filename ~/.gdb_history
set history save on
set history size 256
#-----------------------------------------------------------------------------
# Set values for maximum of arrays, loops, etc.
set $MAXTARGETS     = *((UINT32 *)&const_list + 0)
set $MAXISP         = *((UINT32 *)&const_list + 1)
set $MAXVIRTUALS    = *((UINT32 *)&const_list + 2)
set $CIMTMAX        = *((UINT32 *)&const_list + 3)
set $LUNMAX         = *((UINT32 *)&const_list + 4)
set $MAXSERVERS     = *((UINT32 *)&const_list + 5)
set $ICIMTMAX       = *((UINT32 *)&const_list + 6)
set $MAXCHN         = *((UINT32 *)&const_list + 7)
set $MAXRAIDS       = *((UINT32 *)&const_list + 8)
set $MAXDEV         = *((UINT32 *)&const_list + 9)
set $VRMAX          = *((UINT32 *)&const_list + 10)
set $MAXDRIVES      = *((UINT32 *)&const_list + 11)
set $MAXIF          = *((UINT32 *)&const_list + 12)
set $MAXCTRL        = *((UINT32 *)&const_list + 13)
set $MAXSES         = *((UINT32 *)&const_list + 14)
set $MAXLID         = *((UINT32 *)&const_list + 15)

set $FE_MEMORY_STARTS = 0x48000000
set $EXECUTABLE_START = (UINT32)&__executable_start
set $LIBRARY_STARTS = (UINT32)&_start
#-----------------------------------------------------------------------------
# The "pp" command sets the print pretty option without typing it out.
define pp
    set print pretty $arg0
end
#...
document pp
The "pp" command will set the print pretty option.
    "ON"    - Turn the print pretty ON
    "OFF"   - Turn the print pretty OFF
end
#-----------------------------------------------------------------------------
# This macro prints out the controllers free memory (860/870 pre/post headers).
define fmmdump
    set $fram=(unsigned int)$arg0
    if ($fram == 0)
      set $fram=(unsigned int)&K_ncdram
      printf "memory free for K_ncdram:\n"
    else
      if ($fram == 1)
        set $fram=(unsigned int)&P_ram
        printf "memory free for P_ram:\n"
      else
        if ($fram == 2)
          set $fram=(unsigned int)&c_wcfmm
          printf "memory free for c_wcfmm:\n"
        else
          printf "memory free for %-8.8x:\n", $fram
        end
      end
    end

    printf "  addr: %-8.8x   mem=%-8.8x size=%-8.8x F=%-8.8x S=%-8.8x st=%-2.2x opt=%-2.2x\n", $fram, *$fram, *($fram+4), *($fram+8), *($fram+12), *((unsigned char *)($fram+16)), *((unsigned char *)($fram+17))

    set $fram=*($fram)
    set $lth = 0

# Now loop through the memory sections.
    set $PRE = (struct before_after *)$fram
    while ($PRE != 0)
      set $lth += $PRE->length + 64
      printf "  addr: %-8.8x %11.11s:%5u u/f=%d next=0x%08lx lth=%9d\n", $PRE, $PRE->str, $PRE->line_number, $PRE->used_or_free, (UINT32)($PRE->next), $PRE->length
      set $ADDRESS = $PRE->next
      if ($ADDRESS != 0)
        set $PRE = (struct before_after *)$ADDRESS - 1
      else
        set $PRE = 0
      end
    end
    printf "length of free list = %u\n", $lth
end
#...
document fmmdump
Print out the controllers free memory in hex (arg=0->K_ncdram, 1->P_ram, 2->c_wcfmm).
Expects a struct fmm address as input.
end
#-----------------------------------------------------------------------------
# This macro prints out the controllers free memory (similar to fmmdump).
define mmfree
    set $fram=(unsigned int)$arg0
    if ($fram == 0)
      set $fram=(struct fmm *)&K_ncdram
      printf "memory free for K_ncdram of initial 0x%x (%d):\n", NcdrSize, NcdrSize
      set $maxmem = NcdrSize
    else
      if ($fram == 1)
        set $fram = (struct fmm *)&P_ram
        printf "memory free for P_ram of initial 0x%x (%d):\n", local_memory_pool_start, local_memory_pool_start
        set $maxmem = local_memory_pool_start
      else
        if ($fram == 2)
          set $fram=(struct fmm *)&c_wcfmm
          printf "memory free for c_wcfmm:\n"
          printf "memory free for c_wcfmm of initial 0x%x (%d):\n", WcbSize, WcbSize
          set $maxmem = WcbSize
        else
          set $fram = (struct fmm *)$fram
          printf "memory free for %-8.8x:\n", $fram
          set $maxmem = 0
        end
      end
    end
    set $max_free = 0
    set $startmem = $fram
# first word is origin of free memory.
# second word is length of segment,
# third word is address of FMS (within II?)
# fourth word is secondary free memory pointer.
# byte of memory wait status
# byte of memory options.

    if ((UINT32)(*$fram) >= $EXECUTABLE_START)
      printf "  addr: %-8.8x mem=%-8.8x size=%-8.8x FMS=%-8.8x S=%-8.8x st=%-2.2x opt=%-2.2x\n", (UINT32)$fram, $fram->fmm_first.thd, $fram->fmm_first.len, $fram->fmm_fms, $fram->fmm_sorg, $fram->fmm_waitstat, $fram->fmm_options
      set $fram_free = $fram->fmm_first.len
      set $max_free = $fram_free
    else
      printf "  addr: %-8.8x mem=%-8.8x -- ends here -- st=%-2.2x opt=%-2.2x\n", $fram, $fram->fmm_first.thd, $fram->fmm_waitstat, $fram->fmm_options
      set $fram_free = 0
    end
    set $fram = $fram->fmm_first.thd

# Now loop through the memory sections.
    set $PRE = (struct before_after *)$fram
    set $DONE = 0
    while ($PRE != 0 && $DONE == 0)
      set $fram_free = $fram_free + $PRE->length + 2 * sizeof(struct before_after)
      if ($max_free < $PRE->length + 2*sizeof(struct before_after))
        set $max_free = $PRE->length + 2*sizeof(struct before_after)
      end
      printf "  addr: %-8.8x %11.11s:%5u u/f=%d length=%-8.8x (%d)\n", $PRE, $PRE->str, $PRE->line_number, $PRE->used_or_free, $PRE->length, $PRE->length
      if ($PRE->next == 0)
        set $DONE = 1
      else
        set $PRE = (struct before_after *)$PRE->next - 1
      end
    end
#
    printf "Free=%x(%d)/max_free=%x(%d)", $fram_free, $fram_free, $max_free, $max_free
    if ($maxmem == 0)
      printf "\n"
    else
      printf " of %x(%d), used %x(%d)\n", $maxmem, $maxmem, $maxmem-$fram_free, $maxmem-$fram_free
    end
#
    if ($startmem == (unsigned int)&K_ncdram)
      printf "K_ncdram: ncCur=0x%x(%d) ncMax=0x%x(%d) ncMin=0x%x(%d) ncWait=%d\n", ((II*)&K_ii)->ncCur, ((II*)&K_ii)->ncCur, ((II*)&K_ii)->ncMax, ((II*)&K_ii)->ncMax, ((II*)&K_ii)->ncMin, ((II*)&K_ii)->ncMin, ((II*)&K_ii)->ncWait
      if (((II*)&K_ii)->ncCur != $fram_free)
        printf "ERROR IN FREE SIZES (calculated=%d, ncCur=%d)\n", $fram_free, ((II*)&K_ii)->ncCur
      end
    end
#
    if ($startmem == (unsigned int)&P_ram)
      printf "P_ram: ncCur=0x%x(%d) ncMax=0x%x(%d) ncMin=0x%x(%d) ncWait=%d\n", P_ram.fmm_fms->fms_Available_memory, P_ram.fmm_fms->fms_Available_memory, P_ram.fmm_fms->fms_Maximum_available, P_ram.fmm_fms->fms_Maximum_available, P_ram.fmm_fms->fms_Minimum_available, P_ram.fmm_fms->fms_Minimum_available, P_ram.fmm_fms->fms_Number_tasks_waiting
      if (P_ram.fmm_fms->fms_Available_memory != $fram_free)
        printf "ERROR IN FREE SIZES (calculated=%d, ncCur=%d)\n", $fram_free, P_ram.fmm_fms->fms_Available_memory
      end
    end
end
#...
document mmfree
Print out the controllers free memory (arg 0 does K_ncdram, 1 does P_ram, 2 does c_wcfmm). (similar to fmmdump)
end
#-----------------------------------------------------------------------------
# This macro prints out the gaps in free memory. (doesn't work)
define mmchkgap
  set $ncgram=$arg0
  if ($ncgram == 0)
    set $ncgram = (unsigned int)&K_ncdram
    set $len = *($ncgram + 4)
    set $next = *$ncgram
  else
    if ($ncgram == 1)
      set $ncgram=(unsigned int)&P_ram
      set $len = *($ncgram + 4)
      set $next = *$ncgram
    else
      set $len = ((struct before_after *)$ncgram)->length + 2*sizeof(struct before_after)
      set $next = ((struct before_after *)$ncgram)->next
    end
  end
  set $gap_size = 0
  set $xcmdaddr = (char *)$ncgram + $len
  set $xcmdlen = (char *)$next - (char *)$xcmdaddr
  if ((UINT32)$next >= $EXECUTABLE_START)
    if (($ncgram + $len) < $next)
      printf "    Gap of %9u (0x%-8.8x) bytes between 0x%-8.8x and 0x%-8.8x\n", $xcmdlen, $xcmdlen,
$xcmdaddr, $next
      set $gap_size = $xcmdlen
          printf "    0x%-8.8x: 0x%-8.8x 0x%-8.8x 0x%-8.8x 0x%-8.8x\n", $xcmdaddr, *($xcmdaddr),
*($xcmdaddr+4), *($xcmdaddr+8), *($xcmdaddr+12)
    else
      if ($xcmdaddr > $next)
        printf "ERROR next %-8.8x < current %-8.8x+length %x (%d)=%-8.8x by %x (%d)\n", $next, $ncgram, $len,
$len, $xcmdaddr, $xcmdaddr-$next, $xcmdaddr-$next
      else
        printf "    NO GAP between 0x%-8.8x and 0x%-8.8x of %u (0x%-8.8x) bytes\n", $xcmdaddr, $next,
$xcmdlen, $xcmdlen
      end
      set $DONE = 1
    end
  else
    printf "  End of free memory just before %-8.8x\n", $xcmdaddr
  end
end
#...
document mmchkgap
Print out the gaps in free memory (arg 0 does K_ncdram, 1 does P_ram). (doesn't work)
end
#-----------------------------------------------------------------------------
# This macro prints out the malloced memory.    doesn't work
define mmalloc
  set $ncram=$arg0
  if ($ncram == 0)
    set $ncram = (unsigned int)&K_ncdram
    set $lastmalloc = NcdrAddr + NcdrSize
    set $firstgap = (*$ncram) - (unsigned int)NcdrAddr
  else
    if ($ncram == 1)
      set $nc  ram = (unsigned int)&P_ram
      set $lastmalloc = (char *)&local_memory_start + local_memory_pool_start
      set $firstgap = (*$ncram) - (unsigned int)&local_memory_start
    else
      set $lastmalloc = 0
    end
  end
  set $mgap_size = 0
  if ($arg0 == 0 || $arg0 == 1)
      mmchkgap *$ncram
      set $mgap_size = $mgap_size + $gap_size
      set $ncram = *($ncram)
  end
  set $DONE = 0
  set $ncram = (struct before_after *)$ncram
  while ($ncram >= $EXECUTABLE_START && $DONE == 0)
      mmchkgap $ncram
      set $mgap_size = $mgap_size + $gap_size
      set $ncram = $ncram->next
  end
  printf "Total gap in free=%u (0x%x)\n", $mgap_size, $mgap_size
  if ((UINT32)$lastmalloc >= $EXECUTABLE_START)
    set $lastgap = $lastmalloc - ($ncram + $ncram->length + 2*sizeof(struct before_after))
    set $totalused = $mgap_size + $lastgap + $firstgap
    printf "Total used=%u (0x%x), first_used=%x, last_used=%x\n", $totalused, $totalused, $firstgap, $lastgap
  end
end
#...
document mmalloc
Print out the malloc-ed memory (arg 0 does K_ncdram, 1 does P_ram). (doesn't work)
end
#-----------------------------------------------------------------------------
# This macro prints out the FE controllers free memory.
define femmfree
  mmfree 0
  printf "-----------------------------------------------------------------------------\n"
  mmfree 1
  printf "-----------------------------------------------------------------------------\n"
  mmfree 2
end
#...
document femmfree
Print out the FE controllers free memory for the three sections.
end
#-----------------------------------------------------------------------------
# This macro prints out the BE controllers free memory.
define bemmfree
  mmfree 0
  printf "-----------------------------------------------------------------------------\n"
  mmfree 1
end
#...
document bemmfree
Print out the BE controllers free memory for the two sections.
end
#-----------------------------------------------------------------------------
# This macro prints out the "g" registers.
define g
  printf "g0  %-8.8x %-8.8x %-8.8x %-8.8x  g4  %-8.8x %-8.8x %-8.8x %-8.8x\n", g0, g1, g2, g3, g4, g5, g6, g7
  printf "g8  %-8.8x %-8.8x %-8.8x %-8.8x  g12 %-8.8x %-8.8x %-8.8x %-8.8x\n", g8, g9, g10, g11, g12, g13, g14, g15
#  printf "g0  0x%-8.8x 0x%-8.8x 0x%-8.8x 0x%-8.8x\n", g0, g1, g2, g3
#  printf "g4  0x%-8.8x 0x%-8.8x 0x%-8.8x 0x%-8.8x\n", g4, g5, g6, g7
#  printf "g8  0x%-8.8x 0x%-8.8x 0x%-8.8x 0x%-8.8x\n", g8, g9, g10, g11
#  printf "g12 0x%-8.8x 0x%-8.8x 0x%-8.8x 0x%-8.8x\n", g12, g13, g14, g15
end
#...
document g
Print the i960 assembler registers g0 through g15 (g15 = fp).
end
#-----------------------------------------------------------------------------
# This macro prints out the "r" registers.  It uses "$RR" for their location.
set $RR = -1
define r
  if ($RR == -1)
    set $RR = (UINT32)fp
  end
  printf "r0  %-8.8x %-8.8x %-8.8x %-8.8x  r4  %-8.8x %-8.8x %-8.8x %-8.8x\n", *($RR), *($RR+4), *($RR+8), *($RR+12), *($RR+16), *($RR+20), *($RR+24), *($RR+28)
  printf "r8  %-8.8x %-8.8x %-8.8x %-8.8x  r12 %-8.8x %-8.8x %-8.8x %-8.8x\n", *($RR+32), *($RR+36), *($RR+40), *($RR+44), *($RR+48), *($RR+52), *($RR+56), *($RR+60)
end
#...
document r
Print the i960 assembler registers r0 through r15 (r0=pfp r1=sp r2=rip) [$RR variable].
end
#-----------------------------------------------------------------------------
define i_printi960stack
  r
  set $PFP = (UINT32)*$RR
  while ($PFP != 0)
    set $SP = (UINT32)*($RR+4)
    i_istack
    set $RIP = (UINT32)*($PFP+8)
    printf "routine @ 0x%x   ", $RIP
    iw $RIP
    set $RR = (UINT32)$PFP
    set $PFP = (UINT32)*($PFP)
    if ((UINT32)$RR >= $EXECUTABLE_START)
      r
    end
  end
end
#...
document i_printi960stack
Internal macro for printing an i960 stack frame.
end
#-----------------------------------------------------------------------------
define i_pcbbt
  set $I = (PCB*)$arg0
  set $J = $I->pc_ebp
  set $S = $I->pc_esp
  set $K = 0
printf "c_frame %d ebp=%p\n", $K, $J
  while ((UINT32)$J >= (UINT32)$I)
    set $K = $K+1
    set $routine = *((UINT32*)$J+1)
    printf "c_frame %d ebp=%p esp=%p routine=", $K, $J, $S
    iw $routine
    if ($J - $S == CT_ebp_diff)
      set $routine = *(UINT32*)$S
      printf "    big routine stack top contents=0x%x  ", $routine
      if ((UINT32)$routine >= $EXECUTABLE_START && (UINT32)$routine <= $LIBRARY_STARTS)
        info symbol $routine
        printf "          "
        iw $routine
      else
        printf "\n"
      end
    end
    # Next %esp and %ebp.
    set $S = $J+8
    set $J = *(UINT32*)$J
  end
end
#...
document i_pcbbt
Internal macro to print a "c" backtrace of a PCB.
end
#-----------------------------------------------------------------------------
define i_pcbibt
  set $I = (PCB*)$arg0
  set $J = $I->pc_gRegs
  printf "g0  %-8.8x %-8.8x %-8.8x %-8.8x  g4  %-8.8x %-8.8x %-8.8x %-8.8x\n", $J[0], $J[1], $J[2], $J[3], $J[4], $J[5], $J[6], $J[7]
  printf "g8  %-8.8x %-8.8x %-8.8x %-8.8x  g12 %-8.8x %-8.8x %-8.8x\n", $J[8], $J[9], $J[10], $J[11], $J[12], $J[13], $J[14]
  set $K = (unsigned int*)$I->pc_pfp
  set $routine = $K[2]
  printf "routine @ 0x%x   ", $routine
  iw $routine
  set $RR = (UINT32)($I->pc_pfp)
  i_printi960stack
end
#...
document i_pcbibt
Internal macro to print an "i960" backtrace of a PCB.
end
#-----------------------------------------------------------------------------
define btp
  set $BTP = (PCB*)$arg0
  printf "-----------------------------------------------------------------------------\n"
  printf "PCB = 0x%08x,  ", $BTP
  printf "Name = %32s,  ", ((PCB*)($BTP))->pc_fork_name
  printf "State = 0x%02x\n", ((PCB*)($BTP))->pc_stat
  i_pcbbt $BTP
  printf ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . -\n"
  i_pcbibt $BTP
end
#...
document btp
Print the passed in pcb address - both the "c" and "i960" backtraces.
end
#-----------------------------------------------------------------------------
# Print the backtraces for all but the current pcb - both the "c" and "i960" backtraces.
define btallpcbs
  set $PCB = (PCB*)K_pcborg
  set $DONE = 0
  while ($DONE != 1)
    btp $PCB
    set $PCB = ((PCB*)($PCB))->pc_thd
    if ($PCB == (PCB*)K_pcborg || $PCB == 0)
      set $DONE = 1
      if ($PCB == 0)
        printf "ERROR, next PCB value is zero\n"
      end
    end
  end
end
#...
document btallpcbs
Print the backtraces for all but the current pcb - both the "c" and "i960" backtraces.
end
#-----------------------------------------------------------------------------
define searchpcbs
  set $VALUE = $arg0
  set $MASK = $arg1
  set $INC = $arg2
  set $SEARCHVAL = $VALUE & $MASK
  printf "Searching all stacks for value (0x%08x:%08x) with mask (0x%08x) index=%d\n", $VALUE, $SEARCHVAL, $MASK, $INC
  set $PCB = (PCB*)K_pcborg
  set $DONE = 0
  set $PCBSIZE = sizeof(PCB)
  while ($DONE != 1)
    set $OFFSET = 0
    printf " Examining PCB = 0x%08x\n", $PCB
    while ($OFFSET < $PCBSIZE)
      if (((*(UINT32*)((char *)$PCB+$OFFSET)) & $MASK) == $SEARCHVAL)
        printf " Found in PCB = 0x%08x, Name = %32s, value=0x%08x\n", $PCB, ((PCB*)($PCB))->pc_fork_name, *(UINT32*)((char *)$PCB+$OFFSET)
        set $OFFSET = $PCBSIZE
      else
        set $OFFSET = $OFFSET + $INC
      end
    end
    set $PCB = ((PCB*)($PCB))->pc_thd
    if ($PCB == (PCB*)K_pcborg || $PCB == 0)
      set $DONE = 1
      if ($PCB == 0)
        printf "ERROR, next PCB value is zero\n"
      end
    end
  end
end
#...
document searchpcbs
Macro to search all PCBs for a specified value and mask.
Example: "searchpcbs 0x67040001 0xffff000f 4" searches all PCBs for the 32 bit
         value 0x6704XXX1 and prints the PCB address(es) where it was found, and
         increment by 4 (sizeof UINT32).
end
#-----------------------------------------------------------------------------
define search_private_memory
  set $VALUE = $arg0
  set $MASK = $arg1
  set $INC = $arg2
  set $SEARCHVAL = $VALUE & $MASK
  printf "Searching private memory for value (0x%08x:%08x) with mask (0x%08x) index=%d\n", $VALUE, $SEARCHVAL, $MASK, $INC
  set $MEM = (char *)&local_memory_start
  set $OFFSET = 0
  set $LIMIT = local_memory_pool_start
  while ($OFFSET < $LIMIT)
    if (((*(UINT32*)($MEM+$OFFSET)) & $MASK) == $SEARCHVAL)
      printf " Found at 0x%08x, value = 0x%08x\n", $MEM+$OFFSET, *(UINT32*)($MEM+$OFFSET)
    end
    set $OFFSET = $OFFSET + $INC
  end
end
#...
document search_private_memory
Macro to search all private memory for a specified value and mask.
Example: "search_private_memory 0x67040001 0xffff000f 4" searches all private memory for the 32 bit
         value 0x6704XXX1 and prints the address(es) where it was found, and
         increment by 4 (sizeof UINT32).
end
#-----------------------------------------------------------------------------
define search_BE_shared_memory
  set $VALUE = $arg0
  set $MASK = $arg1
  set $INC = $arg2
  set $SEARCHVAL = $VALUE & $MASK
  printf "Searching BE shared memory for value (0x%08x:%08x) with mask (0x%08x) index=%d\n", $VALUE, $SEARCHVAL, $MASK, $INC
  set $MEM = (char *)startOfBESharedMem
  set $OFFSET = 0
  set $LIMIT = (char *)endOfBESharedMem - $MEM
  while ($OFFSET < $LIMIT)
    if (((*(UINT32*)($MEM+$OFFSET)) & $MASK) == $SEARCHVAL)
      printf " Found at 0x%08x, value = 0x%08x\n", $MEM+$OFFSET, *(UINT32*)($MEM+$OFFSET)
    end
    set $OFFSET = $OFFSET + $INC
  end
end
#...
document search_BE_shared_memory
Macro to search all BE shared memory for a specified value and mask.
Example: "search_BE_shared_memory 0x67040001 0xffff000f 4" searches all BE shared memory for the 32 bit
         value 0x6704XXX1 and prints the address(es) where it was found, and
         increment by 4 (sizeof UINT32).
end
#-----------------------------------------------------------------------------
define search_FE_shared_memory
  set $VALUE = $arg0
  set $MASK = $arg1
  set $INC = $arg2
  set $SEARCHVAL = $VALUE & $MASK
  printf "Searching FE shared memory for value (0x%08x:%08x) with mask (0x%08x) index=%d\n", $VALUE, $SEARCHVAL, $MASK, $INC
  set $MEM = (char *)$FE_MEMORY_STARTS
  set $OFFSET = 0
  set $LIMIT = (char *)((UINT32)(pCCBSharedMem) & 0xffff0000) - $MEM
  while ($OFFSET < $LIMIT)
    if (((*(UINT32*)($MEM+$OFFSET)) & $MASK) == $SEARCHVAL)
      printf " Found at 0x%08x, value = 0x%08x\n", $MEM+$OFFSET, *(UINT32*)($MEM+$OFFSET)
    end
    set $OFFSET = $OFFSET + $INC
  end
end
#...
document search_FE_shared_memory
Macro to search all FE shared memory for a specified value and mask.
Example: "search_FE_shared_memory 0x67040001 0xffff000f 4" searches all FE shared memory for the 32 bit
         value 0x6704XXX1 and prints the address(es) where it was found, and
         increment by 4 (sizeof UINT32).
end
#-----------------------------------------------------------------------------
define search_CCB_shared_memory
  set $VALUE = $arg0
  set $MASK = $arg1
  set $INC = $arg2
  set $SEARCHVAL = $VALUE & $MASK
  printf "Searching CCB shared memory for value (0x%08x:%08x) with mask (0x%08x) index=%d\n", $VALUE, $SEARCHVAL, $MASK, $INC
  set $MEM = (char *)((UINT32)(pCCBSharedMem) & 0xffff0000)
  set $OFFSET = 0
  set $LIMIT = (char *)startOfBESharedMem - $MEM
  while ($OFFSET < $LIMIT)
    if (((*(UINT32*)($MEM+$OFFSET)) & $MASK) == $SEARCHVAL)
      printf " Found at 0x%08x, value = 0x%08x\n", $MEM+$OFFSET, *(UINT32*)($MEM+$OFFSET)
    end
    set $OFFSET = $OFFSET + $INC
  end
end
#...
document search_CCB_shared_memory
Macro to search all CCB shared memory for a specified value and mask.
Example: "search_CCB_shared_memory 0x67040001 0xffff000f 4" searches all CCB shared memory for the 32 bit
         value 0x6704XXX1 and prints the address(es) where it was found, and
         increment by 4 (sizeof UINT32).
end
#-----------------------------------------------------------------------------
# This does an i960 backtrace (with "r" register printing).
define ebt
  set $RR = (UINT32)(SIGNAL_ERRTRAP::sigUsr1Stack)
  i_printi960stack
  set $RR = (UINT32)fp
end
#...
document ebt
Print the i960 assembler backtrace after an error trap (errtrapf or errtrapb). (See ibt.)
end
#-----------------------------------------------------------------------------
# This does an i960 backtrace (with "r" register printing).
define ibt
  set $RR = (UINT32)fp
  i_printi960stack
  set $RR = (UINT32)fp
end
#...
document ibt
Print the i960 assembler backtrace - including the "r" registers for each frame.
end
#-----------------------------------------------------------------------------
# This prints out the current processes "name".
# pc_CT_fork_name = 80 from f_front.CT.c and b_back.CT.c.
define forkname
  set $FN = ((*(unsigned long *)current_pcb_ptr)+80)
  if (*current_pcb_ptr == 0 || *(UINT32*)$FN == 0)
    printf "current_pcb_ptr=%x\n", current_pcb_ptr
    printf "*current_pcb_ptr=%x\n", *(UINT32*)current_pcb_ptr
    printf "(PCB*)current_pcb_ptr->pc_fork_name=%x\n", *$FN
  else
    printf "Current task name %.32s\n", $FN
  end
end
#...
document forkname
Print the name of the i960 assembler task that is (or last was) running.
end
#-----------------------------------------------------------------------------
# arg0 is how many to print (in reverse).
# arg1 is 0 for only HISTORY_KEEP, 1 for HISTORY_REGS_KEEP
# arg2 is 0 for no TSC printing, 1 for HISTORY_TSC_KEEP.
define i_traceloginternal
  set $I = 1
  set $END = $arg0
  set $CT_TL_strings = sizeof(CT_laststrings) / sizeof(CT_laststrings[0])
  if (CT_laststrings[1] != 0)
    while ($I < $END)
      set $J = (CT_last_history_string-$I+$CT_TL_strings)%$CT_TL_strings
      if ($arg2 == 0)
        printf "tracelog[%-2.2d]= %4d -- %s", $I, CT_count_repeats[$J]+1, CT_tracelog+CT_laststrings[$J]
      else
        printf "tracelog[%-2.2d]= %4d -- %lld: %s", $I, CT_count_repeats[$J]+1, CT_last_tsc[$J], CT_tracelog+CT_laststrings[$J]
      end
      if ($arg1 == 1)
        printf "g0  %-8.8x %-8.8x %-8.8x %-8.8x  g4  %-8.8x %-8.8x %-8.8x %-8.8x\n", CT_save_g[$J][0], CT_save_g[$J][1], CT_save_g[$J][2], CT_save_g[$J][3], CT_save_g[$J][4], CT_save_g[$J][5], CT_save_g[$J][6], CT_save_g[$J][7]
        printf "g8  %-8.8x %-8.8x %-8.8x %-8.8x  g12 %-8.8x %-8.8x %-8.8x %-8.8x\n", CT_save_g[$J][8], CT_save_g[$J][9], CT_save_g[$J][10], CT_save_g[$J][11], CT_save_g[$J][12], CT_save_g[$J][13], CT_save_g[$J][14], CT_save_g[$J][15]
        printf "r0  %-8.8x %-8.8x %-8.8x %-8.8x  r4  %-8.8x %-8.8x %-8.8x %-8.8x\n", CT_save_r[$J][0], CT_save_r[$J][1], CT_save_r[$J][2], CT_save_r[$J][3], CT_save_r[$J][4], CT_save_r[$J][5], CT_save_r[$J][6], CT_save_r[$J][7]
        printf "r8  %-8.8x %-8.8x %-8.8x %-8.8x  r12 %-8.8x %-8.8x %-8.8x %-8.8x\n", CT_save_r[$J][8], CT_save_r[$J][9], CT_save_r[$J][10], CT_save_r[$J][11], CT_save_r[$J][12], CT_save_r[$J][13], CT_save_r[$J][14], CT_save_r[$J][15]
      end
      set $I = $I+1
    end
  end
end
#...
document i_traceloginternal
Internal macro for printing HISTORY_KEEP records.
end
#-----------------------------------------------------------------------------
# This prints out the CT_tracelog - last $arg0 entries.
define tracelog
  i_traceloginternal $arg0 0 0
end
#...
document tracelog
Print the last NNN (argument) CT_tracelog entries (HISTORY_KEEP).
end
#-----------------------------------------------------------------------------
# This prints out the CT_tracelog - all 800000 entries.
define tracelogfull
  set $CT_TL_strings = sizeof(CT_laststrings) / sizeof(CT_laststrings[0])
  i_traceloginternal $CT_TL_strings 0 0
end
#...
document tracelogfull
Print all 800000 CT_tracelog entries (HISTORY_KEEP).
end
#-----------------------------------------------------------------------------
define tracelogreg
  i_traceloginternal $arg0 1 0
end
#...
document tracelogreg
Print NNN (argument) tracelog and the g and r registers at all calls/ret's (HISTORY_REGS_KEEP).
end
#-----------------------------------------------------------------------------
define tracelogregfull
  set $CT_TL_strings = sizeof(CT_laststrings) / sizeof(CT_laststrings[0])
  i_traceloginternal $CT_TL_strings 1 0
end
#...
document tracelogregfull
Print 800000 tracelog and the g and r registers at all calls/ret's (HISTORY_REGS_KEEP).
end
#-----------------------------------------------------------------------------
define tracelogtsc
  i_traceloginternal $arg0 0 1
end
#...
document tracelogtsc
Print NNN (argument) tracelog and time via tsc (HISTORY_KEEP HISTORY_TSC_KEEP).
end
#-----------------------------------------------------------------------------
define tracelogtscfull
  set $CT_TL_strings = sizeof(CT_laststrings) / sizeof(CT_laststrings[0])
  i_traceloginternal $CT_TL_strings 0 1
end
#...
document tracelogtscfull
Print 800000 tracelog and time via tsc (HISTORY_KEEP HISTORY_TSC_KEEP).
end
#-----------------------------------------------------------------------------
define tracelogtscreg
  i_traceloginternal $arg0 1 1
end
#...
document tracelogtscreg
Print NNN (argument) tracelog and the g and r registers at all calls/ret's - and time via tsc (HISTORY_REGS_KEEP HISTORY_TSC_KEEP).
end
#-----------------------------------------------------------------------------
define tracelogtscregfull
  set $CT_TL_strings = sizeof(CT_laststrings) / sizeof(CT_laststrings[0])
  i_traceloginternal $CT_TL_strings 1 1
end
#...
document tracelogtscregfull
Print 800000 tracelog and the g and r registers at all calls/ret's - and time via tsc (HISTORY_REGS_KEEP HISTORY_TSC_KEEP).
end
#-----------------------------------------------------------------------------
# This macro prints out the i960 "stack".  It uses "$SP" for their location.
set $SP = (unsigned int)-1
define i_istack
  if ($SP==(UINT32)-1)
    set $SP = (UINT32)*(fp+4)
    set $RR = (UINT32)fp
  end
  set $ST = (UINT32)$RR+64
  set $CT = 0
  if ($ST < $SP)
    printf "Stack\n"
  end
  while ($ST < $SP)
    if ($CT == 0)
      printf "%-8.8x -", $ST
    end
    printf " %-8.8x", *($ST)
    set $CT = $CT+1
    if ($CT == 8)
      set $CT = 0
      printf "\n"
    end
    set $ST = $ST+4
  end
  if ($CT != 0)
    printf "\n"
  end
end
#...
document i_istack
Print the i960 assembler stack (variables $SP and $RR).
end
#-----------------------------------------------------------------------------
define errstack
  set $RR = (UINT32)fp
  set $SP = (UINT32)*($RR+4)
  printf "error.as registers from stack\n"
  printf "g0  %-8.8x %-8.8x %-8.8x %-8.8x  g4  %-8.8x %-8.8x %-8.8x %-8.8x\n", *($SP-0xa0), *($SP-0xa0+4), *($SP-0xa0+8), *($SP-0xa0+12), *($SP-0x90), *($SP-0x90+4), *($SP-0x90+8), *($SP-0x90+12)
  printf "g8  %-8.8x %-8.8x %-8.8x %-8.8x  g12 %-8.8x %-8.8x %-8.8x %-8.8x\n", *($SP-0x80), *($SP-0x80+4), *($SP-0x80+8), *($SP-0x80+12), *($SP-0x70), *($SP-0x70+4), *($SP-0x70+8), *($SP-0x70+12)
  printf "r0  %-8.8x %-8.8x %-8.8x %-8.8x  r4  %-8.8x %-8.8x %-8.8x %-8.8x\n", *($SP-0x60), *($SP-0x60+4), *($SP-0x60+8), *($SP-0x60+12), *($SP-0x50), *($SP-0x50+4), *($SP-0x50+8), *($SP-0x50+12)
  printf "r8  %-8.8x %-8.8x %-8.8x %-8.8x  r12 %-8.8x %-8.8x %-8.8x %-8.8x\n", *($SP-0x40), *($SP-0x40+4), *($SP-0x40+8), *($SP-0x40+12), *($SP-0x30), *($SP-0x30+4), *($SP-0x30+8), *($SP-0x30+12)
end
#...
document errstack
Print the i960 assembler as if errtrap saved registers on it (fp).
end
#-----------------------------------------------------------------------------
define i_printalpha
  set $PA = *(unsigned char *)$arg0
  if ($PA < 0x20 || $PA >= 0x7f)
    printf "?"
  else
    printf "%c", *(char *)($arg0)
  end
end
#...
document i_printalpha
Internal macro for errtrapF and errtrapB (print 1 alpha character or a ?).
end
#-----------------------------------------------------------------------------
define i_printerrtrapdata
  printf "error code=%-8.8x (%d)  base ATU=%-8.8x  ", *($RR), *($RR), *($RR+0x04)
  printf "Firmware revision="
  i_printalpha ($RR+0x08)
  i_printalpha ($RR+0x09)
  i_printalpha ($RR+0x0a)
  i_printalpha ($RR+0x0b)
  printf "  revcount="
  i_printalpha ($RR+0x0c)
  i_printalpha ($RR+0x0d)
  i_printalpha ($RR+0x0e)
  i_printalpha ($RR+0x0f)
  printf "\n"
  printf "pfp=%-8.8x sp=%-8.8x rip=%-8.8x  r3=%-8.8x  r4=%-8.8x  r5=%-8.8x  r6=%-8.8x  r7=%-8.8x\n", *($RR+0x10), *($RR+0x14), *($RR+0x18), *($RR+0x1c), *($RR+0x20), *($RR+0x24), *($RR+0x28), *($RR+0x2c)
  printf " r8=%-8.8x r9=%-8.8x r10=%-8.8x r11=%-8.8x r12=%-8.8x r13=%-8.8x r14=%-8.8x r15=%-8.8x\n", *($RR+0x30), *($RR+0x34), *($RR+0x38), *($RR+0x3c), *($RR+0x40), *($RR+0x44), *($RR+0x48), *($RR+0x4c)
  printf " g0=%-8.8x g1=%-8.8x  g2=%-8.8x  g3=%-8.8x  g4=%-8.8x  g5=%-8.8x  g6=%-8.8x  g7=%-8.8x\n", *($RR+0x50), *($RR+0x54), *($RR+0x58), *($RR+0x5c), *($RR+0x60), *($RR+0x64), *($RR+0x68), *($RR+0x6c)
  printf " g8=%-8.8x g9=%-8.8x g10=%-8.8x g11=%-8.8x g12=%-8.8x g13=%-8.8x g14=%-8.8x g15=%-8.8x\n", *($RR+0x70), *($RR+0x74), *($RR+0x78), *($RR+0x7c), *($RR+0x80), *($RR+0x84), *($RR+0x88), *($RR+0x8c)
  printf "Last CCB heartbeat: 0x%-8.8x, %-8.8x, %-8.8x\n", *($RR+0x94), *($RR+0x98), *($RR+0x9c)
  set $RIP = (UINT32)*($RR+0x18)
  printf "\nroutine @ 0x%x   ", $RIP
  if ($RIP >= $LIBRARY_STARTS || $RIP < $EXECUTABLE_START)
    printf "Not in Assembler section -- error!\n"
  else
    iw $RIP
  end
end
#...
document i_printerrtrapdata
Internal macro for errtrapF and errtrapB.
end
#-----------------------------------------------------------------------------
define errtrap
  set $RR = (UINT32)errTrapAddr
  printf "TRAPADDR %p\n", (void *)$RR
  i_printerrtrapdata
end
#...
document errtrap
Print the TRAPADDR information.
end
#-----------------------------------------------------------------------------
define errtrapfe
  printf "THIS IS NOT VALID SINCE 2007\n"
  set $RR = (UINT32)0x48010000
  printf "FE TRAPADDR, errTrapAddr assumed at 0x48010000\n"
  i_printerrtrapdata
end
#...
document errtrapfe
OBSOLETE - Print the FE TRAPADDR information - assuming it as at location 0x48010000.
end
#...
define errtrapF
  set $RR = (UINT32)errTrapAddr
  printf "FE TRAPADDR\n"
  i_printerrtrapdata
end
#...
document errtrapF
Print the FE TRAPADDR information.
end
#-----------------------------------------------------------------------------
define errtrapB
  set $RR = (UINT32)errTrapAddr
  printf "BE TRAPADDR\n"
  i_printerrtrapdata
end
#...
document errtrapB
Print the BE TRAPADDR information.
end
#-----------------------------------------------------------------------------
define i_printpcbstatetext
    set $STATE_value = $arg0
    if ($STATE_value == 0x00)
      printf "Ready\n"
    else
      if ($STATE_value == 0x01)
      printf "Not ready\n"
      else
      if ($STATE_value == 0x02)
      printf "WAIT SRAM\n"
      else
      if ($STATE_value == 0x03)
      printf "WAIT CACHEABLE\n"
      else
      if ($STATE_value == 0x04)
      printf "WAIT NONCACHEABLE\n"
      else
      if ($STATE_value == 0x05)
      printf "WAIT REMOTE\n"
      else
      if ($STATE_value == 0x06)
      printf "Timed wait\n"
      else
      if ($STATE_value == 0x07)
      printf "21MS TIMER WAIT\n"
      else
      if ($STATE_value == 0x08)
      printf "WAIT MSG0\n"
      else
      if ($STATE_value == 0x09)
      printf "WAIT MSG1\n"
      else
      if ($STATE_value == 0x0A)
      printf "WAIT BELL0\n"
      else
      if ($STATE_value == 0x0B)
      printf "WAIT BELL1\n"
      else
      if ($STATE_value == 0x0C)
      printf "WAIT BELL2\n"
      else
      if ($STATE_value == 0x0D)
      printf "WAIT BELL3\n"
      else
      if ($STATE_value == 0x0E)
      printf "WAIT IO\n"
      else
      if ($STATE_value == 0x0F)
      printf "WAIT NVA\n"
      else
      if ($STATE_value == 0x10)
      printf "HOST RESET WAIT\n"
      else
      if ($STATE_value == 0x11)
      printf "WAIT SEM1\n"
      else
      if ($STATE_value == 0x12)
      printf "WAIT SEM2\n"
      else
      if ($STATE_value == 0x13)
      printf "Scsi Reset Wait\n"
      else
      if ($STATE_value == 0x14)
      printf "WAIT BLK LOCK\n"
      else
      if ($STATE_value == 0x15)
      printf "WAIT COPY LOCK\n"
      else
      if ($STATE_value == 0x16)
      printf "WAIT DMA0\n"
      else
      if ($STATE_value == 0x17)
      printf "WAIT DMA1\n"
      else
      if ($STATE_value == 0x18)
      printf "ISP wait\n"
      else
      if ($STATE_value == 0x19)
      printf "WAIT NONCACHE WBUF\n"
      else
      if ($STATE_value == 0x1A)
      printf "WAIT CACHE WBUF\n"
      else
      if ($STATE_value == 0x1B)
      printf "WAIT FE BE MRP\n"
      else
      if ($STATE_value == 0x1C)
      printf "FC READY WAIT\n"
      else
      if ($STATE_value == 0x1D)
      printf "ONLINE WAIT\n"
      else
      if ($STATE_value == 0x1E)
      printf "WAIT WRT CACHE\n"
      else
      if ($STATE_value == 0x1F)
      printf "WAIT LINK 0\n"
      else
      if ($STATE_value == 0x20)
      printf "WAIT LINK 1\n"
      else
      if ($STATE_value == 0x21)
      printf "WAIT LINK 2\n"
      else
      if ($STATE_value == 0x22)
      printf "WAIT LINK 3\n"
      else
      if ($STATE_value == 0x23)
      printf "WAIT LINK 4\n"
      else
      if ($STATE_value == 0x24)
      printf "WAIT LINK 5\n"
      else
      if ($STATE_value == 0x25)
      printf "WAIT LINK 6\n"
      else
      if ($STATE_value == 0x26)
      printf "WAIT LINK 7\n"
      else
      if ($STATE_value == 0x27)
      printf "QLOGIC WAIT 0\n"
      else
      if ($STATE_value == 0x28)
      printf "QLOGIC WAIT 1\n"
      else
      if ($STATE_value == 0x29)
      printf "QLOGIC WAIT 2\n"
      else
      if ($STATE_value == 0x2A)
      printf "QLOGIC WAIT 3\n"
      else
      if ($STATE_value == 0x2B)
      printf "WAIT MRP\n"
      else
      if ($STATE_value == 0x2C)
      printf "WAIT SYNC NVA\n"
      else
      if ($STATE_value == 0x2D)
      printf "WAIT RAID ERROR\n"
      else
      if ($STATE_value == 0x2E)
      printf "FILE SYS CLEANUP\n"
      else
      if ($STATE_value == 0x2F)
      printf "WAIT RAID INIT\n"
      else
      if ($STATE_value == 0x30)
      printf "WAIT i82559\n"
      else
      if ($STATE_value == 0x31)
      printf "IPC WAIT\n"
      else
      if ($STATE_value == 0x32)
      printf "EVENT WAIT\n"
      else
      if ($STATE_value == 0x33)
      printf "Wait linux targ in\n"
      else
      if ($STATE_value == 0x34)
      printf "Wait linux comp in\n"
      else
      if ($STATE_value == 0x35)
      printf "WAIT P2 NVRAM\n"
      else
      if ($STATE_value == 0x36)
      printf "MM WAIT\n"
      else
      if ($STATE_value == 0x37)
      printf "NV DMA QFULL WAIT\n"
      else
      if ($STATE_value == 0x38)
      printf "NV DMA EXEC WAIT\n"
      else
      if ($STATE_value == 0x39)
      printf "NV DMA COMP WAIT\n"
      else
      if ($STATE_value == 0x3A)
      printf "NV SCRUB WAIT\n"
      else
      if ($STATE_value == 0x3B)
      printf "SCSI I/O WAIT\n"
      else
      if ($STATE_value == 0x3C)
      printf "EPOLL WAIT\n"
      else
      if ($STATE_value == 0x3D)
      printf "SS SSMS READ\n"
      else
      if ($STATE_value == 0x3E)
      printf "SS R OGER READS\n"
      else
      if ($STATE_value == 0x3F)
      printf "SS SSMS MORE TASKS\n"
      else
      if ($STATE_value == 0x40)
      printf "IOCB WAIT\n"
      else
      if ($STATE_value == 0x41)
      printf "MB WAIT PORT 0\n"
      else
      if ($STATE_value == 0x42)
      printf "MB WAIT PORT 1\n"
      else
      if ($STATE_value == 0x43)
      printf "MB WAIT PORT 2\n"
      else
      if ($STATE_value == 0x44)
      printf "MB WAIT PORT 3\n"
      else
      printf "??\n"
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
      end
    end
end
#-----------------------------------------------------------------------------
# This prints the states of all the PCBs in the FE or BE.
#
define pcbstate
  set $PCB = (PCB*)K_pcborg
  set $DONE = 0
  set $I = 0
  while ($DONE != 1)
    set $I = $I + 1
    if ($PCB == (PCB*)K_xpcb)
      printf "*"
    else
      printf " "
    end
    printf "%2.2d PCB = 0x%08x,  ", $I, $PCB
    printf "Name = %32s,  ", ((PCB*)($PCB))->pc_fork_name
    set $STATE = ((PCB*)($PCB))->pc_stat
    printf "State = 0x%02x ", $STATE
    i_printpcbstatetext $STATE
    set $PCB = ((PCB*)($PCB))->pc_thd
    if ($PCB == (PCB*)K_pcborg || $PCB == 0)
      set $DONE = 1
      if ($PCB == 0)
        printf "ERROR, next PCB value is zero\n"
      end
    end
  end
end
#...
document pcbstate
Print the state of the PCB queues in the PROC.
end
#-----------------------------------------------------------------------------
# Added for K010 (2007/12/5) -- This prints out the last 64 tasks to exchange.
#
define lastpcbs
  set $PCBptr = (PCB **)&k_last_pcbs
  set $FORKNAME = (char *)&last_pc_fork_name
  set $LAST_RUNTIME = (UINT64 *)&last_runtime
  set $NumberLastPcbs = (UINT32)($PCBptr[-1]) -1
  if ($NumberLastPcbs == &CAWRAP_LL_TargetTaskCompletion || $NumberLastPcbs > 4096)
    set $NumberLastPcbs = 64 - 1
  end
  set $I = 0
  while ($I < $NumberLastPcbs)
    set $J = $PCBptr[$I]
    set $R = $LAST_RUNTIME[$I] - $LAST_RUNTIME[$I+1]
    set $N = $FORKNAME + $I*32
    printf "%2.2d %9llu 0x%08x", $I, $R, $J
    if ($J != 0)
      printf " %-32.32s, ", $N
      set $STATE = $J->pc_stat
      printf "State=0x%02x ", $STATE
      i_printpcbstatetext $STATE
    else
      printf "\n"
    end
    set $I = $I + 1
  end
  set $J = $PCBptr[$I]
  set $N = $FORKNAME + $I*32
  printf "%2.2d ......... 0x%08x", $I, $J
  if ($J != 0)
    printf " %-32.32s, ", $N
    set $STATE = $J->pc_stat
    printf "State=0x%02x ", $STATE
    i_printpcbstatetext $STATE
  else
    printf "\n"
  end
end
#...
document lastpcbs
Print the last 64 PCBs to enter exchange (or to run).
end
#-----------------------------------------------------------------------------
# This dumps the TAR array _tar [physical.c].
#
define dumpTAR
  set $I = 0
  while ($I < 4)
    set $TAR = tar[$I]
    printf "tar[%d] = 0x%08x\n", $I, $TAR
    while ((UINT32)$TAR >= $EXECUTABLE_START)
      printf "  fthd=0x%08x tid=%u entry=%u opt=%u tsih=%x hardID=%u\n", ((TAR*)($TAR))->fthd, ((TAR*)($TAR))->tid, ((TAR*)($TAR))->entry, ((TAR*)($TAR))->opt, ((TAR*)($TAR))->tsih, ((TAR*)($TAR))->hardID
      printf "  nodeName=0x%016x flags=0x%8.8x vpID=0x%08x portID=0x%08x\n", ((TAR*)($TAR))->nodeName, ((TAR*)($TAR))->flags, ((TAR*)($TAR))->vpID, ((TAR*)($TAR))->portID
      set $TAR = ((TAR*)($TAR))->fthd
    end
    set $I = $I + 1
  end
end
#...
document dumpTAR
Dump the TAR list _tar.
end
#-----------------------------------------------------------------------------
# This dumps the CIMT list cimtDir [debug.c].
#
define dumpcimtDir
  set $I = 0
  while ($I < $CIMTMAX)
    printf "cimtDir[%d]=0x%08x\n", $I, ((CIMT**)(cimtDir))[$I]
    set $CIMT = ((CIMT**)(cimtDir))[$I]
    if ((UINT32)$CIMT >= $EXECUTABLE_START)
      printf "  imtHead=0x%08x imtTail=0x%08x, eHand=0x%08x\n", $CIMT->imtHead, $CIMT->imtTail, $CIMT->eHand
      printf "  num=0x%02x state=0x%02x iState=0x%02x tFlag=0x%04x defTFlag=0x%04x\n", $CIMT->num, $CIMT->state, $CIMT->iState, $CIMT->tFlag, $CIMT->defTFlag
      printf "  curTr=0x%08x begTr=0x%08x endTr=0x%08x\n", $CIMT->curTr, $CIMT->begTr, $CIMT->endTr
      printf "  ltmtHead=0x%08x ltmtTail=0x%08x numHosts=0x%04x\n", $CIMT->ltmtHead, $CIMT->ltmtTail, $CIMT->numHosts
#      dumpimt $CIMT->imtHead $CIMT->imtTail
#      dumpltmt $CIMT->ltmtHead $CIMT->ltmtTail
      set $TR = (UINT32)$CIMT->curTr
      set $J = 0
      printf "  Listing most current possible 4096 tracebuffer non-zero entries\n"
      if ($I == 0)
        printf "  trt_XL$recv       1 Ch EXID  CMD PCBs T-ID  LUN  INIT-ID TIMESTAMP\n"
        printf "  trt_c$cdb4        2 Ch EXID     CDB1     CDB2     CDB3\n"
        printf "  trt_c$imno4       3 Ch EXID    FLAGS  STATUS     INIT-ID TIMESTAMP\n"
        printf "  trt_c$offl4       4 Ch EXID                              TIMESTAMP\n"
        printf "  trt_MAGsubv       5 Ch EXID   VRP#   VID#   LUN   Length TIMESTAMP\n"
        printf "  trt_srpreq        6 Ch EXID SRPc PCBs       LUN  INIT-ID TIMESTAMP\n"
        printf "  trt_srpcomp       7 Ch EXID SRPc PCBs       LUN  INIT-ID TIMESTAMP\n"
        printf "  trt_MAGcomp       8 Ch EXID VRPstatus       LUN  INIT-ID TIMESTAMP\n"
        printf "  trt_ISP$recv      9 Ch EXID  CMD STAT FCAL  LUN  INIT-ID TIMESTAMP\n"
        printf "  trt_mag1iocr     10 Ch EXID CMPL STAT       LUN  INIT-ID TIMESTAMP\n"
        printf "  trt_data         14   Length Twelve bytes of data\n"
        printf "  trt_SENSE        15 Fifteen Bytes of Sense Data\n"
      end
      while ($TR != 0)
        set $TR = $TR - 16
        if ($TR < (UINT32)$CIMT->begTr)
          set $TR = (UINT32)$CIMT->endTr
        end
        if ($TR == (UINT32)$CIMT->curTr)
          set $TR = 0
        else
          set $J = $J + 1
          set $TRtype = *(unsigned char*)($TR)
          set $TRchip = *(unsigned char*)($TR + 1)
          set $TRid = *(unsigned short*)($TR + 2)
          set $T1 = *(UINT32*)($TR)
          set $T2 = *(UINT32*)($TR + 4)
          set $T3 = *(UINT32*)($TR + 8)
          set $T4 = *(UINT32*)($TR + 12)
          if ($T1 == 0 && $T2 == 0 && $T3 == 0 && $T4 == 0)
            set $TR = 0
          else
            printf "  %4d 0x%8.8x  %2d %2d %4.4x ", $J, $TR, $TRtype, $TRchip, $TRid
            if ($TRtype == 1)
              set $TRcmd = *(unsigned char*)($TR + 4)
              set $TRPCBs = *(unsigned char*)($TR + 5)
              set $TRtid = *(unsigned char*)($TR + 6)
              set $TRlun = *(unsigned char*)($TR + 7)
              printf "0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x %8.8x %8.8x\n", $TRcmd,$TRPCBs,$TRtid, $TRlun, $T3, $T4
            else
            if ($TRtype == 2)
              printf "%8.8x %8.8x %8.8x\n", $T2, $T3, $T4
            else
            if ($TRtype == 3)
              set $TRflags = *(unsigned short*)($TR + 4)
              set $TRstatus = *(unsigned short*)($TR + 6)
              printf "%4.4x %4.4x    %8.8x %8.8x\n", $TRflags,$TRstatus, $T3, $T4
            else
            if ($TRtype == 4)
              printf "                             %8.8x\n", $T4
            else
            if ($TRtype == 5)
              set $TRvrpcode = *(unsigned short*)($TR + 4)
# Argh, store a short, then overwrite 1/2 of it with the next.
              set $TRvid = *(unsigned char*)($TR + 6) & 0xff00
              set $TRlun = *(unsigned char*)($TR + 7)
              printf "0x%4.4x 0x%4.4x  0x%2.2x %8.8x %8.8x\n", $TRvrpcode, $TRvid, $TRlun, $T3, $T4
            else
            if ($TRtype == 6)
              set $TRsrpc = *(unsigned char*)($TR + 4)
              set $TRPCBs = *(unsigned char*)($TR + 5)
              set $TRlun = *(unsigned short*)($TR + 6)
              printf "0x%2.2x 0x%2.2x    0x%4.4x %8.8x %8.8x\n", $T1, $TRPCBs, $TRlun, $T3, $T4
            else
            if ($TRtype == 7)
              set $TRsrpcmplstat = *(unsigned char*)($TR + 4)
              set $TRpcbstate = *(unsigned char*)($TR + 5)
              set $TRlun = *(unsigned short*)($TR + 6)
              printf "0x%2.2x 0x%2.2x    0x%4.4x %8.8x %8.8x\n", $TRsrpcmplstat,$TRpcbstate,$TRlun, $T3, $T4
            else
            if ($TRtype == 8)
              set $TRvrpstatus = *(unsigned short*)($TR + 4)
              set $TRlun = *(unsigned short*)($TR + 6)
              printf "0x%4.4x       0x%4.4x %8.8x %8.8x\n", $TRvrpstatus,$TRlun, $T3, $T4
            else
            if ($TRtype == 9)
              set $TRcmd = *(unsigned char*)($TR + 4)
              set $TRstat = *(unsigned char*)($TR + 5)
              set $TRfcal = *(unsigned char*)($TR + 6)
              set $TRlun = *(unsigned char*)($TR + 7)
              printf "0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x %8.8x %8.8x\n", $TRcmd,$TRstat,$TRfcal, $TRlun, $T3, $T4
            else
            if ($TRtype == 10)
              set $TRcmpl = *(unsigned char*)($TR + 4)
              set $TRstat = *(unsigned char*)($TR + 5)
# unused       set $TRfcal = *(unsigned char*)($TR + 6)
              set $TRlun = *(unsigned char*)($TR + 7)
              printf "0x%2.2x 0x%2.2x      0x%2.2x %8.8x %8.8x\n", $TRcmpl,$TRstat, $TRlun, $T3, $T4
            else
            if ($TRtype == 14)
              set $TRlth1 = *(unsigned char*)($TR + 1)
              set $TRlth2 = *(unsigned char*)($TR + 2)
              set $TRlth3 = *(unsigned char*)($TR + 3)
# This might be exactly backwards, won't know until I see one in the trace log.
              set $TRlth = $THlth1 << 16 | $THlth2 << 8 | $THlth3
              printf "%8d %8.8x %8.8x %8.8x\n", $TRlth, $TR2, $T3, $T4
            else
            if ($TRtype == 15)
              set $TRsd1 = *(unsigned char*)($TR + 1)
              set $TRsd2 = *(unsigned char*)($TR + 2)
              set $TRsd3 = *(unsigned char*)($TR + 3)
              printf "%2.2x%2.2x%2.2x %8.8x %8.8x %8.8x\n", $TRsd1,$TRsd2,$TRsd3, $T2, $T3, $T4
            else
              printf "?? %8.8x %8.8x %8.8x\n", $T2, $T3, $T4
            end
            end
            end
            end
            end
            end
            end
            end
            end
            end
            end
            end
          end
        end
      end
    end
    set $I = $I + 1
  end
end
#...
document dumpcimtDir
Dump the CIMT list cimtDir.
end
#-----------------------------------------------------------------------------
# This traverses an ILT list starting at the input parm and stopping when it
# gets it again.
#
define ilttraverse
  set $ILT = $arg0
  set $DONE = 0
  set $COUNT = 0
  while ($DONE != 1 && $ILT != 0)
    set $COUNT = $COUNT + 1
    printf "%4d ILT = 0x%08x  fwdth = 0x%08x  bkth  = 0x%08x\n", $COUNT, $ILT, ((ILT*)($ILT))->fthd, ((ILT*)($ILT))->bthd
    set $ILT = ((ILT*)($ILT))->fthd
    if ($ILT == $arg0 || $ILT == 0)
      set $DONE = 1
      if ($ILT == 0)
        printf "ERROR, next ILT pointer is zero\n"
      end
    end
  end
end
#...
document ilttraverse
Traverse ILT's in the PROC (pass in ilt as argument).
end
#-----------------------------------------------------------------------------
# This traverses an SYNC record list starting at the input parm and stopping when it
# gets it again.
#
define synctraverse
  set $SYN = $arg0
  set $DONE = 0
  set $COUNT = 1
  while ($DONE != 1)
    printf "%05x  Sync=%08x,  ", $COUNT, $SYN
    printf "deplst=%08x  ", ((SYNC*)($SYN))->syn_deplst
    printf "acclst=%08x  ", ((SYNC*)($SYN))->syn_acclst
    printf "segnum=%08x  ", ((SYNC*)($SYN))->syn_segnum
    printf "iscnt=%08x  ", ((SYNC*)($SYN))->syn_iscnt
    printf "master=%08x  ", ((SYNC*)($SYN))->syn_master
    printf "state=%02x\n", ((SYNC*)($SYN))->syn_state
    set $SYN = ((SYNC*)($SYN))->syn_link
    set $COUNT = $COUNT + 1
    if ($SYN == $arg0 || $SYN == 0)
      set $DONE = 1
      if ($SYN == 0)
        printf "End of list, next SYNC pointer is zero\n"
      end
    end
  end
end
#...
document synctraverse
Traverse sync records in the PROC BE (pass in SYNC as argument).
end
#-----------------------------------------------------------------------------
# This traverses the run queue.
#
define xkpcbstate
  set $PCB = xkRunQueue.tskPtr
  set $DONE = 0
  while ($DONE != 1)
    printf "PCB = 0x%08x,  ", $PCB
    printf "Function = 0x%08x,  ", ((XK_PCB*)($PCB))->functionPtr
    printf "State = 0x%02x,  ", ((XK_PCB*)($PCB))->pcb.pc_stat
    printf "pc_pfp 0x%08x, ", ((XK_PCB*)($PCB))->pcb.pc_pfp
    info symbol ((XK_PCB*)($PCB))->functionPtr
    set $PCB = ((XK_PCB*)($PCB))->next
    if ($PCB == xkRunQueue.tskPtr || $PCB == 0)
      set $DONE = 1
      if ($PCB == 0)
        printf "ERROR, next PCB value is zero\n"
      end
    end
  end
end
#...
document xkpcbstate
Print xkRunQueue.tskPtr queue in the CCB.
end
#-----------------------------------------------------------------------------
# Do a pbt with the current pcb pointer as input.
define cpbt
  pbt *current_pcb_ptr
end
#...
document cpbt
Prints the i960 assembler backtrace, for the current PCB.
end
#-----------------------------------------------------------------------------
# Dumps the memory queue.  Enter the anchor as arg0.
define tracemem
  set $I = $arg0
  printf "Buf Addr   Buf Len  NextBuf@\n"
  printf "--------  --------  --------\n"
  while ((UINT32)$I >= $EXECUTABLE_START)
    printf "%8x  %8x  %8x\n", $I, *($I+4), *($I)
    set $I = *($I)
  end
end
#...
document tracemem
Dumps the mem queue.  Enter the anchor (i.e. K_cdram) as arg0.
end
#-----------------------------------------------------------------------------
# This macro prints the vdisks information given the specified format.
define i_vdisks
    set $DSPTYPE = $arg0
    if ($DSPTYPE == 0)
        printf "  VID  Status        NAME        RID\n"
        printf "  ---  ------  ----------------  ---\n"
    end
    if ($DSPTYPE == 1)
        printf " VID  DEVSTAT     CAPACITY     MIRROR   ATTR   RAIDCNT  DRAIDCNT  PCT COMP  OWNER    NAME\n"
        printf " ---  -------  --------------  ------  ------  -------  --------  --------  -----  --------\n"
    end
    set $I = 0
    while ($I < $MAXVIRTUALS)
        if ((*(VDX*)&gVDX).vdd[$I] > 0)
            if ($DSPTYPE == 0)
                printf " %3hu",         gVDX.vdd[$I].vid
                printf "     0x%02x",   gVDX.vdd[$I].status
                printf "  %16s",        gVDX.vdd[$I].name
                set $J = 0
                set $RDD = gVDX.vdd[$I]->pRDD
                printf "  "
                while ($RDD > 0)
                    if ($J > 0)
                        printf ","
                    end
                    printf "%hu", $RDD->rid
                    set $J = $J + 1
                    set $RDD = $RDD->pNRDD
                end
            end
            if ($DSPTYPE == 1)
                printf " %3hu",         gVDX.vdd[$I].vid
                printf "     0x%02x",   gVDX.vdd[$I].status
                printf "  %14llu",      gVDX.vdd[$I].devCap
                printf "    0x%2.2x",   gVDX.vdd[$I].mirror
                printf "  0x%4.4x",     gVDX.vdd[$I].attr
                printf "  %7u",         gVDX.vdd[$I].raidCnt
                printf "  %8u",         gVDX.vdd[$I].draidCnt
                printf "  %8u",         gVDX.vdd[$I].scpComp
                printf "    0x%2.2x",   gVDX.vdd[$I].owner
                printf "  %.16s",       gVDX.vdd[$I].name
            end
            printf "\n"
        end
        set $I = $I + 1
    end
end
#...
document i_vdisks
Print the vdisks information given the specified format.
    0 - VDisk status information
    1 - VDisk standard information
end
#-----------------------------------------------------------------------------
# This macro prints the vdisks status information.
define vdiskstatus
    i_vdisks 0
end
#...
document vdiskstatus
Print the vdisks information in short format (vdisks 0).
end
#-----------------------------------------------------------------------------
# This macro prints the vdisks in standard format.
define vdiskstd
    i_vdisks 1
end
#...
document vdiskstd
Print the vdisks information in standard format (vdisks 1).
end
#-----------------------------------------------------------------------------
# This macro dumps the iddinfo for given port.
define iddinfo
    printf "iddinfo for port %d\n", $arg0
    printf "icimt:0x%8.8x ",(UINT32)I_CIMT_dir[$arg0]
    printf "port:%02d ",I_CIMT_dir[$arg0]->chpid
    printf "lid:0x%04x ",I_CIMT_dir[$arg0]->mylid
    printf "pid:0x%06x ",I_CIMT_dir[$arg0]->mypid
    printf "state:%d\n", I_CIMT_dir[$arg0]->state
    set $pTMT = (TMT *)I_CIMT_dir[$arg0]->tmtQ

    set $tmtbanner = 1
    while ($pTMT > 0)
        if ($tmtbanner > 0)
            printf "  tmt        port    lid     alpa             p_name            n_name       state    ltmt       imt\n"
            printf "-----------------------------------------------------------------------------------------------------\n"
            set $tmtbanner = 0
        end
        printtmt $pTMT
        set $pTMT = $pTMT->tmtLink
    end
end
#...
document iddinfo
Dumps the iddinfo for the given port.
end
#-----------------------------------------------------------------------------
# This macro prints the tmt.
define printtmt
    set $pTMT = $arg0
    set $pLTMT = (LTMT *)$pTMT->ltmt
    printf " 0x%8.8x : ",$pTMT
    printf " %1d ",$pTMT->chipID
    printf " 0x%04x ",$pTMT->lid
    printf " 0x%08x ",$pTMT->alpa
    printf " 0x%08x%08x ", ($pTMT->P_name >> 32), $pTMT->P_name
    printf " 0x%08x%08x ", ($pTMT->N_name >> 32), $pTMT->N_name
    printf " %2d ",$pTMT->state
    printf " 0x%8.8x ",$pTMT->ltmt
    if ($pTMT->ltmt > 0)
        printf " 0x%8.8x ",$pTMT->ltmt->pIMT
    else
        printf "0x00000000 "
    end
    printf "\n"

    set $tlmtbanner = 1
    set $I = 0
    while ($I < $LUNMAX)
        if ($pTMT->tlmtDir[$I] > 0)
            if ($tlmtbanner > 0)
                printf "\t     tlmt        tmt        cimt        whead      wtail       ahead      wtail     lun       misc\n"
                printf "\t----------------------------------------------------------------------------------------------------------\n"
                set $tlmtbanner = 0
            end
            printf "\t"
            printtlmt $pTMT->tlmtDir[$I]
        end
        set $I = $I + 1
    end
end
#...
document printtmt
Prints the tmt.
end
#-----------------------------------------------------------------------------
# This macro prints the list of free tmts.
define printfreetmt
    set $tmtbanner = 1
    set $pTMT = (TMT *)i_sp_tmts
    while ($pTMT > 0)
        if ($tmtbanner > 0)
            printf "  tmt        port    lid     alpa             p_name            n_name       state    ltmt       imt\n"
            printf "-----------------------------------------------------------------------------------------------------\n"
            set $tmtbanner = 0
        end
        printtmt $pTMT
        set $pTMT = $pTMT->tmtLink
    end
end
#...
document printfreetmt
Prints the list of free tmts.
end
#-----------------------------------------------------------------------------
# This macro prints the tlmt struct.
define printfreetlmt
    set $I = (UINT32 *)i_sp_tlmts
    set $tlmtbanner = 1
    while ($I > 0)
        if ($tlmtbanner > 0)
            printf "     tlmt        tmt        cimt        whead      wtail       ahead      wtail     lun       misc\n"
            printf "----------------------------------------------------------------------------------------------------------\n"
            set $tlmtbanner = 0
        end
        printtlmt $I
        set $I = (UINT32 *)$I[0]
    end
end
#...
document printfreetlmt
Prints the list of free tlmts.
end
#-----------------------------------------------------------------------------
# This macro prints the tlmt structure.
define printtlmt
    set $I = (UINT32 *)$arg0
    printf " 0x%8.8x : ",$I
    printf " 0x%8.8x",$I[4]
    printf " 0x%8.8x",$I[3]
    printf " 0x%8.8x",$I[6]
    printf " 0x%8.8x",$I[7]
    printf " 0x%8.8x",$I[8]
    printf " 0x%8.8x",$I[9]
    printf " 0x%8.8x",$I[10]
    printf " 0x%8.8x",$I[2]
    printf "\n"
end
#...
document printtlmt
Prints the tlmt struct.
end
#-----------------------------------------------------------------------------
# This macro prints the list of available vdisks.
define vdisklist
    printf "Virtual Disk List:\n"
    printf "------------------\n"
    set $I = 0
    while ($I < $MAXVIRTUALS)
        if ((*(VDX*)&gVDX).vdd[$I] > 0)
            printf " %3d    0x%8.8x\n", $I, gVDX.vdd[$I]
        end
        set $I = $I + 1
    end
    printf "\n"
end
#...
document vdisklist
Prints the list of available vdisks.
end
#-----------------------------------------------------------------------------
# This macro prints the vdisk information for a given vdisk.
define vdiskinfo
    if ((*(VDX*)&gVDX).vdd[$arg0] != 0)
        printf "Virtual Disk Information (%d):\n", $arg0
        print (*(VDX*)&gVDX).vdd[$arg0]
        set $pVDD = (VDD*)gVDX.vdd[$arg0]
        set $pRDD = $pVDD->pRDD
        print *$pVDD
        printf "  RID  STATUS  ASTATUS\n"
        printf "  ---  ------  -------\n"
        set $I = 0
        while ($I < $pVDD->raidCnt)
            printf "  %3hu",        $pRDD->rid
            printf "    0x%2.2x",   $pRDD->status
            printf "     0x%2.2x",  $pRDD->aStatus
            printf "\n"
            set $pRDD = $pRDD->pNRDD
            set $I = $I + 1
        end
    else
        printf "Virtual Disk Information (%d): NOT AVAILABLE\n", $arg0
    end
end
#...
document vdiskinfo
Print the vdisk information for a given vdisk (argument VID).
    VID - Virtual Disk Identifier
end
#-----------------------------------------------------------------------------
# This macro prints the raids information given the specified format.
define i_raids
    set $DSPTYPE = $arg0
    if ($DSPTYPE == 0)
        printf " RID  VID Type  Status  AStatus  r5SROut\n"
        printf " ---  --- ----  ------  -------  -------\n"
    end
    set $I = 0
    while ($I < $MAXRAIDS)
        if ((*(RDX*)&gRDX).rdd[$I] > 0)
            if ($DSPTYPE == 0)
                printf " %3hu",          gRDX.rdd[$I].rid
                printf " %3hu",          gRDX.rdd[$I].vid
                printf "  %4hu",         gRDX.rdd[$I].type
                printf "    0x%02x",     gRDX.rdd[$I].status
                printf "     0x%02x",    gRDX.rdd[$I].aStatus
                printf "  %7hu",    gRDX.rdd[$I].r5SROut
            end
            printf "\n"
        end
        set $I = $I + 1
    end
end
#...
document i_raids
Print the raid information given the specified format.
    0 - RAID status information
end
#-----------------------------------------------------------------------------
# This macro prints the raid information in standard format.
define raidstatus
    i_raids 0
end
#...
document raidstatus
Print the raid information in standard format.
end
#-----------------------------------------------------------------------------
# This macro prints the list of available raids.
define raidlist
    printf "RAID List:\n"
    printf "----------\n"
    set $I = 0
    while ($I < $MAXRAIDS)
        if ((*(RDX*)&gRDX).rdd[$I] > 0)
            printf " %d\n", $I
        end
        set $I = $I + 1
    end
end
#...
document raidlist
Prints the list of available raids.
end
#-----------------------------------------------------------------------------
# This macro prints the raid information for a given raid.
define raidinfo
    printf "RAID Information (%d):", $arg0
    if ((*(RDX*)&gRDX).rdd[$arg0] > 0)
        set $pRDD = (RDD*)gRDX.rdd[$arg0]
        set $pPSD = $pRDD->extension.pPSD
        printf "\n"
        print *$pRDD
        printf "\n"
        printf "  PID  RID  STATUS  ASTATUS\n"
        printf "  ---  ---  ------  -------\n"
        set $I = 0
        while ($I < $pRDD->psdCnt)
            printf "  %3hu",        $pPSD->pid
            printf "  %3hu",        $pPSD->rid
            printf "    0x%2.2x",   $pPSD->status
            printf "     0x%2.2x",  $pPSD->aStatus
            printf "\n"
            set $pPSD = $pPSD->npsd
            set $I = $I + 1
        end
    end
    if ((*(RDX*)&gRDX).rdd[$arg0] == 0)
        printf " NOT AVAILABLE\n"
    end
end
#...
document raidinfo
Print the raid information for a given raid (argument RID).
    RID - RAID Identifier
end
#-----------------------------------------------------------------------------
# This macro prints be path information for all PDDs
define bepaths
    set $I=0
    printf "    PID     Prt     handle      P0        P1        P2        P3\n"
    printf "    ---     ---     ------    ------    ------    ------    ------\n"
    while ($I < 512)
    
        if ((*(PDX*)&gPDX).pdd[$I] != 0)
                set $DEV = (*(PDX*)&gPDX).pdd[$I].pDev
                if ($DEV != 0)
                    printf "   %3hu       %d      0x%04X    0x%04X    0x%04X    0x%04X    0x%04X\n", (*(PDX*)&gPDX).pdd[$I].pid,$DEV->port,$DEV->lid,$DEV->pLid[0], $DEV->pLid[1],$DEV->pLid[2], $DEV->pLid[3]
                else
                    printf "   %3hu      NO DEV NO PATH\n",(*(PDX*)&gPDX).pdd[$I].pid
                end
        end
        set $I = $I + 1
    end
    
    printf "\n    BAY     Prt     handle      P0        P1        P2        P3\n"
    printf "    ---     ---     ------    ------    ------    ------    ------\n"
    set $I=0
    while ($I < 64)
        if ((*(PDX*)&gEDX).pdd[$I] > 0)
                set $DEV = (*(PDX*)&gEDX).pdd[$I].pDev
                if ($DEV != 0)
                   printf "   %3hu       %d      0x%04X    0x%04X    0x%04X    0x%04X    0x%04X\n", (*(PDX*)&gEDX).pdd[$I].pid,$DEV->port,$DEV->lid,$DEV->pLid[0], $DEV->pLid[1],$DEV->pLid[2], $DEV->pLid[3]
                else
                   printf "    %3hu      NO DEV NO PATH\n",(*(PDX*)&gEDX).pdd[$I].pid
                end
        end
        set $I = $I + 1
    end

    printf "\n    MSC     Prt     handle      P0        P1        P2        P3\n"
    printf "    ---     ---     ------    ------    ------    ------    ------\n"
    set $I=0
    while ($I < 64)
        if ((*(PDX*)&gMDX).pdd[$I] > 0)
                set $DEV = (*(PDX*)&gMDX).pdd[$I].pDev
                if ($DEV != 0)
                    printf "   %3hu       %d      0x%04X    0x%04X    0x%04X    0x%04X    0x%04X\n", (*(PDX*)&gMDX).pdd[$I].pid,$DEV->port,$DEV->lid,$DEV->pLid[0], $DEV->pLid[1],$DEV->pLid[2], $DEV->pLid[3]
                else
                   printf "    %3hu      NO DEV NO PATH\n",(*(PDX*)&gMDX).pdd[$I].pid
                end
        end
        set $I = $I + 1
    end
end
#----
document bepaths
Print the be path information for PDX, EDX and MDX
end
#-----------------------------------------------------------------------------
# This macro prints channel,lun,fd-id information for all PDDs
define cls
    set $I=0
    printf " PID   CH    LUN      FC ID\n"
    printf "---- ---- ------ ----------\n"
    while ($I < 512)
        if ((*(PDX*)&gPDX).pdd[$I] != 0)
            if ((*(PDX*)&gPDX).pdd[$I]->pid != $I)
                printf "ERROR, entry (%d) != pid (%d)\n", $I, (*(PDX*)&gPDX).pdd[$I]->pid
            end
            printf "%4hu 0x%02x 0x%04x 0x%08X\n", $I, (*(PDX*)&gPDX).pdd[$I]->channel, (*(PDX*)&gPDX).pdd[$I]->lun, (*(PDX*)&gPDX).pdd[$I]->id
        end
        set $I = $I + 1
    end
    
    printf " BAY   CH    LUN      FC ID\n"
    printf "---- ---- ------ ----------\n"
    set $I=0
    while ($I < 64)
        if ((*(PDX*)&gEDX).pdd[$I] > 0)
            if ((*(PDX*)&gEDX).pdd[$I]->pid != $I)
                printf "ERROR, entry (%d) != pid (%d)\n", $I, (*(PDX*)&gEDX).pdd[$I]->pid
            end
            printf "%4hu 0x%02x 0x%04x 0x%08X\n", $I, (*(PDX*)&gEDX).pdd[$I]->channel, (*(PDX*)&gEDX).pdd[$I]->lun, (*(PDX*)&gEDX).pdd[$I]->id
        end
        set $I = $I + 1
    end

    printf "MISC   CH    LUN      FC ID\n"
    printf "---- ---- ------ ----------\n"
    set $I=0
    while ($I < 64)
        if ((*(PDX*)&gMDX).pdd[$I] > 0)
            if ((*(PDX*)&gMDX).pdd[$I]->pid != $I)
                printf "ERROR, entry (%d) != pid (%d)\n", $I, (*(PDX*)&gMDX).pdd[$I]->pid
            end
            printf "%4hu 0x%02x 0x%04x 0x%08X\n", $I, (*(PDX*)&gMDX).pdd[$I]->channel, (*(PDX*)&gMDX).pdd[$I]->lun, (*(PDX*)&gMDX).pdd[$I]->id
        end
        set $I = $I + 1
    end
end
#----
document cls
Print the channel, lun, FC-ID information for PDX, EDX and MDX
end
#-----------------------------------------------------------------------------
# This macro prints the pdisks information given the specified format.
define i_pdisks
    set $DSPTYPE = $arg0
    if ($DSPTYPE == 0)
        printf "  PID   Dev  Misc  Post   LID    CHANNEL  TYPE        WWN          Serial#       Name     Class\n"
        printf "  ---  ----  ----  ----  ------  -------  ----  ----------------  ----------  ----------  -----\n"
    end
    if ($DSPTYPE == 1)
        printf "  PID   REM    RBREMAIN          LAS            TAS        SES   SLOT    DNAME      HSDNAME  \n"
        printf "  ---  ----  -------------  -------------  -------------  -----  ----  ----------  ----------\n"
    end
    if ($DSPTYPE == 2)
        printf "  PID  VendorID  TYPE   REV   Product ID        Serial #      CAPACITY (blocks)   \n"
        printf "  ---  --------  ----  -----  ----------------  ------------  --------------------\n"
    end
    if ($DSPTYPE == 3)
        printf "  PID   SES    SLOT      DNAME  \n"
        printf "  ---  -----  ------  ----------\n"
    end
    if ($DSPTYPE == 4)
        printf "  PID   Dev  Misc  Post  Flag  DevPtr        DNAME        DFlags    itQFHead      failQHead     dv_wait   qdepth\n"
        printf "  ---  ----  ----  ----  ----  ----------    --------     ------    ----------    ----------    -------   ------\n"
    end
    set $I = 0
    while ($I < $MAXDRIVES)
        if ((*(PDX*)&gPDX).pdd[$I] > 0)
            if ($DSPTYPE == 0)
                printf "  %3hu",        gPDX.pdd[$I].pid
                printf "  0x%02x",      gPDX.pdd[$I].devStat
                printf "  0x%02x",      gPDX.pdd[$I].miscStat
                printf "  0x%02x",      gPDX.pdd[$I].postStat
                printf "  0x%04x",      gPDX.pdd[$I].id
                printf "   0x%04x",      gPDX.pdd[$I].channel
                printf "  0x%2.2x",     gPDX.pdd[$I].devType
                printf "  %8.8x%8.8x",  (gPDX.pdd[$I].wwn >> 32), gPDX.pdd[$I].wwn
                printf "  0x%8.8x",     gPDX.pdd[$I].ssn
                printf "  %c%c %02hd %02hd  ", gPDX.pdd[$I].devName[0], gPDX.pdd[$I].devName[1], gPDX.pdd[$I].devName[2], gPDX.pdd[$I].devName[3]
                printf "  0x%2.2x",     gPDX.pdd[$I].devClass
            end
            if ($DSPTYPE == 1)
                printf "  %3hu",        gPDX.pdd[$I].pid
                printf "  %4d",         gPDX.pdd[$I].pctRem
                printf "  %13d",        gPDX.pdd[$I].rbRemain
                printf "  %13d",        gPDX.pdd[$I].las
                printf "  %13d",        gPDX.pdd[$I].tas
                printf "  %5d",         gPDX.pdd[$I].ses
                printf "  %4d",         gPDX.pdd[$I].slot
                printf "  %c%c %02hd %02hd  ", gPDX.pdd[$I].devName[0], gPDX.pdd[$I].devName[1], gPDX.pdd[$I].devName[2], gPDX.pdd[$I].devName[3]
                if (gPDX.pdd[$I].hsDevName == 0 || (*(int*)(gPDX.pdd[$I].hsDevName)) == 0)
                  printf " -"
                else
                  if (gPDX.pdd[$I].hsDevName[0] == 'P' && gPDX.pdd[$I].hsDevName[1] == 'D')
                    set $bay = gPDX.pdd[$I].hsDevName[2]
                    set $slot = gPDX.pdd[$I].hsDevName[3]
                    printf " PD %2.2d-%2.2d", $bay, $slot
                  else
                    printf "  0x%8.8x",     *gPDX.pdd[$I].hsDevName
                  end
                end
            end
            if ($DSPTYPE == 2)
                printf "  %3hu",        gPDX.pdd[$I].pid
                printf "  %8.8s",       gPDX.pdd[$I].vendID
                printf "  0x%2.2x",     gPDX.pdd[$I].devType
                printf "  %5.5s",       gPDX.pdd[$I].prodRev
                printf "  %16.16s",     gPDX.pdd[$I].prodID
                printf "  %12.12s",     gPDX.pdd[$I].serial
                printf "  %20lld",      gPDX.pdd[$I].devCap
            end
            if ($DSPTYPE == 3)
                printf "  %3hu",        gPDX.pdd[$I].pid
                printf "  %5d",         gPDX.pdd[$I].ses
                printf "  %6d",         gPDX.pdd[$I].slot
                printf "  %c%c %02hd %02hd  ", gPDX.pdd[$I].devName[0], gPDX.pdd[$I].devName[1], gPDX.pdd[$I].devName[2], gPDX.pdd[$I].devName[3]
            end
            if ($DSPTYPE == 4)
                printf "  %3hu",        gPDX.pdd[$I].pid
                printf "  0x%02x",      gPDX.pdd[$I].devStat
                printf "  0x%02x",      gPDX.pdd[$I].miscStat
                printf "  0x%02x",      gPDX.pdd[$I].postStat
                printf "  0x%02x",      gPDX.pdd[$I].flags
                printf "  0x%08x",      gPDX.pdd[$I].pDev
                printf "    %c%c %02hd %02hd ", gPDX.pdd[$I].devName[0],  gPDX.pdd[$I].devName[1], gPDX.pdd[$I].devName[2], gPDX.pdd[$I].devName[3]
                set $DEV = gPDX.pdd[$I].pDev
                if ($DEV != 0)
                    printf "    0x%02hX      0x%08x    0x%08x    0x%04x", $DEV->flags, $DEV->iltQFHead, $DEV->failQHead, $DEV->wait
                    printf "    0x%02x", $DEV->qCnt
                end
            end
            printf "\n"
        end
        set $I = $I + 1
    end
end
#...
document i_pdisks
Print the pdisks information given the specified format.
    0 - PDisk status information
    1 - PDisk standard information
    2 - PDisk firmware/vendor information
    3 - PDisk SES information
    4 - PDisk DEV information
end
#-----------------------------------------------------------------------------
# This macro prints the pdisks status information.
define pdiskstatus
    i_pdisks 0
end
#...
document pdiskstatus
Print the pdisks status information.
end
#-----------------------------------------------------------------------------
# This macro prints the pdisks information in standard format.
define pdiskstd
    i_pdisks 1
end
#...
document pdiskstd
Print the pdisks information in standard format.
end
#-----------------------------------------------------------------------------
# This macro prints the pdisks firmware/vendor information.
define pdiskfwv
    i_pdisks 2
end
#...
document pdiskfwv
Print the pdisks firmware/vendor information.
end
#-----------------------------------------------------------------------------
# This macro prints the pdisks SES information.
define pdiskses
    i_pdisks 3
end
#...
document pdiskses
Print the pdisks SES information.
end
#-----------------------------------------------------------------------------
# This macro prints the list of available pdisks.
define pdisklist
    printf "Physical Disk List:\n"
    printf "-------------------\n"
    set $I = 0
    while ($I < (sizeof(gPDX.pdd) / 4))
        if ((*(PDX*)&gPDX).pdd[$I] > 0)
            printf " %d\n", $I
        end
        set $I = $I + 1
    end
end
#...
document pdisklist
Prints the list of available pdisks.
end
#-----------------------------------------------------------------------------
# This macro prints the detailed bay information for a given bay.
define bayinfo
    printf "Physical Bay Information (%d):", $arg0
    if ((*(PDX*)&gEDX).pdd[$arg0] != 0)
        printf "\n"
        print *(PDD*)gEDX.pdd[$arg0]
    else
        printf " NOT AVAILABLE\n"
    end
end
#...
document bayinfo
Print the detailed bay information for a given bay (argument PID).
    PID - Physical Disk Identifier (meaning bay number).
end
#-----------------------------------------------------------------------------
# This macro prints the detailed Misc information for a given misc device.
define miscinfo
    printf "Physical MISC Information (%d):", $arg0
    if ((*(PDX*)&gMDX).pdd[$arg0] != 0)
        printf "\n"
        print *(*(PDX*)&gMDX).pdd[$arg0]
    else
        printf " NOT AVAILABLE\n"
    end
end
#...
document miscinfo
Print the detailed misc information for a given misc device (argument PID).
    PID - Physical Disk Identifier (meaning MISC entry number)
end
#-----------------------------------------------------------------------------
# This macro prints the detailed pdisk information for a given pdisk.
define pdiskinfo
    printf "Physical Disk Information (%d):", $arg0
    if ((*(PDX*)&gPDX).pdd[$arg0] != 0)
        printf "\n"
        print *(PDD*)gPDX.pdd[$arg0]
    else
        printf " NOT AVAILABLE\n"
    end
end
#...
document pdiskinfo
Print the detailed pdisk information for a given pdisk (argument PID).
    PID - Physical Disk Identifier
end
#-----------------------------------------------------------------------------
# This macro prints the vcd information.
define vcdlist
    set $I = 0
    printf "  VID  STAT  pIO         pCache      pDirty      pTHead      WRITECOUNT\n"
    printf "  ---  ----  ----------  ----------  ----------  ----------  ----------\n"
    while ($I < $MAXVIRTUALS)
        set $pVCD = ((VCD **)&vcdIndex)[$I]
        if ($pVCD > 0)
            printf "  %3d",     $pVCD->vid
            printf "  0x%2.2x", $pVCD->stat
            printf "  0x%8.8x", $pVCD->pIO
            printf "  0x%8.8x", $pVCD->pCache
            printf "  0x%8.8x", $pVCD->pDirty
            printf "  0x%8.8x", $pVCD->pTHead
            printf "  0x%8.8x", $pVCD->writeCount
            printf "\n"
        end
        set $I = $I + 1
    end
end
#...
document vcdlist
Print the vcd information.
end
#-----------------------------------------------------------------------------
# This macro prints the vcd information for a given vcd.
define vcdinfo
    printf "VCD Information (%d):", $arg0
    set $pVCD = ((VCD**)&vcdIndex)[$arg0]
    if ($pVCD > 0)
        printf "\n"
        print *$pVCD
    end
    if ($pVCD == 0)
        printf " NOT AVAILABLE\n"
    end
end
#...
document vcdinfo
Print the vcd information for a given vcd (argument VID).
    VID - Virtual disk identifier
end
#-----------------------------------------------------------------------------
# This macro prints the vcd cache information for a given vcd.
define vcdcache
    set $I = 0
    set $count = 0
    if ($arg0 == 0xFFFF)
        printf "VCD Cache Information for all virtual disks with IO trees:\n\n"
    else
        set $I = $arg0
        printf "VCD Cache Information (%d, 0x%8.8x):\n\n", $arg0, ((VCD**)&vcdIndex)[$arg0]
    end
    printf "  VID   CURRENT      PARENT      CLEFT       CRIGHT     PAYLOAD    VID  ATTRIB  STATE \n"
    printf "  ---  ----------  ----------  ----------  ----------  ----------  ---  ------  ------\n"
    while (($arg0 == 0xFFFF && $I < $MAXVIRTUALS) || $I == $arg0)
        set $pVCD = ((VCD**)&vcdIndex)[$I]
        if ($pVCD > 0)
            set $pRB = (struct RB*)$pVCD->pCache
            set $pRBPrev = &nil
            if ($pRB == 0)
                set $pRB = &nil
            end
            while ($pRB != &nil)
                set $count = $count + 1
                set $pTG = (TG*)$pRB->dPoint
                printf "  %3d",             $arg0
                printf "  0x%8.8x",         $pRB
                if ($pRB->bParent == &nil)
                    printf "         NIL"
                else
                    printf "  0x%8.8x",         $pRB->bParent
                end
                if ($pRB->cLeft == &nil)
                    printf "         NIL"
                else
                    printf "  0x%8.8x",         $pRB->cLeft
                end
                if ($pRB->cRight == &nil)
                    printf "         NIL"
                else
                    printf "  0x%8.8x",         $pRB->cRight
                end
                printf "  0x%8.8x",         $pRB->dPoint
                printf "  %3d",             $pTG->vid
                printf "  0x%4.4x",         $pTG->attrib
                printf "  0x%4.4x",         $pTG->state
                printf "\n"
                if ($pRB->bParent == 0 && $pRB->cLeft == &nil && $pRB->cRight == &nil)
                    set $pRB = &nil
                else
                    if ($pRB->cLeft != &nil)
                        set $pRB = $pRB->cLeft
                    else
                        if ($pRB->cRight != &nil)
                            set $pRB = $pRB->cRight
                        else
                            set $pRBPrev = $pRB
                            set $pRB = $pRB->bParent
                            while ($pRBPrev != &nil)
                                if ($pRBPrev == $pRB->cLeft)
                                    if ($pRB->cRight != &nil)
                                        set $pRBPrev = &nil
                                        set $pRB = $pRB->cRight
                                    else
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                else
                                    if ($pRBPrev == $pRB->cRight)
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
            end
        end
        set $I = $I + 1
    end
    printf "\n"
    printf "Total Count: %d\n", $count
end
#...
document vcdcache
Print the vcd cache information for a given vcd (argument VID).
    VID - Virtual disk identifier
    0xFFFF for doing all of them.
end
#-----------------------------------------------------------------------------
# This macro prints the vcd cache sda information for a given vcd.
define vcdcachesda
    set $I = 0
    set $count = 0
    if ($arg0 == 0xFFFF)
        printf "VCD Cache Information for all virtual disks with IO trees:\n\n"
    else
        set $I = $arg0
        printf "VCD Cache Information (%d, 0x%8.8x):\n\n", $arg0, ((VCD**)&vcdIndex)[$arg0]
    end
    printf "  VID   CURRENT     PAYLOAD    VID  VSDA                  LEN       \n"
    printf "  ---  ----------  ----------  ---  --------------------  ----------\n"
    while (($arg0 == 0xFFFF && $I < $MAXVIRTUALS) || $I == $arg0)
        set $pVCD = ((VCD**)&vcdIndex)[$I]
        if ($pVCD > 0)
            set $pRB = (struct RB*)$pVCD->pCache
            set $pRBPrev = &nil
            if ($pRB == 0)
                set $pRB = &nil
            end
            while ($pRB != &nil)
                set $count = $count + 1
                set $pTG = (TG*)$pRB->dPoint
                printf "  %3d",             $arg0
                printf "  0x%8.8x",         $pRB
                printf "  0x%8.8x",         $pRB->dPoint
                printf "  %3d",             $pTG->vid
                printf "  %8.8x%8.8x",      (UINT32)($pTG->vsda >> 32), (UINT32)($pTG->vsda)
                printf "  %10d",            $pTG->vLen
                printf "\n"
                if ($pRB->bParent == 0 && $pRB->cLeft == &nil && $pRB->cRight == &nil)
                    set $pRB = &nil
                else
                    if ($pRB->cLeft != &nil)
                        set $pRB = $pRB->cLeft
                    else
                        if ($pRB->cRight != &nil)
                            set $pRB = $pRB->cRight
                        else
                            set $pRBPrev = $pRB
                            set $pRB = $pRB->bParent
                            while ($pRBPrev != &nil)
                                if ($pRBPrev == $pRB->cLeft)
                                    if ($pRB->cRight != &nil)
                                        set $pRBPrev = &nil
                                        set $pRB = $pRB->cRight
                                    else
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                else
                                    if ($pRBPrev == $pRB->cRight)
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
            end
        end
        set $I = $I + 1
    end
    printf "\n"
    printf "Total Count: %d\n", $count
end
#...
document vcdcachesda
Print the vcd cache sda information for a given vcd (argument VID).
    VID - Virtual disk identifier
    0xFFFF for doing all of them.
end
#-----------------------------------------------------------------------------
# This macro prints the vcd cache information for a given vcd.
define vcddirty
    set $I = 0
    set $count = 0
    if ($arg0 == 0xFFFF)
        printf "VCD Dirty Information for all virtual disks with IO trees:\n\n"
    else
        set $I = $arg0
        printf "VCD Dirty Information (%d, 0x%8.8x):\n\n", $arg0, ((VCD**)&vcdIndex)[$I]
    end
    printf "  VID   CURRENT      PARENT      CLEFT       CRIGHT     PAYLOAD    VID  ATTRIB  STATE   DIRTY TREE\n"
    printf "  ---  ----------  ----------  ----------  ----------  ----------  ---  ------  ------  ----------\n"
    while (($arg0 == 0xFFFF && $I < $MAXVIRTUALS) || $I == $arg0)
        set $pVCD = ((VCD**)&vcdIndex)[$I]
        if ($pVCD > 0)
            set $pRB = (struct RB*)$pVCD->pDirty
            set $pRBPrev = &nil
            if ($pRB == 0)
                set $pRB = &nil
            end
            while ($pRB != &nil)
                set $count = $count + 1
                set $pTG = (TG*)$pRB->dPoint
                printf "  %3d",             $arg0
                printf "  0x%8.8x",         $pRB
                if ($pRB->bParent == &nil)
                    printf "         NIL"
                else
                    printf "  0x%8.8x",         $pRB->bParent
                end
                if ($pRB->cLeft == &nil)
                    printf "         NIL"
                else
                    printf "  0x%8.8x",         $pRB->cLeft
                end
                if ($pRB->cRight == &nil)
                    printf "         NIL"
                else
                    printf "  0x%8.8x",         $pRB->cRight
                end
                printf "  0x%8.8x",         $pRB->dPoint
                printf "  %3d",             $pTG->vid
                printf "  0x%4.4x",         $pTG->attrib
                printf "  0x%4.4x",         $pTG->state
                printf "  0x%8.8x",         $pTG->dirtyPtr
                printf "\n"
                if ($pRB->bParent == 0 && $pRB->cLeft == &nil && $pRB->cRight == &nil)
                    set $pRB = &nil
                else
                    if ($pRB->cLeft != &nil)
                        set $pRB = $pRB->cLeft
                    else
                        if ($pRB->cRight != &nil)
                            set $pRB = $pRB->cRight
                        else
                            set $pRBPrev = $pRB
                            set $pRB = $pRB->bParent
                            while ($pRBPrev != &nil)
                                if ($pRBPrev == $pRB->cLeft)
                                    if ($pRB->cRight != &nil)
                                        set $pRBPrev = &nil
                                        set $pRB = $pRB->cRight
                                    else
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                else
                                    if ($pRBPrev == $pRB->cRight)
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
            end
        end
        set $I = $I + 1
    end
    printf "\n"
    printf "Total Count: %d\n", $count
end
#...
document vcddirty
Print the vcd dirty information for a given vcd (argument VID).
    VID - Virtual disk identifier
    0xFFFF for doing all of them.
end
#-----------------------------------------------------------------------------
# This macro prints the dirty vcd sda cache information for a given vcd.
define vcddirtysda
    set $I = 0
    set $count = 0
    if ($arg0 == 0xFFFF)
        printf "VCD Dirty Information for all virtual disks with IO trees:\n\n"
    else
        set $I = $arg0
        printf "VCD Dirty Information (%d, 0x%8.8x):\n\n", $arg0, ((VCD**)&vcdIndex)[$arg0]
    end
    printf "  VID   CURRENT     PAYLOAD    VID  VSDA                  LEN       \n"
    printf "  ---  ----------  ----------  ---  --------------------  ----------\n"
    while (($arg0 == 0xFFFF && $I < $MAXVIRTUALS) || $I == $arg0)
        set $pVCD = ((VCD**)&vcdIndex)[$I]
        if ($pVCD > 0)
            set $pRB = (struct RB*)$pVCD->pDirty
            set $pRBPrev = &nil
            if ($pRB == 0)
                set $pRB = &nil
            end
            while ($pRB != &nil)
                set $count = $count + 1
                set $pTG = (TG*)$pRB->dPoint
                printf "  %3d",             $I
                printf "  0x%8.8x",         $pRB
                printf "  0x%8.8x",         $pRB->dPoint
                printf "  %3d",             $pTG->vid
                printf "  %8.8x%8.8x",      (UINT32)($pTG->vsda >> 32), (UINT32)($pTG->vsda)
                printf "  %10d",            $pTG->vLen
                printf "\n"
                if ($pRB->bParent == 0 && $pRB->cLeft == &nil && $pRB->cRight == &nil)
                    set $pRB = &nil
                else
                    if ($pRB->cLeft != &nil)
                        set $pRB = $pRB->cLeft
                    else
                        if ($pRB->cRight != &nil)
                            set $pRB = $pRB->cRight
                        else
                            set $pRBPrev = $pRB
                            set $pRB = $pRB->bParent
                            while ($pRBPrev != &nil)
                                if ($pRBPrev == $pRB->cLeft)
                                    if ($pRB->cRight != &nil)
                                        set $pRBPrev = &nil
                                        set $pRB = $pRB->cRight
                                    else
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                else
                                    if ($pRBPrev == $pRB->cRight)
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
            end
        end
        set $I = $I + 1
    end
    printf "\n"
    printf "Total Count: %d\n", $count
end
#...
document vcddirtysda
Print the dirty vcd sda cache information for a given vcd (argument VID).
    VID - Virtual disk identifier
    0xFFFF for doing all of them.
end
#-----------------------------------------------------------------------------
# This macro prints the vcd cache information for a given vcd.
define vcdiosda
    set $I = 0
    set $count = 0
    if ($arg0 == 0xFFFF)
        printf "VCD Cache Information for all virtual disks with IO trees:\n\n"
    else
        set $I = $arg0
        printf "VCD Cache Information (%d, 0x%8.8x):\n\n", $arg0, ((VCD**)&vcdIndex)[$arg0]
    end
    printf "  VID   CURRENT     PAYLOAD    MISC        CR          BLKIOCNT    VRP         STATUS      SDA                 LEN       \n"
    printf "  ---  ----------  ----------  ----------  ----------  ----------  ----------  ----------  ------------------  ----------\n"
    while (($arg0 == 0xFFFF && $I < $MAXVIRTUALS) || $I == $arg0)
        set $pVCD = ((VCD**)&vcdIndex)[$I]
        if ($pVCD > 0)
            set $pRB = (struct RB*)$pVCD->pIO
            set $pRBPrev = &nil
            if ($pRB == 0)
                set $pRB = &nil
            end
            while ($pRB != &nil)
                set $count = $count + 1
                set $pILT = (ILT*)$pRB->dPoint
                set $pUINT32 = (UINT32*)$pRB->dPoint
                printf "  %3d",             $I
                printf "  0x%8.8x",         $pRB
                printf "  0x%8.8x",         $pRB->dPoint
                printf "  0x%8.8x",         $pILT->misc
                printf "  0x%8.8x",         $pILT->cr
                printf "  0x%8.8x",         $pUINT32[5]
                printf "  0x%8.8x",         (VRP*)($pUINT32[8])
                printf "  0x%8.8x",         ((VRP*)($pUINT32[8]))->status
                printf "  0x%8.8x%8.8x",    (((VRP*)($pUINT32[8]))->startDiskAddr >> 32), (UINT32)(((VRP*)($pUINT32[8]))->startDiskAddr)
                printf "  0x%8.8x",         ((VRP*)($pUINT32[8]))->length
                printf "\n"
                if ($pRB->bParent == 0 && $pRB->cLeft == &nil && $pRB->cRight == &nil)
                    set $pRB = &nil
                else
                    if ($pRB->cLeft != &nil)
                        set $pRB = $pRB->cLeft
                    else
                        if ($pRB->cRight != &nil)
                            set $pRB = $pRB->cRight
                        else
                            set $pRBPrev = $pRB
                            set $pRB = $pRB->bParent
                            while ($pRBPrev != &nil)
                                if ($pRBPrev == $pRB->cLeft)
                                    if ($pRB->cRight != &nil)
                                        set $pRBPrev = &nil
                                        set $pRB = $pRB->cRight
                                    else
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                else
                                    if ($pRBPrev == $pRB->cRight)
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
            end
        end
        set $I = $I + 1
    end
    printf "\n"
    printf "Total Count: %d\n", $count
end
#...
document vcdiosda
Print the vcd cache sda information for a given vcd (argument VID).
    VID - Virtual disk identifier
    0xFFFF for doing all of them.
end
#-----------------------------------------------------------------------------
# This macro prints the vcd cache information for a given vcd.
define vcdiokeys
    set $I = 0
    set $count = 0
    if ($arg0 == 0xFFFF)
        printf "VCD Cache Information for all virtual disks with IO trees:\n\n"
    else
        set $I = $arg0
        printf "VCD Cache Information (%d, 0x%8.8x):\n\n", $arg0, ((VCD**)&vcdIndex)[$arg0]
    end
    printf "  VID   CURRENT    PARENT      LEFT        RIGHT       FTHD        BTHD        KEY                 KEYM                NODEM             \n"
    printf "  ---  ----------  ----------  ----------  ----------  ----------  ----------  ------------------  ------------------  ------------------\n"
    while (($arg0 == 0xFFFF && $I < $MAXVIRTUALS) || $I == $arg0)
        set $pVCD = ((VCD**)&vcdIndex)[$I]
        if ($pVCD > 0)
            set $pRB = (struct RB*)$pVCD->pIO
            set $pRBPrev = &nil
            if ($pRB == 0)
                set $pRB = &nil
            end
            while ($pRB != &nil)
                set $count = $count + 1
                printf "  %3d",             $I
                printf "  0x%8.8x",         $pRB
                printf "  0x%8.8x",         $pRB->bParent
                printf "  0x%8.8x",         $pRB->cLeft
                printf "  0x%8.8x",         $pRB->cRight
                printf "  0x%8.8x",         $pRB->fthd
                printf "  0x%8.8x",         $pRB->bthd
                printf "  0x%8.8x%8.8x",    ($pRB->key >> 32), (UINT32)$pRB->key
                printf "  0x%8.8x%8.8x",    ($pRB->keym >> 32), (UINT32)$pRB->keym
                printf "  0x%8.8x%8.8x",    ($pRB->nodem >> 32), (UINT32)$pRB->nodem
                printf "\n"
                if ($pRB->bParent == 0 && $pRB->cLeft == &nil && $pRB->cRight == &nil)
                    set $pRB = &nil
                else
                    if ($pRB->cLeft != &nil)
                        set $pRB = $pRB->cLeft
                    else
                        if ($pRB->cRight != &nil)
                            set $pRB = $pRB->cRight
                        else
                            set $pRBPrev = $pRB
                            set $pRB = $pRB->bParent
                            while ($pRBPrev != &nil)
                                if ($pRBPrev == $pRB->cLeft)
                                    if ($pRB->cRight != &nil)
                                        set $pRBPrev = &nil
                                        set $pRB = $pRB->cRight
                                    else
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                else
                                    if ($pRBPrev == $pRB->cRight)
                                        if ($pRB->bParent == 0)
                                            set $pRBPrev = &nil
                                            set $pRB = &nil
                                        else
                                            set $pRBPrev = $pRB
                                            set $pRB = $pRB->bParent
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
            end
        end
        set $I = $I + 1
    end
    printf "\n"
    printf "Total Count: %d\n", $count
end
#...
document vcdiokeys
Print the vcd io key information for a given vcd (argument VID).
    VID - Virtual disk identifier
    0xFFFF for doing all of them.
end
#-----------------------------------------------------------------------------
# This macro prints out all the vcd cache information -- for red/black stuff.
define vcds
  vcdlist
  vcdcache 0xFFFF
  vcdcachesda 0xFFFF
  vcddirty 0xFFFF
  vcddirtysda 0xFFFF
  vcdiosda 0xFFFF
  vcdiokeys 0xFFFF
  vcdm4
end
#...
document vcds
Printout all the vcd cache information (for red/black stuff) for all vcdIndex.
end
#-----------------------------------------------------------------------------
# This macro dumps the information on the CA_OpRetryQue
#
define dump_CA_OpRetryQue
    set $I     = 0
    set $QHEAD = ((QU*)&CA_OpRetryQue)->head
    set $QTAIL = ((QU*)&CA_OpRetryQue)->tail
    set $QCNT  = ((QU*)&CA_OpRetryQue)->qcnt
    set $pILT = (ILT*)$QHEAD
    printf "  VID  FUNC  OPT   STATUS  SDA         LENGTH      ILT         FTHD        VRP       \n"
    printf "  ---  ----  ----  ------  ----------  ----------  ----------  ----------  ----------\n"
    while ($I < $QCNT)
        set $pVRPCMD = (VRPCMD*)(((uint8_t*)$pILT) + 16)
        set $pVRP = $pVRPCMD->pvrVRP
        printf "  %3d",         $pVRP->vid
        printf "  0x%2.2x",     $pVRP->function
        printf "  0x%2.2x",     $pVRP->options
        printf "    0x%2.2x",   $pVRP->status
        printf "  0x%8.8x",     $pVRP->startDiskAddr
        printf "  0x%8.8x",     $pVRP->length
        printf "  0x%8.8x",     $pILT
        printf "  0x%8.8x",     $pILT->fthd
        printf "  0x%8.8x",     $pVRP
        printf "\n"
        set $pILT = $pILT->fthd
        set $I = $I + 1
    end
end
#...
document dump_CA_OpRetryQue
Dumps the information on the CA_OpRetryQue.
end
#-----------------------------------------------------------------------------
# This macro prints the ILT thread starting at arg0
#
define followILT
  set $ILT = $arg0
  while ((UINT32)$ILT >= $EXECUTABLE_START)
    printf "ILT = 0x%08x,  ", $ILT
    print *(ILT*)$ILT
    set $ILT = ((ILT*)($ILT))->fthd
  end
end
#...
document followILT
Prints and traverses ILT queues (argument ILT).
end
#-----------------------------------------------------------------------------
# This macro prints the command record table entries
#
define CRTdump
    set $I = 0
    printf "  SLOT  CMD_CODE    STATE  PCB         IN          OUT         TIMEOUT   \n"
    printf "  ----  ----------  -----  ----------  ----------  ----------  ----------\n"
    while ($I < (sizeof(commandRecordTable) / sizeof(commandRecordTable[0])))
                if (commandRecordTable[$I].callerPCB > 0)
                        printf "  %4d", $I
                        printf "  0x%8.8x", commandRecordTable[$I].commandCode
            printf "  0x%3.3x", commandRecordTable[$I].state
            printf "  0x%8.8x", commandRecordTable[$I].callerPCB
            printf "  0x%8.8x", commandRecordTable[$I].commandBufferIn
            printf "  0x%8.8x", commandRecordTable[$I].commandBufferOut
            printf "  0x%8.8x", commandRecordTable[$I].timeout
            printf "\n"
        end
        set $I = $I + 1
    end
end
#...
document CRTdump
Dumps the command record table.
end
#-----------------------------------------------------------------------------
# This macro creates QLogic dumps for port arg0
#
define i_QLD
  set $PORT = $arg0
  if (ispdump[$PORT].addr != 0)
    dump binary memoryq $arg1 ispdump[$PORT].addr (ispdump[$PORT].addr+ispdump[$PORT].length)
  else
    printf "[NOCORE] NO QLOGIC CORE PRESENT PORT %-2.2x\n", $PORT
  end
end
#...
document i_QLD
Internal macro for QLDump.
end
#..............................................................................
define i_QueDmp
  set $PORT = $arg0
  set $que_f = (unsigned long)ispstr[$PORT]->$arg1->begin
  set $que_l = (unsigned long)ispstr[$PORT]->$arg1->end
  if ($que_f != $que_l)
    dump binary memory $arg2 $que_f $que_l
  end
end
#...
document i_QueDmp
Internal macro for QLDump.
end
#..............................................................................
define QLDump
  i_QLD $arg0 qldmp$arg1$arg0.bin
  i_QueDmp $arg0 reqQue qldmpreq$arg1$arg0.bin
  i_QueDmp $arg0 resQue qldmpres$arg1$arg0.bin
  i_QueDmp $arg0 atioQue qldmpatio$arg1$arg0.bin
  set $que_f = (unsigned long)asyqa[$arg0]->begin
  set $que_l = (unsigned long)asyqa[$arg0]->end
  dump binary memory qldmpasyqa$arg1$arg0.bin $que_f $que_l
end
#...
document QLDump
Macro "QLDump <port> <proc>" takes a QLogic dump.
end
#-----------------------------------------------------------------------------
# This macro prints the write cache information.
#    set $I = BE_BASE-FE_BASE+$I
# Must run this on the BE, not the FE!
# NOTDONEYET -- old cache, pre-870. It now uses memory allocator with pre/post-headers.
define writecache
    set $I = (UINT32)WctAddr
    set $J = $I+(UINT32)WctSize
    set $TGSIZE = (UINT32)64
  printf "I=%8.8x\n", $I
  printf "J=%8.8x\n", $J
  printf "TGSIZE=%u\n", $TGSIZE
    printf "      fthd     bthd   vid rdcnt attr stat             vsda     vLen   bufPtr\n"
    printf "  -------- -------- ----- ----- ---- ---- ---------------- -------- --------\n"
    printf "     ioPtr dirtyPtr  nxDirty   hQueue   tQueue    rsvd1    rsvd2    rsvd3\n"
    printf "  -------- -------- -------- -------- -------- -------- -------- --------\n"
    while ($I < $J)
        set $p = (TG*)($I)
        if (($p->attrib & 1) != 0)
            printf "  %8.8x %8.8x %5d %5d %4.4x %4.4x %16.16x %8.8x %8.8x\n", $p->fthd, $p->bthd, $p->vid, $p->rdCnt, $p->attrib, $p->state, $p->vsda, $p->vLen, $p->bufPtr
            printf "  %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x\n", $p->ioPtr, $p->dirtyPtr, $p->nextDirty, $p->hQueue, $p->tQueue, $p->rsvd1, $p->rsvd2, $p->rsvd3
        end
        set $I = $I + $TGSIZE
    end
end
#...
document writecache
Print the write cache information.
end
#-----------------------------------------------------------------------------
# This is internal macro to print out the flight recorder information (no time stamp).
define i_print_flight
    set $K = $arg0
    set $T0 = *((unsigned char *)($K+0))
    set $T1 = *((unsigned char *)($K+1))
    set $T2 = *((unsigned char *)($K+2))
    set $T3 = *((unsigned char *)($K+3))
    set $p0 = *((unsigned int *)$K)
    set $p1 = *((unsigned int *)($K+4))
    set $p2 = *((unsigned int *)($K+8))
    set $p3 = *((unsigned int *)($K+12))
    printf "  %-8.8lx: %-8.8x %-8.8x %-8.8x %-8.8x ", $K, $p0, $p1, $p2, $p3
    if ($T0 == 0x11)
        printf "isp.as/ispc.c - ISP$initiate_io/isp2400_initiate_io copy CDB to IOCB"
    end
    if ($T0 == 0x12)
        printf "isp.as - ISP$initiate_io copy SGL segments"
    end
    if ($T0 == 0x14)
        printf "isp.as - isp$thread_ilt"
    end
    if ($T0 == 0x15)
        printf "isp.as - ISP$unthread_ilt"
    end
    if ($T0 == 0x16)
        printf "isp.as - isp$submit_marker"
    end
    if ($T0 == 0x18)
        printf "ispc.c - ISP_LoopInitialize"
    end
    if ($T0 == 0x1F)
        if ($T1 == 0x00)
            printf "ispc.c - isp_resetProcess reset qlogic adapter"
        end
        if ($T1 == 0x10)
            printf "ispc.c - isp_processVpControl virtual port iocb processing"
        end
        if ($T1 == 0x11)
            printf "isp.as - isp_reset_chip starting "
        end
        if ($T1 == 0x12)
            printf "isp.as - isp_reset_chip reloading chip"
        end
        if ($T1 == 0x13)
            printf "isp.as - isp_reset_chip completing mailbox with error"
        end
        if ($T1 == 0x14)
            printf "isp.as - isp_reset_chip completing ilt with error"
        end
        if ($T1 == 0x15)
            printf "isp.as - isp_reset_chip releasing resilk (interlock)"
        end
        if ($T1 == 0x20)
            printf "ispc.c - isp_exec_cmd_sri mailbox failed"
        end
        if ($T1 == 0x30)
            printf "ispc.c - isp_processIdAcquisition set vport for target"
        end
        if ($T1 == 0x4D)
            printf "ispc.c - isp_exec_cmd_sri/ISP2400_GetMem mailbox-failed"
        end
        if ($T1 == 0xAE)
            printf "ispc.c - ISP_monitor_async starting loop"
        end
    end
    if ($T0 == 0x20)
        printf "physical.as - p$exec start request"
    end
    if ($T0 == 0x21)
        printf "physical.as - p$comp_ilt physical completion code"
    end
    if ($T0 == 0x22)
        printf "physical.as - p$cancel cancel redundant write request"
    end
    if ($T0 == 0x23)
        printf "physical.as - p$join join two requests into one larger request"
    end
    if ($T0 == 0x24)
        printf "physical.as - p$get_ilt get next ilt for device"
    end
    if ($T0 == 0x26)
        if ($T1 == 0x10)
            printf "fabric.c - f_initDevice"
        end
        if ($T1 == 0x20)
            if ($T3 == 0x00)
                printf "fabric.c - F_find_dev entering"
            end
            if ($T3 == 0x01)
                printf "fabric.c - F_find_dev wakeup"
            end
            if ($T3 == 0x02)
                printf "fabric.c - F_find_dev f_find_pdd"
            end
            if ($T3 == 0x03)
                printf "fabric.c - F_find_dev pDev exists"
            end
            if ($T3 == 0x04)
                printf "fabric.c - F_find_dev pDev does not exist"
            end
            if ($T3 == 0x05)
                printf "fabric.c - F_find_dev pdd does not exist (new device) - *** DC_AllocPDD ***"
            end
            if ($T3 == 0x09)
                printf "fabric.c - F_find_dev exiting"
            end
        end
        if ($T1 == 0x30)
            printf "physical.as - p$init_drv entering"
        end
        if ($T1 == 0x40)
            if ($T3 == 0x00)
                printf "fabric.c - F_moveDevice to new port"
            end
            if ($T3 == 0x01)
                printf "fabric.c - F_moveDevice new port, head of dev list"
            end
            if ($T3 == 0x02)
                printf "fabric.c - F_moveDevice new port, head of start list"
            end
            if ($T3 == 0x03)
                printf "fabric.c - F_moveDevice before removing DEV"
            end
            if ($T3 == 0x04)
                printf "fabric.c - F_moveDevice middle removing DEV"
            end
        end
        if ($T1 == 0x50)
            printf "fabric.c - FAB_removeDevice"
        end
        if ($T1 == 0x60)
            printf "fabric.c - F_findAltPort"
        end
        if ($T1 == 0x70)
            printf "fabric.c - f_purgeDevices"
        end
        if ($T1 == 0x80)
            printf "fabric.c - FAB_removeDevice"
        end
        if ($T1 == 0x90)
            printf "fabric.c - f_detachPort"
        end
        if ($T1 == 0xA0)
            if ($T3 == 0x00)
                printf "fabric.c - discover_fibre_devices fabric up"
            end
            if ($T3 == 0x01)
                printf "fabric.c - F_startPortMonitor start task"
            end
            if ($T3 == 0x02)
                printf "fabric.c - F_startPortMonitor task already running"
            end
            if ($T3 == 0x03)
                printf "fabric.c - discover_loop_devices"
            end
            if ($T3 == 0x04)
                printf "fabric.c - stuff/discover_loop_devices/discover_fabric_devices req up"
            end
            if ($T3 == 0x05)
                printf "fabric.c - stuff/discover_loop_devices LID"
            end
            if ($T3 == 0x08)
                printf "fabric.c - stuff/discover_loop_devices non-xiotech "
            end
            if ($T3 == 0x0A)
                printf "fabric.c - process_loop_up_request lip issued"
            end
            if ($T3 == 0x0C)
                printf "fabric.c - process_loop_down_request lip issued"
            end
            if ($T3 == 0x0F)
                printf "fabric.c - f_portMonitor clear discovery PCB"
            end
        end
        if ($T1 == 0xB0)
            printf "fabric.c - process_loop_down_request req down"
        end
        if ($T1 == 0xC0)
            if ($T3 == 0x00)
                printf "fabric.c - f_discovery"
            end
            if ($T3 == 0x01)
                printf "fabric.c/physical_mon.c - F_startDiscovery/start_discovery"
            end
            if ($T3 == 0x02)
                printf "fabric.c/physical_mon.c - F_startDiscovery/start_discovery task already running"
            end
            if ($T3 == 0x03)
                printf "fabric.c - f_discovery done, clearing PCB"
            end
        end
        if ($T1 == 0xD0)
            if ($T3 == 0x00)
                printf "fabric.c - F_rescanDevice entering"
            end
            if ($T3 == 0x02)
                printf "fabric.c/physical_mon.c - F_rescanDevice/PH_rescanDevice wait for temp PDD processing to complete"
            end
            if ($T3 == 0x0F)
                printf "fabric.c/physical_mon.c - F_rescanDevice/PH_rescanDevice rescan complete"
            end
        end
        if ($T1 == 0xE0)
            if ($T3 == 0x00)
                printf "fabric.c - FAB_ProcessRSCN/FAB_BypassDevice entering"
            end
            if ($T3 == 0x01)
                printf "fabric.c - FAB_BypassDevice bypass done"
            end
            if ($T3 == 0x0F)
                printf "fabric.c - FAB_BypassDevice exiting"
            end
        end
        if ($T1 == 0xE1)
            printf "fabric.c - FAB_ProcessRSCN port/loop down"
        end
        if ($T1 == 0xE2)
            printf "fabric.c - FAB_ProcessRSCN not mag3d"
        end
        if ($T1 == 0xE3)
            printf "fabric.c - FAB_ProcessRSCN get name server entry"
        end
        if ($T1 == 0xE4)
            printf "fabric.c - FAB_ProcessRSCN login to device"
        end
        if ($T1 == 0xE5)
            printf "fabric.c - FAB_ProcessRSCN invalidate LID"
        end
        if ($T1 == 0xE7)
            printf "ispc.c - fabric logout to Port ID"
        end
        if ($T1 == 0xE8)
            printf "dlmbe.as - dlm$precedence"
        end
        if ($T1 == 0xE9)
            printf "fabric.c - FAB_ProcessRSCN exiting"
        end
        if ($T1 == 0xEa)
            printf "dlm.as - .senddg2iltcr_100 datagram not successful"
        end
        if ($T1 == 0xF0)
            if ($T3 == 0x00)
                printf "fabric.c - f_lunDiscovery entering"
            end
            if ($T3 == 0x01)
                printf "fabric.c - f_lunDiscovery p$init_drv created and taskswitched"
            end
        end
    end
    if ($T0 == 0x27)
        printf "physical.as - p$check4retry entering"
    end
    if ($T0 == 0x28)
        printf "physical.as - p$check4retry check condition"
    end
    if ($T0 == 0x29)
        if ($T1 == 0x00)
            printf "physical.as - p$check4retry recovery action taken"
        end
        if ($T1 == 0x10)
            printf "physical.as - p$setup4retry/p$check4retry LIP sent"
        end
        if ($T1 == 0x47)
            printf "physical.as - p$login"
        end
    end
    if ($T0 == 0x2A)
        printf "physical.as - p$check4retry retry scsi task"
    end
    if ($T0 == 0x30)
        printf "raid.as - r$exec process raid request"
    end
    if ($T0 == 0x31)
        printf "raid.as - r$comp complete raid request"
    end
    if ($T0 == 0x32)
        printf "raid5.as - r$r5pcrcomp2 raid5 parity wrong"
    end
    if ($T0 == 0x33)
        printf "raid5.as - r$insrrb insert rrb into prn node"
    end
    if ($T0 == 0x34)
        printf "raid5.as - r$comprrb raid 5 complete RRB"
    end
    if ($T0 == 0x40)
        printf "virtual.as - v$exec - .vex55"
    end
    if ($T0 == 0x41)
        printf "virtual.as - v$vscomp completion of single RRP"
    end
    if ($T0 == 0x42)
        printf "virtual.as - v$vmcomp completion of RRP requests"
    end
    if ($T0 == 0x50)
        printf "LL_LinuxLinkLayer.c - LL_QueueMessageToSend"
    end
    if ($T0 == 0x51)
        printf "LL_LinuxLinkLayer.c - LL_NormalInitiatorProcessing outbound VRP exec"
    end
    if ($T0 == 0x52)
        printf "LL_LinuxLinkLayer.c - LL_NormalTargetProcessing inbound VRP exec"
    end
    if ($T0 == 0x53)
        printf "LL_LinuxLinkLayer.c - LL_TargetTaskCompletion completion routine"
    end
    if ($T0 == 0x54)
        printf "LL_LinuxLinkLayer.c - LL_NormalCompletionProcessing completion exec"
    end
    if ($T0 == 0x55)
        printf "LL_LinuxLinkLayer.c - Link layer processor sync"
    end
    if ($T0 == 0x60)
        printf "cachefe.as - C$que entering"
    end
    if ($T0 == 0x61)
        printf "cachefe.as - c$exec initial overlap check"
    end
    if ($T0 == 0x62)
        printf "cachefe.as - c$qio entering"
    end
    if ($T0 == 0x63)
        printf "cachefe.as - c$ioexec processing"
    end
    if ($T0 == 0x64)
        printf "cachefe.as - c$ncrcomp1 non-cached read complete 1"
    end
    if ($T0 == 0x65)
        printf "cachefe.as - c$ncrcomp2 non-cached read complete 2"
    end
    if ($T0 == 0x66)
        printf "cachefe.as - c$ncwcomp1 non-cached write complete 1"
    end
    if ($T0 == 0x67)
        printf "cachefe.as - c$ncwcomp2 non-cached write complete 2"
    end
    if ($T0 == 0x68)
        printf "cachefe.as - c$nlevelcomp Next Level Completion"
    end
    if ($T0 == 0x69)
        printf "cachefe.as - c$calllower Call lower function"
    end
    if ($T0 == 0x6A)
        printf "cachefe.as - c$callupper Call upper function"
    end
    if ($T0 == 0x6B)
        printf "cachefe.as - c$getwdcomp Write Cache Data complete"
    end
    if ($T0 == 0x6C)
        printf "wcache.as - wc$io_comp completion of I/Osts"
    end
    if ($T0 == 0x6E)
        printf "cachefe.as - c$drpexec processing"
    end
    if ($T0 == 0x6F)
        printf "cachefe.as - c$drpcomp DRP completion"
    end
    if ($T0 == 0x70)
        printf "WC_WRP.c - Write Cache WRP executive"
    end
    if ($T0 == 0x71)
        printf "WC_WRP.c - WRP Queue Function"
    end
    if ($T0 == 0x72)
        printf "WC_WRP.c - TDis Executive"
    end
    if ($T0 == 0x73)
        printf "WC_WRP.c - VDisk temporary Disable function"
    end
    if ($T0 == 0x74)
        printf "WC_WRP.c - VDisk Disable Complete"
    end
    if ($T0 == 0x80)
        printf "cachefe.as - CDriver/MagDriver"
    end
    if ($T0 == 0xB2)
        printf "pm.as - M$ailtw (allocate ilt)"
    end
    if ($T0 == 0xB3)
        printf "pm.as - M$rilt (release ilt)"
    end
    if ($T0 == 0xC0)
        printf "kernel.as - k$malloc memory allocation"
    end
    if ($T0 == 0xC1)
        printf "kernel.as - k$mrel memory release"
    end
    if ($T0 == 0xC2)
        printf "kernel.as - k$dmrel_xxx deferred memory release"
    end
    if ($T0 == 0xC3)
        printf "kernel.as - context switch"
    end
    if ($T0 == 0xD0)
        if ($T1 == 0x00)
            printf "online.as - o$hotswap entering"
        end
        if ($T1 == 0x01)
            printf "online.as - o$hotswap check for drives"
        end
        if ($T1 == 0x02)
            printf "online.as - o$hotswap processing missing device"
        end
        if ($T1 == 0x03)
            printf "online.as - o$hotswap send inquiry to all devices to update PDD status"
        end
    end
    if ($T0 == 0xD1)
        if ($T1 == 0x00)
            printf "online.as - o$inquire entering *** possible D_freepdd ***"
        end
        if ($T1 == 0x01)
            printf "online.as - o$inquire pre O$init_drv call"
        end
        if ($T1 == 0x02)
            printf "online.as - o$inquire inquire function (previous operable)"
        end
        if ($T1 == 0x03)
            printf "online.as - o$inquire inquire function (previous not operable)"
        end
    end
    if ($T0 == 0xD2)
        if ($T1 == 0x00)
            printf "online.as - O$init_drv entering"
        end
        if ($T1 == 0x01)
            printf "online.as - O$init_drv exiting"
        end
    end
    if ($T0 == 0xD3)
        if ($T1 == 0x00)
            printf "definebe.as/rebld.as - D$p2update/.hs10 entering"
        end
        if ($T1 == 0x02)
            printf "rebld.as - rb$canrebuild exiting"
        end
        if ($T1 == 0x03)
            printf "rebld.as - RB$canspare exiting"
        end
        if ($T1 == 0x04)
            printf "rebuild.c - RB_FindHotSpare"
        end
        if ($T1 == 0x05)
            printf "rebld.as - RB_setraidstat set status of RDD"
        end
        if ($T1 == 0x06)
            printf "rebld.as - RB_setpsdstat set status of PSD and RDD"
        end
        if ($T1 == 0x07)
            printf "rebld.as - o$getraiderrorstat get status of raid"
        end
        if ($T1 == 0x08)
            printf "raidinit.as - start raid initialize"
        end
        if ($T1 == 0x09)
            printf "definebe.as - not referenced?"
        end
        if ($T1 == 0x0B)
            printf "rebld.as - RB$faildev process fail device MRP"
        end
        if ($T1 == 0x0D)
            printf "nvram.c - NV_P2GenerateImage entering"
        end
        if ($T1 == 0x0E)
            printf "nvram.c - NV_RestoreNvram entering *** possible DC_AllocPDD ***"
        end
        if ($T1 == 0x10)
            printf "online.as - O$writefailedlabel entering"
        end
        if ($T1 == 0x11)
            printf "online.as - O$writefailedlabel exiting"
        end
    end
    if ($T0 == 0xD4)
        if ($T1 == 0x00)
            printf "rebld.as - rb$rebuildpsd rebuilding"
        end
        if ($T1 == 0x01)
            printf "rebld.as - rb$rebuildpsd rebuilding, spawning rebuild task"
        end
        if ($T1 == 0x02)
            printf "rebld.as - rb$redirectpsd entering"
        end
        if ($T1 == 0x03)
            printf "rebld.as - rb$psd_rebuilder"
        end
        if ($T1 == 0x04)
            printf "rebld.as - rb$psd_rebuilder .dbr630"
        end
        if ($T1 == 0x05)
            printf "rebld.as - rb$psd_rebuilder .dbr930"
        end
        if ($T1 == 0x06)
            printf "rebld.as - rb$psd_rebuilder completion of PID rebuild"
        end
        if ($T1 == 0x07)
            printf "rebld.as - rb$psd_rebuilder abort of rebuild"
        end
        if ($T1 == 0x08)
            printf "rebld.as - RB_SearchForFailedPSDs entering"
        end
        if ($T1 == 0x09)
            printf "rebld.as - RB_SearchForFailedPSDs exiting"
        end
        if ($T1 == 0x0A)
            printf "rebuild.c - RB_UpdateRaidRebuildWritesStatus"
        end
    end
    if ($T0 == 0xD5)
        if ($T1 == 0x00)
            printf "rebld.as - RB$error raid error"
        end
        if ($T1 == 0x01)
            printf "rebld.as - rb$rerror_exec main raid error handling loop"
        end
        if ($T1 == 0x02)
            printf "rebld.as - rb$rerror_exec check raid to see if should hotspare"
        end
        if ($T1 == 0x03)
            printf "rebld.as - RB$error see if rebuild required"
        end
    end
    if ($T0 == 0xDD)
        if ($T1 == 0x0D)
            printf "ispc.c - ISP_DumpQL"
        end
    end
    if ($T0 == 0xE0)
        printf "dlmbe.as - rrp received"
    end
    if ($T0 == 0xE1)
        printf "dlm.as - message execution"
    end
    if ($T0 == 0xE2)
        printf "dlm.as - datagram sent"
    end
    if ($T0 == 0xE3)
        printf "dlmfe.as - Queued to BE SRP executive"
    end
    if ($T0 == 0xE4)
        printf "dlmfe.as - BE SRP executive"
    end
    if ($T0 == 0xE5)
        printf "dlmfe.as - Queued to DRP executive"
    end
    if ($T0 == 0xE6)
        printf "dlmfe.as - DRP executive"
    end
    if ($T0 == 0xE7)
        printf "dlmbe.as - No VLOP"
    end
    if ($T0 == 0xE8)
        printf "dlmbe.as - Starting precedence"
    end
    if ($T0 == 0xE9)
        printf "dlmbe.as - Resync"
    end
    if ($T0 == 0xEA)
        printf "dlm.as - DG 2 ILT completion routine"
    end
    printf "\n"
end
#...
document i_print_flight
Internal macro to print flight recorder information.
end
#-----------------------------------------------------------------------------
# This macro prints out the flight recorder information (no time stamp).
define flight
    set $I = (UINT32)fr_queue
    if ($I != 0)
        set $BEGIN = *((UINT32*)($I+0))
        set $NEXT = *((UINT32*)($I+4))
        set $BEGIN2 = *((UINT32*)($I+8))
        set $END = *((UINT32*)($I+12))
# From NEXT to END, BEGIN to NEXT, display entries.
        set $J = $NEXT
        while ($J < $END)
          i_print_flight $J
          set $J = $J+16
        end
        set $J = $BEGIN
        while ($J < $NEXT)
          i_print_flight $J
          set $J = $J+16
        end
    end
end
#...
document flight
Print the flight recorder information.
end
#-----------------------------------------------------------------------------
# This is internal macro to print out the mrp trace log.
define i_print_mrp
    set $K = $arg0
    set $L = (UINT32)$arg1
    set $p0 = *((unsigned int *)$K)
    set $p1 = *((unsigned int *)($K+4))
    set $p2 = *((unsigned int *)($K+8))
    set $p3 = *((unsigned int *)($K+12))
    printf "%4d: ", $L
    if ($p0 == 0x80000000)
      printf "Start  %d (0x%-4.4x)        at ", $p1, $p1
    else
      if ($p0 == 0x800000FF)
        printf "Finish %d (0x%-4.4x) with %d at ", $p1 & 0xff, $p1 & 0xff, $p1>>8
      else
        if ($p0 == 0xF00000FF)
          printf "Complt %d (0x%-4.4x) with %d at ", $p1 & 0xff, $p1 & 0xff, $p1>>8
        else
          printf "unknown p0=0x%-8.8x, p1=0x%-8.8x at ", $p1, $p2
        end
      end
    end
    printf "0x%8.8x.%8.8x\n", $p2, $p3
end
#...
document i_print_mrp
Internal macro to print mrp information.
end
#-----------------------------------------------------------------------------
# This macro prints out the mrptracelog (defTraceQue).
define mrptracelog
    set $Q = *((UINT32*)(&ptrdefTraceQue))
    set $BEGIN = *((UINT32*)($Q+0))
    set $NEXT = *((UINT32*)($Q+4))
    set $END = *((UINT32*)($Q+8))
    set $PCB = *((UINT32*)($Q+12))
    printf "Start MRP tracelog:\n"
    set $CNT = 1023
# From NEXT to END, BEGIN to NEXT, display entries.
    set $J = $NEXT
    while ($J < $END && $CNT > 0)
      i_print_mrp $J $CNT
      set $J = $J+16
      set $CNT = $CNT-1
    end
    set $J = $BEGIN
    while ($J < $NEXT && $CNT > 0)
      i_print_mrp $J $CNT
      set $J = $J+16
      set $CNT = $CNT-1
    end
end
#...
document mrptracelog
Print the mrp trace log (defTraceQue).
end
#-----------------------------------------------------------------------------
# This macro prints out the link layer queues (between FE/BE/CCB).
define linkqueues
    printf "------------------------------------------------------------------------------\n"
    printf "Inter-process link queues:\n"
    p LINK_QCS
    printf "\n"
    set $Ilq = 0
    while ($Ilq < 8)
      if (LINK_QCS.pqc_QB[$Ilq] != 0)
        set $J = LINK_QCS.pqc_QB[$Ilq]
        printf "Handler %d: flags=0x%02x ord=0x%02x size=0x%04x (%d)  lowWater=0x%04x max=0x%04x\n", $Ilq, $J->qb_flags, $J->qb_ord, $J->qb_size, $J->qb_size, $J->qb_lowWater, $J->qb_max
        printf "           qb_pstat=0x%08x pqb_First=0x%08x pqb_Last=0x%08x\n", $J->qb_pstat, $J->pqb_First, $J->pqb_Last
        set $Icnt = 0
        i_linkilt $Ilq
        while ($ILT !=0)
            set $Icnt = $Icnt + 1
            i_linknextilt $Icnt 0
            if ($ILT == 0 || $ILT == $J->pqb_First)
              set $ILT = 0
            end
        end
        if ($J->qb_size != $Icnt)
            printf "ERROR - qb_size %d does not match ILT list count of %d\n", $J->qb_size, $Icnt
        end
        printf "\n"
      else
        printf "No Handler task %d\n", $Ilq
      end
      set $Ilq = $Ilq + 1
    end
    set $I = 0
    while ($I < 2)
      set $lILT = pMySharedMem->SHMLLI[$I].pLLActiveILTHead
      set $eILT = pMySharedMem->SHMLLI[$I].pLLActiveILTTail
      if ($lILT == $eILT)
        set $lILT = 0
        printf "pLLActiveILTHead[%d] no Active ILTs.\n", $I
      end
      set $C = 1
      while ((UINT32)$lILT != 0)
        i_find_ilt_base $lILT
        set $oILT = ($lILT-1)
        set $oVRP = ((UINT32*)$oILT)[8]
        set $pMRP = (MR_PKT *)$oVRP
        set $nILT = $lILT->misc
        set $nVRP = $lILT->cr
        printf "pLLActiveILTHead[%d] #%d - ILT level %d=%p  base address=%p\n", $I, $C, ($lILT-$ILT), $lILT, $ILT
        if (($pMRP->function == 0x300) || ($pMRP->function == 0x301))
            set $pLOG = (LOG_HEADER_PKT *)($pMRP->pReq)
            printf "    old ILT/VRP/func/event=%p/%p/0x%04x/0x%08x  new ILT/VRP=%p/%p\n", $oILT, $oVRP, $pMRP->function, $pLOG->event, $nILT, $nVRP
        else
            printf "    old ILT/VRP/func/event=%p/%p/0x%04x/0x00000000  new ILT/VRP=%p/%p\n", $oILT, $oVRP, $pMRP->function, $nILT, $nVRP
        end
        set $C = $C + 1
        if ($lILT == $eILT)
          set $lILT = 0
        else
          if ($lILT == $lILT->fthd)
            printf "ERROR - ILT (%p) = ILT->fthd (%p)\n", $ILT, $ILT->fthd
            set $lILT = 0
          else
            set $lILT = $lILT->fthd
          end
        end
      end
      set $I = $I + 1
    end
end
#...
document linkqueues
Print the link layer queues.
end
#-----------------------------------------------------------------------------
# This finds base of ilt by subtracting 0x34 till bottom 6 bits are zero (but max of 7/11).
define i_find_ilt_base
  set $ILT = (ILT*)$arg0
  if (((UINT32)(&K_ii) & 0xffff0000) == $FE_MEMORY_STARTS)
    set $ILTNEST = 11
  else
    set $ILTNEST = 7
  end
  set $cnt = 0
# Originally (before new memory pool with pre-post patterns of 32 bytes), the
# knowledge that an ILT was on a 64 byte aligned memory location was counted on.
  set $ALIGNMENT = 0
#  while ((((UINT32)$ILT & 63) != 0) && ($cnt < $ILTNEST))
# Now, we know that it is 64+32 byte aligned. (Good luck figuring that out.)
  while ((((UINT32)$ILT & 63) != $ALIGNMENT) && ($cnt < $ILTNEST))
    set $ILT = $ILT - 1
    set $cnt = $cnt + 1
  end
end
#...
document i_find_ilt_base
Internal macro to find the ilt base address (if possible).
# This finds base of ilt by subtracting 0x34 till bottom 6 bits are zero (but max of 7/11).
end
#-----------------------------------------------------------------------------
# This internal macro prints out the ILT from base for all 7/11 layers, with possible VRP one back.
define i_printilt_vrp
  set $p_ILT = $arg0
  set $p_CILT = $arg1
  set $p_cnt = $arg2
  set $p_PILT = $arg3
  set $p_VRP = $arg4
  if ($p_cnt == $ILTNEST)
    printf "i_printilt_vrp - Could not calculate base of ILT\n"
  else
    set $i_cnt = 0
    while ($i_cnt < $ILTNEST)
      if ((UINT32)$p_PILT == 0 || (UINT32)$p_PILT == (UINT32)$p_ILT || $p_cnt == $i_cnt)
        if ($p_ILT == $p_CILT)
          printf "*"
        else
          printf " "
        end
        printf "ILT level %d at 0x%08x\n", $i_cnt, $p_ILT
        printf "    fthd=0x%08x bthd=0x%08x misc=0x%08x linux_val=0x%08x\n", $p_ILT->fthd, $p_ILT->bthd, $p_ILT->misc, $p_ILT->linux_val
        printf "    w0=0x%08x   w1=0x%08x   w2=0x%08x   w3=0x%08x\n", ((UINT32*)$p_ILT)[4], ((UINT32*)$p_ILT)[5], ((UINT32*)$p_ILT)[6], ((UINT32*)$p_ILT)[7]
        printf "    w4=0x%08x   w5=0x%08x   w6=0x%08x   w7=0x%08x\n", ((UINT32*)$p_ILT)[8], ((UINT32*)$p_ILT)[9], ((UINT32*)$p_ILT)[10], ((UINT32*)$p_ILT)[11]
        printf "    cr=0x%08x -- ", $p_ILT->cr
        if ($p_ILT->cr != 0xcacacaca)
          set $cr = ((UINT32)($p_ILT->cr)&0x7fffffff)
          info symbol $cr
        else
          printf "\n"
        end
        if ((UINT32)$p_PILT != 0 && $p_PILT == $p_ILT)
          # print out VRP at this level.
          printf "  VRP function=%d strategy=%d status=0x%2.2x vid=%d path=%d options=0x%2.2x\n", $p_VRP->function, $p_VRP->strategy, $p_VRP->status, $p_VRP->vid, $p_VRP->path, $p_VRP->options
          printf "      length=%d pktAddr=%p startDiskAddr=%lld pSGL=%p sglSize=%d\n", $p_VRP->length, $p_VRP->pktAddr, $p_VRP->startDiskAddr, $p_VRP->pSGL, $p_VRP->sglSize
          printf "      gen0=0x%08x gen1=0x%08x gen2=0x%08x gen3=0x%08x\n", $p_VRP->gen0, $p_VRP->gen1, $p_VRP->gen2, $p_VRP->gen3
        end
      end
      set $p_ILT = $p_ILT + 1
      set $i_cnt +=  1
    end
  end
end
#...
document i_printilt_vrp
Internal Macro to print all 7 (BE) or 11 (FE) layers of an ILT with possible VRP at location given.
end
#-----------------------------------------------------------------------------
# This macro prints out the ILT from base for all 7/11 layers.
define printilt
  set $P_ILT = $arg0
  i_find_ilt_base $P_ILT
  i_printilt_vrp $ILT $P_ILT $cnt 0 0
end
#...
document printilt
Macro to print all 7 (BE) or 11 (FE) layers of an ILT.
end
#-----------------------------------------------------------------------------
# This macro prints out the ILT and readies for "next".
define i_linknextilt
  if ($ILT != 0)
    printf "%2d:ILT(0x%08x) fthd=0x%08x bthd=0x%08x misc=0x%08x linux_val=0x%x\n", $arg0, $ILT, $ILT->fthd, $ILT->bthd, $ILT->misc, $ILT->linux_val
    printf "                w0=0x%08x w1=0x%08x w2=0x%08x w3=0x%08x\n", ((UINT32*)$ILT)[4], ((UINT32*)$ILT)[5], ((UINT32*)$ILT)[6], ((UINT32*)$ILT)[7]
    printf "                w4=0x%08x w5=0x%08x w6=0x%08x w7=0x%08x\n", ((UINT32*)$ILT)[8], ((UINT32*)$ILT)[9], ((UINT32*)$ILT)[10], ((UINT32*)$ILT)[11]
    printf "       cr=0x%08x ", $ILT->cr
    if ($ILT->cr != 0xcacacaca && (UINT32)$ILT->cr < (UINT32)$FE_MEMORY_STARTS)
      info symbol $ILT->cr
      if ($arg1 == 0)
        set $pVRPCMD = (VRPCMD*)&(((UINT32*)$ILT)[4])
        set $ILTM1 = $ILT-1
#        set $RRP = (RRP*)(((UINT32*)$ILTM1)[4])
        set $RRP = (((UINT32*)$ILTM1)[4])
        if ((UINT32)$RRP > $EXECUTABLE_START && (UINT32)$RRP < (UINT32)0xf0000000)
#         printf "   RRP=%08x RRP function=0x%04x, SDA= %016x\n", $RRP, $RRP->function, $RRP->startDiskAddr
          printf "   RRP=%08x  rid=%d, function=0x%04x, SDA=%016x\n", $RRP, *((UINT16*)$RRP+2), *(UINT16*)$RRP, *(UINT64*)($RRP+16)
        end
        if ((UINT32)$pVRPCMD > $EXECUTABLE_START)
          set $pMRP = (MR_PKT *)($pVRPCMD->pvrVRP)
          if ((UINT32)$pMRP > $EXECUTABLE_START && (UINT32)$pMRP < (UINT32)0xf0000000)
            set $pReq = (LOG_HEADER_PKT *)($pMRP->pReq)
#.............................................................................
#   p *$pMRP
#           printf "   MRP function=0x%04x, version=0x%02x, pReqPCI=0x%08x\n", $pMRP->function, $pMRP->version, $pMRP->pReqPCI,
#           printf "       pRsp=0x%08x, rspLen=0x%04x, pReq=0x%08x, reqLen=0x%04x\n", $pMRP->rspLen, $pMRP->pRsp, $pMRP->pReq, $pMRP->reqLen
#.............................................................................
#    p *$pReq
            if ((UINT32)$pReq > $EXECUTABLE_START && (UINT32)$pReq < (UINT32)0xf0000000)
              printf "   LOG_HEADER_PKT event=0x%08x (%d), length=0x%08x (%d)\n", $pReq->event, $pReq->event, $pReq->length, $pReq->length
            end
          end
        end
      end
    else
      printf "\n"
    end
    if ($ILT == $ILT->fthd)
      printf "ERROR - ILT (%p) = ILT->fthd (%p)\n", $ILT, $ILT->fthd
      set $ILT = 0
    else
      set $ILT = $ILT->fthd
    end
  end
end
#...
document i_linknextilt
Internal macro to print an ILT. See linkqueues and i_linkilt.
end
#-----------------------------------------------------------------------------
# This macro prints out the first link layer queue ilt address.
# $FE_MEMORY_STARTS is where FE shared memory is.
define i_linkilt
  set $Iilt = $arg0
  if (((UINT32)(&K_ii) & 0xffff0000) == $FE_MEMORY_STARTS)
    set $ILTNEST = 11
  else
    set $ILTNEST = 7
  end
  if ($Iilt >= 0 && $Iilt < $ILTNEST)
    if (LINK_QCS.pqc_QB[$Iilt] != 0)
      set $ILT = LINK_QCS.pqc_QB[$Iilt].pqb_First
      if ($ILT != 0)
#        printf "First ILT for queue %d:\n", $Iilt
        i_linknextilt 1 0
        set $Icnt = 1
      else
        printf "No ILTs for queue %d\n", $Iilt
      end
    end
  end
end
#...
document i_linkilt
Internal macro to print entry in link.pqc_Qb[#] queue. See linkqueues and i_linknextilt.
end
#-----------------------------------------------------------------------------
# This macro dumps the Link Layer queue # arg0.
#
define linkDump
  set $ILT = 0
  set $LAST = 0
  set $I = $arg0
  if (((UINT32)(&K_ii) & 0xffff0000) == $FE_MEMORY_STARTS)
    set $ILTNEST = 11
  else
    set $ILTNEST = 7
  end
  if ($I >= 0 && $I < $ILTNEST)
    if (LINK_QCS.pqc_QB[$I] != 0)
      printf "First ILT for queue %d:\n", $I
      set $ILT = LINK_QCS.pqc_QB[$I].pqb_First
      set $LAST = LINK_QCS.pqc_QB[$I].qb_size
    end
  end
#  printf "SN     FUNCT   EVENT     LENGTH      ILT ADDR    MRP ADDR    sVID  dVID  func  stat  pcnt\n"
#  printf "----   ------  --------  ----------  ----------  ----------  ----  ----  ----  ----  ----\n"
  printf "SN     FUNCT   EVENT     LENGTH      ILT ADDR    MRP ADDR\n"
  printf "----   ------  --------  ----------  ----------  ----------\n"
  set $CNT = 1
  while ($CNT <= $LAST & $ILT != 0)
    set $pVRPCMD = (VRPCMD*)&(((UINT32*)$ILT)[4])
    set $pMRP = (MR_PKT *)($pVRPCMD->pvrVRP)
    set $pReq = (LOG_HEADER_PKT *)($pMRP->pReq)
    if ($pReq->event & 0x1000)
      set $DHC = 68
    else
      if ($pReq->event & 0x2000)
        set $DHC = 72
      else
        set $DHC = 67
      end
    end
    if (($pReq->event & 0xC000) == 0xC000)
      set $IWEF = 70
    else
      if ($pReq->event & 0x8000)
        set $IWEF = 69
      else
        if ($pReq->event & 0x4000)
          set $IWEF = 87
        else
          set $IWEF = 73
        end
      end
    end
    set $EVNT = $pReq->event & 0xFFF
    printf "%4d   0x%04x  %c%c 0x%03x  0x%08x  0x%08x  0x%08x", $CNT, $pMRP->function, $IWEF, $DHC, $EVNT, $pReq->length, $ILT, $pMRP
    if ($pMRP->function == 0x301)
      if ($EVNT == 0x007)
    # Copy complete (LOG_COPY_COMPLETE)
    # LOG_COPY_COMPLETE_DAT typedef logdef.h ... not in be/fe, only ccb, thus decode as below:
# sVID  dVID  func  stat  pcnt\n"
        printf " COPY_COMPLETE"
        set $STAT = *((uint8_t *)(((uint8_t *)$pReq) + 8))
        set $FUNC = *((uint8_t *)(((uint8_t *)$pReq) + 9))
        set $PCNT = *((uint8_t *)(((uint8_t *)$pReq) + 10))
        set $SVID = *((uint8_t *)(((uint8_t *)$pReq) + 12))
        set $DVID = *((uint8_t *)(((uint8_t *)$pReq) + 14))
        if ($STAT == 0x19)
          printf " Auto-Pause"
        else
          if ($STAT == 0x1A)
            printf " Copy-Resume"
          else
            printf " stat=0x%2x", $STAT
          end
        end
        printf " sVID=%d dVID=%d %%=%d funct=0x%2x\n", $SVID, $DVID, $PCNT, $FUNC
      else
        printf " Event=0x%x\n", $EVNT
      end
    end
    set $ILT = $ILT->fthd
    set $CNT = $CNT + 1
  end
  if ($CNT != ($LAST + 1) || $ILT != 0)
    printf "count of %d != %d, or ILT (%p) != 0\n", $CNT, $LAST, $ILT
  end
end
#...
document linkDump
Macro linkDump prints and traverses Link Layer queue (argument 0 to 7).
end
#-----------------------------------------------------------------------------
# The "i_printnormalqueue" is an internal macro to print out a queue containing ILT's.
define i_printnormalqueue
    set $Q = $arg0
    printf " 0x%08x 0x%08x 0x%08x(%d) 0x%08x\n", $Q->head, $Q->tail, $Q->qcnt, $Q->qcnt, $Q->pcb
    set $ILT = $Q->head
    set $C_ILT = 0
    if ($ILT != 0)
        while ($ILT !=0)
            set $C_ILT = $C_ILT + 1
            i_linknextilt $C_ILT 0
        end
    end
    if ($Q->qcnt != $C_ILT)
        printf "ERROR - qcnt %d does not match ILT list count of %d\n", $Q->qcnt, $C_ILT
    end
end
#...
document i_printnormalqueue
Internal macro to print out a queue containing ILT's.
end
#-----------------------------------------------------------------------------
# The "i_printr5execqueue" is an internal macro to print out a queue containing RPN's.
define i_printr5execqueue
    set $Q = $arg0
    printf " 0x%08x 0x%08x 0x%08x(%d) 0x%08x\n", $Q->head, $Q->tail, $Q->qcnt, $Q->qcnt, $Q->pcb
    set $RPN = (RPN*)($Q->head)
    set $C_RPN = 0
    if ($RPN != 0)
        while ($RPN !=0)
            set $C_RPN = $C_RPN + 1
            i_linknextrpn $C_RPN 0
        end
    end
    if ($Q->qcnt != $C_RPN)
        printf "ERROR - qcnt %d does not match RPN list count of %d\n", $Q->qcnt, $C_RPN
    end
end
#...
document i_printr5execqueue
Internal macro to print out the R_r5exec_qu containing RPN's.
end
#-----------------------------------------------------------------------------
# This macro prints out the RPN and readies for "next".
define i_linknextrpn
  if ($RPN != 0)
    printf "%2d:RPN(0x%08x) fthd=0x%08x bthd=0x%08x afthd=0x%08x abthd=0x%x\n", $arg0, $RPN, $RPN->fthd, $RPN->bthd, $RPN->afthd, $RPN->bthd
    printf "                lock=0x%02x act=0x%02x xpedite=0x%02x rsvd1=0x%02x\n", $RPN->lock, $RPN->act, $RPN->xpedite, $RPN->rsvd1
    printf "                rdd=0x%08x rrbhead=0x%08x rrbtail=0x%08x\n", $RPN->rdd, $RPN->rrbhead, $RPN->rrbtail
    printf "                spsd=0x%08x ppsd=0x%08x wpsd=0x%08x rsvd2=0x%08x\n", $RPN->spsd, $RPN->ppsd, $RPN->wpsd, $RPN->rsvd2
    printf "                lsda=0x%016llx spsda=0x%016llx stripe=0x%016llx\n", $RPN->lsda, $RPN->spsda, $RPN->stripe
    printf "                rsvd3=0x%016llx\n", $RPN->rsvd3
    if ($RPN == $RPN->afthd)
      printf "ERROR - RPN (%p) = RPN->afthd (%p)\n", $RPN, $RPN->afthd
      set $RPN = 0
    else
      set $RPN = $RPN->afthd
    end
  end
end
#...
document i_linknextrpn
Internal macro to print an RPN.
end
#-----------------------------------------------------------------------------
# The "i_print_mrp_function_name" is an internal macro to print out the MRP function name.
define i_print_mrp_function_name
    set $FUNCTION = $arg0
    if ($FUNCTION == 0x0100)
        printf "  ccb->be MRCREXP"
    end
    if ($FUNCTION == 0x0101)
        printf "  ccb->be MRGETELIST"
    end
    if ($FUNCTION == 0x0102)
        printf "  ccb->be MRLABEL"
    end
    if ($FUNCTION == 0x0103)
        printf "  ccb->be MRFAIL"
    end
    if ($FUNCTION == 0x0104)
        printf "  ccb->be MRSCSIIO"
    end
    if ($FUNCTION == 0x0105)
        printf "  ccb->be MRINITRAID"
    end
    if ($FUNCTION == 0x0106)
        printf "  ccb->be MROBSOLETE106"
    end
    if ($FUNCTION == 0x0107)
        printf "  ccb->be MRDELVIRT"
    end
    if ($FUNCTION == 0x0108)
        printf "  ccb->be MRSETCACHE"
    end
    if ($FUNCTION == 0x0109)
        printf "  ccb->be MRSERVERPROP"
    end
    if ($FUNCTION == 0x010A)
        printf "  ccb->be MRRESET"
    end
    if ($FUNCTION == 0x010B)
        printf "  ccb->be MRRESTORE"
    end
    if ($FUNCTION == 0x010C)
        printf "  ccb->be MRAWAKE"
    end
    if ($FUNCTION == 0x010D)
        printf "  ccb->be MRWWNLOOKUP"
    end
    if ($FUNCTION == 0x010E)
        printf "  ccb->be MRBEGENERIC"
    end
    if ($FUNCTION == 0x010F)
        printf "  ccb->be MRSTARTSTOP"
    end
    if ($FUNCTION == 0x0110)
        printf "  ccb->be MRSCRUBCTRL"
    end
    if ($FUNCTION == 0x0111)
        printf "  ccb->be MRDEFAULT"
    end
    if ($FUNCTION == 0x0112)
        printf "  ccb->be MRGETBEDEVPATHS"
    end
    if ($FUNCTION == 0x0113)
        printf "  ccb->be MRRESTOREDEV"
    end
    if ($FUNCTION == 0x0114)
        printf "  ccb->be MRDEFRAGMENT"
    end
    if ($FUNCTION == 0x0115)
        printf "  ccb->be MRSETATTR"
    end
    if ($FUNCTION == 0x0116)
        printf "  ccb->be MRBELOOP"
    end
    if ($FUNCTION == 0x0117)
        printf "  ccb->be MRGETSLIST"
    end
    if ($FUNCTION == 0x0118)
        printf "  ccb->be MRGETVLIST"
    end
    if ($FUNCTION == 0x0119)
        printf "  ccb->be MRGETRLIST"
    end
    if ($FUNCTION == 0x011A)
        printf "  ccb->be MRGETPLIST"
    end
    if ($FUNCTION == 0x011B)
        printf "  ccb->be MRGETMLIST"
    end
    if ($FUNCTION == 0x011C)
        printf "  ccb->be MRGETVINFO"
    end
    if ($FUNCTION == 0x011D)
        printf "  ccb->be MRGETRINFO"
    end
    if ($FUNCTION == 0x011E)
        printf "  ccb->be MRGETPINFO"
    end
    if ($FUNCTION == 0x011F)
        printf "  ccb->be MRMAPLUN"
    end
    if ($FUNCTION == 0x0120)
        printf "  ccb->be MRUNMAPLUN"
    end
    if ($FUNCTION == 0x0121)
        printf "  ccb->be MRGETEINFO"
    end
    if ($FUNCTION == 0x0122)
        printf "  ccb->be MRCREATESERVER"
    end
    if ($FUNCTION == 0x0123)
        printf "  ccb->be MRDELETESERVER"
    end
    if ($FUNCTION == 0x0124)
        printf "  ccb->be MRGETMINFO"
    end
    if ($FUNCTION == 0x0125)
        printf "  ccb->be MRVDISKCONTROL"
    end
    if ($FUNCTION == 0x0126)
        printf "  ccb->be MRASSIGNSYSINFO"
    end
    if ($FUNCTION == 0x0127)
        printf "  ccb->be MRBEII"
    end
    if ($FUNCTION == 0x0128)
        printf "  ccb->be MRBELINK"
    end
    if ($FUNCTION == 0x0129)
        printf "  ccb->be MRBEBOOT"
    end
    if ($FUNCTION == 0x012A)
        printf "  ccb->be MRBEDIAG"
    end
    if ($FUNCTION == 0x012B)
        printf "  ccb->be MRBEPROC"
    end
    if ($FUNCTION == 0x012C)
        printf "  ccb->be MRBECODEBURN"
    end
    if ($FUNCTION == 0x012D)
        printf "  ccb->be MRBRWMEM"
    end
    if ($FUNCTION == 0x012E)
        printf "  ccb->be MRCONFIGTARG"
    end
    if ($FUNCTION == 0x012F)
        printf "  ccb->be MRGETMPLIST"
    end
    if ($FUNCTION == 0x0130)
        printf "  ccb->be MRGLOBALPRI"
    end
    if ($FUNCTION == 0x0131)
        printf "  ccb->be MRGETTLIST"
    end
    if ($FUNCTION == 0x0132)
        printf "  ccb->be MRRESETBEPORT"
    end
    if ($FUNCTION == 0x0133)
        printf "  ccb->be MRNAMECHANGE"
    end
    if ($FUNCTION == 0x0134)
        printf "  ccb->be MRREMOTECTRLCNT"
    end
    if ($FUNCTION == 0x0135)
        printf "  ccb->be MRREMOTECTRLINFO"
    end
    if ($FUNCTION == 0x0136)
        printf "  ccb->be MRREMOTEVDISKINFO"
    end
    if ($FUNCTION == 0x0137)
        printf "  ccb->be MRFOREIGNTARGETS"
    end
    if ($FUNCTION == 0x0138)
        printf "  ccb->be MRCREATEVLINK"
    end
    if ($FUNCTION == 0x0139)
        printf "  ccb->be MRVLINKINFO"
    end
    if ($FUNCTION == 0x013A)
        printf "  ccb->be MRCREATECTRLR"
    end
    if ($FUNCTION == 0x013B)
        printf "  ccb->be MRRESCANDEVICE"
    end
    if ($FUNCTION == 0x013C)
        printf "  ccb->be MRRESYNC"
    end
    if ($FUNCTION == 0x013D)
        printf "  ccb->be MRGETLCLIMAGE"
    end
    if ($FUNCTION == 0x013E)
        printf "  ccb->be MRPUTLCLIMAGE"
    end
    if ($FUNCTION == 0x013F)
        printf "  ccb->be MRDELETEDEVICE"
    end
    if ($FUNCTION == 0x0140)
        printf "  ccb->be MRBEMODEPAGE"
    end
    if ($FUNCTION == 0x0141)
        printf "  ccb->be MRDEVICECOUNT"
    end
    if ($FUNCTION == 0x0142)
        printf "  ccb->be MRGETVIDOWNER"
    end
    if ($FUNCTION == 0x0143)
        printf "  ccb->be MRHOTSPAREINFO"
    end
    if ($FUNCTION == 0x0144)
        printf "  ccb->be MRFILECOPY"
    end
    if ($FUNCTION == 0x0145)
        printf "  ccb->be MRBEGETDVLIST"
    end
    if ($FUNCTION == 0x0146)
        printf "  ccb->be MRBEGETPORTLIST"
    end
    if ($FUNCTION == 0x0147)
        printf "  ccb->be MRBREAKVLOCK"
    end
    if ($FUNCTION == 0x0148)
        printf "  ccb->be MRGETSOS"
    end
    if ($FUNCTION == 0x0149)
        printf "  ccb->be MRPUTSOS"
    end
    if ($FUNCTION == 0x014A)
        printf "  ccb->be MRFORCEBEETRAP"
    end
    if ($FUNCTION == 0x014B)
        printf "  ccb->be MRPUTSCMT"
    end
    if ($FUNCTION == 0x014C)
        printf "  ccb->be MRBELOOPPRIMITIVE"
    end
    if ($FUNCTION == 0x014D)
        printf "  ccb->be MRTARGETCONTROL"
    end
    if ($FUNCTION == 0x014E)
        printf "  ccb->be MRFAILCTRL"
    end
    if ($FUNCTION == 0x014F)
        printf "  ccb->be MRNAMEDEVICE"
    end
    if ($FUNCTION == 0x0150)
        printf "  ccb->be MRPUTDG"
    end
    if ($FUNCTION == 0x0151)
        printf "  ccb->be MRNOPBE"
    end
    if ($FUNCTION == 0x0152)
        printf "  ccb->be MRPUTFSYS"
    end
    if ($FUNCTION == 0x0153)
        printf "  ccb->be MRGETDLINK"
    end
    if ($FUNCTION == 0x0154)
        printf "  ccb->be MRGETDLOCK"
    end
    if ($FUNCTION == 0x0155)
        printf "  ccb->be MRDEGRADEPORT"
    end
    if ($FUNCTION == 0x0156)
        printf "  ccb->be MRGETWSINFO"
    end
    if ($FUNCTION == 0x0157)
        printf "  ccb->be MRSETWSINFO"
    end
    if ($FUNCTION == 0x0158)
        printf "  ccb->be MRGETGPINFO"
    end
    if ($FUNCTION == 0x0159)
        printf "  ccb->be MRSETGPINFO"
    end
    if ($FUNCTION == 0x015A)
        printf "  ccb->be MRCHGRAIDNOTMIRRORING"
    end
    if ($FUNCTION == 0x015B)
        printf "  ccb->be MRPUTLDD"
    end
    if ($FUNCTION == 0x015C)
        printf "  ccb->be MRRAIDRECOVER"
    end
    if ($FUNCTION == 0x015D)
        printf "  ccb->be MRPUTDEVCONFIG"
    end
    if ($FUNCTION == 0x015E)
        printf "  ccb->be MRRESYNCDATA"
    end
    if ($FUNCTION == 0x015F)
        printf "  ccb->be MRRESYNCCTL"
    end
    if ($FUNCTION == 0x0160)
        printf "  ccb->be MRREFRESH"
    end
    if ($FUNCTION == 0x0161)
        printf "  ccb->be MRSETVPRI"
    end
    if ($FUNCTION == 0x0162)
        printf "  ccb->be MRVPRI_ENABLE"
    end
    if ($FUNCTION == 0x0163)
        printf "  ccb->be MRPDISKSPINDOWN"
    end
    if ($FUNCTION == 0x0164)
        printf "  ccb->be MRPDISKFAILBACK"
    end
    if ($FUNCTION == 0x0165)
        printf "  ccb->be MRPDISKAUTOFAILBACKENABLEDISABLE"
    end
    if ($FUNCTION == 0x0166)
        printf "  ccb->be MRCFGOPTION"
    end
    if ($FUNCTION == 0x0167)
        printf "  ccb->be MRSETTGINFO"
    end
    if ($FUNCTION == 0x0168)
        printf "  ccb->be MRGETTGINFO"
    end
    if ($FUNCTION == 0x0169)
        printf "  ccb->be MRUPDSID"
    end
    if ($FUNCTION == 0x016A)
        printf "  ccb->be MRSETCHAP"
    end
    if ($FUNCTION == 0x016B)
        printf "  ccb->be MRGETCHAP"
    end
    if ($FUNCTION == 0x016C)
        printf "  ccb->be MRGETGLINFO"
    end
    if ($FUNCTION == 0x016D)
        printf "  ccb->be MRSETGLINFO"
    end
    if ($FUNCTION == 0x016E)
        printf "  ccb->be MRCLEARGLINFO"
    end
    if ($FUNCTION == 0x016F)
        printf "  ccb->be MRGETISNSINFO"
    end
    if ($FUNCTION == 0x0170)
        printf "  ccb->be MRSETISNSINFO"
    end
    if ($FUNCTION == 0x0171)
        printf "  ccb->be MRSETPR"
    end
    if ($FUNCTION == 0x0172)
        printf "  ccb->be MRVIRTREDUNDANCY"
    end
    if ($FUNCTION == 0x0173)
        printf "  ccb->be MRGETASYNC"
    end
    if ($FUNCTION == 0x0174)
        printf "  ccb->be MRGETISEIP"
    end
    if ($FUNCTION == 0x0175)
        printf "  ccb->be MRALLDEVMISS"
    end
    if ($FUNCTION == 0x0176)
        printf "  ccb->be Get Extended vdisk info"
    end
    if ($FUNCTION == 0x0177)
        printf "  ccb->be Emulate BE Qlogic timeout on pdisk"
    end
    if ($FUNCTION == 0x0178)
        printf "  ccb->be MRGETDLINK_GT2TB"
    end
    if ($FUNCTION == 0x01FD)
        printf "  ccb->be MRNOPFSYS"
    end
    if ($FUNCTION == 0x01FE)
        printf "  ccb->be MRFSYSOP"
    end
    if ($FUNCTION == 0x01FF)
        printf "  ccb->be MRBEHBEAT"
    end
    if ($FUNCTION == 0x0200)
        printf "  be->fe MRREPORTSCONFIG"
    end
    if ($FUNCTION == 0x0201)
        printf "  be->fe MRSCONFIGCOMPLETE"
    end
    if ($FUNCTION == 0x0202)
        printf "  be->fe MRREPORTCCONFIG"
    end
    if ($FUNCTION == 0x0203)
        printf "  be->fe MRCCONFIGCOMPLETE"
    end
    if ($FUNCTION == 0x0204)
        printf "  be->fe MRSPARE204"
    end
    if ($FUNCTION == 0x0205)
        printf "  be->fe MRSTOPCACHE"
    end
    if ($FUNCTION == 0x0206)
        printf "  be->fe MRCONTINUECACHE"
    end
    if ($FUNCTION == 0x0207)
        printf "  be->fe MRSETSYSINFO"
    end
    if ($FUNCTION == 0x0208)
        printf "  be->fe MRVCHANGE"
    end
    if ($FUNCTION == 0x0209)
        printf "  be->fe MRSCHANGE"
    end
    if ($FUNCTION == 0x020A)
        printf "  be->fe MRREPORTTARG"
    end
    if ($FUNCTION == 0x020B)
        printf "  be->fe MRRESETCONFIG"
    end
    if ($FUNCTION == 0x020C)
        printf "  be->fe MRSETCNTLSNFE"
    end
    if ($FUNCTION == 0x020D)
        printf "  be->fe MRMMINFO"
    end
    if ($FUNCTION == 0x020F)
        printf "  be->fe MRUPDTGINFO"
    end
    if ($FUNCTION == 0x0210)
        printf "  be->fe MRGETPORTTYPE"
    end
    if ($FUNCTION == 0x0211)
        printf "  be->fe MRSETCHAPFE"
    end
    if ($FUNCTION == 0x0212)
        printf "  be->fe MRSETISNSINFOFE"
    end
    if ($FUNCTION == 0x0213)
        printf "  be->fe MRSETPRES"
    end
    if ($FUNCTION == 0x0214)
        printf "  be->fe MRSETFT"
    end
    if ($FUNCTION == 0x0300)
        printf "  MRLOGFE"
    end
    if ($FUNCTION == 0x0301)
        printf "  MRLOGBE"
    end
    if ($FUNCTION == 0x0400)
        printf "  fe->be MRFEGETVINFO"
    end
    if ($FUNCTION == 0x0401)
        printf "  fe->be MRFESETSEQ"
    end
    if ($FUNCTION == 0x0402)
        printf "  fe->be MRSETMPCONFIGBE"
    end
    if ($FUNCTION == 0x0403)
        printf "  fe->be MRRETRIEVEPR"
    end
    if ($FUNCTION == 0x0500)
        printf "  ccb->fe MRFELOOP"
    end
    if ($FUNCTION == 0x0501)
        printf "  ccb->fe MRGETSINFO"
    end
    if ($FUNCTION == 0x0502)
        printf "  ccb->fe MRGETCINFO"
    end
    if ($FUNCTION == 0x0503)
        printf "  ccb->fe MRFELINK"
    end
    if ($FUNCTION == 0x0504)
        printf "  ccb->fe MRFEII"
    end
    if ($FUNCTION == 0x0505)
        printf "  ccb->fe MRGETCDINFO"
    end
    if ($FUNCTION == 0x0506)
        printf "  ccb->fe MRGETSSTATS"
    end
    if ($FUNCTION == 0x0507)
        printf "  ccb->fe MRSETBATHEALTH"
    end
    if ($FUNCTION == 0x0508)
        printf "  ccb->fe MRRESUMECACHE"
    end
    if ($FUNCTION == 0x0509)
        printf "  ccb->fe MRFEBOOT"
    end
    if ($FUNCTION == 0x050A)
        printf "  ccb->fe MRFEDIAG"
    end
    if ($FUNCTION == 0x050B)
        printf "  ccb->fe MRFEPROC"
    end
    if ($FUNCTION == 0x050C)
        printf "  ccb->fe MRFECODEBURN"
    end
    if ($FUNCTION == 0x050D)
        printf "  ccb->fe MRFRWMEM"
    end
    if ($FUNCTION == 0x050E)
        printf "  ccb->fe MRRESETFEPORT"
    end
    if ($FUNCTION == 0x050F)
        printf "  ccb->fe MRSERVERLOOKUP"
    end
    if ($FUNCTION == 0x0510)
        printf "  ccb->fe MRFEGENERIC"
    end
    if ($FUNCTION == 0x0511)
        printf "  ccb->fe MRSETMPCONFIGFE"
    end
    if ($FUNCTION == 0x0512)
        printf "  ccb->fe MRFEFIBREHLIST"
    end
    if ($FUNCTION == 0x0513)
        printf "  ccb->fe MRFECONTWOMP"
    end
    if ($FUNCTION == 0x0514)
        printf "  ccb->fe MRFEFLUSHWOMP"
    end
    if ($FUNCTION == 0x0515)
        printf "  ccb->fe MRINVFEWC"
    end
    if ($FUNCTION == 0x0516)
        printf "  ccb->fe MRFLUSHBEWC"
    end
    if ($FUNCTION == 0x0517)
        printf "  ccb->fe MRINVBEWC"
    end
    if ($FUNCTION == 0x0518)
        printf "  ccb->fe MRFEMODEPAGE"
    end
    if ($FUNCTION == 0x0519)
        printf "  ccb->fe MRFEGETDVLIST"
    end
    if ($FUNCTION == 0x051A)
        printf "  ccb->fe MRFEGETPORTLIST"
    end
    if ($FUNCTION == 0x051B)
        printf "  ccb->fe MRGETTRLIST"
    end
    if ($FUNCTION == 0x051C)
        printf "  ccb->fe MRSTOPIO"
    end
    if ($FUNCTION == 0x051D)
        printf "  ccb->fe MRSTARTIO"
    end
    if ($FUNCTION == 0x051E)
        printf "  ccb->fe MRFEPORTNOTIFY"
    end
    if ($FUNCTION == 0x051F)
        printf "  ccb->fe MRFORCEFEETRAP"
    end
    if ($FUNCTION == 0x0520)
        printf "  ccb->fe MRFELOOPPRIMITIVE"
    end
    if ($FUNCTION == 0x0521)
        printf "  ccb->fe MRGETTARG"
    end
    if ($FUNCTION == 0x0522)
        printf "  ccb->fe MRFAILPORT"
    end
    if ($FUNCTION == 0x0523)
        printf "  ccb->fe MRNOPFE"
    end
    if ($FUNCTION == 0x0524)
        printf "  ccb->fe MRQFECC"
    end
    if ($FUNCTION == 0x0525)
        printf "  ccb->fe MRQSC"
    end
    if ($FUNCTION == 0x0526)
        printf "  ccb->fe MRQMPC"
    end
    if ($FUNCTION == 0x0527)
        printf "  ccb->fe MRGETHABSTATS"
    end
    if ($FUNCTION == 0x0528)
        printf "  ccb->fe MRGETBATTSTS"
    end
    if ($FUNCTION == 0x0529)
        printf "  ccb->fe MRGETMPCONFIGFE"
    end
    if ($FUNCTION == 0x052A)
        printf "  ccb->fe MRFEPORTGO"
    end
    if ($FUNCTION == 0x052B)
        printf "  ccb->fe MRSETTDISCACHE"
    end
    if ($FUNCTION == 0x052C)
        printf "  ccb->fe MRCLRTDISCACHE"
    end
    if ($FUNCTION == 0x052D)
        printf "  ccb->fe MRQTDISABLEDONE"
    end
    if ($FUNCTION == 0x052E)
        printf "  ccb->fe MRMMTEST"
    end
    if ($FUNCTION == 0x052F)
        printf "  ccb->fe MRFEFLERRORWOMP"
    end
    if ($FUNCTION == 0x0530)
        printf "  ccb->fe MRGETSESSIONS"
    end
    if ($FUNCTION == 0x0531)
        printf "  ccb->fe MRGETSESSIONSPERSERVER"
    end
    if ($FUNCTION == 0x0532)
        printf "  ccb->fe MRGETIDDINFO"
    end
    if ($FUNCTION == 0x0533)
        printf "  ccb->fe MRDLMPATHSTATS"
    end
    if ($FUNCTION == 0x0534)
        printf "  ccb->fe MRDLMPATHSELECTIONALGO"
    end
    if ($FUNCTION == 0x0535)
        printf "  ccb->fe MRPRGET"
    end
    if ($FUNCTION == 0x0536)
        printf "  ccb->fe MRPRCLR"
    end
    if ($FUNCTION == 0x0537)
        printf "  ccb->fe MRPRCONFIGCOMPLETE"
    end
    if ($FUNCTION == 0x0538)
        printf "  ccb->fe MRUPDPRR"
    end
    if ($FUNCTION == 0x05FF)
        printf "  ccb->fe MRFEHBEAT"
    end
    printf "\n"
end
#...
document i_print_mrp_function_name
Internal macro to print out MRP function names.
end
#-----------------------------------------------------------------------------
# The "i_print_d_exec_queue" is an internal macro to print out the d_exec_queue ILT's (MRPs).
define i_print_d_exec_queue
    set $Q = $arg0
    printf " 0x%08x 0x%08x 0x%08x(%d) 0x%08x\n", $Q->head, $Q->tail, $Q->qcnt, $Q->qcnt, $Q->pcb
    set $ILT = $Q->head
    set $C_ILT = 0
    while ($ILT !=0)
        set $C_ILT = $C_ILT + 1
        printf "%2d ILT fthd=0x%08x bthd=0x%08x misc=0x%08x linux_val=0x%x\n", $C_ILT, $ILT->fthd, $ILT->bthd, $ILT->misc, $ILT->linux_val
        set $MRP = (MR_PKT*)(*(unsigned int *)(((char *)$ILT)-52+16))
        printf "  MRP->function=0x%x (%d), pReqPCI=0x%08x", $MRP->function, $MRP->function, $MRP->pReqPCI
        i_print_mrp_function_name $MRP->function
        printf "       rspLen=%d, pRsp=0x%08x, pReq=0x%08x, reqLen=%d\n", $MRP->rspLen, $MRP->pRsp, $MRP->pReq, $MRP->reqLen
        set $ILT = $ILT->fthd
    end
    if ($Q->qcnt != $C_ILT)
        printf "ERROR - qcnt %d does not match ILT list count of %d\n", $Q->qcnt, $C_ILT
    end
end
#...
document i_print_d_exec_queue
Internal macro to print out d_exec_queue queue containing ILT's (MRPs).
end
#-----------------------------------------------------------------------------
# The "printCMqueues" is a macro to print out BE copy manager queues.
define printCMqueues
    printf "CM_rem_uperr=%d\n", *(UINT16*)&CM_rem_uperr
    set $Kcm = *(UINT8*)&cm_cm_act_cnt
    printf "CM_cm_act_que =0x%08x, cm_cm_act_cnt =%d\n", CM_cm_act_que, *(UINT8*)&cm_cm_act_cnt
    set $Icm = CM_cm_act_que
    set $Jcm = 0
    while ($Icm !=0)
        set $Jcm = $Jcm+1
        printf "%2d CM 0x%08x    head       tail        qcnt      pcb\n", $Jcm, $Icm
        printf "   cmpltq:      "
            i_printnormalqueue (QU*)&(((CM*)$Icm)->cmpltq)
        printf "   ctlrqstq:    "
            i_printnormalqueue (QU*)&(((CM*)$Icm)->ctlrqstq)
        printf "   uderrq:      "
            i_printnormalqueue (QU*)&(((CM*)$Icm)->uderrq)
        set $Icm = *((UINT32*)$Icm)
    end
    if ($Jcm != $Kcm)
        printf "ERROR - %d does not match cm_cm_act_cnt of %d\n", $Jcm, $Kcm
    end
    set $Kcor = *(UINT16*)&cm_cor_act_cnt
    printf "CM_cor_act_que=0x%08x, cm_cor_act_cnt=%d\n", CM_cor_act_que, $Kcor
    set $Icor = (COR *)CM_cor_act_que
    set $Jcor = 0
    while ($Icor !=0)
       set $Jcor = $Jcor+1
       printf "%2d COR = 0x%08x\n", $Jcor, $Icor->link
       set $Ccm = $Icor->cm
       if ($Ccm != 0)
          printf "   CM 0x%08x    head       tail        qcnt      pcb\n", $Ccm
          printf "   cmpltq:      "
              i_printnormalqueue (QU*)&(((CM*)$Ccm)->cmpltq)
          printf "   ctlrqstq:    "
              i_printnormalqueue (QU*)&(((CM*)$Ccm)->ctlrqstq)
          printf "   uderrq:      "
              i_printnormalqueue (QU*)&(((CM*)$Ccm)->uderrq)
       end
       set $Icor =$Icor->link
    end
    if ($Jcor != $Kcor)
        printf "ERROR - %d does not match cm_cor_act_cnt of %d\n", $Jcor, $Kcor
    end
end
#...
document printCMqueues
Print out BE copy manager queues.
end
#-----------------------------------------------------------------------------
define pdiskoutstanding
  set $COUNT = 0
  while ($COUNT < $MAXDRIVES)
    set $PDD = (PDD**)&P_pddindx
    set $PDD = $PDD[$COUNT]
    if ($PDD != 0)
      if ($PDD->pDev == 0)
        printf "*PDD %3d  Waiting Ops(dev->qCnt)= N/A  Count to Drives(pdd->qd)=%4d dev->orc= N/A\n", $COUNT, $PDD->qd
      else
        set $DEVPDD = $PDD->pDev->pdd
        if ($PDD->pDev->qCnt != 0 || $PDD->qd != 0 || $PDD->pDev->orc != 0)
          printf " PDD %3d  Waiting Ops(dev->qCnt)=%4d  Count to Drives(pdd->qd)=%4d dev->orc=%4d\n", $COUNT, $PDD->pDev->qCnt, $PDD->qd, $PDD->pDev->orc
        end
        if ($PDD != $DEVPDD)
          printf "PDD address (%p) != $PDD->pDev->pdd (%p)!\n", $PDD, $DEVPDD
        end
      end
    end
    set $COUNT = $COUNT + 1
  end
end
#-----------------------------------------------------------------------------
# This macro prints out all the Back End queues -- well, what it can do!
define allbequeues
# First print out the link queues (between FE/BE/CCB).
    linkqueues
    printf "----------------    head       tail        qcnt         pcb\n"
    printf "gAsyncNVque:    "
        i_printnormalqueue (QU*)&gAsyncNVque
    printf "gGrVdErrorQue:  "
        i_printnormalqueue (QU*)&gGrVdErrorQue
    printf "DLM_vrp_qu:     "
        i_printnormalqueue (QU*)&DLM_vrp_qu
    printf "P_exec_qu:      "
        i_printnormalqueue (QU*)&P_exec_qu
    printf "R_exec_qu:      "
        i_printnormalqueue (QU*)&R_exec_qu
    printf "R_r5exec_qu:    "
        i_printr5execqueue (QU*)&R_r5exec_qu
    printf "RB_rerror_qu:   "
        i_printnormalqueue (QU*)&RB_rerror_qu
    printf "V_exec_hqu:     "
        i_printnormalqueue (QU*)&V_exec_hqu
    printf "V_exec_mqu:     "
        i_printnormalqueue (QU*)&V_exec_mqu
    printf "V_exec_qu:      "
        i_printnormalqueue (QU*)&V_exec_qu
    printf "V_exec_xqu:     "
        i_printnormalqueue (QU*)&V_exec_xqu
    printf "ccsm_sp_qu:     "
        i_printnormalqueue (QU*)&ccsm_sp_qu
    printf "ccsmswap_sp_qu: "
        i_printnormalqueue (QU*)&ccsmswap_sp_qu
    printf "cm_RCC_qu:      "
        i_printnormalqueue (QU*)&cm_RCC_qu
# The following 3 are part of the qpt stuff -- the queues below are dumped.
    printf "cm_exec_qu_high:"
        i_printnormalqueue (QU*)&cm_exec_qu_high
    printf "cm_exec_qu_norm:"
        i_printnormalqueue (QU*)&cm_exec_qu_norm
    printf "cm_exec_qu_low: "
        i_printnormalqueue (QU*)&cm_exec_qu_low
    printf "cm_rc_qu:       "
        i_printnormalqueue (QU*)&cm_rc_qu
    printf "cm_sp_qu:       "
        i_printnormalqueue (QU*)&cm_sp_qu
    printf "cm_wc_qu:       "
        i_printnormalqueue (QU*)&cm_wc_qu
    printf "cmw_qu:         "
        i_printnormalqueue (QU*)&cmw_qu
    printf "d_exec_qu:      "
        i_print_d_exec_queue (QU*)&d_exec_qu
    printf "d_rip_exec_qu:  "
        i_printnormalqueue (QU*)&d_rip_exec_qu
    printf "dlm_lrpcr_qu:   "
        i_printnormalqueue (QU*)&dlm_lrpcr_qu
    printf "dlm_lrpio_qu:   "
        i_printnormalqueue (QU*)&dlm_lrpio_qu
    printf "f_exec_qu:      "
        i_printnormalqueue (QU*)&f_exec_qu
    printf "m_hbeat_qu:     "
        i_printnormalqueue (QU*)&m_hbeat_qu
    printf "------------\n"
    printf "dlm_rtydg_qu:   "
        i_printdlmrtrydgqu (QU*)&dlm_rtydg_qu
    printf "------------\n"
    printCMqueues
    printf "------------\n"
    printf "P_comp_qu:      "
        i_printnormalqueue (QU*)&P_comp_qu
    printf "------------\n"
    DevList
    printf "------------\n"
    DumpDevQueues
    printf "------------\n"
    pdiskoutstanding
    printf "------------\n"
    print_iram_ilt_head_tail
end
#...
document allbequeues
Print out all Back End queues (that it knows about - please fix problems).
See "flight" for flightrecorder log.
See "mrp" for mrp log.
end
#-----------------------------------------------------------------------------
define print_iram_ilt_head_tail
  set $I = 0
  set $ICNT = 0
  while ($I < 4)
    set $H = ((UINT32*)&ilthead)[$I*2]
    if ($H == 0 || $H == (UINT32)(((UINT32*)&ilttail + 2*$I)))
      printf "ilthead[%d] is empty\n", $I
    else
      set $B = ((UINT32*)&ilthead)[$I*2+1]
    end
    set $J = 1
    while ($H != 0 && $H != (UINT32)(((UINT32*)&ilttail + 2*$I)))
      set $ICNT = $ICNT + 1
      printf "ilthead[%d] #%2d => fthd=%08.8x, bthd=%08.8x    Raid2PhysicalILT# %2d\n", $I, $J, (UINT32)$H, (UINT32)$B, $ICNT
      set $W5 = (ILT*)((((ILT*)$H)-1)->ilt_normal.w5)
      set $W5CNT = 1
      if ($W5 != 0)
        printf "   JOINED: #%2d => pjth=%08.8x\n", $W5CNT, (UINT32)$W5
        set $W5 = $W5->fthd
        set $W5CNT = 2
        while ($W5 != 0)
          set $ICNT = $ICNT + 1
          printf "   JOINED: #%2d => fthd=%08.8x                   Raid2PhysicalILT# %2d\n", $W5CNT, (UINT32)$W5, $ICNT
          set $W5CNT = $W5CNT + 1
          set $W5 = $W5->fthd
        end
      end
      set $H = ((ILT*)$H)->fthd
      set $B = ((ILT*)$H)->bthd
      set $J = $J + 1
    end
    set $H = ((UINT32*)&ilttail)[$I*2]
    set $B = ((UINT32*)&ilttail)[$I*2+1]
    printf "ilttail[%d]     => fthd=%08.8x, bthd=%08.8x\n", $I, (UINT32)$H, (UINT32)$B
    set $I = $I + 1
  end
end
#...
document print_iram_ilt_head_tail
Print out the ilthead and ilttail queue of ILTs that are on the physical devices.
end
#-----------------------------------------------------------------------------
define print_dev_queues
    set $CHAN = (CHN**)P_chn_ind
    set $I = 0
    set $TOTAL_IOS = 0
    printf "   # chn  pid DevStat flags  orc unav Perr   wait TimetoFail  dvPort CDB0 rStat qStat retry active recflg\n"
    while ($I < $MAXCHN)
         if ($CHAN[$I] != 0)
            set $DEV = ($CHAN[$I].devList)
            set $DEVCNT = $CHAN[$I].devCnt
            set $J = 0
            while ($J < $DEVCNT)
                set $K = 0
                set $N = 0
                set $ILT =(ILT*) (($DEV)->iltQFHead)
                while ($K < $DEV->qCnt)
                    set $PRP = $ILT -1
                    set $PRP = (PRP*)(*((UINT32*)$PRP + 0x4))
                    set $N = $N + 1
                    if ($PRP != 0)
                        printf "  %2d %3d %4d    0x%02x  0x%02x %4d 0x%02x 0x%02x  0x%02x          %2d %d %d %d %d 0x%02x  0x%02x  0x%02x  0x%02x   0x%02x 0x%04x\n", $N, $DEV->port, $DEV->pdd->pid, $DEV->pdd->devStat, $DEV->flags, $DEV->orc, $DEV->unavail, $DEV->physErr, $DEV->wait, $DEV->TimetoFail, $DEV->dvPort[0], $DEV->dvPort[1], $DEV->dvPort[2], $DEV->dvPort[3], $PRP->cmd[0], $PRP->reqStatus, $PRP->qLogicStatus, $PRP->retry, $DEV->setupretryactive, $DEV->recoveryflags
                        printf "     ILT=0x%08x PRP=0x%08x\n", $ILT, $PRP
                        set $K = $K + 1
                        set $TOTAL_IOS = $TOTAL_IOS + 1
                    else
                        printf "  !! %3d %4d    0x%02x  0x%02x %4d 0x%02x 0x%02x  0x%02x          %2d %d %d %d %d    *** PRP ZERO ***      0x%02x 0x%04x\n", $DEV->port, $DEV->pdd->pid, $DEV->pdd->devStat, $DEV->flags, $DEV->orc, $DEV->unavail, $DEV->physErr, $DEV->wait, $DEV->TimetoFail, $DEV->dvPort[0], $DEV->dvPort[1], $DEV->dvPort[2], $DEV->dvPort[3], $DEV->setupretryactive, $DEV->recoveryflags
                        printf "     ILT=0x%08x PRP=0x%08x\n", $ILT, $PRP
                    end
                    set $ILT = $ILT->fthd
                end
                set $ILT = $DEV->failQHead
                while ($ILT!= 0)
                    set $PRP = $ILT -1
                    set $PRP = (PRP*)(*((UINT32*)$PRP + 0x4))
                    set $N = $N + 1
                    if ($PRP != 0)
                        printf "F %2d %3d %4d    0x%02x  0x%02x %4d 0x%02x 0x%02x  0x%02x          %2d %d %d %d %d 0x%02x  0x%02x  0x%02x  0x%02x   0x%02x 0x%04x F\n", $N, $DEV->port, $DEV->pdd->pid, $DEV->pdd->devStat, $DEV->flags, $DEV->orc, $DEV->unavail, $DEV->physErr, $DEV->wait, $DEV->TimetoFail, $DEV->dvPort[0], $DEV->dvPort[1], $DEV->dvPort[2], $DEV->dvPort[3], $PRP->cmd[0], $PRP->reqStatus, $PRP->qLogicStatus, $PRP->retry, $DEV->setupretryactive, $DEV->recoveryflags
                        printf "     ILT=0x%08x PRP=0x%08x\n", $ILT, $PRP
                        set $K = $K + 1
                        set $TOTAL_IOS = $TOTAL_IOS + 1
                    else
                        printf "F !! %3d %4d    0x%02x  0x%02x %4d 0x%02x 0x%02x  0x%02x          %2d %d %d %d %d    *** PRP ZERO ***      0x%02x 0x%04x F\n", $DEV->port, $DEV->pdd->pid, $DEV->pdd->devStat, $DEV->flags, $DEV->orc, $DEV->unavail, $DEV->physErr, $DEV->wait, $DEV->TimetoFail, $DEV->dvPort[0], $DEV->dvPort[1], $DEV->dvPort[2], $DEV->dvPort[3], $DEV->setupretryactive, $DEV->recoveryflags
                        printf "     ILT=0x%08x PRP=0x%08x\n", $ILT, $PRP
                    end
                    set $ILT = $ILT->fthd
                end
                set $J = $J + 1
                set $DEV = $DEV->nDev
           end
        end
        set $I = $I + 1
    end
    printf "    IO COUNT 0x%02x (%d)\n", $TOTAL_IOS, $TOTAL_IOS
end
#...
document print_dev_queues
Print out the dev queues to figure out what is where.
end
#-----------------------------------------------------------------------------
# Dump all commands attached to the dev stuctures.
define DumpDevQueues
    set $CHAN = (CHN**)P_chn_ind
    set $I = 0
    set $TOTAL_IOS = 0
    printf "  chan    DEV         DevStat  dv_wait  ILT            PRP          CDB0 rStatus qStatus retry setup4retry\n"
    printf "  --------------------------------------------------------------------------------------------------------\n"
    while ($I < $MAXCHN)
         if ($CHAN[$I] != 0)
            set $DEV = ($CHAN[$I].devList)
            set $DEVCNT = $CHAN[$I].devCnt
            set $J = 0
            while ($J < $DEVCNT)
                set $K = 0
                set $ILT =(ILT*) (($DEV)->iltQFHead)
                while ($K < $DEV->qCnt)
                    set $PRP = $ILT -1
                    set $PRP = (PRP*)(*((UINT32*)$PRP + 0x4))
                    printf "    %d     0x%08x    0x%02x    0x%02x    0x%08x     0x%08x ", $I, $DEV, $DEV->pdd->devStat, $DEV->wait, $ILT, $PRP
                    if ($PRP != 0)
                        printf "  0x%02x    0x%02x 0x%02x     0x%02x        0x%02x", $PRP->cmd[0], $PRP->reqStatus, $PRP->qLogicStatus, $PRP->retry, $DEV->setupretryactive
                        set $K = $K + 1
                    end
                    printf "\n"
                    set $TOTAL_IOS = $TOTAL_IOS + 1
                    set $ILT = $ILT->fthd
                end
                set $ILT = $DEV->failQHead
                while ($ILT!= 0)
                    set $PRP = $ILT -1
                    set $PRP = (PRP*)(*((UINT32*)$PRP + 0x4))
                    printf "FQ  %d     0x%08x    0x%02x    0x%02x    0x%08x     0x%08x ", $I, $DEV, $DEV->pdd->devStat, $DEV->wait , $ILT, $PRP
                    if ($PRP != 0)
                        printf "  0x%02x    0x%02x 0x%02x     0x%02x       FAILQ", $PRP->cmd[0], $PRP->reqStatus, $PRP->qLogicStatus, $PRP->retry
                        set $K = $K + 1
                    end
                    printf "\n"
                    set $ILT = $ILT->fthd
                    set $TOTAL_IOS = $TOTAL_IOS + 1
                end
                set $J = $J + 1
                set $DEV = $DEV->nDev
           end
        end
        set $I = $I + 1
    end
    printf "    IO COUNT 0x%02x (%d)\n", $TOTAL_IOS, $TOTAL_IOS
end
#...
document DumpDevQueues
Dump all commands attached to the dev stuctures.
end
#-----------------------------------------------------------------------------
define i_countilts
  set $ILT = (ILT *)$arg0
  set $ILTCNT = 0
  if ($ILT >= $EXECUTABLE_START)
    set $ILT = $ILT->fthd
    while ($ILT >= $EXECUTABLE_START)
      set $ILTCNT = $ILTCNT + 1
      set $ILT = $ILT->fthd
    end
  end
end
#...
document i_countilts
Internal macro to count number of ilts (first argument is a pointer to a pointer to and ilt).
end
#-----------------------------------------------------------------------------
define DevList
    set $CHAN = (CHN**)P_chn_ind
    set $I = 0
    set $devount = 0
    set $ilthead = 0
    set $iltfail = 0
    printf "  chan   DEV      DevStat    PDD       iltQFHead  #   failQHead  #   dv_wait  PID flags\n"
    printf "  ---- ---------- -------  ----------  ---------- --  ---------- --  -------  --- -----\n"
    while ($I < $MAXCHN)
        if ($CHAN[$I] != 0)
            set $DEV = $CHAN[$I].devList
            set $DEVCNT = $CHAN[$I].devCnt
            set $J = 0
            while ($J < $DEVCNT)
                i_countilts $DEV->iltQFHead
                set $ILTCOUNT1 = $ILTCNT
                set $ilthead = $ilthead + $ILTCOUNT1
                i_countilts $DEV->failQHead
                set $ILTCOUNT2 = $ILTCNT
                set $iltfail = $iltfail + $ILTCOUNT2
                printf "    %d  0x%08x   0x%02x   0x%08x", $I, $DEV, $DEV->pdd->devStat, $DEV->pdd
                printf "  0x%08x %2d  0x%08x %2d  0x%04x   %3d %02X\n", $DEV->iltQFHead, $ILTCOUNT1, $DEV->failQHead, $ILTCOUNT2, $DEV->wait, $DEV->pdd->pid, $DEV->flags
                set $J = $J + 1
                set $DEV = $DEV->nDev
                set $devount = $devount + 1
            end
        end
        set $I = $I + 1
    end
    printf " TOTAL: Entries %3d                    iltQFHead %3d  failQHead %3d\n", $devount, $ilthead, $iltfail
end
#...
document DevList
Dump all devices attached to the channel structures.
end
#--------------------------------------------------------------------------
# This is wierd layout of a queue.
define i_printdlmrtrydgqu
    set $Q = $arg0
    printf " 0x%08x 0x%08x\n", $Q->head, $Q->tail
    set $C_ILT = 0
    while ($Q->head !=0)
        set $ILT = (ILT*)((char *)($Q->head) + 0x34)
        i_linknextilt $C_ILT 1
        set $C_ILT = $C_ILT + 1
        printf " Next Queue head 0x%08x\n", $Q->head
        set $Q = (QU*)($Q->head)
    end
end
#...
document i_printdlmrtrydgqu
Internal macro for printing the i_printdlmrtrydgqu queue.
end
#-----------------------------------------------------------------------------
# The "i_printht" internal macro to printout values for a head/tail queue without pcb/count.
define i_printht
    set $Q = $arg0
    printf " 0x%08x 0x%08x\n", $Q->head, $Q->tail
    set $ILT = $Q->head
    set $C_ILT = 0
    if ($ILT != 0)
        while ($ILT !=0)
            set $C_ILT = $C_ILT + 1
            i_linknextilt $C_ILT 1
        end
    end
end
#...
document i_printht
Internal macro to printout values for a head/tail queue without pcb/count.
end
#-----------------------------------------------------------------------------
# The "i_printvcd" internal macro to printout values for a head/tail queue without pcb/count.
define i_printvcd
    set $Q = (QU*)$arg0
    printf " 0x%08x 0x%08x\n", $Q->head, $Q->tail
    set $VCD = (VCD *)$Q->head
    set $C_VCD = 0
    set $C_ILT = 0
    while ($VCD !=0)
        set $C_VCD = $C_VCD + 1
        printf "vid=%d tDisCnt=%d stat=0x%x flags=0x%x writeCount=%d vtv=%d pTHead=%x, pTTail=%x\n", $VCD->vid, $VCD->tDisCnt, $VCD->stat, $VCD->flags, $VCD->writeCount, $VCD->vtv, $VCD->pTHead, $VCD->pTTail
        set $ILT = $VCD->pTHead
        while ($ILT != 0)
            printf "%p ", $ILT
            i_linknextilt $C_ILT 1
            set $C_ILT = $C_ILT + 1
        end
        set $VCD = $VCD->pFwdWait
    end
end
#...
document i_printvcd
Internal macro to printout values for the vcd queue.
end
#-----------------------------------------------------------------------------
define i_vrpcheck
  set $i_VRP = (VRP*)$arg0
  set $ERROR = 0
  if ($i_VRP->function > 26)
    printf "ERROR VRP function (%d) > 26\n", $i_VRP->function
    set $ERROR = 1
  else
    # Check for all we want to specially handle here. vrinput, vrverifyc, vrsync, & 3 == 2.
    if ($i_VRP->function != 1 && $i_VRP->function != 20 && $i_VRP->function != 26 && ($i_VRP->function & 3) != 2)
      printf "Function (%d) not to be validated.\n", $i_VRP->function
      # Assume everything else is ok.
    else
      # If read command, or write command (but not verify checkword)
      if ($i_VRP->function == 1 && ($i_VRP->function != 20 && ($i_VRP->function & 3) == 2) && $i_VRP->length > 4096)
        printf "Function (%d) has sector length greater than 4096 (%d).\n", $i_VRP->function, $i_VRP->length
        set $ERROR = 1
      end
      set $VDMT = ((VDMT**)&MAG_VDMT_dir)[$i_VRP->vid]
      if ((UINT32)$VDMT == 0)
        printf "Function (%d) has bad vid, MAG_VDMT_dir[%d]=%p\n", $i_VRP->function, $i_VRP->vid, $VDMT
        set $ERROR = 1
      else
        if ($i_VRP->startDiskAddr > $VDMT->devCap)
          printf "Function (%d) has bad startDiskAddr (%lld) > (%lld)\n", $i_VRP->function, $i_VRP->startDiskAddr, $VDMT->devCap
          set $ERROR = 1
        else
          if ($i_VRP->startDiskAddr + $i_VRP->length >= $VDMT->devCap)
            printf "Function (%d) has bad startDiskAddr (%lld) +length > (%lld)\n", $i_VRP->function, $i_VRP->startDiskAddr, $i_VRP->length, $VDMT->devCap
            set $ERROR = 1
          end
        end
      end
    end
  end
end
#...
document i_vrpcheck
Internal macro to check VRP. GDB implementation of c$vrpcheck.
end
#-----------------------------------------------------------------------------
# The "i_print_C_exec_qht_entry" internal macro to printout values for the C_exec_qht queue.
define i_print_C_exec_qht_entry
  set $cILT = $arg0
  set $number = $arg1
  i_find_ilt_base $cILT
  if ($cnt == $ILTNEST)
    printf "%3d i_print_C_exec_qht_entry - Could not calculate base of ILT %p\n", $number, $cILT
  else
    # $ILT = starting address.
    if ($ILT == $cILT)
      printf "%3d ILT (%p) AT WRONG PLACE, can't go back one level to the vrp.\n", $number, $ILT
    else
      printf "%3d queue entry ILT level %d at 0x%08x of 0x%08x\n", $number, ((UINT32)$cILT-(UINT32)$ILT)/sizeof(ILT), $cILT, $ILT
      set $PILT = (ILT*)((UINT32)$cILT - sizeof(ILT))
      set $VRP = (VRP*)(((UINT32*)$PILT)[8])
      set $VROPTIONS = $VRP->options
      i_vrpcheck $VRP
      i_printilt_vrp $ILT $cILT $cnt $PILT $VRP
    end
  end
end
#...
document i_print_C_exec_qht_entry
Internal macro to printout values for the i_print_C_exec_qht queue.
end
#-----------------------------------------------------------------------------
# The "i_print_C_exec_qht" internal macro to printout values for the C_exec_qht queue.
define i_print_C_exec_qht
    set $Q = $arg0
    set $Qcd = *($arg1)
    printf " 0x%08x 0x%08x            0x%08x (%d)\n", $Q->head, $Q->tail, $Qcd, $Qcd
    set $QILT = $Q->head
    set $C_ILT = 0
    if ($QILT != 0)
        while ($QILT !=0)
            set $C_ILT = $C_ILT + 1
            i_print_C_exec_qht_entry $QILT $C_ILT
            set $QILT = $QILT->fthd
        end
    end
    if ($Qcd != $C_ILT)
      printf "ERROR - cqd %d does not match ILT list count of %d\n", $Qcd, $C_ILT
    end
end
#...
document i_print_C_exec_qht
Internal macro to printout values for the i_print_C_exec_qht queue.
end
#-----------------------------------------------------------------------------
# The "i_printhtqueue" internal macro to printout values for a head/tail queue with count.
define i_printhtqueue
    set $Q = $arg0
    set $Qcd = *($arg1)
    printf " 0x%08x 0x%08x            0x%08x (%d)\n", $Q->head, $Q->tail, $Qcd, $Qcd
    set $ILT = $Q->head
    set $C_ILT = 0
    if ($ILT != 0)
        while ($ILT !=0)
            set $C_ILT = $C_ILT + 1
            i_linknextilt $C_ILT 0
        end
    end
    if ($Qcd != $C_ILT)
        printf "ERROR - cqd %d does not match ILT list count of %d\n", $Qcd, $C_ILT
    end
end
#...
document i_printhtqueue
Internal macro to printout values for a head/tail queue with count.
end
#-----------------------------------------------------------------------------
# The "i_printhtqDRP" internal macro prints out values for a head/tail queue with count&pcb&DRPs.
define i_printhtqDRP
    set $Q = $arg0
    set $Qcd = *$arg1
    set $PCB = *$arg2
    set $DRP = *$arg3
    printf " 0x%08x 0x%08x 0x%08x(%d) 0x%08x 0x%x (%d)\n", $Q->head, $Q->tail, $Qcd, $Qcd, $PCB, $DRP, $DRP
    set $ILT = $Q->head
    set $C_ILT = 0
    if ($ILT != 0)
        while ($ILT !=0)
            set $C_ILT = $C_ILT + 1
            i_linknextilt $C_ILT 0
        end
    end
    if ($Qcd != $C_ILT)
        printf "ERROR - cqd %d does not match ILT list count of %d\n", $Qcd, $C_ILT
    end
end
#...
document i_printhtqDRP
Internal macro prints out values for a head/tail queue with count&pcb&DRPs.
end
#-----------------------------------------------------------------------------
define i_print_vcd_throttle_queues
  set $I = 0
  set $J = 0
  while ($I < $MAXVIRTUALS)
    set $VCD = ((VCD**)&vcdIndex)[$I]
    if ((UINT32)$VCD != 0)
      if ((UINT32)$VCD->pTHead != 0 || (UINT32)$VCD->pTTail != 0)
        printf "vcdIndex[%d] pTHead=%p pTTail=%p\n", $I, $VCD->pTHead, $VCD->pTTail
        set $J = 1
      end
    end
    set $I = $I + 1
  end
  if ($J == 0)
    printf "No vcd throttle queue ILTs\n"
  end
end
#...
document i_print_vcd_throttle_queues
Internal macro prints out values for a head/tail queue of the vcd throttle queues.
end
#-----------------------------------------------------------------------------
# This macro prints out all the Front End queues -- well, what it can do!
define allfequeues
# First print out the link queues (between FE/BE/CCB).
    linkqueues
    printf "--------------------    head       tail        qcnt         pcb\n"
    printf "CA_OpRetryQue:      "
        i_printnormalqueue (QU*)&CA_OpRetryQue
    printf "gWCMarkCacheQueue:  "
        i_printnormalqueue (QU*)&gWCMarkCacheQueue
    printf "gWCMirrorBETagQueue:"
        i_printnormalqueue (QU*)&gWCMirrorBETagQueue
    printf "wrpQueue:           "
        i_printnormalqueue (QU*)&wrpQueue
    printf "tDisQueue:          "
        i_printnormalqueue (QU*)&tDisQueue
    printf "DLM_vrp_qu:         "
        i_printnormalqueue (QU*)&DLM_vrp_qu
    printf "lld_srp_qu:         "
         i_printnormalqueue (QU*)&lld_srp_qu
    printf "c_wflushq:          "
        i_printnormalqueue (QU*)&c_wflushq
    printf "gMirrorQueue:       "
        i_printnormalqueue (QU*)&gMirrorQueue
    printf "d_exec_qu:          "
        i_print_d_exec_queue (QU*)&d_exec_qu
    printf "m_hbeat_qu:         "
        i_printnormalqueue (QU*)&m_hbeat_qu
    printf "------------\n"
    printf "dlm_rtydg_qu:       "
        i_printdlmrtrydgqu (QU*)&dlm_rtydg_qu
    printf "lld_rtyio_qu:       "
        i_printht (QU*)&lld_rtyio_qu
    printf "------------\n"
    printf "C_drpexec_qht:      "
        i_printhtqueue (QU*)&C_drpexec_qht (UINT32*)&C_drpexec_cqd (UINT32*)&C_drpexec_pcb
    printf "C_ioexec_qht:       "
        i_printhtqueue (QU*)&C_ioexec_qht (UINT32*)&C_ioexec_cqd (UINT32*)&C_ioexec_pcb
    printf "C_exec_qht:         "
        i_print_C_exec_qht (QU*)&C_exec_qht (UINT32*)&C_exec_cqd (UINT32*)&C_exec_pcb
    printf "C_vcd_wait_head:    "
        i_printvcd (QU*)&C_vcd_wait_head
    printf "C_error_qht:        "
        i_printhtqueue (QU*)&C_error_qht (UINT32*)&C_error_cqd (UINT32*)&C_error_pcb
    printf "dlm_srpexec_qht:    "
        i_printhtqueue (QU*)&dlm_srpexec_qht (UINT32*)&dlm_srpexec_cqd (UINT32*)&dlm_srpexec_pcb
    printf "dlm_drpexec_qht:    "
        i_printhtqueue (QU*)&dlm_drpexec_qht (UINT32*)&dlm_drpexec_cqd (UINT32*)&dlm_drpexec_pcb
    printf "--------------------     head       tail        qcnt         pcb     DRPs\n"
    printf "C_cwi_qht:          "
        i_printhtqDRP (QU*)&C_cwi_qht (UINT32*)&C_cwi_cqd (UINT32*)&C_cwi_pcb (UINT32*)&C_cwi_cdrp
    printf "C_swi_qht:          "
        i_printhtqDRP (QU*)&C_swi_qht (UINT32*)&C_swi_cqd (UINT32*)&C_swi_kick_pcb (UINT32*)&C_swi_cdrp
    i_print_vcd_throttle_queues
    printf "------------\n"
    print_iram_ilt_head_tail
end
#...
document allfequeues
Print out all Front End queues (that it knows about -- please fix problems).
See "flight" for flightrecorder log.
See "mrp" for mrp log.
end
#-----------------------------------------------------------------------------
define fe3000CountOpsOutstanding
  fecountopsoutstanding
end
define fe7000countopsoutstanding
  fecountopsoutstanding
end
#
define fecountopsoutstanding
    set $count_throttle = 0
    set $count_blocked = 0
    set $I = 0
# Loop through the VCD list.
    while ($I < $MAXVIRTUALS)
      set $VCD = ((VCD**)&vcdIndex)[$I]
      if ($VCD != 0)
        set $pILT = $VCD->pTHead
# + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
# Loop through the ILTs in the throttle queue and add them to the count to be returned from this function.
        while ((UINT32)$pILT != 0)
# printf "HERE0 vid=%d %p\n", $I, $pILT
# $pILT+4+4 = ilt_normal.w4
          set $pVRP1 = (VRP*)*((UINT32*)$pILT+4+4)
          if ((UINT32)$pVRP1 != 0)
            printf "Throttle pVRP1->(vid=%d,options=0x%x,lba=0x%llx,len=0x%x,func=0x%x)\n", $pVRP1->vid, $pVRP1->options, $pVRP1->startDiskAddr, $pVRP1->length, $pVRP1->function
          end
          set $pILT = $pILT->fthd
          set $count_throttle = $count_throttle + 1
        end
# + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
# Walk the RB Tree and look for operations that are blocked.
        set $pRB = ((VCD**)&vcdIndex)[$I]->pIO
        while ($pRB != 0)
# + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
# Loop on all the duplicate IOs (fthd) and increment the count.
          set $pRBFthd = $pRB
          while ((UINT32)$pRBFthd != 0)
            set $pILT = (ILT *)$pRBFthd->dPoint
            if ($pILT != 0)
              if ($pILT->ilt_normal.w1 > 0)
                set $count_blocked = $count_blocked + 1
                set $pVRP = (VRP*)($pILT->ilt_normal.w4)
                printf "vid=%d pRBFthd-loop count_blocked=%d Blocked for pVRP->(vid=%d,options=0x%x,lba=0x%llx)\n", $I, $count_blocked, $pVRP->vid, $pVRP->options, $pVRP->startDiskAddr
              end
            end
            set $pRBFthd = $pRBFthd->fthd
          end
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
          if ((UINT32)$pRB->bParent == 0 && $pRB->cLeft == (struct RB*)&nil && $pRB->cRight == (struct RB*)&nil)
            set $pRB = (struct RB*)&nil
          else
            if ($pRB->cLeft != (struct RB*)&nil)
              set $pRB = $pRB->cLeft
            else
              if ($pRB->cRight != (struct RB*)&nil)
                set $pRB = $pRB->cRight
              else
                set $pRBPrev = $pRB
                set $pRB = $pRB->bParent
                while ($pRB != (struct RB*)&nil && $pRBPrev != (struct RB*)&nil)
                  if ($pRBPrev == $pRB->cLeft && $pRB->cRight != (struct RB*)&nil)
                    set $pRBPrev = (struct RB*)&nil
                    set $pRB = $pRB->cRight
                  else
                    if ($pRBPrev == $pRB->cLeft || $pRBPrev == $pRB->cRight)
                      if ($pRB->bParent == 0)
                        set $pRBPrev = (struct RB*)&nil
                        set $pRB = (struct RB*)&nil
                      else
                        set $pRBPrev = $pRB
                        set $pRB = $pRB->bParent
                      end
                    end
                  end
                end
              end
            end
          end
        end
      end
      set $I = $I + 1
    end


    set $count = $count_throttle + $count_blocked
    if ($count > C_orc)
      printf "PROBLEM: count = %d (count_throttle = %d, count_blocked = %d) > C__orc=%d -- C_flush_orc=%d\n", $count, $count_throttle, $count_blocked, C_orc, C_flush_orc
    else
      printf "count = %d (count_throttle = %d, count_blocked = %d) > C__orc=%d -- C_flush_orc=%d\n", $count, $count_throttle, $count_blocked, C_orc, C_flush_orc
    end
end
#...
document feCountOpsOutstanding
Print out all Front End outstanding operations for first argument # of vdisks.
end
#-----------------------------------------------------------------------------
define beorcs
  printf "  cow_orc      = %d\n", cow_orc
  printf "  R_errlock    = %d\n", R_errlock
  printf "  RB_rerror_qu#= %d\n", ((QU*)&RB_rerror_qu)->qcnt
  printf "  P_orc        = %d, qcnt = %d\n", P_orc, ((QU*)&P_exec_qu)->qcnt
  printf "  d_init_orc   = %d\n", d_init_orc
  printf "  d_init_lock  = %d [raid initializations stopped/blocked if 1]\n", d_init_lock
  printf "  O_stopcnt    = %d (nested online stop commands)\n", *(UINT8*)&O_stopcnt
  printf "  rb_stopcnt   = %d (nested rebuild stop commands)\n", *(UINT8*)&rb_stopcnt
  printf "  r_stopcnt    = %d (nested scrub stop commands)\n", *(UINT8*)&r_stopcnt
  printf "  r_chkstopcnt = %d (nested mirror scan data checker stop commands)\n", *(UINT8*)&r_chkstopcnt
  printf "  outst_nv_reqs= %d (Count of snapshot NV requests to virtual)\n", outst_nv_reqs
  printf "  rb_rerrorbusy= %d [1=TRUE if raid error task busy]\n", *(UINT8*)&rb_rerrorbusy
  printf "  o_hs_stopped = %d [0=FALSE if o$hotswap is active]\n", *(UINT8*)&o_hs_stopped
  printf "  o_tur_stopped= %d [1=TRUE if online is NOT active]\n", *(UINT8*)&o_tur_stopped
  pdiskoutstanding
  print_dev_queues
  print_iram_ilt_head_tail
  printf "  V_orc        = %d, qcnt = %d\n", V_orc, ((QU*)&V_exec_qu)->qcnt
    set $I = 0
    while ($I < $MAXVIRTUALS)
      if ((*(VDX*)&gVDX).vdd[$I] > 0 && (*(VDX*)&gVDX).vdd[$I]->qd != 0)
        printf "    VDISK %d vd_qd = %d\n", $I, gVDX.vdd[$I]->qd
      end
      set $I = $I + 1
    end
  printf "  R_orc        = %d, qcnt = %d [RRP's sent to raid executive]\n", R_orc, ((QU*)&R_exec_qu)->qcnt
    set $I = 0
    while ($I < $MAXRAIDS)
      if ((*(RDX*)&gRDX).rdd[$I] > 0 && gRDX.rdd[$I]->qd != 0)
        printf "    RAID %d rd_qd = %d\n", $I, gRDX.rdd[$I]->qd
      end
      set $I = $I + 1
    end
end
#...
document beorcs
Print out all Back End orc values.
end
#-----------------------------------------------------------------------------
define feorcs
  # Cache outstanding host request count
  printf "  C_orc              = %d       desired matches fe{3,7}000CountOpsOutstanding\n", C_orc
  # Cache outstanding write SRP count
  printf "  C_owsrpc           = %d       desired 0\n", C_owsrpc
  # Cache outstanding read SRP count
  printf "  C_orsrpc           = %d       desired 0\n", C_orsrpc
  # DLM Vlink Outstanding Request Counter
  printf "  D_vlorc            = %d       desired 0\n", D_vlorc
  # Outstanding outbound VRP count
  printf "  LL_OutCount        = %d\n", (*((LL_STATS**)&L_stattbl))->LL_OutCount
#  printf "  LL_OutCount        = %d       desired 0\n", ((UINT16*)L_stattbl)[0]
  printf "\n"
  printf "  C_flush_orc        = %d       desired 0\n", C_flush_orc
  printf "  CA_OpRetryQue.qcnt = %d\n", ((QU*)&CA_OpRetryQue)->qcnt
  # Following non-zero means allowing vlink ops to go through (temporarily).
  printf "  c_allowVLinkOps    = %d\n", c_allowVLinkOps
  printf "\n"
  # Non-zero means still stop.
  printf "  ca_stopcnt         = %d       non-zero means still running c$stop\n", ((CA*)&C_ca)->stopCnt
  # Current Clear Write Info DRP count
  printf "  C_cwi_cdrp         = %d       desired 0\n", C_cwi_cdrp
  # Current Clear Write Info Queue count
  printf "  C_cwi_cqd          = %d       desired 0\n", C_cwi_cqd
  # 1 = not-ready
  printf "  C_cwi_pcb->pc_stat = %d       expected 1 (not ready)\n", ((PCB*)C_cwi_pcb)->pc_stat
  # Bit 5 (0x20) set is ca_error
  printf "  C_ca->status       = 0x%02x    do not want 0x20 bit set (ca_error)\n", ((CA*)&C_ca)->status
  printf "\n"
  # Controller Throttle Value
  printf "  C_ctv              = %d\n", C_ctv
  # Cache current queue depth
  printf "  C_exec_qcd         = %d\n", C_exec_qcd
  # T/F, Stalled due to WC Resources
  printf "  c_wcresourc        = %d\n", c_wcresourc
  # Current queue depth
  printf "  C_exec_cqd         = %d\n", C_exec_cqd
  # I/O current queue depth
  printf "  C_ioexec_cqd       = %d\n", C_ioexec_cqd
  printf "  mag_busy_count     = %d\n", *(UINT8*)&mag_busy_count
  printf "  c_flushErrorCount  = %d\n", *(UINT32*)&c_flushErrorCount
  printf "  c_eccErrorCount    = %d\n", *(UINT32*)&c_eccErrorCount
  printf "  c_invalidTagCount  = %d\n", *(UINT32*)&c_invalidTagCount
  printf "  c_vidFailed        = 0x%x\n", *(UINT32*)&c_vidFailed
end
#...
document feorcs
Print out all Front End orc values.
end
#-----------------------------------------------------------------------------
# NOTE: this gives all used and free. CAREFUL.
define traversepool
  set $START = $arg0
  if ($START == &K_ncdram)
    set $MEM = (struct before_after *)NcdrAddr + 1
    set $MAX_MEM = (char *)NcdrAddr + NcdrSize
  else
    if ($START == &P_ram)
      set $MEM = (struct before_after *)&local_memory_start + 1
      set $MAX_MEM = (char *)&local_memory_start + local_memory_pool_start - 64
    else
      if ($START == &c_wcfmm)
        set $MEM = (struct before_after *)&c_wcfmm
        set $MAX_MEM = (char *)WcbAddr + WcbSize - 64 - 32
      else
        printf "Bad input to macro traversepool\n"
        set $MEM = 0
        set $MAX_MEM = 0
      end
    end
  end
  set $MEM_START = $MEM
  if ($DETAIL == 0)
    printf "\n"
  else
    printf " "
  end
  set $COUNT = 0
  set $COUNT_USED = 0
  set $COUNT_FREE = 0
  set $LTH_USED = 0
  set $LTH_FREE = 0
  set $MAX_USED = 0
  set $MAX_FREE = 0
  while ($MEM != 0)
    set $RU = $MEM->length < 64 ? 64 : $MEM->length
    set $RU = ($RU + (64 - 1)) & ~(64 - 1)
    set $POST = (struct before_after *)((char *)($MEM + 1) + $RU)
    if ($DETAIL == 0)
      set $PRE = $MEM + 1
      print_before_after $PRE
    end
    set $COUNT = $COUNT + 1
    if ($MEM->used_or_free == 0)
      set $COUNT_FREE = $COUNT_FREE + 1
      set $LTH_FREE = $LTH_FREE + $MEM->length + 64
      if ($MAX_FREE < $MEM->length + 64)
        set $MAX_FREE = $MEM->length + 64
      end
    else
      set $COUNT_USED = $COUNT_USED + 1
      set $LTH_USED = $LTH_USED + $MEM->length + 64
      if ($MAX_USED < $MEM->length + 64)
        set $MAX_USED = $MEM->length + 64
      end
    end
    set $MEM = $POST + 1
    if ($MEM == $arg0)
      printf "ERROR, next pointer points to start\n"
      set $MEM = 0
    end
    if ($MEM >= $MAX_MEM)
      set $MEM = 0
    end
  end
  printf "  Count=%d\n", $COUNT
  if ($DETAIL == 0)
    printf "  Free: #=%d  total memory=0x%x (%d)  max free space=0x%x (%d)\n", $COUNT_FREE, $LTH_FREE, $LTH_FREE, $MAX_FREE, $MAX_FREE
    printf "  Used: #=%d  total memory=0x%x (%d)  max used space=0x%x (%d)\n", $COUNT_USED, $LTH_USED, $LTH_USED, $MAX_USED, $MAX_USED
  end
end
#...
document traversepool
Print out summary or detailed of memory pool usage. CAREFUL, detail can be lots of output.
end
#-----------------------------------------------------------------------------
# Allow this (ccb) name to be used.
define print_mem_header
  print_before_after $arg0
end
#...
document print_mem_header
Print out the before_after structures of a malloc-ed memory address (argument). (ccb name)
end
#...
define pb
  print_before_after $arg0
end
#...
document pb
Short for print_before_after
end
#...
# Print the before and after malloc-ed structures.
define print_before_after
  set $ADDR = (char *)$arg0
  set $PRE = (struct before_after *)$ADDR - 1
  printf "  PRE for  0x%08lx (0x%08lx)", $ADDR, (UINT32)$PRE
  printf " %08lx %11.11s:%5d u/f=%d next=0x%08lx lth=%d %08lx\n", $PRE->pre1, $PRE->str, $PRE->line_number, $PRE->used_or_free, (UINT32)($PRE->next), $PRE->length, $PRE->pre2
# x/8 $PRE
  set $ASIZE = $PRE->length <  64 ? 64 : $PRE->length
  set $AROUND = ($ASIZE + 64 - 1) & ~(64 - 1)
  set $POST = (struct before_after *)($ADDR + $AROUND)
  printf "  POST for 0x%08lx (0x%08lx)", $ADDR, (UINT32)$POST
  printf " %08lx %11.11s:%5d u/f=%d next=0x%08lx lth=%d %08lx\n", $POST->pre1, $POST->str, $POST->line_number, $POST->used_or_free, (UINT32)($POST->next), $POST->length, $POST->pre2
# x/8 $POST
  set $PRINT_NEXT = $POST + 2
end
#...
document print_before_after
Print out the before_after structures of a malloc-ed memory address (argument).
end
#-----------------------------------------------------------------------------
define print_next
  print_before_after $PRINT_NEXT
end
#...
document print_next
Print out the "next" before_after structures of malloc-ed memory. (Use after print_before_after.)
end
#-----------------------------------------------------------------------------
define follow_pool_links
  set $POOL = (struct memory_pool *)&($arg0)
  set $ADDR = $POOL->first_allocated
  set $CNT = 0
  while ((unsigned long)$ADDR != 0 && (unsigned long)$ADDR != 0xdcdcdcdc)
    set $CNT = $CNT + 1
    set $ADDR = ((struct before_after *)$ADDR)->next
  end
  printf "Pool total linked list count=%d, allocated=%d, free=%d + used=%d\n", $CNT, $POOL->num_allocated, $POOL->num_free, $POOL->num_allocated - $POOL->num_free
end
#...
document follow_pool_links
Print out count of allocated structures, using first_allocated->next links, and pool counters. FOR 840 and beyond.
end
#-----------------------------------------------------------------------------
define traverseprivatepool
  if ($DETAIL == 0)
    printf "\n"
  else
    printf " "
  end
  set $START = $arg0.first
  set $DONE = 0
  set $COUNT = 0
  while ($DONE != 1 && $START != 0)
    set $PRE = (struct before_after *)$START - 1
    if ($DETAIL == 0)
      printf "  0x%08x  next = 0x%08x\n", $START, $PRE->next
    end
    set $COUNT = $COUNT + 1
    set $TAIL = $START
    set $START = $PRE->next
    if ($START == $arg0.first || $START == 0)
      set $DONE = 1
      if ($START != 0)
        printf "ERROR, next pointer points to start\n"
      else
          if ($arg0.tail != $TAIL)
            printf "ERROR, tail (0x%08x) not last entry (0x%08x)\n", $arg0.tail, $TAIL
          end
      end
    end
  end
  if ($COUNT == $arg0.num_free)
    printf "  Free=%d  Allocated=%d  Used=%d\n", $COUNT, $arg0.num_allocated, $arg0.num_allocated-$COUNT
  else
    printf "  ERROR, free (%d) != in-pool (%d)   ... allocated=%d\n",  $COUNT, $arg0.num_free, $arg0.num_allocated
  end
end
#...
document traverseprivatepool
Print out summary of private memory pools.
end
#-----------------------------------------------------------------------------
define memlists
  set $DETAIL = 0
  memlistsprint
end
#...
document memlists
Print out all memory lists in detail (each element free in list/pool).
end
set $DETAIL = 1
#-----------------------------------------------------------------------------
# $FE_MEMORY_STARTS is where FE shared memory is.
define memlistsprint
  # FE only
  if (((UINT32)(&K_ii) & 0xffff0000) == $FE_MEMORY_STARTS)
      printf "tmt:"
         traverseprivatepool pool_tmt
      printf "tlmt:"
         traverseprivatepool pool_tlmt
      printf "ismt:"
         traverseprivatepool pool_ismt
      printf "xli:"
         traverseprivatepool pool_xli
      printf "ltmt:"
         traverseprivatepool pool_ltmt
      printf "irp:"
         traverseprivatepool pool_irp
      printf "lsmt:"
         traverseprivatepool pool_lsmt
      printf "imt:"
         traverseprivatepool pool_imt
      printf "vdmt:"
         traverseprivatepool pool_vdmt
      printf "ilmt:"
         traverseprivatepool pool_ilmt
      printf "wc_plholder:"
         traverseprivatepool pool_wc_plholder
      printf "wc_rbinode:"
         traverseprivatepool pool_wc_rbinode
      printf "wc_rbnode:"
         traverseprivatepool pool_wc_rbnode
  end
  # BE only
  if (((UINT32)(&K_ii) & 0xffff0000) != $FE_MEMORY_STARTS)
      printf "prp:"
         traverseprivatepool pool_prp
      printf "rrp:"
         traverseprivatepool pool_rrp
      printf "rpn:"
         traverseprivatepool pool_rpn
      printf "rrb:"
         traverseprivatepool pool_rrb
      printf "vlar:"
         traverseprivatepool pool_vlar
      printf "rm:"
         traverseprivatepool pool_rm
      printf "sm:"
         traverseprivatepool pool_sm
      printf "cm:"
         traverseprivatepool pool_cm
      printf "cor:"
         traverseprivatepool pool_cor
      printf "scd:"
         traverseprivatepool pool_scd
      printf "dcd:"
         traverseprivatepool pool_dcd
      printf "tpmt:"
         traverseprivatepool pool_tpmt
      printf "scmt:"
         traverseprivatepool pool_scmt
# NOTE: this does nothing with the vrp's that are attached to il_w0 of ilt.
      printf "scio:"
         traverseprivatepool pool_scio
  end
  # Both -- put these later, they are big.
  printf "qrp:"
     traverseprivatepool pool_qrp
  printf "dtmt:"
     traverseprivatepool pool_dtmt
  printf "mlmt:"
     traverseprivatepool pool_mlmt
  printf "ilt:"
     traverseprivatepool pool_ilt
  printf "vrp:"
     traverseprivatepool pool_vrp
  printf "pcb:"
     traverseprivatepool pool_pcb

  printf "private memory starts @ 0x%08x for %d (0x%08.8x)\n", &local_memory_start, local_memory_pool_start, local_memory_pool_start
     traversepool &P_ram
  printf "K_ncdram:"
     traversepool &K_ncdram
end
#...
document memlistsprint
Print out all memory lists, use variable $DETAIL for full or count.
end
#-----------------------------------------------------------------------------
define memlistscount
  set $DETAIL = 1
  memlistsprint
end
#...
document memlistscount
For all memory lists, print out the count free, number-allocated, etc.
end
#-----------------------------------------------------------------------------
define print_local_memory
  set $START = (UINT32)&local_memory_start+32
  set $LTH = 0
  while ($LTH < (local_memory_pool_start-64))
    set $PRE = (struct before_after *)$START
    set $SIZE = $PRE->length
    if ($SIZE == 0 || $SIZE > 400*1024*1024)
      printf "Address %p length %u is not reasonable.\n", $START, $SIZE
      stopnow
    end
    set $ADDRESS = $START + sizeof(struct before_after)
    set $ROUND_UP_SIZE = ($SIZE + 64 - 1) & ~(64 - 1)
    set $TOTAL_SIZE = $ROUND_UP_SIZE + sizeof(struct before_after) + sizeof(struct before_after)
    set $POST = (struct before_after *)($ADDRESS + $ROUND_UP_SIZE)
    if ($LTH + $TOTAL_SIZE > (local_memory_pool_start - 64))
      printf "Address %p length %u puts us beyond end of allocated.\n", $ADDRESS, $SIZE
      stopnow
    end
#    printf "  %p %6.6s %p u/f=%d size=%4.4d (%4.4d)\n", $START, $PRE->str, $ADDRESS, $PRE->used_or_free, $SIZE, $TOTAL_SIZE,
    printf "  %p %11.11s:%5u %11.11s:%5u %p u/f=%d size=%5.5x (%5d)\n", $START + 32, $PRE->str, $PRE->line_number, $POST->str, $POST->line_number, $ADDRESS, $PRE->used_or_free, $SIZE, $TOTAL_SIZE,
    if ($PRE->used_or_free != $POST->used_or_free)
      printf "Address %p does not have used_or_free matching -- %d/%d\n", $ADDRESS, $PRE->used_or_free, $POST->used_or_free
      stopnow
    end
    if ($PRE->used_or_free != $POST->used_or_free)
      printf "Address %p does not have used_or_free matching -- %d/%d\n", $ADDRESS, $PRE->used_or_free, $POST->used_or_free
      stopnow
    end
    if ($PRE->pre1 != 0xdddddddd)
      printf "Address %p does not have PRE->pre1 matching 0xdddddddd -- %d\n", $ADDRESS, $PRE->pre1
      stopnow
    end
    if ($PRE->pre2 != 0xdddddddd)
      printf "Address %p does not have PRE->pre2 matching 0xdddddddd -- %d\n", $ADDRESS, $PRE->pre2
      stopnow
    end
    if ($POST->pre1 != 0xdcdcdcdc)
      printf "Address %p does not have POST->pre1 matching 0xdcdcdcdc -- %d\n", $ADDRESS, $POST->pre1
      stopnow
    end
    if ($POST->pre2 != 0xdcdcdcdc)
      printf "Address %p does not have POST->pre2 matching 0xdcdcdcdc -- %d\n", $ADDRESS, $POST->pre2
      stopnow
    end
    if ($PRE->length != $POST->length)
      printf "Address %p does not have length matching -- %d/%d\n", $ADDRESS, $PRE->length, $POST->length
      stopnow
    end
    set $START = $START + $TOTAL_SIZE
    set $LTH = $LTH + $TOTAL_SIZE
  end
end
#...
document print_local_memory
Print out complete local memory allocation (every one).
end
#-----------------------------------------------------------------------------
define print_shared_memory
  set $START = (UINT32)NcdrAddr + 32
  set $LTH = 0
  while ($LTH < (NcdrSize-64))
    set $PRE = (struct before_after *)$START
    set $SIZE = $PRE->length
    if ($SIZE == 0 || $SIZE > 400*1024*1024)
      printf "Address %p length %u is not reasonable.\n", $START, $SIZE
      stopnow
    end
    set $ADDRESS = $START + sizeof(struct before_after)
    set $ROUND_UP_SIZE = ($SIZE + 64 - 1) & ~(64 - 1)
    set $TOTAL_SIZE = $ROUND_UP_SIZE + sizeof(struct before_after) + sizeof(struct before_after)
    set $POST = (struct before_after *)($ADDRESS + $ROUND_UP_SIZE)
    if ($LTH + $TOTAL_SIZE > (NcdrSize - 64))
      printf "Address %p length %u puts us beyond end of allocated.\n", $ADDRESS, $SIZE
      stopnow
    end
    printf "  %p %11.11s:%5u %11.11s:%5u %p u/f=%d size=%5.5x (%5d)\n", $START + 32, $PRE->str, $PRE->line_number, $POST->str, $POST->line_number, $ADDRESS, $PRE->used_or_free, $SIZE, $TOTAL_SIZE,
    if ($PRE->used_or_free != $POST->used_or_free)
      printf "Address %p does not have used_or_free matching -- %d/%d\n", $ADDRESS, $PRE->used_or_free, $POST->used_or_free
      stopnow
    end
    if ($PRE->used_or_free != $POST->used_or_free)
      printf "Address %p does not have used_or_free matching -- %d/%d\n", $ADDRESS, $PRE->used_or_free, $POST->used_or_free
      stopnow
    end
    if ($PRE->pre1 != 0xdddddddd)
      printf "Address %p does not have PRE->pre1 matching 0xdddddddd -- %d\n", $ADDRESS, $PRE->pre1
      stopnow
    end
    if ($PRE->pre2 != 0xdddddddd)
      printf "Address %p does not have PRE->pre2 matching 0xdddddddd -- %d\n", $ADDRESS, $PRE->pre2
      stopnow
    end
    if ($POST->pre1 != 0xdcdcdcdc)
      printf "Address %p does not have POST->pre1 matching 0xdcdcdcdc -- %d\n", $ADDRESS, $POST->pre1
      stopnow
    end
    if ($POST->pre2 != 0xdcdcdcdc)
      printf "Address %p does not have POST->pre2 matching 0xdcdcdcdc -- %d\n", $ADDRESS, $POST->pre2
      stopnow
    end
    if ($PRE->length != $POST->length)
      printf "Address %p does not have length matching -- %d/%d\n", $ADDRESS, $PRE->length, $POST->length
      stopnow
    end
    set $START = $START + $TOTAL_SIZE
    set $LTH = $LTH + $TOTAL_SIZE
  end
end
#...
document print_shared_memory
Print out complete shared memory allocation (every one).
end
#-----------------------------------------------------------------------------
define print_writecache_memory
  set $START = (UINT32)WcbAddr + 32
  set $LTH = 0
  while ($LTH < (WcbSize-64))
    set $PRE = (struct before_after *)$START
    set $SIZE = $PRE->length
    if ($SIZE == 0 || $SIZE > 400*1024*1024)
      printf "Address %p length %u is not reasonable.\n", $START, $SIZE
      stopnow
    else
      set $ADDRESS = $START + sizeof(struct before_after)
      set $ROUND_UP_SIZE = ($SIZE + 64 - 1) & ~(64 - 1)
      set $TOTAL_SIZE = $ROUND_UP_SIZE + sizeof(struct before_after) + sizeof(struct before_after)
      if ($LTH + $TOTAL_SIZE > (WcbSize - 64))
        printf "Address %p length %u puts us beyond end of allocated.\n", $ADDRESS, $SIZE
        stopnow
      else
        printf "  %p %6.6s %p u/f=%d size=%4.4d (%4.4d)\n", $START, $PRE->str, $ADDRESS, $PRE->used_or_free, $SIZE, $TOTAL_SIZE,
        set $START = $START + $TOTAL_SIZE
        set $LTH = $LTH + $TOTAL_SIZE
      end
    end
  end
end
#...
document print_writecache_memory
Print out complete writecache memory allocation (every one).
end
#=============================================================================
# This is for printing the ccsm trace log in ccsm.as
define ccsmtracelog
  set $I = 0
  set $CCSMTRACE = (unsigned int *)&ccsm_tr_area
  set $CCSMSTART = (char *)(*($CCSMTRACE + 1))
  set $CCSMCUR = (char *)(*($CCSMTRACE + 1))
  set $CCSMHEAD = (char *)(*($CCSMTRACE + 2))
  set $CCSMTAIL = (char *)(*($CCSMTRACE + 3))
  set $DONE = 0
  while ($DONE != 1)
    set $I = $I + 1
    set $C0 = ((UINT32*)$CCSMCUR)[0]
    set $C1 = ((UINT32*)$CCSMCUR)[1]
    set $C2 = ((UINT32*)$CCSMCUR)[2]
    set $C3 = ((UINT32*)$CCSMCUR)[3]
    printf "%4d 0x%08x 0x%08x 0x%08x 0x%08x\n", $I, $C0, $C1, $C2, $C3
    set $CCSMCUR = $CCSMCUR + 16
    if ($CCSMCUR == $CCSMSTART )
      set $DONE = 1
    end
    if ($CCSMCUR > $CCSMTAIL)
      set $CCSMCUR = $CCSMHEAD
    end
  end
end
#...
document ccsmtracelog
Print out ccsm trace log kept in ccsm.as.
end
#=============================================================================
# For task dlm$lrpage.
define number_ldds
  set $I = 0
  set $L = (UINT32)&DLM_lddindx
  set $C = 0
  while ($I < 512)
    if ((UINT32)(*(UINT32*)$L) != 0)
      set $C = $C + 1
      printf "%2d DLM_lddindx[%d] has entry 0x%08x\n", $C, $I, (UINT32)(*(UINT32*)$L)
    end
    set $I = $I + 1
    set $L = $L + 4
  end
  printf "There are %d entries in DLM_lddindx.\n", $C
end
#...
document number_ldds
Count the number (and print) the LDDs. For task dlm$lrpage.
end
#-----------------------------------------------------------------------------
# For task dlm$vlchk.
define number_rdlinkdev
  set $I = 0
  set $V = (VDX*)&gVDX
  set $M = sizeof($V->vdd)/sizeof(void *)
  set $C = 0
  while ($I < $M)
    if ((UINT32)($V->vdd[$I]) != 0 && (UINT32)($V->vdd[$I]->pRDD) != 0 && $V->vdd[$I]->pRDD->type == 5)
      set $C = $C + 1
      printf "%2d vdd[%d] is rdlinkdev\n", $C, $I
    end
    set $I = $I + 1
  end
  printf "There are %d rdlinkdev raids.\n", $C
end
#...
document number_rdlinkdev
Count the number (and print) of raids that have type rdlinkdev. For task dlm$vlchk.
end
#-----------------------------------------------------------------------------
# For task dlm$vlock
define number_vlinks
  set $I = 0
  set $V = (VDX*)&gVDX
  set $M = sizeof($V->vdd)/sizeof(void *)
  set $C = 0
  while ($I < $M)
    if ((UINT32)($V->vdd[$I]) != 0 && (UINT32)($V->vdd[$I]->pVLinks) != 0)
      set $C = $C + 1
      printf "%2d vdd[%d] has VLAR 0x%08x\n", $C, $I, (UINT32)($V->vdd[$I]->pVLinks)
    end
    set $I = $I + 1
  end
  printf "There are %d vdisks with pVLinks nonzero.\n", $C
end
#...
document number_vlinks
Count the number (and print) of vdisks that have pVLinks nonzero. For task dlm$vlock.
end
#-----------------------------------------------------------------------------
# This macro prints the vlink/vdisk information.
define vlinks
    set $I = 0
    printf "         VID DEVSTAT         CAPACITY MIRROR   ATTR #RAID scVID PCT Vop   SN# VPORT LUN Raid\n"
    #       .        123 ...0x00 1234567890123456   0x00 0x0000 12345 12345 100   1 12345   xxx 345 xx
    #       --------------------------------------------------------------------------------

    while ($I < $MAXVIRTUALS)
        if ((*(VDX*)&gVDX).vdd[$I] > 0)
# If remote storage ... vlink flag set
            if (gVDX.vdd[$I].attr & 0x0080)
                printf "Remote-->"
                printf "%3hu",      gVDX.vdd[$I].vid
                printf "    0x%02x", gVDX.vdd[$I].status
                printf " %16llu",    gVDX.vdd[$I].devCap
                printf "   0x%02x",  gVDX.vdd[$I].mirror
                printf " 0x%4.4x",   gVDX.vdd[$I].attr
                printf " %5u",       gVDX.vdd[$I].raidCnt + gVDX.vdd[$I].draidCnt
                printf " %5u",     gVDX.vdd[$I].scorVID
                printf " %3u",       gVDX.vdd[$I].scpComp
                printf "   %1u",     gVDX.vdd[$I].grInfo.vdOpState

                set $J = 0
                set $RDD = gVDX.vdd[$I]->pRDD
                set $PID = $RDD->extension.pPSD[0]->pid
                set $DLMAT = (UINT32*)DLM_lddindx
                set $LDD = *($DLMAT + $PID)
                if ($LDD != 0)
                  printf " %5u",    ((LDD*)$LDD)->baseSN
                  printf "   %3u",    ((LDD*)$LDD)->baseCluster
                  printf "   %3u",    ((LDD*)$LDD)->baseVDisk
                else
                  printf "                "
                end
                printf " "
                while ($RDD > 0)
                    if ($J > 0)
                        printf ","
                    end
                    printf "%hu", $RDD->rid
                    set $J = $J + 1
                    set $RDD = $RDD->pNRDD
                end
                printf "\n"
            end
        end
        set $I = $I + 1
    end
end
#...
document vlinks
Print the vlink/vdisk information
end
#-----------------------------------------------------------------------------

#=============================================================================
####
## Modelines:
## Local Variables:
## tab-width: 4
## indent-tabs-mode: nil
## End:
## vi:sw=4 ts=4 expandtab
# End of file .gdbinit
#-----------------------------------------------------------------------------
define map
    printf "FE shared memory starts at      = 0x%08.8x\n", (UINT32)$FE_MEMORY_STARTS
    printf "CCB shared memory starts at     = 0x%08.8x\n", (UINT32)(pCCBSharedMem) & 0xffff0000
    printf "BE shared memory starts at      = 0x%08.8x\n", (UINT32)startOfBESharedMem
    # NVR starts immediately after BE
    printf "NVR memory starts at            = 0x%08.8x\n", (UINT32)endOfBESharedMem
    printf "INFO region starts at           = 0x%08.8x\n", (UINT32)ptr_xio3d_drvinfo
    # MMIO starts at (INFO+4096 + (0x10000000-1)) & ~(0x10000000-1)
    set $MMIOstart = ((UINT32)ptr_xio3d_drvinfo+4096+(0x10000000-1)) & ~(0x10000000-1)
    printf "MMIO region starts at           = 0x%08.8x\n", $MMIOstart
    # BE ONLY starts after MMIO.
    set $BEonly = $MMIOstart + 0x10000000
    printf "BE only memory starts at        = 0x%08.8x\n", $BEonly
    # FE ONLY starts after BE ONLY. Don't know size, but guess it is same as ours.
    set $FEonly = $BEonly + local_memory_pool_start
    printf "FE only memory guessed starts   = 0x%08.8x\n", $FEonly
    # CCB only memory starts after FE ONLY, guessed size matches CCB shared.
    set $CCBonly = $FEonly + local_memory_pool_start
    printf "CCB only memory guessed at      = 0x%08.8x\n", $CCBonly
    # CCB data only memory starts after CCB ONLY, Do not know the size, but limit is 64mb.
    set $CCBdata = $CCBonly + 0x04000000
    printf "CCB data only memory guessed at = 0x%08.8x\n", $CCBdata
    # Guess the end of allocated memory
    set $ENDallocated = $CCBdata + 0x04000000
    printf "end of allocated memory guessed = 0x%08.8x\n", $ENDallocated
    printf "\n"
    if (((UINT32)(&K_ii) & 0xffff0000) == $FE_MEMORY_STARTS)
      printf "MY (FE) private memory starts at= 0x%08.8x\n", (UINT32)&local_memory_start
    else
      printf "MY (BE) private memory starts at= 0x%08.8x\n", (UINT32)&local_memory_start
    end
    printf "size_ILT_ALL_LEVELS             = 0x%08.8x (%u)\n", (UINT32)size_ILT_ALL_LEVELS, (UINT32)size_ILT_ALL_LEVELS
    printf "\n"
    printf "&K_ii & 0xffff0000              = 0x%08.8x\n", ((UINT32)(&K_ii) & 0xffff0000)
    # FE then print BE, BE then print FE
    if (((UINT32)(&K_ii) & 0xffff0000) == $FE_MEMORY_STARTS)
      printf "pBESharedMem                    = 0x%08.8x\n", pBESharedMem
    else
      printf "pFESharedMem                    = 0x%08.8x\n", pFESharedMem
    end
    printf "endOfBESharedMem                = 0x%08.8x\n", (UINT32)endOfBESharedMem
    #
    printf "startOfMySharedMem              = 0x%08.8x\n", (UINT32)startOfMySharedMem
    printf "endOfMySharedMem                = 0x%08.8x\n", (UINT32)endOfMySharedMem
    #
    printf "pCCBSharedMem                   = 0x%8.8x\n", (UINT32)pCCBSharedMem
    printf "pStartOfHeap                    = 0x%08.8x\n", (UINT32)pStartOfHeap
    #
    printf "\n"
    printf "End (Limit) address             = 0x%08.8x\n", (UINT32)LimtAddr
    printf "WriteCache Control Table size   = 0x%08.8x (%u)\n", (UINT32)WcctSize, (UINT32)WcctSize
    printf "Write Cache Control Tabl addr   = 0x%08.8x\n", (UINT32)WcctAddr
    printf "Write Cache Config size         = 0x%08.8x (%u)\n", (UINT32)WccSize, (UINT32)WccSize
    printf "Write Cache Config address      = 0x%08.8x\n", (UINT32)WccAddr
    printf "Write Cache Tag size            = 0x%08.8x (%u)\n", (UINT32)WctSize, (UINT32)WctSize
    printf "Write Cache Tag address         = 0x%08.8x\n", (UINT32)WctAddr
    printf "Write Cache Buffer size         = 0x%08.8x (%u)\n", (UINT32)WcbSize, (UINT32)WcbSize
    printf "Write Cache Buffer address      = 0x%08.8x\n", (UINT32)WcbAddr
    printf "Non-cacheable DRAM address      = 0x%08.8x\n", (UINT32)NcdrAddr
    printf "Non-cacheable DRAM size         = 0x%08.8x (%u)\n", (UINT32)NcdrSize, (UINT32)NcdrSize
    printf "Battery backup data 1 addr      = 0x%08.8x\n", (UINT32)Bbd1Addr
    printf "Battery backup data 2 addr      = 0x%08.8x\n", (UINT32)Bbd2Addr
end
#...
document map
Print out where the various memory segments are (Assumes lengths in gdb macro match Makefile-*.defs).
end
#-----------------------------------------------------------------------------
# Run the ILT free list, return lowest ILT address.
define lowest_free_ilt
  set $I = (ILT*)M_iltorgc
  set $L = $I
  set $H = $I
  set $C = 0
  while ((UINT32)$I != 0)
    if ((UINT32)$I < (UINT32)$L)
      set $L = $I
    end
    if ((UINT32)$I > (UINT32)$H)
      set $H = $I
    end
    set $C = $C + 1
printf "."
    set $I = $I->fthd
  end
  if ($C != ((II*)&K_ii)->iltCur)
    printf "\nERROR: ILT free list contains %d entries, K_ii.iltMax is %d\n", $C, ((II*)&K_ii)->iltCur
  end
  if ($C > ((II*)&K_ii)->iltMax || ((II*)&K_ii)->iltCur > ((II*)&K_ii)->iltMax)
    printf "\nERROR: ILT free list has more free (%d or %d) than K_ii.iltMax %d\n", $C, ((II*)&K_ii)->iltCur, ((II*)&K_ii)->iltMax
  end
  printf "\nThere are %d entries (of %d) on the free ILT list, lowest address = %p, highest = %p\n", $C, ((II*)&K_ii)->iltMax, $L, $H
end
#...
document lowest_free_ilt
Find the lowest addressed free ilt.
end
#=============================================================================
# This macro prints out the allocated ILTs.
# This is for 0830.
#    You have to figure out where the first ILT is.
#     PERF build, BE length of ILT allocation (7*52)=572, rounded to 64 byte boundary is 384.
#     K_ficb is last high bit allocation before it. Thus start is at K_ficb-384, as there
#        is no electric fence in PERF build.
define print_allocated_ilts
  set $ILTSIZE = (size_ILT_ALL_LEVELS+(64-1)) & ~(64-1)
  set $ADDR = (struct before_after *)pool_ilt.first_allocated
  set $CNT = 0
  set $FREECNT = 0
  set $USEDCNT = 0
  while ((unsigned long)$ADDR != 0 && (unsigned long)$ADDR != 0xdcdcdcdc)
    set $CNT = $CNT + 1
    set $ILT = (unsigned long)$ADDR - $ILTSIZE
    set $PRE = (struct before_after *)($ILT - sizeof(struct before_after))
    if ($PRE->used_or_free == 0)
      set $FREECNT = $FREECNT + 1
#      printf "0x%08x free-listILT\n", $ILT
    else
      set $USEDCNT = $USEDCNT + 1
      set $D = size_ILT_ALL_LEVELS/8
      set $LEFT = (size_ILT_ALL_LEVELS - ($D * 8)) / 4
      set $Z = 0
      set $I = (UINT64*)$ILT
      while ($D > 0 && $Z == 0)
        if (*$I != 0)
          set $Z = 1
        end
        set $D = $D - 1
        set $I = $I + 1
      end
      set $I = (UINT32*)$I
      while ($LEFT > 0 && $Z == 0)
        if (*$I != 0)
          set $Z = 1
        end
        set $LEFT = $LEFT - 1
        set $I = $I + 1
      end
      if ($Z != 0)
        printf "0x%08x allocated-non-zeroILT\n", $ILT
      else
        printf "0x%08x allocated-zeroILT\n", $ILT
      end
    end
    set $ADDR = (struct before_after *)($ADDR->next)
  end
  printf "pool_ilt total linked list count=%d, allocated=%d, free=%d + used=%d\n", $CNT, pool_ilt.num_allocated, pool_ilt.num_free, pool_ilt.num_allocated - pool_ilt.num_free
  printf "FREECNT = %d  USEDCNT = %d\n", $FREECNT, $USEDCNT
end
#-----------------------------------------------------------------------------
# This is for 0830.
# Run the ILT free list, return lowest ILT address.
define print_free_ilts
  set $I = (ILT*)M_iltorgc
  while ((UINT32)$I != 0)
    printf "0x%08x free-listILT\n", $I
    set $I = $I->fthd
  end
end
#-----------------------------------------------------------------------------
# This is for 0830.
# Run the RRB free list, return lowest RRB address.
define lowest_free_rrb
  set $I = (ILT*)M_rrborg
  set $L = $I
  set $H = $I
  set $C = 0
  while ((UINT32)$I != 0)
    if ((UINT32)$I < (UINT32)$L)
      set $L = $I
    end
    if ((UINT32)$I > (UINT32)$H)
      set $H = $I
    end
    set $C = $C + 1
printf "."
    set $I = $I->fthd
  end
  if ($C != ((II*)&K_ii)->rrbCur)
    printf "\nERROR: RRB free list contains %d entries, K_ii.rrbMax is %d\n", $C, ((II*)&K_ii)->rrbCur
  end
  if ($C > ((II*)&K_ii)->rrbMax || ((II*)&K_ii)->rrbCur > ((II*)&K_ii)->rrbMax)
    printf "\nERROR: RRB free list has more free (%d or %d) than K_ii.rrbMax %d\n", $C, ((II*)&K_ii)->rrbCur,
((II*)&K_ii)->rrbMax
  end
  printf "\nThere are %d entries (of %d) on the free RRB list, lowest address = %p, highest = %p\n", $C, ((II*)&K_ii)->rrbMax, $L, $H
  set $LOWRRB = $L
end
#-----------------------------------------------------------------------------
# This macro prints out the SCIO free list.
define print_free_scios
  set $ADDR = (unsigned long)pool_scio.first
  set $CNT = 0
  while ($ADDR != 0 && $ADDR != 0xdcdcdcdc)
    set $CNT = $CNT + 1
    set $PRE = (struct before_after *)($ADDR - sizeof(struct before_after))
    printf "0x%08x free-scio\n", $ADDR
    set $ADDR = (unsigned long)($PRE->next)
  end
  printf "pool_scio total linked list count=%d, allocated=%d, free=%d + used=%d\n", $CNT, pool_scio.num_allocated, pool_scio.num_free, pool_scio.num_allocated - pool_scio.num_free
  if ($CNT != pool_scio.num_free)
    printf "ERROR counted free (%d) != Free (%d)\n", $CNT, $pool_scio.num_free
  end
end
#-----------------------------------------------------------------------------
# This is for 0830.
# Run the RRP free list
define print_free_rrps
  set $I = (ILT*)M_rrporg
  while ((UINT32)$I != 0)
    printf "0x%08x free-listRRP\n", $I
    set $I = $I->fthd
  end
end
#-----------------------------------------------------------------------------
# This is for 0830.
define print_allocated_rrps
  # RRP's are allocated after prp's, and before rpn's.
  lowest_free_prp
  lowest_free_rpn
  # Thus LOWPRP-sizeof(RRP) = highest RRP.
  # Thus HIGHRPN+sizeof(RPN) = lowest RRP.
  # sizeof(RPN)=80 -- not in 0830 symbols. (80+63)/64=2 therefore allocated=128
  set $LOWRRP = (UINT32)$HIGHRPN+128
  set $HIGHRRP = (UINT32)$LOWPRP-sizeof(RRP)
  set $RRPSIZE = ((UINT32)((sizeof(RRP)+63)/64))*64
  set $I = $LOWRRP
  while ($I <= $HIGHRRP)
    printf "0x%08x allocated\n", $I
    set $FOUND = 0
    set $C = (ILT*)(*(UINT32*)M_rrporg)
    while ($FOUND == 0 && (UINT32)$C != 0)
      if ($I == (UINT32)$C)
        set $FOUND = 1
      else
        set $C = $C->fthd
      end
    end
    if ($FOUND == 0)
      printf "RRP not found: 0x%08x\n", $I
    end
    set $I = $I + $RRPSIZE
  end
  #
end
#-----------------------------------------------------------------------------
# This is for 0830.
# Run the PRP free list, return lowest/highest PRP addresses.
define lowest_free_prp
  set $I = (ILT*)M_prporg
  set $L = $I
  set $H = $I
  set $C = 0
  while ((UINT32)$I != 0)
    if ((UINT32)$I < (UINT32)$L)
      set $L = $I
    end
    if ((UINT32)$I > (UINT32)$H)
      set $H = $I
    end
    set $C = $C + 1
printf "."
    set $I = $I->fthd
  end
  if ($C != ((II*)&K_ii)->prpCur)
    printf "\nERROR: PRP free list contains %d entries, K_ii.prpMax is %d\n", $C, ((II*)&K_ii)->prpCur
  end
  if ($C > ((II*)&K_ii)->prpMax || ((II*)&K_ii)->prpCur > ((II*)&K_ii)->prpMax)
    printf "\nERROR: PRP free list has more free (%d or %d) than K_ii.prpMax %d\n", $C, ((II*)&K_ii)->prpCur, ((II*)&K_ii)->prpMax
  end
  printf "\nThere are %d entries (of %d) on the free PRP list, lowest address = %p, highest = %p\n", $C, ((II*)&K_ii)->prpMax, $L, $H
  set $LOWPRP = $L
  set $HIGHPRP = $H
end
#-----------------------------------------------------------------------------
# This is for 0830.
# Run the RPN free list, return lowest/highest RPN addresses.
define lowest_free_rpn
  set $I = (ILT*)M_rpnorg
  set $L = $I
  set $H = $I
  set $C = 0
  while ((UINT32)$I != 0)
    if ((UINT32)$I < (UINT32)$L)
      set $L = $I
    end
    if ((UINT32)$I > (UINT32)$H)
      set $H = $I
    end
    set $C = $C + 1
printf "."
    set $I = $I->fthd
  end
  if ($C != ((II*)&K_ii)->rpnCur)
    printf "\nERROR: RPN free list contains %d entries, K_ii.rpnMax is %d\n", $C, ((II*)&K_ii)->rpnCur
  end
  if ($C > ((II*)&K_ii)->rpnMax || ((II*)&K_ii)->rpnCur > ((II*)&K_ii)->rpnMax)
    printf "\nERROR: RPN free list has more free (%d or %d) than K_ii.rpnMax %d\n", $C, ((II*)&K_ii)->rpnCur, ((II*)&K_ii)->rpnMax
  end
  printf "\nThere are %d entries (of %d) on the free RPN list, lowest address = %p, highest = %p\n", $C, ((II*)&K_ii)->rpnMax, $L, $H
  set $LOWRPN = $L
  set $HIGHRPN = $H
end
#-----------------------------------------------------------------------------
# This is for 0830.
# Run the PRP free list
define print_free_prps
  set $I = (ILT*)M_prporg
  while ((UINT32)$I != 0)
    printf "0x%08x free-listPRP\n", $I
    set $I = $I->fthd
  end
end
#-----------------------------------------------------------------------------
# Print only allocated for <-830, else allocated and free.
# K_ficb is last high bit allocation before it. Thus start is at K_ficb-malloced(PRP), as there
# is no electric fence in PERF build.
define print_allocated_prps
  set $PRPSIZE = (sizeof(PRP)+(64-1)) & ~(64-1)
  set $ADDR = (struct before_after *)pool_prp.first_allocated
  set $CNT = 0
  set $FREECNT = 0
  set $USEDCNT = 0
  while ((unsigned long)$ADDR != 0 && (unsigned long)$ADDR != 0xdcdcdcdc)
    set $CNT = $CNT + 1
    set $PRP = (unsigned long)$ADDR - $PRPSIZE
    set $PRE = (struct before_after *)($PRP - sizeof(struct before_after))
    if ($PRE->used_or_free == 0)
      set $FREECNT = $FREECNT + 1
#      printf "0x%08x free-listPRP\n", $PRP
    else
      set $USEDCNT = $USEDCNT + 1
      set $D = sizeof(PRP)/8
      set $LEFT = (sizeof(PRP) - ($D * 8)) / 4
      set $Z = 0
      set $I = (UINT64*)$PRP
      while ($D > 0 && $Z == 0)
        if (*$I != 0)
          set $Z = 1
        end
        set $D = $D - 1
        set $I = $I + 1
      end
      set $I = (UINT32*)$I
      while ($LEFT > 0 && $Z == 0)
        if (*$I != 0)
          set $Z = 1
        end
        set $LEFT = $LEFT - 1
        set $I = $I + 1
      end
      if ($Z != 0)
        printf "0x%08x allocated-non-zeroPRP\n", $PRP
      else
        printf "0x%08x allocated-zeroPRP\n", $PRP
      end
    end
    set $ADDR = (struct before_after *)($ADDR->next)
  end
  printf "pool_prp total linked list count=%d, allocated=%d, free=%d + used=%d\n", $CNT, pool_prp.num_allocated, pool_prp.num_free, pool_prp.num_allocated - pool_prp.num_free
  printf "FREECNT = %d  USEDCNT = %d\n", $FREECNT, $USEDCNT
end
#-----------------------------------------------------------------------------
# Following for either 0830 or 0840.
#-----------------------------------------------------------------------------
define print_d_exec_qu_ilts
    set $Q = (QU*)&d_exec_qu
    set $dILT = $Q->head
    while ($dILT !=0)
        i_find_ilt_base $dILT
        printf "0x%08x d_exec_quILT@0x%08x\n", $ILT, $dILT
        set $dILT = $dILT->fthd
    end
end
#...
document print_d_exec_qu_ilts
Prints out the d_exec_qu ilts.
end
#-----------------------------------------------------------------------------
define print_CMqu_ilts
    set $Icm = CM_cm_act_que
    while ($Icm !=0)
        set $QILT = ((QU*)&(((CM*)$Icm)->cmpltq))->head
        while ($QILT !=0)
            i_find_ilt_base $QILT
            printf "0x%08x cm_act_cmpltq@0x%08x\n", $ILT, $QILT
            set $QILT = $QILT->fthd
        end
        set $QILT = ((QU*)&(((CM*)$Icm)->ctlrqstq))->head
        while ($QILT !=0)
            i_find_ilt_base $QILT
            printf "0x%08x cm_act_ctlrqstq@0x%08x\n", $ILT, $QILT
            set $QILT = $QILT->fthd
        end
        set $QILT = ((QU*)&(((CM*)$Icm)->uderrq))->head
        while ($QILT !=0)
            i_find_ilt_base $QILT
            printf "0x%08x cm_act_uderrq@0x%08x\n", $ILT, $QILT
            set $QILT = $QILT->fthd
        end
        set $Icm = *((UINT32*)$Icm)
    end
#    set $Icor = (COR *)CM_cor_act_que
#    while ($Icor !=0)
#       set $Ccm = $Icor->cm
#       if ($Ccm != 0)
#            set $QILT = ((QU*)&(((CM*)$Ccm)->cmpltq))->head
#            while ($QILT !=0)
#                i_find_ilt_base $QILT
#                printf "0x%08x cm_cor_act_cmpltq@0x%08x\n", $ILT, $QILT
#                set $QILT = $QILT->fthd
#            end
#            set $QILT = ((QU*)&(((CM*)$Ccm)->ctlrqstq))->head
#            while ($QILT !=0)
#                i_find_ilt_base $QILT
#                printf "0x%08x cm_cor_act_ctlrqstq@0x%08x\n", $ILT, $QILT
#                set $QILT = $QILT->fthd
#            end
#            set $QILT = ((QU*)&(((CM*)$Ccm)->uderrq))->head
#            while ($QILT !=0)
#                i_find_ilt_base $QILT
#                printf "0x%08x cm_cor_act_uderrq@0x%08x\n", $ILT, $QILT
#                set $QILT = $QILT->fthd
#            end
#       end
#       set $Icor =$Icor->link
#    end
end
#...
document print_CMqu_ilts
Prints out the print_CMqueue ilts.
end
#-----------------------------------------------------------------------------
define print_C_exec_qht_ilts
    set $Q = (QU*)&C_exec_qht
    set $QILT = $Q->head
    if ($QILT != 0)
        while ($QILT !=0)
            i_find_ilt_base $QILT
            printf "0x%08x C_exec_qhtILT@0x%08x\n", $ILT, $QILT
            set $QILT = $QILT->fthd
        end
    end
end
#...
document print_C_exec_qht_ilts
Prints out the C_exec_qht ilts.
end
#-----------------------------------------------------------------------------
define print_linkqueues_newILTs
    set $I = 0
#    while ($I < 2)
      set $lILT = pMySharedMem->SHMLLI[$I].pLLActiveILTHead
      set $eILT = pMySharedMem->SHMLLI[$I].pLLActiveILTTail
      if ($lILT == $eILT)
        set $lILT = 0
      end
      while ((UINT32)$lILT != 0)
        i_find_ilt_base $lILT
        set $oILT = ($lILT-1)
        set $oVRP = ((UINT32*)$oILT)[8]
        set $nILT = $lILT->misc
        set $nVRP = $lILT->cr
        printf "%p linkqueue-newILT\n", $nILT
        if ($lILT == $eILT)
          set $lILT = 0
        else
          set $lILT = $lILT->fthd
        end
      end
#      set $I = $I + 1
#    end
end
#...
document print_linkqueues_newILTs
Prints out the linkqueue's new ilts (they point to other process ILTs).
end
#-----------------------------------------------------------------------------
define print_linkqueues_oldILTs
    set $I = 0
    while ($I < 2)
      set $lILT = pMySharedMem->SHMLLI[$I].pLLActiveILTHead
      set $eILT = pMySharedMem->SHMLLI[$I].pLLActiveILTTail
      if ($lILT == $eILT)
        set $lILT = 0
      end
      while ((UINT32)$lILT != 0)
        i_find_ilt_base $lILT
        set $oILT = ($lILT-1)
        set $oVRP = ((UINT32*)$oILT)[8]
        set $nILT = $lILT->misc
        set $nVRP = $lILT->cr
        printf "%p linkqueue-oldILT\n", $ILT
        if ($lILT == $eILT)
          set $lILT = 0
        else
          set $lILT = $lILT->fthd
        end
      end
      set $I = $I + 1
    end
end
#...
document print_linkqueues_oldILTs
Prints out the linkqueue's old ilts (they point to out process ILTs before they got sent).
end
#-----------------------------------------------------------------------------
# Internal macro
define print_linked_ilts
    set $dILT = (ILT*)$arg0
    while ($dILT !=0)
        i_find_ilt_base $dILT
        printf "0x%08x linked@0x%08x\n", $ILT, $dILT
        set $dILT = $dILT->fthd
    end
end
#-----------------------------------------------------------------------------
define print_iram_ilthead_ilts
  set $I = 0
  while ($I < 4)
    set $H = ((UINT32*)&ilthead)[$I*2]
    set $J = 1
    while ($H != 0 && $H != (UINT32)(((UINT32*)&ilttail + 2*$I)))
      i_find_ilt_base $H
      printf "%p ilthead[%d]#%d@%p\n", $ILT, $I, $J, $H
      set $W5 = (ILT*)((((ILT*)$H)-1)->ilt_normal.w5)
      set $W5CNT = 1
      if ($W5 != 0)
        i_find_ilt_base $W5
        printf "%p ilthead[%d]#%d-JOINED#%d@%p\n", $ILT, $I, $J, $W5CNT, $W5
        set $W5 = $W5->fthd
        set $W5CNT = 2
        while ($W5 != 0)
          i_find_ilt_base $W5
          printf "%p ilthead[%d]#%d-JOINED#%d@%p\n", $ILT, $I, $J, $W5CNT, $W5
          set $W5CNT = $W5CNT + 1
          set $W5 = $W5->fthd
        end
      end
      set $H = ((ILT*)$H)->fthd
      set $J = $J + 1
    end
    set $I = $I + 1
  end
end
#...
document print_iram_ilthead_ilts
Prints out the iram ilthead ilts.
end
#-----------------------------------------------------------------------------
define print_iram_ilttail_ilts
  set $I = 0
  while ($I < 4)
    set $H = ((UINT32*)&ilttail)[$I*2+1]
    set $J = 1
    while ($H != 0 && $H != (UINT32)(((UINT32*)&ilthead + 2*$I)))
      i_find_ilt_base $H
      printf "%p ilttail[%d]#%d@%p\n", $ILT, $I, $J, $H
      set $H = ((ILT*)$H)->bthd
      set $J = $J + 1
    end
    set $I = $I + 1
  end
end
#...
document print_iram_ilttail_ilts
Prints out the iram ilttail ilts.
end
#-----------------------------------------------------------------------------
define pbailt
  set $StartingAddress = $arg0
  printf "Address 0x%8.8x\n", $StartingAddress
  pilt $StartingAddress
  print_before_after $ILT
  printf "------------------------------------------------------------------------------\n"
end
#-----------------------------------------------------------------------------
# This macro prints out the ILT from base for all 7/11 layers, with possible VRP one back.
define pilt
print_nonzero_ilt_vrp $arg0
end
#.
document pilt
Another name for print_nonzero_ilt_vrp.
end
#-----------------------------------------------------------------------------
define print_nonzero_ilt_vrp
  set $P_ILT = $arg0
  i_find_ilt_base $P_ILT
  i_print_nonzero_ilt_vrp $ILT $P_ILT $cnt 0 0
end
#.
document print_nonzero_ilt_vrp
Macro to print all non-zero 7 (BE) or 11 (FE) layers of an ILT with possible VRP at location given.
end
#-----------------------------------------------------------------------------
define i_print_nonzero_ilt_vrp
  set $p_ILT = $arg0
  set $p_CILT = $arg1
  set $p_cnt = $arg2
  set $p_PILT = $arg3
  set $p_VRP = $arg4
  if ($p_cnt == $ILTNEST)
    printf "i_printilt_vrp - Could not calculate base of ILT\n"
  else
    set $i_cnt = 0
    while ($i_cnt < $ILTNEST)
      if ((UINT32)$p_PILT == 0 || (UINT32)$p_PILT == (UINT32)$p_ILT || $p_cnt == $i_cnt)
        if ($p_ILT == $p_CILT)
          printf "*"
        else
          printf " "
        end
        printf "ILT level %d at 0x%08x\n", $i_cnt, $p_ILT
        set $P = (UINT32*)$p_ILT
        if (*($P) != 0 || *($P + 1) != 0 || *($P + 2) != 0 || *($P + 3) != 0 || *($P + 4) != 0 || *($P + 5) != 0 || *($P + 6) != 0 || *($P + 7) != 0 || *($P + 8) != 0 || *($P + 9) != 0 || *($P + 10) != 0 || *($P + 11) != 0 || *($P + 12) != 0)
          printf "    fthd=0x%08x bthd=0x%08x misc=0x%08x linux_val=0x%08x\n", $p_ILT->fthd, $p_ILT->bthd, $p_ILT->misc, $p_ILT->linux_val
          printf "    w0=0x%08x   w1=0x%08x   w2=0x%08x   w3=0x%08x\n", ((UINT32*)$p_ILT)[4], ((UINT32*)$p_ILT)[5], ((UINT32*)$p_ILT)[6], ((UINT32*)$p_ILT)[7]
          printf "    w4=0x%08x   w5=0x%08x   w6=0x%08x   w7=0x%08x\n", ((UINT32*)$p_ILT)[8], ((UINT32*)$p_ILT)[9], ((UINT32*)$p_ILT)[10], ((UINT32*)$p_ILT)[11]
          printf "    cr=0x%08x -- ", $p_ILT->cr
          if ($p_ILT->cr != 0xcacacaca)
            set $cr = ((UINT32)($p_ILT->cr)&0x7fffffff)
            info symbol $cr
          else
            printf "\n"
          end
        end
        if ((UINT32)$p_PILT != 0 && $p_PILT == $p_ILT)
          # print out VRP at this level.
          printf "  VRP function=%d strategy=%d status=0x%2.2x vid=%d path=%d options=0x%2.2x\n", $p_VRP->function, $p_VRP->strategy, $p_VRP->status, $p_VRP->vid, $p_VRP->path, $p_VRP->options
          printf "      length=%d pktAddr=%p startDiskAddr=%lld pSGL=%p sglSize=%d\n", $p_VRP->length, $p_VRP->pktAddr, $p_VRP->startDiskAddr, $p_VRP->pSGL, $p_VRP->sglSize
          printf "      gen0=0x%08x gen1=0x%08x gen2=0x%08x gen3=0x%08x\n", $p_VRP->gen0, $p_VRP->gen1, $p_VRP->gen2, $p_VRP->gen3
        end
      end
      set $p_ILT = $p_ILT + 1
      set $i_cnt += 1
    end
  end
end
#...
document i_print_nonzero_ilt_vrp
Internal Macro to print all non-zero 7 (BE) or 11 (FE) layers of an ILT with possible VRP at location given.
end
#-----------------------------------------------------------------------------
define pcb_is_where
  set $BTP = (PCB*)$arg0
  set $K = (unsigned int*)$BTP->pc_pfp
  set $routine = (UINT32)$K[2]
  # if will return to "c" do the else.
  if ($routine != (UINT32)&ct_alldone)
    printf "routine @ 0x%x   ", $routine
    iw $routine
  else
    set $J = $BTP->pc_ebp
    set $routine = *((UINT32*)$J+1)
    iw $routine
  end
end
#...
document pcb_is_where
Print where the task will return to -- i960 code, presumes is not running now.
end
#-----------------------------------------------------------------------------
# traverse a list of device pointers
define walkdevs

    set $DEV = (DEV*)$arg0
    set $STARTDEV = (DEV*)$arg0
    set $DONE = 0
    printf "    Ndev          pDev         Port     NN                    LUN\n"
    printf "    ----------    ----------   ----     ------------------    ---\n"
    while ( $DONE != 1)
    
        printf "    %p    %p    %d       %016llX      %02X\n",$DEV->nDev,$DEV->pdev,$DEV->port,$DEV->nodeName,$DEV->lun
        set $DEV = $DEV->nDev
        if ($DEV ==  $STARTDEV )
            set $DONE = 1
        end
    end
end
#----
document walkdevs
traverse a circular list of device pointers through nDev
end
#-----------------------------------------------------------------------------
# This traverses the PCB list starting at the current pcb ptr looking
# for a specific value in the state.
#
define pcbs_are_where
  set $PCB = (PCB*)K_pcborg
  set $DONE = 0
  set $I = 0
  while ($DONE != 1)
    set $I = $I + 1
    if ($PCB == (PCB*)K_xpcb)
      printf "*"
    else
      printf " "
    end
    printf "%2.2d PCB = 0x%08x,  ", $I, $PCB
    printf "Name = %32s,  ", ((PCB*)($PCB))->pc_fork_name
    set $STATE = ((PCB*)($PCB))->pc_stat
    printf "State = 0x%02x ", $STATE
    i_printpcbstatetext $STATE
    pcb_is_where $PCB
    set $PCB = ((PCB*)($PCB))->pc_thd
    if ($PCB == (PCB*)K_pcborg || $PCB == 0)
      set $DONE = 1
      if ($PCB == 0)
        printf "ERROR, next PCB value is zero\n"
      end
    end
  end
end
#...
document pcbs_are_where
Print the state of the PCB queues in the PROC and where it will return to executing.
end
#-----------------------------------------------------------------------------
# $pILT->w1 = pUINT32[5]
# $pILT->w4 = $pUINT32[8] (or vrvrp, $VRP here)
define i_vcdm4_printrb
  if ($arg0 == 0)
    printf " "
  else
    printf "F"
  end
  printf "%3d %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %3u %8.8x %8.8x %7.7llx %8llu %3d\n", $VID, $pRB, $pRB->cLeft, $pRB->cRight, $pRB->fthd, $pILT, $pILT->misc, $pILT->cr, $pUINT32[5], $pVRP, $pVRP->status, $pVRP->startDiskAddr, $pVRP->startDiskAddr, $pVRP->length
end

# This macro prints the vcd cache IO trees as m4 wants it displayed.
define vcdm4
  set $VID = 0
    printf "VCD Cache Information for all virtual disks with IO trees:\n"
  printf " VID  RB*     LEFT     RIGHT    FTHD     ILT*     MISC     CR       BIO VRP      STATUS   SDA hex  SDA dec LEN\n"
  printf " --- -------- -------- -------- -------- -------- -------- -------- --- -------- -------- ------- -------- ---\n"
  set $count = 0
  set $blocked_count = 0
  set $fwd_not_blocked_count = 0
  set $fwd = 0
  while ($VID < $MAXVIRTUALS)
    set $pVCD = ((VCD**)&vcdIndex)[$VID]
    if ($pVCD > 0)
      set $pRB = (struct RB*)$pVCD->pIO
      set $pRBPrev = &nil
      if ($pRB == 0)
        set $pRB = &nil
      end
      while ($pRB != &nil)
        set $count = $count + 1
        set $pILT = (ILT*)$pRB->dPoint
        set $pUINT32 = (UINT32*)$pRB->dPoint
        set $pVRP = (VRP*)($pUINT32[8])
        i_vcdm4_printrb 0
        set $blocked_count = $blocked_count + $pUINT32[5]
        # Print any forward threads.
        if ($pRB->fthd != 0)
          set $saveRB = $pRB
          set $pRB = $saveRB->fthd
          while ($pRB != 0)
            set $fwd = $fwd + 1
            set $count = $count + 1
            set $pILT = (ILT*)$pRB->dPoint
            set $pUINT32 = (UINT32*)$pRB->dPoint
            set $pVRP = (VRP*)($pUINT32[8])
            i_vcdm4_printrb 1
            set $blocked_count = $blocked_count + $pUINT32[5]
            if ($pUINT32[5] != 1)
              set $fwd_not_blocked_count = $fwd_not_blocked_count + 1
            end
            set $pRB = $pRB->fthd
          end
          # restore where we were.
          set $pRB = $saveRB
        end
        # End of forward thread printing.
        if ($pRB->bParent == 0 && $pRB->cLeft == &nil && $pRB->cRight == &nil)
          set $pRB = &nil
        else
          if ($pRB->cLeft != &nil)
            set $pRB = $pRB->cLeft
          else
            if ($pRB->cRight != &nil)
              set $pRB = $pRB->cRight
            else
              set $pRBPrev = $pRB
              set $pRB = $pRB->bParent
              while ($pRBPrev != &nil)
                if ($pRBPrev == $pRB->cLeft)
                  if ($pRB->cRight != &nil)
                    set $pRBPrev = &nil
                    set $pRB = $pRB->cRight
                  else
                    if ($pRB->bParent == 0)
                      set $pRBPrev = &nil
                      set $pRB = &nil
                    else
                      set $pRBPrev = $pRB
                      set $pRB = $pRB->bParent
                    end
                  end
                else
                  if ($pRBPrev == $pRB->cRight)
                    if ($pRB->bParent == 0)
                      set $pRBPrev = &nil
                      set $pRB = &nil
                    else
                      set $pRBPrev = $pRB
                      set $pRB = $pRB->bParent
                    end
                  end
                end
              end
            end
          end
        end
      end
    end
    set $VID = $VID + 1
  end
  printf "\n"
  printf "Total Count: %d    Blocked IO Count=%d    C_orc=%d   fthds=%d  Fthd_not_blocked=%d\n", $count, $blocked_count, C_orc, $fwd, $fwd_not_blocked_count
end
#...
document vcdm4
Print the vcd cache sda information for a given vcd (argument VID).
    VID - Virtual disk identifier
    0xFFFF for doing all of them.
end
#-----------------------------------------------------------------------------
# This prints interesting PCBs in the FE or BE.
#
define m4pcbstate
  set $PCB = (PCB*)K_pcborg
  set $DONE = 0
  set $I = 0
  while ($DONE != 1)
    set $I = $I + 1
    set $STATE = $PCB->pc_stat
# Not "WAIT IO", "Not ready", "ISP wait", "Wait linux comp in", "Wait linux targ in", "FILE SYS CLEANUP"
    if ($STATE != 0x0e && $STATE != 0x01 && $STATE != 0x18 && $STATE != 0x34 && $STATE != 0x33 && $STATE != 0x2e)
      if ($PCB == (PCB*)K_xpcb)
        printf "*"
      else
        printf " "
      end
      printf "%2.2d PCB = 0x%08x,  ", $I, $PCB
      printf "Name = %32s,  ", $PCB->pc_fork_name
      printf "State = 0x%02x ", $STATE
      i_printpcbstatetext $STATE
    end
    set $PCB = (PCB*)$PCB->pc_thd
    if ($PCB == (PCB*)K_pcborg || $PCB == 0)
      set $DONE = 1
      if ($PCB == 0)
        printf "ERROR, next PCB value is zero\n"
      end
    end
  end
end
#...
document m4pcbstate
Print interesting PCB queues in the PROC.
end
#-----------------------------------------------------------------------------
define ddrtable
  if (((UINT32)(&K_ii) & 0xffff0000) == $FE_MEMORY_STARTS)
    set $DDR = (UINT32 *)$FE_MEMORY_STARTS
  else
    set $DDR = (UINT32 *)startOfBESharedMem
  end
  set $DDR = (UINT32 *)((char *)$DDR + 0x1000)

  printf "DDR_TBL_HDR={id=%4.4s, version=%d, length=%d (0x%x), crc=0x%x}\n", &$DDR[0], $DDR[1], $DDR[2], $DDR[2], $DDR[3]

  set $COUNT = $DDR[2]
  set $I = 0

  # Start of first ddr entry.
  set $DDR = $DDR + 4

  while ($I < $COUNT)
    printf "%3d DDR_TABLE_ENTRY={id=%8.8s, address=%p, length=%d (0x%x)\n", $I, &$DDR[0], (void *)$DDR[2], $DDR[3], $DDR[3]
    set $I = $I + 1
    set $DDR = $DDR + 4
  end
end
#...
document ddrtable
Print out the ddrtable header, and the entry's addresses and length.
end
#=============================================================================
define print_pool_allocated
  set $POOL = $arg0
  set $SIZE = ($arg1 + (64-1)) & ~(64-1)
  set $ADDR = (struct before_after *)$POOL.first_allocated
  set $CNT = 0
  set $FREECNT = 0
  set $USEDCNT = 0
  while ((unsigned long)$ADDR != 0 && (unsigned long)$ADDR != 0xdcdcdcdc)
    set $CNT = $CNT + 1
    set $A = (unsigned long)$ADDR - $SIZE
    set $PRE = (struct before_after *)($A - sizeof(struct before_after))
    if ($PRE->used_or_free == 0)
      set $FREECNT = $FREECNT + 1
      printf "0x%08x free-list\n", $A
    else
      set $USEDCNT = $USEDCNT + 1
      set $D = $SIZE/8
      set $LEFT = ($SIZE - ($D * 8)) / 4
      set $Z = 0
      set $I = (UINT64*)$A
      while ($D > 0 && $Z == 0)
        if (*$I != 0)
          set $Z = 1
        end
        set $D = $D - 1
        set $I = $I + 1
      end
      set $I = (UINT32*)$I
      while ($LEFT > 0 && $Z == 0)
        if (*$I != 0)
          set $Z = 1
        end
        set $LEFT = $LEFT - 1
        set $I = $I + 1
      end
      if ($Z != 0)
        printf "0x%08x allocated-non-zero\n", $A
      else
        printf "0x%08x allocated-zero\n", $A
      end
    end
    set $ADDR = (struct before_after *)($ADDR->next)
  end
  printf "pool total linked list count=%d, allocated=%d, free=%d + used=%d\n", $CNT, $POOL.num_allocated, $POOL.num_free, $POOL.num_allocated - $POOL.num_free
  printf "FREECNT = %d  USEDCNT = %d\n", $FREECNT, $USEDCNT
end
#=============================================================================
define find_next_nonzero
  set $ADDR = (UINT32 *)$arg0
  set $CNT = 0
  set $NUMBER = 0
  while ($NUMBER < 64)
    if (*$ADDR != 0)
      printf "Nonzero address at %p -- bytes after = %d (0x%x)\n", $ADDR, $CNT, $CNT
      set $NUMBER = $NUMBER + 1
      set $ADDR = $ADDR + 1
      set $CNT = $CNT + 4
    else
      set $CNT = $CNT + 4
      set $ADDR = $ADDR + 1
    end
  end
end
#=============================================================================
define find_previous_nonzero
  set $ADDR = (UINT32 *)$arg0
  set $CNT = 0
  while ($ADDR != 0)
    if (*$ADDR != 0)
      printf "Nonzero address at %p -- bytes before = %d (0x%x)\n", $ADDR, $CNT, $CNT
      set $ADDR = 0
    else
      set $CNT = $CNT + 4
      set $ADDR = $ADDR - 1
    end
  end
end
#-----------------------------------------------------------------------------
define iw
  if ($arg0 != 0)
    if ((UINT32)$arg0 >= $EXECUTABLE_START && (UINT32)$arg0 <= $LIBRARY_STARTS)
      info symbol $arg0
      printf "          "
    end
    set listsize 0
    list *$arg0
    set listsize 10
  else
    printf "0x%08x\n", $arg0
  end 
end
#...
document iw
Do a list on the argument passed in.
end
#-----------------------------------------------------------------------------
define i
  info symbol $arg0
  iw $arg0
end
#...
document i
Do an info symbol and list on the argument passed in.
end
#=============================================================================
# This macro prints the vdisks information given the specified format.
define verify_vdisks
    printf "\n"
    i_verify_raids
    i_verify_pdisks
    set $vdisk = 0
    while ($vdisk < $MAXVIRTUALS)
      if ((*(VDX*)&gVDX).vdd[$vdisk] > 0)
        set $raidnum=gVDX.vdd[$vdisk].raidCnt
        printf "VID %4u st 0x%02x size %14llu mir 0x%2.2x attr 0x%4.4x own %d cp%% %3d name %16.16s\n", gVDX.vdd[$vdisk].vid, gVDX.vdd[$vdisk].status, gVDX.vdd[$vdisk].devCap, gVDX.vdd[$vdisk].mirror, gVDX.vdd[$vdisk].attr, gVDX.vdd[$vdisk].owner, gVDX.vdd[$vdisk].scpComp, gVDX.vdd[$vdisk].name
        set $J = 0
        set $RDD = gVDX.vdd[$vdisk]->pRDD
        while ($RDD > 0)
          i_verify_raid $RDD->rid
          set $J = $J + 1
          set $RDD = $RDD->pNRDD
        end
        if ($raidnum != $J)
          printf "ERROR, raidCnt (%d) != pointer list (%d)\n", $raidnum, $J
        end
      end
      set $vdisk = $vdisk + 1
    end
end
#...
document verify_vdisks
This macro is used to verify that vdisk and raid and PSD structure is all OK.
end
#-----------------------------------------------------------------------------
# This macro prints the raid information for a given raid.
define i_verify_raid
    set $raid=$arg0
    if ((*(RDX*)&gRDX).rdd[$raid] > 0)
      set $pRDD = (RDD*)gRDX.rdd[$raid]
      printf "  RID %4u VID %4u Type %1d Status 0x%02x aStatus 0x%02x size %14llu\n", gRDX.rdd[$raid].rid, gRDX.rdd[$raid].vid, gRDX.rdd[$raid].type, gRDX.rdd[$raid].status, gRDX.rdd[$raid].aStatus,gRDX.rdd[$raid].devCap
      set $pPSD = $pRDD->extension.pPSD
      set $I = 0
      while ($I < $pRDD->psdCnt)
        printf "    PSD %3u RID %3u Status 0x%2.2x aStatus 0x%2.2x sda %10d lth %10d\n", $pPSD->pid ,$pPSD->rid, $pPSD->status, $pPSD->aStatus, $pPSD->sda ,$pPSD->sLen
        set $pPSD = $pPSD->npsd
        set $I = $I + 1
      end
    else
      printf " NOT AVAILABLE\n"
    end
end
#-----------------------------------------------------------------------------
# Go through raid table and print one line for each.
define i_verify_raids
    set $I = 0
    while ($I < $MAXRAIDS)
      if ((*(RDX*)&gRDX).rdd[$I] > 0)
        printf "RAID %4u %4u size %14llu\n", $I, gRDX.rdd[$I].rid, gRDX.rdd[$I].devCap
      end
      set $I = $I + 1
    end
end
#-----------------------------------------------------------------------------
# This macro prints the pdisks information given the specified format.
define i_verify_pdisks
    set $I = 0
    while ($I < $MAXDRIVES)
      if ((*(PDX*)&gPDX).pdd[$I] > 0)
        printf "PID %3d bay %02.2d-%02.2d type %d cl 0x%02x devstat 0x%02x size %10lld pctRem %2d rbRemain %10d %c%c %02d-%02d\n", gPDX.pdd[$I].pid, gPDX.pdd[$I].ses, gPDX.pdd[$I].slot, gPDX.pdd[$I].devType, gPDX.pdd[$I].devClass, gPDX.pdd[$I].devStat, gPDX.pdd[$I].devCap, gPDX.pdd[$I].pctRem, gPDX.pdd[$I].rbRemain, gPDX.pdd[$I].devName[0], gPDX.pdd[$I].devName[1], gPDX.pdd[$I].devName[2], gPDX.pdd[$I].devName[3]
      end
      set $I = $I + 1
    end
end
#-----------------------------------------------------------------------------
define calclocalimagesize
    set $size = sizeof(NVRL) + sizeof(NVRH)

    set $index_n = 0
    while ($index_n < $MAXRAIDS)
        set $rdd = R_rddindx[$index_n]
        if ($rdd != 0) 
            set $size = $size + sizeof(NVRH) + sizeof(NVRLRDD2)
            set $size = $size + ($rdd->psdCnt * sizeof(NVRLXPSD))
        end
        set $index_n = $index_n + 1
    end

    set $size = $size + sizeof(NVRH)
    set $index_n = 0 
    while ($index_n < $MAXDRIVES) 
        if (P_pddindx[$index_n] != 0)    
            set $size = $size + sizeof(NVRLXPDD)
        end
        set $index_n = $index_n + 1
    end
    set $size = ($size + 15) & 0xFFFFFFF0

    printf "size calculated = 0x%x (%d)\n", $size, $size
end
#-----------------------------------------------------------------------------
# This macro prints out the servers and all vid/lun they are attached.
define servers
    set $S = (SDD**)&S_sddindx
    set $I = 0
    while ($I < $MAXSERVERS)
      set $SDD = $S[$I]
      if ($SDD != 0)
        printf "server %d (tid=%d reqCnt=%d owner=%d wwn=%llx name=%s i_name=%s)\n", $SDD->sid, $SDD->tid, $SDD->reqCnt, $SDD->owner, $SDD->wwn, $SDD->name, $SDD->i_name
	if ($SDD->lvm != 0)
          printf "  vid/lun:"
	end
        set $LVM = $SDD->lvm
        while ($LVM != 0)
          printf " %d/%d", $LVM->vid, $LVM->lun
          set $LVM = $LVM->nlvm
        end
	if ($SDD->lvm != 0)
          printf "\n"
	end
      end
      set $I += 1
    end
end
#...
document servers
Print out the servers and all vid/lun they are attached.
end
#-----------------------------------------------------------------------------
#=============================================================================
# Do not attempt to step into routines without source code.
set step-mode off
# Deactivate "more" on long prints.
set pagination off
# Allow potentially dangerous operations (like quit) without a "y".
set confirm no
# Set output radix to be base 16 by default.
set output-radix 16
# Set print pretty on
pp on
#-----------------------------------------------------------------------------
handle SIGHUP nostop noprint pass
handle SIGWINCH nostop noprint ignore
# handle SIGHUP stop print nopass
handle SIGINT stop print nopass
handle SIGQUIT stop print nopass
handle SIGILL stop print nopass
handle SIGABRT stop print nopass
handle SIGBUS stop print nopass
handle SIGFPE stop print nopass
handle SIGUSR1 stop print nopass
handle SIGSEGV stop print nopass
# handle SIGUSR2 stop print nopass
handle SIGPIPE stop print nopass
# handle SIGALRM stop print nopass
handle SIGTERM stop print nopass
# handle SIGSTKFLT stop print nopass
handle SIGCHLD stop print nopass
handle SIGCONT stop print nopass
handle SIGSTOP stop print nopass
handle SIGTSTP stop print nopass
handle SIGTTIN stop print nopass
handle SIGTTOU stop print nopass
handle SIGURG stop print nopass
handle SIGXCPU stop print nopass
handle SIGXFSZ stop print nopass
handle SIGVTALRM stop print nopass
handle SIGPROF nostop noprint pass
handle SIGIO stop print nopass
handle SIGPWR stop print nopass
handle SIGSYS stop print nopass
# handle SIGKILL stop print nopass
# handle SIGEMT stop print nopass
handle SIGLOST stop print nopass
handle SIGPOLL stop print nopass
handle SIGWIND stop print nopass
handle SIGPHONE stop print nopass
handle SIGWAITING stop print nopass
handle SIGLWP stop print nopass
handle SIGDANGER stop print nopass
handle SIGGRANT stop print nopass
handle SIGRETRACT stop print nopass
handle SIGMSG stop print nopass
handle SIGSOUND stop print nopass
handle SIGSAK stop print nopass
handle SIGPRIO stop print nopass
handle SIGCANCEL stop print nopass
handle SIGINFO stop print nopass
handle SIG32 stop print nopass
handle SIG33 stop print nopass
handle SIG34 stop print nopass
handle SIG35 stop print nopass
handle SIG36 stop print nopass
handle SIG37 stop print nopass
handle SIG38 stop print nopass
handle SIG39 stop print nopass
handle SIG40 stop print nopass
handle SIG41 stop print nopass
handle SIG42 stop print nopass
handle SIG43 stop print nopass
handle SIG44 stop print nopass
handle SIG45 stop print nopass
handle SIG46 stop print nopass
handle SIG47 stop print nopass
handle SIG48 stop print nopass
handle SIG49 stop print nopass
handle SIG50 stop print nopass
handle SIG51 stop print nopass
handle SIG52 stop print nopass
handle SIG53 stop print nopass
handle SIG54 stop print nopass
handle SIG55 stop print nopass
handle SIG56 stop print nopass
handle SIG57 stop print nopass
handle SIG58 stop print nopass
handle SIG59 stop print nopass
handle SIG60 stop print nopass
handle SIG61 stop print nopass
handle SIG62 stop print nopass
handle SIG63 stop print nopass
handle SIG64 stop print nopass
handle SIG65 stop print nopass
handle SIG66 stop print nopass
handle SIG67 stop print nopass
handle SIG68 stop print nopass
handle SIG69 stop print nopass
handle SIG70 stop print nopass
handle SIG71 stop print nopass
handle SIG72 stop print nopass
handle SIG73 stop print nopass
handle SIG74 stop print nopass
handle SIG75 stop print nopass
handle SIG76 stop print nopass
handle SIG77 stop print nopass
handle SIG78 stop print nopass
handle SIG79 stop print nopass
handle SIG80 stop print nopass
handle SIG81 stop print nopass
handle SIG82 stop print nopass
handle SIG83 stop print nopass
handle SIG84 stop print nopass
handle SIG85 stop print nopass
handle SIG86 stop print nopass
handle SIG87 stop print nopass
handle SIG88 stop print nopass
handle SIG89 stop print nopass
handle SIG90 stop print nopass
handle SIG91 stop print nopass
handle SIG92 stop print nopass
handle SIG93 stop print nopass
handle SIG94 stop print nopass
handle SIG95 stop print nopass
handle SIG96 stop print nopass
handle SIG97 stop print nopass
handle SIG98 stop print nopass
handle SIG99 stop print nopass
handle SIG100 stop print nopass
handle SIG101 stop print nopass
handle SIG102 stop print nopass
handle SIG103 stop print nopass
handle SIG104 stop print nopass
handle SIG105 stop print nopass
handle SIG106 stop print nopass
handle SIG107 stop print nopass
handle SIG108 stop print nopass
handle SIG109 stop print nopass
handle SIG110 stop print nopass
handle SIG111 stop print nopass
handle SIG112 stop print nopass
handle SIG113 stop print nopass
handle SIG114 stop print nopass
handle SIG115 stop print nopass
handle SIG116 stop print nopass
handle SIG117 stop print nopass
handle SIG118 stop print nopass
handle SIG119 stop print nopass
handle SIG120 stop print nopass
handle SIG121 stop print nopass
handle SIG122 stop print nopass
handle SIG123 stop print nopass
handle SIG124 stop print nopass
handle SIG125 stop print nopass
handle SIG126 stop print nopass
handle SIG127 stop print nopass
handle EXC_BAD_ACCESS stop print nopass
handle EXC_BAD_INSTRUCTION stop print nopass
handle EXC_ARITHMETIC stop print nopass
handle EXC_EMULATION stop print nopass
handle EXC_SOFTWARE stop print nopass
handle EXC_BREAKPOINT stop print nopass
#=============================================================================
define exit
  quit
end
#=============================================================================
####
## Modelines:
## Local Variables:
## tab-width: 4
## indent-tabs-mode: nil
## End:
## vi:sw=4 ts=4 expandtab
# End of file .gdbinit
