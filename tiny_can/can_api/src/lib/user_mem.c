/***************************************************************************
                         user_mem.c  -  description
                             -------------------
    begin             : 09.09.2013

    copyright         : (C) 2013 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <string.h>
#include "util.h"
#include "user_mem.h"


TMemList *UserMemList = NULL;


void UserMemInit(void)
{
UserMemList = NULL;
}


TMemList *MemAddRef(TMemList **list, const char *str, void *ref)
{
TMemList *neu, *ptr;

if ((!ref) || (!list))
  return(NULL);
if (!(neu = mhs_calloc(1, sizeof(TMemList))))
  return(NULL);
neu->Next = NULL;
neu->Ref = ref;
neu->Description = mhs_strdup(str);
ptr = *list;
if (!ptr)
  // Liste ist leer
  *list = neu;
else
  { // Neues Element anhängen
  while (ptr->Next != NULL) ptr = ptr->Next;
    ptr->Next = neu;
  }
return(neu);
}


void MemDestroy(TMemList **list, uint32_t ref_free)
{
TMemList *ptr, *l;
void *mem;

if (!list)
  return;
l = *list;
while (l != NULL)
  {
  ptr = l->Next;
  safe_free(l->Description);
  if ((ref_free) && ((mem = l->Ref)))
    mhs_free(mem);
  mhs_free(l);
  l = ptr;
  }
*list = NULL;
}


int32_t MemDestroyEntry(TMemList **list, TMemList *entry, uint32_t ref_free)
{
TMemList *l, *prev, *mem;

if (!list)
  return(-1);
l = *list;
prev = NULL;
for (; l; l = l->Next)
  {
  if (l == entry)
    {
    if (prev)
      prev->Next = l->Next;
    else
      *list = l->Next;
    safe_free(l->Description);
    if ((ref_free) && ((mem = l->Ref)))
      mhs_free(mem);
    mhs_free(l);
    return(0);
    }
  prev = l;
  }
return(-1);
}


int32_t MemDestroyEntryByRef(TMemList **list, void *ref, uint32_t ref_free)
{
TMemList *l, *prev;
void *mem;

if (!list)
  return(-1);
l = *list;
prev = NULL;
for (; l; l = l->Next)
  {
  if (l->Ref == ref)
    {
    if (prev)
      prev->Next = l->Next;
    else
      *list = l->Next;
    safe_free(l->Description);
    if ((ref_free) && ((mem = l->Ref)))
      mhs_free(mem);
    mhs_free(l);
    return(0);
    }
  prev = l;
  }
return(-1);
}


TMemList *GetEntryByRef(TMemList *list, void *ref)
{
for (; list; list = list->Next)
  {
  if (list->Ref == ref)
    return(list);
  }
return(NULL);
}
