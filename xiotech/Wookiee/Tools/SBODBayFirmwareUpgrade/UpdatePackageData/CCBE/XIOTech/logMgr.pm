# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Chris Nigbur
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

1;

##############################################################################
#
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:56  RysavyR
# Initial revision
#
# Revision 1.10  2003/04/04 19:23:57  RysavyR
# Prune back the log file to keep its size reasonable.
# Rev by BryanH.
#
# Revision 1.9  2002/10/01 19:01:27  RysavyR
# TBolt00006013:  Add the ability to handle and process BF style packets on
# the X1 port. Reviewed by TimSw.
#
# Revision 1.8  2002/04/23 15:53:07  McmasterM
# TBolt00000000: Remove log-to-screen when output file is not specified.
# Reviewed by Chris Nigbur
#
# Revision 1.7  2001/11/29 16:33:54  NigburC
# Added options to logStart to allow text to be appended to the log file name
# to make it unique automatically.  We can append a timestamp or a process
# ID.
#
# Added code to use the TMP or TEMP environment variables as the path
# for the log file if they are available.
#
# Revision 1.6  2001/11/26 16:10:23  RysavyR
# Fixed bug in call to open (2 parameters, not 3)
#
# Revision 1.5  2001/11/07 13:51:13  NigburC
# Change the usage of the ID tag to HEADER tag.
#
# Revision 1.4  2001/11/02 20:35:35  NigburC
# Code cleanup time, lots of changes.
#
# Added code handling for the 3100 port and new packet interface.  BigfootCL
# is now able to connect to a machine running the new packet interface and
# send a pdiskCount command.
#
# Revision 1.3  2001/11/01 13:19:08  NigburC
# Changed the log file name generation to not use the process id.  This
# will then generate one log file for the command line.
#
# Revision 1.2  2001/10/31 15:42:02  NigburC
# Updated the command line to include the "logInfo" command to display
# the last N log messages.
#
# Revision 1.1.1.1  2001/10/31 12:51:30  NigburC
# Initial integration of Bigfoot command line.
#
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
