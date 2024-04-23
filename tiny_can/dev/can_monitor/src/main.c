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
/*                             M A I N                                    */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : Hauptrogramm Tiny-CAN Monitor                       */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 17.10.05                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include "can_monitor.h"
#include <stdlib.h>
#include <string.h>
#include "dialogs.h"
#include "about_dlg.h"
#include "filter_win.h"
#include "makro.h"
#include "makro_win.h"
#include "setup.h"
#include "file_sel.h"
#include "const.h"
#include "drv_info.h"
#include "datei_info.h"
#include "splash.h"
#include "main_can.h"
#include "can_widget.h"
#include "cmdline.h"
#include "plugin.h"
#include "plugin_win.h"
#include "const.h"
#include "start_win.h"
#include "support.h"
#include "mhs_obj.h"
#include "can_monitor_obj.h"

struct TMainStatus
  {
  GtkWidget *Widget;
  gchar *CanSpeedStr;
  gchar *CanStatusStr;
  gchar *CanRecordStr;
  gchar *LargeMessageStr;
  gchar *MessageStr;
  };



struct TMhsSignal *MhsCanViewSignals = NULL;
struct TCanSpeedList *UserCanSpeedList = NULL;

struct TMainStatus MainStatus;
struct TMainWin MainWin;

#define CWDG_FLAGS CAN_TX_WDG_SEND_BTN | CAN_TX_WDG_EDIT_MODE | CAN_TX_WDG_ENABLE_RETURN_SEND | CAN_TX_WDG_DISPLAY_ENABLE

/*struct TMhsSignal *ProtectedSignals = NULL;
struct TMhsSignal *PublicSignals = NULL;
struct TMhsSignal *CanMsgSignals = NULL;*/

//struct TCanView *CanView = NULL;
//struct TCanView *CanFilterView = NULL;
struct TMakroTableWidget *MainMakroView = NULL;
struct TCanTxWidget *SendenCanWdg[10];

struct TMainVar MainVar;
unsigned short MainEventsMask;
//unsigned int ProgAktion;
unsigned int DataRecord;

struct TCanMsg DefaultCanMsg;
struct TCanMsg LastTransmit;

char DataFile[255];

void ExitApplikation(void);
void SendenButtonCB(struct TCanTxWidget *ctw, int event, struct TCanMsg *can_msg);
void MainMakroRepaintCB(gulong signal, gpointer event_data, gpointer user_data);
void MainMakroRepaintItemCB(gulong signal, gpointer event_data, gpointer user_data);
void FilterEditFinishEventCB(gulong signal, gpointer event_data, gpointer user_data);
void MainMakroViewSelectCB(struct TMakroTableWidget *mtw, int event, struct TMakro *makro);
void CanStatusChangeCB(gulong signal, gpointer event_data, gpointer user_data);
void DataRecordChangeCB(gulong signal, gpointer event_data, gpointer user_data);
void SetupChangeCB(gulong signal, gpointer event_data, gpointer user_data);
void MainShowingChangeCB(gulong signal, gpointer event_data, gpointer user_data);
void SetSymbolLeisten(void);
void SetSendOnReturn(void);
void SetFilterWindow(void);

void on_MNeu_activate(GtkObject * object, gpointer user_data);
void on_MOpen_activate(GtkObject * object, gpointer user_data);
void on_MSave_activate(GtkObject *object, gpointer user_data);
void on_MDateiInfo_activate(GtkObject * object, gpointer user_data);
void on_MBeenden_activate(GtkObject * object, gpointer user_data);
void on_MCanStartStop_activate(GtkObject * object, gpointer user_data);
void on_MCanReset_activate(GtkObject *object, gpointer user_data);
void on_MMakBearbeiten_activate(GtkCheckMenuItem *object, gpointer user_data);
void on_MMakOpen_activate(GtkObject * object, gpointer user_data);
void on_MMakSave_activate(GtkObject * object, gpointer user_data);
void on_MFilMessages_activate(GtkCheckMenuItem *object, gpointer user_data);
void on_MFilBearbeiten_activate(GtkObject * object, gpointer user_data);
void on_MFilOpen_activate(GtkObject * object, gpointer user_data);
void on_MFilSave_activate(GtkObject * object, gpointer user_data);
void on_MStandartLeiste_activate(GtkCheckMenuItem *object, gpointer user_data);
void on_MSendenLeiste_activate(GtkCheckMenuItem *object, gpointer user_data);
void on_MMakroLeiste_activate(GtkCheckMenuItem *object, gpointer user_data);
void on_MButtonsText_activate(GtkCheckMenuItem *object, gpointer user_data);
void on_MEinstellungen_activate(GtkObject * object, gpointer user_data);
void on_MPluginsSetup_activate(GtkObject * object, gpointer user_data);
void on_MConnect_activate(GtkObject * object, gpointer user_data);
void on_MDeviceInfo_activate(GtkObject * object, gpointer user_data);
void on_MInfo_activate(GtkObject * object, gpointer user_data);

void on_NeuButton_clicked(GtkButton *button, gpointer user_data);
void on_FileOpenButton_clicked(GtkButton *button, gpointer user_data);
void on_FileSaveButton_clicked(GtkButton *button, gpointer user_data);
void on_BeendenButton_clicked(GtkButton *button, gpointer user_data);
void on_CanStartStopButton_clicked(GtkButton *button, gpointer user_data);
void on_CanResetButton_clicked(GtkButton *button, gpointer user_data);
void on_EinstellungenButton_clicked(GtkButton *button, gpointer user_data);
void on_MakroPasteButton_clicked(GtkButton *button, gpointer user_data);
void on_MakroSendenButton_clicked(GtkButton *button, gpointer user_data);

void CreateMainWin(void);
void ShowMainWin(void);


void MainSetSensitive(void)
{
unsigned char *sen_list;

if (CanModul.DeviceStatus.DrvStatus == DRV_NOT_LOAD)
  sen_list = &SelectAllCanOffListe[0];
else if (CanModul.DeviceStatus.DrvStatus != DRV_STATUS_CAN_RUN)
  sen_list = &SelectCanOffListe[0];
else
  {
  if ((MainMakroView) && (MainMakroView->SelectedRow))
    sen_list = &SelectAllOnListe[0];
  else
    sen_list = &SelectAllOnMakroOffListe[0];
  }
SetSensitiveList(SensitiveListe, sen_list);
}


/**************************************************************/
/* Programm Status anpassen                                   */
/**************************************************************/
void MainStatusCreate(void)
{
MainStatus.Widget = MainWin.Statusbar;
MainStatus.CanSpeedStr = g_strdup("?");
MainStatus.CanStatusStr = g_strdup(_("CAN: unknown"));
MainStatus.CanRecordStr = g_strdup(_("STOP"));
MainStatus.LargeMessageStr = g_strdup(_("?"));
MainStatus.MessageStr = g_strdup(_("Tiny-CAN connected"));
}


void MainStatusDestroy(void)
{
save_free(MainStatus.CanSpeedStr);
save_free(MainStatus.CanStatusStr);
save_free(MainStatus.CanRecordStr);
save_free(MainStatus.LargeMessageStr);
save_free(MainStatus.MessageStr);
}


void StatusSetSpeed(void)
{
unsigned int speed;
char *str;

if (MainStatus.CanSpeedStr)
  g_free(MainStatus.CanSpeedStr);
if (SetupData.CanSpeedMode)
  {
  str = CanSpeedDBGetShortDesciption(UserCanSpeedList, SetupData.UserCanSpeed);
  if (str)
    MainStatus.CanSpeedStr = g_strdup(str);
  else
    MainStatus.CanSpeedStr = g_strdup("?");
  }
else
  {
  speed = (unsigned int)SetupData.CanSpeed;
  if (!speed)
    MainStatus.CanSpeedStr = g_strdup("?");
  else if (speed < 1000)
    MainStatus.CanSpeedStr = g_strdup_printf("%u kBit/s", speed);
  else if (speed == 1000)
    MainStatus.CanSpeedStr = g_strdup("1 MBit/s");
  else
    MainStatus.CanSpeedStr = g_strdup_printf("%1.2fMBit/s", (float)speed / (float)1000);
  }
}


