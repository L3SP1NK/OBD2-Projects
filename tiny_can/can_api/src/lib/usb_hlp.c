/***************************************************************************
                            usb_hlp.c  -  description
                             -------------------
    begin             : 19.01.2012
    last modify       : 05.05.2023    
    copyright         : (C) 2010 - 2023 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/

/**
    Library to talk to Tiny-CAN devices. You find the latest versions at
       http://www.tiny-can.com/
**/
#include "global.h"
#include <string.h>
#include "util.h"
#include "log.h"
//#include "tcan_drv.h" <*>
#include "global.h"
#include "usb_pnp.h"
#include "errors.h"
#include "index.h"
#include "mhs_thread.h"
#include "usb_hlp.h"

struct TTCanUsbList *TCanUsbList = NULL;
static int32_t LastPnPStatus = -1;
static struct TTCanUsbList *LastUsedDevice = NULL;


void UsbHlpInit(void)
{
TCanUsbList = NULL;
LastPnPStatus = -1;
LastUsedDevice = NULL;
}


void DestroyTCanUsbList(struct TTCanUsbList **list)
{
struct TTCanUsbList *ptr, *l;
int i;

if (!list)
  return;
l = *list;
while (l)
  {
  ptr = l->Next;
  for (i = 0; i < 4; i++)
    safe_free(l->DeviceName[i]);
  safe_free(l->Description);
  safe_free(l->Serial);
  mhs_free(l);
  l = ptr;
  }
*list = NULL;
LastUsedDevice = NULL;
}


struct TTCanUsbList *AddTCanUsbDevice(struct TTCanUsbList **list, char *devicename[], char *description,
                          char *serial, uint32_t type, uint32_t idx, const struct TCanModulDesc *modul)
{
struct TTCanUsbList *ptr, *neu;
int i;

if (!list)
  return(NULL);
neu = (struct TTCanUsbList *)mhs_calloc(1, sizeof(struct TTCanUsbList));
if (!neu)
  return(NULL);        // Nicht genügend Arbetsspeicher
neu->Next = NULL;
neu->DeviceType = type;
neu->TCanIdx = idx;
if (devicename)
  {
  for (i = 0; i < 4; i++)
    neu->DeviceName[i] = mhs_strdup(devicename[i]);
  }
neu->Description = mhs_strdup(description);
neu->Serial = mhs_strdup(serial);
neu->Modul = modul;
neu->Status = PNP_DEVICE_PLUGED;
ptr = *list;
if (!ptr)
  // Liste ist leer
  *list = neu;
else
  { // Neues Element anhängen
  while (ptr->Next != NULL) ptr = ptr->Next;
    ptr->Next = neu;
  }
return(neu);
}


struct TTCanUsbList *GetUsbDevice(char *snr)
{
struct TTCanUsbList *usb_list;

if (!MhsPnP)
  return(NULL);
PnPLock();
// *** Liste der angeschlossenen FTDI-USB Devices erzeugen
UpdateTCanUsbList();

#if defined(ENABLE_LOG_SUPPORT) && defined(MHSTCAN_DRV)
LogPrintTCanUsbList(LOG_USB, TCanUsbList, "Get Usb Device: %s, Hardware Liste:", snr);
#endif
// *** Device Namen u. BaudRate festlegen
usb_list = TCanUsbList;
if ((snr) && (strlen(snr)))
  {  // Device mit entsprechender Snr. suchen
  for (; usb_list; usb_list = usb_list->Next)
    {
    if (usb_list->Status < PNP_DEVICE_PLUGED)
      continue;
#ifndef SLCAN_DRV      
    if (!usb_list->Modul)
      continue;
#endif        
    if (!safe_strcmp(snr, usb_list->Serial))
      break;
    }
  }
else
  {  // Nach einen beliebigen Tiny-CAN Modul suchen
  for (; usb_list; usb_list = usb_list->Next)
    {
    if (usb_list->Status < PNP_DEVICE_PLUGED)
      continue;
#ifndef SLCAN_DRV      
    if (usb_list->Modul)
#endif    
      break;
    }
  }
PnPUnlock();
LastUsedDevice = usb_list;
#if defined(ENABLE_LOG_SUPPORT) && defined(MHSTCAN_DRV)
if (!usb_list)
  LogPrintf(LOG_USB, "Get Usb Device Return: kein Device");
else
  LogPrintf(LOG_USB, "Get Usb Device Return Snr: %s [%s]", usb_list->Serial, usb_list->Description);
#endif
return(usb_list);
}


int32_t GetDevicePnPStatus(void)
{
struct TTCanUsbList *usb_list;
int32_t plug;

PnPLock();
plug = -1; 
if (LastUsedDevice)
  {
  if (LastUsedDevice->Status < PNP_DEVICE_PLUGED)
    {
    plug = 0;
    LastUsedDevice = NULL;
    }
  else
    plug = 1;
  }
else
  {
  plug = 0;
  for (usb_list = TCanUsbList; usb_list; usb_list = usb_list->Next)
    {
    if (usb_list->Status >= PNP_DEVICE_PLUGED)
      {
      plug = 1;
      break;
      }
    }
  }
if ((plug > -1) && (LastPnPStatus != plug))
  LastPnPStatus = plug;
else
  plug = -1;
PnPUnlock();
return(plug);
}
