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
/*                            CAN Funktionen                              */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - CAN Treiber laden und Initialisieren              */
/*                    - CAN Interface konfigurieren                       */
/*                    - CAN Status/Nachrichten lesen                      */
/*                    - CAN Message Tabelle generieren                    */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : main_can.c                                          */
/* ---------------------------------------------------------------------- */
/*  Datum           : 17.10.05                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include "can_monitor.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "setup.h"
#include "filter.h"
#include "const.h"
#include "dialogs.h"
#include "makro.h"
#include "cview.h"
#include "support.h"
#include "drv_scan.h"
#include "can_monitor_obj.h"
#include "main_can.h"

#define SIZE_CAN_RXD_PUFFER 100

struct TCanModulDesc
  {
  uint32_t HwId;
  uint32_t CanFeaturesFlags;
  uint32_t HwRxFilterCount;
  uint32_t HwTxPufferCount;
  };

static const struct TCanModulDesc CanModulTable[] =
  //   ID    |Features|Filter|Puffer
  //         | Flags  |Count |Count
{{0x43414E01,  0x00,     4,     4,  },  // Tiny-CAN I
 {0x43414E81,  0x00,     4,     4,  },  // Tiny-CAN I / 10kBit
 {0x43414E41,  0x00,     4,     4,  },  // Tiny-CAN IB
 {0x43414EC1,  0x00,     4,     4,  },  // Tiny-CAN IB / 10kBit
 {0x43414E02,  0x00,     4,     4,  },  // Tiny-CAN II
 {0x43414E82,  0x00,     4,     4,  },  // Tiny-CAN II / 10 kBit
 {0x43414E42,  0x05,     4,     4,  },  // Tiny-CAN II-XL
 {0x43414E03,  0x00,     8,    16,  },  // Tiny-CAN III
 {0x43414E83,  0x00,     8,    16,  },  // Tiny-CAN III / 10 kBit
 {0x43414E43,  0x07,     8,    16,  },  // Tiny-CAN III-XL
 {0x43414E04,  0x05,     4,     4,  },  // Tiny-CAN M1
 {0x43414E05,  0x05,     8,     4,  },  // Tiny-CAN M232
 {0x43414E06,  0x07,     4,     8,  },  // Tiny-CAN IV-XL
 {0x00000000,  0x00,     0,     0,  }}; // Ende Tabelle

static const struct TCanModulDesc DefaultModul =
 {0x43414E01,  0x00,     4,     4   };  // Tiny-CAN I



struct TCanMsg CanRxDPuffer[SIZE_CAN_RXD_PUFFER];
void CALLBACK_TYPE CanPnPEvent(uint32_t index, int32_t status);
void CALLBACK_TYPE CanStatusEvent(uint32_t index, struct TDeviceStatus *DeviceStatus);
void CALLBACK_TYPE CanRxEvent(uint32_t index, struct TCanMsg *msg, int32_t count);

static const char CanInitStr[] = {"CanRxDMode=1;CanRxDFifoSize=32768;CanTxDFifoSize=2048;CanRxDBufferSize=800"};

struct TCanModul CanModul;

static void ExtractHardwareInfo(char *str, struct TCanModul *modul)
{
int match;
char *tmpstr, *s, *key, *val;
uint32_t hw_id, id;
const struct TCanModulDesc *modul_desc;

tmpstr = g_strdup(str);
s = tmpstr;
modul->CanPortCount = 1;
modul->IntPufferCount = 0xFFFFFFFF;
modul->FilterCount = 0xFFFFFFFF;
modul->CanFeaturesFlags = 0xFFFFFFFF;
do
  {
  // Bezeichner auslesen
  key = GetItemAsString(&s, ":=", &match);
  if (match <= 0)
    break;
  // Value auslesen
  val = GetItemAsString(&s, ";", &match);
  if (match < 0)
    break;
  if (!g_ascii_strcasecmp(key, "ID"))
    {
    id = g_ascii_strtod(val, NULL);
    continue;
    }
  if (!g_ascii_strcasecmp(key, "ANZAHL CAN INTERFACES"))
    {
    modul->CanPortCount = (unsigned int)g_ascii_strtod(val, NULL);
    continue;
    }
  else if (!g_ascii_strcasecmp(key, "ANZAHL INTERVAL PUFFER"))
    {
    modul->IntPufferCount = (unsigned int)g_ascii_strtod(val, NULL);
    continue;
    }
  else if (!g_ascii_strcasecmp(key, "ANZAHL FILTER"))
    {
    modul->FilterCount = (unsigned int)g_ascii_strtod(val, NULL);
    continue;
    }
  else if (!g_ascii_strcasecmp(key, "CAN FEATURES FLAGS"))
    {
    modul->CanFeaturesFlags = (unsigned int)g_ascii_strtod(val, NULL);
    continue;
    }
  }
while(1);
modul->Id = id;
for (modul_desc = &CanModulTable[0]; (hw_id = modul_desc->HwId); modul_desc++)
  {
  if (hw_id == id)
    break;
  }
if (!hw_id)
  {
  if (modul->IntPufferCount == 0xFFFFFFFF)
    modul->IntPufferCount = modul_desc->HwTxPufferCount;
  if (modul->FilterCount == 0xFFFFFFFF)
    modul->FilterCount = modul_desc->HwRxFilterCount;
  if (modul->CanFeaturesFlags == 0xFFFFFFFF)
    modul->CanFeaturesFlags = modul_desc->CanFeaturesFlags;
  }
save_free(tmpstr);
}


int SetupCanSetMode(unsigned long index, unsigned char can_op_mode, unsigned short can_command)
{
if (can_op_mode == OP_CAN_START)
  {
  switch (SetupData.CanOpMode)
    {
    case 0  : { // Normal
              can_op_mode = OP_CAN_START;
              break;
              }
    case 1  : { // Automatic Retransmission disable
              if (CanModul.CanFeaturesFlags & CAN_FEATURE_ARD)
                can_op_mode = OP_CAN_START_NO_RETRANS;
              else
                can_op_mode = OP_CAN_START;
              break;
              }
    case 2  : { // Silent Mode (LOM = Listen only Mode)
              if (CanModul.CanFeaturesFlags & CAN_FEATURE_LOM)
                can_op_mode = OP_CAN_START_LOM;
              else
                can_op_mode = OP_CAN_START;
              break;
              }
    default : {
              can_op_mode = OP_CAN_START;
              }
    }
  }
return(CanSetMode(index, can_op_mode, can_command));
}


/*
******************** TinyCanInit ********************
Funktion  : Low Level Initialisierung

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
void TinyCanInit(void)
{
memset(&CanModul, 0, sizeof(struct TCanModul));
}


static void TinyCanDownDriver(void)
{
TinyCanDisconnect();
CanSetEvents(EVENT_DISABLE_ALL);   // Alle Events löschen
while (!CanEventStatus())
  {
  UpdateGTK();
  }

CanModul.Online = 0;
CanDownDriver();   // Treiber freigeben
}


static void TinyCanUnloadDriver(void)
{
TinyCanDownDriver();
UnloadDriver();      // Treiber freigeben
}


/*
******************** TinyCanDestroy ********************
Funktion  : CAN Treiber (DLL) beenden

Eingaben  : keine

Ausgaben  : keine

Call's    : UnloadDriver
*/
void TinyCanDestroy(void)
{
TinyCanUnloadDriver();
save_free(CanModul.DriverInfo);
save_free(CanModul.HardwareInfoStr);
save_free(CanModul.FileName);
save_free(CanModul.CanDeviceOpenStr);
save_free(CanModul.CanInitDriverStr);
}


