#!/bin/bash
# $Id: convdev.sh 12065 2006-07-25 21:30:35Z RustadM $
# A script to convert the DeviceConfiguration<MODEL>.txt file into a C
# structure initialization.
# Copyright 2006 Xiotech Corporation. All rights reserved.
# Mark D. Rustad, 2006/07/24.

cat $1 | sed -e 's/,\s*/,/g' |
    while IFS="," read mfg dev n1 n2 n3 n4 n5 n6 n7 n8 rest; do
        echo "    {${mfg},${dev},{$n1,$n2,$n3,$n4,$n5,$n6,$n7,$n8}},"
    done

# vi:sw=4 ts=4 expandtab
