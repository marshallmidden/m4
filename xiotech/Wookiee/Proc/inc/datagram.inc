# $Id: datagram.inc 159300 2012-06-16 04:47:51Z m4 $
#
#*****************************************************************************
#
#  NAME: datagram.inc  (Request/response datagram message definitions)
#
#  DESCRIPTION:
#
#       This file defines the equates and data structures used to
#       send/receive request/response datagram messages.
#
#  Copyright (c) 2000-2007 - Xiotech Corporation.   All rights reserved.
#
#*****************************************************************************
#
#*****************************************************************************
#                             General  Equates                               *
#*****************************************************************************
#
# --- Server processor code (dgrq_srvcpu) definitions ------------------------
#
        .set    dg_cpu_interface,0      # Interface processor
        .set    dg_cpu_mmc,0xfe         # MMC processor
        .set    dg_cpu_main,0xff        # main MAGNITUDE processor
#
# --- Path # code (dgrq_path) definitions ------------------------------------
#
        .set    dg_path_any,0xff        # any available path code
#
# --- Request completion status (dgrs_status) definitions --------------------
#
        .set    dg_st_ok,0              # successful
        .set    dg_st_sdsp,1            # source datagram service provider
                                        #  level error
        .set    dg_st_slld,2            # source LLD level error
        .set    dg_st_dlld,3            # destination LLD level error
        .set    dg_st_ddsp,4            # destination datagram service
                                        #  provider level error
        .set    dg_st_srvr,5            # destination server level error
        .set    dg_st_reroute,0x10      # re-route datagram to specified
                                        #  controller error
#
# --- Source DSP level error code #1 (dgrs_ec1) definitions ------------------
#
        .set    dgec1_sdsp_nopath,1     # no path to destination node
#
# --- Source LLD level error code #1 (dgrs_ec1) definitions ------------------
#       (Also look in irp.inc for more Error Codes returned by LLD (really ISP))
#
        .set    dgec1_slld_offl,1       # interface offline
        .set    dgec1_slld_nopath,2     # no path to destination
        .set    dgec1_slld_full,3       # request set full
        .set    dgec1_slld_PE,4         # parameter error
        .set    dgec1_slld_ioerr,5      # I/O error occurred
        .set    dgec1_slld_crc,6        # response header CRC error
        .set    dgec1_slld_scsierr,7    # SCSI error status received on
                                        #  request message
        .set    dgec1_slld_noimt,8      # no IMT for this LTMT
#
# --- Source LLD level error code #2 (dgrs_ec2) definitions ------------------
#
# --- dgec1_slld_invparm secondary error codes
        .set    slld_PE_abt,1           # srp aborted completion
        .set    slld_PE_reqlen,2        # request length = zero
        .set    slld_PE_norwSGL,3       # no response len w SGL
        .set    slld_PE_rwoSGL,4        # response len w/o SGL
        .set    slld_PE_lldid,5         # no link level driver ID
        .set    slld_PE_dlmid,6         # no data link manager ID
        .set    slld_PE_baddlmid,7      # DLM ID not correct
        .set    slld_PE_targetnotmag,8  # target not a magnitude
        .set    slld_PE_offline,9       # fibre offline
        .set    slld_PE_nosess,10       # no sessions open
        .set    slld_PE_noxchgid,11     # no exchange ID
        .set    slld_PE_notmag,12       # not a magnitude
#
# --- Destination LLD level error code #1 (dgrs_ec1) definitions -------------
#
        .set    dgec1_dlld_crc,1        # request header crc error
        .set    dgec1_dlld_invhdr,2     # invalid header parameter
        .set    dgec1_dlld_badsn,3      # wrong dest. serial # in header
        .set    dgec1_dlld_invfc,4      # Invalid request function code
        .set    dgec1_dlld_novdisk,5    # no virtual disk
#
# --- Destination DSP level error code #1 (dgrs_ec1) definitions -------------
#
        .set    dgec1_ddsp_nosrvr,1     # requested server not defined
#
# --- Destination server level error code #1 (dgrs_ec1) definitions ----------
#
        .set    dgec1_srvr_invfc,1      # invalid request function code
        .set    dgec1_srvr_invparm,2    # invalid parameter
        .set    dgec1_srvr_vlconflt,3   # VLink access conflict
        .set    dgec1_srvr_srconflt,4   # server access conflict
        .set    dgec1_srvr_nosvlink,5   # vlink not established to specified
                                        #  source copy device
        .set    dgec1_srvr_nodvlink,6   # vlink not established to specified
                                        #  dest. copy device
        .set    dgec1_srvr_inuse,7      # specified copy device in use
        .set    dgec1_srvr_nocopy,8     # specified copy operation not
                                        #  defined
        .set    dgec1_srvr_dirty,9      # dirty region/segment map table
        .set    dgec1_srvr_rmact,10     # region map state still active
        .set    dgec1_srvr_insres,11    # insufficient resources
#
#*****************************************************************************
#                             End General Equates                            *
#*****************************************************************************
#
#*****************************************************************************
#                       General Message Structures                           *
#*****************************************************************************
#
# --- Begin request message header structure ---------------------------------
#
        .set    dgrq_srvcpu,0           # server processor code         <b>
        .set    dgrq_hdrlen,1           # request header length         <b>
        .set    dgrq_seq,2              # message sequence #            <s>
        .set    dgrq_fc,4               # request function code         <b>
        .set    dgrq_path,5             # path #                        <b>
#
        .set    dgrq_g0,6               # general purpose reg. #0       <b>
        .set    dgrq_g1,7               # general purpose reg. #1       <b>
#
        .set    dgrq_srcsn,8            # source serial #               <w>
        .set    dgrq_dstsn,12           # destination serial #          <w>
#                                                               ******0x10****
        .set    dgrq_srvname,16         # dest. server name             <w>
        .set    dgrq_reqlen,20          # remaining message length      <w>
#
        .set    dgrq_g2,24              # general purpose reg. #2       <b>
        .set    dgrq_g3,25              # general purpose reg. #3       <b>
        .set    dgrq_g4,26              # general purpose reg. #4       <b>
        .set    dgrq_g5,27              # general purpose reg. #5       <b>
#
        .set    dgrq_crc,28             # request message header CRC    <w>
#                                                               ******0x20****
#
# --- End request message header structure -----------------------------------
#
        .set    dgrq_size,32            # size of request message header
#
#
# --- Begin response message header structure --------------------------------
#
        .set    dgrs_status,0           # request completion status     <b>
        .set    dgrs_hdrlen,1           # response header length        <b>
        .set    dgrs_seq,2              # message sequence #            <s>
        .set    dgrs_ec1,4              # error code byte #1            <b>
        .set    dgrs_ec2,5              # error code byte #2            <b>
#
        .set    dgrs_g0,6               # general purpose reg. #0       <b>
        .set    dgrs_g1,7               # general purpose reg. #1       <b>
#
        .set    dgrs_resplen,8          # remaining message length      <w>
        .set    dgrs_crc,12             # response message header CRC   <w>
#                                                               ******0x10****
#
# --- Alternate response message header fields -------------------------------
#
        .set    dgrs_csn,4              # re-route datagram error controller
                                        #  serial number
#
# --- End response message header structure ----------------------------------
#
        .set    dgrs_size,16            # size of response message header
#
#*****************************************************************************
#                   End General Message Structures                           *
#*****************************************************************************
#
#
#*****************************************************************************
#                      Datagram Servers Definitions                          *
#*****************************************************************************
#
# --- Main CPU Resident Servers ----------------------------------------------
#
#  Server Name       Module
#  -----------  --------------------------------------------------------------
#      DLM0     dlm.as - Data-link manager type 0 services
#
# --- DLM0 Server Function Code Definitions ----------------------------------
#
        .set    DLM0_fc_magdd,0         # Get MAGNITUDE Device Database
        .set    DLM0_fc_estvl,1         # Establish VLink
        .set    DLM0_fc_trmvl,2         # Terminate VLink
        .set    DLM0_fc_vdname,3        # VDisk Name Changed
        .set    DLM0_fc_magname,4       # MAGNITUDE Node Name Changed
        .set    DLM0_fc_swpvl,5         # Swap VLink Lock
        .set    DLM0_fc_vquery,6        # VDisk/VLink Query
        .set    DLM0_fc_chgsiz,7        # Change VDisk Size
        .set    DLM0_fc_vlpoll,8        # VLink Poll
        .set    DLM0_fc_vdsize,9        # VDisk size changed
        .set    DLM0_fc_master,10       # Group Master Controller Definition
        .set    DLM0_fc_async_nva,11    # Async module's NVA record
# GT2TB VLINKS -- new functions here.
        .set    DLM0_fc_chgsiz_GT2TB,12 # Change VDisk Size for GT2TB
        .set    DLM0_fc_magdd_GT2TB,13 # Get MAGNITUDE Device Database GT2TB
        .set    DLM0_fc_vdsize_GT2TB,14 # VDisk size changed GT2TB
        .set    DLM0_fc_estvl_GT2TB,15  # Establish VLink GT2TB
        .set    DLM0_fc_swpvl_GT2TB,16  # Swap VLink Lock GT2TB
        .set    DLM0_fc_vquery_GT2TB,17 # VDisk/VLink Query
