/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "Sample8"                      */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL                           */
/*                    - Initialisierung des CAN-Buses                     */
/*                    - Versand einer CAN-Message                         */
/*                    - Empfang von CAN-Messages nur einer ausgewählten ID*/
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


int SetMyCanId(unsigned long id, unsigned char eff)
{
int err, set_end;
struct TMsgFilter msg_filter;

msg_filter.Flags.Long = 0L;
if (eff)
  msg_filter.FilEFF = 1;
msg_filter.FilIdMode = 1;   // Modus Start - Stop
msg_filter.FilEnable = 1;   // Filter freigeben

if (id > 0)
  {
  msg_filter.Code = 0L;    // Start
  msg_filter.Maske = id-1;   // Stop

  if ((err = CanSetFilter(1L | INDEX_SOFT_FLAG, &msg_filter)) < 0)
    return(err);
  }
set_end = 0;
if (eff)
  {
  if (id < 0x1FFFFFFF)
    set_end = 1;
  }
else
  {
  if (id < 0x7FF)
    set_end = 1;
  }
if (set_end)
  {
  msg_filter.Code = id+1;      // Start
  msg_filter.Maske = 0x1FFFFFFF;   // Stop
  if ((err = CanSetFilter(2L | INDEX_SOFT_FLAG, &msg_filter)) < 0)
    return(err);
  }
return(0);
}


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
// **** Schnittstelle PC <-> USB-Tiny öffnen
// COM Port 1 auswählen
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

// **** CAN Bus Start
CanSetMode(0, OP_CAN_START, CAN_CMD_ALL_CLEAR);

printf("Tiny-CAN API Demoprogramm\n\r");
printf("=========================\n\r\n\r");
printf("Empfangene CAN-Messages :\n\r");

if ((err = SetMyCanId(0x100, 0)) < 0)  // Ihre Id ist 0x100, Standart Format
  {
  printf("SetMyCanId Error-Code:%d\n\r", err);
  goto ende;
  }

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
      CanSetMode(0, OP_CAN_RESET, CAN_CMD_ALL_CLEAR);
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
