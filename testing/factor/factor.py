#!/opt/homebrew/bin/python3

from functools import reduce

def factors(n):    
    return set(reduce(list.__add__, 
		([i, n//i] for i in range(1, int(n**0.5) + 1) if n % i == 0)))

for i in range(1, 100):
    print(f'{i}  - {factors(i)}')

quit()