#
# --- End Main CPU Resident Servers ------------------------------------------
#
# --- Interface CPU Resident Servers -----------------------------------------
#
#  Server Name       Module
#  -----------  --------------------------------------------------------------
#      LLD0     lld.as - Link Level Driver type 0 services
#      DLM1     dlmfe.as - Data Link Manager FE type 1 services
#      CAC0     wcache.as - Write Cache type 0 services
#
# --- LLD0 Server Function Code Definitions ----------------------------------
#
        .set    LLD0_fc_estvl,0         # Establish VLink
        .set    LLD0_fc_trmvl,1         # Terminate VLink
        .set    LLD0_fc_pncc,2          # Peer node configuration change
        .set    MAXLLDSC,3              # max # of lld0 server functions
#
# ---   lld server name
#
        .set    LLD0little,0x30444c4c   # LLD0 ser name in ASCII little endianse
        .set    LLD0big,0x4c4c4430      # LLD0 serr name in ASCII big endianse
#
# --- DLM1 Server Function Code Definitions ----------------------------------
#
        .set    DLM1_fc_polldgp,0       # Poll a Datagram Path
        .set    DLM1_fc_estcc,1         # Establish Comm. to a Controller
        .set    DLM1_fc_trmcc,2         # Terminate Comm. to a Controller
# Length/number determined after DLM1$hand definition.
#
#       DLM1 server name
#
        .set    DLM1name,0x314d4c44     # DLM1 server name in ASCII (little end)
#
# --- CAC0 Server Function Code Definitions ----------------------------------
#
        .set    CAC0_fc_wrtmem,0        # Write Memory
        .set    CAC0_fc_rdmem,1         # Read Memory
        .set    CAC0_fc_fldata,2        # Flush Write Cache Mirror Data
        .set    CAC0_fc_setwriteinfo,16 # Set Write Information record
        .set    CAC0_fc_clearwriteinfo,17 # Clear Write Information record
#
#       CAC0 server name
#
        .set    CAC0name,0x30434143     # CAC0 server name in ASCII (little end)
#
# --- End Interface CPU Resident Servers -------------------------------------
#
#
# --- CCB CPU Resident Servers -----------------------------------------
#
#  Server Name       Module
#  -----------  --------------------------------------------------------------
#      CCB0     ???? - CCB type 0 services
#
# --- CCB0 Server Function Code Definitions ----------------------------------
#
        .set    CCB0_fc_sndpkt,0        # Send Packet
        .set    MAXCCB0SC,1             # max # of ccb0 server functions
#
# --- End CCB CPU Resident Servers -------------------------------------
#
#*****************************************************************************
#               End DLM0 Server Function Code Definitions                    *
#*****************************************************************************
#
#*****************************************************************************
#                  DLM0 Server Message Structures                            *
#*****************************************************************************
#
# --- Get MAGNITUDE Device Database Extended Request Information Structure ---
#
        .set    DLM0_rq_magdd_cl,dgrq_g0 # Specified cluster # to get device
                                        #  database for
#
# --- End Get MAGNITUDE Device Database Extended Request Information Structure
#
# --- Get MAGNITUDE Device Database Extended Response Information Structure --
#
#       Extended Response Information Structure defined in file MAGDD.INC!
#
# --- End Get MAGNITUDE Device Database Extended Response Information Structure
#
#
# --- Establish VLink Extended Request Information Structure -----------------
#       The following extended information structure begins after the
#       request header
#
        .set    DLM0_rq_estvl_srcsn,0   # Source MAG serial #             <w>
        .set    DLM0_rq_estvl_srccl,4   # Source MAG cluster #            <b>
        .set    DLM0_rq_estvl_srcvd,5   # Source MAG VDisk #              <b>
        .set    DLM0_rq_estvl_attr,6    # VLink attributes                <b>
                                        # Bit 7 = 1=exclusive access
                                        #     6 = 1=forced access
#jt - potential conflict here. 7.09d fcal/proc have different
#     meanings for bit 6, made one of them a bit 5...
                                        #     5 = 1=persistent reserve
                                        #     4 =
                                        #     3 =
                                        #     2 =
                                        #     1 =
                                        #     0 = 0=cluster/VDisk format
                                        #         1=16-bit VDisk format
        .set    DLM0_rq_estvl_r0,7      # Reserved byte #0                <b>
        .set    DLM0_rq_estvl_name,8    # Source MAG device name         8<b>
        .set    DLM0_rq_estvl_dstsn,16  # Destination MAG serial #        <w>
        .set    DLM0_rq_estvl_dstcl,20  # Destination MAG cluster #       <b>
        .set    DLM0_rq_estvl_dstvd,21  # Destination MAG VDisk #         <b>
        .set    DLM0_rq_estvl_r1,22     # Reserved byte #1                <b>
        .set    DLM0_rq_estvl_r2,23     # Reserved byte #2                <b>
        .set    DLM0_rq_estvl_agnt,24   # Transfer agent MAG serial #     <w>
        .set    DLM0_rq_estvl_size,28   # Size of extended request message
#
# --- End Establish VLink Extended Request Information Structure -------------
#
# --- Establish VLink Extended Response Information Structure ----------------
#       The following structure begins after the response header
#
        .set    DLM0_rs_estvl_basesn,0  # Base MAG serial #               <w>
        .set    DLM0_rs_estvl_basecl,4  # Base MAG cluster #              <b>
        .set    DLM0_rs_estvl_basevd,5  # Base MAG VDisk #                <b>
        .set    DLM0_rs_estvl_secsz,6   # Sector size (bytes)             <s>
        .set    DLM0_rs_estvl_dsiz,8    # VDisk size (sectors)            <w>
        .set    DLM0_rs_estvl_name,12   # Base MAG device name           8<b>
        .set    DLM0_rs_estvl_size,20   # Size of extended response message
# NOTE: GT2TB VLINK below
        .set    DLM0_rs_estvl_dsiz_GT2TB,8  # VDisk size (sectors)         <l>
        .set    DLM0_rs_estvl_name_GT2TB,16 # Base MAG device name        8<b>
        .set    DLM0_rs_estvl_size_GT2TB,24 # Size of extended response message GT2TB
#
        .set    DLM0_rs_estvl_altid,dgrs_g0 # Alternate ID                <s>
#
# --- End Establish VLink Extended Response Information Structure ------------
#
# --- Terminate VLink  Extended Request Information Structure ----------------
#       The following structure begins after the request header
#
        .set    DLM0_rq_trmvl_srcsn,0   # Source MAG serial #             <w>
        .set    DLM0_rq_trmvl_srccl,4   # Source MAG cluster #            <b>
        .set    DLM0_rq_trmvl_srcvd,5   # Source MAG VDisk #              <b>
        .set    DLM0_rq_trmvl_r0,6      # Reserved byte #0                <b>
        .set    DLM0_rq_trmvl_r1,7      # Reserved byte #1                <b>
        .set    DLM0_rq_trmvl_dstsn,8   # Destination MAG serial #        <w>
        .set    DLM0_rq_trmvl_dstcl,12  # Destination MAG cluster #       <b>
        .set    DLM0_rq_trmvl_dstvd,13  # Destination MAG VDisk #         <b>
        .set    DLM0_rq_trmvl_r2,14     # Reserved byte #2                <b>
        .set    DLM0_rq_trmvl_r3,15     # Reserved byte #3                <b>
        .set    DLM0_rq_trmvl_agnt,16   # Transfer agent MAG serial #     <w>
        .set    DLM0_rq_trmvl_size,20   # Size of extended request message
#
# --- End Terminate VLink Extended Request Information Structure -------------
#
# --- Terminate VLink Extended Response Information Structure ----------------
#       The following structure begins after the response header
#
#       No extended response information defined!
#
# --- End Terminate VLink Extended Response Information Structure ------------
#
#
# --- VDisk Name Changed Extended Request Information Structure --------------
#       The following structure begins after the request header
#
        .set    DLM0_rq_vdchg_srcsn,0   # Source MAG serial #             <w>
        .set    DLM0_rq_vdchg_srccl,4   # Source MAG cluster #            <b>
        .set    DLM0_rq_vdchg_srcvd,5   # Source MAG VDisk #              <b>
        .set    DLM0_rq_vdchg_r0,6      # Reserved byte #0                <b>
        .set    DLM0_rq_vdchg_r1,7      # Reserved byte #1                <b>
        .set    DLM0_rq_vdchg_srcname,8 # Source MAG device name         8<b>
        .set    DLM0_rq_vdchg_basesn,16 # Base MAG serial #               <w>
        .set    DLM0_rq_vdchg_basecl,20 # Base MAG cluster #              <b>
        .set    DLM0_rq_vdchg_basevd,21 # Base MAG VDisk #                <b>
        .set    DLM0_rq_vdchg_r2,22     # Reserved byte #2                <b>
        .set    DLM0_rq_vdchg_type,23   # Name change type code           <b>
                                        # 00 = all reference types
                                        # 01 = VLink-to-VDisk type
                                        # 02 = VDisk-to-VLink type
        .set    DLM0_rq_vdchg_basename,24 # Base MAG device name         8<b>
        .set    DLM0_rq_vdchg_size,32   # Size of extended request message
