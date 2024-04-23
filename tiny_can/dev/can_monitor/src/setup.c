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
/*                          Programm Setup                                */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Setup (Einstellungen) Dialog anzeigen             */
/*                    - Setup Datei laden/speichern                       */
/*                    - Setup einstellunge setzen                         */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : setup.c                                             */
/* ---------------------------------------------------------------------- */
/*  Datum           : 17.10.05                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include "can_monitor.h"
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "gtk_util.h"
#include "setup.h"
#include "dialogs.h"
#include "main.h"
#include "main_can.h"
#include "drv_info.h"
#include "const.h"
#include "makro_win.h"
#include "cview.h"
#include "configfile.h"
#include "moofontsel.h"
#include "support.h"

#ifdef __WIN32__
#else
#include "linux_port_setup.h"
#endif


struct TSetupData SetupData;
struct TSetupWin *SetupWin;

const char SETUPDATEI[] = {"can_monitor.cfg"};
const char MAKRODATEI_DEF[] = {"can_monitor.mak"};
const char FILTERDATEI_DEF[] = {"can_monitor.fil"};

void AnsichtWinDestroy(struct TAnsichtSetupWin *ansicht_win);
void AnsichtWinUpdate(struct TAnsichtSetupWin *ansicht_win);
void AnsichtWinGet(struct TAnsichtSetupWin *ansicht_win);

void DataListWinDestroy(struct TDataListSetupWin *data_list_win);
void DataListWinUpdate(struct TDataListSetupWin *data_list_win);
void DataListWinGet(struct TDataListSetupWin *data_list_win);

void FilterWinDestroy(struct TFilterSetupWin *filter_win);
void FilterWinUpdate(struct TFilterSetupWin *filter_win);
void FilterWinGet(struct TFilterSetupWin *filter_win);

void SendenWinDestroy(struct TSendenSetupWin *senden_win);
void SendenWinUpdate(struct TSendenSetupWin *senden_win);
void SendenWinGet(struct TSendenSetupWin *senden_win);

void HardwareWinDestroy(struct THardwareSetupWin *hardware_win);
void HardwareWinUpdate(struct THardwareSetupWin *hardware_win);
void HardwareWinGet(struct THardwareSetupWin *hardware_win);


void on_PortSetupButton_clicked(GtkButton *button, gpointer user_data);


struct TSetupWin *ShowSetupDlg(unsigned long showing_pages);


void SetupInit(void)
{
memset(&SetupData, 0, sizeof(struct TSetupData));
SetupSetDefaults();
}


void SetupDestroy(void)
{
save_free(SetupData.MakroFile);
save_free(SetupData.FilterFile);
save_free(SetupData.DriverFile);
save_free(SetupData.DriverPath);
save_free(SetupData.WdgFontName);
save_free(SetupData.ComDeviceName);
save_free(SetupData.Snr);
}


void SetupSetDefaults(void)
{
// **** Variablen mit Default werten vorbelegen
SetupData.CanSpeed = 125;     // 125 kBit/s (CAN Open defaut)
SetupData.DataClearMode = 0;  // Automatisch löschen
SetupData.RxDLimit = 100000;  // 100.000 Messages
SetupData.FilterTimer = 2;    // 500 ms
save_free(SetupData.MakroFile);
SetupData.MakroFile = g_build_filename(Paths.setup_dir, MAKRODATEI_DEF, NULL);
save_free(SetupData.FilterFile);
SetupData.FilterFile = g_build_filename(Paths.setup_dir, FILTERDATEI_DEF, NULL);
save_free(SetupData.DriverFile);
save_free(SetupData.DriverPath);
SetupData.DriverPath = g_strdup(Paths.base_dir);
SetupData.PortIndex = 0;      // COM Port 1
SetupData.ComDrvMode = 1;     // 1 = USB
SetupData.AnsStandart = 1;
SetupData.AnsMakro = 1;
SetupData.AnsSenden = 1;
SetupData.AnsButtonText = 1;
SetupData.SnrEnabled = 0;
save_free(SetupData.ComDeviceName);
save_free(SetupData.Snr);
SetupData.Snr = g_strdup("");
SetupData.USBEnabled = 0;
SetupData.USBAuto = 0;
SetupData.SendenEntrysCount = 1;
SetupData.SendOnReturn = 1;
save_free(SetupData.WdgFontName);
SetupData.WdgFontName = GetDefaultFont(MainWin.MainWin, 1);
memset(&SetupData.WdgTimeColor, 0, sizeof(GdkColor));
memset(&SetupData.WdgTypeColor, 0, sizeof(GdkColor));
memset(&SetupData.WdgIdColor, 0, sizeof(GdkColor));
memset(&SetupData.WdgDlcColor, 0, sizeof(GdkColor));
memset(&SetupData.WdgData1Color, 0, sizeof(GdkColor));
memset(&SetupData.WdgData2Color, 0, sizeof(GdkColor));
SetupData.ShowTime = 0;
SetupData.ShowDir = 0;
SetupData.ShowType = 1;
SetupData.ShowDlc = 1;
SetupData.ShowData1 = 1;
SetupData.ShowData2 = 3;
SetupData.AutoConnect = 1;
SetupData.FilterFrame = 0;
}


/**************************************************************/
/* Programmeinstellungen laden und speichern                  */
/**************************************************************/
int LoadSetup(void)
{
ConfigFile *cfgfile;
gchar *tmpbuf;
int i;

SetupSetDefaults();
tmpbuf = g_build_filename(Paths.setup_dir, SETUPDATEI, NULL);
cfgfile = NULL;
if (g_file_test(tmpbuf, G_FILE_TEST_IS_REGULAR))
  cfgfile = cfg_open_file(tmpbuf);
g_free(tmpbuf);
if (!cfgfile)
  {
  //msg_box(MSG_TYPE_ERROR, _("Error"), _("Setup Datei kann nicht geladen werden"));
  return(-1);
  }
if (cfg_read_int(cfgfile, "Global", "CanSpeedMode", &i))
  SetupData.CanSpeedMode = (unsigned short)i;
if (cfg_read_int(cfgfile, "Global", "CanSpeed", &i))
  SetupData.CanSpeed = (unsigned short)i;
if (cfg_read_int(cfgfile, "Global", "UserCanSpeed", &i))
  SetupData.UserCanSpeed = (unsigned long)i;
if (cfg_read_int(cfgfile, "Global", "CanOpMode", &i))
  SetupData.CanOpMode = (unsigned long)i;
if (cfg_read_int(cfgfile, "Global", "DataClearMode", &i))
  SetupData.DataClearMode = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "RxDLimit", &i))
  SetupData.RxDLimit = (unsigned long)i;
if (cfg_read_int(cfgfile, "Global", "PortIndex", &i))
  SetupData.PortIndex = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "ComDrvMode", &i))
  SetupData.ComDrvMode = i;
if (cfg_read_int(cfgfile, "Global", "BaudRate", &i))
  SetupData.BaudRate = (unsigned long)i;
if (cfg_read_int(cfgfile, "Global", "FilterTimer", &i))
  SetupData.FilterTimer = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "AnsStandart", &i))
  SetupData.AnsStandart = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "AnsMakro", &i))
  SetupData.AnsMakro = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "AnsSenden", &i))
  SetupData.AnsSenden = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "AnsButtonText", &i))
  SetupData.AnsButtonText = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "AutoConnect", &i))
  SetupData.AutoConnect = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "SnrEnabled", &i))
  SetupData.SnrEnabled = (char)i;
