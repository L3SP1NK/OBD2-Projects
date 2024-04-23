#!/usr/bin/python
# -*- coding: utf-8 -*-

#    send_receive_sample.py - Transmit & Receive CAN-Messages Demo 
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
send_receive_sample V0.01, 03.02.2015 (LGPL)
K.Demlehner (klaus@mhs-elektronik.de)
"""

import os
import sys
import time
import mhsTinyCanDriver as CanDriver
from baseOptionParser import BaseOptionParser

        
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------

def ReadAndPrintMessages(show_timestamp):
    res = canDriver.CanReceive(count = 500)
    if res[0] > 0:                 
        msgs = canDriver.FormatMessages(res[1], showTimestamp=show_timestamp)
        for msg in msgs:
            print(msg)
    else:
        if res[0] < 0:
            print(canDriver.FormatError(res, 'CanReceive'))
            return -1
    return 0
                        
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------


'''Abschnitt Initialisierung'''

print('>>>> SENDEN / EMPFANGEN BEISPIEL <<<<')
# 

# options parser
# --------------
usage = "usage: %prog [options]"
parser = BaseOptionParser(usage, version=VERSION)

parser.add_option("-S", action="store", type="string", dest="snr", metavar="SNR",
                  help="serial number of the device", default=None)

parser.add_option("-b", action="store", type="int", dest="bitrate", metavar="KBIT",
                  help="can bitrate in kBit/s, default = 250", default=250)
                  
parser.add_option("-t", action="store", type="int", dest="timestamp",
                  help="Show time stamp mode, default = 0 (disabled)", default=0)                  
                  
parser.add_option("-x", action="store_true", dest="closeCan", 
                  help="Stops the CAN-Controller when close the interface", default=False)
                  
parser.add_option("-r", action="store_true", dest="openReset", 
                  help="execute a hardware reset when open the interface", default=False)                                    

(options, args) = parser.parse_args()
if options.bitrate:                        
    bitrate = options.bitrate
else:
    bitrate = 250
        
if options.snr:  
    snr = options.snr
else:
    snr = None
       
if options.timestamp:
    show_timestamp = options.timestamp;
else:    
    show_timestamp = 0;  
    
if options.closeCan:
    auto_stop_can = 1
else:    
    auto_stop_can = 0
       
if options.openReset:
    auto_open_reset = 1
else:    
    auto_open_reset = 0
    
# create the driver
canDriver = CanDriver.MhsTinyCanDriver(options={'AutoStopCan':auto_stop_can,'AutoOpenReset':auto_open_reset})

# Open CAN Interface and init
err = canDriver.OpenComplete(snr=snr, canSpeed=bitrate, options={'TimeStampMode':show_timestamp})
print(canDriver.FormatError(err, 'OpenComplete'))
if err:            
    sys.exit(0)

# Read status information
status = canDriver.CanGetDeviceStatus()
print(canDriver.FormatCanDeviceStatus(status[1], status[2], status[3]))

pattern = [0, 0, 0, 0, 0, 0, 0, 0]
count = 0
while True:
    try:
        time.sleep(0.100)
        if count >= 10:
            count = 0
            pattern[0] = pattern[0] + 1
            canDriver.TransmitData(0, msgId = 0x10, msgData = pattern[0:])
        else:
            count = count + 1
        if ReadAndPrintMessages(show_timestamp) < 0:
            break;             
        status = canDriver.CanGetDeviceStatus()
        if status[1] < 8:
            print('Hardware disconnect!')
            break;    
    except KeyboardInterrupt:
        break;
    
# shutdown
canDriver.CanDownDriver()
canDriver.so = None
print('done')                                    
