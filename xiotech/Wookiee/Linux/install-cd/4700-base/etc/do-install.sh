#!/bin/bash -e
#
# Install 4000 filesystems.

. /tmp/sysconf

MNT=/mnt/local
HOSTNAME=ctl-3d
STAT=/tmp/status
LOG=/tmp/log

dev_size() {
	local	path

	path=/sys/block/$1/size
	[[ -f ${path} ]] || { echo 0; return; }
	cat ${path}
}

case "${XIO_HW_TYPE}" in
SMX6DH8-XG2)
	RAID=1
	SWAP=/dev/md2
	GRUB_DEVS="hd0 hd1"
	GRUB_PART=2
	DRIVES="/dev/sda /dev/sdb"
	PARTS=partitions.sda.raid
	;;
*)
	RAID=0
	SWAP=/dev/sda6
	GRUB_DEVS="hd0"
	GRUB_PART=0
	DRIVES="/dev/sda"
	PARTS=partitions.sda
	;;
esac

size=`dev_size sda`
if [[ ${size} -lt 7050000 ]]; then
	fail "sda too small or missing"
fi
if [[ ${size} -lt 9400000 ]]; then
	PARTS=${PARTS}.4G
fi

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
}

do_grub() {
	grub --device-map=${MNT}/boot/grub/device.map --no-floppy --batch <<EOT
root ($1,$2)
setup ($1)
quit
EOT
}

setup_grub() {
	for drive in ${GRUB_DEVS}; do
		do_grub ${drive} ${GRUB_PART} >>${LOG} 2>&1 ||
			fail "GRUB on ${drive} failed"
	done
}

## End of functions

echo -n "Partitioning drives..."
for drive in ${DRIVES}; do
	dn="${drive/#\/dev\//}"
	echo -n " ${dn}"
	sfdisk -L ${drive} < /etc/${PARTS} >>/dev/null 2>&1 || :
	sync; sync
	sfdisk -L ${drive} < /etc/${PARTS} >>${LOG} 2>&1 ||
		fail "sfdisk failed on ${dn}"
done
echo "."

if [ ${RAID} -eq 1 ]; then
	echo 15000 > /proc/sys/dev/raid/speed_limit_max
	echo -n "Creating raids..."
	cat <<"EOF" | while read dev line
/dev/md0 /dev/sda2 /dev/sdb2 missing missing missing missing
/dev/md1 /dev/sda3 /dev/sdb3 missing missing missing missing
/dev/md2 /dev/sda5 /dev/sdb5 missing missing missing missing
/dev/md3 /dev/sda6 /dev/sdb6 missing missing missing missing
EOF
	do
		dn="${dev/#\/dev\//}"
		echo -n " ${dn}"
		mdadm -C ${dev} -l 1 -n 6 -f --run ${line} >>${LOG} 2>&1 ||
			fail "mdadm -C failed on ${dn}"
	done
	echo "."
fi

echo -n "Making filesystems..."
if [ ${RAID} -eq 1 ]; then
	cat <<"EOF"
-L xio-md-opt  /dev/md0
-L xio-md-root /dev/md1
-L xio-md-dump /dev/md3
EOF
else
	cat <<"EOF"
-b 2048 -J size=4 -N 32768 -L xio-root  /dev/sda1
-b 2048 -J size=4 -N 8192 -L xio-opt /dev/sda2
-b 2048 -J size=4 -N 16384 -L xio-xiodata /dev/sda3
-b 4096 -J size=4 -N 1024 -L xio-dump /dev/sda5
EOF
fi | while read line
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
if [ ${RAID} -eq 1 ]; then
	cat <<"EOF"
/dev/md1	/	ext3	rw,noatime,acl,user_xattr
/dev/md0	/opt	ext3	rw,noatime,acl,user_xattr
/dev/md3	/var/log/dump	ext3	rw,noatime,acl,user_xattr
EOF
else
	cat <<"EOF"
/dev/sda1       /       ext3    rw,noatime
/dev/sda2       /opt    ext3    rw,noatime
EOF
fi | restore_fs || fail "restore failed"

if [ ${RAID} -eq 0 ]; then
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
fi

cd /
sync; sync; sync

echo "Install completed." >>${STAT}

# vi:sw=8 ts=8 noexpandtab
