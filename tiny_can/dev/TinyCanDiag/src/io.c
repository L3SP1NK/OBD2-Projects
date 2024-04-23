/***************************************************************************
                            io.c  -  description
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
#include <glib.h>
#include "dialogs.h"
#include "gtk_util.h"
#include "can_drv.h"
#include "hw_types.h"
#include "io.h"

//#define TREIBER_NAME "X:\\tiny_can\\treiber-ex\\mhstcan\\windows\\Debug\\mhstcan.dll" // <*>
//#define TREIBER_INIT "LogFlags=0x0023;LogFile=log.txt;CanCallThread=0" <*>

#if defined(G_OS_WIN32)
  #define TREIBER_NAME NULL
#else
  #define TREIBER_NAME "/opt/tiny_can/can_api/libmhstcan.so" 
#endif  
#define TREIBER_INIT "CanCallThread=0"

#define CAN0_SPEED 1000 // 1 MBit/s
#define CAN1_SPEED 500 // 500 kBit/s

struct TCanModulDesc
  {
  uint32_t HwId;
  guint HardwareType;
  };

static const struct TCanModulDesc CanModulTable[] =
  //   ID      | Hardware Type
  //           |
  {{0x43414E08,  HARDWARE_TYPE_TCAN_M2},  // Tiny-CAN M2
   {0x43414E09,  HARDWARE_TYPE_TCAN_M3},  // Tiny-CAN M2
   {0x0000000,   0}};


static gint ExtractHardwareInfo(const char *str, struct TCanHw *hw)
{
int match;
gint res;
char *tmpstr, *s, *key, *val;
uint32_t hw_id, id;
const struct TCanModulDesc *modul_desc;

res = 0;
id = 0;
tmpstr = g_strdup(str);
s = tmpstr;
hw->CanPortCount = 1;
do
  {
  // Bezeichner auslesen
  key = get_item_as_string(&s, ":=", &match);
  if (match <= 0)
    break;
  // Value auslesen
  val = get_item_as_string(&s, ";", &match);
  if (match < 0)
    break;
  if (!g_ascii_strcasecmp(key, "ID"))
    {
    id = g_ascii_strtod(val, NULL);
    continue;
    }
  if (!g_ascii_strcasecmp(key, "ANZAHL CAN INTERFACES"))
    {
    hw->CanPortCount = (unsigned int)g_ascii_strtod(val, NULL);
    continue;
    }
  }
while(1);
for (modul_desc = &CanModulTable[0]; (hw_id = modul_desc->HwId); modul_desc++)
  {
  if (hw_id == id)
    break;
  }
hw->ConnectedHardwareType = modul_desc->HardwareType;

if (hw->ConnectedHardwareType != hw->HardwareType)
  res = -1;
safe_free(tmpstr);
return(res);
}


int32_t TinyCanOpen(struct TCanHw *hw)
{
int32_t err;
char *str;

/******************************/
/*  Initialisierung           */
/******************************/

safe_free(hw->HardwareInfoStr);
// **** Initialisierung Utility Funktionen
// **** Treiber DLL laden
if ((err = LoadDriver(TREIBER_NAME)) < 0)
  {
  msg_box(MSG_TYPE_ERROR, "Error", "LoadDriver Error-Code:%d", err);
  return(err);
  }
// **** Treiber DLL initialisieren
// Keinen Callback Thread erzeugen, die Callback Funktionen stehen
// nicht zur verfügung
if ((err = CanExInitDriver(TREIBER_INIT)) < 0)
  {
  msg_box(MSG_TYPE_ERROR, "Error", "CanInitDrv Error-Code:%d", err);
  return(err);
  }

// **** Device u. Empfangs-FIFO für das Device erzeugen
if ((err = CanExCreateDevice(&hw->DeviceIndex, "CanRxDFifoSize=16384")) < 0)
  {
  msg_box(MSG_TYPE_ERROR, "Error", "CanExCreateDevice Error-Code:%d", err);
  return(err);
  }
// **** Schnittstelle PC <-> Tiny-CAN öffnen
if ((err = CanDeviceOpen(hw->DeviceIndex, NULL)) < 0)
  {
  msg_box(MSG_TYPE_ERROR, "Error", "CanDeviceOpen Error-Code:%d", err);
  return(err);
  }

err = 0;
if ((str = CanDrvHwInfo(0L)))
  {
  hw->HardwareInfoStr = g_strdup(str);
  err = ExtractHardwareInfo(hw->HardwareInfoStr, hw);
  }
if (err)
  {
  msg_box(MSG_TYPE_ERROR, "Error", "Hardware Type stimmt mit Projekt Type nicht überein");
  return(err);
  }
/*****************************************/
/*  CAN Speed einstellen & Bus starten   */
/*****************************************/
// **** Übertragungsgeschwindigkeit einstellen
CanSetSpeed(INDEX_CHANNEL(hw->DeviceIndex, 0), CAN0_SPEED);
if (hw->HardwareType == HARDWARE_TYPE_TCAN_M3)
  CanSetSpeed(INDEX_CHANNEL(hw->DeviceIndex, 1), CAN1_SPEED);

// Achtung: Um Fehler auf dem Bus zu vermeiden ist die Übertragungsgeschwindigkeit
//          vor dem starten des Busses einzustellen.

// **** CAN Bus Start, alle FIFOs, Filter, Puffer und Fehler löschen
CanSetMode(INDEX_CHANNEL(hw->DeviceIndex, 0), OP_CAN_START, CAN_CMD_ALL_CLEAR);
if (hw->HardwareType == HARDWARE_TYPE_TCAN_M3)
  CanSetMode(INDEX_CHANNEL(hw->DeviceIndex, 1), OP_CAN_START, CAN_CMD_ALL_CLEAR);

