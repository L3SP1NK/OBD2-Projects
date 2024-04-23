/***************************************************************************
                          tar_drv.c  -  description
                             -------------------
    begin             : 01.11.2010
    last modify       : 30.04.2019    
    copyright         : (C) 2010 - 2019 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "com_io.h"
#include "sm_drv.h"
#include "log.h"
#include "tcan_com.h"
#include "tar_drv.h"


/*

Status:
=======

|   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
| CAN 0 TxD |   CAN 0 TxD Status    | CAN 0 RxD | CAN 0 RxD |           CAN 0 Status            |
|   Error   |                       |     OV    |   Pend    |                                   |



CAN x Status:

  0 = Controller Ok
  1 = Error warning
  2 = Error passiv
  3 = Bus Off
  4 = Empfangs Puffer überlauf
  5 = CAN Bus Stop
  7 = Power Up

CAN x TxD Status:
  00 = Empty
  01 = Pend
  10 = Limit ( 1 / 2 Puffergröße überschritten)
  11 = OV

CAN0TxStatus:
=============

|   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|   Box 8   |   Box 7   |   Box 6   |   Box 5   |   Box 4   |   Box 3   |   Box 2   |   Box 1   |
|   Error   |   Error   |   Error   |   Error   |   Error   |   Error   |   Error   |   Error   |


CAN1TxStatus:
=============

|   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|   Box 8   |   Box 7   |   Box 6   |   Box 5   |   Box 4   |   Box 3   |   Box 2   |   Box 1   |
|   Error   |   Error   |   Error   |   Error   |   Error   |   Error   |   Error   |   Error   |

*/

#define CMD_GET_STATUS             0x40
#define CMD_GET_STATUS_ALL         0x41

#define CMD_GET_CAN0_TX_STATUS     0x42
#define CMD_GET_CAN1_TX_STATUS     0x43

#define CMD_GET_SPI_I2C_STATUS     0x44

#define CMD_CAN0_SET               0x01 | CMD_TYPE_EXT1
#define CMD_CAN0_GET               0x02 | CMD_TYPE_EXT1
#define CMD_CAN0_SET_MODE          0x03 | CMD_TYPE_EXT1
#define CMD_CAN0_RXD_CLEAR         0x45
#define CMD_CAN0_RXD_GET_COUNT     0x46
#define CMD_CAN0_RXD               0x47
#define CMD_CAN0_TXD_CLEAR         0x48
#define CMD_CAN0_TXD_GET_COUNT     0x49
#define CMD_CAN0_TXD               0x04 | CMD_TYPE_EXT2
#define CMD_CAN0_RXD_SET_FILTER    0x05 | CMD_TYPE_EXT1
#define CMD_CAN0_RXD_IDX_CLEAR     0x06 | CMD_TYPE_EXT1
#define CMD_CAN0_RXD_IDX_GET_COUNT 0x07 | CMD_TYPE_EXT1
#define CMD_CAN0_RXD_IDX           0x08 | CMD_TYPE_EXT1
#define CMD_CAN0_TXD_IDX_SET_TIMER 0x09 | CMD_TYPE_EXT1
#define CMD_CAN0_TXD_IDX_CLEAR     0x0A | CMD_TYPE_EXT1
#define CMD_CAN0_TXD_IDX_GET_COUNT 0x0B | CMD_TYPE_EXT1
#define CMD_CAN0_TXD_IDX           0x0C | CMD_TYPE_EXT1

