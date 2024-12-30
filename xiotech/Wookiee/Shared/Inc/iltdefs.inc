# $Id: iltdefs.inc 161068 2013-05-14 19:05:20Z marshall_midden $
#******************************************************************************
#
#  NAME: iltdefs.inc
#
#  PURPOSE:
#
#       To define ILT parameter structures for passing between layers.
#       ILT parameter structures are used to manage an operation as
#       it passes through various processing levels as well as to
#       manage an operation at specific levels of processing. An
#       understanding of the flow of processing of operations is
#       helpful in understanding how the ILT parameter structures
#       are used. Two directions of processing flow are defined:
#
#       1). Inbound processing - processes an operation from the
#               receipt of the operation at the Fibre Channel level
#               through to the MAGNITUDE processor.
#       2). Outbound processing - processes an operation from the
#               MAGNITUDE processor through to the Fibre Channel
#               interface.
#
#       The designation here of inbound or outbound does not represent
#       the direction of data transferred between the initiator and the
#       target. It represents the flow of processing within the Fibre
#       Channel Interface (FCI) processor domain.
#
#       Three inbound levels are defined. Inbound level 1 refers to the
#       FC-AL driver level which receives a Fibre Channel event that
#       initiates an operation on the FCI processor. The FC-AL driver
#       builds a data structure at this level which can be referenced
#       at many other levels. This structure therefore is passed along
#       the various levels where needed. Inbound level 2 refers to the
#       device driver level (translation level) which must manage the
#       operation through many other levels of processing. Inbound level 3
#       refers to the MAGNITUDE interface level which sends a request to
#       the MAGNITUDE processor for processing at the MAGNITUDE level.
#
#       Outbound level 1 refers to SRPs received from the MGNITUDE processor
#       for secondary request operations which pertain to a service request
#       sent to the MAGNITUDE processor from the receiving FCI processor.
#       Outbound level 2 refers to the device driver level (translation level)
#       which must process the SRP and schedule operations on the Fibre
#       Channel Interface to perform the necessary operations to satisfy
#       the service request. Outbound level 3 refers to the FC-AL driver
#       which must take these Fibre Channel Interface requests and perform
#       them for the device driver.
#
#       As ILTs are passed between levels of functionality, each processing
#       level uses a level of the ILT for it's own purpose and passes the
#       ILT onto the next level of processing. An ILT can have a mixture
#       of ILT parameter structures within it. An ILT could have inbound
#       level data structures and outbound level data structures imbedded
#       in it. No strict rules are defined.
#
#  Copyright (c) 1996 - 2006 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- INBOUND LEVEL 1 parameter structures ------------------------------------
#
#******************************************************************************
#
#
# --- Fixed Structure fields --------------------------------------------------
#
        .set    idf_cmd,il_w0           # Command byte          [w0] <b>
        .set    idf_ci,idf_cmd+1        # Chip instance         [w0] <b>
        .set    idf_lun,idf_ci+1        # LUN                   [w0] <s>
        .set    idf_init,idf_lun+2      # Initiator ID          [w1] <s>
        .set    idf_vpid,idf_init+2     # Virtual Port ID       [w1] <s>
        .set    idf_exid,idf_vpid+2     # Exchange ID           [w2] <w>
        .set    idf_imt,idf_exid+4      # IMT                   [w3] <w>
        .set    idf_flags,idf_imt+4     # Flags                 [w4] <s>
        .set    idf_status,idf_flags+2  # Status                [w4] <s>
        .set    idf_cdb,idf_imt+4       # iSCSI CDB             [w4] <w>
        .set    idf_taskc,idf_cdb+4     # Task codes            [w5] <b>
        .set    idf_execc,idf_taskc+1   # Execution Codes       [w5] <b>
        .set    idf_taskf,idf_execc+1   # Task flags            [w5] <b>
        .set    idf_rnc,idf_taskf+1     # Request not completed [w5] <b>
        .set    idf_size,idf_imt+4      # Size of fixed iltdef area
        .set    inl1_cr,il_cr           # Inbound level 1 cr         <w>
