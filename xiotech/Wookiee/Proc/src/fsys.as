# $Id: fsys.as 159129 2012-05-12 06:25:16Z marshall_midden $
#**********************************************************************
#
#  NAME: fsys.as
#
#  PURPOSE:
#       To provide common support for interacting with the file system.
#
#  FUNCTIONS:
#  Copyright (c) 2000-2010  Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  FS$VerifyDirectory      # Verify the integrity of the directory
        .globl  FS_InitDirectory        # Initialize the directory
        .globl  FS$WriteFile            # Write a file
        .globl  FS_WriteFile            # Write a file (C call)
        .globl  FS$MultiWrite           # Write a file to all devices
        .globl  FS_MultiWrite           # Write a file to all devices (C call)
        .globl  FS$ReadFile             # Read a file
        .globl  FS_ReadFile             # Read a file (C call)
        .globl  FS$MultiRead            # Read a file from any device
        .globl  FS$UpdateFS             # Update the file system of a new device
        .globl  FS$exec                 # Executive
        .globl  FS$cleanup              # Clean up driver
        .globl  FS$stop                 # Clean up stop
        .globl  FS$resume               # Clean up resume
#
# --- global data declarations ----------------------------------------
#
        .globl  gFSOpWaiting            # Boolean for operation waiting
#
# --- Local data
#
        .data
#
# --- File names
#
FS_dirname:
        .ascii  "Directory "
        .short  fiddirsiz
        .word   fiddirstrt
#
FS_labelname:
        .ascii  "XIO Label "
        .short  fidlabelsiz
        .word   fidlabelstrt
#
FS_benvramname:
        .ascii  "BE NVRAM  "
        .short  fidbenvramsiz
        .word   fidbenvramstrt
#
FS_fenvramname:
        .ascii  "FE NVRAM  "
        .short  fidfenvramsiz
        .word   fidfenvramstrt
#
FS_femcvramname:
        .ascii  "CCB NVRAM "
        .short  fidemcnvramsiz
        .word   fidemcnvramstrt
#
FS_scratchname:
        .ascii  "ST Scratch"
        .short  fidscratchsiz
        .word   fidscratchstrt
#
end_FS_dirname:
        .set    fsdirlen,end_FS_dirname-FS_dirname  # Length of default directory
#
# --- Shared memory data
#
        .section        .shmem
#
# --- Executive QCB
#
        .globl  f_exec_qu               # Easier for debugging
#
        .align  4
f_exec_qu:
        .word   0                       # Queue head
        .word   0                       # Queue tail
        .word   0                       # Queue count
        .word   0                       # Associated PCB
#
        .data
        .globl  f_cleanup_pcb           # Easier for debugging
        .globl  f_stop_cnt              # Easier for debugging
        .globl  f_cleanup_req_run       # Easier for debugging
        .globl  f_cleanup_running       # Easier for debugging
#
        .align  4
f_cleanup_pcb:
        .word   0
#
f_stop_cnt:
        .word   0
#
f_cleanup_req_run:
        .word   0
#
f_cleanup_running:
        .word   0
#
gFSOpWaiting:
        .word   FALSE
#
# --- executable code -------------------------------------------------
#
        .text
#**********************************************************************
#
#  NAME: FS_Que
#
#  PURPOSE:
#       To provide a common means of queuing file system requests
#       to this module.
#
#  DESCRIPTION:
#       The ILT and associated MRP are queued to the tail of the
#       executive queue.  The executive is activated to process this
#       request.  This routine may be called from either the process or
#       interrupt level.
#
#  CALLING SEQUENCE:
#       call    FS_Que
#
#  INPUT:
#       g1 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void FS_Que(ILT* pILT);
        .globl  FS_Que
FS_Que:
c       asm("   .globl  FS$que");
c       asm("FS$que:     ");
        lda     f_exec_qu,r11           # Get queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: FS$exec
#
#  PURPOSE:
#       To provide a means of processing MRP requests which have been
#       previously queued to this module.
#
#  DESCRIPTION:
#       The queuing routine FS$que deposits a MRP request into the queue
#       and activates this executive if necessary.  This executive
#       extracts the next MRP request from the queue and initiates that
#       request.
#
#       The functions called all are provided with the pointer to the MRP.
#
#       The functions are expected to return a length for data returned
#       and the status of the operation.
#
#       The following registers are used for the purposes described above.
#
#           g0 - MRP pointer
#           g1 - return status from function
#           g2 - return length
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
FS$exec:
        ldconst FALSE,r3
        st      r3,gFSOpWaiting         # Init to no operation waiting
        b       .fex20                  # Exchange to start off
#
# --- Set this process to not ready
#
.fex10:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
        call    K$qxchang               # Give up processor
#
# --- Get next queued request
#
.fex20:
        lda     f_exec_qu,r11           # Get executive queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count and PCB
        mov     r12,r10                 # Isolate next queued ILT
        cmpobe  0,r12,.fex10            # Jif none
#
# --- Remove this request from queue ----------------------------------
#
        ld      il_fthd(r12),r12        # Dequeue this ILT
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be      .fex30                  # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.fex30:
#
# --- Prep return packet size and error code
#
# --- Register usage
#
#     g1/g2 - return values from the called function.
#     r11 - holding register for the request pointer (MRP pointer)
#     r10 - holding register for the ILT pointer
#     r9  - opcode (MRP function code)
#
        ldconst deinvpkttyp,g1          # Set invalid function type
        ldconst mrrsiz,g2               # Return packet size
#
# --- Determine type of request
#
        ld      il_w0-ILTBIAS(r10),r11  # Get request ptr
        ldos    mr_func(r11),r9         # Get request function type
        ldconst mrfsysop,r3             # Get the only allowed opcode
        cmpobe  r9,r3,.fex40            # Jif a fsys op.
#
        ldconst mrnopfsys,r3            # Check for no-op
        cmpobne r9,r3,.fex85            # Jif not a nop
#
        mov     r11,g0                  # Load the MRP pointer
        mov     r10,g14                 # Load ILT pointer
        call    D$nop                   # Set return values
#
        ld      mr_rptr(r11),r3         # Get the return data ptr
        stob    g1,mr_status(r3)        # Plug return status code
        st      g2,mr_rlen(r3)          # Set return packet size
        b       .fex90                  # Exit
#
# --- Validate the length of the packet and the return length.
#
.fex40:
        ldconst mfssiz,r4               # Get the expected length
        ldconst deinvpktsiz,g1          # Prep possible error code
        ld      mr_len(r11),r7          # Get the length
        cmpobne r4,r7,.fex85            # Exit w/ error if not equal
#
        ldconst mfsrsiz,r4              # Get the expected return data length
        ldconst deretlenbad,g1          # Prep possible error code
        ld      mr_ralloclen(r11),r7    # Get the return allocation length
        cmpobne r4,r7,.fex85            # Exit w/ error if not equal
#
        ldconst TRUE,r3
        st      r3,gFSOpWaiting         # Indicate operation waiting
#
        call    O$stop                  # Stop online activity
        call    FS$stop                 # Stop any cleanup in progress
c       apool_stop();                   #Stop async activity
#
        ldconst FALSE,r3
        st      r3,gFSOpWaiting         # No operation waiting now
#
        mov     r11,g0                  # Load the MRP pointer
        mov     r10,g14                 # Load ILT pointer
        call   f$fsysop                 # Execute f$fsysop function
#
.fex85:
        ld      mr_rptr(r11),r3         # Get the return data ptr
!       stob    g1,mr_status(r3)        # Plug return status code
!       st      g2,mr_rlen(r3)          # Set return packet size
#
        call    O$resume                # Resume online
        call    FS$resume               # Resume any cleanup in progress
c       apool_start();                  # Resume async activity
#
# --- Send status response
#
.fex90:
        mov     r10,g1                  # Complete this request
        call    K$comp
        b       .fex20
#
#**********************************************************************
#
#  NAME: FS$stop
#
#  PURPOSE:
#       To provide a means of locking out the file system updates from
#       the file system actively running.
#
#  DESCRIPTION:
#       This function will set a signal to the clean up task to tell it
#       to stop as soon as possible.  It will then wait until the cleanup
#       task signals that it went back to sleep.  When the peer call for
#       this function is called, it will note whether or not the cleanup
#       was active or would like to be active and will restart it.
#
#  CALLING SEQUENCE:
#       call FS$stop
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
FS$stop:
        ld      f_stop_cnt,r3           # Get the old stop count
        addo    1,r3,r3                 # Increment the stop indicator
        st      r3,f_stop_cnt
