# $Id: fmtunit.inc 88783 2009-06-11 16:30:31Z m4 $
#*****************************************************************************
#
#  NAME: fmtunit.inc  (FORMAT UNIT process definitions)
#
#  DESCRIPTION:
#
#     Defines equates and data structures for managing a FORMAT UNIT
#     CDB.
#
#  Copyright (c) 1999 - 2007 Xiotech Corporarion.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
        .set    fmtlen,128              # # sectors to format/VRP
        .set    fmtcount,4              # # format VRPs to keep active
#
# --- Begin structure --------------------------------------------------------
#
# --- Format Unit Process Management Table (FUPMT)
#        (maintained in ILT nest level 1 of the task's sec. ILT)
#
        .set    fpmt_link,il_fthd       # Link list field               <w>
        .set    fpmt_priilt,il_misc     # primary task ILT address      <w>
                                        #  0 denotes pri. ILT completed
        .set    fpmt_vdmt,il_w0         # assoc. VDMT address           <w>
        .set    fpmt_ilmt,il_w1         # assoc. ILMT address           <w>
        .set    fpmt_secvrp,il_w2       # secondary ILT/VRP list        <w>
        .set    fpmt_flag,il_w3         # Flags byte #1                 <b>
                                        # Bit 7 = 1=process aborted
                                        #     6 = 1=IMMED flag set
                                        #     5 = 1=overwrite pattern
                                        #           with LBA
                                        #     4 =
                                        #     3 =
                                        #     2 =
                                        #     1 =
                                        #     0 = 1=process completed
        .set    fpmt_secvrpcnt,il_w3+1  # # secondary VRPs outstanding  <b>
        .set    fpmt_srpcnt,il_w3+2     # # SRPs outstanding            <b>
        .set    fpmt_status,il_w3+3     # completion status code        <b>
        .set    fpmt_sda,il_w4          # current process SDA           <l>
        .set    fpmt_endsda,il_w6       # ending SDA                    <l>
#
# --- End of FUPMT data structure
#
#
# --- Primary task ILT nest level #3 data structure used to stage FORMAT
#       UNIT parameters prior to starting the format process to the
#       Virtual Drive.
#
        .set    fu3_pattype,il_w0       # pattern type code             <b>
        .set    fu3_flags,il_w0+1       # Flag byte #1                  <b>
                                        # Bit 7 = 1=process aborted
                                        #     6 = 1=IMMED flag set
                                        #     5 = 1=overwrite pattern
                                        #           with LBA
                                        #     4 =
                                        #     3 =
                                        #     2 =
                                        #     1 =
                                        #     0 =
        .set    fu3_patsgl,il_w1        # pattern SGL/buffer address    <w>
                                        #  0 denotes none
        .set    fu3_fupmt,il_w2         # format unit PMT address       <w>
                                        #  0 denotes none
#
# --- Pattern type code definitions
#
        .set    fupat_dflt,0            # default pattern type code
        .set    fupat_host,1            # use host supplied pattern
#
#---------------------------------------------------------------------------
#
#       FORMAT UNIT process ILTs (other then the FUPMT) are
#       assigned as either primary or secondary ILT/VRPs.
#       Primary process ILT/VRPs are sent to the MAGNITUDE
#       initially to allocate SGL/buffers to be used for
#       writing the FORMAT UNIT initialization pattern to
#       the Virtual Drive. Primary process ILT/VRPs are sent
#       to the MAGNITUDE and remain outstanding until the
#       FORMAT UNIT operation has been completed. Each
#       primary process ILT/VRP is associated with a secondary
#       process ILT/VRP during process initialization. Each
#       primary ILT/VRP shall precipitate an ILT/SRP to be
#       issued by the MAGNITUDE. These ILT/SRPs are saved
#       during the process and their associated SGL/buffers
#       are used in secondary ILT/VRPs to write the initialization
#       pattern to the Virtual Disk. When the format process
#       completes, the ILT/SRPs are returned to the MAGNITUDE
#       with an error so that the data in them is not written
#       to the Virtual Disk, but to simply return the resources
#       back to their owners. This will cause the associated
#       primary process ILT/VRPs to be returned to be deallocated.
#       Once all the resources are returned, the FORMAT UNIT
#       process is terminated and completion status to the
#       issuing host is given if appropriate.
#
#       Process ILT/VRPs, regardless of whether they are used as
#       primary or secondary ILT/VRPs contain data from the
#       associated primary task ILT FCAL parameters. This is
#       necessary for when these ILT/VRPs are sent to the MAGNITUDE.
#       This data is saved in the ILT nest level #1. The only
#       difference in this data between ILTs is the associated
#       VRP address associated with the ILT (stored in the vrvrp
#       field of the ILT).
#
#       Process ILT nest level #2 is used to manage the ILT/VRP's
#       operation. The data structures for this nest level depends
#       on whether the ILT/VRP is being used as a primary or
#       secondary process ILT/VRP. These data structures are
#       defined below.
#
#       These ILT/VRPs are all managed through the associated
#       FUPMT data structure.
#
#--------------------------------------------------------------------------
#
# --- FORMAT UNIT process ILT general data structure definitions
#
#
        .set    funl2_FCAL,il_misc      # pointer to FCAL parms.        [m1]
        .set    funl2_fupmt,il_w0       # assoc. FUPMT address          [w0]
#
# --- FORMAT UNIT process secondary ILT/VRP data structure
#
        .set    fusl2_fupmt,funl2_fupmt # assoc. FUPMT address          [w0]
        .set    fusl2_ilmt,inl2_ilmt    # assoc. ILMT address           [w2]
        .set    fusl2_pilt,il_w3        # assoc. primary ILT/VRP        [w3]
        .set    fusl2_sda,il_w4         # current SDA being processed [w4-w5]
        .set    fusl2_rcvsrp,inl2_rcvsrp # recv. SRP handler routine    [w7]

###
## Modelines:
## Local Variables:
## tab-width: 4
## indent-tabs-mode: nil
## End:
## vi:sw=4 ts=4 expandtab
