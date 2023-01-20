#!/usr/bin/python3 -B
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
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
#-----------------------------------------------------------------------------
#-- global zvar
numarry_name = 0            # The name of the variable.
numarry_maclevel = 1        # The macro level was in effect when created.
numarry_indexes = 2         # The array indexes. []=value, [3]=1-dimen, [2,4]=2-dimen.
numarry_values = 3          # Array of values ([0] for not an array).
numarry_value_type = 4      # Array of types None=not-set, 0=int/float, 1= character string
#-- zvar = []

warray = [ 'abc1', 8,
           [ 1, 2 ],
           [ None, 123.45 ],
           [ None, 0 ] ]    # Not-set-yet, int/float
arrays.append(warray)

warray = [ 'abc2', 8,
            [ 1, 2],
            [ "wxyz", 8589934591 ],
            [ 1, 0 ] ]      # Character string, int/float
arrays.append(warray)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
warray = [ 'def1', 8,
           [ 2 ],
           [ None, 123.45 ],
           [ None, 0 ] ]    # Not-set-yet, int/float
arrays.append(warray)

warray = [ 'def2', 8,
            [ 2],
            [ "wxyz", 8589934591 ],
            [ 1, 0 ] ]      # Character string, int/float
arrays.append(warray)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
warray = [ 'aha0', 8,
            [],
            [ 98765.43210 ],
            [ 0 ] ]         # int/float
arrays.append(warray)

warray = [ 'aha1', 8,
           [ 1, 1, 2 ],
           [ None, 123.45 ],
           [ None, 0 ] ]    # Not-set-yet, int/float
arrays.append(warray)

warray = [ 'aha2', 8,
            [ 1, 1, 2],
            [ "wxyz", 8589934591 ],
            [ 1, 0 ] ]      # Character string, int/float
arrays.append(warray)

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
        # fi
        if c == '$':
            if len(string) >= 7 and string[0:7] == '$union$':
                return string[7:], '$union$', 'OPER'
            # fi
            if len(string) >= 6:
                if string[0:6] == '$mask$':
                    return string[6:], '$mask$', 'OPER'
                # fi
                if string[0:6] == '$diff$':
                    return string[6:], '$diff$', 'OPER'
                # fi
            # fi
            if len(string) >= 5:
                if string[0:5] == '$and$':
                    return string[5:], '$and$', 'OPER'
                # fi
                if string[0:5] == '$cls$':
                    return string[5:], '$cls$', 'OPER'
                # fi
                if string[0:5] == '$ars$':
                    return string[5:], '$ars$', 'OPER'
                # fi
            # fi
            if len(string) >= 4:
                if string[0:4] == '$or$':
                    return string[4:], '$or$', 'OPER'
            # fi
            if len(string) > 1:
                return string[1:], c, 'MISMATCH #a len(string)={}'.format(len(string))
            # fi
            return None, c, 'MISMATCH #b len(string)={}'.format(len(string))
        # fi
        if c == '=' and len(string) >= 2:   # Possible ==, =>, =<
            if string[1] == '=':
                return string[2:], '==', 'OPER'
            # fi
            if string[1] == '>':
                return string[2:], '>=', 'OPER'
            # fi
            if string[1] == '<':
                return string[2:], '<=', 'OPER'
            # fi
            return string[1:], '=', 'OPER'
        # fi
        if c == '!' and len(string) >= 2:   # Possible !=
            if string[1] == '=':
                return string[2:], '!=', 'OPER'
            # fi
            return None, c, 'MISMATCH #c len(string)={}'.format(len(string))
        # fi
        if c == '<' and len(string) >= 2:   # Possible <=
            if string[1] == '=':
                return string[2:], '<=', 'OPER'
            # fi
            return string[1:], '<', 'OPER'
        # fi
        if c == '>' and len(string) >= 2:   # Possible <=
            if string[1] == '=':
                return string[2:], '>=', 'OPER'
            # fi
            return string[1:], '>', 'OPER'
        # fi
        # The +-(/= reach below.
        if len(string) > 1:
            return string[1:], c, 'OPER'
        # fi
        return None, c, 'OPER'
    # fi
    if c.isalpha():                         # First character is [a-zA-Z].
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
    # fi
    if c.isdigit() or c == '.':             # First is a digit.
        m = re.match(r'[0-9.]+', string)
        strg = m.group(0)                   # We know there is at least one character.
        if strg.count('.') > 1:             # Only one decimal place in a number.
            return None, strg, 'MISMATCH #d strg={}'.format(strg)
        # fi
        if len(string) > len(strg):         # Something after this alphabetic string.
            return string[len(strg):], strg, 'NUMBER'
        # fi
        return None, strg, 'NUMBER'
    # fi
    if re.match(r'[\[\](){}]', c):          # Known parenthesis/brackets/braces.
        if len(string) > 1:                 # Something after this alphabetic string.
            return string[1:], c, 'SYNTAX'
        # fi
        return None, c, 'SYNTAX'
    # fi

    # comma reached below.
    if len(string) > 1:
        return string[1:], c, 'OPER'
    # fi

    # Everything else is an error.
    return None, c, 'MISMATCH #e c={}'.format(c)
