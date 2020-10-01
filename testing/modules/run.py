#!/usr/bin/python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
print(type(__name__), "__name__=", __name__)
#-----------------------------------------------------------------------------
import fibo
import builtins
#-----------------------------------------------------------------------------
fibo.fib(9)
print("file run.py", type(fibo.fib2), "fibo.fib2(16)=", fibo.fib2(16))
#-----------------------------------------------------------------------------
print("dir(fibo)=", dir(fibo))
print("dir(builtins)=", dir(builtins))
print("globals()=", globals())
#-----------------------------------------------------------------------------
# End of file run.py

