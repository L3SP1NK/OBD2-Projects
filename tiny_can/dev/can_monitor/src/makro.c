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
#include "can_monitor.h"
#include "hex_entry.h"
#include "dialogs.h"
#include "main_can.h"
#include "support.h"
#include "makro.h"


#define MAX_LINE_SIZE 255


static const gchar FORMAT_EFF_RTR_STR[] = {"EFF/RTR"};
static const gchar FORMAT_EFF_STR[]     = {"EFF    "};
static const gchar FORMAT_STD_RTR_STR[] = {"STD/RTR"};
static const gchar FORMAT_STD_STR[]     = {"STD    "};

struct TMakroHeader
  {
  char ID[8];
  unsigned long Count;
  };


/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
struct TMakro *Makro = NULL;


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
int MakroReadFromTxtFile(char *file_name);
int MakroReadFromBinFile(char *file_name);

int MakCreate(void)
{
Makro = NULL;
return(0);
}


void MakDestroy(void)
{
MakClearListe(MAKRO_ALL);
}


void MakClearListe(unsigned long maske)
{
struct TMakro *makro, *clear;

makro = Makro;
while (makro)
  {
  if (makro->Flags & maske)
    {
    if (makro->Prev)
      makro->Prev->Next = makro->Next;
    if (makro->Next)
      makro->Next->Prev = makro->Prev;
    if (Makro == makro)
      Makro = Makro->Next;
    clear = makro;
    makro = makro->Next;
    g_free(clear);
    }
  else
    makro = makro->Next;
  }
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_MAKRO_REPAINT, NULL);
}


void MakClearPlugins(struct TPlugin *plugin)
{
struct TMakro *makro, *clear;

makro = Makro;
while (makro)
  {
  if (makro->Plugin == plugin)
    {
    if (makro->Prev)
      makro->Prev->Next = makro->Next;
    if (makro->Next)
      makro->Next->Prev = makro->Prev;
    if (Makro == makro)
      Makro = Makro->Next;
    clear = makro;
    makro = makro->Next;
    g_free(clear);
    }
  else
    makro = makro->Next;
  }
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_MAKRO_REPAINT, NULL);
}


void MakRemove(struct TMakro *makro)
{
struct TMakro *tmp;

tmp = Makro;
while (tmp)
  {
  if (tmp != makro)
    tmp = tmp->Next;
  else
    {
    if (tmp->Prev)
      tmp->Prev->Next = tmp->Next;
    if (tmp->Next)
      tmp->Next->Prev = tmp->Prev;
    if (Makro == tmp)
      Makro = Makro->Next;

    g_free(tmp);
    mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_MAKRO_REPAINT, NULL);
    break;
    }
  }
}


struct TMakro *MakCreateNew(void)
{
struct TMakro *new_makro;
struct TMakro *last;

new_makro = (struct TMakro *)g_malloc0(sizeof(struct TMakro));
if (!new_makro)
  return(NULL);
new_makro->Next = NULL;

last = Makro;
if (last)
  {
  while (last->Next)
    last = last->Next;

  last->Next = new_makro;
  new_makro->Prev = last;
  }
else
  {
  new_makro->Prev = NULL;
  Makro = new_makro;
  }
new_makro->Flags = 0L;
new_makro->Plugin = NULL;
new_makro->Name[0] = 0;
memset(&new_makro->CanMsg, 0, sizeof(struct TCanMsg));
new_makro->IntervallTime = 0L;
new_makro->IntervallEnabled = 0;
return(new_makro);
}


int MakAddNew(struct TMakro *makro)
{
unsigned int size;
struct TMakro *new_makro;

if ((new_makro = MakCreateNew()))
  {
  //size = sizeof(struct TMakro) - ((void *)&new_makro->Name[0] - (void *)new_makro);
  size = sizeof(struct TMakro) - ((char *)&new_makro->Name[0] - (char *)new_makro);
  memcpy(&new_makro->Name[0], &makro->Name[0], size);
  return(0);
  }
else
  return(-1);
}


void MakCreateNewEnd(struct TMakro *makro)
{
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_MAKRO_REPAINT, NULL);
}


