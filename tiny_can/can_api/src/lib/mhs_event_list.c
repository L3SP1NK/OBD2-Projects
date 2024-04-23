/***************************************************************************
                         mhs_event_list.c  -  description
                             -------------------
    begin             : 16.10.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "util.h"
#include "mhs_event.h"
#include "mhs_event_list.h"



static TMhsEventItem *EventListItemAdd(TMhsEventItem **items)
{
TMhsEventItem *neu, *ptr;

if (!items)
  return(NULL);  
if (!(neu = (TMhsEventItem *)mhs_calloc(1, sizeof(TMhsEventItem))))
  return(NULL);
neu->Next = NULL;
ptr = *items;
if (!ptr)
  // Liste ist leer
  *items = neu;
else
  { // Neues Element anhängen
  while (ptr->Next != NULL) ptr = ptr->Next;
    ptr->Next = neu;
  }
return(neu);
}


static void EventListClear(TMhsEventItem *items)
{
TMhsEventItem *ptr;

while (items)
  {
  ptr = items->Next;
  mhs_free(items);
  items = ptr;
  }
}


static void EventListDeleteItem(TMhsEventItem **items, TMhsEvent *event)
{
TMhsEventItem *list, *prev;

if (!items)
  return;
prev = NULL;
for (list = *items; list; list = list->Next)
  {
  if (list->Event == event)
    {
    if (prev)
      prev->Next = list->Next;
    else
      *items = list->Next;           
    mhs_free(list);
    break;
    }
  prev = list;
  }
}



TMhsEventList *MhsEventListCreate(void)
{
TMhsEventList *list;

if (!(list = (TMhsEventList *)mhs_calloc(1, sizeof(TMhsEventList))))
  return(NULL);
DRV_LOCK_INIT(list->Lock);
return(list);
}


void MhsEventListDestroy(TMhsEventList **event_list) 
{
TMhsEventList *list;

if (!event_list)
  return;
if (!(list = *event_list))
  return;
EventListClear(list->Items);
DRV_LOCK_DESTROY(list->Lock);
mhs_free(list);    
*event_list = NULL;  
}


void MhsEventListAdd(TMhsEventList *event_list, TMhsEvent *event, uint32_t events)
{
TMhsEventItem *item;

if (!event_list)
  return;  
DRV_LOCK_ENTER(event_list->Lock);
if ((item = EventListItemAdd(&event_list->Items)))
  {
  item->Event = event;
  item->Events = events;
  }
DRV_LOCK_LEAVE(event_list->Lock);  
}


void MhsEventListDelete(TMhsEventList *event_list, TMhsEvent *event)
{
if (!event_list)
  return;
DRV_LOCK_ENTER(event_list->Lock);
EventListDeleteItem(&event_list->Items, event);
DRV_LOCK_LEAVE(event_list->Lock);  
} 


void MhsEventListAddDelete(TMhsEventList *event_list, TMhsEvent *event, uint32_t events)
{
if (!event_list)
  return;
if (events)  
  MhsEventListAdd(event_list, event, events);
else
  MhsEventListDelete(event_list, event);  
} 


void MhsEventListExecute(TMhsEventList *event_list)
{
TMhsEventItem *item;

if (!event_list)
  return;
DRV_LOCK_ENTER(event_list->Lock);
for (item = event_list->Items; item; item = item->Next)
  mhs_event_set(item->Event, item->Events);
DRV_LOCK_LEAVE(event_list->Lock);  
}