#
# --- End Fixed Structure fields ----------------------------------------------
#
#
# --- FC-AL layer to Translation Layer structures -----------------------------
#
#
# --- Immediate Notify Structure
#
# --- Begin structure
#
        .set    incommand,idf_cmd       # Command byte          [w0] <b>
        .set    inchipi,idf_ci          # Chip Instance         [w0] <b>
        .set    inlun,idf_lun           # LUN                   [w0] <s>
        .set    ininitiator,idf_init    # Initiator ID          [w1] <s>
        .set    invpid,idf_vpid         # Virtual Port ID       [w1] <s>
        .set    inseqid,idf_exid        # Sequence ID           [w2] <w>
#
        .set    inflags,idf_flags       # Flags                 [w4] <s>
        .set    instatus,inflags+2      # Status                [w4] <s>
#
        .set    intaskf,idf_taskf       # Task Flags            [w5] <b>
#
# --- End structure
#
# --- SCSI CDB command Structure
#
        .set    sccommand,idf_cmd       # Command byte          [w0] <b>
        .set    scchipi,idf_ci          # Chip Instance         [w0] <b>
        .set    sclun,idf_lun           # LUN                   [w0] <s>
        .set    scinitiator,idf_init    # Initiator ID          [w1] <s>
        .set    scvpid,idf_vpid         # Virtual Port ID       [w1] <s>
        .set    scrxid,idf_exid         # RX_ID                 [w2] <w>
#
        .set    sccdb,idf_cdb           # CDB pointer           [w4] <w>
        .set    sctaskc,sccdb+4         # Task Codes            [w5] <b>
        .set    scexecc,sctaskc+1       # Execution Codes       [w5] <b>
        .set    sctaskf,idf_taskf       # Task Flags            [w5] <b>
#
        .set    scdatalen,scexecc+3     # Data length           [w6] <w>
#
#
# --- Magnitude VRP command Structure
#
        .set    vrcommand,idf_cmd       # Command byte          [w0] <b>
        .set    vrchipi,idf_ci          # Chip Instance         [w0] <b>
        .set    vrlun,idf_lun           # LUN                   [w0] <s>
        .set    vrinitiator,idf_init    # Initiator ID          [w1] <s>
        .set    vrxid,idf_exid          # RX_ID                 [w2] <w>
#
        .set    vrvrp,sccdb             # VRP pointer           [w4] <w>
        .set    vrsrp,vrvrp             # SRP pointer           [w4] alias
        .set    vrtaskc,sctaskc         # Task Codes            [w5] <b>
        .set    vrexecc,scexecc         # Execution Codes       [w5] <b>
        .set    vrtaskf,idf_taskf       # Task Flags            [w5] <b>
#
        .set    vrdatalen,scdatalen     # Data length           [w6] <w>
#
# --- End structure
#
#
#******************************************************************************
#
# --- INBOUND LEVEL 2 parameter structures ------------------------------------
#
#******************************************************************************
#
#
# --- Inbound Level 2
# --- Used to manage inbound CDBs (tasks) as they are processed.
# --- Used to manage operations sent to the MAGNITUDE.
# --- Also referred to as the XL nest level.
#
# --- Task state code (inl2_eh_tstate) definitions
#
        .set    inl2_ts_dorm,0          # 0 = dormant
        .set    inl2_ts_enbl,1          # 1 = enabled
        .set    inl2_ts_blk,2           # 2 = blocked
        .set    inl2_ts_abt,3           # 3 = aborted
