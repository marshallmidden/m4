# $Id: def.inc 161368 2013-07-29 14:53:10Z marshall_midden $
#**********************************************************************
#
#  NAME: def.inc (DEFine)
#
#  DESCRIPTION:
#
#       To provide a common means of platform packet definitions,
#       including those commands issued to the DEFINE module by the CCB.
#
#  Copyright (c) 1997-2005 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
#  TLG (Three Letter Guide) to the commands
#
#
# --- CCB to BE define commands
#
#     mcr  Create or expand a virtual device
#     mld  Label a physical device
#     mfd  Fail a device
#     mufd unfail a device
#     msi  Raw SCSI IO
#     mid  Initialize a RAID device
#     mfs  Internal file system operations
#     mdv  Delete a virtual device
#     msc  Set caching mode
#     mpr  Set server priority
#     mxn  Reset NVRAM, clear devices
#     mrn  Restore from NVRAM
#     mss  Start or stop a device
#     mwl  WWN lookup
#     med  Enable/disable scrubbing
#     msd  Set default label behavior
#     mxi  Stop I/O
#     mri  Start I/O
#     mdr  Restore device
#     mdd  Defragment device
#     maa  Set attribute
#     mbl  Get back end loop information
#     mgx  Get server list
#     mgx  Get virtual device list
#     mgx  Get RAID list
#     mgx  Get physical device list
#     mgx  Get enclosure list
#     mgx  Get misc device list
#     miv  Get virtual device information
#     mir  Get RAID information
#     mip  Get physical device information
#     mml  Map a LUN to a VDisk
#     mul  Unmap a LUN from a VDisk
#     msp  Get space information on disk
#     mms  Create a server
#     mds  Delete a server
#     mvc  Virtual disk control
#     mas  Assign system serial number
#     mgi  Get II information
#     mfp  Fetch PCI information
#     mcb  Burn flash code
#     mct  Configure target
#     mgt  Get target information
#     mbp  Set background priority
#     mgx  Get target list
#     mnc  Get number of remote controllers
#     mci  Get remote controller information
#     mvi  Get remote virtual disk information
#     mft  Set foreign target enable
#     mcv  Create virtual link
#     mvl  Get virtual link information (local)
#     mmc  Create new controller
#     mrb  Perform rebuild or resync stripes
#     mgn  Get NVRAM local image from BE
#     mun  Put NVRAM local image into NVRAM
#     mxd  Delete device (SES or physical disk)
#     mmp  Mode page
#     mdc  Device count for a particular serial number
#     mrh  Hotspare
#     mfc  File System file to file copy
#     mbv  Break VLink check
#     mes  Get SOS structure
#     mps  Put SOS structure
#     met  Force an error trap
#     mpc  Put SCMT structure
#     mlp  Perform BE Loop Primitive
#     mnd  Name device (names vdisk, controller, servers, etc)
#     mni  Get DLink information
#     mki  Get DLock information
#     mfa  FCAL analysis
#     mrm  Modify RAID AStatus Field
#     mr5  Start recovery on an Inoperable RAID 5
#     xxx  Put device configuration
#     myd  Get copy/resync data
#     myc  Copy/resync control
#
#
# --- BE define to FE define commands
#
#     mrs  Report server configuration
#     mrx  Server configuration complete
#     mrc  Report caching configuration
#     mcx  Caching configuration complete
#     mfs  Fetch server statics
#     mxc  Stop caching
#     mcc  Resume (continue) caching
#     mii  Set system information
#     mvu  Virtual disk updated
#     msu  Server updated
#     mrt  Report target
#     mre  Reset configuration
#     mcs  Set controller serial number
#     mmm  Send MM_INFO to FE
#
#
# --- FE define to BE define commands
#
#     miv  Get virtual device info the the FEP (identical to MRP/BEP)
#     msq  Set sequence number
#     msmp Save mirror partner
#     mfhl Fibre heartbeat list
#     mqc  Query FE Controller Communications
#
#
# --- Log events
#
#     mle  Log event
#
#
# --- CCB to FE define commands
#
#     mbl  Get front end loop information
#     mis  Get server information
#     mgc  Get global cache information
#     mgi  Get II information (identical to back end)
#     mfp  Fetch link information (identical to back end)
#     mcd  Get cache device information
#     mgs  Get server statistics
#     mbh  Set battery health
#     mcb  Burn flash code
#     msl  Server lookup
#     mamp Assign front end mirror partner
#     mmp  Mode page
#     mif  Invalidate FE Write Cache
#     met  Force a FE error trap
#     mlp  Perform FE Loop Primitive
#     mqs  Query Stop Complete
#     mqmp Query Mirror Partner Change Possible
#     mstd Set Write Cache Temporary Disable
#     mctd Clear Write Cache Temporary Disable
#     mqtd Query Write Cache Temporary Disable Flush Done
#
# --- Equates ---------------------------------------------------------
#
#       mr_func definitions
#
        .set    mrebfuncbase,0x100      # Base for CCB to back end commands
        .set    mrcrexp,0x100           # Create or expand a virtual device
        .set    mrgetelist,0x101        # Get SES device list
        .set    mrlabel,0x102           # Label a physical device
        .set    mrfail,0x103            # Fail a device
        .set    mrscsiio,0x104          # Raw SCSI IO
        .set    mrinitraid,0x105        # Initialize a RAID device
        .set    mrfcalanal,0x106        # FCAL analysis
        .set    mrdelvirt,0x107         # Delete a virtual device
        .set    mrsetcache,0x108        # Set caching mode
        .set    mrserverprop,0x109      # Set server properties
        .set    mrreset,0x10A           # Reset NVRAM, clear devices
        .set    mrrestore,0x10B         # Restore from NVRAM
        .set    mrawake,0x10C           # Awake
        .set    mrwwnlookup,0x10D       # WWN/LUN lookup
        .set    mrbegeneric,0x10E       # Generic
        .set    mrstartstop,0x10F       # Start or stop a device
        .set    mrscrubctrl,0x110       # Scrubbing/Scanning control
        .set    mrdefault,0x111         # Set default label behavior
        .set    mrgetbedevpaths,0x112   # Get BE Device Paths
        .set    mrrestoredev,0x113      # Restore a physical device
        .set    mrfragment,0x114        # Defragment device
        .set    mrsetattr,0x115         # Set attribute
        .set    mrbeloop,0x116          # Get back end loop information
        .set    mrgetslist,0x117        # Get server list
        .set    mrgetvlist,0x118        # Get virtual device list
        .set    mrgetrlist,0x119        # Get RAID list
        .set    mrgetplist,0x11A        # Get physical device list
        .set    mrgetmlist,0x11B        # Get misc device list
        .set    mrgetvinfo,0x11C        # Get virtual device information
        .set    mrgetrinfo,0x11D        # Get RAID information
        .set    mrgetpinfo,0x11E        # Get physical device information
        .set    mrmaplun,0x11F          # Map a LUN to a VDisk
        .set    mrunmaplun,0x120        # Unmap a LUN from a VDisk
        .set    mrgeteinfo,0x121        # Get SES information
        .set    mrcreateserver,0x122    # Create a server
        .set    mrdeleteserver,0x123    # Delete a server
        .set    mrgetminfo,0x124        # Get misc device info
        .set    mrvdiskcontrol,0x125    # Virtual disk control
        .set    mrassignsysinfo,0x126   # Assign system information
        .set    mrbeii,0x127            # Get back end II information
        .set    mrbelink,0x128          # Get back end link information
#        .set    mrbeboot,0x129          # Was Get back end boot code header
#        .set    mrbediag,0x12A          # Was Get back end diagnostic code header
        .set    mrbeproc,0x12B          # Get back end proc code header
#        .set    mrbecodeburn,0x12C      # Burn back end flash code
        .set    mrberwmem,0x12D         # Read/Write memory on back end
        .set    mrconfigtarg,0x12E      # Configure target
        .set    mrgetmplist,0x12F       # Get Mirror partner list
        .set    mrglobalpri,0x130       # Set background priority
        .set    mrgettlist,0x131        # Get target list
        .set    mrresetbechip,0x132     # Reset BE Qlogic chip
        .set    mrnamechange,0x133      # VDisk or controller name changed
        .set    mrremotectrlcnt,0x134   # Get remote controller count
        .set    mrremotectrlinfo,0x135  # Get remote controller information
        .set    mrremotevdiskinfo,0x136 # Get remote virtual disk information
        .set    mrforeigntargets,0x137  # Set foreign targets
        .set    mrcreatevlink,0x138     # Create vlink
        .set    mrgetvlinkinfo,0x139    # Get virtual link information
        .set    mrcreatectrlr,0x13A     # Create a new controller
        .set    mrrescandevice,0x13B    # Rescan physical devices
        .set    mrresync,0x13C          # Resync RAID or stripes
        .set    mrgetlclimage,0x13D     # Get local NVRAM image
        .set    mrputlclimage,0x13E     # Put local NVRAM image
        .set    mrdeletedevice,0x13F    # Delete SES or disk drive device
        .set    mrbemodepage,0x140      # Mode page
        .set    mrdevicecount,0x141     # Get device count
        .set    mrgetvidowner,0x142     # Get Vdisk Owner
        .set    mrhotspareinfo,0x143    # Hotspare information
        .set    mrfilecopy,0x144        # File system file to file copy
        .set    mrbegetdvlist,0x145     # Get device List
        .set    mrbegetportlist,0x146   # Get BE Port List
        .set    mrbreakvlock,0x147      # Break vlink lock
        .set    mrgetsos,0x0148         # Get SOS structure for a drive
        .set    mrputsos,0x0149         # Update a SOS structure
        .set    mrforcebeetrap,0x14A    # Force a BE error trap
        .set    mrputscmt,0x014B        # Update a SCMT structure
        .set    mrbeloopprimitive,0x14C # Perform a BE Loop Primitive
        .set    mrtargetcontrol,0x14D   # Target movement control
        .set    mrfailctrl,0x14E        # Fail / Unfail controller
        .set    mrnamedevice,0x14F      # Set name of device (vdisk, ctrl, etc.)
        .set    mrebiseDG,0x150         # interprocessor DataGram
        .set    mrnopbe,0x151           # BE no-op
        .set    mrputfsys,0x152         # Put a file system report
        .set    mrgetdlink,0x153        # Get DLink information
        .set    mrgetdlock,0x154        # Get DLock information
        .set    mrdegradeport,0x155     # Degrade / restore port
        .set    mrgetwsinfo,0x156       # Get workset information
        .set    mrsetwsinfo,0x157       # Set workset information
        .set    mrsetbeportconfig,0x158 # Set BE FC port configuration
        .set    mrsaveasyncnv, 0x159    # Save async NV to file
        .set    mrchgraidnotmirroring,0x15A # Chg RAID Not Mirroring State
        .set    mrputldd,0x15B          # Put the LDD
        .set    mrRAIDRecover,0x15C     # Start recovery on an Inoperable RAID 5
        .set    mrputdevconfig,0x15D    # Put device configuration entries
        .set    mrresyncdata,0x15E      # Get copy/resync data
        .set    mrresyncctl,0x15F       # Copy/resync control
        .set    mrrefresh,0x160         # Refresh from NVRAM
        .set    mrsetvpri,0x161
        .set    mrvprienable,0x162
        .set    mrpdiskspindown,0x163   # Spin down physical disk
        .set    mrpdiskfailback,0x164   # Unfail the designated hot spare to
                                        # its previous data drive
        .set    mrpdiskautofailbackenabledisable,0x165   # Enable / Disable Auto FailBack
        .set    mrcfgoption,0x166       # Configure options
        .set    mrsettginfo,0x167       # Set iSCSI target info
        .set    mrgettginfo,0x168       # Get iSCSI target info
        .set    mrupdsid,0x169          # Update iSCSI Server wwn
        .set    mrsetchap,0x16A         # Configure CHAP user
        .set    mrgetchap,0x16B         # CHAP user Info
        .set    mrgetgeolocation,0x16C  #  Get the location of bay and disks (not using it)
        .set    mrsetgeolocation,0x16D  # Set the location of bay and disks
        .set    mrcleargeolocation,0x16E # Clear the location of bay and disks
        .set    mrgetisnsinfo,0x16F     # Get iSNS server info
        .set    mrsetisnsinfo,0x170     # Set iSNS server info
        .set    mrsetpr,0x171           # Configure PR from CCB
        .set    mrvirtredundancy,0x172  # Get redundancy of VDisk
        .set    mrgetasync,0x173        # Get Async information to CCB
        .set    mrgetiseip,0x174        # Get ISE Bay
        .set    mralldevmiss,0x175      # All devices (BE) missing from a DCN
        .set    mrextndvinfo,0x176      # Get extended vdisk info
        .set    mrpdiskqltimeout,0x177  # Emulate BE Qlogic timeout on pdisk
        .set    mrgetdlink_gt2tb,0x178  # Get DLink information GT2TB version
        .set    mrreadwriteio,0x179     # Read/Write a Vdisk or Pdisk
        .set    mrswappids,0x17A        # Swap two PIDs.
        .set    mremulatepab,0x17B      # Emulate PAB on PID (on/off).
        .set    mrebfuncmax,0x17B       # Maximum CCB to back end define cmds.
        .set    mrnopfsys,0x1FD         # Internal file system no op
        .set    mrfsysop,0x1FE          # Internal file system operation
        .set    mrbehbeat,0x1FF         # Back end heartbeat from CCB
#
        .set    mrbffuncbase,0x200      # Base for back end to front end cmds
        .set    mrreportsconfig,0x200   # Report server configuration
        .set    mrsconfigcomplete,0x201 # Server configuration complete
        .set    mrreportcconfig,0x202   # Report caching configuration
        .set    mrcconfigcomplete,0x203 # Caching configuration complete
        .set    mrspare204,0x204        # Spare
        .set    mrstopcache,0x205       # Stop caching
        .set    mrcontinuecache,0x206   # Continue caching
        .set    mrsetsysinfo,0x207      # Set system serial number, etc.
        .set    mrvchange,0x208         # Virtual disk config has changed
        .set    mrschange,0x209         # Server config has changed
        .set    mrreporttarg,0x20A      # Report target config
        .set    mrresetconfig,0x20B     # Reset configuration, NVRAM, etc
        .set    mrsetcntlsnfe,0x20C     # Set controller serial number, FE
        .set    mrmminfo,0x020D         # MM_INFO structure BE => FE
        .set    mrsetconfigopt,0x020E   # Set configurable options BE => FE
        .set    mrupdtginfo,0x20F       #
        .set    mrgetporttype,0x210     #
        .set    mrsetchapfe,0x211       #
        .set    mrsetisnsinfofe,0x212   # set iSNS server info -- front end.
        .set    mrsetpres,0x213         # Config Persistent Reservation info
        .set    mrsetft,0x214           # Set Foreign Target mode (on/off)
        .set    mrbffuncmax,0x214       # Maximum for back end to front end cmds
#
        .set    mrlogfe,0x300           # Create log entry (Front end to CCB)
        .set    mrlogbe,0x301           # Create log entry (Back end to CCB)
#
        .set    mrfbfuncbase,0x400      # Front end to back end base
        .set    mrfegetvinfo,0x400      # Get VDisk info to FEP
        .set    mrfesetseq,0x401        # Set Sequence Number
        .set    mrfesetmp,0x402         # Set Mirror Partner
        .set    mrretrievepr,0x403      # Update Persistent Reservation info
        .set    mrfbfuncmax,0x403       # Front end to back end max
#
        .set    mreffuncbase,0x500      # CCB to front end base
        .set    mrfeloop,0x500          # Get front end loop information
        .set    mrgetsinfo,0x501        # Get server information
        .set    mrgetcinfo,0x502        # Get cache information
        .set    mrfelink,0x503          # Get front end link information
        .set    mrfeii,0x504            # Get front end II information
        .set    mrgetcdinfo,0x505       # Get cache device information
        .set    mrgetsstats,0x506       # Get server statistics
        .set    mrsetbathealth,0x507    # Set battery health
        .set    mrresumecache,0x508     # Resume cache initialization
#        .set    mrfeboot,0x509          # Was Get front end boot code header
#        .set    mrfediag,0x50A          # Was Get front end diagnostic code header
        .set    mrfeproc,0x50B          # Get front end proc code header
#        .set    mrfecodeburn,0x50C      # Burn back end flash code
        .set    mrferwmem,0x50D         # Read/Write memory on front end
        .set    mrresetfechip,0x50E     # Reset FE Qlogic chip
        .set    mrserverlookup,0x50F    # Server lookup MRP
        .set    mrfegeneric,0x510       # Generic
        .set    mrfeassignmp,0x511      # Assign a Mirror Partner
        .set    mrfefibrehlist,0x512    # FE Fibre Heartbeat List for DLM
        .set    mrfecontwomp,0x513      # Continue Cache Init w/o Mirror Partner
        .set    mrfeflushwomp,0x514     # Flush FE Cache w/o Mirror Partner
        .set    mrinvfewc,0x515         # Invalidate the FE Write Cache
        .set    mrflushbewc,0x516       # Flush the BE Write Cache
        .set    mrinvbewc,0x517         # Invalidate the BE Write Cache
        .set    mrfemodepage,0x518      # Mode page
        .set    mrfegetdvlist,0x519     # Get Device List
        .set    mrfegetportlist,0x51A   # Get FE Port List
        .set    mrgettrlist,0x51B       # Get target resource list
        .set    mrstopio,0x51C          # Stop I/O
        .set    mrstartio,0x51D         # Start I/O
        .set    mrfeportnotify,0x51E    # Set FE port event notification
        .set    mrforcefeetrap,0x51F    # Force a FE error trap
        .set    mrfeloopprimitive,0x520 # Perform a FE Loop Primitive
        .set    mrgettarg,0x521         # Get target information
        .set    mrfailport,0x522        # Fail / Unfail port
        .set    mrnopfe,0x523           # No-op
        .set    mrqfecc,0x524           # Query FE Controller Communications
        .set    mrqsc,0x525             # Query Stop Complete
        .set    mrqmpc,0x526            # Query Mirror Partner Change OK?
        .set    mrgethbastats,0x527     # Get HBA statistics
        .set    mrmicromemory,0x528     # Get MM status
        .set    mrgetmpconfigfe,0x529   # Get Mirror Partner configuration
        .set    mrfeportgo,0x52A        # Put Regular Ports on FE Fabric
        .set    mrsettdiscache,0x52B    # Set Temp Disable Cache
        .set    mrclrtdiscache,0x52C    # Clear Temp Disable Cache
        .set    mrqtdisflushdone,0x52D  # Query WC Temp Disable Flush Done
        .set    mrmmtest,0x52E          # Test driver for MM/NV Card
        .set    mrfeflerrorwomp,0x52F   # Flush FE Cache due to Error to MP
        .set    mrgetsessions,0x530     # Get ISCSI Session Info
        .set    mrgetsessionsperserver,0x531 # Get ISCSI Session Info per server
        .set    mrgetiddinfo,0x532 # Get IDD Info
        .set    mrdlmpathstats,0x533    # DLM Path Stats
        .set    mrdlmpathselectionalgo,0x534 # DLM path selection algorithm
        .set    mrprget,0x535           # Get persistent reserve data
        .set    mpprclr,0x536           # Clear persistent reserve data
        .set    mrprcfgcomplete,0x537   # PR config change complete
        .set    mrprcfgupdate,0x538     # PR config change update
        .set    mrsetfeportconfig,0x539 # Set FE FC port configuration
        .set    mreffuncmax,0x539       # Max CCB to front end define commands
#
        .set    mrfehbeat,0x5FF         # Front end heartbeat from CCB
#
#       de_retstat definitions
#
        .set    deok,             0x00  # OK status
        .set    deinvpkttyp,      0x01  # Invalid packet type
        .set    deinvpktsiz,      0x02  # Invalid packet size
        .set    debaddam,         0x03  # Bad DAM entry
        .set    delisterror,      0x04  # error getting list
        .set    de2tblimit,       0x05  # 2TB limit exceeded
        .set    denonxdev,        0x06  # Non-existent device
        .set    deinopdev,        0x07  # Inoperative device
        .set    deinvlabtyp,      0x08  # Invalid labtype
        .set    dedevused,        0x09  # Specified device in use
        .set    deinitinprog,     0x0A  # Initialization in progress
        .set    deinvwwname,      0x0B  # Invalid world wide name
        .set    deioerr,          0x0C  # I/O error
        .set    deinvrtype,       0x0D  # Invalid RAID type
#        .set    decodeburn,       0x0E  # Code burn failed
        .set    dedefnrdy,        0x0F  # Define is not ready for full cmd set
#
        .set    deinsdevcap,      0x10  # Insufficient device capacity
        .set    desesinprogress,  0x11  # SES discovery in progress
        .set    deinvdrvcnt,      0x12  # Invalid drive count
        .set    deactivedef,      0x13  # Active defragmentation
        .set    denotdatalab,     0x14  # Not labelled data device
        .set    deinvdepth,       0x15  # Invalid depth
        .set    deinvstripe,      0x16  # Invalid stripe size
        .set    deinvparity,      0x17  # Invalid parity specified
        .set    deinvvirtid,      0x18  # Invalid virtual ID
        .set    deinvop,          0x19  # Invalid operation
        .set    deinvsesslot,     0x1A  # Invalid SES or slot information
        .set    debusy,           0x1B  # Busy
        .set    demaxluns,        0x1C  # Max LUNs mapped
        .set    denotoprid,       0x1D  # RAID device not operative
        .set    demaxsegs,        0x1E  # Maximum segments exist
        .set    debadnvrec,       0x1F  # nvram p2 record on disk is not valid
#
        .set    deinsuffredund,   0x20  # insufficent redundancy exists
        .set    depidnotused,     0x21  # PID not used by any RAIDs
        .set    denohotspare,     0x22  # No hotspare drive or insufficient capacity
        .set    deinvpri,         0x23  # Invalid priority
        .set    denotsync,        0x24  # Secondary copy no synchronized
        .set    deinsnvram,       0x25  # Insufficient NVRAM
        .set    deforeigndev,     0x26  # Foreign device
        .set    derebuildnotreq,  0x27  # Rebuild not required
        .set    detypemismatch,   0x28  # Device type mismatch on create/expand
        .set    deinvopt,         0x29  # Invalid option
        .set    deinvtid,         0x2A  # Invalid target ID
        .set    deinvctrl,        0x2B  # Invalid controller ID
        .set    deretlenbad,      0x2C  # Invalid return data space allowance
        .set    deactrebuild,     0x2D  # Active rebuild
        .set    detoomuchdata,    0x2E  # Too much data for return
        .set    deinvsid,         0x2F  # Invalid server
#
        .set    deinvvid,         0x30  # Invalid VID number
        .set    deinvrid,         0x31  # Invalid RID number
        .set    deinvpid,         0x32  # Invalid PID number
        .set    deinstable,       0x33  # Insufficient table space
        .set    deinsres,         0x34  # Insufficient resources
        .set    delunmapped,      0x35  # LUN is already mapped
        .set    deinvchan,        0x36  # Invalid port number
        .set    delunnotmapped,   0x37  # LUN is not mapped
        .set    dewcrecvryfailed, 0x38  # Cache recovery failures occurred
        .set    deqresetfailed,   0x39  # Qlogic chip reset failed
        .set    deunasspath,      0x3A  # Unassigned Path
        .set    deoutops,         0x3B  # Outstanding ops prevent completion
        .set    dechecksum,       0x3C  # Bad checksum
        .set    defailed,         0x3D  # operation failed
        .set    decodesame,       0x3E  # Code loads are identical
        .set    destopzero,       0x3F  # Stop Count is already zero
#
        .set    deinsmem,         0x40  # Insufficient memory for file copy
        .set    denotarget,       0x41  # No target for ports
        .set    denoport,         0x42  # No spare ports
        .set    deinvwsid,        0x43  # Invalid work set ID
        .set    demaxsnapshots,   0x44  # Max snapshot count exceeds
#
#--- These error slot IDs are free--can be reused by any application
#    These are part of old georaid code that is removed.
        .set    demaxcopies,      0x45  # Max copy count exceeds
#       unused  0x46
        .set    de_ip_or_subnet_zero, 0x47 # IP or subnet address are 0
        .set    de_set_ip_error,  0x48  # Error setting IP address
        .set    de_vdisk_in_use,  0x49  # Vdisk in use on another controller
#       unused  0x4A
        .set    debaddevtype,     0x4B  # Unknown type used for create/expand
#
        .set    deemptycpylist,   0x50  # The copy list is empty
        .set    denocpymatch,     0x51  # no copy matches the requirments
        .set    denocomm,         0x52  # no communication to the new mirror partner
        .set    depidused,        0x53  # PID used by any RAIDs
        .set    depidspundown,    0x54  # Designated PID has already been spundown
        .set    denohsdname,      0x55  # HSDNAME is not available
        .set    denofailbackpdd,  0x56  # No [drive or valid-drive] found for failback
        .set    denothotlab,      0x57  # Designated device is not a hotspare
        .set    dedisknotinit,    0x58  # Unable to initialize the drive
        .set    deinvhstype,     0x59   # Hotspare not same devType as failing drive
# Nos 60 - 62 are used by ISCSI...they need to define it
        .set    deaswapstate,      0x63  # aswap/aswapback in progress /src vdisk autoswapped
        .set    deaswapinprogress, 0x64  # Auto Swap in progress
        .set    deinvpktversion,   0x65  # Incompatible packet version
        .set    dedestnothotlab,   0x68  # Designated Destination device is not a hotspare
        .set    de64tblimit,       0x69  # 64TB limit exceeded
#
# --- These are old and no longer used.
#
#        .set    dedupdname,10           # Duplicate dname
#        .set    deduprid,14             # Duplicate RAID ID
#        .set    deinvdevcap,15          # Invalid device capacity
#        .set    deinvskew,17            # Invalid skew
#        .set    deincondrvcnt,19        # Inconsistent drive count
#        .set    deundefrid,26           # Undefined RAID ID
#        .set    dedupvid,27             # Duplicate virtual ID
#        .set    deundefvid,28           # Undefined virtual ID
#        .set    deinvmode,33            # Invalid mode
#        .set    deunasshab,34           # Unassigned HAB
#        .set    deinvview,39            # Invalid view
#        .set    deinvmacslot,40         # Invalid MAC slot
#        .set    deinvlun,42             # Invalid LUN
#        .set    deinvattr,43            # Invalid attribute
#        .set    detoobig,44             # Device capacity too large
#        .set    dehsinta,0x7f           # fault trace in host.as code
#        .set    dehsint,0x80            # fault trace in host.as code
#        .set    dehinitdone,0x81        # indicate that host.as init is done.
#
# --- Begin base management request structure -------------------------
        .set    mr_func,0               # Packet type               <s>
        .set    mr_version,mr_func+2    # Version of command        <b>
#
#       reserved 9
#
        .set    mr_pptr,mr_version+10   # MRP pointer across PCI bus<w>