if (cfg_read_int(cfgfile, "Global", "CanPortNumber", &i))
  SetupData.CanPortNumber = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "USBEnabled", &i))
  SetupData.USBEnabled = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "USBAuto", &i))
  SetupData.USBAuto = (unsigned char)i;
if (cfg_read_string(cfgfile, "Global", "MakroFile", &tmpbuf))
  {
  save_free(SetupData.MakroFile);
  SetupData.MakroFile = tmpbuf;
  }
if (cfg_read_string(cfgfile, "Global", "FilterFile", &tmpbuf))
  {
  save_free(SetupData.FilterFile);
  SetupData.FilterFile = tmpbuf;
  }
if (cfg_read_string(cfgfile, "Global", "DriverPath", &tmpbuf))
  {
  save_free(SetupData.DriverPath);
  SetupData.DriverPath = tmpbuf;
  }
if (cfg_read_string(cfgfile, "Global", "DriverFile", &tmpbuf))
  {
  save_free(SetupData.DriverFile);
  SetupData.DriverFile = tmpbuf;
  }
if (cfg_read_string(cfgfile, "Global", "Snr", &tmpbuf))
  {
  save_free(SetupData.Snr);
  SetupData.Snr = tmpbuf;
  }
if (cfg_read_string(cfgfile, "Global", "ComDeviceName", &tmpbuf))
  {
  save_free(SetupData.ComDeviceName);
  SetupData.ComDeviceName = tmpbuf;
  }
if (cfg_read_string(cfgfile, "Global", "DriverInitStr", &tmpbuf))
  {
  save_free(SetupData.DriverInitStr);
  SetupData.DriverInitStr = tmpbuf;
  }
cfg_read_color(cfgfile, "Global", "WdgTimeColor", &SetupData.WdgTimeColor);
cfg_read_color(cfgfile, "Global", "WdgTypeColor", &SetupData.WdgTypeColor);
cfg_read_color(cfgfile, "Global", "WdgIdColor", &SetupData.WdgIdColor);
cfg_read_color(cfgfile, "Global", "WdgDlcColor", &SetupData.WdgDlcColor);
cfg_read_color(cfgfile, "Global", "WdgData1Color", &SetupData.WdgData1Color);
cfg_read_color(cfgfile, "Global", "WdgData2Color", &SetupData.WdgData2Color);
if (cfg_read_string(cfgfile, "Global", "WdgFontName", &tmpbuf))
  {
  save_free(SetupData.WdgFontName);
  SetupData.WdgFontName = tmpbuf;
  }
if (cfg_read_int(cfgfile, "Global", "ShowTime", &i))
  SetupData.ShowTime = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "ShowDir", &i))
  SetupData.ShowDir = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "ShowType", &i))
  SetupData.ShowType = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "ShowDlc", &i))
  SetupData.ShowDlc = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "ShowData1", &i))
  SetupData.ShowData1 = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "ShowData2", &i))
  SetupData.ShowData2 = (unsigned char)i;
if (cfg_read_int(cfgfile, "Global", "SendenEntrysCount", &i))
  SetupData.SendenEntrysCount = i;
if (cfg_read_int(cfgfile, "Global", "SendOnReturn", &i))
  SetupData.SendOnReturn = i;
if (cfg_read_int(cfgfile, "Global", "FilterFrame", &i))
  SetupData.FilterFrame = i;

if (!(g_file_test(SetupData.MakroFile, G_FILE_TEST_IS_REGULAR)))
  {
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error loading macro file"));
  save_free(SetupData.MakroFile);
  SetupData.MakroFile = g_build_filename(Paths.setup_dir, MAKRODATEI_DEF, NULL);
  }
if (!(g_file_test(SetupData.FilterFile, G_FILE_TEST_IS_REGULAR)))
  {
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error loading filter file"));
  save_free(SetupData.FilterFile);
  SetupData.FilterFile = g_build_filename(Paths.setup_dir, FILTERDATEI_DEF, NULL);
  }
if (!(g_file_test(SetupData.DriverPath, G_FILE_TEST_IS_DIR)))
  {
  save_free(SetupData.DriverPath);
  SetupData.DriverPath = g_strdup(Paths.base_dir);
  }
if (SetupData.DriverFile)
  {
  tmpbuf = g_build_filename(SetupData.DriverPath, SetupData.DriverFile, NULL);
  if (!g_file_test(tmpbuf, G_FILE_TEST_IS_REGULAR))
    save_free(SetupData.DriverFile);
  save_free(tmpbuf);
  }
SetSetup();
cfg_free(cfgfile);
return(0);
}


int SaveSetup(void)
{
ConfigFile *cfgfile;
int error;
gchar *filename;

error = 0;
filename = g_build_filename(Paths.setup_dir, SETUPDATEI, NULL);
cfgfile = cfg_open_file(filename);

if (!cfgfile)
  cfgfile = cfg_new();

cfg_write_word(cfgfile, "Global", "CanSpeed", SetupData.CanSpeed);
cfg_write_word(cfgfile, "Global", "CanSpeedMode", SetupData.CanSpeedMode);
cfg_write_byte(cfgfile, "Global", "DataClearMode", SetupData.DataClearMode);
cfg_write_int(cfgfile, "Global", "UserCanSpeed", SetupData.UserCanSpeed);
cfg_write_int(cfgfile, "Global", "CanOpMode", SetupData.CanOpMode);
cfg_write_int(cfgfile, "Global", "RxDLimit", SetupData.RxDLimit);
cfg_write_byte(cfgfile, "Global", "PortIndex", SetupData.PortIndex);
cfg_write_int(cfgfile, "Global", "ComDrvMode", SetupData.ComDrvMode);
cfg_write_int(cfgfile, "Global", "BaudRate", SetupData.BaudRate);
cfg_write_byte(cfgfile, "Global", "FilterTimer", SetupData.FilterTimer);
cfg_write_byte(cfgfile, "Global", "AnsStandart", SetupData.AnsStandart);
cfg_write_byte(cfgfile, "Global", "AnsMakro", SetupData.AnsMakro);
cfg_write_byte(cfgfile, "Global", "AnsSenden", SetupData.AnsSenden);
cfg_write_byte(cfgfile, "Global", "AnsButtonText", SetupData.AnsButtonText);
cfg_write_byte(cfgfile, "Global", "AutoConnect", SetupData.AutoConnect);
cfg_write_byte(cfgfile, "Global", "SnrEnabled", SetupData.SnrEnabled);
cfg_write_byte(cfgfile, "Global", "CanPortNumber", SetupData.CanPortNumber);
cfg_write_byte(cfgfile, "Global", "USBEnabled", SetupData.USBEnabled);
cfg_write_byte(cfgfile, "Global", "USBAuto", SetupData.USBAuto);
if (SetupData.MakroFile)
  cfg_write_string(cfgfile, "Global", "MakroFile", SetupData.MakroFile);
else
  cfg_remove_key(cfgfile, "Global", "MakroFile");
if (SetupData.FilterFile)
cfg_write_string(cfgfile, "Global", "FilterFile", SetupData.FilterFile);
else
  cfg_remove_key(cfgfile, "Global", "FilterFile");
if (SetupData.DriverPath)
  cfg_write_string(cfgfile, "Global", "DriverPath", SetupData.DriverPath);
else
  cfg_remove_key(cfgfile, "Global", "DriverPath");
if (SetupData.DriverFile)
  cfg_write_string(cfgfile, "Global", "DriverFile", SetupData.DriverFile);
else
  cfg_remove_key(cfgfile, "Global", "DriverFile");
if (SetupData.Snr)
cfg_write_string(cfgfile, "Global", "Snr", SetupData.Snr);
else
  cfg_remove_key(cfgfile, "Global", "Snr");
if (SetupData.ComDeviceName)
  cfg_write_string(cfgfile, "Global", "ComDeviceName", SetupData.ComDeviceName);
else
  cfg_remove_key(cfgfile, "Global", "ComDeviceName");
if (SetupData.DriverInitStr)
  cfg_write_string(cfgfile, "Global", "DriverInitStr", SetupData.DriverInitStr);
else
  cfg_remove_key(cfgfile, "Global", "DriverInitStr");
cfg_write_color(cfgfile, "Global", "WdgTimeColor", &SetupData.WdgTimeColor);
cfg_write_color(cfgfile, "Global", "WdgTypeColor", &SetupData.WdgTypeColor);
cfg_write_color(cfgfile, "Global", "WdgIdColor", &SetupData.WdgIdColor);
cfg_write_color(cfgfile, "Global", "WdgDlcColor", &SetupData.WdgDlcColor);
cfg_write_color(cfgfile, "Global", "WdgData1Color", &SetupData.WdgData1Color);
cfg_write_color(cfgfile, "Global", "WdgData2Color", &SetupData.WdgData2Color);
if (SetupData.WdgFontName)
  cfg_write_string(cfgfile, "Global", "WdgFontName", SetupData.WdgFontName);
else
  cfg_remove_key(cfgfile, "Global", "WdgFontName");
cfg_write_byte(cfgfile, "Global", "ShowTime", SetupData.ShowTime);
cfg_write_byte(cfgfile, "Global", "ShowDir", SetupData.ShowDir);
cfg_write_byte(cfgfile, "Global", "ShowType", SetupData.ShowType);
cfg_write_byte(cfgfile, "Global", "ShowDlc", SetupData.ShowDlc);
cfg_write_byte(cfgfile, "Global", "ShowData1", SetupData.ShowData1);
cfg_write_byte(cfgfile, "Global", "ShowData2", SetupData.ShowData2);
cfg_write_int(cfgfile, "Global", "SendenEntrysCount", SetupData.SendenEntrysCount);
cfg_write_int(cfgfile, "Global", "SendOnReturn", SetupData.SendOnReturn);
cfg_write_int(cfgfile, "Global", "FilterFrame", SetupData.FilterFrame);

if (cfg_write_file(cfgfile, filename) == FALSE)
  error = -1;
cfg_free(cfgfile);
save_free(filename);
if (error)
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error loading configuration file"));
return(error);
}


