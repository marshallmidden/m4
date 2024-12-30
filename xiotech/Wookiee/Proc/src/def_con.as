# $Id: def_con.as 161041 2013-05-08 15:16:49Z marshall_midden $
#**********************************************************************
#
#  NAME: def_con.as
#
#  PURPOSE:
#       To provide complete support of configuration definition requests.
#
#  FUNCTIONS:
#  Copyright (c) 1997-2010  Xiotech Corporation. All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  d$crexpvirt             # Create/Expand Virtual device MRP
        .globl  d$deletedevice          # Delete Physical device MRP
        .globl  d$deletevirt            # Delete Virtual device MRP
        .globl  D_convpdd2psd           # Convert PDD to any PSD (C code access)
        .globl  D_allocvdd              # Allocate a virtual device (C code access)
        .globl  D_allocrdd              # Allocate a raid device (C code access)
        .globl  D_freepdd               # Free a physical device
        .globl  D_allocpsd              # Allocate a PSD (C code access)
        .globl  D_rrddpsd               # Delete RDD and associated PSDs
        .globl  D_deleteraid            # Delete RDD and associated PSDs
#
# --- global usage data definitions -----------------------------------
#
        .data
        .align  4
#
# --- local usage data definitions ------------------------------------
#
# --- Default virtual disk name.
d_defaultvname:
        .ascii  "New Disk\0\0\0\0\0\0\0\0"

        .globl vd_vtype
vd_vtype:
        .word 0
#
# --- executable code
#
        .text
#**********************************************************************
#
#  NAME: d$crexpvirt
#
#  PURPOSE:
#       To provide a means of processing the create/expand/test virtual
#       device command issued by the CCB.
#
#  DESCRIPTION:
#       This procedure thoroughly validates the parameters within the
#       MRP packet. If any discrepancies are found, an appropriate
#       error code is returned. If no errors are found, then the operation
#       specified in the MRP is performed.
#
#       In addition, if a test operation is performed with a desired device
#       capacity, the information for that virtual device is set to indicate
#       the actual size that would be used for a create or expand operation.
#
#  CALLING SEQUENCE:
#       call    d$crexpvirt
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       g0.
#
#**********************************************************************
#
d$crexpvirt:
#
# --- Since this function is quite large, there are a few register
# --- usages which will last through the function. They are:
#
# r15 - parameter list pointer
# r14 - pointer to VDX
# r13 - pointer to RDX
# r12 - per drive usage for a RAID creation in allocation units
# r10/r11 - two uses
#         - largest, largest segment and pointer to it
#         - RAID capacity in allocation units
# r8/r9   - smallest, largest segment and pointer to it
#
# r7  - number of allocation units in a RAID line
# r6  - number of user allocation units in a RAID line
#
# g10 - number of drives in the RAID being created
# g9  - depth of the RAID being created
# g6  - RDD created
# g4/g5 - multiplication/division input/output
# g3  - multiplication/division input/output
# g2  - return size
# g1  - error code to be returned
# g0  - CEV pointer
#
# --- Fetch the length and validate it
#
        ld      mr_ptr(g0),r15          # Get parm block pointer
c       sp += 12;
c       *(ulong*)(sp - 4) = g0;         # Save pointer to MRP
c       *(UINT64*)(sp - 12) = 0;        # For test/prepare, save largest found value (0x08 flag).
.cd05:
        lda     V_vddindx,r14           # Get VDX pointer
        lda     R_rddindx,r13           # Get RDX pointer
        ld      mr_rptr(g0),r5          # Get the return block pointer
        ld      mr_len(g0),r4           # Length
#
        ldconst 0,g0                    # Set up the CEV pointer
#
!       ldos    mcr_drives(r15),g10     # Get drive count
        ldconst deinvdrvcnt,g1          # Prep possible error code
        ldconst MAXDRIVES,r3
        cmpobl  r3,g10,.cd1500          # Jif excessive drive count
#
        addo    g10,g10,r3              # Two bytes per drive in length
        addo    mcrsiz,r3,r3            # Add base portion
        ldconst deinvpktsiz,g1          # Prep possible error code
        cmpobne r4,r3,.cd1500           # Invalid packet size
#
        ldconst deinvop,g1              # Prep error code
!       ldob    mcr_op(r15),r3          # Get the operation
        cmpobl  mcrtest,r3,.cd1500      # Jif invalid operation
#
        ldconst desesinprogress,g1      # Prep error code
        ld      S_bgppcb,r4             # Get the PCB of the background process
        ldob    pc_stat(r4),r4          # Get the state of the process
        cmpobne pcnrdy,r4,.cd1500       # Jif not not ready (running)
#
# --- Allocate a CEV structure
#
        mulo    4,g10,g0                # Get size of CEV
        ldconst cevsiz,r4               # Load base size
        addo    r4,g0,g0                # Add base size
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # Allocate space
#
# --- Validate remaining parameters (RAID type, RAID ID, device capacity)
#
        stob    r3,ce_op(g0)            # Save operation
        st      r5,ce_rptr(g0)          # Save return block pointer
        stos    g10,ce_numdaml(g0)      # Save drive count
#
!       ldos    mcr_thresh(r15),r4      # Get min space required per drive
        stos    r4,ce_thresh(g0)        # Set min AU per drive
#
!       ldos    mcr_maxraids(r15),r4    # Get max RAIDs allowed for this op
        ldconst cemaxraid,r3            # Max allowed
        cmpo    r3,r4                   # Select the min of the two
        selg    r3,r4,r4                # r4 has the max RAIDs allowed
        stos    r4,ce_maxraids(g0)      # Set max RAIDs for create/expand
#
        lda     mcr_dmap(r15),r4        # Get drive map address
        st      r4,ce_dmap(g0)          # Save it
#
!       ldob    mcr_flags(r15),r4       # Get the flags
        stob    r4,ce_flags(g0)
#
!       ldob    mcr_minpd(r15),r4       # Get the min PD value
        ldconst MAXPDISKSPERRAID,r3     # Max allowed
        cmpo    r3,r4                   # Select the min of the two
        selg    r3,r4,r4                # r4 has the max RAIDs allowed
        stob    r4,ce_minpd(g0)
#
# --- If this is an expand or create, validate the VID.
#
        ldob    ce_op(g0),r4            # Get opcode
        cmpobe  mcrtest,r4,.cd80        # Jif test
#
        ldconst deinvvid,g1             # Prep error code
        ldconst MAXVIRTUALS,r6          # Maximum value allowed plus one
!       ldos    mcr_vid(r15),r3         # Get the VID to be used
        cmpobge r3,r6,.cd1500           # Jif out of range
#
        ld      vx_vdd(r14)[r3*4],r5    # Get VDD
#
        cmpobe  mcrexpand,r4,.cd10      # Jif an expand
#
        ldconst dedevused,g1            # Prep possible error code
        cmpobne 0,r5,.cd1500            # Jif non-NULL found (VID in use)
        b       .cd80                   # Else VID is OK
#
.cd10:
        cmpobe  0,r5,.cd1500            # Jif undefined
#
        ldconst dedevused,g1            # Prep possible error code
        ld      vd_scdhead(r5),r4       # r4 = SCD's if source of copy
        cmpobne 0,r4,.cd1500            # Jif VD is source in copy op.
#
        ld      vd_dcd(r5),r4           # r4 = DCD's if dest. of copy
        cmpobne 0,r4,.cd1500            # Jif copy dest. VDD
#
# --- Check the RAID count. Since we allow 16 RAIDs per virtual device,
# --- it is safe to check for the availability of all the RAIDs that
# --- we could create in this function. This does mean that if there is
# --- a request to expand a device with 16-ce_maxraids, it will fail due to a
# --- lack of room for more RAIDs even though it could take a few more.
#
        ldob    vd_raidcnt(r5),r4       # Get current segment count
        ldob    vd_draidcnt(r5),r5      # Get current deferred count
        addo    r5,r4,r4                # Get total count in virtual device
        ldos    ce_maxraids(g0),r5      # Get max RAIDs allowed
        addo    r5,r4,r4                # Max we could end up with
        ldconst MAXSEGMENTS,r5          # Assume we need all RAIDs
        ldconst demaxsegs,g1            # Prep possible error code
        cmpobg  r4,r5,.cd1500           # Jif maximum segments

        ld      vx_vdd(r14)[r3*4],r5    # Get VDD again
        ldos    vd_attr(r5),r5          # Get the attributes
        bbs     vdbvlink,r5,.cd80       # Jif vlink
        bbc     vdbasync,r5,.cd70       # Jif not an apool
        mov     g0,r4                   # Save off g0
#
# --    Allow only RAID10 type expansion to an apool.
#
        mov     0,g0                    # clear g0
!       ldob    mcr_rtype(r15),g0       # check raid type of expansion
        ldconst deinvrtype,g1           # prepare error code
#
# --    Allow all other raid types except R5 for Nitrogen
#
        cmpobe  mcrraid5,g0,.cd60       # exit if raid5
#
# --    Check whether the apool can be expanded.
#
        PushRegs(r5)
        mov     0,g0                    # Set apool id
        call    apool_can_expand        # Check for expansion possible
        PopRegs(r5)
        cmpobe  0,g0,.cd60              # Jif can't expand
        mov     r4,g0                   # Restore g0
        b       .cd80                   # Ok to proceed
#
.cd60:
        mov     r4,g0                   # Restore g0
        b       .cd1500                 # Error out
#
# --- Check to make sure this isn't a snapshot
#
.cd70:
        ld      vx_vdd(r14)[r3*4],r5    # Get VDD again
        ld      vd_rdd(r5),r5           # get rdd ptr
        ldob    rd_type(r5),r5
        cmpobe  rdslinkdev,r5,.cd1500   # it is a snapshot so error out
.cd80:
        stos    r3,ce_vid(g0)           # Save it
#
# --- Check for mismatches in the device types being used. This will check
# --- two things. First, if all the drives passes in are the same type and
# --- second, if an expand is mismatching types.
#
        mov     g0,r3
c       g0 = GR_IsCrossGeoLocation((CEV*)g0);
        cmpobe  deok,g0,.cd85           # Jif no cross geolocation
        ld      ce_rptr(r3),r4
        stob    g0,mcr_clflag(r4)       # Save the cross geolocation for the
                                        #  return data
.cd85:
        mov     r3,g0                   # Note: g0 used a dozen or so lines below.

c       g1 = DEF_CheckDeviceType(g0);   # Check that the disk types are same.
        cmpobne deok,g1,.cd1500         # Error out if not good
#
# --- Get the requested device capacity and round it up to the next
# --- multiple of allocation units (AU). This is done by adding one
# --- less than the allocation unit size and dividing by the allocation
# --- unit size and ignoring the remainder. Also save the value in AU
# --- rather than logical blocks. This makes all other calculations
# --- faster.
#
!       ldl     mcr_devcap(r15),g4      # Get device capacity
        ldconst DSKSALLOC-1,r4          # Get allocation size less one
        cmpo    1,0                     # Clear carry
        addc    r4,g4,g4                # Add it into capacity
        addc    0,g5,g5                 # As a long
#
c       *(UINT64*)&g4 = *(UINT64*)&g4 / DSKSALLOC;
        stl     g4,ce_reqdevcap(g0)     # Save the req cap - in alloc units
#
!       ldos    mcr_stripe(r15),r3      # Get stripe size
        stos    r3,ce_sps(g0)           # Save it in the CEV
#
# --- Allocate an RDA to use throughout the function.
#
c       g14 = p_MallocC((MAXDRIVES*4)+4, __FILE__, __LINE__); # Allocate and clear RDA
        st      g14,ce_rda(g0)          # Place RDA pointer into CEV
#
!       ldob    mcr_rtype(r15),g14      # Get RAID type
        stob    g14,ce_rtype(g0)        # Save it
#
# --- Check for a type of RAID and geo-RAID combination that is illegal.
# --- Only R10 is allowed with geo-RAID.
#
        ldconst deinvrtype,g1           # Prep invalid RAID type
        cmpobe  mcrraid10,g14,.cd300    # Jif RAID 10
#
# --- Process each device type. For this pass, determine the RAID type
# --- specific values for depth and sectors per stripe. We cannot
# --- determine the allocation size since we do not know how many drives
# --- will be available on each pass. For example, after the first pass,
# --- one or more drives may not have usable capacity and the drive
# --- count would have to change.
#
        ldconst deinvrtype,g1           # Prep invalid RAID type
        cmpobe  mcrraid10,g14,.cd300    # Jif RAID 10 requested
        cmpobe  mcrraid5,g14,.cd200     # Jif RAID 5 requested
        cmpobe  mcrraid1,g14,.cd150     # Jif RAID 1 requested
        cmpobe  mcrraid0,g14,.cd100     # Jif RAID 0 requested
        cmpobne mcrstd,g14,.cd1500      # Jif not standard requested
#
# --- Process standard device -----------------------------------------
#
        ldconst 1,r3                    # Min drive count is one
        stos    r3,ce_mindrives(g0)     # Save it
#
# --- Fill in other fields in CEV. Device capacity is fine. It may be
# --- too large, but that we will find later.
#
        ldconst 1,r3                    # Set up mirror depth
        stob    r3,ce_depth(g0)
        ldconst 512,r3                  # Set up stripe width for rebuild
        stos    r3,ce_sps(g0)
        b       .cd400
#
# --- Process RAID 0 device -------------------------------------------
#
.cd100:
        ldconst 2,r3                    # Get min drive count
        stos    r3,ce_mindrives(g0)     # Save it in the CEV
#
        ldconst 1,r3                    # Set up mirror depth
        stob    r3,ce_depth(g0)
#
# --- Validate stripe size
#
        call    d$get0stripe            # Get and validate stripe size
        cmpobe  0,g1,.cd400             # Jif no error
        b       .cd1500                 # Jif error
#
# --- Process RAID 1 device -------------------------------------------
#
.cd150:
        stos    g10,ce_mindrives(g0)    # Min drives are equal to depth
#
        ldconst 512,r3                  # Stripe width to use during rebuilding
        stos    r3,ce_sps(g0)
#
        stob    g10,ce_depth(g0)        # Mirror depth
        b       .cd400
#
# --- Process RAID 5 device -------------------------------------------
#
.cd200:
!       ldob    mcr_parity(r15),g9      # Must have at least parity num drives
        stos    g9,ce_mindrives(g0)     # Save it in the CEV
#
# --- Validate stripe size
#
        call    d$get5stripe            # Get and validate stripe size
        cmpobne 0,g1,.cd1500            # Jif error
