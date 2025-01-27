#!/usr/bin/python3 -B
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
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
#-----------------------------------------------------------------------------
#++ import inspect
#++ print(inspect.currentframe().f_code.co_name, '#0', file=sys.stderr, flush=True)
#-----------------------------------------------------------------------------
global arrays
arrays = [ ]
global local_arrays
local_arrays = [ ]
#-----------------------------------------------------------------------------
numarry_name = 0            # The name of the variable.
numarry_maclevel = 1        # The macro level was in effect when created.
numarry_indexes = 2         # The array indexes. []=value, [3]=1-dimen, [2,4]=2-dimen.
numarry_values = 3          # Array of values ([0] for not an array).
numarry_value_type = 4      # Array of types None=not-set, 0=int/float, 1= character string.
numarry_macro_arg = 5       # True if macro argument - normally False.

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
    def __init__(self, symbol, lprio, rprio, eval):
        self.symbol = symbol
        self.lprio = lprio
        self.rprio = rprio
        self.eval = eval
    # End of __init__

    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def __repr__(self):
        return "SymbolDesc: '{}',{},{}".format(self.symbol, self.lprio, self.rprio)
    # End of __repr__

#-----------------------------------------------------------------------------
# Returns:
#   therest - everything after this token
#   token   - the token
#   kind    - 'NUMBER' if a number

def next_token(string):
#PRINT    print("next_token - Entering type(string)='{}' string='{}'".format(type(string),string), file=sys.stderr, flush=True)   # PRINT
    # Make sure string exists. next_token useable by other than tokenize.
    string = string.strip()
    if len(string) <= 0:
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
    if re.match(r'[-+*/=$<>!]', c):         # Possible operation
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
    elif c.isalpha():                       # First character is [a-zA-Z].
        m = re.match(r'[_a-zA-Z0-9]+', string)
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
        m = re.match(r'[0-9.]+', string)
        strg = m.group(0)                   # We know there is at least one character.
        if strg.count('.') > 1:             # Only one decimal place in a number.
            return None, strg, 'MISMATCH #d strg={}'.format(strg)
        elif len(string) > len(strg):         # Something after this alphabetic string.
            b = float(strg)
            return string[len(strg):], b, 'NUMBER'
        # fi
        return None, strg, 'NUMBER'
    elif re.match(r'[\[\](){}]', c):        # Known parenthesis/brackets/braces.
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
def tokenize(code):
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
def identity_eval(args):
#PRINT    print("identity_eval - Entering args='{}'".format(args), file=sys.stderr, flush=True)    # PRINT
    if len(args) == 1:
        if type(args[0]) == SymbolDesc:
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
def get_value(arg):
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
    for wary in arrays + local_arrays:
#PRINT        print('arg[1]={} wary[numarry_name]={} wary[numarry_values]={}'.format(arg[1],wary[numarry_name],wary[numarry_values]), file=sys.stderr, flush=True)  # PRINT
        if arg[1] == wary[numarry_name]:
            if wary[numarry_maclevel] >  maxmaclev:
                maxmaclev = wary[numarry_maclevel]
                maxwary = wary
            # fi
        # fi
    # rof
#PRINT    print("get_value - #i maxwary={}".format(maxwary), file=sys.stderr, flush=True)  # PRINT
    if maxwary is None:
        return [ "ERROR - get_value - unrecognized variable='{}'".format(arg), None ]
    elif len(maxwary[numarry_indexes]) != 0:
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
def fix_to_number(t,a):
    if t == 'CHAR':
        try:
            a = float(a)
            t = 'NUMBER'
        except:
            pass
        # fi
    # fi
    return t, a
# End of fix_to_number

#-----------------------------------------------------------------------------
#        t1,a1,t2,a2 = fix_to_numbers(t1,a1,t2,a2)
# Try to make both a1 and a2 numbers.
def fix_to_numbers(t1,a1,t2,a2):
#PRINT    print("fix_to_numbers - Entering t1={} a1={} t2={} a2={}".format(t1,a1,t2,a2), file=sys.stderr, flush=True)  # PRINT
    t1, a1 = fix_to_number(t1, a1)
    t2, a2 = fix_to_number(t2, a2)
    return t1,a1,t2,a2
# End of fix_to_numbers

