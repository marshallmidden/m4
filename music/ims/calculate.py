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
#-----------------------------------------------------------------------------
#++ import inspect
#++ print(inspect.currentframe().f_code.co_name, '#0')
#-----------------------------------------------------------------------------
global arrays
arrays = [ ]
global local_arrays
local_arrays = [ ]
global note_arrays
note_arrays = [ ]
#-----------------------------------------------------------------------------
numarry_name = 0            # The name of the variable.
numarry_maclevel = 1        # The macro level was in effect when created.
numarry_indexes = 2         # The array indexes. []=value, [3]=1-dimen, [2,4]=2-dimen.
numarry_values = 3          # Array of values ([0] for not an array).
numarry_value_type = 4      # Array of types None=not-set, 0=int/float, 1= character string

#-- warray = [ 'abc', 8,
#--            [ ],             # Zero dimension
#--            [ 123.000 ],
#--            [ 0 ] ]       # Character string, number, No dimensions.
#-- arrays.append(warray)
#-- 
#-- warray = [ 'abc1', 8,
#--            [ 2 ],           # 1 dimension
#--            [ 'abc1', 123.001 ],
#--            [ 1, 0 ] ]       # Character string, number, 1 dimension.
#-- arrays.append(warray)
#-- 
#-- warray = [ 'abc2', 8,
#--            [ 1, 2 ],        # 2 dimensions
#--            [ 'abc2', 123.002 ],
#--            [ 1, 0 ] ]    # Not-set-yet, int/float
#-- arrays.append(warray)
#-- 
#-- warray = [ 'abc3', 8,
#--            [ 1, 1, 2 ],        # 2 dimensions
#--            [ 'abc3', 123.003 ],
#--            [ 1, 0 ] ]    # Not-set-yet, int/float
#-- arrays.append(warray)
#-- 
#-- warray = [ 'def', 8,
#--            [ 2, 2 ],        # 2 dimensions
#--            [ 1, 2, 3, 4 ],
#--            [ 0, 0, 0, 0 ] ]    # Not-set-yet, int/float
#-- arrays.append(warray)
#-- 
#-- warray = [ 'ghi', 8,
#--            [ 3, 2, 2 ],        # 2 dimensions
#--            [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 ],
#--            [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ] ]    # Not-set-yet, int/float
#-- arrays.append(warray)
#-- 
#-- warray = [ 'xyz', 8,
#--            [ ], 
#--            [ 'm1' ],
#--            [ 1 ] ]    # Not-set-yet, int/float
#-- local_arrays.append(warray)
#-- 
#-- warray = [ 'tuv', 8,
#--            [ ], 
#--            [ '1.234' ],
#--            [ 1 ] ]    # Not-set-yet, int/float
#-- local_arrays.append(warray)
#-- 
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
#--    print('next_token - Entering string={}'.format(string))
    # Make sure string exists. next_token useable by other than tokenize.
    if len(string) <= 0:
        return None, None, None
    # fi
    # First character processing is special.
    c = string[0]                           # Character in string
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
        elif c == '=' and len(string) >= 2:   # Possible ==, =>, =<
            if string[1] == '=':
                return string[2:], '==', 'OPER'
            elif string[1] == '>':
                return string[2:], '>=', 'OPER'
            elif string[1] == '<':
                return string[2:], '<=', 'OPER'
            # fi
            return string[1:], '=', 'OPER'
        elif c == '!' and len(string) >= 2:   # Possible !=
            if string[1] == '=':
                return string[2:], '!=', 'OPER'
            # fi
            return None, c, 'MISMATCH #c len(string)={}'.format(len(string))
        elif c == '<' and len(string) >= 2:   # Possible <=
            if string[1] == '=':
                return string[2:], '<=', 'OPER'
            # fi
            return string[1:], '<', 'OPER'
        elif c == '>' and len(string) >= 2:   # Possible <=
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
    elif c.isalpha():                         # First character is [a-zA-Z].
        m = re.match(r'[_a-zA-Z0-9]+', string)
        ret = strg = m.group(0)             # We know there is at least one character.
        if strg == 'lpause':                 # convert a few names to the other name.
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
    elif c.isdigit() or c == '.':             # First is a digit.
        m = re.match(r'[0-9.]+', string)
        strg = m.group(0)                   # We know there is at least one character.
        if strg.count('.') > 1:             # Only one decimal place in a number.
            return None, strg, 'MISMATCH #d strg={}'.format(strg)
        elif len(string) > len(strg):         # Something after this alphabetic string.
            return string[len(strg):], strg, 'NUMBER'
        # fi
        return None, strg, 'NUMBER'
    elif re.match(r'[\[\](){}]', c):          # Known parenthesis/brackets/braces.
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
#--    print('tokenize - Entering')
    code = ''.join(code.split())
    # Get next token.
    last_kind = ''
    last_token = ''
    while code is not None and code != '':
        therest, token, kind = next_token(code)
        if (((last_kind == 'NUMBER' or last_kind == 'ID') and
             (kind == 'NUMBER' or kind == 'ID')) or
            ((last_kind == 'SYNTAX' and kind == 'SYNTAX') and
             (last_token in [')', ']', '}'] and token in ['(', '[', '{'])) or
            (last_kind == 'SYNTAX' and last_token in [')',']','}'] and
             (kind == 'ID' or kind == 'NUMBER')) ):
            # Implied multiplication between numbers and id's.
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
#--    print('identity_eval - Entering args={}'.format(args))
    if len(args) == 1:
        if type(args[0]) == SymbolDesc:
            return [ args[0].symbol ]
        # fi
    elif len(args) == 2:
        a = args[0]
        if a[0] == 'NUMBER':
            return [ a ]
        # fi
    # fi
    return [ "ERROR - ID type(args[0])={} args[0]='{}'".format(type(args[0]), args[0]), None ]