#
# --- Process state code (inl2_pstate) definitions
#
        .set    inl2_ps_wt0,0           # 0 = waiting to begin processing
        .set    inl2_ps_req,1           # 1 = initial request to MAGNITUDE
        .set    inl2_ps_srpblk,2        # 2 = srp blocked wait
        .set    inl2_ps_srpact,3        # 3 = srp active on channel
        .set    inl2_ps_srpcblk,4       # 4 = srp completed & blocked
        .set    inl2_ps_srpcomp,5       # 5 = srp completed & returned to MAG
        .set    inl2_ps_respblk,6       # 6 = response received from MAG and
                                        #     task blocked
        .set    inl2_ps_finalio,7       # 7 = final I/O request (either data
                                        #     transfer w/ending status or just
                                        #     ending status)
        .set    inl2_ps_datatr,8        # 8 = data transfer for immediate type
                                        #     commands (w/o ending status)
#
# --- Task type code (inl2_ttype) definitions
#
        .set    inl2_tt_sim,0           # 0 = simple task
        .set    inl2_tt_hoq,1           # 1 = head-of-queue task
        .set    inl2_tt_ord,2           # 2 = ordered task
                                        # 3 = (reserved)
        .set    inl2_tt_aca,4           # 4 = ACA task
        .set    inl2_tt_untag,5         # 5 = untagged task
#
# --- Event handler table (inl2_ehand) data structure definition
#
        .set    inl2_eh_tstate,0        # task state code               <b>
        .set    inl2_eh_iocomp,4        # task I/O completion           <w>
                                        #  handler routine
        .set    inl2_eh_magcomp,8       # MAGNITUDE request completion  <w>
                                        # handler routine
        .set    inl2_eh_srpreq,12       # SRP request handler routine   <w>
        .set    inl2_eh_srpcomp,16      # SRP request completion handler<w>
                                        #  routine
        .set    inl2_eh_abort,20        # abort task handler routine    <w>
        .set    inl2_eh_reset,24        # reset event handler routine   <w>
        .set    inl2_eh_offline,28      # offline event handler routine <w>
                                        #  (includes offline, link failure
                                        #   etc....)
        .set    inl2_eh_aca,32          # ACA occurred handler routine  <w>
        .set    inl2_eh_acaclr,36       # ACA cleared handler routine   <w>
#
# --- Begin structure
#
        .set    inl2_FCAL,il_misc       # FC-AL param. pointer     [m1] <w>
        .set    inl2_pstate,il_w0       # Process state code       [w0] <b>
        .set    inl2_cdbctl,il_w0+1     # CDB control byte         [w0] <b>
                                        # Bit 7 =
                                        #     6 =
                                        #     5 =
                                        #     4 =
                                        #     3 =
                                        #     2 = NACA
                                        #     1 = LINK
                                        #     0 = FLAG
        .set    inl2_ttype,il_w0+2      # task type code           [w0] <b>
#                                                               ***********
        .set    inl2_flag1,il_w1        # flag byte #1             [w1] <b>
                                        # Bit 7 =
                                        #     6 =
                                        #     5 =
                                        #     4 =
                                        #     3 =
                                        #     2 =
                                        #     1 = 1=on aborted queue
                                        #     0 = 1=on work queue
        .set    inl2_ecode,il_w1+1      # error code               [w1] <b>
        .set    inl2_lldID,il_w1+2      # lld exchange ID          [w1] <b>
#                                                               ***********
        .set    inl2_ecode2,il_w1+3     # error code part 2        [w1] <b>
#                                                               ***********
        .set    inl2_ilmt,il_w2         # assoc. ILMT address      [w2] <w>
#                                                               ***********
        .set    inl2_ehand,il_w3        # Task event handler table [w3] <w>
#                                                               ***********
        .set    inl2_ehand2,il_w4       # Old task event handler   [w4] <w>
                                        #  table
#                                                               ***********
        .set    inl2_dtreq,il_w5        # data transfer request    [w5] <w>
                                        #  size
#                                                               ***********
        .set    inl2_dtlen,il_w6        # data transferred length  [w6] <w>
#                                                               ***********
        .set    inl2_rcvsrp,il_w7       # Recv. SRP handler        [w7] <w>
                                        #  routine
