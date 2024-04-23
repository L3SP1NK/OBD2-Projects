/***************************************************************************
                          mhs_obj.c  -  description
                             -------------------
    begin             : 20.07.2009
    last modify       : 10.03.2021
    copyright         : (C) 2009 - 2021 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "mhs_class.h"
#include "util.h"
#include "mhs_event.h"
#include "mhs_obj.h"

//#define MHS_OBJ_USE_INDEX
//#define MHS_OBJ_USE_CONTEXT_LIST
#define MHS_OBJ_HAVE_ALIAS


#ifdef MHS_OBJ_USE_CONTEXT_LIST
static TMhsObjContext *DefaultObjContext = NULL;
static TMhsObjContext *ObjContextList = NULL;
#endif


static void mhs_object_handler_destroy(TMhsObj *obj);


static const struct TObjFuncs DummyFuncs =
  {
  NULL,
  NULL,
  NULL
  };


/***************************************************************************/
/*                  Object Library Initialisieren                          */
/***************************************************************************/
void mhs_object_init(void)
{
#ifdef MHS_OBJ_USE_CONTEXT_LIST
DefaultObjContext = NULL;
ObjContextList = NULL;
#endif
}


/***************************************************************************/
/*   Object Library freigeben, alle Objecte und Contexte werden gelöscht   */
/***************************************************************************/
#ifdef MHS_OBJ_USE_CONTEXT_LIST

void mhs_object_destroy(void)
{
TMhsObjContext *tmp, *list;

if (!ObjContextList)
  return;
list = ObjContextList;
while (list)
  {
  mhs_all_objects_destroy(list);
  tmp = list->Next;
  safe_free(list->Name);
  mhs_free(list);
  list = tmp;
  }
DefaultObjContext = NULL;
ObjContextList = NULL;
}

#else


void mhs_obj_context_destroy(TMhsObjContext **context)
{
TMhsObjContext *con;

if (!context)
  return;
con = *context;
if (!con)
  return;
mhs_all_objects_destroy(con);
safe_free(con->Name);
mhs_free(con);
*context = NULL;
}

#endif


/***************************************************************************/
/*              Alle Objecte des Contextes löschen                         */
/***************************************************************************/
void mhs_all_objects_destroy(TMhsObjContext *context)
{
TMhsObj *obj, *list;

if (!context)
  return;

list = context->Items;
while (list)
  {
  mhs_object_handler_destroy(list);
  if (list->Funcs->DestroyCB)
    (list->Funcs->DestroyCB)(list);
  obj = list->Next;
  safe_free(list->Name);
#ifdef MHS_OBJ_HAVE_ALIAS    
  safe_free(list->Alias);
#endif  
  mhs_free(list);
  list = obj;
  }
context->Items = NULL;
context->MarkedFirst = NULL;
context->MarkedLast = NULL;
context->DeleteItems = 0;
// Commando Events
context->CmdMarkedFirst = NULL;
context->CmdMarkedLast = NULL;
context->CmdDeleteItems = 0;
}


/***************************************************************************/
/*=========================================================================*/
/*                  C O N T E X T - F U N K T I O N E N                    */
/*=========================================================================*/
/***************************************************************************/

#ifdef MHS_OBJ_USE_CONTEXT_LIST
TMhsObjContext *mhs_obj_context_auto_new(const char *name, TMhsEvent *event)
{
TMhsObjContext *context;

if (!(context = mhs_obj_context_get_by_name(name)))
  {
  context = mhs_obj_context_new(name, event);
  }
return(context);
}


/***************************************************************************/
/*                     Einen neuen Context erzeugen                        */
/***************************************************************************/
TMhsObjContext *mhs_obj_context_new(const char *name, TMhsEvent *event)
{
TMhsObjContext *context, *list;

if (!name)
  return(NULL);

context = (TMhsObjContext *)mhs_calloc(1, sizeof(TMhsObjContext));
if (!context)
  return(NULL);
context->Event = event;
context->Name = mhs_strdup(name);
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
return(context);
}


/***************************************************************************/
/*                      Context mit "Name" suchen                          */
/***************************************************************************/
TMhsObjContext *mhs_obj_context_get_by_name(const char *name)
{
TMhsObjContext *list;

if (!name)
  return(NULL);
for (list = ObjContextList; list; list = list->Next)
  {
  if (!safe_strcasecmp(list->Name, name))
    return(list);          
  }
return(NULL);
}


