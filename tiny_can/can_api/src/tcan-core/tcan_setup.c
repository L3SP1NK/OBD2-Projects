/***************************************************************************
                         tcan_setup.c  -  description
                             -------------------
    begin             : 10.05.2017
    last modify       : 08.07.2022
    copyright         : (C) 2017 - 2022 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/

/**
    Library to talk to Tiny-CAN devices. You find the latest versions at
       http://www.tiny-can.com/
**/
#include "global.h"
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "errors.h"
#include "log.h"
#include "info.h"
#include "can_core.h"
#include "can_puffer.h"
#include "mhs_obj.h"
#include "var_obj.h"
#include "tar_drv.h"
#include "tar_drv2.h"
#include "fd_speeds.h"
#include "ecu_flash.h"
#include "tcan_setup.h"



#define CAN_VAR_SPEED_FLAG             0x0001
#define CAN_VAR_SPEED_BTR_FLAG         0x0002
#define CAN_VAR_TX_ACK_ENABLE_FLAG     0x0004
#define CAN_VAR_ERROR_LOG_ENABLE_FLAG  0x0008
#define CAN_VAR_TIMESTAMP_ENABLE_FLAG  0x0010
#define CAN_VAR_SPEED_DBTR_FLAG        0x0020                                      
#define CAN_VAR_FD_FLAGS_FLAG          0x0040
#define CAN_VAR_FIFO_OV_MODE_FLAG      0x0080  
#define CAN_VAR_SET_ECU_FLASH          0x0100


static void SetupLocal(TCanDevice *dev) 
{  
TMhsObjContext *context, *main_context;

main_context = can_main_get_context();
context = dev->Context;
// **** Alle Setup Variablen setzen
dev->OvMode = mhs_value_get_as_uword("FifoOvMode", ACCESS_INIT, main_context);  
if (mhs_value_get_status("FilterReadIntervall", context) & MHS_VAL_FLAG_WRITE)
  dev->FilterReadIntervall = mhs_value_get_as_ubyte("FilterReadIntervall", ACCESS_INIT, context);
else
  dev->FilterReadIntervall = mhs_value_get_as_ubyte("FilterReadIntervall", ACCESS_INIT, main_context);
if (mhs_value_get_status("HighPollIntervall", context) & MHS_VAL_FLAG_WRITE)
  dev->HighPollIntervall = mhs_value_get_as_ubyte("HighPollIntervall", ACCESS_INIT, context);
else
  dev->HighPollIntervall = mhs_value_get_as_ubyte("HighPollIntervall", ACCESS_INIT, main_context);
if (mhs_value_get_status("IdlePollIntervall", context) & MHS_VAL_FLAG_WRITE)
  dev->IdlePollIntervall = mhs_value_get_as_ubyte("IdlePollIntervall", ACCESS_INIT, context);
else
  dev->IdlePollIntervall = mhs_value_get_as_ubyte("IdlePollIntervall", ACCESS_INIT, main_context);    

if (mhs_value_get_status("TxCanFifoEventLimit", context) & MHS_VAL_FLAG_WRITE)
  dev->TxCanFifoEventLimit = mhs_value_get_as_ubyte("TxCanFifoEventLimit", ACCESS_INIT, context);
else
  dev->TxCanFifoEventLimit = mhs_value_get_as_ubyte("TxCanFifoEventLimit", ACCESS_INIT, main_context);
if (mhs_value_get_status("CommTryCount", context) & MHS_VAL_FLAG_WRITE)
  dev->CommTryCount = mhs_value_get_as_ubyte("CommTryCount", ACCESS_INIT, context);
else
  dev->CommTryCount = mhs_value_get_as_ubyte("CommTryCount", ACCESS_INIT, main_context);  
}


static int32_t GetCanClockCnt(TCanDevice *dev)
{
struct TInfoVarList *info;

if (dev->Protokoll != 2)
  return(0);
if (!(dev->ModulDesc->CanFeaturesFlags & CAN_FEATURE_FD_HARDWARE))
  return(0);
if (!(info = GetInfoByIndex(dev, TCAN_INFO_KEY_CAN_CLOCKS)))
  return(-1);
return((int32_t)GetInfoValueAsULong(info));
}


