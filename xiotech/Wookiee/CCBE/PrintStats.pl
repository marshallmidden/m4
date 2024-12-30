#!/usr/bin/perl -w
#====================================================================
#
# FILE NAME:    PrintStats.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         3/14/2005
#
# DESCRIPTION:  
#
#====================================================================

# Simple example using 'graph':
# PrintStats.pl timsfile.bin R_RELTIME  c1.G_FELOAD | \
#                   graph -g 3 -X "Time (sec)" -Y "FE CPU Load (%)" -T X

use strict;
use	Getopt::Std;
use	Socket;
use Parse::RecDescent;

##########################################################
# math calculator/parser
# 
my $parser = new Parse::RecDescent( q{
    startrule:    calculation eofile  { $return = $item[1]; }
    statement:    grouped_op | grouped_num | number
    grouped_op:   '(' calculation ')' { $return = $item[2]; }
    grouped_num:  '(' number ')' { $return = $item[2]; }
    calculation:  add_calc | sub_calc | mult_calc | div_calc | statement
    add_calc:     statement '+' statement { $return = $item[1]+$item[3]; }
    sub_calc:     statement '-' statement { $return = $item[1]-$item[3]; }
    mult_calc:    statement '*' statement { $return = $item[1]*$item[3]; }
    div_calc:     statement '/' statement { $return = $item[1]/$item[3]; }
    number:       /\d+/
    eofile:       /^\Z/
} );

##########################################################
my $helpOnVars = <<"END_HELP_VARS";

 [G]lobal variables only require the variable 
 name itself in the specification. 
 For example: G_RELTIME
 
 G_RECORDLEN
 G_EYECATCHER
 G_TIMESTAMP
 G_RELTIME - '0' based time
 G_CCOUNT  - controller count
 G_CLIST   - controller list (array)
 
 [C]ontroller variables require a controller 
 id and the variable name in the specification. 
 For example: C0.C_BELOAD

 The controller id can be a wildcard as well: 'c*'.
 For example: C*.C_BELOAD 
 
 C_IP
 C_BELOAD
 C_FELOAD
 C_STATUS
 C_BATTERY
 C_STOPCNT
 C_SIZE
 C_MAXCWR
 C_MAXSGL
 C_NUMTAGS
 C_TAGSDIRTY
 C_TAGSRESIDENT
 C_TAGSFREE
 C_TAGSFLUSHIP
 C_NUMBLKS
 C_BLOCKSDIRTY
 C_BLOCKRESIDENT
 C_BLOCKFREE
 C_BLKSFLUSHIP
 C_SCOUNT - server count
 C_SLIST  - server list (array)
 C_VCOUNT - vdisk count
 C_VLIST  - VDISK list (array)
 C_HCOUNT - habs count
 C_HLIST  - HAB list (array)
 
 [S]erver variables require a controller id and
 a server id along with the variable name in the
 specification. For example: C0.S1.S_PER_CMDS
 
 The server id can be a wildcard as well: 's*'.
 For example: C0.S*.S_PER_CMDS 
 and: C*.S*.S_PER_CMDS 
 
 S_ID 
 S_PER_CMDS 
 S_PER_BYTES
 S_PER_WRITES
 S_PER_WBYTES
 S_PER_READS
 S_PER_RBYTES
 S_QDEPTH     
 
 [V]disk variables require a controller id and
 a vdisk id along with the variable name in the
 specification. For example: C0.V2.V_AVGSC
 
 The vdisk id can be a wildcard as well: 'V*'.
 For example: C0.V*.V_AVGSC
 and C*.V*.V_AVGSC
 
 V_ID 
 V_RPS 
 V_AVGSC
 V_WRTBYRES
 V_WRTBYLEN
 
 [H]ab variables require a controller id and
 a hab id along with the variable name in the
 specification. For example: C0.H1.H_QDEPTH
 
 The hab id can be a wildcard as well: 'H*'.
 For example: C0.H*.H_QDEPTH
 and C*.H*.H_QDEPTH
 
 H_ID
 H_PER_CMDS 
 H_QDEPTH
 H_AVG_REQ

 
END_HELP_VARS
##########################################################

use constant RECORDLEN_LEN => 2;
use constant RECORDLEN_TPL => 
   "S";     # G_RECORDLEN

