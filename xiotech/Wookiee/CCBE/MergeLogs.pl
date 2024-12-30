#!perl -w
#====================================================================
#
# FILE NAME:    MergeLogs.pl
#
# AUTHOR:       Bryan Holty
#
# DATE:         9/11/2003
#
# DESCRIPTION:  Merges logfiles from 2 to N controllers together
#               by date/time.
#
#====================================================================

use IO::Handle;
use	Getopt::Std;
use Time::Local;
use POSIX;

use strict;

our $opt_l=0;
our $opt_s=0;
our $opt_S=0;

#   get the options
getopts('lsS');

if ($opt_S)
{
    $opt_s = 1;
}

my $timeTaken = time();
my $numFiles = scalar(@ARGV) - 1;
my @fileHandles;
my @lineCompares;
my @timeStamps;
my @filesRemaining;
my $outputFileHandle;
my $logcount = 0;
my @statsArrHash;
my @statsArrTimeHash;

my $version = q$Revision: 4298 $;

if (@ARGV>=2)
{
# Open output file
    my $outputFile = shift(@ARGV);
    open $outputFileHandle, ">$outputFile" or die "Can't open output file: $outputFile";
    print "Opened $outputFile for output.\n";
    
# Open and Initialize input files
    for (my $i = 0; $i < $numFiles; ++$i)
    {
        my $fileName = shift(@ARGV);
        open $fileHandles[$i], $fileName or die "Can't open file: $fileName.";
        $filesRemaining[$i] = $i;
        print "Opened $fileName for processing.\n";

        if ( !($lineCompares[$i] = readline($fileHandles[$i])))
        {
           removeRemainingFile($i);
        }
        else
        {
            if (isNewLogLine($lineCompares[$i]))
            {
                $timeStamps[$i] = getTimeStamp($lineCompares[$i]);
            }
            else
            {
                die "$fileName has improper formatting.";
            }
        }
    }

# Process Files
    print "Merging Logs.\n";
    while (my $line = getNextLogMessage())
    {
        print $outputFileHandle $line;
    }
    print "Merging Logs Finished:  $logcount logs processed.\n";

# Close Files
    for (my $i = 0; $i < $numFiles; ++$i)
    {
        close $fileHandles[$i];
    }
    
    if ($opt_s)
    {
        print "Processing Statistics.\n";
        print $outputFileHandle "\n\n";
        processStatistics($outputFileHandle);
        print "Processing Statistics Finished.\n";
    }

    close $outputFileHandle;
    
    printf "MergeLogs Finished %d(sec).\n", (time() - $timeTaken);
}

else
{
    my $script;
    ($script = $0) =~ s|\\|/|g;
    $script =~ s|^.*/||;
    
    print 
    "=========================================\n".
    "              $script                   \n".
    " Copyright 2000-2004 XIOtech Corporation\n".
    "         For Internal Use Only          \n".
    "             $version                   \n".
    "=========================================\n";
    
    print "Usage: $script [-s] [-S] [-l] out cn0 [cn1..]\n" .
          "    -s        Count Statistics\n" .
          "    -S        Super Statistics (Takes some time)\n" .
          "    -l        Use Local Time (default GMT)\n" .
          "    out       Output File\n" .
          "    cn0       Controller 0\n" .
          "    cn1...    Controllers 1 - n\n";
}


##############################################################################
# Name:     getNextLogMessage
#
# Desc:     Gets the next log message in time sequence.
#
# Input:    None
#
# Returns:  Log message if there is one
#           undef if there are no more logs
##############################################################################
sub getNextLogMessage
{
    my $logMsg;
    my $smallTime = 0xFFFFFFFF;
    my $smallTimeIdx = 0;

    if (scalar(@filesRemaining) == 0)
    {
        return undef;
    }
    
    if (scalar(@filesRemaining) >= 2)
    {
        for (my $i = 0; $i < scalar(@filesRemaining); ++$i)
        {
            if ( $timeStamps[$filesRemaining[$i]] < $smallTime )
            {
                $smallTimeIdx = $filesRemaining[$i];
                $smallTime = $timeStamps[$smallTimeIdx];
            }
        }
    }
    else
    {
        $smallTimeIdx = $filesRemaining[0];
    }
    
    $logMsg = getLogMessage($smallTimeIdx);
    
    return $logMsg;    
}

##############################################################################
# Name:     getLogMessage
#
# Desc:     Gets an entire log message.
#
# Input:    controller number (index to fileHandles)
#
# Returns:  Log message
##############################################################################
sub getLogMessage
{
    my ($cn) = @_;
    my $logMsg = $lineCompares[$cn];
    
    if ($opt_l)
    {
        my $newTime = strftime "%X %m/%d/%Y", localtime($timeStamps[$cn]);
        $newTime =~ s/\sAM/am/;
        $newTime =~ s/\sPM/pm/;
        $logMsg =~ s/\d\d:\d\d:\d\d\w\w\s\d\d\/\d\d\/\d\d\d\d/$newTime/;
    }
    
    while ($lineCompares[$cn] = readline($fileHandles[$cn]))
    {
        if (isNewLogLine($lineCompares[$cn]))
        {
            $timeStamps[$cn] = getTimeStamp($lineCompares[$cn]);
            last;
        }
        else
        {
            $logMsg .= "    $lineCompares[$cn]";    
        }
    }
    
    if (!($lineCompares[$cn]))
    {
        removeRemainingFile($cn);
    }
    
    ++$logcount;
    
    if ($opt_s)
    {
        gatherStatistics($cn, $logMsg);    
    }

    return "CN" . $cn . " " .$logMsg;    
}

