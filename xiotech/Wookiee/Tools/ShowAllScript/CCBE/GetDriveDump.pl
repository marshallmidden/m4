#!/perl/bin/perl -w
#====================================================================
#
# FILE NAME:    GetDriveDump.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         7/25/02
#
# DESCRIPTION:  Read the last "N" blocks on a drive and write out to 
#               a file.
#
#====================================================================

use lib 'k:/release/bigfoot/m550/ccbe';

use IO::Handle;
use XIOTech::cmdMgr;
use XIOTech::cmUtils;
use XIOTech::xiotechPackets;

STDOUT->autoflush(1);

unless (@ARGV >= 3) { die 
"\nRetrieves a Seagate \"Drive Dump\" (256 blocks starting at Read ".
"Capacity - 0x1000)\n\n".
"Usage: GetDriveDump.pl ip-address drive-pid file [offset [#blocks]]\n".
"       'offset' defaults to 0x1000\n".
"       '#blocks' defaults to 256\n"}

my ($ip, $pid, $file, $offset, $blocks) = @ARGV;

my $rc;

#####################################################################
# Name:     AsciiHexToBin
#
# Desc:     Converts an ASCII hex string to packed binary data
#
# Input:    data - hex string representing the MRP input data
#           format - byte|short|word  (default: word)
#####################################################################
sub AsciiHexToBin
{
    my ($data, $format) = @_;

    $data =~ s/0x//i;

    if (!defined $data) {
        print "AsciiHexToBin: No input data.\n";
        return undef;
    }

    if (!defined $format) {
        $format = "word";
    }
    
    if ($format !~ /^byte$|^short$|^word$/i) {
        print "AsciiHexToBin: format incorrect ($format).\n";
        return undef;
    }

    # setup to parse the input data string
    my $cCnt;
    my $cTpl;

    if ($format =~ /^byte$/i) {
        $cCnt = 2;
        $cTpl = "C";
    }
    elsif ($format =~ /^short$/i) {
        $cCnt = 4;
        $cTpl = "S";
    }
    else { # word
        $cCnt = 8;
        $cTpl = "L";
    }
    
    my @wData;
    my $i;
    my $template = "";
    my $length = length $data;

    if ($length % $cCnt) {
        print "AsciiHexToBin: Input data string length not correct for the\n" .
              " format selected ($format).\n";
        return undef;
    }
    
    # parse the input data string
    for($i=0; $i<$length; $i+=$cCnt) {
        push @wData, oct("0x" . substr $data, 0, $cCnt);
        $data = substr $data, $cCnt;
        $template .= $cTpl;
    }

    $data = pack $template, @wData;
    return $data;
}

#####################################################################
# Name:     displayError
#
# Desc:     Displays an error message followed by the status and
#           error codes from a command response.
#
# Input:    message and command response hash.
#####################################################################
sub displayError
{
    my ($msg, %rsp) = @_;

    print $msg . "\n";
    print "\n";

    printf "Status Code:    0x%02x  ", $rsp{STATUS};
    
    if (defined($rsp{STATUS_MSG}))
    {
        printf " \"%s\"", $rsp{STATUS_MSG};
    }

    print "\n";
    
    printf "Error Code:     0x%02x  ", $rsp{ERROR_CODE};

    if (defined($rsp{ERROR_MSG}))
    {
        printf " \"%s\"", $rsp{ERROR_MSG};
    }

    print "\n";
}

#####################################################################
#
# Start of mainline
#
#####################################################################

#
# Get read capacity offset
#
if(!defined($offset)) {
    $offset = 0x1000;
}
# convert from hex if necessary
elsif ($offset =~ /^0x/i) {
    $offset = oct $offset;
}

#
# Get number of blocks requested
#
if(!defined($blocks)) {
    $blocks = 256;
}
# convert from hex if necessary
elsif ($blocks =~ /^0x/i) {
    $blocks = oct $blocks;
}

#
# Create a new command manager
#
my $cmdMgr = XIOTech::cmdMgr->new(\*STDOUT);

#
# login to the requested controller
#
$rc = $cmdMgr->login($ip, 3000);
if(! $rc) {
    print "\nERROR: Login to $ip failed...\n";
    exit -1;
}

