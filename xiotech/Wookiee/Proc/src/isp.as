# $Id: isp.as 158811 2011-12-20 20:42:56Z m4 $
#******************************************************************************
#
#  NAME: isp (Common between FrontEnd and BackEnd)
#
#  PURPOSE:
#
#       To provide configuration and other miscellaneous hardware-related
#       services for the QLogic ISP2x00 chip (supports 2200 & 2300).
#
#  FUNCTIONS:
#
#  Process level code:
#
#       ISP$monitor             - Initialize and process default I/O requests
#
#  Interrupt level code:
#
#       ISP$rqx                 - Process request from QLogic instance x
#
#  Initialization and configuration code:
#
#       ISP$configure           - Locate and configure ISP2x00 chips on PCI bus
#       ISP$setupinit           - Setup structures prior to ISP2x00 chip init
#       ISP$initialize          - Initialize an ISP2x00 chip
#       ISP_reset_chip          - Reset and reinitialize an ISP instance
#       ISP$start               - Start ISP tasks
#
#  Miscellaneous functions
#
#       isp$check_thread       - Check an ILT thread for timeout
#       isp$comp               - Completion routine for QLogic IOCBs
#
#  Fabric functions
#
#       ISP$login_fabric_port  - Login to a particular fabric port
#       ISP$logout_fabric_port - Log out of a particular fabric port
#       ISP$get_all_next       - Retrieve information about Fabric ports
#
#  I/O related functions
#
#       ISP$initiate_io        - Initiate an I/O request to QLogic
#
#  ISP Task Management Functions
#
#       ISP$abort_iocb         - Abort a particular command IOCB
#
#  Mailbox command functions
#
#       isp$exec_cmd            - Issue a mailbox cmd to a specific ISP2x00 chip
#       isp$mmbox               - Modify specified mailbox registers upon QRP
#                                 submission/completion
#
#  Copyright (c) 1996 - 2009 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
#.set STEVE_DEBUG,1
#.set RESET_DEBUG,1
#
# --- global declarations -----------------------------------------------------
#
.if INITIATOR
        .globl  ISP$initiate_io         # Process SCSI CDB to a target device
.ifdef FABRIC
        .globl  ISP$get_all_next        # Retrieve information about Fabric ports
        .globl  ISP$login_fabric_port   # Log into a particular fabric port
#        .globl  ISP$logout_fabric_port  # Log out a particular fabric port
.endif # FABRIC
        .globl  ISP$abort_iocb          # Abort a particular command IOCB
.endif # INITIATOR

        .globl  isp$complete_io
.ifdef TARGET
        .globl  ISP$receive_io          # Process I/O request from XL
        .globl  ISP$notify_ack          # Process Immediate Notify acknowledge
.endif # TARGET

        .globl  ISP$start
        .globl  ISP$configure
        .globl  ISP$monitor
        .globl  ISP_SubmitMarker        # C access
.ifdef BACKEND
        .globl  isp$check_initiator
        .globl  isp$AbortIocbTask
.endif  # BACKEND
# For test driver...
        .globl  isp$dump_ql
        .globl  ISP_reset_chip
        .globl  ISP_ResetChip           # C access
        .globl  isp_aqrpw
        .globl  isp_rqrp

        .globl  K$xchang
        .globl  PCI$getconfig
        .globl  PCI$setconfig

.ifdef FRONTEND
        .globl  ISP$is_my_WWN
        .globl  ISP$is_my_ALPA
        .globl  servdb
.endif  # FRONTEND
        .globl  portid                  # Qlogic instance Fabric Port ID
.ifdef FRONTEND
        .globl  intlock                 # LIP interlock flag (temp?)
        .globl  dbflags                 # QLogic Port ID database flags
.if FE_ISCSI_CODE
        .globl  iscsimap
.endif  # FE_ISCSI_CODE
.endif  # FRONTEND
        .globl  sessids

        .globl  .err06
        .globl  .err07
        .globl  .err33
        .globl  .err37

        .globl  ispmap
        .globl  isp2400
        .globl  ispmax
        .globl  ispctr
        .globl  ispfail
        .globl  ispmid
        .globl  isprena
        .globl  isprqwt
        .globl  ispaywt
        .globl  ispfflags
        .globl  fc4flgs
        .globl  ispstr
        .globl  ispdefq
        .globl  rtpcb
        .globl  isprqptr
        .globl  asyqa
        .globl  ilthead
        .globl  ilttail
        .globl  timestamp
        .globl  icb2400
.ifdef BACKEND
        .globl  lidUsed
.endif  # BACKEND
.ifdef FRONTEND
#   This is for ICL functionality
        .globl  iclPortExists
.endif  # FRONTEND

        .data

.ifdef FRONTEND
iclPortExists:
        .word   0
.endif  # FRONTEND
#
        .globl  isp2500
isp2500:
        .word   0                       # bit set for each 2500 card present.
#
        .align  4
#
sessids:
        .space  MAXISP*4,0              # Room for 4 session IDs <s>
#
resilk:
        .word   0                       # Interlock bits
#
        .globl  resilk
#
.if ISP_ERROR_INJECT
ispTmoCnt:
        .word   0                       # Timeout Error Inject Counter
        .globl  ispTmoCnt
ispChkCnt:
        .word   0                       # Check Condition Error Inject Counter
        .globl  ispChkCnt
#
ispInjDev:
        .word   0                       # Error Inject Device
        .globl  ispInjDev
.endif  # ISP_ERROR_INJECT
.if ISP_GAN_DEBUG
ispGANdebug:
        .word   0                       # Force invalid Port ID in GAN rsp
        .globl  ispGANdebug
.endif  # ISP_GAN_DEBUG
.if ISP_RESET_FAIL_DEBUG
ispResetFailDebug:
        .word   0
        .globl  ispResetFailDebug
.endif  # ISP_RESET_FAIL_DEBUG
#
lpmap:
        .space  (MAXISP+MAXICL)*4,0     # Loop Position Map (AL_PA) anchors
#
        .globl  lpmap
#
ispmax:
        .short  0                       # Maximum number of adapter
#
ispfail:
        .word   0                       # MAXISP - Indicator of failed ISP devices
#
ispctr:
        .word   0                       # MAXISP - ISP found counter

ispmid:
        .word   0                       # MAXISP - ISP Multi ID Code Indicator
i_fabric_lid:
        .word   0
#
timestamp:
        .word   0                       # Timestamp for ISP timeout
#
i_timeout_cnt:
        .space  MAXISP*4,0              # get link stats timeout counter
#
.ifdef BACKEND #******************************************************* BACKEND
#
ione_pcb:                               # BE Process Online pcb
        .word   0
#
.endif #*************************************************************** BACKEND

.ifdef FABRIC

gansrb:
        .space  MAXISP*8, 0             # Anchors for request/response buffers
                                        #  Request  +0b
                                        #  Response +4b
.endif  # FABRIC
#
portid:
        .space  MAXISP*4,0              # Space for MAXISP Port IDs
#
.ifdef FRONTEND
intlock:
        .space  MAXISP,0                # LIP interlock flag //// TEMP ////
#
dbflags:
        .space  MAXISP*MAXLID,0         # Port database flags
                                        #  (2048 bytes per instance)
                                        #  Accommodates higher loop IDs for
                                        #  FL port support
servdb:
        .space  MAXISP*4,0              # Server WWN anchors
.endif  # FRONTEND

        .section        .shmem
ispGPIOD:
        .space  MAXISP*2,0xFF           # Space for GPIOD register(2300) contents
.globl  ispGPIOD
ispStallEventCounter:
        .word   0

        .data
ispirq:
        .word   isp$r23q0
        .word   isp$r23q1
        .word   isp$r23q2
        .word   isp$r23q3
        .word   isp$r23q4
        .word   isp$r23q5
        .word   isp$r23q6
        .word   isp$r23q7
        .word   isp$r23q8
        .word   isp$r23q9
        .word   isp$r23q10
        .word   isp$r23q11
        .word   isp$r23q12
        .word   isp$r23q13
        .word   isp$r23q14
        .word   isp$r23q15
.if MAXISP-4
.if MAXISP-8
.if MAXISP-12
.if MAXISP-16
.error MAXISP must be 4, 8, 12 or 16
.endif  /* 16 */
.endif  /* 12 */
.endif  /*  8 */
.endif  /*  4 */

hba_q_cnt:
        .space  MAXISP*2,0              # Space for MAXISP HBA Queue Counters
.globl  hba_q_cnt
#
# --- local equates -----------------------------------------------------------
#

#
# --- beginning of code -------------------------------------------------------
#
        .text

isp$r23qx:
        bx      (g0)                    # Branch to given routine
#
isp$r23q0:
        ldconst 0,r15                   # set chip 0 ordinal
        b       .isp$r23qx
#
isp$r23q1:
        ldconst 1,r15                   # set chip 1 ordinal
        b       .isp$r23qx
#
isp$r23q2:
        ldconst 2,r15                   # set chip 2 ordinal
        b       .isp$r23qx
#
isp$r23q3:
        ldconst 3,r15                   # set chip 3 ordinal
        b       .isp$r23qx
#
isp$r23q4:
        ldconst 4,r15                   # set chip 4 ordinal
        b       .isp$r23qx
#
isp$r23q5:
        ldconst 5,r15                   # set chip 5 ordinal
        b       .isp$r23qx
#
isp$r23q6:
        ldconst 6,r15                   # set chip 6 ordinal
        b       .isp$r23qx
#
isp$r23q7:
        ldconst 7,r15                   # set chip 7 ordinal
        b       .isp$r23qx
#
isp$r23q8:
        ldconst 8,r15                   # set chip 8 ordinal
        b       .isp$r23qx
#
isp$r23q9:
        ldconst 9,r15                   # set chip 9 ordinal
        b       .isp$r23qx
#
isp$r23q10:
        ldconst 10,r15                  # set chip 10 ordinal
        b       .isp$r23qx
#
isp$r23q11:
        ldconst 11,r15                  # set chip 11 ordinal
        b       .isp$r23qx
#
isp$r23q12:
        ldconst 12,r15                  # set chip 12 ordinal
        b       .isp$r23qx
#
isp$r23q13:
        ldconst 13,r15                  # set chip 13 ordinal
        b       .isp$r23qx
#
isp$r23q14:
        ldconst 14,r15                  # set chip 14 ordinal
        b       .isp$r23qx
#
isp$r23q15:
        ldconst 15,r15                  # set chip 15 ordinal
        b       .isp$r23qx

#
# --- Branch from here if it is a 2400 ISP, and handle the
#     interrupt in the C subroutine
.isp$r23qx:
        mov     g0,r11                  # save the g0
        mov     r15, g0                 # chip ordinal to be passed to C call
        call    ISP2400_IntrServiceRoutine  # call 2400 ISR
        mov     r11,g0                  # restore g0
        ret
#
#******************************************************************************
#
#  NAME: ISP$monitor
#
#  PURPOSE:
#
#       QLogic ISP2x00 monitor code.
#
#  DESCRIPTION:
#
#       This task is started with the ordinal of the instance of a QLogic
#       ISP 2x00 chip in register g12.  This chip is then initialized and
#       made ready to accept requests.  This task then handles all IOCBs
#       for this QLogic instance; these are initial I/O requests
#       from initiators on the Loop.  IOCBs returned in error are also
#       processed by this task; the completion routine in the
#       ILT supplied with the IOCB (the "handle" or the system reserved
#       field in all IOCBs) is called with a code designating the status
#       of the command (normal or error).
#
#       Once the QLogic chip is initialized the task <ISP$monitor_async>
#       is started.  This task handles all asynchronous events.
#
#  CALLING SEQUENCE:
#
#       Forked as separate task.
#
#  INPUT:
#
#       g12 = QLogic instance ordinal.
#
#  OUTPUT:
#
#       none.
#
#******************************************************************************
#
ISP$monitor:
        mov     g12,r15                 # preserve ordinal

.ispm10:
.ifdef FRONTEND #***************************************************** FRONTEND
#
# --- Wait for the define to send over the system information.
#
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbc     iisn,r4,.ispm20         # Jif Serial number not defined
        bbc     iiserver,r4,.ispm20     # Jif servers are not defined yet
        bbs     iivdmt,r4,.ispm30       # Jif servers and VDMTs are defined
.ispm20:
.else   # FRONTEND, switching to BACKEND
#
# --- Wait for the controller serial number to be defined.
#
        ld      K_ficb,r4               # FICB address
        ld      fi_cserial(r4),r4       # Get controller serial number
        cmpobne 0,r4,.ispm30            # Jif controller serial number is set
.endif # BACKEND ***************************************************** BACKEND
        ldconst 125,g0
        call    K$twait                 # Wait for 125 msec
        b       .ispm10
#
.ispm30:
# -----------------------------------------------------------------------------
# --- Branch to initialization code
# --- NOTE: Branches back to .ispmfini upon completion
#
        b       .ispminit               # Perform task initialization

.ispmfini:                              # Return from initialization code
#
# --- Setup registers for the processing loop.
#
        ldconst iocbsiz,r3              # sizeof IOCB
        ldconst pcispqwait,r11          # get ISP wait status
        ld      K_xpcb,r12              # get PCB for this task
        ld      ispstr[r15*4],r14       # Get address of ISP struct
        ld      ispresque(r14),r13      # get response queue pointer
        ld      isprsop(r14),r9         # get Response queue OUT pointer addr
        b       .ispm40
#
# --- Set task status to wait for request
#
.ispm35:
.ifdef HISTORY_KEEP
c if (r11 == pcrdy) {
c CT_history_pcb(".ispm35 setting ready pcb", r12);
c }
.endif  # HISTORY_KEEP
        stob    r11,pc_stat(r12)        # set task status to wait
#
# --- Context switch before processing IOCB queue.
#
.ispm40:
#
# --- NOTDONEYET --- This code needs more cleanup.
# --- NOTDONEYET --- Perhaps return and having the task terminate?
# --- NOTDONEYET --- Perhaps having this routine called and then return?
# --- NOTDONEYET --- *sigh*
#
        call    K$qxchang               # sleep until reawakened
        ldconst pcispqwait,r11          # get ISP wait status
        b       .ispm35                 # not sure if can return
# End of ISP$monitor **********************************************************

