/*******************************************************************************
                         can_rx_obj.c  -  description
                             -------------------
    begin             : 08.01.2011
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
#include <string.h>
#include "util.h"
#include "api_cp.h"
#include "mhs_obj.h"
#include "mhs_class.h"
#include "index.h"
#include "can_core.h"
#include "can_fifo.h"
#include "can_puffer.h"
#include "errors.h"
#ifdef MAIN_THREAD_USE_FILE_EVENT
#include "mhs_file_event.h"
#endif
#include "can_obj.h"


/*uint32_t index_to_channels(union TCanIndex index)
{
uint32_t ch_idx, dev_idx;

ch_idx = index.Item.Channel;
dev_idx = index.Item.Device;
dev_idx = 1 << dev_idx;
ch_idx = dev_idx << (ch_idx * 6);
return(ch_idx);
}*/

uint32_t index_to_channels(uint32_t index)
{
union TCanIndex idx;
uint32_t ch_idx, dev_idx;

idx.Long = index;
ch_idx = idx.Item.Channel;
dev_idx = idx.Item.Device;
dev_idx = 1 << dev_idx;
ch_idx = dev_idx << (ch_idx * 6);
return(ch_idx);
}


static void mhs_fifo_auto_ov(TObjCanFifo *fifo)
{
struct TCanFdMsg ov_msg;
uint16_t msgs_lost;

if (fifo->overrun)
  {
  (void)mhs_can_fifo_clear_unlocked(fifo);
  if (fifo->bufsize > 0xFFFF)
    msgs_lost = 0xFFFF;
  else
    msgs_lost = (uint16_t)fifo->bufsize;
  (void)memset(&ov_msg, 0, sizeof(struct TCanFdMsg));
  ov_msg.MsgOV = 1;
  ov_msg.MsgLen = 3;
  ov_msg.MsgData[0] = 3;
  ov_msg.MsgData[1] = lo(msgs_lost); // Messages Lost
  ov_msg.MsgData[2] = hi(msgs_lost);
  (void)mhs_can_fifo_put_fd_unlocked(fifo, &ov_msg, 1);
  }
}

