#!/bin/bash -ex

#-- udevadm trigger /dev/sdc -c change

#-- udevadm trigger -v --dry-run -t subsystems --action=change -s block /dev/dm-1