#******************************************************************************
#
#   NAME: isp$complete_io
#
#   PURPOSE:
#       To allow C code to complete IOCBs
#
#   INPUT:
#       g0 = status
#       g1 = ILT pointer
#       g11 = iocb pointer
#
#******************************************************************************
#
isp$complete_io:
        ld      il_cr(g1),r4            # Get completion handler
        callx   (r4)                    # Call the handler
        ret
#
.if INITIATOR
#
#******************************************************************************
#
#  NAME: ISP$initiate_io
#
#  PURPOSE:
#
#       To provide a means of receiving SCSI CDB from the Translation Layer
#       (or Physical Layer) and pass it to a QLogic instance.
#
#  DESCRIPTION:
#
#       This routine receives an ILT pointed to by g1 that contains parameters
#       necessary to perform an I/O operation as requested.  These parameters,
#       and their mapping to the ILT fields are as follows: (see <iltdefs.inc>)
#
#       Before the request is submitted the Port Database is checked to see if
#       information exists for the target.  If no information exists, the port
#       information is retrieved.
#
#   FRONTEND:
#       il_w1(g1) = pointer to struct as follows:
#
#       xlichipi                     Chip instance for this operation
#       xlifcflgs                    FC-AL flags for this I/O
#       xlitarget                    Target for this I/O
#       xlilun                       LUN for this I/O
#       xlitime                      Timeout in seconds
#       xlidatadir                   Data direction
#       xlicdbptr                    Pointer to SCSI CDB
#       xlisglpt                     Pointer to SGL for operation
#       xlisglnum                    Number of SGL elements
#       otil2_cr                     Completion routine to call after processing
#
#   BACKEND:
#       r_prp-ILTBIAS-ILTBIAS (prev prev ILT nest level) -> PRP structure.
#
#       If the SGL has 3 or fewer elements, a QLogic Command Type 2 IOCB
#       is constructed using the above parameters, and the SGL will be copied
#       within the IOCB.  If the SGL has more than 3 DDs, a Command Type 4 IOCB
#       is constructed, and a copy of the DD list is made in local memory.
#       If any of the data addresses in the SGL map within local memory space,
#       they are translated to the PCI equivalent.  Otherwise no mapping of the
#       SGL addresses is performed.
#
#       This IOCB is then submitted to the indicated QLogic instance.
#
#  CALLING SEQUENCE:
#
#       call    ISP$initiate_io
#
#  INPUT:
#
#       g1 = ILT pointer (frontend), adjusted to FCAL nesting level
#               <il_misc> = pointer to I/O params.
#            Completion routine called when complete or error.
#       Backend:
#       g6 = DEV record pointer (Physical)
#       g7 = ILT pointer for backend (Physical)
#
#  OUTPUT:
#
# ifdef FRONTEND
#       g0 = return status
# else if BACKEND
#       None.
# endif
#
#  REGS DESTROYED:
#
#       None (preserved).
#
#******************************************************************************
#
.ifdef FRONTEND #***************************************************** FRONTEND
# UINT32 ISP_initiate_io(void *);
        .globl ISP_initiate_io
ISP_initiate_io:
        mov     g0,g1                   # g1 = ILT
.endif  #************************************************************* FRONTEND
ISP$initiate_io:
        movl    g0,r14                  # Preserve g0/g1
                                        #  r14 = g0
                                        #  r15 = g1 = frontend ILT pointer
.ifdef FRONTEND #***************************************************** FRONTEND
#
# --- Get pointer to parameters in XLI
#
        ld      il_misc(g1),r13         # Get pointer to XL params
        ldob    xlichipi(r13),g0        # Get chip ID
.if FE_ISCSI_CODE
c       r5 = ICL_IsIclPort((UINT8)g0);
        cmpobe  TRUE,r5,.ispiio_02      # jif ICL port

        ld      iscsimap,r4             # Get iscsimap bitmap
        bbc     g0, r4,.ispfcinitiator

.ispiio_02:
        PushRegs(r3)
        mov     g1,g0
        call    fsl_sgTx
        PopRegs(r3)

        ret
.ispfcinitiator:
.endif  # FE_ISCSI_CODE
.endif  #************************************************************* FRONTEND
.ifdef BACKEND #******************************************************* BACKEND
#
# --- Get pointer to parameters in PRP
#
        ld      r_prp-ILTBIAS-ILTBIAS(g7),r13   # Get pointer to PRP
        mov     g7,r15                  # Compatibility w/ frontend linkage
        ldob    pr_channel(r13),g0      # Get Chip ID
.endif #*************************************************************** BACKEND
.if     DEBUG_FLIGHTREC_I
        stob    g0,fr_parm0+3           # Chip ID
.endif                                  # DEBUG_FLIGHTREC_I

#
# --- GBranch if 2400
#
.ifdef BACKEND
        mov     g7,g1
.endif  # BACKEND
        PushRegs(r3)
        mov     g6,g2
        call    isp2400_initiate_io
        PopRegs(r3)
        ret

# End of ISP$initiate_io ******************************************************


.ifdef TARGET
#
#******************************************************************************
#
#  NAME: ISP$receive_io
#
#  PURPOSE:
#
#       To provide a means of receiving an I/O request from the Translation
#       Layer and pass this request to an ISP2x00 instance.
#
#  DESCRIPTION:
#
#       This routine receives an ILT pointed to by g1 that contains parameters
#       necessary to perform an I/O operation as requested.  These parameters,
#       and their mapping to the ILT fields are as follows:
#
#       il_misc - Pointer to Xlation to FC-AL structure (see <iltdefs.inc>)
#
#       <xl>xxxxx(g1)                   (see <iltdefs.inc>)
#
#  CALLING SEQUENCE:
#
#       call    ISP$receive_io
#
#  INPUT:
#
#       g1 = ILT pointer; completion routine called when complete or error
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g0-g7
#
#******************************************************************************
#
ISP$receive_io:
#
        ldconst 0,r9                    # Use r9 as zero.
        ld      il_misc(g1),r15         # get pointer to struct
        ld      xlFCAL(r15),r11         # get pointer to VRP struct
        ldob    scchipi(r11),g0         # get chip ID
.if FE_ISCSI_CODE
c       r4 = ICL_IsIclPort((UINT8)g0);
        cmpobe  TRUE,r4,.isprio_icl0    # Jif the port is ICL
        ld      iscsimap,r4             # Get iscsimap bitmap
        bbc     g0,r4,.ispfctarget      # jmp if not iSCSI
.isprio_icl0:
        mov     g0,g2                   # g2 = interface #
# when port is not online a call to FeProcMsg can result into crash
# make sure that we call FeProcMsg only when port is online

        ld      ispOnline,r3            # Get online flag
        bbc     g0,r3,.ispoff           # Jif port is not online

        PushRegs(r3)
        mov     g1,g0
        call    iscsiFeProcMsg
        PopRegsVoid(r3)
        ret
.ispoff:

# if port is offline we shall complete the ILT with error

        ld      il_misc(g1),g0         # Get pointer to XL params
c       KernelDispatch(1, (ILT*)g0, 0, 0);
        ret

.ispfctarget:
.endif  # FE_ISCSI_CODE
#
# --- Check for LIP interlock
#
        ldob    intlock[g0*1],r3        # Get interlock byte for this instance
        cmpobne 0,r3,.isprc100          # Jif interlocked

        PushRegs(r3)
        call    isp2400_build_ctio7
        PopRegsVoid(r3)
        ret

#
# --- Return I/O requests to the completion routine, marked as aborted
#     due to LIP interlock.
#
.isprc100:
        ldconst ecioint,g0
        b       K$comp

# End of ISP$receive_io *******************************************************
.endif # TARGET
#

.endif # INITIATOR
#
# -----------------------------------------------------------------------------
# Initialization code, gotten to from label .ispm30
#
.ispminit:
        mov     g12,r15                 # preserve ordinal
#       r15 = QLogic ordinal from now onwards.
#
# --- Allocate port database for this instance
#
c       g0 = s_MallocC(PORTDALLOC|BIT31, __FILE__, __LINE__); # Allocate mem for port database
        st      g0,portdb[r15*4]        # Store anchor
.ifdef FRONTEND #***************************************************** FRONTEND
#
# --- Allocate Server database for this instance, if necessary
#
c       g0 = s_MallocC(SERVDBALLOC|BIT31, __FILE__, __LINE__); # Allocate Server name database
        st      g0,servdb[r15*4]        # Store anchor

.endif # FRONTEND **************************************************** FRONTEND

#
# --- Allocate loop position map space for this instance, if necessary.
#
c       g0 = s_MallocC(LOOPPOSSZ|BIT31, __FILE__, __LINE__);
        st      g0,lpmap[r15*4]         # store anchor
# --- Clear/invalidate loop position map (al_pa map), device list, etc.
        ldconst 0xFFFFFF00,r4           # Set map length 0, invalid al_pa's
        st      r4,(g0)                 # Invalidate loop position map
.ifdef BACKEND #******************************************************* BACKEND
#
# --- Initialize the fabric LID table
#
        mov     g0,r4
        mov     r15,g0
        call    FAB_clearLid
        mov     r4,g0

.endif # BACKEND ****************************************************** BACKEND
#
# --- Setup doubly linked request list head/tail
#
        lda     ilthead[r15*8],r3       # Get head forward/back address
        lda     ilttail[r15*8],r4       # Get tail forward/back address
        ldconst 0,r6                    # Zero
        st      r4,il_fthd(r3)          # Set forward pointer in head
        st      r3,il_bthd(r4)          # set backward pointer in tail
        st      r6,il_bthd(r3)          # Set beginning of chain
        st      r6,il_fthd(r4)          # Set end of chain
#
# --- Allocate async event queue for this instance
#
        ldconst qcb_size+ASYSIZE,r6     # size of QCB + async queue
c       g0 = s_MallocC(r6|BIT31, __FILE__, __LINE__); # Allocate mem for QCB/queue
        st      g0,asyqa[r15*4]         # store anchor
#
        addo    qcb_size,g0,r5          # gen base for queue
        addo    g0,r6,r6                # gen end of queue
        st      r5,qc_begin(g0)         # save BEGIN pointer
        st      r5,qc_in(g0)            # save IN pointer
        st      r5,qc_out(g0)           # save OUT pointer
        st      r6,qc_end(g0)           # save END pointer
#
# --- Setup this task as the default IOCB handler for this instance
#
        ld      K_xpcb,r4               # get PCB for this task
        mov     r4,r5                   # setup for long store
#
# --- Set this task as default IOCB handler
#
        stl     r4,rtpcb[r15*8]         # set as handler and temporarily
                                        #  set up as async event PCB
                                        #  until async monitor task started
#
# --- Initialize online flag.
#
        ld      ispOnline,r3
        clrbit  r15,r3,r3               # Clear online bit for this instance
        st      r3,ispOnline
#
# --- Initialize offline fail flag.
#
        ld      ispofflfail,r3
        setbit  r15,r3,r3               # Set bit for this instance
        st      r3,ispofflfail
#
# --- Initialize ISP control structures and hardware
#
        mov     r15,g0                  # Chip ordinal
        call    ISP$setupinit           # Setup structures and init chip

.ifdef FRONTEND
# start rcv io queue exec
        mov     r15,g2                  # g2 = Port Number
        ldconst ISPRCVIOEXECPRI,g1
        ldconst ISP_RCV_IO_Queue_exec,g0
c       CT_fork_tmp = (ulong)"ISP_RCV_IO_Queue_exec";
        call    K$fork                  #  and start
c       isp_RCVIO_queue[r15].pcb = (PCB*)g0;# Set PCB
.endif

#
# --- Start ISP$monitor_async task for this instance
#
        mov     r15,g2                  # g2 = Port Number
        ldconst ISPMONAPRI,g1           # set up the task priority
        ldconst ISP_monitor_async,g0    # setup task address
c       CT_fork_tmp = (ulong)"ISP_monitor_async";
        call    K$fork                  #  and start
#
# --- Start ISP$monitor_ATIO task for this instance
#
        mov     r15,g2                  # g2 = Port Number
        ldconst ISPMONPRI,g1            # set up the task priority
        ldconst ISP_monitor_atio,g0     # setup task address
c       CT_fork_tmp = (ulong)"ISP_monitor_atio";
        call    K$fork                  #  and start
c       atiomonpcb[r15] = (PCB*)g0;     # Set PCB for handling interrupts

#
# --- Set accepting response queue entries (used by <ISP$rqx>)
#
        ld      isprena,r3              # get enable bits
        setbit  r15,r3,r3               # turn on response queue processing
        st      r3,isprena
        b       .ispmfini

# End of .ispminit ************************************************************


#******************************************************************************
#
#  NAME: isp$foreignPCIDev
#
#  PURPOSE:     Creates a log event for a foreign PCI device.
#
#  DESCRIPTION: Creates a log event for a foreign PCI device.
#
#
#  CALLING SEQUENCE:
#       K$tfork() - forked as task, before kernel is running.
#
#  INPUT:
#       g2 = Bitmap ordinal for QLogic instance. ( 0 - (MAXISP - 1) )
#       g3 = Vendor and Device ID
#
#  OUTPUT:
#       None
#
#******************************************************************************
#
isp$foreignPCIDev:
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleforeignpcidev,r3     # Foreign pci dev log event
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        stos    g2,ecr_port(g0)         # Store port number
.ifdef FRONTEND
        ldconst 0,r3                    # Indicate Front End
.else   # FRONTEND
        ldconst 1,r3                    # Indicate Back End
.endif  # FRONTEND
        stos    r3,ecr_proc(g0)         # 0 = FE, 1 = BE
        st      g3,ecr_reason(g0)       # Store vendor/device ID
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ecrlen);
        ret