# End of next_token

#-----------------------------------------------------------------------------
def tokenize(code):
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
# End of __repr__

#-----------------------------------------------------------------------------
def identity_eval(args):
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
    #-- print("ERROR - ID type(args[0])={} args[0]='{}'".format(type(args[0]), args[0]))
    return [ "ERROR - ID type(args[0])={} args[0]='{}'".format(type(args[0]), args[0]), None ]
# End of identity_eval

#-----------------------------------------------------------------------------
def get_value(arg):
    global arrays

    a = arg
    if arg[0].startswith('ERROR'):
        return arg
    # fi
    if arg[0] == 'NUMBER':
        arg[1] = float(arg[1])
        return arg
    # fi
    if arg[0] == 'COMMA':
        return arg
    # fi
    if arg[0] == 'CHAR':
        return arg
    # fi
    if arg[1] in variables:
        arg[0] = 'NUMBER'
        arg[1] = variables[arg[1]]
        return arg
    # fi
    if arg[0] == 'ID':
        if arg[1] in local_variables:               # NOTDONEYET - delete after wary is done?
            arg[0] = 'NUMBER'                       # NOTDONEYET - delete after wary is done?
            arg[1] = local_variables[arg[1]]        # NOTDONEYET - delete after wary is done?
            return arg                              # NOTDONEYET - delete after wary is done?
        # fi                                        # NOTDONEYET - delete after wary is done?
        maxmaclev = 0
        maxwary = None
        for wary in arrays:
            if arg[1] == wary[numarry_name]:
                if wary[numarry_maclevel] >  maxmaclev:
                    maxmaclev = wary[numarry_maclevel]
                    maxwary = wary
                # fi
            # fi
        # rof
        if maxwary is not None:
            if maxwary[numarry_indexes] == []:
                if maxwary[numarry_value_type][0] is None:
                    return [ "ERROR - get_value - variable is not set yet - '{}'".format(arg), None ]
                # fi
                if maxwary[numarry_value_type][0] == 0:
                    arg[0] = 'NUMBER'
                else:
                    arg[0] = 'CHAR'                       # NOTDONEYET - probably should be 'CHAR'
                # fi
                arg[1] = maxwary[numarry_values][0]
                return arg
            # fi
            return [ "ERROR - get_value - variable is array '{}' ({})".format(arg,maxwary[numarry_indexes]), None ]
        # fi
    # fi

    #-- print("ERROR - get_value - unrecognized variable='{}'".format(arg))
    return [ "ERROR - get_value - unrecognized variable='{}'".format(arg), None ]
# End of get_value

#-----------------------------------------------------------------------------
def compute_value(op, arg1, arg2):
    global arrays

    a2 = get_value(arg2)
    if a2 is None:
        return [ "ERROR - Argument2 is None a2='{}'".format(a2), None ]
    # fi
    if a2[0].startswith('ERROR'):
        return a2
    # fi
    if a2[0] != 'NUMBER':
        return [ "ERROR - Argument2 is not a Number a2='{}'".format(a2), None ]
    # fi

    if op == '=':
        # NOTDONEYET - arg1 is a list
        if arg1[0] != 'ID':
            return [ "ERROR - Argument1 is not a variable name arg1='{}'".format(arg1), None ]
        # fi
        if len(arg1[1]) > 4 and arg1[1][0:4] == 'zvar':
            zarr = arg1[1][4:]
            if not zarr.isnumeric():
                return [ "ERROR - zvar following ('{}') is not a number".format(zarr), arg1[1] ]
            # fi
            zarr = int(zarr)
            if zarr < 0 or zarr >= len(zvar):
                return [ "ERROR - Argument1 ({}) index number {} not in zvar array.".format(arg1,zarr), None ]
            # fi
            warray = zvar[zarr][0]
            windex = zvar[zarr][1]
            if windex < 0 or windex >= len(warray[numarry_values]):
                return [ "ERROR - Argument1 ({}) index number {} not in zvar array.".format(arg1,zarr), None ]
            # fi
            if warray[numarry_value_type] == 0:
                warray[numarry_values][windex] = arg2[1]
            else:
                if warray[numarry_dimensions] == 0:
                    warray[numarry_value_type] = 0
                    warray[numarry_values][windex] = arg2[1]
                else:
                    return [ "ERROR - Trying to put value({}) into character array({}) - Argument1 ({}).".format(arg2[1], warray[numarry_name], arg1), None ]
                # fi
            # fi
            return arg2
        # fi
        if arg1[1] in variables:
            variables[arg1[1]] = arg2[1]
            return arg2
        # fi
        if arg1[1] in local_variables:
            local_variables[arg1[1]] = arg2[1]
            return arg2
        # fi

        # arrays here.
        maxmaclev = 0
        maxwary = None
        for wary in arrays:
            if arg1[1] == wary[numarry_name]:
                if wary[numarry_maclevel] >  maxmaclev:
                    maxmaclev = wary[numarry_maclevel]
                    maxwary = wary
                # fi
            # fi
        # rof
        if maxwary is not None:
            if maxwary[numarry_indexes] != []:
                return [ "ERROR - compute_value - variable is array '{}'".format(arg), None ]
            # fi
