# $Id: dlmfe.as 159300 2012-06-16 04:47:51Z m4 $
#**********************************************************************
#
#  NAME: dlmfe.as
#
#  PURPOSE:
#
#       To provide support for the Data-link Manager logic which
#       supports XIOtech Controller-to-XIOtech Controller functions
#       and services for Fibre communications.
#
#  FUNCTIONS:
#
#       DLM$init       - Data-link Manager initialization
#
#       This module employs 3 permanent processes:
#       dlm$besrp     - Handle BEP SRP requests (from BE DLM)
#       dlm_pollcntrl - Poll requested paths to other controllers
#       dlm$drp       - Handle DRP requests
#
#  Copyright (c) 1999-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
.ifdef TASK_STRESS_TEST
c static int Task_Torture_initialized;
.endif  # TASK_STRESS_TEST

#
# --- local equates ---------------------------------------------------
#
        .set    POLLCNTRL_INTERVAL,1000 # 1 second Poll Controller interval
        .set    POLLCNTRL_RETRY_CNT,1   # 1 times do nothing for problem paths
                                        #  (which means try everytime - was at
                                        #  ten, but lost all paths too often)
        .set    GET_CONTROLLER_NAME_INTERVAL,1000 # 1 second interval
#
# --- global function declarations ------------------------------------
#
.if     MAG2MAG
        .globl  DLM$init                # Module initialization
.endif  # MAG2MAG
        .globl  DLM$quesrp              # Queue SRP from the Back End Processor
        .globl  DLM$quedrp              # Queue DRP from the CCB/Cache
        .globl  DLM$SetFibreHeartbeatList # Set the FE Fibre Heartbeat List
        .globl  DLM$PortReady           # FE Port is now Ready Notification
        .globl  DLM$queryFEcomm         # Query FE Controller Communications
        .globl  DLM_MirrorPartnerFECommAvailable  # "C" function to ASM modules
        .globl  DLM$StartFECheckingMP   # Start Checking FE Comm to the MP
#
# --- global usage data definitions -----------------------------------
#
#
# --- DLM resident data definitions
#
.if     MAG2MAG
        .globl  DLM_servers             # Datagram services handler table
.endif  # MAG2MAG
#
        .data
        .align  3
#
#   BE SRP handler process queue structures
#
dlm_srpexec_qht:
        .space  8,0                     # Queue head/tail
dlm_srpexec_pcb:
        .word   0                       # SRP Executive PCB
dlm_srpexec_cqd:
        .word   0                       # SRP current queue depth
#
#   DRP handler process queue structures
#
dlm_drpexec_qht:
        .space  8,0                     # Queue head/tail
dlm_drpexec_pcb:
        .word   0                       # DRP Executive PCB
dlm_drpexec_cqd:
        .word   0                       # DPR current queue depth

dummyvar:       .word   DLM$quesrp      # Needed to prevent C compiler from
                                        # optimizing out the routine!
#
# --- Datagram services handler table
#
.if MAG2MAG
DLM_servers:
        .ascii  "DLM1"
        .word   dlm$DLM1                # DLM services #1 (FE)
        .ascii  "CAC0"
        .word   dlm$fwd_cache           # Forward the datagram to Cache
#
        .word   0                       # end of table indication
#
# --- BSS section
#
        .section end,bss
#
#**********************************************************************
#
# --- executable code -------------------------------------------------
#
#**********************************************************************
#
#  NAME: DLM$init
#
#  PURPOSE:
#       To provide a means of initializing this module.
#
#  DESCRIPTION:
#       The executive process for this module is
#       established and made ready for execution.
#
#  CALLING SEQUENCE:
#       call    DLM$init
#
#  INPUT:
#       g3 = FICB
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0
#       g1
#
#**********************************************************************
#
        .text
DLM$init:
        mov     0,g13                   # Ensure g13 = 0 for processes
#
# --- Initialize any structures that need to be
#
        movq    0,r12
        st      r12,dlm_srpexec_cqd     # Zero out the BEP SRP Queue Depth
        stl     r12,dlm_srpexec_qht     # Zero out the BEP SRP Queue
        st      r12,dlm_drpexec_cqd     # Zero out the DRP Queue Depth
        stl     r12,dlm_drpexec_qht     # Zero out the DRP Queue
#
# --- Establish DLM VRP executive process
#
        lda     dlm$vrpx,g0             # Establish VRP executive process
        ldconst DLMVEXECPRI,g1
c       CT_fork_tmp = (ulong)"dlm$vrpx";
        call    K$fork
        st      g0,DLM_vrp_qu+qu_pcb    # Save PCB
#
# --- Establish DLM datagram retry process
#
        lda     dlm$retrydg,g0          # Establish datagram retry process
        ldconst DLMRETRYDGPRI,g1
c       CT_fork_tmp = (ulong)"dlm$retrydg";
        call    K$fork
#
# --- Establish DLM Poll Controller process
#
        lda     dlm$pollcntrl,g0        # Establish a poll controller process
        ldconst DLMPOLLCNTRL,g1
c       CT_fork_tmp = (ulong)"dlm$pollcntrl";
        call    K$fork
#
# --- Establish the BE SRP executive process
#
        lda     dlm$besrp,g0            # Establish the BE SRP executive process
        ldconst DLMBESRPPRI,g1
c       CT_fork_tmp = (ulong)"dlm$besrp";
        call    K$fork
        st      g0,dlm_srpexec_pcb      # Save the PCB
#
# --- Establish the DRP executive process
#
        lda     dlm$drp,g0              # Establish the DRP executive process
        ldconst DLMDRPPRI,g1
c       CT_fork_tmp = (ulong)"dlm$drp";
        call    K$fork
        st      g0,dlm_drpexec_pcb      # Save the PCB
#
# --- Exit
#
        ret
#
.endif  # MAG2MAG
#
#**********************************************************************
#
#  NAME: DLM$quesrp
#
#  PURPOSE:
#       To provide a common means of queuing SRPs from the BEP to the
#       dlm$besrp process.
#
#  DESCRIPTION:
#       The ILT and associated request packet are queued for the executive
#       to process.  The executive is then activated to process this request.
#       This routine may be called from either the process or interrupt level.
#
#  CALLING SEQUENCE:
#       call    DLM$quesrp
#
#  INPUT:
#       g1 = ILT
#           il_w0 = VRP that contains the SRP
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
        .text
DLM$quesrp:
c       asm("   .globl  DLM$quesrp");
c       asm("DLM$quesrp:     ");

.if     DEBUG_FLIGHTREC_DLM
        ldconst frt_dlm_qsrp,r3         # DLM - DLM$quesrp
        ld      il_w0-ILTBIAS(g1),r4    # r4 = VRP
        ldos    vr_func(r4),r5          # r5 = VRP Function
        ld      vr_sglptr(r4),r7        # r7 = pointer to the SRP being passed
