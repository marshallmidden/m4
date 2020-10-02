#!/bin/bash
#------------------------------------------------------------------------------
FLAGS=0
if [ "${NFS_FLAGS}" = '' ]; then
    echo "NFS_FLAGS variable not set.\n"
    FLAGS=1
fi
if [ "${READ_IP}" = '' ]; then
    echo "READ_IP variable not set (IP of mount volume).\n"
    FLAGS=1
fi
if [ "${READ_VOL}" = '' ]; then
    echo "READ_VOL variable not set (name of mount volume).\n"
    FLAGS=1
fi
if [ "${READ_MNT}" = '' ]; then
    echo "READ_MNT variable not set (directory to mount upon).\n"
    FLAGS=1
fi
#------------------------------------------------------------------------------
if [ "${TIMEFORMAT}" = '' ]; then
    echo "TIMEFORMAT variable not set (time command format options) use default.\n"
fi
#------------------------------------------------------------------------------
if [ "${FLAGS}" = '1' ]; then
    exit 1
fi
#-----------------------------------------------------------------------------
killall SEEFREE || true		# Every 10 seconds print out memory usage information.
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
./W >> LHR.w 2>&1 &
./SEEFREE >> LHR.seefree 2>&1 &
#-----------------------------------------------------------------------------
set -x
time mount -t nfs ${NFS_FLAGS} ${READ_IP}:${READ_VOL} ${READ_MNT}
set +x
# Run the test.
export FILES=`echo ${READ_MNT}/big_tree_1/ ${READ_MNT}/*_files/[0123456789]`
time ./Y
# Unmount previous run
set -x
time sync && time sync
time umount -f ${READ_MNT} 2>/dev/null	|| true
#=============================================================================
# Let the SEEFREE run a few more moments...
sleep 30
killall SEEFREE || true		# Every 10 seconds print out memory usage information.
killall W || true		# Only if swap is used...
killall W.wait.kill || true
#-----------------------------------------------------------------------------
echo DONE with $0
##############################################################################
exit 0
##############################################################################