void StatusSetDrvStatus(void)
{
save_free(MainStatus.LargeMessageStr);
save_free(MainStatus.MessageStr);

switch (CanModul.DeviceStatus.DrvStatus)
  {
  case DRV_NOT_LOAD            : {
                                 MainStatus.LargeMessageStr = g_strdup(_("Tiny-CAN API driver not loaded"));
                                 break;
                                 }
  case DRV_STATUS_NOT_INIT     : {
                                 MainStatus.LargeMessageStr = g_strdup(_("Initialize Program"));
                                 break;
                                 }
  case DRV_STATUS_INIT         : {
                                 if (SetupData.AutoConnect)
                                   MainStatus.LargeMessageStr = g_strdup(_("Connect Tiny-CAN hardware to PC"));
                                 else
                                   MainStatus.LargeMessageStr = g_strdup(_("Tiny-CAN not connected"));
                                 break;
                                 }
  case DRV_STATUS_PORT_NOT_OPEN :{
                                 MainStatus.LargeMessageStr = g_strdup(_("ERROR opening interface"));
                                 break;
                                 }
  case DRV_STATUS_PORT_OPEN    : {
                                 if (SetupData.AutoConnect)
                                   MainStatus.LargeMessageStr = g_strdup(_("Connect Tiny-CAN hardware to PC"));
                                 else
                                   MainStatus.LargeMessageStr = g_strdup(_("Tiny-CAN not connected"));
                                 break;
                                 }
  case DRV_STATUS_DEVICE_FOUND :
  case DRV_STATUS_CAN_OPEN     : {
                                 MainStatus.LargeMessageStr = g_strdup(_("Connecting to Tiny-CAN ..."));
                                 break;
                                 }
  case DRV_STATUS_CAN_RUN      : {
                                 MainStatus.MessageStr = g_strdup(_("Tiny-CAN connected"));  // System Idle
                                 break;
                                 }
  default                      : MainStatus.LargeMessageStr = g_strdup(_("?"));
  }
}


void StatusSetCanStatus(void)
{
const char *can_status_str;
unsigned char status;

status = CanModul.DeviceStatus.CanStatus;
save_free(MainStatus.CanStatusStr);
switch (status & 0x0F)
  {
  case CAN_STATUS_OK        : {
                              can_status_str = _("CAN: Ok");
                              break;
                              }
  case CAN_STATUS_ERROR     : {
                              can_status_str = _("CAN: Error");
                              break;
                              }
  case CAN_STATUS_WARNING   : {
                              can_status_str = _("CAN: Error warning");
                              break;
                              }
  case CAN_STATUS_PASSIV    : {
                              can_status_str = _("CAN: Error passiv");
                              break;
                              }
  case CAN_STATUS_BUS_OFF   : {
                              can_status_str = _("CAN: Bus off");
                              break;
                              }
  case CAN_STATUS_UNBEKANNT : {
                              can_status_str = _("CAN: unknown");
                              break;
                              }
  default                   : can_status_str = _("CAN: ?");
  }
if (status & BUS_FAILURE)
  MainStatus.CanStatusStr = g_strdup_printf("%s [BUS-FAILURE]", can_status_str);
else
  MainStatus.CanStatusStr = g_strdup(can_status_str);
}


void StatusSetDataRecord(void)
{
save_free(MainStatus.CanRecordStr);
switch (DataRecord)
  {
  case DATA_RECORD_START : {
                           MainStatus.CanRecordStr = g_strdup(_("RUN"));
                           break;
                           }
  case DATA_RECORD_STOP  : {
                           MainStatus.CanRecordStr = g_strdup(_("STOP"));
                           break;
                           }
  case DATA_RECORD_OV    : {
                           MainStatus.CanRecordStr = g_strdup(_("OV"));
                           break;
                           }
  case DATA_RECORD_LIMIT : {
                           MainStatus.CanRecordStr = g_strdup(_("FULL"));
                           break;
                           }
  default                : MainStatus.CanRecordStr = g_strdup(_("?"));
  }
}


void StatusRepaint(void)
{
gint id;
char str[100];

// ***** Statuszeile updaten
if (MainStatus.LargeMessageStr)
  g_strlcpy(str, MainStatus.LargeMessageStr, 100);
else
  {
  if ((MainStatus.CanSpeedStr) && (MainStatus.CanStatusStr) && (MainStatus.CanRecordStr))
    {
    if (MainStatus.MessageStr)
      g_snprintf(str, 100, "%s  |  %s  |  %s  |  %s", MainStatus.CanSpeedStr,
          MainStatus.CanStatusStr, MainStatus.CanRecordStr, MainStatus.MessageStr);
    else
      g_snprintf(str, 100, "%s  |  %s  |  %s", MainStatus.CanSpeedStr,
          MainStatus.CanStatusStr, MainStatus.CanRecordStr);
    }
  else
    g_strlcpy(str, "Status Error", 100);
  }
id = gtk_statusbar_get_context_id(GTK_STATUSBAR(MainStatus.Widget), str);
gtk_statusbar_push(GTK_STATUSBAR(MainStatus.Widget), id, str);
}


void CanStatusChangeCB(gulong signal, gpointer event_data, gpointer user_data)
{
StatusSetDrvStatus();
StatusSetCanStatus();
StatusRepaint();
MainSetSensitive();
}


void DataRecordChangeCB(gulong signal, gpointer event_data, gpointer user_data)
{
StatusSetDataRecord();
StatusRepaint();
}


void SetupChangeCB(gulong signal, gpointer event_data, gpointer user_data)
{
int showing;

StatusSetSpeed();
StatusSetDrvStatus();
StatusSetCanStatus();
StatusRepaint();
MainSetSensitive();
SetSymbolLeisten();
SetSendOnReturn();
// **** CAN Monitor Setup
// Puffer gr��e einstellen
if (CBufferGetSize(MainWin.CanBuffer) != SetupData.RxDLimit)
  CBufferSetSize(MainWin.CanBuffer, SetupData.RxDLimit);
// Schrift
CViewSetViewFont(MainWin.CanView, SetupData.WdgFontName);
CViewSetViewFont(MainWin.CanFilterView, SetupData.WdgFontName);
// Farben einstellen
CViewSetColumnsColor(MainWin.CanView, 2, &SetupData.WdgTimeColor);
CViewSetColumnsColor(MainWin.CanView, 3, &SetupData.WdgTypeColor);
CViewSetColumnsColor(MainWin.CanView, 4, &SetupData.WdgIdColor);
CViewSetColumnsColor(MainWin.CanView, 5, &SetupData.WdgDlcColor);
CViewSetColumnsColor(MainWin.CanView, 6, &SetupData.WdgData1Color);
CViewSetColumnsColor(MainWin.CanView, 7, &SetupData.WdgData2Color);

CViewSetColumnsColor(MainWin.CanFilterView, 2, &SetupData.WdgTimeColor);
CViewSetColumnsColor(MainWin.CanFilterView, 3, &SetupData.WdgTypeColor);
CViewSetColumnsColor(MainWin.CanFilterView, 4, &SetupData.WdgIdColor);
CViewSetColumnsColor(MainWin.CanFilterView, 5, &SetupData.WdgDlcColor);
CViewSetColumnsColor(MainWin.CanFilterView, 6, &SetupData.WdgData1Color);
CViewSetColumnsColor(MainWin.CanFilterView, 7, &SetupData.WdgData2Color);
// Zellen
showing = 0;
/* if (SetupData.ShowTime)
  showing |= SHOW_COL_TIME; */
if (SetupData.ShowType)
  showing |= SHOW_COL_TYPE;
if (SetupData.ShowDlc)
  showing |= SHOW_COL_DLC;

switch (SetupData.ShowData1)
  {
  case 1 : {
           showing |= SHOW_COL_D1_HEX;
           break;
           }
  case 2 : {
           showing |= SHOW_COL_D1_DEZIMAL;
           break;
           }
  case 3 : {
           showing |= SHOW_COL_D1_ASCII;
           break;
           }
  }
switch (SetupData.ShowData2)
  {
  case 1 : {
           showing |= SHOW_COL_D2_HEX;
           break;
           }
  case 2 : {
           showing |= SHOW_COL_D2_DEZIMAL;
           break;
           }
  case 3 : {
           showing |= SHOW_COL_D2_ASCII;
           break;
           }
  }
switch (SetupData.ShowTime)
  {
  case 1 : {
           showing |= SHOW_COL_TIME_SYS;
           break;
           }
  case 2 : {
           showing |= SHOW_COL_TIME_SYS_REL;
           break;
           }
  case 3 : {
           showing |= SHOW_COL_TIME_SYS_MS;
           break;
           }
  case 4 : {
           showing |= SHOW_COL_TIME_SYS_REL_MS;
           break;
           }
  case 5 : {
           showing |= SHOW_COL_TIME_HW;
           break;
           }
  case 6 : {
           showing |= SHOW_COL_TIME_HW_REL;
           break;
           }
  }
CViewSetShow(MainWin.CanFilterView, showing | SHOW_COL_INDEX);
if (SetupData.ShowDir)
  showing |= SHOW_COL_DIR;
CViewSetShow(MainWin.CanView, showing);
// Senden Frame
if (SetupData.SendenEntrysCount != MainVar.CanTransmitEntrysCount)
  CreateSendenWidget(SetupData.SendenEntrysCount);
}


