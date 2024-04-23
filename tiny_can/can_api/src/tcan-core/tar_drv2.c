/***************************************************************************
                         tar_drv2.c  -  description
                             -------------------
    begin             : 06.05.2017
    last modify       : 30.04.2019    
    copyright         : (C) 2017 - 2019 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "tar_drv.h"
#include "tcan_com2.h"
#include "tar_drv2.h"


/*

Status:
=======

|    Bit 7    |    Bit 6    |    Bit 5    |    Bit 4    |    Bit 3    |    Bit 2    |    Bit 1    |    Bit 0    |
+=============+=============+=============+=============+=============+=============+=============+=============+
|                                                  Device Status                                                |
|   Power Up  |             |             |             |             |             |             |             | 
+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+


|    Bit 7    |    Bit 6    |    Bit 5    |    Bit 4    |    Bit 3    |    Bit 2    |    Bit 1    |    Bit 0    |
+=============+=============+=============+=============+=============+=============+=============+=============+      
|                                                     CAN 0                                                     |      
|  CAN 0 TxD  |     CAN 0 TxD Status      |  CAN 0 RxD  |  CAN 0 RxD  | Bus-Failure |        CAN 0 Status       |
|    Error    |                           |      OV     |    Pend     |             |                           |
+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+


CAN x Status:

  0 = Controller Ok
  1 = Error warning
  2 = Error passiv
  3 = Bus Off


CAN x TxD Status:
  00 = Empty
  01 = Pend
  10 = Limit ( 1 / 2 Puffergröße überschritten)
  11 = OV

CAN0TxStatus:
=============

|    Bit 7    |    Bit 6    |    Bit 5    |    Bit 4    |    Bit 3    |    Bit 2    |    Bit 1    |    Bit 0    |
+=============+=============+=============+=============+=============+=============+=============+=============+
|                                                     CAN 0                                                     |
|  Tx-Box 7   |  Tx-Box 6   |  Tx-Box 5   |  Tx-Box 4   |  Tx-Box 3   |  Tx-Box 2   |  Tx-Box 1   |  Tx-Box 0   |
|   Error     |   Error     |   Error     |   Error     |   Error     |   Error     |   Error     |   Error     |
+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+


*/


/**************************************************************/
/* CAN Arbeitsmodus einstellen                                */
/**************************************************************/
int32_t TAR2SetCANMode(TCanDevice *dev, uint8_t channel, uint8_t can_mode)
{
int32_t err;

dev->TxDParameter.Count = 1;
dev->TxDParameter.Data[0] = can_mode;
dev->Kommando = CMD2_CANx_SET_MODE[channel];
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2SetCANMode", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN Speed einstellen                                       */
/**************************************************************/
int32_t TAR2CanSetSpeed(TCanDevice *dev, uint8_t channel, uint32_t speed)
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
  dev->Kommando = CMD2_CANx_SET[channel];
  err = SMDrvCommando(dev, 0);
  }
