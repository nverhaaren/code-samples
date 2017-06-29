#! /usr/bin/python

import sys
import csv

if len(sys.argv) > 1:
    infile = open(ays.argv[1], 'r')
    data = infile.read()
    infile.close()
else:
    data = ''
    while True:
        try:
            data += raw_input() + '\n'
        except EOFError:
            break

data = data.split('\n')
for line in data:
    if not line:
        continue