#                                                               ***********
        .set    inl2_cr,il_cr           # Inbound level 2 cr            <w>
        .set    inl2_cdb,ILTBIAS+il_w4  # temporary copy of CDB       16<b>
#
# --- End structure
#
#******************************************************************************
#
# --- INBOUND LEVEL 3 parameter structures ------------------------------------
#
#******************************************************************************
#
#
# --- Inbound Level 3 (Sent to MAGNITUDE)
# --- Used by MAG interface level for it's purpose.
#
        .set    inl3_FCAL,il_misc       # FC-AL param. pointer       <w>
        .set    inl3_cr,il_cr           # Inbound level 3 cr         <w>
#
#******************************************************************************
#
# --- OUTBOUND LEVEL 1 parameter structures -----------------------------------
#
#******************************************************************************
#
#
# --- Outbound Level 1
# --- Used by MAG interface level to manage SRPs received
# ---  from the MAGNITUDE.
#
        .set    otl1_FCAL,il_w0         # FC-AL param. pointer  [w0] <w>
        .set    otl1_srp,otl1_FCAL+4    # assoc. SRP address    [w1] <w>
        .set    otl1_cmd,otl1_srp+4     # Command byte          [w2] <b>
        .set    otl1_cmpcode,otl1_cmd+1 # Completion code       [w2] <b>
        .set    otl1_srps,otl1_cmpcode+1 # Sizeof SRP           [w2] <s>
        .set    otl1_pciad,otl1_srps+2  # SRP PCI address       [w3] <w>
        .set    otl1_relofset,otl1_pciad+4  # Relative offset   [w4] <w>
        .set    otl1_cr,il_cr           # Outbound level 1 cr        <w>
#
#******************************************************************************
#
# --- OUTBOUND LEVEL 2 parameter structures -----------------------------------
#
#******************************************************************************
#
#
# --- Translation Layer to FC-AL structure
#
        .set    xlcommand,il_w0         # Command byte          [w0] <b>
        .set    xlscsist,xlcommand+1    # SCSI status           [w0] <b>
        .set    xlfcflgs,xlscsist+1     # FC-AL flags           [w0] <b>
#
# --- reserved 1
#
        .set    xlreloff,xlfcflgs+2     # Relative offset       [w1] <w>
        .set    xlsglptr,xlreloff+4     # SGL pointer           [w2] <w>
        .set    xlsnsptr,xlsglptr+4     # Sense data pointer    [w3] <w>
        .set    xlsgllen,xlsnsptr+4     # SGL length in bytes   [w4] <s>
        .set    xlsnslen,xlsgllen+2     # Sense length in bytes [w4] <s>
        .set    xlFCAL,xlsnslen+2       # Primary ILT at INL1   [w5] <w>
                                        #  level
        .set    xl_INL2,xlFCAL+4        # Primary ILT at INL2   [w6] <w>
                                        #  level
        .set    xlreslen,xl_INL2+4      # Residual length       [w7] <w>
        .set    otl2_cr,il_cr           # Outbound level 2 cr        <w>
#
# --- End structure
#
#
# --- Command codes to <isp$receive_io> (xlcommand)
#
        .set    dtxfern,0x00            # No data transfer
        .set    dtxferi,0x10            # Data transfer - back to initiator
        .set    dtxferc,0x11            # Data transfer - initiator to controller
#
# --- Bit definitions for FC-AL flags (xlfcflgs)
#
        .set    xlsndsc,0               # Send SCSI status w/ IO
        .set    xltradr,1               # Translate SGL addresses
                                        #  to PCI address for local
                                        #  HAB memory
#
# --- Value definitions for FC-AL flags (xlfcflgs)
#
        .set    xl_sndsc,0x01           # Send SCSI status w/ IO
        .set    xl_tradr,0x02           # Translate SGL addresses
                                        #  to PCI address for local
                                        #  HAB memory
