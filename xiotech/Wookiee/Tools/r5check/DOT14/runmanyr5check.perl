#!/usr/bin/perl -w

# See file SETUPrunmany.

#
# The idea is to have 59 vdisks created, associated with dot14 in disks /dev/sdc ... sdbi.
#   Start up the ./r5check's with FD's to them.
#   Wait for I/O to stop to dot14 (3 in a row, one second apart).
#   Check all raids not degraded, and operational.
#   Do a write, read, read for each vdisk.
#   Wait for I/O to stop to dot14 (3 in a row, one second apart).
#   Check all raids not degraded, and operational.
#   Loop:
#     fail chosen pdisk.
#     Do a read, read.
#     wait for I/O to finish.
#     Check a raid is degraded, but still operational.
#     unfail chosen pdisk.
#     Do a read, read.
#     wait for I/O to finish.
#     Check all raids not degraded, and operational.
#     Do a write, read, read for each vdisk.
#     Check all raids not degraded, and operational.
#   End of loop.

#-----------------------------------------------------------------------------
# The unix disks in /dev/ to run ./r5check on.
# my @disks = ( 'sdc' );
# my @disks = ( 'sdc', 'sdd', 'sde', 'sdf' );
my @disks = ( 'sdc', 'sdd', 'sde', 'sdf', 'sdg', 'sdh', 'sdi', 'sdj', 'sdk', 'sdl',
              'sdm', 'sdn', 'sdo', 'sdp', 'sdq', 'sdr', 'sds', 'sdt', 'sdu', 'sdv',
              'sdw', 'sdx', 'sdy', 'sdz', 'sdaa', 'sdab', 'sdac', 'sdad', 'sdae', 'sdaf',
              'sdag', 'sdah', 'sdai', 'sdaj', 'sdak', 'sdal', 'sdam', 'sdan', 'sdao', 'sdap',
              'sdaq', 'sdar', 'sdas', 'sdat', 'sdau', 'sdav', 'sdaw', 'sdax', 'sday', 'sdaz',
              'sdba', 'sdbb', 'sdbc', 'sdbd', 'sdbe', 'sdbf', 'sdbg', 'sdbh', 'sdbi' );
 
# The pdisks to fail, one by one.
# my @pdisks = ( 0,1,2,3 );
my @pdisks = ( 0,1,2,3,4,5,6,7,8,9,
              10,11,12,13,14,15,16,17,18,19,
              20,21,22,23,24,25,26,27,28,29,
              30,31,32,33,34,35,36 );

#-----------------------------------------------------------------------------
#-- use strict;
use IO::Handle;

# Script name prefix for this program to run the dot14.bash file (unique disk suffix).
my $cmd = 'run.scripts/exec.run.';
# Script name prefix for bash shell that runs on dot14 (unique disk suffix).
my $bash = 'dot14.bash/run.';

# Size of the vdisks (all assumed the same for simplicity).
my $r5check_file_size           = 141411840;
# Number of contiguous blocks to read at a time.
my $r5check_contiguous_blocks   = 8;
# Offset before reading contiguous blocks -- (0 to contiguous blocks - 1).
my $r5check_offset_blocks       = 0;
# Unique starting value for each ./r5check.
my $r5check_starting_value      =  987654321;
# Add this to above for each disk running a ./r5check.
my $r5check_starting_value_uniq = 1000000000;   # Make sure each r5check program has unique number.
my $r5check_sleep = 1000;                       # Wait tiny time between reads.

# What to send to ./r5check to do a new write to vdisk.
my $do_write = "w\n";
# What to send to ./r5check to do a read (and verify) of vdisk.
my $do_read = "r\n";
# What to send to ./r5check to exit program.
my $do_quit = "q\n";

# Do a write, then three reads.
#--- my @commands = ( $do_write, $do_read, $do_read, $do_read );