#--            if maxwary[numarry_value_type][0] is None:
#--                return [ "ERROR - compute_value - variable is not set yet - '{}'".format(arg), None ]
#--            # fi
            maxwary[numarry_value_type][0] = 0
            maxwary[numarry_values][0] = arg2[1]
            return arg2
        # fi
        print("Assignment to unknown variable '{}', creating it = '{}'".format(arg1, a2))
        variables[arg1[1]] = arg2[1]
        return arg2
    # fi

    a1 = get_value(arg1)
    if a1 is None:
        return [ "ERROR - Argument1 is None a1='{}'".format(a1), None ]
    # fi
    if a1[0].startswith('ERROR'):
        return a1
    # fi
    if a1[0] != 'NUMBER':
        return [ "ERROR - Argument1 is not a Number a1='{}'".format(a1), None ]
    # fi
    a1 = a1[1]
    a2 = a2[1]

    # Math: +, -, *, /, **
    if op == '+':
        return [ 'NUMBER', a1 + a2 ]
    # fi
    if op == '-':
        return [ 'NUMBER',  a1 - a2 ]
    # fi
    if op == '*':
        return [ 'NUMBER',  a1 * a2 ]
    # fi
    if op == '/':
        return [ 'NUMBER',  a1 / a2 ]
    # fi
    if op == '**':
        return [ 'NUMBER',  a1 ** a2 ]
    # fi

    # Logical: >, >=, <=, <, ==, !=
    if op == '<=':
        return [ 'NUMBER', -1 if a1 <= a2 else 0 ]
    # fi
    if op == '>=':
        return [ 'NUMBER',  -1 if a1 >= a2 else 0 ]
    # fi
    if op == '>':
        return [ 'NUMBER',  -1 if a1 > a2 else 0 ]
    # fi
    if op == '<':
        return [ 'NUMBER',  -1 if a1 < a2 else 0 ]
    # fi
    if op == '==':
        return [ 'NUMBER',  -1 if a1 == a2 else 0 ]
    # fi
    if op == '!=':
        return [ 'NUMBER',  -1 if a1 != a2 else 0 ]
    # fi

    # Bitwise: $cls$, $ars$, $mask$, $union$, $diff$
    if op == '$mask$':
        return [ 'NUMBER',  int(a1) & int(a2) ]
    # fi
    if op == '$union$':
        return [ 'NUMBER',  int(a1) | int(a2) ]
    # fi
    if op == '$cls$':
        return [ 'NUMBER',  int(a1) << int(a2) ]
    # fi
    if op == '$ars$':
        return [ 'NUMBER',  int(a1) >> int(a2) ]
    # fi
    if op == '$diff$':
        return [ 'NUMBER',  int(a1) ^ int(a2) ]             # xor
    # fi

    # combination: $and$, $or$
    if op == '$and$':
        return [ 'NUMBER',  -1 if int(a1) == -1 and int(a2) == -1 else 0 ]
    # fi
    if op == '$or$':
        return [ 'NUMBER',  -1 if int(a1) == -1 or int(a2) == -1 else 0 ]
    # fi

    #-- print("ERROR - unknown operator '{}' '{}' '{}'".format(arg1, op, arg2))
    return [ "ERROR - unknown operator '{}' '{}' '{}'".format(arg1, op, arg2) , None ]
# End of compute_value

#-----------------------------------------------------------------------------
def comma_eval(args):
    if args is None or len(args) != 3:
        return [ "ERROR - comma_eval wrong number of arguments, args={}".format(args), None ]
    # fi
    a0 = args[0]
    a1 = args[1]
    a2 = args[2]
    if type(a0) == SymbolDesc or type(a1) != SymbolDesc or type(a2) == SymbolDesc:
        return [ "ERROR - comma_eval args={}".format(args), None ]
    # fi
    if a0[0] != 'NUMBER':
        return [ "ERROR - comma_eval first argument is not a NUMBER, args={}".format(args), None ]
    if a2[0] == 'NUMBER':
        value = [ 'COMMA', [ int(a0[1]), int(a2[1]) ] ]
    elif a2[0] == 'COMMA':
        value = [ 'COMMA', [ int(a0[1]) ] + a2[1] ]
    # fi
    return [ value ]
# End of comma_eval

#-----------------------------------------------------------------------------
def binary_eval(args):
    if args is None or len(args) != 3:
        return [ "ERROR - binary_eval wrong number of arguments, args={}".format(args), None ]
    # fi
    a0 = args[0]
    a1 = args[1]
    a2 = args[2]
    if type(a0) == SymbolDesc or type(a1) != SymbolDesc or type(a2) == SymbolDesc:
        #-- print("ERROR - binary_eval args={}".format(args))
        return [ "ERROR - binary_eval args={}".format(args), None ]
    # fi
    value = compute_value(a1.symbol, a0, a2)
    return [ value ]
