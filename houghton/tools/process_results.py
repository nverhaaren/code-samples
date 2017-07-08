#! /usr/bin/python

import sys
import csv
import re

if len(sys.argv) > 1:
    infile = open(sys.argv[1], 'r')
    data = infile.read()
    infile.close()
else:
    data = ''
    while True:
        try:
            data += raw_input() + '\n'
        except EOFError:
            break

rows = []
row = []
timestamp = ''

data = data.split('\n')
for line in data:
    if not line:
        continue

    if re.match('^([A-Z][a-z]{2} ){2}[ 123]\d [ 0123]\d:\d{2}:\d{2} UTC 20\d{2}$', line):
        if len(row) > 0:
            rows.append(row)
            row = []
        timestamp = line
        continue
        
    if re.match('\d{4}', line):
        if len(row) > 0:
            rows.append(row)
        row = [ timestamp ]

    row.append(line)

#sys.stderr.write(repr(rows) + '\n')

tagged_rows = []
tags = [ 'timestamp', 'flight', 'STATUS', 'DEPARTURE Scheduled time:', 'DEPARTURE Actual time:',
               'DEPARTURE Actual date:', 'ARRIVAL Scheduled time:',
               'ARRIVAL Actual time:', 'ARRIVAL Actual date:', 'parsed status', 'raw' ]
blank_row = {}
for tag in tags:
    blank_row[tag] = ''

tagged_row = dict(blank_row)

for row in rows:
    tagged_row['timestamp'] = row[0]
    tagged_row['flight'] = row[1]

    uppercase = ''
    colon = ''
    for i in range(2, len(row)):
        if re.match('^[A-Z]+$', row[i]):
            uppercase = row[i]
        elif re.match('^.+:$', row[i]):
            colon = row[i]
        else:
            if colon:
                if uppercase + ' ' + colon in tagged_row:
                    tagged_row[uppercase + ' ' + colon] = row[i]
                else:
                    sys.stderr.write('Found ' + uppercase + ' ' + colon + ', not in tagged_row\n')
            else:
                if uppercase in tagged_row:
                    tagged_row[uppercase] = row[i]
                else:
                    sys.stderr.write('Found ' + uppercase + ', not in tagged_row\n')
            uppercase = ''
            colon = ''

    tagged_row['raw'] = '{' + ','.join(map(lambda s: "'" + s.replace('\'', '\\\'').replace('\\', '\\\\') + "'", row[1:])) + '}'
    if tagged_row['STATUS']:
        tagged_status = tagged_row['STATUS']
        if re.match('Arrived.*Late', tagged_status) or re.match('Delayed.*\(arrived.*late\)', tagged_status):
            parsed_status = 'late'
        elif re.match('Arrived.*Early', tagged_status) or re.match('Delayed.*\(arrived.*early\)', tagged_status):
            parsed_status = 'early'
        elif re.match('Arrived.*On Time', tagged_status) or re.match('Delayed.*\(arrived.*on time\)', tagged_status):
            parsed_status = 'on time'
        elif re.match('Canceled', tagged_status):
            parsed_status = 'canceled'
        elif re.match('Delayed', tagged_status):
            parsed_status = 'delayed'
        elif re.match('In Flight', tagged_status):
            parsed_status = 'in flight'
        else:
            parsed_status = 'unknown'
        tagged_row['parsed status'] = parsed_status
        tagged_rows.append(dict(tagged_row))
    tagged_row = dict(blank_row)

#sys.stderr.write(repr(tagged_rows) + '\n')
#sys.stderr.write(repr(blank_row) + '\n')

outwriter = csv.writer(sys.stdout)

outwriter.writerow(map(lambda s: s.lower().replace(':', ''), tags))

for tagged_row in tagged_rows:
    outrow = []
    for tag in tags:
        outrow.append(tagged_row[tag])
    outwriter.writerow(outrow)
