#!/usr/bin/python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------

# Fibonacci numbers module

def fib(n):    # write Fibonacci series up to n
    print("fib({})".format(n))
    a, b = 0, 1
    while a < n:
        print(a, end=' ')
        a, b = b, a+b
    print()

def fib2(n):   # return Fibonacci series up to n
    print("fib2({})".format(n))
    result = []
    a, b = 0, 1
    while a < n:
        result.append(a)
        a, b = b, a+b
    return result

#-----------------------------------------------------------------------------
print("file fibo.py", type(__name__), "__name__=", __name__)
if __name__ == "__main__":
    fib(5)
    print("file fibo.py", type(fib2), "fib2(5)=", fib2(5))
#-----------------------------------------------------------------------------
# End of file fibo.py
