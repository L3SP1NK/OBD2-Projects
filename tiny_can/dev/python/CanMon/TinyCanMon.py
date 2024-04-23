#!/usr/bin/python
# -*- coding: utf-8 -*-

#       TinyCanMon.py - Python Tiny-CAN Monitor
#  Copyright (C) 2015 - 2020 MHS-Elektronik GmbH & Co. KG 

VERSION = \
"""
TinyCanMon V0.02, 06.06.2020 (LGPL)
K.Demlehner (klaus@mhs-elektronik.de)
"""
import sys
sys.path.append('../.')
import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import GObject
import os
import time
import mhsTinyCanDriver as CanDriver
from baseOptionParser import BaseOptionParser

class TinyCanMon:
    def __init__ ( self ):        
        # read ui File
        self.ui = Gtk.Builder()
        self.ui.add_from_file("TinyCanMon.ui")
        self.MainFrame = self.ui.get_object("CanMonWin") 
        # get widget references
        self.StatusBar = self.ui.get_object("StatusBar")
        self.SBContext = self.StatusBar.get_context_id("common context id")
        self.BnCC = self.ui.get_object("BnCreateClose")

        self.BnScroll = self.ui.get_object("BnScroll")
        self.BnScroll.set_active(0)
        self.Scroll = 0
        
        self.EnID = self.ui.get_object("EnID")
        self.EnDLC = self.ui.get_object("EnDLC")
        self.EnExt = self.ui.get_object("EnExt")
        self.EnRtr = self.ui.get_object("EnRtr")
        self.EnFd = self.ui.get_object("EnFd")
        self.EnBrs = self.ui.get_object("EnBrs")
        
        self.EnD = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        for n in range(64):
            self.EnD[n] = self.ui.get_object('EnD%u' % n)
        
        self.CBBaud = self.ui.get_object("CBBaud")
        self.CBFdBaud = self.ui.get_object("CBFdBaud")
        self.SBCanStatus = self.ui.get_object("SBCanStatus")
        self.SBCanContext = self.SBCanStatus.get_context_id("can context id")
                
        # TreeView MVC Initialization
        self.TV = self.ui.get_object("TreeView")
        # Create a Model for the Data to store
        self.model = Gtk.TreeStore(            
            GObject.TYPE_STRING,
            GObject.TYPE_STRING,
            GObject.TYPE_STRING,
            GObject.TYPE_STRING,
            GObject.TYPE_UINT,
            GObject.TYPE_STRING)
        self.TV.set_model(self.model)
        
        self.Renderer = Gtk.CellRendererText()
        column0 = Gtk.TreeViewColumn(" Dir ", self.Renderer, text=0)
        self.TV.append_column(column0)                
        column1 = Gtk.TreeViewColumn(" FD/BRS ", self.Renderer, text=1)
        self.TV.append_column(column1)
        column2 = Gtk.TreeViewColumn(" Ext/Std   ", self.Renderer, text=2)
        self.TV.append_column(column2)
        column3 = Gtk.TreeViewColumn(" ID                ", self.Renderer, text=3)
        self.TV.append_column(column3)
        column4 = Gtk.TreeViewColumn(" DLC ", self.Renderer, text=4)
        self.TV.append_column(column4)
        column5 = Gtk.TreeViewColumn(" DATA (HEX)", self.Renderer, text=5)
        self.TV.append_column(column5)        
        # can state
        self.CanStarted = 0 


    def show(self):
        self.ui.connect_signals (
        {
         "OnExitApp" : self.OnExitApp,
         "OnBnCreateClose" : self.OnBnCreateClose,
         "OnBnScroll" : self.OnBnScroll,        
         "OnMSend" : self.OnMSend,
         "OnBnCanReset" : self.OnBnCanReset,
         "OnBnInfo" : self.OnBnInfo,
         "OnBnClear" : self.OnBnClear,
         }
        )
        self.CanText("CAN stopped")
        # create the driver
        self.TinyCan = CanDriver.MhsTinyCanDriver(fd_mode=1)
        self.TinyCan.CanSetUpEvents(PnPEventCallbackfunc=PnPEventCallback, StatusEventCallbackfunc=StatusEventCallback, RxEventCallbackfunc=RxEventCallback)
        Gdk.threads_enter()
        Gtk.main()
        Gdk.threads_leave()
        # shutdown
        self.TinyCan.CanSetEvents(0)
        time.sleep(0.5)
        self.TinyCan.CanDownDriver()
        self.TinyCan.so = None
  
  
    def OnBnInfo(self, *args):
        info_ui = Gtk.Builder()
        info_ui.add_from_file("info.ui")
        dialog = info_ui.get_object("InfoDialog");
        response = dialog.run()
        dialog.hide()
        return
  
    
    # all the things to do before exit this application
    def ExitApp(self):           
        Gtk.main_quit()
            
    # remove the old statusbar message and add a new one
    def Text(self, Message ):
        self.StatusBar.pop(self.SBCanContext)
        self.StatusBar.push(self.SBCanContext, Message)
        

    def CanText(self, Message):
        self.SBCanStatus.pop(self.SBContext)
        self.SBCanStatus.push(self.SBContext, Message)
        
        
    # Event handler for the frame window       
    def OnExitApp(self, event, widget_object):
        self.ExitApp()
        return

    def OnBnScroll(self, *args):
        if self.Scroll == 0:
            self.Scroll = 1
        else:
            self.Scroll = 0
        self.BnScroll.set_active(self.Scroll)
        return
              
                           
    def OnBnCanReset(self, *args):
        self.TinyCan.resetCanBus()
        return
        
    def OnBnCreateClose(self, *args):    
        if self.BnCC.get_active():
            self.BnCC.set_label('Disconnect');
            bitrate = self.TinyCan.canBitrates[self.CBBaud.get_active()]
            fd_bitrate = self.TinyCan.canFdBitrates[self.CBFdBaud.get_active()]
            err = self.TinyCan.OpenComplete(canSpeed=bitrate, canDSpeed=fd_bitrate, txAck=1)
            if err < 0:
                self.CanText('Device Error')
                self.Text(self.TinyCan.FormatError(err, 'Fehler:'))
            else:
                self.CanText('Device Open')
        else:
            self.BnCC.set_label('Connect');
            self.TinyCan.CanDeviceClose();
            self.CanText('Device Close')
            self.Text('')                        
        return


    def OnBnClear(self, *args):
        self.model.clear()
        return

    # be aware that any changes in the definition above must also be
    # put in the following definition
    def OnMSend(self, *args):
        # check the values if valid
        id = 0
        try:
            id = int (self.EnID.get_text(), 16 )
        except:
            self.Text ( "Invalid ID" )
            return
        length = 0
        try:
            length = int(self.EnDLC.get_text(), 10)
        except:
            self.Text ( "Invalid Data length" )
            return        
        if length > 64 or length < 0:
            self.Text ( "Invalid Data length" )
            return        
        if self.EnExt.get_active():
            m_eff = 1
        else:
            m_eff = 0;
        if self.EnBrs.get_active():
            m_brs = 1
        else:
            m_brs = 0;
        if self.EnFd.get_active():
            m_fd = 1
        else:
            m_fd = 0;
        if self.EnRtr.get_active():
            m_rtr = 1
        else:
            m_rtr = 0;                            
                
        D = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        for n in range(length):
            try:
               D[n] = int(self.EnD[n].get_text(), 16)
            except:
               D[n] = 0                             
        self.TinyCan.TransmitFdData(0, msgId = id, msgLen = length, msgData = D[0:], rtr = m_rtr, eff = m_eff, fd = m_fd, brs = m_brs)
        return
            

    def FormatFdMessages(self, messages=None):                  
        formatedMessages = []
        if messages:
            for message in messages:
                if message.Flags.FlagBits.RTR and message.Flags.FlagBits.EFF:
                    frame_format = 'EFF/RTR'
                else: 
                    if message.Flags.FlagBits.EFF:
                        frame_format = 'EFF'
                    else:
                        if message.Flags.FlagBits.RTR:
                            frame_format = 'STD/RTR'
                        else:
                            frame_format = 'STD'
                if message.Flags.FlagBits.TxD:
                    dir = 'TX'
                else:
                    dir = 'RX'
                if message.Flags.FlagBits.FD and message.Flags.FlagBits.BRS:
                    fd_brs = 'FD/BRS'
                else:
                    if message.Flags.FlagBits.FD:     
                        fd_brs = 'FD'
                    else:
                        fd_brs = ''                        
                length = message.Flags.FlagBits.Length;
                if message.Flags.FlagBits.EFF:
                    id_str = '%08X' % message.Id 
                else:    
                    id_str = '%03X' % message.Id
                msg_data = ""
                if not message.Flags.FlagBits.RTR:
                    chars = 0;                                    
                    for n in range(length):
                        if chars == 8:
                            chars = 1;
                            msg_data = msg_data + '\n'
                        else:
                            chars += 1  
                        msg_data = msg_data + '%02X ' % message.Data[n]
                                            
                row = self.model.append(None)
                self.model.set_value(row, 0, dir)
                self.model.set_value(row, 1, fd_brs)
                self.model.set_value(row, 2, frame_format)                
                self.model.set_value(row, 3, id_str)                  
                self.model.set_value(row, 4, length)
                self.model.set_value(row, 5, msg_data)
                if self.Scroll:
                    path = self.model.get_path(row)
                    self.TV.scroll_to_cell (path, None, False, 0.0, 0.0)
        return                 

# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------

    def ReadAndPrintMessages(self):
        res = self.TinyCan.CanFdReceive(count = 500)
        if res[0] > 0:                 
            self.FormatFdMessages(res[1])              
        else:
            if res[0] < 0:
                self.Text(self.TinyCan.FormatError(res, 'CanReceive'))
                return -1
        return 0 

# --------------------------------------------------------------------------------------------------------
# ----------------------- Event Callbacks ----------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------

def PnPEventCallback(index, status):
    """
    Plug & Play Callback
    status: simple Flag, 0 = disconnect, 1 = connect  
    """    
    Gdk.threads_enter()
    if status:            
        CanMon.CanText('Device Connected')           
    else:
        CanMon.CanText('Device Disconnected')
        CanMon.BnCC.set_active(0)
    Gdk.threads_leave()    
    return


def StatusEventCallback(index, deviceStatusPointer):
    """
    Status Callback
    @param deviceStatusPointer: Pointer to Device Status  
    """
    Gdk.threads_enter()
    deviceStatus = deviceStatusPointer.contents
    CanMon.Text(CanMon.TinyCan.FormatCanDeviceStatus(deviceStatus.DrvStatus,deviceStatus.CanStatus,deviceStatus.FifoStatus))
    Gdk.threads_leave()
    return


def RxEventCallback(index, DummyPointer, count):
    """
    CAN Receive Callback 
    DummyPointer: is set to NULL
    count: Number of Messages
    """     
    Gdk.threads_enter()   
    CanMon.ReadAndPrintMessages()
    Gdk.threads_leave()
    return

# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------
Gdk.threads_init()
CanMon = TinyCanMon()
CanMon.show()



