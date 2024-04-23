/***************************************************************************
                       can_view_obj.c  -  description
                             -------------------
    begin             : 26.07.2009
    copyright         : (C) 2009 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include <glib.h>
#include <string.h>
#include "can_monitor.h"
#include "util.h"
#include "filter.h"
#include "mhs_obj.h"
#include "mhs_event.h"
#include "can_monitor_obj.h"

struct TObjTinyCanStatus *ObjTinyCanStatus = NULL;
struct TObjTinyCanPnP *ObjTinyCanPnP = NULL;
struct TMhsObj *ObjTinyCanBufferFull = NULL;
struct TMhsObj *ObjRxDFilter = NULL;

static void ObjTinyCanStatusEventCB(struct TMhsObj *obj);
static void ObjTinyCanStatusUserCB(struct TMhsObj *obj, struct TMhsObjHandler *handler);
static void ObjTinyCanPnPEventCB(struct TMhsObj *obj);
static void ObjTinyCanPnPUserCB(struct TMhsObj *obj, struct TMhsObjHandler *handler);


static const struct TObjFuncs ObjTinyCanStatusFuncs =
  {
  ObjTinyCanStatusEventCB,
  ObjTinyCanStatusUserCB
  };

static const struct TObjFuncs ObjTinyCanPnPFuncs =
  {
  ObjTinyCanPnPEventCB,
  ObjTinyCanPnPUserCB
  };




int CanViewObjInit(void)
{
struct TMhsObj *obj;

obj = mhs_object_new(&ObjTinyCanStatusFuncs, sizeof(struct TObjTinyCanStatus), NULL);
if (!obj)
  return(-1);
ObjTinyCanStatus = (struct TObjTinyCanStatus *)obj;
obj = mhs_object_new(&ObjTinyCanPnPFuncs, sizeof(struct TObjTinyCanPnP), NULL);
if (!obj)
  return(-1);
ObjTinyCanPnP = (struct TObjTinyCanPnP *)obj;
obj = mhs_object_new(NULL, sizeof(struct TMhsObj), NULL);
if (!obj)
  return(-1);
ObjTinyCanBufferFull = obj;
obj = mhs_object_new(NULL, sizeof(struct TMhsObj), NULL);
if (!obj)
  return(-1);
ObjRxDFilter = obj;
return(0);
}


static void ObjTinyCanStatusEventCB(struct TMhsObj *obj)
{
struct TObjTinyCanStatus *obj_tiny_can_status;

obj_tiny_can_status = (struct TObjTinyCanStatus *)obj;
memcpy(&obj_tiny_can_status->DeviceStatusOut, &obj_tiny_can_status->DeviceStatus, sizeof(struct TDeviceStatus));
}


static void ObjTinyCanStatusUserCB(struct TMhsObj *obj, struct TMhsObjHandler *handler)
{
struct TObjTinyCanStatus *obj_tiny_can_status;
TTinyCanStatusCB user_handler;

obj_tiny_can_status = (struct TObjTinyCanStatus *)obj;
user_handler = (TTinyCanStatusCB)handler->Proc;
(user_handler)(obj, &obj_tiny_can_status->DeviceStatusOut, handler->UserData);
}



static void ObjTinyCanPnPEventCB(struct TMhsObj *obj)
{
struct TObjTinyCanPnP *obj_tiny_can_pnp;

obj_tiny_can_pnp = (struct TObjTinyCanPnP *)obj;
obj_tiny_can_pnp->PnPStatusOut = obj_tiny_can_pnp->PnPStatus;
}


static void ObjTinyCanPnPUserCB(struct TMhsObj *obj, struct TMhsObjHandler *handler)
{
struct TObjTinyCanPnP *obj_tiny_can_pnp;
TTinyCanPnPCB user_handler;

obj_tiny_can_pnp = (struct TObjTinyCanPnP *)obj;
user_handler = (TTinyCanPnPCB)handler->Proc;
(user_handler)(obj, obj_tiny_can_pnp->PnPStatusOut, handler->UserData);
}


/**************************************************************/
/* CAN Treiber Callback Funktionen                            */
/**************************************************************/
void CALLBACK_TYPE CanRxEvent(uint32_t index, struct TCanMsg *msg, int32_t count)
{

if ((DataRecord == DATA_RECORD_START) &&
   ((index & (INDEX_CAN_KANAL_MASK | INDEX_CAN_DEVICE_MASK)) == 0L))
  {
  index &= (INDEX_FIFO_PUFFER_MASK | INDEX_SOFT_FLAG);
  if (!index)
    {
    if (CBufferDataAdd(MainWin.CanBuffer, msg, count) != count)
      (void)mhs_object_set_event(ObjTinyCanBufferFull);
    }
  else
    {
    if (FilWriteMessageByIdx(index, msg))
      (void)mhs_object_set_event(ObjRxDFilter);
    }
  }
}


void CALLBACK_TYPE CanPnPEvent(uint32_t index, int32_t status)
{
struct TMhsObj *obj;
struct TMhsObjContext *context;

if (!ObjTinyCanPnP)
  return;
obj = (struct TMhsObj *)ObjTinyCanPnP;
context = obj->Context;
MHS_LOCK_ENTER(context->Lock);
ObjTinyCanPnP->PnPStatus = status;
(void)mhs_object_set_event_unlocked(obj);
MHS_LOCK_LEAVE(context->Lock);
mhs_set_event(context->MhsEventId);
}


void CALLBACK_TYPE CanStatusEvent(uint32_t index, struct TDeviceStatus *DeviceStatus)
{
struct TMhsObj *obj;
struct TMhsObjContext *context;

if (!ObjTinyCanStatus)
  return;
obj = (struct TMhsObj *)ObjTinyCanStatus;
context = obj->Context;
MHS_LOCK_ENTER(context->Lock);
memcpy(&ObjTinyCanStatus->DeviceStatus, DeviceStatus, sizeof(struct TDeviceStatus));
(void)mhs_object_set_event_unlocked(obj);
MHS_LOCK_LEAVE(context->Lock);
mhs_set_event(context->MhsEventId);
}

