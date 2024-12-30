# Start of file .gdbinit in CCB/Src
#-----------------------------------------------------------------------------
# Deactivate "more" on long prints.
set pagination off
# Allow potentially dangerous operations (like quit) without a "y".
set confirm no
# Do not attempt to step into routines without source code.
set step-mode off
# Set output radix to be base 16 by default.
set output-radix 16
#-----------------------------------------------------------------------------
set history filename ~/.gdb_history
set history save on
set history size 256
#-----------------------------------------------------------------------------
set $FE_MEMORY_STARTS = 0x48000000
set $EXECUTABLE_START = (UINT32)&__executable_start
#-----------------------------------------------------------------------------
# The "pp" command sets the print pretty option without typing it out.
define pp
    set print pretty $arg0
end
#----
document pp
The "pp" command will set the print pretty option.
    "ON"    - Turn the print pretty ON
    "OFF"   - Turn the print pretty OFF
end
#-----------------------------------------------------------------------------
# This prints out the current processes "name".
define forkname
  set $PCB = K_xpcb
  set $XK_PCB = XK_pcb
  if (K_xpcb == 0 || $XK_PCB == 0)
    printf "K_xpcb=%p\n", K_xpcb
    printf "XK_pcb=%p\n", XK_pcb
  else
    printf "K_xpcb->pc_fork_name=%p '%.32s'\n", K_xpcb, K_xpcb->pc_fork_name
    if (XK_pcb->functionPtr != 0)
      printf "XK_pcb->functionPtr="
      iw XK_pcb->functionPtr
    else
      printf "XK_pcb->functionPtr=%p\n", XK_pcb->functionPtr
    end
  end
end
#...
document forkname
Print the names of the task that is (or last was) running.
end
#-----------------------------------------------------------------------------
# The "mutexes" command prints out the CCB mutexex.
define mutexes
  set $MUTEX = 0
  while ($MUTEX < 100)
    if (xkMutexList[$MUTEX].used != 0)
      printf "xkMutexList[%d]",$MUTEX
      printf " init@%s:%u", xkMutexList[$MUTEX].init_file, xkMutexList[$MUTEX].init_line,
      if (xkMutexList[$MUTEX].lock_file != 0)
        printf " locked@%s:%u", xkMutexList[$MUTEX].lock_file, xkMutexList[$MUTEX].lock_line,
      else
        printf " not locked,"
      end
      if (xkMutexList[$MUTEX].lock_func != 0)
        printf " pcb@%p, func=", xkMutexList[$MUTEX].lock_xk_pcb
        iw xkMutexList[$MUTEX].lock_func
      else
        printf " pcb=%p\n", xkMutexList[$MUTEX].lock_xk_pcb
      end
      printf " mutex->{count=%d, kind=%d, owner=%p lock={status=%ld, spinlock=%d}}\n", xkMutexList[$MUTEX].mutex->__m_count, xkMutexList[$MUTEX].mutex->__m_kind, xkMutexList[$MUTEX].mutex->__m_owner, xkMutexList[$MUTEX].mutex->__m_lock.__status, xkMutexList[$MUTEX].mutex->__m_lock.__spinlock
    end
    set $MUTEX=$MUTEX+1
  end
  printf "------------------------------------------------------------------------------\n"
  printf "XK_KERNEL_ACCESS=0\n"
  printf "logMutex=0x%x (%d)\n",logMutex,logMutex
  printf "fileIOMutex=0x%x (%d)\n",fileIOMutex,fileIOMutex
  printf "fileSystemMutex=0x%x (%d)\n",fileSystemMutex,fileSystemMutex
  printf "bigBufferMutex=0x%x (%d)\n",bigBufferMutex,bigBufferMutex
  printf "configUpdateMutex=0x%x (%d)\n",configUpdateMutex,configUpdateMutex
  printf "sendMutex=0x%x (%d)\n",sendMutex,sendMutex
  printf "configJournalMutex=0x%x (%d)\n",configJournalMutex,configJournalMutex
  printf "backtraceMutex=0x%x (%d)\n",backtraceMutex,backtraceMutex
  printf "SM_mpMutex=0x%x (%d)\n",SM_mpMutex,SM_mpMutex
  printf "gPICommandMutex=0x%x (%d)\n",gPICommandMutex,gPICommandMutex
  printf "gClientsMutex=0x%x (%d)\n",gClientsMutex,gClientsMutex
  printf "gMgtListMutex=0x%x (%d)\n",gMgtListMutex,gMgtListMutex
  printf "sessionListMutex=0x%x (%d)\n",sessionListMutex,sessionListMutex
  printf "packetQueueMutex=0x%x (%d)\n",packetQueueMutex,packetQueueMutex
  printf "outStandingMutex=0x%x (%d)\n",outStandingMutex,outStandingMutex
  printf "inProgressMutex=0x%x (%d)\n",inProgressMutex,inProgressMutex
  printf "sequenceNumberMutex=0x%x (%d)\n",sequenceNumberMutex,sequenceNumberMutex
  printf "historyMutex=0x%x (%d)\n",historyMutex,historyMutex
  printf "runningTasksMutex=0x%x (%d)\n",runningTasksMutex,runningTasksMutex
  printf "counterMap.header.busyMutex=0x%x (%d)\n",counterMap.header.busyMutex,counterMap.header.busyMutex
  printf "userPortTransmitBuffer.header.bufferEnqueueMutex=0x%x (%d)\n",userPortTransmitBuffer.header.bufferEnqueueMutex,userPortTransmitBuffer.header.bufferEnqueueMutex
  printf "userPortTransmitBuffer.header.bufferDequeueMutex=0x%x (%d)\n",userPortTransmitBuffer.header.bufferDequeueMutex,userPortTransmitBuffer.header.bufferDequeueMutex
  printf "userPortReceiveBuffer.header.bufferEnqueueMutex=0x%x (%d)\n",userPortReceiveBuffer.header.bufferEnqueueMutex,userPortReceiveBuffer.header.bufferEnqueueMutex
  printf "userPortReceiveBuffer.header.bufferDequeueMutex=0x%x (%d)\n",userPortReceiveBuffer.header.bufferDequeueMutex,userPortReceiveBuffer.header.bufferDequeueMutex
  printf "hwm_data.lock=0x%x (%d)\n",hwm_data.lock,hwm_data.lock
  printf "sesMutex=0x%x (%d)\n",sesMutex,sesMutex
end
#----
document mutexes
The "mutexes" command prints out the CCB mutexex.
end
#-----------------------------------------------------------------------------
# The "argh" command continues trying to print out the CCB threads.
define argh
  while ($THREAD > 0)
    set $LAST=$THREAD
    printf "backtrace 100 full for thread %d\n",$LAST
    set $THREAD=$THREAD-1
    thread $LAST
    bt 100 full
  end
end
#----
document argh
The "argh" command continues trying to print out all ccb threads backtrace. See startargh.
end
#-----------------------------------------------------------------------------
# The "startargh" command tries to print out all ccb threads backtrace.
# There are 1 to 204 threads (2007-12-13).
define startargh
  set $THREAD=205
  argh
end
#----
document startargh
The "startargh" command tries to print out all ccb threads backtrace. See argh.
end
#-----------------------------------------------------------------------------
# The "cbt" command prints out all ccb threads backtrace.
# There are 1 to 204 threads (2007-12-13).
define cbt
  thread apply all bt 16
