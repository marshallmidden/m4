#!/usr/bin/python3 -B
# File: print_method_names.py
# ----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
# ----------------------------------------------------------------------------
import sys
import inspect
# ----------------------------------------------------------------------------
import sys
# ----------------------------------------------------------------------------
def print_methods(obj):
    """ Recursively prints the methods of a Python object, including nested class methods. """
    standard_dunder = ['__dir__', '__name__', '__module__', '__doc__', '__class__', \
        '__dict__', '__slots__', '__match_args__', '__mro__', '__bases__', '__file__', \
        '__wrapped__', '__version__', '__all__', '__debug__', \
        '__defaults__', '__kwdefaults__', '__code__', '__globals__', '__closure__', \
        '__qualname__', '__annotations__', '__type_params__', '__func__', '__self__', \
        '__loader__', '__package__', '__spec__', '__cached__', '__path__', '__traceback__', \
        '__notes__', '__context__', '__cause__', '__suppress_context__', '__objclass__', \
        '__classcell__', '__weakref__', '__origin__', '__args__', '__parameters__', \
        '__unpacked__', '__stdout__', '__stderr__', '__covariant__', '__contravariant__', \
        '__infer_variance__', '__bound__', '__constraints__', '__import__', '__builtins__', \
        '__future__', '__main__', '_DOC_ATTR' ]

    print(f"Method: {obj!r} ({type(obj).__name__})", file=sys.stderr, flush=True)

    for s in standard_dunder:
        if s == '__doc__' or s == '_DOC_ATTR' or s == '__dict__': continue
        if hasattr(obj,s):
            print(f'{s} - {getattr(obj,s)}', file=sys.stderr, flush=True)
        # fi
    # rof
    print("", file=sys.stderr, flush=True)

    methods = [met for met in obj.__dir__() ]

    # Print the methods of the current object
    dotmethod = ''
    dotattribute = ''
    for met in sorted(methods):
        if met in standard_dunder: continue
        m = getattr(obj, met)
        if callable(m):
            dotmethod += f"Method: {met}\n"
        elif hasattr(m, '__class__'):
            dotattribute += f"Attribute: {met} - {getattr(obj,met)}\n"
        else:
            print(f'Unrecognized: {met} m={m}', file=sys.stderr, flush=True)
        # fi
    # rof
    if dotmethod != '': print(dotmethod, file=sys.stderr, flush=True, end='')
    if dotattribute != '': print(dotattribute, file=sys.stderr, flush=True, end='')
# End of print_methods

# ----------------------------------------------------------------------------
#- Special Methods
#- __delattr__: Called when an attribute is deleted with del obj.attribute.
#- __dir__: Called by the dir() function to get a list of attributes.
#- __eq__: Called to compare two objects for equality (==).
#- __format__: Called by the format() function to format the object.
#- __ge__: Called to compare if the object is greater than or equal to another (>=).
#- __getattribute__: Called to get an attribute of the object.
#- __gt__: Called to compare if the object is greater than another (>).
#- __hash__: Called by the hash() function to get the hash value of the object.
#- __init__: Called when an instance of the class is created, initializing the object.
#- __init_subclass__: Called when a class is subclassed.
#- __le__: Called to compare if the object is less than or equal to another (<=).
#- __lt__: Called to compare if the object is less than another (<).
#- __ne__: Called to compare if the object is not equal to another (!=).
#- __new__: Called to create a new instance of a class.
#- __reduce__: Used by the pickle module to support object serialization.
#- __reduce_ex__: A version of __reduce__ with an additional argument for compatibility with different versions of pickle.
#- __repr__: Called by the repr() function to get a string representation of the object for debugging.
#- __setattr__: Called when an attribute is set.
#- __sizeof__: Called by the sys.getsizeof() function to get the size of the object.
#- __str__: Called by the str() function to get a string representation of the object for end-users.
#- __subclasshook__: Called to check if a class is a subclass of another class, used for abstract base classes.
#
#- __mro__: the blasses that the instance B.__mro__ [or B.mro()] is made up of. I.e. class B(A): [B inherits A]
# ----------------------------------------------------------------------------
