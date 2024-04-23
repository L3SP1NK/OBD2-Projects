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
#include "can_monitor.h"
#include <stdio.h>
#include <string.h>
#include "setup.h"
#include "support.h"
#include "linux_port_setup.h"


GtkWidget *PortSetupDlg;
GtkWidget *DeviceNameEdit;

void ShowPortSetupDlg(void);

/**************************************************************/
/* Port Setup Dialog anzeigen                                 */
/**************************************************************/
int ExecutePortSetup(void)
{
unsigned int i;

ShowPortSetupDlg();
if (SetupData.ComDeviceName)
  gtk_entry_set_text(GTK_ENTRY(DeviceNameEdit), SetupData.ComDeviceName);
// **** Dialog anzeigen
if (gtk_dialog_run(GTK_DIALOG(PortSetupDlg)) == GTK_RESPONSE_OK)
  {
  save_free(SetupData.ComDeviceName);
  SetupData.ComDeviceName = g_strdup(gtk_entry_get_text(GTK_ENTRY(DeviceNameEdit)));
  i=1;
  }
else
  i=0;;

gtk_widget_destroy(PortSetupDlg);
return(i);
}


void ShowPortSetupDlg(void)
{  
GtkWidget *dialog_vbox;
GtkWidget *widget;
GtkWidget *hbox;
GtkWidget *cancelbutton1;
GtkWidget *okbutton1;

PortSetupDlg = gtk_dialog_new ();
gtk_window_set_title (GTK_WINDOW (PortSetupDlg), _("Select interface port"));
gtk_window_set_position (GTK_WINDOW (PortSetupDlg), GTK_WIN_POS_CENTER);
gtk_window_set_resizable (GTK_WINDOW (PortSetupDlg), FALSE);
gtk_window_set_type_hint (GTK_WINDOW (PortSetupDlg), GDK_WINDOW_TYPE_HINT_DIALOG);
gtk_dialog_set_has_separator (GTK_DIALOG (PortSetupDlg), FALSE);

dialog_vbox = GTK_DIALOG(PortSetupDlg)->vbox;
gtk_widget_show(dialog_vbox);

hbox = gtk_hbox_new (FALSE, 0);
gtk_widget_show(hbox);
gtk_box_pack_start(GTK_BOX(dialog_vbox), hbox, TRUE, TRUE, 0);

widget = gtk_label_new(_("Device name : "));
gtk_widget_show(widget);
gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);

DeviceNameEdit = gtk_entry_new();
gtk_widget_show (DeviceNameEdit);
gtk_box_pack_start(GTK_BOX(hbox), DeviceNameEdit, TRUE, TRUE, 0);

widget = GTK_DIALOG(PortSetupDlg)->action_area;
gtk_widget_show(widget);
gtk_button_box_set_layout(GTK_BUTTON_BOX (widget), GTK_BUTTONBOX_END);

cancelbutton1 = gtk_button_new();
gtk_widget_show(cancelbutton1);
gtk_dialog_add_action_widget(GTK_DIALOG (PortSetupDlg), cancelbutton1, GTK_RESPONSE_CANCEL);
GTK_WIDGET_SET_FLAGS(cancelbutton1, GTK_CAN_DEFAULT);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show(widget);
gtk_container_add(GTK_CONTAINER(cancelbutton1), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add(GTK_CONTAINER(widget), hbox);

widget = gtk_image_new_from_stock ("gtk-cancel", GTK_ICON_SIZE_BUTTON);
gtk_widget_show(widget);
gtk_box_pack_start(GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic(_("_Cancel"));
gtk_widget_show(widget);
gtk_box_pack_start(GTK_BOX (hbox), widget, FALSE, FALSE, 0);

okbutton1 = gtk_button_new();
gtk_widget_show(okbutton1);
gtk_dialog_add_action_widget(GTK_DIALOG(PortSetupDlg), okbutton1, GTK_RESPONSE_OK);
GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

widget = gtk_alignment_new(0.5, 0.5, 0, 0);
gtk_widget_show(widget);
gtk_container_add(GTK_CONTAINER(okbutton1), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show(hbox);
gtk_container_add(GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock("gtk-ok", GTK_ICON_SIZE_BUTTON);
gtk_widget_show(widget);
gtk_box_pack_start(GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic(_("_Ok"));
gtk_widget_show(widget);
gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);

gtk_widget_show(PortSetupDlg);
}
