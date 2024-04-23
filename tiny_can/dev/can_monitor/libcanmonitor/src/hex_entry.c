/* *********** Tiny-CAN Monitor **************                            */
/* Copyright (C) 2007 Klaus Demlehner (klaus@mhs-elektronik.de)           */
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
/**************************************************************************/
/*                                                            */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    :                     */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : hex_entry.c                                         */
/* ---------------------------------------------------------------------- */
/*  Datum           : 19.10.07                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include "global.h"
#include <ctype.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dialogs.h"
#include "util.h"
#include "gtk_util.h"
#include "hex_entry.h"

const int EditHexWidth[] = {2, 3, 4, 7, 8};
const int EditBinWidth[] = {8, 12, 16, 28, 32};
const int ValueEntryWidth[] = {4, 5, 6, 9, 10};
const int ValueBinEntryWidth[] = {9, 13, 17, 29, 33};



gboolean HexEntryButtonPressCB(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void HexEntryActivateCB(GtkEntry *widget, gpointer user_data);
gboolean HexEntryFocusOutCB(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);
void HexEntryEditChangedCB(GtkEditable *widget, gpointer user_data);

struct THexEntry *HexEntryNew(GtkWidget *parent, unsigned long value, int value_width, int edit_mode,
    int edit_mask, unsigned int event_flags, THexEntryEventCB event_proc, gpointer user_data)
{
struct THexEntry *he;

he = (struct THexEntry *)g_malloc0(sizeof(struct THexEntry));
if(!he)
  return(NULL);
he->Value = value;
if (value_width > EDIT_SIZE_32_BIT)
  he->ValueWidth = EDIT_SIZE_32_BIT;
else
  he->ValueWidth = value_width;
if (edit_mask)
  he->EditMask = edit_mask;
else
  he->EditMask = EDIT_MASK_HEX;
he->EditMode = edit_mode;
he->EditModeOld = edit_mode;

he->BaseWdg = gtk_alignment_new(0.5, 0.5, 0, 0);
gtk_widget_show(he->BaseWdg);
he->EventFlags = event_flags;
he->EventProc = event_proc;
he->UserData = user_data;
he->ChangeValue = 0;
if (parent)
  gtk_box_pack_start(GTK_BOX(parent), he->BaseWdg, FALSE, FALSE, 0);

he->Frame = gtk_frame_new(NULL);
gtk_widget_show(he->Frame);
gtk_container_add(GTK_CONTAINER(he->BaseWdg), he->Frame);

he->Hbox = gtk_hbox_new(FALSE, 2);
gtk_widget_show (he->Hbox);
gtk_container_add(GTK_CONTAINER(he->Frame), he->Hbox);

he->EventBox = gtk_event_box_new ();
gtk_widget_show(he->EventBox);
gtk_box_pack_start(GTK_BOX(he->Hbox), he->EventBox, FALSE, FALSE, 0);
he->Label = gtk_label_new("<span rise=\"-15000\" color=\"blue\"><b>x</b></span>");
gtk_widget_show(he->Label);
gtk_container_add(GTK_CONTAINER(he->EventBox), he->Label);
gtk_label_set_use_markup(GTK_LABEL(he->Label), TRUE);

he->Entry = gtk_entry_new();
gtk_box_pack_start(GTK_BOX(he->Hbox), he->Entry, FALSE, FALSE, 0);
gtk_entry_set_has_frame(GTK_ENTRY(he->Entry), FALSE);
HexEntrySetEntryWidth(he);
gtk_widget_show(he->Entry);

(void)g_signal_connect(G_OBJECT(he->EventBox), "button_press_event",
                 G_CALLBACK(HexEntryButtonPressCB), he);
(void)g_signal_connect(G_OBJECT(he->Entry), "activate",
                 G_CALLBACK(HexEntryActivateCB), he);
(void)g_signal_connect(G_OBJECT(he->Entry), "focus_out_event",
                 G_CALLBACK(HexEntryFocusOutCB), he);
(void)g_signal_connect(G_OBJECT(he->Entry), "changed",
                 G_CALLBACK(HexEntryEditChangedCB), he);
if (parent)
  {
  gtk_widget_realize(he->EventBox);
  gdk_window_set_cursor (he->EventBox->window, gdk_cursor_new(GDK_HAND1));
  }
return(he);
}


void HexEntryDestroy(struct THexEntry *he)
{
if (!he)
  return;
gtk_widget_destroy(he->BaseWdg);
g_free(he);
}


void HexEntrySetEditMode(struct THexEntry *he, unsigned int mode)
{
if (!he)
  return;
if (he->EditMode == mode)
  return;
if (!(he->EditMask & (1L << mode)))
  return;
he->EditMode = mode;
he->EditModeOld = mode;
HexEntrySetEntryWidth(he);
HexEntryReadValueEx(he, 0, 1);
HexEntryRepaintMode(he);
}


void HexEntrySetColor(struct THexEntry *he, GdkColor *text_color, GdkColor *bg_color)
{
if (!he)
  return;
WidgetSetColor(he->Entry, text_color, bg_color);
}


void HexEntrySetValueWidth(struct THexEntry *he, int value_width)
{
if (!he)
  return;
if (value_width > EDIT_SIZE_32_BIT)
  value_width = EDIT_SIZE_32_BIT;
if (he->ValueWidth == value_width)
  return;
he->ValueWidth = value_width;
HexEntrySetEntryWidth(he);
HexEntryReadValueEx(he, 1, 1);
}


unsigned long HexEntryGetValue(struct THexEntry *he)
{
if (!he)
  return(0);
HexEntryReadValueEx(he, 1, 1);
return(he->Value);
}


void HexEntrySetValue(struct THexEntry *he, unsigned long value)
{
if (!he)
  return;
he->Value = value;
HexEntryPrintValue(he);
}


void HexEntrySetEntryWidth(struct THexEntry *he)
{
if (!he)
  return;
if (he->EditMode == EDIT_MODE_BIN)
  gtk_entry_set_width_chars(GTK_ENTRY(he->Entry), ValueBinEntryWidth[he->ValueWidth]);
else
  gtk_entry_set_width_chars(GTK_ENTRY(he->Entry), ValueEntryWidth[he->ValueWidth]);
}


void HexEntryCreateCursor(struct THexEntry *he)
{
if (!he)
  return;
gtk_widget_realize(he->EventBox);
gdk_window_set_cursor (he->EventBox->window, gdk_cursor_new (GDK_HAND1));
}


void HexEntryPrintValue(struct THexEntry *he)
{
int i;
unsigned long value;

if (he->ValueWidth > EDIT_SIZE_32_BIT)
  he->ValueWidth = EDIT_SIZE_32_BIT;
switch (he->EditMode)
  {
  case EDIT_MODE_HEX     : {
                           g_snprintf(he->StrValue, 50, "%0*lX", EditHexWidth[he->ValueWidth], he->Value);
                           break;
                           }
  case EDIT_MODE_DEZIMAL : {
                           g_snprintf(he->StrValue, 50, "%lu", he->Value);
                           break;
                           }
  case EDIT_MODE_BIN     : {
                           i = EditBinWidth[he->ValueWidth];
                           memset(he->StrValue, '0', i);
                           he->StrValue[i] = 0;
                           value = he->Value;
                           for (; i; i--)
                             {
                             if (value & 0x1L)
                               he->StrValue[i-1] = '1';
                             value >>= 1;
                             }
                           break;
                           }
  case EDIT_MODE_CHAR    : {
                           if (isprint((char)he->Value))
                             g_snprintf(he->StrValue, 50, "%c", (char)he->Value);
                           else
                             he->StrValue[0] = 0;
                           break;
                           }
  }
gtk_entry_set_text(GTK_ENTRY(he->Entry), he->StrValue);
}


void HexEntryReadValueEx(struct THexEntry *he, unsigned int auto_edit_mode, unsigned int reprint)
{
int mode;
char *s;
char buffer[50];
unsigned long value;

if (!he)
  return;
g_strlcpy(buffer, gtk_entry_get_text(GTK_ENTRY(he->Entry)), 50);
s = buffer;
mode = HexEntryDetectMode(&s, he->EditMask);
if (mode < 0)
  mode = he->EditMode;
else
  {
  if (auto_edit_mode)
    {
    he->EditMode = mode;
    if (he->EditModeOld != he->EditMode)
      {
      he->EditModeOld = he->EditMode;
      HexEntrySetEntryWidth(he);
      HexEntryRepaintMode(he);
      }
    }
  }
g_strlcpy(he->StrValue, s, 50);
value = 0;
switch (mode)
  {
  case EDIT_MODE_HEX     : {
                           //value = hex_to_long(&s);
                           value = (unsigned long)strtoul(s, NULL, 16);
                           break;
                           }
  case EDIT_MODE_DEZIMAL : {
                           value = (unsigned long)strtoul(s, NULL, 0);
                           break;
                           }
  case EDIT_MODE_BIN     : {
                           value = (unsigned long)strtoul(s, NULL, 2);
                           break;
                           }
  case EDIT_MODE_CHAR    : {
                           value = (unsigned long)((unsigned char)s[0]);
                           break;
                           }
  }
if (he->Value != value)
  {
  he->Value = value;
  if ((he->EventProc) && (he->EventFlags & EDIT_EVENT_CHANGE_VALUE))
    (he->EventProc)(he, EDIT_EVENT_CHANGE_VALUE, value, he->UserData);
  }
if (reprint)
  HexEntryPrintValue(he);
}


void HexEntryRepaintMode(struct THexEntry *he)
{
switch (he->EditMode)
  {
  case EDIT_MODE_HEX     : {
                           gtk_label_set_markup(GTK_LABEL(he->Label), "<span rise=\"-15000\" color=\"blue\"><b>x</b></span>");
                           break;
                           }
  case EDIT_MODE_DEZIMAL : {
                           gtk_label_set_markup(GTK_LABEL(he->Label), "<span rise=\"-15000\" color=\"blue\"><b>d</b></span>");
                           break;
                           }
  case EDIT_MODE_BIN     : {
                           gtk_label_set_markup(GTK_LABEL(he->Label), "<span rise=\"-15000\" color=\"blue\"><b>b</b></span>");
                           break;
                           }
  case EDIT_MODE_CHAR    : {
                           gtk_label_set_markup(GTK_LABEL(he->Label), "<span rise=\"-15000\" color=\"blue\"><b>c</b></span>");
                           break;
                           }
  }
}


gboolean HexEntryButtonPressCB(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
struct THexEntry *he;
unsigned int mode;
int i;

he = (struct THexEntry *)user_data;
if (!he)
  return(FALSE);
HexEntryReadValueEx(he, 0, 0);
mode = he->EditMode;
for (i = 0; i < 4; i++)
  {
  if (++mode > 3)
    mode = 0;
  if (he->EditMask & (0x1L << mode))
    break;
  }
he->EditMode = mode;
if (he->EditModeOld != he->EditMode)
  {
  he->EditModeOld = he->EditMode;
  HexEntrySetEntryWidth(he);
  HexEntryRepaintMode(he);
  }
HexEntryPrintValue(he);
return(FALSE);
}


void HexEntryActivateCB(GtkEntry *widget, gpointer user_data)
{
struct THexEntry *he;

he = (struct THexEntry *)user_data;
if (!he)
  return;
HexEntryReadValueEx(he, 1, 1);
if (he->EventProc)
  {
  if (he->EventFlags & EDIT_EVENT_RETURN)
    (he->EventProc)(he, EDIT_EVENT_RETURN, he->Value, he->UserData);
  }
}


gboolean HexEntryFocusOutCB(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
struct THexEntry *he;

he = (struct THexEntry *)user_data;
if (!he)
  return(FALSE);
HexEntryReadValueEx(he, 1, 1);
if (he->EventProc)
  {
  if (he->EventFlags & EDIT_EVENT_FOUCUS_OUT)
    (he->EventProc)(he, EDIT_EVENT_FOUCUS_OUT, he->Value, he->UserData);
  }
return(FALSE);
}


void HexEntryEditChangedCB(GtkEditable *widget, gpointer user_data)
{
struct THexEntry *he;

he = (struct THexEntry *)user_data;
if (!he)
  return;
HexEntryReadValueEx(he, 0, 0);
}


int HexEntryDetectMode(char **str, int edit_mask)
{
char ch;
char *s;
int mode, len;

s = *str;
g_strstrip(s);   // Führende u. abschließende Leerzeichen löschen
mode = -1;
ch = s[0];
if (edit_mask & EDIT_MASK_CHAR)
  {
  if ((ch == '\'') || (ch == '\"') || (ch == '´'))  // Char
    {
    mode = EDIT_MODE_CHAR;
    s++;
    len = strlen(s);
    if (len)
      {
      ch = s[len-1];
      if ((ch == '\'') || (ch == '\"') || (ch == '´'))
        s[len-1] = 0;
      }
    }
  }
if (edit_mask & EDIT_MASK_DEZIMAL)
  {
  if (ch == '.')
    {
    mode = EDIT_MODE_DEZIMAL;
    s++;
    }
  }
if (edit_mask & EDIT_MASK_BIN)
  {
  if (ch == '#')
    {
    mode = EDIT_MODE_BIN;
    s++;
    }
  }
if (edit_mask & EDIT_MASK_HEX)
  {
  if (ch == '0')
    {
    ch = s[1];
    if ((ch == 'x') || (ch == 'X'))
      {
      mode = EDIT_MODE_HEX;
      s += 2;
      }
    }
  else if (ch == '$')
    {
    mode = EDIT_MODE_HEX;
    s++;
    }
  }
*str = s;
return(mode);
}
