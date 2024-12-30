#!/bin/perl -I ../../CCBE -w
# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Chris Nigbur
#
# Purpose:
#   I/O Regression test script
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

my @objs;
my $i;
my $rc = 1;
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

    print "\n\n";
}

##############################################################################
# Test the statistics operations
##############################################################################
sub test_stats
{
    my($obj) = @_;

    print "\n";
    print "Testing statistics operations...\n";

    my $rc = 1;

    $rc = test_statsEnv($obj);

    if ($rc)
    {
        $rc = test_statsI2c($obj);
    }

    if ($rc)
    {
        $rc = test_statsCacheDevices($obj);
    }

    if ($rc)
    {
        $rc = test_statsLoop($obj);
    }

    if ($rc)
    {
        $rc = test_statsPCI($obj);
    }

    if ($rc)
    {
        $rc = test_statsProc($obj);
    }

    if ($rc)
    {
        $rc = test_statsVDisks($obj);
    }

    if ($rc)
    {
        $rc = test_statsPDisks($obj);
    }

    print "Completed testing statistics operations...\n";

    return $rc;
}

##############################################################################
# Environmental statistics operations
##############################################################################
sub test_statsEnv
{
    my($obj) = @_;

    my $rc = 1;
    my %rsp;

    %rsp = $obj->environmentalStats();

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "ERROR: Failed to retrieve environmental statistics information.";
            displayError($msg, %rsp);
            $rc = 0;
            return $rc;
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        $rc = 0;
        return $rc;
    }

    return $rc;
}

##############################################################################
# I2C statistics operations
##############################################################################
sub test_statsI2c
{
    my($obj) = @_;

    my $rc = 1;
    my %rsp;

    %rsp = $obj->i2cStats();

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "ERROR: Failed to retrieve I2C statistics information.";
            displayError($msg, %rsp);
            $rc = 0;
            return $rc;
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        $rc = 0;
        return $rc;
    }

    return $rc;
}

##############################################################################
# Cache devices statistics operations
##############################################################################
sub test_statsCacheDevices
{
    my($obj) = @_;

    my $rc = 1;
    my %rsp;

    %rsp = $obj->statsCacheDevices(0xFFFF);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to retrieve cache device statistics (0xFFFF).";
            displayError($msg, %rsp);
            $rc = 0;
            return $rc;
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        $rc = 0;
        return $rc;
    }

    return $rc;
}

##############################################################################
# Loop statistics operations
##############################################################################
sub test_statsLoop
{
    my($obj) = @_;

    my $rc = 1;
    my %rsp;
    my %i;

    my $channel = 0;
    my @proc;
    $proc[0] = "FE";
    $proc[1] = "BE";

    my $j;
    for $i (0..1)
    {
        for $j (0..3)
        {
            %rsp = $obj->statsLoop($proc[$i], $j);

            if (%rsp)
            {
                if ($rsp{STATUS} != PI_GOOD && $rsp{ERROR_CODE} != $obj->PI_ERROR_INV_CHAN)
                {
                    my $msg = "Unable to retrieve loop statistics ($proc[$i]) port $j.";
                    displayError($msg, %rsp);
                    $rc = 0;
                    last;
                }
            }
            else
            {
                print "ERROR: Did not receive a response packet.\n";
                $rc = 0;
                last;
            }
        }
    }

    return $rc;
}

##############################################################################
# PCI statistics operations
##############################################################################
sub test_statsPCI
{
    my($obj) = @_;

    my $rc = 1;
    my %rsp;

    %rsp = $obj->statsPCI("ALL");

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to retrieve PCI statistics ($type).";
            displayError($msg, %rsp);
            $rc = 0;
            return $rc;
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        $rc = 0;
        return $rc;
    }

    return $rc;
}

