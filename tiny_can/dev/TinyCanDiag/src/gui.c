/***************************************************************************
                            gui.c  -  description
                             -------------------
    begin             : 24.08.2017
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
#include "main.h"
#include "dialogs.h"
#include "giwled.h"
#include "paths.h"
#include "gtk_util.h"
#include "abaut_dlg.h"
#include "std_op.h"
#include "project.h"
#include "file_sel.h"
#include "io.h"
#include "hw_info.h"
#include "tcan_mx_gui.h"
#include "main_welcome.h"
#include "gui_help.h"
#include "candiag_icons.h"
#include "gui.h"


static const gchar PROG_OFFLINE_MSG[]    = {"Target Offline"};
static const gchar PROG_ERROR_MSG[]      = {"Tiny-CAN kann nicht geöffnet werden"};
static const gchar PORG_GO_ONLINE_MSG[]  = {"Verbindung wird aufgebaut ..."};
static const gchar PROG_RUN_MSG[]       = {"Tiny-CAN Online"};

static const gchar INFO_WIN_TITLE[] = {"Info"};
static const gchar INFO_TITLE_HARDWARE[] = {"<b><big>Tiny-CAN Info Variablen</big></b>"};

static const struct TAbautTxt AbautTxt = {
  .Programm =   "Tiny-CAN Diag",
  .LogoFile =   NULL,
  .Version =    "Version: 1.10",
  .Copyright =  "Copyright (C) 2017 - 2018 MHS-Elektronik GmbH &amp; Co. KG, Germany",
  .Autor =      "Klaus Demlehner",
  .Email =      "klaus@mhs-elektronik.de",
  .Homepage =   "www.MHS-Elektronik.de",
  .ShortLizenz = "This Programm comes with ABSOLUTELY NO WARRANTY; for details type \"GNU Licence\".\n" \
     "This is free software, and you are welcomme to redistribute it under certain\n" \
     "conditions; type \"GNU Licence\" for details",
  .LizenzFile = "COPYING"};

static const gchar WelcomeHeaderString[] = {
  "<span weight=\"bold\" size=\"x-large\" foreground=\"white\">"
  "Tiny-CAN Diag" \
  "</span>\n<span size=\"large\" foreground=\"white\">" \
  "Version: 1.00" \
  "</span>"};


static gint RefreshMainWin(gpointer user_data);


static void SetupRunningLight(struct TCanHw *hw, guint start)
{
GtkWidget *widget;

if (start)
  {
  hw->RunningLightDelay = 0;
  for (hw->RunningLightIndex = 0; hw->RunningLightIndex < hw->IoPinsCount; hw->RunningLightIndex++)
    {
    if (hw->IoPin[hw->RunningLightIndex].Mode == PIN_CFG_DIG_OUT)
      break;
    }
  if ((widget = hw->IoPin[hw->RunningLightIndex].OutWidget))
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
  hw->RunningLight = TRUE;
  }
else
  hw->RunningLight = FALSE;
}


static void SetAllOutputs(struct TCanHw *hw, int state)
{
guint i;
gboolean s;
GtkWidget *widget;

SetupRunningLight(hw, 0);
if (state)
  s = TRUE;
else
  s = FALSE;
for (i = 0; i < hw->IoPinsCount; i++)
  {
  if ((widget = hw->IoPin[i].OutWidget))
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), s);
  }
}


void OpenProjectFile(struct TMainWin *main_win)
{
gchar *file_name;

file_name = g_strdup(main_win->PrjFileName);
if (SelectFileDlg("Project laden", FILE_SELECTION_OPEN, &file_name) > 0)
  LoadPrj(main_win, file_name);
safe_free(file_name);
}


/**************************************************************/
/* Programm Status anpassen                                   */
/**************************************************************/
void UpdateMain(struct TMainWin *main_win)
{
gint id;
const char *msg_str;
char *str;

switch(main_win->ProgMode)
  {
  case PROG_OFFLINE      : {  // Verbindung noch nicht hergestellt
                           msg_str = PROG_OFFLINE_MSG;
                           break;
                           }
  case PROG_ERROR        : {  // COM Port kann nicht geöffnet werden
                           msg_str = PROG_ERROR_MSG;
                           break;
                           }
  case PROG_GO_ONLINE    : { // COM Port geöffnet, Verbindung wird aufgebaut
                           msg_str = PORG_GO_ONLINE_MSG;
                           break;
                           }
  case PROG_RUN          : {  // System Idle
                           msg_str = PROG_RUN_MSG;
                           break;
                           }
  default                : msg_str = PROG_OFFLINE_MSG;

  }
str = g_strdup_printf("Status: %s", msg_str);
id = gtk_statusbar_get_context_id(GTK_STATUSBAR(main_win->Statusbar), str);
gtk_statusbar_push(GTK_STATUSBAR(main_win->Statusbar), id, str);
safe_free(str);
}