/*
******************** TinyCanDisconnect ********************
Funktion  : Schnittstelle PC <-> Tiny-CAN schließen

Eingaben  : keine

Ausgaben  : keine

Call's    : CanDeviceClose, CanGetDeviceStatus
*/
void TinyCanDisconnect(void)
{
CanDeviceClose(0L);                      // Can Schnittstelle schließen
CanModul.Online = 0;                     // Can Bus Offline
(void)CanGetDeviceStatus(0L, &CanModul.DeviceStatus);
}


/*
******************** TinyCanConnect ********************
Funktion  : Tiny-CAN Treiber laden, Treiber und Tiny-CAN Modul
            Initialisieren, Status das Tiny-CAN Moduls abfragen

Eingaben  : keine

Ausgaben  : result => Fehlercode 0 = kein Fehler

Call's    : CanGetDeviceStatus, LoadDriver, CanInitDriver, CanSetOptions,
            CanSetSpeed, CanDeviceOpen, CanDeviceClose, UnloadDriver
*/
int TinyCanConnect(unsigned int mode)
{
int err;
char *driver_full_filename;
char *str;
unsigned int i, t;

// Neuen Treiber ?
if ((!SetupData.DriverPath) || (!SetupData.DriverFile))
  {
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error loading Tiny-CAN API driver"));
  return(-1);
  }
driver_full_filename = g_build_filename(SetupData.DriverPath, SetupData.DriverFile, NULL);

if (mhs_strcmp0(CanModul.FileName, driver_full_filename) != 0)
  {
  save_free(CanModul.FileName);
  CanModul.FileName = driver_full_filename;
  driver_full_filename = NULL;
  DriverInfoFree(&CanModul.DriverInfo);
  mode = TINY_CAN_DRIVER_RELOAD;
  }
save_free(driver_full_filename);
// Treiber Info prüfen und anlegen
if (!CanModul.DriverInfo)
  CanModul.DriverInfo = GetDriverInfo(CanModul.FileName);
if (!CanModul.DriverInfo)
  {
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error loading Tiny-CAN API driver"));
  return(-1);
  }
// **** CAN Driver Init. Configuration erstellen
if (SetupData.ShowTime > 4)
  i = 3;
else
  i = 1;
if (SetupData.DriverInitStr)
  str = g_strdup_printf("ComDrvType=%u;%s;TimeStampMode=%u;%s", SetupData.ComDrvMode, CanInitStr, i, SetupData.DriverInitStr);
else
  str = g_strdup_printf("ComDrvType=%u;%s;TimeStampMode=%u", SetupData.ComDrvMode, CanInitStr, i);

if (mhs_strcmp0(str, CanModul.CanInitDriverStr))
  {
  save_free(CanModul.CanInitDriverStr);
  CanModul.CanInitDriverStr = str;
  str = NULL;
  if (mode < TINY_CAN_DRIVER_RELOAD)
    mode = TINY_CAN_DRIVER_REINIT;
  }
save_free(str);

// **** CAN Device Open Configuration erstellen
str = NULL;
#ifdef __WIN32__
if (SetupData.ComDrvMode == 0)
  {   // RS232
  str = g_strdup_printf("Port=%u;BaudRate=%u", SetupData.PortIndex, (unsigned int)SetupData.BaudRate);
  }
else
  {   // USB
  if ((SetupData.SnrEnabled) && (strlen(SetupData.Snr)))
    str = g_strdup_printf("Snr=%s", SetupData.Snr);
  else
    str = g_strdup("Snr=");
  }
#else
if ((SetupData.SnrEnabled) && (strlen(SetupData.Snr)))
  {
  if ((SetupData.ComDeviceName) && (strlen(SetupData.ComDeviceName)))
    str = g_strdup_printf("COMDEVICENAME=%s;Snr=%s", SetupData.ComDeviceName, SetupData.Snr);
  else
    str = g_strdup_printf("COMDEVICENAME=;Snr=%s", SetupData.Snr);
  }
else if ((SetupData.ComDeviceName) && (strlen(SetupData.ComDeviceName)))
  str = g_strdup_printf("Snr=;COMDEVICENAME=%s", SetupData.ComDeviceName);
else
  str = g_strdup("COMDEVICENAME=;Snr=");
#endif

if (mhs_strcmp0(str, CanModul.CanDeviceOpenStr))
  {
  save_free(CanModul.CanDeviceOpenStr);
  CanModul.CanDeviceOpenStr = str;
  str = NULL;
  if (mode < TINY_CAN_DRIVER_REINIT)
    mode = TINY_CAN_DRIVER_REOPEN;
  }
save_free(str);

if (mode)
  {
  if (mode == TINY_CAN_DRIVER_RELOAD)
    TinyCanUnloadDriver();
  else if (mode == TINY_CAN_DRIVER_REINIT)
    TinyCanDownDriver();
  else if (mode == TINY_CAN_DRIVER_REOPEN)
    TinyCanDisconnect();
  mhs_sleep(500);
  }
// Treiber Status abfragen ?
if (CanGetDeviceStatus(0L, &CanModul.DeviceStatus))
  return(ERR_CAN_CONNECT_FATAL);

err = 0;
switch (CanModul.DeviceStatus.DrvStatus)
  {
  case DRV_NOT_LOAD             : {  // Die Treiber DLL wurde noch nicht geladen
                                  if (LoadDriver(CanModul.FileName) < 0)
                                    {
                                    err = ERR_CAN_LOAD_DRIVER;
                                    break;
                                    }
                                  }
  case DRV_STATUS_NOT_INIT      : {  // Treiber noch nicht Initialisiert
                                  if (CanInitDriver(CanModul.CanInitDriverStr) < 0)
                                    {
                                    err = ERR_CAN_CONNECT_FATAL;
                                    break;
                                    }
                                  CanSetPnPEventCallback(&CanPnPEvent);
                                  CanSetStatusEventCallback(&CanStatusEvent);
                                  CanSetRxEventCallback(&CanRxEvent);
                                  CanSetEvents(EVENT_ENABLE_ALL);  // Alle Events freigeben
                                  }
  case DRV_STATUS_INIT          :    // Treiber erfolgrich Initialisiert
  case DRV_STATUS_PORT_OPEN     :    // Die Schnittstelle wurde geöffnet
  case DRV_STATUS_DEVICE_FOUND  : {  // Verbindung zur Hardware wurde Hergestellt
                                  save_free(CanModul.HardwareInfoStr);
                                  if (CanDeviceOpen(0L, CanModul.CanDeviceOpenStr) < 0)
                                    {
                                    err = ERR_CAN_PC_PORT_OPEN_ERROR;
                                    break;
                                    }
                                  if ((str = CanDrvHwInfo(0L)))
                                    {
                                    CanModul.HardwareInfoStr = g_strdup(str);
                                    ExtractHardwareInfo(CanModul.HardwareInfoStr, &CanModul);
                                    }
                                  else
                                    err = ERR_CAN_PC_PORT_OPEN_ERROR;
                                  break;
                                  }
  case DRV_STATUS_PORT_NOT_OPEN : {  // Die Schnittstelle wurde nicht geöffnet
                                  err = ERR_CAN_PC_PORT_OPEN_ERROR;
                                  break;
                                  }
  }

(void)CanGetDeviceStatus(0L, &CanModul.DeviceStatus);

i = 0;
if (CanModul.DeviceStatus.DrvStatus >= DRV_STATUS_INIT)
  {
  // **** Can Speed einstellen
  if (!SetupData.CanSpeedMode)
    CanSetSpeed(0L, SetupData.CanSpeed);
  else
    CanSetSpeedUser(0L, SetupData.UserCanSpeed);
  // **** AutoConnect, FilterReadIntervall, CanTxAckEnable einstellen
  switch (SetupData.FilterTimer)
    {
    case 0 : {  // 100 ms
             t = 100;
             break;
             }
    case 1 : {  // 250 ms
             t = 250;
             break;
             }
    case 2 : {  // 500 ms
             t = 500;
             break;
             }
    case 3 : {  // 1s
             t = 1000;
             break;
             }
    default : t = 1000;
    }
  if (CanModul.CanFeaturesFlags & CAN_FEATURE_TX_ACK)
    str = g_strdup_printf("CanTxAckEnable=%u;AutoConnect=%u;FilterReadIntervall=%u", SetupData.ShowDir, SetupData.AutoConnect, t);
  else
    str = g_strdup_printf("AutoConnect=%u;FilterReadIntervall=%u", SetupData.AutoConnect, t);
  CanSetOptions(str);
  save_free(str);
  // **** CAN Bus starten
  if (SetupCanSetMode(0L, OP_CAN_START, CAN_CMD_ALL_CLEAR) < 0)
    err = ERR_CAN_NOT_START;
  else
    {
    // Filter Liste neu erzeugen
    FilCreateIndexList();
    FilSetToDriver(0);
    // Intervall Messages setzen
    MakSetIntervall();
    }
  }

(void)CanGetDeviceStatus(0L, &CanModul.DeviceStatus);
if (CanModul.DeviceStatus.DrvStatus >= DRV_STATUS_CAN_OPEN)
  CanModul.Online = 1;

// Signale senden
if (i)
  mhs_signal_emit(SIGC_PUBLIC, SIG_FILTER_EDIT_FINISH, NULL);

mhs_signal_emit(SIGC_PUBLIC, SIG_CAN_MODUL_INIT, NULL);
return(err);
}