#
# --- Validate parity (3, 5 or 9 drive)
#
        ldconst deinvparity,g1          # Prep possible error code
!       ldob    mcr_parity(r15),g9      # Get parity
        cmpobe  3,g9,.cd210             # Jif 3 drive
        cmpobe  5,g9,.cd210             # Jif 5 drive
        cmpobne 9,g9,.cd1500            # Jif not 9 drive
#
# --- Fill in additional CEV fields.
#
.cd210:
        stob    g9,ce_depth(g0)         # Set up parity algorithm
        b       .cd400
#
# --- Process RAID 10 device ------------------------------------------
#
.cd300:
#        ldconst 3,r3                    # Get min drive count
!       ldob    mcr_depth(r15),r3       # Get mirror depth
        addo    1,r3,r3                 # Min drives is one greater than depth
        stos    r3,ce_mindrives(g0)     # Save it in the CEV
#
# --- Validate stripe size
#
        call    d$get0stripe            # Get and validate stripe size
        cmpobne 0,g1,.cd1500            # Jif error
#
# --- Validate mirror depth
#
        ldconst deinvdepth,g1           # Prep possible error code
!       ldob    mcr_depth(r15),g9       # Get mirror depth
        cmpobe  0,g9,.cd1500            # Jif no depth
        cmpoble g10,g9,.cd1500          # Jif depth >= drive count
        stob    g9,ce_depth(g0)         # Save it in CEV
#
# --- Now that the common fields are all filled in, check for minimum
# --- drive count and then build the DAML list. Following this,
# --- start the iterative process of building the RAIDs for this
# --- request.
#
.cd400:
        ldconst deinvdrvcnt,g1          # Prep possible error code
        ldos    ce_mindrives(g0),r3     # Get min drive count
        cmpobl  g10,r3,.cd1500          # Jif less than the min required
#
        ldos    ce_numdaml(g0),r6       # Get the count of drives
        ld      ce_dmap(g0),r5          # Get the address of the drive map
#
# --- Now sort the drive list in the increasing order of the LAS
#
c       DEF_SortPDisks((UINT16*)r5, (UINT16)r6);
.cd410:
        subo    1,r6,r6                 # Decrement count of drives to do
        cmpibg  0,r6,.cd500             # Jif done
#
!       ldos    (r5)[r6*2],g9           # Get PID
        ldconst MAXDRIVES,r3            # Exceed max?
        ldconst deinvpid,g1             # Prep error code
        cmpobge g9,r3,.cd1500           # Jif out of range
#
        lda     P_pddindx,r3            # Get PDX
        ld      px_pdd(r3)[g9*4],r4     # Valid PID?
        ldconst denonxdev,g1            # Prep possible error code
        cmpobe  0,r4,.cd1500            # Jif null pointer
#
# --- Validate drive status
#
        ldconst deinopdev,g1            # Prep possible error code
        ldob    pd_devstat(r4),r3       # Get device status
        cmpobne pdop,r3,.cd1500         # Jif not operative
#
# --- Validate drive class
#
        ldconst denotdatalab,g1         # Prep possible error code
        ldob    pd_class(r4),r3         # Get device class

#
# Drives should be labelled unsafe for Raid0 and Raid None
# and data labelled for Raid1/Raid5/Raid10.
# The 7000 can use data drives for RAID0

!       ldob    mcr_rtype(r15), r7
        cmpobe  mcrraid0,r7,.cd411      # Jif Raid 0
        cmpobne mcrstd,r7,.cd412        # Jif not Raid None

.cd411:

        cmpobe  pdndatalab,r3,.cd420    # Jif labelled unsafe data device
.ifdef 0    # MODEL_7000    -- allow data and raid0 on same drive for 7000.
        cmpobe  pddatalab,r3,.cd420    # Jif labelled data device
.endif
        b       .cd1500                 # Jif not unsafe (for Raid 0 only)
#
.cd412:
        cmpobne pddatalab,r3,.cd1500    # Jif not labelled data device
#
# --- Check for foreign drive
#
.cd420:
        ldconst deforeigndev,g1         # Prep possible error code
        ld      K_ficb,r3               # Get system serial number
        ld      fi_vcgid(r3),r3
        ld      pd_sserial(r4),r4       # Get device system serial number
        cmpobne r3,r4,.cd1500           # Jif foreign drive
#
        PushRegs(r3)                    # Save the G registers
        mov     g0,r4                   # CEV
c       g0 = (UINT32)(void*)DA_DAMBuild(g9);   # Build the DAM for this PID (g9)
        st      g0,ce_daml(r4)[r6*4]    # Save the DAML
        PopRegsVoid(r3)                 # Restore all G registers
        b       .cd410                  # Get next one
#
# --- Now start the process of building RAIDs until we have met the
# --- input requirements for a create/expand or until we have exhausted
# --- resources for a test operation. This is the top of a very large
# --- loop for RAID creation. Use care with the registers. They now
# --- all have very long lasting usages.
#
.cd500:
        PushRegs(r3)                    # Save the G registers
        call    DEF_GetRDA              # RDA will now have all usable DAML
        PopRegsVoid(r3)                 # Restore all G registers
#
        ldos    ce_rdadrives(g0),r3     # Are there any drives w/ capacity
        cmpobe  0,r3,.cd1100            # Jif none available
#
        ld      ce_rda(g0),r6           # RDA pointer
        ldconst ~0,r9                   # Smallest seg (r8 will point to it)
        ldconst 0,r11                   # Largest seg (r10 will point to it)
        ldconst 0,g10                   # Pointer into RDA
#
.cd510:
        ld      (r6)[g10*4],r3          # DAML pointer
        cmpobe  0,r3,.cd540             # Jif done w/RDA
#
# --- Check to see if this DAML's largest space is smaller than the current
# --- smallest space. Note that this is in allocation units, so it is only
# --- a word, not a long value.
#
        ld      da_largest(r3),r4       # Get space size
        cmpoble r9,r4,.cd520            # Jif current smallest is smaller
#
        mov     r4,r9                   # Change the smallest to current
        mov     g10,r8                  # Save index
#
# --- Check to see if this DAML is the largest space. Note that even if
# --- it was the smallest, largest, it could also be the largest, largest
# --- if there was only one disk or multiple disks of the same size.
#
.cd520:
        cmpobge r11,r4,.cd530           # Jif current largest is smaller
#
        mov     r4,r11                  # Save the current largest
        mov     g10,r10                 # Save index
#
.cd530:
        addo    1,g10,g10               # Bump index
        b       .cd510                  # Get next one
#
# --- At this point, we have the smallest, largest available segment
# --- in r9 and the entry number in r8. The largest, largest is in
# --- r11 and the index is in r10. See what we can build with this.
#
# --- First, grab a RID.
#
.cd540:
        ldconst MAXRAIDS,r6             # Max raid count
        ldconst 0,g13                   # Pointer into RDX
#
.cd550:
        ld      rx_rdd(r13)[g13*4],r3   # Get RDD
        cmpobne 0,r3,.cd570             # Jif assigned in permanent basis
#
# --- Search the assigned values in the CEV.
#
        ldconst 0,r3                    # Search CEV
#
.cd560:
        ld      ce_rdd(g0)[r3*4],r4     # Get RDD
        cmpobe  0,r4,.cd600             # Null pointer, RID not found
#
        ldos    rd_rid(r4),r5           # Get RID
        cmpobe  r5,g13,.cd570           # Jif RID in use on temp basis
#
        addo    1,r3,r3                 # Bump index
        b       .cd560                  # Try again
#
.cd570:
        addo    1,g13,g13               # Bump pointer
        subo    1,r6,r6                 # Decrement count
        cmpobne 0,r6,.cd550             # Jif more available
        b       .cd1100                 # Else, error out
#
# --- Process each RAID type. At this point, determine the maximum
# --- size RAID that can be created given the smallest, largest area
# --- across all disks. This information is in r9/r8. The largest,
# --- largest is in r11/r10 and would be used for a standard disk.
# --- g13 has the RID we will assign. g14 will have the RAID type,
# --- g4 will have the RDA address and g10 will have the count of
# --- drives to use in the RAID building.
#
# --- When we leave this area of the code, the amount of space to
# --- allocate on each device in allocation units will be in r12.
# --- The user capacity it represents will be in r10/r11 as a long
# --- in allocation units. If there is sufficient space in the
# --- RAID being defined to complete the allocation, this will be
# --- indicated by a different jump location.
#
.cd600:
        ldob    ce_rtype(g0),g14        # Get RAID type
#
        cmpobe  mcrraid10,g14,.cd800    # Jif RAID 10 requested
        cmpobe  mcrraid5,g14,.cd700     # Jif RAID 5 requested
        cmpobe  mcrraid1,g14,.cd650     # Jif RAID 1 requested
        cmpobe  mcrraid0,g14,.cd610     # Jif RAID 0 requested
#
# --- Process standard device -----------------------------------------
#
# --- Since there was a check following the RDA build for at least
# --- one drive, there is no need to check for a drive count here.
# --- We know there is at least one.
#
        ldconst 1,g10                   # Only one drive allowed
        ld      ce_rda(g0),r3           # Get RDA pointer
        lda     (r3)[r10*4],g4          # Kludge the RDA pointer
        call    d$brddpsd               # Build the RAID
#
# --- Fill in RDD
#
        ldob    ce_depth(g0),r3         # Fetch depth
# -- d$brddpsd does BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_depth(g6)         # Set depth
        ldos    ce_sps(g0),r3           # Fetch sectors per stripe
        st      r3,rd_sps(g6)           # Set sectors per stripe
#
# --- Calculate and possibly back off size of RAID.
#
        mov     r11,r12                 # Per drive allocation
        mov     r11,r10                 # Set the user return sizes
        ldconst 0,r11                   # As a long
#
        ldl     ce_reqdevcap(g0),r4     # Get capacity requested
        cmpobne 0,r5,.cd900             # Jif too much cap for one device
        cmpobe  0,r4,.cd900             # Jif zero (no alloc to be done)
#
        cmpobg  r4,r12,.cd900           # Jif insufficient for full alloc
        mov     r4,r12                  # Amount to allocate for this RAID
        mov     r4,r10                  # Set the total user capacity
                                        # r11 already zero
        b       .cd1000                 # Go allocate the RAID
#
# --- Process RAID 0 device -------------------------------------------
#
.cd610:
#
        ldos    ce_rdadrives(g0),g10    # Get the number of drives
        ldos    ce_mindrives(g0),r3     # Get the min allowed
        cmpobl  g10,r3,.cd1100          # If there aren't enuf drives, done
#
        ld      ce_rda(g0),g4           # Get the RDA pointer
        call    d$brddpsd               # Build the RAID
#
# --- Determine the size of the RAID to create. This will either be
# --- enough to cover the remaining portion of the requested size, or
# --- it will be the maximum size allowed by the parameters (number of
# --- devices with capacity and the minimum largest capacity).
#
# --- For RAID 0, striping is all that is being done. Therefore the
# --- size of the user data and the required disk space is the same.
# --- To find the space required, simply multiply the number of
# --- devices in the RAID by the smallest, largest size available
# --- on the set of devices.
#
# --- Fill in RDD fields.
#
        ldob    ce_depth(g0),r3         # Get depth
# -- d$brddpsd does BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_depth(g6)         # Save it
        ldos    ce_sps(g0),r3           # Get sectors per stripe
        st      r3,rd_sps(g6)           # Save it
        mulo    r3,g10,r3               # Get sectors per unit
        st      r3,rd_spu(g6)           # Save it
#
# --- Determine the capacity that would be created with a full
# --- allocation given the number of drives.
#
        mov     r9,r12                  # Capacity per drive
        emul    r9,g10,r10              # RAID capacity
#
        ldl     ce_reqdevcap(g0),g4     # Get the required capacity
        or      g4,g5,r3                # Check for test only
        cmpobe  0,r3,.cd900             # Jif test
#
# --- Round the requested capacity, which is in allocation units, up
# --- to the number evenly divisible by the number of drives. First,
# --- divide the requested capacity by the number of drives. This is
# --- the rounded down units per device. Round this up if needed and
# --- if there is enough using this amount per device, multiply it by
# --- the number of drives to get the final capacity to be assigned.
# --- If there is not enough space, allocate what is available.
#
c       g3 = *(UINT64*)&g4 % g10;
c       *(UINT64*)&g4 = *(UINT64*)&g4 / g10;
        cmpo    0,g3                    # Check remainder
        sele    1,0,g3                  # Bump count
#
        cmpo    1,0                     # Clear carry
        addc    g3,g4,g4                # Add in rounding factor
        addc    0,g5,g5                 # As a long
#
        cmpobne 0,g5,.cd900             # Jif more than a drive can hold
        cmpobl  r12,g4,.cd900           # Jif not enough space
#
# --- Capacity available is sufficient. Complete the creation.
#
        emul    g4,g10,r10              # r10/r11 has RAID capacity
        mov     g4,r12                  # Set size per drive
        b       .cd1000                 # Done with virtual device
#
# --- Process RAID 1 device -------------------------------------------
#
.cd650:
#
        ldos    ce_rdadrives(g0),g10    # Get the number of drives
        ldos    ce_mindrives(g0),r3     # Get the min allowed
        cmpobl  g10,r3,.cd1100          # If there aren't enuf drives, done
#
        ld      ce_rda(g0),g4           # Get the RDA pointer
        call    d$brddpsd               # Build the RAID
#
# --- Determine the size of the RAID to create. This will either be
# --- enough to cover the remaining portion of the requested size, or
# --- it will be the maximum size allowed by the parameters (number of
# --- devices with capacity and the minimum largest capacity).
#
# --- For RAID 1, mirroring is all that is being done. Therefore the
# --- size of the required disk space is the depth times the user space.
# --- To find the space required, multiply the smallest, largest by the
# --- depth. The user space will be the smallest, largest and the space
# --- required will be the smallest, largest times the depth.
#
# --- Fill in RDD fields.
#
# -- d$brddpsd does BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    g10,rd_depth(g6)        # Save depth (number of drives in RAID)
        ldos    ce_sps(g0),r3           # Get sectors per stripe
        st      r3,rd_sps(g6)           # Save it
#
# --- Determine the capacity that would be created with a full
# --- allocation given the number of drives.
#
        mov     r9,r12                  # Capacity per drive
        mov     r9,r10                  # RAID drive
        ldconst 0,r11                   # As a long
