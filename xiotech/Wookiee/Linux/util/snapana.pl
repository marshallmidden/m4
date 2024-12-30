#!/usr/bin/perl -w
# $Id: snapana.pl 143007 2010-06-22 14:48:58Z m4 $

###
# snapana.pl - Add symbolic information to snapdump file Wookiee_Core_Summaries.dmp.
#
#### NOTE ####
# This executable is found via Wookiee/Makefile example, in this directory.
#
# Marshall Midden, 2006-10-05
#
# Copyright (c) 2006 Xiotech Corporation. All rights reserved.
#

=head1 NAME

snapana.pl - Add symbolic information to combined files in snapdump
file Wookiee_Core_Summaries.dmp, making up to six separate files starting with
"PROCANA.".

=head1 SYNOPSIS

 snapana.pl [-v] [--version] [-p Prefix] [--prefix=Prefix] SnapDump_Directory [Release_Directory]

=head1 OPTIONS

 -v                   Printout version and exit.

 --version            Printout version and exit.

 -p Prefix            Prefix for input/output file names.

 --prefix=Prefix      Prefix for input/output file names.

                      Note: automatic underscore put after
                      the Prefix and before file names.

 SnapDump_Directory   Directory containing the snapdump.

 Release_Directory    Location of release directory.

=head1 DESCRIPTION

snapana.pl creates files for ccbrun.txt, ccbrun.hist, Front.t.txt,
Front.t.hist, Back.t.txt, and Back.t.hist and then runs procana.pl on
the separated files, creating potentially six output files with "PROCANA."
prefixing them. If the --prefix option is specified, that is placed in
front of the "PROCANA." with an underscore in between the two.

=head1 EXAMPLES

 ./snapana.pl /tmp/snapdump6 ~/Wookiee/built-750Debug

 ./snapana.pl ./ ~/projects/Wookiee/built-3000Perf

 ./snapana.pl . /k/Release/750/FROSTEDFLAKES/f118/built-750Perf

 ./snapana.pl .

The above attempts to figure out model and version from snapdump. The
files CCB_Currenttrace.dmp and CCB_Logs.dmp are searched. It uses /k on
wookieebuildpc to find the release. There are many assumptions. If
it cannot figure out where the release directory is, it uses the
directory of the snapana.pl script being run. Engineering release use
the directory of the snapana.pl script.

 ./snapana.pl -p CN1 ~/cqt12345

The above has a prefix of "CN1" (underscore automatically added)
to the file names in directory ${HOME}/cqt12345.

=head1 SEE ALSO

procana.pl

=cut

#-----------------------------------------------------------------------------
use strict;
use integer;
use Cwd;
use Getopt::Long;

#-----------------------------------------------------------------------------
# These are the "release" directories in order. "h" translates to HAM.
my @Releases = ('ALPHABITS', 'BACON', 'CHEERIOS', 'DONUTS', 'EGGS',
                'FROSTEDFLAKES', 'GRAPEFRUIT', 'HAM', 'ICECREAM', 'JAM',
                'KRISPYKREMES', 'LUCKYCHARMS', 'MUESLI', 'NUTNHONEY',
                'OMLETTE', 'PANCAKES', 'QUAKEROATS', 'RICECHEX', 'SPECIALK',
                'TOAST', 'U?', 'V?', 'WAFFLES', 'X?', 'YOGHURT', 'Z?');

#-----------------------------------------------------------------------------
my $binary_ccbrun;
my $binary_Front;
my $binary_Back;
my $procana;
my $line;
my $hfile;
my $hfilesuffix;
my $SnapDump_Directory;
my $Release_Directory;

# Argument variables.
our ($opt_p, $opt_v);
$opt_p = '';

#-----------------------------------------------------------------------------
# Figure out path the the command being executed.
my $command = $0;
if (substr($command,0,1) ne '/')        # if not absolute path
{
    my $cwd = cwd();
    $command = $cwd . '/' . $command;
    $command =~ s|/\./|/|g;
}
my ($command_dir) = ($command =~ m|^(.*)/|);
# print "command=($command)\n";
# print "command_dir=($command_dir)\n";

