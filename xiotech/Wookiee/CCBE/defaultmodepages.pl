#!/bin/perl -w
# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Chris Nigbur
#
# Purpose:
#   Regression test script
##############################################################################
use Getopt::Std;
use FileHandle;
use Text::Abbrev;
use Cwd;

use XIOTech::cmdMgr;
use XIOTech::cmUtils;
use XIOTech::constants;
use XIOTech::xiotechPackets;
use XIOTech::logMgr;

my $rc = 1;
my $obj;
my $currentPort = 3000;

##############################################################################
# Name:     displayError
#
# Desc:     Displays an error message followed by the status and
#           error codes from a command response.
#
# Input:    message and command response hash.
##############################################################################
sub displayError
{
    my ($msg, %rsp) = @_;

    print $msg . "\n";
    print "\n";
    printf "Status Code: 0x%x\n", $rsp{STATUS};
    printf "Error Code:  0x%x\n", $rsp{ERROR_CODE};

    if (defined($rsp{MESSAGE}))
    {
        print "\n";
        printf "Message:\n";
        print $rsp{MESSAGE};
    }
}




sub test01
{
    my ($wwnHi, $wwnLo, $lun) = @_;

    my @deviceID;
    my $cdb;
    my $str;
    my %rsp;
    my $newdata;
    my $i;
    my $j;
    my $pageCode;
    my $pageLen;
    my $byteData;
    my $pageCode1;
    my $pageLen1;
    my $byteData1;
    my $rc = 0;
    
    $deviceID[0]{WWN_LO} = $wwnLo;
    $deviceID[0]{WWN_HI} = $wwnHi;
    $deviceID[0]{PD_LUN} = $lun;

    $cdb = pack("H32", "5A00BF00000000080000000000000000");
    %rsp = $obj->scsiCmd($cdb, undef, @deviceID);

    (
    $len,
    $mediumtype,
    $devspecparm,
    $rsvd,
    $rsvd,
    $blockdescriptorlength
    ) = unpack("nccccs", substr($rsp{DATA}, 0));

    (
    $numblocks,
    $blocklength
    ) = unpack("ll", substr($rsp{DATA}, 8));

    $str = $obj->FormatDataString($rsp{DATA}, 0, "BYTE", undef, $len+2);
    print "#################################################################\n";
    print "Default Mode Data: \n";
    print $str;
    print "\n";

    $newdata = pack("lss", 0, 0, $blockdescriptorlength);
    $newdata .= pack("ll", 0, $blocklength);

    for ($i = 16; $i < $len + 2; )
    {
        ($pageCode, $pageLen) = unpack("cc", substr($rsp{DATA}, $i));

        $pageCode &= 0x7F;

        $newdata .= pack("c", $pageCode);

        $i = $i + 1;

        for ($j = 0; $j < $pageLen; $j++)
        {
            ($byteData) = unpack("c", substr($rsp{DATA}, $i));

            $i = $i + 1;

            $newdata .= pack("c", $byteData);
        }
    }

    $str = $obj->FormatDataString($newdata, 0, "BYTE", undef, $len+2);
    print "Mode Select Data: \n";
    print $str;
    print "\n";

    $cdb = pack("H14", "55110000000000");
    $cdb .= pack("n", $len + 2);
    $cdb .= pack("H14", "00000000000000");

    %rsp = $obj->scsiCmd($cdb, $newdata, @deviceID);

    $cdb = pack("H32", "5A003F00000000080000000000000000");
    %rsp = $obj->scsiCmd($cdb, undef, @deviceID);

    (
    $len,
    $mediumtype,
    $devspecparm,
    $rsvd,
    $rsvd,
    $blockdescriptorlength
    ) = unpack("nccccs", substr($rsp{DATA}, 0));

    (
    $numblocks,
    $blocklength
    ) = unpack("ll", substr($rsp{DATA}, 8));

    $str = $obj->FormatDataString($rsp{DATA}, 0, "BYTE", undef, $len+2);
    print "Current Mode Data: \n";
    print $str;
    print "#################################################################\n";

    for ($i = 16; $i < $len + 2; )
    {
        ($pageCode, $pageLen) = unpack("cc", substr($rsp{DATA}, $i));
        ($pageCode1, $pageLen1) = unpack("cc", substr($newdata, $i));

        $pageCode &= 0x7F;

        if ($pageCode != $pageCode1)
        {
            $rc = $i;
            last;
        }

        $i = $i + 1;

        for ($j = 0; $j < $pageLen; $j++)
        {
            ($byteData) = unpack("c", substr($rsp{DATA}, $i));
            ($byteData1) = unpack("c", substr($newdata, $i));

            if ($byteData != $byteData1)
            {
                $rc = $i;
                last;
            }

            $i = $i + 1;
        }

        if ($rc != 0)
        {
            last;
        }
    }

    if ($rc != 0)
    {
        printf "####  SHIT  ####  rc: 0x%x\n", $rc;
    }
    else
    {
        print "####  NO PROBLEM MAAN  ####\n";
    }

    print "#################################################################\n";
    print "\n";
    print "\n";
}