##############################################################################
# Proc statistics operations
##############################################################################
sub test_statsProc
{
    my($obj) = @_;

    my $rc = 1;
    my %rsp;

    %rsp = $obj->statsProc("ALL");

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to retrieve Proc statistics ($type).";
            displayError($msg, %rsp);
            $rc = 0;
            return $rc;
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        $rc = 0;
        return $rc;
    }

    return $rc;
}

##############################################################################
# VDisks statistics operations
##############################################################################
sub test_statsVDisks
{
    my($obj) = @_;

    my $rc = 1;
    my %rsp;

    %rsp = $obj->statsVDisk();

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to retrieve virtual disk statistics.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        $rc = 0;
        return $rc;
    }

    return $rc;
}

##############################################################################
# PDisks statistics operations
##############################################################################
sub test_statsPDisks
{
    my($obj) = @_;

    my $rc = 1;
    my %rsp;

    %rsp = $obj->physicalDisks();

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to retrieve physical disks statistics.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        $rc = 0;
        return $rc;
    }

    return $rc;
}

##############################################################################
# Clean UP
##############################################################################
sub cleanup
{
    my($obj) = @_;

    print "Deleting each virtual disk....\n";
    
    my %rsp;
    my $rc = 1;
    my $i;

    for ($i = 0; $i < scalar(@vdisks); $i++)
    {
        $rc = vdiskDelete($obj,$vdisks[$i]);
        if(!$rc)
        {
            return $rc;
        }
    }
    print "Completed Deleting virtual disks...\n";

    return $rc;
}

##############################################################################
# Look for memory leaks
##############################################################################
sub test_memleaks
{
    my($obj) = @_;

    print "\n";
    print "Looking for memory leaks...\n";

    my %rsp;
    my $rc = 0; # assume fail

    %rsp = $obj->generic2Command("heap");

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $obj->FormatHeapData($rsp{DATA}, undef);
            $rc = 1; # pass
        }
        else
        {
            my $msg = "ERROR: Unable to retrieve report data.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
    }

    print "Completed memory leak operations...\n";
    print "\n";

    return $rc;
}

##############################################################################
# loadVCG
##############################################################################
sub loadVCG
{
    my($ipaddr) = @_;
    my $i;

    print "Attempting to log into contoller ($ipaddr).\n";

    my $obj = XIOTech::cmdMgr->new(\*STDOUT);

    if (!$obj->login($ipaddr, $currentPort))
    {
        print "Login unsuccessful\n";
        $rc = 0;
        return $rc;
    }

    my @ipaddrs;

    my %rsp = $obj->vcgInfo(0);

    if (!%rsp)
    {
        print "ERROR: Failed to Get VCG infromation.\n";
        $rc = 0;
        return $rc;
    }

    if ($rsp{VCG_CURRENT_NUM_CONTROLLERS} > 0)
    {
        for ($i = 0; $i < $rsp{VCG_CURRENT_NUM_CONTROLLERS}; $i++)
        {
            $ipaddrs[$i] = $rsp{CONTROLLERS}[$i]{IP_ADDRESS};
        }
    }
    else
    {
        $ipaddrs[0] = $ipaddr;
    }

    $obj->logout();
    undef $obj;

    if ($rc)
    {
        for ($i = 0; $i < scalar(@ipaddrs); $i++)
        {
            $obj = XIOTech::cmdMgr->new(\*STDOUT);

            if (!$obj->login($ipaddrs[$i], $currentPort))
            {
                print "Failed to log into controller at IP ($ipaddrs[$i]).\n";
                $rc = 0;
                last;
            }

            $objs[scalar(@objs)] = $obj;
        }
    }

    return $rc;
}

##############################################################################
# Test script start
##############################################################################
my $logFile = XIOTech::logMgr::logStart("io_regression", "TS");
my $ipaddr = 0;
my $loopCount = 1;

