#! /usr/bin/python

buf = [ '' for i in xrange(512) ]

prefix = None
bufidx = 0

while True:
    try:
        line = raw_input()
    except EOFError:
        break

    line_prefix = line[:2]
    if line_prefix != prefix:
        if prefix:
            out.write('\n'.join(buf))
            out.close()
        out = open(line_prefix, 'wb')
        buf = [ '' for i in xrange(512) ]
        buf[0] = line[2:]
        bufidx = 1
        prefix = line_prefix
    else:
        if bufidx == 512:
            out.write('\n'.join(buf))
            buf = [ '' for i in xrange(512) ]
            bufidx = 0

        buf[bufidx] = line[2:]
        bufidx += 1