#-----------------------------------------------------------------------------
def compute_value(op, arg1, arg2):
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
        for wary in arrays + local_arrays:
            if a == wary[numarry_name]:
                if wary[numarry_maclevel] >  maxmaclev:
                    maxmaclev = wary[numarry_maclevel]
                    maxwary = wary
                # fi
            # fi
        # rof
#PRINT        print("compute_value - #h", file=sys.stderr, flush=True)  # PRINT
        if maxwary is None:
            print("Assignment to unknown variable '{}', creating it='{}'".format(arg1, a2), file=sys.stderr, flush=True)
            if arg2[0] == 'NUMBER':
                local_arrays.append( [ a, 0, [ ], [ arg2[1] ], [ 0 ], False ] )
            else:                               # Assume CHAR
                local_arrays.append( [ a, 0, [ ], [ arg2[1] ], [ 1 ], False ] )
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
def quotes_eval(args):
#PRINT    print("quotes_eval - Entering args='{}'".format(args), file=sys.stderr, flush=True) # PRINT
    if args is None or len(args) != 3:
        return [ 'ERROR - quotes_eval wrong number of arguments, args={}'.format(args), None ]
    # fi
    a0 = args[0]
    a1 = args[1]
    a2 = args[2]
    if type(a0) == SymbolDesc or type(a1) != SymbolDesc or type(a2) == SymbolDesc:
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
    return [ value ]
# End of quotes_eval

#-----------------------------------------------------------------------------
def binary_eval(args):
#PRINT    print('binary_eval - Entering args={}'.format(args), file=sys.stderr, flush=True)    # PRINT
    if args is None or len(args) != 3:
        return [ 'ERROR - binary_eval wrong number of arguments, args={}'.format(args), None ]
    # fi
    a0 = args[0]
    a1 = args[1]
    a2 = args[2]
    if type(a0) == SymbolDesc or type(a1) != SymbolDesc or type(a2) == SymbolDesc:
        return [ 'ERROR - binary_eval args={}'.format(args), None ]
    # fi
    value = compute_value(a1.symbol, a0, a2)
    return [ value ]
# End of binary_eval

#-----------------------------------------------------------------------------
def unary_eval(args):
#PRINT    print('unary_eval - Entering args={}', file=sys.stderr, flush=True)    # PRINT
    if len(args) != 2:
        return [ 'ERROR - unary_eval args={}'.format(args), None ]
    elif type(args[0]) == SymbolDesc and type(args[1]) != SymbolDesc:
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
    elif type(args[0]) != SymbolDesc and type(args[1]) == SymbolDesc:
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
def register_presymbol(oper, lprio, rprio, eval=None):
    global presymbols

    if eval is None:
        eval = unary_eval
    # fi
    if type(oper) is str:
        presymbols[oper] = SymbolDesc(oper, lprio, rprio, eval)
    else:
        for op in oper:
            presymbols[op] = SymbolDesc(op, lprio, rprio, eval)
        # rof
    # fi
# End of register_presymbol

#-----------------------------------------------------------------------------
def register_postsymbol(oper, lprio, rprio, eval=None):
    global postsymbols

    if eval is None:
        eval = binary_eval
    # fi
    if type(oper) is str:
        postsymbols[oper] = SymbolDesc(oper, lprio, rprio, eval)
    else:
        for op in oper:
            postsymbols[op] = SymbolDesc(op, lprio, rprio, eval)
        # rof
    # fi
# End of register_postsymbol

#-----------------------------------------------------------------------------
def id_symbol(id):
#PRINT    print('id_symbol - Entering id={}'.format(id), file=sys.stderr, flush=True)  # PRINT
    return SymbolDesc(id, 99999, 100000, identity_eval)
# End of id_symbol

#-----------------------------------------------------------------------------
def evaluate_handle(args):
#PRINT    print('evaluate_handle - Entering args={}'.format(args), file=sys.stderr, flush=True)  # PRINT
    for i in args:
        if type(i) == SymbolDesc:
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
def advance():
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
def reset(s):
    global lexer

#PRINT    print("reset - Entering s='{}'".format(s), file=sys.stderr, flush=True) # PRINT
    lexer = tokenize(s)
    advance()
# End of reset