/***************************************************************************/
/*             Default Context zurückgeben, bzw. erzeugen                  */
/***************************************************************************/
TMhsObjContext *mhs_obj_context_default(void)
{
if (!DefaultObjContext)
  DefaultObjContext = mhs_obj_context_new("application", NULL);
return(DefaultObjContext);
}

#else

/***************************************************************************/
/*                     Einen neuen Context erzeugen                        */
/***************************************************************************/
TMhsObjContext *mhs_obj_context_new(const char *name, TMhsEvent *event)
{
TMhsObjContext *context;

if (!name)
  return(NULL);

context = (TMhsObjContext *)mhs_calloc(1, sizeof(TMhsObjContext));
if (!context)
  return(NULL);
context->Event = event;
context->Name = mhs_strdup(name);
return(context);
}


TMhsObjContext *mhs_obj_context_default(void)
{
return(NULL);
}

#endif


/***************************************************************************/
/*=========================================================================*/
/*                   O B J E C T - F U N K T I O N E N                     */
/*=========================================================================*/
/***************************************************************************/

/***************************************************************************/
/*                        Ein neues Object erzeugen                        */
/***************************************************************************/
#ifdef MHS_OBJ_USE_INDEX
static TMhsObj *mhs_object_create(uint32_t obj_class, uint32_t index, const char *name, const struct TObjFuncs *funcs, int32_t struct_size, TMhsObjContext *context)
#else
static TMhsObj *mhs_object_create(uint32_t obj_class, const char *name, const struct TObjFuncs *funcs, int32_t struct_size, TMhsObjContext *context)
#endif
{
TMhsObj *obj, *list;

#ifndef MHS_OBJ_USE_INDEX
if (!name)
  return(NULL);
#endif
if (!context)
  {
  if (!(context = mhs_obj_context_default()))
    return(NULL);
  }
#ifdef MHS_OBJ_USE_INDEX
if (name)
  {
  if (mhs_object_get_by_name_unlocked(name, context))
    return(NULL);
  }
else
  {
  if (mhs_object_get_by_index_unlocked(index, context))
    return(NULL);
  }
#else
if (mhs_object_get_by_name_unlocked(name, context))
  return(NULL);
#endif
obj = (TMhsObj *)mhs_calloc(1, struct_size);
if (!obj)
  return(NULL);

obj->Context = context;
obj->Class = obj_class;
obj->Name = mhs_strdup(name);
#ifdef MHS_OBJ_USE_INDEX
obj->Index = index;
#endif
obj->MarkedNext = NULL;
obj->MarkedFlag = 0;
obj->EventCount = 0;

obj->CmdMarkedNext = NULL;
obj->CmdMarkedFlag = 0;
obj->CmdEventCount = 0;
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
return(obj);
}


#ifdef MHS_OBJ_USE_INDEX
TMhsObj *mhs_object_new_unlocked(uint32_t obj_class, uint32_t index, const char *name, const struct TObjFuncs *funcs, int32_t struct_size, TMhsObjContext *context)
#else
TMhsObj *mhs_object_new_unlocked(uint32_t obj_class, const char *name, const struct TObjFuncs *funcs, int32_t struct_size, TMhsObjContext *context)
#endif
{
TMhsObj *obj;

if (!context)
  {
  if (!(context = mhs_obj_context_default()))
    return(NULL);
  }
#ifdef MHS_OBJ_USE_INDEX
obj = mhs_object_create(obj_class, index, name, funcs, struct_size, context);
#else
obj = mhs_object_create(obj_class, name, funcs, struct_size, context);
#endif
return(obj);
}

#ifdef MHS_OBJ_USE_INDEX
TMhsObj *mhs_object_new(uint32_t obj_class, uint32_t index, const char *name, const struct TObjFuncs *funcs, int32_t struct_size, TMhsObjContext *context)
#else
TMhsObj *mhs_object_new(uint32_t obj_class, const char *name, const struct TObjFuncs *funcs, int32_t struct_size, TMhsObjContext *context)
#endif
{
TMhsObj *obj;

if (!context)
  {
  if (!(context = mhs_obj_context_default()))
    return(NULL);
  }
mhs_enter_critical(context->Event);
#ifdef MHS_OBJ_USE_INDEX
obj = mhs_object_create(obj_class, index, name, funcs, struct_size, context);
#else
obj = mhs_object_create(obj_class, name, funcs, struct_size, context);
#endif
mhs_leave_critical(context->Event);
return(obj);
}