LOG_FUNCTION_ERROR_STR("TAR2CanSetSpeed", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN Speed über BTR Register einstellen                     */
/**************************************************************/
int32_t TAR2CanSetBtr(TCanDevice *dev, uint8_t channel, uint32_t btr)
{
int32_t err;

// **** Speed Einstellen
dev->TxDParameter.Count = 5;
dev->TxDParameter.Data[0] = CAN_VAR_SPEED_BTR;  // Index
SetUInt32ToParam(dev->TxDParameter.Data, 1, btr);
dev->Kommando = CMD2_CANx_SET[channel];
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2CanSetBtr", err, dev->Io->ErrorString);
return(err);
}


/****************************************************************/
/* CAN-FD Speed Setup                                           */
/****************************************************************/
int32_t TAR2CanSetClkNbtrDBtr(TCanDevice *dev, uint8_t channel, uint8_t set_can_clk, uint8_t clk_idx,
     uint32_t nbtr, uint32_t dbtr)
{
int32_t err;

dev->Kommando = CMD2_CANx_SET[channel];
if (set_can_clk)
  {
  // **** CAN Clock
  dev->TxDParameter.Count = 2; 
  dev->TxDParameter.Data[0] = CAN_VAR_CAN_CLOCK; 
  dev->TxDParameter.Data[1] = clk_idx;  
  err = SMDrvCommando(dev, 0);
  LOG_FUNCTION_ERROR_STR("TAR2CanSetClkNbtrDBtr [CanClock]", err, dev->Io->ErrorString);
  if (err)
    return(err);
  }
dev->TxDParameter.Count = 5;
// **** NBTR 
dev->TxDParameter.Data[0] = CAN_VAR_SPEED_BTR;  // Index
SetUInt32ToParam(dev->TxDParameter.Data, 1, nbtr);
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2CanSetClkNbtrDBtr [NBTR]", err, dev->Io->ErrorString);
if (err)
  return(err);
// **** DBTR  
dev->TxDParameter.Data[0] = CAN_VAR_SPEED_DBTR;  // Index
SetUInt32ToParam(dev->TxDParameter.Data, 1, dbtr);
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2CanSetClkNbtrDBtr [DBTR]", err, dev->Io->ErrorString);
return(err);
}

/****************************************************************/
/* Busgeschwindigkeit für Flexible Datarate einstellen (CAN-FD) */
/****************************************************************/
/*int32_t TAR2CanSetDBtr(TCanDevice *dev, uint8_t channel, uint32_t d_btr) // <*> raus
{
int32_t err;

// **** Speed Einstellen
dev->TxDParameter.Count = 5;
dev->TxDParameter.Data[0] = CAN_VAR_SPEED_DBTR;  // Index
SetUInt32ToParam(dev->TxDParameter.Data, 1, d_btr);
dev->Kommando = CMD2_CANx_SET[channel];
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2CanSetDBtr", err, dev->Io->ErrorString);
return(err);
} */


/**************************************************************/
/* CAN-FD Flags einstellen                                    */
/**************************************************************/
int32_t TAR2CanSetFdFlags(TCanDevice *dev, uint8_t channel, uint8_t fd_flags)
{
int32_t err;

// **** Speed Einstellen
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_FD_FLAGS;  // Index
dev->TxDParameter.Data[1] = fd_flags;  
dev->Kommando = CMD2_CANx_SET[channel];
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2CanSetFdFlags", err, dev->Io->ErrorString);
return(err);
}

/**************************************************************/
/* CAN TX ACK einstellen                                      */
/**************************************************************/
int32_t TAR2CanSetTxAckEnable(TCanDevice *dev, uint8_t channel, uint8_t enable)
{
int32_t err;

// **** Speed Einstellen
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_TX_ACK_ENABLE;  // Index
dev->TxDParameter.Data[1] = enable;
dev->Kommando = CMD2_CANx_SET[channel];
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2CanSetTxAckEnable", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN LOG ERRORS einstellen                                  */
/**************************************************************/
int32_t TAR2CanSetErrorLogEnable(TCanDevice *dev, uint8_t channel, uint8_t enable)
{
int32_t err;

// **** Speed Einstellen
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_ERROR_LOG_ENABLE;  // Index
dev->TxDParameter.Data[1] = enable;
dev->Kommando = CMD2_CANx_SET[channel];
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2CanSetErrorLogEnable", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* Tiny-CAN Status abfragen                                   */
/**************************************************************/
int32_t TAR2GetStatus(TCanDevice *dev)
{
int32_t err;
uint32_t i, channels;
struct TTar2Status tar_status;

tar_status.DevStatus.Byte = 0;
for (i = 0; i < 8; i++)
  tar_status.CanStatus[i].Byte = 0;
dev->Kommando = CMD2_GET_STATUS;
if (!(err = SMDrvCommando(dev, 0)))
  {
  channels = dev->ModulDesc->CanChannels;
  if (dev->RxDParameter.Count < (channels + 1))
    err = ERR_SM_DRV_ACK_COUNT;
  else
    {
    tar_status.DevStatus.Byte = dev->RxDParameter.Data[0];
    for (i = 0; i < channels; i++)
      tar_status.CanStatus[i].Byte = dev->RxDParameter.Data[i + 1];
    }
  }  
SetTar2StatusToDev(dev, &tar_status);  
LOG_FUNCTION_ERROR_STR("TAR2GetStatus", err, dev->Io->ErrorString);
return(err);
} 


/**************************************************************/
/* Empfangs-Filter setzten                                    */
/**************************************************************/
int32_t TAR2SetFilter(TCanDevice *dev, uint8_t channel, uint32_t index, struct TMsgFilter *msg_filter)
{
int32_t err;
uint8_t filter_index;

err = 0;
filter_index = (uint8_t)(index & 0xFF) - 1;
if (filter_index > 7)
  return(ERR_INDEX_RANGE);
dev->TxDParameter.Data[0] = CAN_VAR_FILTER0 + filter_index;  // Index  

if (msg_filter->FilEnable)
  {
  if (msg_filter->FilEFF)
    dev->TxDParameter.Data[1] = 0xC0;
  else
    dev->TxDParameter.Data[1] = 0x80;
  }
else
  dev->TxDParameter.Data[1] = 0x00;    
SetUInt32ToParam(dev->TxDParameter.Data, 2, msg_filter->Code);  
SetUInt32ToParam(dev->TxDParameter.Data, 6, msg_filter->Maske);
dev->TxDParameter.Count = 10;
dev->Kommando = CMD2_CANx_SET[channel];
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2SetFilter", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* CAN TxD Boxes sende Intervall einstellen                   */
/**************************************************************/
int32_t TAR2CanSetIntervall(TCanDevice *dev, uint8_t channel, uint32_t index, uint32_t intervall)
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
if (box_index > 7)
  return(ERR_INDEX_RANGE);
dev->TxDParameter.Count = 3;
dev->TxDParameter.Data[0] = CAN_VAR_TXD_BOX0 + box_index;
dev->TxDParameter.Data[1] = lo(intervall);
dev->TxDParameter.Data[2] = hi(intervall);
dev->Kommando = CMD2_CANx_SET[channel];
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2CanSetIntervall", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* Empfangs - Fifo löschen                                    */
/**************************************************************/
int32_t TAR2RxFifoClear(TCanDevice *dev, uint8_t channel, uint32_t index)
{
int32_t err;
uint8_t fifo_idx;

fifo_idx = (uint8_t)(index & 0x000000FFL);
dev->TxDParameter.Count = 1;
dev->TxDParameter.Data[0] = fifo_idx;
dev->Kommando = CMD2_CANx_RXD_IDX_CLEAR[channel];
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2RxFifoClear", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* Sende - Fifo löschen                                       */
/**************************************************************/
int32_t TAR2TxFifoClear(TCanDevice *dev, uint8_t channel, uint32_t index)
{
int32_t err;
uint8_t fifo_idx;

fifo_idx = (uint8_t)(index & 0x000000FFL);
dev->TxDParameter.Count = 1;
dev->TxDParameter.Data[0] = fifo_idx;
dev->Kommando = CMD2_CANx_TXD_IDX_CLEAR[channel];
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2TxFifoClear", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* Hardware TimeStamps aktivieren                             */
/**************************************************************/
int32_t TAR2SetTimeStamp(TCanDevice *dev, uint8_t enabled)
{
int32_t err;

dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_TIMESTAMP_ENABLE;
dev->TxDParameter.Data[1] = enabled;
dev->Kommando = CMD2_CAN0_SET;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2SetTimeStamp", err, dev->Io->ErrorString);
return(err);
}


int32_t TAR2SetFifoOvMode(TCanDevice *dev, uint8_t ov_mode)
{
int32_t err;

dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_FIFO_OV_MODE;
dev->TxDParameter.Data[1] = ov_mode;
dev->Kommando = CMD2_CAN0_SET;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2SetFifoOvMode", err, dev->Io->ErrorString);
return(err);
}


/**************************************************************/
/* I/O Pins konfigurieren                                     */
/**************************************************************/
int32_t TAR2SetIoConfig(TCanDevice *dev, uint8_t index, uint32_t config)
{
int32_t err;

dev->TxDParameter.Count = 5;
dev->TxDParameter.Data[0] = index;
SetUInt32ToParam(dev->TxDParameter.Data, 1, config);
dev->Kommando = CMD_IO_SET;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2SetIoConfig", err, dev->Io->ErrorString);
return(err);
}
