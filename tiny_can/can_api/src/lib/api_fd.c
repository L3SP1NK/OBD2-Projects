/***************************************************************************
                          api_fd.c  -  description
                             -------------------
    begin             : 10.11.2019
    last modified     : 19.04.2022    
    copyright         : (C) 2019 - 2022 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "config.h"
#include "global.h"
#include <stdio.h>
#include <string.h>
#include "errors.h"
#include "can_types.h"
#include "log.h"
#include "util.h"
#include "var_obj.h"
#include "index.h"
#include "drv_info.h"
#include "can_core.h"
#include "can_obj.h"
#include "can_fifo.h"
#include "can_puffer.h"
#include "mhs_cmd.h"
#include "api_fd.h"


/***************************************************************/
/*  CAN-FD Telegramm(e) senden                                 */
/***************************************************************/
int32_t CALLBACK_TYPE CanFdTransmit(uint32_t index, struct TCanFdMsg *fd_msg, int32_t count)
{
TCanDevice *dev;
int32_t err, i;
struct TCanFdMsg *m;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
index |= INDEX_TXT_FLAG;
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogCanFdMessages(LOG_API_CALL, fd_msg, count, LOG_FMT_FD_TYPE,
      "API-Call Enter: CanTransmit, Index: %#08X, Messages: %d", index, count);
#endif
m = fd_msg;
err = 0;
for (i = count; i; i--)
  {
  if (DriverInit == TCAN_DRV_FD_INIT)
    {
    if (m->MsgFD)
      {
      if (m->MsgLen > 64)
        {
        err = ERR_CAN_DLC_OVERFLOW; 
        break;
        }
      }
    else
      {
      if (m->MsgLen > 8)
        {
        err = ERR_CAN_DLC_OVERFLOW; 
        break;
        }
      }      
    }
  else
    {
    if ((m->MsgFD) || (m->MsgBRS))
      {
      err = ERR_NOT_IN_FD_MODE_INIT;
      break;
      }  
    if (m->MsgLen > 8)
      {
      err = ERR_CAN_DLC_OVERFLOW;
      break;
      }
    }
  m++;      
  }
if (!err)
  {   
  if (!(dev = get_device_and_ref(index)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    err = can_put_fd_msgs_api(index, fd_msg, count);
    device_unref(dev);
    }
  }
LOG_API_EXIT("CanTransmit", err);
return(err);
}


/***************************************************************/
/*  CAN-FD Telegramm(e) empfangen                                 */
/***************************************************************/
int32_t CALLBACK_TYPE CanFdReceive(uint32_t index, struct TCanFdMsg *fd_msg, int32_t count)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogPrintf(LOG_API_CALL, "API-Call Enter: CanReceive, index: %#08X, count: %d", index, count);
 #endif
dev = get_device_and_ref(index);
err = can_get_fd_msgs_api(index, fd_msg, count);
device_unref(dev);
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  {
  if (err >= 0) 
    LogCanFdMessages(LOG_API_CALL, fd_msg, err, LOG_FMT_FD_TYPE_SOURCE,
        "API-Call Exit: CanReceive, count: %d", err);
  else
    LogPrintf(LOG_API_CALL | LOG_ERROR, "API-Call Exit: CanReceive, result: %s", GetErrorString(err));
  }
#endif
return(err);
}