#
# --- End VDisk Name Changed Extended Request Information Structure ----------
#
# --- VDisk Name Changed Extended Response Information Structure -------------
#       The following structure begins after the response header
#
#       No extended response information defined!
#
# --- End VDisk Name Changed Extended Response Information Structure ---------
#
#async
        .set    MAXAPOOLEXP,5
                                                # Apool NV element structure - start
        .set    DLM0_rq_apool_nve_vid,0                  # VID                 <s>
        .set    DLM0_rq_apool_nve_status,2               # status              <s>
        .set    DLM0_rq_apool_nve_apool_id,4             # Apool ID            <s>
        .set    DLM0_rq_apool_nve_jump_to,6              # Jump to element id  <s>
        .set    DLM0_rq_apool_nve_length,8               # Size                <dw>
        .set    DLM0_rq_apool_nve_sda,16                 # SDA                 <dw>
        .set    DLM0_rq_apool_nve_head_pointer,24        # Head Pointer        <dw>
        .set    DLM0_rq_apool_nve_tail_pointer,32        # Tail Pointer        <dw>
        .set    DLM0_rq_apool_nve_attr,40                # Attribute           <w>
        .set    DLM0_rq_apool_nve_attr_offset,44         # Elem jump offst     <dw>
                                                # Apool NV element structure - end
        .set    DLM0_rq_apool_nve_element_size,52
#
# --- Start Async NVA Extended Request info structure  -----------------
#
        .set    DLM0_rq_apool_nv_srcsn,0         # Source MAG serial number    <w>
        .set    DLM0_rq_apool_nv_destsn,4        # Source MAG serial number    <w>
        .set    DLM0_rq_apool_nv_update_type,8   # Async NV update type
                                                 # Apool NV header structure - start
        .set    DLM0_rq_apool_nvh,10
        .set    DLM0_rq_apool_nvh_apool_p7id,DLM0_rq_apool_nvh  # Apool signature <w>
        .set    DLM0_rq_apool_nvh_seq_count,14           # Chronology counter  <dw>
        .set    DLM0_rq_apool_nvh_status,22              # status               <s>
        .set    DLM0_rq_apool_nvh_apool_id,24            # Apool ID            <s>
        .set    DLM0_rq_apool_nvh_cur_head_element,26    # Global head element <s>
        .set    DLM0_rq_apool_nvh_cur_tail_element,28    # Global tail element <s>
        .set    DLM0_rq_apool_nvh_apool_size,30          # Apool size          <dw>
        .set    DLM0_rq_apool_nvh_time_threshold,38      # Apool time threshold  <w>
        .set    DLM0_rq_apool_nvh_mb_threshold,42        # Apool MB threshold  <w>
        .set    DLM0_rq_apool_nvh_last_seq_cnt,46        # Last sequence count <dw>
        .set    DLM0_rq_apool_nvh_alink_count,54         # Alink count  <s>
        .set    DLM0_rq_apool_nvh_element_count,56       # Element count       <s>
        .set    DLM0_rq_apool_nvh_element_entry_size,58  # Element entry size  <s>
        .set    DLM0_rq_apool_nvh_version,60             # Version number      <b>
                                                         # Reserved            <3b>
                                                # Apool NV header structure - end
        .set    DLM0_rq_apool_nvetbl,64         # Apool NV elements table - start
        .set    DLM0_rq_apool_nv_size,DLM0_rq_apool_nvetbl+(MAXAPOOLEXP*DLM0_rq_apool_nve_element_size) # Size (52 + 5*42)of extended request message
#
# ---  MAGNITUDE Node Name Changed Extended Request Information Structure ----
#       The following structure begins after the request header
#
        .set    DLM0_rq_nnchg_sn,0      # MAG serial #                    <w>
        .set    DLM0_rq_nnchg_name,4    # MAG node name                  8<b>
        .set    DLM0_rq_nnchg_size,12   # Size of extended request message
#
# --- End MAGNITUDE Node Name Changed Extended Request Information Structure -
#
# --- MAGNITUDE Node Name Changed Extended Response Information Structure ----
#       The following structure begins after the response header
#
#       No extended response information defined!
#
# --- End MAGNITUDE Node Name Changed Extended Response Information Structure
#
#
# --- Swap VLink Lock Extended Request Information Structure -----------------
#       The following structure begins after the request header
#
        .set    DLM0_rq_swpvl_srcsn,0   # Source MAG serial #             <w>
        .set    DLM0_rq_swpvl_srccl,4   # Source MAG cluster #            <b>
        .set    DLM0_rq_swpvl_srcvd,5   # Source MAG VDisk #              <b>
        .set    DLM0_rq_swpvl_attr,6    # VLink attributes                <b>
                                        # Bit 7 = 1=exclusive access
                                        #     6 = 1=forced access
                                        #     5 =
                                        #     4 =
                                        #     3 =
                                        #     2 =
                                        #     1 =
                                        #     0 = 0=cluster/VDisk format
                                        #         1=16-bit VDisk format
        .set    DLM0_rq_swpvl_r0,7      # Reserved byte #0                <b>
        .set    DLM0_rq_swpvl_name,8    # Source MAG device name         8<b>
        .set    DLM0_rq_swpvl_dstsn,16  # Destination MAG serial #        <w>
        .set    DLM0_rq_swpvl_dstcl,20  # Destination MAG cluster #       <b>
        .set    DLM0_rq_swpvl_dstvd,21  # Destination MAG VDisk #         <b>
        .set    DLM0_rq_swpvl_r1,22     # Reserved byte #1                <b>
        .set    DLM0_rq_swpvl_r2,23     # Reserved byte #2                <b>
        .set    DLM0_rq_swpvl_agnt,24   # Transfer agent MAG serial #     <w>
        .set    DLM0_rq_swpvl_orgsn,28  # Original MAG serial #           <w>
        .set    DLM0_rq_swpvl_orgcl,32  # Original MAG cluster #          <b>
        .set    DLM0_rq_swpvl_orgvd,33  # Original MAG VDisk #            <b>
        .set    DLM0_rq_swpvl_r3,34     # Reserved byte #3                <b>
        .set    DLM0_rq_swpvl_r4,35     # Reserved byte #4                <b>
        .set    DLM0_rq_swpvl_size,36   # Size of extended request message
#
# --- End Swap VLink Lock Extended Request Information Structure -------------
#
# --- Swap VLink Lock Extended Response Information Structure ----------------
#       The following structure begins after the response header
#
        .set    DLM0_rs_swpvl_basesn,0  # Base MAG serial #               <w>
        .set    DLM0_rs_swpvl_basecl,4  # Base MAG cluster #              <b>
        .set    DLM0_rs_swpvl_basevd,5  # Base MAG VDisk #                <b>
        .set    DLM0_rs_swpvl_secsz,6   # Sector size (bytes)             <s>
        .set    DLM0_rs_swpvl_dsiz,8    # VDisk size (sectors)            <w>
        .set    DLM0_rs_swpvl_name,12   # Base MAG device name           8<b>
        .set    DLM0_rs_swpvl_size,20   # Size of extended response message
#
        .set    DLM0_rs_swpvl_altid,dgrs_g0 # Alternate ID                <s>
#
# GT2TB version follows
#
#--     .set    DLM0_rs_swpvl_basesn,0  # Base MAG serial #         <w>
#--     .set    DLM0_rs_swpvl_basecl,4  # Base MAG cluster #        <b>
#--     .set    DLM0_rs_swpvl_basevd,5  # Base MAG VDisk #          <b>
#--     .set    DLM0_rs_swpvl_secsz,6   # Sector size (bytes)       <s>
        .set    DLM0_rs_swpvl_dsiz_GT2TB,8    # VDisk size (sectors)      <l>
        .set    DLM0_rs_swpvl_name_GT2TB,16   # Base MAG device name      8<b>
        .set    DLM0_rs_swpvl_size_GT2TB,24   # Size of extended response message
#
        .set    DLM0_rs_swpvl_altid,dgrs_g0 # Alternate ID                <s>
# --- End Swap VLink Lock Extended Response Information Structure ------------
#
#
# --- VDisk/VLink Query Extended Request Information Structure ---------------
#       The following structure begins after the request header
#
        .set    DLM0_rq_vqury_sn,0      # MAG serial #                    <w>
        .set    DLM0_rq_vqury_cl,4      # MAG cluster #                   <b>
        .set    DLM0_rq_vqury_vd,5      # MAG VDisk #                     <b>
        .set    DLM0_rq_vqury_r0,6      # Reserved byte #0                <b>
        .set    DLM0_rq_vqury_r1,7      # Reserved byte #1                <b>
        .set    DLM0_rq_vqury_size,8    # Size of extended request message
#
# --- End VDisk/VLink Query Extended Request Information Structure -----------
#
# --- VDisk/VLink Query Extended Response Information Structure --------------
#       The following structure begins after the response header
#
        .set    DLM0_rs_vqury_type,0    # VDisk/VLink type code           <b>
#
        .set    vqury_ty_undef,0        # 00 = undefined
        .set    vqury_ty_vdisk,1        # 01 = VDisk
        .set    vqury_ty_vlink,2        # 02 = VLink
        .set    vqury_ty_busy,3         # 03 = VDisk/VLink busy
#
        .set    DLM0_rs_vqury_lock,1    # VDisk/VLink lock flag (T/F)     <b>
        .set    DLM0_rs_vqury_attr,2    # Attributes byte                 <s>
        .set    DLM0_rs_vqury_secsz,4   # Sector size (bytes)             <s>
        .set    DLM0_rs_vqury_r1,6      # Reserved byte #1                <b>
        .set    DLM0_rs_vqury_r2,7      # Reserved byte #2                <b>
        .set    DLM0_rs_vqury_dsiz,8    # VDisk size (sectors)            <w>   GT2TB version required
        .set    DLM0_rs_vqury_name,12   # Base MAG device name           8<b>
