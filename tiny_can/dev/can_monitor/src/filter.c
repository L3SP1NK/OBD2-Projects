/* *********** Tiny-CAN Monitor **************                            */
/* Copyright (C) 2007 Klaus Demlehner (klaus@mhs-elektronik.de)           */
/*                                                                        */
/*   Tiny-CAN Project Homepage: http://www.mhs-elektronik.de              */
/*                                                                        */
/* This program is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by   */
/* the Free Software Foundation; either version 2 of the License, or      */
/* (at your option) any later version.                                    */
/*                                                                        */
/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */
/*                                                                        */
/* You should have received a copy of the GNU General Public License      */
/* along with this program; if not, write to the Free Software            */
/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
#include <glib.h>
#include "can_monitor.h"
#include "hex_entry.h"
#include "dialogs.h"
#include "main_can.h" // CanModul
#include "support.h"
#include "filter.h"

#define SW_FILTER_START_INDEX (INDEX_SOFT_FLAG | 0x00000001L)
#define HW_FILTER_START_INDEX 0x00000001L

#define MAX_LINE_SIZE 255


struct TFilterHeader
  {
  char ID[8];
  unsigned long Count;
  };


const struct TCanMsg INVALID_CAN_MESSSAGE = {.Id = 0xFFFFFFFFL, .Flags.Long = 0x0L, .Data.Bytes = {0,0,0,0,0,0,0,0}, .Time = {0x0L, 0x0L}};
//const struct TCanMsg INVALID_CAN_MESSSAGE = {/*.Id =*/ 0xFFFFFFFFL, /* .Flags.Long = */ 0x0L, /*.Data.Bytes = */ 0,0,0,0,0,0,0,0, /*.Time = */ 0x0L, 0x0L};

/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
static struct TFilter *Filter = NULL;
static GMutex *FilterLock = NULL;

/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
int FilterReadFromTxtFile(char *file_name);
int FilterReadFromBinFile(char *file_name);



int FilCreate(void)
{
FilterLock = g_mutex_new();
Filter = NULL;
return(0);
}


void FilDestroy(void)
{
FilClearListe(FILTER_ALL);
g_mutex_free(FilterLock);
}


void FilClearListe(unsigned long maske)
{
struct TFilter *filter, *clear;

g_mutex_lock(FilterLock);
filter = Filter;
while (filter)
  {
  if (filter->Flags & maske)
    {
    if (filter->Prev)
      filter->Prev->Next = filter->Next;
    if (filter->Next)
      filter->Next->Prev = filter->Prev;
    if (Filter == filter)
      Filter = Filter->Next;
    clear = filter;
    filter = filter->Next;
    g_free(clear);
    }
  else
    filter = filter->Next;
  }
g_mutex_unlock(FilterLock);
}


void FilClearPlugins(struct TPlugin *plugin)
{
struct TFilter *filter, *clear;

if (!plugin)
  return;
g_mutex_lock(FilterLock);
filter = Filter;
while (filter)
  {
  if (filter->Plugin == plugin)
    {
    if (filter->Prev)
      filter->Prev->Next = filter->Next;
    if (filter->Next)
      filter->Next->Prev = filter->Prev;
    if (Filter == filter)
      Filter = Filter->Next;
    clear = filter;
    filter = filter->Next;
    g_free(clear);
    }
  else
    filter = filter->Next;
  }
g_mutex_unlock(FilterLock);
FilEditingFinish();
}


void FilRemove(struct TFilter *filter)
{
struct TFilter *tmp;

g_mutex_lock(FilterLock);
tmp = Filter;
while (tmp)
  {
  if (tmp != filter)
    tmp = tmp->Next;
  else
    {
    if (tmp->Prev)
      tmp->Prev->Next = tmp->Next;
    if (tmp->Next)
      tmp->Next->Prev = tmp->Prev;
    if (Filter == tmp)
      Filter = Filter->Next;

    g_free(tmp);
    break;
    }
  }
g_mutex_unlock(FilterLock);
}


