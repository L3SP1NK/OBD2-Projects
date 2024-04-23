/***************************************************************************
                           about.c  -  description
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
#include <glib.h>
#include <gtk/gtk.h>
#include "support.h"
#include "util.h"
#include "gtk_util.h"
#include "about_dlg.h"


GtkWidget *AboutPageNew(gchar *file_path, const struct TAboutTxt *about_txt)
{
GtkWidget *vbox;
GtkWidget *vbox1;
GtkWidget *hbox;
GtkWidget *widget;
gchar *filename;
gchar *message;

vbox = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox);
vbox1 = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox1);

if (about_txt->LogoFile)
  {
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  filename = CreateFileName(file_path, about_txt->LogoFile);
  widget = gtk_image_new_from_file(filename);
  g_free(filename);
  gtk_widget_show(widget);
  gtk_misc_set_alignment(GTK_MISC(widget), 1.0, 0.0);
  gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
  gtk_misc_set_padding(GTK_MISC (widget), 5, 5);

  gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
  }
else
  gtk_box_pack_start(GTK_BOX(vbox), vbox1, FALSE, FALSE, 0);

message = g_strdup_printf("<span weight=\"bold\" underline=\"single\" size=\"xx-large\">%s</span>\n%s",
    about_txt->Programm, about_txt->Version);
widget = gtk_label_new (message);
g_free(message);
gtk_widget_show(widget);
gtk_box_pack_start(GTK_BOX(vbox1), widget, FALSE, FALSE, 0);
gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);
gtk_misc_set_padding(GTK_MISC(widget), 5, 5);

message = g_strdup_printf("<span foreground=\"blue\">%s\nAutor: %s (%s)\nURL: %s</span>",
    about_txt->Copyright, about_txt->Autor, about_txt->Email, about_txt->Homepage);
widget = gtk_label_new (message);
g_free(message);
gtk_widget_show(widget);
gtk_box_pack_start (GTK_BOX(vbox1), widget, FALSE, FALSE, 0);
gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);
gtk_misc_set_padding(GTK_MISC(widget), 5, 5);

widget = gtk_hseparator_new();
gtk_widget_show(widget);
gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);

if (about_txt->ShortLizenz)
  {
  message = g_strdup_printf("<span foreground=\"red\">%s</span>", about_txt->ShortLizenz);
  widget = gtk_label_new(message);
  g_free(message);
  //gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
  gtk_widget_show(widget);
  gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
  gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
  gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);
  gtk_misc_set_padding(GTK_MISC(widget), 5, 5);
  }

return(vbox);
}


void ShowAboutDlg(gchar *file_path, const struct TAboutTxt *about_txt)
{
GtkWidget *AboutDlg;
GtkWidget *main_vb;
GtkWidget *main_nb;
GtkWidget *widget;
GtkWidget *alignment;
GtkWidget *hbox;
GtkWidget *image;
GtkWidget *page;
gchar *filename;

AboutDlg = gtk_dialog_new ();
gtk_window_set_title (GTK_WINDOW (AboutDlg), _("About")); // <-> Info
gtk_window_set_position (GTK_WINDOW (AboutDlg), GTK_WIN_POS_CENTER_ON_PARENT);
gtk_window_set_resizable (GTK_WINDOW (AboutDlg), FALSE);
gtk_window_set_type_hint (GTK_WINDOW (AboutDlg), GDK_WINDOW_TYPE_HINT_DIALOG);

main_vb = GTK_DIALOG (AboutDlg)->vbox;
gtk_widget_show (main_vb);

main_nb = gtk_notebook_new();
gtk_widget_show(main_nb);
gtk_box_pack_start(GTK_BOX(main_vb), main_nb, TRUE, TRUE, 0);

page = AboutPageNew(file_path, about_txt);
widget = gtk_label_new(about_txt->Programm);
gtk_notebook_append_page(GTK_NOTEBOOK(main_nb), page, widget);

if (about_txt->LizenzFile)
  {
  filename = CreateFileName(file_path, about_txt->LizenzFile);
  page = TextPageNewFromFile(filename);
  g_free(filename);
  widget = gtk_label_new(_("Licenses")); // <-> "Lizenz");
  gtk_notebook_append_page(GTK_NOTEBOOK(main_nb), page, widget);
  }

widget = GTK_DIALOG(AboutDlg)->action_area;
gtk_widget_show(widget);
gtk_button_box_set_layout(GTK_BUTTON_BOX(widget), GTK_BUTTONBOX_END);
// **** Schlieﬂen Button
widget = gtk_button_new ();
gtk_widget_show (widget);
gtk_dialog_add_action_widget(GTK_DIALOG(AboutDlg), widget, GTK_RESPONSE_CLOSE);
GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_DEFAULT);

alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
gtk_widget_show(alignment);
gtk_container_add(GTK_CONTAINER(widget), alignment);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add(GTK_CONTAINER(alignment), hbox);

image = gtk_image_new_from_stock("gtk-quit", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (image);
gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic(_("Close")); // <-> "Schlie\303\237en");
gtk_widget_show (widget);
gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);

/* g_signal_connect ((gpointer) AboutDlg, "destroy", G_CALLBACK(gtk_main_quit),
                    NULL); */
gtk_dialog_run(GTK_DIALOG(AboutDlg));
gtk_widget_destroy(AboutDlg);
}
