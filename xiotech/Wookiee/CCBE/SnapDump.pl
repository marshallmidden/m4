#!/usr/bin/perl -w
#====================================================================
#
# FILE NAME:    SnapDump.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         12/09/2003
#
# DESCRIPTION:  Grabs a controller "snapshot" and/or decodes the
#               snapshot data.
#
#====================================================================

use strict;

use Cwd;
use	Getopt::Std;
use IO::Handle;

use XIOTech::cmdMgr;
use XIOTech::cmUtils;
use XIOTech::PI_Constants;
use XIOTech::constants;
use XIOTech::fmtFIDs;

STDOUT->autoflush(1);

use constant GOOD  => 0;
use constant ERROR => 1;

use constant RC_GOOD                => 0;
use constant RC_LOGIN_FAILED        => 1;
use constant RC_SHELL_CMD_FAILED    => 2;
use constant RC_FILE_OPEN_FAILED    => 3;
use constant RC_PARAMETER_ERROR     => 4;
use constant RC_DECODE_FAILED       => 5;
use constant RC_LOGSIM_NOT_FOUND    => 6;

#
# Globals. Others defined as needed.
#
my %fileList;
my $startDir = cwd();
$startDir =~ s|\\|/|g; # \'s to /'s
my $progDir;
my $logSim;
my $sysRel;
my $errors = 0;
my $warnings = 0;
my $progRC = RC_GOOD;

our ($opt_f, $opt_h, $opt_l, $opt_o, $opt_p, $opt_s); 