#-----------------------------------------------------------------------------
sub dos_chomp($)
{
    my($l) = $_;
    my($c);

    while (defined($l))
    {
        $c = substr($l, -1);
        if ($c eq "\n" || $c eq "\r")
        {
            chop($l);
        }
        else
        {
            last;
        }
    }
    return($l);
}
#-----------------------------------------------------------------------------
# Input file contents in <CORE> is variable $hfile.$hfilesuffix.
# Create output file PROCANA.$hfile.$hfilesuffix and output everything, including the
# "****** File End: ... ******" into it.

sub processfile
{
    my $bin;
    my $ofile;
    my $tfile;

# Set which binary to use for procana.pl.
    if ($hfile eq 'ccbrun')
    {
        $bin = $binary_ccbrun;
    }
    elsif ($hfile eq 'Front.t')
    {
        $bin = $binary_Front;
    }
    elsif ($hfile eq 'Back.t')
    {
        $bin = $binary_Back;
    }
    else
    {
        print "Logic error in program. Unrecognized file ($hfile.$hfilesuffix).\n";
        exit 1;
    }

# Create a temporary file to hold the contents, before procana runs on it.
    $ofile = $SnapDump_Directory . '/' . $opt_p . 'PROCANA.' . $hfile . '.before';
    if (!open(OUT, '>' . $ofile))
    {
        die "Cannot open output file: $ofile\n";
    }

# Output the partial file contents (for this included file).
    print OUT $line . "\n";
    while (<CORE>)
    {
        $line = $_;
        $line = dos_chomp($line);       # no \n nor \r.
        print OUT $line . "\n";

# This is the stopping line.
        if ($line =~ /^\*\*\*\*\*\*   File End: $hfile\.$hfilesuffix   \*\*\*\*\*\*/)
        {
            last;
        }
    }
    close(OUT);

# Specify file for procana.pl output, and run procana.pl.
    $tfile = $SnapDump_Directory . '/' . $opt_p . 'PROCANA.' . $hfile . '.' . $hfilesuffix;
    system( $procana . ' ' . $bin . ' ' . $ofile . "> $tfile" );

# Get rid of temporary file.
    unlink($ofile);

# Say that procana.pl output was created (supposedly).
    print $tfile . " created\n";
}    # End of processfile()

#-----------------------------------------------------------------------------
# Set variable $Release_Directory as determined by snapdump output.

