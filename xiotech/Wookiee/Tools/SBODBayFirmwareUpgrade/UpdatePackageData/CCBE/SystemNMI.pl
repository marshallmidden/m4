#!/bin/perl -I ../../CCBE -w
# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Michael McMaster
#
# Purpose:
#   Election code regression test script
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

use constant GOOD		=> 0;
use constant ERROR		=> 1;

use constant FALSE		=> 0;
use constant TRUE		=> 1;

my @objs;
my $i;
my $rc = ERROR;
my $currentPort = 3100;


##############################################################################
# Name:     DisplayError
#
# Desc:     Displays an error message followed by the status and
#           error codes from a command response.
#
# Input:    message and command response hash.
##############################################################################
sub DisplayError
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
# Test the SystemNMI operation
##############################################################################
sub InjectSystemNMI
{
	my( $controller ) = @_;
    my $rc = ERROR;
    my $address = 0xFE400004;
    my $length  = 1;
    my $proc    = "FE";
    my %rsp;

	##
	# Prevent FE processor from asserting SystemNMI
	##
    %rsp = $controller->ReadMemory($address, $length, $proc);

    print "Clear FE SystemNMI output\n";
    if( $rsp{STATUS} != 0 )
    {
        print "\nThe memory read failed...\n";
    }
    else
    {
        $data = unpack( "C", $rsp{RD_DATA} );
        printf( "  Read:  0x%02x\n", $data );

        $data &= 0x7F;
        printf( "  Write: 0x%02x\n", $data );

        $data = pack( "C", $data );
        %rsp = $controller->WriteMemory($address, $data, $proc);

        if( $rsp{STATUS} == 0 )
        {
            $rc = GOOD;
        }
        else
        {
            print "\nThe memory write failed...\n";
        }
    }

	##
	# Prevent BE processor from asserting SystemNMI
	##
    $address = 0xFE400004;
    $proc = "BE";
    %rsp = $controller->ReadMemory($address, $length, $proc);

    print "Clear BE SystemNMI output\n";
    if( $rsp{STATUS} != 0 )
    {
        print "\nThe memory read failed...\n";
    }
    else
    {
        $data = unpack( "C", $rsp{RD_DATA} );
        printf( "  Read:  0x%02x\n", $data );

        $data &= 0x7F;
        printf( "  Write: 0x%02x\n", $data );

        $data = pack( "C", $data );
        %rsp = $controller->WriteMemory($address, $data, $proc);

        if( $rsp{STATUS} == 0 )
        {
            $rc = GOOD;
        }
        else
        {
            print "\nThe memory write failed...\n";
        }
    }

	##
	# Prevent the CCB from asserting SystemNMI
	##
    $address = 0xFEE80005;
    $proc    = "CCB";

    print "Clear CCB SystemNMI output\n";
    %rsp = $controller->ReadMemory($address, $length, $proc);

    if( $rsp{STATUS} != 0 )
    {
        print "\nThe memory read failed...\n";
    }
    else
    {
        $data = unpack( "C", $rsp{RD_DATA} );
        printf( "  Read:  0x%02x\n", $data );

        $data &= 0x7F;
        printf( "  Write: 0x%02x\n", $data );

        $data = pack( "C", $data );
        %rsp = $controller->WriteMemory($address, $data, $proc);

        if( $rsp{STATUS} == 0 )
        {
            $rc = GOOD;
        }
        else
        {
            print "\nThe memory write failed...\n";
        }
    }

	##
	# Read the SystemNMI input on the FE processor
	##
    $address = 0xFE400000;
    $proc = "FE";

    print "Read FE SystemNMI input\n";
    %rsp = $controller->ReadMemory($address, $length, $proc);

    if( $rsp{STATUS} != 0 )
    {
        print "\nThe memory read failed...\n";
    }
    else
    {
        $data = unpack( "C", $rsp{RD_DATA} );
        printf( "  Read:  0x%02x\n", ($data & 0x80) );
    }

	##
	# Read the SystemNMI input on the BE processor
	##
    $address = 0xFE400000;
    $proc = "BE";

    print "Read BE SystemNMI input\n";
    %rsp = $controller->ReadMemory($address, $length, $proc);

    if( $rsp{STATUS} != 0 )
    {
        print "\nThe memory read failed...\n";
    }
    else
    {
        $data = unpack( "C", $rsp{RD_DATA} );
        printf( "  Read:  0x%02x\n", ($data & 0x80) );
    }

	##
	# Enable the SystemNMI input on the FE processor
	##
    $address = 0xFE400004;
    $proc = "FE";

    print "Enable FE SystemNMI input\n";
    %rsp = $controller->ReadMemory($address, $length, $proc);

    if( $rsp{STATUS} != 0 )
    {
        print "\nThe memory read failed...\n";
    }
    else
    {
        $data = unpack( "C", $rsp{RD_DATA} );
        printf( "  Read:  0x%02x\n", $data );

        $data |= 0x08;
        printf( "  Write: 0x%02x\n", $data );

        $data = pack( "C", $data );
        %rsp = $controller->WriteMemory($address, $data, $proc);

        if( $rsp{STATUS} == 0 )
        {
            $rc = GOOD;
        }
        else
        {
            print "\nThe memory write failed...\n";
        }
    }

	##
	# Enable the SystemNMI input on the BE processor
	##
    $address = 0xFE400004;
    $proc = "BE";

    print "Enable BE SystemNMI input\n";
    %rsp = $controller->ReadMemory($address, $length, $proc);

    if( $rsp{STATUS} != 0 )
    {
        print "\nThe memory read failed...\n";
    }
    else
    {
        $data = unpack( "C", $rsp{RD_DATA} );
        printf( "  Read:  0x%02x\n", $data );

        $data |= 0x08;
        printf( "  Write: 0x%02x\n", $data );

        $data = pack( "C", $data );
        %rsp = $controller->WriteMemory($address, $data, $proc);

        if( $rsp{STATUS} == 0 )
        {
            $rc = GOOD;
        }
        else
        {
            print "\nThe memory write failed...\n";
        }
    }

	##
	# Enable the SystemNMI input on the CCB, and cause the CCB
    # to assert the SystemNMI signal.
	##
    $address = 0xFEE80005;
    $proc    = "CCB";

    print "Enable CCB SystemNMI input and generate SystemNMI\n";
    %rsp = $controller->ReadMemory($address, $length, $proc);

    if( $rsp{STATUS} != 0 )
    {
        print "\nThe memory read failed...\n";
    }
    else
    {
        $data = unpack( "C", $rsp{RD_DATA} );
        printf( "  Read:  0x%02x\n", $data );

        $data |= 0xC0;
        printf( "  Write: 0x%02x\n", $data );

        $data = pack( "C", $data );
        %rsp = $controller->WriteMemory($address, $data, $proc);

        if( $rsp{STATUS} == 0 )
        {
            $rc = GOOD;
        }
        else
        {
            print "\nThe memory write failed...\n";
        }
    }

	##
	# Enable the SystemNMI input on the FE processor
	##
    $address = 0xFE400000;
    $proc = "FE";

    print "Read FE SystemNMI input\n";
    %rsp = $controller->ReadMemory($address, $length, $proc);

    if( $rsp{STATUS} != 0 )
    {
        print "\nThe memory read failed...\n";
    }
    else
    {
        $data = unpack( "C", $rsp{RD_DATA} );
        printf( "  Read:  0x%02x\n", ($data & 0x80) );
    }

	##
	# Enable the SystemNMI input on the BE processor
	##
    $address = 0xFE400000;
    $proc = "BE";

    print "Read BE SystemNMI input\n";
    %rsp = $controller->ReadMemory($address, $length, $proc);

    if( $rsp{STATUS} != 0 )
    {
        print "\nThe memory read failed...\n";
    }
    else
    {
        $data = unpack( "C", $rsp{RD_DATA} );
        printf( "  Read:  0x%02x\n", ($data & 0x80) );
    }

    printf( "\n" );
	return $rc;
}