void MakChangeShowing(struct TMakro *makro)
{
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_MAKRO_REPAINT, NULL);
}


int MakMoveBefore(struct TMakro *makro)
{
struct TMakro *head, *before, *end;

if (!makro)
  return(-1);
if ((before = makro->Prev))
  {
  head = before->Prev;
  end = makro->Next;
  // Next
  if (head)
    head->Next = makro;
  before->Next = end;
  makro->Next = before;
  // Prev
  if (end)
    end->Prev = before;
  before->Prev = makro;
  makro->Prev = head;
  // Start Check
  if (before == Makro)
    Makro = makro;
  mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_MAKRO_REPAINT, NULL);
  return(0);
  }
else
  return(-1);
}


int MakMoveAfter(struct TMakro *makro)
{
struct TMakro *head, *after, *end;

if (!makro)
  return(-1);
if ((after = makro->Next))
  {
  head = makro->Prev;
  end = after->Next;
  // Next
  if (head)
    head->Next = after;
  makro->Next = end;
  after->Next = makro;
  // Prev
  if (end)
    end->Prev = makro;
  makro->Prev = after;
  after->Prev = head;
  // Start Check
  if (makro == Makro)
    Makro = after;
  mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_MAKRO_REPAINT, NULL);
  return(0);
  }
else
  return(-1);
}


struct TMakro *MakGetLast(void)
{
struct TMakro *makro;

makro = Makro;
if (makro)
  {
  while (makro->Next)
    makro = makro->Next;
  }
return(makro);
}


struct TMakro *MakGetFirst(void)
{
struct TMakro *makro;

makro = Makro;
if (makro)
  {
  while (makro->Prev)
    makro = makro->Prev;
  }
return(makro);
}



unsigned int MakGetLength(unsigned long maske)
{
unsigned int len;
struct TMakro *makro;

len = 0;
for (makro = Makro; makro; makro = makro->Next)
  {
  if (makro->Flags & maske)
    len++;
  }
return(len);
}


/**************************************************************/
/*                                 */
/**************************************************************/
void MakSetIntervall(void)
{
struct TMakro *makro;
int i;

i = 1;
for (makro = Makro; makro; makro = makro->Next)
  {
  if ((makro->IntervallEnabled) && (makro->IntervallTime))
    {
    CanTransmit(i, &makro->CanMsg, 1);
    CanTransmitSet(i, 0x8000, makro->IntervallTime * 1000);
    if (++i > CanModul.IntPufferCount)
      break;
    }
  }
for (; i < CanModul.IntPufferCount+1; i++)
  CanTransmitSet(i, 0x8000, 0);
}


/*************************************************************/
/* Makro laden                                               */
/*************************************************************/
int MakroReadFromFile(char *file_name)
{
int res;

if ((res = MakroReadFromTxtFile(file_name)) < 0)
  {
  if (MakroReadFromBinFile(file_name) >= 0)
    res = 0;
  }
if (res < 0)
  {
  MakClearListe(STANDART_MAKRO);
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error loading macro file"));
  }
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_MAKRO_REPAINT, NULL);
mhs_signal_emit(SIGC_PROTECTED, SIG_MAKRO_DLG_REPAINT, NULL);
return(res);
}


int MakroReadFromTxtFile(char *file_name)
{
struct TMakro new_makro;
FILE *datei;
int err, i;
char line[MAX_LINE_SIZE];
char hlp[MAX_LINE_SIZE];
char *str, *item, *endptr;

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
      if (!g_ascii_strcasecmp(str, "MAKROS"))
        {
        err = 0;
        break;
        }
      }
    }
  }

