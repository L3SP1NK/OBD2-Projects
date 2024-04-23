/*******************************************************************************
                           index.c  -  description
                             -------------------
    begin             : 07.01.2011
    copyright         : (C) 2011 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ******************************************************************************/

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
#include "errors.h"
#include "util.h"
#include "can_fifo.h"
//#include "tcan_drv.h" <*>
#include "index.h"


TCanDevice *DeviceList[16];
static DRV_LOCK_TYPE *DeviceLock = NULL;


void index_init(void)
{
uint32_t i;

DeviceLock = NULL;
for (i = 0; i < 16; i++)
  DeviceList[i] = NULL;
}



void index_create(void)
{
DRV_LOCK_INIT(DeviceLock);
}


void index_destroy(void)
{
DRV_LOCK_DESTROY(DeviceLock);
}


uint32_t create_new_device_index(TCanDevice *device)
{
uint32_t index;
int32_t i, free;
TCanDevice *dev;

free = -1;
index = INDEX_INVALID;
DRV_LOCK_ENTER(DeviceLock);
for (i = 0; i < 16; i++)
  {
  dev = DeviceList[i];
  if ((free < 0) && (!dev))
    free = i;
  if (dev == device)
    {
    index = convert_idx_to_device_idx(i);
    break;
    }
  }  
if ((index == INDEX_INVALID) && (free > -1))
  {
  DeviceList[free] = device;
  index = convert_idx_to_device_idx(free);
  }
DRV_LOCK_LEAVE(DeviceLock);  
return(index);
}


TCanDevice *get_device_and_ref(uint32_t index)
{
int32_t idx;
TCanDevice *dev;

if ((index == INDEX_INVALID) || (index & INDEX_USER_MASK))
  return(NULL);  
idx = convert_device_idx_to_idx(index);
DRV_LOCK_ENTER(DeviceLock);
if ((dev = DeviceList[idx]))
  dev->RefCount++;
DRV_LOCK_LEAVE(DeviceLock);
return(dev);
}


void device_unref(TCanDevice *dev)
{
if (!dev)
  return;
DRV_LOCK_ENTER(DeviceLock);
if (dev->RefCount)
  dev->RefCount--;
DRV_LOCK_LEAVE(DeviceLock);
}



TCanDevice *device_create(void)
{
TCanDevice *dev;

if (!(dev = (TCanDevice *)mhs_calloc(1, sizeof(TCanDevice))))
  return(NULL);
dev->Index = create_new_device_index(dev);
if (dev->Index == INDEX_INVALID)
  safe_free(dev);
return(dev);
}


int32_t device_destroy(uint32_t index, uint32_t block)
{
int32_t idx, res;
uint32_t cnt;
TCanDevice *dev;

res = 0;
if (index == INDEX_INVALID)
  return(0);
idx = convert_device_idx_to_idx(index);
DRV_LOCK_ENTER(DeviceLock);
if ((dev = DeviceList[idx]))  
  {
  if (!block)
    {
    if (dev->RefCount)
      res = ERR_DEVICE_BUSY;
    }
  if (!res)    
    DeviceList[idx] = NULL;
  }  
DRV_LOCK_LEAVE(DeviceLock);
if ((!dev) || (res))
  return(res);
    
MhsEventListDestroy(&dev->StatusEventList);
CloseCanDevice(dev);  
if (mhs_value_get_as_ulong("CanRxDFifoSize", ACCESS_INIT, dev->Context))
  mhs_can_fifo_destry(dev->Index);
if (mhs_value_get_as_ulong("CanTxDFifoSize", ACCESS_INIT, dev->Context))
  mhs_can_fifo_destry(dev->Index | INDEX_TXT_FLAG);
cnt = 20; // Wartezeit 50ms * 20 = 1 Sek.  
do
  {  
  if (!dev->RefCount)
    break;
  mhs_sleep(50);  // 50ms warten  
  }
while (--cnt);
if (!cnt)
  res = ERR_DEVICE_BUSY;   
DestroyCanDevice(&dev);
return(res);
}