struct TFilter *FilCreateNewUnlocked(void)
{
struct TFilter *new_filter;
struct TFilter *last;

new_filter = (struct TFilter *)g_malloc0(sizeof(struct TFilter));
if (!new_filter)
  return(NULL);
new_filter->Next = NULL;

last = Filter;
if (last)
  {
  while (last->Next)
    last = last->Next;

  last->Next = new_filter;
  new_filter->Prev = last;
  }
else
  {
  new_filter->Prev = NULL;
  Filter = new_filter;
  }
// Default werte setzen
new_filter->Flags = 0;
new_filter->Plugin = NULL;
new_filter->Name[0] = 0;
new_filter->UpdateMsg = 1;
new_filter->ChangeMsg = 1;
new_filter->UpdateParam = 1;
new_filter->HardwareFilter = 0;
new_filter->PassMessage = 0;
new_filter->IdMode = 0;
new_filter->FormatEff = 0;
new_filter->Id1 = 0L;
new_filter->Id2 = 0L;
new_filter->Index = 0L;
memcpy(&new_filter->CanMsg, &INVALID_CAN_MESSSAGE, sizeof(struct TCanMsg));
return(new_filter);
}


struct TFilter *FilCreateNew(void)
{
struct TFilter *new_filter;

g_mutex_lock(FilterLock);
new_filter = FilCreateNewUnlocked();
g_mutex_unlock(FilterLock);
return(new_filter);
}


int FilChangeUnlocked(struct TFilter *filter, struct TFilter *new_filter)
{
if ((!filter) || (!new_filter))
  return(-1);
strcpy(filter->Name, new_filter->Name);
filter->Flags = new_filter->Flags;
filter->Plugin = new_filter->Plugin;
filter->UpdateMsg = new_filter->UpdateMsg;
filter->ChangeMsg = new_filter->ChangeMsg;
filter->UpdateParam = new_filter->UpdateParam;
filter->HardwareFilter = new_filter->HardwareFilter;
filter->PassMessage = new_filter->PassMessage;
filter->IdMode = new_filter->IdMode;
filter->FormatEff = new_filter->FormatEff;
filter->Id1 = new_filter->Id1;
filter->Id2 = new_filter->Id2;
filter->Index = new_filter->Index;
return(0);
}


int FilChange(struct TFilter *filter, struct TFilter *new_filter)
{
int res;

g_mutex_lock(FilterLock);
res = FilChangeUnlocked(filter, new_filter);
g_mutex_unlock(FilterLock);
return(res);
}


int FilAddNew(struct TFilter *filter)
{
int res;
struct TFilter *new_filter;

g_mutex_lock(FilterLock);
if ((new_filter = FilCreateNewUnlocked()))
  {
  FilChangeUnlocked(new_filter, filter);
  res = 0;
  }
else
  res = -1;
g_mutex_unlock(FilterLock);
return(res);
}



int FilMoveBefore(struct TFilter *filter)
{
int res;
struct TFilter *head, *before, *end;

if (!filter)
  return(-1);
g_mutex_lock(FilterLock);
if ((before = filter->Prev))
  {
  head = before->Prev;
  end = filter->Next;
  // Next
  if (head)
    head->Next = filter;
  before->Next = end;
  filter->Next = before;
  // Prev
  if (end)
    end->Prev = before;
  before->Prev = filter;
  filter->Prev = head;
  // Start Check
  if (before == Filter)
    Filter = filter;
  res = 0;
  }
else
  res = -1;
g_mutex_unlock(FilterLock);
return(res);
}


int FilMoveAfter(struct TFilter *filter)
{
int res;
struct TFilter *head, *after, *end;

if (!filter)
  return(-1);
g_mutex_lock(FilterLock);
if ((after = filter->Next))
  {
  head = filter->Prev;
  end = after->Next;
  // Next
  if (head)
    head->Next = after;
  filter->Next = end;
  after->Next = filter;
  // Prev
  if (end)
    end->Prev = filter;
  filter->Prev = after;
  after->Prev = head;
  // Start Check
  if (filter == Filter)
    Filter = after;
  res = 0;
  }
else
  res = -1;
g_mutex_unlock(FilterLock);
return(res);
}