##############################################################################
# Test script start
##############################################################################
#my $logFile = XIOTech::logMgr::logStart("ElectionStress", "TS");
my $controller = XIOTech::cmdMgr->new(\*STDOUT);
my $numberOfLoops = 1;
my $loopCount = 0;
my $ipaddr = 0;
my %rsp;


##
# Run in file mode or interactive mode based upon if a file is
# passed on the cmdline or not.
##
if( @ARGV >= 1 )
{
    ##
    # Get the first command line parm and see if an IP address
	##
    ($inf) = shift @ARGV;

    ##
    # If its an IP address, we will connect to it automatically
	##
    ($inf, $port) = split( /:/, $inf );

    if ($inf =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/)
    {
        $ipaddr = $inf;
    }

    if( ($port) && ($port =~ /^\d/) )
    {
        $currentPort = $port;
    }

    ##
    # initialize all input parms
	##
    $opt_h = 0;
    $opt_l = 1;

    ##
    # 'getopt' (without the 's') expects all parameters that have an associated
    # text or numeric field to be listed in the input pattern.  'getopts' can
    # be set up to either expect a field or just the option. Refer to the
    # "Programming Perl" book for details.
	##
    getopts( 'hl:' );

    if( $opt_l != 1 )
    {
        print "opt_l = $opt_l\n";
        $numberOfLoops = $opt_l;
    }
}

