#!/usr/bin/python3
#-----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------

import atexit
import os
import readline

histfile = os.path.join(os.path.expanduser("~"), ".updateDNS_history")
try:
    readline.read_history_file(histfile)
    # default history len is -1 (infinite), which may grow unruly
    readline.set_history_length(10000)
except FileNotFoundError:
    pass

atexit.register(readline.write_history_file, histfile)

#-- import sys
#-- import readline
#-- 
#-- readline.parse_and_bind('tab: complete')
#-- readline.parse_and_bind('set editing-mode vi')
#-- 
#-- while True:
#--     try:
#--         sys.stdout.write('ready> ')
#--         sys.stdout.flush()
#-- 
#--         line = sys.stdin.readline()
#--         if not line:
#--             break
#--         line = line.strip()
#--         if not sys.stdin.isatty():
#--             print('READ>',line)
#-- 
#--         if line == 'stop':
#--             break
#--         print('ENTERED: "%s"' % line)
#--     except SystemExit:
#--         sys.exit(0)
#--     except KeyboardInterrupt:
#--         break
#-- # elihw