void SetProgMode(struct TMainWin *main_win, gint prog_mode)
{
GtkWidget *image;
GtkIconSize iconsize;

iconsize = gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_win->MainToolbar));
if (prog_mode != main_win->ProgMode)
  {
  g_signal_handler_block(main_win->ConnectButton, main_win->ConnectButtonSigId);
  main_win->ProgMode = prog_mode;
  switch(main_win->ProgMode)
    {
    case PROG_WELCOMME   : {
                           gtk_widget_set_sensitive(main_win->MNeu, FALSE);
                           gtk_widget_set_sensitive(main_win->MSave, FALSE);
                           gtk_widget_set_sensitive(main_win->NeuButton, FALSE);
                           gtk_widget_set_sensitive(main_win->FileSaveButton, FALSE);
                           gtk_widget_set_sensitive(main_win->ConnectButton, FALSE);
                           break;
                           }
    case PROG_OFFLINE    :    // Verbindung noch nicht hergestellt
    case PROG_ERROR      : {  // COM Port kann nicht geöffnet werden
                           //gtk_widget_set_sensitive(MainWin.MEinstellungen, TRUE);
                           if (main_win->RefreshTimerId)
                             {
                             g_source_remove(main_win->RefreshTimerId);
                             main_win->RefreshTimerId = 0;
                             }
                           gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(main_win->ConnectButton), FALSE);
                           image = gtk_image_new_from_stock(GTK_STOCK_DISCONNECT, iconsize);
                           gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_win->ConnectButton), image);
                           gtk_tool_button_set_label(GTK_TOOL_BUTTON(main_win->ConnectButton), "Verbinden");
                           gtk_widget_show_all(main_win->ConnectButton);
                           break;
                           }
    case PROG_GO_ONLINE   : {  // COM Port geöffnet
                           //gtk_widget_set_sensitive(MainWin.MEinstellungen, FALSE);
                           gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(main_win->ConnectButton), TRUE);
                           image = gtk_image_new_from_stock(GTK_STOCK_DISCONNECT, iconsize);
                           gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_win->ConnectButton), image);
                           gtk_tool_button_set_label(GTK_TOOL_BUTTON(main_win->ConnectButton), "Verbindungsaufbau");
                           gtk_widget_show_all(main_win->ConnectButton);
                           UpdateGTK();
                           break;
                           }
    case PROG_RUN       : {  // System RUN
                           gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(main_win->ConnectButton), TRUE);
                           image = gtk_image_new_from_stock(GTK_STOCK_CONNECT, iconsize);
                           gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_win->ConnectButton), image);
                           gtk_tool_button_set_label(GTK_TOOL_BUTTON(main_win->ConnectButton), "Trennen");
                           gtk_widget_show_all(main_win->ConnectButton);
                           UpdateGTK();
                           break;
                           }
    }
  g_signal_handler_unblock(main_win->ConnectButton, main_win->ConnectButtonSigId);
  if (main_win->ProgMode != PROG_WELCOMME)
    {
    gtk_widget_set_sensitive(main_win->MNeu, TRUE);
    gtk_widget_set_sensitive(main_win->MSave, TRUE);
    gtk_widget_set_sensitive(main_win->NeuButton, TRUE);
    gtk_widget_set_sensitive(main_win->FileSaveButton, TRUE);
    gtk_widget_set_sensitive(main_win->ConnectButton, TRUE);
    }
  if (main_win->ProgMode == PROG_RUN)
    {
    gtk_widget_set_sensitive(main_win->MDevice, TRUE);
    gtk_widget_set_sensitive(main_win->InfoButton, TRUE);
    }
  else
    {
    gtk_widget_set_sensitive(main_win->MDevice, FALSE);
    gtk_widget_set_sensitive(main_win->InfoButton, FALSE);
    }
  UpdateMain(main_win);
  }
}