#-----------------------------------------------------------------------------
def cur_sym(allow_presymbol):
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
def parse_to(prio):
#PRINT    print('parse_to - Entering', file=sys.stderr, flush=True)  # PRINT
    args = []
    while True:
        assert len(args) == 0 or (len(args) == 1 and type(args[0]) != SymbolDesc)
        sym = cur_sym(len(args) == 0)
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
    elif len(args) == 0:
        return None
    # fi
    return "ERROR - parse_to runs off the end of routine '{}'".format(args)
# End of parse_to

#-----------------------------------------------------------------------------
def parse(s):
    global cur_token

#PRINT    print("parse - Entering s='{}'".format(s), file=sys.stderr, flush=True) # PRINT
    reset(s)
    try:                                # NOTDONEYET - move to only around specific OOR plaes.
        res = parse_to(0)
    except:
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

def get_tokens_from_char(strng, start, lth):
    start = int(round(start)) - 1
#PRINT    print("get_tokens_from_char - Entering strng={} start={} lth={}".format(strng,start,lth), file=sys.stderr, flush=True)  # PRINT
    if strng is None or strng == '' or start < 0:
        return ''
    # fi
    a = [i for i in re.split(r'([a-zA-Z0-9_+-]+|[ \t]*[^a-zA-Z0-9_+-]+[ \t]*)', strng) if i]
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
        x = re.sub(r'\s+', r' ', i)
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
    w = ''
    prev = ''
#PRINT    print("get_tokens_from_char - #3", file=sys.stderr, flush=True)  # PRINT
    for y in x:
        if y == '':
            w = w + ' '
        elif re.match(r'\w+', prev) and re.match(r'\w+', y):
            w = w + ' ' + y
        else:
            w = w + y
        # fi
        prev = y
    # rof
#PRINT    print("get_tokens_from_char - #4", file=sys.stderr, flush=True)  # PRINT
#PRINT    print("get_tokens_from_char - return {}".format(w), file=sys.stderr, flush=True)  # PRINT
    return w
# End of get_tokens_from_char

#-----------------------------------------------------------------------------
def result_functions(arg1, arg2):
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
        except:
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
            if type(a2[1]) is not int and type(a2[1]) is not float: 
                return [ "ERROR - character token fetching needs first argument as integer, but this has {}".format(type(a2[1][0])), None ]
            # fi
#PRINT            print('result_functions #1- calling get_tokens_from_char', file=sys.stderr, flush=True)  # PRINT
            x = get_tokens_from_char(a1[1],a2[1],None)
#PRINT            print('result_functions #2- after get_tokens_from_char', file=sys.stderr, flush=True)  # PRINT
        elif a2[0] == 'COMMA':              # Go from token number a2[1][0] for length a2[1][1]
            if len(a2[1]) != 2:
                return [ "ERROR - character token fetching needs 1 or 2 arguments, but this has {}".format(len(a2[1])), None ]
            # fi
            if type(a2[1][0]) is not int and type(a2[1][0]) is not float: 
                return [ "ERROR - character token fetching needs first argument as integer, but this has {}".format(type(a2[1][0])), None ]
            elif type(a2[1][1]) is not int and type(a2[1][1]) is not float: 
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
def common_grouping_eval(args, txt, open_char, close_char):
#PRINT    print("common_grouping_eval - len(args)={} args='{}'".format(len(args),args), file=sys.stderr, flush=True)  # PRINT
    if len(args) == 3:
        r = args[0]
        s = args[1]
        t = args[2]
        if (type(r) == SymbolDesc and args[0].symbol == open_char
            and type(s) != SymbolDesc
            and type(t) == SymbolDesc and t.symbol == close_char):
            return [ s ]
        # fi
    # fi
    if len(args) == 4:
        r = args[0]
        s = args[1]
        t = args[2]
        u = args[3]
        if (type(r) != SymbolDesc
            and type(s) == SymbolDesc and s.symbol == open_char
            and type(t) != SymbolDesc
            and type(u) == SymbolDesc and u.symbol == close_char):
