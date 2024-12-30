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
# Test the election operation
##############################################################################
sub TestElection
{
	my( $loopCount, @nodeList ) = @_;
	my $masterIndex = 0;
    my $rc = GOOD;
	my $parallelStart = FALSE;
	my $elapsedTime = 0;
	my $startTime = 0;

	##
	# Locate the master node
	##
	( $rc, $masterIndex ) = FindMasterIndex( @nodeList );

	if( $rc == GOOD )
	{
		my $electionInProgress = FALSE;
		my $indexCounter = 0;
		my $stateChanged = FALSE;
		my $startTime = time;
        my $logMessage = "ElectionStress loop: $loopCount";

        ##
        # Create text log entry on all controllers
        ##
	    for( $indexCounter = 0; $indexCounter < scalar(@nodeList); $indexCounter++ )
	    {
		    my %logMessageRsp = $nodeList[$indexCounter]{OBJECT}->logTextMessage($logMessage, LOG_TYPE_DEBUG);

            if( %logMessageRsp )
            {
                if( $logMessageRsp{STATUS} != PI_GOOD )
                {
                    my $msg = "Bad return on logTextMessage";
                    DisplayError( $msg, %logMessageRsp );
                }
            }
            else
            {
                print "No response on logTextMessage\n";
            }
	    }

		##
		# Start the election (master only or in parallel)
		##
		if( int(rand(100)) < 80 )
		{
			##
			# Start the election from the master controller
		    ##
            $logMessage = "Starting the election (MASTER)";
            printf( "  $logMessage\n" );

            $nodeList[$masterIndex]{OBJECT}->logTextMessage($logMessage, LOG_TYPE_DEBUG);
		    $rc = StartElection( $nodeList[$masterIndex]{OBJECT} );
		}
		else
		{
			##
			# Start the elections on ALL controllers (in parallel)
		    ##
            $logMessage = "Starting the election (PARALLEL)";
            printf( "  $logMessage\n" );

			for( $indexCounter = 0;
				 (($rc == GOOD) && ($indexCounter < scalar(@nodeList)));
				 $indexCounter++ )
			{
                $nodeList[$indexCounter]{OBJECT}->logTextMessage($logMessage, LOG_TYPE_DEBUG);
			    $rc = StartElection( $nodeList[$indexCounter]{OBJECT} );
			}
		}

		##
		# Remember when the election started
		##
		$startTime = time;

		##
		# Watch the progress of the election
		##
		if( $rc == GOOD )
		{
			$electionInProgress = TRUE;
		 
			##
			# Initialize nodeList state tracking variables
			##
			for( $indexCounter = 0; $indexCounter < scalar(@nodeList); $indexCounter++ )
			{
	            printf( "  %-15s", $nodeList[$indexCounter]{IP_ADDRESS} );
			}

	        printf( "\n" );

			for( $indexCounter = 0; $indexCounter < scalar(@nodeList); $indexCounter++ )
			{
				$nodeList[$indexCounter]{STATE}     = 0;
				$nodeList[$indexCounter]{STATE_MSG} = "";

	            printf( "  CN%-13s", $nodeList[$indexCounter]{NODE} );
			}

	        printf( "  Elapsed Time\n" );

			for( $indexCounter = 0; $indexCounter < scalar(@nodeList); $indexCounter++ )
			{
	            printf( "  ---------------" );
			}

            printf( "  ---------------\n" );
		}
		else
		{
            printf( "  ERROR: Could not start the election\n" );
		}

		##
		# Wait for all controllers to report the election has completed
	    ##
	    while( ($rc == GOOD) &&
	    	   ($electionInProgress == TRUE) )
	    {
			$electionInProgress = FALSE;
			$stateChanged = FALSE;

			for( $indexCounter = 0;
				 ($rc == GOOD) && ($indexCounter < scalar(@nodeList));
				 $indexCounter++ )
			{
				##
				# Ask the controller for the current state of its election
				##
			    my %rsp = $nodeList[$indexCounter]{OBJECT}->vcgElectionState();

				##
				# Wait for the response from the controller
				##
			    if( %rsp )
			    {
			        if( $rsp{STATUS} == PI_GOOD )
			        {
						##
						# Look for state changes for any of the controllers
						##
						if( ($nodeList[$indexCounter]{STATE}     != $rsp{STATE}) ||
							($nodeList[$indexCounter]{STATE_MSG} ne $rsp{STATE_MSG}) )
						{
							$stateChanged = TRUE;
							$nodeList[$indexCounter]{STATE}     =  $rsp{STATE};
							$nodeList[$indexCounter]{STATE_MSG} =  $rsp{STATE_MSG};
						}

						##
						# Check if the election is finished
						##
				        if( ($nodeList[$indexCounter]{STATE} != DEBUG_ED_STATE_FAILED) &&
				            ($nodeList[$indexCounter]{STATE} != DEBUG_ED_STATE_END_TASK) )
						{
				            $electionInProgress = TRUE;
						}
			        }
			        else
			        {
			            my $msg = "Bad return status from vcgElectionState for CN$nodeList[$indexCounter]{NODE}";
			            DisplayError( $msg, %rsp );

						$rc = ERROR;
			        }
			    }
		        else
		        {
		            my $msg = "Unable to retrieve election state for CN$nodeList[$indexCounter]{NODE}";
		            DisplayError( $msg, %rsp );

					$rc = ERROR;
		        }
			}

			##
			# Display a message when the state changes
			##
			if( ($rc == GOOD) &&
				($stateChanged == TRUE) )
			{
				for( $indexCounter = 0; $indexCounter < scalar(@nodeList); $indexCounter++ )
				{
					##
					# Abbreviate the election state message
					##
					my $stateMessage = $nodeList[$indexCounter]{STATE_MSG};
					$stateMessage =~ s/Election|State//ig;
					$stateMessage =~ s/FAILED/FAILED/ig;
					$stateMessage =~ s/[^A-Z]//g;

		            printf( "  %-15s", $stateMessage );
				}

	            printf( "  (%d seconds)\n", time - $startTime );
			}
	    }

		##
		# Calculate and print how long the election took
		##
		$elapsedTime = time - $startTime;
	}
	else
	{
		printf( "FindMasterIndex returned ERROR\n" );
	}

    return( $rc, $elapsedTime );
}