#
# Retrieve WWN/LUN from type/pid
#
my %pdisks;
%pdisks = $cmdMgr->physicalDiskInfo($pid);
if (%pdisks) {
    if ($pdisks{STATUS} == PI_GOOD) {
        $deviceID[0]{WWN_LO} = $pdisks{WWN_LO};
        $deviceID[0]{WWN_HI} = $pdisks{WWN_HI};
        $deviceID[0]{PD_LUN} = $pdisks{PD_LUN};
        printf "\nAccessing Drive WWN: %08X%08X LUN: %d\n", 
        $pdisks{WWN_LO}, $pdisks{WWN_HI}, $pdisks{PD_LUN};
    }
    else {
        my $msg = "\nERROR: Unable to retrieve pdisk/bay info.";
        displayError($msg, %pdisks);
        exit -1;
    }
}
else {
    print "\nERROR: Could not retrieve pdisk WWN.\n";
    exit -1;
}

#
# Issue a read capacity to retrieve the last LBA on the drive
#
my $readCapacityCDB = "25000000000000000000";
my $cdb = AsciiHexToBin($readCapacityCDB, "byte");
my $maxLBA;

if(!defined $cdb) {
    print "\nERROR: Invalid CDB format.\n";
    exit -1;
}    

my %rsp = $cmdMgr->scsiCmd($cdb, undef, @deviceID);
if (%rsp)
{
    #
    # If successful, format the output data
    #
    if ($rsp{STATUS} == PI_GOOD)
    {
        $maxLBA = unpack "N", $rsp{DATA};      
        print "Read Capacity: Max LBA = $maxLBA\n";
    }

    #
    # If failure, display sense data
    #
    else
    {
        my $msg = "\nERROR: SCSI Read Capacity cmd failed.";
        displayError($msg, %rsp);

        printf "Sense Key:      0x%02X\n", $rsp{SENSE_KEY};
        printf "Sense Code:     0x%02X\n", $rsp{ADTL_SENSE_CODE};
        printf "Sense Code Qual:0x%02X\n", $rsp{ADTL_SENSE_CODE_QUAL};
        exit -1;
    }
}
else
{
    print "\nERROR: Could not process SCSI command.\n";
    exit -1;
}

#
# Backup from maxLBA by requested blocks to read
#
my $curLBA = $maxLBA - $offset;
print "Starting to read at LBA: $curLBA\n\n";

#
# Make sure we can write to the requested output file
#
if (-r $file) {
    print "$file already exits. Overwrite? Y/[N] ";
    my $ans = <STDIN>;

    if ($ans !~ /^Y/i) {
        print "\nERROR: Chose a new file name...\n";
        exit -1;
    }
    else {
        print "\n";
    }
}
open OUT, ">$file" or die "\nERROR: Can't write to $file...\n";
binmode OUT;

my $read10CDB;

#
# Loop here reading in 4 block chunks
#
while($blocks) {

    if ($blocks > 4) {
        $blksThisTime = 4;
    }
    else {
        $blksThisTime = $blocks;
    }
    $blocks -= $blksThisTime;

    $read10CDB = "2800" . sprintf("%08X", $curLBA) . "0000" . 
        sprintf("%02X", $blksThisTime) ."00";
    print "Read10 CDB: $read10CDB\n";

    $cdb = AsciiHexToBin($read10CDB, "byte");
    if(!defined $cdb) {
        print "\nERROR: Invalid CDB format.\n";
        exit -1;
    }    

    %rsp = $cmdMgr->scsiCmd($cdb, undef, @deviceID);
    if (%rsp)
    {
        #
        # If successful, write out the output data
        #
        if ($rsp{STATUS} == PI_GOOD)
        {
            print OUT $rsp{DATA};
        }

        #
        # If failure, display sense data
        #
        else
        {
            my $msg = "\nERROR: SCSI Read10 cmd failed.";
            displayError($msg, %rsp);

            printf "Sense Key:      0x%02X\n", $rsp{SENSE_KEY};
            printf "Sense Code:     0x%02X\n", $rsp{ADTL_SENSE_CODE};
            printf "Sense Code Qual:0x%02X\n", $rsp{ADTL_SENSE_CODE_QUAL};
            exit -1;
        }
    }
    else
    {
        print "\nERROR: Could not process SCSI command.\n";
        exit -1;
    }
    
    $curLBA += $blksThisTime;
}

print "\nOutput data written to $file\n\n";
close OUT;
exit 0;


