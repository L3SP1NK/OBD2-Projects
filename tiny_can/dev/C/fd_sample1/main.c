/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "fd_sample1"                   */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - API Treiber laden und für CAN-FD Initialisieren   */
/*                    - Initialisierung des CAN-Buses                     */
/*                    - Versand einer CAN-Message                         */
/*                    - Empfang von CAN-Messages                          */
/*                    - Bus-Status abfragen und BusOff löschen            */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 06.06.2020                                          */
/*  Autor           : Demlehner Klaus                                     */
/*  Copyright       : (C) 2020 by MHS-Elektronik GmbH & Co. KG, Germany   */
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
#include "linux_util.h"
#include "util.h"

/*
Default Setup in config.h

Nominal CAN Speed : 125 kBit/s
Data CAN Speed    : 1 MBit/s
*/

#define TIME_STAMP_MODE ";TimeStampMode=4"
                                 // TimeStampMode
                                 //   0 = Disabled
                                 //   1 = Software Time Stamps
                                 //   2 = Hardware Time Stamps, UNIX-Format
                                 //   3 = Hardware Time Stamps
                                 //   4 = Hardware Time Stamp wenn verfügbar, ansonsten Software Time Stamps

                                //   0     1     2     3     4     5     6     7
static const uint8_t TestData[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                   0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
                                   0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8,
                                   0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8,
                                   0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8,
                                   0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8,
                                   0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8,
                                   0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x55, 0xAA};

/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int err;
struct TDeviceStatus status;   // Status
struct TCanFdMsg fd_msg;
uint32_t device_index;

device_index = INDEX_INVALID;
/******************************/
/*  Initialisierung           */
/******************************/

// **** Initialisierung Utility Funktionen
UtilInit();
// **** Treiber DLL laden
if ((err = LoadDriver(TREIBER_NAME)) < 0)
  {
  printf("LoadDriver Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Treiber DLL initialisieren
// FdMode=1 -> Treiber für CAN-FD initialisieren
// CanCallThread=0 -> Keinen Callback Thread erzeugen, die Callback Funktionen stehen
// nicht zur verfügung
if ((err = CanExInitDriver("FdMode=1;CanCallThread=0"TIME_STAMP_MODE PUBLIC_INIT_STR)) < 0)
  {
  printf("CanInitDrv Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Device u. Empfangs-FIFO für das Device erzeugen
if ((err = CanExCreateDevice(&device_index, "CanRxDFifoSize=16384")) < 0)
  {
  printf("CanExCreateDevice Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Seriennummer setzen
#ifdef HW_SNR
if ((err = CanExSetAsString(device_index, "Snr", HW_SNR)) < 0)  // x Seriennummer eintragen
  {
  printf("CanExSetAsString Error-Code:%d\n\r", err);
  goto ende;
  }
#endif
// **** Schnittstelle PC <-> Tiny-CAN öffnen
if ((err = CanDeviceOpen(device_index, DEVICE_OPEN)) < 0)
  {
  printf("CanDeviceOpen Error-Code:%d\n\r", err);
  goto ende;
  }
/*****************************************/
/*  CAN Speed einstellen & Bus starten   */
/*****************************************/

// **** Übertragungsgeschwindigkeit einstellen
#ifdef CAN_SPEED
if ((err = CanExSetAsUWord(device_index, "CanSpeed1", CAN_SPEED)) < 0)
  {
  printf("Set \"CanSpeed1\" Error-Code: %d\n\r", err);
  goto ende;
  }
#endif
#ifdef CAN_SPEED_BTR_VALUE
if ((err = CanExSetAsULong(device_index, "CanSpeed1User", )) < 0)
  {
  printf("Set \"CanSpeed1User\" Error-Code: %d\n\r", err);
  goto ende;
  }
#endif

#ifdef CAN_DATA_SPEED
if ((err = CanExSetAsUWord(device_index, "CanDSpeed1", CAN_DATA_SPEED)) < 0)
  {
  printf("Set \"CanDSpeed1\" Error-Code: %d\n\r", err);
  goto ende;
  }
#endif
#ifdef CAN_SPEED_DBTR_VALUE
if ((err = CanExSetAsULong(device_index, "CanDSpeed1User", CAN_SPEED_DBTR_VALUE)) < 0)
  {
  printf("Set \"CanDSpeed1User\" Error-Code: %d\n\r", err);
  goto ende;
  }
#endif

// Achtung: Um Fehler auf dem Bus zu vermeiden ist die Übertragungsgeschwindigkeit
//          vor dem starten des Busses einzustellen.

// **** CAN Bus Start, alle FIFOs, Filter, Puffer und Fehler löschen
CanSetMode(device_index, OP_CAN_START, CAN_CMD_ALL_CLEAR);

/******************************/
/*  Message versenden         */
/******************************/

// msg Variable Initialisieren
fd_msg.MsgFlags = 0L; // Alle Flags löschen, Stanadrt Frame Format,
                   // keine RTR, Datenlänge auf 0
fd_msg.MsgFD = 1;       // CAN-FD Message
fd_msg.MsgBRS = 1;      // (B)it (R)ate (S)witch
//fd_msg.MsgRTR = 1;    // Nachricht als RTR Frame versenden
//fd_msg.MsgEFF = 1;    // Nachricht im EFF (Ext. Frame Format) versenden

fd_msg.Id = 0x100;    // Message Id auf 100 Hex
fd_msg.MsgLen = sizeof(TestData);
memcpy(fd_msg.MsgData, TestData, sizeof(TestData));
if ((err = CanFdTransmit(device_index, &fd_msg, 1)) < 0)
  {
  printf("CanFdTransmit Error-Code:%d\n\r", err);
  goto ende;
  }

printf("Tiny-CAN API Demoprogramm - fd_sample1\n\r");
printf("======================================\n\r\n\r");
printf("Empfangene CAN-Messages :\n\r");

while (!KeyHit())
  {
  /******************************/
  /*  Status abfragen           */
  /******************************/
  CanGetDeviceStatus(device_index, &status);

  if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
    {
    if (status.CanStatus == CAN_STATUS_BUS_OFF)
      {
      printf("CAN Status BusOff\n\r");
      CanSetMode(device_index, OP_CAN_RESET, CAN_CMD_NONE);
      }
    }
  else
    {
    printf("CAN Device nicht geöffnet\n\r");
    goto ende;
    }

  if (CanFdReceive(device_index, &fd_msg, 1) > 0)
    PrintFdMessages(&fd_msg, 1, 1, 0);
  }

/******************************/
/*  Treiber beenden           */
/******************************/
ende :

// Device schließen
(void)CanDeviceClose(device_index);
// Device löschen
(void)CanExDestroyDevice(&device_index);
// CanDownDriver wird auch automatisch von UnloadDriver aufgerufen,
// der separate Aufruf ist nicht zwingend notwendig
CanDownDriver();
// **** DLL entladen
UnloadDriver();

return(0);
}
