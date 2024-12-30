#!/usr/bin/perl -w
# $Id: BldCmdCodeHashes.pl 139516 2010-05-03 19:11:35Z tom_marlin $
#====================================================================
#
# FILE NAME:    BldCmdCodeHashes.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         11/21/02
#
# DESCRIPTION:  Builds up Xiotech/CmdCodeHashes.pm for use by
#               TracDec.pl etc.
#
#====================================================================

#
# Open up source and destination files
#
$fn = "CmdCodeHashes.tpl";
open FIN, $fn or die "\nAbort: Can't open $fn...\n";

$fn = ">XIOTech/CmdCodeHashes.pm";
open FOUT, $fn or die "\nAbort: Can't open $fn...\n";


#####################################################################
###                                                               ###
###                       MRP Data                                ###
###                                                               ###
#####################################################################

#
# Search to first insert point
#
$fin = "CmdCodeHashes.tpl";
while (<FIN>) {
    if (/###MRP_DATA###/) {
        $print = 1;
        last;
    }
    print FOUT;
}

$print or die "\nAbort: Can't find \"###MRP_DATA###\" in $fin...\n";

#
# Extract the MRP command codes
#
$fn = "../Shared/Inc/MR_Defs.h";
$print = 0;
open F, $fn or die "\nAbort: Can't open $fn...\n";
print "Searching $fn...\n";

while (<F>) {
    if (/MREBFUNCBASE/) {
        $print = 1;
        last;
    }
}

$print or die "\nAbort: Can't find \"MREBFUNCBASE\" in $fn...\n";

while (<F>) {
    if (/MRP return codes/) {
        last;
    }

    if (/^#define\s+MR/) {
        s/^#define\s+MR//;
        ($name,$val,$theRest) = split;
        chomp $val;
        next if $name =~ /funcmax/i;
        print FOUT "    \$mrpDataH{ $val } = \"$name\";\n";
    }
}

close F;


#####################################################################
###                                                               ###
###                       PI Data                                 ###
###                                                               ###
#####################################################################

#
# Search to next insert point
#
while (<FIN>) {
    if (/###PI_DATA###/) {
        $print = 1;
        last;
    }
    print FOUT;
}

$print or die "\nAbort: Can't find \"###PI_DATA###\" in $fin...\n";

#
# Now extract the PI command codes
#
$fn = "../CCB/Inc/PacketInterface.h";
$print = 0;
open F, $fn or die "\nAbort: Can't open $fn...\n";
print "Searching $fn...\n";

while (<F>) {
    if (/Packet command codes/) {
        $print = 1;
        last;
    }
}

$print or die "\nAbort: Can't find \"Packet command codes\" in $fn...\n";

$theRest = "";
while (<F>) {

    if (/Generic Object Packets/) {
        last;
    }

    if (/^#define\s+PI_/) {
        s/^#define\s+PI_//;
        s/_CMD\s*/ /;
        ($name,$val,$theRest) = split;
        chomp $val;
        print FOUT "    \$pktDataH{ $val } = \"$name\";\n";
    }
}

close F;


#####################################################################
###                                                               ###
###                       IPC Data                                ###
###                                                               ###
#####################################################################

#
# Search to next insert point
#
while (<FIN>) {
    if (/###IPC_DATA###/) {
        $print = 1;
        last;
    }
    print FOUT;
}

$print or die "\nAbort: Can't find \"###IPC_DATA###\" in $fin...\n";

#
# Now extract the IPC command codes
#
$fn = "../CCB/Inc/ipc_packets.h";
$print = 0;
open F, $fn or die "\nAbort: Can't open $fn...\n";
print "Searching $fn...\n";

while (<F>) {
    if (/IPC command codes/) {
        $print = 1;
        last;
    }
}

$print or die "\nAbort: Can't find \"IPC command codes\" in $fn...\n";

$theRest = "";
while (<F>) {

    if (/Structures for all/) {
        last;
    }

    if (/^#define\s+PACKET_IPC_/) {
        s/^#define\s+PACKET_IPC_//;
        ($name,$val,$theRest) = split;
        chomp $val;
        print FOUT "    \$ipcDataH{ $val } = \"$name\";\n";
    }
}

close F;


#####################################################################
###                                                               ###
###                       LOG Data                                ###
###                                                               ###
#####################################################################

#
# Search to next insert point
#
while (<FIN>) {
    if (/###LOG_DATA###/) {
        $print = 1;
        last;
    }
    print FOUT;
}

$print or die "\nAbort: Can't find \"###LOG_DATA###\" in $fin...\n";

#
# Now extract the log command codes
#
$fn = "../Shared/Inc/LOG_Defs.h";
$print = 0;
open F, $fn or die "\nAbort: Can't open $fn...\n";
print "Searching $fn...\n";