#ifdef MHS_OBJ_HAVE_ALIAS    
void mhs_object_set_alias(TMhsObj *obj, const char *alias)
{
if (!obj)
  return;
safe_free(obj->Alias);
obj->Alias = mhs_strdup(alias);
}


char *mhs_object_get_alias(TMhsObj *obj)
{
if (!obj)
  return(NULL);
return(obj->Alias);  
}
#endif


/***************************************************************************/
/*                       Object mit "Index" suchen                         */
/***************************************************************************/
#ifdef MHS_OBJ_USE_INDEX
TMhsObj *mhs_object_get_by_index_unlocked(uint32_t index, TMhsObjContext *context)
{
TMhsObj *list;

if (!context)
  {
  if (!(context = mhs_obj_context_default()))
    return(NULL);
  }
for (list = context->Items; list; list = list->Next)
  {
  if (list->Index == index)
    return(list);
  }
return(NULL);
}


TMhsObj *mhs_object_get_by_index(uint32_t index, TMhsObjContext *context)
{
TMhsObj *obj;

if (!context)
  return(NULL);
mhs_enter_critical(context->Event);
obj = mhs_object_get_by_index_unlocked(index, context);
mhs_leave_critical(context->Event);
return(obj);
}

#endif


/***************************************************************************/
/*                       Object mit "Name" suchen                          */
/***************************************************************************/
TMhsObj *mhs_object_get_by_name_unlocked(const char *name, TMhsObjContext *context)
{
TMhsObj *list;

if (!name)
  return(NULL);
if (!context)
  {
  if (!(context = mhs_obj_context_default()))
    return(NULL);
  }
for (list = context->Items; list; list = list->Next)
  {
  if (!safe_strcasecmp(list->Name, name))  
    return(list);
#ifdef MHS_OBJ_HAVE_ALIAS    
  if (!safe_strcasecmp(list->Alias, name))
    return(list);  
#endif    
  }
return(NULL);
}


TMhsObj *mhs_object_get_by_name(const char *name, TMhsObjContext *context)
{
TMhsObj *obj;

if (!context)
  {
  if (!(context = mhs_obj_context_default()))
    return(NULL);
  }
mhs_enter_critical(context->Event);
obj = mhs_object_get_by_name_unlocked(name, context);
mhs_leave_critical(context->Event);
return(obj);
}


/***************************************************************************/
/*                      Ein Event Handler erzeugen                         */
/***************************************************************************/
static TMhsObjHandler *mhs_obj_handler_new_unlocked(TMhsObj *obj, int32_t type)
{
TMhsObjHandler *handler, *list;

handler = (TMhsObjHandler *)mhs_calloc(1, sizeof(TMhsObjHandler));
if (!handler)
  return(NULL);
if (type)
  {
  list = obj->SyncHandlerList;
  if (!list)
    obj->SyncHandlerList = handler;
  else
    {
    while (list->Next)
      list = list->Next;
    list->Next = handler;
    handler->Next = NULL;
    }
  }
else
  {
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
  }
return(handler);
}


/***************************************************************************/
/*                       Ein Event Handler löschen                         */
/***************************************************************************/
static void mhs_object_handler_destroy(TMhsObj *obj)
{
TMhsObjHandler *tmp, *list;

if (!obj)
  return;
list = obj->HandlerList;
while (list)
  {
  tmp = list->Next;
  mhs_free(list);
  list = tmp;
  }
obj->HandlerList = NULL;
list = obj->SyncHandlerList;
while (list)
  {
  tmp = list->Next;
  mhs_free(list);
  list = tmp;
  }
obj->SyncHandlerList = NULL;
list = obj->CmdHandlerList;
while (list)
  {
  tmp = list->Next;
  mhs_free(list);
  list = tmp;
  }
obj->CmdHandlerList = NULL;
list = obj->CmdSyncHandlerList;
while (list)
  {
  tmp = list->Next;
  mhs_free(list);
  list = tmp;
  }
obj->CmdSyncHandlerList = NULL;
}