/******************************************************************************/
/*                         Callback-Funktionen                                */
/******************************************************************************/
static void on_new_activate(GtkMenuItem *menuitem, gpointer user_data)
{
struct TMainWin *main_win;
(void)menuitem;

if (!(main_win = (struct TMainWin *)user_data))
  return;
DestryPrj(main_win);
}


static void on_open_activate(GtkMenuItem *menuitem, gpointer user_data)
{
struct TMainWin *main_win;
(void)menuitem;

if (!(main_win = (struct TMainWin *)user_data))
  return;
OpenProjectFile(main_win);
}


static void on_save_activate(GtkMenuItem *menuitem, gpointer user_data)
{
gchar *file_name;
struct TMainWin *main_win;
(void)menuitem;

if (!(main_win = (struct TMainWin *)user_data))
  return;
file_name = g_strdup(main_win->PrjFileName);
if (SelectFileDlg("Project speichern", FILE_SELECTION_SAVE, &file_name) > 0)
  SavePrj(main_win, file_name);
safe_free(file_name);
}


static void on_beenden1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
struct TMainWin *main_win;
(void)menuitem;

main_win = (struct TMainWin *)user_data;
ExitApplikation(main_win);
// Programm beenden
gtk_main_quit();
}


static void on_about1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
(void)menuitem;
(void)user_data;

ShowAbautDlg(Paths.base_dir, &AbautTxt);
}


static void on_alle_outputs_off_activate(GtkMenuItem *menuitem, gpointer user_data)
{
struct TMainWin *main_win;
struct TCanHw *hw;
(void)menuitem;

if (!(main_win = (struct TMainWin *)user_data))
  return;
if (!main_win->AppPage)
  return;
if (!(hw = (struct TCanHw *)g_object_get_data(G_OBJECT(main_win->AppPage), "can_hw")))
  return;
SetAllOutputs(hw, 0);
}


static void on_alle_outputs_on_activate(GtkMenuItem *menuitem, gpointer user_data)
{
struct TMainWin *main_win;
struct TCanHw *hw;
(void)menuitem;

if (!(main_win = (struct TMainWin *)user_data))
  return;
if (!main_win->AppPage)
  return;
if (!(hw = (struct TCanHw *)g_object_get_data(G_OBJECT(main_win->AppPage), "can_hw")))
  return;
SetAllOutputs(hw, 1);
}


static void on_alle_outputs_running_activte(GtkCheckMenuItem *menuitem, gpointer user_data)
{
struct TMainWin *main_win;
struct TCanHw *hw;
(void)menuitem;

if (!(main_win = (struct TMainWin *)user_data))
  return;
if (!main_win->AppPage)
  return;
if (!(hw = (struct TCanHw *)g_object_get_data(G_OBJECT(main_win->AppPage), "can_hw")))
  return;
if (menuitem->active == TRUE)
  SetupRunningLight(hw, 1);
else
  SetupRunningLight(hw, 0);
}


void on_config1_activate(GtkCheckMenuItem *object, gpointer user_data)
{
struct TMainWin *main_win;
struct TCanHw *hw;

if (!(main_win = (struct TMainWin *)user_data))
  return;
if (!main_win->AppPage)
  return;
if (!(hw = (struct TCanHw *)g_object_get_data(G_OBJECT(main_win->AppPage), "can_hw")))
  return;
SetupRunningLight(hw, 0);
if (object->active == TRUE)
  {
  if (main_win->RefreshTimerId)
    {
    g_source_remove(main_win->RefreshTimerId);
    main_win->RefreshTimerId = 0;
    }
  SetGuiShowing(hw, 1);
  }
else
  {
  GetConfigFromGui(hw);
  (void)TinyCanSetIoConfig(hw);
  SetGuiShowing(hw, 0);
  if (!main_win->RefreshTimerId)
    main_win->RefreshTimerId = g_timeout_add(WINDOW_REFRESH_TIME, RefreshMainWin, main_win);
  }
}


