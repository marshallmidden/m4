#!/usr/bin/python3
#-----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
import readline # activate readline editing

# File: readline-example-2.py

class Completer:
    def __init__(self, words):
        self.words = words
        self.prefix = None
    def complete(self, prefix, index):
        print("type(prefix)={} prefix={} type(index)={} index={}".format(type(prefix),prefix,type(index),index))
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

import readline

# a set of more or less interesting words
words = "perl", "pyjamas", "python", "pythagoras"

completer = Completer(words)

readline.parse_and_bind("tab: complete")
readline.set_completer(completer.complete)

# try it out!
while 1:
    print(readline.get_completer())
    print(readline.get_begidx())
    print(readline.get_endidx())
    print(readline.get_completer_delims())
    print(repr(input(">>> ")))