#                                                                  *****
        .set    mr_ralloclen,mr_pptr+4  # Return data alloc len     <w>
        .set    mr_rptr,mr_ralloclen+4  # Return data pointer       <w>
        .set    mr_ptr,mr_rptr+4        # Request packet            <w>
        .set    mr_len,mr_ptr+4         # Request size              <w>
# --- End base structure ----------------------------------------------
        .set    mrpsize,mr_len+4        # Size of MRP base structure.
                                        # The max size of the MRP including
                                        # the base structure is determined
                                        # by the VRPALLOC constant.
#
# --- Begin base management response structure ------------------------
#       reserved 3
        .set    mr_status,3             # Response status           <b>
        .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mrrsiz,mr_rlen+4        # Base size
#
# --- Begin create / expand virtual device definitions (mrcrexp)-------
# mcr_rtype
        .set    mcrstd,0                # Standard (non-RAID)
        .set    mcrraid0,1              # RAID 0
        .set    mcrraid1,2              # RAID 1
        .set    mcrraid5,3              # RAID 5
        .set    mcrraid10,4             # RAID 10 (0/1)
#
# mcr_op
        .set    mcrcreate,0             # Create device
        .set    mcrexpand,1             # Expand device
        .set    mcrtest,2               # Test device
#
# mcr_flags
                                        # Bits 5 through 7 unused.
        .set    ce_bits_setsize,4       # 0x10 - Set vdisk size on create.
        .set    ce_bits_7000min,3       # 0x08 - For 7000, increment min PD up to 16 or till works.
        .set    ce_bits_georaid,2       # 0x04 - GeoRaid
        .set    ce_bits_redun,1         # 0x02 - Enforce bay redundancy.
        .set    ce_bits_minpd,0         # 0x01 - The min PD argument, limit to that.
# --- Begin Create / Expand virtual device request base structure -----
        .set    mcr_rtype,0             # RAID type                 <b>
        .set    mcr_op,mcr_rtype+1      # Operation (exp or create) <b>
        .set    mcr_vid,mcr_op+1        # Virtual disk ID (exp only)<s>
#
        .set    mcr_drives,mcr_vid+2    # Number of drives          <s>
        .set    mcr_stripe,mcr_drives+2 # Stripe size               <s>
#
        .set    mcr_devcap,mcr_stripe+2 # Device capacity           <l>
#
        .set    mcr_depth,mcr_devcap+8  # Depth                     <b>
        .set    mcr_parity,mcr_depth+1  # Parity                    <b>
        .set    mcr_maxraids,mcr_parity+1 # Max RAIDs in this op    <s>
#
        .set    mcr_thresh,mcr_maxraids+2 # Min space on each disk  <s>
        .set    mcr_flags,mcr_thresh+2  # Flags for geo-RAID, etc   <b>
        .set    mcr_minpd,mcr_flags+1   # Min physical disks        <b>
#
        .set    mcr_dmap,mcr_minpd+1    # Start of PID list         <s>
# --- End base structure ----------------------------------------------
        .set    mcrsiz,mcr_dmap         # Base size (add 4*mcr_drives)
#
# --- Begin Create / Expand virtual device response base structure ----
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mcr_cvid,mr_rlen+4      # VID created/expanded      <s>
        .set    mcr_clflag,mcr_cvid+2   # Cross Geo Location Flag   <b>
#       reserved 1
        .set    mcr_adevcap,mcr_cvid+4  # Available device capacity <l>
# --- End base structure ----------------------------------------------
        .set    mcrrsiz,mcr_adevcap+8   # Size of response
#
# --- Begin set priority definitions -------
#       mcr_opt
#
        .set    mspnochange,0           # Leave priority unchanged
        .set    mspsetlow,1             # Set low priority
# Status values
        .set    mspok,0                 # Success
        .set    mspbadopt,1             # Invalid Options value
        .set    mspnovids,2             # No VIDs
        .set    mspbadcnt,3             # Invalid VID count
# Return data flags
        .set    mspdone,0               # Success
        .set    mspnovdd,1              # VDD not present
        .set    mspbadpri,2             # Invalid priority
# --- VID-Priority pairs structure -----
        .set msp_vid,0
        .set msp_pri,msp_vid+2
#       Reserved 1b
# --- End base structure ----------------------------------------------
        .set msp_vidprisiz,msp_pri+2
#
# --- Begin set priority request base structure -----
        .set    msp_cnt,0               # No. of vdisks in the list <s>
        .set    msp_respcnt,msp_cnt+2   # No. of vdisks in response <s>
        .set    msp_opt,msp_respcnt+2   # Options                   <s>
#       Reserved 6b
        .set    msp_lst,msp_opt+8       # List (vid,pri...)         <w>
# --- End base structure ----------------------------------------------
        .set    mspsiz,msp_lst+4   #    Base size
#
# --- Begin set priority response base structure ----
#       reserved 3
        .set    mspr_status,mr_status       # Response status           <b>
        .set    mspr_rlen,mspr_status+1     # Return length             <w>
        .set    mspr_nvid,mspr_rlen+4       # Number of Vdisks          <s>
#       Reserved 6b
        .set    mspr_vlist,mspr_nvid+8      # Start of device list     *<w>
# --- End base structure ----------------------------------------------
        .set    msprsiz,mspr_vlist+4       # Size of response
#
# --- Begin Get List definitions (mrgetelist) -------------------------
# --- These are used for all of the list fetching functions
# --- Begin Get List base structure -----------------------------------
        .set    mgx_sid,0               # Starting ID               <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mgxsiz,mgx_sid+4        # Base size
        .set    mtgrsiz,mgx_sid+8       # Base size for get target resource
#
# --- Begin Get List response base structure --------------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mgx_ndevs,mr_rlen+4     # Number of devices         <s>
#       reserved 2
        .set    mgx_list,mgx_ndevs+4    # Start of device list     *<s>
# --- End base structure ----------------------------------------------
        .set    mgxrsiz,mgx_list        # Size of response
                                        # Add two bytes per device
#
# --- Begin label physical device definitions (mrlabel) ---------------
#       mdl_labtype - these MUST match the pd_class definitions
        .set    mldnolabel,0            # Unlabelled device
        .set    mlddatalabel,1          # Labelled data device
        .set    mldsparelabel,2         # Labelled hotspare device
        .set    mldndatalabel,3         # Labelled non-redundant data device
#       mdl_opt bit positions
        .set    mldfull,0               # Full init prior to label
        .set    mldfsys,1               # Init file sytem
        .set    mldduplicate,2          # Duplicate the file sys off good disk
# --- Begin label physical device request base structure --------------
        .set    mld_pid,0               # PID of drive being labelled<s>
        .set    mld_labtype,mld_pid+2   # Label type                <b>
        .set    mld_opt,mld_labtype+1   # Options                   <b>
        .set    mld_dname,mld_opt+1     # Location information      <w>
# --- End base structure ----------------------------------------------
        .set    mldsiz,mld_dname+4      # Size of label request less list
#
# --- Begin label physical device response base structure -------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mldrsiz,mr_rlen+4       # Size of response
#
# --- Begin fail physical device definitions (mrfail) -----------------
#       mfd_options bit definitions
        .set    mfdforce,0              # Fail regardless of redundancy
        .set    mfdusehspid,1           # Use given hotspare pid
        .set    mfdfaillabel,2          # Write the fail label
        .set    mfdspindown,3           # Just spindown
        .set    mfdlograiderror,4       # Send a raid error log event
        .set    mfdchecksys,5           # Check PSDs for rebuild or hotswap
# --- Begin fail physical device request base structure ---------------
        .set    mfd_pid,0               # Device PID                <s>
        .set    mfd_hspid,mfd_pid+2     # Hotspare device PID       <s>
        .set    mfd_options,mfd_hspid+2 # Options                   <b>
#       reserved 3
# --- End base structure ----------------------------------------------
        .set    mfdsiz,mfd_options+4    # Size of fail device request
#
# --- Begin fail physical device response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mfdrsiz,mr_rlen+4       # Size of response
#
# --- Begin set geo location request base structure ---------------
        .set    mfg_bayid,0               # Bay ID                  <b>
        .set    mfg_location,mfg_bayid+1  # Geo location            <b>
#       reserved 2
# --- End base structure ----------------------------------------------
#
# --- Begin clear geo location request base structure ---------------
        .set    mfg_bid,0               # Bay ID                  <s>
#       reserved 2
# --- End base structure ----------------------------------------------
#
# --- Begin VDisk redundancy request base structure ---------------
        .set    mfg_vid,0               # VDisk ID                <s>
#       reserved 2
# --- End base structure ----------------------------------------------
#
# --- Begin unfail physical device definitions (mrpdiskfailback) -----------------
#       mfbd_options bit definitions
        .set    mfbdcancel,0            # cancel the failback operation
# --- Begin un-fail used-hotspare physical device request base structure ---------------
        .set    mfbd_hspid,0            # Hotspare device PID       <s>
        .set    mfbd_options,mfbd_hspid+2 # Options                   <b>
#       reserved 1
# --- End base structure ----------------------------------------------
        .set    mfbdsiz,mfbd_options+2  # Size of unfail device request
#
# --- Begin unfail physical device response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mfbdrsiz,mr_rlen+4      # Size of response
#
# --- Begin Auto FailBack enable / disable  definitions (mrpdiskautofailbackenabledisable) -----------------
#       mfbd_options bit definitions
        .set    mafbenable,1            # Enable Auto FailBack feature
# --- Begin Auto FailBack Enable / Disable request base structure ---------------
        .set    mafb_options,0          # Auto FailBack enable / disable   <b>
#       reserved 3
# --- End base structure ----------------------------------------------
        .set    mafbsiz,mafb_options+4  # Size of auto failback enable / disable request
#
# --- Begin auto failback enable / disable  response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mr_mode,mr_rlen+4       # Mode                      <b>
#       reserved 3
# --- End base structure ----------------------------------------------
        .set    mafbrsiz,mr_mode+4      # Size of response
#
# --- Begin SCSI IO definitions (mrscsiio) ----------------------------
        .set    msicli,1                # Identify device by ch/lun/id
        .set    msiwwn,2                # Identify device by wwn
# --- Begin SCSI IO request base structure ----------------------------
        .set    msi_rsn,0               # Request serial number     <w>
        .set    msi_channel,msi_rsn+4   # FC port number            <b>
        .set    msi_cmdlen,msi_channel+1# Command length            <b>
        .set    msi_func,msi_cmdlen+1   # PRP function              <b>
        .set    msi_strat,msi_func+1    # Strategy                  <b>
        .set    msi_idchc,msi_strat+1   # How to identify target    <b>
#       reserved 3
        .set    msi_wwn,msi_idchc+4     # WWN                       <l>
        .set    msi_id,msi_wwn+8        # FC ID                     <w>
        .set    msi_retry,msi_id+4      # Retry count               <b>
        .set    msi_flags,msi_retry+1   # Flags                     <b>
        .set    msi_lun,msi_flags+1     # LUN                       <s>
        .set    msi_sda,msi_lun+2       # Starting disk address     <l>
        .set    msi_eda,msi_sda+8       # Ending disk address       <l>
        .set    msi_timeout,msi_eda+8   # Timeout value             <w>
#
        .set    msi_dptr,msi_timeout+4  # Delayed Status Block ptr  <w>
#
        .set    msi_bptr,msi_dptr+4     # Buffer pointer            <w>
        .set    msi_blen,msi_bptr+4     # Buffer length             <w>
        .set    msi_cdb,msi_blen+4      # SCSI CDB                  <q>
# --- End base structure ----------------------------------------------
        .set    msisiz,msi_cdb+16       # Size of SCSI IO request
#
# --- Begin SCSI IO response base structure ---------------------------
        .set    msi_ascq,0              # ASCQ                      <b>
        .set    msi_asc,msi_ascq+1      # ASC                       <b>
        .set    msi_skey,msi_asc+1      # Sense key                 <b>
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    msi_rrsn,mr_rlen+4      # Response RSN (same as req)<w>
# --- End base structure ----------------------------------------------
        .set    msirsiz,msi_rrsn+4      # Response size
#
# --- Begin SCSI IO delayed status block base structure ---------------
        .set    msi_dascq,0             # ASCQ                      <b>
        .set    msi_dasc,msi_dascq+1    # ASC                       <b>
        .set    msi_dskey,msi_dasc+1    # Sense key                 <b>
        .set    msi_dstatus,mr_status   # Response status           <b>
        .set    msi_drlen,msi_dstatus+1 # Return length             <w>
#
        .set    msidsiz,msi_drlen+4     # Size of delayed status block
#
# --- Begin READWRITE IO definitions (mrreadwriteio) ------------------
# --- Begin READWRITE IO request base structure -----------------------
        .set    mrw_pv,0                # 'p'disk or 'v'disk        <b>
        .set    mrw_rw,mrw_pv+1         # 'r'ead or 'w'rite         <b>
        .set    mrw_id,mrw_rw+1         # pid or vid                <s>
        .set    mrw_block,mrw_id+2      # Block number to r/w       <l>
        .set    mrw_bptr,mrw_block+8    # Data pointer              <w>
        .set    mrw_dataInLen,mrw_bptr+4 # Buffer length            <w>
# --- End base structure ----------------------------------------------
        .set    mrw_siz,mrw_dataInLen+4 # Size of READWRITE IO request
#
# --- Begin READWRITE IO response base structure ----------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mrwrsiz,mmr_rlen+4      # Response size
#
# --- Begin Swap Two PIDs definitions (mrswappids) --------------------
# --- Begin Swap Two PIDs request base structure ----------------------
        .set    s2p_pid1,0              # First PID to be swapped
        .set    s2p_pid2,2              # Second PID to be swapped
# --- End base structure ----------------------------------------------
        .set    s2psiz,s2p_pid2+2       # Size of request
#
# --- Begin Swap Two PIDs response base structure ---------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    s2prsiz,mr_rlen+4       # Response size
#
# --- Begin Emulate PAB definitions (mremulatepab) --------------------
# --- Begin Emulate PAB request base structure ------------------------
        .set    epab_pid,0              # PID to have PAB emulated on
#       reserved 2 
# --- End base structure ----------------------------------------------
        .set    epabsiz,epab_pid+4        # Size of request
#
# --- Begin Emulate PAB response base structure -----------------------
        .set    epabcnt,mr_status-3     # first byte of reserved 3  <b>
#       reserved 2
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    epabrsiz,mr_rlen+4      # Response size
#
# --- Begin Initialize RAID definitions (mrinitraid) ------------------
# --- Begin Initialize RAID request base structure --------------------
        .set    mid_rid,0               # RAID ID to be initialized <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    midsiz,mid_rid+4        # Size of request
#
# --- Begin Initialize RAID response base structure -------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    midrsiz,mr_rlen+4       # Response size
#
# --- Begin Internal File System Operation definitions (mrfsysop) -----
        .set    mfsoprd,0x00            # Read operation
        .set    mfsopwr,0x01            # Write operation
# --- Begin Internal File System Operation request base structure -----
        .set    mfs_fid,0               # File ID to be read/written<b>
        .set    mfs_conf,mfs_fid+1      # Confirm on a Read         <b>
        .set    mfs_op,mfs_conf+1       # File operation            <b>
#       reserved 1
        .set    mfs_buffptr,mfs_op+2    # Buffer ptr of src/dest    <w>
        .set    mfs_bcount,mfs_buffptr+4# Number of blocks          <s>
        .set    mfs_offset,mfs_bcount+2 # Starting block            <s>
        .set    mfs_pidmap,mfs_offset+2 # Targetted pids (bitmap)   <w>
# --- End base structure ----------------------------------------------
        .set    mfssiz,mfs_pidmap+4     # Size of request
#
# --- Begin Internal File System Operation response base structure ----
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mfs_good,mr_rlen+4      # How many good writes      <w>
        .set    mfs_error,mfs_good+4    # How many bad writes       <w>
        .set    mfs_goodmap,mfs_error+4 # Bit map of good writes   *<w>
# --- End base structure ----------------------------------------------
        .set    mfsrsiz,mfs_goodmap+((MAXDRIVES+7)/8)   # Response size
#
# --- Begin Delete Virtual disk definitions (mrdelvirt) ---------------
# --- Begin Delete virtual disk request base structure ----------------
        .set    mdv_vid,0               # VDisk ID to be deleted    <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mdvsiz,mdv_vid+4        # Size of request
#
# --- Begin Delete virtual disk response base structure ---------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mdvrsiz,mr_rlen+4       # Response size
#
# --- Begin Set Caching Options definitions (mrsetcache) --------------
#       msc_mode
        .set    mscgoff,0x80            # Global Off
        .set    mscgon,0x81             # Global On
# --- Begin Set Caching Options request base structure ----------------
#       reserved 3
        .set    msc_mode,3              # Option                    <b>
# --- End base structure ----------------------------------------------
        .set    mscsiz,msc_mode+1       # Size of request
#
# --- Begin Set Caching Options response base structure ---------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mscrsiz,mr_rlen+4       # Response size
#
# --- Begin Set Server Properties definitions (mrserverprop) ----------
# --- Begin Set Server Properties request base structure --------------
        .set    mpr_sid,0               # Server ID to be changed   <s>
#       reserved 1
        .set    mpr_pri,mpr_sid+3       # Priority                  <b>
        .set    mpr_attrib,mpr_pri+1    # Server attributes         <w>
# --- End base structure ----------------------------------------------
        .set    mprsiz,mpr_attrib+4     # Size of request
#
# --- Begin Set Server Properties response base structure -------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mprrsiz,mr_rlen+4       # Response size
#
# --- Begin Reset NVRAM and clear devices definitions (mrreset) -------
#     mxn_options are also used by mcresetconfig
        .set    mxnall,0                # Clear everything
        .set    mxnfenva,1              # Clear FE NVA records
        .set    mxnnmi,2                # Clear NMI counts
        .set    mxnbenva,4              # Clear BE NVA records
# --- Begin Reset NVRAM and clear devices request base structure ------
        .set    mxn_options,0           # Options                   <w>
# --- End base structure ----------------------------------------------
        .set    mxnsiz,mxn_options+4    # Size of request
#
# --- Begin Reset NVRAM and clear devices response base structure -----
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mxnrsiz,mr_rlen+4       # Response size
#
# --- Begin Restore NVRAM definitions (mrrestore) ---------------------
        .set    mrnobdisk,0             # Option bit read from disk (c/l/i)
        .set    mrnobwwn,1              # Option bit read from disk (wwn)
        .set    mrnobfsys,2             # Option bit read from fsys
        .set    mrnobpci,3              # Option bit get from PCI
        .set    mrnobfid,4              # Option bit get from specific FID
        .set    mrnobrefresh,5          # Option bit refresh
        .set    mrnobrestore,6          # Option bit restore
        .set    mrnobreload,7           # Option bit reload (remove old config)
# --- Begin Restore NVRAM request base structure ----------------------
        .set    mrn_channel,0           # Channel to fetch NVRAM    <b>
        .set    mrn_opt,mrn_channel+1   # Options (disk or PCI)     <b>
        .set    mrn_lun,mrn_opt+1       # LUN                       <s>
        .set    mrn_addr,mrn_lun+2      # PCI address               <w>
        .set    mrn_id,mrn_addr         # ALIAS - FC ID
        .set    mrn_fid,mrn_addr        # ALIAS - FID
#       reserved 4
        .set    mrn_wwn,mrn_addr+8      # WWN                       <l>
# --- End base structure ----------------------------------------------
        .set    mrnsiz,mrn_wwn+8        # Size of request
#
# --- Begin Restore NVRAM response base structure ---------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mrnrsiz,mr_rlen+4       # Response size
#
# --- Begin Awake definitions (mrawake) -------------------------------
# --- Begin Awake request base structure ------------------------------
#       mpg_step values
        .set    mpgnvramrdy,1           # NVRAM can be read up
        .set    mpgmaster,2             # Make this controller master
        .set    mpgslave,4              # Make this controller slave
        .set    mpgccbreq,8             # CCB is required
        .set    mpgp2init,16            # P2 init may now run
        .set    mpgfeinit,32            # FE init may now run
        .set    mpgreplacement,64       # Replacement, load from FSYS
#       mpg_step bit positions
        .set    mpgbnvramrdy,0
        .set    mpgbmaster,1
        .set    mpgbslave,2
        .set    mpgbccbreq,3
        .set    mpgbp2init,4
        .set    mpgbfeinit,5
        .set    mpgbreplacement,6
        .set    mpg_step,0              # Step                      <s>
#       reserved 2                                                  <s>
# --- End base structure ----------------------------------------------
        .set    mpgsiz,mpg_step+4    # Size of request
#
# --- Begin Awake response base structure -----------------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mpgrsiz,mr_rlen+4       # Response size
#
# --- Begin WWN/LUN lookup definitions (mrwwnlookup) ------------------
#     mwl type constants
        .set    mwlnone,0               # No match found
        .set    mwldisk,1               # Disk drive
        .set    mwlmisc,2               # MIscellaneous device
        .set    mwlses,3                # SES enclosure
# --- Begin WWN/LUN lookup request base structure    ------------------
        .set    mwl_lun,0               # Logical unit number       <s>
#       reserved 6
        .set    mwl_wwn,mwl_lun+8       # WWN to look up            <l>
# --- End base structure ----------------------------------------------
        .set    mwlsiz,mwl_wwn+8        # Size of request
#
# --- Begin WWN/LUN lookup response base structure --------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
#       reserved 4
        .set    mwl_id,mr_rlen+8        # ID of device found        <s>
#       reserved 1
        .set    mwl_type,mwl_id+3       # Type of device found      <b>
# --- End base structure ----------------------------------------------
        .set    mwlrsiz,mwl_type+1      # Response size
#
# --- Begin generic MRP definitions (mrbegeneric) (mrfegeneric) -------
#
#     The size of the input and return data is undefined but the first
#     four words are defined here for easier coding.
#
# --- Begin generic request base structure    -------------------------
        .set    mge_parm0,0             # Parm 0                    <w>
        .set    mge_parm1,mge_parm0+4   # Parm 1                    <w>
        .set    mge_parm2,mge_parm1+4   # Parm 2                    <w>
        .set    mge_parm3,mge_parm2+4   # Parm 3                    <w>
# --- End base structure ----------------------------------------------
        .set    mgesiz,mge_parm3+4      # Size of request
#
# --- Begin generic response base structure ---------------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mge_rparm0,mr_rlen+4    # Return parm 0             <w>
        .set    mge_rparm1,mge_rparm0+4 # Return parm 0             <w>
        .set    mge_rparm2,mge_rparm1+4 # Return parm 0             <w>
        .set    mge_rparm3,mge_rparm2+4 # Return parm 0             <w>
# --- End base structure ----------------------------------------------
        .set    mgersiz,mge_rparm3+4    # Response size
#
# --- Begin Start/Stop device definitions (mrstartstop) ---------------
#     mss_dir
        .set    mssstop,0               # Stop
        .set    mssstart,1              # Start
# --- Begin Start/Stop device request base structure ------------------
        .set    mss_dir,0               # Start/stop                <b>
#       reserved 3
        .set    mss_channel,mss_dir+4   # Channel to start/stop     <b>
#       reserved 1
        .set    mss_lun,mss_channel+2   # LUN                       <s>
        .set    mss_id,mss_lun+2        # FC ID                     <w>
# --- End base structure ----------------------------------------------
        .set    msssiz,mss_id+4         # Size of request
#
# --- Begin Start/Stop device response base structure -----------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mssrsiz,mr_rlen+4       # Response size
#
# --- Begin Scrubbing/Scanning Control definitions (mrscrubctrl) ------
#       scrub control bit definitions
        .set    medenable,0             # Enable / Disable
        .set    medchange,31            # 1 = change request
                                        # 0 = return state info only
#       scan control bit definitions match scanning defs in rdd.inc
# --- Begin Scrubbing/Scanning Control request base structure ---------
        .set    med_scrub,0             # Scrubbing                 <w>
        .set    med_scan,med_scrub+4    # Scanning                  <w>
        .set    med_rid,med_scan+4      # Raid ID                   <s>
#       reserved 2                                                 2<b>
# --- End base structure ----------------------------------------------
        .set    medsiz,med_rid+4        # Size of request
#
# --- Begin Scrubbing/Scanning Control response base structure --------
#       reserved 3                                                 3<b>
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    med_rscrub,mr_rlen+4    # Scrub status              <w>
        .set    med_rscan,med_rscrub+4  # Scan status               <w>
        .set    med_passes,med_rscan+4  # Completed scan passes     <w>
        .set    med_scrubp,med_passes+4 # Current pid scrubbed      <s>
        .set    med_scanr,med_scrubp+2  # Current rid scanned       <s>
        .set    med_scrubb,med_scanr+2  # Current block scrubbed    <w>
        .set    med_scanb,med_scrubb+4  # Current block scanned     <w>
# --- End base structure ----------------------------------------------
        .set    medrsiz,med_scanb+4    # Response size
#
# --- Begin Set Default Options definitions (mrdefault) ---------------
#       msd_behav
        .set    msdnone,0               # No label
        .set    msddata,1               # Data label
        .set    msdhotspare,2           # Hotspare label
# --- Begin Set Default Options request base structure ----------------
        .set    msd_behav,0             # Behavior                  <b>
#       reserved 3
# --- End base structure ----------------------------------------------
        .set    msdsiz,msd_behav+4      # Size of request
#
# --- Begin Set Default Options response base structure ---------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    msdrsiz,mr_rlen+4       # Response size
#
# --- Begin Stop IO definitions (mrstopio 0x51C) ----------------------
#       mxi_op
        .set    mxiwait,0               # Wait for Write Cache Flush
                                        #  (ignored if mxinowaithost = 1 or
                                        #  mxinoflush = 1)
                                        #   0 = Do not wait for the Write
                                        #       Cache Flush to complete
                                        #   1 = Wait for the Write Cache Flush
                                        #       to complete
        .set    mxinoflush,1            # Flush Write Cache (ignored if
                                        #  mxinowaithost = 1)
                                        #   0 = Flush the Write Cache
                                        #   1 = Do not Flush the Write Cache
        .set    mxinobackground,2       # Background Flush (ignored if
                                        #  mxinowaithost = 1 or mxinoflush = 0)
                                        #   0 = Allow Background Flushing
                                        #   1 = Do not allow Background
                                        #       flushing - will wait for
                                        #       outstanding Write Cache ops
                                        #       to complete before returning
        .set    mxinowaithost,3         # Wait for Host I/O Completion
                                        #   0 = Wait for outstanding Host I/O
                                        #       to complete before returning
                                        #   1 = Do not wait for outstanding Host
                                        #       I/O to complete
#       mxi_intent
        .set    mxishutdown,0           # Shutdown - intended to show power off
                                        #  of the system is imminent (ignored if
                                        #  mxinowaithost = 1 or mxinoflush = 1)
                                        #   0 = Do not mark Write Cache as empty
                                        #   1 = Mark Write Cache as empty when
                                        #       the flush completes