# End of binary_eval

#-----------------------------------------------------------------------------
def unary_eval(args):
    if len(args) != 2:
        #-- print("ERROR - unary_eval args={}".format(args))
        return [ "ERROR - unary_eval args={}".format(args), None ]
    # fi
    if type(args[0]) == SymbolDesc and type(args[1]) != SymbolDesc:
        op = args[0].symbol
        arg1 = get_value(args[1])
        if arg1 is None:
            return [ "ERROR - Argument2 is None arg1='{}'".format(arg1), None ]
        # fi
        if arg1[0].startswith('ERROR'):
            return arg1
        # fi
        if arg1[0] != 'NUMBER':
            return [ "ERROR - Argument2 is not a Number arg1='{}'".format(arg1), None ]
        # fi
        if op == '-':
            arg1[1] = 0 - arg1[1]
        # fi
        return [ arg1 ]
    # fi
    if type(args[0]) != SymbolDesc and type(args[1]) == SymbolDesc:
        #-- print("ERROR - unary_eval post args={}".format(args))
        return [ "ERROR - unary_eval post args={}".format(args), None ]
    # fi
    #-- print("ERROR - unary_eval args={}".format(args))
    return [ "ERROR - unary_eval args={}".format(args), None ]
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
    return SymbolDesc(id, 99999, 100000, identity_eval)
# End of id_symbol

#-----------------------------------------------------------------------------
def evaluate_handle(args):
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

    try:
        cur_token = lexer.__next__()                    # [ kind, item ]
    except StopIteration:
        cur_token = None
    # yrt
# End of advance

#-----------------------------------------------------------------------------
def reset(s):
    global lexer

    lexer = tokenize(s)
    advance()
# End of reset

#-----------------------------------------------------------------------------
def cur_sym(allow_presymbol):
    global postsymbols
    global presymbols
    global cur_token
    
    if cur_token is None:
        return None
    # fi
    if cur_token[0] == 'ID':                            # kind
        var = id_symbol(cur_token)
        return var
    # fi
    if cur_token[0] == 'NUMBER':                        # kind
        return id_symbol(cur_token)
    # fi
    if allow_presymbol and cur_token[1] in presymbols:  # item
        return presymbols[cur_token[1]]
    # fi
    if cur_token[1] in postsymbols:                     # item
        return postsymbols[cur_token[1]]                # item
    # fi
    #-- print("ERROR - Undefined token{}".format(cur_token))
    return [ "ERROR - Undefined token '{}'".format(cur_token), None ]
# End of cur_sym

#-----------------------------------------------------------------------------
def parse_to(prio):
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
        # fi
        if a[0].startswith('ERROR'):
            return a
        # fi
        if a[0] == 'COMMA':
            return a
        # fi
        if a[0] == 'CHAR':
            return a
        # fi
        if a[0] != 'NUMBER':
            return [ "ERROR - value is not a Number a='{}'".format(a), None ]
        # fi
        return a
    elif len(args) == 0:
        return None
    # fi
    #-- print("ERROR - parse_to runs off the end of routine '{}'".format(args))
    return "ERROR - parse_to runs off the end of routine '{}'".format(args)
# End of parse_to

#-----------------------------------------------------------------------------
def parse(s):
    global cur_token

    reset(s)
    try:                                # NOTDONEYET - move to only around specific OOR plaes.
        res = parse_to(0)
    except:
        #-- print("parse - exception")
        return [ None, "error parsing" ]
    # yrt
    if cur_token is not None:
        #-- print("ERROR - remaining input res='{}' cur_token='{}'".format(res, cur_token))
        return [ "ERROR - remaining input res='{}' cur_token='{}'".format(res, cur_token), None ]
    # fi
    return res
# End of parse

