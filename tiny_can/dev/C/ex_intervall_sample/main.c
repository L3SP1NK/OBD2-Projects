/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "ex_sample1"                   */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL und Initialisierung       */
/*                      im EX-Modus                                       */
/*                    - Initialisierung des CAN-Buses                     */
/*                    - Versand einer CAN-Message                         */
/*                    - Empfang von CAN-Messages                          */
/*                    - Bus-Status abfragen und BusOff löschen            */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 29.09.11                                            */
/*  Autor           : Demlehner Klaus, MHS-Elektronik, 94149 Kößlarn      */
/*                    info@mhs-elektronik.de  www.mhs-elektronik.de       */
/**************************************************************************/
//#include "config.h"

#define COM_MODE USB
#define HW_SNR "04000280"
#define CAN_SPEED CAN_500K_BIT

/*
TCAN_LOG_FLAG_MESSAGE      0x00000001
TCAN_LOG_FLAG_STATUS       0x00000002
TCAN_LOG_FLAG_RX_MSG       0x00000004
TCAN_LOG_FLAG_TX_MSG       0x00000008
TCAN_LOG_FLAG_API_CALL     0x00000010
TCAN_LOG_API_CALL_RX       0x00000020
TCAN_LOG_API_CALL_TX       0x00000040
TCAN_LOG_API_CALL_STATUS   0x00000080
TCAN_LOG_FLAG_ERROR        0x00000100
TCAN_LOG_FLAG_WARN         0x00000200
TCAN_LOG_FLAG_ERR_MSG      0x00000400
TCAN_LOG_FLAG_OV_MSG       0x00000800
TCAN_LOG_USB               0x00008000  // <*> neu
TCAN_LOG_FLAG_DEBUG        0x08000000
TCAN_LOG_FLAG_WITH_TIME    0x40000000
TCAN_LOG_FLAG_DISABLE_SYNC 0x80000000
*/

#define EX_INIT_STR "CanCallThread=0;LogFile=c:\\data\\log.txt;LogFlags=0xC8008F1F"

#include "global.h"
#include <string.h>
#include <stdio.h>
#ifdef __WIN32__
#include <conio.h>
#endif
#include "can_drv.h"

#define INTERVALL1_SUB_INDEX 1
#define mS(t) (t * 1000)  // Wandelt ms in us um

/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int err;
unsigned long i;
struct TDeviceStatus status;   // Status
struct TCanMsg msg;
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
// Keinen Callback Thread erzeugen, die Callback Funktionen stehen
// nicht zur verfügung
if ((err = CanExInitDriver(EX_INIT_STR)) < 0)
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
/*if ((err = CanExSetAsString(device_index, "Snr", "x")) < 0)  // x Seriennummer eintragen
  {
  printf("CanExSetAsString Error-Code:%d\n\r", err);
  goto ende;
  } */
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
CanSetSpeed(device_index, CAN_SPEED);

// Achtung: Um Fehler auf dem Bus zu vermeiden ist die Übertragungsgeschwindigkeit
//          vor dem starten des Busses einzustellen.

// **** CAN Bus Start, alle FIFOs, Filter, Puffer und Fehler löschen
CanSetMode(device_index, OP_CAN_START, CAN_CMD_ALL_CLEAR);
Sleep(250);
/******************************/
/*  Message versenden         */
/******************************/

// msg Variable Initialisieren
msg.MsgFlags = 0L; // Alle Flags löschen, Stanadrt Frame Format,
                   // keine RTR, Datenlänge auf 0

//msg.MsgRTR = 1;    // Nachricht als RTR Frame versenden
//msg.MsgEFF = 1;    // Nachricht im EFF (Ext. Frame Format) versenden

msg.Id = 0x100;    // Message Id auf 100 Hex
msg.MsgLen = 5;    // Datenlänge auf 5
memcpy(msg.MsgData, "HALLO", 5);
if ((err = CanTransmit(device_index | INTERVALL1_SUB_INDEX, &msg, 1)) < 0)
  {
  printf("CanTransmit Error-Code:%d\n\r", err);
  goto ende;
  }
Sleep(250);
if ((err = CanTransmitSet(device_index | INTERVALL1_SUB_INDEX, 0x8000, mS(1000))) < 0)
  {
  printf("CanTransmitSet Error-Code:%d\n\r", err);
  goto ende;
  }

printf("Tiny-CAN API Demoprogramm\n\r");
printf("=========================\n\r\n\r");
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

  if (CanReceive(device_index, &msg, 1) > 0)
    {
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
