#!/mksnt/perl -w
#====================================================================
#
# FILE NAME:    BtHdrDec.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         1/25/03
#
# DESCRIPTION:  Decodes the GetBacktrace.cmd hdr (timestamp, fw
#               header, R/G registers).
#
#====================================================================

use Time::Local;

# GetBacktrace.cmd Header 
use constant BACKTRACE_HDR =>
# ------------ line 1 ------------------
           "S           # backtrace ts year
            C           # backtrace ts month
            C           # backtrace ts date
            
            C           # backtrace ts day
            C           # backtrace ts hours
            C           # backtrace ts min
            C           # backtrace ts sec
            
            L           # backtrace ts systemsec
            
            a4          # fw version
# ------------ line 2 ------------------
            a4          # fw build
            
            a4          # fw builder
            
            S           # fw ts year
            C           # fw ts month
            C           # fw ts date
            
            C           # fw ts day
            C           # fw ts hours
            C           # fw ts min
            C           # fw ts sec
# ------------ line 3 ------------------
            L           # pfp
            L           # sp
            L           # rip
            L           # r3
# ------------ line 4 ------------------
            L           # r4
            L           # r5
            L           # r6
            L           # r7
# ------------ line 5 ------------------
            L           # r8
            L           # r9
            L           # r10
            L           # r11
# ------------ line 6 ------------------
            L           # r12
            L           # r13
            L           # r14
            L           # r15
# ------------ line 7 ------------------
            L           # g0
            L           # g1
            L           # g2
            L           # g3
# ------------ line 8 ------------------
            L           # g4
            L           # g5
            L           # g6
            L           # g7
# ------------ line 9 ------------------
            L           # g8
            L           # g9
            L           # g10
            L           # g11
# ------------ line 10 -----------------
            L           # g12
            L           # g13
            L           # g14
            L           # fp
";


($script = $0) =~ s/^.*\\//;
unless (@ARGV == 1) { die "\nUsage: $script file-to-decode\n\n" }
($infile) = @ARGV;

$outfile = "$infile-out";

#
# Open the output file
#
open OUT, ">$outfile" or die "\nAbort: Can't open $outfile...\n";
print "Output being written to $outfile...\n";

#
# Open the input file
#
open IN, "$infile" or die "\nAbort: Can't open $infile...\n";
binmode IN;

read IN, $buffer, 160 or die "\nAbort: Can't read $infile...\n";

my (
        $bt_yr,         # backtrace ts year
        $bt_mo,         # backtrace ts month
        $bt_dt,         # backtrace ts date

        $bt_dy,         # backtrace ts day
        $bt_hr,         # backtrace ts hours
        $bt_mn,         # backtrace ts min
        $bt_se,         # backtrace ts sec

        $bt_ss,         # backtrace ts systemsec

        $fw_vr,         # fw version
        $fw_bl,         # fw build

        $fw_wh,         # fw builder

        $fw_yr,         # fw ts year
        $fw_mo,         # fw ts month
        $fw_dt,         # fw ts date

        $fw_dy,         # fw ts day
        $fw_hr,         # fw ts hours
        $fw_mn,         # fw ts min
        $fw_se,         # fw ts sec

        $pfp,           # pfp
        $sp,            # sp
        $rip,           # rip
        $r3,            # r3

        $r4,            # r4
        $r5,            # r5
        $r6,            # r6
        $r7,            # r7

        $r8,            # r8
        $r9,            # r9
        $r10,           # r10
        $r11,           # r11

        $r12,           # r12
        $r13,           # r13
        $r14,           # r14
        $r15,           # r15

        $g0,            # g0
        $g1,            # g1
        $g2,            # g2
        $g3,            # g3

        $g4,            # g4
        $g5,            # g5
        $g6,            # g6
        $g7,            # g7

        $g8,            # g8
        $g9,            # g9
        $g10,           # g10
        $g11,           # g11

        $g12,           # g12
        $g13,           # g13
        $g14,           # g14
        $fp,            # fp

) = unpack BACKTRACE_HDR, $buffer;

print  OUT  "\n";
printf OUT  "Time of Backtrace:   %X/%X/%X %X:%02X:%02X (XSSA time)\n", 
        $bt_mo, $bt_dt, $bt_yr, $bt_hr, $bt_mn, $bt_se;
printf OUT  "Time of Last Reboot: %s (XSSA time)\n", 
                LastReboot($bt_ss, $bt_mo, $bt_dt, $bt_yr, $bt_hr, $bt_mn, $bt_se); 
printf OUT  "System Uptime:       %s\n", Uptime($bt_ss);
printf OUT  "CCB FW Version:      %s %s %s %X/%X/%X %X:%02X:%02X (CST/CDT)\n", 
        $fw_vr, $fw_bl, $fw_wh, $fw_mo, $fw_dt, $fw_yr, $fw_hr, $fw_mn, $fw_se;

printf OUT  "\n  Registers:\n".
            "  pfp: %08X      g0: %08X\n".
            "   sp: %08X      g1: %08X\n".
            "  rip: %08X      g2: %08X\n".
            "   r3: %08X      g3: %08X\n".
            "   r4: %08X      g4: %08X\n".
            "   r5: %08X      g5: %08X\n".
            "   r6: %08X      g6: %08X\n".
            "   r7: %08X      g7: %08X\n".
            "   r8: %08X      g8: %08X\n".
            "   r9: %08X      g9: %08X\n".
            "  r10: %08X     g10: %08X\n".
            "  r11: %08X     g11: %08X\n".
            "  r12: %08X     g12: %08X\n".
            "  r13: %08X     g13: %08X\n".
            "  r14: %08X     g14: %08X\n".
            "  r15: %08X      fp: %08X\n",
            $pfp, $g0, $sp,  $g1, $rip, $g2, $r3,  $g3,
            $r4,  $g4, $r5,  $g5, $r6,  $g6, $r7,  $g7,
            $r8,  $g8, $r9,  $g9, $r10, $g10, $r11, $g11,
            $r12, $g12, $r13, $g13, $r14, $g14, $r15, $fp;
print OUT   "\n";

close IN;
close OUT;

sub Uptime
{
    my ($upSec) = @_; 

    my ($days, $hours, $min);

    $days = int($upSec / 86400);
    $upSec -= $days * 86400;

    $hours = int($upSec / 3600);
    $upSec -= $hours * 3600;

    $min = int($upSec / 60);
    $upSec -= $min * 60;

    return sprintf "%u day(s) %u hrs %u min %u sec", $days, $hours, $min, $upSec;
}

sub LastReboot
{
    my ($upSec, $bt_mo, $bt_dt, $bt_yr, $bt_hr, $bt_mn, $bt_se) = @_;
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday);

    $sec =  BCDtoHex($bt_se);
    $min =  BCDtoHex($bt_mn);
    $hour = BCDtoHex($bt_hr);
    $mday = BCDtoHex($bt_dt);
    $mon =  BCDtoHex($bt_mo);
    $year = BCDtoHex($bt_yr);

    $t = timegm($sec,$min,$hour,$mday,$mon-1,$year-1900);

    $t -= $upSec;
    ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday) = gmtime($t);
    
    return sprintf "%u/%u/%u %u:%02u:%02u", 
        $mon+1,$mday,$year+1900,$hour,$min,$sec;
}

sub BCDtoHex 
{
    my ($n) = @_;
    my $rc = 0;

    for(my $i = 0; $i < 4; $i++)
    {
        $rc += ($n & 0xF) * (10 ** $i);
        $n >>= 4;
    }

    return $rc;
}

