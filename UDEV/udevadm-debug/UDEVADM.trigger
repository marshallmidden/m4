#!/bin/bash -ex
#   trigger, remember to have --dry-run option if you run it on production node.
#   
#   # udevadm trigger
#       runs udev triggers -- usually used after "udevadm control --reload-rules"
#       Same as "pkill -HUP udevd". (kill -1)
#   
#   # udevadm trigger --verbose --dry-run --type=devices --subsystem-match=scsi_host
#   /sys/devices/pci0000:00/0000:00:03.0/0000:15:00.0/host5/scsi_host/host5
#   /sys/devices/pci0000:00/0000:00:03.0/0000:15:00.1/host6/scsi_host/host6
#   /sys/devices/pci0000:00/0000:00:1c.0/0000:01:00.0/host0/scsi_host/host0
#   /sys/devices/pci0000:00/0000:00:1f.2/host1/scsi_host/host1
#   /sys/devices/pci0000:00/0000:00:1f.2/host2/scsi_host/host2
#   /sys/devices/pci0000:00/0000:00:1f.5/host3/scsi_host/host3
#   /sys/devices/pci0000:00/0000:00:1f.5/host4/scsi_host/host4
#
#   # udevadm trigger --verbose --dry-run --type=devices --subsystem-match=scsi_disk
#   /sys/devices/pci0000:00/0000:00:03.0/0000:15:00.0/host5/rport-5:0-0/target5:0:0/5:0:0:0/scsi_disk/5:0:0:0
#   ...
#   /sys/devices/pci0000:00/0000:00:03.0/0000:15:00.1/host6/rport-6:0-8/target6:0:5/6:0:5:9/scsi_disk/6:0:5:9
#   /sys/devices/pci0000:00/0000:00:1c.0/0000:01:00.0/host0/target0:1:0/0:1:0:0/scsi_disk/0:1:0:0
#
#   # udevadm trigger --verbose --dry-run --type=devices --subsystem-match=scsi_tape
#
#   # udevadm trigger --verbose --dry-run --type=devices --subsystem-match=fc_host 
#   /sys/devices/pci0000:00/0000:00:03.0/0000:15:00.0/host5/fc_host/host5
#   /sys/devices/pci0000:00/0000:00:03.0/0000:15:00.1/host6/fc_host/host6

# udevadm trigger /dev/sdc -c change

# udevadm trigger -v --dry-run -t subsystems --action=change -s block /dev/dm-1

# udevadm trigger --verbose --type=subsystems --action=remove --subsystem-match=usb --attr-match="idVendor=abcd"

# udevadm test /block/sdd

# udevadm test /devices/pci0000:00/0000:00:14.0/usb3/3-9/3-9.1/3-9.1:1.0/ttyUSB0/tty/ttyUSB0
# NOTE: test defaults to "change" unless "-c" or "--action=" is specified.

