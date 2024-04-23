#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2009, Rene Maurer <rene@cumparsita.ch>
# Copyright (C) 2009, omnitron.ch/allevents.ch
#
# Time-stamp: <2009-08-29 21:04:08 rene>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see http://www.gnu.org/licenses.
#
# Description
#     This class extends the standard OptionParser class with logging
#     options.
#
# Usage
#     see __main__
#
# ---------------------------------------------------------------------- 

VERSION = 'BaseOptionParser V0.15, 03.11.2008'

import os
from optparse import OptionParser
import uselogging
logger = uselogging.getLogger()


# --------------------------------------------------------------------
# --------------------------------------------------------------------
# --------------------------------------------------------------------
class BaseOptionParser(OptionParser):

    if uselogging.loggingAvailable:
        import logging
        debugLevels = {
                'debug'   : logging.DEBUG,
                'info'    : logging.INFO,
                'warning' : logging.WARNING,
                'error'   : logging.ERROR,
                'critical': logging.CRITICAL,
                'applic'  : uselogging.APPLIC,
                'disable' : uselogging.DISABLE }
    else:
        debugLevels = {
                'debug'   : 0,
                'info'    : 0,
                'warning' : 0,
                'error'   : 0,
                'critical': 0,
                'applic'  : 0,
                'disable' : 0 }


    def __init__(self, usage, version,   \
                 configBaseLogging=True, addSimulate=True, addPollingTime=False):

        # TODO search a good looking soution
        usage = usage.replace('%prog', os.linesep + '  python %prog')
        OptionParser.__init__(self, usage, version=version)
        self.consoleLogFormat = None
        self.fileLogFormat = None
        if not uselogging.loggingAvailable:
            configBaseLogging = False
        self.configBaseLogging = configBaseLogging

        if self.configBaseLogging:

            self.add_option("-c", action="store_true",
                            dest="consoleLog", help="debug outputs to console, default=disabled", default=False)

            self.add_option("-C", action="store", type="string", dest="consoleLogLevel", metavar="LEVEL",
                            help="console log level, default=debug", default="debug")

            self.add_option("-f", action="store_true",
                            dest="fileLog", help="debug outputs to file, default=disabled", default=False)

            self.add_option("-F", action="store", type="string", dest="fileLogLevel", metavar="LEVEL",
                            help="file log level, default=debug", default="debug")

            self.add_option("-Z", action="store", type="string", dest="logFile", metavar="LOGFILE",
                            help="log file, default=debug", default="debug")

        if addSimulate:
            self.add_option("-s", action="store_true", dest="simulate",
                          help="simulate mode, default=disable", default=False)

        if addPollingTime:
            self.add_option("-T", action="store", type="float", dest="polltime", metavar="TIME",
                          help="polling time in s, default=1.0", default=1.0)


    def parse_args(self):
        (options, args) = OptionParser.parse_args(self)

        if self.configBaseLogging:

            if options.consoleLog:
                level = options.consoleLogLevel.lower()
                if level not in self.debugLevels:
                    self.error("console log level not valid")
                uselogging.enableConsoleLogging(self.debugLevels[level], format=self.consoleLogFormat)
                

            if options.fileLog:
                level = options.fileLogLevel.lower()
                if level not in self.debugLevels:
                    self.error("file log level not valid")
                uselogging.enableFileLogging(fileNamePath=options.logFile, level=self.debugLevels[level], format=self.fileLogFormat)

        return (options, args)


if __name__ == '__main__':

    usage = "usage: %prog [options]"
    parser = BaseOptionParser(usage, VERSION, configBaseLogging=True)

    (options, args) = parser.parse_args()
    print ('numargs', len(args))
    print (options.consoleLog)
    print (options.consoleLogLevel)
    print (options.fileLog)
    print (options.fileLogLevel)
    
