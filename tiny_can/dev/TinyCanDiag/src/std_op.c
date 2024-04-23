/***************************************************************************
                           std_op.c  -  description
                             -------------------
    begin             : 13.07.2017
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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "gtk_util.h"
#include "gui_help.h"
#include "std_op.h"

struct TStdOpWin
  {
  GtkWidget *Dlg;
  GtkWidget *NameEntry;
  };


static struct TStdOpWin *StdOpDlg = NULL;


gint StdOpWin_delete_event(GtkWidget *widget, GdkEvent event, gpointer daten)
{
(void)widget;
(void)event;
(void)daten;

return(TRUE);
}


/**************************************************************/
/* Fenster erzeugen                                           */
/**************************************************************/
struct TStdOpWin *StdOpDlgCreate(void)
{
struct TStdOpWin *std_op_win;
GtkWidget *widget;
GtkWidget *dialog_vbox;
GtkWidget *table;
GtkWidget *dialog_action_area;

std_op_win = (struct TStdOpWin *)g_malloc0(sizeof(struct TStdOpWin));
if (!std_op_win)
  return(NULL);

widget = gtk_dialog_new ();
gtk_window_set_title(GTK_WINDOW(widget), "?");
gtk_window_set_position(GTK_WINDOW (widget), GTK_WIN_POS_MOUSE);
gtk_window_set_modal(GTK_WINDOW (widget), TRUE);
gtk_window_set_resizable(GTK_WINDOW (widget), FALSE);
gtk_window_set_type_hint(GTK_WINDOW (widget), GDK_WINDOW_TYPE_HINT_DIALOG);
std_op_win->Dlg = widget;

dialog_vbox = GTK_DIALOG(widget)->vbox;
gtk_widget_show(dialog_vbox);
// Tabelle anlegen
table = gtk_table_new(1, 2, FALSE);
gtk_widget_show(table);
gtk_box_pack_start(GTK_BOX(dialog_vbox), table, FALSE, FALSE, 3);
gtk_table_set_row_spacings(GTK_TABLE(table), 3);
// Label Name erzeugen
widget = gtk_label_new ("Name: ");
gtk_widget_show(widget);
gtk_table_attach(GTK_TABLE(table), widget, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_RIGHT);
gtk_misc_set_alignment(GTK_MISC(widget), 1, 0.5);
// Name Entry
widget = gtk_entry_new ();
gtk_widget_show(widget);
gtk_table_attach(GTK_TABLE(table), widget, 1, 2, 0, 1,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (0), 0, 0);
gtk_entry_set_max_length(GTK_ENTRY(widget), 20);
gtk_entry_set_invisible_char(GTK_ENTRY(widget), 9679);
gtk_entry_set_width_chars(GTK_ENTRY(widget), 20);
std_op_win->NameEntry = widget;

dialog_action_area = GTK_DIALOG(std_op_win->Dlg)->action_area;
gtk_widget_show (dialog_action_area);
gtk_button_box_set_layout(GTK_BUTTON_BOX(dialog_action_area), GTK_BUTTONBOX_END);

// Chancel Button
widget = gtk_button_new_from_stock("gtk-cancel");
gtk_widget_show(widget);
gtk_dialog_add_action_widget(GTK_DIALOG(std_op_win->Dlg), widget, GTK_RESPONSE_CANCEL);
GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_DEFAULT);
// Ok Button
widget = gtk_button_new_from_stock("gtk-ok");
gtk_widget_show(widget);
gtk_dialog_add_action_widget(GTK_DIALOG(std_op_win->Dlg), widget, GTK_RESPONSE_OK);
GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_DEFAULT);

(void)g_signal_connect(std_op_win->Dlg, "delete_event", G_CALLBACK(StdOpWin_delete_event), NULL);

return(std_op_win);
}


/**************************************************************/
/* Fenster löschen                                            */
/**************************************************************/
/*void StdOpDlgDestroy(struct TStdOpWin **std_op_win) <*>
{
struct TStdOpWin *win_hlp;

if (std_op_win)
  return;
win_hlp = *std_op_win;
if (win_hlp)
  {
  if (win_hlp->Dlg)
    gtk_widget_destroy(win_hlp->Dlg);
  g_free(win_hlp);
  *std_op_win = NULL;
  }
} */


/**************************************************************/
/* Fenster anzeigen                                           */
/**************************************************************/
void StdOpDlgExecute(struct TCanHw *hw, int pin_idx)
{
struct TIoPin *io_pin;
unsigned int pin_nr;
const gchar *name, *pin_type_str;
gchar str[40];

if (!StdOpDlg)
  StdOpDlg = StdOpDlgCreate();
else
  gtk_widget_show_all(StdOpDlg->Dlg);
if (!StdOpDlg)
  g_error("Fenster kann nicht erzeugt werden");
io_pin = &hw->IoPin[pin_idx];
name = io_pin->Name;
pin_nr = hw->IoPinDefs[pin_idx].Pin;
if (io_pin->Mode == 0)
  pin_type_str = "Input";
else if (io_pin->Mode == 1)
  pin_type_str = "Output";
else if (io_pin->Mode == 2)
  pin_type_str = "Analog Output";
else
  pin_type_str = "Analog Input";

g_snprintf(str, 40,"Optionen Pin: %u - %s", pin_nr, pin_type_str);
gtk_window_set_title(GTK_WINDOW(StdOpDlg->Dlg), str);

if (name)
  gtk_entry_set_text(GTK_ENTRY(StdOpDlg->NameEntry), name);
if (gtk_dialog_run(GTK_DIALOG(StdOpDlg->Dlg)) == GTK_RESPONSE_OK)
  {
  name = gtk_entry_get_text(GTK_ENTRY(StdOpDlg->NameEntry));
  if (strlen(name))
    {
    safe_free(io_pin->Name);
    io_pin->Name = g_strdup(name);
    io_pin->DefaultName = 0;
    }
  else
    IoPinSetDefaultName(hw, pin_idx);
  }
gtk_widget_hide_all(StdOpDlg->Dlg);
}