end
#----
document cbt
The "cbt" command prints out all ccb threads backtrace.
end
#-----------------------------------------------------------------------------
# The "cbtfull" command prints out all ccb threads backtrace with info local.
# There are 1 to 204 threads (2007-12-13).
define cbtfull
  thread apply all bt 40 full
end
#----
document cbtfull
The "cbtfull" command prints out all ccb threads backtrace with info local.
end
#-----------------------------------------------------------------------------
# The "runqueue" command prints the ccb threads in the run queue.
define runqueue
  set $PCB = xkRunQueue.tskPtr
  set $I = 1
  printf "There are %d (0x%x) threads on the run queue\n",xkRunQueue.taskCount,xkRunQueue.taskCount
  set $DONE = 0
  while ($PCB != 0 && $DONE == 0 && $I <= xkRunQueue.taskCount)
    set $ST = $PCB->pcb.pc_stat
    printf "%3d pcb=%p, threadId=%p, pc_stat=0x%x (", $I,$PCB,$PCB->threadId,$ST
    if ($ST <= 0x3C)
      if ($ST == 0x00)
        printf "PCB_READY"
      end
      if ($ST == 0x01)
        printf "PCB_NOT_READY"
      end
      if ($ST == 0x02)
        printf "PCB_WAIT_SRAM"
      end
      if ($ST == 0x03)
        printf "PCB_WAIT_CACHEABLE"
      end
      if ($ST == 0x04)
        printf "PCB_WAIT_NON_CACHEABLE"
      end
      if ($ST == 0x05)
        printf "PCB_WAIT_REMOTE"
      end
      if ($ST == 0x06)
        printf "PCB_TIMED_WAIT"
      end
      if ($ST == 0x07)
        printf "PCB_21MS_TIMER_WAIT"
      end
      if ($ST == 0x08)
        printf "PCB_WAIT_MSG_0"
      end
      if ($ST == 0x09)
        printf "PCB_WAIT_MSG_1"
      end
      if ($ST == 0x0A)
        printf "PCB_WAIT_DOOR_BELL_0"
      end
      if ($ST == 0x0B)
        printf "PCB_WAIT_DOOR_BELL_1"
      end
      if ($ST == 0x0C)
        printf "PCB_WAIT_DOOR_BELL_2"
      end
      if ($ST == 0x0D)
        printf "PCB_WAIT_DOOR_BELL_3"
      end
      if ($ST == 0x0E)
        printf "PCB_WAIT_IO"
      end
      if ($ST == 0x0F)
        printf "PCB_WAIT_NVA"
      end
      if ($ST == 0x10)
        printf "PCB_HOST_RESET_WAIT"
      end
      if ($ST == 0x11)
        printf "PCB_WAIT_SEM_1"
      end
      if ($ST == 0x12)
        printf "PCB_WAIT_SEM_2"
      end
      if ($ST == 0x13)
        printf "PCB_SCSI_RESET_WAIT"
      end
      if ($ST == 0x14)
        printf "PCB_WAIT_BLK_LOCK"
      end
      if ($ST == 0x15)
        printf "PCB_WAIT_COPY_LOCK"
      end
      if ($ST == 0x16)
        printf "PCB_WAIT_DMA_0"
      end
      if ($ST == 0x17)
        printf "PCB_WAIT_DMA_1"
      end
      if ($ST == 0x18)
        printf "PCB_ISP_WAIT"
      end
      if ($ST == 0x19)
        printf "PCB_WAIT_NON_CACHE_WBUF"
      end
      if ($ST == 0x1A)
        printf "PCB_WAIT_CACHE_WBUF"
      end
      if ($ST == 0x1B)
        printf "PCB_WAIT_FE_BE_MRP"
      end
      if ($ST == 0x1C)
        printf "PCB_FC_READY_WAIT"
      end
      if ($ST == 0x1D)
        printf "PCB_ONLINE_WAIT"
      end
      if ($ST == 0x1E)
        printf "PCB_WAIT_WRT_CACHE"
      end
      if ($ST == 0x1F)
        printf "PCB_WAIT_LINK 0x1F"
      end
      if ($ST == 0x20)
        printf "PCB_WAIT_LINK 0x20"
      end
      if ($ST == 0x21)
        printf "PCB_WAIT_LINK 0x21"
      end
      if ($ST == 0x22)
        printf "PCB_WAIT_LINK 0x22"
      end
      if ($ST == 0x23)
        printf "PCB_WAIT_LINK 0x23"
      end
      if ($ST == 0x24)
        printf "PCB_WAIT_LINK 0x24"
      end
      if ($ST == 0x25)
        printf "PCB_WAIT_LINK 0x25"
      end
      if ($ST == 0x26)
        printf "PCB_WAIT_LINK 0x26"
      end
      if ($ST == 0x27)
        printf "PCB_QLOGIC_WAIT 0x27"
      end
      if ($ST == 0x28)
        printf "PCB_QLOGIC_WAIT 0x28"
      end
      if ($ST == 0x29)
        printf "PCB_QLOGIC_WAIT 0x29"
      end
      if ($ST == 0x2A)
        printf "PCB_QLOGIC_WAIT 0x2A"
      end
      if ($ST == 0x2B)
        printf "PCB_WAIT_MRP"
      end
      if ($ST == 0x2C)
        printf "PCB_WAIT_SYNC_NVA"
      end
      if ($ST == 0x2D)
        printf "PCB_WAIT_RAID_ERROR"
      end
      if ($ST == 0x2E)
        printf "PCB_FILE_SYS_CLEANUP"
      end
      if ($ST == 0x2F)
        printf "PCB_WAIT_RAID_INIT"
      end
      if ($ST == 0x30)
        printf "PCB_WAIT_I82559"
      end
      if ($ST == 0x31)
        printf "PCB_IPC_WAIT"
      end
      if ($ST == 0x32)
        printf "PCB_EVENT_WAIT"
      end
      if ($ST == 0x33)
        printf "PCB_WAIT_LINUX_TARG_IN"
      end
      if ($ST == 0x34)
        printf "PCB_WAIT_LINUX_COMP_IN"
      end
      if ($ST == 0x35)
        printf "PCB_WAIT_P2_NVRAM"
      end
      if ($ST == 0x36)
        printf "PCB_MM_WAIT"
      end
      if ($ST == 0x37)
        printf "PCB_NV_DMA_QFULL_WAIT"
      end
      if ($ST == 0x38)
        printf "PCB_NV_DMA_EXEC_WAIT"
      end
      if ($ST == 0x39)
        printf "PCB_NV_DMA_COMP_WAIT"
      end
      if ($ST == 0x3A)
        printf "PCB_NV_SCRUB_WAIT"
      end
      if ($ST == 0x3B)
        printf "UNKNOWN PCB VALUE 0x3B"
      end
      if ($ST == 0x3C)
        printf "PCB_EPOLL_WAIT"
      end
    else
      printf "UNKNOWN PCB VALUE 0x%X", $ST
    end
    printf "), pc_fork_name=%s\n", $PCB->pcb.pc_fork_name
#    info symbol $PCB->functionPtr
    printf "    starting function "
    iw ($PCB->functionPtr)
    set $PCB = $PCB->next
    set $I = $I + 1
    if ($PCB == xkRunQueue.tskPtr || $PCB == 0)
      set $DONE = 1
      if ($PCB == 0)
        printf "ERROR, next PCB value is zero\n"
      end
    end
  end
  if ($PCB != xkRunQueue.tskPtr)
    printf "ERROR -- taskcount (%d) off, PCB->next (%p) not %p\n",xkRunQueue.taskCount,$PCB,xkRunQueue.tskPtr
  end
  if ($I != (xkRunQueue.taskCount+1))
    printf "ERROR -- taskcount (%d) off, we got %d\n",xkRunQueue.taskCount,$I
  end
