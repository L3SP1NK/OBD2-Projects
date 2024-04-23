/***************************************************************************
                           api_cp.c  -  description
                             -------------------
    begin             : 22.01.2011
    last modified     : 19.04.2022    
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
#include "errors.h"
#include "can_types.h"
#include "log.h"
#include "util.h"
#include "mhs_thread.h"
#include "index.h"
#include "can_core.h"
#include "can_obj.h"
#include "can_fifo.h"
#include "can_puffer.h"
//#include "tcan_drv.h" <*>
#ifndef DISABLE_PNP_SUPPORT
  #if defined(MHSIPCAN_DRV)
    #include "net_pnp.h"
  #elif defined(MHSPASSTHRU_DRV)
    #include "passthru_pnp.h"    
  #else  
    #include "usb_hlp.h"
    #include "usb_pnp.h"
  #endif  
#endif
#include "api_cp.h"

//static int32_t LastPnPStatus = -1;
static TMhsThread *CallThread = NULL;
static uint32_t MinEventSleepTime = 5;
static uint32_t ApiCpCanRxDBufferSize = 0;
static struct TCanMsg *ApiCpCanRxDBuffer = NULL;


static void (CALLBACK_TYPE *CanPnPEventCallback)(uint32_t index, int32_t status);    // PNP Event
static void (CALLBACK_TYPE *CanStatusEventCallback)(uint32_t index,
                                    struct TDeviceStatus *device_status);     // Status Change
static void (CALLBACK_TYPE *CanRxEventCallback)(uint32_t index,
                                    struct TCanMsg *msg, int32_t count);      // Rx Event


static void CallThreadExecute(TMhsThread *thread);
static void ExCallThreadExecute(TMhsThread *thread);


void ApiCpInit(void)
{
CallThread = NULL;
MinEventSleepTime = 5;
ApiCpCanRxDBufferSize = 0;
ApiCpCanRxDBuffer = NULL;
}


/***************************************************************/
/*                                                             */
/***************************************************************/
void CALLBACK_TYPE CanSetPnPEventCallback(void (CALLBACK_TYPE *event)(uint32_t index, int32_t status))
{
CanPnPEventCallback = event;
LOG_API_EXIT("CanSetPnPEventCallback", 0);
}


void CALLBACK_TYPE CanSetStatusEventCallback(void (CALLBACK_TYPE *event)
  (uint32_t index, struct TDeviceStatus *DeviceStatus))
{
CanStatusEventCallback = event;
LOG_API_EXIT("CanSetStatusEventCallback", 0);
}


void CALLBACK_TYPE CanSetRxEventCallback(void (CALLBACK_TYPE *event)
  (uint32_t index, struct TCanMsg *msg, int32_t count))
{
CanRxEventCallback = event;
LOG_API_EXIT("CanSetRxEventCallback", 0);
}


/***************************************************************/
/*                                                             */
/***************************************************************/
void CALLBACK_TYPE CanSetEvents(uint16_t events)
{
uint32_t event_mask;
int32_t err;

err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanSetEvents, events: %#04X", events);
#endif
if (!CallThread)
  err = ERR_PUBLIC_ERROR;
else
  {
  event_mask = mhs_event_get_event_mask((TMhsEvent *)CallThread);
  if (events)
    {
    if (events & EVENT_ENABLE_PNP_CHANGE)
      event_mask |= EVENT_PNP_CHANGE;                 // PnP Event freigen
    if (events & EVENT_DISABLE_PNP_CHANGE)
      event_mask &= (~EVENT_PNP_CHANGE);              // PnP Event sperren

    if (events & EVENT_ENABLE_STATUS_CHANGE)
      event_mask |= EVENT_STATUS_CHANGE;              // Status Change Event freigeben
    if (events & EVENT_DISABLE_STATUS_CHANGE)
      event_mask &= (~EVENT_STATUS_CHANGE);           // Status Change Event sperren

    if (events & EVENT_ENABLE_RX_FILTER_MESSAGES)
      event_mask |= EVENT_RX_FILTER_MESSAGES;         // Rx Filter Message Event freigeben
    if (events & EVENT_DISABLE_RX_FILTER_MESSAGES)
      event_mask &= (~EVENT_RX_FILTER_MESSAGES);      // Rx Filter Message Event sperren

    if (events & EVENT_ENABLE_RX_MESSAGES)
      event_mask |= EVENT_RX_MESSAGES;                // Rx Messages Event freigeben
    if (events & EVENT_DISABLE_RX_MESSAGES)
      event_mask &= (~EVENT_RX_MESSAGES);             // Rx Messages Event sperren
    mhs_event_set_event_mask((TMhsEvent *)CallThread, event_mask);
    }
  else
    {
    mhs_event_set_event_mask((TMhsEvent *)CallThread, 0);
    mhs_event_clear((TMhsEvent *)CallThread, 0x7FFFFFFF);
    }
  }
LOG_API_EXIT("CanSetEvents", err);
}