/*****************************/
/* Toolbar buttons callbacks */
/*****************************/
static void NewButtonCB(GtkButton *button, gpointer user_data)
{
(void)button;
(void)user_data;

on_new_activate(NULL, user_data);
}



static void OpenButtonCB(GtkButton *button, gpointer user_data)
{
(void)button;

on_open_activate(NULL, user_data);
}


static void SaveButtonCB(GtkButton *button, gpointer user_data)
{
(void)button;

on_save_activate(NULL, user_data);
}


static void HardwareInfoButtonCB(GtkButton *button, gpointer user_data)
{
struct TMainWin *main_win;
struct TCanHw *hw;
(void)button;

if (!(main_win = (struct TMainWin *)user_data))
  return;
if (!main_win->AppPage)
  return;
if (!(hw = (struct TCanHw *)g_object_get_data(G_OBJECT(main_win->AppPage), "can_hw")))
  return;
if (hw->HardwareInfoStr)
  ShowInfoWin(hw->HardwareInfoStr, INFO_TITLE_HARDWARE, INFO_WIN_TITLE);
}


static void ConnectButtonCB(GtkButton *button, gpointer user_data)
{
struct TMainWin *main_win;
struct TCanHw *hw;
int32_t err;
(void)button;

if (!(main_win = (struct TMainWin *)user_data))
  return;
if (!main_win->AppPage)
  return;
if (!(hw = (struct TCanHw *)g_object_get_data(G_OBJECT(main_win->AppPage), "can_hw")))
  return;
if (main_win->ProgMode < PROG_RUN)
  {  // Verbindung Herstellen
  SetProgMode(main_win, PROG_GO_ONLINE);
  if ((err = TinyCanOpen(hw)) >= 0)
    err = TinyCanSetIoConfig(hw);
  if (err < 0)
    SetProgMode(main_win, PROG_ERROR);
  else
    {
    SetProgMode(main_win, PROG_RUN);
    if (!main_win->RefreshTimerId)
      main_win->RefreshTimerId = g_timeout_add(WINDOW_REFRESH_TIME, RefreshMainWin, main_win);
    }
  }
else
  {  // Verbindung Trennen
  SetProgMode(main_win, PROG_OFFLINE);
  TinyCanClose(hw);
  }
}


// **** Beenden Button
static void BeendenButtonCB(GtkButton *button, gpointer user_data)
{
(void)button;

on_beenden1_activate(NULL, user_data);
}