#       mxi_user
        .set    mxigenuser,0            # 0 = Generic User (all Stop/Resumes
                                        #   must be paired)
                                        # 1-3F = PROC users
        .set    mxidefinefe,1           #   1 = Define FE
        .set    mxibattery,2            #   2 = Battery Health
        .set    mxiwcrecvry,3           #   3 = Write Cache Recovery
                                        #       Note: wcrcvry.c has constant for
                                        #   4-1F = Reserved for FE Stops
        .set    mxidefinebe,32          #  20 = Define BE
        .set    mxitgtctrl,33           #  21 = Target Control
        .set    mxiberebld,34           #  22 = BE Rebuild Update
                                        #  23-3F = Reserved for BE Stops
                                        # 40-7F = CCB users
                                        # 80-BF = XSSA users
                                        # C0-FF = Reserved
# --- Begin Stop IO request base structure ----------------------------
        .set    mxi_op,0                # Options Flag              <b>
        .set    mxi_intent,mxi_op+1     # Intent Flag               <b>
        .set    mxi_user,mxi_intent+1   # User ID                   <b>
#       reserved 1
# --- End base structure ----------------------------------------------
        .set    mxisiz,mxi_user+2       # Size of request
#
# --- Begin Stop IO response base structure ---------------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mxirsiz,mr_rlen+4       # Response size
#
# --- Begin Resume IO definitions (mrstartio 0x51D) -------------------
#       mri_op
        .set    mriclearone,0           # Clear only the last stop issued
        .set    mriclearall,1           # Clear all stop counts for this
                                        #  user (User ID > 0)
#       mri_user
        .set    mrigenuser,mxigenuser   # 0 = Generic User (all Stop/Resumes
                                        #   must be paired)
                                        # 1-3F = PROC users
        .set    mridefinefe,mxidefinefe #   1 = Define FE
        .set    mribattery,mxibattery   #   2 = Battery Health
        .set    mriwcrecvry,mxiwcrecvry #   3 = Write Cache Recovery
                                        #       Note: wcrcvry.c has constant for
                                        #   4-1F = Reserved for FE Stops
        .set    mridefinebe,mxidefinebe #  20 = Define BE
        .set    mritgtctrl,mxitgtctrl   #  21 = Target Control
        .set    mriberebld,mxiberebld   #  22 = BE Rebuild Update task
                                        #  23-3F = Reserved for BE Stops
                                        # 40-7F = CCB users
                                        # 80-BF = XSSA users
                                        # C0-FF = Reserved
# --- Begin Resume IO request base structure --------------------------
        .set    mri_op,0                # Options byte              <b>
#       reserved    1                                               <b>
        .set    mri_user,2              # User ID to continue       <b>
#       reserved    1                                               <b>
# --- End base structure ----------------------------------------------
        .set    mrisiz,mri_user+2       # Size of request
#
# --- Begin Resume IO response base structure -------------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mrirsiz,mr_rlen+4       # Response size
#
# --- Begin restore device definitions (mrrestoredev) -----------------
# --- Begin restore device request base structure ---------------------
        .set    mdr_pid,0               # PID                       <s>
        .set    mdr_opt,mdr_pid+2       # Options                   <s>
# --- End base structure ----------------------------------------------
        .set    mdrsiz,mdr_opt+2        # Size of request
#
# --- Begin restore device response base structure --------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mdrrsiz,mr_rlen+4       # Response size
#
# --- Begin Defragment device definitions (mrfragment) ----------------
# All in C in the MR_Defs.h file
        .set    mddsiz,16               # Must match sizeof struct from C
        .set    mddrsiz,24              # Must match sizeof struct from C
# --- Begin Defragment device request base structure ------------------
# --- End base structure ----------------------------------------------
#
# --- Begin Defragment device response base structure -----------------
# --- End base structure ----------------------------------------------
#
# --- Begin Attach Attribute definitions (mrsetattr) ------------------
# --- Begin Attach Attribute request base structure -------------------
        .set    maa_vid,0               # VDisk ID to be changed    <s>
        .set    maa_attr,maa_vid+2      # Attribute                 <s>
# --- End base structure ----------------------------------------------
        .set    maasiz,maa_attr+2       # Size of request
#
# --- Begin Attach Attribute response base structure ------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    maarsiz,mr_rlen+4       # Response size
#
# --- Begin Fetch Loop Information definitions (mrbeloop) -------------
# --- Begin Fetch Loop Information base structure ---------------------
        .set    mfl_port,0              # Port number               <b>
#       reserved 1
        .set    mfl_option,mfl_port+2   # Option                    <b>
#       reserved 1
# --- End base structure ----------------------------------------------
        .set    mflsiz,mfl_option+2     # Base size
#
# --- Begin Fetch Loop Information response base structure ------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mfl_numhosts,mr_rlen+4  # Number of hosts (FE only) <b>
        .set    mfl_lid,mfl_numhosts+1  # LID (FE only)             <b>
#       reserved 2
        .set    mfl_data,mfl_lid+3      # Start of data            6<w>
        .set    mfl_vendid,mfl_data+24  # Vendor ID of I/F card     <w>
        .set    mfl_model,mfl_vendid+4  # Model of I/F card         <w>
        .set    mfl_revlevel,mfl_model+4# Rev of I/F card           <s>
        .set    mfl_risclevel,mfl_revlevel+2# RISC of I/F card      <s>
        .set    mfl_fpmlevel,mfl_risclevel+2# FPM of I/F card       <b>
        .set    mfl_fplevel,mfl_fpmlevel+1# FP of I/F card          <b>
        .set    mfl_romlevel,mfl_fplevel+1# ROM level of I/F card   <s>
        .set    mfl_fwmajor,mfl_romlevel+2# FW level of I/F card    <s>
        .set    mfl_fwminor,mfl_fwmajor+2# FW level of I/F card     <s>
        .set    mfl_fwsub,mfl_fwminor+2 # FW level of I/F card      <s>
        .set    mfl_fwattrib,mfl_fwsub+2 # FW attribute             <s>
        .set    mfl_dataRate,mfl_fwattrib+2 # data rate             <s>
        .set    mfl_state,mfl_dataRate+2 # state                    <s>
        .set    mfl_numtarg,mfl_state+2 # Number of extensions      <b>
#       reserved 3
        .set    mfl_ext,mfl_numtarg+4   # Extensions for targets  *6<w>
# --- End base structure ----------------------------------------------
        .set    mflrsiz,mfl_ext         # Size of response (plus varible size)
#
# --- Begin Get Virtual Device Info definitions (mrgetvinfo) ----------
# --- Begin Get Virtual Device Info base structure --------------------
        .set    miv_vid,0               # Virtual device ID         <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mivsiz,miv_vid+4        # Base size
#
# --- Begin Get Virtual Device Info response base structure -----------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
#       Duplicate the information in the VDD up to the RAID list.
        .set    miv_vdata,mr_rlen+4     # Start virtual dev data   *<w>
# --- End base structure ----------------------------------------------
#        .set    mivrsiz,miv_vdata+96    # Size of response
#                                        # Add two bytes per RAID
#
# --- Begin Get RAID Device Info definitions (mrgetrinfo) -------------
# --- Begin Get RAID Device Info base structure -----------------------
        .set    mir_rid,0               # RAID device ID            <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mirsiz,mir_rid+4        # Base size
#
# --- Begin Get RAID Device Info response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
#       Duplicate the information in the RDD up to the PSD list.
        .set    mir_rdata,mr_rlen+4     # Start RAID dev data      *<w>
# --- extensions
        .set    mir_pid,0               # PID                       <s>
        .set    mir_pidstat,mir_pid+2   # PID status                <b>
        .set    mir_rpc,mir_pidstat+1   # Rebuild percentage cmplt  <b>
        .set    mir_pidastat,mir_rpc+1  # PID astatus               <b>
#       reserved 3
        .set    mirxsiz,mir_pidastat+4  # Size of extensions
# --- end of extension
# --- End base structure ----------------------------------------------
#        .set    mirrsiz,mir_rdata+112   # Size of response
#                                        # Add four bytes per PSD
#
# --- Begin Get Phys Device Info definitions (mrgetpinfo) -------------
#       mip_options (bit sensitive options)
        .set    miprefresh,0            # Refresh device status (physical devs)
        .set    miprlsels,1             # Do a RLS ELS before returning data
# --- Begin Get Phys Device Info base structure -----------------------
        .set    mip_pid,0               # Phys device ID            <s>
        .set    mip_options,mip_pid+2   # Options                   <b>
#       reserved 1                                                  <b>
# --- End base structure ----------------------------------------------
        .set    mipsiz,mip_options+2    # Base size
#
# --- Begin Get Phys Device Info response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
#       Duplicate the information in the PDD.
        .set    mip_pdata,mr_rlen+4     # Start phys dev data      *<w>
# --- End base structure ----------------------------------------------
        .set    miprsiz,mip_pdata+pddsiz # Size of response
#
# --- Begin Map LUN to VDisk definitions (mrmaplun)--------------------
# --- Begin Map LUN to VDisk base structure ---------------------------
#     options
        .set    mmlmap,0                # Regular map - nuthin fancy
        .set    mmlswap,1               # SID to DSID swap of mappings
        .set    mmlcopy,2               # SID to DSID copy (DSID mapping remain)
        .set    mmlmove,3               # SID to DSID copy (DSID mapping deleted)
#
        .set    mml_sid,0               # Server ID                 <s>
        .set    mml_option,mml_sid+2    # Options                   <s>
        .set    mml_lun,mml_option+2    # LUN                       <s>
        .set    mml_dsid,mml_lun+2      # Destination SID           <s>
        .set    mml_vid,mml_lun+4       # Virtual disk ID           <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mmlsiz,mml_vid+4        # Base size
#
# --- Begin Map LUN to VDisk response base structure ------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mmlrsiz,mr_rlen+4       # Size of response
#
# --- Begin Unmap LUN to VDisk definitions (mrunmaplun) ---------------
# --- Begin Unmap LUN to VDisk base structure -------------------------
        .set    mul_sid,0               # Server ID                 <s>
#       reserved 2
        .set    mul_lun,mml_sid+4       # LUN                       <s>
        .set    mul_vid,mml_lun+2       # VID                       <s>
# --- End base structure ----------------------------------------------
        .set    mulsiz,mul_vid+2        # Base size
#
# --- Begin Unmap LUN to VDisk response base structure ----------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mulrsiz,mr_rlen+4       # Size of response
#
# --- Begin Get SES Information definitions (mrgeteinfo) --------------
# --- This matches the get physical device information structure
# --- request since both use the PDD as the base structure.
# --- End base structure ----------------------------------------------
#
# --- Begin Create Server definitions (mrcreateserver) ----------------
# --- Begin Create Server base structure ------------------------------
        .set    mms_tid,0               # Target ID number          <s>
#       reserved 2
        .set    mms_owner,mms_tid+4     # Owner                     <w>
        .set    mms_wwn,mms_owner+4     # FC WWN                    <l>
        .set    mms_name,mms_wwn+8      # iSCSI Server Name      <256b>
# --- End base structure ----------------------------------------------
        .set    mmssiz,mms_name+256     # Base size
#
# --- Begin Create server response base structure ---------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mms_sid,mr_rlen+4       # Server ID created         <s>
        .set    mms_flag,mms_sid+2      # Flags                     <s>
# --- End base structure ----------------------------------------------
        .set    mmsrsiz,mms_flag+2      # Size of response
#
# --- Begin Delete Server definitions (mrdeleteserver) ----------------
# --- Begin Delete Server base structure ------------------------------
#     option values
        .set    mdsdelete,0             # Delete the SDD when done
        .set    mdsmappings,1           # Only delete the mappings
#
        .set    mds_sid,0               # Server ID                 <s>
        .set    mds_option,mds_sid+2    # Options                   <s>
# --- End base structure ----------------------------------------------
        .set    mdssiz,mds_option+2     # Base size
#
# --- Begin Delete server response base structure ---------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mdsrsiz,mr_rlen+4       # Size of response
#
# --- Begin Get Misc Information definitions (mrgetminfo) -------------
# --- This matches the get physical device information structure
# --- request since both use the PDD as the base structure.
# --- End base structure ----------------------------------------------
#
# --- Begin Virtual Disk Control definitions (mrvdiskcontrol) ---------
#     mvc_subtype
        .set    mvcmovevd,0             # Move virtual device
        .set    mvccopybrk,1            # Start copy and break
        .set    mvccopyswap,2           # Start copy and swap
        .set    mvccopycont,3           # Start continuous copy
        .set    mvcswapvd,4             # Test function (swap vdisks)
        .set    mvcxspeccopy,5          # Break off specified copy
        .set    mvcpausecopy,6          # Pause a copy
        .set    mvcresumecopy,7         # Resume a copy
        .set    mvcabortcopy,8          # Abort a copy
        .set    mvcupdate,9             # Update the status
        .set    mvcslink,10             # create slink
# --- Begin Virtual Disk Control base structure -----------------------
        .set    mvc_subtype,0           # Subtype                   <b>
#       reserved 3
        .set    mvc_svid,mvc_subtype+4  # Source VID                <s>
#       reserved 2
        .set    mvc_dvid,mvc_svid+4     # Destination VID           <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mvcsiz,mvc_dvid+4       # Base size
#
# --- Begin Virtual Disk Control response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mr_mirrorsettype,mr_rlen+4 # Partner Type             <b>
#       reserved 3
        .set    mvcrsiz,mr_mirrorsettype+4 # Size of response
#
# --- Begin Assign System Serial definitions (mrassignsysinfo) --------
        .set    maspoll,0               # Just return the serial number
        .set    masserset,1             # Set system serial number
        .set    masipset,2              # Set IP address
        .set    masclearmp,4            # Clear mirror partner for serial number
        .set    mascserset,8            # Set the controller serial number
# --- Begin Assign System Serial base structure -----------------------
        .set    mas_op,0                # Operation requested       <b>
#       reserved 3
        .set    mas_serial,mas_op+4     # Serial number             <w>
        .set    mas_ipaddr,mas_serial+4 # IP address                <w>
# --- End base structure ----------------------------------------------
        .set    massiz,mas_ipaddr+4     # Base size
#
# --- Begin Assign System Serial response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mas_rvcgid,mr_rlen+4    # Current virtual ctlr group<w>
        .set    mas_rcserial,mas_rvcgid+4# Current ctlr serial      <w>
        .set    mas_ripaddr,mas_rcserial+4 # IP address             <w>
        .set    mas_mp,mas_ripaddr+4    # Mirror partner            <w>
# --- End base structure ----------------------------------------------
        .set    masrsiz,mas_mp+4        # Size of response
#
# --- Begin Get II information definitions (mrbeii) -------------------
# --- Begin Get II information base structure -------------------------
# --- End base structure ----------------------------------------------
        .set    mgisiz,0                # Base size
#
# --- Begin Get II information response base structure ----------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mgi_ii,mr_rlen+4        # II information           *<w>
# --- End base structure ----------------------------------------------
        .set    mgirsiz,mgi_ii+iisiz    # Size of response
#
# --- Begin Get link information definitions (mrbelink) ---------------
# --- Begin Get link information base structure -----------------------
# --- End base structure ----------------------------------------------
        .set    mfpsiz,0                # Base size
#
# --- Begin Get link information response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mfp_data,mr_rlen+4      # LInk information         *<w>
# --- End base structure ----------------------------------------------
        .set    mfprsiz,mfp_data+lssize # Size of response
#
# --- Begin Configure Target information (mrconfigtarg) ---------------
        .set    mctmodport,0            # Modify port
        .set    mctmodopt,1             # Modify hard/soft option
        .set    mctmodlock,2            # Modify lock flag
        .set    mctmodown,3             # Modify controller (owner)
        .set    mctmodcluster,4         # Modify cluster
# --- Begin Configure Target information base structure ---------------
        .set    mct_tid,0               # Target ID (0xffff=new)    <s>
        .set    mct_chan,mct_tid+2      # port number               <b>
        .set    mct_opt,mct_chan+1      # Options                   <b>
        .set    mct_fcid,mct_opt+1      # FC ID                     <b>
#       reserved 1
        .set    mct_lock,mct_fcid+2     # Locked target indicator   <b>
#       reserved 1
        .set    mct_pname,mct_lock+2    # Port WWN                  <l>
        .set    mct_nname,mct_pname+8   # Node WWN                  <l>
        .set    mct_powner,mct_nname+8  # Previous owner            <w>
        .set    mct_owner,mct_powner+4  # Owner ctrl serial number  <w>
        .set    mct_cluster,mct_owner+4 # Clustered target number   <s>
#       reserved 3
        .set    mct_aport,mct_cluster+5 # Alternate port            <b>
#       reserved 6
        .set    mct_mod,mct_aport+7      # Field Modifier           <w>
# --- End base structure ----------------------------------------------
        .set    mctsiz,mct_mod+4        # Base size
#
# --- Begin Configure Target response base structure ------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mctrsiz,mr_rlen+4       # Size of response
#
# --- Begin Get Target information (mrgettarg) ------------------------
# --- Begin Get Target information base structure ---------------------
        .set    mgt_tid,0               # Target ID                 <s>
#       reserved 3
# --- End base structure ----------------------------------------------
        .set    mgtsiz,mgt_tid+4        # Base size
#
# --- Begin Get Target response base structure ------------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mgt_rtid,mr_rlen+4      # Target ID                 <s>
        .set    mgt_chan,mgt_rtid+2     # port number               <b>
        .set    mgt_opt,mgt_chan+1      # Options                   <b>
        .set    mgt_fcid,mgt_opt+1      # FC ID                     <b>
#       reserved 1
        .set    mgt_lock,mgt_fcid+2     # Locked target indicator   <b>
#       reserved 1
        .set    mgt_pname,mgt_lock+2    # Port WWN                  <l>
        .set    mgt_nname,mgt_pname+8   # Node WWN                  <l>
        .set    mgt_powner,mgt_nname+8  # Previous owner            <w>
        .set    mgt_owner,mgt_powner+4  # Owner ctrl serial number  <w>
        .set    mgt_cluster,mgt_owner+4 # Clustered target number   <s>
#       reserved 2
        .set    mgt_pport,mgt_cluster+4 # preferred port            <b>
        .set    mgt_aport,mgt_pport+1   # alternate port            <b>
#       reserved 10
# --- End base structure ----------------------------------------------
        .set    mgtrsiz,mgt_aport+11    # Size of response
#
# --- Begin Set Global Priority information (mrglobalpri) -------------
        .set    mbpmaxpri,7             # Maximum value allowed
# --- Begin Set Global Priority information base structure ------------
        .set    mbp_priority,0          # New priority              <b>
#       reserved 3
# --- End base structure ----------------------------------------------
        .set    mbpsiz,mbp_priority+4   # Base size
#
# --- Begin Set Global Priority response base structure ---------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mbprsiz,mr_rlen+4       # Size of response
#
# --- Begin Get firmware header information (mrbeboot, mrbediag, mrbeproc)
# --- Begin Get firmware header information base structure ------------
# --- End base structure ----------------------------------------------
        .set    mfhsiz,0                # Base size
#
# --- Begin Get firmware header information response base structure ---
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mfh_data,mr_rlen+4      # header information       *<b>
# --- End base structure ----------------------------------------------
        .set    mfhrsiz,mfh_data+fh_size # Size of response
#
# --- Begin Get number of remote controllers definitions (mrremotectrlcnt)
# --- Begin Get number of remote controllers base structure -----------
# --- End base structure ----------------------------------------------
        .set    mncsiz,0                # Base size
#
# --- Begin Get number of remote controllers response base structure --
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mnc_count,mr_rlen+4     # Number of remote ctrl     <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mncrsiz,mnc_count+4     # Size of response
#
# --- Begin Get remote controller info definitions (mrremotectrlinfo) -
# --- Begin Get remote controller info base structure -----------------
        .set    mci_cid,0               # Controller ID             <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mcisiz,mci_cid+4        # Base size
#
# --- Begin Get remote controller info response base structure --------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mci_wwn,mr_rlen+4       # World wide name of ctrl   <l>
        .set    mci_cname,mci_wwn+8     # Controller name          5<w>
        .set    mci_luns,mci_cname+20   # Number of LUNs            <b>
        .set    mci_sctype,mci_luns+1   # Storage controller type   <b>
        .set    mci_cluster,mci_sctype+1# Mag cluster number        <b>
#       reserved 1
        .set    mci_ipaddr,mci_cluster+2# CCB IP address            <w>
        .set    mci_csn,mci_ipaddr+4    # Controller serial number  <w>
# --- End base structure ----------------------------------------------
        .set    mcirsiz,mci_csn+4       # Size of response
#
# --- Begin Get remote virtual disk info definitions (mrremotevdiskinfo)
# --- Begin Get remote virtual disk info base structure ---------------
        .set    mvi_cid,0               # Controller ID             <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mvisiz,mvi_cid+4        # Base size
#
# --- Begin Get remote virtual disk info response base structure ------
        .set    mvi_cnt,0               # Number of entries in list <b>
#       reserved 2
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mvi_data,mr_rlen+4      # Start of data
# --- End base structure ----------------------------------------------
#
        .set    mvi_lun,0               # LUN                       <s>
        .set    mvi_rtype,mvi_lun+2     # RAID type                 <b>
        .set    mvi_cluster,mvi_rtype+1 # Cluster number            <b>
        .set    mvi_attr,mvi_cluster+1  # Attributes                <b>
#       reserved 3
        .set    mvi_devcap,mvi_attr+4   # Device capacity           <l>
        .set    mvi_vcgid,mvi_devcap+8  # Virtual controller group  <w>
        .set    mvi_vid,mvi_vcgid+4     # VID                       <s>
        .set    mvi_rsvd,mvi_vid+2      # VID (copy)                <s>
        .set    mvi_scnt,mvi_rsvd+2     # Server count              <s>
        .set    mvi_vlcnt,mvi_scnt+2    # Vlink count               <s>
#       reserved 4
        .set    mvi_vdname,mvi_vlcnt+6  # Virtual device name     13<w>
#       reserved 12
        .set    mvixsiz,mvi_vdname+64   # Extension size
        .set    mvirsiz,mvi_data+(MAXTARGETS*mvixsiz) # Total size
#
# --- Begin Set foreign target enable definitions (mrforeigntargets) --
# --- Begin Set foreign target enable base structure ------------------
        .set    mft_bmap,0              # Controller enable map     <b>
#       reserved b
# --- End base structure ----------------------------------------------
        .set    mftsiz,mft_bmap+4       # Base size
#
# --- Begin Set foreign target enable response base structure ---------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mftrsiz,mr_rlen+4       # Size of response
#
# --- Begin Create virtual link definitions (mrcreatevlink) -----------
# --- Begin Create virtual link base structure ------------------------
        .set    mcv_cid,0               # Controller ID             <s>
        .set    mcv_vordinal,mcv_cid+2  # Virtual ordinal           <b>
#       reserved 1
        .set    mcv_rvid,mcv_vordinal+2 # VID to use for VLink VDD  <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mcvsiz,mcv_rvid+4       # Base size
#
# --- Begin Create virtual link response base structure ---------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mcv_vid,mr_rlen+4       # Virtual disk ID created   <s>
#       reserved 2
        .set    mcv_ctrlname,mcv_vid+4  # Name of controller       5<w>
        .set    mcv_vdname,mcv_ctrlname+20# Name of vdisk         13<w>
# --- End base structure ----------------------------------------------
        .set    mcvrsiz,mcv_vdname+52    # Size of response
#
# --- Begin Get virtual link info definitions (mrvlinkinfo) -----------
# --- Begin Get virtual link info base structure ----------------------
        .set    mvl_vid,0               # Virtual disk ID           <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mvlsiz,mvl_vid+4        # Base size
#
# --- Begin Get virtual link info response base structure -------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mvl_data,mr_rlen+4      # Virtual disk data         <?>
# --- End base structure ----------------------------------------------
        .set    mvlrsiz,mvl_data+336    # Size of response
#
# --- Begin Create Controller definitions (mrcreatectrlr) -------------
# --- Begin Create Controller base structure --------------------------
        .set    mmc_cserial,0           # Controller serial number  <w>
# --- End base structure ----------------------------------------------
        .set    mmcsiz,mmc_cserial+4    # Base size
#
# --- Begin Create Controller response base structure -----------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mmcrsiz,mr_rlen+4       # Size of response
#
# --- Begin Rescan Device definitions (mrrescandevice) ----------------
        .set    mrdexisting,0           # Rescan existing devices
        .set    mrdluns,1               # Rescan LUNS
        .set    mrdrediscover,2         # Rediscover devices
#
        .set    mrd_type,0              # Scan type                 <b>
#       reserved 3                                                 3<b>
        .set    mrdsiz,mrd_type+4       # Base size
# --- Begin Rescan Device response base structure -----------------
        .set    mrdrsiz,mrrsiz          # Size of response
#
# --- Begin Resync RAID definitions (mrresync) ------------------------
#     mrb_type definitions
        .set    mrbstripe,1             # Sync stripes given NVA table
        .set    mrboneraid,2            # Sync given RAID ID
        .set    mrballraids,3           # Sync all RAIDs on this controller
        .set    mrballnotmirror,4       # Sync all RAIDs in the "Not Mirroring"
                                        #  state from a different controller
                                        #  now located on this controller
        .set    mrblistraids,5          # Sync all RAIDs in the given list
# --- Begin command base structure ------------------------------------
        .set    mrb_type,0              # Type of rebuild             <b>
#       reserved 1                                                    <b>
        .set    mrb_raidid,mrb_type+2   # RAID ID to resync           <s>
        .set    mrb_numrids,mrb_raidid  #  Alias Number of rids in list
        .set    mrb_nva,mrb_raidid+2    # Addr of NVRAM P4 NVA buffer <w>
        .set    mrb_ridlist,mrb_nva     #  Alias Address of RID List
# --- End base structure ----------------------------------------------
        .set    mrbsiz,mrb_nva+4        # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mrbrsiz,mr_rlen+4       # Size of response
#
# --- Begin Get Local Image definitions (mrgetlclimage) ---------------
# --- Begin Get Local Image definitions -------------------------------
        .set    mgn_len,0               # Length allocated to image   <w>
        .set    mgn_addr,mgn_len+4      # Local in which to put image <w>
# --- End base structure ----------------------------------------------
        .set    mgnsiz,mgn_addr+4       # Base size
#
# --- Begin Get Local Image response base structure -------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
        .set    mgn_rlen,mr_rlen+4      # Data length of lcl image    <w>
# --- End base structure ----------------------------------------------
        .set    mgnrsiz,mgn_rlen+4      # Size of response
#
# --- Begin Update Local Image definitions (mrputlclimage) ----------
# --- Begin Update Local Image definitions --------------------------
        .set    mun_addr,0              # Local to grab image from    <w>
# --- End base structure ----------------------------------------------
        .set    munsiz,mun_addr+4       # Base size