#******************************************************************************
#
#  NAME: ISP$configure
#
#  PURPOSE:
#
#       To provide a means of locating and initializing PCI registers for
#       QLogic ISP2x00 chips attached to the secondary PCI bus, and to
#       initialize local structures for each discovered chip.
#
#  DESCRIPTION:
#
#       This routine performs a scan of devices on the secondary PCI bus.
#       If any QLogic ISP2x00 chips are found they are
#       memory mapped into the Secondary Outbound Translation Window area,
#       from the top of the area downwards.  The ISP registers are configured
#       for the base address to allow memory access, and the latency timer
#       is set.  The ISP structure is then initialized for each device
#       and the appropriate bit is set in the ISP bitmap.
#
#       Before the scan is started the secondary PCI bus is reset.
#
#       This routine should only be called once per PCI reset.
#
#  CALLING SEQUENCE:
#
#       call    ISP$configure
#
#  INPUT:
#
#       g0 = PCI device bitmap
#
#  OUTPUT:
#
#       g0 = bitmap of QLogic devices successfully initialized.
#            normalized from 21-30 to 0- (MAXISP - 1). (see <isp2100.inc>)
#            (also stored into <_ispmap>)
#
#  REGS DESTROYED:
#
#       g0-g7
#
#******************************************************************************
#
ISP$configure:
#
# --- allocate space for PCI device headers.
#
c       g0 = s_Malloc((PCIMAXDEV*pcstructsz)|BIT31, __FILE__, __LINE__);
        cmpobe  0,g0,.err33             # Jif can't allocate memory
        st      g0,K_pcidevs            # set anchor for table
        mov     g0,r13                  # preserve table address
#
.if FE_ISCSI_CODE
#
# --- Set the ispmap & iscsimap to zero
#
        ldconst 0,r3
        st      r3,iscsimap             # save iscsimap bitmap
        st      r3,ispmap               # save ispmap bitmap
#
.endif  # FE_ISCSI_CODE
#
# --- Set the maximum number of ISP adapters
#
        ldconst MAXISP,r3
        stos    r3,ispmax
#
# --- Perform PCI device scan.
#
        call    PCI$scanbus             # scan for PCI devices on secondary
                                        #  bus; load vendor/device IDs
#
# --- Device bitmask is returned in g0
#
        mov     g0,r14                  # Get PCI device count
#
# Determine devices to initialize
#
        ldconst pcstructsz,r5           # get sizeof table entry
        ldconst 0,r9                    # initialize found device mask
#
# --- Check for supported devices in bitmap of found devices.
#     Generate another bitmask for the detected QLogic chips.
#
.isp10:
        scanbit r14,r3                  # get first found device
        bno     .isp20                  # jif end of list

        clrbit  r3,r14,r14              # clear found device
        subo    11,r3,r4                # offset bit location
        mulo    r5,r4,r10               # multiply by sizeof table entry
        addo    r13,r10,r10             # get pointer to entry
        ldos    vidr(r10),r11           # get Vendor ID
        ldos    didr(r10),r12           # get Device ID
# c fprintf(stderr, "%s%s:%u got vendor=%04lX, device=%04lX\n", FEBEMESSAGE, __FILE__, __LINE__, r11, r12);
#
.if FE_ISCSI_CODE
#
# --- Check for iSCSI Interfaces (Intel Gigabit Ethernet controllers)
#     We assume that any Intel cards are Ethernet controllers since
#     the scanbus is controlled by the FEDEVS env variable.
#
        ld      iscsimap,r6             # Get iscsimap bitmap
        subo    PCIOFFSET,r4,r8         # Get true channel number, r8 = channel number
#
        ldconst intelvenid,r7           # Intel corporation Vendor ID
        cmpobne r11,r7,.isp10_01        # Jump if vend is is not Intel's
#
        setbit  r8,r6,r6
        st      r6,iscsimap             # save iscsimap bitmap
        b       .isp15
#
.isp10_01:
        clrbit  r8,r6,r6
        st      r6,iscsimap             # save iscsimap bitmap
#
.endif  # FE_ISCSI_CODE
#
# --- check for QLogic ISP2x00 chip at device location
#
        ldconst qvendid,r7              # QLogic Vendor ID
        cmpobe  r11,r7,.isp11           # jif correct vendor ID QLogic
#
# --- check for MicroMemory card at device location
#
        ldconst mmvendid,r15            # MM card Vendor ID
        cmpobne r11,r15,.isp12          # if not MM card, log as Foreign device
        ldconst mmcardid,r8             # Micro Memory Device ID for 5425CN
        cmpobe  r12,r8,.isp10           # Jif MM-5425CN; look for next device
        b       .isp12                  # if not, log as Foreign card device

.isp11:
# --- Code supporting QLA2422/QLE2432 QLogic cards
# c fprintf(stderr, "%s%s:%u qlogic Device %04lx port %04lx\n", FEBEMESSAGE, __FILE__, __LINE__, r12, r4-PCIOFFSET);
        ldconst qdev2422id,r8           # QLogic Device ID for ISP2422
        cmpobe  r12,r8,.isp10_02        # Jif ISP2422
        ldconst qdev2432id,r8           # QLogic Device ID for ISP2432
        cmpobe  r12,r8,.isp10_02        # Jif ISP2432
        ldconst qdev2532id,r8           # QLogic Device ID for ISP2532
        cmpobe  r12,r8,.isp10_05        # Jif ISP2532

#
# --- Foreign PCI card found.  Since the kernel isn't started yet,
#     fork a process to log the error.
#
.isp12:
        ldconst isp$foreignPCIDev,g0    # task to start (BackEnd Process Online)
        ldconst 255,g1                  # lowest priority
        subo    PCIOFFSET,r4,g2         # g2 = channel number
        shlo    16,r12,g3               # combine Vendor and Device ID
        or      r11,g3,g3               # g3 = Vendor and Device ID
c       CT_fork_tmp = (ulong)"isp$foreignPCIDev";
        call    K$tfork
        b       .isp10                  # jump, not correct device ID
#
# --- ISP24xx device found.  Set bitmask to indicate device exists.
#
.isp10_02:
        ld      isp2400,r8              # Get 24xx indicator
        subo    PCIOFFSET,r4,r6         # determine "true" device location
        setbit  r6,r8,r8                # Set 24xx indicator for this instance
        st      r8,isp2400              # Store 24xx indicator
# c fprintf(stderr, "%s%s:%u 2400 card isp2400=0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r8);
        b       .isp14_5
#
# --- ISP25xx device found.  Set bitmask to indicate device exists.
#
.isp10_05:
        ld      isp2500,r8              # Get 25xx indicator
        subo    PCIOFFSET,r4,r6         # determine "true" device location
        setbit  r6,r8,r8                # Set 25xx indicator for this instance
        st      r8,isp2500              # Store 25xx indicator
# c fprintf(stderr, "%s%s:%u 2500 card isp2500=0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r8);
# Note: fall through to .isp14_5.
#
.isp14_5:
        ld      ispirq[r6*4],r10        # get 2[34]xx interrupt handler
        PushRegs(r3)
c       LI_RegisterIRQ(r6, LI_IRQCGlue, r10);
        PopRegsVoid(r3)
#
# --- ISP2x00 device found.  Set bitmask to indicate device exists.
#
.if FE_ISCSI_CODE
.isp15:
.endif  # FE_ISCSI_CODE
        setbit  r4,r9,r9
        b       .isp10                  # look for next device
#
# --- Process list of ISP2x00 devices and initialize PCI registers. -----------
#
.isp20:
        mov     0,r15                   # initialize device output bitmap
        cmpobe  0,r9,.isp100            # jif no devices to initialize
#
# --- setup PCI registers for all found chips
#
        ldconst pcstructsz,r6           # get size of PCI structure
#
.isp30:
        scanbit r9,r3                   # locate first device in bitmask
        bno     .isp100                 # jif done
        clrbit  r3,r9,r9                # clear this device
#
#     r3 = device bit position
#     r6 = size of PCI data structure
        mulo    r6,r3,r7                # Get base addr for PCI struct
        addo    r13,r7,r7               # Add to base of struct
#
.if FE_ISCSI_CODE
#
# --- if iSCSI Interfaces - skip the PCI initialization
#
        ld      iscsimap,r4             # Get iscsimap bitmap
        subo    PCIOFFSET,r3,r8         # determine "true" device location
        bbc     r8,r4,.isp35            # jif not iSCSI port
#
c       g0 = s_MallocC(isprsiz|BIT31, __FILE__, __LINE__); # Allocate/clear ISP - permanent
        st      g0,isprev[r8*4]         # Save anchor/pointer

        ldos    vidr(r7),r11            # get Vendor ID
        ldos    didr(r7),r12            # get Device ID
        st      r11,ispvendid(g0)       # Store vendor ID
        st      r12,ispmodel(g0)        # Store model
        mov     r8,r3
        b       .isp90
#
.isp35:
.endif  # FE_ISCSI_CODE
#
# --- set base address register
#
        ldconst 0x14,g0                 # PCI offset to write data
        addo    11,r3,g1                # offset device address
c       r10 = LI_AccessDevice(r3 + 11, 1);
        cmpobe  0,r10,.isp40            # jif unable to access
