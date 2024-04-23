/**************************************************************************/
/*                Tiny-CAN API Demoprogramm "ex_sample6"                  */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL und Initialisierung       */
/*                      im EX-Modus                                       */
/*                    - Setzen der Callbackfunktionen                     */
/*                    - An- und Abstecken der Hardware, Statusänderungen  */
/*                      und den Empfang von CAN-Messages in Callback-     */
/*                      Funktionen verarbeiten                            */
/*                    - Device Liste auf dem Bildschim ausgeben           */
/*                    - Öffnen u. schließen angeschlossener Devices       */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 09.08.11                                            */
/*  Autor           : Demlehner Klaus, MHS-Elektronik, 94149 Kößlarn      */
/*                    info@mhs-elektronik.de  www.mhs-elektronik.de       */
/**************************************************************************/
#include "config.h"
#include "global.h"
#include <string.h>
#include <stdio.h>

#include "util.h"
#ifdef __WIN32__
#include <conio.h>
#else
#include <pthread.h>
#include "linux_util.h"
#endif
#include "can_drv.h"


const char *DrvStatusStrings[] =
     {"NOT_LOAD",      // Die Treiber DLL wurde noch nicht geladen
      "NOT_INIT",      // Treiber noch nicht Initialisiert (Funktion "CanInitDrv" noch nicht aufgerufen)
      "INIT",          // Treiber erfolgrich Initialisiert
      "PORT_NOT_OPEN", // Die Schnittstelle wurde nicht geöffnet
      "PORT_OPEN",     // Die Schnittstelle wurde geöffnet
      "DEVICE_FOUND",  // Verbindung zur Hardware wurde Hergestellt
      "CAN_OPEN",      // Device wurde geöffnet und erfolgreich Initialisiert
      "CAN_RUN_TX",    // CAN Bus RUN nur Transmitter (wird nicht verwendet !)
      "CAN_RUN"};      // CAN Bus RUN

const char *CanStatusStrings[] =
     {"OK",            // Ok
      "ERROR",         // CAN Error
      "WARNING",       // Error warning
      "PASSIV",        // Error passiv
      "BUS_OFF",       // Bus Off
      "UNBEKANNT"};    // Status Unbekannt

const char *CanFifoStrings[] =
     {"OK",
      "HW_OVERRUN",
      "SW_OVERRUN",
      "HW_SW_OVERRUN",
      "STATUS_UNBEKANNT"};


DRV_LOCK_TYPE Lock;
uint32_t DeviceIndex[4];
int32_t DevicesListCount = 0;
struct TCanDevicesList *DevicesList = NULL;


void PrintUsbDevices(void)
{
int32_t i;
uint32_t idx;
char *str;
char str_puf[100];
struct TCanDevicesList *l;

l = DevicesList;
if (DevicesListCount)
  {
  printf("=========================================================================\n");
  for (i = 0; i < DevicesListCount; i++)
    {
    idx = l[i].TCanIdx;
    // l[i].DeviceName
    if (idx == INDEX_INVALID)
      printf("Key '%d', %s [%s]\n\r", i+1, l[i].Description, l[i].SerialNumber);
    else
      printf("Key '%d', %s [%s] Open: 0x%08X\n\r", i+1, l[i].Description, l[i].SerialNumber, idx);
    printf("   Id               : 0x%08X\n\r", l[i].HwId);
    if (l[i].HwId)
      {
      printf("   CanClock         : %u\n\r", l[i].ModulFeatures.CanClock);
      str = str_puf;
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_LOM)            // Silent Mode (LOM = Listen only Mode)
        str = mhs_stpcpy(str, "LOM ");
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_ARD)            // Automatic Retransmission disable
        str = mhs_stpcpy(str, "ARD ");
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_TX_ACK)         // TX ACK
        str = mhs_stpcpy(str, "TX_ACK ");
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_ERROR_MSGS)
        str = mhs_stpcpy(str, "ERROR_MSGS ");                    // Error Messages Support
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_FD_HARDWARE)
        str = mhs_stpcpy(str, "FD_HARDWARE ");                   // CAN-FD Hardware
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_FIFO_OV_MODE)
        str = mhs_stpcpy(str, "FIFO_OV_MODE ");                  // FIFO OV Mode (Auto Clear, OV CAN Messages)        
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_HW_TIMESTAMP)
        str = mhs_stpcpy(str, "HW_TIMESTAMP ");
      printf("   Features-Flags   : %s\n\r", str_puf);
      printf("   CanChannelsCount : %u\n\r", l[i].ModulFeatures.CanChannelsCount);
      printf("   HwRxFilterCount  : %u\n\r", l[i].ModulFeatures.HwRxFilterCount);
      printf("   HwTxPufferCount  : %u\n\r", l[i].ModulFeatures.HwTxPufferCount);
      }
    printf("\n\r");
    }
  printf("=========================================================================\n");
  }