void MainShowingChangeCB(gulong signal, gpointer event_data, gpointer user_data)
{
SetSymbolLeisten();
}


/**************************************************************/
/* Makro Fenster, Menues und Buttons entsprechend der         */
/* Variable MakroWindow anpassen                              */
/**************************************************************/
void SetMakroWindow(void)
{
gboolean value;

if (MainVar.MakroWindow)
  {
  MakWinShow();
  value = TRUE;
  }
else
  {
  MakWinHide();
  value = FALSE;
  }
DisableEventMMakBearbeiten();
gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MainWin.MMakBearbeiten), value);
EnableEventMMakBearbeiten();
}


/**************************************************************/
/*                               */
/**************************************************************/
void DeleteSendenWidget(void)
{
int i;

for (i = 0; i < 10; i++)
  {
  if (SendenCanWdg[i])
    {
    CanTxWidgetDestroy(SendenCanWdg[i]);
    SendenCanWdg[i] = NULL;
    }
  }
MainVar.CanTransmitEntrysCount = 0;
}


/**************************************************************/
/*                               */
/**************************************************************/
void CreateSendenWidget(int used_lines)
{
int i;

DeleteSendenWidget();
if (used_lines > 10)
  used_lines = 10;
for (i = 0; i < used_lines; i++)
  SendenCanWdg[i] = CanTxWidgetNew(MainWin.CanTxWindow, CWDG_FLAGS, &SendenButtonCB);
MainVar.CanTransmitEntrysCount = used_lines;
}


/**************************************************************/
/*                               */
/**************************************************************/
void SetSendOnReturn(void)
{
int i;

for (i = 0; i < MainVar.CanTransmitEntrysCount; i++)
  {
  if (SendenCanWdg[i])
    CanTxSetReturnAutoSend(SendenCanWdg[i], SetupData.SendOnReturn);
  }
}


/**************************************************************/
/* Symbolleisten anzeigen / ausblenden                        */
/**************************************************************/
void SetSymbolLeisten(void)
{
gboolean value;
GtkToolbarStyle style;

DisableEventButtonLeisten();
// ***** Standart Leiste
if (SetupData.AnsStandart)
  value = TRUE;
else
  value = FALSE;
gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MainWin.MStandartLeiste), value);
if (SetupData.AnsStandart)
  gtk_widget_show(MainWin.MainToolbar);
else
  gtk_widget_hide(MainWin.MainToolbar);
// ***** Makro Leiste
if (SetupData.AnsMakro)
  value = TRUE;
else
  value = FALSE;
gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MainWin.MMakroLeiste), value);
if (SetupData.AnsMakro)
  gtk_widget_show(MainWin.MakrosFrame);
else
  gtk_widget_hide(MainWin.MakrosFrame);
// ***** Filter Leiste
if (SetupData.FilterFrame)
  value = TRUE;
else
  value = FALSE;
gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MainWin.MFilMessages), value);
if (SetupData.FilterFrame)
  gtk_widget_show(MainWin.CanFilterDataView);
else
  gtk_widget_hide(MainWin.CanFilterDataView);
// ***** Senden Leiste
if (SetupData.AnsSenden)
  value = TRUE;
else
  value = FALSE;
gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MainWin.MSendenLeiste), value);
if (SetupData.AnsSenden)
  gtk_widget_show(MainWin.CanTxWindow);
else
  gtk_widget_hide(MainWin.CanTxWindow);
// ***** Text anzeigen/ausblenden
if (SetupData.AnsButtonText)
  {
  value = TRUE;
  style = GTK_TOOLBAR_BOTH;
  }
else
  {
  value = FALSE;
  style = GTK_TOOLBAR_ICONS;
  }
gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MainWin.MButtonsText), value);
gtk_toolbar_set_style(GTK_TOOLBAR(MainWin.MainToolbar), style);
gtk_toolbar_set_style(GTK_TOOLBAR(MainWin.MakrosToolbar), style);
EnableEventButtonLeisten();
}


/**************************************************************/
/* Data Record Variable behandeln                             */
/**************************************************************/
void SetDataRecord(void)
{
GtkWidget *image;
gint iconsize;

DisableEventCanStartStop();
if ((DataRecord == DATA_RECORD_START) || (DataRecord == DATA_RECORD_OV))
  {
  // ***************** Aufzeichnung START
  if (!SetupData.DataClearMode)
    ClearData();
  else if (SetupData.DataClearMode == 1)
    {
    if (CViewGetUsedSize(MainWin.CanView))
      {
      if (msg_box(MSG_TYPE_MESSAGE | DLG_YES_NO_BUTTONS, _("Question"), _("Clear buffer ?")))
        ClearData();
      }
    }
  }
// **** Menue Items updaten
iconsize = gtk_toolbar_get_icon_size(GTK_TOOLBAR(MainWin.MainToolbar));
if ((DataRecord == DATA_RECORD_START) || (DataRecord == DATA_RECORD_OV))
  {
  // Men�
  image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(MainWin.MCanStartStop), image);//MenuStopImage);
  // Button
  gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(MainWin.CanStartStopButton), TRUE);
  image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP, iconsize);
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(MainWin.CanStartStopButton), image);
  gtk_tool_button_set_label(GTK_TOOL_BUTTON(MainWin.CanStartStopButton), _("Stop"));
  gtk_widget_show_all(MainWin.CanStartStopButton);
  }
else
  {
  // Men�
  image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(MainWin.MCanStartStop), image); //;MenuPlayImage);
  // Button
  gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(MainWin.CanStartStopButton), FALSE);
  image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, iconsize);
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(MainWin.CanStartStopButton), image);
  gtk_tool_button_set_label(GTK_TOOL_BUTTON(MainWin.CanStartStopButton), _("Start"));
  gtk_widget_show_all(MainWin.CanStartStopButton);
  }
EnableEventCanStartStop();
mhs_signal_emit(SIGC_PUBLIC, SIG_DATA_RECORD_CHANGE, NULL);
}


/**************************************************************/
/*  Gtk Main Callbacks                                        */
/**************************************************************/
gint delete_event(GtkWidget *widget, GdkEvent event, gpointer daten)
{
ExitApplikation();
return(FALSE);
}


void ende(GtkWidget *widget, gpointer daten)
{
gtk_main_quit();
}


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int no_setup_load, i;
gchar *file_name;

no_setup_load = 0;
PathsInit(argv[0]);

#ifdef ENABLE_NLS
bindtextdomain (GETTEXT_PACKAGE, Paths.locale_dir);
bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
textdomain (GETTEXT_PACKAGE);
#endif

gtk_set_locale();
// g_thread_init(NULL); <*> löschen
gtk_init(&argc, &argv);

FilCreate();
mhs_object_init();
mhs_signal_init(&MhsCanViewSignals);

if (ReadCommandLine(argc, argv))
  g_error(_("invalid command line option, type -h for help"));
// ****** Splash Bildschirm anzeigen
SplashStart(Paths.base_dir, &SplashData, _("starting program"));
// **** Variablen Initialisieren
MainEventsMask = 0x0000;
for (i = 0; i < 10; i++)
  SendenCanWdg[i] = NULL;