struct TFilter *FilGetLast(void)
{
struct TFilter *filter;

filter = Filter;
if (filter)
  {
  while (filter->Next)
    filter = filter->Next;
  }
return(filter);
}


struct TFilter *FilGetFirst(void)
{
struct TFilter *filter;

filter = Filter;
if (filter)
  {
  while (filter->Prev)
    filter = filter->Prev;
  }
return(filter);
}


unsigned int FilGetLength(unsigned long maske)
{
unsigned int len;
struct TFilter *filter;

len = 0;
for (filter = Filter; filter; filter = filter->Next)
  {
  if (maske == FILTER_VISIBLE)
    {
    if ((filter->Flags & FILTER_ENABLED) && (filter->Flags & FILTER_SHOW))
      len++;
    }
  else
    {
    if (filter->Flags & maske)
      len++;
    }
  }
return(len);
}


struct TFilter *FilGetByPos(unsigned long maske, unsigned long pos)
{
struct TFilter *filter;
unsigned long p;

p = 0;
for (filter = Filter; filter; filter = filter->Next)
  {
  if (maske == FILTER_VISIBLE)
    {
    if ((filter->Flags & FILTER_ENABLED) && (filter->Flags & FILTER_SHOW))
      {
      if (p++ == pos)
        return(filter);
      }
    }
  else
    {
    if (filter->Flags & maske)
      {
      if (p++ == pos)
        return(filter);
      }
    }
  }
return(NULL);
}


struct TFilter *FilGetByIdx(unsigned long index)
{
struct TFilter *filter;

for (filter = Filter; filter; filter = filter->Next)
  {
  if (filter->Index == index)
    return(filter);
  }
return(NULL);
}


void FilCreateIndexList(void)
{
struct TFilter *filter;
unsigned long hw_idx, sw_idx;

hw_idx = HW_FILTER_START_INDEX;
sw_idx = SW_FILTER_START_INDEX;
for (filter = Filter; filter; filter = filter->Next)
  {
  if (filter->HardwareFilter)
    {
    if ((hw_idx & 0x0000FFFFL) > CanModul.FilterCount)
      filter->Index = sw_idx++;
    else
      filter->Index = hw_idx++;
    }
  else
    filter->Index = sw_idx++;
  filter->UpdateParam = 1;
  }
}


unsigned long FilGetNewIndex(unsigned long can_kanal, unsigned char hw_filter)
{
struct TFilter *filter;
unsigned long idx;
int hit, i;

if (hw_filter)
  idx = HW_FILTER_START_INDEX | can_kanal;
else
  idx = SW_FILTER_START_INDEX | can_kanal;

hit = 1;
for (i = FilGetLength(FILTER_ALL); i; i--)
  {
  for (filter = Filter; filter; filter = filter->Next)
    {
    if (idx == filter->Index)
      {
      hit = 0;
      break;
      }
    }
  if (hit)
    break;
  }
if (hit)
  {
  if (hw_filter)
    {
    if ((idx & 0x0000FFFFL) > CanModul.FilterCount)
      return(0xFFFFFFFFL);
    else
      return(idx);
    }
  else
    return(idx);
  }
else
  return(0xFFFFFFFFL);
}


int FilSetToDriver(unsigned int all)
{
struct TFilter *filter;
struct TMsgFilter msg_filter;

if (all)
  CanSetMode(0L, OP_CAN_NO_CHANGE, CAN_CMD_HW_FILTER_CLEAR | CAN_CMD_SW_FILTER_CLEAR);
for (filter = Filter; filter; filter = filter->Next)
  {
  if ((all) || (filter->UpdateParam))
    {
    msg_filter.FilFlags = 0L;
    if (filter->Flags & FILTER_ENABLED)
      msg_filter.FilEnable = 1;
    if (filter->PassMessage)
      msg_filter.FilMode = 1;
    switch (filter->IdMode)
      {
      case 0 : {  // Single
               msg_filter.FilIdMode = 2;
               break;
               }
      case 1 : {  // Range
               msg_filter.FilIdMode = 1;
               break;
               }
      case 2 : {  // Masked
               msg_filter.FilIdMode = 0;
               break;
               }
      }
    if (filter->FormatEff)
      msg_filter.FilEFF = 1;
    msg_filter.Code = filter->Id1;
    msg_filter.Maske = filter->Id2;

    if (CanSetFilter(filter->Index, &msg_filter) < 0)
      return(-1);
    filter->UpdateParam = 0;
    }
  }
return(0);
}