#
# --- Now check if the cleanup is running.  If it is, we have terminated
# --- it by setting the stop bit.  We now have to wait for the cleanup
# --- running to clear (if it is set).
#
.fss10:
        ld      f_cleanup_running,r3    # Get the running indicator
        cmpobe  FALSE,r3,.fss20         # Jif not running
#
c       TaskSetMyState(pcfscleanup);    # Set process to wait for fs cleanup signal
        call    K$xchang                # Wait until awakened
        b       .fss10                  # Try again
#
# --- At this point, the clean up has either terminated or wasn't running
# --- to start with.  The stop count will prevent it from running until we
# --- release it.  We are done.
.fss20:
        ret
#
#**********************************************************************
#
#  NAME: FS$resume
#
#  PURPOSE:
#       To provide a means of unlocking the file system updates.
#
#  DESCRIPTION:
#       This function will release and possibly restart the cleanup
#       task.
#
#  CALLING SEQUENCE:
#       call FS$resume
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
FS$resume:
        ld      f_stop_cnt,r3           # Get the old stop count
        subo    1,r3,r3                 # Decrement it
        st      r3,f_stop_cnt
#
# --- Check if the clean up task wanted to still run.  If so, clear
# --- the indicator and start it up.
#
        ld      f_cleanup_req_run,r4    # Get indicator
        cmpobe  FALSE,r4,.fsr10         # FALSE, resume
#
c       TaskReadyByState(pcfscleanup);  # Enable tasks waiting for file system cleanup
#
.fsr10:
        ret
#
#**********************************************************************
#
#  NAME: FS$cleanup
#
#  PURPOSE:
#       To provide a means of processing drives which have bad file
#       systems.
#
#  DESCRIPTION:
#       This task, when running will look for any drives that have
#       bad file systems and perform an update.  If the update is
#       successful, the status of the drive will be changed and the
#       file system report will be generated.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
FS$cleanup:
        ldconst FALSE,r14               # Set indicator that no fsys' were done
        b       .fcs20                  # Start a pass when first started
#
.fcs10:
        ldconst FALSE,r14               # Set indicator that no fsys' were done
#
        ldconst FALSE,r3                # Set running to false
        st      r3,f_cleanup_running
#
c       TaskSetMyState(pcfscleanup);    # Set process to wait for fs cleanup signal
        call    K$qxchang               # Give up processor
#
.fcs20:
        ldos    K_ii+ii_status,r4       # Get the flags
        bbc     iimaster,r4,.fcs10      # Skip if not master
#
        ldconst TRUE,r3
        st      r3,f_cleanup_running    # Indicate it is running
        st      r3,f_cleanup_req_run    # And we want to run (not done with all)
#
# --- Search through all of the PIDs and looks for any bad file systems
# --- If any are found and if the drive is now operable, update the FS
# --- and set the PDD to good FS.  Tell others about it.
#
        ldconst MAXDRIVES-1,r15         # Get index
#
.fcs30:
        ld      f_stop_cnt,r3           # Get the stop indicator
        cmpobne 0,r3,.fcs90             # We are being stopped
#
        ld      P_pddindx[r15*4],g3     # PDD
        cmpobe  0,g3,.fcs80             # Jif undefined
#
        ldob    pd_miscstat(g3),r13     # Get the misc status
        bbc     pdmbfserror,r13,.fcs80  # Jif no file system error
#
        ldob    pd_devstat(g3),r3       # Get the devstat of the device
        cmpobne pdop,r3,.fcs80          # If not operable, don't try to update
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldob    pd_flags(g3),r3
        bbs     pdbebusy,r3,.fcs80
.endif  # MODEL_7400
.endif  # MODEL_3000
#
        ldob    pd_class(g3),r3         # Get device class
        cmpobe  pdunlab,r3,.fcs80       # Jif unlabelled
#
        ld      K_ficb,r3               # Get system serial number from system
        ld      fi_vcgid(r3),r3
        ld      pd_sserial(g3),r4       # Get system serial number from drive
        cmpobne r3,r4,.fcs80            # Jif foreign drive
#
        ldconst 0,g4                    # Allow UpdateFS to be interrupted

c       r4 = GR_IsDriveRemoved((PDD *)g3);
        cmpobe  FALSE,r4,.fcs75