sub FindRelease
{
    my $debug;
    my $model;

    if (! -d '/home/Builds/storage/Wookiee/ByVersion')
    {
        print "/home/Builds/storage/Wookiee/ByVersion directory not found.\n";
      nofirmware:
        $Release_Directory = $command_dir;
        print "Using release directory ($Release_Directory).\n";
        return;
    }

    my $reldir;
    my $release;
    my $version;
    my $i;
    my $FW_Ver = `grep 'CCB FW Ver=' $SnapDump_Directory/$opt_p''CCB_Currenttrace.dmp`;
    if (!defined($FW_Ver) || $FW_Ver eq '')
    {
        print "Can not find firmware version in snapdump data.\n";
        goto nofirmware;
    }
    ($release, $version) = ($FW_Ver =~ /^CCB FW Ver=(.)(...)/);
    if (!defined($release) || !defined($version))
    {
        print "Can not guess firmware version from snapdump data.\n";
        goto nofirmware;
    }

    $FW_Ver = `egrep -i 'kernel.*-750-|kernel.*-3000-' $SnapDump_Directory/$opt_p''CCB_Logs.dmp | tail -1`;
    if (!defined($FW_Ver) || $FW_Ver eq '')
    {
        $FW_Ver = `grep 'LogSim/obj_.*[0-9][0-9][0-9][0-9]*/logsim' $SnapDump_Directory/$opt_p''CCB_Logs.dmp | tail -1`;
        if (!defined($FW_Ver) || $FW_Ver eq '')
        {
            print "Can not find model number in snapdump data.\n";
            goto nofirmware;
        }
        ($model) = ($FW_Ver =~ /LogSim\/obj_([0-9]*)\/logsim/);
    }
    else
    {
        ($model) = ($FW_Ver =~ /SYSTEM-KERNEL: [0-9.]*-([0-9]+)-/);
    }
    if (!defined($model) || ($model ne '750' && $model ne '3000' && $model ne '7000'))
    {
        print "Can not guess model number from snapdump data.\n";
        goto nofirmware;
    }
    
# If engineering release.
    if ("$release$version" =~ /engr/i)
    {
        print "Engineering release.\n";
        $Release_Directory = $command_dir;
        if (-e "$Release_Directory/ccbrun")
        {
            goto gotfirmware;
        }
        $Release_Directory = $command_dir . "/../../built-7000Debug";
        if (-e "$Release_Directory/ccbrun")
        {
            goto gotfirmware;
        }
        $Release_Directory = $command_dir . "/../../built-7000Perf";
        if (-e "$Release_Directory/ccbrun")
        {
            goto gotfirmware;
        }
        $Release_Directory = $command_dir . "/../../built-3000Debug";
        if (-e "$Release_Directory/ccbrun")
        {
            goto gotfirmware;
        }
        $Release_Directory = $command_dir . "/../../built-3000Perf";
        if (-e "$Release_Directory/ccbrun")
        {
            goto gotfirmware;
        }
        $Release_Directory = $command_dir . "/../../built-750Debug";
        if (-e "$Release_Directory/ccbrun")
        {
            goto gotfirmware;
        }
        $Release_Directory = $command_dir . "/../../built-750Perf";
        if (-e "$Release_Directory/ccbrun")
        {
            goto gotfirmware;
        }
        print "Can not find binaries for engineering build with snapana @ ($command_dir).\n";
        usage();
    }
    if ($release =~ /[a-z]/)
    {
        $debug = 1;             # Debug
        $i = ord($release) - ord('a');
        $release = chr(ord('A') + $i); # convert lowercase to uppercase.
    }
    elsif ($release =~ /[A-Z]/)
    {
        $debug = 2;             # production
    }
    else
    {
        print "Can not figure out release version from snapdump data.\n";
        goto nofirmware;
    }

# Get expanded release name. -- /home/Builds/storage/Wookiee/ByVersion/$reldir/$release.
    $reldir = $release . $version;

# First try at variable $Release_Directory.
    $Release_Directory = "/home/Builds/storage/Wookiee/ByVersion/$reldir/";

# See if Release_Directory is present.
    if (! -d $Release_Directory)
    {
        print "$Release_Directory not found, use snapana.pl directory argument.\n";
        goto nofirmware;
    }
    if ($debug == 1)
    {
        $Release_Directory .= "Input/eng/storage/Wookiee/built-${model}Debug";
    }
    else
    {
        $Release_Directory .= "Input/eng/storage/Wookiee/built-${model}Perf";
    }
    if (! -d $Release_Directory)
    {
        print "$Release_Directory not found, use snapana.pl directory.\n";
        goto nofirmware;
    }
  gotfirmware:
    print "Using release directory ($Release_Directory).\n";
}    # End of FindRelease()

#-----------------------------------------------------------------------------
sub usage
{
    print "usage: snapana [-v] [-p=Prefix] SnapDump_Directory [Release_Directory]\n";
    print "       -v                 = print version and exit.\n";
    print "       --version          = print version and exit.\n";
    print "       -p CN1             = Prefix in front of file names.\n";
    print "       --prefix=CN1       = Prefix in front of file names.\n";
    print "       SnapDump_Directory = Location of snapdump information.\n";
    print "       Release_Directory  = Where the release is located.\n";
    exit 0;
}

#-----------------------------------------------------------------------------
# Check if argument specification is correct.
# Output is $SnapDump_Directory and $Release_Directory.