#
        ldl     ce_reqdevcap(g0),g4     # Get the required capacity
        or      g4,g5,r3                # Check for test only
        cmpobe  0,r3,.cd900             # Jif test
#
# --- Determine if the capacity meets the required capacity. If it does
# --- then complete the building process. If not, accumulate this RAID
# --- and build another.
#
        cmpobne 0,g5,.cd900             # Jif more than a drive can hold
        cmpobl  r12,g4,.cd900           # Jif not enough space
#
# --- Capacity available is sufficient. Complete the creation.
#
        movl    g4,r10                  # r10/r11 has RAID capacity
        ldconst 0,g5
        mov     g4,r12                  # Set size per drive
        b       .cd1000                 # Done with virtual device
#
# --- Process RAID 5 device -------------------------------------------
#
.cd700:
        ldos    ce_rdadrives(g0),g10    # Get the number of drives
        ldob    ce_depth(g0),g9         # Get parity
#
        ldos    ce_mindrives(g0),r3     # Get min drives required
        cmpobl  g10,r3,.cd1100          # Jif not enough drives.
#
# --- Determine the size of the RAID to create. This will either be
# --- enough to cover the remaining portion of the requested size, or
# --- it will be the maximum size allowed by the parameters (number of
# --- devices with capacity, parity requirements and the minimum
# --- largest capacity).
#
# --- For RAID 5, we have a parity area added for every group of
# --- user data. The amount added is based upon the depth field.
# --- The user capacity per group is depth-1 allocation units and
# --- the total space required is depth allocation units. This
# --- group will be referred to as a RAID line.
#
# --- Also, the amount of space allocated will fit evenly into the
# --- number of drives in the RAID. For example, if 7 drives are
# --- used for a RAID 5 with parity of 3, then 3 RAID lines are
# --- required to evenly fill the 7 drives. This would yield 14
# --- user data allocation units, 7 parity allocation units for
# --- a total disk space of 21 allocation units (which would fit
# --- evenly on the 7 disks in 3 RAID lines.
#
        ld      ce_rda(g0),g4           # Set RDA pointer
        call    d$brddpsd               # Set up RAID
#
# --- Fill in RDD fields.
#
        ldob    ce_depth(g0),r3         # Get depth
# -- d$brddpsd does BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_depth(g6)         # Save it
        subo    1,r3,r4                 # Get depth minus one
        ldos    ce_sps(g0),r3           # Get sectors per stripe
        st      r3,rd_sps(g6)           # Save it
        mulo    r3,r4,r3                # Get sectors per unit
        st      r3,rd_spu(g6)           # Save it
        ldconst rduninit,r3             # Set status to uninitialized
        stob    r3,rd_status(g6)        # Save it
        ldob    rd_astatus(g6),r3       # Set additional status to uninitialized
        setbit  rdauninit,r3,r3
        stob    r3,rd_astatus(g6)       # Save it
        ld      K_ficb,r3               # Determine if mirroring information
        ld      fi_cserial(r3),r4       #  Get this controllers serial number
        ld      fi_mirrorpartner(r3),r3 #  Get the Mirror Partners serial number
        cmpobe  0,r3,.cd704             # Jif MP is zero - treat like mirroring
                                        #  to ourself
        cmpobne r3,r4,.cd705            # Jif mirroring to another controller
.cd704:
        st      r4,rd_notMirroringCSN(g6) # Not mirroring - save current CSN
                                        #    (if mirroring, set to zero which
                                        #     is already done in allocating
                                        #     the RDD)
#
# --- Determine the multiplier to designate a RAID line.
# --- This is an iterative process of multiplying the
# --- number of drives times a counter and dividing by the parity.
# --- If there is a remainder, then bump the counter until the
# --- remainder disappears. The counter is the multiplier.
#
.cd705:
        mov     1,r4                    # Initialize line counter
#
.cd710:
        mulo    r4,g10,r5               # Calculate multiplier * drive count
        remo    g9,r5,r6                # Check for evenly divisible
        cmpobe  0,r6,.cd720             # Jif so
#
        addo    1,r4,r4                 # Bump multiplier
        b       .cd710                  # Try again
#
# --- r4 has the multiplier in it. Multiply it by the depth minus one
# --- to determine the number of user allocation units per RAID line, or
# --- user AU. This will be used to determine the round-offs.
#
.cd720:
        mulo    r4,g10,r7               # r7 has real AU per RAID line
        subo    1,g9,r3                 # Determine user AU per RAID line
        mulo    r7,r3,r6
        divo    g9,r6,r6                # r6 = user AU per RAID line
#
        ldl     ce_reqdevcap(g0),g4     # Get the required capacity
        or      g4,g5,r3                # Check for test only
        cmpobe  0,r3,.cd750             # Jif test
#
c       g3 = *(UINT64*)&g4 % r6;
c       *(UINT64*)&g4 = *(UINT64*)&g4 / r6;
#
# --- After the round up, g4/g5 will contain the number of RAID lines
# --- required for the RAID.
#
        cmpo    0,g3                    # Check for zero remainder
        sele    1,0,g3                  # Round up
        cmpo    1,0                     # Clear carry
        addc    g3,g4,g4                # Round
        addc    0,g5,g5                 # As a long
        movl    g4,r10                  # Save a copy for later
#
# --- Multiply by the multiplier to get the number of allocation units
# --- per drive required.
#
        mov     r4,g3                   # Set up multiply
c       *(UINT64*)&g4 = *(UINT64*)&g4 * g3; # Get number of AU per drive
        cmpobne 0,g5,.cd750             # Jif over 2^32 required
        cmpobg  g4,r9,.cd750            # Jif not enough per drive
#
# --- There is enough. Allocate the right amount.
#
        mov     g4,r12                  # AU per drive
        mov     r6,g3                   # User capacity per RAID line
        movl    r10,g4                  # Number of RAID lines in RAID
c       *(UINT64*)&g4 = *(UINT64*)&g4 * g3; # UserCap = RAID lines * cap per line
        movl    g4,r10                  # User capacity
        b       .cd1000
#
# --- There is not enough per drive or it is a test. Allocate what
# --- is available.
#
.cd750:
        divo    r4,r9,r3                # r3 has RAID lines per drive
        mulo    r3,r4,r12               # r12 now has AUs per drive
        emul    r3,r6,r10               # r10/r11 has user AUs for drive
        b       .cd900
#
# --- Process RAID 10 device ------------------------------------------
#
.cd800:
        ldos    ce_rdadrives(g0),g10    # Get the number of drives
        ldob    ce_depth(g0),g9         # Get mirror depth
#
        ldos    ce_mindrives(g0),r3     # Get min drives required
        cmpobl  g10,r3,.cd1100          # Jif not enough drives.
#
# --- Determine the size of the RAID to create. This will either be
# --- enough to cover the remaining portion of the requested size, or
# --- it will be the maximum size allowed by the parameters (number of
# --- devices with capacity, mirror requirements and the minimum
# --- largest capacity).
#
# --- For RAID 10, we have a mirror area added for every group of
# --- user data. The amount added is based upon the depth field.
# --- The user capacity per group is 1 allocation units and the
# --- total space required is depth allocation units. Note that
# --- the depth includes the original data (i.e., depth = 2 has
# --- the original user data and one copy).
#
# --- Also, the amount of space allocated will fit evenly into the
# --- number of drives in the RAID. For example, if 5 drives are
# --- used for a RAID 10 with depth of 3, then 3 RAID lines are
# --- required to evenly fill the 5 drives. This would yield 5
# --- user data allocation units, 10 copy allocation units for
# --- a total disk space of 15 allocation units (which would fit
# --- evenly on the 5 disks.
#
# --- Determine the size in allocation units per drive required to
# --- complete the allocation for this device. If we have enough,
# --- truncate and exit.
#
        ld      ce_rda(g0),g4           # Set RDA pointer
        call    d$brddpsd               # Set up RAID
#
# --- Fill in RDD fields.
#
# -- d$brddpsd does BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    g9,rd_depth(g6)         # Set up mirror depth
        ldos    ce_sps(g0),r3           # Get sectors per stripe
        st      r3,rd_sps(g6)           # Save it
#
# --- Determine the multiplier to use for the allocation units
# --- that must be assigned per drive to evenly fill the drives.
# --- This determines the RAID line size. This is an iterative
# --- process of multiplying the number of drives times a counter
# --- and dividing by the depth. If there is a remainder, then
# --- bump the counter until the remainder disappears. The counter
# --- is the multiplier.
#
        mov     1,r4                    # Initialize line counter
#
.cd810:
        mulo    r4,g10,r5               # Calculate line * drive count
        remo    g9,r5,r6                # Check for evenly divisible
        cmpobe  0,r6,.cd820             # Jif so
#
        addo    1,r4,r4                 # Bump line counter
        b       .cd810                  # Try again
#
# --- r4 has the multiplier in it.
#
.cd820:
        mulo    r4,g10,r7               # r7 has AU per RAID line
        divo    g9,r7,r6                # r6 has user AU per RAID line
#
        ldl     ce_reqdevcap(g0),g4     # Get the required capacity
        or      g4,g5,r3                # Check for test only
        cmpobe  0,r3,.cd850             # Jif test
#
c       g3 = *(UINT64*)&g4 % r6;
c       *(UINT64*)&g4 = *(UINT64*)&g4 / r6;
#
# --- Round up the counts. Following the round up, g4/5 will contain
# --- the number of RAID lines required total which is also the number
# --- required per drive.
#
        cmpo    0,g3                    # Check for zero remainder
        sele    1,0,g3                  # Round up
        cmpo    1,0                     # Clear carry
        addc    g3,g4,g4                # Round
        addc    0,g5,g5                 # As a long
        movl    g4,r10                  # Save the number for later
#
# --- Multiply by the number of RAID lines by the multiplier to get
# --- the number of AU per drive required.
#
        mov     r4,g3                   # Set up multiply
c       *(UINT64*)&g4 = *(UINT64*)&g4 * g3; # Get the number of AU per drive
        cmpobne 0,g5,.cd850             # Jif over 2^32 required
        cmpobg  g4,r9,.cd850            # Jif not enough per drive
#
# --- There is enough. Allocate the right amount.
#
        mov     g4,r12                  # AU per drive
        mov     r6,g3                   # User capacity per RAID line
        movl    r10,g4                  # Number of RAID lines in RAID
c       *(UINT64*)&g4 = *(UINT64*)&g4 * g3; # UserCap = RAID lines * cap per line
        movl    g4,r10                  # User capacity
        b       .cd1000
#
# --- There is not enough per drive or it is a test. Allocate what
# --- is available.
#
.cd850:
        divo    r4,r9,r3                # r3 has RAID lines per drive
        mulo    r3,r4,r12               # r12 now has AUs per drive
        emul    r3,r6,r10               # r10/11 has user AUs for RAID
        b       .cd900
#
# --------------------------------------------------------------------
# --- We now have a RAID in g6. Enter it into the table of RAIDs and
# --- summarize the information into the CVE. r12 has the number of
# --- allocation units per drive being allocated. r10/r11 has the user
# --- RAID capacity. We must subtract this from the requested capacity
# --- if the request is non-zero. If it was zero, then we are only
# --- doing a test. In all cases, there is not enough capacity available
# --- to create the virtual device, so just summarize and continue.
# --------------------------------------------------------------------
#
.cd900:
        ldob    ce_numraids(g0),r3      # Get the number of RAIDs
        st      g6,ce_rdd(g0)[r3*4]     # Enter it into the table
        addo    1,r3,r3                 # Bump counter
        stob    r3,ce_numraids(g0)      # Save it
#
# --- Add the user capacity to the cumulative counter. Decrement the
# --- requested size counter. Don't worry about exceeding the size of
# --- the requested capacity since this entry point will not be used
# --- if the available capacity exceeds the remaining requested capacity.
#
        ldl     ce_reqdevcap(g0),r4     # Get the request left
        or      r4,r5,r3                # Check for zero
        cmpobe  0,r3,.cd910             # Just accumulate the count
#
        cmpo    0,0                     # Set carry
        subc    r10,r4,r4               # Subtract
        subc    r11,r5,r5               # As a long
        stl     r4,ce_reqdevcap(g0)     # Save it
#
# --- Accumulate the total capacity in the CEV. After doing this,
# --- determine the real capacity of the RAID in logical blocks and
# --- place it into the RDD.
#
.cd910:
        ldl     ce_devcap(g0),r4        # Get cumulative dev cap
        cmpo    1,0                     # Clear carry
        addc    r4,r10,r4               # Add in the user capacity
        addc    r5,r11,r5               # As a long
        stl     r4,ce_devcap(g0)        # Save it
#
        ldconst DSKSALLOC,g3            # Get user capacity
        movl    r10,g4
c       *(UINT64*)&g4 = *(UINT64*)&g4 * g3;
c       ((RDD *)g6)->devCap = *(UINT64 *)&g4; # Save RAID capacity in RDD
#
# --- Now find and allocate the space in the DAML and PSDs for the RAID
# --- just created. This is done to allow this assignment to be reflected
# --- in the next iteration of the loop.
#
        ld      rd_psd(g6),r6           # Save 1st PSD
        mov     r6,r4                   # Save 1st PSD
#
.cd920:
c       DA_Alloc((void *)g0, (void *)r6, r12); # CEV*, PSD*, number of allocation units
#
        ld      ps_npsd(r6),r6          # Link to next PSD
        cmpobne r4,r6,.cd920            # Jif more
#
# --- Check if there are RAID slots left to use. If there are, continue
# --- to build RAIDs to make this virtual device larger. If there are
# --- no slots left, then just terminate the RAID building.
#
        ldob    ce_numraids(g0),r3      # Check if more room
        ldos    ce_maxraids(g0),r4      # Get max allowed in this operation
        cmpobge r3,r4,.cd1100           # Jif no more room - summarize
#
        b       .cd500                  # Do another one
#
# --------------------------------------------------------------------
# --- The desired capacity was accomplished. This is either a test
# --- or it is an actual allocation. Place the RAID into the array
# --- and either fill in the return data and exit or create/expand
# --- the virtual device.
# --------------------------------------------------------------------
#
.cd1000:
        ldob    ce_numraids(g0),r3      # Get the number of RAIDs
        st      g6,ce_rdd(g0)[r3*4]     # Enter it into the table
        addo    1,r3,r3                 # Bump counter
        stob    r3,ce_numraids(g0)      # Save it