end
#----
document runqueue
The "runqueue" command prints the ccb threads in the run queue.
end
#-----------------------------------------------------------------------------
# The "poolqueue" command prints the ccb threads in the pool queue.
define poolqueue
  set $PCB = xkPoolQueue.tskPtr
  set $I = 1
  printf "There are %d (0x%x) threads on the pool queue\n",xkPoolQueue.taskCount,xkPoolQueue.taskCount
  set $DONE = 0
  while ($PCB != 0 && $DONE == 0 && $I <= xkPoolQueue.taskCount)
    printf "%3d pcb=%p, pc_stat=0x%x, pc_fork_name=%s\n",$I,$PCB,$PCB->pcb.pc_stat,$PCB->pcb.pc_fork_name
    set $PCB = $PCB->next
    set $I = $I + 1
    if ($PCB == xkPoolQueue.tskPtr)
      set $DONE = 1
    end
  end
  if ($PCB != xkPoolQueue.tskPtr)
    printf "ERROR -- taskcount (%d) off, PCB->next (%p) not %p\n",xkPoolQueue.taskCount,$PCB,xkPoolQueue.tskPtr
  end
  if ($I != (xkPoolQueue.taskCount+1))
    printf "ERROR -- taskcount (%d) off, we got %d\n",xkPoolQueue.taskCount,$I
  end