#
# Build the "FID Bucket Hash" that specifies which FIDs to process
# and how to name them.
#
sub BuildFidBucketHash()
{
    my %bucket;

    $bucket{"CCB Backtrace"} =     # Output Filename == hash key
    [
        # FID#  FID Name                           include in interactive dump?
        [ (293, "Backtrace",                                    "yes") ],
        [ (298, "Previous Backtraces",                          "yes") ]
    ];

    $bucket{"CCB Currenttrace"} =
    [
        [ (256, "CCB Trace Buffer",                             "yes") ],
        [ (257, "CCB Serial Buffer",                            "yes") ],
        [ (258, "CCB Heap Stats",                               "yes") ],
        [ (259, "CCB Profile Dump",                             "no") ],
        [ (260, "CCB PCB Dump",                                 "yes") ],
        [ (292, "Command Record Table",                         "yes") ],
        [ (296, "FW Versions",                                  "yes") ],
        [ (297, "Timestamp",                                    "yes") ],
        [ (300, "VCG Info",                                     "yes") ],
        [ (304, "Mirror Partner List",                          "yes") ]
    ];

    #
    # "CCB Currenttrace" is alphabetically before "CCB Logs".  This is
    # important since FID 296 must be processed before the logs are 
    # processed. (The hash keys are processed in sorted order.)
    #
    $bucket{"CCB Logs"} =
    [
        [ (261, "Raw Custmr log flash",                         "yes") ],
        [ (262, "Raw Debug log flash",                          "yes") ],
    ];

    $bucket{"CCB Cached Data"} = 
    [
        [ (263, "DiskBayMap",                                   "yes") ],
        [ (264, "DiskBayPaths",                                 "yes") ],
        [ (265, "PDiskMap",                                     "yes") ],
        [ (266, "PDiskFailMap",                                 "yes") ],
        [ (267, "PDiskRebuildMap",                              "yes") ],
        [ (268, "PDiskPaths",                                   "yes") ],
        [ (269, "VDiskMapP",                                    "yes") ],
        [ (270, "VDiskCopyMap",                                 "yes") ],
        [ (271, "VDiskMirrorMapP",                              "yes") ],
        [ (272, "RaidMap",                                      "yes") ],
        [ (273, "ServerMap",                                    "yes") ],
        [ (274, "TargetMap",                                    "yes") ],
        [ (275, "DiskBays",                                     "yes") ],
        [ (276, "Targets",                                      "yes") ],
        [ (277, "FELoopStats",                                  "yes") ],
        [ (278, "BELoopStats",                                  "yes") ],
        [ (279, "PDisks",                                       "yes") ],
        [ (280, "VDisks",                                       "yes") ],
        [ (281, "Raids",                                        "yes") ],
        [ (282, "Servers",                                      "yes") ]
    ];

    $bucket{"CCB Statistics"} = 
    [
## UNUSED        [ (283, "I2C",                                          "yes") ],
        [ (284, "Proc FE_BE",                                   "yes") ],
        [ (285, "PCI FE_BE",                                    "yes") ],
        [ (286, "Environmental",                                "yes") ],
        [ (287, "Servers",                                      "yes") ],
        [ (288, "VDisks",                                       "yes") ],
        [ (289, "Cache Devices",                                "yes") ],
        [ (290, "Loop FE_BE",                                   "yes") ],
        [ (291, "FCAL Counters",                                "yes") ],
        [ (299, "FCM Counters",                                 "yes") ],
        [ (303, "CCB Stats",                                    "yes") ],
        [ (353, "Bay SES Data",                                 "yes") ],
        [ (354, "iSCSI Stats",                                  "yes") ],
        [ (355, "Async Rep",                                    "yes") ]
    ];

    $bucket{"CCB Misc"} = 
    [
        [ (6,   "Master Config",                                "yes") ],
        [ (7,   "Controller Map",                               "yes") ],
        [ (8,   "Comm Area",                                    "no") ],
        [ (11,  "Copy NVRAM",                                   "no") ],
        [ (19,  "Resource Manager Config 1",                    "yes") ],
        [ (20,  "Resource Manager Config 2",                    "yes") ],
        [ (21,  "Resource Manager Log 1",                       "yes") ],
        [ (22,  "Resource Manager Log 2",                       "yes") ],
        [ (23,  "Config Journal Directory",                     "yes") ],
        [ (294, "XSSA Persistent Store",                        "no") ],
        [ (295, "Proc DDR Tables",                              "yes") ],
        [ (301, "Target Resource List",                         "yes") ],
        [ (302, "Snapshot FID list",                            "yes") ]
    ];

    $bucket{"Wookiee PAM Logs"} = 
    [
        [ (306, "PAM Logs",                                     "yes") ]
    ];

    $bucket{"Wookiee Linux Logs"} = 
    [
        [ (307, "Linux System Logs",                            "yes") ],
        [ (308, "Linux Raid Logs",                              "yes") ],
#--        [ (312, "750 SMP Phy Info Logs",                        "yes") ]
    ];

    $bucket{"Wookiee Core Summaries"} = 
    [
        [ (309, "Core Summaries",                               "yes") ]
    ];

    $bucket{"Qlogic Core Summaries"} = 
    [
        [ (313, "Qlogic Cores ",                                "yes") ]
    ];
# Just comment this section out so that no file is created at all
#    $bucket{"Wookiee Core Files"} = 
#    [
#        [ (310, "Core Files",                                   "no") ]
#    ];
    
    $bucket{"Wookiee Apps Logs"} = 
    [
        [ (311, "Apps Logs",                                    "yes") ]
    ];
    
    $bucket{"FE Backtrace"} = 
    [
        [ (3,   "Front End NVRAM",                              "yes") ],
        [ (512, "Flight recorder wo timestamps",                "yes") ],
        [ (513, "Flight recorder w timestamps",                 "yes") ],
        [ (514, "MRP trace",                                    "yes") ],
        [ (515, "Defrag trace",                                 "yes") ],
        [ (516, "Error Trap registers",                         "yes") ],
        [ (517, "Error Trap internal ram",                      "yes") ],
        [ (518, "Error Trap NMI counts",                        "yes") ],
        [ (519, "Error Trap internal registers",                "yes") ],
        [ (520, "PROC internal information K_ii",               "yes") ],
        [ (521, "Initiator trace log 0",                        "yes") ],
        [ (522, "Initiator trace log 1",                        "yes") ],
        [ (523, "Initiator trace log 2",                        "yes") ],
        [ (524, "Initiator trace log 3",                        "yes") ],
        [ (525, "FE Trace log 0",                               "yes") ],
        [ (526, "FE Trace log 1",                               "yes") ],
        [ (527, "FE Trace log 2",                               "yes") ],
        [ (528, "FE Trace log 3",                               "yes") ],
        [ (545, "Diagnostic data NVRAM Part 5",                 "yes") ],
        [ (562, "Backtrace data NVRAM Part 1",                  "yes") ]
    ];

    $bucket{"BE Backtrace"} = 
    [
        [ (2,   "Back End NVRAM",                               "yes") ],
        [ (5,   "Back End Work Area",                           "yes") ],
        [ (768, "Flight recorder wo timestamps",                "yes") ],
        [ (769, "Flight recorder w timestamps",                 "yes") ],
        [ (770, "MRP trace",                                    "yes") ],
        [ (771, "Defrag trace",                                 "yes") ],
        [ (772, "Error Trap registers",                         "yes") ],
        [ (773, "Error Trap internal ram",                      "yes") ],
        [ (774, "Error Trap NMI counts",                        "yes") ],
        [ (775, "Error Trap internal registers",                "yes") ],
        [ (776, "PROC internal information K_ii",               "yes") ],
        [ (801, "Diagnostic data NVRAM Part 5",                 "yes") ],
        [ (818, "Backtrace data NVRAM Part 1",                  "yes") ]
    ];

    $bucket{"FE Queues"} = 
    [
        [ (533, "Define exec queue",                            "yes") ],
        [ (537, "ISP 0 request queue",                          "yes") ],
        [ (538, "ISP 1 request queue",                          "yes") ],
        [ (539, "ISP 2 request queue",                          "yes") ],
        [ (540, "ISP 3 request queue",                          "yes") ],
        [ (541, "ISP 0 response queue",                         "yes") ],
        [ (542, "ISP 1 response queue",                         "yes") ],
        [ (543, "ISP 2 response queue",                         "yes") ],
        [ (544, "ISP 3 response queue",                         "yes") ],
    ];

    $bucket{"BE Queues"} = 
    [
        [ (785, "Physical exec queue",                          "yes") ],
        [ (786, "Raid exec queue",                              "yes") ],
        [ (787, "Raid 5 exec queue",                            "yes") ],
        [ (788, "Virtual Device exec queue",                    "yes") ],
        [ (789, "Define exec queue",                            "yes") ],
        [ (790, "Raid init exec queue",                         "yes") ],
        [ (791, "Raid XOR completion exec queue",               "yes") ],
        [ (792, "Raid XOR exec queue",                          "yes") ],
        [ (793, "ISP 0 request queue",                          "yes") ],
        [ (794, "ISP 1 request queue",                          "yes") ],
        [ (795, "ISP 2 request queue",                          "yes") ],
        [ (796, "ISP 3 request queue",                          "yes") ],
        [ (797, "ISP 0 response queue",                         "yes") ],
        [ (798, "ISP 1 response queue",                         "yes") ],
        [ (799, "ISP 2 response queue",                         "yes") ],
        [ (800, "ISP 3 response queue",                         "yes") ],
        [ (802, "Raid error exec queue",                        "yes") ],
        [ (804, "Physical comp queue",                          "yes") ],
        [ (816, "File System exec queue",                       "yes") ]
    ];

    $bucket{"FE PCBs"} = 
    [
        [ (556, "Define exec PCB",                              "yes") ],
    ];

    $bucket{"BE PCBs"} = 
    [
        [ (803, "Raid error exec PCB",                          "yes") ],
        [ (805, "Raid error exec PCB",                          "yes") ],
        [ (808, "Physical exec PCB",                            "yes") ],
        [ (809, "Raid exec PCB",                                "yes") ],
        [ (810, "Raid 5 exec PCB",                              "yes") ],
        [ (811, "Virtual Device exec PCB",                      "yes") ],
        [ (812, "Define exec PCB",                              "yes") ],
        [ (813, "Raid init exec PCB",                           "yes") ],
        [ (814, "Raid XOR completion exec PCB",                 "yes") ],
        [ (815, "Raid XOR exec PCB",                            "yes") ],
        [ (817, "File System exec PCB",                         "yes") ]
    ];

    $bucket{"FE Misc"} = 
    [
        [ (550, "FE IRAM",                                      "yes") ],
        [ (563, "FE FICB",                                      "yes") ]
    ];

    $bucket{"BE Misc"} = 
    [
        [ (807, "BE IRAM",                                      "yes") ],
        [ (819, "BE FICB",                                      "yes") ]
    ];

    return %bucket;
}

