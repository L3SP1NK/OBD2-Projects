/***************************************************************************
                         str_list.c  -  description
                             -------------------
    begin             : 11.04.2011
    last modify       : 18.02.2019
    copyright         : (C) 2011 - 2019 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <string.h>
#include "util.h"
#include "str_list.h"



TStrList *StrListAdd(TStrList **list, const char *str)
{
TStrList *neu, *ptr;
int32_t size, str_len;

if ((!list) || (!str))
  return(NULL);
str_len = (int32_t)strlen(str) + 1;  
size = sizeof(TStrList) + str_len;
if (!(neu = mhs_calloc(1, size)))
  return(NULL);
neu->Next = NULL;
safe_strcpy(neu->String, (uint32_t)str_len, str);
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


void StrListDestroy(TStrList **list)
{
TStrList *ptr, *l;

if (!list)
  return;
l = *list;
while (l != NULL)
  {
  ptr = l->Next;
  mhs_free(l);
  l = ptr;
  }
*list = NULL;
}


int32_t StrListFind(TStrList *list, const char *str)
{
int32_t idx;

if (!str)
  return(0);
idx = 1;
for (; list; list = list->Next)
  {
  if (!strcmp(str, list->String))
    return(idx);
  idx++;
  }
return(0);
}
