#!/usr/bin/python3
#-----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------

import sys
import readline

readline.parse_and_bind('tab: complete')
readline.parse_and_bind('set editing-mode vi')

while True:
    try:
        sys.stdout.write('ready> ')
        sys.stdout.flush()

        line = sys.stdin.readline()
        if not line:
            break
        line = line.strip()
        if not sys.stdin.isatty():
            print('READ>',line)

        if line == 'stop':
            break
        print('ENTERED: "%s"' % line)
    except SystemExit:
        sys.exit(0)
    except KeyboardInterrupt:
        break
# elihw
