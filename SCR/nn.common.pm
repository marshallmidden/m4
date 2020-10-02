# Some common subroutines to make all scripts look similar.
#-----------------------------------------------------------------------------
use warnings;
use strict;

use Exporter qw(import);

our @EXPORT_OK = qw(execute cmd_error cmd_warning log_separator log_command log_status_into_file);
#-----------------------------------------------------------------------------
# Execute a command and set global variable output.
sub execute($)
{
    my $cmd = $_[0];

    $main::command = "$main::SSH '$cmd'";
    print STDERR "Executing: $main::command\n";
    $main::output = `${main::command}`;
    $main::status = ($? >> 8) & 255;
    chomp($main::output); chomp($main::output);
    $main::output =~ s/\r//g;
    print STDERR "   Got($main::status): '$main::output'\n";
}   # End of execute

#-----------------------------------------------------------------------------
# Print message, command, it's output, and usage of command, then exit.

sub cmd_error($$$)
{
    my $msg = $_[0];
    my $file = $_[1];
    my $line = $_[2];

    print STDERR "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n";
    print STDERR "ERROR($main::status): ${file}:${line}\n" .
	  "${msg}\n" .
          "command: ${main::command}\n" .
          "${main::output}\n";
    print STDERR "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
    exit 1;
}   # End of cmd_error

#-----------------------------------------------------------------------------
# Print message, command, it's output, and usage of command, and continue.

sub cmd_warning($$$)
{
    my $msg = $_[0];
    my $file = $_[1];
    my $line = $_[2];

    print STDERR "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n";
    print STDERR "WARNING: ${file}:${line}\n" .
	  "${msg}\n" .
          "command: ${main::command}\n" .
          "${main::output}\n";
    print STDERR "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
}   # End of cmd_warning

##############################################################################
##############################################################################
# For log printing below.
##############################################################################
##############################################################################
sub log_separator()
{
    print LOGFILE "==============================================================================\n";
}   # End of log_separator

#-----------------------------------------------------------------------------
my $the_command;                # Command to run.

sub add_command($)
{
    my ($cmd) = @_;

    if (defined($the_command))
    {
        $the_command .= ';';
    }
    else
    {
        $the_command = '';
    }
    $the_command .= "echo \"##### $cmd #####\";";
    $the_command .= "$cmd 2>&1";
}   # End of add_command

#-----------------------------------------------------------------------------
sub run_command($$)
{
    my ($prefix, $where) = @_;
    my $output = `$where '$the_command'`;

    my $str = '##############################################################################';
    substr($str, 5, length($prefix) + 3) = " $prefix: ";
    printf LOGFILE "%s\n%s", $str, $output;

    undef($the_command);
}   # End of run_command

#-----------------------------------------------------------------------------
sub log_status_into_file($$$$$$$$)
{
    if (defined($main::log_things) && $main::log_things ne '')
    {
        return;
    }
    my ($prog, $cmd, $rh, $hq, $nimble, $nimble_login, $source, $target) = @_;
    my $output;

    my $prenum = $cmd;
    $prenum =~ s/[^0-9].*$//;

    open(LOGFILE, '>', "${main::PROG}.log.$prenum") or die "Could not open log file $main::PROG.log.$prenum. $!";

    print STDERR "Printing detailed status to $main::PROG.log.$prenum\n";

    my $date = `/bin/date '+%Y-%m-%d_%H-%M-%S'`;
    print LOGFILE "$date";
    print LOGFILE "After command: $cmd\n";

    log_separator();

    add_command('iscsiadm -m discovery');
    add_command('iscsiadm -m node');
    add_command('iscsiadm -m session');
    add_command('df');
    add_command('mount');
    add_command('lsblk -o NAME,MAJ:MIN,TYPE,FSTYPE,MOUNTPOINT,UUID,MODEL,SIZE');
    add_command('blkid -o list');
    add_command('dmsetup ls --tree -o device');
    add_command('dmsetup info -C');
    add_command('vgs');
    add_command('pvs');
    add_command('lvs');
    add_command('lsmod | grep multipath');
    add_command('ls -l /dev/{sd,dm,vd,md,nvme}*');
    add_command('multipath -ll');
    add_command('lsscsi');
    add_command('hsmadm show');
    add_command('lsmod | grep hsm');
    add_command('tail -n 20 /var/log/messages');
    run_command('rh', "ssh -x root\@$rh");

    log_separator();

    add_command('iscsiadm -m discovery');
    add_command('iscsiadm -m node');
    add_command('iscsiadm -m session');
    add_command('df');
    add_command('mount');
    add_command('lsblk -o NAME,MAJ:MIN,TYPE,FSTYPE,MOUNTPOINT,UUID,MODEL,SIZE');
    add_command('blkid -o list');
    add_command('dmsetup ls --tree -o device');
    add_command('dmsetup info -C');
    add_command('vgs');
    add_command('pvs');
    add_command('lvs');
    add_command('lsmod | grep multipath');
    add_command('ls -l /dev/{sd,dm,vd,md,nvme}*');
    add_command('multipath -ll');
    add_command('/home/padmin/hsm listmigrations');
    add_command('/home/padmin/hsm listschedules');
    add_command('/home/padmin/listmfs');
    add_command('/home/padmin/listhbas');
    add_command('/home/padmin/listtargets');
    add_command('tail -n 20 /var/log/kern.log');
    run_command('HyperQ', "ssh -x root\@$hq");

    log_separator();

    add_command('initiatorgrp --list');
    add_command('vol --list');
    add_command("vol --info $source");
    add_command("vol --info $target");
    run_command('iSCSI Nimble', "ssh -x $nimble_login\@$nimble");

    close(LOGFILE);
}   # End of log_status_into_file

#-----------------------------------------------------------------------------
1;
#-----------------------------------------------------------------------------
