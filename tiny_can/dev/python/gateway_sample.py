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
send_receive_sample V0.02, 21.04.2023 (LGPL)
K.Demlehner (klaus@mhs-elektronik.de)
"""

import os
import sys
import time
import mhsTinyCanDriver as CanDriver
from baseOptionParser import BaseOptionParser
        
def SendMessages(write_device, msgs):        
    for msg in msgs:
         canDriver.TransmitData(write_device, msg.Id, msg.Data[0:], msg.Flags.FlagBits.DLC, msg.Flags.FlagBits.RTR, msg.Flags.FlagBits.EFF);    
        
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------

def ReadAndPrintMessages(read_device, write_device):
    res = canDriver.CanReceive(index = read_device, count = 500)
    if res[0] > 0:
        SendMessages(write_device, res[1])                         
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

print('>>>> Receives data from INPUT Device and transmit <<<<') 
print('>>>>    with different speed to OUTPUT Device     <<<<')

# create the driver
#canDriver = CanDriver.MhsTinyCanDriver(options = {'LogFile':'x:\\tiny_can\dev\\python\\log.txt', 'LogFlags':'0xFFFF'})
canDriver = CanDriver.MhsTinyCanDriver()

# options parser
# --------------
usage = "usage: %prog [options]"
parser = BaseOptionParser(usage, version=VERSION)

parser.add_option("-I", action="store", type="string", dest="snr1", metavar="SNR",
                  help="serial number of the INPUT device", default=None)

parser.add_option("-O", action="store", type="string", dest="snr2", metavar="SNR",
                  help="serial number of the OUTPUT device", default=None)

parser.add_option("-i", action="store", type="int", dest="bitrate1", metavar="KBIT",
                  help="can bitrate (kBit/s) of INPUT device", default=None)

parser.add_option("-o", action="store", type="int", dest="bitrate2", metavar="KBIT",
                  help="can bitrate (kBit/s) of OUTPUT device", default=None)                  

(options, args) = parser.parse_args()

if options.snr1 == None:
    raise NotImplementedError('parameter -I missing')
if options.snr2 == None:
    raise NotImplementedError('parameter -O missing')
if options.bitrate1 == None:
    raise NotImplementedError('parameter -i missing')
if options.bitrate2 == None:
    raise NotImplementedError('parameter -o missing')    
                                                                 
# Create INPUT Device                        
res = canDriver.CanExCreateDevice(options = 'CanRxDFifoSize=16384')
if res[0] >= 0:
    InputDevice = res[1]
else:
    raise NotImplementedError('error create INPUT Device')

# Create OUTPUT Device                        
res = canDriver.CanExCreateDevice(options = 'CanRxDFifoSize=16384')
if res[0] >= 0:
    OutputDevice = res[1]
else:
    raise NotImplementedError('error create OUTPUT Device')                            
                                                                  
# Open INPUT Device
err = canDriver.OpenComplete(index = InputDevice, snr=options.snr1, canSpeed=options.bitrate1)
print(canDriver.FormatError(err, 'OpenComplete'))
if err:            
    sys.exit(0)
    
# Open INPUT Device
err = canDriver.OpenComplete(index = OutputDevice, snr=options.snr2, canSpeed=options.bitrate2)
print(canDriver.FormatError(err, 'OpenComplete'))
if err:            
    sys.exit(0)    

# Read status information
status = canDriver.CanGetDeviceStatus(index = InputDevice)
print(canDriver.FormatCanDeviceStatus(status[1], status[2], status[3]))

while True:
    try:
        time.sleep(0.01)  # NOTE: sleeping time 10 ms !!
        if ReadAndPrintMessages(InputDevice, OutputDevice) < 0:
            break;             
        status = canDriver.CanGetDeviceStatus(InputDevice)
        if status[1] < 8:
            print('INPUT Device disconnect!')
            break;
        status = canDriver.CanGetDeviceStatus(OutputDevice)
        if status[1] < 8:
            print('OUTPUT Device disconnect!')
            break;                
    except KeyboardInterrupt:
        break;
    
# shutdown
canDriver.CanDownDriver()
canDriver.so = None
print('done')                                    
