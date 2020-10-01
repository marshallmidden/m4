#!/usr/bin/python2
#-----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
# File: readline-example-2.py

import os
import sys
import readline
import rlcompleter
if sys.platform == 'darwin':
    readline.parse_and_bind ("bind ^I rl_complete")
else:
    readline.parse_and_bind("tab: complete")

class Completer:
    def __init__(self, words):
        self.words = words
        self.prefix = None
    def complete(self, prefix, index):
        print("complete: type(prefix)={} prefix={} type(index)={} index={}".format(type(prefix), prefix, type(index), index))
        if prefix != self.prefix:
            # we have a new prefix!
            # find all words that start with this prefix
            self.matching_words = [
                w for w in self.words if w.startswith(prefix)
                ]
            self.prefix = prefix
        try:
            return self.matching_words[index]
        except IndexError:
            return None

# a set of more or less interesting words
words = "perl", "pyjamas", "python", "pythagoras"


#-- readline.parse_and_bind("bind ^I rl_complete")
completer = Completer(words)
readline.set_completer(completer.complete)

# try it out!
while 1:
    print repr(raw_input(">>> "))

