#!/usr/bin/perl -w
#====================================================================
#
# FILE NAME:    SCSIRdWr.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         3/22/05
#
# DESCRIPTION:  Read drive data to a file or write drive data from a file.
#
#====================================================================

use strict;
use IO::Handle;
use XIOTech::cmdMgr;
use XIOTech::cmUtils;
use XIOTech::xiotechPackets;

STDOUT->autoflush(1);

unless (@ARGV == 6) 
{ 
    print "\nUsage: SCSIRdWr.pl ip-address read|write pid file ".
    "starting-LBA blocks\n\n";
    print "NOTE: \"read\" means to READ data FROM a PDisk.\n";
    print "      \"write\" means to WRITE data TO a PDisk.\n\n";
    exit 1;
}

my ($ip, $mode, $pid, $file, $lba, $blocks) = @ARGV;
my $rc;

#####################################################################
#
# Start of mainline
#
#####################################################################

#
# Massage the input paramters --
# convert from hex if necessary
#
$lba = oct $lba if ($lba =~ /^0x/i);
$blocks = oct $blocks if ($blocks =~ /^0x/i);

#
# Check the mode parameter
#
if ($mode !~ /^read$|^write$/i)
{
    print "\nERROR: You must specify either READ or WRITE.\n\n";
    exit 1;
}

#
# Make sure we can read or write the requested output file
#
if ($mode =~ /write/i)
{
    open F, "$file" or die "\nERROR: Can't read $file...\n\n";
}
if($mode =~ /read/i)
{
    if (-r $file)
    {
        print "\n$file already exists. Overwrite? Y/[N] ";
        my $ans = <STDIN>;

        if ($ans !~ /^Y/i) {
            print "\nERROR: Choose a new file name...\n\n";
            exit -1;
        }
    }

    open F, ">$file" or die "\nERROR: Can't write to $file...\n\n";
}
binmode F;

#
# Create a new command manager
#
my $cmdMgr = XIOTech::cmdMgr->new(\*STDOUT);

#
# login to the requested controller
#
$rc = $cmdMgr->login($ip, 3100);
if(! $rc) {
    print "\nERROR: Login to $ip failed...\n\n";
    exit -1;
}

#
# Retrieve WWN/LUN from type/pid
#
my %pdisks;
my @deviceID;
%pdisks = $cmdMgr->physicalDiskInfo($pid);
if (%pdisks) {
    if ($pdisks{STATUS} == PI_GOOD) {
        $deviceID[0]{WWN_LO} = $pdisks{WWN_LO};
        $deviceID[0]{WWN_HI} = $pdisks{WWN_HI};
        $deviceID[0]{PD_LUN} = $pdisks{PD_LUN};
        printf "\nDrive WWN: %08X%08X LUN: %d\n\n", 
        $pdisks{WWN_LO}, $pdisks{WWN_HI}, $pdisks{PD_LUN};
    }
    else {
        my $msg = "\nERROR: Unable to retrieve pdisk/bay info.";
        displayError($msg, %pdisks);
        exit -1;
    }
}
else {
    print "\nERROR: Could not retrieve pdisk WWN.\n\n";
    exit -1;
}

#
# Loop here reading in 4 block chunks
#
if($mode =~ /read/i)
{
    my $read10CDB;
    my $blksThisTime;
    while($blocks) 
    {
        if ($blocks > 4) 
        {
            $blksThisTime = 4;
        }
        else 
        {
            $blksThisTime = $blocks;
        }
        $blocks -= $blksThisTime;

        $read10CDB = "2800" . sprintf("%08X", $lba) . "0000" . 
            sprintf("%02X", $blksThisTime) ."00";
        print "Read10 CDB: $read10CDB\n";

        my $cdb = AsciiHexToBin($read10CDB, "byte");
        if(!defined $cdb) 
        {
            print "\nERROR: Invalid CDB format.\n\n";
            exit -1;
        }    

        my %rsp = $cmdMgr->scsiCmd($cdb, undef, @deviceID);
        if (%rsp)
        {
            #
            # If successful, write out the output data
            #
            if ($rsp{STATUS} == PI_GOOD)
            {
                if ($blksThisTime < 4)
                {
                    $rsp{DATA} = substr $rsp{DATA}, 0, $blksThisTime * 512;
                }
                print F $rsp{DATA};
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
            print "\nERROR: Could not process SCSI command.\n\n";
            exit -1;
        }
        
        $lba += $blksThisTime;
    }

    print "\nOutput data written to $file\n\n";
}
else # write from file
#
# Loop here writing in 4 block chunks
#
{
    my $write10CDB;
    my $blksThisTime;
    my $fileBlocksLeft = int((-s F)/512);
    my $lesser;
    my $fileData;
    
    while($blocks and $fileBlocksLeft) 
    {
        $lesser = $blocks < $fileBlocksLeft ? $blocks : $fileBlocksLeft;
        if ($lesser > 4) 
        {
            $blksThisTime = 4;
        }
        else 
        {
            $blksThisTime = $lesser;
        }
        
        $blocks -= $blksThisTime;
        $fileBlocksLeft -= $blksThisTime;

        $write10CDB = "2A00" . sprintf("%08X", $lba) . "0000" . 
            sprintf("%02X", $blksThisTime) ."00";
        print "Write10 CDB: $write10CDB\n";

        my $cdb = AsciiHexToBin($write10CDB, "byte");
        if(!defined $cdb) 
        {
            print "\nERROR: Invalid CDB format.\n\n";
            exit -1;
        }    

        # Read the data from the file
        $rc = read(F, $fileData, $blksThisTime * 512);
        if($rc == 0) 
        {
            print "\nERROR: File read failed ($!).\n\n";
            exit -1;
        }    
        
        my %rsp = $cmdMgr->scsiCmd($cdb, $fileData, @deviceID);
        if (%rsp)
        {
            #
            # If failure, display sense data
            #
            if ($rsp{STATUS} != PI_GOOD)
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
            print "\nERROR: Could not process SCSI command.\n\n";
            exit -1;
        }
        
        $lba += $blksThisTime;
    }
    if ($blocks)
    {
        print "\nWARNING: Ran out of data to write before reaching LBA count.\n";
    }
    elsif ($fileBlocksLeft)
    {
        print "\nWARNING: Reached LBA count before writing all of the file data.\n";
    } 

    print "\nInput file $file written to disk\n\n";
}

$cmdMgr->logout();
close F;
exit 0;


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
#####################################################################
#####################################################################
#####################################################################
#########################################
