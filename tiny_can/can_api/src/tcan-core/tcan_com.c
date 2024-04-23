/***************************************************************************
                          tcan_com.c  -  description
                             -------------------
    begin             : 23.03.2008
    last modify       : 05.05.2023    
    copyright         : (C) 2008 - 2023 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <string.h>  // memcpy
#include "util.h"
#include "tar_drv.h"
#include "sm_drv.h"
#include "can_fifo.h"
#include "can_puffer.h"
#include "can_core.h"
#include "log.h"
#include "tcan_com.h"

#define STATUS_READ_TIMEOUT 3000        // Status mindestens alle 3 Sekunden lesen

/*
Das "COM" Kommando vereint mehrere Kommandos:
 - Senden von CAN Nachrichten
 - Tiny-CAN Status abfragen
 - Filter Nachrichten abfragen
 - CAN Empfangs-Fifo auslesen

Aufbau des Parameter Blocks:

Kommando:
   +------------------------------------------+
   |  Kommando Flags                          |
   |   Bit  0 = -                             |
   |        1 = -                             |
   |        2 = Status abfragen               |
   |        3 = -                             |
   |        4 = -                             |
   |        5 = Filter abfragen               |
   |        6 = RxD Nachrichten Fifo auslesen |
   |        7 = -                             |
   +------------------------------------------+
   |          TxD CAN Nachrichten             |
   +------------------------------------------+

ACK:
   +------------------------------------------+
   |  ACK Flags, nachfolgende Daten           |
   |  entsprechend gesetzter Flags            |
   |   Bit  0 = -                             |
   |        1 = -                             |
   |        2 = Status abfragen               |
   |        3 = -                             |
   |        4 = -                             |
   |        5 = Filter abfragen               |
   |        6 = RxD Nachrichten Fifo auslesen |
   |        7 = -                             |
   +------------------------------------------+
   |          Tiny-CAN Status                 |
   +------------------------------------------+
   |           Filter Nachrichten             |
   +------------------------------------------+
   |            RxD Nachrichten               |
   +------------------------------------------+


CAN Type ID Flags
=================

 Byte |    Bit 7    |    Bit 6    |    Bit 5    |    Bit 4    | ....  
      +-------------+-------------+-------------+-------------+
   1  | NonStdFrame |     EFF     |     RTR     |      Tx     | ....
      +-------------+-------------+-------------+-------------+

CAN Standart Frame RxD
======================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     0     |          ID (Bit 10 - 8)          |                      DLC                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |                                       ID (Bit 7 - 0)                                          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       CAN Daten Bytes                                         |
      +-----------                                                                         -----------+
   .. |                                            . . . .                                            |
      +-----------                                                                         -----------+
   n  |                                            . . . .                                            |  
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

CAN Standart RTR Frame RxD
==========================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     0     |     1     |     0     |                      DLC                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |     0     |     0     |     0     |     0     |     0     |          ID (Bit 10 - 8)          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 7 - 0)                                          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
      
CAN Extended Frame TxD
======================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     1     |     0     |     0     |                      DLC                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |     0     |     0     |     0     |                      ID (Bit 28 - 24)                     |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 23 - 16)                                        |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+   
   2  |                                       ID (Bit 15 - 8)                                         |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 7 - 0)                                          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       CAN Daten Bytes                                         |
      +-----------                                                                         -----------+
   .. |                                            . . . .                                            |
      +-----------                                                                         -----------+
   n  |                                            . . . .                                            |  
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
      
CAN Extended RTR Frame RxD
==========================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     1     |     1     |     0     |                      DLC                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |     0     |     0     |     0     |                      ID (Bit 28 - 24)                     |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 23 - 16)                                        |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+   
   2  |                                       ID (Bit 15 - 8)                                         |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 7 - 0)                                          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

CAN Standart Frame TxD
======================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     0     |     0     |     1     |                      DLC                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |     0     |     0     |     0     |     0     |     0     |          ID (Bit 10 - 8)          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 7 - 0)                                          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       CAN Daten Bytes                                         |
      +-----------                                                                         -----------+
   .. |                                            . . . .                                            |
      +-----------                                                                         -----------+
   n  |                                            . . . .                                            |  
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

CAN Standart RTR Frame TxD
==========================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     0     |     1     |     1     |                      DLC                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |     0     |     0     |     0     |     0     |     0     |          ID (Bit 10 - 8)          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 7 - 0)                                          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

CAN Extended Frame TxD
======================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     1     |     0     |     1     |                      DLC                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |     0     |     0     |     0     |                      ID (Bit 28 - 24)                     |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 23 - 16)                                        |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+   
   2  |                                       ID (Bit 15 - 8)                                         |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 7 - 0)                                          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       CAN Daten Bytes                                         |
      +-----------                                                                         -----------+
   .. |                                            . . . .                                            |
      +-----------                                                                         -----------+
   n  |                                            . . . .                                            |  
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

CAN Extended RTR Frame TxD
==========================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     1     |     1     |     1     |                      DLC                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |     0     |     0     |     0     |                      ID (Bit 28 - 24)                     |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 23 - 16)                                        |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+   
   2  |                                       ID (Bit 15 - 8)                                         |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                       ID (Bit 7 - 0)                                          |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
 
      
*/

