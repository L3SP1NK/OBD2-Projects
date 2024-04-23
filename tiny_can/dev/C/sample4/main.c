/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "Sample4"                      */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL                           */
/*                    - Initialisierung des CAN-Buses                     */
/*                    - Setzen mehrerer CAN Filter                        */
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
if ((err = CanInitDriver(NULL)) < 0)
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


/**********************************************************************/
/*               Filter 1 setzen  (Index = 0x00000001L)               */
/* ================================================================== */
/* Type: Hardware Filter, Single Id                                   */
/* CAN Nachrichten mit der ID 0x010 sollen gefiltert werden           */
/**********************************************************************/
msg_filter.FilFlags = 0L;    // Alle Flags mit 0 Initialisieren
                             // Dlc = 0, RTR = 0, EFF = 0, Mode = 0

msg_filter.Code = 0x010;     // ID = 0x10
msg_filter.Maske = 0x000;    // Wird nicht verwendet

msg_filter.FilIdMode = 2;    // 2 = Single Id

msg_filter.FilEnable = 1;    // Filter freigeben

if ((err = CanSetFilter(0x00000001L, &msg_filter)) < 0)  // Filter mit Index 0x00000001L setzen
  {
  printf("CanSetFilter Error-Code:%d\n\r", err);
  goto ende;
  }


/**********************************************************************/
/*               Filter 2 setzen  (Index = 0x00000002L)               */
/* ================================================================== */
/* Type: Hardware Filter, Maske & Code                                */
/* Die CAN Nachrichten mit der ID 0x000 - 0x001 und 0x400 - 0x401     */
/* sollen gefiltert werden                                            */
/**********************************************************************/
msg_filter.FilFlags = 0L;    // Alle Flags mit 0 Initialisieren
                             // Dlc = 0, RTR = 0, EFF = 0, Mode = 0

//         Bit 11     -     Bit0
// Maske   0 1 1 1 1 1 1 1 1 1 0 => 0x3FE
// Code    0 0 0 0 0 0 0 0 0 0 0 => 0x000
// Filter  X 0 0 0 0 0 0 0 0 0 X
//
msg_filter.Code = 0x000;    // Code = 0x000
msg_filter.Maske = 0x3FE;   // Maske = 0x3FE

msg_filter.FilIdMode = 0;   // 0 = Maske & Code

msg_filter.FilEnable = 1;   // Filter freigeben

if ((err = CanSetFilter(0x00000002L, &msg_filter)) < 0)  // Filter mit Index 0x00000002L setzen
  {
  printf("CanSetFilter Error-Code:%d\n\r", err);
  goto ende;
  }


/**********************************************************************/
/*               Filter 3 setzen  (Index = 0x02000001L)               */
/* ================================================================== */
/* Type: Software Filter, Single Id                                   */
/* CAN Nachrichten mit der ID 0x100 sollen gefiltert werden           */
/**********************************************************************/
msg_filter.FilFlags = 0L;    // Alle Flags mit 0 Initialisieren
                             // Len = 0, RTR = 0, EFF = 0, Mode = 0

msg_filter.Code = 0x100;     // ID = 0x100
msg_filter.Maske = 0x000;    // wird nicht verwendet

msg_filter.FilIdMode = 2;    // 2 = Single Id

msg_filter.FilEnable = 1;    // Filter freigeben

if ((err = CanSetFilter(0x00000001L | INDEX_SOFT_FLAG, &msg_filter)) < 0)  // Filter mit Index 0x02000001L setzen
  {
  printf("CanSetFilter Error-Code:%d\n\r", err);
  goto ende;
  }


/*************************************************************************/
/*                Filter 4 setzen  (Index = 0x02000002L)                 */
/* ===================================================================== */
/* Type: Software Filter, Start & Stop (Einen Bereich von bis Filtern)   */
/* CAN Nachrichten mit der ID ab 0x200 bis 0x215 sollen gefiltert werden */
/*************************************************************************/
msg_filter.FilFlags = 0L;    // Alle Flags mit 0 Initialisieren
                             // Len = 0, RTR = 0, EFF = 0, Mode = 0

msg_filter.Code = 0x200;     // Start ID = 0x200
msg_filter.Maske = 0x215;    // Stop ID = 0x215

msg_filter.FilIdMode = 1;    // 1 = Start & Stop

msg_filter.FilEnable = 1;    // Filter freigeben

if ((err = CanSetFilter(0x00000002L | INDEX_SOFT_FLAG, &msg_filter)) < 0)  // Filter mit Index 0x02000002L setzen
  {
  printf("CanSetFilter Error-Code:%d\n\r", err);
  goto ende;
  }


/**********************************************************************/
/*               Filter 5 setzen  (Index = 0x02000003L)               */
/* ================================================================== */
/* Type: Software Filter, Single Id                                   */
/* CAN Nachrichten mit der ID 0x100 sollen gefiltert werden und im    */
/* Empfangs-Fifo verbleiben                                           */
/**********************************************************************/
msg_filter.FilFlags = 0L;    // Alle Flags mit 0 Initialisieren
                             // Len = 0, RTR = 0, EFF = 0, Mode = 0

msg_filter.Code = 0x100;     // ID = 0x100
msg_filter.Maske = 0x000;    // wird nicht verwendet

msg_filter.FilIdMode = 2;    // 2 = Single Id
msg_filter.FilMode = 1;      // 1 = Die Nachricht wird nicht aus dem Datenstrom gelöscht,
                             //     die Nachricht steht im Puffer und im FIFO

msg_filter.FilEnable = 1;    // Filter freigeben

if ((err = CanSetFilter(0x00000003L | INDEX_SOFT_FLAG, &msg_filter)) < 0)  // Filter mit Index 0x02000003L setzen
  {
  printf("CanSetFilter Error-Code:%d\n\r", err);
  goto ende;
  }

/**************************************************************************************/
/* Die Anzahl der zur Verfügung stehenden Hardware Filter ist abhängig von der        */
/* verwendeten Hardware                                                               */
/*                                                                                    */
/* Wird ein Filter mit dem gleichen Index noch einmal geschrieben                     */
/* so wird das Filter überschrieben                                                   */
/*                                                                                    */
/* Hardwre Filter können als "Single Id (IdMode = 2)" und "Maske & Code (IdMode = 0)" */
/* gesetzt werden, "Start & Stop (IdMode = 1)" ist nicht möglich                      */
/* Software Filter können in allen drei Modis gesetzt werden                          */
/* Aufbau der Struktur "TMsgFilter" ist in der Datei "can_types.h" zu finden          */
/* Die Felder Len, DLCCheck, DataCheck sind für zukünftige Anwendungen reserviert     */
/**************************************************************************************/


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
// **** DLL entladen
UnloadDriver();

return(0);
}
