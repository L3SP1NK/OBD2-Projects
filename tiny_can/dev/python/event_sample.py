#!/usr/bin/python
# -*- coding: utf-8 -*-

#        event_sample.py - Event Callback Function demo 
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
event_sample V0.01, 03.02.2015 (LGPL)
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


# --------------------------------------------------------------------------------------------------------
# ----------------------- Event Callbacks ----------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------

def PnPEventCallback(index, status):
    """
    Simple Plug and Play Event Handler to open the device again after reconnection and Print to STOUT
    @param index: Struct commonly used by the Tiny Can API
    @param status: simple Flag, 0 = disconnect, 1 = connect  
    @return: Nothing
    @author: Patrick Menschel (menschel.p@posteo.de)
    """    
    if status:
        canDriver.CanDeviceOpen(index)
        canDriver.startCanBus(index)            
        print('Device Connected')           
    else:
        print('Device Disconnected')
    return

def StatusEventCallback(index, deviceStatusPointer):
    """
    Simple Callback for Status Change Event, Just Print Status to STDOUT
    @param index: Struct commonly used by the Tiny Can API
    @param deviceStatusPointer: Pointer to Device Status  
    @return: Nothing
    @author: Patrick Menschel (menschel.p@posteo.de)
    """
    deviceStatus = deviceStatusPointer.contents
    print(canDriver.FormatCanDeviceStatus(deviceStatus.DrvStatus,deviceStatus.CanStatus,deviceStatus.FifoStatus))
    return

def RxEventCallback(index, DummyPointer, count):
    """
    Simple Callback for CAN Rx Event, Just Print Message to STDOUT
    @param index: Struct commonly used by the Tiny Can API
    @param DummyPointer: is set to NULL
    @param count: Number of Messages  
    @return: Nothing
    @author: Patrick Menschel (menschel.p@posteo.de)
    @todo: Check for counts > 1, Implement it, are they possible ?? 
    """        
    print("RxEvent from Index {0}".format(index))
    ReadAndPrintMessages()
    return

# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------


'''Abschnitt Initialisierung'''

print('>>>> EVENT CALLBACK BEISPIEL <<<<')

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

# Open CAN Interface and init
err = canDriver.OpenComplete(snr=snr, canSpeed=bitrate)
print(canDriver.FormatError(err, 'OpenComplete'))
if err:            
    sys.exit(0)
canDriver.CanSetUpEvents(PnPEventCallbackfunc=PnPEventCallback, StatusEventCallbackfunc=StatusEventCallback, RxEventCallbackfunc=RxEventCallback)    

# Read status information
status = canDriver.CanGetDeviceStatus()
print(canDriver.FormatCanDeviceStatus(status[1], status[2], status[3]))

try:
    while True:        
         time.sleep(0.100)        
except KeyboardInterrupt:
    pass
    
# shutdown
canDriver.CanSetEvents(0)
time.sleep(0.5)
canDriver.CanDownDriver()
canDriver.so = None
print('done')
