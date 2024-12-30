#!/perl/bin/perl -w

# $Id: updfwhdr.pl 68923 2008-11-18 18:27:13Z mdr $
#====================================================================
#
# FILE NAME:    UpdFwHdr.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         8/9/2000
#
# DESCRIPTION:  Modifies the firmware header file (probably "FwHeader.as")
#               with the current build time, builder's id, revision etc.  
#               It is to be called early in the build process so that
#               the firmware header can be compiled and linked while 
#               containing the proper data.
#
#====================================================================

#
# Include modules
#

use Getopt::Std;


# Function: Show Help/Usage info and exit

sub ShowHelp {
    ($script = $0) =~ s/^.*\\//;
    print "\nUsage: $script [-h] [-v <version>] infile outfile\n\n";
    print "-h: Print this help text.\n";
    print "-i: index-file.\n";
    print "-v <version>: Version label.\n";
    print "\n";
    exit 1;
}

# Clear all input parms

$opt_h = 0;
$opt_v = 0;
$revision = 0;
$revCount = 0;

# Parse the input parameters

getopts('hv:i:') or ShowHelp;

# Print the usage info if requested

if ($opt_h)
{
    ShowHelp;
}

if (defined($opt_i))
{
    open IDX, "$opt_i" or die "\nAbort: Can't open opt_i $opt_i...\n";
    my ($var, $val);

    while (<IDX>)
    {
        if (!/=/)
        {
            next;
        }

        ($var, $val) = split /\s*=\s*/;
        $var =~ s/\s+//g;
        $val =~ s/\s+//g;
        chomp $val;

        if ($var eq "FW_MAJOR_RELEASE")
        {
            if ($val =~ /0x/i)
            {
                $newMajRel = oct $val;
            }
            else
            {
                $newMajRel = $val;
            }
        }
        elsif ($var eq "FW_MINOR_RELEASE")
        {
            if ($val =~ /0x/i)
            {
                $newMinRel = oct $val;
            }
            else
            {
                $newMinRel = $val;
            }
        }
    }

    close IDX;

    if (!defined($newMajRel))
    {
        die "Index file must provide FW_MAJOR_RELEASE value";
    }
    if (!defined($newMinRel))
    {
        die "Index file must provide FW_MINOR_RELEASE value";
    }
}
else
{
    die "index file must be provided with -i";
}

# Set file names

unless (@ARGV == 2) { ShowHelp };
($fin, $fout) = @ARGV;
$fin =~ /(.*)\./ || die "Parameter $1 must contain a . character";
$fdat = $1 . ".dat";

#
# Function: Convert short integer to BCD
#
# short CvtToBCD ( short Num )
#
# Input:   The short to convert to BCD
# Returns: The converted short
#
sub CvtToBCD
{
    my $Num = $_[0];
    my $BCD = 0;
    my $sh;

    for ($sh = 0; $sh < 16; $sh += 4)
    {
        $BCD |= ($Num % 10) << $sh;
        $Num /= 10;
    }
   
    return $BCD;
}


# Get current time (GMT)
@gmt = gmtime;
$gmt[4] += 1;    # mon  -> 1-12
$gmt[5] += 1900; # year -> 2000-??
$gmt[6] += 1;    # wday -> 1-7

for ($i = 0; $i < 7; $i++)
{
    $gmt[$i] = CvtToBCD( $gmt[$i] );
}

# Get username of caller
$buildID = getlogin || getpwuid($<);
if ($?)
{
    $buildID="????";
}
$buildID .= "    ";
$buildID = substr $buildID, 0, 4;

# Declare variables

$revCount               = "";
$timestamp_year         = $gmt[5];
$timestamp_month        = $gmt[4];
$timestamp_date         = $gmt[3];
$timestamp_day          = $gmt[6];
$timestamp_hours        = $gmt[2];
$timestamp_minutes      = $gmt[1];
$timestamp_seconds      = $gmt[0];

# Open input file 
if (! -r $fin) 
{
    die "\nAbort: Can't read $fin...\n";
}
else
{
    open FIN, "$fin" or die "\nAbort: Can't open $fin...\n";
}

# Open output file for writing 
if (-r $fout) 
{
    chmod 0777, $fout or die "\nAbort: Can't chmod $fout...\n";
    unlink $fout or die "\nAbort: Can't unlink $fout...\n";
}
open FOUT, ">$fout" or die "\nAbort: Can't open $fout...\n";

# Read up dat file if it exists
if (-r $fdat) 
{
    chmod 0777, $fdat or die "\nAbort: Can't chmod $fdat...\n";
    open FDAT, "$fdat" or die "\nAbort: Can't open $fdat...\n";

    my ($var, $val);
    while (<FDAT>)
    {
        if (!/=/)
        { 
            next; 
        }
        ($var, $val) = split /\s*=\s*/;
        $var =~ s/\s+//g;
        $val =~ s/\s+//g;
        chomp $val;

        if ($var eq "REVISION")
        {
            $revision = $val;
        }
        elsif ($var eq "REVCOUNT")
        {
            $revCount = $val;
        }
    }
    
    close FDAT;
}