#define TAR_TOGGLE_FLAG       0x01

#define TAR_STREAM_STATUS     0x04
#define TAR_STREAM_TX_CHANNEL 0x08

#define TAR_STREAM_FILTER     0x20
#define TAR_STREAM_RXD        0x40
#define TAR_STREAM_CHANNEL    0x80

/**************************************************************************/
/*                             Kommandos                                  */
/**************************************************************************/               
#define CMD_COM                    0x24 | CMD_TYPE_EXT1               
               
#define CAN_STATUS_TRANSLATE_SIZE 6

static const char CanStatusTranslate[CAN_STATUS_TRANSLATE_SIZE] =
  {CAN_STATUS_OK,          // TINY_CAN_OK         (0) => Ok
   CAN_STATUS_WARNING,     // TINY_CAN_WARNING    (1) => Error warning
   CAN_STATUS_PASSIV,      // TINY_CAN_PASSIV     (2) => Error passiv
   CAN_STATUS_BUS_OFF,     // TINY_CAN_BUS_OFF    (3) => Bus Off
   CAN_STATUS_UNBEKANNT,   // TINY_CAN_OVERRUN    (4) => Overrun
   CAN_STATUS_UNBEKANNT};  // TINY_CAN_UNBEKANNT  (5) => Unbekannt Fehler



