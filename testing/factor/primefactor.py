#!/opt/homebrew/bin/python3

import primefac
import sys

#-- n = int( sys.argv[1] )
for n in range(1, 100):
    factors = list( primefac.primefac(n) )
    print(f'{n}  - {",".join(map(str, factors))}')
# rof
quit()