void SetSetup(void)
{
TinyCanConnect(0);
mhs_signal_emit(SIGC_PUBLIC, SIG_SETUP_CHANGE, NULL);
}


static void ModulInitCB(gulong signal, gpointer event_data, gpointer user_data)
{
if (CanModul.CanFeaturesFlags & CAN_FEATURE_TX_ACK)
  gtk_widget_set_sensitive(SetupWin->AnsichtWin->DirShowCheck, TRUE);
else
  {
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SetupWin->AnsichtWin->DirShowCheck), FALSE);
  gtk_widget_set_sensitive(SetupWin->AnsichtWin->DirShowCheck, FALSE);
  }
}


/**************************************************************/
/* Setup Dialog anzeigen                                      */
/**************************************************************/
void ExecuteSetup(void)
{
struct TCanSpeed can_speed;

SetupWin = ShowSetupDlg(0);
#ifndef __WIN32__
(void)g_signal_connect((gpointer)SetupWin->HardwareWin->PortSetupButton, "clicked", G_CALLBACK(on_PortSetupButton_clicked), NULL);
#endif
mhs_signal_connect(NULL, SIGC_PUBLIC, SIG_CAN_MODUL_INIT, ModulInitCB, NULL);
ModulInitCB(SIG_CAN_MODUL_INIT, NULL, NULL);
if (gtk_dialog_run(GTK_DIALOG(SetupWin->SetupDlg)) == GTK_RESPONSE_OK)
  {
  // ***** CAN Speed
  CanSpeedWinGet(SetupWin->CanSpeedWin, &can_speed);
  SetupData.CanSpeedMode = can_speed.Mode;
  SetupData.CanSpeed = can_speed.CanSpeed;
  SetupData.UserCanSpeed = can_speed.UserCanSpeed;
  SetupData.CanOpMode = can_speed.CanOpMode;
  // **** Daten Liste
  DataListWinGet(SetupWin->DataListWin);
  // **** Ansicht
  AnsichtWinGet(SetupWin->AnsichtWin);
  // **** Filter
  FilterWinGet(SetupWin->FilterWin);
  // **** Senden
  SendenWinGet(SetupWin->SendenWin);
  // **** Hardware
  HardwareWinGet(SetupWin->HardwareWin);
  // **** Treiber
  save_free(SetupData.DriverPath);
  SetupData.DriverPath = DriverWinGetPath(SetupWin->DriverWin);
  save_free(SetupData.DriverFile);
  SetupData.DriverFile = DriverWinGetFile(SetupWin->DriverWin);
  }
SetSetup();

CanSpeedWinDestroy(SetupWin->CanSpeedWin);
DataListWinDestroy(SetupWin->DataListWin);
AnsichtWinDestroy(SetupWin->AnsichtWin);
FilterWinDestroy(SetupWin->FilterWin);
SendenWinDestroy(SetupWin->SendenWin);
HardwareWinDestroy(SetupWin->HardwareWin);
DriverWinDestroy(SetupWin->DriverWin);
mhs_signal_disconnect(SIGC_PUBLIC, SIG_CAN_MODUL_INIT, ModulInitCB);
gtk_widget_destroy(SetupWin->SetupDlg);
SetupWin = NULL;
}


/*****************************/
/* Button callbacks          */
/*****************************/

// **** Port Setup Button
void on_PortSetupButton_clicked(GtkButton *button, gpointer user_data)
{
#ifndef __WIN32__
ExecutePortSetup();
#endif
}


/**************************************************************/
/* Tab: Daten Liste                                           */
/**************************************************************/
struct TDataListSetupWin *DataListWinNew(void)
{
GtkWidget *widget;
GtkWidget *vbox1;
GtkWidget *frame;
GtkWidget *hbox;
GtkWidget *hbox1;
struct TDataListSetupWin *data_list_win;

data_list_win = (struct TDataListSetupWin *)g_malloc0(sizeof(struct TDataListSetupWin));
if (!data_list_win)
  return(NULL);

data_list_win->Base = gtk_vbox_new (FALSE, 0);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(data_list_win->Base), hbox, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);

frame = gtk_frame_new (NULL);
gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
gtk_frame_set_label_align (GTK_FRAME (frame), 0.05, 0.5);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_container_add (GTK_CONTAINER (frame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 0, 7, 0);

vbox1 = gtk_vbox_new (FALSE, 0);
gtk_container_add (GTK_CONTAINER (widget), vbox1);

data_list_win->DatenListe[0] = gtk_radio_button_new_with_mnemonic (NULL, _("automatic clear"));
gtk_box_pack_start (GTK_BOX (vbox1), data_list_win->DatenListe[0], FALSE, FALSE, 0);
gtk_radio_button_set_group (GTK_RADIO_BUTTON (data_list_win->DatenListe[0]), data_list_win->DatenListe_group);
data_list_win->DatenListe_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (data_list_win->DatenListe[0]));

data_list_win->DatenListe[1] = gtk_radio_button_new_with_mnemonic (NULL, _("asking user"));
gtk_box_pack_start (GTK_BOX (vbox1), data_list_win->DatenListe[1], FALSE, FALSE, 0);
gtk_radio_button_set_group (GTK_RADIO_BUTTON (data_list_win->DatenListe[1]), data_list_win->DatenListe_group);
data_list_win->DatenListe_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (data_list_win->DatenListe[1]));