# Run in file mode or interactive mode based upon if a file is
# passed on the cmdline or not.
if (@ARGV>=1)
{
    # Get the first command line parm and see if an IP address
    ($inf) = shift @ARGV;

    # If its an IP address, we will connect to it automatically
    if ($inf =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/)
    {
        $ipaddr = $inf;
    }

    # initialize all input parms
    $opt_h=0;
    $opt_l=0;
    # 'getopt' (without the 's') expects all parameters that have an associated
    # text or numeric field to be listed in the input pattern.  'getopts' can
    # be set up to either expect a field or just the option. Refer to the
    # "Programming Perl" book for details.
    getopts('hl:');

    if($opt_l)
    {
        print "opt_l = $opt_l \n";
        $loopCount = $opt_l;
    }
}

# Autoflush all stdout writes
STDOUT->autoflush(1);

print "\n";
print "==============================================\n";
print "Welcome to the BIGFOOT regression test script!\n";
print "Logging information to:\n";
print "  $logFile\n";
print "  Loop Count = $loopCount \n";
print "==============================================\n";

# connect to initial system if requested on the command line
if ($ipaddr && !%opt_h)
{
    $rc = loadVCG($ipaddr);
}
else
{
    print "Invalid parameters specified.\n";
    print "\n";
    print "io_regression     ipAddr [-l nnn]\n";
    print "\n";
    print "  ipAddr            IP address of a controller in the VCG (required) \n";
    print "  -l nnn            where nnn = loop count to repeat tests \n";
   $rc = 0;
}

print "\n";
print "====================================================\n";
print "I/O Regression testing for a VCG...\n";
print "====================================================\n";

while ($rc && $loopCount)
{
    --$loopCount;

    if ($rc)
    {
        for ($i = 0; $i < scalar(@objs); $i++)
        {
            $rc = test_stats($objs[$i]);

            if (!$rc)
            {
                last;
            }
        }
    }

    if ($rc)
    {
        for ($i = 0; $i < scalar(@objs); $i++)
        {
            $rc = test_memleaks($objs[$i]);

            if (!$rc)
            {
                last;
            }
        }
    }

    if ($rc)
    {
        $rc = cleanup($objs[0]);
    }
}

if ($rc)
{
    for ($i = 0; $i < scalar(@objs); $i++)
    {
        $rc = $objs[$i]->logout();
    }

    XIOTech::logMgr::logStop();
}

print "\n";
print "====================================================\n";
if ($rc)
{
    print "  TESTS COMPLETED SUCCESSFULLY\n";
}
else
{
    print "  TESTS FAILED\n";
}
print "====================================================\n";

if ($rc)
{
    exit 0;
}
else
{
    exit 1;
}
##############################################################################
# $Log$
# Revision 1.2  2006/07/17 20:38:31  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.1.1.1.30.1  2006/05/22 10:27:01  deepakrc
#
# Fix for Tbolt00014402.
#
# Change the include directory from "ccbe" to "CCBE", since the file-system namespace
# is case-sensitive in linux.
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.4  2003/04/22 13:39:32  TeskeJ
# tbolt00008122 - 'pdisks loop' updates and changed 'channel' to 'port'
# rev by Chris
#
# Revision 1.3  2002/12/17 23:36:51  McmasterM
# TBolt00006250: Add support for I2C switch device
# TBolt00006251: Add support for new I2C EEPROMs (component info collection)
# Full switch support and nearly all of the EEPROM support is in place.
#
# Revision 1.2  2002/04/30 20:04:46  NigburC
# TBolt00004033, TBolt00002733, TBolt00002730 - Lots of changes for these
# three defects.  Mainly, modified the VCGInfo request to return all controllers
# configured as part of the VCG instead of just active controllers.  This caused
# changes in CCB, CCBE and UMC code.
# Added the REMOVE, FAIL, UNFAIL, and SHUTDOWN methods for VCGs.
# Not all of these are working completely...just a stage check-in.
#
# Revision 1.1  2002/03/06 21:51:44  NigburC
# Initial integration of the IO regression script.
#
##############################################################################