my $CURRENTCCBE = "/home/marshall_midden/tmp/HEAD/CCBE";
my $ccbcl_connection = "cd ${CURRENTCCBE} && perl ccbCL.pl 10.64.100.92:3000";
my $ccbcl_perfs_hab = "$ccbcl_connection -e 'perfs hab'";
my $ccbcl_raids = "$ccbcl_connection -e 'raids'";
my $ccbcl_pdisktimeout = "$ccbcl_connection -e pdisktimeout";
my $ccbcl_rescan_loop = "$ccbcl_connection -e 'rescan loop'";
my $ccbcl_devstat_pd = "$ccbcl_connection -e 'devstat pd'";
my $ccbcl_bypass = "$ccbcl_connection -e pdiskbypass ";
my $ccbcl_cstop_count = "$ccbcl_connection -e 'globalcacheinfo'";

my $cl_up = "powerupstate\n";
my $cl_raids = "raids\n";

#-----------------------------------------------------------------------------
select(STDOUT);
$|++;           # Make unbuffered.
select(STDERR);
$|++;           # Make unbuffered (should already be).
#-----------------------------------------------------------------------------
sub make_scripts_and_bash
{
    system('rm -f ' . $cmd . '*');
    system('rm -f ' . $bash . '*');

    foreach my $i (@disks)
    {
        my $fd;
        if (!defined(open($fd, '>' . $cmd . $i)))
        {
            die('Cannot open for writing: ' . $cmd . $i);
        }
        $|++;       # Set to unbuffered.
        print $fd "#!/bin/bash -x\n";
        print $fd "sleep 1\n";
        print $fd "ssh root\@10.64.102.14 'cd work/r5check; rm -f ./run.${i}'\n";
        print $fd "sleep 1\n";
        print $fd "scp dot14.bash/run.${i} root\@10.64.102.14:work/r5check/run.${i}\n";
        print $fd "sleep 1\n";
        print $fd "\n";
        print $fd "ssh root\@10.64.102.14 'cd work/r5check; ./run.${i}' > run.output/output.${i} 2>&1\n";
        close($fd);

        # ...
        if (!defined(open($fd, '>' . $bash . $i)))
        {
            die('Cannot open for writing: ' . $bash . $i);
        }
        $|++;       # Set to unbuffered.
        print $fd "#!/bin/bash -x\n";
        print $fd "rm -f LOCK\n";
        print $fd "\n";
        print $fd "./r5check /dev/${i} ${r5check_file_size} ${r5check_contiguous_blocks} ${r5check_offset_blocks} ${r5check_starting_value} ${r5check_sleep}\n";
        # Make sure each program has a unique starting value.
        $r5check_starting_value += $r5check_starting_value_uniq;
        print $fd "\n";
        print $fd "if [ \$? == 0 ]; then\n";
        print $fd "   echo \"Good exit.\"\n";
        print $fd "   exit 0\n";
        print $fd "fi\n";
        print $fd "\n";
        print $fd "if [ -e LOCK ]; then\n";
        print $fd "   echo \"LOCK exists.\"\n";
        print $fd "   exit 0\n";
        print $fd "fi\n";
        print $fd "touch LOCK\n";
        print $fd "\n";
        print $fd "echo \"LOCK created.\"\n";
        print $fd "\n";
        print $fd "ssh root\@10.64.100.92 ./core3d\n";
        print $fd "sleep 120\n";
        print $fd "ssh marshall_midden\@10.64.99.20 'cd crash/scary/92; make get'\n";

        close($fd);
    }
    system('chmod u+x ' . $cmd . '*');
    system('chmod u+x ' . $bash . '*');
}   # end of make_scripts_and_bash

#-----------------------------------------------------------------------------
sub start_r5checks
{
    foreach my $disk (@disks)
    {
        my $FD = 'FD' . "$disk";
        if (!defined(open($FD, '|' . $cmd . $disk)))
        {
            die('Cannot execute (open for writing to) command: ' . $cmd . $disk);
        }
        $|++;       # Set to unbuffered.
        sleep(2);
    }
}   # end of start_r5checks