static int32_t ExtractCanMsg(struct TCanFdMsg *msg, unsigned char **src, int32_t *size, unsigned char mode)
{
unsigned char *data;
int32_t count, tmr_len;
unsigned char header, len;
uint32_t t;

if ((!size) || (!src))
  return(-1);
count = *size;
data = *src;
if (count < 1)
  return(0);

header = *data++;
msg->MsgFlags = 0;
tmr_len = 0;
if (mode >= TIME_STAMP_HW_UNIX)
  tmr_len = 4;
len = header & 0x0F;
msg->MsgLen = len;      // Dlc extrahieren
if (!(header & 0x80))
  {  // STD Frame
  count -= (2 + tmr_len + len);
  if (count < 0)
    return(-1);
  msg->Id = *data++ | (((uint16_t)header << 4) & 0x0700); // Id extrahieren
  }
else
  {
  if (header & 0x20)
    {  // EFF / RTR Frame
    msg->MsgRTR = 1;
    len = 0;
    }
  if (header & 0x10)
    msg->MsgTxD = 1;
  if (header & 0x40)
    {  // EFF Frame
    count -= (5 + tmr_len + len);
    if (count < 0)
      return(-1);
    msg->Id = *((uint32_t *)data);
    data += 4;
    msg->MsgEFF = 1;
    }
  else
    {  // STD Frame
    count -= (3 + tmr_len + len);
    if (count < 0)
      return(-1);
    msg->Id = *((uint16_t *)data);
    data += 2;
    }
  }
// **** Daten lesen
if (len)
  {
  memcpy(&msg->MsgData, data, len); // Daten in FIFO kopieren
  data += len;
  }
if (mode >= TIME_STAMP_HW_UNIX)
  {
  t = *((uint32_t *)data);
  if (mode == TIME_STAMP_HW)
    {    
    msg->Time.Sec = 0;
    msg->Time.USec = t;
    }
  else
    {    
    msg->Time.Sec = t / 10000;
    msg->Time.USec = (t % 10000) * 100;
    }
  data += 4;
  }
else if (mode == TIME_STAMP_SOFT)  
  get_timestamp(&msg->Time);  
else
  {
  msg->Time.Sec = 0L;
  msg->Time.USec = 0L;
  }
if ((msg->MsgTxD) && (!msg->MsgEFF) && (msg->Id & 0x8000))   // Error Message
  {
  msg->MsgTxD = 0;
  msg->MsgErr = 1;
  msg->MsgLen = 4;
  msg->MsgData[2] = msg->MsgData[1]; // Receive Error Counter
  msg->MsgData[3] = msg->MsgData[0]; // Transmit Error Counter
  msg->MsgData[0] = (unsigned char)(msg->Id & 0x0007);
  if (msg->Id & 0x0080)  // Bus Off
    msg->MsgData[1] = 3;  
  else if (msg->Id & 0x0020)  // Error Passiv
    msg->MsgData[1] = 2;
  else if (msg->Id & 0x0040)  // Error Warning
    msg->MsgData[1] = 1;  
  else
    msg->MsgData[1] = 0;
  if (msg->Id & 0x0100)
    msg->MsgData[1] |= 0x10;    
  msg->Id = 0x80000000;    
  }
*src = data;
*size = count;
return(1);
}


static int32_t CanProcessStatus(TCanDevice *dev, uint32_t index, unsigned char *src, int32_t size)
{
int32_t cnt;
struct TTarStatus tar_status;
(void)index;

dev->StatusTimeStamp = dev->TimeNow;     // Status Timeout resetten
if (size < (int32_t)dev->ModulDesc->StatusSize)
  return(-1);
cnt = 1;
tar_status.ExStatus.Byte = 0;  
tar_status.Status.Byte = *src++;
if (dev->ModulDesc->StatusSize == 2)
  {
  tar_status.ExStatus.Byte = *src++;
  cnt++;
  }
if (tar_status.Status.Flag.CanBusStatusBits == 0x07)
  return(-1);
SetTarStatusToDev(dev, tar_status);
return(cnt);
}


void SetTarStatusToDev(TCanDevice *dev, struct TTarStatus tar_status)
{
// **** CAN 0 Status
if (tar_status.Status.Flag.CanBusStatusBits < CAN_STATUS_TRANSLATE_SIZE)
  dev->DeviceStatus.CanStatus = CanStatusTranslate[tar_status.Status.Flag.CanBusStatusBits];
else
  dev->DeviceStatus.CanStatus = CAN_STATUS_UNBEKANNT;
if (tar_status.ExStatus.Flag.BusFailure)  
  dev->DeviceStatus.CanStatus |= BUS_FAILURE;  
// Puffer Status
if (tar_status.Status.Flag.CanRxDOv)
  {
  dev->DeviceStatus.FifoStatus |= FIFO_HW_OVERRUN;
#ifdef ENABLE_LOG_SUPPORT
  if (LogEnable)
    LogPrintf(LOG_ERROR , "FEHLER: [SetDrvStatus] CanRxOv");
#endif
  }
if (tar_status.Status.Flag.CanTxDStatus == TXD_STATUS_EMPTY)
  dev->HwTxDFifoCount[0] = 0;
else if (tar_status.Status.Flag.CanTxDStatus == TXD_STATUS_PEND)
  {
  if (dev->HwTxDFifoCount[0] > dev->ModulDesc->HwTxFifoLimit)
    dev->HwTxDFifoCount[0] = dev->ModulDesc->HwTxFifoLimit;
  }
else
  dev->ReadStatusFlag = 1;  
}


