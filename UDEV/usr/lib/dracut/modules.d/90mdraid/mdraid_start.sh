#!/bin/sh

/usr/bin/echo "STARTED /usr/lib/dracut/modules.d/90mdraid/mdraid_start.sh" > /dev/kmsg

type getargs >/dev/null 2>&1 || . /lib/dracut-lib.sh

_md_start() {
    local _udevinfo
    local _path_s
    local _path_d
    local _md="$1"

/usr/bin/echo "_md_start ${_md} in /usr/lib/dracut/modules.d/90mdraid/mdraid_start.sh" > /dev/kmsg
    _udevinfo="$(udevadm info --query=env --name="${_md}")"
    strstr "$_udevinfo" "MD_LEVEL=container" && continue
    strstr "$_udevinfo" "DEVTYPE=partition" && continue

    _path_s="/sys/$(udevadm info -q path -n "${_md}")/md/array_state"
    [ ! -r "$_path_s" ] && continue

    # inactive ?
    [ "$(cat "$_path_s")" != "inactive" ] && continue

    mdadm -R "${_md}" 2>&1 | vinfo

    # still inactive ?
    [ "$(cat "$_path_s")" = "inactive" ] && continue

    _path_d="${_path_s%/*}/degraded"
    [ ! -r "$_path_d" ] && continue
    > $hookdir/initqueue/work
}

_md_force_run() {
    local _md
    local _UUID
    local _MD_UUID=$(getargs rd.md.uuid -d rd_MD_UUID=)
    [ -n "$_MD_UUID" ] || getargbool 0 rd.auto || return

    if [ -n "$_MD_UUID" ]; then
        _MD_UUID=$(str_replace "$_MD_UUID" "-" "")
        _MD_UUID=$(str_replace "$_MD_UUID" ":" "")

        for _md in /dev/md[0-9_]*; do
/usr/bin/echo "_md_force_run loop1 ${_md} /usr/lib/dracut/modules.d/90mdraid/mdraid_start.sh" > /dev/kmsg
            [ -b "$_md" ] || continue
            _UUID=$(
                /sbin/mdadm -D --export "$_md" \
                    | while read line || [ -n "$line" ]; do
                    str_starts "$line" "MD_UUID=" || continue
                    printf "%s" "${line#MD_UUID=}"
                done
                )

            [ -z "$_UUID" ] && continue
            _UUID=$(str_replace "$_UUID" ":" "")

            # check if we should handle this device
            strstr " $_MD_UUID " " $_UUID " || continue

            _md_start "${_md}"
        done
    else
        # try to force-run anything not running yet
        for _md in /dev/md[0-9_]*; do
/usr/bin/echo "_md_force_run loop2 ${_md} /usr/lib/dracut/modules.d/90mdraid/mdraid_start.sh" > /dev/kmsg
            [ -b "$_md" ] || continue
            _md_start "${_md}"
        done
    fi
}

_md_force_run