#
# --- Begin Update Local Image response base structure ---------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    munrsiz,mr_rlen+4       # Size of response
#
# --- Begin Delete Device definitions (mrdeletedevice) ----------------
#     Type values
        .set    mxdtdisk,0              # Disk drive
        .set    mxdtses,1               # Enclosure
# --- Begin Update Local Image definitions ----------------------------
        .set    mxd_type,0              # Local to grab image from    <b>
#       reserved 1
        .set    mxd_pid,mxd_type+2      # ID of device                <s>
# --- End base structure ----------------------------------------------
        .set    mxdsiz,mxd_pid+2        # Base size
#
# --- Begin Delete Device response base structure ---------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mxdrsiz,mr_rlen+4       # Size of response
#
# --- Begin Mode Page definitions (mrbemodepage, mrfemodepage) --------
#     Word 0 bit definitions
        .set    mmp0dsblheartbeat,0     # Disable heartbeat monitor
        .set    mmp0disbooterrtrap,1    # Disable errtrap handling by boot code
        .set    mmp0ctrlshutdown,2      # Controller shutting down, completely
                                        #   ignore CCB heartbeat failure
# --- Begin command base structure ------------------------------------
        .set    mmp_data0,0             # Word 0 data                 <w>
        .set    mmp_data1,mmp_data0+4   # Word 1 data                 <w>
        .set    mmp_data2,mmp_data1+4   # Word 2 data                 <w>
        .set    mmp_data3,mmp_data2+4   # Word 3 data                 <w>
        .set    mmp_mask0,mmp_data3+4   # Word 0 mask                 <w>
        .set    mmp_mask1,mmp_mask0+4   # Word 1 mask                 <w>
        .set    mmp_mask2,mmp_mask1+4   # Word 2 mask                 <w>
        .set    mmp_mask3,mmp_mask2+4   # Word 3 mask                 <w>
# --- End base structure ----------------------------------------------
        .set    mmpsiz,mmp_mask3+4      # Base size
#
# --- Begin response base structure ---------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mmprsiz,mr_rlen+4       # Size of response
#
# --- Begin Device Count definitions (mrdevicecount) --------------------
# --- Begin Device Count definitions ------------------------------------
        .set    mdc_serial,0            # Search serial number        <w>
# --- End base structure ------------------------------------------------
        .set    mdcsiz,mdc_serial+4     # Base size
#
# --- Begin Device Count response base structure ---   ------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
        .set    mdc_count,mr_rlen+4     # Device count                <w>
# --- End base structure ----------------------------------------------
        .set    mdcrsiz,mdc_count+4     # Size of response
#
# --- Begin Hotspare information definitions (mrhotspareinfo) ---------
# --- Begin base structure --------------------------------------------
        .set    mrh_pid,0               # PID                         <s>
#       reserved 2                                                    <s>
# --- End base structure ----------------------------------------------
        .set    mrhsiz,mrh_pid+4        # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
        .set    mrh_cap,mr_rlen+4       # Required capacity           <l>
# --- End base structure ----------------------------------------------
        .set    mrhrsiz,mrh_cap+8       # Size of response
#
# --- Begin File system file to file copy definitions (mrfilecopy) ----
# --- Begin base structure --------------------------------------------
        .set    mfc_sfid,0              # File ID of source file      <s>
        .set    mfc_soffset,mfc_sfid+2  # Block offset in source      <s>
        .set    mfc_slen,mfc_soffset+2  # Length of copy              <s>
        .set    mfc_dfid,mfc_slen+2     # File ID of destination file <s>
        .set    mfc_doffset,mfc_dfid+2  # Block offset into dest.     <s>
#       reserved 2                                                    <s>
# --- End base structure ----------------------------------------------
        .set    mfcsiz,mfc_doffset+4    # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
        .set    mfc_good,mr_rlen+4      # How many good writes        <w>
        .set    mfc_error,mfc_good+4    # How many bad writes         <w>
        .set    mfc_goodmap,mfc_error+4 # Bit map of good writes     *<w>
# --- End base structure ----------------------------------------------
        .set    mfcrsiz,mfc_goodmap+((MAXDRIVES+7)/8)   # Response size
#
# --- Begin Get SOS structure definitions (mrgetsos) ------------------
# --- Begin base structure --------------------------------------------
        .set    mes_pid,0               # PID for the SOS table       <s>
#       reserved 2                                                    <s>
# --- End base structure ----------------------------------------------
        .set    messiz,mes_pid+4        # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
#       reserved 8
        .set    mes_data,mr_rlen+12     # SOS structure              *<q>
        .set    mes_soshdr,mes_data     # SOS header start area
        .set    mes_sos,mes_soshdr+32   # Two quads of header
# --- End base structure ----------------------------------------------
        .set    mesrsiz,mes_sos+(16*MAXRAIDS)      # Size of response
#
# --- Begin Put Fsysreport structure definitions (mrputfsys) ----------
# --- Begin base structure --------------------------------------------
        .set    mpf_fsys,0              # Fsys report               32<b>
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mpfrsiz,mr_rlen+4       # Size of response
#
# --- Begin FCAL analysis definitions (mrfcalanal) --------------------
#       mfa_options bit definitions
        .set    mfawrite,       0       # Write or read
        .set    mfacontinue,    1       # Continue on error
# --- Begin base structure --------------------------------------------
        .set    mfa_options,0           # Option bits                <b>
#       reserved 1                                                   <b>
        .set    mfa_pid,mfa_options+2   # PID                        <s>
        .set    mfa_xsize,mfa_pid+2     # Transfer size in bytes     <w>
        .set    mfa_xcount,mfa_xsize+4  # Number of transfers        <w>
        .set    mfa_spare1,mfa_xcount+4 # Spare                      <w>
# --- End base structure ----------------------------------------------
        .set    mfasiz,mfa_spare1+4     # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
        .set    mfa_good,mr_rlen+4      # Good transfer count         <w>
        .set    mfa_rspare,mfa_good+4   # Spare response              <w>
# --- End base structure ----------------------------------------------
        .set    mfarsiz,mfa_rspare+4    # Size of response
#
# --- Begin Put SOS structure definitions (mrputsos) ------------------
# --- Begin base structure --------------------------------------------
        .set    mps_sos,0               # SOS entry to update       32<b>
# --- End base structure ----------------------------------------------
        .set    mpssiz,mps_sos+32       # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mpsrsiz,mr_rlen+4       # Size of response
#
# --- Begin Force Error Trap structure definitions (mrforcebeetrap & mrforcefeetrap) ------
# --- Begin base structure --------------------------------------------
# --- End base structure ----------------------------------------------
        .set    metsiz,0                # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    metrsiz,mr_rlen+4       # Size of response
#
# --- Begin Put SCMT structure definitions (mrputscmt) -----------------
# --- Begin base structure --------------------------------------------
        .set    mpc_scmt,0              # SCMT entry to update      52<b>
# --- End base structure ----------------------------------------------
        .set    mpcsiz,mpc_scmt+52      # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mpcrsiz,mr_rlen+4       # Size of response
#
# --- Begin Put LDD structure definitions (mrputldd) -------------------
# --- Begin base structure --------------------------------------------
        .set    mpl_ldd,0               # LDD entry to update        <w>
# --- End base structure ----------------------------------------------
        .set    mplsiz,mpl_ldd+lddsiz   # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mplrsiz,mr_rlen+4       # Size of response
#
# --- Begin ebiseDG structure definitions (mrebiseDG) -----------------
# --- Begin base structure --------------------------------------------
        .set    mDG_order,4             # order                       <b>
        .set    mDG_func,mDG_order+1    # Function                    <b>
        .set    mDG_MirrorTyp,mDG_func+1   # Mirror Type              <b>
        .set    mDG_scpcomp,mDG_MirrorTyp  # percent complete     (alias)
        .set    mDG_Mstate,mDG_MirrorTyp   # mirror state         (alias)
        .set    mDG_CpyTyp,mDG_MirrorTyp+1 # Copy Type                <b>
        .set    mDG_rqctrlsn,mDG_CpyTyp+1  # requesting controller s/n<w>
        .set    mDG_rid,mDG_rqctrlsn+4  # copy reg. ID                <w>
        .set    mDG_rcsn,mDG_rid+4      # copy reg. MAG SN  (vcg s/n) <w>
        .set    mDG_rcscl,mDG_rcsn+4    # copy reg. MAG src cl #      <b>
        .set    mDG_rcsvd,mDG_rcscl+1   # copy reg. MAG src Vdisk #   <b>
        .set    mDG_rcsvid,mDG_rcscl    # copy reg. MAG src VID   (alias)
        .set    mDG_rcdcl,mDG_rcsvd+1   # copy reg. MAG dst cl #      <b>
        .set    mDG_rcdvd,mDG_rcdcl+1   # copy reg. MAG dst Vdisk #   <b>
        .set    mDG_rcdvid,mDG_rcdcl    # copy reg. MAG dst VID   (alias)
        .set    mDG_rssn,mDG_rcdvd+1    # copy reg. src MAG SN        <w>
        .set    mDG_rdsn,mDG_rssn+4     # copy reg. dst MAG SN        <w>
        .set    mDG_rscl,mDG_rdsn+4     # copy reg. src MAG cl #      <b>
        .set    mDG_rsvd,mDG_rscl+1     # copy reg. src MAG Vdisk #   <b>
        .set    mDG_rdcl,mDG_rsvd+1     # copy reg. dst MAG cl #      <b>
        .set    mDG_rdvd,mDG_rdcl+1     # copy reg. dst MAG Vdisk #   <b>
# --- End base structure ----------------------------------------------
        .set    mDGsiz,mDG_rdvd+1       # Base size
#
# --- ebiseDG order definitions
        .set    mDGodr_NVRMrefresh,1    # request NVRAM refresh
        .set    mDGodr_ProcessPCP,2     # process PCP
        .set    mDGodr_UpdateCopy,3     # Update Copy Percent Complete
        .set    mDGodr_StartCopy,4      # Start Copy
#
        .set    mDGodr_ActCpyTsk,8      # Activate Copy Task
        .set    mDGodr_SpnCpyTsk,9      # Suspend Copy Task
#
        .set    mDG_maxorder,16         # Max order
#
# --- ebiseDG function definitions
#     UpdateCopy
        .set    mDGfnc_Percent,1        # update percent complete
        .set    mDGfnc_Mirror,2         # update mirror state
        .set    mDGfnc_SetAttr,3        # set VDD attributes
        .set    mDGfnc_ClrAttr,4        # clear VDD attributes
        .set    mDGfnc_NewOwner,8       # set new owner
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mDGrsiz,mr_rlen+4       # Size of response
#
# --- Begin Loop Primitive structure definitions ----------------------
#     (mrfeloopprimitive and mrbeloopprimitive)
# --- Begin base structure --------------------------------------------
#       mlp_option constants
        .set    mlpresloop,0            # LIP Reset an entire loop
        .set    mlpreslidport,1         # LIP Reset a LID port
        .set    mlpsidpidres,2          # FE: SID LIP Reset, BE: PID LIP Reset
        .set    mlploginlid,0x11        # Login a port specified by a LID
        .set    mlploginpid,0x12        # Login a port specified by a PID
        .set    mlplogoutlid,0x21       # Logout a port specified by a LID
        .set    mlplogoutpid,0x22       # Logout a port specified by a PID
#
        .set    mlp_option,0             # Option                      <s>
        .set    mlp_sid_pid,mlp_option+2 # Pid to use                  <s>
        .set    mlp_port,mlp_sid_pid+2   # Port to use                 <b>
#       reserved 3                                                    3<b>
        .set    mlp_lid,mlp_port+4       # Lid to use                  <w>
# --- End base structure ----------------------------------------------
        .set    mlpsiz,mlp_lid+4        # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mlprsiz,mr_rlen+4       # Size of response
#
# --- Begin failover structure definitions (mrtargetcontrol) ----------
#       mfo_option definitions
        .set    mtcprepmove,0           # Prepare for a target move
        .set    mtccompmove,1           # Target move complete
# --- Begin base structure --------------------------------------------
        .set    mtc_option,0            # Option                      <s>
#       reserved 2                                                    <s>
# --- End base structure ----------------------------------------------
        .set    mtcsiz,mtc_option+4     # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mtcrsiz,mr_rlen+4       # Size of response
#
# --- Begin set device name structure definitions (mrnamedevice) ------
#       mnd_option definitions
        .set    mndserver,0             # Name a server
        .set    mndvdisk,1              # Name a vdisk
        .set    mndvcg,2                # Name a VCG
        .set    mndretvcg,3             # Get VCG name
# --- Begin base structure --------------------------------------------
        .set    mnd_option,0            # Option                      <s>
        .set    mnd_id,mnd_option+2     # ID for vdisk, server        <s>
#       reserved 12                                                 12<b>
        .set    mnd_name,mnd_id+14      # Name field                16<b>
# --- End base structure ----------------------------------------------
        .set    mndsiz,mnd_name+16      # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
        .set    mnd_retname,mr_rlen+4   # Returned name               <q>
# --- End base structure ----------------------------------------------
        .set    mndrsiz,mnd_retname+16  # Size of response
#
# --- Begin Break VLink Lock definitions (mrbreakvlock) ---------------
# --- Begin base structure --------------------------------------------
        .set    mbv_vid,0               # VID for the lock check      <s>
#       reserved 2                                                    <s>
# --- End base structure ----------------------------------------------
        .set    mbvsiz,mbv_vid+4        # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mbvrsiz,mr_rlen+4       # Size of response
#
# --- Begin Get DLink Information definitions (mrgetdlink) ------------
# --- Begin base structure --------------------------------------------
        .set    mni_did,0               # RID for DLink information   <s>
#       reserved 2                                                    <s>
# --- End base structure ----------------------------------------------
        .set    mnisiz,mni_did+4        # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
        .set    mni_id,mr_rlen+4        # RID for DLink info          <s>
        .set    mni_status,mni_id+2     # Status byte                 <b>
                                        # 00 = unused
                                        # 01 = operational
                                        # 02 = inoperative
                                        # 03 = missing
        .set    mni_type,mni_status+1   # Type of link                <b>
                                        # 00 = XIOtech node
                                        # 01 = foreign target
# --- Reserved 1                                                      <b>
        .set    mni_cap,mni_type+2      # Device capacity             <w>
        .set    mni_vname,mni_cap+4     # Name of VDisk              8<b>
        .set    mni_rsn,mni_vname+8     # Remote serial #             <w>
        .set    mni_rcl,mni_rsn+4       # Remote cluster/Vport #      <b>
        .set    mni_rvid,mni_rcl+1      # Remote VDisk/LUN #          <b>
        .set    mni_rhab,mni_rvid+1     # Remote HAB #                <b>
        .set    mni_con,mni_rhab+1      # Number of connections       <b>
        .set    mni_nwwn,mni_con+1      # Node WWN                   8<b>
        .set    mni_pwwn1,mni_nwwn+8    # First port WWN             8<b>
        .set    mni_pwwn2,mni_pwwn1+8   # Second port WWN            8<b>
        .set    mni_bsn,mni_pwwn2+8     # Base serial #               <w>
        .set    mni_bcl,mni_bsn+4       # Base cluster/Vport #        <b>
        .set    mni_bvid,mni_bcl+1      # Base VDisk/LUN #            <b>
# --- Reserved 10                                                   10<b>
        .set    mni_pid,mni_bvid+11     # Product ID                16<b>
        .set    mni_vid,mni_pid+16      # Vendor ID                  8<b>
        .set    mni_nname,mni_vid+8     # Node name                  8<b>
        .set    mni_rev,mni_nname+8     # Product revision           4<b>
        .set    mni_serial,mni_rev+4    # Serial # of device        12<b>
        .set    mnirsiz,mni_serial+12   # Size of response
        .set    mnirsizGT2TB,mnirsiz+4  # Size of response for GT2TB version (capacity UINT64)
# --- End base structure ----------------------------------------------
#
# --- Begin Get DLock Information definitions (mrgetdlock) ------------
# --- Begin base structure --------------------------------------------
        .set    mki_vid,0               # VID for DLock information   <s>
#       reserved 2                                                    <s>
# --- End base structure ----------------------------------------------
        .set    mkisiz,mki_vid+4        # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
        .set    mki_id,mr_rlen+4        # VID for DLock info          <s>
        .set    mki_sn,mki_id+2         # Lock serial #               <w>
        .set    mki_cl,mki_sn+4         # Lock cluster/Vblock #       <b>
        .set    mki_vd,mki_cl+1         # Lock VDisk #                <b>
# --- Reserved 2                                                     2<b>
        .set    mki_vname,mki_vd+3      # VDisk name                 8<b>
        .set    mki_nname,mki_vname+8   # Node name                  8<b>
        .set    mkirsiz,mki_nname+8     # Size of response
# --- End base structure ----------------------------------------------
#
# --- Begin Set Controller S/N definitions (mrsetcntlsnfe) ------------
# --- Begin base structure --------------------------------------------
        .set    mcs_cntlsn,0            # Controller S/N to change to <w>
# --- End base structure ----------------------------------------------
        .set    mcssiz,mcs_cntlsn+4     # Base size
#
# --- Begin response base structure -----------------------------------
#       reserved 3                                                   3<b>
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
# --- End base structure ----------------------------------------------
        .set    mcsrsiz,mr_rlen+4       # Size of response
#
# --- Begin MM Info definitions (mmminfo) -----------------------------
# --- Begin MM Info base structure ------------------------------------
        .set    mmm_pmminfo,0           # pointer to MM Info struct <w>
# --- End base structure ----------------------------------------------
        .set    mmmsiz,mmm_pmminfo+4    # Base size
#
# --- Begin MM Info response base structure ---------------------------
#       reserved 3                                                 3<b>
#       .set    mr_status,mr_status     # Response status           <b>
#       .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mmmrsiz,mr_rlen+4       # Size of response
#
# --- Begin Report Server Config definitions (mrreportsconfig) --------
# --- Begin Report Server Config base structure -----------------------
        .set    mrs_sid,0               # Server ID                 <s>
        .set    mrs_del,mrs_sid+2       # Delete this server        <b>
        .set    mrs_continue,mrs_del+1  # Continue flag             <b>
        .set    mrs_lsid,mrs_continue+1 # Linked SID                <s>
        .set    mrs_nluns,mrs_lsid+2    # Number of LUNs in map     <s>
        .set    mrs_tid,mrs_nluns+2     # Target ID                 <s>
        .set    mrs_status,mrs_tid+2    # Status                    <b>
        .set    mrs_pri,mrs_status+1    # Priority                  <b>
        .set    mrs_attrib,mrs_pri+1    # Server attributes         <w>
        .set    mrs_owner,mrs_attrib+4  # Owner                     <w>
        .set    mrs_wwn,mrs_owner+4     # World wide name           <l>
        .set    mrs_name,mrs_wwn+8      # Name                      <q>
        .set    mrs_lunmap,mrs_name+16  # LUN/VID list             *<w>
# --- End base structure ----------------------------------------------
        .set    mrssiz,mrs_lunmap       # Base size
        .set    mrs_vid,0               # VID
        .set    mrs_lun,mrs_vid+2       # LUN
        .set    mrsxsiz,mrs_lun+2       # Extension size
        .set    mrsmaxentries,8         # 8 entries at a time
#
# --- Begin Report Server Config response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mrsrsiz,mr_rlen+4       # Size of response
#
# --- Begin Server Config Complete definitions (mrconfigscomplete) ----
# --- Begin Server Config Complete base structure ---------------------
# --- End base structure ----------------------------------------------
        .set    mrxsiz,0                # Base size
#
# --- Begin Server Config Complete response base structure ------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mrxrsiz,mr_rlen+4       # Size of response
#
# --- Begin Caching Config Report definitions (mrreportcconfig) ------
#       mrc_op
        .set    mrcmultistart,0         # Multiple caching records to be sent
        .set    mrccontinue,1           # Continuation of larger mapping
        .set    mrcmultidone,2          # Multiple caching records done
        .set    mrcsingle,3             # Single change (no cmplt expected)
        .set    mrcglobal,4             # Just change global setting
        .set    mrcdelete,5             # Delete the single device
#       mrc_features
        .set    mrc_cache_enable,0      # Cache Enable flag
        .set    mrc_mirror_write_info,1 # Mirror Write Information flag
        .set    mrc_copy_dest,2         # Destination of a REDI-Copy operation
        .set    mrc_rebuild_check_needed,3 # Rebuild Check needed before Writes
# --- Begin Caching Config Report base structure ---------------------
        .set    mrc_nvid,0              # Number of records in req  <s>
        .set    mrc_genable,mrc_nvid+2  # Global disable            <b>
        .set    mrc_op,mrc_genable+1    # Operation to be done      <b>
        .set    mrc_vidmap,mrc_op+1     # VID map                  *<w>
# --- End base structure ----------------------------------------------
        .set    mrcsiz,mrc_vidmap       # Base size
#
        .set    mrc_vid,0               # VID                       <s>
        .set    mrc_features,mrc_vid+2  # Features for this VID     <b>
#                                         Reserved                  <b>
        .set    mrc_capacity,mrc_features+2 # Capacity              <l>
        .set    mrcxsiz,mrc_capacity+8  # Size of extension
        .set    mrcmaxentries,(VRPALLOC-vrpsiz-mrcsiz)/mrcxsiz
                                        # Entries must fit within 1 MRP
#
# --- Begin Caching Config Report response base structure ------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mrcrsiz,mr_rlen+4       # Size of response
#
# --- Begin Stop caching definitions (mrstopcache 0x205) --------------
#       mxc_op
        .set    mxcwait,mxiwait         # Wait for Write Cache Flush
                                        #   0 = Do not wait for the Write
                                        #       Cache Flush to complete
                                        #   1 = Wait for the Write Cache Flush
                                        #       to complete
        .set    mxcnoflush,mxinoflush   # Flush Write Cache
                                        #   0 = Flush the Write Cache
                                        #   1 = Do not Flush the Write Cache
        .set    mxcnobackground,mxinobackground # Background Flush
                                        #   0 = Allow Background Flushing
                                        #   1 = Do not allow Background
                                        #       flushing - will wait for
                                        #       outstanding Write Cache ops
                                        #       to complete before returning
        .set    mxcnowaithost,mxinowaithost # Wait for Host I/O Completion
                                        #   0 = Wait for outstanding Host I/O
                                        #       to complete before returning
                                        #   1 = Do not wait for outstanding Host
                                        #       I/O to complete
#       mxc_user
        .set    mxcgenuser,mxigenuser   # 0 = Generic User (all Stop/Resumes
                                        #   must be paired)
                                        # 1-3F = PROC users
        .set    mxcdefinefe,mxidefinefe #   1 = Define FE
        .set    mxcbattery,mxibattery   #   2 = Battery Health
        .set    mxcwcrecvry,mxiwcrecvry #   3 = Write Cache Recovery
                                        #       Note: wcrcvry.c has constant for
                                        #   4-1F = Reserved for FE Stops
        .set    mxcdefinebe,mxidefinebe #  20 = Define BE
                                        #  21-3F = Reserved for BE Stops
                                        # 40-7F = CCB users
                                        # 80-BF = XSSA users
                                        # C0-FF = Reserved
# --- Begin Stop caching base structure -------------------------------
        .set    mxc_op,0                # Options flag byte         <b>
#       reserved 1
        .set    mxc_user,mxc_op+2       # User ID                   <b>
#       reserved 1
# --- End base structure ----------------------------------------------
        .set    mxcsiz,mxc_user+2       # Base size
#
# --- Begin Stop caching response base structure ----------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mxcrsiz,mr_rlen+4       # Size of response
#
# --- Begin Continue caching definitions (mrcontinuecache) ------------
#       mcc_op
        .set    mccclearone,mriclearone # Clear only the last stop issued
        .set    mccclearall,mriclearall # Clear all stop counts for this
                                        #  user (User ID > 0)
#       mcc_user
        .set    mccgenuser,mxigenuser   # 0 = Generic User (all Stop/Resumes
                                        #   must be paired)
                                        # 1-3F = PROC users
        .set    mccdefinefe,mxidefinefe #   1 = Define FE
        .set    mccbattery,mxibattery   #   2 = Battery Health
        .set    mccwcrecvry,mxiwcrecvry #   3 = Write Cache Recovery
                                        #       Note: wcrcvry.c has constant for
                                        #   4-1F = Reserved for FE Stops
        .set    mccdefinebe,mxidefinebe #  20 = Define BE
                                        #  21-3F = Reserved for BE Stops
                                        # 40-7F = CCB users
                                        # 80-BF = XSSA users
                                        # C0-FF = Reserved
# --- Begin Continue caching base structure ---------------------------
        .set    mcc_op,0                # Options byte              <b>
#       reserved    1                                               <b>
        .set    mcc_user,2              # User ID to continue       <b>
#       reserved    1                                               <b>
# --- End base structure ----------------------------------------------
        .set    mccsiz,mcc_user+2       # Base size
#
# --- Begin Continue caching response base structure ------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mccrsiz,mr_rlen+4       # Size of response
#
# --- Begin Set system info definitions (mrsetsysinfo) ----------------
# --- Begin Set system info base structure ----------------------------
        .set    mii_vcgid,0             # Virtual controller group  <w>
        .set    mii_cserial,mii_vcgid+4 # Controller serial number  <w>
        .set    mii_seq,mii_cserial+4   # Sequence number           <w>
        .set    mii_ccbipaddr,mii_seq+4 # IP address of CCB         <w>
        .set    mii_mp,mii_ccbipaddr+4  # Mirror partner            <w>
        .set    mii_ccbreq,mii_mp+4     # CCB required (T/F)        <b>
#       reserved 3                                                 3<b>
        .set    mii_vcgname,mii_ccbreq+4# VCG name                  <q>
# --- End base structure ----------------------------------------------
        .set    miisiz,mii_vcgname+16   # Base size
#
# --- Begin Set system info response base structure -------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    miirsiz,mr_rlen+4       # Size of response
#
# --- Begin Set sequence number definitions (mrfesetseq) --------------
# --- Begin Set sequence number base structure ------------------------
        .set    msq_seq,0               # System serial number      <w>
# --- End base structure ----------------------------------------------
        .set    msqsiz,msq_seq+4        # Base size
#
# --- Begin Set sequence number response base structure ---------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    msqrsiz,mr_rlen+4       # Size of response
#
# --- Begin Virtual disk updated definitions (mrvchange) --------------
# --- Begin Virtual disk base structure -------------------------------
# --- End base structure ----------------------------------------------
        .set    mvusiz,0                # Base size
#
# --- Begin Virtual disk updated response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mvursiz,mr_rlen+4       # Size of response
#
# --- Begin Server updated definitions (mrschange) --------------------
# --- Begin Server updated base structure -----------------------------
# --- End base structure ----------------------------------------------
        .set    msusiz,0                # Base size
#
# --- Begin Server updated response base structure --------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    msursiz,mr_rlen+4       # Size of response
#
# --- Begin Reset Configuration definitions (mrresetconfig) -----------
#     mre_options uses the same values as mxn_options in mrreset
# --- Begin Reset Configuration base structure ------------------------
        .set    mre_options,0           # Options                   <w>