#
# --- Accumulate the capacity.
#
        ldl     ce_devcap(g0),r4        # Get cumulative dev cap
        cmpo    1,0                     # Clear carry
        addc    r4,r10,r4               # Add in the user capacity
        addc    r5,r11,r5               # As a long
        stl     r4,ce_devcap(g0)        # Save it
#
        ldconst DSKSALLOC,g3            # Get user capacity
        movl    r10,g4
c       *(UINT64*)&g4 = *(UINT64*)&g4 * g3;
c       ((RDD *)g6)->devCap = *(UINT64 *)&g4; # Save RAID capacity in RDD
#
# --- Now find and allocate the space in the DAML and PSDs for the RAID
# --- just created. This is done to allow this assignment to be reflected
# --- in the PDD when the DAML is released.
#
        mov     g0,r5                   # Save CEV
        ld      rd_psd(g6),r6           # Save 1st PSD
        mov     r6,r4                   # Save 1st PSD
#
.cd1010:
c       DA_Alloc((void *)r5, (void *)r6, r12); # CEV*, PSD*, number of allocation units
#
        ld      ps_npsd(r6),r6          # Link to next PSD
        cmpobne r4,r6,.cd1010           # Jif more
#
# --------------------------------------------------------------------
# --- Exit. No more RAID IDs available, no more RAID slots, or
# --- not enough drives meet capacity requirements.
# --------------------------------------------------------------------
#
.cd1100:
#
# --- Check if any RAID was created. If not, then there were not
# --- enough resources to do even a single RAID and we should report
# --- this fact.
#
        movl    0,g4                    # Prep zero size
        ld      ce_rptr(g0),r3          # Get return block pointer
!       stl     g4,mcr_adevcap(r3)      # Put it into the return block
#
        ldconst deinsres,g1             # Prep error code
        ldob    ce_numraids(g0),r3      # Any created?
        cmpobne 0,r3,.cd1110            # Jif RAIDs created
#
# --- If this was a test, set the return to OK. Else leave the error code.
#
        ldob    ce_op(g0),r3            # Get the opcode
        cmpobne mcrtest,r3,.cd1500      # If not test, leave error code
#       Get capacity and multiply by allocation units to get logical blocks.
c       *(UINT64*)&g4 = ((CEV*)g0)->devCap * DSKSALLOC;
# --- See if not enough space, and wanting to try increasing minPD.
c       if (*(UINT64*)&g4 < ((MRCREXP_REQ*)r15)->devcap) {
            ldob ce_flags(g0),r4        # get flags in case we need to run again.
            bbs ce_bits_7000min,r4,.cd1500 # if need to run again.
c       }
#
        ldconst deok,g1                 # Else ok
        b       .cd1500
#
# --- Determine the operation. If a test set the return data, blow
# --- away CEV and exit. If a create/expand go do the create/expand.
#
.cd1110:
        ldob    ce_op(g0),r3            # Get the opcode
        cmpobne mcrtest,r3,.cd1200      # Create/expand
#
# --- Fill in the return data.
#
#       Get capacity and multiply by allocation units to get logical blocks.
c       *(UINT64*)&g4 = ((CEV*)g0)->devCap * DSKSALLOC;
#
        ld      ce_rptr(g0),r3          # Get return block pointer
!       stl     g4,mcr_adevcap(r3)      # Put it into the return block
#
# --- See if not enough space, and wanting to try increasing minPD.
c       if (*(UINT64*)&g4 < ((MRCREXP_REQ*)r15)->devcap) {
            ldob ce_flags(g0),r4        # get flags in case we need to run again.
            bbs ce_bits_7000min,r4,.cd1500 # if need to run again.
c       }
        ldconst deok,g1                 # Return code - good
        b       .cd1500                 # Else, exit
#
# --------------------------------------------------------------------
# --- Create the virtual device.
# --- First check if the size created is as large as the requested size.
# --- If not, return an error after cleaning up (make it look like a test).
# --- Then create the VDD if it did not exist, the RIDs are linked into
# --- the VDD and the RDDs are linked into the VDX.
# --- Then the RAIDs themselves are initialized.
# --------------------------------------------------------------------
#
.cd1200:
#
        PushRegs(r3)
        call    DF_CancelDefrag         # Cancel the defrag that may be running
        PopRegsVoid(r3)
#
#       Get capacity and multiply by allocation units to get logical blocks.
c       *(UINT64*)&g4 = ((CEV*)g0)->devCap * DSKSALLOC;
        stl     g4,ce_devcap(g0)        # Save converted value for later
#
c       if (*(UINT64*)&g4 < ((MRCREXP_REQ*)r15)->devcap) {
            b   .cd1500                 # Jif less than requested capacity
c       }
        ldos    ce_vid(g0),r4           # Get the VID to use
        mov     g0,r6                   # Save g0
        ldob    ce_op(g0),r3            # Get the operation
        cmpobne mcrcreate,r3,.cd1210    # Jif no VDD needed (already allocated)
#
# A new vdisk is not associated to vlink, nor snappool, nor apool.
#
# Start to do it.
#
        call    D_allocvdd              # Allocate a VDD
        ldos    vx_ecnt(r14),r3         # Get entry count
        addo    1,r3,r3                 # Increment
        stos    r3,vx_ecnt(r14)         # Save it
        st      g0,vx_vdd(r14)[r4*4]    # Save VDD
#
        ldq     d_defaultvname,r8       # Get the default name
        stq     r8,vd_name(g0)          # Save it
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
.cd1210:
        ld      vx_vdd(r14)[r4*4],g2    # Get VDD
        mov     r6,g0                   # Restore CEV
#
# --- Check for expand too big. If over 2TB, error out.
#
# Add desired expansion with current vdisk capacity (and deferred raid list).
c       *(UINT64 *)&g4 = ((CEV *)g0)->devCap + ((VDD *)g2)->devCap;
c       r3 = (UINT32)((VDD *)g2)->pDRDD; # Deferred RAID list for 2TB accum loop
#
c       while (r3 != 0) {
c         *(UINT64 *)&g4 += ((RDD *)r3)->devCap; # Add in deferred raid sizes.
c         r3 = (UINT32)((RDD *)r3)->pNRDD; # Get the next RDD in the deferred list
c       }
#
# Snappool must not go over 2tb.
c       if (g5 != 0 && BIT_TEST(((VDD*)g2)->attr, VD_BSNAPPOOL)) {
c         if (gss_version[((VDD *)g3)->vid & 1] == SS_NV_VERSION) {
c           g1 = de2tblimit;            # Prep the error code (>2tb)
            b     .cd1500               # Error out if too large
c         } else if (*(UINT64*)&g4 >= (64ULL*1024*1024*1024*1024/512)) { # If greater than 64 terabytes
c           g1 = de64tblimit;           # Prep the error code (>64tb)
            b     .cd1500               # Error out if too large
c         }
c       }
#
        ld      vd_vlinks(g2),r3        # r3 = assoc. VLAR address
        cmpobe  0,r3,.cd1215            # Jif VDisk lock not applied
#
# --- VDisk lock applied. Attempt to notify the lock owner
# --- of the size change and only allow it with the lock
# --- owner's approval. g4 has the new capacity of the device.
#
        mov     g2,g8                   # Input parm (VDD)
        call    DLM$app_vdsize          # Get approval for VDisk size change
        mov     g0,r3
        mov     r6,g0                   # Restore CEV
#
        ldconst dedevused,g1            # Prep possible error code
# NOTE: only expand may do this jump, so vdd does not need to be deleted.
        cmpobne 0,r3,.cd1500            # Jif approval denied
#
.cd1215:
#
# --- VDD is now set up. Place the RAIDs into the RDD.
#
        ldob    ce_numraids(g0),r4      # Stopping point
        ldconst 0,r5                    # Pointer into RDD array
#
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        ldos    rx_ecnt(r13),r6         # Bump the RAID count
        addo    r4,r6,r6
        stos    r6,rx_ecnt(r13)         # Save it
#
.cd1220:
        ld      ce_rdd(g0)[r5*4],r6     # Get RDD pointer
        ldos    rd_rid(r6),r7           # Get RID
        st      r6,rx_rdd(r13)[r7*4]    # Save in RDX
#
# --- If this is a RAID-5, start an initialization on it.
#
        ldob    rd_type(r6),r3
        cmpobne rdraid5,r3,.cd1225      # Jif not RAID-5
#
# --- Load up the VID temporarily since the ownership check done in
# --- the init RAID requires it to be set.
#
        ldos    ce_vid(g0),r3           # Get VID
        stos    r3,rd_vid(r6)           # Set it in the RDD
#
        mov     g0,r3
        mov     r6,g0                   # Load RDD
        call    D_que_rinit             # Queue the raid init request
        mov     r3,g0
#
# --- Loop until all RDDs are processed.
#
.cd1225:
        addo    1,r5,r5                 # Bump index
        subo    1,r4,r4                 # Decrement count
        cmpobne 0,r4,.cd1220            # Jif not done
#
# --- Now update the VDD created/expanded.
#
        ldos    ce_vid(g0),g14          # Get VID
#
        ldob    ce_op(g0),r3            # Create or expand
        cmpobe  mcrexpand,r3,.cd1230    # Jif expand
#
# --- Create
#
        ldos    ce_vid(g0),g8           # Get VID
        ld      vx_vdd(r14)[g8*4],g8    # VDD
#
        stos    g14,vd_vid(g8)          # Set up virtual ID
        ldob    ce_numraids(g0),r3      # Set up RAID count
        stob    r3,vd_raidcnt(g8)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
        ld      vd_vtype,r3
        cmpobe  visethigh,r3,.cd1225_1
        cmpobe  visetmed,r3,.cd1225_2
        cmpobe  visetlow,r3,.cd1225_3
.cd1225_1:
        ldconst visethigh,r3
        b       .cd1226
#
.cd1225_2:
        ldconst visetmed,r3
        b       .cd1226
#
.cd1225_3:
        ldconst visetlow,r3
.cd1226:
        stob    r3,vd_strategy(g8)      # Update priority in VDD
#
c       if ((((CEV*)g0)->flags & CE_FLAGS_SETSIZE) != 0) {
          ldl     mcr_devcap(r15),g4    # Requested capacity
c       } else {
          ldl     ce_devcap(g0),g4      # Get capacity
c       }
        stl     g4,vd_devcap(g8)
        ld      ce_rptr(g0),r3          # Get return parm address
!       stl     g4,mcr_adevcap(r3)      # Save it
#
        ldconst vdnormal,r3             # Assume normal VDisk
#
#
        stos    r3,vd_attr(g8)
#
        ldconst vdop,r3                 # Set to operable
        stob    r3,vd_status(g8)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        PushRegs(r4)
c       GR_UpdateVdiskOpState((VDD *)g8, 0, (UINT8)r3);
        PopRegsVoid(r4)
#
        lda     vd_rdd(g8),r4           # Get addr to place first RDD pointer
        b       .cd1300                 # Continue common processing
#
# --- Process expand virtual device -----------------------------------
#
.cd1230:
        ld      vx_vdd(r14)[g14*4],g8   # Get VDD. Existence already checked.
        ldos    vd_attr(g8),r5          # r5 = vdisk attributes
        bbc     vdbspool,r5,.cd1235     # jif not snappool

        ldos    vd_vid(g8),g2
        call    D$spool_expand          # clear Oger bits before updating dev_cap
.cd1235:
#
        ldob    ce_rtype(g0),r3         # Get RAID type. If R5, defer the RDDs
        cmpobne rdraid5,r3,.cd1280      # Jif not R5
#
        ldob    ce_numraids(g0),r4      # Get additional RAID count
        ldob    vd_draidcnt(g8),r3      # Get the old deferred count
        addo    r3,r4,r4                # Add in the new ones added
        stob    r4,vd_draidcnt(g8)      # Save the deferred count
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
        ld      ce_rptr(g0),r3          # Get return parm address
        ldl     ce_devcap(g0),g4        # Get capacity
        stl     g4,mcr_adevcap(r3)      # Save it
#
# --- Set the address to place the RAIDs into in the r4 register. This is
# --- where the append will take place. We have to get to the end of the
# --- deferred list in case this is an expand on an expanding vdisk which
# --- has not completed initialization.
#
        lda     vd_drdd(g8),r4          # r4 has addr of first ptr to write
        ld      vd_drdd(g8),r3          # r3 has the pointer
#
.cd1250:
        cmpobe  0,r3,.cd1260            # Jif done - r4 has address for RDD ptr
        lda     rd_nvrdd(r3),r4         # Grab the address of the next pointer
        ld      rd_nvrdd(r3),r3         # Move the pointer
        b       .cd1250
#
.cd1260:
        ld      ce_rptr(g0),r5          # Get return block pointer
!       stos    g14,mcr_cvid(r5)        # Save the VID for the return data
#
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        ld      ce_rdd(g0),r5           # Get first RDD
        stos    g14,rd_vid(r5)          # Save virtual ID in RDD
        st      r5,(r4)                 # Place the first RDD in the list
#
        ldconst 0,r4                    # Index into RDD list
        ldob    ce_numraids(g0),r3      # Get RDD count
#
.cd1270:
        subo    1,r3,r3                 # Decrement
        cmpibe  0,r3,.cd1350            # Jif no more RAIDs
#
        addo    1,r4,r4                 # Bump RDD index
        ld      ce_rdd(g0)[r4*4],r6     # Get RDD
        st      r6,rd_nvrdd(r5)         # Link
        mov     r6,r5                   # Move pointer
        stos    g14,rd_vid(r5)          # Save virtual ID in RDD
        b       .cd1270                 # Go again
#
# --- Non-RAID-5 expand operation.
#
.cd1280:
        ld      vx_vdd(r14)[g14*4],g8   # Get VDD. Existence already checked.
#
        ldl     ce_devcap(g0),g4        # Get capacity
        ldl     vd_devcap(g8),r8        # Get virtual device capacity

        cmpo    1,0                     # Clear carry
        addc    g4,r8,r8                # Add the long
        addc    g5,r9,r9                # Add the long
        stl     r8,vd_devcap(g8)        # Save device capacity
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        PushRegs(r5)
        movl    r8,g0                   # g0-1 new dev cap
        ldos    vd_vid(g8),g2           # g2 = vid
        call    apool_expand            # Don't check ret val for now
                                        # since we already asked if we could expand
        PopRegsVoid(r5)
