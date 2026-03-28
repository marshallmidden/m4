#!/usr/bin/python3 -B
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
"""Expression parser and variable system for the music composition preprocessor.

Provides a complete mathematical expression evaluator supporting:
  - Arithmetic:  +, -, *, /, ** (power)
  - Comparison:  ==, !=, <, >, <=, >=
  - Bitwise:     $and$, $or$, $mask$, $union$, $diff$, $cls$, $ars$
  - Assignment:  =
  - Functions:   sin, cos, sqrt, log, ln, abs, ceil, floor, round, etc.
  - Variables:   scalars and multi-dimensional arrays
  - Strings:     character-string values with concatenation

Variables are stored in two lists (global 'arrays' and 'local_arrays'),
each entry being a list with fields accessed via module-level index constants
(numarry_name, numarry_maclevel, etc.).

Can also run standalone as an interactive calculator (python3 calculate.py).
"""
from __future__ import annotations
#-----------------------------------------------------------------------------
# Need to do a bunch of things.
# a) m1 = abc1(1)       # CHAR
# b) character string assignment.
#     Operations on character strings (+)
# c) single and double quotes for strings and operations.
#-----------------------------------------------------------------------------
# TO DO:
#   1) Can change functions into:
#      ftof()   floating input to floating output
#      itoi()   integer(from floating) to integer   (integer input and output)
#      ftoi()   floating to integer                 (integer on output)
#      itof()   integer(from floating) to floating  (floating on output)
#   2) try/except around math ?  Eliminate current? Add small limited - oor stuff?
#   3) Check all errors handled properly. (return None)
#      Can fix otherwise to return error message?
#-----------------------------------------------------------------------------
import sys
import readline
import re
import math
from datetime import datetime
# Pre-compiled regex patterns for performance.
_RE_OCTAL = re.compile(r'^o[0-7]+$')
_RE_OPERATOR = re.compile(r'[-+*/=$<>!]')
_RE_IDENTIFIER = re.compile(r'[_a-zA-Z0-9]+')
_RE_NUMBER = re.compile(r'[0-9.]+')
_RE_BRACKET = re.compile(r'[\[\](){}]')
_RE_CHAR_SPLIT = re.compile(r'([a-zA-Z0-9_+-]+|[ \t]*[^a-zA-Z0-9_+-]+[ \t]*)')
_RE_WHITESPACE = re.compile(r'\s+')
_RE_WORD = re.compile(r'\w+')
_RE_DOLLAR_COMMENT = re.compile(r'[$][$].*$')
#-----------------------------------------------------------------------------
#++ import inspect
#++ print(inspect.currentframe().f_code.co_name, '#0', file=sys.stderr, flush=True)
#-----------------------------------------------------------------------------
global arrays
arrays = [ ]
global local_arrays
local_arrays = [ ]
variable_index = {}             # name -> list of wary entries (for fast lookup)
#-----------------------------------------------------------------------------
numarry_name = 0            # The name of the variable.
numarry_maclevel = 1        # The macro level was in effect when created.
numarry_indexes = 2         # The array indexes. []=value, [3]=1-dimen, [2,4]=2-dimen.
numarry_values = 3          # Array of values ([0] for not an array).
numarry_value_type = 4      # Array of types None=not-set, 0=int/float, 1= character string.
numarry_macro_arg = 5       # True if macro argument - normally False.

def _var_index_add(wary):
    """Add a variable entry to the variable_index for fast lookup."""
    name = wary[numarry_name]
    if name not in variable_index:
        variable_index[name] = []
    # fi
    variable_index[name].append(wary)
# End of _var_index_add

def _var_index_remove(wary):
    """Remove a variable entry from the variable_index."""
    name = wary[numarry_name]
    if name in variable_index:
        try:
            variable_index[name].remove(wary)
        except ValueError:
            pass
        # yrt
        if not variable_index[name]:
            del variable_index[name]
        # fi
    # fi
# End of _var_index_remove

# None if variable is not set yet.
type_is_number = 0          # Type of variable is a number.
type_is_string = 1          # Type of variable is a string.

#-- warray = [ 'abc', 8,
#--            [ ],             # Zero dimension
#--            [ 123.000 ],     # value
#--            [ 0 ],           #  int/float
#--            False ]          # Not macro argument.
#-- arrays.append(warray)
#-- 
#-- warray = [ 'abc1', 8,
#--            [ 2 ],           # 1 dimension
#--            [ None, None ],
#--            [ None, None ],  # Character string, int/float
#--            False ]          # Not macro argument.
#-- arrays.append(warray)
#-- 
#-- warray = [ 'abc2', 8,
#--            [ 1, 2 ],        # 2 dimensions
#--            [ 'abc2', 123.002 ],
#--            [ 1, 0 ],        # character string, int/float
#--            False ]          # Not macro argument.
#-- arrays.append(warray)
#-- 
#-- warray = [ 'abc3', 8,
#--            [ 1, 1, 2 ],     # 3 dimensions
#--            [ 'abc3', 123.003 ],
#--            [ 1, 0 ],        # character string, int/float
#--            False ]          # Not macro argument.
#-- arrays.append(warray)
#-- 
#-- warray = [ 'def', 8,
#--            [ 2, 2 ],        # 2 dimensions
#--            [ 1, 2, 3, 4 ],
#--            [ 0, 0, 0, 0 ],  # Not-set-yet, int/float
#--            False ]          # Not macro argument.
#-- arrays.append(warray)
#-- 
#-- warray = [ 'ghi', 8,
#--            [ 3, 2, 2 ],     # 3 dimensions
#--            [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 ],
#--            [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],     # Not-set-yet, int/float
#--            False ]          # Not macro argument.
#-- arrays.append(warray)
#-- 
#-- warray = [ 'xyz', 8,
#--            [ ], 
#--            [ 'm1' ],
#--            [ 1 ],           # character string
#--            False ]          # Not macro argument.
#-- local_arrays.append(warray)
#-- 
#-- warray = [ 'tuv', 8,
#--            [ ], 
#--            [ '1.234' ],
#--            [ 1 ],           # character string
#--            False ]          # Not macro argument.
#-- local_arrays.append(warray)
#-- 
#-- warray = [ 'CHARSTRING', 8,
#--            [ 2 ], 
#--            [ '3e+4 4c8 [4d2 5a] 3c4 5a2', 'second-argument' ],
#--            [ 1,1 ],         # character strings
#--            False ]          # Not macro argument.
#-- local_arrays.append(warray)
#-----------------------------------------------------------------------------
class SymbolDesc:
    def __init__(self, symbol, lprio, rprio, eval) -> None:
        """Initialize a symbol descriptor with operator string, left/right priorities, and eval function."""
        self.symbol = symbol
        self.lprio = lprio
        self.rprio = rprio
        self.eval = eval
    # End of __init__

    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def __repr__(self) -> str:
        """Return a human-readable string representation of this symbol descriptor."""
        return "SymbolDesc: '{}',{},{}".format(self.symbol, self.lprio, self.rprio)
    # End of __repr__

#-----------------------------------------------------------------------------
def is_octal(string: str) -> bool:
    """Check if a string represents an octal number (starts with 'o', followed by octal digits)."""
    return bool(_RE_OCTAL.match(string))
# is_octal

#-----------------------------------------------------------------------------
# Returns:
#   therest - everything after this token
#   token   - the token
#   kind    - 'NUMBER' if a number

def next_token(string: str) -> tuple[str | None, str | float, str]:
    """Extract the next token from the expression string, classifying it by type (NUMBER, ID, OPER, CHAR, SYNTAX)."""
#PRINT    print("next_token - Entering type(string)='{}' string='{}'".format(type(string),string), file=sys.stderr, flush=True)   # PRINT
    # Make sure string exists. next_token useable by other than tokenize.
    string = string.strip()
    if not string:
        return None, None, None
    # fi
    # First character processing is special.
    c = string[0]                           # Character in string
    if c in [ '"', "'" ]:
        if len(string) >= 1:
            x = string[1:].find(c)          # Find the terminating single/double quote.
            if x >= 0:
                therest = string[x+2:]
                strng = string[1:x+1]
                return therest, strng , 'CHAR'
            # fi
        # fi
    # fi
    if _RE_OPERATOR.match(c):         # Possible operation
        if c == '*' and len(string) >= 2:   # Possible '**'
            if string[1] == '*':
                return string[2:], '**', 'OPER'
            # fi
            return string[1:], '*', 'OPER'
        elif c == '$':
            if len(string) >= 7 and string[0:7] == '$union$':
                return string[7:], '$union$', 'OPER'
            elif len(string) >= 6:
                if string[0:6] == '$mask$':
                    return string[6:], '$mask$', 'OPER'
                elif string[0:6] == '$diff$':
                    return string[6:], '$diff$', 'OPER'
                # fi
            # fi
            if len(string) >= 5:
                if string[0:5] == '$and$':
                    return string[5:], '$and$', 'OPER'
                elif string[0:5] == '$cls$':
                    return string[5:], '$cls$', 'OPER'
                elif string[0:5] == '$ars$':
                    return string[5:], '$ars$', 'OPER'
                # fi
            # fi
            if len(string) >= 4:
                if string[0:4] == '$or$':
                    return string[4:], '$or$', 'OPER'
                # fi
            # fi
            if len(string) > 1:
                return string[1:], c, 'MISMATCH #a len(string)={}'.format(len(string))
            # fi
            return None, c, 'MISMATCH #b len(string)={}'.format(len(string))
        elif c == '=' and len(string) >= 2: # Possible ==, =>, =<
