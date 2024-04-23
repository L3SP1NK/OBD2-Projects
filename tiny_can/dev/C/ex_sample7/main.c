/**************************************************************************/
/*                Tiny-CAN API Demoprogramm "ex_sample7"                  */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : "CanExWaitForEvent" Demo                            */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 10.09.11                                            */
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

#define RX_EVENT     0x00000001
#define PNP_EVENT    0x00000002
#define STATUS_EVENT 0x00000004
#define CMD_EVENT    0x00000008


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


uint32_t DeviceIndex[4];
int32_t DevicesListCount = 0;
struct TCanDevicesList *DevicesList = NULL;
TMhsEvent *Event;
#ifdef __WIN32__
HANDLE Thread;
#else
pthread_t Thread;
#endif
volatile int32_t Cmd;


static void PrintUsbDevices(void)
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


static int32_t GetUsbDevices(void)
{
int32_t num_devs;

DevicesListCount = 0;
CanExDataFree((void **)&DevicesList);
if ((num_devs = CanExGetDeviceList(&DevicesList, 0)) > 0)
  DevicesListCount = num_devs;
return(num_devs);
}


static void ProcessOpenClose(int32_t key_idx)
{
if (Cmd)
  return;
Cmd = key_idx + 1;
CanExSetEvent(Event, CMD_EVENT);
}


static void ProcessCmd(int32_t cmd)
{
int32_t key_idx, i, err, open;
uint32_t dev_idx;
char str[30];

key_idx = cmd - 1;
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
}


static void RxMsg(void)
{
struct TCanMsg message;
unsigned long i;

while (CanReceive(0x80000000, &message, 1) > 0)
  {
  printf("[%02X] ", message.MsgSource);
  printf("id:%03X dlc:%0d data:", message.Id, message.MsgLen);
  if (message.MsgLen)
    {
    for (i = 0; i < message.MsgLen; i++)
      printf("%02X ", message.MsgData[i]);
    }
  else
    printf(" keine");
  printf("\n\r");
  }
}


// Plug & Play Event-Funktion
static void CanPnPEvent(void)
{
GetUsbDevices();
PrintUsbDevices();
}


// Status Event-Funktion
static void CanStatusEvent(void)
{
struct TDeviceStatus status;
uint32_t i, device_index;

for (i = 0; i < 4; i++)
  {
  device_index = DeviceIndex[i];
  CanGetDeviceStatus(device_index, &status);

  if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
    {
    if (status.CanStatus == CAN_STATUS_BUS_OFF)
      {
      printf("[%u] BusOff clear\n\r", i);
      CanSetMode(device_index, OP_CAN_RESET, CAN_CMD_NONE);
      }
    printf(">>> Status[%u]: Drv:%s, Can:%s, FIFO:%s\n\r", i, DrvStatusStrings[status.DrvStatus],
      CanStatusStrings[status.CanStatus], CanFifoStrings[status.FifoStatus]);
    }
  }
}


/****************/
/* Event Thread */
/****************/
#ifdef __WIN32__
static DWORD __stdcall thread_execute(void *data)
#else
static void *thread_execute(void *data)
#endif
{
uint32_t event;

do
  {
  event = CanExWaitForEvent(Event, 0);
  if (event & 0x80000000)        // Beenden Event, Thread Schleife verlassen
    break;
  else if (event & RX_EVENT)     // CAN Rx Event
    RxMsg();
  else if (event & PNP_EVENT)    // Pluy &  Play Event
    CanPnPEvent();
  else if (event & STATUS_EVENT) // Event Status änderung
    CanStatusEvent();
  else if (event & CMD_EVENT)    // Kommando Event
    {
    ProcessCmd(Cmd);
    Cmd = 0;
    }
  }
while (1);
return(0);
}


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int i, err;
char ch;

// Variablen Initialisieren
for (i = 0; i < 4; i++)
  DeviceIndex[i] = INDEX_INVALID;
#ifdef __WIN32__
Thread = NULL;
#else
Thread = -1;
#endif
Cmd = 0;
/*****************************************************/
/*  Message versenden                                */
/*****************************************************/
printf("Tiny-CAN \"CanExWaitForEvent\" Demo\n\r");
printf("=============================\n\r\n\r");
printf("Empfangene CAN Messages :\n\r");

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
// Keinen Callback Thread erzeugen, die Callback Funktionen stehen
// nicht zur verfügung
if ((err = CanExInitDriver("CanCallThread=0")) < 0)
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

/*****************************************************/
/*  Event Thread erzeugen & Events konfigurieren     */
/*****************************************************/
Event = CanExCreateEvent();   // Event Objekt erstellen
// Event Thread erzeugen und starten
#ifdef __WIN32__
Thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_execute,
                            NULL, 0, NULL);
#else
pthread_create(&Thread, NULL, thread_execute, NULL);
#endif
// Events mit API Ereignissen verknüfpen
CanExSetObjEvent(0x80000000, MHS_EVS_OBJECT, Event, RX_EVENT);
CanExSetObjEvent(INDEX_INVALID, MHS_EVS_PNP, Event, PNP_EVENT);
for (i = 0; i < 4; i++)
  CanExSetObjEvent(DeviceIndex[i], MHS_EVS_STATUS, Event, STATUS_EVENT);

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

#ifdef __WIN32__
if (Thread)
  {
  // Terminate Event setzen
  CanExSetEvent(Event, MHS_TERMINATE);
  // Warten bis der Event Thread beenden
  WaitForSingleObject(Thread, INFINITE);
  // Thread freigeben
  CloseHandle(Thread);
  }
#else
// Event Thread beenden
if (Thread > -1)
  {
  // Terminate Event setzen
  CanExSetEvent(Event, MHS_TERMINATE);
  // Thread beenden
  pthread_join(Thread, NULL);
  Thread = -1;
  }
#endif
// **** Device Liste löschen
CanExDataFree((void **)&DevicesList);
// **** DLL entladen
UnloadDriver();

return(0);
}