# --- The following fields are used for VLinks only!
        .set    DLM0_rs_vqury_magsn,20  # MAG serial #                    <w>
        .set    DLM0_rs_vqury_magcl,24  # MAG cluster #                   <b>
        .set    DLM0_rs_vqury_magvd,25  # MAG VDisk #                     <b>
        .set    DLM0_rs_vqury_basecl,26 # Base MAG cluster #              <b>
        .set    DLM0_rs_vqury_basevd,27 # Base MAG VDisk #                <b>
        .set    DLM0_rs_vqury_basesn,28 # Base MAG serial #               <w>
        .set    DLM0_rs_vqury_basename,32 # Base MAG device name         8<b>
# --- The following fields are used if the device is locked!
        .set    DLM0_rs_vqury_vlsn,40   # Source MAG serial #             <w>
        .set    DLM0_rs_vqury_vlcl,44   # Source MAG cluster #            <b>
        .set    DLM0_rs_vqury_vlvd,45   # Source MAG VDisk #              <b>
        .set    DLM0_rs_vqury_vlattr,46 # Attributes                      <b>
                                        # Bit 7 = 1=exclusive access
                                        #     6 = 1=forced access
                                        #     5 =
                                        #     4 =
                                        #     3 =
                                        #     2 =
                                        #     1 =
                                        #     0 = 0=cluster/VDisk format
                                        #         1=16-bit VDisk format
        .set    DLM0_rs_vqury_r3,47     # Reserved byte #3                <b>
        .set    DLM0_rs_vqury_vlname,48 # Source MAG device name         8<b>
        .set    DLM0_rs_vqury_vlagnt,56 # Transfer agent MAG serial #     <w>
        .set    DLM0_rs_vqury_size,60   # Size of extended response message
# GT2TB version below
        .set    DLM0_rs_vqury_dsiz_GT2TB,8    # VDisk size (sectors)      <l>
        .set    DLM0_rs_vqury_name_GT2TB,16   # Base MAG device name     8<b>
# --- The following fields are used for VLinks only!
        .set    DLM0_rs_vqury_magsn_GT2TB,24  # MAG serial #              <w>
        .set    DLM0_rs_vqury_magcl_GT2TB,28  # MAG cluster #             <b>
        .set    DLM0_rs_vqury_magvd_GT2TB,29  # MAG VDisk #               <b>
        .set    DLM0_rs_vqury_basecl_GT2TB,30 # Base MAG cluster #        <b>
        .set    DLM0_rs_vqury_basevd_GT2TB,31 # Base MAG VDisk #          <b>
        .set    DLM0_rs_vqury_basesn_GT2TB,32 # Base MAG serial #         <w>
        .set    DLM0_rs_vqury_basename_GT2TB,36 # Base MAG device name   8<b>
# --- The following fields are used if the device is locked!
        .set    DLM0_rs_vqury_vlsn_GT2TB,44   # Source MAG serial #       <w>
        .set    DLM0_rs_vqury_vlcl_GT2TB,48   # Source MAG cluster #      <b>
        .set    DLM0_rs_vqury_vlvd_GT2TB,49   # Source MAG VDisk #        <b>
        .set    DLM0_rs_vqury_vlattr_GT2TB,50 # Attributes                <b>
                                              # Bit 7 = 1=exclusive access
                                              #     6 = 1=forced access
                                              #     5 =
                                              #     4 =
                                              #     3 =
                                              #     2 =
                                              #     1 =
                                              #     0 = 0=cluster/VDisk format
                                              #         1=16-bit VDisk format
        .set    DLM0_rs_vqury_r3_GT2TB,51     # Reserved byte #3          <b>
        .set    DLM0_rs_vqury_vlname_GT2TB,52 # Source MAG device name   8<b>
        .set    DLM0_rs_vqury_vlagnt_GT2TB,60 # Transfer agent MAG serial # <w>
        .set    DLM0_rs_vqury_size_GT2TB,64   # Size of extended response message
#
# --- End VDisk/VLink Query Extended Response Information Structure ----------
#
# --- Change VDisk Size Extended Request Information Structure ---------------
#       The following structure begins after the request header
#
        .set    DLM0_rq_chgsz_srcsn,0   # Source MAG serial #             <w>
        .set    DLM0_rq_chgsz_srccl,4   # Source MAG cluster #            <b>
        .set    DLM0_rq_chgsz_srcvd,5   # Source MAG VDisk #              <b>
        .set    DLM0_rq_chgsz_r0,6      # Reserved byte #0                <b>
        .set    DLM0_rq_chgsz_r1,7      # Reserved byte #1                <b>
        .set    DLM0_rq_chgsz_dstsn,8   # Destination MAG serial #        <w>
        .set    DLM0_rq_chgsz_dstcl,12  # Destination MAG cluster #       <b>
        .set    DLM0_rq_chgsz_dstvd,13  # Destination MAG VDisk #         <b>
        .set    DLM0_rq_chgsz_r2,14     # Reserved byte #2                <b>
        .set    DLM0_rq_chgsz_r3,15     # Reserved byte #3                <b>
        .set    DLM0_rq_chgsz_dsiz,16   # New VDisk size                  <w>
        .set    DLM0_rq_chgsz_size,20   # Size of extended request message
# NOTE: GT2TB same as above, but last entry is 8 bytes instead of 4.
        .set    DLM0_rq_chgsz_dsiz_GT2TB,16 # OR for GT2TB New VDisk size <l>
        .set    DLM0_rq_chgsz_size_GT2TB,24 # Size of GT2TB extended request message
#
# --- End Change VDisk Size Extended Request Information Structure -----------
#
# --- Change VDisk Size Extended Response Information Structure --------------
#       The following structure begins after the response header
#
#       No extended response information defined!
#
# --- End Change VDisk Size Extended Response Information Structure ----------
#
# --- VLink Poll Extended Request Information Structure ----------------------
#       The following structure begins after the request header
#
        .set    DLM0_rq_vlpol_srcsn,0   # Source MAG serial #             <w>
        .set    DLM0_rq_vlpol_srccl,4   # Source MAG cluster #            <b>
        .set    DLM0_rq_vlpol_srcvd,5   # Source MAG VDisk #              <b>
        .set    DLM0_rq_vlpol_r0,6      # Reserved byte #0                <b>
        .set    DLM0_rq_vlpol_r1,7      # Reserved byte #1                <b>
        .set    DLM0_rq_vlpol_dstsn,8   # Destination MAG serial #        <w>
        .set    DLM0_rq_vlpol_dstcl,12  # Destination MAG cluster #       <b>
        .set    DLM0_rq_vlpol_dstvd,13  # Destination MAG VDisk #         <b>
        .set    DLM0_rq_vlpol_r2,14     # Reserved byte #2                <b>
        .set    DLM0_rq_vlpol_r3,15     # Reserved byte #3                <b>
        .set    DLM0_rq_vlpol_size,16   # Size of extended request message
#
# --- End VLink Poll Extended Request Information Structure ------------------
#
# --- VLink Poll Extended Response Information Structure ---------------------
#
        .set    DLM0_rs_vlpol_stat,dgrs_ec1 # VLink status code           <b>
                                        # 00 = VLink locked by requestor
                                        # 01 = VDisk does not exist
                                        # 02 = VDisk locked by other VLink
                                        # 03 = VDisk exists, not locked
#
        .set    DLM0_rs_vlpol_altid,dgrs_g0 # Alternate ID                <s>
#
#       The following structure begins after the response header
#
#       No extended response information defined!
#
# --- End VLink Poll Extended Response Information Structure -----------------
#
#
# --- VDisk Size Changed Extended Request Information Structure --------------
#       The following structure begins after the request header
#
        .set    DLM0_rq_vdsize_srcsn,0  # Source MAG serial #             <w>
        .set    DLM0_rq_vdsize_srccl,4  # Source MAG cluster #            <b>
        .set    DLM0_rq_vdsize_srcvd,5  # Source MAG VDisk #              <b>
        .set    DLM0_rq_vdsize_r0,6     # Reserved byte #0                <b>
        .set    DLM0_rq_vdsize_r1,7     # Reserved byte #1                <b>
        .set    DLM0_rq_vdsize_dstsn,8  # Destination MAG serial #        <w>
        .set    DLM0_rq_vdsize_dstcl,12 # Destination MAG cluster #       <b>
        .set    DLM0_rq_vdsize_dstvd,13 # Destination MAG VDisk #         <b>
        .set    DLM0_rq_vdsize_r2,14    # Reserved byte #2                <b>
        .set    DLM0_rq_vdsize_r3,15    # Reserved byte #3                <b>
        .set    DLM0_rq_vdsize_devcap,16 # New VDisk size (in sectors)    <w>
        .set    DLM0_rq_vdsize_size,20  # Size of extended request message
# GT2TB versions below.
        .set    DLM0_rq_vdsize_devcap_GT2TB,16 # New VDisk size (in sectors)<l>
        .set    DLM0_rq_vdsize_size_GT2TB,24   # Size of extended request message GT2TB
#
# --- End VDisk Size Changed Extended Request Information Structure ----------
#
# --- VDisk Size Changed Extended Response Information Structure -------------
#
        .set    DLM0_rs_vdsize_stat,dgrs_ec1 # Change acceptance code     <b>
                                        #       if dgrs_status = dg_st_ok
                                        # 00 = change accepted by recipient
                                        # 01 = rejected-assoc. VLink the
                                        #      source of a copy op.
                                        # 02 = rejected-assoc. VLink the
                                        #      destination of a copy op.
                                        # 03 = rejected-no VLink assoc.
                                        #      with the specified VDisk