/*
******************** process_rx_msg ********************
Funktion  : Empfangende Nachrichten verarbeiten

Eingaben  : index => CAN-Device & CAN-Kanal
            msg => CAN Nachricht

Ausgaben  : result => Fehlercode 0 = kein Fehler
                                -1 = Fehler
                                 1 = Message gefiltert
*/
int32_t process_rx_msg(uint32_t index, struct TCanFdMsg *fd_msg)
{
int32_t hit;
#if RX_FILTER_MAX > 0
uint32_t i;
#endif
TMhsObjContext *filter_context, *context;
TMhsObj *obj;
TObjCan *can_obj;
TCanDevice *dev;
uint32_t channels, pass_filter, fill_id_mode;

hit = 0;
channels = index_to_channels(index);
fd_msg->MsgSource = (fd_msg->MsgSource & 0x0F) | ((index >> 16) & 0xF0);
fd_msg->MsgFilHit = 0;
context = can_core_get_context();
filter_context = can_filter_get_context();
dev = get_device(index);
if ((!context) || (!filter_context) || (!dev))
  return(-1);
mhs_enter_critical(filter_context->Event);
hit = 0;
pass_filter = 0; // <*> verschoben
if (!((fd_msg->MsgTxD) || (fd_msg->MsgErr)))
  {
#if RX_FILTER_MAX > 0
  // Nicht vorhandene Hardware Filter werden duch Software Filter simmuliert
  if (dev->ModulDesc)
    i = dev->ModulDesc->HwRxFilterCount;
  else
    i = 0;
  if (i < RX_FILTER_MAX)
    {
    for (obj = mhs_object_get_by_index(++i, dev->RxFilterContext); obj; obj = obj->Next)
      {
      can_obj = (TObjCan *)obj;
      if ((can_obj->Filter.FilEnable) &&
          (can_obj->Filter.FilEFF == fd_msg->MsgEFF) &&
          (can_obj->Filter.FilRTR == fd_msg->MsgRTR))
        {
        if (((fd_msg->Id ^ can_obj->Filter.Code) & can_obj->Filter.Maske) == 0)
          {
          hit = 1;
          break;
          }
        }
      }
    if (hit)
      {
      (void)mhs_can_puffer_put_fd_unlocked((TObjCanPuffer *)obj, fd_msg);
      (void)mhs_object_set_event_unlocked((TMhsObj *)obj);
      }
    }
#endif
// **** Pass Filter
  //pass_filter = 0;  // <*> alt
  if (!hit)
    {
    for (obj = filter_context->Items; obj; obj = obj->Next)
      {
      can_obj = (TObjCan *)obj;
      if ((can_obj->Filter.FilEnable) && (can_obj->Filter.FilIdMode == 3))
        {
        pass_filter = 1;
        if (((fd_msg->Id ^ can_obj->Filter.Code) & can_obj->Filter.Maske) == 0)
          {
          pass_filter = 3;
          break;
          }
        }
      }
    }
  }
if (pass_filter == 1)
  hit = 1;
if (!hit)
  {
  for (obj = filter_context->Items; obj; obj = obj->Next)
    {
    can_obj = (TObjCan *)obj;
    fill_id_mode = can_obj->Filter.FilIdMode;
    if (fill_id_mode == 3)  // 3 = Maske & Code Pass Mode
      continue;
    can_obj = (TObjCan *)obj;
    if (can_obj->Channels & channels)
      {
      if ((can_obj->Filter.FilEnable) &&
          (can_obj->Filter.FilEFF == fd_msg->MsgEFF) &&
          (can_obj->Filter.FilRTR == fd_msg->MsgRTR))
        {
        switch (fill_id_mode)
          {
          case 0 : { // 0 = Maske & Code
                   if (((fd_msg->Id ^ can_obj->Filter.Code) & can_obj->Filter.Maske) == 0)
                     hit = 1;
                   break;
                   }
          case 1 : { // 1 = Start & Stop
                   if ((fd_msg->Id >= can_obj->Filter.Code) && (fd_msg->Id <= can_obj->Filter.Maske))
                     hit = 1;
                   break;
                   }
          case 2 : { // 2 = Single Id
                   if (fd_msg->Id == can_obj->Filter.Code)
                     hit = 1;
                   break;
                   }
          }
        }
      }
    if (hit)
      {
      fd_msg->MsgFilHit = 1;
      if (obj->Class == OBJ_CAN_RX_PUFFER)
        (void)mhs_can_puffer_put_fd_unlocked((TObjCanPuffer *)obj, fd_msg);
      else if (obj->Class == OBJ_CAN_FIFO)
        {
        (void)mhs_can_fifo_put_fd_unlocked((TObjCanFifo *)obj, fd_msg, 1);
        if (dev->OvMode & 0x0100)
          mhs_fifo_auto_ov((TObjCanFifo *)obj);
        }
      (void)mhs_object_set_event_unlocked((TMhsObj *)obj);
      if (can_obj->Filter.FilMode)
        hit = 0;
      else
        break;
      }
    }
  }
mhs_leave_critical(filter_context->Event);
if (hit)
  return(0);
mhs_enter_critical(context->Event);
for (obj = context->Items; obj; obj = obj->Next)
  {
  if (((TObjCan *)obj)->Channels & channels)
    {
    (void)mhs_can_fifo_put_fd_unlocked((TObjCanFifo *)obj, fd_msg, 1);
    if (dev->OvMode & 0x0100)
      mhs_fifo_auto_ov((TObjCanFifo *)obj);
    (void)mhs_object_set_event_unlocked((TMhsObj *)obj);
    }
  }
mhs_leave_critical(context->Event);
return(0);
}


int32_t tx_ack_msg_to_rx(TMhsObjContext *context, uint32_t channels, struct TCanFdMsg *fd_msg)
{
int res;
TMhsObj *obj;

res = 0;
mhs_enter_critical(context->Event);
for (obj = context->Items; obj; obj = obj->Next)
  {
  if (((TObjCan *)obj)->Channels & channels)
    {
    res = mhs_can_fifo_put_fd_unlocked((TObjCanFifo *)obj, fd_msg, 1);
    /*if (dev->OvMode & 0x0100) <*>
      mhs_fifo_auto_ov((TObjCanFifo *)obj);*/
    (void)mhs_object_set_event_unlocked((TMhsObj *)obj);
    }
  }
mhs_leave_critical(context->Event);
return(res);
}