c fprintf(stderr,"%s%s:%u <GR>fsys-issuing uninterrupted fsupdate for pid=%x r4=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid,r4);
        ldconst 1,g4                    # Don't allow UpdateFS to be interupted
.fcs75:
        call    FS$UpdateFS             # Update the file system
        cmpobe  efuinterrupted,g0,.fcs90 # Jif interrupted by stop request
        cmpobne efugood,g0,.fcs80       # Jif failed
        cmpobe  FALSE,r4,.fcs78
        ldconst 1,g0                    # Delay for minimum time
        call    K$twait
.fcs78:
#
        ldconst TRUE,r14                # Fsys update occurred
#
.fcs80:
        subo    1,r15,r15               # Decrement
        cmpible 0,r15,.fcs30            # Jif more to do
c       GR_ResetAllDevMissFlags();
#
        ldconst FALSE,r3                # False means no update required
        st      r3,f_cleanup_req_run    # We no longer want to run
#
.fcs90:
        cmpobe  FALSE,r14,.fcs100       # Jif no changes made
        call    NV_P2Update             # Update NVRAM
#
        ldconst TRUE,g0                 # Master update
        PushRegs(r3)                    # Save register contents
        call    NV_SendFSys             # Report it
        PopRegsVoid(r3)                 # Restore registers
#
.fcs100:
c       TaskReadyByState(pcfscleanup);  # Enable tasks waiting for file system cleanup
        b       .fcs10
#
#**********************************************************************
#
#  NAME: f$fsysop
#
#  PURPOSE:
#       To provide a standard means of reading or writing the internal
#       file system.
#
#  DESCRIPTION:
#       This function will do either a read or write to the internal
#       file system.
#
#       For reads, any file which is readable for the given file will
#       provide the return data.  For writes, all devices will be written
#       and an error count will be returned to the caller.
#
#  CALLING SEQUENCE:
#       call f$fsysop
#
#  INPUT:
#       g0 - MRP.
#
#  OUTPUT:
#       g1 - error code
#       g2 - length of return packet
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
f$fsysop:
#
# --- Set up the return data area and the parm block.
#
        ld      mr_rptr(g0),r15         # Return data pointer
        ld      mr_ptr(g0),r14          # Parm block address
        ldob    mfs_op(r14),r13         # Get the file operation
#
        ldob    mfs_fid(r14),g0         # Get the File ID
        ld      mfs_buffptr(r14),g1     # Get the buffer pointer
        ldos    mfs_bcount(r14),g2      # Get the blcok count
        ldos    mfs_conf(r14),g3        # Get the confirmation
        ldos    mfs_offset(r14),g4      # Get the block offset
        lda     mfs_goodmap(r15),g5     # Get the goodmap address
        ld      mfs_pidmap(r14),g6      # Get the pidmap address
#
# --- Now, based upon the operation, do a read or a write
#
        cmpobe  mfsopwr,r13,.fs10       # Jif a write
#
# --- Do a multiple read
#
        call    FS$MultiRead            # Generate read request
        ldconst 0,r3
!       st      r3,mfs_good(r15)        # Save off good write count
!       st      r3,mfs_error(r15)       # Save off error count
        b       .fs90                   # Exit
#
# --- Do a multiple write to all drives
#
.fs10:
        call    FS$MultiWrite           # Generate write request
!       st      g1,mfs_good(r15)        # Save off good write count
!       st      g2,mfs_error(r15)       # Save off error count
#
.fs90:
        ldconst deok,g1                 # Assume success
        cmpobe  0,g0,.fs100             # Jif no error from read or write
#
        ldconst deioerr,g1              # Error
#
.fs100:
        ldconst mfsrsiz,g2              # Return size
        ret
#
#******************************************************************************
#
#  NAME: FS$VerifyDirectory
#
#  PURPOSE:
#       To provide a common method of reading up and verifying the directory
#       for the internal files system
#
#  DESCRIPTION:
#       This function will read up the first LBA on the device passed into
#       the function and will verify the CRC in the header.  It will then
#       read the entire file into memory and verify the data CRC.  If this
#       all passed, then TRUE will be returned, otherwise FALSE.
#
#  CALLING SEQUENCE:
#       call FS$VerifyDirectory
#
#  INPUT:
#       g0 - if one, DumpMemory() what is wrong.
#       g3 - PDD to verify
#
#  OUTPUT:
#       g0 = boolean (TRUE - verified correctly, FALSE - error in directory)
#
#  REGS DESTROYED:
#       g0
#
#******************************************************************************
#
FS$VerifyDirectory:
        PushRegs                        # Save all G registers (stack relative)
        cmpobe  0,g3,.fvd90             # Jif NULL
c       r8 = g0;
#
# --- Issue read to the directory LBA
#
        lda     O_t_rd,g0               # Pass standard inquiry template
        call    O$genreq                # Generate request
#
        ldconst SECSIZE,g0              # Allocate one block of data space
        call    fs$AllocSpace
        mov     g0,r4                   # Save buffer pointer
#
        ldconst 1,r3
        stob    r3,pr_cmd+8(g2)         # Do a one block read to LBA 0
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobne ecok,g0,.fvd90          # Jif error
#
# --- Now do a CRC check on the header.
#
c       g0 = MSC_CRC32((void *)r4,fshcrcsiz)  # Generate the CRC (into g0)
        ld      fsh_hcrc(r4),r5         # Get the CRC from the file
c       if (g0 != r5) {                 # CRC does not match from the file
c         fprintf(stderr,"%sfsys.as:%u FS$VerifyDirectory pid=%d HEADER CRC MISMATCH (%8.8lx != %8.8lx)\n", FEBEMESSAGE, __LINE__,((PDD*)g3)->pid, r5, g0);
c         if (r8 == 1) {
c           DumpMemory(r4, fshcrcsiz, 0);
c         }
          b       .fvd90                # Set error return from routine.
c       }
#
        ld      fsh_dcrc(r4),r5         # Get the data CRC
        ld      fsh_len(r4),r6          # Get the data length
        call    O$relreq                # Release it
#
# --- Now read up the entire file and CRC check it.
#
        ldconst SECSIZE-1,r3            # Round up the size
        addo    r3,r6,r3                # Add in the rounding factor
        ldconst SECSIZE,r4              # Set up to get sector and byte count
        divo    r4,r3,r3                # r3 is the number of sectors
        mulo    r4,r3,r4                # Get byte count required for buffer
#
        lda     O_t_rd,g0               # Get a new ILT/PRP/SGL
        call    O$genreq                # Generate the structures
#
        mov     r4,g0                   # Get buffer and SGL
        call    fs$AllocSpace
        mov     g0,r7                   # Save pointer
#
        stob    r3,pr_cmd+8(g2)         # Save block count in command
        ldconst 1,r3                    # Read LBA 1 (first directory block)
        stob    r3,pr_cmd+5(g2)         # Save block count in command
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobne ecok,g0,.fvd90          # Jif error
#
c       g0 = MSC_CRC32((void *)r7, r6); # Generate the CRC (into g0)
#
c       if (g0 == r5) {
          call    O$relreq              # Release the request
c         r4 = TRUE;                    # Return value
          b       .fvd100               # exit with true
c       }
#
c       fprintf(stderr,"%sfsys.as:%u FS$VerifyDirectory pid=%d DATA CRC MISMATCH (%8.8lx != %8.8lx)\n", FEBEMESSAGE, __LINE__,((PDD*)g3)->pid, r5, g0);
c       if (r8 == 1) {
c         DumpMemory(r7, r6, 512);
c       }
#
        call    O$relreq                # Release the request
#
.fvd90:
        ldconst FALSE,r4                # Fail
#
.fvd100:
        PopRegs                         # Restore g1 to g14 (stack relative)
        mov     r4,g0                   # Set return code
        ret
#
#******************************************************************************
#
#  NAME: FS_InitDirectory
#
#  PURPOSE:
#       To provide a common method of setting up the directory
#       for the internal files system
#
#  DESCRIPTION:
#       This function will write the first LBA on the device passed into
#       the function.  The directory is contained in this block.  It will
#       then write entries for the files that we currently know about.
#
#       It is anticipated that the list initialized will expand as time goes
#       on and new files are added.
#
#  CALLING SEQUENCE:
#       call FS_InitDirectory
#
#  INPUT:
#       g3 - PDD to initialize
#
#******************************************************************************
#
# C access
# void FS_InitDirectory(PDD* pPDD);
FS_InitDirectory:
        mov     g0,g3
        PushRegs                        # Save all G registers (stack relative)
#
        cmpobe  0,g3,.fid90             # Jif NULL
#
# --- Issue write to the first LBA of the directory itself rather than
# --- to the header.  This is done so that we can get the CRC for the
# --- data block first and then use it in the header.
#
        lda     O_t_wr,g0               # Pass standard inquiry template
        call    O$genreq                # Generate request
#
        ldconst (fiddirsiz-1)*SECSIZE,g0# Allocate one block of data space
        call    fs$AllocSpace
        mov     g0,r15                  # Save buffer pointer
#
# --- Fill in the directory entry.
#
        ldconst fsdirlen,r3             # Length of default directory
        lda     FS_dirname,r4           # Name string
#
.fid10:
        subo    4,r3,r3                 # Decrement pointer
        cmpibg  0,r3,.fid20             # Quit if done with string
#
        ld      (r4)[r3*1],r5           # Get the byte
        st      r5,(r15)[r3*1]          # Put it into the directory
        b       .fid10
#
# --- Write the directory file itself, less the header.  The header will
# --- be written after the CRC for the file is calculated.
#
.fid20:
        ldconst fiddirsiz-1,r3
        stob    r3,pr_cmd+8(g2)         # Write the whole directory file
        ldconst 1,r3
        stob    r3,pr_cmd+5(g2)         # To LBA 1
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobne ecok,g0,.fid90          # Jif error
#
# --- Now do a CRC generation on the data just written
#
        ldconst fsdirlen,r3             # Size to generate
c       r14 = MSC_CRC32((void *)r15,r3) # Generate the CRC (into g0)
#
# --- Now write down the header block.
#
        call    O$relreq                # Release the last request
#
        lda     O_t_wr,g0               # Pass standard inquiry template
        call    O$genreq                # Generate request
#
        ldconst SECSIZE,g0              # Allocate one block of data space
        call    fs$AllocSpace
        mov     g0,r15                  # Save buffer pointer
#
        ldconst fiddir,r3               # Set the FID
        st      r3,fsh_fid(r15)
        ldconst fsdirlen,r3             # Set the file size
        st      r3,fsh_len(r15)
        st      r14,fsh_dcrc(r15)       # Set the data CRC
#
        ldconst fsdnamelen,r3           # Length of directory name
        lda     FS_dirname,r4           # Name string
#
.fid30:
        subo    1,r3,r3                 # Decrement pointer
        cmpibg  0,r3,.fid40             # Quit if done with string
#
        ldob    (r4)[r3*1],r5           # Get the byte
        stob    r5,(r15)[r3*1]          # Put it into the directory
        b       .fid30
#
.fid40:
c       g0 = MSC_CRC32((void *)r15,fshcrcsiz)  # Get the CRC
        st      g0,fsh_hcrc(r15)        # Set the header CRC
#
        ldconst 1,r3
        stob    r3,pr_cmd+8(g2)         # Do a one block write
        ldconst 0,r3
        stob    r3,pr_cmd+5(g2)         # To LBA 0
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
#
.fid90:
        call    O$relreq                # Release the last request
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#******************************************************************************
#
#  NAME: fs$GetDirectoryEntry
#
#  PURPOSE:
#       To provide a common method of finding the starting LBA for a file
#       in the internal file system.
#
#  DESCRIPTION:
#       This function will read up the directory entry and return the starting
#       LBA of the file.  It will also validate the write to see if the entire
#       operation can be done in the file.
#
#  CALLING SEQUENCE:
#       call fs$GetDirectoryEntry
#
#  INPUT:
#       g0 - FID
#       g2 - Size in blocks being requested
#       g3 - PDD
#       g4 - Offset in blocks of start of operation
#
#  OUTPUT:
#       g0 = FID starting LBA (0xffffffff if not found)
#
#******************************************************************************
#
fs$GetDirectoryEntry:
#
# --- Save regs
#
        movq    g0,r12                  # Save them
#
# --- Validate the PDD
#
        cmpobe  0,g3,.fgd95             # Jif no device
#
# --- Determine the block number by offsetting into the file using FID.
#
        ldconst fsdsiz,r3               # Get the size of each entry
        mulo    r3,g0,r3                # Byte offset into file
        ldconst SECSIZE,r4              # Get size of sector
        divo    r4,r3,r5
        addo    1,r5,r5                 # r5 has block number
        remo    r4,r3,r4                # r4 has byte offset
#
# --- Issue read to the directory LBA
#
        lda     O_t_rd,g0               # Pass Zero byte read template
        call    O$genreq                # Generate request
#
        ldconst SECSIZE,g0              # Allocate one block of data space
        call    fs$AllocSpace
        mov     g0,r6                   # Save buffer pointer
#
        ldconst 1,r3
        stob    r3,pr_cmd+8(g2)         # Do a one block read to LBA 0
        stob    r5,pr_cmd+5(g2)         # Set the LBA number
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobne ecok,g0,.fgd95          # Jif error
#
# --- Get the file directory entry
#
        addo    r4,r6,r6                # File directory address
        ld      fsd_offset(r6),g0       # Offset
        ldos    fsd_count(r6),r3        # Count
#
        call    O$relreq                # Release the request
#
# --- Determine if the ending block offset is too large.
#
        addo    g4,r14,r4               # Get the max block to be used
        cmpobge r3,r4,.fgd100           # Jif too small
#
.fgd95:
        ldconst -1,g0                   # Error
#
.fgd100:
        mov     r13,g1                  # Restore g1-g3
        movl    r14,g2
        ret
#
#******************************************************************************
#
#  NAME: FS$WriteFile
#
#  PURPOSE:
#       To provide a common method of writing to the internal file system
#
#  DESCRIPTION:
#       This function will fetch the directory information for a file and
#       then write the data into the file.
#
#  CALLING SEQUENCE:
#       call FS$WriteFile
#
#  INPUT:
#       g0 - FID to write
#       g1 - Buffer address
#       g2 - Length in blocks
#       g3 - PDD to write
#       g4 - Block offset
#
#  OUTPUT:
#       g0 - Success or failure (-1 is failure, 0 is success)
#
#******************************************************************************
#
# C access
# UINT32 FS_WriteFile(UINT32 fid, void* pBuf, UINT32 len,
#                     PDD* pPDD, UINT32 offset);
        .globl  FS_WriteFile
FS_WriteFile:
FS$WriteFile:
        movq    g0,r8                   # Save g0-g3
        movq    g4,r12                  # Save g4-g7
#
        cmpobe  0,g3,.fwf110            # Jif NULL
#
# --- First get the block number and length of the file on the disk.
#
        call    fs$GetDirectoryEntry    # g0 - offset
        ldconst -1,r3
        cmpobe  g0,r3,.fwf110           # A directory problem, exit
        mov     g0,r7                   # Save the starting LBA offset
#
        lda     O_t_wr,g0               # Pass standard inquiry template
        call    O$genreq                # Generate request
#
        ldconst SECSIZE,r3              # Convert to bytes
        mulo    r3,r10,g0
        mov     r9,g3                   # Set pointer address
        call    fs$AllocWOSpace
#
        bswap   r10,r3                  # Convert endians
        shro    16,r3,r3
        stos    r3,pr_cmd+7(g2)         # Update 10 byte command length
#
        addo    r7,r12,r3
        bswap   r3,r3
        st      r3,pr_cmd+2(g2)         # Set the LBA
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobne ecok,g0,.fwf90          # Jif error
#
        ldconst 0,r3                    # Indicate no error
        b       .fwf100
#
.fwf90:
        ldconst -1,r3                   # Indicate error
#
# --- Free the SGL and the ILT/PRP
#
.fwf100:
        call    fs$FreeWOSpace          # Free the SGL
        call    O$relreq                # Release the request
#
.fwf110:
        movq    r8,g0                   # Restore g0-g3
        movq    r12,g4                  # Restore g4-g7
        mov     r3,g0                   # Return code
        ret
#
#******************************************************************************
#
#  NAME: FS$MultiWrite
#
#  PURPOSE:
#       To provide a common method of writing to the internal file system
#       on all disks.
#
#  DESCRIPTION:
#       This function will fork processes to write to all of the drives
#       concurrently.  It will return a count of successful writes and
#       of unsuccessful writes.
#
#  CALLING SEQUENCE:
#       call FS$MultiWrite
#
#  INPUT:
#       g0 - FID to write
#       g1 - Buffer address
#       g2 - Length in blocks
#       g4 - Block offset
#       g5 - Good map address
#
#  OUTPUT:
#       g0 - Success or failure (-1 is failure, 0 is success)
#       g1 - Count of successful writes
#       g2 - Count of unsuccessful writes
#
#******************************************************************************
#
# C access
# UINT32 FS_MultiWrite(UINT32 fid, void* pBuf, UINT32 len,
#                    void* mapAddr, UINT32 offset, UINT32 ppGoodCount, UINT32 ppErrCount);
#
        .globl  FS_MultiWrite
FS_MultiWrite:
        mov     g5,r10                  # Save pointers to counters
        mov     g6,r11                  # Save pointers to counters
        mov     g3,g5                   # Map addr
        call    FS$MultiWrite
        st      g1,(r10)                # Good count
        st      g2,(r11)                # Error count
        ret
#
FS$MultiWrite:
        movq    g0,r8                   # Save g0-g3
        movq    g4,r12                  # Save g4-g7
        ldconst FALSE,r7                # Indicate if a fsys update report reqd
#
# --- Allocate a multi-write record for this write.
#
c       g0 = s_MallocC(fsmsiz, __FILE__, __LINE__); # Get the record
        st      g5,fsm_goodmap(g0)      # Save the good map address
        mov     g0,g5                   # Pointer to multi-write record
#
# --- Write each operable PDD.
#
        ldconst MAXDRIVES,r6            # Max drives
        ldconst 0,r5                    # Index
#
.fmw10:
        ld      P_pddindx[r5*4],g3      # Get the PDD
        cmpobe  0,g3,.fmw20             # Try the next one
#
# --- Validate that the drive is usable.
#
        ldob    pd_devstat(g3),r3       # Get the device status
        cmpobne pdop,r3,.fmw15          # Jif not operable
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldob    pd_flags(g3),r3
        bbs     pdbebusy,r3,.fmw20
.endif  # MODEL_7400
.endif  # MODEL_3000
#
        ldob    pd_class(g3),r3         # Get device class
        cmpobe  pdunlab,r3,.fmw15       # Jif unlabelled
#
        ld      K_ficb,r3               # Get system serial number from system
        ld      fi_vcgid(r3),r3
        ld      pd_sserial(g3),r4       # Get system serial number from drive
        cmpobne r3,r4,.fmw20            # Jif foreign drive
#
# --- Set up the write parameters
#
        ld      fsm_active(g5),r3       # Bump active write counter
        addo    1,r3,r3
        st      r3,fsm_active(g5)       # Save it
#
        movl    r8,g6                   # Pass FID and buffer addr in g6/g7
                                        # Other parms are unchanged
        lda     fs$WriteProc,g0         # Fork a write process
        ldconst FSYSPRI,g1
c       CT_fork_tmp = (ulong)"fs$WriteProc";
        call    K$tfork
        b       .fmw20                  # Do next drive
#
# --- Check if we need to turn on file system error bit and send report
#
.fmw15:
        ldos    pd_poststat(g3),r3      # Get current status (post and dev)
        ldconst (pdinop<<8)+pdfdir,r4   # Check for inoperable and failed fsys
        cmpobe  r3,r4,.fmw20            # If no file system, send no report
#
        ldob    pd_miscstat(g3),r3      # Get misc status
        bbs     pdmbfserror,r3,.fmw20   # If already set, get next drive
#
        setbit  pdmbfserror,r3,r3       # Else set it
c fprintf(stderr, "%s%s:%u Setting pdmbfserror into miscstat for pid=%d miscstat=0x%02lx\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid,r3);
        stob    r3,pd_miscstat(g3)      # Save it
        ldconst TRUE,r7                 # Set the send report indicator
#
.fmw20:
        addo    1,r5,r5                 # Increment index
        cmpobne r5,r6,.fmw10            # Jif more drives
#
# --- Wait for them all to complete.
#
.fmw30:
        ld      fsm_active(g5),r3       # Get counter
        cmpobe  0,r3,.fmw90             # Jif done
#
        ldconst 10,g0                   # Timed wait 10 msec
        call    K$twait
        b       .fmw30                  # Check again
#
.fmw90:
        ldl     fsm_good(g5),r4         # Good and bad count
#
c       s_Free(g5, fsmsiz, __FILE__, __LINE__); # Free old record
#
        mov     r4,g1                   # Good count
        mov     r5,g2                   # Error count
        ldconst 0,g0                    # Prep return code
        cmpobe  0,g2,.fmw100            # Jif no errors
        ldconst -1,g0                   # Else error return
        ldconst TRUE,r7                 # Indicate to send report
#
.fmw100:
        cmpobe  FALSE,r7,.fmw110        # Jif no send reqd
        PushRegs(r4)                    # Save register contents
        ldconst FALSE,g0                # Indicate either master or slave
        call    NV_SendFSys             # Send it
        PopRegsVoid(r4)                 # Restore registers
#
        call    D$p2update              # Save the change in fsys error bits
#
.fmw110:
        mov     r11,g3                  # Restore g3
        movq    r12,g4                  # Restore g4-g7
        ret
#
#******************************************************************************
#
#  NAME: fs$WriteProc
#
#  PURPOSE:
#       To write to a given PDD and track for multi-write.
#       on all disks.
#
#  DESCRIPTION:
#       This function write the single disk and account for any errors
#       to the multi-write record passed in.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g2 - Length in blocks
#       g3 - PDD to write
#       g4 - Block offset
#       g5 - Multi-write record
#       g6 - FID to write
#       g7 - Buffer address
#
#  OUTPUT:
#       g5 record is modified
#
#******************************************************************************
#
fs$WriteProc:
        movl    g6,g0                   # Put FID and buffer addr in right parm
#
        call    FS$WriteFile            # Write the file
        cmpobe  0,g0,.fmp80             # If no error, indicate as such
#
# --- Now set the error indicator in the PDD.
#
        ldob    pd_miscstat(g3),r3      # Get the misc status
        bbs     pdmbfserror,r3,.fmp90   # Jif already set
#
        setbit  pdmbfserror,r3,r3
c fprintf(stderr, "%s%s:%u Setting pdmbfserror into miscstat for pid=%d miscstat=0x%02lx\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid,r3);
        stob    r3,pd_miscstat(g3)      # Set it
#
# --- This is an error.  Indicate such in the passed in record.
#
        ld      fsm_error(g5),r3        # Error
        addo    1,r3,r3
        st      r3,fsm_error(g5)        # Put into record
#
        b       .fmp90                  # Dec outstanding count and leave
#
# --- This is not an error.  Indicate such in the passed in record.
#
.fmp80:
        ld      fsm_good(g5),r3         # Good counter
        addo    1,r3,r3
        st      r3,fsm_good(g5)         # Put into record
#
# --- Set the bit in the good map to indicate this write was a success.
#
        ld      fsm_goodmap(g5),r5      # Get the map address
        cmpobe  0,r5,.fmp90             # Jif no map requested
#
        ldos    pd_pid(g3),r3           # Get the PID
        divo    8,r3,r4                 # Get the byte offset
        addo    r5,r4,r5                # Get the byte within the map
!       ldob    (r5),r6                 # Get the byte
        remo    8,r3,r4                 # Get the bit to set
        setbit  r4,r6,r6                # Set the bit for this PID
!       stob    r6,(r5)                 # Save it
#
.fmp90:
        ld      fsm_active(g5),r3       # Decrement the outstanding count
        subo    1,r3,r3
        st      r3,fsm_active(g5)       # Save it
        ret
#
#******************************************************************************
#
#  NAME: FS$ReadFile
#
#  PURPOSE:
#       To provide a common method of reading from the internal file system
#
#  DESCRIPTION:
#       This function will fetch the directory information for a file and
#       then read the data into the buffer.
#
#  CALLING SEQUENCE:
#       call FS$ReadFile
#
#  INPUT:
#       g0 - FID to read
#       g1 - Buffer address
#       g2 - Length in blocks
#       g3 - PDD to read
#       g4 - Block offset
#
#  OUTPUT:
#       g0 - Success or failure (-1 is failure, 0 is success)
#
#******************************************************************************
#
# C access
# UINT32 FS_ReadFile(UINT32 fid, void* pBuf, UINT32 len,
#                     PDD* pPDD, UINT32 offset);
        .globl  FS_ReadFile
FS_ReadFile:
FS$ReadFile:
        movq    g0,r8                   # Save g0-g3
        movq    g4,r12                  # Save g4-g7
#
        ldconst -1,r3                   # Indicate error
        cmpobe  0,g3,.frf110            # Jif NULL
#
# --- First get the block number and length of the file on the disk.
#
        call    fs$GetDirectoryEntry    # g0 - offset
        cmpobe  g0,r3,.frf110           # A directory problem, exit
        mov     g0,r7                   # Save the starting LBA offset
#
        lda     O_t_rd,g0               # Pass standard inquiry template
        call    O$genreq                # Generate request
#
        ldconst SECSIZE,r3              # Convert to bytes
        mulo    r3,r10,g0
        mov     r9,g3                   # Set pointer address
        call    fs$AllocWOSpace
#
        bswap   r10,r3                  # Convert endians
        shro    16,r3,r3
        stos    r3,pr_cmd+7(g2)         # Update 10 byte command length
#
        addo    r7,r12,r3
        bswap   r3,r3
        st      r3,pr_cmd+2(g2)         # Set the LBA
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobne ecok,g0,.frf90          # Jif error
#
        ldconst 0,r3                    # Indicate no error
        b       .frf100
#
.frf90:
        ldconst -1,r3                   # Indicate error
#
# --- Free the SGL and the ILT/PRP
#
.frf100:
        call    fs$FreeWOSpace          # Free the SGL
        call    O$relreq                # Release the request
#
.frf110:
        movq    r8,g0                   # Restore g0-g3
        movq    r12,g4                  # Restore g4-g7
        mov     r3,g0                   # Return code
        ret
#
#******************************************************************************
#
#  NAME: FS$MultiRead
#
#  PURPOSE:
#       To provide a common method of reading from the internal file system
#       from one of all the disks.
#
#  DESCRIPTION:
#       This function will issue two reads from the first two PDDs that
#       have not been marked as rejected for file system use.  If these two
#       files are identical, then one of them is returned.  If they do not
#       match, a third file is read and compared to the second.  This
#       continues until a match is made.  If no files match, a failure is
#       returned to the sender.
#
#  CALLING SEQUENCE:
#       call FS$MultiRead
#
#  INPUT:
#       g0 - FID to read
#       g1 - Buffer address
#       g2 - Length in blocks
#       g3 - Confirmation (is a double read necessary?)
#       g4 - Block offset
#       g6 - Pidmap address
#
#  OUTPUT:
#       g0 - Success or failure (-1 is failure, 0 is success)
#
#******************************************************************************
#
#  Stack frame definitions
#
        .set    f_nomapread,-40         # flag if read was from pdisk not in
                                        #   the pid bitmap
        .set    f_pidmap,-36            # bitmap of pids to read from
        .set    f_buffersize,-32        # offset for buffer size
        .set    f_buffer1addr,-28       # offset for buffer one address
        .set    f_buffer2addr,-24       # offset for buffer two address
        .set    f_nodevcheck,-20        # offset for don't use problem dev
# C access
# UINT32 FS_MultiRead(UINT32 fid, void* pBuf, UINT32 len,
#                    UINT32 confirm, UINT32 offset);
#
        .globl  FS_MultiRead
FS_MultiRead:
FS$MultiRead:
        lda     40(sp),sp               # allocate small stack frame
#
        movq    g0,r8                   # Save g0-g3
        mov     g5,r14                  # Save g5
        mov     g6,r15                  # Save g6
        st      r15,f_pidmap(sp)        # Store pidmap
        ldconst FALSE,r4                # Initialize f_nomapread
        st      r4,f_nomapread(sp)
        st      r4,f_nodevcheck(sp)
#
# --- Allocate DRAM space for the read buffer
#
        ldconst SECSIZE,r4              # Convert blocks to bytes
        mulo    g2,r4,g6                # g6 = length in bytes
        addo    0x10,g6,r12             # Add 16 bytes for alignment
        st      r12,f_buffersize(sp)    # Store buffer size
c       g0 = s_MallocW(r12, __FILE__, __LINE__); # Get buffer
        st      g0,f_buffer1addr(sp)    # Save buffer address
        and     0x0F,g0,r3              # Align start of buffer with CCB buffer
        and     0x0F,r9,r4
        subo    r3,r4,r5                # Difference of starting addresses
        and     0x0F,r5,r5
        addo    g0,r5,g0
        mov     g0,r6                   # Save read buffer
#
# --- Allocate DRAM space for the compare buffer
#
c       g0 = s_MallocW(r12, __FILE__, __LINE__); # Get buffer
        st      g0,f_buffer2addr(sp)    # Remember buffer address
        and     0x0F,g0,r3              # Align compare buffer with CCB buffer
        subo    r3,r4,r5
        and     0x0F,r5,r5
        addo    g0,r5,g0
        mov     g0,r7                   # Save compare buffer
#
# --- Set up constructs to read into 2 buffers for comparison.  R5 will
# --- determine which buffer gets read into next.  When we read identical
# --- files, the file pointed to by r6 will be returned to caller.  r12 will
# --- will increment with each read and when we have 2 files, we will compare.
#
.fmr10:
        ldconst 0,r12                   # No files read yet
        ldconst 0,r5                    # Read into the read buffer first
#
# --- Start at the first operable PDD.  We will loop through all of them
# --- until we get success.  If none can read the requested fid,
# --- then failure is returned.
#
        ldconst 0,r13                   # Index into PDD
#
.fmr20:
        ld      P_pddindx[r13*4],g3     # Get the PDD
        cmpobe  0,g3,.fmr50             # Try the next one
#
# --- Validate that the drive is usable.
#
        ldob    pd_devtype(g3),r3       # Get the device type
        cmpobl  pddtmaxdisk,r3,.fmr50   # Jif not a drive

        ldob    pd_devstat(g3),r3       # Get the device status
        cmpobne pdop,r3,.fmr50          # Jif not operable
#
        ldob    pd_class(g3),r3         # Get device class
        cmpobe  pdunlab,r3,.fmr50       # Jif unlabelled
#
        ld      f_nodevcheck(sp),r3
        cmpobne 0,r3,.fmr22

        ld      pd_dev(g3),r3           # get dev
        cmpobe  0,r3,.fmr50             # dev == NULL

        ldos    dv_wait(r3),r4
        cmpobne 0,r4,.fmr50             # wait is set. drive is having issues.
#
        ld      dv_failq_hd(r3),r4
        cmpobne 0,r4,.fmr50             # IO already in retry state
.fmr22:
#
        ld      K_ficb,r3               # Get system VCG from system
        ld      pd_sserial(g3),r4       # Get system serial number from drive
        ld      fi_vcgid(r3),r3
        cmpobne r3,r4,.fmr50            # Jif foreign drive
#
# --- See if this drive is BUSY due to ISE event
#
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldob    pd_flags(g3),r3
        bbs     pdbebusy,r3,.fmr50
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- See if this drive was failed by any controller
#
        ldob    pd_miscstat(g3),r3      # Get miscellaneous status
        bbs     pdmbfserror,r3,.fmr50   # If drive failed, go to next PDD
#
# --- See if this drive is in the pid bitmap.  If pidmap address is zero, then
#     it doesn't matter.
#
        cmpobe  0,r15,.fmr25            # Jif pidmap addr is zero
        divo    8,r13,r4                # Get the byte offset
        addo    r15,r4,r4               # Get the byte within the map
!       ldob    (r4),r3                 # Get the byte
        remo    8,r13,r4                # Get the bit to check
        bbc     r4,r3,.fmr50            # Jif drive not in pidmap
#
# --- Set up the read parameters
#
.fmr25:
        mov     r8,g0                   # Restore FID
        cmpobe  0,r5,.fmr30             # Which buffer do we read into this time
        mov     r7,g1                   # Read to the compare buffer
        b       .fmr40
#
.fmr30:
        mov     r6,g1                   # Read to the read buffer
#
.fmr40:
        call    FS$ReadFile             # Do the read
        cmpobne 0,g0,.fmr50             # Jif Read failure
#
# --- If a successful read, see if a confirmation was requested, if not exit successfully
#
        cmpobe  0,r11,.fmr90            # Exit with a success
#
# --- If a successful read, check to see if we have read two files and can compare now
#
        notbit  0,r5,r5                 # Toggle where the next file is read into
        addo    1,r12,r12               # Increment the total number of files read
        cmpobg  2,r12,.fmr50            # Do we have two files to compare yet?
#
# --- Compare both files now and if they match, send the file in the read buffer
# --- back to the CCB.
#
        mov     g4,r3                   # Save Block offset
        mov     r7,g5                   # Load buffer pointers
        mov     r6,g4
c       g0 = !memcmp((void*)g4, (void*)g5, g6);
        mov     r3,g4                   # Restore Block offset
        cmpobe  TRUE,g0,.fmr90          # If they are identical, exit a success
        subo    1,r12,r12               # Force another file read
#
.fmr50:
        addo    1,r13,r13               # Increment index
        ldconst MAXDRIVES,r4
        cmpobne r13,r4,.fmr20           # Jif more drives
#
# --- Not enough drives were read.  First turn off the check for questionable devices
#     and try again after the second pass through the loop and we are still having
#     problems go a third time if a pid bitmap was specified, try reading again without
# --- the bitmap, but return a 'denomapread' if successful.
#
        ld      f_nodevcheck(sp),r3     # already turned off
        cmpobne 0,r3,.fmr52             # jump to pid bitmap logic
        ldconst TRUE,r3                 # set flag
        st      r3,f_nodevcheck(sp)
        b       .fmr10                  # Try again (pass 2)
#
.fmr52:
        cmpobe  0,r15,.fmr55            # Jif no pid bitmap
        ldconst 0,r15                   # r15 = NULL
        ldconst TRUE,r3                 # Set f_nomapread
        st      r3,f_nomapread(sp)
        b       .fmr10                  # Try again (pass 3)
#
# --- No drives were read or there were no matches.  In this case, parse back
# --- through the list of drives and turn off any FSys error bits.  If we
# --- turned off any, retry the reads.  If not, then we got a real bad error
# --- and should drop out.
#
.fmr55:
        ldconst FALSE,r5                # No drive changed
        ldconst MAXDRIVES-1,r13         # Index into PDD
#
.fmr60:
        ld      P_pddindx[r13*4],g3     # Get the PDD
        cmpobe  0,g3,.fmr70             # Try the next one
#
# --- Validate that the drive is usable.
#
        ldob    pd_devtype(g3),r3       # Get the device type
        cmpobl  pddtmaxdisk,r3,.fmr70   # Jif not a drive

        ldob    pd_devstat(g3),r3       # Get the device status
        cmpobne pdop,r3,.fmr70          # Jif not operable
#
        ldob    pd_class(g3),r3         # Get device class
        cmpobe  pdunlab,r3,.fmr70       # Jif unlabelled
#
        ld      K_ficb,r3               # Get system VCG from system
        ld      pd_sserial(g3),r4       # Get system serial number from drive
        ld      fi_vcgid(r3),r3
        cmpobne r3,r4,.fmr70            # Jif foreign drive
#
# --- Skip the drives in BUSY due to ISE event
#
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldob    pd_flags(g3),r3
        bbs     pdbebusy,r3,.fmr70
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- See if this drive was failed by any controller
#
        ldob    pd_miscstat(g3),r3      # Get miscellaneous status
        bbc     pdmbfserror,r3,.fmr70   # If drive failed, go to next PDD
#
        clrbit  pdmbfserror,r3,r3       # Clear it
        stob    r3,pd_miscstat(g3)      # Save miscellaneous status
#
        ldconst 1,r11                   # Change to a confirmed read for safety
        ldconst TRUE,r5                 # Indicate bits changed
#
.fmr70:
        subo    1,r13,r13               # Decrement index
        cmpible 0,r13,.fmr60            # Jif more to do
#
        ld      f_pidmap(sp),r15        # Restore pid bitmap
        cmpobne TRUE,r5,.fmr75          # End if no pid changed any status
        ldconst FALSE,r3                # Reset f_nomapread to FALSE
        st      r3,f_nomapread(sp)
        b       .fmr10
#
.fmr75:
#
# --- If a double read was requested, but only one file was
# --- able to be read, return the file and a success.
#
        cmpobe  0,r11,.fmr80            # Jif confirm not on
        cmpobne 0,r12,.fmr90            # Jif files read
#
.fmr80:
        ldconst -1,r8                   # Error out
        b       .fmr100
#
# --- Success
#
.fmr90:
#
# --- Copy the file in the read buffer over to the CCB buffer
#
        mov     g6,g3                   # Load length in bytes
        mov     r6,g4                   # Load src ptr = read buffer
        mov     r9,g5                   # Load dest ptr = compare buffer
c       if (g3 != 0 ) memcpy((void*)g5, (void*)g4, g3);
#
        ldconst 0,r8                    # Success, just exit
#
.fmr100:
#
# --- Release the read buffers
#
        ld      f_buffer1addr(sp),g0    # Restore read buffer pointer
        ld      f_buffersize(sp),g1     # Restore buffer length
c       s_Free(g0, g1, __FILE__, __LINE__); # Free the read buffer
#
        ld      f_buffer2addr(sp),g0    # Restore compare buffer pointer
        ld      f_buffersize(sp),g1     # Restore buffer length
c       s_Free(g0, g1, __FILE__, __LINE__); # Free the compare buffer
#
        lda     -32(sp),sp              # Restore stack pointer
#
        movq    r8,g0                   # Restore g0-g3 - r8 was overwritten
                                        #   earlier as the return code
        mov     r14,g5                  # Restore g5
        mov     r15,g6                  # Restore g6
        ret
#
#******************************************************************************
#
#  NAME: fs$AllocSpace
#
#  PURPOSE:
#       To provide a common method of allocating an SGL and clearing the
#       buffer for reads or writes.
#
#  DESCRIPTION:
#       This function will take the PRP and ILT and allocate the SGL for
#       the operation.  This is done to remove the requirement for different
#       templates for multiple sized IO operations.
#
#  CALLING SEQUENCE:
#       call fs$AllocSpace
#
#  INPUT:
#       g0 - space required in bytes
#       g1 - ILT
#       g2 - PRP
#
#  OUTPUT:
#       g0 = buffer pointer
#
#  REGS DESTROYED:
#       g0
#
#******************************************************************************
#
fs$AllocSpace:
        cmpobe  0,g0,.as100              # Jif none requested
#
# --- Allocate combined SGL/buffer
#
        st      g0,pr_rqbytes(g2)
#
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        st      g0,pr_sglptr(g2)        # Link SGL to PRP
#
        ld      sg_size(g0),r3          # Set up size of SGL
        setbit  31,r3,r3                # Indicate as borrowed
        st      r3,pr_sglsize(g2)
#
        call    M$clrsgl                # Clear buffer
#
        ld      sg_desc0+sg_addr(g0),g0 # Return buffer address
#
.as100:
        ret
#
#******************************************************************************
#
#  NAME: fs$AllocWOSpace
#
#  PURPOSE:
#       To provide a common method of allocating an SGL given a preallocated buffer.
#
#  DESCRIPTION:
#       This function will take the PRP and ILT and allocate the SGL for
#       the operation.
#
#  CALLING SEQUENCE:
#       call fs$AllocWOSpace
#
#  INPUT:
#       g0 - space required in bytes
#       g1 - ILT
#       g2 - PRP
#       g3 - Buffer pointer
#
#  OUTPUT:
#       None
#
#******************************************************************************
#
fs$AllocWOSpace:
        cmpobe  0,g0,.ag100              # Jif none requested
#
# --- Allocate SGL
#
        st      g0,pr_rqbytes(g2)
#
        call    M$asglwobuf             # Allocate combined SGL/buffer
        st      g0,pr_sglptr(g2)        # Link SGL to PRP
#
        ld      sg_size(g0),r3          # Set up size of SGL
        setbit  31,r3,r3                # Indicate as borrowed
        st      r3,pr_sglsize(g2)
#
.ag100:
        ret
#
#******************************************************************************
#
#  NAME: fs$FreeWOSpace
#
#  PURPOSE:
#       To provide a common method of freeing an SGL given a preallocated buffer.
#
#  DESCRIPTION:
#       This function will take the PRP and ILT and free the SGL for
#       the operation.
#
#  CALLING SEQUENCE:
#       call fs$FreeWOSpace
#
#  INPUT:
#       g2 - PRP
#
#  OUTPUT:
#       None
#
#******************************************************************************
#
fs$FreeWOSpace:
#
# --- Free SGL
#
        ld      pr_sglptr(g2),g0        # Get SGL ptr
        call    M$rsglwobuf             # Allocate combined SGL/buffer
#
        ldconst 0,r3                    # Indicate as borrowed
        st      r3,pr_sglptr(g2)
        ret
#
#******************************************************************************
#
#  NAME: FS$UpdateFS
#
#  PURPOSE:
#       To update a device with the latest version of all the internal files.
#
#  DESCRIPTION:
#       This function will update a drives internal file system by first
#       finding a file system on another drive that is up-to-date, then
#       transferring the entire file system over.
#
#  CALLING SEQUENCE:
#       call FS$UpdateFS
#
#  INPUT:
#       g3 - PDD
#       g4 - 0: allow fs stop interrupt, 1: do not allow fs stop interrupt
#
#  OUTPUT:
#       g0 - Success or failure (efugood is success,
#                                efuinvalidpdd;  'To' PDD is zero,
#                                efuinoppdd;     'To' PDD is Inoperable,
#                                efubadwrite;    'Bad write to 'To' PDD,
#                                efubadread;     No good 'From' PDDs or reads failed,
#                                efubadlblread;  Failed reading 'To' PDDs label file,
#                                efubadlblwrite; Failed writing 'To' PDDs label file,
#                                efuinterrupted; Interrupted with stop, need to retry)
#
#  REGS DESTROYED:
#       None
#
#******************************************************************************
#
# Stack variables
#
        .set    fsu_retval,0            # Return value (g0 overwritten)
        .set    fsu_pdd,12              # PDD passed in (g3 - do not modify)
        .set    fsu_logcode,32          # Log event code to use for logs
        .set    fsu_bigbuffer,36        # Buffer address of big buffer
        .set    fsu_srcwwn,40           # Source WWN
#
# C access
# UINT32 FS_UpdateFS(PDD* pPDD);
#
        .globl  FS_UpdateFS
FS_UpdateFS:
        movl    g0,g3
#       NOTE: fall through
FS$UpdateFS:
#
# --- Save global registers
#
        mov     sp,r15                  # Create stack frame
        lda     64(sp),sp
        stq     g0,(r15)                # save g0-g3
        stq     g4,16(r15)              # save g4-g7
#
        movq    0,r4
        st      r4,fsu_retval(r15)      # Assume good return
        stq     r4,fsu_logcode(r15)     # Clear the variables
        stq     r4,fsu_logcode+16(r15)  # Clear the variables
#
# --- Check if we are master.  If not, do not perform the update.
#
        ldos    K_ii+ii_status,r4       # Get the flags
        bbc     iimaster,r4,.udfs130    # Skip reading of the file if not master
#
        mov     g3,r14                  # For use through function
#
# --- Check if this PDD is ready for an update
#
        ldconst efuinvalidpdd,r3        # Preload invalid PDD return code
        st      r3,fsu_retval(r15)
#
        ldconst mlefsupdfail,r3         # Preload log code into stack
        st      r3,fsu_logcode(r15)
#
        cmpobe  0,r14,.udfs120          # If 'To' PDD is zero, fail
#
        ldos    pd_poststat(r14),r3     # Get current status (post and dev)
        ldconst (pdinop<<8)+pdfdir,r4   # Check for inoperable and failed fsys
        cmpobe  r4,r3,.udfs10           # Jif no file system found
#
        ldconst (pdinop<<8)+pdfdevlab,r4# Check for failed label
        cmpobe  r4,r3,.udfs10           # Jif no file system found
#
        ldconst efuinoppdd,r4           # Preload inoperative PDD return code
        st      r4,fsu_retval(r15)
#
        ldob    pd_devstat(r14),r3      # Get the device status
        cmpobne pdop,r3,.udfs120        # Jif not operable
#
# --- If PDD is BUSY, mark the PID for future FS repair/update
#
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldob    pd_flags(r14),r3
        bbc     pdbebusy,r3,.udfs9
        ldconst efubadlblwrite,r3       # Preload log code into stack
        st      r3,fsu_logcode(r15)

        ldob    pd_miscstat(r14),r3     # Get the misc status
        setbit  pdmbfserror,r3,r3       # Set file system needs repair
        stob    r3,pd_miscstat(r14)     # Save it
c fprintf(stderr,"%s%s:%u <PDD-BUSY>UpdateFS--pid=%x is in busy state, miscstat=0x%02lx..\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)r14)->pid, r3);
        b       .udfs130
.udfs9:
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- Allocate buffer space for the transfer.
#
.udfs10:
        ldconst DSKSALLOC,r9            # Prepare Transfer Length for r/w cmds
        bswap   r9,r9
        shro    16,r9,r9
#
c       g0 = s_MallocW(DSKBALLOC, __FILE__, __LINE__); # Get buffer
        st      g0,fsu_bigbuffer(r15)   # Save buffer address
#
# --- Find a PDD that's operable.  We will start the search for a good drive
# --- on one of the first 16 drives.  Make sure we start on one past the mod
# --- 16 of the PID.  This prevents you from starting on yourself.
#
        ldos    pd_pid(r14),r12         # Index into PDD
        addo    1,r12,r12               # Prevent from starting on yourself
        and     0xF,r12,r12             # Get LS nibble
#
.udfs20:
        ld      P_pddindx[r12*4],r8     # Get PDD pointer
        cmpobe  0,r8,.udfs50            # If NULL, check next pointer
#
        cmpobe  r8,r14,.udfs50          # Jif PDD is the PDD we are updating
#
        ldob    pd_devstat(r8),r3       # Get the device status
        cmpobne pdop,r3,.udfs50         # Jif not operable
#
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldob    pd_flags(r8),r3
        bbs     pdbebusy,r3,.udfs50
.endif  # MODEL_7400
.endif  # MODEL_3000
#
        ldob    pd_class(r8),r3         # Get device class
        cmpobe  pdunlab,r3,.udfs50      # Jif unlabelled
#
        ld      K_ficb,r3               # Get system VCG from system
        ld      pd_sserial(r8),r4       # Get system serial number from drive
        ld      fi_vcgid(r3),r3
        cmpobne r3,r4,.udfs50           # Jif foreign drive
#
# --- Check to see if drive was failed by another controller
#
        ldob    pd_miscstat(r8),r3      # Get miscellaneous status
        bbs     pdmbfserror,r3,.udfs50  # If drive failed, go to next PDD
#
# --- Good device.  Find the location of the label and record it.  Then do
# --- a repetitive read/write to modify the drive being updated.  If the sector
# --- with the label is found, either replace it, or update it based upon
# --- whether or not we have a valid label in memory.
#
        ldconst fidlabel,g0             # Label file
        ldconst LABELSIZE,g2            # Size in blocks
        mov     r8,g3                   # Put PDD in g3
        mov     g4,r13                  # Save g4
        ldconst 1,g4                    # Block one of the label file
        call    fs$GetDirectoryEntry    # Get the block number in the buffer
        mov     r13,g4                  # Restore g4
        mov     g0,r13                  # Label location on this drive
#
        ldconst -1,r3
        cmpobe  r3,r13,.udfs50          # Jif error while getting label location
#
        ldl     pd_wwn(r8),r4           # Get source WWN
        stl     r4,fsu_srcwwn(r15)
#
        ldconst 0,r10                   # r10 - LBA of current read/write
#
# --- Read 0x800 blocks from it.
#
.udfs30:
        cmpobne 0,g4,.udfs35            # We are not being stopped
        ld      f_stop_cnt,g0           # Get the stop indicator
        cmpobe  0,g0,.udfs35            # We are not being stopped
        ldconst efuinterrupted,r3       # Load interrupted return code
        st      r3,fsu_retval(r15)      # Set error return
        ldconst mlefsupdate,r3          # Get log code
        st      r3,fsu_logcode(r15)
        b       .udfs110                # Free mem and exit with interrupted status
#
.udfs35:
        mov     r8,g3                   # Pass PDD to read from
        lda     O_t_rd,g0               # Pass Read Extended template
        call    O$genreq                # Generate request
#
        ldconst DSKBALLOC,g0            # Size of transfer in bytes
        ld      fsu_bigbuffer(r15),g3   # Set pointer address
        call    fs$AllocWOSpace
#
        stos    r9,pr_cmd+7(g2)         # Update Transfer Length
#
        bswap   r10,r3
        st      r3,pr_cmd+2(g2)         # Set the LBA
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        mov     g0,r4                   # Save request status
#
# --- Free the SGL and the ILT/PRP
#
        call    fs$FreeWOSpace          # Free the SGL
        call    O$relreq                # Release the request
#
# --- If Read failed, go to the next PDD
#
        cmpobne ecok,r4,.udfs50         # Read from next PDD
#
# --- Check if the label is in this section.  If so, modify this one with the
# --- values we need.
#
        cmpobl  r13,r10,.udfs40         # Jif we are not in the range
        subo    r10,r13,r3              # Get the offset from the start of I/O
        ldconst DSKSALLOC,r4            # Get the size of this I/O
        cmpobge r3,r4,.udfs40           # Jif not in range
#
# --- This is the I/O with the label, get the label pointer and update the
# --- label fields.
#
        ldconst SECSIZE,r4              # Get the offset
        addo    1,r3,r3                 # Account for zero offset LBA
        mulo    r4,r3,r3                # r3 is the offset to the label
        ld      fsu_bigbuffer(r15),r4   # Get address plus offset
        addo    r3,r4,r3                # Pointer to label
#
        ldob    pd_class(r14),r4        # Get old device class
        stob    r4,xd_class(r3)         # Set up device class
#
        ld      pd_wwn(r14),r4          # Get WWN
        st      r4,xd_wwn(r3)           # Set it in the label
        ld      pd_wwn+4(r14),r4        # Get WWN
        st      r4,xd_wwn+4(r3)         # Set it in the label
#
        ld      pd_dname(r14),r4        # Get position information
        st      r4,xd_dname(r3)         # Set it in the label
#
        ld      xd_sserial(r3),r4       # Get the system serial number
        st      r4,pd_sserial(r14)      # Save it
#
# --- Write 0x800 blocks to device
#
.udfs40:
        mov     r14,g3                  # Pass updating PDD
        lda     O_t_wr,g0               # Pass standard inquiry template
        call    O$genreq                # Generate request
#
        ldconst DSKBALLOC,g0            # Size of transfer in bytes
        ld      fsu_bigbuffer(r15),g3   # Set pointer address
        call    fs$AllocWOSpace
#
        stos    r9,pr_cmd+7(g2)         # Update Transfer Length
#
        bswap   r10,r3
        st      r3,pr_cmd+2(g2)         # Set the LBA
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        mov     g0,r4                   # Save request status
#
# --- Free the SGL and the ILT/PRP
#
        call    fs$FreeWOSpace          # Free the SGL
        call    O$relreq                # Release the request
#
# --- If Write failed, return a failure to the Update and write failed label
#
        cmpobne ecok,r4,.udfs70         # Jif write was unsuccessful
#
# --- Successful write, increment LBA pointer
#
        ldconst DSKSALLOC,r3
        addo    r3,r10,r10              # Increment byte transfer size
        ldconst FSUPDATELEN,r3
        cmpobg  r3,r10,.udfs30          # Jif if not completed
        b       .udfs80                 # Success!
#
.udfs50:
        addo    1,r12,r12               # Bump the index
        ldconst MAXDRIVES,r3            # Get table size
        cmpobne r3,r12,.udfs60          # If not done, check next
        ldconst 0,r12                   # Start at the first drive
#
.udfs60:
        ldos    pd_pid(r14),r3          # Index into PDD
        addo    1,r3,r3                 # Prevent from starting on yourself
        and     0xF,r3,r3               # Get LS nibble
        cmpobne r3,r12,.udfs20          # Not done

        ldconst efubadread,r3           # Load bad read return code
        st      r3,fsu_retval(r15)      # Set error return
        b       .udfs90                 # Exit with failure
#
# --- Write failed label to the drive due to a failed write.
#
.udfs70:
        ldconst xdfailfs,g2             # Failure type
        mov     r14,g3                  # Failed drive
        call    O$writefailedlabel      # Write the label
#
        ldconst efubadwrite,r3          # Indicate bad write
        st      r3,fsu_retval(r15)
#
        b       .udfs110                # Common exit code
#
# --- Update good.  Clean up and exit.
#
.udfs80:
        ldconst efugood,r3              # Return code
        st      r3,fsu_retval(r15)
#
.udfs90:
        ldconst mlefsupdate,r3          # Get log code
        st      r3,fsu_logcode(r15)
#
        ldob    pd_miscstat(r14),r4     # Get old misc status
        clrbit  pdmbfserror,r4,r4       # Clear the fsys error
        stob    r4,pd_miscstat(r14)     # Save it
#
# --- Deallocate the dram
#
.udfs110:
        ld      fsu_bigbuffer(r15),g0   # Buffer address
c       s_Free(g0, DSKBALLOC, __FILE__, __LINE__); # Free the buffer
#
# --- Log a message showing success/failure and devices involved
#
.udfs120:
        ld      fsu_retval(r15),r3      # Get the return value
        cmpobe  efuinterrupted,r3,.udfs130 # Do not log interrupted

# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r3,efu_status(g0)       # Save Update FS status
        ld      fsu_logcode(r15),r3     # Get the code
        st      r3,mle_event(g0)        # Store event as word to clear other bytes
        ldl     pd_wwn(r14),r10         # Get WWN of device being updated (r10, r11)
        stl     r10,efu_wwnto(g0)       # Save WWN (r10, r11)
        ldl     fsu_srcwwn(r15),r10     # Get WWN of device updating from (r10, r11)
        stl     r10,efu_wwnfrom(g0)     # Save WWN (r10, r11)
        ldos    pd_pid(r14),r10         # Get the PID of updated device
        stos    r10,efu_pidto(g0)       # Save the PID of updated device
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], efulen);
#
# --- Restore global registers
#
.udfs130:
        ldq     (r15),g0                # Restore g0-g3 (g0 overwritten w/ ret)
        ldq     16(r15),g4              # Restore g4-g7
        ret                             # No stack adjustment needed
#
#**********************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