#PRINT            print("next_token#1({}) - c='{}'".format(string,c), file=sys.stderr, flush=True) # PRINT
            if string[1] == '=':
                return string[2:], '==', 'OPER'
            elif string[1] == '>':
                return string[2:], '>=', 'OPER'
            elif string[1] == '<':
                return string[2:], '<=', 'OPER'
            # fi
            return string[1:], '=', 'OPER'
        elif c == '!' and len(string) >= 2: # Possible !=
#PRINT            print("next_token#2({}) - c='{}'".format(string,c), file=sys.stderr, flush=True) # PRINT
            if string[1] == '=':
                return string[2:], '!=', 'OPER'
            # fi
            return None, c, 'MISMATCH #c len(string)={}'.format(len(string))
        elif c == '<' and len(string) >= 2: # Possible <=
#PRINT            print("next_token#3({}) - c='{}'".format(string,c), file=sys.stderr, flush=True) # PRINT
            if string[1] == '=':
                return string[2:], '<=', 'OPER'
            # fi
            return string[1:], '<', 'OPER'
        elif c == '>' and len(string) >= 2: # Possible <=
#PRINT            print("next_token#4({}) - c='{}'".format(string,c), file=sys.stderr, flush=True) # PRINT
            if string[1] == '=':
                return string[2:], '>=', 'OPER'
            # fi
            return string[1:], '>', 'OPER'
        # The +-(/= reach below.
        elif len(string) > 1:
            return string[1:], c, 'OPER'
        # fi
        return None, c, 'OPER'
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    elif is_octal(string):                  # First is a leading 'o' for octal number following.
        m = _RE_OCTAL.match(string)
        strg = m.group(0)[1:]
        ret = int(strg,8)
        b = float(ret)
        return string[len(strg)+1:], b, 'NUMBER'
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    elif c.isalpha():                       # First character is [a-zA-Z].
        m = _RE_IDENTIFIER.match(string)
        ret = strg = m.group(0)             # We know there is at least one character.
        if strg == 'lpause':                # convert a few names to the other name.
            ret = 'pause'
        elif strg == 'lstac':
            ret = 'stac'
        elif strg == 'lgrace':
            ret = 'grace'
        # fi
        if len(string) > len(strg):         # Something after this alphabetic string.
            return string[len(strg):], ret, 'ID'
        # fi
        return None, ret, 'ID'
    elif c.isdigit() or c == '.':           # First is a digit.
        m = _RE_NUMBER.match(string)
        strg = m.group(0)                   # We know there is at least one character.
        if strg.count('.') > 1:             # Only one decimal place in a number.
            return None, strg, 'MISMATCH #d strg={}'.format(strg)
        elif len(string) > len(strg):         # Something after this alphabetic string.
            b = float(strg)
            return string[len(strg):], b, 'NUMBER'
        # fi
        return None, strg, 'NUMBER'
    elif _RE_BRACKET.match(c):        # Known parenthesis/brackets/braces.
        if len(string) > 1:                 # Something after this alphabetic string.
            return string[1:], c, 'SYNTAX'
        # fi
        return None, c, 'SYNTAX'
    # comma reached below.
    elif len(string) > 1:
        return string[1:], c, 'OPER'
    # fi

    # Everything else is an error.
    return None, c, 'MISMATCH #e c={}'.format(c)
# End of next_token

#-----------------------------------------------------------------------------
def tokenize(code) -> None:
    """Generator that yields all tokens from the expression string, inserting implied multiplication where needed."""
#PRINT    print("tokenize - Entering code='{}'".format(code), file=sys.stderr, flush=True) # PRINT
    # Get next token.
    last_kind = ''
    last_token = ''
    while code is not None and code != '':
        therest, token, kind = next_token(code)
#PRINT        print("tokenize - {},{},{}=next_token({})".format(therest,token,kind,code), file=sys.stderr, flush=True) # PRINT
        if (last_kind == 'SYNTAX' and kind == 'SYNTAX' and
              (last_token == ')' and token == '{')):
            # character string token limiting.
#PRINT            print("tokenize - imply TOKEN-LIMITING", file=sys.stderr, flush=True) # PRINT
            yield ['OPER', 'TOKEN-LIMITING']
        elif last_kind in ['NUMBER', 'ID'] and kind in ['NUMBER', 'ID']:
            # Implied multiplication between numbers and id's.
            yield ['OPER', '*']
        elif (last_kind == 'SYNTAX' and kind == 'SYNTAX' and
              (last_token in [')', ']', '}'] and token in ['(', '[', '{'])):
            # Implied multiplication between () and ().
            yield ['OPER', '*']
        elif (last_kind == 'SYNTAX' and last_token in [')',']','}'] and
              (kind == 'ID' or kind == 'NUMBER') ):
            # Implied multiplication between () and numbers or id.
            yield ['OPER', '*']
        # fi
        yield [kind, token]
        last_kind = kind
        last_token = token
        code = therest
    # elihw
# End of tokenize

#-----------------------------------------------------------------------------
def identity_eval(args: list) -> list:
    """Evaluate an identity expression (variable reference or numeric literal)."""
#PRINT    print("identity_eval - Entering args='{}'".format(args), file=sys.stderr, flush=True)    # PRINT
    if len(args) == 1:
        if isinstance(args[0], SymbolDesc):
            return [ args[0].symbol ]
        # fi
    elif len(args) == 2:
        a = args[0]
        a = fix_to_number(a[0],a[1])        # Convert char to number.
        if a[0] == 'NUMBER':
            a[1] = float(a[1])
            return [ a ]
        # fi
    # fi
    return [ "ERROR - ID type(args[0])={} args[0]='{}'".format(type(args[0]), args[0]), None ]
# End of identity_eval

#-----------------------------------------------------------------------------
def get_value(arg: list) -> list:
    """Retrieve the value of a variable from global or local arrays, or return a literal value."""
    global arrays
    global local_arrays

#PRINT    print("get_value - Entering arg='{}'".format(arg), file=sys.stderr, flush=True)  # PRINT
    if arg[0].startswith('ERROR'):
#PRINT        print("get_value - #a", file=sys.stderr, flush=True)  # PRINT
        return arg
    elif arg[0] == 'NUMBER':
#PRINT        print("get_value - #b - type(arg[1])={} arg[1]='{}'".format(type(arg[1]),arg[1]), file=sys.stderr, flush=True)  # PRINT
        arg[1] = float(arg[1])
#PRINT        print("get_value - #b1 - type(arg[1])={} arg[1]='{}'".format(type(arg[1]),arg[1]), file=sys.stderr, flush=True)  # PRINT
        return arg
    elif arg[0] == 'COMMA':
#PRINT        print("get_value - #c", file=sys.stderr, flush=True)  # PRINT
        return arg
    elif arg[0] == 'CHAR':
#PRINT        print("get_value - #d", file=sys.stderr, flush=True)  # PRINT
        return arg
    elif arg[0] == 'ADDRESS':
#PRINT        print("get_value - #e", file=sys.stderr, flush=True)  # PRINT
        a = arg[1]
        idx = a[0]
        warray = a[1]
        where_value = warray[numarry_values][idx]
        where_type = warray[numarry_value_type][idx]
        if where_type is None:
            arg[0] = 'CHAR'
            arg[1] = None
        elif where_type == type_is_number:
            arg[0] = 'NUMBER'
            arg[1] = float(where_value)
        else:
            arg[0] = 'CHAR'
            arg[1] = where_value
        # fi
#PRINT        print("get_value - #f", file=sys.stderr, flush=True)  # PRINT
        return arg
    elif arg[0] != 'ID':
#PRINT        print("get_value - #g", file=sys.stderr, flush=True)  # PRINT
        return [ "ERROR - get_value - unrecognized variable type='{}'".format(arg), None ]
    # fi

#PRINT    print("get_value - #h", file=sys.stderr, flush=True)  # PRINT
    maxmaclev = -1
    maxwary = None
#--    print('arrays={}'.format(arrays), file=sys.stderr, flush=True)
#--    print('local_arrays={}'.format(local_arrays), file=sys.stderr, flush=True)
    for wary in variable_index.get(arg[1], []):
#PRINT        print('arg[1]={} wary[numarry_name]={} wary[numarry_values]={}'.format(arg[1],wary[numarry_name],wary[numarry_values]), file=sys.stderr, flush=True)  # PRINT
        if wary[numarry_maclevel] >  maxmaclev:
            maxmaclev = wary[numarry_maclevel]
            maxwary = wary
        # fi
    # rof
#PRINT    print("get_value - #i maxwary={}".format(maxwary), file=sys.stderr, flush=True)  # PRINT
    if maxwary is None:
        return [ "ERROR - get_value - unrecognized variable='{}'".format(arg), None ]
    elif maxwary[numarry_indexes]:
        return [ "ERROR - get_value - array needs '{}' arguments".format(len(maxwary[numarry_indexes])), None ]
    elif maxwary[numarry_value_type][0] is None:
        return [ "ERROR - get_value - variable is not set yet - '{}'".format(arg), None ]
    elif maxwary[numarry_value_type][0] == type_is_number:
        arg[0] = 'NUMBER'
        arg[1] = float(maxwary[numarry_values][0])
    else:
        arg[0] = 'CHAR'
        arg[1] = str(maxwary[numarry_values][0])
    # fi
#PRINT    print("get_value - #z arg[1]='{}'".format(arg[1]), file=sys.stderr, flush=True)  # PRINT
    return arg
# End of get_value

#-----------------------------------------------------------------------------
def fix_to_number(t,a) -> tuple:
    """Attempt to convert a character-string value to a numeric (float) value."""
    if t == 'CHAR':
        try:
            a = float(a)
            t = 'NUMBER'
        except (ValueError, TypeError):
            pass
        # fi
    # fi
    return t, a
# End of fix_to_number

