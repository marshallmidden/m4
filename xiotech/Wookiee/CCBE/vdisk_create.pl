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

my @vdisks;
my $vdisk_count = 0;

my $connectTo1 = 0;


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
# Name:     vdiskCreate
#
# Desc:     Create a virtual disk
#
# Input:
#
# Output:
##############################################################################
sub vdiskCreate
{
    my $rc = 1;
    my ($obj,
    	$capacity,
        $physicalDisks,
        $rtype,
        $stripe,
        $depth,
        $parity) = @_;

    print "\n";

    my @pids;
    my %raidParms;

    if (!defined($capacity))
    {
        print "Invalid or missing capacity.\n";
        $rc = 0;
        return $rc;
    }

    if (!defined($physicalDisks))
    {
        print "Invalid or missing physical disks array.\n";
        $rc = 0;
        return $rc;
    }
    else
    {
        if(uc($physicalDisks) eq "ALL")
		{
			my %pd_info;
			my %pd_cnt = $currentMgr->physicalDiskCount();
			my $cnt = $pd_cnt{COUNT};
			my $usable_disks = 0;
			
			
			for($i = 0; $i < $cnt; ++$i)
			{
				%pd_info = $currentMgr->physicalDiskInfo($i);
				
				if ($pd_info{STATUS} != PI_GOOD)
                {
                    my $msg = "ERROR: Unable to retrieve physical disk label.";
                    displayError($msg, %pd_info);
                    $rc = 0;
                    return $rc;
                }
        
				if($pd_info{PD_CLASS} == 1)
				{
					$pids[$usable_disks] = $i;
					++$usable_disks;
				}
			}
			if ($usable_disks <= 0)
			{
			    print "No physical disks labeled DATA. use PDISKLABEL.\n";
        	    $rc = 0;
                return $rc;
			}
		}
		else
		{
        	@pids = split /,/, $physicalDisks;
		}
    }

    if (!defined($rtype))
    {
        $rtype = RAID_NONE;
    }

    %raidParms = $obj->calcRaidParms(\@pids, $rtype);

    if (!defined($stripe))
    {
        $stripe = $raidParms{STRIPE_SIZE};
    }

    if (!defined($depth))
    {
        $depth = $raidParms{MIRROR_DEPTH};
    }

    if (!defined($parity))
    {
        $parity = $raidParms{PARITY};
    }

    my %rsp = $obj->virtualDiskCreate($capacity,
                                        \@pids,
                                        $rtype,
                                        $stripe,
                                        $depth,
                                        $parity);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $vdisks[$vdisk_count++] = ($rsp{VID});
            $obj->displayVirtualDiskCreate(%rsp);

        	my %info = $obj->virtualDiskInfo($rsp{VID});

	        if (%info)
	        {
	            if ($info{STATUS} == PI_GOOD)
	            {
					#Initialize Raid 5 Vdisks
					if ($rtype == 3)
					{
						$rc = initVdisk($obj,%info);
					}
				}
	            else
	            {
	                my $msg = "Unable to retrieve virtual disk information.";
	                displayError($msg, %info);
	                $rc = 0;
	            }
	        }
	        else
	        {
	            print "ERROR: Failed to receive a response from virtualDiskInfo.\n";
	            $rc = 0;
	        }
			
        }
        else
        {
            my $msg = "ERROR: Unable to create virtual disk.";
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
# Name:     vdiskDelete
#
# Desc:     Deletes a virtual disk
#
# Input:	VID
#
# Output:
##############################################################################
sub vdiskDelete
{
    my $rc = 1;
    my ($obj, $vid) = @_;

    print "\n";

    if (!defined($vid))
    {
        print "Invalid or missing Virtual ID.\n";
        $rc = 0;
        return $rc;
    }

    my %rsp = $obj->virtualDiskDelete($vid);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "ERROR: Unable to delete virtual disk ($vid).";
            displayError($msg, %rsp);
            $rc = 0;
            return $rc;
        }
		else
		{
			--$vdisk_count;
			print "Virtual disk ($vid) deleted successfully.";
		}
    }
    else
    {
        print "ERROR: Failed to receive a response from virtualDiskDelete.\n";
        $rc = 0;
        return $rc;
    }

    return $rc;
}

##############################################################################
# Test the virtual disk operations
##############################################################################
sub initVdisk
{
	my($obj, %info) = @_;

    my $rc = 1;
    my %rsp;
	my $id;
    my %i;

    for ($i = 0; $i < $info{RAIDCNT}; $i++)
    {

		
        $id = $info{RIDS}[$i];
	    %rsp = $obj->virtualDiskInit($id);

	    if (%rsp)
	    {
	        if ($rsp{STATUS} == PI_GOOD)
	        {
	            print "Raid device ($id) initialization started.\n";
	        }
	        else
	        {
	            my $msg = "Unable to initialize raid device.";
	            displayError($msg, %rsp);
				$rc = 0;
	        }
	    }
	    else
	    {
	        print "ERROR: Did not receive a response packet.\n";
	        $rc = 0;
	    }

    }


    return $rc;
}


