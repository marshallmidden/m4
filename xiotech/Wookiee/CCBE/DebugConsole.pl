#!/usr/bin/perl -w
# $Header$
#====================================================================
#
# FILE NAME:    DebugConsole.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         2/19/2002
#
# DESCRIPTION:  Captures debug data (DebugPrintf()'s) sent by the 
#               CCB.  A file can be written simultaneoulsy by simply
#               passing in a filename on the command line.
#
#====================================================================

use strict;
use Getopt::Std;
use Socket;
use IO::Handle;
use File::Basename;
use Win32::Console; 

# Windoze flag
my $windoze = ("$^O" eq "MSWin32");

# color related variables
my $OUT;
my $index = 0;
my @colors;
my %ipcolor;
my $html = 1;
my $lastHtmlColor = "";
my $resetColor;
my $startColor;
my $useColor = 1;

#
# For Windoze, these colors are imported; 
# For others (ANSI), they are assigned below.
#
our (
   $BG_BLACK,
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
# Windoze initialization
#
if ($windoze) 
{
    
    $OUT = new Win32::Console( STD_OUTPUT_HANDLE );
    
    $resetColor = $OUT->Attr();
    $startColor = $FG_WHITE|$BG_BLACK;
}
#
# ANSI initialization. Could've, perhaps should've used
# Term::ANSIColor, but this just seemed simpler...
# "man 4 console_codes" is a good source of information. 
#
else
{
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

    $resetColor = "\e[0m";
    $startColor = "$FG_WHITE" ."$BG_BLACK";
}

#
# Build the color console/html array.
# The order in which the colors are listed is the order they
# are assigned. Leave the console and html definitions grouped together.
#
@colors = ( 
    # console           html
    [($FG_WHITE,        "white")],    # i.e. LIGHTGRAY
    [($FG_YELLOW,       "yellow")],   # i.e. LIGHTBROWN
    [($FG_LIGHTBLUE,    "blue")],
    [($FG_BROWN,        "olive")],
    [($FG_CYAN,         "teal")],
    [($FG_GRAY,         "silver")],
    [($FG_LIGHTMAGENTA, "fuchsia")],
    [($FG_LIGHTCYAN,    "aqua")],
    [($FG_LIGHTGREEN,   "lime")],
    [($FG_LIGHTRED,     "red")],
    [($FG_GREEN,        "green")],
    [($FG_BLUE,         "navy")],
    [($FG_MAGENTA,      "purple")],
    [($FG_RED,          "maroon")],
);

STDOUT->autoflush(1);
STDERR->autoflush(1);

use constant DEBUG_BASE_PORT    =>  3102;
my $channel = 0;

my $date = scalar localtime();
my $outCnt = 0;

my $filen = undef;
my $fsize;
my $handle = undef;

my $filter = 0;

my %ipAddrs;
my %seqByIpAddr;

sub Usage
{
    print "\nUsage: DebugConsole.pl [-h] [-n] [-s] [-c Channel] [logfile] [IP address ...]\n\n";
    print "-h          = This help text.\n";
    print "-c Channel  = This is the channel the CCB will be talking to.  A valid.\n";
    print "              channel is 0 - 19 (default: 0).\n";
    print "-n          = [n]o html - turn off html output.\n";
    print "-o          = n[o] color - turn off color output.\n";
    print "                Tip: '-no' disables all colorization.\n";
    print "-s          = The max file size, in MB. After reaching this size,\n";
    print "              the current file is closed, renamed with the current\n";
    print "              date and time, and a new file to write data to is opened.\n";
    print "logfile     = The file to copy/append incoming messages to.\n";
    print "IP address  = One or more IP addresses to accept meessages from;\n";
    print "              messages from other addresses will be filtered out.\n";
    print "              By default, messages from any address are accepted.\n\n";

    SetColor($resetColor);
    exit;
}

our $opt_h;
our $opt_c;
our $opt_n;
our $opt_s;
our $opt_o;
getopts('hc:nos:');

if ($opt_h)
{
    Usage();
}    
if ($opt_n)
{
    $html = 0;
}    
if ($opt_o)
{
    $useColor = 0;
}    
if ($opt_s)
{
    $fsize = $opt_s;
    if (!($fsize =~ /^\d{1,}$/ and $fsize > 0))
    {
        Usage();
    }
}    
if (defined($opt_c) && ($opt_c < 0 || $opt_c >= 20))
{
    print "Invalid Channel\n";
    exit;
}
elsif (defined($opt_c))
{
    $channel = $opt_c;
}   

#
# Parse the input parameters
#
my $parm;
foreach $parm (@ARGV) 
{
    # If it looks like an IP address, filter on it
    if ($parm =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/)
    {
        $ipAddrs{$parm} = "OK";
        $filter = 1;
    }
    # the *last* parameter that looks like a file is what is opened
    elsif ($parm =~ /^[a-zA-Z.\/\\]{1,}/)
    {
        $filen = $parm;
    }
    # anything invalid triggers the usage message
    else 
    {
        Usage();
    }
}

#
# Change to a new color
#
sub SetColor
{
    if ($useColor)
    {
        my ($attr) = @_;

        if ($windoze)
        {
            $OUT->Attr($attr);
        }
        else
        {
            print $attr;
        }
    }
}

#
# Open the logging file
#
sub OpenOutFile 
{
    my ($fn) = @_;

    # Open the output file if one was requested
    my $hdrFound = 0;
    my $lineCount = 50;

    if ($html)
    {
#        if ($fn !~ /\.htm[l]{0,1}$/i)
#        {
#            $fn .= ".html";
#            warn "\nNote: Changing filename to \"$fn\"\n";
#        }

        if (-r "$fn")
        {
            if (open($handle, "$fn"))
            {
                while(<$handle>)
                {
                    if (/<html>|<head>|<title>/i)
                    {
                        $hdrFound = 1;
                    }
                    last if !$lineCount--;
                }
                close $handle;
            }
        }
    }

    if (! open($handle, ">>$fn")) 
    {
        die "\nCan't open $fn for output ...\n";
    }

    # set to autoflush
    select $handle;
    $| = 1;
    select STDOUT;

    if($html and !$hdrFound)
    {
        print $handle "<html><head><title>Debug Console</title>\n";
        print $handle "<meta http-equiv=\"Content-Type\" content=\"text/html; " .
            "charset=iso-8859-1\"></head>\n";
        print $handle "<body bgcolor=\"black\" text=\"white\">\n";
        print $handle "<pre><font face=\"terminal\"><font color=\"$colors[0][1]\">";
    }

    $date = scalar localtime();
    print $handle "\n===== Start: $date =====\n\n";
    
    return -s $handle;
}

#
# Re-Open the logging file, save the old one off.
#
sub ReOpenOutFile
{
    my ($fn) = @_;
    my ($base, $ext);

    if ($html)
    {
        print $handle "\n\n</body></html>\n";
    }
    
    close $handle;
    undef $handle;
   
    if (scalar ($fn =~ m/([^\.])\.([^\.])/))
    {
        $base = "$`$1";
        $ext = ".$2$'";
    }
    else
    {
        $base = "$fn"; 
        $ext = "";
    }

    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday) = localtime(time);
    my $newFileN = sprintf "%s_%u_%02u_%02u_%02u_%02u_%02u%s",
        $base,
        $year+1900, $mon+1, $mday, $hour, $min, $sec,
        $ext;
    
    rename "$fn", $newFileN or warn "Couldn't rename $fn -> $newFileN\n";
    
    print "\n\nDebugConsole Message: Saving $newFileN\n\n\n";
    return OpenOutFile($fn);
}

