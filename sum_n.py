#!/usr/bin/env python
def sum_to_n(n, size, limit=None):
    """Produce all lists of `size` positive integers in decreasing order
    that add up to `n`."""
    if size == 1:
        yield [n]
        return
    if limit is None:
        limit = n
    start = (n + size - 1) // size
    stop = min(limit, n - size + 1) + 1
    for i in range(start, stop):
        #for tail in sum_to_n(n - i, size - 1, i):
        for tail in sum_to_n(n - i, size - 1, i):
            yield [i] + tail

for n in range(29,30):
    for i in range(1, n+1):
        for partition in sum_to_n(n, i):
            print partition


#from itertools import combinations, chain

#def sum_to_n(n):
    #'Generate the series of +ve integer lists which sum to a +ve integer, n.'
    #from operator import sub
    #b, mid, e = [0], list(range(1, n)), [n]
    #splits = (d for i in range(n) for d in combinations(mid, i)) 
    #return (list(map(sub, chain(s, e), chain(b, s))) for s in splits)

#for p in sum_to_n(3):
    #print p