if (!err)
  {
  MakClearListe(STANDART_MAKRO);
  while (fgets(line, MAX_LINE_SIZE-1, datei))
    {
    memset(&new_makro.CanMsg, 0, sizeof(struct TCanMsg));
    if (sscanf(line, "[%[^]]", hlp) == 1)
      break;
    str = line;
    // Spalte 1 : Name
    item = GetItemAsString(&str, "=", NULL);
    if (!item)
      break;
    g_strlcpy(new_makro.Name, item, 40);
    // Spalte 2 : Flags
    new_makro.Flags = (GetItemAsULong(&str, ";", 0, NULL) & 0xFFFFFFFCL) | STANDART_MAKRO;
    // Spalte 3 : Intervall Enabled
    new_makro.IntervallEnabled = (unsigned char)GetItemAsULong(&str, ";", 0, NULL);
    // Spalte 4 : Intervall Time
    new_makro.IntervallTime = GetItemAsULong(&str, ";", 0, NULL);
    // Spalte 5 : FRAME-Format
    item = GetItemAsString(&str, ";", NULL);
    if (!item)
      break;
    if (g_strstr_len(item, -1, "EFF"))
      new_makro.CanMsg.MsgEFF = 1;
    if (g_strstr_len(item, -1, "RTR"))
      new_makro.CanMsg.MsgRTR = 1;
    // Spalte 6 : ID
    new_makro.CanMsg.Id = GetItemAsULong(&str, ";", 0, NULL);
    // Spalte 7 : Dlc
    new_makro.CanMsg.MsgLen = GetItemAsULong(&str, ";", 0, NULL);
    // Spalte 8 : Daten
    item = GetItemAsString(&str, "=", NULL);
    // Daten
    for (i = 0; i < (int)new_makro.CanMsg.MsgLen; i++)
      {
      while (*item == ' ')
        item++;
      //new_makro.CanMsg.MsgData[i] = (unsigned char)hex_to_long(&item);
      new_makro.CanMsg.MsgData[i] = (unsigned char)strtoul(item, &endptr, 16);
      item = endptr;
      }
    MakAddNew(&new_makro);
    }
  }
if (datei)
  fclose(datei);
return(err);
}


int MakroReadFromBinFile(char *file_name)
{
FILE *datei;
int err;
struct TMakroHeader makro_header;
struct TMakro makro_puffer;
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
  if (fread(&makro_header, sizeof(struct TMakroHeader), 1, datei) != 1)
    err = -1;
  }
if (!err)
  {
  MakClearListe(STANDART_MAKRO);
  if (makro_header.Count)
    {
    for (i = 0; i < (int)makro_header.Count; i++)
      {
      if (fread(&makro_puffer, sizeof(struct TMakro), 1, datei) != 1)
        {
        err = -1;
        break;
        }
      makro_puffer.Flags = (makro_puffer.Flags & 0xFFFFFFFCL) | STANDART_MAKRO;
      MakAddNew(&makro_puffer);
      }
    }
  }
if (datei)
  fclose(datei);
return(err);
}


/**************************************************************/
/* Makro speichern                                            */
/**************************************************************/
int MakroWriteToFile(char *file_name)
{
int err;
unsigned int i;
struct TMakro *makro;
FILE *datei;
const char *msg_type;
gchar *hex_ptr;
unsigned char hex, ch;
gchar buf[50];

err = 0;
datei = fopen(file_name, "wt");
if (!datei)
  err = -1;
else
  {
  fprintf(datei,"[MAKROS]\n");
  hex_ptr = NULL;
  for (makro = Makro; makro; makro = makro->Next)
    {
    if (makro->Flags & STANDART_MAKRO)
      {
      // **** Message Format
      if ((makro->CanMsg.MsgRTR) && (makro->CanMsg.MsgEFF))
        msg_type = FORMAT_EFF_RTR_STR;
      else if (makro->CanMsg.MsgEFF)
        msg_type = FORMAT_EFF_STR;
      else if (makro->CanMsg.MsgRTR)
        msg_type = FORMAT_STD_RTR_STR;
      else
        msg_type = FORMAT_STD_STR;
      // **** Message-Format, Id und Dlc schreiben
      fprintf(datei, "´%s´ = 0x%08lX; %01u; %6lu; %s; 0x%08lX; %01u; ", makro->Name, makro->Flags,
          makro->IntervallEnabled, makro->IntervallTime, msg_type, makro->CanMsg.Id, makro->CanMsg.MsgLen);
      // **** Daten schreiben
      if (!makro->CanMsg.MsgRTR)
        {
        hex_ptr = buf;
        for (i = 0; i < makro->CanMsg.MsgLen; i++)
          {
          ch = makro->CanMsg.MsgData[i];
          hex = ch >> 4;
          if (hex > 9)
            *hex_ptr++ = 55 + hex;
          else
            *hex_ptr++ = '0' + hex;
          hex = ch & 0x0F;
          if (hex > 9)
            *hex_ptr++ = 55 + hex;
          else
            *hex_ptr++ = '0' + hex;
          *hex_ptr++ = ' ';
          }
        }
      *hex_ptr = '\0';
      fprintf(datei, "%s\n", buf);
      }
    }
  }
if (datei)
  fclose(datei);
if (err)
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error saving macro description file"));
return(err);
}