#
#******************************************************************************
#
# --- OUTBOUND LEVEL 3 parameter structures -----------------------------------
#
#******************************************************************************
#
#
# --- Outbound Level 3
# --- Used by the FC-AL driver to manage a FC-AL operation.
#
        .set    otl3_OTL2,il_misc       # Pointer to outbound level 2<w>
                                        #  data structure
        .set    otl3_cr,il_cr           # Outbound level 3 cr        <w>
        .set    otl3_type,il_w0         # IOCB type             [w0] <s>
        .set    otl3_qst,il_w1          # Qlogic status         [w1] <s>
#
#******************************************************************************
#
# --- INITIATOR OUTBOUND LEVEL 1 structures -----------------------------------
#
#******************************************************************************
#
#       common area
#
        .set    oil1_cr,il_cr               # cr                 [cr] <w>

        .set    oil1_chpid,il_w0            # Chip ID            [w0] <b>
        .set    oil1_tmode,oil1_chpid+1       #   discovery task mode    [w0] <b>
        .set    oil1_lun,oil1_tmode+1         # current LUN        [w0] <b>
        .set    oil1_lpmapidx,oil1_lun+1    # ALPA map index     [w0] <b>
        .set    oil1_pid,oil1_lpmapidx+1    # port ID (ALPA)     [w1] <w>
        .set    oil1_flag,oil1_pid+4        # flag byte          [w2] <b>
        .set    oil1_retry,oil1_flag+1      # retry counter      [w2] <b>
        .set    oil1_lid,oil1_retry+1       # current LID  [w2] <s>
        .set    oil1_dslink,oil1_lid+2      # discovery queue lnk[w3] <w>
        .set    oil1_tmt,oil1_dslink+4      # address of TMT     [w4] <w>
        .set    oil1_ILT,oil1_tmt+4         # possible pri ILT   [w5] <w>
        .set    oil1_snsdata,oil1_ILT+4     # sensedata(sg)      [w6] <w>
        .set    oil1_reslen,oil1_snsdata+4  # residual length(sg)[w7] <w>

        .set    oiflg_LID,1                 # scan only this LID
        .set    oiflg_LUN,2                 # scan only this LUN
        .set    oiflg_STID,3                # Suppress identification of
                                            #  target ID to LLD.
        .set    oiflg_ABRT,6                # Abort the operation
        .set    oiflg_PDBC,7                # port data base change indication
        .set    oiflg_mask,0x80             # MASK - remove all but PDBC
#
# --- oil1_mode (task mode) definitions
#
        .set    oimode_parent,0             # discovery task is the parent
        .set    oimode_child,1              # discovery task is a child
        .set    oimode_iscsi,2              # iscsi discovery task

#
#******************************************************************************
#
# --- INITIATOR OUTBOUND LEVEL 2 structures -----------------------------------
#
#******************************************************************************
#
#
# --- Initiator outbound level 2
#
#     Used to send SCSI CDB's to <ISP$initiate_io>
#
# --- Task state code (inl2_eh_tstate) definitions
#
        .set    oil2_ts_dorm,0          # 0 = dormant
        .set    oil2_ts_enbl,1          # 1 = enabled
        .set    oil2_ts_blk,2           # 2 = blocked
        .set    oil2_ts_abt,3           # 3 = aborted