#
        ldob    ce_numraids(g0),r4      # Get additional RAID count
        ldob    vd_raidcnt(g8),r5       # Get current count
        addo    r4,r5,r4                # Add together
        stob    r4,vd_raidcnt(g8)       # Save the increased count
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
        ld      ce_rptr(g0),r3          # Get return parm address
!       stl     g4,mcr_adevcap(r3)      # Save it
#
# --- Find last RDD in the VDD and then append all of the RAIDs
# --- in the RDD list to the virtual device.
#
        ld      vd_rdd(g8),r3           # Get first RDD
        lda     rd_nvrdd(r3),r4         # r4 has addr of first ptr to write
        ld      rd_nvrdd(r3),r3         # r3 has next pointer
        cmpobe  0,r3,.cd1300            # If the pointer NULL - done
#
.cd1290:
        lda     rd_nvrdd(r3),r4         # Get address of pointer
        ld      rd_nvrdd(r3),r3         # Move the pointers
        cmpobne 0,r3,.cd1290            # Non-null, traverse
#
# --- Common completion code. Store the remaining RDDs into the list.
#
.cd1300:
        ld      ce_rptr(g0),r5          # Get return block pointer
!       stos    g14,mcr_cvid(r5)        # Save the VID for the return data
#
        ld      ce_rdd(g0),r5           # Get first RDD
        stos    g14,rd_vid(r5)          # Save virtual ID in RDD
        st      r5,(r4)                 # Place the first RDD in the list
#
        ldconst 0,r4                    # Index into RDD list
        ldob    ce_numraids(g0),r3      # Get RDD count
#
.cd1310:
        subo    1,r3,r3                 # Decrement
        cmpibe  0,r3,.cd1320            # Jif no more RAIDs
#
        addo    1,r4,r4                 # Bump RDD index
        ld      ce_rdd(g0)[r4*4],r6     # Get RDD
        st      r6,rd_nvrdd(r5)         # Link
        mov     r6,r5                   # Move pointer
        stos    g14,rd_vid(r5)          # Save virtual ID in RDD
        b       .cd1310                 # Go again
#
# --- Now that all of the devices are recorded in the structures, set the
# --- status of the virtual device and write out one AU to each physical
# --- drive in each RAID.
#
.cd1320:
        call    RB_setvirtstat           # Set the status of all virtual devices
#
        ldob    ce_numraids(g0),r4      # Stopping point
        ldconst 0,r5                    # Pointer into RDD array
        mov     g0,r3                   # Save CEV
#
.cd1330:
        ld      ce_rdd(g0)[r5*4],g2     # Get RDD pointer
#
# --- Write zeros to first segment of each raid device
#
        call    d$clearfirstAU          # Clear the first one meg of each drive
#
# --- Loop until all RDDs are processed.
#
        mov     r3,g0                   # Restore CEV
        addo    1,r5,r5                 # Bump index
        subo    1,r4,r4                 # Decrement count
        cmpobne 0,r4,.cd1330            # Jif not done
#
# --- Update NVRAM
#
.cd1350:
        movq    0,r4                    # NULL RDD ptrs to prevent deallocation
        stq     r4,ce_rdd(g0)           # Get first set
        stq     r4,ce_rdd+16(g0)        # Second set
#
# --- If this is a 7000 system, enable vdisk cache by default
#
.ifndef MODEL_3000
.ifndef MODEL_7400
c       if (((CEV*)g0)->op == MCR_OP_CREATE) {
c           BIT_SET(((VDD*)g8)->attr, VD_BCACHEEN);
c           BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
c       }
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- Update the remote cache
#
        mov     g0,r3
        mov     g14,g0
        ldconst FALSE,g1                # Addition of a new VDD (not deleting)
        call    D$updrmtcachesingle     # Update single
        call    D$signalvdiskupdate     # Update front end
#
# --- Check for size out of range on hot spares.
#
        ldconst 0,g0                    # No PDD input
        PushRegs(r4)                    # Save register contents
        call    RB_CheckHSCapacity
        PopRegsVoid(r4)                 # Restore registers
#
!       ldob    mcr_op(r15),r5          # Get the operation
        cmpobl  mcrtest,r5,.cd1401      # Jif invalid operation
        PushRegs(r4)                    # Save register contents
c       GR_UpdateVddGeoInfo((VDD*)g8);
        cmpobe  mcrcreate,r5,.cd1400
c       GR_UpdateVddPartners((VDD*)g8);
.cd1400:
        PopRegsVoid(r4)                 # Restore registers
.cd1401:
        cmpobne mcrcreate,r5,.cd1402
c       CM_VdiskEnableInstantMirror((VDD*)g8);

# --- Get and store the VDisk create time in seconds
c       r5 = GetSysTime();              # Get seconds since epoch
        st r5, vd_createTime(g8)        # Store the time in seconds
.cd1402:
#
# --- If the vdisk being expanded is a snap pool, owned by this controller,
#     (that means it is done only on master, slave's spool % usage info is not
#     available on master), Do an update of the percent full.
#
        ldob     ce_op(r3),r5           # Create or expand
        cmpobne  mcrexpand,r5,.cd1450   # Jif not expand
        ldos    vd_attr(g8),r5          # r5 = vdisk attributes
        bbc     vdbspool,r5,.cd1450     # jif not snappool
        ldos    vd_vid(g8),g1
        call    D$update_spool_percent_used # update the percent usage on spool
.cd1450:
        ldos    vd_vid(g8),g1           # Set up virtual ID
        ldob    vd_strategy(g8),g2
        call    D$setvpri
#
        mov     deok,g1                 # Return OK status
        mov     r3,g0                   # Restore CEV
#
# --- Exit. Assumes return data all set and g1 contains the
# --- return code.
#
.cd1500:
# --- If g1 is insufficient resources, then need to see if should do this again
#     with minPD increased, if the ce_bits_7000min bit in flags is set.
c       if (g1 != deinsres) {
            b   .cd1520
c       }
!       ldob    ce_flags(g0),r4         # get flags in case we need to run again.
        bbc     ce_bits_7000min,r4,.cd1520 # if no need to run again.
#-- .ifdef MODEL_7000
#-- c       r6 = 16                         # Management decision to limit to 16 luns on 7000.
#-- .else   # MODEL_7000
c       r6 = 255                        # 8 bit constant, limits to 255.
#-- .endif  # MODEL_7000
c       r5 = ((MRCREXP_REQ*)r15)->minPD + 1;
c       if (r5 <= r6 && r5 <= ((MRCREXP_REQ*)r15)->drives) {
#           Save largest vdiskprepare (or create/expand -- which is ignored) size.
!           ld      ce_rptr(g0),r3          # Get return parm address
c           *(UINT64*)(sp - 12) = MAX(((MRCREXP_RSP *)r3)->devcap, *(UINT64*)(sp - 12));
            call    d$rcev              # Blow away the CEV
!           stob    r5,mcr_minpd(r15)   # Update the min PD value
c           g0 = *(ulong*)(sp - 4);     # Restore pointer to MRP
            b       .cd05               # Try with min PD increased
c       }
!       ldob    ce_op(g0),r3            # Get the opcode
        cmpobne mcrtest,r3,.cd1520      # Create/expand, not prepare/test
        ld      ce_rptr(g0),r3          # Get return parm address
#       Save largest vdiskprepare  size.
c       ((MRCREXP_RSP *)r3)->devcap = MAX(((MRCREXP_RSP *)r3)->devcap, *(UINT64*)(sp - 12));
        ldconst deok,g1                 # Return code - good, size already set.

.cd1520:
        call    d$rcev                  # Blow away the CEV
        ldconst mcrrsiz,g2              # Set return packet size
#
# --- ret instruction does not need sp correctly set, it is correct after return.
        ret
#
#**********************************************************************
#
#  NAME: d$clearfirstAU
#
#  PURPOSE:
#       To provide a means of setting the first AU of each PSD in a
#       RAID to zeros. This is done at RAID creation time to make
#       sure there is no residual data from an old RAID in the new
#       RAID which could confuse a server.
#
#  DESCRIPTION:
#       The input RAID is parsed and each PSD has the first AU written
#       to zeros.
#
#       RAID 5 devices are skipped since all R5 RAIDs are initialized
#       automatically and there is no need to double clear the first AU.
#
#  CALLING SEQUENCE:
#       fork    d$clearfirstAU
#
#  INPUT:
#       g2 = RDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$clearfirstAU:
        ldob    rd_type(g2),r3          # Get RAID type
        cmpobe  rdraid5,r3,.cfa100      # Exit if R5 since all R5 are init'ed
                                        # outside of this function
#
        ld      rd_psd(g2),g9           # Get 1st PSD
        lda     P_pddindx,r8            # Get the base of the PDX
        mov     g9,r7                   # Save 1st PSD
#
.cfa10:
        ldos    ps_pid(g9),g3           # Get corresponding PID
        ld      px_pdd(r8)[g3*4],g3     # Get the PDD
#
# If should do 16 byte SCSI command or 10 byte SCSI command.
c   if ((((PSD*)&g9)->sda & ~0xffffffffULL) != 0ULL) {    # 16 byte command below
    .ifdef DISABLE_WRITE_SAME
        lda     O_t_wrseg_16,g0         # Pass clear segment template
    .else   # DISABLE_WRITE_SAME
        lda     O_t_wrsame_16,g0        # Pass write same template
    .endif # DISABLE_WRITE_SAME
        call    O$genreq                # Generate request
c       ((PRP*)g2)->sda = ((PSD*)g9)->sda; # Set up starting disk address
!       ldl     ps_sda(g9),r4           # Gets starting disk address
        mov     r4,r3                   # Save in r3 due to word and byte swapping.
        bswap   r5,r4                   # byte swap r5 into r4
        bswap   r3,r5                   # byte swap r4 (in r3) into r5.
        stl     r4,pr_cmd+2(g2)         # SDA of request.
c       ((PRP*)g2)->eda = ((PSD*)g9)->sda + DSKSALLOC; # Store ending disk address
        ldconst DSKSALLOC,r9            # get the length
        bswap   r9,r10
        st      r10,pr_cmd+10(g2)       # transfer length
c   } else {                            # 10 byte command below
    .ifdef DISABLE_WRITE_SAME
        lda     O_t_wrseg,g0            # Pass clear segment template
    .else   # DISABLE_WRITE_SAME
        lda     O_t_wrsame,g0           # Pass write same template
    .endif # DISABLE_WRITE_SAME
        call    O$genreq                # Generate request
!       ld      ps_sda(g9),r9           # Gets starting disk address
c       ((PRP*)g2)->sda = ((PSD*)g9)->sda; # Set up starting disk address
        bswap   r9,r10
        st      r10,pr_cmd+2(g2)
c       ((PRP*)g2)->eda = ((PSD*)g9)->sda + DSKSALLOC; # Store ending disk address
        ldconst DSKSALLOC<<16,r9        # get the length
        bswap   r9,r10
        stos    r10,pr_cmd+7(g2)
c   }
        call    O$quereq                # Queue I/O request
        call    O$relreq                # Release previous request
#
# --- Advance to next device
#
        ld      ps_npsd(g9),g9          # Link to next PSD
        cmpobne g9,r7,.cfa10            # Jif more
#
.cfa100:
        ret
#
#**********************************************************************
#
#  NAME: D_convpdd2psd
#
#  PURPOSE:
#       To provide a common means of converting a PDD/PID to any PSD.
#
#  DESCRIPTION:
#       All RDDs are searched to determine if there is a corresponding
#       match to the designated PDD. If found, the 1st PSD is returned.
#       Otherwise, a null PSD is returned.
#
#  CALLING SEQUENCE:
#       call    D_convpdd2psd
#
#  INPUT:
#       g4 = PDD
#
#  OUTPUT:
#       g0 = PSD if non-zero
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# PSD* DC_ConvPDD2PSD(PDD* pPDD);
        .globl  DC_ConvPDD2PSD
DC_ConvPDD2PSD:
        mov     g0,g4                   # PDD
        call    D_convpdd2psd
        ret
#
D_convpdd2psd:
        ldos    pd_pid(g4),r15          # Get PID of PDD
        ldconst MAXRAIDS-1,r6           # Prepare search
#
# --- Check next RAID device
#
.c10:
        ld      R_rddindx[r6*4],r5      # Get next RDD
        cmpobe  0,r5,.c40               # Jif undefined
#
# --- Skip if this is a vlink or a snapshot
#
        ldob    rd_type(r5),r3          # Get type
        cmpobe  rdlinkdev,r3,.c40       # Jif linked device type
        cmpobe  rdslinkdev,r3,.c40      # Jif snapshot device
#
        ld      rd_psd(r5),g0           # Get 1st PSD
        mov     g0,r7                   # Save for compare on loop termination
#
# --- Check next PSD
#
.c20:
        ldos    ps_pid(g0),r3           # Get corresponding PDD
        cmpobe  r3,r15,.c100            # Jif match
#
# --- Advance to next PSD
#
        ld      ps_npsd(g0),g0          # Get next PSD
        cmpobne g0,r7,.c20              # Jif more
#
# --- Advance to next RAID device
#
.c40:
        subo    1,r6,r6                 # Advance to next device
        cmpible 0,r6,.c10               # Jif more
#
        mov     0,g0                    # Return null PSD
#
# --- Exit
#
.c100:
        ret
#
#**********************************************************************
#
#  NAME: D_deleteraid
#
#  PURPOSE:
#       To provide a means of deleting a RAID device in response to a
#       request issued by the CCB to delete a virtual device.
#
#  DESCRIPTION:
#       This is an internal function that will delete the RAID device,
#       delete the PSDs associated with the device and update the
#       drives to reflect the released data areas.
#
#  INPUT:
#       g1 = RDD to be deleted
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
# void DC_DeleteRAID(RDD *pRDD);
        .globl  DC_DeleteRAID
DC_DeleteRAID:
        mov     g0,g1                   # RDD
        call    D_deleteraid
        ret
#
D_deleteraid:
        mov     g0,r8                   # save g0
        cmpobe  0,g1,.ddr900            # Jif NULL, doesn't exist so can't del
#
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
#
# --- Remove the RDD and PSD's after cancelling any rebuilds. The release of
# --- the PSDs will release the space on the disks.
#
        ldos    rd_rid(g1),r6           # Save the RID
        mov     r6,g0                   # Set input parm
        call    RB$cancel_rebld         # Cancel any potential rebuilds
#
        PushRegs(r3)
        call    DF_CancelDefrag         # Cancel the defrag that may be running
        PopRegsVoid(r3)