##############################################################################
# StartElection
##############################################################################
sub StartElection
{
	my( $controller ) = @_;
	my $rc = ERROR;
    my $procNum = 0;  # Dummy value for generic command

	##
	# Start the election from the master controller
	##
    my %rsp = $controller->genericCommand( "ELECTION", $procNum );

	##
	# Wait for the response from the master controller
	##
    if( %rsp )
    {
        if( $rsp{STATUS} == PI_GOOD )
        {
			$rc = GOOD;
        }
        else
        {
            my $msg = "Failed to start election.";
            DisplayError( $msg, %rsp );
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet\n";
    }

	return $rc;
}


##############################################################################
# HeapReport
##############################################################################
sub HeapReport
{
	my( $controller ) = @_;
    my %rsp;
    my $rc = ERROR;

    %rsp = $controller->generic2Command( "heap" );

    if( %rsp )
    {
        if( $rsp{STATUS} == PI_GOOD )
        {
            $controller->FormatHeapData( $rsp{DATA}, undef );
            $rc = GOOD;
        }
        else
        {
            my $msg = "ERROR: Unable to retrieve report data";
            DisplayError( $msg, %rsp );
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet\n";
    }

    return $rc;
}


##############################################################################
# LoginControllers
##############################################################################
sub LoginControllers
{
	my ($ipaddr) = @_;
    my $indexCounter = 0;
    my $nodeCounter = 0;
	my $rc = GOOD;
	my @nodeArray = ();

	##
	# First, build a list of all VCG's controller IP addresses.  After this
	# list is built, figure out what the IP address of the master controller
	# is.  Connect to the master controller and verify that it believes that
	# the correct number of controllers are part of the VCG.
	##

	##
	# Log into the controller whose IP address was passed to us
	##
    my $object = XIOTech::cmdMgr->new(\*STDOUT);

    if( $object->login($ipaddr, $currentPort) )
	{
        print( "Login to $ipaddr successful\n" );

		##
		# Ask for this controller's VCG information and log back out
		##
	    my %vcgInfoRsp = $object->vcgInfo(0);
	    $object->logout();
		undef $object;

	    if( %vcgInfoRsp )
		{
			if( $vcgInfoRsp{VCG_CURRENT_NUM_CONTROLLERS} > 0 )
			{
				##
				# Decode this controller's VCG information to determine how many
				# other controllers are part of the VCG
				##
			    print( "  $ipaddr has $vcgInfoRsp{VCG_CURRENT_NUM_CONTROLLERS} controllers\n" );

				##
				# Give some time for the CCB to close the far side of the connection
				##
				sleep( 1 );
	
				##
				# Login to all OPERATIONAL controllers
				##
			    for( $indexCounter = 0;
			    	 ( ($rc == GOOD) && ($indexCounter < $vcgInfoRsp{VCG_CURRENT_NUM_CONTROLLERS}) );
			    	 $indexCounter++ )
			    {
					if( $vcgInfoRsp{CONTROLLERS}[$indexCounter]{FAILURE_STATE} == FD_STATE_OPERATIONAL )
					{
						$nodeArray[$nodeCounter]{OBJECT}     = XIOTech::cmdMgr->new(\*STDOUT);
						$nodeArray[$nodeCounter]{IP_ADDRESS} = $vcgInfoRsp{CONTROLLERS}[$indexCounter]{IP_ADDRESS};
						$nodeArray[$nodeCounter]{NODE}       = $indexCounter;

						my $loginRsp = $nodeArray[$nodeCounter]{OBJECT}->login($vcgInfoRsp{CONTROLLERS}[$indexCounter]{IP_ADDRESS}, $currentPort);

						if( $loginRsp )
						{
					        print( "    Logged into: $nodeArray[$nodeCounter]{IP_ADDRESS}\n" );
						}
						else
						{
					        print( "    Unable to log into: $vcgInfoRsp{CONTROLLERS}[$indexCounter]{IP_ADDRESS}\n" );
					        $rc = ERROR;
						}

						$nodeCounter++;
						undef $loginRsp;
					}
					else
					{
				        printf( "    INACTIVE: %-15s (%d)\n",
				        	$vcgInfoRsp{CONTROLLERS}[$indexCounter]{IP_ADDRESS},
				        	$vcgInfoRsp{CONTROLLERS}[$indexCounter]{FAILURE_STATE} );
					}
			    }
			}
			else
			{
	            print "ERROR: $ipaddr reports $vcgInfoRsp{VCG_CURRENT_NUM_CONTROLLERS} controllers\n";
				$rc = ERROR;
			}
		}
		else
	    {
	        print "ERROR: Failed to get VCG infromation\n";
	        $rc = ERROR;
	    }
	}
    else
    {
        print "Log into $ipaddr unsuccessful\n";
        $rc = ERROR;
    }

    return( $rc, @nodeArray );
}


##############################################################################
# FindMasterIndex
##############################################################################
sub FindMasterIndex
{
	my (@nodeArray) = @_;
	my $rc = GOOD;
	my $indexCounter = 0;
	my $masterIndex = -1;

	##
	# Ask for this controller's VCG information
	##
	for( $indexCounter = 0; $indexCounter < scalar(@nodeArray); $indexCounter++ )
	{
        #print( "    Getting VCGINFO from $nodeArray[$indexCounter]{IP_ADDRESS}\n" );
		my %vcgInfoRsp = $nodeArray[$indexCounter]{OBJECT}->vcgInfo(0);

		if( %vcgInfoRsp )
		{
			##
			# If this node's IP address is the same as the DSC's address,
			# then we've found the master controller node.
			##
			if( ($nodeArray[$indexCounter]{IP_ADDRESS} eq $vcgInfoRsp{VCG_IP_ADDRESS}) &&
				($vcgInfoRsp{CONTROLLERS}[$nodeArray[$indexCounter]{NODE}]{AM_I_MASTER} == 1) )
			{
				$masterIndex = $indexCounter;
			}
		}
		else
	    {
	        print "ERROR: Failed to get VCG infromation\n";
	        $rc = ERROR;
	    }
	}

	##
	# Verify that the master node was located
	##
	if( ($masterIndex < 0) || 
		($masterIndex > scalar(@nodeArray)) )
	{
        print( "    Unable to find master index\n" );
        $rc = ERROR;
	}

    return( $rc, $masterIndex );
}


##############################################################################
# Test the election operation
##############################################################################
sub CountOperationalControllers
{
	my( @nodeList ) = @_;
	my $masterIndex = 0;
    my $rc = GOOD;
	my $controllerCount = 0;
	my $indexCounter = 0;

	##
	# Locate the master node
	##
	( $rc, $masterIndex ) = FindMasterIndex( @nodeList );

	if( $rc == GOOD )
	{
		##
		# Get the VCGINFO from the master controller and count the
		# number of OPERATIONAL controllers still in the DSC.
	    ##
		my %vcgInfoRsp = $nodeList[$masterIndex]{OBJECT}->vcgInfo(0);

		if( %vcgInfoRsp )
		{
			##
			# Ask for this controller's VCG information
			##
		    for( $indexCounter = 0;
		    	 $indexCounter < $vcgInfoRsp{VCG_CURRENT_NUM_CONTROLLERS};
		    	 $indexCounter++ )
		    {
				if( $vcgInfoRsp{CONTROLLERS}[$indexCounter]{FAILURE_STATE} == FD_STATE_OPERATIONAL )
				{
					$controllerCount++;
				}
			}
		}
		else
		{
	        print "ERROR: Failed to get VCG infromation\n";
	        $rc = ERROR;
		}
	}
	else
	{
        print "ERROR: Failed to count operational controllers\n";
        $rc = ERROR;
	}

    return( $rc, $controllerCount );
}


##############################################################################
# Test script start
##############################################################################
#my $logFile = XIOTech::logMgr::logStart("ElectionStress", "TS");
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
    if ($inf =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/)
    {
        $ipaddr = $inf;
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
print "==============================================\n";
print "Welcome to the BIGFOOT election test script!\n";
print "  Number of loops = $numberOfLoops\n";
print "==============================================\n";

if( !$ipaddr || %opt_h )
{
    print "Invalid parameters specified.\n";
    print "\n";
    print "ElectionStress ipAddr [-l nnn]\n";
    print "  ipAddr       - IP address of a controller in the VCG (required)\n";
    print "  -l nnn       - Where nnn = loop count to repeat tests\n";

	$rc = ERROR;
}
else
{
	$rc = GOOD;
}

##
# Connect to initial system if requested on the command line
# and read the IP addresses of the controllers in the DSC.
##
if( $rc == GOOD )
{
	my @nodeList = ();
	my $elapsedTime = 0;
	my $minTime = NULL;
	my $maxTime = NULL;
	my $totalTime = 0;

	##
	# Log into all operational controllers and build the
	# connections into an object array
	##
	( $rc, @nodeList ) = LoginControllers( $ipaddr );

	if( $rc == GOOD )
	{
		##
		# Go into the election loop
		##
		if( scalar(@nodeList) > 0 )
		{
			##
			# Send the test introduction to the console
			##
			printf( "\n" );
			printf( "====================================================\n" );
			printf( "Election Stress - (%d controllers)\n", scalar(@nodeList) );
			printf( "====================================================\n" );

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
				printf( "Doing election - " );
				if( $numberOfLoops != 0 )
				{
					printf( "(Loop %d of %d)\n", $loopCount, $numberOfLoops );
				}
				else
				{
					printf( "(Loop %d)\n", $loopCount );
				}

				##
				# Look for memory leaks
				##
				#if( $rc == GOOD )
				#{
				#	$rc = HeapReport( $nodeList[$masterIndex]{OBJECT} );
				#}

				##
				# Do an election.
				##
				if( $rc == GOOD )
				{
					##
					# Do the election, and wait for it to complete
					##
				    ($rc, $elapsedTime) = TestElection( $loopCount, @nodeList );

					##
					# Track the min, max, and average times for the elections
					##
				    if( $rc == GOOD )
				    {
						if( ($minTime eq NULL) ||
							($elapsedTime < $minTime) )
						{
							$minTime = $elapsedTime;
						}

						if( ($maxTime eq NULL) ||
							($elapsedTime > $maxTime) )
						{
							$maxTime = $elapsedTime;
						}

						$totalTime = $totalTime + $elapsedTime;

						printf( "  Elapsed: %d, Min: %d, Max: %d, Ave: %d\n",
							$elapsedTime, $minTime, $maxTime, int($totalTime / $loopCount) );
					}

					##
					# Check that the same number of controllers are still in the group
					##
				    if( $rc == GOOD )
				    {
					    ( $rc, $controllerCount ) = CountOperationalControllers( @nodeList );

						if( $rc == GOOD )
						{
							if( scalar(@nodeList) != $controllerCount )
							{
						        printf( "ERROR: Lost a controller during the election\n" );
						        printf( "  Started with %d\n", scalar(@nodeList) );
						        printf( "  Ended   with %d\n", $controllerCount );
								$rc = ERROR;
							}
						}
						else
						{
							printf( "CountOperationalControllers returned ERROR\n" );
						}
				    }
				}

				##
				# Display error message before exiting
				##
				if( $rc == GOOD )
				{
					##
					# Wait a variable amount of time before starting the next election
					# to stress resource manager with elections (0 to 15 seconds).
					##
					my $delay = int( rand(15) );
					printf( "  Pausing for $delay seconds\n" );
					sleep( $delay );
				}
			    else
			    {
			        printf( "ERROR: Election failed\n" );
			    }
			}

			##
			# Close the connections and stop logging if the tests received an error
			##
			print "\n";
			print "====================================================\n";
			if( $rc == GOOD )
			{
			    print "  TESTS COMPLETED SUCCESSFULLY\n";
			}
			else
			{
			    print "  TESTS FAILED\n";

			    for( $i = 0; $i < scalar(@nodeList); $i++ )
			    {
			        $rc = $nodeList[$i]{OBJECT}->logout();
			    }

				XIOTech::logMgr::logStop();
			}
			print "====================================================\n";
		}
		else
		{
			printf( "LoginControllers returned ERROR\n" );
		}
	}
}


##
# Place the correct exit code into ERRORLEVEL
##
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
# Revision 1.7  2004/07/21 15:13:44  McmasterM
# TBolt00010865: Enforce state machine transition into TIMEOUT_CONTROLLES_COMPLETE
# Added code to enforce election state machine for the TCC state.  Also added
# another error injection flag; this one simulates a slow file system.
# Reviewed by Lynn Waggie
#
# Revision 1.6  2004/01/16 16:25:15  McmasterM
# TBolt00009896: Election N-way testing tweaks
# Changed electionstress script to be N-way compatible.
#
# Revision 1.5  2003/03/18 15:52:21  McmasterM
# TBolt00000000: Added 1 second delay after logout - CCB port open delay
#
# Revision 1.4  2002/05/07 19:10:10  McmasterM
# TBolt00002732: VCG Master Elections - Part 1/2
# This has the solution to the ContactAllControllersComplete timeout condition.
# Two new election states were added to support recovery from an election timeout.
# Also, now only the first set of packets are quorumable during the election.
#
# Revision 1.3  2002/04/23 15:54:05  McmasterM
# TBolt00000000: Turned off script logging and fixed a few bugs.
#
# Revision 1.2  2002/04/10 20:01:25  McmasterM
# TBolt00000000: Corrected CVS header information
#
# Revision 1.1  2002/04/10 19:59:32  McmasterM
# TBolt00000000: Initial checkin
#
##############################################################################