/******************************************************************************/
/*                      Timer Callback-Funktionen                             */
/******************************************************************************/
static gint RefreshMainWin(gpointer user_data)
{
struct TMainWin *main_win;
struct TCanHw *hw;
guint size, i;
int err;
uint16_t value;
int32_t long_value;
uint16_t ports[3];
uint32_t mask, pin, keycode;
GtkWidget *widget;

err = 0;
if (!(main_win = (struct TMainWin *)user_data))
  return(FALSE);
if (!main_win->AppPage)
  return(FALSE);
if (!(hw = (struct TCanHw *)g_object_get_data(G_OBJECT(main_win->AppPage), "can_hw")))
  return(FALSE);
// **** Lauflicht
if (hw->RunningLight)
  {
  if (++hw->RunningLightDelay >= 0)
    {
    hw->RunningLightDelay = 0;
    if ((widget = hw->IoPin[hw->RunningLightIndex].OutWidget))
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
    for (hw->RunningLightIndex++; hw->RunningLightIndex < hw->IoPinsCount; hw->RunningLightIndex++)
      {
      if (hw->IoPin[hw->RunningLightIndex].Mode == PIN_CFG_DIG_OUT)
        break;
      }
    if (hw->RunningLightIndex >= hw->IoPinsCount)
      {
      for (hw->RunningLightIndex = 0; hw->RunningLightIndex < hw->IoPinsCount; hw->RunningLightIndex++)
        {
        if (hw->IoPin[hw->RunningLightIndex].Mode == PIN_CFG_DIG_OUT)
          break;
        }
      }
    if ((widget = hw->IoPin[hw->RunningLightIndex].OutWidget))
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
    }
  }
// **** Digital Eingänge lesen
size = hw->IoPinsCount / 16;
if (hw->IoPinsCount % 16)
  size++;
for (i = 0; i < size; i++)
  {
  if ((err = GetDigInPort(hw, i, &ports[i])))
    break;
  }
if (err)
  return(FALSE);
// **** ports[] -> IoPin[].In
pin = 0;
for (i = 0; i < size; i++)
  {
  value = ports[i];
  for (mask = 0x00001; mask < 0x10000; mask <<= 1)
    {
    if (value & (uint16_t)mask)
      hw->IoPin[pin].In = 1;
    else
      hw->IoPin[pin].In = 0;
    if (++pin >= hw->IoPinsCount)
      break;
    }
  }
// **** Analog Eingänge / Encoder lesen
for (i = 0; i < hw->IoPinsCount; i++)
  {
  if (hw->IoPin[i].Mode == PIN_CFG_AN_IN)
    {
    if ((err = GetAnalogInPin(hw, i, &value)))
      break;
    hw->IoPin[i].AIn = value;
    }
  else if (hw->IoPin[i].Mode == PIN_CFG_ENCODER)
    {
    if ((err = GetEncoderInPin(hw, i, &long_value)))
      break;
    hw->IoPin[i].AIn = long_value;
    }
  }
if (err)
  return(FALSE);

// **** Fenster zeichnen
for (i = 0; i < hw->IoPinsCount; i++)
  {
  if (hw->IoPin[i].Mode == PIN_CFG_DIG_IN)
    {
    if (hw->IoPin[i].InWidget)
      {
      if (hw->IoPin[i].In)
        giw_led_set_mode(GIW_LED(hw->IoPin[i].InWidget), 1);
      else
        giw_led_set_mode(GIW_LED(hw->IoPin[i].InWidget), 0);
      }
    }
  else if ((hw->IoPin[i].Mode == PIN_CFG_AN_IN) || (hw->IoPin[i].Mode == PIN_CFG_ENCODER))
    SetWidgetLabelAsInt(hw->IoPin[i].AInWidget, hw->IoPin[i].AIn);
  }
// Taster einlesen
for (i = 0; i < 100; i++)
  {
  if (GetKeycode(hw, &keycode) > 0)
    AddKeyToList(hw, keycode);
  else
    break;
  }
return(TRUE);
}


static GtkWidget *welcome_header_new(void)
{
GtkWidget *item_vb, *item_hb, *eb, *widget;
gchar *filename;

item_vb = ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 0, FALSE);
// colorize vbox
eb = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(eb), item_vb);
#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_override_background_color(eb, GTK_STATE_FLAG_NORMAL, &rgba_header_bar_bg);
#else
gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &header_bar_bg);
#endif
item_hb = ws_gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0, FALSE);
gtk_box_pack_start(GTK_BOX(item_vb), item_hb, FALSE, FALSE, 10);

filename = CreateFileName(Paths.base_dir, TINY_CAN_DIAG_ICONE);
widget = gtk_image_new_from_file(filename);
g_free(filename);
gtk_box_pack_start(GTK_BOX(item_hb), widget, FALSE, FALSE, 10);

widget = gtk_label_new(NULL);
gtk_label_set_markup(GTK_LABEL(widget), WelcomeHeaderString);
gtk_misc_set_alignment(GTK_MISC(widget), 0.0f, 0.5f);
gtk_box_pack_start(GTK_BOX(item_hb), widget, TRUE, TRUE, 5);
gtk_widget_show_all(eb);
return(eb);
}


