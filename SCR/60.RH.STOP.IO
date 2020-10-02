#!/bin/bash -x

IP=$1
if [ "${IP}x" = "x" ]; then
    echo 'First argument must be the IP of the RedHat system to stop running I/O on.'
    exit 1
fi
if [ $# -gt 1 ]; then
    let i=$#+1
    echo "$0 only takes maximum of 1 arguments, not $i"
    exit 1
fi

W="root@${IP}"
SSH="ssh -x ${W}"
# Check IP is reachable.
${SSH} uptime
#-----------------------------------------------------------------------------
# Stop last some.io process that might be running.
$SSH 'cat /root/some.io.pid'
if [ "$?" == 0 ]; then
    $SSH 'kill -1 `cat /root/some.io.pid`; rm -f /root/some.io.pid'
fi
#-----------------------------------------------------------------------------
echo "DONE with $0"
#-----------------------------------------------------------------------------
exit 0
#-----------------------------------------------------------------------------