MainVar.CanTransmitEntrysCount = -1;
DataRecord = DATA_RECORD_STOP;
MainVar.MakroWindow = 0;   // Makro Window hide
MainVar.FilterWindow = 0;  // Filter Window hide
// **** Main Window anzeigen
TinyCanInit();
CanViewObjInit();
CreateMainWin();
SetupInit();
g_signal_connect(MainWin.MainWin, "delete_event", G_CALLBACK(delete_event), NULL);
g_signal_connect(MainWin.MainWin, "destroy", G_CALLBACK(ende), NULL);
// **** Can Monitor Widget erzeugen
MainWin.CanView = CViewNew(MainWin.CanDataView);
MainWin.CanBuffer = CBufferCreate(0);
(void)CViewAttachBuffer(MainWin.CanView, MainWin.CanBuffer);

// **** Can Filter View Widget erzeugen
MainWin.CanFilterView = CViewNew(MainWin.CanFilterDataView);
CViewDataHandlerCreate(MainWin.CanFilterView, 0, &FilGetMsgCB, &FilGetNameCB);
// **** Makro List View erzeugen
MainMakroView = MakWinCreateTable(MainWin.MakroListView, 0, MainMakroViewSelectCB);
// **** User Speed Tabelle laden
UserCanSpeedList = NULL;
file_name = CreateFileName(Paths.setup_dir, "user_speeds.dat");
if (g_file_test(file_name, G_FILE_TEST_IS_REGULAR))
  UserCanSpeedList = CanSpeedDBOpen(file_name);
save_free(file_name);
// **** Status leiste
MainStatusCreate();

UpdateGTK();
// **** Signal Handler binden
g_signal_connect ((gpointer)MainWin.MNeu, "activate", G_CALLBACK(on_MNeu_activate), NULL);
g_signal_connect ((gpointer)MainWin.MOpen, "activate", G_CALLBACK(on_MOpen_activate), NULL);
g_signal_connect ((gpointer)MainWin.MSave, "activate", G_CALLBACK(on_MSave_activate), NULL);
g_signal_connect ((gpointer)MainWin.MDateiInfo, "activate", G_CALLBACK(on_MDateiInfo_activate), NULL);
g_signal_connect ((gpointer)MainWin.MBeenden, "activate", G_CALLBACK(on_MBeenden_activate), NULL);
g_signal_connect ((gpointer)MainWin.MCanStartStop, "activate", G_CALLBACK(on_MCanStartStop_activate), NULL);
g_signal_connect ((gpointer)MainWin.MCanReset, "activate", G_CALLBACK(on_MCanReset_activate), NULL);
g_signal_connect ((gpointer)MainWin.MMakBearbeiten, "activate", G_CALLBACK(on_MMakBearbeiten_activate), NULL);
g_signal_connect ((gpointer)MainWin.MMakOpen, "activate", G_CALLBACK(on_MMakOpen_activate), NULL);
g_signal_connect ((gpointer)MainWin.MMakSave, "activate", G_CALLBACK(on_MMakSave_activate), NULL);
g_signal_connect ((gpointer)MainWin.MFilMessages, "activate", G_CALLBACK(on_MFilMessages_activate), NULL);
g_signal_connect ((gpointer)MainWin.MFilBearbeiten, "activate", G_CALLBACK(on_MFilBearbeiten_activate), NULL);
g_signal_connect ((gpointer)MainWin.MFilOpen, "activate", G_CALLBACK(on_MFilOpen_activate), NULL);
g_signal_connect ((gpointer)MainWin.MFilSave, "activate", G_CALLBACK(on_MFilSave_activate), NULL);
g_signal_connect ((gpointer)MainWin.MStandartLeiste, "activate", G_CALLBACK(on_MStandartLeiste_activate), NULL);
g_signal_connect ((gpointer)MainWin.MSendenLeiste, "activate", G_CALLBACK(on_MSendenLeiste_activate), NULL);
g_signal_connect ((gpointer)MainWin.MMakroLeiste, "activate", G_CALLBACK(on_MMakroLeiste_activate), NULL);
g_signal_connect ((gpointer)MainWin.MButtonsText, "activate", G_CALLBACK(on_MButtonsText_activate), NULL);
g_signal_connect ((gpointer)MainWin.MEinstellungen, "activate", G_CALLBACK(on_MEinstellungen_activate), NULL);
g_signal_connect ((gpointer)MainWin.MPluginsSetup, "activate", G_CALLBACK(on_MPluginsSetup_activate), NULL);
g_signal_connect ((gpointer)MainWin.MConnect, "activate", G_CALLBACK(on_MConnect_activate), NULL);
g_signal_connect ((gpointer)MainWin.MDeviceInfo, "activate", G_CALLBACK(on_MDeviceInfo_activate), NULL);
g_signal_connect ((gpointer)MainWin.MInfo, "activate", G_CALLBACK(on_MInfo_activate), NULL);
g_signal_connect ((gpointer)MainWin.NeuButton, "clicked", G_CALLBACK(on_NeuButton_clicked), NULL);
g_signal_connect ((gpointer)MainWin.FileOpenButton, "clicked", G_CALLBACK(on_FileOpenButton_clicked), NULL);
g_signal_connect ((gpointer)MainWin.FileSaveButton, "clicked", G_CALLBACK(on_FileSaveButton_clicked), NULL);
g_signal_connect ((gpointer)MainWin.BeendenButton, "clicked", G_CALLBACK(on_BeendenButton_clicked), NULL);
g_signal_connect ((gpointer)MainWin.CanStartStopButton, "clicked", G_CALLBACK(on_CanStartStopButton_clicked), NULL);
g_signal_connect ((gpointer)MainWin.CanResetButton, "clicked", G_CALLBACK(on_CanResetButton_clicked), NULL);
g_signal_connect ((gpointer)MainWin.EinstellungenButton, "clicked", G_CALLBACK (on_EinstellungenButton_clicked), NULL);
g_signal_connect ((gpointer)MainWin.MakroSendenButton, "clicked", G_CALLBACK(on_MakroSendenButton_clicked), NULL);
g_signal_connect ((gpointer)MainWin.MakroPasteButton, "clicked", G_CALLBACK(on_MakroPasteButton_clicked), NULL);

// **** MHS Siganl Handler binden
mhs_signal_connect(NULL, SIGC_PUBLIC, SIG_CAN_STATUS_CHANGE, CanStatusChangeCB, NULL);
mhs_signal_connect(NULL, SIGC_PUBLIC, SIG_DATA_RECORD_CHANGE, DataRecordChangeCB, NULL);
mhs_signal_connect(NULL, SIGC_PUBLIC, SIG_SETUP_CHANGE, SetupChangeCB, NULL);
mhs_signal_connect(NULL, SIGC_PUBLIC, SIG_FILTER_EDIT_FINISH, FilterEditFinishEventCB, NULL);

mhs_signal_connect(NULL, SIGC_PROTECTED, SIG_MAIN_SHOWING_CHANGE, MainShowingChangeCB, NULL);
mhs_signal_connect(NULL, SIGC_PROTECTED, SIG_MAIN_MAKRO_REPAINT, MainMakroRepaintCB, NULL);
mhs_signal_connect(NULL, SIGC_PROTECTED, SIG_MAIN_CHANGE_MAKRO_NAME, MainMakroRepaintItemCB, NULL);

(void)mhs_object_event_connect((struct TMhsObj *)ObjTinyCanStatus, (TMhsObjectCB)SyncCanStatusEvent, NULL);
(void)mhs_object_event_connect((struct TMhsObj *)ObjTinyCanPnP, (TMhsObjectCB)SyncCanPnPEvent, NULL);
(void)mhs_object_event_connect(ObjTinyCanBufferFull, SyncCanBufferFull, NULL);
(void)mhs_object_event_connect(ObjRxDFilter, SyncRxDFilterEvent, NULL);
// ****** Setup laden
SplashUpdate(_("loading setup"));
if (LoadSetup() < 0)
  no_setup_load = 1;
// ****** Makro Datei laden
if (g_file_test(SetupData.MakroFile, G_FILE_TEST_IS_REGULAR))
  MakroReadFromFile(SetupData.MakroFile);
// ****** Filter Datei laden
if (g_file_test(SetupData.FilterFile, G_FILE_TEST_IS_REGULAR))
  FilterReadFromFile(SetupData.FilterFile);
// ****** Plugins suchen
SplashUpdate(_("loading plugins"));
PluginsScan();
// ****** Splash Bildschirm l�schen
SplashStop();
// ****** Main Window anzeigen
ShowMainWin();
if (no_setup_load)
  ShowStartWin();
