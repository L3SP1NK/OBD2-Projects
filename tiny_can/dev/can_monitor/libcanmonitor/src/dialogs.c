/***************************************************************************
                          dialogs.c  -  description
                             -------------------
    begin             : 08.04.2008
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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "dialogs.h"


int msg_box(guint flags, const gchar *title, const gchar *message, ...)
{
GtkWidget *dialog;
GtkLabel *label;
GtkHBox *dialog_hbox;
GtkImage *dialog_icon;
gint result;
gint msg_type;
const gchar *stock_id;
gchar *str;
va_list argptr;

switch (flags & 0x03)
  {
  case MSG_TYPE_MESSAGE  : {
                           msg_type = GTK_MESSAGE_INFO;
                           stock_id = GTK_STOCK_DIALOG_INFO;
                           break;
                           }
  case MSG_TYPE_WARNING  : {
                           msg_type = GTK_MESSAGE_WARNING;
                           stock_id = GTK_STOCK_DIALOG_WARNING;
                           break;
                           }
  case MSG_TYPE_FRAGE    : {
                           msg_type = GTK_MESSAGE_QUESTION;
                           stock_id = GTK_STOCK_DIALOG_QUESTION;
                           break;
                           }
  case MSG_TYPE_ERROR    : {
                           msg_type = GTK_MESSAGE_ERROR;
                           stock_id = GTK_STOCK_DIALOG_ERROR;
                           break;
                           }
  default                : {
                           msg_type = GTK_MESSAGE_INFO;
                           stock_id = GTK_STOCK_DIALOG_INFO;
                           }
  }
flags &= 0xFC;
if (flags == DLG_YES_NO_BUTTONS)
  {
  dialog = gtk_dialog_new_with_buttons(title, GTK_WINDOW(NULL),
    (GtkDialogFlags)(GTK_DIALOG_MODAL),
    GTK_STOCK_YES, GTK_RESPONSE_YES,
    GTK_STOCK_NO, GTK_RESPONSE_NO, NULL);
  }
else
  {
  dialog = gtk_dialog_new_with_buttons(title, GTK_WINDOW(NULL),
    (GtkDialogFlags)(GTK_DIALOG_MODAL),
    GTK_STOCK_OK, GTK_RESPONSE_NO, NULL);
  }

gtk_dialog_set_default_response(GTK_DIALOG(dialog),
  GTK_RESPONSE_OK);

dialog_hbox = g_object_new(GTK_TYPE_HBOX, "border-width", 8, "spacing", 10, NULL);

dialog_icon = g_object_new(GTK_TYPE_IMAGE, "stock", stock_id,
                                           "icon-size", GTK_ICON_SIZE_DIALOG,
                                           "xalign", 0.5,
                                           "yalign", 1.0, NULL);
gtk_box_pack_start(GTK_BOX(dialog_hbox), GTK_WIDGET(dialog_icon), FALSE, FALSE, 0);

va_start(argptr, message);
str = g_strdup_vprintf(message, argptr);
va_end(argptr);
label = g_object_new(GTK_TYPE_LABEL,
                     "wrap", TRUE,
                     "use-markup", TRUE,
                     "label", str, NULL);
g_free(str);
gtk_box_pack_start(GTK_BOX(dialog_hbox), GTK_WIDGET(label), FALSE, FALSE, 0);

gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), GTK_WIDGET(dialog_hbox), FALSE, FALSE, 0);
//gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), GTK_WIDGET(dialog_hbox));
gtk_widget_show_all(dialog);
result = gtk_dialog_run(GTK_DIALOG(dialog));
if ((result == GTK_RESPONSE_OK) || (result == GTK_RESPONSE_YES))
  result = 1;
else
  result = 0;
gtk_widget_destroy(dialog);
return(result);
}