GtkWidget *CreateMainToolbar(struct TMainWin *main_win)
{
GtkWidget *widget, *image;
guint tmp_toolbar_icon_size;

main_win->MainToolbar = gtk_toolbar_new();
gtk_widget_show (main_win->MainToolbar);
gtk_toolbar_set_style (GTK_TOOLBAR (main_win->MainToolbar), GTK_TOOLBAR_BOTH);
tmp_toolbar_icon_size = gtk_toolbar_get_icon_size(GTK_TOOLBAR (main_win->MainToolbar));

image = gtk_image_new_from_stock("gtk-new", tmp_toolbar_icon_size);
main_win->NeuButton = (GtkWidget*) gtk_tool_button_new (image, "Neu");
gtk_container_add(GTK_CONTAINER(main_win->MainToolbar), main_win->NeuButton);

image = gtk_image_new_from_stock ("gtk-open", tmp_toolbar_icon_size);
main_win->FileOpenButton = (GtkWidget*) gtk_tool_button_new(image, "Öffnen");
gtk_container_add(GTK_CONTAINER (main_win->MainToolbar), main_win->FileOpenButton);

image = gtk_image_new_from_stock ("gtk-save", tmp_toolbar_icon_size);
main_win->FileSaveButton = (GtkWidget*) gtk_tool_button_new(image, "Speichern");
gtk_container_add(GTK_CONTAINER (main_win->MainToolbar), main_win->FileSaveButton);

widget = (GtkWidget*)gtk_separator_tool_item_new();
gtk_container_add(GTK_CONTAINER(main_win->MainToolbar), widget);

main_win->ConnectButton = (GtkWidget*) gtk_toggle_tool_button_new();
gtk_tool_button_set_label(GTK_TOOL_BUTTON(main_win->ConnectButton), "Verbinden");
image = gtk_image_new_from_stock ("gtk-disconnect", tmp_toolbar_icon_size);
gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON (main_win->ConnectButton), image);
gtk_container_add(GTK_CONTAINER(main_win->MainToolbar), main_win->ConnectButton);

widget = (GtkWidget*) gtk_separator_tool_item_new();
gtk_container_add (GTK_CONTAINER (main_win->MainToolbar), widget);

image = gtk_image_new_from_stock ("gtk-info", tmp_toolbar_icon_size);
main_win->InfoButton = (GtkWidget*) gtk_tool_button_new(image, "Hardware Info");
gtk_container_add(GTK_CONTAINER(main_win->MainToolbar), main_win->InfoButton);

widget = (GtkWidget*) gtk_separator_tool_item_new();
gtk_container_add (GTK_CONTAINER (main_win->MainToolbar), widget);

image = gtk_image_new_from_stock ("gtk-quit", tmp_toolbar_icon_size);
main_win->BeendenButton = (GtkWidget*) gtk_tool_button_new(image, "Exit");
gtk_container_add(GTK_CONTAINER(main_win->MainToolbar), main_win->BeendenButton);

return(main_win->MainToolbar);
}


/*static void GuiDestroy(GtkWidget *widget, gpointer data) <*>
{
struct TMainWin *main_win;
(void)widget;

main_win = (struct TMainWin *)data;
safe_free(main_win->PrjFileName);
g_free(main_win);
} */


static gint GuiDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct TMainWin *main_win;
(void)widget;
(void)event;

main_win = (struct TMainWin *)user_data;
ExitApplikation(main_win);
return(FALSE);
}


void DestroyMainWinData(struct TMainWin *main_win)
{
safe_free(main_win->PrjFileName);
g_free(main_win);
}