// ****** GTK Mainschleife aufrufen
gtk_main(); // GTK+ starten
// Alle MHS-Signale l�schen
mhs_signal_clear_all();
// ****** Plugins entfernen
PluginDestroyAll();
// Makro Fenster
MakWinDestroy();
MakWinDestroyTable(MainMakroView);
MainStatusDestroy();
/* #ifdef USE_EVENT_MODE
gdk_threads_leave();
#endif */
CanSpeedDBDestroy(&UserCanSpeedList);
PathsFree();
SetupDestroy();
mhs_object_destroy();

return 0;
}


/*************************/
/* Programm wird beendet */
/*************************/
void ExitApplikation(void)
{
TinyCanDestroy();
// ****** Setup speichern
SaveSetup();
if (strlen(SetupData.MakroFile) > 0)
  MakroWriteToFile(SetupData.MakroFile);  // MakroFile speichern
if (strlen(SetupData.FilterFile) > 0)
  FilterWriteToFile(SetupData.FilterFile); // FilterFile speichern
UpdateGTK();
DeleteSendenWidget();
}


/******************/
/* Menu callbacks */
/******************/

// **** Datei Neu
void on_MNeu_activate(GtkObject * object, gpointer user_data)
{
ClearData();  // Liste l�schen
}


// **** Datei �ffnen
void on_MOpen_activate(GtkObject * object, gpointer user_data)
{
char *file_name;

file_name = NULL;
if (SelectFileDlg(_("Loading CAN-Trace file"), FILE_SELECTION_OPEN, &file_name) > 0)
  {
  switch (CBufferLoadFile(MainWin.CanBuffer, file_name))
    {
    case -1 : {
              msg_box(MSG_TYPE_ERROR, _("Error"), _("Error loading file"));
              break;
              }
    case -2 : {
              msg_box(MSG_TYPE_ERROR, _("Error"), _("Error reading data from file"));
              break;
              }
    }
  }
save_free(file_name);
}


// **** Datei speichern
void on_MSave_activate(GtkObject *object, gpointer user_data)
{
char *file_name;
union TCanViewShowing show;

file_name = NULL;
if (CViewGetUsedSize(MainWin.CanView) < 1)
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Buffer is empty, no data to writing into file"));
else
  {
  if (SelectFileDlg(_("Saving CAN-Trace file"), FILE_SELECTION_SAVE, &file_name) > 0)
    {
    show.Long = MainWin.CanView->Showing;
    MainWin.CanBuffer->TimeFormat = show.View.Time;
    if (CBufferSaveFile(MainWin.CanBuffer, file_name) < 0)
      msg_box(MSG_TYPE_ERROR, _("Error"), _("Error saving file"));
    }
  }
save_free(file_name);
}


// **** Datei Info
void on_MDateiInfo_activate(GtkObject * object, gpointer user_data)
{
char *s;

s = CBufferGetInfoString(MainWin.CanBuffer);
ShowDateiInfo(_("File information"), _("<b>Text:</b>"), &s);
CBufferSetInfoString(MainWin.CanBuffer, s);
save_free(s);
}


// **** Programm beenden
void on_MBeenden_activate(GtkObject * object, gpointer user_data)
{
ExitApplikation();
// Programm beenden
gtk_main_quit();
}


// **** Aufzeichnung starten
void on_MCanStartStop_activate(GtkObject * object, gpointer user_data)
{
// Status toggeln
if (CheckEventCanStartStop())
  return;
if ((DataRecord == DATA_RECORD_START) || (DataRecord == DATA_RECORD_OV))
  DataRecord = DATA_RECORD_STOP;
else
  DataRecord = DATA_RECORD_START;
SetDataRecord();
}


// **** Controller Reset
void on_MCanReset_activate(GtkObject *object, gpointer user_data)
{
SetupCanSetMode(0L, OP_CAN_START, CAN_CMD_RXD_FIFOS_CLEAR | CAN_CMD_TXD_FIFOS_CLEAR);
}


// **** Makro bearbeiten
void on_MMakBearbeiten_activate(GtkCheckMenuItem *object, gpointer user_data)
{
if (CheckEventMMakBearbeiten())
  return;
if (object->active == TRUE)
  MainVar.MakroWindow = 1;
else
  MainVar.MakroWindow = 0;
SetMakroWindow();
}


// **** Makro Datei laden
void on_MMakOpen_activate(GtkObject * object, gpointer user_data)
{
if (SelectFileDlg(_("Loading macro file"), FILE_SELECTION_OPEN, &SetupData.MakroFile) > 0)
  MakroReadFromFile(SetupData.MakroFile);
}


// **** Makro Datei speichern
void on_MMakSave_activate(GtkObject * object, gpointer user_data)
{
if (SelectFileDlg(_("Saving macro file"), FILE_SELECTION_SAVE, &SetupData.MakroFile) > 0)
  MakroWriteToFile(SetupData.MakroFile);
}


// **** Filter Messages anzeigen
void on_MFilMessages_activate(GtkCheckMenuItem *object, gpointer user_data)
{
if (CheckEventButtonLeisten())
  return;
if (object->active == TRUE)
  SetupData.FilterFrame= 1;
else
  SetupData.FilterFrame = 0;
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_SHOWING_CHANGE, NULL);
}


// **** Filter bearbeiten
void on_MFilBearbeiten_activate(GtkObject * object, gpointer user_data)
{
FlWinExecute();
}


// **** Filter Datei laden
void on_MFilOpen_activate(GtkObject * object, gpointer user_data)
{
if (SelectFileDlg(_("Loading filter file"), FILE_SELECTION_OPEN, &SetupData.FilterFile) > 0)
  {
  FilterReadFromFile(SetupData.FilterFile);
  }
}


// **** Filter Datei speichern
void on_MFilSave_activate(GtkObject * object, gpointer user_data)
{
if (SelectFileDlg(_("Saving filter file"), FILE_SELECTION_SAVE, &SetupData.FilterFile) > 0)
  {
  FilterWriteToFile(SetupData.FilterFile);
  }
}


// **** Standart Leiste anzeigen/verbergen
void on_MStandartLeiste_activate(GtkCheckMenuItem *object, gpointer user_data)
{
if (CheckEventButtonLeisten())
  return;
if (object->active == TRUE)
  SetupData.AnsStandart = 1;
else
  SetupData.AnsStandart = 0;
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_SHOWING_CHANGE, NULL);
}


// **** Senden Leiste anzeigen/verbergen
void on_MSendenLeiste_activate(GtkCheckMenuItem *object, gpointer user_data)
{
if (CheckEventButtonLeisten())
  return;
if (object->active == TRUE)
  SetupData.AnsSenden = 1;
else
  SetupData.AnsSenden = 0;
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_SHOWING_CHANGE, NULL);
}


// **** Makro Leiste anzeigen/verbergen
void on_MMakroLeiste_activate(GtkCheckMenuItem *object, gpointer user_data)
{
if (CheckEventButtonLeisten())
  return;
if (object->active == TRUE)
  SetupData.AnsMakro = 1;
else
  SetupData.AnsMakro = 0;
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_SHOWING_CHANGE, NULL);
}


// **** Buttons Text anzeigen/verbergen
void on_MButtonsText_activate(GtkCheckMenuItem *object, gpointer user_data)
{
if (CheckEventButtonLeisten())
  return;
if (object->active == TRUE)
  SetupData.AnsButtonText = 1;
else
  SetupData.AnsButtonText = 0;
mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_SHOWING_CHANGE, NULL);
}


// **** Optionen Einstellungen
void on_MEinstellungen_activate(GtkObject * object, gpointer user_data)
{
ExecuteSetup();
}


// **** Plugin Setup
void on_MPluginsSetup_activate(GtkObject * object, gpointer user_data)
{
PluginDialogShow();
}


// **** Verbindung zum Tiny-CAN herstellen
void on_MConnect_activate(GtkObject * object, gpointer user_data)
{
if (TinyCanConnect(TINY_CAN_DRIVER_REOPEN) < 0)
  msg_box(MSG_TYPE_ERROR, _("Error"), _("No hardware found or no driver loaded"));
}


// **** Tiny-CAN Info anzeigen
void on_MDeviceInfo_activate(GtkObject * object, gpointer user_data)
{
/* char *info_string;

info_string = CanDrvHwInfo(0L); */
if (CanModul.HardwareInfoStr)
  ShowDrvInfoWin(CanModul.HardwareInfoStr, _("<b><big>Information about Tiny-CAN hardware</big></b>"), _("Info"));
}


