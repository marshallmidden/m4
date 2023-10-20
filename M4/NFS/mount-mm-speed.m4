#!/bin/bash
# We want to run the test Y twice -- in case NFS server caches.
#------------------------------------------------------------------------------
#-- export READ_IP='172.22.14.103'
#-- export READ_VOL='/vol/m4_1G_nfs_v1'

export READ_IP='192.168.15.186'
export READ_VOL='/vol/m4_bigdst1'
#-----------------------------------------------------------------------------
export READ_MNT='/mnt/m4_bigdst1'
export FILES=`echo ${READ_MNT}/*/*/*`
#-----------------------------------------------------------------------------
NFS_FLAGS='-o ro,nosuid,nodev,noexec,noatime,nodiratime,sync,rsize=1048576,wsize=1048576,namlen=255,acregmin=0,acregmax=0,acdirmin=0,acdirmax=0,soft,nocto,noac,nolock,noacl,nordirplus,proto=tcp,timeo=600,retrans=2,sec=sys,mountproto=tcp,lookupcache=none,local_lock=none'
# Take out nouser,cto,nofsc,noiversion,nomand,norelatime,nostrictatime,sharecache

#-----------------------------------------------------------------------------
TIMEFORMAT='real %R sec  user %U sec  system %S sec - %%%P CPU usage'
#------------------------------------------------------------------------------
killall seefree || true		# Every 10 seconds print out memory usage information.
killall W || true		# Only if swap is used...
killall W.wait.kill || true
# Nothing in slabs nor buffer/cache, etc.
sysctl vm.drop_caches=3

echo 'Initialize to supposed Parsec values:'
sysctl vm.min_unmapped_ratio=1
MemSize=`grep MemTotal: /proc/meminfo | awk '{print $2}'`
let MemSize=${MemSize}/50                       # 2% of whatever is on the machine.
sysctl vm.min_free_kbytes=${MemSize}
sysctl vm.vfs_cache_pressure=10000
#-----------------------------------------------------------------------------
# Setup:
mkdir -p ${READ_MNT}
# Clean up left-over mounts from previous runs.
umount -f ${READ_MNT} 2>/dev/null	|| true
#-----------------------------------------------------------------------------
# Ready for test:
./W >> M4.w 2>&1 &
/home/m4/bin/seefree >> M4.seefree 2>&1 &
#-----------------------------------------------------------------------------
set -x
time mount -t nfs ${NFS_FLAGS} ${READ_IP}:${READ_VOL} ${READ_MNT}
set +x
# Run the test.
time ./Y
# Unmount previous run
set -x
time umount -f ${READ_MNT} 2>/dev/null	|| true

#=============================================================================
# Second try.
#-----------------------------------------------------------------------------
time mount -t nfs ${NFS_FLAGS} ${READ_IP}:${READ_VOL} ${READ_MNT}
# Run the test.
set +x
time ./Y
# Unmount previous run
set -x
time umount -f ${READ_MNT} 2>/dev/null	|| true
set +x
#-----------------------------------------------------------------------------
# Let the seefree run a few more moments...
sleep 30
killall seefree || true		# Every 10 seconds print out memory usage information.
killall W || true		# Only if swap is used...
killall W.wait.kill || true
#-----------------------------------------------------------------------------
echo DONE with $0
##############################################################################
exit 0
##############################################################################