data_list_win->DatenListe[2] = gtk_radio_button_new_with_mnemonic (NULL, _("not clear"));
gtk_box_pack_start (GTK_BOX (vbox1), data_list_win->DatenListe[2], FALSE, FALSE, 0);
gtk_radio_button_set_group (GTK_RADIO_BUTTON (data_list_win->DatenListe[2]), data_list_win->DatenListe_group);
data_list_win->DatenListe_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (data_list_win->DatenListe[2]));

widget = gtk_label_new (_("Clear data..."));
gtk_frame_set_label_widget (GTK_FRAME (frame), widget);
gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);

hbox1 = gtk_hbox_new (FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), hbox1, TRUE, TRUE, 0);
gtk_container_set_border_width (GTK_CONTAINER (hbox1), 5);

widget = gtk_label_new (_("Buffer size: "));

gtk_box_pack_start (GTK_BOX (hbox1), widget, FALSE, FALSE, 0);

data_list_win->DatenLimitEdit_adj = gtk_adjustment_new (10, 10, 5000000, 1, 10, 0);
data_list_win->DatenLimitEdit = gtk_spin_button_new (GTK_ADJUSTMENT (data_list_win->DatenLimitEdit_adj), 1, 0);
gtk_box_pack_start (GTK_BOX (hbox1), data_list_win->DatenLimitEdit, FALSE, FALSE, 0);
gtk_widget_set_size_request (data_list_win->DatenLimitEdit, 100, -1);
gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (data_list_win->DatenLimitEdit), TRUE);

widget = gtk_label_new (_("(minimum 10)"));
gtk_box_pack_start (GTK_BOX (hbox1), widget, FALSE, FALSE, 0);

gtk_widget_show_all(data_list_win->Base);
DataListWinUpdate(data_list_win);

return(data_list_win);
}


void DataListWinDestroy(struct TDataListSetupWin *data_list_win)
{
if (data_list_win)
  {
  g_free(data_list_win);
  }
}


void DataListWinUpdate(struct TDataListSetupWin *data_list_win)
{
if (!data_list_win)
  return;
if (SetupData.DataClearMode > 2)
  SetupData.DataClearMode = 2;
gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data_list_win->DatenListe[SetupData.DataClearMode]), TRUE);
gtk_spin_button_set_value(GTK_SPIN_BUTTON(data_list_win->DatenLimitEdit), SetupData.RxDLimit);
}


void DataListWinGet(struct TDataListSetupWin *data_list_win)
{
int i;

if (!data_list_win)
  return;
for (i = 0; i < 3; i++)
  {
  if (GTK_TOGGLE_BUTTON(data_list_win->DatenListe[i])->active == TRUE)
    {
    SetupData.DataClearMode = i;
    break;
    }
  }
if (i == 3)
  SetupData.DataClearMode = 0;
SetupData.RxDLimit = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data_list_win->DatenLimitEdit));
}


/**************************************************************/
/* Tab: Ansicht                                               */
/**************************************************************/
struct TAnsichtSetupWin *AnsichtWinNew(void)
{
GtkWidget *widget;
GtkWidget *vbox1;
GtkWidget *frame;
GtkWidget *hbox;
GtkWidget *table;
GtkWidget *font_wdg;
struct TAnsichtSetupWin *ansicht_win;

ansicht_win = (struct TAnsichtSetupWin *)g_malloc0(sizeof(struct TAnsichtSetupWin));
if (!ansicht_win)
  return(NULL);

ansicht_win->Base = gtk_vbox_new (FALSE, 0);

frame = gtk_frame_new (NULL);
gtk_box_pack_start (GTK_BOX (ansicht_win->Base), frame, FALSE, FALSE, 0);
gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_container_add (GTK_CONTAINER (frame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 4, 4, 7, 4);

font_wdg = moo_font_button_new ();
gtk_container_add (GTK_CONTAINER(widget), font_wdg);
//moo_font_button_set_title(MOO_FONT_BUTTON(font_wdg), "");
moo_font_button_set_monospace(MOO_FONT_BUTTON(font_wdg), TRUE);
moo_font_button_set_filter_visible(MOO_FONT_BUTTON(font_wdg), TRUE);
moo_font_button_set_use_font (MOO_FONT_BUTTON(font_wdg), TRUE);
moo_font_button_set_use_size (MOO_FONT_BUTTON(font_wdg), TRUE);
ansicht_win->FontSel = font_wdg;

widget = gtk_label_new (_(" Font "));
gtk_frame_set_label_widget (GTK_FRAME (frame), widget);
gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);

frame = gtk_frame_new (NULL);
gtk_box_pack_start (GTK_BOX (ansicht_win->Base), frame, FALSE, FALSE, 0);
gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_container_add (GTK_CONTAINER (frame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 4, 4, 7, 4);

table = gtk_table_new (3, 4, FALSE);
gtk_container_add (GTK_CONTAINER (widget), table);
gtk_table_set_row_spacings (GTK_TABLE (table), 4);
gtk_table_set_col_spacings (GTK_TABLE (table), 10);

ansicht_win->TimeColorBtn = gtk_color_button_new ();
gtk_table_attach (GTK_TABLE (table), ansicht_win->TimeColorBtn, 1, 2, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);

ansicht_win->TypeColorBtn = gtk_color_button_new ();
gtk_table_attach (GTK_TABLE (table), ansicht_win->TypeColorBtn, 1, 2, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);

ansicht_win->IdColorBtn = gtk_color_button_new ();
gtk_table_attach (GTK_TABLE (table), ansicht_win->IdColorBtn, 1, 2, 2, 3,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);

widget = gtk_label_new (_("Time-stamp"));
gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new (_("Msg.-Type"));
gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new (_("Id"));
gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 2, 3,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

ansicht_win->DlcColorBtn = gtk_color_button_new ();
gtk_table_attach (GTK_TABLE (table), ansicht_win->DlcColorBtn, 3, 4, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);

ansicht_win->Data1ColorBtn = gtk_color_button_new ();
gtk_table_attach (GTK_TABLE (table), ansicht_win->Data1ColorBtn, 3, 4, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);

ansicht_win->Data2ColorBtn = gtk_color_button_new ();
gtk_table_attach (GTK_TABLE (table), ansicht_win->Data2ColorBtn, 3, 4, 2, 3,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);

widget = gtk_label_new (_("Dlc"));
gtk_table_attach (GTK_TABLE (table), widget, 2, 3, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new (_("Data 1"));
gtk_table_attach (GTK_TABLE (table), widget, 2, 3, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new (_("Data 2"));
gtk_table_attach (GTK_TABLE (table), widget, 2, 3, 2, 3,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new (_(" Colors "));
gtk_frame_set_label_widget (GTK_FRAME (frame), widget);
gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);

frame = gtk_frame_new (NULL);
gtk_box_pack_start (GTK_BOX (ansicht_win->Base), frame, FALSE, FALSE, 0);
gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_container_add (GTK_CONTAINER (frame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 4, 4, 7, 4);

vbox1 = gtk_vbox_new (FALSE, 5);
gtk_container_add (GTK_CONTAINER (widget), vbox1);

hbox = gtk_hbox_new (FALSE, 0);
gtk_box_pack_start (GTK_BOX (vbox1), hbox, TRUE, TRUE, 0);

widget = gtk_label_new (_("Time-stamp: "));
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

ansicht_win->TimeStampBox = gtk_combo_box_new_text ();
gtk_box_pack_start (GTK_BOX (hbox), ansicht_win->TimeStampBox, FALSE, FALSE, 0);
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->TimeStampBox), _("off"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->TimeStampBox), _("System time (absolute)"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->TimeStampBox), _("System time (relative)"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->TimeStampBox), _("System time (absolute, ms)"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->TimeStampBox), _("System time (relative, ms)"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->TimeStampBox), _("HW time stamp (absolute)"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->TimeStampBox), _("HW time stamp (relative)"));

hbox = gtk_hbox_new (FALSE, 5);
gtk_box_pack_start (GTK_BOX (vbox1), hbox, TRUE, TRUE, 0);

ansicht_win->DirShowCheck = gtk_check_button_new_with_mnemonic (_("Tx-Msg."));
gtk_box_pack_start(GTK_BOX(hbox), ansicht_win->DirShowCheck, FALSE, FALSE, 0);

ansicht_win->TypeShowCheck = gtk_check_button_new_with_mnemonic (_("Msg.-Type"));
gtk_box_pack_start(GTK_BOX(hbox), ansicht_win->TypeShowCheck, FALSE, FALSE, 0);

ansicht_win->DlcShowCheck = gtk_check_button_new_with_mnemonic (_("Dlc"));
gtk_box_pack_start(GTK_BOX(hbox), ansicht_win->DlcShowCheck, FALSE, FALSE, 0);

table = gtk_table_new (2, 2, FALSE);
gtk_box_pack_start (GTK_BOX (vbox1), table, TRUE, TRUE, 0);
gtk_table_set_row_spacings (GTK_TABLE (table), 5);
gtk_table_set_col_spacings (GTK_TABLE (table), 5);

widget = gtk_label_new (_("Data 1:"));
gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new (_("Data 2:"));
gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

ansicht_win->Data1Box = gtk_combo_box_new_text ();
gtk_table_attach (GTK_TABLE (table), ansicht_win->Data1Box, 1, 2, 0, 1,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (0), 0, 0);
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->Data1Box), _("Hide"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->Data1Box), _("Hex"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->Data1Box), _("Decimal"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->Data1Box), _("ASCII"));

ansicht_win->Data2Box = gtk_combo_box_new_text ();
gtk_table_attach (GTK_TABLE (table), ansicht_win->Data2Box, 1, 2, 1, 2,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (0), 0, 0);
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->Data2Box), _("Hide"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->Data2Box), _("Hex"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->Data2Box), _("Decimal"));
gtk_combo_box_append_text (GTK_COMBO_BOX (ansicht_win->Data2Box), _("ASCII"));