// **** Hilfe Info
void on_MInfo_activate(GtkObject * object, gpointer user_data)
{
ShowAboutDlg(Paths.base_dir, &AboutTxt);
}


/*****************************/
/* Toolbar buttons callbacks */
/*****************************/

// **** Neu Button
void on_NeuButton_clicked(GtkButton *button, gpointer user_data)
{
on_MNeu_activate(NULL, NULL);
}


// **** Datei �ffnen Button
void on_FileOpenButton_clicked(GtkButton *button, gpointer user_data)
{
on_MOpen_activate(NULL, NULL);
}


// **** Datei speichern Button
void on_FileSaveButton_clicked(GtkButton *button, gpointer user_data)
{
on_MSave_activate(NULL, NULL);
}


// **** Beenden Button
void on_BeendenButton_clicked(GtkButton *button, gpointer user_data)
{
// Programm beenden
ExitApplikation();
gtk_main_quit();
}


// **** Can Start/Stop Button
void on_CanStartStopButton_clicked(GtkButton *button, gpointer user_data)
{
on_MCanStartStop_activate(NULL, NULL);
}


// **** Can Reset Button
void on_CanResetButton_clicked(GtkButton *button, gpointer user_data)
{
on_MCanReset_activate(NULL, NULL);
}


// **** Einstellungen Button
void on_EinstellungenButton_clicked(GtkButton *button, gpointer user_data)
{
on_MEinstellungen_activate(NULL, NULL);
}


// **** Senden Button
void SendenButtonCB(struct TCanTxWidget *ctw, int event, struct TCanMsg *can_msg)
{
SetLastTransmit(can_msg);
if (CanTransmit(0L, can_msg, 1) < 0)
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error transmit CAN message"));
}


// **** Makro Senden Button
void on_MakroSendenButton_clicked(GtkButton *button, gpointer user_data)
{
struct TMakro *makro;
GtkTreeModel *model;
GtkTreeIter iter;
GtkTreeSelection *sel;

if (!MainMakroView)
  return;
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(MainMakroView->Base));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, MAKRO_COLUMN_MAKRO, &makro, -1);
  g_assert(makro != NULL);
  if (CanTransmit(0, &makro->CanMsg, 1) < 0)
    msg_box(MSG_TYPE_ERROR, _("Error"), _("Error transmit CAN message"));
  }
}


// **** Makro Paste Button
void on_MakroPasteButton_clicked(GtkButton *button, gpointer user_data)
{
struct TMakro *makro;
GtkTreeModel *model;
GtkTreeIter iter;
GtkTreeSelection *sel;

if (!MainMakroView)
  return;
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(MainMakroView->Base));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, MAKRO_COLUMN_MAKRO, &makro, -1);
  g_assert(makro != NULL);
  CanTxWidgetSetMsg(SendenCanWdg[0], &makro->CanMsg);
  }
}


/**************************************************************/
/* Makro in Tabelle ausw�hlen Callback                        */
/**************************************************************/
void MainMakroViewSelectCB(struct TMakroTableWidget *mtw, int event, struct TMakro *makro)
{
if (!MainMakroView)
  return;
MainSetSensitive();
}


void MainMakroRepaintCB(gulong signal, gpointer event_data, gpointer user_data)
{
if (!MainMakroView)
  return;
MakWinRepaintTable(MainMakroView);
}


void MainMakroRepaintItemCB(gulong signal, gpointer event_data, gpointer user_data)
{
struct TMakro *makro;

makro = (struct TMakro *)event_data;
if (!MainMakroView)
  return;
MakRepaintMakro(MainMakroView, makro);
}


void FilterEditFinishEventCB(gulong signal, gpointer event_data, gpointer user_data)
{
CViewSetUsedSize(MainWin.CanFilterView, FilGetLength(FILTER_VISIBLE));
CViewQueueDraw(MainWin.CanFilterView);
}


void CreateMainWin(void)
{
GtkWidget *widget;
GtkWidget *vbox1;
GtkWidget *menubar1;
GtkWidget *image;
GtkIconSize tmp_toolbar_icon_size;
GtkWidget *hpaned1;
GtkWidget *vbox2;
GtkWidget *scrolledwindow2;
GtkWidget *vpaned1;
GtkAccelGroup *accel_group;
gchar *filename;

accel_group = gtk_accel_group_new ();

MainWin.MainWin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
gtk_window_set_title (GTK_WINDOW (MainWin.MainWin), _("Tiny CAN Monitor"));
gtk_window_set_position (GTK_WINDOW (MainWin.MainWin), GTK_WIN_POS_CENTER);
gtk_window_set_default_size (GTK_WINDOW (MainWin.MainWin), 750, 450);
filename = CreateFileName(Paths.base_dir, "tiny-can.png");
gtk_window_set_icon_from_file(GTK_WINDOW(MainWin.MainWin), filename, NULL);
save_free(filename);
gtk_widget_realize(MainWin.MainWin);
vbox1 = gtk_vbox_new (FALSE, 0);
gtk_widget_show (vbox1);
gtk_container_add (GTK_CONTAINER(MainWin.MainWin), vbox1);

menubar1 = gtk_menu_bar_new ();
gtk_widget_show (menubar1);
gtk_box_pack_start (GTK_BOX (vbox1), menubar1, FALSE, FALSE, 0);

MainWin.MDatei = gtk_menu_item_new_with_mnemonic (_("_File"));
gtk_widget_show (MainWin.MDatei);
gtk_container_add (GTK_CONTAINER (menubar1), MainWin.MDatei);

MainWin.MDatei_menu = gtk_menu_new ();
gtk_menu_item_set_submenu (GTK_MENU_ITEM (MainWin.MDatei), MainWin.MDatei_menu);

MainWin.MNeu = gtk_image_menu_item_new_from_stock ("gtk-new", accel_group);
gtk_widget_show (MainWin.MNeu);
gtk_container_add (GTK_CONTAINER (MainWin.MDatei_menu), MainWin.MNeu);

MainWin.MOpen = gtk_image_menu_item_new_from_stock ("gtk-open", accel_group);
gtk_widget_show (MainWin.MOpen);
gtk_container_add (GTK_CONTAINER (MainWin.MDatei_menu), MainWin.MOpen);

MainWin.MSave = gtk_image_menu_item_new_from_stock ("gtk-save", accel_group);
gtk_widget_show (MainWin.MSave);
gtk_container_add (GTK_CONTAINER (MainWin.MDatei_menu), MainWin.MSave);

MainWin.MDateiInfo = gtk_menu_item_new_with_mnemonic (_("Edit file information"));
gtk_widget_show (MainWin.MDateiInfo);
gtk_container_add (GTK_CONTAINER (MainWin.MDatei_menu), MainWin.MDateiInfo);

widget = gtk_separator_menu_item_new ();
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (MainWin.MDatei_menu), widget);
gtk_widget_set_sensitive (widget, FALSE);

MainWin.MBeenden = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
gtk_widget_show (MainWin.MBeenden);
gtk_container_add (GTK_CONTAINER (MainWin.MDatei_menu), MainWin.MBeenden);

MainWin.MCan = gtk_menu_item_new_with_mnemonic (_("_CAN"));
gtk_widget_show (MainWin.MCan);
gtk_container_add (GTK_CONTAINER (menubar1), MainWin.MCan);

MainWin.MCan_menu = gtk_menu_new ();
gtk_menu_item_set_submenu (GTK_MENU_ITEM (MainWin.MCan), MainWin.MCan_menu);

MainWin.MCanStartStop = gtk_image_menu_item_new_with_mnemonic (_("_Trace recording start/stop"));
gtk_widget_show (MainWin.MCanStartStop);
gtk_container_add (GTK_CONTAINER (MainWin.MCan_menu), MainWin.MCanStartStop);

image = gtk_image_new_from_stock ("gtk-media-play", GTK_ICON_SIZE_MENU);
gtk_widget_show (image);
gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (MainWin.MCanStartStop), image);

MainWin.MCanReset = gtk_image_menu_item_new_with_mnemonic (_("_Controller reset"));
gtk_widget_show (MainWin.MCanReset);
gtk_container_add (GTK_CONTAINER (MainWin.MCan_menu), MainWin.MCanReset);