int32_t can_put_msgs_api(uint32_t index, struct TCanMsg *msg, int32_t count)
{
int32_t res;
TMhsObj *obj;
TMhsObjContext *context;
TCanDevice *dev;
#ifdef HAVE_TX_ACK_BYPASS
uint32_t channels;
int32_t cnt;
struct TCanMsg *m;
struct TCanFdMsg fd_msg;
#endif

context = can_get_context(index);
#ifdef HAVE_TX_ACK_BYPASS
channels = index_to_channels(index);
#endif
dev = get_device(index);
obj = mhs_object_get_by_index(index, context);
if (!obj)
  res = ERR_INDEX_RANGE;
else
  {
  if ((obj->Class == OBJ_CAN_TX_PUFFER) || (obj->Class == OBJ_CAN_RX_PUFFER))
    res = mhs_can_puffer_put((TObjCanPuffer *)obj, msg);
  else if (obj->Class == OBJ_CAN_FIFO)
    {
    res = mhs_can_fifo_put((TObjCanFifo *)obj, msg, count);
#ifdef HAVE_TX_ACK_BYPASS
    if (dev)  // <*> neu
      {
      if ((TxAckBypass) && (dev->GlobalTxAckEnable) && (res > 0))
        {
        m = msg;
        for (cnt = 0; cnt < res; cnt++)
          {
          m->MsgTxD = 1;
          CanToCanFd(&fd_msg, m);
          if (tx_ack_msg_to_rx(context, channels, &fd_msg) < 1)
            break;
          m++;
          }
        }
      }
#endif
    }
  else
    res = ERR_PUBLIC_ERROR;
  }
#ifdef USE_TX_MESSAGES_EVENT
if (res > 0)
  {
  if (dev)
    {
    if (dev->MainThread)
#ifdef MAIN_THREAD_USE_FILE_EVENT
      (void)set_file_event(dev->MainThreadEvent, TX_MESSAGES_EVENT);
#else
      mhs_event_set((TMhsEvent *)dev->MainThread, TX_MESSAGES_EVENT);
#endif
    }
  }
#endif
return(res);
}


int32_t can_put_fd_msgs_api(uint32_t index, struct TCanFdMsg *fd_msg, int32_t count)
{
int32_t res;
TMhsObj *obj;
TMhsObjContext *context;
TCanDevice *dev;
#ifdef HAVE_TX_ACK_BYPASS
uint32_t channels;
int32_t cnt;
struct TCanFdMsg *m;
#endif

context = can_get_context(index);
#ifdef HAVE_TX_ACK_BYPASS
channels = index_to_channels(index);
#endif
dev = get_device(index);
obj = mhs_object_get_by_index(index, context);
if (!obj)
  res = ERR_INDEX_RANGE;
else
  {
  if ((obj->Class == OBJ_CAN_TX_PUFFER) || (obj->Class == OBJ_CAN_RX_PUFFER))
    res = mhs_can_puffer_put_fd((TObjCanPuffer *)obj, fd_msg);
  else if (obj->Class == OBJ_CAN_FIFO)
    {
    res = mhs_can_fifo_put_fd((TObjCanFifo *)obj, fd_msg, count);
#ifdef HAVE_TX_ACK_BYPASS
    if (dev)  // <*> neu
      {
      if ((TxAckBypass) && (dev->GlobalTxAckEnable) && (res > 0))
        {
        m = fd_msg;
        for (cnt = 0; cnt < res; cnt++)
          {
          m->MsgTxD = 1;
          if (tx_ack_msg_to_rx(context, channels, m) < 1)
            break;
          m++;
          }
        }
      }
#endif
    }
  else
    res = ERR_PUBLIC_ERROR;
  }
#ifdef USE_TX_MESSAGES_EVENT
if (res > 0)
  {
  if ((dev = get_device(index)))
    {
    if (dev->MainThread)
#ifdef MAIN_THREAD_USE_FILE_EVENT
      (void)set_file_event(dev->MainThreadEvent, TX_MESSAGES_EVENT);
#else
      mhs_event_set((TMhsEvent *)dev->MainThread, TX_MESSAGES_EVENT);
#endif
    }
  }
#endif
return(res);
}