#
# --- Translation Layer to FC-AL structure

        .set    oil2_cr,il_cr             # Outbound init level 2  [cr] <w>
        .set    oil2_irp,il_misc          # IRP pointer            [m1] <w>

        .set    oil2_tlmt,il_w0           # TLMT pointer           [w0] <w>
        .set    oil2_ismt,oil2_tlmt+4     # ISMT pointer           [w1] <w>
        .set    oil2_tstate,oil2_ismt+4   # task state             [w2] <b>
        .set    oil2_TTC,oil2_tstate+1    # tag type code          [w2] <b>
        .set    oil2_tagID,oil2_TTC+1     # task tag ID            [w2] <s>
        .set    oil2_rtycnt,oil2_tagID+2  # task retry counter     [w3] <b>
                                          # 3 spare bytes          [w3]3<b>
        .set    oil2_Req_ID,oil2_rtycnt+4 # requestor ID           [w4] <w>
        .set    oil2_tmr1,oil2_Req_ID+4   # Timer 1                [w5] <s>
        .set    oil2_tmr2,oil2_tmr1+2     # Timer 2                [w5] <s>
        .set    oil2_ehand,oil2_tmr2+2    # event handler table    [w6] <w>
        .set    oil2_xli,oil2_ehand+4     # pointer to xli         [w7] <w>
#
# --- End structure
#
#
#******************************************************************************
#
# --- OUTBOUND LEVEL 3 parameter structures ------------------------------------
#
#******************************************************************************
#
#
# --- Outbound Level 3
# --- Used by the FC-AL driver to manage a FC-AL operation.
#
        .set    oil3_ptr,il_misc            # pointer to XL data         <w>
        .set    oil3_cr,il_cr               # Outbound level 3 cr        <w>
        .set    oil3_flag,il_w0             # Flag Byte             [w0] <w>
        .set    oil3_xlen,il_w1             # Xfer length           [w1] <w>
        .set    oiflg_ABORT,1               # Abort the request
#
#******************************************************************************
#
# --- Cache Definitions -------------------------------------------------------
#
#******************************************************************************
#
# --- Inbound level 4 - Magdriver to Cache Layer (Thunderbolt)
#
        .set    ilt4_cr,il_cr           # Completion routine
        .set    ilt4_FCAL,il_misc       # Pointer to FCAL struct for incoming
                                        #   request
#
# --- Locally (Level 4, Cache layer) defined values
#
        .set    iocounter,il_w1         # IO outstanding counter        <s>
#       Reserved    1  byte in il_w1
        .set    ioccode,il_w1+3         # IO Completion code            <b>
        .set    iovlen,il_w2            # IO sizeof request
        .set    iobufptr,il_w3          # IO buffer pointer
#
        .set    ioctag,il_w5            # Cache tag stored address      <w>
        .set    iointcr,il_w7           # Intermediate Completion Routine <w>
#
# --- Note: <iovlen> and <iobufptr> accessed by longword instructions!
#
# --- Placeholder values (Cache Layer) Note overlap with ILT definitions!
#      (allows placeholder/ILT to be used interchangeably for some operations)
#
        .set    plilt,il_w4             # ILT pointer
        .set    plctag,il_w5            # Cache tag pointer
        .set    plintent,il_w6          # Placeholder queue intent
        .set    plmcmd,il_w0            # (alias) Mirror command bits
#
# --- Note: code uses stl/ldl instruction to access above two values, do not
#           reorder! (<plilt> and <plctag>)
#
        .set    plcr,il_cr              # Completion routine
        .set    plcrs,il_w7             # Saved completion routine

#
# --- Bitfield definitions ----------------------------------------------------
#
#
# --- <plintent> field bits
#
        .set    INVALIDATE_REQUEST,0    # Queued for invalidation request
        .set    FLUSH_REQUEST,1         # Queued for flush request
        .set    WRITE_REQUEST,2         # Queued for write request
        .set    READ_REQUEST,3          # Queued for read request
#
# --- <plmcmd> field bits
#
        .set    MIRROR_TAG,0            # Mirror tag
        .set    MIRROR_BUFFER,1         # Mirror buffer
#
# --- Both bits set implies mirror buffer before tag
#
# -----------------------------------------------------------------------------
#
# --- Write Cache definitions for ILT input to Virtual Layer
#
        .set    il_rsvd_w0,il_w0        #                       [w0] <w>
        .set    il_rsvd_w1,il_w1        #                       [w1] <w>
        .set    il_tag,il_w2            # Cache tag pointer     [w2] <w>
        .set    il_philt,il_w3          # Placeholder pointer   [w3] <w>
        .set    il_vrp,vrvrp            # VRP pointer           [w4] <w>
        .set    il_rsvd_w5,il_w5        #                       [w5] <w>
        .set    il_rsvd_w6,il_w6        #                       [w6] <w>
        .set    il_rsvd_w7,il_w7        #                       [w7] <w>