#
# Start a command manager
#
sub StartMgr($$)
{
    my ($ip, $port) = @_;
    my $mgr = XIOTech::cmdMgr->new(\*STDOUT);

    if (defined($mgr))
    {
        my $rc = $mgr->login($ip, $port);
        if ($rc)
        {
            print "INFO: Login to ($ip:$port) successful.\n";
            return $mgr;
        }
    }
    
    # Couldn't establish connection
    print "ERROR 1: Login to ($ip:$port) failed!\n";
    $errors++;
    $progRC = RC_LOGIN_FAILED;
    return undef;
}

#
# define the 'run' (system) subroutine
#
sub run($) 
{
    my ($cmd) = @_;
    print "> $cmd\n";
    my $rc = (system "$cmd") >> 8;
    if ($rc) 
    {
        print "ERROR 2: \"$cmd\" FAILED with return code $rc\n";
        $progRC = RC_SHELL_CMD_FAILED;
        $errors++;
    }
    return $rc;
}

#
# Append a string to a file
#
sub AppendFile($$)
{
    my ($filename, $string) = @_;

    if (open F, ">>$filename")
    {
        print F "$string";
        close F;
    }
    else
    {
        print "ERROR 3: Couldn't open $filename.\n";
        $progRC = RC_FILE_OPEN_FAILED;
        $errors++;
    }
}