# End of identity_eval

#-----------------------------------------------------------------------------
def get_value(arg):
    global arrays
    global local_arrays
    global note_arrays

#--    print('get_value - Entering arg={}'.format(arg))
    if arg[0].startswith('ERROR'):
        return arg
    elif arg[0] == 'NUMBER':
        arg[1] = float(arg[1])
        return arg
    elif arg[0] == 'COMMA':
        return arg
    elif arg[0] == 'CHAR':
        return arg
    elif arg[0] == 'ADDRESS':
        a = arg[1]
        idx = a[0]
        warray = a[1]
        where_value = warray[numarry_values][idx]
        where_type = warray[numarry_value_type][idx]
        if where_type is None:
            arg[0] = 'CHAR'
            arg[1] = 'None'
        elif where_type == 0:
            arg[0] = 'NUMBER'
            arg[1] = where_value
        else:
            arg[0] = 'CHAR'
            arg[1] = where_value
        # fi
        return arg
    elif arg[0] != 'ID':
        return [ "ERROR - get_value - unrecognized variable type='{}'".format(arg), None ]
    # fi

    maxmaclev = -1
    maxwary = None
#--    print('arrays={}'.format(arrays))
#--    print('local_arrays={}'.format(local_arrays))
#--    print('note_arrays={}'.format(note_arrays))
    for wary in arrays + local_arrays + note_arrays:
#--        print('arg[1]={} wary[numarry_name]={}'.format(arg[1],wary[numarry_name]), file=sys.stderr, flush=True)
        if arg[1] == wary[numarry_name]:
            if wary[numarry_maclevel] >  maxmaclev:
                maxmaclev = wary[numarry_maclevel]
                maxwary = wary
            # fi
        # fi
    # rof
    if maxwary is None:
        return [ "ERROR - get_value - unrecognized variable='{}'".format(arg), None ]
    elif len(maxwary[numarry_indexes]) != 0:
        return [ "ERROR - get_value - array needs '{}' arguments".format(len(maxwary[numarry_indexes])), None ]
    elif maxwary[numarry_value_type][0] is None:
        return [ "ERROR - get_value - variable is not set yet - '{}'".format(arg), None ]
    elif maxwary[numarry_value_type][0] == 0:
        arg[0] = 'NUMBER'
        arg[1] = float(maxwary[numarry_values][0])
    else:
        arg[0] = 'CHAR'
        arg[1] = str(maxwary[numarry_values][0])
    # fi
    return arg
