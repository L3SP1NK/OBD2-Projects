/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "Sample3"                      */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL                           */
/*                    - Initialisierung des CAN-Buses                     */
/*                    - Alle 100ms eine CAN-Message versenden             */
/*                      (Sende Puffer 1)                                  */
/*                    - Setzen eines CAN Filters                          */
/*                    - Empfang von CAN-Messages                          */
/*                    - Bus-Status abfragen und BusOff l�schen            */
/*                                                                        */
/*  Version         : 1.10                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 07.03.07                                            */
/*  Autor           : Demlehner Klaus, MHS-Elektronik, 94149 K��larn      */
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
struct TMsgFilter msg_filter;

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
// Gr��e des Sende-Fifos auf 10 und des Empfangs-Fifos auf 100
if ((err = CanInitDriver("CanTxDFifoSize=10;CanRxDFifoSize=100")) < 0)
  {
  printf("CanInitDrv Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Schnittstelle PC <-> USB-Tiny �ffnen
// COM Port 1 ausw�hlen
if ((err = CanDeviceOpen(0, DEVICE_OPEN)) < 0)
  {
  printf("CanDeviceOpen Error-Code:%d\n\r", err);
  goto ende;
  }
/******************************/
/*  CAN Speed einstellen      */
/******************************/
// **** �bertragungsgeschwindigkeit auf 125kBit/s einstellen
CanSetSpeed(0, CAN_SPEED);

// **** CAN Bus Start
CanSetMode(0, OP_CAN_START, CAN_CMD_ALL_CLEAR);

/******************************/
/*  Sende Puffer 1 laden      */
/******************************/
// msg Variable Initialisieren
msg.MsgFlags = 0L; // Alle Flags l�schen, Stanadrt Frame Format,
                   // keine RTR, Datenl�nge auf 0
msg.Id = 0x100;
msg.MsgLen = 5;
memcpy(msg.MsgData, "HALLO", 5);

if ((err = CanTransmit(1, &msg, 1)) < 0)
  {
  printf("CanTransmit Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Intervalltimer auf 100ms setzten
if ((err = CanTransmitSet(1, 0x8000, mS(100))) < 0)
  {
  printf("CanTransmitSet Error-Code:%d\n\r", err);
  goto ende;
  }


/******************************/
/*  Filter 1 setzen           */
/******************************/

//         Bit 11     -     Bit0
// Maske   0 1 1 1 1 1 1 1 1 1 0 => 0x3FF
// Code    0 0 1 0 0 0 0 0 0 0 0 => 0x100
// Filter  X 0 0 0 0 0 0 0 0 0 X
// Die CAN Messages 0x100 u. 0x500 werden gefiltert
msg_filter.Maske = 0x3FF;
msg_filter.Code = 0x100;
msg_filter.Flags.Long = 0L;
msg_filter.FilEnable = 1;   // Filter freigeben

if ((err = CanSetFilter(1, &msg_filter)) < 0)
  {
  printf("CanSetFilter Error-Code:%d\n\r", err);
  goto ende;
  }

printf("Tiny-CAN API Demoprogramm\n\r");
printf("=========================\n\r\n\r");
printf("Empfangene CAN Messages :\n\r");

while (!KeyHit())
  {
  /******************************/
  /*  Status abfragen           */
  /******************************/
  CanGetDeviceStatus(0, &status);

  if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
    {   // CAN Device ge�ffnet 
    if (status.CanStatus == CAN_STATUS_BUS_OFF)
      { // CAN Controller im BusOff
      printf("CAN Status BusOff\n\r");
      CanSetMode(0, OP_CAN_RESET, CAN_CMD_NONE);  // BusOff l�schen
      }
    }
  else
    {
    printf("CAN Device nicht ge�ffnet\n\r");
    goto ende;
    }

  if (CanReceive(0, &msg, 1) > 0)
    {
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
    
  if (CanReceive(1, &msg, 1) > 0)
    {
    printf("F1 -> id:%03X dlc:%01d data:", msg.Id, msg.MsgLen);
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

if ((err = CanTransmitSet(1, 0x8000, 0)) < 0)
  {
  printf("CanTransmitSet Error-Code:%d\n\r", err);
  goto ende;
  }


/******************************/
/*  Treiber beenden           */
/******************************/
ende :
// **** DLL entladen
UnloadDriver();

return(0);
}