#-----------------------------------------------------------------------------
#        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
# Try to make both a1 and a2 numbers.
def fix_to_numbers(t1,a1,t2,a2) -> tuple:
    """Attempt to convert both operands to numeric values."""
#PRINT    print("fix_to_numbers - Entering t1={} a1={} t2={} a2={}".format(t1,a1,t2,a2), file=sys.stderr, flush=True)  # PRINT
    t1, a1 = fix_to_number(t1, a1)
    t2, a2 = fix_to_number(t2, a2)
    return t1,a1,t2,a2
# End of fix_to_numbers

#-----------------------------------------------------------------------------
def compute_value(op: str, arg1: list, arg2: list) -> list:
    """Evaluate a binary operation: arithmetic (+,-,*,/,**), comparison (==,!=,<,>,<=,>=), logical, bitwise, or assignment (=)."""
    global arrays
    global local_arrays

#PRINT    print("compute_value - Entering types:{} {} {} , vals:'{}' '{}' '{}'".format(type(op),type(arg1),type(arg2),op,arg1,arg2), file=sys.stderr, flush=True)  # PRINT
    a2 = get_value(arg2)
#PRINT    print("compute_value - after get_value a2='{}'".format(a2), file=sys.stderr, flush=True)  # PRINT
    t2 = a2[0]
    if a2 is None:
        return [ "ERROR - Argument2 is None a2='{}'".format(a2), None ]
    elif t2.startswith('ERROR'):
        return a2
    # fi
    if arg1 is None:
        return [ "ERROR - Argument1 is None arg1='{}'".format(arg1), None ]
    elif arg1[0].startswith('ERROR'):
#PRINT        print("compute_value - #a", file=sys.stderr, flush=True)  # PRINT
        return arg1
    # fi
    if t2 not in [ 'NUMBER', 'CHAR', 'COMMA' ]:
        return [ "ERROR - Argument2 is not a Number, character string, or comma list - a2='{}'".format(a2), None ]
    elif op == '=':
#PRINT        print("compute_value - #b", file=sys.stderr, flush=True)  # PRINT
        t1 = arg1[0]
        a = arg1[1]
#PRINT        print("compute_value - #c", file=sys.stderr, flush=True)  # PRINT
        if t1 == 'ADDRESS':
#PRINT            print("compute_value - #d", file=sys.stderr, flush=True)  # PRINT
            idx = a[0]
            warray = a[1]
            warray[numarry_values][idx] = arg2[1]
            if arg2[0] == 'NUMBER':
                warray[numarry_value_type][idx] = type_is_number
            elif arg2[0] == 'CHAR':
                warray[numarry_value_type][idx] = type_is_string
            else:
                return [ "ERROR - ADDRESS and arg2 unrecognized '{}'".format(arg2), None]
            # fi
#PRINT            print("compute_value #A- warray={} numarry_macro_arg={} idx={}".format(warray,numarry_macro_arg,idx), file=sys.stderr, flush=True)  # PRINT
#PRINT            print("compute_value #A1 - warray[numarry_macro_arg]={}".format(warray[numarry_macro_arg]), file=sys.stderr, flush=True)  # PRINT
            warray[numarry_macro_arg] = False
#PRINT            print("compute_value - #e", file=sys.stderr, flush=True)  # PRINT
            return arg2
        elif t1 != 'ID':
#PRINT            print("compute_value - #f", file=sys.stderr, flush=True)  # PRINT
            return [ "ERROR - Argument1 is not a variable name arg1='{}'".format(arg1), None ]
        # fi

#PRINT        print("compute_value - #g", file=sys.stderr, flush=True)  # PRINT
        # arrays here.
        maxmaclev = -1
        maxwary = None
        for wary in variable_index.get(a, []):
            if wary[numarry_maclevel] >  maxmaclev:
                maxmaclev = wary[numarry_maclevel]
                maxwary = wary
            # fi
        # rof
#PRINT        print("compute_value - #h", file=sys.stderr, flush=True)  # PRINT
        if maxwary is None:
            print("Assignment to unknown variable '{}', creating it='{}'".format(arg1, a2), file=sys.stderr, flush=True)
            if arg2[0] == 'NUMBER':
                local_arrays.append( [ a, 0, [ ], [ arg2[1] ], [ 0 ], False ] )
                _var_index_add(local_arrays[-1])
            else:                               # Assume CHAR
                local_arrays.append( [ a, 0, [ ], [ arg2[1] ], [ 1 ], False ] )
                _var_index_add(local_arrays[-1])
            # fi
            return arg2
        elif maxwary[numarry_indexes] != []:
#PRINT            print("compute_value - #h1", file=sys.stderr, flush=True)  # PRINT
            return [ "ERROR - compute_value - variable is array '{}'".format(arg1), None ]
        elif arg2[0] == 'NUMBER':
#PRINT            print("compute_value - #i1", file=sys.stderr, flush=True)  # PRINT
            maxwary[numarry_values][0] = float(arg2[1])
#PRINT            print("compute_value - #i2", file=sys.stderr, flush=True)  # PRINT
            maxwary[numarry_value_type][0] = type_is_number
#PRINT            print("compute_value - #i2", file=sys.stderr, flush=True)  # PRINT
        else:                                   # Assume CHAR
#PRINT            print("compute_value - #j", file=sys.stderr, flush=True)  # PRINT
            maxwary[numarry_values][0] = arg2[1]
            maxwary[numarry_value_type][0] = type_is_string
        # fi
#PRINT        print("compute_value - #iB-pre", file=sys.stderr, flush=True)  # PRINT
#PRINT        print("compute_value #B - maxwary={}".format(maxwary), file=sys.stderr, flush=True)  # PRINT
        maxwary[numarry_macro_arg] = False
#PRINT        print("compute_value - #k", file=sys.stderr, flush=True)  # PRINT
        return arg2
    # fi

    a1 = get_value(arg1)
    t1 = a1[0]
    if a1 is None:
        return [ "ERROR - Argument1 is None a1='{}'".format(a1), None ]
    elif t1.startswith('ERROR'):
        return a1
    # fi
    if t1 == 'NUMBER':
        a1 = float(a1[1])
    # fi
    if t2 == 'NUMBER':
        a2 = float(a2[1])
    # fi
    if t1 == 'CHAR':
        a1 = a1[1]
    # fi
    if t2 == 'CHAR':
        a2 = a2[1]
    # fi
    if t1 == 'COMMA':
        a1 = a1[1]
    # fi
    if t2 == 'COMMA':
        a2 = a2[1]
    # fi
    # Might be other types here.
    if t1 not in [ 'NUMBER', 'CHAR', 'COMMA'] or t2 not in [ 'NUMBER', 'CHAR', 'COMMA']:
        return [ 'ERROR - compute_value unexpected types, arg1={} arg2={}'.format(arg1,arg2), None ]
    # fi

    if op == ',':
        if t2 == 'COMMA':
            value = [ 'COMMA', [ a1 ] + a2 ]
        else:
            value = [ 'COMMA', [ a1, a2 ] ]
#--        else:
#--            return [ 'ERROR - compute_value first argument is not a NUMBER nor COMMA, arg1={} arg2={}'.format(arg1,arg2), None ]
        # fi
        return value
