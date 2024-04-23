/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "ex_sample2"                   */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL                           */
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
#include "config.h"
#include "global.h"
#include <string.h>
#include <stdio.h>
#ifdef __WIN32__
#include <conio.h>
#endif
#include "can_drv.h"

// !!!! Seriennummern an eigene Devices anpassen !!!!
#define DEVICE_OPEN_A "Snr=00000000"
#define DEVICE_OPEN_B "Snr=01002362"


#define mS(t) (t * 1000)  // Wandelt ms in us um

#define INTERVALL1_SUB_INDEX  1
#define INTERVALL2_SUB_INDEX  2

/******************************/
/*  Intervall Puffer laden    */
/******************************/
static int32_t SetupAndStartIntervallMsgs(uint32_t index, struct TCanMsg *msg_a, struct TCanMsg *msg_b) 
{
int32_t err;

if ((err = CanTransmit(index + INTERVALL1_SUB_INDEX, msg_a, 1)) < 0)  
  printf("CanTransmit Error-Code:%d\n\r", err);    
// **** Intervalltimer auf 100ms setzten
else if ((err = CanTransmitSet(index + INTERVALL1_SUB_INDEX, 0x8000, mS(100))) < 0)  
  printf("CanTransmitSet Error-Code:%d\n\r", err);
else if ((err = CanTransmit(index + INTERVALL2_SUB_INDEX, msg_b, 1)) < 0)  
  printf("CanTransmit Error-Code:%d\n\r", err);    
// **** Intervalltimer auf 100ms setzten
else if ((err = CanTransmitSet(index + INTERVALL2_SUB_INDEX, 0x8000, mS(1000))) < 0)  
  printf("CanTransmitSet Error-Code:%d\n\r", err);
return(err);    
}  


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int err;
unsigned long i;
struct TDeviceStatus status;   // Status
struct TCanMsg msg, msg_b;
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
// **** Device u. Empfangs-FIFO für das Device erzeugen
if ((err = CanExCreateDevice(&device_index_a, "CanRxDFifoSize=16384")) < 0)
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
// **** Device u. Empfangs-FIFO für das Device erzeugen
if ((err = CanExCreateDevice(&device_index_b, "CanRxDFifoSize=16384")) < 0)
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

printf("Tiny-CAN API Demoprogramm\n\r");
printf("=========================\n\r\n\r");
printf("Empfangene CAN-Messages :\n\r");

// msg Variable Initialisieren
msg.MsgFlags = 0L; // Alle Flags löschen, Stanadrt Frame Format,
                     // keine RTR, Datenlänge auf 0
msg.Id = 0x100;
msg.MsgLen = 8;
memcpy(msg.MsgData, "HALLO 01", 8);

msg_b.MsgFlags = 0L; // Alle Flags löschen, Stanadrt Frame Format,
                     // keine RTR, Datenlänge auf 0
msg_b.Id = 0x101;
msg_b.MsgLen = 8;
memcpy(msg_b.MsgData, "HALLO 02", 8);

if (SetupAndStartIntervallMsgs(device_index_a, &msg, &msg_b) < 0)
  goto ende;
  
msg.MsgFlags = 0L; // Alle Flags löschen, Stanadrt Frame Format,
                     // keine RTR, Datenlänge auf 0
msg.Id = 0x200;
msg.MsgLen = 8;
memcpy(msg.MsgData, "HALLO 11", 8);

msg_b.MsgFlags = 0L; // Alle Flags löschen, Stanadrt Frame Format,
                     // keine RTR, Datenlänge auf 0
msg_b.Id = 0x201;
msg_b.MsgLen = 8;
memcpy(msg_b.MsgData, "HALLO 12", 8);  
if (SetupAndStartIntervallMsgs(device_index_b, &msg, &msg_b) < 0)
  goto ende; 

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
  // **** Nachrichten Empfangen und anzeigen
  if (CanReceive(device_index_a, &msg, 1) > 0)
    {
    printf("A: ");
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
  // **** Nachrichten Empfangen und anzeigen
  if (CanReceive(device_index_b, &msg, 1) > 0)
    {
    printf("B: ");
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

(void)CanTransmitSet(device_index_a + INTERVALL1_SUB_INDEX, 0x8000, 0);
(void)CanTransmitSet(device_index_a + INTERVALL2_SUB_INDEX, 0x8000, 0);
(void)CanTransmitSet(device_index_b + INTERVALL1_SUB_INDEX, 0x8000, 0);
(void)CanTransmitSet(device_index_b + INTERVALL2_SUB_INDEX, 0x8000, 0);
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
