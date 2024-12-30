# $Id: logMgr.pm 144092 2010-07-13 21:40:51Z m4 $
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
#
# Purpose:
#   Functions to log information to a file.
##############################################################################
package XIOTech::logMgr;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
    logStart
    logStop
    logMsg
);

use Time::localtime;

use strict;

my $logFileName;
my $logFileOpen;

##############################################################################
# NAME:     logStart
#
# DESC:     Starts a log manager by opening a file for writing.
#
# INPUT:    NAME    - Name of the file to use for logging.
#           OPTION  - Options for conditionally appending text to the
#                     log file name.
#                     0 = append nothing (default if not specified)
#                     1 or TS = append a timestamp value (_YYYYMMDD_HH_MM_SS)
#                     2 or PID = append the process ID
# OUTPUT:   NONE
##############################################################################
sub logStart
{
    my ($name, $option) = @_;

    my $fullname = $name;

    my $tmp = "";

    if ($ENV{TMP})
    {
        $tmp = $ENV{TMP};

        if ((substr($tmp, (length $tmp) - 1) cmp "\\") != 0)
        {
            $tmp .= "\\";
        }
    }
    elsif ($ENV{TEMP})
    {
        $tmp = $ENV{TEMP};

        if ((substr($tmp, (length $tmp) - 1) cmp "\\") != 0)
        {
            $tmp .= "\\";
        }
    }

    $fullname = $tmp . $fullname;

    if (!defined($option))
    {
        $option = 0;
    }

    if ($option eq "1" || $option eq "TS")
    {
        my ($sec,
            $min,
            $hour,
            $mday,
            $mon,
            $year,
            $wday,
            $yday,
            $isdst) = CORE::localtime(time);

        my $ts = sprintf "_%d%d%d_%d_%d_%d",
                            $year + 1900,
                            $mon + 1,
                            $mday,
                            $hour,
                            $min,
                            $sec;

        $fullname .= $ts;
    }
    elsif ($option eq "2" || $option eq "PID")
    {
        my $proc = $$;
        $fullname .= "-$proc";
    }

    $fullname .= ".log";
    my $rc = open(LOG, ">>$fullname");

    if ($rc)
    {
        LOG->autoflush(1);

        $logFileName = $fullname;
        $logFileOpen = 1;

        print LOG "=========================================================\n";
        print LOG "Starting log: " . ctime() . "\n";
        print LOG "=========================================================\n";
    }
    else
    {
        print "ERROR: Unable to open log file.  Log messages will be ";
        print " displayed to standard output.\n";
        $logFileName = "STDOUT";
    }

    return $logFileName;
}

##############################################################################
# NAME:     logStop
#
# DESC:     Stops the log manager by closing the log file.
##############################################################################
sub logStop
{
    if ($logFileOpen)
    {
        print LOG "=========================================================\n";
        print LOG "Ending log: " . ctime() . "\n";
        print LOG "=========================================================\n";
        print LOG "\n\n";

        close LOG;
        $logFileOpen = 0;
    }

    undef $logFileName;
}

##############################################################################
##############################################################################
use constant LOG_FILE_PRUNE_SIZE => 1000000;
use constant LOG_FILE_MAX_SIZE =>   (LOG_FILE_PRUNE_SIZE + 1000000);
sub logMsg
{
    my ($msg, $msg2) = @_;

    my ($package, $filename, $line, $subroutine) = caller(1);

    if ($msg eq "XIOTech::logMgr")
    {
        $msg = $msg2;
    }

    my $prefix;

    if ($logFileOpen)
    {
        #
        # Prune back the log file to keep it under 2M.
        #
        if (-e $logFileName && -s $logFileName > LOG_FILE_MAX_SIZE)
        {
            my $pruneSz = 0 - LOG_FILE_PRUNE_SIZE;
            my $saveDat;
            
            print STDOUT "pruning $logFileName...";
            close(LOG);
            
            open(LOG, "$logFileName") or warn;
            seek(LOG, $pruneSz, 2) or warn;
            read(LOG, $saveDat, LOG_FILE_PRUNE_SIZE) or warn;
            close(LOG);
            
            unlink($logFileName) or warn;
            
            open(LOG, ">>$logFileName") or warn;
            LOG->autoflush(1);
            print LOG $saveDat;

            print STDOUT "done\n";
        }

        $prefix = ctime() . " ";

        if (defined($subroutine))
        {
            $prefix .= "$subroutine";
        }

        if (defined($line))
        {
            $prefix .= ":$line ";
        }

        my $numTabs = ((76 - length($prefix)) + 3)/4;

        print LOG "$prefix";
        if(int($numTabs) > 0) {
            print LOG "\t" x int($numTabs);
        }
        print LOG "$msg";
    }
}

##############################################################################

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