static int32_t CanRxDProcessMessages(TCanDevice *dev, unsigned char *src, int32_t size)
{
struct TCanFdMsg msg;
unsigned char time_stamp_mode;

// **** Timestamp lesen
if ((dev->ModulDesc->SupportHwTimestamp) && (dev->TimeStampMode >= TIME_STAMP_HW_UNIX))
  time_stamp_mode = dev->TimeStampMode;
else if ((dev->TimeStampMode == TIME_STAMP_SOFT) || (dev->TimeStampMode == TIME_STAMP_HW_SW_UNIX))
  time_stamp_mode = TIME_STAMP_SOFT;
else
  time_stamp_mode = 0;  
while (size)
  {  
  if (ExtractCanMsg(&msg, &src, &size, time_stamp_mode) <= 0)
    return(-1);
  // **** CAN Message speichern
  process_rx_msg(dev->Index, &msg);  
#ifdef ENABLE_LOG_SUPPORT
  // **** Log Datei schreiben
  if (LogEnable)
    LogCanFdMessage(LOG_RX_MSG | LOG_ERR_MSG | LOG_OV_MSG, &msg, LOG_FMT_FD_TYPE_SOURCE, "[RxD]");        
#endif
  }
return(0);
}


static int32_t CanFilterProcessMessages(TCanDevice *dev, unsigned char *src, int32_t size)
{
struct TCanFdMsg fd_msg;
TMhsObjContext *context;
unsigned char fidx, flags;

if (!size)
  return(-1);
size--;
flags = *src++;
if (!(context = dev->RxFilterContext))
  return(0);
for (fidx = 0; fidx < 8; fidx++)
  {
  if (flags & (1 << fidx))
    {
    if (ExtractCanMsg(&fd_msg, &src, &size, TIME_STAMP_HW) <= 0)
      return(-1);
    // **** CAN Message speichern
    (void)mhs_can_puffer_put_fd_by_index(fidx+1, &fd_msg, context);    
    }
  }
return(0);
} 


static int32_t CanTxDProcessMessages(unsigned char *dst, struct TCanFdMsg *msgs, uint16_t count)
{
uint16_t cnt;
unsigned char len;

cnt = 0;
for (; count; count--)
  {
  len = msgs->MsgLen;
  if (msgs->MsgEFF)
    {
    if (!msgs->MsgRTR)
      *dst++ = len | 0xC0;  // EXT
    else
      {
      *dst++ = len | 0xE0;  // EXT / RTR
      len = 0;
      }
    *(uint32_t *)dst = (uint32_t)msgs->Id;
    dst += 4;
    if (len)
      {
      memcpy(dst, &msgs->MsgData, len);
      dst += len;
      }
    len += 5;
    }
  else
    {
    if (!msgs->MsgRTR)
      {  // STD
      *dst++ = (unsigned char)(((msgs->Id >> 4) & 0x70) | len);
      *dst++ = (unsigned char)msgs->Id;
      memcpy(dst, &msgs->MsgData, len);
      dst += len;
      len += 2;
      }
    else
      {  // STD / RTR
      *dst++ = len | 0xA0;
      *(uint16_t *)dst = (uint16_t)msgs->Id;
      dst += 2;
      len = 3;
      }
    }
  cnt += len;
  msgs++;
  }
return(cnt);
}



