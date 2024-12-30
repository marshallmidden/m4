#!/bin/sh

set -x

test -z "/usr/local/bin" || mkdir -p -- "/usr/local/bin"

/usr/bin/install -c 'kme'     '/usr/local/bin/kme'
/usr/bin/install -c 'kmed'    '/usr/local/bin/kmed'
/usr/bin/install -c 'elf2kme' '/usr/local/bin/elf2kme'

test -z "/usr/local/share/kme" || mkdir -p -- "/usr/local/share/kme"

/usr/bin/install -c -m 644 'i80321.kme' '/usr/local/share/kme/i80321.kme'
/usr/bin/install -c -m 644 'gt64011.kme' '/usr/local/share/kme/gt64011.kme'

test -z "/usr/local/man/man1" || mkdir -p -- "/usr/local/man/man1"

/usr/bin/install -c -m 644 './kme.1' '/usr/local/man/man1/kme.1'

test -z "/usr/local/man/man8" || mkdir -p -- "/usr/local/man/man8"

/usr/bin/install -c -m 644 './kmed.8' '/usr/local/man/man8/kmed.8'
