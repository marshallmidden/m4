#!/usr/bin/python3
#-----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
import os
import readline

HIST_FILE = os.path.join(os.path.expanduser("~"), ".updateDNS_history")

if os.path.exists(HIST_FILE):
    readline.read_history_file(HIST_FILE)
readline.set_history_length(10000)


# cmd_simple.py

import cmd

class HelloWorld(cmd.Cmd):
    def do_greet(self, line):
        print("hello")
        return

    def do_EOF(self, line):
        return True

if __name__ == '__main__':
    HelloWorld().cmdloop()

#-----------------------------------------------------------------------------
