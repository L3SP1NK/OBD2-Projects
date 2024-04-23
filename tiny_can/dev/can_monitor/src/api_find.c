/***************************************************************************
                         api_find.c  -  description
                             -------------------
    begin             : 13.09.2008
    copyright         : (C) 2008 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <stdlib.h>
#include "util.h"
#include "paths.h"
#include "drv_scan.h"
#ifdef __WIN32__
#include "win_reg.h"
#endif
#include "api_find.h"

#define TINY_CAN_DIR "tiny_can"
#define CAN_API_DIR  "can_api"


struct TApiPathList
  {
  struct TApiPathList *Next;
  char *Path;
  };

struct TApiPathList *ApiPathList = NULL;

void ApiPathListDestroy(void)
{
struct TApiPathList *l, *h;

l = ApiPathList;
while (l)
  {
  h = l->Next;
  save_free(l->Path);
  g_free(l);
  l = h;
  }
ApiPathList = NULL;
}


static struct TApiPathList *ApiPathListAdd(struct TApiPathList **list)
{
struct TApiPathList *l;

l = *list;
if (!l)
  {
  // Liste ist leer
  l = (struct TApiPathList *)g_malloc0(sizeof(struct TApiPathList));
  *list = l;
  }
else
  { // Neues Element anhängen
  while (l->Next != NULL) l = l->Next;
  l->Next = (struct TApiPathList *)g_malloc0(sizeof(struct TApiPathList));
  l = l->Next;
  if (!l)
    return(NULL);        // Nicht genügend Arbetsspeicher
  }
l->Next = NULL;
return(l);
}


int ApiPathAdd(char *path)
{
struct TApiPathList *item;

item = ApiPathListAdd(&ApiPathList);
if (!item)
  return(-1);
item->Path = g_strdup(path);
return(0);
}



void ApiPathListCreate(void)
{
char *path, *api_path;

ApiPathListDestroy();
#ifdef __WIN32__
if ((path = RegReadApiPath()))
  {
  ApiPathAdd(path);
  g_free(path);
  }
#endif
path = g_strdup(Paths.base_dir);
if ((api_path = g_strstr_len(path, -1,TINY_CAN_DIR)))
  {
  *api_path = '\0';
  api_path = g_strconcat(path, TINY_CAN_DIR, G_DIR_SEPARATOR_S, CAN_API_DIR, G_DIR_SEPARATOR_S, NULL);
  ApiPathAdd(api_path);
  save_free(api_path);
  }
save_free(path);
ApiPathAdd(Paths.base_dir);
}


char *GetApiPath(void)
{
struct TApiPathList *list;

for (list = ApiPathList; list; list = list->Next)
  {
  if (IsDriverDir(list->Path))
    break;
  }
if (list)
  return(g_strdup(list->Path));
else
  return(NULL);
}