##############################################################################
# Clean UP
##############################################################################
sub deleteAllVdisks
{
	my($obj) = @_;

    print "\n";
    print "Deleting ALL virtual disks....\n";my %rsp;
    
    my $rc = 1;
    my $i;


    %rsp = $obj->virtualDiskList();
    
    if ((!%rsp) || ($rsp{STATUS} != PI_GOOD))
    {
        print "ERROR: Failed to receive a response from virtualDiskList.\n";
        $rc = 0;
    }
	else
	{
	    for $i (0..$#{$rsp{LIST}})
	    {
	        my $id = $rsp{LIST}[$i];

			$rc = vdiskDelete($obj,$id);
			if(!$rc)
			{
				print "ERROR: Unable to delete VDisk (%id) \n";
				return $rc;
			}
			else
			{
				print "Deleted Vdisk (%d) \n";
			}

	    }
	}


	print "\n";
    print "Completed Deleting virtual disks...\n";
    print "\n";

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
# Test script start
##############################################################################
my $logFile = XIOTech::logMgr::logStart("vdisk_create", "TS");
my $count = 1;
my $raidType = 0;
my $deleteVdisks = 0;
my $size = 10000;
my %rsp;
my %info;

# Run in file mode or interactive mode based upon if a file is 
# passed on the cmdline or not.
if (@ARGV>=1)
{
    # Get the first command line parm and see if an IP address
    ($inf) = shift @ARGV;

    # If its an IP address, we will connect to it automatically
    if ($inf =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/)
    {
        $connectTo1 = $inf;
    }


    # initialize all input parms
    $opt_h=0;
    $opt_c=0;
    $opt_s=0;
    $opt_t=0;
    $opt_d=0;

    # 'getopt' (without the 's') expects all parameters that have an associated
    # text or numeric field to be listed in the input pattern.  'getopts' can
    # be set up to either expect a field or just the option. Refer to the
    # "Programming Perl" book for details.
    getopts('dhc:s:t:');

    if($opt_c)
    {
        print "opt_c = $opt_c \n";
        $count = $opt_c;
    }
    if($opt_s)
    {
        print "opt_s = $opt_s \n";
        $size = $opt_s;
    }
    if($opt_d)
    {
        print "opt_d = $opt_d \n";
        $deleteVdisks = 1;
    }
    if ($opt_t)
    {
        print "opt_t = $opt_t \n";
		if ($opt_t eq "RAID_NONE")
		{
			$raidType = 0;
		}
		elsif ($opt_t eq "RAID_0")
		{
			$raidType = 1;
		}
		elsif ($opt_t eq "RAID_1")
		{
			$raidType = 2;
		}
		elsif ($opt_t eq "RAID_5")
		{
			$raidType = 3;
		}
		elsif ($opt_t eq "RAID_10")
		{
			$raidType = 4;
		}
		elsif (($opt_t > 0) && ($opt_t < 4))
		{
        	$raidType = $opt_t;
		}
    }



}

# Autoflush all stdout writes
STDOUT->autoflush(1);

print "\n";
print "==============================================\n";
print "Welcome to the BIGFOOT Vdisk create test script!\n";
print "Logging information to:\n";
print "  $logFile\n";
print "==============================================\n";


# Create a command manager to use
# Only parameter is where you want the errors to be reported to
$obj1 = XIOTech::cmdMgr->new(\*STDOUT);

# connect to initial system if requested on the command line
if(($connectTo1) && (!$opt_h)) 
{

    print "Attempting to log into contoller ($connectTo1).\n";
    if ($obj1->login($connectTo1, $currentPort))
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
    print "\n";
    print "vdisk_create	 ipAddr1 [-c nnn] [-s nnn] [-t n] \n";
    print "\n";
    print "  ipAddr1           IP address of master controller (required) \n";
    print "  -c nnn            where nnn = number of vdisks to create\n";
    print "  -d                DELETES ALL EXISTING VDISKS before creating\n";
    print "                    new ones. \n";
    print "  -s nnn            where nnn is the size of vdisk in blocks \n";
    print "  -t nnn            where n  = raid type (default = 0)\n";
    print "                       0 = RAID_NONE\n";
    print "                       1 = RAID_0\n";
    print "                       2 = RAID_1\n";
    print "                       3 = RAID_5\n";
    print "                       4 = RAID_10\n";
    $rc = 0;
}


print "\n";
print "====================================================\n";
print "Vdisk create testing ...\n";

#Get a list of all physical disks
if (($rc) && ($deleteVdisks))
{
	deleteAllVdisks($obj1);
}

#Get a list of all physical disks
if ($rc)
{

	# First get the list of physical disks
	%rsp = $obj1->physicalDiskList();

	if (!%rsp)
	{
	    print "ERROR: Failed to receive a response from physicalDiskList.\n";
	    $rc = 0;
	}

	if ($rsp{STATUS} != PI_GOOD)
	{
	    my $msg = "ERROR: Failed to retrieve physical disk list.";
	    displayError($msg, %rsp);
	    $rc = 0;
	}

	$dlist = join ',', @{ $rsp{LIST} };
}


#Create Requested number of Vdisks
while (($rc) && ($count))
{
    $rc = vdiskCreate($obj1,$size, $dlist, $raidType);

	printf "Vdisk count = %d of %d\n",($opt_c-$count+1), $opt_c;
	--$count;

	if ($rc)
	{
	    %rsp = $obj1->virtualDiskCount();

	    if ((!%rsp) || ($rsp{STATUS} != PI_GOOD))
	    {
	        print "ERROR: Failed to receive virtualDiskCount.\n";
	        $rc = 0;
	    }
		else
		{
	    	print "Virtual Disk Count: " . $rsp{COUNT} . "\n";
		}


	}

}


if ($rc) # last
{
    $rc = $obj1->logout();
	XIOTech::logMgr::logStop();
}

									  
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
