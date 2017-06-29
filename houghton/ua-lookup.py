#! /usr/bin/python

# This script takes a united airlines flight number as an argument, then looks up yesterday's
# status for the flight, saving the response in a file and printing whatever information
# was parsed from that response to stdout.

import requests
import sys
import datetime

from HTMLParser import HTMLParser

flight = sys.argv[1]

yesterday = datetime.date.today() - datetime.timedelta(1)
datestr = ("%.2d/%.2d/%.4d" % (yesterday.month, yesterday.day, yesterday.year)) + ' 00:00:00'
outpath = "/home/nverhaaren/houghton/%.2d-%.2d-%.4d_%s.html" % (yesterday.day, yesterday.month, yesterday.year, flight)
outfile = open(outpath, 'w')

# These parameters are determined by examining what happens when you use their webpage
# to check flight status.
r = requests.get('https://www.united.com/ual/en/us/travel/flight/getstatus',
                 params={ 'FlightNumber' : str(flight), 'FlightDate' : datestr,
                          'ShowSearchForm' : 'False', 'HasTokenException' : 'False',
                          'fromList' : 'False', 'forRefreshStatus' : 'False' })
if r.status_code != 200:
    print 'Received status code', r.status_code, 'from United'
    sys.exit()
#print r.status_code
#print
html = r.text
outfile.write(html.encode('utf-8'))
#print html

class ParseStatus(HTMLParser):
    def __init__(self):
        HTMLParser.__init__(self)
        self.__mode = None
        self.__nesting = 0
        self.__flag = False
    def handle_starttag(self, tag, attrs):
        if tag == 'div':
            if self.__mode != None:
                self.__nesting += 1
            elif ('class', 'fs-status-msg-refresh') in attrs:
                self.__mode = 'STATUS'
            elif ('class', 'fs-departure') in attrs:
                self.__mode = 'DEPARTURE'
            elif ('class', 'fs-arrival') in attrs:
                self.__mode = 'ARRIVAL'

    def handle_endtag(self, tag):
        if tag == 'div':
            if self.__mode != None:
                if self.__nesting == 0:
                    self.__mode = None
                    self.__flag = False
                else:
                    self.__nesting -= 1
    def handle_data(self, data):
        text = data.strip()
        if self.__mode == 'STATUS':
            if text != '' and text != 'Refresh status':
                print 'STATUS'
                print text
        elif self.__mode == 'DEPARTURE' or self.__mode == 'ARRIVAL':
            if 'time:' in text or 'date:' in text:
                print self.__mode
                print text
                self.__flag = True
            elif self.__flag:
                print text
                self.__flag = False

parser = ParseStatus()
parser.feed(html)