##
# Autoflush all stdout writes
##
STDOUT->autoflush(1);

print "\n";
print "====================================================\n";
print "Welcome to the BIGFOOT SystemNMI injection script!\n";
print "  Number of loops = $numberOfLoops\n";
print "====================================================\n";

if( !$ipaddr || %opt_h )
{
    print "Invalid parameters specified.\n";
    print "\n";
    print "SystemNMI ipAddr [-l nnn]\n";
    print "  ipAddr       - IP address of a controller (required)\n";
    print "  -l nnn       - Where nnn = loop count to repeat tests\n";

	$rc = ERROR;
}
else
{
	$rc = GOOD;
}

if( $rc == GOOD )
{
	##
	# Send the test introduction to the console
	##
	printf( "\n" );
	printf( "====================================================\n" );
	printf( "SystemNMI - (%d triggers)\n", $numberOfLoops );
	printf( "====================================================\n" );

	##
	# Log into the controller
	##
    print( "Connecting to $ipaddr on port $currentPort\n" );

	if( !$controller->login($ipaddr, $currentPort) )
	{
	    print "Failed to log into controller at $ipaddr\n";
	    $rc = ERROR;
	}

	##
	# Loop as many times as the user desires, as long as everything is good
	##
	while( ($rc == GOOD) &&
		   (($loopCount < $numberOfLoops) || ($numberOfLoops == 0)) )
	{
	    $loopCount++;

		##
		# Send the current loop count to the console
		##
		printf( "Injecting SystemNMI - " );
		if( $numberOfLoops != 0 )
		{
			printf( "(Loop %d of %d)\n", $loopCount, $numberOfLoops );
		}
		else
		{
			printf( "(Loop %d)\n", $loopCount );
		}

		##
		# Inject the SystemNMI event
		##
		if( InjectSystemNMI($controller) == GOOD )
	    {
            sleep( 1 );
	    }
        else
	    {
            $rc = ERROR;
	        printf( "ERROR: Injection failed\n" );
	    }
	}

	##
	# Close the connections and stop logging if the tests received an error
	##
	if( $rc == ERROR )
	{
	    $rc = $controller->logout();
		XIOTech::logMgr::logStop();
	}

	print "\n";
	print "====================================================\n";
	if( $rc == GOOD )
	{
	    print "  TESTS COMPLETED SUCCESSFULLY\n";
	}
	else
	{
	    print "  TESTS FAILED\n";
	}
	print "====================================================\n";
}

if( $rc == GOOD )
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
# Revision 1.1  2003/03/11 21:38:29  McmasterM
# TBolt00000000: Initial checkin
#
#
##############################################################################
