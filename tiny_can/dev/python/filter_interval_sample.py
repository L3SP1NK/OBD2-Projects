#!/usr/bin/python
# -*- coding: utf-8 -*-

#    filter_interval_sample.py - Filter & Interval CAN-Messages Demo 
# Copyright (C) 2015, Klaus Demlehner <klaus@mhs-elektronik.de>
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

VERSION = \
"""
filter_interval_sample V0.01, 03.02.2015 (LGPL)
K.Demlehner (klaus@mhs-elektronik.de)
"""

import os
import sys
import time
import mhsTinyCanDriver as CanDriver
from baseOptionParser import BaseOptionParser

FILTER1_INDEX = 0x02000001
FILTER2_INDEX = 0x02000002
FILTER3_INDEX = 0x02000003

INTERVAL1_INDEX = 1
INTERVAL2_INDEX = 2

# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------

'''Filter setup'''

def CanFilterSetup():
    # 1. Filter
    #    Software Filter index = FILTER1_INDEX(0x02000001), Single-Frame, Id=0x100, RTR Frames filtern         
    res = canDriver.SetFilter(FILTER1_INDEX, msgId=0x100, rtr=1)
    print(canDriver.FormatError(err, 'SetFilter'))    
    if res < 0:
        return res
      
    # 2. Filter
    #    Software Filter index = FILTER1_INDEX(0x02000002), Bit-Maskiert, Id=0x123 Maske=0x3FF         
    res = canDriver.SetFilter(FILTER2_INDEX, msgId=0x123, msgIdMask=0x3FF)
    print(canDriver.FormatError(err, 'SetFilter'))    
    if res < 0:
        return res
      
    # 3. Filter
    #    Software Filter index = FILTER1_INDEX(0x02000003), Bereich von 0x001 - 0x005 filtern         
    res = canDriver.SetFilter(FILTER3_INDEX, msgIdStart=0x001, msgIdStop=0x005)
    print(canDriver.FormatError(err, 'SetFilter'))    
    if res < 0:
        return res
    return 0

         
''' Interval Setup '''
            
def CanIntervalSetup():
    res = canDriver.SetIntervalMessage(INTERVAL1_INDEX, 0x200, [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77], interval=100)
    print(canDriver.FormatError(err, 'SetIntervalMessage'))    
    if res < 0:
        return res
    res = canDriver.SetIntervalMessage(INTERVAL2_INDEX, 0x201, [ord(x) for x in 'HALLO'], interval=1000)
    print(canDriver.FormatError(err, 'SetIntervalMessage'))    
    if res < 0:
        return res
    return 0
    
def CanIntervalStop():
    canDriver.SetIntervalMessage(INTERVAL1_INDEX, interval=0)
    canDriver.SetIntervalMessage(INTERVAL2_INDEX, interval=0)
    time.sleep(0.5)  
          
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------

'''Filter Nachrichten auslesen und anzeigen'''

def ReadAndPrintMessages():
    res = canDriver.CanReceive(count = 500)
    if res[0] > 0:                 
        msgs = canDriver.FormatMessages(res[1])
        for msg in msgs:
            print(msg)
    else:
        if res[0] < 0:
            print(canDriver.FormatError(res, 'CanReceive'))
            return -1
    return 0
                        
def ReadAndPrintFilter():
    # 1. Filter    
    res = canDriver.CanReceive(FILTER1_INDEX)
    if res[0] > 0:    
        print('Filter 1 Message:')      
        print(canDriver.FormatMessages(res[1]))
    else:
        if res[0] < 0:
            print(canDriver.FormatError(res, 'CanReceive'))
            return -1
    # 2. Filter    
    res = canDriver.CanReceive(FILTER2_INDEX)
    if res[0] > 0:    
        print('Filter 2 Message:')      
        print(canDriver.FormatMessages(res[1]))
    else:
        if res[0] < 0:
            print(canDriver.FormatError(res, 'CanReceive'))
            return -1
    # 3. Filter    
    res = canDriver.CanReceive(FILTER3_INDEX)
    if res[0] > 0:    
        print('Filter 3 Message:')      
        print(canDriver.FormatMessages(res[1]))
    else:
        if res[0] < 0:
            print(canDriver.FormatError(res, 'CanReceive'))
            return -1                        
    return 0            

# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------


'''Abschnitt Initialisierung'''

print('>>>> FILTER BEISPIEL <<<<')

# create the driver
canDriver = CanDriver.MhsTinyCanDriver()

# options parser
# --------------
usage = "usage: %prog [options]"
parser = BaseOptionParser(usage, version=VERSION)

parser.add_option("-S", action="store", type="string", dest="snr", metavar="SNR",
                  help="serial number of the device", default=None)

parser.add_option("-b", action="store", type="int", dest="bitrate", metavar="KBIT",
                  help="can bitrate in kBit/s, default = 250", default=250)

(options, args) = parser.parse_args()
if options.bitrate:                        
    bitrate = options.bitrate
else:
    bitrate = 250
        
if options.snr:  
    snr = options.snr
else:
    snr = None    

# init complete
err = canDriver.OpenComplete(snr=snr, canSpeed=bitrate)
print(canDriver.FormatError(err, 'OpenComplete'))
if err:            
    sys.exit(0)

# Filter Setup
CanFilterSetup()
# Interval Message Setup
CanIntervalSetup()

# device status
status = canDriver.CanGetDeviceStatus()
print(canDriver.FormatCanDeviceStatus(status[1], status[2], status[3]))

try:
    while True:        
        if ReadAndPrintFilter() < 0:
            break;
        if ReadAndPrintMessages() < 0:
            break;             
        status = canDriver.CanGetDeviceStatus()
        if status[1] < 8:
            print('Hardware disconnect!')
            break;    
except KeyboardInterrupt:
    pass

# shutdown
CanIntervalStop()
canDriver.CanDownDriver()
canDriver.so = None
print ('done')                                    
