/***************************************************************************
                           start_win.c  -  description
                             -------------------
    begin             : 28.03.2009
    copyright         : (C) 2009 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include "can_monitor.h"
#include "support.h"
#include "dialogs.h"
#include "file_sel.h"
#include "drv_scan.h"
#include "api_find.h"
#include "drv_win.h"
#include "setup.h"
#include "start_win.h"


static const char FirstStartMessage[] = {
  N_("Tiny CAN-Monitor has been run at the first time on your computer.\n"
     "This wizard helps you with the correct configuration.\n"
     "\n"
     "For communication with the hardware you must load a driver,\n"
     "Tiny-CAN Monitor is looking at the following directory for drivers:")};


static const char FinishMessage[] = {
  N_("You have selected a driver successful, the configuration wizard is closing now.\n"
     "\n"
     "You can change the settings at Options Setup Driver\n")};


static GtkWidget *DriverPathLabel = NULL;
static GtkWidget *DriverLabel = NULL;
static char *DriverPath = NULL;
static struct TDriverSetupWin *DriverSetupWin = NULL;

static GtkWidget *Page1 = NULL;
static GtkWidget *Page2 = NULL;
static GtkWidget *StartWin = NULL;


static void SetPath(void)
{
if (DriverPath)
  {
  if (!IsDriverDir(DriverPath))
    {
    msg_box(MSG_TYPE_ERROR, _("Error"), _("No found Tiny-CAN API drivers at the selected directory."));
    save_free(DriverPath);
    }
  }
if (DriverPath)
  {
  gtk_label_set_markup(GTK_LABEL(DriverPathLabel), DriverPath);
  gtk_assistant_set_page_complete(GTK_ASSISTANT(StartWin), Page1, TRUE);
  DriverWinSetPathFile(DriverSetupWin, DriverPath, NULL);
  }
else
  {
  gtk_label_set_markup(GTK_LABEL(DriverPathLabel), _("<span foreground=\"red\">No directory selected!</span>"));
  gtk_assistant_set_page_complete(GTK_ASSISTANT (StartWin), Page1, FALSE);
  }
}


void DriverWinEventCallback(struct TDriverSetupWin *driver_win, int event)
{
char *str;

if (event)
  {
  str = g_strdup_printf(_("%s selected"), driver_win->SelectedName);
  gtk_label_set_markup(GTK_LABEL(DriverLabel), str);
  save_free(str);
  gtk_assistant_set_page_complete(GTK_ASSISTANT(StartWin), Page2, TRUE);
  }
else
  {
  gtk_label_set_markup(GTK_LABEL(DriverLabel), _("<span foreground=\"red\">No driver selected!</span>"));
  gtk_assistant_set_page_complete(GTK_ASSISTANT (StartWin), Page2, FALSE);
  }
}


static void DirChangeCB(GtkButton *button, gpointer user_data)
{
char *file_name;

file_name = NULL;
if (SelectFileDlg(_("Change directory"), FILE_SELECTION_PATH, &file_name) > 0)
  {
  save_free(DriverPath);
  DriverPath = g_strdup(file_name);
  SetPath();
  }
save_free(file_name);
}


static void on_assistant_apply(GtkWidget *widget, gpointer data)
{
if (!DriverSetupWin)
  return;
// **** Treiber
save_free(SetupData.DriverPath);
SetupData.DriverPath = DriverWinGetPath(DriverSetupWin);
save_free(SetupData.DriverFile);
SetupData.DriverFile = DriverWinGetFile(DriverSetupWin);

SetSetup();
}


static void on_assistant_close_cancel (GtkWidget *widget, gpointer data)
{
DriverWinDestroy(DriverSetupWin);
gtk_widget_destroy(StartWin);
save_free(DriverPath);
StartWin = NULL;
}


static void on_assistant_prepare(GtkWidget *widget, GtkWidget *page, gpointer data)
{
gint current_page, n_pages;
gchar *title;

current_page = gtk_assistant_get_current_page(GTK_ASSISTANT(widget));
n_pages = gtk_assistant_get_n_pages (GTK_ASSISTANT (widget));

title = g_strdup_printf (_("Tiny-CAN Start (page %d of %d)"), current_page + 1, n_pages);
gtk_window_set_title (GTK_WINDOW (widget), title);
g_free(title);
}


static GtkWidget *create_page1(GtkWidget *assistant)
{
GtkWidget *box, *label, *button;
GdkPixbuf *pixbuf;

box = gtk_vbox_new(FALSE, 5);
gtk_container_set_border_width (GTK_CONTAINER (box), 12);

label = gtk_label_new(_(FirstStartMessage));
gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

label = gtk_label_new("?");
gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
DriverPathLabel = label;

button = gtk_button_new_with_label(_("Change"));
gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
(void)g_signal_connect(button, "clicked", G_CALLBACK(DirChangeCB), NULL);

gtk_widget_show_all(box);

gtk_assistant_append_page (GTK_ASSISTANT (assistant), box);
//gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), box, GTK_ASSISTANT_PAGE_CONFIRM);
gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), box, GTK_ASSISTANT_PAGE_INTRO);
gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), box, TRUE);
gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), box, _("Welcome"));

pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), box, pixbuf);
g_object_unref (pixbuf);

return(box);
}


static GtkWidget *create_page2(GtkWidget *assistant)
{
GtkWidget *box, *label, *widget;
GdkPixbuf *pixbuf;

box = gtk_vbox_new (FALSE, 5);
gtk_container_set_border_width (GTK_CONTAINER (box), 12);

DriverSetupWin = DriverWinNew(DriverPath, NULL, 0);
SetDriverWinEventCallback(DriverSetupWin, &DriverWinEventCallback);
widget = DriverSetupWin->Base;
gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);

label = gtk_label_new("?");
gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
DriverLabel = label;

gtk_widget_show_all(box);

gtk_assistant_append_page (GTK_ASSISTANT (assistant), box);
gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), box, GTK_ASSISTANT_PAGE_PROGRESS);
gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), box, TRUE);
gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), box, _("Selecting a driver"));

pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_DIALOG, NULL);
gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), box, pixbuf);
g_object_unref (pixbuf);
return(box);
}


static GtkWidget *create_page3(GtkWidget *assistant)
{
GtkWidget *label;
GdkPixbuf *pixbuf;

label = gtk_label_new(_(FinishMessage));

gtk_widget_show (label);
gtk_assistant_append_page (GTK_ASSISTANT (assistant), label);
gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), label, GTK_ASSISTANT_PAGE_CONFIRM);
gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), label, TRUE);
gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), label, _("Finish"));

pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_APPLY, GTK_ICON_SIZE_DIALOG, NULL);
gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), label, pixbuf);
g_object_unref (pixbuf);
return(label);
}


void ShowStartWin(void)
{
// Automatisch nach Tiny-CAN Api Treibern suchen
ApiPathListCreate();
DriverPath = GetApiPath();
ApiPathListDestroy();

StartWin = gtk_assistant_new();

gtk_window_set_default_size(GTK_WINDOW (StartWin), -1, 300);
gtk_window_set_modal(GTK_WINDOW(StartWin), TRUE);
gtk_window_set_position(GTK_WINDOW(StartWin), GTK_WIN_POS_CENTER);

Page1 = create_page1(StartWin);
Page2 = create_page2(StartWin);
create_page3(StartWin);
SetPath();

g_signal_connect(G_OBJECT(StartWin), "cancel", G_CALLBACK(on_assistant_close_cancel), NULL);
g_signal_connect(G_OBJECT(StartWin), "close", G_CALLBACK(on_assistant_close_cancel), NULL);
g_signal_connect(G_OBJECT(StartWin), "apply", G_CALLBACK(on_assistant_apply), NULL);
g_signal_connect(G_OBJECT(StartWin), "prepare", G_CALLBACK(on_assistant_prepare), NULL);

gtk_widget_show(StartWin);
}

