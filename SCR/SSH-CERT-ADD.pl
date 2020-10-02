#!/usr/bin/perl -w
use strict;
use warnings;
#-----------------------------------------------------------------------------
# Allow include from where this file is located, no matter where executed.
BEGIN {
    my $A = __FILE__;
    my $B = `dirname $A`;
    chomp($B);
    push @INC, "$B";
}
use INIT_DR;
#-----------------------------------------------------------------------------
my $AUTHORIZED_KEYS = '/etc/ssh/authorized_keys';
# my $AUTHORIZED_KEYS = '/root/.ssh/authorized_keys';
#-----------------------------------------------------------------------------
if ( ! -e '/root/.ssh/id_rsa.pub')
{
    `cd /root/.ssh ; echo | ssh-keygen`;
}

my $A=`cat /root/.ssh/id_rsa.pub`;
chomp($A);

print STDERR "doing the key add to $INIT_DR::DR_user_at_ip \n";
my $O=`echo $A | ssh "$INIT_DR::DR_user_at_ip" "
cat >> ${AUTHORIZED_KEYS} ; 
sort -u ${AUTHORIZED_KEYS} > ${AUTHORIZED_KEYS}.new ;
mv ${AUTHORIZED_KEYS}.new ${AUTHORIZED_KEYS}"`;
#-----------------------------------------------------------------------------
if ( ($? & 127) != 0 )
{
print STDERR "$O - $?\n";
    die "Error adding ssh keys #1 - " . __FILE__ . ":" . (caller(0))[3] . ":" . __LINE__ . "\n";
}
#-----------------------------------------------------------------------------
exit 0;
#-----------------------------------------------------------------------------
