/***************************************************************************
                          mhs_obj.c  -  description
                             -------------------
    begin             : 20.07.2009
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
//#include "global.h"
#include <glib.h>
#include "util.h"
#include "mhs_event.h"
#include "mhs_obj.h"


static struct TMhsObjContext *DefaultObjContext = NULL;
static struct TMhsObjContext *ObjContextList = NULL;


static void mhs_obj_handler_destroy(struct TMhsObj *obj);
static gint mhs_event_proc(gpointer data);


static const struct TObjFuncs DummyFuncs =
  {
  NULL,
  NULL
  };


void mhs_object_init(void)
{
DefaultObjContext = NULL;
ObjContextList = NULL;
}


static void mhs_obj_destroy(struct TMhsObjContext *context)
{
struct TMhsObj *obj, *list;

if (!context)
  return;
if (context->MhsEventId)
  {
  g_source_remove(context->MhsEventId);
  context->MhsEventId = 0;
  }
list = context->Items;
while (list)
  {
  mhs_obj_handler_destroy(list);
  obj = list->Next;
  g_free(list);
  list = obj;
  }
context->Items = NULL;
context->MarkedFirst = NULL;
context->MarkedLast = NULL;
context->DeleteItems = 0;

MHS_LOCK_DESTROY(context->Lock);
}


void mhs_object_destroy(void)
{
struct TMhsObjContext *tmp, *list;

if (!ObjContextList)
  return;
list = ObjContextList;
while (list)
  {
  mhs_obj_destroy(list);
  tmp = list->Next;
  g_free(list);
  list = tmp;
  }
DefaultObjContext = NULL;
ObjContextList = NULL;
}


struct TMhsObjContext *mhs_obj_context_new(void)
{
struct TMhsObjContext *context, *list;

context = (struct TMhsObjContext *) g_malloc0(sizeof(struct TMhsObjContext));
if (!context)
  return(NULL);
context->Lock = MHS_LOCK_INIT();
list = ObjContextList;
if (!list)
  ObjContextList = context;
else
  {
  while (list->Next)
    list = list->Next;
  list->Next = context;
  context->Next = NULL;
  }
context->MhsEventId = mhs_event_add((GSourceFunc)mhs_event_proc, (gpointer)context);
return(context);
}


struct TMhsObjContext *mhs_obj_context_default(void)
{
if (!DefaultObjContext)
  DefaultObjContext = mhs_obj_context_new();
return(DefaultObjContext);
}


struct TMhsObj *mhs_object_new(const struct TObjFuncs *funcs, int struct_size, struct TMhsObjContext *context)
{
struct TMhsObj *obj, *list;

if (!context)
  context = mhs_obj_context_default();
obj = (struct TMhsObj *) g_malloc0(struct_size);
if (!obj)
  return(0);
MHS_LOCK_ENTER(context->Lock);
obj->Context = context;
obj->MarkedNext = NULL;
obj->MarkedFlag = 0;
obj->EventCount = 0;
obj->CbEnable = 1;
list = context->Items;
if (!funcs)
  obj->Funcs = &DummyFuncs;
else
  obj->Funcs = funcs;
if (!list)
  context->Items = obj;
else
  {
  while (list->Next)
    list = list->Next;
  list->Next = obj;
  obj->Next = NULL;
  }
MHS_LOCK_LEAVE(context->Lock);
return(obj);
}


static struct TMhsObjHandler *mhs_obj_handler_new_unlocked(struct TMhsObj *obj)
{
struct TMhsObjContext *context;
struct TMhsObjHandler *handler, *list;

context = obj->Context;
handler = (struct TMhsObjHandler *) g_malloc0(sizeof(struct TMhsObjHandler));
if (!handler)
  return(NULL);

list = obj->HandlerList;
if (!list)
  obj->HandlerList = handler;
else
  {
  while (list->Next)
    list = list->Next;
  list->Next = handler;
  handler->Next = NULL;
  }
return(handler);
}


static struct TMhsObjHandler *mhs_obj_handler_new(struct TMhsObj *obj)
{
struct TMhsObjContext *context;
struct TMhsObjHandler *handler;

context = obj->Context;
MHS_LOCK_ENTER(context->Lock);
handler = mhs_obj_handler_new_unlocked(obj);
MHS_LOCK_LEAVE(context->Lock);
return(handler);
}


static void mhs_obj_handler_destroy(struct TMhsObj *obj)
{
struct TMhsObjHandler *tmp, *list;

if (!obj)
  return;
list = obj->HandlerList;
while (list)
  {
  tmp = list->Next;
  g_free(list);
  list = tmp;
  }
obj->HandlerList = NULL;
}



int mhs_object_remove(struct TMhsObj *obj)
{
struct TMhsObjContext *context;
struct TMhsObj *list, *prev;

if (!obj)
  return(0);
context = obj->Context;
MHS_LOCK_ENTER(context->Lock);
context->DeleteItems = 1;
prev = NULL;
// Liste nach "obj" durchsuchen
for (list = context->Items; list; list = list->Next)
  {
  if (list == obj)
    {
    if (prev)
      prev->Next = list->Next;
    else
      context->Items = list->Next;
    mhs_obj_handler_destroy(list);
    g_free(list);
    }
  prev = list;
  }
MHS_LOCK_LEAVE(context->Lock);
if (!list)
  return(-1);
else
  return(0);
}


int mhs_object_valid_unlocked(struct TMhsObj *obj)
{
struct TMhsObjContext *context;
struct TMhsObj *list;

if (!obj)
  return(-1);
context = obj->Context;
for (list = context->Items; list; list = list->Next)
  {
  if (list == obj)
    break;
  }
if (!list)
  return(-1);
else
  return(0);
}


int mhs_object_valid(struct TMhsObj *obj)
{
struct TMhsObjContext *context;
int res;

if (!obj)
  return(-1);
context = obj->Context;
MHS_LOCK_ENTER(context->Lock);
res = mhs_object_valid_unlocked(obj);
MHS_LOCK_LEAVE(context->Lock);
return(res);
}


int mhs_object_set_event_unlocked(struct TMhsObj *obj)
{
struct TMhsObjContext *context;

if (!obj)
  return(-1);
context = obj->Context;
obj->EventCount++;
// **** Marked Liste verwalten
if (!obj->MarkedFlag)
  {
  obj->MarkedFlag = 1;
  if (!context->MarkedFirst)
    context->MarkedFirst = obj;
  if (context->MarkedLast)
    context->MarkedLast->MarkedNext = obj;
  obj->MarkedNext = NULL;
  context->MarkedLast = obj;
  }
return(0);
}


int mhs_object_set_event(struct TMhsObj *obj)
{
struct TMhsObjContext *context;
int res;

if (!obj)
  return(-1);
context = obj->Context;
MHS_LOCK_ENTER(context->Lock);
res = mhs_object_set_event_unlocked(obj);
MHS_LOCK_LEAVE(context->Lock);
mhs_set_event(context->MhsEventId);
return(res);
}


void mhs_object_reset_event(struct TMhsObj *obj)
{
struct TMhsObjContext *context;

if (!obj)
  return;
context = obj->Context;
MHS_LOCK_ENTER(context->Lock);
obj->EventCount = 0;
MHS_LOCK_LEAVE(context->Lock);
}


int mhs_object_get_event_count(struct TMhsObj *obj)
{
struct TMhsObjContext *context;
int res;

if (!obj)
  return(-1);
context = obj->Context;
MHS_LOCK_ENTER(context->Lock);
res = obj->EventCount;
MHS_LOCK_LEAVE(context->Lock);
return(res);
}


int mhs_object_event_connect(struct TMhsObj *obj, TMhsObjectCB proc, gpointer user_data)
{
int res;
struct TMhsObjContext *context;
struct TMhsObjHandler *handler;

if (!obj)
  return(-1);
context = obj->Context;
MHS_LOCK_ENTER(context->Lock);
if ((handler = mhs_obj_handler_new_unlocked(obj)))
  {
  handler->Proc = proc;
  handler->UserData = user_data;
  res = 0;
  }
else
  res = -1;
MHS_LOCK_LEAVE(context->Lock);
return(res);
}


static void mhs_object_update_marked(struct TMhsObjContext *context)
{
struct TMhsObj *obj;

// Alle Elemente durchlaufen
context->MarkedFirst = NULL;
context->MarkedLast = NULL;
for (obj = context->Items; obj; obj = obj->Next)
  {
  if (obj->EventCount > 0)
    {
    obj->MarkedFlag = 1;
    if (!context->MarkedFirst)
      context->MarkedFirst = obj;
    if (context->MarkedLast)
      context->MarkedLast->MarkedNext = obj;
    obj->MarkedNext = NULL;
    context->MarkedLast = obj;
    }
  else
    obj->MarkedFlag = 0;
  }
}


static struct TMhsObj *mhs_object_get_marked(struct TMhsObjContext *context)
{
struct TMhsObj *obj, *hit;

if (context->DeleteItems)
  {
  mhs_object_update_marked(context);
  context->DeleteItems = 0;
  }

hit = NULL;
obj = context->MarkedFirst;
if (obj)
  {
  if (obj->EventCount > 0)
    {
    hit = obj;
    obj->EventCount = 0;
    }
  obj->MarkedFlag = 0;
  if (!(context->MarkedFirst = obj->MarkedNext))
    context->MarkedLast = NULL;
  }
return(hit);
}



gint mhs_event_proc(gpointer data)
{
int i;
struct TMhsObjContext *context;
struct TMhsObj *obj;
struct TMhsObjHandler *handler;

if (!(context = (struct TMhsObjContext *)data))
  return(FALSE);
for (i = 100; i; i--)
  {
  MHS_LOCK_ENTER(context->Lock);
  obj = mhs_object_get_marked(context);
  if ((obj) && (obj->Funcs->Event))
    (obj->Funcs->Event)(obj);
  MHS_LOCK_LEAVE(context->Lock);
  if (!obj)
    break;
  if (obj->CbEnable)
    {
    for (handler = obj->HandlerList; handler; handler = handler->Next)
      {
      if (handler->Proc)
        {
        if (obj->Funcs->UserCB)
          (obj->Funcs->UserCB)(obj, handler);
        else
          (handler->Proc)(obj, handler->UserData);
        }
      }
    }
  }
return(TRUE);
}