# End of get_value

#-----------------------------------------------------------------------------
def compute_value(op, arg1, arg2):
    global arrays
    global local_arrays
    global note_arrays

    a2 = get_value(arg2)
    if a2 is None:
        return [ "ERROR - Argument2 is None a2='{}'".format(a2), None ]
    elif a2[0].startswith('ERROR'):
        return a2
    elif a2[0] not in [ 'NUMBER', 'CHAR' ]:
        return [ "ERROR - Argument2 is not a Number a2='{}'".format(a2), None ]
    elif op == '=':
        if arg1[0] == 'ADDRESS':
            a = arg1[1]
            idx = a[0]
            warray = a[1]
            warray[numarry_values][idx] = arg2[1]
            if arg2[0] == 'NUMBER':
                warray[numarry_value_type][idx] = 0
            elif arg2[0] == 'CHAR':
                warray[numarry_value_type][idx] = 1
            else:
                return [ "ERROR - ADDRESS and arg2 unrecognized '{}'".format(arg2), None]
            # fi
            return arg2
        # fi

        if arg1[0] not in [ 'ID', 'ADDRESS' ]:
            return [ "ERROR - Argument1 is not a variable name arg1='{}'".format(arg1), None ]
        # fi

        # arrays here.
        maxmaclev = -1
        maxwary = None
        for wary in arrays + local_arrays + note_arrays:
            if arg1[1] == wary[numarry_name]:
                if wary[numarry_maclevel] >  maxmaclev:
                    maxmaclev = wary[numarry_maclevel]
                    maxwary = wary
                # fi
            # fi
        # rof
        if maxwary is None:
            print("Assignment to unknown variable '{}', creating it = '{}'".format(arg1, a2))
            if arg2[0] == 'NUMBER':
                local_arrays.append( [ arg1[1],  0, [ ], [ arg2[1] ],  [ 0 ] ] )
            else:                               # Assume CHAR
                local_arrays.append( [ arg1[1],  0, [ ], [ arg2[1] ],  [ 1 ] ] )
            # fi
            return arg2
        elif maxwary[numarry_indexes] != []:
            return [ "ERROR - compute_value - variable is array '{}'".format(arg), None ]
        elif arg2[0] == 'NUMBER':
            maxwary[numarry_value_type][0] = 0
        else:                                   # Assume CHAR
            maxwary[numarry_value_type][0] = 1
        # fi
        maxwary[numarry_values][0] = arg2[1]
        return arg2
    # fi

    a1 = get_value(arg1)
    if a1 is None:
        return [ "ERROR - Argument1 is None a1='{}'".format(a1), None ]
    elif a1[0].startswith('ERROR'):
        return a1
    elif a1[0] != 'NUMBER':
        return [ "ERROR - Argument1 is not a Number a1='{}'".format(a1), None ]
    # fi
    a1 = a1[1]
    a2 = a2[1]

    # Math: +, -, *, /, **
    if op == '+':
        return [ 'NUMBER', a1 + a2 ]
    elif op == '-':
        return [ 'NUMBER',  a1 - a2 ]
    elif op == '*':
        return [ 'NUMBER',  a1 * a2 ]
    elif op == '/':
        return [ 'NUMBER',  a1 / a2 ]
    elif op == '**':
        return [ 'NUMBER',  a1 ** a2 ]
    # Logical: >, >=, <=, <, ==, !=
    elif op == '<=':
        return [ 'NUMBER', -1 if a1 <= a2 else 0 ]
    elif op == '>=':
        return [ 'NUMBER',  -1 if a1 >= a2 else 0 ]
    elif op == '>':
        return [ 'NUMBER',  -1 if a1 > a2 else 0 ]
    elif op == '<':
        return [ 'NUMBER',  -1 if a1 < a2 else 0 ]
    elif op == '==':
        return [ 'NUMBER',  -1 if a1 == a2 else 0 ]
    elif op == '!=':
        return [ 'NUMBER',  -1 if a1 != a2 else 0 ]

    # Bitwise: $cls$, $ars$, $mask$, $union$, $diff$
    elif op == '$mask$':
        return [ 'NUMBER',  int(a1) & int(a2) ]
    elif op == '$union$':
        return [ 'NUMBER',  int(a1) | int(a2) ]
    elif op == '$cls$':
        return [ 'NUMBER',  int(a1) << int(a2) ]
    elif op == '$ars$':
        return [ 'NUMBER',  int(a1) >> int(a2) ]
    elif op == '$diff$':
        return [ 'NUMBER',  int(a1) ^ int(a2) ]             # xor

    # combination: $and$, $or$
    elif op == '$and$':
        return [ 'NUMBER',  -1 if int(a1) == -1 and int(a2) == -1 else 0 ]
    elif op == '$or$':
        return [ 'NUMBER',  -1 if int(a1) == -1 or int(a2) == -1 else 0 ]
    # fi

    return [ "ERROR - unknown operator '{}' '{}' '{}'".format(arg1, op, arg2) , None ]
