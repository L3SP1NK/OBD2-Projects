/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "Sample2"                      */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL und Initialisierung des   */
/*                      CAN-Buses                                         */
/*                    - Setzen der Callbackfunktionen                     */
/*                    - An- und Abstecken der Hardware, Statusänderungen  */
/*                      und den Empfang von CAN-Messages in Callback-     */
/*                      Funktionen verarbeiten                            */
/*                    - Alle 2 Sekunden eine CAN-Message versenden        */
/*                                                                        */
/*  Version         : 1.10                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 07.03.07                                            */
/*  Autor           : Demlehner Klaus, MHS-Elektronik, 94149 Kößlarn      */
/*                    info@mhs-elektronik.de  www.mhs-elektronik.de       */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include "config.h"
#include "global.h"
#include <string.h>
#include <stdio.h>
#ifdef __WIN32__
#include <conio.h>
#endif
#include "can_drv.h"


const char *DrvStatusStrings[] =
     {"DRV_NOT_LOAD",             // Die Treiber DLL wurde noch nicht geladen
      "DRV_STATUS_NOT_INIT",      // Treiber noch nicht Initialisiert (Funktion "CanInitDrv" noch nicht aufgerufen)
      "DRV_STATUS_INIT",          // Treiber erfolgrich Initialisiert
      "DRV_STATUS_PORT_NOT_OPEN", // Die Schnittstelle wurde nicht geöffnet
      "DRV_STATUS_PORT_OPEN",     // Die Schnittstelle wurde geöffnet
      "DRV_STATUS_DEVICE_FOUND",  // Verbindung zur Hardware wurde Hergestellt
      "DRV_STATUS_CAN_OPEN",      // Device wurde geöffnet und erfolgreich Initialisiert
      "DRV_STATUS_CAN_RUN_TX",    // CAN Bus RUN nur Transmitter (wird nicht verwendet !)
      "DRV_STATUS_CAN_RUN"};      // CAN Bus RUN

const char *CanStatusStrings[] =
     {"CAN_STATUS_OK",            // Ok
      "CAN_STATUS_ERROR",         // CAN Error
      "CAN_STATUS_WARNING",       // Error warning
      "CAN_STATUS_PASSIV",        // Error passiv
      "CAN_STATUS_BUS_OFF",       // Bus Off
      "CAN_STATUS_UNBEKANNT"};    // Status Unbekannt

const char *CanFifoStrings[] =
     {"FIFO_OK",
      "CAN_FIFO_HW_OVERRUN",
      "CAN_FIFO_SW_OVERRUN",
      "CAN_FIFO_HW_SW_OVERRUN",
      "CAN_FIFO_STATUS_UNBEKANNT"};


unsigned char Online;   // Hardware Online


// Plug & Play Event-Funktion
static void CALLBACK_TYPE CanPnPEvent(uint32_t index, int32_t status)
{
if (status)
  {
  CanDeviceOpen(0, DEVICE_OPEN);
  // **** CAN Bus Start
  CanSetMode(0, OP_CAN_START, CAN_CMD_ALL_CLEAR);
  Online = 1;
  printf(">>> Tiny-CAN Connect\n\r");
  }
else
  {
  Online = 0;
  printf(">>> Tiny-CAN Disconnect\n\r");
  }
}


// Status Event-Funktion
static void CALLBACK_TYPE CanStatusEvent(uint32_t index, struct TDeviceStatus *status)
{
printf(">>> Status: %s, %s, %s\n\r", DrvStatusStrings[status->DrvStatus],
	   CanStatusStrings[status->CanStatus], CanFifoStrings[status->FifoStatus]);
if (status->DrvStatus >= DRV_STATUS_CAN_OPEN)
  {
  if (status->CanStatus == CAN_STATUS_BUS_OFF)
    {
    printf(">>> CAN Status BusOff clear\n\r");
    CanSetMode(0, OP_CAN_RESET, CAN_CMD_ALL_CLEAR);
    }
  }
}


// RxD Event-Funktion
static void CALLBACK_TYPE CanRxEvent(uint32_t index, struct TCanMsg *msg, int32_t count)
{
struct TCanMsg message;
unsigned long i;

while (CanReceive(0, &message, 1) > 0)
  {
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


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int err;
struct TCanMsg msg;


#ifndef __WIN32__
UtilInit();
#endif
/******************************/
/*  Message versenden         */
/******************************/
printf("Tiny-CAN API Demoprogramm\n\r");
printf("=========================\n\r\n\r");
printf("Empfangene CAN Messages :\n\r");

Online = 0;
/******************************/
/*  Initialisierung           */
/******************************/

// **** Treiber DLL laden
if ((err = LoadDriver(TREIBER_NAME)) < 0)
  {
  printf("LoadDriver Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Treiber DLL initialisieren
if ((err = CanInitDriver(NULL)) < 0)
  {
  printf("CanInitDrv Error-Code:%d\n\r", err);
  goto ende;
  }
/******************************/
/*  CAN Speed einstellen      */
/******************************/
// **** Übertragungsgeschwindigkeit auf 125kBit/s einstellen
CanSetSpeed(0, CAN_125K_BIT);
// **** AutoConnect auf 1
CanSetOptions("AutoConnect=1;AutoReopen=0");
// **** Event Funktionen setzen
CanSetPnPEventCallback(&CanPnPEvent);
CanSetStatusEventCallback(&CanStatusEvent);
CanSetRxEventCallback(&CanRxEvent);
// **** Alle Events freigeben
CanSetEvents(EVENT_ENABLE_ALL);
// **** Schnittstelle PC <-> Tiny-CAN öffnen
if ((err = CanDeviceOpen(0, DEVICE_OPEN)) < 0)
  printf("CanDeviceOpen Error-Code:%d\n\r", err);
else
{
  if (!CanSetMode(0, OP_CAN_START, CAN_CMD_ALL_CLEAR))
    Online = 1;
}
// **** CAN Bus Start

while (!KeyHit())
  {
  // msg Variable Initialisieren
  msg.MsgFlags = 0L; // Alle Flags löschen, Stanadrt Frame Format,
                     // keine RTR, Datenlänge auf 0
  msg.Id = 0x100;
  msg.MsgLen = 5;
  memcpy(msg.MsgData, "HALLO", 5);

  if (Online)
    CanTransmit(0, &msg, 1);
  Sleep(2000);   // 2 Sekunden warten
  }

/******************************/
/*  Treiber beenden           */
/******************************/
ende :

// **** Alle Events sperren
CanSetEvents(EVENT_DISABLE_ALL);
// **** DLL entladen
UnloadDriver();

return(0);
}
