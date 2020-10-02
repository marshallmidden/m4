#!/usr/bin/bash -e

#-----------------------------------------------------------------------------
mkdir -p /home/padmin/log/
#-----------------------------------------------------------------------------
rm -f ~/.targetcli/*
#-----------------------------------------------------------------------------
#- echo mkdir -p /sys/kernel/config/target/iscsi
#-      mkdir -p /sys/kernel/config/target/iscsi
#- echo mkdir -p /sys/kernel/config/target/loopback
#-      mkdir -p /sys/kernel/config/target/loopback
#- echo mkdir -p /sys/kernel/config/target/qla2xxx
#-      mkdir -p /sys/kernel/config/target/qla2xxx
#- echo mkdir -p /sys/kernel/config/target/fc
#-      mkdir -p /sys/kernel/config/target/fc

echo "=============================================================================="
echo /px/bin/targetcli_probe
time /px/bin/targetcli_probe

echo "=============================================================================="
echo /px/bin/startup.py
time /px/bin/startup.py

echo "=============================================================================="
echo /px/bin/targetcli_probe
time /px/bin/targetcli_probe

echo "=============================================================================="
echo /px/bin/px_fs
time /px/bin/px_fs

echo "=============================================================================="
echo /px/bin/cli/blockcfg scan
time /px/bin/cli/blockcfg scan

echo "=============================================================================="
echo /px/bin/cli/listhbas
     /px/bin/cli/listhbas
echo /px/bin/cli/listtargets
     /px/bin/cli/listtargets
echo /px/bin/cli/listmfs
     /px/bin/cli/listmfs

echo "=============================================================================="
su -l parsecweb -c "bundle exec puma" > /var/log/puma.log 2>&1 &
sleep 5
cat /var/log/puma.log
echo "=============================================================================="
/usr/bin/python /var/www/pw/app/assets/scripts/pxmonitor.py >> /px/log/cronlog.log 2>&1 &
echo "=============================================================================="
echo "Done with $0"
