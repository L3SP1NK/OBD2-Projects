#!/usr/bin/python
# -*- coding: utf-8 -*-
            
#                            gateway_sample.py  
#                Receives data from INPUT Device and transmit 
#                    with different speed to OUTPUT Device
#  
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
multi_send_receive_sample V0.02, 21.04.2023
K.Demlehner (klaus@mhs-elektronik.de)
"""

import os
import sys
import time
import mhsTinyCanDriver as CanDriver
from baseOptionParser import BaseOptionParser

INTERVAL1_SUB_INDEX = 1
INTERVAL2_SUB_INDEX = 2
        
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
        
''' Interval Setup '''           
def CanIntervalSetup(device, id1, data1, id2, data2):
    res = canDriver.SetIntervalMessage(device + INTERVAL1_SUB_INDEX, id1, data1, interval=100)
    print(canDriver.FormatError(err, 'SetIntervalMessage'))    
    if res < 0:
        return res
    res = canDriver.SetIntervalMessage(device + INTERVAL2_SUB_INDEX, id2, data2, interval=1000)
    print(canDriver.FormatError(err, 'SetIntervalMessage'))    
    if res < 0:
        return res
    return 0

    
def CanIntervalStop(device):
    canDriver.SetIntervalMessage(device + INTERVAL1_SUB_INDEX, interval=0)
    canDriver.SetIntervalMessage(device + INTERVAL2_SUB_INDEX, interval=0)
    time.sleep(0.5)                 

        
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------

def ReadAndPrintMessages(device):
    res = canDriver.CanReceive(index = device, count = 500)
    if res[0] > 0:
        print('Dev:', hex(device))             
        msgs = canDriver.FormatMessages(res[1])
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

print('>>>>   Open 2 CAN Devices   <<<<') 
print('>>>> send and receive Data  <<<<')

# create the driver
#canDriver = CanDriver.MhsTinyCanDriver(options = {'LogFile':'x:\\tiny_can\dev\\python\\log.txt', 'LogFlags':'0xFFFF'})
canDriver = CanDriver.MhsTinyCanDriver()

# options parser
# --------------
usage = "usage: %prog [options]"
parser = BaseOptionParser(usage, version=VERSION)

parser.add_option("-a", action="store", type="string", dest="snrA", metavar="SNR",
                  help="serial number for CAN device A", default=None)

parser.add_option("-b", action="store", type="string", dest="snrB", metavar="SNR",
                  help="serial number for CAN device B", default=None)

parser.add_option("-A", action="store", type="int", dest="bitrateA", metavar="KBIT",
                  help="can bitrate (kBit/s) for device A", default=100)

parser.add_option("-B", action="store", type="int", dest="bitrateB", metavar="KBIT",
                  help="can bitrate (kBit/s) for device B", default=100)                  

(options, args) = parser.parse_args()

if options.snrA == None:
    raise NotImplementedError('parameter -a missing')
if options.snrB == None:
    raise NotImplementedError('parameter -b missing')
    
                                                                 
# Create Device A                        
res = canDriver.CanExCreateDevice(options = 'CanRxDFifoSize=16384')
if res[0] >= 0:
    DeviceA = res[1]
else:
    raise NotImplementedError('error create Device A')

# Create Device B                       
res = canDriver.CanExCreateDevice(options = 'CanRxDFifoSize=16384')
if res[0] >= 0:
    DeviceB = res[1]
else:
    raise NotImplementedError('error create Device B')                            
                                                                  
# Open Device A
err = canDriver.OpenComplete(index = DeviceA, snr=options.snrA, canSpeed=options.bitrateA)
print(canDriver.FormatError(err, 'OpenComplete'))
if err:            
    sys.exit(0)
    
# Open Device B
err = canDriver.OpenComplete(index = DeviceB, snr=options.snrB, canSpeed=options.bitrateB)
print(canDriver.FormatError(err, 'OpenComplete'))
if err:            
    sys.exit(0)    

# Read status information
status = canDriver.CanGetDeviceStatus(index = DeviceA)
print('Device A:' + canDriver.FormatCanDeviceStatus(status[1], status[2], status[3]))

status = canDriver.CanGetDeviceStatus(index = DeviceB)
print('Device B:' + canDriver.FormatCanDeviceStatus(status[1], status[2], status[3]))

# Setup intervall transmit buffer
CanIntervalSetup(DeviceA, 0x100, [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77], 0x101, [0x01, 0x00])
CanIntervalSetup(DeviceB, 0x200, [0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00], 0x201, [0x02, 0x00])

pattern = [0, 0, 0, 0, 0, 0, 0, 0]
count = 0
while True:
    try:
        time.sleep(0.01)  # NOTE: sleeping time 10 ms !!
        # transmit CAN Messages 
        if count >= 100:
            count = 0
            pattern[0] = pattern[0] + 1
            canDriver.TransmitData(DeviceA, msgId = 0x10, msgData = pattern[0:])
            canDriver.TransmitData(DeviceB, msgId = 0x20, msgData = pattern[0:])
        else:
            count = count + 1
        
        # read CAN Messages from DeviceA & B
        if ReadAndPrintMessages(DeviceA) < 0:
            break; 
        if ReadAndPrintMessages(DeviceB) < 0:
            break;
        # check disconnected hardware                     
        status = canDriver.CanGetDeviceStatus(DeviceA)
        if status[1] < 8:
            print('Device A disconnect!')
            break;
        status = canDriver.CanGetDeviceStatus(DeviceB)
        if status[1] < 8:
            print('Device B disconnect!')
            break;                
    except KeyboardInterrupt:
        break;
    
# shutdown
CanIntervalStop(DeviceA);
CanIntervalStop(DeviceB);

canDriver.CanDownDriver()
canDriver.so = None
print('done')                                    