widget = gtk_label_new (_(" Columns "));
gtk_frame_set_label_widget (GTK_FRAME (frame), widget);
gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);

gtk_widget_show_all(ansicht_win->Base);
AnsichtWinUpdate(ansicht_win);

return(ansicht_win);
}


void AnsichtWinDestroy(struct TAnsichtSetupWin *ansicht_win)
{
if (ansicht_win)
  {
  g_free(ansicht_win);
  }
}


void AnsichtWinUpdate(struct TAnsichtSetupWin *ansicht_win)
{
if (!ansicht_win)
  return;
if (SetupData.WdgFontName)
  moo_font_button_set_font_name(MOO_FONT_BUTTON(ansicht_win->FontSel), SetupData.WdgFontName);
gtk_color_button_set_color(GTK_COLOR_BUTTON(ansicht_win->TimeColorBtn), &SetupData.WdgTimeColor);
gtk_color_button_set_color(GTK_COLOR_BUTTON(ansicht_win->TypeColorBtn), &SetupData.WdgTypeColor);
gtk_color_button_set_color(GTK_COLOR_BUTTON(ansicht_win->IdColorBtn), &SetupData.WdgIdColor);
gtk_color_button_set_color(GTK_COLOR_BUTTON(ansicht_win->DlcColorBtn), &SetupData.WdgDlcColor);
gtk_color_button_set_color(GTK_COLOR_BUTTON(ansicht_win->Data1ColorBtn), &SetupData.WdgData1Color);
gtk_color_button_set_color(GTK_COLOR_BUTTON(ansicht_win->Data2ColorBtn), &SetupData.WdgData2Color);
gtk_combo_box_set_active(GTK_COMBO_BOX(ansicht_win->TimeStampBox), SetupData.ShowTime);
if (SetupData.ShowDir)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ansicht_win->DirShowCheck), TRUE);
else
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ansicht_win->DirShowCheck), FALSE);
if (SetupData.ShowType)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ansicht_win->TypeShowCheck), TRUE);
else
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ansicht_win->TypeShowCheck), FALSE);
if (SetupData.ShowDlc)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ansicht_win->DlcShowCheck), TRUE);
else
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ansicht_win->DlcShowCheck), FALSE);
gtk_combo_box_set_active(GTK_COMBO_BOX(ansicht_win->Data1Box), SetupData.ShowData1);
gtk_combo_box_set_active(GTK_COMBO_BOX(ansicht_win->Data2Box), SetupData.ShowData2);
}


void AnsichtWinGet(struct TAnsichtSetupWin *ansicht_win)
{
gchar *font_name;

if (!ansicht_win)
  return;
font_name = (gchar *)moo_font_button_get_font_name(MOO_FONT_BUTTON(ansicht_win->FontSel));
save_free(SetupData.WdgFontName);
if (font_name)
  SetupData.WdgFontName = g_strdup(font_name);
else
  SetupData.WdgFontName = GetDefaultFont(MainWin.MainWin, 1);
gtk_color_button_get_color(GTK_COLOR_BUTTON(ansicht_win->TimeColorBtn), &SetupData.WdgTimeColor);
gtk_color_button_get_color(GTK_COLOR_BUTTON(ansicht_win->TypeColorBtn), &SetupData.WdgTypeColor);
gtk_color_button_get_color(GTK_COLOR_BUTTON(ansicht_win->IdColorBtn), &SetupData.WdgIdColor);
gtk_color_button_get_color(GTK_COLOR_BUTTON(ansicht_win->DlcColorBtn), &SetupData.WdgDlcColor);
gtk_color_button_get_color(GTK_COLOR_BUTTON(ansicht_win->Data1ColorBtn), &SetupData.WdgData1Color);
gtk_color_button_get_color(GTK_COLOR_BUTTON(ansicht_win->Data2ColorBtn), &SetupData.WdgData2Color);
SetupData.ShowTime = gtk_combo_box_get_active(GTK_COMBO_BOX(ansicht_win->TimeStampBox));
if (GTK_TOGGLE_BUTTON(ansicht_win->DirShowCheck)->active == TRUE)
  SetupData.ShowDir = 1;
else
  SetupData.ShowDir = 0;
if (GTK_TOGGLE_BUTTON(ansicht_win->TypeShowCheck)->active == TRUE)
  SetupData.ShowType = 1;
else
  SetupData.ShowType = 0;
if (GTK_TOGGLE_BUTTON(ansicht_win->DlcShowCheck)->active == TRUE)
  SetupData.ShowDlc = 1;
else
  SetupData.ShowDlc = 0;
SetupData.ShowData1 = gtk_combo_box_get_active(GTK_COMBO_BOX(ansicht_win->Data1Box));
SetupData.ShowData2 = gtk_combo_box_get_active(GTK_COMBO_BOX(ansicht_win->Data2Box));
}