struct TMainWin *CreateMainWin(void)
{
struct TMainWin *main_win;
GtkWidget *vbox;
GtkWidget *hbox1, *vbox1;
GtkWidget *menubar1;
GtkWidget *menuitem1;
GtkWidget *menuitem1_menu;
GtkWidget *open1;
GtkWidget *beenden1;
GtkWidget *io1_menu;
GtkWidget *alle_outputs_0;
GtkWidget *alle_outputs_1;
GtkWidget *alle_outputs_running;
GtkWidget *separator1;
GtkWidget *menuitem4;
GtkWidget *menuitem4_menu;
GtkWidget *about1;
GtkWidget *widget;
GtkAccelGroup *accel_group;

if (!(main_win = (struct TMainWin *)g_malloc0(sizeof(struct TMainWin))))
  return(NULL);
accel_group = gtk_accel_group_new();

main_win->MainWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_window_set_title(GTK_WINDOW(main_win->MainWin), "Tiny-CAN Diag");
gtk_window_set_position(GTK_WINDOW (main_win->MainWin), GTK_WIN_POS_CENTER);
gtk_window_set_resizable(GTK_WINDOW (main_win->MainWin), FALSE);

g_object_set_data(G_OBJECT(main_win->MainWin), "main_win", main_win);
//g_signal_connect(G_OBJECT(main_win->MainWin), "destroy", G_CALLBACK(GuiDestroy), main_win); <*>
g_signal_connect(G_OBJECT(main_win->MainWin), "delete_event", G_CALLBACK(GuiDeleteEvent), main_win);


vbox = gtk_vbox_new (FALSE, 0);
gtk_container_add (GTK_CONTAINER (main_win->MainWin), vbox);

hbox1 = gtk_hbox_new (FALSE, 0);
vbox1 = gtk_vbox_new (FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox1), vbox1, TRUE, TRUE, 0);
widget = welcome_header_new();
gtk_box_pack_end(GTK_BOX(hbox1), widget, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 0);

menubar1 = gtk_menu_bar_new ();
gtk_box_pack_start (GTK_BOX (vbox1), menubar1, FALSE, FALSE, 0);
//gtk_box_pack_start (GTK_BOX (vbox), menubar1, FALSE, FALSE, 0); <*>

menuitem1 = gtk_menu_item_new_with_mnemonic ("_File");
gtk_widget_show (menuitem1);
gtk_container_add (GTK_CONTAINER (menubar1), menuitem1);

menuitem1_menu = gtk_menu_new ();
gtk_menu_item_set_submenu(GTK_MENU_ITEM (menuitem1), menuitem1_menu);

main_win->MNeu = gtk_image_menu_item_new_from_stock ("gtk-new", accel_group);
gtk_container_add (GTK_CONTAINER (menuitem1_menu), main_win->MNeu);

open1 = gtk_image_menu_item_new_from_stock ("gtk-open", accel_group);
gtk_container_add (GTK_CONTAINER (menuitem1_menu), open1);

widget = gtk_menu_item_new_with_mnemonic("Zuletzt geöffnete Dateien");
gtk_container_add(GTK_CONTAINER (menuitem1_menu), widget);

main_win->RecentFilesMenu = gtk_menu_new();
gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget), main_win->RecentFilesMenu);

main_win->MSave = gtk_image_menu_item_new_from_stock ("gtk-save", accel_group);
gtk_container_add (GTK_CONTAINER (menuitem1_menu), main_win->MSave);

widget = gtk_separator_menu_item_new ();
gtk_container_add (GTK_CONTAINER(menuitem1_menu), widget);
gtk_widget_set_sensitive(widget, FALSE);

beenden1 = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
gtk_container_add (GTK_CONTAINER (menuitem1_menu), beenden1);

main_win->MDevice = gtk_menu_item_new_with_mnemonic ("IO");
gtk_container_add (GTK_CONTAINER (menubar1), main_win->MDevice);

io1_menu = gtk_menu_new ();
gtk_menu_item_set_submenu(GTK_MENU_ITEM(main_win->MDevice), io1_menu);

alle_outputs_0 = gtk_menu_item_new_with_mnemonic ("Alle Outputs 0");
gtk_container_add (GTK_CONTAINER (io1_menu), alle_outputs_0);

alle_outputs_1 = gtk_menu_item_new_with_mnemonic ("Alle Outputs 1");
gtk_container_add (GTK_CONTAINER (io1_menu), alle_outputs_1);

alle_outputs_running = gtk_check_menu_item_new_with_mnemonic("Lauflicht");
gtk_container_add (GTK_CONTAINER (io1_menu), alle_outputs_running);

separator1 = gtk_separator_menu_item_new ();
gtk_container_add (GTK_CONTAINER (io1_menu), separator1);
gtk_widget_set_sensitive (separator1, FALSE);

main_win->MConfig = gtk_check_menu_item_new_with_mnemonic ("Config");
gtk_container_add (GTK_CONTAINER (io1_menu), main_win->MConfig);