#
#       The following structure begins after the response header
#
#       No extended response information defined!
#
# --- End VDisk Size Changed Extended Response Information Structure ---------
#
# --- Group Master Controller Definition Extended Request --------------------
# -----------------Information Structure -------------------------------------
#
#       The following structure begins after the request header
#
        .set    DLM0_rq_master_gpsn,0   # Group serial #                  <w>
        .set    DLM0_rq_master_mastsn,4 # Master controller serial #      <w>
        .set    DLM0_rq_master_size,8   # Size of extended request message
#
# --- End Group Master Controller Definition Extended Request ----------------
# --------------------Information Structure ----------------------------------
#
# --- Group Master Controller Definition Extended Response Information -------
# ---------------------------- Structure -------------------------------------
#
#       The following structure begins after the response header
#
#       No extended response information defined!
#
# --- End Group Master Controller Definition Extended Response ---------------
# --------------------Information Structure ----------------------------------
#
#*****************************************************************************
#               End DLM0 Server Message Structures                           *
#*****************************************************************************
#*****************************************************************************
#               CMsp Server Function Code Definitions                        *
#*****************************************************************************
#
# --- CMsp Server Function Code Definitions ----------------------------------
#
        .set    CMsp_fc_op_new,0        # Define new copy operation
        .set    CMsp_fc_op_state,1      # Establish copy operation state
        .set    CMsp_fc_op_term,2       # Terminate copy operation
        .set    CMsp_fc_op_check,3      # Check copy operation state/status
        .set    CMsp_fc_rm_start,4      # Maintain/start region map table
        .set    CMsp_fc_rm_susp,5       # Suspend region map table
        .set    CMsp_fc_rm_term,6       # Terminate region map table
        .set    CMsp_fc_rm_read,7       # Read region/segment map table
        .set    CMsp_fc_rm_clear,8      # Clear region/segment map table
        .set    CMsp_fc_rm_check,9      # Check region map table state/status
        .set    CMsp_fc_op_susp,10      # Suspend copy operation
        .set    CMsp_fc_op_resume,11    # Resume copy operation
        .set    CMsp_fc_op_dmove,12     # Copy device moved
#
#*****************************************************************************
#               End CMsp Server Function Code Definitions                    *
#*****************************************************************************
#
#
#*****************************************************************************
#                  CMsp Server Message Data Structures                       *
#*****************************************************************************
#
# --- Define New Copy Operation Request Data Structure -----------------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_opnew_rid,0     # Copy reg. ID                    <w>
        .set    CMsp_rq_opnew_rcsn,4    # Copy MAG serial number          <w>
        .set    CMsp_rq_opnew_rcscl,8   # Copy MAG source cluster number  <b>
        .set    CMsp_rq_opnew_rcsvd,9   # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_opnew_rcdcl,10  # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_opnew_rcdvd,11  # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_opnew_rssn,12   # Source MAG serial number        <w>
        .set    CMsp_rq_opnew_rdsn,16   # Dest. MAG serial number         <w>
        .set    CMsp_rq_opnew_rscl,20   # Source MAG cluster number       <b>
        .set    CMsp_rq_opnew_rsvd,21   # Source MAG VDisk number         <b>
        .set    CMsp_rq_opnew_rdcl,22   # Dest. MAG cluster number        <b>
        .set    CMsp_rq_opnew_rdvd,23   # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_opnew_cstate,24 # Starting copy state code        <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_opnew_crstate,25 # Starting copy reg. state       <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_opnew_mstate,26 # My current map (cor_mstate)     <b>
                                        #  state code
        .set    CMsp_rq_opnew_gid,27    # User defined group ID           <b>
        .set    CMsp_rq_opnew_label,28  # User defined copy operation   16<b>
                                        #  label (cor_label)
        .set    CMsp_rq_opnew_totalsegs,44 # total segment count          <w>
                                        #  (cor_totalsegs)
        .set    CMsp_rq_opnew_size,48   # Size of extended request message
#
# --- End Define New Copy Operation Request Data Structure -------------------
#
# --- Define New Copy Operation Response Data Structure ----------------------
#       The following data structure begins after the response header
#
#       No extended response data defined!
#
# --- End Define New Copy Operation Response Data Structure ------------------
#
#
# --- Establish Copy Operation State Request Data Structure ------------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_opst_rid,0      # Copy reg. ID                    <w>
        .set    CMsp_rq_opst_rcsn,4     # Copy MAG serial number          <w>
        .set    CMsp_rq_opst_rcscl,8    # Copy MAG source cluster number  <b>
        .set    CMsp_rq_opst_rcsvd,9    # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_opst_rcdcl,10   # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_opst_rcdvd,11   # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_opst_rssn,12    # Source MAG serial number        <w>
        .set    CMsp_rq_opst_rdsn,16    # Dest. MAG serial number         <w>
        .set    CMsp_rq_opst_rscl,20    # Source MAG cluster number       <b>
        .set    CMsp_rq_opst_rsvd,21    # Source MAG VDisk number         <b>
        .set    CMsp_rq_opst_rdcl,22    # Dest. MAG cluster number        <b>
        .set    CMsp_rq_opst_rdvd,23    # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_opst_cstate,24  # New copy state code             <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_opst_crstate,25 # New copy reg. state             <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_opst_mstate,26  # My current map (cor_mstate)     <b>
                                        #  state code
        .set    CMsp_rq_opst_gid,27     # User defined group ID           <b>
        .set    CMsp_rq_opst_label,28   # User defined copy operation   16<b>
                                        #  label (cor_label)
        .set    CMsp_rq_opst_size,44    # Size of extended request message
#
# --- End Establish Copy Operation State Request Data Structure --------------
#
# --- Establish Copy Operation State Response Data Structure -----------------
#       The following data structure begins after the response header
#
#       No extended response data defined!
#
# --- End Establish Copy Operation State Response Data Structure -------------
#
# --- Terminate Copy Operation Request Data Structure ------------------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_optrm_rid,0     # Copy reg. ID                    <w>
        .set    CMsp_rq_optrm_rcsn,4    # Copy MAG serial number          <w>
        .set    CMsp_rq_optrm_rcscl,8   # Copy MAG source cluster number  <b>
        .set    CMsp_rq_optrm_rcsvd,9   # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_optrm_rcdcl,10  # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_optrm_rcdvd,11  # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_optrm_rssn,12   # Source MAG serial number        <w>
        .set    CMsp_rq_optrm_rdsn,16   # Dest. MAG serial number         <w>
        .set    CMsp_rq_optrm_rscl,20   # Source MAG cluster number       <b>
        .set    CMsp_rq_optrm_rsvd,21   # Source MAG VDisk number         <b>
        .set    CMsp_rq_optrm_rdcl,22   # Dest. MAG cluster number        <b>
        .set    CMsp_rq_optrm_rdvd,23   # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_optrm_size,24   # Size of extended request message
#
# --- End Terminate Copy Operation Request Data Structure --------------------
#
# --- Terminate Copy Operation Response Data Structure -----------------------
#       The following data structure begins after the response header
#
#       No extended response data defined!
#
# --- End Terminate Copy Operation Response Data Structure -------------------
#
#
# --- Check Copy Operation State/Status Request Data Structure ---------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_opchk_rid,0     # Copy reg. ID                    <w>
        .set    CMsp_rq_opchk_rcsn,4    # Copy MAG serial number          <w>
        .set    CMsp_rq_opchk_rcscl,8   # Copy MAG source cluster number  <b>
        .set    CMsp_rq_opchk_rcsvd,9   # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_opchk_rcdcl,10  # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_opchk_rcdvd,11  # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_opchk_rssn,12   # Source MAG serial number        <w>
        .set    CMsp_rq_opchk_rdsn,16   # Dest. MAG serial number         <w>
        .set    CMsp_rq_opchk_rscl,20   # Source MAG cluster number       <b>
        .set    CMsp_rq_opchk_rsvd,21   # Source MAG VDisk number         <b>
        .set    CMsp_rq_opchk_rdcl,22   # Dest. MAG cluster number        <b>
        .set    CMsp_rq_opchk_rdvd,23   # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_opchk_cstate,24 # Expected copy state code        <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_opchk_crstate,25 # Expected copy reg. state       <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_opchk_mstate,26 # My current map (cor_mstate)     <b>
                                        #  state code
        .set    CMsp_rq_opchk_gid,27    # User defined group ID           <b>
        .set    CMsp_rq_opchk_label,28  # User defined copy operation   16<b>
                                        #  label (cor_label)
        .set    CMsp_rq_opchk_size,44   # Size of extended request message