#
# Get timestamp routine
#
my $lastTime = 0;
my $timeStamp = "";
sub TimeStamp
{
    my ($sec, $min, $hr, $mday, $mon, $yr, $wday, $yday, $isdst);
    my $temp = time();
    
    # Only run through the localtime() and sprintf() if time() has changed.
    # This should speed up the timestamp considerably (2.6x on average, in fact!)
    if ($temp != $lastTime)
    {
        $lastTime = $temp;
        ($sec, $min, $hr, $mday, $mon, $yr, $wday, $yday, $isdst) = localtime($temp);
        $timeStamp = sprintf("%02u/%02u-%02u:%02u:%02u", 
            $mon+1, $mday, $hr, $min, $sec);
    }
}

#
# Print subroutine.
# Must be called like printf.
#
sub TPrintf
{
    my $fmt = shift @_;
    my $str = sprintf($fmt, @_);
    my $str2;

    ###
    ## Remove the unprintable characters.
    ## The negated list is ' ' through '~' (which is the class of printables)
    ## and we'll allow linefeeds and tabs (but not carriage returns).
    ###
	$str =~ s/[^ -~\n\t]//g;

    my @colorSet;
#    if ($windoze)
    if (1)
    {
        my ($time, $ip, $seq, $txt) = @_;
        
        if (!defined($ipcolor{$ip})) 
        {
            $ipcolor{$ip} = $colors[$index++];
            $index = 0 if $index == @colors;
        }
        @colorSet = @{$ipcolor{$ip}};

        SetColor($colorSet[0]);
        print STDOUT "$str";
        SetColor($resetColor);
    }
    else
    {
        print STDOUT "$str";
    }

    if (defined $handle) 
    {
        if($html)
        {
            $str =~ s/[\r\n]*$//;
            if($lastHtmlColor eq $colorSet[1])
            {
                $str2 = "\n$str"; 
            }
            else
            {
                # push the tag out to the right a bit
                $str2 = "\t\t\t\t\t</font><font color=\"$colorSet[1]\">\n$str"; 
                $lastHtmlColor = $colorSet[1];
            }
        }
        else
        {
            $str2 = "$str"; 
        }
        print $handle $str2;
    
        # Note: because of the cr/lf in dos, this count needs to incremented by
        # 1 for every newline we encounter.  Its not worth counting them, so 
        # incrementing by 2 for each print averages out about right.
        # Note 2: getting the length of this string might be pretty slow (not
        # sure). An alternate way of getting the length of the file is to
        # count the lines we send out, and go check the size of the file every
        # couple hundred lines or so (that'd get us close enough).
        return length($str2) + 2;
    }

    return 0;
}

