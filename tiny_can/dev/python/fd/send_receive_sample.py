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
send_receive_sample V0.01, 06.06.2020 (LGPL)
K.Demlehner (klaus@mhs-elektronik.de)
"""

import sys
sys.path.append('../.')
import os
import time
import mhsTinyCanDriver as CanDriver
from baseOptionParser import BaseOptionParser

        
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------

def ReadAndPrintMessages():
    res = canDriver.CanFdReceive(count = 500)
    if res[0] > 0:                 
        msgs = canDriver.FormatFdMessages(res[1])
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

# create the driver
canDriver = CanDriver.MhsTinyCanDriver(fd_mode=1)

# options parser
# --------------
usage = "usage: %prog [options]"
parser = BaseOptionParser(usage, version=VERSION)

parser.add_option("-S", action="store", type="string", dest="snr", metavar="SNR",
                  help="serial number of the device", default=None)

parser.add_option("-b", action="store", type="int", dest="bitrate", metavar="KBIT",
                  help="can bitrate in kBit/s, default = 250", default=250)

parser.add_option("-d", action="store", type="int", dest="d_bitrate", metavar="KBIT",
                  help="can data bitrate in kBit/s, default = 1000", default=1000)
                  
(options, args) = parser.parse_args()
if options.bitrate:                        
    bitrate = options.bitrate
else:
    bitrate = 250
    
if options.d_bitrate:                        
    d_bitrate = options.d_bitrate
else:
    d_bitrate = 1000    
        
if options.snr:  
    snr = options.snr
else:
    snr = None    

# Open CAN Interface and init
err = canDriver.OpenComplete(snr=snr, canSpeed=bitrate, canDSpeed=d_bitrate)
print(canDriver.FormatError(err, 'OpenComplete'))
if err:            
    sys.exit(0)

# Read status information
status = canDriver.CanGetDeviceStatus()
print(canDriver.FormatCanDeviceStatus(status[1], status[2], status[3]))

pattern = [0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08, \
           0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18, \
           0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28, \
           0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38, \
           0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48, \
           0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58, \
           0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68, \
           0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78]
count = 0                             
while True:
    try:
        time.sleep(0.100)
        if count >= 10:
            count = 0
            pattern[0] = pattern[0] + 1
            canDriver.TransmitFdData(0, msgId = 0x10, msgData = pattern[0:], rtr = 0, eff = 0, fd = 1, brs = 1)
        else:
            count = count + 1
        if ReadAndPrintMessages() < 0:
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
