#!/bin/bash -x

RH_IP=$1
if [ "${RH_IP}x" = "x" ]; then
    echo 'First argument must be the IP of the RedHat system to run I/O on.'
    exit 1
fi
if [ $# -gt 1 ]; then
    let i=$#+1
    echo "$0 only takes maximum of 1 arguments, not $i"
    exit 1
fi


W="root@${RH_IP}"
SSH="ssh -x ${W}"
# Check RH_IP is reachable.
${SSH} uptime
#-----------------------------------------------------------------------------
# Stop last some.io process that might be running.
$SSH 'cat /root/some.io.pid'
if [ "$?" == 0 ]; then
    $SSH 'kill -1 `cat /root/some.io.pid`; rm -f /root/some.io.pid'
fi
#-----------------------------------------------------------------------------
# Check if /mnt is mounted.
${SSH} 'mount | grep -c " on /mnt type "'
if [ "$?" != 0 ]; then
    echo "ERROR: /mnt is not mounted from /root/some.io.device.mnt!"
    exit 1
fi
#-----------------------------------------------------------------------------
# Start the I/O test up
# Save PID of last process started in file: /root/some.io.pid
${SSH} -f '/root/some.io /mnt >/dev/null 2>&1 </dev/null & echo $! > /root/some.io.pid'
#-----------------------------------------------------------------------------
echo "DONE with $0"
#-----------------------------------------------------------------------------
exit 0
#-----------------------------------------------------------------------------