/**************************************************************/
/* Tab: Filter                                                */
/**************************************************************/
struct TFilterSetupWin *FilterWinNew(void)
{
GtkWidget *widget;
GtkWidget *vbox1;
GtkWidget *frame;
struct TFilterSetupWin *filter_win;

filter_win = (struct TFilterSetupWin *)g_malloc0(sizeof(struct TFilterSetupWin));
if (!filter_win)
  return(NULL);

filter_win->Base = gtk_vbox_new (FALSE, 0);

frame = gtk_frame_new (NULL);
gtk_box_pack_start (GTK_BOX (filter_win->Base), frame, FALSE, FALSE, 0);
gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
gtk_frame_set_label_align (GTK_FRAME (frame), 0.05, 0.5);

widget = gtk_label_new (_("Reading filter messages interval"));
gtk_frame_set_label_widget (GTK_FRAME (frame), widget);
gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_container_add (GTK_CONTAINER (frame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 0, 7, 0);

vbox1 = gtk_vbox_new (FALSE, 0);
gtk_container_add (GTK_CONTAINER (widget), vbox1);

filter_win->FilterIntervall[0] = gtk_radio_button_new_with_mnemonic (NULL, _("100 ms"));
gtk_box_pack_start (GTK_BOX (vbox1), filter_win->FilterIntervall[0], FALSE, FALSE, 0);
gtk_radio_button_set_group (GTK_RADIO_BUTTON (filter_win->FilterIntervall[0]), filter_win->FilterIntervall_group);
filter_win->FilterIntervall_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (filter_win->FilterIntervall[0]));

filter_win->FilterIntervall[1] = gtk_radio_button_new_with_mnemonic (NULL, _("250 ms"));
gtk_box_pack_start (GTK_BOX (vbox1), filter_win->FilterIntervall[1], FALSE, FALSE, 0);
gtk_radio_button_set_group (GTK_RADIO_BUTTON (filter_win->FilterIntervall[1]), filter_win->FilterIntervall_group);
filter_win->FilterIntervall_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (filter_win->FilterIntervall[1]));

filter_win->FilterIntervall[2] = gtk_radio_button_new_with_mnemonic (NULL, _("500 ms"));
gtk_box_pack_start (GTK_BOX (vbox1), filter_win->FilterIntervall[2], FALSE, FALSE, 0);
gtk_radio_button_set_group (GTK_RADIO_BUTTON (filter_win->FilterIntervall[2]), filter_win->FilterIntervall_group);
filter_win->FilterIntervall_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (filter_win->FilterIntervall[2]));

filter_win->FilterIntervall[3] = gtk_radio_button_new_with_mnemonic (NULL, _("1 s"));
gtk_box_pack_start (GTK_BOX (vbox1), filter_win->FilterIntervall[3], FALSE, FALSE, 0);
gtk_radio_button_set_group (GTK_RADIO_BUTTON (filter_win->FilterIntervall[3]), filter_win->FilterIntervall_group);
filter_win->FilterIntervall_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (filter_win->FilterIntervall[3]));

gtk_widget_show_all(filter_win->Base);
FilterWinUpdate(filter_win);

return(filter_win);
}


void FilterWinDestroy(struct TFilterSetupWin *filter_win)
{
if (filter_win)
  {
  g_free(filter_win);
  }
}


void FilterWinUpdate(struct TFilterSetupWin *filter_win)
{
if (!filter_win)
  return;
if (SetupData.FilterTimer > 3)
  SetupData.FilterTimer = 3;
gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(filter_win->FilterIntervall[SetupData.FilterTimer]), TRUE);
}


void FilterWinGet(struct TFilterSetupWin *filter_win)
{
int i;

if (!filter_win)
  return;
for (i = 0; i < 4; i++)
  {
  if (GTK_TOGGLE_BUTTON(filter_win->FilterIntervall[i])->active == TRUE)
    {
    SetupData.FilterTimer = i;
    break;
    }
  }
if (i == 4)
  SetupData.FilterTimer = 0;
}


/**************************************************************/
/* Tab: Senden                                                */
/**************************************************************/
struct TSendenSetupWin *SendenWinNew(void)
{
GtkWidget *widget;
GtkWidget *hbox;
struct TSendenSetupWin *senden_win;

senden_win = (struct TSendenSetupWin *)g_malloc0(sizeof(struct TSendenSetupWin));
if (!senden_win)
  return(NULL);

senden_win->Base = gtk_vbox_new(FALSE, 5);
gtk_container_set_border_width (GTK_CONTAINER (senden_win->Base), 5);

hbox = gtk_hbox_new (FALSE, 3);
gtk_box_pack_start (GTK_BOX (senden_win->Base), hbox, FALSE, FALSE, 0);

widget = gtk_label_new(_("Numer of transmissen bars in the window"));
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

senden_win->TxEntryCountEdit_adj = gtk_adjustment_new (1, 0, 10, 1, 5, 0);
senden_win->TxEntryCountEdit = gtk_spin_button_new (GTK_ADJUSTMENT (senden_win->TxEntryCountEdit_adj), 1, 0);
gtk_box_pack_start (GTK_BOX (hbox), senden_win->TxEntryCountEdit, FALSE, FALSE, 0);
gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (senden_win->TxEntryCountEdit), TRUE);
gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (senden_win->TxEntryCountEdit), GTK_UPDATE_IF_VALID);

senden_win->SendOnReturnCheck = gtk_check_button_new_with_mnemonic (_("Transmit message if press \"RETURN\" key"));
gtk_box_pack_start (GTK_BOX (senden_win->Base), senden_win->SendOnReturnCheck, FALSE, FALSE, 0);

gtk_widget_show_all(senden_win->Base);
SendenWinUpdate(senden_win);

return(senden_win);
}


void SendenWinDestroy(struct TSendenSetupWin *senden_win)
{
if (senden_win)
  {
  g_free(senden_win);
  }
}


void SendenWinUpdate(struct TSendenSetupWin *senden_win)
{
if (!senden_win)
  return;
gtk_spin_button_set_value(GTK_SPIN_BUTTON(senden_win->TxEntryCountEdit), SetupData.SendenEntrysCount);
if (SetupData.SendOnReturn)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(senden_win->SendOnReturnCheck), TRUE);
else
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(senden_win->SendOnReturnCheck), FALSE);
}


void SendenWinGet(struct TSendenSetupWin *senden_win)
{
if (!senden_win)
  return;
SetupData.SendenEntrysCount = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(senden_win->TxEntryCountEdit));
if (GTK_TOGGLE_BUTTON(senden_win->SendOnReturnCheck)->active == TRUE)
  SetupData.SendOnReturn = 1;
else
  SetupData.SendOnReturn = 0;
}


/**************************************************************/
/* Tab: Hardware                                              */
/**************************************************************/
#ifdef __WIN32__
static const unsigned long BaudRateTab[] = {   4800,
                                               9600,
                                              14400,
                                              19200,
                                              28800,
                                              38400,
                                              57600,
                                             115200,
                                              10400,
                                             125000,
                                             153600,
                                             230400,
                                             250000,
                                             460800,
                                             500000,
                                             921600,
                                            1000000,
                                            0};

static int BaudToIndex(unsigned long in)
{
int idx;
unsigned long *b;
unsigned long baud;

idx = 0;
for (b = (unsigned long *)BaudRateTab; (baud = *b); b++)
  {
  if (baud ==  in)
    break;
  idx++;
  }
return(idx);
}


static unsigned long IndexToBaud(int index)
{
if ((index >= 17) || (index < 0))
  index = 0;
return(BaudRateTab[index]);
}
#endif


