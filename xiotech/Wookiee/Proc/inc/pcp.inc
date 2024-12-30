#**********************************************************************
#
#  NAME: pcp.inc (Process Control Packet Definitions]
#
#  DESCRIPTION:
#
#       This file contains the data structures and definitions to
#       support process control requests between a client and the
#       primary copy provider.
#
#  Copyright (c) 2001 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
# --- General definitions
#

#
# --- Completion status code (pcp1_status) definitions
#
        .set    pcp1st_ok,0                     # successful
        .set    pcp1st_llfuerr,1                # process error
        .set    pcp1st_pollinhibit,2            # poll inhibit
        .set    pcp1st_taskend,0xff             # CM task termination
#
# --- Request function code (pcp2_function) definitions
#
        .set    pcp1fc_updateerr,1              # update error
        .set    pcp1fc_srccpymv,2               # source copy device moved
        .set    pcp1fc_dstcpymv,3               # source copy device moved
        .set    pcp1fc_swap,4                   # swap raids of mirror
        .set    pcp1fc_break,5                  # break mirror/copy
        .set    pcp1fc_pause,6                  # pause mirror/copy
        .set    pcp1fc_resume,7                 # resume mirror/copy
        .set    pcp1fc_abort,8                  # abort mirror/copy
        .set    pcp1fc_poll,9                   # poll assoc. remote MAGNITUDES
        .set    pcp1fc_srcerr,0x0a              # I/O error occurred on src copy device

        .set    pcp1fc_autobreak,0x0c           # auto break
        .set    pcp1fc_autoabort,0x0d           # auto abort

        .set    pcp1fc_OwnerAcq,0x10            # Ownership Acquired task
        .set    pcp1fc_spndcpytask,0x11         # Suspend copy task

        .set    pcp1fc_InstantMirror,0x12       # Instant Mirror  ## VIJAY

#
#       Process Control Packet Level #1 Definition
#       -----------------------------------------------
#
#       This table defines the data structure used by a client to
#       manage a segment copy request. This level is for the exclusive
#       use of the client.
#
#       This data structure is built and maintained in an ILT.
#
# --- Begin PCP1 main structure ---------------------------------------------
#
        .set    pcp1_link,il_fthd       # link list field                  <w>
        .set    pcp1_cr,il_cr           # completion handler routine       <w>

        .set    pcp1_pcb,il_w0          # client process pcb address       <w>
        .set    pcp1_status,il_w1+0     # completion status                <b>
        .set    pcp1_rtstate,il_w1+1    # requested task state             <b>
                                        # 2 spare bytes                   2<b>
        .set    pcp1_reg1,il_w2         # general purpose register 1       <w>
        .set    pcp1_reg2,il_w3         # general purpose register 2       <w>
        .set    pcp1_reg3,il_w4         # general purpose register 3       <w>
        .set    pcp1_reg4,il_w5         # general purpose register 4       <w>
        .set    pcp1_reg5,il_w6         # general purpose register 5       <w>

        .set    pcp1_cm,il_w7           # cm address                       <w>

#
# --- End PCP1 main structure -----------------------------------------------
#
#
#       Process Control Packet Level #2 Definition
#       -----------------------------------------------
#
#       This table defines the data structure used by a client to
#       request a process control operation.
#
#       This level is built by the client but is used by the
#       process services to manage the request. This data structure
#       is built and maintained in an ILT.
#
# --- Begin PCP2 main structure ---------------------------------------------
#
        .set    pcp2_link,il_fthd       # link list field                  <w>

        .set    pcp2_status,il_pcb      # completion status code           <b>
        .set    pcp2_function,il_pcb+1  # requested function               <b>
                                        #   0 = none
                                        # 2 spare bytes                   2<b>
        .set    pcp2_handler,il_cr      # Vsync handler script             <w>
                                        #   0 = none

        .set    pcp2_rqctrlsn,il_w0     # requesting controller s/n        <w>
        .set    pcp2_rid,il_w1          # copy reg. ID                     <w>
        .set    pcp2_rcsn,il_w2         # copy reg. MAG SN                 <w>
        .set    pcp2_rcscl,il_w3+0      # copy reg. MAG src cl #           <b>
        .set    pcp2_rcsvd,il_w3+1      # copy reg. MAG src Vdisk #        <b>
        .set    pcp2_rcdcl,il_w3+2      # copy reg. MAG dst cl #           <b>
        .set    pcp2_rcdvd,il_w3+3      # copy reg. MAG dst Vdisk #        <b>

        .set    pcp2_rssn,il_w4         # copy reg. src MAG SN             <w>
        .set    pcp2_rdsn,il_w5         # copy reg. dst MAG SN             <w>
        .set    pcp2_rscl,il_w6+0       # copy reg. src MAG cl #           <b>
        .set    pcp2_rsvd,il_w6+1       # copy reg. src MAG Vdisk #        <b>
        .set    pcp2_rdcl,il_w6+2       # copy reg. dst MAG cl #           <b>
        .set    pcp2_rdvd,il_w6+3       # copy reg. dst MAG Vdisk #        <b>

        .set    pcp2_cor,il_w7          # COR address                      <w>
#
# --- End PCP2 main structure -----------------------------------------------
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
