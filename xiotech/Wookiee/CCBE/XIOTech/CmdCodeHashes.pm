# $Id: CmdCodeHashes.pm 159730 2012-08-27 22:03:32Z marshall_midden $
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2002  Xiotech
# ======================================================================
# $RCSfile: CmdCodeHashes.pm
# Author: Randy Rysavy
#
# Purpose: Build the various Command Code hashes and return to caller in
#          one large, all encompassing hash.
# 
# IMPORTANT NOTE: "CmdCodeHashes.pm" is an AUTO-GENERATED FILE.  DO NOT EDIT
# THAT FILE OR THE CHANGES YOU MAKE WILL BE OVERWRITTEN THE NEXT TIME IT IS
# GENERATED.  Instead, make your changes to CmdCodeHashes.tpl, as those
# changes then get reflected in the generated file.
#   
##############################################################################
package XIOTech::CmdCodeHashes;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
    BuildCmdCodeHashTables
);

use strict;

sub BuildCmdCodeHashTables 
{
    my %AllHashes;


    #====================================================================
    #                      M R P ' s
    #====================================================================
    my %mrpIdH;
    my %mrpDataH;

    #
    # MRP id description table
    #
    $mrpIdH{ 99 }  = "MRP Start";
    $mrpIdH{ 100 } = "MRP TOCallBk";
    $mrpIdH{ 0 }   = "MRP Good";
    $mrpIdH{ 1 }   = "MRP Error";
    $mrpIdH{ 2 }   = "MRP InProg";
    $mrpIdH{ 3 }   = "MRP N/A (?)";
    $mrpIdH{ 4 }   = "MRP TimeOut";
    $mrpIdH{ 5 }   = "MRP InvCmd";
    $mrpIdH{ 6 }   = "MRP MallocEr";
    $mrpIdH{ 7 }   = "MRP ParmVEr";
    $mrpIdH{ 8 }   = "MRP NotMastr";
    $mrpIdH{ 9 }   = "MRP PwrUpNC";
    $mrpIdH{ 10 }  = "MRP ElInProg";
    $mrpIdH{ 11 }  = "MRP TunnelEr";
    $mrpIdH{ 12 }  = "MRP R5Resync";
    $mrpIdH{ 128 } = "MRPQ BE Free";
    $mrpIdH{ 129 } = "MRPQ BE M Bl";
    $mrpIdH{ 130 } = "MRPQ BE F Bl";
    $mrpIdH{ 131 } = "MRPQ BE A Bl";
    $mrpIdH{ 256 } = "MRPQ FE Free";
    $mrpIdH{ 257 } = "MRPQ FE M Bl";
    $mrpIdH{ 512 } = "MRPG Start";
    $mrpIdH{ 513 } = "MRPG Finish";

    #
    # MRP data description table
    # 
    $mrpDataH{ 0x0100 } = "CREXP";
    $mrpDataH{ 0x0101 } = "GETELIST";
    $mrpDataH{ 0x0102 } = "LABEL";
    $mrpDataH{ 0x0103 } = "FAIL";
    $mrpDataH{ 0x0104 } = "SCSIIO";
    $mrpDataH{ 0x0105 } = "INITRAID";
    $mrpDataH{ 0x0106 } = "OBSOLETE106";
    $mrpDataH{ 0x0107 } = "DELVIRT";
    $mrpDataH{ 0x0108 } = "SETCACHE";
    $mrpDataH{ 0x0109 } = "SERVERPROP";
    $mrpDataH{ 0x010A } = "RESET";
    $mrpDataH{ 0x010B } = "RESTORE";
    $mrpDataH{ 0x010C } = "AWAKE";
    $mrpDataH{ 0x010D } = "WWNLOOKUP";
    $mrpDataH{ 0x010E } = "BEGENERIC";
    $mrpDataH{ 0x010F } = "STARTSTOP";
    $mrpDataH{ 0x0110 } = "SCRUBCTRL";
    $mrpDataH{ 0x0111 } = "DEFAULT";
    $mrpDataH{ 0x0112 } = "GETBEDEVPATHS";
    $mrpDataH{ 0x0113 } = "RESTOREDEV";
    $mrpDataH{ 0x0114 } = "DEFRAGMENT";
    $mrpDataH{ 0x0115 } = "SETATTR";
    $mrpDataH{ 0x0116 } = "BELOOP";
    $mrpDataH{ 0x0117 } = "GETSLIST";
    $mrpDataH{ 0x0118 } = "GETVLIST";
    $mrpDataH{ 0x0119 } = "GETRLIST";
    $mrpDataH{ 0x011A } = "GETPLIST";
    $mrpDataH{ 0x011B } = "GETMLIST";
    $mrpDataH{ 0x011C } = "GETVINFO";
    $mrpDataH{ 0x011D } = "GETRINFO";
    $mrpDataH{ 0x011E } = "GETPINFO";
    $mrpDataH{ 0x011F } = "MAPLUN";
    $mrpDataH{ 0x0120 } = "UNMAPLUN";
    $mrpDataH{ 0x0121 } = "GETEINFO";
    $mrpDataH{ 0x0122 } = "CREATESERVER";
    $mrpDataH{ 0x0123 } = "DELETESERVER";
    $mrpDataH{ 0x0124 } = "GETMINFO";
    $mrpDataH{ 0x0125 } = "VDISKCONTROL";
    $mrpDataH{ 0x0126 } = "ASSIGNSYSINFO";
    $mrpDataH{ 0x0127 } = "BEII";
    $mrpDataH{ 0x0128 } = "BELINK";
    $mrpDataH{ 0x0129 } = "BEBOOT";
    $mrpDataH{ 0x012A } = "BEDIAG";
    $mrpDataH{ 0x012B } = "BEPROC";
    $mrpDataH{ 0x012C } = "BECODEBURN";
    $mrpDataH{ 0x012D } = "BRWMEM";
    $mrpDataH{ 0x012E } = "CONFIGTARG";
    $mrpDataH{ 0x012F } = "GETMPLIST";
    $mrpDataH{ 0x0130 } = "GLOBALPRI";
    $mrpDataH{ 0x0131 } = "GETTLIST";
    $mrpDataH{ 0x0132 } = "RESETBEPORT";
    $mrpDataH{ 0x0133 } = "NAMECHANGE";
    $mrpDataH{ 0x0134 } = "REMOTECTRLCNT";
    $mrpDataH{ 0x0135 } = "REMOTECTRLINFO";
    $mrpDataH{ 0x0136 } = "REMOTEVDISKINFO";
    $mrpDataH{ 0x0137 } = "FOREIGNTARGETS";
    $mrpDataH{ 0x0138 } = "CREATEVLINK";
    $mrpDataH{ 0x0139 } = "VLINKINFO";
    $mrpDataH{ 0x013A } = "CREATECTRLR";
    $mrpDataH{ 0x013B } = "RESCANDEVICE";
    $mrpDataH{ 0x013C } = "RESYNC";
    $mrpDataH{ 0x013D } = "GETLCLIMAGE";
    $mrpDataH{ 0x013E } = "PUTLCLIMAGE";
    $mrpDataH{ 0x013F } = "DELETEDEVICE";
    $mrpDataH{ 0x0140 } = "BEMODEPAGE";
    $mrpDataH{ 0x0141 } = "DEVICECOUNT";
    $mrpDataH{ 0x0142 } = "GETVIDOWNER";
    $mrpDataH{ 0x0143 } = "HOTSPAREINFO";
    $mrpDataH{ 0x0144 } = "FILECOPY";
    $mrpDataH{ 0x0145 } = "BEGETDVLIST";
    $mrpDataH{ 0x0146 } = "BEGETPORTLIST";
    $mrpDataH{ 0x0147 } = "BREAKVLOCK";
    $mrpDataH{ 0x0148 } = "GETSOS";
    $mrpDataH{ 0x0149 } = "PUTSOS";
    $mrpDataH{ 0x014A } = "FORCEBEETRAP";
    $mrpDataH{ 0x014B } = "PUTSCMT";
    $mrpDataH{ 0x014C } = "BELOOPPRIMITIVE";
    $mrpDataH{ 0x014D } = "TARGETCONTROL";
    $mrpDataH{ 0x014E } = "FAILCTRL";
    $mrpDataH{ 0x014F } = "NAMEDEVICE";
    $mrpDataH{ 0x0150 } = "PUTDG";
    $mrpDataH{ 0x0151 } = "NOPBE";
    $mrpDataH{ 0x0152 } = "PUTFSYS";
    $mrpDataH{ 0x0153 } = "GETDLINK";
    $mrpDataH{ 0x0154 } = "GETDLOCK";
    $mrpDataH{ 0x0155 } = "DEGRADEPORT";
    $mrpDataH{ 0x0156 } = "GETWSINFO";
    $mrpDataH{ 0x0157 } = "SETWSINFO";
    $mrpDataH{ 0x0158 } = "GETGPINFO";
    $mrpDataH{ 0x0159 } = "SETGPINFO";
    $mrpDataH{ 0x015A } = "CHGRAIDNOTMIRRORING";
    $mrpDataH{ 0x015B } = "PUTLDD";
    $mrpDataH{ 0x015C } = "RAIDRECOVER";
    $mrpDataH{ 0x015D } = "PUTDEVCONFIG";
    $mrpDataH{ 0x015E } = "RESYNCDATA";
    $mrpDataH{ 0x015F } = "RESYNCCTL";
    $mrpDataH{ 0x0160 } = "REFRESH";
    $mrpDataH{ 0x0161 } = "SETVPRI";
    $mrpDataH{ 0x0162 } = "VPRI_ENABLE";
    $mrpDataH{ 0x0163 } = "PDISKSPINDOWN";
    $mrpDataH{ 0x0164 } = "PDISKFAILBACK";
    $mrpDataH{ 0x0165 } = "PDISKAUTOFAILBACKENABLEDISABLE";
    $mrpDataH{ 0x0166 } = "CFGOPTION";
    $mrpDataH{ 0x0167 } = "SETTGINFO";
    $mrpDataH{ 0x0168 } = "GETTGINFO";
    $mrpDataH{ 0x0169 } = "UPDSID";
    $mrpDataH{ 0x016A } = "SETCHAP";
    $mrpDataH{ 0x016B } = "GETCHAP";
    $mrpDataH{ 0x016C } = "GETGLINFO";
    $mrpDataH{ 0x016D } = "SETGLINFO";
    $mrpDataH{ 0x016E } = "CLEARGLINFO";
    $mrpDataH{ 0x016F } = "GETISNSINFO";
    $mrpDataH{ 0x0170 } = "SETISNSINFO";
    $mrpDataH{ 0x0171 } = "SETPR";
    $mrpDataH{ 0x0172 } = "VIRTREDUNDANCY";
    $mrpDataH{ 0x0173 } = "GETASYNC";
    $mrpDataH{ 0x0174 } = "GETISEIP";
    $mrpDataH{ 0x0175 } = "ALLDEVMISS";
    $mrpDataH{ 0x01FD } = "NOPFSYS";
    $mrpDataH{ 0x01FE } = "FSYSOP";
    $mrpDataH{ 0x01FF } = "BEHBEAT";
    $mrpDataH{ 0x0200 } = "BFFUNCBASE";
    $mrpDataH{ 0x0200 } = "REPORTSCONFIG";
    $mrpDataH{ 0x0201 } = "SCONFIGCOMPLETE";
    $mrpDataH{ 0x0202 } = "REPORTCCONFIG";
    $mrpDataH{ 0x0203 } = "CCONFIGCOMPLETE";
    $mrpDataH{ 0x0204 } = "SPARE204";
    $mrpDataH{ 0x0205 } = "STOPCACHE";
    $mrpDataH{ 0x0206 } = "CONTINUECACHE";
    $mrpDataH{ 0x0207 } = "SETSYSINFO";
    $mrpDataH{ 0x0208 } = "VCHANGE";
    $mrpDataH{ 0x0209 } = "SCHANGE";
    $mrpDataH{ 0x020A } = "REPORTTARG";
    $mrpDataH{ 0x020B } = "RESETCONFIG";
    $mrpDataH{ 0x020C } = "SETCNTLSNFE";
    $mrpDataH{ 0x020D } = "MMINFO";
    $mrpDataH{ 0x020F } = "UPDTGINFO";
    $mrpDataH{ 0x0210 } = "GETPORTTYPE";
    $mrpDataH{ 0x0211 } = "SETCHAPFE";
    $mrpDataH{ 0x0212 } = "SETISNSINFOFE";
    $mrpDataH{ 0x0213 } = "SETPRES";
    $mrpDataH{ 0x0300 } = "LOGFE";
    $mrpDataH{ 0x0301 } = "LOGBE";
    $mrpDataH{ 0x0400 } = "FBFUNCBASE";
    $mrpDataH{ 0x0400 } = "FEGETVINFO";
    $mrpDataH{ 0x0401 } = "FESETSEQ";
    $mrpDataH{ 0x0402 } = "SETMPCONFIGBE";
    $mrpDataH{ 0x0403 } = "RETRIEVEPR";
    $mrpDataH{ 0x0500 } = "EFFUNCBASE";
    $mrpDataH{ 0x0500 } = "FELOOP";
    $mrpDataH{ 0x0501 } = "GETSINFO";
    $mrpDataH{ 0x0502 } = "GETCINFO";
    $mrpDataH{ 0x0503 } = "FELINK";
    $mrpDataH{ 0x0504 } = "FEII";
    $mrpDataH{ 0x0505 } = "GETCDINFO";
    $mrpDataH{ 0x0506 } = "GETSSTATS";
    $mrpDataH{ 0x0507 } = "SETBATHEALTH";
    $mrpDataH{ 0x0508 } = "RESUMECACHE";
    $mrpDataH{ 0x0509 } = "FEBOOT";
    $mrpDataH{ 0x050A } = "FEDIAG";
    $mrpDataH{ 0x050B } = "FEPROC";
    $mrpDataH{ 0x050C } = "FECODEBURN";
    $mrpDataH{ 0x050D } = "FRWMEM";
    $mrpDataH{ 0x050E } = "RESETFEPORT";
    $mrpDataH{ 0x050F } = "SERVERLOOKUP";
    $mrpDataH{ 0x0510 } = "FEGENERIC";
    $mrpDataH{ 0x0511 } = "SETMPCONFIGFE";
    $mrpDataH{ 0x0512 } = "FEFIBREHLIST";
    $mrpDataH{ 0x0513 } = "FECONTWOMP";
    $mrpDataH{ 0x0514 } = "FEFLUSHWOMP";
    $mrpDataH{ 0x0515 } = "INVFEWC";
    $mrpDataH{ 0x0516 } = "FLUSHBEWC";
    $mrpDataH{ 0x0517 } = "INVBEWC";
    $mrpDataH{ 0x0518 } = "FEMODEPAGE";
    $mrpDataH{ 0x0519 } = "FEGETDVLIST";
    $mrpDataH{ 0x051A } = "FEGETPORTLIST";
    $mrpDataH{ 0x051B } = "GETTRLIST";
    $mrpDataH{ 0x051C } = "STOPIO";
    $mrpDataH{ 0x051D } = "STARTIO";
    $mrpDataH{ 0x051E } = "FEPORTNOTIFY";
    $mrpDataH{ 0x051F } = "FORCEFEETRAP";
    $mrpDataH{ 0x0520 } = "FELOOPPRIMITIVE";
    $mrpDataH{ 0x0521 } = "GETTARG";
    $mrpDataH{ 0x0522 } = "FAILPORT";
    $mrpDataH{ 0x0523 } = "NOPFE";
    $mrpDataH{ 0x0524 } = "QFECC";
    $mrpDataH{ 0x0525 } = "QSC";
    $mrpDataH{ 0x0526 } = "QMPC";
    $mrpDataH{ 0x0527 } = "GETHABSTATS";
    $mrpDataH{ 0x0528 } = "MMCARDGETBATTERYSTATUS";
    $mrpDataH{ 0x0528 } = "GETBATTSTS";
    $mrpDataH{ 0x0529 } = "GETMPCONFIGFE";
    $mrpDataH{ 0x052A } = "FEPORTGO";
    $mrpDataH{ 0x052B } = "SETTDISCACHE";
    $mrpDataH{ 0x052C } = "CLRTDISCACHE";
    $mrpDataH{ 0x052D } = "QTDISABLEDONE";
    $mrpDataH{ 0x052E } = "MMTEST";
    $mrpDataH{ 0x052F } = "FEFLERRORWOMP";
    $mrpDataH{ 0x0530 } = "GETSESSIONS";
    $mrpDataH{ 0x0531 } = "GETSESSIONSPERSERVER";
    $mrpDataH{ 0x0532 } = "GETIDDINFO";
    $mrpDataH{ 0x0533 } = "DLMPATHSTATS";
    $mrpDataH{ 0x0534 } = "DLMPATHSELECTIONALGO";
    $mrpDataH{ 0x0535 } = "PRGET";
    $mrpDataH{ 0x0536 } = "PRCLR";
    $mrpDataH{ 0x0537 } = "PRCONFIGCOMPLETE";
    $mrpDataH{ 0x0538 } = "UPDPRR";
    $mrpDataH{ 0x05FF } = "FEHBEAT";
    $mrpDataH{ 0x0700 } = "CCBTODLM";

    # Add these hashes to the main hash
    $AllHashes{MRP_ID} = \%mrpIdH;
    $AllHashes{MRP_DATA} = \%mrpDataH;




    #====================================================================
    #                      Packet ID's
    #====================================================================
    my %pktIdH;
    my %pktDataH;

    #
    # Packet id description table
    #
    $pktIdH{ 99 } = "Pkt Start";
    $pktIdH{ 0 }  = "Pkt Good";
    $pktIdH{ 1 }  = "Pkt Error";
    $pktIdH{ 2 }  = "Pkt InProg";
    $pktIdH{ 3 }  = "Pkt N/A (?)";
    $pktIdH{ 4 }  = "Pkt TimeOut";
    $pktIdH{ 5 }  = "Pkt InvCmd";
    $pktIdH{ 6 }  = "Pkt MallocEr";
    $mrpIdH{ 7 }  = "Pkt ParmVEr";
    $mrpIdH{ 8 }  = "Pkt NotMastr";
    $mrpIdH{ 9 }  = "Pkt PwrUpNC";
    $mrpIdH{ 10 } = "Pkt ElInProg";
    $mrpIdH{ 11 } = "Pkt TunnelEr";
    $mrpIdH{ 12 } = "Pkt R5Resync";

    #
    # Packet data description table
    #
    $pktDataH{ 0x0001 } = "CONNECT";
    $pktDataH{ 0x0002 } = "DISCONNECT";
    $pktDataH{ 0x0003 } = "PING";
    $pktDataH{ 0x0004 } = "RESET";
    $pktDataH{ 0x0005 } = "POWER_UP_STATE";
    $pktDataH{ 0x0006 } = "POWER_UP_RESPONSE";
    $pktDataH{ 0x0007 } = "X1_COMPATIBILITY_INDEX";
    $pktDataH{ 0x0008 } = "REGISTER_EVENTS";
    $pktDataH{ 0x0009 } = "REGISTER_CLIENT_TYPE";
    $pktDataH{ 0x0010 } = "PDISK_COUNT";
    $pktDataH{ 0x0011 } = "PDISK_LIST";
    $pktDataH{ 0x0012 } = "PDISK_INFO";
    $pktDataH{ 0x0013 } = "PDISK_LABEL";
    $pktDataH{ 0x0014 } = "PDISK_DEFRAG";
    $pktDataH{ 0x0015 } = "PDISK_FAIL";
    $pktDataH{ 0x0016 } = "PDISK_BEACON";
    $pktDataH{ 0x0017 } = "PDISK_UNFAIL";
    $pktDataH{ 0x0018 } = "PDISK_DELETE";
    $pktDataH{ 0x0019 } = "PDISK_BYPASS";
    $pktDataH{ 0x001A } = "PDISK_DEFRAG_STATUS";
    $pktDataH{ 0x001F } = "PDISKS";
    $pktDataH{ 0x0020 } = "VDISK_COUNT";
    $pktDataH{ 0x0021 } = "VDISK_LIST";
    $pktDataH{ 0x0022 } = "VDISK_INFO";
    $pktDataH{ 0x0023 } = "VDISK_CREATE";
    $pktDataH{ 0x0024 } = "VDISK_DELETE";
    $pktDataH{ 0x0025 } = "VDISK_EXPAND";
    $pktDataH{ 0x0026 } = "VDISK_CONTROL";
    $pktDataH{ 0x0027 } = "VDISK_PREPARE";
    $pktDataH{ 0x0028 } = "VDISK_SET_PRIORITY";
    $pktDataH{ 0x0029 } = "VDISK_OWNER";
    $pktDataH{ 0x002A } = "VDISK_SET_ATTRIBUTE";
    $pktDataH{ 0x002B } = "VDISK_PR_GET";
    $pktDataH{ 0x002C } = "VDISK_PR_CLR";
    $pktDataH{ 0x002F } = "VDISKS";
    $pktDataH{ 0x0030 } = "SERVER_COUNT";
    $pktDataH{ 0x0031 } = "SERVER_LIST";
    $pktDataH{ 0x0032 } = "SERVER_INFO";
    $pktDataH{ 0x0033 } = "SERVER_CREATE";
    $pktDataH{ 0x0034 } = "SERVER_DELETE";
    $pktDataH{ 0x0035 } = "SERVER_ASSOCIATE";
    $pktDataH{ 0x0036 } = "SERVER_DISASSOCIATE";
    $pktDataH{ 0x0037 } = "SERVER_SET_PROPERTIES";
    $pktDataH{ 0x0038 } = "SERVER_LOOKUP";
    $pktDataH{ 0x0039 } = "SERVER_WWN_TO_TARGET_MAP";
    $pktDataH{ 0x003F } = "SERVERS";
    $pktDataH{ 0x0040 } = "VLINK_REMOTE_CTRL_COUNT";
    $pktDataH{ 0x0041 } = "VLINK_REMOTE_CTRL_INFO";
    $pktDataH{ 0x0042 } = "VLINK_REMOTE_CTRL_VDISKS";
    $pktDataH{ 0x0043 } = "VLINK_CREATE";
    $pktDataH{ 0x0044 } = "VLINK_INFO";
    $pktDataH{ 0x0045 } = "VLINK_BREAK_LOCK";
    $pktDataH{ 0x0046 } = "VLINK_NAME_CHANGED";
    $pktDataH{ 0x0047 } = "VLINK_DLINK_INFO";
    $pktDataH{ 0x0048 } = "VLINK_DLOCK_INFO";
    $pktDataH{ 0x0050 } = "TARGET_COUNT";
    $pktDataH{ 0x0051 } = "TARGET_LIST";
    $pktDataH{ 0x0052 } = "TARGET_INFO";
    $pktDataH{ 0x0053 } = "TARGET_SET_PROPERTIES";
    $pktDataH{ 0x0054 } = "TARGET_RESOURCE_LIST";
    $pktDataH{ 0x005F } = "TARGETS";
    $pktDataH{ 0x0060 } = "STATS_GLOBAL_CACHE";
    $pktDataH{ 0x0061 } = "STATS_CACHE_DEVICE";
    $pktDataH{ 0x0062 } = "STATS_FRONT_END_PROC";
    $pktDataH{ 0x0063 } = "STATS_BACK_END_PROC";
    $pktDataH{ 0x0064 } = "STATS_FRONT_END_LOOP";
    $pktDataH{ 0x0065 } = "STATS_BACK_END_LOOP";
    $pktDataH{ 0x0066 } = "STATS_FRONT_END_PCI";
    $pktDataH{ 0x0067 } = "STATS_BACK_END_PCI";
    $pktDataH{ 0x0068 } = "STATS_SERVER";
    $pktDataH{ 0x0069 } = "STATS_VDISK";
    $pktDataH{ 0x006A } = "STATS_PROC";
    $pktDataH{ 0x006B } = "STATS_PCI";
    $pktDataH{ 0x006C } = "STATS_CACHE_DEVICES";
    $pktDataH{ 0x006D } = "STATS_ENVIRONMENTAL";
    $pktDataH{ 0x006E } = "STATS_I2C";
    $pktDataH{ 0x006F } = "STATS_HAB";
    $pktDataH{ 0x0070 } = "RAID_COUNT";
    $pktDataH{ 0x0071 } = "RAID_LIST";
    $pktDataH{ 0x0072 } = "RAID_INFO";
    $pktDataH{ 0x0073 } = "RAID_INIT";
    $pktDataH{ 0x0074 } = "RAID_CONTROL";
    $pktDataH{ 0x0075 } = "RAID_MIRRORING";
    $pktDataH{ 0x0076 } = "RAID_RECOVER";
    $pktDataH{ 0x007F } = "RAIDS";
    $pktDataH{ 0x0080 } = "ADMIN_FW_VERSIONS";
    $pktDataH{ 0x0081 } = "ADMIN_FW_SYS_REL_LEVEL";
    $pktDataH{ 0x0082 } = "ADMIN_SETTIME";
    $pktDataH{ 0x0083 } = "ADMIN_LEDCNTL";
    $pktDataH{ 0x0084 } = "ADMIN_SET_IP";
    $pktDataH{ 0x0085 } = "ADMIN_GET_IP";
    $pktDataH{ 0x0086 } = "ADMIN_GETTIME";
    $pktDataH{ 0x0090 } = "DEBUG_MEM_RDWR";
    $pktDataH{ 0x0091 } = "DEBUG_REPORT";
    $pktDataH{ 0x0092 } = "DEBUG_INIT_PROC_NVRAM";
    $pktDataH{ 0x0093 } = "DEBUG_INIT_CCB_NVRAM";
    $pktDataH{ 0x0094 } = "DEBUG_GET_SER_NUM";
    $pktDataH{ 0x0095 } = "DEBUG_MRMMTEST";
    $pktDataH{ 0x0098 } = "DEBUG_STRUCT_DISPLAY";
    $pktDataH{ 0x0099 } = "DEBUG_GET_ELECTION_STATE";
    $pktDataH{ 0x009A } = "DEBUG_SCSI_COMMAND";
    $pktDataH{ 0x009B } = "DEBUG_BE_LOOP_PRIMITIVE";
    $pktDataH{ 0x009C } = "DEBUG_FE_LOOP_PRIMITIVE";
    $pktDataH{ 0x009D } = "DEBUG_GET_STATE_RM";
    $pktDataH{ 0x00A0 } = "VCG_VALIDATION";
    $pktDataH{ 0x00A1 } = "PDISK_SPINDOWN";
    $pktDataH{ 0x00A2 } = "VCG_PREPARE_SLAVE";
    $pktDataH{ 0x00A3 } = "VCG_ADD_SLAVE";
    $pktDataH{ 0x00A4 } = "VCG_PING";
    $pktDataH{ 0x00A5 } = "VCG_INFO";
    $pktDataH{ 0x00A6 } = "VCG_INACTIVATE_CONTROLLER";
    $pktDataH{ 0x00A7 } = "VCG_ACTIVATE_CONTROLLER";
    $pktDataH{ 0x00A8 } = "VCG_SET_CACHE";
    $pktDataH{ 0x00A9 } = "VCG_GET_MP_LIST";
    $pktDataH{ 0x00AA } = "PDISK_FAILBACK";
    $pktDataH{ 0x00AB } = "VCG_APPLY_LICENSE";
    $pktDataH{ 0x00AC } = "VCG_UNFAIL_CONTROLLER";
    $pktDataH{ 0x00AD } = "VCG_FAIL_CONTROLLER";
    $pktDataH{ 0x00AE } = "VCG_REMOVE_CONTROLLER";
    $pktDataH{ 0x00AF } = "VCG_SHUTDOWN";
    $pktDataH{ 0x00B0 } = "DISK_BAY_COUNT";
    $pktDataH{ 0x00B1 } = "DISK_BAY_LIST";
    $pktDataH{ 0x00B2 } = "DISK_BAY_INFO";
    $pktDataH{ 0x00B4 } = "DISK_BAY_DELETE";
    $pktDataH{ 0x00B5 } = "DISK_BAY_ALARM_CTRL";
    $pktDataH{ 0x00B6 } = "DISK_BAY_LED_CTRL";
    $pktDataH{ 0x00B7 } = "SET_GEO_LOCATION";
    $pktDataH{ 0x00BF } = "DISK_BAYS";
    $pktDataH{ 0x00C0 } = "PDISK_AUTO_FAILBACK_ENABLE_DISABLE";
    $pktDataH{ 0x00C1 } = "ENVIRO_DATA_DISK_BAY";
    $pktDataH{ 0x00C2 } = "ENVIRO_DATA_CTRL_AND_BAY";
    $pktDataH{ 0x00C3 } = "VCG_CONFIGURE";
    $pktDataH{ 0x00C4 } = "ENV_II_GET_DATA";
    $pktDataH{ 0x00C5 } = "ISE_GET_STATUS";
    $pktDataH{ 0x00D0 } = "SNAPSHOT_READDIR";
    $pktDataH{ 0x00D1 } = "SNAPSHOT_TAKE";
    $pktDataH{ 0x00D2 } = "SNAPSHOT_LOAD";
    $pktDataH{ 0x00D3 } = "SNAPSHOT_CHANGE";
    $pktDataH{ 0x00E0 } = "FIRMWARE_DOWNLOAD";
    $pktDataH{ 0x00E1 } = "LOG_INFO";
    $pktDataH{ 0x00E2 } = "LOG_CLEAR";
    $pktDataH{ 0x00E3 } = "WRITE_BUFFER_MODE5";
    $pktDataH{ 0x00E4 } = "TRY_CCB_FW";
    $pktDataH{ 0x00E5 } = "ROLLING_UPDATE_PHASE";
    $pktDataH{ 0x00E6 } = "LOG_TEXT_MESSAGE";
    $pktDataH{ 0x00E7 } = "MULTI_PART_XFER";
    $pktDataH{ 0x00E8 } = "CUSTOMER_LOG_ACKNOWLEDGE";
    $pktDataH{ 0x00F0 } = "GENERIC";
    $pktDataH{ 0x00F1 } = "GENERIC2";
    $pktDataH{ 0x00F2 } = "GENERIC_MRP";
    $pktDataH{ 0x0100 } = "PROC_RESTORE_NVRAM";
    $pktDataH{ 0x0101 } = "PROC_RESET_FE_QLOGIC";
    $pktDataH{ 0x0102 } = "PROC_RESET_BE_QLOGIC";
    $pktDataH{ 0x0103 } = "PROC_START_IO";
    $pktDataH{ 0x0104 } = "PROC_STOP_IO";
    $pktDataH{ 0x0105 } = "PROC_ASSIGN_MIRROR_PARTNER";
    $pktDataH{ 0x0106 } = "PROC_BE_PORT_LIST";
    $pktDataH{ 0x0107 } = "PROC_FE_PORT_LIST";
    $pktDataH{ 0x0108 } = "PROC_BE_DEVICE_PATH";
    $pktDataH{ 0x0109 } = "PROC_NAME_DEVICE";
    $pktDataH{ 0x010A } = "PROC_FAIL_CTRL";
    $pktDataH{ 0x010B } = "PROC_FAIL_PORT";
    $pktDataH{ 0x010C } = "PROC_TARGET_CONTROL";
    $pktDataH{ 0x0110 } = "PERSISTENT_DATA_CONTROL";
    $pktDataH{ 0x0111 } = "CLIENT_PERSISTENT_DATA_CONTROL";
    $pktDataH{ 0x0150 } = "WCACHE_INVALIDATE";
    $pktDataH{ 0x0200 } = "MISC_GET_DEVICE_COUNT";
    $pktDataH{ 0x0201 } = "MISC_RESCAN_DEVICE";
    $pktDataH{ 0x0202 } = "MISC_GET_BE_DEVICE_LIST";
    $pktDataH{ 0x0203 } = "MISC_GET_FE_DEVICE_LIST";
    $pktDataH{ 0x0204 } = "MISC_FILE_SYSTEM_READ";
    $pktDataH{ 0x0205 } = "MISC_FILE_SYSTEM_WRITE";
    $pktDataH{ 0x0206 } = "MISC_FAILURE_STATE_SET";
    $pktDataH{ 0x0207 } = "MISC_GET_MODE";
    $pktDataH{ 0x0208 } = "MISC_SET_MODE";
    $pktDataH{ 0x0209 } = "MISC_UNFAIL_INTERFACE";
    $pktDataH{ 0x020A } = "MISC_FAIL_INTERFACE";
    $pktDataH{ 0x020B } = "MISC_SERIAL_NUMBER_SET";
    $pktDataH{ 0x020C } = "MISC_RESYNC_MIRROR_RECORDS";
    $pktDataH{ 0x020D } = "MISC_CONTINUE_WO_MP";
    $pktDataH{ 0x020E } = "MISC_INVALIDATE_BE_WC";
    $pktDataH{ 0x020F } = "MISC_MIRROR_PARTNER_CONTROL";
    $pktDataH{ 0x0210 } = "MISC_GET_WORKSET_INFO";
    $pktDataH{ 0x0211 } = "MISC_SET_WORKSET_INFO";
    $pktDataH{ 0x0212 } = "UNUSED1";
    $pktDataH{ 0x0213 } = "UNUSED2";
    $pktDataH{ 0x0214 } = "CACHE_REFRESH_CCB";
    $pktDataH{ 0x0215 } = "SET_DLM_HEARTBEAT_LIST";
    $pktDataH{ 0x0216 } = "CACHE_FLUSH_BE";
    $pktDataH{ 0x0217 } = "MISC_RESYNC_RAIDS";
    $pktDataH{ 0x0218 } = "MISC_PUTDEVCONFIG";
    $pktDataH{ 0x0219 } = "MISC_GETDEVCONFIG";
    $pktDataH{ 0x021A } = "STATS_BUFFER_BOARD";
    $pktDataH{ 0x021B } = "MISC_MIRROR_PARTNER_GET_CFG";
    $pktDataH{ 0x021C } = "BATTERY_HEALTH_SET";
    $pktDataH{ 0x021D } = "MISC_INVALIDATE_FE_WC";
    $pktDataH{ 0x021E } = "STATS_SERVERS";
    $pktDataH{ 0x021F } = "VCG_VDISK_PRIORITY_ENABLE";
    $pktDataH{ 0x0220 } = "MISC_QUERY_MP_CHANGE";
    $pktDataH{ 0x0221 } = "MISC_RESYNCDATA";
    $pktDataH{ 0x0222 } = "MISC_RESYNCCTL";
    $pktDataH{ 0x0223 } = "MISC_SETTDISCACHE";
    $pktDataH{ 0x0224 } = "MISC_CLRTDISCACHE";
    $pktDataH{ 0x0225 } = "MISC_QTDISCACHE";
    $pktDataH{ 0x0226 } = "MISC_CFGOPTION";
    $pktDataH{ 0x0230 } = "MISC_LOCAL_RAID_INFO";
    $pktDataH{ 0x0229 } = "ISCSI_SET_TGTPARAM";
    $pktDataH{ 0x0231 } = "ISCSI_TGT_INFO";
    $pktDataH{ 0x0232 } = "ISCSI_SET_CHAP";
    $pktDataH{ 0x0233 } = "ISCSI_CHAP_INFO";
    $pktDataH{ 0x0234 } = "CLEAR_GEO_LOCATION";
    $pktDataH{ 0x0235 } = "ISCSI_SESSION_INFO";
    $pktDataH{ 0x0236 } = "ISCSI_SESSION_INFO_SERVER";
    $pktDataH{ 0x0237 } = "GETISNSINFO";
    $pktDataH{ 0x0238 } = "IDD_INFO";
    $pktDataH{ 0x0239 } = "SETISNSINFO";
    $pktDataH{ 0x0240 } = "DLM_PATH_STATS";
    $pktDataH{ 0x0241 } = "DLM_PATH_SELECTION_ALGO";
    $pktDataH{ 0x0242 } = "SET_PR";
    $pktDataH{ 0x0243 } = "GET_CPUCOUNT";
    $pktDataH{ 0x0244 } = "ENABLE_X1_PORT";
    $pktDataH{ 0x0245 } = "VDISK_BAY_REDUNDANT";
    $pktDataH{ 0x0246 } = "GET_BACKEND_TYPE";
    $pktDataH{ 0x0247 } = "QUICK_BREAK_PAUSE_RESUME_MIRROR_START";
    $pktDataH{ 0x0248 } = "QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE";
    $pktDataH{ 0x0249 } = "QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE";
    $pktDataH{ 0x024A } = "BEACON_ISE_COMPONENT";
    $pktDataH{ 0x024B } = "PDISKS_FROM_CACHE";
    $pktDataH{ 0x024C } = "VDISKS_FROM_CACHE";
    $pktDataH{ 0x024D } = "RAIDS_FROM_CACHE";
    $pktDataH{ 0x024E } = "BATCH_SNAPSHOT_START";
    $pktDataH{ 0x024F } = "BATCH_SNAPSHOT_SEQUENCE";
    $pktDataH{ 0x0250 } = "BATCH_SNAPSHOT_EXECUTE";
    $pktDataH{ 0x0300 } = "MFG_CTRL_CLEAN";
    $pktDataH{ 0x0500 } = "LOG_EVENT_MESSAGE";
    $pktDataH{ 0x0501 } = "ASYNC_CHANGED_EVENT";
    $pktDataH{ 0x0502 } = "ASYNC_PING_EVENT";

    # Add these hashes to the main hash
    $AllHashes{PI_ID} = \%pktIdH;
    $AllHashes{PI_DATA} = \%pktDataH;




    #====================================================================
    #                      I P C
    #====================================================================
    my %ipcIdH;
    my %ipcDataH;

    #
    # IPC id description table
    #
    $ipcIdH{ 99 } = "IPC Start";
    $ipcIdH{ 98 } = "IPC Callback";
    $ipcIdH{ 97 } = "IPC DspStart";
    $ipcIdH{ 96 } = "IPC DspDone";
    $ipcIdH{ 95 } = "IPC DspNull";
    $ipcIdH{ 94 } = "IPC DspTunSt";
    $ipcIdH{ 93 } = "IPC TunStart";
    $ipcIdH{ 0 }  = "IPC TimeOut";
    $ipcIdH{ 1 }  = "IPC NoPath";
    $ipcIdH{ 2 }  = "IPC AnyPath";
    $ipcIdH{ 3 }  = "IPC Ethernet";
    $ipcIdH{ 4 }  = "IPC Fibre";
    $ipcIdH{ 5 }  = "IPC Quorum";

    #
    # Packet data description table
    #
    $ipcDataH{ 601 } = "CONFIGURATION_UPDATE";
    $ipcDataH{ 603 } = "REPORT_CONTROLLER_FAILURE";
    $ipcDataH{ 604 } = "COMMAND_STATUS";
    $ipcDataH{ 605 } = "ELECT";
    $ipcDataH{ 607 } = "HEARTBEAT";
    $ipcDataH{ 609 } = "OFFLINE";
    $ipcDataH{ 614 } = "TUNNEL";
    $ipcDataH{ 618 } = "PING";
    $ipcDataH{ 619 } = "ONLINE";
    $ipcDataH{ 620 } = "SIGNAL";
    $ipcDataH{ 627 } = "FLUSH_BE_CACHE";
    $ipcDataH{ 629 } = "ENABLE_DISABLE_CACHE";
    $ipcDataH{ 630 } = "SET_MIRROR_PARTNER";
    $ipcDataH{ 632 } = "CONTINUE_WO_MIRROR";
    $ipcDataH{ 634 } = "RESCAN_DEVICES";
    $ipcDataH{ 636 } = "SET_DLM_HEARTBEAT_LIST";
    $ipcDataH{ 638 } = "ELECT_QUORUM";
    $ipcDataH{ 639 } = "BROADCAST";
    $ipcDataH{ 640 } = "GET_MIRROR_PARTNER";
    $ipcDataH{ 641 } = "GET_MIRROR_PARTNER_RESPONSE";
    $ipcDataH{ 642 } = "ADD_CONTROLLER";
    $ipcDataH{ 643 } = "FLUSH_COMPLETED";
    $ipcDataH{ 644 } = "LED_CHANGE";
    $ipcDataH{ 649 } = "CLIENT_PERSISTENT_DATA_CMD";
    $ipcDataH{ 650 } = "RESYNC_CLIENT_CMD";
    $ipcDataH{ 651 } = "RESYNC_CLIENT_RECORD";
    $ipcDataH{ 652 } = "LATEST_PERSISTENT_DATA";
    $ipcDataH{ 653 } = "SETPRES_DATA_CMD";

    # Add these hashes to the main hash
    $AllHashes{IPC_ID} = \%ipcIdH;
    $AllHashes{IPC_DATA} = \%ipcDataH;




    #====================================================================
    #                      Logs
    #====================================================================
    my %logIdH;
    my %logDataH;

    #
    # Log Message id description table
    #							 
    $logIdH{ 0 } = "Log Event";

    #
    # Log Message data description table
    #							 
    $logDataH{ 0x0000 } = "SCRUB_DONE";
    $logDataH{ 0x0001 } = "NVRAM_RESTORE";
    $logDataH{ 0x0002 } = "BOOT_CODE_EVENT_INFO";
    $logDataH{ 0x0003 } = "BOOT_COMPLETE";
    $logDataH{ 0x0004 } = "DEVICE_REMOVED";
    $logDataH{ 0x0005 } = "DEVICE_INSERTED";
    $logDataH{ 0x0006 } = "DEVICE_REATTACED";
    $logDataH{ 0x0007 } = "COPY_COMPLETE";
    $logDataH{ 0x0008 } = "HOST_OFFLINE";
    $logDataH{ 0x0009 } = "LOG_TEXT_MESSAGE_INFO";
    $logDataH{ 0x000A } = "BUFFER_BOARDS_ENABLED";
    $logDataH{ 0x000B } = "CONFIG_CHANGED";
    $logDataH{ 0x000C } = "CACHE_FLUSH_RECOVER";
    $logDataH{ 0x000D } = "FIRMWARE_UPDATE";
    $logDataH{ 0x000E } = "SERVER_CREATE_OP";
    $logDataH{ 0x000F } = "SERVER_DELETE_OP";
    $logDataH{ 0x0010 } = "SERVER_ASSOC_OP";
    $logDataH{ 0x0011 } = "SERVER_DISASSOC_OP";
    $logDataH{ 0x0012 } = "VDISK_CREATE_OP";
    $logDataH{ 0x0013 } = "VDISK_DELETE_OP";
    $logDataH{ 0x0014 } = "VDISK_EXPAND_OP";
    $logDataH{ 0x0015 } = "VDISK_PREPARE_OP";
    $logDataH{ 0x0016 } = "VDISK_CONTROL_OP";
    $logDataH{ 0x0017 } = "VCG_SET_CACHE_OP";
    $logDataH{ 0x0018 } = "VDISK_SET_ATTRIBUTE_OP";
    $logDataH{ 0x0019 } = "RAID_INIT_OP";
    $logDataH{ 0x001A } = "RAID_CONTROL_OP";
    $logDataH{ 0x001B } = "DEVICE_DELETE_OP";
    $logDataH{ 0x001C } = "VLINK_BREAK_LOCK_OP";
    $logDataH{ 0x001D } = "PROC_ASSIGN_MIRROR_PARTNER_OP";
    $logDataH{ 0x001E } = "DRIVE_BAY_FW_UPDATE";
    $logDataH{ 0x001F } = "PDISK_LABEL_OP";
    $logDataH{ 0x0020 } = "PDISK_FAIL_OP";
    $logDataH{ 0x0021 } = "PDISK_DEFRAG_OP";
    $logDataH{ 0x0022 } = "PSD_REBUILD_DONE";
    $logDataH{ 0x0023 } = "PDD_REBUILD_DONE";
    $logDataH{ 0x0024 } = "HSPARE_DONE";
    $logDataH{ 0x0025 } = "RAID_INIT_DONE";
    $logDataH{ 0x0026 } = "LOOPUP";
    $logDataH{ 0x0026 } = "DLOOPUP";
    $logDataH{ 0x0027 } = "PORTDBCHANGED";
    $logDataH{ 0x0028 } = "CONFIGURATION_SESSION_START";
    $logDataH{ 0x0029 } = "CONFIGURATION_SESSION_END";
    $logDataH{ 0x002A } = "TARGET_SET_PROPERTIES_OP";
    $logDataH{ 0x002B } = "SES_FAN_ON";
    $logDataH{ 0x002C } = "PSD_REBUILD_START";
    $logDataH{ 0x002D } = "EST_VLINK";
    $logDataH{ 0x002E } = "TERM_VLINK";
    $logDataH{ 0x002F } = "SWP_VLINK";
    $logDataH{ 0x0030 } = "NEW_PATH";
    $logDataH{ 0x0031 } = "FW_VERSIONS";
    $logDataH{ 0x0032 } = "VLINK_SIZE_CHANGED";
    $logDataH{ 0x0033 } = "PARITY_CHECK_DONE";
    $logDataH{ 0x0034 } = "DEFRAG_DONE";
    $logDataH{ 0x0035 } = "I2C_BUS_GOOD";
    $logDataH{ 0x0036 } = "VCG_INACTIVATE_CONTROLLER_OP";
    $logDataH{ 0x0037 } = "VCG_ACTIVATE_CONTROLLER_OP";
    $logDataH{ 0x0038 } = "VLINK_CREATE_OP";
    $logDataH{ 0x0039 } = "SERVER_SET_PROPERTIES_OP";
    $logDataH{ 0x003A } = "ETHERNET_LINK_UP";
    $logDataH{ 0x003B } = "WRITE_FLUSH_COMPLETE";
    $logDataH{ 0x003C } = "NVA_RESYNC_FAILED";
    $logDataH{ 0x003D } = "FS_UPDATE";
    $logDataH{ 0x003E } = "PULSE";
    $logDataH{ 0x003F } = "CACHE_TAGS_RECOVERED";
    $logDataH{ 0x0040 } = "NVRAM_RELOAD";
    $logDataH{ 0x0041 } = "IPC_LINK_UP";
    $logDataH{ 0x0042 } = "IPC_BROADCAST";
    $logDataH{ 0x0043 } = "WORKSET_CHANGED";
    $logDataH{ 0x0044 } = "INIT_PROC_NVRAM_OP";
    $logDataH{ 0x0045 } = "INIT_CCB_NVRAM_OP";
    $logDataH{ 0x0046 } = "SET_SYSTEM_SERIAL_NUM_OP";
    $logDataH{ 0x0047 } = "VCG_PREPARE_SLAVE_OP";
    $logDataH{ 0x0048 } = "VCG_ADD_SLAVE_OP";
    $logDataH{ 0x0049 } = "VCG_SET_MIRROR_PARTNERS_OP";
    $logDataH{ 0x004A } = "VCG_START_IO_OP";
    $logDataH{ 0x004B } = "VCG_STOP_IO_OP";
    $logDataH{ 0x004C } = "VCG_GLOBAL_CACHE_OP";
    $logDataH{ 0x004D } = "VCG_APPLY_LICENSE_OP";
    $logDataH{ 0x004E } = "VCG_UNFAIL_CONTROLLER_OP";
    $logDataH{ 0x004F } = "VCG_FAIL_CONTROLLER_OP";
    $logDataH{ 0x0050 } = "VCG_REMOVE_CONTROLLER_OP";
    $logDataH{ 0x0051 } = "VCG_SHUTDOWN_OP";
    $logDataH{ 0x0052 } = "FE_QLOGIC_RESET_OP";
    $logDataH{ 0x0053 } = "BE_QLOGIC_RESET_OP";
    $logDataH{ 0x0054 } = "PROC_START_IO_OP";
    $logDataH{ 0x0055 } = "PROC_STOP_IO_OP";
    $logDataH{ 0x0056 } = "PROC_SYSTEM_NMI";
    $logDataH{ 0x0057 } = "ADMIN_SETTIME_OP";
    $logDataH{ 0x0058 } = "LOG_FAILURE_EVENT_INFO";
    $logDataH{ 0x0059 } = "SNAPSHOT_RESTORED";
    $logDataH{ 0x005A } = "CONTROLLERS_READY";
    $logDataH{ 0x005B } = "POWER_UP_COMPLETE";
    $logDataH{ 0x005C } = "RM_EVENT_INFO";
    $logDataH{ 0x005D } = "ADMIN_SET_IP_OP";
    $logDataH{ 0x005E } = "SNAPSHOT_TAKEN";
    $logDataH{ 0x005F } = "COPY_SYNC";
    $logDataH{ 0x0060 } = "ALL_DEV_MISSING";
    $logDataH{ 0x0061 } = "XSSA_LOG_MESSAGE";
    $logDataH{ 0x0062 } = "DEVICE_FAIL_HS";
    $logDataH{ 0x0063 } = "SMART_EVENT";
    $logDataH{ 0x0064 } = "SPINUP_FAILED";
    $logDataH{ 0x0065 } = "LOG_TEXT_MESSAGE_WARNING";
    $logDataH{ 0x0066 } = "VALIDATION";
    $logDataH{ 0x0067 } = "CACHE_FLUSH_FAILED";
    $logDataH{ 0x0068 } = "FIRMWARE_ALERT";
    $logDataH{ 0x0069 } = "LOOP_DOWN";
    $logDataH{ 0x006A } = "CCB_NVRAM_RESTORED";
    $logDataH{ 0x006B } = "SES_DEV_BYPA";
    $logDataH{ 0x006C } = "SES_DEV_BYPB";
    $logDataH{ 0x006D } = "SES_DEV_OFF";
    $logDataH{ 0x006E } = "SES_PS_TEMP_WARN";
    $logDataH{ 0x006F } = "SES_VOLTAGE_HI_WARN";
    $logDataH{ 0x0070 } = "SES_VOLTAGE_LO_WARN";
    $logDataH{ 0x0071 } = "SES_TEMP_WARN";
    $logDataH{ 0x0072 } = "LOG_UNUSED1";
    $logDataH{ 0x0073 } = "LOST_PATH";
    $logDataH{ 0x0074 } = "ISP_CHIP_RESET";
    $logDataH{ 0x0075 } = "CACHE_MIRROR_FAILED";
    $logDataH{ 0x0076 } = "SINGLE_PATH";
    $logDataH{ 0x0077 } = "HOTSPARE_INOP";
    $logDataH{ 0x0078 } = "ETHERNET_LINK_DOWN";
    $logDataH{ 0x0079 } = "MSG_DELETED";
    $logDataH{ 0x007A } = "VCG_SHUTDOWN_WARN";
    $logDataH{ 0x007B } = "SES_CURRENT_HI_WARN";
    $logDataH{ 0x007C } = "SES_EL_REPORT";
    $logDataH{ 0x007D } = "SES_EL_PRESENT";
    $logDataH{ 0x007E } = "LOG_FAILURE_EVENT_WARN";
    $logDataH{ 0x007F } = "RM_WARN";
    $logDataH{ 0x0080 } = "SES_IO_MOD_PULLED";
    $logDataH{ 0x0081 } = "BUFFER_BOARDS_DISABLED_INFO";
    $logDataH{ 0x0082 } = "BUFFER_BOARDS_DISABLED_WARN";
    $logDataH{ 0x0083 } = "BUFFER_BOARDS_DISABLED_ERROR";
    $logDataH{ 0x0084 } = "BOOT_CODE_EVENT_WARN";
    $logDataH{ 0x0085 } = "DEVICE_FAIL_NO_HS";
    $logDataH{ 0x0086 } = "DEVICE_MISSING";
    $logDataH{ 0x0087 } = "SERIAL_WRONG";
    $logDataH{ 0x0088 } = "NVA_BAD";
    $logDataH{ 0x0089 } = "LOG_TEXT_MESSAGE_ERROR";
    $logDataH{ 0x008A } = "CACHE_DRAM_FAIL";
    $logDataH{ 0x008B } = "CACHE_RECOVER_FAIL";
    $logDataH{ 0x008C } = "COPY_FAILED";
    $logDataH{ 0x008D } = "SES_DEV_FLT";
    $logDataH{ 0x008E } = "SES_PS_OVER_TEMP";
    $logDataH{ 0x008F } = "SES_PS_AC_FAIL";
    $logDataH{ 0x0090 } = "SES_PS_DC_FAIL";
    $logDataH{ 0x0091 } = "SES_FAN_FAIL";
    $logDataH{ 0x0092 } = "SES_TEMP_FAIL";
    $logDataH{ 0x0093 } = "SES_VOLTAGE_HI";
    $logDataH{ 0x0094 } = "SES_VOLTAGE_LO";
    $logDataH{ 0x0095 } = "ERR_TRAP";
    $logDataH{ 0x0096 } = "FOREIGN_PCI";
    $logDataH{ 0x0097 } = "CCB_NVRAM_RESET";
    $logDataH{ 0x0098 } = "WC_SEQNO_BAD";
    $logDataH{ 0x0099 } = "VID_RECOVERY_FAIL";
    $logDataH{ 0x009A } = "INVALID_TAG";
    $logDataH{ 0x009B } = "SES_FAN_OFF";
    $logDataH{ 0x009C } = "SES_PS_DC_OVERVOLT";
    $logDataH{ 0x009D } = "SES_PS_DC_UNDERVOLT";
    $logDataH{ 0x009E } = "SES_PS_DC_OVERCURR";
    $logDataH{ 0x009F } = "SES_PS_OFF";
    $logDataH{ 0x00A0 } = "SES_PS_FAIL";
    $logDataH{ 0x00A1 } = "RSCN";
    $logDataH{ 0x00A2 } = "FILEIO_ERR";
    $logDataH{ 0x00A3 } = "PROC_NOT_READY";
    $logDataH{ 0x00A4 } = "I2C_BUS_FAIL";
    $logDataH{ 0x00A5 } = "ILLEGAL_ELECTION_STATE";
    $logDataH{ 0x00A6 } = "RAID_ERROR";
    $logDataH{ 0x00A7 } = "SERIAL_MISMATCH";
    $logDataH{ 0x00A8 } = "PORT_INIT_FAILED";
    $logDataH{ 0x00A9 } = "PORT_EVENT";
    $logDataH{ 0x00AA } = "NVRAM_CHKSUM_ERR";
    $logDataH{ 0x00AB } = "BATTERY_ALERT";
    $logDataH{ 0x00AC } = "FW_UPDATE_FAILED";
    $logDataH{ 0x00AD } = "BE_INITIATOR";
    $logDataH{ 0x00AE } = "LOST_ALL_PATHS";
    $logDataH{ 0x00AF } = "FS_UPDATE_FAIL";
    $logDataH{ 0x00B0 } = "DEVICE_TIMEOUT";
    $logDataH{ 0x00B1 } = "SES_EL_FAIL";
    $logDataH{ 0x00B2 } = "SES_LOOPAFAIL";
    $logDataH{ 0x00B3 } = "SES_LOOPBFAIL";
    $logDataH{ 0x00B4 } = "SES_SPEEDMIS";
    $logDataH{ 0x00B5 } = "SES_FWMISMATCH";
    $logDataH{ 0x00B6 } = "SES_CURRENT_HI";
    $logDataH{ 0x00B7 } = "IPC_LINK_DOWN";
    $logDataH{ 0x00B8 } = "PROC_COMM_NOT_READY";
    $logDataH{ 0x00B9 } = "NO_OWNED_DRIVES";
    $logDataH{ 0x00BA } = "CTRL_FAILED";
    $logDataH{ 0x00BB } = "CTRL_UNUSED";
    $logDataH{ 0x00BC } = "FWV_INCOMPATIBLE";
    $logDataH{ 0x00BD } = "LOG_FAILURE_EVENT";
    $logDataH{ 0x00BE } = "SNAPSHOT_RESTORE_FAILED";
    $logDataH{ 0x00BF } = "MISSING_DISK_BAY";
    $logDataH{ 0x00C0 } = "WAIT_CORRUPT_BE_NVRAM";
    $logDataH{ 0x00C1 } = "MISSING_CONTROLLER";
    $logDataH{ 0x00C2 } = "RM_ERROR";
    $logDataH{ 0x00C3 } = "BOOT_CODE_EVENT_ERROR";
    $logDataH{ 0x00C4 } = "CHANGE_LED";
    $logDataH{ 0x00C5 } = "WAIT_DISASTER";
    $logDataH{ 0x00C6 } = "CHANGE_NAME";
    $logDataH{ 0x00C7 } = "GET_LIST_ERROR";
    $logDataH{ 0x00C8 } = "ELECTION_STATE_CHANGE";
    $logDataH{ 0x00C9 } = "LOCAL_IMAGE_READY";
    $logDataH{ 0x00CA } = "REFRESH_NVRAM";
    $logDataH{ 0x00CB } = "MAG_DRIVER_ERR";
    $logDataH{ 0x00CC } = "HOST_NONSENSE";
    $logDataH{ 0x00CD } = "HOST_QLOGIC_ERR";
    $logDataH{ 0x00CE } = "RAID_EVENT";
    $logDataH{ 0x00CF } = "IOCB";
    $logDataH{ 0x00D0 } = "SINGLE_BIT_ECC";
    $logDataH{ 0x00D1 } = "RESYNC_DONE";
    $logDataH{ 0x00D2 } = "SHORT_SCSI_EVENT";
    $logDataH{ 0x00D3 } = "LONG_SCSI_EVENT";
    $logDataH{ 0x00D4 } = "HOST_IMED_NOTIFY";
    $logDataH{ 0x00D5 } = "HOST_SENSE_DATA";
    $logDataH{ 0x00D6 } = "ZONE_INQUIRY";
    $logDataH{ 0x00D7 } = "FRAMEDROPPED";
    $logDataH{ 0x00D8 } = "SOCKET_ERROR";
    $logDataH{ 0x00D9 } = "NVRAM_WRITTEN";
    $logDataH{ 0x00DA } = "HBEAT_STOP";
    $logDataH{ 0x00DB } = "PARITY_SCAN_REQUIRED";
    $logDataH{ 0x00DC } = "PDISK_UNFAIL_OP";
    $logDataH{ 0x00DD } = "MB_FAILED";
    $logDataH{ 0x00DE } = "LIP";
    $logDataH{ 0x00DF } = "IOCBTO";
    $logDataH{ 0x00E0 } = "TASKTOOLONG";
    $logDataH{ 0x00E1 } = "VLINK_NAME_CHANGED";
    $logDataH{ 0x00E2 } = "FILEIO_DEBUG";
    $logDataH{ 0x00E3 } = "LOG_TEXT_MESSAGE_DEBUG";
    $logDataH{ 0x00E4 } = "PHY_RETRY";
    $logDataH{ 0x00E5 } = "DVLIST";
    $logDataH{ 0x00E6 } = "PORT_UP";
    $logDataH{ 0x00E7 } = "PHY_ACTION";
    $logDataH{ 0x00E8 } = "NO_LICENSE";
    $logDataH{ 0x00E9 } = "NO_MIRROR_PARTNER";
    $logDataH{ 0x00EA } = "LPDN_RETRY";
    $logDataH{ 0x00EB } = "BOOT_CODE_EVENT_DEBUG";
    $logDataH{ 0x00EC } = "VALIDATION_DEBUG";
    $logDataH{ 0x00ED } = "LOOP_PRIMITIVE_DEBUG";
    $logDataH{ 0x00EE } = "PROC_NAME_DEVICE_OP";
    $logDataH{ 0x00EF } = "HARDWARE_MONITOR_DEBUG";
    $logDataH{ 0x00F0 } = "HARDWARE_MONITOR_STATUS_INFO";
    $logDataH{ 0x00F1 } = "CCB_STATUS_INFO";
    $logDataH{ 0x00F2 } = "PROC_BOARD_STATUS_INFO";
    $logDataH{ 0x00F3 } = "FE_BUFFER_BOARD_STATUS_INFO";
    $logDataH{ 0x00F4 } = "BE_BUFFER_BOARD_STATUS_INFO";
    $logDataH{ 0x00F5 } = "FE_POWER_SUPPLY_STATUS_INFO";
    $logDataH{ 0x00F6 } = "BE_POWER_SUPPLY_STATUS_INFO";
    $logDataH{ 0x00F7 } = "CCB_MEMORY_MODULE_EEPROM_STATUS_INFO";
    $logDataH{ 0x00F8 } = "CHASSIS_EEPROM_STATUS_INFO";
    $logDataH{ 0x00F9 } = "PROC_BOARD_EEPROM_STATUS_INFO";
    $logDataH{ 0x00FA } = "FE_POWER_SUPPLY_ASM_EEPROM_STATUS_INFO";
    $logDataH{ 0x00FB } = "BE_POWER_SUPPLY_ASM_EEPROM_STATUS_INFO";
    $logDataH{ 0x00FC } = "FE_BUFFER_BOARD_EEPROM_STATUS_INFO";
    $logDataH{ 0x00FD } = "BE_BUFFER_BOARD_EEPROM_STATUS_INFO";
    $logDataH{ 0x00FE } = "FE_POWER_SUPPLY_INT_EEPROM_STATUS_INFO";
    $logDataH{ 0x00FF } = "BE_POWER_SUPPLY_INT_EEPROM_STATUS_INFO";
    $logDataH{ 0x0100 } = "HARDWARE_MONITOR_STATUS_WARN";
    $logDataH{ 0x0101 } = "CCB_STATUS_WARN";
    $logDataH{ 0x0102 } = "PROC_BOARD_STATUS_WARN";
    $logDataH{ 0x0103 } = "FE_BUFFER_BOARD_STATUS_WARN";
    $logDataH{ 0x0104 } = "BE_BUFFER_BOARD_STATUS_WARN";
    $logDataH{ 0x0105 } = "FE_POWER_SUPPLY_STATUS_WARN";
    $logDataH{ 0x0106 } = "BE_POWER_SUPPLY_STATUS_WARN";
    $logDataH{ 0x0107 } = "CCB_MEMORY_MODULE_EEPROM_STATUS_WARN";
    $logDataH{ 0x0108 } = "CHASSIS_EEPROM_STATUS_WARN";
    $logDataH{ 0x0109 } = "PROC_BOARD_EEPROM_STATUS_WARN";
    $logDataH{ 0x010A } = "FE_POWER_SUPPLY_ASM_EEPROM_STATUS_WARN";
    $logDataH{ 0x010B } = "BE_POWER_SUPPLY_ASM_EEPROM_STATUS_WARN";
    $logDataH{ 0x010C } = "FE_BUFFER_BOARD_EEPROM_STATUS_WARN";
    $logDataH{ 0x010D } = "BE_BUFFER_BOARD_EEPROM_STATUS_WARN";
    $logDataH{ 0x010E } = "FE_POWER_SUPPLY_INT_EEPROM_STATUS_WARN";
    $logDataH{ 0x010F } = "BE_POWER_SUPPLY_INT_EEPROM_STATUS_WARN";
    $logDataH{ 0x0110 } = "HARDWARE_MONITOR_STATUS_ERROR";
    $logDataH{ 0x0111 } = "CCB_STATUS_ERROR";
    $logDataH{ 0x0112 } = "PROC_BOARD_STATUS_ERROR";
    $logDataH{ 0x0113 } = "FE_BUFFER_BOARD_STATUS_ERROR";
    $logDataH{ 0x0114 } = "BE_BUFFER_BOARD_STATUS_ERROR";
    $logDataH{ 0x0115 } = "FE_POWER_SUPPLY_STATUS_ERROR";
    $logDataH{ 0x0116 } = "BE_POWER_SUPPLY_STATUS_ERROR";
    $logDataH{ 0x0117 } = "CCB_MEMORY_MODULE_EEPROM_STATUS_ERROR";
    $logDataH{ 0x0118 } = "CHASSIS_EEPROM_STATUS_ERROR";
    $logDataH{ 0x0119 } = "PROC_BOARD_EEPROM_STATUS_ERROR";
    $logDataH{ 0x011A } = "FE_POWER_SUPPLY_ASM_EEPROM_STATUS_ERROR";
    $logDataH{ 0x011B } = "BE_POWER_SUPPLY_ASM_EEPROM_STATUS_ERROR";
    $logDataH{ 0x011C } = "FE_BUFFER_BOARD_EEPROM_STATUS_ERROR";
    $logDataH{ 0x011D } = "BE_BUFFER_BOARD_EEPROM_STATUS_ERROR";
    $logDataH{ 0x011E } = "FE_POWER_SUPPLY_INT_EEPROM_STATUS_ERROR";
    $logDataH{ 0x011F } = "BE_POWER_SUPPLY_INT_EEPROM_STATUS_ERROR";
    $logDataH{ 0x0120 } = "HARDWARE_MONITOR_INFO";
    $logDataH{ 0x0121 } = "CCB_PROCESSOR_HW_INFO";
    $logDataH{ 0x0122 } = "CCB_EEPROM_HW_INFO";
    $logDataH{ 0x0123 } = "CCB_MEMORY_MODULE_HW_INFO";
    $logDataH{ 0x0124 } = "CCB_MEMORY_MODULE_EEPROM_HW_INFO";
    $logDataH{ 0x0125 } = "PROC_BOARD_LM80_LM87_HW_INFO";
    $logDataH{ 0x0126 } = "PROC_BOARD_LM75_LM92_HW_INFO";
    $logDataH{ 0x0127 } = "PROC_BOARD_POWER_SUPPLY_PCF8574_HW_INFO";
    $logDataH{ 0x0128 } = "PROC_BOARD_RESET_CONTROL_PCF8574_HW_INFO";
    $logDataH{ 0x0129 } = "CHASSIS_EEPROM_HW_INFO";
    $logDataH{ 0x012A } = "PROC_BOARD_EEPROM_HW_INFO";
    $logDataH{ 0x012B } = "FE_POWER_SUPPLY_ASM_EEPROM_HW_INFO";
    $logDataH{ 0x012C } = "BE_POWER_SUPPLY_ASM_EEPROM_HW_INFO";
    $logDataH{ 0x012D } = "FE_BUFFER_BOARD_LM80_LM87_HW_INFO";
    $logDataH{ 0x012E } = "FE_BUFFER_BOARD_PCF8574_HW_INFO";
    $logDataH{ 0x012F } = "FE_BUFFER_BOARD_MAX1660_HW_INFO";
    $logDataH{ 0x0130 } = "FE_BUFFER_BOARD_EEPROM_HW_INFO";
    $logDataH{ 0x0131 } = "BE_BUFFER_BOARD_LM80_LM87_HW_INFO";
    $logDataH{ 0x0132 } = "BE_BUFFER_BOARD_PCF8574_HW_INFO";
    $logDataH{ 0x0133 } = "BE_BUFFER_BOARD_MAX1660_HW_INFO";
    $logDataH{ 0x0134 } = "BE_BUFFER_BOARD_EEPROM_HW_INFO";
    $logDataH{ 0x0135 } = "FE_POWER_SUPPLY_INT_EEPROM_HW_INFO";
    $logDataH{ 0x0136 } = "BE_POWER_SUPPLY_INT_EEPROM_HW_INFO";
    $logDataH{ 0x0137 } = "I2C_HW_RESERVED_3_INFO";
    $logDataH{ 0x0138 } = "I2C_HW_RESERVED_4_INFO";
    $logDataH{ 0x0139 } = "I2C_HW_RESERVED_5_INFO";
    $logDataH{ 0x013A } = "I2C_HW_RESERVED_6_INFO";
    $logDataH{ 0x013B } = "I2C_HW_RESERVED_7_INFO";
    $logDataH{ 0x013C } = "I2C_HW_RESERVED_8_INFO";
    $logDataH{ 0x013D } = "I2C_HW_RESERVED_9_INFO";
    $logDataH{ 0x013E } = "I2C_HW_RESERVED_10_INFO";
    $logDataH{ 0x013F } = "CCB_BOARD_EEPROM_STATUS_INFO";
    $logDataH{ 0x0140 } = "HARDWARE_MONITOR_WARN";
    $logDataH{ 0x0141 } = "CCB_PROCESSOR_HW_WARN";
    $logDataH{ 0x0142 } = "CCB_EEPROM_HW_WARN";
    $logDataH{ 0x0143 } = "CCB_MEMORY_MODULE_HW_WARN";
    $logDataH{ 0x0144 } = "CCB_MEMORY_MODULE_EEPROM_HW_WARN";
    $logDataH{ 0x0145 } = "PROC_BOARD_LM80_LM87_HW_WARN";
    $logDataH{ 0x0146 } = "PROC_BOARD_LM75_LM92_HW_WARN";
    $logDataH{ 0x0147 } = "PROC_BOARD_POWER_SUPPLY_PCF8574_HW_WARN";
    $logDataH{ 0x0148 } = "PROC_BOARD_RESET_CONTROL_PCF8574_HW_WARN";
    $logDataH{ 0x0149 } = "CHASSIS_EEPROM_HW_WARN";
    $logDataH{ 0x014A } = "PROC_BOARD_EEPROM_HW_WARN";
    $logDataH{ 0x014B } = "FE_POWER_SUPPLY_ASM_EEPROM_HW_WARN";
    $logDataH{ 0x014C } = "BE_POWER_SUPPLY_ASM_EEPROM_HW_WARN";
    $logDataH{ 0x014D } = "FE_BUFFER_BOARD_LM80_LM87_HW_WARN";
    $logDataH{ 0x014E } = "FE_BUFFER_BOARD_PCF8574_HW_WARN";
    $logDataH{ 0x014F } = "FE_BUFFER_BOARD_MAX1660_HW_WARN";
    $logDataH{ 0x0150 } = "FE_BUFFER_BOARD_EEPROM_HW_WARN";
    $logDataH{ 0x0151 } = "BE_BUFFER_BOARD_LM80_LM87_HW_WARN";
    $logDataH{ 0x0152 } = "BE_BUFFER_BOARD_PCF8574_HW_WARN";
    $logDataH{ 0x0153 } = "BE_BUFFER_BOARD_MAX1660_HW_WARN";
    $logDataH{ 0x0154 } = "BE_BUFFER_BOARD_EEPROM_HW_WARN";
    $logDataH{ 0x0155 } = "FE_POWER_SUPPLY_INT_EEPROM_HW_WARN";
    $logDataH{ 0x0156 } = "BE_POWER_SUPPLY_INT_EEPROM_HW_WARN";
    $logDataH{ 0x0157 } = "I2C_HW_RESERVED_3_WARN";
    $logDataH{ 0x0158 } = "I2C_HW_RESERVED_4_WARN";
    $logDataH{ 0x0159 } = "I2C_HW_RESERVED_5_WARN";
    $logDataH{ 0x015A } = "I2C_HW_RESERVED_6_WARN";
    $logDataH{ 0x015B } = "I2C_HW_RESERVED_7_WARN";
    $logDataH{ 0x015C } = "I2C_HW_RESERVED_8_WARN";
    $logDataH{ 0x015D } = "I2C_HW_RESERVED_9_WARN";
    $logDataH{ 0x015E } = "I2C_HW_RESERVED_10_WARN";
    $logDataH{ 0x015F } = "CCB_BOARD_EEPROM_STATUS_WARN";
    $logDataH{ 0x0160 } = "HARDWARE_MONITOR_ERROR";
    $logDataH{ 0x0161 } = "CCB_PROCESSOR_HW_ERROR";
    $logDataH{ 0x0162 } = "CCB_EEPROM_HW_ERROR";
    $logDataH{ 0x0163 } = "CCB_MEMORY_MODULE_HW_ERROR";
    $logDataH{ 0x0164 } = "CCB_MEMORY_MODULE_EEPROM_HW_ERROR";
    $logDataH{ 0x0165 } = "PROC_BOARD_LM80_LM87_HW_ERROR";
    $logDataH{ 0x0166 } = "PROC_BOARD_LM75_LM92_HW_ERROR";
    $logDataH{ 0x0167 } = "PROC_BOARD_POWER_SUPPLY_PCF8574_HW_ERROR";
    $logDataH{ 0x0168 } = "PROC_BOARD_RESET_CONTROL_PCF8574_HW_ERROR";
    $logDataH{ 0x0169 } = "CHASSIS_EEPROM_HW_ERROR";
    $logDataH{ 0x016A } = "PROC_BOARD_EEPROM_HW_ERROR";
    $logDataH{ 0x016B } = "FE_POWER_SUPPLY_ASM_EEPROM_HW_ERROR";
    $logDataH{ 0x016C } = "BE_POWER_SUPPLY_ASM_EEPROM_HW_ERROR";
    $logDataH{ 0x016D } = "FE_BUFFER_BOARD_LM80_LM87_HW_ERROR";
    $logDataH{ 0x016E } = "FE_BUFFER_BOARD_PCF8574_HW_ERROR";
    $logDataH{ 0x016F } = "FE_BUFFER_BOARD_MAX1660_HW_ERROR";
    $logDataH{ 0x0170 } = "FE_BUFFER_BOARD_EEPROM_HW_ERROR";
    $logDataH{ 0x0171 } = "BE_BUFFER_BOARD_LM80_LM87_HW_ERROR";
    $logDataH{ 0x0172 } = "BE_BUFFER_BOARD_PCF8574_HW_ERROR";
    $logDataH{ 0x0173 } = "BE_BUFFER_BOARD_MAX1660_HW_ERROR";
    $logDataH{ 0x0174 } = "BE_BUFFER_BOARD_EEPROM_HW_ERROR";
    $logDataH{ 0x0175 } = "FE_POWER_SUPPLY_INT_EEPROM_HW_ERROR";
    $logDataH{ 0x0176 } = "BE_POWER_SUPPLY_INT_EEPROM_HW_ERROR";
    $logDataH{ 0x0177 } = "I2C_HW_RESERVED_3_ERROR";
    $logDataH{ 0x0178 } = "I2C_HW_RESERVED_4_ERROR";
    $logDataH{ 0x0179 } = "I2C_HW_RESERVED_5_ERROR";
    $logDataH{ 0x017A } = "I2C_HW_RESERVED_6_ERROR";
    $logDataH{ 0x017B } = "I2C_HW_RESERVED_7_ERROR";
    $logDataH{ 0x017C } = "I2C_HW_RESERVED_8_ERROR";
    $logDataH{ 0x017D } = "I2C_HW_RESERVED_9_ERROR";
    $logDataH{ 0x017E } = "I2C_HW_RESERVED_10_ERROR";
    $logDataH{ 0x017F } = "CCB_BOARD_EEPROM_STATUS_ERROR";
    $logDataH{ 0x0180 } = "DIAG_RESULT";
    $logDataH{ 0x0181 } = "CCB_MEMORY_HEALTH_ALERT";
    $logDataH{ 0x0182 } = "DISKBAY_REMOVED";
    $logDataH{ 0x0183 } = "DISKBAY_INSERTED";
    $logDataH{ 0x0184 } = "PROC_MEMORY_HEALTH_ALERT";
    $logDataH{ 0x0185 } = "DISKBAY_MOVED";
    $logDataH{ 0x0186 } = "PCI_CFG_ERR";
    $logDataH{ 0x0187 } = "FCM";
    $logDataH{ 0x0188 } = "HOTSPARE_DEPLETED";
    $logDataH{ 0x0189 } = "CSTOP_LOG";
    $logDataH{ 0x018A } = "CONTROLLER_FAIL";
    $logDataH{ 0x018B } = "DISASTER";
    $logDataH{ 0x018C } = "MIRROR_CAPABLE";
    $logDataH{ 0x018D } = "#UNUSED was - MOVING_TARGETS";
    $logDataH{ 0x018E } = "#UNUSED was - PROC_PRINTF";
    $logDataH{ 0x018F } = "DRV_FLT";
    $logDataH{ 0x0190 } = "BYPASS_DEVICE";
    $logDataH{ 0x0191 } = "VLINK_OPEN_BEGIN";
    $logDataH{ 0x0192 } = "VLINK_OPEN_END";
    $logDataH{ 0x0193 } = "RAID5_INOPERATIVE";
    $logDataH{ 0x0194 } = "NVRAM_WRITE_FAIL";
    $logDataH{ 0x0195 } = "PARITY_CHECK_RAID";
    $logDataH{ 0x0196 } = "ISP_J2";
    $logDataH{ 0x0197 } = "ORPHAN_DETECTED";
    $logDataH{ 0x0198 } = "DEFRAG_VER_DONE";
    $logDataH{ 0x0199 } = "DEFRAG_OP_COMPLETE";
    $logDataH{ 0x019A } = "NV_MEM_EVENT";
    $logDataH{ 0x019B } = "WC_FLUSH";
    $logDataH{ 0x019C } = "WC_SN_VCG_BAD";
    $logDataH{ 0x019D } = "WC_SN_BAD";
    $logDataH{ 0x019E } = "WC_NVMEM_BAD";
    $logDataH{ 0x019F } = "SES_SBOD_EXT";
    $logDataH{ 0x01A0 } = "IPMI_EVENT";
    $logDataH{ 0x01A1 } = "WAIT_CACHE_ERROR";
    $logDataH{ 0x01A2 } = "SES_SBOD_STATECODE";
    $logDataH{ 0x01A3 } = "SES_ELEM_CHANGE";
    $logDataH{ 0x01A4 } = "VDISK_SET_PRIORITY";
    $logDataH{ 0x01A5 } = "PDISK_SPINDOWN_OP";
    $logDataH{ 0x01A6 } = "PDISK_FAILBACK_OP";
    $logDataH{ 0x01A7 } = "PDISK_AUTO_FAILBACK_OP";
    $logDataH{ 0x01A8 } = "DEVICE_RESET";
    $logDataH{ 0x01A9 } = "TARGET_UP_OP";
    $logDataH{ 0x01AA } = "ISCSI_SET_INFO";
    $logDataH{ 0x01AB } = "ISCSI_SET_CHAP";
    $logDataH{ 0x01AC } = "SERVER_LOGGED_IN_OP";
    $logDataH{ 0x01AD } = "ISCSI_GENERIC";
    $logDataH{ 0x01AE } = "SET_GEO_LOCATION";
    $logDataH{ 0x01AF } = "NO_HS_LOCATION_CODE_MATCH";
    $logDataH{ 0x01B0 } = "CLEAR_GEO_LOCATION";
    $logDataH{ 0x01B1 } = "ASWAP_STATE";
    $logDataH{ 0x01B2 } = "SCSI_TIMEOUT";
    $logDataH{ 0x01B3 } = "PDATA_CREATE";
    $logDataH{ 0x01B4 } = "PDATA_REMOVE";
    $logDataH{ 0x01B5 } = "PDATA_WRITE";
    $logDataH{ 0x01B6 } = "NO_CONFIGURATION";
    $logDataH{ 0x01B7 } = "RB_FAILBACK_REINIT_DRIVE";
    $logDataH{ 0x01B8 } = "RB_FAILBACK_CTLR_MISMATCH";
    $logDataH{ 0x01B9 } = "AUTO_FAILBACK";
    $logDataH{ 0x01BA } = "WRONG_SLOT";
    $logDataH{ 0x01BB } = "BAD_CHASIS";
    $logDataH{ 0x01BC } = "ICL_PORT_EVENT";
    $logDataH{ 0x01BD } = "ISNS_CHANGED";
    $logDataH{ 0x01BE } = "PRES_EVENT";
    $logDataH{ 0x01BF } = "PRES_CHANGE";
    $logDataH{ 0x01C0 } = "ERROR_GENERIC";
    $logDataH{ 0x01C1 } = "WARN_GENERIC";
    $logDataH{ 0x01C2 } = "INFO_GENERIC";
    $logDataH{ 0x01C3 } = "VDISK_PR_CLR";
    $logDataH{ 0x01C4 } = "APOOL_CHANGE";
    $logDataH{ 0x01C5 } = "APOOL_CHANGE_I";
    $logDataH{ 0x01C6 } = "APOOL_CHANGE_W";
    $logDataH{ 0x01C7 } = "APOOL_CHANGE_E";
    $logDataH{ 0x01C8 } = "APOOL_CHANGE_D";
    $logDataH{ 0x01C9 } = "ISP_FATAL";
    $logDataH{ 0x01CA } = "COPY_LABEL";
    $logDataH{ 0x01CB } = "SPOOL_CHANGE_I";
    $logDataH{ 0x01CC } = "SPOOL_CHANGE_W";
    $logDataH{ 0x01CD } = "SPOOL_CHANGE_E";
    $logDataH{ 0x01CE } = "SPOOL_CHANGE_D";
    $logDataH{ 0x01CF } = "DRIVE_DELAY";
    $logDataH{ 0x01D0 } = "DELAY_INOP";
    $logDataH{ 0x01D2 } = "ISE_ELEM_CHANGE";
    $logDataH{ 0x01D3 } = "RAID_INOPERATIVE";

    # Add these hashes to the main hash
    $AllHashes{LOG_ID} = \%logIdH;
    $AllHashes{LOG_DATA} = \%logDataH;





    #====================================================================
    #                      X1 Packets
    #====================================================================
    my %x1IdH;
    my %x1DataH;

    #
    # X1 id description table
    #							 
    $x1IdH{ 99 } = "X1  Start";
    $x1IdH{ 0 }  = "X1  Good";

    #
    # X1 data description table
    #							 
    $x1DataH{ 0x10 } = "GET_DLINK";
    $x1DataH{ 0x11 } = "GET_DLOCK_INFO";
    $x1DataH{ 0x12 } = "GET_SU_LIST";
    $x1DataH{ 0x13 } = "GET_SU_LUN_INFO";
    $x1DataH{ 0x20 } = "GET_ENVIRON";
    $x1DataH{ 0x21 } = "GET_PMAP";
    $x1DataH{ 0x22 } = "GET_RMAP";
    $x1DataH{ 0x23 } = "GET_VMAP";
    $x1DataH{ 0x24 } = "GET_HMAP";
    $x1DataH{ 0x25 } = "GET_PSTATS";
    $x1DataH{ 0x26 } = "GET_BSTATS";
    $x1DataH{ 0x27 } = "GET_RSTATS";
    $x1DataH{ 0x28 } = "GET_VSTATS";
    $x1DataH{ 0x29 } = "GET_PINFO";
    $x1DataH{ 0x2A } = "GET_RINFO";
    $x1DataH{ 0x2B } = "GET_VINFO";
    $x1DataH{ 0x2C } = "GET_HINFO";
    $x1DataH{ 0x2D } = "GET_ISALIVE";
    $x1DataH{ 0x2F } = "GET_PSTATE";
    $x1DataH{ 0x30 } = "GET_STATES";
    $x1DataH{ 0x31 } = "GET_DNAME";
    $x1DataH{ 0x32 } = "GET_CPU_LOADS";
    $x1DataH{ 0x35 } = "GET_FREE";
    $x1DataH{ 0x38 } = "WHO_INCOPY";
    $x1DataH{ 0x39 } = "GET_COPY_INFO";
    $x1DataH{ 0x3A } = "SERVER_MAP";
    $x1DataH{ 0x3B } = "GET_MASK_INFO";
    $x1DataH{ 0x3C } = "GET_LUNS_INFO";
    $x1DataH{ 0x3D } = "GET_COPY_STATUS";
    $x1DataH{ 0x3E } = "GET_MIRROR_STATUS";
    $x1DataH{ 0x40 } = "PCHANGED";
    $x1DataH{ 0x41 } = "RCHANGED";
    $x1DataH{ 0x42 } = "VCHANGED";
    $x1DataH{ 0x43 } = "HCHANGED";
    $x1DataH{ 0x44 } = "GET_CONFIG";
    $x1DataH{ 0x45 } = "VCG_CHANGED";
    $x1DataH{ 0x46 } = "ACHANGED";
    $x1DataH{ 0x47 } = "ZCHANGED";
    $x1DataH{ 0x48 } = "READ_MEM";
    $x1DataH{ 0x49 } = "READ_DPRAM";
    $x1DataH{ 0x4A } = "VLINKED_TO";
    $x1DataH{ 0x4B } = "GET_PERF";
    $x1DataH{ 0x4C } = "ACCT_SET";
    $x1DataH{ 0x4D } = "ACCT_GET";
    $x1DataH{ 0x4E } = "ACCT_SELECT";
    $x1DataH{ 0x4F } = "ACCT_LOGIN";
    $x1DataH{ 0x51 } = "SET_LOG";
    $x1DataH{ 0x58 } = "CONFIG_VDISK";
    $x1DataH{ 0x59 } = "CONFIG_HAB";
    $x1DataH{ 0x5A } = "PROBE";
    $x1DataH{ 0x5C } = "DISCONNECT";
    $x1DataH{ 0x5D } = "LOG_ENTRY";
    $x1DataH{ 0x5E } = "SECURE_LOGIN";
    $x1DataH{ 0x5F } = "GET_MS";
    $x1DataH{ 0x60 } = "GET_VCG_INFO";
    $x1DataH{ 0x61 } = "GET_VERSION_INFO";
    $x1DataH{ 0x62 } = "GET_BAY_MAP";
    $x1DataH{ 0x63 } = "GET_BAY_INFO";
    $x1DataH{ 0x64 } = "GET_VPORT_MAP";
    $x1DataH{ 0x65 } = "GET_VPORT_INFO";
    $x1DataH{ 0x66 } = "GET_ELEC_SIG_INFO";
    $x1DataH{ 0x67 } = "GET_BE_LOOP_INFO";
    $x1DataH{ 0x68 } = "GET_WORKSET_INFO";
    $x1DataH{ 0x69 } = "UNUSED1";
    $x1DataH{ 0x6A } = "UNUSED2";
    $x1DataH{ 0x6B } = "PUT_DEV_CONFIG";
    $x1DataH{ 0x6C } = "GET_DEV_CONFIG";
    $x1DataH{ 0x6D } = "GET_SERVER_STATS";
    $x1DataH{ 0x6E } = "GET_HAB_STATS";
    $x1DataH{ 0x6F } = "GET_DEFRAG_STATUS";
    $x1DataH{ 0x70 } = "GET_RESYNC_INFO";
    $x1DataH{ 0x71 } = "RESYNC_CONTROL";
    $x1DataH{ 0x72 } = "LICENSE_CONFIG";
    $x1DataH{ 0x73 } = "GET_MIRROR_IO_STATUS";
    $x1DataH{ 0xBF } = "BIGFOOT_CMD";
    $x1DataH{ 0x9A } = "REPLY_SU_CNT";
    $x1DataH{ 0x9B } = "REPLY_SU_ITEM";
    $x1DataH{ 0x9C } = "REPLY_LUN_CNT";
    $x1DataH{ 0x9D } = "REPLY_LUN_ITEM";
    
    # Add these hashes to the main hash
    $AllHashes{X1_ID} = \%x1IdH;
    $AllHashes{X1_DATA} = \%x1DataH;




    #====================================================================
    #                      X1 VDisk Cfg Packets
    #====================================================================
    my %x1VdcIdH;
    my %x1VdcDataH;

    #
    # X1 VDC id description table
    #							 
    $x1VdcIdH{ 99 } = "X1VDC Start";
    $x1VdcIdH{ 0 }  = "X1VDC Good";

    #
    # X1 VDC data description table
    #							 
    $x1VdcDataH{ 0x01 } = "DELETE";
    $x1VdcDataH{ 0x10 } = "ADD_RAID_0";
    $x1VdcDataH{ 0x11 } = "ADD_RAID_5";
    $x1VdcDataH{ 0x12 } = "ADD_RAID_10";
    $x1VdcDataH{ 0x20 } = "EXP_RAID_0";
    $x1VdcDataH{ 0x21 } = "EXP_RAID_5";
    $x1VdcDataH{ 0x22 } = "EXP_RAID_10";
    $x1VdcDataH{ 0x80 } = "VDISK_MOVE";
    $x1VdcDataH{ 0x81 } = "VDISK_COPY";
    $x1VdcDataH{ 0x82 } = "VDISK_SWAP";
    $x1VdcDataH{ 0x83 } = "VDISK_MIRROR";
    $x1VdcDataH{ 0x84 } = "VDISK_BREAK_MIRROR";
    $x1VdcDataH{ 0x85 } = "VDISK_COPY_PAUSE";
    $x1VdcDataH{ 0x86 } = "VDISK_COPY_RESUME";
    $x1VdcDataH{ 0x87 } = "VDISK_COPY_ABORT";
    $x1VdcDataH{ 0x90 } = "SET_ATTRIBUTE";
    $x1VdcDataH{ 0x91 } = "SET_LOCK";
    $x1VdcDataH{ 0x92 } = "UNUSED1";
    $x1VdcDataH{ 0x99 } = "ERASE_VDISK";
    $x1VdcDataH{ 0xA0 } = "SET_SERVER_NAME";
    $x1VdcDataH{ 0xA1 } = "SET_MASK";
    $x1VdcDataH{ 0xA2 } = "SET_LUN";
    $x1VdcDataH{ 0xA3 } = "SET_DEFMODE";
    $x1VdcDataH{ 0xA4 } = "VDISK_SET_NAME";
    $x1VdcDataH{ 0xA5 } = "MAG_RSVD_1";
    $x1VdcDataH{ 0xA6 } = "VDISK_SET_CACHE_OFF";
    $x1VdcDataH{ 0xA7 } = "SELECT_HAB_FOR_SERVER";
    $x1VdcDataH{ 0xA8 } = "VLINK_BREAK";
    $x1VdcDataH{ 0xA9 } = "VLINK_CREATE";
    $x1VdcDataH{ 0xAA } = "VDISK_SET_CACHE_ON";
    $x1VdcDataH{ 0xAB } = "SELECT_TARGET";
    $x1VdcDataH{ 0xAC } = "ASSIGN_VBLOCK";
    $x1VdcDataH{ 0xAD } = "SET_WORKSET_NAME";
    $x1VdcDataH{ 0xAE } = "SET_DEFAULT_VPORT";
    $x1VdcDataH{ 0xAF } = "ADD_SERVER_TO_WORKSET";
    $x1VdcDataH{ 0xB0 } = "PDISK_LABEL";
    $x1VdcDataH{ 0xB1 } = "PDISK_SPINDOWN";
    $x1VdcDataH{ 0xB2 } = "PDISK_DEFRAG";
    $x1VdcDataH{ 0xB3 } = "PDISK_SCRUB";
    $x1VdcDataH{ 0xB4 } = "PDISK_FAIL";
    $x1VdcDataH{ 0xB5 } = "PDISK_UNFAIL";
    $x1VdcDataH{ 0xB6 } = "PDISK_BEACON";
    $x1VdcDataH{ 0xB7 } = "PDISK_DELETE";
    $x1VdcDataH{ 0xB8 } = "LOG_ACKNOWLEDGE";
    $x1VdcDataH{ 0xB9 } = "DELETE_SERVER";
    $x1VdcDataH{ 0xBA } = "OP_SET_PRIORITY";
    $x1VdcDataH{ 0xBB } = "RAID_RECOVER";
    $x1VdcDataH{ 0xBC } = "SET_GLOBAL_CACHE_ON";
    $x1VdcDataH{ 0xBD } = "SET_GLOBAL_CACHE_OFF";
    $x1VdcDataH{ 0xBE } = "PDISK_AUTOFAILBACK";
    
    # Add these hashes to the main hash
    $AllHashes{X1_VDC_ID} = \%x1VdcIdH;
    $AllHashes{X1_VDC_DATA} = \%x1VdcDataH;




    #====================================================================
    #                      X1 BF Passthru Packets
    #====================================================================
    my %x1BFIdH;

    #
    # X1 BF id description table
    #							 
    $x1BFIdH{ 99 } = "X1BF Start";
    $x1BFIdH{ 0 }  = "X1BF Good";

    #
    # X1 BF data description table
    #							 
    # Note: uses same data as PI_DATA
    
    # Add these hashes to the main hash
    $AllHashes{X1_BF_ID} = \%x1BFIdH;




    #====================================================================
    #                      RM Data Codes
    #====================================================================
    my %rmIdH;
    my %rmDataH;

    #
    # RM id description table
    #
    $rmIdH{ 0 } = "RM Trace";

    #
    # RM data description table
    #							 
    $rmDataH{ 0x00000001 } = "INIT_SHUTDOWN";
    $rmDataH{ 0x00000002 } = "INIT";
    $rmDataH{ 0x00000003 } = "MOVE_TARGET";
    $rmDataH{ 0x00000004 } = "FAIL_CONTROLLER";
    $rmDataH{ 0x00000005 } = "UNFAIL_CONTROLLER";
    $rmDataH{ 0x00000006 } = "FAIL_INTERFACE";
    $rmDataH{ 0x00000007 } = "UNFAIL_INTERFACE";
    $rmDataH{ 0x00000008 } = "RESTORE_INTERFACE";
    $rmDataH{ 0x00000009 } = "BUILD_TARGETS";
    $rmDataH{ 0x00001000 } = "CONFIG_INIT";
    $rmDataH{ 0x00001001 } = "CONFIG_BUILD";
    $rmDataH{ 0x00001002 } = "CONFIG_READ";
    $rmDataH{ 0x00001003 } = "CONFIG_WRITE";
    $rmDataH{ 0x00001004 } = "CONFIG_RESTORE";
    $rmDataH{ 0x00001005 } = "CONFIG_CHECKPOINT";
    $rmDataH{ 0x00001006 } = "CONFIG_CURRENT";
    $rmDataH{ 0x00002000 } = "REALLOC_TASK";
    $rmDataH{ 0x00002001 } = "REALLOC_RELOCATE_TGT";
    $rmDataH{ 0x00002002 } = "REALLOC_PROC_FAILED";
    $rmDataH{ 0x00002003 } = "REALLOC_MV_TGT_FROM_INT";
    $rmDataH{ 0x00002004 } = "REALLOC_REDIST_TGTS";
    $rmDataH{ 0x00002005 } = "REALLOC_REDIST_TGT";
    $rmDataH{ 0x00002006 } = "REALLOC_UPD_INT_STATUS";
    $rmDataH{ 0x00003000 } = "HS_REQUEST_SPARE";
    $rmDataH{ 0x00003001 } = "HS_PROC_HS_REQUEST";
    $rmDataH{ 0x00003002 } = "HS_LOCATE_HS";
    $rmDataH{ 0x00003003 } = "HS_CHECK_HS";
    $rmDataH{ 0x00003004 } = "HS_BUILD_HS_LIST";
    $rmDataH{ 0x00004000 } = "MIRROR_RETRIEVE_MP";
    $rmDataH{ 0x00004001 } = "MIRROR_CHECK_MP";
    $rmDataH{ 0x00004002 } = "MIRROR_SET_MPS";
    $rmDataH{ 0x00004003 } = "MIRROR_DLT_FROM_MC";
    $rmDataH{ 0x00004004 } = "MIRROR_INSERT_MP";
    $rmDataH{ 0x00004005 } = "MIRROR_ADD_MP";
    $rmDataH{ 0x00004006 } = "MIRROR_SET_MP";
    $rmDataH{ 0x00005000 } = "MISC_GET_TGT_CACHE_STATUS";
    $rmDataH{ 0x00005001 } = "MISC_MOVE_TARGET";
    $rmDataH{ 0x00005002 } = "MISC_CLEANUP";
    $rmDataH{ 0x00005003 } = "MISC_BUILD_TARGET";
    $rmDataH{ 0x00005004 } = "MISC_GET_MP";
    
    # Add these hashes to the main hash
    $AllHashes{RM_ID} = \%rmIdH;
    $AllHashes{RM_DATA} = \%rmDataH;


    #====================================================================
    #                      SG Data Codes (Signals)
    #====================================================================
    my %sgIdH;
    my %sgDataH;

    #
    # SG id description table
    #
    $sgIdH{  0 } = 'Signal';
    $sgIdH{ 99 } = 'PAM Hbeat';

    #
    # SG data description table
    #							 
    $sgDataH{ 0 } = 'Signal';
    
    # Add these hashes to the main hash
    $AllHashes{SG_ID} = \%sgIdH;
    $AllHashes{SG_DATA} = \%sgDataH;

    #====================================================================

    return %AllHashes;
}

##############################################################################

1;
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#

