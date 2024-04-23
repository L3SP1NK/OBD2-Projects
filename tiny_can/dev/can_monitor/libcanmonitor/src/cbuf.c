/*******************************************************************************
                            cpuf.c  -  description
                             -------------------
    begin             : 23.03.2008
    copyright         : (C) 2008 - 2009 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "support.h"
#include "can_types.h"
#include "util.h"
#include "mhs_event.h"
#include "cbuf.h"


const char FORMAT_EFF_RTR_STR[] = {"EFF/RTR"};
const char FORMAT_EFF_STR[]     = {"EFF    "};
const char FORMAT_STD_RTR_STR[] = {"STD/RTR"};
const char FORMAT_STD_STR[]     = {"STD    "};
const char FORMAT_INVALID[]     = {"-"};

const char DIR_RX_STR[]         = {"RX"};
const char DIR_TX_STR[]         = {"TX"};


#define MAX_LINE_SIZE 255

static gint CBufferMhsEventProc(gpointer data);


/*
******************** CBufferCreate ********************
Funktion  : Einen CAN Daten Puffer anlegen

Eingaben  : buffer_size => Puffergröße in CAN Nachrichten

Ausgaben  : result => Neu angelegter Puffer

Call's    : g_malloc0, save_free
*/
struct TCanBuffer *CBufferCreate(unsigned int buffer_size)
{
struct TCanBuffer *cbuf;

cbuf = (struct TCanBuffer *)g_malloc0(sizeof(struct TCanBuffer));
if (cbuf)
  {
  cbuf->InfoString = NULL;
  cbuf->MhsEventId = mhs_event_add((GtkFunction)CBufferMhsEventProc, (gpointer)cbuf);
  if (buffer_size)
    {
    cbuf->Data = g_malloc0(sizeof(struct TCanMsg) * buffer_size);
    if (!cbuf->Data)
      save_free(cbuf);
    else
      {
      cbuf->BufferSize = buffer_size;
      cbuf->UsedSize = 0L;
      }
    }
  }
return(cbuf);
}


/*
******************** CBufferDestroy ********************
Funktion  : Einen CAN Daten Puffer löschen

Eingaben  : keine

Ausgaben  : keine

Call's    : save_free
*/
void CBufferDestroy(struct TCanBuffer **cbuf)
{
struct TCanBuffer *buffer;

if (!cbuf)
  return;
buffer = *cbuf;
if (buffer)
  {
  g_source_remove(buffer->MhsEventId);
  save_free(buffer->InfoString);
  save_free(buffer->Data);
  g_free(buffer);
  }
*cbuf = NULL;
}


/*
******************** CBufferDataAdd ********************
Funktion  : Daten in den Puffer schreiben

Eingaben  : cbuf => CAN Daten Puffer
            msgs => Zu schreibende CAN Nachrichten
            count => Anzahl CAN Nachrichten

Ausgaben  : result => -1 = Fehler
                       0 = keine Nachrichten geschrieben, Puffer voll
                      >0 = Anzahl geschriebener Nachrichten

Call's    : memcpy
*/
int CBufferDataAdd(struct TCanBuffer *cbuf, struct TCanMsg *msgs, unsigned int count)
{
if (!cbuf)
  return(-1);
if ((!msgs) || (!count))
  return(0);
if (!cbuf->Data)
  return(-1);
if ((cbuf->UsedSize + count) > cbuf->BufferSize)
  count = cbuf->BufferSize - cbuf->UsedSize;
if (count)
  {
  memcpy(&cbuf->Data[cbuf->UsedSize], msgs, sizeof(struct TCanMsg) * count);
  cbuf->UsedSize += count;
  }
cbuf->LastEvent = CBUF_EVENT_ADD_DATA;
(void)mhs_set_event(cbuf->MhsEventId);
return(count);
}


/*
******************** CBufferDataClear ********************
Funktion  :

Eingaben  : cbuf => CAN Daten Puffer

Ausgaben  : keine

Call's    : keine
*/
int CBufferDataClear(struct TCanBuffer *cbuf)
{
if (!cbuf)
  return(-1);
if (!cbuf->Data)
  return(-1);
cbuf->UsedSize = 0;
cbuf->LastEvent = CBUF_EVENT_CLEAR;
(void)mhs_set_event(cbuf->MhsEventId);
return(0);
}