# --- End base structure ----------------------------------------------
        .set    mresiz,mre_options+4    # Base size
#
# --- Begin Reset Configuration response base structure ---------------
#       reserved 3                                                 3<b>
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mrersiz,mr_rlen+4       # Size of response
#
# --- Begin Get Server Info definitions (mrgetsinfo) ------------------
# --- Begin Get Server Info base structure ----------------------------
        .set    mis_sid,0               # Server ID                 <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    missiz,mis_sid+4        # Base size
#
# --- Begin Get Server Info response base structure -------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
#       Duplicate the information in the SDD up to the hash table.
        .set    mis_sdata,mr_rlen+4     # Start of server data     *<w>
# --- Extensions
        .set    mis_vid,0               # VID
        .set    mis_lun,mis_vid+2       # LUN
# --- End base structure ----------------------------------------------
#        .set    misrsiz,mis_sdata+40    # Size of response
#                                        # Add four bytes per LUN->VDisk
#
# --- Begin Get Server Stats definitions (mrgetsstats) -----------------
# --- Begin Get Server Stats base structure ----------------------------
        .set    mgs_sid,0               # Server ID                 <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mgssiz,mgs_sid+4        # Base size
#
# --- Begin Get Server Stats response base structure -------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mgs_data,mr_rlen+4      # Start of server stats    *<w>
# --- End base structure ----------------------------------------------
        .set    mgsrsiz,mgs_data+imtstatsize # Size of response
#
# --- Begin Set Configurable Options definitions (mrsetconfigopt) ------
# --- Begin Set Configurable Options base structure -------------------
        .set    msco_whql,0             # Options                   <w>
# --- End base structure ----------------------------------------------
        .set    mscosiz,msco_whql+16    # Base size
#
# --- Begin Set Configurable Options response base structure ----------
#       reserved 3                                                 3<b>
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mscorsiz,mr_rlen+4      # Size of response
#
# --- Begin Read/Write Memory definitions (mrberwmem) -----------------
# --- Begin Read/Write Memory base structure  -------------------------
                                        # Next two addrs are from the target
                                        # processors perspective. They must
                                        # be word aligned.
        .set    mrw_saddr,0             # source addr
        .set    mrw_daddr,mrw_saddr+4   # destination addr
        .set    mrw_length,mrw_daddr+4  # length in bytes - multiple of 4
# --- End base structure ----------------------------------------------
        .set    mrwsiz,mrw_length+4     # Base size
#
# --- Begin Read/Write Memory information response base structure -----
#       reserved 3                                                 3<b>
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End response structure ------------------------------------------
        .set    mrwrsiz,mr_rlen+4       # Size of response
#
# --- Begin Get Global Cache Info definitions (mrgetcinfo) ------------
# --- Begin Get Global Cache Info base structure ----------------------
# --- End base structure ----------------------------------------------
        .set    mgcsiz,0                # Base size
#
# --- Begin Get Global Cache Info response base structure -------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mgc_data,mr_rlen+4      # Start of cache data      *<w>
# --- End base structure ----------------------------------------------
        .set    mgcrsiz,mgc_data+casiz  # Size of response (jlw - TDB)
#
# --- Begin Get Cache Device Info definitions (mrgetcdinfo) -----------
# --- Begin Get Cache Device Info base structure ----------------------
        .set    mcd_vid,0               # Virtual Cache Device ID   <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mcdsiz,mcd_vid+4        # Base size
#
# --- Begin Get Global Cache Info response base structure -------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mcd_data,mr_rlen+4      # Start of cache data      *<w>
# --- End base structure ----------------------------------------------
        .set    mcdrsiz,mcd_data+vcdstatsize # Size of response (jlw - TDB)
#
# --- Begin Set Battery Health base structure (mrsetbathelp) ----------
        .set    mbh_brd,0               # Battery board (FE or BE)  <b>
#       reserved 1
        .set    mbh_state,mbh_brd+2     # Battery health state      <b>
#       reserved 1
# --- End base structure ----------------------------------------------
        .set    mbhsiz,mbh_state+2     # Base size
#
# --- Begin Set Battery Health response base structure ----------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mbhrsiz,mr_rlen+4       # Size of response
#
# --- Begin Resume Cache Initialization base structure (mrresumecache)
        .set    mrci_ur,0                # User Response            <b>
#       reserved 3
# --- End base structure ----------------------------------------------
        .set    mrcisiz,mrci_ur+4       # Base size
#
# --- Begin Resume Cache Initialization response base structure -------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mrcirsiz,mr_rlen+4      # Size of response
#
# --- Begin Server Lookup base structure (mrserverlookup) -------------
        .set    msl_wwn,0               # World wide name           <l>
        .set    msl_tid,msl_wwn+8       # TID of server             <s>
#       reserved 6
# --- End base structure ----------------------------------------------
        .set    mslsiz,msl_tid+8        # Base size
#
# --- Begin Server lookup response base structure ---------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    msl_sid,mr_rlen+4       # SID of server             <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mslrsiz,msl_sid+4       # Size of response
#
# --- Begin Save Mirror Partner (mrfesetmp 0x0402) definitions --------
# --- Begin Save Mirror Partner base structure ------------------------
        .set    msmp_mp,0               # Mirror partner ser number <w>
# --- End base structure ----------------------------------------------
        .set    msmpsiz,msmp_mp+4       # Base size
        .set    mpsetconfigbesiz,48     # size of mirror partner info struct
#
# --- Begin Save Mirror Partner response base structure ---------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End response base structure -------------------------------------
        .set    msmprsiz,mr_rlen+4      # Size of response
#
# --- Begin Assign FE Mirror Partner (mrfeassignmp 0x0511) definitions
# --- Begin Assign FE Mirror Partner base structure -------------------
        .set    mamp_nserial,0          # New Mirror Partner Serial Number <w>
                                        #  if zero use Old Mirror Partner SN
# --- End base structure ----------------------------------------------
        .set    mampsiz,mamp_nserial+4  # Base size
        .set    mpconfigfesiz,48        # size of MIRROR PARTNER INFO struct
#
# --- Begin Assign FE Mirror Partner response base structure ----------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mamp_oserial,mr_rlen+4  # Old Mirror Parter SN      <w>
# --- End response base structure -------------------------------------
        .set    mamprsiz,mamp_oserial+4 # Size of response
#
# --- Begin FE Fibre Heartbeat List (mrfefibrehlist 0x0512) definitions
# --- Begin FE Fibre Heartbeat List base structure --------------------
        .set    mfhl_nctrl,0            # Number of controllers in list  <b>
#       reserved 3                                                      3<b>
        .set    mfhl_csnlist,mfhl_nctrl+4 # Controller list             *<w>
# --- End base structure ----------------------------------------------
        .set    mfhlsiz,mfhl_csnlist    # Base size
        .set    mfhl_csn,0              # Controller Serial Number       <w>
#
# --- Begin FE Fibre Heartbeat List response base structure -----------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status                <b>
#--     .set    mr_rlen,mr_status+1     # Return length                  <w>
        .set    mfhl_icsn,mr_rlen+4     # Invalid Controller Number (if any) <w>
# --- End response base structure -------------------------------------
        .set    mfhlrsiz,mfhl_icsn+4    # Size of response
#
# --- Begin Continue Cache without a Mirror Partner (mrfecontwomp 0x0513) definitions
# --- Begin Continue Cache without a Mirror Partner base structure ----
#       No input data
# --- End base structure ----------------------------------------------
#
# --- Begin Continue Cache without a Mirror Partner response base structure
#       Base response packet.
# --- End response base structure -------------------------------------
#
# --- Begin Flush FE Cache and ignore Mirroring (mrfeflushwomp 0x0514) definitions
# --- Begin Flush FE Cache and ignore Mirroring base structure --------
#       No input data
# --- End base structure ----------------------------------------------
#
# --- Begin Flush FE Cache and ignore Mirroring response base structure
#       Base response packet.
# --- End response base structure -------------------------------------
#
# --- Begin Invalidate FE Write Cache (mrinvfewc 0x0515) definitions --
#     Option values
        .set    mifoinvalvid,0          # Invalidate VID list
        .set    mifoglobalinval,1       # Global invalidate
# --- Begin Invalidate FE Write Cache base structure ------------------
        .set    mif_op,0                # Option flag byte               <b>
#       reserved 1
        .set    mif_nvids,mif_op+2      # Number of VIDs to invalidate   <s>
        .set    mif_vidlist,mif_nvids+2 # Start of VID list
# --- End base structure ----------------------------------------------
#
# --- Begin Invalidate FE Write Cache response base structure ---------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status                <b>
#--     .set    mr_rlen,mr_status+1     # Return length                  <w>
        .set    mif_ivid,mr_rlen+4      # Invalid VID (if any)           <s>
#       reserved 2
# --- End response base structure -------------------------------------
        .set    mifrsiz,mif_ivid+4      # Size of response
#
# --- Begin Flush BE Write Cache (mrflushbewc 0x0516) definitions -----
#     Option values
        .set    mfboinvalvid,0          # Invalidate VID list
        .set    mfboglobalinval,1       # Global invalidate
# --- Begin Flush BE Write Cache base structure -----------------------
        .set    mfb_op,0                # Option flag byte               <b>
#       reserved 1
        .set    mfb_nvids,mfb_op+2      # Number of VIDs to invalidate   <s>
        .set    mfb_vidlist,mfb_nvids+2 # Start of VID list
# --- End base structure ----------------------------------------------
#
# --- Begin Flush BE Write Cache response base structure --------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status                <b>
#--     .set    mr_rlen,mr_status+1     # Return length                  <w>
        .set    mfb_ivid,mr_rlen+4      # Invalid VID (if any)           <s>
#       reserved 2
# --- End response base structure -------------------------------------
        .set    mfbrsiz,mfb_ivid+4      # Size of response
#
# --- Begin Invalidate BE Write Cache (mrinvbewc 0x0517) definitions --
#     Option values
        .set    miboinvalvid,0          # Invalidate VID list
        .set    miboglobalinval,1       # Global invalidate
# --- Begin Invalidate BE Write Cache base structure ------------------
        .set    mib_op,0                # Option flag byte               <b>
#       reserved 1
        .set    mib_nvids,mib_op+2      # Number of VIDs to invalidate   <s>
        .set    mib_vidlist,mib_nvids+2 # Start of VID list
# --- End base structure ----------------------------------------------
#
# --- Begin Invalidate BE Write Cache response base structure ---------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status                <b>
#--     .set    mr_rlen,mr_status+1     # Return length                  <w>
        .set    mib_ivid,mr_rlen+4      # Invalid VID (if any)           <s>
#       reserved 2
# --- End response base structure -------------------------------------
        .set    mibrsiz,mib_ivid+4      # Size of response
#
# --- Begin Set FE port event notification ----------------------------
        .set    mpn_t1,0                # Loop down to notify T1         <w>
        .set    mpn_t2,mpn_t1+4         # Reset to notify T2             <w>
        .set    mpn_t3,mpn_t2+4         # Delay before initialize T3     <w>
        .set    mpn_resetRetry,mpn_t3+4 # Reset retry count              <w>
        .set    mpn_t4,mpn_resetRetry+4 # Reset to notify T4             <w>
        .set    mpn_err_limit,mpn_t4+4  # Error limit before notify      <w>
#       reserved 8
# --- End base structure ----------------------------------------------
        .set    mpnsiz,mpn_err_limit+12 # Base size
#
# --- Begin Fail Port base structure ----------------------------------
        .set    mpo_option,0            #  Option                        <s>
        .set    mpo_fail,mpo_option+2   #  Fail it                       <b>
        .set    mpo_qualifier,mpo_fail+2 # Qualifier                     <w>
        .set    mpo_port,mpo_qualifier+4 # port number (Chip Instance)   <w>
#       reserved 4
# --- End base structure ----------------------------------------------
        .set    mposiz,mpo_port+8       # Base size
#
# --- Begin Fail Controller base structure ----------------------------
        .set    mco_option,0            #  Option                        <s>
        .set    mco_qualifier,mco_option+4 # Qualifier                   <w>
        .set    mco_oldOwner,mco_qualifier+4 # Old owner                 <w>
        .set    mco_newOwner,mco_oldOwner+4 # New owwer                  <w>
# --- End base structure ----------------------------------------------
        .set    mcosiz,mco_newOwner+4   # Base size
#
# --- Begin Set Foreign Target (on/off) base structure ----------------
        .set    msft_option,0           #  Option                        <b>
#       reserved 3
# --- End base structure ----------------------------------------------
        .set    msftsiz,msft_option+4   # Base size
        .set    msftrsiz,mr_rlen+4      # Response size (none)
#
# --- Begin heartbeat definitions (mrfehbeat and mrbehbeat) -----------
# --- Begin heartbeat request base structure --------------------------
        .set    mhb_timestamp,0         # Timestamp                3<w>
#       reserved 4                                                  <w>
# --- End base structure ----------------------------------------------
        .set    mhbsiz,mhb_timestamp+16 # Size of request
#
# --- Begin heartbeat response base structure -------------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mhb_iopersec,mr_rlen+4  # Aggregate IO per second   <l>
        .set    mhb_bpersec,mhb_iopersec+8 # Aggregate bytes per sec<l>
# --- End base structure ----------------------------------------------
        .set    mhbrsiz,mhb_bpersec+8   # Response size
#
# --- Begin Query FE Controller Communications definitions (mrqfecc) --
# --- Begin Query FE Controller Communications request base structure -
        .set    mqc_serial,0            # Controller Serial Number  <w>
# --- End base structure ----------------------------------------------
        .set    mqcsiz,mqc_serial+4     # Size of request
#
# --- Begin Query FE Controller Communications response base structure
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mqcrsiz,mr_rlen+4       # Response size
#
# --- Begin Get Workset information definitions (mrgetwsinfo) ---------
# --- Begin Get Workset information request base structure ------------
# --- End base structure ----------------------------------------------
        .set    mgetwssiz,4             # Size of request
#
# --- Begin Get Workset information response base structure -----------
# --- End base structure ----------------------------------------------
#
# --- Begin Set Workset information definitions (mrgetwsinfo) ---------
# --- Begin Set Workset information request base structure ------------
# --- End base structure ----------------------------------------------
        .set    msetwssiz,53            # Size of request
#
# --- Begin Get Workset information response base structure -----------
# --- End base structure ----------------------------------------------
        .set    msetwsrsiz,8            # Response size
#
# --- Begin Emulate BE Qlogic timeout on pdisk (mrpdiskqltimeout) ---------
        .set    mpdiskqltimeoutssiz,3   # Size of request (short pid, char flag)
        .set    mpdiskqltimeoutrsiz,8   # Response size (header only).
#
# --- Begin Change RAID Not Mirroring State (mrchgraidnotmirroring) ---
# --- Begin Change RAID Not Mirroring State request base structure ----
# --- End base structure ----------------------------------------------
        .set    mrchgraidnotmirroringsiz,4    # Size of request
#
# --- Begin Modify RAID AStatus Field response base structure ---------
# --- End response base structure -------------------------------------
        .set    mrchgraidnotmirroringrsiz,mr_rlen+4   # Size of response
# --- End base structure ----------------------------------------------
#
# --- Begin Start recovery on an Inoperable RAID 5 (mrRAIDRecover) ----
# --- Begin Start recovery on an Inoperable RAID 5 request base struct
        .set    mr5_rid,0               # RAID 5 ID - Clear Inop Status  <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mr5siz,mr5_rid+4        # Base size
#
# --- Begin Start recovery on an Inoperable RAID 5 response base struct
# --- End response base structure -------------------------------------
        .set    mr5rsiz,mr_rlen+4       # Size of response
# --- End base structure ----------------------------------------------
#
# --- Begin Put Device Configuration (mrputdevconfig) -----------------
# --- Begin Put Device Configuration request base struct
#       See C declarations (variable size so don't create constants)
# --- End base structure ----------------------------------------------
#
# --- Begin Get Copy/resync Data (mrresyncdata) -----------------------
# --- Begin Get Copy/resync Data request base structure
        .set    myd_format,0            # requested response format    <b>
# --- End base structure ----------------------------------------------
        .set    mydsiz,myd_format+8        # Base size
#
# --- Begin Get Copy/resync Common Data response base structure
#--     .set    mr_status,mr_status     # Response status               <b>
#--     .set    mr_rlen,mr_status+1     # Return length                 <w>
        .set    mydr_strctcnt,mr_rlen+4 # number of cors returned       <s>
        .set    mydr_format,mydr_strctcnt+2 # response format           <b>
        .set    mydr_strctsiz,mydr_format+1 # structure size 0 if > 255 <b>
        .set    mydr_common,mydr_strctsiz+1 # start of command area
# --- Format types
        .set    MRDRCORRSP,0            # complete COR response
        .set    MRDTLCPYRSP,1           # detailed copy response
        .set    MRDRTRACES,2            # CCSM traces response
        .set    MRDIOMAP,3              # copy IO map
# --- Begin Get COR structure response base structure
#     Format 0
        .set    mydrsiz,mydr_common+corsize # size of Get Copy/resync data
                                        #  response
# --- End response base structure -------------------------------------
#     Format 0
# --- Begin Get Copy/resync Data response base structure
#     Format 1
        .set    m1dr_rcsn,0             # Copy ctrl sn                      <w>
        .set    m1dr_rid,m1dr_rcsn+4    # copy registration id              <w>
        .set    m1dr_rcscl,m1dr_rid+4   # copy reg. copy MAG source cl num  <b>
        .set    m1dr_rcsdv,m1dr_rcscl+1 # copy reg. copy MAG source dv num  <b>
        .set    m1dr_rcdcl,m1dr_rcsdv+1 # copy reg. copy MAG dest. cl num   <b>
        .set    m1dr_rcddv,m1dr_rcdcl+1 # copy reg. copy MAG dest dv num    <b>
        .set    m1dr_ccstate,m1dr_rcddv+1  # cor copy state                 <b>
        .set    m1dr_crstate,m1dr_ccstate+1# cor copy registration state    <b>
        .set    m1dr_cmstate,m1dr_crstate+1# cor region/segment map state   <b>
        .set    m1dr_mcstate,m1dr_cmstate+1# cm copy state code             <b>
        .set    m1dr_sp2hdlr,m1dr_mcstate+1# scd p2 handler ordinal         <b>
        .set    m1dr_stype,m1dr_sp2hdlr+1 # scd type code                   <b>
        .set    m1dr_dp2hdlr,m1dr_stype+1 # dcd p2 handler ordinal          <b>
        .set    m1dr_dtype,m1dr_dp2hdlr+1 # dcd type code                   <b>
        .set    m1dr_vmirror,m1dr_dtype+1 # vdd mirror state                <b>
        .set    m1dr_vattr,m1dr_vmirror+1 # vdd attributes                  <b>
        .set    m1dr_owner,m1dr_vattr+1   # copy owner                      <b>
        .set    m1dr_ctype,m1dr_owner+1  # copy type                       <b>
        .set    m1drsize,m1dr_owner+6     # size of structure              6<b>
# --- End base structure ----------------------------------------------
#     Format 1
# --- Begin Copy/resync Control (mrresyncctl) -------------------------
# --- Begin Copy/resync Control request base structure
        .set    myc_fc,0                # function code                 <b>
                                        # 3 spare bytes                3<b>
        .set    myc_rid,myc_fc+4        # copy registration ID          <w>
        .set    myc_csn,myc_rid+4       # copy manager serial #         <w>
        .set    myc_gid,myc_csn+4       # gid number                    <w>
        .set    myc_name,myc_gid+4      # copy user defined name      16<b>
# --- End base structure ----------------------------------------------
        .set    mycsiz,myc_name+16       # Base size
#
# --- Begin Copy/resync Control response base structure
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
        .set    mycrsiz,mr_rlen+4       # response length
#
# --- Begin Configuable Options definitions (mrcfgoption) --------
#     Word 0 bit definitions
        .set    mcoptenableWHQL,0        # Enable WHQL
#
# --- Begin command base structure ------------------------------------
        .set    mcopt_data0,0             # Word 0 data               <w>
        .set    mcopt_mask0,mcopt_data0+4 # Word 0 mask               <w>
# --- End base structure ----------------------------------------------
        .set    mcoptsiz,mcopt_mask0+4      # Base size
#
# --- Begin response base structure ---------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status             <b>
#--     .set    mr_rlen,mr_status+1     # Return length               <w>
        .set    mrcopt_options,mr_rlen+4 # current option settings    <w>
# --- End base structure ----------------------------------------------
        .set    mcoptrsiz,mrcopt_options+4 # Size of response
#
#-----------------------------------------------------------------------------
#
# --- Begin Query Stop Completion (mrqsc 0x0525) ----------------------
# --- Begin Query Stop Completion request base structure
#       No input data
# --- End base structure ----------------------------------------------
#
# --- Begin Query Stop Completion response base structure
#       reserved 3
#--     .set    mr_status,mr_status     # Response status                <b>
#--     .set    mr_rlen,mr_status+1     # Return length                  <w>
        .set    mqscnum,mr_rlen+4       # Number of outstanding ops      <w>
# --- End response base structure -------------------------------------
        .set    mqscrsiz,mqscnum+4      # Size of response
# --- End base structure ----------------------------------------------
#
# --- Begin Query Mirror Partner Change (mrqmpc 0x0526)----------------
#     mpResponse bit values
        .set    mrqmpcnostop,0          # Bit 0 = I/O Is not stopped
        .set    mrqmpcioouts,1          # Bit 1 = I/O are still outstanding
        .set    mrqmpccachedata,2       # Bit 2 = Data Still Cached for old MP
        .set    mrqmpcnocomm,3          # Bit 3 = No Comm Path to new MP
# --- Begin Query Mirror Partner Change request base structure
        .set    mqmpc_serial,0          # Controller Serial Number  <w>
# --- End base structure ----------------------------------------------
        .set    mqmpcsiz,mqmpc_serial+4 # Size of request
#
# --- Begin Query Mirror Partner Change response base structure
#       reserved 3
#--     .set    mr_status,mr_status     # Response status                <b>
#--     .set    mr_rlen,mr_status+1     # Return length                  <w>
        .set    mqmpcrsp,mr_rlen+4      # Mirror Partner Change response <b>
#       reserved 3
# --- End response base structure -------------------------------------
        .set    mqmpcrsiz,mqmpcrsp+4    # Size of response
# --- End base structure ----------------------------------------------
#
# --- Begin Get HBA Stats definitions (mrgethbastats 0x527) -----------
# --- Begin Get HBA Stats base structure ------------------------------
        .set    mghs_port,0             # HBA Port ID               <s>
#       reserved 2
# --- End base structure ----------------------------------------------
        .set    mghssiz,mghs_port+4     # Base size
#
# --- Begin Get Server Stats response base structure -------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# Start of HBA stats
        .set    mghs_cmds,mr_rlen+4     # Periodic command count    <l>
        .set    mghs_qdepth,mghs_cmds+8 # Current Q depth           <s>
#       reserved 6
#       defined but reserved fields follow:
        .set    mghs_avgreqsiz,mghs_qdepth+8     # (average request size) <l>
        .set    mghs_writereqs,mghs_avgreqsiz+8  # (# of write requests)  <l>
        .set    mghs_readreqs,mghs_writereqs+8   # (# of read requests)   <l>
#       reserved 16
# --- End base structure ----------------------------------------------
        .set    mghsrsiz,mghs_readreqs+8+16      # Size of response
#
# --- Set the Write Cache into a Temporary Disable state (mrsettdiscache 0x052B)
# --- Begin Set the Write Cache into a Temporary Disable state req base struc
        .set    mstd_user,0             # Requester of Temp Disable         <b>
        .set    mstd_rsvd,mstd_user+1   # Reserved                         3<b>
# --- End request structure ----------------------------------------------
        .set    mstdcsiz,mstd_rsvd+3    # Size of Request
#
# --- Set the Write Cache into a Temporary Disable state response Structure size
        .set    mstdcrsiz,mr_rlen+4     # Size of response
# --- End base structure ----------------------------------------------
#
# --- Clear the Write Cache Temporary Disable state (mrclrtdiscache 0x052C)
# --- Begin Clear the Write Cache Temporary Disable state request base structure
        .set    mctd_user,0             # Requester of Temp Disable         <b>
        .set    mctd_option,mctd_user+1 # Option                            <b>
        .set    mctd_rsvd,mctd_option+1 # Reserved                         2<b>
# --- End request structure ----------------------------------------------
        .set    mctdcsiz,mctd_rsvd+2    # Size of Request
#
# --- Clear the Write Cache Temporary Disable state response Structure size
        .set    mctdcrsiz,mr_rlen+4     # Size of response
# --- End base structure ----------------------------------------------
#
# --- Query if the WC Temp Disable Flush is Done (mrqtdisflushdone 0x052D)
# --- Query if the WC Temp Disable Flush is Done request base structure
#       No input data
# --- End base structure ----------------------------------------------
#
# --- Query if the WC Temp Disable Flush is Done response base structure
#       reserved 3
#--     .set    mr_status,mr_status     # Response status                <b>
#--     .set    mr_rlen,mr_status+1     # Return length                  <w>
        .set    mqtd_status,mr_rlen+4   # Status of Temp Disable Flush Done <b>
                                        # Reserved                      3<b>
# --- End response base structure -------------------------------------
        .set    mqtdcrsiz,mqtd_status+4 # Size of response
# --- End base structure ----------------------------------------------
#
# --- Begin NV Test (mrmmtest 0x052B)----------------------------------
# --- Begin NV Test request base structure
# --- End base structure ----------------------------------------------
        .set    mrmmtestsiz,8           # Size of request
#
# --- Begin NV Test response base structure
# --- End response base structure -------------------------------------
        .set    mrmmtestrsiz,mr_rlen+4  # Size of response
# --- End base structure ----------------------------------------------
#
# --- Begin Flush FE Cache Due to Error to MP (mrfeflerrorwomp 0x052F) defines
# --- Begin Flush FE Cache Due to Error to MP base structure --------
#       No input data
# --- End base structure ----------------------------------------------
#
# --- Begin Flush FE Cache Due to Error to MP response base structure
#       Base response packet.
# --- End response base structure -------------------------------------
#
#==============================================================================
#
# --- Begin Logging definitions ---------------------------------------
#
#     mle_event
#
#   The following event codes are 16 bit values that have the most significant
#   four bits set to indicate the severity and/or display type for the event.
#   The values here have the default settings for the upper nibble but may be
#   masked off and modified by the logging call to change the type.
#
#   Hidden and debug give additional controls while retaining the severity
#   codes.
#
        .set    mleinform,  0x0000
        .set    mlewarn,    0x4000
        .set    mleerror,   0x8000
        .set    mlefatal,   0xC000
        .set    mlesevmask, 0xC000
#
        .set    mlehidden,  0x2000
        .set    mledebug,   0x1000
        .set    mlecustomer,0x0000