#
        ld      rd_vlop(g1),r4          # r4 = assoc. VLOP address
        cmpobe  0,r4,.ddr400            # Jif no VLOP assoc. with RDD
        mov     g5,r5                   # save g5
        mov     r4,g5                   # g5 = VLOP to abort
        ld      vlop_ehand(r4),r4       # r4 = VLOP event handler table
        ld      vlop_eh_abort(r4),r4    # r4 = VLOP abort event handler
                                        #      routine
        callx   (r4)                    # call event handler routine
        mov     r5,g5                   # restore g5
#
.ddr400:
        ldos    rd_vid(g1),r3           # Get the VID
        ld      V_vddindx[r3*4],r15     # Get the VDD
        cmpobe  0,r15,.ddr500           # Jif NULL
#
# --- De-link the RDD from the VDD. This is done to allow other functions
# --- such as setvirt status to still run against a vdisk being deleted.
#
        ld      vd_rdd(r15),r14         # Get the RDD pointers
        lda     vd_rdd(r15),r13         # Get the address of the pointer
#
.ddr410:
        cmpobe  0,r14,.ddr425           # End of the list?
        cmpobne r14,g1,.ddr420          # Jif not the RDD we are deleting
        ld      rd_nvrdd(r14),r3        # Get the next pointer
        st      r3,(r13)                # Save the pointer
#
        ldob    vd_raidcnt(r15),r3      # Decrement count
        subo    1,r3,r3
        stob    r3,vd_raidcnt(r15)      # Save it count
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        b       .ddr500                 # Done
#
.ddr420:
        lda     rd_nvrdd(r14),r13       # Get the address of the next pointer
        ld      rd_nvrdd(r14),r14       # Move to the next pointer
        b       .ddr410                 # Check the next one
#
# --- Now do the deferred list.
#
.ddr425:
        ld      vd_drdd(r15),r14        # Get the deferred RDD pointers
        lda     vd_drdd(r15),r13        # Get the address of the deferred pointer
#
.ddr430:
        cmpobe  0,r14,.ddr500           # End of the list?
        cmpobne r14,g1,.ddr440          # Jif not the RDD we are deleting
        ld      rd_nvrdd(r14),r3        # Get the next pointer
        st      r3,(r13)                # Save the pointer
#
        ldob    vd_draidcnt(r15),r3     # Decrement count
        subo    1,r3,r3
        stob    r3,vd_draidcnt(r15)     # Save it count
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        b       .ddr500                 # Done
#
.ddr440:
        lda     rd_nvrdd(r14),r13       # Get the address of the next pointer
        ld      rd_nvrdd(r14),r14       # Move to the next pointer
        b       .ddr430                 # Check the next one
#
.ddr500:
        mov     g1,g0                   # This is the RDD we want released
        call    D_rrddpsd               # Release the rdd/psd collection
#
# --- Mark the RDX as invalid and decrement the count of RAIDs
#
        ldconst 0,r4
        lda     R_rddindx,r5            # Get RDX
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r4,rx_rdd(r5)[r6*4]     # Mark the RDD as invalid
        ldos    rx_ecnt(r5),r3          # Get the count
        subo    1,r3,r3                 # Decrement it
        stos    r3,rx_ecnt(r5)          # Set the count
#
        PushRegs(r4)                    # Save register contents
        call    DEF_UMiscStat           # Update PDD misc status
        PopRegsVoid(r4)                 # Restore registers
#
# --- Check all the hot spares to see if they lost their last RAID. If
# --- so, then we clear the hsdname field.
#
        ldconst MAXDRIVES-1,r13         # Get the max PID
#
.ddr600:
        ld      P_pddindx[r13*4],r12    # Get the PDD
        cmpobe  0,r12,.ddr650           # Jif NULL
#
        ldob    pd_class(r12),r3        # Get the class
        cmpobne pdhotlab,r3,.ddr650     # Jif not a hot spare drive
#
        ld      pd_hsdname(r12),r3      # Check if assigned
        cmpobe  0,r3,.ddr650            # Not set, continue
#
        ldos    pd_pid(r12),g0          # Get the PID for calc space
        ldconst TRUE,g1                 # Force the build of DAML
        call    D$calcspaceshell        # Do the DAML build
#
        ld      pd_daml(r12),r3         # Get the DAML
        cmpobe  0,r3,.ddr650            # Jif NULL
#
        ldos    da_count(r3),r3         # Get the count
        cmpobne 1,r3,.ddr650            # Jif RAIDs still on the device
#
        ldconst 0,r3                    # Zero out the hs dname
        st      r3,pd_hsdname(r12)      # Clear it
#
.ddr650:
        subo    1,r13,r13               # Decrement the index
        cmpible 0,r13,.ddr600           # Continue if more to do
#
# --- Exit
#
.ddr900:
        mov     r8,g0                   # restore g0
        ret
#
#**********************************************************************
#
#  NAME: d$deletevirt
#
#  PURPOSE:
#       To provide a means of processing the delete virtual device
#       request issued by the CCB.
#
#  CALLING SEQUENCE:
#       call    d$deletevirt
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$deletevirt:
        ld      mr_ptr(g0),g0           # Get parm block pointer
#
# --- Validate parameters
#
        ldos    mdv_vid(g0),r14         # Get virtual ID
        ldconst deinvvirtid,g1          # Prep possible error code
#
        ldconst MAXVIRTUALS,r3          # Check for over max value
        cmpobge r14,r3,.dv200           # Jif out of range
#
        ld      V_vddindx[r14*4],r15    # Get the VDD pointer
        cmpobe  0,r15,.dv200            # Jif invalid

        ldconst dedevused,g1            # g1 = possible error code
        ldos    vd_attr(r15),r5         # r5 = vdisk attributes
        bbs     vdbasync,r5,.dv200      # skip out, if async
        bbs     vdbspool,r5,.dv200      # don't delete snappool
#
# --- Check if any snapshots associated with VDisk.
#     Do not let a user delete a VDisk that is the source of a snapshot.
#
        ld      vd_outssms(r15),r5      # r5 = first SSMS on list
        ldconst deinvvid,g1             # Prep bad status
#        cmpobne.f 0,r5,.dv200           # Jif VDisk is a source for an active
                                        # snapshot
        ldconst MAXRAIDS,r3
        ldconst 0,r4
.dv00_1:
        cmpobe  r3,r4,.dv00_2           # Jif no more RAIDs to check
        ld      R_rddindx[r4*4],r5      # Get the RDD pointer
        addo    1,r4,r4
        cmpobe  0,r5,.dv00_1            # Jif NULL

        ldob    rd_type(r5),r6
        cmpobne rdslinkdev,r6,.dv00_1

        ld      rd_sps(r5),r6           # Found SS RAID,check source VID, (stashed in sps)
        cmpobe  r6,r14,.dv200           # Skip if this is the source of a snapshot
        b       .dv00_1
#
.dv00_2:

        ldconst dedevused,g1            # g1 = possible error code
        ld      vd_rdd(r15),r5          # check raid type to be sure this is a snapshot
        cmpobe  0,r5,.dv00              # Cannot be snapshot, no raid present
        ldob    rd_type(r5),r8
        cmpobne rdslinkdev,r8,.dv00     # not a snapshot...following might be redundant

        ld      vd_incssms(r15),r5      # r5 = SSMS (this virt is a snapshot)
        cmpobe  0,r5,.dv00              # Jif not a snapshot
#
# ---- PROCESS snapshot specific delete logic
#
        ldos    vd_attr(r15),r6         # r5 = vdisk attributes
        bbs     vdbscd,r6,.dv200        # skip out, if a SS and the source of a copy
#
# --- This device is a snapshot. Release all of the snapshot related data structures.
#
        mov     g0,r8                   # Save g0
        mov     g8,r6                   # Save g8
        mov     r15,g8                  # g8 = vid of ss
        mov     r5,g0                   # g0 = SSMS
        call    d$kill_ss
        mov     r8,g0
        mov     r6,g8
#
# --- Delete the PSD and the RDD. The caller will take care of the VDD.
#

        ld      vd_rdd(r15),r8          # Get the RDD
        ld      rd_psd(r8),r6
c       s_Free(r6, psdsiz, __FILE__, __LINE__); # Free PSD
#
        ldos    rd_rid(r8),r3
        ldconst 0,r6                    # Get a zero
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        st      r6,R_rddindx[r3*4]      # Mark the RDD as invalid
        st      r6,V_vddindx[r14*4]     # Unlink VDD from VDX
        ldos    rx_ecnt+R_rddindx,r3    # Get the count
        subo    1,r3,r3                 # Decrement it
        stos    r3,rx_ecnt+R_rddindx    # Set the count
#
c       s_Free(r8, rddsiz+4, __FILE__, __LINE__); # Free One PSD for a RDD in an Slink
        b       .dv100                  # Go free the Vdd
# --- DONE with snapshot specific deletion 'prep' logic

#
# --- Check if VDisk/VLink lock applied
#
.dv00:
        ld      vd_vlinks(r15),r5       # r5 = assoc. VLAR if VLink defined
        cmpobne 0,r5,.dv200             # Jif VLink defined to this VDisk
#
# --- Check for virtual linked device.
#
        ld      vd_rdd(r15),r3          # r3 = assoc. RDD address
        cmpobe  0,r3,.dv10              # No RDD, cannot be a vlink
        ldob    rd_type(r3),r3          # r3 = RAID type code
        cmpobne rdlinkdev,r3,.dv10      # Jif not linked RAID type
#
# --- Confirmed VLink.
#
#       Check for any copy operations associated with the VLink being
#       terminated and check if they are user suspended. Do not
#       allow a VLink to be deleted if an associated copy operation
#       is still active. If all are suspended, check if the copy device
#       type is a remote copy device. Do not allow remote copy devices
#       to be deleted. If not a remote copy device, adjust the copy
#       registration values to reflect that one of the copy devices
#       is being terminated and set up to communicate the change to
#       all related nodes.
#
        ldconst dedevused,g1            # g1 = error code to return if a copy
                                        #  operation is found to still be
                                        #  active
        ldconst FALSE,r3                # r3 = config. change occurred flag
        ld      vd_scdhead(r15),r4      # r4 = first SCD assoc. with VDD
        cmpobe.t 0,r4,.dv02             # Jif no SCDs assoc. with VDD
.dv01:
        ld      scd_cor(r4),r5          # r5 = assoc. COR address
        ldob    cor_crstate(r5),r6      # r6 = COR registration state
        cmpobne.f corcrst_usersusp,r6,.dv200 # Jif copy operation not user
                                        #  suspended
        ldob    scd_type(r4),r6         # r6 = SCD type code
        cmpobe.f scdt_remote,r6,.dv200 # Jif remote copy device
        ld      scd_link(r4),r4         # r4 = next SCD on list
        cmpobne.f 0,r4,.dv01            # Jif more SCDs to check
.dv02:
        ld      vd_dcd(r15),r4          # r4 = DCD assoc. with VDD
        cmpobe.t 0,r4,.dv03             # Jif no DCD assoc. with VDD
        ld      dcd_cor(r4),r5          # r5 = assoc. COR address
        ldob    cor_crstate(r5),r6      # r6 = COR registration state
        cmpobne.f corcrst_usersusp,r6,.dv200 # Jif copy operation not user
                                        #  suspended
        ldob    dcd_type(r4),r6         # r6 = DCD type code
        cmpobe.f dcdt_remote,r6,.dv200 # Jif remote copy device
.dv03:
        mov     g3,r10                  # save g3
        mov     g4,r11                  # save g4
        ldconst 0,g0
        ldconst 0xffff,g1
.dv04:
        ld      vd_scdhead(r15),r4      # r4 = first SCD assoc. with VDD
        cmpobe.t 0,r4,.dv07             # Jif no SCDs assoc. with VDD
        ldconst TRUE,r3                 # set config. change occurred flag
        ld      scd_link(r4),r7         # r7 = next SCD on list
        ld      scd_cor(r4),g3          # g3 = assoc. COR address
        ldob    scd_type(r4),r6         # r6 = SCD type code
        st      r7,vd_scdhead(r15)      # remove SCD from list
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        st      g0,scd_link(r4)         # clear link list field in SCD
        st      g0,cor_srcvdd(g3)       # remove VDD from COR
        st      g0,scd_vdd(r4)          # remove VDD from SCD
        ld      cor_destvdd(g3),r7      # r7 = dest. VDD address
        stob    g1,cor_rcscl(g3)        # "clear" cluster # in COR
        stob    g1,cor_rcsvd(g3)        # "clear" VDisk # in COR
        cmpobe.f 0,r7,.dv05             # Jif no dest. VDD address defined
        stos    g1,vd_scorvid(r7)       # "clear" source VDD vid in
                                        #  dest. copy device VDD
.dv05:
        cmpobne.f scdt_both,r6,.dv06    # Jif not both local and remote copy
                                        #  device
        stob    g1,cor_rscl(g3)         # "clear" cluster # in COR
        stob    g1,cor_rsvd(g3)         # "clear" VDisk # in COR
.dv06:
        ld      cor_cm(g3),g4           # g4 = assoc. CM address
        cmpobe.f 0,g4,.dv04             # Jif no CM assoc. with COR
        call    CM$pksnd_local_poll     # force a local poll operation
                                        #  to communicate the COR registration
                                        #  changes that are being made
        b       .dv04                   # and check if any more SCDs assoc.
                                        #  with this VDD
.dv07:
        st      g0,vd_scdtail(r15)      # clear SCD list tail pointer field
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                                        #  in VDD
        ld      vd_dcd(r15),r4          # r4 = DCD assoc. with VDD
        cmpobe.t 0,r4,.dv09             # Jif no DCD assoc. with VDD
        ldconst TRUE,r3                 # set config. change occurred flag
        ld      dcd_cor(r4),g3          # g3 = assoc. COR address
        ldob    dcd_type(r4),r6         # r6 = DCD type code
        st      g0,vd_dcd(r15)          # remove DCD from VDD
        st      g0,cor_destvdd(g3)      # remove VDD from COR
        st      g0,dcd_vdd(r4)          # remove VDD from DCD
        stob    g1,cor_rcdcl(g3)        # "clear" cluster # in COR
        stob    g1,cor_rcdvd(g3)        # "clear" VDisk # in COR
        cmpobne.f dcdt_both,r6,.dv08    # Jif not both local and remote copy
                                        #  device
        stob    g1,cor_rdcl(g3)         # "clear" cluster # in COR
        stob    g1,cor_rdvd(g3)         # "clear" VDisk # in COR