/*
******************** CBufferGetUsedSize ********************
Funktion  : Anzahl CAN Nachrichten im Puffer abfragen

Eingaben  : cbuf => CAN Daten Puffer

Ausgaben  : result => Anzahl CAN Nachrichten

Call's    : keine
*/
int CBufferGetUsedSize(struct TCanBuffer *cbuf)
{
if (!cbuf)
  return(-1);
if (!cbuf->Data)
  return(-1);
return((int)cbuf->UsedSize);
}


/*
******************** CBufferGetSize ********************
Funktion  : Maximale Puffergröße abfragen

Eingaben  : cbuf => CAN Daten Puffer

Ausgaben  : result => Maximale Puffergröße

Call's    : keine
*/
int CBufferGetSize(struct TCanBuffer *cbuf)
{
if (!cbuf)
  return(-1);
if (!cbuf->Data)
  return(-1);
return((int)cbuf->BufferSize);
}


int CBufferSetSize(struct TCanBuffer *cbuf, unsigned int buffer_size)
{
if (!cbuf)
  return(-1);
if (buffer_size != cbuf->BufferSize)
  {
  save_free(cbuf->Data);
  if (buffer_size)
    cbuf->Data = g_malloc0(sizeof(struct TCanMsg) * buffer_size);
  cbuf->BufferSize = buffer_size;
  }
return(0);
}

/*
******************** CBufferGetInfoString ********************
Funktion  : CAN Puffer Info String abfragen

Eingaben  : cbuf => CAN Daten Puffer

Ausgaben  : result => Info String

Call's    : keine
*/
char *CBufferGetInfoString(struct TCanBuffer *cbuf)
{
if (!cbuf)
  return(NULL);
return(cbuf->InfoString);
}


/*
******************** CBufferSetInfoString ********************
Funktion  : CAN Puffer Info String setzen

Eingaben  : cbuf => CAN Daten Puffer
            info_string => Info String

Ausgaben  : keine

Call's    : g_strdup
*/
void CBufferSetInfoString(struct TCanBuffer *cbuf, char *info_string)
{
if (!cbuf)
  return;
cbuf->InfoString = g_strdup(info_string);
}


/*
********************  ********************
Funktion  :

Eingaben  :

Ausgaben  : keine

Call's    : keine
*/
int CBufferEventConnect(struct TCanBuffer *cbuf, TMhsBufferCB proc, gpointer user_data)
{
if ((!cbuf) || (!proc))
  return(-1);
cbuf->UserData = user_data;
cbuf->EventProc = proc;
return(0);
}


/*
********************  ********************
Funktion  :

Eingaben  :

Ausgaben  : keine

Call's    : keine
*/
void CBufferEventDisconnect(struct TCanBuffer *cbuf, TMhsBufferCB proc)
{
if (!cbuf)
  return;
cbuf->EventProc = NULL;
}



static gint CBufferMhsEventProc(gpointer data)
{
struct TCanBuffer *cbuf;

if (!(cbuf = (struct TCanBuffer *)data))
  return(FALSE);
if (cbuf->EventProc)
  (cbuf->EventProc)(cbuf, cbuf->UserData, cbuf->LastEvent);
return(TRUE);
}


