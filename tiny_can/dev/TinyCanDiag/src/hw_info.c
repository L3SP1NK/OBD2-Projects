/***************************************************************************
                          hw_info.c  -  description
                             -------------------
    begin             : 23.08.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include <string.h>
#include <ctype.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "gtk_util.h"
#include "hw_info.h"


void ShowInfoWin(char *info_string, const char *title, const char *win_title)
{
GtkWidget *widget, *win, *dialog_vbox, *dialog_action_area, *table;
gint i, lines, match;
gchar *puf, *puf_val, *str, *s, *label_str;

// ***** Zeilen anzahl ermitteln
str = g_strdup(info_string);
s = str;
lines = 0;
do
  {
  (void)get_item_as_string(&s, ":=", &match);
  if (match <= 0)
    break;
  lines++;
  }
while (1);
safe_free(str);

// ***** prüfen ob Max lines Anzahl überschritten wurde
if ((!lines) || (lines >= 50))
  return;

// ***** Dialog erzeugen
win = gtk_dialog_new();
gtk_container_set_border_width(GTK_CONTAINER(win), 3);
if (win_title)
  gtk_window_set_title(GTK_WINDOW(win), win_title);
else
  gtk_window_set_title(GTK_WINDOW(win), "Info");
gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
gtk_window_set_type_hint(GTK_WINDOW(win), GDK_WINDOW_TYPE_HINT_DIALOG);
// **** V-Box
#ifdef GTK3
dialog_vbox = gtk_dialog_get_content_area(GTK_DIALOG(win));
#else
dialog_vbox = GTK_DIALOG(win)->vbox; // GTK2
#endif

// **** Header Label erzeugen (Überschrift)
if (title)
  {
  widget = gtk_label_new (title);
  gtk_box_pack_start(GTK_BOX(dialog_vbox), widget, FALSE, FALSE, 0);
  gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
  gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);
  }
// **** Tabelle erzeugen
table = gtk_table_new(lines, 2, FALSE);
gtk_box_pack_start(GTK_BOX(dialog_vbox), table, TRUE, TRUE, 0);
gtk_table_set_row_spacings(GTK_TABLE (table), 4);
gtk_table_set_col_spacings(GTK_TABLE (table), 3);

str = g_strdup(info_string);
s = str;
for (i = 0; i < lines; i++)
  {
  // Bezeichner auslesen
  puf = get_item_as_string(&s, ":=", NULL);
  // Value auslesen
  puf_val = get_item_as_string(&s, ";", NULL);

  // **** Bezeichner Label erzeugen
  label_str = g_strdup_printf("<span foreground=\"blue\"><big>%s</big></span>:", puf);
  widget = gtk_label_new(label_str);

  gtk_table_attach(GTK_TABLE(table), widget, 0, 1, i, i+1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
  gtk_misc_set_alignment(GTK_MISC(widget), 1, 0.5);

  // **** Value Label erzeugen
  label_str = g_strdup_printf("<span foreground=\"red\"><big>%s</big></span>", puf_val);
  widget = gtk_label_new(label_str);
  gtk_table_attach (GTK_TABLE(table), widget, 1, 2, i, i+1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
  gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);
  }
safe_free(str);
// **** Dialog Aktion Area
#ifdef GTK3
dialog_action_area =  gtk_dialog_get_action_area(GTK_DIALOG(win));;
#else
dialog_action_area = GTK_DIALOG(win)->action_area; // GTK2
#endif
gtk_button_box_set_layout(GTK_BUTTON_BOX(dialog_action_area), GTK_BUTTONBOX_END);
// **** Ok Button
widget = gtk_button_new_from_stock ("gtk-ok");
gtk_dialog_add_action_widget (GTK_DIALOG (win), widget, GTK_RESPONSE_OK);
//GTK_WIDGET_SET_FLAGS(okbutton, GTK_CAN_DEFAULT); <*> gibt es in GTK3 nicht

gtk_widget_show_all(win);
gtk_dialog_run(GTK_DIALOG(win));
gtk_widget_destroy(win);
}