#
# Print the opening banner
#
SetColor($startColor);

print STDOUT  "===========================================\n";
print STDOUT  "     Welcome to the CCB Debug Console!     \n";
print STDOUT  "  Copyright 2002-2004 XIOtech Corporation  \n";
print STDOUT  "           For Internal Use Only           \n";
print STDOUT  "===========================================\n";

print STDOUT "\n===== Start: $date =====\n\n";
if (defined($filen)) 
{
    $outCnt = OpenOutFile($filen);

    print "Logging output to: $filen\n";
    if (defined($fsize)) 
    {
        print "Max file size: $fsize MB\n";

        # Adjust $fsize to megabytes
        $fsize *= 0x100000;
    }
}

if ($filter) 
{
    my @keys = reverse( keys(%ipAddrs) );
    print "Filtering on: @keys\n";
}

print"\n";

SetColor($resetColor);

socket(SOCKFD, AF_INET, SOCK_DGRAM, 0) or die "socket: $!";
bind(SOCKFD, sockaddr_in((DEBUG_BASE_PORT + $channel), inet_aton('0.0.0.0'))) or 
    die "bind: $!";

#
# Main rx loop
#
my $rollVal = 100;
while (1) 
{
    my ($buf, $addr, $port, $ip, $ipascii, $seq);
    
    # read the next datagram
    $addr = recv(SOCKFD, $buf, 0x10000, 0);
   
    # Grab the timestamp.  This is done as a global because it is FASTER!
    TimeStamp();
   
    # get the sender's address
    ($port, $ip) = unpack_sockaddr_in($addr);
    $ipascii = inet_ntoa($ip);
    
    chomp $buf;

    # Take into account the differences between [R1/early to mid-R2] and
    # [late-R2/R3]. If you see sequence numbers > 99, you must be watching
    # late-R2/R3 output, so switch to that mode. This variable can only be 
    # moved up, not back.  
    $seq = unpack "S", $buf;
    if ($rollVal == 100 and $seq >= 100)
    {
        $rollVal = 0x10000;
        $buf .= "\nDebugConsole Message: Switching to R3 mode ($seq)";
    }
    
    if ($filter == 0 or ($filter and defined($ipAddrs{$ipascii}))) 
    { 
        my $diff = 0;
 
        if (defined($seqByIpAddr{$ipascii}) )
        {
            if ($seq <= $seqByIpAddr{$ipascii})
            {
                $diff = ($seq + $rollVal) - $seqByIpAddr{$ipascii};
            }
            else
            {
                $diff = $seq - $seqByIpAddr{$ipascii};
            }
            
            if ( $diff > 1 )
            {
                $diff--;

                # dropped frame
                $outCnt += TPrintf("%s-%s> << %d DROPPED DATAGRAM%s >>\n", 
                        $timeStamp, $ipascii, $diff, $diff > 1 ? "S" : "");
            }
        }   
        
        $seqByIpAddr{$ipascii} = $seq;
    
        $outCnt += TPrintf("%s-%s> %s\n", $timeStamp, $ipascii, unpack("x2 a*", $buf));
        
        if (defined($fsize) and ($outCnt > $fsize))
        {
            $outCnt = ReOpenOutFile($filen);
        }
    }
}
   