static int32_t SetupAll(TCanDevice *dev)
{
TMhsObjContext *context, *main_context;
char str[26];
int32_t err;
uint32_t nbtr, dbtr, i;
uint8_t clk_idx, tx_ack_enable, can_err_msgs_enable, can_fd_flags, channels, channel, 
        time_stamp_mode, global_tx_ack_enable, global_can_err_msgs_enable, set_can_clk;
uint16_t speed, fd_speed;
    
main_context = can_main_get_context();
context = dev->Context;
channels = (uint8_t)dev->ModulDesc->CanChannels;
global_tx_ack_enable = mhs_value_get_as_ubyte("CanTxAckEnable", ACCESS_INIT, main_context);
dev->GlobalTxAckEnable = global_tx_ack_enable; 
global_can_err_msgs_enable = mhs_value_get_as_ubyte("CanErrorMsgsEnable", ACCESS_INIT, main_context);
if (mhs_value_get_status("TimeStampMode", context) & MHS_VAL_FLAG_WRITE)
  time_stamp_mode = mhs_value_get_as_ubyte("TimeStampMode", ACCESS_INIT, context);
else
  time_stamp_mode = mhs_value_get_as_ubyte("TimeStampMode", ACCESS_INIT, main_context);  
if (GetCanClockCnt(dev) > 1)
  set_can_clk = 1;
else    
  set_can_clk = 0;            
for (channel = 0; channel < channels; channel++)
  {
  // ***** Speed Setup
  safe_sprintf(str, 25, "CanSpeed%u", channel + 1);
  speed = mhs_value_get_as_uword(str, ACCESS_INIT, context);
  safe_sprintf(str, 25, "CanSpeed%uUser", channel + 1);
  nbtr = mhs_value_get_as_ulong(str, ACCESS_INIT, context);
  safe_sprintf(str, 25, "CanDSpeed%u", channel + 1);
  fd_speed = mhs_value_get_as_uword(str, ACCESS_INIT, context);
  safe_sprintf(str, 25, "CanDSpeed%uUser", channel + 1);
  dbtr = mhs_value_get_as_ulong(str, ACCESS_INIT, context);
  clk_idx = mhs_value_get_as_ubyte("CanClockIndex", ACCESS_INIT, context);
  safe_sprintf(str, 25, "CanFdFlags%u", channel + 1);
  can_fd_flags = mhs_value_get_as_ubyte(str, ACCESS_INIT, context);
#ifdef HAVE_TX_ACK_BYPASS    
  if (TxAckBypass)  // <*> Neu    
    tx_ack_enable = 0;
  else
    {     
#endif      
    safe_sprintf(str, 25, "Can%uTxAckEnable", channel + 1);
    if (mhs_value_get_status(str, context) & MHS_VAL_FLAG_WRITE)
      tx_ack_enable = mhs_value_get_as_ubyte(str, ACCESS_INIT, context);
    else
      tx_ack_enable = global_tx_ack_enable;
#ifdef HAVE_TX_ACK_BYPASS        
    }
#endif        
  safe_sprintf(str, 25, "Can%uErrorMsgsEnable", channel + 1);
  if (mhs_value_get_status(str, context) & MHS_VAL_FLAG_WRITE)
    can_err_msgs_enable = mhs_value_get_as_ubyte(str, ACCESS_INIT, context);
  else
    can_err_msgs_enable = global_can_err_msgs_enable;
            
  /*************************************/
  /*             Protokoll 2           */
  /*************************************/
  if (dev->Protokoll == 2)
    {
    if (dev->ModulDesc->CanFeaturesFlags & CAN_FEATURE_FD_HARDWARE)
      {
      if (speed)
        {
        if ((err = GetNbtrDBtrValueFromSpeeds(dev, &clk_idx, &nbtr, &dbtr, speed, fd_speed)) < 0)
          return(err);
        }        
      if ((err = TAR2CanSetClkNbtrDBtr(dev, channel, set_can_clk, clk_idx, nbtr, dbtr)))
        return(err);
      if (dev->CanFd)
        {
        if ((err = TAR2CanSetFdFlags(dev, channel, can_fd_flags)))
          return(err);
        }
      else
        {
        if ((err = TAR2CanSetFdFlags(dev, channel, 0)))
          return(err);
        }        
      }
    else
      {              
      if (speed)
        {
        if ((err = TAR2CanSetSpeed(dev, channel, speed)))
          return(err);
        }
      else
        {
        if ((err = TAR2CanSetBtr(dev, channel, nbtr)))
          return(err);
        }
      }
    // **** Tx ACK  
    if (dev->ModulDesc->SupportTxAck)
      {
      if ((err = TAR2CanSetTxAckEnable(dev, channel, tx_ack_enable)))
        return(err);            
      }
    // **** Error Msgs  
    if (dev->ModulDesc->SupportCanErrorMsgs)
      {
      if ((err = TAR2CanSetErrorLogEnable(dev, channel, can_err_msgs_enable)))
        return(err);
      }
    // **** Timestamp    
    dev->TimeStampMode = time_stamp_mode;
    if (dev->ModulDesc->SupportHwTimestamp)
      {
      // **** Hardware Time Stamps einstellen      
      if (time_stamp_mode >= TIME_STAMP_HW_UNIX)
        i = 1;
      else
        i = 0;
      if ((err = TAR2SetTimeStamp(dev, (unsigned char)i)))
        return(err);
      }
    // **** Features   
    if (dev->ModulDesc->CanFeaturesFlags & CAN_FEATURE_FIFO_OV_MODE)
      {     
      if ((err = TAR2SetFifoOvMode(dev, (uint8_t)(dev->OvMode & 0xFF))))
        return(err);    
      }
    if (dev->ModulDesc->CanFeaturesFlags & CAN_FEATURE_ECU_FLASH)
      {
      if ((err = TAR2SetEcuFlashVars(dev)))
        return(err);
      }        
    }
  /*************************************/
  /*             Protokoll 1           */
  /*************************************/
  else
    {
    if (speed)
      {
      if ((err = TARCanSetSpeed(dev, speed)))
        return(err);
      }
    else
      {
      if ((err = TARCanSetBtr(dev, nbtr)))
        return(err);
      }
    if (dev->ModulDesc->SupportTxAck)
      {
      if ((err = TARCanSetTxAckEnable(dev, tx_ack_enable)))
        return(err);
      }
    if (dev->ModulDesc->SupportCanErrorMsgs)
      {
      if ((err = TARCanSetErrorLogEnable(dev, can_err_msgs_enable)))
        return(err);
      }  
    dev->TimeStampMode = time_stamp_mode;
    if (dev->ModulDesc->SupportHwTimestamp)
      {
      // **** Hardware Time Stamps einstellen      
      if (time_stamp_mode >= TIME_STAMP_HW_UNIX)
        i = 1;
      else
        i = 0;
      if ((err = TARSetTimeStamp(dev, (unsigned char)i)))
        return(err);
      }
    }    
  }
return(0);
}   