# End of compute_value

#-----------------------------------------------------------------------------
def comma_eval(args):
    if args is None or len(args) != 3:
        return [ 'ERROR - comma_eval wrong number of arguments, args={}'.format(args), None ]
    # fi
    a0 = args[0]
    a1 = args[1]
    a2 = args[2]
    if type(a0) == SymbolDesc or type(a1) != SymbolDesc or type(a2) == SymbolDesc:
        return [ 'ERROR - comma_eval args={}'.format(args), None ]
    elif a0[0] != 'NUMBER':
        return [ 'ERROR - comma_eval first argument is not a NUMBER, args={}'.format(args), None ]
    elif a2[0] == 'NUMBER':
        value = [ 'COMMA', [ int(a0[1]), int(a2[1]) ] ]
    elif a2[0] == 'COMMA':
        value = [ 'COMMA', [ int(a0[1]) ] + a2[1] ]
    # fi
    return [ value ]
# End of comma_eval

#-----------------------------------------------------------------------------
def binary_eval(args):
#--    print('binary_eval - Entering')
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
#--    print('unary_eval - Entering')
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
            return [ "ERROR - Argument2 is not a Number arg1='{}'".format(arg1), None ]
        elif op == '-':
            arg1[1] = 0 - arg1[1]
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
#--    print('id_symbol - Entering')
    return SymbolDesc(id, 99999, 100000, identity_eval)
# End of id_symbol

#-----------------------------------------------------------------------------
def evaluate_handle(args):
#--    print('evaluate_handle - Entering args={}'.format(args))
    for i in args:
        if type(i) == SymbolDesc:
            a = i.eval(args)
            return a
        # fi
    # rof
    raise RuntimeError('Internal error: no eval found in {}'.format(args))
# End of evaluate_handle

#-----------------------------------------------------------------------------
global lexer
global cur_token

#-----------------------------------------------------------------------------
def advance():
    global lexer
    global cur_token

#--    print('advance - Entering')
    try:
        cur_token = lexer.__next__()                    # [ kind, item ]
    except StopIteration:
        cur_token = None
    # yrt
# End of advance

#-----------------------------------------------------------------------------
def reset(s):
    global lexer

#--    print('reset - Entering')
    lexer = tokenize(s)
    advance()
# End of reset

#-----------------------------------------------------------------------------
def cur_sym(allow_presymbol):
    global postsymbols
    global presymbols
    global cur_token
    