sub CheckArguments
{
    my($c);

    # Allow single character option processing.
    Getopt::Long::Configure("noauto_abbrev",
                            "bundling_override",
                            "permute",
                            "ignore_case_always");

    # Decode the arguments.
    GetOptions( 'v|version', 'p|prefix=s');

    # Handle -v or --version argument.
    if ($opt_v)
    {
        print '$Id: snapana.pl 143007 2010-06-22 14:48:58Z m4 $' . "\n";
        exit 0;
    }

    # Change prefix option to have underscore if given.
    if ($opt_p)
    {
        $opt_p .= '_';
    }
    else
    {
        $opt_p = '';
    }

    # Handle the other arguments, must have at least one..
    if (! defined $ARGV[0])
    {
        print "Error - No SnapDump_Directory specified.\n";
        usage();
    }

    # Expecting the snapdump location.
    if (! defined $ARGV[0])
    {
        usage();
    }

    $SnapDump_Directory = $ARGV[0];
    shift @ARGV;
    # Delete trailing slashes.
    while ($SnapDump_Directory)
    {
        $c = substr($SnapDump_Directory, -1);
        if ($c eq '/')
        {
            chop($SnapDump_Directory);
        }
        else
        {
            last;
        }
    }

    if (! -d $SnapDump_Directory)
    {
        print "Error - ($SnapDump_Directory) is not a directory.\n";
        usage();
    }

    # Expecting the binary location.
    if (! defined $ARGV[0])
    {
        FindRelease();
    }
    else
    {
        $Release_Directory = $ARGV[0];
        shift @ARGV;
        # Delete trailing slashes.
        while ($SnapDump_Directory)
        {
            $c = substr($SnapDump_Directory, -1);
            if ($c eq '/')
            {
                chop($SnapDump_Directory);
            }
            else
            {
                last;
            }
        }

        if (! -d $Release_Directory)
        {
            print "Error - ($Release_Directory) is not a directory.\n";
            usage();
        }
    }

    # No more arguments allowed.
    if (defined $ARGV[0])
    {
        print "Error - Too many arguments ($ARGV[0])\n";
        usage();
    }
}    # End of CheckArguments()

#-----------------------------------------------------------------------------
# Sets variable $procana to path to procana.pl.

sub FindProcana
{
    $procana = $Release_Directory . '/procana.pl';
    if (! -e $procana)
    {
        print "Error - ($procana) script location not found.\n";
        print "        procana.pl expected in snapana.pl location.\n";
        usage();
    }
    if (! -x $procana)
    {
        print "Error - ($procana) is not executable.\n";
        usage();
    }
# print "procana = ($procana)\n";
}    # End of FindProcana()

#-----------------------------------------------------------------------------
# Sets variable $binaries to path to binaries with debug symbols to use.

sub FindBinaries
{
    $binary_ccbrun = $Release_Directory . '/' . 'ccbrun';
    if (! -e $binary_ccbrun)
    {
        print "Error - ccbrun executable does not exist ($binary_ccbrun).\n";
        usage();
    }
    if (! -x $binary_ccbrun)
    {
        print "Error - ccbrun is not executable ($binary_ccbrun).\n";
        usage();
    }

    $binary_Front = $Release_Directory . '/' . 'Front.t';
    if (! -x $binary_Front)
    {
        print "Error - Front.t does not exist or not executable ($binary_Front).\n";
        usage();
    }

    $binary_Back = $Release_Directory . '/' . 'Back.t';
    if (! -x $binary_Back)
    {
        print "Error - Back.t does not exist or not executable ($binary_Back).\n";
        usage();
    }
# print "binary_ccbrun=(${binary_ccbrun})  binary_Front=(${binary_Front}) binary_Back=(${binary_Back})\n";
}    # End of FindBinaries()

#-----------------------------------------------------------------------------
# Main program follows.

# See if arguments are correct.
CheckArguments();

# Try to find procana.pl script.
FindProcana();

# Try to find binaries with symbols.
FindBinaries();

# Try to find Wookiee_Core_Summaries.dmp file in SnapDump_Directory.
my $ifile = $SnapDump_Directory . '/' . $opt_p . 'Wookiee_Core_Summaries.dmp';
if (! open(CORE, '<' . $ifile))
{
    die "Cannot open file: $ifile\n";
}

while (<CORE>)
{
    $line = $_;
    $line = dos_chomp($line);        # no \n nor \r.
    if ($line =~ /^\*\*\*\*\*\*   File Start: (.*)\.(hist|txt)   \*\*\*\*\*\*/)
    {
        $hfile = $1;
        $hfilesuffix = $2;
        if ($hfile eq "ccbrun" ||
            $hfile eq "Front.t" ||
            $hfile eq "Back.t")
        {
            print "Processing file $hfile.$hfilesuffix\n";
            processfile();
        }
        else
        {
            print "ignoring file $hfile.$hfilesuffix\n";
        }
    }
}

close(CORE);

exit(0);

#-----------------------------------------------------------------------------
# vi:ts=4 sw=4 expandtab