#PRINT            print("common_grouping_eval - #4 - r='{}' t='{}'".format(r,t), file=sys.stderr, flush=True)  # PRINT
            a = result_functions(r, t)
            return [ a ]
        #fi
        if (type(r) == SymbolDesc and r.symbol == open_char
            and type(s) != SymbolDesc
            and type(t) == SymbolDesc and t.symbol == close_char
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
        if (type(r) == SymbolDesc and (r.symbol in ['(','[','{'])
            and type(s) != SymbolDesc
# NOTDONEYET = {}
            and type(t) == SymbolDesc and (t.symbol in [')',']','}'])
# NOTDONEYET = {}
            and type(u) == SymbolDesc and (u.symbol in ['(','[','{'])
            and type(v) != SymbolDesc
# NOTDONEYET = {}
            and type(w) == SymbolDesc and (w.symbol in [')',']','}'])):
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
            and type(s) == SymbolDesc and (s.symbol in ['(','[','{'])
            and type(t) != SymbolDesc                       # NUMBER abc
# NOTDONEYET = {}
            and type(u) == SymbolDesc and (u.symbol in [')',']','}'])
# NOTDONEYET = {}
            and type(v) == SymbolDesc and (v.symbol in ['(','[','{'])
            and type(w) != SymbolDesc                       # NUMBER def
# NOTDONEYET = {}
            and type(x) == SymbolDesc and (x.symbol in [')',']','}'])):
#PRINT            print("common_grouping_eval - #7 - r='{}' t='{}'".format(r,t), file=sys.stderr, flush=True)  # PRINT
            a1 = result_functions(r, t)
            if type(a1) == list and len(a1) > 0:
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
def open_paren_eval(args):
#PRINT    print("open_paren_eval - args='{}'".format(args), file=sys.stderr, flush=True)  # PRINT
    return common_grouping_eval(args, 'paren', '(', ')')
# End of open_paren_eval

#-----------------------------------------------------------------------------
def close_paren_eval(args):
#PRINT    print("close_paren_eval - args='{}'".format(args), file=sys.stderr, flush=True) # PRINT
    return [ "ERROR - close_paren_eval args='{}'".format(args), None ]
# End of close_paren_eval

#-----------------------------------------------------------------------------
def open_bracket_eval(args):
#PRINT    print("open_bracket_eval - args='{}'".format(args), file=sys.stderr, flush=True)    # PRINT
    return common_grouping_eval(args, 'bracket', '[', ']')
# End of open_bracket_eval

#-----------------------------------------------------------------------------
def close_bracket_eval(args):
#PRINT    print("close_bracket_eval - args='{}'".format(args), file=sys.stderr, flush=True)   # PRINT
    return [ "ERROR - close_bracket_eval args='{}'".format(args), None ]
# End of close_bracket_eval

#-----------------------------------------------------------------------------
def open_brace_eval(args):
#PRINT    print("open_brace_eval - args='{}'".format(args), file=sys.stderr, flush=True)  # PRINT
# NOTDONEYET = {}
    return common_grouping_eval(args, 'brace', '{', '}')
# End of open_brace_eval

#-----------------------------------------------------------------------------
def close_brace_eval(args):
#PRINT    print("close_brace_eval - args='{}'".format(args), file=sys.stderr, flush=True) # PRINT
    return [ "ERROR - close_brace_eval args='{}'".format(args), None ]
# End of close_brace_eval

#-----------------------------------------------------------------------------
def f_m(arg):
#PRINT    print("f_m - arg='{}'".format(arg), file=sys.stderr, flush=True)    # PRINT
    try:
        a = float(arg[1])
        a = int(a)
    except:
        return [ "ERROR - argument to array m is not an integer '{}'".format(txt, arg), None ]
    # yrt
    if a < 1 or a > 50:
        return [ "ERROR - argument to array m is out of range 1 to 50 '{}'".format(txt, a), None ]
    # fi
    return [ 'ID', 'm' + str(a) ]
# End of f_m

#-----------------------------------------------------------------------------
def f_ceil(arg):
    a = float(arg[1])
    a = math.ceil(a)
    return [ 'NUMBER', a ]
# End of f_ceil

#-----------------------------------------------------------------------------
def f_floor(arg):
    a = float(arg[1])
    a = math.floor(a)
    return [ 'NUMBER', a ]
# End of f_floor

#-----------------------------------------------------------------------------
def f_freq(arg):
    a = float(arg[1])
    a = 968000.0 / a
    return [ 'NUMBER', a ]