struct TFilter *FilReadUpdMessage(struct TFilter *filter, struct TCanMsg *msg)
{
if (!msg)
  return(NULL);
if (!filter)
  filter = Filter;
g_mutex_lock(FilterLock);
for (; filter; filter = filter->Next)
  {
  if (filter->UpdateMsg)
    {
    memcpy(msg, &filter->CanMsg, sizeof(struct TCanMsg));
    filter->UpdateMsg = 0;
    break;
    }
  }
g_mutex_unlock(FilterLock);
return(filter);
}


struct TFilter *FilWriteMessageByIdx(unsigned long index, struct TCanMsg *msg)
{
struct TFilter *filter;

g_mutex_lock(FilterLock);
filter = FilGetByIdx(index);
if (filter)
  {
  filter->UpdateMsg = 1;
  if ((filter->CanMsg.Id != msg->Id) || (filter->CanMsg.MsgLen != msg->MsgLen))
    filter->ChangeMsg = 1;
  else if (memcmp(filter->CanMsg.MsgData, &msg->MsgData[0], msg->MsgLen))
    filter->ChangeMsg = 1;
  memcpy(&filter->CanMsg, msg, sizeof(struct TCanMsg));
  }
g_mutex_unlock(FilterLock);
return(filter);
}


/**************************************************************/
/* Filter laden                                               */
/**************************************************************/
int FilterReadFromFile(char *file_name)
{
int res;

if ((res = FilterReadFromTxtFile(file_name)) < 0)
  {
  if (FilterReadFromBinFile(file_name) >= 0)
    res = 0;
  }
if (res < 0)
  {
  FilClearListe(STANDART_FILTER);
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error loading filter description file"));
  }
FilEditingFinish();
return(res);
}


int FilterReadFromTxtFile(char *file_name)
{
struct TFilter new_filter;
FILE *datei;
int err;
char line[MAX_LINE_SIZE];
char hlp[MAX_LINE_SIZE];
char *str, *item;

err = 0;
datei = NULL;
if (g_file_test(file_name, G_FILE_TEST_IS_REGULAR))
  datei = fopen(file_name, "rt");
else
  err = -1;
if (!datei)
  err = -1;

if (!err)
  {
  err = -2;
  while (fgets(line, MAX_LINE_SIZE-1, datei))
    {
    if (sscanf(line, "[%[^]]", hlp) == 1)
      {
      str = hlp;
      g_strstrip(str);
      if (!g_ascii_strcasecmp(str, "FILTER"))
        {
        err = 0;
        break;
        }
      }
    }
  }

if (!err)
  {
  FilClearListe(STANDART_FILTER);
  while (fgets(line, MAX_LINE_SIZE-1, datei))
    {
    if (sscanf(line, "[%[^]]", hlp) == 1)
      break;
    str = line;
    // Spalte 1 : Name
    item = GetItemAsString(&str, "=", NULL);
    if (!item)
      break;
    g_strlcpy(new_filter.Name, item, 40);
    // Spalte 2 : Flags
    new_filter.Flags = (GetItemAsULong(&str, ";", 0, NULL) & 0xFFFFFFFCL) | STANDART_FILTER;
    // Spalte 3 : Hardware Filter
    new_filter.HardwareFilter = (unsigned char)GetItemAsULong(&str, ";", 0, NULL);
    // Spalte 4 : Pass Message
    new_filter.PassMessage = (unsigned char)GetItemAsULong(&str, ";", 0, NULL);
    // Spalte 5 : Id Mode
    new_filter.IdMode = (unsigned char)GetItemAsULong(&str, ";", 0, NULL);
    // Spalte 6 : FormatEff
    new_filter.FormatEff = (unsigned char)GetItemAsULong(&str, ";", 0, NULL);
    // Spalte 7 : Id 1
    new_filter.Id1 = GetItemAsULong(&str, ";", 0, NULL);
    // Spalte 8 : Id 2
    new_filter.Id2 = GetItemAsULong(&str, ";", 0, NULL);
    FilAddNew(&new_filter);
    }
  }
if (datei)
  fclose(datei);
return(err);
}