#--    print("cur_sym - Entering cur_token='{}'".format(cur_token))
    if cur_token is None:
        return None
    elif cur_token[0] == 'ID':                              # kind
        return id_symbol(cur_token)
    elif cur_token[0] == 'NUMBER':                          # kind
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
#--    print('parse_to - Entering')
    args = []
    while True:
        assert len(args) == 0 or (len(args) == 1 and type(args[0]) != SymbolDesc)
        sym = cur_sym(len(args) == 0)
        if sym is None or prio >= sym.lprio:
            break
        # fi
        while True:
            args.append(sym)
            advance()
            curprio = sym.rprio
            next = parse_to(curprio)
            if next is not None:
                args.append(next)
            # fi
            sym = cur_sym(next is None)
            if sym is None or curprio != sym.lprio:
                break
            # fi
        # elihw

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
        return a
    elif len(args) == 0:
        return None
    # fi
    return "ERROR - parse_to runs off the end of routine '{}'".format(args)
# End of parse_to

#-----------------------------------------------------------------------------
def parse(s):
    global cur_token

#--    print('parse - Entering')
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
def result_functions(arg1, arg2):
    global functions
    global arrays
    global local_arrays
    global note_arrays

#--    print('result_functions - #1 arg1={} arg2={}'.format(arg1,arg2))
    if arg1[0] != 'ID':
        # 'NUMBER' -- implied multiply
        # Do implied multiply
        a1 = get_value(arg1)
        if a1 is None:
            return [ "ERROR - value is None a1='{}'".format(a1), None ]
        elif a1[0].startswith('ERROR'):
            return a1
        elif a1[0] != 'NUMBER':
            return [ "ERROR - value is not a Number a1='{}'".format(a1), None ]
        # fi
        a2 = get_value(arg2)
        if a2 is None:
            return [ "ERROR - value is None a2='{}'".format(a2), None ]
        elif a2[0].startswith('ERROR'):
            return a2
        elif a2[0] != 'NUMBER':
            return [ "ERROR - value is not a Number a2='{}'".format(a2), None ]
        # fi
        a = [ 'NUMBER',  a1[1] * a2[1] ]
        return a
    # fi

    # -- ID --
    if arg1[1] in functions:
        fu = functions[arg1[1]]
        wh = fu[0]
        ar = fu[1]
        if arg2[0] not in ar:
            return [ "ERROR - function '{}' called with wrong argument type {} vs {}".format(arg1[1], arg2[0], ar), None ]
        # fi
        try:
            a = wh(arg2)
            return a
        except:
            return [ "ERROR - performing function '{}' with argument '{}'".format(arg1, arg2), None ]
        # yrt
    # fi

    if arg2[0] in ['NUMBER', 'COMMA', 'CHAR']:
        maxmaclev = -1
        maxwary = None
        for wary in arrays + local_arrays + note_arrays:
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
                elif maxwary[numarry_value_type][d-1] is None:
                    return [ 'ERROR - result_functions - variable {}({}) is not set'.format(arg1, d) ]
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

                d = arg2[1][0]
                x = maxwary[numarry_indexes][0]     # Max dimension of this..
                if d < 1 or d > x:
                    return [ 'ERROR - result_functions - variable {} dimension#1 {} not in range 1 thru {}'.format(arg1, d, x) ]
                # fi
                h = d - 1                           # array index into numarry_values & numarry_value_type
                mult = x
                for a in range(1, l):
                    d = arg2[1][a]
                    x = maxwary[numarry_indexes][a]
                    if d < 1 or d > x:
                        return [ 'ERROR - result_functions - variable {} dimension#{} {} not in range 1 thru {}'.format(arg1, a+1, d, x) ]
                    # fi
                    h = h + (d-1) * mult
                    mult = mult * x
                # rof

                if maxwary[numarry_value_type][h] is None:
                    return [ 'ERROR - result_functions - array {} ({}) is not set yet'.format(arg1,arg2[1]), None ]
                # fi
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
    elif a1[0] != 'NUMBER':
        return [ "ERROR - value is not a Number a1='{}'".format(a1), None ]
    # fi
    a2 = get_value(arg2)
    if a2 is None:
        return [ "ERROR - value is None a2='{}'".format(a2), None ]
    elif a2[0].startswith('ERROR'):
        return a2
    elif a2[0] != 'NUMBER':
        return [ "ERROR - value is not a Number a2='{}'".format(a2), None ]
    elif a1 and a2:
        a = [ 'NUMBER', a1[1] * a2[1] ]
        return a
    # fi
    return [[ "ERROR - Fetching from unknown function '{}' '{}'".format(arg1, arg2) , None ]]