c       if (r7 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u DLM$quesrp sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
!       ld      sr_func(r7),r7          # r7 = SRP Function and status word
        shlo    16,r5,r5                # Set up to have several values in parm0
        or      r5,r3,r3                # r3 = Function, Flight Recorder ID
        st      r3,fr_parm0             # Function, Flight Recorder ID
        st      g1,fr_parm1             # ILT
        st      r4,fr_parm2             # VRP
        st      r7,fr_parm3             # SRP Function and status word
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM

#
# --- Insert ILT into executive queue
#
        ldl     dlm_srpexec_qht,r4      # Get queue head/tail
        ld      dlm_srpexec_pcb,r6      # Get PCB
        ld      dlm_srpexec_cqd,r7      # Get current queue depth
        cmpobne 0,r4,.dqsrp10           # Jif queue not empty
#
# --- Insert into empty queue
#
        mov     g1,r5                   # Update queue head/tail with
        mov     g1,r4                   #  single entry
        b       .dqsrp20
#
# --- Insert into non-empty queue
#
.dqsrp10:
        st      g1,il_fthd(r5)          # Append ILT to end of queue
        mov     g1,r5                   # Update queue tail
.dqsrp20:
        stl     r4,dlm_srpexec_qht
        addo    1,r7,r7                 # Bump queue depth
        st      r7,dlm_srpexec_cqd
#
# --- Activate executive if necessary
#
        ldob    pc_stat(r6),r3          # Get current process status
        mov     pcrdy,r8                # Get ready status
        cmpobne pcnrdy,r3,.dqsrp100     # Jif status other than not ready
#
.ifdef HISTORY_KEEP
c CT_history_pcb(".dqsrp20 setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r6)          # Ready process
#
# --- Exit
#
.dqsrp100:
        ret
#
#**********************************************************************
#
#  NAME: DLM$quedrp
#
#  PURPOSE:
#       To provide a common means of queuing DRPs from the CCB or Cache to the
#       dlm$drp process.
#
#  DESCRIPTION:
#       The ILT and associated request packet are queued for the executive
#       to process.  The executive is then activated to process this request.
#       This routine may be called from either the process or interrupt level.
#
#  CALLING SEQUENCE:
#       call    DLM$quedrp
#
#  INPUT:
#       g1 = ILT
#           il_w0-ILTBIAS = DRP address
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
DLM$quedrp:
c       asm("   .globl  DLM$quedrp");
c       asm("DLM$quedrp:");

.if     DEBUG_FLIGHTREC_DLM
        ldconst frt_dlm_qdrp,r3         # DLM - DLM$quedrp
        ld      il_w0-ILTBIAS(g1),r4    # r4 = DRP
        ldos    dr_func(r4),r5          # r5 = DRP Function
        ld      dr_req_address(r4),r6   # r6 = Datagram Request Header
!       ld      dgrq_srvcpu(r6),r7      # r7 = Server Processor Code (SPC),
                                        #   Header Length, Sequence Number
        shlo    16,r5,r5                # Set up to have several values in parm0
        or      r5,r3,r3                # r3 = Function, Flight Recorder ID
        st      r3,fr_parm0             # Function, Flight Recorder ID
        st      g1,fr_parm1             # ILT
        st      r4,fr_parm2             # DRP
        st      r7,fr_parm3             # SPC, Req Header Length, Sequence Number
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM
#
# --- Insert ILT into executive queue
#
        ldl     dlm_drpexec_qht,r4      # Get queue head/tail
        ld      dlm_drpexec_pcb,r6      # Get PCB
        ld      dlm_drpexec_cqd,r7      # Get current queue depth
        cmpobne 0,r4,.dqdrp10           # Jif queue not empty
#
# --- Insert into empty queue
#
        mov     g1,r5                   # Update queue head/tail with
        mov     g1,r4                   #  single entry
        b       .dqdrp20
#
# --- Insert into non-empty queue
#
.dqdrp10:
        st      g1,il_fthd(r5)          # Append ILT to end of queue
        mov     g1,r5                   # Update queue tail
.dqdrp20:
        stl     r4,dlm_drpexec_qht
        addo    1,r7,r7                 # Bump queue depth
        st      r7,dlm_drpexec_cqd
#
# --- Activate executive if necessary
#
        ldob    pc_stat(r6),r3          # Get current process status
        mov     pcrdy,r8                # Get ready status
        cmpobne pcnrdy,r3,.dqdrp100     # Jif status other than not ready
#
.ifdef HISTORY_KEEP
c CT_history_pcb(".dqdrp20 setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r6)          # Ready process
#
# --- Exit
#
.dqdrp100:
        ret
#
#**********************************************************************
#
#  NAME: DLM$SetFibreHeartbeatList
#
#  PURPOSE:
#       To handle the CCB request to start sending Heartbeats out the FE Fibre
#       to the selected XIOtech Controllers
#
#  DESCRIPTION:
#       The list of selected XIOtech Controllers will be verified that all can
#       be seen by the FE DLM.  Once all are verified as being seen, flags in
#       the MTMT's will be set to begin the heartbeat process for the new list
#       of controllers.
#
#  CALLING SEQUENCE:
#       call    DLM$SetFibreHeartbeatList
#
#  INPUT:
#       g0 = Parameters for MRP
#
#  OUTPUT:
#       g0 = MRP Return Code
#               deok (0x00) - All controllers found and polling started
#               deinvctrl (0x02B) - At least one controller was not found that
#                   was requested
#       g1 = Invalid Controller Number if g0 = deinvctrl otherwise trashed
#
#**********************************************************************
#
DLM$SetFibreHeartbeatList:
        mov     g0,r15                  # r15 = Pointer to the parameter list
        ldob    mfhl_nctrl(r15),r3      # r3 = Number of controllers
        ldconst deok,g0                 # g0 = Preset to Good Status
#
# --- Loop through the list of controllers ensuring all are ok
#
        ldconst 0,r4                    # r4 = Index into controller list
.dfhls20:
        cmpobe  r3,r4,.dfhls30          # Jif all tested OK
        ld      mfhl_csnlist(r15)[r4*4],g0  # g0 = Controller's serial number
        cmpobe  0,g0,.dfhls90           # Jif the Serial Number is invalid
        call    DLM$find_controller     # Determine if the controller has
                                        #  been seen on the FE yet
                                        # Input g0 = Controller Serial Number
                                        # Output g0 = 0 if not seen yet
        cmpobe  0,g0,.dfhls90           # Jif the controller is not available
#
# --- Don't consider if ICL path alone is existing. We don't consider ICL path for
#     heartbeat processing..The current release(5.2) supports ICL path as an additional
#     FE path only.  Fix for CQT 17703.
#
c       r7 = ICL_onlyIclPathExists((void*)g0);
        cmpobe  TRUE,r7,.dfhls90        # Still consider controller not available
c fprintf(stderr,"%s%s:%u DLM$SetFibre MLMT( controller) is available\n", FEBEMESSAGE, __FILE__, __LINE__);
        addo    1,r4,r4                 # Point to the next controller in list
        b       .dfhls20                # Check the next entry
#
# --- All controllers have been found.  Clear all the polling bits from all
#       XIOtech Controllers found so far (MLMTs).
#
.dfhls30:
        ld      dlm_mlmthd,g0           # g0 = The beginning of the MLMT list
.dfhls40:
        cmpobe  0,g0,.dfhls50           # Jif no more XIOtech Controllers
        ldob    mlmt_flags(g0),r5       # r5 = MLMT Flags
        clrbit  MLMT_POLL_PATH,r5,r5    # Turn off the Polling Flag bit
        stob    r5,mlmt_flags(g0)       # Save the MLMT Flags byte
        ld      mlmt_link(g0),g0        # g0 = Next MLMT on the list
        b       .dfhls40                # Continue resetting the Polling bits

#
# --- Set the MLMT status flag to begin polling the controller
#
.dfhls50:
        ldconst 0,r4                    # r4 = Reset to beginning of list
.dfhls60:
        cmpobe  r3,r4,.dfhls95          # Jif all controllers set up
        ld      mfhl_csnlist(r15)[r4*4],g0  # g0 = Controller's serial number
        call    DLM$find_controller     # Get the MLMT for this controller
                                        # Input g0 = Controller Serial Number
                                        # Output g0 = MLMT of controller
        ldob    mlmt_flags(g0),r5       # r5 = MLMT Flags
        setbit  MLMT_POLL_PATH,r5,r5    # Turn on the Polling Flag bit
        stob    r5,mlmt_flags(g0)       # Save the MLMT Flags byte
        addo    1,r4,r4                 # Point to the next controller in list
        b       .dfhls60
#
# --- Invalid Controller Serial Number detected
#
.dfhls90:
        ldconst deinvctrl,g0            # Set return code to Invalid Controller
        ld      mfhl_csnlist(r15)[r4*4],g1  # Show which was not found
        b       .dfhls100
#
# --- All controllers set up.  Set the return to good and exit
#
.dfhls95:
        ldconst deok,g0                 # g0 = Return Good status
#
# --- Exit
#
.dfhls100:
        ret
#
#
.if MAG2MAG
#
#**********************************************************************
#
# ----------------- Data-link Manager Processes -----------------------
#
#**********************************************************************
#
#  NAME: dlm$besrp
#
#  PURPOSE:
#       To provide a means of processing SRP requests from the BE processor
#       at the process level which have been previously queued to this module.
#
#  DESCRIPTION:
#       The queuing routine DLM$quesrp deposits an SRP into the que and
#       activates this executive if necessary.  This executive extracts
#       the next SRP from the queue and initiates the appropriate actions.
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
#**********************************************************************
#
# --- Set this process to not ready
#
.besrp10:
        ld      K_xpcb,r15              # Get this PCB
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
dlm$besrp:
.besrp30:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
        ld      dlm_srpexec_cqd,r3      # Get current queue depth
        cmpobe  0,r3,.besrp10           # Jif none
#
        subo    1,r3,r3                 # Adjust current queue depth
        st      r3,dlm_srpexec_cqd
        ldl     dlm_srpexec_qht,r6      # Get next queue head/tail
#
# --- Dequeue selected request (FIFO fashion)
#
        mov     r6,r12                  # Isolate queued ILT
#
        ld      il_w0-ILTBIAS(r12),r13  # r13 = VRP address from the ILT
        ld      il_fthd(r6),r6          # Dequeue ILT
        ld      vr_sglptr(r13),r14      # r14 = SRP address from the VRP
c       if (r14 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u dlm$besrp sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        cmpo    0,r6                    # Determine if at end of queue or not
!       ldob    sr_func(r14),r11        # r11 = SRP Function word
        sele    r7,0,r7                 # Update queue head/tail
        stl     r6,dlm_srpexec_qht
#

.if     DEBUG_FLIGHTREC_DLM
        ldconst frt_dlm_besrp,r3        # DLM - dlm$besrp
        shlo    8,r11,r7                # Set up to have several values in parm0
        or      r7,r3,r3                # r3 = SRP Function, Flight Recorder ID
        st      r3,fr_parm0             # SRP Function, Flight Recorder ID
        st      r12,fr_parm1            # ILT
        st      r13,fr_parm2            # VRP
        st      r14,fr_parm3            # SRP
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM

#
# --- If this is a VLink SCSI command, update the VLink Outstanding
#       Request Counter
#
        ldconst srxlrp,r3               # Determine if this is a VLink SCSI op
        cmpobne r3,r11,.besrp90         # Jif not a VLink SCSI op
        ld      D_vlorc,g1              # Increment the Outstanding VLink Cntr
        addo    1,g1,g1
        st      g1,D_vlorc
#
# --- Continue setting up to send the request on
#
.besrp90:
!       ld      sr_vrpilt(r14),r3       # r3 = Original ILT/VRP of VLOP
        mov     r12,g1                  # g1 = ILT of SRP
        st      r13,il_w2(r12)          # Save the VRP address in the ILT
        st      r14,vrsrp(g1)           # Save the SRP address in the ILT
        st      r3,il_w3(g1)            # Save ILT/VRP of VLOP in ILT/SRP
#
# --- Save completion routine in ILT
#
        lda     dlm$besrpcomp,g2        # g2 = Completion routine
        st      g2,il_cr(g1)            # Save completion routine at this level
#
# --- Advance ILT to next level
#
        ld      il_misc(r3),r8          # Get the ILT Parms Pointer
        ld      vrvrp(r3),r15           # r15 = VRP
        mov     0,r3
        st      r3,il_fthd(g1)          # Clear this link
        st      r3,ILTBIAS+il_fthd(g1)  # Close link
        lda     ILTBIAS(g1),g1          # Advance to next level
#
# --- Retrieve and save the VRP Pointer in the ILT along with the SRP fields
#
        st      r8,otl1_FCAL(g1)        # Store ILT parms pointer
        st      r14,otl1_srp(g1)        #   and SRP pointer
        lda     K$comp,r7               # Setup completion routine
        ldob    vr_status(r15),r9       # r9 = VRP Completion Status
        mov     g1,r8                   # Preserve ILT pointer
        stob    r11,otl1_cmd(g1)        # Store function code
        stob    r9,otl1_cmpcode(g1)     # Store the VRP Completion Status
#
        st      r7,il_cr(g1)            # Save the Completion routine
#
        lda     ILTBIAS(g1),g1          # Advance to next nesting level
        st      r8,il_misc(g1)          #  and point back to struct
        st      r3,il_cr(g1)            # Clear next nest completion routine
#
# --- Submit SRP to Translation Layer.
#
        call    C$receive_srp
#
        b       .besrp30                # Get the next Request
#
#**********************************************************************
#
#  NAME: dlm$drp
#
#  PURPOSE:
#       To provide a means of processing DRP requests at the process level
#       which have been previously queued to this module.
#
#  DESCRIPTION:
#       The queuing routine DLM$quedrp deposits an DRP into the que and
#       activates this executive if necessary.  This executive extracts
#       the next DRP from the queue and initiates the appropriate actions.
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
#**********************************************************************
#
# --- Set this process to not ready
#
.drp10:
        ld      K_xpcb,r15              # Get this PCB
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
dlm$drp:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
.drp50:
        ld      dlm_drpexec_cqd,r3      # Get current queue depth
        cmpobe  0,r3,.drp10             # Jif none
#
        subo    1,r3,r3                 # Adjust current queue depth
        st      r3,dlm_drpexec_cqd
        ldl     dlm_drpexec_qht,r6      # Get next queue head/tail
#
# --- Dequeue selected request (FIFO fashion)
#
        mov     r6,r10                  # Isolate queued ILT
#
        ld      il_fthd(r6),r6          # Dequeue ILT
        cmpo    0,r6                    # Update queue head/tail
        sele    r7,0,r7
        stl     r6,dlm_drpexec_qht
#
        ld      il_w0-ILTBIAS(r10),r11  # r11 = DRP address from the ILT
        ld      dr_req_address(r11),g4  # g4 = Datagram Request Header
#
.if     DEBUG_FLIGHTREC_DLM
        ldos    dr_func(r11),r7         # r7 = DRP Function word
        ldconst frt_dlm_drp,r3          # DLM - dlm$drp
        shlo    16,r7,r7                # Set up to have several values in parm0
        or      r7,r3,r3                # r3 = DRP Function, Flight Recorder ID
!       ld      dgrq_fc(g4),r7          # r7 = Dg FC, path, dgrq_g0, dgrq_g1
        st      r3,fr_parm0             # DRP Function, Flight Recorder ID
        st      r10,fr_parm1            # ILT
        st      r11,fr_parm2            # DRP
        st      r7,fr_parm3             # Dg Func Code, path, dgrq_g0, dgrq_g1
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM
#
        ld      dr_req_length(r11),g5   # g5 = length of Header and Data
        ld      dr_rsp_address(r11),g6  # g6 = address of response hdr and data
        ld      dr_rsp_length(r11),g7   # g7 = length of response hdr and data
#
        lda     ILTBIAS(r10),g1         # g1 = datagram ILT at nest level #2
        mov     0,r14
        lda     dsc1_rqhdr(r10),g2      # g2 = local request header address
        stob    r14,dr_status(r11)      # save good DRP status
        lda     dsc1_rshdr(r10),g3      # g3 = local response header address
        stl     g2,dsc2_rqhdr_ptr(g1)   # save local header addresses in ILT
#
!       ldq     (g4),r12                # r12-r15 = bytes 0-15 of request
                                        #  header
        stq     r12,(g2)                # copy request header into local
                                        #  request header area
!       ldq     16(g4),r12              # r12-r15 = bytes 16-31 of request
                                        #  header
        stq     r12,16(g2)
        stq     g4,dsc2_rqbuf(g1)       # save request & response buffer
                                        #  addresses & lengths in ILT
        lda     dlm$DRP_iltcr,r12       # r12 = ILT completion handler routine
        ld      dr_sglptr(r11),r13      # r13 = SGL Pointer
        st      r12,il_cr(g1)           # save ILT completion handler routine
                                        #  in ILT
        st      r13,dsc2_sglptr(g1)     # save the SGL pointer
        lda     ILTBIAS(g1),g1          # g1 = datagram ILT at nest level 3
        movq    0,r12                   # zero out this ILT level
        stq     r12,il_w0(g1)
        stq     r12,il_w4(g1)
        ldob    dr_issue_cnt(r11),r3    # Get the requested Issue count
        ldob    dr_timeout(r11),r4      # Get the requested time out (seconds)
        cmpobne 0,r3,.drp150            # Jif a valid requested issue count
        ldconst dsc_rtycnt,r3           # Set the default datagram retry count
.drp150:
        cmpobne 0,r4,.drp175            # Jif a valid time out value was given
        ldconst dsc_timeout,r4          # Set the default datagram timeout
.drp175:
        stob    r3,dsc3_retry(g1)       # Set up the datagram retry count
        stob    r4,dsc3_timeout(g1)     # Set up the datagram time out value
        call    DLM$send_dg             # parse the datagram and handle
        b       .drp50                  # Get the next Request
#
#**********************************************************************
#
#  NAME: dlm$DRP_iltcr
#
#  PURPOSE:
#       Processes a datagram ILT completion event.
#
#  DESCRIPTION:
#       Determines if the datagram should be retried or not.  If an error
#       occurred, attempt to retry the op if allowed by the retry count and the
#       type of error. If no more retries are allowed, the failure is none
#       retryable, or the op was successful it completes the response header
#       and copies it into the response message buffer if one exists. Adjusts
#       the ILT pointer to nest level 1 and returns the ILT/VRP back to
#       the original caller.
#
#  CALLING SEQUENCE:
#       call    dlm$DRP_iltcr
#
#  INPUT:
#       g1 = datagram ILT at nest level 2
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
dlm$DRP_iltcr:
        movq    g0,r12                  # Save g0-g3
        ld      dsc2_rsbuf(g1),r6       # r6 = response buffer address
                                        #  including header
        ldl     dsc2_rqhdr_ptr(g1),r4   # r4 = local request header address
                                        # r5 = local response header address
        ldos    dgrq_seq(r4),r8         # r8 = request message sequence number
        cmpobe  0,r6,.DRPiltcr_200      # Jif no response buffer address
        stos    r8,dgrs_seq(r5)         # Save sequence # in response header
        ldq     (r5),r8                 # r8-r11 = response message header
        ldob    dgrs_status(r5),r7      # r7 = datagram completion status byte
!       stq     r8,(r6)                 # Save response message header in
                                        #  response buffer
        cmpobe  dg_st_ok,r7,.DRPiltcr_200 # Jif datagram successful
        cmpobe  dg_st_sdsp,r7,.DRPiltcr_140 # Jif error from source datagram
                                        #  service provider
        cmpobe  dg_st_slld,r7,.DRPiltcr_140 # Jif error from source LLD level
        cmpobne dg_st_dlld,r7,.DRPiltcr_200 # Jif not a dest LLD error
        ldob    dgrs_ec1(r5),r8         # r8 = Error Code 1 status
        cmpobe  dgec1_dlld_crc,r8,.DRPiltcr_140 # Jif possible transport error
        cmpobge dgec1_dlld_novdisk,r8,.DRPiltcr_200 # Jif not a possible
                                        #  transport error from ISP layer
#
.DRPiltcr_140:
        lda     ILTBIAS(g1),r7          # r7 = datagram ILT at nest level 3
        ldob    dsc3_retry(r7),r8       # r8 = datagram error retry count
        subo    1,r8,r8                 # Dec. error retry count (may have
                                        #  been zero already by dlm$send_dg
                                        #  retrying the operation)
        cmpibge 0,r8,.DRPiltcr_200      # Jif retry count is now expired
        stob    r8,dsc3_retry(r7)       # Save updated error retry count
        call    dlm$dg_retry            # Queue up to retry after delay
        b       .DRPiltcr_1000          #  and we've done our job for now!
#
.DRPiltcr_200:
        lda     -2*ILTBIAS(g1),g1       # Back ILT up to nest level 1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # Call ILT completion handler routine
.DRPiltcr_1000:
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: dlm$besrpcomp
#
#  PURPOSE:
#       To provide a means of handling the completion of the SRP.
#
#  DESCRIPTION:
#       The SRP status is saved in the VRP status before returning back
#       to Link960.  Link960 will save the VRP status across the bus for
#       the originator of the SRP.
#
#  CALLING SEQUENCE:
#       call    dlm$besrpcomp
#
#  INPUT:
#       g0 = SRP completion status
#       g1 = ILT/SRP being completed
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
dlm$besrpcomp:
        ld      vrsrp(g1),r4            # r4 = SRP address
        ld      il_w2(g1),r5            # r5 = VRP address
!       ldob    sr_func(r4),r8          # r8 = SRP Function word
        ldconst srxlrp,r6               # Determine if this is a VLink SCSI op
!       stob    g0,sr_status(r4)        # Save the status in the SRP
        stob    g0,vr_status(r5)        # Save the status in the VRP
#
.if     DEBUG_FLIGHTREC_DLM
        ldconst frt_dlm_besrp,r3        # DLM - dlm$besrp
        shlo    8,r8,r7                 # Set up to have several values in parm0
        or      r7,r3,r3                # r3 = SRP Function, Flight Recorder ID
        shlo    16,g0,r7                # r7 = Status Byte
        or      r7,r3,r3                # r3 = Status, SRP Func, Fl Rec ID
        ldconst 0x80000000,r7           # show that this is the completion rtn
        or      r7,r3,r3                # r3 = Compl, Status, SRP Func, Rec ID
        st      r3,fr_parm0             # Complete, Status, SRP Func, Fl Rec ID
        st      g1,fr_parm1             # ILT
        st      r5,fr_parm2             # VRP
        st      r4,fr_parm3             # SRP
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM
#
# --- If this is a VLink SCSI op, then decrement the Outstanding VLink Counter
#
        cmpobne r6,r8,.besrpcomp90      # Jif not a VLink SCSI op
        ld      D_vlorc,r9              # Decrement the Outstanding VLink Cntr
        subo    1,r9,r9
        st      r9,D_vlorc
#
.besrpcomp90:
        call    K$comp                  # Complete this request
        ret
#
#**********************************************************************
#
#  NAME: dlm$pollcntrl
#
#  PURPOSE:
#       To provide a means of polling a controller periodically.
#
#  DESCRIPTION:
#       Periodically polls paths to controllers that are needed for
#       cross controller communications via the Fibre Front-End.
#       Each controller is polled during the loop and once all controllers
#       have been polled, sleep until the next polling time.
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
#**********************************************************************
#
dlm$pollcntrl:
.pollcntrl_10:
        ldconst POLLCNTRL_INTERVAL,g0   # g0 = time delay period (in msec.)
        call    K$twait                 # Wait for time delay
#
# Start at the Top MLMT and poll each one that is enabled
#
        ld      dlm_mlmthd,r15          # r15 = MLMT Head
#
#   Test if polling is needed for this MLMT
#
.pollcntrl_100:
        cmpobe  0,r15,.pollcntrl_10     # Jif there are no MLMTs to poll
        ldob    mlmt_flags(r15),r14     # r14 = flags byte
        ld      mlmt_dtmthd(r15),r5     # r5 = DTMT Head
        bbc     MLMT_POLL_PATH,r14,.pollcntrl_900 # Jif polling is not wanted
#
# Polling for this controller is wanted - find the next path to poll
#
        cmpobe  0,r5,.pollcntrl_900     # Jif there are no DTMTs with MLMT
        ld      mlmt_lastpolldtmt(r15),r6 # r6 = Last MLMT path polled (DTMT)
                                        #  If zero - will use top of list below
        mov     r5,r13                  # r13 = current DTMT being tested
#
#   Ensure the DTMT associated with the last path polled still exists
#
.pollcntrl_300:
        cmpobe  r13,r6,.pollcntrl_330   # Jif the last path polled was found
        ld      dml_mllist(r13),r13     # r13 = next DTMT to test
        cmpobne 0,r13,.pollcntrl_300    # Jif there is a next DTMT to test
        mov     r5,r13                  # r13 = top of list (path was not
                                        #   found in the list - start at top)
#
#   The last path polled was found (or start at the top).  Get the next
#       operational DTMT and use it as the next path to poll.
#
.pollcntrl_330:
        mov     r13,r7                  # r7 = DTMT to find a good path to poll
.pollcntrl_350:
        ld      dml_mllist(r7),r7       # r7 = Next DTMT to check for good path
        cmpobne 0,r7,.pollcntrl_370     # Jif there is a next DTMT
        mov     r5,r7                   # r7 = Start over at the beginning
.pollcntrl_370:
        ldob    dtmt_state(r7),r10      # r10 = State of the DTMT
        ldob    dml_poll_sent(r7),r8    # r8 = Poll already sent flag
        ld      dtmt_pri_dtmt(r7),r11   # r11 = Primary DTMT
        cmpobne dtmt_st_op,r10,.pollcntrl_375 # Jif the DTMT is not operational
        cmpobne FALSE,r8,.pollcntrl_390 # Jif Poll still outstanding
        cmpobe  0,r11,.pollcntrl_500    # Jif there is no Primary DTMT - found
                                        #  DTMT to use
        ldob    dtmt_state(r11),r12     # r12 = State of Primary DTMT
        cmpobe  dtmt_st_op,r12,.pollcntrl_500 # Jif Primary DTMT is operational
                                        #  - found a DTMT to use
        b       .pollcntrl_390          # Primary DTMT not ready yet - find a
                                        #  different DTMT
.pollcntrl_375:
        cmpobne dtmt_st_notop,r10,.pollcntrl_390 # Jif the DTMT is not "not op"
                                        #           such as "initializing"
        ldob    dml_rtycnt(r7),r9       # r9 = retry count for problem path
        addo    1,r9,r9                 # increment the retry count
        cmpobne POLLCNTRL_RETRY_CNT,r9,.pollcntrl_380 # Jif not waited enough
        mov     0,r9                    # Set the retry count to 0
.pollcntrl_380:
        stob    r9,dml_rtycnt(r7)       # Save the updated retry count
        be      .pollcntrl_500          # Jif retry count expired - poll path
.pollcntrl_390:
        cmpobne r7,r13,.pollcntrl_350   # Jif the entire DTMT list not searched
        b       .pollcntrl_900          # No good DTMT found on list, next MLMT
#
# A DTMT was found that can be polled - do the polling
#
.pollcntrl_500:
        ldob    dml_path(r7),r12        # r12 = this DTMT path to poll
#
#   Build a datagram
#
        mov     0,g10                   # g10 = Request Data Length (0)
        mov     0,g11                   # g11 = Response Data Length (0)
        st      r7,mlmt_lastpolldtmt(r15) # Save the last DTMT path polled
        call    DLM$get_dg              # Get an ILT and set up a Datagram
                                        # g1 = datagram ILT at nest level 1
        ldt     dlm_polldgp_hdr,r8      # r8-r11 = bytes 0-15 of request
                                        #  message header
        ld      mlmt_sn(r15),r11        # r11 = dest. serial number
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r6   # r6 = local request header address
        bswap   r11,r11                 # Make big-endian format
        stq     r8,(r6)                 # Save bytes 0-15 of header
        ldconst TRUE,r5                 # Set flag saying Poll Sent
        ldq     dlm_polldgp_hdr+16,r8   # r8-r11 = bytes 16-31 of request
                                        #  message header
        stq     r8,16(r6)               # Save bytes 16-31 of header
        stob    r12,dgrq_path(r6)       # Save specified path to use
        st      r7,dgrq_g2(r6)          # Save DTMT in the request for comp rtn
        stob    r5,dml_poll_sent(r7)    # Show this DTMT as being polled
#
#   Send the datagram
#
        lda     dlm$poll_cr,g0          # g0 = Completion Routine
        st      g0,il_cr(g1)            # Save the Completion Routine address
        lda     ILTBIAS(g1),g1          # g1 = datagram ILT at nest level 3
        st      r15,dsc3_mlmt(g1)       # Save what MLMT is being used
        ldconst 1,r8                    # Set the datagram fields
        stob    r8,dsc3_retry(g1)       # Initialize retry count (no retry)
        ldconst 2,r8                    # Set a 2 second timeout (some
                                        #  infrequent failures at 1 second)
        stob    r8,dsc3_timeout(g1)     # Initialize timeout (seconds)
        st      r7,dsc3_reqdtmt(g1)     # Set Requested DTMT to use
        call    DLM$send_dg             # Send the Datagram
#
# Get the next MLMT to poll
#
.pollcntrl_900:
        ld      mlmt_link(r15),r15      # r15 = next MLMT on list
        b       .pollcntrl_100          # Go poll some more
#
#**********************************************************************
#
#  NAME: dlm$poll_cr
#
#  PURPOSE:
#       Handle the completion of Polling the Datagram Path request.
#
#  DESCRIPTION:
#       Determines the status of the polling.  If all is OK, free the datagram
#       and return.  If the datagram received an error, determine what type
#       of error and act appropriately:  Link failure - mark the path
#       not operational and send a note to the CCB;  If a remote failure,
#       log a software error and continue.  If the path is marked not
#       operational and all other paths are marked not operational, then
#       send notification to the CCB that no paths are available.
#
#  CALLING SEQUENCE:
#       call dlm$poll_cr
#
#  INPUT:
#       g1 = datagram ILT at nest level 2
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
dlm$poll_cr:
#
# Determine if the DTMT is still valid for this poll
#
        ld      dsc2_rqhdr_ptr(g1),r4   # r4 = request header pointer
        ld      dtmt_banner,r9          # r9 = DTMT banner pattern
        ld      dgrq_g2(r4),r5          # r5 = DTMT used in the poll
        ld      dgrq_dstsn(r4),r7       # r7 = requested serial number
        ld      dtmt_bnr(r5),r6         # r6 = banner value from DTMT
        ld      dsc2_rshdr_ptr(g1),r3   # r3 = local response header address
        ldconst FALSE,r10               # Clear out the Poll Sent flag
        stob    r10,dml_poll_sent(r5)   # Show this DTMT as being polled
        cmpobne r6,r9,.pollcr_900       # Jif specified DTMT not in use
        bswap   r7,r7                   # Make Serial Number little endian
        ld      dml_sn(r5),r8           # r8 = XIOtech Controller serial #
        ldob    dgrs_status(r3),r15     # r15 = request completion status
        cmpobne r8,r7,.pollcr_900       # Jif the DTMT has been reassigned
#
# Determine the outcome of the poll
#
        ldob    dtmt_state(r5),r6       # r6 = State of the path
        ldos    dml_poll_cnt(r5),r10    # r10 = Current Poll Count
        cmpobne dg_st_ok,r15,.pollcr_100 # Jif an error was reported on request
#
# Poll completed successfully.  If the path was previously not operational,
# make operational.  Otherwise continue the poll process.
#
        ldconst dtmt_st_op,r7           # r7 = Operational status
        cmpobne r7,r6,.pollcr_50        # Jif DTMT is not "operational"
        addo    1,r10,r10               # Increment and save the Poll Count
        stos    r10,dml_poll_cnt(r5)
        b       .pollcr_900             # Operational - all done
#
.pollcr_50:
        cmpobne dtmt_st_notop,r6,.pollcr_900 # Jif the DTMT was not "not op"
                                        #       such as "initializing"
        ld      dml_mlmt(r5),r10        # r10 = MLMT associated with the DTMT
        ldob    mlmt_flags(r10),r6      # r6 = MLMT flags
        clrbit  MLMT_LOST_ALL_SENT,r6,r6 # Always clear the Message Sent Flag
        stob    r6,mlmt_flags(r10)
        stob    r7,dtmt_state(r5)       # Set DTMT to operational
#
#       Notify the CCB of the path coming alive again
#
        PushRegs                        # Save all G registers (stack relative)
        ld      dtmt_lldmt(r5),g3       # g3 = LLDMT associated with DTMT
        ldob    dml_path(r5),g1         # g1 = XIOtech Controller path #
        ldob    dml_cl(r5),g2           # g2 = XIOtech Controller cluster #
        ldob    lldmt_channel(g3),g4    # g4 = This controllers path number
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r8,log_dlm_ccb_path_sn(g0) # Save serial # for CCB
        stob    g1,log_dlm_ccb_path_other_path(g0) # Save other controllers path #
        stob    g2,log_dlm_ccb_path_cl(g0) # Save assigned cluster # for CCB
        stob    g4,log_dlm_ccb_path_this_path(g0) # Save this controllers path number

        /* Add a flag if the path is of ICL type. */
        ldob    dtmt_icl(r5),r9
c       ICL_NotifyIclPathMade((void*)r5);
.if ICL_DEBUG
        cmpobne TRUE,r9,.pollcr_icl01
c fprintf(stderr,"%s%s:%u dlm$poll_cr ICL sending ICL new path message\n", FEBEMESSAGE, __FILE__, __LINE__);
.pollcr_icl01:
.endif  # ICL_DEBUG
        stob    r9,log_dlm_ccb_path_icl_flag(g0) # Set ICL path flag

# This is to reduce customer log messages (warnings/info) with quit lost/made messages.
c       r4 = DL_remove_delayed_message(r8, g1, g2, g4, r9);
c       if (r4 == 1 || r4 == 2) {
c           r7 = mlenewpathinfo;
c       } else {
c           r7 = mlenewpathdebug;
c       }
        stos    r7,mle_event(g0)        # Save the message type
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_path_size);
        PopRegsVoid                     # Restore all G registers (stack relative)
        b       .pollcr_900             # All Done - release resources and exit
#
# Error occurred, figure out what to do
#
.pollcr_100:
        ldob    dgrs_ec1(r3),r4         # r4 = Error Code 1 status
        ldob    dgrs_ec2(r3),r3         # r3 = Error Code 2 status
        ldconst 0,r10                   # Clear the successful Poll Count
        stos    r10,dml_poll_cnt(r5)
        cmpobne dg_st_slld,r15,.pollcr_120 # Jif not a local interface problem
        cmpobne dgec1_slld_PE,r4,.pollcr_101 # Jif not a parameter error
        cmpobe  slld_PE_baddlmid,r3,.pollcr_300 # Jif a Bad DLM ID passed -
                                        #  Could happen if DG sent and before
                                        #   put on Fibre the ISP resets the
                                        #   chip, the LTMTs are blown away, and
                                        #   then LLD grabs the DG and reports
                                        #   Bad DLM ID (LTMT value is zero).
                                        #   Report the Path was lost.
        b       .pollcr_200             # All other Parameter Errors are unexp.
#
.pollcr_101:
        cmpobne cmplt_IOE,r4,.pollcr_102 # Jif not I/O Error Reported by LLD
        cmpobe  ioe_TODisc,r3,.pollcr_900 # Jif in the middle of discovery -
                                        #    Path not sure at the moment, let
                                        #    retry again.
        b       .pollcr_300             # All other Source LLD errors expected
                                        #  Qlogic fibre going down, etc.
#
.pollcr_102:
        cmpobne cmplt_ME,r4,.pollcr_300 # Jif not Misc Error reported by LLD -
                                        #  all others are reporting fibre down
        ldconst iocbplg,r9              # r9 = Port Database Changed
        cmpobe  r9,r3,.pollcr_900       # Jif the Port Database changed - fibre
                                        #  flap, let retry again.
        b       .pollcr_300             # All others show fibre going down.
#
.pollcr_120:
        cmpobne dg_st_sdsp,r15,.pollcr_140 # Jif not a source DLM error
        cmpobe  dgec1_sdsp_nopath,r4,.pollcr_300 # Jif "no path" - could have
                                        #  been in retry when the path went
                                        #  away and now no path exists.
        cmpobe  dgec1_slld_noimt,r4,.pollcr_300 # Jif "no IMT"
#
.pollcr_140:
        cmpobne dg_st_dlld,r15,.pollcr_200 # Jif not a dest LLD error - log
        cmpobe  dgec1_dlld_crc,r4,.pollcr_300 # Jif possible transport error
        cmpobe  dgec1_dlld_badsn,r4,.pollcr_300 # During movement of targets
                                        #  due to a failover, the DG can be
                                        #  sent to the same WWN but end on a
                                        #  different controller, so treat as
                                        #  a failed path (target moved off
                                        #  expected controller).
        cmpobl  dgec1_dlld_novdisk,r4,.pollcr_300 # Jif possible transport
                                        #  errors from ISP layer
#
#   Something unexpected was returned.  Report the Software fault.
#
.pollcr_200:
# dlm_sft14 means "DLM Poll Unexpected Error"
        ldconst dlm_sft14,r9            # r9 = error code to log
        mov     g0,r7                   # Save g0
        lda     dlm_sft,g0              # g0 = Software Fault Log Area
        st      r9,efa_ec(g0)           # Save the Error Code
        st      r15,efa_data(g0)        # Save the Datagram Status
        st      r4,efa_data+4(g0)       # Save the Error Code 1 status
        st      r3,efa_data+8(g0)       # Save the Error Code 2 status
        ldconst 16,r9                   # Number of bytes saved (ec + data)
        st      r9,mle_len(g0)          # Save the number of bytes to send
        call    M$soft_flt              # g0 = log data
        mov     r7,g0                   # Restore g0
#
#   Determine if the path is still valid.  If so, mark the path as not
#       operational to not use it again for a while and log a message to
#       the CCB.  If all paths are not operations, send another log message
#       to the CCB.
#
.pollcr_300:
        PushRegs                        # Save all G registers (stack relative)
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g3 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.

        stob    r15,log_dlm_ccb_path_dg_status(g3) # Save Datagram Status in Log message
        stob    r4,log_dlm_ccb_path_dg_ec1(g3) # Save the Error Code 1
        stob    r3,log_dlm_ccb_path_dg_ec2(g3) # Save the Error Code 2
        mov     0,r4                    # Set up to clear the retry count field
        ldconst dtmt_st_notop,r7        # r7 = Not operational status
        cmpobe  r7,r6,.pollcr_490       # Jif DTMT is already not op
        cmpobne dtmt_st_op,r6,.pollcr_490 # Jif DTMT is not "operational"
                                        #   such as "initializing"
        stob    r7,dtmt_state(r5)       # Set DTMT as not operational
        stob    r4,dml_rtycnt(r5)       # Clear the retry count field
#
#       Notify the CCB of the path failure, if the ports are not being reset
#           by the controller (the ISP layer will have already reported the
#           Port Problem).
#
        st      r8,log_dlm_ccb_path_sn(g3) # Save serial # for CCB Logging
        ld      dtmt_lldmt(r5),r11      # r11 = LLDMT associated with DTMT
        ldob    lldmt_channel(r11),r12  # r12 = This controllers path
        mov     r12,g0                  # g0 = Port being interrogated
.if FE_ICL
        ldob    dtmt_icl(r5),r7         # Get ICL flag
        cmpobe  TRUE,r7,.pollcr_icl04   # for ICL ,forcibly sending log message.
.endif  # FE_ICL
        call    ISP_IsReady             # g0 = Number of Ports ready
        cmpobe  0,g0,.pollcr_490        # Jif all Ports are not ready (do not
                                        #  report the error - done by ISP)
.if FE_ICL
.pollcr_icl04:
.endif  # FE_ICL

        ldob    dml_path(r5),g1         # g1 = XIOtech Controller path #
        ldob    dml_cl(r5),g2           # g2 = XIOtech Controller cluster #
        ldconst mlelostpathdebug,r7     # Lost an established path message
        stob    g1,log_dlm_ccb_path_other_path(g3) # save other controllers path #
        stob    g2,log_dlm_ccb_path_cl(g3) # save assigned cluster # for CCB
        stob    r12,log_dlm_ccb_path_this_path(g3) # save this controllers path number
        stos    r7,mle_event(g3)        # save the message type

        /* Add a flag if the path is of ICL type. */
        ldob    dtmt_icl(r5),r7
c       ICL_NotifyIclPathLost((void *)r5);
.if ICL_DEBUG
        cmpobne TRUE,r7,.pollcr_icl05
c fprintf(stderr,"%s%s:%u dlm$poll_cr ICL sending ICL path lost message\n", FEBEMESSAGE, __FILE__, __LINE__);
.pollcr_icl05:
.endif  # ICL_DEBUG
        stob    r7,log_dlm_ccb_path_icl_flag(g3) # Set ICL path flag
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_lost_path_size);
#                             (event, controllerSN, opath, cluster, tpath, iclPathFlag);

        PushRegs(r10)
c       DL_add_delayed_message(mlelostpathwarn, r8, g1, g2, r12, r7);
        PopRegsVoid(r10)
#
#       Determine if there are any operational paths to the other controller
#
        ld      dml_mlmt(r5),g0         # g0 = MLMT associated with the DTMT
        ld      mlmt_dtmthd(g0),g1      # g1 = Head of DTMTs for MLMT
.pollcr_400:
        ldob    dtmt_state(g1),g2       # g2 = State of the DTMT path
        cmpobne dtmt_st_notop,g2,.pollcr_490 # Jif there is an operational or
                                        #           initializing path
        ld      dml_mllist(g1),g1       # Point to the next DTMT
        cmpobne 0,g1,.pollcr_400        # Jif there is another DTMT
        ldob    mlmt_flags(g0),r7       # r7 = MLMT flags
        setbit  MLMT_LOST_ALL_SENT,r7,r7 # Set the Message Sent Flag
        stob    r7,mlmt_flags(g0)
#
#       Notify the CCB of all paths to the controller have failed unless all
#           the ports are down (ISP Reported the problems already)
#
        ldconst 0xFF,g0                 # g0 = Are any ports up
        call    ISP_IsReady             # g0 = number of ports up
        cmpobe  0,g0,.pollcr_490        # Jif all Ports are not ready (do not
                                        #  report the error - done by ISP)
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlelostallpaths,r7      # Lost an established path message
        stos    r7,mle_event(g0)        # Save message type
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_path_size);

.pollcr_490:
        PopRegsVoid                     # Restore all G registers (stack relative)
#
# All done, release the datagram ILT and resources
#
.pollcr_900:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        b       DLM$put_dg              # return datagram resources and return
#
#**********************************************************************
#
#  NAME: DLM$StartFECheckingMP
#
#  PURPOSE:
#       Start the FE checking for communications to the Mirror Partner
#
#  DESCRIPTION:
#       This function will kick off the task that will monitor the paths to
#       the Mirror Partner.
#
#  CALLING SEQUENCE:
#       call    DLM$StartFECheckingMP
#
#  INPUT:
#       None
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
DLM$StartFECheckingMP:
        movl    g0,r14                  # Save g0-g1
        mov     g14,r13                 # Save g14
        lda     DLM_MirrorPartnerFECommAvailable,g0  # Start the Task
        ldconst DLMPOLLCNTRL,g1
c       CT_fork_tmp = (ulong)"DLM_MirrorPartnerFECommAvailable";
        call    K$tfork
        movl    r14,g0                  # Restore g0-g1
        mov     r13,g14                 # Restore g14
        ret
#
#**********************************************************************
#
#  NAME: DLM$PortReady
#
#  PURPOSE:
#       To provide a function to let DLM know that an ISP port is now ready.
#
#  DESCRIPTION:
#       May be used to turn off "New Path" messages.  When this message is
#       received, DLM must determine which port has come ready.
#
#  CALLING SEQUENCE:
#       call    DLM$PortReady
#
#  INPUT:
#       None
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
DLM$PortReady:
        ret
#
#**********************************************************************
#
#  NAME: DLM$queryFEcomm
#
#  PURPOSE:
#       Return the status of communications to another controller
#
#  DESCRIPTION:
#       Checks to see if the controller is in the MLMT list and if there are any
#       active communications (in the DTMT list). If all are good, return
#       Communications OK.  If any fail, return unable to communicate.
#
#  CALLING SEQUENCE:
#       call    DLM$queryFEcomm
#
#  INPUT:
#       g0 = Controller Serial Number
#
#  OUTPUT:
#       g0 = Return Status
#            ecok (0) = Communications is available to the other controller
#            deinvctrl (2B) = Unable to communicate at this time
#
#**********************************************************************
#
DLM$queryFEcomm:
#
# --- Determine if the controller has ever been seen
#
                                        # Input g0 = Controller Serial Number
        call    DLM$find_controller     # Output g0 = MLMT for controller or
                                        #  zero if not found
        cmpobe  0,g0,.dqfec90           # Jif there is no controller found
#
# --- Don't consider if ICL path when it is alone existing for establishing mirror
#     partners..The current release(5.2) supports ICL path as an additional FE path
#     only... Fix for CQT 17703.
#
c       r7 = ICL_onlyIclPathExists((void*)g0);
        cmpobe  TRUE,r7,.dqfec90        # Still consider controller not available
#
# --- Determine if there is at least one active communications path to the
#       other controller
#
        ld      mlmt_dtmthd(g0),r4      # r4 = DTMT Head
.dqfec20:
        cmpobe  0,r4,.dqfec90           # Jif all the paths have been searched
        ldob    dtmt_state(r4),r5       # r5 = State of the DTMT
        ld      dtmt_pri_dtmt(r4),r6    # r6 = Primary DTMT
        cmpobne dtmt_st_op,r5,.dqfec80  # Jif the DTMT is not operational
        cmpobe  0,r6,.dqfec95           # Jif there is no Primary DTMT - found
                                        #  a DTMT that is open
        ldob    dtmt_state(r6),r7       # r7 = State of Primary DTMT
        cmpobe  dtmt_st_op,r7,.dqfec95  # Jif Primary DTMT is operational -
                                        #  found a DTMT that is open
#
# --- No good path found yet, keep looking
#
.dqfec80:
        ld      dml_mllist(r4),r4       # r4 = Next DTMT to check for good path
        b       .dqfec20                # Keep looking for a good path
#
# --- Return
#
.dqfec90:
        ldconst deinvctrl,g0            # The controller cannot be talked to
        b       .dqfec100
#
.dqfec95:
        ldconst ecok,g0                 # The controller communications is open
#
.dqfec100:
        ret
#
#**********************************************************************
#
#  NAME: dlm$vrphand
#
#  PURPOSE:
#       Data-link Manager VRP request function handler routine table.
#
#  DESCRIPTION:
#       This table contains the VRP request handler routines used by
#       the data-link manager.
#
#  CALLING SEQUENCE:
#       None.
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level 2
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
        .data
dlm$vrphand:
        .word   dlm$LOP                 # 0x40 vrlldop - link-level driver operational
        .word   dlm$MLE                 # 0x41 vrmlest - MAGNITUDE link established
        .word   dlm$MLT                 # 0x42 vrmlterm - MAGNITUDE link terminated
        .word   dlm$SBE                 # 0x43 vrftid - Foreign Target identified (Send to BE)
        .word   dlm$SBE                 # 0x44 vrftterm - Foreign Target terminated (Send to BE)
        .word   dlm$MRC                 # 0x45 vrmsgrcv - message received
        .word   dlm$SBE                 # 0x46 vrsterm - link session terminated (Send to BE)
        .text
#
#**********************************************************************
#
#  NAME: dlm$SBE
#
#  PURPOSE:
#       Sends the VRP request on to the Back-End Processor
#
#  DESCRIPTION:
#       The VRP request will be forwarded to the Back-End to handle.
#
#  CALLING SEQUENCE:
#       call    dlm$SBE
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level 2
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
dlm$SBE:
        movq    g0,r12                  # Save g0-g3
        mov     g14,r11                 # Save g14
#
# --- Set the completion routine to K$comp
#
        lda     K$comp,r3
        st      r3,il_cr(g14)           # Save completion routine at this level
#
# --- Clear the Link
#
        mov     0,r4
        st      r4,il_fthd(g14)         # Close link
#
# --- Queue request
#
        mov     g14,g1                  # g1 = ILT to send to the Back end
        st      g13,vrvrp(g14)          # vrvrp points to the VRP in the ILT
        call    L$que                   # Queue request
#
# --- Exit
#
        movq    r12,g0                  # Restore g0-g3
        mov     r11,g14                 # Restore g14
#
        ret
#
#******************************************************************************
#
# ____________________ DTMT EVENT HANDLER TABLES ______________________________
#
#******************************************************************************
#
#******************************************************************************
#
# ____________________ DTMT EVENT HANDLER ROUTINES ____________________________
#
#******************************************************************************
#
#******************************************************************************
# _______________________ DLM Server Handler Routines _________________________
#
#******************************************************************************
#
#  NAME:  dlm$DLM1
#
#  PURPOSE:
#       Handle datagram services for server DLM1.
#
#  DESCRIPTION:
#       Decodes the request function code in the datagram message
#       and vectors to the proper routine to process the request.
#
#  CALLING SEQUENCE:
#       call    dlm$DLM1
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm$DLM1:
        ldob    dgrq_fc(g2),r4          # r4 = request function code
        ldconst DLM1$maxfc,r5           # r5 = max. valid function code
        cmpobge r5,r4,.DLM1_100         # Jif valid request function code
#
# --- Invalid request function code. Return error to requestor.
#
        call    DLM$srvr_invfc          # pack and return invalid function
                                        #  code response to requestor
        b       .DLM1_1000              # and we're out of here!
#
# --- Valid request function code.
#
.DLM1_100:
        shlo    2,r4,r4                 # r4 = function code * 4
        ld      DLM1$hand(r4),r4        # r4 = request handler routine
        callx   (r4)                    # and go to request handler routine
.DLM1_1000:
        ret
#
#******************************************************************************
#
#  NAME:  DLM1$hand
#
#  PURPOSE:
#       This table contains the valid request handler routines for
#       the DLM1 datagram server.
#
#  DESCRIPTION:
#       This table contains the request handler routines for the DLM1
#       datagram server in order based on the request function code.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
        .data
DLM1$hand:
        .word   dlm1$polldgp            # 0 DLM1_fc_polldgp - poll a datagram path
        .word   dlm1$estcc              # 1 DLM1_fc_estcc - establish Controller Communications
        .word   dlm1$trmcc              # 2 DLM1_fc_trmcc - terminate Controller Communications
endDLM1$hand:
        .set    DLM1$maxfc,((endDLM1$hand-DLM1$hand)/4)-1 # maximum valid function code
#
        .text
#
#******************************************************************************
#
#  NAME:  dlm1$polldgp
#
#  PURPOSE:
#       Processes a "Poll a Datagram Path" datagram request.
#
#  DESCRIPTION:
#       Validates the request and returns successful response.
#
#  CALLING SEQUENCE:
#       call    dlm1$polldgp
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm1$polldgp:
        mov     g0,r15                  # save g0
        mov     0,g0                    # set request received OK
#
        cmpobne 0,g5,.polldgp_50        # Jif remaining request message
                                        #  length != 0
        cmpobe  0,g7,.polldgp_100       # Jif response buffer = 0
.polldgp_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .polldgp_1000           # and get out of here!
#
.polldgp_100:
        ld      dgrq_dstsn(g2),r4       # r4 = specified dest. serial #
        bswap   r4,r4                   # in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r3       # r3 = my Controller serial #
        cmpobne r3,r4,.polldgp_50       # Jif serial # not me
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.polldgp_1000:
        mov     r15,g0                  # restore g0
        ret
#
#******************************************************************************
#
#  NAME:  dlm1$estcc
#
#  PURPOSE:
#       Processes an "Establish Controller Communications" datagram request.
#
#  DESCRIPTION:
#       Determines if the specified Controller is known to us and if so
#       sets a flag to allow the polling process know to poll the controller.
#
#  CALLING SEQUENCE:
#       call    dlm1$estcc
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm1$estcc:
        mov     g0,r15                  # save g0
        mov     0,g0                    # set request received OK (no logging)
#
        cmpobne 0,g5,.estcc_50          # Jif remaining request message
                                        #  length != 0
        cmpobe  0,g7,.estcc_100         # Jif response buffer = 0
#
# Invalid Parameter
#
.estcc_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .estcc_1000             # and get out of here!
#
# No Path to Controller found
#
.estcc_75:
        ldq     dlm$sdsp_nopath,r4      # r4-r7 = No Path to controller response
        b       .estcc_900              # Report the error and get out of here!
#
.estcc_100:
        ld      dgrq_g2(g2),r14         # r14 = Other Controller Serial Number
        ld      dlm_mlmthd,r12          # r12 = first MLMT on list
.estcc_300:
        cmpobe  0,r12,.estcc_75         # Jif no MLMTs on list
        ld      mlmt_sn(r12),r11        # r11 = MLMT Controller serial #
        cmpobe  r11,r14,.estcc_400      # Jif MLMT for assoc. Controller
        ld      mlmt_link(r12),r12      # r12 = next MLMT on list
        b       .estcc_300              # and check next MLMT for match
#
.estcc_400:
        ld      mlmt_dtmthd(r12),r11    # r11 = first DTMT assoc. with MLMT
        cmpobe  0,r11,.estcc_75         # Jif no DTMTs assoc. with MLMT
        ldob    mlmt_flags(r12),r10     # r10 = flags byte
        setbit  MLMT_POLL_PATH,r10,r10  # r10 = Poll Path bit set
        stob    r10,mlmt_flags(r12)     # save the new flags for the polling
                                        #   routine
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
.estcc_900:
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.estcc_1000:
        mov     r15,g0                  # restore g0
        ret
#
#******************************************************************************
#
#  NAME:  dlm1$trmcc
#
#  PURPOSE:
#       Processes a "Terminate Controller Communications" datagram request.
#
#  DESCRIPTION:
#       Determines if the communications does exist (polling of controller
#       paths) so terminates it (polling of controller).
#
#  CALLING SEQUENCE:
#       call    dlm1$trmcc
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm1$trmcc:
        mov     g0,r15                  # save g0
        mov     0,g0                    # set request received OK (no logging)
#
        cmpobne 0,g5,.trmcc_50          # Jif remaining request message
                                        #  length != 0
        cmpobe  0,g7,.trmcc_100         # Jif response buffer = 0
#
# Invalid Parameter
#
.trmcc_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .trmcc_1000             # and get out of here!
#
# No Path to Controller found
#
.trmcc_75:
        ldq     dlm$sdsp_nopath,r4      # r4-r7 = No Path to controller response
        b       .trmcc_900              # report the error and get out of here!
#
.trmcc_100:
        ld      dgrq_g2(g2),r14         # r14 = Other Controller Serial Number
        ld      dlm_mlmthd,r12          # r12 = first MLMT on list
.trmcc_300:
        cmpobe  0,r12,.trmcc_75         # Jif no MLMTs on list
        ld      mlmt_sn(r12),r11        # r11 = MLMT Controller serial #
        cmpobe  r11,r14,.trmcc_400      # Jif MLMT for assoc. Controller
        ld      mlmt_link(r12),r12      # r12 = next MLMT on list
        b       .trmcc_300              # and check next MLMT for match
#
# The controller was found in the MLMT list
#
.trmcc_400:
        ldob    mlmt_flags(r12),r10     # r10 = flags byte
        bbc     MLMT_POLL_PATH,r10,.trmcc_75 # Jif not polling this controller
        clrbit  MLMT_POLL_PATH,r10,r10  # r10 = Poll Path bit cleared
        stob    r10,mlmt_flags(r12)     # save the new flags for the polling
                                        #   routine
#
# Walk the DTMTs associated with this polling controller and set the paths
#   to "Operational".  If used by other tasks, they will detect any problems,
#   if they still exist at the time of the datagram.
#
        ld      mlmt_dtmthd(r12),r4     # Get the first DTMT
        ldconst dtmt_st_op,r5           # Set the DTMTs as operational
.trmcc_500:
        cmpobe  0,r4,.trmcc_800         # Jif there are no more DTMTs to follow
        stob    r5,dtmt_state(r4)       # Set the DTMT as operational
        ld      dml_mllist(r4),r4       # Get the next DTMT
        b       .trmcc_500
#
# All done - send good response
#
.trmcc_800:
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
.trmcc_900:
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.trmcc_1000:
        mov     r15,g0                  # restore g0
        ret
#
#******************************************************************************
# ____________________________ SUBROUTINES ____________________________________
#
#******************************************************************************
#
#******************************************************************************
#
#  NAME:  dlm$fwd_cache
#
#  PURPOSE:
#       Handle an incoming request for the CAC0 provider.  Builds a DRP
#       and then forwards the DRP to Cache.
#
#  DESCRIPTION:
#       Builds the DRP based on the incoming request and then forwards it
#       to the Cache layer to handle.
#
#       NOTE:  This module assumes the original buffers had the header and
#           data contiguous.
#
#  CALLING SEQUENCE:
#       call    dlm$fwd_cache
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       g0 = address of the DRP
#
#******************************************************************************
#
dlm$fwd_cache:
        ldconst drdlmtocache,g0         # g0 = function code (DLM to Cache)
        ldconst dgrq_size,r12           # r12 = request header size
        subo    r12,g4,g4               # g4 = request address
                                        #  including header and data
        addo    r12,g5,g5               # g5 = request length
                                        #  including header and data
        cmpobe  0,g6,.dlmfwdcache_100   # Jif there is no response address
        ldconst dgrs_size,r12           # r12 = response header size
        subo    r12,g6,g6               # g6 = response address
                                        #  including header and data
        addo    r12,g7,g7               # g7 = response length
                                        #  including header and data
.dlmfwdcache_100:
        call    dlm$create_drp          # Create a DRP with all the necessary
                                        #   values to forward to the Cache layer
                                        # g0 = address of DRP
        lda     dlm$drp_comp,r12        # r12 = completion routine address
        mov     0,r3
        st      r12,il_cr(g1)           # Save this completion routine away
        ld      il_misc-ILTBIAS(g1),r4  # Copy the ILT Parms Pointer
        st      r3,il_w2(g1)            # Show no VRP to the completion routine
        st      r4,il_misc(g1)          # Save the ILT Parms Pointer
        st      g0,vrvrp(g1)            # Save the DRP in the ILT
        st      g3,il_w1(g1)            # Save the local response header
        st      r3,il_fthd(g1)          # Close link
.if DLMFE_DRIVER
        call    dlmt$dlmfe_que          # Queue to the DLM Test Driver
.else   # if !(DLMFE_DRIVER)
        lda     ILTBIAS(g1),g1          # bump to the next level
        st      r3,il_fthd(g1)          # Close link
        call    C$quedrp                # Send this to Cache
.endif  # if DLMFE_DRIVER
        ret
#
#******************************************************************************
#
#  NAME:  dlm$findbedlmid
#
#  PURPOSE:
#       Finds the matching FE DLM ID (dtmt) based on a BE DLM ID (dtmt)
#
#  DESCRIPTION:
#       Searches the DTMTs on the LLDMT list to find a FE DTMT that contains
#       is associated with a BE DTMT.  If no match is found, then g4 is
#       set to 0.  If a match is found, g4 gets the FE DTMT value.
#
#  CALLING SEQUENCE:
#       call    dlm$findbedlmid
#
#  INPUT:
#       g4 = BE DLM ID to associate with a FE DLM ID
#       g6 = LLDMT pointer to look for a matching FE DLM ID
#
#  OUTPUT:
#       g4 = 0 if no FE DLM ID was found that matches the BE DLM ID, or
#          = FE DLM ID that matches the BE DLM ID
#
#******************************************************************************
#
dlm$findbedlmid:
        mov     g4,r3                   # Save the BE DLM ID (dtmt)
        ld      lldmt_dtmthd(g6),g4     # g4 = DTMT head
.findbedlmid_50:
        cmpobe  0,g4,.findbedlmid_100   # Jif at the end of the list
        ld      dml_bedlmid(g4),r4      # r4 = BE DLM ID from FE DTMT
        cmpobe  r3,r4,.findbedlmid_100  # Jif the associated FE DTMT was found
        ld      dtmt_link(g4),g4        # g4 = the next DTMT on the list
        b       .findbedlmid_50         # No match yet, check the next DTMT
#
.findbedlmid_100:
        ret
#
#******************************************************************************
#
#  NAME:  dlm$create_drp
#
#  PURPOSE:
#       Create a DRP for the requestor that will be sent to the appropriate
#       DRP handler.
#
#  DESCRIPTION:
#       Allocates the memory and then builds the DRP based on the input
#       parameters passed it.  The address of the newly created DRP is
#       returned to the requester.
#
#  CALLING SEQUENCE:
#       call    dlm$create_drp
#
#  INPUT:
#       g0 = function code to put in the DRP
#       g4 = request message address (includes header and data)
#       g5 = request message length (includes header and data)
#       g6 = response message address (includes header and data)
#       g7 = response message length (includes header and data)
#
#  OUTPUT:
#       g0 = address of the DRP
#
#******************************************************************************
#
dlm$create_drp:
        mov     g0,r15                  # Save g0
c       g0 = s_MallocC(drpsiz, __FILE__, __LINE__); # get the memory for a DRP and clear
        stos    r15,dr_func(g0)         # Save the passed in function
        st      g4,dr_req_address(g0)   # Save the Request address
        st      g5,dr_req_length(g0)    # Save the Request length
        st      g6,dr_rsp_address(g0)   # Save the Response address
        st      g7,dr_rsp_length(g0)    # Save the Response length
                                        # Default the Timeout and Issue Count
        ret
#
#******************************************************************************
#
#  NAME:  dlm$drp_comp
#
#  PURPOSE:
#       Handle the completion of a DRP
#
#  DESCRIPTION:
#       The status from the DRP is copied to the VRP for further completion
#       handling and the DRP is freed.
#
#  CALLING SEQUENCE:
#       call    dlm$drp_comp
#
#  INPUT:
#       g1 = ILT of the completing function
#           il_w2 = original VRP to save the status in (if available)
#           vrvrp = original DRP that is being finished
#
#  OUTPUT:
#       g0 = Status of the requested DRP function
#
#******************************************************************************
#
dlm$drp_comp:
        mov     g1,r15                  # Save g1
        ld      il_w2(g1),r3            # r3 = original VRP request
        ld      vrvrp(g1),g0            # g0 = original DRP request
        ldob    dr_status(g0),r4        # r4 = status of the request
        cmpobe  0,r3,.drpComp_10        # If no VRP, skip updating status
        stob    r4,vr_status(r3)        # copy the status to the VRP
.drpComp_10:
#
# Copy the response into the local response header for further processing
#
        ld      il_w1(g1),r6            # r6 = local response header
        ld      dr_rsp_address(g0),r7   # r7 = response header that the server
                                        #   put the status in
        ldq     (r7),r8                 # r8-r11 = response header
        stq     r8,(r6)                 # save in the local response header
                                        # g0 = DRP address
c       s_Free(g0, drpsiz, __FILE__, __LINE__); # Free DRP memory
        mov     r4,g0                   # set up to return the status
        mov     r15,g1                  # restore g1
        call    K$comp                  # complete this level of the ILT
        ret
#
.endif  # MAG2MAG
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