#-----------------------------------------------------------------------------
def result_functions(arg1, arg2):
    global functions
    global arrays

    print("result_functions - #1 arg1={} arg2={}".format(arg1,arg2))
    if arg1[0] != 'ID':
        # 'NUMBER' -- implied multiply
        # Do implied multiply
        a1 = get_value(arg1)
        if a1 is None:
            return [ "ERROR - value is None a1='{}'".format(a1), None ]
        # fi
        if a1[0].startswith('ERROR'):
            return a1
        # fi
        if a1[0] != 'NUMBER':
            return [ "ERROR - value is not a Number a1='{}'".format(a1), None ]
        # fi
        a2 = get_value(arg2)
        if a2 is None:
            return [ "ERROR - value is None a2='{}'".format(a2), None ]
        # fi
        if a2[0].startswith('ERROR'):
            return a2
        # fi
        if a2[0] != 'NUMBER':
            return [ "ERROR - value is not a Number a2='{}'".format(a2), None ]
        # fi
        a = [[ 'NUMBER',  a1[1] * a2[1] ]]
        return a
    # fi
    # -- ID --
    if arg2[0] not in ['NUMBER', 'COMMA']:
        print("ERROR - Argument to function/array/implied-multiply is not a number '{}'".format(arg2))
        return [ "ERROR - Argument to function is not a number '{}'".format(arg2), None ]
    # fi

    if arg1[1] in functions:
        try:
            a = [ functions[arg1[1]](arg2[1]) ]
            return a
        except:
            #-- print("ERROR - performing function '{}' with argument '{}'".format(arg1, arg2))
            return [ "ERROR - performing function '{}' with argument '{}'".format(arg1, arg2), None ]
        # yrt
    # fi

    maxmaclev = 0
    maxwary = None
    for wary in arrays:
        if arg1[1] == wary[numarry_name]:
            if wary[numarry_maclevel] > maxmaclev:
                maxmaclev = wary[numarry_maclevel]
                maxwary = wary
            # fi
        # fi
    # rof
    if maxwary is not None:
        print("result_functions - #5b arg1={} arg2={}".format(arg1,arg2))
        print("                       maxwary={}".format(maxwary))
        print("                       maxwary[numarry_name]={}".format(maxwary[numarry_name]))
        print("                       maxwary[numarry_maclevel]={}".format(maxwary[numarry_maclevel]))
        print("                       maxwary[numarry_indexes]={}".format(maxwary[numarry_indexes]))
        print("                       maxwary[numarry_values]={}".format(maxwary[numarry_values]))
        print("                       maxwary[numarry_value_type]={}".format(maxwary[numarry_value_type]))
        a = [ None, None]
        if maxwary[numarry_indexes] == []:
            # NOTDONEYET - implied multiply of variable with following argument.
            print("result_functions - #5c arg1={} arg2={}".format(arg1,arg2))
            if maxwary[numarry_value_type][0] is None:
                return [ "ERROR - get_value - variable is not set yet - '{}'".format(arg1[0]), None ]
            # fi
            print("result_functions - #5d arg1={} arg2={}".format(arg1,arg2))
            print("result_functions - #5e arg1={} arg2={}".format(arg1,arg2))
            if maxwary[numarry_value_type][0] == 0:
                a[0] = 'NUMBER'
            else:
                a[0] = 'CHAR'                       # NOTDONEYET - probably should be 'CHAR'
            # fi
            print("result_functions - #5f arg1={} arg2={}".format(arg1,arg2))
            a[1] = maxwary[numarry_values][0]
            print("result_functions - #5g arg1={} arg2={} a={}".format(arg1,arg2,a))
            return a
        elif len(maxwary[numarry_indexes]) == 1:
            print("result_functions - #5h arg1={} arg2={}".format(arg1,arg2))
            if arg2[0] != 'COMMA' or len(arg2[1]) != 1:
                return [ "ERROR - get_value - variable needs 1 arguments to array '{}' ({})".format(arg,maxwary[numarry_indexes]), None ]
            # fi
            idx = int(arg2[1][0])
            if maxwary[numarry_value_type][idx] is None:
                return [ "ERROR - get_value - array ({}) is not set yet".format(arg2[0]), None ]
            # fi
            if maxwary[numarry_value_type][idx] == 0:
                a[0] = 'NUMBER'
            else:
                a[0] = 'CHAR'                       # NOTDONEYET - probably should be 'CHAR'
            # fi
            a[1] = maxwary[numarry_values][idx]
            return a
        elif len(maxwary[numarry_indexes]) == 2:
            print("result_functions - #5i arg1={} arg2={}".format(arg1,arg2))
            if arg2[0] != 'COMMA' or len(arg2[1]) != 2:
                return [ "ERROR - get_value - variable needs 2 arguments to array '{}' ({})".format(arg,maxwary[numarry_indexes]), None ]
            # fi
            idx = int(arg2[1][0]) + int(arg2[1][1]) * maxwary[numarry_indexes][0]
            if maxwary[numarry_value_type][idx] is None:
                return [ "ERROR - get_value - array ({},{}) is not set yet".format(arg2[0],arg2[1]), None ]
            # fi
            if maxwary[numarry_value_type][idx] == 0:
                a[0] = 'NUMBER'
            else:
                a[0] = 'CHAR'                       # NOTDONEYET - probably should be 'CHAR'
            # fi
            a[1] = maxwary[numarry_values][idx]
            return a
        elif len(maxwary[numarry_indexes]) == 3:
            print("result_functions - #5j arg1={} arg2={}".format(arg1,arg2))
            print("                       arg2[0]={}".format(arg2[0]))
            print("                       arg2[1]={}".format(arg2[1]))
            if arg2[0] != 'COMMA' or type(arg2[1]) is not list or len(arg2[1]) != 3:
                print("arg2[0] not COMMA or len(arg2[1]) != 3")
                if type(arg2[1]) is not list:
                    ix = 1
                else:
                    ix = len(arg2[1])
                # fi
                return [ "ERROR - get_value - variable needs 3 arguments to array '{}' ({}) not {}".format(arg1,maxwary[numarry_indexes],ix), None ]
            # fi
            i1 = int(arg2[1][0]) - 1
            i2 = int(arg2[1][1]) - 1
            i3 = int(arg2[1][2]) - 1
            if i1 < 0 or i1 > (maxwary[numarry_indexes][0] -1):
                return [ "ERROR - get_value - first dimension ({}) out of range ({})".format(i1+1,maxwary[numarry_indexes][0]), None ]
            elif i2 < 0 or i2 > (maxwary[numarry_indexes][1] -1):
                return [ "ERROR - get_value - second dimension ({}) out of range ({})".format(i2+1,maxwary[numarry_indexes][1]), None ]
            elif i3 < 0 or i3 > (maxwary[numarry_indexes][2] -1):
                return [ "ERROR - get_value - second dimension ({}) out of range ({})".format(i3+1,maxwary[numarry_indexes][2]), None ]
            # fi
            mult = maxwary[numarry_indexes][0] * maxwary[numarry_indexes][1]
            idx = i1 + i2 * maxwary[numarry_indexes][0] + i3 * mult
            if maxwary[numarry_value_type][idx] is None:
                return [ "ERROR - get_value - array ({},{},{}) is not set yet".format(i1+1,i2+1,i3+1), None ]
            # fi
            if maxwary[numarry_value_type][idx] == 0:
                a[0] = 'NUMBER'
            else:
                a[0] = 'CHAR'                       # NOTDONEYET - probably should be 'CHAR'
            # fi
            a[1] = maxwary[numarry_values][idx]
            return a
        else:
            return [ "ERROR - get_value - array has too many dimensions '{}'".format(maxwary[numarry_indexes]), None ]
        # fi
        return [ "ERROR - get_value - variable is array '{}' ({})".format(arg,maxwary[numarry_indexes]), None ]
    # fi

    # See if can get_value(arg1[0]) -- if can, then do implied multiply.
    a1 = get_value(arg1)
    if a1 is None:
        return [ "ERROR - value is None a1='{}'".format(a1), None ]
    # fi
    if a1[0].startswith('ERROR'):
        return a1
    # fi
    if a1[0] != 'NUMBER':
        return [ "ERROR - value is not a Number a1='{}'".format(a1), None ]
    # fi
    a2 = get_value(arg2)
    if a2 is None:
        return [ "ERROR - value is None a2='{}'".format(a2), None ]
    # fi
    if a2[0].startswith('ERROR'):
        return a2
    # fi
    if a2[0] != 'NUMBER':
        return [ "ERROR - value is not a Number a2='{}'".format(a2), None ]
    # fi
    if a1 and a2:
        a = [[ 'NUMBER', a1[1] * a2[1] ]]
        return a
    # fi
    #-- print("ERROR - Fetching from unknown function '{}' '{}'".format(arg1, arg2))
    return [[ "ERROR - Fetching from unknown function '{}' '{}'".format(arg1, arg2) , None ]]