#
# Process the bucket-O-fids. 
#
my $sep = "==================================================\n";
sub ProcessFidBucket($\@$$$)
{
    my ($bucketName, $pFidArray, $fwVers, $mode, $option) = @_;
    my ($inputDir, $manager);
    my ($fid, $fidName, $fidInclude);
    my $pData;
    my $procLog = 0;
    my $rc;

    if ($mode !~ /^dump$|^decode$/i) 
    {
        print "ERROR 4: Mode must be \"dump\" or \"decode\"\n";
        $progRC = RC_PARAMETER_ERROR;
        $errors++;
        return;
    }
    else
    {
        $inputDir = $option if $mode =~ /decode/i;
        $manager = $option  if $mode =~ /dump/i;
    }

    unlink "$bucketName";
    unlink "261.tmp";
    unlink "262.tmp";

    print "BUCKET NAME: \"$bucketName\"\n";

    for (my $i = 0; $i < @$pFidArray; $i++)
    {
        $fid        = @$pFidArray[$i]->[0];
        $fidName    = @$pFidArray[$i]->[1];
        $fidInclude = @$pFidArray[$i]->[2];
        
        # Print the tag name to the file
        my $tag;
        ($tag = $fidName) =~ s/ /_/g;
        $tag .= "_$fid";
        AppendFile($bucketName, 
                "\n\n".
                "$sep"."$tag(){}\n"."$sep");

        # Read the data
        undef $pData;
        
        #
        # Read the fids from the directory 
        #
        if ($inputDir)
        {
            my @files = glob("$inputDir/*FID_$fid.bin");
            if (@files == 1)
            {
                print "$sep"."Reading up FID $fid - $fidName:\n".
                        "  $files[0]\n";
                
                if ( ($fid >= 305) && ($fid <= 352) )
                {
                    my $fileData = $files[0];
                    $pData = \$fileData;
                }
                elsif (open IN, "$files[0]")
                {
                    binmode IN;
                    my $oldRS = $/;
                    undef $/;
                    my $fileData = <IN>; # slurp in the whole file
                    close IN;
                    $/ = $oldRS;
                    $pData = \$fileData;
                }
                else
                {
                    print "ERROR 3: Couldn't open $files[0].\n";
                    $progRC = RC_FILE_OPEN_FAILED;
                    $errors++;
                }
                
                # Remove it from the "fileList" hash
                delete $fileList{$fid};
            }

            if (!defined($pData))
            {
                print "$sep";
                print "WARNING 1: Couldn't find FID $fid!\n";
                $warnings++;
            }
        }

        #
        # Else go fetch the fids in real time from the controller
        #
        elsif ($manager)
        {
            if ($fidInclude =~ /y/i)
            {
                print "$sep"."Fetching FID $fid - $fidName\n";
                my %data = $manager->MPXReadFID($fid);

                if (%data and $data{STATUS} == 0)
                {
                    $pData = \$data{RD_DATA};
                }
            }
            else
            {
                print "$sep"."Skipping FID $fid - $fidName\n";
            }
        }

        #
        # Common section: now that we have the fid data, process it through
        # the fid decoders.
        #
        if ($pData and length($$pData) > 0)
        {
            if ($fid == 261 or $fid == 262)
            {
                open LOG, ">$fid.tmp" or warn "Couldn't create $fid.tmp\n";
                binmode LOG;
                print LOG $$pData;
                close LOG;
                $procLog = 1;

                # Cleanup previous copies of the flash data if it exists
                unlink "CustomerLogs_FID_261.bin" if $fid == 261;
                unlink "DebugLogs_FID_262.bin" if $fid == 262;
            }
            else
            {
                $rc = CCBEDecodeFids ($$pData, $fid , ">$bucketName",$manager);
                if ($rc != GOOD)
                {
                    print "ERROR 5: Couldn't decode FID $fid - $fidName\n";
                    $progRC = RC_DECODE_FAILED;
                    $errors++;
                }
            }

            if ($fid == 296)
            {
                my ($hdrMagicNum) = unpack DDR_FID_HEADER, $$pData;
                my $offset = 92;

                if ($hdrMagicNum == DDR_FID_HEADER_MAGIC_NUM)
                {
                    $offset += 32;
                }
                $sysRel = unpack "x$offset A4", $$pData;
                print "INFO: System Release = $sysRel\n";
            }
        }
        else
        {
            AppendFile($bucketName, 
                    "Fid was not found or length was 0. No data to format.\n\n");
        }
    }

    #
    # For the special case of the log files, process them here, if found.
    #
    if ($procLog)
    {
        my $opt261 = "";
        my $opt262 = "";
        
        if (-r "261.tmp")
        {
            $opt261 = "-c261.tmp";
        }
        
        if (-r "262.tmp")
        {
            $opt262 = "-d262.tmp";
        }
        
        #
        # Locate LogSim.exe
        #
        my $foundLogsim = 0;
        if ($opt_l)
        {
            LOGSIM: 
            {
                # Is this a path directly to LogSim.exe?
                $logSim = "$opt_l";
                print "1: Looking for LogSim: $logSim ... ";
                if (-d $logSim)
                {
                    # nope, go on
                    print "nope\n";
                }
                elsif (-x $logSim)
                {
                    print "found it\n";
                    $foundLogsim = 1;
                    last LOGSIM;
                }
                
                # Else, can we find it down the SysRel path?
                $logSim = "$opt_l/$sysRel/LogSim.exe";
                print "2: Looking for LogSim: $logSim ... ";
                if ($sysRel and -x $logSim)
                {
                    print "found it\n";
                    $foundLogsim = 1;
                    last LOGSIM;
                }
                else
                {
                    print "nope\n";
                }
                
                # Or is it simply in the dir specified?
                $logSim = "$opt_l/LogSim.exe";
                print "3: Looking for LogSim: $logSim ... ";
                if (-x $logSim)
                {
                    print "found it\n";
                    $foundLogsim = 1;
                    last LOGSIM;
                }
                else
                {
                    print "nope\n";
                }
            }
        }
        else
        {
            $logSim = "$progDir/LogSim.exe";
            if (-r "$logSim")
            {
                $foundLogsim = 1;
            }
	    else
	    {
		$logSim = "$progDir/../CCB/LogSim/obj_7000/logsim";
		if (-x "$logSim")
		{
		    $foundLogsim = 1;
		}
		else
		{
		    $logSim = "$progDir/../CCB/LogSim/obj_3000/logsim";
		    if (-x "$logSim")
		    {
			$foundLogsim = 1;
		    }
		    else
		    {
			print "Can not find LogSim ...\n";
		    }
		}
	    }
        }
        
        if ($foundLogsim)
        {
            print "LogSim   = $logSim\n";
            
            run "$logSim process -e $opt261 $opt262 -f$bucketName";
            AppendFile($bucketName, 
                    "\nNote: These logs were decoded with \"$logSim\"\n");
                
#            unlink "261.tmp";
#            unlink "262.tmp";
            unlink "logsimdata";
            unlink "logsimdatadebug";
        }
        else
        {
            print "ERROR 6: Couldn't find LogSim.exe!\n";
            
            $progRC = RC_LOGSIM_NOT_FOUND;
            $errors++;
        }
        
        # In any case, keep the log bin files around
        my $tmpName;
        if (-r "261.tmp")
        {
            $tmpName = BucketName("CustomerLogs_FID_261", "bin");
        
            print "Saving $tmpName\n";
            rename "261.tmp", "$tmpName";
        }
        if (-r "262.tmp")
        {
            $tmpName = BucketName("DebugLogs_FID_262", "bin");
        
            print "Saving $tmpName\n";
            rename "262.tmp", "$tmpName";
        }
    }
    
    #
    # Output a final seperator
    #
    print "$sep";
}

