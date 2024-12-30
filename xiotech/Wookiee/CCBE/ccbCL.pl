#!/usr/bin/perl -w
# $Id: ccbCL.pl 163153 2014-04-11 23:33:11Z marshall_midden $
##############################################################################
# Copyright (c) 2001-2008  Xiotech Corporation. All rights reserved.
# ======================================================================
# Author: Randy Rysavy / Chris Nigbur / Tim Swatosh
#
# Purpose:
#   Interactive Command Line Interface for BIGFOOT
##############################################################################
#-- use strict;
#-- use strict "refs";
use strict "vars";
#-- use strict "subs";
#-- use diagnostics;

use Getopt::Std;
use FileHandle;
use Text::Abbrev;
use Cwd;
use Socket;
use IO::Select;

if ("$^O" eq "MSWin32")
{
    require Win32::Console;                                  # <WINDOWS ONLY>
    require Archive::Tar;                                    # <WINDOWS ONLY>
}

use XIOTech::cmdMgr;
use XIOTech::cmUtils;
use XIOTech::constants;
use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::logMgr;

use XIOTech::fmtFIDs;

use constant GOOD => 0;
use constant ERROR => 1;
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
sub uniq { my %seen; return grep(!($seen{$_}++), @_); } # a util function
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Forward references.
sub checkevacdatapac();
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Passed into routines as the arguments to the command typed in (read from file).
my @args;

my $useReadLine = 0;
my $noVerboseOutput = 0;
my $clArg;
foreach $clArg (@ARGV)
{
    if ($clArg =~ /^\s*-E/) {
        $noVerboseOutput = 1;
        last;
    }
}

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
if ("$^O" ne "MSWin32")
{
    require  Term::ReadLine;                                 # <LINUX ONLY>
}

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
use IO::Handle;
STDOUT->autoflush(1);
STDERR->autoflush(1);

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
our ($opt_a, $opt_b, $opt_c, $opt_d, $opt_e, $opt_f, $opt_g, $opt_h, $opt_i,
     $opt_j, $opt_k, $opt_l, $opt_m, $opt_n, $opt_o, $opt_p, $opt_q, $opt_r,
     $opt_s, $opt_t, $opt_u, $opt_v, $opt_w, $opt_x, $opt_y, $opt_z,
     $opt_A, $opt_B, $opt_C, $opt_D, $opt_E, $opt_F, $opt_G, $opt_H, $opt_I,
     $opt_J, $opt_K, $opt_L, $opt_M, $opt_N, $opt_O, $opt_P, $opt_Q, $opt_R,
     $opt_S, $opt_T, $opt_U, $opt_V, $opt_W, $opt_X, $opt_Y, $opt_Z);

our (
   $FG_BLACK, $FG_WHITE, $FG_YELLOW, $FG_LIGHTBLUE, $FG_BROWN, $FG_CYAN,
   $FG_GRAY, $FG_LIGHTMAGENTA, $FG_LIGHTCYAN, $FG_LIGHTGREEN, $FG_LIGHTRED,
   $FG_GREEN, $FG_BLUE, $FG_MAGENTA, $FG_RED, $BG_BLACK, $BG_WHITE,
   $BG_YELLOW, $BG_LIGHTBLUE, $BG_BROWN, $BG_CYAN, $BG_GRAY, $BG_LIGHTMAGENTA,
   $BG_LIGHTCYAN, $BG_LIGHTGREEN, $BG_LIGHTRED, $BG_GREEN, $BG_BLUE,
   $BG_MAGENTA, $BG_RED);


# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
my $windoze = ("$^O" eq "MSWin32");

my $currentConnection = -1;
my $currentIP = "";
my $currentPort = 3000;
my $currentType = "UNKNOWN";
my $currentMgr;
my $numNextConnection = 0;
my %connections;

my $logFile = "ccbCL";

my $ccbMapFile = "../CCB/obj_3000Debug/ccbrun.map";
my $beMapFile = "../Proc/obj_3000Debug/Back.map";
my $feMapFile = "../Proc/obj_3000Debug/Front.map";
my $defaultPort = 3000;

# The color option flag
my $color = 0;

my $redirectOpen = 0;
my %varHash; 
my %help;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
BuildHelp();
if ($ARGV[0] and ($ARGV[0] =~ /^-h/ or $ARGV[0] =~ /^--h/))
{
    print "$help{HELP}{LONG}\n";
    exit 1;
}

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
my $startDir = cwd();

my $prompt = "";
my $cmd;

#
# Logging is disabled by default. Uncomment this line to enable it.
# XIOTech::logMgr::logStart($logFile);

my %cmdHash = abbrev (keys %help);

###########################################################
################# Color related functions #################
###########################################################
my $OUT = undef;
my @colors = ();
my $resetColor;

if (! $windoze)
{
    $FG_BLACK        = "\e[30m";
    $FG_RED          = "\e[31m";
    $FG_GREEN        = "\e[32m";
    $FG_BROWN        = "\e[33m";
    $FG_BLUE         = "\e[34m";
    $FG_MAGENTA      = "\e[35m";
    $FG_CYAN         = "\e[36m";
    $FG_GRAY         = "\e[37m";
    $FG_LIGHTRED     = "\e[31;1m";
    $FG_LIGHTGREEN   = "\e[32;1m";
    $FG_YELLOW       = "\e[33;1m";
    $FG_LIGHTBLUE    = "\e[34;1m";
    $FG_LIGHTMAGENTA = "\e[35;1m";
    $FG_LIGHTCYAN    = "\e[36;1m";
    $FG_WHITE        = "\e[37;1m";

    $BG_BLACK        = "\e[40m";
    $BG_RED          = "\e[41m";
    $BG_GREEN        = "\e[42m";
    $BG_BROWN        = "\e[43m";
    $BG_BLUE         = "\e[44m";
    $BG_MAGENTA      = "\e[45m";
    $BG_CYAN         = "\e[46m";
    $BG_GRAY         = "\e[47m";
    $BG_LIGHTRED     = "\e[41;1m"; # The 'bolded' BG colors don't work right
    $BG_LIGHTGREEN   = "\e[42;1m"; # under Linux. Oh well.
    $BG_YELLOW       = "\e[43;1m";
    $BG_LIGHTBLUE    = "\e[44;1m";
    $BG_LIGHTMAGENTA = "\e[45;1m";
    $BG_LIGHTCYAN    = "\e[46;1m";
    $BG_WHITE        = "\e[47;1m";

    $resetColor = "\e[0m";
}

my $backgroundColor = $BG_BLACK; # default

# The order in which the colors are listed is the order they
# are assigned.
@colors = (
        $FG_WHITE,
        $FG_YELLOW,
        $FG_LIGHTBLUE,
        $FG_BROWN,
        $FG_CYAN,
        $FG_GRAY,
        $FG_LIGHTMAGENTA,
        $FG_LIGHTCYAN,
        $FG_LIGHTGREEN,
        $FG_LIGHTRED,
        $FG_GREEN,
        $FG_BLUE,
        $FG_MAGENTA,
        $FG_RED);

#
# Change to a new color
#
sub SetColor($)
{
    my ($index) = @_;
    my $attr;

    return if ! $color;

    if ($index < 0)
    {
        $attr = $resetColor;
    }
    else
    {
        $index %= scalar(@colors);
        if ($windoze)
        {
            $attr = ($colors[$index]|$backgroundColor);
        }
        else
        {
            $attr = ("$colors[$index]" . "$backgroundColor");
        }
    }

    if ($windoze)
    {
        $OUT->Attr($attr);
    }
    else
    {
        print "$resetColor"."$attr";
    }
}

# Set default color
SetColor(-1);
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# Read the CCBCL ini file if it exists. Check "C:/" first.
# Then try "$startDir/".
# NOTE: Some of the .ini options can get overridden by the
# command line options (color for example).
my $username = getlogin || getpwuid($<);
my $iniFile = $windoze ? "c:/ccbCL.ini" : "$ENV{HOME}"."/.ccbCL.ini";

if (!-r $iniFile)
{
    $iniFile = "$startDir/ccbCL.ini";
}

if (-r $iniFile)
{
    my ($var, $val);
    open INI, "$iniFile";
    if ($noVerboseOutput == 0) {
        print "Reading $iniFile...\n";
    }
    while (<INI>) {
        if (!/=/) {
            next;
        }
        ($var, $val) = split /\s*=\s*/;
        $var =~ s/\s+//g;
        $val =~ s/\s+//g;
        chomp $val;

        if ($var eq "PORT") {
            $defaultPort=$val;
        }
        if ($var eq "CCBMAP") {
            $ccbMapFile=$val;
        }
        if ($var eq "BEMAP") {
            $beMapFile=$val;
        }
        if ($var eq "FEMAP") {
            $feMapFile=$val;
        }
        if ($var eq "COLOR") {
            $color = 1 if ($val =~ /^y/i)
        }
        if ($var eq "USE_READLINE") {
            $useReadLine = 1 if ($val =~ /^y/i)
        }
        if ($var eq "COLORORDER")
        {
            @colors = ();
            my $col;
            foreach $col (split /,/, $val)
            {
                push @colors, $FG_BLACK         if ($col =~ /^BLACK$/i);
                push @colors, $FG_WHITE         if ($col =~ /^WHITE$/i);
                push @colors, $FG_YELLOW        if ($col =~ /^YELLOW/i);
                push @colors, $FG_LIGHTCYAN     if ($col =~ /^LIGHTCYAN/i);
                push @colors, $FG_BROWN         if ($col =~ /^BROWN/i);
                push @colors, $FG_CYAN          if ($col =~ /^CYAN/i);
                push @colors, $FG_GRAY          if ($col =~ /^GRAY/i);
                push @colors, $FG_LIGHTMAGENTA  if ($col =~ /^LIGHTMAGENTA/i);
                push @colors, $FG_LIGHTBLUE     if ($col =~ /^LIGHTBLUE/i);
                push @colors, $FG_LIGHTGREEN    if ($col =~ /^LIGHTGREEN/i);
                push @colors, $FG_LIGHTRED      if ($col =~ /^LIGHTRED/i);
                push @colors, $FG_GREEN         if ($col =~ /^GREEN/i);
                push @colors, $FG_BLUE          if ($col =~ /^BLUE/i);
                push @colors, $FG_MAGENTA       if ($col =~ /^MAGENTA/i);
                push @colors, $FG_RED           if ($col =~ /^RED/i);
            }
        }
        if ($var eq "BACKGROUND_COLOR")
        {
            $backgroundColor = $BG_BLACK        if ($val =~ /^BLACK$/i);
            $backgroundColor = $BG_WHITE        if ($val =~ /^WHITE$/i);
            $backgroundColor = $BG_YELLOW       if ($val =~ /^YELLOW$/i);
            $backgroundColor = $BG_LIGHTCYAN    if ($val =~ /^LIGHTCYAN$/i);
            $backgroundColor = $BG_BROWN        if ($val =~ /^BROWN$/i);
            $backgroundColor = $BG_CYAN         if ($val =~ /^CYAN$/i);
            $backgroundColor = $BG_GRAY         if ($val =~ /^GRAY$/i);
            $backgroundColor = $BG_LIGHTMAGENTA if ($val =~ /^LIGHTMAGENTA$/i);
            $backgroundColor = $BG_LIGHTBLUE    if ($val =~ /^LIGHTBLUE$/i);
            $backgroundColor = $BG_LIGHTGREEN   if ($val =~ /^LIGHTGREEN$/i);
            $backgroundColor = $BG_LIGHTRED     if ($val =~ /^LIGHTRED$/i);
            $backgroundColor = $BG_GREEN        if ($val =~ /^GREEN$/i);
            $backgroundColor = $BG_BLUE         if ($val =~ /^BLUE$/i);
            $backgroundColor = $BG_MAGENTA      if ($val =~ /^MAGENTA$/i);
            $backgroundColor = $BG_RED          if ($val =~ /^RED$/i);
        }
    }
    close INI;
}

# Run in file mode or interactive mode based upon if a file is
# passed on the cmdline or not.
my @connectTo;
my @commandFiles;
my $cmdf = "STDIN";
my $dashEcmdFile = $windoze ? "c:/temp/CCBCLdashEfile.cmd" :
                                          "/tmp/CCBCLdashEfile$$.cmd";
while (@ARGV)
{
    # Get the first command line parm and see if an IP address
    ($clArg) = shift @ARGV;

    if ($clArg =~ /-c/)
    {
        $color = 1;
        next;
    }

    if ($clArg =~ /-r/)
    {
        $useReadLine = 1;
        next;
    }

    if ($clArg =~ /-H/)
    {
        $useReadLine = 2;
        next;
    }

    if ($clArg =~ /^\s*-e/i) {
        # Assume the rest of the line is command data
        my $dashEdata = "$' " . "@ARGV";  # -e is removed
        $dashEdata =~ s/^\s*\"//;         # beginning quote is removed
        $dashEdata =~ s/\"\s*$//;         # ending quote is removed
        my @dashEarray = split /;/, $dashEdata;
        open DASHE, ">$dashEcmdFile";
        while (@dashEarray) {
            print DASHE shift(@dashEarray) . "\n";
        }
        if ($noVerboseOutput == 0) {
            print DASHE "quit\n";  # Always quit with -e
        }
        close DASHE;
        push @commandFiles, "$dashEcmdFile";
        last;
    }

    # If its an IP address, we will connect to it automatically
    if (($clArg =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/) or
       ($clArg =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}:\d{1,5}?$/))

    {
        push @connectTo, $clArg;
    }
    else {
        push @commandFiles, $clArg;
    }
}

# Setup color stuff for Windows
if ($windoze)
{
    if (!defined($OUT))
    {
        $OUT = new Win32::Console('STD_OUTPUT_HANDLE');
    }

    $resetColor = $OUT->Attr();
}

if (@commandFiles) {
    $cmdf = GetNextCmdFile();
}
else {
    # Read from STDIN
    $cmdf = "STDIN";
}

# Extract the version number from this file
my $version =  q$Revision: 163153 $;


# Autoflush all stdout writes
STDOUT->autoflush(1);

# Shortcuts for controller types
sub isBigfoot()
{
    return $currentMgr->{CONTROLLER_TYPE} == CTRL_TYPE_BIGFOOT;
}
sub isWookiee()
{
    return $currentMgr->{CONTROLLER_TYPE} == CTRL_TYPE_WOOKIEE;
}

sub is7400()
{
    return ($currentMgr->{CONTROLLER_TYPE} == CTRL_TYPE_7400);
}

sub is7000()
{
    return ($currentMgr->{CONTROLLER_TYPE} == CTRL_TYPE_7000);
}

sub is4700()
{
    return ($currentMgr->{CONTROLLER_TYPE} == CTRL_TYPE_4700);
}

sub is4000()
{
    return ($currentMgr->{CONTROLLER_TYPE} == CTRL_TYPE_4000);
}

sub is3100()
{
    return ($currentMgr->{CONTROLLER_TYPE} == CTRL_TYPE_3100);
}

sub is750()
{
    return $currentMgr->{CONTROLLER_TYPE} == CTRL_TYPE_750;
}

###########################################################
################# ReadLine Initialization #################
###########################################################
my $term;
my $lastHCmd;
my $historyFile;
sub InitializeReadline()
{
    $term = Term::ReadLine->new("ccbcl");
    my $attribs = $term->Attribs;
    $attribs->{already_prompted} = 0;  # doesn't work...
    $term->ornaments(0);               # shut off funky prompt underline
    $historyFile = "$ENV{HOME}" . "/.ccbcl_history";

    if ($useReadLine == 1 && -r $historyFile)
    {
        # Only trim on startup
        $term->history_truncate_file($historyFile, 1000);
        $term->ReadHistory($historyFile);
        $lastHCmd = $term->history_get($attribs->{history_length});
    }
    else
    {
        # Create the history file if doesn't exist
        open F, ">$historyFile";
        close F;
    }
}
if (!$windoze and $useReadLine)
{
    InitializeReadline();
}

##############################################################################
# Name:     displayPrompt
#
# Desc:     Display the command line prompt, includes the IP address
#           of the current connection (if active).
#
# Input:    IP Address
##############################################################################
sub getPrompt()
{
    my $g_prompt = "\n";

    if ($currentIP eq "")
    {
        $g_prompt .= "CCB CL";
    }
    else
    {
        my $type = substr($currentType,0,1);
        $g_prompt .= "[$currentConnection] $currentIP:$currentPort:$type";
    }

    $g_prompt .= "> ";
    return $g_prompt;
}

sub displayPrompt()
{
    if ($windoze)
    {
        $prompt = getPrompt();
    print $prompt;
    }
}

##############################################################################
# Name:     runcmd
#
# Desc:     Run a shell command
#
# Input:    Shell command string to run.
##############################################################################
sub runcmd($)
{
    my ($cmd) = @_;
    my $rc = (system "$cmd") >> 8;
    if ($rc)
    {
        print "\"$cmd\" FAILED with return code $rc\n";
    }
    return $rc;
}

##############################################################################
# Name:     isActiveConnection
#
# Desc:     Check to see if there is an active connection.
#
# Input:    None
##############################################################################
sub isActiveConnection()
{
    if (($currentConnection < 0) or
        ($currentIP eq "") or
        (!defined($currentMgr)) or
        (!defined($connections{$currentConnection})))
    {
        print "Current connection is not valid.\n";
        return 0;
    }

    #
    # Automatically reconnect to the controller if the connection
    # has been timed out -- test ports only (not 2341).
    #
    if ($currentMgr->{'PORT'} == 3100 or
        $currentMgr->{'PORT'} == 3200 or
        $currentMgr->{'PORT'} == 3000)
    {
        my $socket = $currentMgr->{'socket'};
        my $reader = IO::Select->new();
        $reader->add($socket);

        # Can we read the socket? If so, this means we are disconnected.
        # (I think we are getting an EOF)
        my ($ready) = IO::Select->select($reader, undef, undef, 0);

        if (defined($ready))
        {
            # we are disconnected, so try a reconnect
            print "\nReconnecting...\n";
            my $rc = $currentMgr->login($currentMgr->{'HOST'}, $currentMgr->{'PORT'}, $noVerboseOutput);

            if (!$rc)
            {
                print "Reconnection attempt failed, removing controller ".
                      "from connection list.\n";
                logout();
                return 0;
            }
        }
    }

    return 1;
}

##############################################################################
# Individual "worker" functions
##############################################################################

##############################################################################
# Name:     chgConn
#
# Desc:     Change to another active connection
#
# Input:    Connection ID
##############################################################################
sub chgConn()
{
    my ($id) = @args;

    my ($ip, $port);
    my $i;

    if (defined($id)) {
        if ($id =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/) {
            $ip = $id;
            undef $port;
            for ($i = 0; $i < $numNextConnection; ++$i)
            {
                if (defined($connections{$i}{"IP"}))
                {
                    if ($ip eq $connections{$i}{"IP"})
                    {
                        $id = $i;
                        last;
                    }
                }
            }
        }
        elsif ($id =~ /^\d{4,5}?$/) {
            $port = $id;
            undef $ip;
            for ($i = 0; $i < $numNextConnection; ++$i)
            {
                if (defined($connections{$i}{"IP"}))
                {
                    if ($port eq $connections{$i}{"PORT"})
                    {
                        $id = $i;
                        last;
                    }
                }
            }
        }
        elsif ($id =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}:\d{1,5}?$/) {
            ($ip, $port) = split(/:/, $id);
            for ($i = 0; $i < $numNextConnection; ++$i)
            {
                if ( defined($connections{$i}{"IP"}) and
                        defined($connections{$i}{"PORT"}) )
                {
                    if ($ip eq $connections{$i}{"IP"} and
                            $port eq $connections{$i}{"PORT"})
                    {
                        $id = $i;
                        last;
                    }
                }
            }
        }
    }

    if (defined($id) &&
        defined($connections{$id}) &&
        defined($connections{$id}{"IP"}) &&
        defined($connections{$id}{"MGR"}) &&
        defined($connections{$id}{"PORT"}))
    {
        $currentConnection = $id;
        $currentIP = $connections{$currentConnection}{"IP"};
        $currentMgr = $connections{$currentConnection}{"MGR"};
        $currentPort = $connections{$currentConnection}{"PORT"};
        $currentType = $connections{$currentConnection}{"TYPE"};

        SetColor($currentConnection);
#        print "\n";
#        print "Current connection changed to ($currentIP:$currentPort).\n";
    }
    else
    {
        print "\n";
        print "No connection or invalid or missing connection ID.\n";
    }
}

##############################################################################
# Name:     dspConn
#
# Desc:     Display active connections
#
# Input:    None
##############################################################################
sub dspConn()
{
    my $id;

    SetColor(-1);

    print "\nAvailable Connections:\n\n";

    foreach $id (sort keys %connections)
    {
        SetColor($id);

        my $ip = $connections{$id}{"IP"};
        my $port = $connections{$id}{"PORT"};
        my $type = $connections{$id}{"TYPE"};
        if (defined($ip))
        {
            if ($currentConnection == $id) {
                print " [$id]\t$ip:$port:$type\n";
            }
            else {
                print "  $id\t$ip:$port:$type\n";
            }
        }
    }

    SetColor($currentConnection);
}

##############################################################################
# Name:     batteryHealthSet
#
# Desc:     Sets the battery health for a given board.
#
# Input:    Health State.
##############################################################################
sub batteryHealthSet()
{
    my ($state) = @args;

    print "\n";

    if (!defined($state))
    {
        print "Missing battery state.\n";
    }

    if (uc($state) eq "GOOD")
    {
        $state = 0;
    }
    elsif (uc($state) eq "BAD")
    {
        $state = 1;
    }

    # Send the request to change the FE battery health state.
    my %rsp = $currentMgr->batteryHealthSet(0, $state);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Battery health updated.\n";
        }
        else
        {
            my $msg = "Unable to set the battery health.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     deviceCount
#
# Desc:     Gets the device count for the given serial number.
#
# Input:    Serial Number
##############################################################################
sub deviceCount()
{
    my ($sn) = @args;

    print "\n";

    if (!defined($sn))
    {
        print "Missing serial number.\n";
        return;
    }

    my %rsp = $currentMgr->deviceCount($sn);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayDeviceCount(%rsp);
        }
        else
        {
            my $msg = "Unable to get the device count ($sn).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     deviceName
#
# Desc:     Sets or retrieves a device name.
#
# Input:    None
##############################################################################
sub deviceName()
{
    my ($id, $option, $name) = @args;

    print "\n";

    if (!defined($id))
    {
        print "Missing identifier.\n";
        return;
    }

    if (!defined($option))
    {
        print "Missing option.\n";
        return;
    }

    if (uc($option) eq "SERVER")
    {
        $option = MNDSERVER;
    }
    elsif (uc($option) eq "VDISK")
    {
        $option = MNDVDISK;
    }
    elsif (uc($option) eq "VCG")
    {
        $option = MNDVCG;
    }
    elsif (uc($option) eq "RETVCG")
    {
        $option = MNDRETVCG;
    }

    if ($option != MNDSERVER &&
        $option != MNDVDISK &&
        $option != MNDVCG &&
        $option != MNDRETVCG)
    {
        print "Invalid option.\n";
        return;
    }

    if (!defined($name))
    {
        if ($option == MNDRETVCG)
        {
            $name = " ";
        }
        else
        {
            print "Missing name.\n";
            return;
        }
    }

    my %rsp = $currentMgr->deviceName($id, $option, $name);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayDeviceName(%rsp);
        }
        else
        {
            my $msg = "Unable to set or retrieve device name.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     deviceConfigGet
#
# Desc:     Get the device configuration information.
#
# Input:    none
##############################################################################
sub deviceConfigGet()
{
    print "\n";

    my %rsp = $currentMgr->deviceConfigGet();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayDeviceConfigGet(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "ERROR: Unable to retrieve device configuration information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     deviceConfigSet
#
# Desc:     Updates the device configuration information.
#
# Input:    Device configuration file name.
##############################################################################
sub deviceConfigSet()
{
    my ($fileName) = @args;
    my $handle;
    my $buf;
    my $fData = "";
    my @fLines;
    my @lineData;
    my $fileItem;
    my $deviceCount = 0;
    my @devices;
    my %deviceInfo;
    my $i;

    print "\n";

    if (!defined($fileName))
    {
        print "Missing configuration file.\n";
        return;
    }

    unless (open($handle, '<', $fileName))
    {
        print "Invalid or unavailable file.\n";
        return;
    }

    while (!eof($handle))
    {
        read $handle, $buf, 50;
        $fData .= $buf;
    }
    close $handle;

    @fLines = split /\n/, $fData;

    for ($i = 0; $i < scalar(@fLines); $i++)
    {
        @lineData = split /, */, $fLines[$i];

        # Parse out the Vendor text by trimming off the quotes.
        $devices[$i]{DEVVENDOR} = $lineData[0];
        $devices[$i]{DEVVENDOR} =~ s/"//g;

        # Parse out the Product ID text by trimming off the quotes.
        $devices[$i]{DEVPRODID} = $lineData[1];
        $devices[$i]{DEVPRODID} =~ s/"//g;

        # Each of the device flags are in hex to convert that text
        # into actual values.
        $devices[$i]{DEVFLAGS}[0] = oct $lineData[2];
        $devices[$i]{DEVFLAGS}[1] = oct $lineData[3];
        $devices[$i]{DEVFLAGS}[2] = oct $lineData[4];
        $devices[$i]{DEVFLAGS}[3] = oct $lineData[5];
        $devices[$i]{DEVFLAGS}[4] = oct $lineData[6];
        $devices[$i]{DEVFLAGS}[5] = oct $lineData[7];
        $devices[$i]{DEVFLAGS}[6] = oct $lineData[8];
        $devices[$i]{DEVFLAGS}[7] = oct $lineData[9];

        $deviceCount++;
    }
    my %rsp = $currentMgr->deviceConfigSet(\@devices);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Device configuration information updated.\n";
        }
        else
        {
            my $msg = "ERROR: Unable to update device configuration information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     devStatus
#
# Desc:     Display device status.
#
# Input:    None
##############################################################################
sub devStatus()
{
    my @info = @args;
    my $n_args = scalar(@info);
    my $swtch = "N";
    my $dmy;
    my $i = 0;

    if ($n_args > 0 && (($info[0]) =~ m/\s*\/\w+/))
    {
        ($dmy, $swtch) = split /\//, $info[0];
    }

    if ($n_args <= 0 || ((uc($info[0]) eq "ALL")) || ((($info[0]) =~ m/\s*\/\w+/) && (!defined($info[1])))
        || ((($info[0]) =~ m/\s*\/\w+/) && ((uc($info[1]) eq "ALL"))))
    {
        devStatusHelper("PD", $swtch);
        devStatusHelper("VD", $swtch);
        devStatusHelper("RD", $swtch);
    }
    else
    {
        if (($info[0]) =~ m/\s*\/\w+/)
        {
            $i = 1;
        }

        for (; $i <  $n_args; ++$i)
        {
            my $dev = $info[$i];

            if (!defined($dev) ||
               (!(uc($dev) eq "PD") && !(uc($dev) eq "RD") &&
                !(uc($dev) eq "VD")))
            {
                print "Invalid device, parameter ". ($i+1) .
                      ". use help devstatus for more info\n";
            }
            else
            {
                devStatusHelper($dev, $swtch);
            }
        }
    }
}

##############################################################################
# Name:     devStatusHelper
#
# Desc:     Display device status to the screen.
#
# Input:    DEV -   Device to gather information for
##############################################################################
sub devStatusHelper($$)
{
#   It is assumed that since this is a helper function, error checking
#   is done in the parent function devStatus

    my ($dev, $swtch) = @_;

    print "\n";

    if (uc($dev) eq "PD")
    {
        print "Retrieving physical disk status information...\n";
    }
    elsif (uc($dev) eq "RD")
    {
        print "Retrieving raid device status information...\n";
    }
    elsif (uc($dev) eq "VD")
    {
        print "Retrieving virtual disk status information...\n";
    }
    print "\n";

    my %rsp = $currentMgr->deviceStatus($dev);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayDeviceStatus($dev, $swtch, %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve device status.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
    print "\n";
}

##############################################################################
# Name:     engDebug
#
# Desc:     Engineering Debug Pass-Through Command.
#
# Input:    None
##############################################################################
sub engDebug
{
    print "\n";
    print "Running engineering debug pass-through command.\n";
    print "\n";

    my @in_args;

    for (my $i = 0; $i < scalar(@args); ++$i)
    {
        $in_args[$i] = $args[$i];
    }

    $currentMgr->engDebug(\@in_args);

    print "\n";
}

##############################################################################
# Name:     writeBuffer
#
# Desc:     downloads new code to a device
#
# Input:
#
# Output:
##############################################################################
sub writeBuffer
{
    my ($type, $physicalDisks, $filename) = @args;

    print "\n";

    my @pids;
    my @arr;
    my $i = 0;
    my %raidParms;

    if (!defined($type) || $type !~ /^PDISK$|^BAY$/i)
    {
        print "\n";
        print "Invalid or missing type (PDISK|BAY).\n";
        return;
    }

    if (!defined($physicalDisks))
    {
        print "Invalid or missing physical disks or bays array.\n";
        return;
    }

    if (defined $filename)
    {
        $filename =~ s/\./\\./g;    # escape the '.'s
        m/$filename.*/;             # find the filename in $_
        ($filename = $&) =~ s/"//g; # strip the quotes (if any)"

        if (! -r $filename)
        {
            print "Missing or unreadable file\n";
            return;
        }
    }
    else
    {
        print "Invalid or missing filename.\n";
        return;
    }

    @pids = $currentMgr->rangeToList($physicalDisks);

    if ($type =~ /^PDISK$/i)
    {

        for ($i = 0; $i < scalar(@pids); ++$i)
        {
            my %pdisks = $currentMgr->physicalDiskInfo($pids[$i]);

            if (%pdisks)
            {
                if ($pdisks{STATUS} == PI_GOOD)
                {
                    $arr[$i]{WWN_LO} = $pdisks{WWN_LO};
                    $arr[$i]{WWN_HI} = $pdisks{WWN_HI};
                    $arr[$i]{PD_LUN}    = $pdisks{PD_LUN};
                }
                else
                {
                    my $msg = "Unable to retrieve pdisk info.";
                    displayError($msg, %pdisks);
                    return;
                }
            }
            else
            {
                print "ERROR: Did not receive a response packet.\n";
                logout();
            }
        }

    }

    elsif ($type =~ /^BAY$/i)
    {

        for ($i = 0; $i < scalar(@pids); ++$i)
        {
            my %pdisks = $currentMgr->diskBayInfo($pids[$i]);

            if (%pdisks)
            {
                if ($pdisks{STATUS} == PI_GOOD)
                {
                    $arr[$i]{WWN_LO} = $pdisks{WWN_LO};
                    $arr[$i]{WWN_HI} = $pdisks{WWN_HI};
                    $arr[$i]{PD_LUN}    = $pdisks{PD_LUN};
                }
                else
                {
                    my $msg = "Unable to retrieve disk bay info.";
                    displayError($msg, %pdisks);
                    return;
                }
            }
            else
            {
                print "ERROR: Did not receive a response packet.\n";
                logout();
            }
        }

    }

    my %rsp = $currentMgr->writeBuffer($i, $filename, @arr);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "WRITEBUFFER successful.\n";
        }
        else
        {
            my $msg = "WRITEBUFFER failed.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     scsiCmd
#
# Desc:     issues a generic scsi cmd to a device
#
# Input:
#
# Output:
##############################################################################
sub scsiCmd
{
    my ($type, $device, $CDB, $inpData) = @args;

    print "\n";

    my @deviceID;
    my $special_print = 0;
    my $msg;

    #
    # Parse output data length
    #
    my $flen;

    #
    # Parse type
    #
    if (!defined($type) || $type !~ /^PDISK$|^BAY$/i)
    {
        print "\n";
        print "Invalid or missing type (PDISK|BAY).\n";
        return;
    }

    #
    # Parse device pid
    #
    if (!defined($device))
    {
        print "Invalid or missing physical disk or bay id.\n";
        return;
    }

    #
    # Parse CDB
    #
    if (!defined($CDB))
    {
        print "Missing CDB.\n";
        return;
    }

    #
    # Look for basic CDB's
    #
    if ($CDB =~ /^TEST_UNIT_READY$|^TUR$/i)
    {
        $CDB = "000000000000";
        $flen = 0;
        print "Issuing CDB \"$CDB\"\n";
    }
    elsif ($CDB =~ /^REQUEST_SENSE$|^RS$/i)
    {
        $CDB = "03000000FF00";
        $flen = 256;
        print "Issuing CDB \"$CDB\"\n";
    }
    elsif ($CDB =~ /^INQUIRY$|^INQ$/i)
    {
        $CDB = "12000000FF00";
        $flen = 256;
        print "Issuing CDB \"$CDB\"\n";
    }
    elsif ($CDB =~ /^START_UNIT$|^START$/i)
    {
        $CDB = "1B0000000100";
        $flen = 0;
        print "Issuing CDB \"$CDB\"\n";
    }
    elsif ($CDB =~ /^STOP_UNIT$|^STOP$/i)
    {
        $CDB = "1B0000000000";
        $flen = 0;
        print "Issuing CDB \"$CDB\"\n";
    }
    elsif ($CDB =~ /^LOG_PAGE_32$|^LOG32$/i)
    {
        $CDB = "4d003200000000010400";
        $flen = 0x104;
        $special_print = 1;
        print "Issuing CDB \"$CDB\"\n";
    }
    elsif ($CDB =~ /^PAGE30$|^PAGE_30$/i)
    {
        $CDB = "4d003000000000200000";
        $flen = 8160;
        print "Issuing CDB \"$CDB\"\n";
    }

    if (defined($opt_l))
    {
        $flen = $opt_l;
    }
    #
    # Translate the cdb field
    #
    my $cdb = AsciiHexToBin($CDB, "byte");
    if (!defined $cdb) {
        print "Invalid CDB format.\n";
        return;
    }

    #
    # Parse input data
    #
    if (defined($inpData)) {
        $inpData = AsciiHexToBin($inpData, "byte");
        if (!defined $inpData) {
            print "Invalid Data format.\n";
            return;
        }
    }
    else {
        $inpData = undef;
    }

    #
    # Retrieve WWN/LUN from type/pid
    #
    my %pdisks;
    if ($type =~ /^PDISK$/i)
    {
        %pdisks = $currentMgr->physicalDiskInfo($device);
    }
    else
    {
        %pdisks = $currentMgr->diskBayInfo($device);
    }

    if (%pdisks)
    {
        if ($pdisks{STATUS} == PI_GOOD)
        {
            $deviceID[0]{WWN_LO} = $pdisks{WWN_LO};
            $deviceID[0]{WWN_HI} = $pdisks{WWN_HI};
            $deviceID[0]{PD_LUN} = $pdisks{PD_LUN};
        }
        else
        {
            $msg = "Unable to retrieve pdisk/bay info.";
            displayError($msg, %pdisks);
            return;
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    if (defined($opt_s))
    {
        $deviceID[0]{PD_LUN} = $opt_s;
    }

    if (defined($opt_V))
    {
        printf "Sending to WWN %08x%08x LUN %d SCSI command %s\n",
            $deviceID[0]{WWN_LO}, $deviceID[0]{WWN_HI}, $deviceID[0]{PD_LUN}, $CDB;
    }
    #
    # Call the scsi cmd handler
    #
    my %rsp = $currentMgr->scsiCmd($cdb, $inpData, @deviceID);
    if (%rsp)
    {
        #
        # If successful, format the output data
        #
        if ($rsp{STATUS} == PI_GOOD)
        {
            if ($special_print == 1)
            {
                print "Log Sense SCSI cmd successful.\n\n";
                my $msgLP32;
                $msgLP32 = $currentMgr->FmtLogPage32(\$rsp{DATA});
                print $msgLP32;
                print "\n\n";
            }
            else
            {
                print "SCSI cmd successful.\n\n";
                if (!defined($inpData)) {
                    $currentMgr->FormatData($rsp{DATA}, 0x00000000,
                        "byte", undef, $flen);
                }
            }
        }
        #
        # If failure, display sense data
        #
        else
        {
            $msg = "SCSI cmd failed.";
            displayError($msg, %rsp);

            printf "Sense Key:      0x%02X\n", $rsp{SENSE_KEY};
            printf "Sense Code:     0x%02X\n", $rsp{ADTL_SENSE_CODE};
            printf "Sense Code Qual:0x%02X\n", $rsp{ADTL_SENSE_CODE_QUAL};
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     page30 for luns of an ISE
##############################################################################
sub page30bay
{
    my ($bay) = @args;
    my %rsp;
    my $i;
    my $msg;

    # Output data length
    my $flen = 8192;

    # Parse bay ID.
    if (!defined($bay))
    {
        print "Need a bay number.\n";
        return;
    }

    if (!defined($opt_n))
    {
       $opt_n = 1;
    }
    my $results_original;
    my %luns;
    $luns{0} = $luns{1} = $luns{2} = $luns{3} = $luns{4} = 
      $luns{5} = $luns{6} = $luns{7} = $luns{8} = $luns{9} = $luns{10} =
      $luns{11} = $luns{12} = $luns{13} = $luns{14} = $luns{15} = $luns{16} = 0;
    print "ISE #$bay page 0x30 fetching and comparing:\n";
    printf "ISELUN status speed portset  status speed portset\n";
    my $cnt = 0;
    my $flagthis = 0;
    $opt_n++;
    my $totalcnt = 0;
    while ($opt_n-- > 0)
    {
        if ($cnt >= 80)
        {
            printf " %d\n", $totalcnt;
            $cnt = 0;
        }
        if (defined($results_original))
        {
            $cnt++;
            $totalcnt++;
            print ".";
        }
        # Retrieve WWN/LUN from bay number.
        my %pdisks = $currentMgr->diskBayInfo($bay);

        if (!%pdisks)
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
        if ($pdisks{STATUS} != PI_GOOD)
        {
            $msg = "Unable to retrieve pdisk/bay info.";
            displayError($msg, %pdisks);
            return;
        }

        my @deviceID;
        $deviceID[0]{WWN_LO} = $pdisks{WWN_LO};
        $deviceID[0]{WWN_HI} = $pdisks{WWN_HI};
        $deviceID[0]{PD_LUN} = $pdisks{PD_LUN};

        if ($deviceID[0]{PD_LUN} != 0)
        {
            print "ERROR: LUN for bay ID is not zero.\n";
            return;
        }

        # Translate the cdb field.
        my $cdb = AsciiHexToBin("4d003000000000200000", "byte");

        my @results;

        foreach $i (sort { $a <=> $b } keys %luns)
        {
            $deviceID[0]{PD_LUN} = $i;
            # Call the scsi cmd handler.
            undef %rsp;
            %rsp = $currentMgr->scsiCmd($cdb, undef, @deviceID);
            if (%rsp)
            {
                # If successful, format the output data.
                if ($rsp{STATUS} == PI_GOOD)
                {
# printf "Page 0x30 for WWN %08x%08x LUN %d\n", $deviceID[0]{WWN_LO}, $deviceID[0]{WWN_HI}, $i;
#--                 $currentMgr->FormatData($rsp{DATA}, 0x00000000, "byte", undef, $flen);
                    $results[$i] = $rsp{DATA};
#     UINT8       fc_port_status; // controller_fc_port_status                        // 325 and 477
#     UINT8       fc_port_speed;  // controller_fc_port_speed                         // 326 and 478
#     UINT8       fc_port_set;    // controller_fc_port_speed_setting                 // 327 and 479
#    # printf "check byte %d\n", $k;
                    if (!defined($results_original))
                    {
                        $results_original = $rsp{DATA};
                    }
                    if ($flagthis == 0)
                    {
                        my $a = ord(substr($results[$i], 325, 1));
                        my $b = ord(substr($results[$i], 326, 1));
                        my $c = ord(substr($results[$i], 327, 1));
                        my $d = ord(substr($results[$i], 325, 1));
                        my $e = ord(substr($results[$i], 326, 1));
                        my $f = ord(substr($results[$i], 327, 1));
printf "%2d-%02d    0x%02x  0x%02x  0x%02x      0x%02x  0x%02x  0x%02x\n", $bay, $i, $a,$b,$c, $d,$e,$f;
                    }
                }
                else
                {
                    delete($luns{$i});
#-- printf "Page 0x30 SCSI cmd failed to WWN %08x%08x LUN %d\n", $deviceID[0]{WWN_HI}, $deviceID[0]{WWN_LO}, $i;
#-- displayError("scsi command log sense for page 0x30 failed", %rsp);
#-- printf "Sense Key:      0x%02X\n", $rsp{SENSE_KEY};
#-- printf "Sense Code:     0x%02X\n", $rsp{ADTL_SENSE_CODE};
#-- printf "Sense Code Qual:0x%02X\n", $rsp{ADTL_SENSE_CODE_QUAL};
                }
            }
            else
            {
                printf "Page 0x30 No response to SCSI cmd for WWN %08x%08x LUN %d\n", $deviceID[0]{WWN_HI}, $deviceID[0]{WWN_LO}, $i;
            }
        }
        $flagthis = 1;


        # 0=PageCode
        # 28=hw_id
        # 180=num_mrcs
        # 189=pad[3]
        # 328=mrc0.fc_port_status
        # 480=mrc1.fc_port_status
        # 600=bat1.status
        # 704=ps0.status
        # 710=ps0.beacon
        # 804=ps.status
        # 810=ps1.beacon
        # 904=dp0.status
        # 912=dp0.type
        # 1020=dp1.type
        my @start = (   0,   28,  180,  189,  328,  480,  600,  704,  710, 804, 810, 904,  912, 1020);
        my @end   = (  24,  124,  188,  324,  476,  596,  700,  709,  796, 809, 896, 911, 1019, 1120);
        # 24=temp
        # 124=uptime->avg_bytes transferred
        # 188=cache_dirty
        # 324=mrc0.temp
        # 476=mrc1.temp
        # 596=bat0.remaining_charge
        # 700=bat1.remaining_charge
        # 709=ps0.temp
        # 796=ps0.fan1_speed
        # 809=ps1.temp
        # 896=ps1.fan1_speed
        # 911=dp0.temp
        # 1019=dp1.temp
        # 1120=end of datapacs and start of drives
        my $first_failure;

        foreach $i (sort { $a <=> $b } keys %luns)
        {
            if (!defined($results[$i]))
            {
                next;
            }

    # printf "results[$i] is %d long,  results[0] is %d long\n", length($results[$i]), length($results[0]);

            my $k;
            my $j;
            my $ret = 0;

            for ($j = 0; $j < 14; $j++)
            {
                for ($k = $start[$j]; $k < $end[$j]; $k++)
                {
    # printf "check byte %d\n", $k;
                    my $p = substr($results_original, $k, 1);
                    my $q = substr($results[$i], $k, 1);
                    if ("$p" ne "$q")
                    {
                        $ret = 1;
                        printf "  byte %d [o/n](%02x != %02x)  ", $k, ord($p), ord($q);
                    }
                }
            }
            if ($ret == 0)
            {
#                printf "$i matches LUN 0\n";
                next;
            }
            print "\n";

            if (!defined($first_failure))
            {
                printf "$i does not match LUN 0\n";
                $first_failure = $i;
                next;
            }

            $ret = 0;
            for ($j = 0; $j < 12; $j++)
            {
                for ($k = $start[$j]; $k < $end[$j]; $k++)
                {
    # printf "check byte %d\n", $k;
                    my $p = substr($results[$i], $k, 1);
                    my $q = substr($results[$first_failure], $k, 1);
                    if ("$p" ne "$q")
                    {
                        $ret = 1;
                        printf "  byte %d (%02x != %02x)  ", $k, ord($p), ord($q);
                    }
                }
            }
            if ($ret == 0)
            {
                printf "$i matches LUN $first_failure\n";
                next;
            }
            print "\n";
            printf "$i does not match LUN 0 nor LUN $first_failure\n";
        }
        if (defined($first_failure))
        {
            last;
        }
    }
    print "\n";
}   # End of page30bay

##############################################################################
# Name:     fwUpdateFWK
#
# Desc:     Send a firmware kit to the CCB. This routine is called from
#           fwUpdate.
#
# Input:    Filename
##############################################################################
sub fwUpdateFWK($)
{
    my ($filen) = @_;
    my $dir = "c:\temp";
    my $tmpf = "fwUpdateFWK.ima";

    if (!defined($filen) or (! -r $filen))
    {
        print "Missing or unreadable file\n";
        return;
    }

    # Read in the tar FWK file
    my $fwk = Archive::Tar->new($filen, 1);
    my @image_list = $fwk->list_files();

    if (defined($ENV{TEMP}))
    {
        $dir = $ENV{TEMP};
    }
    elsif (defined($ENV{TMP}))
    {
        $dir = $ENV{TMP};
    }

    my $image;
    foreach $image (@image_list)
    {
        my $buffer = $fwk->get_content($image);
        if (!open FWK, ">$dir/$tmpf")
        {
            print "Couldn't open $dir/$tmpf\n";
            return;
        }
        binmode FWK;
        print FWK $buffer;
        close FWK;

        print "Sending $image...\n";
        fwUpdate("$dir/$tmpf");
    }

    unlink("$dir/$tmpf");
}



##############################################################################
# Name:     fwUpdate
#
# Desc:     Send a firmware file to the CCB.
#
# Input:    Filename
##############################################################################
sub fwUpdate($$)
{
    my ($filen, $cmd) = @_;

    print "\n";

    if (!defined($filen) or (! -r $filen))
    {
        print "Missing or unreadable file\n";
        return;
    }

    # If the file is a firmware kit (.fwk), go handle it.
    if ($filen =~ /\.fwk$/i)
    {
        return fwUpdateFWK($filen);
    }

    my %rsp;
    if ($cmd eq "FWUPDATE") {
        %rsp = $currentMgr->sendFirmware($filen);
    }
    else {
        %rsp = $currentMgr->mpxSendFirmware($filen);
    }

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Firmware update successful!\n";
        }
        else
        {
            my $msg = "Firmware update failed. See logs for details.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     fwVersion
#
# Desc:     Displays the current firmware versions.
#
# Input:    NONE
##############################################################################
sub fwVersion
{
    my $continue = 1;
    print "\n";

    my %rsp = $currentMgr->fwVersion();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayFirmwareVersion(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "ERROR: Unable to retrieve firmware information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
        $continue = 0;
    }

    # Go get the mach code levels (hack for now, until we change the
    # way that fw levels are retrieved from the proc).
    if ($continue && isBigfoot()) {
        my $revRaw;
        my $revCooked;
        my %data = $currentMgr->ReadMemory(0xFEE80003, 1, "CCB");
        if ($data{STATUS} != 0) {
            print "\nCouldn't read CCB Mach revision...\n";
        }
        else {
            $revRaw = unpack("C", $data{RD_DATA}) >> 4;
            printf "CCB_BOARD   %X\n", $revRaw;

            $revRaw = unpack("C", $data{RD_DATA}) & 0xF;
            $revCooked = "?";
            $revCooked = "old"   if ($revRaw == 0x2);
            $revCooked = "rev A" if ($revRaw == 0x3);
            $revCooked = "rev B" if ($revRaw == 0x4);
            printf "CCB_MACH    %X (%s)\n", $revRaw, $revCooked;
        }

        print "-" x 21 . "\n";

        %data = $currentMgr->ReadMemory(0xFE400003, 1, "BE");
        if ($data{STATUS} != 0) {
            print "\nCouldn't read BE Mach revision...\n";
        }
        else {
            $revRaw = unpack("C", $data{RD_DATA}) >> 4;
            printf "BE_BOARD    %X\n", $revRaw;

            $revRaw = unpack("C", $data{RD_DATA}) & 0xF;
            $revCooked = "?";
            $revCooked = "B" if ($revRaw == 0x8);
            $revCooked = "5" if ($revRaw == 0x9);
            $revCooked = "6" if ($revRaw == 0xA);
            $revCooked = "A" if ($revRaw == 0xB);
            printf "BE_MACH     %X (rev %s)\n", $revRaw, $revCooked;
        }

        print "-" x 21 . "\n";

        %data = $currentMgr->ReadMemory(0xFE400003, 1, "FE");
        if ($data{STATUS} != 0) {
            print "\nCouldn't read FE Mach revision...\n";
        }
        else {
            $revRaw = unpack("C", $data{RD_DATA}) >> 4;
            printf "FE_BOARD    %X\n", $revRaw;

            $revRaw = unpack("C", $data{RD_DATA}) & 0xF;
            $revCooked = "?";
            $revCooked = "B" if ($revRaw == 0x8);
            $revCooked = "5" if ($revRaw == 0x9);
            $revCooked = "6" if ($revRaw == 0xA);
            $revCooked = "A" if ($revRaw == 0xB);
            printf "FE_MACH     %X (rev %s)\n", $revRaw, $revCooked;
        }

        print "-" x 21 . "\n";
    }

    print "\n";
}

##############################################################################
# Name:     FwSysRel
#
# Desc:     Gets the overall Firmware System Release Level for the controller
#
# Input:    none
##############################################################################
sub FwSysRel
{
    print "\n";

    my %rsp = $currentMgr->FWSystemRelease();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayFWSysRel(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to get the Firmware System Release information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     failureStateSet
#
# Desc:     Set the failure state of a controller
#
# Input:    NONE
##############################################################################
sub failureStateSet
{
    my ($serialNumber, $state) = @args;

    print "\n";

    my %rsp = $currentMgr->failureStateSet($serialNumber, $state);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Failure state for controller ($serialNumber) has been set.\n";
        }
        else
        {
            my $msg = "ERROR: Unable to set failure state.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}
##############################################################################
# Name:     timeSync
#
# Desc:     Sync controller's real time clock to the local pc.
#
# Input:    NONE
##############################################################################
sub timeSync
{
    print "\n";
    my $systime;

    if ($opt_o)
    {
        $systime = time() - ($opt_o * 3600);
    }
    else
    {
        $systime = time();
    }

    print scalar(gmtime($systime));
    my %rsp = $currentMgr->timeSet($systime);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
        }
        else
        {
            my $msg = "ERROR: Unable to sync time.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     getTime
#
# Desc:     Get controller's time.
#
# Input:    NONE
##############################################################################
sub getTime
{
    my %rsp = $currentMgr->GetTime();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayTime(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve controller time.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

}

##############################################################################
# Name:     ipAddressSet
#
# Desc:     Set a new ip, subnet mask, and gateway.
#
# Input:    ipAdr       - long ip address
#           subNetAdr   - long subnet mask ip address
#           gateWayAdr  - long gateway ip address
#
##############################################################################
sub ipAddressSet
{
    print "\n";
    my ($serial, $ipAdr, $subNetAdr, $gateWayAdr) = @args;

    my $newIpAdr;
    my $newSubNetAdr;
    my $newGateWayAdr;

    if (!defined($serial))
    {
        print "Missing Serial Number.\n";
        return;
    }

    if (!defined($ipAdr))
    {
        print "Missing IP Address.\n";
        return;
    }
    elsif ($ipAdr eq "0")
    {
        $newIpAdr = 0;
    }
    elsif ($ipAdr !~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/)
    {
        print "Invalid IP Address.\n";
        return;
    }
    else
    {
        $newIpAdr = unpack("L", $currentMgr->ip2long($ipAdr));
    }

    if (!defined($subNetAdr))
    {
        print "Missing Subnet Mask Address.\n";
        return;
    }
    elsif ($subNetAdr eq "0")
    {
        $newSubNetAdr = 0;
    }
    elsif ($subNetAdr !~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/)
    {
        print "Invalid Subnet Mask Address.\n";
        return;
    }
    else
    {
        $newSubNetAdr = unpack("L", $currentMgr->ip2long($subNetAdr));
    }

    if (!defined($gateWayAdr))
    {
        print "Missing Gateway Address.\n";
        return;
    }
    elsif ($gateWayAdr eq "0")
    {
        $newGateWayAdr = 0;
    }
    elsif ($gateWayAdr !~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/)
    {
        print "Invalid Gateway Address.\n";
        return;
    }
    else
    {
        $newGateWayAdr = unpack("L", $currentMgr->ip2long($gateWayAdr));
    }

    my %rsp = $currentMgr->ipSet($serial, $newIpAdr, $newSubNetAdr, $newGateWayAdr);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "IP Addresses set Successfully.\n";
        }
        else
        {
            my $msg = "ERROR: Unable to set new addresses.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     ipAddressGet
#
# Desc:     Get IP address/network information.
#
# Input:    None
##############################################################################
sub ipAddressGet
{
    print "\n";

    my %rsp = $currentMgr->ipGet();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayIpInfo(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve network information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     ledControl
#
# Desc:     Sync controller's real time clock to the local pc.
#
# Input:    NONE
##############################################################################
sub ledControl
{
    print "\n";

    my ($ledStr, $value, $blink) = @args;
    my $led = 0;
    my @parmArray;

    if (!defined($ledStr)|| !defined($value))
    {
        print "Invalid or missing parameter.\n";
        return;
    }

    if (uc($ledStr) eq "ATTN")
    {
        $led = 0;
    }
    else
    {
        $led = $ledStr;
    }

    if (!defined($blink))
    {
        $blink = 0;
    }

    my %rsp = $currentMgr->ledSet($led, $value, $blink);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
        }
        else
        {
            my $msg = "ERROR: Unable to set led.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     AsciiHexToBin
#
# Desc:     Converts an ASCII hex string to packed binary data
#
# Input:    data - hex string representing the MRP input data
#           format - byte|short|word  (default: word)
##############################################################################
sub AsciiHexToBin($$)
{
    my ($data, $format) = @_;

    $data =~ s/0x//i;

    if (!defined $data) {
        print "AsciiHexToBin: No input data.\n";
        return undef;
    }

    if (!defined $format) {
        $format = "word";
    }

    if ($format !~ /^byte$|^short$|^word$/i) {
        print "AsciiHexToBin: format incorrect ($format).\n";
        return undef;
    }

    # setup to parse the input data string
    my $cCnt;
    my $cTpl;

    if ($format =~ /^byte$/i) {
        $cCnt = 2;
        $cTpl = "C";
    }
    elsif ($format =~ /^short$/i) {
        $cCnt = 4;
        $cTpl = "S";
    }
    else { # word
        $cCnt = 8;
        $cTpl = "L";
    }

    my @wData;
    my $i;
    my $template = "";
    my $length = length $data;

    if ($length % $cCnt) {
        print "AsciiHexToBin: Input data string length not correct for the\n" .
              " format selected ($format).\n";
        return undef;
    }

    # parse the input data string
    for ($i=0; $i<$length; $i+=$cCnt) {
        push @wData, oct("0x" . substr $data, 0, $cCnt);
        $data = substr $data, $cCnt;
        $template .= $cTpl;
    }

    $data = pack $template, @wData;
    return $data;
}

##############################################################################
# Name:     debugAddressSet
#
# Desc:     Calls PI_GenCommand with the parameters that are passed in.
#
# Input:    String ip address or off or default
##############################################################################
sub debugAddressSet
{
    print "\n";

    my ($ipaddr, $chan) = @args;
    my @parmArray;
    my $port;

    if (!defined($ipaddr))
    {
        print "Invalid or missing ipaddr.\n";
        return;
    }

    if (uc($ipaddr) eq "PI")
    {
        $ipaddr = 0xFFFFFFFF;
    }
    elsif (uc($ipaddr) eq "OFF")
    {
        $ipaddr = 0x0;
    }
    elsif (uc($ipaddr) eq "THIS")
    {
        ($port, $ipaddr) = sockaddr_in(getsockname($currentMgr->{'socket'}));

        if (defined($ipaddr))
        {
            $ipaddr = unpack "l", $ipaddr;
        }
        else
        {
            print "Couldn't retrieve this computer's IP address.\n";
            return;
        }
    }
    elsif ($ipaddr =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/)
    {
        $ipaddr = unpack "L", inet_aton($ipaddr);
    }
    else
    {
        print "Invalid ipaddr.\n";
        return;
    }

    if (!defined($chan))
    {
        $chan = 0;
    }
    elsif ($chan < 0 || $chan >= 20)
    {
        print "Invalid port.\n";
        return;
    }

    $parmArray[0] = $ipaddr;
    $parmArray[1] = $chan;

    my %rsp = $currentMgr->genericCommand("DEBUGADDR", @parmArray);

    if (%rsp)
    {
        print "DebugConsole address set to " . inet_ntoa(pack "L", $ipaddr) .
                ", channel $chan\n";
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     genFunction
#
# Desc:     Calls PI_GenFunction() with the parameters that are passed in.
#
# Input:    parameter1-8 - integral parameters specified in hex or decimal
##############################################################################
sub genFunction
{
    print "\n";

    my $parm;
    my @parmArray;

    # convert input paramters from hex if necessary
    foreach $parm (@args)
    {
        if ($parm =~ /^0x/i) {
            $parm = oct $parm;
        }
        push @parmArray, $parm;
    }

    my %rsp = $currentMgr->genericCommand("FUNCTION", @parmArray);

    if (%rsp)
    {
        print "PI_GenFunction() returned $rsp{STATUS}\n";
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     genMRP
#
# Desc:     Calls a "generic" MRP.
#
# Input:    mrpCmd    - the MRP command code
#           rspDataLn - the response data length
#           data      - hex string representing the MRP input data (optional)
#           -t byte|short|word  (input type off 'data' field, default: word)
#           -o byte|short|word  (output data format, default: word)
##############################################################################
sub genMRP
{
    my ($mrpCmd, $rspDataLn, $data) = @args;

    print "\n";

    # see if alternate format chosen
    my $inpFormat = "word";
    if ($opt_t) {
#        print "opt_t=$opt_t\n";
        if ($opt_t =~ /^byte$|^short$|^word$/i) {
            $inpFormat = $opt_t;
        }
        else {
            print "-t must specify 'byte', 'short', 'word'\n";
            return;
        }
    }

    # see if alternate format chosen
    my $outFormat = "word";
    if ($opt_o) {
#        print "opt_o=$opt_o\n";
        if ($opt_o =~ /^byte$|^short$|^word$/i) {
            $outFormat = $opt_o;
        }
        else {
            print "-o must specify 'byte', 'short', 'word'\n";
            return;
        }
    }

    # convert mrpCmd from hex if necessary
    if ($mrpCmd =~ /^0x/i) {
        $mrpCmd = oct $mrpCmd;
    }

    # convert rspDataLn from hex if necessary
    if ($rspDataLn =~ /^0x/i) {
        $rspDataLn = oct $rspDataLn;
    }

    if (defined $data) {
        $data = AsciiHexToBin($data, $inpFormat);;
        if (!defined $data) {
            print "The data field was NOT correct.\n";
            return;
        }
    }

    my %rsp = $currentMgr->genericMRP($mrpCmd, $rspDataLn, $data);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Generic MRP completed successfully.  Return data:\n\n";

            # After MRP data returned, format it appropriately.
            $currentMgr->FormatData($rsp{RSP_DATA}, 0x00000000,
                    $outFormat, undef, undef);

        }
        else
        {
            my $msg = "ERROR: Generic MRP FAILED!";
            displayError($msg, %rsp);
            print "\n";
            if (defined($rsp{RSP_DATA}))
            {
                $currentMgr->FormatData($rsp{RSP_DATA}, 0x00000000,
                        $outFormat, undef, undef);
            }
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     FOREIGNTARGET
#
# Desc:     Turns foreign target support on/off.
#
# Input:    'on' or 'off'
##############################################################################
sub FOREIGNTARGET
{
    my ($onoff) = @args;

    my $mrpCmd = 0x137;
    my $rspDataLn = 8;
    my $on = '0f000000';
    my $off = '00000000';

    print "\n";

    my $data;

    if (defined($onoff) && $onoff eq 'on') {
        $data = AsciiHexToBin($on, 'byte');
        if (!defined $data) {
            print "The data field was NOT correct.\n";
            return;
        }
    } elsif (defined($onoff) && $onoff eq 'off') {
        $data = AsciiHexToBin($off, 'byte');
        if (!defined $data) {
            print "The data field was NOT correct.\n";
            return;
        }
    } else {
        print "Expected on or off for argument to Foreign Target command.\n";
        return;
    }

    my %rsp = $currentMgr->genericMRP($mrpCmd, $rspDataLn, $data);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Foreign Target change completed successfully.\n";

        }
        else
        {
            my $msg = "ERROR: Foreign Target change FAILED!";
            displayError($msg, %rsp);
            print "\n";
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
    print "\n";
}   # End of FOREIGNTARGET

##############################################################################
# Name:     EMULATEPAB
#
# Desc:     EMULATES PAB, off=0, on=1, i/o count=2
#
# Input:    'on' or 'off' or 'count' or 1/0/2. (Count of outstanding I/Os)
##############################################################################
sub EMULATEPAB
{
    my ($ise, $onoff) = @args;

    if (!defined($ise) || !defined($onoff))
    {
        print " Expected two arguments to Emulate PAB command.\n";
        return;
    }
    if ($onoff eq 'on') {
        $onoff = 1;
    } elsif ($onoff eq 'off') {
        $onoff = 0;
    } elsif ($onoff eq 'count') {
        $onoff = 2;
    }
    if ($onoff < 0 || $onoff > 2) {
        print " Expected on/off/count (1,0,2) for argument to Emulate PAB command.\n";
        return;
    }

    my $mrpCmd = 0x17B;
    my $rspDataLn = 8;
    my $data = pack('SS', $ise, $onoff);

    my %rsp = $currentMgr->genericMRP($mrpCmd, $rspDataLn, $data);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my ($r1) = unpack("C", $rsp{RSP_DATA});
            if ($onoff != 2) {
                my $str = 'UNKNOWN';
                if ($onoff == 0) {
                    $str = 'OFF';
                } elsif ($onoff == 1) {
                    $str = 'ON';
                }
                print " EMULATE PAB for ISE=$ise $str ($onoff).\n";
            }
            printf " Count of outstanding I/Os for ISE=$ise count=$r1\n";
        }
        else
        {
            my $msg = " ERROR: EMULATE PAB FAILED!";
            displayError($msg, %rsp);
            print "\n";
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
}   # End of EMULATEPAB

##############################################################################
# Name:     SWAPPIDS
#
# Desc:     SWAPS two PIDS.
#
##############################################################################
sub SWAPPIDS
{
    my ($pid1, $pid2) = @args;

    if (!defined($pid1) || !defined($pid2))
    {
        print " Expected two arguments to SWAPPIDS command.\n";
        return;
    }

    my $mrpCmd = 0x17A;
    my $rspDataLn = 8;
    my $data = pack('SS', $pid1, $pid2);

    my %rsp = $currentMgr->genericMRP($mrpCmd, $rspDataLn, $data);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print " SWAPPIDS for $pid1 and $pid2\n";
        }
        else
        {
            my $msg = " ERROR: SWAPPIDS FAILED!";
            displayError($msg, %rsp);
            print "\n";
        }
    }
    else
    {
        print " ERROR: Did not receive a response packet.\n";
        logout();
    }
}   # End of SWAPPIDS

##############################################################################
# Name:     ISEIPS
#
# Desc:     Get ISE IP addresses
#
# Input:    BAY ID of ISE to get IP addresses for.
##############################################################################
sub ISEIPS
{
    my ($bid) = @args;

    if (!defined($bid))
    {
        print " Expected one argument, the bay ID.\n";
        return;
    }

    my $mrpCmd = 0x174;
    my $rspDataLn = 28;
    my $data = pack('SS', $bid, 0);

    my %rsp = $currentMgr->genericMRP($mrpCmd, $rspDataLn, $data);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my ($rh1,$rh2,$rbid,$resvd,$ip1,$ip2,$wwn1,$wwn2) = unpack("LLSS LLLL", $rsp{RSP_DATA});
            my @wwn;
            $wwn[0] = $wwn1 & 0xff;
            $wwn[1] = ($wwn1 >> 8) & 0xff;
            $wwn[2] = ($wwn1 >> 16) & 0xff;
            $wwn[3] = ($wwn1 >> 24) & 0xff;
            $wwn[4] = $wwn2 & 0xff;
            $wwn[5] = ($wwn2 >> 8) & 0xff;
            $wwn[6] = ($wwn2 >> 16) & 0xff;
            $wwn[7] = ($wwn2 >> 24) & 0xff;
            printf " Bay ID=$rbid wwn=%02x%02x%02x%02x%02x%02x%02x%02x ip1=%s ip2=%s\n", @wwn, $currentMgr->long2ip($ip1),$currentMgr->long2ip($ip2);
        }
        else
        {
            my $msg = " ERROR: ISEIPS FAILED!";
            displayError($msg, %rsp);
            print "\n";
        }
    }
    else
    {
        print " ERROR: Did not receive a response packet.\n";
        logout();
    }
}   # End of ISEIPS

##############################################################################
# Name:     Jumpers
#
# Desc:     printout the status of the BE port flags.
#
# Input:    none
##############################################################################
sub Jumpers
{

    # 116 = MRBELOOP
    my $mrpCmd = 0x116;
    my $rspDataLn = 256;
    my $data;
    my $port;
    my %rsp;

    print "\n";

    foreach $mrpCmd ( 0x116, 0x0500)
    {
    for ($port = 0; $port < 16; $port++)
    {
        $data = "0000000" . "$port";
        $data = AsciiHexToBin($data, "word");;
        %rsp = $currentMgr->genericMRP($mrpCmd, $rspDataLn, $data);
        if (%rsp)
        {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $unpackc = "C" x 74;
            my @array = unpack $unpackc, $rsp{RSP_DATA};
    #       printf STDERR "port $port STATUS=%02X\n", $array[73];
            # PORT_GPIOD_J2_JUMPER_MASK = 0x04
            # PORT_GPIOD_ACTIVE_MASK = 0x08
            if ($array[73] == 0)
            {
            printf "%s port $port information not available\n",
                ($mrpCmd == 0x116)?"STORAGE":"HOST   ";
            }
            else
            {
            printf "%s port $port %s Jumper J2 is %s\n",
                ($mrpCmd == 0x116)?"STORAGE":"HOST   ",
                ($array[73] & 0x08)==0?"inactive":"active  ",
                ($array[73] & 0x04)==0?"INCORRECT/BAD/ERROR":"OK";
            }
        }
        else
        {
            # Ignore if message is Invalid port number.
            if ($rsp{ERROR_CODE} == 0x36)
            {
            last;
            }
            my $msg = "ERROR: Generic MRP FAILED!";
            displayError($msg, %rsp);
            print "\n";
            last CMD;
        }
        }
        else
        {
        print "ERROR: Did not receive a response packet.\n";
        logout();
        last;
        }
    }
    }
}

##############################################################################
# Name:     cacheTest
#
# Desc:     Test the cache on the CCB.
#
# Input:    test    - Test to run or Stop to Stop
#           timeOut - Time between tests
##############################################################################
sub cacheTest
{
    print "\n";

    my ($test, $timeOut) = @args;
    my @parmArray;
    my $testAction = 0;
    my $testRun    = 0;
    my %rsp;

    if (!defined($timeOut))
    {
        $timeOut = 0;
    }

    if (!defined($test))
    {
        print "Missing test.\n";
        return;
    }

    if (uc($test) eq "CHANGE")
    {
        $testAction = $currentMgr->PI_GENERIC_CACHE_TEST_START;
        $testRun = $currentMgr->PI_GENERIC_CACHE_TEST_ASYNC;
        $timeOut = 0;
    }
    elsif (uc($test) eq "X1")
    {
        $testAction = $currentMgr->PI_GENERIC_CACHE_TEST_START;
        $testRun = $currentMgr->PI_GENERIC_CACHE_TEST_X1;
    }
    elsif (uc($test) eq "CACHE")
    {
        $testAction = $currentMgr->PI_GENERIC_CACHE_TEST_START;
        $testRun = $currentMgr->PI_GENERIC_CACHE_TEST_CACHE;
    }
    elsif (uc($test) eq "STOP")
    {
        $testAction = $currentMgr->PI_GENERIC_CACHE_TEST_STOP;
    }
    else
    {
        print "Invalid test $test. try (CHANGE|X1|CACHE|STOP)\n";
        return;
    }

    if (uc($test) eq "CHANGE")
    {
        if ($opt_P) {
            $timeOut |= $currentMgr->X1_ASYNC_PCHANGED;
        }
        if ($opt_R) {
            $timeOut |= $currentMgr->X1_ASYNC_RCHANGED;
        }
        if ($opt_V) {
            $timeOut |= $currentMgr->X1_ASYNC_VCHANGED;
        }
        if ($opt_H) {
            $timeOut |= $currentMgr->X1_ASYNC_HCHANGED;
        }
    #    if ($opt_x) {
    #        $timeOut |= $currentMgr->X1_ASYNC_ACHANGED;
    #    }
        if ($opt_Z) {
            $timeOut |= $currentMgr->X1_ASYNC_ZCHANGED;
        }
        if ($opt_A) {
            $timeOut |= $currentMgr->X1_ASYNC_VCG_ELECTION_STATE_CHANGE;
        }
        if ($opt_B) {
            $timeOut |= $currentMgr->X1_ASYNC_VCG_ELECTION_STATE_ENDED;
        }
        if ($opt_C) {
            $timeOut |= $currentMgr->X1_ASYNC_VCG_POWERUP;
        }
        if ($opt_D) {
            $timeOut |= $currentMgr->X1_ASYNC_VCG_CFG_CHANGED;
        }
    }

    push @parmArray, $testAction;
    push @parmArray, $testRun;
    push @parmArray, $timeOut;


    %rsp = $currentMgr->genericCommand("CACHE_TEST", @parmArray);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Test Start Successful.\n";
        }
        else
        {
            my $msg = "Test Start Unuccessful.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     CacheRefreshCCB
#
# Desc:     Refresh the cache on the CCB.
#
# Input:    cacheMask           - Which cache(s) to refresh
#           waitForCompletion   - TRUE=wait
##############################################################################
sub CacheRefreshCCB
{
    print "\n";

    my ($cacheMask, $waitForCompletion) = @args;
    my %rsp;

    if (!defined($cacheMask))
    {
        print "Missing cacheMask.\n";
        return;
    }
    elsif ($cacheMask =~ /^0x/i)
    {
        $cacheMask = oct $cacheMask;
    }


    if (!defined($waitForCompletion))
    {
        $waitForCompletion = 1;
    }
    elsif (uc($waitForCompletion) eq "TRUE")
    {
        $waitForCompletion = 1;
    }
    else
    {
        $waitForCompletion = 0;
    }


    %rsp = $currentMgr->RefreshCCBCahe($cacheMask, $waitForCompletion);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "CCB Cache Refresh Successful.\n";
        }
        else
        {
            my $msg = "CCB Cache Refresh Unuccessful.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     Alink
#
# Desc:     Create APOOL or ALINK, or see what's possible.
#
# Input:    APOOL size          - Size of APOOL to create.
#           WHAT                - See possible disks we can link to.
#           vid cid ord         - controller and vdisk number to create alink.
#           DELETE vid          - Delete alink.
#           LINK vid            - Turn vlink into alink.
#           UNLINK vid          - Turn alink into vlink.
##############################################################################
sub Alink
{
    my ($vid, $cid, $ord) = @args;

    # Need to turn on vdisk attribute hidden (0x01) for APOOL or ALINK.
#    my $HIDDEN_ATTRIBUTE = 0x01;
    my $ASYNC_ATTRIBUTE = 0x04;
    my $VLINK_ATTRIBUTE = 0x80;
    my $CACHE_ATTRIBUTE = 0x0100;
    my $ASYNC_CACHE;
    my $VLINK_CACHE;
    my @pids;

    if ($currentType eq '7000' || $currentType eq '4700')
    {
        $ASYNC_CACHE = $ASYNC_ATTRIBUTE | $CACHE_ATTRIBUTE;
        $VLINK_CACHE = $VLINK_ATTRIBUTE | $CACHE_ATTRIBUTE;
    } else {
        $ASYNC_CACHE = $ASYNC_ATTRIBUTE;
        $VLINK_CACHE = $VLINK_ATTRIBUTE;
    }

    print "\n";

    if (!defined($vid))
    {
        print "Missing arguments.\n";
        return;
    }

    if ($vid =~ /^WHAT$/i)
    {
        my %rsp = $currentMgr->virtualLinkCtrlCount();
        my $cnt;

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                print "Number of remote controllers: " . $rsp{COUNT} . "\n";
                $cnt = $rsp{COUNT};
            }
            else
            {
                my $msg = "Unable to retrieve number of remote controllers.";
                displayError($msg, %rsp);
                return;
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet (to virtualLinkCtrlCount).\n";
            logout();
            return
        }

        print "CID ORD LUN RTYPE CLUSTER ATTR CAPACITY SERIAL_NUM VID1 VID2 SCNT VLCNT VDNAME\n";
        print "--- --- --- ----- ------- ---- -------- ---------- ---- ---- ---- ----- ------\n";
        for (my $j = 0; $j < $cnt; $j++)
        {
            undef(%rsp);
            %rsp = $currentMgr->virtualLinkCtrlVDisks($j);
            if (%rsp)
            {
                if ($rsp{STATUS} == PI_GOOD)
                {
                    if ($rsp{COUNT} > 0)
                    {
# Following changed from following routine.
#                        $currentMgr->displayVirtualLinkCtrlVDisks(%rsp);
                        for (my $i = 0; $i < $rsp{COUNT}; ++$i)
                        {
                            printf "%3u %3u %3u  0x%02X     %3u 0x%02X",
                                    $j, $i,
                                    $rsp{VDDS}[$i]{LUN},
                                    $rsp{VDDS}[$i]{RTYPE},
                                    $rsp{VDDS}[$i]{CLUSTER},
                                    $rsp{VDDS}[$i]{ATTR};
                            printf "%9u", $rsp{VDDS}[$i]{DEVCAP};
                            printf "   %8lu  %3u  %3u  %3u   %3u",
                                    $rsp{VDDS}[$i]{SSERIAL},
                                    $rsp{VDDS}[$i]{VID1},
                                    $rsp{VDDS}[$i]{VID2},
                                    $rsp{VDDS}[$i]{SCNT},
                                    $rsp{VDDS}[$i]{VLCNT};
                            printf " " . $rsp{VDDS}[$i]{VDNAME};
                            print  "\n";
                        }

                    }
                }
                else
                {
                    my $msg = "Unable to receive information about remote controller (to virtualLinkCtrlVDisks).\n";
                    displayError($msg, %rsp);
                }
            }
            else
            {
                print "ERROR: Did not receive a response packet (to virtualLinkCtrlVDisks).\n";
                logout();
                return;
            }
        }
        return;
    }

    # If to create an apool of specified size.
    if ($vid =~ /^APOOL$/i)
    {
        if (!defined($cid))
        {
            print "Invalid or missing capacity.\n";
            return;
        }
        my $capacity = $cid;
        # We need to find which disks to create it on.
        my %pdisks = $currentMgr->physicalDisks();
        my $usable_disks = 0;
        for (my $i = 0; $i < $pdisks{COUNT}; $i++)
        {
            # Must be operational disk.
            if ($pdisks{PDISKS}[$i]{PD_DEVSTAT} != DEVSTATUS_OPERATIONAL) { next; }
            # Must be Fibre Channel disk.
            if ($pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_FC_DISK &&
                $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_ISE_HIGH_PERF &&
                $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_ISE_PERF &&
                $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_ISE_BALANCE &&
                $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_ISE_CAPACITY) { next; }
            # Must be data disk.
            if ($pdisks{PDISKS}[$i]{PD_CLASS} == LABEL_TYPE_DATA)
            {
                $pids[$usable_disks] = $pdisks{PDISKS}[$i]{PD_PID};
                ++$usable_disks;
            }
        }
        if ($usable_disks <= 0)
        {
            print "No physical disks labeled as type DATA. use PDISKLABEL.\n";
            return;
        }
        # Must be vdisk 0.
        my $vdisk = 0;
        # Determine raid 10 parameters for all data disks.
        my %raidParms = $currentMgr->calcRaidParms(\@pids, RAID_10);
        my $stripe = $raidParms{STRIPE_SIZE};
        my $depth = $raidParms{MIRROR_DEPTH};
        my $parity = $raidParms{PARITY};
        my $maxraids = 4;
        my $threshold = 10;
        my $flags = 0;
        my $minPD = 0;

        my %rsp = $currentMgr->virtualDiskCreate($capacity,
                    \@pids, RAID_10, $stripe, $depth, $parity, $vdisk,
                    $maxraids, $threshold, $flags, $minPD);
        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                $currentMgr->displayVirtualDiskCreate(%rsp);
            }
            else
            {
                my $msg = "Unable to create virtual disk $vdisk of size $capacity.";
                displayError($msg, %rsp);
                return;
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet (to virtualDiskCreate).\n";
            logout();
            return;
        }

        # Need to turn on vdisk attribute hidden (0x01) for vdisk 0.
        %rsp = $currentMgr->virtualDiskSetAttributes(0, $ASYNC_CACHE);
        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                printf("Virtual disk 0 set attributes: 0x%04X Successful.\n", $ASYNC_CACHE);
            }
            else
            {
                my $msg = sprintf("Virtual disk 0 set attributes: 0x%04X Unsuccessful.\n", $ASYNC_CACHE);
                displayError($msg, %rsp);
                return;
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet (to virtualDiskSetAttributes).\n";
            logout();
            return;
        }
        return;
    }

    # Set alink attribute on an alink.
    if ($vid =~ /^LINK$/i)
    {
        my $link_vid = $cid;
        # Need to turn on vdisk attribute hidden (0x01) for vdisk (with 0x80 = vlink).
        my %rsp = $currentMgr->virtualDiskSetAttributes($link_vid, $VLINK_CACHE | $ASYNC_CACHE);
        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                printf("Set virtual disk $link_vid attributes: 0x%04X Successful.\n", $VLINK_CACHE | $ASYNC_CACHE);
            }
            else
            {
                my $msg = sprintf("Virtual disk $link_vid set attributes: 0x%04X Unsuccessful.\n", $VLINK_CACHE | $ASYNC_CACHE);
                displayError($msg, %rsp);
                return;
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet (to virtualDiskSetAttributes).\n";
            logout();
            return;
        }
        return;
    }

    # Unset alink attribute on an alink.
    if ($vid =~ /^DELETE$/i || $vid =~ /^UNLINK$/i)
    {
        my $unlink_vid = $cid;
        # Need to turn off vdisk attribute hidden (0x01) for vdisk (with 0x80 = vlink).
        my %rsp = $currentMgr->virtualDiskSetAttributes($unlink_vid, $VLINK_CACHE);
        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                printf("Set virtual disk $unlink_vid attributes: 0x%04X Successful.\n", $VLINK_CACHE);
            }
            else
            {
                my $msg = sprintf("Virtual disk $unlink_vid set attributes: 0x%04X Unsuccessful.\n", $VLINK_CACHE);
                displayError($msg, %rsp);
                return;
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet (to virtualDiskSetAttributes).\n";
            logout();
            return;
        }
        if ($vid =~ /^UNLINK$/i)
        {
            return;
        }
    }

    # Delete an alink.
    if ($vid =~ /^DELETE$/i)
    {
        my $delete_vid = $cid;
        my %rsp = $currentMgr->virtualDiskDelete($delete_vid);
        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                printf("Virtual disk $delete_vid deleted.\n");
            }
            else
            {
                my $msg = sprintf("Unable to delete virtual disk $delete_vid.\n");
                displayError($msg, %rsp);
                return;
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet (to virtualDiskDelete).\n";
            logout();
            return;
        }
        return;
    }

    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    # Creating an alink if here, need (vid cid ord).
    # First set the vlink static data area in the Proc via vlinkctrlvdisks.

    my %rsp;
    %rsp = $currentMgr->virtualLinkCtrlVDisks($cid);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
#            $currentMgr->displayVirtualLinkCtrlVDisks(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve remote controller virtual disks.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    # Now create the vlink.
    %rsp = $currentMgr->virtualLinkCreate($cid, $ord, $vid);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVirtualLinkCreate(%rsp);
        }
        else
        {
            my $msg = "Unable to create remote controller vlink (virtualLinkCreate).";
            displayError($msg, %rsp);
            return;
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet (to virtualLinkCreate).\n";
        logout();
        return;
    }

    # Need to turn on vdisk attribute hidden (0x01) for vdisk (with 0x80 = vlink).
    %rsp = $currentMgr->virtualDiskSetAttributes($vid, $ASYNC_CACHE | $VLINK_CACHE);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            printf("Set virtual disk $vid attributes: 0x%04X Successful.\n", $ASYNC_CACHE | $VLINK_CACHE);
        }
        else
        {
            my $msg = sprintf("Virtual disk $vid set attributes: 0x%04X Unsuccessful.\n", $ASYNC_CACHE | $VLINK_CACHE);
            displayError($msg, %rsp);
            return;
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet (to virtualDiskSetAttributes).\n";
        logout();
        return;
    }
}

##############################################################################
# Name:     disasterTest
#
# Desc:     Test the disaster mode handling on the CCB.
#
# Input:    flag    - Parameter passed to CCB
##############################################################################
sub disasterTest
{
    print "\n";

    my ($flag) = @args;
    my %rsp;

    if (!defined($flag))
    {
        print "Missing flag.\n";
        return;
    }

    %rsp = $currentMgr->genericCommand("DISASTER_TEST", $flag);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Disaster Test Successful.\n";
        }
        else
        {
            my $msg = "Disaster Test Unuccessful.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     keepAliveTest
#
# Desc:     Test the disaster mode handling on the CCB.
#
# Input:    flag    - Parameter passed to CCB
##############################################################################
sub keepAliveTest
{
    print "\n";

    my ($flag, $slot) = @args;
    my %rsp;

    if (!defined($flag))
    {
        print "Missing flag.\n";
        return;
    }

    if (!defined($slot))
    {
        if ( ($flag == PI_GENERIC_KEEP_ALIVE_TEST_RESET) ||
            ($flag == PI_GENERIC_KEEP_ALIVE_TEST_DISABLE) ||
            ($flag == PI_GENERIC_KEEP_ALIVE_TEST_ENABLE) )
        {
            $slot = 0;
        }
        else
        {
            print "Missing slot.\n";
            return;
        }
    }

    %rsp = $currentMgr->genericCommand("KEEP_ALIVE_TEST", $flag, $slot);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "KeepAlive Test Successful.\n";
        }
        else
        {
            my $msg = "KeepAlive Test Unuccessful.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     fioMapTest
#
# Desc:     Test the disaster mode handling on the CCB.
#
# Input:    flag    - Parameter passed to CCB
##############################################################################
sub fioMapTest
{
    print "\n";

    my ($parm1, $parm2, $parm3) = @args;
    my %rsp;

    if (!defined($parm1))
    {
        print "Missing parm1.\n";
        return;
    }

    if (!defined($parm2))
    {
        print "Missing parm2.\n";
        return;
    }

    if (!defined($parm3))
    {
        if ( ($parm2 == PI_GENERIC_FIO_MAP_TEST_RESET) ||
            ($parm2 == PI_GENERIC_FIO_MAP_TEST_CLEAR) )
        {
            $parm3 = 0;
        }
        else
        {
            print "Missing parm3\n";
            return;
        }
    }

    %rsp = $currentMgr->genericCommand("FIO_MAP_TEST", $parm1, $parm2, $parm3);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "FIO Map Test Successful.\n";
        }
        else
        {
            my $msg = "FIO Map Test Unuccessful.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     fcmCounterTest
#
# Desc:     Test the FCAL counter routines
#
# Input:    flag    - Parameter passed to CCB
##############################################################################
sub fcmCounterTest
{
    print "\n";

    my ($parm1, $parm2, $parm3) = @args;
    my %rsp;

    if (!defined($parm1))
    {
        print "Missing parm1\n";
        return;
    }

    $parm2 = 0;
    $parm3 = 0;

    %rsp = $currentMgr->genericCommand("FCM_COUNTER_TEST", $parm1, $parm2, $parm3);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "FCM Counter Test Successful.\n";
        }
        else
        {
            my $msg = "FCM Counter Test Unuccessful.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     logEvent
#
# Desc:     Logs an event to the CCB.
#
# Input:    logdata
##############################################################################
sub logEvent
{
    print "\n";

    my ($logdata) = @args;
    my @parmArray;
    my %rsp;

    if (!defined($logdata))
    {
        print "Missing logdata.\n";
        return;
    }

    push @parmArray, $logdata;

    %rsp = $currentMgr->genericCommand("LOG_EVENT", @parmArray);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Log Event Successful.\n";
        }
        else
        {
            my $msg = "Log Event Unsuccessful.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     errorTrap
#
# Desc:     Calls the failure manager with the corresponding failure type.
#
# Input:    type    (CCB|FE|BE|ALL)
##############################################################################
sub errorTrap
{
    print "\n";

    my ($type) = @args;
    my @parmArray;
    my $cmd;

    if (!defined($type))
    {
        print "Invalid or missing type.\n";
        return;
    }

    if (uc($type) eq "CCB")
    {
        $cmd = $currentMgr->PI_GENERIC_ERROR_TRAP_CCB;
    }
    elsif (uc($type) eq "FE")
    {
        $cmd = $currentMgr->PI_GENERIC_ERROR_TRAP_FE;
    }
    elsif (uc($type) eq "BE")
    {
        $cmd = $currentMgr->PI_GENERIC_ERROR_TRAP_BE;
    }
    elsif (uc($type) eq "ALL")
    {
        $cmd = $currentMgr->PI_GENERIC_ERROR_TRAP_ALL;
    }
    else
    {
        print "Invalid type. valid types are (CCB|FE|BE|ALL).\n";
        return;
    }
    push @parmArray, $cmd;

    my %rsp = $currentMgr->genericCommand("ERROR_TRAP", @parmArray);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Error Trap Successful.\n";
        }
        else
        {
            my $msg = "Error Trap Unsuccessful.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     failureManagerTest
#
# Desc:     Calls the failure manager with the corresponding failure type.
#
# Input:    type    -   contoller
#                       interface
##############################################################################
sub failureManagerTest
{
    print "\n";

    my ($action, $type, $csn, $interface) = @args;
    my @parmArray;
    my $cmd;
    my %rsp;

    if (!defined($action))
    {
        print "Invalid or missing action.\n";
        return;
    }
    if (uc($action) ne "F" && uc($action) ne "U")
    {
        print "Invalid action. valid actions are (F|U).\n";
        return;
    }

    if (!defined($type))
    {
        print "Invalid or missing type.\n";
        return;
    }

    if (!defined($csn))
    {
        print "Invalid or missing controller serial number.\n";
        return;
    }

    if (uc($type) eq "CONT" && uc($action) eq "F")
    {
        $cmd = $currentMgr->FAILURE_MANAGER_FAIL_CONTROLLER;
    }
    elsif (uc($type) eq "INT" && uc($action) eq "F")
    {
        $cmd = $currentMgr->FAILURE_MANAGER_FAIL_INTERFACE;
    }
    elsif (uc($type) eq "CONT" && uc($action) eq "U")
    {
        $cmd = $currentMgr->RESOURCE_MANAGER_UNFAIL_CONTROLLER;
    }
    elsif (uc($type) eq "INT" && uc($action) eq "U")
    {
        $cmd = $currentMgr->RESOURCE_MANAGER_UNFAIL_INTERFACE;
    }
    else
    {
        print "Invalid type.\n";
        return;
    }
    push @parmArray, $cmd;
    push @parmArray, $csn;

    if (!defined($interface) && (($cmd == FAILURE_MANAGER_FAIL_INTERFACE) ||
                                 ($cmd == RESOURCE_MANAGER_UNFAIL_INTERFACE)))
    {
        print "Invalid or missing interface.\n";
        return;
    }
    elsif (($cmd == FAILURE_MANAGER_FAIL_INTERFACE) ||
           ($cmd == RESOURCE_MANAGER_UNFAIL_INTERFACE))
    {
        push @parmArray, $interface;
    }


    # The Unfail Interface function has its own packet interface command.
    # The other functions use the generic command request.
    if ($cmd == RESOURCE_MANAGER_UNFAIL_INTERFACE)
    {
        %rsp = $currentMgr->UnfailInterface($csn, $interface);
    }
    elsif ($cmd == FAILURE_MANAGER_FAIL_INTERFACE)
    {
        %rsp = $currentMgr->FailInterface($csn, $interface);
    }
    else
    {
        %rsp = $currentMgr->genericCommand("FAILURE", @parmArray);
    }

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Failure Manager Notified.\n";
        }
        else
        {
            my $msg = "ERROR: Unable to intialize failure manager.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     interfaceFail
#
# Desc:     Calls the failure manager to fail an interface
#
# Input:    ctrlSN      - controller serial number
#           interface   - interface ID
##############################################################################
sub interfaceFail
{
    print "\n";

    my ($ctrlSN, $interface) = @args;
    my %rsp;

    if (!defined($ctrlSN))
    {
        print "Missing controller serial number.\n";
        return;
    }

    if (!defined($interface))
    {
        print "Missing interface ID.\n";
        return;
    }

    %rsp = $currentMgr->FailInterface($ctrlSN, $interface);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Fail interface requested.\n";
        }
        else
        {
            my $msg = "ERROR: Unable to fail interface.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     interfaceUnfail
#
# Desc:     Calls the failure manager to unfail an interface
#
# Input:    ctrlSN      - controller serial number
#           interface   - interface ID
##############################################################################
sub interfaceUnfail
{
    print "\n";

    my ($ctrlSN, $interface) = @args;
    my %rsp;

    if (!defined($ctrlSN))
    {
        print "Missing controller serial number.\n";
        return;
    }

    if (!defined($interface))
    {
        print "Missing interface ID.\n";
        return;
    }

    %rsp = $currentMgr->UnfailInterface($ctrlSN, $interface);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Unfail interface requested.\n";
        }
        else
        {
            my $msg = "ERROR: Unable to unfail interface.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     sosTable
#
# Desc:     Displays sos table.
#
# Input:    pid    -   Physical disk id
##############################################################################
sub sosTable
{
    print "\n";

    my ($pid) = @args;

    my @parmArray;

    if (!defined($pid))
    {
        print "Invalid or missing pid.\n";
        return;
    }
    my %rsp = $currentMgr->getSos($pid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displaySos(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "ERROR: Could not retrieve SOS table.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     getReport
#
# Desc:     Retrieves a debug report.
#
# Input:    Which report (one of: HEAP, TRACE, PCB, PROFILE, PACKET)
#           file - file to write the data to.
#           -l link-map       Name of linker map file
##############################################################################
sub getReport
{
    my ($report, $outfile) = @args;

    print "\n";

    if (!defined($report) || $report !~ /^HEAP$|^TRACE$|^PCB$|^PROFILE$|^PACKET$/i)
    {
        print "\n";
        print "Invalid or missing report identifier.\n";
        return;
    }

    # see if an output goes to a file
    if ($opt_f )
    {
        print "Please specify \"outfile\" as the last parameter on the ".
              "command line.\n\"Help GETREPORT\" for details.\n";
        return;
    }

    if (!defined($outfile) and
            $report =~ /^TRACE$|^PCB$|^PROFILE$|^PACKET$/i)
    {
        print "Missing \"outfile\" parameter.\n";
        return;
    }

    my $binfile;
    if (defined($outfile))
    {
        $binfile = "$outfile-bin";
    }

    # see if a link-map specified
    my $linkmap;
    if ($opt_l) {
        if ($report !~ /^PCB$|^PROFILE$/i)
        {
            print "No link file needed for a $report report, option ignored.\n";
        }
        else
        {
            $linkmap = $opt_l;
        }
    }
    else
    {
        if ($report =~ /^PCB$|^PROFILE$/i)
        {
            print "-l must reference an existing linker map file\n";
            return;
        }
    }

    my %rsp = $currentMgr->generic2Command($report);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            #
            # Process HEAP data
            #
            if ($report =~ /^HEAP$/i)
            {
                my $msg = $currentMgr->FormatHeapData($rsp{DATA}, $outfile);
                if (defined($msg))
                {
                    print $msg;
                }
            }

            #
            # Process TRACE data
            #
            if ($report =~ /^TRACE$/i)
            {
                print "Please call \"FIDREAD 256\" to pull down trace data\n";
                return;
            }

            #
            # Process PACKET data
            #
            if ($report =~ /^PACKET$/i) {
                print "Writing PACKET stats data to $binfile ...\n";
                my $rc = open OF, ">$binfile";
                if (!$rc) {
                    print "Couldn't open $binfile ...\n";
                }
                else {
                    binmode OF;
                    print OF $rsp{DATA};
                    close OF;
                    $rc = runcmd("perl PackDec.pl $binfile");
                    if ($rc == 0)
                    {
                        print "Renaming $binfile-out => $outfile\n";
                        rename "$binfile-out", $outfile;
                    }
                }
            }

            #
            # Process PCB data
            #
            if ($report =~ /^PCB$/i) {
                print "Writing pcb data to $binfile ...\n";
                my $rc = open OF, ">$binfile";
                if (!$rc) {
                    print "Couldn't open $binfile ...\n";
                }
                else {
                    binmode OF;
                    print OF $rsp{DATA};
                    close OF;
                    if (defined($linkmap)) {
                        if (!isBigfoot()) {$rc = runcmd("perl WK_PCBDec.pl -l $linkmap $binfile");}
                        else {$rc = runcmd("perl PCBDec.pl -l $linkmap $binfile");}
                        if ($rc == 0)
                        {
                            print "Renaming $binfile-out => $outfile\n";
                            rename "$binfile-out", $outfile;
                        }
                    }
                    else {
                        print "Renaming $binfile => $outfile\n";
                        rename "$binfile", $outfile;
                    }
                }
            }

            #
            # Process PROFILE data
            #
            if ($report =~ /^PROFILE$/i) {
                print "Writing profile data to $binfile ...\n";
                my $rc = open OF, ">$binfile";
                if (!$rc) {
                    print "Couldn't open $binfile ...\n";
                }
                else {
                    binmode OF;
                    print OF $rsp{DATA};
                    close OF;
                    if (defined($linkmap)) {
                        $rc = runcmd("perl ProfDec.pl -l $linkmap $binfile");
                        if ($rc == 0)
                        {
                            print "Renaming $binfile-out => $outfile\n";
                            rename "$binfile-out", $outfile;
                        }
                    }
                    else {
                        print "Renaming $binfile => $outfile\n";
                        rename "$binfile", $outfile;
                    }
                }
            }

        }
        else
        {
            my $msg = "Unable to retrieve report data.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# NAME:     globalCacheInfo
#
# DESC:     Displays global cache information.
#
# INPUT:    NONE
##############################################################################
sub globalCacheInfo
{
    print "\n";

    my %rsp = $currentMgr->globalCacheInfo();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $gcimsg = $currentMgr->displayGlobalCacheInfo(%rsp);
            print $gcimsg;
        }
        else
        {
            my $msg = "ERROR: Unable to retrieve global cache information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     globalCacheSet
#
# Desc:     Sets the global caching mode.
#
# Input:    MODE    = Caching mode
#                       OFF = Caching disabled
#                       ON = Caching enabled
##############################################################################
sub globalCacheSet
{
    my ($mode) = @args;

    print "\n";

    if (!defined($mode))
    {
        print "Invalid or missing mode.\n";
        return;
    }

    if (uc($mode) eq "OFF")
    {
        $mode = 0x80;
    }
    elsif (uc($mode) eq "ON")
    {
        $mode = 0x81;
    }

    if ($mode != 0x80 && $mode != 0x81)
    {
        print "Invalid mode specified, valid values are:\n";
        print "  OFF = Caching disabled\n";
        print "  ON = Caching enabled\n";
        return;
    }

    my %rsp = $currentMgr->globalCacheSet($mode);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            if ($mode == 0x80)
            {
                print "Global caching disabled.\n";
            }
            else
            {
                print "Global caching enabled.\n";
            }
        }
        else
        {
            my $msg = "Unable to set global caching mode.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     initProcNVRAM
#
# Desc:     Initialize PROC NVRAM
#
# Input:    intiProcNVRAM
##############################################################################
sub initProcNVRAM
{
    my ($mode) = @args;

    print "\n";

    if (!defined($mode))
    {
        $mode = "ALL";
    }

    if (uc($mode) eq "ALL")
    {
        $mode = PROC_INIT_NVRAM;
    }
    elsif (uc($mode) eq "FE")
    {
        $mode = PROC_INIT_FE_NVA_RECORDS;
    }
    elsif (uc($mode) eq "NMI")
    {
        $mode = PROC_INIT_NMI_COUNTS;
    }
    elsif (uc($mode) eq "BE")
    {
        $mode = PROC_INIT_BE_NVA_RECORDS;
    }
    else
    {
        print "Invalid or missing mode.\n";
        return;
    }

    my %rsp = $currentMgr->initProcNVRAM($mode);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Initialization of PROC NVRAM complete.\n";
        }
        else
        {
            my $msg = "ERROR: Unable to intialize PROC NVRAM.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     initCcbNVRAM
#
# Desc:     Initialize CCB NVRAM
#
# Input:    NONE
##############################################################################
sub initCcbNVRAM
{
    print "\n";

    my ($type) = @args;

    print "\n";

    if (!defined($type))
    {
        $type = "FULL";
    }

    if (uc($type) eq "FULL")
    {
        $type = INIT_CCB_NVRAM_TYPE_FULL;
    }
    elsif (uc($type) eq "LICENSE")
    {
        $type = INIT_CCB_NVRAM_TYPE_LICENSE;
    }
    else
    {
        print "Invalid or missing type.\n";
        return;
    }

    my %rsp = $currentMgr->initCCBNVRAM($type);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Initialization of CCB NVRAM complete.\n";
        }
        else
        {
            my $msg = "ERROR: Unable to intialize CCB NVRAM.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     logClear
#
# Desc:     Clear the log entries on the current connecion.
#
# Input:    None
##############################################################################
sub logClear
{
    $currentMgr->logClear();
}

##############################################################################
# Name:     login
#
# Desc:     Establish connection to a Bigfoot controller
#
# Input:    IP Address
##############################################################################
sub login
{
    my ($ip_port) = @args;

    my $rc;

    # Make sure that IP (and optionally port) was passed on the command line
    if (!defined($ip_port))
    {
        print "\n";
        print "Missing IP Address.\n";
        return;
    }

    # If the port number was passed it was done with a colon separator
    my ($ip, $port) = split /:/, $ip_port;

    if (!defined($port))
    {
        # If a port number was not passed on the command line default to 3000
        $port = $defaultPort;
    }

    my $obj = XIOTech::cmdMgr->new(\*STDOUT);

    if (defined($obj))
    {
        if ($noVerboseOutput == 0) {
            print "\n";
            print "Attempting to establish connection to ($ip:$port)...\n";
        }

        $rc = $obj->login($ip, $port, $noVerboseOutput);

        if ($rc)
        {
            # reuse open slot if possible
            $currentConnection = -1;
            for (my $i=0; $i<$numNextConnection; $i++)
            {
                if (!defined$connections{$i})
                {
                    $currentConnection = $i;
                    last;
                }
            }

            if ($currentConnection == -1)
            {
                $currentConnection = $numNextConnection++;
            }

            $currentIP = $ip;
            $currentPort = $port;
            $currentMgr = $obj;

            if (defined($currentMgr->{CONTROLLER_TYPE}))
            {
                $currentType = "Bigfoot" if (isBigfoot());
                $currentType = "Wookiee" if (isWookiee());
                $currentType = "750" if (is750());
                $currentType = "3100" if (is3100());
                $currentType = "4000" if (is4000());
                $currentType = "4700" if (is4700());
                $currentType = "7000" if (is7000());
                $currentType = "7400" if (is7400());
            }
            else
            {
                $currentMgr->{CONTROLLER_TYPE} = CTRL_TYPE_UNKNOWN;
                $currentType = "Unknown";
            }

            $connections{$currentConnection} = {"IP" => $ip,
                                                "MGR" => $obj,
                                                "PORT" => $port,
                                                "TYPE" => $currentType};

            SetColor($currentConnection);
            if ($noVerboseOutput == 0) {
                print "Login to $currentType controller at ($ip:$port) successful, " .
                        "connection ID: $currentConnection\n";
            }
            
            $currentMgr->storefwVersion($currentMgr);
        }
        else
        {
            print "Login to ($ip:$port) FAILED!.\n";
            undef $obj;
        }
    }
    else
    {
        print "Failed to create command manager object.\n";
    }
}

##############################################################################
# Name:     logInfoClassic
#
# Desc:     Displays the last N log messages.
#
# Input:    Number of log messages to display.
#           display mode
#           starting sequence number
#           -f filename  (output filename)
##############################################################################
sub logInfoClassic
{
    my ($count, $mode, $sequence) = @args;
    my $gmt = 0;
    my $showAckStatus = 0;

    print "\n";

    if ($opt_A)
    {
        $showAckStatus = 1;
    }

    if ($opt_G)
    {
        print "got here\n";
        $gmt = 1;
    }

    if (!defined($mode) )
    {
        $mode = 0x10;
    }
    if ($mode =~ /^0x/i)
    {
        $mode = oct $mode;
    }
    if (!defined($count) || $count < 1)
    {
        $count = 10;
    }
    if (!defined($sequence) )
    {
        $sequence = 0;
    }
    if ($mode | 0x04)
    {
        $sequence = (($count + $sequence) - 1);
    }

    # see if an output goes to a file
    my $outfile;
    if ($opt_f) {
#        print "opt_f=$opt_f\n";
        $outfile = $opt_f;
    }

    my %rsp = $currentMgr->logInfo($count, $mode, $sequence);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "NOTE: Use the new DSPLOGS command if you can't figure out the ".
            "'mode' parameter on LOGINFO :-)\n\n";
            $currentMgr->displayLogInfo($outfile, $mode, $gmt, $showAckStatus, %rsp);
        }
        else
        {
            my $msg = "Unable to retrieve log information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     SetControllerType
#
# Desc:     Set the controller type for the current connection.
#
# Input:    type
##############################################################################
sub SetControllerType
{
    my ($type) = @args;

    if (!defined($type) or $type !~ /^BIGFOOT$|^WOOKIEE$|^750$|^3100$|^4000$|^7000$|^4700$|^7400$/i)
    {
        print "'type' must be BIGFOOT or WOOKIEE or 750\n";
        return;
    }

    if ($type =~ /BIGFOOT/i)
    {
        $currentMgr->{CONTROLLER_TYPE} = CTRL_TYPE_BIGFOOT;
        $connections{$currentConnection}{"TYPE"} = $currentType = "Bigfoot";
    }
    if ($type =~ /WOOKIEE/i)
    {
        $currentMgr->{CONTROLLER_TYPE} = CTRL_TYPE_WOOKIEE;
        $connections{$currentConnection}{"TYPE"} = $currentType = "Wookiee";
    }
    if ($type =~ /3100/i)
    {
        $currentMgr->{CONTROLLER_TYPE} = CTRL_TYPE_3100;
        $connections{$currentConnection}{"TYPE"} = $currentType = "3100";
    }
    if ($type =~ /4000/i)
    {
        $currentMgr->{CONTROLLER_TYPE} = CTRL_TYPE_4000;
        $connections{$currentConnection}{"TYPE"} = $currentType = "4000";
    }
    if ($type =~ /4700/i)
    {
        $currentMgr->{CONTROLLER_TYPE} = CTRL_TYPE_4700;
        $connections{$currentConnection}{"TYPE"} = $currentType = "4700";
    }
    if ($type =~ /7000/i)
    {
        $currentMgr->{CONTROLLER_TYPE} = CTRL_TYPE_7000;
        $connections{$currentConnection}{"TYPE"} = $currentType = "7000";
    }
    if ($type =~ /7400/i)
    {
        $currentMgr->{CONTROLLER_TYPE} = CTRL_TYPE_7400;
        $connections{$currentConnection}{"TYPE"} = $currentType = "7400";
    }
    if ($type =~ /750/i)
    {
        $currentMgr->{CONTROLLER_TYPE} = CTRL_TYPE_750;
        $connections{$currentConnection}{"TYPE"} = $currentType = "750";
    }

    print "\nOK\n";
}

##############################################################################
# Name:     logInfo
#
# Desc:     Displays the last N log messages.
#
# DSPLOGS [-A] [-C] [-D] [-B|-E] [-G] [-f filename] [[count] sequenceNumber]
##############################################################################
sub logInfo
{
    my ($count, $sequence) = @args;
    my $mode = 0x10;
    my $gmt = 0;
    my $showAckStatus = 0;

    print "\n";

    if ($opt_A)
    {
        $showAckStatus = 1;
    }

    if ($opt_C and $opt_D)
    {
        $mode |= 0x10;  # already the default
    }
    elsif ($opt_D)
    {
        $mode &= ~0x10;
        $mode |= 0x8;
    }
    elsif ($opt_C)
    {
        $mode &= ~0x10;
    }

    if ($opt_B and $opt_E)
    {
        print "Only -B or -E can be specified, not both at one time\n";
        return;
    }
    elsif ($opt_B)
    {
        $mode |= 0x1;
    }
    elsif ($opt_E)
    {
        $mode |= 0x2;
    }

    if ($opt_G)
    {
        $gmt = 1;
    }

    if (!defined($count) || ((uc($count) ne "ALL") && ($count < 1)))
    {
        $count = 10;
    }

    if (!defined($sequence) )
    {
        $sequence = 0;
    }
    else
    {
        $mode |= 0x4;
        $sequence = (($count + $sequence) - 1);
    }

    # see if an output goes to a file
    my $outfile;
    if ($opt_f) {
        $outfile = $opt_f;
    }

    my %rsp = $currentMgr->logInfo($count, $mode, $sequence);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            # strip quotes if present
            $opt_g =~ s/^"(.*)"$/$1/ if defined ($opt_g);
            $currentMgr->displayLogInfo2($outfile, $mode, $gmt, $showAckStatus,
                \%rsp, $opt_g);
        }
        else
        {
            my $msg = "Unable to retrieve log information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     logTextMessage
#
# Desc:     Send a text log message.
#
# Input:    msg     -   Message to send
##############################################################################
sub logTextMessage
{
    my $msg;
    my $type = LOG_TYPE_DEBUG;

    if (defined($opt_v) &&
        (($opt_v == LOG_TYPE_INFO) ||
        ($opt_v == LOG_TYPE_WARNING) ||
        ($opt_v == LOG_TYPE_ERROR)))
    {
        $type = $opt_v;
    }

    while (scalar(@args) > 0)
    {
        $msg .= shift @args;
        $msg .= " ";
    }

    print "\n";

    if (!defined($msg) )
    {
        print "No message defined.\n";
        return;
    }

    my %rsp = $currentMgr->logTextMessage($msg, $type);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Log message sent successfully.\n";
        }
        else
        {
            my $msg = "Unable to send log message.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     logAcknowledge
#
# Desc:     Acknowledge logs.
#
# Input:    log sequence numbers
##############################################################################
sub logAcknowledge
{
    my ($seqNum) = @args;

#    my $msg;
#    my $type = LOG_TYPE_DEBUG;
#
#    if (defined($opt_v) &&
#        (($opt_v == LOG_TYPE_INFO) ||
#        ($opt_v == LOG_TYPE_WARNING) ||
#        ($opt_v == LOG_TYPE_ERROR)))
#    {
#        $type = $opt_v;
#    }
#
#    while (scalar(@args) > 0)
#    {
#        $msg .= shift @args;
#        $msg .= " ";
#    }
#
#    print "\n";
#
#    if (!defined($msg) )
#    {
#        print "No message defined.\n";
#        return;
#    }
#

    if (!defined($seqNum))
    {
        print "Missing sequence number.\n";
        return;
    }
    print "\n";
    my %rsp = $currentMgr->logAcknowledge($seqNum);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Log acknowledged.\n";
        }
        else
        {
            my $msg = "Unable to acknowledge log.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     logout
#
# Desc:     Close active connection
#
# Input:    None
##############################################################################
sub logout
{
    my $cmdMgr = $connections{$currentConnection}{"MGR"};
    my $ip = $connections{$currentConnection}{"IP"};

    if (!defined($cmdMgr) || !defined($ip))
    {
        print "\nNo connection.\n";
        return;
    }

    print "\n";
    print "Logging out of ($ip).\n";

    $cmdMgr->logout();
    delete $connections{$currentConnection};
    $currentConnection = -1;
    $currentIP = "";
    undef $currentMgr;

    print "Connection to ($ip) closed.\n";

    my $i;
    for ($i = 0; $i < $numNextConnection; ++$i)
    {
        if (defined($connections{$i}) &&
            defined($connections{$i}{"IP"}) &&
            defined($connections{$i}{"MGR"}))
        {
            $currentConnection = $i;
            $currentIP = $connections{$currentConnection}{"IP"};
            $currentMgr = $connections{$currentConnection}{"MGR"};
            $currentPort = $connections{$currentConnection}{"PORT"};
            $currentType = $connections{$currentConnection}{"TYPE"};

            last;
        }
    }

    print "\n";

    SetColor($currentConnection);
}

##############################################################################
# Name:     CalcCRC()
#
# Desc:     Calculates a CRC32 over the input data
#
# Input:    Packed, binary data
#
# Returns:  CRC32
#
##############################################################################
# CRC table used by 'crc32'
my @CRCtable = (
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
);

sub CalcCRC($)
{
    my ($data) = @_;
    my $crc = 0xFFFFFFFF;
    my $len = length($data);
    my $template = "C" . $len;
    my $p;
    my @pp=unpack $template, $data;

    for (my $i=0; $i<$len; $i++) {
        $p=$pp[$i];
        $crc = (($crc>>8) & 0x00FFFFFF) ^ $CRCtable[($crc ^ $p) & 0xFF];
    }

    return ($crc ^ 0xFFFFFFFF);
}

##############################################################################
# Name:     memRead()
#
# Desc:     Read memory from specified processor
#
# Input:    Address
#           length
#           -p CCB|BE|FE  (processor)
#           -t byte|short|word|binary  (output type)
#           -f filename  (output filename)
#           -m filename  (mapfile to use)
#
##############################################################################
my %symHash;
sub memRead($)
{
    my $cmd = @_;

    # see if alternate processor chosen
    my $proc = "CCB";

    print "\n";

    if ($opt_p) {
#        print "opt_p=$opt_p\n";
        if ($opt_p =~ /^CCB$|^BE$|^FE$/i) {
            $proc = $opt_p;
        }
        else {
            print "-p must specify 'CCB', 'FE' or 'BE'\n";
            return;
        }
    }

    # see if alternate format chosen
    my $format = "word";
    if ($opt_t) {
#        print "opt_t=$opt_t\n";
        if ($opt_t =~ /^byte$|^short$|^word$|^binary$/i) {
            $format = $opt_t;
        }
        else {
            print "-t must specify 'byte', 'short', 'word' or 'binary'\n";
            return;
        }
    }

    # see if an output goes to a file
    my $outfile;
    if ($opt_f) {
#        print "opt_f=$opt_f\n";
        $outfile = $opt_f;
    }

    # get the associated mapfile
    my $localMapFile = undef;
    if ($opt_m) {
        $localMapFile = $opt_m;
    }

    # retrieve the command line args
    my ($address, $length) = @args;

    if (!defined($address)) {
        print "Missing address parameter\n";
        return;
    }

    # check for symbol
    if ($address =~ /^[a-zA-Z]/) {
        my $name = uc $address;
        if (ReadSymbols($proc, $localMapFile)) {
            return;
        }
        if (defined($symHash{"$name"}{ADDR})) {
            $address = $symHash{$name}{ADDR};
            if (!defined($length)) {
                $length = $symHash{$name}{LEN};
            }
            printf "Found symbol: %s 0x%X, 0x%X/%u bytes\n\n",
            $symHash{$name}{ACTUAL}, $symHash{$name}{ADDR},
            $symHash{$name}{LEN}, $symHash{$name}{LEN};
        }
        else {
            print "Couldn't find symbol: \"$address\"\n";
            return;
        }
    }

    if (!defined($length)) {
        print "Missing length parameter\n";
            return;
        }

    # Convert from hex if necessary
    elsif ($address =~ /^0x/i) {
        $address = oct $address;
    }

    if ($length =~ /^0x/i) {
        $length = oct $length;
    }

    # determime if force is required
    if ($opt_F) {
        $length = $length | 0x80000000;
    }


    # Read the data
    my %data;
    if ($cmd =~ /^MPXMEMREAD$/) {
        %data = $currentMgr->MPXReadMemory($address, $length, $proc);
    }
    else {
        %data = $currentMgr->ReadMemory($address, $length, $proc);
    }

    if ($data{STATUS} != 0) {
        print "\n";
        print "The memory read failed...\n";
    }
    else {
        # Calculate CRC if asked to do so
        if ($opt_C)
        {
            my $crc = CalcCRC($data{RD_DATA});
            printf "\nCRC32 = 0x%08X\n", $crc;
        }

        if (! $opt_C || defined($outfile) )
        {
            # After data read up, format it appropriately.
            $currentMgr->FormatData($data{RD_DATA}, $address, $format, $outfile, undef);
        }
    }

    print "\n";
}

##############################################################################
# Name:     ReadSymbols()
#
# Desc:     Read map file and build hash table of symbols/addresses/lengths
#
# Input:    proc
#           filename
#
##############################################################################
my $symFileTS = 0;
sub ReadSymbols($$)
{
    my ($proc, $lmap) = @_;
    my $print;

    if (!defined ($lmap)) {
        $lmap = $ccbMapFile; # dft in case no other matches (shouldn't happen)
        if ($proc =~ /^CCB$/i) {
            $lmap = $ccbMapFile;
        }
        if ($proc =~ /^BE$/i) {
            $lmap = $beMapFile;
        }
        if ($proc =~ /^FE$/i) {
            $lmap = $feMapFile;
        }
    }

    if (! -r $lmap) {
        print "Can't open $lmap...\n";
        return 1;
    }
    my @stat = stat $lmap;

    if ($symFileTS == 0 or $symFileTS != $stat[9]) {
        print "Reading symbols from $lmap...\n";

        open FIN, $lmap or die "\nAbort: Can't open $lmap...\n";

        @stat = stat FIN;
        $symFileTS = $stat[9];

        if (isBigfoot())
        {
            #
            # Search to the link editor section
            #
            my $fin = "CmdCodeHashes.tpl";
            while (<FIN>) {
                if (/LINK EDITOR MEMORY MAP/) {
                    $print = 1;
                    last;
                }
            }

            if (!$print) {
                print "Can't find \"LINK EDITOR MEMORY MAP\" in $fin...\n";
                return 1;
            }

            #
            # create symbol hash
            #
            %symHash = ();
            my $lastAddress = 0;
            my $length = 0;
            my $lastLength = 0;
            my $lastName = "";
            while (<FIN>) {
                my @line = split;
                if (scalar @line == 2) {
                    (my $name = $line[1]) =~ s/^_//;

                    if (($name =~ /^\.text/) or
                            ($name =~ /^\.data/) or
                            ($name =~ /^\.bss/) or
                            ($name =~ /ic_name_rules/) or
                            ($name =~ /gcc2_compiled/) or
                            ($name =~ /gnu_compiled/)
                      ) {
                        next;
                    }

                    my $address = oct $line[0];
                    if ($address != $lastAddress) {
                        $lastLength = $length;
                        $length = $address-$lastAddress;
                    }

                    $symHash{uc $lastName}{ADDR} = $lastAddress;
                    $symHash{uc $lastName}{LEN} = $length;
                    $symHash{uc $lastName}{ACTUAL} = $lastName;

                    $lastAddress = $address;
                    $lastName = $name;
                }
            }

            close FIN;
        }
        elsif (!isBigfoot())
        {
            #
            # read and sort the file
            #
            my @sorted;
            while (<FIN>)
            {
                push @sorted, $_;
            }
            close FIN;
            @sorted = sort @sorted;

            #
            # create symbol hash
            #
            %symHash = ();
            my $lastAddress = 0;
            my $length = 0;
            my $lastLength = 0;
            my $lastName = "";
            while (@sorted) {
                $_ = shift @sorted;
                next if /^\s$/;
                chomp;
                my @line = split;
                if (@line == 2 and $line[0] =~ /^0x/ and
                    ($line[1] !~ /^obj\//) and
                    ($line[1] !~ /^\/usr\//) and
                    ($line[1] !~ /^0x/))
                {
                    my $name = $line[1];

                    my $address = oct $line[0];
                    if ($address != $lastAddress) {
                        $lastLength = $length;
                        $length = $address-$lastAddress;
                    }

                    $symHash{uc $lastName}{ADDR} = $lastAddress;
                    $symHash{uc $lastName}{LEN} = $length;
                    $symHash{uc $lastName}{ACTUAL} = $lastName;

                    $lastAddress = $address;
                    $lastName = $name;
                }
            }

        }
        else
        {
            return 1;
        }
    }
    return 0;
}


##############################################################################
# Name:     memWrite()
#
# Desc:     Write memory in specified processor
#
# Input:    Address
#           data
#           -p CCB|BE|FE  (processor)
#           -t byte|short|word  (input type)
#           -f filename (binary data) (exclusive with 'data' & 'type')
#
##############################################################################
sub memWrite($)
{
    my $cmd = @_;

    # see if alternate processor chosen
    my $proc = "CCB";

    print "\n";

    if ($opt_p) {
#        print "opt_p=$opt_p\n";
        if ($opt_p =~ /^CCB$|^BE$|^FE$/i) {
            $proc = $opt_p;
        }
        else {
            print "-p must specify 'CCB', 'FE' or 'BE'\n";
            return;
        }
    }

    # See if file specified
    my $fiddata;
    if ($opt_f)
    {
        # Read up the file data
        if (!open(F, "$opt_f"))
        {
            print "Specified file not readable\n";
            return;
        }

        if ($opt_t)
        {
            print "'type' not supported when an input file is specified.\n";
            return;
        }

        binmode F;
        read F, $fiddata, -s $opt_f;
        close F;

        if (length($fiddata) == 0)
        {
            print "Specified file is empty\n";
            return;
        }
    }

    # see if alternate format chosen
    my $format = "word";
    if ($opt_t) {
#        print "opt_t=$opt_t\n";

        if ($proc !~ /^CCB$/)
        {
            print "-t only allowed for writes to the CCB.\n";
            return;
        }

        if ($opt_t =~ /^byte$|^short$|^word$/i) {
            $format = $opt_t;
        }
        else {
            print "-t must specify 'byte', 'short', 'word'\n";
            return;
        }
    }

    # retrieve the command line args
    my ($address, $data) = @args;

    if (!defined($address)) {
        print "Missing address parameter\n";
        return;
    }

    if (!defined($data) and !defined($fiddata)) {
        print "Missing data or input file parameter\n";
        return;
    }

    if (defined($data) and defined($fiddata)) {
        print "Can't specify 'data' and input file at the same time\n";
        return;
    }

    # convert from hex if necessary
    if ($address =~ /^0x/i) {
        $address = oct $address;
    }

    if (defined($data))
    {
        $data = AsciiHexToBin($data, $format);;
        if (!defined $data) {
            print "The data was NOT written.\n";
            return;
        }
    }
    else
    {
        $data = $fiddata;
    }

    if ($proc !~ /^CCB$/ and (length($data) % 4))
    {
        print "When writing a processor other than the CCB,\n";
        print "the data length must be a multiple of 4.\n";
        return;
    }

    # do the write
    my %rsp;
    if ($cmd =~ /^MPXMEMWRITE$/) {
        %rsp = $currentMgr->MPXWriteMemory($address, $data, $proc);
    }
    else
    {
        %rsp = $currentMgr->WriteMemory($address, $data, $proc);
    }

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "The data was successfully written.\n";
        }
        else
        {
            my $msg = "The memory write failed.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pDataChecksum()
#
# Desc:     reset persistent data
#
# Input:    offset
#           length
#
##############################################################################
sub pDataChecksum
{
    # retrieve the command line args
    my ($offset, $length) = @args;

    if (!defined($offset) or !defined($length)) {
        print "Missing offset or length parameter(s)\n";
        return;
    }

    # Convert from hex if necessary
    if ($offset =~ /^0x/i) {
        $offset = oct $offset;
    }
    if ($length =~ /^0x/i) {
        $length = oct $length;
    }

    # Read the data
    my %data = $currentMgr->persistentDataControl(
                                $currentMgr->PERSISTENT_DATA_OPTION_CHECKSUM,
                                $offset,
                                $length,
                                0);
    if ($data{STATUS} != 0) {
        displayError("The persistent data reset failed!", %data);
        return;
    }
    else
    {
        printf "Persistent checksum for 0x%08X to 0x%08X is %d\n",
                $offset, ($offset + $length), $data{CHECK_SUM};
    }

    print "\n";
}

##############################################################################
# Name:     pDataReset()
#
# Desc:     reset persistent data
#
# Input:    offset
#           length
#
##############################################################################
sub pDataReset
{
    # retrieve the command line args
    my ($offset, $length) = @args;

    if (!defined($offset) or !defined($length)) {
        print "Missing offset or length parameter(s)\n";
        return;
    }

    # Convert from hex if necessary
    if ($offset =~ /^0x/i) {
        $offset = oct $offset;
    }
    if ($length =~ /^0x/i) {
        $length = oct $length;
    }

    # Read the data
    my %data = $currentMgr->persistentDataControl(
                                $currentMgr->PERSISTENT_DATA_OPTION_RESET,
                                $offset,
                                $length,
                                0);
    if ($data{STATUS} != 0) {
        displayError("The persistent data reset failed!", %data);
        return;
    }
    else
    {
        printf "Persistent data reset successful from 0x%08X to 0x%08X\n",
                $offset, ($offset + $length);
    }

    print "\n";
}

##############################################################################
# Name:     pDataRead()
#
# Desc:     Read persistent data
#
# Input:    offset
#           length
#           -t byte|short|word|binary  (output type)
#           -f filename  (output filename)
#
##############################################################################
sub pDataRead
{
# see if alternate format chosen
    my $format = "word";
    if ($opt_t) {
#        print "opt_t=$opt_t\n";
        if ($opt_t =~ /^byte$|^short$|^word$|^binary$/i) {
            $format = $opt_t;
        }
        else {
            print "-t must specify 'byte', 'short', 'word' or 'binary'\n";
            return;
        }
    }

    # see if an output goes to a file
    my $outfile;
    if ($opt_f) {
#        print "opt_f=$opt_f\n";
        $outfile = $opt_f;
    }

    # retrieve the command line args
    my ($offset, $length) = @args;

    if (!defined($offset) or !defined($length)) {
        print "Missing offset or length parameter(s)\n";
        return;
    }

    # Convert from hex if necessary
    if ($offset =~ /^0x/i) {
        $offset = oct $offset;
    }
    if ($length =~ /^0x/i) {
        $length = oct $length;
    }

    # Read the data
    my %data = $currentMgr->persistentDataControl(
                                $currentMgr->PERSISTENT_DATA_OPTION_READ,
                                $offset,
                                $length,
                                0);
    if ($data{STATUS} != 0) {
        displayError("The persistent data read failed!", %data);
        return;
    }

    # After data read up, format it appropriately.
    $currentMgr->FormatData($data{RD_DATA}, $offset, $format, $outfile, undef);

    print "\n";
}

##############################################################################
# Name:     pDataWrite()
#
# Desc:     Write persistent data
#
# Input:    offset
#           data
#           -t byte|short|word  (input type)
#
##############################################################################
sub pDataWrite
{
    # see if alternate format chosen
    my $format = "word";
    if ($opt_t) {
#        print "opt_t=$opt_t\n";
        if ($opt_t =~ /^byte$|^short$|^word$/i) {
            $format = $opt_t;
        }
        else {
            print "-t must specify 'byte', 'short', 'word' or 'binary'\n";
            return;
        }
    }

    # retrieve the command line args
    my ($offset, $data) = @args;

    if (!defined($offset) or !defined($data)) {
        print "Missing offset or data parameter(s)\n";
        return;
    }

    # convert from hex if necessary
    if ($offset =~ /^0x/i) {
        $offset = oct $offset;
    }

    $data = AsciiHexToBin($data, $format);;
    if (!defined $data) {
        print "The data was NOT written.\n";
        return;
    }

    # do the write
    my %rsp = $currentMgr->persistentDataControl(
                                $currentMgr->PERSISTENT_DATA_OPTION_WRITE,
                                $offset,
                                length $data,
                                $data);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "$rsp{LENGTH} bytes successfully written, " .
                  "checksum: $rsp{CHECK_SUM}.\n";
        }
        else
        {
            my $msg = "The persistent data write failed.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pEwokDataCtrl()
#
# Desc:     control EWOK persistent data
#
# Input:    offset
#           length
#
##############################################################################
sub pEwokDataCtrl
{
    # retrieve the command line args
    my ($opt, $recordname, $offset, $length, $writedata, $totrunc) = @args;

    if (!defined($opt))
    {
        print "Missing parameter(s)\n";
        return;
    }

    my $recdata="";
    my $recname="";
    my $startByte=0;
    my $len=0;
    my $trunc=0;

    my $delall=0;

    if (defined($recordname))
    {
        $recname = $recordname;
    }

    if ($opt == 99)
    {
        $delall = 1;
        $opt = 5;
    }

    if ($opt == 1 || $opt == 2)  # Create/Remove
    {
        if (!defined($recordname))
        {
            print "Missing parameter(s)\n";
            return;
        }
    }
    if ($opt == 3)      # Read
    {
        if (!defined($offset) || !defined($length))
        {
            print "Missing parameter(s)\n";
            return;
        }
        else
        {
            $startByte = $offset;
            $len = $length;
        }
    }
    if ($opt == 4)    # Write
    {
        if (!defined($offset) || !defined($length) || !defined($writedata))
        {
            print "Missing parameter(s)\n";
            return;
        }
        else
        {
            $startByte = $offset;
            $len = $length;
            $recdata = $writedata;
            if (defined($totrunc))
            {
                $trunc = $totrunc;
            }
        }
    }

    my %data = $currentMgr->ewokDataControl(
                                $opt,
                                $recname,
                                $startByte,
                                $len,
                                $trunc,
                                $recdata);

    if ($data{STATUS} != 0) {
        displayError("The EWOK persistent data reset failed!", %data);
        return;
    }
    else
    {
        if ($delall == 0)
        {
            $currentMgr->displayewokDataControlResponsePacket(%data);
        }
        else
        {
            printf "Deleting all records, %u total\n", $data{REC_COUNT};

            for (my $index = 0; $index < $data{REC_COUNT}; ++$index)
            {
                printf "  Deleting Record %s\n", $data{$index}{RC_NAME};
                $currentMgr->ewokDataControl(2,$data{$index}{RC_NAME},0,0,0,0);
            }
            printf "Deleted all records, %u total\n", $data{REC_COUNT};
        }
    }

    print "\n";
}

##############################################################################
# Name:     registerEvents()
#
# Desc:     register async events
#
# Input:    opt
#           register
#
##############################################################################
sub registerEvents
{
    # retrieve the command line args
    my ($opt, $register1, $register2) = @args;

    if (!defined($opt))
    {
        print "Missing parameter(s)\n";
        return;
    }
    my $registerEvents1;
    my $registerEvents2;
    if ($opt == 0)
    {
        if (!defined($register1))
        {
            print "Missing parameter(s)\n";
            return;
        }
        if (!defined($register2))
        {
            print "Missing parameter(s)\n";
            return;
        }
        $registerEvents1 = $register1;
        $registerEvents2 = $register2;

    }
    else
    {
        $registerEvents1 = 0;
        $registerEvents2 = 0;
    }

    my $rc = $currentMgr->registerAsyncEvents(
                                $opt,
                                $registerEvents1,
                                $registerEvents2);

#    if ($data{STATUS} != 0) {
#        displayError("Register Events failed!", %data);
#        return;
#    }
#    else
#    {
#        printf "Register Events successful\n";
#        if ($data{REGISTERED} == 1)
#        {
#            printf "Registration ON\n";
#        }
#        else
#        {
#            printf "Registration OFF\n";
#        }
#    }
#
#    print "\n";
}

##############################################################################
# Name:     fidRead()
#
# Desc:     Read the specified File ID
#
# Input:    fid
#           -t byte|short|word|fmt|binary  (output type)
#           -f filename  (output filename)
#           -l length  Number of bytes to display (default: entire FID)
#
##############################################################################
sub fidRead($)
{
    my $cmd = @_;

    print "\n";

    # see if alternate format chosen
    my $format = "fmt";
    if ($opt_t) {
#        print "opt_t=$opt_t\n";
        if ($opt_t =~ /^byte$|^short$|^word$|^fmt$|^binary$/i) {
            $format = $opt_t;
        }
        else {
            print "-t must specify 'byte', 'short', 'word', 'binary' or 'fmt'\n";
            return;
        }
    }

    # see if an output goes to a file
    my $outfile;
    if ($opt_f) {
#        print "opt_f=$opt_f\n";
        $outfile = $opt_f;
    }

    # see if a length was entered
    my $length = undef;
    if ($opt_l) {
#        print "opt_l=$opt_l\n";
        $length = $opt_l;
    }

    # retrieve the command line args
    my ($fid) = @args;

    if (!defined($fid))
    {
        print "Missing FID\n";
        return;
    }

    # Convert from hex if necessary
    if ($fid =~ /^0x/i) {
        $fid = oct $fid;
    }

    # Linux FileRead rsvd fids
    if ( ($fid >= 0x100) && ($fid < 0x200) &&
         (($fid & 0xFF) >= 0x31) &&
         (($fid & 0xFF) <= 0x60) )
    {
        if (isBigfoot())
        {
            print "These FIDS are only available for Wookiee\n";
            return;
        }
        $format = "fmt";
        $cmd = "";
    }

    # Read the data
    my %data = $currentMgr->MPXReadFID($fid);

    if (%data)
    {
        if ($data{STATUS} != 0)
        {
            print "\n";
            print "The FID read failed.  status = $data{STATUS}  $data{STATUS_MSG}\n";
            print "                      errorCode = $data{ERROR_CODE}, $data{PI_ERROR_MSG};\n",
        }
        else
        {
            if ($opt_S and $format !~ /^binary$/i)
            {
                print "Note: '-S' ignored (type must be 'binary')\n\n";
            }

            # After data read up, format it appropriately.
            if ($format ne "fmt")
            {
                my $data;

                # formats other than 'formatted'
                if ($opt_S and $format =~ /^binary$/i)
                {
                    $currentMgr->FormatData(substr($data{RD_DATA}, 32), 0, $format, $outfile, $length);
                }
                else
                {
                    $currentMgr->FormatData($data{RD_DATA}, 0, $format, $outfile, $length);
                }
            }
            else
            {
                # $format is 'fmt' for formatted output
                my $clen = length($data{RD_DATA});

                if ($clen > 0 )
                {
                    if (defined($opt_b) && $opt_b =~ /^0x/i)
                    {
                        $opt_b = oct $opt_b;
                    }
                    if (CCBEDecodeFids($data{RD_DATA}, $fid , $outfile, undef, $opt_b) == GOOD)
                    {
                        # If Linux file read, uncompress if desired.
                        if ($opt_u && defined($outfile) &&
                            ($fid >= 0x100) && ($fid < 0x200) &&
                            (($fid & 0xFF) >= 0x31) &&
                            (($fid & 0xFF) <= 0x60) )
                        {
                            if (!(-d $opt_u))
                            {
                                mkdir $opt_u;
                            }
                            print "Uncompressing to directory $opt_u: ";
                            if ( (system ("gzip -df $outfile.tgz") == 0) &&
                                (system ("tar -xf $outfile.tar -C $opt_u") == 0) )
                            {
                                print "SUCCESS\n";
                                unlink "$outfile.tgz"     if (-e "$outfile.tgz");
                                unlink "$outfile.tar"     if (-e "$outfile.tar");
                            }
                            else
                            {
                                print "FAILED - Saving as $outfile.tgz\n" if (-e "$outfile.tgz");
                                print "FAILED - Saving as $outfile.tar\n" if (-e "$outfile.tar");
                            }
                        }
                        elsif (defined($outfile))
                        {
                            print "File Saved as $outfile\n";
                        }
                    }
                    else
                    {
                        print "Error decoding FID $fid\n";
                    }
                }
                else
                {
                    print "Fid length was 0. No data to format.\n\n";
                }
            }
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
    print "\n";
}

##############################################################################
# Name:     linuxFileRead()
#
# Desc:     Read the specified Linux File
#
# Input:    filename    - Filename to read
#           -f          - File name to write to.
#
##############################################################################
sub linuxFileRead
{

    print "\n";

    # retrieve the command line args
    my ($linFile) = @args;
    my $fid = 305;
    my $outFile = "";
    my %data;
    my $fh;

    if (!defined($linFile))
    {
        print "Missing Filename\n";
        return;
    }

    if ($opt_f)
    {
        $outFile = $opt_f;
    }
    else
    {
        if ($linFile =~ /\/([A-Za-z0-9\._\-]+)$/)
        {
            $outFile = $1;
        }
        else
        {
            $outFile = "temp";
        }
    }

    print "Copying remote $linFile to local $outFile\n";

    chomp $linFile;

    %data = $currentMgr->MPXWriteFID($fid, $linFile);

    if (%data)
    {
        if ($data{STATUS} != 0)
        {
            print "\n";
            print "The FID write failed.  status = $data{STATUS}  $data{STATUS_MSG}\n";
            print "                       errorCode = $data{ERROR_CODE}, $data{PI_ERROR_MSG};\n",
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    # Read the data
    %data = $currentMgr->MPXReadFID($fid);


    if (%data)
    {
        if ($data{STATUS} != 0)
        {
            print "\n";
            print "The FID read failed.  status = $data{STATUS}  $data{STATUS_MSG}\n";
            print "                      errorCode = $data{ERROR_CODE}, $data{PI_ERROR_MSG};\n",

        }
        else
        {
            my $clen = length($data{RD_DATA});

            if ($clen > 0 )
            {
                CCBEDecodeFids ($data{RD_DATA}, $fid , "$outFile.bz2");
                print "File Saved as $outFile.bz2\n";
            }
            else
            {
                print "Fid length was 0. No data to format.\n\n";
            }
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }


    print "\n";
}
##############################################################################
# Name:     fidDecode()
#
# Desc:     Decode the specified file, based upon its FID
#
# Input:    input file
#           -f file : optional output file
#           -i FID  : optional FID id
#
##############################################################################
sub fidDecode
{
    # see if an output goes to a file
    my $outfile = undef;
    if ($opt_f) {
        $outfile = $opt_f;
    }

    # see if an output goes to a file
    my $fid = undef;
    if ($opt_i) {
        $fid = $opt_i;
    }

    # retrieve the command line args
    my ($infile) = @args;

    if (!defined($infile))
    {
        print "Missing file\n";
        return;
    }

    # Parse the filename for the FID id if none specified The filename must be
    # of this format:  xxxxxxFID_Nnxxxx, Where the 'x's are any charcter, the
    # "FID_Nn" is "FID_" followed by any number of digits, followed by
    # something other than a digit (or end of string).
    if (!defined($fid))
    {
        my $tmp = $infile;
        if ($infile =~ /FID_\d/i)
        {
            $tmp =~ s/^.*FID_//i;
            $tmp =~ m/\d+/;
            if (defined($&))
            {
                $fid = $&;
            }
            print "Extracted FID \"$fid\" from the input filename.\n";
        }

        # If no fid found, can't go on
        if (!defined($fid))
        {
            print "Can't figure out the FID from the filename, please specify a FID id\n";
            return;
        }
    }

    # Read up the file data
    if (!open(F, "$infile"))
    {
        print "File unreadable.\n";
        return;
    }

    # read up the fid
    my $fiddata;
    binmode F;
    read F, $fiddata, -s $infile;
    close F;

    my $clen = length($fiddata);
    if ($clen > 0 )
    {
       CCBEDecodeFids ($fiddata, $fid, $outfile);
       print "\n";
    }
    else
    {
        print "Fid length was 0. No data to format.\n\n";
    }
}

##############################################################################
# Name:     fidWrite()
#
# Desc:     Write the specified File ID
#
# Input:    fid
#           filename  (binary input data)
#
##############################################################################
sub fidWrite
{
    print "\n";

    # retrieve the command line args
    my ($fid, $file) = @args;
    my $fiddata;
    my $noHdrFlag = 0;

    if (!defined($fid))
    {
        print "Missing FID.\n";
        return;
    }

    if (!defined($file))
    {
        print "Missing file parameter.\n";
        return;
    }

    if ($opt_N)
    {
        $noHdrFlag = 1;
    }

    if ($opt_T)
    {
        $fiddata = $file;
    }
    else
    {
        # Read up the file data
        if (!open(F, "$file"))
        {
            print "File unreadable.\n";
            return;
        }

        binmode F;
        read F, $fiddata, -s $file;
        close F;
    }

    if (($fid >= 305) && ($fid <= 306))
    {
       chomp $fiddata;
    }

    my %data = $currentMgr->MPXWriteFID($fid, $fiddata, $noHdrFlag);

    if (%data)
    {
        if ($data{STATUS} != 0)
        {
            print "\n";
            print "The FID write failed.  status = $data{STATUS}  $data{STATUS_MSG}\n";
            print "                       errorCode = $data{ERROR_CODE}, $data{PI_ERROR_MSG};\n",
        }
        else
        {
            print "FID $fid written, " . length($fiddata) . " bytes\n";
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     structureDisplayInfo
#
# Desc:     Displays a structure.
#
# Input:    Structure to display.
##############################################################################
sub structureDisplayInfo
{
    my ($cmd) = @args;

    print "\n";

    if (!defined($cmd) || $cmd < 0)
    {
        print "Invalid or missing stucture identifier.\n";
        return;
    }

    my %rsp = $currentMgr->structureInfo($cmd);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayStrucureInfo(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve structure.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     mmtest
#
# Desc:
#
# Input:
##############################################################################
sub mmtest
{
    my ($option, $offset) = @args;

    print "\n";

    if (!defined($offset))
    {
        $offset = 0;
    }

    my %rsp = $currentMgr->mmtest($option, $offset);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            printf "MM Test (0x%x, 0x%x) submitted.\n", $option, $offset;
        }
        else
        {
            my $msg = "Unable to submit MM Test";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     modeBitInfo
#
# Desc:     Displays mode bits.
#
# Input:    none.
##############################################################################
sub modeBitInfo
{
    print "\n";

    my %rsp = $currentMgr->modeDataGet();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayModeDataInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve mode bits.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     modeBitSet
#
# Desc:     Sets the mode bits.
#
# Input:    bits and masks.
##############################################################################
sub modeBitSet
{
    my ($bits, $mask) = @args;

    my %bitHash;

    if (!defined($bits))
    {
        print "Invalid or missing bits.\n";
        return;
    }
    if (!defined($mask))
    {
        print "Invalid or missing mask.\n";
        return;
    }

    if ($bits =~ /^0x/i) {
        $bits = oct $bits;
    }
    if ($mask =~ /^0x/i) {
        $mask = oct $mask;
    }

    if (!$opt_p) {
        $opt_p = "CCB";
    }

    if ($opt_p =~ /^CCB$/i) {
        if ($opt_D) { # DPrintf mode bits?
            $bitHash{CCB_BITS_DPRINTF}       =$bits;
            $bitHash{CCB_BITS_DPRINTF_MASK}  =$mask;
        }
        else {
            $bitHash{CCB_BITS1}         =$bits;
            $bitHash{CCB_BITS_MASK1}    =$mask;
        }
    }
    elsif ($opt_p =~ /^PROC$/i) {
        $bitHash{PROC_BITS1}        =$bits;
        $bitHash{PROC_BITS_MASK1}   =$mask;
    }
    else {
        print "-p must specify 'CCB' or 'PROC'\n";
        return;
    }


    print "\n";

    my %rsp = $currentMgr->modeDataSet(%bitHash);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Mode bits set successful.\n";
        }
        else
        {
            my $msg = "Unable to retrieve mode bits.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     modebits
#
# Desc:     Sets the mode bits.
#
# Input:    flags
#           on|off|only
##############################################################################
sub buildCCBModebitsFlagHash
{
    my @temp;
    my %flagHash;

    #              name           bit       action for bit=1
    push @temp, [("IPC_HEARTBEATS", 0x00000001, "disable")]; # IPC Heartbeat Disable
    push @temp, [("IPC_WATCHDOG",   0x00000002, "disable")]; # IPC H/B Watchdog Disable
    push @temp, [("LOC_HEARTBEATS", 0x00000004, "disable")]; # Local Heartbeat Disable
    push @temp, [("LOC_STATISTICS", 0x00000008, "disable")]; # Local Statistics Disable
    push @temp, [("DUMP_LCL_IMAGE", 0x00000010, "enable" )]; # Dump Local Image Enable
    push @temp, [("FAILURE_MGR",    0x00000020, "disable")]; # Failure Manager Disable
    push @temp, [("CTRL_SUICIDE",   0x00000040, "disable")]; # Controller Suicide Disable
    push @temp, [("FM_RESTART",     0x00000100, "disable")]; # FM Restart Disable
    push @temp, [("DIAG_PORTS",     0x00000200, "enable" )]; # Diagnostic Ports Enable

    if (!isBigfoot())
    {
        push @temp, [("INACTIVATE_POWER",     0x00001000, "enable" )]; # Power-off Wookiee Inactivation
    }

    while (@temp)
    {
        $flagHash{$temp[0][0]}   = $temp[0][1];
        $flagHash{$temp[0][1]}   = $temp[0][0];
        $flagHash{$temp[0][0]."_BIT_EQ_1_ACTION"} = $temp[0][2];
        shift @temp;
    }

    return %flagHash;
}

sub buildProcModebitsFlagHash
{
    my @temp;
    my %flagHash;

    # Disable heartbeat monitor
    push @temp, [("P_HB_WATCHDOG",        0x00000001, "disable")];

    # Disable errtrap handling by boot code
    push @temp, [("P_BOOT_ERR_HANDLING", 0x00000002, "disable")];

    # Controller shutting down. Ignore
    #   CCB heartbeat failure
    push @temp, [("P_CTRL_SHUTDOWN",     0x00000004, "enable")];

    while (@temp)
    {
        $flagHash{$temp[0][0]}   = $temp[0][1];
        $flagHash{$temp[0][1]}   = $temp[0][0];
        $flagHash{$temp[0][0]."_BIT_EQ_1_ACTION"} = $temp[0][2];
        shift @temp;
    }

    return %flagHash;
}

sub modebits
{
    if ($opt_I)
    {
        modeBitInfo();
        @args = ();
    }

    if ($opt_S)
    {
        modeBitSet();
        @args = ();
    }

    my ($flags, $action) = @args;
    my %bitHash;
    my %ccbflagDef = buildCCBModebitsFlagHash();
    my %procflagDef = buildProcModebitsFlagHash();
    my @ccbkeys;

    print "\n";

    if (!defined($flags))
    {
        my %rsp = $currentMgr->modeDataGet();

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                print "Flag:                State:    Bits Set:\n";
                print "----------------------------------------\n";
                printf("CCB mode bits:                   0x%04X\n", $rsp{CCB_MODE_BITS1});
                print "----------------------------------------\n";

                @ccbkeys = sort(keys(%ccbflagDef));

                while (@ccbkeys)
                {
                    my $theKey = shift @ccbkeys;
                    if ($theKey !~ /_BIT_EQ_1_ACTION/ and ($theKey !~ /^\d+/))
                    {
                        my $theAction = $theKey . "_BIT_EQ_1_ACTION";
                        printf "%-20s ", $theKey;
                        if ($rsp{CCB_MODE_BITS1} & $ccbflagDef{$theKey})
                        {
                            if ($ccbflagDef{$theAction} eq "enable")
                            {
                                printf "Enabled     0x%04x\n", $ccbflagDef{$theKey};
                            }
                            else
                            {
                                printf "Disabled    0x%04x\n", $ccbflagDef{$theKey};
                            }
                        }
                        else
                        {
                            if ($ccbflagDef{$theAction} eq "enable")
                            {
                                print "Disabled    0x0\n";
                            }
                            else
                            {
                                print "Enabled     0x0\n";
                            }
                        }
                    }
                }
                print "\n";

                print "----------------------------------------\n";
                printf("Proc mode bits:                  0x%04X\n", $rsp{PROC_MODE_BITS1});
                print "----------------------------------------\n";

                my @prockeys = sort(keys(%procflagDef));

                while (@prockeys)
                {
                    my $theKey = shift @prockeys;
                    if ($theKey !~ /_BIT_EQ_1_ACTION/ and ($theKey !~ /^\d+/))
                    {
                        my $theAction = $theKey . "_BIT_EQ_1_ACTION";
                        printf "%-20s ", $theKey;
                        if ($rsp{PROC_MODE_BITS1} & $procflagDef{$theKey})
                        {
                            if ($procflagDef{$theAction} eq "enable")
                            {
                                printf "Enabled     0x%04x\n", $procflagDef{$theKey};
                            }
                            else
                            {
                                printf "Disabled    0x%04x\n", $procflagDef{$theKey};
                            }
                        }
                        else
                        {
                            if ($procflagDef{$theAction} eq "enable")
                            {
                                print "Disabled    0x0\n";
                            }
                            else
                            {
                                print "Enabled     0x0\n";
                            }
                        }
                    }
                }
            }
            else
            {
                my $msg = "Unable to retrieve MODEBITS.";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }

        return;
    }

    if (!defined($action))
    {
        print "Action must be SET, CLR, ENABLE or DISABLE.\n";
        return;
    }
    else
    {
        if ($action !~ /^SET$|^CLR$|^ENABLE$|^DISABLE$/i)
        {
            print "Action must be SET, CLR, ENABLE or DISABLE.\n";
            return;
        }
    }

    my @flagArray = $currentMgr->rangeToList($flags);
    my $ccbbits = 0;
    my $ccbmask = 0;
    my $procbits = 0;
    my $procmask = 0;

    while (@flagArray)
    {
        my $theFlag = uc(shift @flagArray);

        if (!defined($ccbflagDef{$theFlag}) and !defined($procflagDef{$theFlag}))
        {
            print "Undefined flag: $theFlag.\n";
            return;
        }

        my $theAction = $theFlag . "_BIT_EQ_1_ACTION";

        if ($theFlag =~ /^P_/)
        {
            $procmask |= $procflagDef{$theFlag};
        }
        else
        {
            $ccbmask |= $ccbflagDef{$theFlag};
        }

        if ($action =~ /^SET$/i)
        {
            print "Setting $theFlag\n";

            if ($theFlag =~ /^P_/)
            {
                $procbits |= $procflagDef{$theFlag};
            }
            else
            {
                $ccbbits |= $ccbflagDef{$theFlag};
            }
        }
        elsif ($action =~ /^CLR$/i)
        {
            print "Clearing $theFlag\n";
        }
        elsif ($action =~ /^ENABLE$/i)
        {
            print "Enabling $theFlag\n";

            if ($theFlag =~ /^P_/)
            {
                if ($procflagDef{$theAction} eq "enable")
                {
                    $procbits |= $procflagDef{$theFlag};
                }
            }
            else
            {
                if ($ccbflagDef{$theAction} eq "enable")
                {
                    $ccbbits |= $ccbflagDef{$theFlag};
                }
            }
        }
        elsif ($action =~ /^DISABLE$/i)
        {
            print "Disabling $theFlag\n";

            if ($theFlag =~ /^P_/)
            {
                if ($procflagDef{$theAction} eq "disable")
                {
                    $procbits |= $procflagDef{$theFlag};
                }
            }
            else
            {
                if ($ccbflagDef{$theAction} eq "disable")
                {
                    $ccbbits |= $ccbflagDef{$theFlag};
                }
            }
        }
    }

    $bitHash{CCB_BITS1}        = $ccbbits;
    $bitHash{CCB_BITS_MASK1}   = $ccbmask;
    $bitHash{PROC_BITS1}       = $procbits;
    $bitHash{PROC_BITS_MASK1}  = $procmask;

    my %rsp = $currentMgr->modeDataSet(%bitHash);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to set MODE BITS.";
            displayError($msg, %rsp);
        }
        else
        {
            # Do a recursive call to print out current bits set
            @args = ();
            modebits();
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     cfgoption
#
# Desc:     Configure Options
#
# Input:    bits and masks.
##############################################################################
sub cfgoption
{
    my ($bits, $mask) = @args;

    my %bitHash;

    if (!defined($bits))
    {
        $bits = 0;
    }

    if (!defined($mask))
    {
        $mask = 0;
    }

    if ($bits =~ /^0x/i)
    {
        $bits = oct $bits;
    }

    if ($mask =~ /^0x/i)
    {
        $mask = oct $mask;
    }

    print "\n";

    my %rsp = $currentMgr->cfgoption($bits, $mask);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displaycfgoption(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve configuration options.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     dprintf
#
# Desc:     Sets the dprintf mode bits.
#
# Input:    flags
#           on|off|only
##############################################################################
sub buildDprintfFlagHash
{
    my @temp;
    my %flagHash;

    push @temp, [("DEFAULT",            0x00000001)];
    push @temp, [("CACHE_REF",          0x00000002)];
    push @temp, [("XSSA_DEBUG",         0x00000004)];
    push @temp, [("ELECTION",           0x00000008)];
    push @temp, [("IPC",                0x00000010)];
    push @temp, [("IPC_MSGS",           0x00000020)];
    push @temp, [("X1_COMMANDS",        0x00000040)];
    push @temp, [("X1_PROTOCOL",        0x00000080)];
    push @temp, [("I2C",                0x00000100)];
    push @temp, [("RM",                 0x00000200)];
    push @temp, [("SES",                0x00000400)];
    push @temp, [("ETHERNET",           0x00000800)];
    push @temp, [("MD5",                0x00001000)];
    push @temp, [("FCALMON",            0x00002000)];
    push @temp, [("SM_HB",              0x00004000)];
    push @temp, [("PI_COMMANDS",        0x00008000)];
    push @temp, [("PROC_PRINTF",        0x00010000)];
    push @temp, [("ELECTION_VERBOSE",   0x00020000)];
    push @temp, [("IPMI",               0x00040000)];
    push @temp, [("IPMI_VERBOSE",       0x00080000)];

    while (@temp)
    {
        $flagHash{$temp[0][0]} = $temp[0][1];
        $flagHash{$temp[0][1]} = $temp[0][0];
        shift @temp;
    }

    # These are multi-bit, so handle them seperately
    $flagHash{ALL}     =         0xFFFFFFFF;
    $flagHash{BASIC}   =         ($flagHash{DEFAULT} |
                                  $flagHash{ELECTION} |
                                  $flagHash{RM});
    return %flagHash;
}

sub dprintf
{
    my ($flags, $action) = @args;

    my %bitHash;
    my %flagDef = buildDprintfFlagHash();

    print "\n";

    if (!defined($flags))
    {
        my %rsp = $currentMgr->modeDataGet();

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                print "DPRINTF mode bits enabled:\n\n";

                my $count;
                my $bit;
                for ($count = 0; $count < 32; $count++)
                {
                    $bit = (1 << $count) & $rsp{CCB_MODE_DPRINTF_BITS};
                    if ($bit)
                    {
                        if (defined($flagDef{$bit}))
                        {
                            print "$flagDef{$bit}\n";
                        }
                        else
                        {
                            printf "UNDEFINED (0x%08x)\n", $bit;
                        }
                    }
                }
            }
            else
            {
                my $msg = "Unable to retrieve DPRINTF mode bits.";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }

        return;
    }

    if (!defined($action))
    {
        print "Action must be ON, OFF or ONLY.\n";
        return;
    }
    else
    {
        if ($action !~ /^ON$|^OFF$|^ONLY$/i)
        {
            print "Action must be ON, OFF or ONLY.\n";
            return;
        }
    }

    my @flagArray = $currentMgr->rangeToList($flags);
    my $bits;
    my $mask;

    if ($action =~ /^ONLY$/i)
    {
        $mask = $flagDef{ALL};
    }
    else
    {
        $mask = 0;
    }

    while (@flagArray)
    {
        my $theFlag = uc(shift @flagArray);

        if (!defined($flagDef{$theFlag}))
        {
            print "Undefined flag: $theFlag.\n";
            return;
        }

        $mask |= $flagDef{$theFlag};

        if ($action =~ /^ON$|^ONLY$/i)
        {
            print "Enabling $theFlag\n";
            $bits |= $flagDef{$theFlag};
        }
        else
        {
            print "Disabling $theFlag\n";
        }
    }
    if ($action =~ /^ONLY$/i)
    {
        print "Disabling others\n";
    }

    $bitHash{CCB_BITS_DPRINTF}       = $bits;
    $bitHash{CCB_BITS_DPRINTF_MASK}  = $mask;

    my %rsp = $currentMgr->modeDataSet(%bitHash);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to set DPRINTF mode bits.";
            displayError($msg, %rsp);
        }
        else
        {
            # Do a recursive call to print out current bits set
            @args = ();
            dprintf();
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pdisks
#
# Desc:     Displays physical disk information for all physical disks.
#
# Input:    None
##############################################################################
sub pdisks
{
    my ($dsptype) = @args;

    my %rsp;
    my %rspList;
    my %rspInfo;

    print "\n";

    if (!defined($dsptype))
    {
        $dsptype = "STD";
    }

    if (uc($dsptype) eq "LOOP")
    {
        $rsp{STATUS} = PI_GOOD;
        $rsp{COUNT} = 0;
        my $debug = 0;

        ##
        # Make the status line and grab the list of the physical disks
        ##
        my %powerUpInfo = $currentMgr->powerUpState( );
        if ( ($powerUpInfo{STATUS} == PI_GOOD) &&
            ($powerUpInfo{STATE} == POWER_UP_COMPLETE) )
        {
            print("\nGetting physical disk list") if (!$debug);
            %rspList = $currentMgr->physicalDiskList();

            my $i;
            for $i (0..$#{$rspList{LIST}})
            {
                ##
                # Update the status line.  Large numbers of pdisks take forever, so
                #   give the user some feedback that something is being done.
                ##
                if ($debug)
                {
                    printf( "Getting physical disk info from PID: %d  (%d of %d)\n",
                        $rspList{LIST}[$i],
                        $i + 1,
                        $#{$rspList{LIST}} + 1 );
                }
                else
                {
                    printf( "\rGetting physical disk info from PID: %d  (%d of %d)",
                        $rspList{LIST}[$i],
                        $i + 1,
                        $#{$rspList{LIST}} + 1 );
                }

                my %pdisk;
                my @deviceID;
                my %pdiskInfo = $currentMgr->physicalDiskInfo($rspList{LIST}[$i]);

                if (%pdiskInfo)
                {
                    if ($pdiskInfo{STATUS} == PI_GOOD)
                    {
                        ##
                        # Got the physical disk information - save it into the hash
                        ##
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_PID}       = $pdiskInfo{PD_PID};
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_DNAME}     = $pdiskInfo{PD_DNAME};
                        $rsp{PDISKS}[$rsp{COUNT}]{SES}          = $pdiskInfo{SES};
                        $rsp{PDISKS}[$rsp{COUNT}]{SLOT}         = $pdiskInfo{SLOT};
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_ID}        = $pdiskInfo{PD_ID};
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_CHANNEL}   = $pdiskInfo{PD_CHANNEL};
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_LOOPMAP}   = $pdiskInfo{PD_LOOPMAP};
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_DEVTYPE}   = $pdiskInfo{PD_DEVTYPE};

                        $rsp{PDISKS}[$rsp{COUNT}]{WWN_LO} = $pdiskInfo{WWN_LO};
                        $rsp{PDISKS}[$rsp{COUNT}]{WWN_HI} = $pdiskInfo{WWN_HI};

                        ##
                        # Zero the counters before reading the log sense data
                        ##
                        $rsp{PDISKS}[$rsp{COUNT}]{CIP}          = "N/A";

                        $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_A}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_A}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_A}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_A}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_A}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_A}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_A}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_A}     = "N/A";

                        $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_B}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_B}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_B}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_B}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_B}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_B}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_B}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_B}     = "N/A";

                        $rsp{PDISKS}[$rsp{COUNT}]{POM}          = "N/A";

                        ##
                        # Only get the counters for supported devices.
                        # SATA and SSD drives don't provide the counters, so
                        # make sure we don't request the data from them.
                        ##
                        if ($pdiskInfo{PD_DEVTYPE} == PD_DT_FC_DISK ||
                            $pdiskInfo{PD_DEVTYPE} == PD_DT_ECON_ENT)
                        {
                            ##
                            # Read the logSense page from the drive to get the loop
                            #   counters for both loops.  We get the counters by
                            #   issuing the LogSense CDB on the 'temperature' log page.
                            ##
                            $deviceID[0]{WWN_LO} = $pdiskInfo{WWN_LO};
                            $deviceID[0]{WWN_HI} = $pdiskInfo{WWN_HI};
                            $deviceID[0]{PD_LUN} = $pdiskInfo{PD_LUN};

                            my $logSenseCDB = "4d004d00000000010000";
                            my $cdb = AsciiHexToBin($logSenseCDB, "byte");
                            %rspInfo = $currentMgr->scsiCmd($cdb, undef, @deviceID);

                            if (%rspInfo)
                            {
                                if ($rspInfo{STATUS} == PI_GOOD)
                                {
                                    ##
                                    # SCSI command succeeded - grab the counters from the returned data
                                    ##
                                    $currentMgr->FormatData($rspInfo{DATA}, 0x00000000, "byte", undef, 256) if ($debug);

                                    my $currentByte = 2;
                                    my $pageLength = unpack("n", substr($rspInfo{DATA}, $currentByte));
                                    $currentByte += 2;
                                    printf("Page length:    %d bytes\n", $pageLength) if ($debug);

                                    while ($currentByte < $pageLength)
                                    {
                                        my $parameterCode = unpack("n", substr($rspInfo{DATA}, $currentByte));
                                        printf("Parameter code: 0x%04x\n", $parameterCode) if ($debug);

                                        if ($parameterCode == 0x0000)
                                        {
                                            printf("Temperature data\n") if ($debug);
                                            $currentByte += 6;
                                        }
                                        elsif ($parameterCode == 0x0001)
                                        {
                                            printf("Reference temperature data\n") if ($debug);
                                            $currentByte += 6;
                                        }
                                        elsif ($parameterCode == 0x0002)
                                        {
                                            printf("Undocumented temperature data\n") if ($debug);
                                            $currentByte += 6;
                                        }
                                        elsif ($parameterCode == 0x80FF)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{CIP} = unpack("b", substr($rspInfo{DATA}, $currentByte + 5));
                                            printf("CIP:      0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{CIP}) if ($debug);
                                            $currentByte += 6;
                                        }
                                        elsif ($parameterCode == 0x8100)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_A}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LFCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8101)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_A}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LSCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_A}) if ($debug);
                                        }
                                        elsif ( ($parameterCode == 0x8102) ||
                                               ($parameterCode == 0x8103) )
                                        {
                                            printf("Undocumented parameter data\n") if ($debug);
                                            $currentByte += 8;
                                        }
                                        elsif ($parameterCode == 0x8104)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_A}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("ITCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8105)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_A}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("ICCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8106)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_A} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF7I_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8107)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_A} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF7R_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8108)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_A} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF8I_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8109)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_A} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF8R_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8110)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_B}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LFCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8111)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_B}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LSCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_B}) if ($debug);
                                        }
                                        elsif ( ($parameterCode == 0x8112) ||
                                               ($parameterCode == 0x8113) )
                                        {
                                            printf("Undocumented parameter data\n") if ($debug);
                                            $currentByte += 8;
                                        }
                                        elsif ($parameterCode == 0x8114)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_B}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("ITCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8115)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_B}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("ICCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8116)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_B} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF7I_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8117)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_B} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF7R_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8118)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_B} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF8I_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8119)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_B} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF8R_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_B}) if ($debug);
                                        }
                                        else
                                        {
                                            printf("Default\n") if ($debug);
                                            $currentByte = $pageLength;
                                        }
                                    }

                                    printf("\n") if ($debug);
                                }
                            }

                            ##
                            # Read the modeSenseControl page from the drive to get
                            #   the state of the GLTSD bit.
                            ##
#---                            if (0)
#---                            {
#---                                printf("\n") if ($debug);
#---                                my $modeSenseCDB = "1a080a00ff00";
#---                                $cdb = AsciiHexToBin($modeSenseCDB, "byte");
#---                                %rspInfo = $currentMgr->scsiCmd($cdb, undef, @deviceID);
#---
#---                                if (%rspInfo)
#---                                {
#---                                    if ($rspInfo{STATUS} == PI_GOOD)
#---                                    {
#---                                        ##
#---                                        # SCSI command succeeded
#---                                        ##
#---                                        $currentMgr->FormatData($rspInfo{DATA}, 0x00000000, "byte", undef, 256) if ($debug);
#---                                    }
#---                                    else
#---                                    {
#---                                        printf("Mode sense failed\n") if ($debug);
#---                                    }
#---                                }
#---
#---                                printf("\n") if ($debug);
#---                            }

                            ##
                            # Read the factoryLog page from the drive to get the
                            #   power-on minute counter for the drive.  We get the
                            #   counters by issuing the LogSense CDB for the
                            #   'factory log' page.
                            ##
                            printf("\n") if ($debug);
                            my $factoryLogPageCDB = "4d007e00000000010000";
                            $cdb = AsciiHexToBin($factoryLogPageCDB, "byte");
                            %rspInfo = $currentMgr->scsiCmd($cdb, undef, @deviceID);

                            if (%rspInfo)
                            {
                                if ($rspInfo{STATUS} == PI_GOOD)
                                {
                                    ##
                                    # SCSI command succeeded - grab the counters from the returned data
                                    ##
                                    $currentMgr->FormatData($rspInfo{DATA}, 0x00000000, "byte", undef, 256) if ($debug);

                                    my $currentByte = 2;
                                    my $pageLength = unpack("n", substr($rspInfo{DATA}, $currentByte));
                                    $currentByte += 2;
                                    printf("Page length:    %d bytes\n", $pageLength) if ($debug);

                                    while ($currentByte < $pageLength)
                                    {
                                        my $parameterCode = unpack("n", substr($rspInfo{DATA}, $currentByte));
                                        printf("Parameter code: 0x%04x\n", $parameterCode) if ($debug);

                                        if ($parameterCode == 0x0000)
                                        {
                                            printf("Power On Minutes\n") if ($debug);

                                            my $reserved = unpack("C", substr($rspInfo{DATA}, $currentByte + 2));
                                            printf("  Reserved:     0x%02x\n", $reserved) if ($debug);

                                            my $dataLength = unpack("C", substr($rspInfo{DATA}, $currentByte + 3));
                                            printf("  Data length:  0x%02x\n", $dataLength) if ($debug);

                                            $rsp{PDISKS}[$rsp{COUNT}]{POM} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            printf("  Data:         0x%08x (%d minutes)\n",
                                                $rsp{PDISKS}[$rsp{COUNT}]{POM}, $rsp{PDISKS}[$rsp{COUNT}]{POM}) if ($debug);

                                            $currentByte += 8;
                                        }
                                        elsif ($parameterCode == 0x0008)
                                        {
                                            printf("SMART Minute Timer\n") if ($debug);

                                            my $reserved = unpack("C", substr($rspInfo{DATA}, $currentByte + 2));
                                            printf("  Reserved:     0x%02x\n", $reserved) if ($debug);

                                            my $dataLength = unpack("C", substr($rspInfo{DATA}, $currentByte + 3));
                                            printf("  Data length:  0x%02x\n", $dataLength) if ($debug);

                                            my $data = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            printf("  Data:         0x%08x (%d minutes)\n", $data, $data) if ($debug);
                                            $currentByte += 8;
                                        }
                                        else
                                        {
                                            printf("Default\n") if ($debug);
                                            $currentByte = $pageLength;
                                        }

                                    }

                                    printf("\n") if ($debug);
                                }
                            }
                        }

                        ##
                        # Bump the pdisk counter, but do this LAST!
                        ##
                        $rsp{COUNT} = $rsp{COUNT} + 1;
                    }
                    else
                    {
                        my $msg = "\nERROR: Unable to retrieve pdisk info.";
                        displayError($msg, %pdiskInfo);
                        exit -1;
                    }
                }
                else
                {
                    print "\nERROR: Could not retrieve pdisk WWN.\n";
                    exit -1;
                }
            }

            ##
            # Clear the status line and return to the beginning of the same line
            ##
            print("\r                                                              \r") if (!$debug);
        }
        else
        {
            print "\nERROR: Bad powerup state\n";
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
        }
    }
    else
    {
        %rsp = $currentMgr->physicalDisks();
    }

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayPhysicalDisks($dsptype, %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve physical disk information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}






##############################################################################
# Name:     pdiskscache
#
# Desc:     Displays physical disk information for all physical disks from cache
#
# Input:    None
##############################################################################
sub pdiskscache
{
    my ($dsptype) = @args;

    my %rsp;
    my %rspList;
    my %rspInfo;

    print "\n";

    if (!defined($dsptype))
    {
        $dsptype = "STD";
    }

    if (uc($dsptype) eq "LOOP")
    {
        $rsp{STATUS} = PI_GOOD;
        $rsp{COUNT} = 0;
        my $debug = 0;

        ##
        # Make the status line and grab the list of the physical disks
        ##
        my %powerUpInfo = $currentMgr->powerUpState( );
        if ( ($powerUpInfo{STATUS} == PI_GOOD) &&
            ($powerUpInfo{STATE} == POWER_UP_COMPLETE) )
        {
            print("\nGetting physical disk list") if (!$debug);
            %rspList = $currentMgr->physicalDiskList();

            my $i;
            for $i (0..$#{$rspList{LIST}})
            {
                ##
                # Update the status line.  Large numbers of pdisks take forever, so
                #   give the user some feedback that something is being done.
                ##
                if ($debug)
                {
                    printf( "Getting physical disk info from PID: %d  (%d of %d)\n",
                        $rspList{LIST}[$i],
                        $i + 1,
                        $#{$rspList{LIST}} + 1 );
                }
                else
                {
                    printf( "\rGetting physical disk info from PID: %d  (%d of %d)",
                        $rspList{LIST}[$i],
                        $i + 1,
                        $#{$rspList{LIST}} + 1 );
                }

                my %pdisk;
                my @deviceID;
                my %pdiskInfo = $currentMgr->physicalDiskInfo($rspList{LIST}[$i]);

                if (%pdiskInfo)
                {
                    if ($pdiskInfo{STATUS} == PI_GOOD)
                    {
                        ##
                        # Got the physical disk information - save it into the hash
                        ##
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_PID}       = $pdiskInfo{PD_PID};
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_DNAME}     = $pdiskInfo{PD_DNAME};
                        $rsp{PDISKS}[$rsp{COUNT}]{SES}          = $pdiskInfo{SES};
                        $rsp{PDISKS}[$rsp{COUNT}]{SLOT}         = $pdiskInfo{SLOT};
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_ID}        = $pdiskInfo{PD_ID};
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_CHANNEL}   = $pdiskInfo{PD_CHANNEL};
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_LOOPMAP}   = $pdiskInfo{PD_LOOPMAP};
                        $rsp{PDISKS}[$rsp{COUNT}]{PD_DEVTYPE}   = $pdiskInfo{PD_DEVTYPE};

                        $rsp{PDISKS}[$rsp{COUNT}]{WWN_LO} = $pdiskInfo{WWN_LO};
                        $rsp{PDISKS}[$rsp{COUNT}]{WWN_HI} = $pdiskInfo{WWN_HI};

                        ##
                        # Zero the counters before reading the log sense data
                        ##
                        $rsp{PDISKS}[$rsp{COUNT}]{CIP}          = "N/A";

                        $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_A}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_A}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_A}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_A}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_A}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_A}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_A}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_A}     = "N/A";

                        $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_B}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_B}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_B}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_B}      = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_B}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_B}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_B}     = "N/A";
                        $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_B}     = "N/A";

                        $rsp{PDISKS}[$rsp{COUNT}]{POM}          = "N/A";

                        ##
                        # Only get the counters for supported devices.
                        # SATA and SSD drives don't provide the counters, so
                        # make sure we don't request the data from them.
                        ##
                        if ($pdiskInfo{PD_DEVTYPE} == PD_DT_FC_DISK ||
                            $pdiskInfo{PD_DEVTYPE} == PD_DT_ECON_ENT)
                        {
                            ##
                            # Read the logSense page from the drive to get the loop
                            #   counters for both loops.  We get the counters by
                            #   issuing the LogSense CDB on the 'temperature' log page.
                            ##
                            $deviceID[0]{WWN_LO} = $pdiskInfo{WWN_LO};
                            $deviceID[0]{WWN_HI} = $pdiskInfo{WWN_HI};
                            $deviceID[0]{PD_LUN} = $pdiskInfo{PD_LUN};

                            my $logSenseCDB = "4d004d00000000010000";
                            my $cdb = AsciiHexToBin($logSenseCDB, "byte");
                            %rspInfo = $currentMgr->scsiCmd($cdb, undef, @deviceID);

                            if (%rspInfo)
                            {
                                if ($rspInfo{STATUS} == PI_GOOD)
                                {
                                    ##
                                    # SCSI command succeeded - grab the counters from the returned data
                                    ##
                                    $currentMgr->FormatData($rspInfo{DATA}, 0x00000000, "byte", undef, 256) if ($debug);

                                    my $currentByte = 2;
                                    my $pageLength = unpack("n", substr($rspInfo{DATA}, $currentByte));
                                    $currentByte += 2;
                                    printf("Page length:    %d bytes\n", $pageLength) if ($debug);

                                    while ($currentByte < $pageLength)
                                    {
                                        my $parameterCode = unpack("n", substr($rspInfo{DATA}, $currentByte));
                                        printf("Parameter code: 0x%04x\n", $parameterCode) if ($debug);

                                        if ($parameterCode == 0x0000)
                                        {
                                            printf("Temperature data\n") if ($debug);
                                            $currentByte += 6;
                                        }
                                        elsif ($parameterCode == 0x0001)
                                        {
                                            printf("Reference temperature data\n") if ($debug);
                                            $currentByte += 6;
                                        }
                                        elsif ($parameterCode == 0x0002)
                                        {
                                            printf("Undocumented temperature data\n") if ($debug);
                                            $currentByte += 6;
                                        }
                                        elsif ($parameterCode == 0x80FF)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{CIP} = unpack("b", substr($rspInfo{DATA}, $currentByte + 5));
                                            printf("CIP:      0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{CIP}) if ($debug);
                                            $currentByte += 6;
                                        }
                                        elsif ($parameterCode == 0x8100)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_A}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LFCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8101)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_A}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LSCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_A}) if ($debug);
                                        }
                                        elsif ( ($parameterCode == 0x8102) ||
                                               ($parameterCode == 0x8103) )
                                        {
                                            printf("Undocumented parameter data\n") if ($debug);
                                            $currentByte += 8;
                                        }
                                        elsif ($parameterCode == 0x8104)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_A}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("ITCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8105)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_A}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("ICCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8106)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_A} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF7I_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8107)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_A} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF7R_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8108)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_A} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF8I_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8109)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_A} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF8R_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_A}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8110)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_B}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LFCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8111)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_B}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LSCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_B}) if ($debug);
                                        }
                                        elsif ( ($parameterCode == 0x8112) ||
                                               ($parameterCode == 0x8113) )
                                        {
                                            printf("Undocumented parameter data\n") if ($debug);
                                            $currentByte += 8;
                                        }
                                        elsif ($parameterCode == 0x8114)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_B}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("ITCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8115)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_B}  = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("ICCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8116)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_B} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF7I_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8117)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_B} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF7R_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8118)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_B} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF8I_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_B}) if ($debug);
                                        }
                                        elsif ($parameterCode == 0x8119)
                                        {
                                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_B} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            $currentByte += 8;
                                            printf("LIPF8R_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_B}) if ($debug);
                                        }
                                        else
                                        {
                                            printf("Default\n") if ($debug);
                                            $currentByte = $pageLength;
                                        }
                                    }

                                    printf("\n") if ($debug);
                                }
                            }

                            ##
                            # Read the modeSenseControl page from the drive to get
                            #   the state of the GLTSD bit.
                            ##
#---                            if (0)
#---                            {
#---                                printf("\n") if ($debug);
#---                                my $modeSenseCDB = "1a080a00ff00";
#---                                $cdb = AsciiHexToBin($modeSenseCDB, "byte");
#---                                %rspInfo = $currentMgr->scsiCmd($cdb, undef, @deviceID);
#---
#---                                if (%rspInfo)
#---                                {
#---                                    if ($rspInfo{STATUS} == PI_GOOD)
#---                                    {
#---                                        ##
#---                                        # SCSI command succeeded
#---                                        ##
#---                                        $currentMgr->FormatData($rspInfo{DATA}, 0x00000000, "byte", undef, 256) if ($debug);
#---                                    }
#---                                    else
#---                                    {
#---                                        printf("Mode sense failed\n") if ($debug);
#---                                    }
#---                                }
#---
#---                                printf("\n") if ($debug);
#---                            }

                            ##
                            # Read the factoryLog page from the drive to get the
                            #   power-on minute counter for the drive.  We get the
                            #   counters by issuing the LogSense CDB for the
                            #   'factory log' page.
                            ##
                            printf("\n") if ($debug);
                            my $factoryLogPageCDB = "4d007e00000000010000";
                            $cdb = AsciiHexToBin($factoryLogPageCDB, "byte");
                            %rspInfo = $currentMgr->scsiCmd($cdb, undef, @deviceID);

                            if (%rspInfo)
                            {
                                if ($rspInfo{STATUS} == PI_GOOD)
                                {
                                    ##
                                    # SCSI command succeeded - grab the counters from the returned data
                                    ##
                                    $currentMgr->FormatData($rspInfo{DATA}, 0x00000000, "byte", undef, 256) if ($debug);

                                    my $currentByte = 2;
                                    my $pageLength = unpack("n", substr($rspInfo{DATA}, $currentByte));
                                    $currentByte += 2;
                                    printf("Page length:    %d bytes\n", $pageLength) if ($debug);

                                    while ($currentByte < $pageLength)
                                    {
                                        my $parameterCode = unpack("n", substr($rspInfo{DATA}, $currentByte));
                                        printf("Parameter code: 0x%04x\n", $parameterCode) if ($debug);

                                        if ($parameterCode == 0x0000)
                                        {
                                            printf("Power On Minutes\n") if ($debug);

                                            my $reserved = unpack("C", substr($rspInfo{DATA}, $currentByte + 2));
                                            printf("  Reserved:     0x%02x\n", $reserved) if ($debug);

                                            my $dataLength = unpack("C", substr($rspInfo{DATA}, $currentByte + 3));
                                            printf("  Data length:  0x%02x\n", $dataLength) if ($debug);

                                            $rsp{PDISKS}[$rsp{COUNT}]{POM} = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            printf("  Data:         0x%08x (%d minutes)\n",
                                                $rsp{PDISKS}[$rsp{COUNT}]{POM}, $rsp{PDISKS}[$rsp{COUNT}]{POM}) if ($debug);

                                            $currentByte += 8;
                                        }
                                        elsif ($parameterCode == 0x0008)
                                        {
                                            printf("SMART Minute Timer\n") if ($debug);

                                            my $reserved = unpack("C", substr($rspInfo{DATA}, $currentByte + 2));
                                            printf("  Reserved:     0x%02x\n", $reserved) if ($debug);

                                            my $dataLength = unpack("C", substr($rspInfo{DATA}, $currentByte + 3));
                                            printf("  Data length:  0x%02x\n", $dataLength) if ($debug);

                                            my $data = unpack("N", substr($rspInfo{DATA}, $currentByte + 4));
                                            printf("  Data:         0x%08x (%d minutes)\n", $data, $data) if ($debug);
                                            $currentByte += 8;
                                        }
                                        else
                                        {
                                            printf("Default\n") if ($debug);
                                            $currentByte = $pageLength;
                                        }

                                    }

                                    printf("\n") if ($debug);
                                }
                            }
                        }

                        ##
                        # Bump the pdisk counter, but do this LAST!
                        ##
                        $rsp{COUNT} = $rsp{COUNT} + 1;
                    }
                    else
                    {
                        my $msg = "\nERROR: Unable to retrieve pdisk info.";
                        displayError($msg, %pdiskInfo);
                        exit -1;
                    }
                }
                else
                {
                    print "\nERROR: Could not retrieve pdisk WWN.\n";
                    exit -1;
                }
            }

            ##
            # Clear the status line and return to the beginning of the same line
            ##
            print("\r                                                              \r") if (!$debug);
        }
        else
        {
            print "\nERROR: Bad powerup state\n";
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
        }
    }
    else
    {
        %rsp = $currentMgr->physicalDisksCache();
    }

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayPhysicalDisks($dsptype, %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve physical disk information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}






##############################################################################
# Name:     pdiskBeacon
#
# Desc:     Beacon a physical disks.
#
# Input:    ID of the physical disk.
##############################################################################
sub pdiskBeacon
{
    my ($id, $dur) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing physical disk identifier.\n";
        return;
    }

    if (!defined($dur))
    {
        $dur = 10;
    }

    my %rsp = $currentMgr->physicalDiskBeacon($id, $dur);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Physical disk ($id) is beaconing.\n";
        }
        else
        {
            my $msg = "Unable to beacon physical disk ($id).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pdiskCount
#
# Desc:     Displays the current count of physical disks.
#
# Input:    None
##############################################################################
sub pdiskCount
{
    print "\n";

    my %rsp = $currentMgr->getObjectCount(PI_PDISK_COUNT_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Number of physical disks: " . $rsp{COUNT} . "\n";
        }
        else
        {
            my $msg = "Unable to retrieve number of physical disks.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     miscCount
#
# Desc:     Displays the current count of MISC devices.
#
# Input:    None
##############################################################################
sub miscCount
{
    print "\n";

    my %rsp = $currentMgr->getObjectCount(PI_MISC_COUNT_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Number of MISC devices: " . $rsp{COUNT} . "\n";
        }
        else
        {
            my $msg = "Unable to retrieve number of MISC devices.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}   # End of miscCount

##############################################################################
# Name:     pdiskDefrag
#
# Desc:     Defragment a physical disks.
#
# Input:    ID of the physical disk.
##############################################################################
sub pdiskDefrag
{
    my ($id) = @args;

    print "\n";

    if (uc($id) eq "ALL")
    {
        $id = 0xFFFF;
    }
    elsif (uc($id) eq "STOP")
    {
        $id = 0xFFFE;
    }
    elsif (uc($id) eq "ORPHAN")
    {
        $id = 0xFFFD;
    }

    if (defined($id) && $id =~ /^0x/i)
    {
        $id = oct $id;
    }

    if (!defined($id) || uc($id) eq "STATUS" || $id < 0)
    {
        pdiskDefragStatus();
    }
    else
    {
        my %rsp = $currentMgr->physicalDiskDefrag($id);

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                if ($id == 0xFFFF)
                {
                    $id = "ALL";
                }
                elsif ($id == 0xFFFE)
                {
                    $id = "STOP";
                }
                elsif ($id == 0xFFFD)
                {
                    $id = "ORPHAN";
                }

                print "Physical disk ($id) is defragmenting.\n";
            }
            else
            {
                my $msg = "Unable to defragment physical disk ($id).";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }

    print "\n";
}

##############################################################################
# Name:     pdiskDefragStatus
#
# Desc:     Status of Defragment of physical disk(s).
#
# Input:    none
##############################################################################
sub pdiskDefragStatus
{
    print "\n";


    my %rsp = $currentMgr->physicalDiskDefragStatus();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayPDiskDefragStatus(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve physical disk defrag status.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pdiskFail
#
# Desc:     Fail a physical disks.
#
# Input:    ID of the physical disk.
# Input:    WWN of the physical disk.
#           FORCE option
##############################################################################
sub pdiskFail
{
    my ($id, $hspid, $force) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "\n";
        print "Invalid or missing physical disk identifier.\n";
        return;
    }

    if (!defined($force))
    {
        $force = 0;
    }

    # convert from hex if hex was used
    if ($force =~ /^0x/i) {
        $force = oct $force;
    }

    if (!defined($hspid))
    {
        $hspid = 0;
    }

    my %rsp = $currentMgr->physicalDiskFail($id, $hspid, $force);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Physical disk ($id) is failed.\n";
        }
        else
        {
            my $msg = "Unable to fail physical disk ($id).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pdiskInfo
#
# Desc:     Displays information for a physical disks.
#
# Input:    ID of the physical disk.
##############################################################################
sub pdiskInfo
{
    my ($id, $options) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing physical disk identifier.\n";
        return;
    }

    if (!defined($options))
    {
        $options = 0;
    }

    my %rsp = $currentMgr->physicalDiskInfo($id, $options);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayPhysicalDiskInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve physical disk information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pdiskLabel
#
# Desc:     Label a physical disk.
#
# Input:    ID of the physical disk, label type.
##############################################################################
sub pdiskLabel
{
    my ($id, $type) = @args;

    print "\n";

    if (!defined($id))
    {
        print "Missing physical disk identifier.\n";
        return;
    }

    if (!defined($type))
    {
        $type = LABEL_TYPE_DATA;
    }

    if ($type =~ /^0x/i)
    {
        $type = oct $type;
    }

    my @pids;
    my $msg;

    if (uc($id) eq "ALL" or uc($id) eq "0XFFFF")
    {
        my %rsplist = $currentMgr->physicalDiskList();

        if (%rsplist)
        {
            if ($rsplist{STATUS} == PI_GOOD)
            {
                for my $i (0..$#{$rsplist{LIST}})
                {
                    $pids[$i] = $rsplist{LIST}[$i];
                }
            }
            else
            {
                $msg = "Unable to retrieve list of physical disks.";
                displayError($msg, %rsplist);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }
    else
    {
        @pids = $currentMgr->rangeToList($id);
    }

    if (uc($id) eq "ALL" or uc($id) eq "0XFFFF")
    {
        print "Labeling all visible physical disks...\n";
    }

    my %rsp = $currentMgr->physicalDiskLabel(\@pids, $type);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Physical disks labeled ($type) - ";
            for (my $i = 0; $i < scalar(@pids); $i++)
            {
                if ($i > 0)
                {
                    print ",";
                }

                printf "%hu", $pids[$i];
            }

            print "\n";
        }
        else
        {
            $msg = "Unable to label physical disk(s).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     getGeoLocation
#
# Desc:     Gets the Geo location of the bay and drives.
#
# Input:    NONE.
##############################################################################
sub getGeoLocation
{
    print "\n";

    my %rsp = $currentMgr->geoLocationStatus();

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable retrieve Geo Location Status.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     clearGeoLocation
#
# Desc:     Clears the Geo location of all the bays and its associated drives.
#
# Input:    NONE.
##############################################################################
sub clearGeoLocation
{
    print "\n";

    my %rsp = $currentMgr->clearGeoLocation();

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to clear the Geo Location.";
            displayError($msg, %rsp);
        }

        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = "Successfully cleared the Geo Location of all the Bays.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     setGeoLocation
#
# Desc:     Set the Geo location of the bay and drives.
#
# Input:    ID of the Bay to be set.
# Input:    Location to be set.
##############################################################################
sub setGeoLocation
{
    my ($bayId, $location) = @args;

    print "\n";

    if ((!defined($bayId)) && (!defined($location)))
    {
        print "Missing Bay ID and location Code.\n";
        return;
    }

    if (!defined($bayId))
    {
        print "Missing Bay ID.\n";
        return;
    }

    if (!defined($location))
    {
        print "Missing location Code.\n";
        return;
    }

    my %rsp = $currentMgr->setBayGeoLocation($bayId, $location);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Successfully set the Geo location of the Bay ($bayId).";
            print "\n";
            return;
        }
        else
        {
            my $msg = "Unable to set the geolocation.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     pdiskSpindown
#
# Desc:     Spin down a physical disk.
#
# Input:    ID of the physical disk.
##############################################################################
sub pdiskSpindown
{
    my ($id, $type) = @args;

    print "\n";

    if (!defined($id))
    {
        print "Missing physical disk identifier.\n";
        return;
    }

    my %rsp = $currentMgr->physicalDiskSpindown($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Physical disk ($id) spin down successful.";
            print "\n";
        }
        else
        {
            my $msg = "Unable to spindown physical disk.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     dlmPathSelectionAlgo
#
# Desc:     Selects the DLM Path Algorithm in case of ICL.
#
# Input:    option.
##############################################################################
sub dlmPathSelectionAlgo
{
    my ($option) = @args;

    print "\n";

    if (!defined($option))
    {
        print  "Unable to select the DLM Path Algorithm.\n";
        print  "No option given.\n";
        return;
    }
    else
    {
        my %rsp = $currentMgr->dlmPathSelectionAlgorithm($option);

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
#                my $msg = $currentMgr->displaydlmPathSelectionAlgorithm(%rsp);
#                print $msg;
            }
            else
            {
                my $msg = "Unable to select the DLM Path Algorithm.";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }
    print "\n";
}


##############################################################################
# Name:     pdiskAutoFailBack
#
# Desc:     Enables/Disables the auto failback feature.
#
# Input:    option.
##############################################################################
sub pdiskAutoFailBack
{
    my ($option) = @args;

    print "\n";

    if (!defined($option))
    {
        $option = 0;
    }
    my %rsp = $currentMgr->physicalDiskAutoFailBack($option);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayPDiskAutoFailback(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to enable/disable the auto failback feature.";
            $msg .= "Invalid option given.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
    print "\n";
}


##############################################################################
# Name:     pdiskFailBack
#
# Desc:     Unfail an used hotspare physical disks.
#
# Input:    ID of the used hotspare physical disk.
# Input:    option.
##############################################################################
sub pdiskFailBack
{
    my ($id, $option) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "\n";
        print "Invalid or missing used hotspare physical disk identifier.\n";
        return;
    }

    if (!defined($option))
    {
        $option = 0;
    }

    my %rsp = $currentMgr->physicalDiskFailBack($id, $option);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            if ($option == 0)
            {
                print "Used hotspare Physical disk ($id) is failed back to its original location.\n";
                print "The used hotspare now becomes unused hotspare.\n"
            }
            else
            {
                print "FailBack for the designated hotspare is cancelled.\n";
                print "You can never failback this device.\n";
            }
        }
        else
        {
            my $msg = "Unable to failback the used hotspare physical disk ($id).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     miscList
#
# Desc:     Displays a list of MISC device identifiers.
#
# Input:    None
##############################################################################
sub miscList
{
    print "\n";

    my %rsp = $currentMgr->getObjectList(PI_MISC_LIST_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "MISC Device List:\n";
            my $msg = $currentMgr->displayObjectList(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve list of MISC device identifiers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pdiskList
#
# Desc:     Displays a list of physical disk identifiers.
#
# Input:    None
##############################################################################
sub pdiskList
{
    print "\n";

    my %rsp = $currentMgr->getObjectList(PI_PDISK_LIST_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Physical Disk List:\n";
            my $msg = $currentMgr->displayObjectList(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve list of physical disk identifiers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pdiskRestore
#
# Desc:     Restore a physical disks.
#
# Input:    ID of the physical disk.
##############################################################################
sub pdiskRestore
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing physical disk identifier.\n";
        return;
    }

    my %rsp = $currentMgr->physicalDiskRestore($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Physical disk ($id) restored.\n";
        }
        else
        {
            my $msg = "Unable to retrieve physical disk information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pdiskDelete
#
# Desc:     Deletes a pdisk.
#
# Input:    pid   -  Device ID
##############################################################################
sub pdiskDelete
{
    my ($pid) = @args;

    if (!defined($pid))
    {
        print "Missing id\n";
        return;
    }

    my %rsp = $currentMgr->physicalDiskDelete($pid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Successful physical disk delete id: $pid.\n";
        }
        else
        {
            my $msg = "Unable to delete device.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
    print "\n";
}

##############################################################################
# Name:     pdiskLedState
#
# Desc:     Sets the led state of a controller's pdisk.
#
# Input:    type    -   contoller
#                       interface
##############################################################################
sub pdiskLedState
{
    print "\n";

    my ($pid, $state) = @args;
    my @parmArray;
    my $cmd;
    my $wwn1;
    my $wwn2;

    if (!defined($pid))
    {
        print "Invalid or missing pid.\n";
        return;
    }

    if (!defined($state))
    {
        print "Invalid or missing state.\n";
        return;
    }
    elsif (($state < 0) || ($state > 4))
    {
        print "Invalid state.\n";
        return;
    }

    my %rsp = $currentMgr->physicalDiskInfo($pid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            push @parmArray, $rsp{WWN_LO};
            push @parmArray, $rsp{WWN_HI};
            push @parmArray, $state;

            %rsp = $currentMgr->genericCommand("SET_LED", @parmArray);

            if (%rsp)
            {
                if ($rsp{STATUS} == PI_GOOD)
                {
                    print "LED STATE SET.\n";
                }
                else
                {
                    my $msg = "ERROR: Unable to set led state.";
                    displayError($msg, %rsp);
                }
            }
            else
            {
                print "ERROR: Did not receive a response packet.\n";
                logout();
            }
        }
        else
        {
            my $msg = "Unable to retrieve physical disk information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }



    print "\n";
}

##############################################################################
# Name:     pdiskBypass
#
# Desc:     Bypass a physical disks.
#
# Input:    ID of the physical disk.
##############################################################################
sub pdiskBypass
{
    my ($ses, $slot, $setting) = @args;

    print "\n";

    if (!defined($ses) || $ses < 0)
    {
        print "Invalid or missing SES.\n";
        return;
    }

    if (!defined($slot) || $slot < 0)
    {
        print "Invalid or missing slot.\n";
        return;
    }

    if ($setting =~ /^0x/i)
    {
        $setting = oct $setting;
    }

    my %rsp = $currentMgr->physicalDiskBypass($ses, $slot, $setting);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            printf "Physical disk (%d, %d) bypass (0x%x).\n",
                    $ses,
                    $slot,
                    $setting;
        }
        else
        {
            my $msg = "Unable to bypass physical disk.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     pdiskTimeout
#
# Desc:     Timeout a physical disks (emulate qlogic timeout).
#
# Input:    ID of the physical disk.
##############################################################################
sub pdiskTimeout
{
    my ($pid, $setting) = @args;

    print "\n";

    if (!defined($pid) || $pid < 0)
    {
        print "Invalid or missing SES.\n";
        return;
    }

    if (!defined($setting))
    {
        print "Invalid or missing setting.\n";
        return;
    }
    elsif (($setting < 0) || ($setting > 1))
    {
        print "Invalid setting -- must be 0 or 1.\n";
        return;
    }

    if ($setting =~ /^0x/i)
    {
        $setting = oct $setting;
    }

    my %rsp = $currentMgr->pdiskEmulateQlogicTimeout($pid, $setting);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            printf "Physical disk (%d) emulate qlogic timeout (0x%x).\n",
                    $pid, $setting;
        }
        else
        {
            my $msg = "Unable to emulate qlogic timeout on physical disk.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     powerUpState
#
# Desc:     Displays the current power-up state of the controller.
#
# Inputs:   NONE
##############################################################################
sub powerUpState
{
    print "\n";

    my %rsp = $currentMgr->powerUpState();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayPowerUpState(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve power-up state.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
}

##############################################################################
# Name:     powerUpResponse
#
# Desc:     Sends a response to a power-up "wait" state.
#
# Inputs:   serial number of controller
##############################################################################
sub powerUpResponse
{
    my ($state, $astatus, $response) = @args;

    print "\n";

    if (!defined($state))
    {
        print "Missing state parameter\n";
        return;
    }

    if (!defined($astatus))
    {
        print "Missing astatus parameter\n";
        return;
    }

    if (!defined($response))
    {
        print "Missing response parameter\n";
        return;
    }

    if ($state =~ /^0x/i)
    {
        $state = oct $state;
    }

    if ($astatus =~ /^0x/i)
    {
        $astatus = oct $astatus;
    }

    my %rsp = $currentMgr->powerUpResponse($state, $astatus, $response);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            printf "Power-up response sent.\n";
        }
        else
        {
            my $msg = "Unable to send power-up response.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     rmState
#
# Desc:     Displays the current resource manager state for the controller.
#
# Inputs:   NONE
##############################################################################
sub rmState
{
    print "\n";

    my %rsp = $currentMgr->rmState();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayRMState(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve resource manager state.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     diskBays
#
# Desc:     Displays disk bay information for all disk bays.
#
# Input:    None
##############################################################################
sub diskBays
{
    print "\n";

    my %rsp = $currentMgr->diskBays();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayDiskBays(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve disk bays.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     counts
#
# Desc:     Displays the current counts of things.
#
# Input:    None
##############################################################################
sub counts
{
    getCpuCount();

    @args = ("BE", "4");
    portList();
    diskBayCount();
    pdiskCount();
    miscCount();
    vdiskCount();
    raidCount();

    @args = ("FE", "4");
    portList();
    targetCount();
    serverCount();
    vlinkCtrlCount();
}   # End of counts

##############################################################################
# Name:     diskBayCount
#
# Desc:     Displays the current count of physical disks.
#
# Input:    None
##############################################################################
sub diskBayCount
{
    print "\n";

    my %rsp = $currentMgr->diskBayCount();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Number of disk bays: " . $rsp{COUNT} . "\n";
        }
        else
        {
            my $msg = "Unable to retrieve number of disk bays.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     diskBayList
#
# Desc:     Displays a list of disk bay identifiers.
#
# Input:    None
##############################################################################
sub diskBayList
{
    print "\n";

    my %rsp = $currentMgr->diskBayList();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Disk Bay List:\n";
            my $msg = $currentMgr->displayObjectList(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve list of disk bay identifiers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     diskBayEnviro
#
# Desc:     Displays Environmental information for a disk Bay.
#
# Input:    ID of the disk bay.
##############################################################################
sub diskBayEnviro
{
    my ($bid) = @args;

    print "\n";

    if (!defined($bid) || $bid < 0)
    {
        print "Invalid or missing disk bay identifier.\n";
        return;
    }

    my %rsp = $currentMgr->diskBayEnviro($bid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayDiskBayEnviro(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve disk bay environmental information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     diskBayInfo
#
# Desc:     Displays information for a disk Bay.
#
# Input:    ID of the disk bay.
##############################################################################
sub diskBayInfo
{
    my ($bid) = @args;

    print "\n";

    if (!defined($bid) || $bid < 0)
    {
        print "Invalid or missing disk bay identifier.\n";
        return;
    }

    my %rsp = $currentMgr->diskBayInfo($bid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayDiskBayInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve disk bay information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     diskBayAlarmCtrl
#
# Desc:     Sends alarm control byte to the disk bay
#
# Input:    bid      - ID of the disk bay.
#           settings - alarm control byte
##############################################################################
sub diskBayAlarmControl
{
    my ($bid, $settings) = @args;

    print "\n";

    if (!defined($bid) || $bid < 0)
    {
        print "Invalid or missing disk bay identifier.\n";
        return;
    }

    if (!defined($settings) || $settings eq "")
    {
        print "Missing disk bay alarm control settings value.\n";
        return;
    }

    # Convert settings from hex if necessary
    if ($settings =~ /^0x/i)
    {
        $settings = oct $settings;
    }


    my %rsp = $currentMgr->diskBayAlarmControl($bid, $settings);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Disk bay alarm control value was set.\n";
        }
        else
        {
            my $msg = "Unable to set the disk bay alarm control value.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     singlevids
#
# Desc:     Single PSD(PID) VIDs on ISEs
#
##############################################################################
sub singlevidsonise()
{
    my @Bays2List;
    my $ListThese = 0;

    # Parse arguments.
    while (defined($args[$ListThese]))
    {
        $Bays2List[$ListThese] = $args[$ListThese];
        $ListThese += 1;
    }

    # Get current diskbay information.
    my %bays = $currentMgr->diskBays();
    if (!%bays || $bays{STATUS} != PI_GOOD)
    {
        printf "Error getting bay statuses\n";
        return;
    }

    # Get current physical disk information.
    my %disks = $currentMgr->physicalDisks();
    if (!%disks || $disks{STATUS} != PI_GOOD)
    {
        printf "Error getting pdisk statuses\n";
        return;
    }

    # Get current virtual disk information.
    my %vdisks = $currentMgr->virtualDisks();
    if (!%vdisks || $vdisks{STATUS} != PI_GOOD)
    {
        printf "Error getting vdisk statuses\n";
        return;
    }

    # Get devstat raid information.
    my %rsp = $currentMgr->deviceStatus("RD");
    if (!%rsp || $rsp{STATUS} != PI_GOOD)
    {
        printf "Error getting raid device statuses\n";
        return;
    }

    my @pdisks;
    my @dbays;
    my $i;
    my $j;
    for ($i = 0; $i < $bays{COUNT}; ++$i)
    {
        $dbays[$i]{PD_BID} = $bays{BAYS}[$i]{PD_BID};
        my $count = 0;
        for ($j = 0; $j < $disks{COUNT}; ++$j)
        {
            if ($bays{BAYS}[$i]{PD_BID} == $disks{PDISKS}[$j]{SES})
            {
                $pdisks[$i][$count]{PD_PID} = $disks{PDISKS}[$j]{PD_PID};
                $pdisks[$i][$count]{SLOT}   = $disks{PDISKS}[$j]{SLOT};
                $count++;
            }
        }
        $dbays[$i]{PD_COUNT} = $count;
        $dbays[$i]{PDISKS} = $pdisks[$i];
    }

    my @sortBays;
    my @sortDisks;
    my $i2;
    my $j2 = -1;
    my $loc = 0;
    # Sort bays, lowest to highest.
    for ($i = 0; $i < $bays{COUNT}; ++$i)
    {
        $i2 = 8589934592;                       # 2^33 -- big high number.
        for ($j = 0; $j < $bays{COUNT}; ++$j)
        {
            if ($dbays[$j]{PD_BID} < $i2 && $dbays[$j]{PD_BID} > $j2)
            {
                $loc = $j;
                $i2 = $dbays[$j]{PD_BID};
            }
        }
        $sortBays[$i] = $dbays[$loc];           # smallest Bay ID.
        $j2 = $i2;                              # Next loop find bays greater than this.

        my $k;
        my $k1;
        my $l;
        my $l1 = -1;
        my $loc1;
        # Sort slot in bay (LUN), lowest first.
        for ($k = 0; $k < $sortBays[$i]{PD_COUNT}; ++$k)
        {
            $k1 = 8589934592;                   # 2^33 -- big high number.
            for ($l = 0; $l < $sortBays[$i]{PD_COUNT}; ++$l)
            {
                if ($sortBays[$i]{PDISKS}[$l]{SLOT} < $k1 && $sortBays[$i]{PDISKS}[$l]{SLOT} > $l1)
                {
                    $loc1 = $l;
                    $k1 = $sortBays[$i]{PDISKS}[$l]{SLOT};
                }
            }
            $pdisks[$k] = $sortBays[$i]{PDISKS}[$loc1];
            $l1 = $k1;
        }
        $sortBays[$i]{PDISKS} = [@pdisks];
    }

    # Print things below.

    my $flag;
    my %rids;
    my %vids;
    my %vids_name;
    my $pi;
    my $pj;
    my $msg;
    my $msg_warning;
    for ($i = 0; $i < $bays{COUNT}; ++$i)
    {
        $flag = 0;
        undef(%rids);
        undef(%vids);
        undef(%vids_name);
        for ($j = 0; $j < $sortBays[$i]{PD_COUNT}; ++$j)
        {
            # Find VDisks that contain this PID.
            for $pi (0..$#{$rsp{LIST}})
            {
                if ($rsp{LIST}[$pi]{PSDCNT} == 1)
                {
                    for ($pj = 0; $pj < $rsp{LIST}[$pi]{PSDCNT}; $pj++)
                    {
                        if ($rsp{LIST}[$pi]{PIDS}[$pj]{PID} == $sortBays[$i]{PDISKS}[$j]{PD_PID})
                        {
                            $vids{$rsp{LIST}[$pi]{VID}} = $sortBays[$i]{PDISKS}[$j]{SLOT};
                            $vids_name{$rsp{LIST}[$pi]{VID}} = '';
                            my $v;
                            for ($v = 0; $v < $vdisks{COUNT}; ++$v)
                            {
                                if ($vdisks{VDISKS}[$v]{VID} == $rsp{LIST}[$pi]{VID})
                                {
                                    $vids_name{$rsp{LIST}[$pi]{VID}} = $vdisks{VDISKS}[$v]{NAME};
                                    last;
                                }
                            }
                            last;
                        }
                    }
                }

            }
        }

        my @s_v = sort {$a <=> $b} keys %vids;
        undef($msg);
        $msg_warning = '';
        my @dup_luns;
        foreach my $s_v (@s_v)
        {
            if (!defined($dup_luns[$vids{$s_v}]))
            {
                $dup_luns[$vids{$s_v}] = $s_v;
            }
            else
            {
                $msg_warning .= "WARNING: ISE $sortBays[$i]{PD_BID} LUN $vids{$s_v} on VDISKS $dup_luns[$vids{$s_v}]\($vids_name{$dup_luns[$vids{$s_v}]}\) and $s_v\($vids_name{$s_v}\)\n";
            }
            if (!defined($msg))
            {
                $msg = $s_v . '(' . $vids_name{$s_v} . ')' . ':' . $vids{$s_v};
            }
            else
            {
                $msg .= ', ' . $s_v . '(' . $vids_name{$s_v} . ')' . ':' . $vids{$s_v};
            }
        }
        my $printit = 0;
        if ($ListThese != 0)
        {
            for (my $z = 0; $z < $ListThese; $z++)
            {
                if ($sortBays[$i]{PD_BID} == $Bays2List[$z])
                {
                    $printit = 1;
                    last;
                }
            }
        }
        else
        {
            $printit = 1;
        }
        if ($printit == 1)
        {
            if (defined($msg) && $msg ne '')
            {
                if ($msg_warning ne '')
                {
                    printf "%s", $msg_warning;
                }
                printf "ISE %d VDisk(NAME):LUN = %s\n", $sortBays[$i]{PD_BID}, $msg;
            }
            else
            {
                printf "Nothing for ISE %d\n", $sortBays[$i]{PD_BID};
            }
        }
    }
}   # End of singlevidsonise

##############################################################################
# Name:     vidsondatapac
#
# Desc:     VIDs On Datapac
#
##############################################################################
sub vidsondatapac()
{
    my @Bays2List;
    my @Datapacs2List;
    my $ListThese = 0;

    # Parse arguments.
    while (defined($args[$ListThese]))
    {
        if (!defined($args[$ListThese + 1]))
        {
            printf "Argument %s needs another argument of 1 or 2 for the datapac.\n", $args[$ListThese];
            return;
        }
        $Bays2List[$ListThese/2] = $args[$ListThese];
        if ($args[$ListThese+1] < 1 || $args[$ListThese+1] > 2)
        {
            printf "argument %d should be 1 or 2 for the datapac, not %d.\n", $ListThese+1+1, $args[$ListThese+1];
            return;
        }
        $Datapacs2List[$ListThese/2] = $args[$ListThese+1];
        $ListThese += 2;
    }

    # Get current diskbay information.
    my %bays = $currentMgr->diskBays();
    if (!%bays || $bays{STATUS} != PI_GOOD)
    {
        printf "Error getting bay statuses\n";
        return;
    }

    # Get current physical disk information.
    my %disks = $currentMgr->physicalDisks();
    if (!%disks || $disks{STATUS} != PI_GOOD)
    {
        printf "Error getting pdisk statuses\n";
        return;
    }

    # Get devstat raid information.
    my %rsp = $currentMgr->deviceStatus("RD");
    if (!%rsp || $rsp{STATUS} != PI_GOOD)
    {
        printf "Error getting raid device statuses\n";
        return;
    }

    my @pdisks;
    my @dbays;
    my $i;
    my $j;
    for ($i = 0; $i < $bays{COUNT}; ++$i)
    {
        $dbays[$i]{PD_BID} = $bays{BAYS}[$i]{PD_BID};
        my $count = 0;
        for ($j = 0; $j < $disks{COUNT}; ++$j)
        {
            if ($bays{BAYS}[$i]{PD_BID} == $disks{PDISKS}[$j]{SES})
            {
                $pdisks[$i][$count]{PD_PID} = $disks{PDISKS}[$j]{PD_PID};
                $pdisks[$i][$count]{SLOT}   = $disks{PDISKS}[$j]{SLOT};
                $count++;
            }
        }
        $dbays[$i]{PD_COUNT} = $count;
        $dbays[$i]{PDISKS} = $pdisks[$i];
    }

    my @sortBays;
    my @sortDisks;
    my $i2;
    my $j2 = -1;
    my $loc = 0;
    # Sort bays, lowest to highest.
    for ($i = 0; $i < $bays{COUNT}; ++$i)
    {
        $i2 = 8589934592;                       # 2^33 -- big high number.
        for ($j = 0; $j < $bays{COUNT}; ++$j)
        {
            if ($dbays[$j]{PD_BID} < $i2 && $dbays[$j]{PD_BID} > $j2)
            {
                $loc = $j;
                $i2 = $dbays[$j]{PD_BID};
            }
        }
        $sortBays[$i] = $dbays[$loc];           # smallest Bay ID.
        $j2 = $i2;                              # Next loop find bays greater than this.

        my $k;
        my $k1;
        my $l;
        my $l1 = -1;
        my $loc1;
        # Sort slot in bay (LUN), lowest first.
        for ($k = 0; $k < $sortBays[$i]{PD_COUNT}; ++$k)
        {
            $k1 = 8589934592;                   # 2^33 -- big high number.
            for ($l = 0; $l < $sortBays[$i]{PD_COUNT}; ++$l)
            {
                if ($sortBays[$i]{PDISKS}[$l]{SLOT} < $k1 && $sortBays[$i]{PDISKS}[$l]{SLOT} > $l1)
                {
                    $loc1 = $l;
                    $k1 = $sortBays[$i]{PDISKS}[$l]{SLOT};
                }
            }
            $pdisks[$k] = $sortBays[$i]{PDISKS}[$loc1];
            $l1 = $k1;
        }
        $sortBays[$i]{PDISKS} = [@pdisks];
    }


    # Print things below.

    my $flag;
    my %rids;
    my %vids;
    my $pi;
    my $pj;
    my $msg;
    for ($i = 0; $i < $bays{COUNT}; ++$i)
    {
        my $dp;
        for ($dp = 1; $dp <= 2; $dp++)
        {
            $flag = 0;
            undef(%rids);
            undef(%vids);
            for ($j = 0; $j < $sortBays[$i]{PD_COUNT}; ++$j)
            {
                if (($sortBays[$i]{PDISKS}[$j]{SLOT} % 2) == 2-$dp)
                {
                    if ($opt_V)
                    {
                        if ($flag != 0)
                        {
                            printf "     $dp";
                        }
                        else
                        {
                            printf "ISE DP LUN PID\n";
                            printf "--- -- --- ---\n";
                            printf "%2d   $dp", $sortBays[$i]{PD_BID};
                        }
                        $flag = 1;
                        printf "%3d %4d\n", $sortBays[$i]{PDISKS}[$j]{SLOT}, $sortBays[$i]{PDISKS}[$j]{PD_PID};

                        # Find raids that contain this PID.
                        for $pi (0..$#{$rsp{LIST}})
                        {
                            for ($pj = 0; $pj < $rsp{LIST}[$pi]{PSDCNT}; $pj++)
                            {
                                if ($rsp{LIST}[$pi]{PIDS}[$pj]{PID} == $sortBays[$i]{PDISKS}[$j]{PD_PID})
                                {
                                    $rids{$rsp{LIST}[$pi]{RID}} = 0;
                                    last;
                                }
                            }
                        }
                    }

                    # Find VDisks that contain this PID.
                    for $pi (0..$#{$rsp{LIST}})
                    {
                        for ($pj = 0; $pj < $rsp{LIST}[$pi]{PSDCNT}; $pj++)
                        {
                            if ($rsp{LIST}[$pi]{PIDS}[$pj]{PID} == $sortBays[$i]{PDISKS}[$j]{PD_PID})
                            {
                                $vids{$rsp{LIST}[$pi]{VID}} = 0;
                                last;
                            }
                        }
                    }
                }
            }
            if ($opt_V)
            {
                $msg = join(',', sort {$a <=> $b} keys %rids);
                if ($msg ne '')
                {
                    printf "ISE %d  DP $dp   Raids:  %s\n", $sortBays[$i]{PD_BID}, $msg;
                }
            }
            $msg = join(',', sort {$a <=> $b} keys %vids);
            my $printit = 0;
            if ($ListThese != 0)
            {
                for (my $z = 0; $z < $ListThese/2; $z++)
                {
# printf "bay=%s Bays2List[$z]=%s dp=%d   Datapacs2List[$z]=%d\n", $sortBays[$i]{PD_BID}, $Bays2List[$z], $dp, $Datapacs2List[$z];
                    if ($sortBays[$i]{PD_BID} == $Bays2List[$z])
                    {
                        if (!defined($Datapacs2List[$z]) || $Datapacs2List[$z] == $dp)
                        {
                            $printit = 1;
                            last;
                        }
                    }
                }
            }
            else
            {
                $printit = 1;
            }
            if ($printit == 1)
            {
                if ($msg ne '')
                {
                    printf "ISE %d  DP $dp   VDisks: %s\n", $sortBays[$i]{PD_BID}, $msg;
                    if ($opt_V)
                    {
                        printf "\n";
                    }
                }
                else
                {
                    printf "Nothing for ISE %d  DP $dp\n", $sortBays[$i]{PD_BID};
                }
            }
        }
    }
}   # End of vidsondatapac

##############################################################################
# Name:     diskBayStatus
#
# Desc:     Displays disk bay status for all disk bays.
#
# Input:    None
##############################################################################
sub diskBayStatus
{
    print "\n";

    my %rsp = $currentMgr->diskBayStatus();

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable retrieve Disk Bay Status.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     diskBayDelete
#
# Desc:     Deletes a diskBay.
#
# Input:    bid   -  Device ID
##############################################################################
sub diskBayDelete
{
    my ($bid) = @args;

    if (!defined($bid))
    {
        print "Missing id\n";
        return;
    }

    my %rsp = $currentMgr->diskBayDelete($bid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Successful disk bay delete id: $bid.\n";
        }
        else
        {
            my $msg = "Unable to delete device.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
    print "\n";
}

##############################################################################
# Name:     quit
#
# Desc:     Exits the Bigfoot command line, called from QUIT or EXIT.
#
# Input:    None
##############################################################################
sub quit
{
    if ($noVerboseOutput == 0) {
        print "\n";
        print "Closing all connections...\n";
    }

    foreach my $id (sort keys %connections)
    {
        my $cmdMgr = $connections{$id}{"MGR"};
        my $ip = $connections{$id}{"IP"};

        if (defined($ip))
        {
            $cmdMgr->logout();
        }

        delete $connections{$id};
    }

    $currentConnection = -1;
    $currentIP = "";

    XIOTech::logMgr::logStop();
    SetColor($currentConnection);
}

##############################################################################
# Name:     mfgClean
#
# Desc:     Returns the controller to a known state.
#
# Input:    Option - Full clean or partial.
##############################################################################
sub mfgClean
{
    my ($option) = @args;

    print "\n";

    if (defined($opt_c) || defined($opt_f) || defined($opt_s) || defined($opt_o))
    {
        print "Please do not use lowercase options. Lower case options require a parameter to follow.\n";
        return;
    }
    if (!defined($option))
    {
        $option = "LICENSE";
    }

    if (uc($option) eq "FULL")
    {
        $option = INIT_CCB_NVRAM_TYPE_FULL;
    }
    elsif (uc($option) eq "LICENSE")
    {
        $option = INIT_CCB_NVRAM_TYPE_LICENSE;
    }
    else
    {
        print "Invalid or missing option.\n";
        return;
    }

    if ($opt_C)
    {
        # Bit 1 in the option field turned on means to not
        # clear the logs.
        $option |= 0x02;
    }

    if ($opt_F)
    {
        # Bit 2 in the option field turned on means to brute
        # force clean the drives.
        $option |= 0x04;
    }

    if ($opt_S)
    {
        # Bit 3 in the option field turned on means to send
        # a few messages to the serial port.
        $option |= 0x08;
    }

    if ($opt_O)
    {
        # Bit 4 in the option field turned on means to power
        # down controller when finished.
        $option |= 0x10;
    }

    my %rsp = $currentMgr->mfgClean($option);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to clean controller.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     reset
#
# Desc:     Resets the specified processor(s).
#
# Input:    Which processor (one of: CCB, FE, BE or ALL)
##############################################################################
sub resetProcessor
{
    my ($processor, $type) = @args;

    print "\n";

    if (!defined($processor)) {
        $processor = "CCB";
    }

    if (!defined($type)) {
        $type = 0;
    }

    if ($processor !~ /^CCB$|^ALL$/i) {
        print "\n";
        print "Invalid or missing processor identifier.\n";
        return;
    }

    if ($processor =~ /^CCB$|^ALL$/i && $type == 0)
    {
        print "Note: This action terminates your connection to the CCB.\n";
        print "      Press <^C> to exit and then restart ccbCL.\n";
        print "\n";
    }
    elsif ($processor =~ /^CCB$|^ALL$/i && $type == 1)
    {
        print "Note: This action terminates your connection to the CCB.\n";
        print "      Disconnect now and reconnect when the CCB is ready.\n";
        print "\n";
    }

    my $procNum;
    $procNum = 0 if $processor =~ /^CCB$/i;
    $procNum = 3 if $processor =~ /^ALL$/i;

    my %rsp = $currentMgr->resetProcessor($procNum, $type);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_SOCKET_ERROR)
        {
            my $msg = "Lost communications with CCB (probably resetting)";
            displayError($msg, %rsp);
        }
        elsif ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to reset the requested processor.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     assignMP
#
# Desc:     Assign the mirror partner for this controller.
#
# Inputs:   serial number of mirror partner
##############################################################################
sub assignMP
{
    my ($serialNum) = @args;

    print "\n";

    if (!defined($serialNum))
    {
        print "Missing serial number data parameter\n";
        return;
    }

    my %rsp = $currentMgr->assignMirrorPartner($serialNum);

    if (%rsp)
    {
        my $oldSN = $rsp{MIRROR_PARTNER};
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Mirror partner set to: $serialNum\n";
            print "Old mirror partner:    $oldSN\n";
        }
        else
        {
            my $msg = "Unable to set mirror partner.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     resetQlogic
#
# Desc:     reset Qlogic Cards values.
#
# Input:    TYPE        - reset type
#                           BE - reset BE
#                           FE - reset FE
#           PORT        - Port to reset
#                           0-3
#                           ANY
#                           ALL
##############################################################################
sub resetQlogic
{
    my ($type, $chan, $option) = @args;

    my $any_channels = 0xF0;
    my $all_channels = 0xFF;
    my %rsp;

    print "\n";

    if (!defined($option))
    {
        $option = RESET_QLOGIC_RESET_INITIALIZE;
    }

    if ($option =~ /^0x/i) {
        $option = oct $option;}

    if (!defined($chan))
    {
        $chan = "ALL";
    }

    if (uc($chan) eq "ALL")
    {
        $chan = $all_channels;
    }
    elsif (uc($chan) eq "ANY")
    {
        $chan = $any_channels;
    }
    elsif ($chan < 0 || $chan >= 4)
    {
        print "Invalid parameter port ($chan).\n";
        return;
    }

    if (defined($type))
    {
        if (uc($type) eq "BE")
        {
            %rsp = $currentMgr->resetQlogicBE($chan, $option);
        }
        elsif (uc($type) eq "FE")
        {
            %rsp = $currentMgr->resetQlogicFE($chan, $option);
        }
        else
        {
            print "Invalid parameter type ($type).\n";
            return;
        }

        if (%rsp)
        {
            if (uc($chan) eq $all_channels)
            {
                $chan = "ALL ports";
            }
            elsif (uc($chan) eq $any_channels)
            {
                $chan = "ANY ports";
            }
            else
            {
                $chan = "port $chan";
            }
            if ($rsp{STATUS} == PI_GOOD)
            {
                print "$chan reset on " . uc($type) . "\n";
            }
            else
            {
                my $msg = "Unable to reset $chan on " . uc($type) . "\n";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }
    else
    {
        print "Missing parameter \"type\".\n";
        return;
    }
}

##############################################################################
# Name:     loopPrimitive
#
# Desc:     Loop primitive command
#
# Input:    type        - command type
#                           BE
#                           FE
#
#           option      - loop primitive option
#                           LP_RESET_LOOP           0x0000
#                           LP_RESET_LID_PORT       0x0001
#                           LP_SID_PID_RESET        0x0002
#                           LP_INITIATE_LIP         0x0003
#                           LP_LOGIN_LID            0x0011
#                           LP_LOGIN_PID            0x0012
#                           LP_LOGOUT_LID           0x0021
#                           LP_LOGOUT_PID           0x0022
#                           LP_TARGET_RESET_LID     0x0031
#                           LP_TARGET_RESET_PID     0x0032
#                           LP_PORT_BYPASS_LID      0x0041
#                           LP_PORT_BYPASS_PID      0x0042
#                           LP_PORT_ENABLE_LID      0x0043
#                           LP_PORT_ENABLE_PID      0x0044
#                           LP_PORT_BYPASS_TEST_LID 0x00F1
#                           LP_PORT_BYPASS_TEST_PID 0x00F2
#
#           id          - pid or sid
#
#           port
#
#           lid
#
##############################################################################
sub loopPrimitive
{
    my ($type, $option, $id, $port, $lid) = @args;

    my $all_channels = 0xFF;
    my %rsp;

    print "\n";

    if (!defined($option))
    {
        print "Missing option.\n";
        return;
    }

    if (!defined($id))
    {
        print "Missing id (PID for Back End, SID for Front End).\n";
        return;
    }

    if (!defined($port))
    {
        print "Missing port.\n";
        return;
    }

    if (!defined($lid))
    {
        print "Missing lid.\n";
        return;
    }


    # Call the appropriate function based on the request type.
    if (defined($type))
    {
        if (uc($type) eq "BE")
        {
            %rsp = $currentMgr->loopPrimitiveBE($option, $id, $port, $lid);
        }
        elsif (uc($type) eq "FE")
        {
            %rsp = $currentMgr->loopPrimitiveFE($option, $id, $port, $lid);
        }
        else
        {
            print "Invalid parameter type ($type).\n";
            return;
        }

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                print "Loop primitive command successful.\n";
            }
            else
            {
                my $msg = "Unable to execute loop primitive command\n";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }
    else
    {
        print "Missing parameter \"type\".\n";
        return;
    }
}


##############################################################################
# Name:     beDevicePaths
#
# Desc:     BE device paths.
#
# Input:    type(PDISK|MISC|ENC)
#           format(BP|PA)
##############################################################################
sub beDevicePaths
{
    my ($type, $format) = @args;

    my %rsp;
    my $realType;
    my $realFormat;

    print "\n";

    if (!defined($format) && defined($type) && uc($type eq '1'))
    {
        $type = 'PDISK';
        $format = '1';
    }
    if (!defined($type))
    {
        $realType = $currentMgr->PATH_PHYSICAL_DISK;
    }
    elsif (uc($type) ne 'PDISK' &&
           uc($type) ne 'PDISKS' &&
           uc($type) ne 'MISC' &&
           uc($type) ne 'ENC' &&
           uc($type) ne 'SES')
    {
        print "Invalid parameter type ($type).\n";
        return;
    }
    else
    {
        $realType = $currentMgr->PATH_PHYSICAL_DISK   if uc($type) =~ /^PDISKS*$/i;
        $realType = $currentMgr->PATH_MISC_DEVICE     if uc($type) =~ /^MISC$/i;
        $realType = $currentMgr->PATH_ENCLOSURES      if uc($type) =~ /^ENC$|^SES$/i;
    }

    if (!defined($format))
    {
        $realFormat = $currentMgr->FORMAT_PID_PATH_ARRAY;
    }
    elsif (uc($format) ne "BP" &&
           uc($format) ne "PA" &&
           uc($format) ne "1")
    {
        print "Invalid parameter format ($format).\n";
        return;
    }
    else
    {
        $realFormat = $currentMgr->FORMAT_PID_BITPATH    if uc($format) =~ /^BP$/i;
        $realFormat = $currentMgr->FORMAT_PID_PATH_ARRAY if uc($format) =~ /^PA$/i;
        $realFormat = $currentMgr->FORMAT_PID_PATH_ARRAY if ($format eq '1');
    }

    %rsp = $currentMgr->beDevicePaths($realType, $realFormat);

    my $msg;

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            if (defined($format) && $format eq '1')
            {
                $rsp{SIZE} = 1;
            }
            $msg = $currentMgr->displayDevicePath(%rsp);
            print $msg;
        }
        else
        {
            $msg = "Error getting Device Paths\n";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
}

##############################################################################
# Name:     deviceList
#
# Desc:     show the devicelist on a channel.
#
# Input:    TYPE        - processor type
#                           BE - reset BE
#                           FE - reset FE (default)
#           PORT        - Port to display
#                           0-3 (default: 0)
##############################################################################
sub deviceList
{
    my ($type, $chan) = @args;

    my %rsp;

    print "\n";

    if (!defined($chan))
    {
        $chan = 0;
    }

    if (($chan < 0 || $chan > 4))
    {
        print "Invalid parameter port.\n";
        return;
    }

    if (!defined($type))
    {
        $type = "FE";
    }

    if ((uc($type) ne "BE") && (uc($type) ne "FE"))
    {
        print "invalid parameter \"type\".\n";
        return;
    }
    else
    {
        %rsp = $currentMgr->deviceList($type, $chan);

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                my $msg = $currentMgr->displayDeviceList(%rsp);
                print $msg;
            }
            else
            {
                my $msg = "Unable get devicelist on port $chan for device " . uc($type) . "\n";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }
}

##############################################################################
# Name:     portList
#
# Desc:     Displays a list of ports (fibre channel adapters) on the
#           front or back end
#
# Input:    None
##############################################################################
sub portList
{
    my ($type, $listType) = @args;
    my $commandCode;

    print "\n";

    if (!defined($listType))
    {
        $listType = 0;
    }

    if (!defined($type))
    {
        $type = "FE";
    }
    else
    {
        $type = uc($type);
    }

    if ($type ne "BE" && $type ne "FE")
    {
        print "invalid parameter \"type\".\n";
        return;
    }

    if ($type eq "BE")
    {
        $commandCode = PI_PROC_BE_PORT_LIST_CMD;
    }
    elsif ($type eq "FE")
    {
        $commandCode = PI_PROC_FE_PORT_LIST_CMD;
    }

    my %rsp = $currentMgr->getPortList($commandCode, $listType);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            if ($type eq "BE")
            {
                print "Back End Port List:\n";
            }
            elsif ($type eq "FE")
            {
                print "Front End Port List:\n";
            }

            my $msg = $currentMgr->displayObjectList(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve list of ports.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     rescanDevice
#
# Desc:     Rescans the devices.
#
# Input:    NONE
##############################################################################
sub rescanDevice
{
    my ($type) = @args;

    print "\n";

    if (!defined($type))
    {
        $type = "LIST";
    }

    if (uc($type) ne "LIST" && uc($type) ne "LUNS" && uc($type) ne "LOOP")
    {
        print "invalid parameter \"$type\".\n";
        return;
    }

    my %rsp = $currentMgr->rescanDevice($type);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to rescan the devices.";
            displayError($msg, %rsp);
        }
        else
        {
            print "Rescan $type Successful.\n";
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     resyncCtl
#
# Desc:     Submit a resync control request.
#
# Input:    NONE
##############################################################################
sub resyncCtl
{
    print "\n";

    my ($fc, $rid, $csn, $gid, $name) = @args;

    if (!defined($fc))
    {
        print "Invalid function code parameter.\n";
        return;
    }

    if ($csn =~ /^0x/i)
    {
        $csn = oct $csn;
    }

    if (!defined($name))
    {
        $name = "               ";
    }

    if (uc($fc) eq "DEL")
    {
        $fc = 0x00;
    }
    elsif (uc($fc) eq "NAME")
    {
        $fc = 0x01;
    }
    elsif (uc($fc) eq "SWAP")
    {
        $fc = 0x02;
    }

    my %rsp = $currentMgr->resyncCtl($fc, $rid, $csn, $gid, $name);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Failed the resync control operation.";
            displayError($msg, %rsp);
        }
        else
        {
            print "Resync Control successful.\n";
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     resyncData
#
# Desc:     Retrieves the resync data
#
# Input:    NONE
##############################################################################
sub resyncData
{
    print "\n";

    my ($display_fmt, $display_cnt) = @args;
    my $data_fmt;

    if (defined($display_fmt))
    {
        if (uc($display_fmt) eq "STD")
        {
            $data_fmt = 0;
        }
        elsif (uc($display_fmt) eq "VIDS")
        {
            $data_fmt = 0;
        }
        elsif (uc($display_fmt) eq "OCSE")
        {
            $data_fmt = 0;
        }
        elsif (uc($display_fmt) eq "DTL")
        {
            $data_fmt = 1;
        }
        elsif (uc($display_fmt) eq "TRACE")
        {
            $data_fmt = 2;
        }
         elsif (uc($display_fmt) eq "PAUSE")
        {
            $data_fmt = 3;
        }
       else
        {
            my $msg = "*** Ambiguous display format entered, defaulted to STD format ***\n\n";
            print $msg;

            $display_fmt = "STD";
            $data_fmt = 0;
        }
    }
    else
    {
        $display_fmt = "STD";
        $data_fmt = 0;
    }

    if (defined($display_cnt))
    {
        if (($display_cnt > 4096) || ($display_cnt < 0))
        {
            $display_cnt = 4096;
        }
    }
    else
    {
        $display_cnt = 4096;
    }

    my %rsp = $currentMgr->resyncData($data_fmt);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayResyncData($display_fmt, $display_cnt, %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve the resync data.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     resyncMirrors
#
# Desc:     Resyncs the mirrors
#
# Input:    NONE
##############################################################################
sub resyncMirrors
{
    print "\n";

    my ($type, $rid) = @args;

    if (!defined($type))
    {
        print "Missing type parameter\n";
        return;
    }

    if (!defined($rid))
    {
        $rid = 0;
    }

    my %rsp = $currentMgr->resyncMirrors($type, $rid);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to resync the mirrors.";
            displayError($msg, %rsp);
        }
        else
        {
            print "Resync successful.\n";
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     scrubInfo
#
# Desc:     Displays the scrubbing information.
#
# Inputs:   NONE
##############################################################################
sub scrubInfo
{
    print "\n";

    my %rsp = $currentMgr->scrubInfo();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayScrubInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve the scrubbing information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     scrubSet
#
# Desc:     Set the different scrubbing options.
#
# Inputs:   NONE
##############################################################################
sub scrubSet
{
    my ($scrub_control, $paritycontrol, $raidid) = @args;

    print "\n";

    if (!defined($scrub_control))
    {
        print "Missing scrubbing control parameter\n";
        return;
    }
#
    if (!defined($paritycontrol))
    {
        print "Missing parity control parameter\n";
        return;
    }
#
    if (uc($scrub_control) eq "ENABLE")
    {
        $scrub_control = SCRUB_ENABLE;
    }
    elsif (uc($scrub_control) eq "DISABLE")
    {
        $scrub_control = SCRUB_DISABLE;
    }
    else
    {
        print "Invalid scrubbing control option.\n";
        print "Valid options:\n";
        print "  ENABLE     = Enable scrubbing\n";
        print "  DISABLE    = Disable scrubbing\n";
        return;
    }
#
    if ($paritycontrol =~ /^0x/i)
    {
        $paritycontrol = oct $paritycontrol;
    }
    else
    {
        my $pc = 0x80000000;
        my @parts = split /\|/, $paritycontrol;

        for (my $i = 0; $i < scalar(@parts); ++$i)
        {
            my $part = $parts[$i];

            if (uc($part) eq "DEFAULT")
            {
                $pc |= SCRUB_PC_DEFAULT_MASK;
            }
            elsif (uc($part) eq "MARKED")
            {
                $pc |= SCRUB_PC_MARKED_MASK;
            }
            elsif (uc($part) eq "CORRUPT")
            {
                $pc |= SCRUB_PC_CORRUPT_MASK;
            }
            elsif (uc($part) eq "SPECIFIC")
            {
                $pc |= SCRUB_PC_SPECIFIC_MASK;
            }
            elsif (uc($part) eq "CLEARLOGS")
            {
                $pc |= SCRUB_PC_CLEARLOGS_MASK;
            }
            elsif (uc($part) eq "1PASS")
            {
                $pc |= SCRUB_PC_1PASS_MASK;
            }
            elsif (uc($part) eq "CORRECT")
            {
                $pc |= SCRUB_PC_CORRECT_MASK;
            }
            elsif (uc($part) eq "ENABLE")
            {
                $pc |= SCRUB_PC_ENABLE_MASK;
            }
            elsif (uc($part) eq "DISABLE")
            {
                $pc &= SCRUB_PC_DISABLE_MASK;
            }
            else
            {
                printf "Invalid scanning control option: %s\n", uc($part);
                return;
            }
        }
        $paritycontrol = $pc;
    }
#
    if (!defined($raidid) && ($paritycontrol & SCRUB_PC_SPECIFIC_MASK))
    {
        print "Missing raid identifier, parameter required when parity\n";
        print "control option for a specific raid is specified.\n";
        return;
    }
    elsif (!defined($raidid)) # pass a dummy value for the RID
    {
        $raidid = 0xFFFF;
    }
#
    my %rsp = $currentMgr->scrubSet($scrub_control, $paritycontrol, $raidid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayScrubInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to set the scrubbing options.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     serialNumGet
#
# Desc:     Displays one of the controllers serial numbers.
#
# Inputs:   NONE
##############################################################################
sub serialNumGet
{
    print "\n";

    my %rsp = $currentMgr->serialNumGet();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displaySerialNumbers(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve serial number.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     serialNumSet
#
# Desc:     Displays one of the controllers serial numbers.
#
# Inputs:   serial number of controller
##############################################################################
sub serialNumSet
{
    my ($serialNum, $which) = @args;

    print "\n";

    if (!defined($serialNum))
    {
        print "Missing serial number data parameter\n";
        return;
    }

    if (!defined($which))
    {
        $which = SYSTEM_SN;
    }

    my %rsp = $currentMgr->serialNumSet($serialNum, $which);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            printf "System serial number set: $serialNum\n";
        }
        else
        {
            my $msg = "Unable to set serial number.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     servers
#
# Desc:     Displays server information for all servers.
#
# Input:    None
##############################################################################
sub servers
{
    print "\n";

    my %rsp = $currentMgr->servers();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayServers(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve servers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     serverProp
#
# Desc:     Server properties
#
# Input:    SID - server identifier
#           PRI - priority
#           ATTR - attributes
##############################################################################
sub serverProp
{
    my ($sid, $pri, $attr) = @args;

    print "\n";

    if (!defined($sid) || $sid < 0)
    {
        print "Invalid or missing server identifier.\n";
        return;
    }

    if (!defined($pri))
    {
        print "Invalid or missing priority.\n";
        return;
    }

    if (!defined($attr))
    {
        print "Invalid or missing attributes.\n";
        return;
    }

    # Convert from hex if necessary
    if ($pri =~ /^0x/i) {
        $pri = oct $pri;
    }
    if ($attr =~ /^0x/i) {
        $attr = oct $attr;
    }

    my %rsp = $currentMgr->serverProperties($sid, $pri, $attr);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Properties for Server ($sid) set successfully.\n";
        }
        else
        {
            my $msg = "Unable to set server properties.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     serverAssoc
#
# Desc:     Associate a server to a virtual disk.
#
# Input:    SID - server identifier
#           LUN - Lun to use in the association
#           VID - virtual disk identifier
##############################################################################
sub serverAssoc
{
    my ($sid, $lun, $vid, $option, $dsid) = @args;

    print "\n";

    if (!defined($sid) || $sid < 0)
    {
        print "Invalid or missing server identifier.\n";
        return;
    }

    if (!defined($lun))
    {
        print "Invalid or missing LUN.\n";
        return;
    }

    if (!defined($vid) || $vid < 0)
    {
        print "Invalid or missing virtual disk identifier.\n";
        return;
    }

    if (!defined($option))
    {
        $option = 0;
    }

    if (!defined($dsid))
    {
        $dsid = 0;
    }

    if ($lun =~ /^0x/i)
    {
        $lun = oct $lun;
    }

    my %rsp = $currentMgr->serverAssociateEx($sid, $lun, $vid, $option, $dsid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Server associate completed successfully ($sid, " .
                    "$lun, $vid, $option, $dsid).\n";
        }
        else
        {
            my $msg = "Unable to associate server.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     serverCount
#
# Desc:     Displays the current count of servers.
#
# Input:    None
##############################################################################
sub serverCount
{
    print "\n";

    my %rsp = $currentMgr->getObjectCount(PI_SERVER_COUNT_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Number of servers: " . $rsp{COUNT} . "\n";
        }
        else
        {
            my $msg = "Unable to retrieve number of servers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     serverCreate
#
# Desc:     Create a server.
#
# Input:    ID of the target.
#           WWN
##############################################################################
sub serverCreate
{
    my ($tid, $owner, $wwn,$iname) = @args;

    my $msg;
    my %rsp;

    print "\n";

    if (!defined($tid) || $tid < 0)
    {
        print "Invalid or missing target identifier.\n";
        return;
    }

    if (!defined($owner))
    {
        print "Missing owner parameter.\n";
        return;
    }

    if (!defined($wwn))
    {
        print "Missing wwn parameter.\n";
        return;
    }

    my $i_name;

    if (!defined($iname))
    {
        $i_name = "";
    }
    else
    {
        $i_name = $iname;
    }

    %rsp = $currentMgr->serverCreate($tid, $owner, $wwn,$i_name);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayServerCreate(%rsp);
        }
        else
        {
            $msg = "Unable to create server.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     serverDelete
#
# Desc:     Delete a server.
#
# Input:    ID of the server.
##############################################################################
sub serverDelete
{
    my ($id, $option) = @args;
    my @sids;
    my $sid;
    my %rsplist;
    my %rsp;
    my $i;

    print "\n";

    if (!defined($id))
    {
        print "Invalid or missing server identifier.\n";
        return;
    }

    if (uc($id) eq "ALL")
    {
        %rsplist = $currentMgr->servers();

        if (%rsplist)
        {
            if ($rsplist{STATUS} == PI_GOOD)
            {
                for $i (0..$#{$rsplist{SERVERS}})
                {
                    $sids[$i] = $rsplist{SERVERS}[$i]{SID};
                }
            }
            else
            {
                my $msg = "Unable to retrieve list of servers.";
                displayError($msg, %rsplist);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }
    else
    {
        @sids = $currentMgr->rangeToList($id);
    }

    if (!defined($option))
    {
        $option = 0;
    }

    if (uc($id) eq "ALL")
    {
        print "Deleting all servers...\n";
    }

    for ($i = 0; $i < scalar(@sids); $i++)
    {
        $sid = $sids[$i];

        %rsp = $currentMgr->serverDeleteEx($sid, $option);

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                print "Server ($sid) deleted.\n";
            }
            else
            {
                my $msg = "Unable to delete server ($sid).";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }

    print "\n";
}

##############################################################################
# Name:     serverDisassoc
#
# Desc:     Disassociate a server.
#
# Input:    SID - server identifier
#           LUN - Lun to use in the association
##############################################################################
sub serverDisassoc
{
    my ($sid, $lun, $vid) = @args;

    print "\n";

    if (!defined($sid) || $sid < 0)
    {
        print "Invalid or missing server identifier.\n";
        return;
    }

    if (!defined($lun))
    {
        print "Invalid or missing LUN.\n";
        return;
    }

    if (!defined($vid))
    {
        print "Invalid or missing VID.\n";
        return;
    }

    my %rsp = $currentMgr->serverDisassociate($sid, $lun, $vid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Server ($sid) is disassociated from lun-vid ($lun-$vid).\n";
        }
        else
        {
            my $msg = "Unable to disassociate server.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     serverInfo
#
# Desc:     Displays server information.
#
# Input:    ID of the server.
##############################################################################
sub serverInfo
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing server identifier.\n";
        return;
    }

    my %rsp = $currentMgr->serverInfo($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayServerInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retreive server information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     getWorksetInfo
#
# Desc:     Displays workset information.
#
# Input:    ID of the workset.
##############################################################################
sub getWorksetInfo
{
    my ($id) = @args;

    print "\n";

    if (!defined($id))
    {
        print "Invalid or missing workset identifier.\n";
        return;
    }

    if (uc($id) eq "ALL" or uc($id) eq "0XFFFF")
    {
        $id = 65535;
    }

    my %rsp = $currentMgr->getWorksetInfo($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayWorksetInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retreive workset information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     setWorksetInfo
#
# Desc:     Set workset information.
#
# Input:    ID of the workset, workset info.
##############################################################################
sub setWorksetInfo
{
    my ($id, $name, $vBlkList, $serverList, $defaultVPort) = @args;

    print "\n";

    # Create arrays of VBlock and Server IDs from the input strings of
    # comma separated values.
    my @vBlks;                  # VBlock array
    my @servers;                # Server array

    my $clearWorksets = 0;

    if ($opt_C)
    {
        if (!defined($id) || $id < 0)
        {
            print "Invalid or missing workset identifier.\n";
            return;
        }

        $clearWorksets = 1;
    }
    else
    {
        if (!defined($id) || $id < 0)
        {
            print "Invalid or missing workset identifier.\n";
            return;
        }

        if (!defined($name))
        {
            print "Invalid or missing Workset name.\n";
            return;
        }

        if (!defined($vBlkList))
        {
            print "Invalid or missing VBlock list.\n";
            return;
        }

        if (!defined($serverList))
        {
            print "Invalid or missing server list.\n";
            return;
        }

        if (!defined($defaultVPort))
        {
            print "Invalid or missing default VPort ID.\n";
            return;
        }

        # Get the individual values from the string using split.
        @vBlks = $currentMgr->rangeToList($vBlkList);
        @servers = $currentMgr->rangeToList($serverList);
    }


    # Pass the 2 arrays above by reference
    my %rsp = $currentMgr->setWorksetInfo($id, $name, \@vBlks, \@servers,
                                          $clearWorksets, $defaultVPort);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to set workset information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     serverList
#
# Desc:     Displays a list of server identifiers.
#
# Input:    None
##############################################################################
sub serverList
{
    print "\n";

    my %rsp = $currentMgr->getObjectList(PI_SERVER_LIST_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Server List:\n";
            my $msg = $currentMgr->displayObjectList(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve list of server identifiers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     serverWwnToTargetMap
#
# Desc:     Displays a list of server WWNs and the port bit map
#           associated with each
#
# Input:    None
##############################################################################
sub serverWwnToTargetMap
{
    print "\n";

    my %rsp = $currentMgr->serverWwnToTargetMap();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayServerWwnToTargetMap(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve list of server WWNs and associated targets.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     statsCacheDevices
#
# Desc:     Displays statistics for a cache device.
#
# Input:    NONE
##############################################################################
sub statsCacheDevices
{
    my ($id) = @args;

    print "\n";

    if (!defined($id))
    {
        $id = 0xFFFF;
    }

    my %rsp = $currentMgr->statsCacheDevices($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->statsCacheDevicesDisplay($id, %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve cache devices statistics ($id).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     statsLoop
#
# Desc:     Displays statistics for a loop.
#
# Input:    TYPE    - Which statistics
#                       BE - Back end
#                       FE - Front end
#           OPTION
##############################################################################
sub statsLoop
{
    my ($type, $option) = @args;

    print "\n";

    if (!defined($type))
    {
        print "Missing type parameter.\n";
        return;
    }

    # If no option is defined, use a default.
    if (!defined($option))
    {
        $option = 0;
    }
    # Extended stats data - error counters
    elsif (uc($option) eq "EXT")
    {
        $option = 0x01;
    }
    # Invalid option default
    else
    {
        $option = 0;
    }

    if (uc($type) ne "BE" && uc($type) ne "FE")
    {
        print "Invalid type parameter.  Valid type values:\n";
        print "  BE - Back end statistics\n";
        print "  FE - Front end statistics\n";
        return;
    }

    my %rsp = $currentMgr->statsLoop($type, $option);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->statsLoopDisplay($type, %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve loop statistics ($type).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     statsPCI
#
# Desc:     Displays statistics for a PCI.
#
# Input:    TYPE - Which statistics
#                    BE - Back end
#                    FE - Front end
##############################################################################
sub statsPCI
{
    my ($type) = @args;

    print "\n";

    if (!defined($type))
    {
        $type = "ALL";
    }

    if (uc($type) ne "BE" && uc($type) ne "FE" && $type ne uc("ALL"))
    {
        print "Invalid type parameter.  Valid type values:\n";
        print "  BE - Back end statistics\n";
        print "  FE - Front end statistics\n";
        return;
    }

    my %rsp = $currentMgr->statsPCI($type);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->statsPCIDisplay($type, %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve PCI statistics ($type).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     statsProc
#
# Desc:     Displays statistics for a Proc.
#
# Input:    TYPE - Which statistics
#                    BE - Back end
#                    FE - Front end
##############################################################################
sub statsProc
{
    my ($type) = @args;

    print "\n";

    if (!defined($type))
    {
        $type = "ALL";
    }

    if (uc($type) ne "BE" && uc($type) ne "FE" && uc($type) ne "ALL")
    {
        print "Invalid type parameter.  Valid type values:\n";
        print "  BE - Back end statistics\n";
        print "  FE - Front end statistics\n";
        return;
    }

    my %rsp = $currentMgr->statsProc($type);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->statsProcDisplay($type, %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve Proc statistics ($type).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     statsServer
#
# Desc:     Displays statistics for a server.
#
# Input:    ID - Server identifier
##############################################################################
sub statsServer
{
    my ($id) = @args;

    print "\n";

    if (!defined($id))
    {
        print "Missing server identifier.\n";
        return;
    }

    my %rsp = $currentMgr->statsServer($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->statsServerDisplay(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve server statistics ($id).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     statsServers
#
# Desc:     Displays statistics for all valid servers on this controller.
#
# Input:    none
##############################################################################
sub statsServers
{
    my ($dsptype) = @args;

    print "\n";

    my %rsp = $currentMgr->statsServers();

    if (!defined($dsptype))
    {
        $dsptype = "ALL";
    }

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->statsServersDisplay(uc($dsptype), %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve servers statistics.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     statsVDisk
#
# Desc:     Displays statistics for virtual disks.
#
# Input:    NONE
##############################################################################
sub statsVDisk
{
    print "\n";

    my %rsp = $currentMgr->statsVDisk();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->statsVDiskDisplay(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve virtual disk statistics.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     Perfs
#
# Desc:     Displays statistics for hbas, virtual disks, and pdisks.
#
# Input:    NONE
##############################################################################
sub Perfs
{
    my ($display_fmt, $display_subfmt) = @args;

    if (defined($display_fmt))
    {
        if (uc($display_fmt) eq 'STD')
        {
            $display_fmt = 'ALL';
            if (defined($display_subfmt))
            {
                if (uc($display_subfmt) eq 'MBPS' || uc($display_subfmt) eq 'MB/S')
                {
                    $display_subfmt = 'MBPS'
                }
                elsif (uc($display_subfmt) eq 'IOPS' || uc($display_subfmt) eq 'IO/S')
                {
                    $display_subfmt = 'IOPS'
                }
                elsif (uc($display_subfmt) eq 'QD')
                {
                    $display_subfmt = 'QD'
                }
                else
                {
                    print "*** Ambiguous display sub-format entered, defaulted to no sub-format ***\n\n";
                    $display_subfmt = 'ALL'
                }
            }
            else
            {
                $display_subfmt = 'ALL';
            }
        }
        elsif (uc($display_fmt) eq 'HAB')
        {
            $display_fmt = 'HAB';
            $display_subfmt = 'ALL';
        }
        elsif (uc($display_fmt) eq 'VID')
        {
            $display_fmt = 'VID';
            if (defined($display_subfmt))
            {
                if (uc($display_subfmt) eq 'MBPS' || uc($display_subfmt) eq 'MB/S')
                {
                    $display_subfmt = 'MBPS'
                }
                elsif (uc($display_subfmt) eq 'IOPS' || uc($display_subfmt) eq 'IO/S')
                {
                    $display_subfmt = 'IOPS'
                }
                elsif (uc($display_subfmt) eq 'QD')
                {
                    $display_subfmt = 'QD'
                }
                else
                {
                    print "*** Ambiguous display sub-format entered, defaulted to no sub-format ***\n\n";
                    $display_subfmt = 'ALL'
                }
            }
            else
            {
                $display_subfmt = 'ALL'
            }
        }
        elsif (uc($display_fmt) eq 'PID')
        {
            $display_fmt = 'PID';
            if (defined($display_subfmt))
            {
                if (uc($display_subfmt) eq 'MBPS' || uc($display_subfmt) eq 'MB/S')
                {
                    $display_subfmt = 'MBPS'
                }
                elsif (uc($display_subfmt) eq 'IOPS' || uc($display_subfmt) eq 'IO/S')
                {
                    $display_subfmt = 'IOPS'
                }
                elsif (uc($display_subfmt) eq 'QD')
                {
                    $display_subfmt = 'QD'
                }
                else
                {
                    print "*** Ambiguous display sub-format entered, defaulted to no sub-format ***\n\n";
                    $display_subfmt = 'ALL'
                }
            }
            else
            {
                $display_subfmt = 'ALL'
            }
        }
        else
        {
            print "*** Ambiguous display format entered, defaulted to STD format ***\n\n";
            $display_fmt = 'ALL';
            $display_subfmt = 'ALL';
        }
    }
    else
    {
        $display_fmt = 'ALL';
        $display_subfmt = 'ALL';
    }
    print "\n";

    if ($display_fmt eq 'ALL' || $display_fmt eq 'HAB')
    {
      for (my $id = 0; $id < 4; $id++)
      {
          my %rsp = $currentMgr->statsHAB($id);
          if (%rsp)
          {
              if ($rsp{STATUS} == PI_GOOD)
              {
                  my $msg = sprintf " HAB#%d:", $id;
                  $msg .= $currentMgr->perfsHABDisplay(%rsp);
                  print $msg;
              }
          }
          else
          {
              print "ERROR: Did not receive a statsHAB  response packet.\n";
              logout();
          }
      }
    }

    if ($display_fmt eq 'ALL' || $display_fmt eq 'VID')
    {
        my %rsp = $currentMgr->statsVDisk();

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                my $msg = $currentMgr->perfsVDiskDisplay($display_subfmt, %rsp);
                print $msg;
            }
            else
            {
                my $msg = "Unable to retrieve virtual disk statistics.";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a statsVDisk response packet.\n";
            logout();
        }
    }

    if ($display_fmt eq 'ALL' || $display_fmt eq 'PID')
    {
        my %pdisks = $currentMgr->physicalDisks();
        my $cnt = $pdisks{COUNT};

        my $msg = sprintf    "PID      0      1      2      3      4      5      6      7      8      9";
        if ($display_subfmt eq 'ALL' || $display_subfmt eq 'MBPS')
        {
            $msg .= sprintf  "\nMB/S  ------ ------ ------ ------ ------ ------ ------ ------ ------ ------";
            for (my $i = 0; $i < $cnt; ++$i)
            {
              if ($i % 10 == 0)
              {
                 $msg .= sprintf "\n %3u: ", $i;
              }
        #      if ($pdisks{PDISKS}[$i]{PD_DEVSTAT} != DEVSTATUS_OPERATIONAL)
        #      {
        #        next;
        #      }
              if ($pdisks{PDISKS}[$i]{PD_RPS} == 0)
              {
                 $msg .= sprintf " %5ld ",  ($pdisks{PDISKS}[$i]{PD_RPS} * $pdisks{PDISKS}[$i]{PD_AVGSC} )/2048.0;
              }
              else
              {
                 $msg .= sprintf " %5.2f ",  ($pdisks{PDISKS}[$i]{PD_RPS} * $pdisks{PDISKS}[$i]{PD_AVGSC} )/2048.0;
              }
            }
        }
        if ($display_subfmt eq 'ALL' || $display_subfmt eq 'IOPS')
        {
            $msg .= sprintf "\nIOPS  ------ ------ ------ ------ ------ ------ ------ ------ ------ ------";
            for (my $i = 0; $i < $cnt; ++$i)
            {
              if ($i % 10 == 0)
              {
                 $msg .= sprintf "\n %3u: ", $i;
              }
              $msg .= sprintf " %5ld ",  $pdisks{PDISKS}[$i]{PD_RPS};
            }
        }
        if ($display_subfmt eq 'ALL' || $display_subfmt eq 'QD')
        {
            $msg .= sprintf "\nQD    ------ ------ ------ ------ ------ ------ ------ ------ ------ ------";
            for (my $i = 0; $i < $cnt; ++$i)
            {
              if ($i % 10 == 0)
              {
                 $msg .= sprintf "\n %3u: ", $i;
              }
              $msg .= sprintf " %5ld ",  $pdisks{PDISKS}[$i]{PD_QD};
            }
        }
        print $msg;
        print "\n";
    }
}

##############################################################################
# Name:     statsEnv
#
# Desc:     Display environmental statistics
#
# Input:    None
##############################################################################
sub statsEnv
{
    print "\n";

    my %rsp = $currentMgr->environmentalStatsExtended();

    my $msg;

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = $currentMgr->displayEnvironmentalStatsExtended(%rsp);
            print $msg;
        }
        else
        {
            $msg = "Unable to retrieve environmental statistics.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     envII
#
# Desc:     Display environmental statistics
#
# Input:    None
##############################################################################
sub envII
{
    print "\n";

    my %rsp = $currentMgr->piEnvIIGet();

    my $msg;

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = $currentMgr->envIIDisplay(%rsp);
            print $msg;
        }
        else
        {
            $msg = "Unable to retrieve environmental statistics.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     StatsHAB
#
# Desc:     Display HAB statistics
#
# Input:    None
##############################################################################
sub StatsHAB
{
    my ($id) = @args;

    print "\n";

    if (!defined($id))
    {
        print "Missing HAB identifier.\n";
        return;
    }

    my %rsp = $currentMgr->statsHAB($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->statsHABDisplay(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve HAB statistics.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     statsBufferBoard
#
# Desc:     Display buffer board (MicroMemory) information and statistics
#
# Input:    Command code - MRP definition
##############################################################################
sub statsBufferBoard
{
    my($commandCode) = @args;
    print "\n";

    if (!defined($commandCode))
    {
        $commandCode = 0;
    }
    my %rsp = $currentMgr->statsBufferBoard($commandCode);

    my $msg;

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = $currentMgr->displayStatsBufferBoard(%rsp);
            print $msg;
        }
        else
        {
            $msg = "Unable to retrieve buffer board statistics.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     takeSnapshot
#
# Desc:     Take a system configuration snapshot.
#
# Input:    type of snapshot
#           description
##############################################################################
sub takeSnapshot
{
    my ($type) = shift @args;

    my $description = sprintf "@args";
    $description =~ s/"//g; # strip the quotes (if any)"

    my $msg;
    my %rsp;

    print "\n";

    if (!defined($type)) {
        $type = 1;
    }

    if (!defined($description) or $description =~ /^$/) {
        $description = "No description specified.";
    }

    %rsp = $currentMgr->takeSnapshot($type, $description);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD) {
            print "Snapshot taken successfully.\n";
        }
        else {
            $msg = "Unable to take snapshot.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     loadSnapshot
#
# Desc:     Load a system configuration snapshot.
#
# Input:    directory index of snapshot
#           flags indicating which fids to reload
##############################################################################
sub loadSnapshot
{
    my ($index, $flags) = @args;

    my $msg;
    my %rsp;

    print "\n";

    if (!defined($index)) {
        print "No sequence number specified.\n";
        return;
    }

    if (!defined($flags)) {
        $flags = 0xFFFFFFFF;
    }
    else {
        my @f = split /\|/, $flags;

        my $flags = 0;
        while (my $tf = shift @f) {
            if ($tf =~ /^M$/i) {
                $flags += 0x01;
                next;
            }
            if ($tf =~ /^C$/i) {
                $flags += 0x02;
                next;
            }
            if ($tf =~ /^N$/i) {
                $flags += 0x04;
                next;
            }
            if ($tf =~ /^ALL$/i) {
                $flags = 0x7;
                last;
            }
            print "Bad flag specified.\n";
            return;
        }
    }

    %rsp = $currentMgr->loadSnapshot($index, $flags);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD) {
            print "Snapshot loaded successfully.\n";
        }
        else {
            $msg = "Unable to load snapshot.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     changeSnapshot
#
# Desc:     Change a system configuration snapshot.
#
# Input:    directory index of snapshot
#           status (READ_ONLY, DELETE etc)
#           description
##############################################################################
sub changeSnapshot
{
    my $index = shift @args;
    my $status = shift @args;
    my $description;

    if (@args) {
        $description = sprintf "@args";
        $description =~ s/"//g; # strip the quotes (if any)"
    }

    my $msg;
    my %rsp;

    print "\n";

    if (!defined($index)) {
        print "No sequence number specified.\n";
        return;
    }

    if (defined($status) and $status !~ /^NC$|^DEL$|^NORM$|^KEEP$/i) {
        print "Bad status specified.\n";
        return;
    }

    my $stat = 0;  # No Change
    if (defined($status)) {
        $stat = 2 if $status =~ /^DEL$/i;
        $stat = 3 if $status =~ /^NORM$/i;
        $stat = 5 if $status =~ /^KEEP$/i;
    }

    %rsp = $currentMgr->changeSnapshot($index, $stat, $description);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD) {
            print "Snapshot changed successfully.\n";
        }
        else {
            $msg = "Unable to change snapshot.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     readdirSnapshot
#
# Desc:     Read and display the snapshot directory.
#
# Input:    none
##############################################################################
sub readdirSnapshot
{
    my ($index) = @args;
    if (! defined($index)) {
        $index = -1; # "ALL"
    }

    my %rsp = $currentMgr->readdirSnapshot();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD) {
            $currentMgr->displaySnapshot($index, %rsp);
        }
        else {
            my $msg = "Couldn't read snapshot directory.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     targets
#
# Desc:     Displays target information for all targets.
#
# Input:    None
##############################################################################
sub targets
{
    print "\n";

    my %rsp = $currentMgr->targets();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $tmsg = $currentMgr->displayTargets(%rsp);
            print $tmsg;
        }
        else
        {
            my $msg = "Unable to retrieve targets.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     targetStatus
#
# Desc:     displays target status.
#
# Input:    None
##############################################################################
sub targetStatus
{
    my ($id) = @args;
    print "\n";

    my %rsp = $currentMgr->targetStatus($id);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to retrieve number of targets.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     targetCount
#
# Desc:     Displays the current count of targets.
#
# Input:    None
##############################################################################
sub targetCount
{
    print "\n";

    my %rsp = $currentMgr->getObjectCount(PI_TARGET_COUNT_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Number of targets: " . $rsp{COUNT} . "\n";
        }
        else
        {
            my $msg = "Unable to retrieve number of targets.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     targetInfo
#
# Desc:     Displays target information.
#
# Input:    ID of the target.
##############################################################################
sub targetInfo
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing target identifier.\n";
        return;
    }

    my %rsp = $currentMgr->targetInfo($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $timsg = $currentMgr->displayTargetInfo(%rsp);
            print $timsg;
        }
        else
        {
            my $msg = "Unable to retrieve target information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     targetList
#
# Desc:     Displays a list of target identifiers.
#
# Input:    None
##############################################################################
sub targetList
{
    print "\n";

    my %rsp = $currentMgr->getObjectList(PI_TARGET_LIST_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Target List:\n";
            my $msg = $currentMgr->displayObjectList(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve list of target identifiers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     targetMove
#
# Desc:     Move a target to another controller and/or channel.
#
# Input:    TARGET_ID           - Target Identifier
#           DEST_CONTROLLER_SN  - Destination controller serial number
#           CHANNEL             - Optional channel on destination controller
##############################################################################
sub targetMove
{
    my ($target_id, $dest_controller_sn, $channel) = @args;

    print "\n";

    if (!defined($target_id) || $target_id < 0)
    {
        print "Invalid or missing target identifier.\n";
        return;
    }

    if (!defined($dest_controller_sn) || $dest_controller_sn < 0)
    {
        print "Invalid or missing destination controller serial number.\n";
        return;
    }

    if (!defined($channel))
    {
        $channel = 0xFF;
    }

    my %rsp = $currentMgr->targetMove($target_id,
                                        $dest_controller_sn,
                                        $channel);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Target ($target_id) moved to controller ($dest_controller_sn).\n";
        }
        else
        {
            my $msg = "Unable to retrieve list of target identifiers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     targetSetProperties
#
# Desc:     Set the properties of a target.
#
# Input:    ID          - Target identifier
#           OPTION      - Option for hard or soft ID
#           LOOP_ID     - Loop ID
#           CHANNEL     - Channel number
#           OWNER       - Serial number of the controller who owns this target
#           CLUSTER     - Target cluster identifier
##############################################################################
sub targetSetProperties
{
    my ($id,
        $option,
        $loop_id,
        $channel,
        $owner,
        $cluster,
        $lock) = @args;

    print "\n";

    my %rsp;
    my $toLockOrNotToLock;
    my $theOneAndOnlyOption;

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing target identifier.\n";
        return;
    }

    %rsp = $currentMgr->targetInfo($id);

    if (!%rsp || $rsp{STATUS} != PI_GOOD)
    {
        if (%rsp)
        {
            my $msg = "Unable to retrieve the target information.";
            displayError($msg, %rsp);
            return;
        }
        else
        {
            print "ERROR: Did not receive a response packet while\n";
            print "retrieving the target information.\n";
            logout();
        }
    }

    if (uc($option) eq "HARD")
    {
        $theOneAndOnlyOption = $rsp{TGD_OPT} | 0x1;

        if (!defined($loop_id))
        {
            print "Missing loop identifier parameter.\n";
            return;
        }
    }
    elsif (uc($option) eq "SOFT")
    {
        $theOneAndOnlyOption = $rsp{TGD_OPT} & 0xFE;
        $loop_id = 0;
    }
    else
    {
        print "Invalid option parameter value ($option).\n";
        print "Valid values are:\n";
        print "  HARD - Target uses hard IDs\n";
        print "  SOFT - Target uses soft IDs\n";
        return;
    }

    if (!defined($channel))
    {
        $channel = $rsp{TGD_CHAN};
    }

    if (!defined($owner))
    {
        $owner = $rsp{TGD_OWNER};
    }

    if (!defined($cluster))
    {
        $cluster = $rsp{TGD_CLUSTER};
    }

    if (!defined($lock))
    {
        $toLockOrNotToLock = $rsp{LOCK};
    }
    elsif (uc($lock) eq "LOCK")
    {
        $toLockOrNotToLock = 8;
    }
    elsif (uc($lock) eq "UNLOCK")
    {
        $toLockOrNotToLock = 0;
    }
    else
    {
        print "Invalid lock option: ($lock).\n";
        return;
    }

    %rsp = $currentMgr->targetSetProperties($id,
                                            $channel,
                                            $theOneAndOnlyOption,
                                            $loop_id,
                                            $owner,
                                            $cluster,
                                            $toLockOrNotToLock);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Properties for target ($id) set.\n";
        }
        else
        {
            my $msg = "Unable to set target properties.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     targetResList
#
# Desc:     Displays the target resource list.
#
# Input:    TID         - Target identifier
#           LIST_TYPE   - List type (1 = SERVER, 2 = VIRTUAL DISK)
#           SID         - Starting ID
##############################################################################
sub targetResList
{
    my @listTypeString =  ("Servers with Stats",
                        "Servers",
                        "Default Servers",
                        "Unmanaged Servers",
                        "Managed Servers",
                        "VLink Servers",
                        "Logged on Servers",
                        "Active Servers",
                        "Virtual Disks",
                        "Virtual Disks with cache enabled",
                        "Virtual Disks with cache disabled",
                        "Unmapped Virtual Disks",
                        "LUN Mappings",
                        "LUN Mappings - cached VDisks only",
                        "LUN Mappings - uncached VDisks only",
                        "LUN Mappings - default",
                        "Logged on Servers - from database");

    my @listTypeString2 = ("Servers with Stats (sid / WWN format)",
                        "Servers (sid / WWN format)",
                        "Default Servers (sid / WWN format)",
                        "Unmanaged Servers (sid / WWN format)",
                        "Managed Servers (sid / WWN format)",
                        "VLink Servers (sid / WWN format)",
                        "Logged on Servers (sid / WWN format)",
                        "Active Servers (sid / WWN format)",
                        "Not implemented",
                        "Not implemented",
                        "Not implemented",
                        "Not implemented",
                        "Not implemented",
                        "Not implemented",
                        "Not implemented",
                        "Not implemented",
                        "Logged on Servers - from database (sid / WWN format)"
                        );

    my ($listType,
        $tid,
        $sid) = @args;

    print "\n";

    my %rsp;
    my $msg = "";

    if (!defined($listType))
    {
        $listType = 0;
    }

    # Convert from hex if necessary
    if ($listType =~ /^0x/i)
    {
        $listType = oct $listType;
    }

    if (!defined($tid))
    {
        $tid = 65535; # Default to return all targets
    }
    # Convert from hex if necessary
    elsif ($tid =~ /^0x/i)
    {
        $tid = oct $tid;
    }

    if (!defined($sid))
    {
        $sid = 0; # Default to starting ID of zero (gets all resources)
    }

    %rsp = $currentMgr->targetResList($tid, $listType, $sid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            if ($rsp{COUNT} > 0)
            {
                if ($listType < 32)
                {
                    print "Target Resource List: ".$listTypeString[$listType]. "\n";
                }
                else
                {
                    print "Target Resource List: ".$listTypeString2[$listType - 32]. "\n\n";
                }

                $msg = $currentMgr->displayTargetResList($listType, %rsp);
                print $msg;
            }
            else
            {
                print "There are no resources for Target Resource List: ";
                if ($listType < 32)
                {
                    print "$listTypeString[$listType]\n";
                }
                else
                {
                    print "$listTypeString2[$listType - 32]\n";
                }

            }
        }
        else
        {
            $msg = "Unable to retrieve list of target identifiers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     targetTest
#
# Desc:     Move targets from one interface to another.
#
# Input:    TARGET -        target
#           OLDIF -         old interface slot
#           NEWIF -         new interface slot
#           TESTPASSES -    number of times to test
#           TIMEINTERVAL -  time interval between passes
#           TIMEOLDNEW -    time interval between old and new interface change
#           VERBOSE -       verbose (serial console output)  0 = off,  1 = on
#
##############################################################################
sub targetTest
{
    my ($target,
        $oldIF,
        $newIF,
        $testPasses,
        $timeInterval,
        $timeOldNew,
        $verbose) = @args;

    print "\n";


# All inputs are required at this time and must be greater than 0
# verbose is the exception, it must be 1 or 0
    if (!defined($target) || $target < 0)
    {
        print "Invalid or missing target identifier.\n";
        return;
    }

    if (!defined($oldIF) || $oldIF < 0)
    {
        print "Invalid or missing oldIF.\n";
        return;
    }

    if (!defined($newIF) || $newIF < 0)
    {
        print "Invalid or missing newIF.\n";
        return;
    }

    if (!defined($testPasses) || $testPasses < 0)
    {
        print "Invalid or missing testPasses.\n";
        return;
    }

    if (!defined($timeInterval) || $timeInterval < 0)
    {
        print "Invalid or missing timeInterval.\n";
        return;
    }

    if (!defined($timeOldNew) || $timeOldNew < 0)
    {
        print "Invalid or missing timeOldNew.\n";
        return;
    }

    if (!defined($verbose) || $verbose < 0 || $verbose > 1)
    {
        print "Invalid or missing verbose.\n";
        return;
    }


    my %rsp = $currentMgr->targetTest($target,
                                      $oldIF,
                                      $newIF,
                                      $testPasses,
                                      $timeInterval,
                                      $timeOldNew,
                                      $verbose);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Target test started.\n";
        }
        else
        {
            my $msg = "Unable to start target test.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     timeout
#
# Desc:     Set timeout values.
#
# Input:    TYPE        - Timeout type
#                           CCBCL - CCB Command Line timeout value
#                           MRP - MRP Timeout value
#                           CCB - CCB Timeout value
#                           IPC - IPC Timeout value
#           VALUE       - Time in seconds for the timeout value
##############################################################################
sub timeout
{
    my ($type,
        $value) = @args;

    print "\n";

    if (!defined($value))
    {
        $value = 10;
    }

    if ($value < 0)
    {
        print "Invalid timeout value parameter, must be greater than zero.\n";
        return;
    }

    if (defined($type))
    {
        if (uc($type) eq "CCBCL")
        {
            $currentMgr->{TIMEOUT} = $value;

            print "Command line timeout set to ($value) seconds.\n";
        }
        elsif (uc($type) eq "MRP" || uc($type) eq "CCB" || uc($type) eq "IPC")
        {
            my %rsp = $currentMgr->timeoutMRP($type, $value);

            if (%rsp)
            {
                if ($rsp{STATUS} == PI_GOOD)
                {
                    print "$type global timeout set ($value).\n";
                }
                else
                {
                    my $msg = "Unable to set $type global timeout.";
                    displayError($msg, %rsp);
                }
            }
            else
            {
                print "ERROR: Did not receive a response packet.\n";
                logout();
            }
        }
        else
        {
            print "Invalid type parameter value ($type).\n";
            print "Valid values are:\n";
            print "  CCBCL - CCBCL Command Line timeout value\n";
            print "  MRP - MRP timeout value\n";
            print "  CCB - CCB timeout value\n";
            print "  IPC - IPC timeout value\n";
        }
    }
    else
    {
        printf "Current CCB timeout value: %d\n", $currentMgr->{TIMEOUT};
    }

    print "\n";
}

##############################################################################
# Name:     vcgActivateController
#
# Desc:     Activate a controller that is part of this group but is currently
#           in the inactivated state.
#
# Inputs:   controller serial number
##############################################################################
sub vcgActivateController
{
    my ($serialNumber) = @args;

    print "\n";

    if (!defined($serialNumber))
    {
        print "Missing serialNumber parameter.\n";
        return;
    }

    my %rsp = $currentMgr->vcgActivateController($serialNumber);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Controller has been activated.\n";
        }
        else
        {
            my $msg = "Unable to activate the controller.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgAddController
#
# Desc:     Add slave into a virtual controller group.
#
# Inputs:   IP address of controller
##############################################################################
sub vcgAddController
{
    my ($ipAddress) = @args;

    print "\n";

    if (!defined($ipAddress))
    {
        print "Missing address.\n";
        return;
    }

    my $obj;
    my $i;

    for ($i = 0; $i < $numNextConnection; $i++)
    {
        if (defined($connections{$i}{"IP"}) &&
            defined($connections{$i}{"MGR"}))
        {
            if ($ipAddress eq $connections{$i}{"IP"})
            {
                $obj = $connections{$i}{"MGR"};
                last;
            }
        }
    }

    if (!defined($obj))
    {
        # If the port number was passed it was done with a colon separator
        my ($ip, $port) = split /:/, $ipAddress;

        if (!defined($port))
        {
            # If a port number was not passed on the command line default to 3000
            $port = 3000;
        }

        $obj = XIOTech::cmdMgr->new(\*STDOUT);

        if (defined($obj))
        {
            if ($noVerboseOutput == 0) {
                print "\n";
                print "Attempting to establish connection to ($ip:$port)...\n";
            }

            my $rc = $obj->login($ip, $port, $noVerboseOutput);

            if ($rc)
            {
                $connections{$numNextConnection} = {"IP" => $ip,
                                                    "MGR" => $obj,
                                                    "PORT" => $port};
                $numNextConnection += 1;

                print "Login to ($ip:$port) successful, " .
                        "connection ID: $currentConnection\n";
            }
            else
            {
                print "Login to ($ip:$port) failed.  See log file for more information.\n";
                undef $obj;
            }
        }
    }

    if (!defined($obj))
    {
        print "Cannot find existing connection for controller to add ($ipAddress).\n";
        return;
    }

    my %rsp = $currentMgr->vcgAddController($obj);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Slave added to VCG.\n";
        }
        else
        {
            my $msg = "Unable to add slave to virtual controller group.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgApplyLicense
#
# Desc:     Apply license information to virtual controller group.
#
# Inputs:   virtual controller group identifier for the group
#           maximum number of controllers the group can contain.
##############################################################################
sub vcgApplyLicense
{
    my ($vcgid, $maxctrls) = @args;

    print "\n";

    if (!defined($vcgid))
    {
        print "Missing vcgid parameter.\n";
        return;
    }

    if (!defined($maxctrls))
    {
        print "Missing max controllers parameter.\n";
        return;
    }

    my %rsp = $currentMgr->vcgApplyLicense($vcgid, $maxctrls);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "License applied.\n";
        }
        else
        {
            my $msg = "Unable to apply license information to virtual controller group.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgControllerInfo
#
# Desc:     Virtual controller group controller information.
#
# Input:    virtual controller group ID
##############################################################################
sub vcgControllerInfo
{
    my ($serialNumber) = @args;

    print "\n";

    my %rsp = $currentMgr->vcgControllerInfo($serialNumber);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVCGControllerInfo(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve virtual controller group controller information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgFailController
#
# Desc:     Fail a controller that is part of this group
#
# Inputs:   controller serial number
##############################################################################
sub vcgFailController
{
    my ($serialNumber) = @args;

    print "\n";

    if (!defined($serialNumber))
    {
        print "Missing serialNumber parameter.\n";
        return;
    }

    my %rsp = $currentMgr->vcgFailController($serialNumber);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Controller has been failed.\n";
        }
        else
        {
            my $msg = "Unable to fail the controller.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgInactivateController
#
# Desc:     Inactivate a controller that is part of this group
#
# Inputs:   controller serial number
##############################################################################
sub vcgInactivateController
{
    my ($serialNumber) = @args;

    print "\n";

    if (!defined($serialNumber))
    {
        print "Missing serialNumber parameter.\n";
        return;
    }

    my %rsp = $currentMgr->vcgInactivateController($serialNumber);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Controller has been inactivated.\n";
        }
        else
        {
            my $msg = "Unable to inactivate the controller.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgInfo
#
# Desc:     Virtual controller group information.
#
# Input:    virtual controller group ID
##############################################################################
sub vcgInfo
{
    print "\n";

    my %rsp = $currentMgr->vcgInfo(0);
    my $msg;

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = $currentMgr->displayVCGInfo(%rsp);
            print $msg;
        }
        else
        {
            $msg = "Unable to retrieve virtual controller group information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgPing
#
# Desc:     Ping another controller in the virtual control group.
#
# Input:    serial number of controller to ping
##############################################################################
sub vcgPing
{
    my ($serialNumber) = @args;

    print "\n";

    my %rsp = $currentMgr->vcgPing($serialNumber);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Controller ($serialNumber) PINGED successfully.\n";
        }
        else
        {
            my $msg = "Unable to ping controller ($serialNumber).";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgMPList
#
# Desc:     Virtual controller group mirror partner list.
#
# Input:    NONE
##############################################################################
sub vcgMPList
{
    print "\n";

    my %rsp = $currentMgr->vcgMPList();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayVCGMPList(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve virtual controller group mirror partner list.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgRemoveController
#
# Desc:     Remove a controller that is part of this group.
#
# Inputs:   controller serial number
##############################################################################
sub vcgRemoveController
{
    my ($ipAddress) = @args;

    print "\n";

    if (!defined($ipAddress))
    {
        print "Missing address.\n";
        return;
    }

    # Convert IP address
    my $ipAddr =  unpack "L", $currentMgr->ip2long($ipAddress);

    my $obj;
    my $i;

    for ($i = 0; $i < $numNextConnection; $i++)
    {
        if ($ipAddress eq $connections{$i}{"IP"})
        {
            $obj = $connections{$i}{"MGR"};
            last;
        }
    }

    if (!defined($obj))
    {
        print "Cannot find existing connection for controller to remove ($ipAddress).\n";
        return;
    }

    my %rsp = $currentMgr->vcgRemoveController($obj);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Controller has been removed.\n";
        }
        else
        {
            my $msg = "Unable to remove the controller.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgShutdown
#
# Desc:     Shuts down the VCG, what else did you expect?
#
# Input:    NONE
##############################################################################
sub vcgShutdown
{
    print "\n";

    my %rsp = $currentMgr->vcgShutdown();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "VCG shutdown has commenced.  Did you remember to take " .
                    "the garbage this morning?\n";
        }
        else
        {
            my $msg = "Unable to shutdown the VCG.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     vcgDoElection
#
# Desc:     starts an election on the controller it was called
#
# Input:    none
##############################################################################
sub vcgDoElection
{
    print "\n";

    my $procNum = 0;  #dummy value

    my %rsp = $currentMgr->genericCommand("ELECTION", $procNum);

    if (%rsp)
    {
        if ($rsp{STATUS} != PI_GOOD)
        {
            my $msg = "Failed to start election.";
            displayError($msg, %rsp);
        }
        else
        {
            print "Election started successfully.\n";
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgElectionState
#
# Desc:     Displays the election state.
#
# Input:    None
##############################################################################
sub vcgElectionState
{
    print "\n";

    my %rsp = $currentMgr->vcgElectionState();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            printf "Election State: %d\n", $rsp{STATE};
            print $rsp{STATE_MSG};
            print "\n";
        }
        else
        {
            my $msg = "Unable to retrieve election state.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgUnfailController
#
# Desc:     Unfail a controller that is part of this group but is currently
#           in the failed state.
#
# Inputs:   controller serial number
##############################################################################
sub vcgUnfailController
{
    my ($serialNumber) = @args;

    print "\n";

    if (!defined($serialNumber))
    {
        print "Missing serialNumber parameter.\n";
        return;
    }

    my %rsp = $currentMgr->vcgUnfailController($serialNumber);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Controller has been unfailed.\n";
            print "If the controller is waiting to complete power-up it " .
                    "should complete its power-up automatically.\n";
            print "If the controller was already powered-up then you will " .
                    "need to run an election (vcgdoelection).\n";
        }
        else
        {
            my $msg = "Unable to unfail the controller.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     vcgValidation
#
# Desc:     Starts group redundancy validation on this group.
#
# Input:    None
##############################################################################
sub vcgValidation
{
    my $flags = 0;
    print "\n";

    if (!$opt_N && !$opt_D &&!$opt_W && !$opt_S && !$opt_E && !$opt_F &&
        !$opt_C && !$opt_B && !$opt_L && !$opt_H && !$opt_I && !$opt_A)
    {
        $flags |= $currentMgr->PI_VCG_VAL_RUN_IMMED;
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_ALL;
        print "DEFAULT = RUN ALL IMMEDIATE\n";
    }

    if ($opt_I)
    {
        $flags |= $currentMgr->PI_VCG_VAL_RUN_IMMED;
    }

    if ($opt_A)
    {
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_ALL;
    }

    if ($opt_N)
    {
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_NORMAL;
    }

    if ($opt_D)
    {
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_DAILY;
    }

    if ($opt_B)
    {
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_BACK_END;
    }

    if ($opt_W)
    {
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_HW;
    }

    if ($opt_S)
    {
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_STORAGE;
    }

    if ($opt_E)
    {
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_SERVER;
    }

    if ($opt_C)
    {
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_COMM;
    }

    if ($opt_L)
    {
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_BE_LOOP;
    }

    if ($opt_F)
    {
        $flags |= $currentMgr->PI_VCG_VAL_TYPE_SHELF_ID;
    }

    my %rsp = $currentMgr->vcgValidation($flags);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "VCG validation started.\n";
        }
        else
        {
            my $msg = "Failed to start VCG validation.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vcgValidateController
#
# Desc:     Validate a potential slave can be added to our group.
#
# Inputs:   IP address of controller
##############################################################################
sub vcgValidateController
{
    my ($ipAddress) = @args;

    print "\n";

    if (!defined($ipAddress))
    {
        print "Missing address.\n";
        return;
    }

    # Convert IP address
    my $ipAddr =  unpack "L", $currentMgr->ip2long($ipAddress);

    my $obj;
    my $i;

    for ($i = 0; $i < $numNextConnection; $i++)
    {
        if ($ipAddress eq $connections{$i}{"IP"})
        {
            $obj = $connections{$i}{"MGR"};
            last;
        }
    }

    if (!defined($obj))
    {
        print "Cannot find existing connection for controller to validate ($ipAddress).\n";
        return;
    }

    my %rsp = $currentMgr->vcgValidateController($obj);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "$ipAddress can be added to our VCG.\n";
        }
        else
        {
            my $msg = "$ipAddress failed validation check.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     vcgConfigController
#
# Desc:     Remove a controller that is part of this group.
#
# Inputs:   controller serial number
##############################################################################
sub vcgConfigController
{
    my ($ipAddress, $subnetMask, $dfltGateway, $dscId, $node, $replaceFlag) = @args;

    print "\n";

    my $subnet = unpack "L", $currentMgr->ip2long("255.255.240.0");
    my $gateway = unpack "L", $currentMgr->ip2long("10.64.128.1");
    my $nodeId = 0;
    my $replacement = 0;
    my $sno = 0;

    if (!defined($ipAddress))
    {
        print "Missing address.\n";
        return;
    }

    # Convert IP address
    my $ipAddr =  unpack "L", $currentMgr->ip2long($ipAddress);

    if (defined($subnetMask))
    {
        $subnet = unpack "L", $currentMgr->ip2long($subnetMask);
    }

    if (defined($dfltGateway))
    {
        $gateway = unpack "L", $currentMgr->ip2long($dfltGateway);
    }

    if (defined($dscId))
    {
        $sno = $dscId;
    }

    if (defined($node))
    {
        $nodeId = $node;
    }

    if (defined($replaceFlag))
    {
        $replacement = $replaceFlag;
    }

    my %rsp = $currentMgr->configController($ipAddr, $subnet, $gateway, $sno, $nodeId, $replacement);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Controller has been configured.\n";
        }
        else
        {
            my $msg = "Unable to configure the controller.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     GetBEType
#
# Desc:     Get the betype fabric or loop.
#
# Input:    None
##############################################################################
sub getBEType
{

    my %rsp = $currentMgr->GetBEType();
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayBEType(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve the BE Type";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     RegisterClientType
#
# Desc:     Register the client type for this connection/session.
#
# Input:    None
##############################################################################
sub regClientType
{
    my ($type) = @args;

    print "\n";

    if (!defined($type))
    {
        print "Missing option.\n";
        return;
    }

    my %rsp = $currentMgr->RegisterClientType($type);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayNClients(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to Register Client Type";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     GetCpuCount
#
# Desc:     Get the cpu count information.
#
# Input:    None
##############################################################################
sub getCpuCount
{

    my %rsp = $currentMgr->GetCPUCount();
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayCpuCount(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve the CPU count.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}



##############################################################################
# Name:     vdisks
#
# Desc:     Displays virtual disk information for all virtual disks.
#
# Input:    None
##############################################################################
sub vdisks
{
    my ($dsptype) = @args;

    print "\n";

    if (!defined($dsptype))
    {
        $dsptype = "STD";
    }

    my %rsp = $currentMgr->virtualDisks();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayVirtualDisks($dsptype, %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve the virtual disks.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     vdiskscache
#
# Desc:     Displays virtual disk information for all virtual disks from cache
#
# Input:    None
##############################################################################
sub vdiskscache
{
    my ($dsptype) = @args;

    print "\n";

    if (!defined($dsptype))
    {
        $dsptype = "STD";
    }

    my %rsp = $currentMgr->virtualDisksCache();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayVirtualDisks($dsptype, %rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve the virtual disks.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     vdiskControl
#
# Desc:     Control a virtual disk
#
# Input:
#
# Output:
##############################################################################
sub vdiskControl
{
    my ($op,
        $svid,
        $dvid) = @args;

    print "\n";

    if (!defined($op))
    {
        print "Invalid or missing control operation.\n";
        return;
    }
    if ($op =~ /^0x/i)
    {
        $op = oct $op;
    }

    if (!defined($svid))
    {
        print "Invalid or missing source virtual disk identifier.\n";
        return;
    }

    if (!defined($dvid))
    {
        print "Invalid or missing destination virtual disk identifier.\n";
        return;
    }

    my %rsp = $currentMgr->virtualDiskControl($op,
                                                $svid,
                                                $dvid);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Virtual disk control operation ($op) started.\n";
        }
        else
        {
            my $msg = "Unable to complete virtual disk control operation.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     quickMirrorPauseStart
#
# Desc:     start command for quick pause mirror
#
# Input:
#
# Output:
##############################################################################
sub quickMirrorPauseStart
{
    my ($count) = @args;

    print "\n";

    if (!defined($count))
    {
        print "Invalid or missing Number.\n";
        return;
    }

    my %rsp = $currentMgr->quickPauseBreakMirrorStart($count);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "quickPauseBreakMirror Start cmd done.\n";
        }
        else
        {
            my $msg = "Unable to complete quickPausebreakmirrorStart cmd.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     quickMirrorPauseSequence
#
# Desc:     sequence command for quickpausebreak mirror
#
# Input:
#
# Output:
##############################################################################
sub quickMirrorPauseSequence
{
    my ($op,
        $dvid) = @args;

    print "\n";

    if (!defined($op))
    {
        print "Invalid or missing control operation.\n";
        return;
    }

    if (!defined($dvid))
    {
        print "Invalid or missing destination virtual disk identifier.\n";
        return;
    }

    my %rsp = $currentMgr->quickPauseBreakMirrorSequence($op,
                                                         $dvid);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print " quickPauseBreakMirror sequence loaded for execution.\n";
        }
        else
        {
            my $msg = "Unable to complete quickPauseBreakMirror sequence cmd.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     quickMirrorPauseExecute
#
# Desc:     execute command for quick pause mirror
#
# Input:
#
# Output:
##############################################################################
sub quickMirrorPauseExecute
{
    my ($action) = @args;

    print "\n";

    if (!defined($action))
    {
        print "Invalid or missing action.\n";
        return;
    }

    my %rsp = $currentMgr->quickPauseBreakMirrorExecute($action);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "quickPauseBreakMirror execute cmd done.\n";
        }
        else
        {
            my $msg = "Unable to complete quickPausebreakmirrorExecute cmd.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}




##############################################################################
# Name:     batchSnapshotStart
#
# Desc:     start command for batch snapshot
#
# Input:
#
# Output:
##############################################################################
sub batchSnapshotStart
{
    my ($count) = @args;

    print "\n";

    if (!defined($count))
    {
        print "Invalid or missing Number.\n";
        return;
    }

    my %rsp = $currentMgr->batchSnapshotCmdStart($count);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Batch Snapshot Start cmd done.\n";
        }
        else
        {
            my $msg = "Unable to complete Batch Snapshot Start cmd.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     batchSnapshotSequence
#
# Desc:     sequence command for batch snapshots
#
# Input:
#
# Output:
##############################################################################
sub batchSnapshotSequence
{
    my ($svid,
        $dvid) = @args;

    print "\n";

    if (!defined($svid))
    {
        print "Invalid or missing source virtual disk identifier.\n";
        return;
    }

    if (!defined($dvid))
    {
        print "Invalid or missing destination virtual disk identifier.\n";
        return;
    }

    my %rsp = $currentMgr->batchSnapshotCmdSequence($svid,$dvid);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "  Batch snapshot sequence loaded for execution.\n";
        }
        else
        {
            my $msg = "Unable to complete batch snapshot sequence cmd.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}



##############################################################################
# Name:     batchSnapshotExecute
#
# Desc:     execute command for quick pause mirror
#
# Input:
#
# Output:
##############################################################################
sub batchSnapshotExecute
{
    my ($action) = @args;

    print "\n";

    if (!defined($action))
    {
        print "Invalid or missing action.\n";
        return;
    }

    my %rsp = $currentMgr->batchSnapshotCmdExecute($action);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            if ($action == 1)
            {
                print "Batch Snapshot execute cmd done.\n";
            }
            else
            {
                print "Batch Snapshot Cacnel cmd done.\n";
            }
        }
        else
        {
            my $msg = "Unable to complete batch snapshot execute cmd.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     vdiskCount
#
# Desc:     Displays the current count of virtual disks.
#
# Input:    None
##############################################################################
sub vdiskCount
{
    print "\n";

    my %rsp = $currentMgr->getObjectCount(PI_VDISK_COUNT_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Number of virtual disks: " . $rsp{COUNT} . "\n";
        }
        else
        {
            my $msg = "Unable to retrieve number of virtual disks.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
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
    my ($capacity,
        $physicalDisks,
        $rtype,
        $stripe,
        $depth,
        $parity,
        $maxraids,
        $threshold,
        $flags,
        $minPD) = @args;

    print "\n";

    my @pids;
    my @rangeList;
    my %raidParms;
    my $vid = undef;
    my %pdisks;
    my $cnt;
    my $usable_disks;

    if (defined($opt_v))
    {
        if (($opt_v >= 0) && ($opt_v < 4096))
        {
            $vid = $opt_v;
        }
        else
        {
            print "Invalid VID $opt_v.\n";
            return;
        }
    }

    if (!defined($capacity))
    {
        print "Invalid or missing capacity.\n";
        return;
    }
    if ($capacity =~ /^0x/i)
    {
        $capacity = oct $capacity;
    }

    if (!defined($rtype))
    {
        $rtype = RAID_NONE;
    }

    if (!defined($physicalDisks))
    {
        print "Invalid or missing physical disks array.\n";
        return;
    }
    else
    {
        if (uc($physicalDisks) eq "ALL" ||
            uc($physicalDisks) eq "FC" ||
            uc($physicalDisks) eq "SATA" ||
            uc($physicalDisks) eq "SSD" ||
            uc($physicalDisks) eq "ECONENT")
        {
            %pdisks = $currentMgr->physicalDisks();
            $cnt = $pdisks{COUNT};
            $usable_disks = 0;

            for (my $i = 0; $i < $cnt; ++$i)
            {
                if ($pdisks{PDISKS}[$i]{PD_DEVSTAT} != DEVSTATUS_OPERATIONAL)
                {
                    next;
                }

                if (uc($physicalDisks) eq "FC" &&
                    $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_FC_DISK)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "SATA" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_SATA)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "SSD" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_SSD)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "ECONENT" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_ECON_ENT)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "SAS" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_SAS)
                {
                    next;
                }

                if ($rtype == RAID_1 || $rtype == RAID_10 || $rtype == RAID_5 ||
                    ((is7000() || is4700()) && ($rtype == RAID_NONE || $rtype == RAID_0)))
                {
                    if ($pdisks{PDISKS}[$i]{PD_CLASS} == LABEL_TYPE_DATA)
                    {
                        $pids[$usable_disks] = $pdisks{PDISKS}[$i]{PD_PID};
                        ++$usable_disks;
                    }
                }
                elsif ($rtype == RAID_NONE || $rtype == RAID_0)
                {
                    if ($pdisks{PDISKS}[$i]{PD_CLASS} == LABEL_TYPE_NDATA)
                    {
                        $pids[$usable_disks] = $pdisks{PDISKS}[$i]{PD_PID};
                        ++$usable_disks;
                    }
                }
            }

            if ($usable_disks <= 0)
            {
                print "No physical disks labeled DATA. use PDISKLABEL.\n";
                return;
            }
        }
        else
        {
            @pids = $currentMgr->rangeToList($physicalDisks);
        }
    }

    %raidParms = $currentMgr->calcRaidParms(\@pids, $rtype);

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

    if (!defined($maxraids))
    {
        $maxraids = 4;
    }

    if (!defined($threshold))
    {
        $threshold = 10;
    }

    if (!defined($flags))
    {
        $flags = 0;
    }
    if ($flags =~ /^0x/i)
    {
        $flags = oct $flags;
    }

    if (!defined($minPD))
    {
        $minPD = 0;
    }

    my %rsp = $currentMgr->virtualDiskCreate($capacity,
                                                \@pids,
                                                $rtype,
                                                $stripe,
                                                $depth,
                                                $parity,
                                                $vid,
                                                $maxraids,
                                                $threshold,
                                                $flags,
                                                $minPD);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVirtualDiskCreate(%rsp);
        }
        else
        {
            my $msg = "Unable to create virtual disk.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vdiskSetPriority
#
# Desc:     Set Priorities for virtual disks
#
# Input:
#
# Output:
##############################################################################
sub vdiskSetPriority
{
    my (@list) = @args;
    my @pair;
    my @vpripairs;
    my $numvals;
    my $opt = 0;
    my $respcnt = 0;
    my $i;
    my $j = 0;
    my $n;
    my $cnt;

    print "\n";
    $numvals = $#list;
    if ($numvals == -1)
    {
        print "Invalid/Missing VID-Priority List\n";
        return;
    }

    $cnt = $numvals;
    for ($i = 0; $i <= $numvals; $i++)
    {
        @pair = $currentMgr->rangeToList($list[$i]);
    $n = $#pair + 1;
        if ($i == $numvals)
    {
        if ($n == 1)
        {
            if ($i == 0)
            {
            print "Invalid/Missing VID-Priority List\n";
            return;
            }
        $opt = $pair[0];
        if ($opt == 0 || $opt == 1)
        {
        }
        else
        {
                print "Invalid Option/VID/Priority.\n";
                return;
        }
        }
        else
        {
            if ($n == 2)
            {
            if ($pair[0] < 0 || $pair[0] > 4096)
            {
                    print "Invalid VID $pair[0].\n";
                    return;
            }
            if ($pair[1] < 0 || $pair[1] > 2)
            {
                    print "Invalid Priority $pair[1].\n";
                    return;
            }
            $vpripairs[$j] = $pair[0];
            $j = $j+1;
            $vpripairs[$j] = $pair[1];
            $j = $j+1;
            $cnt = $numvals + 1;
            }
            else
            {
                print "Invalid Option/VID/Priority.\n";
                return;
            }
        }
    }
    else
    {
        if ($n == 2)
        {
        if ($pair[0] < 0 || $pair[0] > 4096)
        {
                print "Invalid VID $pair[0].\n";
                return;
        }
        if ($pair[1] < 0 || $pair[1] > 2)
        {
                print "Invalid Priority $pair[1].\n";
                return;
        }
        $vpripairs[$j] = $pair[0];
        $j = $j+1;
        $vpripairs[$j] = $pair[1];
        $j = $j+1;
        }
        else
        {
            print "Invalid VID/Priority.\n";
            return;
        }
    }
    }
#    if ($opt == 1)
#    {
#   for ($i=$j;$i<1024;$i++)
#   {
#       $vpripairs[$i] = 0;
#   }
#   $j = 512;
#    }
#    else
#    {
    $j = $j/2;
#    }
#    print "\n@vpripairs, $opt";
    if ($opt == 1)
    {
    $respcnt = 512;
    }
    else
    {
    $respcnt = $j;
    }

    my %rsp = $currentMgr->virtualDiskSetPriority($j,$respcnt,$opt,@vpripairs);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVirtualDiskSetPriority(%rsp);
        }
        else
        {
            my $msg = "Unable to set virtual disk priorities.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vdiskPriorityEnable
#
# Desc:     Enable/Disable VDisk Priority feature
#
# Input:
#
# Output:
##############################################################################
sub vdiskPriorityEnable
{
    my ($id) = @args;

    print "\n";
    if (!defined($id))
    {
        print "Invalid/Missing Mode\n";
        return;
    }

    my $opt;

    if (uc($id) eq "ON")
    {
        $opt = 1;
    }
    else
    {
        if (uc($id) eq "OFF")
        {
            $opt = 0;
        }
        else
        {
            if (uc($id) eq "STATUS")
            {
                $opt = 2;
            }
            else
            {
                print "Invalid Mode\n";
                return;
            }
        }
    }

    my %rsp = $currentMgr->vdiskPriorityEnable($opt);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVdiskPriorityEnable(%rsp);
        }
        else
        {
            my $msg = "Unable to enable/disable VDisk Priority feature.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vdiskPRClr
#
# Desc:     Clear Vdisk persistent reserve info
#
# Input:
#
# Output:
##############################################################################
sub vdiskPRClr
{
    my ($vid) = @args;

    print "\n";
    if (!defined($vid))
    {
        print "Invalid/Missing VID\n";
        return;
    }

    my %rsp = $currentMgr->vdiskPRClr($vid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Virtual disk $vid PR cleared\n";
        }
        else
        {
            my $msg = "Unable to clear persistent reserve info.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
    print "\n";
}
##############################################################################
# Name:     vdiskPRGet
#
# Desc:     Get Vdisk persistent reserve info
#
# Input:
#
# Output:
##############################################################################
sub vdiskPRGet
{
    my ($vid) = @args;

    print "\n";
    if (!defined($vid))
    {
        print "Invalid/Missing VID\n";
        return;
    }

    my %rsp = $currentMgr->vdiskPRGet($vid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
#            $currentMgr->displayVdiskPRGet(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve persistent reserve info.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vdiskPrepare
#
# Desc:     Prepare a virtual disk
#
# Input:
#
# Output:
##############################################################################
sub vdiskPrepare
{
    my ($capacity,
        $physicalDisks,
        $rtype,
        $stripe,
        $depth,
        $parity,
        $maxraids,
        $threshold,
        $flags,
        $minPD) = @args;

    print "\n";

    my @pids;
    my %raidParms;
    my %pdisks;
    my $cnt;
    my $usable_disks;

    if (!defined($capacity))
    {
        print "Invalid or missing capacity.\n";
        return;
    }
    if ($capacity =~ /^0x/i)
    {
        $capacity = oct $capacity;
    }

    if (!defined($rtype))
    {
        $rtype = RAID_NONE;
    }

    if (!defined($physicalDisks))
    {
        print "Invalid or missing physical disks array.\n";
        return;
    }
    else
    {
        if (uc($physicalDisks) eq "ALL" ||
            uc($physicalDisks) eq "FC" ||
            uc($physicalDisks) eq "SATA" ||
            uc($physicalDisks) eq "SSD" ||
            uc($physicalDisks) eq "ECONENT")
        {
            %pdisks = $currentMgr->physicalDisks();
            $cnt = $pdisks{COUNT};
            $usable_disks = 0;

            for (my $i = 0; $i < $cnt; ++$i)
            {
                if ($pdisks{PDISKS}[$i]{PD_DEVSTAT} != DEVSTATUS_OPERATIONAL)
                {
                    next;
                }

                if (uc($physicalDisks) eq "FC" &&
                    $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_FC_DISK)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "SATA" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_SATA)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "SSD" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_SSD)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "ECONENT" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_ECON_ENT)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "SAS" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_SAS)
                {
                    next;
                }
                if ($rtype == RAID_1 || $rtype == RAID_10 || $rtype == RAID_5)
                {
                    if ($pdisks{PDISKS}[$i]{PD_CLASS} == LABEL_TYPE_DATA)
                    {
                        $pids[$usable_disks] = $pdisks{PDISKS}[$i]{PD_PID};
                        ++$usable_disks;
                    }
                }
                elsif ($rtype == RAID_NONE || $rtype == RAID_0)
                {
                    if ($pdisks{PDISKS}[$i]{PD_CLASS} == LABEL_TYPE_NDATA)
                    {
                        $pids[$usable_disks] = $pdisks{PDISKS}[$i]{PD_PID};
                        ++$usable_disks;
                    }
                }
            }

            if ($usable_disks <= 0)
            {
                print "No physical disks labeled DATA. use PDISKLABEL.\n";
                return;
            }
        }
        else
        {
            @pids = $currentMgr->rangeToList($physicalDisks);
        }
    }

    %raidParms = $currentMgr->calcRaidParms(\@pids, $rtype);

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

    if (!defined($maxraids))
    {
        $maxraids = 4;
    }

    if (!defined($threshold))
    {
        $threshold = 10;
    }

    if (!defined($flags))
    {
        $flags = 0;
    }
    if ($flags =~ /^0x/i)
    {
        $flags = oct $flags;
    }

    if (!defined($minPD))
    {
        $minPD = 0;
    }

    my %rsp = $currentMgr->virtualDiskPrepare($capacity,
                                                \@pids,
                                                $rtype,
                                                $stripe,
                                                $depth,
                                                $parity,
                                                $maxraids,
                                                $threshold,
                                                $flags,
                                                $minPD);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVirtualDiskCreate(%rsp);
        }
        else
        {
            my $msg = "Unable to prepare virtual disk.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vdiskExpand
#
# Desc:     Expand a virtual disk
#
# Input:
#
# Output:
##############################################################################
sub vdiskExpand
{
    my ($vid,
        $capacity,
        $physicalDisks,
        $rtype,
        $stripe,
        $depth,
        $parity,
        $maxraids,
        $threshold,
        $flags,
        $minPD) = @args;

    print "\n";

    my @pids;
    my %raidParms;
    my %pdisks;
    my $cnt;
    my $usable_disks;

    if (!defined($vid))
    {
        print "Invalid or missing virtual disk identifier.\n";
        return;
    }

    if (!defined($capacity))
    {
        print "Invalid or missing capacity.\n";
        return;
    }
    if ($capacity =~ /^0x/i)
    {
        $capacity = oct $capacity;
    }

    if (!defined($rtype))
    {
        $rtype = RAID_NONE;
    }

    if (!defined($physicalDisks))
    {
        print "Invalid or missing physical disks array.\n";
        return;
    }
    else
    {
        if (uc($physicalDisks) eq "ALL" ||
            uc($physicalDisks) eq "FC" ||
            uc($physicalDisks) eq "SATA" ||
            uc($physicalDisks) eq "SSD" ||
            uc($physicalDisks) eq "ECONENT")
        {
            %pdisks = $currentMgr->physicalDisks();
            $cnt = $pdisks{COUNT};
            $usable_disks = 0;

            for (my $i = 0; $i < $cnt; ++$i)
            {
                if ($pdisks{PDISKS}[$i]{PD_DEVSTAT} != DEVSTATUS_OPERATIONAL)
                {
                    next;
                }

                if (uc($physicalDisks) eq "FC" &&
                    $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_FC_DISK)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "SATA" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_SATA)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "SSD" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_SSD)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "ECONENT" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_ECON_ENT)
                {
                    next;
                }
                elsif (uc($physicalDisks) eq "SAS" &&
                        $pdisks{PDISKS}[$i]{PD_DEVTYPE} != PD_DT_SAS)
                {
                    next;
                }
                if ($rtype == RAID_1 || $rtype == RAID_10 || $rtype == RAID_5)
                {
                    if ($pdisks{PDISKS}[$i]{PD_CLASS} == LABEL_TYPE_DATA)
                    {
                        $pids[$usable_disks] = $pdisks{PDISKS}[$i]{PD_PID};
                        ++$usable_disks;
                    }
                }
                elsif ($rtype == RAID_NONE || $rtype == RAID_0)
                {
                    if ($pdisks{PDISKS}[$i]{PD_CLASS} == LABEL_TYPE_NDATA)
                    {
                        $pids[$usable_disks] = $pdisks{PDISKS}[$i]{PD_PID};
                        ++$usable_disks;
                    }
                }
            }

            if ($usable_disks <= 0)
            {
                print "No physical disks labeled DATA. use PDISKLABEL.\n";
                return;
            }
        }
        else
        {
            @pids = $currentMgr->rangeToList($physicalDisks);
        }
    }

    %raidParms = $currentMgr->calcRaidParms(\@pids, $rtype);

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

    if (!defined($maxraids))
    {
        $maxraids = 4;
    }

    if (!defined($threshold))
    {
        $threshold = 10;
    }

    if (!defined($flags))
    {
        $flags = 0;
    }
    if ($flags =~ /^0x/i)
    {
        $flags = oct $flags;
    }

    if (!defined($minPD))
    {
        $minPD = 0;
    }

    my %rsp = $currentMgr->virtualDiskExpand($vid,
                                                $capacity,
                                                \@pids,
                                                $rtype,
                                                $stripe,
                                                $depth,
                                                $parity,
                                                $maxraids,
                                                $threshold,
                                                $flags,
                                                $minPD);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVirtualDiskCreate(%rsp);
        }
        else
        {
            my $msg = "Unable to expand virtual disk.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     iSCSISetInfo
#
# Desc:     Configure iSCSI parameters for a target.
#
# Input:    ParamName   - Parameter name
#           Paramval    - Parameter value
##############################################################################
sub iSCSISetInfo
{
    my ($tid,$paramId,
        $paramVal) = @args;

    my $keyval;

    if (!defined($tid))
    {
        print "Invalid or missing target Identifier.\n";
        return;
    }

    if (!defined($paramId))
    {
        print "Invalid or missing parameter name.\n";
        return;
    }

    if (!defined($paramVal))
    {
        print "Invalid or missing parameter value.\n";
        return;
    }

    if ($paramId == 0 || $paramId == 1 || $paramId == 2)
    {
        my @tip = $currentMgr->parseIP($paramVal);
        my $l = @tip;
        if ($l != 4)
        {
            print "Invalid IP.\n";
            return;
        }
        if ($tip[0] >255 || $tip[1] >255 || $tip[2] >255 || $tip[3] >255)
        {
            print "Invalid IP.\n";
            return;
        }

        $keyval = ($tip[0]) + ($tip[1] << 8) + ($tip[2] << 16) + ($tip[3] << 24);

    }
    else
    {
        if ($paramId > 24)
        {
            print "Invalid parameter Id.\n";
            return;
        }
        $keyval = $paramVal;
    }

    my %rsp;

    %rsp = $currentMgr->iSCSISetTgtParam($tid,$paramId,$keyval);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "iSCSI parameter ($paramId) for target ($tid) configured.\n";
        }
        else
        {
            my $msg = "Unable to configure parameter.";
            displayError($msg, %rsp);
        }
   }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     iSCSIGetInfo
#
# Desc:     Get info of an iSCSI target
#
# Input:    tid   - Target Identifier
##############################################################################
sub iSCSIGetInfo
{
    my ($tid) = @args;

    if (!defined($tid))
    {
        print "Invalid or missing Target ID.\n";
        return;
    }

    my %rsp;
    my $msg;

    %rsp = $currentMgr->iSCSITgtInfo($tid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = $currentMgr->displayiSCSITgtInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to get iSCSI target Info.";
            displayError($msg, %rsp);
        }
   }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     iSCSIChapSetInfo
#
# Desc:     Configure chap user info for a target.
#
# Input:    ParamName   - Parameter name
#           Paramval    - Parameter value
##############################################################################
sub iSCSIChapSetInfo
{
    my ($opt,$tid,$sname,
        $secret1,$secret2) = @args;

    my $keyval;

    if (!defined($opt))
    {
        print "Invalid or missing option.\n";
        return;
    }

    if (!defined($tid))
    {
        print "Invalid or missing target Identifier.\n";
        return;
    }

    if (!defined($sname))
    {
        print "Invalid or missing server name.\n";
        return;
    }

    my $key1;
    my $key2;
    my $type;

    if ($opt == 0)
    {
        if (!defined($secret1))
        {
            print "Invalid or missing secret.\n";
            return;
        }
        $key1 = $secret1;
        if (!defined($secret2))
        {
            $type = 0;
            $key2 = "";
        }
        else
        {
            $type = 2;
            $key2 = $secret2;
        }
    }
    else
    {
        $key1 = "";
        $key2 = "";
        $type = 0;
    }

    my %rsp;
    %rsp = $currentMgr->iSCSISetChap($opt,$tid,$type,$sname,$key1,$key2);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "CHAP User info configured on target ($tid).\n";
        }
        else
        {
            my $msg = "Unable to configure CHAP user info.";
            displayError($msg, %rsp);
        }
   }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     iSCSIChapGetInfo
#
# Desc:     Get CHAP User info of an iSCSI target
#
# Input:    tid   - Target Identifier
##############################################################################
sub iSCSIChapGetInfo
{
    my ($tid) = @args;

    if (!defined($tid))
    {
        print "Invalid or missing Target ID.\n";
        return;
    }

    my %rsp;
    my $msg;

    %rsp = $currentMgr->iSCSIChapInfo($tid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = $currentMgr->displayiSCSIChapInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to get iSCSI target Info.";
            displayError($msg, %rsp);
        }
   }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}
##############################################################################
# Name:     dlmPathStats
#
# Desc:     Displays DLM Path stats between controllers
#
# Input:    NONE
##############################################################################
sub dlmPathStats
{
    my %rsp;
    my $msg;

    %rsp = $currentMgr->dlmPathStats();

#    if (%rsp)
#   {
#        if ($rsp{STATUS} == PI_GOOD)
#        {
#            $msg = $currentMgr->displayDLMPathStats(%rsp);
#            print $msg;
#        }
#        else
#        {
#            my $msg = "Unable to get DLM Path stats.";
#            displayError($msg, %rsp);
#        }
#  }
#   else
#   {
#       print "ERROR: Did not receive a response packet.\n";
#       logout();
#   }

    print "\n";
}


##############################################################################
# Name:     iSCSIStats
#
# Desc:     Get iSCSI Session info
#
# Input:    sid   - Session Id
##############################################################################
sub iSCSIStats
{
    my ($tid) = @args;

    if (!defined($tid))
    {
        print "Invalid or missing Target ID.\n";
        return;
    }

    my %rsp;
    my $msg;

    %rsp = $currentMgr->iSCSISessionInfo($tid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = $currentMgr->displayiSCSIStats(%rsp);
#            print "\nSession Stats:\n";
#            print "\n TID : $tid\n";
            print $msg;
        }
        else
        {
            my $msg = "Unable to get iSCSI Session Info.";
            displayError($msg, %rsp);
        }
   }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     iSCSIStatsServer
#
# Desc:     Get iSCSI Session info
#
# Input:    sid   - Session Id
##############################################################################
sub iSCSIStatsServer
{
    my ($sname) = @args;

    if (!defined($sname))
    {
        print "Invalid or missing Server Name.\n";
        return;
    }

    my %rsp;
    my $msg;

    %rsp = $currentMgr->iSCSIServerSessionInfo($sname);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = $currentMgr->displayiSCSIStats(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to get iSCSI Stats on server.";
            displayError($msg, %rsp);
        }
   }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     iddInfo
#
# Desc:     Get IDD info
#
# Input:    none
##############################################################################
sub iddInfo
{
    my %rsp;
    my $msg;

    %rsp = $currentMgr->IDDInfo();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = $currentMgr->displayIDDInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to get IDD Info.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     iSNSGetInfo
#
# Desc:     Get iSNS information from DSC.
#
# Input:    None
#
# Output:   Display the information
##############################################################################
sub iSNSGetInfo
{
    my %rsp;
    my $msg;

    %rsp = $currentMgr->iSNSInfo();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = $currentMgr->displayiSNSInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to get iSNS Info.";
            displayError($msg, %rsp);
        }
   }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     iSNSSetInfo
#
# Desc:     Configure iSNS parameters for a Controller.
#
# Input:    enable  - enable/disable isns
#           autod   - autodiscovery of isns server feature
#           nset    - number of ip, port values exist
#           ip      - ip address of the isns server
#           tcp     - tcp port of the isns
##############################################################################
sub iSNSSetInfo
{
    my ($flags, $ip1, $port1, $proto1, $ip2, $port2 , $proto2) = @args;

    my $keyval1;
    my $keyval2;
    my @tip;
    my $l;

    if (!defined($flags))
    {
        print "Invalid or missing ISNS enable option.\n";
        return;
    }
    if (defined($ip1))
    {
        @tip = $currentMgr->parseIP($ip1);
        if (!defined($port1))
        {
            $port1 = 3205;
        }
        if (!defined($proto1))
        {
            $proto1 = 0;
        }
    }
    else {
        $tip[0]  = 0;
        $tip[1]  = 0;
        $tip[2]  = 0;
        $tip[3]  = 0;
        $port1 = 0;
        $proto1 = 0;
    }
    $l = @tip;
    if ($l != 4)
    {
        print "Invalid  1 IP1.\n";
        return;
    }
    if ($tip[0] >255 || $tip[1] >255 || $tip[2] >255 || $tip[3] >255)
    {
        print "Invalid 2 IP1.\n";
        return;
    }
    $keyval1 = ($tip[0]) + ($tip[1] << 8) + ($tip[2] << 16) + ($tip[3] << 24);

    if (defined($ip2))
    {
        @tip = $currentMgr->parseIP($ip2);
        if (!defined($port2))
        {
            $port2 = 3205;
        }
        if (!defined($proto2))
        {
            $proto2 = 0;
        }
    }
    else {
        $tip[0]  = 0;
        $tip[1]  = 0;
        $tip[2]  = 0;
        $tip[3]  = 0;
        $port2 = 0;
        $proto2 = 0;
    }
    $l = @tip;
    if ($l != 4)
    {
        print "Invalid 1 IP2.\n";
        return;
    }
    if ($tip[0] >255 || $tip[1] >255 || $tip[2] >255 || $tip[3] >255)
    {
        print "Invalid 2 IP2.\n";
        return;
    }
    $keyval2 = ($tip[0]) + ($tip[1] << 8) + ($tip[2] << 16) + ($tip[3] << 24);

    my %rsp;

    %rsp = $currentMgr->iSNSSetParam($flags, $flags, $keyval1, $port1, $proto1, $keyval2, $port2, $proto2);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "iSNS parameter configured.\n";
        }
        else
        {
            my $msg = "Unable to configure parameter.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     PIGetX1Env
#
# Desc:     Get Controller and Bay Environmental Info across the PI
#           interface but using the X1 packet definition
#
# Input:
#
# Output:
##############################################################################
sub PIGetX1Env
{
    print "\n";

    my $msg;

    my %rsp = $currentMgr->piGetX1Env();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            XIOTech::cmdMgr::DisplayX1EnvironmentalInfo(%rsp);
        }
        else
        {
            $msg = "Unable to retrieve controller & environmental info.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";

}

##############################################################################
# Name:     diskReadWrite
#
# Desc:     Reads/Write from/to a disk
#
##############################################################################
sub diskReadWrite
{
    my ($type, $rw, $filename, $id, $block, $numberblocks) = @args;
    my $msg;
    my %rsp;
    my $handle;

    # Parse type
    if (!defined($type))
    {
        print "Invalid or missing type (PDISK|VDISK).\n";
        return;
    }
    if ($type =~ /^P/i)
    {
        $type = 'p';
    } elsif ($type =~ /^V/i) {
        $type = 'v';
    } else {
        print "Invalid or missing type (PDISK|VDISK).\n";
        return;
    }
    # Parse read or write
    if (!defined($rw))
    {
        print "Invalid or missing (READ|WRITE).\n";
        return;
    }
    if ($rw =~ /^R/i)
    {
        $rw = 'r';
    } elsif ($rw =~ /^W/i) {
        $rw = 'w';
    } else {
        print "Invalid or missing type (READ|WRITE).\n";
        return;
    }
    # Parse filename
    if (!defined($filename))
    {
        print "Missing filename.\n";
        return;
    }
    if ($rw eq 'w')
    {
        if (!open($handle, '<', $filename))
        {
            print "Invalid or unavailable file '$filename'.\n";
            return;
        }
    } else {
        if (!open($handle, '>', $filename))
        {
            print "Invalid file '$filename'.\n";
            return;
        }
    }

    # Parse device id
    if (!defined($id))
    {
        print "Invalid or missing physical disk or bay id.\n";
        return;
    }
    # convert from hex if hex was used
    if ($id =~ /^0x/i) {
        $id = oct $id;
    }
    # Parse block
    if (!defined($block))
    {
        print "Missing block number.\n";
        return;
    }
    if ($block =~ /^0x/i)
    {
        $block = oct $block;
    }
    # Parse numberblocks
    if (!defined($numberblocks))
    {
        print "Missing numberblocks.\n";
        return;
    }
    if ($numberblocks =~ /^0x/i)
    {
        $numberblocks = oct $numberblocks;
    }
    if ($numberblocks <= 0)
    {
        print "Bad number of blocks.\n";
        return;
    }

    $msg = "DISKREADWRITE($type $rw $filename $id $block $numberblocks)";
    %rsp = $currentMgr->logTextMessage($msg, LOG_TYPE_DEBUG);

    # Call the cmd handler
    %rsp = $currentMgr->diskReadWrite($type, $rw, $handle, $id, $block, $numberblocks);
    if (%rsp)
    {
        # If successful, format the output data
        if ($rsp{STATUS} == PI_GOOD)
        {
            $msg = "End DISKREADWRITE($type $rw $filename $id $block $numberblocks)";
            %rsp = $currentMgr->logTextMessage($msg, LOG_TYPE_DEBUG);
            my $outwc = `wc -c $filename`;
            print "Success - $type $rw $filename $id $block $numberblocks\n  bytes : $outwc";
        }
        # If failure, display sense data
        else
        {
            $msg = "Error block $rsp{BLOCK} DISKREADWRITE($type $rw $filename $id $block $numberblocks)";
            my %error_rsp = $currentMgr->logTextMessage($msg, LOG_TYPE_DEBUG);
            $msg = "Failed on block=$rsp{BLOCK}";
            displayError($msg, %rsp);
        }
    }
    else
    {
        $msg = "DISKREADWRITE Did not receive a response packet - ($type $rw $filename $id $block $numberblocks)";
        %rsp = $currentMgr->logTextMessage($msg, LOG_TYPE_DEBUG);
        print "ERROR: Did not receive a response packet.\n";
    }
    close($handle);
}   # End of diskReadWrite

##############################################################################
# Name:     PIISEStatus
#
# Desc:     Get ISE bay status using the PI packet definition.
#
##############################################################################
sub PIISEStatus
{
    print "\n";

    my $msg;

    my($bayid,$parts) = @args;
    my @bayids;
    if (defined($opt_b))
    {
        print "Please do not use lowercase option -b, use -B for brief. Lower case options require a parameter to follow.\n";
        return;
    }
    if (!defined($bayid))
    {
        $bayid = -1;                # Flag to print all ISEs.
    }
    else
    {
        if ($bayid =~ /^[0-9,-]+$/)
        {
            @bayids = $currentMgr->rangeToList($bayid);
        }
        else
        {
            $parts = $bayid;
            $bayid = -1;
        }
    }

    my %rsp = $currentMgr->piISEStatus();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            if ($bayid == -1)
            {
                XIOTech::cmdMgr::DisplayISEStatus($bayid, $parts, $opt_B, %rsp);
            }
            else
            {
                foreach $bayid (@bayids)
                {
                    XIOTech::cmdMgr::DisplayISEStatus($bayid, $parts, $opt_B, %rsp);
                }
            }
        }
        else
        {
            $msg = "Unable to retrieve ISE status.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
}

##############################################################################
# Name:     PIISEBeacon
#
# Desc:     Beacon ISE bay component.
#
##############################################################################
sub PIISEBeacon
{
    print "\n";

    my($bayid, $component, $subcomponent, $light_on_off) = @args;
    my $msg;

    if (!defined($bayid))
    {
        print "Missing ISE bay id, ".
        print "Try the diskbays command to get the bayid.\n";
        return;
    }

    if (!defined($component) || !($component =~ /^[0-7]$/))
    {
        print "Missing ISE component number (0 through 7).\n";
        return;
    }

    if (!defined($subcomponent) || !($subcomponent =~ /^[12]$/))
    {
        print "Missing ISE sub component number 1 or 2.\n";
        return;
    }
    
    if (!defined($light_on_off) || !($light_on_off =~ /^[01]$/))
    {
        print "Do you want the light off (0) or on (1) missing.\n";
        return;
    }

    my %rsp = $currentMgr->piISEBeacon($bayid, $component, $subcomponent, $light_on_off);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Successfully sent the beacon ISE command.";
        }
        else
        {
            $msg = "Unable to send ISE beacon.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
    print "\n";
}

##############################################################################
# Name:     vdiskDelete
#
# Desc:     Delete a virutal disk.
#
# Input:    ID of the virtual disk.
##############################################################################
sub vdiskDelete
{
    my ($id) = @args;
    my @vids;
    my $vid;
    my %rsplist;
    my %rsp;
    my $i;

    print "\n";

    if (!defined($id))
    {
        print "Invalid or missing virtual disk identifier.\n";
        return;
    }

    if (uc($id) eq "ALL")
    {
        %rsplist = $currentMgr->virtualDiskList();

        if (%rsplist)
        {
            if ($rsplist{STATUS} == PI_GOOD)
            {
                for $i (0..$#{$rsplist{LIST}})
                {
                    $vids[$i] = $rsplist{LIST}[$i];
                }
            }
            else
            {
                my $msg = "Unable to retrieve list of virtual disks.";
                displayError($msg, %rsplist);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }
    else
    {
        @vids = $currentMgr->rangeToList($id);
    }

    if (uc($id) eq "ALL")
    {
        print "Deleting all virtual disks...\n";
    }

    for ($i = 0; $i < scalar(@vids); $i++)
    {
        $vid = $vids[$i];

        %rsp = $currentMgr->virtualDiskDelete($vid);

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                print "Virtual disk ($vid) deleted.\n";
            }
            else
            {
                my $msg = "Unable to delete virtual disk ($vid).";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }

    print "\n";
}

##############################################################################
# Name:     vdiskInfo
#
# Desc:     Displays information for a virtual disk
#
# Input:    ID of the virtual disk.
##############################################################################
sub vdiskInfo
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing virtual disk identifier.\n";
        return;
    }

    my %rsp = $currentMgr->virtualDiskInfo($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayVirtualDiskInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve virtual disk information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vdiskBayRedundant
#
# Desc:     Checks whether the given virtual disk is Bay redundant or not
#
# Input:    ID of the virtual disk.
##############################################################################
sub vdiskBayRedundant
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing virtual disk identifier.\n";
        return;
    }

    my %rsp = $currentMgr->vdiskBayRedundant($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            if ($rsp{REDUNDANCY_STATUS} == 0x00)
            {
                my $msg = "Given VDisk is Bay Redundant.";;
                print $msg;
                return;
            }
            elsif ($rsp{REDUNDANCY_STATUS} == 0x01)
            {
                my $msg = "Given VDisk is NOT Bay Redundant.";;
                print $msg;
                return;
            }
            elsif ($rsp{REDUNDANCY_STATUS} == 0x0D)
            {
                my $msg = "Given VDisk is VLink or SNAPSHOT.";;
                print $msg;
                return;
            }
        }
        elsif ($rsp{STATUS} == PI_ERROR)
        {
            if ($rsp{ERROR_CODE} == 24)
            {
                my $msg = "Invalid Virtual ID.";;
                displayError($msg, %rsp);
            }
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     raidInit
#
# Desc:     Initialize a raid device.
#
# Input:    ID of the raid device.
##############################################################################
sub raidInit
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing raid device identifier.\n";
        return;
    }

    my %rsp = $currentMgr->virtualDiskInit($id);

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
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     raidRecover
#
# Desc:     Recover an inoperative raid device.
#
# Input:    ID of the raid device.
##############################################################################
sub raidRecover
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing raid device identifier.\n";
        return;
    }

    my %rsp = $currentMgr->raidRecover($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Raid $id recovered\n";
        }
        else
        {
            my $msg = "Unable to recover raid $id.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vdiskLBAConvert
#
# Desc:     Convert a virtual disk LBA to a physical disk LBA.
#
# Input:    Virtual disk identifier.
#           LBA to convert.
##############################################################################
sub vdiskLBAConvert
{
    my ($vid, $vdiskLBA) = @args;

    my %vinfo;
    my %raids;
    my $userstripe = 0;
    my $datastripe = 0;
    my $startingdrive = 0;
    my $stripeoffset = 0;
    my $sps = 0;
    my $spu = 0;
    my $depth = 0;
    my $psdcnt = 0;
    my $offsetinstripe = 0;
    my $parityoffset = 0;
    my $stripestartoffset = 0;

    my $pid = 0;
    my $rid = 0;
    my $lba = 0;

    my $i = 0;
    my $j = 0;
    my $idxRaid = 0;

    my $idxPDisk = 0;

    print "\n";

    if (!defined($vid) || $vid < 0)
    {
        print "Invalid or missing virtual device identifier.\n";
        return;
    }

    if ($vdiskLBA =~ /^0x/i)
    {
        $vdiskLBA = oct $vdiskLBA;
    }

    $lba = $vdiskLBA;

    %vinfo = $currentMgr->virtualDiskInfo($vid);

    if (%vinfo)
    {
        if ($vinfo{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to retrieve vdisk information.";
            displayError($msg, %vinfo);

            return;
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();

        return;
    }

    %raids = $currentMgr->raids();

    if (%raids)
    {
        if ($raids{STATUS} != PI_GOOD)
        {
            my $msg = "Unable to retrieve raids information.";
            displayError($msg, %raids);

            return;
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();

        return;
    }

    for ($i = 0; $i < $vinfo{RAIDCNT}; $i++)
    {
        $rid = $vinfo{RIDS}[$i];

        for ($j = 0; $j < $raids{COUNT}; $j++)
        {
            if ($raids{RAIDS}[$j]{RID} == $rid)
            {
                last;
            }
        }

        if ($lba > $raids{RAIDS}[$j]{CAPACITY})
        {
            $lba = $lba - $raids{RAIDS}[$j]{CAPACITY};
        }
        else
        {
            $idxRaid = $j;
            last;
        }
    }

    $sps = $raids{RAIDS}[$j]{SPS};
    $spu = $raids{RAIDS}[$j]{SPU};

    $psdcnt = $raids{RAIDS}[$j]{PSDCNT};
    if ($raids{RAIDS}[$j]{TYPE} ==1)
    {
       $depth = $psdcnt ;
    }
    else
    {
        $depth = $raids{RAIDS}[$j]{DEPTH};
    }
    $userstripe = int($vdiskLBA / $sps);
    if ($spu)
    {
        $datastripe = ($userstripe * $depth) / ($spu/$sps);
    }
    else
    {
        $datastripe = ($userstripe * $depth);
    }

    $startingdrive = $datastripe % $psdcnt;
    $stripeoffset = (int($datastripe / $psdcnt) * $sps) + ($vdiskLBA % $sps);
    $offsetinstripe = $datastripe % $depth;
    $stripestartoffset = (int($datastripe / $depth) * $depth) % $psdcnt;

    $idxPDisk = $stripestartoffset;

    if ($psdcnt % $depth)
    {
        $parityoffset = 0;
        $offsetinstripe++;
    }
    else
    {
        $parityoffset = ((int($datastripe / $psdcnt)) % 4) % $depth;

        if ($parityoffset <= $offsetinstripe)
        {
            $offsetinstripe++;
        }
    }

    printf "  vdiskLBA:       %10d (0x%8.8x)\n", $vdiskLBA, $vdiskLBA;
    printf "  idxRaid:        %10d (0x%8.8x)\n", $idxRaid, $idxRaid;
    printf "  sps:            %10d (0x%8.8x)\n", $sps, $sps;
    printf "  depth:          %10d (0x%8.8x)\n", $depth, $depth;
    printf "  psdcnt:         %10d (0x%8.8x)\n", $psdcnt, $psdcnt;
    printf "  userstripe:     %10d (0x%8.8x)\n", $userstripe, $userstripe;
    printf "  datastripe:     %10d (0x%8.8x)\n", $datastripe, $datastripe;
    printf "  startingdrive:  %10d (0x%8.8x)\n", $startingdrive, $startingdrive;
    printf "  stripeoffset:   %10d (0x%8.8x)\n", $stripeoffset, $stripeoffset;
    printf "  stripestartoff: %10d (0x%8.8x)\n", $stripestartoffset, $stripestartoffset;
    printf "  offsetinstripe: %10d (0x%8.8x)\n", $offsetinstripe, $offsetinstripe;
    printf "  parityoffset:   %10d (0x%8.8x)\n", $parityoffset, $parityoffset;
    print  "\n";
    print  "    PID      SDA\n";
    print  "    ---  ----------\n";

    my $offset = 0;
    my $pidwithinstripe = 0;

    for ($i = 0; $i < $depth; $i++)
    {
        $pid = $raids{RAIDS}[$idxRaid]{PIDS}[$idxPDisk]{PID};

        if ($pidwithinstripe == $offsetinstripe)
        {
            printf "    %3d", $raids{RAIDS}[$idxRaid]{PIDS}[$idxPDisk]{PID};
        }
        else
        {
            printf "    %3d", $raids{RAIDS}[$idxRaid]{PIDS}[$idxPDisk]{PID};
        }

        my %sosInfo = $currentMgr->getSos($pid);

        if (%sosInfo && $sosInfo{STATUS} == PI_GOOD)
        {
            for ($j = 0; $j < $sosInfo{COUNT}; $j++)
            {
                if ($sosInfo{LIST}[$j]{RID} == $rid)
                {
                    if ($pidwithinstripe++ == $offsetinstripe)
                    {
                        printf "  0x%8.8x <----- Here it is dude!\n", ($sosInfo{LIST}[$j]{SDA} + $stripeoffset + $offset);
                    }
                    else
                    {
                        printf "  0x%8.8x\n", ($sosInfo{LIST}[$j]{SDA} + $stripeoffset + $offset);
                    }
                    last;
                }
            }
        }
        else
        {
            printf "  %10s\n", "NA";
        }

        if ($idxPDisk == ($psdcnt - 1))
        {
            # Bump to the next stripe
            $idxPDisk = 0;
            $offset = $sps;
        }
        else
        {
            $idxPDisk = $idxPDisk + 1;
        }
    }

    print "\n";
}

##############################################################################
# Name:     vdiskList
#
# Desc:     Displays a list of virtual disk identifiers.
#
# Input:    None
##############################################################################
sub vdiskList
{
    print "\n";

    my %rsp = $currentMgr->getObjectList(PI_VDISK_LIST_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Virtual Disk List:\n";
            my $msg = $currentMgr->displayObjectList(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve list of virtual disk identifiers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     raids
#
# Desc:     Displays raid information for all raids.
#
# Input:    None
##############################################################################
sub raids
{
    print "\n";

    my %rsp = $currentMgr->raids();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayRaids(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve the raids.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     raidscache
#
# Desc:     Displays raid information for all raids from cache
#
# Input:    None
##############################################################################
sub raidscache
{
    print "\n";

    my %rsp = $currentMgr->raidsCache();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayRaids(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve the raids.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     raidBeacon
#
# Desc:     Beacon a raid Device.
#
# Input:    ID of the raid device.
#           DURATION of beacon
##############################################################################
sub raidBeacon
{
    my ($id, $dur) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing raid identifier.\n";
        return;
    }

    if (!defined($dur))
    {
        $dur = 10;
    }

    my %rsp = $currentMgr->virtualDiskRaidInfo($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my %rsp2 = $currentMgr->raidBeacon($dur, %rsp);

            if ($rsp2{STATUS} != PI_GOOD)
            {
                my $msg = "Unable to retrieve raid information.";
                displayError($msg, %rsp);
            }
        }
        else
        {
            my $msg = "Unable to retrieve raid information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     raidCount
#
# Desc:     Displays the current count of raid devices.
#
# Input:    None
##############################################################################
sub raidCount
{
    print "\n";

    my %rsp = $currentMgr->getObjectCount(PI_RAID_COUNT_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Number of raid devices: " . $rsp{COUNT} . "\n";
        }
        else
        {
            my $msg = "Unable to retrieve number of raid devices.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     raidList
#
# Desc:     Displays a list of raid device identifiers.
#
# Input:    None
##############################################################################
sub raidList
{
    print "\n";

    my %rsp = $currentMgr->getObjectList(PI_RAID_LIST_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Raid Device List:\n";
            my $msg = $currentMgr->displayObjectList(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve list of raid device identifiers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     raidInfo
#
# Desc:     Displays raid information.
#
# Input:    ID of the raid.
##############################################################################
sub raidInfo
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing virtual disk identifier.\n";
        return;
    }

    my %rsp = $currentMgr->virtualDiskRaidInfo($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayVirtualDiskRaidInfo(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve raid information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     rollingUpdatePhase
#
# Desc:     Execute a rolling update phase command.
#
# Input:    ControllerSN
#           phase (1 / 2)
##############################################################################
sub rollingUpdatePhase
{
    my ($sn, $phase) = @args;

    print "\n";

    if (!defined($sn))
    {
        print "Missing controller serial number.\n";
        return;
    }

    if (!defined($phase))
    {
        print "Missing phase designator.\n";
        return;
    }

    my %rsp = $currentMgr->rollingUpdatePhase($sn, $phase);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Rolling update phase $phase completed.\n";
        }
        else
        {
            my $msg = "Unable to complete Rolling update phase $phase.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vdiskSetCache
#
# Desc:     Sets the caching mode for a virtual disk.
#
# Input:    ID      = identifier of the virtual disk
#           MODE    = Caching mode
#                       0 or OFF = Caching disabled
#                       1 or ON = Caching enabled
##############################################################################
sub vdiskSetCache
{
    my ($id, $mode) = @args;
    my @vids;
    my $vid;
    my %rsplist;
    my %rsp;
    my $i;

    print "\n";

    if (!defined($id))
    {
        print "Missing virtual disk identifier.\n";
        return;
    }

    if (!defined($mode))
    {
        print "Invalid or missing mode.\n";
        return;
    }

    if (uc($mode) eq "OFF")
    {
        $mode = 0x00;
    }
    elsif (uc($mode) eq "ON")
    {
        $mode = 0x01;
    }

    if ($mode != 0x00 && $mode != 0x01)
    {
        print "Invalid mode specified, valid values are:\n";
        print "  0 or OFF = Caching disabled\n";
        print "  1 or ON = Caching enabled\n";
        return;
    }

    if (uc($id) eq "ALL")
    {
        %rsplist = $currentMgr->virtualDiskList();

        if (%rsplist)
        {
            if ($rsplist{STATUS} == PI_GOOD)
            {
                for $i (0..$#{$rsplist{LIST}})
                {
                    $vids[$i] = $rsplist{LIST}[$i];
                }
            }
            else
            {
                my $msg = "Unable to retrieve list of virtual disks.";
                displayError($msg, %rsplist);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }
    else
    {
        @vids = $currentMgr->rangeToList($id);
    }

    if (uc($id) eq "ALL")
    {
        print "Changing cache setting for all virtual disks...\n";
    }

    for ($i = 0; $i < scalar(@vids); $i++)
    {
        $vid = $vids[$i];

        %rsp = $currentMgr->virtualDiskSetCache($vid, $mode);

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                if ($mode == 0x00)
                {
                    print "Caching for virtual disk ($vid) disabled.\n";
                }
                else
                {
                    print "Caching for virtual disk ($vid) enabled.\n";
                }
            }
            else
            {
                my $msg = "Unable to set virtual disk caching.";
                displayError($msg, %rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            logout();
        }
    }

    print "\n";
}

##############################################################################
# Name:     vdiskSetAttributes
#
# Desc:     Sets the caching mode for a virtual disk.
#
# Input:    ID      = identifier of the virtual disk
#           MODE    = Caching mode
##############################################################################
sub vdiskSetAttributes
{
    my ($id, $mode) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing virtual disk identifier.\n";
        return;
    }

    if (!defined($mode))
    {
        print "Invalid or missing mode.\n";
        return;
    }

    if ($mode =~ /^0x/i)
    {
        $mode = oct $mode;
    }
    elsif ($mode eq '0')
    {
        $mode = 0;
    }
    else
    {
        print "mode must be in HEX (0x0000 - 0xFFFF).\n";
        return;
    }

    if ($mode > 0xFFFF)
    {
        print "mode must be between 0x0000 and 0xFFFF.\n";
        return;
    }

    my %rsp = $currentMgr->virtualDiskSetAttributes($id, $mode);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            printf("Virtual disk set attributes: 0x%04X Successful.\n", $mode);
        }
        else
        {
            my $msg = sprintf("Virtual disk set attributes: 0x%04X Unsuccessful.\n", $mode);
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vdiskOwner
#
# Desc:     Displays the owners of a virtual disk.
#
# Input:    ID      = identifier of the virtual disk
##############################################################################
sub vdiskOwner
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing virtual disk identifier.\n";
        return;
    }

    my %rsp = $currentMgr->virtualDiskOwner($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVirtualDiskOwner(%rsp);
        }
        else
        {
            my $msg = "Unable to set virtual disk caching.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vlinks
#
# Desc:     Displays virtual disk information for all virtual disks.
#
# Input:    None
##############################################################################
sub vlinks
{
    print "\n";

    my %rsp = $currentMgr->virtualLinks();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $msg = $currentMgr->displayVirtualLinks(%rsp);
            print $msg;
        }
        else
        {
            my $msg = "Unable to retrieve the virtual links.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vlinkCtrlCount
#
# Desc:     Displays the current count of remote controllers.
#
# Input:    None
##############################################################################
sub vlinkCtrlCount
{
    print "\n";

    my %rsp = $currentMgr->virtualLinkCtrlCount();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Number of remote controllers: " . $rsp{COUNT} . "\n";
        }
        else
        {
            my $msg = "Unable to retrieve number of remote controllers.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vlinkCtrlInfo
#
# Desc:     Displays information for a remote controller
#
# Input:    ID of the remote controller.
##############################################################################
sub vlinkCtrlInfo
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing remote controller identifier.\n";
        return;
    }

    my %rsp = $currentMgr->virtualLinkCtrlInfo($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVirtualLinkCtrlInfo(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve remote controller information.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vlinkCtrlVDisks
#
# Desc:     Displays information for a remote controllers virtual disks
#
# Input:    ID of the remote controller.
##############################################################################
sub vlinkCtrlVDisks
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing remote controller identifier.\n";
        return;
    }

    my %rsp = $currentMgr->virtualLinkCtrlVDisks($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVirtualLinkCtrlVDisks(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve remote controller virtual disks.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vlinkCreate
#
# Desc:     Create a virtual link.
#
# Input:    CONTROLLER_ID   - ordinal of the remote controller.
#           VIRTUAL_DISK_ID - ordinal of the virtual disk.
##############################################################################
sub vlinkCreate
{
    my ($cid, $vid) = @args;

    print "\n";

    my $rvid = undef;

    if (defined($opt_v))
    {
        if (($opt_v >= 0) && ($opt_v < 4096))
        {
            $rvid = $opt_v;
        }
        else
        {
            print "Invalid VID $opt_v.\n";
            return;
        }
    }

    if (!defined($cid) || $cid < 0)
    {
        print "Invalid or missing remote controller ordinal.\n";
        return;
    }

    if (!defined($vid) || $vid < 0)
    {
        print "Invalid or missing virtual disk ordinal.\n";
    }

    # First set the vlink static data area in the Proc via vlinkctrlvdisks.

    my %rsp;
    %rsp = $currentMgr->virtualLinkCtrlVDisks($cid);
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
#            $currentMgr->displayVirtualLinkCtrlVDisks(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve remote controller virtual disks.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    # Now create the vlink
    %rsp = $currentMgr->virtualLinkCreate($cid, $vid, $rvid);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVirtualLinkCreate(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve remote controller virtual disks.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vlinkInfo
#
# Desc:     Displays information for a virtual link
#
# Input:    ID of the virtual link.
##############################################################################
sub vlinkInfo
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing virtual link identifier.\n";
        return;
    }

    my %rsp = $currentMgr->virtualLinkInfo($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->displayVirtualLinkInfo(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve remote controller virtual disks.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     vlinkBreakLock
#
# Desc:     Check for Virtual Links for the input vid.  Clean them up if
#           inactive or return an error if in use.
#
# Input:    ID of the virtual disk.
##############################################################################
sub vlinkBreakLock
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing virtual disk identifier.\n";
        return;
    }

    my %rsp = $currentMgr->virtualLinkBreakLock($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "VLink Break Lock request was successful.\n";
        }
        else
        {
            my $msg = "Unable to complete VLink Break Lock request.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     VLinkDLinkInfo
#
# Desc:     Displays information for a virtual link (X1 DLink)
#
# Input:    RID of the virtual link.
##############################################################################
sub VLinkDLinkInfo
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing virtual link identifier (RID).\n";
        return;
    }

    my %rsp = $currentMgr->GetVLinkDLinkInfoGT2TB($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->DisplayX1VLinkInfo(%rsp);
        }
        else
        {
# print "Retry 32 bit version.\n";
            %rsp = $currentMgr->GetVLinkDLinkInfo($id);

            if (%rsp)
            {
                if ($rsp{STATUS} == PI_GOOD)
                {
                    $currentMgr->DisplayX1VLinkInfo(%rsp);
                }
                else
                {
                    my $msg = "Unable to retrieve VLink DLink Info.";
                    displayError($msg, %rsp);
                }
            }
            else
            {
                print "ERROR: Did not receive a response packet -- 32 bit version.\n";
                logout();
            }
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     VLinkDLockInfo
#
# Desc:     Displays information for a virtual link DLock
#
# Input:    RID of the virtual link.
##############################################################################
sub VLinkDLockInfo
{
    my ($id) = @args;

    print "\n";

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing virtual link identifier (RID).\n";
        return;
    }

    my %rsp = $currentMgr->GetVLinkDLockInfo($id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            $currentMgr->DisplayX1VLinkLockInfo(%rsp);
        }
        else
        {
            my $msg = "Unable to retrieve VLink DLock info.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     getName
#
# Desc:     Request a VLink related name from the CCB.  Names are stored
#           using the Bigfoot file system.
#
# Input:    Component FID  - VDISK, VLINKRMTCTRL, CONTROLLER, VLINKRMTVDISK
#           ID             - ID of the component
##############################################################################
sub getName
{
    my ($fid, $id) = @args;

    print "\n";

    if (!defined($fid) || $fid < 0)
    {
        print "Invalid or missing file ID.\n";
        return;
    }

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing component ID.\n";
        return;
    }

    my %rsp = $currentMgr->getVLinkRelatedName($fid, $id);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Name: $rsp{NAME}\n";
        }
        else
        {
            printf("No name record was found for FID=%d  ID=%d", $fid, $id);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}


##############################################################################
# Name:     writeName
#
# Desc:     Write a name record to the Bigfoot file system.
#
# Input:    Component FID  - VDISK, VLINKRMTCTRL, CONTROLLER, VLINKRMTVDISK
#           ID             - ID of the component
##############################################################################
sub writeName
{
#    my ($fid, $id, $name) = @args;

    my ($fid) = shift @args;
    my ($id) = shift @args;
    my $name = sprintf "@args";
    $name =~ s/"//g; # strip the quotes (if any)"

    print "\n";

    if (!defined($fid) || $fid < 0)
    {
        print "Invalid or missing file ID.\n";
        return;
    }

    if (!defined($id) || $id < 0)
    {
        print "Invalid or missing component ID.\n";
        return;
    }

    if (!defined($name))
    {
        print "Missing name.\n";
        return;
    }

    my %rsp = $currentMgr->writeVLinkRelatedName($fid, $id, $name);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "writeName completed successfully.\n";
        }
        else
        {
            my $msg = "Unable to complete writeName request.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }

    print "\n";
}

##############################################################################
# Name:     dispDir
#
# Desc:     Displays the files in a directory
#
# Input:    None
##############################################################################
sub dispDir
{
    # Get the command line pattern matching parameters.
    # Get them into a form that works for grep.
    my ($pattern) = join '$|^', @args;
    if ($pattern eq "") {
        $pattern = "*";
    }
    $pattern =~ s/\./\\./g;
    $pattern =~ s/\*/.*/g;
    $pattern = "^$pattern\$";
#    print "pattern = \"$pattern\"\n";

    # Get the list of files in this directory;
    # Filter out unwanted.
    opendir DIR, cwd();
    my @files = grep /$pattern/i, readdir DIR;
    closedir DIR;

    # Print the list of files in a form similar to DIR
    print "\n";
    my $fname;
    for my $idx (@files) {
        if (-d $idx) {
            $fname = "[$idx]";
        }
        else {
            $fname = "$idx";
        }
        my @fstat = stat $idx;
        my @time = localtime($fstat[9]);
        printf "%02d/%02d/%02d %2d:%02d:%02d %8d %s\n",
        $time[4]+1,$time[3],$time[5]-100,$time[2],$time[1],$time[0],
        $fstat[7], $fname;
    }
}

##############################################################################
# Name:     displayError
#
# Desc:     Displays an error message followed by the status and
#           error codes from a command response.
#
# Input:    message and command response hash.
##############################################################################
sub displayError($%)
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

    print "\n";
}

##############################################################################
#
# Command line processing
#
##############################################################################
sub AreYouSure
{
    if ($cmdf eq "STDIN") {
        print "\nThis is a VERY, VERY DANGEROUS COMMAND.\n";
        print "Are you sure you want to proceed? [N]/Y ";

        my $ans = <$cmdf>;

        if ($ans =~ /^Y/i) {
            print "\nThis could be CATASTROPHIC, are you REALLY sure\n";
            print "that you want to execute this command [N]/Y ";

            $ans = <$cmdf>;

            if ($ans =~ /^Y/i) {
                print "\nYou Da Man! Let's ROLL!!!\n";
                return 1;
            }
        }

        print "\nWhat a wimp. No Guts, No Glory...\n";
        return 0;
    }

    return 1;
}

#-----------------------------------------------------------------------------
# Start of main routine.

if ($noVerboseOutput == 0) {
# initial prompt
    print "\n";
    print "============================================\n";
    print "Welcome to the CCB command line interface!  \n";
    print "Copyright 2000-2003 XIOtech Corporation     \n";
    print "        For Internal Use Only               \n";
    print "           ($version)                  \n";
    print "============================================\n";
}

# connect to initial system if requested on the command line
my $connNum = @connectTo;
while (@connectTo)
{
    @args = shift @connectTo;
    login();
}
if ($connNum and $currentConnection == -1)
{
    print "\nAll connection attempts failed, exiting.\n\n";
    exit;
}
if ($connNum > 1) {
    @args = (0);
    chgConn();
}

displayPrompt();

# loop on STDIN
my @multiCmd = ();
my $oneLastTime = 0;
CMD: while (1)
{
    my $localDebug = 0;

    if ($redirectOpen == 1)
    {
        $redirectOpen = 0;
        close(RFH);
        select(STDOUT);
        displayPrompt();
    }

    # Handle multiple commands per line, seperated by ';'s
    if (@multiCmd)
    {
        $_ = shift @multiCmd;

        if ($redirectOpen)
        {
            $redirectOpen--;
        }
    }
    else
    {
        if ($useReadLine == 0 or $windoze or $cmdf ne "STDIN" or !defined($term))
        {
            $_ = <$cmdf>;
        }
        else  # use 'readline' library on Linux
        {
            $_ = $term->readline(getPrompt());
            if (defined($_))
            {
                $_ =~ s/\s*$//; # remove any trailing spaces
                if (($_ ne "") and (uc($_) ne uc($lastHCmd)))
                {
                    if ($useReadLine == 1)
                    {
                        $term->append_history(1, $historyFile);
                    }
                    $lastHCmd = $_;
                }

                # Substitute $HOME anywhere we see '~'. Of course then we can't do
                # anything with files with a '~' in them, but who cares?
                $_ =~ s/\~/$ENV{HOME}/g;
            }
        }

        if (!defined($_))
        {
            if ($noVerboseOutput == 0) {
                print "\n";
            }
            last CMD;
        }

        # Check if this command is redirected
        if (/>{1,2}/)
        {
            my $redirectFileName = $';
            chomp $redirectFileName;
            $_ = $`;
            if (open RFH, "$& $redirectFileName")
            {
                select(RFH);
                $redirectOpen = 1;
                displayPrompt();
            }
            else
            {
                print "\nCan't open $redirectFileName for output!\n";
            }
        }

        # Make sure that the ':' that is matched is NOT followed by a
        # '/' or '\', which would indicate a filename ("c:/foo").
        if (/;|:[^\/\\]/)
        {
            @multiCmd = split /\s{0,};\s{0,}/;
            print STDERR ">> 1 multiCmd: @multiCmd <<\n" if $localDebug;

            # Each command can have a list of controllers to perform
            # the command on: "1,2:vcginfo" or "A:vcginfo" (all).
            # Push the contoller changes onto the multiCmd array as commands.
            my $i;
            my @multiCmd2 = ();
            my @multiCmdTmp = ();
            for ($i=0; $i<@multiCmd; $i++)
            {
                print STDERR ">> processing multiCmd[$i] <<\n" if $localDebug;

                # ip:port are exceptions to the rule; don't break them apart.
                if ($multiCmd[$i] =~ /:[^\/\\]/ and
                    ($multiCmd[$i] !~ /\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}:\d{1,5}?$/))
                {
                    $multiCmd[$i] =~ m/:/;
                    my $pre_m = $`;
                    my $post_m = $';
                    print "pre \"$pre_m\"  post \"$post_m\"\n" if $localDebug;

                    # Another exception: if the command is "CD" (chdir),
                    # let it go through unaltered.
                    if ($pre_m =~ /^cd\s/i)
                    {
                        print "Leaving alone...\n" if $localDebug;
                        goto LEAVE_ALONE;  # Arrrgh!
                    }

                    # iSCSI Names in servercreate and iscsi commands can have ":"
                    if ($pre_m =~ /^iscsi*/i)
                    {
                        print "Leaving alone...\n" if $localDebug;
                        goto LEAVE_ALONE;  # Arrrgh!
                    }
                    if ($pre_m =~ /^servercr*/i)
                    {
                        print "Leaving alone...\n" if $localDebug;
                        goto LEAVE_ALONE;  # Arrrgh!
                    }


                    @multiCmdTmp = $currentMgr->rangeToList($pre_m);

                    try_again:
                    my $j;
                    for ($j=0; $j<@multiCmdTmp; $j++)
                    {
                        if ($multiCmdTmp[$j] =~ /^\s*[A0-9]/i)
                        {
                            if ($multiCmdTmp[$j] =~ /^\s*A/i)
                            {
                                shift @multiCmdTmp; # pop off the 'A'
                                $j--;
                                my $k;
                                foreach $k (reverse(sort(keys(%connections))))
                                {
                                    unshift @multiCmdTmp, $k;
                                    print STDERR ">> multiCmdTmp: @multiCmdTmp <<\n"
                                        if $localDebug;
                                }

                                goto try_again;
                            }
                        }
                        else
                        {
                            @multiCmd = ();
                            print "\nUnknown connection ID -- try again.\n";
                            displayPrompt();
                            next CMD;
                        }

                        push  @multiCmd2, $multiCmdTmp[$j];
                        my $myCmd = $post_m;
                        chomp $myCmd;
                        push  @multiCmd2, $myCmd;
                        print STDERR ">> multiCmd2: @multiCmd2 <<\n" if $localDebug;
                    }
                }
                else
                {
                    LEAVE_ALONE:
                    push  @multiCmd2, $multiCmd[$i];
                }
            }

            @multiCmd = @multiCmd2;

            # Always return to where you started from unless the final command
            # is a connection request.
            if ($multiCmd[scalar(@multiCmd)-1] !~
                    /^co.*\s+\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}/i)
            {
                push @multiCmd, $currentConnection if $currentConnection >= 0;
            }

            if ($redirectOpen)
            {
                $redirectOpen = @multiCmd + 1;
            }

            print STDERR ">> 2 multiCmd: @multiCmd <<\n" if $localDebug;

            displayPrompt();
            next CMD;
        }
    }

    # Change all '\'s to '/'s right away.  There should never be a need
    # to explicitly enter a '\' anyway, so get rid of them as they tend
    # to cause problems.
    # OK I found a reason to leave them -- if the line is being passed
    # as a shell command (preceded by a '!') leave as is...
    s/\\/\//g unless ($_ =~ /^\s*!/);

    # If reading from a file, skip blank lines and comment lines.
    # Also, print out *real* commands as they are about to be executed.
    if (@multiCmd or $oneLastTime)
    {
        if (@multiCmd == 1)
        {
            $oneLastTime = 1;
        }
        else
        {
            $oneLastTime = 0;
        }

        # print command
        print $_;
    }
    elsif ($cmdf ne "STDIN")
    {
        if ($_ =~ /^\s*#/)
        {
            print "$_";
            displayPrompt();
            next CMD;
        }

        if ($_ =~ /^\s$/)
        {
            next CMD;
        }

        # Make var substitutions if possible
        my $key;
        foreach $key (keys %varHash) {
            s/\$$key/$varHash{$key}/g;
        }

        # print command
        print $_;
    }

    # if '!' indicating a shell command, go do it
    if ($_ =~ /^\s*!/) {
        s/^\s*!//;
        system "$_";

        displayPrompt();
        next CMD;
    }

    # simulate a command line by stuffing STDIN into ARGV
    @ARGV = split;

    # extract the command
    $cmd = shift @ARGV;

    # clear all input parameters from the last command
    undef $opt_a;   undef $opt_n;   undef $opt_A;   undef $opt_N;
    undef $opt_b;   undef $opt_o;   undef $opt_B;   undef $opt_O;
    undef $opt_c;   undef $opt_p;   undef $opt_C;   undef $opt_P;
    undef $opt_d;   undef $opt_q;   undef $opt_D;   undef $opt_Q;
    undef $opt_e;   undef $opt_r;   undef $opt_E;   undef $opt_R;
    undef $opt_f;   undef $opt_s;   undef $opt_F;   undef $opt_S;
    undef $opt_g;   undef $opt_t;   undef $opt_G;   undef $opt_T;
    undef $opt_h;   undef $opt_u;   undef $opt_H;   undef $opt_U;
    undef $opt_i;   undef $opt_v;   undef $opt_I;   undef $opt_V;
    undef $opt_j;   undef $opt_w;   undef $opt_J;   undef $opt_W;
    undef $opt_k;   undef $opt_x;   undef $opt_K;   undef $opt_X;
    undef $opt_l;   undef $opt_y;   undef $opt_L;   undef $opt_Y;
    undef $opt_m;   undef $opt_z;   undef $opt_M;   undef $opt_Z;

    # 'getopt' (without the 's') expects all parameters that have an associated
    # text or numeric field to be listed in the input pattern.  'getopts' can
    # be set up to either expect a field or just the option. Refer to the
    # "Programming Perl" book for details.
    #
    # Uppercase options (A, B, C...) take no additional parameters
    # Lowercase options (a, b, c...) can be followed by additional parameters
    #                   e.g. -a 45
    getopts('ABCDEFGHIJKLMNOPQRSTUVWXYZa:b:c:d:e:f:g:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z');

    # Transfer ARGV to our own array for the remainder of the processing
    @args = @ARGV;

    # Parse the input command
    CMD_SW:
    {
        if (!defined($cmd)) {
#            if (($currentConnection >= 0) &&
#                ($currentIP ne "") &&
#                (defined($currentMgr)) &&
#                (defined($connections{$currentConnection})))
#            {
#                $currentMgr->vcgInfo(0);
#            }

#            if ($currentPort == 2341)
#            {
#                $currentMgr->_receiveX1PacketSync(0, "flush");
#            }

            displayPrompt();
            next CMD;
        }

        # Expand an abbreviated command
        my $cmd1 = $cmd;

        $cmd = $cmdHash{uc($cmd)};
        if (! defined($cmd)) {
            $cmd = $cmd1;
        }

        if (($opt_H || (defined($args[0]) && $args[0] eq '?')) && $cmd !~ /^HELP$/i)
        {
            @args = ($cmd);
            $opt_H = 0;
            help();
            last CMD_SW;
        }

        if ($cmd =~ /^CONNECT$/i) {
            login();
            last CMD_SW;
        }

        if ($cmd =~ /^DISCONNECT$/i) {
            logout();
            last CMD_SW;
        }

        if ($cmd =~ /^[0-9]+$/i) {
            @args = ($cmd);
            chgConn();
            last CMD_SW;
        }

        if ($cmd =~ /^CHGCONN$/i) {
            chgConn() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DSPCONN$/i) {
            dspConn() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^BATTERYHEALTHSET$/i) {
            batteryHealthSet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DEVICECOUNT$/i) {
            deviceCount() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DEVICENAME$/i) {
            deviceName() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DEVICECONFIGGET$/i) {
            deviceConfigGet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DEVICECONFIGSET$/i) {
            deviceConfigSet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DEVSTATUS$/i) {
            devStatus() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ENGDEBUG$/i) {
            engDebug() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^WRITEBUFFER$/i) {
            writeBuffer() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SCSICMD$/i) {
            scsiCmd() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PAGE30BAY$/i) {
            page30bay() if isActiveConnection();
            last CMD_SW;
        }

        if (($cmd =~ /^FWUPDATE$/i) || ($cmd =~ /^MPXFWUPDATE$/i)) {
            if (isActiveConnection())
            {
                # move into own function someday...
                my ($filename) = @args;
                if (defined $filename)
                {
                    $filename =~ s/\./\\./g;    # escape the '.'s
                    m/$filename.*/;             # find the filename in $_
                    ($filename = $&) =~ s/"//g; # strip the quotes (if any)"
                    fwUpdate($filename, $cmd);
                }
            }
            last CMD_SW;
        }

        if ($cmd =~ /^FWVERSION$/i) {
            fwVersion() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^FWSYSREL$/i) {
            FwSysRel() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^FAILURESTATESET$/i)
        {
            failureStateSet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TIMESYNC$/i) {
            timeSync() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^IPSET$/i) {
            ipAddressSet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^IPGET$/i) {
            ipAddressGet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DEBUGADDR$/i) {
            debugAddressSet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GENFUNCTION$/i) {
            genFunction() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GETTIME$/i) {
            getTime() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GENMRP$/i) {
            genMRP() if isActiveConnection();
            last CMD_SW;
        }
        if ($cmd =~ /^FOREIGNTARGET$/i) {
            FOREIGNTARGET() if isActiveConnection();
            last CMD_SW;
        }
        if ($cmd =~ /^ISEIPS$/i) {
            ISEIPS() if isActiveConnection();
            last CMD_SW;
        }
        if ($cmd =~ /^EMULATEPAB$/i) {
            EMULATEPAB() if isActiveConnection();
            last CMD_SW;
        }
        if ($cmd =~ /^SWAPPIDS$/i) {
            SWAPPIDS() if isActiveConnection();
            last CMD_SW;
        }
        if ($cmd =~ /^JUMPERS$/i) {
            Jumpers() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^CACHETEST$/i) {
            cacheTest() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^CACHEREFRESHCCB$/i) {
            CacheRefreshCCB() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ALINK$/i) {
            Alink() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DISASTERTEST$/i) {
            disasterTest() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^KEEPALIVETEST$/i) {
            keepAliveTest() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^FIOMAPTEST$/i) {
            fioMapTest() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^FCMCOUNTERTEST$/i) {
            fcmCounterTest() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^LOGEVENT$/i) {
            logEvent() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ERRORTRAP$/i) {
            errorTrap() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^FAILUREMANAGERTEST$/i) {
            failureManagerTest() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^INTERFACEFAIL$/i) {
            interfaceFail() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^INTERFACEUNFAIL$/i) {
            interfaceUnfail() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SOSTABLE$/i) {
            sosTable() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GETREPORT$/i) {
            getReport() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GLOBALCACHEINFO$/i) {
            globalCacheInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GLOBALCACHESET$/i) {
            globalCacheSet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^INITPROCNVRAM$/i) {
            if (isActiveConnection()) {
                initProcNVRAM() if AreYouSure();
            }
            last CMD_SW;
        }

        if ($cmd =~ /^INITCCBNVRAM$/i) {
            if (isActiveConnection()) {
                initCcbNVRAM() if AreYouSure();
            }
            last CMD_SW;
        }

        if ($cmd =~ /^HELP$/i || $cmd eq '?') {
            help();
            last CMD_SW;
        }

        if ($cmd =~ /^LET$/i) {
            m/\w+\s*=\s*[\w".]+/;
            my @vars = split /=/, $&;
            $vars[0] =~ s/\s+//g;
            $vars[1] =~ s/\s+|"//g; # - add (") to revive syntax coloring.
            $varHash{$vars[0]} = $vars[1];
            last CMD_SW;
        }

        if ($cmd =~ /^LOGCLEAR$/i) {
            logClear() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^LOGINFO$/i) {
            logInfoClassic() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DSPLOGS$/i) {
            logInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^LOGTEXTMESSAGE$/i) {
            logTextMessage() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^LOGACKNOWLEDGE$/i) {
            logAcknowledge() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^LEDCONTROL$/i) {
            ledControl() if isActiveConnection();
            last CMD_SW;
        }

        if (($cmd =~ /^MEMREAD$/i) or ($cmd =~ /^MPXMEMREAD$/i)) {
            memRead($cmd) if isActiveConnection();
            last CMD_SW;
        }

        if (($cmd =~ /^MEMWRITE$/i) or ($cmd =~ /^MPXMEMWRITE$/i)) {
            memWrite($cmd) if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDATACHECKSUM$/i) {
            pDataChecksum() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDATARESET$/i) {
            pDataReset() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDATAREAD$/i) {
            pDataRead() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDATAWRITE$/i) {
            pDataWrite() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^EWOKDATACONTROL$/i) {
            pEwokDataCtrl() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^REGISTEREVENTS$/i) {
            registerEvents() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^LINUXFILEREAD$/i) {
            if (isBigfoot())
            {
                print "Only available for Wookiee controllers\n";
            }
            else
            {
                linuxFileRead() if isActiveConnection();
            }
            last CMD_SW;
        }

        if ($cmd =~ /^FIDREAD$/i) {
            fidRead($cmd) if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^CCBBACKTRACE$/i) {
            unshift @args, 293;
            fidRead($cmd) if isActiveConnection();
            last CMD_SW;

        }

        if ($cmd =~ /^FIDDECODE$/i) {
            fidDecode();
            last CMD_SW;
        }

        if ($cmd =~ /^FIDWRITE$/i) {
            fidWrite() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STRUCTUREINFO$/i) {
            structureDisplayInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^NETSTAT$/i) {
            @args = (2);
            structureDisplayInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^MMTEST$/i) {
            mmtest() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^MODEBITINFO$/i) {
            print "\nPlease use the MODEBITS command.  MODEBITS -I gets\n" .
                  "you the old MODEBITINFO interface.\n";
            $opt_I = 1;
            modebits() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^MODEBITSET$/i) {
            print "\nPlease use the MODEBITS command.  MODEBITS -S gets\n" .
                  "you the old MODEBITSET interface.\n";
            $opt_S = 1;
            modebits() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^MODEBITS$/i) {
            modebits() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^CFGOPTION$/i) {
            cfgoption() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DPRINTF$/i) {
            dprintf() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKS$/i) {
            pdisks() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKSCACHE$/i) {
            pdiskscache() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKBEACON$/i) {
            pdiskBeacon() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKTIMEOUT$/i) {
            pdiskTimeout() if isActiveConnection();
            last CMD_SW;
        }
        
        if ($cmd =~ /^MISCCOUNT$/i) {
            miscCount() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKCOUNT$/i) {
            pdiskCount() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKDEFRAG$/i) {
            pdiskDefrag() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKDEFRAGSTATUS$/i) {
            pdiskDefragStatus() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKFAIL$/i) {
            pdiskFail() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKINFO$/i) {
            pdiskInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKLABEL$/i) {
            pdiskLabel() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^MISCLIST$/i) {
            miscList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKLIST$/i) {
            pdiskList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKRESTORE$/i) {
            if (isActiveConnection()) {
                pdiskRestore() if AreYouSure();
            }
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKDELETE$/i) {
            pdiskDelete() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKLEDSTATE$/i) {
            pdiskLedState() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKBYPASS$/i) {
            if (isActiveConnection()) {
                pdiskBypass() if AreYouSure();
            }
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKSPINDOWN$/i) {
            if (isActiveConnection()) {
                pdiskSpindown() if isActiveConnection();
            }
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKAUTOFAILBACK$/i) {
            pdiskAutoFailBack() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PDISKFAILBACK$/i) {
            pdiskFailBack() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SETGEOLOCATION$/i) {
            setGeoLocation() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^CLEARGEOLOCATION$/i) {
            clearGeoLocation() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GETGEOLOCATION$/i) {
            getGeoLocation() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SINGLEVIDSONISE$/i) {
            singlevidsonise() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VIDSONDATAPACS$/i) {
            vidsondatapac() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^CHECKEVACDATAPAC$/i) {
            checkevacdatapac() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DISKBAYS$/i) {
            diskBays() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^COUNTS$/i) {
            counts() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DISKBAYCOUNT$/i) {
            diskBayCount() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DISKBAYLIST$/i) {
            diskBayList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DISKBAYINFO$/i) {
            diskBayInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DISKBAYENVIRO$/i) {
            diskBayEnviro() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DISKBAYALARMCTRL$/i) {
            diskBayAlarmControl() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DISKBAYSTATUS$/i) {
            diskBayStatus() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DISKBAYDELETE$/i) {
            diskBayDelete() if isActiveConnection();
            last CMD_SW;
        }

        if (($cmd =~ /^QUIT$/i) || ($cmd =~ /^EXIT$/i)) {
            quit();
            print "\n";
            last CMD;
        }

        if ($cmd =~ /^MFGCLEAN$/i) {
            if (isActiveConnection()) {
                mfgClean() if AreYouSure();
            }
            last CMD_SW;
        }

        if ($cmd =~ /^RESET$/i) {
            resetProcessor() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ASSIGNMP$/i) {
            assignMP() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RESETQLOGIC$/i) {
            resetQlogic() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^LOOPPRIMITIVE$/i) {
            loopPrimitive() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RESCANDEVICE$/i) {
            rescanDevice() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DEVICELIST$/i) {
            deviceList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PORTLIST$/i) {
            portList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^BEDEVICEPATHS$/i) {
            beDevicePaths() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RESYNCMIRRORS$/i) {
            resyncMirrors() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RUN$/i || $cmd =~ /^SOURCE$/i ) {
            # move into own function someday...
            $cmd = shift @args;
            if (defined $cmd) {
                $cmd =~ s/\./\\./g;    # escape the '.'s
                m/$cmd.*/;             # find the cmd in $_
                ($cmd = $&) =~ s/"//g; # strip the quotes (") (if any)
                if (-r $cmd) {
                    $cmdf = 'CMDF';
                    open $cmdf, $cmd or $cmdf = "STDIN";
                }
                else {
                    print "Can't run \"$cmd\"\n";
                }
            }
            last CMD_SW;
        }

        if ($cmd =~ /^POWERUPSTATE$/i) {
            powerUpState() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^POWERUPRESPONSE$/i) {
            powerUpResponse() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RESYNCCTL$/i) {
            resyncCtl() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RESYNCDATA$/i) {
            resyncData() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RMSTATE$/i) {
            rmState() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SCRUBINFO$/i) {
            scrubInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SCRUBSET$/i) {
            scrubSet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERIALNUMBERS$/i) {
            serialNumGet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SETSERIALNUM$/i) {
            serialNumSet() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERVERS$/i) {
            servers() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERVERPROPERTY$/i) {
            serverProp() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERVERASSOC$/i) {
            serverAssoc() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERVERCOUNT$/i) {
            serverCount() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERVERCREATE$/i) {
            serverCreate() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERVERDELETE$/i) {
            serverDelete() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERVERDISASSOC$/i) {
            serverDisassoc() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERVERINFO$/i) {
            serverInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GETWORKSETINFO$/i) {
            getWorksetInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SETWORKSETINFO$/i) {
            setWorksetInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERVERLIST$/i) {
            serverList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SERVERWWNTOTARGETMAP$/i) {
            serverWwnToTargetMap() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STATSCACHEDEV$/i) {
            statsCacheDevices() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STATSLOOP$/i) {
            statsLoop() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STATSPCI$/i) {
            statsPCI() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STATSPROC$/i) {
            statsProc() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STATSSERVER$/i) {
            statsServer() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STATSSERVERS$/i) {
            statsServers() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STATSVDISK$/i) {
            statsVDisk() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PERFS$/i) {
            Perfs() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STATSENVIRONMENTAL$/i) {
            statsEnv() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ENVIIGET$/i) {
            envII() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STATSHAB$/i) {
            StatsHAB() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^STATSBUFFERBOARD$/i) {
            statsBufferBoard() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SNAPTAKE$/i) {
            takeSnapshot() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SNAPLOAD$/i) {
            loadSnapshot() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SNAPCHANGE$/i) {
            changeSnapshot() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SNAPREADDIR$/i) {
            readdirSnapshot() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TARGETS$/i) {
            targets() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TARGETSTATUS$/i) {
            targetStatus() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TARGETCOUNT$/i) {
            targetCount() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TARGETINFO$/i) {
            targetInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TARGETLIST$/i) {
            targetList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TARGETMOVE$/i) {
            targetMove() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TARGETSETPROP$/i) {
            targetSetProperties() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ISCSISETINFO$/i) {
            iSCSISetInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ISCSIGETINFO$/i) {
            iSCSIGetInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ISCSICHAPSETINFO$/i) {
            iSCSIChapSetInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ISCSICHAPGETINFO$/i) {
            iSCSIChapGetInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ISCSISTATS$/i) {
            iSCSIStats() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ISCSISTATSSERVER$/i) {
            iSCSIStatsServer() if isActiveConnection();
            last CMD_SW;
        }
        if ($cmd =~ /^ISNSSETINFO$/i) {
            iSNSSetInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ISNSGETINFO$/i) {
            iSNSGetInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^IDDINFO$/i) {
            iddInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TARGETRESLIST$/i) {
            targetResList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DLMPATHSTATS$/i) {
            dlmPathStats() if isActiveConnection();
            last CMD_SW;
        }
        if ($cmd =~ /^DLMPATHSELECTIONALGO$/i) {
            dlmPathSelectionAlgo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TARGETTEST$/i) {
            targetTest() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^TIMEOUT$/i) {
            timeout() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGACTIVATECONTROLLER$/i)
        {
            vcgActivateController() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGADDCONTROLLER$/i) {
            vcgAddController() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGAPPLYLICENSE$/i)
        {
            vcgApplyLicense() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGFAILCONTROLLER$/i)
        {
            vcgFailController() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGINACTIVATECONTROLLER$/i)
        {
            vcgInactivateController() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGINFO$/i) {
            vcgInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGPING$/i) {
            vcgPing() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGMPLIST$/i) {
            vcgMPList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GETCPUCOUNT$/i)
        {
            getCpuCount() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GETBETYPE$/i)
        {
            getBEType() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^REGISTERCLIENTTYPE$/i)
        {
            regClientType() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGREMOVECONTROLLER$/i)
        {
            vcgRemoveController() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGSHUTDOWN$/i)
        {
            vcgShutdown() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGDOELECTION$/i) {
            vcgDoElection() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGELECTIONSTATE$/i) {
            vcgElectionState() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGUNFAILCONTROLLER$/i)
        {
            vcgUnfailController() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGVALIDATION$/i) {
            vcgValidation(0) if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGCONFIGURECONTROLLER$/i)
        {
            vcgConfigController() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VCGVALIDATECONTROLLER$/i) {
            vcgValidateController() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKS$/i) {
            vdisks() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKSCACHE$/i) {
            vdiskscache() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKCONTROL$/i) {
            vdiskControl() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^QUICKMIRRORPAUSESTART$/i) {
            quickMirrorPauseStart() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^QUICKMIRRORPAUSESEQUENCE$/i) {
            quickMirrorPauseSequence() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^QUICKMIRRORPAUSEEXECUTE$/i) {
            quickMirrorPauseExecute() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^BATCHSNAPSHOTSTART$/i) {
            batchSnapshotStart() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^BATCHSNAPSHOTSEQUENCE$/i) {
            batchSnapshotSequence() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^BATCHSNAPSHOTEXECUTE$/i) {
            batchSnapshotExecute() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKCOUNT$/i) {
            vdiskCount() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKCREATE$/i) {
            vdiskCreate() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKSETPRIORITY$/i) {
            vdiskSetPriority() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GLOBALVDISKPRIORITY$/i) {
            vdiskPriorityEnable() if isActiveConnection();
            last CMD_SW;
        }
        if ($cmd =~ /^VDISKPRGET$/i) {
            vdiskPRGet() if isActiveConnection();
            last CMD_SW;
        }
        if ($cmd =~ /^VDISKPRCLR$/i) {
            vdiskPRClr() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKDELETE$/i) {
            vdiskDelete() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GETVDISKREDUNDANCY$/i) {
            vdiskBayRedundant() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKEXPAND$/i) {
            vdiskExpand() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKINFO$/i) {
            vdiskInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKOWNER$/i) {
            vdiskOwner() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RAIDINIT$/i) {
            raidInit() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKLBACONVERT$/i) {
            vdiskLBAConvert() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKLIST$/i) {
            vdiskList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKPREPARE$/i) {
            vdiskPrepare() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKSETCACHE$/i) {
            vdiskSetCache() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VDISKSETATTRIBUTES$/i) {
            vdiskSetAttributes() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RAIDS$/i) {
            raids() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RAIDSCACHE$/i) {
            raidscache() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RAIDBEACON$/i) {
            raidBeacon() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RAIDCOUNT$/i) {
            raidCount() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RAIDLIST$/i) {
            raidList() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RAIDINFO$/i) {
            raidInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^RAIDRECOVER$/i) {
            raidRecover() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ROLLINGUPDATEPHASE$/i) {
            rollingUpdatePhase() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VLINKS$/i) {
            vlinks() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VLINKCTRLCOUNT$/i) {
            vlinkCtrlCount() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VLINKCTRLINFO$/i) {
            vlinkCtrlInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VLINKCTRLVDISKS$/i) {
            vlinkCtrlVDisks() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VLINKCREATE$/i) {
            vlinkCreate() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VLINKINFO$/i) {
            vlinkInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VLINKBREAKLOCK$/i) {
            vlinkBreakLock() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VLINKDLINKINFO$/i) {
            VLinkDLinkInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^VLINKDLOCKINFO$/i) {
            VLinkDLockInfo() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^GETNAME$/i)
        {
            getName() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^WRITENAME$/i)
        {
            writeName() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ISESTATUS$/i)
        {
            PIISEStatus() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^ISEBEACON$/i)
        {
            PIISEBeacon() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PIGETX1ENV$/i)
        {
            PIGetX1Env() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^DISKREADWRITE$/i)
        {
            diskReadWrite() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^SETCONTROLLERTYPE$/i)
        {
            SetControllerType() if isActiveConnection();
            last CMD_SW;
        }

        if ($cmd =~ /^PWD$/i) {
            print "\nPWD: " . cwd() . "\n";
            last CMD_SW;
        }

        if ($cmd =~ /^CD$/i) {
            # move into own function someday...
            my ($newdir) = @args;
            if (defined $newdir) {
                $newdir =~ s/\./\\./g;
                m/$newdir.*/;             # find the dir in $_
                ($newdir = $&) =~ s/"//g; # strip the quotes (in any)"
                if ($newdir ne "") {
                    chdir $newdir or print "\nCan't \"cd $newdir\"\n";
                }
            }
            print "\nPWD: " . cwd() . "\n";
            last CMD_SW;
        }

        if (($cmd =~ /^LS$/i) || ($cmd =~ /^DIR$/i)) {
            dispDir();
            last CMD_SW;
        }

        if (($cmd =~ /^CLEAR$/i) || ($cmd =~ /^CLS$/i)) {
            if ($windoze)
            {
                system "cls";
            }
            else
            {
                system "clear";
            }
            last CMD_SW;
        }

        if ($cmd =~ /^SHELL$/i) {
            print "Shelling to an external command interpreter.\n";
            print "Type EXIT to return to ccbcl.pl\n\n";
            if ("$^O" eq "MSWin32")
            {
                system "cmd";
            }
            else
            {
                system "$ENV{SHELL}";
            }
            last CMD_SW;
        }

        if ($cmd =~ /^#$/) {
            # Ignore # as a comment.
            last CMD_SW;
        }

        # default case - unknown command
        print "\nUnknown or ambiguous command -- try again.\n";
        last CMD_SW;
    } # end of CMD_SW

    displayPrompt();
}

# Remove $dashEcmdFile.
unlink($dashEcmdFile);
exit 0;

# End of main routine
#-----------------------------------------------------------------------------

# Remove $dashEcmdFile.
unlink($dashEcmdFile);
exit 0;

# End of main routine
#-----------------------------------------------------------------------------

sub GetNextCmdFile
{
    my $cmdFile;
    my $handle = "STDIN";

    while (@commandFiles) {
        ($cmdFile) = shift @commandFiles;

        if (-r $cmdFile) {
            $handle = 'CMDF';
            open $handle, $cmdFile;
            last;
        }
        else {
            print "Can't open $cmdFile!\n";
            next;
        }
    }
    return $handle;
}

if ($cmdf ne "STDIN")
{
    close $cmdf;
    $cmdf = GetNextCmdFile();

    # empty out the substition hash
    foreach my $key (keys %varHash) {
        delete $varHash{$key};
    }

    goto CMD;
}

##############################################################################
# Name:     genHTML
#
# Desc:     Generate an html version of the help text.
##############################################################################
sub genHTML
{
    my $text;
    my @keys;
    my $rc = 0;

    @keys = sort(keys(%help));

    # Write the html header
    $rc = open HTML, ">$startDir/ccbcl.html";
    if (!$rc)
    {
        print "Couldn't open \"$startDir/ccbcl.html\" for write.\n";
        return;
    }
    print HTML "<html>\n";
    print HTML "<head><title>CCBCL Help</title></head>\n";
    print HTML "<body>\n";
    print HTML "<h1 align=\"CENTER\">CCBCL Help</h1>\n";

    # Loop through each command and display the short text,
    # adding a link for the location of the long description
    print HTML "<table>\n";
    print HTML "<tr align=\"LEFT\"><th>Command</th><th>Description</th></tr>\n";
    foreach my $key (@keys) {
        $text = $help{$key}{SHORT};
        chomp($text);  # be tolerant of missed/extra newlines

        printf(HTML "<tr><a name=\"%sIDX\"></a>\n", $key);
        printf(HTML "<td><strong><a href=\"ccbcl.html#%s\">%s</a></strong></td>\n",
            $key, $key);
        printf(HTML "<td>%s</td></tr>\n", $text);
    }
    print HTML "</table>\n";

    # Loop through each command and display the long text.
    foreach my $key (@keys) {
        $text = $help{$key}{SHORT};
        $text =~ s/[<>]//g;
        chomp($text);  # be tolerant of missed/extra newlines
        printf(HTML "<a name=\"%s\"></a><hr><pre><code>%s\n\n", $key, $text);

        $text = $help{$key}{LONG};
        $text =~ s/[<>]//g;
        $text = "<strong>$text</code></pre>";
        $text =~ s/\n/<\/strong>\n/;

        chomp($text);  # be tolerant of missed/extra newlines
        printf(HTML "%s\n", $text);

        printf(HTML "<strong><a href=\"ccbcl.html#%sIDX\">Back</a></strong>\n",
            $key);
    }

    # finish up
    print HTML "<hr>\n";
    print HTML "</body>\n";
    print HTML "</html>\n";
    close HTML;

    system "cmd /c start $startDir/ccbcl.html";
}

##############################################################################
# Name:     help
#
# Desc:     Help function. Allows partial commands to be matched and displayed.
#
# Input:    Optional command or partial command.
##############################################################################
sub help
{
    my ($search) = @args;

    my @keys;
    my $lastKey;

    @keys = sort(keys(%help));

    if ($opt_H)
    {
        genHTML();
        return;
    }

    print "\n";

    # If no search parameter, show the help help
    if (!defined($search)) {
        $search = "HELP";
    }

    # Show all commands
    elsif ($search eq "*") {
        foreach my $key (@keys) {
            my $short = $help{$key}{SHORT};
            chomp($short);  # be tolerant of missed/extra newlines
            printf("%-20s %s\n", $key, $short);
        }
        return;
    }

    # else be more specific
    # determine how many matches we have
    my $count = 0;
    my $matchBegin = 0;
    foreach my $key (@keys) {
        if ($key =~ /$search/i) {
            $lastKey = $key;
            if ($key =~ /^$search/i) {
                $matchBegin = 1;
            }
            if ($key =~ /^$search$/i) {
                $count = 1;
                last;
            }
            $count++;
        }
    }

    # more than 1, display the list
    if (($count > 1) or ($count == 1 and $matchBegin == 0)) {
        foreach my $key (@keys) {
            if ($key =~ /$search/i) {
                my $short = $help{$key}{SHORT};
                chomp($short);  # be tolerant of missed/extra newlines
                printf("%-20s %s\n", $key, $short);
            }
        }
    }

    # only 1, display the extended help
    elsif ($count == 1) {
        my $short = $help{$lastKey}{SHORT};
        chomp($short);  # be tolerant of missed/extra newlines
        print "$short\n\n";
        my $long = $help{$lastKey}{LONG};
        chomp($long);  # be tolerant of missed/extra newlines
        print "$long\n";
    }

    # else not found
    else {
        print "No help found for \"$search\"\n";
    }
}

sub BuildHelp
{
##############################################################################
#
#   RULES for writing help text:
#
#   1) The COMMAND that you are describing is the hash KEY, it must be
#      spelled correctly in both the SHORT and LONG description definitions,
#      and must be UPPERCASE.
#
#   2) The SHORT description must be 1 line only, and fairly short at that.
#      The SHORT description sting DOES NOT end with a newline (\n).
#
#   3) The LONG description can be as long as you want it to be.  Make sure
#      to end each line with a newline (\n).  Concatenate lines with the
#      period (.) operator to form one big string.  The final line must be
#      terminated with a semicolon (;).
#
#   4) Order of commands does not matter - the help routine alphabetizes
#      everything by COMMAND/KEY.
#
##############################################################################
# short description
    $help{ALINK}{SHORT} =
        "ALINK [APOOL] [WHAT] [DELETE] [LINK] [UNLINK] [vid cid ord] - Subarguments control ALINK.";

# long description
    $help{ALINK}{LONG} =
        "ALINK APOOL size       Create APOOL of specified size.\n" .
        "ALINK WHAT             List possible connections for VLINKS/ALINKS.\n" .
        "ALINK DELETE vid       Delete alink.\n" .
        "ALINK UNLINK vid       Turn alink into vlink.\n" .
        "ALINK LINK vid         Turn vlink into alink.\n" .
        "ALINK vid cid ord      Create ALINK at vdisk vid to cid and disk ord.\n" . 
        "      attribute bit for vdisk = bit 2, or 0x0004.\n\n";

##############################################################################
# short description
    $help{ASSIGNMP}{SHORT} =
        "Assign the mirror parnter for this controller.";

# long description
    $help{ASSIGNMP}{LONG} =
        "ASSIGNMP serial_number\n" .
        "\n" .
        "  serial_number      Mirror partner serial number\n";

##############################################################################
# short description
    $help{BATCHSNAPSHOTSTART}{SHORT} =
        "Start command for Batch Snapshot.";

# long description
    $help{BATCHSNAPSHOTSTART}{LONG} =
        "BATCHSNAPSHOTSTART count\n" .
        "\n" .
        "  count       Number of batch snapthot list is going to be sent in sequence command.\n";

##############################################################################
# short description
    $help{BATCHSNAPSHOTSEQUENCE}{SHORT} =
        "Sequence command for Batch snapshot.";

# long description
    $help{BATCHSNAPSHOTSEQUENCE}{LONG} =
        "BATCHSNAPSHOTSEQUENCE src_vid dest_vid\n" .
        "\n" .
        "  src_vid            Source Virtual disk.\n" .
        "  dest_vid           Destination virtual disk.\n";

##############################################################################
# short description
    $help{BATCHSNAPSHOTEXECUTE}{SHORT} =
        "Execute/Cancel command for Batch Snapshot list.";

# long description
    $help{BATCHSNAPSHOTEXECUTE}{LONG} =
        "BATCHSNAPSHOTEXECUTE action\n" .
        "\n" .
        "  action          execute command option.\n" .
        "                       CANCEL          0x00\n" .
        "                       GO              0x01\n" ;

##############################################################################
# short description
    $help{BATTERYHEALTHSET}{SHORT} =
        "Sets the battery health for a given board.";

# long description
    $help{BATTERYHEALTHSET}{LONG} =
        "BATTERYHEALTHSET state\n" .
        "\n" .
        "  state                Battery State (can the battery support the 72 hour requirement)\n" .
        "                         0 or GOOD = 72 hour requirement, YES\n" .
        "                         1 or BAD  = 72 hour requirement, NO\n";

##############################################################################
# short description
    $help{BEDEVICEPATHS}{SHORT} =
        "Lists the Backend Device Paths.";

# long description
    $help{BEDEVICEPATHS}{LONG} =
        "BEDEVICEPATHS [type] [format]\n" .
        "  type(PDISK|PDISKS|MISC|ENC|SES)  Type of device list to retrieve.\n" .
        "                             PDISK or PDISKS - Physical disks (default).\n" .
        "                             MISC            - Miscellaneous devices.\n" .
        "                             ENC or SES      - Disk bays.\n" .
        "                             1               - Display PDISK with format 1.\n" .
        "  format(BP|1|PA)        Format of the return values.\n" .
        "                             BP    - Bit path.\n" .
        "                             1     - Path Array, only those with count != 2.\n" .
        "                             PA    - Path Array.\n";

##############################################################################
# short description
    $help{CACHEREFRESHCCB}{SHORT} =
        "Refresh the CCB Cache.";

# long description
    $help{CACHEREFRESHCCB}{LONG} =
        "CACHEREFRESHCCB cacheMask [waitForCompletion]\n" .
        "\n" .
        "  cacheMask            which cache(s) to refresh:\n" .
        "                       More help coming soon.  For now look at CacheManager.h\n" .
        "                       for the appropriate CACHE_INVALIDATE_ value\n" .
        "\n" .
        "  waitForCompletion    (DEFAULT=TRUE)\n" .
        "                       TRUE = returns after cache has been refreshed\n" .
        "                       FALSE = returns immediately\n\n";

##############################################################################
# short description
    $help{CACHETEST}{SHORT} =
        "Tests the cache function directly or through X1 async interface.";

# long description
    $help{CACHETEST}{LONG} =
        "CACHETEST [options] test [timeout]\n" .
        "\n" .
        "  options (Only for \"test\" CHANGE):\n" .
        "                       -A (X1_ASYNC_VCG_ELECTION_STATE_CHANGE)\n" .
        "                       -B (X1_ASYNC_VCG_ELECTION_STATE_ENDED)\n" .
        "                       -C (X1_ASYNC_VCG_POWERUP)\n" .
        "                       -D (X1_ASYNC_VCG_CFG_CHANGED)\n" .
        "                       -H (X1_ASYNC_HCHANGED)\n" .
        "                       -P (X1_ASYNC_PCHANGED)\n" .
        "                       -R (X1_ASYNC_RCHANGED)\n" .
        "                       -V (X1_ASYNC_VCHANGED)\n" .
        "                       -Z (X1_ASYNC_ZCHANGED)\n" .
        "\n" .
        "  test (CHANGE|X1|CACHE|STOP)\n" .
        "                       CHANGE - Send X1 change event without cache refresh\n" .
        "                       X1     - Test through X1 async notifications\n" .
        "                       CACHE  - Test cache directly.\n" .
        "                       STOP   - Stop all tests.\n" .
        "                                (TESTS WILL RUN UNTIL THIS IS DONE!)\n" .
        "\n" .
        "  timeout              Timeout in milliseconds between each test (default 0).\n";

##############################################################################
# short description
    $help{CCBBACKTRACE}{SHORT} =
        "Reads and formats the CCB NVRAM backtrace data (same as FIDREAD 293).";

# long description
    $help{CCBBACKTRACE}{LONG} =
        "CCBBACKTRACE [-t byte|short|word|binary] [-f filename] [-l length]\n" .
        "\n" .
        "  -t byte|short|word|binary|fmt   Format of output data (default: fmt).\n" .
        "  -f filename        Name of file to write output to.\n" .
        "  -l length          Number of bytes to display (default: entire FID).\n";

##############################################################################
# short description
    $help{CD}{SHORT} =
        "Changes the current working directory.";

# long description
    $help{CD}{LONG} =
        "CD directory\n" .
        "\n" .
        "  directory          The directory path to change to.\n";

##############################################################################
# short description
    $help{CFGOPTION}{SHORT} =
        "Configures options for the controller.";

# long description
    $help{CFGOPTION}{LONG} =
        "CFGOPTION [data] [mask]\n" .
        "\n" .
        "  If no parameters specified, CFGOPTION \"gets\" the current options.\n" .
        "\n" .
        "  data             32 bit data\n" .
        "  mask             32 bit mask\n" .
        "\n" .
        "  Bit definitions:\n" .
        "    0 = WHQL\n";

##############################################################################
# short description
    $help{CHECKEVACDATAPAC}{SHORT} =
        "Check system for ISE datapac evacuation purposes.";

# long description
    $help{CHECKEVACDATAPAC}{LONG} =
        "CHECKEVACDATAPAC ISE DATAPAC [ISE DATAPAC] ...\n" .
        "  ISE                An ISE diskbay number (example: 2).\n" .
        "  DATAPAC            An ISE DATAPAC number (either 1 or 2).\n";

##############################################################################
# short description
    $help{CHGCONN}{SHORT} =
        "Changes the active connection.";

# long description
    $help{CHGCONN}{LONG} =
        "CHGCONN num\n" .
        "\n" .
        "  num               Connection identifier (from DSPCONN cmd) or\n" .
        "                    IP or PORT number or IP:PORT combination of the\n" .
        "                    active connection to switch control to.\n\n".
        "NOTE: the 'CHGCONN' itself is optional -- entering just 'num' for\n".
        "      the command is the same as entering 'CHGCONN num'.\n";

##############################################################################
# short description
    $help{CLEAR}{SHORT} =
        "Clear the screen.";

# long description
    $help{CLEAR}{LONG} =
        "CLEAR/CLS\n";

##############################################################################
# short description
    $help{CLEARGEOLOCATION}{SHORT} =
        "Clears the Geo location of all the bays.";

# long description
    $help{CLEARGEOLOCATION}{LONG} =
        "CLEARGEOLOCATION\n";

##############################################################################
# short description
    $help{CONNECT}{SHORT} =
        "Establishes a connection to a Bigfoot controller.";

# long description
    $help{CONNECT}{LONG} =
        "CONNECT ipaddress\n" .
        "\n" .
        "  ipaddress         IP Address of the controller (nnn.nnn.nnn.nnn).\n";

##############################################################################
# short description
    $help{DEBUGADDR}{SHORT} =
        "Sets the ip address of the Debug Console.";

# long description
    $help{DEBUGADDR}{LONG} =
        "DEBUGADDR ipadr [channel]\n" .
        "\n" .
        "  ipadr          ip address of the machine to send debug info to.\n" .
        "                 Other valid inputs:\n" .
        "                     THIS - use this machines ip address.\n" .
        "                     PI   - use the Packet Intf connected machine's ip address.\n" .
        "                     OFF  - turn the debug console off.\n" .
        "  channel(0-19)  Channel to send debug info to (default: 0).\n";

##############################################################################
# short description
    $help{DEVICECONFIGGET}{SHORT} =
        "Retrieve the device configuration.";

# long description
    $help{DEVICECONFIGGET}{LONG} =
        "DEVICECONFIGGET\n";

##############################################################################
# short description
    $help{DEVICECONFIGSET}{SHORT} =
        "Updates the device configuration.";

# long description
    $help{DEVICECONFIGSET}{LONG} =
        "DEVICECONFIGSET file\n" .
        "\n" .
        "  file               Device configuration file name\n" .
        "                       (fully qualified)\n";

##############################################################################
# short description
    $help{DEVICECOUNT}{SHORT} =
        "Gets the device count for the given serial number.";

# long description
    $help{DEVICECOUNT}{LONG} =
        "DEVICECOUNT serial_number\n" .
        "\n" .
        "  serial_number      Serial number.\n";

##############################################################################
# short description
    $help{DEVICELIST}{SHORT} =
        "Displays the devices on the specified port.";

# long description
    $help{DEVICELIST}{LONG} =
        "DEVICELIST [processor] [port]\n" .
        "\n" .
        "  processor          Processor to display: FE|BE\n" .
        "                     (default: FE)\n" .
        "  port               port to display\n" .
        "                         (0 - 3) display individual port\n" .
        "                         (default: 0)\n";

##############################################################################
# short description
    $help{DEVICENAME}{SHORT} =
        "Sets or retrieves a device name.";

# long description
    $help{DEVICENAME}{LONG} =
        "DEVICENAME id option [name]\n" .
        "\n" .
        "  id                 Device identifier.\n" .
        "  option             Option for the command:\n" .
        "                       0 or SERVER     = Set server name\n" .
        "                       1 or VDISK      = Set virtual disk name\n" .
        "                       2 or VCG        = Set VCG name\n" .
        "                       3 or RETVCG     = Retrieve VCG name\n" .
        "  name               Name for the device.\n";

##############################################################################
# short description
    $help{DEVSTATUS}{SHORT} =
        "Displays device status information.";

# long description
    $help{DEVSTATUS}{LONG} =
        "DEVSTATUS ([switch])[device](s)........\n" .
        "\n" .
        "  ([switch])          Optional switch.\n" .
        "                      /s  - display usage information per second\n" .
        "                      /b  - display only bad devices/vdisks/raids\n" .
        "                            no switch is default information\n" .
        "  [device]            Device type.\n" .
        "                      ALL - All disks[default].\n" .
        "                      PD -  Physical disks.\n" .
        "                      RD -  Raid device.\n" .
        "                      VD -  Virtual disks.\n" .
        "  ALL can only be used by itself as a parameter.\n";

##############################################################################
# short description
    $help{DISASTERTEST}{SHORT} =
        "Tests the disaster mode functionality.";

# long description
    $help{DISASTERTEST}{LONG} =
        "DISASTERTEST flag\n" .
        "\n" .
        "  flag (value)\n" .
        "                       0     - Reset all disasterData storage (NVRAM)\n" .
        "                       1     - Clear disasterDetected flag\n" .
        "                       2     - Set disasterDetected flag\n";

##############################################################################
# short description
    $help{DISCONNECT}{SHORT} =
        "Closes the current connection.";

# long description
    $help{DISCONNECT}{LONG} =
        "DISCONNECT\n";

##############################################################################
# short description
    $help{DISKBAYALARMCTRL}{SHORT} =
        "Sends an alarm control byte to a disk bay.";

# long description
    $help{DISKBAYALARMCTRL}{LONG} =
        "DISKBAYALARMCTRL bayid  settings\n" .
        "\n" .
        "  bayid            Identifier of the disk bay.\n" .
        "  settings (bits)  0x01 = Unrecoverable level\n" .
        "                   0x02 = Critical level\n" .
        "                   0x04 = Non critical level\n" .
        "                   0x08 = Informational level\n" .
        "                   0x10 = Set reminder\n" .
        "                   0x40 = Muted\n" .
        "                   0x80 = Request muted\n";

##############################################################################
# short description
    $help{COUNTS}{SHORT} =
        "Displays the number of everything in the system.";

# long description
    $help{COUNTS}{LONG} =
        "COUNTS\n";

##############################################################################
# short description
    $help{DISKBAYCOUNT}{SHORT} =
        "Displays the number of disk bays bypass cards.";

# long description
    $help{DISKBAYCOUNT}{LONG} =
        "DISKBAYCOUNT\n";

##############################################################################
# short description
    $help{DISKBAYDELETE}{SHORT} =
        "Deletes a disk bay.";

# long description
    $help{DISKBAYDELETE}{LONG} =
        "DISKBAYDELETE id\n" .
        "\n" .
        "  id                  Id of disk bay to delete.\n";

##############################################################################
# short description
    $help{DISKBAYENVIRO}{SHORT} =
        "Displays disk bay environment information.";

# long description
    $help{DISKBAYENVIRO}{LONG} =
        "DISKBAYENVIRO bayid\n" .
        "\n" .
        "  bayid            Identifier of the disk bay.\n";

##############################################################################
# short description
    $help{DISKBAYINFO}{SHORT} =
        "Displays disk bay information.";

# long description
    $help{DISKBAYINFO}{LONG} =
        "DISKBAYINFO bayid\n" .
        "\n" .
        "  bayid            Identifier of the disk bay.\n";

##############################################################################
# short description
    $help{DISKBAYLIST}{SHORT} =
        "Displays a list of disk bay identifiers.";

# long description
    $help{DISKBAYLIST}{LONG} =
        "DISKBAYLIST\n";

##############################################################################
# short description
    $help{DISKBAYS}{SHORT} =
        "Displays disk bay information for all disk bays.";

# long description
    $help{DISKBAYS}{LONG} =
        "DISKBAYS\n";

##############################################################################
# short description
    $help{DISKBAYSTATUS}{SHORT} =
        "Displays diskbays and associated PDisks.";

# long description
    $help{DISKBAYSTATUS}{LONG} =
        "DISKBAYSTATUS\n";

##############################################################################
# short description
    $help{DISKREADWRITE}{SHORT} =
        "Read or write to a physical or virtual disk.";

# long description
    $help{DISKREADWRITE}{LONG} =
        "DISKREADWRITE P/V R/W Filename ID Block NumberBlocks\n".
        "       P/V         - PDISK | VDISK -- do command to PDisk or VDisk\n".
        "       R/W         - READ | WRITE  -- Do a Read or Write\n".
        "       Filename    - Local file name to Read into or Write from\n".
        "       ID          - PID or VID of disk to Read or Write\n".
        "       Block       - Block number of VID or PID to start with\n".
        "       NumberBlocks- Number of blocks to Read or Write\n";

##############################################################################
# short description
    $help{DLMPATHSELECTIONALGO}{SHORT} =
        "Selects/Displays the DLM Path selection algorithm.";

# long description
    $help{DLMPATHSELECTIONALGO}{LONG} =
        "DLMPATHSELECTIONALGO algoType\n" .
        "\n" .
        "  Algorithm Type\n" .
        "                       0 = Selects the round robin algorithm between FC and ICL paths -- default\n" .
        "                       1 = Selects the ICL-50 percent weighted round robin algorithm\n" .
        "                       2 = Selects the ICL-path alone (100 percent ICL.....)\n" .
        "                       3 = Displays the current algorithm\n";

##############################################################################
# short description
    $help{DLMPATHSTATS}{SHORT} =
        "Displays DLM Path stats between controllers .";

# long description
    $help{DLMPATHSTATS}{LONG} =
        "DLMPATHSTATS\n";

##############################################################################
# short description
    $help{DPRINTF}{SHORT} =
        "Gets/Sets the DPRINTF mode bits.";

# long description
    $help{DPRINTF}{LONG} =
        "DPRINTF [flag(s) action]\n" .
        "\n" .
        "  If no parameters specified, DPRINTF \"gets\" the current enabled flags.\n" .
        "\n" .
        "  flag(s):  Comma seperated list of flags.\n" .
        "            ('++' indicates default ON, all others default OFF):\n" .
        "\n" .
        "       CACHE_REF           Caching\n" .
        "    ++ DEFAULT             Default debug messages\n" .
        "    ++ ELECTION            Election\n" .
        "       ETHERNET            Ethernet Driver\n" .
        "       FCALMON             FCAL Monitor\n" .
        "       I2C                 I2C bus\n" .
        "       IPC                 Inter-Processor Communications\n" .
        "       IPC_MSGS            IPC message delivery debug\n" .
        "       MD5                 MD5 signature\n" .
        "       PI_COMMANDS         PI related messages\n" .
        "    ++ PROC_PRINTF         Proc code printf messages\n" .
        "    ++ RM                  Resource Manager\n" .
        "       SES                 SCSI Enclosure Services\n" .
        "       SM_HB               Sequence Manager Heart Beat\n" .
        "       X1_COMMANDS         Display X1 commands as they come down\n" .
        "       X1_PROTOCOL         Display X1 command DATA as it is transferred\n" .
        "       XSSA_DEBUG          XSSA persistant data debug\n" .
        "       ELECTION_VERBOSE    Election (verbose)\n" .
        "       IPMI                IPMI - hardware monitor\n" .
        "       IPMI_VERBOSE        IPMI (verbose)\n" .
        "\n" .
        "       BASIC               Combined bits representing \"default ON/++\" flags\n" .
        "       ALL                 Combined bits representing ALL flags\n" .
        "\n" .
        "  action:\n" .
        "\n" .
        "       ON              Enable the listed flags, other flags unchanged\n" .
        "       OFF             Disable the listed flags, other flags unchanged\n" .
        "       ONLY            Enable the listed flags; Disable all other flags\n" ;

##############################################################################
# short description
    $help{DSPCONN}{SHORT} =
        "Displays a list of available connections.";

# long description
    $help{DSPCONN}{LONG} =
        "DSPCONN\n";

##############################################################################
# short description
    $help{DSPLOGS}{SHORT} =
        "Displays log messages (new interface).";

# long description
    $help{DSPLOGS}{LONG} =
        "DSPLOGS [-A] [-C] [-D] [-B|-E] [-G] [-f filename] [-g reg-exp] [[count] seqNumber]\n\n" .
        "  -A                 Display log event [A]cknowledgement status.\n" .
        "  -C                 Display [C]ustomer logs\n" .
        "  -D                 Display [D]ebug logs\n" .
        "                       -C/-D Default: display ALL logs (merged)\n".
        "  -B                 Display [B]inary data\n" .
        "  -E                 Display [E]xtended data\n" .
        "                       Only -B or -E can be specified, not both at one time\n".
        "  -G                 Display time in [G]MT. Default: Local Time\n" .
        "  -f filename        Name of [f]ile to write output to.\n" .
        "  -g reg-exp         Filter on regular expression (case insensitive; also,\n" .
        "                     don't specify ':'s in the reg-exp, it won't work -- use\n" .
        "                     '.'s instead).\n" .
        "  count              Number of log messages to display. Default: 10\n" .
        "  seqNumber          Starting Master Sequence number if looking at merged\n".
        "                       logs; Sequence number if -C or -D specified.\n";

##############################################################################
# short description
    $help{ENGDEBUG}{SHORT} =
        "Engineering debug pass-through command.";

# long description
    $help{ENGDEBUG}{LONG} =
        "ENGDEBUG [arg1] [arg2] ... [argN]\n";

##############################################################################
# short description
    $help{ENVIIGET}{SHORT} =
        "Displays environmental statistics.";

# long description
    $help{ENVIIGET}{LONG} =
        "ENVIIGET\n";

##############################################################################
# short description
    $help{ERRORTRAP}{SHORT} =
        "Error traps the CCB, FE, or BE.";

# long description
    $help{ERRORTRAP}{LONG} =
        "ERRORTRAP type\n" .
        "\n" .
        "  type (CCB|FE|BE|ALL)\n" .
        "                     CCB - Error trap CCB.\n" .
        "                     FE  - Error trap FE.\n" .
        "                     BE  - Error trap BE.\n" .
        "                     ALL - Error trap CCB, FE, and BE.\n";

##############################################################################
# short description
    $help{EWOKDATACONTROL}{SHORT} =
        "Write client persistent data.";

# long description
    $help{EWOKDATACONTROL}{LONG} =
        "EWOKDATACONTROL opt [recname [offset [length [writedata [flags]]]]\n" .
        "\n" .
        "  opt                option - values\n" .
        "                        0 - nop\n".
        "                        1 - create\n".
        "                        2 - remove\n".
        "                        3 - read\n".
        "                        4 - write\n".
        "                        5 - list\n".
        "  recname            record name\n".
        "  offset             offset for read/write\n".
        "  length             data length\n".
        "  writedata          data to write\n".
        "  flags              flags - values\n".
        "                        Bit 0 - lock\n".
        "                        Bit 1 - unlock\n".
        "                        Bit 2 - trunc\n";

##############################################################################
# short description
    $help{FAILUREMANAGERTEST}{SHORT} =
        "Sends Failure to Failure Manager.";

# long description
    $help{FAILUREMANAGERTEST}{LONG} =
        "FAILUREMANAGERTEST action type serialnum [interfaceID]\n" .
        "\n" .
        "  action (F|U)       One of: F, or U.\n" .
        "                     F -  Fail.\n" .
        "                     U  - Unfail.\n" .
        "  type (CONT|INT)    One of: CONT, or INT.\n" .
        "                     CONT - action serialnum.\n" .
        "                     INT  - action interfaceID on Controller serialnum.\n" .
        "  serialnum          Serial number of controller.\n" .
        "  interface          Interface on controller (Required for type INT).\n";

##############################################################################
# short description
    $help{FAILURESTATESET}{SHORT} =
        "Set the failure state of a controller.";

# long description
    $help{FAILURESTATESET}{LONG} =
        "FAILURESTATESET serialNumber state\n" .
        "\n" .
        "  serialNumber       Serial number of the controller.\n" .
        "  state              Failure state to set.\n" .
        "                       0 = FD_STATE_UNUSED\n" .
        "                       1 = FD_STATE_FAILED\n" .
        "                       2 = FD_STATE_OPERATIONAL\n" .
        "                       3 = FD_STATE_POR\n" .
        "                       4 = FD_STATE_ADD_CONTROLLER_TO_VCG\n" .
        "                       5 = FD_STATE_STRANDED_CACHE_DATA\n" .
        "                       6 = FD_STATE_FIRMWARE_UPDATE_INACTIVE\n" .
        "                       7 = FD_STATE_FIRMWARE_UPDATE_ACTIVE\n" .
        "                       8 = FD_STATE_UNFAIL_CONTROLLER\n" .
        "                       9 = FD_STATE_VCG_SHUTDOWN\n";

##############################################################################
# short description
    $help{FCMCOUNTERTEST}{SHORT} =
        "Tests the FCAL counter functionality.";

# long description
    $help{FCMCOUNTERTEST}{LONG} =
        "FCMCOUNTERTEST parm1\n" .
        "\n" .
        "  parm1 (value)\n" .
        "                       0     - Dump FCAL counters to serial port\n" .
        "                       1     - Get baseline FCAL counters\n" .
        "                       2     - Update FCAL counters\n" .
        "                       3     - Calculate delta values\n" .
        "                       4     - (TASK) Simulate major FCAL event\n" .
        "                       5     - (TASK) Simulate minor FCAL event\n";

##############################################################################
# short description
    $help{FIDDECODE}{SHORT} =
        "Decode a binary FID file (i.e. from a snapshot).";

# long description
    $help{FIDDECODE}{LONG} =
        "FIDDECODE [-f output-file] [-i fid-id] input-file\n" .
        "\n" .
        "  -f output-file    Name of file to write output to.\n" .
        "  -i fid-id         FID to decode (extracts from 'input-file' if possible).\n" .
        "  input-file        Name of file to read input from.\n" ;

##############################################################################
# short description
    $help{FIDREAD}{SHORT} =
        "Read the specified file ID (both physical and logical).";

# long description
    $help{FIDREAD}{LONG} =
        "FIDREAD [-t byte|short|word|binary] [-S] [-f filename] [-l length] FID\n" .
        "\n" .
        "  -t byte|short|word|binary|fmt   Format of output data (default: fmt).\n" .
        "  -S                 Skip over 32 byte FID Header (binary mode only).\n" .
        "  -f filename        Name of output file.  Undecoded FIDs are BZip2 (.tbz)\n" .
        "  -l length          Number of bytes to display (default: entire FID).\n" .
        "  -b base for fid 6  Memory address for fid 6 if not 0922\n" .
        "  FID                File ID to read.\n" .
        "\n" .
        "  FID    Description\n" .
        "-------- -----------\n" .
        "  0      FID Directory\n" .
        "  1      Device Label\n" .
        "  2      Back End NVRAM\n" .
        "  3      Front End NVRAM\n" .
        "  4      (unused)\n" .
        "  5      Back End Work Area\n" .
        "  6      Master Config\n" .
        "  7      Controller Map\n" .
        "  8      Comm Area\n" .
        "  9-10   (unused)\n" .
        " 11      Copy NVRAM\n" .
        " 12-18   (unused)\n" .
        " 19      Resource Manager Config 1\n" .
        " 20      Resource Manager Config 2\n" .
        " 21      Resource Manager Log 1\n" .
        " 22      Resource Manager Log 2\n" .
        " 23      Snapshot Directory\n" .
        " 24-55   Master Config Checkpoints\n" .
        " 56-87   Controller Map Checkpoints\n" .
        " 88-119  BE NVRAM Checkpoints\n" .
        "120-255  (unused)\n" .
        "\n" .
        "Logical FIDS:\n" .
        "CCB:                             FE / BE:\n" .
        "--------------------------      -------------------------------------------\n" .
        "256  CCB Trace Buffer           512 / 768  Flight recorder w/o timestamps\n" .
        "257  CCB Serial Buffer          513 / 769  Flight recorder w/timestamps\n" .
        "258  CCB Heap Stats             514 / 770  MRP trace\n" .
        "259  CCB Profile Dump           515 / 771  Defrag trace\n" .
        "260  CCB PCB Dump               516 / 772  Error Trap registers\n" .
        "261  Raw Custmr log flash       517 / 773  Error Trap internal ram\n" .
        "262  Raw Debug log flash        518 / 774  Error Trap NMI counts\n" .
        "263  Cache DiskBayMap           519 / 775  Error Trap internal registers\n" .
        "264  Cache DiskBayPaths         520 / 776  PROC internal information (K_ii)\n" .
        "265  Cache PDiskMap             521 / 777  Initiator trace log 0\n" .
        "266  Cache PDiskFailMap         522 / 778  Initiator trace log 1\n" .
        "267  Cache PDiskRebuildMap      523 / 779  Initiator trace log 2\n" .
        "268  Cache PDiskPaths           524 / 780  Initiator trace log 3\n" .
        "269  Cache VDiskMapP            525 / 781  FE Trace log 0\n" .
        "270  Cache VDiskCopyMap         526 / 782  FE Trace log 1\n" .
        "271  Cache VDiskMirrorMapP      527 / 783  FE Trace log 2\n" .
        "272  Cache RaidMap              528 / 784  FE Trace log 3\n" .
        "273  Cache ServerMap            529 / 785  Physical exec queue\n" .
        "274  Cache TargetMap            530 / 786  Raid exec queue\n" .
        "275  Cache DiskBays             531 / 787  Raid 5 exec queue\n" .
        "276  Cache Targets              532 / 788  Virtual Device exec queue\n" .
        "277  Cache FELoopStats          533 / 789  Define exec queue\n" .
        "278  Cache BELoopStats          534 / 790  Raid init exec queue\n" .
        "279  Cache PDisks               535 / 791  Raid XOR completion exec queue\n" .
        "280  Cache VDisks               536 / 792  Raid XOR exec queue\n" .
        "281  Cache Raids                537 / 793  ISP 0 request queue\n" .
        "282  Cache Servers              538 / 794  ISP 1 request queue\n" .
        "283  UNUSED (was I2C stats)     539 / 795  ISP 2 request queue\n" .
        "284  Stats Proc (FE & BE)       540 / 796  ISP 3 request queue\n" .
        "285  Stats PCI (FE & BE)        541 / 797  ISP 0 response queue\n" .
        "286  Stats Environmental        542 / 798  ISP 1 response queue\n" .
        "287  Stats Servers              543 / 799  ISP 2 response queue\n" .
        "288  Stats VDisks               544 / 800  ISP 3 response queue\n" .
        "289  Stats Cache Devices        545 / 801  Diagnostic data NVRAM Part 5\n" .
        "290  Stats Loop (FE & BE)       546 / 802  Raid error exec queue\n" .
        "291  Stats FCAL Counters        547 / 803  Raid error exec PCB\n" .
        "292  Command Record Table       548 / 804  Physical comp queue\n" .
        "293  CCB NVRAM                  549 / 805  Raid error exec PCB\n" .
        "294  XSSA Persistent Store      550 / 806  FE IRAM\n" .
        "295  Proc DDR Tables            551 / 807  BE IRAM\n" .
        "296  FW Versions                552 / 808  Physical exec PCB\n" .
        "297  Timestamp                  553 / 809  Raid exec PCB\n" .
        "298  CCB NVRAM Flash Copies     554 / 810  Raid 5 exec PCB\n" .
        "299  FCM Counters               555 / 811  Virtual Device exec PCB\n" .
        "300  VCG Info                   556 / 812  Define exec PCB\n" .
        "301  Active Servers             557 / 813  Raid init exec PCB\n" .
        "302  FID List                   558 / 814  Raid XOR completion exec PCB\n" .
        "303  CCB Statistics             559 / 815  Raid XOR exec PCB\n" .
        "304  Mirror Partner List        560 / 816  File System exec queue\n" .
        "305  Wookiee Linux File R/W     561 / 817  File System exec PCB\n" .
        "306  Wookiee PAM Logs           562 / 818  Backtrace data NVRAM Part 1\n" .
        "307  Wookiee Linux System Logs  563 / 819  FW Init Control Block (FICB)\n" .
        "308  Wookiee Linux Raid Logs    564 / 820  Profile Data\n" .
        "309  Wookiee Core Summaries     565 / 821  ISP0 ATIO queue\n" .
        "310  Wookiee Core Files         566 / 822  ISP1 ATIO queue\n" .
        "311  Wookiee Apps Logs          567 / 823  ISP2 ATIO queue\n" .
        "312  750 SMP PHY Info Logs      568 / 824  ISP3 ATIO queue\n" .
        "313  Wookiee Qlogic Cores\n".
        "353  Bay SES Data\n" .
        "354  iSCSI Stats\n" .
        "355  Async Rep\n" .
        "\n" .
        "1024  NVRAM Part 2 % usage -- master only for reserved size\n";

##############################################################################
# short description
    $help{FIDWRITE}{SHORT} =
        "Write the specified file ID (multi-pkt, physical FIDs only).";

# long description
    $help{FIDWRITE}{LONG} =
        "FIDWRITE [-N] FID file\n" .
        "\n" .
        "  -N                 Write the file with NO header\n" .
        "  FID                File ID to write.\n" .
        "  file               binary data file to write to specified FID.\n";

##############################################################################
# short description
    $help{FIOMAPTEST}{SHORT} =
        "Tests the FIO map control functionality.";

# long description
    $help{FIOMAPTEST}{LONG} =
        "FIOMAPTEST parm1 parm2 parm3\n" .
        "\n" .
        "  parm1 (value)\n" .
        "                       0     - Work with read map\n" .
        "                       1     - Work with write map\n" .
        "  parm2 (value)\n" .
        "                       0     - Reset\n" .
        "                       1     - Clear\n" .
        "                       2     - Set (uses parm3)\n" .
        "  parm3 (value)\n";

##############################################################################
# short description
    $help{FOREIGNTARGET}{SHORT} =
        "Turn Foreign Target on/off";

# long description
    $help{FOREIGNTARGET}{LONG} =
        "FOREIGNTARGET on/off\n";

##############################################################################
# short description
    $help{ISEIPS}{SHORT} =
        "Get IPs for ISE";

# long description
    $help{ISEIPS}{LONG} =
        "ISEIPS Get IPs for ISE\n";

##############################################################################
# short description
    $help{EMULATEPAB}{SHORT} =
        "Emulate PAB for BAY/ISE off/on/count (0,1,2)";

# long description
    $help{EMULATEPAB}{LONG} =
        "EMULATEPAB ISE#(or Bay#) on/off/count [1,0,2]\n" .
        "example: emulatepab 8 on     -- turn PAB on for 300 seconds\n" .
        "example: emulatepab 8 1\n" .
        "example: emulatepab 8 off   -- turn emulated PAB (or real PAB off)\n";

##############################################################################
# short description
    $help{SWAPPIDS}{SHORT} =
        "SWAP two PIDS";

# long description
    $help{SWAPPIDS}{LONG} =
        "SWAPPIDS pid1 pid2\n";

##############################################################################
# short description
    $help{FWUPDATE}{SHORT} =
        "Update firmware on the CCB or Proc.";

# long description
    $help{FWUPDATE}{LONG} =
        "FWUPDATE filename\n" .
        "\n" .
        "  filename          The name of the file to send to the CCB.\n" .
        "                    Firmware Kits can be specified as well (\".fwk\")\n";

##############################################################################
# short description
    $help{FWSYSREL}{SHORT} =
        "Displays firmware system release information.";

# long description
    $help{FWSYSREL}{LONG} =
        "FWSYSREL\n";

##############################################################################
# short description
    $help{FWVERSION}{SHORT} =
        "Displays firmware version information.";

# long description
    $help{FWVERSION}{LONG} =
        "FWVERSION\n";

##############################################################################
# short description
    $help{GLOBALCACHEINFO}{SHORT} =
        "Displays global cache information.";

# long description
    $help{GLOBALCACHEINFO}{LONG} =
        "GLOBALCACHEINFO\n";

##############################################################################
# short description
    $help{GLOBALCACHESET}{SHORT} =
        "Set the global caching mode.";

# long description
    $help{GLOBALCACHESET}{LONG} =
        "GLOBALCACHESET mode\n" .
        "\n" .
        "  mode               Caching mode.\n" .
        "                       OFF = Caching disabled\n" .
        "                       ON = Caching enabled\n";

##############################################################################
# short description
    $help{GENFUNCTION}{SHORT} =
        "Call PI_GenFunction with passed in parameters.";

# long description
    $help{GENFUNCTION}{LONG} =
        "GENFUNCTION [[parameter1]..[parameter8]]\n" .
        "\n" .
        "  parameterN        Integral parameters that are passed to PI_GenFunction().\n";

##############################################################################
# short description
    $help{GENMRP}{SHORT} =
        "Issue a generic MRP.";

# long description
    $help{GENMRP}{LONG} =
        "GENMRP [-t byte|short|word] [-o byte|short|word] mrpCmd rspDataLn [data]\n" .
        "\n" .
        "  -t byte|short|word   Format of input data field (default: word).\n" .
        "  -o byte|short|word   Format of output data (default: word).\n" .
        "  mrpCmd             The MRP command code.\n" .
        "  rspDataLn          The MRP response data length.\n" .
        "  data               The MRP input data packet (ex. F234A034C756FFFE).\n" .
        "                       Leave blank if no input data.\n";

##############################################################################
# short description
    $help{GETBETYPE}{SHORT} =
        " Get backend type information.";
# long description
    $help{GETBETYPE}{LONG} =
        " Get backend type information.\n".
        "        Type             Backend Type.\n".
        "                         0  -   Loop\n".
        "                         1  -   Fabric\n";

##############################################################################
# short description
    $help{GETCPUCOUNT}{SHORT} =
        " Get cpu count information.";
# long description
    $help{GETCPUCOUNT}{LONG} =
        " Get cpu count information of the controller";

##############################################################################
# short description
    $help{GETGEOLOCATION}{SHORT} =
        "Displays the Geo location info of the bay(s) and associated PDisks.";

# long description
    $help{GETGEOLOCATION}{LONG} =
        "GETGEOLOCATION\n";

##############################################################################
# short description
    $help{GETNAME}{SHORT} =
        "Get the name associated with a VLink related component.";

# long description
    $help{GETNAME}{LONG} =
        "GETNAME component_fid  component_id\n" .
        "\n" .
        "  component_fid      VDISK=10, VLINKRMTCTRL=12, CONTROLLER=13, VLINKRMTVDISK=18\n" .
        "  component_id       ID for the component\n";

##############################################################################
# short description
    $help{GETREPORT}{SHORT} =
        "Retrieves various debug reports.";

# long description
    $help{GETREPORT}{LONG} =
        "GETREPORT [-l link-map] report outfile\n" .
        "\n" .
        "  report             One of: HEAP, TRACE, PCB, PROFILE, or PACKET.\n" .
        "  outfile            File to write output data to (optional for HEAP)\n" .
        "  -l filename        Name of CCB map file to use when processing the\n".
        "                       report.  This is optional, and only applicable\n".
        "                       to PCB and PROFILE report types.\n";

##############################################################################
# short description
    $help{GETTIME}{SHORT} =
        "Get the controller time.";

# long description
    $help{GETTIME}{LONG} =
        "GETTIME\n";

##############################################################################
# short description
    $help{GETVDISKREDUNDANCY}{SHORT} =
        "Displays whether the given virtual disk is bay redundant or not.";

# long description
    $help{GETVDISKREDUNDANCY}{LONG} =
        "GETVDISKREDUNDANCY vdiskid\n" .
        "\n" .
        "  vdiskid            Virtual disk identifier.\n";

##############################################################################
# short description
    $help{GETWORKSETINFO}{SHORT} =
        "Displays workset information.";

# long description
    $help{GETWORKSETINFO}{LONG} =
        "GETWORKSETINFO worksetID\n" .
        "\n" .
        "  worksetID    Workset identifier (ALL or 0xFFFF for all worksets).\n";

##############################################################################
# short description
    $help{GLOBALVDISKPRIORITY}{SHORT} =
        "Enable VDisk Priority feature.";
# long description
    $help{GLOBALVDISKPRIORITY}{LONG} =
        "GLOBALVDISKPRIORITY     Mode\n".
        "  Mode               ON/OFF -\n" .
        "                     ON     Turn the feature ON\n" .
        "                     OFF    Turn the feature OFF\n" .
        "                     STATUS Current State\n";

##############################################################################
# short description
    $help{HELP}{SHORT} =
        "Provides help information for ccbCL.pl commands.";

# long description
    $help{HELP}{LONG} =
        "HELP [-H] [command or partial command]\n" .
        "\n" .
        "  -H                 Generates an HTML document containing all of\n" .
        "                     the ccbCL commands. The output file is named\n" .
        "                     \"ccbCL.html\" in the same directory that\n" .
        "                     ccbCL.pl is located.\n" .
        "\n" .
        "  command            Displays help information on that command.\n" .
        "\n" .
        "                     If '*' is passed in for command, brief help\n" .
        "                     for ALL available commands is displayed.\n" .
        "\n" .
        "An alternate method of obtaining help on a particular command is to\n" .
        "specify the '-H' switch after the command name (except for HELP --.\n" .
        "see above).\n" .
        "\n" .
        "ccbCL.pl Command Line Parameters:\n" .
        "---------------------------------\n" .
        "\n" .
        "  ip-addresse(s)    - These controllers will be connected to upon startup.\n" .
        "  command file(s)   - These command files will be executed at startup.\n" .
        "  -c                - enable colorized output (different color per\n" .
        "                      connection).\n" .
        "  -r                - Use the 'readline' library on Linux\n" .
        "  -H                - Pretend to use the readline library on Linux (prompts, etc.)\n" .
        "  -e \"commands\"     - A ';' seperated list of commands that will be\n" .
        "                      executed at startup. ccbCL.pl will then terminate.\n" .
        "  -E \"commands\"     - like -e, but no verbose output when run.\n" .
        "\n" .
        "\"ccbCL.ini\" Initialization File:\n" .
        "--------------------------------\n" .
        "\n" .
        "  Certain defaults can be overridden by setting them in this file.\n" .
        "  Upon ccbCL.pl startup, it first searches for ccbCL.ini in c:/,\n" .
        "  (~/.ccbCl.ini on Linux) then checks for it in the ccbCL.pl startup\n" .
        "  directory.\n" .
        "\n" .
        "  To set the default login port (3000, 3100, 3200):\n" .
        "    PORT=3200\n" .
        "\n" .
        "  To set the '-c' colorization option (yes, no):\n" .
        "    COLOR=yes\n" .
        "\n" .
        "  To set the order in which the colors are assigned. Color choices:\n" .
        "    \"WHITE, YELLOW, LIGHTBLUE, BROWN,\n" .
        "     CYAN, GRAY, LIGHTMAGENTA, LIGHTCYAN,\n" .
        "     LIGHTGREEN, LIGHTRED, GREEN, BLUE,\n" .
        "     MAGENTA, RED, BLACK\"\n" .
        "\n" .
        "    COLORORDER=WHITE, YELLOW, LIGHTBLUE, BROWN, CYAN, GRAY (etc)\n" .
        "\n" .
        "  To set the background color (same color choices as for COLORORDER):\n" .
        "  Note: if you choose WHITE as your background, you will probably want to\n" .
        "  change COLORORDER as well. Also, on Windows, you will have to change the\n" .
        "  background color of your DOS window through the \"Properties\" menu item to\n" .
        "  match what you select here.\n" .
        "\n" .
        "    BACKGROUND_COLOR=BLACK\n" .
        "\n" .
        "  To set the default map file to use with the MEMREAD command\n" .
        "  set the appropriate variable to the desired file path:\n" .
        "\n" .
        "    CCBMAP=K:\\Release\\Bigfoot\\Corona\\perf\\C220\\ccb\\Build\\CCBRun.map\n".
        "    BEMAP=K:\\Release\\Bigfoot\\Corona\\perf\\C220\\proc\\BuildBE\\BERun.map\n".
        "    FEMAP=K:\\Release\\Bigfoot\\Corona\\perf\\C220\\proc\\BuildFE\\FERun.map\n".
        "\n" .
        "Basic \"Command Line\" usage:\n" .
        "---------------------------\n" .
        "\n" .
        "* A command is defined as:\n" .
        "\n" .
        "  [List of connection id's :] THECOMMAND [-option(s)] parameters\n" .
        "\n" .
        "  \"List of connection id's\": a comma seperated list of\n" .
        "        connection identifiers (as displayed by DSPCONN) that the\n" .
        "        following command will be executed on. \"A\" or \"ALL\" can be\n" .
        "        used to indicate that the command should be executed on all\n" .
        "        controllers. This field is optional; if it is specified, it\n" .
        "        must be seperated from THECOMMAND by a colon \":\".\n" .
        "\n" .
        "  THECOMMAND: \"Partial\" commands can be entered -- only enough\n" .
        "        letters to make the command unique are required.\n" .
        "\n" .
        "  -option(s): hyphened options (if used) must follow THECOMMAND,\n" .
        "        before required parameters are listed (command dependent).\n" .
        "\n" .
        "  parameters: 0 or more required parameters (command dependent)\n" .
        "\n" .
        "* Multiple commands can be entered on a single line when seperated by\n" .
        "        a semi-colon \";\".\n" .
        "\n" .
        "* Command scripts (files) can be written following the same rules as\n" .
        "        if the commands were interactively entered. They can be listed\n" .
        "        as optional parameters when ccbCL.pl is invoked and will be\n" .
        "        executed in the order they are listed.  They can also be\n" .
        "        executed with the RUN command.\n" .
        "\n" .
        "* STDOUT redirection: STDOUT can be redirected to a file on most\n" .
        "        commands with the \">\" and \">>\" operators as in a standard\n" .
        "        shell.  This is not implemented for all commands however.\n" .
        "\n" .
        "* Example:\n" .
        "\n" .
        "  0,1,3:FWV; all:POWERUPSTATE; 2:VCGINFO > afile\n" .
        "\n" .
        "  This command will issue a FWVERSION command on connections 0, 1\n" .
        "  and 3, followed by a POWERUPSTATE on ALL connected controllers,\n" .
        "  followed by a VCGINFO on connection 2.  All output is redirected\n" .
        "  and captured in file \"afile\"\n" .
        "\n";

##############################################################################
# Name:     iddInfo
#
# Desc:     Get IDD info
# short description
    $help{IDDINFO}{SHORT} =
        "Get IDD info";

# long description
    $help{IDDINFO}{LONG} =
        "IDDINFO\n" .
        "                     Get IDD info.\n";

##############################################################################
# short description
    $help{INITCCBNVRAM}{SHORT} =
        "Initialize CCB NVRAM.";

# long description
    $help{INITCCBNVRAM}{LONG} =
        "INITCCBNVRAM [type]\n" .
        "  type               Type of initialization.\n" .
        "                       FULL    = init all nvram  (default)\n" .
        "                       LICENSE = init license flag ONLY\n";

##############################################################################
# short description
    $help{INITPROCNVRAM}{SHORT} =
        "Initialize PROC NVRAM.";

# long description
    $help{INITPROCNVRAM}{LONG} =
        "INITPROCNVRAM\n" .
        "  mode               Init mode.\n" .
        "                       ALL = init all nvram (default)\n" .
        "                       FE  = init FE NVA Records ONLY!\n" .
        "                       NMI = init NMI counts ONLY\n" .
        "                       BE  = init BE NVA Records ONLY!\n";

##############################################################################
# short description
    $help{INTERFACEFAIL}{SHORT} =
        "Fails an interface.";

# long description
    $help{INTERFACEFAIL}{LONG} =
        "INTERFACEFAIL serialnum interfaceID\n" .
        "\n" .
        "  serialnum          Serial number of controller.\n" .
        "  interface          Interface on controller.\n";

##############################################################################
# short description
    $help{INTERFACEUNFAIL}{SHORT} =
        "Unfails an interface.";

# long description
    $help{INTERFACEUNFAIL}{LONG} =
        "INTERFACEUNFAIL serialnum interfaceID\n" .
        "\n" .
        "  serialnum          Serial number of controller.\n" .
        "  interface          Interface on controller.\n";

##############################################################################
# short description
    $help{IPGET}{SHORT} =
        "Get the controller's Serial Number, IP address, Subnet Mask, and Gateway.";

# long description
    $help{IPGET}{LONG} =
        "IPGET\n";

##############################################################################
# short description
    $help{IPSET}{SHORT} =
        "Set the controller IP address, Subnet Mask, and Gateway.";

# long description
    $help{IPSET}{LONG} =
        "IPSET serNum ipAdr subMask gateway\n" .
        "\n" .
        "  serNum         Controller serial number to change.\n" .
        "\n" .
        "  ipAdr          New Controller IP address.\n" .
        "  subMask        New Controller Subnet Mask address.\n" .
        "  gateway        New Controller Gateway address.\n" .
        "\n" .
        "                 Valid inputs for each of the above (ipAdr, subMask, gateway):\n" .
        "                     Any valid ip address.\n";

##############################################################################
# short description
    $help{ISEBEACON}{SHORT} =
        "Beacon ISE Bay Component Light_on_off.";

# long description
    $help{ISEBEACON}{LONG} =
        "ISEBEACON bayid component light_on_off\n".
        "       bayid     -  ISE bay id\n".
        "       component -  type: 0 = beacon ISE chassis\n".
        "                          1 = beacon ISE mrc\n".
        "                          2 = beacon ISE power supply\n".
        "                          3 = beacon ISE battery pack\n".
        "                          4 = beacon ISE DataPac\n".
        "                          5 = beacon ISE SFP\n".
        "                          6 = beacon ISE CAP\n".
        "                          7 = beacon ISE Bezel\n".
        "       subcomponent- type:1 = 1st component\n".
        "                          2 = 2nd component\n".
        "       light_on_off:      0 = turn light off\n".
        "                          1 = turn light on\n";

##############################################################################
# short description
    $help{ISESTATUS}{SHORT} =
        "Display ISE Bay status.";

# long description
    $help{ISESTATUS}{LONG} =
        "ISESTATUS [-B] [bayid] [component,component,...]\n".
        "       -B           Display only Status line on component(s)\n".
        "NOTE: must be capital B, lower case letters require an argument.\n".
        "       bayid     -  ISE bay id\n".
        "       component -  ise | bay     = ISE chassis\n".
        "                    mrc           = ISE mrc\n".
        "                    ps | power    = ISE power supply\n".
        "                    bat           = ISE battery pack\n".
        "                    datapac | pac = ISE DataPac\n";

##############################################################################
# short description
    $help{ISCSICHAPGETINFO}{SHORT} =
        "Display CHAP User information of an ISCSI Target.";

# long description
    $help{ISCSICHAPGETINFO}{LONG} =
        "ISCSICHAPGETINFO tid\n" .
        "\n" .
        "  tid                  Target Identifier\n";

##############################################################################
# short description
    $help{ISCSICHAPSETINFO}{SHORT} =
        "Configure CHAP user information on an ISCSI Target.";

# long description
    $help{ISCSICHAPSETINFO}{LONG} =
        "ISCSICHAPSETINFO option tid serverName secret1 secret2\n" .
        "\n" .
        "  option               Option values:\n".
        "                       0 - Add/Modify\n".
        "                       1 - Remove\n".
        "  tid                  Target Identifier\n".
        "  serverName           iSCSI Server Name string\n".
        "  secret1              secret for 1-way CHAP\n".
        "  secret2              secret for 2-way CHAP\n";

##############################################################################
# short description
    $help{ISCSIGETINFO}{SHORT} =
        "Display information of an ISCSI Target.";

# long description
    $help{ISCSIGETINFO}{LONG} =
        "ISCSIGETINFO tid\n" .
        "\n" .
        "  tid                  Target Identifier\n";

##############################################################################
# short description
    $help{ISCSISETINFO}{SHORT} =
        "Configure an iSCSI negotiable parameter on a target.";

# long description
    $help{ISCSISETINFO}{LONG} =
        "ISCSISETINFO tid paramId paramVal\n" .
        "\n" .
        "  tid                     Target Identifier\n" .
        "  paramId                 Parameter Id\n" .
        "                             Values -\n" .
        "                              0  ip\n" .
        "                              1  subnetMask\n" .
        "                              2  gateway\n" .
        "                              3  maxConnections\n" .
        "                              4  initialR2T\n" .
        "                              5  immediateData\n" .
        "                              6  dataSequenceInOrder\n" .
        "                              7  dataPDUInOrder\n" .
        "                              8  ifMarker\n" .
        "                              9  ofMarker\n" .
        "                             10  errorRecoveryLevel\n" .
        "                             11  targetPortalGroupTag\n" .
        "                             12  maxBurstLength\n" .
        "                             13  firstBurstLength\n" .
        "                             14  defaultTime2Wait\n" .
        "                             15  defaultTime2Retain\n" .
        "                             16  maxOutstandingR2T\n" .
        "                             17  maxRecvDataSegmentLength\n" .
        "                             18  ifMarkInt\n" .
        "                             19  ofMarkInt\n" .
        "                             20  headerDigest\n" .
        "                             21  dataDigest\n" .
        "                             22  authMethod\n" .
        "                             23  MTU size\n" .
        "                             24  Target Alias\n\n" .
        "  paramVal                Parameter Value\n";

##############################################################################
# short description
    $help{ISCSISTATS}{SHORT} =
        "Display Session information.";

# long description
    $help{ISCSISTATS}{LONG} =
        "ISCSISTATS tid\n" .
        "\n" .
        "  tid                  Target Identifier\n";

##############################################################################
# short description
    $help{ISCSISTATSSERVER}{SHORT} =
        "Display Session information.";

# long description
    $help{ISCSISTATSSERVER}{LONG} =
        "ISCSISTATSSERVER sname\n" .
        "\n" .
        "  sname                  iSCSI Server Name\n";

##############################################################################
# short description
    $help{ISNSGETINFO}{SHORT} =
        "Get iSNS information.";

# long description
    $help{ISNSGETINFO}{LONG} =
        "ISNSGETINFO\n" .
        "\n" .
                                " Get the iSNS information of the controller\n";

##############################################################################
# short description
    $help{ISNSSETINFO}{SHORT} =
        "Set iSNS information.";

# long description
    $help{ISNSSETINFO}{LONG} =
        "ISNSSETINFO flags ip1 port1 proto1 ip2 port2 proto2 ...\n" .
        "\n" .
        "  flags          iSNS enable/disable option\n".
                    "                         0 - disable\n".
                    "                         1 - enable\n".
                    "                         2 - auto disable\n".
                    "                         3 - auto enable\n".
        "  ip             iSNS server ip\n".
        "  port           tcp/udp port to use\n".
        "  proto          Protocol to use to connect to the Server\n".
                    "                         0 - tcp (default)\n".
                    "                         1 - udp\n";

##############################################################################
# short description
    $help{JUMPERS}{SHORT} =
        "Issue generic MRP's to get status of Qlogic J2 jumpers.";

# long description
    $help{JUMPERS}{LONG} =
        "JUMPERS\n" .
        "  Issue generic MRP's to get status of Qlogic J2 jumpers.\n";

##############################################################################
# short description
    $help{KEEPALIVETEST}{SHORT} =
        "Tests the keepAlive mode functionality.";

# long description
    $help{KEEPALIVETEST}{LONG} =
        "KEEPALIVETEST flag slot\n" .
        "\n" .
        "  flag (value)\n" .
        "                       0     - Reset all keepAlive data\n" .
        "                       1     - Clear keepAlive for slot\n" .
        "                       2     - Set keepAlive for slot\n" .
        "                       3     - Disable keepAlive system\n" .
        "                       4     - Enable keepAlive system\n" .
        "  slot (value)\n";

##############################################################################
# short description
    $help{LEDCONTROL}{SHORT} =
        "Set or Clear controller LEDs.";

# long description
    $help{LEDCONTROL}{LONG} =
        "LEDCONTROL led value\n" .
        "\n" .
        "  led                led to control\n" .
        "                     ATTN - Front panel attention LED.\n" .
        "  value              value to set led.\n" .
        "                     0 = off.\n" .
        "                     1 = on.\n" .
        "\n" .
        "750\n" .
        "LEDCONTROL led value [blink]\n" .
        "\n" .
        "  led                led to control\n" .
        "                     0     = Attention\n" .
        "                     1     = Status\n" .
        "                     2     = Session\n" .
        "                     3     = Beacon\n" .
        "                     4     = Beacon Other\n" .
        "                     0xFF  = Kick off led_control test\n" .
        "  value              value to set led.\n" .
        "                     0 = off.\n" .
        "                     1 = on ok.\n" .
        "                     2 = on warn.\n" .
        "                     3 = on error.\n" .
        "  blink              blink rate to use.\n" .
        "                     0 = solid [default].\n" .
        "                     1 = slow.\n" .
        "                     2 = fast.\n";

##############################################################################
# short description
    $help{LET}{SHORT} =
        "Assign a local variable in a command file.";

# long description
    $help{LET}{LONG} =
        "LET \$varname= some value\n" .
        "\n" .
        "  \$varname          Variable names always begin with a \$.\n";

##############################################################################
# short description
    $help{LINUXFILEREAD}{SHORT} =
        "Reads the specified file from the Wookiee Controller.";

# long description
    $help{LINUXFILEREAD}{LONG} =
        "LINUXFILEREAD [-f filename] linuxFileName".
        "\n" .
        "  -f filename        Name of file to write output to.\n" .
        "\n" .
        "Note: Returns a .bz2 file (Tar/Compressed).\n" .
        "      You will need bunzip2 or 7-Zip to extract.\n";

##############################################################################
# short description
    $help{LOGACKNOWLEDGE}{SHORT} =
        "Acknowledge a log event.";

# long description
    $help{LOGACKNOWLEDGE}{LONG} =
        "LOGACKNOWLEDGE sequenceNumber\n" .
        "\n" .
        "  sequenceNumber     Log Sequence Number.\n";

##############################################################################
# short description
    $help{LOGCLEAR}{SHORT} =
        "Clear the log entries.";

# long description
    $help{LOGCLEAR}{LONG} =
        "LOGCLEAR\n";

##############################################################################
# short description
    $help{LOGEVENT}{SHORT} =
        "Logs an event to the CCB.";

# long description
    $help{LOGEVENT}{LONG} =
        "LOGEVENT data_2_log\n";

##############################################################################
# short description
    $help{LOGINFO}{SHORT} =
        "Displays log messages (classic interface).";

# long description
    $help{LOGINFO}{LONG} =
        "LOGINFO [-f filename][-A][-G] count mode sequenceNumber\n" .
        "\n" .
        "  count              Number of log messages to display.\n" .
        "                     Default = 10\n" .
        "  mode               Mode in which to transport logs.\n" .
        "                     Bit 0 = 0  - Text Mode.\n" .
        "                     Bit 0 = 1  - Binary Mode.\n" .
        "                     Bit 1 = 0  - No Extended Data.\n" .
        "                     Bit 1 = 1  - Extended Data (text mode).\n" .
        "                     Bit 2 = 0  - Ignore Sequence Number.\n" .
        "                     Bit 2 = 1  - Use Sequence Number.\n" .
        "                     Bit 3 = 0  - User logs.\n" .
        "                     Bit 3 = 1  - Debug logs.\n" .
        "                     Bit 4 = 0  - Normal logs.\n" .
        "                     Bit 4 = 1  - Merge Customer and Debug logs.\n" .
        "                     Default = 0x10.\n" .
        "  sequenceNumber     Starting sequence number.\n" .
        "  -f filename        Name of file to write output to.\n" .
        "  -A                 Display log event Acknowledgement status.\n" .
        "  -G                 Display time in GMT.\n";

##############################################################################
# short description
    $help{LOGTEXTMESSAGE}{SHORT} =
        "Send a text log message to the CCB.";

# long description
    $help{LOGTEXTMESSAGE}{LONG} =
        "LOGTEXTMESSAGE [ -v type] message\n" .
        "\n" .
        "  -v type            0   INFO.\n" .
        "                     1   WARNING.\n" .
        "                     2   ERROR.\n" .
        "                     3   DEBUG(default).\n" .
        "\n" .
        "  message            Text message to send.\n";

##############################################################################
# short description
    $help{LOOPPRIMITIVE}{SHORT} =
        "Issue a loop primitive to the BE or FE.";

# long description
    $help{LOOPPRIMITIVE}{LONG} =
        "LOOPPRIMITIVE  processor  option  id  port  lid\n" .
        "\n" .
        "  processor      Issue primitive to: FE|BE\n" .
        "  option         options -\n" .
        "                    0 - LIP Reset loop       = LIP(FF,al_ps)\n" .
        "                    1 - LIP Reset lid port   = LIP(al_pd,al_ps)\n" .
        "                    2 - LIP Reset sid or pid = LIP(al_pd,al_ps)\n" .
        "                    3 - Initiate LIP         = LIP(F7,al_ps)\n" .
        "                   17 - Login lid\n" .
        "                   18 - Login pid\n" .
        "                   33 - Logout lid\n" .
        "                   34 - Logout pid\n" .
        "                   49 - Target Reset lid\n" .
        "                   50 - Target Reset pid\n" .
        "                   65 - Loop Port Bypass (LPB) lid\n" .
        "                   66 - Loop Port Bypass (LPB) pid\n" .
        "                   81 - Loop Port Enable (LPE) lid\n" .
        "                   82 - Loop Port Enable (LPE) pid\n" .
        "                   Test modes below valid until next RESCAN LOOP\n" .
        "                   241 - Port bypass test lid\n" .
        "                   242 - Port bypass test pid\n" .
        "  id             pid (BE) or sid (FE)\n" .
        "  port           port\n" .
        "  lid            loop id\n";

##############################################################################
# short description
    $help{LS}{SHORT} =
        $help{DIR}{SHORT} =
        "Displays files in the current working directory.";

# long description
    $help{LS}{LONG} =
        $help{DIR}{LONG} =
        "LS/DIR\n";

##############################################################################
# short description
    $help{MEMREAD}{SHORT} =
        "Read memory on specified processor.";

# long description
    $help{MEMREAD}{LONG} =
        "(MPX)MEMREAD [-p CCB|FE|BE] [-t byte|short|word|binary] ".
        "[-f filename] [-C] [-F] address length\n" .
        "\n" .
        "  -p CCB|FE|BE       Processor to read from (default: CCB).\n" .
        "  -t byte|short|word|binary   Format of output data (default: word).\n" .
        "  -f filename        Name of file to write output to.\n" .
        "  -m filename        Name of map file to read symbols from.\n" .
        "  -C                 Calculate and print a CRC32 over the data.\n" .
        "                     Note: Output will NOT print to screen.\n" .
        "  -F                 Force memory access.\n" .
        "                        Note: Invalid addresses may cause processor to fault.\n" .
        "  address            Address or symbol to read (ex. 0xA0020000 or K_timel)\n".
        "  length             Number of bytes to read. If length is not specified,\n".
        "                     and a symbolic address is requested, a best guess at\n".
        "                     the length is made.\n";

##############################################################################
# short description
    $help{MEMWRITE}{SHORT} =
        "Write memory on specified processor.";

# long description
    $help{MEMWRITE}{LONG} =
        "(MPX)MEMWRITE [-p CCB|FE|BE] [-t byte|short|word] [-f filename] [-F] address data\n" .
        "\n" .
        "  -p CCB|FE|BE       Processor to write to (default: CCB).\n" .
        "  -t byte|short|word   Format of input data (default: word).\n" .
        "  -f filename        Name of file to read binary data from.\n" .
        "  -F                 Force memory access.\n" .
        "                        Note: Invalid addresses may cause processor to fault.\n" .
        "  address            Address to read (ex. 0xA0020000).\n" .
        "  data               Data to write (ex. A034C756FFFE).\n";

##############################################################################
# short description
    $help{MFGCLEAN}{SHORT} =
        "Cleans the controller.";

# long description
    $help{MFGCLEAN}{LONG} =
        "MFGCLEAN [-C] [-F] [-O] [-S] [option]\n" .
        "\n" .
        "  Option             Option for how to clean the controller (default LICENSE).\n" .
        "                         FULL - Returns the controller to state where serial console configuration is required\n" .
        "                         LICENSE - Returns the controller to a state where a license is required\n" .
        "  -C                 DO NOT clear logs on the controller.\n" .
        "  -F                 Brute force clean the drives.\n" .
        "  -O                 Power controller off when finished.\n" .
        "  -S                 Send a few messages to serial console.\n" ;

##############################################################################
# short description
    $help{MMTEST}{SHORT} =
        "Submit a MM card test.";

# long description
    $help{MMTEST}{LONG} =
        "MMTEST option [offset]\n" .
        "\n" .
        "  option           Test to run\n" .
        "                     0 = Inject a single-bit ECC error\n" .
        "                     1 = Inject a multi-bit ECC error\n" .
        "                     2 = Fail the MM card\n" .
        "                     3 = WC signature - set to 'data flushed'\n" .
        "                     4 = WC signature - invalidate controller S/N\n" .
        "                     5 = WC signature - invalidate sequence number\n" .
        "  offset           Offset in the MM card to submit the test\n";

##############################################################################
# short description
    $help{MODEBITINFO}{SHORT} =
        "Displays mode bits. DEPRECATED - see MODEBITS";

# long description
    $help{MODEBITINFO}{LONG} =
        "MODEBITINFO\n";

##############################################################################
# short description
    $help{MODEBITS}{SHORT} =
        "Gets/Sets the mode bits.";

# long description
    $help{MODEBITS}{LONG} =
        "MODEBITS [-I|-S] [flag(s) action]\n" .
        "\n" .
        "  -I Calls the old MODEBITINFO interface.\n" .
        "  -S Calls the old MODEBITSET interface.\n" .
        "\n" .
        "  If no parameters specified, MODEBITS \"gets\" the current enabled flags.\n" .
        "\n" .
        "  flag(s):  Comma seperated list of flags.\n" .
        "\n" .
        "  CCB MODE BITS:\n" .
        "    IPC_HEARTBEATS     0x0001 - IPC Heartbeats\n" .
        "    IPC_WATCHDOG       0x0002 - IPC Heartbeat Watchdog\n" .
        "    LOC_HEARTBEATS     0x0004 - Local Heartbeats\n" .
        "    LOC_STATISTICS     0x0008 - Local Statistics\n" .
        "    DUMP_LCL_IMAGE     0x0010 - Dump Local Image\n" .
        "    FAILURE_MGR        0x0020 - Failure Manager\n" .
        "    CTRL_SUICIDE       0x0040 - Controller Suicide\n" .
        "    FM_RESTART         0x0100 - Failure Manager Restart\n" .
        "    DIAG_PORTS         0x0200 - Diagnostic Ports\n" .
        "    INACTIVATE_POWER   0x1000 - Wookiee Inactivate Controller Power-off Disable\n" .
        "\n" .
        "  PROC MODE BITS:\n" .
        "    P_HB_WATCHDOG       0x0001 - Heartbeat Watchdog\n" .
        "    P_BOOT_ERR_HANDLING 0x0002 - Errtrap handling by boot code\n" .
        "    P_CTRL_SHUTDOWN     0x0004 - Controller shutting down. Ignore\n" .
        "                                    CCB heartbeat failure.\n" .
        "  action:\n" .
        "\n" .
        "    ENABLE          Enable the listed flag function, others unchanged\n" .
        "    DISABLE         Disable the listed flag function, others unchanged\n" .
        "    SET             Set the listed flags, other flags unchanged\n" .
        "    CLR             Clear the listed flags, other flags unchanged\n" ;

##############################################################################
# short description
    $help{MODEBITSET}{SHORT} =
        "Sets the mode bits. DEPRECATED - see MODEBITS";

# long description
    $help{MODEBITSET}{LONG} =
        "MODEBITSET [-D] [-p <CCB|PROC>] mode mask\n" .
        "\n" .
        "  -p CCB|PROC        Processor to send bits to (default: CCB).\n" .
        "  -D                 Set the CCB DPrintf() level bits (32 bit).\n" .
        "  mode               32 bit mode.\n" .
        "  mask               32 bit mask.\n" .
        "\n" .
        "  CCB MODE BITS (see mode.h in CCB for updates):\n" .
        "    0x0001 - IPC Heartbeat Disable\n" .
        "    0x0002 - IPC Heartbeat Watchdog Disable\n" .
        "    0x0004 - Local Heartbeat Disable\n" .
        "    0x0008 - Local Statistics Disable\n" .
        "    0x0010 - Dump Local Image Enable\n" .
        "    0x0020 - Failure Manager Disable\n" .
        "    0x0040 - Controller Suicide Disable\n" .
        "    0x0100 - Failure Manager Restart Disable\n" .
        "    0x0200 - Diagnostic Ports Enable\n" .
        "    0x1000 - Wookiee Inactivate Controller Power-off Disable\n" .
        "\n" .
        "  CCB DPRINTF MODE BITS (see mode.h in CCB for updates):\n" .
        "    0x0000 - OFF\n" .
        "    0x0001 - Default\n" .
        "    0x0001 - Debug\n" .
        "    0x0002 - Cache Refresh\n" .
        "    0x0004 - XSSA Debug\n" .
        "    0x0008 - Election\n\n" .

        "    0x0010 - IPC\n" .
        "    0x0020 - IPC Messages\n" .
        "    0x0040 - X1 Commands\n" .
        "    0x0080 - X1 Protocol\n\n" .

        "    0x0100 - I2C\n" .
        "    0x0200 - Resource Manager\n" .
        "    0x0400 - SCSI Enclosure Services\n" .
        "    0x0800 - Ethernet\n\n" .

        "    0x1000 - MD5\n" .
        "    0x2000 - Fibre Channel Monitor\n" .
        "    0x4000 - Sequence Manager\n" .
        "    0x8000 - Packet Interface\n\n" .

        "    0x10000 - Proc code printf messages\n\n" .

        "  PROC MODE BITS (see mode.h in CCB for updates):\n" .
        "    0x0001 - Heartbeat Watchdog Enable\n";

##############################################################################
# short description
    $help{MPXFWUPDATE}{SHORT} =
        "Update firmware on the CCB or Proc (multi-pkt).";

# long description
    $help{MPXFWUPDATE}{LONG} =
        "MPXFWUPDATE filename\n" .
        "\n" .
        "  filename          The name of the file to send to the CCB.\n";

##############################################################################
# short description
    $help{MPXMEMREAD}{SHORT} =
        "Read memory on specified processor (multi-pkt).";

# long description
    $help{MPXMEMREAD}{LONG} =
        $help{MEMREAD}{LONG};

##############################################################################
# short description
    $help{MPXMEMWRITE}{SHORT} =
        "Write memory on specified processor (multi-pkt).";

# long description
    $help{MPXMEMWRITE}{LONG} =
        $help{MEMWRITE}{LONG};

##############################################################################
# short description
    $help{NETSTAT}{SHORT} =
        "Prints active TCP socket connection data (same as STRUCTUREINFO 2).";

# long description
    $help{NETSTAT}{LONG} =
        "NETSTAT\n";

##############################################################################
# short description
    $help{PDISKAUTOFAILBACK}{SHORT} =
        "Enables, Disables or displays the state of the auto failback feature\n";

# long description
    $help{PDISKAUTOFAILBACK}{LONG} =
        "PDISKAUTOFAILBACK option\n" .
        "\n" .
        "  option\n" .
        "                       0 = Disables Auto failback feature  -- default\n" .
        "                       1 = Enables Auto failback feature\n" .
        "                       2 = Displays the current state of auto failback feature\n";

##############################################################################
# short description
    $help{PDATACHECKSUM}{SHORT} =
        "Get a byte checksum for the data.";

# long description
    $help{PDATACHECKSUM}{LONG} =
        "PDATACHECKSUM offset length\n" .
        "\n" .
        "  offset             Offset to start reset (ex. 0x00F0).\n" .
        "  length             Number of bytes to reset.\n";

##############################################################################
# short description
    $help{PDATAREAD}{SHORT} =
        "Read persistent data.";

# long description
    $help{PDATAREAD}{LONG} =
        "PDATAREAD [-t byte|short|word|binary] ".
        "[-f filename] offset length\n" .
        "\n" .
        "  -t byte|short|word|binary   Format of output data (default: word).\n" .
        "  -f filename        Name of file to write output to.\n" .
        "  offset             Offset to start read (ex. 0xA0020000).\n" .
        "  length             Number of bytes to read.\n";

##############################################################################
# short description
    $help{PDATARESET}{SHORT} =
        "Reset persistent data.";

# long description
    $help{PDATARESET}{LONG} =
        "PDATARESET offset length\n" .
        "\n" .
        "  offset             Offset to start reset (ex. 0x00F0).\n" .
        "  length             Number of bytes to reset.\n";

##############################################################################
# short description
    $help{PDATAWRITE}{SHORT} =
        "Write persistent data.";

# long description
    $help{PDATAWRITE}{LONG} =
        "PDATAWRITE [-i byte|short|word] offset data\n" .
        "\n" .
        "  -t byte|short|word   Format of input data (default: word).\n" .
        "  offset             offset to write at (ex. 0xA0020000).\n" .
        "  data               Data to write (ex. A034C756FFFE).\n";

##############################################################################
# short description
    $help{PDISKBEACON}{SHORT} =
        "Beacon a physical disk.";

# long description
    $help{PDISKBEACON}{LONG} =
        "PDISKBEACON pdisk_id [duration]\n" .
        "\n" .
        "  pdisk_id           Identifier of the physical disk.\n" .
        "  duration           Time in seconds to beacon the physical\n" .
        "                     disk (default: 10 seconds).\n";

##############################################################################
# short description
    $help{PDISKBYPASS}{SHORT} =
        "Bypass a physical disk.";

# long description
    $help{PDISKBYPASS}{LONG} =
        "PDISKBYPASS ses slot setting\n" .
        "\n" .
        "  ses                SES of the disk bay.\n" .
        "  slot               Slot of the physical disk.\n" .
        "  setting            Bypass setting for the physical disk.\n" .
        "                       0x0 = Unbypass\n" .
        "                       0x4 = Bypass - B\n" .
        "                       0x8 = Bypass - A\n" .
        "                       0xC = Bypass - A and B\n";

##############################################################################
# short description
    $help{PDISKCOUNT}{SHORT} =
        "Displays the number of physical disks.";

# long description
    $help{PDISKCOUNT}{LONG} =
        "PDISKCOUNT\n";

##############################################################################
# short description
    $help{MISCCOUNT}{SHORT} =
        "Displays the number of MISC devices.";

# long description
    $help{MISCCOUNT}{LONG} =
        "MISCCOUNT\n";

##############################################################################
# short description
    $help{PDISKDEFRAG}{SHORT} =
        "Defragment a physical disk.";

# long description
    $help{PDISKDEFRAG}{LONG} =
        "PDISKDEFRAG pdiskid\n" .
        "\n" .
        "  pdiskid            Identifier of the physical disk.\n" .
        "                       ALL or 0xFFFF - Defrag all physical disks\n" .
        "                       STOP or 0xFFFE - Stop all defrag operations\n" .
        "                       ORPHAN or 0xFFFD - Terminate all orphans\n" .
        "                       PID - Physical disk identifier\n";

##############################################################################
# short description
    $help{PDISKDEFRAGSTATUS}{SHORT} =
        "Status of physical disk defragmentation.";

# long description
    $help{PDISKDEFRAGSTATUS}{LONG} =
        "PDISKDEFRAGSTATUS\n";

##############################################################################
# short description
    $help{PDISKDELETE}{SHORT} =
        "Deletes a physical disk.";

# long description
    $help{PDISKDELETE}{LONG} =
        "PDISKDELETE id\n" .
        "\n" .
        "  id                  Id of physical disk to delete.\n";

##############################################################################
# short description
    $help{PDISKFAIL}{SHORT} =
        "Fail a physical disk.";

# long description
    $help{PDISKFAIL}{LONG} =
        "PDISKFAIL pdiskid [hsdiskid] [force]\n" .
        "\n" .
        "  pdiskid            Identifier of the physical disk.\n" .
        "  hsdiskid           Identifier of the spare physical disk.\n" .
        "  option             Option to enforce or ignore redundancy requirment.\n" .
        "                       bit 0 = 0 = Enforce redundancy requirement\n" .
        "                       bit 0 = 1 = Ignore redundancy requirement\n" .
        "                       bit 1 = 0 = Doesn't matter which hot spare\n" .
        "                       bit 1 = 1 = Use [hsdiskid] as hotspare\n" .
        "                       bit 2 = 0 = Do not write a fail label on the drive\n" .
        "                       bit 2 = 1 = Write a fail label on the drive\n";

##############################################################################
# short description
    $help{PDISKFAILBACK}{SHORT} =
        "Fails back the used hotspare back to the physical disk in the original location from where it was failed\n";

# long description
    $help{PDISKFAILBACK}{LONG} =
        "PDISKFAILBACK hsdiskid [option]\n" .
        "\n" .
        "  hsdiskid           Identifier of the spare physical disk on which data rebuilt from a failed drive\n" .
        "  option             Stream of bits each of which represents an option\n" .
        "                       bit 0 = 0 = Failback (rebuild) hotspare  -- default\n" .
        "                       bit 0 = 1 = Cancel the failback forever\n" .
        "                       Other bits --- unsed\n";

##############################################################################
# short description
    $help{PDISKINFO}{SHORT} =
        "Displays physical disk information.";

# long description
    $help{PDISKINFO}{LONG} =
        "PDISKINFO pdiskid [options]\n" .
        "\n" .
        "  pdiskid            Identifier of the physical disk.\n" .
        "  options            Options for retrieval (bit sensitive, default 0 - Standard).\n" .
        "                       0x1 - Init drive before returning\n" .
        "                       0x2 - Return RLS-ELS information\n";

##############################################################################
# short description
    $help{PDISKLABEL}{SHORT} =
        "Label or unlabel a physical disk.";

# long description
    $help{PDISKLABEL}{LONG} =
        "PDISKLABEL pdisk_id [class]\n" .
        "\n" .
        "  pdisk_id           Identifier of the physical disk.\n" .
        "                       ALL or 0xFFFF - Label all physical disks in the pdisklist.\n" .
        "                       ID - Array of physical disk identifiers to label.\n" .
        "                            This is a comma separated list of values\n" .
        "                            and ranges (i.e. 1,2,4-7,10,13-18).\n" .
        "  class              Label class (default 1=data).\n" .
        "                       0    = unlabeled\n" .
        "                       1    = data\n" .
        "                       2    = hot spare\n" .
        "                       3    = unsafe\n" .
        "                       0xFE = fix DNAME\n" .
        "                       0xFF = relabel\n";

##############################################################################
# short description
    $help{PDISKLEDSTATE}{SHORT} =
        "Set the LED state of a pdisk.";

# long description
    $help{PDISKLEDSTATE}{LONG} =
        "PDISKLEDSTATE pid state\n" .
        "\n" .
        "  pid                Identifier of the physical disk.\n" .
        "  state              State to set the led to.\n" .
        "                       0 = All OFF\n" .
        "                       1 = Fail ON\n" .
        "                       2 = Fail OFF\n" .
        "                       3 = Id ON\n" .
        "                       4 = Id OFF\n";

##############################################################################
# short description
    $help{PDISKLIST}{SHORT} =
        "Displays a list of physical disk identifiers.";

# long description
    $help{PDISKLIST}{LONG} =
        "PDISKLIST\n";

##############################################################################
# short description
    $help{MISCLIST}{SHORT} =
        "Displays a list of MISC device identifiers.";

# long description
    $help{MISCLIST}{LONG} =
        "MISCLIST\n";

##############################################################################
# short description
    $help{PDISKRESTORE}{SHORT} =
        "Restore a physical disk.";

# long description
    $help{PDISKRESTORE}{LONG} =
        "PDISKRESTORE pdiskid\n" .
        "\n" .
        "  pdiskid            Identifier of the physical disk.\n";

##############################################################################
# short description
    $help{PDISKS}{SHORT} =
        "Displays physical disk information for all physical disks.";

# long description
    $help{PDISKS}{LONG} =
        "PDISKS [display_type]\n" .
        "\n" .
        "  display_type       What information to display (default STD).\n" .
        "                       STD    - Standard display\n" .
        "                       LOOP   - Loop information\n" .
        "                       FWV    - Firmware/Vendor Information\n" .
        "                       SES    - SES and DNAME Information\n" .
        "                       PORTS  - Port information\n" .
        "                       DEFRAG - Defrag Information\n" .
        "                       BB     - PDisks by Bay - stats too\n" .
        "                       SMART  - PDisks by Bay & SATA SMART\n" .
        "                       CMPL   - Physical Disks info (all)\n";

##############################################################################
# short description
    $help{PDISKSCACHE}{SHORT} =
        "Displays physical disk information for all physical disks from Cache.";

# long description
    $help{PDISKSCACHE}{LONG} =
        "PDISKSCACHE [display_type]\n" .
        "\n" .
        "  display_type       What information to display (default STD).\n" .
        "                       STD    - Standard display\n" .
        "                       LOOP   - Loop information\n" .
        "                       FWV    - Firmware/Vendor Information\n" .
        "                       SES    - SES and DNAME Information\n" .
        "                       PORTS  - Port information\n" .
        "                       DEFRAG - Defrag Information\n";

##############################################################################
# short description
    $help{PDISKSPINDOWN}{SHORT} =
        "Spin down a physical disk.";

# long description
    $help{PDISKSPINDOWN}{LONG} =
        "PDISKSPINDOWN pid\n" .
        "\n" .
        "  pid                Identifier of the physical disk.\n";

##############################################################################
# short description
    $help{PDISKTIMEOUT}{SHORT} =
        "Emulate qlogic timeout on a physical disk.";

# long description
    $help{PDISKTIMEOUT}{LONG} =
        "PDISKTIMEOUT pdisk_id [on/off]\n" .
        "\n" .
        "  pdisk_id           Identifier of the physical disk.\n" .
        "  on/off             on = 0 (no timeout), 1= emulate qlogic timeout for physical drive.\n";

##############################################################################
# short description
    $help{PERFS}{SHORT} =
        "Display Controller Performance data.";

# long description
    $help{PERFS}{LONG} =
        "PERFS [fmt] [subfmt]  -- note: missing device numbers are concatenated(!?!)\n" .
        "\n" .
        "  fmt    Display format.\n" .
        "            STD - display HAB, VID, and PID statistics\n" .
        "            HAB - Display of HAB statistics\n" .
        "            VID - Display of VID statistics\n" .
        "            PID - Display of PID statistics\n" .
        "  subfmt  Display sub-format.\n" .
        "            missing - display MBPS, IOPS, and QD\n" .
        "            MBPS - Display of MBPS statistics\n" .
        "            IOPS - Display of IOPS statistics\n" .
        "            QD   - Display of QD statistics\n";

##############################################################################
# short description
    $help{PIGETX1ENV}{SHORT} =
        "Retrieve X1 Environmental Info using a PI packet.";

# long description
    $help{PIGETX1ENV}{LONG} =
        "PIGETX1ENV\n";

##############################################################################
# short description
    $help{PORTLIST}{SHORT} =
        "List of ports (fibre channel adapters) on the front or back end.";

# long description
    $help{PORTLIST}{LONG} =
        "PORTLIST processor [listType]\n" .
        "  processor              Processor to display portlist for: FE|BE\n" .
        "  listType(0|1|2|3|4|5|6|7|8|16|17)" .
        "                         0 - All ports (default).\n" .
        "                         1 - Initialized ports.\n" .
        "                         2 - Failed ports.\n" .
        "                         3 - Initializing ports.\n" .
        "                         4 - Good ports.\n" .
        "                         5 - Online ports.\n" .
        "                         6 - Offline ports.\n" .
        "                         7 - Marked as failed ports.\n" .
        "                         8 - Exist/Good bitmap list.\n" .
        "                         16 - Ports with targets (FE Only).\n" .
        "                         17 - Ports without targets (FE Only).\n";

##############################################################################
# short description
    $help{POWERUPRESPONSE}{SHORT} =
        "Send a response to the current power-up wait state.";

# long description
    $help{POWERUPRESPONSE}{LONG} =
        "POWERUPRESPONSE state astatus response\n" .
        "  state                Power-up state to which this command is responding.\n" .
        "                           0x0002 - Firmware versions incompatible.\n" .
        "                           0x0004 - Processor communications not established.\n" .
        "                           0x0008 - Missing or invalid license.\n" .
        "                           0x0010 - No owned drives.\n" .
        "                           0x0015 - Disaster mode detected.\n" .
        "                           0x0020 - Missing controllers.\n" .
        "                           0x0100 - Missing disk bays.\n" .
        "                           0x0200 - Corrupt BE NVRAM.\n" .
        "                           0x0810 - Cache Error.\n" .
        "  astatus              Power-up additional status to which this command is responding.\n" .
        "                           0x0000 - Unknown or unused.\n" .
        "                           0x0001 - Write Cache, Bad Sequence Number\n" .
        "                           0x0002 - Write Cache, Bad VCG Serial Number\n" .
        "                           0x0003 - Write Cache, Bad Serial Number\n" .
        "                           0x0004 - Write Cache, Bad NVMEM\n" .
        "  response             Response to the power-up state.\n" .
        "                           1 - Retry\n" .
        "                           2 - Continue\n" .
        "                           3 - Reset\n" .
        "                           4 - Continue Keep-Alive\n" .
        "                           5 - Write Cache, Save Data\n" .
        "                           6 - Write Cache, Discard Data\n";

##############################################################################
# short description
    $help{POWERUPSTATE}{SHORT} =
        "Display the power-up state of this controller.";

# long description
    $help{POWERUPSTATE}{LONG} =
        "POWERUPSTATE\n";

##############################################################################
# short description
    $help{PWD}{SHORT} =
        "Displays the current working directory.";

# long description
    $help{PWD}{LONG} =
        "PWD\n";

##############################################################################
# short description
    $help{QUICKMIRRORPAUSEEXECUTE}{SHORT} =
        "Execute command for quick pause break resume on list of mirrors.";

# long description
    $help{QUICKMIRRORPAUSEEXECUTE}{LONG} =
        "QUICKMIRRORPAUSEEXECUTE action\n" .
        "\n" .
        "  action          execute command option.\n" .
        "                       CANCEL          0x00\n" .
        "                       GO              0x01\n" ;

##############################################################################
# short description
    $help{QUICKMIRRORPAUSESEQUENCE}{SHORT} =
        "Sequence command for quick pause break resume mirror.";

# long description
    $help{QUICKMIRRORPAUSESEQUENCE}{LONG} =
        "QUICKMIRRORPAUSESEQUENCE operation dest_vid\n" .
        "\n" .
        "  operation          Control operation to perform.\n" .
        "                       BREAK_SPECIFIED_COPY    0x05\n" .
        "                       PAUSE_COPY              0x06\n" .
        "                       RESUME_COPY             0x07\n" .
        "  dest_vid           Destination virtual disk.\n";

##############################################################################
# short description
    $help{QUICKMIRRORPAUSESTART}{SHORT} =
        "Start command for quick pause break resume mirror.";

# long description
    $help{QUICKMIRRORPAUSESTART}{LONG} =
        "QUICKMIRRORPAUSESTART count\n" .
        "\n" .
        "  count       Number of mirror list is going to be sent in sequence command.\n";

##############################################################################
# short description
    $help{QUIT}{SHORT} =
        $help{EXIT}{SHORT} =
        "Quits the Bigfoot command line.";

# long description
    $help{QUIT}{LONG} =
        $help{EXIT}{LONG} =
        "QUIT/EXIT\n";

##############################################################################
# short description
    $help{RAIDBEACON}{SHORT} =
        "Beacon a raid device.";

# long description
    $help{RAIDBEACON}{LONG} =
        "RAIDBEACON raid_id [duration]\n" .
        "\n" .
        "  raid_id            Identifier of the raid device.\n" .
        "  duration           Time in seconds to beacon each physical\n" .
        "                     disk in the raid device(default: 10 seconds).\n";

##############################################################################
# short description
    $help{RAIDCOUNT}{SHORT} =
        "Displays the number of raid devices.";

# long description
    $help{RAIDCOUNT}{LONG} =
        "RAIDCOUNT\n";

##############################################################################
# short description
    $help{RAIDINFO}{SHORT} =
        "Displays raid information.";

# long description
    $help{RAIDINFO}{LONG} =
        "RAIDINFO raidid\n" .
        "\n" .
        "  raidid             Raid identifier.\n";

##############################################################################
# short description
    $help{RAIDINIT}{SHORT} =
        "Initialize a raid device.";

# long description
    $help{RAIDINIT}{LONG} =
        "RAIDINIT raid_id\n" .
        "\n" .
        "  raid_id            Raid identifier.\n";

##############################################################################
# short description
    $help{RAIDLIST}{SHORT} =
        "Displays a list of raid device identifiers.";

# long description
    $help{RAIDLIST}{LONG} =
        "RAIDLIST\n";

##############################################################################
# short description
    $help{RAIDRECOVER}{SHORT} =
        "Attempt to recover an inoperative raid.";

# long description
    $help{RAIDRECOVER}{LONG} =
        "RAIDRECOVER id\n" .
        "\n" .
        "  id                 Raid identifier.\n";

##############################################################################
# short description
    $help{RAIDS}{SHORT} =
        "Displays raid information for all raids.";

# long description
    $help{RAIDS}{LONG} =
        "RAIDS\n";

##############################################################################
# short description
    $help{RAIDSCACHE}{SHORT} =
        "Displays raid information for all raids from cache.";

# long description
    $help{RAIDSCACHE}{LONG} =
        "RAIDSCACHE\n";

##############################################################################
# short description
    $help{REGISTERCLIENTTYPE}{SHORT} =
        " Register the client type of the connection.";
# long description
    $help{REGISTERCLIENTTYPE}{LONG} =
        " Register client type with the type specified.\n".
        " Returns the number of clients of that type already exists.\n".
        "        Type             client type.\n".
        "                         1  -   IWS\n";


##############################################################################
# short description
    $help{REGISTEREVENTS}{SHORT} =
        "Register Asynchronous events.";

# long description
    $help{REGISTEREVENTS}{LONG} =
        "REGISTEREVENTS option mode\n" .
        "\n" .
        "  option             0 - Set registration,\n" .
        "                     1 - Get current registration status (does not work).\n" .
        "  mode               Bit field: 0 - OFF, 1 - ON.\n" .
        "                     -1 for all 32 bits (of 64 bit field)\n" .
        "                     See CCB/Inc/X1_AsyncEventHandler.h\n";

##############################################################################
# short description
    $help{RESCANDEVICE}{SHORT} =
        "Rescans the devices.";

# long description
    $help{RESCANDEVICE}{LONG} =
        "RESCANDEVICE type\n" .
        "\n" .
        "  type          LIST|LUNS|LOOP\n" .
        "                LIST - rescan current devices (default)\n" .
        "                LUNS - rescan the luns\n" .
        "                LOOP - rediscover devices\n";

##############################################################################
# short description
    $help{RESET}{SHORT} =
        "Resets the specified processor.";

# long description
    $help{RESET}{LONG} =
        "RESET processor [mode]\n" .
        "\n" .
        "  processor          Processor to reset: CCB|ALL (default CCB).\n" .
        "  mode               Reset mode to execute (default 0).\n" .
        "                         0 = immediate\n" .
        "                         1 = forked/delayed\n" .
        "                         2 = forked/delayed with reconnect\n";

##############################################################################
# short description
    $help{RESETQLOGIC}{SHORT} =
        "Resets the specified Qlogic card.";

# long description
    $help{RESETQLOGIC}{LONG} =
        "RESETQLOGIC processor [port] [option]\n" .
        "\n" .
        "  processor          Processor to reset: FE|BE\n" .
        "  port               port to reset\n" .
        "                         (0 - 3) reset individual port\n" .
        "                         ANY - Only ports needed.\n" .
        "                         ALL - Reset all ports (default)\n" .
        "  option             reset option\n" .
        "                         0x00 - Reset and initialize (default).\n" .
        "                         0x01 - Reset only.\n" .
        "                         0x02 - Reset and initialize if offline.\n" .
        "                         0x03 - Reset only offline.\n" .
        "                         0x04 - Reset, initialize, and log.\n" .
        "                         0x05 - Reset and log.\n" .
        "                         0xFE - Force System Error.\n" .
        "                         0xFF - Reset, initialize, and log.\n";

##############################################################################
# short description
    $help{RESYNCCTL}{SHORT} =
        "Submit the resync control operation\n";

# long description
    $help{RESYNCCTL}{LONG} =
        "RESYNCCTL fc rid csn gid name\n" .
        "\n" .
        "  fc                   Function code\n" .
        "                         NAME  or 0x01 - Set name/group\n" .
        "                         DEL   or 0x00 - Delete COR\n" .
        "                         SWAP  or 0x02 - Swap mirror\n" .
        "  rid                  Copy registration ID\n" .
        "  csn                  Copy manager serial number\n" .
        "  gid                  GID Number\n" .
        "  name                 Copy user defined name (16 bytes)\n";

##############################################################################
# short description
    $help{RESYNCDATA}{SHORT} =
        "Displays the resync data.";

# long description
    $help{RESYNCDATA}{LONG} =
        "RESYNCDATA [display_format] [trace_entry_count]\n" .
        "\n" .
        "  display_format       Display format.\n" .
        "                         STD   - Standard display formatting\n" .
        "                         VIDS  - Display of VIDs involved with copies\n" .
        "                         OCSE  - Operation Control State Engine\n" .
        "                         DTL   - Detailed display formatting\n"   .
        "                         PAUSE - Copies NOT paused map\n"   .
        "                         TRACE - Display CCSM traces\n"   .
        "  trace_entry_count     0-4096 - Number of trace enties to display\n"  .
        "                                 (default = all)\n";

##############################################################################
# short description
    $help{RESYNCMIRRORS}{SHORT} =
        "Submit the resync request to the controller.";

# long description
    $help{RESYNCMIRRORS}{LONG} =
        "RESYNCMIRRORS type [rid]\n" .
        "  type                     Type of resync being submitted.\n" .
        "                             1 = Sync stripes given NVA table\n" .
        "                             2 = Sync given RID\n" .
        "                             3 = Sync all raids on this controller\n" .
        "                             4 = Sync all raids with NOT MIRRORING\n" .
        "                                 on a different controller but now\n" .
        "                                 owned by this controller\n" .
        "                             5 = Sync all raids in the given list\n" .
        "                                 NOTE: List is not available through\n" .
        "                                       CCBE operations.\n" .
        "  rid                      Raid Identifier\n";

##############################################################################
# short description
    $help{ROLLINGUPDATEPHASE}{SHORT} =
        "Execute rolling update phase command.";

# long description
    $help{ROLLINGUPDATEPHASE}{LONG} =
        "ROLLINGUPDATEPHASE controllerSN phase\n" .
        "\n" .
        "  controllerSN       Controller Serial Number.\n" .
        "  phase              1 or 2.\n";

##############################################################################
# short description
    $help{RMSTATE}{SHORT} =
        "Display the resource manager state for this controller.";

# long description
    $help{RMSTATE}{LONG} =
        "RMSTATE\n";

##############################################################################
# short description
    $help{SOURCE}{SHORT} =
        "Run a command file. -- see RUN (same thing)";

# long description
    $help{SOURCE}{LONG} =
        "SOURCE filename\n" .
        "\n" .
        "  filename           Command file to run\n";

##############################################################################
# short description
    $help{RUN}{SHORT} =
        "Run a command file. -- see SOURCE (same thing)";

# long description
    $help{RUN}{LONG} =
        "RUN filename\n" .
        "\n" .
        "  filename           Command file to run\n";

##############################################################################
# short description
    $help{SCRUBINFO}{SHORT} =
        "Displays the scrubbing information.";

# long description
    $help{SCRUBINFO}{LONG} =
        "SCRUBINFO\n";

##############################################################################
# short description
    $help{SCRUBSET}{SHORT} =
        "Set the different scrubbing options.";

# long description
    $help{SCRUBSET}{LONG} =
        "SCRUBSET scrub_control parity_control raidid\n" .
        "\n" .
        "  scrub_control      Scrubbing control.\n" .
        "                       ENABLE    = Enable scrubbing\n" .
        "                       DISABLE   = Disable scrubbing\n" .
        "  parity_control     Parity control options set as a hex\n" .
        "                     value or as a set of flags.\n" .
        "                       HEX VALUE: 0x########\n" .
        "                       FLAGS - | separated list of flags\n" .
        "                           DEFAULT - Enable default values\n" .
        "                           MARKED - Scan marked PSDs\n" .
        "                           CORRUPT - Corrupt parity data\n" .
        "                           SPECIFIC - Check specific raid\n" .
        "                           CLEARLOGS - Clear logs at start\n" .
        "                           1PASS - Single pass\n" .
        "                           CORRECT - Enable error correction\n" .
        "                           ENABLE - Enable parity scan\n" .
        "                           DISABLE - Disable parity scan\n" .
        "  raid_id            Identifier of the raid to scrub.\n";

##############################################################################
# short description
    $help{SCSICMD}{SHORT} =
        "Issue a SCSI cmd to a drive bay or physical disk.";

# long description
    $help{SCSICMD}{LONG} =
        "SCSICMD [-V] [-l length] [-s SCSILUN] type ordinal CDB [data]\n" .
        "\n" .
        "  -V                Print out scsi command just before sending it.\n" .
        "  -l length         Length of output data to display (max == dft == 2048).\n" .
        "  -s SCSILUN        LUN to use when sending to WWN of device (overrides device default)\n" .
        "  type (PDISK|BAY)  which device type the ordinal is associated with.\n" .
        "  ordinal           an ordinal related to the type.\n" .
        "  CDB               CDB (ex. 12000000FF00).\n" .
        "                    A few common commands/CDB's can be specified as follows:\n" .
        "                       TEST_UNIT_READY or TUR\n" .
        "                       REQUEST_SENSE or RS\n" .
        "                       INQUIRY or INQ\n" .
        "                       START_UNIT or START\n" .
        "                       STOP_UNIT or STOP\n" .
        "                       LOG_PAGE_32 or LOG32\n" .
        "                       PAGE_30 or PAGE30 (log sense page 0x30)\n" .
        "  data              Data to send (ex. A034C756FFFE).\n" .
        "";

##############################################################################
# short description
    $help{PAGE30BAY}{SHORT} =
        "Get page 0x30 results from first 16 luns of an ISE.";

# long description
    $help{PAGE30BAY}{LONG} =
        "PAGE30BAY [-n number_loops] bay_id\n" .
        "  -n number_loops   Number of times to query all 16 luns.\n" .
        "  bay_id            The Bay ID(s) to query all LUNs for page 0x30.\n";

##############################################################################
# short description
    $help{SERIALNUMBERS}{SHORT} =
        "Displays the serial numbers for this controller.";

# long description
    $help{SERIALNUMBERS}{LONG} =
        "SERIALNUMBERS\n";

##############################################################################
# short description
    $help{SERVERASSOC}{SHORT} =
        "Associate a server to a virutal disk.";

# long description
    $help{SERVERASSOC}{LONG} =
        "SERVERASSOC serverid lun vdiskid [option] [dest_sid]\n" .
        "\n" .
        "  serverid          Server identifier.\n" .
        "  lun               Lun to use in the association (0 - 63).\n" .
        "  vdiskid           Virtual disk identifier.\n" .
        "  option            Associate options:\n" .
        "                      0 = Regular map - nuthin fancy\n" .
        "                      1 = SID to DSID swap of mappings\n" .
        "                      2 = SID to DSID copy (DSID mapping remain)\n" .
        "                      3 = SID to DSID copy (DSID mapping deleted)\n" .
        "                      4 = SID to DSID link (DSID mapping deleted)\n" .
        "  dest_sid          Destination server identifier.\n";

##############################################################################
# short description
    $help{SERVERCOUNT}{SHORT} =
        "Displays the number of servers.";

# long description
    $help{SERVERCOUNT}{LONG} =
        "SERVERCOUNT\n";

##############################################################################
# short description
    $help{SERVERCREATE}{SHORT} =
        "Create a server.";

# long description
    $help{SERVERCREATE}{LONG} =
        "SERVERCREATE target_id owner wwn\n" .
        "\n" .
        "  target_id         Target identifier.\n" .
        "  owner             owner.\n" .
        "  wwn               World wide name.\n";

##############################################################################
# short description
    $help{SERVERDELETE}{SHORT} =
        "Delete a server.";

# long description
    $help{SERVERDELETE}{LONG} =
        "SERVERDELETE serverid [option]\n" .
        "\n" .
        "  serverid          Server identifier.\n" .
        "  option             Delete server option.\n" .
        "                       0 = Delete the SDD when done\n" .
        "                       1 = Only delete the mappings (including mapping on linked servers)\n" .
        "                       2 = Delete the SDD and linked SDDs\n" .
        "                       3 = Remove server from linked server list\n";

##############################################################################
# short description
    $help{SERVERDISASSOC}{SHORT} =
        "Disassociate a server.";

# long description
    $help{SERVERDISASSOC}{LONG} =
        "SERVERDISASSOC serverid lun vid\n" .
        "\n" .
        "  serverid          Server identifier.\n" .
        "  lun               Lun to disassociate (0 - 63).\n" .
        "  vid               Virtual disk ID to disassociate.\n";

##############################################################################
# short description
    $help{SERVERINFO}{SHORT} =
        "Displays server information.";

# long description
    $help{SERVERINFO}{LONG} =
        "SERVERINFO serverid\n" .
        "\n" .
        "  serverid          Server identifier.\n";

##############################################################################
# short description
    $help{SERVERLIST}{SHORT} =
        "Displays a list of servers identifiers.";

# long description
    $help{SERVERLIST}{LONG} =
        "SERVERLIST\n";

##############################################################################
# short description
    $help{SERVERPROPERTY}{SHORT} =
        "Set server properties.";

# long description
    $help{SERVERPROPERTY}{LONG} =
        "SERVERPROPERTY serverid priority attributes\n" .
        "\n" .
        "  serverid          Server identifier.\n" .
        "  priority          priority.\n" .
        "                      0 - 128.\n" .
        "  attributes        attributes.\n" .
        "                      bit 0  = 1  new.\n" .
        "                      bit 0  = 0  managed server.\n" .
        "                      bit 1  = 1  hide.\n" .
        "                      bit 1  = 0  unhide.\n" .
        "                      bit 31 = 1  default server.\n" .
        "                      bit 31 = 0  not default.\n";

##############################################################################
# short description
    $help{SERVERS}{SHORT} =
        "Displays server information for all servers.";

# long description
    $help{SERVERS}{LONG} =
        "SERVERS\n";

##############################################################################
# short description
    $help{SERVERWWNTOTARGETMAP}{SHORT} =
        "Displays active server WWN and associated targets.";

# long description
    $help{SERVERWWNTOTARGETMAP}{LONG} =
        "SERVERWWNTOTARGETMAP\n";

##############################################################################
# short description
    $help{SETCONTROLLERTYPE}{SHORT} =
        "Set the controller type for the current connection.";

# long description
    $help{SETCONTROLLERTYPE}{LONG} =
        "SETCONTROLLERTYPE type\n" .
        "\n" .
        "  type               BIGFOOT|WOOKIEE|750\n";

##############################################################################
# short description
    $help{SETGEOLOCATION}{SHORT} =
        "Sets the Geo location for the specified bay.";

# long description
    $help{SETGEOLOCATION}{LONG} =
        "SETGEOLOCATION bayId locationCode\n" .
        "\n" .
        "  bayId                Bay ID.\n" .
        "  location             LocationCode\n";

##############################################################################
# short description
    $help{SETSERIALNUM}{SHORT} =
        "Sets the controllers serial numbers.";

# long description
    $help{SETSERIALNUM}{LONG} =
        "SETSERIALNUM serialNumber which\n" .
        "\n" .
        "  serialNumber       New serial number for the controller.\n" .
        "  which              Which serial number to set.\n" .
        "                         1 = Controller Serial Number\n" .
        "                         2 = System Serial Number\n";

##############################################################################
# short description
    $help{SETWORKSETINFO}{SHORT} =
        "Set workset information.";

# long description
    $help{SETWORKSETINFO}{LONG} =
        "SETWORKSETINFO [-C]  worksetID  name  vBlockList  serverList  defaultVPort\n" .
        "\n" .
        "  -C           Clear Worksets specified by worksetID.  All other parms ignored.\n".
        "  worksetID    Workset identifier.\n".
        "  name         Workset name.\n" .
        "  vBlockList   Comma separated list of VBlocks (0 - 15).\n" .
        "                 This is a comma separated list of values\n" .
        "                 and ranges (i.e. 1,2,4-7,10,13-18).\n" .
        "  serverList   Comma separated list of Servers (0 - 255).\n" .
        "                 This is a comma separated list of values\n" .
        "                 and ranges (i.e. 1,2,4-7,10,13-18).\n" .
        "  defaultVPort Default VPort ID.\n";

##############################################################################
# short description
    $help{SHELL}{SHORT} =
        "Shell out to an external shell.";

# long description
    $help{SHELL}{LONG} =
        "SHELL\n";

##############################################################################
# short description
    $help{SNAPCHANGE}{SHORT} =
        "Change a system snapshot.";

# long description
    $help{SNAPCHANGE}{LONG} =
        "SNAPCHANGE seq# status [description]]\n" .
        "\n" .
        "  seq#              The snapshot sequence number.\n" .
        "  status            NC = No Change (default)\n" .
        "                    KEEP = Mark this entry \"read-only.\"\n" .
        "                    DEL  = Delete this entry.\n" .
        "                    NORM = Mark entry as \"normal.\"\n" .
        "  description       New text description of this snapshot entry.\n";

##############################################################################
# short description
    $help{SNAPLOAD}{SHORT} =
        "Load a system snapshot.";

# long description
    $help{SNAPLOAD}{LONG} =
        "SNAPLOAD seq# [flags]\n" .
        "\n" .
        "  seq#              The snapshot sequence number.\n" .
        "  flags             ALL = Load all FIDS (default).\n" .
        "                    'M' = Load the Master Config Record.\n" .
        "                    'C' = Load the Controller Map.\n" .
        "                    'N' = Load the BE NVRAM.\n" .
        "  (Note: flags are or'd together: M|N).\n";

##############################################################################
# short description
    $help{SNAPREADDIR}{SHORT} =
        "Read the snapshot directory.";

# long description
    $help{SNAPREADDIR}{LONG} =
        "SNAPREADDIR\n";

##############################################################################
# short description
    $help{SNAPTAKE}{SHORT} =
        "Take a system snapshot.";

# long description
    $help{SNAPTAKE}{LONG} =
        "SNAPTAKE type [description]\n" .
        "\n" .
        "  type              1 = Manual (default)\n" .
        "                    2 = Power Up\n" .
        "                    3 = Power Down\n" .
        "                    4 = Config Change\n" .
        "  description       Text description of this snapshot.\n";

##############################################################################
# short description
    $help{SOSTABLE}{SHORT} =
        "Retrieve and print an SOS table.";

# long description
    $help{SOSTABLE}{LONG} =
        "SOSTABLE pid\n" .
        "\n" .
        "  pid                Pid to retrieve SOS table.\n";

##############################################################################
# short description
    $help{STATSBUFFERBOARD}{SHORT} =
        "Displays buffer board (MicroMemory) statistics.";

# long description
    $help{STATSBUFFERBOARD}{LONG} =
        "STATSBUFFERBOARD commandCode\n" .
        "\n" .
        "  commandCode       0 = info only\n" .
        "                    1 = shut down\n" .
        "\n" ;

##############################################################################
# short description
    $help{STATSCACHEDEV}{SHORT} =
        "Displays statistics for a cache device.";

# long description
    $help{STATSCACHEDEV}{LONG} =
        "STATSCACHEDEV [vdisk_id]\n" .
        "\n" .
        "  vdisk_id          Virtual disk identifier.\n";

##############################################################################
# short description
    $help{STATSENVIRONMENTAL}{SHORT} =
        "Displays environmental statistics.";

# long description
    $help{STATSENVIRONMENTAL}{LONG} =
        "STATSENVIRONMENTAL\n";

##############################################################################
# short description
    $help{STATSHAB}{SHORT} =
        "Displays HAB statistics.";

# long description
    $help{STATSHAB}{LONG} =
        "STATSHAB   id\n" .
        "\n" .
        "  id       0-3 for FE, 4-7 for BE (not currently supported)\n";

##############################################################################
# short description
    $help{STATSLOOP}{SHORT} =
        "Displays statistics for front end or back end loop.";

# long description
    $help{STATSLOOP}{LONG} =
        "STATSLOOP type  option\n" .
        "\n" .
        "  type         Which statistics.\n" .
        "                   BE - Back end loop statistics\n" .
        "                   FE - Front end loop statistics\n".
        "  option       0x00 - Normal data (default).\n" .
        "               EXT  - Extended data.\n";

##############################################################################
# short description
    $help{STATSPCI}{SHORT} =
        "Displays statistics for back end PCI.";

# long description
    $help{STATSPCI}{LONG} =
        "STATSPCI [type]\n" .
        "\n" .
        "  type               Which statistics.\n" .
        "                       BE - Back end PCI statistics\n" .
        "                       FE - Front end PCI statistics\n";

##############################################################################
# short description
    $help{STATSPROC}{SHORT} =
        "Displays statistics for back end proc.";

# long description
    $help{STATSPROC}{LONG} =
        "STATSPROC [type]\n" .
        "\n" .
        "  type               Which statistics.\n" .
        "                       BE - Back end proc statistics\n" .
        "                       FE - Front end proc statistics\n";

##############################################################################
# short description
    $help{STATSSERVER}{SHORT} =
        "Displays statistics for a server.";

# long description
    $help{STATSSERVER}{LONG} =
        "STATSSERVER serverid\n" .
        "\n" .
        "  serverid          Server identifier.\n";

##############################################################################
# short description
    $help{STATSSERVERS}{SHORT} =
        "Displays statistics for all valid servers.";

# long description
    $help{STATSSERVERS}{LONG} =
        "STATSSERVERS [display_type]\n" .
        "\n" .
        "  display_type       What information to display (default ALL).\n" .
        "                       ALL   - All statistics\n" .
        "                       AGG   - Aggregate statistics\n" .
        "                       PER   - Periodic statistics\n" .
        "                       MISC  - Miscellaneous statistics\n" .
        "\n";

##############################################################################
# short description
    $help{STATSVDISK}{SHORT} =
        "Displays statisitcs for virtual disks.";

# long description
    $help{STATSVDISK}{LONG} =
        "STATSVDISK\n";

##############################################################################
# short description
    $help{STRUCTUREINFO}{SHORT} =
        "Displays a strucure from the CCB.";

# long description
    $help{STRUCTUREINFO}{LONG} =
        "STRUCTUREINFO structure\n" .
        "\n" .
        "  structure      Identifier of the structure.\n\n" .
        "  struct ID      Description\n" .
        "  ---------      -----------\n" .
        "      1          Master Config\n" .
        "      2          Socket Connection Data\n" .
        "      3          SES Device and Page 2\n" .
        "      4          Snapshot Directory\n" .
        "      5          CCB Statistics\n";

##############################################################################
# short description
    $help{TARGETCOUNT}{SHORT} =
        "Displays the number of targets.";

# long description
    $help{TARGETCOUNT}{LONG} =
        "TARGETCOUNT\n";

##############################################################################
# short description
    $help{TARGETINFO}{SHORT} =
        "Displays target information.";

# long description
    $help{TARGETINFO}{LONG} =
        "TARGETINFO targetid\n" .
        "\n" .
        "  targetid          Target identifier.\n";

##############################################################################
# short description
    $help{TARGETLIST}{SHORT} =
        "Displays a list of target identifiers.";

# long description
    $help{TARGETLIST}{LONG} =
        "TARGETLIST\n";

##############################################################################
# short description
    $help{TARGETMOVE}{SHORT} =
        "Move a target to another controller and/or port.";

# long description
    $help{TARGETMOVE}{LONG} =
        "TARGETMOVE target_id dest_controller_sn [port]\n" .
        "\n" .
        "  target_id          Target identifier\n" .
        "  dest_controller_sn Destination controller serial number\n" .
        "  port               Port on destination controller\n";

##############################################################################
# short description
    $help{TARGETRESLIST}{SHORT} =
        "Displays a list of resources associated with a Target ID.";

# long description
    $help{TARGETRESLIST}{LONG} =
        "TARGETRESLIST list_type [target_id] [starting_id]\n" .
        "\n" .
        "  list_type          List requested.\n" .
        "                       0       Servers with Stats\n" .
        "                       1       Servers\n" .
        "                       2       Default Servers\n" .
        "                       3       Unmanaged Servers\n" .
        "                       4       Managed Servers\n" .
        "                       5       VLink Servers\n" .
        "                       6       Logged on Servers\n" .
        "                       7       Active Servers\n" .
        "                       8       Virtual Disks\n" .
        "                       9       Virtual Disks with cache enabled\n" .
        "                      10       Virtual Disks with cache disabled\n" .
        "                      11       Unmapped Virtual Disks\n" .
        "                      12       LUN Mappings\n" .
        "                      13       LUN Mappings - cached VDisks only\n" .
        "                      14       LUN Mappings - uncached VDisks only\n" .
        "                      15       LUN Mappings - default\n" .
        "                      16       Logged on Servers - from database\n\n" .

        "                      32       Servers with Stats  (sid / WWN format)\n" .
        "                      33       Servers             (sid / WWN format)\n" .
        "                      34       Default Servers     (sid / WWN format)\n" .
        "                      35       Unmanaged Servers   (sid / WWN format)\n" .
        "                      36       Managed Servers     (sid / WWN format)\n" .
        "                      37       VLink Servers       (sid / WWN format)\n" .
        "                      38       Logged on Servers   (sid / WWN format)\n" .
        "                      39       Active Servers      (sid / WWN format)\n" .
        "                      40-47    NOT IMPLEMENTED\n" .
        "                      48       Logged on Servers - from database (sid / WWN format)\n" .

        "  target_id          Target identifier (default 0xFFFF = ALL)\n" .
        "  starting_id        Starting Identifier (default 0).\n";

##############################################################################
# short description
    $help{TARGETS}{SHORT} =
        "Displays target information for all targets.";

# long description
    $help{TARGETS}{LONG} =
        "TARGETS\n";

##############################################################################
# short description
    $help{TARGETSETPROP}{SHORT} =
        "Set the properties of a target.";

# long description
    $help{TARGETSETPROP}{LONG} =
        "TARGETSETPROP target_id option loop_id [port] [owner] [cluster] [lock]\n" .
        "\n" .
        "  target_id          Target identifier\n" .
        "  option             Option for hard or soft ID\n" .
        "                       HARD - Set this target to use a hard ID\n" .
        "                       SOFT - Set this target to use soft IDs\n" .
        "  loop_id            Loop identifier\n" .
        "  port               Port number\n" .
        "  owner              Serial number of the controlle that owns this target\n" .
        "  cluster            Target cluster identifier.\n" .
        "  lock (UNLOCK|LOCK) UNLOCK - Unlock the target\n" .
        "                     LOCK   - Lock the target\n";

##############################################################################
# short description
    $help{TARGETSTATUS}{SHORT} =
        "Displays the status of targets.";

# long description
    $help{TARGETSTATUS}{LONG} =
        "TARGETSTATUS [targetid]\n" .
        "\n" .
        "  targetid          Target identifier.\n" .
        "                    (default: ALL).\n";

##############################################################################
# short description
    $help{TARGETTEST}{SHORT} =
        "Move targets from one interface to another.";

# long description
    $help{TARGETTEST}{LONG} =
        "TARGETTEST target_id oldIF newIF testPasses timeInterval timeOldNew verbose\n" .
        "\n" .
        "  target_id          Target identifier\n" .
        "  oldIF              old interface slot\n" .
        "  newIF              new interface slot\n" .
        "  testPasses         number of times to test\n" .
        "  timeInterval       time interval between passes\n" .
        "  timeOldNew         time interval between old and new interface change\n" .
        "  verbose            verbose (serial console output)  0 = off,  1 = on\n";

##############################################################################
# short description
    $help{TIMEOUT}{SHORT} =
        "Set timeout values.";

# long description
    $help{TIMEOUT}{LONG} =
        "TIMEOUT [type] [value]\n" .
        "\n" .
        "  type               Timeout type.\n" .
        "                       CCBCL - CCBCL Command Line Timeout Value (the\n".
        "                               amount of time ccbCL.pl waits for a\n".
        "                               response from the CCB before timing\n".
        "                               out the request)\n" .
        "                       MRP   - MRP Timeout Value (the amount of time\n".
        "                               the CCB waits for a response from the FE\n".
        "                               or BE before timing out the MRP request)\n" .
        "                       CCB   - CCB Timeout Value (the amount of time the\n".
        "                               CCB waits for a client request (ccbCl.pl\n".
        "                               or XSSA) before closing the connection)\n" .
        "                       IPC   - IPC Timeout Value (the amount of time\n".
        "                               one CCB waits for another CCB's response\n".
        "                               before timing out the IPC request)\n" .
        "  value              Time in seconds for the timeout value.\n";

##############################################################################
# short description
    $help{TIMESYNC}{SHORT} =
        "Sync the Real time clock on the controller.";

# long description
    $help{TIMESYNC}{LONG} =
        "TIMESYNC\n" .
        "  -o n   where n is the number of hours to subtract from\n" .
        "         GMT (allows setting to local time)\n";

##############################################################################
# short description
    $help{VCGACTIVATECONTROLLER}{SHORT} =
        "Activate a controller that is part of this group.";

# long description
    $help{VCGACTIVATECONTROLLER}{LONG} =
        "VCGACTIVATECONTROLLER serialNumber\n" .
        "\n" .
        "  serialNumber       Serial number of the controller to activate.\n";

##############################################################################
# short description
    $help{VCGADDCONTROLLER}{SHORT} =
        "Adds slave controller to virtual controller group.";

# long description
    $help{VCGADDCONTROLLER}{LONG} =
        "VCGADDCONTROLLER ipAddress\n" .
        "\n" .
        "  ipAddress          IP Address of the controller (nnn.nnn.nnn.nnn).\n";

##############################################################################
# short description
    $help{VCGAPPLYLICENSE}{SHORT} =
        "Apply license information to virtual controller group.";

# long description
    $help{VCGAPPLYLICENSE}{LONG} =
        "VCGAPPLYLICENSE vcgid max_controllers\n" .
        "\n" .
        "  vcgid (Serial #)   Virtual controller group identifier.\n" .
        "  max_controllers    Maximum number of controllers for this group.\n";

##############################################################################
# short description
    $help{VCGCONFIGURECONTROLLER}{SHORT} =
        "Configure defaults for a controller.";

# long description
    $help{VCGCONFIGURECONTROLLER}{LONG} =
        "VCGCONFIGURECONTROLLER ipAddress subnetMask defaultGateway dscId nodeId replacementFlag\n" .
        "\n" .
        "  ipAddress          IP Address of the controller (nnn.nnn.nnn.nnn).\n" .
        "  subnetMask         Subnet Mask (nnn.nnn.nnn.nnn).\n" .
        "  defaultGateway     Default Gateway (nnn.nnn.nnn.nnn).\n" .
        "  dscID              DSC ID.\n" .
        "  nodeId             Node ID(0/1).\n" .
        "  replacFlag         Replacement Status(0/1).\n";

##############################################################################
# short description
    $help{VCGDOELECTION}{SHORT} =
        "Starts an election.";

# long description
    $help{VCGDOELECTION}{LONG} =
        "VCGDOELECTION\n";

##############################################################################
# short description
    $help{VCGELECTIONSTATE}{SHORT} =
        "Displays the state of an election.";

# long description
    $help{VCGELECTIONSTATE}{LONG} =
        "VCGELECTIONSTATE\n";

##############################################################################
# short description
    $help{VCGFAILCONTROLLER}{SHORT} =
        "Fail a controller that is part of this group.";

# long description
    $help{VCGFAILCONTROLLER}{LONG} =
        "VCGFAILCONTROLLER serialNumber\n" .
        "\n" .
        "  serialNumber       Serial number of the controller to fail.\n";

##############################################################################
# short description
        $help{VCGINACTIVATECONTROLLER}{SHORT} =
        "Inactivate a controller that is part of this group.";

# long description
    $help{VCGINACTIVATECONTROLLER}{LONG} =
        "VCGINACTIVATECONTROLLER serialNumber\n" .
        "\n" .
        "  serialNumber       Serial number of the controller to inactivate.\n";

##############################################################################
# short description
        $help{VCGINFO}{SHORT} =
        "Displays virtual controller group information.";

# long description
    $help{VCGINFO}{LONG} =
        "VCGINFO\n";

##############################################################################
# short description
        $help{VCGMPLIST}{SHORT} =
        "Displays the virtual controller group mirror partner list.";

# long description
    $help{VCGMPLIST}{LONG} =
        "VCGMPLIST\n";

##############################################################################
# short description
        $help{VCGPING}{SHORT} =
        "Performs Ping from one controller in VCG to another.";

# long description
    $help{VCGPING}{LONG} =
        "VCGPING serialNumber\n" .
        "\n" .
        "  serialNumber      Serial number of controller to Ping\n";

##############################################################################
# short description
    $help{VCGREMOVECONTROLLER}{SHORT} =
        "Remove a controller that is part of this group.";

# long description
    $help{VCGREMOVECONTROLLER}{LONG} =
        "VCGREMOVECONTROLLER ipAddress\n" .
        "\n" .
        "  ipAddress          IP Address of the controller (nnn.nnn.nnn.nnn).\n";

##############################################################################
# short description
    $help{VCGSHUTDOWN}{SHORT} =
        "Shutdown the VCG.";

# long description
    $help{VCGSHUTDOWN}{LONG} =
        "VCGSHUTDOWN\n";

##############################################################################
# short description
    $help{VCGUNFAILCONTROLLER}{SHORT} =
        "Unfail a controller that is part of this group but is currently in the failed state.";

# long description
    $help{VCGUNFAILCONTROLLER}{LONG} =
        "VCGUNFAILCONTROLLER serialNumber\n" .
        "\n" .
        "  serialNumber       Serial number of the controller to unfail.\n";

##############################################################################
# short description
    $help{VCGVALIDATION}{SHORT} =
        "Starts group redundancy validation on this group.";

# long description
    $help{VCGVALIDATION}{LONG} =
        "VCGVALIDATION\n" .
        "                     Starts group redundancy validation on this group.\n" .
        "  -I   Run Immediately\n" .
        "  -A   Valiate ALL\n" .
        "  -N   Valiate as Normal\n" .
        "  -D   Valiate as Daily\n" .
        "  -B   Valiate Back End\n" .
        "  -W   Valiate Hardware\n" .
        "  -S   Valiate Storage\n" .
        "  -E   Valiate Servers\n" .
        "  -C   Valiate Communications\n" .
        "  -L   Valiate BE loop\n" .
        "  -F   Valiate Bay/ISE Shelf ID\n";

##############################################################################
# short description
    $help{VCGVALIDATECONTROLLER}{SHORT} =
        "Validate a potential slave to make sure it can be added to our group.";

# long description
    $help{VCGVALIDATECONTROLLER}{LONG} =
        "VCGVALIDATECONTROLLER ipAddress\n" .
        "\n" .
        "  ipAddress          IP Address of the controller (nnn.nnn.nnn.nnn).\n";

##############################################################################
# short description
    $help{VDISKCONTROL}{SHORT} =
        "Control a virtual disk.";

# long description
    $help{VDISKCONTROL}{LONG} =
        "VDISKCONTROL operation src_vid dest_vid\n" .
        "\n" .
        "  operation          Control operation to perform.\n" .
        "                       MOVE_VDISK              0x00 -- not on 4000 | 7000 | 4700 | 7400\n" .
        "                       COPY_BREAK              0x01\n" .
        "                       COPY_SWAP               0x02\n" .
        "                       COPY_CONTINUOUS         0x03\n" .
        "                       APPEND_SRC_TO_DEST      0x04\n" .
        "                       BREAK_SPECIFIED_COPY    0x05\n" .
        "                       PAUSE_COPY              0x06\n" .
        "                       RESUME_COPY             0x07\n" .
        "                       ABORT_COPY              0x08\n" .
        "                       SNAPSHOT_CREATE         0x0A  (10 decimal)\n" .
        "                       BREAK_ALL_COPIES        0x0E\n" .
        "  src_vid            Source virtual disk.\n" .
        "  dest_vid           Destination virtual disk.\n";

##############################################################################
# short description
    $help{VDISKCOUNT}{SHORT} =
        "Displays the number of virtual disks.";

# long description
    $help{VDISKCOUNT}{LONG} =
        "VDISKCOUNT\n";

##############################################################################
# short description
    $help{VDISKCREATE}{SHORT} =
        "Create a virtual disk.";

# long description
    $help{VDISKCREATE}{LONG} =
        "VDISKCREATE [-v VID] capacity physicalDisks [raidType] [stripe] [mirrorDepth] [parity] [maxRaids] [threshold] [flags] [minPD]\n" .
        "\n" .
        "  -v VID             VID to use when creating virtual disk.\n" .
        "  capacity           Capacity of the virtual disk in blocks.\n" .
        "  physicalDisks      One of the following group indicator or\n" .
        "                     individual PIDs of the physical disks you want\n" .
        "                     included in the virtual disk.  This\n" .
        "                     is a comma separated list of values and ranges\n" .
        "                     (i.e. 1,2,4-7,10,13-18).\n" .
        "                     Physical Disk Type Groups\n" .
        "                       ALL     = PIDs for ALL DATA disks\n" .
        "                       FC      = PIDs for Fibre Channel DATA disks\n" .
        "                       SATA    = PIDs for Serial ATA DATA disks\n" .
        "                       SSD     = PIDs for Solid State DATA disks\n" .
        "                       ECONENT = PIDs for Economy Enterprise DATA disks\n" .
        "  raidType           Raid type of the virtual disk.  Valid\n" .
        "                     values are (default 0):\n" .
        "                       0 = RAID_NONE\n" .
        "                       1 = RAID_0\n" .
        "                       2 = RAID_1\n" .
        "                       3 = RAID_5\n" .
        "                       4 = RAID_10\n" .
        "  stripe             Stripe size in sectors (default 512).\n" .
        "  mirrorDepth        Mirror depth (default 2).\n" .
        "  parity             Parity for the raid (3, 5, 9).\n" .
        "  maxRaids           Max RAIDs in this operation (default 4).\n" .
        "  threshold          Minimum space on each disk in MB (default 10).\n" .
        "  flags              Use flag bits defined below (default 0).\n" .
        "                       0x01 = Exact minimum PDisks (argument)\n" .
        "                       0x02 = Bay & bus redundancy\n" .
        "                       0x04 = GeoRAID\n" .
        "                       0x08 = 7000/4700, try min PDisks, loop to 16, or works\n" .
        "                       0x10 = Make vdisk create size be what is passed in capacity\n" .
        "  minPD              Min # of PDisks per RAID create (default 0).\n" .
        "\n" .
        "  Valid Values:\n" .
        "  RAID_NONE  -   No Requirements\n" .
        "  RAID_0     -   Must have 2 or more physical disks\n" .
        "                 Stripe size between 8 - 512 by powers of 2\n" .
        "  RAID_1     -   Must have 2 or more physical disks\n" .
        "                 Mirror depth must = the number of physical disks\n" .
        "  RAID_5     -   Number of physical disk must be >= the Parity\n" .
        "                 Parity must be 3, 5, or 9\n" .
        "                 Stripe size between 8 - 64 by powers of 2\n" .
        "  RAID_10    -   Must have 2 or more physical disks\n" .
        "                 Mirror depth must <= the number of physical disks\n" .
        "                 Stripe size between 8 - 512 by powers of 2\n";

##############################################################################
# short description
    $help{VDISKDELETE}{SHORT} =
        "Delete a virtual disk.";

# long description
    $help{VDISKDELETE}{LONG} =
        "VDISKDELETE vdiskid\n" .
        "\n" .
        "  vdiskid            A comma separated list of values and ranges\n" .
        "                     of virtual disk identifiers or ALL\n" .
        "                     (i.e. 1,2,4-7,10,13-18).\n";

##############################################################################
# short description
    $help{VDISKEXPAND}{SHORT} =
        "Expand a virtual disk.";

# long description
    $help{VDISKEXPAND}{LONG} =
        "VDISKEXPAND vdiskid capacity physicalDisks [raidType] [stripe] [mirrorDepth] [parity] [maxRaids] [threshold] [flags] [minPD]\n" .
        "\n" .
        "  vdiskid            Virtual disk identifier.\n" .
        "  capacity           Capacity of the virtual disk in blocks.\n" .
        "  physicalDisks      One of the following group indicator or\n" .
        "                     individual PIDs of the physical disks you want\n" .
        "                     included in the virtual disk.  This\n" .
        "                     is a comma separated list of values and ranges\n" .
        "                     (i.e. 1,2,4-7,10,13-18).\n" .
        "                     Physical Disk Type Groups\n" .
        "                       ALL     = PIDs for ALL DATA disks\n" .
        "                       FC      = PIDs for Fibre Channel DATA disks\n" .
        "                       SATA    = PIDs for Serial ATA DATA disks\n" .
        "                       SSD     = PIDs for Solid State DATA disks\n" .
        "                       ECONENT = PIDs for Economy Enterprise DATA disks\n" .
        "  raidType           Raid type of the virtual disk.  Valid\n" .
        "                     values are (default 0):\n" .
        "                       0 = RAID_NONE\n" .
        "                       1 = RAID_0\n" .
        "                       2 = RAID_1\n" .
        "                       3 = RAID_5\n" .
        "                       4 = RAID_10\n" .
        "  stripe             Stripe size in sectors (default 512).\n" .
        "  mirrorDepth        Mirror depth (default 2).\n" .
        "  parity             Parity for the raid (3, 5, 9).\n" .
        "  maxRaids           Max RAIDs in this operation (default 4).\n" .
        "  threshold          Minimum space on each disk in MB (default 10).\n" .
        "  flags              Use flag bits defined below (default 0).\n" .
        "                       0x01 = Exact minimum PDisks (argument)\n" .
        "                       0x02 = Bay & bus redundancy\n" .
        "                       0x04 = GeoRAID\n" .
        "                       0x08 = 7000/4700, try min PDisks, loop to 16, or works\n" .
        "  minPD              Min # of PDisks per RAID create (default 0).\n" .
        "\n" .
        "  Valid Values:\n" .
        "  RAID_NONE  -   No Requirements\n" .
        "  RAID_0     -   Must have 2 or more physical disks\n" .
        "                 Stripe size between 8 - 512 by powers of 2\n" .
        "  RAID_1     -   Must have 2 or more physical disks\n" .
        "                 Mirror depth must = the number of physical disks\n" .
        "  RAID_5     -   Number of physical disk must be >= the Parity\n" .
        "                 Parity must be 3, 5, or 9\n" .
        "                 Stripe size between 8 - 64 by powers of 2\n" .
        "  RAID_10    -   Must have 2 or more physical disks\n" .
        "                 Mirror depth must <= the number of physical disks\n" .
        "                 Stripe size between 8 - 512 by powers of 2\n";

##############################################################################
# short description
    $help{VDISKINFO}{SHORT} =
        "Displays virtual disk information.";

# long description
    $help{VDISKINFO}{LONG} =
        "VDISKINFO vdiskid\n" .
        "\n" .
        "  vdiskid            Virtual disk identifier.\n";

##############################################################################
# short description
    $help{VDISKLBACONVERT}{SHORT} =
        "Convert a virtual disk LBA to a physical disk LBA.";

# long description
    $help{VDISKLBACONVERT}{LONG} =
        "VDISKLBACONVERT vdiskid vdiskLBA\n" .
        "\n" .
        "  vdiskid            Virtual disk identifier.\n" .
        "  vdiskLBA           Virtual disk LBA.\n";

##############################################################################
# short description
    $help{VDISKLIST}{SHORT} =
        "Displays a list of virtual disk identifiers.";

# long description
    $help{VDISKLIST}{LONG} =
        "VDISKLIST\n";

##############################################################################
# short description
    $help{VDISKOWNER}{SHORT} =
        "Displays the owners of a virtual disk.";

# long description
    $help{VDISKOWNER}{LONG} =
        "VDISKOWNER vdiskid\n" .
        "\n" .
        "  vdiskid            Virtual disk identifier.\n";

##############################################################################
# short description
    $help{VDISKPREPARE}{SHORT} =
        "Prepare a virtual disk.";

# long description
    $help{VDISKPREPARE}{LONG} =
        "VDISKPREPARE capacity physicalDisks [raidType] [stripe] [mirrorDepth] [parity] [maxRaids] [threshold] [flags] [minPD]\n" .
        "\n" .
        "  capacity           Capacity of the virtual disk in blocks.\n" .
        "  physicalDisks      One of the following group indicator or\n" .
        "                     individual PIDs of the physical disks you want\n" .
        "                     included in the virtual disk.  This\n" .
        "                     is a comma separated list of values and ranges\n" .
        "                     (i.e. 1,2,4-7,10,13-18).\n" .
        "                     Physical Disk Type Groups\n" .
        "                       ALL     = PIDs for ALL DATA disks\n" .
        "                       FC      = PIDs for Fibre Channel DATA disks\n" .
        "                       SATA    = PIDs for Serial ATA DATA disks\n" .
        "                       SSD     = PIDs for Solid State DATA disks\n" .
        "                       ECONENT = PIDs for Economy Enterprise DATA disks\n" .
        "  raidType           Raid type of the virtual disk.  Valid\n" .
        "                     values are (default 0):\n" .
        "                       0 = RAID_NONE\n" .
        "                       1 = RAID_0\n" .
        "                       2 = RAID_1\n" .
        "                       3 = RAID_5\n" .
        "                       4 = RAID_10\n" .
        "  stripe             Stripe size in sectors (default 512).\n" .
        "  mirrorDepth        Mirror depth (default 2).\n" .
        "  parity             Parity for the raid (3, 5, 9).\n" .
        "  maxRaids           Max RAIDs in this operation (default 4).\n" .
        "  threshold          Minimum space on each disk in MB (default 10).\n" .
        "  flags              Use flag bits defined below (default 0).\n" .
        "                       0x01 = Exact minimum PDisks (argument)\n" .
        "                       0x02 = Bay & bus redundancy\n" .
        "                       0x04 = GeoRAID\n" .
        "                       0x08 = 7000/4700, try min PDisks, loop to 16, or works\n" .
        "  minPD              Min # of PDisks per RAID create (default 0).\n" .
        "\n" .
        "  Valid Values:\n" .
        "  RAID_NONE  -   No Requirements\n" .
        "  RAID_0     -   Must have 2 or more physical disks\n" .
        "                 Stripe size between 8 - 512 by powers of 2\n" .
        "  RAID_1     -   Must have 2 or more physical disks\n" .
        "                 Mirror depth must = the number of physical disks\n" .
        "  RAID_5     -   Number of physical disk must be >= the Parity\n" .
        "                 Parity must be 3, 5, or 9\n" .
        "                 Stripe size between 8 - 64 by powers of 2\n" .
        "  RAID_10    -   Must have 2 or more physical disks\n" .
        "                 Mirror depth must <= the number of physical disks\n" .
        "                 Stripe size between 8 - 512 by powers of 2\n";

##############################################################################
# short description
    $help{VDISKPRCLR}{SHORT} =
        "Clears persistent reserve information for a specified Vdisk.";

# long description
    $help{VDISKPRCLR}{LONG} =
        "VDISKPRCLR     VID\n";

##############################################################################
# short description
    $help{VDISKPRGET}{SHORT} =
        "Return Vdisk persistent reserve information.";

# long description
    $help{VDISKPRGET}{LONG} =
        "VDISKPRGET     VID\n";

##############################################################################
# short description
    $help{VDISKS}{SHORT} =
        "Displays virtual disk information for all virtual disks.";

# long description
    $help{VDISKS}{LONG} =
        "VDISKS [display_type]\n" .
        "\n" .
        "  display_type       What information to display (default STD).\n" .
        "                       STD      - Standard display\n" .
        "                       TIME     - VDisk create time and last access time Information\n" .
        "                       STATS    - VDisk statistics Information\n" .
        "                       GEOFLAGS - GEORAID Flags related Information\n";

##############################################################################
# short description
    $help{VDISKSCACHE}{SHORT} =
        "Displays virtual disk information for all virtual disks from Cache.";

# long description
    $help{VDISKSCACHE}{LONG} =
        "VDISKSCACHE [display_type]\n" .
        "\n" .
        "  display_type       What information to display (default STD).\n" .
        "                       STD      - Standard display\n" .
        "     DOES NOT WORK!!   TIME     - VDisk create time and last access time Information\n" .
        "                       STATS    - VDisk statistics Information\n" .
        "                       GEOFLAGS - GEORAID Flags related Information\n";

##############################################################################
# short description
    $help{VDISKSETATTRIBUTES}{SHORT} =
        "Sets the caching mode for a virtual disk.";

# long description
    $help{VDISKSETATTRIBUTES}{LONG} =
        "VDISKSETATTRIBUTES vdiskid mode\n" .
        "\n" .
        "  vdiskid            Virtual disk identifier.\n" .
        "  mode               new attributes (Hex 0x0000 - 0xFFFF).\n" .
        "    15 = Lock                                      0x8000\n" .
        "    14 = Instant mirror bit                        0x4000\n" .
        "    13 = Snappool                                  0x2000\n" .
        "    12 = Backend Busy                              0x1000\n" .
        "    11 = In use by X1                              0x0800\n" .
        "    10 = In use by X1                              0x0400\n" .
        "     9 = In use by X1                              0x0200\n" .
        "     8 = Cache enabled (1=TRUE)                    0x0100\n" .
        "     7 = VLink flag (1=TRUE)                       0x0080\n" .
        "     6 = VDisk has VLink lock applied (1=TRUE)     0x0040\n" .
        "     5 = VDisk is source copy device               0x0020\n" .
        "     4 = VDisk is destination copy device          0x0010\n" .
        "     3 = Suspended flag                            0x0008\n" .
        "     2 = Asynch (alink or apool)                   0x0004\n" .
        "   1-0 = VDisk attribute code (normal=0,hidden=1,private=2)\n";


##############################################################################
# short description
    $help{VDISKSETCACHE}{SHORT} =
        "Sets the caching mode for a virtual disk.";

# long description
    $help{VDISKSETCACHE}{LONG} =
        "VDISKSETCACHE vdiskid mode\n" .
        "\n" .
        "  vdiskid            A comma separated list of values and ranges\n" .
        "                     of virtual disk identifiers or ALL\n" .
        "                     (i.e. 1,2,4-7,10,13-18).\n" .
        "  mode               Caching mode.\n" .
        "                       0 or OFF = Caching disabled\n" .
        "                       1 or ON = Caching enabled\n";

##############################################################################
# short description
    $help{VDISKSETPRIORITY}{SHORT} =
        "Set Priority for a virtual disk.";

# long description
    $help{VDISKSETPRIORITY}{LONG} =
        "VDISKSETPRIORITY     VID,priority [VID,priority] [...] [option]\n".
        "  VID                VID of the virtual disk.\n" .
        "  priority           priority of the vdisk -\n" .
        "                       0     Low Priority\n" .
        "                       1     Medium Priority\n" .
        "                       2     High Priority\n" .
        "  option             options -\n" .
        "                       0     None\n" .
        "                       1     Set the priority of all the\n".
    "                 remaining virtual disks (not specified\n".    "                 in the list) to Low\n";

##############################################################################
# short description
    $help{SINGLEVIDSONISE}{SHORT} =
        "Displays Vdisks with single PSD/PID on ISEs.";

# long description
    $help{SINGLEVIDSONISE}{LONG} =
        "SINGLEVIDSONISE [-V] [ISE] [...]\n" .
        "  ISE                The ISE/BAY number (like 8)\n";

##############################################################################
# short description
    $help{VIDSONDATAPACS}{SHORT} =
        "Displays ISES, datapacs and associated PIDS.";

# long description
    $help{VIDSONDATAPACS}{LONG} =
        "VIDSONDATAPACS [-V] [ISE DATAPAC] [...]\n" .
        "  -V                 Display LUNs, PIDS, RAIDS too (capital V).\n" .
        "  ISE                The ISE/BAY number (like 8)\n" .
        "  DATAPAC            The DATAPAC numer (1 or 2)\n";

##############################################################################
# short description
    $help{VLINKBREAKLOCK}{SHORT} =
        "Check for virtual links and clean them up if inactive.";

# long description
    $help{VLINKBREAKLOCK}{LONG} =
        "VLINKBREAKLOCK virtual_disk_id\n" .
        "\n" .
        "  virtual_disk_id     Virtual Disk identifier.\n";

##############################################################################
# short description
    $help{VLINKCREATE}{SHORT} =
        "Creates a virtual link.";

# long description
    $help{VLINKCREATE}{LONG} =
        "VLINKCREATE [-v VID] controller virtual_disk\n" .
        "  -v VID            VID to use when creating virtual disk.\n" .
        "\n" .
        "  controller        Controller Index Number.\n" .
        "  virtual_disk      Virtual disk ordinal.\n";

##############################################################################
# short description
    $help{VLINKCTRLCOUNT}{SHORT} =
        "Displays the count of remote controllers.";

# long description
    $help{VLINKCTRLCOUNT}{LONG} =
        "VLINKCTRLCOUNT\n";

##############################################################################
# short description
    $help{VLINKCTRLINFO}{SHORT} =
        "Displays the information for a remote controller.";

# long description
    $help{VLINKCTRLINFO}{LONG} =
        "VLINKCTRLINFO controller_id\n" .
        "\n" .
        "  controller_id     Controller identifier.\n";

##############################################################################
# short description
    $help{VLINKCTRLVDISKS}{SHORT} =
        "Displays the virtual disks of a remote controller.";

# long description
    $help{VLINKCTRLVDISKS}{LONG} =
        "VLINKCTRLVDISKS controller_id\n" .
        "\n" .
        "  controller_id     Controller identifier.\n";

##############################################################################
# short description
    $help{VLINKDLINKINFO}{SHORT} =
        "Displays the information of a virtual link (X1 DLink) [RID].";

# long description
    $help{VLINKDLINKINFO}{LONG} =
        "VLINKDLINKINFO vlink_id\n" .
        "\n" .
        "  vlink_id          Virtual link identifier (RID).\n";

##############################################################################
# short description
    $help{VLINKDLOCKINFO}{SHORT} =
        "Displays the information of a virtual link (X1 DLock) [RID].";

# long description
    $help{VLINKDLOCKINFO}{LONG} =
        "VLINKDLOCKINFO vlink_id\n" .
        "\n" .
        "  vlink_id          Virtual link identifier (RID).\n";

##############################################################################
# short description
    $help{VLINKINFO}{SHORT} =
        "Displays the information of a virtual link.";

# long description
    $help{VLINKINFO}{LONG} =
        "VLINKINFO vlink_id\n" .
        "\n" .
        "  vlink_id          Virtual link identifier.\n";

##############################################################################
# short description
    $help{VLINKS}{SHORT} =
        "Displays the configured virtual links.";

# long description
    $help{VLINKS}{LONG} =
        "VLINKS\n";

##############################################################################
# short description
    $help{WRITEBUFFER}{SHORT} =
        "Download code to drive bay or physical disk.";

# long description
    $help{WRITEBUFFER}{LONG} =
        "WRITEBUFFER type ordinal(s) filename\n" .
        "\n" .
        "  type (PDISK|BAY)  which device type the ordinal(s) are associated.\n" .
        "  ordinal           an ordinal related to the type, or a comma seperated\n" .
        "                    list of ordinal ranges i.e. (1,2,4-7,10,13-18) NO SPACES.\n" .
        "  filename          The name of the file to download.\n";

##############################################################################
# short description
    $help{WRITENAME}{SHORT} =
        "Write a component name to the file system.";

# long description
    $help{WRITENAME}{LONG} =
        "WRITENAME component_fid  component_id  name\n" .
        "\n" .
        "  component_fid      VDISK=10, VLINKRMTCTRL=12, CONTROLLER=13, VLINKRMTVDISK=18\n" .
        "  component_id       ID for the component\n" .
        "  name               ASCII string\n";

##############################################################################
# short description
    $help{"!SHELL-CMD"}{SHORT} =
        "Run an external shell command.";

# long description
    $help{"!SHELL-CMD"}{LONG} =
        "! shell-command\n";

##############################################################################
}

use Math::BigFloat;

# The following is here for bigint.
##############################################################################
my @file;   # The whole NVRAM BE file.

my %PDR;    # Physical Disk Record in NVRAM.

my %PIDS_IN_USE;        # Key set for each PID that is in any raid.
my %RAID_SEGLTH;        # RAID record, segment length
my %RAID_PID;           # RAID record, array of PIDs
my %RAID_STARTLBA;      # RAID record, array of starting LBAs.
my %VIDS_IN_USE;        # VIDS in use are the keys, the values are join(',', @raids);
my %VDISK_RAIDS;        # All raids in use are keys, values are vdisk that used it.
my %VIDS_CAPACITY;      # All vids are keys, values are capacity of vdisk.

#-----------------------------------------------------------------------------
sub parse_input_file_PDR()
{
    my @physical_drive_record;
    @physical_drive_record = grep {/--- Begin physical drive record structure ---------------------------/} @file;
    undef(%PDR);
    foreach (@physical_drive_record)
    {
        my @line = split("\n");
        my ($pid, $prodid, $capacity, $wwn, $lun, $ses, $slot, $p, $d);
        ($pid) =                $line[1]  =~ /^Physical device IDs    0x(....)/;
        $pid = hex($pid);
        if (defined($PDR{$pid}))
        {
            die ("Error, PID $pid defined twice\n");
        }
        ($prodid) =             $line[6]  =~ /^Product ID             (.*)$/;
        ($capacity) =           $line[7]  =~ /^Drive Capacity         ([0-9.]*) GB/;
        # Note: If not a seagate normally numbered drive (size after ST[0-9]), there is no capacity line present.
        if (defined($capacity))
        {
            ($wwn) =                $line[10] =~ /^World wide name        ([0-9a-fA-F]*)/;
            ($lun) =                $line[11] =~ /^LUN                    0x([0-9a-fA-F]*)/;
            ($slot, $ses, $d, $p) = $line[13] =~ /^DName                  0x(..)(..)(..)(..)/;
        }
        else
        {
            $capacity = 0;
            ($wwn) =                $line[9] =~ /^World wide name        ([0-9a-fA-F]*)/;
            ($lun) =                $line[10] =~ /^LUN                    0x([0-9a-fA-F]*)/;
            ($slot, $ses, $d, $p) = $line[12] =~ /^DName                  0x(..)(..)(..)(..)/;
        }
        if ($p != 50 || $d != 44)
        {
            printf "??? %3d SES-SLOT %02d-%02d LUN %2d WWN %s size %7.2f ProductID %s\n",
                $pid, $ses, $slot, $lun, $wwn, $capacity, $prodid;
        }
        else
        {
            my %P;
            $P{SES} = $ses;
            $P{SLOT} = $slot;
            $P{LUN} = $lun;
            $P{WWN} = $wwn;
            $P{CAPACITY} = $capacity;
            $P{PRODID} = $prodid;
            $PDR{$pid} = \%P;
        }
    }
}   # End of parse_input_file_PDR

#-----------------------------------------------------------------------------
sub parse_input_file_RAID()
{
    my @raid_record;
    @raid_record = grep {/--- Begin RAID record structure -------------------------------------/} @file;
    undef(%PIDS_IN_USE);
    undef(%RAID_SEGLTH);
    undef(%RAID_PID);
    undef(%RAID_STARTLBA);
    foreach (@raid_record)
    {
        my ($rid, $type, $seglthp, $seglth, $psd, $pid, $startlba);
        my @line = split("\n");
        ($rid) =                $line[1]  =~ /^RAID ID                0x(....)/;
        $rid = hex($rid);
        if (defined($RAID_SEGLTH{$rid}))
        {
            die ("Error, raid $rid defined twice\n");
        }
        ($type) =               $line[2]  =~ /^Type                   0x(..)/;
        $type = hex($type);
        if ($type != 0 && $type != 1 && $type != 2 && $type != 3 && $type != 4)
        {
            next;               # If not raid standard, 0, 1, 5, 10 then vlink or snapshot and ignore.
        }
        my %seglth;
        ($seglth) =             $line[9]  =~ /^Segment length \/ 256   0x([0-9a-fA-F]*)/;
        $seglth = hex($seglth) | (0 << 32); # Make 32 bit machine convert this to 64 bit value.

        my $i = 0;
        my @raid_pid;
        my @raid_startlba;
        while (defined($line[13+$i]))
        {
            ($psd, $pid, $startlba) = $line[13+$i]  =~ /^ *([0-9]*)   0x([0-9a-fA-F]*)  *0x..  *0x..  *0x([0-9a-fA-F]*)/;
            if ($psd != $i)
            {
                die("PSD not in order for raid $rid\n");
            }
            $pid = hex($pid);
            $PIDS_IN_USE{$pid} = $pid;
            $startlba = hex($startlba);
            push @raid_pid, $pid;
            push @raid_startlba, $startlba;
            $i++;
        }
        $RAID_SEGLTH{$rid} = $seglth;
        $RAID_PID{$rid} = \@raid_pid;
        $RAID_STARTLBA{$rid} = \@raid_startlba;
    }
}   # end of parse_input_file_RAID()

#-----------------------------------------------------------------------------
sub parse_input_file_VDISK()
{
    my @vdisk_record;
    @vdisk_record = grep {/--- Begin Virtual Device record structure ---------------------------/} @file;
    undef(%VIDS_IN_USE);
    undef(%VDISK_RAIDS);
    undef(%VIDS_CAPACITY);
    foreach (@vdisk_record)
    {
        my ($vid, $count, $capacity, $rid, $capacityh, $capacityl);
        my @line = split("\n");
        ($vid) =                $line[1]  =~ /^VDisk device ID        ([0-9]*)/;
        if (defined($VIDS_IN_USE{$vid}))
        {
            die ("Error, vdisk $vid defined twice\n");
        }
        ($count) =              $line[2]  =~ /^VDisk deferred RAIDs   0x([0-9a-fA-F][0-9a-fA-F])/;
        if ($count ne '00')
        {
            die ("Vdisk $vid has deferred raids of 0x$count\n");
        }
        ($count) =              $line[3]  =~ /^VDisk RAID count       0x([0-9a-fA-F][0-9a-fA-F])/;
        $count = hex($count);
        ($capacityh, $capacityl) = $line[4]  =~ /^Device capacity        0x([0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F])([0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F])/;
        $capacity = (hex($capacityh) << 32) || hex($capacityl);

        my $i = 0;
        my %raid_id;
        while (defined($line[8+$i]) && $line[8+$i] =~ /^Raid ID                0x/)
        {
            ($rid) =         $line[8+$i]  =~ /^Raid ID                0x([0-9a-fA-F]*)/;
            $rid = hex($rid);
            if (defined($raid_id{$rid}))
            {
                die("Raid ID $rid defined twice for vdisk $vid\n");
            }
            $raid_id{$rid} = $rid;
            if (defined($VDISK_RAIDS{$rid}))
            {
                die("Raid ID $rid defined twice, second one for vdisk $vid, first for $VDISK_RAIDS{$rid}\n");
            }
            $VDISK_RAIDS{$rid} = $vid;
            $i++;
        }
        $VIDS_IN_USE{$vid} = join(',', keys(%raid_id));
        $VIDS_CAPACITY{$vid} = $capacity;
    }
}   # end of parse_input_file_VDISK()

#-----------------------------------------------------------------------------
sub parse_input_file()
{
    parse_input_file_PDR();     # Physical Disk Records
#-- printf "pids=%s\n", join(',', sort {$a <=> $b} keys %PDR);
# printf "PID SES-SLOT LUN\n";
# foreach my $i (sort {$a <=> $b} keys %PDR)
# {
#   my $j=$PDR{$i};
#   printf "%3d  %2d-%02d   %2d\n", $i, $$j{SES}, $$j{SLOT}, $$j{LUN};
# }

    parse_input_file_RAID();    # RAID records

# foreach my $r (sort {$a <=> $b} keys %RAID_SEGLTH)
# {
#   my $s;
#   $s = $RAID_SEGLTH{$r};
#   my $p;
#   $p = $RAID_PID{$r};
#   printf "raid=%3d #pids=%3d seglth=%10u (0x%08x)\n", $r, scalar @$p, $s, $s;
# }

    parse_input_file_VDISK();   # VDISK records

}   # End of parse_input_file()

#-----------------------------------------------------------------------------
sub list_lbas(%)
{
    my %disks = @_;
    my %GAP;

    foreach my $pid (sort {$a <=> $b} keys %PIDS_IN_USE)
    {
        my @pidsdas;
        my @gap;
        foreach my $r (sort {$a <=> $b} keys %RAID_SEGLTH)
        {
            my $p = $RAID_PID{$r};
            my $l = $RAID_STARTLBA{$r};
            for (my $i = 0; $i < scalar @$p; $i++)
            {
                if ($$p[$i] == $pid)
                {
                    push @pidsdas, $$l[$i];
                    last;
                }
            }
        }
        # Get PID out of PDR array.
        my $P = $PDR{$pid};
#        printf "----------------------------------------------------------\n";
#        printf "PID %3d WWN - 0x%16s, Drive capacity - %7.2f\n", $pid, $$P{WWN}, $$P{CAPACITY};
#        printf "  RaidID    ---- Start LBA ----    --- Segment Length ---\n";

        my $pidsda = 0;
        my $seg = 0x00040000;
        while (@pidsdas > 0)
        {
            my $lastsda = $pidsda;
            $pidsda = shift @pidsdas;
            my $gapsda = $lastsda + $seg;
            if ($pidsda > $gapsda)
            {
#                printf "**GAP** %10u (0x%08x)   %10u (0x%08x)%s\n", $gapsda, $gapsda, ($pidsda - $gapsda), ($pidsda - $gapsda),
#                    ($pidsda - $gapsda) < 20480 ? '  TINY' : '';
#  Must be greater than 10mb of space on a disk to even consider using it.
                if (($pidsda - $gapsda) >= 20480)
                {
                    push @gap, $gapsda;
                    push @gap, ($pidsda - $gapsda);
                }
            }
#            if ($pidsda < ($lastsda + $seg))
#            {
#                printf " ******* OVERLAP ERROR **********\n";
#                printf " From sector 0x%08X to 0x%08X\n", $pidsda, ($lastsda + $seg - 1);
#            }
# Go through raid records by rid, if $pidsda == $RAID_PID{$r} and $segment length, sda, pid --
            foreach my $r (sort {$a <=> $b} keys %RAID_SEGLTH)
            {
                my $p = $RAID_PID{$r};
                my $l = $RAID_STARTLBA{$r};
                for (my $i = 0; $i < scalar @$p; $i++)
                {
                    if ($$p[$i] == $pid && $$l[$i] == $pidsda)
                    {
                        $seg = $RAID_SEGLTH{$r};
#                        printf "  %3d   %10u (0x%08x)   %10u (0x%08x)\n", $r, $pidsda, $pidsda, $seg, $seg;
                        last;
                    }
                }
            }
        }

        my $capacity = 0;
        for (my $j = 0; $j < $disks{COUNT}; ++$j)
        {
            if ($pid == $disks{PDISKS}[$j]{PD_PID})
            {
                $capacity = $disks{PDISKS}[$j]{CAPACITY};
                last;
            }
        }

        my $gapsda = $pidsda + $seg;
#        if ($gapsda > $capacity)
#        {
#            printf " ******* ERROR, beyond the capacity of the disk *******\n";
#        }
        if ($gapsda < ($capacity))
        {
#            printf "**GAP** %10u (0x%08x)   %10u (0x%08x)%s\n", $gapsda, $gapsda, ($capacity - $gapsda), ($capacity - $gapsda),
#                    ($capacity - $gapsda) < 20480 ? '  TINY' : '';
#  Must be greater than 10mb of space on a disk to even consider using it.
            if (($capacity - $gapsda) >= 20480)
            {
                push @gap, $gapsda;
                push @gap, ($capacity - $gapsda);
            }
        }
# Save for use outside this routine.
        $GAP{$pid} = \@gap;
    }
    return %GAP;
}   # End of list_lbas()

#-----------------------------------------------------------------------------

sub do_fid_2_parsing(%)
{
    my %disks = @_;

    # Read the data
    my %data = $currentMgr->MPXReadFID(2);
    if (!%data)
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
    if ($data{STATUS} != 0)
    {
        print "\n";
        print "The FID read failed.  status = $data{STATUS}  $data{STATUS_MSG}\n";
        print "                      errorCode = $data{ERROR_CODE}, $data{PI_ERROR_MSG};\n",
        return;
    }

    my $clen = length($data{RD_DATA});
    if ($clen <= 32 )
    {
        print "Fid length was $clen. No data to format.\n\n";
        return;
    }
    # Get rid of the FID header
    $data{RD_DATA} = substr($data{RD_DATA}, 32);

    my $msg = DumpNVR2($data{RD_DATA}, 0);
    @file = split("\n\n", $msg);

    # Create associate arrays of the NVRAM records.
    parse_input_file();
}   # End of do_fid_2_parsing

#-----------------------------------------------------------------------------
# Name:     checkevacdatapac
# Desc:     Check for evacuating a datapac
sub checkevacdatapac()
{
    my @Bays2List;
    my @Datapacs2List;
    my $ListThese = 0;

    # Parse arguments.
    while (defined($args[$ListThese]))
    {
        $Bays2List[$ListThese/2] = $args[$ListThese];
        if (defined($args[$ListThese+1]))
        {
            if ($args[$ListThese+1] < 1 || $args[$ListThese+1] > 2)
            {
                printf "argument %d should be 1 or 2 for the datapac, not %d.\n", $ListThese+1+1, $args[$ListThese+1];
                return;
            }
        }
        $Datapacs2List[$ListThese/2] = $args[$ListThese+1];
        $ListThese += 2;
    }

    # Get current diskbay information.
    my %bays = $currentMgr->diskBays();
    if (!%bays || $bays{STATUS} != PI_GOOD)
    {
        printf "Error getting bay statuses\n";
        return;
    }

    # Get current physical disk information.
    my %disks = $currentMgr->physicalDisks();
    if (!%disks || $disks{STATUS} != PI_GOOD)
    {
        printf "Error getting pdisk statuses\n";
        return;
    }

    # Get devstat raid information.
    my %rsp = $currentMgr->deviceStatus("RD");
    if (!%rsp || $rsp{STATUS} != PI_GOOD)
    {
        printf "Error getting raid device statuses\n";
        return;
    }

    my @pdisks;
    my @dbays;
    my $i;
    my $j;
    for ($i = 0; $i < $bays{COUNT}; ++$i)
    {
        $dbays[$i]{PD_BID} = $bays{BAYS}[$i]{PD_BID};
        my $count = 0;
        for ($j = 0; $j < $disks{COUNT}; ++$j)
        {
            if ($bays{BAYS}[$i]{PD_BID} == $disks{PDISKS}[$j]{SES})
            {
                $pdisks[$i][$count]{PD_PID}   = $disks{PDISKS}[$j]{PD_PID};
                $pdisks[$i][$count]{SLOT}     = $disks{PDISKS}[$j]{SLOT};
                $pdisks[$i][$count]{SES}      = $disks{PDISKS}[$j]{SES};
                $pdisks[$i][$count]{CAPACITY} = $disks{PDISKS}[$j]{CAPACITY};
                $count++;
            }
        }
        $dbays[$i]{PD_COUNT} = $count;
        $dbays[$i]{PDISKS} = $pdisks[$i];
    }

    my @sortBays;
    my @sortDisks;
    my $i2;
    my $j2 = -1;
    my $loc = 0;
    # Sort bays, lowest to highest.
    for ($i = 0; $i < $bays{COUNT}; ++$i)
    {
        $i2 = 8589934592;                       # 2^33 -- big high number.
        for ($j = 0; $j < $bays{COUNT}; ++$j)
        {
            if ($dbays[$j]{PD_BID} < $i2 && $dbays[$j]{PD_BID} > $j2)
            {
                $loc = $j;
                $i2 = $dbays[$j]{PD_BID};
            }
        }
        $sortBays[$i] = $dbays[$loc];           # smallest Bay ID.
        $j2 = $i2;                              # Next loop find bays greater than this.

        my $k;
        my $k1;
        my $l;
        my $l1 = -1;
        my $loc1;
        # Sort slot in bay (LUN), lowest first.
        for ($k = 0; $k < $sortBays[$i]{PD_COUNT}; ++$k)
        {
            $k1 = 8589934592;                   # 2^33 -- big high number.
            for ($l = 0; $l < $sortBays[$i]{PD_COUNT}; ++$l)
            {
                if ($sortBays[$i]{PDISKS}[$l]{SLOT} < $k1 && $sortBays[$i]{PDISKS}[$l]{SLOT} > $l1)
                {
                    $loc1 = $l;
                    $k1 = $sortBays[$i]{PDISKS}[$l]{SLOT};
                }
            }
            $pdisks[$k] = $sortBays[$i]{PDISKS}[$loc1];
            $l1 = $k1;
        }
        $sortBays[$i]{PDISKS} = [@pdisks];
    }


    # Print things below.

    my $flag;
    my %rids;
    my %vids;
    my $pi;
    my $pj;
    my $v_msg;
    my $r_msg;
    my $v_evac = '';
    my $r_evac = '';
    for ($i = 0; $i < $bays{COUNT}; ++$i)
    {
        my $dp;
        for ($dp = 1; $dp <= 2; $dp++)
        {
            $flag = 0;
            undef(%rids);
            undef(%vids);
            for ($j = 0; $j < $sortBays[$i]{PD_COUNT}; ++$j)
            {
                if (($sortBays[$i]{PDISKS}[$j]{SLOT} % 2) == 2-$dp)
                {
                    if ($opt_V)
                    {
                        if ($flag != 0)
                        {
                            printf "     $dp";
                        }
                        else
                        {
                            printf "ISE DP LUN PID\n";
                            printf "--- -- --- ---\n";
                            printf "%2d   $dp", $sortBays[$i]{PD_BID};
                        }
                        $flag = 1;
                        printf "%3d %4d\n", $sortBays[$i]{PDISKS}[$j]{SLOT}, $sortBays[$i]{PDISKS}[$j]{PD_PID};

                    }
                    # Find raids that contain this PID.
                    for $pi (0..$#{$rsp{LIST}})
                    {
                        for ($pj = 0; $pj < $rsp{LIST}[$pi]{PSDCNT}; $pj++)
                        {
                            if ($rsp{LIST}[$pi]{PIDS}[$pj]{PID} == $sortBays[$i]{PDISKS}[$j]{PD_PID})
                            {
                                $rids{$rsp{LIST}[$pi]{RID}} = 0;
                                last;
                            }
                        }
                    }

                    # Find VDisks that contain this PID.
                    for $pi (0..$#{$rsp{LIST}})
                    {
                        for ($pj = 0; $pj < $rsp{LIST}[$pi]{PSDCNT}; $pj++)
                        {
                            if ($rsp{LIST}[$pi]{PIDS}[$pj]{PID} == $sortBays[$i]{PDISKS}[$j]{PD_PID})
                            {
                                $vids{$rsp{LIST}[$pi]{VID}} = 0;
                                last;
                            }
                        }
                    }
                }
            }
            $r_msg = join(',', sort {$a <=> $b} keys %rids);
            $v_msg = join(',', sort {$a <=> $b} keys %vids);
            my $printit = 0;
            if ($ListThese != 0)
            {
                for (my $z = 0; $z < $ListThese/2; $z++)
                {
# printf "bay=%s Bays2List[$z]=%s dp=%d   Datapacs2List[$z]=%d\n", $sortBays[$i]{PD_BID}, $Bays2List[$z], $dp, $Datapacs2List[$z];
                    if ($sortBays[$i]{PD_BID} == $Bays2List[$z])
                    {
                        if (!defined($Datapacs2List[$z]) || $Datapacs2List[$z] == $dp)
                        {
                            $printit = 1;
                            last;
                        }
                    }
                }
            }
            else
            {
                $printit = 1;
            }
            if ($v_msg ne '' && $printit == 1)
            {
                if ($opt_V)
                {
                    printf "ISE %d  DP $dp   Raids:  %s\n", $sortBays[$i]{PD_BID}, $r_msg;
                }
                printf "ISE %d  DP $dp   VDisks: %s\n", $sortBays[$i]{PD_BID}, $v_msg;
                if ($r_evac eq '')
                {
                    $r_evac = $r_msg;
                }
                else
                {
                    $r_evac .= ',' . $r_msg;
                }
                if ($v_evac eq '')
                {
                    $v_evac = $v_msg;
                }
                else
                {
                    $v_evac .= ',' . $v_msg;
                }
                if ($opt_V)
                {
                    printf "\n";
                }
            }
        }
    }
    my @v_evac = sort { $a <=> $b } uniq(split(',',$v_evac));
    my @r_evac = sort { $a <=> $b } uniq(split(',',$r_evac));
printf "vdisks=%s\n", join(',', @v_evac);
printf "rdisks=%s\n", join(',', @r_evac);

    do_fid_2_parsing(%disks);

# What is needed:
# * a) Each gap on all PDisks.          %GAP{$pid}
# * b) Each Vdisk needing moving.       @v_evac
# *     printf "vdisks=%s\n", join(',', @v_evac);
# *    1) Size of each vdisk needing moving.
# *             %VIDS_CAPACITY      key is vid, value is capacity.
#      2) Each raid on each PID ?? for stripe size -- to get real space used/freed.
#   c) quick check of enough space to move largest vdisk (raid calculations here ... 0/1/10/3#3-5-9).
#   d) check if enough space to do all at the same time.
#    No: print out the first one and note that this one must be done first.
#    Yes: print out the creates and vdiskcontrol's to do copy/swap's.
# etc. NOTDONEYET.

# Then need to see how big those vdisks/raids are "physical" and "virtually".
# Need to get space available -- but not on evacuating ise/datapac.
# See if can move the biggest vdisk.
#    need a "create" emulation -- given pids, will it fit.

    # Do the work
    my %GAP = list_lbas(%disks);
    my $space = Math::BigFloat->new();
    foreach $i (sort {$a <=> $b} keys %GAP)
    {
        my $gaps = $GAP{$i};
        my $count = 0;
        while ($count < scalar(@$gaps))
        {
#            printf "PID %3d gap at %10u for %10u\n", $i, @$gaps[$count], @$gaps[$count+1];
            $space->badd(@$gaps[$count+1]);
            $count += 2;
        }
    }
    my $space_bytes = Math::BigFloat->new($space);
    $space_bytes->bmul(512);
    my $space_mb = Math::BigFloat->new($space_bytes);
    $space_mb->bdiv(1024*1024);
    my $space_gb = Math::BigFloat->new($space_mb);
    $space_gb->bdiv(1024);
    my $space_tb = Math::BigFloat->new($space_gb);
    $space_tb->bdiv(1024);

    # Following limits printing to 2 decimal points for mb, gb, tb.
    my $Fl;
    my $Ff;
    ($Fl,$Ff) = $space->length();
# printf "space=%s l=%d,f=%d\n", $space, $Fl, $Ff;
    $space->accuracy($Fl - $Ff);
    ($Fl,$Ff) = $space_bytes->length();
# printf "space_bytes=%s l=%d,f=%d\n", $space_bytes, $Fl, $Ff;
    $space_bytes->accuracy($Fl - $Ff);
    ($Fl,$Ff) = $space_mb->length();
# printf "space_mb=%s l=%d,f=%d\n", $space_mb, $Fl, $Ff;
    $space_mb->accuracy($Fl - $Ff + 2);
    ($Fl,$Ff) = $space_gb->length();
# printf "space_gb=%s l=%d,f=%d\n", $space_gb, $Fl, $Ff;
    $space_gb->accuracy($Fl - $Ff + 2);
    ($Fl,$Ff) = $space_tb->length();
# printf "space_tb=%s l=%d,f=%d\n", $space_tb, $Fl, $Ff;
    $space_tb->accuracy($Fl - $Ff + 2);
printf "Total useable free blocks is %s -- %s bytes -- %s mb -- %s gb -- %s tb\n", $space->bstr(), $space_bytes->bstr(), $space_mb->bstr(), $space_gb->bstr(), $space_tb->bstr();

    # Clean up memory usage.
    undef(@file);
    undef(%PDR);
    undef(%PIDS_IN_USE);
    undef(%RAID_SEGLTH);
    undef(%RAID_PID);
    undef(%RAID_STARTLBA);
}   # End of checkevacdatapac()

#-----------------------------------------------------------------------------
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