c fprintf(stderr, "%s%s:%u Device %ld at %08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r3, r10);
#
# --- set base address into PCI structure
#
#    r13 = table base address
#
        st      r10,pcibaddr(r7)        # Store PCI address into struct
#
# --- set latency timer
#
        ldconst pltr,g0                 # PCI offset to write data
        ldconst qlatency,g3             # PCI master latency timer value
        ldconst 1,g2                    # Write 1 byte
        call    PCI$setconfig           # Set latency timer
        cmpobe  FALSE,g4,.isp40         # jif master-abort when writing data
#
# --- Set cacheline size equal to PCI bridge cacheline size
#
        ldconst clsr,g0                 # PCI offset to write data
c       g3 = 0x10;
#
# --- Set ISP2x00 Cacheline Size to PCI bridge size
#
        ldconst 1,g2                    # Write 1 byte
        call    PCI$setconfig           # Set cacheline size register
        cmpobe  FALSE,g4,.isp40         # Jif master-abort when writing data
#
# --- Enable memory access
#
        ldconst pcmdr,g0                # Get Primary Command Register
        ldconst 2,g2                    # Get 16 bits of data
        call    PCI$getconfig           # Get data
        cmpobe  FALSE,g4,.isp40         # Jif master-abort when reading data
#
# --- Set flags to:
#
#       Set Write and Invalidate
#       Bus Master
#       Memory Address Space Enable
#       Disable I/O Address Space
#
c       g3 |= PCI_COMMAND_SERR | PCI_COMMAND_PARITY | PCI_COMMAND_INVALIDATE | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
c       g3 &= ~PCI_COMMAND_IO;
#
        ldconst 2,g2                    # Write 16 bits of data
        call    PCI$setconfig           # Set config data
        cmpobne FALSE,g4,.isp50         # Jif data written successful
#
# --- Report the PCI set config error
#
.isp40:
        subo    PCIOFFSET,r3,g2         # g2 = port
        mov     g0,g3                   # g3 = offset
        mov     g2,g4                   # g4 = count
        ldconst isp_pciConfigError,g0   # task to start
        ldconst 255,g1                  # lowest priority
c       CT_fork_tmp = (ulong)"isp_pciConfigError";
c fprintf(stderr, "%s%s:%u PCI config error, port=%ld, offset=%ld, count=%ld\n", FEBEMESSAGE, __FILE__, __LINE__, g2, g3, g4);
        call    K$tfork
        b       .isp30
#
# --- offset bit position by <PCIOFFSET> to normalize bitmask
#     (interrupt ID matches bit position) (see <pci.inc>)
# --- If >= MAXISP, exit - at max device capacity.
#
.isp50:
        subo    PCIOFFSET,r3,r3         # determine "true" device location
        cmpoble MAXISP,r3,.err06        # jif too many devices

        lda     isp24m0(r10),r4         # Get 2400 mailbox offset

#
# --- Check memory-mapped mailbox registers for proper response
#
#.isp60:
        ldos    ispm0(r4),g0            # get Outb Mailbox Reg 1
        cmpobe  4,g0,.isp70             # Jif busy
        cmpobne 0,g0,.isp30             # Jif not idle; improper response
#
# --- Check for ISP signature in mailbox registers
#
.isp70:
        ldos    ispm1(r4),g0            # get first 16 bits of signature
        ldconst isp1,g2                 # get signature
        cmpobne g0,g2,.isp30            # Jif improper signature
#
        ldos    ispm2(r4),g0            # get second 16 bits of signature
        ldconst isp2,g2
        cmpobne g0,g2,.isp30            # Jif improper signature
#
        ldos    didr(r7),r12            # get Device ID
        ldconst qdev2422id,r8           # QLogic Device ID for ISP2422
        cmpobe  r12,r8,.isp74           # Jif ISP2422
        ldconst qdev2432id,r8           # QLogic Device ID for ISP2432
        cmpobe  r12,r8,.isp74           # Jif ISP2432
        ldconst qdev2532id,r8           # QLogic Device ID for ISP2532
        cmpobe  r12,r8,.isp74           # Jif ISP2432
        ldos    ispm3(r4),g0            # get third 16 bits of signature
        ldconst isp3,g2
        cmpobne g0,g2,.isp30            # Jif improper signature
        b       .isp75

.isp74:
        ldos    ispm3(r4),g0            # get third 16 bits of signature
        cmpobne g0,r8,.isp30            # Jif improper signature
#
# --- Correct signature received.
#     Set reset interlock bit for this chip to prevent other resets
#     until chip is initialized for the first time.
#

.isp75:
        ld      resilk,g0               # Set reset interlock bit
        setbit  r3,g0,g0
        st      g0,resilk
#
# --- Now fetch revision levels.
#
        ldos    ispm4(r4),g4            # get RISC revision level
        ldos    ispm5(r4),g5            # get FB & FPM revision level
        ldos    ispm6(r4),g6            # get FB & FPM revision level
        ldos    ispm7(r4),g7            # get RISC ROM revision level
#
# --- Allocate and initialize ISP revision structure for this device.
#
c       g0 = s_MallocC(isprsiz|BIT31, __FILE__, __LINE__); # Allocate/clear ISP - permanent
        st      g0,isprev[r3*4]         # Save anchor/pointer
        stos    g4,isprevlvl(g0)        # store revision level
        stos    g5,isprsclvl(g0)        #  RISC revision level
        stos    g6,ispfpmlvl(g0)        #  FB & FPM revision levels
        stos    g7,ispromlvl(g0)        #  RISC ROM revision level

        ldos    vidr(r7),r11            # get Vendor ID
        ldos    didr(r7),r12            # get Device ID
        st      r11,ispvendid(g0)       # Store vendor ID
        st      r12,ispmodel(g0)        # Store model
#
# --- Allocate and initialize ISP data structure for this device.
#
c       g0 = s_MallocC(ispsiz|BIT31, __FILE__, __LINE__); # Allocate/clear ISP - permanent
        st      g0,ispstr[r3*4]         # Save anchor/pointer
        st      r10,ispbasead(g0)       # set PCI base addr in ISP struct

        lda     isp24rqip(r10),r4       # Add Request queue IN pointer offset
        st      r4,isprqip(g0)          # set Request queue IN pointer
        lda     isp24rqop(r10),r4       # Add Request queue OUT pointer offset
        st      r4,isprqop(g0)          # set Request queue OUT pointer
        lda     isp24rsip(r10),r4       # Add Response queue IN pointer offset
        st      r4,isprsip(g0)          # set Response queue IN pointer
        lda     isp24rsop(r10),r4       # Add Response queue OUT pointer offset
        st      r4,isprsop(g0)          # set Response queue OUT pointer
        lda     isp24m0(r10),r4         # Add 2400 mailbox offset
        st      r4,ispmbox(g0)          # set mailbox base addr in ISP struct

.if FE_ISCSI_CODE
.isp90:
.endif  # FE_ISCSI_CODE
        setbit  r3,r15,r15              # Set this device enabled
#
# --- Increment ISP device counter.  If >= MAXISP, exit - at max device capacity.
#
        ld      ispctr,r4               # get counter
        addo    1,r4,r4                 # Increment counter
        st      r4,ispctr
        cmpobg  MAXISP,r4,.isp30        # Setup other devices if < MAXISP
# -----------------------------------------------------------------------------
# --- Configuration complete.  Move bitmask in r15 to g0 and return.
# --- Also clear any outstanding PCI Scan interrupts and enable
#       interrupts for discovered chips.
#
.isp100:
        st      r15,ispmap              # Save device bitmask
        mov     r15,g0                  # Return bitmask
#
.if MULTI_ID
        st      r15,ispmid              # Save Multi-ID indicator
.endif  # MULTI_ID
#
        ret
# End of ISP$configure ********************************************************


#******************************************************************************
#
#  NAME: ISP$setupinit
#
#  PURPOSE:
#       To provide a means of initializing the control structures necessary
#       for communication with a QLogic ISP2x00 chip instance.
#
#  DESCRIPTION:
#       This routine is passed an ordinal for an instance of a QLogic ISP2x00
#       chip, derived from the bitmask of found devices.  The necessary ISP
#       control structures are allocated and/or initialized, then ISP$initialize
#       is invoked to complete the chip initialization.
#;#       If an error is encountered during initialization the error code is
#;#       returned in register g1.
#
#       This routine should only be called once per QLogic chip after PCI/ISP
#       reset.
#
#  CALLING SEQUENCE:
#       call    ISP$setupinit
#
#  INPUT:
#       g0  = Bitmap ordinal for QLogic instance. ( 0 - (MAXISP - 1) )
#
#  OUTPUT:
#;#       g0 = T/F; TRUE = chip successfully initialized.
#;#       g1 = error code if initialization failed.
#
#  REGS DESTROYED:
#       g0
#
#******************************************************************************
#
isp$init_queue:
# -----------------------------------------------------------------------------
# --- Common subroutine to setup ISP request/response queue.
# --- r3 = ICB structure address
# --- r5 = sizeof queue
# --- r7 = PCI offset value
# --- r8 = Address of ISP struct
# --- g0 = Queue anchor
#
        cmpo    0,g0                    # Check for nonzero anchor
        bne     .ispiniq10              # Jif already allocated
#
c       g0 = s_MallocC((r5 + 64)|BIT31, __FILE__, __LINE__); # Get queue, and round up by 64.
#
.ispiniq10:
# -----------------------------------------------------------------------------
# --- Setup QCB for request/response queue
#
        addo    qcb_size,g0,r4          # get location of queue
        addo    63,r4,r4                # add 63
        and     0xFFFFFFC0,r4,r4        # align to 64 bytes
        addo    r4,r5,r6                # get limit of queue
        and     0xFFFFFFC0,r6,r6        # get limit of queue
        st      r4,qc_begin(g0)         # set BEGIN of queue
        st      r4,qc_in(g0)            # set IN of queue
        st      r4,qc_out(g0)           # set OUT of queue
        st      r6,qc_end(g0)           # set END pointer
#
# --- Setup for return, g0 = queue pointer, r4 = PCI address of queue
        addo    r7,r4,r4                # Offset to PCI space
        bx      (g14)                   # Back to caller
#
ISP$setupinit:
        mov     g1,r11                  # Save g1
        mov     g2,r12                  # Save g2
# -----------------------------------------------------------------------------
# --- Allocate ICB, if necessary
#
        mov     g0,r15                  # preserve ordinal
        ld      ispstr[g0*4],r8         # Get address of ISP struct
        ld      ispicbstr(r8),r3        # Check for ICB already allocated
        cmpobne 0,r3,.ispsetup30        # Jif already allocated
        PushRegs(r14)
        call    ISP2400_SetupInit
        PopRegs(r14)
        mov     g0,r4
        cmpobe  0,r4,.err33             # Jif cant assign memory
        st      r4,ispicbstr(r8)        # Save location of ICB into ISP struct

        mov     g0,r3                   # Preserve ICB address
        ld      K_poffset,r7            # get PCI offset value
#
# r3 = ICB
# r8 = ISP
#
.ispsetup30:
# -----------------------------------------------------------------------------
# --- Setup request queue.
#
        ldconst reqalloc+qcb_size,r5    # size of request queue + QCB
        ld      ispreqque(r8),g0        # Check for req queue already allocated
        balx    isp$init_queue,g14      # ALlocate and init queue
#
        st      g0,ispreqque(r8)        # store request queue in ISP struct
        movl    g0,r11
        mov     r15,g0
        mov     r4,g1
        ld      icb2400,r10
        setbit  0,r10,r10
        st      r10,icb2400
        PushRegs(r14)
        call    ISP2400_IcbStore
        PopRegs(r14)
        movl    r11,g0

# --- Add a Debug Data Retrieval (DDR) table entry for ISP request queue

        mov     g0,g1                   # Load request queue location
        ldconst de_ireqq0,g0            # Load the isp request queue DDR offset
        addo    g0,r15,g0               # Add chip ordinal to get proper isp q
        mov     r5,g2                   # Load request queue length
c       M_addDDRentry(g0, g1, g2);
# -----------------------------------------------------------------------------
# --- Setup response queue.
#
        ldconst resalloc+qcb_size,r5    # size of response queue + QCB
        ld      ispresque(r8),g0        # Check for res queue already allocated
        balx    isp$init_queue,g14      # Allocate and init queue
#
        st      g0,ispresque(r8)        # store response queue in ISP struct
        movl    g0,r11
        mov     r15,g0
        mov     r4,g1
        ld      icb2400,r10
        setbit  1,r10,r10
        st      r10,icb2400
        PushRegs(r14)
        call    ISP2400_IcbStore
        PopRegs(r14)
        movl    r11,g0

# --- Add a Debug Data Retrieval (DDR) table entry for ISP response queue

        mov     g0,g1                   # Load response queue location
        ldconst de_irspq0,g0            # Load the isp response queue DDR offset
        addo    g0,r15,g0               # Add chip ordinal to get proper isp q
        mov     r5,g2                   # Load response queue length
c       M_addDDRentry(g0, g1, g2);
#
# -----------------------------------------------------------------------------
# --- Setup atio queue.
#
        ldconst atioqalloc+qcb_size,r5    # size of atio queue + QCB
        ld      ispatioque(r8),g0         # Check for atio queue already allocated
        balx    isp$init_queue,g14        # Allocate and init queue
        st      g0,ispatioque(r8)         # store atio queue in ISP struct
        movl    g0,r11
        mov     r15,g0
        mov     r4,g1
        ld      icb2400,r10
        setbit  2,r10,r10
        st      r10,icb2400;
        PushRegs(r14)
        call    ISP2400_IcbStore
        PopRegs(r14)
        movl    r11,g0
#
# --- Add a Debug Data Retrieval (DDR) table entry for ISP response queue
#
        mov     g0,g1                   # Load atio queue location
        ldconst de_atioq0,g0            # Load the isp atio queue DDR offset
        addo    g0,r15,g0               # Add chip ordinal to get proper isp q
        mov     r5,g2                   # Load atio queue length
c       M_addDDRentry(g0, g1, g2);
# -----------------------------------------------------------------------------

# --- Continue with chip initialization
        mov     r15,g0                  # Setup chip ordinal
        call    ISP$initialize          # Continue with chip initialization
#
# --- Clear reset interlock
#
        ld      resilk,r3
        clrbit  r15,r3,r3               # Clear reset interlock bit
        st      r3,resilk

.ifdef RESET_DEBUG
        ldconst reset_task,g0           # Start reset task
c       CT_fork_tmp = (ulong)"reset_task";
        call    K$fork
.endif  # RESET_DEBUG
        mov     r11,g1                  # Restore g1
        mov     r12,g2                  # Restore g2
        ret
# End of ISP$setupinit ********************************************************
#
#******************************************************************************
#
#  NAME: ISP$initialize
#
#  PURPOSE:
#       To provide a means of initializing the control structures necessary
#       for communication with a QLogic ISP2x00 chip instance.
#
#  DESCRIPTION:
#       This routine is passed an ordinal for an instance of a QLogic ISP2x00
#       chip, derived from the bitmask of found devices.  It tests the chip
#       interface and attached memory, downloads the QLogic firmware to the
#       chip, and returns with the chip in operational status.
#       If an error is encountered during initialization the error code is
#       returned in register g1.
#
#       The Secondary Outbound Addressing Window must have already been
#       programmed for the required memory addressing range.
#
#       This routine should only be called once per QLogic chip after PCI/ISP
#       reset.  ISP$setupinit should have been run previously.
#
#       If the symbol MULTI_ID is defined this routine builds a multi-id Initialize
#       Firmware Control Block from the list of targets for this interface
#       (see <isp$build_targets>) and uses this list to initialize the Virtual
#       Ports (multiple targets) for this interface.  The Primary Port WWN is
#       derived from the controller serial number and interface number.
#
#  CALLING SEQUENCE:
#       call    ISP$initialize
#
#  INPUT:
#       g0 = Bitmap ordinal for QLogic instance. ( 0 - (MAXISP - 1) )
#
#  OUTPUT:
#       g0 = 0 - Chip successfully initialized.
#            n - Chip failed to initialize error code
#
#  REGS DESTROYED:
#       g0
#       g1
#       g2
#       g3
#       g4
#
#       g14
#
#******************************************************************************
#
ISP$initialize:
        mov     g0,r15                  # Save port number.
#
        ld      ispstr[g0*4],r14        # Get address of ISP struct
        ld      ispbasead(r14),r5       # get chip base address
#
# --- Enable interrupts from ISP(2400)
#
        ld      isp24intc(r5),r3          # get ISP Interrupt Control Register
        setbit  isp24inte,r3,r3           # Enable Risc interrupts on PCI
        st      r3,isp24intc(r5)
c       FORCE_WRITE_BARRIER;

        movl    g0,r8                   # Save g0,g1
#
# --- Determine type of ISP chip (set up r4 with value)
#
        ld      K_pcidevs,r4            # get anchor for PCI structs
        ldconst pcstructsz,r3           # Sizeof one PCI struct
        addo    PCIOFFSET,g0,r6         # Determine proper PCI dev addr
        mulo    r3,r6,r3                # get offset into struct
        addo    r3,r4,r4                # get struct address
        ldos    didr(r4),r4             # get device ID

        ldconst qdev2422id,r3           # 2422 chip device ID
        cmpobe  r3,r4,.ispi25           # jif 2422

        ldconst qdev2432id,r3           # 2432 chip device ID
        cmpobe  r3,r4,.ispi25           # jif 2432

        ldconst qdev2532id,r3           # 2532 chip device ID
        cmpobe  r3,r4,.ispi25           # jif 2532
#
# --- Setup error code
#
        ldconst ecrbaddev,r5            # Invalid Device
        b       .ispierr100

.ispi25:
#
# --- Read GPIOD register on 2300 series.
#     GPIOD input bit 2:  1 = J2 jumper on pins 1-2 (Disable laser)
#                         0 = J2 jumper on pins 2-3 (Enable laser)
#
        PushRegs(r3)
        call    isp2400_ReadGPIOD       # Fetch the GPIOD register contents
        stos    g0,ispGPIOD[r15*2]      # r15 = port
c       WaitPortConfig();
        PopRegsVoid(r3)
#
# --- Load ISP firmware
#
        PushRegs(r3)                    # Save register contents
        call    isp_loadQFW
        PopRegs(r3)                     # Restore registers (except g0)
        mov     g0,r5                   # Save return value
        movl    r8,g0                   # Restore g0,g1
        cmpobne 0,r5,.ispierr100
#
# --- Setup Initialization Control Block (ICB) for this chip
#
        mov     g0,r8                   # save g0
        mov     r15, g0
        PushRegs(r3)                    # Save register contents
        call    ISP2400_InitFW
        PopRegs(r3)
        mov     g0,r5                   # save the return value
        mov     r8,g0                   # restore g0
        cmpobne 0,r5,.ispierr100        # jif if retvalue not zero
        b       .ispi80

# --- Reset this port, but do not re-initialize.

.ispierr100:
        mov     r15,g0                  # g0 = chip to reset
        ldconst ecro,g1                 # g1 = reason - Reset, no initialize
c fprintf(stderr, "%s%s:%u .isperr100: g0=%ld, g1=%ld, r5=%08lX\n", FEBEMESSAGE, __FILE__, __LINE__, g0, g1, r5);
        call    ISP_reset_chip
#
# --- Set the bit indicating this port failed.
#
        ld      ispmap,r3               # Get ISP found device map
        clrbit  r15,r3,r3
        st      r3,ispmap               # Save device bitmask

        ld      ispfail,r3
        setbit  r15,r3,r3
        st      r3,ispfail              # Save failed device bitmask
#
# --- Send log message for initialization failed
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleportinitfailed,r3    # ISP port initialization failure
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        stos    r15,ecr_port(g0)        # Store port number
.ifdef FRONTEND
        ldconst 0,r3                    # Indicate Front End
.else   # FRONTEND
        ldconst 1,r3                    # Indicate Back End
.endif  # FRONTEND
        stos    r3,ecr_proc(g0)         # 0 = FE, 1 = BE
        ldconst 0,r4
        st      r5,ecr_reason(g0)       # Store reason code
        st      r4,ecr_count(g0)        # Clear count
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ecrlen);
#
# --- Set failed return code
#
        mov     r5,g0                   # set unsuccessfully initialized
        b       .ispi100