# End of result_functions

#-----------------------------------------------------------------------------
def common_grouping_eval(args, txt, open_char, close_char):
    print("common_grouping_eval - len(args)={} args='{}'".format(len(args),args))
    if len(args) == 3:
        print("common_grouping_eval - #3")
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
        print("common_grouping_eval - #4")
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
            # fi
            if a1[0].startswith('ERROR'):
                return a1
            # fi
            if a1[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a1='{}'".format(a1), None ]
            # fi
            a2 = get_value(u)
            if a2 is None:
                return [ "ERROR - value is None a2='{}'".format(a2), None ]
            # fi
            if a2[0].startswith('ERROR'):
                return a2
            # fi
            if a2[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a2='{}'".format(a2), None ]
            # fi
            return [[ 'NUMBER',  a1[1] * a2[1] ]]
        #fi
        print("common_grouping_eval - #4c")
    # fi
    if len(args) == 6:
        print("common_grouping_eval - #6")
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
            # fi
            if a1[0].startswith('ERROR'):
                return a1
            # fi
            if a1[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a1='{}'".format(a1), None ]
            # fi
            a2 = get_value(v)
            if a2 is None:
                return [ "ERROR - value is None a2='{}'".format(a2), None ]
            # fi
            if a2[0].startswith('ERROR'):
                return a2
            # fi
            if a2[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a2='{}'".format(a2), None ]
            # fi
            return [[ 'NUMBER',  a1[1] * a2[1] ]]
        #fi
    # fi
    if len(args) == 7:
        print("common_grouping_eval - #7")
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
            print("common_grouping_eval - before result_functions #2 a1={}".format(a1))
            a1 = result_functions(r, t)
            print("common_grouping_eval - after result_functions #2 a1={}".format(a1))
            if type(a1) == list and len(a1) > 0:
                a1 = a1[0]
            # fi
            if a1 is None:
                return [ "ERROR - value is None a1='{}'".format(a1), None ]
            # fi
            if a1[0].startswith('ERROR'):
                return a1
            # fi
            if a1[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a1='{}'".format(a1), None ]
            # fi
            a2 = get_value(w)
            if a2 is None:
                return [ "ERROR - value is None a2='{}'".format(a2), None ]
            # fi
            if a2[0].startswith('ERROR'):
                return a2
            # fi
            if a2[0] != 'NUMBER':
                return [ "ERROR - value is not a Number a2='{}'".format(a2), None ]
            # fi
            a = [[ 'NUMBER',  a1[1] * a2[1] ]]
            return a
        #fi
    # fi
    #-- print("ERROR - open_{}_eval args='{}'".format(txt, args))
    return [ "ERROR - open_{}_eval args='{}'".format(txt, args), None ]
# End of common_grouping_eval

#-----------------------------------------------------------------------------
def open_paren_eval(args):
    return common_grouping_eval(args, 'paren', '(', ')')
# End of open_paren_eval

#-----------------------------------------------------------------------------
def close_paren_eval(args):
    #-- print("ERROR - close_paren_eval args='{}'".format(args))
    return [ "ERROR - close_paren_eval args='{}'".format(args), None ]
# End of close_paren_eval

#-----------------------------------------------------------------------------
def open_bracket_eval(args):
    return common_grouping_eval(args, 'bracket', '[', ']')
# End of open_bracket_eval

#-----------------------------------------------------------------------------
def close_bracket_eval(args):
    #-- print("ERROR - close_bracket_eval args='{}'".format(args))
    return [ "ERROR - close_bracket_eval args='{}'".format(args), None ]
# End of close_bracket_eval

#-----------------------------------------------------------------------------
def open_brace_eval(args):
    return common_grouping_eval(args, 'brace', '{', '}')
# End of open_brace_eval

#-----------------------------------------------------------------------------
def close_brace_eval(args):
    #-- print("ERROR - close_brace_eval args='{}'".format(args))
    return [ "ERROR - close_brace_eval args='{}'".format(args), None ]
# End of close_brace_eval

#-----------------------------------------------------------------------------
def f_zvar(arg):
    global zvar

    try:
        arg = float(arg)
        arg = int(arg)
    except:
        #-- print("ERROR - argument to array m is not an integer '{}'".format(txt, args))
        return [ "ERROR - argument to array m is not an integer '{}'".format(txt, args), None ]
    # yrt
    if arg < 0 or arg >= len(zvar):
        return [ "ERROR - argument ({}) index number {} not in zvar array.".format(arg1,arg), None ]
    # fi
    warray = zvar[arg][0]
    windex = zvar[arg][1]
    if windex < 0 or windex >= len(warray[numarry_values]):
        return [ "ERROR - Argument ({}) index number {} not in zvar array.".format(arg1,arg), None ]
    # fi
    return [ 'ID', 'zvar' + str(arg) ]
# End of f_zvar

#-----------------------------------------------------------------------------
def f_m(arg):
    try:
        arg = float(arg)
        arg = int(arg)
    except:
        #-- print("ERROR - argument to array m is not an integer '{}'".format(txt, args))
        return [ "ERROR - argument to array m is not an integer '{}'".format(txt, args), None ]
    # yrt
    if arg < 1 or arg > 50:
        return [ "ERROR - argument to array m is out of range 1 to 50 '{}'".format(txt, args), None ]
    # fi
    return [ 'ID', 'm' + str(arg) ]
# End of f_m

#-----------------------------------------------------------------------------
def f_ceil(arg):
    a = float(arg)
    a = math.ceil(a)
    return [ 'NUMBER', a ]
# End of f_ceil

#-----------------------------------------------------------------------------
def f_floor(arg):
    a = float(arg)
    a = math.floor(a)
    return [ 'NUMBER', a ]
# End of f_floor

#-----------------------------------------------------------------------------
def f_freq(arg):
    a = float(arg)
    a = 968000.0 / a
    return [ 'NUMBER', a ]
# End of f_freq

#-----------------------------------------------------------------------------
def f_nearest(arg):
    a = float(arg)
    a = int(round(math.log(a / 27.5) * (12.0 / 0.693147)))
    return [ 'NUMBER', a ]
# End of f_nearest

#-----------------------------------------------------------------------------
def f_abs(arg):
    a = float(arg)
    a = abs(a)
    return [ 'NUMBER', a ]
# End of f_abs

#-----------------------------------------------------------------------------
def f_arctan(arg):
    a = float(arg)
    a = math.atan(a)
    return [ 'NUMBER', a ]
# End of f_arctan

#-----------------------------------------------------------------------------
def f_cos(arg):
    a = float(arg)
    a = math.cos(a)
    return [ 'NUMBER', a ]
# End of f_cos

#-----------------------------------------------------------------------------
def f_exp(arg):
    a = float(arg)
    a = math.exp(a)
    return [ 'NUMBER', a ]
# End of f_exp

#-----------------------------------------------------------------------------
def f_frac(arg):
    a = float(arg)
    a = int(round(a)) - a
    return [ 'NUMBER', a ]
# End of f_frac

#-----------------------------------------------------------------------------
def f_int(arg):
    a = float(arg)
    a = int(a)
    return [ 'NUMBER', a ]
# End of f_int

#-----------------------------------------------------------------------------
def f_log(arg):
    a = float(arg)
    try:
        a = math.log(a, 10)
    except:
        a = 0
    # ytr
    return [ 'NUMBER', a ]
# End of f_log

#-----------------------------------------------------------------------------
def f_ln(arg):
    a = float(arg)
    a = math.log(a)
    return [ 'NUMBER', a ]
# End of f_ln

#-----------------------------------------------------------------------------
def f_round(arg):
    a = float(arg)
    a = int(round(a))
    return [ 'NUMBER', a ]
# End of f_round

#-----------------------------------------------------------------------------
def f_sign(arg):
    a = float(arg)
    if a < 0:
        return [ 'NUMBER', -1 ]
    elif a > 0:
        return [ 'NUMBER', 1 ]
    # fi
    return [ 'NUMBER', 0 ]
# End of f_sign

#-----------------------------------------------------------------------------
def f_sin(arg):
    a = float(arg)
    a = math.sin(a)
    return [ 'NUMBER', a ]
# End of f_sin

#-----------------------------------------------------------------------------
def f_sqrt(arg):
    a = float(arg)
    a = math.sqrt(a)
    return [ 'NUMBER', a ]
# End of f_sqrt

#-----------------------------------------------------------------------------
# Invert logical expression (-1 = true, 0 = false). Make anything non-zero be true.
def f_not(arg):
    a = float(arg)
    if a < 0 or a > 0:
        a = -1
    else:
        a = 0
    # fi
    return [ 'NUMBER', a ]
# End of f_not

#-----------------------------------------------------------------------------
def f_print(arg):
    arg = float(arg)
    print(arg)
    return [ 'NUMBER', arg ]
# End of f_print

#-- #-----------------------------------------------------------------------------
#-- def f_pal(arg):
#--     return [ 'NUMBER', 12345.67890 ]
#-- # End of f_pal
#--
#-----------------------------------------------------------------------------
global functions
functions = {
#...............................................................................
    'm':        f_m,                        # Array of m1, m2, m3, ...
    'zvar':     f_zvar,                     # Complicated array used by IMS.
#...............................................................................
    'abs':      f_abs,
    'arctan':   f_arctan,
    'ceil':     f_ceil,
    'cos':      f_cos,
    'exp':      f_exp,
    'floor':    f_floor,
    'frac':     f_frac,
    'freq':     f_freq,
    'int':      f_int,
    'log':      f_log,
    'ln':       f_ln,
    'nearest':  f_nearest,
    'not':      f_not,
    'print':    f_print,
    'round':    f_round,
    'sign':     f_sign,
    'sin':      f_sin,
    'sqrt':     f_sqrt,
#...............................................................................
#--     'pal':  f_pal,                      # For testing purposes only.
    }

#-----------------------------------------------------------------------------
global local_variables
local_variables = {}

global variables
variables = {
    'pi': math.pi,
    'e': math.e,
    'tau': math.tau,
#   1   .   .   .   2   .   .   .   3   .   .   .   4   .   .   .   5   .   .   .
    'm1': 0,        'm2': 0,        'm3': 0,        'm4': 0,        'm5': 0,
    'm6': 0,        'm7': 0,        'm8': 0,        'm9': 0,        'm10': 0,
    'm11': 0,       'm12': 0,       'm13': 0,       'm14': 0,       'm15': 0,
    'm16': 0,       'm17': 0,       'm18': 0,       'm19': 0,       'm20': 0,
    'm21': 0,       'm22': 0,       'm23': 0,       'm24': 0,       'm25': 0,
    'm26': 0,       'm27': 0,       'm28': 0,       'm29': 0,       'm30': 0,
    'm31': 0,       'm32': 0,       'm33': 0,       'm34': 0,       'm35': 0,
    'm36': 0,       'm37': 0,       'm38': 0,       'm39': 0,       'm40': 0,
    'm41': 0,       'm42': 0,       'm43': 0,       'm44': 0,       'm45': 0,
    'm46': 0,       'm47': 0,       'm48': 0,       'm49': 0,       'm50': 0,
    }

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
# Get a line. Put it in "line" and return it.
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
                #-- print("line='{}'".format(line), file=sys.stderr)
                return line
            # fi
        except EOFError:
            print("Read gave EOF", file=sys.stderr)
        except SystemExit:
            print("Read gave system exit", file=sys.stderr)
        except KeyboardInterrupt:
            print("Read got keyboard interrupt", file=sys.stderr)
        except:
            print("Read got a processing error", file=sys.stderr)
            print("   ", sys.exc_info()[0], sys.exc_info, file=sys.stderr)
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
if __name__ == "__main__":
    main()
    exit(0)
# fi
#-----------------------------------------------------------------------------
# End of file calculate.py
#-----------------------------------------------------------------------------
