#!/usr/bin/python3
#-----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
import os
import readline

HIST_FILE = os.path.join(os.path.expanduser("~"), ".updateDNS_history")

def input_loop():
    if os.path.exists(HIST_FILE):
        readline.read_history_file(HIST_FILE)
    readline.set_history_length(10000)
    try:
        while True:
            line = input('Prompt ("stop" to quit): ')
            if line == 'stop':
                break
    finally:
        readline.write_history_file(HIST_FILE)

# Prompt the user for text
input_loop()