/* int FilterReadFromTxtFile(char *file_name)
{
struct TFilter new_filter;
FILE *datei;
int err;
char line[MAX_LINE_SIZE];
char name[MAX_LINE_SIZE];
char item[MAX_LINE_SIZE];
char *str, *s, *endptr;

err = 0;
datei = NULL;
if (g_file_test(file_name, G_FILE_TEST_IS_REGULAR))
  datei = fopen(file_name, "rt");
else
  err = -1;
if (!datei)
  err = -1;

if (!err)
  {
  err = -2;
  while (fgets(line, MAX_LINE_SIZE-1, datei))
    {
    str = line;
    if (sscanf(str, "[%[^]]", item) == 1)
      {
      str = item;
      strstrip(&str);
      strupc(str);
      if (strcmp(str, "FILTER") == 0)
        {
        err = 0;
        break;
        }
      }
    }
  }

if (!err)
  {
  FilClearListe(STANDART_FILTER);
  while (fgets(line, MAX_LINE_SIZE-1, datei))
    {
    if (sscanf(line, "[%[^]]", name) == 1)
      break;
    // Spalte 1: Name
    if (sscanf(line, "\"%[^\"]\" = %[^]", name, item) == 2)
      {
      if (!strcmp(name, "\"\""))
        {
        err = -1;
        break;
        }
      g_strlcpy(new_filter.Name, name, 40);

      new_filter.HardwareFilter = (unsigned char)strtol(s, (char**)&endptr, 0);
      equal_error_break(s, endptr);
      s = endptr;
      jump_next_item(s);
      new_filter.PassMessage = (unsigned char)strtol(s, (char**)&endptr, 0);
      if (s == endptr)
        {
        err = -1;
        break;
        }
      s = endptr;
      jump_next_item(s);
      new_filter.IdMode = (unsigned char)strtol(s, (char**)&endptr, 0);
      if (s == endptr)
        {
        err = -1;
        break;
        }
      s = endptr;
      jump_next_item(s);
      new_filter.FormatEff = (unsigned char)strtol(s, (char**)&endptr, 0);
      if (s == endptr)
        {
        err = -1;
        break;
        }
      s = endptr;
      jump_next_item(s);
      new_filter.Id1 = (unsigned long)strtol(s, (char**)&endptr, 0);
      if (s == endptr)
        {
        err = -1;
        break;
        }
      s = endptr;
      jump_next_item(s);
      new_filter.Id2 = (unsigned long)strtol(s, (char**)&endptr, 0);
      if (s == endptr)
        {
        err = -1;
        break;
        }
      FilAddNew(&new_filter);
      }
    }
  }
if (datei)
  fclose(datei);
if (err)
  {
  FilClearListe(STANDART_FILTER);
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Filter Datei kann nicht geladen werden"));
  }
FilEditingFinish();
return(err);
} */


int FilterReadFromBinFile(char *file_name)
{
FILE *datei;
int err;
struct TFilterHeader filter_header;
struct TFilter filter_puffer;
int i;

err = 0;
datei = NULL;
if (g_file_test(file_name, G_FILE_TEST_IS_REGULAR))
  datei = fopen(file_name, "rb");
else
  err = -1;
if (!datei)
  err = -1;
if (!err)
  {
  if (fread(&filter_header, sizeof(struct TFilterHeader), 1, datei) != 1)
    err = -1;
  }
if (!err)
  {
  FilClearListe(STANDART_FILTER);
  if (filter_header.Count)
    {
    for (i = 0; i < (int)filter_header.Count; i++)
      {
      if (fread(&filter_puffer, sizeof(struct TFilter), 1, datei) != 1)
        {
        err = -1;
        break;
        }
      filter_puffer.Flags = (filter_puffer.Flags & 0xFFFFFFFCL) | STANDART_FILTER;
      FilAddNew(&filter_puffer);
      }
    }
  }
if (datei)
  fclose(datei);
return(err);
}