image = gtk_image_new_from_stock ("gtk-stop", GTK_ICON_SIZE_MENU);
gtk_widget_show (image);
gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (MainWin.MCanReset), image);

MainWin.MMakro = gtk_menu_item_new_with_mnemonic (_("_Macro"));
gtk_widget_show (MainWin.MMakro);
gtk_container_add (GTK_CONTAINER (menubar1), MainWin.MMakro);

MainWin.MMakro_menu = gtk_menu_new ();
gtk_menu_item_set_submenu (GTK_MENU_ITEM (MainWin.MMakro), MainWin.MMakro_menu);

MainWin.MMakBearbeiten = gtk_check_menu_item_new_with_mnemonic (_("_Edit"));
gtk_widget_show (MainWin.MMakBearbeiten);
gtk_container_add (GTK_CONTAINER (MainWin.MMakro_menu), MainWin.MMakBearbeiten);

MainWin.MMakOpen = gtk_image_menu_item_new_with_mnemonic (_("_Load file"));
gtk_widget_show (MainWin.MMakOpen);
gtk_container_add (GTK_CONTAINER (MainWin.MMakro_menu), MainWin.MMakOpen);

image = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
gtk_widget_show (image);
gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (MainWin.MMakOpen), image);

MainWin.MMakSave = gtk_image_menu_item_new_with_mnemonic (_("_Save file"));
gtk_widget_show (MainWin.MMakSave);
gtk_container_add (GTK_CONTAINER (MainWin.MMakro_menu), MainWin.MMakSave);

image = gtk_image_new_from_stock ("gtk-save", GTK_ICON_SIZE_MENU);
gtk_widget_show (image);
gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (MainWin.MMakSave), image);

MainWin.MFilter = gtk_menu_item_new_with_mnemonic (_("_Filter"));
gtk_widget_show (MainWin.MFilter);
gtk_container_add (GTK_CONTAINER (menubar1), MainWin.MFilter);

MainWin.MFilter_menu = gtk_menu_new ();
gtk_menu_item_set_submenu (GTK_MENU_ITEM (MainWin.MFilter), MainWin.MFilter_menu);

MainWin.MFilMessages = gtk_check_menu_item_new_with_mnemonic (_("_Show messages"));
gtk_widget_show (MainWin.MFilMessages);
gtk_container_add (GTK_CONTAINER (MainWin.MFilter_menu), MainWin.MFilMessages);

MainWin.MFilBearbeiten = gtk_menu_item_new_with_mnemonic (_("_Edit"));
gtk_widget_show (MainWin.MFilBearbeiten);
gtk_container_add (GTK_CONTAINER (MainWin.MFilter_menu), MainWin.MFilBearbeiten);

MainWin.MFilOpen = gtk_image_menu_item_new_with_mnemonic (_("_Load file"));
gtk_widget_show (MainWin.MFilOpen);
gtk_container_add (GTK_CONTAINER (MainWin.MFilter_menu), MainWin.MFilOpen);

image = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
gtk_widget_show (image);
gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (MainWin.MFilOpen), image);

MainWin.MFilSave = gtk_image_menu_item_new_with_mnemonic (_("_Save file"));
gtk_widget_show (MainWin.MFilSave);
gtk_container_add (GTK_CONTAINER (MainWin.MFilter_menu), MainWin.MFilSave);

image = gtk_image_new_from_stock ("gtk-save", GTK_ICON_SIZE_MENU);
gtk_widget_show (image);
gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (MainWin.MFilSave), image);

MainWin.MPlugins = gtk_menu_item_new_with_mnemonic (_("_Plugins"));
gtk_widget_show (MainWin.MPlugins);
gtk_container_add (GTK_CONTAINER (menubar1), MainWin.MPlugins);

MainWin.MPlugins_menu = gtk_menu_new ();
gtk_menu_item_set_submenu (GTK_MENU_ITEM (MainWin.MPlugins), MainWin.MPlugins_menu);

MainWin.MAnsicht = gtk_menu_item_new_with_mnemonic (_("_View"));
gtk_widget_show (MainWin.MAnsicht);
gtk_container_add (GTK_CONTAINER (menubar1), MainWin.MAnsicht);

MainWin.MAnsicht_menu = gtk_menu_new ();
gtk_menu_item_set_submenu (GTK_MENU_ITEM (MainWin.MAnsicht), MainWin.MAnsicht_menu);

MainWin.MStandartLeiste = gtk_check_menu_item_new_with_mnemonic (_("Show standard bar"));
gtk_widget_show (MainWin.MStandartLeiste);
gtk_container_add (GTK_CONTAINER (MainWin.MAnsicht_menu), MainWin.MStandartLeiste);
gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (MainWin.MStandartLeiste), TRUE);

MainWin.MSendenLeiste = gtk_check_menu_item_new_with_mnemonic (_("Show transmit bar"));
gtk_widget_show (MainWin.MSendenLeiste);
gtk_container_add (GTK_CONTAINER (MainWin.MAnsicht_menu), MainWin.MSendenLeiste);
gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (MainWin.MSendenLeiste), TRUE);

MainWin.MMakroLeiste = gtk_check_menu_item_new_with_mnemonic (_("Show macor bar"));
gtk_widget_show (MainWin.MMakroLeiste);
gtk_container_add (GTK_CONTAINER (MainWin.MAnsicht_menu), MainWin.MMakroLeiste);

widget = gtk_separator_menu_item_new ();
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (MainWin.MAnsicht_menu), widget);
gtk_widget_set_sensitive (widget, FALSE);

MainWin.MButtonsText = gtk_check_menu_item_new_with_mnemonic (_("Show button text"));
gtk_widget_show (MainWin.MButtonsText);
gtk_container_add (GTK_CONTAINER (MainWin.MAnsicht_menu), MainWin.MButtonsText);

MainWin.MOptionen = gtk_menu_item_new_with_mnemonic (_("_Options"));
gtk_widget_show (MainWin.MOptionen);
gtk_container_add (GTK_CONTAINER (menubar1), MainWin.MOptionen);

MainWin.MOptionen_menu = gtk_menu_new ();
gtk_menu_item_set_submenu (GTK_MENU_ITEM (MainWin.MOptionen), MainWin.MOptionen_menu);

MainWin.MEinstellungen = gtk_image_menu_item_new_with_mnemonic (_("_Setup"));
gtk_widget_show (MainWin.MEinstellungen);
gtk_container_add (GTK_CONTAINER (MainWin.MOptionen_menu), MainWin.MEinstellungen);

image = gtk_image_new_from_stock ("gtk-properties", GTK_ICON_SIZE_MENU);
gtk_widget_show (image);
gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (MainWin.MEinstellungen), image);

MainWin.MPluginsSetup = gtk_menu_item_new_with_mnemonic (_("_Plugins"));
gtk_widget_show (MainWin.MPluginsSetup);
gtk_container_add (GTK_CONTAINER (MainWin.MOptionen_menu), MainWin.MPluginsSetup);

widget = gtk_separator_menu_item_new ();
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (MainWin.MOptionen_menu), widget);
gtk_widget_set_sensitive (widget, FALSE);

MainWin.MConnect = gtk_menu_item_new_with_mnemonic (_("Established connection to Tiny-CAN"));
gtk_widget_show (MainWin.MConnect);
gtk_container_add (GTK_CONTAINER (MainWin.MOptionen_menu), MainWin.MConnect);

MainWin.MDeviceInfo = gtk_menu_item_new_with_mnemonic (_("Show Tiny-CAN information"));
gtk_widget_show (MainWin.MDeviceInfo);
gtk_container_add (GTK_CONTAINER (MainWin.MOptionen_menu), MainWin.MDeviceInfo);

MainWin.MHilfe = gtk_menu_item_new_with_mnemonic (_("_Help"));
gtk_widget_show (MainWin.MHilfe);
gtk_container_add (GTK_CONTAINER (menubar1), MainWin.MHilfe);

MainWin.MHilfe_menu = gtk_menu_new ();
gtk_menu_item_set_submenu (GTK_MENU_ITEM (MainWin.MHilfe), MainWin.MHilfe_menu);

MainWin.MInfo = gtk_menu_item_new_with_mnemonic (_("_About"));
gtk_widget_show (MainWin.MInfo);
gtk_container_add (GTK_CONTAINER (MainWin.MHilfe_menu), MainWin.MInfo);