use constant HEADER_LEN => 18;
use constant HEADER_TPL =>
   "A14     # G_EYECATCHER
    L";     # G_TIMESTAMP
    
use constant DEVICECNT_LEN => 2;
use constant DEVICECNT_TPL => 
   "S";     # G_CCOUNT - controller count

# The above DEVICECNT is the count of the controllers 
# in the record. The controller info extends to the end 
# of the data.
use constant STATSLOAD_LEN => 6;
use constant STATSLOAD_TPL =>
   "L       # C_IP
    C       # C_BELOAD
    C";     # C_FELOAD
    
use constant GLOBALCACHEINFO_LEN => 55;
use constant GLOBALCACHEINFO_TPL =>
   "C       # C_STATUS
    C       # C_BATTERY
    C       # C_STOPCNT
    L       # C_SIZE
    L       # C_MAXCWR
    L       # C_MAXSGL
    L       # C_NUMTAGS
    L       # C_TAGSDIRTY
    L       # C_TAGSRESIDENT
    L       # C_TAGSFREE
    L       # C_TAGSFLUSHIP
    L       # C_NUMBLKS
    L       # C_BLOCKSDIRTY
    L       # C_BLOCKRESIDENT
    L       # C_BLOCKFREE
    L";     # C_BLKSFLUSHIP

# There is a DEVICECNT_TPL in here:
#               C_SCOUNT - server count

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
#               C_VCOUNT - vdisk count
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
#               C_HCOUNT - habs count
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
our ($opt_k); 
getopts('k');
 
#
# Print out help/usage info
#
my $script = $0;
$script =~ s/\\/\//g;  # back slashes -> forward slashes
$script =~ s/^.*\///;  # get base name
if (@ARGV < 1) 
{ 
    print "\nUsage: $script [-k] filename [variable1, variable2 ...]\n";
    print "  - variables can include math operations and parenthesis for grouping.\n";
    print "  - Example: $script timsfile.bin \"G_RELTIME, (c0.c_feload+c1.c_feload)/2, c*.s*.s_per_cmds\"\n";
    print "$helpOnVars\n";
    exit 1;
}