menuitem4 = gtk_menu_item_new_with_mnemonic ("_Help");
gtk_container_add (GTK_CONTAINER (menubar1), menuitem4);

menuitem4_menu = gtk_menu_new ();
gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem4), menuitem4_menu);

about1 = gtk_menu_item_new_with_mnemonic ("_About");
gtk_container_add (GTK_CONTAINER (menuitem4_menu), about1);

widget = CreateMainToolbar(main_win);
gtk_box_pack_start (GTK_BOX (vbox1), widget, FALSE, FALSE, 0);
//gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0); <*>

//widget = gtk_hseparator_new (); <*>
//gtk_box_pack_start (GTK_BOX (vbox1), widget, TRUE, TRUE, 4);

main_win->Notebook = gtk_notebook_new();
gtk_notebook_set_show_tabs(GTK_NOTEBOOK(main_win->Notebook), FALSE);
// 1. Seite -> StartWin
widget = welcome_new(main_win);
gtk_notebook_append_page(GTK_NOTEBOOK(main_win->Notebook), widget, NULL);
// 2. Seite -> App
main_win->AppContainer = gtk_alignment_new(0.5, 0.5, 5, 5);
gtk_notebook_append_page(GTK_NOTEBOOK(main_win->Notebook), main_win->AppContainer, NULL);

gtk_box_pack_start(GTK_BOX(vbox), main_win->Notebook, TRUE, TRUE, 0);

main_win->Statusbar = gtk_statusbar_new ();

gtk_box_pack_start (GTK_BOX (vbox), main_win->Statusbar, FALSE, FALSE, 0);
gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (main_win->Statusbar), FALSE);

gtk_signal_connect(GTK_OBJECT(main_win->MainWin), "destroy", GTK_SIGNAL_FUNC(gtk_exit), main_win);
g_signal_connect ((gpointer) main_win->MNeu, "activate", G_CALLBACK(on_new_activate), main_win);
g_signal_connect ((gpointer) open1, "activate", G_CALLBACK(on_open_activate), main_win);
g_signal_connect ((gpointer) main_win->MSave, "activate", G_CALLBACK(on_save_activate), main_win);
g_signal_connect ((gpointer) beenden1, "activate", G_CALLBACK (on_beenden1_activate), main_win);
g_signal_connect ((gpointer) alle_outputs_0, "activate", G_CALLBACK (on_alle_outputs_off_activate), main_win);
g_signal_connect ((gpointer) alle_outputs_1, "activate", G_CALLBACK (on_alle_outputs_on_activate), main_win);
g_signal_connect ((gpointer) alle_outputs_running, "activate", G_CALLBACK (on_alle_outputs_running_activte), main_win);
main_win->MConfigSigId = g_signal_connect ((gpointer) main_win->MConfig, "activate", G_CALLBACK (on_config1_activate), main_win);
g_signal_connect ((gpointer) about1, "activate", G_CALLBACK (on_about1_activate), main_win);

g_signal_connect(main_win->NeuButton, "clicked", G_CALLBACK(NewButtonCB), main_win);
g_signal_connect(main_win->FileOpenButton, "clicked", G_CALLBACK(OpenButtonCB), main_win);
g_signal_connect(main_win->FileSaveButton, "clicked", G_CALLBACK(SaveButtonCB), main_win);
main_win->ConnectButtonSigId = g_signal_connect(main_win->ConnectButton, "toggled", G_CALLBACK(ConnectButtonCB), main_win);
g_signal_connect(main_win->InfoButton, "clicked", G_CALLBACK(HardwareInfoButtonCB), main_win);
g_signal_connect(main_win->BeendenButton, "clicked", G_CALLBACK(BeendenButtonCB), main_win);

gtk_window_add_accel_group (GTK_WINDOW(main_win->MainWin), accel_group);
gtk_widget_show_all(main_win->MainWin);
gtk_window_set_position(GTK_WINDOW(main_win->MainWin), GTK_WIN_POS_CENTER_ALWAYS);
SetProgMode(main_win, PROG_WELCOMME);
return(main_win);
}
