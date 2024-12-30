# $Id: ldd.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: ldd.inc (Linked Device Description definitions)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the Linked Device
#       Data (LDD) packet. This structure is used to manage linked
#       devices, both MAGNITUDE link and Foreign Target devices.
#
#  Copyright (c) 2000-2002 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
#       ld_class definitions - same as pd_class with the following exceptions
#
#       Note: Correlate changes to ld_class with pd_class in pdd.inc.
#
        .set    ldmld,pdmld             # MAGNITUDE link device
        .set    ldftd,pdftd             # Foreign Target device
#
# --- Linked device state code (ld_state) definitions
#
        .set    ldd_st_undef,0          # undefined (non-existant)
        .set    ldd_st_op,1             # operational
        .set    ldd_st_inop,2           # inoperative
        .set    ldd_st_uninit,3         # uninitialized (missing)
        .set    ldd_st_pterm,4          # pending termination
        .set    ldd_st_term,5           # terminated
#
# --- Begin basic structure --------------------------------------------
#
# --- Section - Fields to save in NVRAM
#                                                       ***************
        .set    ld_class,0              # Device class              <b>
        .set    ld_state,ld_class+1     # LDD state code            <b>
        .set    ld_pmask,ld_state+1     # path mask                 <b>
                                        #  Path # bit map mask
                                        #  0 = path blocked
                                        #  1 = path enabled
        .set    ld_ppri,ld_pmask+1      # path priority             <b>
                                        #  Path # bit map
                                        #  checked if path not blocked
                                        #  0 = secondary path
                                        #  1 = primary path
        .set    ld_owner,ld_ppri+1      # Current Owning Controller <w>
        .set    ld_devcap,ld_owner+4    # Device capacity in sectors<l>
#                                                                  *****
        .set    ld_vendid,ld_devcap+8   # Vendor ID                 <l>
        .set    ld_rev,ld_vendid+8      # Product revision          <w>
        .set    ld_altid,ld_rev+4       # Alternate ID              <s>
#
#   Reserved 2                                                     2<b>
#                                                                  *****
        .set    ld_prodid,ld_rev+8      # Product ID                <q>
#                                                                  *****
        .set    ld_serial,ld_prodid+16  # Serial number             <t>*
#
#   Reserved 4                                                     4<b>
#                                                                  *****
        .set    ld_lun,ld_serial+16     # Foreign Target LUN #      <s>
        .set    ld_basevd,ld_lun+2      # base MAG VDisk #          <s>
        .set    ld_basecl,ld_basevd+2   # base MAG cluster #        <b>
#
#   Reserved 3                                                     3<b>
#
        .set    ld_basesn,ld_basecl+4   # base MAG serial #         <w>
#
#   Reserved 4                                                     4<b>
#                                                                  *****
        .set    ld_basename,ld_basesn+8 # Base MAG device name    16<b>
#                                                                  *****
        .set    ld_basenode,ld_basename+16# Base MAG node name      <l>
#
#   Reserved 8                                                     8<b>
#                                                                  *****
#
# --- Section - Fields not needed in NVRAM
#
        .set    ld_tpmthd,ld_basenode+16 # assoc. TPMT list head ptr <w>
        .set    ld_lasttpmt,ld_tpmthd+4 # last path TPMT used       <w>
        .set    ld_ailthd,ld_lasttpmt+4 # active ILT op. list       <w>
                                        #  head pointer
        .set    ld_ailttl,ld_ailthd+4   # active ILT op. list tail  <w>
                                        #  pointer
#                                                                  *****
        .set    ld_pcb,ld_ailttl+4      # activate process PCB      <w>
                                        #  Contains the PCB address
                                        #  of the process that updates
                                        #  paths for the specified
                                        #  linked device.
        .set    ld_flag1,ld_pcb+4       # process event flag #1     <b>
        .set    ld_lock,ld_flag1+1      # LDD lock                  <b>
        .set    ld_numpath,ld_lock+1    # Number of paths           <b>
#
#   Reserved 1                                                      <b>
#
        .set    ld_ord,ld_numpath+2     # LDD ordinal               <s>
        .set    ld_rtycnt,ld_ord+2      # error retry ILT count     <s>
#
#   Reserved 4                                                     4<b>
#
# --- End basic structure ----------------------------------------------
#
        .set    lddsiz,ld_rtycnt+6      # Size of LDD
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
