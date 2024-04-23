/***************************************************************************
                         can_test.c  -  description
                             -------------------
    begin             : 06.05.2017
    last modify       : 08.08.2021    
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
#include "can_test.h"


static int32_t TAR2CanTestSetMode(TCanDevice *dev, uint16_t mode)
{
int32_t err;

dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = lo(mode);
dev->TxDParameter.Data[1] = hi(mode);
dev->Kommando = CMD_CAN_TEST_SET;
err = SMDrvCommando(dev, 0);
LOG_FUNCTION_ERROR_STR("TAR2CanTestSetMode", err, dev->Io->ErrorString);
return(err);
}


int32_t TAR2CanTestExecute(TCanDevice *dev, uint8_t enable)
{
int32_t err;
TMhsObjContext *context;
uint32_t lvalue;
uint8_t bvalue;

context = dev->Context;
if ((err = TAR2CanTestSetMode(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2CanTestExecute", err, dev->Io->ErrorString);
  return(err);
  }
if (!enable)
  return(0);  
// **** CanTestMode
/*bvalue = mhs_value_get_as_ubyte("CanTestMode", ACCESS_INIT, context); <*>
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_CAN_TEST_MODE;
dev->TxDParameter.Data[1] = bvalue;
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2CanTestExecute", err, dev->Io->ErrorString);
  return(err);
  }   */
// **** CanTestFillChar
bvalue = mhs_value_get_as_ubyte("CanTestFillChar", ACCESS_INIT, context);
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_CAN_TEST_FILL_CHAR;
dev->TxDParameter.Data[1] = bvalue;
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2CanTestExecute", err, dev->Io->ErrorString);
  return(err);
  }
// **** CanTestMsgFlags
bvalue = mhs_value_get_as_ubyte("CanTestMsgFlags", ACCESS_INIT, context);
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_CAN_TEST_MSG_FLAGS;
dev->TxDParameter.Data[1] = bvalue;
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2CanTestExecute", err, dev->Io->ErrorString);
  return(err);
  }    
// **** CanTestMsgLength
bvalue = mhs_value_get_as_ubyte("CanTestMsgLength", ACCESS_INIT, context);
dev->TxDParameter.Count = 2;
dev->TxDParameter.Data[0] = CAN_VAR_CAN_TEST_MSG_LENGTH;
dev->TxDParameter.Data[1] = bvalue;
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2CanTestExecute", err, dev->Io->ErrorString);
  return(err);
  }
// **** CanTestMsgId 
lvalue = mhs_value_get_as_ulong("CanTestMsgId", ACCESS_INIT, context);
dev->TxDParameter.Count = 5;
dev->TxDParameter.Data[0] = CAN_VAR_CAN_TEST_MSG_ID;
SetUInt32ToParam(dev->TxDParameter.Data, 1, lvalue);
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2CanTestExecute", err, dev->Io->ErrorString);
  return(err);
  }  
// **** CanTestCounter
lvalue = mhs_value_get_as_ulong("CanTestCounter", ACCESS_INIT, context);
dev->TxDParameter.Count = 5;
dev->TxDParameter.Data[0] = CAN_VAR_CAN_TEST_COUNTER;
SetUInt32ToParam(dev->TxDParameter.Data, 1, lvalue);
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2CanTestExecute", err, dev->Io->ErrorString);
  return(err);
  }
// **** CanTestDelay
lvalue = mhs_value_get_as_ulong("CanTestDelay", ACCESS_INIT, context);
dev->TxDParameter.Count = 5;
dev->TxDParameter.Data[0] = CAN_VAR_CAN_TEST_DELAY;
SetUInt32ToParam(dev->TxDParameter.Data, 1, lvalue);
dev->Kommando = CMD2_CAN0_SET;
if ((err = SMDrvCommando(dev, 0)))
  {
  LOG_FUNCTION_ERROR_STR("TAR2CanTestExecute", err, dev->Io->ErrorString);
  return(err);
  }  
  
bvalue = mhs_value_get_as_ubyte("CanTestMode", ACCESS_INIT, context);  
return(TAR2CanTestSetMode(dev, (uint16_t)bvalue));  
}  