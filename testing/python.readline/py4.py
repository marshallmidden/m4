#!/usr/bin/python3
#-----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
import os
import readline
import rlcompleter
import sys
# Use the tab key for completion
if sys.platform == 'darwin':
    readline.parse_and_bind ("bind ^I rl_complete")
else:
    readline.parse_and_bind("tab: complete")
#-----------------------------------------------------------------------------
import logging
LOG_FILENAME = '/tmp/completer.log'
#-- logging.basicConfig(format='%(message)s', filename=LOG_FILENAME, level=logging.DEBUG)
logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s',
                    filename=LOG_FILENAME,
                    level=logging.DEBUG,
                    datefmt='%Y-%m-%d_%H-%M-%S')

#-----------------------------------------------------------------------------
class dnsCompleter:

    def __init__(self, options):
        self.options = options
        self.current_candidates = []

    def complete(self, text, state):
        logging.debug('entering complete(text={}, state={})'.format(text, state))
        response = None
        if state == 0:
            # This is the first time for this text,
            # so build a match list.

            origline = readline.get_line_buffer().rstrip()
            line = origline.lstrip()
            logging.debug('line={}'.format(line))
            delta = len(origline) - len(line)
            logging.debug('delta={}'.format(delta))
            begin = readline.get_begidx() - delta
            end = readline.get_endidx() - delta
            being_completed = line[begin:end]
            words = line.split()

            logging.debug('origline={}'.format(origline))
            logging.debug('begin={}'.format(begin))
            logging.debug('end={}'.format(end))
            logging.debug('being_completed={}'.format(being_completed))
            logging.debug('words={}'.format(words))

            logging.debug("type(self.options)={} self.options={}".format(type(self.options), self.options))
            if not words:
                self.current_candidates = sorted(self.options)
            else:
                try:
                    if begin == 0:
                        # first word
                        candidates = self.options
                    else:
                        # later word
                        return None
                    # fi

                    if being_completed:
                        # match options with portion of input
                        # being completed
                        self.current_candidates = [ w for w in candidates if w.startswith(being_completed) ]
                    else:
                        # matching empty string,
                        # use all candidates
                        self.current_candidates = candidates
                    # fi

                    logging.debug('candidates={} self.candidates={}'.format(candidates, self.current_candidates))

                except (KeyError, IndexError) as err:
                    logging.error('completion error: {}'.format(err))
                    self.current_candidates = []
                # yrt
            # fi
        # fi
        try:
            response = self.current_candidates[state]
        except IndexError:
            response = None
        # fi
        logging.debug('exiting complete with {} (from {}[{}]'.format(response, self.current_candidates, state))
        return response

def input_loop():
    line = ''
    print("here#1", file=sys.stderr)
    while line != 'quit':
        print("here#2", file=sys.stderr)
        input('Prompt ("quit" to quit): ')
        print("here#3", file=sys.stderr)
        line = readline.get_line_buffer().strip()
        print("here#4", file=sys.stderr)
        print("type(line)={} line='{}'".format(type(line), line))
        print("here#5", file=sys.stderr)
        print('Dispatch {}'.format(line))
        print("here#6", file=sys.stderr)
    # elihw

# Register our completer function
tab_words = ['exit ', 'quit ', 'help ', 'add ', 'remove ', 'delete ', 'show ']

completer = dnsCompleter( tab_words )

readline.set_completer(completer.complete)

# Prompt the user for text
input_loop()
#-----------------------------------------------------------------------------