#
# -- Send a log message for initialization completed successfully
#
.ispi80:
.if ISP_INIT_MSG
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleportup,r3            # ISP port initialization succeeded
        stos    r15,ecr_port(g0)        # Store port number
        st      r3,mle_event(g0)        # Store as word to clear other bytes
.ifdef FRONTEND
        ldconst 0,r3                    # Indicate Front End
.else   # FRONTEND
        ldconst 1,r3                    # Indicate Back End
.endif  # FRONTEND
        ldconst 0,r4
        ldconst ecrgood,r5              # Show the completion is good
        stos    r3,ecr_proc(g0)         # 0 = FE, 1 = BE
        st      r4,ecr_count(g0)        # Clear count
        st      r5,ecr_reason(g0)       # Store reason code
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ecrlen);
        mov     0,g0                    # Set up the completion status
.endif  # ISP_INIT_MSG
#
# --- Done with initialization of chip.
#
        ld      ispfail,r3
        bbc     r15,r3,.ispi100
        clrbit  r15,r3,r3               # Clear bit for this port
        st      r3,ispfail              # Save failed device bitmask

        ld      ispmap,r3               # Get ISP found device map
        setbit  r15,r3,r3               # Set bit for this port
        st      r3,ispmap               # Save device bitmask

.ispi100:
        ld      ispmap,r3
        ld      ispfail,r4
        ld      ispofflfail,r5
c fprintf(stderr, "%s%s:%u .ispi100: Chip initialized, ispmap=%02lX, ispfail=%02lX, ispofflfail=%02lx\n", FEBEMESSAGE, __FILE__, __LINE__, r3, r4, r5);
        ret                             # Done
# End of ISP$initialize
#
.ifdef BACKEND
#******************************************************************************
#
#  NAME: isp$AbortIocbTask
#
#  PURPOSE:
#       Used by check thread to abort an iocb on the BE.  Check thread may not
#       call abort iocb directly because it can hang the firmaware HB process
#       which prevents us from reseting the qlogic port after multiple iocb or
#       mailbox timouts. this is BE only.
#
#  DESCRIPTION:
#      calls ISP$abort_iocb
#  INPUT:
#
#       g2 = LUN for the command
#       g3 = XL ILT address
#       g4 = QLogic chip instance ordinal (0-3).
#       g5 = Loop ID
#
isp$AbortIocbTask:
        mov     g4,g0           #mov parameters to registers
        mov     g5,g1
        call    ISP$abort_iocb
        ret
.endif  # BACKEND
#
#******************************************************************************
#
#  NAME: isp$check_initiator
#
#  PURPOSE:  Checks if the initiator is a XIOtech controller.
#
#
#  DESCRIPTION: Checks if the initiator is a XIOtech controller.  Creates
#               a log event if the initiator is not a XIOtech box.
#
#               XIOtech controllers are allowed to provide storage
#               devices for the back end.
#
#  CALLING SEQUENCE:
#
#       call    isp$check_initiator
#
#  INPUT:
#
#       g0 = Channel number
#       g1 = Current LID
#       g2 = Ptr to device node WWN
#
#  OUTPUT:
#
#       g0 == 0 - not a XIOTech initiator.
#          <> 0 - XIOtech initiator
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
.ifdef BACKEND #******************************************************* BACKEND
#
isp$check_initiator:
#
        movl    g0,r8                   # Preserve g0/g1 (r8, r9)
        movl    g6,r12                  # Preserve g6/g7 (r12, r13)
#
# --- Check if this initiator is a XIOtech box.
#
        ldl     (g2),r4                 # Get WWN
c       g6 = M_chk4XIO(*(UINT64*)&r4);  # is this a XIOtech Controller ???
        cmpobe  0,g6,.ici10             # Jif not a XIOtech box
#
# --- This is a Xiotech box.  Check for a Magnitude.
#
        cmpobe  1,g6,.ici20             # Jif this is a Magnitude
#
# --- Check if this is a FE XIO target port
#
        mov     r4,r6                   # Get MSW of WWN
        extract 12,4,r6                 # Get 'e' nibble from WWN
        ldconst (WWNFNode>>20)&0xF,r7   # Value for FE target port
        cmpobe  r6,r7,.ici20            # Jif a XIOtech FE target port
        ldconst (WWNENode>>20)&0xF,r7   # Value for XIOtech disc enclosure
        cmpobe  r6,r7,.ici20            # Jif a XIOtech disc enclosure
#
# --- Set return value that indicates this is not a XIOtech target
#
        ldconst 0,g6                    # Indicate not a XIOtech target
        b       .ici20                  # Don't log this
#
# --- Log the Initiator detected on Back End log event.
#
.ici10:
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlebeinitiator,r3       # Initiator detected on the back end
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        st      r8,eib_port(g0)         # Store port number
        st      r9,eib_lid(g0)          # Store LID
        stl     r4,eib_wwn(g0)          # Store WWN
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], eiblen);
#
# --- Exit
#
.ici20:
        mov     g6,g0                   # Return value
        mov     r9,g1                   # Restore g1
        movl    r12,g6                  # Restore g6/g7 (r12, r13)
        ret
.endif # BACKEND ****************************************************** BACKEND
#
.if INITIATOR
#
#******************************************************************************
#
#  NAME: ISP$abort_iocb
#
#  PURPOSE:
#       Aborts a particular command IOCB.
#
#  DESCRIPTION:
#       Abort Command IOCB aborts a particular command IOCB that has been submitted
#       previously to the QLogic instance.  The identifier for this IOCB is the ILT
#       address that was passed to the <ISP$initiate_command> routine.  The Loop ID
#       is required as well as the LUN of the target command to abort.
#
#       If an error occurs on the abort command to the ISP instance,
#       FALSE is returned in register g0.
#
#  CALLING SEQUENCE:
#       call    ISP$abort_iocb
#
#  INPUT:
#       g0 = QLogic chip instance ordinal (0-3).
#       g1 = Loop ID
#       g2 = LUN for the command
#       g3 = XL ILT address
#
#  OUTPUT:
#       g0 = success code; T/F
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
ISP$abort_iocb:
        PushRegs(r3)                    # Save all G registers
        call    ISP_AbortIOCB
        PopRegs(r3)                     # Restore g1-g14
        ret
#
# End of ISP$abort_iocb *******************************************************
#

.endif # INITIATOR
#
.if INITIATOR
#
#******************************************************************************
#
#  NAME: isp$submit_marker
#
#  PURPOSE:
#
#       Submits a Marker IOCB to the indicated QLogic instance with a Modifier
#       code as specified in register g1.
#
#  DESCRIPTION:
#
#       This routine submits a Marker IOCB to a QLogic instance.  The Marker
#       modifier value is set to the value as stored in register g1.
#
#  CALLING SEQUENCE:
#
#       call     isp$submit_marker
#
#  INPUT:
#
#       g0 = QLogic chip instance ordinal (0-3).
#       g1 = modifier value.
#       g2 = Loop ID
#       g3 = LUN
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g0,g1
#
#******************************************************************************
#
ISP_SubmitMarker:
.ifdef FRONTEND
.endif  # FRONTEND
#
        mov     g1,r9                   # Preserve modifier value
#
# --- Issue a Marker IOCB to this instance. g0 = instance
#
        ld      ispstr[g0*4],g1
        ld      isprqip(g1),r11         # get Request queue in pointer address
#
c       {
c       UINT32 tmp;
c       UINT16 *iocb;
c       iocb = isp_get_iocb(g0,&tmp);
c       g1 = (UINT32)iocb;
c       g0 = tmp;
c       }
        cmpobe  0,g1,.ispsm100          # Jif no IOCB obtained
#
# --- Generate Marker IOCB
# --- g0 = Offset into queue
# --- g1 = IOCB
#
        ldconst iomar,r3                # Marker type IOCB
        ldconst 0x1,r4                  # Set entry count
        ldconst 0,r7                    # Zero
        stob    r3,ioentyp(g1)          # Store command type
        stob    r4,ioencnt(g1)          # Store entry count

        stos    r7,iosystd(g1)          # Clear sys def 1/entry status
        st      r7,iosr2(g1)            # Clear handle for ILT
#
#        stob    r7,0x8(g1)              # Clear Reserved
        stos    g2,0x8(g1)              # Set target ID
        stob    r7,0xB(g1)              # Clear Reserved
        stos    r7,0xC(g1)              # Clear Flags

# set 8 byte lun for 2400 little endian
        stos    r7,0x10(g1)
        stos    g3,0x12(g1)              # Set LUN
        st      r7,0x14(g1)              #clearing bits
#
# --- Gen Modifier for Marker IOCB r9 = value
#
        stob    r9,0xA(g1)              # set Modifier type

#
# --- Submit IOCB
#
c       FORCE_WRITE_BARRIER;
        stos    g0,(r11)

.if     DEBUG_FLIGHTREC_I
        ldconst frt_isp_marker,r4       # Type
        st      r4,fr_parm0             # ISP - isp$submit_marker event
        st      g0,fr_parm1             # QLogic chip instance
        st      g2,fr_parm2             # Set target ID
        st      r9,fr_parm3             # set Modifier type
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_I

.ispsm100:
        ret                             # Done!
# End of isp$submit_marker ****************************************************

.ifdef FABRIC
#
#******************************************************************************
#
#  NAME: ISP$login_fabric_port
#
#  PURPOSE:
#
#       This routine is used to login a particular 24-bit fabric ID into
#       a port number managed by the ISP.
#
#  DESCRIPTION:
#
#       Parameters passed to this routine are the following:
#
#          24-bit fabric ID
#          Port number to assign to this ID (loop ID)
#
#       Codes returned by this routine are the following:
#
#          <islfnor>   Operation completed normally.
#          <islfnfc>   No fabric connection available for this instance.
#          <islferr>   Command parameter error.
#          <islfpiu>   Port ID already in use, assigned to another Loop ID
#          <islfliu>   Loop ID already in use, assigned to another Port ID
#          <islfiiu>   All IDs are in use.
#          <islfune>   Unknown error on command.
#
#       For <islfpiu>, <islfliu> and <islfune> errors, the following information is also
#       returned:
#
#          <islfpiu>:  Loop ID used for the Port ID in error
#          <isflliu>:  Port ID used for the Loop ID in error
#          <islfune>:  Actual code returned on command
#
#  CALLING SEQUENCE:
#
#       call    ISP$login_fabric_port
#
#  INPUT:
#
#       g0 = QLogic chip instance ordinal (0-3).
#       g1 = Loop ID to assign
#       g2 = Port ID for assignment
#       g3 = Login Options
#
#  OUTPUT:
#
#       g0 = Completion code
#       g1 = Additional information
#               completion      value
#               code
#
#               00              SCSI capacity:
#                                 0 = Target not SCSI capable
#                                 1 = Target SCSI capable
#               03              port ID used
#               04              loop ID
#               07              reason:
#                                 01 = No loop
#                                 02 = IOCB could not be allocated
#                                 03 = exchange resource could not
#                                      be allocated
#                                 04 = ELS timed out or loop device
#                                      was not present
#                                 05 = no fabric loop port
#                                 06 = remote device does not support
#                                      target function.
#                                 0d = LS_RJT was received in response to PLOGI.
#                                      See OMB 6,7 for the LS_RJT error codes.
#
#  REGS DESTROYED:
#
#       All g registers may be destroyed
#
#******************************************************************************
#
ISP$login_fabric_port:
#
        mov     g0,r15                  # Preserve g0
        mov     g1,r14                  # Preserve g1
        mov     g2,r13                  # Preserve g2
#
c       r4 =ISP_LoginFabricPort(g0,(UINT32*)&r14,g2)
# Need to change return codes to make them match what the next layer is expecting
        and     0xffff,r4,r10;          # mask r10 return

        mov     r14,g1                  # Move SCSI target status to g1 (Loop ID)
        ldconst ismcmdc,r11             # Check for normal response
        ldconst islfnor,g0              # Designate normal completion
        cmpobe  r10,r11,.isplf200       # Jif normal completion

        ldconst ismccpe,r11             # Check for command parameter error
        ldconst islferr,g0              # Designate command par err
        cmpobe  r10,r11,.isplf200       # Jif command parameter error

        ldconst ismcpiu,r11             # Check for Port ID used error
        ldconst islfpiu,g0              # Designate Port ID used
        cmpobe  r10,r11,.isplf200       # Jif Port ID used error

        ldconst ismcliu,r11             # Check for nport handle used error
        ldconst islfliu,g0              # designate loop id already in use error
        cmpobe  r10,r11,.isplf200       # Jif nphandle in use error

        ldconst ismciiu,r11             # Check for all ID's in use
        ldconst islfiiu,g0              # Designate Loop ID used
        cmpobe  r10,r11,.isplf200       # Jif all ID's in use

        ldconst ismnosw,r11             # check for no fabric error
        ldconst islfnfc, g0             # designate no fabric
        cmpobe  r10,r11,.isplf200       # Jif no fabric error

        ldconst ismcmde,r11             # Check for command error
        ldconst islfcmde,g0             # Designate command error
        cmpobe  r10,r11,.isplf200       # Jif command error
#
# --- Unknown error
#
        ldconst islfune,g0              # Designate unknown error
        mov     r10,g1                  # Move actual error code to g1
.isplf200:
        ret
#
# End of ISP$login_fabric_port ************************************************