MainWin.MainToolbar = gtk_toolbar_new ();
gtk_widget_show (MainWin.MainToolbar);
gtk_box_pack_start (GTK_BOX (vbox1), MainWin.MainToolbar, FALSE, FALSE, 0);
gtk_toolbar_set_style (GTK_TOOLBAR (MainWin.MainToolbar), GTK_TOOLBAR_BOTH);
tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (MainWin.MainToolbar));

image = gtk_image_new_from_stock ("gtk-new", tmp_toolbar_icon_size);
gtk_widget_show (image);
MainWin.NeuButton = (GtkWidget*) gtk_tool_button_new (image, _("New"));
gtk_widget_show (MainWin.NeuButton);
gtk_container_add (GTK_CONTAINER (MainWin.MainToolbar), MainWin.NeuButton);

image = gtk_image_new_from_stock ("gtk-open", tmp_toolbar_icon_size);
gtk_widget_show (image);
MainWin.FileOpenButton = (GtkWidget*) gtk_tool_button_new (image, _("Load"));
gtk_widget_show (MainWin.FileOpenButton);
gtk_container_add (GTK_CONTAINER (MainWin.MainToolbar), MainWin.FileOpenButton);

image = gtk_image_new_from_stock ("gtk-save", tmp_toolbar_icon_size);
gtk_widget_show (image);
MainWin.FileSaveButton = (GtkWidget*) gtk_tool_button_new (image, _("Save"));
gtk_widget_show (MainWin.FileSaveButton);
gtk_container_add (GTK_CONTAINER (MainWin.MainToolbar), MainWin.FileSaveButton);

widget = (GtkWidget*) gtk_separator_tool_item_new ();
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (MainWin.MainToolbar), widget);

image = gtk_image_new_from_stock ("gtk-quit", tmp_toolbar_icon_size);
gtk_widget_show (image);
MainWin.BeendenButton = (GtkWidget*) gtk_tool_button_new (image, _("Exit"));
gtk_widget_show (MainWin.BeendenButton);
gtk_container_add (GTK_CONTAINER (MainWin.MainToolbar), MainWin.BeendenButton);

widget = (GtkWidget*) gtk_separator_tool_item_new ();
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (MainWin.MainToolbar), widget);

MainWin.CanStartStopButton = (GtkWidget*) gtk_toggle_tool_button_new ();
gtk_tool_button_set_label (GTK_TOOL_BUTTON (MainWin.CanStartStopButton), _("Start"));
image = gtk_image_new_from_stock ("gtk-media-play", tmp_toolbar_icon_size);
gtk_widget_show (image);
gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (MainWin.CanStartStopButton), image);
gtk_widget_show (MainWin.CanStartStopButton);
gtk_container_add (GTK_CONTAINER (MainWin.MainToolbar), MainWin.CanStartStopButton);

image = gtk_image_new_from_stock ("gtk-stop", tmp_toolbar_icon_size);
gtk_widget_show (image);
MainWin.CanResetButton = (GtkWidget*) gtk_tool_button_new (image, _("CAN reset"));
gtk_widget_show (MainWin.CanResetButton);
gtk_container_add (GTK_CONTAINER (MainWin.MainToolbar), MainWin.CanResetButton);

widget = (GtkWidget*) gtk_separator_tool_item_new ();
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (MainWin.MainToolbar), widget);

image = gtk_image_new_from_stock ("gtk-properties", tmp_toolbar_icon_size);
gtk_widget_show (image);
MainWin.EinstellungenButton = (GtkWidget*) gtk_tool_button_new (image, _("Setup"));
gtk_widget_show (MainWin.EinstellungenButton);
gtk_container_add (GTK_CONTAINER (MainWin.MainToolbar), MainWin.EinstellungenButton);

hpaned1 = gtk_hpaned_new ();
gtk_widget_show (hpaned1);
gtk_box_pack_start (GTK_BOX (vbox1), hpaned1, TRUE, TRUE, 0);
gtk_paned_set_position (GTK_PANED (hpaned1), 220);

MainWin.MakrosFrame = gtk_frame_new (NULL);
gtk_widget_show (MainWin.MakrosFrame);
gtk_paned_pack1 (GTK_PANED (hpaned1), MainWin.MakrosFrame, FALSE, FALSE);
gtk_container_set_border_width (GTK_CONTAINER (MainWin.MakrosFrame), 5);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (MainWin.MakrosFrame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 5, 5, 5);

vbox2 = gtk_vbox_new (FALSE, 0);
gtk_widget_show (vbox2);
gtk_container_add (GTK_CONTAINER (widget), vbox2);

scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
gtk_widget_show (scrolledwindow2);
gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow2, TRUE, TRUE, 3);
gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_SHADOW_IN);

MainWin.MakroListView = gtk_tree_view_new ();
gtk_widget_show (MainWin.MakroListView);
gtk_container_add (GTK_CONTAINER (scrolledwindow2), MainWin.MakroListView);
gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (MainWin.MakroListView), FALSE);

MainWin.MakrosToolbar = gtk_toolbar_new ();
gtk_widget_show (MainWin.MakrosToolbar);
gtk_box_pack_start (GTK_BOX (vbox2), MainWin.MakrosToolbar, FALSE, FALSE, 0);
gtk_toolbar_set_style (GTK_TOOLBAR (MainWin.MakrosToolbar), GTK_TOOLBAR_BOTH);
tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (MainWin.MakrosToolbar));

image = gtk_image_new_from_stock ("gtk-redo", tmp_toolbar_icon_size);
gtk_widget_show (image);
MainWin.MakroSendenButton = (GtkWidget*) gtk_tool_button_new (image, _("Transmit"));
gtk_widget_show (MainWin.MakroSendenButton);
gtk_container_add (GTK_CONTAINER (MainWin.MakrosToolbar), MainWin.MakroSendenButton);

image = gtk_image_new_from_stock ("gtk-jump-to", tmp_toolbar_icon_size);
gtk_widget_show (image);
MainWin.MakroPasteButton = (GtkWidget*) gtk_tool_button_new (image, _("Paste"));
gtk_widget_show (MainWin.MakroPasteButton);
gtk_container_add (GTK_CONTAINER (MainWin.MakrosToolbar), MainWin.MakroPasteButton);

widget = gtk_label_new (_(" Macros "));
gtk_widget_show (widget);
gtk_frame_set_label_widget (GTK_FRAME (MainWin.MakrosFrame), widget);
gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);

vpaned1 = gtk_vpaned_new ();
gtk_widget_show (vpaned1);
gtk_paned_pack2 (GTK_PANED (hpaned1), vpaned1, TRUE, TRUE);
gtk_paned_set_position (GTK_PANED (vpaned1), 250);

MainWin.CanDataView = gtk_vbox_new (FALSE, 0);
gtk_widget_show (MainWin.CanDataView);
gtk_paned_pack1 (GTK_PANED (vpaned1), MainWin.CanDataView, FALSE, TRUE);

MainWin.CanFilterDataView = gtk_vbox_new (FALSE, 0);
gtk_widget_show (MainWin.CanFilterDataView);
gtk_paned_pack2 (GTK_PANED (vpaned1), MainWin.CanFilterDataView, TRUE, TRUE);

widget = gtk_label_new (_("<b>Filter messages</b>"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (MainWin.CanFilterDataView), widget, FALSE, FALSE, 0);
gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);

MainWin.CanTxWindow = gtk_vbox_new (FALSE, 0);
gtk_widget_show (MainWin.CanTxWindow);
gtk_box_pack_start (GTK_BOX (vbox1), MainWin.CanTxWindow, FALSE, FALSE, 0);

MainWin.Statusbar = gtk_statusbar_new ();
gtk_widget_show (MainWin.Statusbar);
gtk_box_pack_start (GTK_BOX (vbox1), MainWin.Statusbar, FALSE, FALSE, 0);
gtk_container_set_border_width (GTK_CONTAINER (MainWin.Statusbar), 2);

gtk_window_add_accel_group (GTK_WINDOW (MainWin.MainWin), accel_group);
}


void ShowMainWin(void)
{
gtk_widget_show(MainWin.MainWin);
}



#if defined(__WIN32__) && defined(_WINDOWS)
int APIENTRY WinMain(HINSTANCE hInstance,
           HINSTANCE hPrevInstance,
           LPSTR lpCmdLine, int nCmdShow)
{
return main(__argc, __argv);
}
#endif
