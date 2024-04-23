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
#include "dialogs.h"
#include "gtk_util.h"
#include "configfile.h"
#include "tcan_mx_gui.h"
#include "gui_help.h"
#include "io.h"
#include "recent_files.h"
#include "project.h"


static gint LoadPrjData(struct TCanHw *hw, ConfigFile *cfgfile)
{
gchar section[10];
gchar *tmpbuf;
guint i;
int mode;

tmpbuf = NULL;
for (i = 0; i < hw->IoPinsCount; i++)
  {
  g_snprintf(section, 10, "IDX%u", i);
  if (cfg_read_string(cfgfile, section, "Name", &tmpbuf))
    {
    safe_free(hw->IoPin[i].Name);
    hw->IoPin[i].Name = g_strdup(tmpbuf);
    safe_free(tmpbuf);
    }
  if (cfg_read_int(cfgfile, section, "Mode", &mode))
    hw->IoPin[i].Mode = mode;
  }
return(0);
}


static gint SavePrjFile(struct TCanHw *hw, const gchar *filename)
{
ConfigFile *cfgfile;
char section[10];
guint i;
int res;

res = 0;
if (!filename)
  return(-1);
cfgfile = cfg_open_file(filename);

if (!cfgfile)
  cfgfile = cfg_new();
cfg_write_int(cfgfile, "GLOBAL", "HardwareType", hw->HardwareType);
for (i = 0; i < hw->IoPinsCount; i++)
  {
  g_snprintf(section, 10, "IDX%u", i);
  if (hw->IoPin[i].Name)
    cfg_write_string(cfgfile, section, "Name", hw->IoPin[i].Name);
  else
    cfg_remove_key(cfgfile, section, "Name");
  cfg_write_int(cfgfile, section, "Mode", hw->IoPin[i].Mode);
  }
if (cfg_write_file(cfgfile, filename) == FALSE)
  res = -1;
cfg_free(cfgfile);
return(res);
}


void NewPrj(struct TMainWin *main_win, guint hardware_type)
{
GtkWidget *widget;
struct TCanHw *hw;

hw = NULL;
if (main_win->AppPage)
  hw = (struct TCanHw *)g_object_get_data(G_OBJECT(main_win->AppPage), "can_hw");
if (hw)
  {
  //SetProgMode(main_win, PROG_OFFLINE); <*>
  TinyCanClose(hw);
  }
widget = TCanMxGuiCreate(hardware_type);
main_win->AppPage = widget;
g_signal_handler_block(main_win->MConfig, main_win->MConfigSigId);
gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(main_win->MConfig), TRUE);
g_signal_handler_unblock(main_win->MConfig, main_win->MConfigSigId);
SetProgMode(main_win, PROG_OFFLINE);
gtk_container_add(GTK_CONTAINER(main_win->AppContainer), widget);
gtk_notebook_set_current_page(GTK_NOTEBOOK(main_win->Notebook), APP_PAGE_INDEX);
}


void DestryPrj(struct TMainWin *main_win)
{
if (main_win->AppPage)
  {
  SetProgMode(main_win, PROG_OFFLINE);
  gtk_container_remove(GTK_CONTAINER(main_win->AppContainer), main_win->AppPage);
  main_win->AppPage = NULL;
  }
gtk_notebook_set_current_page(GTK_NOTEBOOK(main_win->Notebook), START_PAGE_INDEX);
SetProgMode(main_win, PROG_WELCOMME);
}


void LoadPrj(struct TMainWin *main_win, const gchar *filename)
{
ConfigFile *cfgfile;
struct TCanHw *hw;
gint hardware_type;
gchar *str;
gboolean res;

if (!main_win)
  return;
res = FALSE;
if ((cfgfile = cfg_open_file(filename)))
  res = cfg_read_int(cfgfile, "GLOBAL", "HardwareType", &hardware_type);
if ((res == FALSE) || (hardware_type < HARDWARE_TYPE_TCAN_M2) || (hardware_type > HARDWARE_TYPE_TCAN_M3))
  {
  msg_box(MSG_TYPE_ERROR, "Error", "Feher beim laden des Projects %s", filename);
  cfg_free(cfgfile);
  return;
  }
DestryPrj(main_win);
NewPrj(main_win, hardware_type);

hw = (struct TCanHw *)g_object_get_data(G_OBJECT(main_win->AppPage), "can_hw");
LoadPrjData(hw, cfgfile);
safe_free(main_win->PrjFileName);
main_win->PrjFileName = g_strdup(filename);
str = g_strdup_printf("Tiny-CAN Diag - [%s]", filename);
gtk_window_set_title(GTK_WINDOW(main_win->MainWin), str);
g_free(str);

/*SetupRunningLight(hw, 0); <*>
  if (RefreshTimerId)
    {
    g_source_remove(RefreshTimerId);
    RefreshTimerId = 0;
    }
  if (!LoadConfigFile(hw, file_name))
    {
    safe_free(CfgFileName);
    CfgFileName = g_strdup(file_name);
    }
  SetConfigToGui(hw);
  (void)TinyCanSetIoConfig(hw);
  SetGuiShowing(hw, 0);
  if (!RefreshTimerId)
    RefreshTimerId = g_timeout_add(WINDOW_REFRESH_TIME, RefreshMainWin, main_win);*/
SetConfigToGui(hw);
(void)TinyCanSetIoConfig(hw);
SetGuiShowing(hw, 0);
RecentFileAddToList(main_win, filename);
UpdateRecentItems(main_win);
cfg_free(cfgfile);
}


void SavePrj(struct TMainWin *main_win, const gchar *filename)
{
struct TCanHw *hw;
gchar *str;

hw = NULL;
if (main_win)
  {
  if (main_win->AppPage)
    hw = (struct TCanHw *)g_object_get_data(G_OBJECT(main_win->AppPage), "can_hw");
  }
if (!hw)
  msg_box(MSG_TYPE_ERROR, "Error", "Feher beim speichern des Projects %s", filename);
else
  {
  if (!SavePrjFile(hw, filename))
    {
    safe_free(main_win->PrjFileName);
    main_win->PrjFileName = g_strdup(filename);
    RecentFileAddToList(main_win, filename);
    UpdateRecentItems(main_win);
    str = g_strdup_printf("Tiny-CAN Diag - [%s]", filename);
    gtk_window_set_title(GTK_WINDOW(main_win->MainWin), str);
    g_free(str);
    }
  else
    msg_box(MSG_TYPE_ERROR, "Error", "Feher beim speichern des Projects %s", filename);
  }
}
