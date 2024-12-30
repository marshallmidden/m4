#! /bin/sh
#
# Copyright (c) 2001 SuSE GmbH Nuernberg, Germany.  All rights reserved.
#
# /etc/init.d/boot.md
#
### BEGIN INIT INFO
# Provides:          boot.md
# Required-Start:    boot.proc 
# X-SuSE-Should-Start: boot.ibmsis boot.scsidev
# Default-Start:     B
# Default-Stop:
# Description:       start multiple devices
### END INIT INFO

. /etc/rc.status

rc_reset

case "$1" in
    start)
	rc_splash "early stop"
	;;
    stop)
	;;
    status)
	rc_failed 4
	rc_status -v
	;;
    *)
	echo "Usage: $0 {start|stop|status}"
	exit 1
	;;
esac

rc_exit
	