return(0);
}


int32_t TinyCanSetIoConfig(struct TCanHw *hw)
{
int32_t err;
uint32_t idx, cfg, sub_idx, c, cfg_idx;
gchar str[20];

cfg_idx = 0;
err = 0;
for (idx = 0; idx < hw->IoPinsCount; )
  {
  cfg = 0;
  for (sub_idx = 0; sub_idx < 8; sub_idx++)
    {
    c = hw->IoPin[idx++].Mode;
    if (c > PIN_CFG_ENCODER)
      c = PIN_CFG_ENCODER;
    cfg |= (c << (sub_idx * 4));
    if (idx >= hw->IoPinsCount)
      break;
    }
  g_snprintf(str, 20, "IoCfg%u", cfg_idx++);
  if ((err = CanExSetAsULong(hw->DeviceIndex, str, cfg)) < 0)
    break;
  }
return(err);
}


void TinyCanClose(struct TCanHw *hw)
{
// Device schließen
(void)CanDeviceClose(hw->DeviceIndex);
// Device löschen
(void)CanExDestroyDevice(&hw->DeviceIndex);

CanDownDriver();
// **** DLL entladen
UnloadDriver();
}


int GetPinIndex(struct TCanHw *hw, int pin, int pin_type)
{
int i, idx;

idx = 0;
if (pin >= (int)hw->IoPinsCount)
  return(-1);
for (i = 0; i <= pin; i++)
  {
  if (hw->IoPinDefs[i].Flags & pin_type)
    {
    if (i == pin)
      return(idx);
    idx++;
    }
  }
return(-1);
}


int32_t SetOutPin(struct TCanHw *hw, int pin, int state)
{
int32_t err;
uint16_t idx, bit, out;
gchar str[20];

err = 0;
idx = pin / 16;
bit = 0x0001 << (pin % 16);
g_snprintf(str, 20,"IoPort%uOut", idx);
if ((err = CanExGetAsUWord(hw->DeviceIndex, str, &out)) < 0)
  msg_box(MSG_TYPE_ERROR, "Error", "ERROR (PIN:%u) set_pin: %d", pin, err);
else
  {
  if (state)
    out |= bit;
  else
    out &= (~bit);
  if ((err = CanExSetAsUWord(hw->DeviceIndex, str, out)) < 0)
    msg_box(MSG_TYPE_ERROR, "Error", "ERROR (PIN:%u) set_pin: %d", pin, err);
  }
return(err);
}


int32_t SetAnalogOutPin(struct TCanHw *hw, int pin, uint16_t value)
{
int32_t err, idx;
gchar str[20];

err = 0;
if ((idx = GetPinIndex(hw, pin, PIN_A_OUT)) < 0)
  {
  msg_box(MSG_TYPE_ERROR, "Error", "ERROR (PIN:%u) Pin nicht als Analog-Output konfigurierbar", pin);
  return(-1);
  }
g_snprintf(str, 20,"IoPort%uAnalogOut", idx);
if ((err = CanExSetAsUWord(hw->DeviceIndex, str, value)) < 0)
  msg_box(MSG_TYPE_ERROR, "Error", "ERROR (PIN:%u) set_pin: %d", pin, err);
return(err);
}


int32_t GetAnalogInPin(struct TCanHw *hw, int pin, uint16_t *value)
{
int32_t err, idx;
gchar str[20];

err = 0;
if ((idx = GetPinIndex(hw, pin, PIN_A_IN)) < 0)
  {
  msg_box(MSG_TYPE_ERROR, "Error", "ERROR (PIN:%u) Pin nicht als Analog-Input konfigurierbar", pin);
  return(-1);
  }
g_snprintf(str, 20,"IoPort%uAnalog", idx);
if ((err = CanExGetAsUWord(hw->DeviceIndex, str, value)) < 0)
  msg_box(MSG_TYPE_ERROR, "Error", "ERROR (PIN:%u) set_pin: %d", pin, err);
return(err);
}


int32_t GetEncoderInPin(struct TCanHw *hw, int pin, int32_t *value)
{
int32_t err, idx;
gchar str[20];

err = 0;
if ((idx = GetPinIndex(hw, pin, PIN_ENC_A)) < 0)
  {
  msg_box(MSG_TYPE_ERROR, "Error", "ERROR (PIN:%u) Pin nicht als Encoder-Input konfigurierbar", pin);
  return(-1);
  }
// idx /= 2; <*>
g_snprintf(str, 20,"IoPort%uEnc", idx);
if ((err = CanExGetAsLong(hw->DeviceIndex, str, value)) < 0)
  msg_box(MSG_TYPE_ERROR, "Error", "ERROR (PIN:%u) set_pin: %d", pin, err);
return(err);
}


int32_t GetDigInPort(struct TCanHw *hw, int port, uint16_t *value)
{
int32_t err;
gchar str[20];

err = 0;
g_snprintf(str, 20,"IoPort%uIn", port);
if ((err = CanExGetAsUWord(hw->DeviceIndex, str, value)) < 0)
  msg_box(MSG_TYPE_ERROR, "Error", "ERROR (Port:%u) set_pin: %d", port, err);
return(err);
}


int32_t GetKeycode(struct TCanHw *hw, uint32_t *value)
{
int32_t err;

err = 0;
if ((err = CanExGetAsULong(hw->DeviceIndex, "Keycode", value)) < 0)
  msg_box(MSG_TYPE_ERROR, "Error", "ERROR GetKey: %d", err);
return(err);
}