/***************************************************************/
/*                                                             */
/***************************************************************/
uint32_t CALLBACK_TYPE CanEventStatus(void)
{
uint32_t status;

LOG_API_ENTER("CanEventStatus");
if (!CallThread)
  return(3);
status = mhs_wait_status((TMhsEvent *)CallThread);
if ((!(status & MHS_EVENT_STATUS_INIT))
   || (!(status & MHS_EVENT_STATUS_PEND))
   || (status & MHS_EVENT_STATUS_TERMINATE))
  return(1);
else
  return(0);
}


TMhsThread *CreateApiCp(uint32_t priority, uint32_t min_sleep_time)
{
if ((CallThread = mhs_create_thread(CallThreadExecute, NULL, priority, 0)))
  {
  MinEventSleepTime = min_sleep_time;
  }
return(CallThread);
}


int32_t RunApiCp(TMhsThread *thread, TCanDevice *dev)
{
return(mhs_run_thread_ex(thread, dev));
}

TMhsThread *ExCreateApiCp(uint32_t priority, uint32_t min_sleep_time)
{
if ((CallThread = mhs_create_thread(ExCallThreadExecute, NULL, priority, 1)))
  {
  MinEventSleepTime = min_sleep_time;
  }
return(CallThread);
}


TMhsThread *GetApiCpTheread(void)
{
return(CallThread);
}


void DestoryApiCp(void)
{
mhs_destroy_thread(&CallThread, 0);
safe_free(ApiCpCanRxDBuffer);
ApiCpCanRxDBufferSize = 0;
}


int32_t ApiCpCreateRxDBuffer(uint32_t size)
{
safe_free(ApiCpCanRxDBuffer);
ApiCpCanRxDBufferSize = 0;
if (!size)
  return(0);
ApiCpCanRxDBuffer = mhs_malloc(size * sizeof(struct TCanMsg));
if (!ApiCpCanRxDBuffer)
  {
  ApiCpCanRxDBufferSize = 0;
  return(ERR_ALLOCATE_RESOURCES);
  }
else
  {
  ApiCpCanRxDBufferSize = size;
  return(0);
  }
}


/***************************************************************/
/*  Callback Thread                                            */
/***************************************************************/
static void CallThreadExecute(TMhsThread *thread)
{
TMhsObjContext *context;
TMhsObj *obj;
uint32_t events, i;
#ifndef DISABLE_PNP_SUPPORT
int32_t pnp;
#endif
//int32_t  connect;
struct TCanMsg msg;
TCanDevice *dev;

dev = (TCanDevice *)thread->Data;
// ****** Thread Initialisieren ******
// ****** Thread Schleife ******
do
  {
  mhs_sleep(MinEventSleepTime);
  events = mhs_wait_for_event((TMhsEvent *)thread, 0);
#ifndef DISABLE_PNP_SUPPORT
  if (events & EVENT_PNP_CHANGE)
    {
    // ***** Plug & Play Event
    if (CanPnPEventCallback)
      {
      pnp = GetDevicePnPStatus();
      if (pnp >= 0)
        {
        (CanPnPEventCallback)(0, (uint32_t)pnp);
        }
      }
    }
#endif
  // ***** Status ï¿½nderung A
  if ((events & EVENT_STATUS_CHANGE) && (CanStatusEventCallback))
    (CanStatusEventCallback)(0, &dev->DeviceStatus);
  // ***** Receive Filter Message A
  if ((events & EVENT_RX_FILTER_MESSAGES) && (CanRxEventCallback))
    {
    context = can_filter_get_context();
    if ((obj = mhs_object_get_marked(context)))
      {
      if (obj->Class == OBJ_CAN_RX_PUFFER)
        {
        (void)mhs_can_puffer_get((TObjCanPuffer *)obj, &msg);
        (CanRxEventCallback)(obj->Index, &msg, 1);
        }
      }
    // Hardware Filter abfragen
#if RX_FILTER_MAX > 0
    context = dev->RxFilterContext;
    for (i = 1; i <= RX_FILTER_MAX; i++)  // dev->ModulDesc->HwRxFilterCount
      {
      if ((obj = mhs_object_get_by_index(i, context)))
        {
        if (((TObjCan *)obj)->Filter.FilEnable)
          {
          if (mhs_can_puffer_count((TObjCanPuffer *)obj) > 0)
            {
            (void)mhs_can_puffer_get((TObjCanPuffer *)obj, &msg);
            (CanRxEventCallback)(i, &msg, 1);
            }
          }
        }
      }
#endif
    }
  // ***** Receive CAN Message A
  if ((events & EVENT_RX_MESSAGES) && (CanRxEventCallback))
    {
    if (dev->CanRxDBuffer)
      {
      if ((i = mhs_can_fifo_get_by_index(0, dev->CanRxDBuffer, dev->CanRxDBufferSize)) > 0)
        (CanRxEventCallback)(0, dev->CanRxDBuffer, i);
      }
    else
      (CanRxEventCallback)(0, NULL, 0);
    }
  }
while (thread->Run);
// ****** Thread beenden ******
}


