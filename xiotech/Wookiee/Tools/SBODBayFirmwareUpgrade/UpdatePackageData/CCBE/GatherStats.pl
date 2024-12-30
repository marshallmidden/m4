#!/usr/bin/perl -w
#====================================================================
#
# FILE NAME:    GatherStats.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         3/14/2005
#
# DESCRIPTION:  
#
#====================================================================

use strict;
use	Getopt::Std;
use IO::Handle;
use Socket;

use XIOTech::cmdMgr;
use XIOTech::cmUtils;
use XIOTech::PI_Constants;
use XIOTech::PI_CommandCodes;

##########################################################

use constant RECORDLEN_LEN => 2;
use constant RECORDLEN_TPL => 
   "S";     # R_RECORDLEN

use constant HEADER_LEN => 18;
use constant HEADER_TPL =>
   "A14     # R_EYECATCHER
    L";     # R_TIMESTAMP
    
use constant DEVICECNT_LEN => 2;
use constant DEVICECNT_TPL => 
   "S";     # R_DEVICECNT - controller count

# The above DEVICECNT is the count of the controllers 
# in the record. The controller info extends to the end 
# of the data.
use constant STATSLOAD_LEN => 6;
use constant STATSLOAD_TPL =>
   "L       # G_IP
    C       # G_BELOAD
    C";     # G_FELOAD
    
use constant GLOBALCACHEINFO_LEN => 55;
use constant GLOBALCACHEINFO_TPL =>
   "C       # G_STATUS
    C       # G_BATTERY
    C       # G_STOPCNT
    L       # G_SIZE
    L       # G_MAXCWR
    L       # G_MAXSGL
    L       # G_NUMTAGS
    L       # G_TAGSDIRTY
    L       # G_TAGSRESIDENT
    L       # G_TAGSFREE
    L       # G_TAGSFLUSHIP
    L       # G_NUMBLKS
    L       # G_BLOCKSDIRTY
    L       # G_BLOCKRESIDENT
    L       # G_BLOCKFREE
    L";     # G_BLKSFLUSHIP

# There is a DEVICECNT_TPL in here:
#               S_DEVICECNT - server count

# The above DEVICECNT is the count of the following
# devices (STATSSERVER_TPL's) in the record.   
use constant STATSSERVER_LEN => 22;
use constant STATSSERVER_TPL =>
   "S       # S_ID 
    S       # S_PER_CMDS 
    L       # S_PER_BYTES
    S       # S_PER_WRITES
    L       # S_PER_WBYTES
    S       # S_PER_READS
    L       # S_PER_RBYTES
    S";     # S_QDEPTH     

# There is a DEVICECNT_TPL in here:
#               V_DEVICECNT - vdisk count
#  
# The above DEVICECNT is the count of the following
# devices (STATSVDISK_TPL's) in the record.   
    
use constant STATSVDISK_LEN => 16;
use constant STATSVDISK_TPL =>
   "S       # V_ID 
    S       # V_RPS 
    L       # V_AVGSC
    L       # V_WRTBYRES
    L";     # V_WRTBYLEN

# There is a DEVICECNT_TPL in here:
#               H_DEVICECNT - habs count
# 
# The above DEVICECNT is the count of the following
# devices (STATSHAB_TPL's) in the record.   
    
use constant STATSHAB_LEN => 10;
use constant STATSHAB_TPL =>
   "S       # H_ID
    S       # H_PER_CMDS 
    S       # H_QDEPTH
    L";     # H_AVG_REQ

##########################################################
    
#
# Get the command line args
#
our ($opt_a, $opt_s, $opt_m); 
getopts('as:m:');
 
#
# Print out help/usage info
#
my $script = $0;
$script =~ s/\\/\//g;  # back slashes -> forward slashes
$script =~ s/^.*\///;  # get base name
if (@ARGV < 1) 
{ 
    print "\nUsage: $script [-a] [-s sec] [-m min-to-run] filename ".
            "controller-IP [controller-IP ...]\n\n";
    exit 1;
}

#
# Ask for a yes no response. Default if enter key pressed == no.
# Returns 1 for yes, 0 for no.
#
sub AskForYesNo
{
    my ($question) = @_;
    my $answer; 
    my $rc = 0; # 'no' is the default

    print "$question Y/[N] ";
    $answer = <STDIN>;
    $rc = 1 if ($answer =~ /^Y/i); 
 
    return $rc;
}

