#!/usr/bin/python3
#-----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------

import sys
import subprocess

def run_command(cmd = 'echo'):
    try:
        # capture_output in result.stdout.
        # capture_stderr in result.stderr.
        # check for non-zero exit code -- try/except.
        # input = string of bytes
        result = subprocess.run(
                                [cmd, "-nvte", "-"],
                                capture_output=True, text=True,
                                input='hi there.\n\r\tHow are you?\nEOF',
                                check=True
        )
        print("stdout:\n{}".format(result.stdout))
        print("stderr:\n{}".format(result.stderr))
    except:
        print("Error running executable {} - {}".format(cmd, sys.exc_info()[0]))
    # ytr
# End of run_command

run_command('ohoh')
run_command('/bin/cat')
