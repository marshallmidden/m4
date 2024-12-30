#!/bin/perl -w
# $Header$
##############################################################################
# Xiotech
# Copyright (c) 2005  Xiotech
# ======================================================================
# $RCSfile$
# Author:   Jeff Williams
#
# Purpose:
#   Test SBOD drive bays to see if there is an error in any of the
#   connections that would dictate shutting down a test.
#
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
my $rc_check = 1;
my $obj;
my $currentPort = 3100;

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

##############################################################################
# Test the physical disk operations
##############################################################################
sub checkSBOD
{
    print "\n";
    print "Getting drive bay information...\n";

    my $rc = 1;
    my %rsp;
    my %i;

    %rsp = $obj->diskBayList();
    
    if (!%rsp)
    {
        print "ERROR: Failed to receive a response from diskBayList.\n";
        $rc = 0;
        return $rc;
    }

    if ($rsp{STATUS} != PI_GOOD)
    {
        my $msg = "ERROR: Failed to retrieve disk bay list.";
        displayError($msg, %rsp);
        $rc = 0;
        return $rc;
    }

    print "Checking for SBOD connection status...\n";
    
    for $i (0..$#{$rsp{LIST}})
    {
        my $id = $rsp{LIST}[$i];
        my %info = $obj->diskBayInfo($id);

        if (%info)
        {
            if ($info{STATUS} == PI_GOOD)
            {
                my $bayID    = $info{PD_BID};
                my $bayVend  = $info{VENDID};
                my $bayType  = $info{PD_DEVTYPE};
                my $bayWWNL  = $info{WWN_LO};
                my $bayWWNH  = $info{WWN_HI};
                my $bayLUN   = $info{PD_LUN};

                #
                # Check for SBOD.  If this bay is an SBOD, then we want to 
                # pull up page 0x80 and 0x81 and examine the four host ports
                # to see                 
                if ($bayType == PD_DT_SBOD_SES)
                {
                    my @deviceID;
                    my $cdb;
                    my $str;
                    my $rsp2;
                    my $portStatus;

                    $deviceID[0]{WWN_LO} = $bayWWNL;
                    $deviceID[0]{WWN_HI} = $bayWWNH;
                    $deviceID[0]{PD_LUN} = $bayLUN;
        
                    $cdb = pack("H32", "1C018000FF0000000000000000000000");
                    %rsp2 = $obj->scsiCmd($cdb, undef, @deviceID);
        
                    for ($i = 0; $i < 4; $i++)
                    {
                        (
                        $portStatus
                        ) = unpack("c", substr($rsp2{DATA}, 14 + ($i * 8)));

                        if ($portStatus == 0x3C)
                        {
                            $rc = 0;
                            printf "Bay %8.8x%8.8x port A:%hu status: 0x%2.2x\n", 
                                    $bayWWNL, $bayWWNH, $i, $portStatus;
                        }
                    }
                
                    $deviceID[0]{WWN_LO} = $bayWWNL;
                    $deviceID[0]{WWN_HI} = $bayWWNH;
                    $deviceID[0]{PD_LUN} = $bayLUN;
        
                    $cdb = pack("H32", "1C018100FF0000000000000000000000");
                    %rsp2 = $obj->scsiCmd($cdb, undef, @deviceID);
        
                    for ($i = 0; $i < 4; $i++)
                    {
                        (
                        $portStatus
                        ) = unpack("c", substr($rsp2{DATA}, 14 + ($i * 8)));

                        if ($portStatus == 0x3C)
                        {
                            $rc = 0;
                            printf "Bay %8.8x%8.8x port B:%hu status: 0x%2.2x\n", 
                                    $bayWWNL, $bayWWNH, $i, $portStatus;
                        }
                    }
                
                }
            }
            else
            {
                my $msg = "Unable to retrieve disk bay information.";
                displayError($msg, %info);
                $rc = 0;
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            $rc = 0;
        }
    }

    print "Completed testing disk bay status...\n";
    print "\n";

    return $rc;
}

##############################################################################
# Test script start
##############################################################################
my $logFile = XIOTech::logMgr::logStart("jlwtest", "TS");

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
print "Welcome to the SBOD connection test program!  \n";
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
    $rc_check = checkSBOD();
}

if ($rc) # last
{
    $rc = $obj->logout();
}

if ($rc_check)
{
    print "  TESTS COMPLETED SUCCESSFULLY\n";
    open(OUTF, ">c:\\sbodchk.out");
    print OUTF "PASSED";
    close OUTF;
    exit 0;
}
else
{
    print "  TESTS FAILED\n";
    open(OUTF, ">c:\\sbodchk.out");
    print OUTF "FAILED";
    close OUTF;
    exit 1;
}

##############################################################################
# Change log:
# $Log$
# Revision 1.4  2005/05/27 16:01:51  WilliamsJ
# TBolt00000000 - shortened file name
#
# Revision 1.3  2005/05/27 15:04:48  WilliamsJ
# TBolt00000000 - changed to output to a file.
#
# Revision 1.2  2005/05/17 18:07:08  WilliamsJ
# Check for SBOD port status.
#
# Revision 1.1  2005/05/17 16:33:02  WilliamsJ
# Check for SBOD port status.
#
##############################################################################