#******************************************************************************
#
#  NAME: ISP$get_all_next
#
#  PURPOSE:
#
#       This routine queries the fabric name server for registered ports.
#
#  DESCRIPTION:
#
#       Parameters passed to this routine are the following:
#
#          24-bit fabric ID
#
#          Codes returned by this routine are the following:
#
#          <isgnnor>   Operation completed normally.
#          <isgnnfc>   No fabric connection available for this instance.
#          <isgnerr>   Command parameter error.
#          <isgnune>   Unknown error on command.
#
#       For <isgnune> errors, the actual error code encountered is returned
#                     in register g1.
#
#          <isgnune>:  Actual code returned on command
#
#       The query passed to the Fabric name server results in information being
#       returned for the next highest Fabric Port ID registered on the server;
#       this information is passed back to the calling routine as a pointer to
#       the buffer in register g1.  The Port ID used as reference to return
#       the next highest ID is taken from the Port ID field passed to this
#       routine in register g1.
#
#       If no higher Port ID exists on the name server the information returned
#       is then the first registered ID.  This is how the calling routine determines
#       that no more ports are left to query.
#
#       The format of the buffer is described in the ISP 2200 Firmware Interface
#       Specification, section 8.4.1, Get All Next command.
#
#       Note that the quantities returned in this buffer will be in big-endian format.
#
#       The buffer used to contain the response is static and will be overwritten
#       on each call.
#
#  CALLING SEQUENCE:
#
#       call    ISP$get_all_next
#
#  INPUT:
#
#       g0 = QLogic chip instance ordinal (0-3).
#       g1 = 24 bit fabric port ID used to retrieve next highest ID from name server.
#
#  OUTPUT:
#
#       g0 = Completion code
#       g1 = Buffer address for response
#
#  REGS DESTROYED:
#
#       All g registers may be destroyed
#
#******************************************************************************
#
ISP$get_all_next:
#
        mov     g0,r15                  # Preserve g0
        mov     g1,r14                  # Preserve g1
#
        ld      ispfflags,r3            # Get fabric connect flag bits
        bbc     g0,r3,.ispgn100         # Jif no fabric connect
#
# --- Assign request/response buffer if necessary
#
        lda     gansrb[r15*8],r3        # Get request buffer anchor addr
        ld      (r3),r12                # Get request anchor
        cmpobne 0,r12,.ispgn10          # Jif already assigned
#
# --- Assign request buffer and initialize.
#
c       g0 = s_MallocC(ganbsize|BIT31, __FILE__, __LINE__); # Get request buffer and clear it
        st      g0,(r3)                 # Store buffer anchor
        mov     g0,r12                  # Preserve request pointer
#
# --- Assign response buffer.
#
c       g0 = s_MallocC((ganrsbln*2)|BIT31, __FILE__, __LINE__); # Get response buffer and clear it
        st      g0,4(r3)                # Store location of buffer

.ispgn10:
        PushRegs(r4)
        ld      4(r3),g1                # Get location of the response buffer
        #mov     g0, g1                  # g1 -Response Buffer(for C call)
        mov     r15,g0                  # g0 = chip instance
        mov     r14,g2                  # g2 = PortId
        call    isp2400_sendctGAN
        mov     g0,r10
        PopRegs(r4)
        ld      4(r3),g1                # Get location of the response buffer

        ldconst ismcmdc,r4              # Normal response code
        ldconst isgnnor,g0              # Designate normal return
        cmpobe  r4,r10,.ispgn110        # Jif done
#
        ldconst ismccpe,r11             # Check for command parameter error
        ldconst isgnerr,g0              # Designate command par err
        cmpobe  r10,r11,.ispgn110       # Jif param error

        ldconst ispnli,r11              # Check for NLI status
        mov     r10,g1                  # response code
        ldconst isgnnli,g0              # Set to Not logged in error
        cmpobe  r10,r11, .ispgn110      # Jif NLI status
#
# --- Unknown error
#
        ldconst isgnune,g0              # Designate unknown error
        mov     r10,g1                  # Return code in g1
        ret
#
.ispgn100:
        ldconst isgnnfc,g0              # Set to No Fabric Connected for this
                                        #   instance
        ret

.ispgn110:
.if ISP_GAN_DEBUG
#
# -- Inject Port ID error if requested
#
        cmpobne r4,r10,.ispgn120        # Only inject error on successful cmd

        ld      ispGANdebug,r9
        cmpobe  0,r9,.ispgn120          # Jif no error injection requested

        st      r8,gan_ptype+CT_HDR(g1) # Set Port ID to invalid value
        subo    1,r9,r9                 # Decrement error injection count
        st      r9,ispGANdebug
.ispgn120:
.endif  # ISP_GAN_DEBUG
        ret
# End of ISP$get_all_next *****************************************************

.endif # FABRIC
.endif # INITIATOR


#******************************************************************************
#
#  NAME: ISP$start
#
#  PURPOSE:
#
#       QLogic ISP2x00 monitor start code.
#
#  DESCRIPTION:
#
#       This routine starts the task <ISP$monitor> for each detected instance
#       of QLogic ISP2x00 chips connected to the Secondary PCI bus.  Register
#       g12 is used to pass the instance ordinal to the task. The bitmask
#       in <_ispmap> is used to determine which instances exist.
#
#  CALLING SEQUENCE:
#
#       call    ISP$start
#
#  INPUT:
#
#       none.
#
#  OUTPUT:
#
#       none.
#
#  REGS DESTROYED:
#
#       g0
#       g12
#
#******************************************************************************
#
ISP$start:
        ld      ispmap,r4               # get ISP found device bitmap
.ispst10:
#
# --- Spawn a monitor task for each existing channel chip
#
        scanbit r4,g12                  # get first found device
        bno     .ispst90                # jif end of list
        clrbit  g12,r4,r4               # clear found device
#
.if FE_ISCSI_CODE
#
# --- if iSCSI Interfaces - call iSCSI init functions
#
        ld      iscsimap,r7             # Get iscsimap bitmap
        bbc     g12,r7,.ispst20         # Jif
#
#
# --- Setup doubly linked request list head/tail
#
        lda     ilthead[g12*8],r5       # Get head forward/back address
        lda     ilttail[g12*8],r6       # Get tail forward/back address
        ldconst 0,r7                    # Zero
        st      r6,il_fthd(r5)          # Set forward pointer in head
        st      r5,il_bthd(r6)          # set backward pointer in tail
        st      r7,il_bthd(r5)          # Set beginning of chain
        st      r7,il_fthd(r6)          # Set end of chain
#
        mov     g12,g2
        ldconst ISPMONPRI,g1            # Priority of the Monitor task
        lda     fsl_Init,g0             # task to start
c       CT_fork_tmp = (ulong)"fsl_Init";
        call    K$fork                  # start monitor task
#
        b       .ispst10                # loop for more
#
.ispst20:
.endif  # FE_ISCSI_CODE
#
        ldconst ISPMONPRI,g1            # Priority of the Monitor task
        ldconst ISP$monitor,g0          # task to start
c       CT_fork_tmp = (ulong)"ISP$monitor";
        call    K$fork                  # start monitor task
#
        b       .ispst10                # loop for more
#
.ispst90:
.if FE_ISCSI_CODE
#
# Fork a task to initialize the ICL port, if it is existing
#
        ld      iclPortExists,r5        # Check whether ICL exists
        cmpobne TRUE,r5,.ispst95        # Jif not exists