/**************************************************************/
/* Liste lï¿½chen                                              */
/**************************************************************/
void ClearData(void)
{
SetupCanSetMode(0L, OP_CAN_NO_CHANGE, CAN_CMD_RXD_FIFOS_CLEAR | CAN_CMD_TXD_FIFOS_CLEAR);
CViewClearAll(MainWin.CanView);
}


void SyncCanBufferFull(struct TMhsObj *obj, gpointer user_data)
{
DataRecord = DATA_RECORD_LIMIT;
SetDataRecord();
}

void SyncCanPnPEvent(struct TMhsObj *obj, int status, gpointer user_data)
{
if (status)
  {  // Tiny-CAN Hardware wurde verbunden
  (void)TinyCanConnect(0);
  }
else
  {  // Tiny-CAN Hardware wurde getrennt
  DataRecord = DATA_RECORD_STOP;
  SetDataRecord();
  }
(void)CanGetDeviceStatus(0L, &CanModul.DeviceStatus);
mhs_signal_emit(SIGC_PUBLIC, SIG_CAN_STATUS_CHANGE, NULL);
}


void SyncCanStatusEvent(struct TMhsObj *obj, struct TDeviceStatus *device_status, gpointer user_data)
{

if (device_status->FifoStatus & FIFO_HW_SW_OVERRUN)
  DataRecord = DATA_RECORD_OV;
//CanGetDeviceStatus(0L, &CanModul.DeviceStatus);
memcpy(&CanModul.DeviceStatus, device_status, sizeof(struct TDeviceStatus));
mhs_signal_emit(SIGC_PUBLIC, SIG_CAN_STATUS_CHANGE, NULL);
}


void SyncRxDFilterEvent(struct TMhsObj *obj, gpointer user_data)
{
struct TFilter *filter;
struct TCanMsg msg;

filter = FilReadUpdMessage(NULL, &msg);
while (filter)
  {
  mhs_signal_emit(SIGC_CAN, filter->Index, &msg);
  filter = FilReadUpdMessage(filter, &msg);
  }
CViewQueueDraw(MainWin.CanFilterView);
}


struct TCanMsg *FilGetMsgCB(unsigned long index)
{
struct TFilter *filter;

filter = FilGetByPos(FILTER_VISIBLE, index);
if (!filter)
  return(NULL);
else
  return(&filter->CanMsg);
}


char *FilGetNameCB(unsigned long index)
{
struct TFilter *filter;

filter = FilGetByPos(FILTER_VISIBLE, index);
if (!filter)
  return(NULL);
else
  return(filter->Name);
}