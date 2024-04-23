/***************************************************************************
                         ecu_flash.c  -  description
                             -------------------
    begin             : 06.05.2017
    last modify       : 07.08.2021    
    copyright         : (C) 2017 - 2021 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "ecu_flash.h"

struct TEcuFlashCfgData
  {
  uint8_t IsoTpFlags;
  uint8_t IsoTpTxPadContent;
  uint32_t IsoTpTxId;
  uint32_t IsoTpRxId;
  uint8_t CanDataOkDlc;  
  uint8_t CanDataErrorDlc;  
  uint8_t CanDataOkMask[8]; 
  uint8_t CanDataOkCode[8];
  uint8_t CanDataErrorMask[8];
  uint8_t CanDataErrorCode[8];
  };


static void SetCanDataBuf(TObjValue *obj, void *user_data)
{
TMhsObjContext *context;
TCanDevice *dev;
uint8_t *data_ptr;

if ((!obj) || (!user_data))
  return;
dev = (TCanDevice *)user_data;
context = dev->Context;
data_ptr = (uint8_t *)obj->Value.Ptr;
dev->CanDataBuf = data_ptr;
dev->CanDataExec = 0;
dev->CanDataBufWritten = 0;
if (!data_ptr)  
  dev->CanDataBufSize = 0;  
else
  dev->CanDataBufSize = mhs_value_get_as_ulong("CanDataBufSize", ACCESS_INIT, context);    
}

    
void EcuFlashCreateDevice(TCanDevice *dev)
{
TMhsObjContext *context;
TMhsObj *obj;

context = dev->Context;
obj = mhs_object_get_by_name("CanDataBuf", context);
mhs_object_cmd_event_connect(obj, 1, (TMhsObjectCB)&SetCanDataBuf, (void *)dev);
}

     
int32_t TAR2SetEcuFlashVars(TCanDevice *dev)
{
int32_t i, err;
TMhsObjContext *context;
struct TEcuFlashCfgData cfg;
uint8_t *data;

context = dev->Context;
cfg.IsoTpFlags = mhs_value_get_as_ubyte("IsoTpFlags", ACCESS_INIT, context);       
cfg.IsoTpTxPadContent = mhs_value_get_as_ubyte("IsoTpTxPadContent", ACCESS_INIT, context);
cfg.IsoTpTxId = mhs_value_get_as_ulong("IsoTpTxId", ACCESS_INIT, context);        
cfg.IsoTpRxId = mhs_value_get_as_ulong("IsoTpRxId", ACCESS_INIT, context);
cfg.CanDataOkDlc = mhs_value_get_as_ubyte("CanDataOkDlc", ACCESS_INIT, context);  
cfg.CanDataErrorDlc = mhs_value_get_as_ubyte("CanDataErrorDlc", ACCESS_INIT, context);  
mhs_value_get_as_ubyte_array(&data, "CanDataOkMask", ACCESS_INIT, context);        
memcpy(cfg.CanDataOkMask, data, 8);       
mhs_value_get_as_ubyte_array(&data, "CanDataOkCode", ACCESS_INIT, context);
for (i = 0; i < cfg.CanDataOkDlc; i++)
  cfg.CanDataOkCode[i] = cfg.CanDataOkMask[i] & data[i];
for (; i < 8; i++)
  {
  cfg.CanDataOkCode[i] = 0;
  cfg.CanDataOkMask[i] = 0;
  }      
//memcpy(cfg.CanDataOkCode, data, 8); <*> raus    
mhs_value_get_as_ubyte_array(&data, "CanDataErrorMask", ACCESS_INIT, context);    
memcpy(cfg.CanDataErrorMask, data, 8);     
mhs_value_get_as_ubyte_array(&data, "CanDataErrorCode", ACCESS_INIT, context);
for (i = 0; i < cfg.CanDataErrorDlc; i++)
  cfg.CanDataErrorCode[i] = cfg.CanDataErrorMask[i] & data[i];
for (; i < 8; i++)
  {
  cfg.CanDataErrorCode[i] = 0;
  cfg.CanDataErrorMask[i] = 0;
  }  
//memcpy(cfg.CanDataErrorCode, data, 8); <*> raus 
dev->IsoTpBlockSize = mhs_value_get_as_ulong("IsoTpBlockSize", ACCESS_INIT, context);    
// **** IsoTpFlags
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_ISO_TP_FLAGS;
dev->TxDParameter.Data[1] = cfg.IsoTpFlags;
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2SetEcuFlashVars", err, dev->Io->ErrorString);
  return(err);
  }
// **** IsoTpTxPadContent
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_ISO_TP_TX_PAD_CONTENT;
dev->TxDParameter.Data[1] = cfg.IsoTpTxPadContent;
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2SetEcuFlashVars", err, dev->Io->ErrorString);
  return(err);
  }
// **** IsoTpTxId 
dev->TxDParameter.Count = 5;
dev->TxDParameter.Data[0] = CAN_VAR_ISO_TP_TX_ID;
SetUInt32ToParam(dev->TxDParameter.Data, 1, cfg.IsoTpTxId);
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2SetEcuFlashVars", err, dev->Io->ErrorString);
  return(err);
  }  
 // **** IsoTpRxId
dev->TxDParameter.Count = 5;
dev->TxDParameter.Data[0] = CAN_VAR_ISO_TP_RX_ID;
SetUInt32ToParam(dev->TxDParameter.Data, 1, cfg.IsoTpRxId);
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2SetEcuFlashVars", err, dev->Io->ErrorString);
  return(err);
  }
// **** CanDataOkDlc
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_ECU_FL_DATA_OK_DLC;
dev->TxDParameter.Data[1] = cfg.CanDataOkDlc;
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2SetEcuFlashVars", err, dev->Io->ErrorString);
  return(err);
  }
// **** CanDataOkMask
dev->TxDParameter.Count = 9;
dev->TxDParameter.Data[0] = CAN_VAR_ECU_FL_DATA_OK_MASK;
memcpy(&dev->TxDParameter.Data[1], cfg.CanDataOkMask, 8);
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2SetEcuFlashVars", err, dev->Io->ErrorString);
  return(err);
  }
// **** CanDataOkCode
dev->TxDParameter.Count = 9;
dev->TxDParameter.Data[0] = CAN_VAR_ECU_FL_DATA_OK_CODE;
memcpy(&dev->TxDParameter.Data[1], cfg.CanDataOkCode, 8);
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2SetEcuFlashVars", err, dev->Io->ErrorString);
  return(err);
  } 
// **** CanDataErrorDlc
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_ECU_FL_DATA_ERROR_DLC;
dev->TxDParameter.Data[1] = cfg.CanDataErrorDlc;
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2SetEcuFlashVars", err, dev->Io->ErrorString);
  return(err);
  }  
// **** CanDataErrorMask
dev->TxDParameter.Count = 9;
dev->TxDParameter.Data[0] = CAN_VAR_ECU_FL_DATA_ERROR_MASK;
memcpy(&dev->TxDParameter.Data[1], cfg.CanDataErrorMask, 8);
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2SetEcuFlashVars", err, dev->Io->ErrorString);
  return(err);
  } 
// **** CanDataErrorCode
dev->TxDParameter.Count = 9;
dev->TxDParameter.Data[0] = CAN_VAR_ECU_FL_DATA_ERROR_CODE;
memcpy(&dev->TxDParameter.Data[1], cfg.CanDataErrorCode, 8);
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2SetEcuFlashVars", err, dev->Io->ErrorString);
  return(err);
  }
return(0);           
}