end
#----
document poolqueue
The "poolqueue" command prints the ccb threads in the pool queue.
end
#-----------------------------------------------------------------------------
#-- define i_memfree
#--   set $PREV = (MEM_FREE *)&$arg0.fm_s0base
#--   set $CUR = $PREV->next
#--   if ($CUR == 0)
#--     printf "First address is null?\n";
#--     print $arg0
#--     print $PREV
#--   else
#--     set $I = 0
#--     set $FREE = 0
#--     set $MAXFREE = 0
#--     while ($CUR != 0)
#--       printf "%2.2d: Address %p for %d\n", $I, $CUR, $CUR->len
#--       if ($CUR->len > $MAXFREE)
#--         set $MAXFREE = $CUR->len
#--       end
#--       set $FREE = $FREE + $CUR->len
#--       set $CUR = $CUR->next
#--       set $I = $I + 1
#--     end
#--   end
#--   printf "Total free = %d, Maximum free segment = %d\n", $FREE, $MAXFREE
#--   if ($I != $arg0.fm_chain_len)
#--     printf "ERROR -- $arg0.fm_chain_len = %d, we found %d segments\n", $arg0.fm_chain_len, $I
#--   end
#--   if ($FREE != $arg0.fm_cur_avl)
#--     printf "ERROR -- $arg0.fm_cur_avl = %d, we found total %d bytes free\n", $arg0.fm_cur_avl, $FREE
#--   end
#--   printf "  origin=%p, firstfreeaddr=%p\n", $arg0.fm_origin,$arg0.fm_s0base
#--   printf "  max_avl=%d (%x), min_avl=%d (%x), cur_avl=%d (%x)\n",$arg0.fm_max_avl,$arg0.fm_max_avl,$arg0.fm_min_avl,$arg0.fm_min_avl,$arg0.fm_cur_avl,$arg0.fm_cur_avl
#--   printf "  count=%d (%x) alloctions, waits=%d (%x) threads\n",$arg0.fm_count,$arg0.fm_count,$arg0.fm_waits,$arg0.fm_waits
#--   printf "  chain_len=%d (%x) nodes\n",$arg0.fm_chain_len,$arg0.fm_chain_len
#-- end
#-- #----
#-- document i_memfree
#-- The "i_memfree" command prints out the ccb's free memory list for the argument.
#-- end
#-----------------------------------------------------------------------------
#-- # The "memfree" command prints out the ccb's free memory list.
#-- define memfree
#--   printf "Memory pool K_heapCB\n"
#--   i_memfree K_heapCB
#--   printf "\n"
#--   printf "Memory pool K_sharedHeapCB\n"
#--   i_memfree K_sharedHeapCB
#-- end
#-- #----
#-- document memfree
#-- The "memfree" command prints out both of the ccb's free memory list.
#-- end
#-----------------------------------------------------------------------------
#-- # The "memdump" command prints out the whole ccb memory list.
#-- define memdump
#--   printf "Memory pool K_heapCB\n"
#--   i_memdump K_heapCB
#--   printf "\n"
#--   printf "Memory pool K_sharedHeapCB\n"
#--   i_memdump K_sharedHeapCB
#-- end
#-- #----
#-- document memdump
#-- The "memdump" command prints out both of the ccb's memory lists.
#-- end
#-- #----
#-- define i_memdump
#--     set $HEAP = $arg0
#--     set $ADDR = (MEM_HEADER *)$HEAP->fm_origin
#--     set $START = (UINT32)$ADDR
#--     set $END = $HEAP->fm_origin + $HEAP->fm_max_avl
#--     set $COUNT = 0
#--     set $ALLOCATED = 0
#--     set $FREE = 0
#--     set $FREESIZE = 0
#--     set $USEDSIZE = 0
#--     while ($ADDR != 0 && $ADDR < $END)
#--       set $COUNT = $COUNT + 1
#--       printf "%4d address %p ", $COUNT, ($ADDR + 1)
#--       # Check if allocated memory, or a free memory pointer address.
#--       if ($ADDR->size < $START && $ADDR->size != 0)
#--         set $MEM=(char *)($ADDR + 1)
#--         print_mem_header $MEM
#--         set $FENCE = (MEM_FENCE *)($MEM + (($ADDR->size + 15) & ~15))
#--         if ($FENCE->fence[0] != 0xFFFFFFFF || $FENCE->fence[1] != 0xFFFFFFFF || $FENCE->fence[2] != 0xFFFFFFFF || $FENCE->fence[3] != 0xFFFFFFFF)
#--           printf "-- Corrupted electric fence at the end!\n"
#--           set $ADDR = 0
#--         else
#--           # If not allocated memory, or freed memory -- problems!
#--           if (((MEM_HEADER*)$ADDR)->tag != 0x4d4d)
#--               printf "-- Corrupted memory %d\n", *((UINT32*)0xffff0000)
#-- printf "%d\n", *((UINT32*)0xffff0000)
#--               set $ADDR = 0
#--           else
#--               set $USEDSIZE = $USEDSIZE + ((UINT32)($FENCE + 1) - (UINT32)$ADDR)
#--               set $ADDR = (MEM_HEADER *)($FENCE + 1)
#--               set $ALLOCATED = $ALLOCATED + 1
#--           end
#--         end
#--       else
#--       # We are freed memory -- most likely.
#--         set $FM = (MEM_FREE*)$ADDR
#--         if ($FM->len > $HEAP->fm_max_avl)
#--           printf "-- Corrupted memory -- not free!\n"
#-- printf "%d\n", *((UINT32*)0xffff0000)
#--           set $ADDR = 0
#--         else
#--           printf "size=%6ld free memory (total %d)\n", $FM->len - sizeof(MEM_HEADER) - sizeof(MEM_FENCE), $FM->len
#--           set $FREESIZE = $FREESIZE + $FM->len
#-- #          set $ADDR = (MEM_HEADER *)((UINT32)($ADDR + 1) + $FM->len + sizeof(MEM_FENCE))
#--           set $ADDR = (MEM_HEADER *)((UINT32)$ADDR + $FM->len)
#-- # printf "%d\n", *((UINT32*)0xffff0000)
#--           set $FREE = $FREE + 1
#--         end
#--       end
#--     end
#--     printf "%d segments allocated, and %d segments free => %d free, %d used\n", $ALLOCATED, $FREE, $FREESIZE, $USEDSIZE
#--     if ($ALLOCATED != $HEAP->fm_count)
#--         printf "ERROR - %d segments allocated does not match KERNEL_HEAP_STRUCT of %d\n", $ALLOCATED, $HEAP->fm_count
#--     end
#--     if ($FREESIZE != $HEAP->fm_cur_avl)
#--         printf "ERROR - %d free does does not match KERNEL_HEAP_STRUCT of %d\n", $FREESIZE, $HEAP->fm_cur_avl
#--     end
#--     if ($USEDSIZE + $FREESIZE != $HEAP->fm_max_avl)
#--         printf "ERROR - %d free + %d used does does not match KERNEL_HEAP_STRUCT of %d\n", $FREESIZE, $USEDSIZE, $HEAP->fm_max_avl
#--     end
#-- end
#-- #----
#-- document i_memdump
#-- The "i_memdump" command prints out the ccb's memory list (argument is a *KERNEL_HEAP_STRUCT).
#-- end
#-----------------------------------------------------------------------------
# The "eventtrace" command prints out the ccb's evQueue.
define eventtrace
  set $K = evQueue.evNextP
  set $STOP = $K
  set $FIRST = evQueue.evBaseP
  set $WRAP = evQueue.evEndP
  set $DONE = 0
  set $cnt = 10000
  while ($DONE == 0)
    set $MAJOR = ($K->id >> 16 ) & 0xffff
    set $MINOR = $K->id & 0xffff
    printf "%5d ", $cnt
    set $cnt = $cnt - 1
    if ($MAJOR == 0x8000)
      printf "MRP"
      if ($MINOR == 99)
        printf " START  "
      else
        if ($MINOR == 100)
          printf " TMOCBCK"
        else
          if ($MINOR == 0)
            printf " PI-GOOD"
          else
            if ($MINOR == 1)
              printf " PIERROR"
            else
              if ($MINOR == 2)
                printf " INPRGRS"
              else
                if ($MINOR == 4)
                  printf " TIMEOUT"
                else
                  if ($MINOR == 5)
                    printf " INV-CMD"
                  else
                    if ($MINOR == 6)
                      printf " MALLOCE"
                    else
                      if ($MINOR == 7)
                        printf " PARMERR"
                      else
                        if ($MINOR == 128)
                          printf " BEQFREE"
                        else
                          if ($MINOR == 129)
                            printf " BEQBLKD"
                          else
                            if ($MINOR == 130)
                              printf " BEQFSBK"
                            else
                              if ($MINOR == 131)
                                printf " BEQALLB"
                              else
                                if ($MINOR == 256)
                                  printf " FEQFREE"
                                else
                                  if ($MINOR == 257)
                                    printf " FEQBLKD"
                                  else
                                    if ($MINOR == 512)
                                      printf " GCTSTRT"
                                    else
                                      if ($MINOR == 513)
                                        printf " GCTDONE"
                                      else
                                        printf " ?????? "
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
    else
      if ($MAJOR == 0x4000)
        printf "PKT"
        if ($MINOR == 99)
          printf " START  "
        else
          if ($MINOR == 0)
            printf " PI-GOOD"
          else
            if ($MINOR == 1)
              printf " PIERROR"
            else
              if ($MINOR == 2)
                printf " INPRGRS"
              else
                if ($MINOR == 4)
                  printf " TIMEOUT"
                else
                  if ($MINOR == 5)
                    printf " INV-CMD"
                  else
                    if ($MINOR == 6)
                      printf " MALLOCE"
                    else
                      if ($MINOR == 7)
                        printf " PARMERR"
                      else
                        printf " ?????? "
                      end
                    end
                  end
                end
              end
            end
          end
        end
      else
        if ($MAJOR == 0x2000)
          printf "IPC"
          if ($MINOR == 99)
            printf " START  "
          else
            if ($MINOR == 98)
              printf " CALLBCK"
            else
              if ($MINOR == 97)
                printf " DSPSTRT"
              else
                if ($MINOR == 96)
                  printf " DSPDONE"
                else
                  if ($MINOR == 95)
                    printf " DSPNULL"
                  else
                    if ($MINOR == 94)
                      printf " DSPTUNS"
                    else
                      if ($MINOR == 93)
                        printf " TUNSTRT"
                      else
                        if ($MINOR == 0)
                          printf " TIMEOUT"
                        else
                          if ($MINOR == 1)
                            printf " NO_PATH"
                          else
                            if ($MINOR == 2)
                              printf " ANYPATH"
                            else
                              if ($MINOR == 3)
                                printf " ETHERNT"
                              else
                                if ($MINOR == 4)
                                  printf " FIBRE  "
                                else
                                  if ($MINOR == 5)
                                    printf " QUORUM "
                                  else
                                    printf " ?????? "
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
        else
          if ($MAJOR == 0x1000)
            printf "LOG        "
          else
            if ($MAJOR == 0x0800)
              printf "X1         "
            else
              if ($MAJOR == 0x0400)
                printf "X1_VDC     "
              else
                if ($MAJOR == 0x0200)
                  printf "X1_BF      "
                else
                  if ($MAJOR == 0x0100)
                    printf "RM         "
                  else
                    if ($MAJOR == 0x0080)
                      printf "SIG"
                      if ($MINOR == 99)
                        printf " HRTBEAT"
                      else
                        printf " ?????? "
                      end
                    else
                      if ($MAJOR == 0x0040)
                        printf "UNUSED_B22 "
                      else
                        if ($MAJOR == 0x0020)
                          printf "UNUSED_B21 "
                        else
                          if ($MAJOR == 0x0010)
                            printf "UNUSED_B20 "
                          else
                            if ($MAJOR == 0x0008)
                              printf "UNUSED_B19 "
                            else
                              if ($MAJOR == 0x0004)
                                printf "UNUSED_B18 "
                              else
                                if ($MAJOR == 0x0002)
                                  printf "UNUSED_B17 "
                                else
                                  printf "UNUSED_B16 "
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
    printf "%8d data=%#08x  @ %8d.%08d\n", $MINOR, $K->data, $K->tCoarse, $K->tFine
    set $K = $K + 1
    if ($K >= $WRAP)
      set $K = $FIRST
    end
    if ($K == $STOP)
      set $DONE = 1
    end
  end
end
#----
document eventtrace
The "eventtrace" command prints out the ccb's evQueue (packet trace events).
end
#-----------------------------------------------------------------------------
# This macro prints out the ILT from base for all 4 layers.
define printilt
  set $ILT = (ILT*)$arg0
  set $ILTNEST = 4
  set $cnt = 0
# We know that an ILT is on a 32 byte aligned memory location.
  while ((((UINT32)$ILT & 63) != 0) && ($cnt < $ILTNEST))
    set $ILT = $ILT - 1
    set $cnt = $cnt + 1
  end
  if ($cnt == $ILTNEST)
    printf "Could not calculate base of ILT\n"
  else
    set $cnt = 0
    while ($cnt < $ILTNEST)
      if ($ILT == $arg0)
        printf "*"
      else
        printf " "
      end
      printf "ILT level %d at 0x%08x\n", $cnt, $ILT
      printf "    fthd=0x%08x bthd=0x%08x misc=0x%08x linux_val=0x%08x\n", $ILT->fthd, $ILT->bthd, $ILT->misc, $ILT->linux_val
      printf "    w0=0x%08x   w1=0x%08x   w2=0x%08x   w3=0x%08x\n", ((UINT32*)$ILT)[4], ((UINT32*)$ILT)[5], ((UINT32*)$ILT)[6], ((UINT32*)$ILT)[7]
      printf "    w4=0x%08x   w5=0x%08x   w6=0x%08x   w7=0x%08x\n", ((UINT32*)$ILT)[8], ((UINT32*)$ILT)[9], ((UINT32*)$ILT)[10], ((UINT32*)$ILT)[11]