##############################################################################
# Name:     isNewLogLine
#
# Desc:     Tells whether the line passed in is the start of a new log message.
#
# Input:    line to check
#
# Returns:  1 if line is a new log message
#           0 if line is not a new log message
##############################################################################
sub isNewLogLine
{
    my ($line) = @_;

    if ($line =~ /^[0-9]*\s[\S|\s]\s[0-9]*\s\d\d:\d\d:\d\d\w\w\s\d\d\/\d\d\/\d\d\d\d/)
    {
        return 1;
    }
    
    return 0;
}

##############################################################################
# Name:     removeRemainingFile
#
# Desc:     remove controller (index to fileHandles) from the 
#           filesRemaining array.
#
# Input:    controller (index to fileHandles)
#
# Returns:  None
##############################################################################
sub removeRemainingFile
{
    my ($idx) = @_;

#    print "removeRemainingFile $idx\n";

    for (my $i = 0; $i < scalar(@filesRemaining); ++$i)
    {
        if ( $idx == $filesRemaining[$i])
        {
            $filesRemaining[$i] = $filesRemaining[0];
            shift @filesRemaining;
            last;
        }
    }
}

##############################################################################
# Name:     getTimeStamp
#
# Desc:     Get a GMT timestamp for the time in new log line.
#
# Input:    line of a new log message
#
# Returns:  long timestamp
##############################################################################
sub getTimeStamp
{
    my ($tm) = @_;
    
    $tm =~ s/^[0-9]*\s[\S|\s]\s[0-9]*\s//;		        # Strip out the env data
    $tm = substr($tm, 0, 21);

    my $newTm;
    my $tmDf = 0;
    my @splitTmDt = split / /, $tm;
    my @spltTm = split /:/, $splitTmDt[0];
    
    my ($lhour, $lmin, $lsec) = @spltTm;
    $lsec = substr($lsec,0,2);
    
    my ($lmon, $lmday, $lyear)= split /\//, $splitTmDt[1];
    
    my $gmTm = timegm($lsec,$lmin,$lhour,$lmday,($lmon-1),$lyear);
    
    if ((substr($spltTm[2],2,3) eq "am") && ($lhour == 12))
    {
        $gmTm -= 43200; #12 hours
    }
    elsif ((substr($spltTm[2],2,3) eq "pm") && ($lhour != 12))  
    {
        $gmTm += 43200; #12 hours
    }
    
   return $gmTm;
}

##############################################################################
# Name:     gatherStatistics
#
# Desc:     Gathers statistics.
#
# Input:    cn, logMsg
#
# Returns:  None
##############################################################################
sub gatherStatistics
{
    my ($cn, $logMsg) = @_;
    my $eventCode;
    my $status;
    my $timeStamp;
    my $shortLogMsg;


    # Retrieve the timestamp, event code, status, and short log message.
    $logMsg =~ /(\d\d:\d\d:\d\d\w\w\s\d\d\/\d\d\/\d\d\d\d)\s*(\S*)\s*(\S*)\s*(.*)\n/;
    $timeStamp = $1;
    $eventCode = $2;
    $status = $3;
    $shortLogMsg = $4;
    
    # Global Statistic counts.
    ++$statsArrHash[0xFF]{$eventCode};
    ++$statsArrHash[0xFF]{$status};

    # Controller Statistic counts.
    ++$statsArrHash[$cn]{$eventCode};
    ++$statsArrHash[$cn]{$status};
    
    if ($opt_S)
    {
        # Global Statistic timelines.
        push @{$statsArrTimeHash[0xFF]{$eventCode}}, "$timeStamp $shortLogMsg";

        # Controller Statistic timelines.
        push @{$statsArrTimeHash[$cn]{$eventCode}},  "$timeStamp $shortLogMsg";
    }
}

##############################################################################
# Name:     processStatistics
#
# Desc:     Processes statistics.
#
# Input:    None
#
# Returns:  statistics string
##############################################################################
sub processStatistics
{
    my ($out) = @_;
    my $hashKeys;
    my @tmpArr;

    print $out "Statistics\n";
    printf $out "%10s", "Identifier";
    for (my $i = 0; $i < $numFiles; ++$i)
    {
        printf $out "     %6s", "CN$i"; 
    }

    printf $out "     %6s", "Total";
    print $out "\n";
    
    print $out "----------";
    for (my $i = 0; $i < $numFiles; ++$i)
    {
        printf $out "     %6s", "------"; 
    }

    printf $out "     %6s", "------";
    print $out "\n";

    foreach $hashKeys (sort(keys %{$statsArrHash[0xFF]}))
    {
        printf $out "%10s", $hashKeys;
        
        for (my $i = 0; $i < $numFiles; ++$i)
        {
            if (exists $statsArrHash[$i]{$hashKeys})
            {
                printf $out "     %6d", $statsArrHash[$i]{$hashKeys};
            }
            else
            {
                printf $out "     %6d", 0;
            }
        }

        printf $out "     %6d", $statsArrHash[0xFF]{$hashKeys};

        print $out "\n";
    }
    
    if ($opt_S)
    {

        print $out "\n\nStats for Each code\n";

        for (my $i = 0; $i < $numFiles; ++$i)
        {
            printf $out "  CN" . $i . "\n";
        
            foreach $hashKeys (sort(keys %{$statsArrTimeHash[$i]}))
            {
                print $out "    $hashKeys\n";
                @tmpArr = @{$statsArrTimeHash[$i]{$hashKeys}};
            
                for (my $j = 0; $j < scalar(@tmpArr); ++$j)
                {
                    print $out "      $tmpArr[$j]\n";
                }
            }
        
            print $out "\n";
        }
    }
}
