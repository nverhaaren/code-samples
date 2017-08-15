#! /usr/bin/python

import hashlib, getpass
import sys

if len(sys.argv) > 1 and sys.argv[1] == '-s':
    sha = True
else:
    sha = False

while True:
    try:
        passhash = hashlib.sha1(getpass.getpass()).hexdigest().upper()
        if sha:
            print passhash
    except EOFError:
        print
        break
        
    filename = passhash[:2]
    comp = passhash[2:]
    
    with open(filename, 'r') as f:
        while True:
            a = f.readline()
            if a.strip() == comp:
                print "Found"
                break
            elif not a:
                print "Not found"
                break
    print
