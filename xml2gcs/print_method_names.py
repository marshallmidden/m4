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

    standard_dunder = [
        '_DOC_ATTR', '__abs__', '__add__', '__aenter__', '__aexit__', '__aiter__',
        '__all__', '__and__', '__anext__', '__annotations__', '__args__', '__await__',
        '__bases__', '__bool__', '__bound__', '__buffer__', '__builtins__', '__bytes__',
        '__cached__', '__cause__', '__ceil__', '__class__', '__class_getitem__',
        '__classcell__', '__closure__', '__code__', '__complex__', '__constraints__',
        '__contains__', '__context__', '__contravariant__', '__copy__', '__covariant__',
        '__debug__', '__deepcopy__', '__defaults__', '__del__', '__delattr__', '__delete__',
        '__delitem__', '__dict__', '__dir__', '__divmod__', '__doc__', '__enter__',
        '__eq__', '__exit__', '__file__', '__float__', '__floor__', '__floordiv__',
        '__format__', '__fspath__', '__func__', '__future__', '__ge__', '__get__',
        '__getattr__', '__getattribute__', '__getformat__', '__getitem__', '__getnewargs__',
        '__getnewargs_ex__', '__getstate__', '__globals__', '__gt__', '__hash__', '__iadd__',
        '__iand__', '__ifloordiv__', '__ilshift__', '__imatmul__', '__imod__', '__import__',
        '__imul__', '__index__', '__infer_variance__', '__init__', '__init_subclass__',
        '__instancecheck__', '__int__', '__invert__', '__ior__', '__ipow__', '__irshift__',
        '__isub__', '__iter__', '__itruediv__', '__ixor__', '__kwdefaults__', '__le__',
        '__len__', '__length_hint__', '__loader__', '__lshift__', '__lt__', '__main__',
        '__match_args__', '__matmul__', '__missing__', '__mod__', '__module__', '__mro__',
        '__mro_entries__', '__mul__', '__name__', '__ne__', '__neg__', '__new__', '__next__',
        '__notes__', '__objclass__', '__or__', '__origin__', '__package__', '__parameters__',
        '__path__', '__pos__', '__post_init__', '__pow__', '__prepare__', '__qualname__',
        '__radd__', '__rand__', '__rdivmod__', '__reduce__', '__reduce_ex__',
        '__release_buffer__', '__repr__', '__reversed__', '__rfloordiv__', '__rlshift__',
        '__rmatmul__', '__rmod__', '__rmul__', '__ror__', '__round__', '__rpow__',
        '__rrshift__', '__rshift__', '__rsub__', '__rt__', '__rtruediv__', '__rxor__',
        '__self__', '__set__', '__set_name__', '__setattr__', '__setitem__', '__setstate__',
        '__sizeof__', '__slots__', '__spec__', '__stderr__', '__stdout__', '__str__',
        '__sub__', '__subclasscheck__', '__subclasses__', '__subclasshook__',
        '__suppress_context__', '__traceback__', '__truediv__', '__trunc__', '__type_params__',
        '__unpacked__', '__version__', '__weakref__', '__wrapped__', '__xor__' ]

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
