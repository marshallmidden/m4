# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

def debug(printit, valueit):
    import sys
    import os
    import inspect

    callerframerecord = inspect.stack()[1]
    frame = callerframerecord[0]
    info = inspect.getframeinfo(frame)
    print("%s:%d:%s" % (os.path.basename(info.filename), info.lineno, info.function),
          "type(%s)=" % (printit), type(valueit), "%s=" % printit, valueit, file=sys.stderr)

# End of file debug.py
