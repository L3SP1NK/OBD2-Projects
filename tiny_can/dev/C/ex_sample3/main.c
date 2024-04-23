/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "ex_ample3"                    */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : Demonstriert die Verwendung eines virtuellen        */
/*                    Empfangs-Puffer, 2 Devices schreiben in den Puffer  */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 29.09.11                                            */
/*  Autor           : Demlehner Klaus, MHS-Elektronik, 94149 Kößlarn      */
/*                    info@mhs-elektronik.de  www.mhs-elektronik.de       */
/**************************************************************************/
#include "config.h"
#include "global.h"
#include <string.h>
#include <stdio.h>
#ifdef __WIN32__
#include <conio.h>
#endif
#include "can_drv.h"

// !!!! Seriennummern an eigene Devices anpassen !!!!
//#define DEVICE_OPEN_A "Snr=02090028"
#define DEVICE_OPEN_A "Snr="
#define DEVICE_OPEN_B "Snr=02080003"

#define RX_FIFO_INDEX 0x80000000

/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int err;
unsigned long i;
struct TDeviceStatus status;   // Status
struct TCanMsg msg;
uint32_t device_index_a, device_index_b;


/******************************/
/*  Initialisierung           */
/******************************/
device_index_a = INDEX_INVALID;
device_index_b = INDEX_INVALID;

// **** Initialisierung Utility Funktionen
UtilInit();
// **** Treiber DLL laden
if ((err = LoadDriver(TREIBER_NAME)) < 0)
  {
  printf("LoadDriver Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Treiber DLL initialisieren
// Keinen Callback Thread erzeugen, die Callback Funktionen stehen 
// nicht zur verfügung
if ((err = CanExInitDriver("CanCallThread=0")) < 0)
  {
  printf("CanInitDrv Error-Code:%d\n\r", err);
  goto ende;
  }
/********************************/
/*  Device A erzeugen u. öffnen */
/********************************/
// **** Device erzeugen
if ((err = CanExCreateDevice(&device_index_a, NULL)) < 0)
  {
  printf("CanExCreateDevice Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Schnittstelle PC <-> Tiny-CAN öffnen
if ((err = CanDeviceOpen(device_index_a, DEVICE_OPEN_A)) < 0)
  {
  printf("CanDeviceOpen Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Übertragungsgeschwindigkeit einstellen
CanSetSpeed(device_index_a, CAN_SPEED);

// Achtung: Um Fehler auf dem Bus zu vermeiden ist die Übertragungsgeschwindigkeit
//          vor dem starten des Busses einzustellen.

// **** CAN Bus Start, alle FIFOs, Filter, Puffer und Fehler löschen
CanSetMode(device_index_a, OP_CAN_START, CAN_CMD_ALL_CLEAR);
/********************************/
/*  Device B erzeugen u. öffnen */
/********************************/
// **** Device erzeugen
if ((err = CanExCreateDevice(&device_index_b, NULL)) < 0)
  {
  printf("CanExCreateDevice Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Schnittstelle PC <-> Tiny-CAN öffnen
if ((err = CanDeviceOpen(device_index_b, DEVICE_OPEN_B)) < 0)
  {
  printf("CanDeviceOpen Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Übertragungsgeschwindigkeit einstellen
CanSetSpeed(device_index_b, CAN_SPEED);

// Achtung: Um Fehler auf dem Bus zu vermeiden ist die Übertragungsgeschwindigkeit
//          vor dem starten des Busses einzustellen.

// **** CAN Bus Start, alle FIFOs, Filter, Puffer und Fehler löschen
CanSetMode(device_index_b, OP_CAN_START, CAN_CMD_ALL_CLEAR);

/*************************************************************/
/*  Empfangs FIFO  erzeugen und mit allen Devices verknüpfen */
/*************************************************************/
if ((err = CanExCreateFifo(RX_FIFO_INDEX, 10000, NULL, 0, 0x0))) 
  {
  printf("CanExCreateFifo Error-Code:%d\n\r", err);
  }
/*************************************************************/
/*  Devices mit dem FIFO verknüpfen                          */
/*************************************************************/
// Wird beim Aufruf von CanExCreateFifo als letzter Parameter 0xFFFFFFFF
// sind automatisch alle Devices mit dem FIFO verknüpft
(void)CanExBindFifo(RX_FIFO_INDEX, device_index_a, 1);
(void)CanExBindFifo(RX_FIFO_INDEX, device_index_b, 1);


printf("Tiny-CAN API Demoprogramm\n\r");
printf("=========================\n\r\n\r");
printf("Empfangene CAN-Messages :\n\r");

while (!KeyHit())
  {
  /**************/
  /*  Device A  */
  /**************/
  // **** Status abfragen 
  CanGetDeviceStatus(device_index_a, &status);

  if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
    {
    if (status.CanStatus == CAN_STATUS_BUS_OFF)
      {
      printf("CAN Status BusOff\n\r");
      CanSetMode(device_index_a, OP_CAN_RESET, CAN_CMD_NONE);
      }
    }
  else
    {
    printf("CAN Device nicht geöffnet\n\r");
    goto ende;
    }
  /**************/
  /*  Device B  */
  /**************/
  // **** Status abfragen 
  CanGetDeviceStatus(device_index_b, &status);

  if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
    {
    if (status.CanStatus == CAN_STATUS_BUS_OFF)
      {
      printf("CAN Status BusOff\n\r");
      CanSetMode(device_index_b, OP_CAN_RESET, CAN_CMD_NONE);
      }
    }
  else
    {
    printf("CAN Device nicht geöffnet\n\r");
    goto ende;
    }
  /********************************************/
  /*  Nachrichten FIFO auslesen und anzeigen  */
  /********************************************/ 
  if (CanReceive(RX_FIFO_INDEX, &msg, 1) > 0)
    {
    printf("[%02X]: ", msg.MsgSource);
    //printf("%10lu.%10lu ", msg.Time.Sec, msg.Time.USec);
    printf("id:%03X dlc:%01d data:", msg.Id, msg.MsgLen);
    if (msg.MsgLen)
      {
      for (i = 0; i < msg.MsgLen; i++)
        printf("%02X ", msg.MsgData[i]);
      }
    else
      printf(" keine");
    printf("\n\r");
    }
  }

/******************************/
/*  Treiber beenden           */
/******************************/
ende :

// Device schließen
(void)CanDeviceClose(device_index_a);
(void)CanDeviceClose(device_index_b);
// Device löschen
(void)CanExDestroyDevice(&device_index_a);
(void)CanExDestroyDevice(&device_index_b);
// CanDownDriver wird auch automatisch von UnloadDriver aufgerufen,
// der separate Aufruf ist nicht zwingend notwendig
CanDownDriver();
// **** DLL entladen
UnloadDriver();

return(0);
}