int32_t can_get_msgs_api(uint32_t index, struct TCanMsg *msg, int32_t count)
{
int32_t res;
TMhsObj *obj;
TMhsObjContext *context;

context = can_get_context(index);
obj = mhs_object_get_by_index(index, context);

if (!obj)
  res = ERR_INDEX_RANGE;
else
  {
  if ((obj->Class == OBJ_CAN_TX_PUFFER) || (obj->Class == OBJ_CAN_RX_PUFFER))
    {
    if ((res = mhs_can_puffer_get((TObjCanPuffer *)obj, msg)) < 0)
      res = ERR_READ_PUFFER;
    }
  else if (obj->Class == OBJ_CAN_FIFO)
    res = mhs_can_fifo_get((TObjCanFifo *)obj, msg, count);
  else
    res = -1;
  }
return(res);
}


int32_t can_get_fd_msgs_api(uint32_t index, struct TCanFdMsg *fd_msg, int32_t count)
{
int32_t res;
TMhsObj *obj;
TMhsObjContext *context;

context = can_get_context(index);
obj = mhs_object_get_by_index(index, context);

if (!obj)
  res = ERR_INDEX_RANGE;
else
  {
  if ((obj->Class == OBJ_CAN_TX_PUFFER) || (obj->Class == OBJ_CAN_RX_PUFFER))
    {
    if ((res = mhs_can_puffer_get_fd((TObjCanPuffer *)obj, fd_msg)) < 0)
      res = ERR_READ_PUFFER;
    }
  else if (obj->Class == OBJ_CAN_FIFO)
    res = mhs_can_fifo_get_fd((TObjCanFifo *)obj, fd_msg, count);
  else
    res = -1;
  }
return(res);
}


int32_t can_clear_msgs(uint32_t index)
{
int32_t res;
TMhsObj *obj;
TMhsObjContext *context;

context = can_get_context(index);
obj = mhs_object_get_by_index(index, context);

if (!obj)
  res = ERR_INDEX_RANGE;
else
  {
  if ((obj->Class == OBJ_CAN_TX_PUFFER) || (obj->Class == OBJ_CAN_RX_PUFFER))
    res = mhs_can_puffer_clear((TObjCanPuffer *)obj);
  else if (obj->Class == OBJ_CAN_FIFO)
    res = mhs_can_fifo_clear((TObjCanFifo *)obj);
  else
    res = -1;
  }
return(res);
}


int32_t can_get_msg_count(uint32_t index)
{
int32_t res;
TMhsObj *obj;
TMhsObjContext *context;

context = can_get_context(index);
obj = mhs_object_get_by_index(index, context);

if (!obj)
  res = ERR_INDEX_RANGE;
else
  {
  if (obj->Class == OBJ_CAN_RX_PUFFER)
    {
    if ((res = mhs_can_puffer_count((TObjCanPuffer *)obj)) < 0)
      res = ERR_READ_PUFFER;
    }
  else if (obj->Class == OBJ_CAN_FIFO)
    res = mhs_can_fifo_count((TObjCanFifo *)obj);
  else
    res = -1;
  }
return(res);
}


int32_t can_set_filter(uint32_t index, struct TMsgFilter *msg_filter)
{
int32_t res;
TMhsObj *obj;
TObjCan *can_obj;
TMhsObjContext *context;
TMhsThread *thread;

res = 0;
context = can_get_context(index);
obj = mhs_object_get_by_index(index, context);
if (!obj)
  {
  if (!(obj = (TMhsObj *)mhs_can_puffer_create(OBJ_CAN_RX_PUFFER, index, NULL, context)))
    res = -1;
  else
    {
    if ((thread = GetApiCpTheread()))
      (void)mhs_object_events_set(obj, (TMhsEvent *)thread, EVENT_RX_FILTER_MESSAGES);
    }
  }
if (obj)
  {
  if (obj->Class == OBJ_CAN_RX_PUFFER)
    {
    can_obj = (TObjCan *)obj;
    can_obj->Channels = 0xFFFFFFFF;  // <*>
    memcpy(&can_obj->Filter, msg_filter, sizeof(struct TMsgFilter));
    if (can_obj->Filter.FilIdMode == 2)// 2 = Single Id
      can_obj->Filter.Maske = 0xFFFFFFFF;
    if ((!(index & INDEX_SOFT_FLAG)) && (index & INDEX_FIFO_PUFFER_MASK))
      mhs_object_set_cmd_event(obj);
    }
  }
else
  res = -1;
return(res);
}


void can_sw_filter_remove_all(void)
{
TMhsObjContext *context;

if (!(context = can_filter_get_context()))
  return;
mhs_enter_critical(context->Event);
mhs_all_objects_destroy(context);
mhs_leave_critical(context->Event);
}