/***************************************************************************/
/*                       Ein Object löschen                                */
/***************************************************************************/
static int32_t mhs_object_remove_unlocked(TMhsObj *obj)
{
TMhsObjContext *context;
TMhsObj *list, *prev;

if (!obj)
  return(0);
context = obj->Context;
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
    mhs_object_handler_destroy(list);
    if (list->Funcs->DestroyCB)
      (list->Funcs->DestroyCB)(list);
    safe_free(list->Name);
#ifdef MHS_OBJ_HAVE_ALIAS    
    safe_free(list->Alias);
#endif    
    mhs_free(list);
    break;
    }
  prev = list;
  }
if (!list)
  return(-1);
else
  return(0);
}


int32_t mhs_object_remove(TMhsObj *obj)
{
int32_t res;
TMhsObjContext *context;

if (!obj)
  return(0);
context = obj->Context;
mhs_enter_critical(context->Event);
res = mhs_object_remove_unlocked(obj);
mhs_leave_critical(context->Event);
return(res);
}


/***************************************************************************/
/*                     Ein Object mit "Index" löschen                      */
/***************************************************************************/
#ifdef MHS_OBJ_USE_INDEX
int32_t mhs_obj_remove_by_index(uint32_t index, TMhsObjContext *context)
{
int32_t res;
TMhsObj *obj;

if (!context)
  return(0);
mhs_enter_critical(context->Event);
if (!(obj = mhs_object_get_by_index_unlocked(index, context)))
  res = -1;
else
  res = mhs_object_remove_unlocked(obj);
mhs_leave_critical(context->Event);
return(res);
}
#endif


/***************************************************************************/
/*                     Ein Object mit "Namen" löschen                      */
/***************************************************************************/
int32_t mhs_obj_remove_by_name(const char *name, TMhsObjContext *context)
{
int32_t res;
TMhsObj *obj;

if (!context)
  {
  if (!(context = mhs_obj_context_default()))
    return(0);
  }
mhs_enter_critical(context->Event);
if (!(obj = mhs_object_get_by_name_unlocked(name, context)))
  res = -1;
else
  res = mhs_object_remove_unlocked(obj);
mhs_leave_critical(context->Event);
return(res);
}