#
# --- End Check Copy Operation State/Status Request Data Structure -----------
#
# --- Check Copy Operation State/Status Response Data Structure --------------
#       The following data structure begins after the response header
#
        .set    CMsp_rs_opchk_rid,0     # Copy reg. ID                    <w>
        .set    CMsp_rs_opchk_rcsn,4    # Copy MAG serial number          <w>
        .set    CMsp_rs_opchk_rcscl,8   # Copy MAG source cluster number  <b>
        .set    CMsp_rs_opchk_rcsvd,9   # Copy MAG source VDisk number    <b>
        .set    CMsp_rs_opchk_rcdcl,10  # Copy MAG dest. cluster number   <b>
        .set    CMsp_rs_opchk_rcdvd,11  # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rs_opchk_rssn,12   # Source MAG serial number        <w>
        .set    CMsp_rs_opchk_rdsn,16   # Dest. MAG serial number         <w>
        .set    CMsp_rs_opchk_rscl,20   # Source MAG cluster number       <b>
        .set    CMsp_rs_opchk_rsvd,21   # Source MAG VDisk number         <b>
        .set    CMsp_rs_opchk_rdcl,22   # Dest. MAG cluster number        <b>
        .set    CMsp_rs_opchk_rdvd,23   # Dest. MAG VDisk number          <b>
        .set    CMsp_rs_opchk_cstate,24 # Current copy state code         <b>
                                        #  (cor_cstate)
        .set    CMsp_rs_opchk_crstate,25 # Current copy reg. state        <b>
                                        #  (cor_crstate) code
        .set    CMsp_rs_opchk_mstate,26 # My current map (cor_mstate)     <b>
                                        #  state code
        .set    CMsp_rs_opchk_size,28   # Size of extended request message
#
# --- End Check Copy Operation State/Status Response Data Structure ----------
#
#
# ---  Maintain/Start Region Map Table Request Data Structure ----------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_rmst_rid,0      # Copy reg. ID                    <w>
        .set    CMsp_rq_rmst_rcsn,4     # Copy MAG serial number          <w>
        .set    CMsp_rq_rmst_rcscl,8    # Copy MAG source cluster number  <b>
        .set    CMsp_rq_rmst_rcsvd,9    # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_rmst_rcdcl,10   # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_rmst_rcdvd,11   # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_rmst_rssn,12    # Source MAG serial number        <w>
        .set    CMsp_rq_rmst_rdsn,16    # Dest. MAG serial number         <w>
        .set    CMsp_rq_rmst_rscl,20    # Source MAG cluster number       <b>
        .set    CMsp_rq_rmst_rsvd,21    # Source MAG VDisk number         <b>
        .set    CMsp_rq_rmst_rdcl,22    # Dest. MAG cluster number        <b>
        .set    CMsp_rq_rmst_rdvd,23    # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_rmst_cstate,24  # Current copy state code         <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_rmst_crstate,25 # Current copy reg. state         <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_rmst_mstate,26  # My current map (cor_mstate)     <b>
                                        #  state code
        .set    CMsp_rq_rmst_size,28    # Size of extended request message
#
# --- End Maintain/Start Region Map Table Request Data Structure -------------
#
# --- Maintain/Start Region Map Table Response Data Structure ----------------
#       The following data structure begins after the response header
#
#       No extended response data defined!
#
# --- End Maintain/Start Region Map Table Response Data Structure ------------
#
#
# --- Suspend Region Map Table Request Data Structure ------------------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_rmsusp_rid,0    # Copy reg. ID                    <w>
        .set    CMsp_rq_rmsusp_rcsn,4   # Copy MAG serial number          <w>
        .set    CMsp_rq_rmsusp_rcscl,8  # Copy MAG source cluster number  <b>
        .set    CMsp_rq_rmsusp_rcsvd,9  # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_rmsusp_rcdcl,10 # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_rmsusp_rcdvd,11 # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_rmsusp_rssn,12  # Source MAG serial number        <w>
        .set    CMsp_rq_rmsusp_rdsn,16  # Dest. MAG serial number         <w>
        .set    CMsp_rq_rmsusp_rscl,20  # Source MAG cluster number       <b>
        .set    CMsp_rq_rmsusp_rsvd,21  # Source MAG VDisk number         <b>
        .set    CMsp_rq_rmsusp_rdcl,22  # Dest. MAG cluster number        <b>
        .set    CMsp_rq_rmsusp_rdvd,23  # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_rmsusp_cstate,24 # Current copy state code        <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_rmsusp_crstate,25 # Current copy reg. state       <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_rmsusp_mstate,26 # My current map (cor_mstate)    <b>
                                        #  state code
        .set    CMsp_rq_rmsusp_size,28  # Size of extended request message
#
# --- End Suspend Region Map Table Request Data Structure --------------------
#
# --- Suspend Region Map Table Response Data Structure -----------------------
#       The following data structure begins after the response header
#
#       No extended response data defined!
#
# --- End Suspend Region Map Table Response Data Structure -------------------
#
#
# --- Terminate Region Map Table Request Data Structure ----------------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_rmterm_rid,0    # Copy reg. ID                    <w>
        .set    CMsp_rq_rmterm_rcsn,4   # Copy MAG serial number          <w>
        .set    CMsp_rq_rmterm_rcscl,8  # Copy MAG source cluster number  <b>
        .set    CMsp_rq_rmterm_rcsvd,9  # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_rmterm_rcdcl,10 # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_rmterm_rcdvd,11 # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_rmterm_rssn,12  # Source MAG serial number        <w>
        .set    CMsp_rq_rmterm_rdsn,16  # Dest. MAG serial number         <w>
        .set    CMsp_rq_rmterm_rscl,20  # Source MAG cluster number       <b>
        .set    CMsp_rq_rmterm_rsvd,21  # Source MAG VDisk number         <b>
        .set    CMsp_rq_rmterm_rdcl,22  # Dest. MAG cluster number        <b>
        .set    CMsp_rq_rmterm_rdvd,23  # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_rmterm_cstate,24 # Current copy state code        <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_rmterm_crstate,25 # Current copy reg. state       <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_rmterm_size,28  # Size of extended request message
#
# --- End Terminate Region Map Table Request Data Structure ------------------
#
# --- Terminate Region Map Table Response Data Structure ---------------------
#       The following data structure begins after the response header
#
#       No extended response data defined!
#
# --- End Terminate Region Map Table Response Data Structure -----------------
#
# --- Read Region/Segment Map Table Request Data Structure -------------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_rmrd_rid,0      # Copy reg. ID                    <w>
        .set    CMsp_rq_rmrd_rcsn,4     # Copy MAG serial number          <w>
        .set    CMsp_rq_rmrd_rcscl,8    # Copy MAG source cluster number  <b>
        .set    CMsp_rq_rmrd_rcsvd,9    # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_rmrd_rcdcl,10   # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_rmrd_rcdvd,11   # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_rmrd_rssn,12    # Source MAG serial number        <w>
        .set    CMsp_rq_rmrd_rdsn,16    # Dest. MAG serial number         <w>
        .set    CMsp_rq_rmrd_rscl,20    # Source MAG cluster number       <b>
        .set    CMsp_rq_rmrd_rsvd,21    # Source MAG VDisk number         <b>
        .set    CMsp_rq_rmrd_rdcl,22    # Dest. MAG cluster number        <b>
        .set    CMsp_rq_rmrd_rdvd,23    # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_rmrd_cstate,24  # Current copy state code         <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_rmrd_crstate,25 # Current copy reg. state         <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_rmrd_mstate,26  # My current map (cor_mstate)     <b>
                                        #  state code
        .set    CMsp_rq_rmrd_reg,28     # Region map number               <w>
        .set    CMsp_rq_rmrd_size,32    # Size of extended request message
#
# --- End Read Region/Segment Map Table Request Data Structure ---------------
#
# --- Read Region/Segment Map Table Response Data Structure ------------------
#       The following data structure begins after the response header
#
        .set    CMsp_rs_rmrd_mstate,0   # My current map (cor_mstate)     <b>
                                        #  state code
        .set    CMsp_rs_rmrd_cnt,4      # Dirty segment count             <w>
        .set    CMsp_rs_rmrd_table,8    # Segment map table           1024<b>
        .set    CMsp_rs_rmrd_size,1032  # Size of extended response message
#
# --- End Read Region/Segment Map Table Response Data Structure --------------
#
# --- Clear Region/Segment Map Table Request Data Structure ------------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_rmclr_rid,0     # Copy reg. ID                    <w>
        .set    CMsp_rq_rmclr_rcsn,4    # Copy MAG serial number          <w>
        .set    CMsp_rq_rmclr_rcscl,8   # Copy MAG source cluster number  <b>
        .set    CMsp_rq_rmclr_rcsvd,9   # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_rmclr_rcdcl,10  # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_rmclr_rcdvd,11  # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_rmclr_rssn,12   # Source MAG serial number        <w>
        .set    CMsp_rq_rmclr_rdsn,16   # Dest. MAG serial number         <w>
        .set    CMsp_rq_rmclr_rscl,20   # Source MAG cluster number       <b>
        .set    CMsp_rq_rmclr_rsvd,21   # Source MAG VDisk number         <b>
        .set    CMsp_rq_rmclr_rdcl,22   # Dest. MAG cluster number        <b>
        .set    CMsp_rq_rmclr_rdvd,23   # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_rmclr_cstate,24 # Current copy state code         <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_rmclr_crstate,25 # Current copy reg. state        <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_rmclr_reg,28    # Region map number               <w>
        .set    CMsp_rq_rmclr_size,32   # Size of extended request message