#-----------------------------------------------------------------------------
sub write_r5check_command($)
{
    my ($out) = @_;
    my $prt = $out;
    chomp($prt);
printf STDERR "writing to all r5checks '$prt'\n";

    foreach my $disk (@disks)
    {
        my $FD = 'FD' . "$disk";
        print $FD $out;
    }
}   # end of write_r5check_command

#-----------------------------------------------------------------------------
sub terminate_r5checks
{
printf STDERR "terminate r5check programs\n";

    write_r5check_command($do_quit);

    sleep(5);

    foreach my $disk (@disks)
    {
printf STDERR "closing $disk\n";
        my $FD = 'FD' . "$disk";
        close($FD);
    }
printf STDERR "done closing all disks\n";
}   # end of terminate_r5checks

#-----------------------------------------------------------------------------
sub get_cstop_count
{
printf STDERR "get_cstop_count\n";
    my $retry = 24;
    while ($retry-- > 0)
    {
        my $out = `$ccbcl_cstop_count`;
#-- printf STDERR "cstop_count ='$out'\n";
        my @lines = split("\n+", $out);
        if ($#lines < 0)
        {
printf STDERR "get_cstop_count number of lines = 0?  retries left $retry\n";
            sleep(10);
            next;
        }
        my @g = grep(/CA_STOPCNT:/, @lines);
        if ($#g < 0)
        {
printf STDERR "get_cstop_count number of grepped HAB lines is = 0?\n";
            sleep(10);
            next;
        }
        my $g = $g[0];
        ($g =~ /^  *CA_STOPCNT: *(\d+)/);
        my $stopcnt = $1;
        if ($stopcnt != 0)
        {
#-- printf STDERR "stopcnt=$stopcnt\n";
            return 1;
        }
        return 0;
    }
    printf STDERR "Retries have expired in get_cstop_count.\n";
    exit 1;
}   # end of get_cstop_count

#-----------------------------------------------------------------------------
sub get_perfs_zero
{
    my $retry = 5;
    while ($retry-- > 0)
    {
        my $out = `$ccbcl_perfs_hab`;
        my @lines = split("\n+", $out);
        if ($#lines < 0)
        {
printf STDERR "get_perfs_zero number of lines = 0?  retries left $retry\n";
            sleep(10);
            next;
        }
        my @g = grep(/HAB#/ , @lines);
        if ($#g < 0)
        {
printf STDERR "get_perfs_zero number of grepped HAB lines is = 0?\n";
            sleep(10);
            next;
        }
        foreach my $g ( @g )
        {
            ($g =~ /^  *HAB#[0-3]:  *IOPS= *(\d+), MB.S= *([0-9.]+) */);
            my $iops = $1;
            my $mbps = $2;
            if ($iops != 0 || $mbps != 0)
            {
                my $dt = `/bin/date '+%Y-%m-%d@%H:%M:%S'`;
                chomp($dt);
printf STDERR "$dt iops=$iops, mbps=$mbps\n";
                return 1;
            }
        }
        return 0;
    }
    printf STDERR "Retries have expired in get_perfs_zero.\n";
    exit 1;
}   # end of get_perfs_zero

#-----------------------------------------------------------------------------
sub wait_perfs_zero
{
    my $four_times = 0;

printf STDERR "wait perfs zero\n";
    while (1)
    {
        if (get_perfs_zero() == 0)
        {
            $four_times += 1;
            if ($four_times == 4)
            {
#--printf STDERR "wait_perfs zero four times\n";
                return;
            }
#--printf STDERR "wait_perfs zero $four_times\n";
        }
        else
        {
#--printf STDERR "wait_perfs not zero\n";
            $four_times = 0;
        }
        sleep 5;
    }
}   # end of wait_perfs_zero

#-----------------------------------------------------------------------------
sub wait_no_io
{
    my $retrycnt = 0;

printf STDERR "wait no io\n";
    while ($retrycnt++ < 32)
    {
        if (get_cstop_count() != 0)
        {
            sleep(4);
            next;
        }
        wait_perfs_zero();
        if (get_cstop_count() != 0)
        {
            sleep(4);
            next;
        }
        return;
    }
    printf STDERR "Retries have expired in wait_no_io.\n";
}   # end of wait_no_io

#-----------------------------------------------------------------------------
sub rescan_loop()
{
printf STDERR "rescan_loop\n";
    my $retry = 5;
    while ($retry-- > 0)
    {
        my $out = `$ccbcl_rescan_loop`;
        my @lines = split("\n+", $out);
        if ($#lines < 0)
        {
printf STDERR "rescan_loop number of lines = 0?  retries left $retry\n";
            sleep(10);
            next;
        }
        foreach my $g ( @lines )
        {
            if ($g =~ /Rescan loop Successful/)
            {
                return;
            }
        }
printf STDERR "rescan_loop retries left $retry\n";
        sleep 10;
    }
    printf STDERR "Retries have expired in rescan_loop.\n";
    exit 1;
}   # end of rescan_loop

#-----------------------------------------------------------------------------
sub get_ses_slot($)
{
    my ($pdisk) = @_;

    my $retry = 5;
    while ($retry-- > 0)
    {
        my $out = `$ccbcl_devstat_pd`;
        my @lines = split("\n+", $out);
        if ($#lines < 0)
        {
printf STDERR "get_ses_slot number of lines = 0?  retries left $retry\n";
            sleep(10);
            next;
        }
        foreach my $g ( @lines )
        {
            ($g =~ /^.  *(\d+)  *.*   PD-(.)(\d\d)    0x. - [DU][AN][TS]A/);
            if (!defined($1) || !defined($2) || !defined($3))
            {
#-- printf STDERR "output=did not match line '$g'";
                next
            }
            my $p = $1;
            my $ses = $2;
            my $slot = $3;
            if ($p != $pdisk)
            {
                next;
            }

            $ses = ord($ses) - ord('A');
            $slot = $slot + 0;
            return(($ses, $slot));
        }
printf STDERR "get_ses_slot retries left $retry\n";
        sleep 10;
    }
    printf STDERR "Retries have expired in get_ses_slot.\n";
    exit 1;
}   # end of get_ses_slot

#-----------------------------------------------------------------------------
sub pdisk_bypass($$$)
{
    my ($ses, $slot, $v) = @_;

printf STDERR "pdisk_bypass $ses $slot $v\n";
    my $retry = 5;
    while ($retry-- > 0)
    {
        my $out = `$ccbcl_bypass $ses $slot $v`;
        my @lines = split("\n+", $out);
        if ($#lines < 0)
        {
printf STDERR "pdisk_bypass number of lines = 0?  retries left $retry\n";
            sleep(10);
            next;
        }
        foreach my $g ( @lines )
        {
            ($g =~ /^Physical disk \((\d+), *(\d+)\) bypass \($v\)/);
            if (!defined($1) || !defined($2))
            {
#-- printf STDERR "output=did not match line '$g'";
                next
            }
            my $s = $1;
            my $t = $2;
            if ($s != $ses || $t != $slot)
            {
printf STDERR "pdisk_bypass did not do right disk ($ses, $slot) != ($s, $t)\n";
                exit 1;
            }
            return;
        }
printf STDERR "pdisk_bypass retries left $retry\n";
        sleep 10;
    }
    printf STDERR "Retries have expired in pdisk_bypass.\n";
    exit 1;
}   # end of pdisk_bypass

#-----------------------------------------------------------------------------
#---sub pdisk_fail($)
#---{
#---    my ($pdisk) = @_;
#---
#---    my ($ses, $slot) = get_ses_slot($pdisk);
#---
#---    my $retry = 5;
#---    while ($retry-- > 0)
#---    {
#---        my $out = `$ccbcl_pdisktimeout $pdisk 1`;
#---printf STDERR "pdisk_fail out=$out\n";
#---        my @lines = split("\n+", $out);
#---        if ($#lines < 0)
#---        {
#---printf STDERR "pdisk_fail number of lines = 0?  retries left $retry\n";
#---            sleep(10);
#---            next;
#---        }
#---        foreach my $g ( @lines )
#---        {
#---            ($g =~ /^Physical disk \((\d+)\) emulate qlogic timeout \(0x1\)/);
#---            if (!defined($1))
#---            {
#---#-- printf STDERR "output=did not match line '$g'";
#---                next
#---            }
#---            my $p = $1;
#---            if ($p != $pdisk)
#---            {
#---printf STDERR "pdisk_fail did not do the right disk ($p != $pdisk)\n";
#---                exit 1;
#---            }
#---            return;
#---        }
#---printf STDERR "pdisk_fail retries left $retry\n";
#---        sleep 10;
#---    }
#---    exit 1;
#---}   # end of pdisk_fail

#-----------------------------------------------------------------------------
#---sub pdisk_restore($)
#---{
#---    my ($pdisk) = @_;
#---
#---    my $retry = 5;
#---    while ($retry-- > 0)
#---    {
#---        my $out = `$ccbcl_pdisktimeout $pdisk 0`;
#---        my @lines = split("\n+", $out);
#---        if ($#lines < 0)
#---        {
#---printf STDERR "pdisk_restore number of lines = 0?  retries left $retry\n";
#---            sleep(10);
#---            next;
#---        }
#---        foreach my $g ( @lines )
#---        {
#---            ($g =~ /^Physical disk \((\d+)\) emulate qlogic timeout \(0x0\)/);
#---            if (!defined($1))
#---            {
#---                next
#---            }
#---            my $p = $1;
#---            if ($p != $pdisk)
#---            {
#---printf STDERR "pdisk_restore did not do the right disk ($p != $pdisk)\n";
#---                exit 1;
#---            }
#---            return;
#---        }
#---printf STDERR "pdisk_restore retries left $retry\n";
#---        sleep 10;
#---    }
#---    exit 1;
#---}   # end of pdisk_restore

#-----------------------------------------------------------------------------
sub raids_not_degraded_and_operational
{
    my $retry = 5;
    my $ret = 0;

    while ($retry-- > 0)
    {
        my $out = `$ccbcl_raids`;
        my @lines = split("\n+", $out);
        if ($#lines < 0)
        {
printf STDERR "raids_not_degraded_and_operational number of lines = 0?  retries left $retry\n";
            sleep(10);
            next;
        }
        foreach my $g ( @lines )
        {
            #           RID   TYPE   SPS      Devstat            astat          vid capacty %init FAIL misc notmirror
            ($g =~ /^ *(\d+)  *\d+  *\d+  *(0x[0-9A-Fa-f]+)  *(0x[0-9A-Fa-f]+)  *(\d+)  *\d+  *\d+  *\d+ *\d+  *\d+/);
            if (!defined($1))
            {
                next
            }
            my $rid = $1;
            my $devstat = $2;
            my $astat = $3;
            my $vid = $4;
            if ($devstat eq "0x00" || $devstat eq "0x01" || $devstat eq "0x02" ||
                $devstat eq "0x04" || $devstat eq "0x05")
            {
printf STDERR "RAID IN BAD STATE rid=$rid, devstat=$devstat, astat=$astat, vid=$vid\n";
                $ret = 1;
            }
            elsif ($devstat eq "0x11" || $devstat ne "0x10" || $astat ne "0x00")
            {
printf STDERR "RAID degraded -- not normally operating -- rid=$rid, devstat=$devstat, astat=$astat, vid=$vid\n";
                $ret = 1;
            }
        }
        return $ret;
    }
    printf STDERR "Retries have expired in raids_not_degraded_and_operational.\n";
    exit 1;
}   # end of raids_not_degraded_and_operational

#=============================================================================

# Main program follows:
my $dt = `/bin/date '+%Y-%m-%d@%H:%M:%S'`;
chomp($dt);
printf STDERR "$dt starting program runmanyr5check.perl\n";

make_scripts_and_bash();

# Start up the ./r5check's with FD's to them.
start_r5checks();

# Wait for I/O to stop to dot14 (3 in a row, one second apart).
wait_no_io();

# Check all raids not degraded, and operational.
if (raids_not_degraded_and_operational() != 0)
{
    printf STDERR "RAID degraded should not be when starting.\n";
    exit 1;
}

# Do a write, read, read for each vdisk.
write_r5check_command($do_write);
sleep(5);
wait_no_io();
write_r5check_command($do_read);
sleep(5);
wait_no_io();

# Check all raids not degraded, and operational.
if (raids_not_degraded_and_operational() != 0)
{
    printf STDERR "RAID degraded should not be when script hasn't failed anything yet.\n";
    exit 1;
}

my $test_number = 1;
$dt = `/bin/date '+%Y-%m-%d@%H:%M:%S'`;
chomp($dt);
printf STDERR "$dt DONE with test $test_number.\n";

#   Loop:
foreach my $pdisk ( @pdisks )
{
    $test_number++;
printf STDERR "Starting to fail PDISK $pdisk, test $test_number\n";

#   fail chosen pdisk.
#--    pdisk_fail($pdisk);
    my ($ses, $slot) = get_ses_slot($pdisk);
    pdisk_bypass($ses, $slot, "0xc");
    sleep(10);
    rescan_loop();
    sleep(60);
    rescan_loop();
    sleep(60);

#   Do a read, read.
    write_r5check_command($do_read);
    sleep(5);
    wait_no_io();
    write_r5check_command($do_read);
    sleep(5);
    wait_no_io();

#     Check a raid is degraded, but still operational.
    if (raids_not_degraded_and_operational() != 1)
    {
        printf STDERR "A raid did not go degraded when $pdisk ($ses, $slot) was bypassed.\n";
#         exit 1;
    }

#   Do a write, read.
    write_r5check_command($do_write);
    sleep(5);
    wait_no_io();
    write_r5check_command($do_read);
    sleep(5);
    wait_no_io();

#   unfail chosen pdisk.
#--    pdisk_restore($pdisk);
    pdisk_bypass($ses, $slot, "0x0");
    sleep(10);
    rescan_loop();
    sleep(60);
    rescan_loop();
    sleep(60);

#   Do a read, read.
    write_r5check_command($do_read);
    sleep(5);
    wait_no_io();
    write_r5check_command($do_read);
    sleep(5);
    wait_no_io();

#   Check all raids not degraded, and operational.
    if (raids_not_degraded_and_operational() != 0)
    {
        printf STDERR "A raid is still degraded when $pdisk ($ses, $slot) was returned.\n";
#??        exit 1;
    }

#   Do a write, read, read for each vdisk.
    write_r5check_command($do_write);
    sleep(5);
    wait_no_io();
    write_r5check_command($do_read);
    sleep(5);
    wait_no_io();

#   Check all raids not degraded, and operational.
    if (raids_not_degraded_and_operational() != 0)
    {
        printf STDERR "A raid went degraded when $pdisk ($ses, $slot) was returned.\n";
        exit 1;
    }
    $dt = `/bin/date '+%Y-%m-%d@%H:%M:%S'`;
    chomp($dt);
    printf STDERR "$dt DONE with test $test_number.\n";
# End of loop.
}


$dt = `/bin/date '+%Y-%m-%d@%H:%M:%S'`;
chomp($dt);
printf STDERR "$dt DONE with all tests.\n";

terminate_r5checks();
exit 0;

##############################################################################
# End of File runmanyr5check.perl
#
## Modelines:
## vi:sw=4 ts=4 expandtab