/**************************************************************/
/* Filter laden                                               */
/**************************************************************/
int FilterWriteToFile(char *file_name)
{
int err;
struct TFilter *filter;
FILE *datei;

err = 0;
datei = fopen(file_name, "wt");
if (!datei)
  err = -1;
else
  {
  fprintf(datei, "[FILTER]\n");
  for (filter = Filter; filter; filter = filter->Next)
    {
    if (filter->Flags & STANDART_FILTER)
      {
      fprintf(datei, "´%s´ = 0x%08lX; %d; %d; %d; %d; 0x%08lX; 0x%08lX\n", filter->Name, filter->Flags, filter->HardwareFilter,
             filter->PassMessage, filter->IdMode, filter->FormatEff, filter->Id1, filter->Id2);
      }
    }
  }
if (datei)
  fclose(datei);
if (err)
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error saving filter description file"));
return(err);
}

/*
int FilterWriteToFile(char *file_name)
{
FILE *datei;
int error;
struct TFilterHeader filter_header;
struct TFilter *filter;

error = 0;
datei = fopen(file_name, "wb");
if (!datei)
  error = -1;
if (!error)
  {
  filter_header.Count = FilGetLength(STANDART_FILTER);
  if (fwrite(&filter_header, sizeof(struct TFilterHeader), 1, datei) != 1)
    error = -1;
  }
if (!error)
  {
  for (filter = Filter; filter; filter = filter->Next)
    {
    if (filter->Flags & STANDART_FILTER)
      {
      if (fwrite(filter, sizeof(struct TFilter), 1, datei) != 1)
        {
        error = -1;
        break;
        }
      }
    }
  }
fclose(datei);
if (error)
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Filter Datei kann nicht gespeichert werden"));
return(error);
} */


int FilSetByPlugin(struct TPlugin *plugin, char *name, struct TFilter *filter)
{
int res;
struct TFilter *l;

res = 0;
if ((!plugin) || (!name) || (!filter))
  return(-1);
for (l = Filter; l; l = l->Next)
  {
  if ((l->Plugin == plugin) && (l->Flags & PLUGIN_FILTER))
    {
    if (!strcmp(l->Name, name))
      {
      res = 1;
      break;
      }
    }
  }
filter->Flags |= PLUGIN_FILTER;
filter->Plugin = plugin;
strncpy(filter->Name, name, 50);
if (res)
  return(FilChange(l, filter));
else
  return(FilAddNew(filter));
}


int FilGetByPlugin(struct TPlugin *plugin, char *name, struct TFilter *filter)
{
struct TFilter *l;

if ((!name) || (!filter))
  return(-1);
for (l = Filter; l; l = l->Next)
  {
  if ((l->Plugin == plugin) && (l->Flags & PLUGIN_FILTER))
    {
    if (!strcmp(l->Name, name))
      {
      memcpy(filter, l, sizeof(struct TFilter));
      return(1);
      }
    }
  }
return(0);
}


int FilClearByPlugin(struct TPlugin *plugin, char *name)
{
struct TFilter *l;

if (!name)
  return(-1);
for (l = Filter; l; l = l->Next)
  {
  if ((l->Plugin == plugin) && (l->Flags & PLUGIN_FILTER))
    {
    if (!strcmp(l->Name, name))
      {
      FilRemove(l);
      return(1);
      }
    }
  }
return(0);
}


void FilEditingFinish(void)
{
FilCreateIndexList();
(void)FilSetToDriver(1);
mhs_signal_emit(SIGC_PUBLIC, SIG_FILTER_EDIT_FINISH, NULL);
}
