/***************************************************************************
                         mhs_user_event.c  -  description
                             -------------------
    begin             : 11.10.2015
    copyright         : (C) 2015 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "util.h"
#include "mhs_event.h"
#include "mhs_user_event.h"


static TMhsUserEvent *UserEventsList = NULL;


void mhs_user_event_init(void)
{
UserEventsList = NULL;
}


TMhsEvent *mhs_user_event_create(void)
{
TMhsUserEvent *neu, *ptr;

if (!(neu = (TMhsUserEvent *)mhs_event_create_ex(sizeof(TMhsUserEvent))))
  return(NULL);
neu->Next = NULL;
ptr = UserEventsList;
if (!ptr)
  // Liste ist leer
  UserEventsList = neu;
else
  { // Neues Element anhängen
  while (ptr->Next != NULL) ptr = ptr->Next;
    ptr->Next = neu;
  }
return((TMhsEvent *)neu);
}


void mhs_user_event_destroy(void)
{
TMhsUserEvent *ptr, *l;

l = UserEventsList;
while (l)
  {
  ptr = l->Next;   
  mhs_free(l);
  l = ptr;
  }
UserEventsList = NULL;
}


void mhs_user_event_set_all(uint32_t events)
{
TMhsUserEvent *l;

for (l = UserEventsList; l; l = l->Next)
  {
  mhs_event_set((TMhsEvent *)l, events);
  }
}


int32_t mhs_user_event_valid(TMhsEvent *event_obj)
{
TMhsUserEvent *l;

if (!event_obj)
  return(0);
for (l = UserEventsList; l; l = l->Next)
  {
  if (l == (TMhsUserEvent *)event_obj)
    return(0);
  }
return(-1);
}
