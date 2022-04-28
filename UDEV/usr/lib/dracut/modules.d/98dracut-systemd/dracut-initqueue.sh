#!/bin/sh

export DRACUT_SYSTEMD=1
if [ -f /dracut-state.sh ]; then
    . /dracut-state.sh 2>/dev/null
fi
type getarg >/dev/null 2>&1 || . /lib/dracut-lib.sh

source_conf /etc/conf.d

make_trace_mem "hook initqueue" '1:shortmem' '2+:mem' '3+:slab'
getarg 'rd.break=initqueue' -d 'rdbreak=initqueue' && emergency_shell -n initqueue "Break before initqueue"

RDRETRY=$(getarg rd.retry -d 'rd_retry=')
RDRETRY=${RDRETRY:-180}
RDRETRY=$(($RDRETRY*2))
export RDRETRY

main_loop=0
export main_loop

/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 1" > /dev/kmsg

#-----------------------------------------------------------------------------
check_finished_IQ() {
    local f
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh check_finished_IQ dir $hookdir/initqueue/finished/*.sh > /dev/kmsg
    for f in $hookdir/initqueue/finished/*.sh; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh check_finished_IQ f=$f > /dev/kmsg
        if [ "$f" = "$hookdir/initqueue/finished/*.sh" ]; then
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 1check_finished_IQ none other check return 0 > /dev/kmsg
	    return 0
	fi
	if [ ! -e "$f" ]; then
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 2check_finished_IQ file does not exist return 1 > /dev/kmsg
	    return 1
	fi
	. "$f"
	r=$?
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 3check_finished_IQ return return=$r > /dev/kmsg
	if [ "$r" != 0 ]; then
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 4check_finished_IQ file returned failure return 1 > /dev/kmsg
	    return 1
	fi
    done
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh check_finished_IQ return 0" > /dev/kmsg
    return 0
}
#-----------------------------------------------------------------------------
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 2" > /dev/kmsg
for g in $hookdir/initqueue/finished/*.sh; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh g $g > /dev/kmsg
    [ "$g" != "$hookdir/initqueue/finished/*.sh" ] && [ -e "$g" ] && cat "$g" > /dev/kmsg
done
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh sh $hookdir/initqueue/*.sh > /dev/kmsg
for g in $hookdir/initqueue/*.sh; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh g $g > /dev/kmsg
    [ "$g" != "$hookdir/initqueue/*.sh" ] && [ -e "$g" ] && cat "$g" > /dev/kmsg
done
for g in $hookdir/initqueue/settled/*.sh; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh g $g > /dev/kmsg
    [ "$g" != "$hookdir/initqueue/settled/*.sh" ] && [ -e "$g" ] && cat "$g" > /dev/kmsg
done
for g in /run/systemd/ask-password/ask.*; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh g $g > /dev/kmsg
    [ "$g" != "/run/systemd/ask-password/ask.*" ] && [ -e "$g" ] && cat "$g" > /dev/kmsg
done
for g in $hookdir/initqueue/timeout/*.sh; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh g $g > /dev/kmsg
    [ "$g" != "$hookdir/initqueue/timeout/*.sh" ] && [ -e "$g" ] && cat "$g" > /dev/kmsg
done
#-----------------------------------------------------------------------------

while :; do

/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 3" > /dev/kmsg
    check_finished_IQ && break

/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 4" > /dev/kmsg
    udevadm settle --exit-if-exists=$hookdir/initqueue/work

/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 5" > /dev/kmsg
    check_finished_IQ && break

/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 6" > /dev/kmsg
    if [ -f $hookdir/initqueue/work ]; then
        rm -f -- "$hookdir/initqueue/work"
    fi

/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 7job $hookdir/initqueue/*.sh > /dev/kmsg
    for job in $hookdir/initqueue/*.sh; do
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 8 job=$job" > /dev/kmsg
        [ -e "$job" ] || break
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 9" > /dev/kmsg
        job=$job . $job
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 10" > /dev/kmsg
        check_finished_IQ && break 2
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 11" > /dev/kmsg
    done

/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 12" > /dev/kmsg
    udevadm settle --timeout=0 >/dev/null 2>&1 || continue

/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 13dir $hookdir/initqueue/settled/*.sh > /dev/kmsg
    for job in $hookdir/initqueue/settled/*.sh; do
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 14 job=$job" > /dev/kmsg
        [ -e "$job" ] || break
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 15" > /dev/kmsg
        job=$job . $job
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 16" > /dev/kmsg
        check_finished_IQ && break 2
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 17" > /dev/kmsg
    done

/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 18" > /dev/kmsg
    udevadm settle --timeout=0 >/dev/null 2>&1 || continue

/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 19" > /dev/kmsg
    # no more udev jobs and queues empty.
    sleep 0.5

/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 20dir /run/systemd/ask-password/ask.* > /dev/kmsg
    for i in /run/systemd/ask-password/ask.*; do
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 21 i=$i" > /dev/kmsg
        [ -e "$i" ] && continue 2
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 22" > /dev/kmsg
    done

/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 23 main_loop=$main_loop RDRETRY=$RDRETRY" > /dev/kmsg
    if [ $main_loop -gt $((2*$RDRETRY/3)) ]; then
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 24" > /dev/kmsg
        warn "dracut-initqueue timeout - starting timeout scripts"
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 25dir $hookdir/initqueue/timeout/*.sh > /dev/kmsg
        for job in $hookdir/initqueue/timeout/*.sh; do
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 26 job=$job" > /dev/kmsg
            [ -e "$job" ] || break
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 27" > /dev/kmsg
            job=$job . $job
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 28" > /dev/kmsg
            udevadm settle --timeout=0 >/dev/null 2>&1 || main_loop=0
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 29" > /dev/kmsg
            [ -f $hookdir/initqueue/work ] && main_loop=0
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 30 main_loop=${main_loop}" > /dev/kmsg
            [ $main_loop -eq 0 ] && break
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 31" > /dev/kmsg
        done
    fi

    main_loop=$(($main_loop+1))
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 32 main_loop=$main_loop RDRETRY=$RDRETRY" > /dev/kmsg
    if [ $main_loop -gt $RDRETRY ]; then
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 33 main_loop=$main_loop RDRETRY=$RDRETRY" > /dev/kmsg
        if ! [ -f /sysroot/etc/fstab ] || ! [ -e /sysroot/sbin/init ] ; then
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 34" > /dev/kmsg
            emergency_shell "Could not boot."
        fi
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 35" > /dev/kmsg
        warn "Not all disks have been found."
        warn "You might want to regenerate your initramfs."
        break
    fi

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 36 main_loop=$main_loop RDRETRY=$RDRETRY" > /dev/kmsg
for g in $hookdir/initqueue/finished/*.sh; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh g $g > /dev/kmsg
    [ "$g" != "$hookdir/initqueue/finished/*.sh" ] && [ -e "$g" ] && cat "$g" > /dev/kmsg
done
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh sh $hookdir/initqueue/*.sh > /dev/kmsg
for g in $hookdir/initqueue/*.sh; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh g $g > /dev/kmsg
    [ "$g" != "$hookdir/initqueue/*.sh" ] && [ -e "$g" ] && cat "$g" > /dev/kmsg
done
for g in $hookdir/initqueue/settled/*.sh; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh g $g > /dev/kmsg
    [ "$g" != "$hookdir/initqueue/settled/*.sh" ] && [ -e "$g" ] && cat "$g" > /dev/kmsg
done
for g in /run/systemd/ask-password/ask.*; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh g $g > /dev/kmsg
    [ "$g" != "/run/systemd/ask-password/ask.*" ] && [ -e "$g" ] && cat "$g" > /dev/kmsg
done
for g in $hookdir/initqueue/timeout/*.sh; do
/usr/bin/echo /usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh g $g > /dev/kmsg
    [ "$g" != "$hookdir/initqueue/timeout/*.sh" ] && [ -e "$g" ] && cat "$g" > /dev/kmsg
done
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh 37 main_loop=$main_loop RDRETRY=$RDRETRY" > /dev/kmsg
done
#-----------------------------------------------------------------------------

/usr/bin/echo "/usr/lib/dracut/modules.d/98dracut-systemd/dracut-initqueue.sh end-finished" > /dev/kmsg

unset f
unset g
unset job
unset queuetriggered
unset main_loop
unset RDRETRY

export -p > /dracut-state.sh

exit 0
