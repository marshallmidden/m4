#!/bin/bash -ex
F=`/bin/ls -1 /var/crash | tail -n 1`

# MODULE="/usr/lib/modules/3.10.0-693.11.1.el7.lightspeed.x86_64/build/vmlinux"
# MODULE="/usr/lib/modules/3.10.0-693.21.1.el7.lightspeed.x86_64/build/vmlinux"

crash --dec ${MODULE} /var/crash/${F}/vmcore