if (! $revision)
{
    $revision = "Engr";
}

$release = sprintf "%02X%02X", $newMajRel, $newMinRel >> 8;

# Set revCount then write out the dat file
if ($opt_v)
{
    if ($opt_v ne $revision)
    {
        $revision = $opt_v;
        $revCount = 0;
    }
}

$revision .= "    ";
$revision = substr $revision, 0, 4;

$release .= "    ";
$release = substr $release, 0, 4;

if (++$revCount > 9999)
{
    $revCount = 1;
}

open FDAT, ">$fdat" or die "\nAbort: Can't open $fdat...\n";
printf FDAT "REVISION = $revision\n";
printf FDAT "REVCOUNT = %u\n", $revCount;
close FDAT;

# Scan input file; write output file
while(<FIN>) {

    if (/PRE_PATCH_REVISION/) {
        $data = pack "A4", $revision;
        $str = unpack "L", $data;
        $data = sprintf "0x%08x%s", $str,
                    " " x (length("PRE_PATCH_REVISION") - 10);
        ( $line = $_ ) =~ s/PRE_PATCH_REVISION/$data/;
        print FOUT $line;
        next;
    }
    
    if (/PRE_PATCH_REVCOUNT/) {
        $revCount = sprintf "%04d", $revCount;
        $data = pack "A4", $revCount;
        $str = unpack "L", $data;
        $data = sprintf "0x%08x%s", $str,
                    " " x (length("PRE_PATCH_REVCOUNT") - 10);
        ( $line = $_ ) =~ s/PRE_PATCH_REVCOUNT/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_BUILDID/) {
        $data = pack "a4", $buildID;
        $str = unpack "L", $data;
        $data = sprintf "0x%08x%s", $str,
                    " " x (length("PRE_PATCH_BUILDID") - 10);
        ( $line = $_ ) =~ s/PRE_PATCH_BUILDID/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_YEAR/) {
        $data = sprintf "0x%04X%s", $timestamp_year,
                    " " x (length("PRE_PATCH_YEAR") - 6);
        ( $line = $_ ) =~ s/PRE_PATCH_YEAR/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_MONTH/) {
        $data = sprintf "0x%02X%s", $timestamp_month,
                " " x (length("PRE_PATCH_MONTH") - 4);
        ( $line = $_ ) =~ s/PRE_PATCH_MONTH/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_DATE/) {
        $data = sprintf "0x%02X%s", $timestamp_date,
                    " " x (length("PRE_PATCH_DATE") - 4);
        ( $line = $_ ) =~ s/PRE_PATCH_DATE/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_DAY/) {
        $data = sprintf "0x%02X%s", $timestamp_day,
                    " " x (length("PRE_PATCH_DAY") - 4);
        ( $line = $_ ) =~ s/PRE_PATCH_DAY/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_HOURS/) {
        $data = sprintf "0x%02X%s", $timestamp_hours,
                    " " x (length("PRE_PATCH_HOURS") - 4);
        ( $line = $_ ) =~ s/PRE_PATCH_HOURS/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_MINUTES/) {
        $data = sprintf "0x%02X%s", $timestamp_minutes,
                    " " x (length("PRE_PATCH_MINUTES") - 4);
        ( $line = $_ ) =~ s/PRE_PATCH_MINUTES/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_SECONDS/) {
        $data = sprintf "0x%02X%s", $timestamp_seconds,
                    " " x (length("PRE_PATCH_SECONDS") - 4);
        ( $line = $_ ) =~ s/PRE_PATCH_SECONDS/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_RELEASE/)
    {
        $data = pack "A4", $release;
        $str = unpack "L", $data;
        $data = sprintf "0x%08x%s", $str,
                    " " x (length("PRE_PATCH_RELEASE") - 10);
        ( $line = $_ ) =~ s/PRE_PATCH_RELEASE/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_FW_MAJ/)
    {
        $data = sprintf "0x%04x%s", $newMajRel,
                    " " x (length("PRE_PATCH_FW_MAJ") - 6);
        ( $line = $_ ) =~ s/PRE_PATCH_FW_MAJ/$data/;
        print FOUT $line;
        next;
    }

    if (/PRE_PATCH_FW_MIN/)
    {
        $data = sprintf "0x%04x%s", $newMinRel,
                    " " x (length("PRE_PATCH_FW_MIN") - 6);
        ( $line = $_ ) =~ s/PRE_PATCH_FW_MIN/$data/;
        print FOUT $line;
        next;
    }

    print FOUT;
}

close FIN;
close FOUT;

exit 0;

###
## Modelines:
## Local Variables:
## tab-width: 4
## indent-tabs-mode: nil
## End:
## vi:ts=4 sw=4 expandtab
