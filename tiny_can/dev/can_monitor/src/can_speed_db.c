/***************************************************************************
                       can_speed_db.c  -  description
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
#include "can_speed_db.h"



void CanSpeedDBDestroy(struct TCanSpeedList **list)
{
struct TCanSpeedList *l, *h;

if (!list)
  return;
l = *list;
while (l)
  {
  h = l->Next;
  save_free(l->Description);
  g_free(l);
  l = h;
  }
*list = NULL;
}


static struct TCanSpeedList *CanSpeedDBAdd(struct TCanSpeedList **list)
{
struct TCanSpeedList *l;

l = *list;
if (!l)
  {
  // Liste ist leer
  l = (struct TCanSpeedList *)g_malloc0(sizeof(struct TCanSpeedList));
  *list = l;
  }
else
  { // Neues Element anhängen
  while (l->Next != NULL) l = l->Next;
  l->Next = (struct TCanSpeedList *)g_malloc0(sizeof(struct TCanSpeedList));
  l = l->Next;
  if (!l)
    return(NULL);        // Nicht genügend Arbetsspeicher
  }
l->Next = NULL;
return(l);
}


char *ExtractItemString(char **instr)
{
int len;
char *str, *start, *end;

str = *instr;
// Führende Leerzeichen löschen
while (*str == ' ')
  str++;
// " Zeichen löschen
if (*str == '"')
  {
  str++;
  start = str;
  while ((*str != '"') && (*str))
    str++;
  end = str;
  if (*str == '"')
    str++;
  while ((*str != '|') && (*str))
    str++;
  }
else
  {
  start = str;
  while ((*str != '|') && (*str))
    str++;
  end = str;
  }
if (*str == '|')
  str++;
*instr = str;

len = end - start;
str = (char *)g_malloc(len + 1);
if (!str)
  return(NULL);
if (len)
  memcpy(str, start, len);
str[len] = 0;
return(str);
}


struct TCanSpeedList *CanSpeedDBOpen(char *file_name)
{
gchar *line, *str, *endptr;
gchar *value, *description, *short_description;
GIOChannel *iochannel;
GError *error;
struct TCanSpeedList *can_speed_list, *item;

error = NULL;
can_speed_list = NULL;
iochannel = NULL;
line = NULL;
iochannel = g_io_channel_new_file(file_name, "r", &error);
if (error)
  return NULL;

while (g_io_channel_read_line(iochannel,&line,NULL,NULL,&error) == G_IO_STATUS_NORMAL)
  {
  str = line;
  // Führende Leerzeichen löschen
  while (*str == ' ')
    str++;
  // Kommentar Zeile / Leerzeile
  if ((*str != ';') && (*str != '#') && (*str != 0))
    {
    description = ExtractItemString(&str);
    short_description = ExtractItemString(&str);
    if (!strlen(short_description))
      {
      save_free(short_description);
      short_description = g_strdup(description);
      }
    value = ExtractItemString(&str);
    if ((strlen(description)) && (strlen(value)))
      {
      item = CanSpeedDBAdd(&can_speed_list);
      item->Value = strtoul(value, &endptr, 0);
      item->ShortDescription = short_description;
      item->Description = description;
      }
    else
      {
      save_free(short_description);
      save_free(description);
      }
    save_free(value);
    }
  g_free(line);
  }
g_io_channel_unref(iochannel);

return(can_speed_list);
}


char *CanSpeedDBGetShortDesciption(struct TCanSpeedList *list, unsigned long value)
{
for (; list; list = list->Next)
  {
  if (list->Value == value)
    break;
  }
if (list)
  return(list->ShortDescription);
else
  return(NULL);
}