.dv08:
        ld      cor_cm(g3),g4           # g4 = assoc. CM address
        cmpobe.f 0,g4,.dv09             # Jif no CM assoc. with COR
        call    CM$pksnd_local_poll     # force a local poll operation
                                        #  to communicate the COR registration
                                        #  changes that are being made
.dv09:
        cmpobne TRUE,r3,.dv09d          # Jif config. changed occurred flag
                                        #  not TRUE
        call    CCSM$cco                # generate config. change occurred
                                        #  event to CCSM
.dv09d:
        mov     g14,r4                  # save g14
        mov     g8,r5                   # save g8
        mov     r14,g14                 # VID
        mov     r15,g8                  # VDD
        call    d$deletevlink           # Handle the vlink portion
        mov     r4,g14                  # restore g14
        mov     r5,g8                   # restore g8
#
        b       .dv100                  # Clean up the VDD itself
#
.dv10:
#
# --- Check if any copy operations associated with VDisk.
# --- Do not let a user delete a VDisk that is associated with
# --- a copy operation.
#
        ld      vd_scdhead(r15),r5      # r5 = first SCD on list
        cmpobne 0,r5,.dv200             # Jif VDisk is a source copy device
#
        ld      vd_dcd(r15),r5          # r5 = assoc. DCD address
        cmpobne 0,r5,.dv200             # Jif VDisk is a dest. copy device
#
# --- For each RAID in the Virtual disk, check if it is initializing and
# --- if it is, set the terminate bit. Once all RAIDs in the VDisk are
# --- done, delete the RDDs. r15 has the VDD, r14 has the VID.
#
        PushRegs(r3)
        call    DF_CancelDefrag         # Cancel the defrag that may be running
        PopRegsVoid(r3)
#
        ldconst FALSE,r13               # Raid initializing flag
        ld      vd_rdd(r15),g1          # Get 1st RAID segment
        cmpobe  0,g1,.dv26              # If no raids, go check deferred raids
#
.dv20:
        ld      rd_nvrdd(g1),r3         # Get next RAID in this VDD
        ld      rd_iprocs(g1),r4        # Get init procs left
        cmpobe  0,r4,.dv25              # Jif not initializing
#
        ldconst TRUE,r13                # Set to indicate we are waiting
#
        ldob    rd_astatus(g1),r4       # Get alternate status
        setbit  rdatermbg,r4,r4         # Set terminate background bit
        stob    r4,rd_astatus(g1)       # Save it back

c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
.dv25:
        mov     g1,g0                   # Set up to delete from RInit queue
        call    D$delrip                # Take RIP off the queue
        call    R$del_RB_qu             # Take RDD off RB_rerror_qu (in g0)
#
        mov     r3,g1                   # Get next pointer
        cmpobne 0,g1,.dv20              # Done? no, go do another
#
# --- Now check the deferred RAIDs
#
.dv26:
        ld      vd_drdd(r15),g1         # Get first deferred RAID pointer
#
.dv30:
        cmpobe  0,g1,.dv40              # Jif NULL
#
        ld      rd_nvrdd(g1),r3         # Get next RAID in this VDD
        ld      rd_iprocs(g1),r4        # Get init procs left
        cmpobe  0,r4,.dv35              # Jif not initializing
#
        ldconst TRUE,r13                # Set to indicate we are waiting
#
        ldob    rd_astatus(g1),r4       # Get alternate status
        setbit  rdatermbg,r4,r4         # Set terminate background bit
        stob    r4,rd_astatus(g1)       # Save it back

c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
.dv35:
        mov     g1,g0                   # Set up to delete from RInit queue
        call    D$delrip                # Take RIP off the queue
        call    R$del_RB_qu             # Take RDD off RB_rerror_qu (in g0)
#
        mov     r3,g1                   # Get next pointer
        b       .dv30                   # Check again for NULL
#
.dv40:
        cmpobe  FALSE,r13,.dv75         # Jif done
#
        ldconst 125,g0
        call    K$twait
#
        b       .dv10                   # Do it again
#
# --- Detach RDD's from VDD. First do the regular list, then the deferred list.
#
.dv75:
        ld      vd_rdd(r15),g1          # Get 1st RAID segment
        cmpobe  0,g1,.dv81              # If no raid segments
#
.dv80:
        ld      rd_nvrdd(g1),r3         # Get next RAID in this VDD
        call    D_deleteraid            # Delete the RAID
        mov     r3,g1                   # Get next pointer
        cmpobne 0,g1,.dv80              # Done? no, go do another
#
.dv81:
        ld      vd_drdd(r15),g1         # Get first deferred RAID pointer
#
.dv90:
        cmpobe  0,g1,.dv95              # Jif NULL
        ld      rd_nvrdd(g1),r3         # Get next pointer
        call    D_deleteraid            # Delete the RAID
        mov     r3,g1                   # Get next pointer
        b       .dv90                   # Check again
#
.dv95:
        ldconst 0,r3
        st      r3,V_vddindx[r14*4]     # Unlink VDD from VDX
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        ldos    vd_vid(r15),g0          # Get the VID
        cmpobne g0,r14,.dv101           # Branch if vdd structure hosed
#
# ---  Release VDD
#
.dv100:
        mov     r15,g0                  # Remove all mappings
        call    d$delvirtmappings       # Get rid of them
#
# --- Delete the memory allocated for the VDisk stats
#
# DEF_DeallocVDStatsMemory calls Free(g0=,g1=), but is ok via below.
c       DEF_DeallocVDStatsMemory((VDD*)r15);
#
.dv101:
c       s_Free(r15, vddsiz, __FILE__, __LINE__); # Release this VDD
#
        ldos    V_vddindx+vx_ecnt,r3    # Get count
        subo    1,r3,r3                 # Decrement
        stos    r3,V_vddindx+vx_ecnt    # Save it
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# --- Update NVRAM
#
        mov     r14,g0
        ldconst TRUE,g1                 # Indicate a deletion
        call    D$updrmtcachesingle     # Update remote cache
#
# --- Check for size out of range on hot spares.
#
        ldconst 0,g0                    # No PDD input
        PushRegs(r3)                    # Save register contents
        call    RB_CheckHSCapacity
        PopRegsVoid(r3)                 # Restore registers
#
        call    D$p2updateconfig        # Update NVRAM part II
        mov     deok,g1                 # Return OK status
#
# --- Exit
#
.dv200:
        ldconst mdvrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$delvirtmappings
#
#  PURPOSE:
#       To provide a means of processing the delete all mappings of a
#       specified VID from all servers.
#
#  CALLING SEQUENCE:
#       call    d$delvirtmappings
#
#  INPUT:
#       g0 = VDD
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$delvirtmappings:
        movl    g0,r14                  # Save the g0/g1
        ldos    vd_vid(g0),r13          # Get the VID
        ldconst MAXSERVERS-1,r12        # Index into SDD table
#
.dvm10:
        ld      S_sddindx[r12*4],r11    # SDD being checked
        cmpobe  0,r11,.dvm100           # NULL server, get next one
#
# --- Do both lists. The visible mappings first, then the invisible ones.
#
        lda     sd_lvm(r11),r8          # Trailing pointer
        ld      sd_lvm(r11),r9          # Get the first pointer
#
.dvm20:
        cmpobe  0,r9,.dvm40             # Jif done
#
# --- Traverse the list until a NULL pointer is found or the LVM
# --- we are trying to find is encountered.
#
        ldos    lv_vid(r9),r3           # Get the VID
        cmpobne r3,r13,.dvm30           # Jif not the one we want
#
# --- Delete the mapping, update the counts in the SDD, and inform the FE
# --- that the mappings were changed.
#
        ld      lv_nlvm(r9),r3          # Get the next pointer
        st      r3,(r8)                 # Save it in the trailer
#
c       s_Free(r9, lvsiz, __FILE__, __LINE__);
#
        ldos    sd_nluns(r11),r3        # Decrement LUN count
        subo    1,r3,r3                 # Decrement
        stos    r3,sd_nluns(r11)        # Save it
#
# --- Update remote
#
        ldos    sd_sid(r11),g0          # Set SID for input parm
        ldconst FALSE,g1                # False = do not delete
        call    D_updrmtserver          # Update server record
        call    D$signalserverupdate    # Indicate that the update occurred
        b       .dvm100                 # Done, since only one mapping can exist
#
.dvm30:
        lda     lv_nlvm(r9),r8          # Get trailing pointer
        ld      lv_nlvm(r9),r9          # Get next LVM
        b       .dvm20                  # Check again
#
# --- Do the invisible ones.
#
.dvm40:
        lda     sd_ilvm(r11),r8         # Trailing pointer
        ld      sd_ilvm(r11),r9         # Get the first pointer
        ldconst FALSE,r14               # Did we get rid of any mappings?
#
.dvm50:
        cmpobe  0,r9,.dvm100            # Jif done
#
# --- Traverse the list until a NULL pointer is found or the LVM
# --- we are trying to find is encountered.
#
        ldos    lv_vid(r9),r3           # Get the VID
        cmpobne r3,r13,.dvm60           # Jif not the one we want
#
# --- Delete the mapping, update the counts in the SDD, and inform the FE
# --- that the mappings were changed.
#
        ld      lv_nlvm(r9),r3          # Get the next pointer
        st      r3,(r8)                 # Save it in the trailer
#
c       s_Free(r9, lvsiz, __FILE__, __LINE__);
#
        ldos    sd_nluns(r11),r3        # Decrement LUN count
        subo    1,r3,r3                 # Decrement
        stos    r3,sd_nluns(r11)        # Save it
#
# --- Update remote
#
        ldos    sd_sid(r11),g0          # Set SID for input parm
        ldconst FALSE,g1                # False = do not delete
        call    D_updrmtserver          # Update server record
        call    D$signalserverupdate    # Indicate that the update occurred
        b       .dvm100                 # Done, since only one mapping can exist
#
.dvm60:
        lda     lv_nlvm(r9),r8          # Get trailing pointer
        ld      lv_nlvm(r9),r9          # Get next LVM
        b       .dvm50                  # Check again
#
.dvm100:
        subo    1,r12,r12               # Decrement index
        cmpible 0,r12,.dvm10            # Jif more to do
#
        movl    r14,g0                  # Restore g0/g1
        ret
#
#**********************************************************************
#
#  NAME: D_rrddpsd
#
#  PURPOSE:
#       To provide a means of releasing a specific RDD and its
#       associated PSDs back to local SRAM.
#
#  DESCRIPTION:
#       Each PSD referred to by the RDD is released back to local SRAM.
#       Then the RDD is released back to local SRAM.
#
#  CALLING SEQUENCE:
#       call    D_rrddpsd
#
#  INPUT:
#       g0 = RDD
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
# void DC_RelRDDPSD(RDD *pRDD);
        .globl  DC_RelRDDPSD
DC_RelRDDPSD:
D_rrddpsd:
        movl    g0,r14                  # Save g0-g1
        cmpobe  0,r14,.drr100           # Jif NULL RDD
#
# --- Check if Raid is initializing
#
        ldob    rd_status(r14),r4       # Get RDD status
        cmpobne rdinit,r4,.drr20        # Jif RDD not actively initting
#
# --- Set the terminate background bit for RDD
#
        ldob    rd_astatus(r14),r3      # Get alternate status
        setbit  rdatermbg,r3,r3         # Set terminate background bit
        stob    r3,rd_astatus(r14)      # Save it back

c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
# --- Wait until initing is done
#
.drr10:
        ld      rd_iprocs(r14),r4       # Get initialization processes running
        cmpobe  0,r4,.drr30             # Jif done initializing
#
        ldconst 125,g0                  # Wait one second
        call    K$twait
        b       .drr10
#
# --- If Raid is scheduled to be initialized, the function will pull it
# --- off of the 'scheduled to be initialized' queue. If it is not there,
# --- this will do nothing.
#
.drr20:
        call    D$delrip                # Take RIP off the queue
        call    R$del_RB_qu             # Take RDD off RB_rerror_qu (in g0)
#
# --- Loop thru each PSD
#
.drr30:
        ld      rd_psd(r14),r13         # Get 1st PSD
        mov     r13,r11
        cmpobe  0,r13,.drr50            # Jif NULL PSD
#
        ldob    rd_type(r14),r10        # Get type
        cmpobe  rdlinkdev,r10,.drr45    # Jif linked type
        cmpobe  rdslinkdev,r10,.drr45    # Jif snapshot type
#
# --- Release PSD. We will loop through until the circularly linked
# --- list hits the PSD we started with.
#
.drr40:
        mov     r11,g0                  # Get the PSD
c       DA_Release((void *)g0);         # Release the space
#
.drr45:
        ld      ps_npsd(r11),r12        # Next PSD (for linked there is only one)
c       s_Free(r11, psdsiz, __FILE__, __LINE__);  # Release current PSD
#
        mov     r12,r11                 # Move next to current PSD
        cmpobne r13,r12,.drr40          # Do another one if not looped around
#
# --- Release RDD
#
.drr50:
        ldos    rd_psdcnt(r14),g1       # Compute size of RDD
        mulo    4,g1,g1
        lda     rddsiz(g1),g1
c       s_Free(r14, g1, __FILE__, __LINE__);
#
# --- Exit
#
.drr100:
        movl    r14,g0                  # Restore g0-g1
        ret
#
#**********************************************************************
#
#  NAME: d$rcev
#
#  PURPOSE:
#       To provide a means of releasing a specific CEV and its
#       associated structures back to local SRAM.
#
#  DESCRIPTION:
#       Each DAML referred to by the CEV is released back to local SRAM.
#       Each RDD referred to by the CEV is released back to local SRAM
#       along with the associated PSDs. The CEV is released also.
#
#  CALLING SEQUENCE:
#       call    d$rcev
#
#  INPUT:
#       g0 = CEV
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$rcev:
        cmpobe  0,g0,.rv100             # If null, nothing to delete
        movl    g0,r14                  # Save g0-g1
        ldconst cemaxraid-1,r8          # Get the size of the RDD list
#
.rv10:
        ld      ce_rdd(r14)[r8*4],g0    # Get the RDD pointer
        cmpobe  0,g0,.rv50              # If null, get next list
#
# --- Deallocate the space and release RDD and PSDs. As we process each PSD
# --- deallocate the memory for it also since we are getting rid of all of it.
#
        mov     g0,r13                  # Save the RDD
        ld      rd_psd(g0),g0           # Get the first PSD
        mov     g0,r12                  # Save it for the end check