#
# Open up the input file
#
my $file = shift @ARGV;
open *LOG, "$file" or die "\nFailed to open $file, errno $!.\n";
binmode *LOG;
my $fh = *LOG;
my $flen = -s $fh;

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
# Parse a record from the input file
#
my $absTimeOffset = 0;
sub ParseNextRecord
{
    my %data;
    my $rdata;
    my $rc;
    my $offset = 0;
    
    undef %data;
    
    my $eof = 0;
    if (($flen - tell($fh)) >= 2)
    {
        $rc = read $fh, $rdata, 2;
        if ($rc == 0)
        {
            print STDERR "File read failure, errno $!.\n";
            $eof = 1;
        }

        ( $data{G_RECORDLEN} ) = unpack RECORDLEN_TPL, $rdata; 

        if (($flen - tell($fh)) >= $data{G_RECORDLEN})
        {
            $rc = read $fh, $rdata, $data{G_RECORDLEN};
            if ($rc == 0)
            {
                print STDERR "File read failure, errno $!.\n";
                $eof = 1;
            }
        }
    }
    else
    {
        $eof = 1;
    }

    #
    # Exit if EOF
    #
    return %data if ($eof);
    
    #
    # Unpack the header
    #
    (
        $data{ G_EYECATCHER },
        $data{ G_TIMESTAMP },
    ) = unpack "x$offset".HEADER_TPL, $rdata; 
    $offset += HEADER_LEN;
    $absTimeOffset = $data{ G_TIMESTAMP } if( $absTimeOffset == 0 );
    $data{ G_RELTIME } = $data{ G_TIMESTAMP } - $absTimeOffset;

    #
    # Get the CONTROLLER count
    #
    (
        $data{ G_CCOUNT }
    ) = unpack "x$offset".DEVICECNT_TPL, $rdata; 
    $offset += DEVICECNT_LEN;
    
    #
    # Loop on controller count
    #
    my @clist = ();
    for (my $cid = 0; $cid < $data{ G_CCOUNT }; $cid++)
    {
        my $ip;
        (
            # STATSLOAD
            $ip,
            $data{ $cid }{ C_BELOAD },
            $data{ $cid }{ C_FELOAD },
            # GLOBALCACHEINFO
            $data{ $cid }{ C_STATUS },
            $data{ $cid }{ C_BATTERY },
            $data{ $cid }{ C_STOPCNT },
            $data{ $cid }{ C_SIZE },
            $data{ $cid }{ C_MAXCWR },
            $data{ $cid }{ C_MAXSGL },
            $data{ $cid }{ C_NUMTAGS },
            $data{ $cid }{ C_TAGSDIRTY },
            $data{ $cid }{ C_TAGSRESIDENT },
            $data{ $cid }{ C_TAGSFREE },
            $data{ $cid }{ C_TAGSFLUSHIP },
            $data{ $cid }{ C_NUMBLKS },
            $data{ $cid }{ C_BLOCKSDIRTY },
            $data{ $cid }{ C_BLOCKRESIDENT },
            $data{ $cid }{ C_BLOCKFREE },
            $data{ $cid }{ C_BLKSFLUSHIP }
        ) = unpack "x$offset".STATSLOAD_TPL.GLOBALCACHEINFO_TPL, $rdata; 
        $offset += STATSLOAD_LEN + GLOBALCACHEINFO_LEN;
        $data{ $cid }{ C_IP } = inet_ntoa(pack "L", $ip);
        
        #
        # Get the STATSSERVER count
        #
        (
            $data{ $cid }{ C_SCOUNT }
        ) = unpack "x$offset".DEVICECNT_TPL, $rdata; 
        $offset += DEVICECNT_LEN;
        
        #
        # Now unpack the STATSSERVER data
        #
        my @slist = ();
        for (my $i = 0; $i < $data{ $cid }{ C_SCOUNT }; $i++)
        {
            my ($sid) = unpack "x$offset".STATSSERVER_TPL, $rdata; 
            (
                $data{ $cid }{ $sid }{ S_ID },
                $data{ $cid }{ $sid }{ S_PER_CMDS },
                $data{ $cid }{ $sid }{ S_PER_BYTES },
                $data{ $cid }{ $sid }{ S_PER_WRITES },
                $data{ $cid }{ $sid }{ S_PER_WBYTES },
                $data{ $cid }{ $sid }{ S_PER_READS},
                $data{ $cid }{ $sid }{ S_PER_RBYTES },
                $data{ $cid }{ $sid }{ S_QDEPTH }
            ) = unpack "x$offset".STATSSERVER_TPL, $rdata; 
            $offset += STATSSERVER_LEN;
            push @slist, $sid;
        }
        $data{ $cid }{ C_SLIST } = \@slist;

        #
        # Get the STATSVDISK count
        #
        (
            $data{ $cid }{ C_VCOUNT }
        ) = unpack "x$offset".DEVICECNT_TPL, $rdata; 
        $offset += DEVICECNT_LEN;
        
        #
        # Now unpack the STATSVDISK data
        #
        my @vlist = ();
        for (my $i = 0; $i < $data{ $cid }{ C_VCOUNT }; $i++)
        {
            my ($vid) = unpack "x$offset".STATSVDISK_TPL, $rdata; 
            (
                $data{ $cid }{ $vid }{ V_ID },
                $data{ $cid }{ $vid }{ V_RPS },
                $data{ $cid }{ $vid }{ V_AVGSC },
                $data{ $cid }{ $vid }{ V_WRTBYRES },
                $data{ $cid }{ $vid }{ V_WRTBYLEN },
            ) = unpack "x$offset".STATSVDISK_TPL, $rdata; 
            $offset += STATSVDISK_LEN;
            push @vlist, $vid;
        }
        $data{ $cid }{ C_VLIST } = \@vlist;

        #
        # Get the STATSHAB count
        #
        (
            $data{ $cid }{ C_HCOUNT }
        ) = unpack "x$offset".DEVICECNT_TPL, $rdata; 
        $offset += DEVICECNT_LEN;
        
        #
        # Now unpack the STATSHAB data
        #
        my @hlist = ();
        for (my $i = 0; $i < $data{ $cid }{ C_HCOUNT }; $i++)
        {
            my ($hid) = unpack "x$offset".STATSHAB_TPL, $rdata; 
            (
                $data{ $cid }{ $hid }{ H_ID },
                $data{ $cid }{ $hid }{ H_PER_CMDS },
                $data{ $cid }{ $hid }{ H_QDEPTH },
                $data{ $cid }{ $hid }{ H_AVG_REQ },
            ) = unpack "x$offset".STATSHAB_TPL, $rdata; 
            $offset += STATSHAB_LEN;
            push @hlist, $hid;
        }
        $data{ $cid }{ C_HLIST } = \@hlist;
        
        # Don't forget to save the controller id
        push @clist, $cid;
    }
    $data{ G_CLIST } = \@clist;

    return %data;
}