/***************************************************************/
/*  Ex Callback Thread                                         */
/***************************************************************/
static void ExCallThreadExecute(TMhsThread *thread)
{
TMhsObjContext *context;
TMhsObj *obj;
uint32_t events, i, idx;
TCanDevice *dev;
struct TCanMsg msg;

// ****** Thread Initialisieren ******
// ****** Thread Schleife ******
do
  {
  mhs_sleep(MinEventSleepTime);
  events = mhs_wait_for_event((TMhsEvent *)thread, 0);
  // ***** Plug & Play Event
  if ((events & EVENT_PNP_CHANGE) && (CanPnPEventCallback))
    (CanPnPEventCallback)(0, 0);
  // ***** Status Änderung
  for (i = 0; i < 16; i++)
    {
    if ((dev = DeviceList[i]))
      {
      if (dev->StatusChange)
        {
        dev->StatusChange = 0;
        if ((events & EVENT_STATUS_CHANGE) && (CanStatusEventCallback))
          (CanStatusEventCallback)(convert_idx_to_device_idx(i), &dev->DeviceStatus);
        }
      }
    }
  // ***** Receive Filter Message A
  if ((events & EVENT_RX_FILTER_MESSAGES) && (CanRxEventCallback))
    {
    context = can_filter_get_context();
    if ((obj = mhs_object_get_marked(context)))
      {
      if (obj->Class == OBJ_CAN_RX_PUFFER)
        {
        (void)mhs_can_puffer_get((TObjCanPuffer *)obj, &msg);
        (CanRxEventCallback)(obj->Index, &msg, 1);
        }
      }
    // Hardware Filter abfragen
#if RX_FILTER_MAX > 0
    context = dev->RxFilterContext;
    if (dev->ModulDesc)
      {
      for (i = 1; i <= dev->ModulDesc->HwRxFilterCount; i++)
        {
        if ((obj = mhs_object_get_by_index(i, context)))
          {
          if (mhs_can_puffer_count((TObjCanPuffer *)obj) > 0)
            {
            (void)mhs_can_puffer_get((TObjCanPuffer *)obj, &msg);
            (CanRxEventCallback)(i, &msg, 1);
            }
          }
        }
      }
#endif
    }

  // ***** Receive CAN Message A
  if ((events & EVENT_RX_MESSAGES) && (CanRxEventCallback))
    {
    if ((obj = mhs_object_get_marked(can_core_get_context())))
      {
      if (obj->Class == OBJ_CAN_FIFO)
        {
        idx = obj->Index;
        if (ApiCpCanRxDBuffer)
          {
          if ((i = mhs_can_fifo_get_by_index(idx, ApiCpCanRxDBuffer, ApiCpCanRxDBufferSize)) > 0)
            (CanRxEventCallback)(idx, ApiCpCanRxDBuffer, i);
          }
        else
          (CanRxEventCallback)(idx, NULL, 0);
        }
      }
    }
  }
while (thread->Run);
// ****** Thread beenden ******
}
