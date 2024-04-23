/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "Sample1"                      */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL                           */
/*                    - Initialisierung des CAN-Buses                     */
/*                    - Versand einer CAN-Message                         */
/*                    - Empfang von CAN-Messages                          */
/*                    - Bus-Status abfragen und BusOff löschen            */
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


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int err;
unsigned long i;
struct TDeviceStatus status;   // Status
struct TCanMsg msg;

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
if ((err = CanInitDriver(TREIBER_INIT)) < 0)
  {
  printf("CanInitDrv Error-Code:%d\n\r", err);
  goto ende;
  }

// **** Tiny-CAN Device öffnen
if ((err = CanDeviceOpen(0, DEVICE_OPEN)) < 0)
  {
  printf("CanDeviceOpen Error-Code:%d\n\r", err);
  goto ende;
  }

/******************************/
/*  CAN Speed einstellen      */
/******************************/
// **** Übertragungsgeschwindigkeit auf 125kBit/s einstellen
CanSetSpeed(0, CAN_SPEED);

// **** CAN Bus Start, alle FIFOs, Filter, Puffer und Fehler löschen
CanSetMode(0, OP_CAN_START, CAN_CMD_ALL_CLEAR);

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
if ((err = CanTransmit(0, &msg, 1)) < 0)
  {
  printf("CanTransmit Error-Code:%d\n\r", err);
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
  CanGetDeviceStatus(0, &status);

  if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
    {
    if (status.CanStatus == CAN_STATUS_BUS_OFF)
      {
      printf("CAN Status BusOff\n\r");
      CanSetMode(0, OP_CAN_RESET, CAN_CMD_NONE);
      }
    }
  else
    {
    printf("CAN Device nicht geöffnet\n\r");
    goto ende;
    }

  if (CanReceive(0, &msg, 1) > 0)
    {
    printf("%10u.%10u ", msg.Time.Sec, msg.Time.USec);
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

CanDownDriver();
// **** DLL entladen
UnloadDriver();

return(0);
}