#
.rv20:
c       g0 = (UINT32)(void *)DA_Release((void *)g0); # Release the space
        ld      ps_npsd(g0),r11         # Get the next one
#
c       s_Free(g0, psdsiz, __FILE__, __LINE__); # Release current PSD
#
        mov     r11,g0                  # Restore the NEXT pointer
        cmpobne g0,r12,.rv20            # Jif not done
#
# --- Release RDD
#
        mov     r13,g0                  # Pass RDD
        ldos    rd_psdcnt(r13),g1       # Compute size of RDD
        mulo    4,g1,g1
c       s_Free(r13, g1 + sizeof(RDD), __FILE__, __LINE__); # Release RDD
#
.rv50:
        subo    1,r8,r8                 # Decrement counter
        cmpible 0,r8,.rv10              # If not last list, do again
#
# --- Remove the RDA
#
        mov     r14,g0                  # Restore CEV pointer
        PushRegs(r4)                    # Save register contents
        call    DEF_RelRDA              # Release it
        PopRegsVoid(r4)                 # Restore registers
#
# --- Remove CEV itself
#
        ldos    ce_numdaml(r14),g1      # Set up size to return
        mulo    4,g1,g1                 # Get size of CEV
        ldconst cevsiz,r4               # Load base size
        addo    r4,g1,g1                # Add base size
c       s_Free(r14, g1, __FILE__, __LINE__); # Delete space
        movl    r14,g0                  # Restore saved regs
#
# --- Exit
#
.rv100:
        ret
#
#**********************************************************************
#
#  NAME: d$brddpsd
#
#  PURPOSE:
#       To provide a common means of building the RDD and PSDs associated
#       with the RDA.
#
#  DESCRIPTION:
#       An RDD is assigned from local SRAM. A PSD entry is then assigned
#       from local SRAM for each DAM entry within the RDA. The common
#       fields within the RDD and PSD are initialized.
#
#  CALLING SEQUENCE:
#       call    d$brddpsd
#
#  INPUT:
#       g4  = RDA (list of DAML structures)
#       g10 = drive count
#       g13 = RAID ID
#       g14 = RAID type
#
#  OUTPUT:
#       g6  = RDD
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$brddpsd:
        movl    g0,r14                  # Save g0-g1
#
# --- Allocate RDD and link to RDD index
#
        shlo    2,g10,g0                # Compute variable sized portion
                                        #  of the RDD
c       g6 = s_MallocC((rddsiz|BIT31) + g0, __FILE__, __LINE__); # Allocate and clear RDD
#
# --- Initialize RDD
#
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */

        stob    g14,rd_type(g6)         # Set up RAID type
        stos    g13,rd_rid(g6)          # Set up RAID ID
        ldconst rdop,r3                 # Set status to operational
        stob    r3,rd_status(g6)
        stos    g10,rd_psdcnt(g6)       # Set up drive count
#
        mov     g4,r13                  # Get RDA origin
        lda     rd_psd(g6),r12          # Get 1st PSD ptr origin
        mov     0,r3                    # Clear previous PSD ptr
        mov     g10,r11                 # Get drive count
#
# --- Allocate next PSD
#
.bp10:
c       g0 = s_MallocC(psdsiz|BIT31, __FILE__, __LINE__); # Allocate and clear PSD
        st      g0,(r12)                # Link RDD to PSD
        cmpobe  0,r3,.bp20              # Jif previous PSD undefined
#
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      g0,ps_npsd(r3)          # Link previous PSD to this PSD
#
# --- Initialize PSD
#
.bp20:
        ld      (r13),r10               # Get corresponding DAML
        ld      da_pdd(r10),r10         # Get corresponding PDD
        ldos    pd_pid(r10),r4          # Set up PID
        stos    r4,ps_pid(g0)
#
        ldconst psop,r4                 # Set status to operational
        stob    r4,ps_status(g0)
        stos    g13,ps_rid(g0)          # Set up RAID ID
#
# --- Bump to next PSD
#
        lda     4(r12),r12              # Advance PSD ptr
        lda     4(r13),r13              # Advance RDA ptr
        mov     g0,r3                   # Save previous PSD ptr
        subo    1,r11,r11               # Adjust remaining drive count
        cmpobne 0,r11,.bp10             # Jif more
#
# --- Link last PSD to first PSD
#
        ld      rd_psd(g6),r4           # Link last PSD to 1st PSD
        st      r4,ps_npsd(r3)
#
# --- Exit
#
        movl    r14,g0                  # Restore g0-g1
#
        ret
#
#**********************************************************************
#
#  NAME: d$get0stripe
#
#  PURPOSE:
#       To provide a common means of extracting and validating the
#       RAID 0/10 stripe size from the MRP packet.
#
#  DESCRIPTION:
#       The stripe size is extracted from the CEV structure. The size
#       is then checked to determine if it is a power of 2. If so, a
#       normal return is made to the caller.
#
#  CALLING SEQUENCE:
#       call    d$get0stripe
#
#  INPUT:
#       g0 = CEV
#
#  OUTPUT:
#       g1 = error code
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$get0stripe:
#
# --- Get stripe size from MRP
#
        ldconst deinvstripe,g1          # Prep possible error code
        ldos    ce_sps(g0),r5           # Get stripe size
        scanbit r5,r3                   # Check for power of 2
        bno     .gs100                  # Jif zero
#
        clrbit  r3,r5,r4                # Clear 1st bit found
        cmpobne 0,r4,.gs100             # Jif not zero
#
# --- Validate stripe size
#
        ldconst MIN0STRIPE,r3           # Get minimum stripe size
        cmpobl  r5,r3,.gs100            # Jif below minimum
#
        ldconst MAX0STRIPE,r3           # Get maximum stripe size
        cmpobg  r5,r3,.gs100            # Jif above maximum
#
        mov     ecok,g1                 # Set OK return
#
# --- Exit
#
.gs100:
        ret
#
#**********************************************************************
#
#  NAME: d$get5stripe
#
#  PURPOSE:
#       To provide a common means of extracting and validating the
#       RAID 5 stripe size from the CEV structure.
#
#  DESCRIPTION:
#       The stripe size is extracted from the CEV structure. The size
#       is then checked to determine if it is a power of 2. If so, a
#       normal return is made to the caller.
#
#  CALLING SEQUENCE:
#       call    d$get5stripe
#
#  INPUT:
#       g0 = CEV
#
#  OUTPUT:
#       g1 = error code
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$get5stripe:
#
# --- Get stripe size from MRP
#
        ldconst deinvstripe,g1          # Prep possible error code
        ldos    ce_sps(g0),r5           # Get stripe size
        scanbit r5,r3                   # Check for power of 2
        bno     .gt100                  # Jif zero
#
        clrbit  r3,r5,r4                # Clear 1st bit found
        cmpobne 0,r4,.gt100             # Jif not zero
#
# --- Validate stripe size
#
        ldconst MIN5STRIPE,r3           # Get minimum stripe size
        cmpobl  r5,r3,.gt100            # Jif below minimum
#
        ldconst MAX5STRIPE,r3           # Get maximum stripe size
        cmpobg  r5,r3,.gt100            # Jif above maximum
#
        mov     ecok,g1                 # Set OK return
#
# --- Exit
#
.gt100:
        ret
#
#**********************************************************************
#
#  NAME: D_allocvdd
#
#  PURPOSE:
#       To provide a standard means of allocating the space and
#       clearing out the memory for a virtual device record (VDD).
#
#  DESCRIPTION:
#       This function will allocate fixed memory for an VDD record
#       and will clear out the memory in anticipation of the caller
#       filling in pertinent fields.
#
#  CALLING SEQUENCE:
#       call D_allocvdd
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 - address of the VDD.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# VDD *DC_AllocVDD(void);
        .globl  DC_AllocVDD             # C access
DC_AllocVDD:
D_allocvdd:
c       g0 = s_MallocC(vddsiz|BIT31, __FILE__, __LINE__); # Allocate and clear VDD
        ret
#
#**********************************************************************
#
#  NAME: D_allocrdd
#
#  PURPOSE:
#       To provide a standard means of allocating the space and
#       clearing out the memory for a RAID record (RDD).
#
#  DESCRIPTION:
#       This function will allocate fixed memory for an RDD record
#       and will clear out the memory in anticipation of the caller
#       filling in pertinent fields.
#
#  CALLING SEQUENCE:
#       call D_allocrdd
#
#  INPUT:
#       g0 - number of PSDs in this RAID.
#
#  OUTPUT:
#       g0 - address of the RDD.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# RDD *DC_AllocRDD(UINT32 numPSDs);
        .globl  DC_AllocRDD             # C access
DC_AllocRDD:
D_allocrdd:
c       g0 = s_MallocC((rddsiz|BIT31) + (g0*4), __FILE__, __LINE__);
        ret
#
#**********************************************************************
#
#  NAME: D_allocpdd/_D_freepdd
#
#  PURPOSE:
#       To provide a standard means of freeing or allocating the space and
#       clearing out the memory for a physical device record (PDD).
#
#  DESCRIPTION:
#       This function will allocate fixed memory for an PDD record
#       and will clear out the memory in anticipation of the caller
#       filling in pertinent fields.
#
#       For freeing, it will release the memory and associated DAMLs.
#
#  CALLING SEQUENCE:
#       call DC_AllocPDD
#       call D_freepdd
#
#  INPUT:
#       g0 - address of the PDD to be freed.
#
#  OUTPUT:
#       g0 - address of the PDD allocated.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# PDD* DC_AllocPDD(void);
        .globl  DC_AllocPDD
DC_AllocPDD:
c       g0 = s_MallocC(pddsiz|BIT31, __FILE__, __LINE__);
        ret
#
# C access
# void DC_RelPDD(PDD* pPDD);
        .globl  DC_RelPDD
DC_RelPDD:
D_freepdd:
#
#
        cmpobe  0,g0,.fpdd100           # Jif already gone
        ld      pd_daml(g0),r15         # Get DAML
#
        ld      pd_dev(g0),r4           # get dev pointer
        cmpobe  0,r4,.fpdd050           # if null jump
? # crash - cqt# 24581 - 2008-06-13 -- BE PDD points to DEV in DELAY_MEMORY_FREE - failed @ ld 40+r4,r5 with babababa
?       ld      dv_pdd(r4),r5
        cmpobne r5,g0,.fpdd050          # if dev->pdd != pdd being freed jump
        st      0,dv_pdd(r4)            # set dev->pdd to NULL
.fpdd050:
c       s_Free(g0, pddsiz, __FILE__, __LINE__);
#
        cmpobe  0,r15,.fpdd100          # Jif no DAML
        mov     r15,g0
c       s_Free(g0, DAML_SIZE, __FILE__, __LINE__);
#
# --- Exit
#
.fpdd100:
        ret
#
#**********************************************************************
#
#  NAME: D_allocpsd
#
#  PURPOSE:
#       To provide a standard means of allocating the space and
#       clearing out the memory for a physical segment descriptor (PSD).
#
#  DESCRIPTION:
#       This function will allocate fixed memory for an PSD record
#       and will clear out the memory in anticipation of the caller
#       filling in pertinent fields.
#
#  CALLING SEQUENCE:
#       call D_allocpsd
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 - address of the PSD.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# PSD* DC_AllocPSD(void);
        .globl  DC_AllocPSD             # C access
DC_AllocPSD:
D_allocpsd:
c       g0 = s_MallocC(psdsiz|BIT31, __FILE__, __LINE__);
        ret
#
#**********************************************************************
#
#  NAME: d$deletedevice
#
#  PURPOSE:
#       To provide a common means of deleting a device which is no longer
#       in use.
#
#  DESCRIPTION:
#       The device identified is deleted from the tables and the NVRAM
#       is saves. For a disk drive, the drive is checked to make sure
#       that there are no RAIDs defined on the device. For an SES device
#       it is simply blown away.
#
#  CALLING SEQUENCE:
#       call    d$deletedevice
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return length
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$deletedevice:
        ld      mr_ptr(g0),g0           # Get parm block
        ldob    mxd_type(g0),r15        # Get device type
        ldos    mxd_pid(g0),r14         # Get the PID
#
        ldconst deinvopt,g1             # Prep error code
        cmpobne mxdtdisk,r15,.xd50      # Jif not a disk
#
        ldconst deinvpid,g1             # Check for valid PDD
        ldconst MAXDRIVES,r4            # Max value plus one
        cmpobge r14,r4,.xd100           # Jif in error (too big of PID)
#
        ld      P_pddindx[r14*4],g4     # Get the PDD
        cmpobe  0,g4,.xd100             # Invalid PDD (null pointer)
#
        ldconst dedevused,g1            # Prep error code
        call    D_convpdd2psd           # Check for valid data on device
        cmpobne 0,g0,.xd100             # Jif data defined on device

        ld      pd_dev(g4),g0           # get device
        cmpobe  0,g0,.xd45              # is it null
c       g0 = FAB_IsDevInUse((struct DEV *)g0);
        cmpobne 0,g0,.xd100             # Jif device in use.
.xd45:
#
# --- Finally validated. Delete the PDD and record it in the tables.
#
        ldconst 0,r4
        st      r4,P_pddindx[r14*4]     # Zero the entry in the PDX
#
        ldos    px_ecnt+P_pddindx,r4
        subo    1,r4,r4
        stos    r4,px_ecnt+P_pddindx    # Decrement device count
        b       .xd90                   # Update NVRAM
#
.xd50:
        cmpobne mxdtses,r15,.xd100      # Jif not an enclosure (error)
#
        ldconst deinvpid,g1             # Check for valid SES pointer
        ldconst MAXSES,r4               # Max value plus one
        cmpobge r14,r4,.xd100           # Jif in error (too big of PID)
#
        ld      E_pddindx[r14*4],g4     # Get the PDD
        cmpobe  0,g4,.xd100             # Invalid PDD (null pointer)
#
        ldconst 0,r4
        st      r4,E_pddindx[r14*4]     # Zero the entry in the PDX
#
        ldos    px_ecnt+E_pddindx,r4
        subo    1,r4,r4
        stos    r4,px_ecnt+E_pddindx    # Decrement device count
#
.xd90:
        mov     g4,g0                   # Input to free memory
        call    D_freepdd               # Free it
#
        call    D$p2updateconfig        # Save NVRAM
        ldconst deok,g1                 # Return status
#
.xd100:
        ldconst mxdrsiz,g2              # Return length
        ret
#
#**********************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