#
#
        .set    mlescrubdone,                           0x0000+mledebug   # Scrub complete log event
        .set    mlenvramrestore,                        0x0001+mleinform  # NVRAM restore log event
        .set    mlebootcodeeventinfo,                   0x0002+mleinform  # Boot code generated event
        .set    mleboot,                                0x0003+mleinform  # Basic boot complete log event
        .set    mledevremoved,                          0x0004+mleerror   # Device removed log event
        .set    mledevinsert,                           0x0005+mleinform  # Device inserted log event
        .set    mledevreattach,                         0x0006+mleinform  # Device reattached log event
        .set    mlecopycomplt,                          0x0007+mleinform  # Copy complete log event
        .set    mlehostoffline,                         0x0008+mleinform  # Offline log event
        .set    mlelogtextmessageinfo,                  0x0009+mleinform  # Fan restored log event
        .set    mlebufferboardsenabled,                 0x000A+mleinform  # Cache boards are ready log event
        .set    mleconfigchanged,                       0x000B+mledebug   # Back end config changed log event
        .set    mlecacheflushrec,                       0x000C+mleinform  # Cache Flush Recovered log event
        .set    mlefirmwareupdate,                      0x000D+mleinform  # Firmware Updated
        .set    mlecreate_server_op,                    0x000E+mledebug
        .set    mledelete_server_op,                    0x000F+mleinform
#
        .set    mleassoc_server_vdisk_op,               0x0010+mleinform
        .set    mledisassoc_server_vdisk_op,            0x0011+mleinform
        .set    mlecreate_vdisk_op,                     0x0012+mleinform
        .set    mledelete_vdisk_op,                     0x0013+mleinform
        .set    mleexpand_vdisk_op,                     0x0014+mleinform
        .set    mleprepare_vdisk_op,                    0x0015+mleinform
        .set    mlecontrol_vdisk_op,                    0x0016+mleinform
        .set    mlesetcache_vdisk_op,                   0x0017+mleinform
        .set    mlesetattribute_vdisk_op,               0x0018+mleinform
        .set    mleinit_raid_op,                        0x0019+mleinform
        .set    mlecontrol_raid_op,                     0x001A+mleinform
        .set    mledelete_device_op,                    0x001B+mleinform
        .set    mlebreaklock_vlink_op,                  0x001C+mleinform
        .set    mleprocassignmirrorpartner_op,          0x001D+mleinform
        .set    mledrivebayfwupdate,                    0x001E+mleinform
        .set    mlepdisk_setup_op,                      0x001F+mleinform
#
        .set    mlepdisk_fail_op,                       0x0020+mleinform
        .set    mledefrag,                              0x0021+mleinform
        .set    mlepsdrebuilddone,                      0x0022            # PSD rebuild was compeleted
        .set    mlepddrebuilddone,                      0x0023+mleinform  # Full PDD rebuild was completed
        .set    mlehsparedone,                          0x0024+mleinform  # Hot spare operation was completed
        .set    mleraidinitdone,                        0x0025            # RAID initialization was completed
        .set    mleloopup,                              0x0026+mleinform  # Loop Up
        .set    mledloopup,                             0x0026+mledebug   # Loop Up
        .set    mleportdbchanged,                       0x0027+mledebug   # Port data base changed
        .set    mleconfigsessionstarted,                0x0028+mleinform
        .set    mleconfigsessionended,                  0x0029+mleinform
        .set    mlesettargetproperties,                 0x002A+mleinform
        .set    mlesesfanon,                            0x002B+mleinform
        .set    mlepsdrebuildstrt,                      0x002C+mledebug   # PSD rebuild started
        .set    mleestvlink,                            0x002D+mleinform  # Establish VLink Dg
        .set    mletermvlink,                           0x002E+mleinform  # Terminate VLink Dg
        .set    mleswpvlink,                            0x002F+mleinform  # Swap a VLink DG
#
        .set    mlenewpathdebug,                        0x0030+mledebug   # New Path to XIOtech
        .set    mlenewpathinfo,                         0x0030+mleinform  # New Path to XIOtech
        .set    mlefwversions,                          0x0031+mledebug   # Firmware versions
        .set    mlevlszchg,                             0x0032+mleinform  # VLink Size Changed
        .set    mleparitychkdone,                       0x0033+mleinform  # Parity scan pass complete
        .set    mledefragdone,                          0x0034+mleinform  # Defrag on a RAID complete
        .set    mlei2cbusgood,                          0x0035+mleinform  # I2C Bus Good
        .set    mlevcginactivatecontroller_op,          0x0036+mleinform  # LM80 Good
        .set    mlevcgactivatecontroller_op,            0x0037+mleinform  # LM75 Good
        .set    mlecreatevlinkop,                       0x0038+mleinform  # Create Virtual Link
        .set    mleserversetpropertiesop,               0x0039+mleinform  # Server Set Properties
        .set    mleethernetlinkup,                      0x003A+mleinform  # Ethernet link up or changed state
        .set    mleflushcomplete,                       0x003B+mledebug   # Write Cache Flush Complete
        .set    mlenvarecord,                           0x003C+mledebug   # Couldn't resynch NVA record
        .set    mlefsupdate,                            0x003D+mledebug   # Drive updated with File System
        .set    mlepulse,                               0x003E+mledebug   # System heartbeat log event
        .set    mletagsrecovered,                       0x003F+mleinform  # Cache recovery complete
#
        .set    mlenvramreload,                         0x0040+mledebug   # NVRAM reload log event
        .set    mleipclinkup,                           0x0041+mleinform  # IPC Link up
        .set    mleipcbroadcast,                        0x0042+mleinform  # IPC broadcast a message
        .set    mletargetmoveop,                        0x0043+mleinform  # Target Move op
        .set    mleinitprocnvramop,                     0x0044+mledebug   # Init Proc NVRAM Op
        .set    mleinitccbnvramop,                      0x0045+mleinform  # Init CCB NVRAM Op
        .set    mlesetsystemserialnumop,                0x0046+mleinform  # Set System Serial Number Op
        .set    mlevcgprepareslaveop,                   0x0047+mleinform  # VCG Prepare Slave Op
        .set    mlevcgaddslaveop,                       0x0048+mleinform  # VCG Add Slave Op
        .set    mlevcgsetmirrorpartnersop,              0x0049+mleinform  # VCG Set Mirror Partners Op
        .set    mlevcgstartioop,                        0x004A+mleinform  # VCG Start IO Op
        .set    mlevcgstopioop,                         0x004B+mleinform  # VCG Stop IO Op
        .set    mlevcgglobalcacheop,                    0x004C+mleinform  # VCG Global Cache Op
        .set    mlevcgapplylicenseop,                   0x004D+mleinform  # VCG Apply License Op
        .set    mlevcgunfailcontrollerop,               0x004E+mleinform  # VCG Unfail Controller Op
        .set    mlevcgfailcontrollerop,                 0x004F+mleinform  # VCG Fail Controller Op
#
        .set    mlevcgremovecontrollerop,               0x0050+mleinform  # VCG Remove Controller Op
        .set    mlevcgshutdownop,                       0x0051+mleinform  # VCG Shutdown Op
        .set    mlefeqlogicresetop,                     0x0052+mledebug   # Reset Front End QLogic
        .set    mlebeqlogicresetop,                     0x0053+mledebug   # Reset Back End QLogic
        .set    mleprocstartioop,                       0x0054+mleinform  # Proc Start IO
        .set    mleprocstopioop,                        0x0055+mleinform  # Proc Stop IO
        .set    mlesystemnmi,                           0x0056+mleinform  # SystemNMI log event
        .set    mleadminsettimeop,                      0x0057+mledebug   # Set System Time
        .set    mlelogfailureeventinfo,                 0x0058+mleinform  # Failure management message
        .set    mlesnapshotrestored,                    0x0059+mleinform  # Restore successful
        .set    mlecontrollersready,                    0x005A+mleinform  # Power-up all controllers ready
        .set    mlepowerupcomplete,                     0x005B+mleinform  # Power-up sequencing completed
        .set    mlermeventinfo,                         0x005C+mleinform  # Resource Manager message
        .set    mleadminsetip_op,                       0x005D+mleinform  # Resource Manager message
        .set    mlesapshottaken,                        0x005E+mleinform  # Capture taken
        .set    mlecopysync,                            0x005F+mleinform  # Copy has entered sync state
#
        .set    mlealldevmissing,                       0x0060+mleerror   # All drives missing or inoperable
                                                      # 0x0061            # A log Message sent by the XSSA
        .set    mledevfailhs,                           0x0062+mleerror   # Device fail (w/ hot spare)
        .set    mlesmart,                               0x0063+mleerror   # SMART enunciation log event
        .set    mlespinupfailed,                        0x0064+mlewarn    # Drive did not spin up
        .set    mlelogtextmessagewarning,               0x0065+mlewarn    # Processor temp high log event
        .set    mlevalidationwarning,                   0x0066+mlewarn    # Fan speed alert log event
        .set    mlecacheflushfail,                      0x0067+mlewarn    # Cache Flush Failed log event
        .set    mlefirmwarealert,                       0x0068+mlewarn    # Firmware Alert
        .set    mleloopdown,                            0x0069+mlewarn    # Loop Down
        .set    mleccbnvramrestored,                    0x006A+mlewarn
        .set    mlesesdevicebypassA,                    0x006B+mlewarn
        .set    mlesesdevicebypassB,                    0x006C+mlewarn
        .set    mlesesdeviceoff,                        0x006D+mlewarn
        .set    mlesespstempwarn,                       0x006E+mlewarn
        .set    mlesesvolthiwarn,                       0x006F+mlewarn
#
        .set    mlesesvoltlowarn,                       0x0070+mlewarn
        .set    mlesestemphiwarn,                       0x0071+mlewarn
        .set    mlesestemplowarn,                       0x0072+mlewarn
        .set    mlelostpathdebug,                       0x0073+mledebug   # Lost a path to a XIOtech box
        .set    mlelostpathwarn,                        0x0073+mlewarn    # Warning: Lost path to XIOtech box
        .set    mleispchipreset,                        0x0074+mlewarn    # ISP Chip reset
        .set    mlecachemirrorfailed,                   0x0075+mleerror   # Cache N-Way mirror failed
        .set    mlesinglepath,                          0x0076+mlewarn    # Only a single path exists to device
        .set    mlehotspareinop,                        0x0077+mlewarn    # Hotspare no longer operative
        .set    mleethernetlinkdown,                    0x0078+mlewarn    # Ethernet link down
        .set    mlemsgdeleted,                          0x0079+mlewarn
        .set    mlevcgshutdownwarn,                     0x007A+mlewarn
        .set    mlesescurrenthiwarn,                    0x007B+mlewarn    # SES current over high limit warn
        .set    mleseselreport,                         0x007C+mlewarn    # SES electronics active ctrl change
        .set    mleseselpresent,                        0x007D+mlewarn    # SES electronics present
        .set    mlelogfailureeventwarn,                 0x007E+mlewarn    # Failure management message
        .set    mlermwarn,                              0x007F+mlewarn
#
                                                      # 0x0080            # SES IO Module Pulled
        .set    mlebufferboardsdisabledinfo,            0x0081+mleinform
        .set    mlebufferboardsdisabledwarn,            0x0082+mlewarn
        .set    mlebufferboardsdisablederror,           0x0083+mleerror   # Cache boards are not ready log event
        .set    mlebootcodeeventwarn,                   0x0084+mlewarn    # Boot code generated event
        .set    mledevfailnhs,                          0x0085+mleerror   # Device fail (w/o hot spare)
        .set    mledevmissing,                          0x0086+mleerror   # Device missing log event
        .set    mleserialwrong,                         0x0087+mleerror   # System serial number mismatch
        .set    mlenvabad,                              0x0088+mleerror   # NVA checksum error
        .set    mlelogtesxtmessageerror,                0x0089+mleerror   # I2C init failed log event
        .set    mlecacheDRAMfail,                       0x008A+mleerror   # Cache DRAM failed log event
        .set    mlecacherecoverfail,                    0x008B+mleerror   # Cache recovery fault log event
        .set    mlecopyfailed,                          0x008C+mleerror   # Copy failed log event
        .set    mlesesdevflt,                           0x008D+mleerror   # SES device element fault
        .set    mlesespsovertemp,                       0x008E+mleerror   # SES power supply over temp fault
        .set    mlesespsacfail,                         0x008F+mleerror   # SES power supply AC failure
#
        .set    mlesespsdcfail,                         0x0090+mleerror   # SES power supply DC failure
        .set    mlesesfanfail,                          0x0091+mleerror   # SES fan failure
        .set    mlesestempfail,                         0x0092+mleerror   # SES temperature failure
        .set    mlesesvoltagehi,                        0x0093+mleerror   # SES voltage over high limits
        .set    mlesesvoltagelo,                        0x0094+mleerror   # SES voltage under low limits
        .set    mleerrtrap,                             0x0095+mlefatal   # Error trap
        .set    mleforeignpcidev,                       0x0096+mleerror   # Foreign PCI device detected
        .set    mleccbnvramreset,                       0x0097+mleerror   # CCB NVRAM reset
        .set    mlewcseqnobad,                          0x0098+mleerror   # Write cache sequence number bad
        .set    mlevidrecoverfail,                      0x0099+mleerror   # Cache recovery for VID failed
        .set    mleinvalidtag,                          0x009A+mleerror   # Invalid tag during Cache recovery
        .set    mlesesfanoff,                           0x009B+mleerror
        .set    mlesespsdcovervolt,                     0x009C+mleerror
        .set    mlesespsdcundervolt,                    0x009D+mleerror
        .set    mlesespsdcovercurr,                     0x009E+mleerror
        .set    mlesespsoff,                            0x009F+mleerror
#
        .set    mlesespsfail,                           0x00A0+mleerror
        .set    mlesestemplofail,                       0x00A1+mleerror
        .set    mlefileioerr,                           0x00A2+mleerror
        .set    mleprocnotready,                        0x00A3+mleerror   # Proccessor did not come ready
        .set    mlei2cbusfail,                          0x00A4+mleerror   # I2C Bus Hang Failure
        .set    mleillegalelectionstate,                0x00A5+mleerror   # Illegal election state change
        .set    mleraiderror,                           0x00A6+mleerror   # RAID detected drive error
        .set    mleserialmismatch,                      0x00A7+mleerror   # FE/BE Controller serial number mismatch
        .set    mleportinitfailed,                      0x00A8+mleerror   # Port initialization failure
        .set    mleportevent,                           0x00A9+mleerror   # Port event notification
        .set    mlenvramcsum,                           0x00AA+mleerror   # Part 2 NVRAM checksum error log event
        .set    mlebatterynotfound,                     0x00AB+mleerror   # Battery not found log event
        .set    mlefwupdatefailed,                      0x00AC+mleerror
        .set    mlebeinitiator,                         0x00AD+mleerror   # Initiator on BE detected
        .set    mlelostallpaths,                        0x00AE+mledebug   # Lost all paths to a XIOtech box
        .set    mlefsupdfail,                           0x00AF+mleerror   # File System update failure on drive
#
        .set    mledevto,                               0x00B0+mleerror   # Device timeout log event
        .set    mleseselfail,                           0x00B1+mleerror   # SES electronics failed
        .set    mlesesloopafail,                        0x00B2+mleerror   # SES loop A failed
        .set    mlesesloopbfail,                        0x00B3+mleerror   # SES loop B failed
        .set    mlesesspeedmis,                         0x00B4+mleerror   # SES speed mismatch
        .set    mlesesfwmismatch,                       0x00B5+mleerror   # SES firmware mismatch
        .set    mlesescurrenthi,                        0x00B6+mleerror   # SES current over high limit
        .set    mleipclinkdown,                         0x00B7+mleerror   # IPC Link down
        .set    mleproccommnotready,                    0x00B8+mleerror   # Processor communication not ready
        .set    mlenoowneddrives,                       0x00B9+mleerror   # No owned drives available
        .set    mlectrlfailed,                          0x00BA+mleerror   # Controller is failed
        .set    mlectrlunused,                          0x00BB+mleerror   # Controller is not used in the group
        .set    mlefwvincompatible,                     0x00BC+mleerror   # Controller firmware versions are incompatible
        .set    mlelogfailureevent,                     0x00BD+mleerror   # Failure management message
        .set    mlesnapshotrestorefailed,               0x00BE+mleerror   # Restore attempt failed
        .set    mlemissingdiskbay,                      0x00BF+mleerror   # One or more disk bays are missing
#
        .set    mlewaitcorruptbenvram,                  0x00C0+mleerror   # Inital load of BE NVRAM was corrupted
        .set    mlemissingcontroller,                   0x00C1+mleerror
        .set    mle_rm_error,                           0x00C2+mleerror   # RM ERROR class messages
        .set    mlebootcodeeventerror,                  0x00C3+mleerror   # Boot code generated event
        .set    mleled,                                 0x00C4+mledebug   # LED change log event
        .set    mlersvd_C5,                             0x00C5+mledebug
        .set    mlechangename,                          0x00C6+mledebug
        .set    mlegetlisterror,                        0x00C7+mledebug   # GetList() call failed - info msg
        .set    mleelectionstatechange,                 0x00C8+mledebug   # Changed election state
        .set    mlelclimagerdy,                         0x00C9+mledebug   # Local image ready
        .set    mlerefreshnvram,                        0x00CA+mledebug   # Refresh the NVRAM on other controllers
        .set    mlemagdriver,                           0x00CB+mledebug   # MagDriver error log event
        .set    mlehostnonsense,                        0x00CC+mleinform  # Host nonSense log event
        .set    mlehostqlogic,                          0x00CD+mledebug   # Host QLogic log event
        .set    mleraidevent,                           0x00CE+mledebug   # RAID error, rebuild, and hotspare generic logs
        .set    mleiocb,                                0x00CF+mledebug   # IOCB log event
#
        .set    mlesingleecc,                           0x00D0+mledebug   # Single-bit ECC error has occurred
        .set    mleresyncdone,                          0x00D1+mledebug   # Stripe resync completed
        .set    mleshortscsi,                           0x00D2+mleinform  # Short SCSI log event
        .set    mlelongscsi,                            0x00D3+mleinform  # Long SCSI log event
        .set    mlehostimmednotify,                     0x00D4+mledebug   # Immediate Notify log event
        .set    mlehostsense,                           0x00D5+mledebug   # Host Sense log event
        .set    mlezoneinquiry,                         0x00D6+mledebug   # Zoneing inquiry log event
        .set    mleframedropped,                        0x00D7+mledebug   # Frame Dropped
        .set    mlecpnsocketerror,                      0x00D8+mledebug   # Socket error
        .set    mlenvramwritten,                        0x00D9+mledebug   # NVRAM was written
        .set    mlehbeatstop,                           0x00DA+mledebug   # CCB heartbeat stop
        .set    mleparityscanrequired,                  0x00DB+mledebug   # Parity scan required
        .set    mlepdiskunfail_op,                      0x00DC+mledebug   # Loop Up
        .set    mlembfailed,                            0x00DD+mledebug   # Mailbox command failed
        .set    mlelip,                                 0x00DE+mledebug   # LIP Event
        .set    mleiocbto,                              0x00DF+mledebug   # IOCB timeout log event
#
        .set    mletasktoolong,                         0x00E0+mledebug   # Task took too long between exchanges
        .set    mlevlinknamechanged,                    0x00E1+mledebug   # Name chg notify - CCB to BEP
        .set    mlefileiodebug,                         0x00E2+mledebug   # Internal File I/O *warning*
        .set    mlelogtextmessagedebug,                 0x00E3+mledebug   # Log a text message
        .set    mlephyretry,                            0x00E4+mledebug   # Physical retry
        .set    mledvlist,                              0x00E5+mledebug   # Device List sent to online
        .set    mleportup,                              0x00E6+mledebug   # Qlogic Port Initialized successfully
        .set    mlephyaction,                           0x00E7+mledebug   # Physical action
        .set    mlenolicense,                           0x00E8+mledebug   # No license applied to this controller
        .set    mlenomirrorpartner,                     0x00E9+mledebug   # No Mirror Partner set up
        .set    mlelpdnretry,                           0x00EA+mledebug   # Loop Down reset retry
        .set    mlebootcodeeventdebug,                  0x00EB+mledebug   # Boot code generated event
        .set    mlevalidationdebug,                     0x00EC+mledebug   # Validation debug
        .set    mleloopprimitivedebug,                  0x00ED+mledebug   # Loop primitive debug
        .set    mleprocnamedeviceop,                    0x00EE+mleinform  # Change device name
        .set    mlehardwaremonitordebug,                0x00EF+mledebug   # HW Monitor Debug Message
#
        .set    mlehardwaremonitorstatusinfo,           0x00F0+mleinform
        .set    mleccbstatusinfo,                       0x00F1+mleinform
        .set    mleprocboardstatusinfo,                 0x00F2+mleinform
        .set    mlefebufferboardstatusinfo,             0x00F3+mleinform
        .set    mlebebufferboardstatusinfo,             0x00F4+mleinform
        .set    mlefepowersupplystatusinfo,             0x00F5+mleinform
        .set    mlebepowersupplystatusinfo,             0x00F6+mleinform
        .set    mlei2cstatusreserved1info,              0x00F7+mleinform
        .set    mlei2cstatusreserved2info,              0x00F8+mleinform
        .set    mlei2cstatusreserved3info,              0x00F9+mleinform
        .set    mlei2cstatusreserved4info,              0x00FA+mleinform
        .set    mlei2cstatusreserved5info,              0x00FB+mleinform
        .set    mlei2cstatusreserved6info,              0x00FC+mleinform
        .set    mlei2cstatusreserved7info,              0x00FD+mleinform
        .set    mlei2cstatusreserved8info,              0x00FE+mleinform
        .set    mlei2cstatusreserved9info,              0x00FF+mleinform
#
        .set    mlehardwaremonitorstatuswarn,           0x0100+mlewarn
        .set    mleccbstatuswarn,                       0x0101+mlewarn
        .set    mleprocboardstatuswarn,                 0x0102+mlewarn
        .set    mlefebufferboardstatuswarn,             0x0103+mlewarn
        .set    mlebebufferboardstatuswarn,             0x0104+mlewarn
        .set    mlefepowersupplystatuswarn,             0x0105+mlewarn
        .set    mlebepowersupplystatuswarn,             0x0106+mlewarn
        .set    mlei2cstatusreserved1warn,              0x0107+mlewarn
        .set    mlei2cstatusreserved2warn,              0x0108+mlewarn
        .set    mlei2cstatusreserved3warn,              0x0109+mlewarn
        .set    mlei2cstatusreserved4warn,              0x010A+mlewarn
        .set    mlei2cstatusreserved5warn,              0x010B+mlewarn
        .set    mlei2cstatusreserved6warn,              0x010C+mlewarn
        .set    mlei2cstatusreserved7warn,              0x010D+mlewarn
        .set    mlei2cstatusreserved8warn,              0x010E+mlewarn
        .set    mlei2cstatusreserved9warn,              0x010F+mlewarn
#
        .set    mlehardwaremonitorstatuserror,          0x0110+mlewarn
        .set    mleccbstatuserror,                      0x0111+mleerror
        .set    mleprocboardstatuserror,                0x0112+mleerror
        .set    mlefebufferboardstatuserror,            0x0113+mleerror
        .set    mlebebufferboardstatuserror,            0x0114+mleerror
        .set    mlefepowersupplystatuserror,            0x0115+mleerror
        .set    mlebepowersupplystatuserror,            0x0116+mleerror
        .set    mlei2cstatusreserved1error,             0x0117+mleerror
        .set    mlei2cstatusreserved2error,             0x0118+mleerror
        .set    mlei2cstatusreserved3error,             0x0119+mleerror
        .set    mlei2cstatusreserved4error,             0x011A+mleerror
        .set    mlei2cstatusreserved5error,             0x011B+mleerror
        .set    mlei2cstatusreserved6error,             0x011C+mleerror
        .set    mlei2cstatusreserved7error,             0x011D+mleerror
        .set    mlei2cstatusreserved8error,             0x011E+mleerror
        .set    mlei2cstatusreserved9error,             0x011F+mleerror
#
        .set    mlehardwaremonitorinfo,                 0x0120+mleinform
        .set    mleccbhwinfo,                           0x0121+mleinform
        .set    mleccbeepromhwinfo,                     0x0122+mleinform
        .set    mleccbmemorymodulehwinfo,               0x0123+mleinform
        .set    mleccbmemorymoduleeepromhwinfo,         0x0124+mleinform
        .set    mleprocboardlm80lm87hwinfo,             0x0125+mleinform
        .set    mleprocboardlm75lm92hwinfo,             0x0126+mleinform
        .set    mleprocboardpowersupplypcf8574hwinfo,   0x0127+mleinform
        .set    mleprocboardresetcontrolpcf8574hwinfo,  0x0128+mleinform
        .set    mlechassiseepromhwinfo,                 0x0129+mleinform
        .set    mleprocboardeepromhwinfo,               0x012A+mleinform
        .set    mlefepowersupplyeepromhwinfo,           0x012B+mleinform
        .set    mlebepowersupplyeepromhwinfo,           0x012C+mleinform
        .set    mlefebufferboardlm80lm87hwinfo,         0x012D+mleinform
        .set    mlefebufferboardpcf8574hwinfo,          0x012E+mleinform
        .set    mlefebufferboardmax1660hwinfo,          0x012F+mleinform
#
        .set    mlefebufferboardeepromhwinfo,           0x0130+mleinform
        .set    mlebebufferboardlm80lm87hwinfo,         0x0131+mleinform
        .set    mlebebufferboardpcf8574hwinfo,          0x0132+mleinform
        .set    mlebebufferboardmax1660hwinfo,          0x0133+mleinform
        .set    mlebebufferboardeepromhwinfo,           0x0134+mleinform
        .set    mlei2chwreserved1info,                  0x0135+mleinform
        .set    mlei2chwreserved2info,                  0x0136+mleinform
        .set    mlei2chwreserved3info,                  0x0137+mleinform
        .set    mlei2chwreserved4info,                  0x0138+mleinform
        .set    mlei2chwreserved5info,                  0x0139+mleinform
        .set    mlei2chwreserved6info,                  0x013A+mleinform
        .set    mlei2chwreserved7info,                  0x013B+mleinform
        .set    mlei2chwreserved8info,                  0x013C+mleinform
        .set    mlei2chwreserved9info,                  0x013D+mleinform
        .set    mlei2chwreserved10info,                 0x013E+mleinform
        .set    mlei2chwreserved11info,                 0x013F+mleinform