#
# --- End Clear Region/Segment Map Table Request Data Structure --------------
#
# --- Clear Region/Segment Map Table Response Data Structure -----------------
#       The following data structure begins after the response header
#
#       No extended response data defined!
#
# --- End Clear Region/Segment Map Table Response Data Structure -------------
#
#
# --- Check Region Map Table State/Status Request Data Structure -------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_rmchk_rid,0     # Copy reg. ID                    <w>
        .set    CMsp_rq_rmchk_rcsn,4    # Copy MAG serial number          <w>
        .set    CMsp_rq_rmchk_rcscl,8   # Copy MAG source cluster number  <b>
        .set    CMsp_rq_rmchk_rcsvd,9   # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_rmchk_rcdcl,10  # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_rmchk_rcdvd,11  # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_rmchk_rssn,12   # Source MAG serial number        <w>
        .set    CMsp_rq_rmchk_rdsn,16   # Dest. MAG serial number         <w>
        .set    CMsp_rq_rmchk_rscl,20   # Source MAG cluster number       <b>
        .set    CMsp_rq_rmchk_rsvd,21   # Source MAG VDisk number         <b>
        .set    CMsp_rq_rmchk_rdcl,22   # Dest. MAG cluster number        <b>
        .set    CMsp_rq_rmchk_rdvd,23   # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_rmchk_cstate,24 # Current copy state code         <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_rmchk_crstate,25 # Current copy reg. state        <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_rmchk_mstate,26 # My current map (cor_mstate)     <b>
                                        #  state code
        .set    CMsp_rq_rmchk_size,28   # Size of extended request message
#
# --- End Check Region Map Table State/Status Request Data Structure ---------
#
# --- Check Region Map Table State/Status Response Data Structure ------------
#       The following data structure begins after the response header
#
        .set    CMsp_rs_rmchk_mstate,0  # My current map (cor_mstate)     <b>
                                        #  state code
        .set    CMsp_rs_rmchk_cnts,4    # Region dirty segment count   256<w>
                                        #  array
        .set    CMsp_rs_rmchk_size,1028 # Size of extended request message
#
# --- End Check Region Map Table State/Status Response Data Structure --------
#
#
# --- Suspend Copy Operation Request Data Structure --------------------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_opsusp_rid,0    # Copy reg. ID                    <w>
        .set    CMsp_rq_opsusp_rcsn,4   # Copy MAG serial number          <w>
        .set    CMsp_rq_opsusp_rcscl,8  # Copy MAG source cluster number  <b>
        .set    CMsp_rq_opsusp_rcsvd,9  # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_opsusp_rcdcl,10 # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_opsusp_rcdvd,11 # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_opsusp_rssn,12  # Source MAG serial number        <w>
        .set    CMsp_rq_opsusp_rdsn,16  # Dest. MAG serial number         <w>
        .set    CMsp_rq_opsusp_rscl,20  # Source MAG cluster number       <b>
        .set    CMsp_rq_opsusp_rsvd,21  # Source MAG VDisk number         <b>
        .set    CMsp_rq_opsusp_rdcl,22  # Dest. MAG cluster number        <b>
        .set    CMsp_rq_opsusp_rdvd,23  # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_opsusp_cstate,24 # Current copy state code        <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_opsusp_crstate,25 # Current copy reg. state       <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_opsusp_mstate,26 # My current map (cor_mstate)    <b>
                                        #  state code
        .set    CMsp_rq_opsusp_size,28  # Size of extended request message
#
# --- End Suspend Copy Operation Request Data Structure ----------------------
#
# --- Suspend Copy Operation Response Data Structure -------------------------
#       The following data structure begins after the response header
#
#       No extended response data defined!
#
# --- End Suspend Copy Operation Response Data Structure ---------------------
#
#
# --- Resume Copy Operation Request Data Structure ---------------------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_opresm_rid,0    # Copy reg. ID                    <w>
        .set    CMsp_rq_opresm_rcsn,4   # Copy MAG serial number          <w>
        .set    CMsp_rq_opresm_rcscl,8  # Copy MAG source cluster number  <b>
        .set    CMsp_rq_opresm_rcsvd,9  # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_opresm_rcdcl,10 # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_opresm_rcdvd,11 # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_opresm_rssn,12  # Source MAG serial number        <w>
        .set    CMsp_rq_opresm_rdsn,16  # Dest. MAG serial number         <w>
        .set    CMsp_rq_opresm_rscl,20  # Source MAG cluster number       <b>
        .set    CMsp_rq_opresm_rsvd,21  # Source MAG VDisk number         <b>
        .set    CMsp_rq_opresm_rdcl,22  # Dest. MAG cluster number        <b>
        .set    CMsp_rq_opresm_rdvd,23  # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_opresm_cstate,24 # Current copy state code        <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_opresm_crstate,25 # Current copy reg. state       <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_opresm_mstate,26 # My current map (cor_mstate)    <b>
                                        #  state code
        .set    CMsp_rq_opresm_size,28  # Size of extended request message
#
# --- End Resume Copy Operation Request Data Structure -----------------------
#
# --- Resume Copy Operation Response Data Structure --------------------------
#       The following data structure begins after the response header
#
#       No extended response data defined!
#
# --- End Resume Copy Operation Response Data Structure ----------------------
#
#
# --- Copy Device Moved Request Data Structure -------------------------------
#       The following data structure begins after the request header
#
        .set    CMsp_rq_opdmov_rid,0    # Copy reg. ID                    <w>
        .set    CMsp_rq_opdmov_rcsn,4   # Copy MAG serial number          <w>
        .set    CMsp_rq_opdmov_rcscl,8  # Copy MAG source cluster number  <b>
        .set    CMsp_rq_opdmov_rcsvd,9  # Copy MAG source VDisk number    <b>
        .set    CMsp_rq_opdmov_rcdcl,10 # Copy MAG dest. cluster number   <b>
        .set    CMsp_rq_opdmov_rcdvd,11 # Copy MAG dest. VDisk number     <b>
        .set    CMsp_rq_opdmov_rssn,12  # Source MAG serial number        <w>
        .set    CMsp_rq_opdmov_rdsn,16  # Dest. MAG serial number         <w>
        .set    CMsp_rq_opdmov_rscl,20  # Source MAG cluster number       <b>
        .set    CMsp_rq_opdmov_rsvd,21  # Source MAG VDisk number         <b>
        .set    CMsp_rq_opdmov_rdcl,22  # Dest. MAG cluster number        <b>
        .set    CMsp_rq_opdmov_rdvd,23  # Dest. MAG VDisk number          <b>
        .set    CMsp_rq_opdmov_cstate,24 # Current copy state code        <b>
                                        #  (cor_cstate)
        .set    CMsp_rq_opdmov_crstate,25 # Current copy reg. state       <b>
                                        #  (cor_crstate) code
        .set    CMsp_rq_opdmov_mstate,26 # My current map (cor_mstate)    <b>
                                        #  state code
        .set    CMsp_rq_opdmov_size,28  # Size of extended request message
#
# --- End Copy Device Moved Request Data Structure ---------------------------
#
# --- Copy Device Moved Response Data Structure ------------------------------
#       The following data structure begins after the response header
#
#       No extended response data defined!
#
# --- End Copy Device Moved Response Data Structure --------------------------
#
#
#*****************************************************************************
#               End CMsp Server Message Data Structures                      *
#*****************************************************************************
#
#
#*****************************************************************************
#                  LLD0 Server Message Structures                            *
#*****************************************************************************
#
# --- Establish VLink Extended Request Information Structure -----------------
#       The following structure begins after the request header
#
        .set    LLD0_rq_estvl_srccl,0   # Source MAG cluster #            <b>
        .set    LLD0_rq_estvl_srcpath,1 # Source MAG path #               <b>
                                        # 2 spare bytes                  2<b>
        .set    LLD0_rq_estvl_dstcl,4   # Destination MAG cluster #       <b>
        .set    LLD0_rq_estvl_dstvd,5   # Destination MAG VDisk #         <b>
                                        # 2 spare bytes                  2<b>
        .set    LLD0_rq_estvl_size,8    # Size of extended request message
#
# --- End Establish VLink Extended Request Information Structure -------------
#
# --- Establish VLink Extended Response Information Structure ----------------
#       The following structure begins after the response header
#
#       NOTE: Currently there is no response with the exception of the
#              response header.
#
        .set    LLD0_rs_estvl_size,00   # Size of extended response message
#
# --- End Establish VLink Extended Response Information Structure ------------
#
# --- Terminate VLink Extended Request Information Structure -----------------
#       The following structure begins after the request header
#
        .set    LLD0_rq_trmvl_srccl,0   # Source MAG cluster #            <b>
        .set    LLD0_rq_trmvl_srcpath,1 # Source MAG path #               <b>
                                        # 2 spare bytes                  2<b>
        .set    LLD0_rq_trmvl_dstcl,4   # Destination MAG cluster #       <b>
        .set    LLD0_rq_trmvl_dstvd,5   # Destination MAG VDisk #         <b>
                                        # 2 spare bytes                  2<b>
        .set    LLD0_rq_trmvl_size,8    # Size of extended request message
#
# --- End Terminate VLink Extended Request Information Structure -------------
#
# --- Terminate VLink Extended Response Information Structure ----------------
#       The following structure begins after the response header
#
#       NOTE: Currently there is no response with the exception of the
#              response header.
        .set    LLD0_rs_trmvl_size,00   # Size of extended response message
#
# --- End Terminate VLink Extended Response Information Structure ------------
#
# --- Peer Node Configuration Change Extended Request Information Structure --
#       The following structure begins after the request header
#
        .set    LLD0_rq_pncc_srccl,0    # Source MAG cluster #            <b>
        .set    LLD0_rq_pncc_srcpath,1  # Source MAG path #               <b>
                                        # 2 spare bytes                  2<b>
        .set    LLD0_rq_pncc_nname,4    # Source Node Name               8<b>
        .set    LLD0_rq_pncc_nsn,12     # Source Node S/N                4<b>
                                        # 2 spare bytes                  2<b>
        .set    LLD0_rq_pncc_size,16    # Size of extended request message
