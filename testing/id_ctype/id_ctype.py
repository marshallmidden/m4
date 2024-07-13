#!/usr/bin/python3 -B
# ----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
# ----------------------------------------------------------------------------
import ctypes
  
#-----------------------------------------------------------------------------  
val = [1, 2, 3, 4, 5]
print("Actual value of val[] - {}  type(val)={}".format(val,type(val)))

# get the memory address of the python object 
x = id(val)
print("Memory address of val[] - ", x)

# get the value through memory address
a = ctypes.cast(x, ctypes.py_object).value
print("a = ctypes.cast(x, ctypes.py_object) - ", a)
print("____________________________________")
#-----------------------------------------------------------------------------  
val = (1, 2, 3, 4, 5)
print("Actual value of val() - {}  type(val)={}".format(val,type(val)))
  
# get the memory address of the python object
x = id(val)
print("Memory address of val() - ", x)
  
# get the value through memory address
a = ctypes.cast(x, ctypes.py_object).value
print("a = ctypes.cast(x, ctypes.py_object) - ", a)
print("____________________________________")
#-----------------------------------------------------------------------------  
  
# variable declaration
val = {1, 2, 3, 4, 5}
print("Actual value of val ?? - {}  type(val)={}".format(val,type(val)))
  
# get the memory address of the python object 
x = id(val)
print("Memory address of val ?? - ", x)
  
# get the value through memory address
a = ctypes.cast(x, ctypes.py_object).value
print("a = ctypes.cast(x, ctypes.py_object) - ", a)
print("____________________________________")
#-----------------------------------------------------------------------------  
  
# variable declaration
val = {'id': 1, "name": "sravan kumar", "address": "kakumanu"}
  
# display variable
print("Actual value of val dict - {}  type(val)={}".format(val,type(val)))
  
# get the memory address of the python object 
x = id(val)
print("Memory address of val dict - ", x)
  
# get the value through memory address
a = ctypes.cast(x, ctypes.py_object).value
print("a = ctypes.cast(x, ctypes.py_object) - ", a)
print("____________________________________")
#-----------------------------------------------------------------------------  