/*
******************** CBufferLoadFile ********************
Funktion  : Eine Datei in den Puffer laden

Eingaben  : cbuf => CAN Daten Puffer
            file_name => Dateiname

Ausgaben  : result => 0 = Datei erfolgreich geladen
                      -1 = Fehler beim laden der Datei

Call's    : save_free, fopen, fgets, sscanf, g_strstrip, g_ascii_strcasecmp,
            fgetpos, fsetpos, g_stpcpy, GetItemAsString, GetItemAsULong,
            memcpy, g_malloc0, fclose
*/
int CBufferLoadFile(struct TCanBuffer *cbuf, char *file_name)
{
FILE *datei;
int err, i;
char line[MAX_LINE_SIZE];
char item[MAX_LINE_SIZE];
char *str, *s, *endptr;
struct TCanMsg msg;
unsigned long info_size;
fpos_t file_position;

err = 0;
info_size = 0;
if ((!cbuf) || (!file_name))
  return(-1);
if (CBufferDataClear(cbuf) < 0)
  return(-1);
save_free(cbuf->InfoString);
datei = fopen(file_name, "rt");
if (datei == NULL)
  err = -1;
// Zur Section "HEADER" springen
if (!err)
  {
  err = -2;
  while (fgets(line, MAX_LINE_SIZE-1, datei))
    {
    if (sscanf(line, "[%[^]]", item) == 1)
      {
      g_strstrip(item);
      if (!g_ascii_strcasecmp(item, "HEADER"))
        {
        err = 0;
        break;
        }
      }
    }
  }
// Größe des Info Blocks bestimmen
fgetpos(datei, &file_position);
if (!err)
  {
  info_size = 0;
  err = -2;
  while (fgets(line, MAX_LINE_SIZE-1, datei))
    {
    if (sscanf(line, "[%[^]]", item) == 1)
      {
      g_strstrip(item);
      if (!g_ascii_strcasecmp(item, "DATA"))
        err = 0;
      break;
      }
    else
      info_size += (strlen(line) + 2);
    }
  }
// Info Block einlesen
if ((!err) && (info_size))
  {
  info_size += 1;
  cbuf->InfoString = g_malloc0(info_size);
  if (cbuf->InfoString)
    {
    str = cbuf->InfoString;
    fsetpos(datei, &file_position);
    err = -2;
    while (fgets(line, MAX_LINE_SIZE-3, datei))
      {
      if (sscanf(line, "[%[^]]", item) == 1)
        {
        err = 0;
        break;
        }
      else
        {
        str = g_stpcpy(str, line);
        *str++ = '\n';
        }
      }
    str = '\0';
    }
  }
// CAN Daten lesen
if (!err)
  {
  while (fgets(line, MAX_LINE_SIZE-1, datei))
    {
    memset(&msg, 0, sizeof(struct TCanMsg));
    str = line;
    if (sscanf(str, "[%[^]]", item) == 1)
      break;
    // Spalte 1: Zeilen-Nummer
    if (!(s = GetItemAsString(&str, ";", NULL)))
      break;
    // Spalte 2: Time-Stamp
    if (!(s = GetItemAsString(&str, ";", NULL)))
      break;
    // Spalte 3: FRAME-Format
    if (!(s = GetItemAsString(&str, ";", NULL)))
      break;
    if (g_strrstr(s, "EFF"))
      msg.MsgEFF = 1;
    if (g_strrstr(s, "RTR"))
      msg.MsgRTR = 1;
    // Spalte 4: ID
    /* if (!(s = GetItemAsString(&str, ";", NULL))
      break;
    msg.Id = hex_to_long(&s); */
    msg.Id = GetItemAsULong(&str, ";", 16, NULL);
    // Spalte 5: Dlc
    msg.MsgLen = (unsigned char)GetItemAsULong(&str, ";", 0, NULL);
    // Spalte 2: Daten
    if (!(s = GetItemAsString(&str, ";", NULL)))
      break;
    // Daten
    for (i = 0; i < (int)msg.MsgLen; i++)
      {
      while (*s == ' ')
        s++;
      //msg.MsgData[i] = (unsigned char)hex_to_long(&s);
      msg.MsgData[i] = (unsigned char)strtoul(s, &endptr, 16);
      s = endptr;
      }
    // Message speichern
    if (cbuf->UsedSize < cbuf->BufferSize)
      {
      memcpy(&cbuf->Data[cbuf->UsedSize], &msg, sizeof(struct TCanMsg));
      cbuf->UsedSize++;
      }
    else
      {
      err = -1;
      break;
      }
    }
  }
if (datei)
  fclose(datei);
cbuf->LastEvent = CBUF_EVENT_LOAD;
(void)mhs_set_event(cbuf->MhsEventId);
return(err);
}