/*

mode -> Flags
         SETUP_LOCAL   = Keine Daten zur Hardware übertragen
         SETUP_SET_ALL = Alle Setup Variablen setzen
         SETUP_CAN_STOP = CAN-Bus stoppen
         SETUP_CAN_CLEANUP = Rx & Tx FIFO löschen, Rx-OV & Tx-OV löschen

*/
int32_t ProcessSetup(TCanDevice *dev, uint32_t mode)
{
int32_t err;
TMhsObj *obj;
TObjValue *value;
uint8_t idx, channels, channel;
TMhsObjContext *context; // , *main_context; <*> raus
#if RX_FILTER_MAX > 0
TMhsObjContext *filter_context;
#endif
#if TX_PUFFER_MAX > 0
TMhsObjContext *tx_puffer_context;
struct TCanMsg msg;
#endif
uint8_t set_io_cfg;
uint16_t mask, set_puffer;
uint16_t set_filter[MAX_CAN_CHANNELS];
uint32_t io_cfg[MAX_IO_CFG];
union TCanIndex index;
uint32_t cmd, i;
char str[26];
#if (TX_PUFFER_MAX > 0) || (RX_FILTER_MAX > 0)
TObjCanPuffer *puffer;
#endif

//main_context = can_main_get_context(); <*> raus
context = dev->Context;
if (mode & (SETUP_SET_ALL | SETUP_LOCAL))
  SetupLocal(dev);
if (mode & SETUP_LOCAL)
  return(0);
if ((!dev->ModulDesc) || (dev->DeviceStatus.DrvStatus < DRV_STATUS_PORT_OPEN))
  return(0);
#if RX_FILTER_MAX > 0
filter_context = dev->RxFilterContext;
#endif
#if TX_PUFFER_MAX > 0
tx_puffer_context = dev->TxPufferContext;
#endif
set_io_cfg = 0x00;
channels = (uint8_t)dev->ModulDesc->CanChannels;
if (mode & SETUP_SET_ALL)
  {
  for (channel = 0; channel < channels; channel++)
    set_filter[channel] = 0xFFFF;    
  set_puffer = 0xFFFF;
  }
else
  {
  for (channel = 0; channel < channels; channel++)
    set_filter[channel] = 0x0000;
  set_puffer = 0x0000;
  }
for (i = 50; i; i--)
  {
  if (!(obj = mhs_object_get_cmd_marked(context)))
    break;
  channel = (uint8_t)obj->UserUInt & 0x03;
  idx = (uint8_t)(obj->UserUInt >> 8);
  cmd = obj->UserUInt & 0xFFFF0000;
  value = (TObjValue *)obj;
  switch (cmd)
    {
    case IO_VAR_PORT_OUT        : {
                                  dev->DigOut[channel] = value->Value.U16;
                                  dev->DigOutSet |= (0x01 << channel);
                                  break;
                                  }
    case IO_VAR_PORT_ANALOG_OUT : {
                                  dev->AnalogOut[channel] = value->Value.U16;
                                  dev->AnalogOutSet |= (0x01 << channel);
                                  break;
                                  }
    case IO_VAR_CFG             : {
                                  io_cfg[channel] = value->Value.U32;
                                  set_io_cfg |= (0x01 << channel);
                                  break;
                                  }
    case FILTER_READ_INTERVALL  : {
                                  dev->FilterReadIntervall = value->Value.U32;
                                  break;
                                  }
    case HIGH_POLL_INTERVALL    : {
                                  dev->HighPollIntervall = value->Value.U32;
                                  break;
                                  }
    case IDLE_POLL_INTERVALL    : {
                                  dev->IdlePollIntervall = value->Value.U32;
                                  break;
                                  }
    case TX_CAN_FIFO_EVENT_LIMIT: {
                                  dev->TxCanFifoEventLimit = value->Value.U32;
                                  break;
                                  }
    case COMM_TRY_COUNT         : {
                                  dev->CommTryCount = value->Value.U8;
                                  break;
                                  }                               
    }
  }
// **** Filter
#if RX_FILTER_MAX > 0
for (i = 32; i; i--)
  {
  if (!(obj = mhs_object_get_cmd_marked(filter_context)))
    break;
  index.Long = obj->Index;
  if (index.Long & (INDEX_FIFO_VIRTUAL | INDEX_SOFT_FLAG | INDEX_TXT_FLAG))
    continue;
  channel = index.Item.Channel;
  set_filter[channel] |= (0x0001 << (index.Item.SubIndex - 1));
  }
#endif
#if TX_PUFFER_MAX > 0
if (dev->Protokoll != 2)
  {
  for (i = 32; i; i--)
    {
    if (!(obj = mhs_object_get_cmd_marked(tx_puffer_context)))
      break;
    index.Long = obj->Index;
    set_puffer |= (0x0001 << (index.Item.SubIndex - 1));
    }
  }
#endif

if (mode & SETUP_CAN_STOP)
  {
  if (dev->Protokoll == 2)
    {
    for (channel = 0; channel < channels; channel++)
      {
      if ((err = TAR2SetCANMode(dev, channel, OP_CAN_STOP)))  // CAN-Bus stoppen
        return(err);
      }
    }
  else
    {
    if ((err = TARSetCANMode(dev, OP_CAN_STOP)))  // CAN-Bus stoppen
      return(err);
    }
  }
if (mode & SETUP_SET_ALL)
  {
  if ((err = SetupAll(dev)) <0)
    return(err);
  if (dev->Protokoll == 2)
    {
    if ((i = dev->ModulDesc->DigIoCount))
      {
      idx = (uint8_t)(i / 8);
      if ((i % 8))
        idx++;
      set_io_cfg = 0xFF;
      for (channel = 0; channel < idx; channel++)
        {
        safe_sprintf(str, 25, "IoCfg%u", channel);
        io_cfg[channel] = mhs_value_get_as_ulong(str, ACCESS_PUBLIC, context);
        }

      idx = (uint8_t)(i / 16);
      if ((i % 16))
        idx++;
      dev->DigOutSet = 0;
      for (channel = 0; channel < idx; channel++)
        {
        safe_sprintf(str, 25, "IoPort%uOut", channel);
        dev->DigOut[channel] = mhs_value_get_as_uword(str, ACCESS_PUBLIC, context);
        dev->DigOutSet |= (0x01 << channel);
        }      
      for (channel = 0; channel < 4; channel++)
        {
        safe_sprintf(str, 25, "IoPort%uAnalogOut", channel);
        dev->AnalogOut[channel] = mhs_value_get_as_uword(str, ACCESS_PUBLIC, context);
        dev->AnalogOutSet |= (0x01 << channel);
        }
      }
    }
  }
if ((dev->Protokoll == 2) && ((i = dev->ModulDesc->DigIoCount)))
  {
  if (set_io_cfg)
    {
    idx = (uint8_t)(i / 8);
    if ((i % 8))
      idx++;
    dev->UseDigIo = 0;
    for (channel = 0; channel < idx; channel++)
      {
      safe_sprintf(str, 25, "IoCfg%u", channel);
      if (mhs_value_get_as_ulong(str, ACCESS_PUBLIC, context))
        {
        dev->UseDigIo = 1;
        break;
        }
      }
    }
  }
else
  dev->UseDigIo = 0;
                   
/*************************************/
/*             Protokoll 2           */
/*************************************/
if (dev->Protokoll == 2)
  {       
#if RX_FILTER_MAX > 0
  // **** Filter einstellen
  for (channel = 0; channel < channels; channel++)
    {
    mask = 0x0001;
    index.Long =  dev->Index;
    for (i = 1; i <= dev->ModulDesc->HwRxFilterCount; i++)
      {
      if (mask & set_filter[channel])
        {
        index.Item.Channel = channel;
        index.Item.SubIndex = i;
        if ((puffer = (TObjCanPuffer *)mhs_object_get_by_index(index.Long, filter_context)))
          {
          if ((err = TAR2SetFilter(dev, channel, i, &((TObjCan *)puffer)->Filter)) < 0)
            return(err);
          }
        }
      mask <<= 1;
      }
    }
#endif
  if ((i = dev->ModulDesc->DigIoCount))
    {
    idx = (uint8_t)(i / 8);
    if ((i % 8))
      idx++;
    mask = 0x0001;
    for (channel = 0; channel < idx; channel++)
      {
      if (mask & set_io_cfg)
        {
        if ((err = TAR2SetIoConfig(dev, channel, io_cfg[channel])) < 0)
          return(err);
        }
      mask <<= 1;
      }
    }
  }
/*************************************/
/*             Protokoll 1           */
/*************************************/
else
  {
#if RX_FILTER_MAX > 0
  // **** Filter einstellen
  mask = 0x0001;
  for (i = 1; i <= dev->ModulDesc->HwRxFilterCount; i++)
    {
    if (mask & set_filter[0])
      {
      if ((puffer = (TObjCanPuffer *)mhs_object_get_by_index(i, filter_context)))
        {
        if ((err = TARSetFilter(dev, i, &((TObjCan *)puffer)->Filter)) < 0)
          return(err);
        }
      }
    mask <<= 1;
    }
#endif
#if TX_PUFFER_MAX > 0
  // **** Intervall Boxen setzen
  mask = 0x0001;
  for (i = 1; i <= dev->ModulDesc->HwTxPufferCount; i++)
    {
    if (mask & set_puffer)
      {
      if ((puffer = (TObjCanPuffer *)mhs_object_get_by_index(i | dev->Index | INDEX_TXT_FLAG, tx_puffer_context)))
        {
        if (mhs_can_puffer_get_copy(puffer, &msg) > 0)
          {
          if ((err = TARTxFifoPut(dev, i, &msg, 1)) < 0)
            return(err);
          }
        else
          mhs_can_puffer_set_intervall(puffer, 0);
        if ((err = TARCanSetIntervall(dev, i, mhs_can_puffer_get_intervall(puffer))) < 0)
          return(err);
        }
       else
        {
        if ((err = TARCanSetIntervall(dev, i, 0)) < 0)
          return(err);
        }
      }
    mask <<= 1;
    }
#endif
  }

if (mode & SETUP_CAN_CLEANUP)
  {
  if (dev->Protokoll == 2)
    {
    for (channel = 0; channel < channels; channel++)
      {
      // CAN Stop, Rx & Tx FIFO löschen, Rx-OV & Tx-OV löschen
      if ((err = TAR2SetCANMode(dev, channel, OP_CAN_STOP | 0xA0)))            
        return(err);
      }
    }
  else
    {
    // CAN Stop, Rx & Tx FIFO löschen, Rx-OV & Tx-OV löschen
    if ((err = TARSetCANMode(dev, OP_CAN_STOP | 0xA0))) 
      return(err);
    }
  }
return(0);
}












