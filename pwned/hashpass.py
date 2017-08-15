#! /usr/bin/python

import hashlib, getpass

print hashlib.sha1(getpass.getpass()).hexdigest().upper()