while (<F>) {
    if (/LOG_EVENT_CODES/) {
        $print = 1;
        last;
    }
}

$print or die "\nAbort: Can't find \"LOG_EVENT_CODES\" in $fn...\n";

$theRest = "";
my $type;
while (<F>) {
    if (/Public defines - macros/) {
        last;
    }

    if (/^#define\s+LOG_/) {
        s/^#define\s+LOG_//;
        ($name,$theRest) = split;

        # $theRest contains something of the form "LOG_Debug(0x0000)"
        # but with LOG_Error, LOG_Warning, and LOG_Info (etc)

        m/LOG_.*?\)/ =~ $theRest;
        $val = $&;
        chop $val;
        ($type, $val) = split /\(/, $val;
        $val = oct $val;

        $val = sprintf "0x%04X", $val;
        print FOUT "    \$logDataH{ $val } = \"$name\";\n";
    }
}

close F;


#####################################################################
###                                                               ###
###                       X1 Data                                 ###
###                                                               ###
#####################################################################

#
# Search to next insert point
#
while (<FIN>) {
    if (/###X1_DATA###/) {
        $print = 1;
        last;
    }
    print FOUT;
}

$print or die "\nAbort: Can't find \"###X1_DATA###\" in $fin...\n";

#
# Now extract the x1 command codes
#
$fn = "../CCB/Inc/X1_CmdCodes.h";
$print = 0;
open F, $fn or die "\nAbort: Can't open $fn...\n";
print "Searching $fn...\n";

#while (<F>) {
#    if (/INFO event codes/) {
#        $print = 1;
#        last;
#    }
#}
#
#$print or die "\nAbort: Can't find \"INFO event codes\" in $fn...\n";

$theRest = "";
while (<F>) {

#    if (/Obsoleted events/) {
#        last;
#    }

    if (/^#define\s+X1PKT_/) {
        s/^#define\s+X1PKT_//;
        ($name,$val,$theRest) = split;
        chomp $val;
        print FOUT "    \$x1DataH{ $val } = \"$name\";\n";
    }
}

close F;


#####################################################################
###                                                               ###
###                       X1 VDC Data                             ###
###                                                               ###
#####################################################################

#
# Search to next insert point
#
while (<FIN>) {
    if (/###X1_VDC_DATA###/) {
        $print = 1;
        last;
    }
    print FOUT;
}

$print or die "\nAbort: Can't find \"###X1_VDC_DATA###\" in $fin...\n";

#
# Now extract the x1 vdisk cfg command codes
#
$fn = "../CCB/Inc/X1_CmdCodes.h";
$print = 0;
open F, $fn or die "\nAbort: Can't open $fn...\n";
print "Searching $fn...\n";

#while (<F>) {
#    if (/INFO event codes/) {
#        $print = 1;
#        last;
#    }
#}
#
#$print or die "\nAbort: Can't find \"INFO event codes\" in $fn...\n";

$theRest = "";
while (<F>) {

#    if (/Obsoleted events/) {
#        last;
#    }

    if (/^#define\s+CFG_VDISK_/) {
        s/^#define\s+CFG_VDISK_//;
        ($name,$val,$theRest) = split;
        chomp $val;
        print FOUT "    \$x1VdcDataH{ $val } = \"$name\";\n";
    }
}

close F;


#####################################################################
###                                                               ###
###                        RM Data                                ###
###                                                               ###
#####################################################################

#
# Search to next insert point
#
while (<FIN>) {
    if (/###RM_DATA###/) {
        $print = 1;
        last;
    }
    print FOUT;
}

$print or die "\nAbort: Can't find \"###RM_DATA###\" in $fin...\n";

#
# Now extract the log command codes
#
$fn = "../CCB/Inc/trace.h";
$print = 0;
open F, $fn or die "\nAbort: Can't open $fn...\n";
print "Searching $fn...\n";

while (<F>) {
    if (/RM trace event/) {
        $print = 1;
        last;
    }
}

$print or die "\nAbort: Can't find \"RM trace event\" in $fn...\n";

$theRest = "";
while (<F>) {

    if (/Public function prototypes/) {
        last;
    }

    if (/^#define\s+TRACE_RM_/) {
        s/^#define\s+TRACE_RM_//;
        ($name,$val,$theRest) = split;
        chomp $val;
        print FOUT "    \$rmDataH{ $val } = \"$name\";\n";
    }
}

close F;


#####################################################################
###                                                               ###
###                       Done...                                 ###
###                                                               ###
#####################################################################

#
# Copy out the rest and exit
#
while (<FIN>) {
    print FOUT;
}

close FIN;
close FOUT;