/*
int MakroWriteToFile(char *file_name)
{
FILE *datei;
int error;
struct TMakroHeader makro_header;
struct TMakro *makro;

makro_header.Count = MakGetLength(STANDART_MAKRO);
error = 0;
datei = fopen(file_name, "wb");
if (!datei)
  error = -1;
if (!error)
  {
  if (fwrite(&makro_header, sizeof(struct TMakroHeader), 1, datei) != 1)
    error = -1;
  }
if (!error)
  {
  for (makro = Makro; makro; makro = makro->Next)
    {
    if (makro->Flags & STANDART_MAKRO)
      {
      if (fwrite(makro, sizeof(struct TMakro), 1, datei) != 1)
        {
        error = -1;
        break;
        }
      }
    }
  }
fclose(datei);
if (error)
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Makro Datei kann nicht gespeichert werden"));
return(error);
} */



int MakSetByPlugin(struct TPlugin *plugin, char *name, struct TCanMsg *msg, unsigned long intervall)
{
struct TMakro *l;
int res;

res = 0;
if ((!plugin) || (!name) || (!msg))
  return(-1);
for (l = Makro; l; l = l->Next)
  {
  if ((l->Plugin == plugin) && (l->Flags & PLUGIN_MAKRO))
    {
    if (!strcmp(l->Name, name))
      {
      res = 1;
      break;
      }
    }
  }
if (!res)
  {
  if ((l = MakCreateNew()))
    strncpy(l->Name, name, 50);
  else
    return(-1);
  }
memcpy(&l->CanMsg, msg, sizeof(struct TCanMsg));
l->Flags |= PLUGIN_MAKRO;
l->Plugin = plugin;
if (intervall)
  {
  l->IntervallTime = intervall;
  l->IntervallEnabled = 1;
  }
else
  {
  l->IntervallTime = 0;
  l->IntervallEnabled = 0;
  }
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_MAKRO_REPAINT, NULL);
return(res);
}


int MakGetByPlugin(struct TPlugin *plugin, char *name, struct TCanMsg *msg, unsigned long *intervall)
{
struct TMakro *l;

if ((!name) || (!msg) || (!intervall))
  return(-1);
for (l = Makro; l; l = l->Next)
  {
  if ((l->Plugin == plugin) && (l->Flags & PLUGIN_MAKRO))
    {
    if (!strcmp(l->Name,name))
      {
      if (l->IntervallEnabled)
        *intervall = l->IntervallTime;
      else
        *intervall = 0L;
      memcpy(msg, &l->CanMsg, sizeof(struct TCanMsg));
      return(1);
      }
    }
  }
return(0);
}


int MakClearByPlugin(struct TPlugin *plugin, char* name)
{
struct TMakro *l;

if (!name)
  return(-1);
for (l = Makro; l; l = l->Next)
  {
  if ((l->Plugin == plugin) && (l->Flags & PLUGIN_MAKRO))
    {
    if (!strcmp(l->Name, name))
      {
      MakRemove(l);
      return(1);
      }
    }
  }
return(0);
}


int MakSendByPlugin(struct TPlugin *plugin, char *name, unsigned long index)
{
struct TMakro *l;

if (!name)
  return(-1);
for (l = Makro; l; l = l->Next)
  {
  if ((l->Plugin == plugin) && (l->Flags & PLUGIN_MAKRO))
    {
    if (!strcmp(l->Name, name))
      {
      if (CanTransmit(index, &l->CanMsg, 1) < 0)
        return(-1);
      else
        return(1);
      }
    }
  }
return(0);
}