#
# --- End Peer Node Configuration Change Extended Request Information Structure
#
# --- Peer Node Configuration Change Extended Response Information Structure -
#       The following structure begins after the response header
#
#       NOTE: Currently there is no response with the exception of the
#              response header.
        .set    LLD0_rs_pncc_size,00   # Size of extended response message
#
# --- End Peer Node Configuration Change Extended Response Info Structure ----
#
#*****************************************************************************
#               End LLD0 Server Message Structures                           *
#*****************************************************************************
#
#
#*****************************************************************************
#                  DLM1 Server Message Structures                            *
#*****************************************************************************
#
# --- Poll a Datagram Path Extended Request Information Structure ------------
#       The following structure begins after the request header
#
#       NOTE: There is no Extended Request Information defined!
#
# --- End Poll a Datagram Path Extended Request Information Structure --------
#
# --- Poll a Datagram Path Extended Response Information Structure -----------
#       The following structure begins after the response header
#
#       NOTE: There is no response information defined!
#
#
# --- End Poll a Datagram Path Extended Response Information Structure -------
#
#
# --- Establish Communications to a Controller Ext Req Information Structure -
#       The following structure begins after the request header
#
#       NOTE: There is no Extended Request Information defined!
#
#   dgrq_g2-g5 will be used to base the serial number of the controller
#       that is wanted to be polled
#
#
# --- End Establish Communications to a Controller Ext Req Inf Structure ----
#
# --- Establish Communications to a Controller Ext Response Info Structure ---
#       The following structure begins after the response header
#
#       NOTE: There is no response information defined!
#
#
# --- End Establish Communications to a Controller Ext Response Info Structure
#
#
# --- Terminate Communications to a Controller Ext Req Information Structure -
#       The following structure begins after the request header
#
#       NOTE: There is no Extended Request Information defined!
#
#   dgrq_g2-g5 will be used to base the serial number of the controller
#       that is wanted to be polled
#
#
# --- End Terminate Communications to a Controller Ext Req Info Structure ----
#
# --- Terminate Communications to a Controller Ext Response Info Structure ---
#       The following structure begins after the response header
#
#       NOTE: There is no response information defined!
#
#
# --- End Terminate Communications to a Controller Ext Response Info Structure
#
#*****************************************************************************
#               End DLM1 Server Message Structures                           *
#*****************************************************************************
#
#
#*****************************************************************************
#                  CAC0 Server Message Structures                            *
#*****************************************************************************
#
# --- Error Code Definitions for 0x05 Status
#
        .set    CAC0_RANGE_ERROR,1      # EC #1 = Range Error
                                        # EC #2 = Which Memory Entry failed
        .set    CAC0_BAD_DRP_FC,2       # EC #1 = Bad DRP Function Code
                                        # dgrs_g0-dgrs_g1 = DRP Function Code
        .set    CAC0_BAD_DG_FC,3        # EC #1 = Bad Datagram Function Code
                                        # EC #2 = Function Code received
        .set    CAC0_BAD_MEM_REGION,4   # EC #1 = Bad Memory Region Specified
                                        # EC #2 = Invalid bits in Region field
        .set    CAC0_DATA_XFER_FAILED_SRP,5 # EC #1 = Data Transfer Failed
                                        # EC #2 = SRP Return code
        .set    CAC0_DATA_XFER_FAILED_ILT,6 # EC #1 = Data Transfer Failed
                                        # EC #2 = ILT Return code
        .set    CAC0_BAD_NVA_ID,7       # EC #1 = Bad NVA Record ID
                                        # EC #2 = M$rp4nva error code
#
# --- Write/Read Memory Extended Request Information Structure ---------------
#       The following structure begins after the request header
#
        .set    CAC0_rq_mem_addr,0      # Memory Address to write/read    <w>
        .set    CAC0_rq_mem_len,4       # Memory Length to write/read     <w>
        .set    CAC0_Entry_len,8        # size of address-length pair
#
#
# --- The following bit defines dgrq_g0 for which memory to write or read ----
#
        .set    CAC0_WCCT,0             # Write Cache Control Table
        .set    CAC0_WCC,1              # Write Cache Configuration
        .set    CAC0_BBD,2              # Battery Backup Data (diagnostic data)
        .set    CAC0_TAG,4              # Write Cache Tag
        .set    CAC0_DATA,5             # Write Cache Data
#
        .set    CAC0_WRTMEM_MASK,0x37   # Write Memory Allowable Bit Mask
        .set    CAC0_RDMEM_MASK,0x3F    # Read Memory Allowable Bit Mask
#
# --- End Write/Read Memory Extended Request Information Structure -----------
#
# --- Write Memory Extended Response Information Structure -------------------
#       The following structure begins after the response header
#
#       NOTE: Currently there is no response with the exception of the
#              response header.
#
#
# --- End Write Memory Extended Response Information Structure ---------------
#
#
# --- Read Memory Extended Request Information Structure ---------------------
#       The following structure begins after the request header
#
#           NOTE: See above for the structure
#
#
# --- End Read Memory Extended Request Information Structure -----------------
#
# --- Read Memory Extended Response Information Structure --------------------
#       The following structure begins after the response header
#
#   The response data is the actual memory read
#
# --- End Read Memory Extended Response Information Structure ----------------
#
#
# --- Flush Write Cache Mirror Data Extended Request Information Structure ---
#       The following structure begins after the request header
#
        .set    CAC0_rq_fldata_vid,0    # VDisk Number (VID) to Flush     <s>
                                        # 2 spare bytes                  2<b>
        .set    CAC0_rq_fldata_lba,4    # Logical Block Address to begin  <w>
        .set    CAC0_rq_fldata_len,8    # Number of Blocks to Flush       <w>
                                        # 4 spare bytes                   <w>
#
# --- End Flush Write Cache Mirror Data Extended Request Information Structure
#
# --- Flush Write Cache Mirror Data Extended Response Information Structure --
#       The following structure begins after the response header
#
#       NOTE: Currently there is no response with the exception of the
#              response header.
#
#   EC #1 definitions
#
        .set    CAC0_ec1_ndf,0x00       # No Data Found in range specified
        .set    CAC0_ec1_invalidVIDstate,0x01 # VID in an Invalid State
        .set    CAC0_ec1_flush_error,0x02 # An error occurred during the
                                        #   flush - see ec#2 (error code
                                        #   received during flush)
#
# --- End Flush Write Cache Mirror Data Ext Response Information Structure ---
#
#
# --- Set Write Information Extended Request Information Structure -----------
#       The following structure begins after the request header
#
        .set    CAC0_rq_swi_num,0       # Number of Records that follow
        .set    CAC0_rq_swi_hdrsz,4     # Size of Header
        .set    CAC0_rq_swi_ilt,0       # ILT associated with this record <w>
        .set    CAC0_rq_swi_nvaa,4      # NVA Address to use              <w>
                                        # NOTE: the following 3 fields must
                                        #  match the NV Record format required
                                        #  by M$ap4nva
        .set    CAC0_rq_swi_vid,8       # VDisk Number (VID) of Write Info<s>
                                        # 2 spare bytes                  2<b>
        .set    CAC0_rq_swi_len,12      # Number of Blocks to write       <w>
        .set    CAC0_rq_swi_slba,16     # Starting Logical Block Address  <l>
        .set    CAC0_rq_swi_size,24     # Size of one SWI Request Record
#
# --- End Set Write Information Extended Request Information Structure -------
#
# --- Set Write Information Extended Response Information Structure ----------
#       The following structure begins after the response header
#
        .set    CAC0_rs_swi_num,0       # Number of Records that follow
        .set    CAC0_rs_swi_hdrsz,4     # Size of header
        .set    CAC0_rs_swi_ilt,0       # ILT associated with record      <w>
        .set    CAC0_rs_swi_nvaa,4      # NVA Address returned            <w>
        .set    CAC0_rs_swi_size,8      # Size of one SWI Response record
#
# --- End Set Write Information Extended Response Information Structure ------
#
#
# --- Clear Write Information Request Information Structure ------------------
#       The following structure begins after the request header
#
        .set    CAC0_rq_cwi_num,0       # Number of record IDs to follow
        .set    CAC0_rq_cwi_nvaa,4      # 4 byte NVA Address's follow
                                        #  (variable length - no set size)
#
# --- End Clear Write Information Extended Request Information Structure -----
#
# --- Clear Write Information Extended Response Information Structure --------
#       The following structure begins after the response header
#
#       NOTE: Currently there is no response with the exception of the
#              response header.
#
#
# --- End Clear Write Information Extended Response Information Structure ---
#
#
#*****************************************************************************
#               End CAC0 Server Message Structures                           *
#*****************************************************************************
#
#
#*****************************************************************************
#                  CCB0 Server Message Structures                            *
#*****************************************************************************
#
# --- Send Packet Extended Request Information Structure ---------------------
#       The following structure begins after the request header
#
#       The Extended Request Information is the Send Packet Data
#
# --- End Send Packet Extended Request Information Structure -----------------
#
# --- Send Packet Extended Response Information Structure --------------------
#       The following structure begins after the response header
#
#       NOTE: Currently there is no response with the exception of the
#              response header.
#
#
# --- End Send Packet Extended Response Information Structure ----------------
#
#*****************************************************************************
#               End CCB0 Server Message Structures                           *
#*****************************************************************************
#
#
#******************************************************************************
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