c fprintf(stderr,"%s%s:%u <isp.as> ICL..port is existing=%u\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)ICL_PORT);
#
#
# --- Setup doubly linked request list head/tail for ICL port also
#
        ldconst ICL_PORT,g2
        lda     ilthead[g2*8],r5        # Get head forward/back address
        lda     ilttail[g2*8],r6        # Get tail forward/back address
        ldconst 0,r7                    # Zero
        st      r6,il_fthd(r5)          # Set forward pointer in head
        st      r5,il_bthd(r6)          # set backward pointer in tail
        st      r7,il_bthd(r5)          # Set beginning of chain
        st      r7,il_fthd(r6)          # Set end of chain
        ldconst ISPMONPRI,g1            # Priority of the Monitor task
        lda     fsl_Init,g0             # task to start
c fprintf(stderr,"%s%s:%u <isp.as> ICL... forking fsl_Init for ICL port=%u\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)ICL_PORT);
c       CT_fork_tmp = (ulong)"fsl_Init";
        call    K$fork                  # start monitor task

.ispst95:
.endif  # FE_ISCSI_CODE

.if FE_ISCSI_CODE
#
# --- if no FC Interfaces - skip spawning the monitorFwhb task
#
        ld      ispmap,r4               # get ISP found device bitmap
        ld      iscsimap,r7             # Get iscsimap bitmap
        cmpobe  r4,r7,.ispst100         # Jif
.endif  # FE_ISCSI_CODE
#
# --- Start process to monitor firmware heart beats
#
        ldconst ISPMONCPRI,g1           # Process priority
        lda     isp_monitorFwhb,g0      # Start monitor heart beat
c       CT_fork_tmp = (ulong)"isp_monitorFwhb";
        call    K$fork
#
.if FE_ISCSI_CODE
.ispst100:
.endif  # FE_ISCSI_CODE
        ret                             # Done
# End of ISP$start ************************************************************


.ifdef TARGET
#
#******************************************************************************
#
#  NAME: ISP$notify_ack
#
#  PURPOSE:
#
#       To provide a means of acknowledging an Immediate Notify message after
#       it has been processed by the Translation Layer.
#
#  DESCRIPTION:
#
#       This routine receives an ILT pointed to by g1 that contains parameters
#       necessary to perform an Immediate Notify Acknowledge as requested.
#       The parameters in the ILT are as follows:
#
#       Sequence ID of original request
#       Initiator ID of original request
#       Chip Instance
#       LUN of request
#       Task flags from original request
#       Virtual Port of request
#
#       See <iltdefs.inc> for the structure in the ILT used to store these
#       parameters.
#
#       This routine also checks for the locally defined Clear LIP Reset
#       status flag. (See <iocb.inc>).  If this is set the Notify ACK
#       Clear LIP Reset is generated.
#
#       The completion handler used does not check for errors on this request.
#
#
#  CALLING SEQUENCE:
#
#       call    ISP$notify_ack
#
#  INPUT:
#
#       g1 = ILT pointer
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
        .globl ISP_NotifyAck
ISP_NotifyAck:
ISP$notify_ack:
#
# --- Get an IOCB from the queue of this instance
#
        movl    g0,r12                  # save g0/g1
        ldob    inchipi(g1),g0          # get chip ID
#
.if FE_ISCSI_CODE
#
# --- if iSCSI Interfaces - call iSCSI init functions
#
c       r3 = ICL_IsIclPort((UINT8)g0);
        cmpobe TRUE,r3,.isprna_icl0     # Jif port is ICL
#
        ld      iscsimap,r3             # Get iscsimap bitmap
        bbc     g0,r3,.ispna10          # Jif
#
.isprna_icl0:
#
        PushRegs(r3)                    # Save register contents
        movl    r12,g0
        call    fsl_tmf_cb1
        PopRegsVoid(r3)                 # Restore register contents
#
        b       .ispna100
#
.endif  # FE_ISCSI_CODE
.ispna10:
#
        mov     g0,r15                  # Preserve chip ID
        ld      ispstr[g0*4],r14        # Get address of ISP struct
#
        ld      isprqip(r14),r10        # get Request Queue IN pointer address
#
        mov     r14,g1                  # setup params for <isp$get_iocb>
#
c       {
c       UINT32 tmp;
c       UINT16 *iocb;
c       iocb = isp_get_iocb(g0,&tmp);
c       g1 = (UINT32)iocb;
c       g0 = tmp;
c       }
        cmpobe  0,g1,.ispna100          # Jif no IOCB obtained
#
# --- g1 = pointer to IOCB; g0 = offset into queue for new IN pointer
#     r12 = orig. g0 r13 = pointer to ILT

        PushRegs(r4)                    # preserve g0 g1 g2
        mov     r15, g0                 # chip ID
        mov     g1, g2                  #iocb
        mov     r13,g1                  #ilt
        call    isp2400_build_ntack     # fill in iocb
        PopRegsVoid(r4)                 # restore regs

        ldconst 30,r9
        st      r9,i_timeout(r13)       # Set timeout counter

# --- Done with Notify Ack.  Now place on queue.
#
# --- Place offset onto submission queue for this instance

c       FORCE_WRITE_BARRIER;
        stos    g0,(r10)                # put on IN queue
#
# --- Thread ILT onto linked list for this instance r15 = chip instance
#     r13 = ILT address
#
        mov     r15,g0                  # g0 = chip instance
        mov     r13,g1                  # g1 = ILT address
c       isp_thread_ilt(g0, g1);         # Thread this ILT onto list.
.ispna100:
        movl    r12,g0                  # Restore g0
        ret
# End of ISP$notify_ack *******************************************************

.endif # TARGET


#******************************************************************************
#
#  NAME: ISP_reset_chip
#
#  PURPOSE:
#
#       Resets and optionally reinitializes a particular ISP instance.
#
#  DESCRIPTION:
#
#       This routine is used to completely reset and reinitialize an ISP
#       instance. This is accomplished with the following steps:
#
#       1).  A Loop Down event is generated for this instance.
#       2).  The ISP interrupt vector is masked to prevent interrupt servicing
#            during the reset process.
#       3).  Software reset is generated for the ISP instance.
#       4).  The interrupt is unmasked.
#       5).  The Session ID is incremented.
#       6).  ISP$initialize is called for the particular chip.
#       7).  The ILT thread for the ISP instance is traversed and each ILT
#            completion routine is called with a "dummy" IOCB indicating command
#            aborted.
#       8).  Any QRPs waiting for execution are marked in error and returned to
#            the calling task.
#
#  CALLING SEQUENCE:
#
#       call    ISP_reset_chip
#
#  INPUT:
#
#       g0 = ISP instance (0 - MAXISP-1)
#       g1 = Reason code (-1 = Reset Only)
#
#  OUTPUT:
#
#       g0 = 0 = Successfully reset
#            n = failed - error code
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
ISP_reset_chip:
ISP_ResetChip:
#
        movl    g0,r14                  # Preserve ISP instance, reason code
c fprintf(stderr, "%s%s:%u ISP_reset_chip: g0=0x%08lX, g1=0x%08lX\n", FEBEMESSAGE, __FILE__, __LINE__, g0, g1);
.ifdef ISP_DEBUG_FWSTATE
        PushRegs(r3)                    # Save register contents
c fprintf(stderr, "%s%s:%u ISP_reset_chip: ISP firmware state = 0x%04X\n", FEBEMESSAGE, __FILE__, __LINE__, ISP_GetFirmwareState(g0));
        PopRegs(r3)                     # Restore register contents
.endif  # ISP_DEBUG_FWSTATE
.if     DEBUG_FLIGHTREC_I
c       MSC_FlightRec(FR_ISP_RESET_ENTER,g0,g1,0);
.endif  # DEBUG_FLIGHTREC_I
#
        mov     sp,r11                  # allocate stack frame
        lda     60(sp),sp
        stq     g0,(r11)                # save g0-g3
        stq     g4,16(r11)              # save g4-g7
        stq     g8,32(r11)              # save g8-g11
        stt     g12,48(r11)             # save g12-g14
#
        st      0,(r11)                 # Preset return status to GOOD
#
.if FE_ISCSI_CODE
#
# --- if iSCSI Interfaces - call iSCSI init functions
#
c       r3 = ICL_IsIclPort((UINT8)r14);
.if ICL_DEBUG
        cmpobne TRUE,r3,.isprst_icl11
c fprintf(stderr, "%s%s:%u <ISP_ResetChip>ICL.. calling fsl_ResetPort\n", FEBEMESSAGE, __FILE__, __LINE__);
.isprst_icl11:
.endif  # ICL_DEBUG
        cmpobe  TRUE,r3, .isprst_icl0   # jif port is icl
        ld      iscsimap,r3             # Get iscsimap bitmap
        bbc     r14,r3,.isprst1         # Jif
#
.isprst_icl0:
#
        PushRegs(r3)                    # Save register contents
        movl    r14,g0
        call    fsl_ResetPort
        st      g0,(r11)                # Preset return status
        PopRegsVoid(r3)                 # Restore register contents
        b       .isprst500
#
.isprst1:
.endif  # FE_ISCSI_CODE
#
#
# --- Check if a reset is already in progress.
#
        ld      resilk,r3
        bbs     r14,r3,.isprst500       # Jif if being reset
#     Set reset interlock first, so we know the chip reset generated the event.
        ld      resilk,r3               # Set reset interlock bit
        setbit  r14,r3,r3
        st      r3,resilk
#
# --- Check if reason code indicated a user requested reset
#

        cmpobe  ecrild,g1,.isprst40     # Jif Reset and initialize if offline
        cmpobe  ecrold,g1,.isprst40     # Jif Reset only if offline
        ldconst mleispfatal,r3          # We are in a forced system error (qlogic),
                                        # or we are attempting to load fw into iscsi port.
        cmpobe  ecrfatal,g1,.isprst30   # Jif fatal error
        cmpobge ecrold,g1,.isprst50     # Jif non-reportable reason code
        ldconst aspsye,r3
        cmpobne r3,g1,.isprst10         # Jif not system error (0x8002)
#
# --- Increment the system error (AEN 0x8002) count.
#
c fprintf(stderr, "%s%s:%u Increment system error count\n", FEBEMESSAGE, __FILE__, __LINE__);
        ld      ispSysErr[r14*4],r4
        addo    1,r4,r4
        st      r4,ispSysErr[r14*4]     # Increment count
#
        ldconst 2,r5                    # Allow only two error.
        ld      mpn+mpn_t4,r6           # Get T4.
        b       .isprst20
#
.isprst10:
        cmpobne ecrdebugro,r15,.isprst15 # Jif not Debug Reset only
        ldconst mleportinitfailed,r3    # Port initialization failure
        b       .isprst30
#
# --- Increment the port failure reset count.
#
.isprst15:
        ld      ispprc[r14*4],r4
        addo    1,r4,r4
        st      r4,ispprc[r14*4]        # Increment count
#
        ld      mpn+mpn_err_limit,r5    # Get Error limit.
        ld      mpn+mpn_t4,r6           # Get T4.
#
# --- Determine the log event type
#
.isprst20:
        ldconst mleispchipreset,r3      # ISP chip reset log event
        cmpobe  0,r6,.isprst30          # Jif T4 is zero
        cmpobl  r4,r5,.isprst30         # Jif below notification threshold
        ldconst mleportevent,r3         # Port event notification log event
#
# --- Generate log event
#
.isprst30:
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        stos    r14,ecr_port(g0)        # Store port number
.ifdef FRONTEND
        ldconst 0,r3                    # Indicate Front End
.else   # FRONTEND
        ldconst 1,r3                    # Indicate Back End
.endif  # FRONTEND
        stos    r3,ecr_proc(g0)         # 0 = FE, 1 = BE
        st      r15,ecr_reason(g0)      # Store reason code
        st      r4,ecr_count(g0)        # Store count
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ecrlen);
#
# --- Fork the port failure handler (note: this clears the error counters...)
#
        ld      isppfpcb[r14*4],r3      # Get port failure handler PCB
        cmpobne 0,r3,.isprst50          # Jif PCB exists
c       g0 = -1;                        # flag task being created.
        st      g0,isppfpcb[r14*4]
#
        lda     isp_portFailureHandler,g0 # g0 = entry pointer
        ld      K_xpcb,r4
        ldob    pc_pri(r4),r3
        subo    1,r3,g1                 # g1 = process priority
        mov     r14,g2                  # g2 = port number
c       CT_fork_tmp = (ulong)"isp_portFailureHandler";
        call    K$tfork
        st      g0,isppfpcb[r14*4]      # Save to indicate task active
        call    K$xchang                # Exchange processes
        b       .isprst50
#
# --- Check if the port is currently online
#
.isprst40:
        ld      ispOnline,r3            # Get online flag
        bbs     r14,r3,.isprst220       # Jif port is online
#
# --- Generate loop down event.
#
.isprst50:
        mov     r14,g0                  # g0 = chip ID
        PushRegs(r3)                    # Save register contents
        call    ISP_LoopDown
        PopRegsVoid(r3)                 # Restore registers
.ifdef FRONTEND #***************************************************** FRONTEND
#
# --- Immediate Notify; generate ILT for passage to Translation Layer ---------
#
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
#
# --- Stuff fields needed for offline event.
#
#     These are:
#
#       Chip Instance
#
        ldconst rinitcmd,r3             # set reset/initialize event
        stob    r3,incommand(g1)        # store command byte
        mov     r14,g0                  # Ensure Chip ID is in g0
        stob    g0,inchipi(g1)          # Store Chip ID
#
# --- Gen pointer to structure
#
        mov     g1,r3                   # pointer to param structure
#
        ldconst 0,r5                    # zero
        stob    r5,idf_rnc(g1)          # Clear request completed flag
        st      r5,il_cr(g1)            # Clear completion routine
#
# --- Move ILT to Translation Layer nesting level
#
        lda     ILTBIAS(g1),g1
#
        st      r5,il_cr(g1)            # Clear completion routine
        st      r3,il_misc(g1)          # store pointer to param struct
#
# --- Invoke Translation Layer (to delete IMTs)
#
        PushRegs(r3)                    # Save register contents
        call    C_recv_scsi_io          # send to Xlation Layer (C-driver)
        PopRegsVoid(r3)                 # Restore registers
#
# --- Notify I-Driver Layer (to delete IMTs)
#
.if     INITIATOR
        mov     r14,g0                  # g0 = chip instance
        call    I_recv_offline          # send to I-driver Layer
.endif  # INITIATOR
.endif # FRONTEND **************************************************** FRONTEND
#
# --- Disable response queue processing
#
        ld      isprena,r4
        clrbit  r14,r4,r4
        st      r4,isprena
#
# --- Increment session ID for this instance (used by async event queue monitor)
#
        ld      sessids[r14*4],r4
        addo    1,r4,r4
        st      r4,sessids[r14*4]

.if     DEBUG_FLIGHTREC_I
c       MSC_FlightRec(FR_ISP_RESET_CHIP,r14,0,0);
.endif  # DEBUG_FLIGHTREC_I

        mov     r14,g0
#

        PushRegs(r3)
        call    ISP2400_ResetChip
        PopRegsVoid(r3)                 # Restore registers
#
        #mov     g0,r5                   # save the return value
        #cmpobne 0,r5,.isprst100         # jif if retvalue not zero<RVISTI>
        #c       fprintf(stderr,"After ISP2400_ResetChip retvalu %lX\n", r5);
        ld      ispstr[r14*4],r4        # Get ptr to ISP structure
        ld      ispbasead(r4),r13       # Get chip base address

        ld      ispresque(r4),r5        # get response queue pointer
        ld      qc_begin(r5),r6         # Get beginning of queue
        st      r6,qc_in(r5)            # Set IN pointer
        st      r6,qc_out(r5)           #   and OUT pointer
#
# --- Reset atio queue pointers to empty (2400 only).
#
        ld      ispatioque(r4),r5       # get atio queue pointer
        ld      qc_begin(r5),r6         # Get beginning of queue
        st      r6,qc_in(r5)            # Set IN pointer
        st      r6,qc_out(r5)           # and OUT pointer
#
# --- Reset async event queue to empty
#
        ld      asyqa[r14*4],r5         # get async event processing QCB anch
        ld      qc_begin(r5),r6         # Get beginning of queue
        st      r6,qc_in(r5)            # Set IN pointer
        st      r6,qc_out(r5)           #   and OUT pointer
#
# --- Clear stall bit (Async event queue full indicator).
#
        ld      ispaywt,r4
        clrbit  r14,r4,r4
        st      r4,ispaywt              # Clear stall bit

        st      0,(r11)                 # Preset return status to GOOD
#
# --- Traverse the target list for this port & invalidate the port assignments.
#     Note that Control Ports (CP's) use a target ID of 0xFFFF.
#
        ld      tar[r14*4],r3           # Get TARget structure anchor for port
        cmpibe  0,r3,.isprst108         # Jif nonexistent
        lda     MAXTARGETS-1,r6         # Set max. valid target ID value
        lda     0xFF,r4                 # Set invalid target ID value
.isprst106:
        ldos    4(r3),r10               # Get target ID
        cmpobg  r10,r6,.isprst107       # Jif target ID > max. valid (CP)
        stob    r4,ispPortAssignment(r10) # Invalidate the port assignment
.isprst107:
        ld      (r3),r3                 # Follow forward thread to next target
        cmpibne 0,r3,.isprst106         # Jif next target exists
#
# --- Check ILT queue for chip instance; detach for later processing.
#
.isprst108:
# We must get the ILT's before getting ilthead/ilttail lists, because there is
# remote chance that there might be a task switch - and the detached list would
# be in a bad state and potentially cause isp_unthread_ilt to reset everything
# when it is doing it's forward/backward checks for a good ILT list.

# Get "dummy" head of list.
c       r5 = get_ilt();                 # Allocate a dummy head ILT and preserve in r5
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r5);
.endif # M4_DEBUG_ILT
# Get "dummy" tail of list.
c       g1 = get_ilt();                 # Allocate a dummy tail ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT

        lda     ilthead[r14*8],r3       # Get head of list
        lda     ilttail[r14*8],r10      # Get tail of list
        ld      il_fthd(r3),r12         # Check first entry
c       if (r10 == r12) {
c           r12 = 0;                    # Clear list pointer -- flag empty.
c           put_ilt(r5);                # We don't need either ILT.
c           put_ilt(g1);                # We don't need either ILT.
            b   .isprst110              # Jump if list is empty
c       }
#
# --- List exists; detach it r12 = list head r10 = tail pointer
#
        ld      il_bthd(r10),r6         # Get end of list
        mov     0,r13
        st      r10,il_fthd(r3)         # Set forward pointer in head  -- ilthead(fthd) -> ilttail
        st      r3,il_bthd(r10)         # set backward pointer in tail -- ilttail(bthd) -> ilthead
        st      r13,il_bthd(r3)         # Set beginning of chain       -- ilthead(bthd) -> 0
        st      r13,il_fthd(r10)        # Set end of chain             -- ilttail(fthd) -> 0
        st      r12,il_fthd(r5)         # r5 = dummy head ILT; set fwd -- iltd1(fthd) -> firstilt
        st      r5,il_bthd(r12)         #  and back ptr                -- firstilt(bthd) -> iltd1
        st      r6,il_bthd(g1)          # g1 = dummy tail ILT; set fwd -- iltd2(bthd) -> lastilt
        st      g1,il_fthd(r6)          #  and back ptr                -- lastilt(fth) ->iltd2
        mov     g1,r6                   # Preserve ILT pointer to dummy tail in r6
#
# --- Check for QRPs outstanding; detach thread if exists
#
.isprst110:
        ldconst 0,r13
        ld      ispdefq[r14*8],r8       # get head of deferred queue
        st      r13,ispdefq[r14*8]      # Clear deferred queue
        st      r13,ispdefq+4[r14*8]    # clear tail pointer of deferred queue
#
# --- Chip RISC processor reset; clear pending interrupt.
#
c       {
c           int return_code, irq_mask = (int)(1 << r14);
c           return_code = CT_write(CT_xiofd, &irq_mask, sizeof (irq_mask));
c           if (return_code != sizeof (k_processed)) {
c fprintf(stderr, "%s%s:%u XIO3D write returned %d, errno %d\n", FEBEMESSAGE, __FILE__, __LINE__, return_code, errno);
c                       perror ("XIO3D write error");
c           }
c       }
#
# --- Check if the soft reset was successful; if not, set this port as failed.
#
        ld      (r11),g0                # Fetch return code
        cmpobe  0,g0,.isprst115         # Jif still GOOD

        ld      ispmap,r3               # Get ISP found device map
        clrbit  r14,r3,r3
        st      r3,ispmap               # Save device bitmask

        ld      ispfail,r3
        setbit  r14,r3,r3
        st      r3,ispfail              # Save failed device bitmask

        b       .isprst130              # Skip the initialization check
#
# --- Check if the Qlogic Chip is to be re-initialized
#
.isprst115:
        cmpobe  ecro,r15,.isprst130     # Jif Reset Only
        cmpobe  ecrold,r15,.isprst130   # Jif Reset only if offline
        cmpobe  ecrfatal,r15,.isprst130   # Jif fatal error
        cmpobe  ecrdebugro,r15,.isprst130 # Jif Debug Reset only
        cmpobne ecrild,r15,.isprst120   # Jif Not Reset and initialize if offline
        ld      mpn+mpn_t3,g0           # Get Delay before initialize timer.
        cmpobe  0,g0,.isprst120         # Jif zero delay
        call    K$twait