#--    elif t2 != 'NUMBER':
#--        return [ "ERROR - Argument2 is not a Number a2='{}'".format(a2), None ]
    # fi

    # Math: +, -, *, /, **
    if op == '+':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if (t1 == 'NUMBER' and t2 == 'NUMBER') or (t1 == 'CHAR' and t2 == 'CHAR'):
            return [ t1, a1 + a2 ]
        elif t1 == 'CHAR' or t2 == 'CHAR':
            return [ t1, str(a1) + str(a2) ]
        # fi
        return [ "ERROR - Arguments are not Numbers or Characters a1='{}' a2='{}'".format(a1,a2), None ]
    # fi
    if op == '-':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #a Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ t1,  a1 - a2 ]
    elif op == '*':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #b Arguments are not Numbers (first:{},'{}') (second:{},'{}')".format(t1,a1,t2,a2), None ]
        # fi
        return [ t1,  a1 * a2 ]
    elif op == '/':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #c Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        if a2 == 0:
            return [ "ERROR - Second argument is zero a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        f = a1 / a2
        return [ t1,  a1 / a2 ]
    elif op == '**':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #d Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ t1,  a1 ** a2 ]
    # Logical: >, >=, <=, <, ==, !=
    elif op == '<=':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #e Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER', -1 if a1 <= a2 else 0 ]
    elif op == '>=':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #f Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER',  -1 if a1 >= a2 else 0 ]
    elif op == '>':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #g Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER',  -1 if a1 > a2 else 0 ]
    elif op == '<':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #h Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER',  -1 if a1 < a2 else 0 ]
    elif op == '==':
        return [ 'NUMBER',  -1 if a1 == a2 else 0 ]
    elif op == '!=':
        return [ 'NUMBER',  -1 if a1 != a2 else 0 ]

    # Bitwise: $cls$, $ars$, $mask$, $union$, $diff$
    elif op == '$mask$':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #i Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER',  int(a1) & int(a2) ]
    elif op == '$union$':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #j Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER',  int(a1) | int(a2) ]
    elif op == '$cls$':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #k Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER',  int(a1) << int(a2) ]
    elif op == '$ars$':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #l Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER',  int(a1) >> int(a2) ]
    elif op == '$diff$':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #m Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER',  int(a1) ^ int(a2) ]             # xor

    # combination: $and$, $or$
    elif op == '$and$':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #n Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER',  -1 if int(a1) == -1 and int(a2) == -1 else 0 ]
    elif op == '$or$':
        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
        if t1 != 'NUMBER' or t2 != 'NUMBER':
            return [ "ERROR - #o Arguments are not Numbers a1='{}' a2='{}'".format(a1,a2), None ]
        # fi
        return [ 'NUMBER',  -1 if int(a1) == -1 or int(a2) == -1 else 0 ]

    elif op == 'TOKEN-LIMITING':
        if t1 != 'CHAR':
            return [ "ERROR - first operand must be a character variable a1='{}' t1={}".format(a1,t1), None ]
        elif t2 not in ['NUMBER', 'COMMA']:
            return [ "ERROR - second operand must be a NUMBER or comma separated values a2='{}' t2={}".format(a2,t2), None ]
        elif t2 == 'COMMA' and len(a2) != 2:
            return [ "ERROR - second operand comma separated values more than 2 a2='{}' t2={}".format(a2,t2), None ]
        elif t2 == 'NUMBER':
#PRINT            print('compute_value #1- calling get_tokens_from_char', file=sys.stderr, flush=True)  # PRINT
            x = get_tokens_from_char(a1, a2, None)
#PRINT            print('compute_value #2- after get_tokens_from_char', file=sys.stderr, flush=True)  # PRINT
        else:
#PRINT            print('compute_value #3- calling get_tokens_from_char', file=sys.stderr, flush=True)  # PRINT
            x = get_tokens_from_char(a1, a2[0], a2[1])
#PRINT            print('compute_value #3- after get_tokens_from_char', file=sys.stderr, flush=True)  # PRINT
        # fi
        return [ 'CHAR',  x ]
    # fi

    return [ "ERROR - unknown operator '{}' '{}' '{}'".format(arg1, op, arg2) , None ]
# End of compute_value

#-----------------------------------------------------------------------------
def quotes_eval(args: list) -> list:
    """Evaluate a quoted string literal, returning it as a CHAR token."""
#PRINT    print("quotes_eval - Entering args='{}'".format(args), file=sys.stderr, flush=True) # PRINT
    if args is None or len(args) != 3:
        return [ 'ERROR - quotes_eval wrong number of arguments, args={}'.format(args), None ]
    # fi
    a0 = args[0]
    a1 = args[1]
    a2 = args[2]
    if isinstance(a0, SymbolDesc) or type(a1) != SymbolDesc or isinstance(a2, SymbolDesc):
        return [ 'ERROR - quotes_eval args={}'.format(args), None ]
    elif a0[0] != 'NUMBER':
        return [ 'ERROR - quotes_eval first argument is not a NUMBER, args={}'.format(args), None ]
    elif a2[0] == 'NUMBER':
#--        value = [ 'COMMA', [ int(a0[1]), int(a2[1]) ] ]
        return [ 'ERROR - quotes_eval second argument is a NUMBER, args={} going COMMA?'.format(args), None ]
    elif a2[0] == 'COMMA':
#--        value = [ 'COMMA', [ int(a0[1]) ] + a2[1] ]
        return [ 'ERROR - quotes_eval second argument is a COMMA, args={} going COMMA?'.format(args), None ]
    # fi
    return [ 'ERROR - quotes_eval unexpected args={}'.format(args), None ]
# End of quotes_eval

#-----------------------------------------------------------------------------
def binary_eval(args: list) -> list:
    """Evaluate a binary expression: look up both operands and apply the operator."""
#PRINT    print('binary_eval - Entering args={}'.format(args), file=sys.stderr, flush=True)    # PRINT
    if args is None or len(args) != 3:
        return [ 'ERROR - binary_eval wrong number of arguments, args={}'.format(args), None ]
    # fi
    a0 = args[0]
    a1 = args[1]
    a2 = args[2]
    if isinstance(a0, SymbolDesc) or type(a1) != SymbolDesc or isinstance(a2, SymbolDesc):
        return [ 'ERROR - binary_eval args={}'.format(args), None ]
    # fi
    value = compute_value(a1.symbol, a0, a2)
    return [ value ]
# End of binary_eval

#-----------------------------------------------------------------------------
def unary_eval(args: list) -> list:
    """Evaluate a unary expression (prefix minus/negation)."""
#PRINT    print('unary_eval - Entering args={}', file=sys.stderr, flush=True)    # PRINT
    if len(args) != 2:
        return [ 'ERROR - unary_eval args={}'.format(args), None ]
    elif isinstance(args[0], SymbolDesc) and type(args[1]) != SymbolDesc:
        op = args[0].symbol
        arg1 = get_value(args[1])
        if arg1 is None:
            return [ "ERROR - Argument2 is None arg1='{}'".format(arg1), None ]
        elif arg1[0].startswith('ERROR'):
            return arg1
        elif arg1[0] != 'NUMBER':
            arg1[0], arg1[1] = fix_to_number(arg1[0], arg1[1])
            if arg1[0] != 'NUMBER':
                return [ "ERROR - Argument2 is not a Number arg1='{}'".format(arg1), None ]
            # fi
        # fi
        if op == '-':
            arg1[1] = 0.0 - arg1[1]
        # fi
        return [ arg1 ]
    elif type(args[0]) != SymbolDesc and isinstance(args[1], SymbolDesc):
        return [ 'ERROR - unary_eval post args={}'.format(args), None ]
    # fi
    return [ 'ERROR - unary_eval args={}'.format(args), None ]
# End of unary_eval

#-----------------------------------------------------------------------------
global presymbols
presymbols = {}
global postsymbols
postsymbols = {}
#-----------------------------------------------------------------------------
def register_presymbol(oper, lprio, rprio, eval=None) -> None:
    """Register a prefix operator with its left/right binding priorities and evaluation function."""
    global presymbols

    if eval is None:
        eval = unary_eval
    # fi
    if isinstance(oper, str):
        presymbols[oper] = SymbolDesc(oper, lprio, rprio, eval)
    else:
        for op in oper:
            presymbols[op] = SymbolDesc(op, lprio, rprio, eval)
        # rof
    # fi
# End of register_presymbol

#-----------------------------------------------------------------------------
def register_postsymbol(oper, lprio, rprio, eval=None) -> None:
    """Register an infix or postfix operator with its binding priorities and evaluation function."""
    global postsymbols

    if eval is None:
        eval = binary_eval
    # fi
    if isinstance(oper, str):
        postsymbols[oper] = SymbolDesc(oper, lprio, rprio, eval)
    else:
        for op in oper:
            postsymbols[op] = SymbolDesc(op, lprio, rprio, eval)
        # rof
    # fi
# End of register_postsymbol

#-----------------------------------------------------------------------------
def id_symbol(id: str) -> SymbolDesc:
    """Create a SymbolDesc for an identifier token."""
#PRINT    print('id_symbol - Entering id={}'.format(id), file=sys.stderr, flush=True)  # PRINT
    return SymbolDesc(id, 99999, 100000, identity_eval)
# End of id_symbol

#-----------------------------------------------------------------------------
def evaluate_handle(args: list) -> list:
    """Dispatch to the appropriate evaluation function for an operator or identifier."""
#PRINT    print('evaluate_handle - Entering args={}'.format(args), file=sys.stderr, flush=True)  # PRINT
    for i in args:
        if isinstance(i, SymbolDesc):
            a = i.eval(args)
            return a
        # fi
    # rof
    raise RuntimeError('Internal error: no eval found in {}'.format(args), file=sys.stderr, flush=True)
# End of evaluate_handle

#-----------------------------------------------------------------------------
global lexer
global cur_token

#-----------------------------------------------------------------------------
def advance() -> None:
    """Advance to the next token in the token stream."""
    global lexer
    global cur_token

#PRINT    print('advance - Entering', file=sys.stderr, flush=True)   # PRINT
    try:
        cur_token = lexer.__next__()                    # [ kind, item ]
#PRINT        print('advance - cur_token={}'.format(cur_token), file=sys.stderr, flush=True) # PRINT
    except StopIteration:
        cur_token = None
    # yrt
# End of advance

#-----------------------------------------------------------------------------
def reset(s: str) -> None:
    """Initialize (or reinitialize) the tokenizer with a new expression string."""
    global lexer

#PRINT    print("reset - Entering s='{}'".format(s), file=sys.stderr, flush=True) # PRINT
    lexer = tokenize(s)
    advance()
# End of reset

#-----------------------------------------------------------------------------
def cur_sym(allow_presymbol: bool) -> SymbolDesc | list:
    """Get the SymbolDesc for the current token, distinguishing prefix vs. infix context."""
    global postsymbols
    global presymbols
    global cur_token
    
#PRINT    print("cur_sym - Entering cur_token='{}' allow_presymbol={}".format(cur_token,allow_presymbol), file=sys.stderr, flush=True)   # PRINT
    if cur_token is None:
        return None
    elif cur_token[0] == 'ID':                              # kind
        return id_symbol(cur_token)
    elif cur_token[0] == 'NUMBER':                          # kind
        return id_symbol(cur_token)
    elif cur_token[0] == 'CHAR':                            # kind
        return id_symbol(cur_token)
    elif cur_token[0] == 'ADDRESS':                         # kind
        return id_symbol(cur_token)
    elif allow_presymbol and cur_token[1] in presymbols:    # item
        return presymbols[cur_token[1]]
    elif cur_token[1] in postsymbols:                       # item
        return postsymbols[cur_token[1]]                    # item
    # fi
    return [ "ERROR - Undefined token '{}'".format(cur_token), None ]
# End of cur_sym

#-----------------------------------------------------------------------------
def parse_to(prio: int) -> list | None | str:
    """Recursive-descent parser using operator-precedence climbing."""
#PRINT    print('parse_to - Entering', file=sys.stderr, flush=True)  # PRINT
    args = []
    while True:
        assert not args or (len(args) == 1 and type(args[0]) != SymbolDesc)
        sym = cur_sym(not args)
        if sym is None or prio >= sym.lprio:
            break
        # fi
#PRINT        print('parse_to - args={}'.format(args), file=sys.stderr, flush=True)  # PRINT
        while True:
#PRINT            print('parse_to - args={} sym={}'.format(args,sym), file=sys.stderr, flush=True)  # PRINT
            args.append(sym)
            advance()
            curprio = sym.rprio
            next = parse_to(curprio)
            if next is not None:
#PRINT                print('parse_to - args={} next={}'.format(args,next), file=sys.stderr, flush=True)  # PRINT
                args.append(next)
            # fi
            sym = cur_sym(next is None)
            if sym is None or curprio != sym.lprio:
                break
            # fi
        # elihw

#PRINT        print('parse_to - calling evaluate_handle', file=sys.stderr, flush=True)  # PRINT
        args = evaluate_handle(args)
        if args and len(args) > 1 and args[0].startswith('ERROR'):
            return args
        # fi
    # elihw

    if len(args) == 1:
        a = get_value(args[0])
        if a is None:
            return [ "ERROR - value is None a='{}'".format(a), None ]
        elif a[0].startswith('ERROR'):
            return a
        elif a[0] == 'COMMA':
            return a
        elif a[0] == 'CHAR':
            return a
        elif a[0] != 'NUMBER':
            return [ "ERROR - value is not a Number a='{}'".format(a), None ]
        # fi
        a[1] = float(a[1])
        return a
    elif not args:
        return None
    # fi
    return "ERROR - parse_to runs off the end of routine '{}'".format(args)
# End of parse_to

#-----------------------------------------------------------------------------
def parse(s: str) -> list:
    """Parse and evaluate a complete expression string, returning [type, value]."""
    global cur_token

#PRINT    print("parse - Entering s='{}'".format(s), file=sys.stderr, flush=True) # PRINT
    reset(s)
    try:                                # NOTDONEYET - move to only around specific OOR plaes.
        res = parse_to(0)
    except Exception:
        return [ None, 'error parsing' ]
    # yrt
    if cur_token is not None:
        return [ "ERROR - remaining input res='{}' cur_token='{}'".format(res, cur_token), None ]
    # fi
    return res
# End of parse

#-----------------------------------------------------------------------------
# Returns character string of tokens requested.
# strng = The string to get tokens from.
# start = token number to start at (1 = first).
# lth   = number of tokens to get. Note: delimiters except for space/comma count as a token.
#       = None means to go "start" to end of string.

def get_tokens_from_char(strng: str, start: int, lth: int) -> str:
    """Extract a substring of space-separated tokens from a character-string variable."""
    start = int(round(start)) - 1
#PRINT    print("get_tokens_from_char - Entering strng={} start={} lth={}".format(strng,start,lth), file=sys.stderr, flush=True)  # PRINT
    if strng is None or strng == '' or start < 0:
        return ''
    # fi
    a = [i for i in _RE_CHAR_SPLIT.split(strng) if i]
#PRINT    print("get_tokens_from_char - a={}".format(a), file=sys.stderr, flush=True)  # PRINT
    # Get rid of only spaces.
    new = []
    for i in a:
        if i in ' ':
            continue
        # fi
#PRINT        print("get_tokens_from_char - #0a", file=sys.stderr, flush=True)  # PRINT
#--        x = i.replace(' ', '')
#--        x = x.replace("\t", '')
        x = _RE_WHITESPACE.sub(r' ', i)
#PRINT        print("get_tokens_from_char - #0b", file=sys.stderr, flush=True)  # PRINT
        new.append(x)
    # rof
#PRINT    print("get_tokens_from_char - #1", file=sys.stderr, flush=True)  # PRINT
    if len(new) < start:
        return ''
    # fi
    if lth is None:
        lth = len(new)
    else:
        lth = int(round(start + lth))
    # fi
#PRINT    print("get_tokens_from_char - #2", file=sys.stderr, flush=True)  # PRINT
    x = new[start : lth]
    parts = []
    prev = ''
#PRINT    print("get_tokens_from_char - #3", file=sys.stderr, flush=True)  # PRINT
    for y in x:
        if y == '':
            parts.append(' ')
        elif _RE_WORD.match(prev) and _RE_WORD.match(y):
            parts.append(' ')
            parts.append(y)
        else:
            parts.append(y)
        # fi
        prev = y
    # rof
#PRINT    print("get_tokens_from_char - #4", file=sys.stderr, flush=True)  # PRINT
#PRINT    print("get_tokens_from_char - return {}".format(w), file=sys.stderr, flush=True)  # PRINT
    return ''.join(parts)
# End of get_tokens_from_char

#-----------------------------------------------------------------------------
def result_functions(arg1: list, arg2: list) -> list:
    """Evaluate built-in function calls and array-subscript operations."""
    global functions
    global arrays
    global local_arrays

#PRINT    print('result_functions - #1 arg1={} arg2={}'.format(arg1,arg2), file=sys.stderr, flush=True)  # PRINT
    if arg1[0] != 'ID':
        # 'NUMBER' -- implied multiply
        # Do implied multiply
        a1 = get_value(arg1)
        if a1 is None:
            return [ "ERROR - value is None a1='{}'".format(a1), None ]
        elif a1[0].startswith('ERROR'):
            return a1
        elif a1[0] != 'NUMBER':
            return [ "ERROR - value is not a Number a1='{}' arg2={} -- implied multiply #1 result_functions".format(a1,arg2), None ]
        # fi
        a2 = get_value(arg2)
        if a2 is None:
            return [ "ERROR - value is None a2='{}'".format(a2), None ]
        elif a2[0].startswith('ERROR'):
            return a2
        elif a2[0] != 'NUMBER':
            return [ "ERROR #1- value is not a Number a2='{}'".format(a2), None ]
        # fi
        a = [ 'NUMBER',  a1[1] * a2[1] ]
        return a
    # fi

    # -- ID --
    if arg1[1] in functions:
        fu = functions[arg1[1]]
        wh = fu[0]
        ar = fu[1]
#PRINT        print(f'result_functions - #2 ar={ar} arg2={arg2}', file=sys.stderr, flush=True)  # PRINT
        flagit = False
        for aaa in ar:
#PRINT            print(f'result_functions - #3 aaa={aaa}', file=sys.stderr, flush=True)  # PRINT
            my_regex = aaa + r'$'
            if re.match(my_regex, arg2[0]):
#PRINT                print(f'result_functions - #10 my_regex={my_regex} aaa={aaa}', file=sys.stderr, flush=True)  # PRINT
                flagit = True
                break
            # fi
        # rof
        if not flagit:
            return [ "ERROR - function '{}' called with wrong argument type {} vs {}".format(arg1[1], arg2[0], ar), None ]
        # fi
        try:
#PRINT            print(f'result_functions - #11 arg2={arg2}', file=sys.stderr, flush=True)  # PRINT
            a = wh(arg2)
#PRINT            print(f'result_functions - #12 a={a}', file=sys.stderr, flush=True)  # PRINT
            return a
        except Exception:
            return [ "ERROR - performing function '{}' with argument '{}'".format(arg1, arg2), None ]
        # yrt
    # fi

    if arg2[0] in ['NUMBER', 'COMMA', 'CHAR']:
        maxmaclev = -1
        maxwary = None
        for wary in arrays + local_arrays:
            if arg1[1] == wary[numarry_name]:
                if wary[numarry_maclevel] > maxmaclev:
                    maxmaclev = wary[numarry_maclevel]
                    maxwary = wary
                # fi
            # fi
        # rof
        if maxwary is not None:
            arg = [ None, None]
            l = len(maxwary[numarry_indexes])
            if l == 0:                      # Do an implied multiply.
                pass
            elif l == 1:
                d = int(arg2[1])
                if arg2[0] != 'NUMBER':
                    return [ "ERROR - result_functions - variable needs 1 dimension to array '{}' not {}".format(arg1, l) ]
                elif d < 1 or d > maxwary[numarry_indexes][0]:
                    return [ 'ERROR - result_functions - variable {} 1st dimension {} not in range 1 thru {}'.format(arg1, d, maxwary[numarry_indexes][0]) ]
#--                elif maxwary[numarry_value_type][d-1] is None:
#--                    return [ 'ERROR - result_functions - variable {}({}) is not set'.format(arg1, d) ]
                # fi
                arg = [ 'ADDRESS', [ d - 1, maxwary] ]
                return arg
            else:
                if arg2[0] != 'COMMA':
                    return [ 'ERROR - result_functions - variable {} needs {} dimensions for array'.format(arg1, l) ]
                # fi
                d = len(arg2[1])
                if l != d:
                    return [ "ERROR - result_functions - variable {} needs {} dimensions to array '{}' not {}".format(arg1, l, d) ]
                # fi

                d = int(round(float(arg2[1][0])))
                x = maxwary[numarry_indexes][0]     # Max dimension of this..
                if d < 1 or d > x:
                    return [ 'ERROR - result_functions - variable {} dimension#1 {} not in range 1 thru {}'.format(arg1, d, x) ]
                # fi
                h = d - 1                           # array index into numarry_values & numarry_value_type
                mult = x
                for a in range(1, l):
                    d = int(round(float(arg2[1][a])))
                    x = maxwary[numarry_indexes][a]
                    if d < 1 or d > x:
                        return [ 'ERROR - result_functions - variable {} dimension#{} {} not in range 1 thru {}'.format(arg1, a+1, d, x) ]
                    # fi
                    h = h + (d-1) * mult
                    mult = mult * x
                # rof

#--                if maxwary[numarry_value_type][h] is None:
#--                    return [ 'ERROR - result_functions - array {} ({}) is not set yet'.format(arg1,arg2[1]), None ]
#--                # fi
                arg = [ 'ADDRESS', [ h, maxwary] ]
                return arg
            # fi
        # fi
    # fi

    # See if can get_value(arg1[0]) -- if can, then do implied multiply.
    a1 = get_value(arg1)
    if a1 is None:
        return [ "ERROR - value is None a1='{}'".format(a1), None ]
    elif a1[0].startswith('ERROR'):
        return a1
    elif a1[0] == 'CHAR':
        a2 = get_value(arg2)
        if a2[0] == 'NUMBER':               # Go from token number a2[1] to end of string.
            if not isinstance(a2[1], int) and not isinstance(a2[1], float): 
                return [ "ERROR - character token fetching needs first argument as integer, but this has {}".format(type(a2[1][0])), None ]
            # fi
#PRINT            print('result_functions #1- calling get_tokens_from_char', file=sys.stderr, flush=True)  # PRINT
            x = get_tokens_from_char(a1[1],a2[1],None)
#PRINT            print('result_functions #2- after get_tokens_from_char', file=sys.stderr, flush=True)  # PRINT
        elif a2[0] == 'COMMA':              # Go from token number a2[1][0] for length a2[1][1]
            if len(a2[1]) != 2:
                return [ "ERROR - character token fetching needs 1 or 2 arguments, but this has {}".format(len(a2[1])), None ]
            # fi
            if not isinstance(a2[1][0], int) and not isinstance(a2[1][0], float): 
                return [ "ERROR - character token fetching needs first argument as integer, but this has {}".format(type(a2[1][0])), None ]
            elif not isinstance(a2[1][1], int) and not isinstance(a2[1][1], float): 
                return [ "ERROR - character token fetching needs second argument as integer, but this has {}".format(type(a2[1][0])), None ]
            # fi
#PRINT            print('result_functions #3- calling get_tokens_from_char', file=sys.stderr, flush=True)  # PRINT
            x = get_tokens_from_char(a1[1], a2[1][0], a2[1][1])
#PRINT            print('result_functions #4- after get_tokens_from_char', file=sys.stderr, flush=True)  # PRINT
        else:
            return [ "ERROR - character token fetching has bad ({},{}) type ({}) for token indexes".format(type(a2[1][0]), type(a2[1][1]), a2[0]), None ]
        # fi
        return [ 'CHAR', x ]
    elif a1[0] != 'NUMBER':
        return [ "ERROR - value is not a Number a1='{}' arg2={} -- implied multiply #2 result_functions".format(a1,arg2), None ]
    # fi
    a2 = get_value(arg2)
    if a2 is None:
        return [ "ERROR - value is None a2='{}'".format(a2), None ]
    elif a2[0].startswith('ERROR'):
        return a2
    elif a2[0] != 'NUMBER':
        return [ "ERROR #2- value is not a Number a2='{}'".format(a2), None ]
    elif a1 and a2:
        a = [ 'NUMBER', a1[1] * a2[1] ]
        return a
    # fi
    return [[ "ERROR - Fetching from unknown function '{}' '{}'".format(arg1, arg2) , None ]]
# End of result_functions

#-----------------------------------------------------------------------------
def common_grouping_eval(args: list, txt: str, open_char: str, close_char: str) -> list:
    """Common handler for subexpressions inside (), [], or {}."""
#PRINT    print("common_grouping_eval - len(args)={} args='{}'".format(len(args),args), file=sys.stderr, flush=True)  # PRINT
    if len(args) == 3:
        r = args[0]
        s = args[1]
        t = args[2]
        if (isinstance(r, SymbolDesc) and args[0].symbol == open_char
            and type(s) != SymbolDesc
            and isinstance(t, SymbolDesc) and t.symbol == close_char):
            return [ s ]
        # fi
    # fi
    if len(args) == 4:
        r = args[0]
        s = args[1]
        t = args[2]
        u = args[3]
        if (type(r) != SymbolDesc
            and isinstance(s, SymbolDesc) and s.symbol == open_char
            and type(t) != SymbolDesc
            and isinstance(u, SymbolDesc) and u.symbol == close_char):
#PRINT            print("common_grouping_eval - #4 - r='{}' t='{}'".format(r,t), file=sys.stderr, flush=True)  # PRINT
            a = result_functions(r, t)
            return [ a ]
        #fi
        if (isinstance(r, SymbolDesc) and r.symbol == open_char
            and type(s) != SymbolDesc
            and isinstance(t, SymbolDesc) and t.symbol == close_char
            and type(u) != SymbolDesc):
            # Do implied multiply
            a1 = get_value(s)
            if a1 is None:
                return [ "ERROR - value is None a1='{}'".format(a1), None ]
            elif a1[0].startswith('ERROR'):
                return a1
            elif a1[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a1='{}' u={} implied multiply #3 -- common_grouping_eval".format(a1,u), None ]
            # fi
            a2 = get_value(u)
            if a2 is None:
                return [ "ERROR - value is None a2='{}'".format(a2), None ]
            elif a2[0].startswith('ERROR'):
                return a2
            elif a2[0] != 'NUMBER':
                return [ "ERROR #3- value is not a Number a2='{}'".format(a2), None ]
            # fi
            return [[ 'NUMBER',  a1[1] * a2[1] ]]
        #fi
    # fi
    if len(args) == 6:
        r = args[0]
        s = args[1]
        t = args[2]
        u = args[3]
        v = args[4]
        w = args[5]
# NOTDONEYET = {}
        if (isinstance(r, SymbolDesc) and (r.symbol in ['(','[','{'])
            and type(s) != SymbolDesc
# NOTDONEYET = {}
            and isinstance(t, SymbolDesc) and (t.symbol in [')',']','}'])
# NOTDONEYET = {}
            and isinstance(u, SymbolDesc) and (u.symbol in ['(','[','{'])
            and type(v) != SymbolDesc
# NOTDONEYET = {}
            and isinstance(w, SymbolDesc) and (w.symbol in [')',']','}'])):
            # Do implied multiply
            a1 = get_value(s)
            if a1 is None:
                return [ "ERROR - value is None a1='{}'".format(a1), None ]
            elif a1[0].startswith('ERROR'):
                return a1
            elif a1[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a1='{}' v={} implied multiply #4 -- common_grouping_eval".format(a1,v), None ]
            # fi
            a2 = get_value(v)
            if a2 is None:
                return [ "ERROR - value is None a2='{}'".format(a2), None ]
            elif a2[0].startswith('ERROR'):
                return a2
            elif a2[0] != 'NUMBER':
                return [ "ERROR #4- value is not a Number a2='{}'".format(a2), None ]
            # fi
            return [[ 'NUMBER',  a1[1] * a2[1] ]]
        #fi
    # fi
    if len(args) == 7:
        r = args[0]
        s = args[1]
        t = args[2]
        u = args[3]
        v = args[4]
        w = args[5]
        x = args[6]
        if (type(r) != SymbolDesc                           # ID m
# NOTDONEYET = {}
            and isinstance(s, SymbolDesc) and (s.symbol in ['(','[','{'])
            and type(t) != SymbolDesc                       # NUMBER abc
# NOTDONEYET = {}
            and isinstance(u, SymbolDesc) and (u.symbol in [')',']','}'])
# NOTDONEYET = {}
            and isinstance(v, SymbolDesc) and (v.symbol in ['(','[','{'])
            and type(w) != SymbolDesc                       # NUMBER def
# NOTDONEYET = {}
            and isinstance(x, SymbolDesc) and (x.symbol in [')',']','}'])):
#PRINT            print("common_grouping_eval - #7 - r='{}' t='{}'".format(r,t), file=sys.stderr, flush=True)  # PRINT
            a1 = result_functions(r, t)
            if isinstance(a1, list) and a1:
                a1 = a1[0]
            elif a1 is None:
                return [ "ERROR - value is None a1='{}'".format(a1), None ]
            elif a1[0].startswith('ERROR'):
                return a1
            elif a1[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a1='{}' w={} implied multiply #5 -- common_grouping_eval".format(a1,w), None ]
            # fi
            a2 = get_value(w)
            if a2 is None:
                return [ "ERROR - value is None a2='{}'".format(a2), None ]
            elif a2[0].startswith('ERROR'):
                return a2
            elif a2[0] != 'NUMBER':
                return [ "ERROR #5- value is not a Number a2='{}'".format(a2), None ]
            # fi
            a = [[ 'NUMBER',  a1[1] * a2[1] ]]
            return a
        #fi
    # fi
    return [ "ERROR - open_{}_eval args='{}'".format(txt, args), None ]
# End of common_grouping_eval

#-----------------------------------------------------------------------------
def open_paren_eval(args: list) -> list:
    """Handle an opening parenthesis in an expression."""
#PRINT    print("open_paren_eval - args='{}'".format(args), file=sys.stderr, flush=True)  # PRINT
    return common_grouping_eval(args, 'paren', '(', ')')
# End of open_paren_eval

#-----------------------------------------------------------------------------
def close_paren_eval(args: list) -> list:
    """Handle a closing parenthesis in an expression."""
#PRINT    print("close_paren_eval - args='{}'".format(args), file=sys.stderr, flush=True) # PRINT
    return [ "ERROR - close_paren_eval args='{}'".format(args), None ]
# End of close_paren_eval

#-----------------------------------------------------------------------------
def open_bracket_eval(args: list) -> list:
    """Handle an opening bracket in an expression."""
#PRINT    print("open_bracket_eval - args='{}'".format(args), file=sys.stderr, flush=True)    # PRINT
    return common_grouping_eval(args, 'bracket', '[', ']')
# End of open_bracket_eval

#-----------------------------------------------------------------------------
def close_bracket_eval(args: list) -> list:
    """Handle a closing bracket in an expression."""
#PRINT    print("close_bracket_eval - args='{}'".format(args), file=sys.stderr, flush=True)   # PRINT
    return [ "ERROR - close_bracket_eval args='{}'".format(args), None ]
# End of close_bracket_eval

#-----------------------------------------------------------------------------
def open_brace_eval(args: list) -> list:
    """Handle an opening brace in an expression."""
#PRINT    print("open_brace_eval - args='{}'".format(args), file=sys.stderr, flush=True)  # PRINT
# NOTDONEYET = {}
    return common_grouping_eval(args, 'brace', '{', '}')
# End of open_brace_eval

#-----------------------------------------------------------------------------
def close_brace_eval(args: list) -> list:
    """Handle a closing brace in an expression."""
#PRINT    print("close_brace_eval - args='{}'".format(args), file=sys.stderr, flush=True) # PRINT
    return [ "ERROR - close_brace_eval args='{}'".format(args), None ]
# End of close_brace_eval

#-----------------------------------------------------------------------------
def f_m(arg: list) -> list:
    """Access one of the temporary variables m1-m100 by numeric index."""
#PRINT    print("f_m - arg='{}'".format(arg), file=sys.stderr, flush=True)    # PRINT
    try:
        a = float(arg[1])
        a = int(a)
    except (ValueError, TypeError):
        return [ "ERROR - argument to array m is not an integer '{}'".format(arg), None ]
    # yrt
    if a < 1 or a > 100:
        return [ "ERROR - argument to array m is out of range 1 to 100 '{}'".format(a), None ]
    # fi
    return [ 'ID', 'm' + str(a) ]
# End of f_m

#-----------------------------------------------------------------------------
def f_ceil(arg: list) -> list:
    """Return the ceiling (smallest integer >= value) of a number."""
    a = float(arg[1])
    a = math.ceil(a)
    return [ 'NUMBER', a ]
# End of f_ceil

#-----------------------------------------------------------------------------
def f_floor(arg: list) -> list:
    """Return the floor (largest integer <= value) of a number."""
    a = float(arg[1])
    a = math.floor(a)
    return [ 'NUMBER', a ]
# End of f_floor

#-----------------------------------------------------------------------------
def f_freq(arg: list) -> list:
    """Convert a MIDI note number to its frequency in Hz."""
    a = float(arg[1])
    a = 968000.0 / a
    return [ 'NUMBER', a ]
# End of f_freq

#-----------------------------------------------------------------------------
def f_nearest(arg: list) -> list:
    """Find the nearest MIDI note number for a given frequency in Hz."""
    a = float(arg[1])
    a = int(round(math.log(a / 27.5) * (12.0 / 0.693147)))
    return [ 'NUMBER', a ]
# End of f_nearest

#-----------------------------------------------------------------------------
def f_abs(arg: list) -> list:
    """Return the absolute value of a number."""
    a = float(arg[1])
    a = math.fabs(a)
    return [ 'NUMBER', a ]
# End of f_abs

#-----------------------------------------------------------------------------
def f_arctan(arg: list) -> list:
    """Compute the arctangent (inverse tangent) in radians."""
    a = float(arg[1])
    a = math.atan(a)
    return [ 'NUMBER', a ]
# End of f_arctan

#-----------------------------------------------------------------------------
def f_cos(arg: list) -> list:
    """Compute the cosine of an angle in radians."""
    a = float(arg[1])
    a = math.cos(a)
    return [ 'NUMBER', a ]
# End of f_cos

#-----------------------------------------------------------------------------
def f_exp(arg: list) -> list:
    """Compute e raised to the given power."""
    a = float(arg[1])
    a = math.exp(a)
    return [ 'NUMBER', a ]
# End of f_exp

#-----------------------------------------------------------------------------
def f_frac(arg: list) -> list:
    """Return the fractional part of a number (value minus its integer part)."""
    a = float(arg[1])
    a = a - int(a)
    return [ 'NUMBER', a ]
# End of f_frac

#-----------------------------------------------------------------------------
def f_float(arg: list) -> list:
    """Convert a value to a floating-point number."""
    a = float(arg[1])
    return [ 'NUMBER', a ]
# End of f_float

#-----------------------------------------------------------------------------
def f_int(arg: list) -> list:
    """Convert a value to an integer by truncation."""
    a = float(arg[1])
    a = int(a)
    return [ 'NUMBER', a ]
# End of f_int

#-----------------------------------------------------------------------------
def f_in(arg: list) -> list:
    """Check if a value is within a range: returns 1 if min <= value <= max, else 0."""
#PRINT    print("f_in - arg='{}'".format(arg), file=sys.stderr, flush=True)   # PRINT
    val = arg[1]
    if len(val) != 3:
        return [ "ERROR - in() needs two arguments, not: {}'".format(val), None ]
    # fi
    # check if types of 3 arguments are float/int. 
    n = 1
    for x in val:
        if not isinstance(x, float) and not isinstance(x, int):
            return [ "ERROR - in() needs arguments to be integer or float. #{} {} is type {}".format(n, x, type(x)), None ]
        # fi
        n = n + 1
    # rof
    val1 = val[0]
    val2 = val[1]
    val3 = val[2]
    if val3 < val1:
        return [ "ERROR - in() needs third argument ({}) greater than or equal first ({})'".format(val3,val1), None ]
    elif val1 <= val2 and val2 <= val3:
        value = -1
    else:
        value = 0
    # fi
    return [ 'NUMBER', value ]
# End of f_in

#-----------------------------------------------------------------------------
def f_log(arg: list) -> list:
    """Compute the base-10 logarithm of a number."""
    a = float(arg[1])
    try:
        a = math.log(a, 10)
    except (ValueError, TypeError):
        a = 0
    # ytr
    return [ 'NUMBER', a ]
# End of f_log

#-----------------------------------------------------------------------------
def f_ln(arg: list) -> list:
    """Compute the natural (base-e) logarithm of a number."""
    a = float(arg[1])
    a = math.log(a)
    return [ 'NUMBER', a ]
# End of f_ln

#-----------------------------------------------------------------------------
def f_round(arg: list) -> list:
    """Round a number to the nearest integer."""
    a = float(arg[1])
    a = int(round(a))
    return [ 'NUMBER', a ]
# End of f_round

#-----------------------------------------------------------------------------
def f_sign(arg: list) -> list:
    """Return the sign of a number: -1, 0, or 1."""
    a = float(arg[1])
    if a < 0:
        return [ 'NUMBER', -1 ]
    elif a > 0:
        return [ 'NUMBER', 1 ]
    # fi
    return [ 'NUMBER', 0 ]
# End of f_sign

#-----------------------------------------------------------------------------
def f_sin(arg: list) -> list:
    """Compute the sine of an angle in radians."""
    a = float(arg[1])
    a = math.sin(a)
    return [ 'NUMBER', a ]
# End of f_sin

#-----------------------------------------------------------------------------
def f_sqrt(arg: list) -> list:
    """Compute the square root of a number."""
    a = float(arg[1])
    a = math.sqrt(a)
    return [ 'NUMBER', a ]
# End of f_sqrt

#-----------------------------------------------------------------------------
# Invert logical expression (-1 = true, 0 = false). Make anything non-zero be true.
def f_not(arg: list) -> list:
    """Logical negation: return -1 if value is 0, else return 0."""
    a = float(arg[1])
    if a < 0 or a > 0:
        a = 0
    else:
        a = -1
    # fi
    return [ 'NUMBER', a ]
# End of f_not

#-----------------------------------------------------------------------------
def f_defined(arg: list) -> list:
    """Check if a variable is defined and has been assigned a value."""
    which = arg[0]
    val = arg[1]
    if which in [ 'NUMBER', 'CHAR', 'COMMA']:
        return ['NUMBER', -1.0]                 # True
    elif len(which) > 8 and which[0:8] == 'ERROR - ':
        return ['NUMBER', 0.0]                  # False
    else:
        print(f'f_defined - UNKNOWN TYPE={which} len(which)={len(which)} which[0:9]={which[0:9]}', file=sys.stderr, flush=True)
        return ['NUMBER', 0.0]                  # False
    # fi
# End of f_defined

#-----------------------------------------------------------------------------
def f_print(arg: list) -> list:
    """Print a value to stderr for debugging, then return it."""
    which = arg[0]
    val = arg[1]
    if which in [ 'NUMBER', 'CHAR']:
        print(val)
        return arg
    elif which == 'COMMA':
        a = ','.join(str(i) for i in val)
        print(a)
        return arg
    else:
        print('PRINT: UNKNOWN TYPE={}'.format(arg), file=sys.stderr, flush=True)
        return arg
    # fi
# End of f_print

#-----------------------------------------------------------------------------
def f_max(arg: list) -> list:
    """Return the larger of two values."""
    val = arg[1]
    if len(val) != 2:
        return [ "ERROR - max needs two arguments, not: {}'".format(val), None ]
    # fi
    n = 1
    for x in val:
        if not isinstance(x, float) and not isinstance(x, int):
            return [ "ERROR - max() needs arguments to be integer or float. #{} {} is type {}".format(n, x, type(x)), None ]
        # fi
        n = n + 1
    # rof
    val1 = arg[1][0]
    val2 = arg[1][1]
    value = val1 if val1 > val2 else val2
    return [ 'NUMBER', value ]
# End of f_max

#-----------------------------------------------------------------------------
def f_min(arg: list) -> list:
    """Return the smaller of two values."""
    val = arg[1]
    if len(val) != 2:
        return [ "ERROR - min needs two arguments, not: {}'".format(val), None ]
    # fi
    n = 1
    for x in val:
        if not isinstance(x, float) and not isinstance(x, int):
            return [ "ERROR - min() needs arguments to be integer or float. #{} {} is type {}".format(n, x, type(x)), None ]
        # fi
        n = n + 1
    # rof
    val1 = arg[1][0]
    val2 = arg[1][1]
    value = val1 if val1 <= val2 else val2
    return [ 'NUMBER', value ]
# End of f_min

#-----------------------------------------------------------------------------
def f_mod(arg: list) -> list:
    """Compute the modulo (remainder) of two numbers."""
#PRINT    print("f_mod - arg='{}'".format(arg), file=sys.stderr, flush=True)  # PRINT
    val = arg[1]
    if len(val) != 2:
        return [ "ERROR - mod needs two arguments, not: {}'".format(val), None ]
    # fi
    n = 1
    for x in val:
        if not isinstance(x, float) and not isinstance(x, int):
            return [ "ERROR - mod() needs arguments to be integer or float. #{} {} is type {}".format(n, x, type(x)), None ]
        # fi
        n = n + 1
    # rof
    val1 = arg[1][0]
    val2 = arg[1][1]
    if val2 == 0:
        return [ "ERROR - mod needs second argument positive, not: {}'".format(val2), None ]
    # fi
    if isinstance(val1, int) and isinstance(val2, int):
        value = val1 % val2
    else:
        value = math.fmod(float(val1), float(val2))
    # fi
    return [ 'NUMBER', value ]
# End of f_mod

#-----------------------------------------------------------------------------
def f_clock(arg: list) -> list:
    """Get the current date/time formatted as a string component (0=year, 1=month, etc.)."""
    now = datetime.now()
    it = arg[1]
    a = [now.year, now.month, now.day, now.hour, now.minute, now.second, now.microsecond]
    if it <= 0.0 or it > 7.0:
        return [ 'COMMA', a]
    # fi
    return [ 'NUMBER', a[int(it)-1] ]
# End of f_clock

#-----------------------------------------------------------------------------
global functions
functions = {
#    NAME       Function        array of argument-types
    'm':        [ f_m,          ['NUMBER' ]],           # Array of m1, m2, m3, ...
#...............................................................................
# Math functions.
    'abs':      [ f_abs,        ['NUMBER']],
    'arctan':   [ f_arctan,     ['NUMBER']],
    'ceil':     [ f_ceil,       ['NUMBER']],
    'cos':      [ f_cos,        ['NUMBER']],
    'exp':      [ f_exp,        ['NUMBER']],
    'float':    [ f_float,      ['NUMBER', 'CHAR']],
    'floor':    [ f_floor,      ['NUMBER']],
    'frac':     [ f_frac,       ['NUMBER']],
    'int':      [ f_int,        ['NUMBER', 'CHAR']],
    'in':       [ f_in,         ['COMMA']],
    'log':      [ f_log,        ['NUMBER']],
    'ln':       [ f_ln,         ['NUMBER']],
    'max':      [ f_max,        ['COMMA']],
    'min':      [ f_min,        ['COMMA']],
    'mod':      [ f_mod,        ['COMMA']],
    'not':      [ f_not,        ['NUMBER']],
    'round':    [ f_round,      ['NUMBER']],
    'sign':     [ f_sign,       ['NUMBER']],
    'sin':      [ f_sin,        ['NUMBER']],
    'sqrt':     [ f_sqrt,       ['NUMBER']],
    'defined':  [ f_defined,    ['NUMBER', 'CHAR', 'COMMA', 'ERROR - .*']],
    'clock':    [ f_clock,      ['NUMBER']],     # NOTDONEYET
#...............................................................................
# Old musicomp functions.
    'freq':     [ f_freq,       ['NUMBER']],
    'nearest':  [ f_nearest,    ['NUMBER']],
#...............................................................................
    'print':    [ f_print,      ['NUMBER','CHAR','COMMA']],
    }

#-----------------------------------------------------------------------------
#               Name of variable
#                       Macro-level
#                          Zero dimension
#                               Value
#                                             Number (not character string)
arrays.append( [ 'pi',  0, [ ], [ math.pi ],  [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'e',   0, [ ], [ math.e ],   [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'tau', 0, [ ], [ math.tau ], [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm1',  0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm2',  0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm3',  0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm4',  0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm5',  0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm6',  0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm7',  0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm8',  0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm9',  0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm10', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm11', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm12', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm13', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm14', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm15', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm16', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm17', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm18', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm19', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm20', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm21', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm22', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm23', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm24', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm25', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm26', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm27', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm28', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm29', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm30', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm31', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm32', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm33', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm34', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm35', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm36', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm37', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm38', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm39', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm40', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm41', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm42', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm43', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm44', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm45', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm46', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm47', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm48', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm49', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm50', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm51', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm52', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm53', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm54', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm55', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm56', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm57', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm58', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm59', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm60', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm61', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm62', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm63', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm64', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm65', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm66', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm67', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm68', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm69', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm70', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm71', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm72', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm73', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm74', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm75', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm76', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm77', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm78', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm79', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm80', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm81', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm82', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm83', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm84', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm85', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm86', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm87', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm88', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm89', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm90', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm91', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm92', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm93', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm94', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm95', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm96', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm97', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm98', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
arrays.append( [ 'm99', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])

arrays.append( [ 'm100', 0, [ ], [ 0 ],        [ 0 ] , False ] )
_var_index_add(arrays[-1])
#-----------------------------------------------------------------------------
def cexp_parser() -> None:
    """Initialize the expression parser: register all operators and functions with their precedence levels."""
    #                    oper,                         lprio, rprio, eval
    # Assignment.
    register_postsymbol('=',                               5,   4)                      # r to l
    register_presymbol('"' + "'",                          6,   7, quotes_eval)         # quotes to eoln
    # Special, dimensions, functions.
    register_postsymbol(',',                              11,  10)                      # l to r
    # Special, character string token limiting.
    register_postsymbol('TOKEN-LIMITING',                 21,  22)                      # l to r
    # Combination.
    register_postsymbol(['$or$', '$and$'],                30,  31)                      # l to r
    # Logical.
    register_postsymbol(['>','>=','<','<=','==','!='],    40,  41)                      # l to r
    # Bitwise.
    register_postsymbol(['$union$', '$mask$', '$diff$',
                         '$cls$', '$ars$'],               50,  51)                      # l to r
    # Math.
    register_postsymbol(['+', '-'],                       60,  61)                      # l to r
    register_postsymbol('/',                              70,  71)                      # l to r
    register_postsymbol('*',                              80,  81)                      # l to r
    register_postsymbol('**',                             91,  90)                      # r to l
    # Leading + or -.
    register_presymbol(['+', '-'],                       101, 100, unary_eval)          # r to l
    # Parenthesis.
    register_postsymbol('(',                            1000,   1, open_paren_eval)     # l to r
    register_postsymbol(')',                               1, 1000, close_paren_eval)   # r to l
    register_postsymbol('[',                            1000,   1, open_bracket_eval)   # l to r
    register_postsymbol(']',                               1, 1000, close_bracket_eval) # r to l
# NOTDONEYET = {}
    register_postsymbol('{',                            1000,   1, open_brace_eval)
# NOTDONEYET = {}
    register_postsymbol('}',                               1, 1000, close_brace_eval)   # r to l
    return
# End of cexp_parser

#-----------------------------------------------------------------------------
# Get a line. Put it in 'line' and return it.
def get_line() -> str:
    """Read a line of input from stdin or an interactive readline prompt."""
    global linecount

    while True:
        try:
            if sys.stdin.isatty():
                if sys.platform == 'darwin':
                    input('input> ')
                    line = readline.get_line_buffer()
                else:
                    line = input('input> ')
                # fi
            else:
                line = sys.stdin.readline()
            # fi
            if line is None or line == '':
                sys.exit(0)
            # fi
            linecount = linecount + 1
            if line:
                # delete anything from $$ onwards.
                line = _RE_DOLLAR_COMMENT.sub(r'', line)
                # ignore leading and trailing spaces.
                line = line.strip()
                return line
            # fi
        except EOFError:
            print('Read gave EOF', file=sys.stderr)
        except SystemExit:
            print('Read gave system exit', file=sys.stderr)
        except KeyboardInterrupt:
            print('Read got keyboard interrupt', file=sys.stderr)
        except Exception:
            print('Read got a processing error', file=sys.stderr)
            print('   ', sys.exc_info()[0], sys.exc_info, file=sys.stderr, flush=True)
        # yrt
        break
    # elihw
    sys.exit(0)
# End of get_line

#-----------------------------------------------------------------------------
# Parse and process line.
def process_line(t: str, line: str) -> None:
    """Parse and evaluate a single expression line, printing the result."""
#PRINT    print("process_line - Entering line='{}'".format(line), file=sys.stderr, flush=True)    # PRINT
    wline = line.strip()                 # Ignore leading and trailing whitespace.
    if wline == 'quit' or wline == 'exit':
        sys.exit(0)
    # fi

    try:
        exp = parse(wline)
        print('{} -> {}'.format(wline, exp))
    except RuntimeError as run_error:
        print('Unable to parse {}: {}'.format(wline, run_error))
    # yrt
    return
# End of process_line

#-----------------------------------------------------------------------------
# Main program follows.
def main() -> None:
    """Main entry point for standalone interactive calculator mode."""
    cexp_parser()              # Set up for parsing.

    while True:
        line = get_line()
        if line:
            # Split line on spaces, then process line.
            t = ''
            process_line(t, line)
        # fi
    # elihw
    return
# End of main

# #-----------------------------------------------------------------------------
global linecount
linecount = 0
#-----------------------------------------------------------------------------
if __name__ == '__main__':
    main()
    exit(0)
# fi
#-----------------------------------------------------------------------------
# End of file calculate.py
#-----------------------------------------------------------------------------