/*
******************** CBufferSaveFile ********************
Funktion  : Puffer in eine Datei schreiben

Eingaben  : cbuf => CAN Daten Puffer
            file_name => Dateiname

Ausgaben  : result => 0 = Datei erfolgreich geladen
                      -1 = Fehler beim schreiben der Datei

Call's    : fopen, fprintf, fclose, strlen, diff_timestamp, diff_time,
            g_stpcpy
*/
int CBufferSaveFile(struct TCanBuffer *cbuf, char *file_name)
{
FILE *datei;
unsigned int i;
struct TCanMsg *msg;
unsigned long line;
const char *msg_type;
unsigned long msg_len;
char *ascii_ptr;
char *hex_ptr;
unsigned char hex;
unsigned char ch;
gchar buf1[50];
gchar buf2[50];
struct TTime t_diff, start_time;
guint32 t;

if ((!cbuf) || (!file_name))
  return(-1);
if (!cbuf->Data)
  return(-1);
if (!cbuf->UsedSize)
  return(-2);

datei = fopen(file_name, "wt");
if (!datei)
  return(-2);

// Section "HEADER"
fprintf(datei, "[HEADER]\n");
// Datei Info schreiben
if (cbuf->InfoString)
  {
  if (strlen(cbuf->InfoString) > 0)
    {
    fprintf(datei, "%s", cbuf->InfoString);
    if (cbuf->InfoString[strlen(cbuf->InfoString)-1] != '\n')
      fprintf(datei, "\n");
    }
  }
// Section "DATA"
fprintf(datei,"[DATA]\n");

// Daten schreiben
msg = cbuf->Data;
start_time.Sec = msg->Time.Sec;
start_time.USec = msg->Time.USec;

for (line = 1; line <= cbuf->UsedSize; line++)
  {
  // **** Zeilen-Nummer
  fprintf(datei, "%7lu; ", line);
  // **** Time-Stamp schreiben
  switch (cbuf->TimeFormat)
    {
    case 0 : {
             fprintf(datei, "; ");
             break;
             }
    case 1 : { // System Time (Absolut)
             fprintf(datei, "%10u.%-6u; ", (unsigned int)msg->Time.Sec, (unsigned int)msg->Time.USec);
             break;
             }
    case 2 : { // System Time (Relative)
             diff_timestamp(&t_diff, &msg->Time, &start_time);
             fprintf(datei, "%10u.%-6u; ", (unsigned int)t_diff.Sec, (unsigned int)t_diff.USec);
             break;
             }
    case 3 : { // System Time (Absolut, ms)
             fprintf(datei, "%10u.%03u; ", (unsigned int)msg->Time.Sec, (unsigned int)(msg->Time.USec / 1000));
             break;
             }
    case 4 : { // System Time (Relative, ms)
             diff_timestamp(&t_diff, &msg->Time, &start_time);
             fprintf(datei, "%10u.%03u; ", (unsigned int)t_diff.Sec, (unsigned int)(t_diff.USec / 1000));
             break;
             }
    case 5 : { // HW Time Stamp (Absolute)
             t = msg->Time.USec;
             fprintf(datei, "%9u.%u; ", (t / 10), (t % 10));
             break;
             }
    case 6 : { // HW Time Stamp (Relative)
             t = msg->Time.USec;
             t = diff_time(t, start_time.USec);
             fprintf(datei, "%9u.%u; ", (t / 10), (t % 10));
             break;
             }
    }
  // **** Message Format
  if ((msg->MsgRTR) && (msg->MsgEFF))
    msg_type = FORMAT_EFF_RTR_STR;
  else if (msg->MsgEFF)
    msg_type = FORMAT_EFF_STR;
  else if (msg->MsgRTR)
    msg_type = FORMAT_STD_RTR_STR;
  else
    msg_type = FORMAT_STD_STR;

  if (msg->MsgEFF)
    fprintf(datei, "%s; %08X; %01u; ", msg_type, (unsigned int)msg->Id, msg->MsgLen);
  else
    fprintf(datei, "%s; %08X; %01u; ", msg_type, (unsigned int)msg->Id, msg->MsgLen);

  // **** CAN Daten schreiben
  if (msg->MsgRTR)
    msg_len = 0;
  else
    msg_len = msg->MsgLen;

  hex_ptr = buf1;
  ascii_ptr = buf2;
  for (i = 0; i < msg_len; i++)
    {
    ch = msg->MsgData[i];
    if (ascii_ptr)
      {
      if ((ch <= 32) || (ch >= 126))
        *ascii_ptr++ = '.';
      else
        *ascii_ptr++ = (char)ch;
      }
    if (hex_ptr)
      {
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
  for (; i < 8; i++)
    {
    if (ascii_ptr)
      *ascii_ptr++ = ' ';
    if (hex_ptr)
      hex_ptr = g_stpcpy(hex_ptr, "   ");
    }
  if (ascii_ptr)
    *ascii_ptr = '\0';
  if (hex_ptr)
    *hex_ptr = '\0';

  fprintf(datei, "%s ; %s\n", buf1, buf2);

  msg++;
  }
fprintf(datei, "[END]\n");
fclose(datei);
return(0);
}