##############################################################################
# Test the physical disk operations
##############################################################################
sub test
{
    print "\n";
    print "Clearing physical disk mode pages...\n";
    print "\n";

    my $rc = 1;
    my %pdisks;
    my %i;

    %pdisks = $obj->physicalDisks();
    
    if (!%pdisks)
    {
        print "ERROR: Failed to receive a response from physicalDisks.\n";
        $rc = 0;
        return $rc;
    }

    if ($pdisks{STATUS} != PI_GOOD)
    {
        my $msg = "ERROR: Failed to retrieve physical disks.";
        displayError($msg, %pdisks);
        $rc = 0;
        return $rc;
    }

#    my $i = 0;
    for (my $i = 0; $i < $pdisks{COUNT}; $i++)
    {
        my $wwnHI = $pdisks{PDISKS}[$i]{WWN_HI};
        my $wwnLO = $pdisks{PDISKS}[$i]{WWN_LO};
        my $lun = $pdisks{PDISKS}[$i]{PD_LUN};

        test01($wwnHI, $wwnLO, $lun);
    }

    print "\n";
    print "Completed clearing of physical disk mode pages...\n";
    print "\n";

    return $rc;
}

##############################################################################
# Test script start
##############################################################################
my $logFile = XIOTech::logMgr::logStart("pdiskcapacity", "TS");

# Run in file mode or interactive mode based upon if a file is 
# passed on the cmdline or not.
my $connectTo = 0;
if (@ARGV>=1)
{
    # Get the first command line parm and see if an IP address
    ($inf) = shift @ARGV;

    # If its an IP address, we will connect to it automatically
    if ($inf =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/)
    {
        $connectTo = $inf;
    }
}

# Autoflush all stdout writes
STDOUT->autoflush(1);

print "\n";
print "==============================================\n";
print "Welcome to the BIGFOOT regression test script!\n";
print "Logging information to:\n";
print "  $logFile\n";
print "==============================================\n";

# Create a command manager to use
# Only parameter is where you want the errors to be reported to
$obj = XIOTech::cmdMgr->new(\*STDOUT);

# connect to initial system if requested on the command line
if($connectTo)
{
    print "Attempting to log into contoller ($connectTo).\n";
    if ($obj->login($connectTo, $currentPort))
    {
        print "login packet successful\n";
    }
    else
    {
        print "Login unsuccessful\n";
        $rc = 0;
    }
}
else
{
    print "Invalid or missing IP address on command line.\n";
    $rc = 0;
}

if ($rc)
{
    $rc = test();
}

if ($rc) # last
{
    $rc = $obj->logout();
}

print "\n";
print "====================================================\n";
print "Physical Disk Capacity Listing...\n";

if ($rc)
{
    print "  TESTS COMPLETED SUCCESSFULLY\n";
}
else
{
    print "  TESTS FAILED\n";
}
print "====================================================\n";

##############################################################################