else
  printf("keine Devices gefunden.\n");
}


int32_t GetUsbDevices(void)
{
int32_t num_devs;

DevicesListCount = 0;
CanExDataFree((void **)&DevicesList);
if ((num_devs = CanExGetDeviceList(&DevicesList, 0)) > 0)
  DevicesListCount = num_devs;
return(num_devs);
}


// Plug & Play Event-Funktion
void DRV_CALLBACK_TYPE CanPnPEvent(uint32_t index, int32_t status)
{
DRV_LOCK_ENTER(&Lock);
GetUsbDevices();
PrintUsbDevices();
DRV_LOCK_LEAVE(&Lock);
}


// Status Event-Funktion
void DRV_CALLBACK_TYPE CanStatusEvent(uint32_t index, struct TDeviceStatus *status)
{
DRV_LOCK_ENTER(&Lock);
printf(">>> Status: Drv:%s, Can:%s, FIFO:%s\n\r", DrvStatusStrings[status->DrvStatus],
	   CanStatusStrings[status->CanStatus], CanFifoStrings[status->FifoStatus]);
DRV_LOCK_LEAVE(&Lock);
}


// RxD Event-Funktion
void DRV_CALLBACK_TYPE CanRxEvent(uint32_t index, struct TCanMsg *msg, int32_t count)
{
struct TCanMsg message;
unsigned long i;

DRV_LOCK_ENTER(&Lock);
while (CanReceive(0x80000000, &message, 1) > 0)
  {
  printf("[%02X] ", message.MsgSource);
  printf("id:%03X dlc:%01d data:", message.Id, message.MsgLen);
  if (message.MsgLen)
    {
    for (i = 0; i < message.MsgLen; i++)
      printf("%02X ", message.MsgData[i]);
    }
  else
    printf(" keine");
  printf("\n\r");
  }
DRV_LOCK_LEAVE(&Lock);
}


void ProcessOpenClose(int32_t key_idx)
{
int32_t i, err, open;
uint32_t dev_idx;
char str[30];

DRV_LOCK_ENTER(&Lock);
dev_idx = DeviceIndex[key_idx];
// Device in der Liste suchen
open = 0;
for (i = 0; i < DevicesListCount; i++)
  {
  if (dev_idx == DevicesList[i].TCanIdx)
    {
    open = 1;
    break;
    }
  }
if (open)
  {
  (void)CanDeviceClose(dev_idx);
  printf("CanDeviceClose [Key '%d']\n\r", key_idx+1);
  }
else
  {
  if (key_idx < DevicesListCount)
    {
    sprintf(str, "Snr=%s", DevicesList[key_idx].SerialNumber);
    if ((err = CanDeviceOpen(dev_idx, str)) < 0)
      printf("CanDeviceOpen [Key '%d'] Error-Code:%d\n\r", key_idx+1, err);
    else
      {
      printf("CanDeviceOpen [Key '%d'] ok\n\r", key_idx+1);
      // **** CAN Bus Start
      (void)CanSetMode(dev_idx, OP_CAN_START, CAN_CMD_ALL_CLEAR);
      }
    }
  }
DRV_LOCK_LEAVE(&Lock);
}


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int i, err;
char ch;