#      printf "    cr=0x%08x -- ", $ILT->cr
#      info symbol $cr
      printf "    cr="
      set $cr = ((UINT32)($ILT->cr)&0x7fffffff)
      iw $cr
      set $ILT = $ILT + 1
      set $cnt += 1
    end
  end
end
#----
document printilt
Macro to print all 4 (CCB) layers of an ILT.
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
            if ($ILT == 0 || $ILT->fthd == $ILT)
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
        set $nILT = $lILT->misc
        set $nVRP = $lILT->cr
        printf "pLLActiveILTHead[%d] #%d - ILT level %d=%p  base address=%p\n", $I, $C, ($lILT-$ILT), $lILT, $ILT
         printf "    old ILT/VRP=%p/%p/  new ILT/VRP=%p/%p\n", $oILT, $oVRP, $nILT, $nVRP
        set $C = $C + 1
        if ($lILT == $eILT)
          set $lILT = 0
        else
          if ($lILT == $lILT->fthd)
            printf "ERROR - lILT (%p) = lILT->fthd (%p)\n", $lILT, $lILT->fthd
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
  set $ILTNEST = 4
  set $cnt = 0
# Originally (before new memory pool with pre-post patterns of 32 bytes), the
# knowledge that an ILT was on a 64 byte aligned memory location was counted on.
  while ((((UINT32)$ILT & 15) != 0) && ($cnt < $ILTNEST))
    set $ILT = $ILT - 1
    set $cnt = $cnt + 1
  end
end
#----
document i_find_ilt_base
Internal macro to find the ilt base address (if possible).
# This finds base of ilt by subtracting 0x34 till bottom 6 bits are zero (but max of 7/11).
end
#-----------------------------------------------------------------------------
# This macro prints out the ILT and readies for "next".
define i_linknextilt
  if ($ILT != 0)
    printf "%2d:ILT(0x%08x) fthd=0x%08x bthd=0x%08x misc=0x%08x linux_val=0x%x\n", $arg0, $ILT, $ILT->fthd, $ILT->bthd, $ILT->misc, $ILT->linux_val
    printf "                w0=0x%08x w1=0x%08x w2=0x%08x w3=0x%08x\n", ((UINT32*)$ILT)[4], ((UINT32*)$ILT)[5], ((UINT32*)$ILT)[6], ((UINT32*)$ILT)[7]
    printf "                w4=0x%08x w5=0x%08x w6=0x%08x w7=0x%08x\n", ((UINT32*)$ILT)[8], ((UINT32*)$ILT)[9], ((UINT32*)$ILT)[10], ((UINT32*)$ILT)[11]
#    printf "       cr=0x%08x ", $ILT->cr
    printf "       cr="
    if ((UINT32)$ILT->cr != 0xcacacaca && (UINT32)$ILT->cr < (UINT32)0x48000000)
#      info symbol $ILT->cr
      iw $ILT->cr
      if ($arg1 == 0)
        set $pVRPCMD = (VRPCMD*)&(((UINT32*)$ILT)[4])
        set $ILTM1 = $ILT-1
#        set $RRP = (RRP*)(((UINT32*)$ILTM1)[4])
        set $RRP = (((UINT32*)$ILTM1)[4])
        if ((UINT32)$RRP > (UINT32)&__executable_start && (UINT32)$RRP < (UINT32)0xf0000000)
#         printf "   RRP=%08x RRP function=0x%04x, SDA= %016x\n", $RRP, $RRP->function, $RRP->startDiskAddr
         printf "   RRP=%08x  rid=%d, function=0x%04x, SDA=%016x\n", $RRP, *((UINT16*)$RRP+2), *(UINT16*)$RRP, *(UINT64*)($RRP+16)
        end
        if ((UINT32)$pVRPCMD > (UINT32)&__executable_start)
          set $pMRP = (MR_PKT *)($pVRPCMD->pvrVRP)
          if ((UINT32)$pMRP > (UINT32)&__executable_start && (UINT32)$pMRP < (UINT32)0xf0000000)
            set $pReq = (LOG_HEADER_PKT *)($pMRP->pReq)