struct THardwareSetupWin *HardwareWinNew(void)
{
GtkWidget *widget, *hbox;
#ifdef __WIN32__
GtkWidget *table;
#else
GtkWidget *hbuttonbox;
#endif
struct THardwareSetupWin *hardware_win;

hardware_win = (struct THardwareSetupWin *)g_malloc0(sizeof(struct THardwareSetupWin));
if (!hardware_win)
  return(NULL);

hardware_win->Base = gtk_vbox_new (FALSE, 5);
gtk_container_set_border_width (GTK_CONTAINER (hardware_win->Base), 5);

hardware_win->AutoConnectSwitch = gtk_check_button_new_with_mnemonic (_("Connect to Tiny-CAN automatically"));
gtk_box_pack_start (GTK_BOX (hardware_win->Base), hardware_win->AutoConnectSwitch, FALSE, FALSE, 0);
gtk_container_set_border_width (GTK_CONTAINER (hardware_win->AutoConnectSwitch), 5);

hardware_win->SnrFrame = gtk_frame_new (NULL);
gtk_box_pack_start (GTK_BOX (hardware_win->Base), hardware_win->SnrFrame, FALSE, FALSE, 0);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_container_add (GTK_CONTAINER (hardware_win->SnrFrame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 4, 4, 12, 4);

hbox = gtk_hbox_new (FALSE, 3);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_label_new (_("Tiny-CAN serial number"));
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

hardware_win->SnrEdit = gtk_entry_new ();
gtk_box_pack_start (GTK_BOX (hbox), hardware_win->SnrEdit, FALSE, FALSE, 0);
gtk_entry_set_max_length (GTK_ENTRY (hardware_win->SnrEdit), 8);
gtk_entry_set_width_chars (GTK_ENTRY (hardware_win->SnrEdit), 10);

hardware_win->SnrEnableSwitch = gtk_check_button_new_with_mnemonic (_("Use serial number"));
gtk_frame_set_label_widget (GTK_FRAME (hardware_win->SnrFrame), hardware_win->SnrEnableSwitch);

#ifdef __WIN32__

hardware_win->ComDrvMode[1] = gtk_radio_button_new_with_mnemonic (NULL, _("USB"));
gtk_radio_button_set_group (GTK_RADIO_BUTTON (hardware_win->ComDrvMode[1]), hardware_win->ComDrvMode_group);
hardware_win->ComDrvMode_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (hardware_win->ComDrvMode[1]));
gtk_box_pack_start(GTK_BOX(hardware_win->Base), hardware_win->ComDrvMode[1], FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(hardware_win->ComDrvMode[1]), 5);

hardware_win->ComDrvMode[0] = gtk_radio_button_new_with_mnemonic (NULL, _("RS232"));
gtk_radio_button_set_group(GTK_RADIO_BUTTON (hardware_win->ComDrvMode[0]), hardware_win->ComDrvMode_group);
hardware_win->ComDrvMode_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (hardware_win->ComDrvMode[0]));
//gtk_box_pack_start (GTK_BOX (vbox1), hardware_win->ComDrvMode[0], FALSE, FALSE, 0);

hardware_win->Rs232Frame = gtk_frame_new (NULL);
gtk_box_pack_start(GTK_BOX(hardware_win->Base), hardware_win->Rs232Frame, FALSE, FALSE, 0);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_container_add (GTK_CONTAINER (hardware_win->Rs232Frame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT(widget), 4, 4, 12, 4);
gtk_frame_set_label_widget(GTK_FRAME(hardware_win->Rs232Frame), hardware_win->ComDrvMode[0]);

table = gtk_table_new (2, 2, FALSE);
gtk_container_add (GTK_CONTAINER (widget), table);

gtk_table_set_row_spacings (GTK_TABLE (table), 5);
gtk_table_set_col_spacings (GTK_TABLE (table), 5);

widget = gtk_label_new(_("COM Port:"));
gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);

widget = gtk_label_new(_("Baudrate:"));
gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);

hardware_win->PortBox = gtk_combo_box_new_text ();
gtk_table_attach (GTK_TABLE (table), hardware_win->PortBox, 1, 2, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);
gtk_combo_box_append_text(GTK_COMBO_BOX(hardware_win->PortBox), _("1"));
gtk_combo_box_append_text(GTK_COMBO_BOX(hardware_win->PortBox), _("2"));
gtk_combo_box_append_text(GTK_COMBO_BOX(hardware_win->PortBox), _("3"));
gtk_combo_box_append_text(GTK_COMBO_BOX(hardware_win->PortBox), _("4"));
gtk_combo_box_append_text(GTK_COMBO_BOX(hardware_win->PortBox), _("5"));
gtk_combo_box_append_text(GTK_COMBO_BOX(hardware_win->PortBox), _("6"));
gtk_combo_box_append_text(GTK_COMBO_BOX(hardware_win->PortBox), _("7"));
gtk_combo_box_append_text(GTK_COMBO_BOX(hardware_win->PortBox), _("8"));

hardware_win->BaudRateBox = gtk_combo_box_new_text ();
gtk_table_attach (GTK_TABLE (table), hardware_win->BaudRateBox, 1, 2, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("  4800 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("  9600 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _(" 14400 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _(" 19200 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _(" 28800 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _(" 38400 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _(" 57600 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("115200 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _(" 10400 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("125000 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("153600 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("230400 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("250000 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("460800 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("500000 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("921600 Baud"));
gtk_combo_box_append_text(GTK_COMBO_BOX (hardware_win->BaudRateBox), _("1 M Baud   "));

// **** Port setup botton
#else
hbuttonbox = gtk_hbutton_box_new ();
gtk_box_pack_start (GTK_BOX (hardware_win->Base), hbuttonbox, FALSE, FALSE, 0);
gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox), 5);

hardware_win->PortSetupButton = gtk_button_new ();
gtk_container_add (GTK_CONTAINER (hbuttonbox), hardware_win->PortSetupButton);
GTK_WIDGET_SET_FLAGS (hardware_win->PortSetupButton, GTK_CAN_DEFAULT);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_container_add (GTK_CONTAINER (hardware_win->PortSetupButton), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-disconnect", GTK_ICON_SIZE_BUTTON);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic (_("Port setup"));
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
gtk_misc_set_padding (GTK_MISC (widget), 5, 0);
#endif

hbox = gtk_hbox_new(FALSE, 5);
gtk_box_pack_start(GTK_BOX(hardware_win->Base), hbox, FALSE, FALSE, 0);

widget = gtk_label_new (_("Driver Options:"));
gtk_box_pack_start(GTK_BOX (hbox), widget, FALSE, FALSE, 0);

hardware_win->DriverInitStrEdit = gtk_entry_new ();
gtk_box_pack_start(GTK_BOX(hbox), hardware_win->DriverInitStrEdit, TRUE, TRUE, 0);

gtk_widget_show_all(hardware_win->Base);
HardwareWinUpdate(hardware_win);

return(hardware_win);
}


void HardwareWinDestroy(struct THardwareSetupWin *hardware_win)
{
if (hardware_win)
  {
  g_free(hardware_win);
  }
}


void HardwareWinUpdate(struct THardwareSetupWin *hardware_win)
{
char *s, *str;

if (!hardware_win)
  return;
if (SetupData.AutoConnect)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hardware_win->AutoConnectSwitch), TRUE);
else
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hardware_win->AutoConnectSwitch), FALSE);
if (SetupData.Snr)
  {
  str = g_strdup(SetupData.Snr);
  s = str;
  g_strstrip(s);
  if (strlen(s))
    gtk_entry_set_text(GTK_ENTRY(hardware_win->SnrEdit), s);
  else
    SetupData.SnrEnabled = 0;
  save_free(str);
  }
else
  SetupData.SnrEnabled = 0;
if (SetupData.SnrEnabled)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hardware_win->SnrEnableSwitch), TRUE);
else
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hardware_win->SnrEnableSwitch), FALSE);