#
# Start a command manager
#
sub StartMgr
{
    my ($ip_port) = @_;
    my ($ip, $port) = split /:/, $ip_port;
    $port = 3200 if (!defined($port));
    
    my $mgr = XIOTech::cmdMgr->new(\*STDOUT);

    if (defined($mgr))
    {
        if ($mgr->login($ip, $port))
        {
            print "Login to ($ip:$port) successful.\n";
            return $mgr;
        }
    }
    
    # Couldn't establish connection
    print "\nLogin to ($ip:$port) failed!\n";
    return undef;
}

my $file = shift @ARGV;
my $append = 1 if ($opt_a);
my $secDelay = 5;
my $minToRun = 0;
my $rc;

if ($opt_s)
{
    if ($opt_s =~ /^\d{1,2}$/ and ($opt_s >= 1))
    {
        $secDelay = $opt_s;
    }
    else
    {
        print "\n-s must specify a value between 1-99 seconds.\n";
        exit 1;
    }
}

if ($opt_m)
{
    if ($opt_m =~ /^\d{1,4}$/ and ($opt_m >= 1))
    {
        $minToRun = $opt_m;
    }
    else
    {
        print "\n-t must specify a value between 1-9999 minutes.\n";
        exit 1;
    }
}

if ($append)
{
    $rc = open *LOG, ">>$file";
}
else
{
    if (-r $file)
    {
        if (AskForYesNo("\n$file exists. Overwrite?"))
        {
            $rc = open *LOG, ">$file";
        }
        else
        {
            print "\nChoose a new file to log to.\n";
            exit 1;
        }
    }
    else
    {
        $rc = open *LOG, ">$file";
    }
}
binmode *LOG;
*LOG->autoflush(1);
my $fh = *LOG;

#
# Open up a command manager for each controller
#
if (@ARGV == 0)
{
    print "\nYou must specify at least one controller to log from.\n";
    exit 1;
}

my @mgrs;
while (@ARGV)
{
    my $ctlr = StartMgr(shift @ARGV);

    if (defined ($ctlr))
    {
        push @mgrs, $ctlr;
    }
    else
    {
        # login failed.
        exit 1;
    }
}