/*****************************************************/
/*  Message versenden                                */
/*****************************************************/
printf("Tiny-CAN \"GetDeviceList\" Demo\n\r");
printf("=============================\n\r\n\r");
//printf("Empfangene CAN Messages :\n\r");

DRV_LOCK_INIT(&Lock);
/*****************************************************/
/*  Treiber laden & Initialisieren                   */
/*****************************************************/

// **** Treiber DLL laden
if ((err = LoadDriver(TREIBER_NAME)) < 0)
  {
  printf("LoadDriver Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Treiber DLL im extended Mode initialisieren
if ((err = CanExInitDriver(NULL)) < 0)
  {
  printf("CanExInitDrv Error-Code:%d\n\r", err);
  goto ende;
  }
/*****************************************************/
/*  Device Objekte erzeugen                          */
/*****************************************************/
for (i = 0; i < 4; i++)
  {
  if ((err = CanExCreateDevice(&DeviceIndex[i], NULL)))
    printf("CanExCreateDevice [%u] Error-Code:%d\n\r", i, err);
  else
    printf("CanExCreateDevice [%u] Index:0x%08X\n\r", i, DeviceIndex[i]);
  }
/*****************************************************/
/*  Devices konfigurieren                            */
/*****************************************************/
for (i = 0; i < 4; i++)
  {
  // **** Übertragungsgeschwindigkeit einstellen
  if ((err = CanExSetAsUWord(DeviceIndex[i], "CanSpeed1", CAN_SPEED)) < 0)
    printf("Set \"CanSpeed1\" Fehler: %d\n\r", err);
  // **** 0 = Transmit Message Request sperren
  if ((err = CanExSetAsUByte(DeviceIndex[i], "CanTxAckEnable", 0)) < 0)
    printf("Set \"CanTxAckEnable\" Fehler: %d\n\r", err);
  // **** 3 = Hardware Time Stamps
  if ((err = CanExSetAsUByte(DeviceIndex[i], "TimeStampMode", 3)) < 0)
    printf("Set \"TimeStampMode\" Fehler: %d\n\r", err);
  }
/*****************************************************/
/*  Empfangs FIFO  erzeugen                          */
/*****************************************************/
if ((err = CanExCreateFifo(0x80000000, 10000, NULL, 0, 0xFFFFFFFF)))
  {
  printf("CanExCreateFifo Error-Code:%d\n\r", err);
  }
else
  printf("CanExCreateFifo ok\n\r");
printf("\n\r");
/*****************************************************/
/*  Device Liste erzeugen & ausgeben                 */
/*****************************************************/
GetUsbDevices();
PrintUsbDevices();

// **** Event Funktionen setzen
CanSetPnPEventCallback(&CanPnPEvent);
CanSetStatusEventCallback(&CanStatusEvent);
CanSetRxEventCallback(&CanRxEvent);
// **** Alle Events freigeben
CanSetEvents(EVENT_ENABLE_ALL);

ch = '\0';
do
  {
  if (KeyHit())
    {
    ch = getch();
    switch (ch)
      {
      case '1' : {
                 ProcessOpenClose(0);
                 break;
                 }
      case '2' : {
                 ProcessOpenClose(1);
                 break;
                 }
      case '3' : {
                 ProcessOpenClose(2);
                 break;
                 }
      case '4' : {
                 ProcessOpenClose(3);
                 break;
                 }
      }
    }
  }
while (ch != 'q');

/******************************/
/*  Treiber beenden           */
/******************************/
ende :

// **** Alle Events sperren
CanSetEvents(EVENT_DISABLE_ALL);
// **** Device Liste löschen
CanExDataFree((void **)&DevicesList);
// **** DLL entladen
UnloadDriver();

DRV_LOCK_DESTROY(&Lock);
return(0);
}