# End of f_freq

#-----------------------------------------------------------------------------
def f_nearest(arg):
    a = float(arg[1])
    a = int(round(math.log(a / 27.5) * (12.0 / 0.693147)))
    return [ 'NUMBER', a ]
# End of f_nearest

#-----------------------------------------------------------------------------
def f_abs(arg):
    a = float(arg[1])
    a = math.fabs(a)
    return [ 'NUMBER', a ]
# End of f_abs

#-----------------------------------------------------------------------------
def f_arctan(arg):
    a = float(arg[1])
    a = math.atan(a)
    return [ 'NUMBER', a ]
# End of f_arctan

#-----------------------------------------------------------------------------
def f_cos(arg):
    a = float(arg[1])
    a = math.cos(a)
    return [ 'NUMBER', a ]
# End of f_cos

#-----------------------------------------------------------------------------
def f_exp(arg):
    a = float(arg[1])
    a = math.exp(a)
    return [ 'NUMBER', a ]
# End of f_exp

#-----------------------------------------------------------------------------
def f_frac(arg):
    a = float(arg[1])
    a = int(round(a)) - a
    return [ 'NUMBER', a ]
# End of f_frac

#-----------------------------------------------------------------------------
def f_float(arg):
    a = float(arg[1])
    return [ 'NUMBER', a ]
# End of f_float

#-----------------------------------------------------------------------------
def f_int(arg):
    a = float(arg[1])
    a = int(a)
    return [ 'NUMBER', a ]
# End of f_int

#-----------------------------------------------------------------------------
def f_in(arg):
#PRINT    print("f_in - arg='{}'".format(arg), file=sys.stderr, flush=True)   # PRINT
    val = arg[1]
    if len(val) != 3:
        return [ "ERROR - in() needs two arguments, not: {}'".format(val), None ]
    # fi
    # check if types of 3 arguments are float/int. 
    n = 1
    for x in val:
        if type(x) is not float and type(x) is not int:
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
def f_log(arg):
    a = float(arg[1])
    try:
        a = math.log(a, 10)
    except:
        a = 0
    # ytr
    return [ 'NUMBER', a ]
# End of f_log

#-----------------------------------------------------------------------------
def f_ln(arg):
    a = float(arg[1])
    a = math.log(a)
    return [ 'NUMBER', a ]
# End of f_ln

#-----------------------------------------------------------------------------
def f_round(arg):
    a = float(arg[1])
    a = int(round(a))
    return [ 'NUMBER', a ]
# End of f_round

#-----------------------------------------------------------------------------
def f_sign(arg):
    a = float(arg[1])
    if a < 0:
        return [ 'NUMBER', -1 ]
    elif a > 0:
        return [ 'NUMBER', 1 ]
    # fi
    return [ 'NUMBER', 0 ]
# End of f_sign

#-----------------------------------------------------------------------------
def f_sin(arg):
    a = float(arg[1])
    a = math.sin(a)
    return [ 'NUMBER', a ]
# End of f_sin

#-----------------------------------------------------------------------------
def f_sqrt(arg):
    a = float(arg[1])
    a = math.sqrt(a)
    return [ 'NUMBER', a ]
# End of f_sqrt

#-----------------------------------------------------------------------------
# Invert logical expression (-1 = true, 0 = false). Make anything non-zero be true.
def f_not(arg):
    a = float(arg[1])
    if a < 0 or a > 0:
        a = -1
    else:
        a = 0
    # fi
    return [ 'NUMBER', a ]
# End of f_not

#-----------------------------------------------------------------------------
def f_defined(arg):
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
def f_print(arg):
    which = arg[0]
    val = arg[1]
    if which in [ 'NUMBER', 'CHAR']:
        print(val)
        return arg
    elif which == 'COMMA':
        a = ''
        for i in val:
            if a != '':
                a = a + ','
            # fi
            a = a + str(i)
        # rof
        print(a)
        return arg
    else:
        print('PRINT: UNKNOWN TYPE={}'.format(arg), file=sys.stderr, flush=True)
        return arg
    # fi
# End of f_print