#
        .set    mlehardwaremonitorwarn,                 0x0140+mlewarn
        .set    mleccbhwwarn,                           0x0141+mlewarn
        .set    mleccbeepromhwwarn,                     0x0142+mlewarn
        .set    mleccbmemorymodulehwwarn,               0x0143+mlewarn
        .set    mleccbmemorymoduleeepromhwwarn,         0x0144+mlewarn
        .set    mleprocboardlm80lm87hwwarn,             0x0145+mlewarn
        .set    mleprocboardlm75lm92hwwarn,             0x0146+mlewarn
        .set    mleprocboardpowersupplypcf8574hwwarn,   0x0147+mlewarn
        .set    mleprocboardresetcontrolpcf8574hwwarn,  0x0148+mlewarn
        .set    mlechassiseepromhwwarn,                 0x0149+mlewarn
        .set    mleprocboardeepromhwwarn,               0x014A+mlewarn
        .set    mlefepowersupplyeepromhwwarn,           0x014B+mlewarn
        .set    mlebepowersupplyeepromhwwarn,           0x014C+mlewarn
        .set    mlefebufferboardlm80lm87hwwarn,         0x014D+mlewarn
        .set    mlefebufferboardpcf8574hwwarn,          0x014E+mlewarn
        .set    mlefebufferboardmax1660hwwarn,          0x014F+mlewarn
#
        .set    mlefebufferboardeepromhwwarn,           0x0150+mlewarn
        .set    mlebebufferboardlm80lm87hwwarn,         0x0151+mlewarn
        .set    mlebebufferboardpcf8574hwwarn,          0x0152+mlewarn
        .set    mlebebufferboardmax1660hwwarn,          0x0153+mlewarn
        .set    mlebebufferboardeepromhwwarn,           0x0154+mlewarn
        .set    mlei2chwreserved1warn,                  0x0155+mlewarn
        .set    mlei2chwreserved2warn,                  0x0156+mlewarn
        .set    mlei2chwreserved3warn,                  0x0157+mlewarn
        .set    mlei2chwreserved4warn,                  0x0158+mlewarn
        .set    mlei2chwreserved5warn,                  0x0159+mlewarn
        .set    mlei2chwreserved6warn,                  0x015A+mlewarn
        .set    mlei2chwreserved7warn,                  0x015B+mlewarn
        .set    mlei2chwreserved8warn,                  0x015C+mlewarn
        .set    mlei2chwreserved9warn,                  0x015D+mlewarn
        .set    mlei2chwreserved10warn,                 0x015E+mlewarn
        .set    mlei2chwreserved11warn,                 0x015F+mlewarn
#
        .set    mlehardwaremonitorerror,                0x0160+mleerror
        .set    mleccbhwerror,                          0x0161+mleerror
        .set    mleccbeepromhwerror,                    0x0162+mleerror
        .set    mleccbmemorymodulehwerror,              0x0163+mleerror
        .set    mleccbmemorymoduleeepromhwerror,        0x0164+mleerror
        .set    mleprocboardlm80lm87hwerror,            0x0165+mleerror
        .set    mleprocboardlm75lm92hwerror,            0x0166+mleerror
        .set    mleprocboardpowersupplypcf8574hwerror,  0x0167+mleerror
        .set    mleprocboardresetcontrolpcf8574hwerror, 0x0168+mleerror
        .set    mlechassiseepromhwerror,                0x0169+mleerror
        .set    mleprocboardeepromhwerror,              0x016A+mleerror
        .set    mlefepowersupplyeepromhwerror,          0x016B+mleerror
        .set    mlebepowersupplyeepromhwerror,          0x016C+mleerror
        .set    mlefebufferboardlm80lm87hwerror,        0x016D+mleerror
        .set    mlefebufferboardpcf8574hwerror,         0x016E+mleerror
        .set    mlefebufferboardmax1660hwerror,         0x016F+mleerror
#
        .set    mlefebufferboardeepromhwerror,          0x0170+mleerror
        .set    mlebebufferboardlm80lm87hwerror,        0x0171+mleerror
        .set    mlebebufferboardpcf8574hwerror,         0x0172+mleerror
        .set    mlebebufferboardmax1660hwerror,         0x0173+mleerror
        .set    mlebebufferboardeepromhwerror,          0x0174+mleerror
        .set    mlei2chwreserved1error,                 0x0175+mleerror
        .set    mlei2chwreserved2error,                 0x0176+mleerror
        .set    mlei2chwreserved3error,                 0x0177+mleerror
        .set    mlei2chwreserved4error,                 0x0178+mleerror
        .set    mlei2chwreserved5error,                 0x0179+mleerror
        .set    mlei2chwreserved6error,                 0x017A+mleerror
        .set    mlei2chwreserved7error,                 0x017B+mleerror
        .set    mlei2chwreserved8error,                 0x017C+mleerror
        .set    mlei2chwreserved9error,                 0x017D+mleerror
        .set    mlei2chwreserved10error,                0x017E+mleerror
        .set    mlei2chwreserved11error,                0x017F+mleerror
#
        .set    mlediagresult,                          0x0180+mleinform
        .set    mleccbmemoryhealthalert,                0x0181+mleerror   # CCB memory health
        .set    mlediskbayremoved,                      0x0182+mlewarn    # Diskbay removed log event
        .set    mlediskbayinserted,                     0x0183+mleinform  # Diskbay inserted log event
        .set    mleprocmemoryhealthalert,               0x0184+mleerror   # Proc memory health
        .set    mlediskbaymoved,                        0x0185+mleinform  # Diskbay moved log event
        .set    mlepcicfgerr,                           0x0186+mleerror   # PCI set config error
        .set    mlefcalmonitor,                         0x0187+mlewarn    # FCAL health monitor
        .set    mlehsdepleted,                          0x0188+mleerror   # Hotspares depleted
        .set    mlecstoplog,                            0x0189+mledebug   # C$Stop recovery step log
        .set    mlecontrollerfailed,                    0x018A+mleerror   # CCB Controller failed
        .set    mlecontrollerdisaster,                  0x018B+mleerror   # CCB Controller disaster
        .set    mlemirrorcapable,                       0x018C+mleinform  # Now capable to mirror to Mirror Partner
#UNUSED .set    mlemovingtargets,                       0x018D+mledebug   # Moving Targets log message
#UNUSED .set    mleprocprintf,                          0x018E+mledebug   # Proc code debug printf message
        .set    mledrvflt,                              0x018F+mledebug   # Drive fault/reset FRU code 0xCC
#
        .set    mlebypassdevice,                        0x0190+mledebug   # Bypass Device action
        .set    mlevlinkopenbegin,                      0x0191+mledebug   # VLink Open Process beginning
        .set    mlevlinkopenend,                        0x0192+mledebug   # VLink Open Process complete
        .set    mleR5inop,                              0x0193+mleerror   # RAID 5 Inoperative Log Message
        .set    mlenvramwritefail,                      0x0194+mleerror   # NVRAM write verification failed
        .set    mleparitycheckraid,                     0x0195+mledebug   # Parity Chk RAID Begin, end, term
        .set    mleispj2,                               0x0196+mlewarn    # ISP 23xx J2 jumper set incorrectly
        .set    mleOrphanDetected,                      0x0197+mledebug   # Orphan detected
        .set    mleDefragVerDone,                       0x0198+mledebug   # Defrag Verification step complete
        .set    mleDefragOpComplete,                    0x0199+mleinform  # Defrag verification step done
        .set    mleNVMemEvent,                          0x019A+mleerror   # MM Fatal Event detected
        .set    mleWCFlushlog,                          0x019B+mledebug   # Flush Taking Long Log
        .set    mleWCSNVCGBad,                          0x019C+mleerror   # WC - swapped with other CN in VCG
        .set    mleWCSNBad,                             0x019D+mleerror   # WC - swapped with foreign CN
        .set    mleWCNVMEMBad,                          0x019E+mleerror   # WC - NV Memory unavailable
        .set    mledevreset,                            0x01A8+mlewarn    # Device reset log event
        # 0x1A9 - 0x01AD are used by iSCSI
#
#
        .set    mlescsitimeout,                         0x01B2+mleerror   # SCSI command timed out
        .set    mleiclportevent,                        0x01BC+mleinform  # ICL port event
        .set    mlePRClr,                               0x01C3+mleinform  # Clear PR for VID
        .set    mleAPOOLChanged,                        0x01C4+mleinform  # Log message about APOOL change.
                                                        # 0x01C5 used for APOOL message
                                                        # 0x01C6 used for APOOL message
                                                        # 0x01C7 used for APOOL message
                                                        # 0x01C8 used for APOOL message
        .set    mleispfatal,                            0x01C9+mleerror   # Fatal error from qlogic card
        .set    mlecopylabel,                           0x01CA+mleinform # Info message about copy label/description setting
        .set    mleSPOOL_CHANGE_I,                      0x01CB+mleinform
        .set    mleSPOOL_CHANGE_W,                      0x01CC+mlewarn
        .set    mleSPOOL_CHANGE_E,                      0x01CD+mleerror
        .set    mleSPOOL_CHANGE_D,                      0x01CE+mledebug
        .set    mledrivedelay,                          0x01CF+mleerror
#
        .set    mledelayinop,                           0x01D0+mleerror
        .set    mleiseelemchg,                          0x01D2+mlewarn    # ISE environment change
        .set    mleRaidinop,                            0x01D3+mleerror   # RAID Inoperative Log Message
#
        .set    mleglobalcachemode,                     0x01D8+mleinform  # Global Cache Mode
        .set    mlegr,                                  0x01D9+mleinform  # Georaid event
        .set    mlecm,                                  0x01DA+mleinform  # Copy Manager event (Instant mirror)
#
##
# ICL port events/states
##
        .set logiclportinitok,     1
        .set logiclportinitfailed, 2
        .set logiclportloopup,     3
        .set logiclportloopdown,   4
        .set logiclportchipreset,  5
#
#
# --- Begin Logging base structure ------------------------------------
        .set    mle_event,0             # Event type                <s>
#       reserved 2
        .set    mle_len,mle_event+4     # Logging event length      <w>
        .set    mle_bstream,mle_len+4   # Byte stream beginning     <*>
# --- End base structure ----------------------------------------------
        .set    mlesiz,mle_bstream      # Base size
        .set    mlemaxsiz,mle_bstream+64# Max size
#
# --- Begin Logging response base structure ---------------------------
#       reserved 3
#--     .set    mr_status,mr_status     # Response status           <b>
#--     .set    mr_rlen,mr_status+1     # Return length             <w>
# --- End base structure ----------------------------------------------
        .set    mlersiz,mr_rlen+4       # Size of response
#
# --- IOCB Entry (mleiocb) --------------------------------------------
        .set    eiocb_port,mle_bstream     # FC port number         <b>
#       reserved 1
        .set    eiocb_proc,eiocb_port+2    # FE or BE indicator     <b>
#       reserved 1
        .set    eiocb_reason,eiocb_proc+2  # Reason Code            <w>
        .set    eiocb_data,eiocb_reason+4  # IOCB data            64<b>
        .set    eiocblen,eiocb_data+64     # Total size
#
# --- Short SCSI event (mleshortscsi) ---------------------------------
        .set    ess_prpstat,mle_bstream # PRP status                <b>
        .set    ess_scsistat,ess_prpstat+1 # SCSI status            <b>
        .set    ess_qstatus,ess_scsistat+1 # QLogic status          <b>
        .set    ess_retry,ess_qstatus+1    # Retries remaining      <b>
        .set    ess_port,ess_retry+1       # FC port number         <b>
        .set    ess_lun,ess_port+1      # LUN                       <b>
        .set    ess_pid,ess_lun+1       # pid from pdd              <s>
        .set    ess_id,ess_pid+2        # FC ID                     <w>
        .set    ess_wwn,ess_id+4        # FC WWN                    <l>
        .set    ess_cdb,ess_wwn+8       # SCSI cdb                 4<w>
        .set    esslen,ess_cdb+16       # Total size
#
# --- Long SCSI event (mlelongscsi) -----------------------------------
        .set    esl_prpstat,mle_bstream # PRP status                <b>
        .set    esl_scsistat,esl_prpstat+1 # SCSI status            <b>
        .set    esl_qstatus,esl_scsistat+1 # QLogic status          <b>
#       reserved 1
        .set    esl_port,esl_qstatus+2  # FC port number            <b>
#       reserved 1
        .set    esl_lun,esl_port+2      # LUN                       <s>
        .set    esl_id,esl_lun+2        # FC ID                     <w>
        .set    esl_wwn,esl_id+4        # FC WWN                    <l>
        .set    esl_cdb,esl_wwn+8       # SCSI cdb                 4<w>
        .set    esl_sense,esl_cdb+16    # SCSI sense data          8<w>
        .set    esllen,esl_sense+32     # Total size
#
# --- Device Timeout (mleiocbto) ---------------------------------------
        .set    eto_port,mle_bstream    # FC port number            <b>
#       reserved 1
        .set    eto_proc,eto_port+2     # FE or BE indicator        <b>
#       reserved 1
        .set    eto_iocb,eto_proc+2     # IOCB Type                 <b>
#       reserved 1
        .set    eto_ilt,eto_iocb+2      # ILT (IOCB handle)         <w>
        .set    eto_lun,eto_ilt+4       # LUN                       <s>
        .set    eto_id,eto_lun+2        # FC ID                     <w>
        .set    eto_wwn,eto_id+4        # FC WWN                    <l>
        .set    eto_cdb,eto_wwn+8       # SCSI cdb                 4<w>
        .set    etolen,eto_cdb+16       # Total size
#
# --- SMART enunciation (mlesmart) ------------------------------------
        .set    esu_port,mle_bstream    # FC port number            <b>
#       reserved 1
        .set    esu_lun,esu_port+2      # LUN                       <s>
        .set    esu_id,esu_lun+2        # FC ID                     <w>
        .set    esu_wwn,esu_id+4        # FC WWN                    <l>
        .set    esu_cdb,esu_wwn+8       # SCSI cdb                 4<w>
        .set    esu_sense,esu_cdb+16    # SCSI sense data          8<w>
        .set    esulen,esu_sense+32     # Total size
#
# --- Scrub cycle complete (mlescrubdone) -----------------------------
        .set    esclen,mle_bstream      # Total size
#
# --- Device missing (mledevmissing) ----------------------------------
        .set    edx_channel,mle_bstream # FC port number            <b>
        .set    edx_rsvd,edx_channel+1  # Reserved                  <b>
        .set    edx_lun,edx_rsvd+1      # LUN                       <s>
        .set    edx_id,edx_lun+2        # FC ID                     <w>
        .set    edx_wwn,edx_id+4        # World wide name           <l>
        .set    edxlen,edx_wwn+8        # Total size
#
# --- NVRAM checksum error (mlenvramcsum) -----------------------------
        .set    enclen,mle_bstream      # Total size
#
# --- NVRAM restore (mlenvramrestore) ---------------------------------
        .set    enrlen,mle_bstream      # Total size
#
# --- LED change (mleled) ---------------------------------------------
        .set    elc_wwn,mle_bstream     # FC world wide name        <l>
        .set    elc_state,elc_wwn+8     # LED state                 <b>
#       reserved 1
        .set    elc_lun,elc_state+2     # Total size                <s>
        .set    elclen,elc_lun+2        # Total size
#
# --- Basic boot complete (mleboot) -----------------------------------
        .set    ebclen,mle_bstream      # Total size
#
# --- Device failed w/ hot spare (mledevfailhs) -----------------------
        .set    edh_channel,mle_bstream # FC port number            <b>
#       reserved 1                        must follow channel       <b>
        .set    edh_lun,edh_channel+2   # LUN - must follow channel <s>
        .set    edh_pid,edh_lun+2       # PID                       <s>
#       reserved 2                                                  <s>
        .set    edh_id,edh_pid+4        # FC ID                     <w>
        .set    edh_wwn,edh_id+4        # World wide name           <l>
        .set    edh_hschan,edh_wwn+8    # Hot spare FC port         <b>
#       reserved 1                        must follow hschan        <b>
        .set    edh_hslun,edh_hschan+2  # LUN - must follow hschan  <s>
        .set    edh_hspid,edh_hslun+2   # Hot spare PID             <s>
#       reserved 2                                                  <s>
        .set    edh_hsid,edh_hspid+4    # Hot spare FC ID           <w>
        .set    edh_hswwn,edh_hsid+4    # Hot spare FC WWN          <l>
        .set    edhlen,edh_hswwn+8      # Total size
#
# --- Device failed but no hot spare available (mledevfailnhs) --------
        .set    edn_channel,mle_bstream # FC port number            <b>
#       reserved 1                        must follow channel       <b>
        .set    edn_lun,edn_channel+2   # LUN - must follow channel <s>
        .set    edn_pid,edn_lun+2       # PID                       <s>
#       reserved 2                                                  <s>
        .set    edn_rid,edn_pid+4       # RAID ID                   <s>
        .set    edn_vid,edn_rid+2       # Virtual device ID         <s>
        .set    edn_id,edn_vid+2        # FC ID                     <w>
        .set    edn_wwn,edn_id+4        # World wide name           <l>
        .set    ednlen,edn_wwn+8        # Total size
#
# --- Device removed (mledevremoved) ----------------------------------
        .set    edr_channel,mle_bstream # FC port number            <b>
#       reserved 1
        .set    edr_lun,edr_channel+2   # LUN                       <s>
        .set    edr_id,edr_lun+2        # FC ID                     <w>
        .set    edr_wwn,edr_id+4        # FC WWN                    <l>
        .set    edr_pid,edr_wwn+8       # PID                       <s>
        .set    edr_type,edr_pid+2      # Device Type               <b>
        .set    edrlen,edr_type+1       # Total size
#
# --- Device inserted (mledevinsert) ----------------------------------
        .set    edi_channel,mle_bstream # FC port number            <b>
        .set    edi_geoflags,edi_channel+1 # GEORAID                <b>
#       reserved 1
        .set    edi_lun,edi_channel+2   # LUN                       <s>
        .set    edi_id,edi_lun+2        # FC ID                     <w>
        .set    edi_wwn,edi_id+4        # FC WWN                    <l>
        .set    edi_pid,edi_wwn+8       # PID                       <s>
        .set    edi_type,edi_pid+2      # Device Type               <b>
        .set    edilen,edi_type+1       # Total size
#
# --- Device reattached (mledevreattach) ------------------------------
        .set    eda_channel,mle_bstream # FC port number            <b>
#       reserved 1
        .set    eda_lun,eda_channel+2   # LUN                       <s>
        .set    eda_id,eda_lun+2        # FC ID                     <w>
        .set    eda_wwn,eda_id+4        # FC WWN                    <l>
        .set    eda_pid,eda_wwn+8       # PID                       <s>
        .set    eda_type,eda_pid+2      # Device Type               <b>
#       reserved 4
        .set    edalen,eda_type+5        # Total size
#
# --- System working (mlepulse) ---------------------------------------
        .set    epllen,mle_bstream      # Total size
#
# --- REDI copy complete (mlecopycomplt) ------------------------------
# --- REDI copy failed (mlecopyfailed) --------------------------------
# --- REDI copy sync (mlecopysync) ------------------------------------
        .set    ecs_compstat,mle_bstream    # Completion status     <b>
        .set    ecs_function,ecs_compstat+1 # function code         <b>
        .set    ecs_percent,ecs_function+1  # Percentage complete   <b>
#       reserved 1                                                  <b>
        .set    ecs_copyvid,ecs_percent+2   # Source VID            <s>
        .set    ecs_destvid,ecs_copyvid+2   # Destination VID       <s>
        .set    ecslen,ecs_destvid+2        # Total size
#
# --- SNAPPOOL Messages -----------------------------------------------
# following are submessage codes from LOG_Defs.h
.set SS_BUFFER_OK_NOW,  1          # INFO  Message: "SS Buffer #x (VID=y) is now operational"
.set SS_BUFFER_BAD,     0x81       # ERROR Message: "SS Buffer #x (VID=y) is inoperable"
.set SS_BUFFER_FULL_OK, 0x4        # INFO  Message: "SS Buffer is <80 full"
.set SS_BUFFER_FULL_WARN,0x44      # WARN  Message: "SS Buffer is >=80% full"
.set SS_BUFFER_FULL,    0x84       # ERROR Message: "SS Buffer is 100% full"
.set SS_BUFFER_EMPTY,   5          # DEBUG Message: "SS Buffer is empty"
.set SS_BUFFER_SET,     6          # INFO  Message: "SS Buffer %x (VID=%y) is defined"
.set SS_BUFFER_UNSET,   7          # INFO  Message: "SS Buffer %x (VID=%y) is unset"
.set SS_CREATE_GOOD,    0x3C       # INFO  Message: "Batch SS Created (Src=%d, SS=%d)"
.set SS_FATAL_ERROR,    0xA0       # ERROR Message: "Unexpected SS Error: Errorcode=%xx"
.set SS_BUFFER_OWNER_SET,    0xa   # INFO  Message: "SS Buffer %d Ownership Set"
.set SS_BUFFER_OWNER_UNSET,  0xb   # INFO  Message: "SS Buffer %d Ownership Released"
.set SS_NVRAM_RESTORED_OK,   0xc   # INFO  Message: "SS NVRAM Restored OK"
.set SS_NVRAM_RESTORED_BAD,  0xd   # ERROR Message: "SS NVRAM Restore Failed. EC=%d.%d.%d"
.set SS_NVRAM_UNCONFIGURED,  0xe   # INFO  Message: "SS NVRAM is unconfigured."
.set SS_NVRAM_INITIALIZED,   0xf   # INFO  Message: "SS NVRAM Initialized."
.set SS_CREATE_FAIL,         0xCC  # ERROR Message: "Batch SS Create Failed (Src=%d, SS=%d) (ec=%d)"
#
# -- Severity definitions --------
.set  log_as_info,            0    # Information
.set  log_as_warning,         1    # Warning
.set  log_as_error,           2    # Error
.set  log_as_debug,           4    # Debug
#
# typedef struct LOG_SPOOLEVENT_DAT
# {
#     UINT8    sub_event_code;        /**< extended event codes (see SS_EVENT_CODES list) */
#     UINT8    severity;              /**< severity: 0-info, 1-warning, 2=error, 4=debug */
#     UINT8    skip_notify;           /**< future use to restrict events if too many for ewok */
#     UINT8    rsvd;
#     INT32    errorCode;             /**< extended error code if needed */
#     INT32    value1;                /**< misc value depending on sub event code */
#     INT32    value2;                /**< misc value depending on sub event code */
# } LOG_SPOOLEVENT_DAT;
#
        .set    esp_sub_event_code,mle_bstream      # Completion status     <b>
        .set    esp_severity,esp_sub_event_code+1   # sev:debug,info,warn.. <b>
        .set    esp_skip_notify,esp_severity+1      # future simplification <b>
#       reserved 1                                                          <b>
        .set    esp_errorCode,esp_skip_notify+2     # error code            <w>
        .set    esp_value1,esp_errorCode+4          # extra info            <w>
        .set    esp_value2,esp_value1+4             # extra info            <w>
#
        .set    esplen,esp_value2+4        # Total size
#
# --- System serial number mismatch (mleserialwrong) ------------------
        .set    esm_channel,mle_bstream # FC port number            <b>
#       reserved 1
        .set    esm_lun,esm_channel+2   # LUN                       <s>
        .set    esm_id,esm_lun+2        # FC ID                     <w>
        .set    esm_wwn,esm_id+4        # World wide name           <l>
        .set    esm_sserial,esm_wwn+8   # System serial number      <w>
        .set    esmlen,esm_sserial+4    # Total size
#
# --- NVA checksum bad (mlenvabad) ------------------------------------
#     Byte definitions for ena_proc and ena_part.
        .set    enafe,0x0001                # FE
        .set    enabe,0x0002                # BE
        .set    enapart3,0x0300             # Part 3 NVRAM
        .set    enapart4,0x0400             # Part 4 NVRAM
#
        .set    ena_proc,mle_bstream        # Proc - FE or BE       <b>
        .set    ena_part,ena_proc+1         # NVRAM part - 3 or 4   <b>
#       reserved 2                                                  <s>
        .set    enalen,ena_part+3           # Total size
#
# --- Offline (mlehostoffline) ----------------------------------------
        .set    eol_cnum,mle_bstream        # Channel #             <b>
#       reserved 3                                                 3<b>
        .set    eollen,eol_cnum+4           # Total size
#
# --- Immediate Notify (mlehostimmednotify) ---------------------------
        .set    ein_cnum,mle_bstream        # Channel #             <b>
#       reserved 1                                                  <b>
        .set    ein_taskflags,ein_cnum+2    # Task flags            <b>
        .set    ein_qstatus,ein_taskflags+1 # QLogic status         <b>
        .set    einlen,ein_qstatus+1        # Total size
#
# --- MagDriver error (mlemagdriver) ----------------------------------
# --- Host nonSense error (mlehostnonsense) ---------------------------
# --- Host QLogic error (mlehostqlogic) -------------------------------
# --- Host sense error (mlehostsense) ---------------------------------
        .set    emd_cnum,mle_bstream        # Channel #             <b>
        .set    emd_errcode,emd_cnum+1      # Error code            <b>
        .set    emd_vid,emd_errcode+1       # Virtual Drive ID      <s>
        .set    emd_wwn,emd_vid+2           # World Wide Name       <l>
        .set    emd_tid,emd_wwn+8           # Target #              <s>
        .set    emd_cnt,emd_tid+2           # Count of times seen   <w>
        .set    emdlen,emd_cnt+4            # Total size
        .set    ems_sense,emd_tid+2         # SCSI sense data      5<w>
        .set    emslen,ems_sense+20         # Total size
#
# --- Zone Inquiry (mlezoneinquiry) -----------------------------------
        .set    ezi_cnum,mle_bstream        # Channel #             <b>
#       reserved 1                                                  <b>
        .set    ezi_tid,ezi_cnum+2          # Target ID             <s>
        .set    ezi_owner,ezi_tid+2         # Owner                 <w>
        .set    ezi_wwn,ezi_owner+4         # World Wide Name       <l>
        .set    ezi_name,ezi_wwn+8          # iSCSI Server name  <256b>
        .set    ezilen,ezi_name+256         # Total size
#
# --- Cache Flush Failed (mlecacheflushfail) --------------------------
# --- Cache Flush Recovered (mlecacheflushrec) ------------------------
        .set    ecf_vid,mle_bstream         # Virtual ID number     <s>
        .set    ecf_ecode,ecf_vid+2         # Error Code            <b>
#       reserved 1
        .set    ecflen,ecf_ecode+2          # Total size
#
# --- Firmware alert (mlefirmwarealert) -------------------------------
        .set    efa_ec,mle_bstream          # firmware alert error code <w>
        .set    efa_data,efa_ec+4           # firmware alert data     15<w>
        .set    efalen,mle_bstream+64       # Total size
#
#  Firmware Alert Error Code Definitions (unique for every location called)
#       CCB is assigned 0x00000000 to 0x7FFFFFFF
        .set    PROCSFT,0x80000000          # Beginning of PROC Error Codes