#ifdef __WIN32__
if (SetupData.ComDrvMode > 1)
  SetupData.ComDrvMode = 1;
gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hardware_win->ComDrvMode[SetupData.ComDrvMode]), TRUE);

gtk_combo_box_set_active(GTK_COMBO_BOX(hardware_win->PortBox), SetupData.PortIndex-1);
gtk_combo_box_set_active(GTK_COMBO_BOX(hardware_win->BaudRateBox), BaudToIndex(SetupData.BaudRate));
#endif

if (SetupData.DriverInitStr)
  gtk_entry_set_text(GTK_ENTRY(hardware_win->DriverInitStrEdit), SetupData.DriverInitStr);
}


void HardwareWinGet(struct THardwareSetupWin *hardware_win)
{
#ifdef __WIN32__
int i;
#endif
char *s, *str;

if (!hardware_win)
  return;
if (GTK_TOGGLE_BUTTON(hardware_win->AutoConnectSwitch)->active == TRUE)
  SetupData.AutoConnect = 1;
else
  SetupData.AutoConnect = 0;
save_free(SetupData.Snr);
str = g_strdup(gtk_entry_get_text(GTK_ENTRY(hardware_win->SnrEdit)));
s = str;
g_strstrip(s);
if (strlen(s))
  SetupData.Snr = g_strdup(s);
save_free(str);
if ((GTK_TOGGLE_BUTTON(hardware_win->SnrEnableSwitch)->active == TRUE) && (SetupData.Snr))
  SetupData.SnrEnabled = 1;
else
  SetupData.SnrEnabled = 0;

#ifdef __WIN32__
for (i = 0; i < 2; i++)
  {
  if (GTK_TOGGLE_BUTTON(hardware_win->ComDrvMode[i])->active == TRUE)
    {
    SetupData.ComDrvMode = i;
    break;
    }
  }
if (i == 2)
  SetupData.ComDrvMode = 0;
SetupData.PortIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(hardware_win->PortBox)) + 1;
SetupData.BaudRate = IndexToBaud(gtk_combo_box_get_active(GTK_COMBO_BOX(hardware_win->BaudRateBox)));
#endif

save_free(SetupData.DriverInitStr);
str = g_strdup(gtk_entry_get_text(GTK_ENTRY(hardware_win->DriverInitStrEdit)));
s = str;
g_strstrip(s);
if (strlen(s))
  SetupData.DriverInitStr = g_strdup(s);
save_free(str);
}


struct TSetupWin *ShowSetupDlg(unsigned long showing_pages)
{
GtkWidget *widget;
GtkWidget *hbox;
GtkWidget *dialog_vbox;
GtkWidget *notebook;
GtkWidget *dialog_action_area;
GtkWidget *page;
struct TCanSpeed can_speed;
struct TSetupWin *setup_win;


setup_win = (struct TSetupWin *)g_malloc0(sizeof(struct TSetupWin));
if (!setup_win)
  return(NULL);

// **** Dialog erzeugen
setup_win->SetupDlg = gtk_dialog_new();
gtk_window_set_title(GTK_WINDOW(setup_win->SetupDlg), _("Setup"));
gtk_window_set_position(GTK_WINDOW(setup_win->SetupDlg), GTK_WIN_POS_CENTER);
gtk_window_set_resizable(GTK_WINDOW(setup_win->SetupDlg), FALSE);
gtk_window_set_type_hint(GTK_WINDOW(setup_win->SetupDlg), GDK_WINDOW_TYPE_HINT_DIALOG);
//gtk_window_set_modal(GTK_WINDOW(setup_win->SetupDlg), TRUE);

dialog_vbox = GTK_DIALOG(setup_win->SetupDlg)->vbox;
gtk_widget_show (dialog_vbox);

// **** Notebook erzeugen
notebook = gtk_notebook_new ();
gtk_widget_show(notebook);
gtk_box_pack_start(GTK_BOX(dialog_vbox), notebook, TRUE, TRUE, 0);

if (!showing_pages)
  showing_pages = 0xFFFFFFFF;

if (showing_pages & SETUP_SHOW_CAN_SPEED)
  {
  can_speed.Mode = SetupData.CanSpeedMode;
  can_speed.CanSpeed = SetupData.CanSpeed;
  can_speed.UserCanSpeed = SetupData.UserCanSpeed;
  can_speed.CanOpMode = SetupData.CanOpMode;
  setup_win->CanSpeedWin = CanSpeedWinNew(&can_speed);
  page = setup_win->CanSpeedWin->Base;
  widget = gtk_label_new(_("CAN"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, widget);
  }
if (showing_pages & SETUP_SHOW_DATA_LIST)
  {
  setup_win->DataListWin = DataListWinNew();
  page = setup_win->DataListWin->Base;
  widget = gtk_label_new(_("Data window"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, widget);
  }
if (showing_pages & SETUP_SHOW_ANSICHT)
  {
  setup_win->AnsichtWin = AnsichtWinNew();
  page = setup_win->AnsichtWin->Base;
  widget = gtk_label_new(_("View"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, widget);
  }
if (showing_pages & SETUP_SHOW_FILTER)
  {
  setup_win->FilterWin = FilterWinNew();
  page = setup_win->FilterWin->Base;
  widget = gtk_label_new(_("Filter"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, widget);
  }
if (showing_pages & SETUP_SHOW_SENDEN)
  {
  setup_win->SendenWin = SendenWinNew();
  page = setup_win->SendenWin->Base;
  widget = gtk_label_new(_("Transmit"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, widget);
  }
if (showing_pages & SETUP_SHOW_HARDWARE)
  {
  setup_win->HardwareWin = HardwareWinNew();
  page = setup_win->HardwareWin->Base;
  widget = gtk_label_new(_("Hardware"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, widget);
  }
if (showing_pages & SETUP_SHOW_DRIVER)
  {
  setup_win->DriverWin = DriverWinNew(SetupData.DriverPath, SetupData.DriverFile,
     DRV_WIN_SHOW_PATH | DRV_WIN_SHOW_ERRORS);
  page = setup_win->DriverWin->Base;
  widget = gtk_label_new(_("Driver"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, widget);
  }

dialog_action_area = GTK_DIALOG (setup_win->SetupDlg)->action_area;
gtk_widget_show (dialog_action_area);
gtk_button_box_set_layout (GTK_BUTTON_BOX(dialog_action_area), GTK_BUTTONBOX_END);

// **** Cancel Button
setup_win->CancelButton = gtk_button_new ();
gtk_widget_show (setup_win->CancelButton);
gtk_dialog_add_action_widget (GTK_DIALOG (setup_win->SetupDlg), setup_win->CancelButton, GTK_RESPONSE_CANCEL);
GTK_WIDGET_SET_FLAGS (setup_win->CancelButton, GTK_CAN_DEFAULT);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (setup_win->CancelButton), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-cancel", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic(_("_Cancel"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
// **** Ok Button
setup_win->OkButton = gtk_button_new ();
gtk_widget_show (setup_win->OkButton);
gtk_dialog_add_action_widget (GTK_DIALOG (setup_win->SetupDlg), setup_win->OkButton, GTK_RESPONSE_OK);
GTK_WIDGET_SET_FLAGS (setup_win->OkButton, GTK_CAN_DEFAULT);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (setup_win->OkButton), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-ok", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic (_("_Ok"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

return(setup_win);
}
