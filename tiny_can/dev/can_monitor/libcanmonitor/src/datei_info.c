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
/*  Datei Name      : datei_info.c                                        */
/* ---------------------------------------------------------------------- */
/*  Datum           : 17.10.05                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include "global.h"
#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include "util.h"
#include "datei_info.h"


GtkTextBuffer *DateiInfoBuffer = NULL;
GtkWidget *DateiInfoDlg;
GtkWidget *DateiInfoText;


gchar *GetInfoText(void)
{
GtkTextIter start;
GtkTextIter end;

if (!DateiInfoBuffer)
  return(NULL);
// Start und Stop Inter abfragen
gtk_text_buffer_get_start_iter (DateiInfoBuffer, &start);
gtk_text_buffer_get_end_iter (DateiInfoBuffer, &end);
// Text aus Puffer lesen
return(gtk_text_buffer_get_text (DateiInfoBuffer, &start, &end, FALSE));
}


void SetInfoText(char *str)
{
if (!DateiInfoBuffer)
  return;
gtk_text_buffer_set_text(DateiInfoBuffer, str, -1);
}


void ShowDateiInfo(const gchar *title, const gchar *header, gchar **info_text)
{
GtkWidget *dialog_vbox1;
GtkWidget *widget;
gchar *text;

if (!info_text)
  return;
text = *info_text;

DateiInfoDlg = gtk_dialog_new ();
gtk_window_set_title (GTK_WINDOW (DateiInfoDlg), title);
gtk_window_set_position (GTK_WINDOW (DateiInfoDlg), GTK_WIN_POS_CENTER);
gtk_window_set_type_hint (GTK_WINDOW (DateiInfoDlg), GDK_WINDOW_TYPE_HINT_DIALOG);

dialog_vbox1 = GTK_DIALOG (DateiInfoDlg)->vbox;
gtk_widget_show (dialog_vbox1);

widget = gtk_label_new (header);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (dialog_vbox1), widget, FALSE, FALSE, 0);
gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_scrolled_window_new (NULL, NULL);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (dialog_vbox1), widget, TRUE, TRUE, 0);
gtk_widget_set_size_request (widget, 400, 300);
gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_IN);

DateiInfoText = gtk_text_view_new ();
gtk_widget_show (DateiInfoText);
gtk_container_add (GTK_CONTAINER (widget), DateiInfoText);
gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (DateiInfoText), FALSE);

widget = GTK_DIALOG (DateiInfoDlg)->action_area;
gtk_widget_show (widget);
gtk_button_box_set_layout (GTK_BUTTON_BOX (widget), GTK_BUTTONBOX_END);

widget = gtk_button_new_from_stock ("gtk-close");
gtk_widget_show (widget);
gtk_dialog_add_action_widget (GTK_DIALOG (DateiInfoDlg), widget, GTK_RESPONSE_OK);
GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_DEFAULT);

gtk_widget_show (DateiInfoDlg);

DateiInfoBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(DateiInfoText));
if (text)
  {
  SetInfoText(text);
  g_free(text);
  }

gtk_dialog_run(GTK_DIALOG(DateiInfoDlg));
*info_text = GetInfoText();
gtk_widget_destroy(DateiInfoDlg);
}