int32_t MainCommProc(TCanDevice *dev)
{
#if RX_FILTER_MAX > 0
TMhsObjContext *context;
TObjCan *filter;
#endif
TObjCanFifo *fifo; 
int32_t size, err, res, read_filter;
uint32_t i, filter_read_intervall;
unsigned char *data;
unsigned char type;

filter_read_intervall = dev->FilterReadIntervall;
/**********************************/
/* Get Kommando Daten aufbereiten */
/**********************************/
type = 0;
size = 1;
data = &dev->TxDParameter.Data[1];
// ****** Festlegen ob Status abgefragt wird
if (dev->ReadStatusFlag)
  {
  dev->ReadStatusFlag = 0;
  type |= TAR_STREAM_STATUS;
  }
else if (mhs_diff_time(dev->TimeNow, dev->StatusTimeStamp) >= STATUS_READ_TIMEOUT)
  {  // JA
  dev->StatusTimeStamp = dev->TimeNow;     // Status Timeout resetten
  type |= TAR_STREAM_STATUS;
  }
// ****** Festlegen ob Filter Nachrichten abgefragt werden
#if RX_FILTER_MAX > 0
if (mhs_diff_time(dev->TimeNow, dev->FilterTimeStamp) >= filter_read_intervall)
  {
  dev->FilterTimeStamp = dev->TimeNow;     // Filter Timeout resetten
  read_filter = 0;
  context = dev->RxFilterContext;
  for (i = 1; i <= dev->ModulDesc->HwRxFilterCount; i++)
    {
    if ((filter = (TObjCan *)mhs_object_get_by_index(i, context)))
      {
      if (filter->Filter.FilEnable)
        {
        read_filter = 1;
        break;
        }
      }
    }
  if ((dev->DeviceStatus.DrvStatus == DRV_STATUS_CAN_RUN) && (read_filter))
    type |= TAR_STREAM_FILTER;
  }
#endif
// ****** Versand von CAN Nachrichten
if (dev->HwTxDFifoCount[0] < dev->ModulDesc->HwTxFifoSize)
  {
  i = dev->ModulDesc->HwTxFifoSize - dev->HwTxDFifoCount[0];
  if (i > TX_CAN_TX_LIMIT)
    i = TX_CAN_TX_LIMIT;
  if ((fifo = mhs_can_fifo_get_obj_by_index(dev->Index | INDEX_TXT_FLAG)))
    {
    if ((res = mhs_can_fifo_get_fd(fifo, dev->TxCanBuffer, i)) > 0) // <*> ändern! ??
      {
      dev->ReadStatusFlag = 1;
      dev->HwTxDFifoCount[0] += res;
      size += CanTxDProcessMessages(data, dev->TxCanBuffer, (uint16_t)res);
#ifdef ENABLE_LOG_SUPPORT
      if (LogEnable) 
        LogCanFdMessages(LOG_TX_MSG, dev->TxCanBuffer, res, LOG_FMT_FD_TYPE,        
            "Dev-Index: %#08X TXD Messages", dev->Index);
#endif    
      res = mhs_can_fifo_count(fifo);
      if (res <= (int32_t)dev->TxCanFifoEventLimit)
        (void)mhs_object_set_event((TMhsObj *)fifo);   
      }
    }
  }
dev->TxDParameter.Data[0] = type;
dev->TxDParameter.Count = size;
dev->Kommando = CMD_COM;
err = SMDrvCommando(dev, 0);
if (!err)
  {
  /**********************************/
  /* Empfangene Daten verarbeiten   */
  /**********************************/
  if (dev->RxDParameter.Count >= 2)
    {
    size = dev->RxDParameter.Count-1;
    data = &dev->RxDParameter.Data[0];
    type = *data++;
    // **** Status
    if (type & TAR_STREAM_STATUS)
      {
      if ((res = CanProcessStatus(dev, 0L, data, size)) < 0)
        err = -1;
      else
        {
        size -= res;
        data += res;
        }
      }
    // **** CAN Nachrichten Empfang
    if (!err)
      {
      if (type & TAR_STREAM_RXD)
        {
        dev->CommIdleCounter = 2;
        if (CanRxDProcessMessages(dev, data, size) < 0)
          err = -1;
        }
      else
        {
        if (dev->CommIdleCounter)
          dev->CommIdleCounter--;
        }  
      }
    if (!err)
      {
      if (type & TAR_STREAM_FILTER)
        {
        if (CanFilterProcessMessages(dev, data, size) < 0)
          err = -1;
        }
      }
    }
  }
/*if (type & TAR_STREAM_RXD)
  return(1);
else
  return(0); */

LOG_FUNCTION_ERROR_STR("MainCommProc", err, dev->Io->ErrorString);
return(err);
}