#
# Outer loop runs forever, with a sleep at the top
#
print "Recording...\n";
my $recordNum = 1;
my $endTime = time() + ($minToRun * 60);
MAIN: while (1)
{
    my $recordTime;
    my $record;
    my $ctrl;
    my $deviceData;
    my $deviceCount;
    my $deviceID;
    my @deviceList;
    my %rsp;
    my %rsp2;
    
    #
    # Wait here for the next pass
    #
    if ($minToRun and time() > $endTime)
    {
        print "Done: $minToRun minutes.\n";
        last;
    }
    sleep $secDelay;

    #
    # Pack the initial header and controller count.
    #
    my $tmpStr = sprintf(">REC_%u", $recordNum++);
    $record = pack(HEADER_TPL, $tmpStr, time);
    $record .= pack(DEVICECNT_TPL, scalar(@mgrs));
    
    #
    # Inner loop runs once for each controller
    #
    foreach $ctrl (@mgrs)
    {
        #
        # Stuff in the controller IP address and processor utilization
        # 
        %rsp = $ctrl->statsProc("ALL");
        if (!%rsp or $rsp{STATUS} != PI_GOOD)
        {
            print "STATSPROC to controller $ctrl->{HOST} failed.\n";
            next MAIN;
        }
        else
        {
            $record .= pack(STATSLOAD_TPL, 
                unpack("L",inet_aton($ctrl->{HOST})), # sigh...
                (100-$rsp{BE_II_UTZN}), 
                (100-$rsp{FE_II_UTZN}));
        }

        #
        # Go get the global cache info
        #
        %rsp = $ctrl->globalCacheInfo();
        if (!%rsp or $rsp{STATUS} != PI_GOOD)
        {
            print "GLOBALCACHEINFO to controller $ctrl->{HOST} failed.\n";
            next MAIN;
        }
        else
        {
            $record .= pack(GLOBALCACHEINFO_TPL, 
                $rsp{CA_STATUS},
                $rsp{CA_BATTERY},
                $rsp{CA_STOPCNT},
                $rsp{CA_SIZE},
                $rsp{CA_MAXCWR},
                $rsp{CA_MAXSGL},
                $rsp{CA_NUMTAGS},
                $rsp{CA_TAGSDIRTY},
                $rsp{CA_TAGSRESIDENT},
                $rsp{CA_TAGSFREE},
                $rsp{CA_TAGSFLUSHIP},
                $rsp{CA_NUMBLKS},
                $rsp{CA_BLOCKSDIRTY},
                $rsp{CA_BLOCKRESIDENT},
                $rsp{CA_BLOCKFREE},
                $rsp{CA_BLKSFLUSHIP});
        }

        #
        # Get the server list
        # 
        %rsp = $ctrl->getObjectList(PI_SERVER_LIST_CMD);
        if (!%rsp or $rsp{STATUS} != PI_GOOD)
        {
            print "SERVERLIST to controller $ctrl->{HOST} failed.\n";
            next MAIN;
        }
        else 
        {
            @deviceList = @{ $rsp{LIST} };
        }   

        #
        # Get the server stats for each server
        # 
        $deviceData = "";
        $deviceCount = 0;
        foreach $deviceID (@deviceList)
        {
            %rsp = $ctrl->statsServer($deviceID);
            if (!%rsp or $rsp{STATUS} != PI_GOOD)
            {
#                print "STATSSERVER $deviceID to controller $ctrl->{HOST} failed.\n";
            }
            else
            {
                $deviceData .= pack(STATSSERVER_TPL, 
                    $deviceID,
                    $rsp{PER_CMDS},
                    $rsp{PER_BYTES},
                    $rsp{PER_WRITES},
                    $rsp{PER_WBYTES},
                    $rsp{PER_READS},
                    $rsp{PER_RBYTES},
                    $rsp{QDEPTH});
                
                $deviceCount++;
            }
        }
        
        #
        # Add the server count and data to the record
        #
        $record .= pack(DEVICECNT_TPL, $deviceCount);
        $record .= $deviceData;
        
        #
        # Get the vdisk stats for each vdisk
        # 
        %rsp = $ctrl->statsVDisk();
        %rsp2 = $ctrl->statsCacheDevices(0xFFFF);
        if (!%rsp or $rsp{STATUS} != PI_GOOD)
        {
            print "STATSVDISK to controller $ctrl->{HOST} failed.\n";
            next MAIN;
        }
        elsif (!%rsp2 or $rsp2{STATUS} != PI_GOOD)
        {
            print "STATSCACHEDEV to controller $ctrl->{HOST} failed.\n";
            next MAIN;
        }
        else
        {
            $deviceData = "";
            $deviceCount = $rsp{COUNT};
            for ($deviceID=0; $deviceID < $deviceCount; $deviceID++)
            {
                $deviceData .= pack(STATSVDISK_TPL, 
                    $rsp{VDISKS}[$deviceID]{VID},
                    $rsp{VDISKS}[$deviceID]{RPS},
                    $rsp{VDISKS}[$deviceID]{AVGSC},
                    $rsp2{CACHEDEVS}[$deviceID]{VC_WRTBYRES},
                    $rsp2{CACHEDEVS}[$deviceID]{VC_WRTBYLEN});
                if ( $rsp{VDISKS}[$deviceID]{VID} !=
                    $rsp2{CACHEDEVS}[$deviceID]{VC_VID})
                {
                    print "Uh-Oh ...  $rsp{VDISKS}[$deviceID]{VID} != ".
                        "$rsp2{CACHEDEVS}[$deviceID]{VC_VID} !!\n";
                }
            }
        }
        
        #
        # Add the vdisk count and data
        #
        $record .= pack(DEVICECNT_TPL, $deviceCount);
        $record .= $deviceData;

        #
        # Setup the FE HAB list
        # 
        @deviceList = (0,1,2,3);

        #
        # Get the hab stats for each fe hab
        # 
        $deviceData = "";
        $deviceCount = 0;
        foreach $deviceID (@deviceList)
        {
            %rsp = $ctrl->statsHAB($deviceID);
            if (!%rsp or $rsp{STATUS} != PI_GOOD)
            {
#                print "STATSHAB $deviceID to controller $ctrl->{HOST} failed.\n";
            }
            else
            {
                $deviceData .= pack(STATSHAB_TPL, 
                    $deviceID,
                    $rsp{PER_CMDS},
                    $rsp{QDEPTH},
                    $rsp{AVG_REQ});
                
                $deviceCount++;
            }
        }
        
        #
        # Add the hab count and data to the record
        #
        $record .= pack(DEVICECNT_TPL, $deviceCount);
        $record .= $deviceData;
    }
    
    #
    # Pad out to multiple of 16
    #
    my $rlen = length($record) + 2;
    my $remainder = $rlen % 16;
    if ($remainder != 0)
    {
        $record .= "\0" x (16-$remainder);
        $rlen = length($record) + 2;
    }
    
    #
    # Write it out
    #
#    print "writing out $rlen bytes to $file\n";
    print $fh pack RECORDLEN_TPL, length($record);
    print $fh $record;

    if ($recordNum % 20 == 0)
    {
        print "$recordNum records recorded ($rlen byte records).\n";
    }
}

close $fh;

exit 0;