#
# --- Re-enable interrupts & re-initialize the Qlogic Chip
#
.isprst120:
        ldconst 0,g13                   # Setup constant zero register
        mov     r14,g0                  #  and chip instance
        call    ISP$initialize          # Initialize chip
        st      g0,(r11)                # Save return value
#
# --- If an ILT chain exists, return all ILTs with aborted status. ------------
#
.isprst130:
        cmpobe  0,r12,.isprst170        # Jif list empty
#
# --- List exists; allocate dummy IOCB and process all ILTs.
#
c       r9 = s_MallocC(iocbsiz, __FILE__, __LINE__); # Allocate "dummy" IOCB
#
# --- r8 = QRP thread
#     r5 = Dummy ILT pointer (head), r6 = Dummy ILT pointer (tail)
#
# It is possible that the first ILT was removed with all the task switches in
# ISP$initialize(), and potential with s_MallocC().
        ld      il_fthd(r5),g1
c       if (g1 != r12) {
c         fprintf(stderr, "%s%s:%u M4 ISP_ResetChip validated.\n", FEBEMESSAGE, __FILE__, __LINE__);
c       }
#
# --- Setup error code
        ldconst iocbabt,r3              # Set command aborted error code
        stos    r3,0x08(r9)             # Setup error code
#
.isprst140:
c       g1 = isp_unthread_ilt_1(g1);    # Take off our dummy list before processing it
c   if (g1 == 0) {
c       fprintf(stderr, "%s%s:%u .isprst140 got invalid ilt from isp_unthread_ilt_1\n", FEBEMESSAGE, __FILE__, __LINE__);
        b       .isprst165              # invalid ILT, cleanup - attempt to keep running.
c   }
        ldob    otl3_type(g1),r4        # Get command type from ILT
        cmpobe  ntack,r4,.isprst145     # Jif notify acknowledge IOCB
        cmpobe  iocb_with_wait_ilt_type,r4,.isprst155      # send iocb and wait
.ifdef BACKEND
        cmpobne cmio7,r4,.isprst145     # Jif not Command Type 7 IOCB
#
# --- Store "IOCB returned because of reset port" in PRP status
#
        ldconst ecrstport,g0            # IOCB returned because of reset port
        ldconst 0x29,r4                 # Device unavailable status
        ld      r_prp-ILTBIAS-ILTBIAS(g1),r13   # Get pointer to PRP
        stob    g0,pr_rstatus(r13)      # Update PRP request status
        stob    r4,pr_qstatus(r13)      # Update PRP qlogic status
.endif  # BACKEND
.isprst145:
        ldconst ecrstport,g0            # IOCB returned because of reset port
        lda     -ILTBIAS(g1),g1         # Get previous level of ILT
.isprst147:
        ld      il_cr(g1),r4            # Get completion routine address
.if     DEBUG_FLIGHTREC_I
c       MSC_FlightRec(FR_ISP_RESET_CLEARILT,r14,g1,r4);
.endif  # DEBUG_FLIGHTREC_I
        mov     0,g13
#
        mov     r9,g11                  # Setup IOCB address
        callx   (r4)                    # Call completion routine
        b       .isprst160
#
# .isprst150:
# .ifdef M4_DEBUG_ILT
# c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
# .endif # M4_DEBUG_ILT
# c       put_ilt(g1);                    # Deallocate ILT
#         b       .isprst160
#
.isprst155:
        ldconst ecrstport,g0            # IOCB returned because of reset port
        lda     -ILTBIAS(g1),g1         # Get previous level of ILT (lvl0)
        st      g0,il_w1(g1)            # save status
        b       .isprst147
#
.isprst160:
# ILT was unlinked, so dummy head ILT now points to the next ILT (or dummy tail).
        ld      il_fthd(r5),g1
        cmpobne g1,r6,.isprst140        # Jif more to process
#
.isprst165:
# --- Deallocate "dummy" IOCB
c       s_Free(r9, iocbsiz, __FILE__, __LINE__);
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r5);
.endif # M4_DEBUG_ILT
c       put_ilt(r5);                    # Deallocate dummy head ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r6);
.endif # M4_DEBUG_ILT
c       put_ilt(r6);                    # Deallocate dummy tail ILT
#
.isprst170:
#
# --- If a QRP thread exists, return all commands with invalid command status
#     and awaken the tasks waiting for responses.
#
        cmpobe  0,r8,.isprst200         # r8 = head of thread - Jif no thread
#
# --- Force QRP error
#
.isprst180:
.if     DEBUG_FLIGHTREC_I
c       MSC_FlightRec(FR_ISP_RESET_CLEARMB,r14,r8,((QRP*)r8)->imbr[0]);
.endif  # DEBUG_FLIGHTREC_I
        ldconst ismcmdi,r5              # Set invalid QRP command
        stos    r5,qrpombr(r8)          # Store into mailbox reg 0 in QRP
#
# --- Store error code into QRP and activate waiting task
#
        ld      qrppcbr(r8),r4          # get PCB
        ldconst pcrdy,r5                # state to change task to
        ldob    pc_stat(r4),r6          # get task state
        lda     PCB_QLMBX_RSP_WAIT(r14),r7
        cmpobne r7,r6,.isprst190        # jif task not waiting for response
#
# --- set task status to ready
#
.ifdef HISTORY_KEEP
c CT_history_pcb(".isprst180 setting ready pcb", r4);
.endif  # HISTORY_KEEP
        stob    r5,pc_stat(r4)          # activate task
#
.isprst190:
#
# --- Set command completed flag in QRP to TRUE
#
        ldconst TRUE,r4
        stob    r4,qrpstflg(r8)
        ld      qrpfthd(r8),r8          # Get next QRP
        cmpobne 0,r8,.isprst180         # Jif more QRPs to process
#
# --- Check whether the chip was successfully reset & re-initialized.
#
.isprst200:
        ld      (r11),g0                # Fetch return code
        cmpobne 0,g0,.isprst220         # Jif failed

        cmpobe  ecro,r15,.isprst220     # Jif Reset Only
        cmpobe  ecrold,r15,.isprst220   # Jif Reset only if offline
        cmpobne ecrdebugro,r15,.isprst210 # Jif Debug Reset only
#
# --- Set the failed bit for this port.
#
        ld      ispfail,r3
        setbit  r14,r3,r3
        st      r3,ispfail              # Set failed bit for this port
        b       .isprst220
#
# --- Enable response queue processing for this port.
#
.isprst210:
        ld      isprena,r4              # get response queue en flags
        setbit  r14,r4,r4               # Allow response queue processing
        st      r4,isprena              # Set response queue en flags
#
# --- Clear reset interlock bit for this chip.
#
.isprst220:
.if     DEBUG_FLIGHTREC_I
c       MSC_FlightRec(FR_ISP_RESET_RELEASE_LOCK,r14,0,0);
.endif  # DEBUG_FLIGHTREC_I
        ld      resilk,r3
        clrbit  r14,r3,r3               # Clear reset interlock bit
        st      r3,resilk
#
# --- Check the return value & log an error message if nonzero (unsuccessful).
#
.isprst500:
        ld      (r11),g0
        cmpobe  0,g0,.isprst505         # Jif return value is GOOD (zero)
.ifdef FRONTEND
c       r3 = ICL_IsIclPort((UINT8)r14);
        cmpobe  FALSE,r3,.isprst_icl01
c fprintf(stderr, "%s%s:%u <isp.as: ResetChip> ICL port init failed\n", FEBEMESSAGE, __FILE__, __LINE__);

c       ICL_LogEvent(logiclportinitfailed);
        b       .isprst505
#
.isprst_icl01:
.endif #FRONTEND
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleportinitfailed,r3    # Port Initialization Failed log event
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        stos    r14,ecr_port(g0)        # Store port number
.ifdef FRONTEND
        ldconst 0,r3                    # Indicate Front End
.else   # FRONTEND
        ldconst 1,r3                    # Indicate Back End
.endif  # FRONTEND
        stos    r3,ecr_proc(g0)         # 0 = FE, 1 = BE
        st      r15,ecr_reason(g0)      # Store reason code
        st      0,ecr_count(g0)         # Store count of 0
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ecrlen);
#
# --- Exit, return value in g0.
#
.isprst505:
        ldq     (r11),g0                # restore g1-g3 (g0 = return value)
        ldq     16(r11),g4              # restore g4-g7
        ldq     32(r11),g8              # restore g8-g11
        ldt     48(r11),g12             # restore g12-g14
        ret
#
.ifdef RESET_DEBUG
#
reset_task:
#
.restsk10:
        ldconst 30000,g0                # Time interval
        call    K$twait
#
        ldconst 1,g0                    # Designate chip 1 to reset
        ldconst ecro,g1                 # Reason code = Reset Only
        call    ISP_reset_chip          # Reset the chip

        ldconst 2,g0                    # Designate chip 2 to reset
        ldconst ecro,g1                 # Reason code = Reset Only
        call    ISP_reset_chip          # Reset the chip

        ldconst 10000,g0                # Time interval
        call    K$twait

        ldconst 1,g0                    # Designate chip 1 to reset
        ldconst ecri,g1                 # Reason code = Reset/Initialize
        call    ISP_reset_chip          # Reset the chip

        ldconst 2,g0                    # Designate chip 2 to reset
        ldconst ecri,g1                 # Reason code = Reset/Initialize
        call    ISP_reset_chip          # Reset the chip
        b       .restsk10
.endif  # RESET_DEBUG
#
# End of ISP_reset_chip ******************************************************


.ifdef FRONTEND
#******************************************************************************
#
#  NAME: ISP$is_my_WWN
#
#  PURPOSE: To determine if the specified World Wide Name
#           belong to primary ports or virtual ports of
#           the Qlogic cards in the system.
#
#  DESCRIPTION:  Scans all the tar structure looking for a matching
#                World Wide Port Name.
#
#  CALLING SEQUENCE:
#
#       call    ISP$is_my_WWN
#
#  INPUT:
#
#       g0 = Ptr to Port WWN ID.
#
#  OUTPUT:
#
#       g0 = TRUE - target found.
#            FALSE - target not found.
#
#  REGS DESTROYED:
#
#       g0
#
#******************************************************************************
#
# UINT32 ISP_IsMyWWN(void *);
    .globl ISP_IsMyWWN
ISP_IsMyWWN:
ISP$is_my_WWN:

        ldl     (g0),r14                # Get WWN
        ldconst FALSE,g0                # Initialize return value
#
# --- Examine all targets on all channels for a matching WWN.
#
        ldconst 0,r9                    # Start with Channel 0
.imt10:
        ld      tar[r9*4],r8            # Get TAR anchor
        cmpobe  0,r8,.imt40             # Jif no targets on this channel
#
# --- Compare port WWN
#
.imt20:
        ldob    taropt(r8),r4           # Get target options
        bbc     tarena,r4,.imt30        # Jif target not enabled
        ldl     tarptn(r8),r4           # Get Port WWN
        cmpobne r4,r14,.imt30           # Jif no match
        cmpobne r5,r15,.imt30           # Jif no match
        ldconst TRUE,g0                 # Port ID found
        b       .imt50
#
# --- Increment to the next target structure.  If none,
#     increment to the next channel.
#
.imt30:
        ld      tarfthd(r8),r8          # Get the next TAR structure
        cmpobne 0,r8,.imt20             # Loop for more VPs
.imt40:
        addo    1,r9,r9                 # Increment ISP instance
        cmpobg  MAXISP,r9,.imt10
.imt50:

        ret
.endif /* FRONTEND */


.ifdef FRONTEND
#******************************************************************************
#
#  NAME: ISP$is_my_ALPA
#
#  PURPOSE: To determine if the specified ALPA
#           belong to primary ports or virtual ports of
#           the the specified Qlogic card.
#
#
#  DESCRIPTION:  Scans the tar structure looking for a matching LID
#
#
#  CALLING SEQUENCE:
#
#       call    ISP$is_my_ALPA
#
#  INPUT:
#
#       g0 = channel.
#       g1 = LID.
#
#  OUTPUT:
#
#       g0 = TRUE - target found.
#            FALSE - target not found.
#
#  REGS DESTROYED:
#
#       g0
#
#******************************************************************************
#
ISP$is_my_ALPA:

        ld      tar[g0*4],r8            # Get TAR anchor
        ldconst FALSE,g0                # Initialize return value
#
# --- Check if TAR structure exists
#
.iml10:
        cmpobe  0,r8,.iml30             # Jif no targets on this channel
#
# --- Check if target is enabled
#
        ldob    taropt(r8),r4           # Get target options
        bbc     tarena,r4,.iml20        # Jif target not enabled
#
# --- Compare LID
#
        ldob    tarportid(r8),r4        # Get alpa just low byte
        cmpobne r4,g1,.iml20            # Jif no match
        ldconst TRUE,g0                 # alpa found
        b       .iml30
#
# --- Increment to the next target structure.
#
.iml20:
        ld      tarfthd(r8),r8          # Get the next TAR structure
        b       .iml10                  # Loop for more TARs
#
.iml30:
        ret
.endif /* FRONTEND */

.ifdef FRONTEND
#
#******************************************************************************
#
#  NAME: ISP$ilt_thread_find
#
#  PURPOSE:
#
#       Checks to see if the given ILT is on the ISP ILT threads
#
#  DESCRIPTION:
#
#  CALLING SEQUENCE:
#
#       call       ISP$ilt_thread_find
#
#  INPUT:
#
#       g0 = QLogic chip instance ordinal (0-3).
#       g1 = ILT address at FCAL nesting level.
#
#  OUTPUT:
#
#       g0 = ILT pointer passed in if found, NULL otherwise
#
#  OUTPUT:
#
#       None
#
#  REGS DESTROYED:
#
#       None
#
#******************************************************************************
#
ISP$ilt_thread_find:
        lda     ilthead[g0*8],r15       # Get origin for this instance
        lda     ilttail[g0*8],r14       # Get tail for this instance
#
        ldconst 0,g0                    # set return value to fail
#
.ispfth10:
        ld      il_fthd(r15),r15        # get next ILT in list
        cmpobe  r14,r15,.ispfth100      # Jif end of chain
        cmpobne g1,r15,.ispfth10        # Jif not the find ILT
#
# --- Found the ILT in the thread
#
        mov     g1,g0                   # Success - return the ILT
#
.ispfth100:
        ret
.endif /* FRONTEND */

#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