#
#
# -----------------------------------------------------------------------------
#
# --- Write Cache definitions for ILT input to DLM
#
        .set    il_wcdlmdrp,il_w0       # DRP pointer                  [w0] <w>
        .set    il_wcdlmreqsize,il_w1   # DRP, Datagram size           [w1] <w>
        .set    il_wcdlmphilt,il_w3     # Placeholder pointer          [w3] <w>
        .set    il_wcdlmtag,il_w4       # Tag chain pointer            [w4] <w>
#
#       The following are also used in chains of tags being mirrored
#
        .set    il_wcdlmtagcnt,il_w5    # Num of tags in chain         [w5] <w>
        .set    il_wcdlmdrpcnt,il_wcdlmtagcnt # Num of DRPs in chain   [w5] <w>
        .set    il_wcdlmdrpec,il_w6     # Composite Error Code of DRPs [w6] <w>
#
#       The following are also used in setting and clearing Write Information
#
        .set    il_wcdlmnvaa,il_w7      # NVA Address                  [w7] <w>
#
#******************************************************************************
#
# --- BACKEND RAID LAYER parameter structures ---------------------------------
#
#******************************************************************************
#
        .set    r_prp,il_w0             # PRP pointer
        .set    r_mir,il_w1             # Mirrored ILT
#--        .set    r_sn,il_w2              # Session node - No longer used.
#
#******************************************************************************
#
# --- BACKEND DLM LAYER parameter structures ----------------------------------
#
#******************************************************************************
#
        .set    dlmi_vrp,il_w0          # VRP pointer
        .set    dlmi_srp,il_w0          # SRP Pointer (alias)
        .set    dlmi_srpvrp,il_w4       # VRP in transferring SRP to FEP
        .set    dlmi_path,il_w7         # Path Number
#
#******************************************************************************
#
# --- BACKEND PHYSICAL LAYER parameter structures -----------------------------
#
#******************************************************************************
#
        .set    p_cmd,il_w0             # SCSI opcode           <b>
#          .set    p_tag,il_w0+1           # Queue tag number      <b>
#          .set    p_ovr,il_w0+2 :1        # Overlap indicator     <s><bit-0>
#          .set    p_wrt,il_w0+2 :1        # Write indicator       <s><bit-1>
#          .set    p_pri,il_w0+2 :14       # Priority (aging)      <s><bit2-15>
        .set    p_sda,il_w1             # SDA w1 & w2           <l>
        .set    p_eda,il_w3             # EDA w3 & w4           <l>
        .set    p_jth,il_w5             # Join thread           <w>
        .set    p_jct,il_w6             # Join count            <w>
        .set    p_scb,il_w6             #  also SCB pointer     <w>
        .set    p_qhd,il_w7             # Exec or DEV Q head    <w>
#
#******************************************************************************
#
# --- BACKEND ISP LAYER parameter structures ----------------------------------
#
#******************************************************************************
#
        .set    i_iocb,il_w0            # IOCB type
        .set    i_ddlst,il_w1           # Pointer to DD list copy
        .set    i_dev,il_w2             # Pointer to DEV record
        .set    i_fcp_iu,il_w3          # pointer to fcp command iu
        .set    i_cmio7cpy,il_w4        # pointer to a copy of cmio7 iocb (debug only)
        .set    i_timeout,il_w6         # ISP timeout
#
#******************************************************************************
#
# --- BACKEND DEFINE LAYER Raid Initialization structure ----------------------
#
#******************************************************************************
#
        .set    ri_rdd,il_w0            # RDD pointer
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