/***************************************************************************/
/*                    Prüfen ob Object gültig (Unlocked)                   */
/***************************************************************************/
int32_t mhs_object_valid_unlocked(TMhsObj *obj)
{
TMhsObjContext *context;
TMhsObj *list;

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


/***************************************************************************/
/*                        Prüfen ob Object gültig                          */
/***************************************************************************/
int32_t mhs_object_valid(TMhsObj *obj)
{
TMhsObjContext *context;
int32_t res;

if (!obj)
  return(-1);
context = obj->Context;
mhs_enter_critical(context->Event);
res = mhs_object_valid_unlocked(obj);
mhs_leave_critical(context->Event);
return(res);
}


/***************************************************************************/
/*                      Object "Ereignis" markieren                        */
/***************************************************************************/
static int32_t mhs_object_set_event_marker(TMhsObj *obj)
{
TMhsObjContext *context;

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

#ifdef MHS_OBJ_USE_EVENTS
int32_t mhs_object_events_set(TMhsObj *obj, TMhsEvent *event, uint32_t events)
{
if (!obj)
  return(-1);
obj->Event = event;
obj->EventFlags = events;
return(0);
}
#endif


/***************************************************************************/
/*              Object "Ereignis" auslösen (Unlocked)                      */
/***************************************************************************/
int32_t mhs_object_set_event_unlocked(TMhsObj *obj)
{
int32_t res;

res = mhs_object_set_event_marker(obj);
#ifdef MHS_OBJ_USE_EVENTS
if (obj->Event)
  {
  if (obj->Context->Event == obj->Event)
    mhs_event_set_unlocked(obj->Event, obj->EventFlags);
  else
    mhs_event_set(obj->Event, obj->EventFlags);
  }
#endif
return(res);
}


/***************************************************************************/
/*            Object "Ereignis" auslösen & Sync Handler aufrufen           */
/***************************************************************************/
int32_t mhs_object_set_event(TMhsObj *obj)
{
TMhsObjContext *context;
TMhsObjHandler *handler;
int32_t res;

if (!obj)
  return(-1);
context = obj->Context;
if (obj->CbEnable)
  {
  for (handler = obj->SyncHandlerList; handler; handler = handler->Next)
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
mhs_enter_critical(context->Event);
res = mhs_object_set_event_marker(obj);
mhs_leave_critical(context->Event);
#ifdef MHS_OBJ_USE_EVENTS
if (obj->Event)
  mhs_event_set(obj->Event, obj->EventFlags);
#endif
return(res);
}


/***************************************************************************/
/*                                       */
/***************************************************************************/
void mhs_object_reset_event(TMhsObj *obj)
{
TMhsObjContext *context;

if (!obj)
  return;
context = obj->Context;
mhs_enter_critical(context->Event);
obj->EventCount = 0;
mhs_leave_critical(context->Event);
}


/***************************************************************************/
/*                                       */
/***************************************************************************/
int32_t mhs_object_get_event_count(TMhsObj *obj)
{
TMhsObjContext *context;
int32_t res;

if (!obj)
  return(-1);
context = obj->Context;
mhs_enter_critical(context->Event);
res = obj->EventCount;
mhs_leave_critical(context->Event);
return(res);
}


/***************************************************************************/
/*                      Event Callback binden                              */
/***************************************************************************/
int32_t mhs_object_event_connect(TMhsObj *obj, int32_t type, TMhsObjectCB proc, void *user_data)
{
int32_t res;
//TMhsObjContext *context;
TMhsObjHandler *handler;

if (!obj)
  return(-1);
//context = obj->Context;
//mhs_enter_critical(context->Event);
if (type)
  {
  for (handler = obj->SyncHandlerList; handler; handler = handler->Next)
    {
    if (handler->Proc == proc)
      break;
    }
  }
else
  {
  // Liste nach "handler" durchsuchen
  for (handler = obj->HandlerList; handler; handler = handler->Next)
    {
    if (handler->Proc == proc)
      break;
    }
  }
if (!handler)
  {
  if ((handler = mhs_obj_handler_new_unlocked(obj, type)))
    {
    handler->Proc = proc;
    handler->UserData = user_data;
    res = 0;
    }
  else
    res = -1;
  }
else
  res = 0;
//mhs_leave_critical(context->Event);
return(res);
}


/***************************************************************************/
/*                      Event Callback abtrennen                           */
/***************************************************************************/
int32_t mhs_object_event_disconnect(TMhsObj *obj, TMhsObjectCB proc)
{
int32_t res;
//TMhsObjContext *context;
TMhsObjHandler *list, *prev;

if (!obj)
  return(-1);
//context = obj->Context;
//mhs_enter_critical(context->Event);
// Liste nach "handler" durchsuchen
prev = NULL;
for (list = obj->SyncHandlerList; list; list = list->Next)
  {
  if (list->Proc == proc)
    {
    if (prev)
      prev->Next = list->Next;
    else
      obj->SyncHandlerList = list->Next;
    mhs_free(list);
    break;
    }
  prev = list;
  }
if (!list)
  {
  prev = NULL;
  // Liste nach "handler" durchsuchen
  for (list = obj->HandlerList; list; list = list->Next)
    {
    if (list->Proc == proc)
      {
      if (prev)
        prev->Next = list->Next;
      else
        obj->HandlerList = list->Next;
      mhs_free(list);
      break;
      }
    prev = list;
    }
  }
if (list)
  res = 0;
else
  res = -1;
//mhs_leave_critical(context->Event);
return(res);
}


int32_t mhs_object_event_block(TMhsObj *obj)
{
if (!obj)
  return(-1);
obj->CbEnable = 0;
return(0);
}


int32_t mhs_object_event_unblock(TMhsObj *obj)
{
if (!obj)
  return(-1);
obj->CbEnable = 1;
return(0);
}


int32_t mhs_object_set_user_data(TMhsObj *obj, void *user_ptr, uint32_t user_uint)
{
if (!obj)
  return(-1);
obj->UserPtr = user_ptr;
obj->UserUInt = user_uint;
return(0);
}
  

static void mhs_object_update_marked(TMhsObjContext *context)
{
TMhsObj *obj;

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


/***************************************************************************/
/*  "Ereignis" markiertes Object zurückgeben u. "Ereignis" löschen         */
/***************************************************************************/
TMhsObj *mhs_object_get_marked(TMhsObjContext *context)
{
TMhsObj *obj, *hit;

mhs_enter_critical(context->Event);
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
mhs_leave_critical(context->Event);
return(hit);
}


/***************************************************************************/
/*  Async Callbackfunktionen verarbeiten                                   */
/***************************************************************************/
int32_t mhs_process_events(TMhsObjContext *context)
{
int32_t i;
TMhsObj *obj;
TMhsObjHandler *handler;

if (!context)
  return(-1);
for (i = 50; i; i--)
  {
  obj = mhs_object_get_marked(context);

  if ((obj) && (obj->Funcs->Event))
    (obj->Funcs->Event)(obj);
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
return(0);
}


/***************************************************************************/
/*=========================================================================*/
/*            C O M M A N D O - E V E N T - F U N K T I O N E N            */
/*=========================================================================*/
/***************************************************************************/

/***************************************************************************/
/*                      Object "Ereignis" markieren                        */
/***************************************************************************/
static int32_t mhs_object_set_cmd_event_marker(TMhsObj *obj)
{
TMhsObjContext *context;

if (!obj)
  return(-1);
context = obj->Context;
obj->CmdEventCount++;
// **** Marked Liste verwalten
if (!obj->CmdMarkedFlag)
  {
  obj->CmdMarkedFlag = 1;
  if (!context->CmdMarkedFirst)
    context->CmdMarkedFirst = obj;
  if (context->CmdMarkedLast)
    context->CmdMarkedLast->CmdMarkedNext = obj;
  obj->CmdMarkedNext = NULL;
  context->CmdMarkedLast = obj;
  }
return(0);
}


/***************************************************************************/
/*                      Ein Event Handler erzeugen                         */
/***************************************************************************/
static TMhsObjHandler *mhs_cmd_obj_handler_new_unlocked(TMhsObj *obj, int32_t type)
{
TMhsObjHandler *handler, *list;

handler = (TMhsObjHandler *)mhs_calloc(1, sizeof(TMhsObjHandler));
if (!handler)
  return(NULL);
if (type)
  {
  list = obj->CmdSyncHandlerList;
  if (!list)
    obj->CmdSyncHandlerList = handler;
  else
    {
    while (list->Next)
      list = list->Next;
    list->Next = handler;
    handler->Next = NULL;
    }
  }
else
  {
  list = obj->CmdHandlerList;
  if (!list)
    obj->CmdHandlerList = handler;
  else
    {
    while (list->Next)
      list = list->Next;
    list->Next = handler;
    handler->Next = NULL;
    }
  }
return(handler);
}

/***************************************************************************/
/*            Object "Ereignis" auslösen & Sync Handler aufrufen           */
/***************************************************************************/
int32_t mhs_object_set_cmd_event(TMhsObj *obj)
{
TMhsObjContext *context;
TMhsObjHandler *handler;
int32_t res;

if (!obj)
  return(-1);
context = obj->Context;
for (handler = obj->CmdSyncHandlerList; handler; handler = handler->Next)
  {
  if (handler->Proc)
    (handler->Proc)(obj, handler->UserData);
  }
mhs_enter_critical(context->Event);
res = mhs_object_set_cmd_event_marker(obj);
mhs_leave_critical(context->Event);
/*#ifdef MHS_OBJ_USE_EVENTS
if (obj->Event)
  mhs_event_set(obj->Event, obj->EventFlags);
#endif */
return(res);
}

/***************************************************************************/
/*                      Event Callback binden                              */
/***************************************************************************/
int32_t mhs_object_cmd_event_connect(TMhsObj *obj, int32_t type, TMhsObjectCB proc, void *user_data)
{
int32_t res;
//TMhsObjContext *context;
TMhsObjHandler *handler;

if (!obj)
  return(-1);
//context = obj->Context;
//mhs_enter_critical(context->Event);
if (type)
  {
  for (handler = obj->CmdSyncHandlerList; handler; handler = handler->Next)
    {
    if (handler->Proc == proc)
      break;
    }
  }
else
  {
  // Liste nach "handler" durchsuchen
  for (handler = obj->CmdHandlerList; handler; handler = handler->Next)
    {
    if (handler->Proc == proc)
      break;
    }
  }
if (!handler)
  {
  if ((handler = mhs_cmd_obj_handler_new_unlocked(obj, type)))
    {
    handler->Proc = proc;
    handler->UserData = user_data;
    res = 0;
    }
  else
    res = -1;
  }
else
  res = 0;
//mhs_leave_critical(context->Event);
return(res);
}


/***************************************************************************/
/*                      Event Callback abtrennen                           */
/***************************************************************************/
int32_t mhs_object_cmd_event_disconnect(TMhsObj *obj, TMhsObjectCB proc)
{
int32_t res;
//TMhsObjContext *context;
TMhsObjHandler *list, *prev;

if (!obj)
  return(-1);
//context = obj->Context;
//mhs_enter_critical(context->Event);
// Liste nach "handler" durchsuchen
prev = NULL;
for (list = obj->CmdSyncHandlerList; list; list = list->Next)
  {
  if (list->Proc == proc)
    {
    if (prev)
      prev->Next = list->Next;
    else
      obj->CmdSyncHandlerList = list->Next;
    mhs_free(list);
    break;
    }
  prev = list;
  }
if (!list)
  {
  prev = NULL;
  // Liste nach "handler" durchsuchen
  for (list = obj->CmdHandlerList; list; list = list->Next)
    {
    if (list->Proc == proc)
      {
      if (prev)
        prev->Next = list->Next;
      else
        obj->CmdHandlerList = list->Next;
      mhs_free(list);
      break;
      }
    prev = list;
    }
  }
if (list)
  res = 0;
else
  res = -1;
//mhs_leave_critical(context->Event);
return(res);
}


static void mhs_object_update_cmd_marked(TMhsObjContext *context)
{
TMhsObj *obj;

// Alle Elemente durchlaufen
context->CmdMarkedFirst = NULL;
context->CmdMarkedLast = NULL;
for (obj = context->Items; obj; obj = obj->Next)
  {
  if (obj->CmdEventCount > 0)
    {
    obj->CmdMarkedFlag = 1;
    if (!context->CmdMarkedFirst)
      context->CmdMarkedFirst = obj;
    if (context->CmdMarkedLast)
      context->CmdMarkedLast->CmdMarkedNext = obj;
    obj->CmdMarkedNext = NULL;
    context->CmdMarkedLast = obj;
    }
  else
    obj->CmdMarkedFlag = 0;
  }
}


/***************************************************************************/
/*  "Ereignis" markiertes Object zurückgeben u. "Ereignis" löschen         */
/***************************************************************************/
TMhsObj *mhs_object_get_cmd_marked(TMhsObjContext *context)
{
TMhsObj *obj, *hit;

mhs_enter_critical(context->Event);
if (context->CmdDeleteItems)
  {
  mhs_object_update_cmd_marked(context);
  context->CmdDeleteItems = 0;
  }

hit = NULL;
obj = context->CmdMarkedFirst;
if (obj)
  {
  if (obj->CmdEventCount > 0)
    {
    hit = obj;
    obj->CmdEventCount = 0;
    }
  obj->CmdMarkedFlag = 0;
  if (!(context->CmdMarkedFirst = obj->CmdMarkedNext))
    context->CmdMarkedLast = NULL;
  }
mhs_leave_critical(context->Event);
return(hit);
}


/***************************************************************************/
/*  Cmd Event status abfragen und löschen                                  */
/***************************************************************************/

int32_t mhs_object_get_cmd_event_count(TMhsObj *obj, uint32_t clear)
{
TMhsObj *list, *prev;
TMhsObjContext *context;
int32_t res;

if (!obj)
  return(-1);
context = obj->Context;
mhs_enter_critical(context->Event);
res = obj->EventCount;
if ((res) && (clear))
  {
  obj->EventCount = 0;
  obj->CmdMarkedFlag = 0;
  prev = NULL;
  for (list = context->CmdMarkedFirst; list; list = list->CmdMarkedNext)
    {
    if (obj == list)
      {
      if (prev)
        {
        prev->CmdMarkedNext = list->CmdMarkedNext;
        if (context->CmdMarkedLast == list)
          context->CmdMarkedLast = prev;
        }
      else
        {
        if (!(context->CmdMarkedFirst = list->CmdMarkedNext))
          context->CmdMarkedLast = NULL;
        }  
      break;
      }
    prev = list;      
    }  
  }
mhs_leave_critical(context->Event);
return(res);
}


/***************************************************************************/
/*  Async Callbackfunktionen verarbeiten                                   */
/***************************************************************************/
int32_t mhs_process_cmd_events(TMhsObjContext *context)
{
int32_t i;
TMhsObj *obj;
TMhsObjHandler *handler;

if (!context)
  return(-1);
for (i = 50; i; i--)
  {
  obj = mhs_object_get_cmd_marked(context);
  if (!obj)
    break;

  for (handler = obj->CmdHandlerList; handler; handler = handler->Next)
    {
    if (handler->Proc)
      (handler->Proc)(obj, handler->UserData);
    }
  }
return(0);
}
