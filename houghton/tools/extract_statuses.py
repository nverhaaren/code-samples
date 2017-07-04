#! /usr/bin/python

expect_status = False

while True:
    try:
        line = raw_input()
    except EOFError:
        break

    if line == 'STATUS':
        expect_status = True
        continue

    if expect_status:
        print line

    expect_status = False
