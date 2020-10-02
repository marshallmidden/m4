#!/bin/bash

# Get latest kernel dump.
F=`/bin/ls -1 /var/crash | tail -n 1`

mv /var/crash/$F /home/m4/src/crash/

exit 0