#
# Parse the display parameters
#
my $rawInputString = "@ARGV";

#
# Print a row of column headers
#
my @colHdrs = split /,/, $rawInputString;
print "#";
foreach my $colH (@colHdrs)
{
    $colH =~ s/\s//g;
    
    print uc($colH)."  ";
}
print "\n";

#
# Pad all of the special math characters with spaces (makes life easier
# when splitting things apart).
# 
$rawInputString =~ s/(\*\.)/##/g;
$rawInputString =~ s/([\(\)\+\-\*\/,])/ $1 /g;
$rawInputString =~ s/##/*./g;
#print "-> $rawInputString \n";

#
# Create a "math template" that is filled in with data for each variable as
# each record is read and is then passed to the math parser as the final step.
# 
my @inputVars;
my $mathStringTpl = "";
my $count = 0;
foreach my $inp (split /\s/, "$rawInputString")
{
    if (length($inp) > 1 and $inp !~ /^\d+$/)
    {
        $mathStringTpl .= "[$count]";
        $count++;
        push @inputVars, $inp;
    } 
    else
    {
        $mathStringTpl .= $inp;
    } 
    $mathStringTpl .= " ";
}
#print "--> $mathStringTpl\n";

#
# Parse the actual input variables themselves.
# #
my @parms;
foreach my $parm (@inputVars)
{
    my @bits = split /\./, $parm;
    my $ctl = "*";
    my $mod = "*";
    my $var = "*";
    my $mtype = undef;
    my $vtype = undef;

    foreach my $bit (@bits)
    {
        if ($bit =~ /^C(\d+)$/i)
        {
            $ctl = uc($1);
        }
        if ($bit =~ /^([SVH])(\d+)$/i)
        {
            $mtype = uc($1);
            $mod = $2;
        }
        if ($bit =~ /^(([GCSVH])_[A-Z_0-9]+)$/i)
        {
            $vtype = uc($2);
            $var = uc($1);
        }
    }
#    print "$parm:        $ctl $mtype $mod $vtype $var\n";
    if ($mtype and $mtype ne $vtype)
    {
        print "\nVariable definition mismatch: $parm\n";
        exit 1;
    }
    
    push @parms, [ $ctl, $mod, $var ];
}

#
# If '-k', print out the SID's, VID's, and HID's, then exit
#
if ($opt_k)
{
    my %rec = ParseNextRecord;

    foreach my $ctl (sort(@{$rec{G_CLIST}}))
    {
        print "\nController $ctl:\n";
        print "  SID's: @{$rec{$ctl}{C_SLIST}}\n";
        print "  VID's: @{$rec{$ctl}{C_VLIST}}\n";
        print "  HID's: @{$rec{$ctl}{C_HLIST}}\n";
    }
    print "\n";
    exit 1;
}

#
# Run through the input file record by record and output the requested data
#
use constant CTL => 0;
use constant MOD => 1;
use constant VAR => 2;

