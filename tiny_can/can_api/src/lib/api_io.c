/***************************************************************************
                           api_io.c  -  description
                             -------------------
    begin             : 11.07.2022
    last modified     : 20.07.2022
    copyright         : (C) 2011 - 2022 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <string.h>
#include "errors.h"
#include "can_types.h"
#include "log.h"
#include "util.h"
#include "var_obj.h"
#include "index.h"
#include "rs485.h"
#include "api_io.h"


#ifndef ENABLE_EX_IO_SUPPORT
  #error define ENABLE_EX_IO_SUPPORT when link api_io.c
#endif


int32_t CALLBACK_TYPE CanExIoOpen(uint32_t index, const char *name, uint32_t *io_id)
{
TCanDevice *dev;
TRS485Port *rs485port;
int32_t i, err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExIoOpen, index: 0x%08X, name: %s",
            index, name);
#endif
if ((!name) || (!io_id))
  err = ERR_PARAM;
else
  {
  *io_id = index;
  if (!(dev = get_device_and_ref(index)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    if (!(rs485port = dev->RS485Port))
      err = -1; // <*>
    else
      {
      rs485port->Command = RS485_COMMAND_OPEN;
      for (i = 0; i < 20; i++)
        { 
        if (!rs485port->Command)
          break;
        mhs_sleep(50);
        }            
      }  
    device_unref(dev);
    }
  }  
LOG_API_EXIT("CanExIoOpen", err);
return(err);
}


int32_t CALLBACK_TYPE CanExIoClose(uint32_t io_id)
{
TCanDevice *dev;
TRS485Port *rs485port;
int32_t i, err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExIoClose, io_id: 0x%08X", io_id);
#endif
if (!(dev = get_device_and_ref(io_id)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  if (!(rs485port = dev->RS485Port))
    err = -1; // <*>
  else
    {
    rs485port->Command = RS485_COMMAND_CLOSE;
    for (i = 0; i < 20; i++)
      { 
      if (!rs485port->Command)
        break;
      mhs_sleep(50);
      }
    }
  device_unref(dev);
  }
LOG_API_EXIT("CanExIoClose", err);
return(err);
}
   

int32_t CALLBACK_TYPE CanExIoWrite(uint32_t io_id, void *data, uint32_t *size, uint32_t timeout)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExIoWrite, io_id: 0x%08X", io_id);
#endif
if ((!data) || (!size))
  err = ERR_PARAM;
else
  {
  if (!(dev = get_device_and_ref(io_id)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    err = RS485MsIoRw(dev, (TRS485MsData *)data, *size, timeout);
    device_unref(dev);
    }  
  }  
LOG_API_EXIT("CanExIoWrite", err);
return(err);
}


int32_t CALLBACK_TYPE CanExIoRead(uint32_t io_id, void *data, uint32_t *size, uint32_t timeout)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExIoRead, io_id: 0x%08X", io_id);
#endif
if ((!data) || (!size))
  err = ERR_PARAM;
else
  {
  if (!(dev = get_device_and_ref(io_id)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    err = RS485IoRead(dev, data, size, timeout);
    device_unref(dev);
    }
  }  
LOG_API_EXIT("CanExIoRead", err);
return(err);
}