# End of result_functions

#-----------------------------------------------------------------------------
def common_grouping_eval(args, txt, open_char, close_char):
#--    print("common_grouping_eval - len(args)={} args='{}'".format(len(args),args))
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
                return [ "ERROR - value is not a Number a1='{}'".format(a1), None ]
            # fi
            a2 = get_value(u)
            if a2 is None:
                return [ "ERROR - value is None a2='{}'".format(a2), None ]
            elif a2[0].startswith('ERROR'):
                return a2
            elif a2[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a2='{}'".format(a2), None ]
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
        if (type(r) == SymbolDesc and (r.symbol in ['(','[','{'])
            and type(s) != SymbolDesc
            and type(t) == SymbolDesc and (t.symbol in [')',']','}'])
            and type(u) == SymbolDesc and (u.symbol in ['(','[','{'])
            and type(v) != SymbolDesc
            and type(w) == SymbolDesc and (w.symbol in [')',']','}'])):
            # Do implied multiply
            a1 = get_value(s)
            if a1 is None:
                return [ "ERROR - value is None a1='{}'".format(a1), None ]
            elif a1[0].startswith('ERROR'):
                return a1
            elif a1[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a1='{}'".format(a1), None ]
            # fi
            a2 = get_value(v)
            if a2 is None:
                return [ "ERROR - value is None a2='{}'".format(a2), None ]
            elif a2[0].startswith('ERROR'):
                return a2
            elif a2[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a2='{}'".format(a2), None ]
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
            and type(s) == SymbolDesc and (s.symbol in ['(','[','{'])
            and type(t) != SymbolDesc                       # NUMBER abc
            and type(u) == SymbolDesc and (u.symbol in [')',']','}'])
            and type(v) == SymbolDesc and (v.symbol in ['(','[','{'])
            and type(w) != SymbolDesc                       # NUMBER def
            and type(x) == SymbolDesc and (x.symbol in [')',']','}'])):
            a1 = result_functions(r, t)
            if type(a1) == list and len(a1) > 0:
                a1 = a1[0]
            elif a1 is None:
                return [ "ERROR - value is None a1='{}'".format(a1), None ]
            elif a1[0].startswith('ERROR'):
                return a1
            elif a1[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a1='{}'".format(a1), None ]
            # fi
            a2 = get_value(w)
            if a2 is None:
                return [ "ERROR - value is None a2='{}'".format(a2), None ]
            elif a2[0].startswith('ERROR'):
                return a2
            elif a2[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a2='{}'".format(a2), None ]
            # fi
            a = [[ 'NUMBER',  a1[1] * a2[1] ]]
            return a
        #fi
    # fi
    return [ "ERROR - open_{}_eval args='{}'".format(txt, args), None ]
# End of common_grouping_eval

#-----------------------------------------------------------------------------
def open_paren_eval(args):
    return common_grouping_eval(args, 'paren', '(', ')')
# End of open_paren_eval

#-----------------------------------------------------------------------------
def close_paren_eval(args):
    return [ "ERROR - close_paren_eval args='{}'".format(args), None ]
# End of close_paren_eval

#-----------------------------------------------------------------------------
def open_bracket_eval(args):
    return common_grouping_eval(args, 'bracket', '[', ']')
# End of open_bracket_eval

#-----------------------------------------------------------------------------
def close_bracket_eval(args):
    return [ "ERROR - close_bracket_eval args='{}'".format(args), None ]
# End of close_bracket_eval

#-----------------------------------------------------------------------------
def open_brace_eval(args):
    return common_grouping_eval(args, 'brace', '{', '}')
# End of open_brace_eval

#-----------------------------------------------------------------------------
def close_brace_eval(args):
    return [ "ERROR - close_brace_eval args='{}'".format(args), None ]
# End of close_brace_eval

#-----------------------------------------------------------------------------
def f_m(arg):
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
# End of f_int

#-----------------------------------------------------------------------------
def f_int(arg):
    a = float(arg[1])
    a = int(a)
    return [ 'NUMBER', a ]
# End of f_int

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
        print('PRINT: UNKNOWN TYPE={}'.format(arg))
        return arg
    # fi
# End of f_print

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
    'log':      [ f_log,        ['NUMBER']],
    'ln':       [ f_ln,         ['NUMBER']],
    'not':      [ f_not,        ['NUMBER']],
    'round':    [ f_round,      ['NUMBER']],
    'sign':     [ f_sign,       ['NUMBER']],
    'sin':      [ f_sin,        ['NUMBER']],
    'sqrt':     [ f_sqrt,       ['NUMBER']],
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
arrays.append( [ 'pi',  0, [ ], [ math.pi ],  [ 0 ] ] )
arrays.append( [ 'e',   0, [ ], [ math.e ],   [ 0 ] ] )
arrays.append( [ 'tau', 0, [ ], [ math.tau ], [ 0 ] ] )

arrays.append( [ 'm1',  0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm2',  0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm3',  0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm4',  0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm5',  0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm6',  0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm7',  0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm8',  0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm9',  0, [ ], [ 0 ],        [ 0 ] ] )

arrays.append( [ 'm10', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm11', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm12', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm13', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm14', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm15', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm16', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm17', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm18', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm19', 0, [ ], [ 0 ],        [ 0 ] ] )

arrays.append( [ 'm20', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm21', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm22', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm23', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm24', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm25', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm26', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm27', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm28', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm29', 0, [ ], [ 0 ],        [ 0 ] ] )

arrays.append( [ 'm30', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm31', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm32', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm33', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm34', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm35', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm36', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm37', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm38', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm39', 0, [ ], [ 0 ],        [ 0 ] ] )

arrays.append( [ 'm40', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm41', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm42', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm43', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm44', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm45', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm46', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm47', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm48', 0, [ ], [ 0 ],        [ 0 ] ] )
arrays.append( [ 'm49', 0, [ ], [ 0 ],        [ 0 ] ] )

arrays.append( [ 'm50', 0, [ ], [ 0 ],        [ 0 ] ] )
#-----------------------------------------------------------------------------
def cexp_parser():
    #                    oper,                         lprio, rprio, eval
    # Assignment.
    register_postsymbol('=',                               5,   4)                      # r to l
    register_postsymbol(',',                               9,   8, comma_eval)          # r to l
    # Cominbation.
    register_postsymbol(['$or$', '$and$'],                10,  11)                      # l to r
    # Logical.
    register_postsymbol(['>','>=','<','<=','==','!='],    20,  21)                      # l to r
    # Bitwise.
    register_postsymbol(['$union$', '$mask$', '$diff$',
                         '$cls$', '$ars$'],               30,  31)                      # l to r
    # Math.
    register_postsymbol(['+', '-'],                       40,  41)                      # l to r
    register_postsymbol('/',                              50,  51)                      # l to r
    register_postsymbol('*',                              60,  61)                      # l to r
    register_postsymbol('**',                             71,  70)                      # r to l
    # Leading + or -.
    register_presymbol(['+', '-'],                        81,  80, unary_eval)          # r to l
    # Parenthesis.
    register_postsymbol('(',                            1000,   1, open_paren_eval)     # l to r
    register_postsymbol(')',                               1, 1000, close_paren_eval)   # r to l
    register_postsymbol('[',                            1000,   1, open_bracket_eval)   # l to r
    register_postsymbol(']',                               1, 1000, close_bracket_eval) # r to l
    register_postsymbol('{',                            1000,   1, open_brace_eval)
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
                line = re.sub('[$][$].*$', '', line)
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
            print('   ', sys.exc_info()[0], sys.exc_info, file=sys.stderr)
        # yrt
        break
    # elihw
    sys.exit(0)
# End of get_line

#-----------------------------------------------------------------------------
# Parse and process line.
def process_line(t, line):
    wline = ''.join(line.split())
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