while(1)
#for (my $rsr=0; $rsr<5; $rsr++)
{
    my %rec = ParseNextRecord;
    my $mathString = $mathStringTpl;
    my $dataStr = "";

    #
    # Check of the end of the file
    #
    if (!defined($rec{G_TIMESTAMP}))
    {
        exit 0;
    } 

    for (my $idx=0; $idx<@parms; $idx++)
    {
        # Look for a record header global variable
        if ( $parms[$idx][VAR] =~ /^G_/)
        {
            if (!defined($rec{ $parms[$idx][VAR] }))
            {
                $dataStr = "x1x";
            }
            else
            {
                $dataStr = "$rec{ $parms[$idx][VAR] }";
            }
        }
        
        # Look for a controller global variable (non-wildcard: c0.c_feload)
        elsif ( $parms[$idx][VAR] =~ /^C_/ and $parms[$idx][CTL] !~ /\*/)
        {
            if (!defined($rec{ $parms[$idx][CTL] }{ $parms[$idx][VAR] }))
            {
                $dataStr = "x2x";
            }
            else
            {
                $dataStr = "$rec{ $parms[$idx][CTL] }{ $parms[$idx][VAR] }";
            }
        }
        
        # Look for a controller global variable (w/wildcard: c*.c_feload)
        elsif ( $parms[$idx][VAR] =~ /^C_/ and $parms[$idx][CTL] =~ /\*/)
        {
            my $sum = 0;
            foreach my $ctl (sort(@{$rec{G_CLIST}}))
            {
                $sum += $rec{$ctl}{$parms[$idx][VAR]};
            }
            $dataStr = "$sum";
        }
        
        # Finally look for other variables (non-wildcard: c0.s0.s_qdepth)
        elsif (defined($rec{ $parms[$idx][CTL] }{ $parms[$idx][MOD] }
                    { $parms[$idx][VAR] }))
        {
            $dataStr = "$rec{ $parms[$idx][CTL] }{ $parms[$idx][MOD] }{ $parms[$idx][VAR] }";
        }
        
        # Look for other variables (ctl wildcard, other non-wildcard: c*.s0.s_qdepth)
        elsif ( $parms[$idx][CTL] =~ /\*/ and $parms[$idx][MOD] !~ /\*/)
        {
            my $sum = 0;
            foreach my $ctl (sort(@{$rec{G_CLIST}}))
            {
                if (defined($rec{$ctl}{$parms[$idx][MOD]}{$parms[$idx][VAR]}))
                {
                    $sum += $rec{$ctl}{$parms[$idx][MOD]}{$parms[$idx][VAR]};
                }
            }
            $dataStr = "$sum";
        }
        
        # Look for other variables (ctl non-wildcard, other wildcard: c0.s*.s_qdepth)
        elsif ( $parms[$idx][CTL] !~ /\*/ and $parms[$idx][MOD] =~ /\*/)
        {
            my @mlist;
            @mlist = @{$rec{$parms[$idx][CTL]}{C_SLIST}} if $parms[$idx][VAR] =~ /^S_/;
            @mlist = @{$rec{$parms[$idx][CTL]}{C_VLIST}} if $parms[$idx][VAR] =~ /^V_/;
            @mlist = @{$rec{$parms[$idx][CTL]}{C_HLIST}} if $parms[$idx][VAR] =~ /^H_/;
            
            my $sum = 0;
            foreach my $mod (sort(@mlist))
            {
                if (defined($rec{$parms[$idx][CTL]}{$mod}{$parms[$idx][VAR]}))
                {
                    $sum += $rec{$parms[$idx][CTL]}{$mod}{$parms[$idx][VAR]};
                }
            }
            $dataStr = "$sum";
        }
        
        # Look for other variables (ctl wildcard, other wildcard: c*.s*.s_qdepth)
        elsif ( $parms[$idx][CTL] =~ /\*/ and $parms[$idx][MOD] =~ /\*/)
        {
            my $sum = 0;
            foreach my $ctl (sort(@{$rec{G_CLIST}}))
            {
                my @mlist;
                @mlist = @{$rec{$ctl}{C_SLIST}} if $parms[$idx][VAR] =~ /^S_/;
                @mlist = @{$rec{$ctl}{C_VLIST}} if $parms[$idx][VAR] =~ /^V_/;
                @mlist = @{$rec{$ctl}{C_HLIST}} if $parms[$idx][VAR] =~ /^H_/;
                foreach my $mod (sort(@mlist))
                {
                    if (defined($rec{$ctl}{$mod}{$parms[$idx][VAR]}))
                    {
                        $sum += $rec{$ctl}{$mod}{$parms[$idx][VAR]};
                    }
                }
            }
            $dataStr = "$sum";
        }
        
        # Print an error placeholder
        else
        {
            $dataStr = "x3x";
        }

        $mathString =~ s/\[$idx\]/$dataStr/;
    }

    #
    # Call the parser on each input variable group
    #
    foreach my $outv ( split /,/, $mathString ) 
    {
        my $tmp = $parser->startrule($outv);
        $tmp = int($tmp + 0.5);
        print "$tmp ";
    }
    print "\n";
}
close $fh;


