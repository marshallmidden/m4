# ----------------------------------------------------------------------------
# File: print_recursive_classes.py
# ----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
# ----------------------------------------------------------------------------
import sys
global visited
# ----------------------------------------------------------------------------
def print_vars(var, indent=0):
    global visited

    if indent == 0:
        visited = set()
    # fi
    indent_str = ' ' * indent
    if isinstance(var, (int, float, str, bool, type(None))):
        print(f"{indent_str}{var!r} ({type(var).__name__})", file=sys.stderr, flush=True)
    elif isinstance(var, tuple):
        print(f"{indent_str}Tuple ({len(var)} elements):", file=sys.stderr, flush=True)
        for i, item in enumerate(var):
            print(f"{indent_str}  Element {i}:", file=sys.stderr, flush=True)
            print_vars(item, indent + 4)
        # rof
    elif isinstance(var, list):
        print(f"{indent_str}List ({len(var)} elements):", file=sys.stderr, flush=True)
        for i, item in enumerate(var):
            print(f"{indent_str}  Element {i}:", file=sys.stderr, flush=True)
            print_vars(item, indent + 4)
        # rof
    elif isinstance(var, dict):
        print(f"{indent_str}Dictionary ({len(var)} items):", file=sys.stderr, flush=True)
        for key, value in var.items():
            print(f"{indent_str}  Key {key!r}:", file=sys.stderr, flush=True)
            print_vars(value, indent + 4)
        # rof
    elif hasattr(var, '__dict__'):
        # For objects with a __dict__ attribute (usually custom classes), print attributes
        if id(var) in visited:
            print(f"{indent_str}Already visited {var.__class__.__name__} (id={id(var)})", file=sys.stderr, flush=True)
            return
        # fi
        visited.add(id(var))
        print(f"{indent_str}Object {var.__class__.__name__} (id={id(var)}):", file=sys.stderr, flush=True)
        for attr, value in var.__dict__.items():
            if value is None or isinstance(value, (int, float, str, bool, type(None))):
                print(f"{indent_str}  Attribute {attr!r}: {value}", file=sys.stderr, flush=True)
            elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, dict) or hasattr(value, '__dict__'):
                print(f"{indent_str}  Attribute {attr!r}:", file=sys.stderr, flush=True)
                print_vars(value, indent + 4)
            else:
                print(f"{indent_str}  Attribute {attr!r}: Unrecognized type {type(value).__name__}", file=sys.stderr, flush=True)
            # fi
        # rof
    else:
        # For other types, just print the type name
        print(f"{indent_str}Unrecognized type: {type(var).__name__}", file=sys.stderr, flush=True)
    # fi

# ----------------------------------------------------------------------------