#
        .set    dlm_sft1,PROCSFT+0x00       # DLM device capacity too large #1
        .set    dlm_sft2,PROCSFT+0x01       # DLM device capacity too large #2
        .set    dlm_sft3,PROCSFT+0x02       # DLM device capacity too large #3
        .set    dlm_sft4,PROCSFT+0x03       # DLM device capacity too large #4
        .set    dlm_sft5,PROCSFT+0x04       # DLM device capacity too large #5
        .set    dlm_sft6,PROCSFT+0x05       # DLM device capacity too large #6
        .set    dlm_sft7,PROCSFT+0x06       # DLM device capacity too large #7
        .set    dlm_sft8,PROCSFT+0x07       # DLM device capacity too large #8
        .set    dlm_sft9,PROCSFT+0x08       # DLM LDD/VID not corelated #1
        .set    dlm_sft10,PROCSFT+0x09      # DLM LDD/VID not corelated #2
        .set    dlm_sft11,PROCSFT+0x0A      # DLM SDA too large
        .set    dlm_sft12,PROCSFT+0x0B      # DLM VID/LDD not corelated
        .set    dlm_sft13,PROCSFT+0x0C      # DLM device capacity too large #9
        .set    dlm_sft14,PROCSFT+0x0D      # DLM Poll Unexpected Error
        .set    dlm_sft15,PROCSFT+0x0E      # DLM extended SGL looks bad
                                            # DLM reserved 15-31
        .set    magd_sft1,PROCSFT+0x20      # Magdrvr inquiry VIDs not match
        .set    magd_sft2,PROCSFT+0x21      # Magdrvr capacity too large #1
        .set    magd_sft3,PROCSFT+0x22      # Magdrvr capacity too large #2
        .set    magd_sft4,PROCSFT+0x23      # Magdrvr capacity too large #3
        .set    magd_sft5,PROCSFT+0x24      # Magdrvr unsupported Receive Diag
        .set    magd_sft6,PROCSFT+0x25      # Magdrvr unsupported Log Sense
        .set    magd_sft7,PROCSFT+0x26      # Magdrvr unsupported Log Select
        .set    magd_sft8,PROCSFT+0x27      # Magdrvr - build_maci - max VID
        .set    magd_sft9,PROCSFT+0x28      # Magdrvr - build_maci - max LUN
        .set    magd_sft10,PROCSFT+0x29     # Magdrvr - build_maci - no VDMT
        .set    magd_sft11,PROCSFT+0x2A     # Magdrvr - build_maci - used LUN
                                            # Magdrvr reserved 11-31
        .set    cac_sft1,PROCSFT+0x40       # Cache - Flush all took > 2 minutes
        .set    cac_sft2,PROCSFT+0x41       # Cache - Invalid VID in BE Tags
        .set    cac_sft3,PROCSFT+0x42       # Cache - More than one Bad BE Tag
        .set    cac_sft4,PROCSFT+0x43       # Cache - SWI ILT not found in list
        .set    cac_sft5,PROCSFT+0x44       # Cache - SWI too many ILTs
        .set    cac_sft6,PROCSFT+0x45       # Cache - BE Tag overlaps other Tag
        .set    cac_sft7,PROCSFT+0x46       # Cache - Rebuild Check Unexp Error
        .set    cac_sft8,PROCSFT+0x47       # Cache - Mirror BE NV Card Wrong
        .set    cac_sft9,PROCSFT+0x48       # Cache - WRP invalid data
        .set    cac_sft10,PROCSFT+0x49      # Cache - WRP invalid VID
        .set    cac_sft11,PROCSFT+0x4A      # Cache - ILT not on Queue
        .set    cac_sft12,PROCSFT+0x4B      # Cache - Invalid WRP VID
                                            # Cache reserved 12-31
        .set    isp_sft1,PROCSFT+0x60       # ISP - Chip reset
                                            # ISP reserved 1-31
        .set    nva_sft1,PROCSFT+0x80       # NVA - Length too large to handle
                                            # NVA reserved 1-31
#
# ---  Cache DRAM failed (mlecacheDRAMfail) ---------------------------
        .set    edf_status,mle_bstream      # Error status
        .set    edflen,edf_status+4         # Total size
#
# --- Cache recovery fault (mlecacherecoverfail) ----------------------
        .set    erf_lba,mle_bstream         # LBA
        .set    erf_length,erf_lba+8        # length
        .set    erf_status,erf_length+4     # Error Status
        .set    erf_mode,erf_status+1       # Recovery Mode
        .set    erf_vid,erf_mode+1          # VID
        .set    erflen,erf_vid+2            # Total size
#
# --- Error trap (mleerrtrap) and SystemNMI (mlesystemnmi)----------------------
# Notes: mlesystemnmi does not get a specific structure.  It will use this
#        error trap structure.
        .set    eet_data,mle_bstream        # errtrap data        36<w>
        .set    eetlen,eet_data+(4*(4+32))  # Total size - 4 words + 32 regs
#
# --- Loop Up (mleloopup) ---------------------------------------------
        .set    elu_port,mle_bstream        # FC port number        <b>
        .set    elu_flags,elu_port+1        # Flags (loop/fabric)   <b>
        .set    elu_proc,elu_flags+1        # FE or BE indicator    <b>
        .set    elu_failed,elu_proc+1       # Failed indicator      <b>
        .set    elu_lid,elu_failed+1        # Loop ID               <s>
        .set    elu_state,elu_lid+2         # login state           <s>
        .set    elu_lpmap,elu_state+2       # Loop Map
        .set    elulen,mlemaxsiz            # Total size
#
# --- Loop Down failure (mleloopdown) ---------------------------------
        .set    eld_port,mle_bstream        # FC port number        <b>
#       reserved 1                                                  <b>
        .set    eld_proc,eld_port+2         # FE or BE indicator    <b>
#       reserved 1                                                  <b>
        .set    eldlen,eld_proc+2           # Total size
#
# --- PSD Rebuild Done (mlepsdrebuildstrt) (mlepsdrebuilddone) --------
# --- epr_errcode bit definitions (See LOG_Defs.h, RB_ECOK as an example.)
        .set    eprecok,0                   # No error (value, not bit)
        .set    eprecbusy,0                 # 0x0001 - Not used
        .set    eprecinop,1                 # 0x0002 - Drive being rebuilt inoperable
        .set    eprecbadtype,2              # 0x0004 - RAID type is not rebuildable
        .set    eprecwrfault,3              # 0x0008 - Write fault on R10
        .set    eprecnotready,4             # 0x0010 - R5 not operative
        .set    eprecr5inop,5               # 0x0020 - R5 became inoperable
        .set    eprecdeleted,6              # 0x0040 - RBR deleted
        .set    epreccancel,7               # 0x0080 - RBR cancelled
        .set    eprecdelay,8                # 0x0100 - RBR delayed too long
        .set    eprecowner,9                # 0x0200 - VID ownership changed
        .set    eprecdone,10                # 0x0400 - PSD already done rebuilding
#
        .set    epr_rid,mle_bstream         # RAID ID               <s>
        .set    epr_vid,epr_rid+2           # Virtual device ID     <s>
#
        .set    epr_lun,epr_vid+2           # LUN                   <s>
        .set    epr_errcode,epr_lun+2       # Error code            <s>
#
        .set    epr_wwn,epr_errcode+2       # WWN                   <l>
        .set    eprlen,epr_wwn+8            # Total size
#
# --- PDD Rebuild Done and hot spare complete (mlepddrebuilddone) -----
#       reserved 4
        .set    eph_lun,mle_bstream+4       # LUN                   <s>
        .set    eph_pid,eph_lun+2           # PID                   <s>
        .set    eph_wwn,eph_pid+2           # WWN                   <l>
        .set    ephlen,eph_wwn+8            # Total size
#
# --- RAID initialization complete (mleraidinitdone) ------------------
#       eri_status
        .set    erigood,0
        .set    eriterm,1
        .set    erifail,2
#
        .set    eri_status,mle_bstream      # Completion status     <b>
#       reserved 3                                                 3<b>
        .set    eri_rid,eri_status+4        # RAID ID               <s>
        .set    eri_vid,eri_rid+2           # Virtual disk ID       <s>
        .set    eri_ecnt,eri_vid+2          # Error count           <w>
#
        .set    erilen,eri_ecnt+4           # Total size
#
# --- ISP Chip reset , Port init failure (mleispchipreset) (mleportinitfailed)
#           (mleportevent) (mleportup)
        .set    ecri,0                      # Reset and initialize
        .set    ecro,1                      # Reset only
        .set    ecrild,2                    # Reset and initialize if offline
        .set    ecrold,3                    # Reset only if offline
# NOTE:  Log events are generated for values > ecrold
        .set    ecrdebug,4                  # Reset, initialize, and log
        .set    ecrdebugro,5                # Reset and log
        .set    ecrgood,0x0000              # QLogic Initialize Successful
#
        .set    ecrfwhb,0x0101              # Firmware Heartbeat failure
        .set    ecrcmdto,0x0102             # Command timeout
        .set    ecrfatal,0x0103             # Fatal Error
        .set    ecrasyqof,0x104             # Async queue overlow
        .set    ecrbadiocb,0x105            # Bad IOCB type
        .set    ecrnoiltiocb,0x106          # No ILT in IOCB
        .set    ecrnoiltcr,0x107            # No ILT completion routine
        .set    ecrthread,0x108             # ILT unthread error
        .set    ecrqinerr,0x109             # Request Queue IN ptr out of range
        .set    ecrqouterr,0x10A            # Response Queue OUT ptr out of range
        .set    ecrmboxtmo,0x10B            # Placeholder, used by 'C' code only
        .set    ecrinvganportid,0x10C       # Bad Port ID in GAN response
        .set    ecrriscpaused,0x10D         # ISP risc paused
#
        .set    ecrbaddev,0x201             # Bad device
        .set    ecrdead,0x202               # No-op command failed
        .set    ecrmbxfail,0x203            # mailbox test failed
        .set    ecrnofw,0x204               # No firmware found
        .set    ecrloadram,0x205            # Load Ram command failed
        .set    ecrchecksum,0x206           # Verify firmware checksum failed
        .set    ecrabout,0x207              # About firmware command failed
        .set    ecrinitcmd,0x208            # Init mailbox command failed
        .set    ecrsfocmd,0x209             # Set firmware options command failed
#
        .set    eclpnoloop,0x301            # No Loop after power on
        .set    eclpdnretry,0x302           # Loop down retry
        .set    eclpdnfail,0x303            # Loop down retry failed
#
        .set    ecr_port,mle_bstream        # FC port number     <b>
#       reserved 1                                                  <b>
        .set    ecr_proc,ecr_port+2         # FE or BE indicator    <b>
#       reserved 1                                                  <b>
        .set    ecr_reason,ecr_proc+2       # Reason Code           <w>
        .set    ecr_count,ecr_reason+4      # Count                 <w>
        .set    ecrlen,ecr_count+4          # Total size
#
# --- Initiator detected on Back End (mlebeinitiator) -----------------
        .set    eib_port,mle_bstream        # FC port number     <b>
#       reserved 3                                                 3<b>
        .set    eib_lid,eib_port+4          # LID                   <w>
        .set    eib_wwn,eib_lid+4           # World Wide Name       <l>
        .set    eiblen,eib_wwn+8            # Total size
#
# --- Change name (mlechangename) -------------------------------------
        .set    ecnctrl,0x0100              # Controller name type
        .set    ecnvdisk,0x0008             # Virtual disk name type
        .set    ecnvlink,0x0200             # Virtual link name type
#
        .set    ecn_vlid,mle_bstream        # Name VLink ID         <s>
        .set    ecn_type,ecn_vlid+2         # Type                  <s>
        .set    ecn_ctrlname,ecn_type+2     # Controller name      5<w>
        .set    ecn_vdname,ecn_ctrlname+20  # VDisk name          13<w>
        .set    ecnlen,ecn_vdname+52        # Total size
        .set    ecnplen,ecnlen-mle_bstream  # Parameter size
#
#       Return data for get name
#--     .set    mr_status,mr_status         # Status byte
#--     .set    mr_rlen,mr_rlen             # Return length
        .set    ecnrlen,mr_rlen+4           # Size of return data
#
# --- NVRAM write complete (mlenvramwritten) --------------------------
        .set    evclen,mle_bstream          # No data in packet
#
# --- CCB Heartbeat Stop (mlehbeatstop) -------------------------------
# --- CCB Heartbeat Stop constants
        .set    ehsprocfe,0
        .set    ehsprocbe,1
#
        .set    ehs_proc,mle_bstream        # Processor (FE or BE)          <b>
#       reserved 3
        .set    ehslen,ehs_proc+4           # Total size
#
# --- Cache Mirror Failed (mlecachemirrorfailed) ----------------------
        .set    emf_controller_sn,mle_bstream # Controller Serial Number    <w>
        .set    emf_ilt_status,emf_controller_sn+4 # The g0 status value    <w>
        .set    emf_drp_status,emf_ilt_status+4 # DRP status code           <b>
        .set    emf_dg_status,emf_drp_status+1 # Datagram status code       <b>
        .set    emf_dg_ec1,emf_dg_status+1  # Datagram Error Code #1        <b>
        .set    emf_dg_ec2,emf_dg_ec1+1     # Datagram Error Code #2        <b>
#
        .set    emflen,emf_dg_ec2+1         # Total size
#
# --- Parity scanner pass completed (mleparitychkdone) ----------------
        .set    epc_misc,mle_bstream        # Miscompare count
        .set    epc_pass,epc_misc+4         # Pass number
        .set    epc_startraid,epc_pass+4    # Starting RAID ID
        .set    epc_prevctrl,epc_startraid+4# Previous control value
        .set    epc_invfunc,epc_prevctrl+4  # Number of invalid functions
        .set    epc_reserved,epc_invfunc+4  # Number of reserved errors
        .set    epc_other,epc_reserved+4    # Number of other errors
        .set    epclen,epc_other+4          # Total size
#
# --- Local Image Ready (mlelclimagerdy) ------------------------------
        .set    eli_len,mle_bstream         # Length of lcl image   <w>
        .set    eli_ptr,eli_len+4           # Pointer to lcl image  <w>
        .set    elilen,eli_ptr+4            # Total size
        .set    eliplen,elilen-mle_bstream  # Parameter size
#
# --- Flush Complete (mleflushcomplete) -------------------------------
        .set    efc_vid,mle_bstream         # VID of Flush Complete <w>
                                            #  (0xFFFFFFFF if Global)
        .set    efclen,efc_vid+4            # Total size
#
# --- Can't resync NVA (mlenvarecord) ---------------------------------
        .set    env_status,mle_bstream      # Status                <b>
#       reserved 3                                                 3<b>
        .set    env_vid,env_status+4        # Virtual ID            <s>
        .set    env_rid,env_vid+2           # RAID ID               <s>
        .set    env_vsda,env_rid+2          # VID Starting Disk Addr<l>
        .set    env_rsda,env_vsda+8         # RID Starting Disk Addr<l>
        .set    env_vlen,env_rsda+8         # VID Length in blocks  <w>
        .set    env_rlen,env_vlen+4         # RID Length in blocks  <w>
        .set    envlen,env_rlen+4           # Total size
#
# --- File System Update (mlefsupdate, mlefsupdfail) ------------------
        .set    efugood,        0
        .set    efuinvalidpdd,  1           # 'To' PDD is 0, 'From' WWN is invalid
        .set    efuinoppdd,     2           # 'To' PDD in inoperable, invalid WWN
        .set    efubadwrite,    3           # Bad write to 'To' PDD
        .set    efubadread,     4           # No good 'from' PDDs or reads failed
        .set    efubadlblread,  5           # Label read from 'To' PDD failed
        .set    efubadlblwrite, 6           # Label write to 'To' PDD failed
        .set    efuinterrupted, 7           # Update interrupted by stop filesystem
#
        .set    efu_status,mle_bstream      # FS update status      <w>
        .set    efu_wwnto,efu_status+4      # Updated device WWN    <l>
        .set    efu_wwnfrom,efu_wwnto+8     # Updating device WWN   <l>
        .set    efu_pidto,efu_wwnfrom+8     # Updated device PID    <s>
#        reserved1                                                  <s>
        .set    efulen,efu_pidto+4          # Total size
#
# --- Single path to device (mlesinglepath) ---------------------------
        .set    dsp_devwwn,mle_bstream      # Device w/ single path <l>
        .set    dsplen,dsp_devwwn+8         # Total size
#
# --- Hotspare inoperative (mlehotspareinop) --------------------------
        .set    hsi_pid,mle_bstream         # PID                   <s>
#       reserved 2                                                  <s>
        .set    hsi_wwn,hsi_pid+4           # WWN                   <l>
        .set    hsilen,hsi_wwn+8            # Total size
#
# --- ECC error (mlesingleecc & mlefatalecc) --------------------------
        .set    ecc_proc,mle_bstream        # Processor (0=FE,1=BE) <b>
#       reserved 1                                                  <b>
        .set    ecc_etype,ecc_proc+2        # Error type            <b>
#       reserved 2                                                  <b>
        .set    ecc_memloc,ecc_etype+2      # Location in memory    <w>
        .set    ecc_elog,ecc_memloc+4       # ELOG register         <w>
        .set    ecc_snglcnt,ecc_elog+4      # Single-bit count      <w>
        .set    ecc_multicnt,ecc_snglcnt+4  # Multi-bit count       <w>
        .set    ecc_manycnt,ecc_multicnt+4  # > 2 ECC count         <w>
        .set    ecc_mcecnt,ecc_manycnt+4    # Total ECC error count <w>
        .set    ecclen,ecc_mcecnt+4         # Total size
#
# --- RAID error (mleraiderror) ---------------------------------------
        .set    ere_lun,mle_bstream         # LUN                   <s>
        .set    ere_vid,ere_lun+2           # Virtual ID            <s>
        .set    ere_rid,ere_vid+2           # RAID ID               <s>
        .set    ere_pid,ere_rid+2           # PID                   <s>
        .set    ere_wwn,ere_pid+2           # PDisk WWN             <l>
#
        .set    erelen,ere_wwn+8            # Total size
#
# --- RAID event (mleraidevent) ---------------------------------------
#       Event types
        .set    erlexeccomp,0x0001          # Error exec complete
#
        .set    erl_type,mle_bstream        # Event type            <s>
#       reserved 30                                               15<s>
        .set    erllen,erl_type+32          # Total size
#
# --- FE/BE controller serial number mismatch (mleserialmismatch) -----
        .set    ecm_feserial,mle_bstream    # FE serial number      <w>
        .set    ecm_beserial,ecm_feserial+4 # BE serial number      <w>
#
        .set    ecmlen,ecm_beserial+4       # Total size
#
# --- Stripe resync done (mleresyncdone) ------------------------------
        .set    erd_validnva,mle_bstream    # Valid NVA entries     <w>
        .set    erd_attempts,erd_validnva+4 # Resync attepts        <w>
        .set    erd_errors,erd_attempts+4   # Resync errors         <w>
#
        .set    erdlen,erd_errors+4         # Total size
#
# --- Broadcast an IPC message (mleipcbroadcast) ----------------------
        .set    ebisesos,0                  # SOS subevent
        .set    ebisescmt,1                 # SCMT subevent
        .set    ebiselcl,2                  # Put local image
        .set    ebiseDG,3                   # Datagram subevent
        .set    ebisefsys,4                 # File system report
        .set    ebiseLDD,5                  # Send LDD
        .set    ebiseISID,6                 # Send iSCSI server WWN
#
        .set    ebisossiz,32                # 32 bytes for SOS
        .set    ebiscmtsiz,52               # 52 bytes for SCMT
        .set    ebiDGsiz,52                 # 52 bytes for DataGram
        .set    ebifsyssiz,48               # 48 bytes for Fsys
#
        .set    ebimaxsiz,52                # 52 bytes max (64 - 12 byte hdr)
#
        .set    ebibtmaster,   0x0001       # Broadcast to master
        .set    ebibtslave,    0x0002       # Broadcast to slaves other than self
        .set    ebibtself,     0x0004       # Broadcast to self if slave
        .set    ebibtspec,     0x0008       # Broadcast to specific controller
        .set    ebibtothers,   0x0010       # Send to all but myself
#
        .set    ebibtall,      0x0007       # send to all controllers
        .set    ebibtallslaves,0x0014       # send to all slaves including self if slave
#
        .set    ebi_subevent,mle_bstream    # Subevent type         <s>
        .set    ebi_bcasttype,ebi_subevent+2# Broadcast type        <s>
        .set    ebi_serial,ebi_bcasttype+2  # Size of data to follow<w>
        .set    ebi_datasize,ebi_serial+4   # Serial num to send to <w>
        .set    ebi_data,ebi_datasize+4     # Data packet          *<w>
#
        .set    ebilen,ebi_data             # Total size
#
# --- Parity scan required (mleparityscanrequired)
        .set    eps_rid,mle_bstream         # RAID ID               <s>
#       reserved 2                          # Reserved              <s>
#
        .set    epslen,eps_rid+4            # Total size
#
# --- Task took too long before exchange (mletasktoolong) -------------
        .set    etl_time,mle_bstream        # Time between exchanges        <w>
        .set    etl_caller,etl_time+4       # The caller of the exchange    <w>
        .set    etl_old_caller,etl_caller+4 # The previous xchange caller   <w>
        .set    etl_pcb,etl_old_caller+4    # The PCB of task               <w>
#
        .set    etllen,etl_pcb+4            # Total size
        .set    etlplen,etllen-mle_bstream  # Parameter size
#
# --- Hotspares depleted (mlehsdepleted) ------------------------------
#        hsd_type bit definitions (future use)
        .set    hsdcnc,0                    # CNC is depleted
        .set    hsdbay,1                    # Indicated bay is depleted
        .set    hsdgeo,2                    # Indicated geo is depleted
        .set    hsdtoosmall,3               # A drive in system too big to spare
#
        .set    hsd_type,mle_bstream        # Type of depletion             <s>
        .set    hsd_dev,hsd_type+2          # Bay/pool depleted             <s>
        .set    hsd_devType,hsd_dev+2       # Device type                   <b>
#       reserved 3                                                         3<b>
        .set    hsdlen,hsd_devType+4        # Total size
#
# --- C$Stop Recovery Step Log (mlecstoplog) --------------------------
        .set    e_cstop_time,mle_bstream    # Time of recovery ste          <w>
        .set    e_cstop_function,e_cstop_time+4 # The recovery function     <w>
        .set    e_cstop_corc,e_cstop_function+4 # C_orc at time of recovery <w>
        .set    e_cstop_cowsrpc,e_cstop_corc+4 # C_owsrpc at time of rcvry  <w>
        .set    e_cstop_corsrpc,e_cstop_cowsrpc+4 # C_orsrpc at recovery    <w>
        .set    e_cstop_dvlorc,e_cstop_corsrpc+4 # D_vlorc at time of rcvry <w>
        .set    e_cstop_linkout,e_cstop_dvlorc+4 # Link960 Outstanding VRPs <w>
#
        .set    e_cstop_len,e_cstop_linkout+4 # Total size
#
# --- Mirror Capable (mlemirrorcapable) -------------------------------
        .set    e_mc_mirror_partner_sn,mle_bstream # Mirror Partner SN      <w>
#
        .set    e_mc_len,e_mc_mirror_partner_sn+4 # Total size
#
# --- RAID 5 Inoperative Log Message (mleR5inop) ----------------------
        .set    e_r5inop_vid,mle_bstream    # VDisk ID of Inoperative RAID  <s>
        .set    e_r5inop_rid,e_r5inop_vid+2 # RAID ID of Inoperative RAID   <s>
#
        .set    e_r5inop_len,e_r5inop_rid+2 # Total size
#
# --- Parity check RAID log (mleparitycheckraid)
        .set    e_pcr_start,    0           # Parity Check for RID Started
        .set    e_pcr_end,      1           # Parity Check for RID Complete
        .set    e_pcr_term,     2           # Parity Check for RID Terminated
#
        .set    e_pcr_rid,mle_bstream       # RAID ID                       <s>
        .set    e_pcr_type,e_pcr_rid+2      # Type - begin, end, terminated <s>
#
        .set    e_pcrlen,e_pcr_type+2       # Total size
#
# --- Write Cache Flush Taking Too Long log (mleWCFlushLog)
        .set    e_wcflush_time,mle_bstream  # Time from beginning of Flush  <w>
        .set    e_wcflush_tag_dirty,e_wcflush_time+4 # Tags Dirty Count     <w>
        .set    e_wcflush_tag_res,e_wcflush_tag_dirty+4 # Tags Resident cnt <w>
        .set    e_wcflush_tag_fip,e_wcflush_tag_res+4 # Tags Flush in Progr <w>
        .set    e_wcflush_blks_dirty,e_wcflush_tag_fip+4 # Blocks Dirty cnt <w>
        .set    e_wcflush_blks_res,e_wcflush_blks_dirty+4 # Blocks Resident <w>
        .set    e_wcflush_blks_fip,e_wcflush_blks_res+4 # Blocks Flush in Pr <w>
        .set    e_wcflush_inv_cntr,e_wcflush_blks_fip+4 # Flush & inval cnt <w>
        .set    e_wcflush_corc,e_wcflush_inv_cntr+4 # Cache Outs Request Cnt <w>
        .set    e_wcflush_ca,e_wcflush_corc+4 # Cache Status, Status2, Stops <w>
        .set    e_wcflush_orc,e_wcflush_ca+4 # Outstanding VRP cnt to BE    <w>
        .set    e_wcflush_util,e_wcflush_orc+4 # Processor Utilization      <w>
#
        .set    e_wcflush_len,e_wcflush_util+4  # Total size
#
# --- SCSI timeout event (mlescsitimeout) --------------------------
        .set    est_flags_timeout,      0   # Flag value for timeout watchdog
        .set    est_flags_comp,         1   # Flag valud for completion function
#
        .set    est_port,mle_bstream        # FC port number         <b>
        .set    est_flags,est_port+1        # Flags                  <b>
        .set    est_lun,est_flags+1         # LUN                    <s>
        .set    est_id,est_lun+2            # FC ID                  <w>
        .set    est_wwn,est_id+4            # FC WWN                 <l>
        .set    est_cdb,est_wwn+8           # SCSI cdb              4<w>
        .set    estlen,est_cdb+16           # Total size
#
# --- A drive has failed many operations (mledrivedelay) ---------------------------
        .set    ddly_devwwn,mle_bstream     # Device w/ single path <l>
        .set    ddlylen,ddly_devwwn+8       # Total size
#
# --- RAID Inoperative Log Message (mleRaidinop) ----------------------
        .set    e_rinop_vid,mle_bstream     # VDisk ID of Inoperative RAID  <s>
        .set    e_rinop_rid,e_rinop_vid+2   # RAID ID of Inoperative RAID   <s>
#
        .set    e_rinop_len,e_rinop_rid+2   # Total size
#
# --- Global Cache Mode Log Message (mleglobalcachemode) ----------------------
        .set    epc_mode,mle_bstream        # Global cache mode  <b>
        .set    epclen,epc_mode+1           # Total size
#
# Modelines:
# Local Variables:
# tab-width: 4
# indent-tabs-mode: nil
# End:
# vi:sw=4 ts=4 expandtab
#
