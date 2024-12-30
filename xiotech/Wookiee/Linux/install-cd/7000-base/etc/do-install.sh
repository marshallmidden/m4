#!/bin/bash -e
#
# Install 7000 filesystems.

MNT=/mnt/local
HOSTNAME=ctl-7000
STAT=/tmp/status
LOG=/tmp/log
SWAP=/dev/sda6

. /etc/restore_functions

load_rpms() {
	cp /cdrom/*.rpm ${MNT} || fail "cp of rpms failed"
	chroot ${MNT} rpm -i --force --nodeps /*.rpm ||
		fail "rpm returned $?"
	rm -f ${MNT}/*.rpm
}

setup_kernel() {
	test -f ${MNT}/opt/xiotech/release/pl*/kernel.tgz ||
		fail "kernel not found"
	gunzip -c ${MNT}/opt/xiotech/release/pl*/kernel.tgz |
		tar xf - -C ${MNT} || fail "installing kernel"
	cp ${MNT}/opt/xiotech/release/pl*/kernelver ${MNT}/opt/xiotech ||
		fail "setting /opt/xiotech/kernelver"
	cp -d ${MNT}/boot/vmlinuz-3d ${MNT}/boot/vmlinuz-3d.prev ||
		fail "setting previous kernel link"
	. /tmp/sysconf || fail "Sysconf failure"
	cp ${MNT}/boot/grub/menu.lst-${XIO_HW_TYPE} ${MNT}/boot/grub/menu.lst ||
		fail "setting grub config for kernel"
}

setup_platform() {
	cp /tmp/sysconf ${MNT}/etc/sysconfig/xio3d || fail "Config failure"
	chroot ${MNT} chkconfig xio3d on || fail "activating xio3d failed"
	rm -f ${MNT}/etc/sysconfig/network/ifcfg-eth*
	mkdir -p -m 755 ${MNT}/mnt/floppy

#	local -a plpath
#	local plver
#	local ethpci

#	plpath=(`echo ${MNT}/opt/xiotech/release/pl*`)
#	plver=${plpath##*-0}
#	test "${plver%??-????}" = "5" || return
#	ethpci=0000:03.02.0
#	cat <<EOF > ${MNT}/etc/sysconfig/network/ifcfg-eth0
#BOOTPROTO='dhcp'
#MTU='' 
#REMOTE_IPADDR=''
#STARTMODE='onboot'
#UNIQUE='x0Ln.lXjqHkaYUc8'
#_nm_name='${ethpci}'
#EOF
}

do_grub() {
	grub --device-map=${MNT}/boot/grub/device.map --no-floppy --batch <<EOT
root ($1,$2)
setup ($1)
quit
EOT
}

setup_grub() {
	do_grub hd0 0 >>${LOG} 2>&1 || fail "GRUB on hd0 failed"
}

dev_size() {
	local	path

	path=/sys/block/$1/size
	[[ -f ${path} ]] || { echo 0; return; }
	cat ${path}
}

## End of functions

echo -n "Partitioning drives..."
for drive in /dev/sda; do
	dn="${drive/#\/dev\//}"
	echo -n " ${dn}"
	size=`dev_size ${dn}`
	if [[ ${size} -lt 7050000 ]]; then
		fail "${dn} too small or missing"
	fi
	if [[ ${size} -lt 9400000 ]]; then
		ext="${dn}.4G"
	else
		ext="${dn}"
	fi
	sfdisk -L ${drive} < /etc/partitions.${ext} >>/dev/null 2>&1 || :
	sync; sync
	sfdisk -L ${drive} < /etc/partitions.${ext} >>${LOG} 2>&1 ||
		fail "sfdisk failed on ${dn}"
done
echo "."

echo -n "Making filesystems..."

cat <<"EOF" | while read line
-b 2048 -J size=4 -N 32768 -L xio-root  /dev/sda1
-b 2048 -J size=4 -N 8192 -L xio-opt /dev/sda2
-b 2048 -J size=4 -N 16384 -L xio-xiodata /dev/sda3
-b 4096 -J size=4 -N 1024 -L xio-dump /dev/sda5
EOF
do
	dn="${line/#*\/dev\//}"
	echo -n " ${dn}"
	mkfs.ext3 -j ${line} >>${LOG} 2>&1 ||
		fail "mkfs.ext3 ${dn} failed"
done
echo "."

echo -n "Making swap on ${SWAP/#\/dev\//}"
mkswap ${SWAP} >>${LOG} 2>&1 || fail "mkswap failed on ${SWAP}"
echo "mkswap succeeded on ${SWAP}" >>${STAT}
echo -n ", activating swap"
swapon ${SWAP} || fail "swapon ${SWAP} failed"
echo "."

cd ${MNT}
restore_fs <<"EOF" || fail "restore failed"
/dev/sda1	/	ext3	rw,noatime
/dev/sda2	/opt	ext3	rw,noatime
EOF

cat > ${MNT}/etc/fstab <<EOF || fail "Setting fstab failed"
/dev/sda1            /                    ext3       noatime 1 1
/dev/sda2            /opt                 ext3       noatime 1 2
/dev/sda3            /opt/xiotech/xiodata ext3       noatime 1 2
/dev/sda5            /var/log/dump        ext3       noatime 1 2
/dev/sda6            swap                 swap       pri=42            0 0
tmpfs                /tmp                 tmpfs      size=64m          0 0
tmpfs                /var/tmp             tmpfs      size=64m          0 0
devpts               /dev/pts             devpts     mode=0620,gid=5   0 0
proc                 /proc                proc       defaults          0 0
sysfs                /sys                 sysfs      noauto            0 0
/dev/cdrom           /mnt/cdrom           iso9660    noauto,ro,nosuid,nodev,exec,iocharset=utf8 0 0
EOF

cp ${MNT}/etc/fstab ${MNT}/etc/fstab.std

cd /
sync; sync; sync

echo "Install completed." >>${STAT}

# vi:sw=8 ts=8 noexpandtab
