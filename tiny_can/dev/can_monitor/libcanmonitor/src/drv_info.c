/* *********** Tiny-CAN Monitor **************                            */
/* Copyright (C) 2005 Klaus Demlehner (klaus@mhs-elektronik.de)           */
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
/*                          Datei Info Dialog                             */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : "Datei Info" Dialog anzeigen                        */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : drv_info.c                                          */
/* ---------------------------------------------------------------------- */
/*  Datum           : 17.10.05                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include "global.h"
#include <glib.h>
#include <gtk/gtk.h>
#include "util.h"
#include "drv_info.h"



void ShowDrvInfoWin(char *info_string, const char *title, const char *win_title)
{
GtkWidget *InfoWin;
GtkWidget *dialog_vbox;
GtkWidget *dialog_action_area;
GtkWidget *okbutton;
GtkWidget *table;
GtkWidget *header_label;
GtkWidget *bezeichner_label[50];
GtkWidget *value_label[50];

int i, zeilen, match;
char *tmpstr, *str, *key, *val, *label_str;

// ***** Zeilen anzahl ermitteln
tmpstr = g_strdup(info_string);
str = tmpstr;
zeilen = 0;
do
  {
  //match = scan(&str, ":=", puf, 100);
  (void)GetItemAsString(&str, ":=", &match);
  if (match <= 0) 
    break;
  zeilen++;
  }
while (1);
save_free(tmpstr);

// ***** prüfen ob Max Zeilen Anzahl überschritten wurde
if ((!zeilen) || (zeilen >= 50))
  return;

// ***** Dialog erzeugen
InfoWin = gtk_dialog_new();
gtk_container_set_border_width (GTK_CONTAINER (InfoWin), 3);
if (win_title)
  gtk_window_set_title (GTK_WINDOW (InfoWin), win_title);
else
  gtk_window_set_title (GTK_WINDOW (InfoWin), "Info");

gtk_window_set_position (GTK_WINDOW (InfoWin), GTK_WIN_POS_CENTER);
gtk_window_set_resizable (GTK_WINDOW (InfoWin), FALSE);
gtk_window_set_type_hint (GTK_WINDOW (InfoWin), GDK_WINDOW_TYPE_HINT_DIALOG);
// **** V-Box
dialog_vbox = GTK_DIALOG (InfoWin)->vbox;
gtk_widget_show(dialog_vbox);
// **** Header Label erzeugen (Überschrift)
if (title)
  {
  header_label = gtk_label_new (title);
  gtk_widget_show (header_label);
  gtk_box_pack_start (GTK_BOX(dialog_vbox), header_label, FALSE, FALSE, 0);
  gtk_label_set_use_markup (GTK_LABEL(header_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC(header_label), 0, 0.5);
  }
// **** Tabelle erzeugen 
table = gtk_table_new(zeilen, 2, FALSE);
gtk_widget_show(table);
gtk_box_pack_start(GTK_BOX(dialog_vbox), table, TRUE, TRUE, 0);
gtk_table_set_row_spacings(GTK_TABLE (table), 4);
gtk_table_set_col_spacings(GTK_TABLE (table), 3);

tmpstr = g_strdup(info_string);
str = tmpstr;
for (i = 0; i < zeilen; i++)
  {
  // Bezeichner auslesen
  key = GetItemAsString(&str, ":=", &match);
  // Value auslesen
  val = GetItemAsString(&str, ";", &match);
  // **** Bezeichner Label erzeugen
  label_str = g_strdup_printf("<span foreground=\"blue\"><big>%s</big></span>:", key);
  bezeichner_label[i] = gtk_label_new(label_str);
  save_free(label_str);
  gtk_widget_show (bezeichner_label[i]);
  gtk_table_attach(GTK_TABLE(table), bezeichner_label[i], 0, 1, i, i+1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup(GTK_LABEL(bezeichner_label[i]), TRUE);
  gtk_misc_set_alignment(GTK_MISC(bezeichner_label[i]), 1, 0.5);

  // **** Value Label erzeugen
  label_str = g_strdup_printf("<span foreground=\"red\"><big>%s</big></span>", val);
  value_label[i] = gtk_label_new(label_str);
  save_free(label_str);
  gtk_widget_show (value_label[i]);
  gtk_table_attach (GTK_TABLE(table), value_label[i], 1, 2, i, i+1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL(value_label[i]), TRUE);
  gtk_misc_set_alignment (GTK_MISC(value_label[i]), 0, 0.5);
  }

// **** Dialog Aktion Area
dialog_action_area = GTK_DIALOG (InfoWin)->action_area;
gtk_widget_show (dialog_action_area);
gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);
// **** Ok Button
okbutton = gtk_button_new_from_stock ("gtk-ok");
gtk_widget_show (okbutton);
gtk_dialog_add_action_widget (GTK_DIALOG (InfoWin), okbutton, GTK_RESPONSE_OK);
GTK_WIDGET_SET_FLAGS(okbutton, GTK_CAN_DEFAULT);

gtk_widget_show(InfoWin);
gtk_dialog_run(GTK_DIALOG(InfoWin));
gtk_widget_destroy(InfoWin);
save_free(tmpstr);
}