#define CMD_CAN1_SET               0x0D | CMD_TYPE_EXT1
#define CMD_CAN1_GET               0x0E | CMD_TYPE_EXT1
#define CMD_CAN1_SET_MODE          0x0F | CMD_TYPE_EXT1
#define CMD_CAN1_RXD_CLEAR         0x4A
#define CMD_CAN1_RXD_GET_COUNT     0x4B
#define CMD_CAN1_RXD               0x4C
#define CMD_CAN1_TXD_CLEAR         0x4D
#define CMD_CAN1_TXD_GET_COUNT     0x4E
#define CMD_CAN1_TXD               0x10 | CMD_TYPE_EXT2
#define CMD_CAN1_RXD_SET_FILTER    0x11 | CMD_TYPE_EXT1
#define CMD_CAN1_RXD_IDX_CLEAR     0x12 | CMD_TYPE_EXT1
#define CMD_CAN1_RXD_IDX_GET_COUNT 0x13 | CMD_TYPE_EXT1
#define CMD_CAN1_RXD_IDX           0x14 | CMD_TYPE_EXT1
#define CMD_CAN1_TXD_IDX_SET_TIMER 0x15 | CMD_TYPE_EXT1
#define CMD_CAN1_TXD_IDX_CLEAR     0x16 | CMD_TYPE_EXT1
#define CMD_CAN1_TXD_IDX_GET_COUNT 0x17 | CMD_TYPE_EXT1
#define CMD_CAN1_TXD_IDX           0x18 | CMD_TYPE_EXT1

#define CMD_CAN0_RXD_ALL           0x4F
#define CMD_CAN1_RXD_ALL           0x50

#define CMD_SPI_SET                0x19 | CMD_TYPE_EXT1
#define CMD_SPI_GET                0x1A | CMD_TYPE_EXT1
#define CMD_SPI_SET_MODE           0x1B | CMD_TYPE_EXT1
#define CMD_SPI_TXD                0x1C | CMD_TYPE_EXT2

#define CMD_I2C_SET                0x1D | CMD_TYPE_EXT1
#define CMD_I2C_GET                0x1E | CMD_TYPE_EXT1
#define CMD_I2C_SET_MODE           0x1F | CMD_TYPE_EXT1
#define CMD_I2C_TXD                0x20 | CMD_TYPE_EXT2

#define CMD_GET_INPUT              0x21 | CMD_TYPE_EXT1
#define CMD_SET_OUTPUT             0x22 | CMD_TYPE_EXT1
#define CMD_GET_AD                 0x51
#define CMD_GET_INFO               0x23 | CMD_TYPE_EXT1

#define CMD_START_READ_INFO        0x52
#define CMD_READ_INFO              0x53

#define CMD_SET_TIMESTAMP          0x25 | CMD_TYPE_EXT1

// System Kommandos
#define CMD_PING                   0x7E  // SINGLE | Ping
#define CMD_BIOS_VERSION           0x7F  // SINGLE | BIOS Version abfragen
//#define CMD_BIOS                   0xBF  // EXT I  | Bios Kommando
//#define CMD_START_BIOS             0xBE  // EXT I  | Bios starten

const uint32_t CAN_SPEED_TAB[] = {10,     // 0 = 10 kBit/s
                                  20,     // 1 = 20 kBit/s
                                  50,     // 2 = 50 kBit/s
                                  100,    // 3 = 100 kBit/s
                                  125,    // 4 = 125 kBit/s
                                  250,    // 5 = 250 kBit/s
                                  500,    // 6 = 500 kBit/s
                                  800,    // 7 = 800 kBit/s
                                  1000,   // 8 = 1 MBit/s
                                  0};     // Ende der Liste

const uint8_t CMD2_CANx_SET_MODE[] = {CMD2_CAN0_SET_MODE, CMD2_CAN1_SET_MODE, 0xFF};
const uint8_t CMD2_CANx_SET[] = {CMD2_CAN0_SET, CMD2_CAN1_SET, 0xFF};
const uint8_t CMD2_CANx_RXD_IDX_CLEAR[] = {CMD2_CAN0_RXD_IDX_CLEAR, CMD2_CAN0_RXD_IDX_CLEAR, 0xFF};
const uint8_t CMD2_CANx_TXD_IDX_CLEAR[] = {CMD2_CAN0_TXD_IDX_CLEAR, CMD2_CAN0_TXD_IDX_CLEAR, 0xFF};