#
# Given an informal bucket name, make it into the real file name
#
sub BucketName($$)
{
    my ($bucketName, $ext) = @_;
    
    $bucketName =~ s/ /_/g;
    if ($opt_s)
    {
        $bucketName .= "_"."$opt_s";
    }
    if ($opt_p)
    {
        $bucketName = "$opt_p"."_".$bucketName;
    }
    $bucketName .= ".$ext";

    return $bucketName;
}

#
# Convert an arbitrary path to an absolute path, based off
# of the starting directory.
#
sub GetAbsolutePath
{
    my ($path) = @_;

    $path =~ s|\\|/|g; # \'s to /'s

    # Absolute paths begin with:  x:/... or /... 
    # everything else would be relative.
    if ($path =~ /^[a-z]{1,1}:\//i or $path =~ /^\//i )
    {
        # absolute path; OK as is
    }
    else
    {
        $path = "$startDir/$path";
    }

    return $path;
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
# This is where main() starts. 
# Parse the input parameters.
#
my $startTime = time();

#
# Get the command line args
#
getopts('fhl:o:p:s:');
    
my $script;
($script = $0) =~ s|\\|/|g; # \'s to /'s
$script =~ s|^.*/||;

#
# This is a lot of monkeying around to get the program directory...
# There must be a better way.
#
($progDir = $0) =~ s|\\|/|g;
$progDir =~ m|^.*/|;
$progDir = $&;
if (! $progDir)
{
    $progDir = "$startDir";
}
$progDir = GetAbsolutePath($progDir);
$progDir =~ s|/$||;

# Extract the version number from this file
my $buildTime = "TIMESTAMP"; # DO NOT CHANGE THIS LINE -- IT MUST REMAIN SET TO 
                             # 'TIMESTAMP', AS IT IS PATCHED AT .EXE BUILD TIME!

my $version = q$Revision: 144092 $;
print 
"---------------------------------------\n".
"             $script\n".
"Copyright 2000-2009 Xiotech Corporation\n".
"        For Internal Use Only          \n".
"             $version\n";
if ($buildTime !~ /TIMESTAMP/)
{
    print "        $buildTime\n";
}
print 
"---------------------------------------\n";

if (@ARGV < 1 or $opt_h) 
{ 
    ShowHelp();
}

#
# Check for proper amount of command line parameters
#
if ($script =~ /SnapDump.pl/i)
{
    print "\n\n".
        "!!**********************************************************!!\n".
        "!!                                                          !!\n".
        "!!  NOTE: Please run SnapDump.exe instead of $script    !!\n".
        "!!        to gather the controller's snapshot data.         !!\n".
        "!!        SnapDump.exe can be found in the corresponding    !!\n".
        "!!        ...\\Release\\...\\Test\\Executables directory        !!\n".
        "!!        on \\\\rststore\\private.                            !!\n".
        "!!                                                          !!\n".
        "!!**********************************************************!!\n".
        "\n\n";

    if (! $opt_f and ! AskForYesNo "Do you want to continue with SnapDump.pl")
    {
        exit 1;
    }
}

sub ShowHelp
{
    print "\n".
          "Usage: $script [-h] [-p prefix] [-s suffix] [-l LogSim-Dir] [-o Output-Dir] ".
          "IP-Addr:port =OR= Input-Dir \n".
          "\n".
#          "       -a            - In IP-Addr mode: grab ALL fids. By default, only\n".
#          "                         the fids listed in fid 302 (if available) are\n".
#          "                         retrieved.\n".
          "       -h             - this help text\n".
          "       -l LogSim-Dir  - path to logsim.exe or directory of where LogSim.exe\n".
          "                        can be found.  This can be specified in several ways:\n".
          "                         - the full path to LogSim.exe\n".
          "                         - the directory path of where LogSim.exe is found\n".
          "                         - the base directory path to where sub-dir's of\n".
          "                           various system release versions can be found:\n".
          "                           example: c:/mydir/0130\n".
          "                                    c:/mydir/0200\n".
          "                                    c:/mydir/0250\n".
          "                           'c:/mydir' is passed in; once the system release\n".
          "                           of the snapshot is determined, it looks in\n".
          "                           'c:/mydir/NNNN' for LogSim.exe where NNNN is the\n".
          "                           system release level.\n\n".
          "                           Note: if '-l' is omitted, the directory where\n".
          "                           $script is located is searched for LogSim.exe.\n".
          "       -o Output-Dir - directory where the output files will be written.\n".
          "       -p prefix     - Prefix prepended to the output file name.\n".
          "       -s suffix     - Suffix appended to the output file name.\n".
          "       IP-Addr       - address of the system you want to dump.\n".
          "                       :port - (optional) port number.\n".
          "         =OR=\n".
          "       Input-Dir     - directory where the snapshot files can be found.\n".
          "\n";
	exit 1; # this is the ONLY EARLY exit! Let's keep it that way.
}

print "startDir = $startDir\n";
print "progDir  = $progDir\n";

#
# If full path to LogSim.exe supplied, verify it exists -- exit if not.
# This should not interfere with the operation of the snap server, it always
# calls with a "base" directory.
#
if ($opt_l)
{
    $opt_l = GetAbsolutePath($opt_l);

    if (! -x $opt_l)
    {
        # OK here's another early exit...
        die "\nCouldn't find LogSim.exe (or its equivalent)!\n";
    }
}

#
# Create the output directory.
# Make sure there are NO CHDIR's called before this point!
#
my $outDir;
if ($opt_o)
{
    $outDir = GetAbsolutePath($opt_o);
    mkdir $outDir;
    # And yet another early exit... (but that's it :-)
    chdir $outDir or die "\nCouldn't chdir $outDir\n";
}
else
{
    if (AskForYesNo "You did not supply an output directory, ".
        "do you want to continue anyway?")
    {
        $outDir = "$startDir";
    }
    else
    {
        # AYAEE! (And Yet Another Early Exit) (but that's *really* it :-)
        die "\nPlease supply an output directory with '-o'!\n";
    }
}
print "outDir   = $outDir\n";

#
# Build up the fid hash
#
my %fidHash = BuildFidBucketHash();
my $bucket;
my $argType = "unknown";


print "\n$sep";

#
# Loop through directories and or controllers
#
while(@ARGV)
{
    my $arg0 = shift @ARGV;

    #
    # Check to see if the input is an IP address or a directory
    #
    if (($arg0 =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/) or
            ($arg0 =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}:\d{1,5}?$/)) # IP address?
    {
        $argType = "ipaddress";
    }
    else
    {
        $arg0 = GetAbsolutePath($arg0);
    
        if (-d $arg0)
        {
            $argType = "directory";
        }
    }

    if ($argType eq "ipaddress")
    {
        #
        # If the port number was passed it was done with a colon separator
        #
        my ($ip, $port) = split /:/, $arg0;

        if (!defined($port))
        {
            # If a port number was not passed on the command line default to 3000
            $port = 3000;
        }

        #
        # Connect to target system
        #
        my $mgr = StartMgr($ip, $port);

        if (!defined($mgr))
        {
            $errors++;
            last;
        }

        foreach $bucket (sort keys(%fidHash))
        {
            my $pFid2dArray = $fidHash{$bucket};
            ProcessFidBucket(BucketName($bucket, "dmp"), @$pFid2dArray, 0, "dump", $mgr);
        }
    }
    elsif ($argType eq "directory")
    {
        #
        # Build up a list of all of the files in the directory that are "FIDs"
        #
        my @allFiles = glob("$arg0/*-FID_*");
        my $fidName;
        my $fidNum;
        foreach $fidName (@allFiles)
        {
            $fidName =~ m/FID_/;
            $fidNum = "$'";
            $fidNum =~ s/\.bin//i;

            $fileList{$fidNum} = $fidName;
        }

        foreach $bucket (sort keys(%fidHash))
        {
            my $pFid2dArray = $fidHash{$bucket};
            ProcessFidBucket(BucketName($bucket, "dmp"), @$pFid2dArray, 0, "decode", $arg0);
        }

        if (keys(%fileList))
        {
            my @miscArray;
            foreach $fidNum (keys(%fileList))
            {
                push @miscArray, [($fidNum, "Unknown FID", "no")];
            }
            ProcessFidBucket(BucketName("Unknown FIDs", "dmp"), @miscArray, 0, "decode", $arg0);
        }
    }
    else 
    {
        print "ERROR 4: You must specify either an IP address or a directory!\n";
        $progRC = RC_PARAMETER_ERROR;
        $errors++;
    }

    last; # don't loop -- not supported yet.
}

print "\nProcessing time: " . (time() - $startTime) . " seconds.\n\n";

print "WARNINGS: $warnings\n";
print "ERRORS:   $errors\n\n";
exit $progRC;

#########################
# 
#  TODO:
#
#  --  Allow multiple controllers/directories on command line (maybe).
#         This gets troublesome because the output files get overwritten...
#
#  --  When interactive, optionally read the Fid-o-fids, and only
#         grab those fids.  Right now we grab everything in the list,
#         (based upon the "yes/no" in the 3rd field).
#
#########################