#-----------------------------------------------------------------------------
def f_max(arg):
    val = arg[1]
    if len(val) != 2:
        return [ "ERROR - max needs two arguments, not: {}'".format(val), None ]
    # fi
    n = 1
    for x in val:
        if type(x) is not float and type(x) is not int:
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
def f_min(arg):
    val = arg[1]
    if len(val) != 2:
        return [ "ERROR - min needs two arguments, not: {}'".format(val), None ]
    # fi
    n = 1
    for x in val:
        if type(x) is not float and type(x) is not int:
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
def f_mod(arg):
#PRINT    print("f_mod - arg='{}'".format(arg), file=sys.stderr, flush=True)  # PRINT
    val = arg[1]
    if len(val) != 2:
        return [ "ERROR - mod needs two arguments, not: {}'".format(val), None ]
    # fi
    n = 1
    for x in val:
        if type(x) is not float and type(x) is not int:
            return [ "ERROR - mod() needs arguments to be integer or float. #{} {} is type {}".format(n, x, type(x)), None ]
        # fi
        n = n + 1
    # rof
    val1 = arg[1][0]
    val2 = arg[1][1]
    if val2 == 0:
        return [ "ERROR - mod needs second argument positive, not: {}'".format(val2), None ]
    # fi
    if type(val1) is int and type(val2) is int:
        value = val1 % val2
    else:
        value = math.fmod(float(val1), float(val2))
    # fi
    return [ 'NUMBER', value ]
# End of f_mod

#-----------------------------------------------------------------------------
def f_clock(arg):
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
arrays.append( [ 'e',   0, [ ], [ math.e ],   [ 0 ] , False ] )
arrays.append( [ 'tau', 0, [ ], [ math.tau ], [ 0 ] , False ] )

arrays.append( [ 'm1',  0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm2',  0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm3',  0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm4',  0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm5',  0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm6',  0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm7',  0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm8',  0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm9',  0, [ ], [ 0 ],        [ 0 ] , False ] )

arrays.append( [ 'm10', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm11', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm12', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm13', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm14', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm15', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm16', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm17', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm18', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm19', 0, [ ], [ 0 ],        [ 0 ] , False ] )

arrays.append( [ 'm20', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm21', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm22', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm23', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm24', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm25', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm26', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm27', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm28', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm29', 0, [ ], [ 0 ],        [ 0 ] , False ] )

arrays.append( [ 'm30', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm31', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm32', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm33', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm34', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm35', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm36', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm37', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm38', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm39', 0, [ ], [ 0 ],        [ 0 ] , False ] )

arrays.append( [ 'm40', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm41', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm42', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm43', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm44', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm45', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm46', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm47', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm48', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm49', 0, [ ], [ 0 ],        [ 0 ] , False ] )

arrays.append( [ 'm50', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm51', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm52', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm53', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm54', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm55', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm56', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm57', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm58', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm59', 0, [ ], [ 0 ],        [ 0 ] , False ] )

arrays.append( [ 'm60', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm61', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm62', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm63', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm64', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm65', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm66', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm67', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm68', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm69', 0, [ ], [ 0 ],        [ 0 ] , False ] )

arrays.append( [ 'm70', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm71', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm72', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm73', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm74', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm75', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm76', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm77', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm78', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm79', 0, [ ], [ 0 ],        [ 0 ] , False ] )

arrays.append( [ 'm80', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm81', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm82', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm83', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm84', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm85', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm86', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm87', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm88', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm89', 0, [ ], [ 0 ],        [ 0 ] , False ] )

arrays.append( [ 'm90', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm91', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm92', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm93', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm94', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm95', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm96', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm97', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm98', 0, [ ], [ 0 ],        [ 0 ] , False ] )
arrays.append( [ 'm99', 0, [ ], [ 0 ],        [ 0 ] , False ] )

arrays.append( [ 'm100', 0, [ ], [ 0 ],        [ 0 ] , False ] )
#-----------------------------------------------------------------------------
def cexp_parser():
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
def get_line():
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
                line = re.sub(r'[$][$].*$', r'', line)
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
        except:
            print('Read got a processing error', file=sys.stderr)
            print('   ', sys.exc_info()[0], sys.exc_info, file=sys.stderr, flush=True)
        # yrt
        break
    # elihw
    sys.exit(0)
# End of get_line

#-----------------------------------------------------------------------------
# Parse and process line.
def process_line(t, line):
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
def main():
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