#.............................................................................
#   p *$pMRP
#           printf "   MRP function=0x%04x, version=0x%02x, pReqPCI=0x%08x\n", $pMRP->function, $pMRP->version, $pMRP->pReqPCI,
#           printf "       pRsp=0x%08x, rspLen=0x%04x, pReq=0x%08x, reqLen=0x%04x\n", $pMRP->rspLen, $pMRP->pRsp, $pMRP->pReq, $pMRP->reqLen
#.............................................................................
#    p *$pReq
            if ((UINT32)$pReq > (UINT32)&__executable_start && (UINT32)$pReq < (UINT32)0xf0000000)
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
# 0x48000000 is where FE shared memory is.
define i_linkilt
  set $Iilt = $arg0
  set $ILTNEST = 4
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
#----
document i_linkilt
Internal macro to print entry in link.pqc_Qb[#] queue. See linkqueues and i_linknextilt.
end
#-----------------------------------------------------------------------------
define print_nonzero_commandRecordTable
  set $I = 0
  set $COUNT = 0
  while ($I < 1000)
    if (commandRecordTable[$I].fence[0] == 0)
    else
      set $COUNT = $COUNT + 1
      printf "%2d commandRecordTable[%d] command %u (0x%x)\n", $COUNT, $I, commandRecordTable[$I].commandCode, commandRecordTable[$I].commandCode
      printf "   fence=%16.16s  callerPCB=%p  timeout=%d (0x%x)\n", commandRecordTable[$I].fence, commandRecordTable[$I].callerPCB, commandRecordTable[$I].timeout, commandRecordTable[$I].timeout
      printf "   commandBufferIn=%p  commandBufferOut=%p\n", commandRecordTable[$I].commandBufferIn, commandRecordTable[$I].commandBufferOut
      printf "   state=%d  completion=%d  timeoutAction=%d\n", commandRecordTable[$I].state, commandRecordTable[$I].completion, commandRecordTable[$I].timeoutAction
    end
    set $I = $I + 1
  end
end
#-----------------------------------------------------------------------------
#-- define print_ilt_vrp
#--   set $ILTinput = (ILT*)$arg0
#--   set $VRPinput = (VRP*)$arg1
#--   printf "ilt=%p vrp=%p  delta=%d\n", $ILTinput, $VRPinput, (char *)$VRPinput - (char *)$ILTinput
#--   print_mem_fence $arg0
#--   x/60 (char *)$ILTinput-16
#--   print_mem_fence $arg1
#--   x/40 (char *)$VRPinput-16
#-- end
define print_ilt_2_vrp
  set $ILT = (ILT*)$arg0
# LL_TargetTaskCompletion
  if ($ILT->cr == 0x080d09d5)
    printf "%p => %p (fthd=%p)\n", $ILT, (VRP*)$ILT->ilt_normal.w0, (VRP*)$ILT->fthd
  else
# PI_MRPCallback
    if ($ILT->cr == 0x0806f734)
      printf "%p => %p (fthd=%p)\n", $ILT, (VRP*)(($ILT + 1)->ilt_normal.w4), (VRP*)$ILT->fthd
    else
      printf "%p Hosed\n", $ILT
    end
  end
end
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# This macro prints out the controllers free memory.
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
        printf "memory free for %-8.8x:\n", $fram
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
Print out the controllers free memory in hex (arg 0 does K_ncdram, 1 does P_ram).
end
#-----------------------------------------------------------------------------
# This macro prints out the controllers free memory.
define mmfree
    set $fram=(unsigned int)$arg0
    if ($fram == 0)
      set $fram=(unsigned int)&K_ncdram
      set $mem = (struct before_after *)((char *)&SHMEM_END + 32)
      set $maxmem = (char *)startOfBESharedMem - (char *)$mem
      printf "memory free for K_ncdram of initial 0x%x (%d):\n", $maxmem, $maxmem
    else
      if ($fram == 1)
        set $fram = (unsigned int)&P_ram
        printf "memory free for P_ram of initial 0x%x (%d):\n", local_memory_pool_start, local_memory_pool_start
        set $maxmem = P_ram.fmm_fms->fms_Maximum_available
      else
        printf "memory free for %-8.8x:\n", $fram
        set $maxmem = 0
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
      printf "  addr: %-8.8x mem=%-8.8x size=%-8.8x FMS=%-8.8x S=%-8.8x st=%-2.2x opt=%-2.2x\n", $fram, *$fram, *($fram+4), *($fram+8), *($fram+12), *((unsigned char *)($fram+16)), *((unsigned char *)($fram+17))
      set $fram_free = *($fram+4)
      if ($max_free < $fram_free)
        set $max_free = $fram_free
      end
    else
      printf "  addr: %-8.8x mem=%-8.8x -- ends here -- st=%-2.2x opt=%-2.2x\n", $fram, *$fram, *((unsigned char *)($fram+16)), *((unsigned char *)($fram+17))
      set $fram_free = 0
    end
    set $fram = *($fram)

# Now loop through the memory sections.
    set $PRE = (struct before_after *)$fram
    set $DONE = 0
    while ($PRE != 0 && $DONE == 0)
      set $fram_free = $fram_free + $PRE->length + 2 * sizeof(struct before_after)
      if ($max_free < $PRE->length + 64)
        set $max_free = $PRE->length + 64
      end
      printf "  addr: %-8.8x %11.11s:%u u/f=%d length=%-8.8x\n", $PRE, $PRE->str, $PRE->line_number, $PRE->used_or_free, $PRE->length
      mmchkgap $PRE
      set $PRE = $PRE->next
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
      printf "K_ncdram: Avail=0x%x (%d)  Max=0x%x (%d)  Min=0x%x (%d)  WaitCount=%d\n", K_ncdram->fmm_fms->fms_Available_memory, K_ncdram->fmm_fms->fms_Available_memory, K_ncdram->fmm_fms->fms_Maximum_available, K_ncdram->fmm_fms->fms_Maximum_available, K_ncdram->fmm_fms->fms_Minimum_available, K_ncdram->fmm_fms->fms_Minimum_available, xkMallocWaitCount
      if (K_ncdram->fmm_fms->fms_Available_memory != $fram_free)
        printf "ERROR IN FREE SIZES (calculated=%d, fms_Available_memory=%d)\n", $fram_free, K_ncdram->fmm_fms->fms_Available_memory
      end
    end
#
    if ($startmem == (unsigned int)&P_ram)
      printf "P_ram: Avail=0x%x (%d)  Max=0x%x (%d)  Min=0x%x (%d)\n", P_ram.fmm_fms->fms_Available_memory, P_ram.fmm_fms->fms_Available_memory, P_ram.fmm_fms->fms_Maximum_available, P_ram.fmm_fms->fms_Maximum_available, P_ram.fmm_fms->fms_Minimum_available, P_ram.fmm_fms->fms_Minimum_available
      if (P_ram.fmm_fms->fms_Available_memory != $fram_free)
        printf "ERROR IN FREE SIZES (calculated=%d, ncCur=%d)\n", $fram_free, P_ram.fmm_fms->fms_Available_memory
      end
    end
end
#...
document mmfree
Print out the controllers free memory (arg 0 does K_ncdram, 1 does P_ram).
end
#-----------------------------------------------------------------------------
# This macro prints out the gaps in free memory.
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
      printf "    Gap of %9u (0x%-8.8x) bytes between 0x%-8.8x and 0x%-8.8x\n", $xcmdlen, $xcmdlen, $xcmdaddr, $next
      set $gap_size = $xcmdlen
          printf "    0x%-8.8x: 0x%-8.8x 0x%-8.8x 0x%-8.8x 0x%-8.8x\n", $xcmdaddr, *($xcmdaddr), *($xcmdaddr+4), *($xcmdaddr+8), *($xcmdaddr+12)
    else
      if ($xcmdaddr > $next)
        printf "ERROR next %-8.8x < current %-8.8x+length %x (%d)=%-8.8x by %x (%d)\n", $next, $ncgram, $len, $len, $xcmdaddr, $xcmdaddr-$next, $xcmdaddr-$next
      else
        printf "    NO GAP between 0x%-8.8x and 0x%-8.8x of %u (0x%-8.8x) bytes\n", $xcmdaddr, $next, $xcmdlen, $xcmdlen
      end
      set $DONE = 1
    end
  else
    printf "  End of free memory just before %-8.8x\n", $xcmdaddr
  end
end
#...
document mmchkgap
Print out the gaps in free memory (arg 0 does K_ncdram, 1 does P_ram).
end
#-----------------------------------------------------------------------------
# This macro prints out the malloced memory.
define mmalloc
  set $ncram=$arg0
  if ($ncram == 0)
    set $ncram = (unsigned int)&K_ncdram
    set $lastmalloc = NcdrAddr + NcdrSize
    set $firstgap = (*$ncram) - (unsigned int)NcdrAddr
  else
    if ($ncram == 1)
      set $ncram = (unsigned int)&P_ram
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
Print out the malloc-ed memory (arg 0 does K_ncdram, 1 does P_ram).
end
#-----------------------------------------------------------------------------
# This macro prints out the CCB controllers free memory.
define ccbmmfree
  mmfree 0
  printf "-----------------------------------------------------------------------------\n"
  mmfree 1
  printf "-----------------------------------------------------------------------------\n"
end
#...
document ccbmmfree
Print out the CCB controllers free memory for the two sections.
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
  set $START = (((UINT32)&SHMEM_END + 0x3f) & ~0x3f) + 32
  set $LTH = 0
  set $TOTAL_SHARED_SIZE = 0x0A000000 - ($START - ((UINT32)(&MAKE_DEFS_CCB_LOAD) & 0xffff0000))
  while ($LTH < ($TOTAL_SHARED_SIZE-32))
    set $PRE = (struct before_after *)$START
    set $SIZE = $PRE->length
    if ($SIZE == 0 || $SIZE > 400*1024*1024)
      printf "Address %p length %u is not reasonable.\n", $START, $SIZE
      stopnow
    end
    set $ADDRESS = $START + sizeof(struct before_after)
    set $ROUND_UP_SIZE = ($SIZE + 64 - 1) & ~(64 - 1)
    set $POST = (struct before_after *)($ADDRESS + $ROUND_UP_SIZE)
    set $TOTAL_SIZE = $ROUND_UP_SIZE + sizeof(struct before_after) + sizeof(struct before_after)
    if ($LTH + $TOTAL_SIZE > ($TOTAL_SHARED_SIZE - 32))
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
define search_CCB_shared_memory
  set $VALUE = $arg0
  set $MASK = $arg1
  set $INC = $arg2
  set $SEARCHVAL = $VALUE & $MASK
  printf "Searching CCB shared memory for value (0x%08x:%08x) with mask (0x%08x) index=%d\n", $VALUE, $SEARCHVAL, $MASK, $INC
  set $MEM = (char *)((UINT32)(&MAKE_DEFS_CCB_LOAD) & 0xffff0000)
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
define search_memory
  set $MEM = $arg0
  set $LIMIT = $arg1
  set $VALUE = $arg2
  set $MASK = $arg3
  set $INC = $arg4
  set $SEARCHVAL = $VALUE & $MASK
  printf "Searching CCB shared memory for value (0x%08x:%08x) with mask (0x%08x) index=%d\n", $VALUE, $SEARCHVAL, $MASK, $INC
  set $MEM = (char *)((UINT32)(&MAKE_DEFS_CCB_LOAD) & 0xffff0000)
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
document search_memory
Macro to search all CCB shared memory for a specified value and mask.
------------------------------------------------------------------------------
Example: "search_memory 0x4075000 12812000 0x67040001 0xffff000f 4" searches
memory starting at 0x4075000 for 12812000 bytes for the 32 bit
         value 0x6704XXX1 and prints the address(es) where it was found, and
         increment by 4 (sizeof UINT32).
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
Internal macro to count number of ilts (first argument is a pointer to a pointer to and ilt.
end
#-----------------------------------------------------------------------------
define traversepool
  set $START = $arg0
  if ($START == &K_ncdram)
    set $MEM = (struct before_after *)((char *)&SHMEM_END + 32)
    set $MAX_MEM = (char *)startOfBESharedMem - (char *)$MEM
  else
    set $MEM = (struct before_after *)((char *)&local_memory_start + 32)
    set $MAX_MEM = (char *)&local_memory_start + local_memory_pool_start
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
Print out summary of original memory pool methodology.
end
#-----------------------------------------------------------------------------
# Allow this (ccb) name to be used.
define print_mem_header
  print_before_after $arg1
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
  printf "%c PRE for  0x%08lx (0x%08lx)", $PRE->used_or_free == 0 ? 'F' : ($PRE->used_or_free == 2 ?  'D':' '), $ADDR, (UINT32)$PRE
  printf "  %08lx %11.11s:%4d u/f=%d next=0x%08lx lth=%5d %08lx\n", $PRE->pre1, $PRE->str, $PRE->line_number, $PRE->used_or_free, (UINT32)($PRE->next), $PRE->length, $PRE->pre2
  set $ASIZE = $PRE->length <  64 ? 64 : $PRE->length
  set $AROUND = ($ASIZE + 64 - 1) & ~(64 - 1)
  set $POST = (struct before_after *)($ADDR + $AROUND)
  printf "  POST for 0x%08lx (0x%08lx)", $ADDR, (UINT32)$POST
  printf "  %08lx %11.11s:%4d u/f=%d next=0x%08lx lth=%5d %08lx\n", $POST->pre1, $POST->str, $POST->line_number, $POST->used_or_free, (UINT32)($POST->next), $POST->length, $POST->pre2
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
  printf "private memory starts @ 0x%08x for %d (0x%08.8x)\n", &local_memory_start, local_memory_pool_start, local_memory_pool_start
  printf "P_ram:"
     traversepool &P_ram
  printf "K_ncdram:"
     traversepool &K_ncdram
  printf "ilt:"
     traverseprivatepool pool_ilt
  printf "vrp:"
     traverseprivatepool pool_vrp
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
define map
    printf "FE shared memory starts at      = 0x%08.8x\n", (UINT32)$FE_MEMORY_STARTS
    printf "CCB shared memory starts at     = 0x%08.8x\n", (UINT32)(&MAKE_DEFS_CCB_LOAD) & 0xffff0000
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
    printf "MY CCB private memory starts at = 0x%08.8x\n", (UINT32)&local_memory_start
    printf "size_ILT_ALL_LEVELS             = 0x%08.8x (%u)\n", (UINT32)size_ILT_ALL_LEVELS, (UINT32)size_ILT_ALL_LEVELS
    printf "\n"
    printf "endOfBESharedMem                = 0x%08.8x\n", (UINT32)endOfBESharedMem
    #
    printf "startOfMySharedMem              = 0x%08.8x\n", (UINT32)startOfMySharedMem
    printf "endOfMySharedMem                = 0x%08.8x\n", (UINT32)endOfMySharedMem
    #
    printf "&MAKE_DEFS_CCB_LOAD             = 0x%8.8x\n", (UINT32)&MAKE_DEFS_CCB_LOAD
    printf "pStartOfHeap                    = 0x%08.8x\n", (UINT32)pStartOfHeap
end
#...
document map
Print out where the various memory segments are (Assumes lengths in gdb macro match Makefile-*.defs).
end
#-----------------------------------------------------------------------------
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
# This macro prints out the ILT from base for all 7/11 layers, with possible VRP one back.
define pilt
print_nonzero_ilt_vrp $arg0
end

define print_nonzero_ilt_vrp
  set $P_ILT = $arg0
  i_find_ilt_base $P_ILT
  i_print_nonzero_ilt_vrp $ILT $P_ILT $cnt 0 0
end

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
#          printf "    cr=0x%08x -- ", $p_ILT->cr
          printf "    cr="
          if ($p_ILT->cr != 0xcacacaca)
            set $cr = ((UINT32)($p_ILT->cr)&0x7fffffff)
#            info symbol $cr
            iw $cr
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
document print_nonzero_ilt_vrp
Internal Macro to print all non-zero 7 (BE) or 11 (FE) layers of an ILT with possible VRP at location given.
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
define print_shared_memory
  set $START = ((UINT32)&SHMEM_END) + 32
  set $mem = (struct before_after *)((char *)&SHMEM_END + 32)
  set $maxmem = (char *)startOfBESharedMem - (char *)$mem - 32
  set $LTH = 0
  set $DONE = 0
  while ($LTH < $maxmem && $DONE == 0)
    set $PRE_at = (struct before_after *)$START
    set $SIZE = $PRE_at->length
    if ($SIZE == 0 || $SIZE > 300*1024*1024)
      printf "Address %p length %u is not reasonable.\n", $START, $SIZE
      set $DONE = 1
    else
      set $ADDRESS = $START + sizeof(struct before_after)
      set $ROUND_UP_SIZE = ($SIZE + 64 - 1) & ~(64 - 1)
      set $TOTAL_SIZE = $ROUND_UP_SIZE + sizeof(struct before_after) + sizeof(struct before_after)
      if ($LTH + $TOTAL_SIZE > $maxmem)
        printf "Address %p length %u puts us beyond end of allocated.\n", $ADDRESS, $SIZE
        set $DONE = 1
      else
        print_before_after $ADDRESS
        set $START = $START + $TOTAL_SIZE
        set $LTH = $LTH + $TOTAL_SIZE
      end
    end
  end
end
#...
document print_shared_memory
Print out complete local memory allocation (every one).
end
#=============================================================================
define print_local_memory
  set $START = (UINT32)&local_memory_start + 32
  set $maxmem = P_ram.fmm_fms->fms_Maximum_available
  set $LTH = 0
  set $DONE = 0
  while ($LTH < $maxmem && $DONE == 0)
    set $PRE_at = (struct before_after *)$START
    set $SIZE = $PRE_at->length
    if ($SIZE == 0 || $SIZE > 300*1024*1024)
      printf "Address %p length %u is not reasonable.\n", $START, $SIZE
      set $DONE = 1
    else
      set $ADDRESS = $START + sizeof(struct before_after)
      set $ROUND_UP_SIZE = ($SIZE + 64 - 1) & ~(64 - 1)
      set $TOTAL_SIZE = $ROUND_UP_SIZE + sizeof(struct before_after) + sizeof(struct before_after)
      if ($LTH + $TOTAL_SIZE > $maxmem)
        printf "Address %p length %u puts us beyond end of allocated.\n", $ADDRESS, $SIZE
        set $DONE = 1
      else
        print_before_after $ADDRESS
        set $START = $START + $TOTAL_SIZE
        set $LTH = $LTH + $TOTAL_SIZE
      end
    end
  end
end
#...
document print_local_memory
Print out complete local memory allocation (every one).
end
#=============================================================================
# arg0 is how many to print (in reverse).
define i_traceloginternal
  set $I = $arg3
  set $END = $arg0
  set $CT_TL_strings = sizeof(CT_laststrings) / sizeof(CT_laststrings[0])
  if (CT_laststrings[1] != 0)
    while ($I < $END)
      set $J = (CT_last_history_string-$I+$CT_TL_strings)%$CT_TL_strings
      if ($arg2 == 0)
        printf "tracelog[%-2.2d]= %p -- %s", $I, CT_last_functionPtr[$J], CT_tracelog+CT_laststrings[$J]
      else
        set $K = (CT_last_history_string-$I-1+$CT_TL_strings+$CT_TL_strings)%$CT_TL_strings
        printf "tracelog[%-2.2d]= %p -- %9llu: %s", $I, CT_last_functionPtr[$J], CT_last_tsc[$J] - CT_last_tsc[$K], CT_tracelog+CT_laststrings[$J]
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
# arg0 is entry to print.
define tracelogsingletsc
  set $I = $arg0
  set $CT_TL_strings = sizeof(CT_laststrings) / sizeof(CT_laststrings[0])
  set $J = (CT_last_history_string-$I+$CT_TL_strings)%$CT_TL_strings
  set $K = (CT_last_history_string-$I-1+$CT_TL_strings+$CT_TL_strings)%$CT_TL_strings
  printf "tracelog[%-2.2d]= %p -- %9llu: %s", $I, CT_last_functionPtr[$J], CT_last_tsc[$J] - CT_last_tsc[$K], CT_tracelog+CT_laststrings[$J]
  printf "                  -- %16llu\n", CT_last_tsc[$J]
end
#...
document tracelogsingletsc
Print out a single tracelog entry
end
#-----------------------------------------------------------------------------
# This prints out the CT_tracelog - last $arg0 entries.
define tracelog
  i_traceloginternal $arg0 0 0 1
end
#...
document tracelog
Print the last NNN (argument) CT_tracelog entries (HISTORY_KEEP).
end
#-----------------------------------------------------------------------------
# This prints out the CT_tracelog - all 800000 entries.
define tracelogfull
  set $CT_TL_strings = sizeof(CT_laststrings) / sizeof(CT_laststrings[0])
  i_traceloginternal $CT_TL_strings 0 0 1
end
#...
document tracelogfull
Print all 800000 CT_tracelog entries (HISTORY_KEEP).
end
#-----------------------------------------------------------------------------
define tracelogtsc
  i_traceloginternal $arg0 0 1 1
end
#...
document tracelogtsc
Print NNN (argument) tracelog and time via tsc (HISTORY_KEEP HISTORY_TSC_KEEP).
end
#-----------------------------------------------------------------------------
define tracelogtscfrom
  set $FROM = $arg0
  set $NUM = $FROM + $arg1
  i_traceloginternal $NUM 0 1 $FROM
end
#...
document tracelogtscfrom
Print (2nd argument) tracelog and tsc time starting at (1st argument) -- HISTORY_KEEP+HISTORY_TSC_KEEP.
end
#-----------------------------------------------------------------------------
define tracelogtscfull
  set $CT_TL_strings = sizeof(CT_laststrings) / sizeof(CT_laststrings[0])
  i_traceloginternal $CT_TL_strings 0 1 1
end
#...
document tracelogtscfull
Print 800000 tracelog and time via tsc (HISTORY_KEEP HISTORY_TSC_KEEP).
end
#-----------------------------------------------------------------------------
define lastpcbs
  set $NumberPCB = sizeof(k_last_pcbs) / sizeof(k_last_pcbs[0])
  set $PCBptr = &k_last_pcbs[0]
  set $I = 0
  while ($I < $NumberPCB - 1)
    set $J = k_last_pcbs[$I]
    printf "%3d %9llu 0x%08x %-13.13s, ", $I, last_runtime[$I] - last_runtime[$I+1], k_last_pcbs[$I], last_pc_fork_name[$I]
#    info symbol last_functionPtr[$I]
    iw last_functionPtr[$I]
    set $I = $I + 1
  end
  printf "%3d ......... 0x%08x %-13.13s, ", $I, k_last_pcbs[$I], last_pc_fork_name[$I]
#  info symbol last_functionPtr[$I]
  iw last_functionPtr[$I]
end
#...
document lastpcbs
Print the last 64 XK_PCBs to run. (see alllastpcbs if you want to specify the number -- debug build has 512).
end
#-----------------------------------------------------------------------------
#-- define iw
#--   if ($arg0 != 0)
#--     set listsize 1
# Following doesn't work any more 2011-05-13
#--     list *$arg0
#--     set listsize 10
#--   else
#--     printf "0x%08x\n", $arg0
#--   end
#-- end
define iw
  if ($arg0 != 0)
    printf "0x%08x ", $arg0
    info symbol $arg0
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
# Do not attempt to step into routines without source code.
set step-mode off
# Deactivate "more" on long prints.
set pagination off
# Allow potentially dangerous operations (like quit) without a "y".
set confirm no
# Set output radix to be base 16 by default.
set output-radix 16
#-----------------------------------------------------------------------------
handle SIGWINCH nostop noprint nopass

handle SIGHUP stop print nopass
handle SIGINT stop print nopass
handle SIGQUIT stop print nopass
handle SIGILL stop print nopass
handle SIGABRT stop print nopass
handle SIGBUS stop print nopass
handle SIGFPE stop print nopass
handle SIGUSR1 stop print nopass
handle SIGSEGV stop print nopass
# handle SIGUSR2 stop print nopass
handle SIGPIPE noprint pass
# handle SIGALRM stop print nopass
handle SIGTERM stop print nopass
# handle SIGSTKFLT stop print nopass
handle SIGCHLD noprint pass
handle SIGCONT stop print nopass
handle SIGSTOP stop print nopass
handle SIGTSTP stop print nopass
handle SIGTTIN stop print nopass
handle SIGTTOU stop print nopass
handle SIGURG stop print nopass
handle SIGXCPU stop print nopass
handle SIGXFSZ stop print nopass
handle SIGVTALRM stop print nopass
handle SIGPROF stop print nopass
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

#-----------------------------------------------------------------------------
define exit
  quit
end
#-----------------------------------------------------------------------------
####
## Modelines:
## Local Variables:
## tab-width: 4
## indent-tabs-mode: nil
## End:
## vi:sw=4 ts=4 expandtab
# End of file .gdbinit in CCB/Src