/**************************************************************/
/* Tiny-CAN Status abfragen                                   */
/**************************************************************/
int32_t TARGetStatus(TCanDevice *dev)
{
int32_t err;
struct TTarStatus tar_status;

tar_status.Status.Byte = 0;
tar_status.ExStatus.Byte = 0;
dev->Kommando = CMD_GET_STATUS_ALL;
if (!(err = SMDrvCommando(dev, 0)))
  {
  if (dev->RxDParameter.Count < dev->ModulDesc->StatusSize)
    err = ERR_SM_DRV_ACK_COUNT;
  else
    {
    if (dev->ModulDesc->StatusSize > 1)
      tar_status.ExStatus.Byte = dev->RxDParameter.Data[1];
    tar_status.Status.Byte = dev->RxDParameter.Data[0];
    }
  }  
SetTarStatusToDev(dev, tar_status);  
LOG_FUNCTION_ERROR_STR("TARGetStatus", err, dev->Io->ErrorString);
return(err);
} 


/**************************************************************/
/* Empfangs-Filter setzten                                    */
/**************************************************************/
int32_t TARSetFilter(TCanDevice *dev, uint32_t index, struct TMsgFilter *msg_filter)
{
int32_t err;
uint8_t filter_index;

err = 0;
filter_index = (uint8_t)(index & 0xFF) - 1;
if (filter_index > 7)
  return(ERR_INDEX_RANGE);
dev->TxDParameter.Data[0] = filter_index;
if (msg_filter->FilEnable)
  {
  if (msg_filter->FilEFF)
    dev->TxDParameter.Data[1] = 0xC0;
  else
    dev->TxDParameter.Data[1] = 0x80;
  SetUInt32ToParam(dev->TxDParameter.Data, 2, msg_filter->Maske);
  SetUInt32ToParam(dev->TxDParameter.Data, 6, msg_filter->Code);
  dev->TxDParameter.Count = 10;
  }
else
  {
  dev->TxDParameter.Data[1] = 0x00;
  dev->TxDParameter.Count = 2;
  }
dev->Kommando = CMD_CAN0_RXD_SET_FILTER;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARSetFilter", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* Ping / Pong                                                */
/**************************************************************/
int32_t TARCommSync(TCanDevice *dev)
{
TMhsObjContext *io_context;
int32_t err, res, i;
uint32_t rxd_timeout;
uint8_t c;

if (ComIsOpen(dev->Io) < 1)
  return(ERR_SM_PORT_NOT_OPEN);
err = ERR_SM_RX_ACK_TIMEOUT;
io_context = dev->Io->Context;
rxd_timeout = mhs_value_get_as_ulong("RxDTimeout", ACCESS_INIT, io_context);
(void)mhs_value_set_as_ulong("RxDTimeout", 20, ACCESS_INIT, 0, io_context);  
for (i = 2; i; i--)
  {
  if (ComWriteByte(dev->Io, 0x00) < 0)
    {
    err = ERR_SM_TX_DATA;
    break;
    }
  if ((res = ComReadByte(dev->Io, &c)) < 0)
    {
    err = ERR_SM_RX_DATA;
    break;
    }
  if ((res == 1) && (c == 0x00))      
    {
    err = 0;
    break;
    }  
  if (i > 1)
    {    
    mhs_sleep(150);
    ComFlushBuffer(dev->Io);
    }
  }
(void)mhs_value_set_as_ulong("RxDTimeout", rxd_timeout, ACCESS_INIT, 0, io_context);  
LOG_FUNCTION_ERROR_STR("TARCommSync", err, dev->Io->ErrorString);  
return(err);
}


int32_t TARPing(TCanDevice *dev)
{
int32_t err;

dev->Kommando = CMD_PING;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARPing", err, dev->Io->ErrorString);
return(err);
}


int32_t TARGetBiosVersion(TCanDevice *dev, struct TBiosVersionInfo *info)
{
int32_t err;

dev->Kommando = CMD_BIOS_VERSION;
if (!(err = SMDrvCommando(dev, 0)))
  {
  if (dev->RxDParameter.Count != 6)
    err = ERR_SM_DRV_ACK_COUNT;
  else
    {  
    info->Version = dev->RxDParameter.Data[0];                        // Protokoll Version
    info->Flags = dev->RxDParameter.Data[1];                          // Flags
    info->TxMTU = dev->RxDParameter.Data[2] | ((uint16_t)dev->RxDParameter.Data[3] << 8); // Tx MTU
    info->RxMTU = dev->RxDParameter.Data[4] | ((uint16_t)dev->RxDParameter.Data[5] << 8); // Rx MTU
    }
  }  
LOG_FUNCTION_ERROR_STR("TARGetBiosVersion", err, dev->Io->ErrorString);
return(err);
}


int32_t TARInfoStartRead(TCanDevice *dev)
{
int32_t err;

dev->Kommando = CMD_START_READ_INFO;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARInfoStartRead", err, dev->Io->ErrorString);
return(err);
}


int32_t TARInfoGetNext(TCanDevice *dev, struct TTarInfoVar *info)
{
int32_t err;

dev->Kommando = CMD_READ_INFO;
if (!(err = SMDrvCommando(dev, 0)))
  {
  if (dev->RxDParameter.Count == 0)
    return(0);
  else if (dev->RxDParameter.Count > sizeof(struct TCanInfoVar))
    err = ERR_SM_DRV_ACK_COUNT;
  else
    {
    memcpy(info, &dev->RxDParameter.Data[0], dev->RxDParameter.Count);
    return(1);
    }
  }
LOG_FUNCTION_ERROR_STR("TARInfoGetNext", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* Hardware TimeStamps aktivieren                             */
/**************************************************************/
int32_t TARSetTimeStamp(TCanDevice *dev, uint8_t enabled)
{
int32_t err;

dev->TxDParameter.Count = 1;
dev->TxDParameter.Data[0] = enabled;
dev->Kommando = CMD_SET_TIMESTAMP;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARSetTimeStamp", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN Arbeitsmodus einstellen                                */
/**************************************************************/
int32_t TARSetCANMode(TCanDevice *dev, uint8_t can_mode)
{
int32_t err;

dev->TxDParameter.Count = 1;
dev->TxDParameter.Data[0] = can_mode;
dev->Kommando = CMD_CAN0_SET_MODE;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARSetCANMode", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN Speed einstellen                                       */
/**************************************************************/
int32_t TARCanSetSpeed(TCanDevice *dev, uint32_t speed)
{
uint32_t s;
int32_t err;
uint8_t i;

for (i = 0; (s = CAN_SPEED_TAB[i]) != speed; i++)
  {
  if (!s)
    break;
  }
if (!s)
  err = -1;
else
  {
  i++;
  // **** Speed Einstellen
  dev->TxDParameter.Count = 2;
  dev->TxDParameter.Data[0] = CAN_VAR_SPEED;  // Index
  dev->TxDParameter.Data[1] = i;
  dev->Kommando = CMD_CAN0_SET;
  err = SMDrvCommando(dev, 0);
  }
LOG_FUNCTION_ERROR_STR("TARCanSetSpeed", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN Speed über BTR Register einstellen                     */
/**************************************************************/
int32_t TARCanSetBtr(TCanDevice *dev, uint32_t btr)
{
int32_t err;

// **** Speed Einstellen
dev->TxDParameter.Count = 5;
dev->TxDParameter.Data[0] = CAN_VAR_SPEED_BTR;  // Index
SetUInt32ToParam(dev->TxDParameter.Data, 1, btr);
dev->Kommando = CMD_CAN0_SET;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARCanSetBtr", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN TX ACK einstellen                                      */
/**************************************************************/
int32_t TARCanSetTxAckEnable(TCanDevice *dev, uint8_t enable)
{
int32_t err;

// **** Speed Einstellen
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_TX_ACK_ENABLE;  // Index
dev->TxDParameter.Data[1] = enable;
dev->Kommando = CMD_CAN0_SET;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARCanSetTxAckEnable", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN LOG ERRORS einstellen                                  */
/**************************************************************/
int32_t TARCanSetErrorLogEnable(TCanDevice *dev, uint8_t enable)
{
int32_t err;

// **** Speed Einstellen
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_ERROR_LOG_ENABLE;  // Index
dev->TxDParameter.Data[1] = enable;
dev->Kommando = CMD_CAN0_SET;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARCanSetErrorLogEnable", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN TxD Boxes sende Intervall einstellen                   */
/**************************************************************/
int32_t TARCanSetIntervall(TCanDevice *dev, uint32_t index, uint32_t intervall)
{
int32_t err;
uint8_t box_index;

if (intervall)
  {
  if (intervall < 1000)
    intervall = 1;
  else
    intervall = intervall / 1000;
  }
box_index = (uint8_t)(index & 0xFF) - 1;
if (box_index > 15)
  return(ERR_INDEX_RANGE);
dev->TxDParameter.Count = 3;
dev->TxDParameter.Data[0] = box_index;
dev->TxDParameter.Data[1] = lo(intervall);
dev->TxDParameter.Data[2] = hi(intervall);
dev->Kommando = CMD_CAN0_TXD_IDX_SET_TIMER;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARCanSetIntervall", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN Controller Reset                                       */
/**************************************************************/
int32_t TARCANReset(TCanDevice *dev)
{
int32_t err;

dev->TxDParameter.Count = 1;
dev->TxDParameter.Data[0] = 0x30;
dev->Kommando = CMD_CAN0_SET_MODE;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARCANReset", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* Empfangs - Fifo löschen                                    */
/**************************************************************/
int32_t TARRxFifoClear(TCanDevice *dev, uint32_t index)
{
int32_t err;
uint8_t fifo_idx;

dev->TxDParameter.Count = 0;
fifo_idx = (uint8_t)(index & 0x000000FFL);
if (!fifo_idx)
  dev->Kommando = CMD_CAN0_RXD_CLEAR;
else
  {
  dev->TxDParameter.Count = 1;
  dev->TxDParameter.Data[0] = fifo_idx;
  dev->Kommando = CMD_CAN0_RXD_IDX_CLEAR;
  }
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARRxFifoClear", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* Sende - Fifo Daten lesen                                   */
/**************************************************************/
int32_t TARTxFifoPut(TCanDevice *dev, uint32_t index, struct TCanMsg *msgs, uint16_t count)
{
int32_t err;
uint16_t cnt;
uint8_t fifo_idx, len;
uint8_t *dest;

err = 0;
fifo_idx = (uint8_t)(index & 0x000000FFL);
if (!fifo_idx)
  {
  dest = &dev->TxDParameter.Data[0];
  cnt = 0;
  }
else
  {
  dev->TxDParameter.Data[0] = fifo_idx;
  dest = &dev->TxDParameter.Data[1];
  cnt = 1;
  }

for (; count; count--)
  {
  len = msgs->MsgLen;
  if (msgs->MsgEFF)
    {
    if (!msgs->MsgRTR)
      *dest++ = len | 0xC0;  // EXT
    else
      {
      *dest++ = len | 0xE0;  // EXT / RTR
      len = 0;
      }
    *(uint32_t *)dest = (uint32_t)msgs->Id;
    dest += 4;
    if (len)
      {
      memcpy(dest, &msgs->MsgData, len);
      dest += len;
      }
    len += 5;
    }
  else
    {
    if (!msgs->MsgRTR)
      {  // STD
      *dest++ = (uint8_t)(((msgs->Id >> 4) & 0x70) | len);
      *dest++ = (uint8_t)msgs->Id;
      memcpy(dest, &msgs->MsgData, len);
      dest += len;
      len += 2;
      }
    else
      {  // STD / RTR
      *dest++ = len | 0xA0;
      *(uint16_t *)dest = (uint16_t)msgs->Id;
      dest += 2;
      len = 3;
      }
    }
  cnt += len;
  msgs++;
  }

dev->TxDParameter.Count = cnt;
if (!fifo_idx)
  dev->Kommando = CMD_CAN0_TXD;
else
  dev->Kommando = CMD_CAN0_TXD_IDX;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARTxFifoPut", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* Sende - Fifo löschen                                       */
/**************************************************************/
int32_t TARTxFifoClear(TCanDevice *dev, uint32_t index)
{
int32_t err;
uint8_t fifo_idx;

dev->TxDParameter.Count = 0;
fifo_idx = (uint8_t)(index & 0x000000FFL);
if (!fifo_idx)
  dev->Kommando = CMD_CAN0_TXD_CLEAR;
else
  {
  dev->TxDParameter.Count = 1;
  dev->TxDParameter.Data[0] = fifo_idx;
  dev->Kommando = CMD_CAN0_TXD_IDX_CLEAR;
  }
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TARTxFifoClear", err, dev->Io->ErrorString);
return(err);
}

