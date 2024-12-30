# $Id: qcs.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#   NAME: qcs.inc
#
#   DESCRIPTION:
#
#       To define a common means of queuing control between software
#       layers.
#
#   Copyright (c) 1996 - 2002 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates ---------------------------------------------------------
#
#       bit flags for qc_flags field
#
        .set    qcpbusy0,0              # queue handler 0 busy flag
        .set    qcpbusy1,1              # queue handler 1 busy flag
        .set    qcpbusy2,2              # queue handler 2 busy flag
        .set    qcpbusy3,3              # queue handler 3 busy flag
        .set    qcpbusy4,4              # queue handler 4 busy flag
        .set    qcpbusy5,5              # queue handler 5 busy flag
        .set    qcpbusy6,6              # queue handler 6 busy flag
        .set    qcpbusy7,7              # queue handler 7 busy flag
#
#       bit flags for <qc_stat> status bits
#       these indicate there are queued entries ready to process for this queue
#       set up highest priority to lowest priority
#
        .set    qc0,0
        .set    qc1,1
        .set    qc2,2
        .set    qc3,3
        .set    qc4,4
        .set    qc5,5
        .set    qc6,6
        .set    qc7,7
#
# --- Begin structure -------------------------------------------------
#
        .set    qc_flags,0              # Operation flags
        .set    qc_stat,qc_flags+1      # Queue status bits
        .set    qc_nent,qc_stat+1       # Number of queued entries in all QBs
        .set    qc_pcb0,qc_nent+2       # PCB of q0 handler task
        .set    qc_pcb1,qc_pcb0+4       # PCB of q1 handler task
        .set    qc_pcb2,qc_pcb1+4       # PCB of q2 handler task
        .set    qc_pcb3,qc_pcb2+4       # PCB of q3 handler task
        .set    qc_pcb4,qc_pcb3+4       # PCB of q4 handler task
        .set    qc_pcb5,qc_pcb4+4       # PCB of q5 handler task
        .set    qc_pcb6,qc_pcb5+4       # PCB of q6 handler task
        .set    qc_pcb7,qc_pcb6+4       # PCB of q7 handler task
        .set    qc_qb0,qc_pcb7+4        # first QB
        .set    qc_qb1,qc_qb0+4         # second QB
        .set    qc_qb2,qc_qb1+4         # third QB
        .set    qc_qb3,qc_qb2+4         # fourth QB
        .set    qc_qb4,qc_qb3+4         # fifth QB
        .set    qc_qb5,qc_qb4+4         # sixth QB
        .set    qc_qb6,qc_qb5+4         # seventh QB
        .set    qc_qb7,qc_qb6+4         # eighth QB
#
# --- End structure ---------------------------------------------------
#
        .set    qcsiz,qc_qb7+4          # sizeof QCB
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
