# $Id: gan.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: gan.inc (Get All Next Command Response)
#
#  DESCRIPTION:
#
#
#  Copyright (c) 1996 - 1999 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
        .set    CT_HDR,16               # length of CT_HDR
        .set    PID_MASK,0xffffff00     # mask to isolate PID
        .set    SCSI_FCP,0x01           # FC4 type SCSI FCP
#
#
# --- Begin structure -------------------------------------------------
#
        .set    gan_ptype,0             # port type                     <b>
        .set    gan_pid,gan_ptype+1     # port ID (ALPA)               3<b>

        .set    gan_P_name,gan_pid+3    # port name                    8<b>
        .set    gan_spnl,gan_P_name+8   # symbolic port name length     <b>
        .set    gan_spn,gan_spnl+1      # symbolic port name         255<b>

        .set    gan_N_name,gan_spn+255  # node name                    8<b>
        .set    gan_snnl,gan_N_name+8   # symbolic node name length     <b>
        .set    gan_snn,gan_snnl+1      # symbolic node name         255<b>

        .set    gan_IPA,gan_snn+255     # initial process associator   8<b>
        .set    gan_IP_node,gan_IPA+8   # IP address (of node)        16<b>
        .set    gan_COS,gan_IP_node+16  # class of service             4<b>
        .set    gan_FC4,gan_COS+4       # FC-4 types                  32<b>
        .set    gan_IP_port,gan_FC4+32  # IP address (of port)        16<b>
        .set    gan_F_name,gan_IP_port+16 # fabric port name           8<b>
                                        # 1 spare byte                  <b>
        .set    gan_HA,gan_F_name+8     # Hard Address                 3<b>
#
# --- End structure ---------------------------------------------------
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
